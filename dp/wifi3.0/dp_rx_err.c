/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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

#include "dp_types.h"
#include "dp_rx.h"
#include "dp_peer.h"
#include "dp_internal.h"
#include "hal_api.h"
#include "qdf_trace.h"
#include "qdf_nbuf.h"
#ifdef CONFIG_MCL
#include <cds_ieee80211_common.h>
#else
#include <linux/ieee80211.h>
#endif
#include "dp_rx_defrag.h"
#include <enet.h>	/* LLC_SNAP_HDR_LEN */

#ifdef RX_DESC_DEBUG_CHECK
static inline bool dp_rx_desc_check_magic(struct dp_rx_desc *rx_desc)
{
	if (qdf_unlikely(rx_desc->magic != DP_RX_DESC_MAGIC)) {
		return false;
	}
	rx_desc->magic = 0;
	return true;
}
#else
static inline bool dp_rx_desc_check_magic(struct dp_rx_desc *rx_desc)
{
	return true;
}
#endif

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
	struct dp_ast_entry *ase;
	uint16_t sa_idx = 0;
	uint8_t *data;

	/*
	 * Multicast Echo Check is required only if vdev is STA and
	 * received pkt is a multicast/broadcast pkt. otherwise
	 * skip the MEC check.
	 */
	if (vdev->opmode != wlan_op_mode_sta)
		return false;

	if (!hal_rx_msdu_end_da_is_mcbc_get(rx_tlv_hdr))
		return false;

	data = qdf_nbuf_data(nbuf);
	/*
	 * if the received pkts src mac addr matches with vdev
	 * mac address then drop the pkt as it is looped back
	 */
	if (!(qdf_mem_cmp(&data[DP_MAC_ADDR_LEN],
			vdev->mac_addr.raw,
			DP_MAC_ADDR_LEN)))
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
	if (hal_rx_msdu_end_sa_is_valid_get(rx_tlv_hdr)) {
		sa_idx = hal_rx_msdu_end_sa_idx_get(rx_tlv_hdr);

		if ((sa_idx < 0) ||
				(sa_idx >= (WLAN_UMAC_PSOC_MAX_PEERS * 2))) {
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
			ase = dp_peer_ast_hash_find(soc,
							&data[DP_MAC_ADDR_LEN]);

		}
	} else
		ase = dp_peer_ast_hash_find(soc, &data[DP_MAC_ADDR_LEN]);

	if (ase) {
		ase->ast_idx = sa_idx;
		soc->ast_table[sa_idx] = ase;

		if (ase->pdev_id != vdev->pdev->pdev_id) {
			qdf_spin_unlock_bh(&soc->ast_lock);
			QDF_TRACE(QDF_MODULE_ID_DP,
				QDF_TRACE_LEVEL_INFO,
				"Detected DBDC Root AP %pM, %d %d",
				&data[DP_MAC_ADDR_LEN], vdev->pdev->pdev_id,
				ase->pdev_id);
			return false;
		}

		if ((ase->type == CDP_TXRX_AST_TYPE_MEC) ||
				(ase->peer != peer)) {
			qdf_spin_unlock_bh(&soc->ast_lock);
			QDF_TRACE(QDF_MODULE_ID_DP,
				QDF_TRACE_LEVEL_INFO,
				"received pkt with same src mac %pM",
				&data[DP_MAC_ADDR_LEN]);

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
dp_rx_link_desc_return_by_addr(struct dp_soc *soc, void *link_desc_addr,
					uint8_t bm_action)
{
	struct dp_srng *wbm_desc_rel_ring = &soc->wbm_desc_rel_ring;
	void *wbm_rel_srng = wbm_desc_rel_ring->hal_srng;
	void *hal_soc = soc->hal_soc;
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
dp_rx_link_desc_return(struct dp_soc *soc, void *ring_desc, uint8_t bm_action)
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
static uint32_t dp_rx_msdus_drop(struct dp_soc *soc, void *ring_desc,
		struct hal_rx_mpdu_desc_info *mpdu_desc_info,
		union dp_rx_desc_list_elem_t **head,
		union dp_rx_desc_list_elem_t **tail,
		uint32_t quota)
{
	uint32_t rx_bufs_used = 0;
	void *link_desc_va;
	struct hal_buf_info buf_info;
	struct hal_rx_msdu_list msdu_list; /* MSDU's per MPDU */
	int i;
	uint8_t *rx_tlv_hdr;
	uint32_t tid;

	hal_rx_reo_buf_paddr_get(ring_desc, &buf_info);

	link_desc_va = dp_rx_cookie_2_link_desc_va(soc, &buf_info);

	/* No UNMAP required -- this is "malloc_consistent" memory */
	hal_rx_msdu_list_get(link_desc_va, &msdu_list,
		&mpdu_desc_info->msdu_count);

	for (i = 0; (i < mpdu_desc_info->msdu_count) && quota--; i++) {
		struct dp_rx_desc *rx_desc =
			dp_rx_cookie_2_va_rxdma_buf(soc,
			msdu_list.sw_cookie[i]);

		qdf_assert(rx_desc);

		if (!dp_rx_desc_check_magic(rx_desc)) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
					FL("Invalid rx_desc cookie=%d"),
					msdu_list.sw_cookie[i]);
			return rx_bufs_used;
		}

		rx_bufs_used++;
		tid = hal_rx_mpdu_start_tid_get(rx_desc->rx_buf_start);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"Packet received with PN error for tid :%d", tid);

		rx_tlv_hdr = qdf_nbuf_data(rx_desc->nbuf);
		if (hal_rx_encryption_info_valid(rx_tlv_hdr))
			hal_rx_print_pn(rx_tlv_hdr);

		/* Just free the buffers */
		qdf_nbuf_free(rx_desc->nbuf);

		dp_rx_add_to_free_desc_list(head, tail, rx_desc);
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
dp_rx_pn_error_handle(struct dp_soc *soc, void *ring_desc,
		      struct hal_rx_mpdu_desc_info *mpdu_desc_info,
		      union dp_rx_desc_list_elem_t **head,
		      union dp_rx_desc_list_elem_t **tail,
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
			"(%02x:%02x:%02x:%02x:%02x:%02x)\n",
			peer,
			peer->mac_addr.raw[0], peer->mac_addr.raw[1],
			peer->mac_addr.raw[2], peer->mac_addr.raw[3],
			peer->mac_addr.raw[4], peer->mac_addr.raw[5]);

	}
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
		"Packet received with PN error");

	/* No peer PN policy -- definitely drop */
	if (!peer_pn_policy)
		rx_bufs_used = dp_rx_msdus_drop(soc, ring_desc,
						mpdu_desc_info,
						head, tail, quota);

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
dp_rx_2k_jump_handle(struct dp_soc *soc, void *ring_desc,
		     struct hal_rx_mpdu_desc_info *mpdu_desc_info,
		     union dp_rx_desc_list_elem_t **head,
		     union dp_rx_desc_list_elem_t **tail,
		     uint32_t quota)
{
	return dp_rx_msdus_drop(soc, ring_desc, mpdu_desc_info,
				head, tail, quota);
}

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
dp_rx_chain_msdus(struct dp_soc *soc, qdf_nbuf_t nbuf, uint8_t *rx_tlv_hdr,
								uint8_t mac_id)
{
	bool mpdu_done = false;
	qdf_nbuf_t curr_nbuf = NULL;
	qdf_nbuf_t tmp_nbuf = NULL;

	/* TODO: Currently only single radio is supported, hence
	 * pdev hard coded to '0' index
	 */
	struct dp_pdev *dp_pdev = soc->pdev_list[mac_id];

	if (!dp_pdev->first_nbuf) {
		qdf_nbuf_set_rx_chfrag_start(nbuf, 1);
		dp_pdev->ppdu_id = HAL_RX_HW_DESC_GET_PPDUID_GET(rx_tlv_hdr);
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

		hal_rx_mon_hw_desc_get_mpdu_status(rx_tlv_hdr,
				&(dp_pdev->ppdu_info.rx_status));

	}

	if (dp_pdev->ppdu_id == hal_rx_attn_phy_ppdu_id_get(rx_tlv_hdr) &&
	    hal_rx_attn_msdu_done_get(rx_tlv_hdr)) {
		qdf_nbuf_set_rx_chfrag_end(nbuf, 1);
		qdf_assert_always(dp_pdev->first_nbuf == true);
		dp_pdev->first_nbuf = false;
		mpdu_done = true;
	}

	DP_RX_LIST_APPEND(dp_pdev->invalid_peer_head_msdu,
				dp_pdev->invalid_peer_tail_msdu,
				nbuf);

	return mpdu_done;
}

