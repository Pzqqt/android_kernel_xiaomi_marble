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
#include "dp_internal.h"
#include "hal_api.h"
#include "qdf_trace.h"
#include "qdf_nbuf.h"
#include "dp_rx_defrag.h"
#include "dp_ipa.h"
#ifdef WIFI_MONITOR_SUPPORT
#include "dp_htt.h"
#include <dp_mon.h>
#endif
#ifdef FEATURE_WDS
#include "dp_txrx_wds.h"
#endif
#include <enet.h>	/* LLC_SNAP_HDR_LEN */
#include "qdf_net_types.h"
#include "dp_rx_buffer_pool.h"

#define dp_rx_err_alert(params...) QDF_TRACE_FATAL(QDF_MODULE_ID_DP_RX_ERROR, params)
#define dp_rx_err_err(params...) QDF_TRACE_ERROR(QDF_MODULE_ID_DP_RX_ERROR, params)
#define dp_rx_err_warn(params...) QDF_TRACE_WARN(QDF_MODULE_ID_DP_RX_ERROR, params)
#define dp_rx_err_info(params...) \
	__QDF_TRACE_FL(QDF_TRACE_LEVEL_INFO_HIGH, QDF_MODULE_ID_DP_RX_ERROR, ## params)
#define dp_rx_err_info_rl(params...) \
	__QDF_TRACE_RL(QDF_TRACE_LEVEL_INFO_HIGH, QDF_MODULE_ID_DP_RX_ERROR, ## params)
#define dp_rx_err_debug(params...) QDF_TRACE_DEBUG(QDF_MODULE_ID_DP_RX_ERROR, params)

#ifndef QCA_HOST_MODE_WIFI_DISABLED

/* Max buffer in invalid peer SG list*/
#define DP_MAX_INVALID_BUFFERS 10

/* Max regular Rx packet routing error */
#define DP_MAX_REG_RX_ROUTING_ERRS_THRESHOLD 20
#define DP_MAX_REG_RX_ROUTING_ERRS_IN_TIMEOUT 10
#define DP_RX_ERR_ROUTE_TIMEOUT_US (5 * 1000 * 1000) /* micro seconds */

#ifdef FEATURE_MEC
bool dp_rx_mcast_echo_check(struct dp_soc *soc,
			    struct dp_peer *peer,
			    uint8_t *rx_tlv_hdr,
			    qdf_nbuf_t nbuf)
{
	struct dp_vdev *vdev = peer->vdev;
	struct dp_pdev *pdev = vdev->pdev;
	struct dp_mec_entry *mecentry = NULL;
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

	/*
	 * if the received pkts src mac addr matches with the
	 * wired PCs MAC addr which is behind the STA or with
	 * wireless STAs MAC addr which are behind the Repeater,
	 * then drop the pkt as it is looped back
	 */
	if (hal_rx_msdu_end_sa_is_valid_get(soc->hal_soc, rx_tlv_hdr)) {
		sa_idx = hal_rx_msdu_end_sa_idx_get(soc->hal_soc, rx_tlv_hdr);

		if ((sa_idx < 0) ||
		    (sa_idx >= wlan_cfg_get_max_ast_idx(soc->wlan_cfg_ctx))) {
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				  "invalid sa_idx: %d", sa_idx);
			qdf_assert_always(0);
		}

		qdf_spin_lock_bh(&soc->ast_lock);
		ase = soc->ast_table[sa_idx];

		/*
		 * this check was not needed since MEC is not dependent on AST,
		 * but if we dont have this check SON has some issues in
		 * dual backhaul scenario. in APS SON mode, client connected
		 * to RE 2G and sends multicast packets. the RE sends it to CAP
		 * over 5G backhaul. the CAP loopback it on 2G to RE.
		 * On receiving in 2G STA vap, we assume that client has roamed
		 * and kickout the client.
		 */
		if (ase && (ase->peer_id != peer->peer_id)) {
			qdf_spin_unlock_bh(&soc->ast_lock);
			goto drop;
		}

		qdf_spin_unlock_bh(&soc->ast_lock);
	}

	qdf_spin_lock_bh(&soc->mec_lock);

	mecentry = dp_peer_mec_hash_find_by_pdevid(soc, pdev->pdev_id,
						   &data[QDF_MAC_ADDR_SIZE]);
	if (!mecentry) {
		qdf_spin_unlock_bh(&soc->mec_lock);
		return false;
	}

	qdf_spin_unlock_bh(&soc->mec_lock);

drop:
	dp_rx_err_info("%pK: received pkt with same src mac " QDF_MAC_ADDR_FMT,
		       soc, QDF_MAC_ADDR_REF(&data[QDF_MAC_ADDR_SIZE]));

	return true;
}
#endif
#endif /* QCA_HOST_MODE_WIFI_DISABLED */

void dp_rx_link_desc_refill_duplicate_check(
				struct dp_soc *soc,
				struct hal_buf_info *buf_info,
				hal_buff_addrinfo_t ring_buf_info)
{
	struct hal_buf_info current_link_desc_buf_info = { 0 };

	/* do duplicate link desc address check */
	hal_rx_buffer_addr_info_get_paddr(ring_buf_info,
					  &current_link_desc_buf_info);

	/*
	 * TODO - Check if the hal soc api call can be removed
	 * since the cookie is just used for print.
	 * buffer_addr_info is the first element of ring_desc
	 */
	hal_rx_buf_cookie_rbm_get(soc->hal_soc,
				  (uint32_t *)ring_buf_info,
				  &current_link_desc_buf_info);

	if (qdf_unlikely(current_link_desc_buf_info.paddr ==
			 buf_info->paddr)) {
		dp_info_rl("duplicate link desc addr: %llu, cookie: 0x%x",
			   current_link_desc_buf_info.paddr,
			   current_link_desc_buf_info.sw_cookie);
		DP_STATS_INC(soc, rx.err.dup_refill_link_desc, 1);
	}
	*buf_info = current_link_desc_buf_info;
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
		dp_rx_err_err("%pK: WBM RELEASE RING not initialized", soc);
		return status;
	}

	/* do duplicate link desc address check */
	dp_rx_link_desc_refill_duplicate_check(
				soc,
				&soc->last_op_info.wbm_rel_link_desc,
				link_desc_addr);

	if (qdf_unlikely(hal_srng_access_start(hal_soc, wbm_rel_srng))) {

		/* TODO */
		/*
		 * Need API to convert from hal_ring pointer to
		 * Ring Type / Ring Id combo
		 */
		dp_rx_err_err("%pK: HAL RING Access For WBM Release SRNG Failed - %pK",
			      soc, wbm_rel_srng);
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

		DP_STATS_INC(soc, rx.err.hal_ring_access_full_fail, 1);

		dp_info_rl("WBM Release Ring (Id %d) Full(Fail CNT %u)",
			   srng->ring_id,
			   soc->stats.rx.err.hal_ring_access_full_fail);
		dp_info_rl("HP 0x%x Reap HP 0x%x TP 0x%x Cached TP 0x%x",
			   *srng->u.src_ring.hp_addr,
			   srng->u.src_ring.reap_hp,
			   *srng->u.src_ring.tp_addr,
			   srng->u.src_ring.cached_tp);
		QDF_BUG(0);
	}
done:
	hal_srng_access_end(hal_soc, wbm_rel_srng);
	return status;

}

qdf_export_symbol(dp_rx_link_desc_return_by_addr);

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

#ifndef QCA_HOST_MODE_WIFI_DISABLED

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
	struct rx_desc_pool *rx_desc_pool;
	struct dp_rx_desc *rx_desc;
	/* First field in REO Dst ring Desc is buffer_addr_info */
	void *buf_addr_info = ring_desc;
	struct buffer_addr_info cur_link_desc_addr_info = { 0 };
	struct buffer_addr_info next_link_desc_addr_info = { 0 };

	hal_rx_reo_buf_paddr_get(soc->hal_soc, ring_desc, &buf_info);

	/* buffer_addr_info is the first element of ring_desc */
	hal_rx_buf_cookie_rbm_get(soc->hal_soc,
				  (uint32_t *)ring_desc,
				  &buf_info);

	link_desc_va = dp_rx_cookie_2_link_desc_va(soc, &buf_info);
	if (!link_desc_va) {
		dp_rx_err_debug("link desc va is null, soc %pk", soc);
		return rx_bufs_used;
	}

more_msdu_link_desc:
	/* No UNMAP required -- this is "malloc_consistent" memory */
	hal_rx_msdu_list_get(soc->hal_soc, link_desc_va, &msdu_list,
			     &mpdu_desc_info->msdu_count);

	for (i = 0; (i < mpdu_desc_info->msdu_count); i++) {
		rx_desc = soc->arch_ops.dp_rx_desc_cookie_2_va(
						soc, msdu_list.sw_cookie[i]);

		qdf_assert_always(rx_desc);

		/* all buffers from a MSDU link link belong to same pdev */
		*mac_id = rx_desc->pool_id;
		pdev = dp_get_pdev_for_lmac_id(soc, rx_desc->pool_id);
		if (!pdev) {
			dp_rx_err_debug("%pK: pdev is null for pool_id = %d",
					soc, rx_desc->pool_id);
			return rx_bufs_used;
		}

		if (!dp_rx_desc_check_magic(rx_desc)) {
			dp_rx_err_err("%pK: Invalid rx_desc cookie=%d",
				      soc, msdu_list.sw_cookie[i]);
			return rx_bufs_used;
		}

		rx_desc_pool = &soc->rx_desc_buf[rx_desc->pool_id];
		dp_ipa_rx_buf_smmu_mapping_lock(soc);
		dp_ipa_handle_rx_buf_smmu_mapping(soc, rx_desc->nbuf,
						  rx_desc_pool->buf_size,
						  false);
		qdf_nbuf_unmap_nbytes_single(soc->osdev, rx_desc->nbuf,
					     QDF_DMA_FROM_DEVICE,
					     rx_desc_pool->buf_size);
		rx_desc->unmapped = 1;
		dp_ipa_rx_buf_smmu_mapping_unlock(soc);

		rx_desc->rx_buf_start = qdf_nbuf_data(rx_desc->nbuf);

		rx_bufs_used++;
		tid = hal_rx_mpdu_start_tid_get(soc->hal_soc,
						rx_desc->rx_buf_start);
		dp_rx_err_err("%pK: Packet received with PN error for tid :%d",
			      soc, tid);

		rx_tlv_hdr = qdf_nbuf_data(rx_desc->nbuf);
		if (hal_rx_encryption_info_valid(soc->hal_soc, rx_tlv_hdr))
			hal_rx_print_pn(soc->hal_soc, rx_tlv_hdr);

		/* Just free the buffers */
		dp_rx_buffer_pool_nbuf_free(soc, rx_desc->nbuf, *mac_id);

		dp_rx_add_to_free_desc_list(&pdev->free_list_head,
					    &pdev->free_list_tail, rx_desc);
	}

	/*
	 * If the msdu's are spread across multiple link-descriptors,
	 * we cannot depend solely on the msdu_count(e.g., if msdu is
	 * spread across multiple buffers).Hence, it is
	 * necessary to check the next link_descriptor and release
	 * all the msdu's that are part of it.
	 */
	hal_rx_get_next_msdu_link_desc_buf_addr_info(
			link_desc_va,
			&next_link_desc_addr_info);

	if (hal_rx_is_buf_addr_info_valid(
				&next_link_desc_addr_info)) {
		/* Clear the next link desc info for the current link_desc */
		hal_rx_clear_next_msdu_link_desc_buf_addr_info(link_desc_va);

		dp_rx_link_desc_return_by_addr(soc, buf_addr_info,
					       HAL_BM_ACTION_PUT_IN_IDLE_LIST);
		hal_rx_buffer_addr_info_get_paddr(
				&next_link_desc_addr_info,
				&buf_info);
		/* buffer_addr_info is the first element of ring_desc */
		hal_rx_buf_cookie_rbm_get(soc->hal_soc,
					  (uint32_t *)&next_link_desc_addr_info,
					  &buf_info);
		cur_link_desc_addr_info = next_link_desc_addr_info;
		buf_addr_info = &cur_link_desc_addr_info;

		link_desc_va =
			dp_rx_cookie_2_link_desc_va(soc, &buf_info);

		goto more_msdu_link_desc;
	}
	quota--;
	dp_rx_link_desc_return_by_addr(soc, buf_addr_info,
				       HAL_BM_ACTION_PUT_IN_IDLE_LIST);
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

	peer_id = dp_rx_peer_metadata_peer_id_get(soc,
					       mpdu_desc_info->peer_meta_data);


	peer = dp_peer_get_ref_by_id(soc, peer_id, DP_MOD_ID_RX_ERR);

	if (qdf_likely(peer)) {
		/*
		 * TODO: Check for peer specific policies & set peer_pn_policy
		 */
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"discard rx due to PN error for peer  %pK  "QDF_MAC_ADDR_FMT,
			peer, QDF_MAC_ADDR_REF(peer->mac_addr.raw));

		dp_peer_unref_delete(peer, DP_MOD_ID_RX_ERR);
	}
	dp_rx_err_err("%pK: Packet received with PN error", soc);

	/* No peer PN policy -- definitely drop */
	if (!peer_pn_policy)
		rx_bufs_used = dp_rx_msdus_drop(soc, ring_desc,
						mpdu_desc_info,
						mac_id, quota);

	return rx_bufs_used;
}

