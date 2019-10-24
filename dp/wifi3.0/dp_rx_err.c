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

#include "hal_hw_headers.h"
#include "dp_types.h"
#include "dp_rx.h"
#include "dp_peer.h"
#include "dp_internal.h"
#include "hal_api.h"
#include "qdf_trace.h"
#include "qdf_nbuf.h"
#include "dp_rx_defrag.h"
#ifdef FEATURE_WDS
#include "dp_txrx_wds.h"
#endif
#include <enet.h>	/* LLC_SNAP_HDR_LEN */
#include "qdf_net_types.h"

/* Max buffer in invalid peer SG list*/
#define DP_MAX_INVALID_BUFFERS 10

/**
 * dp_rx_mcast_echo_check() - check if the mcast pkt is a loop
 *			      back on same vap or a different vap.
 *
 * @soc: core DP main context
 * @peer: dp peer handler
 * @rx_tlv_hdr: start of the rx TLV header
 * @nbuf: pkt buffer
 *
 * Return: bool (true if it is a looped back pkt else false)
 *
 */
static inline bool dp_rx_mcast_echo_check(struct dp_soc *soc,
					struct dp_peer *peer,
					uint8_t *rx_tlv_hdr,
					qdf_nbuf_t nbuf)
{
	struct dp_vdev *vdev = peer->vdev;
	struct dp_ast_entry *ase = NULL;
	uint16_t sa_idx = 0;
	uint8_t *data;

	/*
	 * Multicast Echo Check is required only if vdev is STA and
	 * received pkt is a multicast/broadcast pkt. otherwise
	 * skip the MEC check.
	 */
	if (vdev->opmode != wlan_op_mode_sta)
		return false;

	if (!hal_rx_msdu_end_da_is_mcbc_get(soc->hal_soc, rx_tlv_hdr))
		return false;

	data = qdf_nbuf_data(nbuf);
	/*
	 * if the received pkts src mac addr matches with vdev
	 * mac address then drop the pkt as it is looped back
	 */
	if (!(qdf_mem_cmp(&data[QDF_MAC_ADDR_SIZE],
			vdev->mac_addr.raw,
			QDF_MAC_ADDR_SIZE)))
		return true;

	/*
	 * In case of qwrap isolation mode, donot drop loopback packets.
	 * In isolation mode, all packets from the wired stations need to go
	 * to rootap and loop back to reach the wireless stations and
	 * vice-versa.
	 */
	if (qdf_unlikely(vdev->isolation_vdev))
		return false;

	/* if the received pkts src mac addr matches with the
	 * wired PCs MAC addr which is behind the STA or with
	 * wireless STAs MAC addr which are behind the Repeater,
	 * then drop the pkt as it is looped back
	 */
	qdf_spin_lock_bh(&soc->ast_lock);
	if (hal_rx_msdu_end_sa_is_valid_get(soc->hal_soc, rx_tlv_hdr)) {
		sa_idx = hal_rx_msdu_end_sa_idx_get(soc->hal_soc, rx_tlv_hdr);

		if ((sa_idx < 0) ||
		    (sa_idx >= wlan_cfg_get_max_ast_idx(soc->wlan_cfg_ctx))) {
			qdf_spin_unlock_bh(&soc->ast_lock);
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
					"invalid sa_idx: %d", sa_idx);
			qdf_assert_always(0);
		}

		ase = soc->ast_table[sa_idx];
		if (!ase) {
			/* We do not get a peer map event for STA and without
			 * this event we don't know what is STA's sa_idx.
			 * For this reason the AST is still not associated to
			 * any index postion in ast_table.
			 * In these kind of scenarios where sa is valid but
			 * ast is not in ast_table, we use the below API to get
			 * AST entry for STA's own mac_address.
			 */
			ase = dp_peer_ast_list_find(soc, peer,
						    &data[QDF_MAC_ADDR_SIZE]);
			if (ase) {
				ase->ast_idx = sa_idx;
				soc->ast_table[sa_idx] = ase;
				ase->is_mapped = TRUE;
			}
		}
	} else {
		ase = dp_peer_ast_hash_find_by_pdevid(soc,
						      &data[QDF_MAC_ADDR_SIZE],
						      vdev->pdev->pdev_id);
	}

	if (ase) {

		if (ase->pdev_id != vdev->pdev->pdev_id) {
			qdf_spin_unlock_bh(&soc->ast_lock);
			QDF_TRACE(QDF_MODULE_ID_DP,
				QDF_TRACE_LEVEL_INFO,
				"Detected DBDC Root AP %pM, %d %d",
				&data[QDF_MAC_ADDR_SIZE], vdev->pdev->pdev_id,
				ase->pdev_id);
			return false;
		}

		if ((ase->type == CDP_TXRX_AST_TYPE_MEC) ||
				(ase->peer != peer)) {
			qdf_spin_unlock_bh(&soc->ast_lock);
			QDF_TRACE(QDF_MODULE_ID_DP,
				QDF_TRACE_LEVEL_INFO,
				"received pkt with same src mac %pM",
				&data[QDF_MAC_ADDR_SIZE]);

			return true;
		}
	}
	qdf_spin_unlock_bh(&soc->ast_lock);
	return false;
}

/**
 * dp_rx_link_desc_return_by_addr - Return a MPDU link descriptor to
 *					(WBM) by address
 *
 * @soc: core DP main context
 * @link_desc_addr: link descriptor addr
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
dp_rx_link_desc_return_by_addr(struct dp_soc *soc,
			       hal_buff_addrinfo_t link_desc_addr,
			       uint8_t bm_action)
{
	struct dp_srng *wbm_desc_rel_ring = &soc->wbm_desc_rel_ring;
	hal_ring_handle_t wbm_rel_srng = wbm_desc_rel_ring->hal_srng;
	hal_soc_handle_t hal_soc = soc->hal_soc;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	void *src_srng_desc;

	if (!wbm_rel_srng) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"WBM RELEASE RING not initialized");
		return status;
	}

	if (qdf_unlikely(hal_srng_access_start(hal_soc, wbm_rel_srng))) {

		/* TODO */
		/*
		 * Need API to convert from hal_ring pointer to
		 * Ring Type / Ring Id combo
		 */
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("HAL RING Access For WBM Release SRNG Failed - %pK"),
			wbm_rel_srng);
		DP_STATS_INC(soc, rx.err.hal_ring_access_fail, 1);
		goto done;
	}
	src_srng_desc = hal_srng_src_get_next(hal_soc, wbm_rel_srng);
	if (qdf_likely(src_srng_desc)) {
		/* Return link descriptor through WBM ring (SW2WBM)*/
		hal_rx_msdu_link_desc_set(hal_soc,
				src_srng_desc, link_desc_addr, bm_action);
		status = QDF_STATUS_SUCCESS;
	} else {
		struct hal_srng *srng = (struct hal_srng *)wbm_rel_srng;
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("WBM Release Ring (Id %d) Full"), srng->ring_id);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"HP 0x%x Reap HP 0x%x TP 0x%x Cached TP 0x%x",
			*srng->u.src_ring.hp_addr, srng->u.src_ring.reap_hp,
			*srng->u.src_ring.tp_addr, srng->u.src_ring.cached_tp);
	}
done:
	hal_srng_access_end(hal_soc, wbm_rel_srng);
	return status;

}

/**
 * dp_rx_link_desc_return() - Return a MPDU link descriptor to HW
 *				(WBM), following error handling
 *
 * @soc: core DP main context
 * @ring_desc: opaque pointer to the REO error ring descriptor
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
dp_rx_link_desc_return(struct dp_soc *soc, hal_ring_desc_t ring_desc,
		       uint8_t bm_action)
{
	void *buf_addr_info = HAL_RX_REO_BUF_ADDR_INFO_GET(ring_desc);

	return dp_rx_link_desc_return_by_addr(soc, buf_addr_info, bm_action);
}

/**
 * dp_rx_msdus_drop() - Drops all MSDU's per MPDU
 *
 * @soc: core txrx main context
 * @ring_desc: opaque pointer to the REO error ring descriptor
 * @mpdu_desc_info: MPDU descriptor information from ring descriptor
 * @head: head of the local descriptor free-list
 * @tail: tail of the local descriptor free-list
 * @quota: No. of units (packets) that can be serviced in one shot.
 *
 * This function is used to drop all MSDU in an MPDU
 *
 * Return: uint32_t: No. of elements processed
 */
static uint32_t
dp_rx_msdus_drop(struct dp_soc *soc, hal_ring_desc_t ring_desc,
		 struct hal_rx_mpdu_desc_info *mpdu_desc_info,
		 uint8_t *mac_id,
		 uint32_t quota)
{
	uint32_t rx_bufs_used = 0;
	void *link_desc_va;
	struct hal_buf_info buf_info;
	struct dp_pdev *pdev;
	struct hal_rx_msdu_list msdu_list; /* MSDU's per MPDU */
	int i;
	uint8_t *rx_tlv_hdr;
	uint32_t tid;

	hal_rx_reo_buf_paddr_get(ring_desc, &buf_info);

	link_desc_va = dp_rx_cookie_2_link_desc_va(soc, &buf_info);

	/* No UNMAP required -- this is "malloc_consistent" memory */
	hal_rx_msdu_list_get(soc->hal_soc, link_desc_va, &msdu_list,
			     &mpdu_desc_info->msdu_count);

	for (i = 0; (i < mpdu_desc_info->msdu_count) && quota--; i++) {
		struct dp_rx_desc *rx_desc =
			dp_rx_cookie_2_va_rxdma_buf(soc,
			msdu_list.sw_cookie[i]);

		qdf_assert_always(rx_desc);

		/* all buffers from a MSDU link link belong to same pdev */
		*mac_id = rx_desc->pool_id;
		pdev = soc->pdev_list[rx_desc->pool_id];

		if (!dp_rx_desc_check_magic(rx_desc)) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
					FL("Invalid rx_desc cookie=%d"),
					msdu_list.sw_cookie[i]);
			return rx_bufs_used;
		}

		qdf_nbuf_unmap_single(soc->osdev,
				      rx_desc->nbuf, QDF_DMA_FROM_DEVICE);

		rx_desc->rx_buf_start = qdf_nbuf_data(rx_desc->nbuf);

		rx_bufs_used++;
		tid = hal_rx_mpdu_start_tid_get(soc->hal_soc,
						rx_desc->rx_buf_start);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"Packet received with PN error for tid :%d", tid);

		rx_tlv_hdr = qdf_nbuf_data(rx_desc->nbuf);
		if (hal_rx_encryption_info_valid(soc->hal_soc, rx_tlv_hdr))
			hal_rx_print_pn(soc->hal_soc, rx_tlv_hdr);

		/* Just free the buffers */
		qdf_nbuf_free(rx_desc->nbuf);

		dp_rx_add_to_free_desc_list(&pdev->free_list_head,
					    &pdev->free_list_tail, rx_desc);
	}

	/* Return link descriptor through WBM ring (SW2WBM)*/
	dp_rx_link_desc_return(soc, ring_desc, HAL_BM_ACTION_PUT_IN_IDLE_LIST);

	return rx_bufs_used;
}

