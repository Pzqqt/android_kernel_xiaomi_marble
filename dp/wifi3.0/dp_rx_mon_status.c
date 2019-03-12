/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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
#include "hal_rx.h"
#include "hal_api.h"
#include "qdf_trace.h"
#include "qdf_nbuf.h"
#include "hal_api_mon.h"
#include "dp_rx_mon.h"
#include "dp_internal.h"
#include "qdf_mem.h"   /* qdf_mem_malloc,free */

#ifdef FEATURE_PERPKT_INFO
#include "dp_ratetable.h"
#endif

/**
* dp_rx_populate_cdp_indication_ppdu() - Populate cdp rx indication structure
* @pdev: pdev ctx
* @ppdu_info: ppdu info structure from ppdu ring
* @ppdu_nbuf: qdf nbuf abstraction for linux skb
*
* Return: none
*/
#ifdef FEATURE_PERPKT_INFO
static inline void
dp_rx_populate_cdp_indication_ppdu(struct dp_pdev *pdev,
	struct hal_rx_ppdu_info *ppdu_info,
	qdf_nbuf_t ppdu_nbuf)
{
	struct dp_peer *peer;
	struct dp_soc *soc = pdev->soc;
	struct dp_ast_entry *ast_entry;
	struct cdp_rx_indication_ppdu *cdp_rx_ppdu;
	uint32_t ast_index;

	cdp_rx_ppdu = (struct cdp_rx_indication_ppdu *)ppdu_nbuf->data;

	cdp_rx_ppdu->first_data_seq_ctrl =
		ppdu_info->rx_status.first_data_seq_ctrl;
	cdp_rx_ppdu->frame_ctrl =
		ppdu_info->rx_status.frame_control;
	cdp_rx_ppdu->ppdu_id = ppdu_info->com_info.ppdu_id;
	cdp_rx_ppdu->length = ppdu_info->rx_status.ppdu_len;
	cdp_rx_ppdu->duration = ppdu_info->rx_status.duration;
	cdp_rx_ppdu->u.bw = ppdu_info->rx_status.bw;
	cdp_rx_ppdu->tcp_msdu_count = ppdu_info->rx_status.tcp_msdu_count;
	cdp_rx_ppdu->udp_msdu_count = ppdu_info->rx_status.udp_msdu_count;
	cdp_rx_ppdu->other_msdu_count = ppdu_info->rx_status.other_msdu_count;
	cdp_rx_ppdu->u.nss = ppdu_info->rx_status.nss;
	cdp_rx_ppdu->u.mcs = ppdu_info->rx_status.mcs;
	if ((ppdu_info->rx_status.sgi == VHT_SGI_NYSM) &&
		(ppdu_info->rx_status.preamble_type == HAL_RX_PKT_TYPE_11AC))
		cdp_rx_ppdu->u.gi = CDP_SGI_0_4_US;
	else
		cdp_rx_ppdu->u.gi = ppdu_info->rx_status.sgi;
	cdp_rx_ppdu->u.ldpc = ppdu_info->rx_status.ldpc;
	cdp_rx_ppdu->u.preamble = ppdu_info->rx_status.preamble_type;
	cdp_rx_ppdu->u.ppdu_type = ppdu_info->rx_status.reception_type;
	cdp_rx_ppdu->u.ltf_size = (ppdu_info->rx_status.he_data5 >>
				   QDF_MON_STATUS_HE_LTF_SIZE_SHIFT) & 0x3;
	cdp_rx_ppdu->num_mpdu = ppdu_info->com_info.mpdu_cnt_fcs_ok;
	cdp_rx_ppdu->rssi = ppdu_info->rx_status.rssi_comb;
	cdp_rx_ppdu->timestamp = ppdu_info->rx_status.tsft;
	cdp_rx_ppdu->channel = ppdu_info->rx_status.chan_num;
	cdp_rx_ppdu->beamformed = ppdu_info->rx_status.beamformed;
	cdp_rx_ppdu->num_msdu = (cdp_rx_ppdu->tcp_msdu_count +
			cdp_rx_ppdu->udp_msdu_count +
			cdp_rx_ppdu->other_msdu_count);
	cdp_rx_ppdu->num_bytes = ppdu_info->rx_status.ppdu_len;
	if (CDP_FC_IS_RETRY_SET(cdp_rx_ppdu->frame_ctrl))
		cdp_rx_ppdu->retries += ppdu_info->com_info.mpdu_cnt_fcs_ok;

	if (ppdu_info->com_info.mpdu_cnt_fcs_ok > 1)
		cdp_rx_ppdu->is_ampdu = 1;
	else
		cdp_rx_ppdu->is_ampdu = 0;

	cdp_rx_ppdu->tid = ppdu_info->rx_status.tid;
	cdp_rx_ppdu->lsig_a = ppdu_info->rx_status.rate;

	ast_index = ppdu_info->rx_status.ast_index;
	if (ast_index >= wlan_cfg_get_max_ast_idx(soc->wlan_cfg_ctx)) {
		cdp_rx_ppdu->peer_id = HTT_INVALID_PEER;
		return;
	}

	ast_entry = soc->ast_table[ast_index];
	if (!ast_entry) {
		cdp_rx_ppdu->peer_id = HTT_INVALID_PEER;
		return;
	}
	peer = ast_entry->peer;
	if (!peer || peer->peer_ids[0] == HTT_INVALID_PEER) {
		cdp_rx_ppdu->peer_id = HTT_INVALID_PEER;
		return;
	}

	qdf_mem_copy(cdp_rx_ppdu->mac_addr,
		     peer->mac_addr.raw, QDF_MAC_ADDR_SIZE);
	cdp_rx_ppdu->peer_id = peer->peer_ids[0];
	cdp_rx_ppdu->vdev_id = peer->vdev->vdev_id;
	cdp_rx_ppdu->u.ltf_size = ppdu_info->rx_status.ltf_size;
}
#else
static inline void
dp_rx_populate_cdp_indication_ppdu(struct dp_pdev *pdev,
		struct hal_rx_ppdu_info *ppdu_info,
		qdf_nbuf_t ppdu_nbuf)
{
}
#endif
/**
 * dp_rx_stats_update() - Update per-peer statistics
 * @soc: Datapath SOC handle
 * @peer: Datapath peer handle
 * @ppdu: PPDU Descriptor
 *
 * Return: None
 */
