
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

#ifndef _DP_TXRX_WDS_H_
#define _DP_TXRX_WDS_H_

/* WDS AST entry aging timer value */
#define DP_WDS_AST_AGING_TIMER_DEFAULT_MS	120000
#define DP_WDS_AST_AGING_TIMER_CNT \
((DP_WDS_AST_AGING_TIMER_DEFAULT_MS / DP_AST_AGING_TIMER_DEFAULT_MS) - 1)
void dp_soc_wds_attach(struct dp_soc *soc);
void dp_soc_wds_detach(struct dp_soc *soc);

void
dp_rx_da_learn(struct dp_soc *soc,
	       uint8_t *rx_tlv_hdr,
	       struct dp_peer *ta_peer,
	       qdf_nbuf_t nbuf);

void dp_tx_mec_handler(struct dp_vdev *vdev, uint8_t *status);
#ifdef FEATURE_AST
/*
 * dp_peer_delete_ast_entries(): Delete all AST entries for a peer
 * @soc - datapath soc handle
 * @peer - datapath peer handle
 *
 * Delete the AST entries belonging to a peer
 */
static inline void dp_peer_delete_ast_entries(struct dp_soc *soc,
					      struct dp_peer *peer)
{
	struct dp_ast_entry *ast_entry, *temp_ast_entry;

	qdf_spin_lock_bh(&soc->ast_lock);
	DP_PEER_ITERATE_ASE_LIST(peer, ast_entry, temp_ast_entry)
		dp_peer_del_ast(soc, ast_entry);

	peer->self_ast_entry = NULL;
	qdf_spin_unlock_bh(&soc->ast_lock);
}
static void dp_peer_teardown_wifi3(struct cdp_vdev *vdev_hdl, void *peer_hdl)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_hdl;
	struct dp_peer *peer = (struct dp_peer *)peer_hdl;
	struct dp_soc *soc = (struct dp_soc *)vdev->pdev->soc;

	/*
	 * For BSS peer, new peer is not created on alloc_node if the
	 * peer with same address already exists , instead refcnt is
	 * increased for existing peer. Correspondingly in delete path,
	 * only refcnt is decreased; and peer is only deleted , when all
	 * references are deleted. So delete_in_progress should not be set
	 * for bss_peer, unless only 2 reference remains (peer map reference
	 * and peer hash table reference).
	 */
	if (peer->bss_peer && (qdf_atomic_read(&peer->ref_cnt) > 2))
		return;

	peer->delete_in_progress = true;
	dp_peer_delete_ast_entries(soc, peer);
}
#endif
#ifdef FEATURE_WDS
static inline bool dp_tx_da_search_override(struct dp_vdev *vdev)
{
	struct dp_soc *soc = vdev->pdev->soc;

	/*
	 * If AST index override support is available (HKv2 etc),
	 * DA search flag be enabled always
	 *
	 * If AST index override support is not available (HKv1),
	 * DA search flag should be used for all modes except QWRAP
	 */
	if (soc->ast_override_support || !vdev->proxysta_vdev)
		return true;

	return false;
}
#endif
#ifdef WDS_VENDOR_EXTENSION
void
dp_txrx_peer_wds_tx_policy_update(struct cdp_peer *peer_handle,
				  int wds_tx_ucast, int wds_tx_mcast);
void
dp_txrx_set_wds_rx_policy(struct cdp_vdev *vdev_handle,
			  u_int32_t val);
#endif

/**
 * dp_rx_wds_add_or_update_ast() - Add or update the ast entry.
 *
 * @soc: core txrx main context
 * @ta_peer: WDS repeater peer
 * @mac_addr: mac address of the peer
 * @is_ad4_valid: 4-address valid flag
 * @is_sa_valid: source address valid flag
 * @is_chfrag_start: frag start flag
 * @sa_idx: source-address index for peer
 * @sa_sw_peer_id: software source-address peer-id
 *
 * Return: void:
 */
