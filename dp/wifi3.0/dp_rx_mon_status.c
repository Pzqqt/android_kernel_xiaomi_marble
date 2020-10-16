/*
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
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

#include "htt.h"

#ifdef FEATURE_PERPKT_INFO
#include "dp_ratetable.h"
#endif

static inline
QDF_STATUS dp_rx_mon_status_buffers_replenish(struct dp_soc *dp_soc,
					      uint32_t mac_id,
					      struct dp_srng *dp_rxdma_srng,
					      struct rx_desc_pool *rx_desc_pool,
					      uint32_t num_req_buffers,
					      union dp_rx_desc_list_elem_t **desc_list,
					      union dp_rx_desc_list_elem_t **tail,
					      uint8_t owner);

static inline void
dp_rx_populate_cfr_non_assoc_sta(struct dp_pdev *pdev,
				 struct hal_rx_ppdu_info *ppdu_info,
				 struct cdp_rx_indication_ppdu *cdp_rx_ppdu);

/**
 * dp_rx_mon_handle_status_buf_done () - Handle status buf DMA not done
 *
 * @pdev: DP pdev handle
 * @mon_status_srng: Monitor status SRNG
 *
 * As per MAC team's suggestion, If HP + 2 entry's DMA done is set,
 * skip HP + 1 entry and start processing in next interrupt.
 * If HP + 2 entry's DMA done is not set, poll onto HP + 1 entry
 * for it's DMA done TLV to be set.
 *
 * Return: enum dp_mon_reap_status
 */
enum dp_mon_reap_status
dp_rx_mon_handle_status_buf_done(struct dp_pdev *pdev,
				 void *mon_status_srng)
{
	struct dp_soc *soc = pdev->soc;
	hal_soc_handle_t hal_soc;
	void *ring_entry;
	uint32_t rx_buf_cookie;
	qdf_nbuf_t status_nbuf;
	struct dp_rx_desc *rx_desc;
	void *rx_tlv;
	QDF_STATUS buf_status;

	hal_soc = soc->hal_soc;

	ring_entry = hal_srng_src_peek_n_get_next_next(hal_soc,
						       mon_status_srng);
	if (!ring_entry) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  FL("Monitor status ring entry is NULL "
			     "for SRNG: %pK"),
			     mon_status_srng);
		return DP_MON_STATUS_NO_DMA;
	}
	rx_buf_cookie = HAL_RX_BUF_COOKIE_GET(ring_entry);
	rx_desc = dp_rx_cookie_2_va_mon_status(soc, rx_buf_cookie);

	qdf_assert(rx_desc);

	status_nbuf = rx_desc->nbuf;

	qdf_nbuf_sync_for_cpu(soc->osdev, status_nbuf,
			      QDF_DMA_FROM_DEVICE);

	rx_tlv = qdf_nbuf_data(status_nbuf);
	buf_status = hal_get_rx_status_done(rx_tlv);

	/* If status buffer DMA is not done,
	 * 1. As per MAC team's suggestion, If HP + 2 entry's DMA done is set,
	 * replenish HP + 1 entry and start processing in next interrupt.
	 * 2. If HP + 2 entry's DMA done is not set
	 * hold on to mon destination ring.
	 */
	if (buf_status != QDF_STATUS_SUCCESS) {
		dp_err_rl("Monitor status ring: DMA is not done "
			     "for nbuf: %pK", status_nbuf);
		pdev->rx_mon_stats.tlv_tag_status_err++;
		return DP_MON_STATUS_NO_DMA;
	}

	pdev->rx_mon_stats.status_buf_done_war++;

	return DP_MON_STATUS_REPLENISH;
}

#ifndef QCA_SUPPORT_FULL_MON
/**
 * dp_rx_mon_process () - Core brain processing for monitor mode
 *
 * This API processes monitor destination ring followed by monitor status ring
 * Called from bottom half (tasklet/NET_RX_SOFTIRQ)
 *
 * @soc: datapath soc context
 * @int_ctx: interrupt context
 * @mac_id: mac_id on which interrupt is received
 * @quota: Number of status ring entry that can be serviced in one shot.
 *
 * @Return: Number of reaped status ring entries
 */
static inline uint32_t
dp_rx_mon_process(struct dp_soc *soc, struct dp_intr *int_ctx,
		  uint32_t mac_id, uint32_t quota)
{
	return quota;
}
#endif

#ifdef WLAN_RX_PKT_CAPTURE_ENH
#include "dp_rx_mon_feature.h"
#else
static QDF_STATUS
dp_rx_handle_enh_capture(struct dp_soc *soc, struct dp_pdev *pdev,
			 struct hal_rx_ppdu_info *ppdu_info)
{
	return QDF_STATUS_SUCCESS;
}

static void
dp_rx_mon_enh_capture_process(struct dp_pdev *pdev, uint32_t tlv_status,
			      qdf_nbuf_t status_nbuf,
			      struct hal_rx_ppdu_info *ppdu_info,
			      bool *nbuf_used)
{
}
#endif

#ifdef WLAN_TX_PKT_CAPTURE_ENH
#include "dp_rx_mon_feature.h"
#else
static QDF_STATUS
dp_send_ack_frame_to_stack(struct dp_soc *soc,
			   struct dp_pdev *pdev,
			   struct hal_rx_ppdu_info *ppdu_info)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef FEATURE_PERPKT_INFO
static inline void
dp_rx_populate_rx_rssi_chain(struct hal_rx_ppdu_info *ppdu_info,
			     struct cdp_rx_indication_ppdu *cdp_rx_ppdu)
{
	uint8_t chain, bw;
	int8_t rssi;

	for (chain = 0; chain < SS_COUNT; chain++) {
		for (bw = 0; bw < MAX_BW; bw++) {
			rssi = ppdu_info->rx_status.rssi_chain[chain][bw];
			if (rssi != DP_RSSI_INVAL)
				cdp_rx_ppdu->rssi_chain[chain][bw] = rssi;
			else
				cdp_rx_ppdu->rssi_chain[chain][bw] = 0;
		}
	}
}

/*
 * dp_rx_populate_su_evm_details() - Populate su evm info
 * @ppdu_info: ppdu info structure from ppdu ring
 * @cdp_rx_ppdu: rx ppdu indication structure
 */
static inline void
dp_rx_populate_su_evm_details(struct hal_rx_ppdu_info *ppdu_info,
			      struct cdp_rx_indication_ppdu *cdp_rx_ppdu)
{
	uint8_t pilot_evm;
	uint8_t nss_count;
	uint8_t pilot_count;

	nss_count = ppdu_info->evm_info.nss_count;
	pilot_count = ppdu_info->evm_info.pilot_count;

	if ((nss_count * pilot_count) > DP_RX_MAX_SU_EVM_COUNT) {
		qdf_err("pilot evm count is more than expected");
		return;
	}
	cdp_rx_ppdu->evm_info.pilot_count = pilot_count;
	cdp_rx_ppdu->evm_info.nss_count = nss_count;

	/* Populate evm for pilot_evm  = nss_count*pilot_count */
	for (pilot_evm = 0; pilot_evm < nss_count * pilot_count; pilot_evm++) {
		cdp_rx_ppdu->evm_info.pilot_evm[pilot_evm] =
			ppdu_info->evm_info.pilot_evm[pilot_evm];
	}
}

/**
 * dp_rx_inc_rusize_cnt() - increment pdev stats based on RU size
 * @pdev: pdev ctx
 * @rx_user_status: mon rx user status
 *
 * Return: bool
 */
static inline bool
dp_rx_inc_rusize_cnt(struct dp_pdev *pdev,
		     struct mon_rx_user_status *rx_user_status)
{
	uint32_t ru_size;
	bool is_data;

	ru_size = rx_user_status->ofdma_ru_size;

	if (dp_is_subtype_data(rx_user_status->frame_control)) {
		DP_STATS_INC(pdev,
			     ul_ofdma.data_rx_ru_size[ru_size], 1);
		is_data = true;
	} else {
		DP_STATS_INC(pdev,
			     ul_ofdma.nondata_rx_ru_size[ru_size], 1);
		is_data = false;
	}

	return is_data;
}

/**
 * dp_rx_populate_cdp_indication_ppdu_user() - Populate per user cdp indication
 * @pdev: pdev ctx
 * @ppdu_info: ppdu info structure from ppdu ring
 * @cdp_rx_ppdu: Rx PPDU indication structure
 *
 * Return: none
 */
