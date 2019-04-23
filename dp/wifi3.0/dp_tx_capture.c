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

#include <htt.h>
#include "qdf_trace.h"
#include "qdf_nbuf.h"
#include "dp_peer.h"
#include "dp_types.h"
#include "dp_internal.h"
#include "dp_rx_mon.h"
#include "htt_ppdu_stats.h"
#include "dp_htt.h"
#include "qdf_mem.h"   /* qdf_mem_malloc,free */
#include "cdp_txrx_cmn_struct.h"
#include <enet.h>

#include "dp_tx_capture.h"

#ifdef WLAN_TX_PKT_CAPTURE_ENH

/*
 * dp_peer_tid_queue_init() – Initialize ppdu stats queue per TID
 * @peer: Datapath peer
 *
 */
void dp_peer_tid_queue_init(struct dp_peer *peer)
{
	int tid;
	struct dp_tx_tid *tx_tid;

	for (tid = 0; tid < DP_MAX_TIDS; tid++) {
		tx_tid = &peer->tx_capture.tx_tid[tid];
		tx_tid->tid = tid;
		qdf_nbuf_queue_init(&tx_tid->msdu_comp_q);
		tx_tid->max_ppdu_id = 0;

		/* spinlock create */
		qdf_spinlock_create(&tx_tid->tid_lock);
	}
}

/*
 * dp_peer_tid_queue_cleanup() – remove ppdu stats queue per TID
 * @peer: Datapath peer
 *
 */
void dp_peer_tid_queue_cleanup(struct dp_peer *peer)
{
	int tid;
	struct dp_tx_tid *tx_tid;

	for (tid = 0; tid < DP_MAX_TIDS; tid++) {
		tx_tid = &peer->tx_capture.tx_tid[tid];

		/* spinlock destroy */
		qdf_spinlock_destroy(&tx_tid->tid_lock);

		qdf_nbuf_queue_free(&tx_tid->msdu_comp_q);
		tx_tid->max_ppdu_id = 0;
	}
}

/*
 * dp_peer_update_80211_hdr: update 80211 hdr
 * @vdev: DP VDEV
 * @peer: DP PEER
 *
 * return: void
 */
void dp_peer_update_80211_hdr(struct dp_vdev *vdev, struct dp_peer *peer)
{
	struct ieee80211_frame *ptr_wh;

	ptr_wh = &peer->tx_capture.tx_wifi_hdr;

	/* i_addr1 - Receiver mac address */
	/* i_addr2 - Transmitter mac address */
	/* i_addr3 - Destination mac address */

	qdf_mem_copy(ptr_wh->i_addr1,
		     peer->mac_addr.raw,
		     QDF_MAC_ADDR_SIZE);
	qdf_mem_copy(ptr_wh->i_addr3,
		     peer->mac_addr.raw,
		     QDF_MAC_ADDR_SIZE);
	qdf_mem_copy(ptr_wh->i_addr2,
		     vdev->mac_addr.raw,
		     QDF_MAC_ADDR_SIZE);
}

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
	} else {
		if (pdev->mgmtctrl_frm_info.last_nbuf)
			qdf_nbuf_free(pdev->mgmtctrl_frm_info.last_nbuf);

		/* pull ppdu_id from the packet */
		qdf_nbuf_pull_head(nbuf, sizeof(uint32_t));

		pdev->mgmtctrl_frm_info.last_nbuf = nbuf;
	}
}

/**
 * dp_tx_ppdu_stats_attach - Initialize Tx PPDU stats and enhanced capture
 * @pdev: DP PDEV
 *
 * Return: none
 */
void dp_tx_ppdu_stats_attach(struct dp_pdev *pdev)
{
	/* Work queue setup for HTT stats and tx capture handling */
	qdf_create_work(0, &pdev->tx_capture.ppdu_stats_work,
			dp_tx_ppdu_stats_process,
			pdev);
	pdev->tx_capture.ppdu_stats_workqueue =
		qdf_alloc_unbound_workqueue("ppdu_stats_work_queue");
	STAILQ_INIT(&pdev->tx_capture.ppdu_stats_queue);
	STAILQ_INIT(&pdev->tx_capture.ppdu_stats_defer_queue);
	qdf_spinlock_create(&pdev->tx_capture.ppdu_stats_lock);
	pdev->tx_capture.ppdu_stats_queue_depth = 0;
	pdev->tx_capture.ppdu_stats_next_sched = 0;
}

/**
 * dp_tx_ppdu_stats_detach - Cleanup Tx PPDU stats and enhanced capture
 * @pdev: DP PDEV
 *
 * Return: none
 */
void dp_tx_ppdu_stats_detach(struct dp_pdev *pdev)
{
	struct ppdu_info *ppdu_info, *tmp_ppdu_info = NULL;

	if (!pdev || !pdev->tx_capture.ppdu_stats_workqueue)
		return;

	qdf_flush_workqueue(0, pdev->tx_capture.ppdu_stats_workqueue);
	qdf_destroy_workqueue(0, pdev->tx_capture.ppdu_stats_workqueue);

	qdf_spinlock_destroy(&pdev->tx_capture.ppdu_stats_lock);

	STAILQ_FOREACH_SAFE(ppdu_info,
			    &pdev->tx_capture.ppdu_stats_queue,
			    ppdu_info_queue_elem, tmp_ppdu_info) {
		STAILQ_REMOVE(&pdev->tx_capture.ppdu_stats_queue,
			      ppdu_info, ppdu_info, ppdu_info_queue_elem);
		qdf_nbuf_free(ppdu_info->nbuf);
		qdf_mem_free(ppdu_info);
	}
}

/**
 * dp_update_msdu_to_list(): Function to queue msdu from wbm
 * @pdev: dp_pdev
 * @peer: dp_peer
 * @ts: hal tx completion status
 * @netbuf: msdu
 *
 * return: status
 */
