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

#include "hal_hw_headers.h"
#include "dp_types.h"
#include "dp_rx.h"
#include "dp_tx.h"
#include "dp_peer.h"
#include "hal_rx.h"
#include "hal_api.h"
#include "qdf_nbuf.h"
#ifdef MESH_MODE_SUPPORT
#include "if_meta_hdr.h"
#endif
#include "dp_internal.h"
#include "dp_ipa.h"
#include "dp_hist.h"
#include "dp_rx_buffer_pool.h"
#ifdef WIFI_MONITOR_SUPPORT
#include "dp_htt.h"
#include <dp_mon.h>
#endif
#ifdef FEATURE_WDS
#include "dp_txrx_wds.h"
#endif

#ifdef DUP_RX_DESC_WAR
void dp_rx_dump_info_and_assert(struct dp_soc *soc,
				hal_ring_handle_t hal_ring,
				hal_ring_desc_t ring_desc,
				struct dp_rx_desc *rx_desc)
{
	void *hal_soc = soc->hal_soc;

	hal_srng_dump_ring_desc(hal_soc, hal_ring, ring_desc);
	dp_rx_desc_dump(rx_desc);
}
#else
void dp_rx_dump_info_and_assert(struct dp_soc *soc,
				hal_ring_handle_t hal_ring_hdl,
				hal_ring_desc_t ring_desc,
				struct dp_rx_desc *rx_desc)
{
	hal_soc_handle_t hal_soc = soc->hal_soc;

	dp_rx_desc_dump(rx_desc);
	hal_srng_dump_ring_desc(hal_soc, hal_ring_hdl, ring_desc);
	hal_srng_dump_ring(hal_soc, hal_ring_hdl);
	qdf_assert_always(0);
}
#endif

#ifndef QCA_HOST_MODE_WIFI_DISABLED
#ifdef RX_DESC_SANITY_WAR
QDF_STATUS dp_rx_desc_sanity(struct dp_soc *soc, hal_soc_handle_t hal_soc,
			     hal_ring_handle_t hal_ring_hdl,
			     hal_ring_desc_t ring_desc,
			     struct dp_rx_desc *rx_desc)
{
	uint8_t return_buffer_manager;

	if (qdf_unlikely(!rx_desc)) {
		/*
		 * This is an unlikely case where the cookie obtained
		 * from the ring_desc is invalid and hence we are not
		 * able to find the corresponding rx_desc
		 */
		goto fail;
	}

	return_buffer_manager = hal_rx_ret_buf_manager_get(hal_soc, ring_desc);
	if (qdf_unlikely(!(return_buffer_manager ==
				HAL_RX_BUF_RBM_SW1_BM(soc->wbm_sw0_bm_id) ||
			 return_buffer_manager ==
				HAL_RX_BUF_RBM_SW3_BM(soc->wbm_sw0_bm_id)))) {
		goto fail;
	}

	return QDF_STATUS_SUCCESS;

fail:
	DP_STATS_INC(soc, rx.err.invalid_cookie, 1);
	dp_err("Ring Desc:");
	hal_srng_dump_ring_desc(hal_soc, hal_ring_hdl,
				ring_desc);
	return QDF_STATUS_E_NULL_VALUE;

}
#endif
#endif /* QCA_HOST_MODE_WIFI_DISABLED */

/**
 * dp_pdev_frag_alloc_and_map() - Allocate frag for desc buffer and map
 *
 * @dp_soc: struct dp_soc *
 * @nbuf_frag_info_t: nbuf frag info
 * @dp_pdev: struct dp_pdev *
 * @rx_desc_pool: Rx desc pool
 *
 * Return: QDF_STATUS
 */
#ifdef DP_RX_MON_MEM_FRAG
static inline QDF_STATUS
dp_pdev_frag_alloc_and_map(struct dp_soc *dp_soc,
			   struct dp_rx_nbuf_frag_info *nbuf_frag_info_t,
			   struct dp_pdev *dp_pdev,
			   struct rx_desc_pool *rx_desc_pool)
{
	QDF_STATUS ret = QDF_STATUS_E_FAILURE;

	(nbuf_frag_info_t->virt_addr).vaddr =
			qdf_frag_alloc(rx_desc_pool->buf_size);

	if (!((nbuf_frag_info_t->virt_addr).vaddr)) {
		dp_err("Frag alloc failed");
		DP_STATS_INC(dp_pdev, replenish.frag_alloc_fail, 1);
		return QDF_STATUS_E_NOMEM;
	}

	ret = qdf_mem_map_page(dp_soc->osdev,
			       (nbuf_frag_info_t->virt_addr).vaddr,
			       QDF_DMA_FROM_DEVICE,
			       rx_desc_pool->buf_size,
			       &nbuf_frag_info_t->paddr);

	if (qdf_unlikely(QDF_IS_STATUS_ERROR(ret))) {
		qdf_frag_free((nbuf_frag_info_t->virt_addr).vaddr);
		dp_err("Frag map failed");
		DP_STATS_INC(dp_pdev, replenish.map_err, 1);
		return QDF_STATUS_E_FAULT;
	}

	return QDF_STATUS_SUCCESS;
}
#else
static inline QDF_STATUS
dp_pdev_frag_alloc_and_map(struct dp_soc *dp_soc,
			   struct dp_rx_nbuf_frag_info *nbuf_frag_info_t,
			   struct dp_pdev *dp_pdev,
			   struct rx_desc_pool *rx_desc_pool)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* DP_RX_MON_MEM_FRAG */

#ifdef WLAN_FEATURE_DP_RX_RING_HISTORY
/**
 * dp_rx_refill_ring_record_entry() - Record an entry into refill_ring history
 * @soc: Datapath soc structure
 * @ring_num: Refill ring number
 * @num_req: number of buffers requested for refill
 * @num_refill: number of buffers refilled
 *
 * Returns: None
 */
static inline void
dp_rx_refill_ring_record_entry(struct dp_soc *soc, uint8_t ring_num,
			       hal_ring_handle_t hal_ring_hdl,
			       uint32_t num_req, uint32_t num_refill)
{
	struct dp_refill_info_record *record;
	uint32_t idx;
	uint32_t tp;
	uint32_t hp;

	if (qdf_unlikely(ring_num >= MAX_PDEV_CNT ||
			 !soc->rx_refill_ring_history[ring_num]))
		return;

	idx = dp_history_get_next_index(&soc->rx_refill_ring_history[ring_num]->index,
					DP_RX_REFILL_HIST_MAX);

	/* No NULL check needed for record since its an array */
	record = &soc->rx_refill_ring_history[ring_num]->entry[idx];

	hal_get_sw_hptp(soc->hal_soc, hal_ring_hdl, &tp, &hp);
	record->timestamp = qdf_get_log_timestamp();
	record->num_req = num_req;
	record->num_refill = num_refill;
	record->hp = hp;
	record->tp = tp;
}
#else
static inline void
dp_rx_refill_ring_record_entry(struct dp_soc *soc, uint8_t ring_num,
			       hal_ring_handle_t hal_ring_hdl,
			       uint32_t num_req, uint32_t num_refill)
{
}
#endif

/**
 * dp_pdev_nbuf_alloc_and_map() - Allocate nbuf for desc buffer and map
 *
 * @dp_soc: struct dp_soc *
 * @mac_id: Mac id
 * @num_entries_avail: num_entries_avail
 * @nbuf_frag_info_t: nbuf frag info
 * @dp_pdev: struct dp_pdev *
 * @rx_desc_pool: Rx desc pool
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS
dp_pdev_nbuf_alloc_and_map_replenish(struct dp_soc *dp_soc,
				     uint32_t mac_id,
				     uint32_t num_entries_avail,
				     struct dp_rx_nbuf_frag_info *nbuf_frag_info_t,
				     struct dp_pdev *dp_pdev,
				     struct rx_desc_pool *rx_desc_pool)
{
	QDF_STATUS ret = QDF_STATUS_E_FAILURE;

	(nbuf_frag_info_t->virt_addr).nbuf =
		dp_rx_buffer_pool_nbuf_alloc(dp_soc,
					     mac_id,
					     rx_desc_pool,
					     num_entries_avail);
	if (!((nbuf_frag_info_t->virt_addr).nbuf)) {
		dp_err("nbuf alloc failed");
		DP_STATS_INC(dp_pdev, replenish.nbuf_alloc_fail, 1);
		return QDF_STATUS_E_NOMEM;
	}

	ret = dp_rx_buffer_pool_nbuf_map(dp_soc, rx_desc_pool,
					 nbuf_frag_info_t);
	if (qdf_unlikely(QDF_IS_STATUS_ERROR(ret))) {
		dp_rx_buffer_pool_nbuf_free(dp_soc,
			(nbuf_frag_info_t->virt_addr).nbuf, mac_id);
		dp_err("nbuf map failed");
		DP_STATS_INC(dp_pdev, replenish.map_err, 1);
		return QDF_STATUS_E_FAULT;
	}

	nbuf_frag_info_t->paddr =
		qdf_nbuf_get_frag_paddr((nbuf_frag_info_t->virt_addr).nbuf, 0);

	dp_ipa_handle_rx_buf_smmu_mapping(dp_soc,
			       (qdf_nbuf_t)((nbuf_frag_info_t->virt_addr).nbuf),
			       rx_desc_pool->buf_size,
			       true);

	ret = dp_check_paddr(dp_soc, &((nbuf_frag_info_t->virt_addr).nbuf),
			     &nbuf_frag_info_t->paddr,
			     rx_desc_pool);
	if (ret == QDF_STATUS_E_FAILURE) {
		DP_STATS_INC(dp_pdev, replenish.x86_fail, 1);
		return QDF_STATUS_E_ADDRNOTAVAIL;
	}

	return QDF_STATUS_SUCCESS;
}

/*
 * dp_rx_buffers_replenish() - replenish rxdma ring with rx nbufs
 *			       called during dp rx initialization
 *			       and at the end of dp_rx_process.
 *
 * @soc: core txrx main context
 * @mac_id: mac_id which is one of 3 mac_ids
 * @dp_rxdma_srng: dp rxdma circular ring
 * @rx_desc_pool: Pointer to free Rx descriptor pool
 * @num_req_buffers: number of buffer to be replenished
 * @desc_list: list of descs if called from dp_rx_process
 *	       or NULL during dp rx initialization or out of buffer
 *	       interrupt.
 * @tail: tail of descs list
 * @func_name: name of the caller function
 * Return: return success or failure
 */