#ifdef FEATURE_PERPKT_INFO
static inline void dp_rx_rate_stats_update(struct dp_peer *peer,
					   struct cdp_rx_indication_ppdu *ppdu)
{
	uint32_t ratekbps = 0;
	uint32_t ppdu_rx_rate = 0;
	uint32_t nss = 0;
	uint32_t rix;

	if (!peer || !ppdu)
		return;

	if (ppdu->u.nss == 0)
		nss = 0;
	else
		nss = ppdu->u.nss - 1;

	ratekbps = dp_getrateindex(ppdu->u.gi,
				   ppdu->u.mcs,
				   nss,
				   ppdu->u.preamble,
				   ppdu->u.bw,
				   &rix);

	if (!ratekbps)
		return;

	ppdu->rix = rix;
	DP_STATS_UPD(peer, rx.last_rx_rate, ratekbps);
	dp_ath_rate_lpf(peer->stats.rx.avg_rx_rate, ratekbps);
	ppdu_rx_rate = dp_ath_rate_out(peer->stats.rx.avg_rx_rate);
	DP_STATS_UPD(peer, rx.rnd_avg_rx_rate, ppdu_rx_rate);

	if (peer->vdev)
		peer->vdev->stats.rx.last_rx_rate = ratekbps;
}

static void dp_rx_stats_update(struct dp_pdev *pdev, struct dp_peer *peer,
			       struct cdp_rx_indication_ppdu *ppdu)
{
	struct dp_soc *soc = NULL;
	uint8_t mcs, preamble, ac = 0;
	uint16_t num_msdu;
	bool is_invalid_peer = false;

	mcs = ppdu->u.mcs;
	preamble = ppdu->u.preamble;
	num_msdu = ppdu->num_msdu;

	if (pdev)
		soc = pdev->soc;
	else
		return;

	if (!peer) {
		is_invalid_peer = true;
		peer = pdev->invalid_peer;
	}

	if (!soc || soc->process_rx_status)
		return;

	DP_STATS_UPD(peer, rx.rssi, ppdu->rssi);

	if ((preamble == DOT11_A) || (preamble == DOT11_B))
		ppdu->u.nss = 1;

	if (ppdu->u.nss)
		DP_STATS_INC(peer, rx.nss[ppdu->u.nss - 1], num_msdu);