QDF_STATUS
dp_update_msdu_to_list(struct dp_soc *soc,
		       struct dp_pdev *pdev,
		       struct dp_peer *peer,
		       struct hal_tx_completion_status *ts,
		       qdf_nbuf_t netbuf)
{
	struct dp_tx_tid *tx_tid;
	struct msdu_completion_info *msdu_comp_info;

	if (!peer) {
		QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE, QDF_TRACE_LEVEL_ERROR,
			  "%s: %d peer NULL !", __func__, __LINE__);
		return QDF_STATUS_E_FAILURE;
	}

	if (ts->tid > DP_NON_QOS_TID) {
		QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE, QDF_TRACE_LEVEL_ERROR,
			  "%s: %d peer_id %d, tid %d > NON_QOS_TID!",
			  __func__, __LINE__, ts->peer_id, ts->tid);
		return QDF_STATUS_E_FAILURE;
	}

	tx_tid = &peer->tx_capture.tx_tid[ts->tid];

	if (!tx_tid) {
		QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE, QDF_TRACE_LEVEL_ERROR,
			  "%s: %d tid[%d] NULL !", __func__, __LINE__, ts->tid);
		return QDF_STATUS_E_FAILURE;
	}

	qdf_nbuf_unmap(soc->osdev, netbuf, QDF_DMA_TO_DEVICE);

	if (!qdf_nbuf_push_head(netbuf, sizeof(struct msdu_completion_info))) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  FL("No headroom"));
		return QDF_STATUS_E_NOMEM;
	}

	msdu_comp_info = (struct msdu_completion_info *)qdf_nbuf_data(netbuf);

	/* copy msdu_completion_info to control buffer */
	msdu_comp_info->ppdu_id = ts->ppdu_id;
	msdu_comp_info->peer_id = ts->peer_id;
	msdu_comp_info->tid = ts->tid;
	msdu_comp_info->first_msdu = ts->first_msdu;
	msdu_comp_info->last_msdu = ts->last_msdu;
	msdu_comp_info->msdu_part_of_amsdu = ts->msdu_part_of_amsdu;
	msdu_comp_info->transmit_cnt = ts->transmit_cnt;
	msdu_comp_info->tsf = ts->tsf;

	/* update max ppdu_id */
	tx_tid->max_ppdu_id = ts->ppdu_id;
	pdev->tx_capture.last_msdu_id = ts->ppdu_id;

	QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE, QDF_TRACE_LEVEL_INFO,
		  "msdu_completion: ppdu_id[%d] peer_id[%d] tid[%d] rel_src[%d] status[%d] tsf[%u] A[%d] CNT[%d]",
		  ts->ppdu_id, ts->peer_id, ts->tid, ts->release_src,
		  ts->status, ts->tsf, ts->msdu_part_of_amsdu,
		  ts->transmit_cnt);

	/* lock here */
	qdf_spin_lock(&tx_tid->tid_lock);

	/* add nbuf to tail queue per peer tid */
	qdf_nbuf_queue_add(&tx_tid->msdu_comp_q, netbuf);

	qdf_spin_unlock(&tx_tid->tid_lock);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_tx_add_to_comp_queue() - add completion msdu to queue
 * @soc: DP Soc handle
 * @tx_desc: software Tx descriptor
 * @ts : Tx completion status from HAL/HTT descriptor
 * @peer: DP peer
 *
 * Return: none
 */
QDF_STATUS dp_tx_add_to_comp_queue(struct dp_soc *soc,
				   struct dp_tx_desc_s *desc,
				   struct hal_tx_completion_status *ts,
				   struct dp_peer *peer)
{
	int ret = QDF_STATUS_E_FAILURE;

	if (desc->pdev->tx_capture_enabled == 1 &&
	    (ts->status == HAL_TX_TQM_RR_REM_CMD_TX ||
	    ts->status == HAL_TX_TQM_RR_FRAME_ACKED)) {
		ret = dp_update_msdu_to_list(soc, desc->pdev,
					     peer, ts, desc->nbuf);
	}
	return ret;
}

/**
 * dp_process_ppdu_stats_update_failed_bitmap(): update failed bitmap
 * @pdev: dp_pdev
 * @data: tx completion ppdu desc
 * @ppdu_id: ppdu id
 * @size: size of bitmap
 *
 * return: status
 */
void dp_process_ppdu_stats_update_failed_bitmap(struct dp_pdev *pdev,
						void *data,
						uint32_t ppdu_id,
						uint32_t size)
{
	struct cdp_tx_completion_ppdu_user *user;
	uint32_t mpdu_tried;
	uint32_t ba_seq_no;
	uint32_t start_seq;
	uint32_t num_mpdu;
	uint32_t diff;
	uint32_t carry = 0;
	uint32_t bitmask = 0;

	uint32_t i;
	uint32_t k;
	uint32_t ba_bitmap = 0;

	user = (struct cdp_tx_completion_ppdu_user *)data;

	/* get number of mpdu from ppdu_desc */
	mpdu_tried = user->mpdu_tried_mcast + user->mpdu_tried_ucast;

	ba_seq_no = user->ba_seq_no;
	start_seq = user->start_seq;
	num_mpdu = user->num_mpdu;

	/* assumption: number of mpdu will be less than 32 */

	QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE, QDF_TRACE_LEVEL_INFO,
		  "ppdu_id[%d] ba_seq_no[%d] start_seq_no[%d] mpdu_tried[%d]",
		  ppdu_id, ba_seq_no, start_seq, mpdu_tried);

	for (i = 0; i < size; i++) {
		QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE, QDF_TRACE_LEVEL_INFO,
			  "ppdu_id[%d] ba_bitmap[%x] enqueue_bitmap[%x]",
			  ppdu_id, user->ba_bitmap[i], user->enq_bitmap[i]);
	}

	if (start_seq <= ba_seq_no) {
		diff = ba_seq_no - start_seq;
		bitmask = (1 << diff) - 1;
		for (i = 0; i < size; i++) {
			ba_bitmap = user->ba_bitmap[i];

			user->failed_bitmap[i] = (ba_bitmap << diff);
			user->failed_bitmap[i] |= (bitmask & carry);

			carry = ((ba_bitmap & (bitmask << (32 - diff))) >>
				(32 - diff));

			user->failed_bitmap[i] = user->enq_bitmap[i] &
						 user->failed_bitmap[i];
		}
	} else {
		diff = start_seq - ba_seq_no;
		/* array index */
		k = diff >> 5;
		diff = diff & 0x1F;

		bitmask = (1 << diff) - 1;
		for (i = 0; i < size; i++, k++) {
			ba_bitmap = user->ba_bitmap[k];
			user->failed_bitmap[i] = ba_bitmap >> diff;
			/* get next ba_bitmap */
			ba_bitmap = user->ba_bitmap[k + 1];
			carry = (ba_bitmap & bitmask);
			user->failed_bitmap[i] |=
				((carry & bitmask) << (32 - diff));

			user->failed_bitmap[i] = user->enq_bitmap[i] &
						 user->failed_bitmap[i];
		}
	}
}

/*
 * dp_soc_set_txrx_ring_map_single()
 * @dp_soc: DP handler for soc
 *
 * Return: Void
 */
static void dp_soc_set_txrx_ring_map_single(struct dp_soc *soc)
{
	uint32_t i;

	for (i = 0; i < WLAN_CFG_INT_NUM_CONTEXTS; i++) {
		soc->tx_ring_map[i] =
			dp_cpu_ring_map[DP_SINGLE_TX_RING_MAP][i];
	}
}

/*
 * dp_iterate_free_peer_msdu_q()- API to free msdu queue
 * @pdev_handle: DP_PDEV handle
 *
 * Return: void
 */