QDF_STATUS __dp_rx_buffers_replenish(struct dp_soc *dp_soc, uint32_t mac_id,
				struct dp_srng *dp_rxdma_srng,
				struct rx_desc_pool *rx_desc_pool,
				uint32_t num_req_buffers,
				union dp_rx_desc_list_elem_t **desc_list,
				union dp_rx_desc_list_elem_t **tail,
				const char *func_name)
{
	uint32_t num_alloc_desc;
	uint16_t num_desc_to_free = 0;
	struct dp_pdev *dp_pdev = dp_get_pdev_for_lmac_id(dp_soc, mac_id);
	uint32_t num_entries_avail;
	uint32_t count;
	int sync_hw_ptr = 1;
	struct dp_rx_nbuf_frag_info nbuf_frag_info = {0};
	void *rxdma_ring_entry;
	union dp_rx_desc_list_elem_t *next;
	QDF_STATUS ret;
	void *rxdma_srng;

	rxdma_srng = dp_rxdma_srng->hal_srng;

	if (qdf_unlikely(!dp_pdev)) {
		dp_rx_err("%pK: pdev is null for mac_id = %d",
			  dp_soc, mac_id);
		return QDF_STATUS_E_FAILURE;
	}

	if (qdf_unlikely(!rxdma_srng)) {
		dp_rx_debug("%pK: rxdma srng not initialized", dp_soc);
		DP_STATS_INC(dp_pdev, replenish.rxdma_err, num_req_buffers);
		return QDF_STATUS_E_FAILURE;
	}

	dp_rx_debug("%pK: requested %d buffers for replenish",
		    dp_soc, num_req_buffers);

	hal_srng_access_start(dp_soc->hal_soc, rxdma_srng);

	num_entries_avail = hal_srng_src_num_avail(dp_soc->hal_soc,
						   rxdma_srng,
						   sync_hw_ptr);

	dp_rx_debug("%pK: no of available entries in rxdma ring: %d",
		    dp_soc, num_entries_avail);

	if (!(*desc_list) && (num_entries_avail >
		((dp_rxdma_srng->num_entries * 3) / 4))) {
		num_req_buffers = num_entries_avail;
	} else if (num_entries_avail < num_req_buffers) {
		num_desc_to_free = num_req_buffers - num_entries_avail;
		num_req_buffers = num_entries_avail;
	}

	if (qdf_unlikely(!num_req_buffers)) {
		num_desc_to_free = num_req_buffers;
		hal_srng_access_end(dp_soc->hal_soc, rxdma_srng);
		goto free_descs;
	}

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
			dp_rx_err("%pK: no free rx_descs in freelist", dp_soc);
			DP_STATS_INC(dp_pdev, err.desc_alloc_fail,
					num_req_buffers);
			hal_srng_access_end(dp_soc->hal_soc, rxdma_srng);
			return QDF_STATUS_E_NOMEM;
		}

		dp_rx_debug("%pK: %d rx desc allocated", dp_soc, num_alloc_desc);
		num_req_buffers = num_alloc_desc;
	}


	count = 0;

	while (count < num_req_buffers) {
		/* Flag is set while pdev rx_desc_pool initialization */
		if (qdf_unlikely(rx_desc_pool->rx_mon_dest_frag_enable))
			ret = dp_pdev_frag_alloc_and_map(dp_soc,
							 &nbuf_frag_info,
							 dp_pdev,
							 rx_desc_pool);
		else
			ret = dp_pdev_nbuf_alloc_and_map_replenish(dp_soc,
								   mac_id,
					num_entries_avail, &nbuf_frag_info,
					dp_pdev, rx_desc_pool);

		if (qdf_unlikely(QDF_IS_STATUS_ERROR(ret))) {
			if (qdf_unlikely(ret  == QDF_STATUS_E_FAULT))
				continue;
			break;
		}

		count++;

		rxdma_ring_entry = hal_srng_src_get_next(dp_soc->hal_soc,
							 rxdma_srng);
		qdf_assert_always(rxdma_ring_entry);

		next = (*desc_list)->next;

		/* Flag is set while pdev rx_desc_pool initialization */
		if (qdf_unlikely(rx_desc_pool->rx_mon_dest_frag_enable))
			dp_rx_desc_frag_prep(&((*desc_list)->rx_desc),
					     &nbuf_frag_info);
		else
			dp_rx_desc_prep(&((*desc_list)->rx_desc),
					&nbuf_frag_info);

		/* rx_desc.in_use should be zero at this time*/
		qdf_assert_always((*desc_list)->rx_desc.in_use == 0);

		(*desc_list)->rx_desc.in_use = 1;
		(*desc_list)->rx_desc.in_err_state = 0;
		dp_rx_desc_update_dbg_info(&(*desc_list)->rx_desc,
					   func_name, RX_DESC_REPLENISHED);
		dp_verbose_debug("rx_netbuf=%pK, paddr=0x%llx, cookie=%d",
				 nbuf_frag_info.virt_addr.nbuf,
				 (unsigned long long)(nbuf_frag_info.paddr),
				 (*desc_list)->rx_desc.cookie);

		hal_rxdma_buff_addr_info_set(dp_soc->hal_soc, rxdma_ring_entry,
					     nbuf_frag_info.paddr,
						(*desc_list)->rx_desc.cookie,
						rx_desc_pool->owner);

		*desc_list = next;

	}

	dp_rx_refill_ring_record_entry(dp_soc, dp_pdev->lmac_id, rxdma_srng,
				       num_req_buffers, count);

	hal_srng_access_end(dp_soc->hal_soc, rxdma_srng);

	dp_rx_schedule_refill_thread(dp_soc);

	dp_verbose_debug("replenished buffers %d, rx desc added back to free list %u",
			 count, num_desc_to_free);

	/* No need to count the number of bytes received during replenish.
	 * Therefore set replenish.pkts.bytes as 0.
	 */
	DP_STATS_INC_PKT(dp_pdev, replenish.pkts, count, 0);

free_descs:
	DP_STATS_INC(dp_pdev, buf_freelist, num_desc_to_free);
	/*
	 * add any available free desc back to the free list
	 */
	if (*desc_list)
		dp_rx_add_desc_list_to_free_list(dp_soc, desc_list, tail,
			mac_id, rx_desc_pool);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(__dp_rx_buffers_replenish);

/*
 * dp_rx_deliver_raw() - process RAW mode pkts and hand over the
 *				pkts to RAW mode simulation to
 *				decapsulate the pkt.
 *
 * @vdev: vdev on which RAW mode is enabled
 * @nbuf_list: list of RAW pkts to process
 * @peer: peer object from which the pkt is rx
 *
 * Return: void
 */
void
dp_rx_deliver_raw(struct dp_vdev *vdev, qdf_nbuf_t nbuf_list,
					struct dp_peer *peer)
{
	qdf_nbuf_t deliver_list_head = NULL;
	qdf_nbuf_t deliver_list_tail = NULL;
	qdf_nbuf_t nbuf;

	nbuf = nbuf_list;
	while (nbuf) {
		qdf_nbuf_t next = qdf_nbuf_next(nbuf);

		DP_RX_LIST_APPEND(deliver_list_head, deliver_list_tail, nbuf);

		DP_STATS_INC(vdev->pdev, rx_raw_pkts, 1);
		DP_STATS_INC_PKT(peer, rx.raw, 1, qdf_nbuf_len(nbuf));
		/*
		 * reset the chfrag_start and chfrag_end bits in nbuf cb
		 * as this is a non-amsdu pkt and RAW mode simulation expects
		 * these bit s to be 0 for non-amsdu pkt.
		 */
		if (qdf_nbuf_is_rx_chfrag_start(nbuf) &&
			 qdf_nbuf_is_rx_chfrag_end(nbuf)) {
			qdf_nbuf_set_rx_chfrag_start(nbuf, 0);
			qdf_nbuf_set_rx_chfrag_end(nbuf, 0);
		}

		nbuf = next;
	}

	vdev->osif_rsim_rx_decap(vdev->osif_vdev, &deliver_list_head,
				 &deliver_list_tail, peer->mac_addr.raw);

	vdev->osif_rx(vdev->osif_vdev, deliver_list_head);
}

#ifndef QCA_HOST_MODE_WIFI_DISABLED
#ifndef FEATURE_WDS
void dp_rx_da_learn(struct dp_soc *soc, uint8_t *rx_tlv_hdr,
		    struct dp_peer *ta_peer, qdf_nbuf_t nbuf)
{
}
#endif

/*
 * dp_rx_intrabss_mcbc_fwd() - Does intrabss forward for mcast packets
 *
 * @soc: core txrx main context
 * @ta_peer	: source peer entry
 * @rx_tlv_hdr	: start address of rx tlvs
 * @nbuf	: nbuf that has to be intrabss forwarded
 * @tid_stats	: tid stats pointer
 *
 * Return: bool: true if it is forwarded else false
 */
bool dp_rx_intrabss_mcbc_fwd(struct dp_soc *soc, struct dp_peer *ta_peer,
			     uint8_t *rx_tlv_hdr, qdf_nbuf_t nbuf,
			     struct cdp_tid_rx_stats *tid_stats)
{
	uint16_t len;
	qdf_nbuf_t nbuf_copy;

	if (dp_rx_intrabss_eapol_drop_check(soc, ta_peer, rx_tlv_hdr,
					    nbuf))
		return true;

	if (!dp_rx_check_ndi_mdns_fwding(ta_peer, nbuf))
		return false;

	/* If the source peer in the isolation list
	 * then dont forward instead push to bridge stack
	 */
	if (dp_get_peer_isolation(ta_peer))
		return false;

	nbuf_copy = qdf_nbuf_copy(nbuf);
	if (!nbuf_copy)
		return false;

	len = QDF_NBUF_CB_RX_PKT_LEN(nbuf);
	/* set TX notify flag 0 to avoid unnecessary TX comp callback */
	qdf_nbuf_tx_notify_comp_set(nbuf_copy, 0);
	if (dp_tx_send((struct cdp_soc_t *)soc,
		       ta_peer->vdev->vdev_id, nbuf_copy)) {
		DP_STATS_INC_PKT(ta_peer, rx.intra_bss.fail, 1, len);
		tid_stats->fail_cnt[INTRABSS_DROP]++;
		qdf_nbuf_free(nbuf_copy);
	} else {
		DP_STATS_INC_PKT(ta_peer, rx.intra_bss.pkts, 1, len);
		tid_stats->intrabss_cnt++;
	}
	return false;
}

/*
 * dp_rx_intrabss_ucast_fwd() - Does intrabss forward for unicast packets
 *
 * @soc: core txrx main context
 * @ta_peer: source peer entry
 * @tx_vdev_id: VDEV ID for Intra-BSS TX
 * @rx_tlv_hdr: start address of rx tlvs
 * @nbuf: nbuf that has to be intrabss forwarded
 * @tid_stats: tid stats pointer
 *
 * Return: bool: true if it is forwarded else false
 */
bool dp_rx_intrabss_ucast_fwd(struct dp_soc *soc, struct dp_peer *ta_peer,
			      uint8_t tx_vdev_id,
			      uint8_t *rx_tlv_hdr, qdf_nbuf_t nbuf,
			      struct cdp_tid_rx_stats *tid_stats)
{
	uint16_t len;

	if (dp_rx_intrabss_eapol_drop_check(soc, ta_peer, rx_tlv_hdr,
					    nbuf))
		return true;

	len = QDF_NBUF_CB_RX_PKT_LEN(nbuf);

	/* linearize the nbuf just before we send to
	 * dp_tx_send()
	 */
	if (qdf_unlikely(qdf_nbuf_is_frag(nbuf))) {
		if (qdf_nbuf_linearize(nbuf) == -ENOMEM)
			return false;

		nbuf = qdf_nbuf_unshare(nbuf);
		if (!nbuf) {
			DP_STATS_INC_PKT(ta_peer,
					 rx.intra_bss.fail, 1, len);
			/* return true even though the pkt is
			 * not forwarded. Basically skb_unshare
			 * failed and we want to continue with
			 * next nbuf.
			 */
			tid_stats->fail_cnt[INTRABSS_DROP]++;
			return false;
		}
	}

	if (!dp_tx_send((struct cdp_soc_t *)soc,
			tx_vdev_id, nbuf)) {
		DP_STATS_INC_PKT(ta_peer, rx.intra_bss.pkts, 1,
				 len);
	} else {
		DP_STATS_INC_PKT(ta_peer, rx.intra_bss.fail, 1,
				 len);
		tid_stats->fail_cnt[INTRABSS_DROP]++;
		return false;
	}

	return true;
}

#endif /* QCA_HOST_MODE_WIFI_DISABLED */

#ifdef MESH_MODE_SUPPORT

/**
 * dp_rx_fill_mesh_stats() - Fills the mesh per packet receive stats
 *
 * @vdev: DP Virtual device handle
 * @nbuf: Buffer pointer
 * @rx_tlv_hdr: start of rx tlv header
 * @peer: pointer to peer
 *
 * This function allocated memory for mesh receive stats and fill the
 * required stats. Stores the memory address in skb cb.
 *
 * Return: void
 */

void dp_rx_fill_mesh_stats(struct dp_vdev *vdev, qdf_nbuf_t nbuf,
				uint8_t *rx_tlv_hdr, struct dp_peer *peer)
{
	struct mesh_recv_hdr_s *rx_info = NULL;
	uint32_t pkt_type;
	uint32_t nss;
	uint32_t rate_mcs;
	uint32_t bw;
	uint8_t primary_chan_num;
	uint32_t center_chan_freq;
	struct dp_soc *soc = vdev->pdev->soc;

	/* fill recv mesh stats */
	rx_info = qdf_mem_malloc(sizeof(struct mesh_recv_hdr_s));

	/* upper layers are resposible to free this memory */

	if (!rx_info) {
		dp_rx_err("%pK: Memory allocation failed for mesh rx stats",
			  vdev->pdev->soc);
		DP_STATS_INC(vdev->pdev, mesh_mem_alloc, 1);
		return;
	}

	rx_info->rs_flags = MESH_RXHDR_VER1;
	if (qdf_nbuf_is_rx_chfrag_start(nbuf))
		rx_info->rs_flags |= MESH_RX_FIRST_MSDU;

	if (qdf_nbuf_is_rx_chfrag_end(nbuf))
		rx_info->rs_flags |= MESH_RX_LAST_MSDU;

	if (hal_rx_tlv_get_is_decrypted(soc->hal_soc, rx_tlv_hdr)) {
		rx_info->rs_flags |= MESH_RX_DECRYPTED;
		rx_info->rs_keyix = hal_rx_msdu_get_keyid(soc->hal_soc,
							  rx_tlv_hdr);
		if (vdev->osif_get_key)
			vdev->osif_get_key(vdev->osif_vdev,
					&rx_info->rs_decryptkey[0],
					&peer->mac_addr.raw[0],
					rx_info->rs_keyix);
	}

	rx_info->rs_snr = peer->stats.rx.snr;
	rx_info->rs_rssi = rx_info->rs_snr + DP_DEFAULT_NOISEFLOOR;

	soc = vdev->pdev->soc;
	primary_chan_num = hal_rx_tlv_get_freq(soc->hal_soc, rx_tlv_hdr);
	center_chan_freq = hal_rx_tlv_get_freq(soc->hal_soc, rx_tlv_hdr) >> 16;

	if (soc->cdp_soc.ol_ops && soc->cdp_soc.ol_ops->freq_to_band) {
		rx_info->rs_band = soc->cdp_soc.ol_ops->freq_to_band(
							soc->ctrl_psoc,
							vdev->pdev->pdev_id,
							center_chan_freq);
	}
	rx_info->rs_channel = primary_chan_num;
	pkt_type = hal_rx_tlv_get_pkt_type(soc->hal_soc, rx_tlv_hdr);
	rate_mcs = hal_rx_tlv_rate_mcs_get(soc->hal_soc, rx_tlv_hdr);
	bw = hal_rx_tlv_bw_get(soc->hal_soc, rx_tlv_hdr);
	nss = hal_rx_msdu_start_nss_get(soc->hal_soc, rx_tlv_hdr);
	rx_info->rs_ratephy1 = rate_mcs | (nss << 0x8) | (pkt_type << 16) |
				(bw << 24);

	qdf_nbuf_set_rx_fctx_type(nbuf, (void *)rx_info, CB_FTYPE_MESH_RX_INFO);

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_MED,
		FL("Mesh rx stats: flags %x, rssi %x, chn %x, rate %x, kix %x, snr %x"),
						rx_info->rs_flags,
						rx_info->rs_rssi,
						rx_info->rs_channel,
						rx_info->rs_ratephy1,
						rx_info->rs_keyix,
						rx_info->rs_snr);

}

