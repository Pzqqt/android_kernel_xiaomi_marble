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
#include "../../../cmn_dev/fw_hdr/fw/htt.h"
#include "dp_peer.h"
#include "hal_rx.h"
#include "hal_api.h"
#include "qdf_nbuf.h"
#include "dp_types.h"
#include "dp_internal.h"
#include "dp_tx.h"
#include "enet.h"
#include "dp_txrx_wds.h"

/* Generic AST entry aging timer value */
#define DP_AST_AGING_TIMER_DEFAULT_MS	5000
#define DP_VLAN_UNTAGGED 0
#define DP_VLAN_TAGGED_MULTICAST 1
#define DP_VLAN_TAGGED_UNICAST 2
#define DP_MAX_VLAN_IDS 4096
#define DP_INVALID_AST_IDX 0xffff
#define DP_INVALID_FLOW_PRIORITY 0xff
#define DP_PEER_AST0_FLOW_MASK 0x4
#define DP_PEER_AST1_FLOW_MASK 0x8
#define DP_PEER_AST2_FLOW_MASK 0x1
#define DP_PEER_AST3_FLOW_MASK 0x2
#define DP_MAX_AST_INDEX_PER_PEER 4

static void
dp_peer_age_ast_entries(struct dp_soc *soc, struct dp_peer *peer, void *arg)
{
	struct dp_ast_entry *ase, *temp_ase;
	bool check_wds_ase = *(bool *)arg;

	DP_PEER_ITERATE_ASE_LIST(peer, ase, temp_ase) {
		/*
		 * Do not expire static ast entries and HM WDS entries
		 */
		if (ase->type != CDP_TXRX_AST_TYPE_WDS &&
		    ase->type != CDP_TXRX_AST_TYPE_MEC &&
		    ase->type != CDP_TXRX_AST_TYPE_DA)
			continue;

		/* Expire MEC entry every n sec. This needs to be expired in
		 * case if STA backbone is made as AP backbone, In this case
		 * it needs to be re-added as a WDS entry.
		 */
		if (ase->is_active && ase->type ==  CDP_TXRX_AST_TYPE_MEC) {
			ase->is_active = FALSE;
			continue;
		} else if (ase->is_active &&  check_wds_ase) {
			ase->is_active = FALSE;
			continue;
		}

		if (ase->type == CDP_TXRX_AST_TYPE_MEC) {
			DP_STATS_INC(soc, ast.aged_out, 1);
			dp_peer_del_ast(soc, ase);
		} else if (check_wds_ase) {
			DP_STATS_INC(soc, ast.aged_out, 1);
			dp_peer_del_ast(soc, ase);
		}
	}
}

static void dp_ast_aging_timer_fn(void *soc_hdl)
{
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;
	bool check_wds_ase = false;

	if (soc->wds_ast_aging_timer_cnt++ >= DP_WDS_AST_AGING_TIMER_CNT) {
		soc->wds_ast_aging_timer_cnt = 0;
		check_wds_ase = true;
	}

	/* AST list access lock */
	qdf_spin_lock_bh(&soc->ast_lock);

	dp_soc_iterate_peer(soc, dp_peer_age_ast_entries, (void *)&check_wds_ase,
			    DP_MOD_ID_AST);
	qdf_spin_unlock_bh(&soc->ast_lock);

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
	peer = dp_vdev_bss_peer_ref_n_get(soc, vdev,
					  DP_MOD_ID_AST);

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
	dp_peer_unref_delete(peer, DP_MOD_ID_AST);
}

#ifndef QCA_HOST_MODE_WIFI_DISABLED

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
 * dp_txrx_set_wds_rx_policy() - API to store datapath
 *                            config parameters
 * @soc - datapath soc handle
 * @vdev_id - id of datapath vdev handle
 * @cfg: ini parameter handle
 *
 * Return: status
 */
#ifdef WDS_VENDOR_EXTENSION
QDF_STATUS
dp_txrx_set_wds_rx_policy(struct cdp_soc_t *soc_hdl, uint8_t vdev_id,
			  u_int32_t val)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_peer *peer;
	struct dp_vdev *vdev = dp_vdev_get_ref_by_id(soc, vdev_id,
						     DP_MOD_ID_MISC);
	if (!vdev) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  FL("vdev is NULL for vdev_id %d"), vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	peer = dp_vdev_bss_peer_ref_n_get(vdev, DP_MOD_ID_AST);

	if (peer) {
		peer->wds_ecm.wds_rx_filter = 1;
		peer->wds_ecm.wds_rx_ucast_4addr =
			(val & WDS_POLICY_RX_UCAST_4ADDR) ? 1 : 0;
		peer->wds_ecm.wds_rx_mcast_4addr =
			(val & WDS_POLICY_RX_MCAST_4ADDR) ? 1 : 0;
		dp_peer_unref_delete(peer, DP_MOD_ID_AST);
	}

	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_MISC);
	return QDF_STATUS_SUCCESS;
}