static void  dp_iterate_free_peer_msdu_q(void *pdev_hdl)
{
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_hdl;
	struct dp_soc *soc = pdev->soc;
	struct dp_vdev *vdev = NULL;
	struct dp_peer *peer = NULL;

	qdf_spin_lock_bh(&soc->peer_ref_mutex);
	qdf_spin_lock_bh(&pdev->vdev_list_lock);
	DP_PDEV_ITERATE_VDEV_LIST(pdev, vdev) {
		DP_VDEV_ITERATE_PEER_LIST(vdev, peer) {
			int tid;
			struct dp_tx_tid *tx_tid;

			for (tid = 0; tid < DP_MAX_TIDS; tid++) {
				tx_tid = &peer->tx_capture.tx_tid[tid];

				/* spinlock hold */
				qdf_spin_lock(&tx_tid->tid_lock);
				qdf_nbuf_queue_free(&tx_tid->msdu_comp_q);
				qdf_spin_unlock(&tx_tid->tid_lock);
			}
		}
	}
	qdf_spin_unlock_bh(&pdev->vdev_list_lock);
	qdf_spin_unlock_bh(&soc->peer_ref_mutex);
}

/*
 * dp_config_enh_tx_capture()- API to enable/disable enhanced tx capture
 * @pdev_handle: DP_PDEV handle
 * @val: user provided value
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
dp_config_enh_tx_capture(struct cdp_pdev *pdev_handle, int val)
{
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;

	pdev->tx_capture_enabled = val;

	if (pdev->tx_capture_enabled) {
		dp_soc_set_txrx_ring_map_single(pdev->soc);
		if (!pdev->pktlog_ppdu_stats)
			dp_h2t_cfg_stats_msg_send(pdev,
						  DP_PPDU_STATS_CFG_SNIFFER,
						  pdev->pdev_id);
	} else {
		dp_soc_set_txrx_ring_map(pdev->soc);
		dp_h2t_cfg_stats_msg_send(pdev,
					  DP_PPDU_STATS_CFG_ENH_STATS,
					  pdev->pdev_id);
		dp_iterate_free_peer_msdu_q(pdev);
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * get_number_of_1s(): Function to get number of 1s
 * @value: value to find
 *
 * return: number of 1s
 */
static
inline uint32_t get_number_of_1s(uint32_t value)
{
	uint32_t shift[] = {1, 2, 4, 8, 16};
	uint32_t magic_number[] = { 0x55555555, 0x33333333, 0x0F0F0F0F,
				    0x00FF00FF, 0x0000FFFF};
	uint8_t k = 0;

	for (; k <= 4; k++) {
		value = (value & magic_number[k]) +
			((value >> shift[k]) & magic_number[k]);
	}

	return value;
}

/**
 * dp_tx_print_bitmap(): Function to print bitmap
 * @pdev: dp_pdev
 * @ppdu_desc: ppdu completion descriptor
 * @user_inder: user index
 * @ppdu_id: ppdu id
 *
 * return: status
 */
static
QDF_STATUS dp_tx_print_bitmap(struct dp_pdev *pdev,
			      struct cdp_tx_completion_ppdu *ppdu_desc,
			      uint32_t user_index,
			      uint32_t ppdu_id)
{
	struct cdp_tx_completion_ppdu_user *user;
	uint8_t i;
	uint32_t mpdu_tried;
	uint32_t ba_seq_no;
	uint32_t start_seq;
	uint32_t num_mpdu;
	uint32_t fail_num_mpdu = 0;

	user = &ppdu_desc->user[user_index];

	/* get number of mpdu from ppdu_desc */
	mpdu_tried = user->mpdu_tried_mcast + user->mpdu_tried_ucast;

	ba_seq_no = user->ba_seq_no;
	start_seq = user->start_seq;
	num_mpdu = user->mpdu_success;

	if (user->tid > DP_NON_QOS_TID) {
		QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE, QDF_TRACE_LEVEL_ERROR,
			  "%s: ppdu[%d] peer_id[%d] TID[%d] > NON_QOS_TID!",
			  __func__, ppdu_id, user->peer_id, user->tid);

		return QDF_STATUS_E_FAILURE;
	}

	if (mpdu_tried != num_mpdu) {
		QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE, QDF_TRACE_LEVEL_INFO,
			  "%s: ppdu[%d] peer[%d] tid[%d] ba[%d] start[%d] mpdu_tri[%d] num_mpdu[%d] is_mcast[%d]",
			  __func__, ppdu_id, user->peer_id, user->tid,
			  ba_seq_no, start_seq, mpdu_tried,
			  num_mpdu, user->is_mcast);

		for (i = 0; i < CDP_BA_256_BIT_MAP_SIZE_DWORDS; i++) {
			QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE,
				  QDF_TRACE_LEVEL_ERROR,
				  "ppdu_id[%d] ba_bitmap[0x%x] enqueue_bitmap[0x%x] failed_bitmap[0x%x]",
				  ppdu_id, user->ba_bitmap[i],
				  user->enq_bitmap[i],
				  user->failed_bitmap[i]);

			fail_num_mpdu +=
				get_number_of_1s(user->failed_bitmap[i]);
		}
	}

	if (fail_num_mpdu == num_mpdu && num_mpdu)
		QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE, QDF_TRACE_LEVEL_ERROR,
			  "%s: %d ppdu_id[%d] num_mpdu[%d, %d]",
			  __func__, __LINE__, ppdu_id, num_mpdu, fail_num_mpdu);

	return QDF_STATUS_SUCCESS;
}

static uint32_t dp_tx_update_80211_hdr(struct dp_pdev *pdev,
				       struct dp_peer *peer,
				       void *data,
				       qdf_nbuf_t nbuf)
{
	struct cdp_tx_completion_ppdu *ppdu_desc;
	struct ieee80211_frame *ptr_wh;
	struct ieee80211_qoscntl *ptr_qoscntl;
	uint32_t mpdu_buf_len;
	uint8_t *ptr_hdr;

	ppdu_desc = (struct cdp_tx_completion_ppdu *)data;
	ptr_wh = &peer->tx_capture.tx_wifi_hdr;
	ptr_qoscntl = &peer->tx_capture.tx_qoscntl;

	/*
	 * update framectrl only for first ppdu_id
	 * rest of mpdu will have same frame ctrl
	 * mac address and duration
	 */
	if (ppdu_desc->ppdu_id != peer->tx_capture.tx_wifi_ppdu_id) {
		ptr_wh->i_fc[1] = (ppdu_desc->frame_ctrl & 0xFF00) >> 8;
		ptr_wh->i_fc[0] = (ppdu_desc->frame_ctrl & 0xFF);

		ptr_wh->i_dur[1] = (ppdu_desc->tx_duration & 0xFF00) >> 8;
		ptr_wh->i_dur[0] = (ppdu_desc->tx_duration & 0xFF);

		ptr_qoscntl->i_qos[1] = (ppdu_desc->user[0].qos_ctrl &
					 0xFF00) >> 8;
		ptr_qoscntl->i_qos[0] = (ppdu_desc->user[0].qos_ctrl & 0xFF);

		peer->tx_capture.tx_wifi_ppdu_id = ppdu_desc->ppdu_id;
	}

	mpdu_buf_len = sizeof(struct ieee80211_frame) + LLC_SNAP_HDR_LEN;
	mpdu_buf_len += sizeof(struct ieee80211_qoscntl);

	nbuf->protocol = qdf_htons(ETH_P_802_2);

	/* update ieee80211_frame header */
	if (!qdf_nbuf_push_head(nbuf, mpdu_buf_len)) {
		QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE, QDF_TRACE_LEVEL_ERROR,
			  FL("No headroom"));
		return QDF_STATUS_E_NOMEM;
	}

	ptr_hdr = (void *)qdf_nbuf_data(nbuf);
	qdf_mem_copy(ptr_hdr, ptr_wh, sizeof(struct ieee80211_frame));

	ptr_hdr = ptr_hdr + (sizeof(struct ieee80211_frame));

	/* update qoscntl header */
	qdf_mem_copy(ptr_hdr, ptr_qoscntl, sizeof(struct ieee80211_qoscntl));

	ptr_hdr = ptr_hdr + sizeof(struct ieee80211_qoscntl);

	/* update LLC */
	*ptr_hdr =  LLC_SNAP_LSAP;
	*(ptr_hdr + 1) = LLC_SNAP_LSAP;
	*(ptr_hdr + 2) = LLC_UI;
	*(ptr_hdr + 3) = 0x00;
	*(ptr_hdr + 4) = 0x00;
	*(ptr_hdr + 5) = 0x00;
	/* TYPE: IPV4 ?? */
	*(ptr_hdr + 6) = (ETHERTYPE_IPV4 & 0xFF00) >> 8;
	*(ptr_hdr + 7) = (ETHERTYPE_IPV4 & 0xFF);

	qdf_nbuf_trim_tail(nbuf, qdf_nbuf_len(nbuf) - mpdu_buf_len);
	return 0;
}

