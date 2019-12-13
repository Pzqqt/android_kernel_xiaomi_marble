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

#include <htt.h>
#include <hal_hw_headers.h>
#include <hal_api.h>
#include "dp_peer.h"
#include "dp_types.h"
#include "dp_internal.h"
#include "dp_rx_mon.h"
#include "htt_stats.h"
#include "htt_ppdu_stats.h"
#include "dp_htt.h"
#include "dp_rx.h"
#include "qdf_mem.h"   /* qdf_mem_malloc,free */
#include "cdp_txrx_cmn_struct.h"

#ifdef FEATURE_PERPKT_INFO
#include "dp_ratetable.h"
#endif

#define HTT_TLV_HDR_LEN HTT_T2H_EXT_STATS_CONF_TLV_HDR_SIZE

#define HTT_HTC_PKT_POOL_INIT_SIZE 64

#define HTT_MSG_BUF_SIZE(msg_bytes) \
	((msg_bytes) + HTC_HEADER_LEN + HTC_HDR_ALIGNMENT_PADDING)

#define HTT_PID_BIT_MASK 0x3

#define DP_EXT_MSG_LENGTH 2048

#define HTT_MGMT_CTRL_TLV_HDR_RESERVERD_LEN 16

#define HTT_SHIFT_UPPER_TIMESTAMP 32
#define HTT_MASK_UPPER_TIMESTAMP 0xFFFFFFFF00000000

/*
 * dp_htt_get_ppdu_sniffer_ampdu_tlv_bitmap() - Get ppdu stats tlv
 * bitmap for sniffer mode
 * @bitmap: received bitmap
 *
 * Return: expected bitmap value, returns zero if doesn't match with
 * either 64-bit Tx window or 256-bit window tlv bitmap
 */
int
dp_htt_get_ppdu_sniffer_ampdu_tlv_bitmap(uint32_t bitmap)
{
	if (bitmap == (HTT_PPDU_SNIFFER_AMPDU_TLV_BITMAP_64))
		return HTT_PPDU_SNIFFER_AMPDU_TLV_BITMAP_64;
	else if (bitmap == (HTT_PPDU_SNIFFER_AMPDU_TLV_BITMAP_256))
		return HTT_PPDU_SNIFFER_AMPDU_TLV_BITMAP_256;

	return 0;
}

#ifdef FEATURE_PERPKT_INFO
/*
 * dp_peer_copy_delay_stats() - copy ppdu stats to peer delayed stats.
 * @peer: Datapath peer handle
 * @ppdu: PPDU Descriptor
 *
 * Return: None
 *
 * on Tx data frame, we may get delayed ba set
 * in htt_ppdu_stats_user_common_tlv. which mean we get Block Ack(BA) after we
 * request Block Ack Request(BAR). Successful msdu is received only after Block
 * Ack. To populate peer stats we need successful msdu(data frame).
 * So we hold the Tx data stats on delayed_ba for stats update.
 */
static inline void
dp_peer_copy_delay_stats(struct dp_peer *peer,
			 struct cdp_tx_completion_ppdu_user *ppdu)
{
	struct dp_pdev *pdev;
	struct dp_vdev *vdev;

	if (peer->last_delayed_ba) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "BA not yet recv for prev delayed ppdu[%d]\n",
			  peer->last_delayed_ba_ppduid);
		vdev = peer->vdev;
		if (vdev) {
			pdev = vdev->pdev;
			pdev->stats.cdp_delayed_ba_not_recev++;
		}
	}

	peer->delayed_ba_ppdu_stats.ltf_size = ppdu->ltf_size;
	peer->delayed_ba_ppdu_stats.stbc = ppdu->stbc;
	peer->delayed_ba_ppdu_stats.he_re = ppdu->he_re;
	peer->delayed_ba_ppdu_stats.txbf = ppdu->txbf;
	peer->delayed_ba_ppdu_stats.bw = ppdu->bw;
	peer->delayed_ba_ppdu_stats.nss = ppdu->nss;
	peer->delayed_ba_ppdu_stats.preamble = ppdu->preamble;
	peer->delayed_ba_ppdu_stats.gi = ppdu->gi;
	peer->delayed_ba_ppdu_stats.dcm = ppdu->dcm;
	peer->delayed_ba_ppdu_stats.ldpc = ppdu->ldpc;
	peer->delayed_ba_ppdu_stats.dcm = ppdu->dcm;
	peer->delayed_ba_ppdu_stats.mpdu_tried_ucast = ppdu->mpdu_tried_ucast;
	peer->delayed_ba_ppdu_stats.mpdu_tried_mcast = ppdu->mpdu_tried_mcast;
	peer->delayed_ba_ppdu_stats.frame_ctrl = ppdu->frame_ctrl;
	peer->delayed_ba_ppdu_stats.qos_ctrl = ppdu->qos_ctrl;
	peer->delayed_ba_ppdu_stats.dcm = ppdu->dcm;

	peer->delayed_ba_ppdu_stats.ru_start = ppdu->ru_start;
	peer->delayed_ba_ppdu_stats.ru_tones = ppdu->ru_tones;
	peer->delayed_ba_ppdu_stats.is_mcast = ppdu->is_mcast;

	peer->delayed_ba_ppdu_stats.user_pos = ppdu->user_pos;
	peer->delayed_ba_ppdu_stats.mu_group_id = ppdu->mu_group_id;

	peer->last_delayed_ba = true;
}

/*
 * dp_peer_copy_stats_to_bar() - copy delayed stats to ppdu stats.
 * @peer: Datapath peer handle
 * @ppdu: PPDU Descriptor
 *
 * Return: None
 *
 * For Tx BAR, PPDU stats TLV include Block Ack info. PPDU info
 * from Tx BAR frame not required to populate peer stats.
 * But we need successful MPDU and MSDU to update previous
 * transmitted Tx data frame. Overwrite ppdu stats with the previous
 * stored ppdu stats.
 */
static void
dp_peer_copy_stats_to_bar(struct dp_peer *peer,
			  struct cdp_tx_completion_ppdu_user *ppdu)
{
	ppdu->ltf_size = peer->delayed_ba_ppdu_stats.ltf_size;
	ppdu->stbc = peer->delayed_ba_ppdu_stats.stbc;
	ppdu->he_re = peer->delayed_ba_ppdu_stats.he_re;
	ppdu->txbf = peer->delayed_ba_ppdu_stats.txbf;
	ppdu->bw = peer->delayed_ba_ppdu_stats.bw;
	ppdu->nss = peer->delayed_ba_ppdu_stats.nss;
	ppdu->preamble = peer->delayed_ba_ppdu_stats.preamble;
	ppdu->gi = peer->delayed_ba_ppdu_stats.gi;
	ppdu->dcm = peer->delayed_ba_ppdu_stats.dcm;
	ppdu->ldpc = peer->delayed_ba_ppdu_stats.ldpc;
	ppdu->dcm = peer->delayed_ba_ppdu_stats.dcm;
	ppdu->mpdu_tried_ucast = peer->delayed_ba_ppdu_stats.mpdu_tried_ucast;
	ppdu->mpdu_tried_mcast = peer->delayed_ba_ppdu_stats.mpdu_tried_mcast;
	ppdu->frame_ctrl = peer->delayed_ba_ppdu_stats.frame_ctrl;
	ppdu->qos_ctrl = peer->delayed_ba_ppdu_stats.qos_ctrl;
	ppdu->dcm = peer->delayed_ba_ppdu_stats.dcm;

	ppdu->ru_start = peer->delayed_ba_ppdu_stats.ru_start;
	ppdu->ru_tones = peer->delayed_ba_ppdu_stats.ru_tones;
	ppdu->is_mcast = peer->delayed_ba_ppdu_stats.is_mcast;

	ppdu->user_pos = peer->delayed_ba_ppdu_stats.user_pos;
	ppdu->mu_group_id = peer->delayed_ba_ppdu_stats.mu_group_id;

	peer->last_delayed_ba = false;
}

/*
 * dp_tx_rate_stats_update() - Update rate per-peer statistics
 * @peer: Datapath peer handle
 * @ppdu: PPDU Descriptor
 *
 * Return: None
 */
static void
dp_tx_rate_stats_update(struct dp_peer *peer,
			struct cdp_tx_completion_ppdu_user *ppdu)
{
	uint32_t ratekbps = 0;
	uint64_t ppdu_tx_rate = 0;
	uint32_t rix;
	uint16_t ratecode = 0;

	if (!peer || !ppdu)
		return;

	ratekbps = dp_getrateindex(ppdu->gi,
				   ppdu->mcs,
				   ppdu->nss,
				   ppdu->preamble,
				   ppdu->bw,
				   &rix,
				   &ratecode);

	DP_STATS_UPD(peer, tx.last_tx_rate, ratekbps);

	if (!ratekbps)
		return;

	/* Calculate goodput in non-training period
	 * In training period, don't do anything as
	 * pending pkt is send as goodput.
	 */
	if ((!peer->bss_peer) && (!ppdu->sa_is_training)) {
		ppdu->sa_goodput = ((ratekbps / CDP_NUM_KB_IN_MB) *
				(CDP_PERCENT_MACRO - ppdu->current_rate_per));
	}
	ppdu->rix = rix;
	ppdu->tx_ratekbps = ratekbps;
	ppdu->tx_ratecode = ratecode;
	peer->stats.tx.avg_tx_rate =
		dp_ath_rate_lpf(peer->stats.tx.avg_tx_rate, ratekbps);
	ppdu_tx_rate = dp_ath_rate_out(peer->stats.tx.avg_tx_rate);
	DP_STATS_UPD(peer, tx.rnd_avg_tx_rate, ppdu_tx_rate);

	if (peer->vdev) {
		/*
		 * In STA mode:
		 *	We get ucast stats as BSS peer stats.
		 *
		 * In AP mode:
		 *	We get mcast stats as BSS peer stats.
		 *	We get ucast stats as assoc peer stats.
		 */
		if (peer->vdev->opmode == wlan_op_mode_ap && peer->bss_peer) {
			peer->vdev->stats.tx.mcast_last_tx_rate = ratekbps;
			peer->vdev->stats.tx.mcast_last_tx_rate_mcs = ppdu->mcs;
		} else {
			peer->vdev->stats.tx.last_tx_rate = ratekbps;
			peer->vdev->stats.tx.last_tx_rate_mcs = ppdu->mcs;
		}
	}
}

/*
 * dp_tx_stats_update() - Update per-peer statistics
 * @pdev: Datapath pdev handle
 * @peer: Datapath peer handle
 * @ppdu: PPDU Descriptor
 * @ack_rssi: RSSI of last ack received
 *
 * Return: None
 */
static void
dp_tx_stats_update(struct dp_pdev *pdev, struct dp_peer *peer,
		   struct cdp_tx_completion_ppdu_user *ppdu,
		   uint32_t ack_rssi)
{
	uint8_t preamble, mcs;
	uint16_t num_msdu;
	uint16_t num_mpdu;
	uint16_t mpdu_tried;
	uint16_t mpdu_failed;

	preamble = ppdu->preamble;
	mcs = ppdu->mcs;
	num_msdu = ppdu->num_msdu;
	num_mpdu = ppdu->mpdu_success;
	mpdu_tried = ppdu->mpdu_tried_ucast + ppdu->mpdu_tried_mcast;
	mpdu_failed = mpdu_tried - num_mpdu;

	/* If the peer statistics are already processed as part of
	 * per-MSDU completion handler, do not process these again in per-PPDU
	 * indications */
	if (pdev->soc->process_tx_status)
		return;

	if (ppdu->completion_status != HTT_PPDU_STATS_USER_STATUS_OK) {
		/*
		 * All failed mpdu will be retried, so incrementing
		 * retries mpdu based on mpdu failed. Even for
		 * ack failure i.e for long retries we get
		 * mpdu failed equal mpdu tried.
		 */
		DP_STATS_INC(peer, tx.retries, mpdu_failed);
		DP_STATS_INC(peer, tx.tx_failed, ppdu->failed_msdus);
		return;
	}

	if (ppdu->is_ppdu_cookie_valid)
		DP_STATS_INC(peer, tx.num_ppdu_cookie_valid, 1);

	if (ppdu->mu_group_id <= MAX_MU_GROUP_ID &&
	    ppdu->ppdu_type != HTT_PPDU_STATS_PPDU_TYPE_SU) {
		if (unlikely(!(ppdu->mu_group_id & (MAX_MU_GROUP_ID - 1))))
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				  "mu_group_id out of bound!!\n");
		else
			DP_STATS_UPD(peer, tx.mu_group_id[ppdu->mu_group_id],
				     (ppdu->user_pos + 1));
	}

	if (ppdu->ppdu_type == HTT_PPDU_STATS_PPDU_TYPE_MU_OFDMA ||
	    ppdu->ppdu_type == HTT_PPDU_STATS_PPDU_TYPE_MU_MIMO_OFDMA) {
		DP_STATS_UPD(peer, tx.ru_tones, ppdu->ru_tones);
		DP_STATS_UPD(peer, tx.ru_start, ppdu->ru_start);
		switch (ppdu->ru_tones) {
		case RU_26:
			DP_STATS_INC(peer, tx.ru_loc[RU_26_INDEX].num_msdu,
				     num_msdu);
			DP_STATS_INC(peer, tx.ru_loc[RU_26_INDEX].num_mpdu,
				     num_mpdu);
			DP_STATS_INC(peer, tx.ru_loc[RU_26_INDEX].mpdu_tried,
				     mpdu_tried);
		break;
		case RU_52:
			DP_STATS_INC(peer, tx.ru_loc[RU_52_INDEX].num_msdu,
				     num_msdu);
			DP_STATS_INC(peer, tx.ru_loc[RU_52_INDEX].num_mpdu,
				     num_mpdu);
			DP_STATS_INC(peer, tx.ru_loc[RU_52_INDEX].mpdu_tried,
				     mpdu_tried);
		break;
		case RU_106:
			DP_STATS_INC(peer, tx.ru_loc[RU_106_INDEX].num_msdu,
				     num_msdu);
			DP_STATS_INC(peer, tx.ru_loc[RU_106_INDEX].num_mpdu,
				     num_mpdu);
			DP_STATS_INC(peer, tx.ru_loc[RU_106_INDEX].mpdu_tried,
				     mpdu_tried);
		break;
		case RU_242:
			DP_STATS_INC(peer, tx.ru_loc[RU_242_INDEX].num_msdu,
				     num_msdu);
			DP_STATS_INC(peer, tx.ru_loc[RU_242_INDEX].num_mpdu,
				     num_mpdu);
			DP_STATS_INC(peer, tx.ru_loc[RU_242_INDEX].mpdu_tried,
				     mpdu_tried);
		break;
		case RU_484:
			DP_STATS_INC(peer, tx.ru_loc[RU_484_INDEX].num_msdu,
				     num_msdu);
			DP_STATS_INC(peer, tx.ru_loc[RU_484_INDEX].num_mpdu,
				     num_mpdu);
			DP_STATS_INC(peer, tx.ru_loc[RU_484_INDEX].mpdu_tried,
				     mpdu_tried);
		break;
		case RU_996:
			DP_STATS_INC(peer, tx.ru_loc[RU_996_INDEX].num_msdu,
				     num_msdu);
			DP_STATS_INC(peer, tx.ru_loc[RU_996_INDEX].num_mpdu,
				     num_mpdu);
			DP_STATS_INC(peer, tx.ru_loc[RU_996_INDEX].mpdu_tried,
				     mpdu_tried);
		break;
		}
	}

	/*
	 * All failed mpdu will be retried, so incrementing
	 * retries mpdu based on mpdu failed. Even for
	 * ack failure i.e for long retries we get
	 * mpdu failed equal mpdu tried.
	 */
	DP_STATS_INC(peer, tx.retries, mpdu_failed);
	DP_STATS_INC(peer, tx.tx_failed, ppdu->failed_msdus);

	DP_STATS_INC(peer, tx.transmit_type[ppdu->ppdu_type].num_msdu,
		     num_msdu);
	DP_STATS_INC(peer, tx.transmit_type[ppdu->ppdu_type].num_mpdu,
		     num_mpdu);
	DP_STATS_INC(peer, tx.transmit_type[ppdu->ppdu_type].mpdu_tried,
		     mpdu_tried);

	DP_STATS_INC_PKT(peer, tx.comp_pkt,
			num_msdu, (ppdu->success_bytes +
				ppdu->retry_bytes + ppdu->failed_bytes));
	DP_STATS_UPD(peer, tx.tx_rate, ppdu->tx_rate);
	DP_STATS_INC(peer, tx.sgi_count[ppdu->gi], num_msdu);
	DP_STATS_INC(peer, tx.bw[ppdu->bw], num_msdu);
	DP_STATS_INC(peer, tx.nss[ppdu->nss], num_msdu);
	if (ppdu->tid < CDP_DATA_TID_MAX)
		DP_STATS_INC(peer, tx.wme_ac_type[TID_TO_WME_AC(ppdu->tid)],
			     num_msdu);
	DP_STATS_INCC(peer, tx.stbc, num_msdu, ppdu->stbc);
	DP_STATS_INCC(peer, tx.ldpc, num_msdu, ppdu->ldpc);
	if (!(ppdu->is_mcast) && ppdu->ack_rssi_valid)
		DP_STATS_UPD(peer, tx.last_ack_rssi, ack_rssi);

	DP_STATS_INCC(peer,
			tx.pkt_type[preamble].mcs_count[MAX_MCS-1], num_msdu,
			((mcs >= MAX_MCS_11A) && (preamble == DOT11_A)));
	DP_STATS_INCC(peer,
			tx.pkt_type[preamble].mcs_count[mcs], num_msdu,
			((mcs < MAX_MCS_11A) && (preamble == DOT11_A)));
	DP_STATS_INCC(peer,
			tx.pkt_type[preamble].mcs_count[MAX_MCS-1], num_msdu,
			((mcs >= MAX_MCS_11B) && (preamble == DOT11_B)));
	DP_STATS_INCC(peer,
			tx.pkt_type[preamble].mcs_count[mcs], num_msdu,
			((mcs < (MAX_MCS_11B)) && (preamble == DOT11_B)));
	DP_STATS_INCC(peer,
			tx.pkt_type[preamble].mcs_count[MAX_MCS-1], num_msdu,
			((mcs >= MAX_MCS_11A) && (preamble == DOT11_N)));
	DP_STATS_INCC(peer,
			tx.pkt_type[preamble].mcs_count[mcs], num_msdu,
			((mcs < MAX_MCS_11A) && (preamble == DOT11_N)));
	DP_STATS_INCC(peer,
			tx.pkt_type[preamble].mcs_count[MAX_MCS-1], num_msdu,
			((mcs >= MAX_MCS_11AC) && (preamble == DOT11_AC)));
	DP_STATS_INCC(peer,
			tx.pkt_type[preamble].mcs_count[mcs], num_msdu,
			((mcs < MAX_MCS_11AC) && (preamble == DOT11_AC)));
	DP_STATS_INCC(peer,
			tx.pkt_type[preamble].mcs_count[MAX_MCS-1], num_msdu,
			((mcs >= (MAX_MCS - 1)) && (preamble == DOT11_AX)));
	DP_STATS_INCC(peer,
			tx.pkt_type[preamble].mcs_count[mcs], num_msdu,
			((mcs < (MAX_MCS - 1)) && (preamble == DOT11_AX)));
	DP_STATS_INCC(peer, tx.ampdu_cnt, num_msdu, ppdu->is_ampdu);
	DP_STATS_INCC(peer, tx.non_ampdu_cnt, num_msdu, !(ppdu->is_ampdu));

	dp_peer_stats_notify(pdev, peer);

#if defined(FEATURE_PERPKT_INFO) && WDI_EVENT_ENABLE
	dp_wdi_event_handler(WDI_EVENT_UPDATE_DP_STATS, pdev->soc,
			     &peer->stats, ppdu->peer_id,
			     UPDATE_PEER_STATS, pdev->pdev_id);
#endif
}
#endif

#ifdef WLAN_TX_PKT_CAPTURE_ENH
#include "dp_tx_capture.h"
#else
static inline void
dp_process_ppdu_stats_update_failed_bitmap(struct dp_pdev *pdev,
					   void *data,
					   uint32_t ppdu_id,
					   uint32_t size)
{
}
#endif

/*
 * htt_htc_pkt_alloc() - Allocate HTC packet buffer
 * @htt_soc:	HTT SOC handle
 *
 * Return: Pointer to htc packet buffer
 */
static struct dp_htt_htc_pkt *
htt_htc_pkt_alloc(struct htt_soc *soc)
{
	struct dp_htt_htc_pkt_union *pkt = NULL;

	HTT_TX_MUTEX_ACQUIRE(&soc->htt_tx_mutex);
	if (soc->htt_htc_pkt_freelist) {
		pkt = soc->htt_htc_pkt_freelist;
		soc->htt_htc_pkt_freelist = soc->htt_htc_pkt_freelist->u.next;
	}
	HTT_TX_MUTEX_RELEASE(&soc->htt_tx_mutex);

	if (!pkt)
		pkt = qdf_mem_malloc(sizeof(*pkt));
	return &pkt->u.pkt; /* not actually a dereference */
}

/*
 * htt_htc_pkt_free() - Free HTC packet buffer
 * @htt_soc:	HTT SOC handle
 */
static void
htt_htc_pkt_free(struct htt_soc *soc, struct dp_htt_htc_pkt *pkt)
{
	struct dp_htt_htc_pkt_union *u_pkt =
		(struct dp_htt_htc_pkt_union *)pkt;

	HTT_TX_MUTEX_ACQUIRE(&soc->htt_tx_mutex);
	u_pkt->u.next = soc->htt_htc_pkt_freelist;
	soc->htt_htc_pkt_freelist = u_pkt;
	HTT_TX_MUTEX_RELEASE(&soc->htt_tx_mutex);
}

/*
 * htt_htc_pkt_pool_free() - Free HTC packet pool
 * @htt_soc:	HTT SOC handle
 */
static void
htt_htc_pkt_pool_free(struct htt_soc *soc)
{
	struct dp_htt_htc_pkt_union *pkt, *next;
	pkt = soc->htt_htc_pkt_freelist;
	while (pkt) {
		next = pkt->u.next;
		qdf_mem_free(pkt);
		pkt = next;
	}
	soc->htt_htc_pkt_freelist = NULL;
}

/*
 * htt_htc_misc_pkt_list_trim() - trim misc list
 * @htt_soc: HTT SOC handle
 * @level: max no. of pkts in list
 */
static void
htt_htc_misc_pkt_list_trim(struct htt_soc *soc, int level)
{
	struct dp_htt_htc_pkt_union *pkt, *next, *prev = NULL;
	int i = 0;
	qdf_nbuf_t netbuf;

	HTT_TX_MUTEX_ACQUIRE(&soc->htt_tx_mutex);
	pkt = soc->htt_htc_pkt_misclist;
	while (pkt) {
		next = pkt->u.next;
		/* trim the out grown list*/
		if (++i > level) {
			netbuf =
				(qdf_nbuf_t)(pkt->u.pkt.htc_pkt.pNetBufContext);
			qdf_nbuf_unmap(soc->osdev, netbuf, QDF_DMA_TO_DEVICE);
			qdf_nbuf_free(netbuf);
			qdf_mem_free(pkt);
			pkt = NULL;
			if (prev)
				prev->u.next = NULL;
		}
		prev = pkt;
		pkt = next;
	}
	HTT_TX_MUTEX_RELEASE(&soc->htt_tx_mutex);
}

/*
 * htt_htc_misc_pkt_list_add() - Add pkt to misc list
 * @htt_soc:	HTT SOC handle
 * @dp_htt_htc_pkt: pkt to be added to list
 */