/**
 * dp_txrx_peer_wds_tx_policy_update() - API to set tx wds policy
 *
 * @cdp_soc: DP soc handle
 * @vdev_id: id of vdev handle
 * @peer_mac: peer mac address
 * @wds_tx_ucast: policy for unicast transmission
 * @wds_tx_mcast: policy for multicast transmission
 *
 * Return: void
 */
QDF_STATUS
dp_txrx_peer_wds_tx_policy_update(struct cdp_soc_t *soc,  uint8_t vdev_id,
				  uint8_t *peer_mac, int wds_tx_ucast,
				  int wds_tx_mcast)
{
	struct dp_peer *peer = dp_peer_find_hash_find((struct dp_soc *)soc,
						       peer_mac, 0,
						       vdev_id,
						       DP_MOD_ID_AST);
	if (!peer) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  FL("peer is NULL for mac %pM vdev_id %d"),
			  peer_mac, vdev_id);
		return QDF_STATUS_E_INVAL;
	}

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

	dp_peer_unref_delete(peer, DP_MOD_ID_AST);
	return QDF_STATUS_SUCCESS;
}

int dp_wds_rx_policy_check(uint8_t *rx_tlv_hdr,
			   struct dp_vdev *vdev,
			   struct dp_peer *peer)
{
	struct dp_peer *bss_peer;
	int fr_ds, to_ds, rx_3addr, rx_4addr;
	int rx_policy_ucast, rx_policy_mcast;
	hal_soc_handle_t hal_soc = vdev->pdev->soc->hal_soc;
	int rx_mcast = hal_rx_msdu_end_da_is_mcbc_get(hal_soc, rx_tlv_hdr);