static inline void
dp_rx_populate_cdp_indication_ppdu_user(struct dp_pdev *pdev,
					struct hal_rx_ppdu_info *ppdu_info,
					struct cdp_rx_indication_ppdu
					*cdp_rx_ppdu)
{
	struct dp_peer *peer;
	struct dp_soc *soc = pdev->soc;
	struct dp_ast_entry *ast_entry;
	uint32_t ast_index;
	int i;
	struct mon_rx_user_status *rx_user_status;
	struct mon_rx_user_info *rx_user_info;
	struct cdp_rx_stats_ppdu_user *rx_stats_peruser;
	int ru_size;
	bool is_data = false;
	uint32_t num_users;

	num_users = ppdu_info->com_info.num_users;
	for (i = 0; i < num_users; i++) {
		if (i > OFDMA_NUM_USERS)
			return;

		rx_user_status =  &ppdu_info->rx_user_status[i];
		rx_user_info = &ppdu_info->rx_user_info[i];
		rx_stats_peruser = &cdp_rx_ppdu->user[i];

		ast_index = rx_user_status->ast_index;
		if (ast_index >= wlan_cfg_get_max_ast_idx(soc->wlan_cfg_ctx)) {
			rx_stats_peruser->peer_id = HTT_INVALID_PEER;
			continue;
		}

		ast_entry = soc->ast_table[ast_index];
		if (!ast_entry || ast_entry->peer_id == HTT_INVALID_PEER) {
			rx_stats_peruser->peer_id = HTT_INVALID_PEER;
			continue;
		}

		peer = dp_peer_get_ref_by_id(soc, ast_entry->peer_id,
					     DP_MOD_ID_RX_PPDU_STATS);
		if (!peer) {
			rx_stats_peruser->peer_id = HTT_INVALID_PEER;
			continue;
		}

		rx_stats_peruser->first_data_seq_ctrl =
			rx_user_status->first_data_seq_ctrl;

		rx_stats_peruser->frame_control_info_valid =
			rx_user_status->frame_control_info_valid;
		rx_stats_peruser->frame_control =
			rx_user_status->frame_control;

		rx_stats_peruser->qos_control_info_valid =
			rx_user_info->qos_control_info_valid;
		rx_stats_peruser->qos_control =
			rx_user_info->qos_control;
		rx_stats_peruser->tcp_msdu_count =
			rx_user_status->tcp_msdu_count;
		rx_stats_peruser->udp_msdu_count =
			rx_user_status->udp_msdu_count;
		rx_stats_peruser->other_msdu_count =
			rx_user_status->other_msdu_count;

		rx_stats_peruser->num_msdu =
			rx_stats_peruser->tcp_msdu_count +
			rx_stats_peruser->udp_msdu_count +
			rx_stats_peruser->other_msdu_count;

		rx_stats_peruser->preamble_type =
			rx_user_status->preamble_type;
		rx_stats_peruser->mpdu_cnt_fcs_ok =
			rx_user_status->mpdu_cnt_fcs_ok;
		rx_stats_peruser->mpdu_cnt_fcs_err =
			rx_user_status->mpdu_cnt_fcs_err;
		qdf_mem_copy(&rx_stats_peruser->mpdu_fcs_ok_bitmap,
			     &rx_user_status->mpdu_fcs_ok_bitmap,
			     HAL_RX_NUM_WORDS_PER_PPDU_BITMAP *
			     sizeof(rx_user_status->mpdu_fcs_ok_bitmap[0]));
		rx_stats_peruser->mpdu_ok_byte_count =
			rx_user_status->mpdu_ok_byte_count;
		rx_stats_peruser->mpdu_err_byte_count =
			rx_user_status->mpdu_err_byte_count;

		cdp_rx_ppdu->num_mpdu += rx_user_status->mpdu_cnt_fcs_ok;
		cdp_rx_ppdu->num_msdu += rx_stats_peruser->num_msdu;
		rx_stats_peruser->retries =
			CDP_FC_IS_RETRY_SET(rx_stats_peruser->frame_control) ?
			rx_stats_peruser->mpdu_cnt_fcs_ok : 0;

		if (rx_stats_peruser->mpdu_cnt_fcs_ok > 1)
			rx_stats_peruser->is_ampdu = 1;
		else
			rx_stats_peruser->is_ampdu = 0;

		rx_stats_peruser->tid = ppdu_info->rx_status.tid;

		qdf_mem_copy(rx_stats_peruser->mac_addr,
			     peer->mac_addr.raw, QDF_MAC_ADDR_SIZE);
		rx_stats_peruser->peer_id = peer->peer_id;
		cdp_rx_ppdu->vdev_id = peer->vdev->vdev_id;
		rx_stats_peruser->vdev_id = peer->vdev->vdev_id;
		rx_stats_peruser->mu_ul_info_valid = 0;

		dp_peer_unref_delete(peer, DP_MOD_ID_RX_PPDU_STATS);
		if (cdp_rx_ppdu->u.ppdu_type == HAL_RX_TYPE_MU_OFDMA ||
		    cdp_rx_ppdu->u.ppdu_type == HAL_RX_TYPE_MU_MIMO) {
			if (rx_user_status->mu_ul_info_valid) {
				rx_stats_peruser->nss = rx_user_status->nss;
				rx_stats_peruser->mcs = rx_user_status->mcs;
				rx_stats_peruser->mu_ul_info_valid =
					rx_user_status->mu_ul_info_valid;
				rx_stats_peruser->ofdma_ru_start_index =
					rx_user_status->ofdma_ru_start_index;
				rx_stats_peruser->ofdma_ru_width =
					rx_user_status->ofdma_ru_width;
				rx_stats_peruser->user_index = i;
				ru_size = rx_user_status->ofdma_ru_size;
				/*
				 * max RU size will be equal to
				 * HTT_UL_OFDMA_V0_RU_SIZE_RU_996x2
				 */
				if (ru_size >= OFDMA_NUM_RU_SIZE) {
					dp_err("invalid ru_size %d\n",
					       ru_size);
					return;
				}
				is_data = dp_rx_inc_rusize_cnt(pdev,
							       rx_user_status);
			}
			if (is_data) {
				/* counter to get number of MU OFDMA */
				pdev->stats.ul_ofdma.data_rx_ppdu++;
				pdev->stats.ul_ofdma.data_users[num_users]++;
			}
		}
	}
}

/**
 * dp_rx_populate_cdp_indication_ppdu() - Populate cdp rx indication structure
 * @pdev: pdev ctx
 * @ppdu_info: ppdu info structure from ppdu ring
 * @cdp_rx_ppdu: Rx PPDU indication structure
 *
 * Return: none
 */
static inline void
dp_rx_populate_cdp_indication_ppdu(struct dp_pdev *pdev,
				   struct hal_rx_ppdu_info *ppdu_info,
				   struct cdp_rx_indication_ppdu *cdp_rx_ppdu)
{
	struct dp_peer *peer;
	struct dp_soc *soc = pdev->soc;
	struct dp_ast_entry *ast_entry;
	uint32_t ast_index;
	uint32_t i;

	cdp_rx_ppdu->first_data_seq_ctrl =
		ppdu_info->rx_status.first_data_seq_ctrl;
	cdp_rx_ppdu->frame_ctrl =
		ppdu_info->rx_status.frame_control;
	cdp_rx_ppdu->tcp_msdu_count = ppdu_info->rx_status.tcp_msdu_count;
	cdp_rx_ppdu->udp_msdu_count = ppdu_info->rx_status.udp_msdu_count;
	cdp_rx_ppdu->other_msdu_count = ppdu_info->rx_status.other_msdu_count;
	cdp_rx_ppdu->u.preamble = ppdu_info->rx_status.preamble_type;
	/* num mpdu is consolidated and added together in num user loop */
	cdp_rx_ppdu->num_mpdu = ppdu_info->com_info.mpdu_cnt_fcs_ok;
	/* num msdu is consolidated and added together in num user loop */
	cdp_rx_ppdu->num_msdu = (cdp_rx_ppdu->tcp_msdu_count +
				 cdp_rx_ppdu->udp_msdu_count +
				 cdp_rx_ppdu->other_msdu_count);

	cdp_rx_ppdu->retries = CDP_FC_IS_RETRY_SET(cdp_rx_ppdu->frame_ctrl) ?
		ppdu_info->com_info.mpdu_cnt_fcs_ok : 0;

	if (ppdu_info->com_info.mpdu_cnt_fcs_ok > 1)
		cdp_rx_ppdu->is_ampdu = 1;
	else
		cdp_rx_ppdu->is_ampdu = 0;
	cdp_rx_ppdu->tid = ppdu_info->rx_status.tid;


	ast_index = ppdu_info->rx_status.ast_index;
	if (ast_index >= wlan_cfg_get_max_ast_idx(soc->wlan_cfg_ctx)) {
		cdp_rx_ppdu->peer_id = HTT_INVALID_PEER;
		cdp_rx_ppdu->num_users = 0;
		goto end;
	}

	ast_entry = soc->ast_table[ast_index];
	if (!ast_entry || ast_entry->peer_id == HTT_INVALID_PEER) {
		cdp_rx_ppdu->peer_id = HTT_INVALID_PEER;
		cdp_rx_ppdu->num_users = 0;
		goto end;
	}
	peer = dp_peer_get_ref_by_id(soc, ast_entry->peer_id,
				     DP_MOD_ID_RX_PPDU_STATS);
	if (!peer) {
		cdp_rx_ppdu->peer_id = HTT_INVALID_PEER;
		cdp_rx_ppdu->num_users = 0;
		goto end;
	}

	qdf_mem_copy(cdp_rx_ppdu->mac_addr,
		     peer->mac_addr.raw, QDF_MAC_ADDR_SIZE);
	cdp_rx_ppdu->peer_id = peer->peer_id;
	cdp_rx_ppdu->vdev_id = peer->vdev->vdev_id;

	cdp_rx_ppdu->ppdu_id = ppdu_info->com_info.ppdu_id;
	cdp_rx_ppdu->length = ppdu_info->rx_status.ppdu_len;
	cdp_rx_ppdu->duration = ppdu_info->rx_status.duration;
	cdp_rx_ppdu->u.bw = ppdu_info->rx_status.bw;
	cdp_rx_ppdu->u.nss = ppdu_info->rx_status.nss;
	cdp_rx_ppdu->u.mcs = ppdu_info->rx_status.mcs;
	if ((ppdu_info->rx_status.sgi == VHT_SGI_NYSM) &&
	    (ppdu_info->rx_status.preamble_type == HAL_RX_PKT_TYPE_11AC))
		cdp_rx_ppdu->u.gi = CDP_SGI_0_4_US;
	else
		cdp_rx_ppdu->u.gi = ppdu_info->rx_status.sgi;
	cdp_rx_ppdu->u.ldpc = ppdu_info->rx_status.ldpc;
	cdp_rx_ppdu->u.ppdu_type = ppdu_info->rx_status.reception_type;
	cdp_rx_ppdu->u.ltf_size = (ppdu_info->rx_status.he_data5 >>
				   QDF_MON_STATUS_HE_LTF_SIZE_SHIFT) & 0x3;
	cdp_rx_ppdu->rssi = ppdu_info->rx_status.rssi_comb;
	cdp_rx_ppdu->timestamp = ppdu_info->rx_status.tsft;
	cdp_rx_ppdu->channel = ppdu_info->rx_status.chan_num;
	cdp_rx_ppdu->beamformed = ppdu_info->rx_status.beamformed;
	cdp_rx_ppdu->num_bytes = ppdu_info->rx_status.ppdu_len;
	cdp_rx_ppdu->lsig_a = ppdu_info->rx_status.rate;
	cdp_rx_ppdu->u.ltf_size = ppdu_info->rx_status.ltf_size;

	dp_rx_populate_rx_rssi_chain(ppdu_info, cdp_rx_ppdu);
	dp_rx_populate_su_evm_details(ppdu_info, cdp_rx_ppdu);
	cdp_rx_ppdu->rx_antenna = ppdu_info->rx_status.rx_antenna;

	cdp_rx_ppdu->nf = ppdu_info->rx_status.chan_noise_floor;
	for (i = 0; i < MAX_CHAIN; i++)
		cdp_rx_ppdu->per_chain_rssi[i] = ppdu_info->rx_status.rssi[i];

	cdp_rx_ppdu->is_mcast_bcast = ppdu_info->nac_info.mcast_bcast;

	cdp_rx_ppdu->num_users = ppdu_info->com_info.num_users;

	cdp_rx_ppdu->num_mpdu = 0;
	cdp_rx_ppdu->num_msdu = 0;

	dp_rx_populate_cdp_indication_ppdu_user(pdev, ppdu_info, cdp_rx_ppdu);

	dp_peer_unref_delete(peer, DP_MOD_ID_RX_PPDU_STATS);