/**
 * dp_rx_null_q_desc_handle() - Function to handle NULL Queue
 *                              descriptor violation on either a
 *                              REO or WBM ring
 *
 * @soc: core DP main context
 * @nbuf: buffer pointer
 * @rx_tlv_hdr: start of rx tlv header
 * @pool_id: mac id
 *
 * This function handles NULL queue descriptor violations arising out
 * a missing REO queue for a given peer or a given TID. This typically
 * may happen if a packet is received on a QOS enabled TID before the
 * ADDBA negotiation for that TID, when the TID queue is setup. Or
 * it may also happen for MC/BC frames if they are not routed to the
 * non-QOS TID queue, in the absence of any other default TID queue.
 * This error can show up both in a REO destination or WBM release ring.
 *
 */
static void
dp_rx_null_q_desc_handle(struct dp_soc *soc,
			qdf_nbuf_t nbuf,
			uint8_t *rx_tlv_hdr,
			uint8_t pool_id)
{
	uint32_t pkt_len, l2_hdr_offset;
	uint16_t msdu_len;
	struct dp_vdev *vdev;
	uint16_t peer_id = 0xFFFF;
	struct dp_peer *peer = NULL;
	uint8_t tid;

	qdf_nbuf_set_rx_chfrag_start(nbuf,
			hal_rx_msdu_end_first_msdu_get(rx_tlv_hdr));
	qdf_nbuf_set_rx_chfrag_end(nbuf,
			hal_rx_msdu_end_last_msdu_get(rx_tlv_hdr));

	l2_hdr_offset = hal_rx_msdu_end_l3_hdr_padding_get(rx_tlv_hdr);
	msdu_len = hal_rx_msdu_start_msdu_len_get(rx_tlv_hdr);
	pkt_len = msdu_len + l2_hdr_offset + RX_PKT_TLVS_LEN;

	/* Set length in nbuf */
	qdf_nbuf_set_pktlen(nbuf, pkt_len);

	/*
	 * Check if DMA completed -- msdu_done is the last bit
	 * to be written
	 */
	if (!hal_rx_attn_msdu_done_get(rx_tlv_hdr)) {

		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				FL("MSDU DONE failure"));

		hal_rx_dump_pkt_tlvs(rx_tlv_hdr, QDF_TRACE_LEVEL_INFO);
		qdf_assert(0);
	}

	peer_id = hal_rx_mpdu_start_sw_peer_id_get(rx_tlv_hdr);
	peer = dp_peer_find_by_id(soc, peer_id);

	if (!peer) {
		bool mpdu_done = false;
		struct dp_pdev *pdev = soc->pdev_list[pool_id];

		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
		FL("peer is NULL"));

		mpdu_done = dp_rx_chain_msdus(soc, nbuf, rx_tlv_hdr, pool_id);
		/* Trigger invalid peer handler wrapper */
		dp_rx_process_invalid_peer_wrapper(soc, nbuf, mpdu_done);

		if (mpdu_done) {
			pdev->invalid_peer_head_msdu = NULL;
			pdev->invalid_peer_tail_msdu = NULL;
		}
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
	qdf_nbuf_pull_head(nbuf, (l2_hdr_offset + RX_PKT_TLVS_LEN));

	if (dp_rx_mcast_echo_check(soc, peer, rx_tlv_hdr, nbuf)) {
		/* this is a looped back MCBC pkt, drop it */
		qdf_nbuf_free(nbuf);
		return;
	}
	/*
	 * In qwrap mode if the received packet matches with any of the vdev
	 * mac addresses, drop it. Donot receive multicast packets originated
	 * from any proxysta.
	 */
	if (check_qwrap_multicast_loopback(vdev, nbuf)) {
		qdf_nbuf_free(nbuf);
		return;
	}


	if (qdf_unlikely((peer->nawds_enabled == true) &&
			hal_rx_msdu_end_da_is_mcbc_get(rx_tlv_hdr))) {
		QDF_TRACE(QDF_MODULE_ID_DP,
					QDF_TRACE_LEVEL_DEBUG,
					"%s free buffer for multicast packet",
					 __func__);
		DP_STATS_INC(peer, rx.nawds_mcast_drop, 1);
		qdf_nbuf_free(nbuf);
		return;
	}

	if (!dp_wds_rx_policy_check(rx_tlv_hdr, vdev, peer,
				hal_rx_msdu_end_da_is_mcbc_get(rx_tlv_hdr))) {
		QDF_TRACE(QDF_MODULE_ID_DP,
				QDF_TRACE_LEVEL_ERROR,
				FL("mcast Policy Check Drop pkt"));
		/* Drop & free packet */
		qdf_nbuf_free(nbuf);
		return;
	}

	/* WDS Source Port Learning */
	if (qdf_likely(vdev->rx_decap_type == htt_cmn_pkt_type_ethernet &&
		vdev->wds_enabled))
		dp_rx_wds_srcport_learn(soc, rx_tlv_hdr, peer, nbuf);

	if (hal_rx_mpdu_start_mpdu_qos_control_valid_get(rx_tlv_hdr)) {
		/* TODO: Assuming that qos_control_valid also indicates
		 * unicast. Should we check this?
		 */
		tid = hal_rx_mpdu_start_tid_get(rx_tlv_hdr);
		if (peer &&
			peer->rx_tid[tid].hw_qdesc_vaddr_unaligned == NULL) {
			/* IEEE80211_SEQ_MAX indicates invalid start_seq */
			dp_rx_tid_setup_wifi3(peer, tid, 1, IEEE80211_SEQ_MAX);
		}
	}