static void
htt_htc_misc_pkt_list_add(struct htt_soc *soc, struct dp_htt_htc_pkt *pkt)
{
	struct dp_htt_htc_pkt_union *u_pkt =
				(struct dp_htt_htc_pkt_union *)pkt;
	int misclist_trim_level = htc_get_tx_queue_depth(soc->htc_soc,
							pkt->htc_pkt.Endpoint)
				+ DP_HTT_HTC_PKT_MISCLIST_SIZE;

	HTT_TX_MUTEX_ACQUIRE(&soc->htt_tx_mutex);
	if (soc->htt_htc_pkt_misclist) {
		u_pkt->u.next = soc->htt_htc_pkt_misclist;
		soc->htt_htc_pkt_misclist = u_pkt;
	} else {
		soc->htt_htc_pkt_misclist = u_pkt;
	}
	HTT_TX_MUTEX_RELEASE(&soc->htt_tx_mutex);

	/* only ce pipe size + tx_queue_depth could possibly be in use
	 * free older packets in the misclist
	 */
	htt_htc_misc_pkt_list_trim(soc, misclist_trim_level);
}

/**
 * DP_HTT_SEND_HTC_PKT() - Send htt packet from host
 * @soc : HTT SOC handle
 * @pkt: pkt to be send
 * @cmd : command to be recorded in dp htt logger
 * @buf : Pointer to buffer needs to be recored for above cmd
 *
 * Return: None
 */
static inline void DP_HTT_SEND_HTC_PKT(struct htt_soc *soc,
				       struct dp_htt_htc_pkt *pkt, uint8_t cmd,
				       uint8_t *buf)
{
	htt_command_record(soc->htt_logger_handle, cmd, buf);
	if (htc_send_pkt(soc->htc_soc, &pkt->htc_pkt) ==
	    QDF_STATUS_SUCCESS)
		htt_htc_misc_pkt_list_add(soc, pkt);
}

/*
 * htt_htc_misc_pkt_pool_free() - free pkts in misc list
 * @htt_soc:	HTT SOC handle
 */
static void
htt_htc_misc_pkt_pool_free(struct htt_soc *soc)
{
	struct dp_htt_htc_pkt_union *pkt, *next;
	qdf_nbuf_t netbuf;

	pkt = soc->htt_htc_pkt_misclist;

	while (pkt) {
		next = pkt->u.next;
		netbuf = (qdf_nbuf_t) (pkt->u.pkt.htc_pkt.pNetBufContext);
		qdf_nbuf_unmap(soc->osdev, netbuf, QDF_DMA_TO_DEVICE);

		soc->stats.htc_pkt_free++;
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_LOW,
			 "%s: Pkt free count %d",
			 __func__, soc->stats.htc_pkt_free);

		qdf_nbuf_free(netbuf);
		qdf_mem_free(pkt);
		pkt = next;
	}
	soc->htt_htc_pkt_misclist = NULL;
}

/*
 * htt_t2h_mac_addr_deswizzle() - Swap MAC addr bytes if FW endianness differ
 * @tgt_mac_addr:	Target MAC
 * @buffer:		Output buffer
 */
static u_int8_t *
htt_t2h_mac_addr_deswizzle(u_int8_t *tgt_mac_addr, u_int8_t *buffer)
{
#ifdef BIG_ENDIAN_HOST
	/*
	 * The host endianness is opposite of the target endianness.
	 * To make u_int32_t elements come out correctly, the target->host
	 * upload has swizzled the bytes in each u_int32_t element of the
	 * message.
	 * For byte-array message fields like the MAC address, this
	 * upload swizzling puts the bytes in the wrong order, and needs
	 * to be undone.
	 */
	buffer[0] = tgt_mac_addr[3];
	buffer[1] = tgt_mac_addr[2];
	buffer[2] = tgt_mac_addr[1];
	buffer[3] = tgt_mac_addr[0];
	buffer[4] = tgt_mac_addr[7];
	buffer[5] = tgt_mac_addr[6];
	return buffer;
#else
	/*
	 * The host endianness matches the target endianness -
	 * we can use the mac addr directly from the message buffer.
	 */
	return tgt_mac_addr;
#endif
}

/*
 * dp_htt_h2t_send_complete_free_netbuf() - Free completed buffer
 * @soc:	SOC handle
 * @status:	Completion status
 * @netbuf:	HTT buffer
 */
static void
dp_htt_h2t_send_complete_free_netbuf(
	void *soc, A_STATUS status, qdf_nbuf_t netbuf)
{
	qdf_nbuf_free(netbuf);
}

/*
 * dp_htt_h2t_send_complete() - H2T completion handler
 * @context:	Opaque context (HTT SOC handle)
 * @htc_pkt:	HTC packet
 */
static void
dp_htt_h2t_send_complete(void *context, HTC_PACKET *htc_pkt)
{
	void (*send_complete_part2)(
		void *soc, QDF_STATUS status, qdf_nbuf_t msdu);
	struct htt_soc *soc =  (struct htt_soc *) context;
	struct dp_htt_htc_pkt *htt_pkt;
	qdf_nbuf_t netbuf;

	send_complete_part2 = htc_pkt->pPktContext;

	htt_pkt = container_of(htc_pkt, struct dp_htt_htc_pkt, htc_pkt);

	/* process (free or keep) the netbuf that held the message */
	netbuf = (qdf_nbuf_t) htc_pkt->pNetBufContext;
	/*
	 * adf sendcomplete is required for windows only
	 */
	/* qdf_nbuf_set_sendcompleteflag(netbuf, TRUE); */
	if (send_complete_part2) {
		send_complete_part2(
			htt_pkt->soc_ctxt, htc_pkt->Status, netbuf);
	}
	/* free the htt_htc_pkt / HTC_PACKET object */
	htt_htc_pkt_free(soc, htt_pkt);
}

/*
 * htt_h2t_ver_req_msg() - Send HTT version request message to target
 * @htt_soc:	HTT SOC handle
 *
 * Return: 0 on success; error code on failure
 */
static int htt_h2t_ver_req_msg(struct htt_soc *soc)
{
	struct dp_htt_htc_pkt *pkt;
	qdf_nbuf_t msg;
	uint32_t *msg_word;

	msg = qdf_nbuf_alloc(
		soc->osdev,
		HTT_MSG_BUF_SIZE(HTT_VER_REQ_BYTES),
		/* reserve room for the HTC header */
		HTC_HEADER_LEN + HTC_HDR_ALIGNMENT_PADDING, 4, TRUE);
	if (!msg)
		return QDF_STATUS_E_NOMEM;

	/*
	 * Set the length of the message.
	 * The contribution from the HTC_HDR_ALIGNMENT_PADDING is added
	 * separately during the below call to qdf_nbuf_push_head.
	 * The contribution from the HTC header is added separately inside HTC.
	 */
	if (qdf_nbuf_put_tail(msg, HTT_VER_REQ_BYTES) == NULL) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: Failed to expand head for HTT_H2T_MSG_TYPE_VERSION_REQ msg",
			__func__);
		return QDF_STATUS_E_FAILURE;
	}

	/* fill in the message contents */
	msg_word = (u_int32_t *) qdf_nbuf_data(msg);

	/* rewind beyond alignment pad to get to the HTC header reserved area */
	qdf_nbuf_push_head(msg, HTC_HDR_ALIGNMENT_PADDING);

	*msg_word = 0;
	HTT_H2T_MSG_TYPE_SET(*msg_word, HTT_H2T_MSG_TYPE_VERSION_REQ);

	pkt = htt_htc_pkt_alloc(soc);
	if (!pkt) {
		qdf_nbuf_free(msg);
		return QDF_STATUS_E_FAILURE;
	}
	pkt->soc_ctxt = NULL; /* not used during send-done callback */

	SET_HTC_PACKET_INFO_TX(&pkt->htc_pkt,
		dp_htt_h2t_send_complete_free_netbuf, qdf_nbuf_data(msg),
		qdf_nbuf_len(msg), soc->htc_endpoint,
		1); /* tag - not relevant here */

	SET_HTC_PACKET_NET_BUF_CONTEXT(&pkt->htc_pkt, msg);
	DP_HTT_SEND_HTC_PKT(soc, pkt, HTT_H2T_MSG_TYPE_VERSION_REQ, NULL);
	return 0;
}

/*
 * htt_srng_setup() - Send SRNG setup message to target
 * @htt_soc:	HTT SOC handle
 * @mac_id:	MAC Id
 * @hal_srng:	Opaque HAL SRNG pointer
 * @hal_ring_type:	SRNG ring type
 *
 * Return: 0 on success; error code on failure
 */
int htt_srng_setup(struct htt_soc *soc, int mac_id,
		   hal_ring_handle_t hal_ring_hdl,
		   int hal_ring_type)
{
	struct dp_htt_htc_pkt *pkt;
	qdf_nbuf_t htt_msg;
	uint32_t *msg_word;
	struct hal_srng_params srng_params;
	qdf_dma_addr_t hp_addr, tp_addr;
	uint32_t ring_entry_size =
		hal_srng_get_entrysize(soc->hal_soc, hal_ring_type);
	int htt_ring_type, htt_ring_id;
	uint8_t *htt_logger_bufp;

	/* Sizes should be set in 4-byte words */
	ring_entry_size = ring_entry_size >> 2;

	htt_msg = qdf_nbuf_alloc(soc->osdev,
		HTT_MSG_BUF_SIZE(HTT_SRING_SETUP_SZ),
		/* reserve room for the HTC header */
		HTC_HEADER_LEN + HTC_HDR_ALIGNMENT_PADDING, 4, TRUE);
	if (!htt_msg)
		goto fail0;

	hal_get_srng_params(soc->hal_soc, hal_ring_hdl, &srng_params);
	hp_addr = hal_srng_get_hp_addr(soc->hal_soc, hal_ring_hdl);
	tp_addr = hal_srng_get_tp_addr(soc->hal_soc, hal_ring_hdl);

	switch (hal_ring_type) {
	case RXDMA_BUF:
#ifdef QCA_HOST2FW_RXBUF_RING
		if (srng_params.ring_id ==
		    (HAL_SRNG_WMAC1_SW2RXDMA0_BUF0)) {
			htt_ring_id = HTT_HOST1_TO_FW_RXBUF_RING;
			htt_ring_type = HTT_SW_TO_SW_RING;
#ifdef IPA_OFFLOAD
		} else if (srng_params.ring_id ==
		    (HAL_SRNG_WMAC1_SW2RXDMA0_BUF2)) {
			htt_ring_id = HTT_HOST2_TO_FW_RXBUF_RING;
			htt_ring_type = HTT_SW_TO_SW_RING;
#endif
#else
		if (srng_params.ring_id ==
			(HAL_SRNG_WMAC1_SW2RXDMA0_BUF0 +
			  (mac_id * HAL_MAX_RINGS_PER_LMAC))) {
			htt_ring_id = HTT_RXDMA_HOST_BUF_RING;
			htt_ring_type = HTT_SW_TO_HW_RING;
#endif
		} else if (srng_params.ring_id ==
#ifdef IPA_OFFLOAD
			 (HAL_SRNG_WMAC1_SW2RXDMA0_BUF1 +
#else
			 (HAL_SRNG_WMAC1_SW2RXDMA1_BUF +
#endif
			  (mac_id * HAL_MAX_RINGS_PER_LMAC))) {
			htt_ring_id = HTT_RXDMA_HOST_BUF_RING;
			htt_ring_type = HTT_SW_TO_HW_RING;
		} else {
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				   "%s: Ring %d currently not supported",
				   __func__, srng_params.ring_id);
			goto fail1;
		}

		dp_info("ring_type %d ring_id %d htt_ring_id %d hp_addr 0x%llx tp_addr 0x%llx",
			hal_ring_type, srng_params.ring_id, htt_ring_id,
			(uint64_t)hp_addr,
			(uint64_t)tp_addr);
		break;
	case RXDMA_MONITOR_BUF:
		htt_ring_id = HTT_RXDMA_MONITOR_BUF_RING;
		htt_ring_type = HTT_SW_TO_HW_RING;
		break;
	case RXDMA_MONITOR_STATUS:
		htt_ring_id = HTT_RXDMA_MONITOR_STATUS_RING;
		htt_ring_type = HTT_SW_TO_HW_RING;
		break;
	case RXDMA_MONITOR_DST:
		htt_ring_id = HTT_RXDMA_MONITOR_DEST_RING;
		htt_ring_type = HTT_HW_TO_SW_RING;
		break;
	case RXDMA_MONITOR_DESC:
		htt_ring_id = HTT_RXDMA_MONITOR_DESC_RING;
		htt_ring_type = HTT_SW_TO_HW_RING;
		break;
	case RXDMA_DST:
		htt_ring_id = HTT_RXDMA_NON_MONITOR_DEST_RING;
		htt_ring_type = HTT_HW_TO_SW_RING;
		break;

	default:
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: Ring currently not supported", __func__);
			goto fail1;
	}

	/*
	 * Set the length of the message.
	 * The contribution from the HTC_HDR_ALIGNMENT_PADDING is added
	 * separately during the below call to qdf_nbuf_push_head.
	 * The contribution from the HTC header is added separately inside HTC.
	 */
	if (qdf_nbuf_put_tail(htt_msg, HTT_SRING_SETUP_SZ) == NULL) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: Failed to expand head for SRING_SETUP msg",
			__func__);
		return QDF_STATUS_E_FAILURE;
	}

	msg_word = (uint32_t *)qdf_nbuf_data(htt_msg);

	/* rewind beyond alignment pad to get to the HTC header reserved area */
	qdf_nbuf_push_head(htt_msg, HTC_HDR_ALIGNMENT_PADDING);

	/* word 0 */
	*msg_word = 0;
	htt_logger_bufp = (uint8_t *)msg_word;
	HTT_H2T_MSG_TYPE_SET(*msg_word, HTT_H2T_MSG_TYPE_SRING_SETUP);

	if ((htt_ring_type == HTT_SW_TO_HW_RING) ||
			(htt_ring_type == HTT_HW_TO_SW_RING))
		HTT_SRING_SETUP_PDEV_ID_SET(*msg_word,
			 DP_SW2HW_MACID(mac_id));
	else
		HTT_SRING_SETUP_PDEV_ID_SET(*msg_word, mac_id);

	dp_info("%s: mac_id %d", __func__, mac_id);
	HTT_SRING_SETUP_RING_TYPE_SET(*msg_word, htt_ring_type);
	/* TODO: Discuss with FW on changing this to unique ID and using
	 * htt_ring_type to send the type of ring
	 */
	HTT_SRING_SETUP_RING_ID_SET(*msg_word, htt_ring_id);

	/* word 1 */
	msg_word++;
	*msg_word = 0;
	HTT_SRING_SETUP_RING_BASE_ADDR_LO_SET(*msg_word,
		srng_params.ring_base_paddr & 0xffffffff);

	/* word 2 */
	msg_word++;
	*msg_word = 0;
	HTT_SRING_SETUP_RING_BASE_ADDR_HI_SET(*msg_word,
		(uint64_t)srng_params.ring_base_paddr >> 32);

	/* word 3 */
	msg_word++;
	*msg_word = 0;
	HTT_SRING_SETUP_ENTRY_SIZE_SET(*msg_word, ring_entry_size);
	HTT_SRING_SETUP_RING_SIZE_SET(*msg_word,
		(ring_entry_size * srng_params.num_entries));
	dp_info("%s: entry_size %d", __func__, ring_entry_size);
	dp_info("%s: num_entries %d", __func__, srng_params.num_entries);
	dp_info("%s: ring_size %d", __func__,
		(ring_entry_size * srng_params.num_entries));
	if (htt_ring_type == HTT_SW_TO_HW_RING)
		HTT_SRING_SETUP_RING_MISC_CFG_FLAG_LOOPCOUNT_DISABLE_SET(
						*msg_word, 1);
	HTT_SRING_SETUP_RING_MISC_CFG_FLAG_MSI_SWAP_SET(*msg_word,
		!!(srng_params.flags & HAL_SRNG_MSI_SWAP));
	HTT_SRING_SETUP_RING_MISC_CFG_FLAG_TLV_SWAP_SET(*msg_word,
		!!(srng_params.flags & HAL_SRNG_DATA_TLV_SWAP));
	HTT_SRING_SETUP_RING_MISC_CFG_FLAG_HOST_FW_SWAP_SET(*msg_word,
		!!(srng_params.flags & HAL_SRNG_RING_PTR_SWAP));

	/* word 4 */
	msg_word++;
	*msg_word = 0;
	HTT_SRING_SETUP_HEAD_OFFSET32_REMOTE_BASE_ADDR_LO_SET(*msg_word,
		hp_addr & 0xffffffff);

	/* word 5 */
	msg_word++;
	*msg_word = 0;
	HTT_SRING_SETUP_HEAD_OFFSET32_REMOTE_BASE_ADDR_HI_SET(*msg_word,
		(uint64_t)hp_addr >> 32);

	/* word 6 */
	msg_word++;
	*msg_word = 0;
	HTT_SRING_SETUP_TAIL_OFFSET32_REMOTE_BASE_ADDR_LO_SET(*msg_word,
		tp_addr & 0xffffffff);

	/* word 7 */
	msg_word++;
	*msg_word = 0;
	HTT_SRING_SETUP_TAIL_OFFSET32_REMOTE_BASE_ADDR_HI_SET(*msg_word,
		(uint64_t)tp_addr >> 32);

	/* word 8 */
	msg_word++;
	*msg_word = 0;
	HTT_SRING_SETUP_RING_MSI_ADDR_LO_SET(*msg_word,
		srng_params.msi_addr & 0xffffffff);

	/* word 9 */
	msg_word++;
	*msg_word = 0;
	HTT_SRING_SETUP_RING_MSI_ADDR_HI_SET(*msg_word,
		(uint64_t)(srng_params.msi_addr) >> 32);

	/* word 10 */
	msg_word++;
	*msg_word = 0;
	HTT_SRING_SETUP_RING_MSI_DATA_SET(*msg_word,
		srng_params.msi_data);

	/* word 11 */
	msg_word++;
	*msg_word = 0;
	HTT_SRING_SETUP_INTR_BATCH_COUNTER_TH_SET(*msg_word,
		srng_params.intr_batch_cntr_thres_entries *
		ring_entry_size);
	HTT_SRING_SETUP_INTR_TIMER_TH_SET(*msg_word,
		srng_params.intr_timer_thres_us >> 3);

	/* word 12 */
	msg_word++;
	*msg_word = 0;
	if (srng_params.flags & HAL_SRNG_LOW_THRES_INTR_ENABLE) {
		/* TODO: Setting low threshold to 1/8th of ring size - see
		 * if this needs to be configurable
		 */
		HTT_SRING_SETUP_INTR_LOW_TH_SET(*msg_word,
			srng_params.low_threshold);
	}
	/* "response_required" field should be set if a HTT response message is
	 * required after setting up the ring.
	 */
	pkt = htt_htc_pkt_alloc(soc);
	if (!pkt)
		goto fail1;

	pkt->soc_ctxt = NULL; /* not used during send-done callback */

	SET_HTC_PACKET_INFO_TX(
		&pkt->htc_pkt,
		dp_htt_h2t_send_complete_free_netbuf,
		qdf_nbuf_data(htt_msg),
		qdf_nbuf_len(htt_msg),
		soc->htc_endpoint,
		HTC_TX_PACKET_TAG_RUNTIME_PUT); /* tag for no FW response msg */

	SET_HTC_PACKET_NET_BUF_CONTEXT(&pkt->htc_pkt, htt_msg);
	DP_HTT_SEND_HTC_PKT(soc, pkt, HTT_H2T_MSG_TYPE_SRING_SETUP,
			    htt_logger_bufp);

	return QDF_STATUS_SUCCESS;

fail1:
	qdf_nbuf_free(htt_msg);
fail0:
	return QDF_STATUS_E_FAILURE;
}

/*
 * htt_h2t_rx_ring_cfg() - Send SRNG packet and TLV filter
 * config message to target
 * @htt_soc:	HTT SOC handle
 * @pdev_id:	PDEV Id
 * @hal_srng:	Opaque HAL SRNG pointer
 * @hal_ring_type:	SRNG ring type
 * @ring_buf_size:	SRNG buffer size
 * @htt_tlv_filter:	Rx SRNG TLV and filter setting
 * Return: 0 on success; error code on failure
 */
int htt_h2t_rx_ring_cfg(struct htt_soc *htt_soc, int pdev_id,
			hal_ring_handle_t hal_ring_hdl,
			int hal_ring_type, int ring_buf_size,
			struct htt_rx_ring_tlv_filter *htt_tlv_filter)
{
	struct htt_soc *soc = (struct htt_soc *)htt_soc;
	struct dp_htt_htc_pkt *pkt;
	qdf_nbuf_t htt_msg;
	uint32_t *msg_word;
	struct hal_srng_params srng_params;
	uint32_t htt_ring_type, htt_ring_id;
	uint32_t tlv_filter;
	uint8_t *htt_logger_bufp;
	struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx = soc->dp_soc->wlan_cfg_ctx;
	uint32_t mon_drop_th = wlan_cfg_get_mon_drop_thresh(wlan_cfg_ctx);

	htt_msg = qdf_nbuf_alloc(soc->osdev,
		HTT_MSG_BUF_SIZE(HTT_RX_RING_SELECTION_CFG_SZ),
	/* reserve room for the HTC header */
	HTC_HEADER_LEN + HTC_HDR_ALIGNMENT_PADDING, 4, TRUE);
	if (!htt_msg)
		goto fail0;

	hal_get_srng_params(soc->hal_soc, hal_ring_hdl, &srng_params);

	switch (hal_ring_type) {
	case RXDMA_BUF:
		htt_ring_id = HTT_RXDMA_HOST_BUF_RING;
		htt_ring_type = HTT_SW_TO_HW_RING;
		break;
	case RXDMA_MONITOR_BUF:
		htt_ring_id = HTT_RXDMA_MONITOR_BUF_RING;
		htt_ring_type = HTT_SW_TO_HW_RING;
		break;
	case RXDMA_MONITOR_STATUS:
		htt_ring_id = HTT_RXDMA_MONITOR_STATUS_RING;
		htt_ring_type = HTT_SW_TO_HW_RING;
		break;
	case RXDMA_MONITOR_DST:
		htt_ring_id = HTT_RXDMA_MONITOR_DEST_RING;
		htt_ring_type = HTT_HW_TO_SW_RING;
		break;
	case RXDMA_MONITOR_DESC:
		htt_ring_id = HTT_RXDMA_MONITOR_DESC_RING;
		htt_ring_type = HTT_SW_TO_HW_RING;
		break;
	case RXDMA_DST:
		htt_ring_id = HTT_RXDMA_NON_MONITOR_DEST_RING;
		htt_ring_type = HTT_HW_TO_SW_RING;
		break;

	default:
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: Ring currently not supported", __func__);
		goto fail1;
	}