/**
 * dp_rx_pn_error_handle() - Handles PN check errors
 *
 * @soc: core txrx main context
 * @ring_desc: opaque pointer to the REO error ring descriptor
 * @mpdu_desc_info: MPDU descriptor information from ring descriptor
 * @head: head of the local descriptor free-list
 * @tail: tail of the local descriptor free-list
 * @quota: No. of units (packets) that can be serviced in one shot.
 *
 * This function implements PN error handling
 * If the peer is configured to ignore the PN check errors
 * or if DP feels, that this frame is still OK, the frame can be
 * re-injected back to REO to use some of the other features
 * of REO e.g. duplicate detection/routing to other cores
 *
 * Return: uint32_t: No. of elements processed
 */
static uint32_t
dp_rx_pn_error_handle(struct dp_soc *soc, hal_ring_desc_t ring_desc,
		      struct hal_rx_mpdu_desc_info *mpdu_desc_info,
		      uint8_t *mac_id,
		      uint32_t quota)
{
	uint16_t peer_id;
	uint32_t rx_bufs_used = 0;
	struct dp_peer *peer;
	bool peer_pn_policy = false;

	peer_id = DP_PEER_METADATA_PEER_ID_GET(
				mpdu_desc_info->peer_meta_data);


	peer = dp_peer_find_by_id(soc, peer_id);

	if (qdf_likely(peer)) {
		/*
		 * TODO: Check for peer specific policies & set peer_pn_policy
		 */
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"discard rx due to PN error for peer  %pK  "
			"(%02x:%02x:%02x:%02x:%02x:%02x)",
			peer,
			peer->mac_addr.raw[0], peer->mac_addr.raw[1],
			peer->mac_addr.raw[2], peer->mac_addr.raw[3],
			peer->mac_addr.raw[4], peer->mac_addr.raw[5]);

		dp_peer_unref_del_find_by_id(peer);
	}
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
		"Packet received with PN error");

	/* No peer PN policy -- definitely drop */
	if (!peer_pn_policy)
		rx_bufs_used = dp_rx_msdus_drop(soc, ring_desc,
						mpdu_desc_info,
						mac_id, quota);

	return rx_bufs_used;
}

/**
 * dp_rx_2k_jump_handle() - Handles Sequence Number Jump by 2K
 *
 * @soc: core txrx main context
 * @ring_desc: opaque pointer to the REO error ring descriptor
 * @mpdu_desc_info: MPDU descriptor information from ring descriptor
 * @head: head of the local descriptor free-list
 * @tail: tail of the local descriptor free-list
 * @quota: No. of units (packets) that can be serviced in one shot.
 *
 * This function implements the error handling when sequence number
 * of the MPDU jumps suddenly by 2K.Today there are 2 cases that
 * need to be handled:
 *	A) CSN (Current Sequence Number) = Last Valid SN (LSN) + 2K
 *	B) CSN = LSN + 2K, but falls within a "BA sized window" of the SSN
 * For case A) the protocol stack is invoked to generate DELBA/DEAUTH frame
 * For case B), the frame is normally dropped, no more action is taken
 *
 * Return: uint32_t: No. of elements processed
 */
static uint32_t
dp_rx_2k_jump_handle(struct dp_soc *soc, hal_ring_desc_t ring_desc,
		     struct hal_rx_mpdu_desc_info *mpdu_desc_info,
		     uint8_t *mac_id, uint32_t quota)
{
	return dp_rx_msdus_drop(soc, ring_desc, mpdu_desc_info,
				mac_id, quota);
}

#ifdef DP_INVALID_PEER_ASSERT
#define DP_PDEV_INVALID_PEER_MSDU_CHECK(head, tail) \
		do {                                \
			qdf_assert_always(!(head)); \
			qdf_assert_always(!(tail)); \
		} while (0)
#else
#define DP_PDEV_INVALID_PEER_MSDU_CHECK(head, tail) /* no op */
#endif

/**
 * dp_rx_chain_msdus() - Function to chain all msdus of a mpdu
 *                       to pdev invalid peer list
 *
 * @soc: core DP main context
 * @nbuf: Buffer pointer
 * @rx_tlv_hdr: start of rx tlv header
 * @mac_id: mac id
 *
 *  Return: bool: true for last msdu of mpdu
 */
static bool
dp_rx_chain_msdus(struct dp_soc *soc, qdf_nbuf_t nbuf,
		  uint8_t *rx_tlv_hdr, uint8_t mac_id)
{
	bool mpdu_done = false;
	qdf_nbuf_t curr_nbuf = NULL;
	qdf_nbuf_t tmp_nbuf = NULL;

	/* TODO: Currently only single radio is supported, hence
	 * pdev hard coded to '0' index
	 */
	struct dp_pdev *dp_pdev = soc->pdev_list[mac_id];

	/* if invalid peer SG list has max values free the buffers in list
	 * and treat current buffer as start of list
	 *
	 * current logic to detect the last buffer from attn_tlv is not reliable
	 * in OFDMA UL scenario hence add max buffers check to avoid list pile
	 * up
	 */
	if (!dp_pdev->first_nbuf ||
	    (dp_pdev->invalid_peer_head_msdu &&
	    QDF_NBUF_CB_RX_NUM_ELEMENTS_IN_LIST
	    (dp_pdev->invalid_peer_head_msdu) >= DP_MAX_INVALID_BUFFERS)) {
		qdf_nbuf_set_rx_chfrag_start(nbuf, 1);
		dp_pdev->ppdu_id = hal_rx_hw_desc_get_ppduid_get(soc->hal_soc,
								 rx_tlv_hdr);
		dp_pdev->first_nbuf = true;

		/* If the new nbuf received is the first msdu of the
		 * amsdu and there are msdus in the invalid peer msdu
		 * list, then let us free all the msdus of the invalid
		 * peer msdu list.
		 * This scenario can happen when we start receiving
		 * new a-msdu even before the previous a-msdu is completely
		 * received.
		 */
		curr_nbuf = dp_pdev->invalid_peer_head_msdu;
		while (curr_nbuf) {
			tmp_nbuf = curr_nbuf->next;
			qdf_nbuf_free(curr_nbuf);
			curr_nbuf = tmp_nbuf;
		}

		dp_pdev->invalid_peer_head_msdu = NULL;
		dp_pdev->invalid_peer_tail_msdu = NULL;
		hal_rx_mon_hw_desc_get_mpdu_status(soc->hal_soc, rx_tlv_hdr,
				&(dp_pdev->ppdu_info.rx_status));

	}

	if (dp_pdev->ppdu_id == hal_rx_attn_phy_ppdu_id_get(rx_tlv_hdr) &&
	    hal_rx_attn_msdu_done_get(rx_tlv_hdr)) {
		qdf_nbuf_set_rx_chfrag_end(nbuf, 1);
		qdf_assert_always(dp_pdev->first_nbuf == true);
		dp_pdev->first_nbuf = false;
		mpdu_done = true;
	}

	/*
	 * For MCL, invalid_peer_head_msdu and invalid_peer_tail_msdu
	 * should be NULL here, add the checking for debugging purpose
	 * in case some corner case.
	 */
	DP_PDEV_INVALID_PEER_MSDU_CHECK(dp_pdev->invalid_peer_head_msdu,
					dp_pdev->invalid_peer_tail_msdu);
	DP_RX_LIST_APPEND(dp_pdev->invalid_peer_head_msdu,
				dp_pdev->invalid_peer_tail_msdu,
				nbuf);

	return mpdu_done;
}

static
void dp_rx_wbm_err_handle_bar(struct dp_soc *soc,
			      struct dp_peer *peer,
			      qdf_nbuf_t nbuf)
{
	uint8_t *rx_tlv_hdr;
	unsigned char type, subtype;
	uint16_t start_seq_num;
	uint32_t tid;
	struct ieee80211_frame_bar *bar;

	/*
	 * 1. Is this a BAR frame. If not Discard it.
	 * 2. If it is, get the peer id, tid, ssn
	 * 2a Do a tid update
	 */

	rx_tlv_hdr = qdf_nbuf_data(nbuf);
	bar = (struct ieee80211_frame_bar *)(rx_tlv_hdr +
					     sizeof(struct rx_pkt_tlvs));

	type = bar->i_fc[0] & IEEE80211_FC0_TYPE_MASK;
	subtype = bar->i_fc[0] & IEEE80211_FC0_SUBTYPE_MASK;

	if (!(type == IEEE80211_FC0_TYPE_CTL &&
	      subtype == QDF_IEEE80211_FC0_SUBTYPE_BAR)) {
		dp_err_rl("Not a BAR frame!");
		return;
	}

	tid = hal_rx_mpdu_start_tid_get(soc->hal_soc, rx_tlv_hdr);
	qdf_assert_always(tid < DP_MAX_TIDS);

	start_seq_num = le16toh(bar->i_seq) >> IEEE80211_SEQ_SEQ_SHIFT;

	dp_info_rl("tid %u window_size %u start_seq_num %u",
		   tid, peer->rx_tid[tid].ba_win_size, start_seq_num);

	dp_rx_tid_update_wifi3(peer, tid,
			       peer->rx_tid[tid].ba_win_size,
			       start_seq_num);
}