	DP_STATS_INC(peer, rx.sgi_count[ppdu->u.gi], num_msdu);
	DP_STATS_INC(peer, rx.bw[ppdu->u.bw], num_msdu);
	DP_STATS_INC(peer, rx.reception_type[ppdu->u.ppdu_type], num_msdu);
	DP_STATS_INCC(peer, rx.ampdu_cnt, num_msdu, ppdu->is_ampdu);
	DP_STATS_INCC(peer, rx.non_ampdu_cnt, num_msdu, !(ppdu->is_ampdu));
	DP_STATS_UPD(peer, rx.rx_rate, mcs);
	DP_STATS_INCC(peer,
			rx.pkt_type[preamble].mcs_count[MAX_MCS - 1], num_msdu,
			((mcs >= MAX_MCS_11A) && (preamble == DOT11_A)));
	DP_STATS_INCC(peer,
			rx.pkt_type[preamble].mcs_count[mcs], num_msdu,
			((mcs < MAX_MCS_11A) && (preamble == DOT11_A)));
	DP_STATS_INCC(peer,
			rx.pkt_type[preamble].mcs_count[MAX_MCS - 1], num_msdu,
			((mcs >= MAX_MCS_11B) && (preamble == DOT11_B)));
	DP_STATS_INCC(peer,
			rx.pkt_type[preamble].mcs_count[mcs], num_msdu,
			((mcs < MAX_MCS_11B) && (preamble == DOT11_B)));
	DP_STATS_INCC(peer,
			rx.pkt_type[preamble].mcs_count[MAX_MCS - 1], num_msdu,
			((mcs >= MAX_MCS_11A) && (preamble == DOT11_N)));
	DP_STATS_INCC(peer,
			rx.pkt_type[preamble].mcs_count[mcs], num_msdu,
			((mcs < MAX_MCS_11A) && (preamble == DOT11_N)));
	DP_STATS_INCC(peer,
			rx.pkt_type[preamble].mcs_count[MAX_MCS - 1], num_msdu,
			((mcs >= MAX_MCS_11AC) && (preamble == DOT11_AC)));
	DP_STATS_INCC(peer,
			rx.pkt_type[preamble].mcs_count[mcs], num_msdu,
			((mcs < MAX_MCS_11AC) && (preamble == DOT11_AC)));
	DP_STATS_INCC(peer,
			rx.pkt_type[preamble].mcs_count[MAX_MCS - 1], num_msdu,
			((mcs >= (MAX_MCS - 1)) && (preamble == DOT11_AX)));
	DP_STATS_INCC(peer,
			rx.pkt_type[preamble].mcs_count[mcs], num_msdu,
			((mcs < (MAX_MCS - 1)) && (preamble == DOT11_AX)));
	/*
	 * If invalid TID, it could be a non-qos frame, hence do not update
	 * any AC counters
	 */
	ac = TID_TO_WME_AC(ppdu->tid);
	if (ppdu->tid != HAL_TID_INVALID)
		DP_STATS_INC(peer, rx.wme_ac_type[ac], num_msdu);
	dp_peer_stats_notify(peer);
	DP_STATS_UPD(peer, rx.last_rssi, ppdu->rssi);

	if (is_invalid_peer)
		return;

	dp_rx_rate_stats_update(peer, ppdu);

#if defined(FEATURE_PERPKT_INFO) && WDI_EVENT_ENABLE
	dp_wdi_event_handler(WDI_EVENT_UPDATE_DP_STATS, pdev->soc,
			     &peer->stats, ppdu->peer_id,
			     UPDATE_PEER_STATS, pdev->pdev_id);
#endif
}
#endif

/**
 * dp_rx_handle_mcopy_mode() - Allocate and deliver first MSDU payload
 * @soc: core txrx main context
 * @pdev: pdev strcuture
 * @ppdu_info: structure for rx ppdu ring
 *
 * Return: QDF_STATUS_SUCCESS - If nbuf to be freed by caller
 *         QDF_STATUS_E_ALREADY - If nbuf not to be freed by caller
 */
#ifdef FEATURE_PERPKT_INFO
static inline QDF_STATUS
dp_rx_handle_mcopy_mode(struct dp_soc *soc, struct dp_pdev *pdev,
			struct hal_rx_ppdu_info *ppdu_info, qdf_nbuf_t nbuf)
{
	uint8_t size = 0;
	struct ieee80211_frame *wh;
	uint32_t *nbuf_data;

	if (!ppdu_info->msdu_info.first_msdu_payload)
		return QDF_STATUS_SUCCESS;

	if (pdev->m_copy_id.rx_ppdu_id == ppdu_info->com_info.ppdu_id)
		return QDF_STATUS_SUCCESS;

	pdev->m_copy_id.rx_ppdu_id = ppdu_info->com_info.ppdu_id;

	wh = (struct ieee80211_frame *)(ppdu_info->msdu_info.first_msdu_payload
					+ 4);
	size = (ppdu_info->msdu_info.first_msdu_payload -
				qdf_nbuf_data(nbuf));
	ppdu_info->msdu_info.first_msdu_payload = NULL;

	if (qdf_nbuf_pull_head(nbuf, size) == NULL)
		return QDF_STATUS_SUCCESS;

	if (((wh->i_fc[0] & IEEE80211_FC0_TYPE_MASK) ==
	     IEEE80211_FC0_TYPE_MGT) ||
	     ((wh->i_fc[0] & IEEE80211_FC0_TYPE_MASK) ==
	     IEEE80211_FC0_TYPE_CTL)) {
		return QDF_STATUS_SUCCESS;
	}

	nbuf_data = (uint32_t *)qdf_nbuf_data(nbuf);
	*nbuf_data = pdev->ppdu_info.com_info.ppdu_id;
	/* only retain RX MSDU payload in the skb */
	qdf_nbuf_trim_tail(nbuf, qdf_nbuf_len(nbuf) -
				ppdu_info->msdu_info.payload_len);
	dp_wdi_event_handler(WDI_EVENT_RX_DATA, soc,
			nbuf, HTT_INVALID_PEER, WDI_NO_VAL, pdev->pdev_id);
	return QDF_STATUS_E_ALREADY;
}
#else
static inline QDF_STATUS
dp_rx_handle_mcopy_mode(struct dp_soc *soc, struct dp_pdev *pdev,
			struct hal_rx_ppdu_info *ppdu_info, qdf_nbuf_t nbuf)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * dp_rx_handle_smart_mesh_mode() - Deliver header for smart mesh
 * @soc: Datapath SOC handle
 * @pdev: Datapath PDEV handle
 * @ppdu_info: Structure for rx ppdu info
 * @nbuf: Qdf nbuf abstraction for linux skb
 *
 * Return: 0 on success, 1 on failure
 */