/**
 * dp_rx_filter_mesh_packets() - Filters mesh unwanted packets
 *
 * @vdev: DP Virtual device handle
 * @nbuf: Buffer pointer
 * @rx_tlv_hdr: start of rx tlv header
 *
 * This checks if the received packet is matching any filter out
 * catogery and and drop the packet if it matches.
 *
 * Return: status(0 indicates drop, 1 indicate to no drop)
 */

QDF_STATUS dp_rx_filter_mesh_packets(struct dp_vdev *vdev, qdf_nbuf_t nbuf,
					uint8_t *rx_tlv_hdr)
{
	union dp_align_mac_addr mac_addr;
	struct dp_soc *soc = vdev->pdev->soc;

	if (qdf_unlikely(vdev->mesh_rx_filter)) {
		if (vdev->mesh_rx_filter & MESH_FILTER_OUT_FROMDS)
			if (hal_rx_mpdu_get_fr_ds(soc->hal_soc,
						  rx_tlv_hdr))
				return  QDF_STATUS_SUCCESS;

		if (vdev->mesh_rx_filter & MESH_FILTER_OUT_TODS)
			if (hal_rx_mpdu_get_to_ds(soc->hal_soc,
						  rx_tlv_hdr))
				return  QDF_STATUS_SUCCESS;

		if (vdev->mesh_rx_filter & MESH_FILTER_OUT_NODS)
			if (!hal_rx_mpdu_get_fr_ds(soc->hal_soc,
						   rx_tlv_hdr) &&
			    !hal_rx_mpdu_get_to_ds(soc->hal_soc,
						   rx_tlv_hdr))
				return  QDF_STATUS_SUCCESS;

		if (vdev->mesh_rx_filter & MESH_FILTER_OUT_RA) {
			if (hal_rx_mpdu_get_addr1(soc->hal_soc,
						  rx_tlv_hdr,
					&mac_addr.raw[0]))
				return QDF_STATUS_E_FAILURE;

			if (!qdf_mem_cmp(&mac_addr.raw[0],
					&vdev->mac_addr.raw[0],
					QDF_MAC_ADDR_SIZE))
				return  QDF_STATUS_SUCCESS;
		}

		if (vdev->mesh_rx_filter & MESH_FILTER_OUT_TA) {
			if (hal_rx_mpdu_get_addr2(soc->hal_soc,
						  rx_tlv_hdr,
						  &mac_addr.raw[0]))
				return QDF_STATUS_E_FAILURE;

			if (!qdf_mem_cmp(&mac_addr.raw[0],
					&vdev->mac_addr.raw[0],
					QDF_MAC_ADDR_SIZE))
				return  QDF_STATUS_SUCCESS;
		}
	}

	return QDF_STATUS_E_FAILURE;
}

#else
void dp_rx_fill_mesh_stats(struct dp_vdev *vdev, qdf_nbuf_t nbuf,
				uint8_t *rx_tlv_hdr, struct dp_peer *peer)
{
}

QDF_STATUS dp_rx_filter_mesh_packets(struct dp_vdev *vdev, qdf_nbuf_t nbuf,
					uint8_t *rx_tlv_hdr)
{
	return QDF_STATUS_E_FAILURE;
}

#endif

#ifdef FEATURE_NAC_RSSI
/**
 * dp_rx_process_invalid_peer(): Function to pass invalid peer list to umac
 * @soc: DP SOC handle
 * @mpdu: mpdu for which peer is invalid
 * @mac_id: mac_id which is one of 3 mac_ids(Assuming mac_id and
 * pool_id has same mapping)
 *
 * return: integer type
 */
uint8_t dp_rx_process_invalid_peer(struct dp_soc *soc, qdf_nbuf_t mpdu,
				   uint8_t mac_id)
{
	struct dp_invalid_peer_msg msg;
	struct dp_vdev *vdev = NULL;
	struct dp_pdev *pdev = NULL;
	struct ieee80211_frame *wh;
	qdf_nbuf_t curr_nbuf, next_nbuf;
	uint8_t *rx_tlv_hdr = qdf_nbuf_data(mpdu);
	uint8_t *rx_pkt_hdr = hal_rx_pkt_hdr_get(soc->hal_soc, rx_tlv_hdr);

	if (!HAL_IS_DECAP_FORMAT_RAW(soc->hal_soc, rx_tlv_hdr)) {
		dp_rx_debug("%pK: Drop decapped frames", soc);
		goto free;
	}

	wh = (struct ieee80211_frame *)rx_pkt_hdr;

	if (!DP_FRAME_IS_DATA(wh)) {
		dp_rx_debug("%pK: NAWDS valid only for data frames", soc);
		goto free;
	}

	if (qdf_nbuf_len(mpdu) < sizeof(struct ieee80211_frame)) {
		dp_rx_err("%pK: Invalid nbuf length", soc);
		goto free;
	}

	pdev = dp_get_pdev_for_lmac_id(soc, mac_id);

	if (!pdev || qdf_unlikely(pdev->is_pdev_down)) {
		dp_rx_err("%pK: PDEV %s", soc, !pdev ? "not found" : "down");
		goto free;
	}

	if (dp_monitor_filter_neighbour_peer(pdev, rx_pkt_hdr) ==
	    QDF_STATUS_SUCCESS)
		return 0;

	TAILQ_FOREACH(vdev, &pdev->vdev_list, vdev_list_elem) {
		if (qdf_mem_cmp(wh->i_addr1, vdev->mac_addr.raw,
				QDF_MAC_ADDR_SIZE) == 0) {
			goto out;
		}
	}

	if (!vdev) {
		dp_rx_err("%pK: VDEV not found", soc);
		goto free;
	}

out:
	msg.wh = wh;
	qdf_nbuf_pull_head(mpdu, soc->rx_pkt_tlv_size);
	msg.nbuf = mpdu;
	msg.vdev_id = vdev->vdev_id;

	/*
	 * NOTE: Only valid for HKv1.
	 * If smart monitor mode is enabled on RE, we are getting invalid
	 * peer frames with RA as STA mac of RE and the TA not matching
	 * with any NAC list or the the BSSID.Such frames need to dropped
	 * in order to avoid HM_WDS false addition.
	 */
	if (pdev->soc->cdp_soc.ol_ops->rx_invalid_peer) {
		if (dp_monitor_drop_inv_peer_pkts(vdev) == QDF_STATUS_SUCCESS) {
			dp_rx_warn("%pK: Drop inv peer pkts with STA RA:%pm",
				   soc, wh->i_addr1);
			goto free;
		}
		pdev->soc->cdp_soc.ol_ops->rx_invalid_peer(
				(struct cdp_ctrl_objmgr_psoc *)soc->ctrl_psoc,
				pdev->pdev_id, &msg);
	}

free:
	/* Drop and free packet */
	curr_nbuf = mpdu;
	while (curr_nbuf) {
		next_nbuf = qdf_nbuf_next(curr_nbuf);
		qdf_nbuf_free(curr_nbuf);
		curr_nbuf = next_nbuf;
	}

	return 0;
}

/**
 * dp_rx_process_invalid_peer_wrapper(): Function to wrap invalid peer handler
 * @soc: DP SOC handle
 * @mpdu: mpdu for which peer is invalid
 * @mpdu_done: if an mpdu is completed
 * @mac_id: mac_id which is one of 3 mac_ids(Assuming mac_id and
 * pool_id has same mapping)
 *
 * return: integer type
 */
void dp_rx_process_invalid_peer_wrapper(struct dp_soc *soc,
					qdf_nbuf_t mpdu, bool mpdu_done,
					uint8_t mac_id)
{
	/* Only trigger the process when mpdu is completed */
	if (mpdu_done)
		dp_rx_process_invalid_peer(soc, mpdu, mac_id);
}
#else
uint8_t dp_rx_process_invalid_peer(struct dp_soc *soc, qdf_nbuf_t mpdu,
				   uint8_t mac_id)
{
	qdf_nbuf_t curr_nbuf, next_nbuf;
	struct dp_pdev *pdev;
	struct dp_vdev *vdev = NULL;
	struct ieee80211_frame *wh;
	uint8_t *rx_tlv_hdr = qdf_nbuf_data(mpdu);
	uint8_t *rx_pkt_hdr = hal_rx_pkt_hdr_get(soc->hal_soc, rx_tlv_hdr);

	wh = (struct ieee80211_frame *)rx_pkt_hdr;

	if (!DP_FRAME_IS_DATA(wh)) {
		QDF_TRACE_ERROR_RL(QDF_MODULE_ID_DP,
				   "only for data frames");
		goto free;
	}

	if (qdf_nbuf_len(mpdu) < sizeof(struct ieee80211_frame)) {
		dp_rx_info_rl("%pK: Invalid nbuf length", soc);
		goto free;
	}

	pdev = dp_get_pdev_for_lmac_id(soc, mac_id);
	if (!pdev) {
		dp_rx_info_rl("%pK: PDEV not found", soc);
		goto free;
	}

	qdf_spin_lock_bh(&pdev->vdev_list_lock);
	DP_PDEV_ITERATE_VDEV_LIST(pdev, vdev) {
		if (qdf_mem_cmp(wh->i_addr1, vdev->mac_addr.raw,
				QDF_MAC_ADDR_SIZE) == 0) {
			qdf_spin_unlock_bh(&pdev->vdev_list_lock);
			goto out;
		}
	}
	qdf_spin_unlock_bh(&pdev->vdev_list_lock);

	if (!vdev) {
		dp_rx_info_rl("%pK: VDEV not found", soc);
		goto free;
	}

out:
	if (soc->cdp_soc.ol_ops->rx_invalid_peer)
		soc->cdp_soc.ol_ops->rx_invalid_peer(vdev->vdev_id, wh);
free:
	/* reset the head and tail pointers */
	pdev = dp_get_pdev_for_lmac_id(soc, mac_id);
	if (pdev) {
		pdev->invalid_peer_head_msdu = NULL;
		pdev->invalid_peer_tail_msdu = NULL;
	}

	/* Drop and free packet */
	curr_nbuf = mpdu;
	while (curr_nbuf) {
		next_nbuf = qdf_nbuf_next(curr_nbuf);
		qdf_nbuf_free(curr_nbuf);
		curr_nbuf = next_nbuf;
	}

	/* Reset the head and tail pointers */
	pdev = dp_get_pdev_for_lmac_id(soc, mac_id);
	if (pdev) {
		pdev->invalid_peer_head_msdu = NULL;
		pdev->invalid_peer_tail_msdu = NULL;
	}

	return 0;
}

void dp_rx_process_invalid_peer_wrapper(struct dp_soc *soc,
					qdf_nbuf_t mpdu, bool mpdu_done,
					uint8_t mac_id)
{
	/* Process the nbuf */
	dp_rx_process_invalid_peer(soc, mpdu, mac_id);
}
#endif

#ifndef QCA_HOST_MODE_WIFI_DISABLED

#ifdef RECEIVE_OFFLOAD
/**
 * dp_rx_print_offload_info() - Print offload info from RX TLV
 * @soc: dp soc handle
 * @msdu: MSDU for which the offload info is to be printed
 *
 * Return: None
 */
static void dp_rx_print_offload_info(struct dp_soc *soc,
				     qdf_nbuf_t msdu)
{
	dp_verbose_debug("----------------------RX DESC LRO/GRO----------------------");
	dp_verbose_debug("lro_eligible 0x%x",
			 QDF_NBUF_CB_RX_LRO_ELIGIBLE(msdu));
	dp_verbose_debug("pure_ack 0x%x", QDF_NBUF_CB_RX_TCP_PURE_ACK(msdu));
	dp_verbose_debug("chksum 0x%x", QDF_NBUF_CB_RX_TCP_CHKSUM(msdu));
	dp_verbose_debug("TCP seq num 0x%x", QDF_NBUF_CB_RX_TCP_SEQ_NUM(msdu));
	dp_verbose_debug("TCP ack num 0x%x", QDF_NBUF_CB_RX_TCP_ACK_NUM(msdu));
	dp_verbose_debug("TCP window 0x%x", QDF_NBUF_CB_RX_TCP_WIN(msdu));
	dp_verbose_debug("TCP protocol 0x%x", QDF_NBUF_CB_RX_TCP_PROTO(msdu));
	dp_verbose_debug("TCP offset 0x%x", QDF_NBUF_CB_RX_TCP_OFFSET(msdu));
	dp_verbose_debug("toeplitz 0x%x", QDF_NBUF_CB_RX_FLOW_ID(msdu));
	dp_verbose_debug("---------------------------------------------------------");
}