/**
 * dp_rx_oor_handle() - Handles the msdu which is OOR error
 *
 * @soc: core txrx main context
 * @nbuf: pointer to msdu skb
 * @peer_id: dp peer ID
 * @rx_tlv_hdr: start of rx tlv header
 *
 * This function process the msdu delivered from REO2TCL
 * ring with error type OOR
 *
 * Return: None
 */
static void
dp_rx_oor_handle(struct dp_soc *soc,
		 qdf_nbuf_t nbuf,
		 uint16_t peer_id,
		 uint8_t *rx_tlv_hdr)
{
	uint32_t frame_mask = FRAME_MASK_IPV4_ARP | FRAME_MASK_IPV4_DHCP |
				FRAME_MASK_IPV4_EAPOL | FRAME_MASK_IPV6_DHCP;
	struct dp_peer *peer = NULL;

	peer = dp_peer_get_ref_by_id(soc, peer_id, DP_MOD_ID_RX_ERR);
	if (!peer) {
		dp_info_rl("peer not found");
		goto free_nbuf;
	}

	if (dp_rx_deliver_special_frame(soc, peer, nbuf, frame_mask,
					rx_tlv_hdr)) {
		DP_STATS_INC(soc, rx.err.reo_err_oor_to_stack, 1);
		dp_peer_unref_delete(peer, DP_MOD_ID_RX_ERR);
		return;
	}

free_nbuf:
	if (peer)
		dp_peer_unref_delete(peer, DP_MOD_ID_RX_ERR);

	DP_STATS_INC(soc, rx.err.reo_err_oor_drop, 1);
	qdf_nbuf_free(nbuf);
}

/**
 * dp_rx_err_nbuf_pn_check() - Check if the PN number of this current packet
 *				is a monotonous increment of packet number
 *				from the previous successfully re-ordered
 *				frame.
 * @soc: Datapath SOC handle
 * @ring_desc: REO ring descriptor
 * @nbuf: Current packet
 *
 * Return: QDF_STATUS_SUCCESS, if the pn check passes, else QDF_STATUS_E_FAILURE
 */
static inline QDF_STATUS
dp_rx_err_nbuf_pn_check(struct dp_soc *soc, hal_ring_desc_t ring_desc,
			qdf_nbuf_t nbuf)
{
	uint64_t prev_pn, curr_pn[2];

	hal_rx_reo_prev_pn_get(soc->hal_soc, ring_desc, &prev_pn);
	hal_rx_tlv_get_pn_num(soc->hal_soc, qdf_nbuf_data(nbuf), curr_pn);

	if (curr_pn[0] > prev_pn)
		return QDF_STATUS_SUCCESS;

	return QDF_STATUS_E_FAILURE;
}

#ifdef WLAN_SKIP_BAR_UPDATE
static
void dp_rx_err_handle_bar(struct dp_soc *soc,
			  struct dp_peer *peer,
			  qdf_nbuf_t nbuf)
{
	dp_info_rl("BAR update to H.W is skipped");
	DP_STATS_INC(soc, rx.err.bar_handle_fail_count, 1);
}
#else
static
void dp_rx_err_handle_bar(struct dp_soc *soc,
			  struct dp_peer *peer,
			  qdf_nbuf_t nbuf)
{
	uint8_t *rx_tlv_hdr;
	unsigned char type, subtype;
	uint16_t start_seq_num;
	uint32_t tid;
	QDF_STATUS status;
	struct ieee80211_frame_bar *bar;

	/*
	 * 1. Is this a BAR frame. If not Discard it.
	 * 2. If it is, get the peer id, tid, ssn
	 * 2a Do a tid update
	 */

	rx_tlv_hdr = qdf_nbuf_data(nbuf);
	bar = (struct ieee80211_frame_bar *)(rx_tlv_hdr + soc->rx_pkt_tlv_size);

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

	status = dp_rx_tid_update_wifi3(peer, tid,
					peer->rx_tid[tid].ba_win_size,
					start_seq_num,
					true);
	if (status != QDF_STATUS_SUCCESS) {
		dp_err_rl("failed to handle bar frame update rx tid");
		DP_STATS_INC(soc, rx.err.bar_handle_fail_count, 1);
	} else {
		DP_STATS_INC(soc, rx.err.ssn_update_count, 1);
	}
}
#endif

/**
 * _dp_rx_bar_frame_handle(): Core of the BAR frame handling
 * @soc: Datapath SoC handle
 * @nbuf: packet being processed
 * @mpdu_desc_info: mpdu desc info for the current packet
 * @tid: tid on which the packet arrived
 * @err_status: Flag to indicate if REO encountered an error while routing this
 *		frame
 * @error_code: REO error code
 *
 * Return: None
 */
static void
_dp_rx_bar_frame_handle(struct dp_soc *soc, qdf_nbuf_t nbuf,
			struct hal_rx_mpdu_desc_info *mpdu_desc_info,
			uint32_t tid, uint8_t err_status, uint32_t error_code)
{
	uint16_t peer_id;
	struct dp_peer *peer;

	peer_id = dp_rx_peer_metadata_peer_id_get(soc,
					       mpdu_desc_info->peer_meta_data);
	peer = dp_peer_get_tgt_peer_by_id(soc, peer_id, DP_MOD_ID_RX_ERR);
	if (!peer)
		return;

	dp_info("BAR frame: peer = " QDF_MAC_ADDR_FMT
		" peer_id = %d"
		" tid = %u"
		" SSN = %d"
		" error status = %d",
		QDF_MAC_ADDR_REF(peer->mac_addr.raw),
		peer->peer_id,
		tid,
		mpdu_desc_info->mpdu_seq,
		err_status);

	if (err_status == HAL_REO_ERROR_DETECTED) {
		switch (error_code) {
		case HAL_REO_ERR_BAR_FRAME_2K_JUMP:
			/* fallthrough */
		case HAL_REO_ERR_BAR_FRAME_OOR:
			dp_rx_err_handle_bar(soc, peer, nbuf);
			DP_STATS_INC(soc, rx.err.reo_error[error_code], 1);
			break;
		default:
			DP_STATS_INC(soc, rx.bar_frame, 1);
		}
	}

	dp_peer_unref_delete(peer, DP_MOD_ID_RX_ERR);
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
	struct dp_pdev *dp_pdev = dp_get_pdev_for_lmac_id(soc, mac_id);

	if (!dp_pdev) {
		dp_rx_err_debug("%pK: pdev is null for mac_id = %d", soc, mac_id);
		return mpdu_done;
	}
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
		dp_pdev->ppdu_id = hal_rx_get_ppdu_id(soc->hal_soc,
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

		dp_monitor_get_mpdu_status(dp_pdev, soc, rx_tlv_hdr);
	}

	if (dp_pdev->ppdu_id == hal_rx_attn_phy_ppdu_id_get(soc->hal_soc,
							    rx_tlv_hdr) &&
	    hal_rx_attn_msdu_done_get(soc->hal_soc, rx_tlv_hdr)) {
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

/**
 * dp_rx_bar_frame_handle() - Function to handle err BAR frames
 * @soc: core DP main context
 * @ring_desc: Hal ring desc
 * @rx_desc: dp rx desc
 * @mpdu_desc_info: mpdu desc info
 *
 * Handle the error BAR frames received. Ensure the SOC level
 * stats are updated based on the REO error code. The BAR frames
 * are further processed by updating the Rx tids with the start
 * sequence number (SSN) and BA window size. Desc is returned
 * to the free desc list
 *
 * Return: none
 */
static void
dp_rx_bar_frame_handle(struct dp_soc *soc,
		       hal_ring_desc_t ring_desc,
		       struct dp_rx_desc *rx_desc,
		       struct hal_rx_mpdu_desc_info *mpdu_desc_info,
		       uint8_t err_status,
		       uint32_t err_code)
{
	qdf_nbuf_t nbuf;
	struct dp_pdev *pdev;
	struct rx_desc_pool *rx_desc_pool;
	uint8_t *rx_tlv_hdr;
	uint32_t tid;

	nbuf = rx_desc->nbuf;
	rx_desc_pool = &soc->rx_desc_buf[rx_desc->pool_id];
	dp_ipa_rx_buf_smmu_mapping_lock(soc);
	dp_ipa_handle_rx_buf_smmu_mapping(soc, nbuf,
					  rx_desc_pool->buf_size,
					  false);
	qdf_nbuf_unmap_nbytes_single(soc->osdev, nbuf,
				     QDF_DMA_FROM_DEVICE,
				     rx_desc_pool->buf_size);
	rx_desc->unmapped = 1;
	dp_ipa_rx_buf_smmu_mapping_unlock(soc);
	rx_tlv_hdr = qdf_nbuf_data(nbuf);
	tid = hal_rx_mpdu_start_tid_get(soc->hal_soc,
					rx_tlv_hdr);
	pdev = dp_get_pdev_for_lmac_id(soc, rx_desc->pool_id);

	_dp_rx_bar_frame_handle(soc, nbuf, mpdu_desc_info, tid, err_status,
				err_code);
	dp_rx_link_desc_return(soc, ring_desc,
			       HAL_BM_ACTION_PUT_IN_IDLE_LIST);
	dp_rx_buffer_pool_nbuf_free(soc, rx_desc->nbuf,
				    rx_desc->pool_id);
	dp_rx_add_to_free_desc_list(&pdev->free_list_head,
				    &pdev->free_list_tail,
				    rx_desc);
}

#endif /* QCA_HOST_MODE_WIFI_DISABLED */

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
	struct dp_peer *peer = NULL;
	struct dp_rx_tid *rx_tid = NULL;
	uint32_t frame_mask = FRAME_MASK_IPV4_ARP;

	peer = dp_peer_get_ref_by_id(soc, peer_id, DP_MOD_ID_RX_ERR);
	if (!peer) {
		dp_rx_err_info_rl("%pK: peer not found", soc);
		goto free_nbuf;
	}

	if (tid >= DP_MAX_TIDS) {
		dp_info_rl("invalid tid");
		goto nbuf_deliver;
	}

	rx_tid = &peer->rx_tid[tid];
	qdf_spin_lock_bh(&rx_tid->tid_lock);

	/* only if BA session is active, allow send Delba */
	if (rx_tid->ba_status != DP_RX_BA_ACTIVE) {
		qdf_spin_unlock_bh(&rx_tid->tid_lock);
		goto nbuf_deliver;
	}

	if (!rx_tid->delba_tx_status) {
		rx_tid->delba_tx_retry++;
		rx_tid->delba_tx_status = 1;
		rx_tid->delba_rcode =
			IEEE80211_REASON_QOS_SETUP_REQUIRED;
		qdf_spin_unlock_bh(&rx_tid->tid_lock);
		if (soc->cdp_soc.ol_ops->send_delba) {
			DP_STATS_INC(soc, rx.err.rx_2k_jump_delba_sent, 1);
			soc->cdp_soc.ol_ops->send_delba(
					peer->vdev->pdev->soc->ctrl_psoc,
					peer->vdev->vdev_id,
					peer->mac_addr.raw,
					tid,
					rx_tid->delba_rcode,
					CDP_DELBA_2K_JUMP);
		}
	} else {
		qdf_spin_unlock_bh(&rx_tid->tid_lock);
	}

nbuf_deliver:
	if (dp_rx_deliver_special_frame(soc, peer, nbuf, frame_mask,
					rx_tlv_hdr)) {
		DP_STATS_INC(soc, rx.err.rx_2k_jump_to_stack, 1);
		dp_peer_unref_delete(peer, DP_MOD_ID_RX_ERR);
		return;
	}

free_nbuf:
	if (peer)
		dp_peer_unref_delete(peer, DP_MOD_ID_RX_ERR);
	DP_STATS_INC(soc, rx.err.rx_2k_jump_drop, 1);
	qdf_nbuf_free(nbuf);
}

#if defined(QCA_WIFI_QCA6390) || defined(QCA_WIFI_QCA6490) || \
    defined(QCA_WIFI_QCA6750) || defined(QCA_WIFI_KIWI)
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
	uint8_t *rx_pkt_hdr = hal_rx_pkt_hdr_get(soc->hal_soc, rx_tlv_hdr);
	struct dp_pdev *pdev = dp_get_pdev_for_lmac_id(soc, pool_id);
	struct ieee80211_frame *wh = (struct ieee80211_frame *)rx_pkt_hdr;

	if (!pdev) {
		dp_rx_err_debug("%pK: pdev is null for pool_id = %d",
				soc, pool_id);
		return false;
	}
	/*
	 * WAR- In certain types of packets if peer_id is not correct then
	 * driver may not be able find. Try finding peer by addr_2 of
	 * received MPDU
	 */
	if (wh)
		peer = dp_peer_find_hash_find(soc, wh->i_addr2, 0,
					      DP_VDEV_ALL, DP_MOD_ID_RX_ERR);
	if (peer) {
		dp_verbose_debug("MPDU sw_peer_id & ast_idx is corrupted");
		hal_rx_dump_pkt_tlvs(soc->hal_soc, rx_tlv_hdr,
				     QDF_TRACE_LEVEL_DEBUG);
		DP_STATS_INC_PKT(soc, rx.err.rx_invalid_peer_id,
				 1, qdf_nbuf_len(nbuf));
		qdf_nbuf_free(nbuf);

		dp_peer_unref_delete(peer, DP_MOD_ID_RX_ERR);
		return true;
	}
	return false;
}