	/*
	 * Set the length of the message.
	 * The contribution from the HTC_HDR_ALIGNMENT_PADDING is added
	 * separately during the below call to qdf_nbuf_push_head.
	 * The contribution from the HTC header is added separately inside HTC.
	 */
	if (qdf_nbuf_put_tail(htt_msg, HTT_RX_RING_SELECTION_CFG_SZ) == NULL) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: Failed to expand head for RX Ring Cfg msg",
			__func__);
		goto fail1; /* failure */
	}

	msg_word = (uint32_t *)qdf_nbuf_data(htt_msg);

	/* rewind beyond alignment pad to get to the HTC header reserved area */
	qdf_nbuf_push_head(htt_msg, HTC_HDR_ALIGNMENT_PADDING);

	/* word 0 */
	htt_logger_bufp = (uint8_t *)msg_word;
	*msg_word = 0;
	HTT_H2T_MSG_TYPE_SET(*msg_word, HTT_H2T_MSG_TYPE_RX_RING_SELECTION_CFG);

	/*
	 * pdev_id is indexed from 0 whereas mac_id is indexed from 1
	 * SW_TO_SW and SW_TO_HW rings are unaffected by this
	 */
	if (htt_ring_type == HTT_SW_TO_SW_RING ||
			htt_ring_type == HTT_SW_TO_HW_RING)
		HTT_RX_RING_SELECTION_CFG_PDEV_ID_SET(*msg_word,
						DP_SW2HW_MACID(pdev_id));

	/* TODO: Discuss with FW on changing this to unique ID and using
	 * htt_ring_type to send the type of ring
	 */
	HTT_RX_RING_SELECTION_CFG_RING_ID_SET(*msg_word, htt_ring_id);

	HTT_RX_RING_SELECTION_CFG_STATUS_TLV_SET(*msg_word,
		!!(srng_params.flags & HAL_SRNG_MSI_SWAP));

	HTT_RX_RING_SELECTION_CFG_PKT_TLV_SET(*msg_word,
		!!(srng_params.flags & HAL_SRNG_DATA_TLV_SWAP));

	HTT_RX_RING_SELECTION_CFG_RX_OFFSETS_VALID_SET(*msg_word,
						htt_tlv_filter->offset_valid);

	if (mon_drop_th > 0)
		HTT_RX_RING_SELECTION_CFG_DROP_THRESHOLD_VALID_SET(*msg_word,
								   1);
	else
		HTT_RX_RING_SELECTION_CFG_DROP_THRESHOLD_VALID_SET(*msg_word,
								   0);

	/* word 1 */
	msg_word++;
	*msg_word = 0;
	HTT_RX_RING_SELECTION_CFG_RING_BUFFER_SIZE_SET(*msg_word,
		ring_buf_size);

	/* word 2 */
	msg_word++;
	*msg_word = 0;

	if (htt_tlv_filter->enable_fp) {
		/* TYPE: MGMT */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			FP, MGMT, 0000,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_ASSOC_REQ) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			FP, MGMT, 0001,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_ASSOC_RES) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			FP, MGMT, 0010,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_REASSOC_REQ) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			FP, MGMT, 0011,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_REASSOC_RES) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			FP, MGMT, 0100,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_PROBE_REQ) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			FP, MGMT, 0101,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_PROBE_RES) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			FP, MGMT, 0110,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_TIM_ADVT) ? 1 : 0);
		/* reserved */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, FP,
			MGMT, 0111,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_RESERVED_7) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			FP, MGMT, 1000,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_BEACON) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			FP, MGMT, 1001,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_ATIM) ? 1 : 0);
	}

	if (htt_tlv_filter->enable_md) {
			/* TYPE: MGMT */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MD, MGMT, 0000,
			(htt_tlv_filter->md_mgmt_filter &
			FILTER_MGMT_ASSOC_REQ) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MD, MGMT, 0001,
			(htt_tlv_filter->md_mgmt_filter &
			FILTER_MGMT_ASSOC_RES) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MD, MGMT, 0010,
			(htt_tlv_filter->md_mgmt_filter &
			FILTER_MGMT_REASSOC_REQ) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MD, MGMT, 0011,
			(htt_tlv_filter->md_mgmt_filter &
			FILTER_MGMT_REASSOC_RES) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MD, MGMT, 0100,
			(htt_tlv_filter->md_mgmt_filter &
			FILTER_MGMT_PROBE_REQ) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MD, MGMT, 0101,
			(htt_tlv_filter->md_mgmt_filter &
			FILTER_MGMT_PROBE_RES) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MD, MGMT, 0110,
			(htt_tlv_filter->md_mgmt_filter &
			FILTER_MGMT_TIM_ADVT) ? 1 : 0);
		/* reserved */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, MD,
			MGMT, 0111,
			(htt_tlv_filter->md_mgmt_filter &
			FILTER_MGMT_RESERVED_7) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MD, MGMT, 1000,
			(htt_tlv_filter->md_mgmt_filter &
			FILTER_MGMT_BEACON) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MD, MGMT, 1001,
			(htt_tlv_filter->md_mgmt_filter &
			FILTER_MGMT_ATIM) ? 1 : 0);
	}

	if (htt_tlv_filter->enable_mo) {
		/* TYPE: MGMT */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MO, MGMT, 0000,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_ASSOC_REQ) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MO, MGMT, 0001,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_ASSOC_RES) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MO, MGMT, 0010,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_REASSOC_REQ) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MO, MGMT, 0011,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_REASSOC_RES) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MO, MGMT, 0100,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_PROBE_REQ) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MO, MGMT, 0101,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_PROBE_RES) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MO, MGMT, 0110,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_TIM_ADVT) ? 1 : 0);
		/* reserved */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, MO,
			MGMT, 0111,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_RESERVED_7) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MO, MGMT, 1000,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_BEACON) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MO, MGMT, 1001,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_ATIM) ? 1 : 0);
	}

	/* word 3 */
	msg_word++;
	*msg_word = 0;

	if (htt_tlv_filter->enable_fp) {
		/* TYPE: MGMT */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			FP, MGMT, 1010,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_DISASSOC) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			FP, MGMT, 1011,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_AUTH) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			FP, MGMT, 1100,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_DEAUTH) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			FP, MGMT, 1101,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_ACTION) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			FP, MGMT, 1110,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_ACT_NO_ACK) ? 1 : 0);
		/* reserved*/
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1, FP,
			MGMT, 1111,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_RESERVED_15) ? 1 : 0);
	}

	if (htt_tlv_filter->enable_md) {
			/* TYPE: MGMT */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			MD, MGMT, 1010,
			(htt_tlv_filter->md_mgmt_filter &
			FILTER_MGMT_DISASSOC) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			MD, MGMT, 1011,
			(htt_tlv_filter->md_mgmt_filter &
			FILTER_MGMT_AUTH) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			MD, MGMT, 1100,
			(htt_tlv_filter->md_mgmt_filter &
			FILTER_MGMT_DEAUTH) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			MD, MGMT, 1101,
			(htt_tlv_filter->md_mgmt_filter &
			FILTER_MGMT_ACTION) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			MD, MGMT, 1110,
			(htt_tlv_filter->md_mgmt_filter &
			FILTER_MGMT_ACT_NO_ACK) ? 1 : 0);
	}

	if (htt_tlv_filter->enable_mo) {
		/* TYPE: MGMT */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			MO, MGMT, 1010,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_DISASSOC) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			MO, MGMT, 1011,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_AUTH) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			MO, MGMT, 1100,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_DEAUTH) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			MO, MGMT, 1101,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_ACTION) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			MO, MGMT, 1110,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_ACT_NO_ACK) ? 1 : 0);
		/* reserved*/
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1, MO,
			MGMT, 1111,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_RESERVED_15) ? 1 : 0);
	}

	/* word 4 */
	msg_word++;
	*msg_word = 0;

	if (htt_tlv_filter->enable_fp) {
		/* TYPE: CTRL */
		/* reserved */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, FP,
			CTRL, 0000,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_RESERVED_1) ? 1 : 0);
		/* reserved */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, FP,
			CTRL, 0001,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_RESERVED_2) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, FP,
			CTRL, 0010,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_TRIGGER) ? 1 : 0);
		/* reserved */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, FP,
			CTRL, 0011,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_RESERVED_4) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, FP,
			CTRL, 0100,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_BF_REP_POLL) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, FP,
			CTRL, 0101,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_VHT_NDP) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, FP,
			CTRL, 0110,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_FRAME_EXT) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, FP,
			CTRL, 0111,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_CTRLWRAP) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, FP,
			CTRL, 1000,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_BA_REQ) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, FP,
			CTRL, 1001,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_BA) ? 1 : 0);
	}

	if (htt_tlv_filter->enable_md) {
		/* TYPE: CTRL */
		/* reserved */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MD,
			CTRL, 0000,
			(htt_tlv_filter->md_ctrl_filter &
			FILTER_CTRL_RESERVED_1) ? 1 : 0);
		/* reserved */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MD,
			CTRL, 0001,
			(htt_tlv_filter->md_ctrl_filter &
			FILTER_CTRL_RESERVED_2) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MD,
			CTRL, 0010,
			(htt_tlv_filter->md_ctrl_filter &
			FILTER_CTRL_TRIGGER) ? 1 : 0);
		/* reserved */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MD,
			CTRL, 0011,
			(htt_tlv_filter->md_ctrl_filter &
			FILTER_CTRL_RESERVED_4) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MD,
			CTRL, 0100,
			(htt_tlv_filter->md_ctrl_filter &
			FILTER_CTRL_BF_REP_POLL) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MD,
			CTRL, 0101,
			(htt_tlv_filter->md_ctrl_filter &
			FILTER_CTRL_VHT_NDP) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MD,
			CTRL, 0110,
			(htt_tlv_filter->md_ctrl_filter &
			FILTER_CTRL_FRAME_EXT) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MD,
			CTRL, 0111,
			(htt_tlv_filter->md_ctrl_filter &
			FILTER_CTRL_CTRLWRAP) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MD,
			CTRL, 1000,
			(htt_tlv_filter->md_ctrl_filter &
			FILTER_CTRL_BA_REQ) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MD,
			CTRL, 1001,
			(htt_tlv_filter->md_ctrl_filter &
			FILTER_CTRL_BA) ? 1 : 0);
	}

	if (htt_tlv_filter->enable_mo) {
		/* TYPE: CTRL */
		/* reserved */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
			CTRL, 0000,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_RESERVED_1) ? 1 : 0);
		/* reserved */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
			CTRL, 0001,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_RESERVED_2) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
			CTRL, 0010,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_TRIGGER) ? 1 : 0);
		/* reserved */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
			CTRL, 0011,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_RESERVED_4) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
			CTRL, 0100,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_BF_REP_POLL) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
			CTRL, 0101,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_VHT_NDP) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
			CTRL, 0110,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_FRAME_EXT) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
			CTRL, 0111,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_CTRLWRAP) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
			CTRL, 1000,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_BA_REQ) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
			CTRL, 1001,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_BA) ? 1 : 0);
	}

	/* word 5 */
	msg_word++;
	*msg_word = 0;
	if (htt_tlv_filter->enable_fp) {
		/* TYPE: CTRL */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			CTRL, 1010,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_PSPOLL) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			CTRL, 1011,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_RTS) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			CTRL, 1100,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_CTS) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			CTRL, 1101,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_ACK) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			CTRL, 1110,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_CFEND) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			CTRL, 1111,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_CFEND_CFACK) ? 1 : 0);
		/* TYPE: DATA */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			DATA, MCAST,
			(htt_tlv_filter->fp_data_filter &
			FILTER_DATA_MCAST) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			DATA, UCAST,
			(htt_tlv_filter->fp_data_filter &
			FILTER_DATA_UCAST) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			DATA, NULL,
			(htt_tlv_filter->fp_data_filter &
			FILTER_DATA_NULL) ? 1 : 0);
	}

	if (htt_tlv_filter->enable_md) {
		/* TYPE: CTRL */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MD,
			CTRL, 1010,
			(htt_tlv_filter->md_ctrl_filter &
			FILTER_CTRL_PSPOLL) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MD,
			CTRL, 1011,
			(htt_tlv_filter->md_ctrl_filter &
			FILTER_CTRL_RTS) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MD,
			CTRL, 1100,
			(htt_tlv_filter->md_ctrl_filter &
			FILTER_CTRL_CTS) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MD,
			CTRL, 1101,
			(htt_tlv_filter->md_ctrl_filter &
			FILTER_CTRL_ACK) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MD,
			CTRL, 1110,
			(htt_tlv_filter->md_ctrl_filter &
			FILTER_CTRL_CFEND) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MD,
			CTRL, 1111,
			(htt_tlv_filter->md_ctrl_filter &
			FILTER_CTRL_CFEND_CFACK) ? 1 : 0);
		/* TYPE: DATA */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MD,
			DATA, MCAST,
			(htt_tlv_filter->md_data_filter &
			FILTER_DATA_MCAST) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MD,
			DATA, UCAST,
			(htt_tlv_filter->md_data_filter &
			FILTER_DATA_UCAST) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MD,
			DATA, NULL,
			(htt_tlv_filter->md_data_filter &
			FILTER_DATA_NULL) ? 1 : 0);
	}

	if (htt_tlv_filter->enable_mo) {
		/* TYPE: CTRL */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			CTRL, 1010,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_PSPOLL) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			CTRL, 1011,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_RTS) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			CTRL, 1100,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_CTS) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			CTRL, 1101,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_ACK) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			CTRL, 1110,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_CFEND) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			CTRL, 1111,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_CFEND_CFACK) ? 1 : 0);
		/* TYPE: DATA */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			DATA, MCAST,
			(htt_tlv_filter->mo_data_filter &
			FILTER_DATA_MCAST) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			DATA, UCAST,
			(htt_tlv_filter->mo_data_filter &
			FILTER_DATA_UCAST) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			DATA, NULL,
			(htt_tlv_filter->mo_data_filter &
			FILTER_DATA_NULL) ? 1 : 0);
	}

	/* word 6 */
	msg_word++;
	*msg_word = 0;
	tlv_filter = 0;
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, MPDU_START,
		htt_tlv_filter->mpdu_start);
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, MSDU_START,
		htt_tlv_filter->msdu_start);
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, PACKET,
		htt_tlv_filter->packet);
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, MSDU_END,
		htt_tlv_filter->msdu_end);
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, MPDU_END,
		htt_tlv_filter->mpdu_end);
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, PACKET_HEADER,
		htt_tlv_filter->packet_header);
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, ATTENTION,
		htt_tlv_filter->attention);
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, PPDU_START,
		htt_tlv_filter->ppdu_start);
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, PPDU_END,
		htt_tlv_filter->ppdu_end);
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, PPDU_END_USER_STATS,
		htt_tlv_filter->ppdu_end_user_stats);
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter,
		PPDU_END_USER_STATS_EXT,
		htt_tlv_filter->ppdu_end_user_stats_ext);
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, PPDU_END_STATUS_DONE,
		htt_tlv_filter->ppdu_end_status_done);
	/* RESERVED bit maps to header_per_msdu in htt_tlv_filter*/
	 htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, RESERVED,
		 htt_tlv_filter->header_per_msdu);

	HTT_RX_RING_SELECTION_CFG_TLV_FILTER_IN_FLAG_SET(*msg_word, tlv_filter);

	msg_word++;
	*msg_word = 0;
	if (htt_tlv_filter->offset_valid) {
		HTT_RX_RING_SELECTION_CFG_RX_PACKET_OFFSET_SET(*msg_word,
					htt_tlv_filter->rx_packet_offset);
		HTT_RX_RING_SELECTION_CFG_RX_HEADER_OFFSET_SET(*msg_word,
					htt_tlv_filter->rx_header_offset);

		msg_word++;
		*msg_word = 0;
		HTT_RX_RING_SELECTION_CFG_RX_MPDU_END_OFFSET_SET(*msg_word,
					htt_tlv_filter->rx_mpdu_end_offset);
		HTT_RX_RING_SELECTION_CFG_RX_MPDU_START_OFFSET_SET(*msg_word,
					htt_tlv_filter->rx_mpdu_start_offset);

		msg_word++;
		*msg_word = 0;
		HTT_RX_RING_SELECTION_CFG_RX_MSDU_END_OFFSET_SET(*msg_word,
					htt_tlv_filter->rx_msdu_end_offset);
		HTT_RX_RING_SELECTION_CFG_RX_MSDU_START_OFFSET_SET(*msg_word,
					htt_tlv_filter->rx_msdu_start_offset);

		msg_word++;
		*msg_word = 0;
		HTT_RX_RING_SELECTION_CFG_RX_ATTENTION_OFFSET_SET(*msg_word,
					htt_tlv_filter->rx_attn_offset);
		msg_word++;
		*msg_word = 0;
	} else {
		msg_word += 4;
		*msg_word = 0;
	}

	if (mon_drop_th > 0)
		HTT_RX_RING_SELECTION_CFG_RX_DROP_THRESHOLD_SET(*msg_word,
								mon_drop_th);

	/* "response_required" field should be set if a HTT response message is
	 * required after setting up the ring.
	 */
	pkt = htt_htc_pkt_alloc(soc);
	if (!pkt)
		goto fail1;

	pkt->soc_ctxt = NULL; /* not used during send-done callback */

	SET_HTC_PACKET_INFO_TX(
		&pkt->htc_pkt,
		dp_htt_h2t_send_complete_free_netbuf,
		qdf_nbuf_data(htt_msg),
		qdf_nbuf_len(htt_msg),
		soc->htc_endpoint,
		1); /* tag - not relevant here */

	SET_HTC_PACKET_NET_BUF_CONTEXT(&pkt->htc_pkt, htt_msg);
	DP_HTT_SEND_HTC_PKT(soc, pkt, HTT_H2T_MSG_TYPE_RX_RING_SELECTION_CFG,
			    htt_logger_bufp);
	return QDF_STATUS_SUCCESS;

fail1:
	qdf_nbuf_free(htt_msg);
fail0:
	return QDF_STATUS_E_FAILURE;
}

#if defined(HTT_STATS_ENABLE)
static inline QDF_STATUS dp_send_htt_stat_resp(struct htt_stats_context *htt_stats,
					struct dp_soc *soc, qdf_nbuf_t htt_msg)

{
	uint32_t pdev_id;
	uint32_t *msg_word = NULL;
	uint32_t msg_remain_len = 0;

	msg_word = (uint32_t *) qdf_nbuf_data(htt_msg);

	/*COOKIE MSB*/
	pdev_id = *(msg_word + 2) & HTT_PID_BIT_MASK;

	/* stats message length + 16 size of HTT header*/
	msg_remain_len = qdf_min(htt_stats->msg_len + 16,
				(uint32_t)DP_EXT_MSG_LENGTH);

	dp_wdi_event_handler(WDI_EVENT_HTT_STATS, soc,
			msg_word,  msg_remain_len,
			WDI_NO_VAL, pdev_id);

	if (htt_stats->msg_len >= DP_EXT_MSG_LENGTH) {
		htt_stats->msg_len -= DP_EXT_MSG_LENGTH;
	}
	/* Need to be freed here as WDI handler will
	 * make a copy of pkt to send data to application
	 */
	qdf_nbuf_free(htt_msg);
	return QDF_STATUS_SUCCESS;
}
#else
static inline QDF_STATUS
dp_send_htt_stat_resp(struct htt_stats_context *htt_stats,
		      struct dp_soc *soc, qdf_nbuf_t htt_msg)
{
	return QDF_STATUS_E_NOSUPPORT;
}
#endif
/**
 * dp_process_htt_stat_msg(): Process the list of buffers of HTT EXT stats
 * @htt_stats: htt stats info
 *
 * The FW sends the HTT EXT STATS as a stream of T2H messages. Each T2H message
 * contains sub messages which are identified by a TLV header.
 * In this function we will process the stream of T2H messages and read all the
 * TLV contained in the message.
 *
 * THe following cases have been taken care of
 * Case 1: When the tlv_remain_length <= msg_remain_length of HTT MSG buffer
 *		In this case the buffer will contain multiple tlvs.
 * Case 2: When the tlv_remain_length > msg_remain_length of HTT MSG buffer.
 *		Only one tlv will be contained in the HTT message and this tag
 *		will extend onto the next buffer.
 * Case 3: When the buffer is the continuation of the previous message
 * Case 4: tlv length is 0. which will indicate the end of message
 *
 * return: void
 */
static inline void dp_process_htt_stat_msg(struct htt_stats_context *htt_stats,
					struct dp_soc *soc)
{
	htt_tlv_tag_t tlv_type = 0xff;
	qdf_nbuf_t htt_msg = NULL;
	uint32_t *msg_word;
	uint8_t *tlv_buf_head = NULL;
	uint8_t *tlv_buf_tail = NULL;
	uint32_t msg_remain_len = 0;
	uint32_t tlv_remain_len = 0;
	uint32_t *tlv_start;
	int cookie_val;
	int cookie_msb;
	int pdev_id;
	bool copy_stats = false;
	struct dp_pdev *pdev;

	/* Process node in the HTT message queue */
	while ((htt_msg = qdf_nbuf_queue_remove(&htt_stats->msg))
		!= NULL) {
		msg_word = (uint32_t *) qdf_nbuf_data(htt_msg);
		cookie_val = *(msg_word + 1);
		htt_stats->msg_len = HTT_T2H_EXT_STATS_CONF_TLV_LENGTH_GET(
					*(msg_word +
					HTT_T2H_EXT_STATS_TLV_START_OFFSET));

		if (cookie_val) {
			if (dp_send_htt_stat_resp(htt_stats, soc, htt_msg)
					== QDF_STATUS_SUCCESS) {
				continue;
			}
		}

		cookie_msb = *(msg_word + 2);
		pdev_id = *(msg_word + 2) & HTT_PID_BIT_MASK;
		pdev = soc->pdev_list[pdev_id];

		if (cookie_msb >> 2) {
			copy_stats = true;
		}

		/* read 5th word */
		msg_word = msg_word + 4;
		msg_remain_len = qdf_min(htt_stats->msg_len,
				(uint32_t) DP_EXT_MSG_LENGTH);
		/* Keep processing the node till node length is 0 */
		while (msg_remain_len) {
			/*
			 * if message is not a continuation of previous message
			 * read the tlv type and tlv length
			 */
			if (!tlv_buf_head) {
				tlv_type = HTT_STATS_TLV_TAG_GET(
						*msg_word);
				tlv_remain_len = HTT_STATS_TLV_LENGTH_GET(
						*msg_word);
			}

			if (tlv_remain_len == 0) {
				msg_remain_len = 0;

				if (tlv_buf_head) {
					qdf_mem_free(tlv_buf_head);
					tlv_buf_head = NULL;
					tlv_buf_tail = NULL;
				}

				goto error;
			}

			if (!tlv_buf_head)
				tlv_remain_len += HTT_TLV_HDR_LEN;

			if ((tlv_remain_len <= msg_remain_len)) {
				/* Case 3 */
				if (tlv_buf_head) {
					qdf_mem_copy(tlv_buf_tail,
							(uint8_t *)msg_word,
							tlv_remain_len);
					tlv_start = (uint32_t *)tlv_buf_head;
				} else {
					/* Case 1 */
					tlv_start = msg_word;
				}

				if (copy_stats)
					dp_htt_stats_copy_tag(pdev,
							      tlv_type,
							      tlv_start);
				else
					dp_htt_stats_print_tag(pdev,
							       tlv_type,
							       tlv_start);

				if (tlv_type == HTT_STATS_PEER_DETAILS_TAG ||
				    tlv_type == HTT_STATS_PEER_STATS_CMN_TAG)
					dp_peer_update_inactive_time(pdev,
								     tlv_type,
								     tlv_start);

				msg_remain_len -= tlv_remain_len;

				msg_word = (uint32_t *)
					(((uint8_t *)msg_word) +
					tlv_remain_len);

				tlv_remain_len = 0;

				if (tlv_buf_head) {
					qdf_mem_free(tlv_buf_head);
					tlv_buf_head = NULL;
					tlv_buf_tail = NULL;
				}

			} else { /* tlv_remain_len > msg_remain_len */
				/* Case 2 & 3 */
				if (!tlv_buf_head) {
					tlv_buf_head = qdf_mem_malloc(
							tlv_remain_len);

					if (!tlv_buf_head) {
						QDF_TRACE(QDF_MODULE_ID_TXRX,
								QDF_TRACE_LEVEL_ERROR,
								"Alloc failed");
						goto error;
					}

					tlv_buf_tail = tlv_buf_head;
				}

				qdf_mem_copy(tlv_buf_tail, (uint8_t *)msg_word,
						msg_remain_len);
				tlv_remain_len -= msg_remain_len;
				tlv_buf_tail += msg_remain_len;
			}
		}

		if (htt_stats->msg_len >= DP_EXT_MSG_LENGTH) {
			htt_stats->msg_len -= DP_EXT_MSG_LENGTH;
		}

		qdf_nbuf_free(htt_msg);
	}
	return;

error:
	qdf_nbuf_free(htt_msg);
	while ((htt_msg = qdf_nbuf_queue_remove(&htt_stats->msg))
			!= NULL)
		qdf_nbuf_free(htt_msg);
}