#ifdef QCA_WIFI_NAPIER_EMULATION /* Debug code, remove later */
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			 "%s: p_id %d msdu_len %d hdr_off %d",
			 __func__, peer_id, msdu_len, l2_hdr_offset);

	print_hex_dump(KERN_ERR, "\t Pkt Data:", DUMP_PREFIX_NONE, 32, 4,
					qdf_nbuf_data(nbuf), 128, false);
#endif /* NAPIER_EMULATION */

	if (qdf_unlikely(vdev->rx_decap_type == htt_cmn_pkt_type_raw)) {
		qdf_nbuf_set_next(nbuf, NULL);
		dp_rx_deliver_raw(vdev, nbuf, peer);
	} else {
		if (qdf_unlikely(peer->bss_peer)) {
			QDF_TRACE(QDF_MODULE_ID_DP,
					QDF_TRACE_LEVEL_INFO,
					FL("received pkt with same src MAC"));
			/* Drop & free packet */
			qdf_nbuf_free(nbuf);
			return;
		}

		if (vdev->osif_rx) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
				FL("vdev %pK osif_rx %pK"), vdev,
				vdev->osif_rx);
			qdf_nbuf_set_next(nbuf, NULL);
			vdev->osif_rx(vdev->osif_vdev, nbuf);
			DP_STATS_INCC_PKT(vdev->pdev, rx.multicast, 1,
				qdf_nbuf_len(nbuf),
				hal_rx_msdu_end_da_is_mcbc_get(
					rx_tlv_hdr));
			DP_STATS_INC_PKT(vdev->pdev, rx.to_stack, 1,
							qdf_nbuf_len(nbuf));
		} else {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				FL("INVALID vdev %pK OR osif_rx"), vdev);
			DP_STATS_INC(soc, rx.err.invalid_vdev, 1);
		}
	}
	return;
}