/**
 * dp_rx_check_pkt_len() - Check for pktlen validity
 * @soc: DP SOC context
 * @pkt_len: computed length of the pkt from caller in bytes
 *
 * Return: true if pktlen > RX_BUFFER_SIZE, else return false
 *
 */
static inline
bool dp_rx_check_pkt_len(struct dp_soc *soc, uint32_t pkt_len)
{
	if (qdf_unlikely(pkt_len > RX_DATA_BUFFER_SIZE)) {
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
bool dp_rx_check_pkt_len(struct dp_soc *soc, uint32_t pkt_len)
{
	return false;
}

#endif

#ifndef QCA_HOST_MODE_WIFI_DISABLED
/**
 * dp_rx_err_is_pn_check_needed() - Check if the packet number check is needed
 *				for this frame received in REO error ring.
 * @soc: Datapath SOC handle
 * @error: REO error detected or not
 * @error_code: Error code in case of REO error
 *
 * Return: true if pn check if needed in software,
 *	false, if pn check if not needed.
 */
static inline bool
dp_rx_err_is_pn_check_needed(struct dp_soc *soc, uint8_t error,
			     uint32_t error_code)
{
	return (soc->features.pn_in_reo_dest &&
		(error == HAL_REO_ERROR_DETECTED &&
		 (hal_rx_reo_is_2k_jump(error_code) ||
		  hal_rx_reo_is_oor_error(error_code) ||
		  hal_rx_reo_is_bar_oor_2k_jump(error_code))));
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
	uint32_t pkt_len;
	uint16_t msdu_len;
	struct dp_vdev *vdev;
	uint8_t tid;
	qdf_ether_header_t *eh;
	struct hal_rx_msdu_metadata msdu_metadata;
	uint16_t sa_idx = 0;
	bool is_eapol;
	bool enh_flag;

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

	hal_rx_msdu_metadata_get(soc->hal_soc, rx_tlv_hdr, &msdu_metadata);
	msdu_len = hal_rx_msdu_start_msdu_len_get(soc->hal_soc, rx_tlv_hdr);
	pkt_len = msdu_len + msdu_metadata.l3_hdr_pad + soc->rx_pkt_tlv_size;

	if (qdf_likely(!qdf_nbuf_is_frag(nbuf))) {
		if (dp_rx_check_pkt_len(soc, pkt_len))
			goto drop_nbuf;

		/* Set length in nbuf */
		qdf_nbuf_set_pktlen(
			nbuf, qdf_min(pkt_len, (uint32_t)RX_DATA_BUFFER_SIZE));
		qdf_assert_always(nbuf->data == rx_tlv_hdr);
	}

	/*
	 * Check if DMA completed -- msdu_done is the last bit
	 * to be written
	 */
	if (!hal_rx_attn_msdu_done_get(soc->hal_soc, rx_tlv_hdr)) {

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
		struct dp_pdev *pdev = dp_get_pdev_for_lmac_id(soc, pool_id);

		if (!pdev) {
			dp_err_rl("pdev is null for pool_id = %d", pool_id);
			return QDF_STATUS_E_FAILURE;
		}

		dp_err_rl("peer is NULL");
		DP_STATS_INC_PKT(soc, rx.err.rx_invalid_peer, 1,
				 qdf_nbuf_len(nbuf));

		/* QCN9000 has the support enabled */
		if (qdf_unlikely(soc->wbm_release_desc_rx_sg_support)) {
			mpdu_done = true;
			nbuf->next = NULL;
			/* Trigger invalid peer handler wrapper */
			dp_rx_process_invalid_peer_wrapper(soc,
					nbuf, mpdu_done, pool_id);
		} else {
			mpdu_done = dp_rx_chain_msdus(soc, nbuf, rx_tlv_hdr, pool_id);
			/* Trigger invalid peer handler wrapper */
			dp_rx_process_invalid_peer_wrapper(soc,
					pdev->invalid_peer_head_msdu,
					mpdu_done, pool_id);
		}

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
		qdf_nbuf_pull_head(nbuf, soc->rx_pkt_tlv_size);
	else
		qdf_nbuf_pull_head(nbuf, (msdu_metadata.l3_hdr_pad +
				   soc->rx_pkt_tlv_size));

	DP_STATS_INC_PKT(vdev, rx_i.null_q_desc_pkt, 1, qdf_nbuf_len(nbuf));

	dp_vdev_peer_stats_update_protocol_cnt(vdev, nbuf, NULL, 0, 1);

	if (hal_rx_msdu_end_sa_is_valid_get(soc->hal_soc, rx_tlv_hdr)) {
		sa_idx = hal_rx_msdu_end_sa_idx_get(soc->hal_soc, rx_tlv_hdr);

		if ((sa_idx < 0) ||
		    (sa_idx >= wlan_cfg_get_max_ast_idx(soc->wlan_cfg_ctx))) {
			DP_STATS_INC(soc, rx.err.invalid_sa_da_idx, 1);
			goto drop_nbuf;
		}
	}

	if ((!soc->mec_fw_offload) &&
	    dp_rx_mcast_echo_check(soc, peer, rx_tlv_hdr, nbuf)) {
		/* this is a looped back MCBC pkt, drop it */
		DP_STATS_INC_PKT(peer, rx.mec_drop, 1,
				 qdf_nbuf_len(nbuf));
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
		DP_STATS_INC(peer, rx.policy_check_drop, 1);
		goto drop_nbuf;
	}
	/* WDS Source Port Learning */
	if (!soc->ast_offload_support &&
	    qdf_likely(vdev->rx_decap_type == htt_cmn_pkt_type_ethernet &&
		       vdev->wds_enabled))
		dp_rx_wds_srcport_learn(soc, rx_tlv_hdr, peer, nbuf,
					msdu_metadata);

	if (hal_rx_is_unicast(soc->hal_soc, rx_tlv_hdr)) {
		tid = hal_rx_tid_get(soc->hal_soc, rx_tlv_hdr);
		if (!peer->rx_tid[tid].hw_qdesc_vaddr_unaligned)
			dp_rx_tid_setup_wifi3(peer, tid, 1, IEEE80211_SEQ_MAX);
			/* IEEE80211_SEQ_MAX indicates invalid start_seq */
	}

	eh = (qdf_ether_header_t *)qdf_nbuf_data(nbuf);

	if (!peer->authorize) {
		is_eapol = qdf_nbuf_is_ipv4_eapol_pkt(nbuf) ||
			   qdf_nbuf_is_ipv4_wapi_pkt(nbuf);

		if (is_eapol) {
			if (qdf_mem_cmp(eh->ether_dhost,
					&vdev->mac_addr.raw[0],
					QDF_MAC_ADDR_SIZE))
				goto drop_nbuf;
		} else {
			goto drop_nbuf;
		}
	}

	/*
	 * Drop packets in this path if cce_match is found. Packets will come
	 * in following path depending on whether tidQ is setup.
	 * 1. If tidQ is setup: WIFILI_HAL_RX_WBM_REO_PSH_RSN_ROUTE and
	 * cce_match = 1
	 *    Packets with WIFILI_HAL_RX_WBM_REO_PSH_RSN_ROUTE are already
	 *    dropped.
	 * 2. If tidQ is not setup: WIFILI_HAL_RX_WBM_REO_PSH_RSN_ERROR and
	 * cce_match = 1
	 *    These packets need to be dropped and should not get delivered
	 *    to stack.
	 */
	if (qdf_unlikely(dp_rx_err_cce_drop(soc, vdev, nbuf, rx_tlv_hdr))) {
		goto drop_nbuf;
	}

	if (qdf_unlikely(vdev->rx_decap_type == htt_cmn_pkt_type_raw)) {
		qdf_nbuf_set_next(nbuf, NULL);
		dp_rx_deliver_raw(vdev, nbuf, peer);
	} else {
		enh_flag = vdev->pdev->enhanced_stats_en;
		qdf_nbuf_set_next(nbuf, NULL);
		DP_PEER_TO_STACK_INCC_PKT(peer, 1, qdf_nbuf_len(nbuf),
					  enh_flag);
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
			DP_PEER_MC_INCC_PKT(peer, 1, qdf_nbuf_len(nbuf),
					    enh_flag);

			if (QDF_IS_ADDR_BROADCAST(eh->ether_dhost))
				DP_PEER_BC_INCC_PKT(peer, 1, qdf_nbuf_len(nbuf),
						    enh_flag);
		}

		qdf_nbuf_set_exc_frame(nbuf, 1);
		dp_rx_deliver_to_stack(soc, vdev, peer, nbuf, NULL);
	}
	return QDF_STATUS_SUCCESS;

drop_nbuf:
	qdf_nbuf_free(nbuf);
	return QDF_STATUS_E_FAILURE;
}

/**
 * dp_rx_reo_err_entry_process() - Handles for REO error entry processing
 *
 * @soc: core txrx main context
 * @ring_desc: opaque pointer to the REO error ring descriptor
 * @mpdu_desc_info: pointer to mpdu level description info
 * @link_desc_va: pointer to msdu_link_desc virtual address
 * @err_code: reo erro code fetched from ring entry
 *
 * Function to handle msdus fetched from msdu link desc, currently
 * only support 2K jump, OOR error.
 *
 * Return: msdu count processed.
 */
static uint32_t
dp_rx_reo_err_entry_process(struct dp_soc *soc,
			    void *ring_desc,
			    struct hal_rx_mpdu_desc_info *mpdu_desc_info,
			    void *link_desc_va,
			    enum hal_reo_error_code err_code)
{
	uint32_t rx_bufs_used = 0;
	struct dp_pdev *pdev;
	int i;
	uint8_t *rx_tlv_hdr_first;
	uint8_t *rx_tlv_hdr_last;
	uint32_t tid = DP_MAX_TIDS;
	uint16_t peer_id;
	struct dp_peer *peer;
	struct dp_rx_desc *rx_desc;
	struct rx_desc_pool *rx_desc_pool;
	qdf_nbuf_t nbuf;
	struct hal_buf_info buf_info;
	struct hal_rx_msdu_list msdu_list;
	uint16_t num_msdus;
	struct buffer_addr_info cur_link_desc_addr_info = { 0 };
	struct buffer_addr_info next_link_desc_addr_info = { 0 };
	/* First field in REO Dst ring Desc is buffer_addr_info */
	void *buf_addr_info = ring_desc;
	qdf_nbuf_t head_nbuf = NULL;
	qdf_nbuf_t tail_nbuf = NULL;
	uint16_t msdu_processed = 0;
	QDF_STATUS status;
	bool ret, is_pn_check_needed;
	uint8_t rx_desc_pool_id;

	peer_id = dp_rx_peer_metadata_peer_id_get(soc,
					       mpdu_desc_info->peer_meta_data);
	is_pn_check_needed = dp_rx_err_is_pn_check_needed(soc,
							  HAL_REO_ERROR_DETECTED,
							  err_code);
more_msdu_link_desc:
	hal_rx_msdu_list_get(soc->hal_soc, link_desc_va, &msdu_list,
			     &num_msdus);
	for (i = 0; i < num_msdus; i++) {
		rx_desc = soc->arch_ops.dp_rx_desc_cookie_2_va(
						soc,
						msdu_list.sw_cookie[i]);

		qdf_assert_always(rx_desc);

		rx_desc_pool_id = rx_desc->pool_id;
		/* all buffers from a MSDU link belong to same pdev */
		pdev = dp_get_pdev_for_lmac_id(soc, rx_desc_pool_id);

		nbuf = rx_desc->nbuf;
		ret = dp_rx_desc_paddr_sanity_check(rx_desc,
						    msdu_list.paddr[i]);
		if (!ret) {
			DP_STATS_INC(soc, rx.err.nbuf_sanity_fail, 1);
			rx_desc->in_err_state = 1;
			continue;
		}

		rx_desc_pool = &soc->rx_desc_buf[rx_desc_pool_id];
		dp_ipa_rx_buf_smmu_mapping_lock(soc);
		dp_ipa_handle_rx_buf_smmu_mapping(soc, nbuf,
						  rx_desc_pool->buf_size,
						  false);
		qdf_nbuf_unmap_nbytes_single(soc->osdev, nbuf,
					     QDF_DMA_FROM_DEVICE,
					     rx_desc_pool->buf_size);
		rx_desc->unmapped = 1;
		dp_ipa_rx_buf_smmu_mapping_unlock(soc);

		QDF_NBUF_CB_RX_PKT_LEN(nbuf) = msdu_list.msdu_info[i].msdu_len;
		rx_bufs_used++;
		dp_rx_add_to_free_desc_list(&pdev->free_list_head,
					    &pdev->free_list_tail, rx_desc);

		DP_RX_LIST_APPEND(head_nbuf, tail_nbuf, nbuf);

		if (qdf_unlikely(msdu_list.msdu_info[i].msdu_flags &
				 HAL_MSDU_F_MSDU_CONTINUATION))
			continue;

		if (dp_rx_buffer_pool_refill(soc, head_nbuf,
					     rx_desc_pool_id)) {
			/* MSDU queued back to the pool */
			goto process_next_msdu;
		}

		rx_tlv_hdr_first = qdf_nbuf_data(head_nbuf);
		rx_tlv_hdr_last = qdf_nbuf_data(tail_nbuf);

		if (qdf_unlikely(head_nbuf != tail_nbuf)) {
			nbuf = dp_rx_sg_create(soc, head_nbuf);
			qdf_nbuf_set_is_frag(nbuf, 1);
			DP_STATS_INC(soc, rx.err.reo_err_oor_sg_count, 1);
		}

		if (is_pn_check_needed) {
			status = dp_rx_err_nbuf_pn_check(soc, ring_desc, nbuf);
			if (QDF_IS_STATUS_ERROR(status)) {
				DP_STATS_INC(soc, rx.err.pn_in_dest_check_fail,
					     1);
				qdf_nbuf_free(nbuf);
				goto process_next_msdu;
			}

			hal_rx_tlv_populate_mpdu_desc_info(soc->hal_soc,
							   qdf_nbuf_data(nbuf),
							   mpdu_desc_info);
			peer_id = dp_rx_peer_metadata_peer_id_get(soc,
					mpdu_desc_info->peer_meta_data);

			if (mpdu_desc_info->bar_frame)
				_dp_rx_bar_frame_handle(soc, nbuf,
							mpdu_desc_info, tid,
							HAL_REO_ERROR_DETECTED,
							err_code);
		}

		switch (err_code) {
		case HAL_REO_ERR_REGULAR_FRAME_2K_JUMP:
		case HAL_REO_ERR_2K_ERROR_HANDLING_FLAG_SET:
		case HAL_REO_ERR_BAR_FRAME_2K_JUMP:
			/*
			 * only first msdu, mpdu start description tlv valid?
			 * and use it for following msdu.
			 */
			if (hal_rx_msdu_end_first_msdu_get(soc->hal_soc,
							   rx_tlv_hdr_last))
				tid = hal_rx_mpdu_start_tid_get(
							soc->hal_soc,
							rx_tlv_hdr_first);

			dp_2k_jump_handle(soc, nbuf, rx_tlv_hdr_last,
					  peer_id, tid);
			break;

		case HAL_REO_ERR_REGULAR_FRAME_OOR:
		case HAL_REO_ERR_BAR_FRAME_OOR:
			dp_rx_oor_handle(soc, nbuf, peer_id, rx_tlv_hdr_last);
			break;
		case HAL_REO_ERR_QUEUE_DESC_ADDR_0:
			peer = dp_peer_get_ref_by_id(soc, peer_id, DP_MOD_ID_RX_ERR);
			if (!peer)
				dp_info_rl("peer is null peer_id %u", peer_id);
			dp_rx_null_q_desc_handle(soc, nbuf, rx_tlv_hdr_last,
						 rx_desc_pool_id, peer);
			if (peer)
				dp_peer_unref_delete(peer, DP_MOD_ID_RX_ERR);
			break;
		default:
			dp_err_rl("Non-support error code %d", err_code);
			qdf_nbuf_free(nbuf);
		}

process_next_msdu:
		msdu_processed++;
		head_nbuf = NULL;
		tail_nbuf = NULL;
	}

	/*
	 * If the msdu's are spread across multiple link-descriptors,
	 * we cannot depend solely on the msdu_count(e.g., if msdu is
	 * spread across multiple buffers).Hence, it is
	 * necessary to check the next link_descriptor and release
	 * all the msdu's that are part of it.
	 */
	hal_rx_get_next_msdu_link_desc_buf_addr_info(
			link_desc_va,
			&next_link_desc_addr_info);

	if (hal_rx_is_buf_addr_info_valid(
				&next_link_desc_addr_info)) {
		/* Clear the next link desc info for the current link_desc */
		hal_rx_clear_next_msdu_link_desc_buf_addr_info(link_desc_va);
		dp_rx_link_desc_return_by_addr(
				soc,
				buf_addr_info,
				HAL_BM_ACTION_PUT_IN_IDLE_LIST);

		hal_rx_buffer_addr_info_get_paddr(
				&next_link_desc_addr_info,
				&buf_info);
		/* buffer_addr_info is the first element of ring_desc */
		hal_rx_buf_cookie_rbm_get(soc->hal_soc,
					  (uint32_t *)&next_link_desc_addr_info,
					  &buf_info);
		link_desc_va =
			dp_rx_cookie_2_link_desc_va(soc, &buf_info);
		cur_link_desc_addr_info = next_link_desc_addr_info;
		buf_addr_info = &cur_link_desc_addr_info;

		goto more_msdu_link_desc;
	}

	dp_rx_link_desc_return_by_addr(soc, buf_addr_info,
				       HAL_BM_ACTION_PUT_IN_IDLE_LIST);
	if (qdf_unlikely(msdu_processed != mpdu_desc_info->msdu_count))
		DP_STATS_INC(soc, rx.err.msdu_count_mismatch, 1);

	return rx_bufs_used;
}

#endif /* QCA_HOST_MODE_WIFI_DISABLED */

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
	if (!hal_rx_attn_msdu_done_get(soc->hal_soc, rx_tlv_hdr)) {

		dp_err_rl("MSDU DONE failure");

		hal_rx_dump_pkt_tlvs(soc->hal_soc, rx_tlv_hdr,
				     QDF_TRACE_LEVEL_INFO);
		qdf_assert(0);
	}

	l2_hdr_offset = hal_rx_msdu_end_l3_hdr_padding_get(soc->hal_soc,
							   rx_tlv_hdr);
	msdu_len = hal_rx_msdu_start_msdu_len_get(soc->hal_soc, rx_tlv_hdr);
	pkt_len = msdu_len + l2_hdr_offset + soc->rx_pkt_tlv_size;

	if (dp_rx_check_pkt_len(soc, pkt_len)) {
		/* Drop & free packet */
		qdf_nbuf_free(nbuf);
		return;
	}
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
		dp_rx_err_info_rl("%pK: INVALID vdev %pK OR osif_rx", soc,
				 vdev);
		/* Drop & free packet */
		qdf_nbuf_free(nbuf);
		DP_STATS_INC(soc, rx.err.invalid_vdev, 1);
		return;
	}

	/*
	 * Advance the packet start pointer by total size of
	 * pre-header TLV's
	 */
	dp_rx_skip_tlvs(soc, nbuf, l2_hdr_offset);

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
			dp_rx_err_info("%pK: mesh pkt filtered", soc);
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
		qdf_nbuf_set_exc_frame(nbuf, 1);
		dp_rx_deliver_to_stack(soc, vdev, peer, nbuf, NULL);
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
		fragno = dp_rx_frag_get_mpdu_frag_number(soc,
							 qdf_nbuf_data(nbuf));
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

/*
 * dp_rx_deliver_to_osif_stack() - function to deliver rx pkts to stack
 * @soc: DP soc
 * @vdv: DP vdev handle
 * @peer: pointer to the peer object
 * @nbuf: skb list head
 * @tail: skb list tail
 * @is_eapol: eapol pkt check
 *
 * Return: None
 */
#ifdef QCA_SUPPORT_EAPOL_OVER_CONTROL_PORT
static inline void
dp_rx_deliver_to_osif_stack(struct dp_soc *soc,
			    struct dp_vdev *vdev,
			    struct dp_peer *peer,
			    qdf_nbuf_t nbuf,
			    qdf_nbuf_t tail,
			    bool is_eapol)
{
	if (is_eapol && soc->eapol_over_control_port)
		dp_rx_eapol_deliver_to_stack(soc, vdev, peer, nbuf, NULL);
	else
		dp_rx_deliver_to_stack(soc, vdev, peer, nbuf, NULL);
}
#else
static inline void
dp_rx_deliver_to_osif_stack(struct dp_soc *soc,
			    struct dp_vdev *vdev,
			    struct dp_peer *peer,
			    qdf_nbuf_t nbuf,
			    qdf_nbuf_t tail,
			    bool is_eapol)
{
	dp_rx_deliver_to_stack(soc, vdev, peer, nbuf, NULL);
}
#endif

/**
 * dp_rx_err_route_hdl() - Function to send EAPOL frames to stack
 *                            Free any other packet which comes in
 *                            this path.
 *
 * @soc: core DP main context
 * @nbuf: buffer pointer
 * @peer: peer handle
 * @rx_tlv_hdr: start of rx tlv header
 * @err_src: rxdma/reo
 *
 * This function indicates EAPOL frame received in wbm error ring to stack.
 * Any other frame should be dropped.
 *
 * Return: SUCCESS if delivered to stack
 */
static void
dp_rx_err_route_hdl(struct dp_soc *soc, qdf_nbuf_t nbuf,
		    struct dp_peer *peer, uint8_t *rx_tlv_hdr,
		    enum hal_rx_wbm_error_source err_src)
{
	uint32_t pkt_len;
	uint16_t msdu_len;
	struct dp_vdev *vdev;
	struct hal_rx_msdu_metadata msdu_metadata;
	bool is_eapol;

	hal_rx_msdu_metadata_get(soc->hal_soc, rx_tlv_hdr, &msdu_metadata);
	msdu_len = hal_rx_msdu_start_msdu_len_get(soc->hal_soc, rx_tlv_hdr);
	pkt_len = msdu_len + msdu_metadata.l3_hdr_pad + soc->rx_pkt_tlv_size;

	if (qdf_likely(!qdf_nbuf_is_frag(nbuf))) {
		if (dp_rx_check_pkt_len(soc, pkt_len))
			goto drop_nbuf;

		/* Set length in nbuf */
		qdf_nbuf_set_pktlen(
			nbuf, qdf_min(pkt_len, (uint32_t)RX_DATA_BUFFER_SIZE));
		qdf_assert_always(nbuf->data == rx_tlv_hdr);
	}

	/*
	 * Check if DMA completed -- msdu_done is the last bit
	 * to be written
	 */
	if (!hal_rx_attn_msdu_done_get(soc->hal_soc, rx_tlv_hdr)) {
		dp_err_rl("MSDU DONE failure");
		hal_rx_dump_pkt_tlvs(soc->hal_soc, rx_tlv_hdr,
				     QDF_TRACE_LEVEL_INFO);
		qdf_assert(0);
	}

	if (!peer)
		goto drop_nbuf;

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
		qdf_nbuf_pull_head(nbuf, soc->rx_pkt_tlv_size);
	else
		qdf_nbuf_pull_head(nbuf, (msdu_metadata.l3_hdr_pad +
				   soc->rx_pkt_tlv_size));

	dp_vdev_peer_stats_update_protocol_cnt(vdev, nbuf, NULL, 0, 1);

	/*
	 * Indicate EAPOL frame to stack only when vap mac address
	 * matches the destination address.
	 */
	is_eapol = qdf_nbuf_is_ipv4_eapol_pkt(nbuf);
	if (is_eapol || qdf_nbuf_is_ipv4_wapi_pkt(nbuf)) {
		qdf_ether_header_t *eh =
			(qdf_ether_header_t *)qdf_nbuf_data(nbuf);
		if (qdf_mem_cmp(eh->ether_dhost, &vdev->mac_addr.raw[0],
				QDF_MAC_ADDR_SIZE) == 0) {
			DP_STATS_INC_PKT(vdev, rx_i.routed_eapol_pkt, 1,
					 qdf_nbuf_len(nbuf));

			/*
			 * Update the protocol tag in SKB based on
			 * CCE metadata.
			 */
			dp_rx_update_protocol_tag(soc, vdev, nbuf, rx_tlv_hdr,
						  EXCEPTION_DEST_RING_ID,
						  true, true);
			/* Update the flow tag in SKB based on FSE metadata */
			dp_rx_update_flow_tag(soc, vdev, nbuf, rx_tlv_hdr,
					      true);
			DP_PEER_TO_STACK_INCC_PKT(peer, 1, qdf_nbuf_len(nbuf),
						  vdev->pdev->enhanced_stats_en);
			qdf_nbuf_set_exc_frame(nbuf, 1);
			qdf_nbuf_set_next(nbuf, NULL);

			dp_rx_deliver_to_osif_stack(soc, vdev, peer, nbuf,
						    NULL, is_eapol);

			return;
		}
	}

drop_nbuf:

	DP_STATS_INCC(soc, rx.reo2rel_route_drop, 1,
		      err_src == HAL_RX_WBM_ERR_SRC_REO);
	DP_STATS_INCC(soc, rx.rxdma2rel_route_drop, 1,
		      err_src == HAL_RX_WBM_ERR_SRC_RXDMA);

	qdf_nbuf_free(nbuf);
}

#ifndef QCA_HOST_MODE_WIFI_DISABLED

#ifdef DP_RX_DESC_COOKIE_INVALIDATE
/**
 * dp_rx_link_cookie_check() - Validate link desc cookie
 * @ring_desc: ring descriptor
 *
 * Return: qdf status
 */
static inline QDF_STATUS
dp_rx_link_cookie_check(hal_ring_desc_t ring_desc)
{
	if (qdf_unlikely(HAL_RX_REO_BUF_LINK_COOKIE_INVALID_GET(ring_desc)))
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_rx_link_cookie_invalidate() - Invalidate link desc cookie
 * @ring_desc: ring descriptor
 *
 * Return: None
 */
static inline void
dp_rx_link_cookie_invalidate(hal_ring_desc_t ring_desc)
{
	HAL_RX_REO_BUF_LINK_COOKIE_INVALID_SET(ring_desc);
}
#else
static inline QDF_STATUS
dp_rx_link_cookie_check(hal_ring_desc_t ring_desc)
{
	return QDF_STATUS_SUCCESS;
}

static inline void
dp_rx_link_cookie_invalidate(hal_ring_desc_t ring_desc)
{
}
#endif

#ifdef WLAN_FEATURE_DP_RX_RING_HISTORY
/**
 * dp_rx_err_ring_record_entry() - Record rx err ring history
 * @soc: Datapath soc structure
 * @paddr: paddr of the buffer in RX err ring
 * @sw_cookie: SW cookie of the buffer in RX err ring
 * @rbm: Return buffer manager of the buffer in RX err ring
 *
 * Returns: None
 */
static inline void
dp_rx_err_ring_record_entry(struct dp_soc *soc, uint64_t paddr,
			    uint32_t sw_cookie, uint8_t rbm)
{
	struct dp_buf_info_record *record;
	uint32_t idx;

	if (qdf_unlikely(!soc->rx_err_ring_history))
		return;

	idx = dp_history_get_next_index(&soc->rx_err_ring_history->index,
					DP_RX_ERR_HIST_MAX);

	/* No NULL check needed for record since its an array */
	record = &soc->rx_err_ring_history->entry[idx];

	record->timestamp = qdf_get_log_timestamp();
	record->hbi.paddr = paddr;
	record->hbi.sw_cookie = sw_cookie;
	record->hbi.rbm = rbm;
}
#else
static inline void
dp_rx_err_ring_record_entry(struct dp_soc *soc, uint64_t paddr,
			    uint32_t sw_cookie, uint8_t rbm)
{
}
#endif

#ifdef HANDLE_RX_REROUTE_ERR
static int dp_rx_err_handle_msdu_buf(struct dp_soc *soc,
				     hal_ring_desc_t ring_desc)
{
	int lmac_id = DP_INVALID_LMAC_ID;
	struct dp_rx_desc *rx_desc;
	struct hal_buf_info hbi;
	struct dp_pdev *pdev;

	hal_rx_reo_buf_paddr_get(soc->hal_soc, ring_desc, &hbi);

	rx_desc = dp_rx_cookie_2_va_rxdma_buf(soc, hbi.sw_cookie);

	/* sanity */
	if (!rx_desc) {
		DP_STATS_INC(soc, rx.err.reo_err_msdu_buf_invalid_cookie, 1);
		goto assert_return;
	}

	if (!rx_desc->nbuf)
		goto assert_return;

	dp_rx_err_ring_record_entry(soc, hbi.paddr,
				    hbi.sw_cookie,
				    hal_rx_ret_buf_manager_get(soc->hal_soc,
							       ring_desc));
	if (hbi.paddr != qdf_nbuf_get_frag_paddr(rx_desc->nbuf, 0)) {
		DP_STATS_INC(soc, rx.err.nbuf_sanity_fail, 1);
		rx_desc->in_err_state = 1;
		goto assert_return;
	}

	/* After this point the rx_desc and nbuf are valid */
	dp_ipa_rx_buf_smmu_mapping_lock(soc);
	qdf_assert_always(!rx_desc->unmapped);
	dp_ipa_handle_rx_buf_smmu_mapping(soc,
					  rx_desc->nbuf,
					  RX_DATA_BUFFER_SIZE,
					  false);
	qdf_nbuf_unmap_nbytes_single(soc->osdev,
				     rx_desc->nbuf,
				     QDF_DMA_FROM_DEVICE,
				     RX_DATA_BUFFER_SIZE);
	rx_desc->unmapped = 1;
	dp_ipa_rx_buf_smmu_mapping_unlock(soc);
	dp_rx_buffer_pool_nbuf_free(soc, rx_desc->nbuf,
				    rx_desc->pool_id);

	pdev = dp_get_pdev_for_lmac_id(soc, rx_desc->pool_id);
	lmac_id = rx_desc->pool_id;
	dp_rx_add_to_free_desc_list(&pdev->free_list_head,
				    &pdev->free_list_tail,
				    rx_desc);
	return lmac_id;

assert_return:
	qdf_assert(0);
	return lmac_id;
}

static int dp_rx_err_exception(struct dp_soc *soc, hal_ring_desc_t ring_desc)
{
	int ret;
	uint64_t cur_time_stamp;

	DP_STATS_INC(soc, rx.err.reo_err_msdu_buf_rcved, 1);

	/* Recover if overall error count exceeds threshold */
	if (soc->stats.rx.err.reo_err_msdu_buf_rcved >
	    DP_MAX_REG_RX_ROUTING_ERRS_THRESHOLD) {
		dp_err("pkt threshold breached! reo_err_msdu_buf_rcved %u first err pkt time_stamp %llu",
		       soc->stats.rx.err.reo_err_msdu_buf_rcved,
		       soc->rx_route_err_start_pkt_ts);
		qdf_trigger_self_recovery(NULL, QDF_RX_REG_PKT_ROUTE_ERR);
	}

	cur_time_stamp = qdf_get_log_timestamp_usecs();
	if (!soc->rx_route_err_start_pkt_ts)
		soc->rx_route_err_start_pkt_ts = cur_time_stamp;

	/* Recover if threshold number of packets received in threshold time */
	if ((cur_time_stamp - soc->rx_route_err_start_pkt_ts) >
						DP_RX_ERR_ROUTE_TIMEOUT_US) {
		soc->rx_route_err_start_pkt_ts = cur_time_stamp;

		if (soc->rx_route_err_in_window >
		    DP_MAX_REG_RX_ROUTING_ERRS_IN_TIMEOUT) {
			qdf_trigger_self_recovery(NULL,
						  QDF_RX_REG_PKT_ROUTE_ERR);
			dp_err("rate threshold breached! reo_err_msdu_buf_rcved %u first err pkt time_stamp %llu",
			       soc->stats.rx.err.reo_err_msdu_buf_rcved,
			       soc->rx_route_err_start_pkt_ts);
		} else {
			soc->rx_route_err_in_window = 1;
		}
	} else {
		soc->rx_route_err_in_window++;
	}

	ret = dp_rx_err_handle_msdu_buf(soc, ring_desc);

	return ret;
}
#else /* HANDLE_RX_REROUTE_ERR */

static int dp_rx_err_exception(struct dp_soc *soc, hal_ring_desc_t ring_desc)
{
	qdf_assert_always(0);

	return DP_INVALID_LMAC_ID;
}
#endif /* HANDLE_RX_REROUTE_ERR */

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
	uint8_t err_status;
	struct hal_rx_mpdu_desc_info mpdu_desc_info;
	struct hal_buf_info hbi;
	struct dp_pdev *dp_pdev;
	struct dp_srng *dp_rxdma_srng;
	struct rx_desc_pool *rx_desc_pool;
	void *link_desc_va;
	struct hal_rx_msdu_list msdu_list; /* MSDU's per MPDU */
	uint16_t num_msdus;
	struct dp_rx_desc *rx_desc = NULL;
	QDF_STATUS status;
	bool ret;
	uint32_t error_code = 0;
	bool sw_pn_check_needed;

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
		dp_rx_err_err("%pK: HAL RING Access Failed -- %pK", soc,
			      hal_ring_hdl);
		goto done;
	}

	while (qdf_likely(quota-- && (ring_desc =
				hal_srng_dst_peek(hal_soc,
						  hal_ring_hdl)))) {

		DP_STATS_INC(soc, rx.err_ring_pkts, 1);
		err_status = hal_rx_err_status_get(hal_soc, ring_desc);
		buf_type = hal_rx_reo_buf_type_get(hal_soc, ring_desc);

		if (err_status == HAL_REO_ERROR_DETECTED)
			error_code = hal_rx_get_reo_error_code(hal_soc,
							       ring_desc);

		qdf_mem_set(&mpdu_desc_info, sizeof(mpdu_desc_info), 0);
		sw_pn_check_needed = dp_rx_err_is_pn_check_needed(soc,
								  err_status,
								  error_code);
		if (!sw_pn_check_needed) {
			/*
			 * MPDU desc info will be present in the REO desc
			 * only in the below scenarios
			 * 1) pn_in_dest_disabled:  always
			 * 2) pn_in_dest enabled: All cases except 2k-jup
			 *			and OOR errors
			 */
			hal_rx_mpdu_desc_info_get(hal_soc, ring_desc,
						  &mpdu_desc_info);
		}

		if (HAL_RX_REO_DESC_MSDU_COUNT_GET(ring_desc) == 0)
			goto next_entry;

		/*
		 * For REO error ring, only MSDU LINK DESC is expected.
		 * Handle HAL_RX_REO_MSDU_BUF_ADDR_TYPE exception case.
		 */
		if (qdf_unlikely(buf_type != HAL_RX_REO_MSDU_LINK_DESC_TYPE)) {
			int lmac_id;

			lmac_id = dp_rx_err_exception(soc, ring_desc);
			if (lmac_id >= 0)
				rx_bufs_reaped[lmac_id] += 1;
			goto next_entry;
		}

		hal_rx_buf_cookie_rbm_get(hal_soc, (uint32_t *)ring_desc,
					  &hbi);
		/*
		 * check for the magic number in the sw cookie
		 */
		qdf_assert_always((hbi.sw_cookie >> LINK_DESC_ID_SHIFT) &
					soc->link_desc_id_start);

		status = dp_rx_link_cookie_check(ring_desc);
		if (qdf_unlikely(QDF_IS_STATUS_ERROR(status))) {
			DP_STATS_INC(soc, rx.err.invalid_link_cookie, 1);
			break;
		}

		hal_rx_reo_buf_paddr_get(soc->hal_soc, ring_desc, &hbi);
		link_desc_va = dp_rx_cookie_2_link_desc_va(soc, &hbi);
		hal_rx_msdu_list_get(soc->hal_soc, link_desc_va, &msdu_list,
				     &num_msdus);
		dp_rx_err_ring_record_entry(soc, msdu_list.paddr[0],
					    msdu_list.sw_cookie[0],
					    msdu_list.rbm[0]);
		// TODO - BE- Check if the RBM is to be checked for all chips
		if (qdf_unlikely((msdu_list.rbm[0] !=
					dp_rx_get_rx_bm_id(soc)) &&
				 (msdu_list.rbm[0] !=
				  soc->idle_link_bm_id) &&
				 (msdu_list.rbm[0] !=
					dp_rx_get_defrag_bm_id(soc)))) {
			/* TODO */
			/* Call appropriate handler */
			if (!wlan_cfg_get_dp_soc_nss_cfg(soc->wlan_cfg_ctx)) {
				DP_STATS_INC(soc, rx.err.invalid_rbm, 1);
				dp_rx_err_err("%pK: Invalid RBM %d",
					      soc, msdu_list.rbm[0]);
			}

			/* Return link descriptor through WBM ring (SW2WBM)*/
			dp_rx_link_desc_return(soc, ring_desc,
					HAL_BM_ACTION_RELEASE_MSDU_LIST);
			goto next_entry;
		}

		rx_desc = soc->arch_ops.dp_rx_desc_cookie_2_va(
						soc,
						msdu_list.sw_cookie[0]);
		qdf_assert_always(rx_desc);

		mac_id = rx_desc->pool_id;

		if (sw_pn_check_needed) {
			goto process_reo_error_code;
		}

		if (mpdu_desc_info.bar_frame) {
			qdf_assert_always(mpdu_desc_info.msdu_count == 1);

			dp_rx_bar_frame_handle(soc, ring_desc, rx_desc,
					       &mpdu_desc_info, err_status,
					       error_code);

			rx_bufs_reaped[mac_id] += 1;
			goto next_entry;
		}

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
				goto next_entry;
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
				/* ignore duplicate RX desc and continue */
				/* Pop out the descriptor */
				goto next_entry;
			}

			ret = dp_rx_desc_paddr_sanity_check(rx_desc,
							    msdu_list.paddr[0]);
			if (!ret) {
				DP_STATS_INC(soc, rx.err.nbuf_sanity_fail, 1);
				rx_desc->in_err_state = 1;
				goto next_entry;
			}

			count = dp_rx_frag_handle(soc,
						  ring_desc, &mpdu_desc_info,
						  rx_desc, &mac_id, quota);

			rx_bufs_reaped[mac_id] += count;
			DP_STATS_INC(soc, rx.rx_frags, 1);
			goto next_entry;
		}