/**
 * dp_rx_fill_gro_info() - Fill GRO info from RX TLV into skb->cb
 * @soc: DP SOC handle
 * @rx_tlv: RX TLV received for the msdu
 * @msdu: msdu for which GRO info needs to be filled
 * @rx_ol_pkt_cnt: counter to be incremented for GRO eligible packets
 *
 * Return: None
 */
void dp_rx_fill_gro_info(struct dp_soc *soc, uint8_t *rx_tlv,
			 qdf_nbuf_t msdu, uint32_t *rx_ol_pkt_cnt)
{
	struct hal_offload_info offload_info;

	if (!wlan_cfg_is_gro_enabled(soc->wlan_cfg_ctx))
		return;

	if (hal_rx_tlv_get_offload_info(soc->hal_soc, rx_tlv, &offload_info))
		return;

	*rx_ol_pkt_cnt = *rx_ol_pkt_cnt + 1;

	QDF_NBUF_CB_RX_LRO_ELIGIBLE(msdu) = offload_info.lro_eligible;
	QDF_NBUF_CB_RX_TCP_PURE_ACK(msdu) = offload_info.tcp_pure_ack;
	QDF_NBUF_CB_RX_TCP_CHKSUM(msdu) =
			hal_rx_tlv_get_tcp_chksum(soc->hal_soc,
						  rx_tlv);
	QDF_NBUF_CB_RX_TCP_SEQ_NUM(msdu) = offload_info.tcp_seq_num;
	QDF_NBUF_CB_RX_TCP_ACK_NUM(msdu) = offload_info.tcp_ack_num;
	QDF_NBUF_CB_RX_TCP_WIN(msdu) = offload_info.tcp_win;
	QDF_NBUF_CB_RX_TCP_PROTO(msdu) = offload_info.tcp_proto;
	QDF_NBUF_CB_RX_IPV6_PROTO(msdu) = offload_info.ipv6_proto;
	QDF_NBUF_CB_RX_TCP_OFFSET(msdu) = offload_info.tcp_offset;
	QDF_NBUF_CB_RX_FLOW_ID(msdu) = offload_info.flow_id;

	dp_rx_print_offload_info(soc, msdu);
}
#endif /* RECEIVE_OFFLOAD */

/**
 * dp_rx_adjust_nbuf_len() - set appropriate msdu length in nbuf.
 *
 * @soc: DP soc handle
 * @nbuf: pointer to msdu.
 * @mpdu_len: mpdu length
 * @l3_pad_len: L3 padding length by HW
 *
 * Return: returns true if nbuf is last msdu of mpdu else retuns false.
 */
static inline bool dp_rx_adjust_nbuf_len(struct dp_soc *soc,
					 qdf_nbuf_t nbuf,
					 uint16_t *mpdu_len,
					 uint32_t l3_pad_len)
{
	bool last_nbuf;
	uint32_t pkt_hdr_size;

	pkt_hdr_size = soc->rx_pkt_tlv_size + l3_pad_len;

	if ((*mpdu_len + pkt_hdr_size) > RX_DATA_BUFFER_SIZE) {
		qdf_nbuf_set_pktlen(nbuf, RX_DATA_BUFFER_SIZE);
		last_nbuf = false;
		*mpdu_len -= (RX_DATA_BUFFER_SIZE - pkt_hdr_size);
	} else {
		qdf_nbuf_set_pktlen(nbuf, (*mpdu_len + pkt_hdr_size));
		last_nbuf = true;
		*mpdu_len = 0;
	}

	return last_nbuf;
}

/**
 * dp_get_l3_hdr_pad_len() - get L3 header padding length.
 *
 * @soc: DP soc handle
 * @nbuf: pointer to msdu.
 *
 * Return: returns padding length in bytes.
 */
static inline uint32_t dp_get_l3_hdr_pad_len(struct dp_soc *soc,
					     qdf_nbuf_t nbuf)
{
	uint32_t l3_hdr_pad = 0;
	uint8_t *rx_tlv_hdr;
	struct hal_rx_msdu_metadata msdu_metadata;

	while (nbuf) {
		if (!qdf_nbuf_is_rx_chfrag_cont(nbuf)) {
			/* scattered msdu end with continuation is 0 */
			rx_tlv_hdr = qdf_nbuf_data(nbuf);
			hal_rx_msdu_metadata_get(soc->hal_soc,
						 rx_tlv_hdr,
						 &msdu_metadata);
			l3_hdr_pad = msdu_metadata.l3_hdr_pad;
			break;
		}
		nbuf = nbuf->next;
	}

	return l3_hdr_pad;
}

/**
 * dp_rx_sg_create() - create a frag_list for MSDUs which are spread across
 *		     multiple nbufs.
 * @soc: DP SOC handle
 * @nbuf: pointer to the first msdu of an amsdu.
 *
 * This function implements the creation of RX frag_list for cases
 * where an MSDU is spread across multiple nbufs.
 *
 * Return: returns the head nbuf which contains complete frag_list.
 */
qdf_nbuf_t dp_rx_sg_create(struct dp_soc *soc, qdf_nbuf_t nbuf)
{
	qdf_nbuf_t parent, frag_list, next = NULL;
	uint16_t frag_list_len = 0;
	uint16_t mpdu_len;
	bool last_nbuf;
	uint32_t l3_hdr_pad_offset = 0;

	/*
	 * Use msdu len got from REO entry descriptor instead since
	 * there is case the RX PKT TLV is corrupted while msdu_len
	 * from REO descriptor is right for non-raw RX scatter msdu.
	 */
	mpdu_len = QDF_NBUF_CB_RX_PKT_LEN(nbuf);

	/*
	 * this is a case where the complete msdu fits in one single nbuf.
	 * in this case HW sets both start and end bit and we only need to
	 * reset these bits for RAW mode simulator to decap the pkt
	 */
	if (qdf_nbuf_is_rx_chfrag_start(nbuf) &&
					qdf_nbuf_is_rx_chfrag_end(nbuf)) {
		qdf_nbuf_set_pktlen(nbuf, mpdu_len + soc->rx_pkt_tlv_size);
		qdf_nbuf_pull_head(nbuf, soc->rx_pkt_tlv_size);
		return nbuf;
	}

	l3_hdr_pad_offset = dp_get_l3_hdr_pad_len(soc, nbuf);
	/*
	 * This is a case where we have multiple msdus (A-MSDU) spread across
	 * multiple nbufs. here we create a fraglist out of these nbufs.
	 *
	 * the moment we encounter a nbuf with continuation bit set we
	 * know for sure we have an MSDU which is spread across multiple
	 * nbufs. We loop through and reap nbufs till we reach last nbuf.
	 */
	parent = nbuf;
	frag_list = nbuf->next;
	nbuf = nbuf->next;

	/*
	 * set the start bit in the first nbuf we encounter with continuation
	 * bit set. This has the proper mpdu length set as it is the first
	 * msdu of the mpdu. this becomes the parent nbuf and the subsequent
	 * nbufs will form the frag_list of the parent nbuf.
	 */
	qdf_nbuf_set_rx_chfrag_start(parent, 1);
	/*
	 * L3 header padding is only needed for the 1st buffer
	 * in a scattered msdu
	 */
	last_nbuf = dp_rx_adjust_nbuf_len(soc, parent, &mpdu_len,
					  l3_hdr_pad_offset);

	/*
	 * MSDU cont bit is set but reported MPDU length can fit
	 * in to single buffer
	 *
	 * Increment error stats and avoid SG list creation
	 */
	if (last_nbuf) {
		DP_STATS_INC(soc, rx.err.msdu_continuation_err, 1);
		qdf_nbuf_pull_head(parent,
				   soc->rx_pkt_tlv_size + l3_hdr_pad_offset);
		return parent;
	}

	/*
	 * this is where we set the length of the fragments which are
	 * associated to the parent nbuf. We iterate through the frag_list
	 * till we hit the last_nbuf of the list.
	 */
	do {
		last_nbuf = dp_rx_adjust_nbuf_len(soc, nbuf, &mpdu_len, 0);
		qdf_nbuf_pull_head(nbuf,
				   soc->rx_pkt_tlv_size);
		frag_list_len += qdf_nbuf_len(nbuf);

		if (last_nbuf) {
			next = nbuf->next;
			nbuf->next = NULL;
			break;
		}

		nbuf = nbuf->next;
	} while (!last_nbuf);

	qdf_nbuf_set_rx_chfrag_start(nbuf, 0);
	qdf_nbuf_append_ext_list(parent, frag_list, frag_list_len);
	parent->next = next;

	qdf_nbuf_pull_head(parent,
			   soc->rx_pkt_tlv_size + l3_hdr_pad_offset);
	return parent;
}

#endif /* QCA_HOST_MODE_WIFI_DISABLED */

#ifdef QCA_PEER_EXT_STATS
/*
 * dp_rx_compute_tid_delay - Computer per TID delay stats
 * @peer: DP soc context
 * @nbuf: NBuffer
 *
 * Return: Void
 */
void dp_rx_compute_tid_delay(struct cdp_delay_tid_stats *stats,
			     qdf_nbuf_t nbuf)
{
	struct cdp_delay_rx_stats  *rx_delay = &stats->rx_delay;
	uint32_t to_stack = qdf_nbuf_get_timedelta_ms(nbuf);

	dp_hist_update_stats(&rx_delay->to_stack_delay, to_stack);
}
#endif /* QCA_PEER_EXT_STATS */

/**
 * dp_rx_compute_delay() - Compute and fill in all timestamps
 *				to pass in correct fields
 *
 * @vdev: pdev handle
 * @tx_desc: tx descriptor
 * @tid: tid value
 * Return: none
 */
void dp_rx_compute_delay(struct dp_vdev *vdev, qdf_nbuf_t nbuf)
{
	uint8_t ring_id = QDF_NBUF_CB_RX_CTX_ID(nbuf);
	int64_t current_ts = qdf_ktime_to_ms(qdf_ktime_get());
	uint32_t to_stack = qdf_nbuf_get_timedelta_ms(nbuf);
	uint8_t tid = qdf_nbuf_get_tid_val(nbuf);
	uint32_t interframe_delay =
		(uint32_t)(current_ts - vdev->prev_rx_deliver_tstamp);
	struct cdp_tid_rx_stats *rstats =
		&vdev->pdev->stats.tid_stats.tid_rx_stats[ring_id][tid];

	dp_update_delay_stats(NULL, rstats, to_stack, tid,
			      CDP_DELAY_STATS_REAP_STACK, ring_id);
	/*
	 * Update interframe delay stats calculated at deliver_data_ol point.
	 * Value of vdev->prev_rx_deliver_tstamp will be 0 for 1st frame, so
	 * interframe delay will not be calculate correctly for 1st frame.
	 * On the other side, this will help in avoiding extra per packet check
	 * of vdev->prev_rx_deliver_tstamp.
	 */
	dp_update_delay_stats(NULL, rstats, interframe_delay, tid,
			      CDP_DELAY_STATS_RX_INTERFRAME, ring_id);
	vdev->prev_rx_deliver_tstamp = current_ts;
}

/**
 * dp_rx_drop_nbuf_list() - drop an nbuf list
 * @pdev: dp pdev reference
 * @buf_list: buffer list to be dropepd
 *
 * Return: int (number of bufs dropped)
 */
static inline int dp_rx_drop_nbuf_list(struct dp_pdev *pdev,
				       qdf_nbuf_t buf_list)
{
	struct cdp_tid_rx_stats *stats = NULL;
	uint8_t tid = 0, ring_id = 0;
	int num_dropped = 0;
	qdf_nbuf_t buf, next_buf;

	buf = buf_list;
	while (buf) {
		ring_id = QDF_NBUF_CB_RX_CTX_ID(buf);
		next_buf = qdf_nbuf_queue_next(buf);
		tid = qdf_nbuf_get_tid_val(buf);
		if (qdf_likely(pdev)) {
			stats = &pdev->stats.tid_stats.tid_rx_stats[ring_id][tid];
			stats->fail_cnt[INVALID_PEER_VDEV]++;
			stats->delivered_to_stack--;
		}
		qdf_nbuf_free(buf);
		buf = next_buf;
		num_dropped++;
	}

	return num_dropped;
}

#ifdef QCA_SUPPORT_WDS_EXTENDED
/**
 * dp_rx_deliver_to_stack_ext() - Deliver to netdev per sta
 * @soc: core txrx main context
 * @vdev: vdev
 * @peer: peer
 * @nbuf_head: skb list head
 *
 * Return: true if packet is delivered to netdev per STA.
 */