void htt_t2h_stats_handler(void *context)
{
	struct dp_soc *soc = (struct dp_soc *)context;
	struct htt_stats_context htt_stats;
	uint32_t *msg_word;
	qdf_nbuf_t htt_msg = NULL;
	uint8_t done;
	uint32_t rem_stats;

	if (!soc || !qdf_atomic_read(&soc->cmn_init_done)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"soc: 0x%pK, init_done: %d", soc,
			qdf_atomic_read(&soc->cmn_init_done));
		return;
	}

	qdf_mem_zero(&htt_stats, sizeof(htt_stats));
	qdf_nbuf_queue_init(&htt_stats.msg);

	/* pull one completed stats from soc->htt_stats_msg and process */
	qdf_spin_lock_bh(&soc->htt_stats.lock);
	if (!soc->htt_stats.num_stats) {
		qdf_spin_unlock_bh(&soc->htt_stats.lock);
		return;
	}
	while ((htt_msg = qdf_nbuf_queue_remove(&soc->htt_stats.msg)) != NULL) {
		msg_word = (uint32_t *) qdf_nbuf_data(htt_msg);
		msg_word = msg_word + HTT_T2H_EXT_STATS_TLV_START_OFFSET;
		done = HTT_T2H_EXT_STATS_CONF_TLV_DONE_GET(*msg_word);
		qdf_nbuf_queue_add(&htt_stats.msg, htt_msg);
		/*
		 * Done bit signifies that this is the last T2H buffer in the
		 * stream of HTT EXT STATS message
		 */
		if (done)
			break;
	}
	rem_stats = --soc->htt_stats.num_stats;
	qdf_spin_unlock_bh(&soc->htt_stats.lock);

	/* If there are more stats to process, schedule stats work again.
	 * Scheduling prior to processing ht_stats to queue with early
	 * index
	 */
	if (rem_stats)
		qdf_sched_work(0, &soc->htt_stats.work);

	dp_process_htt_stat_msg(&htt_stats, soc);
}

/*
 * dp_get_ppdu_info_user_index: Find and allocate a per-user descriptor for a PPDU,
 * if a new peer id arrives in a PPDU
 * pdev: DP pdev handle
 * @peer_id : peer unique identifier
 * @ppdu_info: per ppdu tlv structure
 *
 * return:user index to be populated
 */
#ifdef FEATURE_PERPKT_INFO
static uint8_t dp_get_ppdu_info_user_index(struct dp_pdev *pdev,
						uint16_t peer_id,
						struct ppdu_info *ppdu_info)
{
	uint8_t user_index = 0;
	struct cdp_tx_completion_ppdu *ppdu_desc;
	struct cdp_tx_completion_ppdu_user *ppdu_user_desc;

	ppdu_desc = (struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);

	while ((user_index + 1) <= ppdu_info->last_user) {
		ppdu_user_desc = &ppdu_desc->user[user_index];
		if (ppdu_user_desc->peer_id != peer_id) {
			user_index++;
			continue;
		} else {
			/* Max users possible is 8 so user array index should
			 * not exceed 7
			 */
			qdf_assert_always(user_index <= CDP_MU_MAX_USER_INDEX);
			return user_index;
		}
	}

	ppdu_info->last_user++;
	/* Max users possible is 8 so last user should not exceed 8 */
	qdf_assert_always(ppdu_info->last_user <= CDP_MU_MAX_USERS);
	return ppdu_info->last_user - 1;
}

/*
 * dp_process_ppdu_stats_common_tlv: Process htt_ppdu_stats_common_tlv
 * pdev: DP pdev handle
 * @tag_buf: buffer containing the tlv htt_ppdu_stats_common_tlv
 * @ppdu_info: per ppdu tlv structure
 *
 * return:void
 */
static void dp_process_ppdu_stats_common_tlv(struct dp_pdev *pdev,
		uint32_t *tag_buf, struct ppdu_info *ppdu_info)
{
	uint16_t frame_type;
	uint16_t frame_ctrl;
	uint16_t freq;
	struct dp_soc *soc = NULL;
	struct cdp_tx_completion_ppdu *ppdu_desc = NULL;
	uint64_t ppdu_start_timestamp;
	uint32_t *start_tag_buf;

	start_tag_buf = tag_buf;
	ppdu_desc = (struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);

	tag_buf = start_tag_buf + HTT_GET_STATS_CMN_INDEX(RING_ID_SCH_CMD_ID);
	ppdu_info->sched_cmdid =
		HTT_PPDU_STATS_COMMON_TLV_SCH_CMDID_GET(*tag_buf);
	ppdu_desc->num_users =
		HTT_PPDU_STATS_COMMON_TLV_NUM_USERS_GET(*tag_buf);

	tag_buf = start_tag_buf + HTT_GET_STATS_CMN_INDEX(QTYPE_FRM_TYPE);
	frame_type = HTT_PPDU_STATS_COMMON_TLV_FRM_TYPE_GET(*tag_buf);
	ppdu_desc->htt_frame_type = frame_type;

	frame_ctrl = ppdu_desc->frame_ctrl;

	switch (frame_type) {
	case HTT_STATS_FTYPE_TIDQ_DATA_SU:
	case HTT_STATS_FTYPE_TIDQ_DATA_MU:
	case HTT_STATS_FTYPE_SGEN_QOS_NULL:
		/*
		 * for management packet, frame type come as DATA_SU
		 * need to check frame_ctrl before setting frame_type
		 */
		if (HTT_GET_FRAME_CTRL_TYPE(frame_ctrl) <= FRAME_CTRL_TYPE_CTRL)
			ppdu_desc->frame_type = CDP_PPDU_FTYPE_CTRL;
		else
			ppdu_desc->frame_type = CDP_PPDU_FTYPE_DATA;
	break;
	case HTT_STATS_FTYPE_SGEN_MU_BAR:
	case HTT_STATS_FTYPE_SGEN_BAR:
		ppdu_desc->frame_type = CDP_PPDU_FTYPE_BAR;
		ppdu_desc->bar_ppdu_id = ppdu_info->ppdu_id;
	break;
	default:
		ppdu_desc->frame_type = CDP_PPDU_FTYPE_CTRL;
	break;
	}

	tag_buf = start_tag_buf + HTT_GET_STATS_CMN_INDEX(FES_DUR_US);
	ppdu_desc->tx_duration = *tag_buf;

	tag_buf = start_tag_buf + HTT_GET_STATS_CMN_INDEX(START_TSTMP_L32_US);
	ppdu_desc->ppdu_start_timestamp = *tag_buf;

	tag_buf = start_tag_buf + HTT_GET_STATS_CMN_INDEX(CHAN_MHZ_PHY_MODE);
	freq = HTT_PPDU_STATS_COMMON_TLV_CHAN_MHZ_GET(*tag_buf);
	if (freq != ppdu_desc->channel) {
		soc = pdev->soc;
		ppdu_desc->channel = freq;
		if (soc && soc->cdp_soc.ol_ops->freq_to_channel)
			pdev->operating_channel =
		soc->cdp_soc.ol_ops->freq_to_channel(soc->ctrl_psoc,
						     pdev->pdev_id, freq);
	}

	ppdu_desc->phy_mode = HTT_PPDU_STATS_COMMON_TLV_PHY_MODE_GET(*tag_buf);

	dp_tx_capture_htt_frame_counter(pdev, frame_type);

	tag_buf = start_tag_buf + HTT_GET_STATS_CMN_INDEX(START_TSTMP_U32_US);
	ppdu_start_timestamp = *tag_buf;
	ppdu_desc->ppdu_start_timestamp |= ((ppdu_start_timestamp <<
					     HTT_SHIFT_UPPER_TIMESTAMP) &
					    HTT_MASK_UPPER_TIMESTAMP);

	ppdu_desc->ppdu_end_timestamp = ppdu_desc->ppdu_start_timestamp +
					ppdu_desc->tx_duration;
	/* Ack time stamp is same as end time stamp*/
	ppdu_desc->ack_timestamp = ppdu_desc->ppdu_end_timestamp;

	ppdu_desc->ppdu_end_timestamp = ppdu_desc->ppdu_start_timestamp +
					ppdu_desc->tx_duration;

	ppdu_desc->bar_ppdu_start_timestamp = ppdu_desc->ppdu_start_timestamp;
	ppdu_desc->bar_ppdu_end_timestamp = ppdu_desc->ppdu_end_timestamp;
	ppdu_desc->bar_tx_duration = ppdu_desc->tx_duration;

	/* Ack time stamp is same as end time stamp*/
	ppdu_desc->ack_timestamp = ppdu_desc->ppdu_end_timestamp;
}

/*
 * dp_process_ppdu_stats_user_common_tlv: Process ppdu_stats_user_common
 * @tag_buf: buffer containing the tlv htt_ppdu_stats_user_common_tlv
 * @ppdu_info: per ppdu tlv structure
 *
 * return:void
 */
static void dp_process_ppdu_stats_user_common_tlv(
		struct dp_pdev *pdev, uint32_t *tag_buf,
		struct ppdu_info *ppdu_info)
{
	uint16_t peer_id;
	struct cdp_tx_completion_ppdu *ppdu_desc;
	struct cdp_tx_completion_ppdu_user *ppdu_user_desc;
	uint8_t curr_user_index = 0;
	struct dp_peer *peer;
	struct dp_vdev *vdev;

	ppdu_desc =
		(struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);

	tag_buf++;
	peer_id = HTT_PPDU_STATS_USER_RATE_TLV_SW_PEER_ID_GET(*tag_buf);

	curr_user_index =
		dp_get_ppdu_info_user_index(pdev,
					    peer_id, ppdu_info);
	ppdu_user_desc = &ppdu_desc->user[curr_user_index];

	if (peer_id == DP_SCAN_PEER_ID) {
		ppdu_desc->vdev_id =
			HTT_PPDU_STATS_USER_COMMON_TLV_VAP_ID_GET(*tag_buf);
		vdev =
		       dp_get_vdev_from_soc_vdev_id_wifi3(pdev->soc,
							  ppdu_desc->vdev_id);
		if (!vdev)
			return;
		qdf_mem_copy(ppdu_user_desc->mac_addr, vdev->mac_addr.raw,
			     QDF_MAC_ADDR_SIZE);
	} else {
		peer = dp_peer_find_by_id(pdev->soc, peer_id);
		if (!peer)
			return;
		qdf_mem_copy(ppdu_user_desc->mac_addr,
			     peer->mac_addr.raw, QDF_MAC_ADDR_SIZE);
		dp_peer_unref_del_find_by_id(peer);
	}

	ppdu_user_desc->peer_id = peer_id;

	tag_buf++;

	if (HTT_PPDU_STATS_USER_COMMON_TLV_DELAYED_BA_GET(*tag_buf)) {
		ppdu_user_desc->delayed_ba = 1;
		ppdu_desc->delayed_ba = 1;
	}

	if (HTT_PPDU_STATS_USER_COMMON_TLV_MCAST_GET(*tag_buf)) {
		ppdu_user_desc->is_mcast = true;
		ppdu_user_desc->mpdu_tried_mcast =
		HTT_PPDU_STATS_USER_COMMON_TLV_MPDUS_TRIED_GET(*tag_buf);
		ppdu_user_desc->num_mpdu = ppdu_user_desc->mpdu_tried_mcast;
	} else {
		ppdu_user_desc->mpdu_tried_ucast =
		HTT_PPDU_STATS_USER_COMMON_TLV_MPDUS_TRIED_GET(*tag_buf);
	}

	tag_buf++;

	ppdu_user_desc->qos_ctrl =
		HTT_PPDU_STATS_USER_COMMON_TLV_QOS_CTRL_GET(*tag_buf);
	ppdu_user_desc->frame_ctrl =
		HTT_PPDU_STATS_USER_COMMON_TLV_FRAME_CTRL_GET(*tag_buf);
	ppdu_desc->frame_ctrl = ppdu_user_desc->frame_ctrl;

	if (ppdu_user_desc->delayed_ba)
		ppdu_user_desc->mpdu_success = 0;

	tag_buf += 3;

	if (HTT_PPDU_STATS_IS_OPAQUE_VALID_GET(*tag_buf)) {
		ppdu_user_desc->ppdu_cookie =
			HTT_PPDU_STATS_HOST_OPAQUE_COOKIE_GET(*tag_buf);
		ppdu_user_desc->is_ppdu_cookie_valid = 1;
	}
}


/**
 * dp_process_ppdu_stats_user_rate_tlv() - Process htt_ppdu_stats_user_rate_tlv
 * @pdev: DP pdev handle
 * @tag_buf: T2H message buffer carrying the user rate TLV
 * @ppdu_info: per ppdu tlv structure
 *
 * return:void
 */
static void dp_process_ppdu_stats_user_rate_tlv(struct dp_pdev *pdev,
		uint32_t *tag_buf,
		struct ppdu_info *ppdu_info)
{
	uint16_t peer_id;
	struct dp_peer *peer;
	struct cdp_tx_completion_ppdu *ppdu_desc;
	struct cdp_tx_completion_ppdu_user *ppdu_user_desc;
	uint8_t curr_user_index = 0;
	struct dp_vdev *vdev;

	ppdu_desc = (struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);

	tag_buf++;
	peer_id = HTT_PPDU_STATS_USER_RATE_TLV_SW_PEER_ID_GET(*tag_buf);

	curr_user_index =
		dp_get_ppdu_info_user_index(pdev,
					    peer_id, ppdu_info);
	ppdu_user_desc = &ppdu_desc->user[curr_user_index];
	if (peer_id == DP_SCAN_PEER_ID) {
		vdev =
		       dp_get_vdev_from_soc_vdev_id_wifi3(pdev->soc,
							  ppdu_desc->vdev_id);
		if (!vdev)
			return;
	} else {
		peer = dp_peer_find_by_id(pdev->soc, peer_id);
		if (!peer)
			return;
		dp_peer_unref_del_find_by_id(peer);
	}

	ppdu_user_desc->peer_id = peer_id;

	ppdu_user_desc->tid =
		HTT_PPDU_STATS_USER_RATE_TLV_TID_NUM_GET(*tag_buf);

	tag_buf += 1;

	ppdu_user_desc->user_pos =
		HTT_PPDU_STATS_USER_RATE_TLV_USER_POS_GET(*tag_buf);
	ppdu_user_desc->mu_group_id =
		HTT_PPDU_STATS_USER_RATE_TLV_MU_GROUPID_GET(*tag_buf);

	tag_buf += 1;

	ppdu_user_desc->ru_start =
		HTT_PPDU_STATS_USER_RATE_TLV_RU_START_GET(*tag_buf);
	ppdu_user_desc->ru_tones =
		(HTT_PPDU_STATS_USER_RATE_TLV_RU_END_GET(*tag_buf) -
		HTT_PPDU_STATS_USER_RATE_TLV_RU_START_GET(*tag_buf)) + 1;

	tag_buf += 2;

	ppdu_user_desc->ppdu_type =
		HTT_PPDU_STATS_USER_RATE_TLV_PPDU_TYPE_GET(*tag_buf);

	tag_buf++;
	ppdu_user_desc->tx_rate = *tag_buf;

	ppdu_user_desc->ltf_size =
		HTT_PPDU_STATS_USER_RATE_TLV_LTF_SIZE_GET(*tag_buf);
	ppdu_user_desc->stbc =
		HTT_PPDU_STATS_USER_RATE_TLV_STBC_GET(*tag_buf);
	ppdu_user_desc->he_re =
		HTT_PPDU_STATS_USER_RATE_TLV_HE_RE_GET(*tag_buf);
	ppdu_user_desc->txbf =
		HTT_PPDU_STATS_USER_RATE_TLV_TXBF_GET(*tag_buf);
	ppdu_user_desc->bw =
		HTT_PPDU_STATS_USER_RATE_TLV_BW_GET(*tag_buf) - 2;
	ppdu_user_desc->nss = HTT_PPDU_STATS_USER_RATE_TLV_NSS_GET(*tag_buf);
	ppdu_user_desc->mcs = HTT_PPDU_STATS_USER_RATE_TLV_MCS_GET(*tag_buf);
	ppdu_user_desc->preamble =
		HTT_PPDU_STATS_USER_RATE_TLV_PREAMBLE_GET(*tag_buf);
	ppdu_user_desc->gi = HTT_PPDU_STATS_USER_RATE_TLV_GI_GET(*tag_buf);
	ppdu_user_desc->dcm = HTT_PPDU_STATS_USER_RATE_TLV_DCM_GET(*tag_buf);
	ppdu_user_desc->ldpc = HTT_PPDU_STATS_USER_RATE_TLV_LDPC_GET(*tag_buf);
}

/*
 * dp_process_ppdu_stats_enq_mpdu_bitmap_64_tlv: Process
 * htt_ppdu_stats_enq_mpdu_bitmap_64_tlv
 * pdev: DP PDEV handle
 * @tag_buf: buffer containing the tlv htt_ppdu_stats_enq_mpdu_bitmap_64_tlv
 * @ppdu_info: per ppdu tlv structure
 *
 * return:void
 */
static void dp_process_ppdu_stats_enq_mpdu_bitmap_64_tlv(
		struct dp_pdev *pdev, uint32_t *tag_buf,
		struct ppdu_info *ppdu_info)
{
	htt_ppdu_stats_enq_mpdu_bitmap_64_tlv *dp_stats_buf =
		(htt_ppdu_stats_enq_mpdu_bitmap_64_tlv *)tag_buf;

	struct cdp_tx_completion_ppdu *ppdu_desc;
	struct cdp_tx_completion_ppdu_user *ppdu_user_desc;
	uint8_t curr_user_index = 0;
	uint16_t peer_id;
	uint32_t size = CDP_BA_64_BIT_MAP_SIZE_DWORDS;

	ppdu_desc = (struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);

	tag_buf++;

	peer_id =
	HTT_PPDU_STATS_ENQ_MPDU_BITMAP_TLV_SW_PEER_ID_GET(*tag_buf);

	if (!dp_peer_find_by_id_valid(pdev->soc, peer_id))
		return;

	curr_user_index = dp_get_ppdu_info_user_index(pdev, peer_id, ppdu_info);

	ppdu_user_desc = &ppdu_desc->user[curr_user_index];
	ppdu_user_desc->peer_id = peer_id;

	ppdu_user_desc->start_seq = dp_stats_buf->start_seq;
	qdf_mem_copy(&ppdu_user_desc->enq_bitmap, &dp_stats_buf->enq_bitmap,
		     sizeof(uint32_t) * CDP_BA_64_BIT_MAP_SIZE_DWORDS);

	dp_process_ppdu_stats_update_failed_bitmap(pdev,
						   (void *)ppdu_user_desc,
						   ppdu_info->ppdu_id,
						   size);
}

/*
 * dp_process_ppdu_stats_enq_mpdu_bitmap_256_tlv: Process
 * htt_ppdu_stats_enq_mpdu_bitmap_256_tlv
 * soc: DP SOC handle
 * @tag_buf: buffer containing the tlv htt_ppdu_stats_enq_mpdu_bitmap_256_tlv
 * @ppdu_info: per ppdu tlv structure
 *
 * return:void
 */
static void dp_process_ppdu_stats_enq_mpdu_bitmap_256_tlv(
		struct dp_pdev *pdev, uint32_t *tag_buf,
		struct ppdu_info *ppdu_info)
{
	htt_ppdu_stats_enq_mpdu_bitmap_256_tlv *dp_stats_buf =
		(htt_ppdu_stats_enq_mpdu_bitmap_256_tlv *)tag_buf;

	struct cdp_tx_completion_ppdu *ppdu_desc;
	struct cdp_tx_completion_ppdu_user *ppdu_user_desc;
	uint8_t curr_user_index = 0;
	uint16_t peer_id;
	uint32_t size = CDP_BA_256_BIT_MAP_SIZE_DWORDS;

	ppdu_desc = (struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);

	tag_buf++;

	peer_id =
	HTT_PPDU_STATS_ENQ_MPDU_BITMAP_TLV_SW_PEER_ID_GET(*tag_buf);

	if (!dp_peer_find_by_id_valid(pdev->soc, peer_id))
		return;

	curr_user_index = dp_get_ppdu_info_user_index(pdev, peer_id, ppdu_info);

	ppdu_user_desc = &ppdu_desc->user[curr_user_index];
	ppdu_user_desc->peer_id = peer_id;

	ppdu_user_desc->start_seq = dp_stats_buf->start_seq;
	qdf_mem_copy(&ppdu_user_desc->enq_bitmap, &dp_stats_buf->enq_bitmap,
		     sizeof(uint32_t) * CDP_BA_256_BIT_MAP_SIZE_DWORDS);

	dp_process_ppdu_stats_update_failed_bitmap(pdev,
						   (void *)ppdu_user_desc,
						   ppdu_info->ppdu_id,
						   size);
}

/*
 * dp_process_ppdu_stats_user_cmpltn_common_tlv: Process
 * htt_ppdu_stats_user_cmpltn_common_tlv
 * soc: DP SOC handle
 * @tag_buf: buffer containing the tlv htt_ppdu_stats_user_cmpltn_common_tlv
 * @ppdu_info: per ppdu tlv structure
 *
 * return:void
 */
static void dp_process_ppdu_stats_user_cmpltn_common_tlv(
		struct dp_pdev *pdev, uint32_t *tag_buf,
		struct ppdu_info *ppdu_info)
{
	uint16_t peer_id;
	struct cdp_tx_completion_ppdu *ppdu_desc;
	struct cdp_tx_completion_ppdu_user *ppdu_user_desc;
	uint8_t curr_user_index = 0;
	uint8_t bw_iter;
	htt_ppdu_stats_user_cmpltn_common_tlv *dp_stats_buf =
		(htt_ppdu_stats_user_cmpltn_common_tlv *)tag_buf;

	ppdu_desc = (struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);

	tag_buf++;
	peer_id =
		HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_SW_PEER_ID_GET(*tag_buf);

	if (!dp_peer_find_by_id_valid(pdev->soc, peer_id))
		return;

	curr_user_index = dp_get_ppdu_info_user_index(pdev, peer_id, ppdu_info);
	ppdu_user_desc = &ppdu_desc->user[curr_user_index];
	ppdu_user_desc->peer_id = peer_id;
	ppdu_desc->last_usr_index = curr_user_index;

	ppdu_user_desc->completion_status =
		HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_COMPLETION_STATUS_GET(
				*tag_buf);

	ppdu_user_desc->tid =
		HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_TID_NUM_GET(*tag_buf);


	tag_buf++;
	if (qdf_likely(ppdu_user_desc->completion_status ==
			HTT_PPDU_STATS_USER_STATUS_OK)) {
		ppdu_desc->ack_rssi = dp_stats_buf->ack_rssi;
		ppdu_user_desc->ack_rssi_valid = 1;
	} else {
		ppdu_user_desc->ack_rssi_valid = 0;
	}

	tag_buf++;

	ppdu_user_desc->mpdu_success =
	HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_MPDU_SUCCESS_GET(*tag_buf);

	ppdu_user_desc->mpdu_failed =
	HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_MPDU_TRIED_GET(*tag_buf) -
						ppdu_user_desc->mpdu_success;

	tag_buf++;

	ppdu_user_desc->long_retries =
		HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_LONG_RETRY_GET(*tag_buf);

	ppdu_user_desc->short_retries =
	HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_SHORT_RETRY_GET(*tag_buf);
	ppdu_user_desc->retry_msdus =
		ppdu_user_desc->long_retries + ppdu_user_desc->short_retries;