/**
 * dp_2k_jump_handle() - Function to handle 2k jump exception
 *                        on WBM ring
 *
 * @soc: core DP main context
 * @nbuf: buffer pointer
 * @rx_tlv_hdr: start of rx tlv header
 * @peer_id: peer id of first msdu
 * @tid: Tid for which exception occurred
 *
 * This function handles 2k jump violations arising out
 * of receiving aggregates in non BA case. This typically
 * may happen if aggregates are received on a QOS enabled TID
 * while Rx window size is still initialized to value of 2. Or
 * it may also happen if negotiated window size is 1 but peer
 * sends aggregates.
 *
 */

void
dp_2k_jump_handle(struct dp_soc *soc,
		  qdf_nbuf_t nbuf,
		  uint8_t *rx_tlv_hdr,
		  uint16_t peer_id,
		  uint8_t tid)
{
	uint32_t ppdu_id;
	struct dp_peer *peer = NULL;
	struct dp_rx_tid *rx_tid = NULL;

	peer = dp_peer_find_by_id(soc, peer_id);
	if (!peer || peer->delete_in_progress) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "peer not found");
		goto free_nbuf;
	}
	rx_tid = &peer->rx_tid[tid];
	if (qdf_unlikely(!rx_tid)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "rx_tid is NULL!!");
		goto free_nbuf;
	}
	qdf_spin_lock_bh(&rx_tid->tid_lock);
	ppdu_id = hal_rx_attn_phy_ppdu_id_get(rx_tlv_hdr);

	/*
	 * If BA session is created and a non-aggregate packet is
	 * landing here then the issue is with sequence number mismatch.
	 * Proceed with delba even in that case
	 */
	if (rx_tid->ppdu_id_2k != ppdu_id &&
	    rx_tid->ba_status != DP_RX_BA_ACTIVE) {
		rx_tid->ppdu_id_2k = ppdu_id;
		qdf_spin_unlock_bh(&rx_tid->tid_lock);
		goto free_nbuf;
	}
	if (!rx_tid->delba_tx_status) {
		rx_tid->delba_tx_retry++;
		rx_tid->delba_tx_status = 1;
		rx_tid->delba_rcode =
			IEEE80211_REASON_QOS_SETUP_REQUIRED;
		qdf_spin_unlock_bh(&rx_tid->tid_lock);
		if (soc->cdp_soc.ol_ops->send_delba)
			soc->cdp_soc.ol_ops->send_delba(
					peer->vdev->pdev->soc->ctrl_psoc,
					peer->vdev->vdev_id,
					peer->mac_addr.raw,
					tid,
					rx_tid->delba_rcode);
	} else {
		qdf_spin_unlock_bh(&rx_tid->tid_lock);
	}

free_nbuf:
	if (peer)
		dp_peer_unref_del_find_by_id(peer);
	qdf_nbuf_free(nbuf);
	return;
}

#if defined(QCA_WIFI_QCA6390) || defined(QCA_WIFI_QCA6490)
/**
 * dp_rx_null_q_handle_invalid_peer_id_exception() - to find exception
 * @soc: pointer to dp_soc struct
 * @pool_id: Pool id to find dp_pdev
 * @rx_tlv_hdr: TLV header of received packet
 * @nbuf: SKB
 *
 * In certain types of packets if peer_id is not correct then
 * driver may not be able find. Try finding peer by addr_2 of
 * received MPDU. If you find the peer then most likely sw_peer_id &
 * ast_idx is corrupted.
 *
 * Return: True if you find the peer by addr_2 of received MPDU else false
 */
static bool
dp_rx_null_q_handle_invalid_peer_id_exception(struct dp_soc *soc,
					      uint8_t pool_id,
					      uint8_t *rx_tlv_hdr,
					      qdf_nbuf_t nbuf)
{
	struct dp_peer *peer = NULL;
	uint8_t *rx_pkt_hdr = hal_rx_pkt_hdr_get(rx_tlv_hdr);
	struct dp_pdev *pdev = soc->pdev_list[pool_id];
	struct ieee80211_frame *wh = (struct ieee80211_frame *)rx_pkt_hdr;

	/*
	 * WAR- In certain types of packets if peer_id is not correct then
	 * driver may not be able find. Try finding peer by addr_2 of
	 * received MPDU
	 */
	if (wh)
		peer = dp_find_peer_by_addr((struct cdp_pdev *)pdev,
					    wh->i_addr2);
	if (peer) {
		dp_verbose_debug("MPDU sw_peer_id & ast_idx is corrupted");
		hal_rx_dump_pkt_tlvs(soc->hal_soc, rx_tlv_hdr,
				     QDF_TRACE_LEVEL_DEBUG);
		DP_STATS_INC_PKT(soc, rx.err.rx_invalid_peer_id,
				 1, qdf_nbuf_len(nbuf));
		qdf_nbuf_free(nbuf);

		return true;
	}
	return false;
}

/**
 * dp_rx_null_q_check_pkt_len_exception() - Check for pktlen validity
 * @soc: DP SOC context
 * @pkt_len: computed length of the pkt from caller in bytes
 *
 * Return: true if pktlen > RX_BUFFER_SIZE, else return false
 *
 */
static inline
bool dp_rx_null_q_check_pkt_len_exception(struct dp_soc *soc, uint32_t pkt_len)
{
	if (qdf_unlikely(pkt_len > RX_BUFFER_SIZE)) {
		DP_STATS_INC_PKT(soc, rx.err.rx_invalid_pkt_len,
				 1, pkt_len);
		return true;
	} else {
		return false;
	}
}

#else
static inline bool
dp_rx_null_q_handle_invalid_peer_id_exception(struct dp_soc *soc,
					      uint8_t pool_id,
					      uint8_t *rx_tlv_hdr,
					      qdf_nbuf_t nbuf)
{
	return false;
}

static inline
bool dp_rx_null_q_check_pkt_len_exception(struct dp_soc *soc, uint32_t pkt_len)
{
	return false;
}

#endif

/**
 * dp_rx_null_q_desc_handle() - Function to handle NULL Queue
 *                              descriptor violation on either a
 *                              REO or WBM ring
 *
 * @soc: core DP main context
 * @nbuf: buffer pointer
 * @rx_tlv_hdr: start of rx tlv header
 * @pool_id: mac id
 * @peer: peer handle
 *
 * This function handles NULL queue descriptor violations arising out
 * a missing REO queue for a given peer or a given TID. This typically
 * may happen if a packet is received on a QOS enabled TID before the
 * ADDBA negotiation for that TID, when the TID queue is setup. Or
 * it may also happen for MC/BC frames if they are not routed to the
 * non-QOS TID queue, in the absence of any other default TID queue.
 * This error can show up both in a REO destination or WBM release ring.
 *
 * Return: QDF_STATUS_SUCCESS, if nbuf handled successfully. QDF status code
 *         if nbuf could not be handled or dropped.
 */
static QDF_STATUS
dp_rx_null_q_desc_handle(struct dp_soc *soc, qdf_nbuf_t nbuf,
			 uint8_t *rx_tlv_hdr, uint8_t pool_id,
			 struct dp_peer *peer)
{
	uint32_t pkt_len, l2_hdr_offset;
	uint16_t msdu_len;
	struct dp_vdev *vdev;
	uint8_t tid;
	qdf_ether_header_t *eh;

	qdf_nbuf_set_rx_chfrag_start(nbuf,
				hal_rx_msdu_end_first_msdu_get(soc->hal_soc,
							       rx_tlv_hdr));
	qdf_nbuf_set_rx_chfrag_end(nbuf,
				   hal_rx_msdu_end_last_msdu_get(soc->hal_soc,
								 rx_tlv_hdr));
	qdf_nbuf_set_da_mcbc(nbuf, hal_rx_msdu_end_da_is_mcbc_get(soc->hal_soc,
								  rx_tlv_hdr));
	qdf_nbuf_set_da_valid(nbuf,
			      hal_rx_msdu_end_da_is_valid_get(soc->hal_soc,
							      rx_tlv_hdr));
	qdf_nbuf_set_sa_valid(nbuf,
			      hal_rx_msdu_end_sa_is_valid_get(soc->hal_soc,
							      rx_tlv_hdr));

	l2_hdr_offset = hal_rx_msdu_end_l3_hdr_padding_get(soc->hal_soc,
							   rx_tlv_hdr);
	msdu_len = hal_rx_msdu_start_msdu_len_get(rx_tlv_hdr);
	pkt_len = msdu_len + l2_hdr_offset + RX_PKT_TLVS_LEN;

	if (qdf_likely(!qdf_nbuf_is_frag(nbuf))) {
		if (dp_rx_null_q_check_pkt_len_exception(soc, pkt_len))
			goto drop_nbuf;

		/* Set length in nbuf */
		qdf_nbuf_set_pktlen(nbuf,
				    qdf_min(pkt_len, (uint32_t)RX_BUFFER_SIZE));
		qdf_assert_always(nbuf->data == rx_tlv_hdr);
	}