static inline bool
dp_rx_deliver_to_stack_ext(struct dp_soc *soc, struct dp_vdev *vdev,
			   struct dp_peer *peer, qdf_nbuf_t nbuf_head)
{
	/*
	 * When extended WDS is disabled, frames are sent to AP netdevice.
	 */
	if (qdf_likely(!vdev->wds_ext_enabled))
		return false;

	/*
	 * There can be 2 cases:
	 * 1. Send frame to parent netdev if its not for netdev per STA
	 * 2. If frame is meant for netdev per STA:
	 *    a. Send frame to appropriate netdev using registered fp.
	 *    b. If fp is NULL, drop the frames.
	 */
	if (!peer->wds_ext.init)
		return false;

	if (peer->osif_rx)
		peer->osif_rx(peer->wds_ext.osif_peer, nbuf_head);
	else
		dp_rx_drop_nbuf_list(vdev->pdev, nbuf_head);

	return true;
}

#else
static inline bool
dp_rx_deliver_to_stack_ext(struct dp_soc *soc, struct dp_vdev *vdev,
			   struct dp_peer *peer, qdf_nbuf_t nbuf_head)
{
	return false;
}
#endif

#ifdef PEER_CACHE_RX_PKTS
/**
 * dp_rx_flush_rx_cached() - flush cached rx frames
 * @peer: peer
 * @drop: flag to drop frames or forward to net stack
 *
 * Return: None
 */
void dp_rx_flush_rx_cached(struct dp_peer *peer, bool drop)
{
	struct dp_peer_cached_bufq *bufqi;
	struct dp_rx_cached_buf *cache_buf = NULL;
	ol_txrx_rx_fp data_rx = NULL;
	int num_buff_elem;
	QDF_STATUS status;

	if (qdf_atomic_inc_return(&peer->flush_in_progress) > 1) {
		qdf_atomic_dec(&peer->flush_in_progress);
		return;
	}

	qdf_spin_lock_bh(&peer->peer_info_lock);
	if (peer->state >= OL_TXRX_PEER_STATE_CONN && peer->vdev->osif_rx)
		data_rx = peer->vdev->osif_rx;
	else
		drop = true;
	qdf_spin_unlock_bh(&peer->peer_info_lock);

	bufqi = &peer->bufq_info;

	qdf_spin_lock_bh(&bufqi->bufq_lock);
	qdf_list_remove_front(&bufqi->cached_bufq,
			      (qdf_list_node_t **)&cache_buf);
	while (cache_buf) {
		num_buff_elem = QDF_NBUF_CB_RX_NUM_ELEMENTS_IN_LIST(
								cache_buf->buf);
		bufqi->entries -= num_buff_elem;
		qdf_spin_unlock_bh(&bufqi->bufq_lock);
		if (drop) {
			bufqi->dropped = dp_rx_drop_nbuf_list(peer->vdev->pdev,
							      cache_buf->buf);
		} else {
			/* Flush the cached frames to OSIF DEV */
			status = data_rx(peer->vdev->osif_vdev, cache_buf->buf);
			if (status != QDF_STATUS_SUCCESS)
				bufqi->dropped = dp_rx_drop_nbuf_list(
							peer->vdev->pdev,
							cache_buf->buf);
		}
		qdf_mem_free(cache_buf);
		cache_buf = NULL;
		qdf_spin_lock_bh(&bufqi->bufq_lock);
		qdf_list_remove_front(&bufqi->cached_bufq,
				      (qdf_list_node_t **)&cache_buf);
	}
	qdf_spin_unlock_bh(&bufqi->bufq_lock);
	qdf_atomic_dec(&peer->flush_in_progress);
}

/**
 * dp_rx_enqueue_rx() - cache rx frames
 * @peer: peer
 * @rx_buf_list: cache buffer list
 *
 * Return: None
 */
static QDF_STATUS
dp_rx_enqueue_rx(struct dp_peer *peer, qdf_nbuf_t rx_buf_list)
{
	struct dp_rx_cached_buf *cache_buf;
	struct dp_peer_cached_bufq *bufqi = &peer->bufq_info;
	int num_buff_elem;

	dp_debug_rl("bufq->curr %d bufq->drops %d", bufqi->entries,
		    bufqi->dropped);
	if (!peer->valid) {
		bufqi->dropped = dp_rx_drop_nbuf_list(peer->vdev->pdev,
						      rx_buf_list);
		return QDF_STATUS_E_INVAL;
	}

	qdf_spin_lock_bh(&bufqi->bufq_lock);
	if (bufqi->entries >= bufqi->thresh) {
		bufqi->dropped = dp_rx_drop_nbuf_list(peer->vdev->pdev,
						      rx_buf_list);
		qdf_spin_unlock_bh(&bufqi->bufq_lock);
		return QDF_STATUS_E_RESOURCES;
	}
	qdf_spin_unlock_bh(&bufqi->bufq_lock);

	num_buff_elem = QDF_NBUF_CB_RX_NUM_ELEMENTS_IN_LIST(rx_buf_list);

	cache_buf = qdf_mem_malloc_atomic(sizeof(*cache_buf));
	if (!cache_buf) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "Failed to allocate buf to cache rx frames");
		bufqi->dropped = dp_rx_drop_nbuf_list(peer->vdev->pdev,
						      rx_buf_list);
		return QDF_STATUS_E_NOMEM;
	}

	cache_buf->buf = rx_buf_list;

	qdf_spin_lock_bh(&bufqi->bufq_lock);
	qdf_list_insert_back(&bufqi->cached_bufq,
			     &cache_buf->node);
	bufqi->entries += num_buff_elem;
	qdf_spin_unlock_bh(&bufqi->bufq_lock);

	return QDF_STATUS_SUCCESS;
}

static inline
bool dp_rx_is_peer_cache_bufq_supported(void)
{
	return true;
}
#else
static inline
bool dp_rx_is_peer_cache_bufq_supported(void)
{
	return false;
}

static inline QDF_STATUS
dp_rx_enqueue_rx(struct dp_peer *peer, qdf_nbuf_t rx_buf_list)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#ifndef DELIVERY_TO_STACK_STATUS_CHECK
/**
 * dp_rx_check_delivery_to_stack() - Deliver pkts to network
 * using the appropriate call back functions.
 * @soc: soc
 * @vdev: vdev
 * @peer: peer
 * @nbuf_head: skb list head
 * @nbuf_tail: skb list tail
 *
 * Return: None
 */
static void dp_rx_check_delivery_to_stack(struct dp_soc *soc,
					  struct dp_vdev *vdev,
					  struct dp_peer *peer,
					  qdf_nbuf_t nbuf_head)
{
	if (qdf_unlikely(dp_rx_deliver_to_stack_ext(soc, vdev,
						    peer, nbuf_head)))
		return;

	/* Function pointer initialized only when FISA is enabled */
	if (vdev->osif_fisa_rx)
		/* on failure send it via regular path */
		vdev->osif_fisa_rx(soc, vdev, nbuf_head);
	else
		vdev->osif_rx(vdev->osif_vdev, nbuf_head);
}

#else
/**
 * dp_rx_check_delivery_to_stack() - Deliver pkts to network
 * using the appropriate call back functions.
 * @soc: soc
 * @vdev: vdev
 * @peer: peer
 * @nbuf_head: skb list head
 * @nbuf_tail: skb list tail
 *
 * Check the return status of the call back function and drop
 * the packets if the return status indicates a failure.
 *
 * Return: None
 */
static void dp_rx_check_delivery_to_stack(struct dp_soc *soc,
					  struct dp_vdev *vdev,
					  struct dp_peer *peer,
					  qdf_nbuf_t nbuf_head)
{
	int num_nbuf = 0;
	QDF_STATUS ret_val = QDF_STATUS_E_FAILURE;

	/* Function pointer initialized only when FISA is enabled */
	if (vdev->osif_fisa_rx)
		/* on failure send it via regular path */
		ret_val = vdev->osif_fisa_rx(soc, vdev, nbuf_head);
	else if (vdev->osif_rx)
		ret_val = vdev->osif_rx(vdev->osif_vdev, nbuf_head);

	if (!QDF_IS_STATUS_SUCCESS(ret_val)) {
		num_nbuf = dp_rx_drop_nbuf_list(vdev->pdev, nbuf_head);
		DP_STATS_INC(soc, rx.err.rejected, num_nbuf);
		if (peer)
			DP_STATS_DEC(peer, rx.to_stack.num, num_nbuf);
	}
}
#endif /* ifdef DELIVERY_TO_STACK_STATUS_CHECK */

/*
 * dp_rx_validate_rx_callbacks() - validate rx callbacks
 * @soc DP soc
 * @vdev: DP vdev handle
 * @peer: pointer to the peer object
 * nbuf_head: skb list head
 *
 * Return: QDF_STATUS - QDF_STATUS_SUCCESS
 *			QDF_STATUS_E_FAILURE
 */