	ppdu_user_desc->is_ampdu =
		HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_IS_AMPDU_GET(*tag_buf);
	ppdu_info->is_ampdu = ppdu_user_desc->is_ampdu;

	ppdu_desc->resp_type =
		HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_RESP_TYPE_GET(*tag_buf);
	ppdu_desc->mprot_type =
		HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_MPROT_TYPE_GET(*tag_buf);
	ppdu_desc->rts_success =
		HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_RTS_SUCCESS_GET(*tag_buf);
	ppdu_desc->rts_failure =
		HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_RTS_FAILURE_GET(*tag_buf);

	/*
	 * increase successful mpdu counter from
	 * htt_ppdu_stats_user_cmpltn_common_tlv
	 */
	ppdu_info->mpdu_compltn_common_tlv += ppdu_user_desc->mpdu_success;

	/*
	 * MU BAR may send request to n users but we may received ack only from
	 * m users. To have count of number of users respond back, we have a
	 * separate counter bar_num_users per PPDU that get increment for every
	 * htt_ppdu_stats_user_cmpltn_common_tlv
	 */
	ppdu_desc->bar_num_users++;

	tag_buf++;
	for (bw_iter = 0; bw_iter < CDP_RSSI_CHAIN_LEN; bw_iter++) {
		ppdu_user_desc->rssi_chain[bw_iter] =
		HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_CHAIN_RSSI_GET(*tag_buf);
		tag_buf++;
	}

	ppdu_user_desc->sa_tx_antenna =
		HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_TX_ANTENNA_MASK_GET(*tag_buf);

	tag_buf++;
	ppdu_user_desc->sa_is_training =
		HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_IS_TRAINING_GET(*tag_buf);
	if (ppdu_user_desc->sa_is_training) {
		ppdu_user_desc->sa_goodput =
			HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_PENDING_TRAINING_PKTS_GET(*tag_buf);
	}

	tag_buf++;
	for (bw_iter = 0; bw_iter < CDP_NUM_SA_BW; bw_iter++) {
		ppdu_user_desc->sa_max_rates[bw_iter] =
			HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_MAX_RATES_GET(tag_buf[bw_iter]);
	}

	tag_buf += CDP_NUM_SA_BW;
	ppdu_user_desc->current_rate_per =
		HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_CURRENT_RATE_PER_GET(*tag_buf);
}

/*
 * dp_process_ppdu_stats_user_compltn_ba_bitmap_64_tlv: Process
 * htt_ppdu_stats_user_compltn_ba_bitmap_64_tlv
 * pdev: DP PDEV handle
 * @tag_buf: buffer containing the htt_ppdu_stats_user_compltn_ba_bitmap_64_tlv
 * @ppdu_info: per ppdu tlv structure
 *
 * return:void
 */
static void dp_process_ppdu_stats_user_compltn_ba_bitmap_64_tlv(
		struct dp_pdev *pdev, uint32_t *tag_buf,
		struct ppdu_info *ppdu_info)
{
	htt_ppdu_stats_user_compltn_ba_bitmap_64_tlv *dp_stats_buf =
		(htt_ppdu_stats_user_compltn_ba_bitmap_64_tlv *)tag_buf;
	struct cdp_tx_completion_ppdu_user *ppdu_user_desc;
	struct cdp_tx_completion_ppdu *ppdu_desc;
	uint8_t curr_user_index = 0;
	uint16_t peer_id;

	ppdu_desc = (struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);

	tag_buf++;

	peer_id =
	HTT_PPDU_STATS_USER_CMPLTN_BA_BITMAP_TLV_SW_PEER_ID_GET(*tag_buf);

	if (!dp_peer_find_by_id_valid(pdev->soc, peer_id))
		return;

	curr_user_index = dp_get_ppdu_info_user_index(pdev, peer_id, ppdu_info);

	ppdu_user_desc = &ppdu_desc->user[curr_user_index];
	ppdu_user_desc->peer_id = peer_id;

	ppdu_user_desc->ba_seq_no = dp_stats_buf->ba_seq_no;
	qdf_mem_copy(&ppdu_user_desc->ba_bitmap, &dp_stats_buf->ba_bitmap,
		     sizeof(uint32_t) * CDP_BA_64_BIT_MAP_SIZE_DWORDS);
	ppdu_user_desc->ba_size = CDP_BA_64_BIT_MAP_SIZE_DWORDS * 32;
}

/*
 * dp_process_ppdu_stats_user_compltn_ba_bitmap_256_tlv: Process
 * htt_ppdu_stats_user_compltn_ba_bitmap_256_tlv
 * pdev: DP PDEV handle
 * @tag_buf: buffer containing the htt_ppdu_stats_user_compltn_ba_bitmap_256_tlv
 * @ppdu_info: per ppdu tlv structure
 *
 * return:void
 */
static void dp_process_ppdu_stats_user_compltn_ba_bitmap_256_tlv(
		struct dp_pdev *pdev, uint32_t *tag_buf,
		struct ppdu_info *ppdu_info)
{
	htt_ppdu_stats_user_compltn_ba_bitmap_256_tlv *dp_stats_buf =
		(htt_ppdu_stats_user_compltn_ba_bitmap_256_tlv *)tag_buf;
	struct cdp_tx_completion_ppdu_user *ppdu_user_desc;
	struct cdp_tx_completion_ppdu *ppdu_desc;
	uint8_t curr_user_index = 0;
	uint16_t peer_id;

	ppdu_desc = (struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);

	tag_buf++;

	peer_id =
	HTT_PPDU_STATS_USER_CMPLTN_BA_BITMAP_TLV_SW_PEER_ID_GET(*tag_buf);

	if (!dp_peer_find_by_id_valid(pdev->soc, peer_id))
		return;

	curr_user_index = dp_get_ppdu_info_user_index(pdev, peer_id, ppdu_info);

	ppdu_user_desc = &ppdu_desc->user[curr_user_index];
	ppdu_user_desc->peer_id = peer_id;

	ppdu_user_desc->ba_seq_no = dp_stats_buf->ba_seq_no;
	qdf_mem_copy(&ppdu_user_desc->ba_bitmap, &dp_stats_buf->ba_bitmap,
		     sizeof(uint32_t) * CDP_BA_256_BIT_MAP_SIZE_DWORDS);
	ppdu_user_desc->ba_size = CDP_BA_256_BIT_MAP_SIZE_DWORDS * 32;
}

/*
 * dp_process_ppdu_stats_user_compltn_ack_ba_status_tlv: Process
 * htt_ppdu_stats_user_compltn_ack_ba_status_tlv
 * pdev: DP PDE handle
 * @tag_buf: buffer containing the htt_ppdu_stats_user_compltn_ack_ba_status_tlv
 * @ppdu_info: per ppdu tlv structure
 *
 * return:void
 */
static void dp_process_ppdu_stats_user_compltn_ack_ba_status_tlv(
		struct dp_pdev *pdev, uint32_t *tag_buf,
		struct ppdu_info *ppdu_info)
{
	uint16_t peer_id;
	struct cdp_tx_completion_ppdu *ppdu_desc;
	struct cdp_tx_completion_ppdu_user *ppdu_user_desc;
	uint8_t curr_user_index = 0;

	ppdu_desc = (struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);

	tag_buf += 2;
	peer_id =
	HTT_PPDU_STATS_USER_CMPLTN_ACK_BA_STATUS_TLV_SW_PEER_ID_GET(*tag_buf);

	if (!dp_peer_find_by_id_valid(pdev->soc, peer_id))
		return;

	curr_user_index = dp_get_ppdu_info_user_index(pdev, peer_id, ppdu_info);

	ppdu_user_desc = &ppdu_desc->user[curr_user_index];
	ppdu_user_desc->peer_id = peer_id;

	tag_buf++;
	/* not to update ppdu_desc->tid from this TLV */
	ppdu_user_desc->num_mpdu =
		HTT_PPDU_STATS_USER_CMPLTN_ACK_BA_STATUS_TLV_NUM_MPDU_GET(*tag_buf);

	ppdu_user_desc->num_msdu =
		HTT_PPDU_STATS_USER_CMPLTN_ACK_BA_STATUS_TLV_NUM_MSDU_GET(*tag_buf);

	ppdu_user_desc->success_msdus = ppdu_user_desc->num_msdu;

	tag_buf++;
	ppdu_user_desc->start_seq =
		HTT_PPDU_STATS_USER_CMPLTN_ACK_BA_STATUS_TLV_START_SEQ_GET(
			*tag_buf);

	tag_buf++;
	ppdu_user_desc->success_bytes = *tag_buf;

	/* increase successful mpdu counter */
	ppdu_info->mpdu_ack_ba_tlv += ppdu_user_desc->num_mpdu;
}

/*
 * dp_process_ppdu_stats_user_common_array_tlv: Process
 * htt_ppdu_stats_user_common_array_tlv
 * pdev: DP PDEV handle
 * @tag_buf: buffer containing the htt_ppdu_stats_user_compltn_ack_ba_status_tlv
 * @ppdu_info: per ppdu tlv structure
 *
 * return:void
 */
static void dp_process_ppdu_stats_user_common_array_tlv(
		struct dp_pdev *pdev, uint32_t *tag_buf,
		struct ppdu_info *ppdu_info)
{
	uint32_t peer_id;
	struct cdp_tx_completion_ppdu *ppdu_desc;
	struct cdp_tx_completion_ppdu_user *ppdu_user_desc;
	uint8_t curr_user_index = 0;
	struct htt_tx_ppdu_stats_info *dp_stats_buf;

	ppdu_desc = (struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);

	tag_buf++;
	dp_stats_buf = (struct htt_tx_ppdu_stats_info *)tag_buf;
	tag_buf += 3;
	peer_id =
		HTT_PPDU_STATS_ARRAY_ITEM_TLV_PEERID_GET(*tag_buf);

	if (!dp_peer_find_by_id_valid(pdev->soc, peer_id)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"Invalid peer");
		return;
	}

	curr_user_index = dp_get_ppdu_info_user_index(pdev, peer_id, ppdu_info);

	ppdu_user_desc = &ppdu_desc->user[curr_user_index];

	ppdu_user_desc->retry_bytes = dp_stats_buf->tx_retry_bytes;
	ppdu_user_desc->failed_bytes = dp_stats_buf->tx_failed_bytes;

	tag_buf++;

	ppdu_user_desc->success_msdus =
		HTT_PPDU_STATS_ARRAY_ITEM_TLV_TX_SUCC_MSDUS_GET(*tag_buf);
	ppdu_user_desc->retry_bytes =
		HTT_PPDU_STATS_ARRAY_ITEM_TLV_TX_RETRY_MSDUS_GET(*tag_buf);
	tag_buf++;
	ppdu_user_desc->failed_msdus =
		HTT_PPDU_STATS_ARRAY_ITEM_TLV_TX_FAILED_MSDUS_GET(*tag_buf);
}

/*
 * dp_process_ppdu_stats_flush_tlv: Process
 * htt_ppdu_stats_flush_tlv
 * @pdev: DP PDEV handle
 * @tag_buf: buffer containing the htt_ppdu_stats_flush_tlv
 * @ppdu_info: per ppdu tlv structure
 *
 * return:void
 */
static void
dp_process_ppdu_stats_user_compltn_flush_tlv(struct dp_pdev *pdev,
					     uint32_t *tag_buf,
					     struct ppdu_info *ppdu_info)
{
	struct cdp_tx_completion_ppdu *ppdu_desc;
	uint32_t peer_id;
	uint8_t tid;
	struct dp_peer *peer;

	ppdu_desc = (struct cdp_tx_completion_ppdu *)
				qdf_nbuf_data(ppdu_info->nbuf);
	ppdu_desc->is_flush = 1;

	tag_buf++;
	ppdu_desc->drop_reason = *tag_buf;

	tag_buf++;
	ppdu_desc->num_msdu = HTT_PPDU_STATS_FLUSH_TLV_NUM_MSDU_GET(*tag_buf);
	ppdu_desc->num_mpdu = HTT_PPDU_STATS_FLUSH_TLV_NUM_MPDU_GET(*tag_buf);
	ppdu_desc->flow_type = HTT_PPDU_STATS_FLUSH_TLV_FLOW_TYPE_GET(*tag_buf);

	tag_buf++;
	peer_id = HTT_PPDU_STATS_FLUSH_TLV_SW_PEER_ID_GET(*tag_buf);
	tid = HTT_PPDU_STATS_FLUSH_TLV_TID_NUM_GET(*tag_buf);

	ppdu_desc->user[0].peer_id = peer_id;
	ppdu_desc->user[0].tid = tid;

	ppdu_desc->queue_type =
			HTT_PPDU_STATS_FLUSH_TLV_QUEUE_TYPE_GET(*tag_buf);

	peer = dp_peer_find_by_id(pdev->soc, peer_id);
	if (!peer)
		return;

	if (ppdu_desc->drop_reason == HTT_FLUSH_EXCESS_RETRIES) {
		DP_STATS_INC(peer,
			     tx.excess_retries_per_ac[TID_TO_WME_AC(tid)],
			     ppdu_desc->num_msdu);
	}

	dp_peer_unref_del_find_by_id(peer);
}

#ifndef WLAN_TX_PKT_CAPTURE_ENH
/*
 * dp_deliver_mgmt_frm: Process
 * @pdev: DP PDEV handle
 * @nbuf: buffer containing the htt_ppdu_stats_tx_mgmtctrl_payload_tlv
 *
 * return: void
 */
void dp_deliver_mgmt_frm(struct dp_pdev *pdev, qdf_nbuf_t nbuf)
{
	if (pdev->tx_sniffer_enable || pdev->mcopy_mode) {
		dp_wdi_event_handler(WDI_EVENT_TX_MGMT_CTRL, pdev->soc,
				     nbuf, HTT_INVALID_PEER,
				     WDI_NO_VAL, pdev->pdev_id);
	}
}
#endif

/*
 * dp_process_ppdu_stats_tx_mgmtctrl_payload_tlv: Process
 * htt_ppdu_stats_tx_mgmtctrl_payload_tlv
 * @pdev: DP PDEV handle
 * @tag_buf: buffer containing the htt_ppdu_stats_tx_mgmtctrl_payload_tlv
 * @length: tlv_length
 *
 * return:QDF_STATUS_SUCCESS if nbuf as to be freed in caller
 */
static QDF_STATUS
dp_process_ppdu_stats_tx_mgmtctrl_payload_tlv(struct dp_pdev *pdev,
					      qdf_nbuf_t tag_buf,
					      uint32_t ppdu_id)
{
	uint32_t *nbuf_ptr;
	uint8_t trim_size;
	size_t head_size;
	struct cdp_tx_mgmt_comp_info *ptr_mgmt_comp_info;

	if ((!pdev->tx_sniffer_enable) && (!pdev->mcopy_mode) &&
	    (!pdev->bpr_enable) && (!pdev->tx_capture_enabled))
		return QDF_STATUS_SUCCESS;

	trim_size = ((pdev->mgmtctrl_frm_info.mgmt_buf +
		      HTT_MGMT_CTRL_TLV_HDR_RESERVERD_LEN) -
		      qdf_nbuf_data(tag_buf));

	if (!qdf_nbuf_pull_head(tag_buf, trim_size))
		return QDF_STATUS_SUCCESS;

	qdf_nbuf_trim_tail(tag_buf, qdf_nbuf_len(tag_buf) -
			    pdev->mgmtctrl_frm_info.mgmt_buf_len);

	if (pdev->tx_capture_enabled) {
		head_size = sizeof(struct cdp_tx_mgmt_comp_info);
		if (qdf_unlikely(qdf_nbuf_headroom(tag_buf) < head_size)) {
			qdf_err("Fail to get headroom h_sz %d h_avail %d\n",
				head_size, qdf_nbuf_headroom(tag_buf));
			qdf_assert_always(0);
			return QDF_STATUS_E_NOMEM;
		}
		ptr_mgmt_comp_info = (struct cdp_tx_mgmt_comp_info *)
					qdf_nbuf_push_head(tag_buf, head_size);
		qdf_assert_always(ptr_mgmt_comp_info);
		ptr_mgmt_comp_info->ppdu_id = ppdu_id;
		ptr_mgmt_comp_info->is_sgen_pkt = true;
	} else {
		head_size = sizeof(ppdu_id);
		nbuf_ptr = (uint32_t *)qdf_nbuf_push_head(tag_buf, head_size);
		*nbuf_ptr = ppdu_id;
	}

	if (pdev->bpr_enable) {
		dp_wdi_event_handler(WDI_EVENT_TX_BEACON, pdev->soc,
				     tag_buf, HTT_INVALID_PEER,
				     WDI_NO_VAL, pdev->pdev_id);
	}

	dp_deliver_mgmt_frm(pdev, tag_buf);

	return QDF_STATUS_E_ALREADY;
}

/**
 * dp_validate_fix_ppdu_tlv(): Function to validate the length of PPDU
 *
 * If the TLV length sent as part of PPDU TLV is less that expected size i.e
 * size of corresponding data structure, pad the remaining bytes with zeros
 * and continue processing the TLVs
 *
 * @pdev: DP pdev handle
 * @tag_buf: TLV buffer
 * @tlv_expected_size: Expected size of Tag
 * @tlv_len: TLV length received from FW
 *
 * Return: Pointer to updated TLV
 */
static inline uint32_t *dp_validate_fix_ppdu_tlv(struct dp_pdev *pdev,
						 uint32_t *tag_buf,
						 uint16_t tlv_expected_size,
						 uint16_t tlv_len)
{
	uint32_t *tlv_desc = tag_buf;

	qdf_assert_always(tlv_len != 0);

	if (tlv_len < tlv_expected_size) {
		qdf_mem_zero(pdev->ppdu_tlv_buf, tlv_expected_size);
		qdf_mem_copy(pdev->ppdu_tlv_buf, tag_buf, tlv_len);
		tlv_desc = pdev->ppdu_tlv_buf;
	}

	return tlv_desc;
}

/**
 * dp_process_ppdu_tag(): Function to process the PPDU TLVs
 * @pdev: DP pdev handle
 * @tag_buf: TLV buffer
 * @tlv_len: length of tlv
 * @ppdu_info: per ppdu tlv structure
 *
 * return: void
 */
static void dp_process_ppdu_tag(struct dp_pdev *pdev, uint32_t *tag_buf,
		uint32_t tlv_len, struct ppdu_info *ppdu_info)
{
	uint32_t tlv_type = HTT_STATS_TLV_TAG_GET(*tag_buf);
	uint16_t tlv_expected_size;
	uint32_t *tlv_desc;

	switch (tlv_type) {
	case HTT_PPDU_STATS_COMMON_TLV:
		tlv_expected_size = sizeof(htt_ppdu_stats_common_tlv);
		tlv_desc = dp_validate_fix_ppdu_tlv(pdev, tag_buf,
						    tlv_expected_size, tlv_len);
		dp_process_ppdu_stats_common_tlv(pdev, tlv_desc, ppdu_info);
		break;
	case HTT_PPDU_STATS_USR_COMMON_TLV:
		tlv_expected_size = sizeof(htt_ppdu_stats_user_common_tlv);
		tlv_desc = dp_validate_fix_ppdu_tlv(pdev, tag_buf,
						    tlv_expected_size, tlv_len);
		dp_process_ppdu_stats_user_common_tlv(pdev, tlv_desc,
						      ppdu_info);
		break;
	case HTT_PPDU_STATS_USR_RATE_TLV:
		tlv_expected_size = sizeof(htt_ppdu_stats_user_rate_tlv);
		tlv_desc = dp_validate_fix_ppdu_tlv(pdev, tag_buf,
						    tlv_expected_size, tlv_len);
		dp_process_ppdu_stats_user_rate_tlv(pdev, tlv_desc,
						    ppdu_info);
		break;
	case HTT_PPDU_STATS_USR_MPDU_ENQ_BITMAP_64_TLV:
		tlv_expected_size =
			sizeof(htt_ppdu_stats_enq_mpdu_bitmap_64_tlv);
		tlv_desc = dp_validate_fix_ppdu_tlv(pdev, tag_buf,
						    tlv_expected_size, tlv_len);
		dp_process_ppdu_stats_enq_mpdu_bitmap_64_tlv(
				pdev, tlv_desc, ppdu_info);
		break;
	case HTT_PPDU_STATS_USR_MPDU_ENQ_BITMAP_256_TLV:
		tlv_expected_size =
			sizeof(htt_ppdu_stats_enq_mpdu_bitmap_256_tlv);
		tlv_desc = dp_validate_fix_ppdu_tlv(pdev, tag_buf,
						    tlv_expected_size, tlv_len);
		dp_process_ppdu_stats_enq_mpdu_bitmap_256_tlv(
				pdev, tlv_desc, ppdu_info);
		break;
	case HTT_PPDU_STATS_USR_COMPLTN_COMMON_TLV:
		tlv_expected_size =
			sizeof(htt_ppdu_stats_user_cmpltn_common_tlv);
		tlv_desc = dp_validate_fix_ppdu_tlv(pdev, tag_buf,
						    tlv_expected_size, tlv_len);
		dp_process_ppdu_stats_user_cmpltn_common_tlv(
				pdev, tlv_desc, ppdu_info);
		break;
	case HTT_PPDU_STATS_USR_COMPLTN_BA_BITMAP_64_TLV:
		tlv_expected_size =
			sizeof(htt_ppdu_stats_user_compltn_ba_bitmap_64_tlv);
		tlv_desc = dp_validate_fix_ppdu_tlv(pdev, tag_buf,
						    tlv_expected_size, tlv_len);
		dp_process_ppdu_stats_user_compltn_ba_bitmap_64_tlv(
				pdev, tlv_desc, ppdu_info);
		break;
	case HTT_PPDU_STATS_USR_COMPLTN_BA_BITMAP_256_TLV:
		tlv_expected_size =
			sizeof(htt_ppdu_stats_user_compltn_ba_bitmap_256_tlv);
		tlv_desc = dp_validate_fix_ppdu_tlv(pdev, tag_buf,
						    tlv_expected_size, tlv_len);
		dp_process_ppdu_stats_user_compltn_ba_bitmap_256_tlv(
				pdev, tlv_desc, ppdu_info);
		break;
	case HTT_PPDU_STATS_USR_COMPLTN_ACK_BA_STATUS_TLV:
		tlv_expected_size =
			sizeof(htt_ppdu_stats_user_compltn_ack_ba_status_tlv);
		tlv_desc = dp_validate_fix_ppdu_tlv(pdev, tag_buf,
						    tlv_expected_size, tlv_len);
		dp_process_ppdu_stats_user_compltn_ack_ba_status_tlv(
				pdev, tlv_desc, ppdu_info);
		break;
	case HTT_PPDU_STATS_USR_COMMON_ARRAY_TLV:
		tlv_expected_size =
			sizeof(htt_ppdu_stats_usr_common_array_tlv_v);
		tlv_desc = dp_validate_fix_ppdu_tlv(pdev, tag_buf,
						    tlv_expected_size, tlv_len);
		dp_process_ppdu_stats_user_common_array_tlv(
				pdev, tlv_desc, ppdu_info);
		break;
	case HTT_PPDU_STATS_USR_COMPLTN_FLUSH_TLV:
		tlv_expected_size = sizeof(htt_ppdu_stats_flush_tlv);
		tlv_desc = dp_validate_fix_ppdu_tlv(pdev, tag_buf,
						    tlv_expected_size, tlv_len);
		dp_process_ppdu_stats_user_compltn_flush_tlv(pdev, tlv_desc,
							     ppdu_info);
		break;
	default:
		break;
	}
}

/**
 * dp_ppdu_desc_user_stats_update(): Function to update TX user stats
 * @pdev: DP pdev handle
 * @ppdu_info: per PPDU TLV descriptor
 *
 * return: void
 */