#define MAX_MONITOR_HEADER (512)

/**
 * dp_tx_mon_restitch_mpdu_from_msdus(): Function to restitch msdu to mpdu
 * @pdev: dp_pdev
 * @peer: dp_peer
 * @head_msdu: head msdu queue
 *
 * return: status
 */
static uint32_t
dp_tx_mon_restitch_mpdu_from_msdus(struct dp_pdev *pdev,
				   struct dp_peer *peer,
				   struct cdp_tx_completion_ppdu *ppdu_desc,
				   qdf_nbuf_queue_t *head_msdu)
{
	qdf_nbuf_t curr_nbuf = NULL;
	qdf_nbuf_t first_nbuf = NULL;
	qdf_nbuf_t prev_nbuf = NULL;
	qdf_nbuf_t mpdu_nbuf = NULL;

	qdf_nbuf_queue_t *mpdu = NULL;
	uint32_t num_mpdu = 0;

	struct msdu_completion_info *ptr_msdu_info = NULL;

	uint8_t first_msdu = 0;
	uint8_t last_msdu = 0;
	uint32_t frag_list_sum_len = 0;
	uint8_t first_msdu_not_seen = 1;

	num_mpdu = ppdu_desc->num_mpdu;
	mpdu = &ppdu_desc->mpdu_q;

	curr_nbuf = qdf_nbuf_queue_remove(head_msdu);

	while (curr_nbuf) {
		ptr_msdu_info =
			(struct msdu_completion_info *)qdf_nbuf_data(curr_nbuf);

		first_msdu = ptr_msdu_info->first_msdu;
		last_msdu = ptr_msdu_info->last_msdu;

		/* pull msdu_completion_info added in pre header */
		/* pull ethernet header from header */
		qdf_nbuf_pull_head(curr_nbuf,
				   sizeof(struct msdu_completion_info) +
				   sizeof(qdf_ether_header_t));

		if (first_msdu && first_msdu_not_seen) {
			first_nbuf = curr_nbuf;
			frag_list_sum_len = 0;
			first_msdu_not_seen = 0;

			mpdu_nbuf = qdf_nbuf_alloc(pdev->soc->osdev,
						   MAX_MONITOR_HEADER,
						   MAX_MONITOR_HEADER,
						   4, FALSE);

			if (!mpdu_nbuf)
				QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE,
					  QDF_TRACE_LEVEL_FATAL,
					  "MPDU head allocation failed !!!");

			dp_tx_update_80211_hdr(pdev, peer,
					       ppdu_desc, mpdu_nbuf);

			/* update first buffer to previous buffer */
			prev_nbuf = curr_nbuf;

		} else if (first_msdu && !first_msdu_not_seen) {
			QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE,
				  QDF_TRACE_LEVEL_FATAL,
				  "!!!!! NO LAST MSDU\n");
			/*
			 * no last msdu in a mpdu
			 * handle this case
			 */
			qdf_nbuf_free(curr_nbuf);
			/* assert here if no last msdu found */
			QDF_BUG(0);
		} else if (!first_msdu && first_msdu_not_seen) {
			QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE,
				  QDF_TRACE_LEVEL_FATAL,
				  "!!!!! NO FIRST MSDU\n");
			/*
			 * no first msdu in a mpdu
			 * handle this case
			 */
			qdf_nbuf_free(curr_nbuf);
			/* assert here if no first msdu found */
			QDF_BUG(0);
		} else {
			/* update current buffer to previous buffer next */
			prev_nbuf->next = curr_nbuf;
			/* move the previous buffer to next buffer */
			prev_nbuf = prev_nbuf->next;
		}

		frag_list_sum_len += qdf_nbuf_len(curr_nbuf);

		if (last_msdu) {
			/*
			 * first nbuf will hold list of msdu
			 * stored in prev_nbuf
			 */
			qdf_nbuf_append_ext_list(mpdu_nbuf,
						 first_nbuf,
						 frag_list_sum_len);

			/* add mpdu to mpdu queue */
			qdf_nbuf_queue_add(mpdu, mpdu_nbuf);
			first_nbuf = NULL;

			/* next msdu will start with first msdu */
			first_msdu_not_seen = 1;
			goto check_for_next_msdu;
		}

		/* get next msdu from the head_msdu */
		curr_nbuf = qdf_nbuf_queue_remove(head_msdu);

		if (!curr_nbuf) {
			/* msdu missed in list */
			QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE,
				  QDF_TRACE_LEVEL_FATAL,
				  "!!!! WAITING for msdu but list empty !!!!");
		}

		continue;

check_for_next_msdu:
		if (qdf_nbuf_is_queue_empty(head_msdu))
			return 0;
		curr_nbuf = qdf_nbuf_queue_remove(head_msdu);
	}

	return 0;
}

/**
 * dp_tx_msdu_dequeue(): Function to dequeue msdu from peer based tid
 * @peer: dp_peer
 * @ppdu_id: ppdu_id
 * @tid: tid
 * @num_msdu: number of msdu
 * @head: head queue
 * @start_tsf: start tsf from ppdu_desc
 * @end_tsf: end tsf from ppdu_desc
 *
 * return: status
 */