	/*
	 * Check if DMA completed -- msdu_done is the last bit
	 * to be written
	 */
	if (!hal_rx_attn_msdu_done_get(rx_tlv_hdr)) {

		dp_err_rl("MSDU DONE failure");
		hal_rx_dump_pkt_tlvs(soc->hal_soc, rx_tlv_hdr,
				     QDF_TRACE_LEVEL_INFO);
		qdf_assert(0);
	}

	if (!peer &&
	    dp_rx_null_q_handle_invalid_peer_id_exception(soc, pool_id,
							  rx_tlv_hdr, nbuf))
		return QDF_STATUS_E_FAILURE;

	if (!peer) {
		bool mpdu_done = false;
		struct dp_pdev *pdev = soc->pdev_list[pool_id];

		dp_err_rl("peer is NULL");
		DP_STATS_INC_PKT(soc, rx.err.rx_invalid_peer, 1,
				 qdf_nbuf_len(nbuf));

		mpdu_done = dp_rx_chain_msdus(soc, nbuf, rx_tlv_hdr, pool_id);
		/* Trigger invalid peer handler wrapper */
		dp_rx_process_invalid_peer_wrapper(soc,
						   pdev->invalid_peer_head_msdu,
						   mpdu_done, pool_id);

		if (mpdu_done) {
			pdev->invalid_peer_head_msdu = NULL;
			pdev->invalid_peer_tail_msdu = NULL;
		}

		return QDF_STATUS_E_FAILURE;
	}

	vdev = peer->vdev;
	if (!vdev) {
		dp_err_rl("Null vdev!");
		DP_STATS_INC(soc, rx.err.invalid_vdev, 1);
		goto drop_nbuf;
	}

	/*
	 * Advance the packet start pointer by total size of
	 * pre-header TLV's
	 */
	if (qdf_nbuf_is_frag(nbuf))
		qdf_nbuf_pull_head(nbuf, RX_PKT_TLVS_LEN);
	else
		qdf_nbuf_pull_head(nbuf, (l2_hdr_offset + RX_PKT_TLVS_LEN));

	if (dp_rx_mcast_echo_check(soc, peer, rx_tlv_hdr, nbuf)) {
		/* this is a looped back MCBC pkt, drop it */
		DP_STATS_INC_PKT(peer, rx.mec_drop, 1, qdf_nbuf_len(nbuf));
		goto drop_nbuf;
	}

	/*
	 * In qwrap mode if the received packet matches with any of the vdev
	 * mac addresses, drop it. Donot receive multicast packets originated
	 * from any proxysta.
	 */
	if (check_qwrap_multicast_loopback(vdev, nbuf)) {
		DP_STATS_INC_PKT(peer, rx.mec_drop, 1, qdf_nbuf_len(nbuf));
		goto drop_nbuf;
	}


	if (qdf_unlikely((peer->nawds_enabled == true) &&
			hal_rx_msdu_end_da_is_mcbc_get(soc->hal_soc,
						       rx_tlv_hdr))) {
		dp_err_rl("free buffer for multicast packet");
		DP_STATS_INC(peer, rx.nawds_mcast_drop, 1);
		goto drop_nbuf;
	}

	if (!dp_wds_rx_policy_check(rx_tlv_hdr, vdev, peer)) {
		dp_err_rl("mcast Policy Check Drop pkt");
		goto drop_nbuf;
	}
	/* WDS Source Port Learning */
	if (qdf_likely(vdev->rx_decap_type == htt_cmn_pkt_type_ethernet &&
		vdev->wds_enabled))
		dp_rx_wds_srcport_learn(soc, rx_tlv_hdr, peer, nbuf);

	if (hal_rx_is_unicast(soc->hal_soc, rx_tlv_hdr)) {
		tid = hal_rx_tid_get(soc->hal_soc, rx_tlv_hdr);
		if (!peer->rx_tid[tid].hw_qdesc_vaddr_unaligned)
			dp_rx_tid_setup_wifi3(peer, tid, 1, IEEE80211_SEQ_MAX);
			/* IEEE80211_SEQ_MAX indicates invalid start_seq */
	}

	if (qdf_unlikely(vdev->rx_decap_type == htt_cmn_pkt_type_raw)) {
		qdf_nbuf_set_next(nbuf, NULL);
		dp_rx_deliver_raw(vdev, nbuf, peer);
	} else {
		if (vdev->osif_rx) {
			qdf_nbuf_set_next(nbuf, NULL);
			DP_STATS_INC_PKT(peer, rx.to_stack, 1,
					 qdf_nbuf_len(nbuf));

			/*
			 * Update the protocol tag in SKB based on
			 * CCE metadata
			 */
			dp_rx_update_protocol_tag(soc, vdev, nbuf, rx_tlv_hdr,
						  EXCEPTION_DEST_RING_ID,
						  true, true);

			/* Update the flow tag in SKB based on FSE metadata */
			dp_rx_update_flow_tag(soc, vdev, nbuf,
					      rx_tlv_hdr, true);

			if (qdf_unlikely(hal_rx_msdu_end_da_is_mcbc_get(
					soc->hal_soc, rx_tlv_hdr) &&
					 (vdev->rx_decap_type ==
					  htt_cmn_pkt_type_ethernet))) {
				eh = (qdf_ether_header_t *)qdf_nbuf_data(nbuf);

				DP_STATS_INC_PKT(peer, rx.multicast, 1,
						 qdf_nbuf_len(nbuf));
				if (QDF_IS_ADDR_BROADCAST(eh->ether_dhost)) {
					DP_STATS_INC_PKT(peer, rx.bcast, 1,
							 qdf_nbuf_len(nbuf));
				}
			}

			vdev->osif_rx(vdev->osif_vdev, nbuf);

		} else {
			dp_err_rl("INVALID osif_rx. vdev %pK", vdev);
			DP_STATS_INC(soc, rx.err.invalid_vdev, 1);
			goto drop_nbuf;
		}
	}
	return QDF_STATUS_SUCCESS;

drop_nbuf:
	qdf_nbuf_free(nbuf);
	return QDF_STATUS_E_FAILURE;
}

/**
 * dp_rx_process_rxdma_err() - Function to deliver rxdma unencrypted_err
 *			       frames to OS or wifi parse errors.
 * @soc: core DP main context
 * @nbuf: buffer pointer
 * @rx_tlv_hdr: start of rx tlv header
 * @peer: peer reference
 * @err_code: rxdma err code
 * @mac_id: mac_id which is one of 3 mac_ids(Assuming mac_id and
 * pool_id has same mapping)
 *
 * Return: None
 */
void
dp_rx_process_rxdma_err(struct dp_soc *soc, qdf_nbuf_t nbuf,
			uint8_t *rx_tlv_hdr, struct dp_peer *peer,
			uint8_t err_code, uint8_t mac_id)
{
	uint32_t pkt_len, l2_hdr_offset;
	uint16_t msdu_len;
	struct dp_vdev *vdev;
	qdf_ether_header_t *eh;
	bool is_broadcast;

	/*
	 * Check if DMA completed -- msdu_done is the last bit
	 * to be written
	 */
	if (!hal_rx_attn_msdu_done_get(rx_tlv_hdr)) {

		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				FL("MSDU DONE failure"));

		hal_rx_dump_pkt_tlvs(soc->hal_soc, rx_tlv_hdr,
				     QDF_TRACE_LEVEL_INFO);
		qdf_assert(0);
	}

	l2_hdr_offset = hal_rx_msdu_end_l3_hdr_padding_get(soc->hal_soc,
							   rx_tlv_hdr);
	msdu_len = hal_rx_msdu_start_msdu_len_get(rx_tlv_hdr);
	pkt_len = msdu_len + l2_hdr_offset + RX_PKT_TLVS_LEN;

	/* Set length in nbuf */
	qdf_nbuf_set_pktlen(nbuf, pkt_len);

	qdf_nbuf_set_next(nbuf, NULL);

	qdf_nbuf_set_rx_chfrag_start(nbuf, 1);
	qdf_nbuf_set_rx_chfrag_end(nbuf, 1);

	if (!peer) {
		QDF_TRACE_ERROR_RL(QDF_MODULE_ID_DP, "peer is NULL");
		DP_STATS_INC_PKT(soc, rx.err.rx_invalid_peer, 1,
				qdf_nbuf_len(nbuf));
		/* Trigger invalid peer handler wrapper */
		dp_rx_process_invalid_peer_wrapper(soc, nbuf, true, mac_id);
		return;
	}

	vdev = peer->vdev;
	if (!vdev) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				FL("INVALID vdev %pK OR osif_rx"), vdev);
		/* Drop & free packet */
		qdf_nbuf_free(nbuf);
		DP_STATS_INC(soc, rx.err.invalid_vdev, 1);
		return;
	}

	/*
	 * Advance the packet start pointer by total size of
	 * pre-header TLV's
	 */
	qdf_nbuf_pull_head(nbuf, l2_hdr_offset + RX_PKT_TLVS_LEN);

	if (err_code == HAL_RXDMA_ERR_WIFI_PARSE) {
		uint8_t *pkt_type;

		pkt_type = qdf_nbuf_data(nbuf) + (2 * QDF_MAC_ADDR_SIZE);
		if (*(uint16_t *)pkt_type == htons(QDF_ETH_TYPE_8021Q)) {
			if (*(uint16_t *)(pkt_type + DP_SKIP_VLAN) ==
							htons(QDF_LLC_STP)) {
				DP_STATS_INC(vdev->pdev, vlan_tag_stp_cnt, 1);
				goto process_mesh;
			} else {
				goto process_rx;
			}
		}
	}
	if (vdev->rx_decap_type == htt_cmn_pkt_type_raw)
		goto process_mesh;

	/*
	 * WAPI cert AP sends rekey frames as unencrypted.
	 * Thus RXDMA will report unencrypted frame error.
	 * To pass WAPI cert case, SW needs to pass unencrypted
	 * rekey frame to stack.
	 */
	if (qdf_nbuf_is_ipv4_wapi_pkt(nbuf)) {
		goto process_rx;
	}
	/*
	 * In dynamic WEP case rekey frames are not encrypted
	 * similar to WAPI. Allow EAPOL when 8021+wep is enabled and
	 * key install is already done
	 */
	if ((vdev->sec_type == cdp_sec_type_wep104) &&
	    (qdf_nbuf_is_ipv4_eapol_pkt(nbuf)))
		goto process_rx;