process_reo_error_code:
		/*
		 * Expect REO errors to be handled after this point
		 */
		qdf_assert_always(err_status == HAL_REO_ERROR_DETECTED);

		dp_info_rl("Got pkt with REO ERROR: %d", error_code);

		switch (error_code) {
		case HAL_REO_ERR_PN_CHECK_FAILED:
		case HAL_REO_ERR_PN_ERROR_HANDLING_FLAG_SET:
			DP_STATS_INC(soc, rx.err.reo_error[error_code], 1);
			dp_pdev = dp_get_pdev_for_lmac_id(soc, mac_id);
			if (dp_pdev)
				DP_STATS_INC(dp_pdev, err.reo_error, 1);
			count = dp_rx_pn_error_handle(soc,
						      ring_desc,
						      &mpdu_desc_info, &mac_id,
						      quota);

			rx_bufs_reaped[mac_id] += count;
			break;
		case HAL_REO_ERR_REGULAR_FRAME_2K_JUMP:
		case HAL_REO_ERR_2K_ERROR_HANDLING_FLAG_SET:
		case HAL_REO_ERR_BAR_FRAME_2K_JUMP:
		case HAL_REO_ERR_REGULAR_FRAME_OOR:
		case HAL_REO_ERR_BAR_FRAME_OOR:
		case HAL_REO_ERR_QUEUE_DESC_ADDR_0:
			DP_STATS_INC(soc, rx.err.reo_error[error_code], 1);
			dp_pdev = dp_get_pdev_for_lmac_id(soc, mac_id);
			if (dp_pdev)
				DP_STATS_INC(dp_pdev, err.reo_error, 1);
			count = dp_rx_reo_err_entry_process(
					soc,
					ring_desc,
					&mpdu_desc_info,
					link_desc_va,
					error_code);

			rx_bufs_reaped[mac_id] += count;
			break;
		case HAL_REO_ERR_QUEUE_DESC_INVALID:
		case HAL_REO_ERR_AMPDU_IN_NON_BA:
		case HAL_REO_ERR_NON_BA_DUPLICATE:
		case HAL_REO_ERR_BA_DUPLICATE:
		case HAL_REO_ERR_BAR_FRAME_NO_BA_SESSION:
		case HAL_REO_ERR_BAR_FRAME_SN_EQUALS_SSN:
		case HAL_REO_ERR_QUEUE_DESC_BLOCKED_SET:
			DP_STATS_INC(soc, rx.err.reo_error[error_code], 1);
			count = dp_rx_msdus_drop(soc, ring_desc,
						 &mpdu_desc_info,
						 &mac_id, quota);
			rx_bufs_reaped[mac_id] += count;
			break;
		default:
			/* Assert if unexpected error type */
			qdf_assert_always(0);
		}