static inline void
dp_rx_wds_add_or_update_ast(struct dp_soc *soc, struct dp_peer *ta_peer,
			    uint8_t *wds_src_mac, uint8_t is_ad4_valid,
			    uint8_t is_sa_valid, uint8_t is_chfrag_start,
			    uint16_t sa_idx, uint16_t sa_sw_peer_id)
{
	struct dp_peer *sa_peer;
	struct dp_ast_entry *ast;
	uint32_t flags = IEEE80211_NODE_F_WDS_HM;
	uint32_t ret = 0;
	struct dp_neighbour_peer *neighbour_peer = NULL;
	struct dp_pdev *pdev = ta_peer->vdev->pdev;

	/* For AP mode : Do wds source port learning only if it is a
	 * 4-address mpdu
	 *
	 * For STA mode : Frames from RootAP backend will be in 3-address mode,
	 * till RootAP does the WDS source port learning; Hence in repeater/STA
	 * mode, we enable learning even in 3-address mode , to avoid RootAP
	 * backbone getting wrongly learnt as MEC on repeater
	 */
	if (ta_peer->vdev->opmode != wlan_op_mode_sta) {
		if (!(is_chfrag_start && is_ad4_valid))
			return;
	} else {
		/* For HKv2 Source port learing is not needed in STA mode
		 * as we have support in HW
		 */
		if (soc->ast_override_support)
			return;
	}

	if (qdf_unlikely(!is_sa_valid)) {
		ret = dp_peer_add_ast(soc, ta_peer, wds_src_mac,
				      CDP_TXRX_AST_TYPE_WDS, flags);
		return;
	}

	qdf_spin_lock_bh(&soc->ast_lock);
	ast = soc->ast_table[sa_idx];
	qdf_spin_unlock_bh(&soc->ast_lock);

	if (!ast) {
		/*
		 * In HKv1, it is possible that HW retains the AST entry in
		 * GSE cache on 1 radio , even after the AST entry is deleted
		 * (on another radio).
		 *
		 * Due to this, host might still get sa_is_valid indications
		 * for frames with SA not really present in AST table.
		 *
		 * So we go ahead and send an add_ast command to FW in such
		 * cases where sa is reported still as valid, so that FW will
		 * invalidate this GSE cache entry and new AST entry gets
		 * cached.
		 */
		if (!soc->ast_override_support) {
			ret = dp_peer_add_ast(soc, ta_peer, wds_src_mac,
					      CDP_TXRX_AST_TYPE_WDS, flags);
			return;
		}
		if (soc->ast_override_support) {
			/* In HKv2 smart monitor case, when NAC client is
			 * added first and this client roams within BSS to
			 * connect to RE, since we have an AST entry for
			 * NAC we get sa_is_valid bit set. So we check if
			 * smart monitor is enabled and send add_ast command
			 * to FW.
			 */
			if (pdev->neighbour_peers_added) {
				qdf_spin_lock_bh(&pdev->neighbour_peer_mutex);
				TAILQ_FOREACH(neighbour_peer,
					      &pdev->neighbour_peers_list,
					      neighbour_peer_list_elem) {
					if (!qdf_mem_cmp(&neighbour_peer->
							 neighbour_peers_macaddr
							 , wds_src_mac,
							 QDF_MAC_ADDR_SIZE)) {
						ret = dp_peer_add_ast
							(soc,
							 ta_peer,
							 wds_src_mac,
							 CDP_TXRX_AST_TYPE_WDS,
							 flags);
						QDF_TRACE
							(QDF_MODULE_ID_DP,
							 QDF_TRACE_LEVEL_INFO,
							 "sa valid and nac roamed to wds");
						break;
					}
				}
				qdf_spin_unlock_bh(&pdev->neighbour_peer_mutex);
			}
			return;
		}
	}

	if ((ast->type == CDP_TXRX_AST_TYPE_WDS_HM) ||
	    (ast->type == CDP_TXRX_AST_TYPE_WDS_HM_SEC))
		return;

	/*
	 * Ensure we are updating the right AST entry by
	 * validating ast_idx.
	 * There is a possibility we might arrive here without
	 * AST MAP event , so this check is mandatory
	 */
	if (ast->is_mapped && (ast->ast_idx == sa_idx))
		ast->is_active = TRUE;

	if (sa_sw_peer_id != ta_peer->peer_ids[0]) {
		sa_peer = ast->peer;

		if ((ast->type != CDP_TXRX_AST_TYPE_STATIC) &&
		    (ast->type != CDP_TXRX_AST_TYPE_SELF) &&
		    (ast->type != CDP_TXRX_AST_TYPE_STA_BSS)) {
			if (ast->pdev_id != ta_peer->vdev->pdev->pdev_id) {
				/* This case is when a STA roams from one
				 * repeater to another repeater, but these
				 * repeaters are connected to root AP on
				 * different radios.
				 * Ex: rptr1 connected to ROOT AP over 5G
				 * and rptr2 connected to ROOT AP over 2G
				 * radio
				 */
				qdf_spin_lock_bh(&soc->ast_lock);
				dp_peer_del_ast(soc, ast);
				qdf_spin_unlock_bh(&soc->ast_lock);
			} else {
				/* this case is when a STA roams from one
				 * reapter to another repeater, but inside
				 * same radio.
				 */
				qdf_spin_lock_bh(&soc->ast_lock);
				dp_peer_update_ast(soc, ta_peer, ast, flags);
				qdf_spin_unlock_bh(&soc->ast_lock);
				return;
			}
		}
		/*
		 * Do not kickout STA if it belongs to a different radio.
		 * For DBDC repeater, it is possible to arrive here
		 * for multicast loopback frames originated from connected
		 * clients and looped back (intrabss) by Root AP
		 */
		if (ast->pdev_id != ta_peer->vdev->pdev->pdev_id)
			return;

		/*
		 * Kickout, when direct associated peer(SA) roams
		 * to another AP and reachable via TA peer
		 */
		if ((sa_peer->vdev->opmode == wlan_op_mode_ap) &&
		    !sa_peer->delete_in_progress) {
			sa_peer->delete_in_progress = true;
			if (soc->cdp_soc.ol_ops->peer_sta_kickout) {
				soc->cdp_soc.ol_ops->peer_sta_kickout(
						sa_peer->vdev->pdev->ctrl_pdev,
						wds_src_mac);
			}
		}
	}
}