	return;
end:
	dp_rx_populate_cfr_non_assoc_sta(pdev, ppdu_info, cdp_rx_ppdu);
}
#else
static inline void
dp_rx_populate_cdp_indication_ppdu(struct dp_pdev *pdev,
				   struct hal_rx_ppdu_info *ppdu_info,
				   struct cdp_rx_indication_ppdu *cdp_rx_ppdu)
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
					   struct cdp_rx_indication_ppdu *ppdu,
					   uint32_t user)
{
	uint32_t ratekbps = 0;
	uint32_t ppdu_rx_rate = 0;
	uint32_t nss = 0;
	uint8_t mcs = 0;
	uint32_t rix;
	uint16_t ratecode;
	struct cdp_rx_stats_ppdu_user *ppdu_user = NULL;

	if (!peer || !ppdu)
		return;

	if (ppdu->u.ppdu_type != HAL_RX_TYPE_SU) {
		ppdu_user = &ppdu->user[user];

		if (ppdu_user->nss == 0)
			nss = 0;
		else
			nss = ppdu_user->nss - 1;
		mcs = ppdu_user->mcs;

	} else {
		if (ppdu->u.nss == 0)
			nss = 0;
		else
			nss = ppdu->u.nss - 1;
		mcs = ppdu->u.mcs;
	}

	ratekbps = dp_getrateindex(ppdu->u.gi,
				   mcs,
				   nss,
				   ppdu->u.preamble,
				   ppdu->u.bw,
				   &rix,
				   &ratecode);

	if (!ratekbps)
		return;

	ppdu->rix = rix;
	DP_STATS_UPD(peer, rx.last_rx_rate, ratekbps);
	dp_ath_rate_lpf(peer->stats.rx.avg_rx_rate, ratekbps);
	ppdu_rx_rate = dp_ath_rate_out(peer->stats.rx.avg_rx_rate);
	DP_STATS_UPD(peer, rx.rnd_avg_rx_rate, ppdu_rx_rate);
	ppdu->rx_ratekbps = ratekbps;
	ppdu->rx_ratecode = ratecode;

	if (peer->vdev)
		peer->vdev->stats.rx.last_rx_rate = ratekbps;
}

static void dp_rx_stats_update(struct dp_pdev *pdev,
			       struct cdp_rx_indication_ppdu *ppdu)
{
	struct dp_soc *soc = NULL;
	uint8_t mcs, preamble, ac = 0, nss, ppdu_type;
	uint16_t num_msdu;
	uint8_t pkt_bw_offset;
	struct dp_peer *peer;
	struct cdp_rx_stats_ppdu_user *ppdu_user;
	uint32_t i;
	enum cdp_mu_packet_type mu_pkt_type;

	if (pdev)
		soc = pdev->soc;
	else
		return;

	if (!soc || soc->process_rx_status)
		return;

	preamble = ppdu->u.preamble;
	ppdu_type = ppdu->u.ppdu_type;

	for (i = 0; i < ppdu->num_users && i < CDP_MU_MAX_USERS; i++) {
		peer = NULL;
		ppdu_user = &ppdu->user[i];
		peer = dp_peer_get_ref_by_id(soc, ppdu_user->peer_id,
					     DP_MOD_ID_RX_PPDU_STATS);

		if (!peer)
			peer = pdev->invalid_peer;

		if (ppdu_type == HAL_RX_TYPE_SU) {
			mcs = ppdu->u.mcs;
			nss = ppdu->u.nss;
		} else {
			mcs = ppdu_user->mcs;
			nss = ppdu_user->nss;
		}

		num_msdu = ppdu_user->num_msdu;
		switch (ppdu->u.bw) {
		case CMN_BW_20MHZ:
			pkt_bw_offset = PKT_BW_GAIN_20MHZ;
			break;
		case CMN_BW_40MHZ:
			pkt_bw_offset = PKT_BW_GAIN_40MHZ;
			break;
		case CMN_BW_80MHZ:
			pkt_bw_offset = PKT_BW_GAIN_80MHZ;
			break;
		case CMN_BW_160MHZ:
			pkt_bw_offset = PKT_BW_GAIN_160MHZ;
			break;
		default:
			pkt_bw_offset = 0;
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
				  "Invalid BW index = %d", ppdu->u.bw);
		}

		DP_STATS_UPD(peer, rx.rssi, (ppdu->rssi + pkt_bw_offset));

		if (peer->stats.rx.avg_rssi == INVALID_RSSI)
			peer->stats.rx.avg_rssi =
				CDP_RSSI_IN(peer->stats.rx.rssi);
		else
			CDP_RSSI_UPDATE_AVG(peer->stats.rx.avg_rssi,
					    peer->stats.rx.rssi);

		if ((preamble == DOT11_A) || (preamble == DOT11_B))
			nss = 1;

		if (ppdu_type == HAL_RX_TYPE_SU) {
			if (nss) {
				DP_STATS_INC(peer, rx.nss[nss - 1], num_msdu);
				DP_STATS_INC(peer, rx.ppdu_nss[nss - 1], 1);
			}

			DP_STATS_INC(peer, rx.mpdu_cnt_fcs_ok,
				     ppdu_user->mpdu_cnt_fcs_ok);
			DP_STATS_INC(peer, rx.mpdu_cnt_fcs_err,
				     ppdu_user->mpdu_cnt_fcs_err);
		}

		if (ppdu_type >= HAL_RX_TYPE_MU_MIMO &&
		    ppdu_type <= HAL_RX_TYPE_MU_OFDMA) {
			if (ppdu_type == HAL_RX_TYPE_MU_MIMO)
				mu_pkt_type = RX_TYPE_MU_MIMO;
			else
				mu_pkt_type = RX_TYPE_MU_OFDMA;

			if (nss) {
				DP_STATS_INC(peer, rx.nss[nss - 1], num_msdu);
				DP_STATS_INC(peer,
					rx.rx_mu[mu_pkt_type].ppdu_nss[nss - 1],
					1);
			}

			DP_STATS_INC(peer,
				     rx.rx_mu[mu_pkt_type].mpdu_cnt_fcs_ok,
				     ppdu_user->mpdu_cnt_fcs_ok);
			DP_STATS_INC(peer,
				     rx.rx_mu[mu_pkt_type].mpdu_cnt_fcs_err,
				     ppdu_user->mpdu_cnt_fcs_err);
		}

		DP_STATS_INC(peer, rx.sgi_count[ppdu->u.gi], num_msdu);
		DP_STATS_INC(peer, rx.bw[ppdu->u.bw], num_msdu);
		DP_STATS_INC(peer, rx.reception_type[ppdu->u.ppdu_type],
			     num_msdu);
		DP_STATS_INC(peer, rx.ppdu_cnt[ppdu->u.ppdu_type], 1);
		DP_STATS_INCC(peer, rx.ampdu_cnt, num_msdu,
			      ppdu_user->is_ampdu);
		DP_STATS_INCC(peer, rx.non_ampdu_cnt, num_msdu,
			      !(ppdu_user->is_ampdu));
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
		DP_STATS_INCC(peer,
			rx.su_ax_ppdu_cnt.mcs_count[MAX_MCS - 1], 1,
			((mcs >= (MAX_MCS - 1)) && (preamble == DOT11_AX) &&
			(ppdu_type == HAL_RX_TYPE_SU)));
		DP_STATS_INCC(peer,
			rx.su_ax_ppdu_cnt.mcs_count[mcs], 1,
			((mcs < (MAX_MCS - 1)) && (preamble == DOT11_AX) &&
			(ppdu_type == HAL_RX_TYPE_SU)));
		DP_STATS_INCC(peer,
			rx.rx_mu[RX_TYPE_MU_OFDMA].ppdu.mcs_count[MAX_MCS - 1],
			1, ((mcs >= (MAX_MCS - 1)) &&
			(preamble == DOT11_AX) &&
			(ppdu_type == HAL_RX_TYPE_MU_OFDMA)));
		DP_STATS_INCC(peer,
			rx.rx_mu[RX_TYPE_MU_OFDMA].ppdu.mcs_count[mcs],
			1, ((mcs < (MAX_MCS - 1)) &&
			(preamble == DOT11_AX) &&
			(ppdu_type == HAL_RX_TYPE_MU_OFDMA)));
		DP_STATS_INCC(peer,
			rx.rx_mu[RX_TYPE_MU_MIMO].ppdu.mcs_count[MAX_MCS - 1],
			1, ((mcs >= (MAX_MCS - 1)) &&
			(preamble == DOT11_AX) &&
			(ppdu_type == HAL_RX_TYPE_MU_MIMO)));
		DP_STATS_INCC(peer,
			rx.rx_mu[RX_TYPE_MU_MIMO].ppdu.mcs_count[mcs],
			1, ((mcs < (MAX_MCS - 1)) &&
			(preamble == DOT11_AX) &&
			(ppdu_type == HAL_RX_TYPE_MU_MIMO)));

		/*
		 * If invalid TID, it could be a non-qos frame, hence do not
		 * update any AC counters
		 */
		ac = TID_TO_WME_AC(ppdu_user->tid);

		if (ppdu->tid != HAL_TID_INVALID)
			DP_STATS_INC(peer, rx.wme_ac_type[ac], num_msdu);
		dp_peer_stats_notify(pdev, peer);
		DP_STATS_UPD(peer, rx.last_rssi, ppdu->rssi);

		dp_peer_qos_stats_notify(pdev, ppdu_user);
		if (peer == pdev->invalid_peer)
			continue;

		if (dp_is_subtype_data(ppdu->frame_ctrl))
			dp_rx_rate_stats_update(peer, ppdu, i);

#if defined(FEATURE_PERPKT_INFO) && WDI_EVENT_ENABLE
		dp_wdi_event_handler(WDI_EVENT_UPDATE_DP_STATS, pdev->soc,
				     &peer->stats, ppdu->peer_id,
				     UPDATE_PEER_STATS, pdev->pdev_id);
#endif
		dp_peer_unref_delete(peer, DP_MOD_ID_RX_PPDU_STATS);
	}
}
#endif

/**
 * dp_rx_handle_mcopy_mode() - Allocate and deliver first MSDU payload
 * @soc: core txrx main context
 * @pdev: pdev structure
 * @ppdu_info: structure for rx ppdu ring
 * @nbuf: QDF nbuf
 * @fcs_ok_mpdu_cnt: fcs passsed mpdu index
 * @deliver_frame: flag to deliver wdi event
 *
 * Return: QDF_STATUS_SUCCESS - If nbuf to be freed by caller
 *         QDF_STATUS_E_ALREADY - If nbuf not to be freed by caller
 */