static inline int
dp_rx_handle_smart_mesh_mode(struct dp_soc *soc, struct dp_pdev *pdev,
			      struct hal_rx_ppdu_info *ppdu_info,
			      qdf_nbuf_t nbuf)
{
	uint8_t size = 0;

	if (!pdev->monitor_vdev) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "[%s]:[%d] Monitor vdev is NULL !!",
			  __func__, __LINE__);
		return 1;
	}
	if (!ppdu_info->msdu_info.first_msdu_payload) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "[%s]:[%d] First msdu payload not present",
			  __func__, __LINE__);
		return 1;
	}

	/* Adding 4 bytes to get to start of 802.11 frame after phy_ppdu_id */
	size = (ppdu_info->msdu_info.first_msdu_payload -
		qdf_nbuf_data(nbuf)) + 4;
	ppdu_info->msdu_info.first_msdu_payload = NULL;

	if (qdf_nbuf_pull_head(nbuf, size) == NULL) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "[%s]:[%d] No header present",
			__func__, __LINE__);
		return 1;
	}

	/* Only retain RX MSDU payload in the skb */
	qdf_nbuf_trim_tail(nbuf, qdf_nbuf_len(nbuf) -
			   ppdu_info->msdu_info.payload_len);
	qdf_nbuf_update_radiotap(&(pdev->ppdu_info.rx_status),
				 nbuf, sizeof(struct rx_pkt_tlvs));
	pdev->monitor_vdev->osif_rx_mon(pdev->monitor_vdev->osif_vdev,
					nbuf, NULL);
	pdev->ppdu_info.rx_status.monitor_direct_used = 0;
	return 0;
}

/**
* dp_rx_handle_ppdu_stats() - Allocate and deliver ppdu stats to cdp layer
* @soc: core txrx main context
* @pdev: pdev strcuture
* @ppdu_info: structure for rx ppdu ring
*
* Return: none
*/
#ifdef FEATURE_PERPKT_INFO
static inline void
dp_rx_handle_ppdu_stats(struct dp_soc *soc, struct dp_pdev *pdev,
			struct hal_rx_ppdu_info *ppdu_info)
{
	qdf_nbuf_t ppdu_nbuf;
	struct dp_peer *peer;
	struct cdp_rx_indication_ppdu *cdp_rx_ppdu;

	/*
	 * Do not allocate if fcs error,
	 * ast idx invalid / fctl invalid
	 */
	if (ppdu_info->com_info.mpdu_cnt_fcs_ok == 0)
		return;

	if (ppdu_info->nac_info.fc_valid &&
	    ppdu_info->nac_info.to_ds_flag &&
	    ppdu_info->nac_info.mac_addr2_valid) {
		struct dp_neighbour_peer *peer = NULL;
		uint8_t rssi = ppdu_info->rx_status.rssi_comb;

		qdf_spin_lock_bh(&pdev->neighbour_peer_mutex);
		if (pdev->neighbour_peers_added) {
			TAILQ_FOREACH(peer, &pdev->neighbour_peers_list,
				      neighbour_peer_list_elem) {
				if (!qdf_mem_cmp(&peer->neighbour_peers_macaddr,
						 &ppdu_info->nac_info.mac_addr2,
						 QDF_MAC_ADDR_SIZE)) {
					peer->rssi = rssi;
					break;
				}
			}
		}
		qdf_spin_unlock_bh(&pdev->neighbour_peer_mutex);
	}

	/* need not generate wdi event when mcopy and
	 * enhanced stats are not enabled
	 */
	if (!pdev->mcopy_mode && !pdev->enhanced_stats_en)
		return;

	if (!pdev->mcopy_mode) {
		if (!ppdu_info->rx_status.frame_control_info_valid)
			return;

		if (ppdu_info->rx_status.ast_index == HAL_AST_IDX_INVALID)
			return;
	}
	ppdu_nbuf = qdf_nbuf_alloc(soc->osdev,
			sizeof(struct cdp_rx_indication_ppdu), 0, 0, FALSE);
	if (ppdu_nbuf) {
		dp_rx_populate_cdp_indication_ppdu(pdev, ppdu_info, ppdu_nbuf);
		qdf_nbuf_put_tail(ppdu_nbuf,
				sizeof(struct cdp_rx_indication_ppdu));
		cdp_rx_ppdu = (struct cdp_rx_indication_ppdu *)ppdu_nbuf->data;
		peer = dp_peer_find_by_id(soc, cdp_rx_ppdu->peer_id);
		if (peer) {
			cdp_rx_ppdu->cookie = (void *)peer->wlanstats_ctx;
			dp_rx_stats_update(pdev, peer, cdp_rx_ppdu);
			dp_peer_unref_del_find_by_id(peer);
		}
		if (cdp_rx_ppdu->peer_id != HTT_INVALID_PEER) {
			dp_wdi_event_handler(WDI_EVENT_RX_PPDU_DESC,
					     soc, ppdu_nbuf,
					     cdp_rx_ppdu->peer_id,
					     WDI_NO_VAL, pdev->pdev_id);
		} else if (pdev->mcopy_mode) {
			dp_wdi_event_handler(WDI_EVENT_RX_PPDU_DESC, soc,
					ppdu_nbuf, HTT_INVALID_PEER,
					WDI_NO_VAL, pdev->pdev_id);
		} else {
			qdf_nbuf_free(ppdu_nbuf);
		}
	}
}
#else
static inline void
dp_rx_handle_ppdu_stats(struct dp_soc *soc, struct dp_pdev *pdev,
			struct hal_rx_ppdu_info *ppdu_info)
{
}
#endif