static
uint32_t dp_tx_msdu_dequeue(struct dp_peer *peer, uint32_t ppdu_id,
			    uint16_t tid, uint32_t num_msdu,
			    qdf_nbuf_queue_t *head,
			    uint32_t start_tsf, uint32_t end_tsf)
{
	struct dp_tx_tid *tx_tid  = NULL;
	uint32_t msdu_ppdu_id;
	qdf_nbuf_t curr_msdu = NULL;
	qdf_nbuf_t prev_msdu = NULL;
	qdf_nbuf_t nbuf = NULL;
	struct msdu_completion_info *ptr_msdu_info = NULL;

	uint32_t wbm_tsf;

	uint32_t matched = 0;

	if (qdf_unlikely(!peer))
		return 0;

	tx_tid = &peer->tx_capture.tx_tid[tid];

	if (qdf_unlikely(!tx_tid))
		return 0;

	if (qdf_nbuf_is_queue_empty(&tx_tid->msdu_comp_q))
		return 0;

	curr_msdu = qdf_nbuf_queue_first(&tx_tid->msdu_comp_q);

	while (curr_msdu) {
		if (qdf_nbuf_queue_len(head) == num_msdu)
			break;

		ptr_msdu_info =
			(struct msdu_completion_info *)qdf_nbuf_data(curr_msdu);

		msdu_ppdu_id = ptr_msdu_info->ppdu_id;

		wbm_tsf = ptr_msdu_info->tsf;

		if (msdu_ppdu_id == ppdu_id) {
			matched = 1;

			if (wbm_tsf > start_tsf && wbm_tsf < end_tsf) {
				/*packet found */
			} else if (wbm_tsf < start_tsf) {
				/* lock here */
				qdf_spin_lock(&tx_tid->tid_lock);

				/* remove the aged packet */
				nbuf = qdf_nbuf_queue_remove(
						&tx_tid->msdu_comp_q);

				qdf_spin_unlock(&tx_tid->tid_lock);
				qdf_nbuf_free(nbuf);

				curr_msdu = qdf_nbuf_queue_first(
						&tx_tid->msdu_comp_q);
				prev_msdu = NULL;
				continue;
			} else if (wbm_tsf > end_tsf) {
				/*
				 * Do we need delta in above case.
				 * for searching ppdu out of ppdu tsf
				 */
				break;
			}

			if (qdf_likely(!prev_msdu)) {
				/* lock here */
				qdf_spin_lock(&tx_tid->tid_lock);

				/* remove head */
				curr_msdu = qdf_nbuf_queue_remove(
						&tx_tid->msdu_comp_q);

				qdf_spin_unlock(&tx_tid->tid_lock);
				/* add msdu to head queue */
				qdf_nbuf_queue_add(head, curr_msdu);
				/* get next msdu from msdu_comp_q */
				curr_msdu = qdf_nbuf_queue_first(
						&tx_tid->msdu_comp_q);
				continue;
			} else {
				/* lock here */
				qdf_spin_lock(&tx_tid->tid_lock);

				/* update prev_msdu next to current msdu next */
				prev_msdu->next = curr_msdu->next;
				/* set current msdu next as NULL */
				curr_msdu->next = NULL;
				/* decrement length */
				((qdf_nbuf_queue_t *)(
					&tx_tid->msdu_comp_q))->qlen--;

				qdf_spin_unlock(&tx_tid->tid_lock);

				/* add msdu to head queue */
				qdf_nbuf_queue_add(head, curr_msdu);
				/* set previous msdu to current msdu */
				curr_msdu = prev_msdu->next;
				continue;
			}
		}

		prev_msdu = curr_msdu;
		curr_msdu = prev_msdu->next;
	}

	if (qdf_nbuf_queue_len(head) != num_msdu)
		matched = 0;

	return matched;
}

/**
 * get_mpdu_clone_from_next_ppdu(): Function to clone missing mpdu from
 * next ppdu
 * @nbuf_ppdu_desc_list: nbuf list
 * @ppdu_desc_cnt: ppdu_desc_cnt
 * @missed_seq_no:
 * @ppdu_id: ppdu_id
 * @mpdu_info: cdp_tx_indication_mpdu_info
 *
 * return: void
 */
static
qdf_nbuf_t get_mpdu_clone_from_next_ppdu(qdf_nbuf_t nbuf_ppdu_desc_list[],
					 uint32_t ppdu_desc_cnt,
					 uint32_t missed_seq_no,
					 uint32_t ppdu_id)
{
	uint32_t i = 0;
	uint32_t k = 0;
	uint32_t mpdu_tried = 0;
	uint32_t found = 0;
	uint32_t seq_no = 0;
	struct cdp_tx_completion_ppdu *ppdu_desc = NULL;
	qdf_nbuf_t mpdu = NULL;

	for (i = 1; i < ppdu_desc_cnt; i++) {
		ppdu_desc = (struct cdp_tx_completion_ppdu *)
				qdf_nbuf_data(nbuf_ppdu_desc_list[i]);

		mpdu_tried = ppdu_desc->user[0].mpdu_tried_ucast +
				ppdu_desc->user[0].mpdu_tried_mcast;

		/* check if seq number is between the range */
		if ((missed_seq_no >= ppdu_desc->user[0].start_seq) &&
		    (missed_seq_no <= ppdu_desc->user[0].start_seq +
		     mpdu_tried)) {
			seq_no = ppdu_desc->user[0].start_seq;

			if (ppdu_desc->user[0].failed_bitmap[k] &
			    (1 << (missed_seq_no - seq_no))) {
				found = 1;
				break;
			}
		}
	}

	if (found == 0) {
		/* mpdu not found in sched cmd id */
		QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE, QDF_TRACE_LEVEL_ERROR,
			  "%s: missed seq_no[%d] ppdu_id[%d] [%d] not found!!!",
			  __func__, missed_seq_no, ppdu_id, ppdu_desc_cnt);
		return NULL;
	}

	QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE, QDF_TRACE_LEVEL_ERROR,
		  "%s: seq_no[%d] missed ppdu_id[%d] m[%d] found in ppdu_id[%d]!!",
		  __func__,
		  missed_seq_no, ppdu_id,
		  (missed_seq_no - seq_no), ppdu_desc->ppdu_id);

	mpdu = qdf_nbuf_queue_first(&ppdu_desc->mpdu_q);
	if (!mpdu) {
		/* bitmap shows it found  sequence number, but queue empty */
		/* do we need to allocate skb and send instead of NULL ? */
		QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE, QDF_TRACE_LEVEL_ERROR,
			  "%s: missed seq_no[%d] ppdu_id[%d] [%d] found but queue empty!!!",
			  __func__, missed_seq_no, ppdu_id, ppdu_desc_cnt);
		return NULL;
	}

	for (i = 0; i < (missed_seq_no - seq_no); i++) {
		mpdu = mpdu->next;
		if (!mpdu) {
			/*
			 * bitmap shows it found  sequence number,
			 * but queue empty, do we need to allocate
			 * skb and send instead of NULL ?
			 * add counter here:
			 */
			return NULL;
		}
	}

	return qdf_nbuf_expand(mpdu, 0, 0);
}

/**
 * dp_tx_update_user_mpdu_info(): Function to update mpdu info
 * from ppdu_desc
 * @ppdu_id: ppdu_id
 * @mpdu_info: cdp_tx_indication_mpdu_info
 * @user: cdp_tx_completion_ppdu_user
 *
 * return: void
 */