#ifdef FEATURE_PERPKT_INFO
static inline QDF_STATUS
dp_rx_handle_mcopy_mode(struct dp_soc *soc, struct dp_pdev *pdev,
			struct hal_rx_ppdu_info *ppdu_info, qdf_nbuf_t nbuf,
			uint8_t fcs_ok_mpdu_cnt, bool deliver_frame)
{
	uint16_t size = 0;
	struct ieee80211_frame *wh;
	uint32_t *nbuf_data;

	if (!ppdu_info->ppdu_msdu_info[fcs_ok_mpdu_cnt].first_msdu_payload)
		return QDF_STATUS_SUCCESS;

	/* For M_COPY mode only one msdu per ppdu is sent to upper layer*/
	if (pdev->mcopy_mode == M_COPY) {
		if (pdev->m_copy_id.rx_ppdu_id == ppdu_info->com_info.ppdu_id)
			return QDF_STATUS_SUCCESS;
	}

	wh = (struct ieee80211_frame *)(ppdu_info->ppdu_msdu_info[fcs_ok_mpdu_cnt].first_msdu_payload + 4);

	size = (ppdu_info->ppdu_msdu_info[fcs_ok_mpdu_cnt].first_msdu_payload -
				qdf_nbuf_data(nbuf));

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
	qdf_nbuf_trim_tail(nbuf, qdf_nbuf_len(nbuf) - ppdu_info->ppdu_msdu_info[fcs_ok_mpdu_cnt].payload_len);
	if (deliver_frame) {
		pdev->m_copy_id.rx_ppdu_id = ppdu_info->com_info.ppdu_id;
		dp_wdi_event_handler(WDI_EVENT_RX_DATA, soc,
				     nbuf, HTT_INVALID_PEER,
				     WDI_NO_VAL, pdev->pdev_id);
	}
	return QDF_STATUS_E_ALREADY;
}
#else
static inline QDF_STATUS
dp_rx_handle_mcopy_mode(struct dp_soc *soc, struct dp_pdev *pdev,
			struct hal_rx_ppdu_info *ppdu_info, qdf_nbuf_t nbuf,
			uint8_t fcs_ok_cnt, bool deliver_frame)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * dp_rx_mcopy_handle_last_mpdu() - cache and delive last MPDU header in a
 * status buffer if MPDU end tlv is received in different buffer
 * @soc: core txrx main context
 * @pdev: pdev structure
 * @ppdu_info: structure for rx ppdu ring
 * @status_nbuf: QDF nbuf
 *
 * Return: void
 */
#ifdef FEATURE_PERPKT_INFO
static inline void
dp_rx_mcopy_handle_last_mpdu(struct dp_soc *soc, struct dp_pdev *pdev,
			     struct hal_rx_ppdu_info *ppdu_info,
			     qdf_nbuf_t status_nbuf)
{
	QDF_STATUS mcopy_status;
	qdf_nbuf_t nbuf_clone = NULL;
	/* If the MPDU end tlv and RX header are received in different buffers,
	 * process the RX header based on fcs status.
	 */
	if (pdev->mcopy_status_nbuf) {
		/* For M_COPY mode only one msdu per ppdu is sent to upper layer*/
		if (pdev->mcopy_mode == M_COPY) {
			if (pdev->m_copy_id.rx_ppdu_id ==
			    ppdu_info->com_info.ppdu_id)
				goto end1;
		}

		if (ppdu_info->is_fcs_passed) {
			nbuf_clone = qdf_nbuf_clone(pdev->mcopy_status_nbuf);
			if (!nbuf_clone) {
				QDF_TRACE(QDF_MODULE_ID_TXRX,
					  QDF_TRACE_LEVEL_ERROR,
					  "Failed to clone nbuf",
					  __func__, __LINE__);
				goto end1;
			}

			pdev->m_copy_id.rx_ppdu_id = ppdu_info->com_info.ppdu_id;
			dp_wdi_event_handler(WDI_EVENT_RX_DATA, soc,
					     nbuf_clone,
					     HTT_INVALID_PEER,
					     WDI_NO_VAL, pdev->pdev_id);
			ppdu_info->is_fcs_passed = false;
		}
end1:
		qdf_nbuf_free(pdev->mcopy_status_nbuf);
		pdev->mcopy_status_nbuf = NULL;
	}

	/* If the MPDU end tlv and RX header are received in different buffers,
	 * preserve the RX header as the fcs status will be received in MPDU
	 * end tlv in next buffer. So, cache the buffer to be processd in next
	 * iteration
	 */
	if ((ppdu_info->fcs_ok_cnt + ppdu_info->fcs_err_cnt) !=
	    ppdu_info->com_info.mpdu_cnt) {
		pdev->mcopy_status_nbuf = qdf_nbuf_clone(status_nbuf);
		if (pdev->mcopy_status_nbuf) {
			mcopy_status = dp_rx_handle_mcopy_mode(
							soc, pdev,
							ppdu_info,
							pdev->mcopy_status_nbuf,
							ppdu_info->fcs_ok_cnt,
							false);
			if (mcopy_status == QDF_STATUS_SUCCESS) {
				qdf_nbuf_free(pdev->mcopy_status_nbuf);
				pdev->mcopy_status_nbuf = NULL;
			}
		}
	}
}
#else
static inline void
dp_rx_mcopy_handle_last_mpdu(struct dp_soc *soc, struct dp_pdev *pdev,
			     struct hal_rx_ppdu_info *ppdu_info,
			     qdf_nbuf_t status_nbuf)
{
}
#endif

/**
 * dp_rx_mcopy_process_ppdu_info() - update mcopy ppdu info
 * @ppdu_info: structure for rx ppdu ring
 * @tlv_status: processed TLV status
 *
 * Return: void
 */
#ifdef FEATURE_PERPKT_INFO
static inline void
dp_rx_mcopy_process_ppdu_info(struct dp_pdev *pdev,
			      struct hal_rx_ppdu_info *ppdu_info,
			      uint32_t tlv_status)
{
	if (!pdev->mcopy_mode)
		return;

	/* The fcs status is received in MPDU end tlv. If the RX header
	 * and its MPDU end tlv are received in different status buffer then
	 * to process that header ppdu_info->is_fcs_passed is used.
	 * If end tlv is received in next status buffer then com_info.mpdu_cnt
	 * will be 0 at the time of receiving MPDU end tlv and we update the
	 * is_fcs_passed flag based on ppdu_info->fcs_err.
	 */
	if (tlv_status != HAL_TLV_STATUS_MPDU_END)
		return;

	if (!ppdu_info->fcs_err) {
		if (ppdu_info->fcs_ok_cnt >
		    HAL_RX_MAX_MPDU_H_PER_STATUS_BUFFER) {
			dp_err("No. of MPDUs(%d) per status buff exceeded",
					ppdu_info->fcs_ok_cnt);
			return;
		}
		if (ppdu_info->com_info.mpdu_cnt)
			ppdu_info->fcs_ok_cnt++;
		else
			ppdu_info->is_fcs_passed = true;
	} else {
		if (ppdu_info->com_info.mpdu_cnt)
			ppdu_info->fcs_err_cnt++;
		else
			ppdu_info->is_fcs_passed = false;
	}
}
#else
static inline void
dp_rx_mcopy_process_ppdu_info(struct dp_pdev *pdev,
			      struct hal_rx_ppdu_info *ppdu_info,
			      uint32_t tlv_status)
{
}
#endif

#ifdef FEATURE_PERPKT_INFO
static inline void
dp_rx_process_mcopy_mode(struct dp_soc *soc, struct dp_pdev *pdev,
			 struct hal_rx_ppdu_info *ppdu_info,
			 uint32_t tlv_status,
			 qdf_nbuf_t status_nbuf)
{
	QDF_STATUS mcopy_status;
	qdf_nbuf_t nbuf_clone = NULL;
	uint8_t fcs_ok_mpdu_cnt = 0;

	dp_rx_mcopy_handle_last_mpdu(soc, pdev, ppdu_info, status_nbuf);

	if (qdf_unlikely(!ppdu_info->com_info.mpdu_cnt))
		goto end;

	if (qdf_unlikely(!ppdu_info->fcs_ok_cnt))
		goto end;

	/* For M_COPY mode only one msdu per ppdu is sent to upper layer*/
	if (pdev->mcopy_mode == M_COPY)
		ppdu_info->fcs_ok_cnt = 1;

	while (fcs_ok_mpdu_cnt < ppdu_info->fcs_ok_cnt) {
		nbuf_clone = qdf_nbuf_clone(status_nbuf);
		if (!nbuf_clone) {
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				  "Failed to clone nbuf",
				  __func__, __LINE__);
			goto end;
		}

		mcopy_status = dp_rx_handle_mcopy_mode(soc, pdev,
						       ppdu_info,
						       nbuf_clone,
						       fcs_ok_mpdu_cnt,
						       true);

		if (mcopy_status == QDF_STATUS_SUCCESS)
			qdf_nbuf_free(nbuf_clone);

		fcs_ok_mpdu_cnt++;
	}
end:
	qdf_nbuf_free(status_nbuf);
	ppdu_info->fcs_ok_cnt = 0;
	ppdu_info->fcs_err_cnt = 0;
	ppdu_info->com_info.mpdu_cnt = 0;
	qdf_mem_zero(&ppdu_info->ppdu_msdu_info,
		     HAL_RX_MAX_MPDU_H_PER_STATUS_BUFFER
		     * sizeof(struct hal_rx_msdu_payload_info));

}
#else
static inline void
dp_rx_process_mcopy_mode(struct dp_soc *soc, struct dp_pdev *pdev,
			 struct hal_rx_ppdu_info *ppdu_info,
			 uint32_t tlv_status,
			 qdf_nbuf_t status_nbuf)
{
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
	if (!qdf_nbuf_update_radiotap(&pdev->ppdu_info.rx_status, nbuf,
				      qdf_nbuf_headroom(nbuf))) {
		DP_STATS_INC(pdev, dropped.mon_radiotap_update_err, 1);
		return 1;
	}

	pdev->monitor_vdev->osif_rx_mon(pdev->monitor_vdev->osif_vdev,
					nbuf, NULL);
	pdev->ppdu_info.rx_status.monitor_direct_used = 0;
	return 0;
}

#if defined(WLAN_CFR_ENABLE) && defined(WLAN_ENH_CFR_ENABLE)
/*
 * dp_rx_mon_handle_cfr_mu_info() - Gather macaddr and ast_index of peer(s) in
 * the PPDU received, this will be used for correlation of CFR data captured
 * for an UL-MU-PPDU
 * @pdev: pdev ctx
 * @ppdu_info: pointer to ppdu info structure populated from ppdu status TLVs
 * @cdp_rx_ppdu: Rx PPDU indication structure
 *
 * Return: none
 */
static inline void
dp_rx_mon_handle_cfr_mu_info(struct dp_pdev *pdev,
			     struct hal_rx_ppdu_info *ppdu_info,
			     struct cdp_rx_indication_ppdu *cdp_rx_ppdu)
{
	struct dp_peer *peer;
	struct dp_soc *soc = pdev->soc;
	struct dp_ast_entry *ast_entry;
	struct mon_rx_user_status *rx_user_status;
	struct cdp_rx_stats_ppdu_user *rx_stats_peruser;
	uint32_t num_users;
	int user_id;
	uint32_t ast_index;

	qdf_spin_lock_bh(&soc->ast_lock);