	if (vdev->opmode == wlan_op_mode_ap) {
		bss_peer = dp_vdev_bss_peer_ref_n_get(vdev, DP_MOD_ID_AST);
		/* if wds policy check is not enabled on this vdev, accept all frames */
		if (bss_peer && !bss_peer->wds_ecm.wds_rx_filter) {
			dp_peer_unref_delete(bss_peer, DP_MOD_ID_AST);
			return 1;
		}
		rx_policy_ucast = bss_peer->wds_ecm.wds_rx_ucast_4addr;
		rx_policy_mcast = bss_peer->wds_ecm.wds_rx_mcast_4addr;
		dp_peer_unref_delete(bss_peer, DP_MOD_ID_AST);
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

	fr_ds = hal_rx_mpdu_get_fr_ds(hal_soc, rx_tlv_hdr);
	to_ds = hal_rx_mpdu_get_to_ds(hal_soc, rx_tlv_hdr);
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

/**
 * dp_tx_add_groupkey_metadata - Add group key in metadata
 * @vdev: DP vdev handle
 * @msdu_info: MSDU info to be setup in MSDU descriptor
 * @group_key: Group key index programmed in metadata
 *
 * Return: void
 */
#ifdef QCA_MULTIPASS_SUPPORT
static
void dp_tx_add_groupkey_metadata(struct dp_vdev *vdev,
		struct dp_tx_msdu_info_s *msdu_info, uint16_t group_key)
{
	struct htt_tx_msdu_desc_ext2_t *meta_data =
		(struct htt_tx_msdu_desc_ext2_t *)&msdu_info->meta_data[0];

	qdf_mem_zero(meta_data, sizeof(struct htt_tx_msdu_desc_ext2_t));

	/*
	 * When attempting to send a multicast packet with multi-passphrase,
	 * host shall add HTT EXT meta data "struct htt_tx_msdu_desc_ext2_t"
	 * ref htt.h indicating the group_id field in "key_flags" also having
	 * "valid_key_flags" as 1. Assign “key_flags = group_key_ix”.
	 */
	HTT_TX_MSDU_EXT2_DESC_FLAG_VALID_KEY_FLAGS_SET(msdu_info->meta_data[0], 1);
	HTT_TX_MSDU_EXT2_DESC_KEY_FLAGS_SET(msdu_info->meta_data[2], group_key);
}

/**
 * dp_tx_remove_vlan_tag - Remove 4 bytes of vlan tag
 * @vdev: DP vdev handle
 * @tx_desc: Tx Descriptor Handle
 *
 * Return: void
 */
static
void dp_tx_remove_vlan_tag(struct dp_vdev *vdev, qdf_nbuf_t nbuf)
{
	struct vlan_ethhdr veth_hdr;
	struct vlan_ethhdr *veh = (struct vlan_ethhdr *)nbuf->data;

	/*
	 * Extract VLAN header of 4 bytes:
	 * Frame Format : {dst_addr[6], src_addr[6], 802.1Q header[4], EtherType[2], Payload}
	 * Before Removal : xx xx xx xx xx xx xx xx xx xx xx xx 81 00 00 02 08 00 45 00 00...
	 * After Removal  : xx xx xx xx xx xx xx xx xx xx xx xx 08 00 45 00 00...
	 */
	qdf_mem_copy(&veth_hdr, veh, sizeof(veth_hdr));
	qdf_nbuf_pull_head(nbuf, ETHERTYPE_VLAN_LEN);
	veh = (struct vlan_ethhdr *)nbuf->data;
	qdf_mem_copy(veh, &veth_hdr, 2 * QDF_MAC_ADDR_SIZE);
	return;
}

/**
 * dp_tx_need_multipass_process - If frame needs multipass phrase processing
 * @vdev: DP vdev handle
 * @tx_desc: Tx Descriptor Handle
 * @vlan_id: vlan id of frame
 *
 * Return: whether peer is special or classic
 */
static
uint8_t dp_tx_need_multipass_process(struct dp_soc *soc, struct dp_vdev *vdev,
			   qdf_nbuf_t buf, uint16_t *vlan_id)
{
	struct dp_peer *peer = NULL;
	qdf_ether_header_t *eh = (qdf_ether_header_t *)qdf_nbuf_data(buf);
	struct vlan_ethhdr *veh = NULL;
	bool not_vlan = ((vdev->tx_encap_type == htt_cmn_pkt_type_raw) ||
			(htons(eh->ether_type) != ETH_P_8021Q));

	if (qdf_unlikely(not_vlan))
		return DP_VLAN_UNTAGGED;

	veh = (struct vlan_ethhdr *)eh;
	*vlan_id = (ntohs(veh->h_vlan_TCI) & VLAN_VID_MASK);

	if (qdf_unlikely(DP_FRAME_IS_MULTICAST((eh)->ether_dhost))) {
		qdf_spin_lock_bh(&vdev->mpass_peer_mutex);
		TAILQ_FOREACH(peer, &vdev->mpass_peer_list,
			      mpass_peer_list_elem) {
			if (*vlan_id == peer->vlan_id) {
				qdf_spin_unlock_bh(&vdev->mpass_peer_mutex);
				return DP_VLAN_TAGGED_MULTICAST;
			}
		}
		qdf_spin_unlock_bh(&vdev->mpass_peer_mutex);
		return DP_VLAN_UNTAGGED;
	}

	peer = dp_peer_find_hash_find(soc, eh->ether_dhost, 0, DP_VDEV_ALL,
				      DP_MOD_ID_TX_MULTIPASS);

	if (qdf_unlikely(peer == NULL))
		return DP_VLAN_UNTAGGED;

	/*
	 * Do not drop the frame when vlan_id doesn't match.
	 * Send the frame as it is.
	 */
	if (*vlan_id == peer->vlan_id) {
		dp_peer_unref_delete(peer, DP_MOD_ID_TX_MULTIPASS);
		return DP_VLAN_TAGGED_UNICAST;
	}

	dp_peer_unref_delete(peer, DP_MOD_ID_TX_MULTIPASS);
	return DP_VLAN_UNTAGGED;
}

/**
 * dp_tx_multipass_process - Process vlan frames in tx path
 * @soc: dp soc handle
 * @vdev: DP vdev handle
 * @nbuf: skb
 * @msdu_info: msdu descriptor
 *
 * Return: status whether frame needs to be dropped or transmitted
 */
bool dp_tx_multipass_process(struct dp_soc *soc, struct dp_vdev *vdev,
			     qdf_nbuf_t nbuf,
			     struct dp_tx_msdu_info_s *msdu_info)
{
	uint16_t vlan_id = 0;
	uint16_t group_key = 0;
	uint8_t is_spcl_peer = DP_VLAN_UNTAGGED;
	qdf_nbuf_t nbuf_copy = NULL;

	if (HTT_TX_MSDU_EXT2_DESC_FLAG_VALID_KEY_FLAGS_GET(msdu_info->meta_data[0])) {
		return true;
	}

	is_spcl_peer = dp_tx_need_multipass_process(soc, vdev, nbuf, &vlan_id);

	if ((is_spcl_peer != DP_VLAN_TAGGED_MULTICAST) &&
	    (is_spcl_peer != DP_VLAN_TAGGED_UNICAST))
		return true;

	if (is_spcl_peer == DP_VLAN_TAGGED_UNICAST) {
		dp_tx_remove_vlan_tag(vdev, nbuf);
		return true;
	}

	/* AP can have classic clients, special clients &
	 * classic repeaters.
	 * 1. Classic clients & special client:
	 *	Remove vlan header, find corresponding group key
	 *	index, fill in metaheader and enqueue multicast
	 *	frame to TCL.
	 * 2. Classic repeater:
	 *	Pass through to classic repeater with vlan tag
	 *	intact without any group key index. Hardware
	 *	will know which key to use to send frame to
	 *	repeater.
	 */
	nbuf_copy = qdf_nbuf_copy(nbuf);

	/*
	 * Send multicast frame to special peers even
	 * if pass through to classic repeater fails.
	 */
	if (nbuf_copy) {
		struct dp_tx_msdu_info_s msdu_info_copy;
		qdf_mem_zero(&msdu_info_copy, sizeof(msdu_info_copy));
		msdu_info_copy.tid = HTT_TX_EXT_TID_INVALID;
		HTT_TX_MSDU_EXT2_DESC_FLAG_VALID_KEY_FLAGS_SET(msdu_info_copy.meta_data[0], 1);
		nbuf_copy = dp_tx_send_msdu_single(vdev, nbuf_copy, &msdu_info_copy, HTT_INVALID_PEER, NULL);
		if (nbuf_copy) {
			qdf_nbuf_free(nbuf_copy);
			qdf_err("nbuf_copy send failed");
		}
	}

	group_key = vdev->iv_vlan_map[vlan_id];

	/*
	 * If group key is not installed, drop the frame.
	 */
	if (!group_key)
		return false;

	dp_tx_remove_vlan_tag(vdev, nbuf);
	dp_tx_add_groupkey_metadata(vdev, msdu_info, group_key);
	msdu_info->exception_fw = 1;
	return true;
}

/**
 * dp_rx_multipass_process - insert vlan tag on frames for traffic separation
 * @vdev: DP vdev handle
 * @nbuf: skb
 * @tid: traffic priority
 *
 * Return: bool: true in case of success else false
 * Success is considered if:
 *  i. If frame has vlan header
 *  ii. If the frame comes from different peer and dont need multipass processing
 * Failure is considered if:
 *  i. Frame comes from multipass peer but doesn't contain vlan header.
 *  In failure case, drop such frames.
 */
bool dp_rx_multipass_process(struct dp_peer *peer, qdf_nbuf_t nbuf, uint8_t tid)
{
	struct vlan_ethhdr *vethhdrp;

	if (qdf_unlikely(!peer->vlan_id))
	       return true;

	vethhdrp = (struct vlan_ethhdr *)qdf_nbuf_data(nbuf);
	/*
	 * h_vlan_proto & h_vlan_TCI should be 0x8100 & zero respectively
	 * as it is expected to be padded by 0
	 * return false if frame doesn't have above tag so that caller will
	 * drop the frame.
	 */
	if (qdf_unlikely(vethhdrp->h_vlan_proto != htons(QDF_ETH_TYPE_8021Q)) ||
	    qdf_unlikely(vethhdrp->h_vlan_TCI != 0))
		return false;

	vethhdrp->h_vlan_TCI = htons(((tid & 0x7) << VLAN_PRIO_SHIFT) |
		(peer->vlan_id & VLAN_VID_MASK));
	return true;
}

#endif /* QCA_MULTIPASS_SUPPORT */

#endif /* QCA_HOST_MODE_WIFI_DISABLED */

#ifdef QCA_MULTIPASS_SUPPORT

/**
 * dp_peer_multipass_list_remove: remove peer from list
 * @peer: pointer to peer
 *
 * return: void
 */
void dp_peer_multipass_list_remove(struct dp_peer *peer)
{
	struct dp_vdev *vdev = peer->vdev;
	struct dp_peer *tpeer = NULL;
	bool found = 0;

	qdf_spin_lock_bh(&vdev->mpass_peer_mutex);
	TAILQ_FOREACH(tpeer, &vdev->mpass_peer_list, mpass_peer_list_elem) {
		if (tpeer == peer) {
			found = 1;
			TAILQ_REMOVE(&vdev->mpass_peer_list, peer, mpass_peer_list_elem);
			break;
		}
	}

	qdf_spin_unlock_bh(&vdev->mpass_peer_mutex);

	if (found)
		dp_peer_unref_delete(peer, DP_MOD_ID_TX_MULTIPASS);
}

/**
 * dp_peer_multipass_list_add: add to new multipass list
 * @dp_soc: soc handle
 * @peer_mac: mac address
 * @vdev_id: vdev id for peer
 * @vlan_id: vlan_id
 *
 * return: void
 */
static void dp_peer_multipass_list_add(struct dp_soc *soc, uint8_t *peer_mac,
				       uint8_t vdev_id, uint16_t vlan_id)
{
	struct dp_peer *peer =
			dp_peer_find_hash_find(soc, peer_mac, 0, vdev_id,
					       DP_MOD_ID_TX_MULTIPASS);

	if (qdf_unlikely(!peer)) {
		qdf_err("NULL peer");
		return;
	}

	/* If peer already exists in vdev multipass list, do not add it.
	 * This may happen if key install comes twice or re-key
	 * happens for a peer.
	 */
	if (peer->vlan_id) {
		dp_debug("peer already added to vdev multipass list"
			 "MAC: "QDF_MAC_ADDR_FMT" vlan: %d ",
			 QDF_MAC_ADDR_REF(peer->mac_addr.raw), peer->vlan_id);
		dp_peer_unref_delete(peer, DP_MOD_ID_TX_MULTIPASS);
		return;
	}

	/*
	 * Ref_cnt is incremented inside dp_peer_find_hash_find().
	 * Decrement it when element is deleted from the list.
	 */
	peer->vlan_id = vlan_id;
	qdf_spin_lock_bh(&peer->vdev->mpass_peer_mutex);
	TAILQ_INSERT_HEAD(&peer->vdev->mpass_peer_list, peer,
			  mpass_peer_list_elem);
	qdf_spin_unlock_bh(&peer->vdev->mpass_peer_mutex);
}

/**
 * dp_peer_set_vlan_id: set vlan_id for this peer
 * @cdp_soc: soc handle
 * @vdev_id: vdev id for peer
 * @peer_mac: mac address
 * @vlan_id: vlan id for peer
 *
 * return: void
 */
void dp_peer_set_vlan_id(struct cdp_soc_t *cdp_soc,
		uint8_t vdev_id, uint8_t *peer_mac,
		uint16_t vlan_id)
{
	struct dp_soc *soc = (struct dp_soc *)cdp_soc;
	struct dp_vdev *vdev =
		dp_vdev_get_ref_by_id((struct dp_soc *)soc, vdev_id,
				      DP_MOD_ID_TX_MULTIPASS);

	if (vdev && vdev->multipass_en) {
		dp_peer_multipass_list_add(soc, peer_mac, vdev_id, vlan_id);
		dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_TX_MULTIPASS);
	}
}

/**
 * dp_set_vlan_groupkey: set vlan map for vdev
 * @soc: pointer to soc
 * @vdev_id : id of vdev
 * @vlan_id: vlan_id
 * @group_key: group key for vlan
 *
 * return: set success/failure
 */
QDF_STATUS dp_set_vlan_groupkey(struct cdp_soc_t *soc_hdl, uint8_t vdev_id,
				uint16_t vlan_id, uint16_t group_key)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_vdev *vdev = dp_vdev_get_ref_by_id(soc, vdev_id,
						     DP_MOD_ID_TX_MULTIPASS);
	QDF_STATUS status;