static void
dp_tx_update_user_mpdu_info(uint32_t ppdu_id,
			    struct cdp_tx_indication_mpdu_info *mpdu_info,
			    struct cdp_tx_completion_ppdu_user *user)
{
	mpdu_info->ppdu_id = ppdu_id;

	mpdu_info->frame_ctrl = user->frame_ctrl;
	mpdu_info->qos_ctrl = user->qos_ctrl;
	mpdu_info->tid = user->tid;
	mpdu_info->ltf_size = user->ltf_size;
	mpdu_info->he_re = user->he_re;
	mpdu_info->txbf = user->txbf;
	mpdu_info->bw = user->bw;
	mpdu_info->nss = user->nss;
	mpdu_info->mcs = user->mcs;
	mpdu_info->preamble = user->preamble;
	mpdu_info->gi = user->gi;

	mpdu_info->ack_rssi = user->ack_rssi[0];
	mpdu_info->tx_rate = user->tx_rate;
	mpdu_info->ldpc = user->ldpc;

	qdf_mem_copy(mpdu_info->mac_address, user->mac_addr, 6);

	mpdu_info->ba_start_seq = user->ba_seq_no;

	qdf_mem_copy(mpdu_info->ba_bitmap, user->ba_bitmap,
		     CDP_BA_256_BIT_MAP_SIZE_DWORDS * sizeof(uint32_t));
}

static inline
void dp_tx_update_sequence_number(qdf_nbuf_t nbuf, uint32_t seq_no)
{
	struct ieee80211_frame *ptr_wh = NULL;
	uint16_t wh_seq = 0;

	if (!nbuf)
		return;

	/* update sequence number in frame header */
	ptr_wh = (struct ieee80211_frame *)qdf_nbuf_data(nbuf);

	wh_seq = (seq_no & 0xFFF) << 4;
	qdf_mem_copy(ptr_wh->i_seq, &wh_seq, sizeof(uint16_t));
}

/**
 * dp_send_mpdu_info_to_stack(): Function to deliver mpdu info to stack
 * to upper layer
 * @pdev: DP pdev handle
 * @nbuf_ppdu_desc_list: ppdu_desc_list per sched cmd id
 * @ppdu_desc_cnt: number of ppdu_desc_cnt
 *
 * return: status
 */
static
QDF_STATUS dp_send_mpdu_info_to_stack(struct dp_pdev *pdev,
				      qdf_nbuf_t nbuf_ppdu_desc_list[],
				      uint32_t ppdu_desc_cnt)
{
	uint32_t ppdu_id;
	uint32_t desc_cnt;

	for (desc_cnt = 0; desc_cnt < ppdu_desc_cnt; desc_cnt++) {
		struct cdp_tx_completion_ppdu *ppdu_desc;
		uint32_t mpdu_tried;
		uint32_t num_mpdu;
		uint32_t k;
		uint32_t i;
		uint32_t seq_no;
		uint32_t len;
		qdf_nbuf_t mpdu_nbuf;

		ppdu_desc = (struct cdp_tx_completion_ppdu *)
			qdf_nbuf_data(nbuf_ppdu_desc_list[desc_cnt]);

		if (!ppdu_desc)
			continue;

		ppdu_id = ppdu_desc->ppdu_id;

		if (ppdu_desc->frame_type == CDP_PPDU_FTYPE_CTRL) {
			struct cdp_tx_indication_info tx_capture_info;
			struct cdp_tx_indication_mpdu_info *mpdu_info;

			mpdu_info = &tx_capture_info.mpdu_info;

			mpdu_info->channel = ppdu_desc->channel;
			mpdu_info->frame_type = ppdu_desc->frame_type;
			mpdu_info->ppdu_start_timestamp =
						ppdu_desc->ppdu_start_timestamp;
			mpdu_info->ppdu_end_timestamp =
						ppdu_desc->ppdu_end_timestamp;
			mpdu_info->tx_duration = ppdu_desc->tx_duration;
			mpdu_info->seq_no = seq_no;
			mpdu_info->num_msdu = ppdu_desc->num_msdu;

			/* update cdp_tx_indication_mpdu_info */
			dp_tx_update_user_mpdu_info(ppdu_desc->ppdu_id,
						    &tx_capture_info.mpdu_info,
						    &ppdu_desc->user[0]);

			tx_capture_info.mpdu_info.channel_num =
				pdev->operating_channel;

			if (qdf_nbuf_is_queue_empty(&ppdu_desc->mpdu_q))
				continue;

			tx_capture_info.mpdu_nbuf =
				qdf_nbuf_queue_remove(&ppdu_desc->mpdu_q);

			/*
			 * send MPDU to osif layer
			 * do we need to update mpdu_info before tranmit
			 * get current mpdu_nbuf
			 */
			dp_wdi_event_handler(WDI_EVENT_TX_DATA, pdev->soc,
					     &tx_capture_info, HTT_INVALID_PEER,
					     WDI_NO_VAL, pdev->pdev_id);

			continue;
		}

		ppdu_id = ppdu_desc->ppdu_id;
		/* find mpdu tried is same as success mpdu */

		mpdu_tried = ppdu_desc->user[0].mpdu_tried_ucast +
				ppdu_desc->user[0].mpdu_tried_mcast;
		num_mpdu = ppdu_desc->user[0].mpdu_success;

		/* get length */
		len = qdf_nbuf_queue_len(&ppdu_desc->mpdu_q);

		/* find list of missing sequence */
		for (i = 0, k = 0; i < mpdu_tried; i++) {
			struct cdp_tx_indication_info tx_capture_info;
			struct cdp_tx_indication_mpdu_info *mpdu_info;

			mpdu_info = &tx_capture_info.mpdu_info;

			/* missed seq number */
			seq_no = ppdu_desc->user[0].start_seq + i;

			if (!(ppdu_desc->user[0].failed_bitmap[k] & 1 << i)) {
				QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE,
					  QDF_TRACE_LEVEL_ERROR,
					  "%s: finding missing seq no: %d in other ppdu list cnt[%d]",
					  __func__, seq_no, ppdu_desc_cnt);

				/* send rest of ppdu_desc list */
				mpdu_nbuf = get_mpdu_clone_from_next_ppdu(
						nbuf_ppdu_desc_list + desc_cnt,
						ppdu_desc_cnt - desc_cnt,
						seq_no,
						ppdu_desc->ppdu_id);

				/* check mpdu_nbuf NULL */
				if (!mpdu_nbuf)
					continue;

				qdf_nbuf_queue_add(&ppdu_desc->mpdu_q,
						   mpdu_nbuf);

			} else {
				/* any error case we need to handle */
			}

			/* k need to be increase, if i increased more than 32 */

			mpdu_info->channel = ppdu_desc->channel;
			mpdu_info->frame_type = ppdu_desc->frame_type;
			mpdu_info->ppdu_start_timestamp =
						ppdu_desc->ppdu_start_timestamp;
			mpdu_info->ppdu_end_timestamp =
						ppdu_desc->ppdu_end_timestamp;
			mpdu_info->tx_duration = ppdu_desc->tx_duration;
			mpdu_info->seq_no = seq_no;
			mpdu_info->num_msdu = ppdu_desc->num_msdu;

			/* update cdp_tx_indication_mpdu_info */
			dp_tx_update_user_mpdu_info(ppdu_id,
						    &tx_capture_info.mpdu_info,
						    &ppdu_desc->user[0]);

			tx_capture_info.mpdu_info.channel_num =
				pdev->operating_channel;

			tx_capture_info.mpdu_nbuf =
				qdf_nbuf_queue_remove(&ppdu_desc->mpdu_q);

			dp_tx_update_sequence_number(tx_capture_info.mpdu_nbuf,
						     seq_no);

			/*
			 * send MPDU to osif layer
			 * do we need to update mpdu_info before tranmit
			 * get current mpdu_nbuf
			 */
			dp_wdi_event_handler(WDI_EVENT_TX_DATA, pdev->soc,
					     &tx_capture_info,
					     HTT_INVALID_PEER,
					     WDI_NO_VAL, pdev->pdev_id);
		}

		qdf_nbuf_queue_free(&ppdu_desc->mpdu_q);
		qdf_nbuf_free(nbuf_ppdu_desc_list[desc_cnt]);
	}

	return QDF_STATUS_SUCCESS;
}