next_entry:
		dp_rx_link_cookie_invalidate(ring_desc);
		hal_srng_dst_get_next(hal_soc, hal_ring_hdl);
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

#ifdef DROP_RXDMA_DECRYPT_ERR
/**
 * dp_handle_rxdma_decrypt_err() - Check if decrypt err frames can be handled
 *
 * Return: true if rxdma decrypt err frames are handled and false otheriwse
 */
static inline bool dp_handle_rxdma_decrypt_err(void)
{
	return false;
}
#else
static inline bool dp_handle_rxdma_decrypt_err(void)
{
	return true;
}
#endif

static inline bool
dp_rx_is_sg_formation_required(struct hal_wbm_err_desc_info *info)
{
	/*
	 * Currently Null Queue and Unencrypted error handlers has support for
	 * SG. Other error handler do not deal with SG buffer.
	 */
	if (((info->wbm_err_src == HAL_RX_WBM_ERR_SRC_REO) &&
	     (info->reo_err_code == HAL_REO_ERR_QUEUE_DESC_ADDR_0)) ||
	    ((info->wbm_err_src == HAL_RX_WBM_ERR_SRC_RXDMA) &&
	     (info->rxdma_err_code == HAL_RXDMA_ERR_UNENCRYPTED)))
		return true;

	return false;
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
	uint8_t buf_type;
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
	uint8_t msdu_continuation = 0;
	bool process_sg_buf = false;
	uint32_t wbm_err_src;

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
		dp_rx_err_err("%pK: HAL RING Access Failed -- %pK",
			      soc, hal_ring_hdl);
		goto done;
	}

	while (qdf_likely(quota)) {
		ring_desc = hal_srng_dst_get_next(hal_soc, hal_ring_hdl);
		if (qdf_unlikely(!ring_desc))
			break;

		/* XXX */
		buf_type = HAL_RX_WBM_BUF_TYPE_GET(ring_desc);

		/*
		 * For WBM ring, expect only MSDU buffers
		 */
		qdf_assert_always(buf_type == HAL_RX_WBM_BUF_TYPE_REL_BUF);

		wbm_err_src = hal_rx_wbm_err_src_get(hal_soc, ring_desc);
		qdf_assert((wbm_err_src == HAL_RX_WBM_ERR_SRC_RXDMA) ||
			   (wbm_err_src == HAL_RX_WBM_ERR_SRC_REO));

		if (soc->arch_ops.dp_wbm_get_rx_desc_from_hal_desc(soc,
								   ring_desc,
								   &rx_desc)) {
			dp_rx_err_err("get rx desc from hal_desc failed");
			continue;
		}

		qdf_assert_always(rx_desc);

		if (!dp_rx_desc_check_magic(rx_desc)) {
			dp_rx_err_err("%pk: Invalid rx_desc %pk",
				      soc, rx_desc);
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
			continue;
		}

		hal_rx_wbm_err_info_get(ring_desc, &wbm_err_info, hal_soc);
		nbuf = rx_desc->nbuf;
		rx_desc_pool = &soc->rx_desc_buf[rx_desc->pool_id];
		dp_ipa_rx_buf_smmu_mapping_lock(soc);
		dp_ipa_handle_rx_buf_smmu_mapping(soc, nbuf,
						  rx_desc_pool->buf_size,
						  false);
		qdf_nbuf_unmap_nbytes_single(soc->osdev, nbuf,
					     QDF_DMA_FROM_DEVICE,
					     rx_desc_pool->buf_size);
		rx_desc->unmapped = 1;
		dp_ipa_rx_buf_smmu_mapping_unlock(soc);

		if (qdf_unlikely(soc->wbm_release_desc_rx_sg_support &&
				 dp_rx_is_sg_formation_required(&wbm_err_info))) {
			/* SG is detected from continuation bit */
			msdu_continuation =
				hal_rx_wbm_err_msdu_continuation_get(hal_soc,
								     ring_desc);
			if (msdu_continuation &&
			    !(soc->wbm_sg_param.wbm_is_first_msdu_in_sg)) {
				/* Update length from first buffer in SG */
				soc->wbm_sg_param.wbm_sg_desc_msdu_len =
					hal_rx_msdu_start_msdu_len_get(
						soc->hal_soc,
						qdf_nbuf_data(nbuf));
				soc->wbm_sg_param.wbm_is_first_msdu_in_sg = true;
			}

			if (msdu_continuation) {
				/* MSDU continued packets */
				qdf_nbuf_set_rx_chfrag_cont(nbuf, 1);
				QDF_NBUF_CB_RX_PKT_LEN(nbuf) =
					soc->wbm_sg_param.wbm_sg_desc_msdu_len;
			} else {
				/* This is the terminal packet in SG */
				qdf_nbuf_set_rx_chfrag_start(nbuf, 1);
				qdf_nbuf_set_rx_chfrag_end(nbuf, 1);
				QDF_NBUF_CB_RX_PKT_LEN(nbuf) =
					soc->wbm_sg_param.wbm_sg_desc_msdu_len;
				process_sg_buf = true;
			}
		}

		/*
		 * save the wbm desc info in nbuf TLV. We will need this
		 * info when we do the actual nbuf processing
		 */
		wbm_err_info.pool_id = rx_desc->pool_id;
		hal_rx_priv_info_set_in_tlv(soc->hal_soc,
					    qdf_nbuf_data(nbuf),
					    (uint8_t *)&wbm_err_info,
					    sizeof(wbm_err_info));

		rx_bufs_reaped[rx_desc->pool_id]++;

		if (qdf_nbuf_is_rx_chfrag_cont(nbuf) || process_sg_buf) {
			DP_RX_LIST_APPEND(soc->wbm_sg_param.wbm_sg_nbuf_head,
					  soc->wbm_sg_param.wbm_sg_nbuf_tail,
					  nbuf);
			if (process_sg_buf) {
				if (!dp_rx_buffer_pool_refill(
					soc,
					soc->wbm_sg_param.wbm_sg_nbuf_head,
					rx_desc->pool_id))
					DP_RX_MERGE_TWO_LIST(
						nbuf_head, nbuf_tail,
						soc->wbm_sg_param.wbm_sg_nbuf_head,
						soc->wbm_sg_param.wbm_sg_nbuf_tail);
				dp_rx_wbm_sg_list_reset(soc);
				process_sg_buf = false;
			}
		} else if (!dp_rx_buffer_pool_refill(soc, nbuf,
						     rx_desc->pool_id)) {
			DP_RX_LIST_APPEND(nbuf_head, nbuf_tail, nbuf);
		}

		dp_rx_add_to_free_desc_list(&head[rx_desc->pool_id],
						&tail[rx_desc->pool_id],
						rx_desc);

		/*
		 * if continuation bit is set then we have MSDU spread
		 * across multiple buffers, let us not decrement quota
		 * till we reap all buffers of that MSDU.
		 */
		if (qdf_likely(!msdu_continuation))
			quota -= 1;
	}
