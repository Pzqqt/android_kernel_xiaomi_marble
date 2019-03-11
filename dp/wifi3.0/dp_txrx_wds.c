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
#include "dp_peer.h"
#include "hal_rx.h"
#include "hal_api.h"
#include "qdf_nbuf.h"
#include "dp_types.h"
#include "dp_internal.h"
#include "dp_txrx_wds.h"

/* Generic AST entry aging timer value */
#define DP_AST_AGING_TIMER_DEFAULT_MS	1000

static void dp_ast_aging_timer_fn(void *soc_hdl)
{
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;
	struct dp_pdev *pdev;
	struct dp_vdev *vdev;
	struct dp_peer *peer;
	struct dp_ast_entry *ase, *temp_ase;
	int i;
	bool check_wds_ase = false;

	if (soc->wds_ast_aging_timer_cnt++ >= DP_WDS_AST_AGING_TIMER_CNT) {
		soc->wds_ast_aging_timer_cnt = 0;
		check_wds_ase = true;
	}

	 /* Peer list access lock */
	qdf_spin_lock_bh(&soc->peer_ref_mutex);

	/* AST list access lock */
	qdf_spin_lock_bh(&soc->ast_lock);

	for (i = 0; i < MAX_PDEV_CNT && soc->pdev_list[i]; i++) {
		pdev = soc->pdev_list[i];
		qdf_spin_lock_bh(&pdev->vdev_list_lock);
		DP_PDEV_ITERATE_VDEV_LIST(pdev, vdev) {
			DP_VDEV_ITERATE_PEER_LIST(vdev, peer) {
				DP_PEER_ITERATE_ASE_LIST(peer, ase, temp_ase) {
					/*
					 * Do not expire static ast entries
					 * and HM WDS entries
					 */
					if (ase->type !=
					    CDP_TXRX_AST_TYPE_WDS &&
					    ase->type !=
					    CDP_TXRX_AST_TYPE_MEC &&
					    ase->type !=
					    CDP_TXRX_AST_TYPE_DA)
						continue;

					/* Expire MEC entry every n sec.
					 * This needs to be expired in
					 * case if STA backbone is made as
					 * AP backbone, In this case it needs
					 * to be re-added as a WDS entry.
					 */
					if (ase->is_active && ase->type ==
					    CDP_TXRX_AST_TYPE_MEC) {
						ase->is_active = FALSE;
						continue;
					} else if (ase->is_active &&
						   check_wds_ase) {
						ase->is_active = FALSE;
						continue;
					}

					if (ase->type ==
					    CDP_TXRX_AST_TYPE_MEC) {
						DP_STATS_INC(soc,
							     ast.aged_out, 1);
						dp_peer_del_ast(soc, ase);
					} else if (check_wds_ase) {
						DP_STATS_INC(soc,
							     ast.aged_out, 1);
						dp_peer_del_ast(soc, ase);
					}
				}
			}
		}
		qdf_spin_unlock_bh(&pdev->vdev_list_lock);
	}

	qdf_spin_unlock_bh(&soc->ast_lock);
	qdf_spin_unlock_bh(&soc->peer_ref_mutex);

	if (qdf_atomic_read(&soc->cmn_init_done))
		qdf_timer_mod(&soc->ast_aging_timer,
			      DP_AST_AGING_TIMER_DEFAULT_MS);
}

/*
 * dp_soc_wds_attach() - Setup WDS timer and AST table
 * @soc:		Datapath SOC handle
 *
 * Return: None
 */
void dp_soc_wds_attach(struct dp_soc *soc)
{
	soc->wds_ast_aging_timer_cnt = 0;
	qdf_timer_init(soc->osdev, &soc->ast_aging_timer,
		       dp_ast_aging_timer_fn, (void *)soc,
		       QDF_TIMER_TYPE_WAKE_APPS);

	qdf_timer_mod(&soc->ast_aging_timer, DP_AST_AGING_TIMER_DEFAULT_MS);
}

/*
 * dp_soc_wds_detach() - Detach WDS data structures and timers
 * @txrx_soc: DP SOC handle
 *
 * Return: None
 */
void dp_soc_wds_detach(struct dp_soc *soc)
{
	qdf_timer_stop(&soc->ast_aging_timer);
	qdf_timer_free(&soc->ast_aging_timer);
}

/**
 * dp_rx_da_learn() - Add AST entry based on DA lookup
 *			This is a WAR for HK 1.0 and will
 *			be removed in HK 2.0
 *
 * @soc: core txrx main context
 * @rx_tlv_hdr	: start address of rx tlvs
 * @ta_peer	: Transmitter peer entry
 * @nbuf	: nbuf to retrieve destination mac for which AST will be added
 *
 */