	if (!vdev || !vdev->multipass_en) {
		status = QDF_STATUS_E_INVAL;
		goto fail;
	}

	if (!vdev->iv_vlan_map) {
		uint16_t vlan_map_size = (sizeof(uint16_t))*DP_MAX_VLAN_IDS;
		vdev->iv_vlan_map = (uint16_t *)qdf_mem_malloc(vlan_map_size);

		if (!vdev->iv_vlan_map) {
			QDF_TRACE_ERROR(QDF_MODULE_ID_DP, "iv_vlan_map");
			status = QDF_STATUS_E_NOMEM;
			goto fail;
		}

		/*
		 * 0 is invalid group key.
		 * Initilalize array with invalid group keys.
		 */
		qdf_mem_zero(vdev->iv_vlan_map, vlan_map_size);
	}

	if (vlan_id >= DP_MAX_VLAN_IDS) {
		status = QDF_STATUS_E_INVAL;
		goto fail;
	}

	vdev->iv_vlan_map[vlan_id] = group_key;
	status = QDF_STATUS_SUCCESS;
fail:
	if (vdev)
		dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_TX_MULTIPASS);
	return status;
}

/**
 * dp_tx_vdev_multipass_deinit: set vlan map for vdev
 * @vdev_handle: pointer to vdev
 *
 * return: void
 */