void
dp_ppdu_desc_user_stats_update(struct dp_pdev *pdev,
			       struct ppdu_info *ppdu_info)
{
	struct cdp_tx_completion_ppdu *ppdu_desc = NULL;
	struct dp_peer *peer = NULL;
	uint32_t tlv_bitmap_expected;
	uint32_t tlv_bitmap_default;
	uint16_t i;
	uint32_t num_users;

	ppdu_desc = (struct cdp_tx_completion_ppdu *)
		qdf_nbuf_data(ppdu_info->nbuf);

	ppdu_desc->num_users = ppdu_info->last_user;
	ppdu_desc->ppdu_id = ppdu_info->ppdu_id;

	tlv_bitmap_expected = HTT_PPDU_DEFAULT_TLV_BITMAP;
	if (pdev->tx_sniffer_enable || pdev->mcopy_mode ||
	    pdev->tx_capture_enabled) {
		if (ppdu_info->is_ampdu)
			tlv_bitmap_expected =
				dp_htt_get_ppdu_sniffer_ampdu_tlv_bitmap(
					ppdu_info->tlv_bitmap);
	}

	tlv_bitmap_default = tlv_bitmap_expected;

	if (ppdu_desc->frame_type == CDP_PPDU_FTYPE_BAR) {
		num_users = ppdu_desc->bar_num_users;
		ppdu_desc->num_users = ppdu_desc->bar_num_users;
	} else {
		num_users = ppdu_desc->num_users;
	}

	for (i = 0; i < num_users; i++) {
		ppdu_desc->num_mpdu += ppdu_desc->user[i].num_mpdu;
		ppdu_desc->num_msdu += ppdu_desc->user[i].num_msdu;

		peer = dp_peer_find_by_id(pdev->soc,
					  ppdu_desc->user[i].peer_id);
		/**
		 * This check is to make sure peer is not deleted
		 * after processing the TLVs.
		 */
		if (!peer)
			continue;

		ppdu_desc->user[i].cookie = (void *)peer->wlanstats_ctx;

		/*
		 * different frame like DATA, BAR or CTRL has different
		 * tlv bitmap expected. Apart from ACK_BA_STATUS TLV, we
		 * receive other tlv in-order/sequential from fw.
		 * Since ACK_BA_STATUS TLV come from Hardware it is
		 * asynchronous So we need to depend on some tlv to confirm
		 * all tlv is received for a ppdu.
		 * So we depend on both HTT_PPDU_STATS_COMMON_TLV and
		 * ACK_BA_STATUS_TLV. for failure packet we won't get
		 * ACK_BA_STATUS_TLV.
		 */
		if (!(ppdu_info->tlv_bitmap &
		      (1 << HTT_PPDU_STATS_COMMON_TLV)) ||
		    (!(ppdu_info->tlv_bitmap &
		       (1 << HTT_PPDU_STATS_USR_COMPLTN_ACK_BA_STATUS_TLV)) &&
		     (ppdu_desc->user[i].completion_status ==
		      HTT_PPDU_STATS_USER_STATUS_OK))) {
			dp_peer_unref_del_find_by_id(peer);
			continue;
		}

		/**
		 * Update tx stats for data frames having Qos as well as
		 * non-Qos data tid
		 */

		if ((ppdu_desc->user[i].tid < CDP_DATA_TID_MAX ||
		     (ppdu_desc->user[i].tid == CDP_DATA_NON_QOS_TID) ||
		     (ppdu_desc->htt_frame_type ==
		      HTT_STATS_FTYPE_SGEN_QOS_NULL)) &&
		      (ppdu_desc->frame_type != CDP_PPDU_FTYPE_CTRL)) {

			dp_tx_stats_update(pdev, peer,
					   &ppdu_desc->user[i],
					   ppdu_desc->ack_rssi);
			dp_tx_rate_stats_update(peer, &ppdu_desc->user[i]);
		}

		dp_peer_unref_del_find_by_id(peer);
		tlv_bitmap_expected = tlv_bitmap_default;
	}
}

#ifndef WLAN_TX_PKT_CAPTURE_ENH

/**
 * dp_ppdu_desc_deliver(): Function to deliver Tx PPDU status descriptor
 * to upper layer
 * @pdev: DP pdev handle
 * @ppdu_info: per PPDU TLV descriptor
 *
 * return: void
 */
static
void dp_ppdu_desc_deliver(struct dp_pdev *pdev,
			  struct ppdu_info *ppdu_info)
{
	struct cdp_tx_completion_ppdu *ppdu_desc = NULL;
	qdf_nbuf_t nbuf;

	ppdu_desc = (struct cdp_tx_completion_ppdu *)
		qdf_nbuf_data(ppdu_info->nbuf);

	dp_ppdu_desc_user_stats_update(pdev, ppdu_info);

	/*
	 * Remove from the list
	 */
	TAILQ_REMOVE(&pdev->ppdu_info_list, ppdu_info, ppdu_info_list_elem);
	nbuf = ppdu_info->nbuf;
	pdev->list_depth--;
	qdf_mem_free(ppdu_info);

	qdf_assert_always(nbuf);

	ppdu_desc = (struct cdp_tx_completion_ppdu *)
		qdf_nbuf_data(nbuf);

	/**
	 * Deliver PPDU stats only for valid (acked) data frames if
	 * sniffer mode is not enabled.
	 * If sniffer mode is enabled, PPDU stats for all frames
	 * including mgmt/control frames should be delivered to upper layer
	 */
	if (pdev->tx_sniffer_enable || pdev->mcopy_mode) {
		dp_wdi_event_handler(WDI_EVENT_TX_PPDU_DESC, pdev->soc,
				nbuf, HTT_INVALID_PEER,
				WDI_NO_VAL, pdev->pdev_id);
	} else {
		if (ppdu_desc->num_mpdu != 0 && ppdu_desc->num_users != 0 &&
				ppdu_desc->frame_ctrl & HTT_FRAMECTRL_DATATYPE) {

			dp_wdi_event_handler(WDI_EVENT_TX_PPDU_DESC,
					pdev->soc, nbuf, HTT_INVALID_PEER,
					WDI_NO_VAL, pdev->pdev_id);
		} else
			qdf_nbuf_free(nbuf);
	}
	return;
}

#endif

/**
 * dp_get_ppdu_desc(): Function to allocate new PPDU status
 * desc for new ppdu id
 * @pdev: DP pdev handle
 * @ppdu_id: PPDU unique identifier
 * @tlv_type: TLV type received
 *
 * return: ppdu_info per ppdu tlv structure
 */
static
struct ppdu_info *dp_get_ppdu_desc(struct dp_pdev *pdev, uint32_t ppdu_id,
			uint8_t tlv_type)
{
	struct ppdu_info *ppdu_info = NULL;

	/*
	 * Find ppdu_id node exists or not
	 */
	TAILQ_FOREACH(ppdu_info, &pdev->ppdu_info_list, ppdu_info_list_elem) {

		if (ppdu_info && (ppdu_info->ppdu_id == ppdu_id)) {
			break;
		}
	}

	if (ppdu_info) {
		if (ppdu_info->tlv_bitmap & (1 << tlv_type)) {
			/**
			 * if we get tlv_type that is already been processed
			 * for ppdu, that means we got a new ppdu with same
			 * ppdu id. Hence Flush the older ppdu
			 * for MUMIMO and OFDMA, In a PPDU we have
			 * multiple user with same tlv types. tlv bitmap is
			 * used to check whether SU or MU_MIMO/OFDMA
			 */
			if (!(ppdu_info->tlv_bitmap &
			    (1 << HTT_PPDU_STATS_SCH_CMD_STATUS_TLV)))
				return ppdu_info;

			/**
			 * apart from ACK BA STATUS TLV rest all comes in order
			 * so if tlv type not ACK BA STATUS TLV we can deliver
			 * ppdu_info
			 */
			if (tlv_type ==
			    HTT_PPDU_STATS_USR_COMPLTN_ACK_BA_STATUS_TLV)
				return ppdu_info;

			dp_ppdu_desc_deliver(pdev, ppdu_info);
		} else {
			return ppdu_info;
		}
	}

	/**
	 * Flush the head ppdu descriptor if ppdu desc list reaches max
	 * threshold
	 */
	if (pdev->list_depth > HTT_PPDU_DESC_MAX_DEPTH) {
		ppdu_info = TAILQ_FIRST(&pdev->ppdu_info_list);
		dp_ppdu_desc_deliver(pdev, ppdu_info);
	}

	/*
	 * Allocate new ppdu_info node
	 */
	ppdu_info = qdf_mem_malloc(sizeof(struct ppdu_info));
	if (!ppdu_info)
		return NULL;

	ppdu_info->nbuf = qdf_nbuf_alloc(pdev->soc->osdev,
			sizeof(struct cdp_tx_completion_ppdu), 0, 4,
			TRUE);
	if (!ppdu_info->nbuf) {
		qdf_mem_free(ppdu_info);
		return NULL;
	}

	ppdu_info->ppdu_desc =
		(struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);
	qdf_mem_zero(qdf_nbuf_data(ppdu_info->nbuf),
			sizeof(struct cdp_tx_completion_ppdu));

	if (qdf_nbuf_put_tail(ppdu_info->nbuf,
			sizeof(struct cdp_tx_completion_ppdu)) == NULL) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"No tailroom for HTT PPDU");
		qdf_nbuf_free(ppdu_info->nbuf);
		ppdu_info->nbuf = NULL;
		ppdu_info->last_user = 0;
		qdf_mem_free(ppdu_info);
		return NULL;
	}

	/**
	 * No lock is needed because all PPDU TLVs are processed in
	 * same context and this list is updated in same context
	 */
	TAILQ_INSERT_TAIL(&pdev->ppdu_info_list, ppdu_info,
			ppdu_info_list_elem);
	pdev->list_depth++;
	return ppdu_info;
}

/**
 * dp_htt_process_tlv(): Function to process each PPDU TLVs
 * @pdev: DP pdev handle
 * @htt_t2h_msg: HTT target to host message
 *
 * return: ppdu_info per ppdu tlv structure
 */

static struct ppdu_info *dp_htt_process_tlv(struct dp_pdev *pdev,
		qdf_nbuf_t htt_t2h_msg)
{
	uint32_t length;
	uint32_t ppdu_id;
	uint8_t tlv_type;
	uint32_t tlv_length, tlv_bitmap_expected;
	uint8_t *tlv_buf;
	struct ppdu_info *ppdu_info = NULL;
	struct cdp_tx_completion_ppdu *ppdu_desc = NULL;
	struct dp_peer *peer;
	uint32_t i = 0;

	uint32_t *msg_word = (uint32_t *) qdf_nbuf_data(htt_t2h_msg);

	length = HTT_T2H_PPDU_STATS_PAYLOAD_SIZE_GET(*msg_word);

	msg_word = msg_word + 1;
	ppdu_id = HTT_T2H_PPDU_STATS_PPDU_ID_GET(*msg_word);


	msg_word = msg_word + 3;
	while (length > 0) {
		tlv_buf = (uint8_t *)msg_word;
		tlv_type = HTT_STATS_TLV_TAG_GET(*msg_word);
		tlv_length = HTT_STATS_TLV_LENGTH_GET(*msg_word);
		if (qdf_likely(tlv_type < CDP_PPDU_STATS_MAX_TAG))
			pdev->stats.ppdu_stats_counter[tlv_type]++;

		if (tlv_length == 0)
			break;

		tlv_length += HTT_TLV_HDR_LEN;

		/**
		 * Not allocating separate ppdu descriptor for MGMT Payload
		 * TLV as this is sent as separate WDI indication and it
		 * doesn't contain any ppdu information
		 */
		if (tlv_type == HTT_PPDU_STATS_TX_MGMTCTRL_PAYLOAD_TLV) {
			pdev->mgmtctrl_frm_info.mgmt_buf = tlv_buf;
			pdev->mgmtctrl_frm_info.ppdu_id = ppdu_id;
			pdev->mgmtctrl_frm_info.mgmt_buf_len =
				HTT_PPDU_STATS_TX_MGMTCTRL_TLV_FRAME_LENGTH_GET
						(*(msg_word + 1));
			msg_word =
				(uint32_t *)((uint8_t *)tlv_buf + tlv_length);
			length -= (tlv_length);
			continue;
		}

		ppdu_info = dp_get_ppdu_desc(pdev, ppdu_id, tlv_type);
		if (!ppdu_info)
			return NULL;
		ppdu_info->ppdu_id = ppdu_id;
		ppdu_info->tlv_bitmap |= (1 << tlv_type);

		dp_process_ppdu_tag(pdev, msg_word, tlv_length, ppdu_info);

		/**
		 * Increment pdev level tlv count to monitor
		 * missing TLVs
		 */
		pdev->tlv_count++;
		ppdu_info->last_tlv_cnt = pdev->tlv_count;
		msg_word = (uint32_t *)((uint8_t *)tlv_buf + tlv_length);
		length -= (tlv_length);
	}

	if (!ppdu_info)
		return NULL;

	pdev->last_ppdu_id = ppdu_id;

	tlv_bitmap_expected = HTT_PPDU_DEFAULT_TLV_BITMAP;

	if (pdev->tx_sniffer_enable || pdev->mcopy_mode ||
	    pdev->tx_capture_enabled) {
		if (ppdu_info->is_ampdu)
			tlv_bitmap_expected =
				dp_htt_get_ppdu_sniffer_ampdu_tlv_bitmap(
					ppdu_info->tlv_bitmap);
	}

	ppdu_desc = ppdu_info->ppdu_desc;

	if (!ppdu_desc)
		return NULL;

	if (ppdu_desc->user[ppdu_desc->last_usr_index].completion_status !=
	    HTT_PPDU_STATS_USER_STATUS_OK) {
		tlv_bitmap_expected = tlv_bitmap_expected & 0xFF;
	}

	if (ppdu_desc->frame_type == CDP_PPDU_FTYPE_DATA &&
	    (ppdu_info->tlv_bitmap & (1 << HTT_PPDU_STATS_COMMON_TLV)) &&
	    ppdu_desc->delayed_ba) {
		for (i = 0; i < ppdu_desc->num_users; i++) {
			uint32_t ppdu_id;

			ppdu_id = ppdu_desc->ppdu_id;
			peer = dp_peer_find_by_id(pdev->soc,
						  ppdu_desc->user[i].peer_id);
			/**
			 * This check is to make sure peer is not deleted
			 * after processing the TLVs.
			 */
			if (!peer)
				continue;

			/**
			 * save delayed ba user info
			 */
			if (ppdu_desc->user[i].delayed_ba) {
				dp_peer_copy_delay_stats(peer,
							 &ppdu_desc->user[i]);
				peer->last_delayed_ba_ppduid = ppdu_id;
			}
			dp_peer_unref_del_find_by_id(peer);
		}
	}

	/*
	 * when frame type is BAR and STATS_COMMON_TLV is set
	 * copy the store peer delayed info to BAR status
	 */
	if (ppdu_desc->frame_type == CDP_PPDU_FTYPE_BAR &&
	    (ppdu_info->tlv_bitmap & (1 << HTT_PPDU_STATS_COMMON_TLV))) {
		for (i = 0; i < ppdu_desc->bar_num_users; i++) {
			peer = dp_peer_find_by_id(pdev->soc,
						  ppdu_desc->user[i].peer_id);
			/**
			 * This check is to make sure peer is not deleted
			 * after processing the TLVs.
			 */
			if (!peer)
				continue;

			if (peer->last_delayed_ba) {
				dp_peer_copy_stats_to_bar(peer,
							  &ppdu_desc->user[i]);
				ppdu_desc->bar_ppdu_id = ppdu_desc->ppdu_id;
				ppdu_desc->ppdu_id =
					peer->last_delayed_ba_ppduid;
			}
			dp_peer_unref_del_find_by_id(peer);
		}
	}

	/*
	 * for frame type DATA and BAR, we update stats based on MSDU,
	 * successful msdu and mpdu are populate from ACK BA STATUS TLV
	 * which comes out of order. successful mpdu also populated from
	 * COMPLTN COMMON TLV which comes in order. for every ppdu_info
	 * we store successful mpdu from both tlv and compare before delivering
	 * to make sure we received ACK BA STATUS TLV. For some self generated
	 * frame we won't get ack ba status tlv so no need to wait for
	 * ack ba status tlv.
	 */
	if (ppdu_desc->frame_type != CDP_PPDU_FTYPE_CTRL &&
	    ppdu_desc->htt_frame_type != HTT_STATS_FTYPE_SGEN_QOS_NULL) {
		/*
		 * successful mpdu count should match with both tlv
		 */
		if (ppdu_info->mpdu_compltn_common_tlv !=
		    ppdu_info->mpdu_ack_ba_tlv)
			return NULL;
	}

	/**
	 * Once all the TLVs for a given PPDU has been processed,
	 * return PPDU status to be delivered to higher layer.
	 * tlv_bitmap_expected can't be available for different frame type.
	 * But STATS COMMON TLV is the last TLV from the FW for a ppdu.
	 * apart from ACK BA TLV, FW sends other TLV in sequential order.
	 * flush tlv comes separate.
	 */
	if ((ppdu_info->tlv_bitmap != 0 &&
	     (ppdu_info->tlv_bitmap & (1 << HTT_PPDU_STATS_COMMON_TLV))) ||
	    (ppdu_info->tlv_bitmap &
	     (1 << HTT_PPDU_STATS_USR_COMPLTN_FLUSH_TLV)))
		return ppdu_info;

	return NULL;
}
#endif /* FEATURE_PERPKT_INFO */

/**
 * dp_txrx_ppdu_stats_handler() - Function to process HTT PPDU stats from FW
 * @soc: DP SOC handle
 * @pdev_id: pdev id
 * @htt_t2h_msg: HTT message nbuf
 *
 * return:void
 */
#if defined(WDI_EVENT_ENABLE)
#ifdef FEATURE_PERPKT_INFO
static bool dp_txrx_ppdu_stats_handler(struct dp_soc *soc,
				       uint8_t pdev_id, qdf_nbuf_t htt_t2h_msg)
{
	struct dp_pdev *pdev = soc->pdev_list[pdev_id];
	struct ppdu_info *ppdu_info = NULL;
	bool free_buf = true;

	if (!pdev)
		return true;

	if (!pdev->enhanced_stats_en && !pdev->tx_sniffer_enable &&
	    !pdev->mcopy_mode && !pdev->bpr_enable)
		return free_buf;

	ppdu_info = dp_htt_process_tlv(pdev, htt_t2h_msg);

	if (pdev->mgmtctrl_frm_info.mgmt_buf) {
		if (dp_process_ppdu_stats_tx_mgmtctrl_payload_tlv
		    (pdev, htt_t2h_msg, pdev->mgmtctrl_frm_info.ppdu_id) !=
		    QDF_STATUS_SUCCESS)
			free_buf = false;
	}

	if (ppdu_info)
		dp_ppdu_desc_deliver(pdev, ppdu_info);

	pdev->mgmtctrl_frm_info.mgmt_buf = NULL;
	pdev->mgmtctrl_frm_info.mgmt_buf_len = 0;
	pdev->mgmtctrl_frm_info.ppdu_id = 0;

	return free_buf;
}
#else
static bool dp_txrx_ppdu_stats_handler(struct dp_soc *soc,
				       uint8_t pdev_id, qdf_nbuf_t htt_t2h_msg)
{
	return true;
}
#endif
#endif

/**
 * dp_txrx_fw_stats_handler() - Function to process HTT EXT stats
 * @soc: DP SOC handle
 * @htt_t2h_msg: HTT message nbuf
 *
 * return:void
 */
static inline void dp_txrx_fw_stats_handler(struct dp_soc *soc,
		qdf_nbuf_t htt_t2h_msg)
{
	uint8_t done;
	qdf_nbuf_t msg_copy;
	uint32_t *msg_word;

	msg_word = (uint32_t *) qdf_nbuf_data(htt_t2h_msg);
	msg_word = msg_word + 3;
	done = HTT_T2H_EXT_STATS_CONF_TLV_DONE_GET(*msg_word);

	/*
	 * HTT EXT stats response comes as stream of TLVs which span over
	 * multiple T2H messages.
	 * The first message will carry length of the response.
	 * For rest of the messages length will be zero.
	 *
	 * Clone the T2H message buffer and store it in a list to process
	 * it later.
	 *
	 * The original T2H message buffers gets freed in the T2H HTT event
	 * handler
	 */
	msg_copy = qdf_nbuf_clone(htt_t2h_msg);

	if (!msg_copy) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO,
				"T2H messge clone failed for HTT EXT STATS");
		goto error;
	}

	qdf_spin_lock_bh(&soc->htt_stats.lock);
	qdf_nbuf_queue_add(&soc->htt_stats.msg, msg_copy);
	/*
	 * Done bit signifies that this is the last T2H buffer in the stream of
	 * HTT EXT STATS message
	 */
	if (done) {
		soc->htt_stats.num_stats++;
		qdf_sched_work(0, &soc->htt_stats.work);
	}
	qdf_spin_unlock_bh(&soc->htt_stats.lock);

	return;

error:
	qdf_spin_lock_bh(&soc->htt_stats.lock);
	while ((msg_copy = qdf_nbuf_queue_remove(&soc->htt_stats.msg))
			!= NULL) {
		qdf_nbuf_free(msg_copy);
	}
	soc->htt_stats.num_stats = 0;
	qdf_spin_unlock_bh(&soc->htt_stats.lock);
	return;

}

/*
 * htt_soc_attach_target() - SOC level HTT setup
 * @htt_soc:	HTT SOC handle
 *
 * Return: 0 on success; error code on failure
 */
int htt_soc_attach_target(struct htt_soc *htt_soc)
{
	struct htt_soc *soc = (struct htt_soc *)htt_soc;

	return htt_h2t_ver_req_msg(soc);
}

void htt_set_htc_handle(struct htt_soc *htt_soc, HTC_HANDLE htc_soc)
{
	htt_soc->htc_soc = htc_soc;
}

HTC_HANDLE htt_get_htc_handle(struct htt_soc *htt_soc)
{
	return htt_soc->htc_soc;
}

struct htt_soc *htt_soc_attach(struct dp_soc *soc, HTC_HANDLE htc_handle)
{
	int i;
	int j;
	int alloc_size = HTT_SW_UMAC_RING_IDX_MAX * sizeof(unsigned long);
	struct htt_soc *htt_soc = NULL;

	htt_soc = qdf_mem_malloc(sizeof(*htt_soc));
	if (!htt_soc) {
		dp_err("HTT attach failed");
		return NULL;
	}

	for (i = 0; i < MAX_PDEV_CNT; i++) {
		htt_soc->pdevid_tt[i].umac_ttt = qdf_mem_malloc(alloc_size);
		if (!htt_soc->pdevid_tt[i].umac_ttt)
			break;
		qdf_mem_set(htt_soc->pdevid_tt[i].umac_ttt, alloc_size, -1);
		htt_soc->pdevid_tt[i].lmac_ttt = qdf_mem_malloc(alloc_size);
		if (!htt_soc->pdevid_tt[i].lmac_ttt) {
			qdf_mem_free(htt_soc->pdevid_tt[i].umac_ttt);
			break;
		}
		qdf_mem_set(htt_soc->pdevid_tt[i].lmac_ttt, alloc_size, -1);
	}
	if (i != MAX_PDEV_CNT) {
		for (j = 0; j < i; j++) {
			qdf_mem_free(htt_soc->pdevid_tt[i].umac_ttt);
			qdf_mem_free(htt_soc->pdevid_tt[i].lmac_ttt);
		}
		return NULL;
	}

	htt_soc->dp_soc = soc;
	htt_soc->htc_soc = htc_handle;
	HTT_TX_MUTEX_INIT(&htt_soc->htt_tx_mutex);

	return htt_soc;
}