done:
	dp_srng_access_end(int_ctx, soc, hal_ring_hdl);

	for (mac_id = 0; mac_id < MAX_PDEV_CNT; mac_id++) {
		if (rx_bufs_reaped[mac_id]) {
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
		uint8_t err_code;
		uint8_t *tlv_hdr;
		uint32_t peer_meta_data;
		rx_tlv_hdr = qdf_nbuf_data(nbuf);

		/*
		 * retrieve the wbm desc info from nbuf TLV, so we can
		 * handle error cases appropriately
		 */
		hal_rx_priv_info_get_from_tlv(soc->hal_soc, rx_tlv_hdr,
					      (uint8_t *)&wbm_err_info,
					      sizeof(wbm_err_info));

		peer_meta_data = hal_rx_mpdu_peer_meta_data_get(soc->hal_soc,
								rx_tlv_hdr);
		peer_id = dp_rx_peer_metadata_peer_id_get(soc, peer_meta_data);
		peer = dp_peer_get_ref_by_id(soc, peer_id, DP_MOD_ID_RX_ERR);

		if (!peer)
			dp_info_rl("peer is null peer_id%u err_src%u err_rsn%u",
				   peer_id, wbm_err_info.wbm_err_src,
				   wbm_err_info.reo_psh_rsn);

		/* Set queue_mapping in nbuf to 0 */
		dp_set_rx_queue(nbuf, 0);

		next = nbuf->next;

		/*
		 * Form the SG for msdu continued buffers
		 * QCN9000 has this support
		 */
		if (qdf_nbuf_is_rx_chfrag_cont(nbuf)) {
			nbuf = dp_rx_sg_create(soc, nbuf);
			next = nbuf->next;
			/*
			 * SG error handling is not done correctly,
			 * drop SG frames for now.
			 */
			qdf_nbuf_free(nbuf);
			dp_info_rl("scattered msdu dropped");
			nbuf = next;
			if (peer)
				dp_peer_unref_delete(peer, DP_MOD_ID_RX_ERR);
			continue;
		}

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
					break;
				/* TODO */
				/* Add per error code accounting */
				case HAL_REO_ERR_REGULAR_FRAME_2K_JUMP:
					if (peer)
						DP_STATS_INC(peer,
							     rx.err.jump_2k_err,
							     1);

					pool_id = wbm_err_info.pool_id;

					if (hal_rx_msdu_end_first_msdu_get(soc->hal_soc,
									   rx_tlv_hdr)) {
						tid =
						hal_rx_mpdu_start_tid_get(hal_soc, rx_tlv_hdr);
					}
					QDF_NBUF_CB_RX_PKT_LEN(nbuf) =
					hal_rx_msdu_start_msdu_len_get(
						soc->hal_soc, rx_tlv_hdr);
					nbuf->next = NULL;
					dp_2k_jump_handle(soc, nbuf,
							  rx_tlv_hdr,
							  peer_id, tid);
					break;
				case HAL_REO_ERR_REGULAR_FRAME_OOR:
					if (peer)
						DP_STATS_INC(peer,
							     rx.err.oor_err, 1);
					if (hal_rx_msdu_end_first_msdu_get(soc->hal_soc,
									   rx_tlv_hdr)) {
						tid =
							hal_rx_mpdu_start_tid_get(hal_soc, rx_tlv_hdr);
					}
					QDF_NBUF_CB_RX_PKT_LEN(nbuf) =
						hal_rx_msdu_start_msdu_len_get(
						soc->hal_soc, rx_tlv_hdr);
					nbuf->next = NULL;
					dp_rx_oor_handle(soc, nbuf,
							 peer_id,
							 rx_tlv_hdr);
					break;
				case HAL_REO_ERR_BAR_FRAME_2K_JUMP:
				case HAL_REO_ERR_BAR_FRAME_OOR:
					if (peer)
						dp_rx_err_handle_bar(soc,
								     peer,
								     nbuf);
					qdf_nbuf_free(nbuf);
					break;

				case HAL_REO_ERR_PN_CHECK_FAILED:
				case HAL_REO_ERR_PN_ERROR_HANDLING_FLAG_SET:
					if (peer)
						DP_STATS_INC(peer,
							     rx.err.pn_err, 1);
					qdf_nbuf_free(nbuf);
					break;

				default:
					dp_info_rl("Got pkt with REO ERROR: %d",
						   wbm_err_info.reo_err_code);
					qdf_nbuf_free(nbuf);
				}
			} else if (wbm_err_info.reo_psh_rsn
					== HAL_RX_WBM_REO_PSH_RSN_ROUTE) {
				dp_rx_err_route_hdl(soc, nbuf, peer,
						    rx_tlv_hdr,
						    HAL_RX_WBM_ERR_SRC_REO);
			} else {
				/* should not enter here */
				dp_rx_err_alert("invalid reo push reason %u",
						wbm_err_info.reo_psh_rsn);
				qdf_nbuf_free(nbuf);
				qdf_assert_always(0);
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
					if (peer)
						DP_STATS_INC(peer,
							     rx.err.rxdma_wifi_parse_err,
							     1);

					pool_id = wbm_err_info.pool_id;
					dp_rx_process_rxdma_err(soc, nbuf,
								rx_tlv_hdr,
								peer,
								wbm_err_info.
								rxdma_err_code,
								pool_id);
					break;

				case HAL_RXDMA_ERR_TKIP_MIC:
					dp_rx_process_mic_error(soc, nbuf,
								rx_tlv_hdr,
								peer);
					if (peer)
						DP_STATS_INC(peer, rx.err.mic_err, 1);
					break;

				case HAL_RXDMA_ERR_DECRYPT:

					if (peer) {
						DP_STATS_INC(peer, rx.err.
							     decrypt_err, 1);
						qdf_nbuf_free(nbuf);
						break;
					}

					if (!dp_handle_rxdma_decrypt_err()) {
						qdf_nbuf_free(nbuf);
						break;
					}

					pool_id = wbm_err_info.pool_id;
					err_code = wbm_err_info.rxdma_err_code;
					tlv_hdr = rx_tlv_hdr;
					dp_rx_process_rxdma_err(soc, nbuf,
								tlv_hdr, NULL,
								err_code,
								pool_id);
					break;
				case HAL_RXDMA_MULTICAST_ECHO:
					DP_STATS_INC_PKT(peer, rx.mec_drop, 1,
							 qdf_nbuf_len(nbuf));
					qdf_nbuf_free(nbuf);
					break;
				default:
					qdf_nbuf_free(nbuf);
					dp_err_rl("RXDMA error %d",
						  wbm_err_info.rxdma_err_code);
				}
			} else if (wbm_err_info.rxdma_psh_rsn
					== HAL_RX_WBM_RXDMA_PSH_RSN_ROUTE) {
				dp_rx_err_route_hdl(soc, nbuf, peer,
						    rx_tlv_hdr,
						    HAL_RX_WBM_ERR_SRC_RXDMA);
			} else if (wbm_err_info.rxdma_psh_rsn
					== HAL_RX_WBM_RXDMA_PSH_RSN_FLUSH) {
				dp_rx_err_err("rxdma push reason %u",
						wbm_err_info.rxdma_psh_rsn);
				DP_STATS_INC(soc, rx.err.rx_flush_count, 1);
				qdf_nbuf_free(nbuf);
			} else {
				/* should not enter here */
				dp_rx_err_alert("invalid rxdma push reason %u",
						wbm_err_info.rxdma_psh_rsn);
				qdf_nbuf_free(nbuf);
				qdf_assert_always(0);
			}
		} else {
			/* Should not come here */
			qdf_assert(0);
		}

		if (peer)
			dp_peer_unref_delete(peer, DP_MOD_ID_RX_ERR);

		nbuf = next;
	}
	return rx_bufs_used; /* Assume no scale factor for now */
}