/**
* dp_rx_err_deliver() - Function to deliver error frames to OS
*
* @soc: core DP main context
* @rx_desc : pointer to the sw rx descriptor
* @head: pointer to head of rx descriptors to be added to free list
* @tail: pointer to tail of rx descriptors to be added to free list
* quota: upper limit of descriptors that can be reaped
*
* Return: uint32_t: No. of Rx buffers reaped
*/
static void
dp_rx_err_deliver(struct dp_soc *soc, qdf_nbuf_t nbuf, uint8_t *rx_tlv_hdr)
{
	uint32_t pkt_len, l2_hdr_offset;
	uint16_t msdu_len;
	struct dp_vdev *vdev;
	uint16_t peer_id = 0xFFFF;
	struct dp_peer *peer = NULL;
	struct ether_header *eh;
	bool isBroadcast;

	/*
	 * Check if DMA completed -- msdu_done is the last bit
	 * to be written
	 */
	if (!hal_rx_attn_msdu_done_get(rx_tlv_hdr)) {

		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				FL("MSDU DONE failure"));

		hal_rx_dump_pkt_tlvs(rx_tlv_hdr, QDF_TRACE_LEVEL_INFO);
		qdf_assert(0);
	}

	peer_id = hal_rx_mpdu_start_sw_peer_id_get(rx_tlv_hdr);
	peer = dp_peer_find_by_id(soc, peer_id);

	l2_hdr_offset = hal_rx_msdu_end_l3_hdr_padding_get(rx_tlv_hdr);
	msdu_len = hal_rx_msdu_start_msdu_len_get(rx_tlv_hdr);
	pkt_len = msdu_len + l2_hdr_offset + RX_PKT_TLVS_LEN;

	/* Set length in nbuf */
	qdf_nbuf_set_pktlen(nbuf, pkt_len);

	qdf_nbuf_set_next(nbuf, NULL);

	qdf_nbuf_set_rx_chfrag_start(nbuf, 1);
	qdf_nbuf_set_rx_chfrag_end(nbuf, 1);

	if (!peer) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				FL("peer is NULL"));
		DP_STATS_INC_PKT(soc, rx.err.rx_invalid_peer, 1,
				qdf_nbuf_len(nbuf));
		/* Trigger invalid peer handler wrapper */
		dp_rx_process_invalid_peer_wrapper(soc, nbuf, true);
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

	/* Drop & free packet if mesh mode not enabled */
	if (!vdev->mesh_vdev) {
		qdf_nbuf_free(nbuf);
		DP_STATS_INC(soc, rx.err.invalid_vdev, 1);
		return;
	}

	/*
	 * Advance the packet start pointer by total size of
	 * pre-header TLV's
	 */
	qdf_nbuf_pull_head(nbuf, (l2_hdr_offset + RX_PKT_TLVS_LEN));

	if (dp_rx_filter_mesh_packets(vdev, nbuf, rx_tlv_hdr)
							== QDF_STATUS_SUCCESS) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_MED,
				FL("mesh pkt filtered"));
		DP_STATS_INC(vdev->pdev, dropped.mesh_filter, 1);

		qdf_nbuf_free(nbuf);
		return;

	}
	dp_rx_fill_mesh_stats(vdev, nbuf, rx_tlv_hdr, peer);

	if (qdf_unlikely(hal_rx_msdu_end_da_is_mcbc_get(rx_tlv_hdr) &&
				(vdev->rx_decap_type ==
				htt_cmn_pkt_type_ethernet))) {
		eh = (struct ether_header *)qdf_nbuf_data(nbuf);
		isBroadcast = (IEEE80211_IS_BROADCAST
				(eh->ether_dhost)) ? 1 : 0 ;
		if (isBroadcast) {
			DP_STATS_INC_PKT(peer, rx.bcast, 1,
					qdf_nbuf_len(nbuf));
		}
	}

	if (qdf_unlikely(vdev->rx_decap_type == htt_cmn_pkt_type_raw)) {
		dp_rx_deliver_raw(vdev, nbuf, peer);
	} else {
		DP_STATS_INC(vdev->pdev, rx.to_stack.num, 1);
		vdev->osif_rx(vdev->osif_vdev, nbuf);
	}

	return;
}