void
dp_rx_da_learn(struct dp_soc *soc,
	       uint8_t *rx_tlv_hdr,
	       struct dp_peer *ta_peer,
	       qdf_nbuf_t nbuf)
{
	/* For HKv2 DA port learing is not needed */
	if (qdf_likely(soc->ast_override_support))
		return;

	if (qdf_unlikely(!ta_peer))
		return;

	if (qdf_unlikely(ta_peer->vdev->opmode != wlan_op_mode_ap))
		return;

	if (!soc->da_war_enabled)
		return;

	if (qdf_unlikely(!qdf_nbuf_is_da_valid(nbuf) &&
			 !qdf_nbuf_is_da_mcbc(nbuf))) {
		dp_peer_add_ast(soc,
				ta_peer,
				qdf_nbuf_data(nbuf),
				CDP_TXRX_AST_TYPE_DA,
				IEEE80211_NODE_F_WDS_HM);
	}
}

/**
 * dp_tx_mec_handler() - Tx  MEC Notify Handler
 * @vdev: pointer to dp dev handler
 * @status : Tx completion status from HTT descriptor
 *
 * Handles MEC notify event sent from fw to Host
 *
 * Return: none
 */
void dp_tx_mec_handler(struct dp_vdev *vdev, uint8_t *status)
{
	struct dp_soc *soc;
	uint32_t flags = IEEE80211_NODE_F_WDS_HM;
	struct dp_peer *peer;
	uint8_t mac_addr[QDF_MAC_ADDR_SIZE], i;

	if (!vdev->mec_enabled)
		return;

	/* MEC required only in STA mode */
	if (vdev->opmode != wlan_op_mode_sta)
		return;

	soc = vdev->pdev->soc;
	qdf_spin_lock_bh(&soc->peer_ref_mutex);
	peer = TAILQ_FIRST(&vdev->peer_list);
	qdf_spin_unlock_bh(&soc->peer_ref_mutex);

	if (!peer) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  FL("peer is NULL"));
		return;
	}

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
		  "%s Tx MEC Handler",
		  __func__);

	for (i = 0; i < QDF_MAC_ADDR_SIZE; i++)
		mac_addr[(QDF_MAC_ADDR_SIZE - 1) - i] =
					status[(QDF_MAC_ADDR_SIZE - 2) + i];

	if (qdf_mem_cmp(mac_addr, vdev->mac_addr.raw, QDF_MAC_ADDR_SIZE))
		dp_peer_add_ast(soc,
				peer,
				mac_addr,
				CDP_TXRX_AST_TYPE_MEC,
				flags);
}

/**
 * dp_txrx_set_wds_rx_policy() - API to store datapath
 *                            config parameters
 * @vdev_handle - datapath vdev handle
 * @cfg: ini parameter handle
 *
 * Return: status
 */
#ifdef WDS_VENDOR_EXTENSION
void
dp_txrx_set_wds_rx_policy(struct cdp_vdev *vdev_handle,	u_int32_t val)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;
	struct dp_peer *peer;

	if (vdev->opmode == wlan_op_mode_ap) {
		/* for ap, set it on bss_peer */
		TAILQ_FOREACH(peer, &vdev->peer_list, peer_list_elem) {
			if (peer->bss_peer) {
				peer->wds_ecm.wds_rx_filter = 1;
				peer->wds_ecm.wds_rx_ucast_4addr =
					(val & WDS_POLICY_RX_UCAST_4ADDR) ?
					1 : 0;
				peer->wds_ecm.wds_rx_mcast_4addr =
					(val & WDS_POLICY_RX_MCAST_4ADDR) ?
					1 : 0;
				break;
			}
		}
	} else if (vdev->opmode == wlan_op_mode_sta) {
		peer = TAILQ_FIRST(&vdev->peer_list);
		peer->wds_ecm.wds_rx_filter = 1;
		peer->wds_ecm.wds_rx_ucast_4addr =
			(val & WDS_POLICY_RX_UCAST_4ADDR) ? 1 : 0;
		peer->wds_ecm.wds_rx_mcast_4addr =
			(val & WDS_POLICY_RX_MCAST_4ADDR) ? 1 : 0;
	}
}

/**
 * dp_txrx_peer_wds_tx_policy_update() - API to set tx wds policy
 *
 * @peer_handle - datapath peer handle
 * @wds_tx_ucast: policy for unicast transmission
 * @wds_tx_mcast: policy for multicast transmission
 *
 * Return: void
 */
void
dp_txrx_peer_wds_tx_policy_update(struct cdp_peer *peer_handle,
				  int wds_tx_ucast, int wds_tx_mcast)
{
	struct dp_peer *peer = (struct dp_peer *)peer_handle;

	if (wds_tx_ucast || wds_tx_mcast) {
		peer->wds_enabled = 1;
		peer->wds_ecm.wds_tx_ucast_4addr = wds_tx_ucast;
		peer->wds_ecm.wds_tx_mcast_4addr = wds_tx_mcast;
	} else {
		peer->wds_enabled = 0;
		peer->wds_ecm.wds_tx_ucast_4addr = 0;
		peer->wds_ecm.wds_tx_mcast_4addr = 0;
	}

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
		  "Policy Update set to :\n");
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
		  "peer->wds_enabled %d\n", peer->wds_enabled);
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
		  "peer->wds_ecm.wds_tx_ucast_4addr %d\n",
		  peer->wds_ecm.wds_tx_ucast_4addr);
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
		  "peer->wds_ecm.wds_tx_mcast_4addr %d\n",
		  peer->wds_ecm.wds_tx_mcast_4addr);
}