process_mesh:

	if (!vdev->mesh_vdev && err_code == HAL_RXDMA_ERR_UNENCRYPTED) {
		qdf_nbuf_free(nbuf);
		DP_STATS_INC(soc, rx.err.invalid_vdev, 1);
		return;
	}

	if (vdev->mesh_vdev) {
		if (dp_rx_filter_mesh_packets(vdev, nbuf, rx_tlv_hdr)
				      == QDF_STATUS_SUCCESS) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_MED,
				  FL("mesh pkt filtered"));
			DP_STATS_INC(vdev->pdev, dropped.mesh_filter, 1);

			qdf_nbuf_free(nbuf);
			return;
		}
		dp_rx_fill_mesh_stats(vdev, nbuf, rx_tlv_hdr, peer);
	}
process_rx:
	if (qdf_unlikely(hal_rx_msdu_end_da_is_mcbc_get(soc->hal_soc,
							rx_tlv_hdr) &&
		(vdev->rx_decap_type ==
				htt_cmn_pkt_type_ethernet))) {
		eh = (qdf_ether_header_t *)qdf_nbuf_data(nbuf);
		is_broadcast = (QDF_IS_ADDR_BROADCAST
				(eh->ether_dhost)) ? 1 : 0 ;
		DP_STATS_INC_PKT(peer, rx.multicast, 1, qdf_nbuf_len(nbuf));
		if (is_broadcast) {
			DP_STATS_INC_PKT(peer, rx.bcast, 1,
					qdf_nbuf_len(nbuf));
		}
	}

	if (qdf_unlikely(vdev->rx_decap_type == htt_cmn_pkt_type_raw)) {
		dp_rx_deliver_raw(vdev, nbuf, peer);
	} else {
		/* Update the protocol tag in SKB based on CCE metadata */
		dp_rx_update_protocol_tag(soc, vdev, nbuf, rx_tlv_hdr,
					  EXCEPTION_DEST_RING_ID, true, true);
		/* Update the flow tag in SKB based on FSE metadata */
		dp_rx_update_flow_tag(soc, vdev, nbuf, rx_tlv_hdr, true);
		DP_STATS_INC(peer, rx.to_stack.num, 1);
		vdev->osif_rx(vdev->osif_vdev, nbuf);
	}

	return;
}

/**
 * dp_rx_process_mic_error(): Function to pass mic error indication to umac
 * @soc: core DP main context
 * @nbuf: buffer pointer
 * @rx_tlv_hdr: start of rx tlv header
 * @peer: peer handle
 *
 * return: void
 */
void dp_rx_process_mic_error(struct dp_soc *soc, qdf_nbuf_t nbuf,
			     uint8_t *rx_tlv_hdr, struct dp_peer *peer)
{
	struct dp_vdev *vdev = NULL;
	struct dp_pdev *pdev = NULL;
	struct ol_if_ops *tops = NULL;
	uint16_t rx_seq, fragno;
	uint8_t is_raw;
	unsigned int tid;
	QDF_STATUS status;
	struct cdp_rx_mic_err_info mic_failure_info;

	if (!hal_rx_msdu_end_first_msdu_get(soc->hal_soc,
					    rx_tlv_hdr))
		return;

	if (!peer) {
		dp_info_rl("peer not found");
		goto fail;
	}

	vdev = peer->vdev;
	if (!vdev) {
		dp_info_rl("VDEV not found");
		goto fail;
	}

	pdev = vdev->pdev;
	if (!pdev) {
		dp_info_rl("PDEV not found");
		goto fail;
	}

	is_raw = HAL_IS_DECAP_FORMAT_RAW(soc->hal_soc, qdf_nbuf_data(nbuf));
	if (is_raw) {
		fragno = dp_rx_frag_get_mpdu_frag_number(qdf_nbuf_data(nbuf));
		/* Can get only last fragment */
		if (fragno) {
			tid = hal_rx_mpdu_start_tid_get(soc->hal_soc,
							qdf_nbuf_data(nbuf));
			rx_seq = hal_rx_get_rx_sequence(soc->hal_soc,
							qdf_nbuf_data(nbuf));

			status = dp_rx_defrag_add_last_frag(soc, peer,
							    tid, rx_seq, nbuf);
			dp_info_rl("Frag pkt seq# %d frag# %d consumed "
				   "status %d !", rx_seq, fragno, status);
			return;
		}
	}

	if (hal_rx_mpdu_get_addr1(soc->hal_soc, qdf_nbuf_data(nbuf),
				  &mic_failure_info.da_mac_addr.bytes[0])) {
		dp_err_rl("Failed to get da_mac_addr");
		goto fail;
	}

	if (hal_rx_mpdu_get_addr2(soc->hal_soc, qdf_nbuf_data(nbuf),
				  &mic_failure_info.ta_mac_addr.bytes[0])) {
		dp_err_rl("Failed to get ta_mac_addr");
		goto fail;
	}

	mic_failure_info.key_id = 0;
	mic_failure_info.multicast =
		IEEE80211_IS_MULTICAST(mic_failure_info.da_mac_addr.bytes);
	qdf_mem_zero(mic_failure_info.tsc, MIC_SEQ_CTR_SIZE);
	mic_failure_info.frame_type = cdp_rx_frame_type_802_11;
	mic_failure_info.data = NULL;
	mic_failure_info.vdev_id = vdev->vdev_id;

	tops = pdev->soc->cdp_soc.ol_ops;
	if (tops->rx_mic_error)
		tops->rx_mic_error(soc->ctrl_psoc, pdev->pdev_id,
				   &mic_failure_info);

fail:
	qdf_nbuf_free(nbuf);
	return;
}

uint32_t
dp_rx_err_process(struct dp_intr *int_ctx, struct dp_soc *soc,
		  hal_ring_handle_t hal_ring_hdl, uint32_t quota)
{
	hal_ring_desc_t ring_desc;
	hal_soc_handle_t hal_soc;
	uint32_t count = 0;
	uint32_t rx_bufs_used = 0;
	uint32_t rx_bufs_reaped[MAX_PDEV_CNT] = { 0 };
	uint8_t mac_id = 0;
	uint8_t buf_type;
	uint8_t error, rbm;
	struct hal_rx_mpdu_desc_info mpdu_desc_info;
	struct hal_buf_info hbi;
	struct dp_pdev *dp_pdev;
	struct dp_srng *dp_rxdma_srng;
	struct rx_desc_pool *rx_desc_pool;
	uint32_t cookie = 0;
	void *link_desc_va;
	struct hal_rx_msdu_list msdu_list; /* MSDU's per MPDU */
	uint16_t num_msdus;
	struct dp_rx_desc *rx_desc = NULL;

	/* Debug -- Remove later */
	qdf_assert(soc && hal_ring_hdl);

	hal_soc = soc->hal_soc;

	/* Debug -- Remove later */
	qdf_assert(hal_soc);

	if (qdf_unlikely(dp_srng_access_start(int_ctx, soc, hal_ring_hdl))) {

		/* TODO */
		/*
		 * Need API to convert from hal_ring pointer to
		 * Ring Type / Ring Id combo
		 */
		DP_STATS_INC(soc, rx.err.hal_ring_access_fail, 1);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("HAL RING Access Failed -- %pK"), hal_ring_hdl);
		goto done;
	}

	while (qdf_likely(quota-- && (ring_desc =
				hal_srng_dst_get_next(hal_soc,
						      hal_ring_hdl)))) {

		DP_STATS_INC(soc, rx.err_ring_pkts, 1);

		error = HAL_RX_ERROR_STATUS_GET(ring_desc);

		qdf_assert(error == HAL_REO_ERROR_DETECTED);

		buf_type = HAL_RX_REO_BUF_TYPE_GET(ring_desc);
		/*
		 * For REO error ring, expect only MSDU LINK DESC
		 */
		qdf_assert_always(buf_type == HAL_RX_REO_MSDU_LINK_DESC_TYPE);

		cookie = HAL_RX_REO_BUF_COOKIE_GET(ring_desc);
		/*
		 * check for the magic number in the sw cookie
		 */
		qdf_assert_always((cookie >> LINK_DESC_ID_SHIFT) &
							LINK_DESC_ID_START);

		/*
		 * Check if the buffer is to be processed on this processor
		 */
		rbm = hal_rx_ret_buf_manager_get(ring_desc);

		hal_rx_reo_buf_paddr_get(ring_desc, &hbi);
		link_desc_va = dp_rx_cookie_2_link_desc_va(soc, &hbi);
		hal_rx_msdu_list_get(soc->hal_soc, link_desc_va, &msdu_list,
				     &num_msdus);

		if (qdf_unlikely((msdu_list.rbm[0] != DP_WBM2SW_RBM) &&
				(msdu_list.rbm[0] !=
					HAL_RX_BUF_RBM_WBM_IDLE_DESC_LIST) &&
				(msdu_list.rbm[0] != DP_DEFRAG_RBM))) {
			/* TODO */
			/* Call appropriate handler */
			if (!wlan_cfg_get_dp_soc_nss_cfg(soc->wlan_cfg_ctx)) {
				DP_STATS_INC(soc, rx.err.invalid_rbm, 1);
				QDF_TRACE(QDF_MODULE_ID_DP,
					  QDF_TRACE_LEVEL_ERROR,
					  FL("Invalid RBM %d"),
					     msdu_list.rbm[0]);
			}

			/* Return link descriptor through WBM ring (SW2WBM)*/
			dp_rx_link_desc_return(soc, ring_desc,
					HAL_BM_ACTION_RELEASE_MSDU_LIST);
			continue;
		}

		rx_desc = dp_rx_cookie_2_va_rxdma_buf(soc,
						      msdu_list.sw_cookie[0]);
		qdf_assert_always(rx_desc);

		mac_id = rx_desc->pool_id;

		/* Get the MPDU DESC info */
		hal_rx_mpdu_desc_info_get(ring_desc, &mpdu_desc_info);

		if (mpdu_desc_info.mpdu_flags & HAL_MPDU_F_FRAGMENT) {
			/*
			 * We only handle one msdu per link desc for fragmented
			 * case. We drop the msdus and release the link desc
			 * back if there are more than one msdu in link desc.
			 */
			if (qdf_unlikely(num_msdus > 1)) {
				count = dp_rx_msdus_drop(soc, ring_desc,
							 &mpdu_desc_info,
							 &mac_id, quota);
				rx_bufs_reaped[mac_id] += count;
				continue;
			}

			count = dp_rx_frag_handle(soc,
						  ring_desc, &mpdu_desc_info,
						  rx_desc, &mac_id, quota);

			rx_bufs_reaped[mac_id] += count;
			DP_STATS_INC(soc, rx.rx_frags, 1);
			continue;
		}

		if (hal_rx_reo_is_pn_error(ring_desc)) {
			/* TOD0 */
			DP_STATS_INC(soc,
				rx.err.
				reo_error[HAL_REO_ERR_PN_CHECK_FAILED],
				1);
			/* increment @pdev level */
			dp_pdev = dp_get_pdev_for_lmac_id(soc, mac_id);
			if (dp_pdev)
				DP_STATS_INC(dp_pdev, err.reo_error, 1);
			count = dp_rx_pn_error_handle(soc,
						      ring_desc,
						      &mpdu_desc_info, &mac_id,
						      quota);

			rx_bufs_reaped[mac_id] += count;
			continue;
		}

		if (hal_rx_reo_is_2k_jump(ring_desc)) {
			/* TOD0 */
			DP_STATS_INC(soc,
				rx.err.
				reo_error[HAL_REO_ERR_REGULAR_FRAME_2K_JUMP],
				1);
			/* increment @pdev level */
			dp_pdev = dp_get_pdev_for_lmac_id(soc, mac_id);
			if (dp_pdev)
				DP_STATS_INC(dp_pdev, err.reo_error, 1);

			count = dp_rx_2k_jump_handle(soc,
						     ring_desc, &mpdu_desc_info,
						     &mac_id, quota);

			rx_bufs_reaped[mac_id] += count;
			continue;
		}
	}