/**
 * dp_rx_process_mic_error(): Function to pass mic error indication to umac
 * @soc: DP SOC handle
 * @rx_desc : pointer to the sw rx descriptor
 * @head: pointer to head of rx descriptors to be added to free list
 * @tail: pointer to tail of rx descriptors to be added to free list
 *
 * return: void
 */
void
dp_rx_process_mic_error(struct dp_soc *soc,
			qdf_nbuf_t nbuf,
			uint8_t *rx_tlv_hdr)
{
	struct dp_vdev *vdev = NULL;
	struct dp_pdev *pdev = NULL;
	struct ol_if_ops *tops = NULL;
	struct ieee80211_frame *wh;
	uint8_t *rx_pkt_hdr;
	struct dp_peer *peer;
	uint16_t peer_id;

	if (!hal_rx_msdu_end_first_msdu_get(rx_tlv_hdr))
		return;

	rx_pkt_hdr = hal_rx_pkt_hdr_get(qdf_nbuf_data(nbuf));
	wh = (struct ieee80211_frame *)rx_pkt_hdr;

	peer_id = hal_rx_mpdu_start_sw_peer_id_get(rx_tlv_hdr);
	peer = dp_peer_find_by_id(soc, peer_id);
	if (!peer) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"peer not found");
		goto fail;
	}

	vdev = peer->vdev;
	if (!vdev) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"VDEV not found");
		goto fail;
	}

	pdev = vdev->pdev;
	if (!pdev) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"PDEV not found");
		goto fail;
	}

	tops = pdev->soc->cdp_soc.ol_ops;
	if (tops->rx_mic_error)
		tops->rx_mic_error(pdev->osif_pdev, vdev->vdev_id, wh);

fail:
	qdf_nbuf_free(nbuf);
	return;
}

/**
 * dp_rx_err_process() - Processes error frames routed to REO error ring
 *
 * @soc: core txrx main context
 * @hal_ring: opaque pointer to the HAL Rx Error Ring, which will be serviced
 * @quota: No. of units (packets) that can be serviced in one shot.
 *
 * This function implements error processing and top level demultiplexer
 * for all the frames routed to REO error ring.
 *
 * Return: uint32_t: No. of elements processed
 */