int dp_wds_rx_policy_check(uint8_t *rx_tlv_hdr,
			   struct dp_vdev *vdev,
			   struct dp_peer *peer)
{
	struct dp_peer *bss_peer;
	int fr_ds, to_ds, rx_3addr, rx_4addr;
	int rx_policy_ucast, rx_policy_mcast;
	int rx_mcast = hal_rx_msdu_end_da_is_mcbc_get(rx_tlv_hdr);

	if (vdev->opmode == wlan_op_mode_ap) {
		TAILQ_FOREACH(bss_peer, &vdev->peer_list, peer_list_elem) {
			if (bss_peer->bss_peer) {
				/* if wds policy check is not enabled on this vdev, accept all frames */
				if (!bss_peer->wds_ecm.wds_rx_filter) {
					return 1;
				}
				break;
			}
		}
		rx_policy_ucast = bss_peer->wds_ecm.wds_rx_ucast_4addr;
		rx_policy_mcast = bss_peer->wds_ecm.wds_rx_mcast_4addr;
	} else {             /* sta mode */
		if (!peer->wds_ecm.wds_rx_filter) {
			return 1;
		}
		rx_policy_ucast = peer->wds_ecm.wds_rx_ucast_4addr;
		rx_policy_mcast = peer->wds_ecm.wds_rx_mcast_4addr;
	}

	/* ------------------------------------------------
	 *                       self
	 * peer-             rx  rx-
	 * wds  ucast mcast dir policy accept note
	 * ------------------------------------------------
	 * 1     1     0     11  x1     1      AP configured to accept ds-to-ds Rx ucast from wds peers, constraint met; so, accept
	 * 1     1     0     01  x1     0      AP configured to accept ds-to-ds Rx ucast from wds peers, constraint not met; so, drop
	 * 1     1     0     10  x1     0      AP configured to accept ds-to-ds Rx ucast from wds peers, constraint not met; so, drop
	 * 1     1     0     00  x1     0      bad frame, won't see it
	 * 1     0     1     11  1x     1      AP configured to accept ds-to-ds Rx mcast from wds peers, constraint met; so, accept
	 * 1     0     1     01  1x     0      AP configured to accept ds-to-ds Rx mcast from wds peers, constraint not met; so, drop
	 * 1     0     1     10  1x     0      AP configured to accept ds-to-ds Rx mcast from wds peers, constraint not met; so, drop
	 * 1     0     1     00  1x     0      bad frame, won't see it
	 * 1     1     0     11  x0     0      AP configured to accept from-ds Rx ucast from wds peers, constraint not met; so, drop
	 * 1     1     0     01  x0     0      AP configured to accept from-ds Rx ucast from wds peers, constraint not met; so, drop
	 * 1     1     0     10  x0     1      AP configured to accept from-ds Rx ucast from wds peers, constraint met; so, accept
	 * 1     1     0     00  x0     0      bad frame, won't see it
	 * 1     0     1     11  0x     0      AP configured to accept from-ds Rx mcast from wds peers, constraint not met; so, drop
	 * 1     0     1     01  0x     0      AP configured to accept from-ds Rx mcast from wds peers, constraint not met; so, drop
	 * 1     0     1     10  0x     1      AP configured to accept from-ds Rx mcast from wds peers, constraint met; so, accept
	 * 1     0     1     00  0x     0      bad frame, won't see it
	 *
	 * 0     x     x     11  xx     0      we only accept td-ds Rx frames from non-wds peers in mode.
	 * 0     x     x     01  xx     1
	 * 0     x     x     10  xx     0
	 * 0     x     x     00  xx     0      bad frame, won't see it
	 * ------------------------------------------------
	 */

	fr_ds = hal_rx_mpdu_get_fr_ds(rx_tlv_hdr);
	to_ds = hal_rx_mpdu_get_to_ds(rx_tlv_hdr);
	rx_3addr = fr_ds ^ to_ds;
	rx_4addr = fr_ds & to_ds;

	if (vdev->opmode == wlan_op_mode_ap) {
		if ((!peer->wds_enabled && rx_3addr && to_ds) ||
				(peer->wds_enabled && !rx_mcast && (rx_4addr == rx_policy_ucast)) ||
				(peer->wds_enabled && rx_mcast && (rx_4addr == rx_policy_mcast))) {
			return 1;
		}
	} else {           /* sta mode */
		if ((!rx_mcast && (rx_4addr == rx_policy_ucast)) ||
				(rx_mcast && (rx_4addr == rx_policy_mcast))) {
			return 1;
		}
	}
	return 0;
}
#endif