	num_users = ppdu_info->com_info.num_users;
	for (user_id = 0; user_id < num_users; user_id++) {
		if (user_id > OFDMA_NUM_USERS) {
			qdf_spin_unlock_bh(&soc->ast_lock);
			return;
		}

		rx_user_status =  &ppdu_info->rx_user_status[user_id];
		rx_stats_peruser = &cdp_rx_ppdu->user[user_id];
		ast_index = rx_user_status->ast_index;

		if (ast_index >= wlan_cfg_get_max_ast_idx(soc->wlan_cfg_ctx)) {
			rx_stats_peruser->peer_id = HTT_INVALID_PEER;
			continue;
		}

		ast_entry = soc->ast_table[ast_index];
		if (!ast_entry || ast_entry->peer_id == HTT_INVALID_PEER) {
			rx_stats_peruser->peer_id = HTT_INVALID_PEER;
			continue;
		}

		peer = dp_peer_get_ref_by_id(soc, ast_entry->peer_id,
					     DP_MOD_ID_RX_PPDU_STATS);
		if (!peer) {
			rx_stats_peruser->peer_id = HTT_INVALID_PEER;
			continue;
		}

		qdf_mem_copy(rx_stats_peruser->mac_addr,
			     peer->mac_addr.raw, QDF_MAC_ADDR_SIZE);
		dp_peer_unref_delete(peer, DP_MOD_ID_RX_PPDU_STATS);
	}

	qdf_spin_unlock_bh(&soc->ast_lock);
}

/*
 * dp_rx_mon_populate_cfr_ppdu_info() - Populate cdp ppdu info from hal ppdu
 * info
 * @pdev: pdev ctx
 * @ppdu_info: ppdu info structure from ppdu ring
 * @cdp_rx_ppdu : Rx PPDU indication structure
 *
 * Return: none
 */
static inline void
dp_rx_mon_populate_cfr_ppdu_info(struct dp_pdev *pdev,
				 struct hal_rx_ppdu_info *ppdu_info,
				 struct cdp_rx_indication_ppdu *cdp_rx_ppdu)
{
	int chain;

	cdp_rx_ppdu->ppdu_id = ppdu_info->com_info.ppdu_id;
	cdp_rx_ppdu->timestamp = ppdu_info->rx_status.tsft;
	cdp_rx_ppdu->u.ppdu_type = ppdu_info->rx_status.reception_type;
	cdp_rx_ppdu->num_users = ppdu_info->com_info.num_users;

	for (chain = 0; chain < MAX_CHAIN; chain++)
		cdp_rx_ppdu->per_chain_rssi[chain] =
			ppdu_info->rx_status.rssi[chain];
	dp_rx_mon_handle_cfr_mu_info(pdev, ppdu_info, cdp_rx_ppdu);
}

/**
 * dp_cfr_rcc_mode_status() - Return status of cfr rcc mode
 * @pdev: pdev ctx
 *
 * Return: True or False
 */

static inline bool
dp_cfr_rcc_mode_status(struct dp_pdev *pdev)
{
	return pdev->cfr_rcc_mode;
}

/*
 * dp_rx_mon_populate_cfr_info() - Populate cdp ppdu info from hal cfr info
 * @pdev: pdev ctx
 * @ppdu_info: ppdu info structure from ppdu ring
 * @cdp_rx_ppdu: Rx PPDU indication structure
 *
 * Return: none
 */
static inline void
dp_rx_mon_populate_cfr_info(struct dp_pdev *pdev,
			    struct hal_rx_ppdu_info *ppdu_info,
			    struct cdp_rx_indication_ppdu *cdp_rx_ppdu)
{
	struct cdp_rx_ppdu_cfr_info *cfr_info;

	if (!dp_cfr_rcc_mode_status(pdev))
		return;

	cfr_info = &cdp_rx_ppdu->cfr_info;

	cfr_info->bb_captured_channel
		= ppdu_info->cfr_info.bb_captured_channel;
	cfr_info->bb_captured_timeout
		= ppdu_info->cfr_info.bb_captured_timeout;
	cfr_info->bb_captured_reason
		= ppdu_info->cfr_info.bb_captured_reason;
	cfr_info->rx_location_info_valid
		= ppdu_info->cfr_info.rx_location_info_valid;
	cfr_info->chan_capture_status
		= ppdu_info->cfr_info.chan_capture_status;
	cfr_info->rtt_che_buffer_pointer_high8
		= ppdu_info->cfr_info.rtt_che_buffer_pointer_high8;
	cfr_info->rtt_che_buffer_pointer_low32
		= ppdu_info->cfr_info.rtt_che_buffer_pointer_low32;
}

/**
 * dp_update_cfr_dbg_stats() - Increment RCC debug statistics
 * @pdev: pdev structure
 * @ppdu_info: structure for rx ppdu ring
 *
 * Return: none
 */
static inline void
dp_update_cfr_dbg_stats(struct dp_pdev *pdev,
			struct hal_rx_ppdu_info *ppdu_info)
{
	struct hal_rx_ppdu_cfr_info *cfr = &ppdu_info->cfr_info;

	DP_STATS_INC(pdev,
		     rcc.chan_capture_status[cfr->chan_capture_status], 1);
	if (cfr->rx_location_info_valid) {
		DP_STATS_INC(pdev, rcc.rx_loc_info_valid_cnt, 1);
		if (cfr->bb_captured_channel) {
			DP_STATS_INC(pdev, rcc.bb_captured_channel_cnt, 1);
			DP_STATS_INC(pdev,
				     rcc.reason_cnt[cfr->bb_captured_reason],
				     1);
		} else if (cfr->bb_captured_timeout) {
			DP_STATS_INC(pdev, rcc.bb_captured_timeout_cnt, 1);
			DP_STATS_INC(pdev,
				     rcc.reason_cnt[cfr->bb_captured_reason],
				     1);
		}
	}
}

/*
 * dp_rx_handle_cfr() - Gather cfr info from hal ppdu info
 * @soc: core txrx main context
 * @pdev: pdev ctx
 * @ppdu_info: ppdu info structure from ppdu ring
 *
 * Return: none
 */
static inline void
dp_rx_handle_cfr(struct dp_soc *soc, struct dp_pdev *pdev,
		 struct hal_rx_ppdu_info *ppdu_info)
{
	qdf_nbuf_t ppdu_nbuf;
	struct cdp_rx_indication_ppdu *cdp_rx_ppdu;

	dp_update_cfr_dbg_stats(pdev, ppdu_info);
	if (!ppdu_info->cfr_info.bb_captured_channel)
		return;

	ppdu_nbuf = qdf_nbuf_alloc(soc->osdev,
				   sizeof(struct cdp_rx_indication_ppdu),
				   0,
				   0,
				   FALSE);
	if (ppdu_nbuf) {
		cdp_rx_ppdu = (struct cdp_rx_indication_ppdu *)ppdu_nbuf->data;

		dp_rx_mon_populate_cfr_info(pdev, ppdu_info, cdp_rx_ppdu);
		dp_rx_mon_populate_cfr_ppdu_info(pdev, ppdu_info, cdp_rx_ppdu);
		qdf_nbuf_put_tail(ppdu_nbuf,
				  sizeof(struct cdp_rx_indication_ppdu));
		dp_wdi_event_handler(WDI_EVENT_RX_PPDU_DESC, soc,
				     ppdu_nbuf, HTT_INVALID_PEER,
				     WDI_NO_VAL, pdev->pdev_id);
	}
}

/**
 * dp_rx_populate_cfr_non_assoc_sta() - Populate cfr ppdu info for PPDUs from
 * non-associated stations
 * @pdev: pdev ctx
 * @ppdu_info: ppdu info structure from ppdu ring
 * @cdp_rx_ppdu: Rx PPDU indication structure
 *
 * Return: none
 */
static inline void
dp_rx_populate_cfr_non_assoc_sta(struct dp_pdev *pdev,
				 struct hal_rx_ppdu_info *ppdu_info,
				 struct cdp_rx_indication_ppdu *cdp_rx_ppdu)
{
	if (!dp_cfr_rcc_mode_status(pdev))
		return;

	if (ppdu_info->cfr_info.bb_captured_channel)
		dp_rx_mon_populate_cfr_ppdu_info(pdev, ppdu_info, cdp_rx_ppdu);
}

/**
 * dp_bb_captured_chan_status() - Get the bb_captured_channel status
 * @ppdu_info: structure for rx ppdu ring
 *
 * Return: Success/ Failure
 */

static inline QDF_STATUS
dp_bb_captured_chan_status(struct dp_pdev *pdev,
			   struct hal_rx_ppdu_info *ppdu_info)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct hal_rx_ppdu_cfr_info *cfr = &ppdu_info->cfr_info;

	if (dp_cfr_rcc_mode_status(pdev)) {
		if (cfr->bb_captured_channel)
			status = QDF_STATUS_SUCCESS;
	}

	return status;
}
#else
static inline void
dp_rx_mon_handle_cfr_mu_info(struct dp_pdev *pdev,
			     struct hal_rx_ppdu_info *ppdu_info,
			     struct cdp_rx_indication_ppdu *cdp_rx_ppdu)
{
}

static inline void
dp_rx_mon_populate_cfr_ppdu_info(struct dp_pdev *pdev,
				 struct hal_rx_ppdu_info *ppdu_info,
				 struct cdp_rx_indication_ppdu *cdp_rx_ppdu)
{
}

static inline void
dp_rx_mon_populate_cfr_info(struct dp_pdev *pdev,
			    struct hal_rx_ppdu_info *ppdu_info,
			    struct cdp_rx_indication_ppdu *cdp_rx_ppdu)
{
}

static inline void
dp_rx_handle_cfr(struct dp_soc *soc, struct dp_pdev *pdev,
		 struct hal_rx_ppdu_info *ppdu_info)
{
}

static inline void
dp_rx_populate_cfr_non_assoc_sta(struct dp_pdev *pdev,
				 struct hal_rx_ppdu_info *ppdu_info,
				 struct cdp_rx_indication_ppdu *cdp_rx_ppdu)
{
}

static inline void
dp_update_cfr_dbg_stats(struct dp_pdev *pdev,
			struct hal_rx_ppdu_info *ppdu_info)
{
}

static inline QDF_STATUS
dp_bb_captured_chan_status(struct dp_pdev *pdev,
			   struct hal_rx_ppdu_info *ppdu_info)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline bool