uint32_t
dp_rx_err_process(struct dp_soc *soc, void *hal_ring, uint32_t quota)
{
	void *hal_soc;
	void *ring_desc;
	union dp_rx_desc_list_elem_t *head = NULL;
	union dp_rx_desc_list_elem_t *tail = NULL;
	uint32_t rx_bufs_used = 0;
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

	/* Debug -- Remove later */
	qdf_assert(soc && hal_ring);

	hal_soc = soc->hal_soc;

	/* Debug -- Remove later */
	qdf_assert(hal_soc);

	if (qdf_unlikely(hal_srng_access_start(hal_soc, hal_ring))) {

		/* TODO */
		/*
		 * Need API to convert from hal_ring pointer to
		 * Ring Type / Ring Id combo
		 */
		DP_STATS_INC(soc, rx.err.hal_ring_access_fail, 1);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("HAL RING Access Failed -- %pK"), hal_ring);
		goto done;
	}

	while (qdf_likely(quota-- && (ring_desc =
				hal_srng_dst_get_next(hal_soc, hal_ring)))) {

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
		hal_rx_msdu_list_get(link_desc_va, &msdu_list, &num_msdus);

		if (qdf_unlikely((msdu_list.rbm[0] != DP_WBM2SW_RBM) &&
				(msdu_list.rbm[0] !=
					HAL_RX_BUF_RBM_WBM_IDLE_DESC_LIST))) {
			/* TODO */
			/* Call appropriate handler */
			DP_STATS_INC(soc, rx.err.invalid_rbm, 1);
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				FL("Invalid RBM %d"), msdu_list.rbm[0]);

			/* Return link descriptor through WBM ring (SW2WBM)*/
			dp_rx_link_desc_return(soc, ring_desc,
					HAL_BM_ACTION_RELEASE_MSDU_LIST);
			continue;
		}

		/* Get the MPDU DESC info */
		hal_rx_mpdu_desc_info_get(ring_desc, &mpdu_desc_info);

		if (mpdu_desc_info.mpdu_flags & HAL_MPDU_F_FRAGMENT) {
			/* TODO */
			rx_bufs_used += dp_rx_frag_handle(soc,
					ring_desc, &mpdu_desc_info,
					&head, &tail, quota);
			DP_STATS_INC(soc, rx.rx_frags, 1);
			continue;
		}

		if (hal_rx_reo_is_pn_error(ring_desc)) {
			/* TOD0 */
			DP_STATS_INC(soc,
				rx.err.
				reo_error[HAL_REO_ERR_PN_CHECK_FAILED],
				1);
			rx_bufs_used += dp_rx_pn_error_handle(soc,
					ring_desc, &mpdu_desc_info,
					&head, &tail, quota);
			continue;
		}

		if (hal_rx_reo_is_2k_jump(ring_desc)) {
			/* TOD0 */
			DP_STATS_INC(soc,
				rx.err.
				reo_error[HAL_REO_ERR_REGULAR_FRAME_2K_JUMP],
				1);
			rx_bufs_used += dp_rx_2k_jump_handle(soc,
					ring_desc, &mpdu_desc_info,
					&head, &tail, quota);
			continue;
		}
	}

done:
	hal_srng_access_end(hal_soc, hal_ring);

	if (soc->rx.flags.defrag_timeout_check)
		dp_rx_defrag_waitlist_flush(soc);

	/* Assume MAC id = 0, owner = 0 */
	if (rx_bufs_used) {
		dp_pdev = soc->pdev_list[0];
		dp_rxdma_srng = &dp_pdev->rx_refill_buf_ring;
		rx_desc_pool = &soc->rx_desc_buf[0];

		dp_rx_buffers_replenish(soc, 0, dp_rxdma_srng, rx_desc_pool,
			rx_bufs_used, &head, &tail);
	}

	return rx_bufs_used; /* Assume no scale factor for now */
}

/**
 * dp_rx_wbm_err_process() - Processes error frames routed to WBM release ring
 *
 * @soc: core txrx main context
 * @hal_ring: opaque pointer to the HAL Rx Error Ring, which will be serviced
 * @quota: No. of units (packets) that can be serviced in one shot.
 *
 * This function implements error processing and top level demultiplexer
 * for all the frames routed to WBM2HOST sw release ring.
 *
 * Return: uint32_t: No. of elements processed
 */