done:
	dp_srng_access_end(int_ctx, soc, hal_ring_hdl);

	if (soc->rx.flags.defrag_timeout_check) {
		uint32_t now_ms =
			qdf_system_ticks_to_msecs(qdf_system_ticks());

		if (now_ms >= soc->rx.defrag.next_flush_ms)
			dp_rx_defrag_waitlist_flush(soc);
	}

	for (mac_id = 0; mac_id < MAX_PDEV_CNT; mac_id++) {
		if (rx_bufs_reaped[mac_id]) {
			dp_pdev = dp_get_pdev_for_lmac_id(soc, mac_id);
			dp_rxdma_srng = &soc->rx_refill_buf_ring[mac_id];
			rx_desc_pool = &soc->rx_desc_buf[mac_id];

			dp_rx_buffers_replenish(soc, mac_id, dp_rxdma_srng,
						rx_desc_pool,
						rx_bufs_reaped[mac_id],
						&dp_pdev->free_list_head,
						&dp_pdev->free_list_tail);
			rx_bufs_used += rx_bufs_reaped[mac_id];
		}
	}

	return rx_bufs_used; /* Assume no scale factor for now */
}

uint32_t
dp_rx_wbm_err_process(struct dp_intr *int_ctx, struct dp_soc *soc,
		      hal_ring_handle_t hal_ring_hdl, uint32_t quota)
{
	hal_ring_desc_t ring_desc;
	hal_soc_handle_t hal_soc;
	struct dp_rx_desc *rx_desc;
	union dp_rx_desc_list_elem_t *head[MAX_PDEV_CNT] = { NULL };
	union dp_rx_desc_list_elem_t *tail[MAX_PDEV_CNT] = { NULL };
	uint32_t rx_bufs_used = 0;
	uint32_t rx_bufs_reaped[MAX_PDEV_CNT] = { 0 };
	uint8_t buf_type, rbm;
	uint32_t rx_buf_cookie;
	uint8_t mac_id;
	struct dp_pdev *dp_pdev;
	struct dp_srng *dp_rxdma_srng;
	struct rx_desc_pool *rx_desc_pool;
	uint8_t *rx_tlv_hdr;
	qdf_nbuf_t nbuf_head = NULL;
	qdf_nbuf_t nbuf_tail = NULL;
	qdf_nbuf_t nbuf, next;
	struct hal_wbm_err_desc_info wbm_err_info = { 0 };
	uint8_t pool_id;
	uint8_t tid = 0;

	/* Debug -- Remove later */
	qdf_assert(soc && hal_ring_hdl);

	hal_soc = soc->hal_soc;

	/* Debug -- Remove later */
	qdf_assert(hal_soc);

	if (qdf_unlikely(dp_srng_access_start(int_ctx, soc, hal_ring_hdl))) {

		/* TODO */
		/*
		 * Need API to convert from hal_ring pointer to
		 * Ring Type / Ring Id combo
		 */
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("HAL RING Access Failed -- %pK"), hal_ring_hdl);
		goto done;
	}

	while (qdf_likely(quota-- && (ring_desc =
				hal_srng_dst_get_next(hal_soc,
						      hal_ring_hdl)))) {

		/* XXX */
		buf_type = HAL_RX_WBM_BUF_TYPE_GET(ring_desc);

		/*
		 * For WBM ring, expect only MSDU buffers
		 */
		qdf_assert_always(buf_type == HAL_RX_WBM_BUF_TYPE_REL_BUF);

		qdf_assert((HAL_RX_WBM_ERR_SRC_GET(ring_desc)
				== HAL_RX_WBM_ERR_SRC_RXDMA) ||
				(HAL_RX_WBM_ERR_SRC_GET(ring_desc)
				== HAL_RX_WBM_ERR_SRC_REO));

		/*
		 * Check if the buffer is to be processed on this processor
		 */
		rbm = hal_rx_ret_buf_manager_get(ring_desc);

		if (qdf_unlikely(rbm != HAL_RX_BUF_RBM_SW3_BM)) {
			/* TODO */
			/* Call appropriate handler */
			DP_STATS_INC(soc, rx.err.invalid_rbm, 1);
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				FL("Invalid RBM %d"), rbm);
			continue;
		}

		rx_buf_cookie =	HAL_RX_WBM_BUF_COOKIE_GET(ring_desc);

		rx_desc = dp_rx_cookie_2_va_rxdma_buf(soc, rx_buf_cookie);
		qdf_assert_always(rx_desc);

		if (!dp_rx_desc_check_magic(rx_desc)) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
					FL("Invalid rx_desc cookie=%d"),
					rx_buf_cookie);
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
			DP_STATS_INC(soc, rx.err.hal_wbm_rel_dup, 1);
			dp_rx_dump_info_and_assert(soc, hal_ring_hdl,
						   ring_desc, rx_desc);
		}

		nbuf = rx_desc->nbuf;
		qdf_nbuf_unmap_single(soc->osdev, nbuf,	QDF_DMA_FROM_DEVICE);

		/*
		 * save the wbm desc info in nbuf TLV. We will need this
		 * info when we do the actual nbuf processing
		 */
		hal_rx_wbm_err_info_get(ring_desc, &wbm_err_info, hal_soc);
		wbm_err_info.pool_id = rx_desc->pool_id;
		hal_rx_wbm_err_info_set_in_tlv(qdf_nbuf_data(nbuf),
								&wbm_err_info);

		rx_bufs_reaped[rx_desc->pool_id]++;

		DP_RX_LIST_APPEND(nbuf_head, nbuf_tail, rx_desc->nbuf);
		dp_rx_add_to_free_desc_list(&head[rx_desc->pool_id],
						&tail[rx_desc->pool_id],
						rx_desc);
	}