static inline QDF_STATUS
dp_rx_validate_rx_callbacks(struct dp_soc *soc,
			    struct dp_vdev *vdev,
			    struct dp_peer *peer,
			    qdf_nbuf_t nbuf_head)
{
	int num_nbuf;

	if (qdf_unlikely(!vdev || vdev->delete.pending)) {
		num_nbuf = dp_rx_drop_nbuf_list(NULL, nbuf_head);
		/*
		 * This is a special case where vdev is invalid,
		 * so we cannot know the pdev to which this packet
		 * belonged. Hence we update the soc rx error stats.
		 */
		DP_STATS_INC(soc, rx.err.invalid_vdev, num_nbuf);
		return QDF_STATUS_E_FAILURE;
	}

	/*
	 * highly unlikely to have a vdev without a registered rx
	 * callback function. if so let us free the nbuf_list.
	 */
	if (qdf_unlikely(!vdev->osif_rx)) {
		if (peer && dp_rx_is_peer_cache_bufq_supported()) {
			dp_rx_enqueue_rx(peer, nbuf_head);
		} else {
			num_nbuf = dp_rx_drop_nbuf_list(vdev->pdev,
							nbuf_head);
			DP_PEER_TO_STACK_DECC(peer, num_nbuf,
					      vdev->pdev->enhanced_stats_en);
		}
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dp_rx_deliver_to_stack(struct dp_soc *soc,
				  struct dp_vdev *vdev,
				  struct dp_peer *peer,
				  qdf_nbuf_t nbuf_head,
				  qdf_nbuf_t nbuf_tail)
{
	if (dp_rx_validate_rx_callbacks(soc, vdev, peer, nbuf_head) !=
					QDF_STATUS_SUCCESS)
		return QDF_STATUS_E_FAILURE;

	if (qdf_unlikely(vdev->rx_decap_type == htt_cmn_pkt_type_raw) ||
			(vdev->rx_decap_type == htt_cmn_pkt_type_native_wifi)) {
		vdev->osif_rsim_rx_decap(vdev->osif_vdev, &nbuf_head,
				&nbuf_tail, peer->mac_addr.raw);
	}

	dp_rx_check_delivery_to_stack(soc, vdev, peer, nbuf_head);

	return QDF_STATUS_SUCCESS;
}

#ifdef QCA_SUPPORT_EAPOL_OVER_CONTROL_PORT
QDF_STATUS dp_rx_eapol_deliver_to_stack(struct dp_soc *soc,
					struct dp_vdev *vdev,
					struct dp_peer *peer,
					qdf_nbuf_t nbuf_head,
					qdf_nbuf_t nbuf_tail)
{
	if (dp_rx_validate_rx_callbacks(soc, vdev, peer, nbuf_head) !=
					QDF_STATUS_SUCCESS)
		return QDF_STATUS_E_FAILURE;

	vdev->osif_rx_eapol(vdev->osif_vdev, nbuf_head);

	return QDF_STATUS_SUCCESS;
}
#endif

#ifndef QCA_HOST_MODE_WIFI_DISABLED
#ifdef VDEV_PEER_PROTOCOL_COUNT
#define dp_rx_msdu_stats_update_prot_cnts(vdev_hdl, nbuf, peer) \
{ \
	qdf_nbuf_t nbuf_local; \
	struct dp_peer *peer_local; \
	struct dp_vdev *vdev_local = vdev_hdl; \
	do { \
		if (qdf_likely(!((vdev_local)->peer_protocol_count_track))) \
			break; \
		nbuf_local = nbuf; \
		peer_local = peer; \
		if (qdf_unlikely(qdf_nbuf_is_frag((nbuf_local)))) \
			break; \
		else if (qdf_unlikely(qdf_nbuf_is_raw_frame((nbuf_local)))) \
			break; \
		dp_vdev_peer_stats_update_protocol_cnt((vdev_local), \
						       (nbuf_local), \
						       (peer_local), 0, 1); \
	} while (0); \
}
#else
#define dp_rx_msdu_stats_update_prot_cnts(vdev_hdl, nbuf, peer)
#endif

/**
 * dp_rx_msdu_stats_update() - update per msdu stats.
 * @soc: core txrx main context
 * @nbuf: pointer to the first msdu of an amsdu.
 * @rx_tlv_hdr: pointer to the start of RX TLV headers.
 * @peer: pointer to the peer object.
 * @ring_id: reo dest ring number on which pkt is reaped.
 * @tid_stats: per tid rx stats.
 *
 * update all the per msdu stats for that nbuf.
 * Return: void
 */
void dp_rx_msdu_stats_update(struct dp_soc *soc, qdf_nbuf_t nbuf,
			     uint8_t *rx_tlv_hdr, struct dp_peer *peer,
			     uint8_t ring_id,
			     struct cdp_tid_rx_stats *tid_stats)
{
	bool is_ampdu, is_not_amsdu;
	uint32_t sgi, mcs, tid, nss, bw, reception_type, pkt_type;
	struct dp_vdev *vdev = peer->vdev;
	bool enh_flag;
	qdf_ether_header_t *eh;
	uint16_t msdu_len = QDF_NBUF_CB_RX_PKT_LEN(nbuf);

	dp_rx_msdu_stats_update_prot_cnts(vdev, nbuf, peer);
	is_not_amsdu = qdf_nbuf_is_rx_chfrag_start(nbuf) &
			qdf_nbuf_is_rx_chfrag_end(nbuf);

	DP_STATS_INC_PKT(peer, rx.rcvd_reo[ring_id], 1, msdu_len);
	DP_STATS_INCC(peer, rx.non_amsdu_cnt, 1, is_not_amsdu);
	DP_STATS_INCC(peer, rx.amsdu_cnt, 1, !is_not_amsdu);
	DP_STATS_INCC(peer, rx.rx_retries, 1, qdf_nbuf_is_rx_retry_flag(nbuf));

	tid_stats->msdu_cnt++;
	if (qdf_unlikely(qdf_nbuf_is_da_mcbc(nbuf) &&
			 (vdev->rx_decap_type == htt_cmn_pkt_type_ethernet))) {
		eh = (qdf_ether_header_t *)qdf_nbuf_data(nbuf);
		enh_flag = vdev->pdev->enhanced_stats_en;
		DP_PEER_MC_INCC_PKT(peer, 1, msdu_len, enh_flag);
		tid_stats->mcast_msdu_cnt++;
		if (QDF_IS_ADDR_BROADCAST(eh->ether_dhost)) {
			DP_PEER_BC_INCC_PKT(peer, 1, msdu_len, enh_flag);
			tid_stats->bcast_msdu_cnt++;
		}
	}

	/*
	 * currently we can return from here as we have similar stats
	 * updated at per ppdu level instead of msdu level
	 */
	if (!soc->process_rx_status)
		return;

	peer->stats.rx.last_rx_ts = qdf_system_ticks();

	/*
	 * TODO - For KIWI this field is present in ring_desc
	 * Try to use ring desc instead of tlv.
	 */
	is_ampdu = hal_rx_mpdu_info_ampdu_flag_get(soc->hal_soc, rx_tlv_hdr);
	DP_STATS_INCC(peer, rx.ampdu_cnt, 1, is_ampdu);
	DP_STATS_INCC(peer, rx.non_ampdu_cnt, 1, !(is_ampdu));

	sgi = hal_rx_tlv_sgi_get(soc->hal_soc, rx_tlv_hdr);
	mcs = hal_rx_tlv_rate_mcs_get(soc->hal_soc, rx_tlv_hdr);
	tid = qdf_nbuf_get_tid_val(nbuf);
	bw = hal_rx_tlv_bw_get(soc->hal_soc, rx_tlv_hdr);
	reception_type = hal_rx_msdu_start_reception_type_get(soc->hal_soc,
							      rx_tlv_hdr);
	nss = hal_rx_msdu_start_nss_get(soc->hal_soc, rx_tlv_hdr);
	pkt_type = hal_rx_tlv_get_pkt_type(soc->hal_soc, rx_tlv_hdr);

	DP_STATS_INCC(peer, rx.rx_mpdu_cnt[mcs], 1,
		      ((mcs < MAX_MCS) && QDF_NBUF_CB_RX_CHFRAG_START(nbuf)));
	DP_STATS_INCC(peer, rx.rx_mpdu_cnt[MAX_MCS - 1], 1,
		      ((mcs >= MAX_MCS) && QDF_NBUF_CB_RX_CHFRAG_START(nbuf)));
	DP_STATS_INC(peer, rx.bw[bw], 1);
	/*
	 * only if nss > 0 and pkt_type is 11N/AC/AX,
	 * then increase index [nss - 1] in array counter.
	 */
	if (nss > 0 && (pkt_type == DOT11_N ||
			pkt_type == DOT11_AC ||
			pkt_type == DOT11_AX))
		DP_STATS_INC(peer, rx.nss[nss - 1], 1);

	DP_STATS_INC(peer, rx.sgi_count[sgi], 1);
	DP_STATS_INCC(peer, rx.err.mic_err, 1,
		      hal_rx_tlv_mic_err_get(soc->hal_soc, rx_tlv_hdr));
	DP_STATS_INCC(peer, rx.err.decrypt_err, 1,
		      hal_rx_tlv_decrypt_err_get(soc->hal_soc, rx_tlv_hdr));

	DP_STATS_INC(peer, rx.wme_ac_type[TID_TO_WME_AC(tid)], 1);
	DP_STATS_INC(peer, rx.reception_type[reception_type], 1);

	DP_STATS_INCC(peer, rx.pkt_type[pkt_type].mcs_count[MAX_MCS - 1], 1,
		      ((mcs >= MAX_MCS_11A) && (pkt_type == DOT11_A)));
	DP_STATS_INCC(peer, rx.pkt_type[pkt_type].mcs_count[mcs], 1,
		      ((mcs <= MAX_MCS_11A) && (pkt_type == DOT11_A)));
	DP_STATS_INCC(peer, rx.pkt_type[pkt_type].mcs_count[MAX_MCS - 1], 1,
		      ((mcs >= MAX_MCS_11B) && (pkt_type == DOT11_B)));
	DP_STATS_INCC(peer, rx.pkt_type[pkt_type].mcs_count[mcs], 1,
		      ((mcs <= MAX_MCS_11B) && (pkt_type == DOT11_B)));
	DP_STATS_INCC(peer, rx.pkt_type[pkt_type].mcs_count[MAX_MCS - 1], 1,
		      ((mcs >= MAX_MCS_11A) && (pkt_type == DOT11_N)));
	DP_STATS_INCC(peer, rx.pkt_type[pkt_type].mcs_count[mcs], 1,
		      ((mcs <= MAX_MCS_11A) && (pkt_type == DOT11_N)));
	DP_STATS_INCC(peer, rx.pkt_type[pkt_type].mcs_count[MAX_MCS - 1], 1,
		      ((mcs >= MAX_MCS_11AC) && (pkt_type == DOT11_AC)));
	DP_STATS_INCC(peer, rx.pkt_type[pkt_type].mcs_count[mcs], 1,
		      ((mcs <= MAX_MCS_11AC) && (pkt_type == DOT11_AC)));
	DP_STATS_INCC(peer, rx.pkt_type[pkt_type].mcs_count[MAX_MCS - 1], 1,
		      ((mcs >= MAX_MCS) && (pkt_type == DOT11_AX)));
	DP_STATS_INCC(peer, rx.pkt_type[pkt_type].mcs_count[mcs], 1,
		      ((mcs < MAX_MCS) && (pkt_type == DOT11_AX)));
}

#ifndef WDS_VENDOR_EXTENSION
int dp_wds_rx_policy_check(uint8_t *rx_tlv_hdr,
			   struct dp_vdev *vdev,
			   struct dp_peer *peer)
{
	return 1;
}
#endif

#ifdef RX_DESC_DEBUG_CHECK
/**
 * dp_rx_desc_nbuf_sanity_check - Add sanity check to catch REO rx_desc paddr
 *				  corruption
 *
 * @ring_desc: REO ring descriptor
 * @rx_desc: Rx descriptor
 *
 * Return: NONE
 */
QDF_STATUS dp_rx_desc_nbuf_sanity_check(struct dp_soc *soc,
					hal_ring_desc_t ring_desc,
					struct dp_rx_desc *rx_desc)
{
	struct hal_buf_info hbi;

	hal_rx_reo_buf_paddr_get(soc->hal_soc, ring_desc, &hbi);
	/* Sanity check for possible buffer paddr corruption */
	if (dp_rx_desc_paddr_sanity_check(rx_desc, (&hbi)->paddr))
		return QDF_STATUS_SUCCESS;

	return QDF_STATUS_E_FAILURE;
}

/**
 * dp_rx_desc_nbuf_len_sanity_check - Add sanity check to catch Rx buffer
 *				      out of bound access from H.W
 *
 * @soc: DP soc
 * @pkt_len: Packet length received from H.W
 *
 * Return: NONE
 */
static inline void
dp_rx_desc_nbuf_len_sanity_check(struct dp_soc *soc,
				 uint32_t pkt_len)
{
	struct rx_desc_pool *rx_desc_pool;

	rx_desc_pool = &soc->rx_desc_buf[0];
	qdf_assert_always(pkt_len <= rx_desc_pool->buf_size);
}
#else
static inline void
dp_rx_desc_nbuf_len_sanity_check(struct dp_soc *soc, uint32_t pkt_len) { }
#endif

#ifdef DP_RX_PKT_NO_PEER_DELIVER
/**
 * dp_rx_deliver_to_stack_no_peer() - try deliver rx data even if
 *				      no corresbonding peer found
 * @soc: core txrx main context
 * @nbuf: pkt skb pointer
 *
 * This function will try to deliver some RX special frames to stack
 * even there is no peer matched found. for instance, LFR case, some
 * eapol data will be sent to host before peer_map done.
 *
 * Return: None
 */
void dp_rx_deliver_to_stack_no_peer(struct dp_soc *soc, qdf_nbuf_t nbuf)
{
	uint16_t peer_id;
	uint8_t vdev_id;
	struct dp_vdev *vdev = NULL;
	uint32_t l2_hdr_offset = 0;
	uint16_t msdu_len = 0;
	uint32_t pkt_len = 0;
	uint8_t *rx_tlv_hdr;
	uint32_t frame_mask = FRAME_MASK_IPV4_ARP | FRAME_MASK_IPV4_DHCP |
				FRAME_MASK_IPV4_EAPOL | FRAME_MASK_IPV6_DHCP;

	peer_id = QDF_NBUF_CB_RX_PEER_ID(nbuf);
	if (peer_id > soc->max_peer_id)
		goto deliver_fail;

	vdev_id = QDF_NBUF_CB_RX_VDEV_ID(nbuf);
	vdev = dp_vdev_get_ref_by_id(soc, vdev_id, DP_MOD_ID_RX);
	if (!vdev || vdev->delete.pending || !vdev->osif_rx)
		goto deliver_fail;

	if (qdf_unlikely(qdf_nbuf_is_frag(nbuf)))
		goto deliver_fail;

	rx_tlv_hdr = qdf_nbuf_data(nbuf);
	l2_hdr_offset =
		hal_rx_msdu_end_l3_hdr_padding_get(soc->hal_soc, rx_tlv_hdr);

	msdu_len = QDF_NBUF_CB_RX_PKT_LEN(nbuf);
	pkt_len = msdu_len + l2_hdr_offset + soc->rx_pkt_tlv_size;
	QDF_NBUF_CB_RX_NUM_ELEMENTS_IN_LIST(nbuf) = 1;

	qdf_nbuf_set_pktlen(nbuf, pkt_len);
	qdf_nbuf_pull_head(nbuf, soc->rx_pkt_tlv_size + l2_hdr_offset);

	if (dp_rx_is_special_frame(nbuf, frame_mask)) {
		qdf_nbuf_set_exc_frame(nbuf, 1);
		if (QDF_STATUS_SUCCESS !=
		    vdev->osif_rx(vdev->osif_vdev, nbuf))
			goto deliver_fail;
		DP_STATS_INC(soc, rx.err.pkt_delivered_no_peer, 1);
		dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_RX);
		return;
	}

deliver_fail:
	DP_STATS_INC_PKT(soc, rx.err.rx_invalid_peer, 1,
			 QDF_NBUF_CB_RX_PKT_LEN(nbuf));
	qdf_nbuf_free(nbuf);
	if (vdev)
		dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_RX);
}
#else
void dp_rx_deliver_to_stack_no_peer(struct dp_soc *soc, qdf_nbuf_t nbuf)
{
	DP_STATS_INC_PKT(soc, rx.err.rx_invalid_peer, 1,
			 QDF_NBUF_CB_RX_PKT_LEN(nbuf));
	qdf_nbuf_free(nbuf);
}
#endif

/**
 * dp_rx_srng_get_num_pending() - get number of pending entries
 * @hal_soc: hal soc opaque pointer
 * @hal_ring: opaque pointer to the HAL Rx Ring
 * @num_entries: number of entries in the hal_ring.
 * @near_full: pointer to a boolean. This is set if ring is near full.
 *
 * The function returns the number of entries in a destination ring which are
 * yet to be reaped. The function also checks if the ring is near full.
 * If more than half of the ring needs to be reaped, the ring is considered
 * approaching full.
 * The function useses hal_srng_dst_num_valid_locked to get the number of valid
 * entries. It should not be called within a SRNG lock. HW pointer value is
 * synced into cached_hp.
 *
 * Return: Number of pending entries if any
 */