dp_cfr_rcc_mode_status(struct dp_pdev *pdev)
{
	return false;
}
#endif

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
	struct cdp_rx_indication_ppdu *cdp_rx_ppdu;

	/*
	 * Do not allocate if fcs error,
	 * ast idx invalid / fctl invalid
	 *
	 * In CFR RCC mode - PPDU status TLVs of error pkts are also needed
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

	/* need not generate wdi event when mcopy, cfr rcc mode and
	 * enhanced stats are not enabled
	 */
	if (!pdev->mcopy_mode && !pdev->enhanced_stats_en &&
	    !dp_cfr_rcc_mode_status(pdev))
		return;

	if (dp_cfr_rcc_mode_status(pdev))
		dp_update_cfr_dbg_stats(pdev, ppdu_info);

	if (!ppdu_info->rx_status.frame_control_info_valid ||
	    (ppdu_info->rx_status.ast_index == HAL_AST_IDX_INVALID)) {
		if (!(pdev->mcopy_mode ||
		      (dp_bb_captured_chan_status(pdev, ppdu_info) ==
		       QDF_STATUS_SUCCESS)))
			return;
	}

	ppdu_nbuf = qdf_nbuf_alloc(soc->osdev,
				   sizeof(struct cdp_rx_indication_ppdu),
				   0, 0, FALSE);
	if (ppdu_nbuf) {
		cdp_rx_ppdu = (struct cdp_rx_indication_ppdu *)ppdu_nbuf->data;

		dp_rx_mon_populate_cfr_info(pdev, ppdu_info, cdp_rx_ppdu);
		dp_rx_populate_cdp_indication_ppdu(pdev,
						   ppdu_info, cdp_rx_ppdu);
		if (!qdf_nbuf_put_tail(ppdu_nbuf,
				       sizeof(struct cdp_rx_indication_ppdu)))
			return;

		dp_rx_stats_update(pdev, cdp_rx_ppdu);

		if (cdp_rx_ppdu->peer_id != HTT_INVALID_PEER) {
			dp_wdi_event_handler(WDI_EVENT_RX_PPDU_DESC,
					     soc, ppdu_nbuf,
					     cdp_rx_ppdu->peer_id,
					     WDI_NO_VAL, pdev->pdev_id);
		} else if (pdev->mcopy_mode || dp_cfr_rcc_mode_status(pdev)) {
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
* @pdev_id: mac_id/pdev_id correspondinggly for MCL and WIN
*
* Return: none
*/
static inline void
dp_rx_process_peer_based_pktlog(struct dp_soc *soc,
				struct hal_rx_ppdu_info *ppdu_info,
				qdf_nbuf_t status_nbuf, uint32_t pdev_id)
{
	struct dp_peer *peer;
	struct dp_ast_entry *ast_entry;
	uint32_t ast_index;

	ast_index = ppdu_info->rx_status.ast_index;
	if (ast_index < wlan_cfg_get_max_ast_idx(soc->wlan_cfg_ctx)) {
		ast_entry = soc->ast_table[ast_index];
		if (ast_entry) {
			peer = dp_peer_get_ref_by_id(soc, ast_entry->peer_id,
						     DP_MOD_ID_RX_PPDU_STATS);
			if (peer) {
				if ((peer->peer_id != HTT_INVALID_PEER) &&
				    (peer->peer_based_pktlog_filter)) {
					dp_wdi_event_handler(
							WDI_EVENT_RX_DESC, soc,
							status_nbuf,
							peer->peer_id,
							WDI_NO_VAL, pdev_id);
				}
				dp_peer_unref_delete(peer,
						     DP_MOD_ID_RX_PPDU_STATS);
			}
		}
	}
}

#if defined(HTT_UL_OFDMA_USER_INFO_V0_W0_VALID_M)
static inline void
dp_rx_ul_ofdma_ru_size_to_width(
	uint32_t ru_size,
	uint32_t *ru_width)
{
	uint32_t width;

	width = 0;
	switch (ru_size) {
	case HTT_UL_OFDMA_V0_RU_SIZE_RU_26:
		width = 1;
		break;
	case HTT_UL_OFDMA_V0_RU_SIZE_RU_52:
		width = 2;
		break;
	case HTT_UL_OFDMA_V0_RU_SIZE_RU_106:
		width = 4;
		break;
	case HTT_UL_OFDMA_V0_RU_SIZE_RU_242:
		width = 9;
		break;
	case HTT_UL_OFDMA_V0_RU_SIZE_RU_484:
		width = 18;
		break;
	case HTT_UL_OFDMA_V0_RU_SIZE_RU_996:
		width = 37;
		break;
	case HTT_UL_OFDMA_V0_RU_SIZE_RU_996x2:
		width = 74;
		break;
	default:
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "RU size to width convert err");
		break;
	}
	*ru_width = width;
}

static inline void
dp_rx_mon_handle_mu_ul_info(struct hal_rx_ppdu_info *ppdu_info)
{
	struct mon_rx_user_status *mon_rx_user_status;
	uint32_t num_users;
	uint32_t i;
	uint32_t mu_ul_user_v0_word0;
	uint32_t mu_ul_user_v0_word1;
	uint32_t ru_width;
	uint32_t ru_size;

	if (!(ppdu_info->rx_status.reception_type == HAL_RX_TYPE_MU_OFDMA ||
	      ppdu_info->rx_status.reception_type == HAL_RX_TYPE_MU_MIMO))
		return;

	num_users = ppdu_info->com_info.num_users;
	if (num_users > HAL_MAX_UL_MU_USERS)
		num_users = HAL_MAX_UL_MU_USERS;
	for (i = 0; i < num_users; i++) {
		mon_rx_user_status = &ppdu_info->rx_user_status[i];
		mu_ul_user_v0_word0 =
			mon_rx_user_status->mu_ul_user_v0_word0;
		mu_ul_user_v0_word1 =
			mon_rx_user_status->mu_ul_user_v0_word1;

		if (HTT_UL_OFDMA_USER_INFO_V0_W0_VALID_GET(
			mu_ul_user_v0_word0) &&
			!HTT_UL_OFDMA_USER_INFO_V0_W0_VER_GET(
			mu_ul_user_v0_word0)) {
			mon_rx_user_status->mcs =
				HTT_UL_OFDMA_USER_INFO_V0_W1_MCS_GET(
				mu_ul_user_v0_word1);
			mon_rx_user_status->nss =
				HTT_UL_OFDMA_USER_INFO_V0_W1_NSS_GET(
				mu_ul_user_v0_word1) + 1;

			mon_rx_user_status->mu_ul_info_valid = 1;
			mon_rx_user_status->ofdma_ru_start_index =
				HTT_UL_OFDMA_USER_INFO_V0_W1_RU_START_GET(
				mu_ul_user_v0_word1);

			ru_size =
				HTT_UL_OFDMA_USER_INFO_V0_W1_RU_SIZE_GET(
				mu_ul_user_v0_word1);
			dp_rx_ul_ofdma_ru_size_to_width(ru_size, &ru_width);
			mon_rx_user_status->ofdma_ru_width = ru_width;
			mon_rx_user_status->ofdma_ru_size = ru_size;
		}
	}
}
#else
static inline void
dp_rx_mon_handle_mu_ul_info(struct hal_rx_ppdu_info *ppdu_info)
{
}
#endif

/**
 * dp_rx_mon_status_process_tlv() - Process status TLV in status
 *	buffer on Rx status Queue posted by status SRNG processing.
 * @soc: core txrx main context
 * @int_ctx: interrupt context
 * @mac_id: mac_id which is one of 3 mac_ids _ring
 * @quota: amount of work which can be done
 *
 * Return: none
 */
static inline void
dp_rx_mon_status_process_tlv(struct dp_soc *soc, struct dp_intr *int_ctx,
			     uint32_t mac_id, uint32_t quota)
{
	struct dp_pdev *pdev = dp_get_pdev_for_lmac_id(soc, mac_id);
	struct hal_rx_ppdu_info *ppdu_info;
	qdf_nbuf_t status_nbuf;
	uint8_t *rx_tlv;
	uint8_t *rx_tlv_start;
	uint32_t tlv_status = HAL_TLV_STATUS_BUF_DONE;
	QDF_STATUS enh_log_status = QDF_STATUS_SUCCESS;
	struct cdp_pdev_mon_stats *rx_mon_stats;
	int smart_mesh_status;
	enum WDI_EVENT pktlog_mode = WDI_NO_VAL;
	bool nbuf_used;
	uint32_t rx_enh_capture_mode;

	if (!pdev) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "pdev is null for mac_id = %d", mac_id);
		return;
	}

	ppdu_info = &pdev->ppdu_info;
	rx_mon_stats = &pdev->rx_mon_stats;

	if (pdev->mon_ppdu_status != DP_PPDU_STATUS_START)
		return;

	rx_enh_capture_mode = pdev->rx_enh_capture_mode;

	while (!qdf_nbuf_is_queue_empty(&pdev->rx_status_q)) {

		status_nbuf = qdf_nbuf_queue_remove(&pdev->rx_status_q);

		rx_tlv = qdf_nbuf_data(status_nbuf);
		rx_tlv_start = rx_tlv;
		nbuf_used = false;

		if ((pdev->monitor_vdev) || (pdev->enhanced_stats_en) ||
		    (pdev->mcopy_mode) || (dp_cfr_rcc_mode_status(pdev)) ||
		    (rx_enh_capture_mode != CDP_RX_ENH_CAPTURE_DISABLED)) {
			do {
				tlv_status = hal_rx_status_get_tlv_info(rx_tlv,
						ppdu_info, pdev->soc->hal_soc,
						status_nbuf);

				dp_rx_mon_update_dbg_ppdu_stats(ppdu_info,
								rx_mon_stats);

				dp_rx_mon_enh_capture_process(pdev, tlv_status,
					status_nbuf, ppdu_info,
					&nbuf_used);

				dp_rx_mcopy_process_ppdu_info(pdev,
							      ppdu_info,
							      tlv_status);

				rx_tlv = hal_rx_status_get_next_tlv(rx_tlv);

				if ((rx_tlv - rx_tlv_start) >=
					RX_MON_STATUS_BUF_SIZE)
					break;

			} while ((tlv_status == HAL_TLV_STATUS_PPDU_NOT_DONE) ||
				 (tlv_status == HAL_TLV_STATUS_HEADER) ||
				 (tlv_status == HAL_TLV_STATUS_MPDU_END) ||
				 (tlv_status == HAL_TLV_STATUS_MSDU_END));
		}
		if (pdev->dp_peer_based_pktlog) {
			dp_rx_process_peer_based_pktlog(soc, ppdu_info,
							status_nbuf,
							pdev->pdev_id);
		} else {
			if (pdev->rx_pktlog_mode == DP_RX_PKTLOG_FULL)
				pktlog_mode = WDI_EVENT_RX_DESC;
			else if (pdev->rx_pktlog_mode == DP_RX_PKTLOG_LITE)
				pktlog_mode = WDI_EVENT_LITE_RX;

			if (pktlog_mode != WDI_NO_VAL)
				dp_wdi_event_handler(pktlog_mode, soc,
						     status_nbuf,
						     HTT_INVALID_PEER,
						     WDI_NO_VAL, pdev->pdev_id);
		}

		/* smart monitor vap and m_copy cannot co-exist */
		if (ppdu_info->rx_status.monitor_direct_used && pdev->neighbour_peers_added
		    && pdev->monitor_vdev) {
			smart_mesh_status = dp_rx_handle_smart_mesh_mode(soc,
						pdev, ppdu_info, status_nbuf);
			if (smart_mesh_status)
				qdf_nbuf_free(status_nbuf);
		} else if (qdf_unlikely(pdev->mcopy_mode)) {
			dp_rx_process_mcopy_mode(soc, pdev,
						 ppdu_info, tlv_status,
						 status_nbuf);
		} else if (rx_enh_capture_mode != CDP_RX_ENH_CAPTURE_DISABLED) {
			if (!nbuf_used)
				qdf_nbuf_free(status_nbuf);

			if (tlv_status == HAL_TLV_STATUS_PPDU_DONE)
				enh_log_status =
				dp_rx_handle_enh_capture(soc,
							 pdev, ppdu_info);
		} else {
			qdf_nbuf_free(status_nbuf);
		}

		if (tlv_status == HAL_TLV_STATUS_PPDU_NON_STD_DONE) {
			dp_rx_mon_deliver_non_std(soc, mac_id);
		} else if (tlv_status == HAL_TLV_STATUS_PPDU_DONE) {
			rx_mon_stats->status_ppdu_done++;
			dp_rx_mon_handle_mu_ul_info(ppdu_info);

			if (pdev->tx_capture_enabled
			    != CDP_TX_ENH_CAPTURE_DISABLED)
				dp_send_ack_frame_to_stack(soc, pdev,
							   ppdu_info);

			if (pdev->enhanced_stats_en ||
			    pdev->mcopy_mode || pdev->neighbour_peers_added)
				dp_rx_handle_ppdu_stats(soc, pdev, ppdu_info);
			else if (dp_cfr_rcc_mode_status(pdev))
				dp_rx_handle_cfr(soc, pdev, ppdu_info);

			pdev->mon_ppdu_status = DP_PPDU_STATUS_DONE;

			/*
			* if chan_num is not fetched correctly from ppdu RX TLV,
			 * get it from pdev saved.
			 */
			if (qdf_unlikely(pdev->ppdu_info.rx_status.chan_num == 0))
				pdev->ppdu_info.rx_status.chan_num = pdev->mon_chan_num;
			/*
			 * if chan_freq is not fetched correctly from ppdu RX TLV,
			 * get it from pdev saved.
			 */
			if (qdf_unlikely(pdev->ppdu_info.rx_status.chan_freq == 0)) {
				pdev->ppdu_info.rx_status.chan_freq =
					pdev->mon_chan_freq;
			}

			if (!soc->full_mon_mode)
				dp_rx_mon_dest_process(soc, int_ctx, mac_id,
						       quota);

			pdev->mon_ppdu_status = DP_PPDU_STATUS_START;
		}
	}
	return;
}