done:
	dp_srng_access_end(int_ctx, soc, hal_ring_hdl);

	for (mac_id = 0; mac_id < MAX_PDEV_CNT; mac_id++) {
		if (rx_bufs_reaped[mac_id]) {
			dp_pdev = dp_get_pdev_for_lmac_id(soc, mac_id);
			dp_rxdma_srng = &soc->rx_refill_buf_ring[mac_id];
			rx_desc_pool = &soc->rx_desc_buf[mac_id];

			dp_rx_buffers_replenish(soc, mac_id, dp_rxdma_srng,
					rx_desc_pool, rx_bufs_reaped[mac_id],
					&head[mac_id], &tail[mac_id]);
			rx_bufs_used += rx_bufs_reaped[mac_id];
		}
	}

	nbuf = nbuf_head;
	while (nbuf) {
		struct dp_peer *peer;
		uint16_t peer_id;
		uint8_t e_code;
		uint8_t *tlv_hdr;
		rx_tlv_hdr = qdf_nbuf_data(nbuf);

		/*
		 * retrieve the wbm desc info from nbuf TLV, so we can
		 * handle error cases appropriately
		 */
		hal_rx_wbm_err_info_get_from_tlv(rx_tlv_hdr, &wbm_err_info);

		peer_id = hal_rx_mpdu_start_sw_peer_id_get(soc->hal_soc,
							   rx_tlv_hdr);
		peer = dp_peer_find_by_id(soc, peer_id);

		if (!peer)
			dp_info_rl("peer is null peer_id%u err_src%u err_rsn%u",
				   peer_id, wbm_err_info.wbm_err_src,
				   wbm_err_info.reo_psh_rsn);

		/* Set queue_mapping in nbuf to 0 */
		dp_set_rx_queue(nbuf, 0);

		next = nbuf->next;

		if (wbm_err_info.wbm_err_src == HAL_RX_WBM_ERR_SRC_REO) {
			if (wbm_err_info.reo_psh_rsn
				== HAL_RX_WBM_REO_PSH_RSN_ERROR) {

				DP_STATS_INC(soc,
					rx.err.reo_error
					[wbm_err_info.reo_err_code], 1);
				/* increment @pdev level */
				pool_id = wbm_err_info.pool_id;
				dp_pdev = dp_get_pdev_for_lmac_id(soc, pool_id);
				if (dp_pdev)
					DP_STATS_INC(dp_pdev, err.reo_error,
						     1);

				switch (wbm_err_info.reo_err_code) {
				/*
				 * Handling for packets which have NULL REO
				 * queue descriptor
				 */
				case HAL_REO_ERR_QUEUE_DESC_ADDR_0:
					pool_id = wbm_err_info.pool_id;
					dp_rx_null_q_desc_handle(soc, nbuf,
								 rx_tlv_hdr,
								 pool_id, peer);
					nbuf = next;
					if (peer)
						dp_peer_unref_del_find_by_id(
									peer);
					continue;
				/* TODO */
				/* Add per error code accounting */
				case HAL_REO_ERR_REGULAR_FRAME_2K_JUMP:
					pool_id = wbm_err_info.pool_id;

					if (hal_rx_msdu_end_first_msdu_get(soc->hal_soc,
									   rx_tlv_hdr)) {
						peer_id =
						hal_rx_mpdu_start_sw_peer_id_get(soc->hal_soc,
										 rx_tlv_hdr);
						tid =
						hal_rx_mpdu_start_tid_get(hal_soc, rx_tlv_hdr);
					}
					dp_2k_jump_handle(soc, nbuf, rx_tlv_hdr,
							  peer_id, tid);
					nbuf = next;
					if (peer)
						dp_peer_unref_del_find_by_id(
									peer);
					continue;
				case HAL_REO_ERR_BAR_FRAME_2K_JUMP:
				case HAL_REO_ERR_BAR_FRAME_OOR:
					if (peer)
						dp_rx_wbm_err_handle_bar(soc,
									 peer,
									 nbuf);
					break;

				default:
					dp_info_rl("Got pkt with REO ERROR: %d",
						   wbm_err_info.reo_err_code);
					break;
				}
			}
		} else if (wbm_err_info.wbm_err_src ==
					HAL_RX_WBM_ERR_SRC_RXDMA) {
			if (wbm_err_info.rxdma_psh_rsn
					== HAL_RX_WBM_RXDMA_PSH_RSN_ERROR) {
				DP_STATS_INC(soc,
					rx.err.rxdma_error
					[wbm_err_info.rxdma_err_code], 1);
				/* increment @pdev level */
				pool_id = wbm_err_info.pool_id;
				dp_pdev = dp_get_pdev_for_lmac_id(soc, pool_id);
				if (dp_pdev)
					DP_STATS_INC(dp_pdev,
						     err.rxdma_error, 1);

				switch (wbm_err_info.rxdma_err_code) {
				case HAL_RXDMA_ERR_UNENCRYPTED:

				case HAL_RXDMA_ERR_WIFI_PARSE:
					pool_id = wbm_err_info.pool_id;
					dp_rx_process_rxdma_err(soc, nbuf,
								rx_tlv_hdr,
								peer,
								wbm_err_info.
								rxdma_err_code,
								pool_id);
					nbuf = next;
					if (peer)
						dp_peer_unref_del_find_by_id(peer);
					continue;

				case HAL_RXDMA_ERR_TKIP_MIC:
					dp_rx_process_mic_error(soc, nbuf,
								rx_tlv_hdr,
								peer);
					nbuf = next;
					if (peer) {
						DP_STATS_INC(peer, rx.err.mic_err, 1);
						dp_peer_unref_del_find_by_id(
									peer);
					}
					continue;

				case HAL_RXDMA_ERR_DECRYPT:
					pool_id = wbm_err_info.pool_id;
					e_code = wbm_err_info.rxdma_err_code;
					tlv_hdr = rx_tlv_hdr;
					if (peer) {
						DP_STATS_INC(peer, rx.err.
							     decrypt_err, 1);
					} else {
						dp_rx_process_rxdma_err(soc,
									nbuf,
									tlv_hdr,
									NULL,
									e_code,
									pool_id
									);
						nbuf = next;
						continue;
					}

					QDF_TRACE(QDF_MODULE_ID_DP,
						QDF_TRACE_LEVEL_DEBUG,
					"Packet received with Decrypt error");
					break;

				default:
					dp_err_rl("RXDMA error %d",
						  wbm_err_info.rxdma_err_code);
				}
			}
		} else {
			/* Should not come here */
			qdf_assert(0);
		}

		if (peer)
			dp_peer_unref_del_find_by_id(peer);

		hal_rx_dump_pkt_tlvs(hal_soc, rx_tlv_hdr,
				     QDF_TRACE_LEVEL_DEBUG);
		qdf_nbuf_free(nbuf);
		nbuf = next;
	}
	return rx_bufs_used; /* Assume no scale factor for now */
}

/**
 * dup_desc_dbg() - dump and assert if duplicate rx desc found
 *
 * @soc: core DP main context
 * @rxdma_dst_ring_desc: void pointer to monitor link descriptor buf addr info
 * @rx_desc: void pointer to rx descriptor
 *
 * Return: void
 */
static void dup_desc_dbg(struct dp_soc *soc,
			 hal_rxdma_desc_t rxdma_dst_ring_desc,
			 void *rx_desc)
{
	DP_STATS_INC(soc, rx.err.hal_rxdma_err_dup, 1);
	dp_rx_dump_info_and_assert(
			soc,
			soc->rx_rel_ring.hal_srng,
			hal_rxdma_desc_to_hal_ring_desc(rxdma_dst_ring_desc),
			rx_desc);
}

/**
 * dp_rx_err_mpdu_pop() - extract the MSDU's from link descs
 *
 * @soc: core DP main context
 * @mac_id: mac id which is one of 3 mac_ids
 * @rxdma_dst_ring_desc: void pointer to monitor link descriptor buf addr info
 * @head: head of descs list to be freed
 * @tail: tail of decs list to be freed

 * Return: number of msdu in MPDU to be popped
 */
static inline uint32_t
dp_rx_err_mpdu_pop(struct dp_soc *soc, uint32_t mac_id,
	hal_rxdma_desc_t rxdma_dst_ring_desc,
	union dp_rx_desc_list_elem_t **head,
	union dp_rx_desc_list_elem_t **tail)
{
	void *rx_msdu_link_desc;
	qdf_nbuf_t msdu;
	qdf_nbuf_t last;
	struct hal_rx_msdu_list msdu_list;
	uint16_t num_msdus;
	struct hal_buf_info buf_info;
	uint32_t rx_bufs_used = 0;
	uint32_t msdu_cnt;
	uint32_t i;
	uint8_t push_reason;
	uint8_t rxdma_error_code = 0;
	uint8_t bm_action = HAL_BM_ACTION_PUT_IN_IDLE_LIST;
	struct dp_pdev *pdev = dp_get_pdev_for_lmac_id(soc, mac_id);
	uint32_t rx_link_buf_info[HAL_RX_BUFFINFO_NUM_DWORDS];
	hal_rxdma_desc_t ring_desc;

	msdu = 0;

	last = NULL;

	hal_rx_reo_ent_buf_paddr_get(rxdma_dst_ring_desc, &buf_info,
					&msdu_cnt);

	push_reason =
		hal_rx_reo_ent_rxdma_push_reason_get(rxdma_dst_ring_desc);
	if (push_reason == HAL_RX_WBM_RXDMA_PSH_RSN_ERROR) {
		rxdma_error_code =
			hal_rx_reo_ent_rxdma_error_code_get(rxdma_dst_ring_desc);
	}

	do {
		rx_msdu_link_desc =
			dp_rx_cookie_2_link_desc_va(soc, &buf_info);

		qdf_assert(rx_msdu_link_desc);

		hal_rx_msdu_list_get(soc->hal_soc, rx_msdu_link_desc,
				     &msdu_list, &num_msdus);

		if (msdu_list.sw_cookie[0] != HAL_RX_COOKIE_SPECIAL) {
			/* if the msdus belongs to NSS offloaded radio &&
			 * the rbm is not SW1_BM then return the msdu_link
			 * descriptor without freeing the msdus (nbufs). let
			 * these buffers be given to NSS completion ring for
			 * NSS to free them.
			 * else iterate through the msdu link desc list and
			 * free each msdu in the list.
			 */
			if (msdu_list.rbm[0] != HAL_RX_BUF_RBM_SW3_BM &&
				wlan_cfg_get_dp_pdev_nss_enabled(
							  pdev->wlan_cfg_ctx))
				bm_action = HAL_BM_ACTION_RELEASE_MSDU_LIST;
			else {
				for (i = 0; i < num_msdus; i++) {
					struct dp_rx_desc *rx_desc =
						dp_rx_cookie_2_va_rxdma_buf(soc,
							msdu_list.sw_cookie[i]);
					qdf_assert_always(rx_desc);
					msdu = rx_desc->nbuf;
					/*
					 * this is a unlikely scenario
					 * where the host is reaping
					 * a descriptor which
					 * it already reaped just a while ago
					 * but is yet to replenish
					 * it back to HW.
					 * In this case host will dump
					 * the last 128 descriptors
					 * including the software descriptor
					 * rx_desc and assert.
					 */
					ring_desc = rxdma_dst_ring_desc;
					if (qdf_unlikely(!rx_desc->in_use)) {
						dup_desc_dbg(soc,
							     ring_desc,
							     rx_desc);
						continue;
					}

					qdf_nbuf_unmap_single(soc->osdev, msdu,
						QDF_DMA_FROM_DEVICE);

					QDF_TRACE(QDF_MODULE_ID_DP,
						QDF_TRACE_LEVEL_DEBUG,
						"[%s][%d] msdu_nbuf=%pK ",
						__func__, __LINE__, msdu);

					qdf_nbuf_free(msdu);
					rx_bufs_used++;
					dp_rx_add_to_free_desc_list(head,
						tail, rx_desc);
				}
			}
		} else {
			rxdma_error_code = HAL_RXDMA_ERR_WAR;
		}

		/*
		 * Store the current link buffer into to the local structure
		 * to be used for release purpose.
		 */
		hal_rxdma_buff_addr_info_set(rx_link_buf_info, buf_info.paddr,
					     buf_info.sw_cookie, buf_info.rbm);

		hal_rx_mon_next_link_desc_get(rx_msdu_link_desc, &buf_info);
		dp_rx_link_desc_return_by_addr(soc,
					       (hal_buff_addrinfo_t)
						rx_link_buf_info,
						bm_action);
	} while (buf_info.paddr);