#endif /* QCA_HOST_MODE_WIFI_DISABLED */

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
	struct rx_desc_pool *rx_desc_pool;

	if (!pdev) {
		dp_rx_err_debug("%pK: pdev is null for mac_id = %d",
				soc, mac_id);
		return rx_bufs_used;
	}

	msdu = 0;

	last = NULL;

	hal_rx_reo_ent_buf_paddr_get(soc->hal_soc, rxdma_dst_ring_desc,
				     &buf_info, &msdu_cnt);

	push_reason =
		hal_rx_reo_ent_rxdma_push_reason_get(rxdma_dst_ring_desc);
	if (push_reason == HAL_RX_WBM_RXDMA_PSH_RSN_ERROR) {
		rxdma_error_code =
			hal_rx_reo_ent_rxdma_error_code_get(rxdma_dst_ring_desc);
	}

	do {
		rx_msdu_link_desc =
			dp_rx_cookie_2_link_desc_va(soc, &buf_info);

		qdf_assert_always(rx_msdu_link_desc);

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
			if (msdu_list.rbm[0] !=
				HAL_RX_BUF_RBM_SW3_BM(soc->wbm_sw0_bm_id) &&
			    wlan_cfg_get_dp_pdev_nss_enabled(
							pdev->wlan_cfg_ctx))
				bm_action = HAL_BM_ACTION_RELEASE_MSDU_LIST;
			else {
				for (i = 0; i < num_msdus; i++) {
					struct dp_rx_desc *rx_desc =
						soc->arch_ops.
						dp_rx_desc_cookie_2_va(
							soc,
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

					rx_desc_pool = &soc->
						rx_desc_buf[rx_desc->pool_id];
					dp_ipa_rx_buf_smmu_mapping_lock(soc);
					dp_ipa_handle_rx_buf_smmu_mapping(
							soc, msdu,
							rx_desc_pool->buf_size,
							false);
					qdf_nbuf_unmap_nbytes_single(
						soc->osdev, msdu,
						QDF_DMA_FROM_DEVICE,
						rx_desc_pool->buf_size);
					rx_desc->unmapped = 1;
					dp_ipa_rx_buf_smmu_mapping_unlock(soc);

					dp_rx_err_debug("%pK: msdu_nbuf=%pK ",
							soc, msdu);

					dp_rx_buffer_pool_nbuf_free(soc, msdu,
							rx_desc->pool_id);
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
		hal_rxdma_buff_addr_info_set(soc->hal_soc, rx_link_buf_info,
					     buf_info.paddr, buf_info.sw_cookie,
					     buf_info.rbm);

		hal_rx_mon_next_link_desc_get(soc->hal_soc, rx_msdu_link_desc,
					      &buf_info);
		dp_rx_link_desc_return_by_addr(soc,
					       (hal_buff_addrinfo_t)
						rx_link_buf_info,
						bm_action);
	} while (buf_info.paddr);

	DP_STATS_INC(soc, rx.err.rxdma_error[rxdma_error_code], 1);
	if (pdev)
		DP_STATS_INC(pdev, err.rxdma_error, 1);

	if (rxdma_error_code == HAL_RXDMA_ERR_DECRYPT) {
		dp_rx_err_err("%pK: Packet received with Decrypt error", soc);
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
		dp_rx_err_err("%pK: HAL Monitor Destination Ring Init Failed -- %pK",
			      soc, err_dst_srng);
		return 0;
	}

	hal_soc = soc->hal_soc;

	qdf_assert(hal_soc);

	if (qdf_unlikely(dp_srng_access_start(int_ctx, soc, err_dst_srng))) {
		dp_rx_err_err("%pK: HAL Monitor Destination Ring Init Failed -- %pK",
			      soc, err_dst_srng);
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
		if (wlan_cfg_per_pdev_lmac_ring(soc->wlan_cfg_ctx)) {
			dp_rxdma_srng = &soc->rx_refill_buf_ring[mac_id];
			rx_desc_pool = &soc->rx_desc_buf[mac_id];
		} else {
			dp_rxdma_srng = &soc->rx_refill_buf_ring[pdev->lmac_id];
			rx_desc_pool = &soc->rx_desc_buf[pdev->lmac_id];
		}

		dp_rx_buffers_replenish(soc, mac_id, dp_rxdma_srng,
			rx_desc_pool, rx_bufs_used, &head, &tail);

		work_done += rx_bufs_used;
	}

	return work_done;
}

#ifndef QCA_HOST_MODE_WIFI_DISABLED

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
	struct rx_desc_pool *rx_desc_pool;

	msdu = 0;

	last = NULL;

	hal_rx_reo_ent_buf_paddr_get(soc->hal_soc, rxdma_dst_ring_desc,
				     &buf_info, &msdu_cnt);

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
					soc->arch_ops.dp_rx_desc_cookie_2_va(
							soc,
							msdu_list.sw_cookie[i]);
				qdf_assert_always(rx_desc);
				rx_desc_pool =
					&soc->rx_desc_buf[rx_desc->pool_id];
				msdu = rx_desc->nbuf;

				dp_ipa_rx_buf_smmu_mapping_lock(soc);
				dp_ipa_handle_rx_buf_smmu_mapping(
						soc, msdu,
						rx_desc_pool->buf_size,
						false);

				qdf_nbuf_unmap_nbytes_single(
							soc->osdev,
							msdu,
							QDF_DMA_FROM_DEVICE,
							rx_desc_pool->buf_size);
				rx_desc->unmapped = 1;
				dp_ipa_rx_buf_smmu_mapping_unlock(soc);

				dp_rx_buffer_pool_nbuf_free(soc, msdu,
							    rx_desc->pool_id);
				rx_bufs_used++;
				dp_rx_add_to_free_desc_list(head,
							    tail, rx_desc);
			}
		}

		/*
		 * Store the current link buffer into to the local structure
		 * to be used for release purpose.
		 */
		hal_rxdma_buff_addr_info_set(soc->hal_soc, rx_link_buf_info,
					     buf_info.paddr, buf_info.sw_cookie,
					     buf_info.rbm);

		hal_rx_mon_next_link_desc_get(soc->hal_soc, rx_msdu_link_desc,
					      &buf_info);
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
	struct dp_rx_desc *rx_desc = NULL;
	struct rx_desc_pool *rx_desc_pool;
	uint32_t rx_bufs_reaped = 0;
	union dp_rx_desc_list_elem_t *head = NULL;
	union dp_rx_desc_list_elem_t *tail = NULL;
	uint8_t pool_id;

	hal_rx_reo_buf_paddr_get(soc->hal_soc, hal_desc, &buf_info);

	if (!buf_info.paddr) {
		DP_STATS_INC(soc, tx.wbm_internal_error[WBM_INT_ERROR_REO_NULL_BUFFER], 1);
		return;
	}

	/* buffer_addr_info is the first element of ring_desc */
	hal_rx_buf_cookie_rbm_get(soc->hal_soc, (uint32_t *)hal_desc,
				  &buf_info);
	pool_id = DP_RX_DESC_COOKIE_POOL_ID_GET(buf_info.sw_cookie);

	if (buf_type == HAL_WBM_RELEASE_RING_2_BUFFER_TYPE) {
		DP_STATS_INC(soc, tx.wbm_internal_error[WBM_INT_ERROR_REO_NULL_MSDU_BUFF], 1);
		rx_desc = soc->arch_ops.dp_rx_desc_cookie_2_va(
							soc,
							buf_info.sw_cookie);

		if (rx_desc && rx_desc->nbuf) {
			rx_desc_pool = &soc->rx_desc_buf[rx_desc->pool_id];
			dp_ipa_rx_buf_smmu_mapping_lock(soc);
			dp_ipa_handle_rx_buf_smmu_mapping(
						soc, rx_desc->nbuf,
						rx_desc_pool->buf_size,
						false);
			qdf_nbuf_unmap_nbytes_single(soc->osdev, rx_desc->nbuf,
						     QDF_DMA_FROM_DEVICE,
						     rx_desc_pool->buf_size);
			rx_desc->unmapped = 1;
			dp_ipa_rx_buf_smmu_mapping_unlock(soc);

			dp_rx_buffer_pool_nbuf_free(soc, rx_desc->nbuf,
						    rx_desc->pool_id);
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
		dp_rxdma_srng = &soc->rx_refill_buf_ring[pool_id];
		rx_desc_pool = &soc->rx_desc_buf[pool_id];

		dp_rx_buffers_replenish(soc, pool_id, dp_rxdma_srng,
					rx_desc_pool,
					rx_bufs_reaped,
					&head, &tail);
	}
}

#endif /* QCA_HOST_MODE_WIFI_DISABLED */