void dp_tx_vdev_multipass_deinit(struct dp_vdev *vdev)
{
	struct dp_peer *peer = NULL;
	qdf_spin_lock_bh(&vdev->mpass_peer_mutex);
	TAILQ_FOREACH(peer, &vdev->mpass_peer_list, mpass_peer_list_elem)
		qdf_err("Peers present in mpass list : %llx",
			peer->mac_addr.raw);
	qdf_spin_unlock_bh(&vdev->mpass_peer_mutex);

	if (vdev->iv_vlan_map) {
		qdf_mem_free(vdev->iv_vlan_map);
		vdev->iv_vlan_map = NULL;
	}

	qdf_spinlock_destroy(&vdev->mpass_peer_mutex);
}

/**
 * dp_peer_multipass_list_init: initialize peer mulitpass list
 * @vdev_handle: pointer to vdev
 *
 * return: set success/failure
 */
void dp_peer_multipass_list_init(struct dp_vdev *vdev)
{
	/*
	 * vdev->iv_vlan_map is allocated when the first configuration command
	 * is issued to avoid unnecessary allocation for regular mode VAP.
	 */
	TAILQ_INIT(&vdev->mpass_peer_list);
	qdf_spinlock_create(&vdev->mpass_peer_mutex);
}
#endif /* QCA_MULTIPASS_SUPPORT */

#ifdef QCA_PEER_MULTIQ_SUPPORT

/**
 * dp_peer_reset_flowq_map() - reset peer flowq map table
 * @peer - dp peer handle
 *
 * Return: none
 */
void dp_peer_reset_flowq_map(struct dp_peer *peer)
{
	int i = 0;

	if (!peer)
		return;

	for (i = 0; i < DP_PEER_AST_FLOWQ_MAX; i++) {
		peer->peer_ast_flowq_idx[i].is_valid = false;
		peer->peer_ast_flowq_idx[i].valid_tid_mask = false;
		peer->peer_ast_flowq_idx[i].ast_idx = DP_INVALID_AST_IDX;
		peer->peer_ast_flowq_idx[i].flowQ = DP_INVALID_FLOW_PRIORITY;
	}
}