/*
 * dp_rx_nbuf_prepare() - prepare RX nbuf
 * @soc: core txrx main context
 * @pdev: core txrx pdev context
 *
 * This function alloc & map nbuf for RX dma usage, retry it if failed
 * until retry times reaches max threshold or succeeded.
 *
 * Return: qdf_nbuf_t pointer if succeeded, NULL if failed.
 */
static inline qdf_nbuf_t
dp_rx_nbuf_prepare(struct dp_soc *soc, struct dp_pdev *pdev)
{
	uint8_t *buf;
	int32_t nbuf_retry_count;
	QDF_STATUS ret;
	qdf_nbuf_t nbuf = NULL;

	for (nbuf_retry_count = 0; nbuf_retry_count <
		QDF_NBUF_ALLOC_MAP_RETRY_THRESHOLD;
			nbuf_retry_count++) {
		/* Allocate a new skb using alloc_skb */
		nbuf = qdf_nbuf_alloc_no_recycler(RX_MON_STATUS_BUF_SIZE,
						  RX_MON_STATUS_BUF_RESERVATION,
						  RX_DATA_BUFFER_ALIGNMENT);

		if (!nbuf) {
			DP_STATS_INC(pdev, replenish.nbuf_alloc_fail, 1);
			continue;
		}

		buf = qdf_nbuf_data(nbuf);

		memset(buf, 0, RX_MON_STATUS_BUF_SIZE);

		ret = qdf_nbuf_map_nbytes_single(soc->osdev, nbuf,
						 QDF_DMA_FROM_DEVICE,
						 RX_MON_STATUS_BUF_SIZE);

		/* nbuf map failed */
		if (qdf_unlikely(QDF_IS_STATUS_ERROR(ret))) {
			qdf_nbuf_free(nbuf);
			DP_STATS_INC(pdev, replenish.map_err, 1);
			continue;
		}
		/* qdf_nbuf alloc and map succeeded */
		break;
	}

	/* qdf_nbuf still alloc or map failed */
	if (qdf_unlikely(nbuf_retry_count >=
			QDF_NBUF_ALLOC_MAP_RETRY_THRESHOLD))
		return NULL;

	return nbuf;
}

/*
 * dp_rx_mon_status_srng_process() - Process monitor status ring
 *	post the status ring buffer to Rx status Queue for later
 *	processing when status ring is filled with status TLV.
 *	Allocate a new buffer to status ring if the filled buffer
 *	is posted.
 * @soc: core txrx main context
 * @int_ctx: interrupt context
 * @mac_id: mac_id which is one of 3 mac_ids
 * @quota: No. of ring entry that can be serviced in one shot.

 * Return: uint32_t: No. of ring entry that is processed.
 */
static inline uint32_t
dp_rx_mon_status_srng_process(struct dp_soc *soc, struct dp_intr *int_ctx,
			      uint32_t mac_id, uint32_t quota)
{
	struct dp_pdev *pdev = dp_get_pdev_for_lmac_id(soc, mac_id);
	hal_soc_handle_t hal_soc;
	void *mon_status_srng;
	void *rxdma_mon_status_ring_entry;
	QDF_STATUS status;
	enum dp_mon_reap_status reap_status;
	uint32_t work_done = 0;

	if (!pdev) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "pdev is null for mac_id = %d", mac_id);
		return work_done;
	}

	mon_status_srng = soc->rxdma_mon_status_ring[mac_id].hal_srng;

	qdf_assert(mon_status_srng);
	if (!mon_status_srng || !hal_srng_initialized(mon_status_srng)) {

		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s %d : HAL Monitor Status Ring Init Failed -- %pK",
			__func__, __LINE__, mon_status_srng);
		return work_done;
	}

	hal_soc = soc->hal_soc;

	qdf_assert(hal_soc);

	if (qdf_unlikely(dp_srng_access_start(int_ctx, soc, mon_status_srng)))
		goto done;

	/* mon_status_ring_desc => WBM_BUFFER_RING STRUCT =>
	 * BUFFER_ADDR_INFO STRUCT
	 */
	while (qdf_likely((rxdma_mon_status_ring_entry =
		hal_srng_src_peek_n_get_next(hal_soc, mon_status_srng))
			&& quota--)) {
		uint32_t rx_buf_cookie;
		qdf_nbuf_t status_nbuf;
		struct dp_rx_desc *rx_desc;
		uint8_t *status_buf;
		qdf_dma_addr_t paddr;
		uint64_t buf_addr;
		struct rx_desc_pool *rx_desc_pool;

		rx_desc_pool = &soc->rx_desc_status[mac_id];
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
				dp_info_rl("tlv tag status error hp:%u, tp:%u",
					   hp, tp);

				/* RxDMA status done bit might not be set even
				 * though tp is moved by HW.
				 */

				/* If done status is missing:
				 * 1. As per MAC team's suggestion,
				 *    when HP + 1 entry is peeked and if DMA
				 *    is not done and if HP + 2 entry's DMA done
				 *    is set. skip HP + 1 entry and
				 *    start processing in next interrupt.
				 * 2. If HP + 2 entry's DMA done is not set,
				 *    poll onto HP + 1 entry DMA done to be set.
				 *    Check status for same buffer for next time
				 *    dp_rx_mon_status_srng_process
				 */
				reap_status = dp_rx_mon_handle_status_buf_done(pdev,
									mon_status_srng);
				if (reap_status == DP_MON_STATUS_NO_DMA)
					continue;
				else if (reap_status == DP_MON_STATUS_REPLENISH) {
					qdf_nbuf_unmap_nbytes_single(
							soc->osdev, status_nbuf,
							QDF_DMA_FROM_DEVICE,
							rx_desc_pool->buf_size);
					qdf_nbuf_free(status_nbuf);
					goto buf_replenish;
				}
			}
			qdf_nbuf_set_pktlen(status_nbuf,
					    RX_MON_STATUS_BUF_SIZE);

			qdf_nbuf_unmap_nbytes_single(soc->osdev, status_nbuf,
						     QDF_DMA_FROM_DEVICE,
						     rx_desc_pool->buf_size);

			/* Put the status_nbuf to queue */
			qdf_nbuf_queue_add(&pdev->rx_status_q, status_nbuf);

		} else {
			union dp_rx_desc_list_elem_t *desc_list = NULL;
			union dp_rx_desc_list_elem_t *tail = NULL;
			uint32_t num_alloc_desc;

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

buf_replenish:
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

			dp_info_rl("fail to allocate or map qdf_nbuf");
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

	dp_srng_access_end(int_ctx, soc, mon_status_srng);

	return work_done;

}

uint32_t
dp_rx_mon_status_process(struct dp_soc *soc, struct dp_intr *int_ctx,
			 uint32_t mac_id, uint32_t quota)
{
	uint32_t work_done;

	work_done = dp_rx_mon_status_srng_process(soc, int_ctx, mac_id, quota);
	quota -= work_done;
	dp_rx_mon_status_process_tlv(soc, int_ctx, mac_id, quota);

	return work_done;
}

#ifndef DISABLE_MON_CONFIG
uint32_t
dp_mon_process(struct dp_soc *soc, struct dp_intr *int_ctx,
	       uint32_t mac_id, uint32_t quota)
{
	if (qdf_unlikely(soc->full_mon_mode))
		return dp_rx_mon_process(soc, int_ctx, mac_id, quota);

	return dp_rx_mon_status_process(soc, int_ctx, mac_id, quota);
}
#else
uint32_t
dp_mon_process(struct dp_soc *soc, struct dp_intr *int_ctx,
	       uint32_t mac_id, uint32_t quota)
{
	return 0;
}
#endif

QDF_STATUS
dp_rx_pdev_mon_status_buffers_alloc(struct dp_pdev *pdev, uint32_t mac_id)
{
	uint8_t pdev_id = pdev->pdev_id;
	struct dp_soc *soc = pdev->soc;
	struct dp_srng *mon_status_ring;
	uint32_t num_entries;
	struct rx_desc_pool *rx_desc_pool;
	struct wlan_cfg_dp_soc_ctxt *soc_cfg_ctx;
	union dp_rx_desc_list_elem_t *desc_list = NULL;
	union dp_rx_desc_list_elem_t *tail = NULL;

	soc_cfg_ctx = soc->wlan_cfg_ctx;
	mon_status_ring = &soc->rxdma_mon_status_ring[mac_id];

	num_entries = mon_status_ring->num_entries;

	rx_desc_pool = &soc->rx_desc_status[mac_id];

	dp_debug("Mon RX Desc Pool[%d] entries=%u",
		 pdev_id, num_entries);

	return dp_rx_mon_status_buffers_replenish(soc, mac_id, mon_status_ring,
						  rx_desc_pool, num_entries,
						  &desc_list, &tail,
						  HAL_RX_BUF_RBM_SW3_BM);
}