uint32_t
dp_rx_wbm_err_process(struct dp_soc *soc, void *hal_ring, uint32_t quota)
{
	void *hal_soc;
	void *ring_desc;
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

	/* Debug -- Remove later */
	qdf_assert(soc && hal_ring);

	hal_soc = soc->hal_soc;

	/* Debug -- Remove later */
	qdf_assert(hal_soc);

	if (qdf_unlikely(hal_srng_access_start(hal_soc, hal_ring))) {

		/* TODO */
		/*
		 * Need API to convert from hal_ring pointer to
		 * Ring Type / Ring Id combo
		 */
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("HAL RING Access Failed -- %pK"), hal_ring);
		goto done;
	}

	while (qdf_likely(quota-- && (ring_desc =
				hal_srng_dst_get_next(hal_soc, hal_ring)))) {

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
		qdf_assert(rx_desc);

		if (!dp_rx_desc_check_magic(rx_desc)) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
					FL("Invalid rx_desc cookie=%d"),
					rx_buf_cookie);
			continue;
		}

		nbuf = rx_desc->nbuf;
		qdf_nbuf_unmap_single(soc->osdev, nbuf,	QDF_DMA_BIDIRECTIONAL);

		/*
		 * save the wbm desc info in nbuf TLV. We will need this
		 * info when we do the actual nbuf processing
		 */
		hal_rx_wbm_err_info_get(ring_desc, &wbm_err_info);
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
	hal_srng_access_end(hal_soc, hal_ring);

	for (mac_id = 0; mac_id < MAX_PDEV_CNT; mac_id++) {
		if (rx_bufs_reaped[mac_id]) {
			dp_pdev = soc->pdev_list[mac_id];
			dp_rxdma_srng = &dp_pdev->rx_refill_buf_ring;
			rx_desc_pool = &soc->rx_desc_buf[mac_id];

			dp_rx_buffers_replenish(soc, mac_id, dp_rxdma_srng,
					rx_desc_pool, rx_bufs_reaped[mac_id],
					&head[mac_id], &tail[mac_id]);
			rx_bufs_used += rx_bufs_reaped[mac_id];
		}
	}

	nbuf = nbuf_head;
	while (nbuf) {
		rx_tlv_hdr = qdf_nbuf_data(nbuf);
		/*
		 * retrieve the wbm desc info from nbuf TLV, so we can
		 * handle error cases appropriately
		 */
		hal_rx_wbm_err_info_get_from_tlv(rx_tlv_hdr, &wbm_err_info);

		/* Set queue_mapping in nbuf to 0 */
		dp_set_rx_queue(nbuf, 0);

		next = nbuf->next;
		if (wbm_err_info.wbm_err_src == HAL_RX_WBM_ERR_SRC_REO) {
			if (wbm_err_info.reo_psh_rsn
				== HAL_RX_WBM_REO_PSH_RSN_ERROR) {

				DP_STATS_INC(soc,
					rx.err.reo_error
					[wbm_err_info.reo_err_code], 1);

				switch (wbm_err_info.reo_err_code) {
				/*
				 * Handling for packets which have NULL REO
				 * queue descriptor
				 */
				case HAL_REO_ERR_QUEUE_DESC_ADDR_0:
					pool_id = wbm_err_info.pool_id;
					QDF_TRACE(QDF_MODULE_ID_DP,
						QDF_TRACE_LEVEL_WARN,
						"Got pkt with REO ERROR: %d",
						wbm_err_info.reo_err_code);
					dp_rx_null_q_desc_handle(soc,
								nbuf,
								rx_tlv_hdr,
								pool_id);
					nbuf = next;
					continue;
				/* TODO */
				/* Add per error code accounting */

				default:
					QDF_TRACE(QDF_MODULE_ID_DP,
						QDF_TRACE_LEVEL_DEBUG,
						"REO error %d detected",
						wbm_err_info.reo_err_code);
				}
			}
		} else if (wbm_err_info.wbm_err_src ==
					HAL_RX_WBM_ERR_SRC_RXDMA) {
			if (wbm_err_info.rxdma_psh_rsn
					== HAL_RX_WBM_RXDMA_PSH_RSN_ERROR) {
				struct dp_peer *peer = NULL;
				uint16_t peer_id = 0xFFFF;

				DP_STATS_INC(soc,
					rx.err.rxdma_error
					[wbm_err_info.rxdma_err_code], 1);
				peer_id = hal_rx_mpdu_start_sw_peer_id_get(rx_tlv_hdr);
				peer = dp_peer_find_by_id(soc, peer_id);

				switch (wbm_err_info.rxdma_err_code) {
				case HAL_RXDMA_ERR_UNENCRYPTED:
					dp_rx_err_deliver(soc,
							nbuf,
							rx_tlv_hdr);
					nbuf = next;
					continue;

				case HAL_RXDMA_ERR_TKIP_MIC:
					dp_rx_process_mic_error(soc,
								nbuf,
								rx_tlv_hdr);
					nbuf = next;
					if (peer)
						DP_STATS_INC(peer, rx.err.mic_err, 1);
					continue;

				case HAL_RXDMA_ERR_DECRYPT:
					if (peer)
						DP_STATS_INC(peer, rx.err.decrypt_err, 1);
					QDF_TRACE(QDF_MODULE_ID_DP,
						QDF_TRACE_LEVEL_DEBUG,
					"Packet received with Decrypt error");
					break;

				default:
					QDF_TRACE(QDF_MODULE_ID_DP,
							QDF_TRACE_LEVEL_DEBUG,
							"RXDMA error %d",
							wbm_err_info.
								rxdma_err_code);
				}
			}
		} else {
			/* Should not come here */
			qdf_assert(0);
		}

		hal_rx_dump_pkt_tlvs(rx_tlv_hdr, QDF_TRACE_LEVEL_DEBUG);
		qdf_nbuf_free(nbuf);
		nbuf = next;
	}
	return rx_bufs_used; /* Assume no scale factor for now */
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
	void *rxdma_dst_ring_desc,
	union dp_rx_desc_list_elem_t **head,
	union dp_rx_desc_list_elem_t **tail)
{
	void *rx_msdu_link_desc;
	qdf_nbuf_t msdu;
	qdf_nbuf_t last;
	struct hal_rx_msdu_list msdu_list;
	uint16_t num_msdus;
	struct hal_buf_info buf_info;
	void *p_buf_addr_info;
	void *p_last_buf_addr_info;
	uint32_t rx_bufs_used = 0;
	uint32_t msdu_cnt;
	uint32_t i;
	uint8_t push_reason;
	uint8_t rxdma_error_code = 0;
	uint8_t bm_action = HAL_BM_ACTION_PUT_IN_IDLE_LIST;
	struct dp_pdev *pdev = dp_get_pdev_for_mac_id(soc, mac_id);