/**
 * dp_peer_get_flowid_from_flowmask() - get flow id from flow mask
 * @peer - dp peer handle
 * @mask - flow mask
 *
 * Return: flow id
 */
static int dp_peer_get_flowid_from_flowmask(struct dp_peer *peer,
		uint8_t mask)
{
	if (!peer) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: Invalid peer\n", __func__);
		return -1;
	}

	if (mask & DP_PEER_AST0_FLOW_MASK)
		return DP_PEER_AST_FLOWQ_UDP;
	else if (mask & DP_PEER_AST1_FLOW_MASK)
		return DP_PEER_AST_FLOWQ_NON_UDP;
	else if (mask & DP_PEER_AST2_FLOW_MASK)
		return DP_PEER_AST_FLOWQ_HI_PRIO;
	else if (mask & DP_PEER_AST3_FLOW_MASK)
		return DP_PEER_AST_FLOWQ_LOW_PRIO;

	return DP_PEER_AST_FLOWQ_MAX;
}

/**
 * dp_peer_get_ast_valid() - get ast index valid from mask
 * @mask - mask for ast valid bits
 * @index - index for an ast
 *
 * Return - 1 if ast index is valid from mask else 0
 */
static inline bool dp_peer_get_ast_valid(uint8_t mask, uint16_t index)
{
	if (index == 0)
		return 1;
	return ((mask) & (1 << ((index) - 1)));
}

/**
 * dp_peer_ast_index_flow_queue_map_create() - create ast index flow queue map
 * @soc - genereic soc handle
 * @is_wds - flag to indicate if peer is wds
 * @peer_id - peer_id from htt peer map message
 * @peer_mac_addr - mac address of the peer
 * @ast_info - ast flow override information from peer map
 *
 * Return: none
 */
void dp_peer_ast_index_flow_queue_map_create(void *soc_hdl,
		bool is_wds, uint16_t peer_id, uint8_t *peer_mac_addr,
		struct dp_ast_flow_override_info *ast_info)
{
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;
	struct dp_peer *peer = NULL;
	uint8_t i;

	/*
	 * Ast flow override feature is supported
	 * only for connected client
	 */
	if (is_wds)
		return;

	peer = dp_peer_get_ref_by_id(soc, peer_id, DP_MOD_ID_AST);
	if (!peer) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: Invalid peer\n", __func__);
		return;
	}

	/* Valid only in AP mode */
	if (peer->vdev->opmode != wlan_op_mode_ap) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"%s: Peer ast flow map not in STA mode\n", __func__);
		goto end;
	}

	/* Making sure the peer is for this mac address */
	if (!qdf_is_macaddr_equal((struct qdf_mac_addr *)peer_mac_addr,
				(struct qdf_mac_addr *)peer->mac_addr.raw)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: Peer mac address mismatch\n", __func__);
		goto end;
	}

	/* Ast entry flow mapping not valid for self peer map */
	if (qdf_is_macaddr_equal((struct qdf_mac_addr *)peer_mac_addr,
				(struct qdf_mac_addr *)peer->vdev->mac_addr.raw)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: Ast flow mapping not valid for self peer \n", __func__);
		goto end;
	}

	/* Fill up ast index <---> flow id mapping table for this peer */
	for (i = 0; i < DP_MAX_AST_INDEX_PER_PEER; i++) {

		/* Check if this ast index is valid */
		peer->peer_ast_flowq_idx[i].is_valid =
			dp_peer_get_ast_valid(ast_info->ast_valid_mask, i);
		if (!peer->peer_ast_flowq_idx[i].is_valid)
			continue;

		/* Get the flow queue id which is mapped to this ast index */
		peer->peer_ast_flowq_idx[i].flowQ =
			dp_peer_get_flowid_from_flowmask(peer,
					ast_info->ast_flow_mask[i]);
		/*
		 * Update tid valid mask only if flow id HIGH or
		 * Low priority
		 */
		if (peer->peer_ast_flowq_idx[i].flowQ ==
				DP_PEER_AST_FLOWQ_HI_PRIO) {
			peer->peer_ast_flowq_idx[i].valid_tid_mask =
				ast_info->tid_valid_hi_pri_mask;
		} else if (peer->peer_ast_flowq_idx[i].flowQ ==
				DP_PEER_AST_FLOWQ_LOW_PRIO) {
			peer->peer_ast_flowq_idx[i].valid_tid_mask =
				ast_info->tid_valid_low_pri_mask;
		}

		/* Save the ast index for this entry */
		peer->peer_ast_flowq_idx[i].ast_idx = ast_info->ast_idx[i];
	}

	if (soc->cdp_soc.ol_ops->peer_ast_flowid_map) {
		soc->cdp_soc.ol_ops->peer_ast_flowid_map(
				soc->ctrl_psoc, peer->peer_id,
				peer->vdev->vdev_id, peer_mac_addr);
	}