#if defined(WDI_EVENT_ENABLE) && !defined(REMOVE_PKT_LOG)
/*
 * dp_ppdu_stats_ind_handler() - PPDU stats msg handler
 * @htt_soc:	 HTT SOC handle
 * @msg_word:    Pointer to payload
 * @htt_t2h_msg: HTT msg nbuf
 *
 * Return: True if buffer should be freed by caller.
 */
static bool
dp_ppdu_stats_ind_handler(struct htt_soc *soc,
				uint32_t *msg_word,
				qdf_nbuf_t htt_t2h_msg)
{
	u_int8_t pdev_id;
	bool free_buf;
	qdf_nbuf_set_pktlen(htt_t2h_msg, HTT_T2H_MAX_MSG_SIZE);
	pdev_id = HTT_T2H_PPDU_STATS_PDEV_ID_GET(*msg_word);
	pdev_id = DP_HW2SW_MACID(pdev_id);
	free_buf = dp_txrx_ppdu_stats_handler(soc->dp_soc, pdev_id,
					      htt_t2h_msg);
	dp_wdi_event_handler(WDI_EVENT_LITE_T2H, soc->dp_soc,
		htt_t2h_msg, HTT_INVALID_PEER, WDI_NO_VAL,
		pdev_id);
	return free_buf;
}
#else
static bool
dp_ppdu_stats_ind_handler(struct htt_soc *soc,
				uint32_t *msg_word,
				qdf_nbuf_t htt_t2h_msg)
{
	return true;
}
#endif

#if defined(WDI_EVENT_ENABLE) && \
	!defined(REMOVE_PKT_LOG)
/*
 * dp_pktlog_msg_handler() - Pktlog msg handler
 * @htt_soc:	 HTT SOC handle
 * @msg_word:    Pointer to payload
 *
 * Return: None
 */
static void
dp_pktlog_msg_handler(struct htt_soc *soc,
		      uint32_t *msg_word)
{
	uint8_t pdev_id;
	uint32_t *pl_hdr;

	pdev_id = HTT_T2H_PKTLOG_PDEV_ID_GET(*msg_word);
	pdev_id = DP_HW2SW_MACID(pdev_id);
	pl_hdr = (msg_word + 1);
	dp_wdi_event_handler(WDI_EVENT_OFFLOAD_ALL, soc->dp_soc,
		pl_hdr, HTT_INVALID_PEER, WDI_NO_VAL,
		pdev_id);
}
#else
static void
dp_pktlog_msg_handler(struct htt_soc *soc,
		      uint32_t *msg_word)
{
}
#endif

/*
 * time_allow_print() - time allow print
 * @htt_ring_tt:	ringi_id array of timestamps
 * @ring_id:		ring_id (index)
 *
 * Return: 1 for successfully saving timestamp in array
 *	and 0 for timestamp falling within 2 seconds after last one
 */
static bool time_allow_print(unsigned long *htt_ring_tt, u_int8_t ring_id)
{
	unsigned long tstamp;
	unsigned long delta;

	tstamp = qdf_get_system_timestamp();

	if (!htt_ring_tt)
		return 0; //unable to print backpressure messages

	if (htt_ring_tt[ring_id] == -1) {
		htt_ring_tt[ring_id] = tstamp;
		return 1;
	}
	delta = tstamp - htt_ring_tt[ring_id];
	if (delta >= 2000) {
		htt_ring_tt[ring_id] = tstamp;
		return 1;
	}

	return 0;
}

static void dp_htt_alert_print(enum htt_t2h_msg_type msg_type,
			       u_int8_t pdev_id, u_int8_t ring_id,
			       u_int16_t hp_idx, u_int16_t tp_idx,
			       u_int32_t bkp_time, char *ring_stype)
{
	dp_alert("msg_type: %d pdev_id: %d ring_type: %s ",
		 msg_type, pdev_id, ring_stype);
	dp_alert("ring_id: %d hp_idx: %d tp_idx: %d bkpressure_time_ms: %d ",
		 ring_id, hp_idx, tp_idx, bkp_time);
}

/*
 * dp_htt_bkp_event_alert() - htt backpressure event alert
 * @msg_word:	htt packet context
 * @htt_soc:	HTT SOC handle
 *
 * Return: after attempting to print stats
 */
static void dp_htt_bkp_event_alert(u_int32_t *msg_word, struct htt_soc *soc)
{
	u_int8_t ring_type;
	u_int8_t pdev_id;
	u_int8_t ring_id;
	u_int16_t hp_idx;
	u_int16_t tp_idx;
	u_int32_t bkp_time;
	enum htt_t2h_msg_type msg_type;
	struct dp_soc *dpsoc;
	struct dp_pdev *pdev;
	struct dp_htt_timestamp *radio_tt;

	if (!soc)
		return;

	dpsoc = (struct dp_soc *)soc->dp_soc;
	msg_type = HTT_T2H_MSG_TYPE_GET(*msg_word);
	ring_type = HTT_T2H_RX_BKPRESSURE_RING_TYPE_GET(*msg_word);
	pdev_id = HTT_T2H_RX_BKPRESSURE_PDEV_ID_GET(*msg_word);
	pdev_id = DP_HW2SW_MACID(pdev_id);
	pdev = (struct dp_pdev *)dpsoc->pdev_list[pdev_id];
	ring_id = HTT_T2H_RX_BKPRESSURE_RINGID_GET(*msg_word);
	hp_idx = HTT_T2H_RX_BKPRESSURE_HEAD_IDX_GET(*(msg_word + 1));
	tp_idx = HTT_T2H_RX_BKPRESSURE_TAIL_IDX_GET(*(msg_word + 1));
	bkp_time = HTT_T2H_RX_BKPRESSURE_TIME_MS_GET(*(msg_word + 2));
	radio_tt = &soc->pdevid_tt[pdev_id];

	switch (ring_type) {
	case HTT_SW_RING_TYPE_UMAC:
		if (!time_allow_print(radio_tt->umac_ttt, ring_id))
			return;
		dp_htt_alert_print(msg_type, pdev_id, ring_id, hp_idx, tp_idx,
				   bkp_time, "HTT_SW_RING_TYPE_UMAC");
	break;
	case HTT_SW_RING_TYPE_LMAC:
		if (!time_allow_print(radio_tt->lmac_ttt, ring_id))
			return;
		dp_htt_alert_print(msg_type, pdev_id, ring_id, hp_idx, tp_idx,
				   bkp_time, "HTT_SW_RING_TYPE_LMAC");
	break;
	default:
		dp_htt_alert_print(msg_type, pdev_id, ring_id, hp_idx, tp_idx,
				   bkp_time, "UNKNOWN");
	break;
	}

	dp_print_ring_stats(pdev);
	dp_print_napi_stats(pdev->soc);
}

/*
 * dp_htt_t2h_msg_handler() - Generic Target to host Msg/event handler
 * @context:	Opaque context (HTT SOC handle)
 * @pkt:	HTC packet
 */
static void dp_htt_t2h_msg_handler(void *context, HTC_PACKET *pkt)
{
	struct htt_soc *soc = (struct htt_soc *) context;
	qdf_nbuf_t htt_t2h_msg = (qdf_nbuf_t) pkt->pPktContext;
	u_int32_t *msg_word;
	enum htt_t2h_msg_type msg_type;
	bool free_buf = true;

	/* check for successful message reception */
	if (pkt->Status != QDF_STATUS_SUCCESS) {
		if (pkt->Status != QDF_STATUS_E_CANCELED)
			soc->stats.htc_err_cnt++;

		qdf_nbuf_free(htt_t2h_msg);
		return;
	}

	/* TODO: Check if we should pop the HTC/HTT header alignment padding */

	msg_word = (u_int32_t *) qdf_nbuf_data(htt_t2h_msg);
	msg_type = HTT_T2H_MSG_TYPE_GET(*msg_word);
	htt_event_record(soc->htt_logger_handle,
			 msg_type, (uint8_t *)msg_word);
	switch (msg_type) {
	case HTT_T2H_MSG_TYPE_BKPRESSURE_EVENT_IND:
	{
		dp_htt_bkp_event_alert(msg_word, soc);
		break;
	}
	case HTT_T2H_MSG_TYPE_PEER_MAP:
		{
			u_int8_t mac_addr_deswizzle_buf[QDF_MAC_ADDR_SIZE];
			u_int8_t *peer_mac_addr;
			u_int16_t peer_id;
			u_int16_t hw_peer_id;
			u_int8_t vdev_id;
			u_int8_t is_wds;
			struct dp_soc *dpsoc = (struct dp_soc *)soc->dp_soc;

			peer_id = HTT_RX_PEER_MAP_PEER_ID_GET(*msg_word);
			hw_peer_id =
				HTT_RX_PEER_MAP_HW_PEER_ID_GET(*(msg_word+2));
			vdev_id = HTT_RX_PEER_MAP_VDEV_ID_GET(*msg_word);
			peer_mac_addr = htt_t2h_mac_addr_deswizzle(
				(u_int8_t *) (msg_word+1),
				&mac_addr_deswizzle_buf[0]);
			QDF_TRACE(QDF_MODULE_ID_TXRX,
				QDF_TRACE_LEVEL_INFO,
				"HTT_T2H_MSG_TYPE_PEER_MAP msg for peer id %d vdev id %d n",
				peer_id, vdev_id);

			/*
			 * check if peer already exists for this peer_id, if so
			 * this peer map event is in response for a wds peer add
			 * wmi command sent during wds source port learning.
			 * in this case just add the ast entry to the existing
			 * peer ast_list.
			 */
			is_wds = !!(dpsoc->peer_id_to_obj_map[peer_id]);
			dp_rx_peer_map_handler(soc->dp_soc, peer_id, hw_peer_id,
					       vdev_id, peer_mac_addr, 0,
					       is_wds);
			break;
		}
	case HTT_T2H_MSG_TYPE_PEER_UNMAP:
		{
			u_int16_t peer_id;
			u_int8_t vdev_id;
			u_int8_t mac_addr[QDF_MAC_ADDR_SIZE] = {0};
			peer_id = HTT_RX_PEER_UNMAP_PEER_ID_GET(*msg_word);
			vdev_id = HTT_RX_PEER_UNMAP_VDEV_ID_GET(*msg_word);

			dp_rx_peer_unmap_handler(soc->dp_soc, peer_id,
						 vdev_id, mac_addr, 0);
			break;
		}
	case HTT_T2H_MSG_TYPE_SEC_IND:
		{
			u_int16_t peer_id;
			enum cdp_sec_type sec_type;
			int is_unicast;

			peer_id = HTT_SEC_IND_PEER_ID_GET(*msg_word);
			sec_type = HTT_SEC_IND_SEC_TYPE_GET(*msg_word);
			is_unicast = HTT_SEC_IND_UNICAST_GET(*msg_word);
			/* point to the first part of the Michael key */
			msg_word++;
			dp_rx_sec_ind_handler(
				soc->dp_soc, peer_id, sec_type, is_unicast,
				msg_word, msg_word + 2);
			break;
		}

	case HTT_T2H_MSG_TYPE_PPDU_STATS_IND:
		{
			free_buf = dp_ppdu_stats_ind_handler(soc, msg_word,
							     htt_t2h_msg);
			break;
		}

	case HTT_T2H_MSG_TYPE_PKTLOG:
		{
			dp_pktlog_msg_handler(soc, msg_word);
			break;
		}

	case HTT_T2H_MSG_TYPE_VERSION_CONF:
		{
			htc_pm_runtime_put(soc->htc_soc);
			soc->tgt_ver.major = HTT_VER_CONF_MAJOR_GET(*msg_word);
			soc->tgt_ver.minor = HTT_VER_CONF_MINOR_GET(*msg_word);
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_HIGH,
				"target uses HTT version %d.%d; host uses %d.%d",
				soc->tgt_ver.major, soc->tgt_ver.minor,
				HTT_CURRENT_VERSION_MAJOR,
				HTT_CURRENT_VERSION_MINOR);
			if (soc->tgt_ver.major != HTT_CURRENT_VERSION_MAJOR) {
				QDF_TRACE(QDF_MODULE_ID_TXRX,
					QDF_TRACE_LEVEL_ERROR,
					"*** Incompatible host/target HTT versions!");
			}
			/* abort if the target is incompatible with the host */
			qdf_assert(soc->tgt_ver.major ==
				HTT_CURRENT_VERSION_MAJOR);
			if (soc->tgt_ver.minor != HTT_CURRENT_VERSION_MINOR) {
				QDF_TRACE(QDF_MODULE_ID_TXRX,
					QDF_TRACE_LEVEL_WARN,
					"*** Warning: host/target HTT versions"
					" are different, though compatible!");
			}
			break;
		}
	case HTT_T2H_MSG_TYPE_RX_ADDBA:
		{
			uint16_t peer_id;
			uint8_t tid;
			uint8_t win_sz;
			uint16_t status;
			struct dp_peer *peer;

			/*
			 * Update REO Queue Desc with new values
			 */
			peer_id = HTT_RX_ADDBA_PEER_ID_GET(*msg_word);
			tid = HTT_RX_ADDBA_TID_GET(*msg_word);
			win_sz = HTT_RX_ADDBA_WIN_SIZE_GET(*msg_word);
			peer = dp_peer_find_by_id(soc->dp_soc, peer_id);

			/*
			 * Window size needs to be incremented by 1
			 * since fw needs to represent a value of 256
			 * using just 8 bits
			 */
			if (peer) {
				status = dp_addba_requestprocess_wifi3(peer,
						0, tid, 0, win_sz + 1, 0xffff);

				/*
				 * If PEER_LOCK_REF_PROTECT enbled dec ref
				 * which is inc by dp_peer_find_by_id
				 */
				dp_peer_unref_del_find_by_id(peer);

				QDF_TRACE(QDF_MODULE_ID_TXRX,
					QDF_TRACE_LEVEL_INFO,
					FL("PeerID %d BAW %d TID %d stat %d"),
					peer_id, win_sz, tid, status);

			} else {
				QDF_TRACE(QDF_MODULE_ID_TXRX,
					QDF_TRACE_LEVEL_ERROR,
					FL("Peer not found peer id %d"),
					peer_id);
			}
			break;
		}
	case HTT_T2H_MSG_TYPE_EXT_STATS_CONF:
		{
			dp_txrx_fw_stats_handler(soc->dp_soc, htt_t2h_msg);
			break;
		}
	case HTT_T2H_MSG_TYPE_PEER_MAP_V2:
		{
			u_int8_t mac_addr_deswizzle_buf[QDF_MAC_ADDR_SIZE];
			u_int8_t *peer_mac_addr;
			u_int16_t peer_id;
			u_int16_t hw_peer_id;
			u_int8_t vdev_id;
			bool is_wds;
			u_int16_t ast_hash;

			peer_id = HTT_RX_PEER_MAP_V2_SW_PEER_ID_GET(*msg_word);
			hw_peer_id =
			HTT_RX_PEER_MAP_V2_HW_PEER_ID_GET(*(msg_word + 2));
			vdev_id = HTT_RX_PEER_MAP_V2_VDEV_ID_GET(*msg_word);
			peer_mac_addr =
			htt_t2h_mac_addr_deswizzle((u_int8_t *)(msg_word + 1),
						   &mac_addr_deswizzle_buf[0]);
			is_wds =
			HTT_RX_PEER_MAP_V2_NEXT_HOP_GET(*(msg_word + 3));
			ast_hash =
			HTT_RX_PEER_MAP_V2_AST_HASH_VALUE_GET(*(msg_word + 3));
			QDF_TRACE(QDF_MODULE_ID_TXRX,
				  QDF_TRACE_LEVEL_INFO,
				  "HTT_T2H_MSG_TYPE_PEER_MAP msg for peer id %d vdev id %d n",
				  peer_id, vdev_id);

			dp_rx_peer_map_handler(soc->dp_soc, peer_id,
					       hw_peer_id, vdev_id,
					       peer_mac_addr, ast_hash,
					       is_wds);
			break;
		}
	case HTT_T2H_MSG_TYPE_PEER_UNMAP_V2:
		{
			u_int8_t mac_addr_deswizzle_buf[QDF_MAC_ADDR_SIZE];
			u_int8_t *mac_addr;
			u_int16_t peer_id;
			u_int8_t vdev_id;
			u_int8_t is_wds;

			peer_id =
			HTT_RX_PEER_UNMAP_V2_SW_PEER_ID_GET(*msg_word);
			vdev_id = HTT_RX_PEER_UNMAP_V2_VDEV_ID_GET(*msg_word);
			mac_addr =
			htt_t2h_mac_addr_deswizzle((u_int8_t *)(msg_word + 1),
						   &mac_addr_deswizzle_buf[0]);
			is_wds =
			HTT_RX_PEER_UNMAP_V2_NEXT_HOP_GET(*(msg_word + 2));
			QDF_TRACE(QDF_MODULE_ID_TXRX,
				  QDF_TRACE_LEVEL_INFO,
				  "HTT_T2H_MSG_TYPE_PEER_UNMAP msg for peer id %d vdev id %d n",
				  peer_id, vdev_id);

			dp_rx_peer_unmap_handler(soc->dp_soc, peer_id,
						 vdev_id, mac_addr,
						 is_wds);
			break;
		}
	default:
		break;
	};

	/* Free the indication buffer */
	if (free_buf)
		qdf_nbuf_free(htt_t2h_msg);
}

/*
 * dp_htt_h2t_full() - Send full handler (called from HTC)
 * @context:	Opaque context (HTT SOC handle)
 * @pkt:	HTC packet
 *
 * Return: enum htc_send_full_action
 */
static enum htc_send_full_action
dp_htt_h2t_full(void *context, HTC_PACKET *pkt)
{
	return HTC_SEND_FULL_KEEP;
}

/*
 * dp_htt_hif_t2h_hp_callback() - HIF callback for high priority T2H messages
 * @context:	Opaque context (HTT SOC handle)
 * @nbuf:	nbuf containing T2H message
 * @pipe_id:	HIF pipe ID
 *
 * Return: QDF_STATUS
 *
 * TODO: Temporary change to bypass HTC connection for this new HIF pipe, which
 * will be used for packet log and other high-priority HTT messages. Proper
 * HTC connection to be added later once required FW changes are available
 */
static QDF_STATUS
dp_htt_hif_t2h_hp_callback (void *context, qdf_nbuf_t nbuf, uint8_t pipe_id)
{
	QDF_STATUS rc = QDF_STATUS_SUCCESS;
	HTC_PACKET htc_pkt;

	qdf_assert_always(pipe_id == DP_HTT_T2H_HP_PIPE);
	qdf_mem_zero(&htc_pkt, sizeof(htc_pkt));
	htc_pkt.Status = QDF_STATUS_SUCCESS;
	htc_pkt.pPktContext = (void *)nbuf;
	dp_htt_t2h_msg_handler(context, &htc_pkt);

	return rc;
}

/*
 * htt_htc_soc_attach() - Register SOC level HTT instance with HTC
 * @htt_soc:	HTT SOC handle
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
htt_htc_soc_attach(struct htt_soc *soc)
{
	struct htc_service_connect_req connect;
	struct htc_service_connect_resp response;
	QDF_STATUS status;
	struct dp_soc *dpsoc = soc->dp_soc;

	qdf_mem_zero(&connect, sizeof(connect));
	qdf_mem_zero(&response, sizeof(response));

	connect.pMetaData = NULL;
	connect.MetaDataLength = 0;
	connect.EpCallbacks.pContext = soc;
	connect.EpCallbacks.EpTxComplete = dp_htt_h2t_send_complete;
	connect.EpCallbacks.EpTxCompleteMultiple = NULL;
	connect.EpCallbacks.EpRecv = dp_htt_t2h_msg_handler;

	/* rx buffers currently are provided by HIF, not by EpRecvRefill */
	connect.EpCallbacks.EpRecvRefill = NULL;

	/* N/A, fill is done by HIF */
	connect.EpCallbacks.RecvRefillWaterMark = 1;

	connect.EpCallbacks.EpSendFull = dp_htt_h2t_full;
	/*
	 * Specify how deep to let a queue get before htc_send_pkt will
	 * call the EpSendFull function due to excessive send queue depth.
	 */
	connect.MaxSendQueueDepth = DP_HTT_MAX_SEND_QUEUE_DEPTH;

	/* disable flow control for HTT data message service */
	connect.ConnectionFlags |= HTC_CONNECT_FLAGS_DISABLE_CREDIT_FLOW_CTRL;

	/* connect to control service */
	connect.service_id = HTT_DATA_MSG_SVC;

	status = htc_connect_service(soc->htc_soc, &connect, &response);

	if (status != QDF_STATUS_SUCCESS)
		return status;

	soc->htc_endpoint = response.Endpoint;

	hif_save_htc_htt_config_endpoint(dpsoc->hif_handle, soc->htc_endpoint);

	htt_interface_logging_init(&soc->htt_logger_handle);
	dp_hif_update_pipe_callback(soc->dp_soc, (void *)soc,
		dp_htt_hif_t2h_hp_callback, DP_HTT_T2H_HP_PIPE);

	return QDF_STATUS_SUCCESS; /* success */
}

/*
 * htt_soc_initialize() - SOC level HTT initialization
 * @htt_soc: Opaque htt SOC handle
 * @ctrl_psoc: Opaque ctrl SOC handle
 * @htc_soc: SOC level HTC handle
 * @hal_soc: Opaque HAL SOC handle
 * @osdev: QDF device
 *
 * Return: HTT handle on success; NULL on failure
 */
void *
htt_soc_initialize(struct htt_soc *htt_soc,
		   struct cdp_ctrl_objmgr_psoc *ctrl_psoc,
		   HTC_HANDLE htc_soc,
		   hal_soc_handle_t hal_soc_hdl, qdf_device_t osdev)
{
	struct htt_soc *soc = (struct htt_soc *)htt_soc;

	soc->osdev = osdev;
	soc->ctrl_psoc = ctrl_psoc;
	soc->htc_soc = htc_soc;
	soc->hal_soc = hal_soc_hdl;

	if (htt_htc_soc_attach(soc))
		goto fail2;

	return soc;

fail2:
	return NULL;
}

void htt_soc_htc_dealloc(struct htt_soc *htt_handle)
{
	htt_interface_logging_deinit(htt_handle->htt_logger_handle);
	htt_htc_misc_pkt_pool_free(htt_handle);
	htt_htc_pkt_pool_free(htt_handle);
}

/*
 * htt_soc_htc_prealloc() - HTC memory prealloc
 * @htt_soc: SOC level HTT handle
 *
 * Return: QDF_STATUS_SUCCESS on Success or
 * QDF_STATUS_E_NOMEM on allocation failure
 */
QDF_STATUS htt_soc_htc_prealloc(struct htt_soc *soc)
{
	int i;

	soc->htt_htc_pkt_freelist = NULL;
	/* pre-allocate some HTC_PACKET objects */
	for (i = 0; i < HTT_HTC_PKT_POOL_INIT_SIZE; i++) {
		struct dp_htt_htc_pkt_union *pkt;
		pkt = qdf_mem_malloc(sizeof(*pkt));
		if (!pkt)
			return QDF_STATUS_E_NOMEM;

		htt_htc_pkt_free(soc, &pkt->u.pkt);
	}
	return QDF_STATUS_SUCCESS;
}

/*
 * htt_soc_detach() - Free SOC level HTT handle
 * @htt_hdl: HTT SOC handle
 */
void htt_soc_detach(struct htt_soc *htt_hdl)
{
	int i;
	struct htt_soc *htt_handle = (struct htt_soc *)htt_hdl;

	for (i = 0; i < MAX_PDEV_CNT; i++) {
		qdf_mem_free(htt_handle->pdevid_tt[i].umac_ttt);
		qdf_mem_free(htt_handle->pdevid_tt[i].lmac_ttt);
	}

	HTT_TX_MUTEX_DESTROY(&htt_handle->htt_tx_mutex);
	qdf_mem_free(htt_handle);

}