	msdu = 0;

	last = NULL;

	hal_rx_reo_ent_buf_paddr_get(rxdma_dst_ring_desc, &buf_info,
		&p_last_buf_addr_info, &msdu_cnt);

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

		hal_rx_msdu_list_get(rx_msdu_link_desc, &msdu_list, &num_msdus);

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
					qdf_assert(rx_desc);
					msdu = rx_desc->nbuf;

					qdf_nbuf_unmap_single(soc->osdev, msdu,
						QDF_DMA_FROM_DEVICE);

					QDF_TRACE(QDF_MODULE_ID_DP,
						QDF_TRACE_LEVEL_DEBUG,
						"[%s][%d] msdu_nbuf=%pK \n",
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

		hal_rx_mon_next_link_desc_get(rx_msdu_link_desc, &buf_info,
			&p_buf_addr_info);

		dp_rx_link_desc_return(soc, p_last_buf_addr_info, bm_action);
		p_last_buf_addr_info = p_buf_addr_info;

	} while (buf_info.paddr);

	DP_STATS_INC(soc, rx.err.rxdma_error[rxdma_error_code], 1);

	if (rxdma_error_code == HAL_RXDMA_ERR_DECRYPT) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"Packet received with Decrypt error");
	}

	return rx_bufs_used;
}

/**
* dp_rxdma_err_process() - RxDMA error processing functionality
*
* @soc: core txrx main contex
* @mac_id: mac id which is one of 3 mac_ids
* @hal_ring: opaque pointer to the HAL Rx Ring, which will be serviced
* @quota: No. of units (packets) that can be serviced in one shot.

* Return: num of buffers processed
*/
uint32_t
dp_rxdma_err_process(struct dp_soc *soc, uint32_t mac_id, uint32_t quota)
{
	struct dp_pdev *pdev = dp_get_pdev_for_mac_id(soc, mac_id);
	int mac_for_pdev = dp_get_mac_id_for_mac(soc, mac_id);
	void *hal_soc;
	void *rxdma_dst_ring_desc;
	void *err_dst_srng;
	union dp_rx_desc_list_elem_t *head = NULL;
	union dp_rx_desc_list_elem_t *tail = NULL;
	struct dp_srng *dp_rxdma_srng;
	struct rx_desc_pool *rx_desc_pool;
	uint32_t work_done = 0;
	uint32_t rx_bufs_used = 0;

#ifdef DP_INTR_POLL_BASED
	if (!pdev)
		return 0;
#endif
	err_dst_srng = pdev->rxdma_err_dst_ring[mac_for_pdev].hal_srng;

	if (!err_dst_srng) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"%s %d : HAL Monitor Destination Ring Init \
			Failed -- %pK\n",
			__func__, __LINE__, err_dst_srng);
		return 0;
	}

	hal_soc = soc->hal_soc;

	qdf_assert(hal_soc);

	if (qdf_unlikely(hal_srng_access_start(hal_soc, err_dst_srng))) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"%s %d : HAL Monitor Destination Ring Init \
			Failed -- %pK\n",
			__func__, __LINE__, err_dst_srng);
		return 0;
	}

	while (qdf_likely(quota-- && (rxdma_dst_ring_desc =
		hal_srng_dst_get_next(hal_soc, err_dst_srng)))) {

			rx_bufs_used += dp_rx_err_mpdu_pop(soc, mac_id,
						rxdma_dst_ring_desc,
						&head, &tail);
	}

	hal_srng_access_end(hal_soc, err_dst_srng);

	if (rx_bufs_used) {
		dp_rxdma_srng = &pdev->rx_refill_buf_ring;
		rx_desc_pool = &soc->rx_desc_buf[mac_id];

		dp_rx_buffers_replenish(soc, mac_id, dp_rxdma_srng,
			rx_desc_pool, rx_bufs_used, &head, &tail);

		work_done += rx_bufs_used;
	}

	return work_done;
}