end:
	/* Release peer reference */
	dp_peer_unref_delete(peer, DP_MOD_ID_AST);
}

/**
 * dp_peer_find_ast_index_by_flowq_id() - API to get ast idx for a given flowid
 * @soc - soc handle
 * @peer_mac_addr - mac address of the peer
 * @flow_id - flow id to find ast index
 *
 * Return: ast index for a given flow id, -1 for fail cases
 */
int dp_peer_find_ast_index_by_flowq_id(struct cdp_soc_t *soc,
		uint16_t vdev_id, uint8_t *peer_mac_addr,
		uint8_t flow_id, uint8_t tid)
{
	struct dp_peer *peer = NULL;
	uint8_t i;
	uint16_t ast_index;

	if (flow_id >= DP_PEER_AST_FLOWQ_MAX) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"Invalid Flow ID %d\n", flow_id);
		return -1;
	}

	peer = dp_peer_find_hash_find((struct dp_soc *)soc,
				peer_mac_addr, 0, vdev_id,
				DP_MOD_ID_AST);
	if (!peer) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"%s: Invalid peer\n", __func__);
		return -1;
	}

	 /*
	  * Loop over the ast entry <----> flow-id mapping to find
	  * which ast index entry has this flow queue id enabled.
	  */
	for (i = 0; i < DP_PEER_AST_FLOWQ_MAX; i++) {
		if (peer->peer_ast_flowq_idx[i].flowQ == flow_id)
			/*
			 * Found the matching index for this flow id
			 */
			break;
	}

	/*
	 * No match found for this flow id
	 */
	if (i == DP_PEER_AST_FLOWQ_MAX) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"%s: ast index not found for flow %d\n", __func__, flow_id);
		dp_peer_unref_delete(peer, DP_MOD_ID_AST);
		return -1;
	}

	/* Check whether this ast entry is valid */
	if (!peer->peer_ast_flowq_idx[i].is_valid) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"%s: ast index is invalid for flow %d\n", __func__, flow_id);
		dp_peer_unref_delete(peer, DP_MOD_ID_AST);
		return -1;
	}

	if (flow_id == DP_PEER_AST_FLOWQ_HI_PRIO ||
			flow_id == DP_PEER_AST_FLOWQ_LOW_PRIO) {
		/*
		 * check if this tid is valid for Hi
		 * and Low priority flow id
		 */
		if ((peer->peer_ast_flowq_idx[i].valid_tid_mask
					& (1 << tid))) {
			/* Release peer reference */
			ast_index = peer->peer_ast_flowq_idx[i].ast_idx;
			dp_peer_unref_delete(peer, DP_MOD_ID_AST);
			return ast_index;
		} else {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
					"%s: TID %d is not valid for flow %d\n",
					__func__, tid, flow_id);
			/*
			 * TID is not valid for this flow
			 * Return -1
			 */
			dp_peer_unref_delete(peer, DP_MOD_ID_AST);
			return -1;
		}
	}

	/*
	 * TID valid check not required for
	 * UDP/NON UDP flow id
	 */
	ast_index = peer->peer_ast_flowq_idx[i].ast_idx;
	dp_peer_unref_delete(peer, DP_MOD_ID_AST);
	return ast_index;
}
#endif

void dp_hmwds_ast_add_notify(struct dp_peer *peer,
			     uint8_t *mac_addr,
			     enum cdp_txrx_ast_entry_type type,
			     QDF_STATUS err,
			     bool is_peer_map)
{
	struct dp_vdev *dp_vdev = peer->vdev;
	struct dp_pdev *dp_pdev = dp_vdev->pdev;
	struct cdp_peer_hmwds_ast_add_status add_status;

	/* Ignore ast types other than HM */
	if ((type != CDP_TXRX_AST_TYPE_WDS_HM) &&
	    (type != CDP_TXRX_AST_TYPE_WDS_HM_SEC))
		return;

	/* existing ast delete in progress, will be attempted
	 * to add again after delete is complete. Send status then.
	 */
	if (err == QDF_STATUS_E_AGAIN)
		return;

	/* peer map pending, notify actual status
	 * when peer map is received.
	 */
	if (!is_peer_map && (err == QDF_STATUS_SUCCESS))
		return;

	qdf_mem_zero(&add_status, sizeof(add_status));
	add_status.vdev_id = dp_vdev->vdev_id;
	/* For type CDP_TXRX_AST_TYPE_WDS_HM_SEC dp_peer_add_ast()
	 * returns QDF_STATUS_E_FAILURE as it is host only entry.
	 * In such cases set err as success. Also err code set to
	 * QDF_STATUS_E_ALREADY indicates entry already exist in
	 * such cases set err as success too. Any other error code
	 * is actual error.
	 */
	if (((type == CDP_TXRX_AST_TYPE_WDS_HM_SEC) &&
	     (err == QDF_STATUS_E_FAILURE)) ||
	    (err == QDF_STATUS_E_ALREADY)) {
		err = QDF_STATUS_SUCCESS;
	}
	add_status.status = err;
	qdf_mem_copy(add_status.peer_mac, peer->mac_addr.raw,
		     QDF_MAC_ADDR_SIZE);
	qdf_mem_copy(add_status.ast_mac, mac_addr,
		     QDF_MAC_ADDR_SIZE);
#ifdef WDI_EVENT_ENABLE
	dp_wdi_event_handler(WDI_EVENT_HMWDS_AST_ADD_STATUS, dp_pdev->soc,
			     (void *)&add_status, 0,
			     WDI_NO_VAL, dp_pdev->pdev_id);
#endif
}