uint32_t dp_rx_srng_get_num_pending(hal_soc_handle_t hal_soc,
				    hal_ring_handle_t hal_ring_hdl,
				    uint32_t num_entries,
				    bool *near_full)
{
	uint32_t num_pending = 0;

	num_pending = hal_srng_dst_num_valid_locked(hal_soc,
						    hal_ring_hdl,
						    true);

	if (num_entries && (num_pending >= num_entries >> 1))
		*near_full = true;
	else
		*near_full = false;

	return num_pending;
}

#endif /* QCA_HOST_MODE_WIFI_DISABLED */

#ifdef WLAN_SUPPORT_RX_FISA
void dp_rx_skip_tlvs(struct dp_soc *soc, qdf_nbuf_t nbuf, uint32_t l3_padding)
{
	QDF_NBUF_CB_RX_PACKET_L3_HDR_PAD(nbuf) = l3_padding;
	qdf_nbuf_pull_head(nbuf, l3_padding + soc->rx_pkt_tlv_size);
}

/**
 * dp_rx_set_hdr_pad() - set l3 padding in nbuf cb
 * @nbuf: pkt skb pointer
 * @l3_padding: l3 padding
 *
 * Return: None
 */
static inline
void dp_rx_set_hdr_pad(qdf_nbuf_t nbuf, uint32_t l3_padding)
{
	QDF_NBUF_CB_RX_PACKET_L3_HDR_PAD(nbuf) = l3_padding;
}
#else
void dp_rx_skip_tlvs(struct dp_soc *soc, qdf_nbuf_t nbuf, uint32_t l3_padding)
{
	qdf_nbuf_pull_head(nbuf, l3_padding + soc->rx_pkt_tlv_size);
}

static inline
void dp_rx_set_hdr_pad(qdf_nbuf_t nbuf, uint32_t l3_padding)
{
}
#endif

#ifndef QCA_HOST_MODE_WIFI_DISABLED

#ifdef DP_RX_DROP_RAW_FRM
/**
 * dp_rx_is_raw_frame_dropped() - if raw frame nbuf, free and drop
 * @nbuf: pkt skb pointer
 *
 * Return: true - raw frame, dropped
 *	   false - not raw frame, do nothing
 */
bool dp_rx_is_raw_frame_dropped(qdf_nbuf_t nbuf)
{
	if (qdf_nbuf_is_raw_frame(nbuf)) {
		qdf_nbuf_free(nbuf);
		return true;
	}

	return false;
}
#endif

#ifdef WLAN_FEATURE_DP_RX_RING_HISTORY
/**
 * dp_rx_ring_record_entry() - Record an entry into the rx ring history.
 * @soc: Datapath soc structure
 * @ring_num: REO ring number
 * @ring_desc: REO ring descriptor
 *
 * Returns: None
 */
void
dp_rx_ring_record_entry(struct dp_soc *soc, uint8_t ring_num,
			hal_ring_desc_t ring_desc)
{
	struct dp_buf_info_record *record;
	struct hal_buf_info hbi;
	uint32_t idx;

	if (qdf_unlikely(!soc->rx_ring_history[ring_num]))
		return;

	hal_rx_reo_buf_paddr_get(soc->hal_soc, ring_desc, &hbi);

	/* buffer_addr_info is the first element of ring_desc */
	hal_rx_buf_cookie_rbm_get(soc->hal_soc, (uint32_t *)ring_desc,
				  &hbi);

	idx = dp_history_get_next_index(&soc->rx_ring_history[ring_num]->index,
					DP_RX_HIST_MAX);

	/* No NULL check needed for record since its an array */
	record = &soc->rx_ring_history[ring_num]->entry[idx];

	record->timestamp = qdf_get_log_timestamp();
	record->hbi.paddr = hbi.paddr;
	record->hbi.sw_cookie = hbi.sw_cookie;
	record->hbi.rbm = hbi.rbm;
}
#endif

#ifdef WLAN_DP_FEATURE_SW_LATENCY_MGR
/**
 * dp_rx_update_stats() - Update soc level rx packet count
 * @soc: DP soc handle
 * @nbuf: nbuf received
 *
 * Returns: none
 */
void dp_rx_update_stats(struct dp_soc *soc, qdf_nbuf_t nbuf)
{
	DP_STATS_INC_PKT(soc, rx.ingress, 1,
			 QDF_NBUF_CB_RX_PKT_LEN(nbuf));
}
#endif

#ifdef WLAN_FEATURE_PKT_CAPTURE_V2
/**
 * dp_rx_deliver_to_pkt_capture() - deliver rx packet to packet capture
 * @soc : dp_soc handle
 * @pdev: dp_pdev handle
 * @peer_id: peer_id of the peer for which completion came
 * @ppdu_id: ppdu_id
 * @netbuf: Buffer pointer
 *
 * This function is used to deliver rx packet to packet capture
 */
void dp_rx_deliver_to_pkt_capture(struct dp_soc *soc,  struct dp_pdev *pdev,
				  uint16_t peer_id, uint32_t is_offload,
				  qdf_nbuf_t netbuf)
{
	if (wlan_cfg_get_pkt_capture_mode(soc->wlan_cfg_ctx))
		dp_wdi_event_handler(WDI_EVENT_PKT_CAPTURE_RX_DATA, soc, netbuf,
				     peer_id, is_offload, pdev->pdev_id);
}

void dp_rx_deliver_to_pkt_capture_no_peer(struct dp_soc *soc, qdf_nbuf_t nbuf,
					  uint32_t is_offload)
{
	if (wlan_cfg_get_pkt_capture_mode(soc->wlan_cfg_ctx))
		dp_wdi_event_handler(WDI_EVENT_PKT_CAPTURE_RX_DATA_NO_PEER,
				     soc, nbuf, HTT_INVALID_VDEV,
				     is_offload, 0);
}
#endif

#endif /* QCA_HOST_MODE_WIFI_DISABLED */