#define SCHED_MAX_PPDU_CNT 10
/**
 * dp_tx_ppdu_stats_process - Deferred PPDU stats handler
 * @context: Opaque work context (PDEV)
 *
 * Return: none
 */
void dp_tx_ppdu_stats_process(void *context)
{
	uint32_t curr_sched_cmdid;
	uint32_t last_ppdu_id;
	uint32_t ppdu_cnt;
	uint32_t ppdu_desc_cnt = 0;
	uint32_t j;
	struct dp_pdev *pdev = (struct dp_pdev *)context;
	struct ppdu_info *ppdu_info, *tmp_ppdu_info;
	uint32_t now_ms = qdf_system_ticks_to_msecs(qdf_system_ticks());
	struct ppdu_info *sched_ppdu_list[SCHED_MAX_PPDU_CNT];
	qdf_nbuf_t nbuf_ppdu_desc_list[SCHED_MAX_PPDU_CNT];
	struct dp_pdev_tx_capture *ptr_tx_cap;

	ptr_tx_cap = &pdev->tx_capture;

	/* Move the PPDU entries to defer list */
	qdf_spin_lock_bh(&ptr_tx_cap->ppdu_stats_lock);
	STAILQ_CONCAT(&ptr_tx_cap->ppdu_stats_defer_queue,
		      &ptr_tx_cap->ppdu_stats_queue);
	ptr_tx_cap->ppdu_stats_queue_depth = 0;
	qdf_spin_unlock_bh(&ptr_tx_cap->ppdu_stats_lock);

	while (!STAILQ_EMPTY(&ptr_tx_cap->ppdu_stats_defer_queue)) {
		ppdu_info =
			STAILQ_FIRST(&ptr_tx_cap->ppdu_stats_defer_queue);
		curr_sched_cmdid = ppdu_info->sched_cmdid;

		ppdu_cnt = 0;
		STAILQ_FOREACH_SAFE(ppdu_info,
				    &ptr_tx_cap->ppdu_stats_defer_queue,
				    ppdu_info_queue_elem, tmp_ppdu_info) {
			if (curr_sched_cmdid != ppdu_info->sched_cmdid)
				break;
			sched_ppdu_list[ppdu_cnt++] = ppdu_info;
			qdf_assert_always(ppdu_cnt <= SCHED_MAX_PPDU_CNT);
		}
		if (ppdu_info && (curr_sched_cmdid == ppdu_info->sched_cmdid) &&
		    ptr_tx_cap->ppdu_stats_next_sched < now_ms)
			break;

		last_ppdu_id = sched_ppdu_list[ppdu_cnt - 1]->ppdu_id;
		STAILQ_REMOVE_HEAD_UNTIL(&ptr_tx_cap->ppdu_stats_defer_queue,
					 sched_ppdu_list[ppdu_cnt - 1],
					 ppdu_info_queue_elem);

		ppdu_desc_cnt = 0;
		/* Process tx buffer list based on last_ppdu_id stored above */
		for (j = 0; j < ppdu_cnt; j++) {
			struct cdp_tx_completion_ppdu *ppdu_desc = NULL;
			struct dp_peer *peer = NULL;
			qdf_nbuf_t nbuf;
			uint32_t retries = 0;
			uint32_t ret = 0;
			qdf_nbuf_queue_t head_msdu;
			uint32_t start_tsf = 0;
			uint32_t end_tsf = 0;
			uint16_t tid = 0;
			uint32_t num_msdu = 0;
			uint32_t qlen = 0;

			qdf_nbuf_queue_init(&head_msdu);

			ppdu_info = sched_ppdu_list[j];
			ppdu_desc = (struct cdp_tx_completion_ppdu *)
				qdf_nbuf_data(ppdu_info->nbuf);
			pdev->tx_ppdu_proc++;

			dp_ppdu_desc_user_stats_update(pdev, ppdu_info);
			/*
			 * While processing/corelating Tx buffers, we should
			 * hold the entire PPDU list for the give sched_cmdid
			 * instead of freeing below.
			 */
			nbuf = ppdu_info->nbuf;
			qdf_mem_free(ppdu_info);

			qdf_assert_always(nbuf);

			ppdu_desc = (struct cdp_tx_completion_ppdu *)
				qdf_nbuf_data(nbuf);

			/* send WDI event */
			if (!pdev->tx_capture_enabled) {
				/**
				 * Deliver PPDU stats only for valid (acked)
				 * data frames if sniffer mode is not enabled.
				 * If sniffer mode is enabled,
				 * PPDU stats for all frames including
				 * mgmt/control frames should be delivered
				 * to upper layer
				 */
				if (pdev->tx_sniffer_enable ||
				    pdev->mcopy_mode) {
					dp_wdi_event_handler(
							WDI_EVENT_TX_PPDU_DESC,
							pdev->soc,
							nbuf,
							HTT_INVALID_PEER,
							WDI_NO_VAL,
							pdev->pdev_id);
				} else {
					if (ppdu_desc->num_mpdu != 0 &&
					    ppdu_desc->num_users != 0 &&
					    (ppdu_desc->frame_ctrl &
					     HTT_FRAMECTRL_DATATYPE)) {
						dp_wdi_event_handler(
							WDI_EVENT_TX_PPDU_DESC,
							pdev->soc,
							nbuf,
							HTT_INVALID_PEER,
							WDI_NO_VAL,
							pdev->pdev_id);
					} else {
						qdf_nbuf_free(nbuf);
					}
				}

				continue;
			}

			peer = dp_peer_find_by_id(pdev->soc,
						  ppdu_desc->user[0].peer_id);

			/*
			 * check whether it is bss peer,
			 * if bss_peer no need to process further
			 */
			if (!peer->bss_peer &&
			    pdev->tx_capture_enabled &&
			    (ppdu_desc->frame_type == CDP_PPDU_FTYPE_DATA) &&
			    (!ppdu_desc->user[0].completion_status)) {
				/* print the bit map */
				dp_tx_print_bitmap(pdev, ppdu_desc,
						   0, ppdu_desc->ppdu_id);
				if (ppdu_desc->user[0].tid > DP_NON_QOS_TID) {
					QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE,
						  QDF_TRACE_LEVEL_ERROR,
						  "%s: ppdu[%d] peer_id[%d] TID[%d] > NON_QOS_TID!",
						  __func__,
						  ppdu_desc->ppdu_id,
						  ppdu_desc->user[0].peer_id,
						  ppdu_desc->user[0].tid);

					dp_peer_unref_del_find_by_id(peer);
					qdf_nbuf_free(nbuf);
					continue;
				}
dequeue_msdu_again:

				tid = ppdu_desc->user[0].tid;
				num_msdu = ppdu_desc->user[0].num_msdu;
				start_tsf = ppdu_desc->ppdu_start_timestamp;
				end_tsf = ppdu_desc->ppdu_end_timestamp;
				/*
				 * retrieve msdu buffer based on ppdu_id & tid
				 * based msdu queue and store it in local queue
				 * sometimes, wbm comes late than per ppdu
				 * stats. Assumption: all packets are SU,
				 * and packets comes in order
				 */
				ret = dp_tx_msdu_dequeue(peer,
							 ppdu_desc->ppdu_id,
							 tid, num_msdu,
							 &head_msdu, start_tsf,
							 end_tsf);

				if (!ret && (++retries < 3)) {
					/* wait for wbm to complete */
					qdf_sleep(100);
					goto dequeue_msdu_again;
				}

				if (qdf_nbuf_is_queue_empty(&head_msdu)) {
					dp_peer_unref_del_find_by_id(peer);
					/*
					 * head msdu is NULL
					 * free nbuf as no processing required
					 */
					qdf_nbuf_free(nbuf);
					continue;
				}

				/*
				 * now head_msdu hold - msdu list for that
				 * particular ppdu_id, restitch mpdu from
				 * msdu and create a mpdu queue
				 */
				dp_tx_mon_restitch_mpdu_from_msdus(pdev,
								   peer,
								   ppdu_desc,
								   &head_msdu);

				/*
				 * sanity: free local head msdu queue
				 * do we need this ?
				 */
				qdf_nbuf_queue_free(&head_msdu);

				nbuf_ppdu_desc_list[ppdu_desc_cnt++] = nbuf;
				qlen = qdf_nbuf_queue_len(&ppdu_desc->mpdu_q);
				/* print ppdu_desc info for debugging purpose */
				QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE,
					  QDF_TRACE_LEVEL_INFO,
					  "%s: ppdu[%d], p_id[%d], tid[%d], n_mpdu[%d %d] n_msdu[%d] retr[%d] qlen[%d] s_tsf[%u] dur[%u] seq[%d] [%d %d]",
					  __func__, ppdu_desc->ppdu_id,
					  ppdu_desc->user[0].peer_id,
					  ppdu_desc->user[0].tid,
					  ppdu_desc->num_mpdu,
					  ppdu_desc->user[0].mpdu_success,
					  ppdu_desc->num_msdu, retries,
					  qlen,
					  ppdu_desc->ppdu_start_timestamp,
					  ppdu_desc->tx_duration,
					  ppdu_desc->user[0].start_seq,
					  ppdu_cnt,
					  ppdu_desc_cnt);

				nbuf->next =
				qdf_nbuf_queue_first(&ppdu_desc->mpdu_q);
			} else if (ppdu_desc->frame_type ==
				   CDP_PPDU_FTYPE_CTRL) {
				nbuf->next =
				qdf_nbuf_queue_first(&ppdu_desc->mpdu_q);

				nbuf_ppdu_desc_list[ppdu_desc_cnt++] = nbuf;
			} else {
				qdf_nbuf_queue_free(&ppdu_desc->mpdu_q);

				qdf_nbuf_free(nbuf);
			}

			dp_peer_unref_del_find_by_id(peer);
		}

		/*
		 * At this point we have mpdu queued per ppdu_desc
		 * based on packet capture flags send mpdu info to upper stack
		 */
		if (pdev->tx_capture_enabled)
			dp_send_mpdu_info_to_stack(pdev,
						   nbuf_ppdu_desc_list,
						   ppdu_desc_cnt);
	}
}