QDF_STATUS
dp_rx_pdev_mon_status_desc_pool_alloc(struct dp_pdev *pdev, uint32_t mac_id)
{
	uint8_t pdev_id = pdev->pdev_id;
	struct dp_soc *soc = pdev->soc;
	struct dp_srng *mon_status_ring;
	uint32_t num_entries;
	struct rx_desc_pool *rx_desc_pool;
	struct wlan_cfg_dp_soc_ctxt *soc_cfg_ctx;

	soc_cfg_ctx = soc->wlan_cfg_ctx;
	mon_status_ring = &soc->rxdma_mon_status_ring[mac_id];

	num_entries = mon_status_ring->num_entries;

	rx_desc_pool = &soc->rx_desc_status[mac_id];

	dp_debug("Mon RX Desc Pool[%d] entries=%u", pdev_id, num_entries);

	rx_desc_pool->desc_type = DP_RX_DESC_STATUS_TYPE;
	return dp_rx_desc_pool_alloc(soc, num_entries + 1, rx_desc_pool);
}

void
dp_rx_pdev_mon_status_desc_pool_init(struct dp_pdev *pdev, uint32_t mac_id)
{
	uint32_t i;
	uint8_t pdev_id = pdev->pdev_id;
	struct dp_soc *soc = pdev->soc;
	struct dp_srng *mon_status_ring;
	uint32_t num_entries;
	struct rx_desc_pool *rx_desc_pool;
	struct wlan_cfg_dp_soc_ctxt *soc_cfg_ctx;

	soc_cfg_ctx = soc->wlan_cfg_ctx;
	mon_status_ring = &soc->rxdma_mon_status_ring[mac_id];

	num_entries = mon_status_ring->num_entries;

	rx_desc_pool = &soc->rx_desc_status[mac_id];

	dp_debug("Mon RX Desc status Pool[%d] init entries=%u",
		 pdev_id, num_entries);

	rx_desc_pool->owner = HAL_RX_BUF_RBM_SW3_BM;
	rx_desc_pool->buf_size = RX_MON_STATUS_BUF_SIZE;
	rx_desc_pool->buf_alignment = RX_DATA_BUFFER_ALIGNMENT;
	/* Disable frag processing flag */
	dp_rx_enable_mon_dest_frag(rx_desc_pool, false);

	dp_rx_desc_pool_init(soc, mac_id, num_entries + 1, rx_desc_pool);

	qdf_nbuf_queue_init(&pdev->rx_status_q);

	pdev->mon_ppdu_status = DP_PPDU_STATUS_START;

	qdf_mem_zero(&pdev->ppdu_info, sizeof(pdev->ppdu_info));

	/*
	 * Set last_ppdu_id to HAL_INVALID_PPDU_ID in order to avoid ppdu_id
	 * match with '0' ppdu_id from monitor status ring
	 */
	pdev->ppdu_info.com_info.last_ppdu_id = HAL_INVALID_PPDU_ID;

	qdf_mem_zero(&pdev->rx_mon_stats, sizeof(pdev->rx_mon_stats));

	dp_rx_mon_init_dbg_ppdu_stats(&pdev->ppdu_info,
				      &pdev->rx_mon_stats);

	for (i = 0; i < MAX_MU_USERS; i++) {
		qdf_nbuf_queue_init(&pdev->mpdu_q[i]);
		pdev->is_mpdu_hdr[i] = true;
	}

	qdf_mem_zero(pdev->msdu_list, sizeof(pdev->msdu_list[MAX_MU_USERS]));

	pdev->rx_enh_capture_mode = CDP_RX_ENH_CAPTURE_DISABLED;
}

void
dp_rx_pdev_mon_status_desc_pool_deinit(struct dp_pdev *pdev, uint32_t mac_id) {
	uint8_t pdev_id = pdev->pdev_id;
	struct dp_soc *soc = pdev->soc;
	struct rx_desc_pool *rx_desc_pool;

	rx_desc_pool = &soc->rx_desc_status[mac_id];

	dp_debug("Mon RX Desc status Pool[%d] deinit", pdev_id);

	dp_rx_desc_pool_deinit(soc, rx_desc_pool);
}

void
dp_rx_pdev_mon_status_desc_pool_free(struct dp_pdev *pdev, uint32_t mac_id) {
	uint8_t pdev_id = pdev->pdev_id;
	struct dp_soc *soc = pdev->soc;
	struct rx_desc_pool *rx_desc_pool;

	rx_desc_pool = &soc->rx_desc_status[mac_id];

	dp_debug("Mon RX Status Desc Pool Free pdev[%d]", pdev_id);

	dp_rx_desc_pool_free(soc, rx_desc_pool);
}

void
dp_rx_pdev_mon_status_buffers_free(struct dp_pdev *pdev, uint32_t mac_id)
{
	uint8_t pdev_id = pdev->pdev_id;
	struct dp_soc *soc = pdev->soc;
	struct rx_desc_pool *rx_desc_pool;

	rx_desc_pool = &soc->rx_desc_status[mac_id];

	dp_debug("Mon RX Status Desc Pool Free pdev[%d]", pdev_id);

	dp_rx_desc_nbuf_free(soc, rx_desc_pool);
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
	struct dp_pdev *dp_pdev = dp_get_pdev_for_lmac_id(dp_soc, mac_id);

	if (!dp_pdev) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "pdev is null for mac_id = %d", mac_id);
		return QDF_STATUS_E_FAILURE;
	}

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

	while (count <= num_req_buffers) {
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
		rxdma_ring_entry = hal_srng_src_get_cur_hp_n_move_next(
						dp_soc->hal_soc,
						rxdma_srng);

		if (qdf_unlikely(!rxdma_ring_entry)) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
					"[%s][%d] rxdma_ring_entry is NULL, count - %d",
					__func__, __LINE__, count);
			qdf_nbuf_unmap_nbytes_single(dp_soc->osdev, rx_netbuf,
						     QDF_DMA_FROM_DEVICE,
						     rx_desc_pool->buf_size);
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

#if !defined(DISABLE_MON_CONFIG) && defined(MON_ENABLE_DROP_FOR_MAC)
/**
 * dp_mon_status_srng_drop_for_mac() - Drop the mon status ring packets for
 *  a given mac
 * @pdev: DP pdev
 * @mac_id: mac id
 * @quota: maximum number of ring entries that can be processed
 *
 * Return: Number of ring entries reaped
 */
static uint32_t
dp_mon_status_srng_drop_for_mac(struct dp_pdev *pdev, uint32_t mac_id,
				uint32_t quota)
{
	struct dp_soc *soc = pdev->soc;
	void *mon_status_srng;
	hal_soc_handle_t hal_soc;
	void *ring_desc;
	uint32_t reap_cnt = 0;

	if (qdf_unlikely(!soc || !soc->hal_soc))
		return reap_cnt;

	mon_status_srng = soc->rxdma_mon_status_ring[mac_id].hal_srng;

	if (qdf_unlikely(!mon_status_srng ||
			 !hal_srng_initialized(mon_status_srng)))
		return reap_cnt;

	hal_soc = soc->hal_soc;

	if (qdf_unlikely(hal_srng_access_start(hal_soc, mon_status_srng)))
		return reap_cnt;

	while ((ring_desc =
		hal_srng_src_peek_n_get_next(hal_soc, mon_status_srng)) &&
		reap_cnt < MON_DROP_REAP_LIMIT && quota--) {
		uint64_t buf_addr;

		buf_addr = (HAL_RX_BUFFER_ADDR_31_0_GET(ring_desc) |
		   ((uint64_t)(HAL_RX_BUFFER_ADDR_39_32_GET(ring_desc)) << 32));

		if (qdf_unlikely(!buf_addr)) {
			struct rx_desc_pool *rx_desc_pool;
			qdf_dma_addr_t iova;
			qdf_nbuf_t status_nbuf;
			struct dp_rx_desc *rx_desc;
			union dp_rx_desc_list_elem_t *rx_desc_elem;

			rx_desc_pool = dp_rx_get_mon_desc_pool(soc, mac_id,
							       pdev->pdev_id);

			qdf_spin_lock_bh(&rx_desc_pool->lock);

			if (!rx_desc_pool->freelist) {
				qdf_spin_unlock_bh(&rx_desc_pool->lock);
				break;
			}
			rx_desc_elem = rx_desc_pool->freelist;
			rx_desc_pool->freelist = rx_desc_pool->freelist->next;
			qdf_spin_unlock_bh(&rx_desc_pool->lock);

			rx_desc = &rx_desc_elem->rx_desc;

			status_nbuf = dp_rx_nbuf_prepare(soc, pdev);

			if (qdf_unlikely(!status_nbuf)) {
				union dp_rx_desc_list_elem_t *desc_list = NULL;
				union dp_rx_desc_list_elem_t *tail = NULL;

				dp_info_rl("fail to allocate or map nbuf");
				dp_rx_add_to_free_desc_list(&desc_list, &tail,
							    rx_desc);
				dp_rx_add_desc_list_to_free_list(soc,
								 &desc_list,
								 &tail, mac_id,
								 rx_desc_pool);

				hal_rxdma_buff_addr_info_set(ring_desc, 0, 0,
							 HAL_RX_BUF_RBM_SW3_BM);

				break;
			}

			iova = qdf_nbuf_get_frag_paddr(status_nbuf, 0);

			rx_desc->nbuf = status_nbuf;
			rx_desc->in_use = 1;

			hal_rxdma_buff_addr_info_set(ring_desc, iova,
						     rx_desc->cookie,
						     HAL_RX_BUF_RBM_SW3_BM);
		}

		reap_cnt++;
		hal_srng_src_get_next(hal_soc, mon_status_srng);
	}

	hal_srng_access_end(hal_soc, mon_status_srng);

	return reap_cnt;
}

uint32_t dp_mon_drop_packets_for_mac(struct dp_pdev *pdev, uint32_t mac_id,
				     uint32_t quota)
{
	uint32_t work_done;

	work_done = dp_mon_status_srng_drop_for_mac(pdev, mac_id, quota);
	dp_mon_dest_srng_drop_for_mac(pdev, mac_id);

	return work_done;
}
#else
uint32_t dp_mon_drop_packets_for_mac(struct dp_pdev *pdev, uint32_t mac_id,
				     uint32_t quota)
{
	return 0;
}
#endif