/**
* dp_rx_process_peer_based_pktlog() - Process Rx pktlog if peer based
* filtering enabled
* @soc: core txrx main context
* @ppdu_info: Structure for rx ppdu info
* @status_nbuf: Qdf nbuf abstraction for linux skb
* @mac_id: mac_id/pdev_id correspondinggly for MCL and WIN
*
* Return: none
*/
static inline void
dp_rx_process_peer_based_pktlog(struct dp_soc *soc,
				struct hal_rx_ppdu_info *ppdu_info,
				qdf_nbuf_t status_nbuf, uint32_t mac_id)
{
	struct dp_peer *peer;
	struct dp_ast_entry *ast_entry;
	uint32_t ast_index;

	ast_index = ppdu_info->rx_status.ast_index;
	if (ast_index < (WLAN_UMAC_PSOC_MAX_PEERS * 2)) {
		ast_entry = soc->ast_table[ast_index];
		if (ast_entry) {
			peer = ast_entry->peer;
			if (peer && (peer->peer_ids[0] != HTT_INVALID_PEER)) {
				if (peer->peer_based_pktlog_filter) {
					dp_wdi_event_handler(
							WDI_EVENT_RX_DESC, soc,
							status_nbuf,
							peer->peer_ids[0],
							WDI_NO_VAL, mac_id);
				}
			}
		}
	}
}