/**
 * dp_h2t_ext_stats_msg_send(): function to contruct HTT message to pass to FW
 * @pdev: DP PDEV handle
 * @stats_type_upload_mask: stats type requested by user
 * @config_param_0: extra configuration parameters
 * @config_param_1: extra configuration parameters
 * @config_param_2: extra configuration parameters
 * @config_param_3: extra configuration parameters
 * @mac_id: mac number
 *
 * return: QDF STATUS
 */
QDF_STATUS dp_h2t_ext_stats_msg_send(struct dp_pdev *pdev,
		uint32_t stats_type_upload_mask, uint32_t config_param_0,
		uint32_t config_param_1, uint32_t config_param_2,
		uint32_t config_param_3, int cookie_val, int cookie_msb,
		uint8_t mac_id)
{
	struct htt_soc *soc = pdev->soc->htt_handle;
	struct dp_htt_htc_pkt *pkt;
	qdf_nbuf_t msg;
	uint32_t *msg_word;
	uint8_t pdev_mask = 0;
	uint8_t *htt_logger_bufp;

	msg = qdf_nbuf_alloc(
			soc->osdev,
			HTT_MSG_BUF_SIZE(HTT_H2T_EXT_STATS_REQ_MSG_SZ),
			HTC_HEADER_LEN + HTC_HDR_ALIGNMENT_PADDING, 4, TRUE);

	if (!msg)
		return QDF_STATUS_E_NOMEM;

	/*TODO:Add support for SOC stats
	 * Bit 0: SOC Stats
	 * Bit 1: Pdev stats for pdev id 0
	 * Bit 2: Pdev stats for pdev id 1
	 * Bit 3: Pdev stats for pdev id 2
	 */
	mac_id = dp_get_mac_id_for_pdev(mac_id, pdev->pdev_id);

	pdev_mask = 1 << DP_SW2HW_MACID(mac_id);
	/*
	 * Set the length of the message.
	 * The contribution from the HTC_HDR_ALIGNMENT_PADDING is added
	 * separately during the below call to qdf_nbuf_push_head.
	 * The contribution from the HTC header is added separately inside HTC.
	 */
	if (qdf_nbuf_put_tail(msg, HTT_H2T_EXT_STATS_REQ_MSG_SZ) == NULL) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"Failed to expand head for HTT_EXT_STATS");
		qdf_nbuf_free(msg);
		return QDF_STATUS_E_FAILURE;
	}

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
		"-----%s:%d----\n cookie <-> %d\n config_param_0 %u\n"
		"config_param_1 %u\n config_param_2 %u\n"
		"config_param_4 %u\n -------------",
		__func__, __LINE__, cookie_val, config_param_0,
		config_param_1, config_param_2,	config_param_3);

	msg_word = (uint32_t *) qdf_nbuf_data(msg);

	qdf_nbuf_push_head(msg, HTC_HDR_ALIGNMENT_PADDING);
	htt_logger_bufp = (uint8_t *)msg_word;
	*msg_word = 0;
	HTT_H2T_MSG_TYPE_SET(*msg_word, HTT_H2T_MSG_TYPE_EXT_STATS_REQ);
	HTT_H2T_EXT_STATS_REQ_PDEV_MASK_SET(*msg_word, pdev_mask);
	HTT_H2T_EXT_STATS_REQ_STATS_TYPE_SET(*msg_word, stats_type_upload_mask);

	/* word 1 */
	msg_word++;
	*msg_word = 0;
	HTT_H2T_EXT_STATS_REQ_CONFIG_PARAM_SET(*msg_word, config_param_0);

	/* word 2 */
	msg_word++;
	*msg_word = 0;
	HTT_H2T_EXT_STATS_REQ_CONFIG_PARAM_SET(*msg_word, config_param_1);

	/* word 3 */
	msg_word++;
	*msg_word = 0;
	HTT_H2T_EXT_STATS_REQ_CONFIG_PARAM_SET(*msg_word, config_param_2);

	/* word 4 */
	msg_word++;
	*msg_word = 0;
	HTT_H2T_EXT_STATS_REQ_CONFIG_PARAM_SET(*msg_word, config_param_3);

	HTT_H2T_EXT_STATS_REQ_CONFIG_PARAM_SET(*msg_word, 0);

	/* word 5 */
	msg_word++;

	/* word 6 */
	msg_word++;
	*msg_word = 0;
	HTT_H2T_EXT_STATS_REQ_CONFIG_PARAM_SET(*msg_word, cookie_val);

	/* word 7 */
	msg_word++;
	*msg_word = 0;
	/*Using last 2 bits for pdev_id */
	cookie_msb = ((cookie_msb << 2) | pdev->pdev_id);
	HTT_H2T_EXT_STATS_REQ_CONFIG_PARAM_SET(*msg_word, cookie_msb);

	pkt = htt_htc_pkt_alloc(soc);
	if (!pkt) {
		qdf_nbuf_free(msg);
		return QDF_STATUS_E_NOMEM;
	}

	pkt->soc_ctxt = NULL; /* not used during send-done callback */

	SET_HTC_PACKET_INFO_TX(&pkt->htc_pkt,
			dp_htt_h2t_send_complete_free_netbuf,
			qdf_nbuf_data(msg), qdf_nbuf_len(msg),
			soc->htc_endpoint,
			/* tag for FW response msg not guaranteed */
			HTC_TX_PACKET_TAG_RUNTIME_PUT);

	SET_HTC_PACKET_NET_BUF_CONTEXT(&pkt->htc_pkt, msg);
	DP_HTT_SEND_HTC_PKT(soc, pkt, HTT_H2T_MSG_TYPE_EXT_STATS_REQ,
			    htt_logger_bufp);
	return 0;
}

/* This macro will revert once proper HTT header will define for
 * HTT_H2T_MSG_TYPE_PPDU_STATS_CFG in htt.h file
 * */
#if defined(WDI_EVENT_ENABLE)
/**
 * dp_h2t_cfg_stats_msg_send(): function to construct HTT message to pass to FW
 * @pdev: DP PDEV handle
 * @stats_type_upload_mask: stats type requested by user
 * @mac_id: Mac id number
 *
 * return: QDF STATUS
 */
QDF_STATUS dp_h2t_cfg_stats_msg_send(struct dp_pdev *pdev,
		uint32_t stats_type_upload_mask, uint8_t mac_id)
{
	struct htt_soc *soc = pdev->soc->htt_handle;
	struct dp_htt_htc_pkt *pkt;
	qdf_nbuf_t msg;
	uint32_t *msg_word;
	uint8_t pdev_mask;

	msg = qdf_nbuf_alloc(
			soc->osdev,
			HTT_MSG_BUF_SIZE(HTT_H2T_PPDU_STATS_CFG_MSG_SZ),
			HTC_HEADER_LEN + HTC_HDR_ALIGNMENT_PADDING, 4, true);

	if (!msg) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
		"Fail to allocate HTT_H2T_PPDU_STATS_CFG_MSG_SZ msg buffer");
		qdf_assert(0);
		return QDF_STATUS_E_NOMEM;
	}

	/*TODO:Add support for SOC stats
	 * Bit 0: SOC Stats
	 * Bit 1: Pdev stats for pdev id 0
	 * Bit 2: Pdev stats for pdev id 1
	 * Bit 3: Pdev stats for pdev id 2
	 */
	pdev_mask = 1 << DP_SW2HW_MACID(mac_id);

	/*
	 * Set the length of the message.
	 * The contribution from the HTC_HDR_ALIGNMENT_PADDING is added
	 * separately during the below call to qdf_nbuf_push_head.
	 * The contribution from the HTC header is added separately inside HTC.
	 */
	if (qdf_nbuf_put_tail(msg, HTT_H2T_PPDU_STATS_CFG_MSG_SZ) == NULL) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"Failed to expand head for HTT_CFG_STATS");
		qdf_nbuf_free(msg);
		return QDF_STATUS_E_FAILURE;
	}

	msg_word = (uint32_t *) qdf_nbuf_data(msg);

	qdf_nbuf_push_head(msg, HTC_HDR_ALIGNMENT_PADDING);
	*msg_word = 0;
	HTT_H2T_MSG_TYPE_SET(*msg_word, HTT_H2T_MSG_TYPE_PPDU_STATS_CFG);
	HTT_H2T_PPDU_STATS_CFG_PDEV_MASK_SET(*msg_word, pdev_mask);
	HTT_H2T_PPDU_STATS_CFG_TLV_BITMASK_SET(*msg_word,
			stats_type_upload_mask);

	pkt = htt_htc_pkt_alloc(soc);
	if (!pkt) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"Fail to allocate dp_htt_htc_pkt buffer");
		qdf_assert(0);
		qdf_nbuf_free(msg);
		return QDF_STATUS_E_NOMEM;
	}

	pkt->soc_ctxt = NULL; /* not used during send-done callback */

	SET_HTC_PACKET_INFO_TX(&pkt->htc_pkt,
			dp_htt_h2t_send_complete_free_netbuf,
			qdf_nbuf_data(msg), qdf_nbuf_len(msg),
			soc->htc_endpoint,
			1); /* tag - not relevant here */

	SET_HTC_PACKET_NET_BUF_CONTEXT(&pkt->htc_pkt, msg);
	DP_HTT_SEND_HTC_PKT(soc, pkt, HTT_H2T_MSG_TYPE_PPDU_STATS_CFG,
			    (uint8_t *)msg_word);
	return 0;
}
#endif

void
dp_peer_update_inactive_time(struct dp_pdev *pdev, uint32_t tag_type,
			     uint32_t *tag_buf)
{
	switch (tag_type) {
	case HTT_STATS_PEER_DETAILS_TAG:
	{
		htt_peer_details_tlv *dp_stats_buf =
			(htt_peer_details_tlv *)tag_buf;

		pdev->fw_stats_peer_id = dp_stats_buf->sw_peer_id;
	}
	break;
	case HTT_STATS_PEER_STATS_CMN_TAG:
	{
		htt_peer_stats_cmn_tlv *dp_stats_buf =
			(htt_peer_stats_cmn_tlv *)tag_buf;

		struct dp_peer *peer = dp_peer_find_by_id(pdev->soc,
						pdev->fw_stats_peer_id);

		if (peer && !peer->bss_peer) {
			peer->stats.tx.inactive_time =
				dp_stats_buf->inactive_time;
			qdf_event_set(&pdev->fw_peer_stats_event);
		}
		if (peer)
			dp_peer_unref_del_find_by_id(peer);
	}
	break;
	default:
		qdf_err("Invalid tag_type");
	}
}

/**
 * dp_htt_rx_flow_fst_setup(): Send HTT Rx FST setup message to FW
 * @pdev: DP pdev handle
 * @fse_setup_info: FST setup parameters
 *
 * Return: Success when HTT message is sent, error on failure
 */
QDF_STATUS
dp_htt_rx_flow_fst_setup(struct dp_pdev *pdev,
			 struct dp_htt_rx_flow_fst_setup *fse_setup_info)
{
	struct htt_soc *soc = pdev->soc->htt_handle;
	struct dp_htt_htc_pkt *pkt;
	qdf_nbuf_t msg;
	u_int32_t *msg_word;
	struct htt_h2t_msg_rx_fse_setup_t *fse_setup;
	uint8_t *htt_logger_bufp;
	u_int32_t *key;

	msg = qdf_nbuf_alloc(
		soc->osdev,
		HTT_MSG_BUF_SIZE(sizeof(struct htt_h2t_msg_rx_fse_setup_t)),
		/* reserve room for the HTC header */
		HTC_HEADER_LEN + HTC_HDR_ALIGNMENT_PADDING, 4, TRUE);

	if (!msg)
		return QDF_STATUS_E_NOMEM;

	/*
	 * Set the length of the message.
	 * The contribution from the HTC_HDR_ALIGNMENT_PADDING is added
	 * separately during the below call to qdf_nbuf_push_head.
	 * The contribution from the HTC header is added separately inside HTC.
	 */
	if (!qdf_nbuf_put_tail(msg,
			       sizeof(struct htt_h2t_msg_rx_fse_setup_t))) {
		qdf_err("Failed to expand head for HTT RX_FSE_SETUP msg");
		return QDF_STATUS_E_FAILURE;
	}

	/* fill in the message contents */
	msg_word = (u_int32_t *)qdf_nbuf_data(msg);

	memset(msg_word, 0, sizeof(struct htt_h2t_msg_rx_fse_setup_t));
	/* rewind beyond alignment pad to get to the HTC header reserved area */
	qdf_nbuf_push_head(msg, HTC_HDR_ALIGNMENT_PADDING);
	htt_logger_bufp = (uint8_t *)msg_word;

	*msg_word = 0;
	HTT_H2T_MSG_TYPE_SET(*msg_word, HTT_H2T_MSG_TYPE_RX_FSE_SETUP_CFG);

	fse_setup = (struct htt_h2t_msg_rx_fse_setup_t *)msg_word;

	HTT_RX_FSE_SETUP_PDEV_ID_SET(*msg_word, fse_setup_info->pdev_id);

	msg_word++;
	HTT_RX_FSE_SETUP_NUM_REC_SET(*msg_word, fse_setup_info->max_entries);
	HTT_RX_FSE_SETUP_MAX_SEARCH_SET(*msg_word, fse_setup_info->max_search);
	HTT_RX_FSE_SETUP_IP_DA_SA_PREFIX_SET(*msg_word,
					     fse_setup_info->ip_da_sa_prefix);

	msg_word++;
	HTT_RX_FSE_SETUP_BASE_ADDR_LO_SET(*msg_word,
					  fse_setup_info->base_addr_lo);
	msg_word++;
	HTT_RX_FSE_SETUP_BASE_ADDR_HI_SET(*msg_word,
					  fse_setup_info->base_addr_hi);

	key = (u_int32_t *)fse_setup_info->hash_key;
	fse_setup->toeplitz31_0 = *key++;
	fse_setup->toeplitz63_32 = *key++;
	fse_setup->toeplitz95_64 = *key++;
	fse_setup->toeplitz127_96 = *key++;
	fse_setup->toeplitz159_128 = *key++;
	fse_setup->toeplitz191_160 = *key++;
	fse_setup->toeplitz223_192 = *key++;
	fse_setup->toeplitz255_224 = *key++;
	fse_setup->toeplitz287_256 = *key++;
	fse_setup->toeplitz314_288 = *key;

	msg_word++;
	HTT_RX_FSE_SETUP_HASH_VALUE_SET(*msg_word, fse_setup->toeplitz31_0);
	msg_word++;
	HTT_RX_FSE_SETUP_HASH_VALUE_SET(*msg_word, fse_setup->toeplitz63_32);
	msg_word++;
	HTT_RX_FSE_SETUP_HASH_VALUE_SET(*msg_word, fse_setup->toeplitz95_64);
	msg_word++;
	HTT_RX_FSE_SETUP_HASH_VALUE_SET(*msg_word, fse_setup->toeplitz127_96);
	msg_word++;
	HTT_RX_FSE_SETUP_HASH_VALUE_SET(*msg_word, fse_setup->toeplitz159_128);
	msg_word++;
	HTT_RX_FSE_SETUP_HASH_VALUE_SET(*msg_word, fse_setup->toeplitz191_160);
	msg_word++;
	HTT_RX_FSE_SETUP_HASH_VALUE_SET(*msg_word, fse_setup->toeplitz223_192);
	msg_word++;
	HTT_RX_FSE_SETUP_HASH_VALUE_SET(*msg_word, fse_setup->toeplitz255_224);
	msg_word++;
	HTT_RX_FSE_SETUP_HASH_VALUE_SET(*msg_word, fse_setup->toeplitz287_256);
	msg_word++;
	HTT_RX_FSE_SETUP_HASH_314_288_SET(*msg_word,
					  fse_setup->toeplitz314_288);

	pkt = htt_htc_pkt_alloc(soc);
	if (!pkt) {
		qdf_err("Fail to allocate dp_htt_htc_pkt buffer");
		qdf_assert(0);
		qdf_nbuf_free(msg);
		return QDF_STATUS_E_RESOURCES; /* failure */
	}

	pkt->soc_ctxt = NULL; /* not used during send-done callback */

	SET_HTC_PACKET_INFO_TX(
		&pkt->htc_pkt,
		dp_htt_h2t_send_complete_free_netbuf,
		qdf_nbuf_data(msg),
		qdf_nbuf_len(msg),
		soc->htc_endpoint,
		1); /* tag - not relevant here */

	SET_HTC_PACKET_NET_BUF_CONTEXT(&pkt->htc_pkt, msg);

	DP_HTT_SEND_HTC_PKT(soc, pkt, HTT_H2T_MSG_TYPE_RX_FSE_SETUP_CFG,
			    htt_logger_bufp);

	qdf_info("HTT_H2T RX_FSE_SETUP sent to FW for pdev = %u",
		 fse_setup_info->pdev_id);
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_ANY, QDF_TRACE_LEVEL_DEBUG,
			   (void *)fse_setup_info->hash_key,
			   fse_setup_info->hash_key_len);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_htt_rx_flow_fse_operation(): Send HTT Flow Search Entry msg to
 * add/del a flow in HW
 * @pdev: DP pdev handle
 * @fse_op_info: Flow entry parameters
 *
 * Return: Success when HTT message is sent, error on failure
 */
QDF_STATUS
dp_htt_rx_flow_fse_operation(struct dp_pdev *pdev,
			     struct dp_htt_rx_flow_fst_operation *fse_op_info)
{
	struct htt_soc *soc = pdev->soc->htt_handle;
	struct dp_htt_htc_pkt *pkt;
	qdf_nbuf_t msg;
	u_int32_t *msg_word;
	struct htt_h2t_msg_rx_fse_operation_t *fse_operation;
	uint8_t *htt_logger_bufp;

	msg = qdf_nbuf_alloc(
		soc->osdev,
		HTT_MSG_BUF_SIZE(sizeof(struct htt_h2t_msg_rx_fse_operation_t)),
		/* reserve room for the HTC header */
		HTC_HEADER_LEN + HTC_HDR_ALIGNMENT_PADDING, 4, TRUE);
	if (!msg)
		return QDF_STATUS_E_NOMEM;

	/*
	 * Set the length of the message.
	 * The contribution from the HTC_HDR_ALIGNMENT_PADDING is added
	 * separately during the below call to qdf_nbuf_push_head.
	 * The contribution from the HTC header is added separately inside HTC.
	 */
	if (!qdf_nbuf_put_tail(msg,
			       sizeof(struct htt_h2t_msg_rx_fse_operation_t))) {
		qdf_err("Failed to expand head for HTT_RX_FSE_OPERATION msg");
		return QDF_STATUS_E_FAILURE;
	}

	/* fill in the message contents */
	msg_word = (u_int32_t *)qdf_nbuf_data(msg);

	memset(msg_word, 0, sizeof(struct htt_h2t_msg_rx_fse_operation_t));
	/* rewind beyond alignment pad to get to the HTC header reserved area */
	qdf_nbuf_push_head(msg, HTC_HDR_ALIGNMENT_PADDING);
	htt_logger_bufp = (uint8_t *)msg_word;

	*msg_word = 0;
	HTT_H2T_MSG_TYPE_SET(*msg_word, HTT_H2T_MSG_TYPE_RX_FSE_OPERATION_CFG);

	fse_operation = (struct htt_h2t_msg_rx_fse_operation_t *)msg_word;

	HTT_RX_FSE_OPERATION_PDEV_ID_SET(*msg_word, fse_op_info->pdev_id);
	msg_word++;
	HTT_RX_FSE_IPSEC_VALID_SET(*msg_word, false);
	if (fse_op_info->op_code == DP_HTT_FST_CACHE_INVALIDATE_ENTRY) {
		HTT_RX_FSE_OPERATION_SET(*msg_word,
					 HTT_RX_FSE_CACHE_INVALIDATE_ENTRY);
		msg_word++;
		HTT_RX_FSE_OPERATION_IP_ADDR_SET(
		*msg_word,
		qdf_htonl(fse_op_info->rx_flow->flow_tuple_info.src_ip_31_0));
		msg_word++;
		HTT_RX_FSE_OPERATION_IP_ADDR_SET(
		*msg_word,
		qdf_htonl(fse_op_info->rx_flow->flow_tuple_info.src_ip_63_32));
		msg_word++;
		HTT_RX_FSE_OPERATION_IP_ADDR_SET(
		*msg_word,
		qdf_htonl(fse_op_info->rx_flow->flow_tuple_info.src_ip_95_64));
		msg_word++;
		HTT_RX_FSE_OPERATION_IP_ADDR_SET(
		*msg_word,
		qdf_htonl(fse_op_info->rx_flow->flow_tuple_info.src_ip_127_96));
		msg_word++;
		HTT_RX_FSE_OPERATION_IP_ADDR_SET(
		*msg_word,
		qdf_htonl(fse_op_info->rx_flow->flow_tuple_info.dest_ip_31_0));
		msg_word++;
		HTT_RX_FSE_OPERATION_IP_ADDR_SET(
		*msg_word,
		qdf_htonl(fse_op_info->rx_flow->flow_tuple_info.dest_ip_63_32));
		msg_word++;
		HTT_RX_FSE_OPERATION_IP_ADDR_SET(
		*msg_word,
		qdf_htonl(fse_op_info->rx_flow->flow_tuple_info.dest_ip_95_64));
		msg_word++;
		HTT_RX_FSE_OPERATION_IP_ADDR_SET(
		*msg_word,
		qdf_htonl(
		fse_op_info->rx_flow->flow_tuple_info.dest_ip_127_96));
		msg_word++;
		HTT_RX_FSE_SOURCEPORT_SET(
			*msg_word,
			fse_op_info->rx_flow->flow_tuple_info.src_port);
		HTT_RX_FSE_DESTPORT_SET(
			*msg_word,
			fse_op_info->rx_flow->flow_tuple_info.dest_port);
		msg_word++;
		HTT_RX_FSE_L4_PROTO_SET(
			*msg_word,
			fse_op_info->rx_flow->flow_tuple_info.l4_protocol);
	} else if (fse_op_info->op_code == DP_HTT_FST_CACHE_INVALIDATE_FULL) {
		HTT_RX_FSE_OPERATION_SET(*msg_word,
					 HTT_RX_FSE_CACHE_INVALIDATE_FULL);
	} else if (fse_op_info->op_code == DP_HTT_FST_DISABLE) {
		HTT_RX_FSE_OPERATION_SET(*msg_word, HTT_RX_FSE_DISABLE);
	} else if (fse_op_info->op_code == DP_HTT_FST_ENABLE) {
		HTT_RX_FSE_OPERATION_SET(*msg_word, HTT_RX_FSE_ENABLE);
	}

	pkt = htt_htc_pkt_alloc(soc);
	if (!pkt) {
		qdf_err("Fail to allocate dp_htt_htc_pkt buffer");
		qdf_assert(0);
		qdf_nbuf_free(msg);
		return QDF_STATUS_E_RESOURCES; /* failure */
	}

	pkt->soc_ctxt = NULL; /* not used during send-done callback */

	SET_HTC_PACKET_INFO_TX(
		&pkt->htc_pkt,
		dp_htt_h2t_send_complete_free_netbuf,
		qdf_nbuf_data(msg),
		qdf_nbuf_len(msg),
		soc->htc_endpoint,
		1); /* tag - not relevant here */

	SET_HTC_PACKET_NET_BUF_CONTEXT(&pkt->htc_pkt, msg);

	DP_HTT_SEND_HTC_PKT(soc, pkt, HTT_H2T_MSG_TYPE_RX_FSE_OPERATION_CFG,
			    htt_logger_bufp);

	qdf_info("HTT_H2T RX_FSE_OPERATION_CFG sent to FW for pdev = %u",
		 fse_op_info->pdev_id);

	return QDF_STATUS_SUCCESS;
}