	DP_STATS_INC(soc, rx.err.rxdma_error[rxdma_error_code], 1);
	if (pdev)
		DP_STATS_INC(pdev, err.rxdma_error, 1);

	if (rxdma_error_code == HAL_RXDMA_ERR_DECRYPT) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"Packet received with Decrypt error");
	}

	return rx_bufs_used;
}

uint32_t
dp_rxdma_err_process(struct dp_intr *int_ctx, struct dp_soc *soc,
		     uint32_t mac_id, uint32_t quota)
{
	struct dp_pdev *pdev = dp_get_pdev_for_lmac_id(soc, mac_id);
	hal_rxdma_desc_t rxdma_dst_ring_desc;
	hal_soc_handle_t hal_soc;
	void *err_dst_srng;
	union dp_rx_desc_list_elem_t *head = NULL;
	union dp_rx_desc_list_elem_t *tail = NULL;
	struct dp_srng *dp_rxdma_srng;
	struct rx_desc_pool *rx_desc_pool;
	uint32_t work_done = 0;
	uint32_t rx_bufs_used = 0;

	if (!pdev)
		return 0;

	err_dst_srng = soc->rxdma_err_dst_ring[mac_id].hal_srng;

	if (!err_dst_srng) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"%s %d : HAL Monitor Destination Ring Init \
			Failed -- %pK",
			__func__, __LINE__, err_dst_srng);
		return 0;
	}

	hal_soc = soc->hal_soc;

	qdf_assert(hal_soc);

	if (qdf_unlikely(dp_srng_access_start(int_ctx, soc, err_dst_srng))) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"%s %d : HAL Monitor Destination Ring Init \
			Failed -- %pK",
			__func__, __LINE__, err_dst_srng);
		return 0;
	}

	while (qdf_likely(quota-- && (rxdma_dst_ring_desc =
		hal_srng_dst_get_next(hal_soc, err_dst_srng)))) {

			rx_bufs_used += dp_rx_err_mpdu_pop(soc, mac_id,
						rxdma_dst_ring_desc,
						&head, &tail);
	}

	dp_srng_access_end(int_ctx, soc, err_dst_srng);

	if (rx_bufs_used) {
		dp_rxdma_srng = &soc->rx_refill_buf_ring[mac_id];
		rx_desc_pool = &soc->rx_desc_buf[mac_id];

		dp_rx_buffers_replenish(soc, mac_id, dp_rxdma_srng,
			rx_desc_pool, rx_bufs_used, &head, &tail);

		work_done += rx_bufs_used;
	}

	return work_done;
}

static inline uint32_t
dp_wbm_int_err_mpdu_pop(struct dp_soc *soc, uint32_t mac_id,
			hal_rxdma_desc_t rxdma_dst_ring_desc,
			union dp_rx_desc_list_elem_t **head,
			union dp_rx_desc_list_elem_t **tail)
{
	void *rx_msdu_link_desc;
	qdf_nbuf_t msdu;
	qdf_nbuf_t last;
	struct hal_rx_msdu_list msdu_list;
	uint16_t num_msdus;
	struct hal_buf_info buf_info;
	uint32_t rx_bufs_used = 0, msdu_cnt, i;
	uint32_t rx_link_buf_info[HAL_RX_BUFFINFO_NUM_DWORDS];

	msdu = 0;

	last = NULL;

	hal_rx_reo_ent_buf_paddr_get(rxdma_dst_ring_desc, &buf_info,
				     &msdu_cnt);

	do {
		rx_msdu_link_desc =
			dp_rx_cookie_2_link_desc_va(soc, &buf_info);

		if (!rx_msdu_link_desc) {
			DP_STATS_INC(soc, tx.wbm_internal_error[WBM_INT_ERROR_REO_NULL_LINK_DESC], 1);
			break;
		}

		hal_rx_msdu_list_get(soc->hal_soc, rx_msdu_link_desc,
				     &msdu_list, &num_msdus);

		if (msdu_list.sw_cookie[0] != HAL_RX_COOKIE_SPECIAL) {
			for (i = 0; i < num_msdus; i++) {
				struct dp_rx_desc *rx_desc =
					dp_rx_cookie_2_va_rxdma_buf(
							soc,
							msdu_list.sw_cookie[i]);
				qdf_assert_always(rx_desc);
				msdu = rx_desc->nbuf;

				qdf_nbuf_unmap_single(soc->osdev, msdu,
						      QDF_DMA_FROM_DEVICE);

				qdf_nbuf_free(msdu);
				rx_bufs_used++;
				dp_rx_add_to_free_desc_list(head,
							    tail, rx_desc);
			}
		}

		/*
		 * Store the current link buffer into to the local structure
		 * to be used for release purpose.
		 */
		hal_rxdma_buff_addr_info_set(rx_link_buf_info, buf_info.paddr,
					     buf_info.sw_cookie, buf_info.rbm);

		hal_rx_mon_next_link_desc_get(rx_msdu_link_desc, &buf_info);
		dp_rx_link_desc_return_by_addr(soc, (hal_buff_addrinfo_t)
					rx_link_buf_info,
				       HAL_BM_ACTION_PUT_IN_IDLE_LIST);
	} while (buf_info.paddr);

	return rx_bufs_used;
}

/*
 *
 * dp_handle_wbm_internal_error() - handles wbm_internal_error case
 *
 * @soc: core DP main context
 * @hal_desc: hal descriptor
 * @buf_type: indicates if the buffer is of type link disc or msdu
 * Return: None
 *
 * wbm_internal_error is seen in following scenarios :
 *
 * 1.  Null pointers detected in WBM_RELEASE_RING descriptors
 * 2.  Null pointers detected during delinking process
 *
 * Some null pointer cases:
 *
 * a. MSDU buffer pointer is NULL
 * b. Next_MSDU_Link_Desc pointer is NULL, with no last msdu flag
 * c. MSDU buffer pointer is NULL or Next_Link_Desc pointer is NULL
 */
void
dp_handle_wbm_internal_error(struct dp_soc *soc, void *hal_desc,
			     uint32_t buf_type)
{
	struct hal_buf_info buf_info = {0};
	struct dp_pdev *dp_pdev;
	struct dp_rx_desc *rx_desc = NULL;
	uint32_t rx_buf_cookie;
	uint32_t rx_bufs_reaped = 0;
	union dp_rx_desc_list_elem_t *head = NULL;
	union dp_rx_desc_list_elem_t *tail = NULL;
	uint8_t pool_id;

	hal_rx_reo_buf_paddr_get(hal_desc, &buf_info);

	if (!buf_info.paddr) {
		DP_STATS_INC(soc, tx.wbm_internal_error[WBM_INT_ERROR_REO_NULL_BUFFER], 1);
		return;
	}

	rx_buf_cookie = HAL_RX_REO_BUF_COOKIE_GET(hal_desc);
	pool_id = DP_RX_DESC_COOKIE_POOL_ID_GET(rx_buf_cookie);

	if (buf_type == HAL_WBM_RELEASE_RING_2_BUFFER_TYPE) {
		DP_STATS_INC(soc, tx.wbm_internal_error[WBM_INT_ERROR_REO_NULL_MSDU_BUFF], 1);
		rx_desc = dp_rx_cookie_2_va_rxdma_buf(soc, rx_buf_cookie);

		if (rx_desc && rx_desc->nbuf) {
			qdf_nbuf_unmap_single(soc->osdev, rx_desc->nbuf,
					      QDF_DMA_FROM_DEVICE);

			rx_desc->unmapped = 1;

			qdf_nbuf_free(rx_desc->nbuf);
			dp_rx_add_to_free_desc_list(&head,
						    &tail,
						    rx_desc);

			rx_bufs_reaped++;
		}
	} else if (buf_type == HAL_WBM_RELEASE_RING_2_DESC_TYPE) {
		rx_bufs_reaped = dp_wbm_int_err_mpdu_pop(soc, pool_id,
							 hal_desc,
							 &head, &tail);
	}

	if (rx_bufs_reaped) {
		struct rx_desc_pool *rx_desc_pool;
		struct dp_srng *dp_rxdma_srng;

		DP_STATS_INC(soc, tx.wbm_internal_error[WBM_INT_ERROR_REO_BUFF_REAPED], 1);
		dp_pdev = soc->pdev_list[pool_id];
		dp_rxdma_srng = &soc->rx_refill_buf_ring[pool_id];
		rx_desc_pool = &soc->rx_desc_buf[pool_id];

		dp_rx_buffers_replenish(soc, pool_id, dp_rxdma_srng,
					rx_desc_pool,
					rx_bufs_reaped,
					&head, &tail);
	}
}