/**
* dp_rx_mon_status_process_tlv() - Process status TLV in status
*	buffer on Rx status Queue posted by status SRNG processing.
* @soc: core txrx main context
* @mac_id: mac_id which is one of 3 mac_ids _ring
*
* Return: none
*/
static inline void
dp_rx_mon_status_process_tlv(struct dp_soc *soc, uint32_t mac_id,
	uint32_t quota)
{
	struct dp_pdev *pdev = dp_get_pdev_for_mac_id(soc, mac_id);
	struct hal_rx_ppdu_info *ppdu_info;
	qdf_nbuf_t status_nbuf;
	uint8_t *rx_tlv;
	uint8_t *rx_tlv_start;
	uint32_t tlv_status = HAL_TLV_STATUS_BUF_DONE;
	QDF_STATUS m_copy_status = QDF_STATUS_SUCCESS;
	struct cdp_pdev_mon_stats *rx_mon_stats;
	int smart_mesh_status;
	enum WDI_EVENT pktlog_mode = WDI_NO_VAL;

	ppdu_info = &pdev->ppdu_info;
	rx_mon_stats = &pdev->rx_mon_stats;

	if (pdev->mon_ppdu_status != DP_PPDU_STATUS_START)
		return;

	while (!qdf_nbuf_is_queue_empty(&pdev->rx_status_q)) {

		status_nbuf = qdf_nbuf_queue_remove(&pdev->rx_status_q);

		rx_tlv = qdf_nbuf_data(status_nbuf);
		rx_tlv_start = rx_tlv;

		if ((pdev->monitor_vdev) || (pdev->enhanced_stats_en) ||
				pdev->mcopy_mode) {

			do {
				tlv_status = hal_rx_status_get_tlv_info(rx_tlv,
						ppdu_info, pdev->soc->hal_soc);

				dp_rx_mon_update_dbg_ppdu_stats(ppdu_info,
								rx_mon_stats);

				rx_tlv = hal_rx_status_get_next_tlv(rx_tlv);

				if ((rx_tlv - rx_tlv_start) >= RX_BUFFER_SIZE)
					break;

			} while (tlv_status == HAL_TLV_STATUS_PPDU_NOT_DONE);
		}
		if (pdev->dp_peer_based_pktlog) {
			dp_rx_process_peer_based_pktlog(soc, ppdu_info,
							status_nbuf, mac_id);
		} else {
			if (pdev->rx_pktlog_mode == DP_RX_PKTLOG_FULL)
				pktlog_mode = WDI_EVENT_RX_DESC;
			else if (pdev->rx_pktlog_mode == DP_RX_PKTLOG_LITE)
				pktlog_mode = WDI_EVENT_LITE_RX;

			if (pktlog_mode != WDI_NO_VAL)
				dp_wdi_event_handler(pktlog_mode, soc,
						     status_nbuf,
						     HTT_INVALID_PEER,
						     WDI_NO_VAL, mac_id);
		}

		/* smart monitor vap and m_copy cannot co-exist */
		if (ppdu_info->rx_status.monitor_direct_used && pdev->neighbour_peers_added
		    && pdev->monitor_vdev) {
			smart_mesh_status = dp_rx_handle_smart_mesh_mode(soc,
						pdev, ppdu_info, status_nbuf);
			if (smart_mesh_status)
				qdf_nbuf_free(status_nbuf);
		} else if (pdev->mcopy_mode) {
			m_copy_status = dp_rx_handle_mcopy_mode(soc,
						pdev, ppdu_info, status_nbuf);
			if (m_copy_status == QDF_STATUS_SUCCESS)
				qdf_nbuf_free(status_nbuf);
		} else {
			qdf_nbuf_free(status_nbuf);
		}

		if (tlv_status == HAL_TLV_STATUS_PPDU_NON_STD_DONE) {
			dp_rx_mon_deliver_non_std(soc, mac_id);
		} else if (tlv_status == HAL_TLV_STATUS_PPDU_DONE) {
			rx_mon_stats->status_ppdu_done++;
			if (pdev->enhanced_stats_en ||
			    pdev->mcopy_mode || pdev->neighbour_peers_added)
				dp_rx_handle_ppdu_stats(soc, pdev, ppdu_info);

			pdev->mon_ppdu_status = DP_PPDU_STATUS_DONE;
			dp_rx_mon_dest_process(soc, mac_id, quota);
			pdev->mon_ppdu_status = DP_PPDU_STATUS_START;
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
 *
 * @soc: core txrx main context
 * @mac_id: mac_id which is one of 3 mac_ids
 * @quota: No. of ring entry that can be serviced in one shot.

 * Return: uint32_t: No. of ring entry that is processed.
 */
static inline uint32_t
dp_rx_mon_status_srng_process(struct dp_soc *soc, uint32_t mac_id,
	uint32_t quota)
{
	struct dp_pdev *pdev = dp_get_pdev_for_mac_id(soc, mac_id);
	void *hal_soc;
	void *mon_status_srng;
	void *rxdma_mon_status_ring_entry;
	QDF_STATUS status;
	uint32_t work_done = 0;
	int mac_for_pdev = dp_get_mac_id_for_mac(soc, mac_id);

	mon_status_srng = pdev->rxdma_mon_status_ring[mac_for_pdev].hal_srng;

	qdf_assert(mon_status_srng);
	if (!mon_status_srng || !hal_srng_initialized(mon_status_srng)) {

		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s %d : HAL Monitor Status Ring Init Failed -- %pK",
			__func__, __LINE__, mon_status_srng);
		return work_done;
	}

	hal_soc = soc->hal_soc;

	qdf_assert(hal_soc);

	if (qdf_unlikely(hal_srng_access_start(hal_soc, mon_status_srng)))
		goto done;

	/* mon_status_ring_desc => WBM_BUFFER_RING STRUCT =>
	 * BUFFER_ADDR_INFO STRUCT
	 */
	while (qdf_likely((rxdma_mon_status_ring_entry =
		hal_srng_src_peek(hal_soc, mon_status_srng))
			&& quota--)) {
		uint32_t rx_buf_cookie;
		qdf_nbuf_t status_nbuf;
		struct dp_rx_desc *rx_desc;
		uint8_t *status_buf;
		qdf_dma_addr_t paddr;
		uint64_t buf_addr;

		buf_addr =
			(HAL_RX_BUFFER_ADDR_31_0_GET(
				rxdma_mon_status_ring_entry) |
			((uint64_t)(HAL_RX_BUFFER_ADDR_39_32_GET(
				rxdma_mon_status_ring_entry)) << 32));

		if (qdf_likely(buf_addr)) {

			rx_buf_cookie =
				HAL_RX_BUF_COOKIE_GET(
					rxdma_mon_status_ring_entry);
			rx_desc = dp_rx_cookie_2_va_mon_status(soc,
				rx_buf_cookie);

			qdf_assert(rx_desc);

			status_nbuf = rx_desc->nbuf;

			qdf_nbuf_sync_for_cpu(soc->osdev, status_nbuf,
				QDF_DMA_FROM_DEVICE);

			status_buf = qdf_nbuf_data(status_nbuf);

			status = hal_get_rx_status_done(status_buf);

			if (status != QDF_STATUS_SUCCESS) {
				uint32_t hp, tp;
				hal_get_sw_hptp(hal_soc, mon_status_srng,
						&tp, &hp);
				QDF_TRACE(QDF_MODULE_ID_DP,
				QDF_TRACE_LEVEL_ERROR,
				"[%s][%d] status not done - hp:%u, tp:%u",
				__func__, __LINE__, hp, tp);
				/* WAR for missing status: Skip status entry */
				hal_srng_src_get_next(hal_soc, mon_status_srng);
				continue;
			}
			qdf_nbuf_set_pktlen(status_nbuf, RX_BUFFER_SIZE);

			qdf_nbuf_unmap_single(soc->osdev, status_nbuf,
				QDF_DMA_FROM_DEVICE);

			/* Put the status_nbuf to queue */
			qdf_nbuf_queue_add(&pdev->rx_status_q, status_nbuf);

		} else {
			union dp_rx_desc_list_elem_t *desc_list = NULL;
			union dp_rx_desc_list_elem_t *tail = NULL;
			struct rx_desc_pool *rx_desc_pool;
			uint32_t num_alloc_desc;

			rx_desc_pool = &soc->rx_desc_status[mac_id];

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

			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"%s: fail to allocate or map qdf_nbuf",
				__func__);
			dp_rx_add_to_free_desc_list(&desc_list,
						&tail, rx_desc);
			dp_rx_add_desc_list_to_free_list(soc, &desc_list,
						&tail, mac_id, rx_desc_pool);

			hal_rxdma_buff_addr_info_set(
						rxdma_mon_status_ring_entry,
						0, 0, HAL_RX_BUF_RBM_SW3_BM);
			work_done++;
			break;
		}

		paddr = qdf_nbuf_get_frag_paddr(status_nbuf, 0);

		rx_desc->nbuf = status_nbuf;
		rx_desc->in_use = 1;

		hal_rxdma_buff_addr_info_set(rxdma_mon_status_ring_entry,
			paddr, rx_desc->cookie, HAL_RX_BUF_RBM_SW3_BM);

		hal_srng_src_get_next(hal_soc, mon_status_srng);
		work_done++;
	}
done:

	hal_srng_access_end(hal_soc, mon_status_srng);

	return work_done;

}
/*
 * dp_rx_mon_status_process() - Process monitor status ring and
 *	TLV in status ring.
 *
 * @soc: core txrx main context
 * @mac_id: mac_id which is one of 3 mac_ids
 * @quota: No. of ring entry that can be serviced in one shot.

 * Return: uint32_t: No. of ring entry that is processed.
 */
static inline uint32_t
dp_rx_mon_status_process(struct dp_soc *soc, uint32_t mac_id, uint32_t quota) {
	uint32_t work_done;

	work_done = dp_rx_mon_status_srng_process(soc, mac_id, quota);
	quota -= work_done;
	dp_rx_mon_status_process_tlv(soc, mac_id, quota);

	return work_done;
}
/**
 * dp_mon_process() - Main monitor mode processing roution.
 *	This call monitor status ring process then monitor
 *	destination ring process.
 *	Called from the bottom half (tasklet/NET_RX_SOFTIRQ)
 * @soc: core txrx main context
 * @mac_id: mac_id which is one of 3 mac_ids
 * @quota: No. of status ring entry that can be serviced in one shot.

 * Return: uint32_t: No. of ring entry that is processed.
 */
uint32_t
dp_mon_process(struct dp_soc *soc, uint32_t mac_id, uint32_t quota) {
	return dp_rx_mon_status_process(soc, mac_id, quota);
}

/**
 * dp_rx_pdev_mon_status_detach() - detach dp rx for status ring
 * @pdev: core txrx pdev context
 * @mac_id: mac_id/pdev_id correspondinggly for MCL and WIN
 *
 * This function will detach DP RX status ring from
 * main device context. will free DP Rx resources for
 * status ring
 *
 * Return: QDF_STATUS_SUCCESS: success
 *         QDF_STATUS_E_RESOURCES: Error return
 */
QDF_STATUS
dp_rx_pdev_mon_status_detach(struct dp_pdev *pdev, int mac_id)
{
	struct dp_soc *soc = pdev->soc;
	struct rx_desc_pool *rx_desc_pool;

	rx_desc_pool = &soc->rx_desc_status[mac_id];
	if (rx_desc_pool->pool_size != 0) {
		if (!dp_is_soc_reinit(soc))
			dp_rx_desc_pool_free(soc, mac_id, rx_desc_pool);
		else
			dp_rx_desc_nbuf_pool_free(soc, rx_desc_pool);
	}

	return QDF_STATUS_SUCCESS;
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
	struct dp_pdev *dp_pdev = dp_get_pdev_for_mac_id(dp_soc, mac_id);

	rxdma_srng = dp_rxdma_srng->hal_srng;

	qdf_assert(rxdma_srng);

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
		"[%s][%d] requested %d buffers for replenish",
		__func__, __LINE__, num_req_buffers);

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
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"[%s][%d] no free rx_descs in freelist",
			__func__, __LINE__);
			return QDF_STATUS_E_NOMEM;
		}

		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"[%s][%d] %d rx desc allocated", __func__, __LINE__,
			num_alloc_desc);

		num_req_buffers = num_alloc_desc;
	}

	hal_srng_access_start(dp_soc->hal_soc, rxdma_srng);
	num_entries_avail = hal_srng_src_num_avail(dp_soc->hal_soc,
				rxdma_srng, sync_hw_ptr);

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
		"[%s][%d] no of available entries in rxdma ring: %d",
		  __func__, __LINE__, num_entries_avail);

	if (num_entries_avail < num_req_buffers) {
		num_desc_to_free = num_req_buffers - num_entries_avail;
		num_req_buffers = num_entries_avail;
	}

	while (count < num_req_buffers) {
		rx_netbuf = dp_rx_nbuf_prepare(dp_soc, dp_pdev);

		/*
		 * qdf_nbuf alloc or map failed,
		 * keep HP in mon_status_ring unchanged,
		 * wait dp_rx_mon_status_srng_process
		 * to fill in buffer at current HP.
		 */
		if (qdf_unlikely(!rx_netbuf)) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"%s: qdf_nbuf allocate or map fail, count %d",
				__func__, count);
			break;
		}

		paddr = qdf_nbuf_get_frag_paddr(rx_netbuf, 0);

		next = (*desc_list)->next;
		rxdma_ring_entry = hal_srng_src_get_next(dp_soc->hal_soc,
							 rxdma_srng);

		if (qdf_unlikely(!rxdma_ring_entry)) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
					"[%s][%d] rxdma_ring_entry is NULL, count - %d",
					__func__, __LINE__, count);
			qdf_nbuf_unmap_single(dp_soc->osdev, rx_netbuf,
					      QDF_DMA_BIDIRECTIONAL);
			qdf_nbuf_free(rx_netbuf);
			break;
		}

		(*desc_list)->rx_desc.nbuf = rx_netbuf;
		(*desc_list)->rx_desc.in_use = 1;
		count++;

		hal_rxdma_buff_addr_info_set(rxdma_ring_entry, paddr,
			(*desc_list)->rx_desc.cookie, owner);

		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"[%s][%d] rx_desc=%pK, cookie=%d, nbuf=%pK, \
			paddr=%pK",
			__func__, __LINE__, &(*desc_list)->rx_desc,
			(*desc_list)->rx_desc.cookie, rx_netbuf,
			(void *)paddr);

		*desc_list = next;
	}

	hal_srng_access_end(dp_soc->hal_soc, rxdma_srng);

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
		"successfully replenished %d buffers", num_req_buffers);

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
		"%d rx desc added back to free list", num_desc_to_free);

	/*
	 * add any available free desc back to the free list
	 */
	if (*desc_list) {
		dp_rx_add_desc_list_to_free_list(dp_soc, desc_list, tail,
			mac_id, rx_desc_pool);
	}

	return QDF_STATUS_SUCCESS;
}
/**
 * dp_rx_pdev_mon_status_attach() - attach DP RX monitor status ring
 * @pdev: core txrx pdev context
 * @ring_id: ring number
 * This function will attach a DP RX monitor status ring into pDEV
 * and replenish monitor status ring with buffer.
 *
 * Return: QDF_STATUS_SUCCESS: success
 *         QDF_STATUS_E_RESOURCES: Error return
 */