QDF_STATUS dp_rx_vdev_detach(struct dp_vdev *vdev)
{
	QDF_STATUS ret;

	if (vdev->osif_rx_flush) {
		ret = vdev->osif_rx_flush(vdev->osif_vdev, vdev->vdev_id);
		if (!QDF_IS_STATUS_SUCCESS(ret)) {
			dp_err("Failed to flush rx pkts for vdev %d\n",
			       vdev->vdev_id);
			return ret;
		}
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
dp_pdev_nbuf_alloc_and_map(struct dp_soc *dp_soc,
			   struct dp_rx_nbuf_frag_info *nbuf_frag_info_t,
			   struct dp_pdev *dp_pdev,
			   struct rx_desc_pool *rx_desc_pool)
{
	QDF_STATUS ret = QDF_STATUS_E_FAILURE;

	(nbuf_frag_info_t->virt_addr).nbuf =
		qdf_nbuf_alloc(dp_soc->osdev, rx_desc_pool->buf_size,
			       RX_BUFFER_RESERVATION,
			       rx_desc_pool->buf_alignment, FALSE);
	if (!((nbuf_frag_info_t->virt_addr).nbuf)) {
		dp_err("nbuf alloc failed");
		DP_STATS_INC(dp_pdev, replenish.nbuf_alloc_fail, 1);
		return ret;
	}

	ret = qdf_nbuf_map_nbytes_single(dp_soc->osdev,
					 (nbuf_frag_info_t->virt_addr).nbuf,
					 QDF_DMA_FROM_DEVICE,
					 rx_desc_pool->buf_size);

	if (qdf_unlikely(QDF_IS_STATUS_ERROR(ret))) {
		qdf_nbuf_free((nbuf_frag_info_t->virt_addr).nbuf);
		dp_err("nbuf map failed");
		DP_STATS_INC(dp_pdev, replenish.map_err, 1);
		return ret;
	}

	nbuf_frag_info_t->paddr =
		qdf_nbuf_get_frag_paddr((nbuf_frag_info_t->virt_addr).nbuf, 0);

	ret = dp_check_paddr(dp_soc, &((nbuf_frag_info_t->virt_addr).nbuf),
			     &nbuf_frag_info_t->paddr,
			     rx_desc_pool);
	if (ret == QDF_STATUS_E_FAILURE) {
		dp_err("nbuf check x86 failed");
		DP_STATS_INC(dp_pdev, replenish.x86_fail, 1);
		return ret;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
dp_pdev_rx_buffers_attach(struct dp_soc *dp_soc, uint32_t mac_id,
			  struct dp_srng *dp_rxdma_srng,
			  struct rx_desc_pool *rx_desc_pool,
			  uint32_t num_req_buffers)
{
	struct dp_pdev *dp_pdev = dp_get_pdev_for_lmac_id(dp_soc, mac_id);
	hal_ring_handle_t rxdma_srng = dp_rxdma_srng->hal_srng;
	union dp_rx_desc_list_elem_t *next;
	void *rxdma_ring_entry;
	qdf_dma_addr_t paddr;
	struct dp_rx_nbuf_frag_info *nf_info;
	uint32_t nr_descs, nr_nbuf = 0, nr_nbuf_total = 0;
	uint32_t buffer_index, nbuf_ptrs_per_page;
	qdf_nbuf_t nbuf;
	QDF_STATUS ret;
	int page_idx, total_pages;
	union dp_rx_desc_list_elem_t *desc_list = NULL;
	union dp_rx_desc_list_elem_t *tail = NULL;
	int sync_hw_ptr = 1;
	uint32_t num_entries_avail;

	if (qdf_unlikely(!dp_pdev)) {
		dp_rx_err("%pK: pdev is null for mac_id = %d",
			  dp_soc, mac_id);
		return QDF_STATUS_E_FAILURE;
	}

	if (qdf_unlikely(!rxdma_srng)) {
		DP_STATS_INC(dp_pdev, replenish.rxdma_err, num_req_buffers);
		return QDF_STATUS_E_FAILURE;
	}

	dp_debug("requested %u RX buffers for driver attach", num_req_buffers);

	hal_srng_access_start(dp_soc->hal_soc, rxdma_srng);
	num_entries_avail = hal_srng_src_num_avail(dp_soc->hal_soc,
						   rxdma_srng,
						   sync_hw_ptr);
	hal_srng_access_end(dp_soc->hal_soc, rxdma_srng);

	if (!num_entries_avail) {
		dp_err("Num of available entries is zero, nothing to do");
		return QDF_STATUS_E_NOMEM;
	}

	if (num_entries_avail < num_req_buffers)
		num_req_buffers = num_entries_avail;

	nr_descs = dp_rx_get_free_desc_list(dp_soc, mac_id, rx_desc_pool,
					    num_req_buffers, &desc_list, &tail);
	if (!nr_descs) {
		dp_err("no free rx_descs in freelist");
		DP_STATS_INC(dp_pdev, err.desc_alloc_fail, num_req_buffers);
		return QDF_STATUS_E_NOMEM;
	}

	dp_debug("got %u RX descs for driver attach", nr_descs);

	/*
	 * Try to allocate pointers to the nbuf one page at a time.
	 * Take pointers that can fit in one page of memory and
	 * iterate through the total descriptors that need to be
	 * allocated in order of pages. Reuse the pointers that
	 * have been allocated to fit in one page across each
	 * iteration to index into the nbuf.
	 */
	total_pages = (nr_descs * sizeof(*nf_info)) / PAGE_SIZE;

	/*
	 * Add an extra page to store the remainder if any
	 */
	if ((nr_descs * sizeof(*nf_info)) % PAGE_SIZE)
		total_pages++;
	nf_info = qdf_mem_malloc(PAGE_SIZE);
	if (!nf_info) {
		dp_err("failed to allocate nbuf array");
		DP_STATS_INC(dp_pdev, replenish.rxdma_err, num_req_buffers);
		QDF_BUG(0);
		return QDF_STATUS_E_NOMEM;
	}
	nbuf_ptrs_per_page = PAGE_SIZE / sizeof(*nf_info);

	for (page_idx = 0; page_idx < total_pages; page_idx++) {
		qdf_mem_zero(nf_info, PAGE_SIZE);

		for (nr_nbuf = 0; nr_nbuf < nbuf_ptrs_per_page; nr_nbuf++) {
			/*
			 * The last page of buffer pointers may not be required
			 * completely based on the number of descriptors. Below
			 * check will ensure we are allocating only the
			 * required number of descriptors.
			 */
			if (nr_nbuf_total >= nr_descs)
				break;
			/* Flag is set while pdev rx_desc_pool initialization */
			if (qdf_unlikely(rx_desc_pool->rx_mon_dest_frag_enable))
				ret = dp_pdev_frag_alloc_and_map(dp_soc,
						&nf_info[nr_nbuf], dp_pdev,
						rx_desc_pool);
			else
				ret = dp_pdev_nbuf_alloc_and_map(dp_soc,
						&nf_info[nr_nbuf], dp_pdev,
						rx_desc_pool);
			if (QDF_IS_STATUS_ERROR(ret))
				break;

			nr_nbuf_total++;
		}

		hal_srng_access_start(dp_soc->hal_soc, rxdma_srng);

		for (buffer_index = 0; buffer_index < nr_nbuf; buffer_index++) {
			rxdma_ring_entry =
				hal_srng_src_get_next(dp_soc->hal_soc,
						      rxdma_srng);
			qdf_assert_always(rxdma_ring_entry);

			next = desc_list->next;
			paddr = nf_info[buffer_index].paddr;
			nbuf = nf_info[buffer_index].virt_addr.nbuf;

			/* Flag is set while pdev rx_desc_pool initialization */
			if (qdf_unlikely(rx_desc_pool->rx_mon_dest_frag_enable))
				dp_rx_desc_frag_prep(&desc_list->rx_desc,
						     &nf_info[buffer_index]);
			else
				dp_rx_desc_prep(&desc_list->rx_desc,
						&nf_info[buffer_index]);
			desc_list->rx_desc.in_use = 1;
			dp_rx_desc_alloc_dbg_info(&desc_list->rx_desc);
			dp_rx_desc_update_dbg_info(&desc_list->rx_desc,
						   __func__,
						   RX_DESC_REPLENISHED);

			hal_rxdma_buff_addr_info_set(dp_soc->hal_soc ,rxdma_ring_entry, paddr,
						     desc_list->rx_desc.cookie,
						     rx_desc_pool->owner);
			dp_ipa_handle_rx_buf_smmu_mapping(
						dp_soc, nbuf,
						rx_desc_pool->buf_size,
						true);

			desc_list = next;
		}

		dp_rx_refill_ring_record_entry(dp_soc, dp_pdev->lmac_id,
					       rxdma_srng, nr_nbuf, nr_nbuf);
		hal_srng_access_end(dp_soc->hal_soc, rxdma_srng);
	}

	dp_info("filled %u RX buffers for driver attach", nr_nbuf_total);
	qdf_mem_free(nf_info);

	if (!nr_nbuf_total) {
		dp_err("No nbuf's allocated");
		QDF_BUG(0);
		return QDF_STATUS_E_RESOURCES;
	}

	/* No need to count the number of bytes received during replenish.
	 * Therefore set replenish.pkts.bytes as 0.
	 */
	DP_STATS_INC_PKT(dp_pdev, replenish.pkts, nr_nbuf, 0);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(dp_pdev_rx_buffers_attach);

/**
 * dp_rx_enable_mon_dest_frag() - Enable frag processing for
 *              monitor destination ring via frag.
 *
 * Enable this flag only for monitor destination buffer processing
 * if DP_RX_MON_MEM_FRAG feature is enabled.
 * If flag is set then frag based function will be called for alloc,
 * map, prep desc and free ops for desc buffer else normal nbuf based
 * function will be called.
 *
 * @rx_desc_pool: Rx desc pool
 * @is_mon_dest_desc: Is it for monitor dest buffer
 *
 * Return: None
 */
#ifdef DP_RX_MON_MEM_FRAG
void dp_rx_enable_mon_dest_frag(struct rx_desc_pool *rx_desc_pool,
				bool is_mon_dest_desc)
{
	rx_desc_pool->rx_mon_dest_frag_enable = is_mon_dest_desc;
	if (is_mon_dest_desc)
		dp_alert("Feature DP_RX_MON_MEM_FRAG for mon_dest is enabled");
}
#else
void dp_rx_enable_mon_dest_frag(struct rx_desc_pool *rx_desc_pool,
				bool is_mon_dest_desc)
{
	rx_desc_pool->rx_mon_dest_frag_enable = false;
	if (is_mon_dest_desc)
		dp_alert("Feature DP_RX_MON_MEM_FRAG for mon_dest is disabled");
}
#endif

qdf_export_symbol(dp_rx_enable_mon_dest_frag);

/*
 * dp_rx_pdev_desc_pool_alloc() -  allocate memory for software rx descriptor
 *				   pool
 *
 * @pdev: core txrx pdev context
 *
 * Return: QDF_STATUS - QDF_STATUS_SUCCESS
 *			QDF_STATUS_E_NOMEM
 */
QDF_STATUS
dp_rx_pdev_desc_pool_alloc(struct dp_pdev *pdev)
{
	struct dp_soc *soc = pdev->soc;
	uint32_t rxdma_entries;
	uint32_t rx_sw_desc_num;
	struct dp_srng *dp_rxdma_srng;
	struct rx_desc_pool *rx_desc_pool;
	uint32_t status = QDF_STATUS_SUCCESS;
	int mac_for_pdev;

	mac_for_pdev = pdev->lmac_id;
	if (wlan_cfg_get_dp_pdev_nss_enabled(pdev->wlan_cfg_ctx)) {
		dp_rx_info("%pK: nss-wifi<4> skip Rx refil %d",
			   soc, mac_for_pdev);
		return status;
	}

	dp_rxdma_srng = &soc->rx_refill_buf_ring[mac_for_pdev];
	rxdma_entries = dp_rxdma_srng->num_entries;

	rx_desc_pool = &soc->rx_desc_buf[mac_for_pdev];
	rx_sw_desc_num = wlan_cfg_get_dp_soc_rx_sw_desc_num(soc->wlan_cfg_ctx);

	rx_desc_pool->desc_type = DP_RX_DESC_BUF_TYPE;
	status = dp_rx_desc_pool_alloc(soc,
				       rx_sw_desc_num,
				       rx_desc_pool);
	if (status != QDF_STATUS_SUCCESS)
		return status;

	return status;
}

/*
 * dp_rx_pdev_desc_pool_free() - free software rx descriptor pool
 *
 * @pdev: core txrx pdev context
 */
void dp_rx_pdev_desc_pool_free(struct dp_pdev *pdev)
{
	int mac_for_pdev = pdev->lmac_id;
	struct dp_soc *soc = pdev->soc;
	struct rx_desc_pool *rx_desc_pool;

	rx_desc_pool = &soc->rx_desc_buf[mac_for_pdev];

	dp_rx_desc_pool_free(soc, rx_desc_pool);
}

/*
 * dp_rx_pdev_desc_pool_init() - initialize software rx descriptors
 *
 * @pdev: core txrx pdev context
 *
 * Return: QDF_STATUS - QDF_STATUS_SUCCESS
 *			QDF_STATUS_E_NOMEM
 */
QDF_STATUS dp_rx_pdev_desc_pool_init(struct dp_pdev *pdev)
{
	int mac_for_pdev = pdev->lmac_id;
	struct dp_soc *soc = pdev->soc;
	uint32_t rxdma_entries;
	uint32_t rx_sw_desc_num;
	struct dp_srng *dp_rxdma_srng;
	struct rx_desc_pool *rx_desc_pool;

	rx_desc_pool = &soc->rx_desc_buf[mac_for_pdev];
	if (wlan_cfg_get_dp_pdev_nss_enabled(pdev->wlan_cfg_ctx)) {
		/**
		 * If NSS is enabled, rx_desc_pool is already filled.
		 * Hence, just disable desc_pool frag flag.
		 */
		dp_rx_enable_mon_dest_frag(rx_desc_pool, false);

		dp_rx_info("%pK: nss-wifi<4> skip Rx refil %d",
			   soc, mac_for_pdev);
		return QDF_STATUS_SUCCESS;
	}

	if (dp_rx_desc_pool_is_allocated(rx_desc_pool) == QDF_STATUS_E_NOMEM)
		return QDF_STATUS_E_NOMEM;

	dp_rxdma_srng = &soc->rx_refill_buf_ring[mac_for_pdev];
	rxdma_entries = dp_rxdma_srng->num_entries;

	soc->process_rx_status = CONFIG_PROCESS_RX_STATUS;

	rx_sw_desc_num =
	wlan_cfg_get_dp_soc_rx_sw_desc_num(soc->wlan_cfg_ctx);

	rx_desc_pool->owner = dp_rx_get_rx_bm_id(soc);
	rx_desc_pool->buf_size = RX_DATA_BUFFER_SIZE;
	rx_desc_pool->buf_alignment = RX_DATA_BUFFER_ALIGNMENT;
	/* Disable monitor dest processing via frag */
	dp_rx_enable_mon_dest_frag(rx_desc_pool, false);

	dp_rx_desc_pool_init(soc, mac_for_pdev,
			     rx_sw_desc_num, rx_desc_pool);
	return QDF_STATUS_SUCCESS;
}

/*
 * dp_rx_pdev_desc_pool_deinit() - de-initialize software rx descriptor pools
 * @pdev: core txrx pdev context
 *
 * This function resets the freelist of rx descriptors and destroys locks
 * associated with this list of descriptors.
 */
void dp_rx_pdev_desc_pool_deinit(struct dp_pdev *pdev)
{
	int mac_for_pdev = pdev->lmac_id;
	struct dp_soc *soc = pdev->soc;
	struct rx_desc_pool *rx_desc_pool;

	rx_desc_pool = &soc->rx_desc_buf[mac_for_pdev];

	dp_rx_desc_pool_deinit(soc, rx_desc_pool, mac_for_pdev);
}

/*
 * dp_rx_pdev_buffers_alloc() - Allocate nbufs (skbs) and replenish RxDMA ring
 *
 * @pdev: core txrx pdev context
 *
 * Return: QDF_STATUS - QDF_STATUS_SUCCESS
 *			QDF_STATUS_E_NOMEM
 */
QDF_STATUS
dp_rx_pdev_buffers_alloc(struct dp_pdev *pdev)
{
	int mac_for_pdev = pdev->lmac_id;
	struct dp_soc *soc = pdev->soc;
	struct dp_srng *dp_rxdma_srng;
	struct rx_desc_pool *rx_desc_pool;
	uint32_t rxdma_entries;

	dp_rxdma_srng = &soc->rx_refill_buf_ring[mac_for_pdev];
	rxdma_entries = dp_rxdma_srng->num_entries;

	rx_desc_pool = &soc->rx_desc_buf[mac_for_pdev];

	/* Initialize RX buffer pool which will be
	 * used during low memory conditions
	 */
	dp_rx_buffer_pool_init(soc, mac_for_pdev);

	return dp_pdev_rx_buffers_attach(soc, mac_for_pdev, dp_rxdma_srng,
					 rx_desc_pool, rxdma_entries - 1);
}

/*
 * dp_rx_pdev_buffers_free - Free nbufs (skbs)
 *
 * @pdev: core txrx pdev context
 */
void
dp_rx_pdev_buffers_free(struct dp_pdev *pdev)
{
	int mac_for_pdev = pdev->lmac_id;
	struct dp_soc *soc = pdev->soc;
	struct rx_desc_pool *rx_desc_pool;

	rx_desc_pool = &soc->rx_desc_buf[mac_for_pdev];

	dp_rx_desc_nbuf_free(soc, rx_desc_pool);
	dp_rx_buffer_pool_deinit(soc, mac_for_pdev);
}

#ifdef DP_RX_SPECIAL_FRAME_NEED
bool dp_rx_deliver_special_frame(struct dp_soc *soc, struct dp_peer *peer,
				 qdf_nbuf_t nbuf, uint32_t frame_mask,
				 uint8_t *rx_tlv_hdr)
{
	uint32_t l2_hdr_offset = 0;
	uint16_t msdu_len = 0;
	uint32_t skip_len;

	l2_hdr_offset =
		hal_rx_msdu_end_l3_hdr_padding_get(soc->hal_soc, rx_tlv_hdr);

	if (qdf_unlikely(qdf_nbuf_is_frag(nbuf))) {
		skip_len = l2_hdr_offset;
	} else {
		msdu_len = QDF_NBUF_CB_RX_PKT_LEN(nbuf);
		skip_len = l2_hdr_offset + soc->rx_pkt_tlv_size;
		qdf_nbuf_set_pktlen(nbuf, msdu_len + skip_len);
	}

	QDF_NBUF_CB_RX_NUM_ELEMENTS_IN_LIST(nbuf) = 1;
	dp_rx_set_hdr_pad(nbuf, l2_hdr_offset);
	qdf_nbuf_pull_head(nbuf, skip_len);

	if (peer->vdev) {
		dp_rx_send_pktlog(soc, peer->vdev->pdev, nbuf,
				  QDF_TX_RX_STATUS_OK);
	}

	if (dp_rx_is_special_frame(nbuf, frame_mask)) {
		dp_info("special frame, mpdu sn 0x%x",
			hal_rx_get_rx_sequence(soc->hal_soc, rx_tlv_hdr));
		qdf_nbuf_set_exc_frame(nbuf, 1);
		dp_rx_deliver_to_stack(soc, peer->vdev, peer,
				       nbuf, NULL);
		return true;
	}

	return false;
}
#endif

#ifdef WLAN_FEATURE_MARK_FIRST_WAKEUP_PACKET
void dp_rx_mark_first_packet_after_wow_wakeup(struct dp_pdev *pdev,
					      uint8_t *rx_tlv,
					      qdf_nbuf_t nbuf)
{
	struct dp_soc *soc;

	if (!pdev->is_first_wakeup_packet)
		return;

	soc = pdev->soc;
	if (hal_get_first_wow_wakeup_packet(soc->hal_soc, rx_tlv)) {
		qdf_nbuf_mark_wakeup_frame(nbuf);
		dp_info("First packet after WOW Wakeup rcvd");
	}
}
#endif