/**
 * dp_ppdu_desc_deliver(): Function to deliver Tx PPDU status descriptor
 * to upper layer
 * @pdev: DP pdev handle
 * @ppdu_info: per PPDU TLV descriptor
 *
 * return: void
 */
void dp_ppdu_desc_deliver(struct dp_pdev *pdev,
			  struct ppdu_info *ppdu_info)
{
	uint32_t now_ms = qdf_system_ticks_to_msecs(qdf_system_ticks());
	struct cdp_tx_completion_ppdu *ppdu_desc = NULL;

	TAILQ_REMOVE(&pdev->ppdu_info_list, ppdu_info, ppdu_info_list_elem);
	pdev->list_depth--;

	ppdu_desc = (struct cdp_tx_completion_ppdu *)
			qdf_nbuf_data(ppdu_info->nbuf);

	/*
	 * before schdeuling, for management frame
	 * copy last mgmt nbuf to ppdu_desc
	 */
	if (ppdu_info->ppdu_id == pdev->mgmtctrl_frm_info.ppdu_id &&
	    pdev->mgmtctrl_frm_info.last_nbuf) {
		qdf_nbuf_queue_add(&ppdu_desc->mpdu_q,
				   pdev->mgmtctrl_frm_info.last_nbuf);
		pdev->mgmtctrl_frm_info.last_nbuf = NULL;
		ppdu_desc->frame_type = CDP_PPDU_FTYPE_CTRL;
		ppdu_desc->ppdu_id = ppdu_info->ppdu_id;
	}

	qdf_spin_lock_bh(&pdev->tx_capture.ppdu_stats_lock);
	STAILQ_INSERT_TAIL(&pdev->tx_capture.ppdu_stats_queue,
			   ppdu_info, ppdu_info_queue_elem);
	pdev->tx_capture.ppdu_stats_queue_depth++;
	qdf_spin_unlock_bh(&pdev->tx_capture.ppdu_stats_lock);

	if ((pdev->tx_capture.ppdu_stats_queue_depth >
	    DP_TX_PPDU_PROC_THRESHOLD) ||
	    (pdev->tx_capture.ppdu_stats_next_sched > now_ms)) {
		qdf_queue_work(0, pdev->tx_capture.ppdu_stats_workqueue,
			       &pdev->tx_capture.ppdu_stats_work);
		pdev->tx_capture.ppdu_stats_next_sched =
			now_ms + DP_TX_PPDU_PROC_TIMEOUT;
	}
}

#endif