QDF_STATUS
dp_rx_pdev_mon_status_attach(struct dp_pdev *pdev, int ring_id) {
	struct dp_soc *soc = pdev->soc;
	union dp_rx_desc_list_elem_t *desc_list = NULL;
	union dp_rx_desc_list_elem_t *tail = NULL;
	struct dp_srng *mon_status_ring;
	uint32_t num_entries;
	struct rx_desc_pool *rx_desc_pool;
	QDF_STATUS status;
	int mac_for_pdev = dp_get_mac_id_for_mac(soc, ring_id);

	mon_status_ring = &pdev->rxdma_mon_status_ring[mac_for_pdev];

	num_entries = mon_status_ring->num_entries;

	rx_desc_pool = &soc->rx_desc_status[ring_id];

	dp_info("Mon RX Status Pool[%d] entries=%d",
		ring_id, num_entries);

	status = dp_rx_desc_pool_alloc(soc, ring_id, num_entries + 1,
				       rx_desc_pool);
	if (!QDF_IS_STATUS_SUCCESS(status))
		return status;

	dp_debug("Mon RX Status Buffers Replenish ring_id=%d", ring_id);

	status = dp_rx_mon_status_buffers_replenish(soc, ring_id,
						    mon_status_ring,
						    rx_desc_pool,
						    num_entries,
						    &desc_list, &tail,
						    HAL_RX_BUF_RBM_SW3_BM);

	if (!QDF_IS_STATUS_SUCCESS(status))
		return status;

	qdf_nbuf_queue_init(&pdev->rx_status_q);

	pdev->mon_ppdu_status = DP_PPDU_STATUS_START;

	qdf_mem_zero(&(pdev->ppdu_info.rx_status),
		     sizeof(pdev->ppdu_info.rx_status));

	qdf_mem_zero(&pdev->rx_mon_stats,
		     sizeof(pdev->rx_mon_stats));

	dp_rx_mon_init_dbg_ppdu_stats(&pdev->ppdu_info,
				      &pdev->rx_mon_stats);

	return QDF_STATUS_SUCCESS;
}