/**
 * dp_rx_wds_srcport_learn() - Add or update the STA PEER which
 *				is behind the WDS repeater.
 *
 * @soc: core txrx main context
 * @rx_tlv_hdr: base address of RX TLV header
 * @ta_peer: WDS repeater peer
 * @nbuf: rx pkt
 *
 * Return: void:
 */
static inline void
dp_rx_wds_srcport_learn(struct dp_soc *soc,
			uint8_t *rx_tlv_hdr,
			struct dp_peer *ta_peer,
			qdf_nbuf_t nbuf)
{
	uint16_t sa_sw_peer_id = hal_rx_msdu_end_sa_sw_peer_id_get(rx_tlv_hdr);
	uint8_t sa_is_valid = hal_rx_msdu_end_sa_is_valid_get(rx_tlv_hdr);
	uint8_t wds_src_mac[IEEE80211_ADDR_LEN];
	uint16_t sa_idx;
	uint8_t is_chfrag_start = 0;
	uint8_t is_ad4_valid = 0;

	if (qdf_unlikely(!ta_peer))
		return;

	is_chfrag_start = qdf_nbuf_is_rx_chfrag_start(nbuf);
	if (is_chfrag_start)
		is_ad4_valid = hal_rx_get_mpdu_mac_ad4_valid(rx_tlv_hdr);

	memcpy(wds_src_mac, (qdf_nbuf_data(nbuf) + IEEE80211_ADDR_LEN),
	       IEEE80211_ADDR_LEN);

	/*
	 * Get the AST entry from HW SA index and mark it as active
	 */
	sa_idx = hal_rx_msdu_end_sa_idx_get(rx_tlv_hdr);

	dp_rx_wds_add_or_update_ast(soc, ta_peer, wds_src_mac, is_ad4_valid,
				    sa_is_valid, is_chfrag_start,
				    sa_idx, sa_sw_peer_id);
}

/*
 * dp_rx_ast_set_active() - set the active flag of the astentry
 *				    corresponding to a hw index.
 * @soc: core txrx main context
 * @sa_idx: hw idx
 * @is_active: active flag
 *
 */
static inline QDF_STATUS dp_rx_ast_set_active(struct dp_soc *soc,
					      uint16_t sa_idx, bool is_active)
{
	struct dp_ast_entry *ast;

	qdf_spin_lock_bh(&soc->ast_lock);
	ast = soc->ast_table[sa_idx];

	/*
	 * Ensure we are updating the right AST entry by
	 * validating ast_idx.
	 * There is a possibility we might arrive here without
	 * AST MAP event , so this check is mandatory
	 */
	if (ast && ast->is_mapped && (ast->ast_idx == sa_idx)) {
		ast->is_active = is_active;
		qdf_spin_unlock_bh(&soc->ast_lock);
		return QDF_STATUS_SUCCESS;
	}

	qdf_spin_unlock_bh(&soc->ast_lock);
	return QDF_STATUS_E_FAILURE;
}
#endif /* DP_TXRX_WDS*/