#ifdef FEATURE_PERPKT_INFO
/**
 * dp_get_completion_indication_for_stack() - send completion to stack
 * @soc : dp_soc handle
 * @pdev: dp_pdev handle
 * @peer: dp peer handle
 * @ts: transmit completion status structure
 * @netbuf: Buffer pointer for free
 *
 * This function is used for indication whether buffer needs to be
 * sent to stack for freeing or not
*/
QDF_STATUS
dp_get_completion_indication_for_stack(struct dp_soc *soc,
				       struct dp_pdev *pdev,
				       struct dp_peer *peer,
				       struct hal_tx_completion_status *ts,
				       qdf_nbuf_t netbuf,
				       uint64_t time_latency)
{
	struct tx_capture_hdr *ppdu_hdr;
	uint16_t peer_id = ts->peer_id;
	uint32_t ppdu_id = ts->ppdu_id;
	uint8_t first_msdu = ts->first_msdu;
	uint8_t last_msdu = ts->last_msdu;
	uint32_t txcap_hdr_size = sizeof(struct tx_capture_hdr);

	if (qdf_unlikely(!pdev->tx_sniffer_enable && !pdev->mcopy_mode &&
			 !pdev->latency_capture_enable))
		return QDF_STATUS_E_NOSUPPORT;

	if (!peer) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  FL("Peer Invalid"));
		return QDF_STATUS_E_INVAL;
	}

	if (pdev->mcopy_mode) {
		/* If mcopy is enabled and mcopy_mode is M_COPY deliver 1st MSDU
		 * per PPDU. If mcopy_mode is M_COPY_EXTENDED deliver 1st MSDU
		 * for each MPDU
		 */
		if (pdev->mcopy_mode == M_COPY) {
			if ((pdev->m_copy_id.tx_ppdu_id == ppdu_id) &&
			    (pdev->m_copy_id.tx_peer_id == peer_id)) {
				return QDF_STATUS_E_INVAL;
			}
		}

		if (!first_msdu)
			return QDF_STATUS_E_INVAL;

		pdev->m_copy_id.tx_ppdu_id = ppdu_id;
		pdev->m_copy_id.tx_peer_id = peer_id;
	}

	if (qdf_unlikely(qdf_nbuf_headroom(netbuf) < txcap_hdr_size)) {
		netbuf = qdf_nbuf_realloc_headroom(netbuf, txcap_hdr_size);
		if (!netbuf) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				  FL("No headroom"));
			return QDF_STATUS_E_NOMEM;
		}
	}

	if (!qdf_nbuf_push_head(netbuf, txcap_hdr_size)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  FL("No headroom"));
		return QDF_STATUS_E_NOMEM;
	}

	ppdu_hdr = (struct tx_capture_hdr *)qdf_nbuf_data(netbuf);
	qdf_mem_copy(ppdu_hdr->ta, peer->vdev->mac_addr.raw,
		     QDF_MAC_ADDR_SIZE);
	qdf_mem_copy(ppdu_hdr->ra, peer->mac_addr.raw,
		     QDF_MAC_ADDR_SIZE);
	ppdu_hdr->ppdu_id = ppdu_id;
	ppdu_hdr->peer_id = peer_id;
	ppdu_hdr->first_msdu = first_msdu;
	ppdu_hdr->last_msdu = last_msdu;
	if (qdf_unlikely(pdev->latency_capture_enable)) {
		ppdu_hdr->tsf = ts->tsf;
		ppdu_hdr->time_latency = time_latency;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_send_completion_to_stack() - send completion to stack
 * @soc :  dp_soc handle
 * @pdev:  dp_pdev handle
 * @peer_id: peer_id of the peer for which completion came
 * @ppdu_id: ppdu_id
 * @netbuf: Buffer pointer for free
 *
 * This function is used to send completion to stack
 * to free buffer
*/
void dp_send_completion_to_stack(struct dp_soc *soc,  struct dp_pdev *pdev,
				 uint16_t peer_id, uint32_t ppdu_id,
				 qdf_nbuf_t netbuf)
{
	dp_wdi_event_handler(WDI_EVENT_TX_DATA, soc,
			     netbuf, peer_id,
			     WDI_NO_VAL, pdev->pdev_id);
}
#endif
