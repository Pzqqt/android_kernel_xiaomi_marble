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

#define MAX_MONITOR_HEADER (512)
#define MAX_DUMMY_FRM_BODY (128)
#define DP_BA_ACK_FRAME_SIZE (sizeof(struct ieee80211_ctlframe_addr2) + 36)
#define DP_ACK_FRAME_SIZE (struct ieee80211_frame_min_one)
#define DP_MAX_MPDU_64 64
#define DP_NUM_WORDS_PER_PPDU_BITMAP_64 (DP_MAX_MPDU_64 >> 5)
#define DP_NUM_BYTES_PER_PPDU_BITMAP_64 (DP_MAX_MPDU_64 >> 3)
#define DP_NUM_BYTES_PER_PPDU_BITMAP (HAL_RX_MAX_MPDU >> 3)
#define DP_IEEE80211_BAR_CTL_TID_S 12
#define DP_IEEE80211_BA_S_SEQ_S 4
#define DP_IEEE80211_BAR_CTL_COMBA 0x0004

/* Macros to handle sequence number bitmaps */

/* HW generated rts frame flag */
#define SEND_WIFIRTS_LEGACY_E 1

/* HW generated 11 AC static bw flag */
#define SEND_WIFIRTS_11AC_STATIC_BW_E 2

/* HW generated 11 AC dynamic bw flag */
#define SEND_WIFIRTS_11AC_DYNAMIC_BW_E 3

/* HW generated cts frame flag */
#define SEND_WIFICTS2SELF_E 4

/* Size (in bits) of a segment of sequence number bitmap */
#define SEQ_SEG_SZ_BITS(_seqarr) (sizeof(_seqarr[0]) << 3)

/* Array index of a segment of sequence number bitmap */
#define SEQ_SEG_INDEX(_seqarr, _seqno) ((_seqno) / SEQ_SEG_SZ_BITS(_seqarr))

/* Bit mask of a seqno within a segment of sequence bitmap */
#define SEQ_SEG_MSK(_seqseg, _index) \
	(1 << ((_index) & ((sizeof(_seqseg) << 3) - 1)))

/* Check seqno bit in a segment of sequence bitmap */
#define SEQ_SEG_BIT(_seqseg, _index) \
	((_seqseg) & SEQ_SEG_MSK((_seqseg), _index))

/* Segment of sequence bitmap containing a given sequence number */
#define SEQ_SEG(_seqarr, _seqno) \
	(_seqarr[(_seqno) / (sizeof(_seqarr[0]) << 3)])

/* Check seqno bit in the sequence bitmap */
#define SEQ_BIT(_seqarr, _seqno) \
	SEQ_SEG_BIT(SEQ_SEG(_seqarr, (_seqno)), (_seqno))

/* Lower 32 mask for timestamp us as completion path has 32 bits timestamp */
#define LOWER_32_MASK 0xFFFFFFFF

/* Maximum time taken to enqueue next mgmt pkt */
#define MAX_MGMT_ENQ_DELAY 10000

/* Schedule id counter mask in ppdu_id */
#define SCH_ID_MASK 0xFF

#ifdef WLAN_TX_PKT_CAPTURE_ENH

/*
 * dp_tx_capture_htt_frame_counter: increment counter for htt_frame_type
 * pdev: DP pdev handle
 * htt_frame_type: htt frame type received from fw
 *
 * return: void
 */
void dp_tx_capture_htt_frame_counter(struct dp_pdev *pdev,
				     uint32_t htt_frame_type)
{
	if (htt_frame_type >= TX_CAP_HTT_MAX_FTYPE)
		return;

	pdev->tx_capture.htt_frame_type[htt_frame_type]++;
}

/*
 * dp_tx_cature_stats: print tx capture stats
 * @pdev: DP PDEV handle
 *
 * return: void
 */
void dp_print_pdev_tx_capture_stats(struct dp_pdev *pdev)
{
	struct dp_pdev_tx_capture *ptr_tx_cap;
	uint8_t i = 0, j = 0;

	ptr_tx_cap = &(pdev->tx_capture);

	DP_PRINT_STATS("tx capture stats:");
	DP_PRINT_STATS(" mgmt control enqueue stats:");
	for (i = 0; i < TXCAP_MAX_TYPE; i++) {
		for (j = 0; j < TXCAP_MAX_SUBTYPE; j++) {
			if (ptr_tx_cap->ctl_mgmt_q[i][j].qlen)
				DP_PRINT_STATS(" ctl_mgmt_q[%d][%d] = queue_len[%d]",
				i, j, ptr_tx_cap->ctl_mgmt_q[i][j].qlen);
		}
	}
	DP_PRINT_STATS(" mgmt control retry queue stats:");
	for (i = 0; i < TXCAP_MAX_TYPE; i++) {
		for (j = 0; j < TXCAP_MAX_SUBTYPE; j++) {
			if (ptr_tx_cap->ctl_mgmt_q[i][j].qlen)
				DP_PRINT_STATS(" retries_ctl_mgmt_q[%d][%d] = queue_len[%d]",
				i, j,
				ptr_tx_cap->retries_ctl_mgmt_q[i][j].qlen);
		}
	}

	for (i = 0; i < TX_CAP_HTT_MAX_FTYPE; i++) {
		if (!ptr_tx_cap->htt_frame_type[i])
			continue;
		DP_PRINT_STATS(" sgen htt frame type[%d] = %d",
			       i, ptr_tx_cap->htt_frame_type[i]);
	}
}

/**
 * dp_peer_or_pdev_tx_cap_enabled - Returns status of tx_cap_enabled
 * based on global per-pdev setting or per-peer setting
 * @pdev: Datapath pdev handle
 * @peer: Datapath peer
 *
 * Return: true if feature is enabled on a per-pdev basis or if
 * enabled for the given peer when per-peer mode is set, false otherwise
 */
inline bool
dp_peer_or_pdev_tx_cap_enabled(struct dp_pdev *pdev,
			       struct dp_peer *peer)
{
	if ((pdev->tx_capture_enabled ==
	     CDP_TX_ENH_CAPTURE_ENABLE_ALL_PEERS) ||
	    ((pdev->tx_capture_enabled ==
	      CDP_TX_ENH_CAPTURE_ENDIS_PER_PEER) &&
	     peer->tx_cap_enabled))
		return true;
	return false;
}

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
		qdf_nbuf_queue_init(&tx_tid->pending_ppdu_q);
		tx_tid->max_ppdu_id = 0;

		/* spinlock create */
		qdf_spinlock_create(&tx_tid->tid_lock);
	}
}

static
void dp_peer_tx_cap_tid_queue_flush(struct dp_peer *peer)
{
	int tid;
	struct dp_tx_tid *tx_tid;

	for (tid = 0; tid < DP_MAX_TIDS; tid++) {
		tx_tid = &peer->tx_capture.tx_tid[tid];

		qdf_spin_lock_bh(&tx_tid->tid_lock);
		qdf_nbuf_queue_free(&tx_tid->msdu_comp_q);
		qdf_spin_unlock_bh(&tx_tid->tid_lock);

		tx_tid->max_ppdu_id = 0;
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

		qdf_spin_lock_bh(&tx_tid->tid_lock);
		qdf_nbuf_queue_free(&tx_tid->msdu_comp_q);
		qdf_spin_unlock_bh(&tx_tid->tid_lock);

		/* spinlock destroy */
		qdf_spinlock_destroy(&tx_tid->tid_lock);
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
		return;
	}
	if (pdev->tx_capture_enabled == CDP_TX_ENH_CAPTURE_ENABLE_ALL_PEERS ||
	    pdev->tx_capture_enabled == CDP_TX_ENH_CAPTURE_ENDIS_PER_PEER) {
		/* invoke WDI event handler here send mgmt pkt here */
		struct ieee80211_frame *wh;
		uint8_t type, subtype;
		struct cdp_tx_mgmt_comp_info *ptr_mgmt_hdr;

		ptr_mgmt_hdr = (struct cdp_tx_mgmt_comp_info *)
				qdf_nbuf_data(nbuf);
		wh = (struct ieee80211_frame *)(qdf_nbuf_data(nbuf) +
			sizeof(struct cdp_tx_mgmt_comp_info));
		type = (wh->i_fc[0] & IEEE80211_FC0_TYPE_MASK) >>
			IEEE80211_FC0_TYPE_SHIFT;
		subtype = (wh->i_fc[0] & IEEE80211_FC0_SUBTYPE_MASK) >>
			IEEE80211_FC0_SUBTYPE_SHIFT;

		if (!ptr_mgmt_hdr->ppdu_id || !ptr_mgmt_hdr->tx_tsf ||
		    (!type && !subtype)) {
			/*
			 * if either ppdu_id and tx_tsf are zero then
			 * storing the payload won't be useful
			 * in constructing the packet
			 * Hence freeing the packet
			 */
			qdf_nbuf_free(nbuf);
			return;
		}

		qdf_spin_lock_bh(
			&pdev->tx_capture.ctl_mgmt_lock[type][subtype]);
		qdf_nbuf_queue_add(&pdev->tx_capture.ctl_mgmt_q[type][subtype],
			nbuf);
		qdf_spin_unlock_bh(
			&pdev->tx_capture.ctl_mgmt_lock[type][subtype]);
		QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE, QDF_TRACE_LEVEL_DEBUG,
			"dlvr mgmt frm(0x%08x): fc 0x%x %x, dur 0x%x%x\n",
			ptr_mgmt_hdr->ppdu_id, wh->i_fc[1], wh->i_fc[0],
			wh->i_dur[1], wh->i_dur[0]);
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
	int i, j;
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
	pdev->tx_capture.ppdu_stats_defer_queue_depth = 0;
	pdev->tx_capture.ppdu_dropped = 0;
	for (i = 0; i < TXCAP_MAX_TYPE; i++) {
		for (j = 0; j < TXCAP_MAX_SUBTYPE; j++) {
			qdf_nbuf_queue_init(
				&pdev->tx_capture.ctl_mgmt_q[i][j]);
			qdf_spinlock_create(
				&pdev->tx_capture.ctl_mgmt_lock[i][j]);
		}
	}
	qdf_mem_zero(&pdev->tx_capture.dummy_ppdu_desc,
		     sizeof(struct cdp_tx_completion_ppdu));
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
	int i, j;

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

	STAILQ_FOREACH_SAFE(ppdu_info,
			    &pdev->tx_capture.ppdu_stats_defer_queue,
			    ppdu_info_queue_elem, tmp_ppdu_info) {
		STAILQ_REMOVE(&pdev->tx_capture.ppdu_stats_defer_queue,
			      ppdu_info, ppdu_info, ppdu_info_queue_elem);
		qdf_nbuf_free(ppdu_info->nbuf);
		qdf_mem_free(ppdu_info);
	}
	for (i = 0; i < TXCAP_MAX_TYPE; i++) {
		for (j = 0; j < TXCAP_MAX_SUBTYPE; j++) {
			qdf_spin_lock_bh(
				&pdev->tx_capture.ctl_mgmt_lock[i][j]);
			qdf_nbuf_queue_free(
				&pdev->tx_capture.ctl_mgmt_q[i][j]);
			qdf_spin_unlock_bh(
				&pdev->tx_capture.ctl_mgmt_lock[i][j]);
			qdf_spinlock_destroy(
				&pdev->tx_capture.ctl_mgmt_lock[i][j]);
		}
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

	if (ts->tid > DP_MAX_TIDS) {
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
	msdu_comp_info->status = ts->status;

	/* update max ppdu_id */
	tx_tid->max_ppdu_id = ts->ppdu_id;
	pdev->tx_capture.last_msdu_id = ts->ppdu_id;

	QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE, QDF_TRACE_LEVEL_INFO,
		  "msdu_completion: ppdu_id[%d] peer_id[%d] tid[%d] rel_src[%d] status[%d] tsf[%u] A[%d] CNT[%d]",
		  ts->ppdu_id, ts->peer_id, ts->tid, ts->release_src,
		  ts->status, ts->tsf, ts->msdu_part_of_amsdu,
		  ts->transmit_cnt);

	/* lock here */
	qdf_spin_lock_bh(&tx_tid->tid_lock);

	/* add nbuf to tail queue per peer tid */
	qdf_nbuf_queue_add(&tx_tid->msdu_comp_q, netbuf);

	qdf_spin_unlock_bh(&tx_tid->tid_lock);

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

	if (peer && dp_peer_or_pdev_tx_cap_enabled(desc->pdev, peer) &&
	    ((ts->status == HAL_TX_TQM_RR_FRAME_ACKED) ||
	    (ts->status == HAL_TX_TQM_RR_REM_CMD_TX) ||
	    ((ts->status == HAL_TX_TQM_RR_REM_CMD_AGED) && ts->transmit_cnt))) {
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
	int last_set_bit;

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

	/* Handle sequence no. wraparound */
	if (start_seq <= ba_seq_no) {
		diff = ba_seq_no - start_seq;
		/* Sequence delta of more than 2048 is considered wraparound
		 * and we extend start_seq to be more than ba_seq just to
		 * adjust failed_bitmap
		 */
		if (qdf_unlikely(diff > (IEEE80211_SEQ_MAX / 2))) {
			diff = (start_seq - ba_seq_no) &
				(IEEE80211_SEQ_MAX - 1);
			start_seq = ba_seq_no + diff;
		}
	} else {
		diff = start_seq - ba_seq_no;
		/* Sequence delta of more than 2048 is considered wraparound
		 * and we extend ba_seq to be more than start_seq just to
		 * adjust failed_bitmap
		 */
		if (qdf_unlikely(diff > (IEEE80211_SEQ_MAX / 2))) {
			diff = (ba_seq_no - start_seq) &
				(IEEE80211_SEQ_MAX - 1);
			ba_seq_no = start_seq + diff;
		}
	}

	/* Adjust failed_bitmap to start from same seq_no as enq_bitmap */
	last_set_bit = 0;
	if (start_seq <= ba_seq_no) {
		bitmask = (1 << diff) - 1;
		for (i = 0; i < size; i++) {
			ba_bitmap = user->ba_bitmap[i];

			user->failed_bitmap[i] = (ba_bitmap << diff);
			user->failed_bitmap[i] |= (bitmask & carry);

			carry = ((ba_bitmap & (bitmask << (32 - diff))) >>
				(32 - diff));

			user->failed_bitmap[i] = user->enq_bitmap[i] &
						 user->failed_bitmap[i];
			if (user->enq_bitmap[i]) {
				last_set_bit = i * 32 +
					qdf_fls(user->enq_bitmap[i]) - 1;
			}
		}
	} else {
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
			if (user->enq_bitmap[i]) {
				last_set_bit = i * 32 +
					qdf_fls(user->enq_bitmap[i]) - 1;
			}
		}
	}
	user->last_enq_seq = user->start_seq + last_set_bit;
	user->ba_size = user->last_enq_seq - user->start_seq + 1;
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
				qdf_nbuf_t ppdu_nbuf = NULL;
				struct cdp_tx_completion_ppdu *ppdu_desc =
					NULL;
				int i;
				tx_tid = &peer->tx_capture.tx_tid[tid];

				/* spinlock hold */
				qdf_spin_lock_bh(&tx_tid->tid_lock);
				qdf_nbuf_queue_free(&tx_tid->msdu_comp_q);
				qdf_spin_unlock_bh(&tx_tid->tid_lock);
				while ((ppdu_nbuf = qdf_nbuf_queue_remove(
				       &tx_tid->pending_ppdu_q))) {
					ppdu_desc =
					(struct cdp_tx_completion_ppdu *)
					qdf_nbuf_data(ppdu_nbuf);
					if (!ppdu_desc->mpdus) {
						qdf_nbuf_free(ppdu_nbuf);
						continue;
					}
					for (i = 0; i <
					   ppdu_desc->user[0].ba_size; i++) {
						if (!ppdu_desc->mpdus[i])
							continue;
						qdf_nbuf_free(
							ppdu_desc->mpdus[i]);
					}
					qdf_mem_free(ppdu_desc->mpdus);
					ppdu_desc->mpdus = NULL;
					qdf_nbuf_free(ppdu_nbuf);
				}
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
dp_config_enh_tx_capture(struct cdp_pdev *pdev_handle, uint8_t val)
{
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;
	int i, j;

	qdf_spin_lock(&pdev->tx_capture.config_lock);
	pdev->tx_capture_enabled = val;

	if (pdev->tx_capture_enabled == CDP_TX_ENH_CAPTURE_ENABLE_ALL_PEERS ||
	    pdev->tx_capture_enabled == CDP_TX_ENH_CAPTURE_ENDIS_PER_PEER) {
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
		for (i = 0; i < TXCAP_MAX_TYPE; i++) {
			for (j = 0; j < TXCAP_MAX_SUBTYPE; j++) {
				qdf_spin_lock_bh(
					&pdev->tx_capture.ctl_mgmt_lock[i][j]);
				qdf_nbuf_queue_free(
					&pdev->tx_capture.ctl_mgmt_q[i][j]);
				qdf_spin_unlock_bh(
					&pdev->tx_capture.ctl_mgmt_lock[i][j]);
			}
		}
	}

	qdf_spin_unlock(&pdev->tx_capture.config_lock);
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

	if (user->tid > DP_MAX_TIDS) {
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
				  QDF_TRACE_LEVEL_INFO,
				  "ppdu_id[%d] ba_bitmap[0x%x] enqueue_bitmap[0x%x] failed_bitmap[0x%x]",
				  ppdu_id, user->ba_bitmap[i],
				  user->enq_bitmap[i],
				  user->failed_bitmap[i]);

			fail_num_mpdu +=
				get_number_of_1s(user->failed_bitmap[i]);
		}
	}

	if (fail_num_mpdu == num_mpdu && num_mpdu)
		QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE, QDF_TRACE_LEVEL_DEBUG,
			  "%s: %d ppdu_id[%d] num_mpdu[%d, %d]",
			  __func__, __LINE__, ppdu_id, num_mpdu, fail_num_mpdu);

	return QDF_STATUS_SUCCESS;
}

static uint32_t dp_tx_update_80211_hdr(struct dp_pdev *pdev,
				       struct dp_peer *peer,
				       void *data,
				       qdf_nbuf_t nbuf,
				       uint16_t ether_type,
				       uint8_t *src_addr)
{
	struct cdp_tx_completion_ppdu *ppdu_desc;
	struct ieee80211_frame *ptr_wh;
	struct ieee80211_qoscntl *ptr_qoscntl;
	uint32_t mpdu_buf_len;
	uint8_t *ptr_hdr;
	uint16_t eth_type = qdf_htons(ether_type);

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
		/* Update Addr 3 (SA) with SA derived from ether packet */
		qdf_mem_copy(ptr_wh->i_addr3, src_addr, QDF_MAC_ADDR_SIZE);

		peer->tx_capture.tx_wifi_ppdu_id = ppdu_desc->ppdu_id;
	}

	mpdu_buf_len = sizeof(struct ieee80211_frame) + LLC_SNAP_HDR_LEN;
	if (qdf_likely(ppdu_desc->user[0].tid != DP_NON_QOS_TID))
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
	if (qdf_likely(ppdu_desc->user[0].tid != DP_NON_QOS_TID)) {
		qdf_mem_copy(ptr_hdr, ptr_qoscntl,
			     sizeof(struct ieee80211_qoscntl));
		ptr_hdr = ptr_hdr + sizeof(struct ieee80211_qoscntl);
	}

	/* update LLC */
	*ptr_hdr =  LLC_SNAP_LSAP;
	*(ptr_hdr + 1) = LLC_SNAP_LSAP;
	*(ptr_hdr + 2) = LLC_UI;
	*(ptr_hdr + 3) = 0x00;
	*(ptr_hdr + 4) = 0x00;
	*(ptr_hdr + 5) = 0x00;
	*(ptr_hdr + 6) = (eth_type & 0xFF00) >> 8;
	*(ptr_hdr + 7) = (eth_type & 0xFF);


	qdf_nbuf_trim_tail(nbuf, qdf_nbuf_len(nbuf) - mpdu_buf_len);
	return 0;
}

/**
 * dp_tx_mon_restitch_mpdu(): Function to restitch msdu to mpdu
 * @pdev: dp_pdev
 * @peer: dp_peer
 * @head_msdu: head msdu queue
 *
 * return: status
 */
static uint32_t
dp_tx_mon_restitch_mpdu(struct dp_pdev *pdev, struct dp_peer *peer,
			struct cdp_tx_completion_ppdu *ppdu_desc,
			qdf_nbuf_queue_t *head_msdu,
			qdf_nbuf_queue_t *mpdu_q)
{
	qdf_nbuf_t curr_nbuf = NULL;
	qdf_nbuf_t first_nbuf = NULL;
	qdf_nbuf_t prev_nbuf = NULL;
	qdf_nbuf_t mpdu_nbuf = NULL;

	struct msdu_completion_info *ptr_msdu_info = NULL;

	uint8_t first_msdu = 0;
	uint8_t last_msdu = 0;
	uint32_t frag_list_sum_len = 0;
	uint8_t first_msdu_not_seen = 1;
	uint16_t ether_type = 0;
	qdf_ether_header_t *eh = NULL;

	curr_nbuf = qdf_nbuf_queue_remove(head_msdu);

	while (curr_nbuf) {
		ptr_msdu_info =
			(struct msdu_completion_info *)qdf_nbuf_data(curr_nbuf);

		first_msdu = ptr_msdu_info->first_msdu;
		last_msdu = ptr_msdu_info->last_msdu;

		eh = (qdf_ether_header_t *)(curr_nbuf->data +
					   sizeof(struct msdu_completion_info));
		ether_type = eh->ether_type;

		/* pull msdu_completion_info added in pre header */
		qdf_nbuf_pull_head(curr_nbuf,
			sizeof(struct msdu_completion_info));

		if (first_msdu && first_msdu_not_seen) {
			first_nbuf = curr_nbuf;
			frag_list_sum_len = 0;
			first_msdu_not_seen = 0;

			/* pull ethernet header from first MSDU alone */
			qdf_nbuf_pull_head(curr_nbuf,
					   sizeof(qdf_ether_header_t));
			mpdu_nbuf = qdf_nbuf_alloc(pdev->soc->osdev,
						   MAX_MONITOR_HEADER,
						   MAX_MONITOR_HEADER,
						   4, FALSE);

			if (!mpdu_nbuf) {
				QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE,
					  QDF_TRACE_LEVEL_FATAL,
					  "MPDU head allocation failed !!!");
				goto free_ppdu_desc_mpdu_q;
			}

			dp_tx_update_80211_hdr(pdev, peer,
					       ppdu_desc, mpdu_nbuf,
					       ether_type, eh->ether_shost);

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
			/*
			 * No last msdu found because WBM comes out
			 * of order, free the pkt
			 */
			goto free_ppdu_desc_mpdu_q;
		} else if (!first_msdu && first_msdu_not_seen) {
			QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE,
				  QDF_TRACE_LEVEL_FATAL,
				  "!!!!! NO FIRST MSDU\n");
			/*
			 * no first msdu in a mpdu
			 * handle this case
			 */
			qdf_nbuf_free(curr_nbuf);
			/*
			 * no first msdu found beacuse WBM comes out
			 * of order, free the pkt
			 */
			goto free_ppdu_desc_mpdu_q;
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
			qdf_nbuf_queue_add(mpdu_q, mpdu_nbuf);
			first_nbuf = NULL;
			mpdu_nbuf = NULL;

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

free_ppdu_desc_mpdu_q:
	/* free already chained msdu pkt */
	while (first_nbuf) {
		curr_nbuf = first_nbuf;
		first_nbuf = first_nbuf->next;
		qdf_nbuf_free(curr_nbuf);
	}

	/* free allocated mpdu hdr */
	if (mpdu_nbuf)
		qdf_nbuf_free(mpdu_nbuf);
	/* free queued remaining msdu pkt per ppdu */
	qdf_nbuf_queue_free(head_msdu);
	/* free queued mpdu per ppdu */
	qdf_nbuf_queue_free(mpdu_q);
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
			    qdf_nbuf_queue_t *head_xretries,
			    uint32_t start_tsf, uint32_t end_tsf)
{
	struct dp_tx_tid *tx_tid  = NULL;
	uint32_t msdu_ppdu_id;
	qdf_nbuf_t curr_msdu = NULL;
	qdf_nbuf_t prev_msdu = NULL;
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

	/* lock here */
	qdf_spin_lock_bh(&tx_tid->tid_lock);

	curr_msdu = qdf_nbuf_queue_first(&tx_tid->msdu_comp_q);

	while (curr_msdu) {
		if (qdf_nbuf_queue_len(head) == num_msdu) {
			matched = 1;
			break;
		}

		ptr_msdu_info =
			(struct msdu_completion_info *)qdf_nbuf_data(curr_msdu);

		msdu_ppdu_id = ptr_msdu_info->ppdu_id;

		wbm_tsf = ptr_msdu_info->tsf;

		if ((ptr_msdu_info->status == HAL_TX_TQM_RR_REM_CMD_TX) ||
		    (ptr_msdu_info->status == HAL_TX_TQM_RR_REM_CMD_AGED)) {
			/* Frames removed due to excessive retries */
			qdf_nbuf_queue_remove(&tx_tid->msdu_comp_q);
			qdf_nbuf_queue_add(head_xretries, curr_msdu);
			curr_msdu = qdf_nbuf_queue_first(
				&tx_tid->msdu_comp_q);
			prev_msdu = NULL;
			continue;
		}

		if (wbm_tsf > end_tsf) {
			/* PPDU being matched is older than MSDU at head of
			 * completion queue. Return matched=1 to skip PPDU
			 */
			matched = 1;
			break;
		}

		if (wbm_tsf && (wbm_tsf < start_tsf)) {
			/* remove the aged packet */
			qdf_nbuf_queue_remove(&tx_tid->msdu_comp_q);
			qdf_nbuf_free(curr_msdu);

			curr_msdu = qdf_nbuf_queue_first(
					&tx_tid->msdu_comp_q);
			prev_msdu = NULL;
			continue;
		}
		if (msdu_ppdu_id == ppdu_id) {
			if (qdf_likely(!prev_msdu)) {
				/* remove head */
				qdf_nbuf_queue_remove(&tx_tid->msdu_comp_q);

				/* add msdu to head queue */
				qdf_nbuf_queue_add(head, curr_msdu);
				/* get next msdu from msdu_comp_q */
				curr_msdu = qdf_nbuf_queue_first(
						&tx_tid->msdu_comp_q);
				continue;
			} else {
				/* update prev_msdu next to current msdu next */
				prev_msdu->next = curr_msdu->next;
				/* set current msdu next as NULL */
				curr_msdu->next = NULL;
				/* decrement length */
				((qdf_nbuf_queue_t *)(
					&tx_tid->msdu_comp_q))->qlen--;

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

	qdf_spin_unlock_bh(&tx_tid->tid_lock);

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
					 uint16_t missed_seq_no,
					 uint16_t peer_id, uint32_t ppdu_id)
{
	uint32_t i = 0;
	uint32_t found = 0;
	uint32_t seq_no = 0;
	struct cdp_tx_completion_ppdu *ppdu_desc = NULL;
	qdf_nbuf_t mpdu = NULL;

	for (i = 1; i < ppdu_desc_cnt; i++) {
		ppdu_desc = (struct cdp_tx_completion_ppdu *)
				qdf_nbuf_data(nbuf_ppdu_desc_list[i]);

		/* check if seq number is between the range */
		if ((peer_id == ppdu_desc->user[0].peer_id) &&
		    ((missed_seq_no >= ppdu_desc->user[0].start_seq) &&
		    (missed_seq_no <= ppdu_desc->user[0].last_enq_seq))) {
			seq_no = ppdu_desc->user[0].start_seq;
			if (SEQ_BIT(ppdu_desc->user[0].failed_bitmap,
			    (missed_seq_no - seq_no))) {
				found = 1;
				break;
			}
		}
	}

	if (found == 0) {
		/* mpdu not found in sched cmd id */
		QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE, QDF_TRACE_LEVEL_DEBUG,
			  "%s: missed seq_no[%d] ppdu_id[%d] [%d] not found!!!",
			  __func__, missed_seq_no, ppdu_id, ppdu_desc_cnt);
		return NULL;
	}

	QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE, QDF_TRACE_LEVEL_DEBUG,
		  "%s: seq_no[%d] missed ppdu_id[%d] m[%d] found in ppdu_id[%d]!!",
		  __func__,
		  missed_seq_no, ppdu_id,
		  (missed_seq_no - seq_no), ppdu_desc->ppdu_id);

	mpdu = qdf_nbuf_queue_first(&ppdu_desc->mpdu_q);
	if (!mpdu) {
		/* bitmap shows it found  sequence number, but
		 * MPDU not found in PPDU
		 */
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

	return qdf_nbuf_copy_expand(mpdu, MAX_MONITOR_HEADER, 0);
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
	mpdu_info->ppdu_cookie = user->ppdu_cookie;

	mpdu_info->long_retries = user->long_retries;
	mpdu_info->short_retries = user->short_retries;
	mpdu_info->completion_status = user->completion_status;

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

static inline
void dp_update_frame_ctrl_from_frame_type(void *desc)
{
	struct cdp_tx_completion_ppdu *ppdu_desc = desc;

	/* frame control is not set properly, sometimes it is zero */
	switch (ppdu_desc->htt_frame_type) {
	case HTT_STATS_FTYPE_SGEN_NDPA:
	case HTT_STATS_FTYPE_SGEN_NDP:
	case HTT_STATS_FTYPE_SGEN_AX_NDPA:
	case HTT_STATS_FTYPE_SGEN_AX_NDP:
		ppdu_desc->frame_ctrl = (IEEE80211_FC0_SUBTYPE_NDPA |
					 IEEE80211_FC0_TYPE_CTL);
	break;
	case HTT_STATS_FTYPE_SGEN_BRP:
	case HTT_STATS_FTYPE_SGEN_MU_BRP:
		ppdu_desc->frame_ctrl = (IEEE80211_FC0_SUBTYPE_BRPOLL |
					 IEEE80211_FC0_TYPE_CTL);
	break;
	case HTT_STATS_FTYPE_SGEN_RTS:
	case HTT_STATS_FTYPE_SGEN_MU_RTS:
		ppdu_desc->frame_ctrl = (IEEE80211_FC0_SUBTYPE_RTS |
					 IEEE80211_FC0_TYPE_CTL);
	break;
	case HTT_STATS_FTYPE_SGEN_CTS:
		ppdu_desc->frame_ctrl = (IEEE80211_FC0_SUBTYPE_CTS |
					 IEEE80211_FC0_TYPE_CTL);
	break;
	case HTT_STATS_FTYPE_SGEN_CFEND:
		ppdu_desc->frame_ctrl = (IEEE80211_FC0_SUBTYPE_CF_END |
					 IEEE80211_FC0_TYPE_CTL);
	break;
	case HTT_STATS_FTYPE_SGEN_MU_TRIG:
		ppdu_desc->frame_ctrl = (IEEE80211_FC0_SUBTYPE_TRIGGER |
					 IEEE80211_FC0_TYPE_CTL);
	break;
	case HTT_STATS_FTYPE_SGEN_BAR:
	case HTT_STATS_FTYPE_SGEN_MU_BAR:
		ppdu_desc->frame_ctrl = (IEEE80211_FC0_SUBTYPE_BAR |
					  IEEE80211_FC0_TYPE_CTL);
	break;
	}
}

/**
 * dp_send_dummy_mpdu_info_to_stack(): send dummy payload to stack
 * to upper layer if complete
 * @pdev: DP pdev handle
 * @desc: cdp tx completion ppdu desc
 *
 * return: status
 */
static inline
QDF_STATUS dp_send_dummy_mpdu_info_to_stack(struct dp_pdev *pdev,
					  void *desc)
{
	struct dp_peer *peer;
	struct cdp_tx_completion_ppdu *ppdu_desc = desc;
	struct cdp_tx_completion_ppdu_user *user = &ppdu_desc->user[0];
	struct ieee80211_ctlframe_addr2 *wh_min;
	uint16_t frame_ctrl_le, duration_le;
	struct cdp_tx_indication_info tx_capture_info;
	struct cdp_tx_indication_mpdu_info *mpdu_info;
	uint8_t type, subtype;

	qdf_mem_set(&tx_capture_info,
		    sizeof(struct cdp_tx_indication_info),
		    0);

	tx_capture_info.mpdu_nbuf =
		qdf_nbuf_alloc(pdev->soc->osdev,
		MAX_MONITOR_HEADER + MAX_DUMMY_FRM_BODY,
		MAX_MONITOR_HEADER,
		4, FALSE);
	if (!tx_capture_info.mpdu_nbuf)
		return QDF_STATUS_E_ABORTED;

	mpdu_info = &tx_capture_info.mpdu_info;

	mpdu_info->resp_type = ppdu_desc->resp_type;
	mpdu_info->mprot_type = ppdu_desc->mprot_type;
	mpdu_info->rts_success = ppdu_desc->rts_success;
	mpdu_info->rts_failure = ppdu_desc->rts_failure;

	/* update cdp_tx_indication_mpdu_info */
	dp_tx_update_user_mpdu_info(ppdu_desc->bar_ppdu_id,
				    &tx_capture_info.mpdu_info,
				    &ppdu_desc->user[0]);
	tx_capture_info.ppdu_desc = ppdu_desc;

	mpdu_info->ppdu_id = ppdu_desc->ppdu_id;
	mpdu_info->channel_num = pdev->operating_channel;
	mpdu_info->channel = ppdu_desc->channel;
	mpdu_info->frame_type = ppdu_desc->frame_type;
	mpdu_info->ppdu_start_timestamp = ppdu_desc->ppdu_start_timestamp;
	mpdu_info->ppdu_end_timestamp = ppdu_desc->ppdu_end_timestamp;
	mpdu_info->tx_duration = ppdu_desc->tx_duration;
	mpdu_info->seq_no = user->start_seq;
	qdf_mem_copy(mpdu_info->mac_address, user->mac_addr, QDF_MAC_ADDR_SIZE);

	mpdu_info->ba_start_seq = user->ba_seq_no;
	qdf_mem_copy(mpdu_info->ba_bitmap, user->ba_bitmap,
		     CDP_BA_256_BIT_MAP_SIZE_DWORDS * sizeof(uint32_t));

	mpdu_info->frame_ctrl = ppdu_desc->frame_ctrl;

	type = (ppdu_desc->frame_ctrl & IEEE80211_FC0_TYPE_MASK);
	subtype = (ppdu_desc->frame_ctrl & IEEE80211_FC0_SUBTYPE_MASK);

	if (type == IEEE80211_FC0_TYPE_CTL &&
	    subtype == IEEE80211_FC0_SUBTYPE_BAR) {
		mpdu_info->frame_ctrl = (IEEE80211_FC0_SUBTYPE_BAR |
					  IEEE80211_FC0_TYPE_CTL);
		mpdu_info->ppdu_id = ppdu_desc->bar_ppdu_id;
		mpdu_info->ppdu_start_timestamp =
					ppdu_desc->bar_ppdu_start_timestamp;
		mpdu_info->ppdu_end_timestamp =
					ppdu_desc->bar_ppdu_end_timestamp;
		mpdu_info->tx_duration = ppdu_desc->bar_tx_duration;
	}

	wh_min = (struct ieee80211_ctlframe_addr2 *)
		qdf_nbuf_data(
		tx_capture_info.mpdu_nbuf);
	qdf_mem_zero(wh_min, MAX_DUMMY_FRM_BODY);
	frame_ctrl_le =
		qdf_cpu_to_le16(mpdu_info->frame_ctrl);
	duration_le =
		qdf_cpu_to_le16(ppdu_desc->bar_tx_duration);
	wh_min->i_fc[1] = (frame_ctrl_le & 0xFF00) >> 8;
	wh_min->i_fc[0] = (frame_ctrl_le & 0xFF);
	wh_min->i_aidordur[1] = (duration_le & 0xFF00) >> 8;
	wh_min->i_aidordur[0] = (duration_le & 0xFF);
	qdf_mem_copy(wh_min->i_addr1,
		mpdu_info->mac_address,
		QDF_MAC_ADDR_SIZE);

	peer = dp_peer_find_by_id(pdev->soc, user->peer_id);
	if (peer) {
		struct dp_vdev *vdev = NULL;
		vdev = peer->vdev;
		if (vdev)
			qdf_mem_copy(wh_min->i_addr2,
				     vdev->mac_addr.raw,
				     QDF_MAC_ADDR_SIZE);
		dp_peer_unref_del_find_by_id(peer);
	}

	qdf_nbuf_set_pktlen(tx_capture_info.mpdu_nbuf,
		sizeof(*wh_min));
	QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE,
		QDF_TRACE_LEVEL_DEBUG,
		"HTT_FTYPE[%d] frm(0x%08x): fc %x %x, dur 0x%x%x\n",
		ppdu_desc->htt_frame_type, mpdu_info->ppdu_id,
		wh_min->i_fc[1], wh_min->i_fc[0],
		wh_min->i_aidordur[1], wh_min->i_aidordur[0]);
	/*
	 * send MPDU to osif layer
	 */
	dp_wdi_event_handler(WDI_EVENT_TX_DATA, pdev->soc,
			     &tx_capture_info, HTT_INVALID_PEER,
			     WDI_NO_VAL, pdev->pdev_id);

	if (tx_capture_info.mpdu_nbuf)
		qdf_nbuf_free(tx_capture_info.mpdu_nbuf);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_send_dummy_rts_cts_frame(): send dummy rts and cts frame out
 * to upper layer if complete
 * @pdev: DP pdev handle
 * @cur_ppdu_desc: cdp tx completion ppdu desc
 *
 * return: void
 */
static
void dp_send_dummy_rts_cts_frame(struct dp_pdev *pdev,
				 struct cdp_tx_completion_ppdu *cur_ppdu_desc)
{
	struct cdp_tx_completion_ppdu *ppdu_desc;
	struct dp_pdev_tx_capture *ptr_tx_cap;
	struct dp_peer *peer;
	uint8_t rts_send;

	rts_send = 0;
	ptr_tx_cap = &pdev->tx_capture;
	ppdu_desc = &ptr_tx_cap->dummy_ppdu_desc;

	ppdu_desc->channel = cur_ppdu_desc->channel;
	ppdu_desc->num_mpdu = 1;
	ppdu_desc->num_msdu = 1;
	ppdu_desc->user[0].ppdu_type = HTT_PPDU_STATS_PPDU_TYPE_SU;
	ppdu_desc->bar_num_users = 0;
	ppdu_desc->num_users = 1;

	if (cur_ppdu_desc->mprot_type == SEND_WIFIRTS_LEGACY_E ||
	    cur_ppdu_desc->mprot_type == SEND_WIFIRTS_11AC_DYNAMIC_BW_E ||
	    cur_ppdu_desc->mprot_type == SEND_WIFIRTS_11AC_STATIC_BW_E) {
		rts_send = 1;
		/*
		 *  send dummy RTS frame followed by CTS
		 *  update frame_ctrl and htt_frame_type
		 */
		ppdu_desc->htt_frame_type = HTT_STATS_FTYPE_SGEN_RTS;
		ppdu_desc->frame_type = CDP_PPDU_FTYPE_CTRL;
		ppdu_desc->ppdu_start_timestamp =
				cur_ppdu_desc->ppdu_start_timestamp;
		ppdu_desc->ppdu_end_timestamp =
				cur_ppdu_desc->ppdu_end_timestamp;
		ppdu_desc->user[0].peer_id = cur_ppdu_desc->user[0].peer_id;
		ppdu_desc->frame_ctrl = (IEEE80211_FC0_SUBTYPE_RTS |
					 IEEE80211_FC0_TYPE_CTL);
		qdf_mem_copy(&ppdu_desc->user[0].mac_addr,
			     &cur_ppdu_desc->user[0].mac_addr,
			     QDF_MAC_ADDR_SIZE);

		dp_send_dummy_mpdu_info_to_stack(pdev, ppdu_desc);
	}

	if ((rts_send && cur_ppdu_desc->rts_success) ||
	    cur_ppdu_desc->mprot_type == SEND_WIFICTS2SELF_E) {
		/* send dummy CTS frame */
		ppdu_desc->htt_frame_type = HTT_STATS_FTYPE_SGEN_CTS;
		ppdu_desc->frame_type = CDP_PPDU_FTYPE_CTRL;
		ppdu_desc->frame_ctrl = (IEEE80211_FC0_SUBTYPE_CTS |
					 IEEE80211_FC0_TYPE_CTL);
		ppdu_desc->ppdu_start_timestamp =
				cur_ppdu_desc->ppdu_start_timestamp;
		ppdu_desc->ppdu_end_timestamp =
				cur_ppdu_desc->ppdu_end_timestamp;
		ppdu_desc->user[0].peer_id = cur_ppdu_desc->user[0].peer_id;
		peer = dp_peer_find_by_id(pdev->soc,
					  cur_ppdu_desc->user[0].peer_id);
		if (peer) {
			struct dp_vdev *vdev = NULL;

			vdev = peer->vdev;
			if (vdev)
				qdf_mem_copy(&ppdu_desc->user[0].mac_addr,
					     vdev->mac_addr.raw,
					     QDF_MAC_ADDR_SIZE);
			dp_peer_unref_del_find_by_id(peer);
		}

		dp_send_dummy_mpdu_info_to_stack(pdev, ppdu_desc);
	}
}

/**
 * dp_send_data_to_stack(): Function to deliver mpdu info to stack
 * to upper layer
 * @pdev: DP pdev handle
 * @nbuf_ppdu_desc_list: ppdu_desc_list per sched cmd id
 * @ppdu_desc_cnt: number of ppdu_desc_cnt
 *
 * return: status
 */
static
void dp_send_data_to_stack(struct dp_pdev *pdev,
			   struct cdp_tx_completion_ppdu *ppdu_desc)
{
	struct cdp_tx_indication_info tx_capture_info;
	struct cdp_tx_indication_mpdu_info *mpdu_info;
	int i;
	uint32_t seq_no, start_seq;
	uint32_t ppdu_id = ppdu_desc->ppdu_id;

	qdf_mem_set(&tx_capture_info,
		    sizeof(struct cdp_tx_indication_info),
		    0);

	mpdu_info = &tx_capture_info.mpdu_info;

	mpdu_info->channel = ppdu_desc->channel;
	mpdu_info->frame_type = ppdu_desc->frame_type;
	mpdu_info->ppdu_start_timestamp =
				ppdu_desc->ppdu_start_timestamp;
	mpdu_info->ppdu_end_timestamp =
				ppdu_desc->ppdu_end_timestamp;
	mpdu_info->tx_duration = ppdu_desc->tx_duration;
	mpdu_info->num_msdu = ppdu_desc->num_msdu;

	mpdu_info->resp_type = ppdu_desc->resp_type;
	mpdu_info->mprot_type = ppdu_desc->mprot_type;
	mpdu_info->rts_success = ppdu_desc->rts_success;
	mpdu_info->rts_failure = ppdu_desc->rts_failure;

	/* update cdp_tx_indication_mpdu_info */
	dp_tx_update_user_mpdu_info(ppdu_id,
				    &tx_capture_info.mpdu_info,
				    &ppdu_desc->user[0]);
	tx_capture_info.ppdu_desc = ppdu_desc;

	tx_capture_info.mpdu_info.channel_num =
		pdev->operating_channel;

	if (ppdu_desc->mprot_type)
		dp_send_dummy_rts_cts_frame(pdev, ppdu_desc);

	start_seq = ppdu_desc->user[0].start_seq;
	for (i = 0; i < ppdu_desc->user[0].ba_size; i++) {
		if (qdf_likely(ppdu_desc->user[0].tid !=
		    DP_NON_QOS_TID) &&
		    !(SEQ_BIT(ppdu_desc->user[0].enq_bitmap, i))) {
			continue;
		}
		seq_no = start_seq + i;
		if (!ppdu_desc->mpdus[i])
			continue;

		tx_capture_info.mpdu_nbuf = ppdu_desc->mpdus[i];
		ppdu_desc->mpdus[i] = NULL;
		mpdu_info->seq_no = seq_no;
		dp_tx_update_sequence_number(tx_capture_info.mpdu_nbuf, seq_no);
		/*
		 * send MPDU to osif layer
		 * do we need to update mpdu_info before tranmit
		 * get current mpdu_nbuf
		 */
		dp_wdi_event_handler(WDI_EVENT_TX_DATA, pdev->soc,
				     &tx_capture_info,
				     HTT_INVALID_PEER,
				     WDI_NO_VAL, pdev->pdev_id);

		if (tx_capture_info.mpdu_nbuf)
			qdf_nbuf_free(tx_capture_info.mpdu_nbuf);
	}
}

static void
dp_tx_mon_proc_xretries(struct dp_pdev *pdev, struct dp_peer *peer,
			uint16_t tid)
{
	struct dp_tx_tid *tx_tid = &peer->tx_capture.tx_tid[tid];
	struct cdp_tx_completion_ppdu *ppdu_desc;
	struct cdp_tx_completion_ppdu *xretry_ppdu;
	qdf_nbuf_t ppdu_nbuf;
	qdf_nbuf_t mpdu_nbuf;
	uint32_t mpdu_tried = 0;
	int i;
	uint32_t seq_no;

	xretry_ppdu = &tx_tid->xretry_ppdu;

	if (qdf_nbuf_is_queue_empty(&tx_tid->pending_ppdu_q)) {
		qdf_nbuf_queue_free(&xretry_ppdu->mpdu_q);
		return;
	}

	if (qdf_nbuf_is_queue_empty(&xretry_ppdu->mpdu_q))
		return;

	ppdu_nbuf = qdf_nbuf_queue_first(&tx_tid->pending_ppdu_q);
	while (ppdu_nbuf) {
		struct msdu_completion_info *ptr_msdu_info = NULL;

		ppdu_desc = (struct cdp_tx_completion_ppdu *)
			qdf_nbuf_data(ppdu_nbuf);
		if (ppdu_desc->pending_retries) {
			uint32_t start_seq = ppdu_desc->user[0].start_seq;

			mpdu_tried = ppdu_desc->user[0].mpdu_tried_ucast +
			ppdu_desc->user[0].mpdu_tried_mcast;
			mpdu_nbuf = qdf_nbuf_queue_first(&xretry_ppdu->mpdu_q);
			for (i = 0; (mpdu_tried > 0) && mpdu_nbuf; i++) {
				if (!(SEQ_BIT(ppdu_desc->user[0].enq_bitmap,
				    i)))
					continue;
				mpdu_tried--;
				/* missed seq number */
				seq_no = start_seq + i;

				if (SEQ_BIT(ppdu_desc->user[0].failed_bitmap, i))
					continue;
				QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE,
					  QDF_TRACE_LEVEL_INFO,
					"%s: fill seqno %d from xretries",
					__func__, seq_no);

				ptr_msdu_info = (struct msdu_completion_info *)
					(qdf_nbuf_data(qdf_nbuf_get_ext_list(
					mpdu_nbuf)) -
					(sizeof(struct msdu_completion_info) +
					sizeof(qdf_ether_header_t)));
				ptr_msdu_info->transmit_cnt--;
				SEQ_SEG(ppdu_desc->user[0].failed_bitmap, i) |=
				SEQ_SEG_MSK(ppdu_desc->user[0].failed_bitmap[0],
					    i);
				ppdu_desc->pending_retries--;
				if (ptr_msdu_info->transmit_cnt == 0) {
					ppdu_desc->mpdus[seq_no - start_seq] =
						mpdu_nbuf;
					qdf_nbuf_queue_remove(
						&xretry_ppdu->mpdu_q);
					mpdu_nbuf = qdf_nbuf_queue_first(
						&xretry_ppdu->mpdu_q);
				} else {
					ppdu_desc->mpdus[seq_no - start_seq] =
					qdf_nbuf_copy_expand(mpdu_nbuf,
							     MAX_MONITOR_HEADER,
							     0);
					mpdu_nbuf =
						qdf_nbuf_queue_next(mpdu_nbuf);
				}
			}
		}

		if ((ppdu_desc->pending_retries == 0) && (ppdu_nbuf ==
			qdf_nbuf_queue_first(&tx_tid->pending_ppdu_q))) {
			qdf_nbuf_queue_remove(&tx_tid->pending_ppdu_q);
			/* Deliver PPDU */
			dp_send_data_to_stack(pdev, ppdu_desc);
			qdf_nbuf_queue_free(&ppdu_desc->mpdu_q);
			qdf_mem_free(ppdu_desc->mpdus);
			ppdu_desc->mpdus = NULL;
			qdf_nbuf_free(ppdu_nbuf);
			ppdu_nbuf = qdf_nbuf_queue_first(
				&tx_tid->pending_ppdu_q);
		} else {
			ppdu_nbuf = qdf_nbuf_queue_next(ppdu_nbuf);
		}
	}
	qdf_nbuf_queue_free(&xretry_ppdu->mpdu_q);
}

#define MAX_PENDING_PPDUS 32
static void
dp_tx_mon_proc_pending_ppdus(struct dp_pdev *pdev, struct dp_tx_tid *tx_tid,
			     qdf_nbuf_t nbuf_ppdu_desc_list[], uint32_t
			     ppdu_desc_cnt, qdf_nbuf_queue_t *head_ppdu,
			     uint32_t peer_id)
{
	struct cdp_tx_completion_ppdu *ppdu_desc = NULL;
	struct cdp_tx_completion_ppdu *cur_ppdu_desc = NULL;
	qdf_nbuf_t pend_ppdu;
	uint32_t ppdu_cnt;
	uint32_t failed_seq;
	uint32_t cur_index, cur_start_seq, cur_last_seq;
	int i, k;
	bool last_pend_ppdu = false;
	qdf_nbuf_t tmp_nbuf;

	pend_ppdu = qdf_nbuf_queue_first(&tx_tid->pending_ppdu_q);
	if (!pend_ppdu) {
		for (ppdu_cnt = 0; ppdu_cnt < ppdu_desc_cnt; ppdu_cnt++) {
			if (!nbuf_ppdu_desc_list[ppdu_cnt])
				continue;
			ppdu_desc = (struct cdp_tx_completion_ppdu *)
				qdf_nbuf_data(
				nbuf_ppdu_desc_list[ppdu_cnt]);
			if (!ppdu_desc || (peer_id !=
			    ppdu_desc->user[0].peer_id) || (tx_tid->tid !=
			    ppdu_desc->user[0].tid))
				continue;
			if ((ppdu_desc->pending_retries == 0) &&
			    qdf_nbuf_is_queue_empty(&tx_tid->pending_ppdu_q) &&
			    qdf_nbuf_is_queue_empty(head_ppdu)) {
				dp_send_data_to_stack(pdev, ppdu_desc);
				qdf_nbuf_queue_free(&ppdu_desc->mpdu_q);
				qdf_mem_free(ppdu_desc->mpdus);
				ppdu_desc->mpdus = NULL;
				tmp_nbuf = nbuf_ppdu_desc_list[ppdu_cnt];
				nbuf_ppdu_desc_list[ppdu_cnt] = NULL;
				qdf_nbuf_free(tmp_nbuf);
			} else {
				qdf_nbuf_queue_add(head_ppdu,
						   nbuf_ppdu_desc_list[ppdu_cnt]);
				nbuf_ppdu_desc_list[ppdu_cnt] = NULL;
			}
		}
		return;
	}
	while (pend_ppdu) {
		qdf_nbuf_t mpdu_nbuf;

		/* Find missing mpdus from current schedule list */
		ppdu_cnt = 0;

		if (!nbuf_ppdu_desc_list[ppdu_cnt]) {
			ppdu_cnt++;
			if (ppdu_cnt < ppdu_desc_cnt)
				continue;
			break;
		}

		ppdu_desc = (struct cdp_tx_completion_ppdu *)qdf_nbuf_data(
			pend_ppdu);

		cur_ppdu_desc = (struct cdp_tx_completion_ppdu *)qdf_nbuf_data(
			nbuf_ppdu_desc_list[ppdu_cnt]);
		if (pend_ppdu == qdf_nbuf_queue_last(
		    &tx_tid->pending_ppdu_q)) {
			last_pend_ppdu = true;
			qdf_nbuf_queue_add(head_ppdu,
					   nbuf_ppdu_desc_list[ppdu_cnt]);
			nbuf_ppdu_desc_list[ppdu_cnt] = NULL;
		}
		cur_index = 0;
		cur_start_seq = cur_ppdu_desc->user[0].start_seq;
		cur_last_seq = cur_ppdu_desc->user[0].last_enq_seq;
		if (qdf_unlikely(ppdu_desc->user[0].ba_size >
		    CDP_BA_256_BIT_MAP_SIZE_DWORDS *
		    SEQ_SEG_SZ_BITS(ppdu_desc->user[0].failed_bitmap))) {
			qdf_assert_always(0);
			return;
		}
		for (i = 0; (i < ppdu_desc->user[0].ba_size) && cur_ppdu_desc;
		     i++) {
			if (!(i & (SEQ_SEG_SZ_BITS(
			    ppdu_desc->user[0].failed_bitmap) - 1))) {
				k =
				SEQ_SEG_INDEX(ppdu_desc->user[0].failed_bitmap,
					      i);
				failed_seq =
					ppdu_desc->user[0].failed_bitmap[k] ^
					ppdu_desc->user[0].enq_bitmap[k];
			}

			/* Skip to next bitmap segment if there are no
			 * more holes in current segment
			 */
			if (!failed_seq) {
				i = ((k + 1) *
				SEQ_SEG_SZ_BITS(ppdu_desc->user[0].failed_bitmap))
						- 1;
				continue;
			}
			if (!(SEQ_SEG_BIT(failed_seq, i)))
				continue;
			failed_seq ^= SEQ_SEG_MSK(failed_seq, i);
			mpdu_nbuf = cur_ppdu_desc->mpdus[cur_index];
			if (mpdu_nbuf) {
				QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE,
					  QDF_TRACE_LEVEL_INFO,
					"%s: fill seqno %d (%d) from swretries",
					__func__,
					ppdu_desc->user[0].start_seq + i,
					ppdu_desc->ppdu_id);
				ppdu_desc->mpdus[i] =
				qdf_nbuf_copy_expand(mpdu_nbuf,
						     MAX_MONITOR_HEADER, 0);
				ppdu_desc->user[0].failed_bitmap[k] |=
				SEQ_SEG_MSK(ppdu_desc->user[0].failed_bitmap[k],
					    i);
				ppdu_desc->pending_retries--;
			}
			cur_index++;
			/* Skip through empty slots in current PPDU */
			while (!(SEQ_BIT(cur_ppdu_desc->user[0].enq_bitmap,
			       cur_index))) {
				struct cdp_tx_completion_ppdu *next_ppdu = NULL;

				cur_index++;
				if (cur_index <= (cur_last_seq -
				    cur_start_seq))
					continue;
				cur_ppdu_desc = NULL;
				/* Check if subsequent PPDUs in this schedule
				 * has higher sequence numbers enqueued
				 */
				while (ppdu_cnt < (ppdu_desc_cnt - 1)) {
					ppdu_cnt++;
					if (!nbuf_ppdu_desc_list[ppdu_cnt])
						continue;
					next_ppdu =
					(struct cdp_tx_completion_ppdu *)
					qdf_nbuf_data(
						      nbuf_ppdu_desc_list[
							ppdu_cnt]);
					if (!next_ppdu || (peer_id !=
					    next_ppdu->user[0].peer_id))
						continue;
					if (last_pend_ppdu) {
						qdf_nbuf_queue_add(head_ppdu,
							nbuf_ppdu_desc_list[ppdu_cnt]);
						nbuf_ppdu_desc_list[ppdu_cnt] =
							NULL;
					}
					if (next_ppdu->user[0].last_enq_seq >
					    cur_last_seq) {
						cur_ppdu_desc = next_ppdu;
						break;
					}
				}
				if (!cur_ppdu_desc)
					break;
				/* Start from seq. no following cur_last_seq
				 * since everything before is already populated
				 * from previous PPDU
				 */
				cur_start_seq =
					cur_ppdu_desc->user[0].start_seq;
				cur_index = (cur_last_seq >= cur_start_seq) ?
					cur_last_seq - cur_start_seq + 1 : 0;
				cur_last_seq =
					cur_ppdu_desc->user[0].last_enq_seq;
			}
		}
		if ((pend_ppdu ==
		    qdf_nbuf_queue_first(&tx_tid->pending_ppdu_q)) &&
		    (ppdu_desc->pending_retries == 0)) {
			qdf_nbuf_queue_remove(&tx_tid->pending_ppdu_q);
			dp_send_data_to_stack(pdev, ppdu_desc);
			qdf_nbuf_queue_free(&ppdu_desc->mpdu_q);
			qdf_mem_free(ppdu_desc->mpdus);
			ppdu_desc->mpdus = NULL;
			qdf_nbuf_free(pend_ppdu);
			pend_ppdu = qdf_nbuf_queue_first(
				&tx_tid->pending_ppdu_q);
		} else {
			pend_ppdu = qdf_nbuf_queue_next(pend_ppdu);
		}
	}
}

static uint32_t
dp_send_mgmt_ctrl_to_stack(struct dp_pdev *pdev,
			   qdf_nbuf_t nbuf_ppdu_desc,
			   struct cdp_tx_indication_info *ptr_tx_cap_info,
			   qdf_nbuf_t mgmt_ctl_nbuf,
			   bool is_payload)
{
	struct cdp_tx_completion_ppdu *ppdu_desc;
	struct cdp_tx_indication_mpdu_info *mpdu_info;
	struct ieee80211_frame *wh;
	uint16_t duration_le, seq_le;
	struct ieee80211_frame_min_one *wh_min;
	uint16_t frame_ctrl_le;
	uint8_t type, subtype;

	mpdu_info = &ptr_tx_cap_info->mpdu_info;
	ppdu_desc = (struct cdp_tx_completion_ppdu *)
			qdf_nbuf_data(nbuf_ppdu_desc);

	type = (ppdu_desc->frame_ctrl &
		IEEE80211_FC0_TYPE_MASK) >>
		IEEE80211_FC0_TYPE_SHIFT;
	subtype = (ppdu_desc->frame_ctrl &
		IEEE80211_FC0_SUBTYPE_MASK) >>
		IEEE80211_FC0_SUBTYPE_SHIFT;

	if (is_payload) {
		wh = (struct ieee80211_frame *)qdf_nbuf_data(mgmt_ctl_nbuf);

		if (subtype != IEEE80211_FC0_SUBTYPE_BEACON) {
			duration_le = qdf_cpu_to_le16(ppdu_desc->tx_duration);
			wh->i_dur[1] = (duration_le & 0xFF00) >> 8;
			wh->i_dur[0] = duration_le & 0xFF;
			seq_le = qdf_cpu_to_le16(ppdu_desc->user[0].start_seq <<
						 IEEE80211_SEQ_SEQ_SHIFT);
			wh->i_seq[1] = (seq_le & 0xFF00) >> 8;
			wh->i_seq[0] = seq_le & 0xFF;
		}
		QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE,
			  QDF_TRACE_LEVEL_DEBUG,
			  "ctrl/mgmt frm(0x%08x): fc 0x%x 0x%x\n",
			  ptr_tx_cap_info->mpdu_info.ppdu_id,
			  wh->i_fc[1], wh->i_fc[0]);
		QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE,
			  QDF_TRACE_LEVEL_DEBUG,
			  "desc->ppdu_id 0x%08x\n", ppdu_desc->ppdu_id);

		/* append ext list */
		qdf_nbuf_append_ext_list(ptr_tx_cap_info->mpdu_nbuf,
					 mgmt_ctl_nbuf,
					 qdf_nbuf_len(mgmt_ctl_nbuf));
	} else {
		wh_min = (struct ieee80211_frame_min_one *)
				qdf_nbuf_data(ptr_tx_cap_info->mpdu_nbuf);
		qdf_mem_zero(wh_min, MAX_DUMMY_FRM_BODY);
		frame_ctrl_le = qdf_cpu_to_le16(ppdu_desc->frame_ctrl);
		duration_le = qdf_cpu_to_le16(ppdu_desc->tx_duration);
		wh_min->i_fc[1] = (frame_ctrl_le & 0xFF00) >> 8;
		wh_min->i_fc[0] = (frame_ctrl_le & 0xFF);
		wh_min->i_dur[1] = (duration_le & 0xFF00) >> 8;
		wh_min->i_dur[0] = (duration_le & 0xFF);
		qdf_mem_copy(wh_min->i_addr1, mpdu_info->mac_address,
			     QDF_MAC_ADDR_SIZE);
		qdf_nbuf_set_pktlen(ptr_tx_cap_info->mpdu_nbuf,
				    sizeof(*wh_min));
		QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE,
			  QDF_TRACE_LEVEL_DEBUG,
			  "frm(0x%08x): fc %x %x, dur 0x%x%x\n",
			  ptr_tx_cap_info->mpdu_info.ppdu_id,
			  wh_min->i_fc[1], wh_min->i_fc[0],
			  wh_min->i_dur[1], wh_min->i_dur[0]);
	}


	dp_wdi_event_handler(WDI_EVENT_TX_DATA, pdev->soc,
			     ptr_tx_cap_info, HTT_INVALID_PEER,
			     WDI_NO_VAL, pdev->pdev_id);

	if (ptr_tx_cap_info->mpdu_nbuf)
		qdf_nbuf_free(ptr_tx_cap_info->mpdu_nbuf);

	return 0;
}

static uint32_t
dp_update_tx_cap_info(struct dp_pdev *pdev,
		      qdf_nbuf_t nbuf_ppdu_desc,
		      void *tx_info, bool is_payload)
{
	struct cdp_tx_completion_ppdu *ppdu_desc;
	struct cdp_tx_indication_info *tx_capture_info =
		(struct cdp_tx_indication_info *)tx_info;
	struct cdp_tx_indication_mpdu_info *mpdu_info;

	ppdu_desc = (struct cdp_tx_completion_ppdu *)
			qdf_nbuf_data(nbuf_ppdu_desc);

	qdf_mem_set(tx_capture_info, sizeof(struct cdp_tx_indication_info), 0);
	mpdu_info = &tx_capture_info->mpdu_info;

	mpdu_info->channel = ppdu_desc->channel;
	mpdu_info->frame_type = ppdu_desc->frame_type;
	mpdu_info->ppdu_start_timestamp = ppdu_desc->ppdu_start_timestamp;
	mpdu_info->ppdu_end_timestamp = ppdu_desc->ppdu_end_timestamp;
	mpdu_info->tx_duration = ppdu_desc->tx_duration;
	mpdu_info->seq_no = ppdu_desc->user[0].start_seq;
	mpdu_info->num_msdu = ppdu_desc->num_msdu;

	/* update cdp_tx_indication_mpdu_info */
	dp_tx_update_user_mpdu_info(ppdu_desc->ppdu_id,
				    &tx_capture_info->mpdu_info,
				    &ppdu_desc->user[0]);
	tx_capture_info->ppdu_desc = ppdu_desc;

	tx_capture_info->mpdu_info.channel_num = pdev->operating_channel;
	tx_capture_info->mpdu_info.ppdu_id = ppdu_desc->ppdu_id;
	if (is_payload)
		tx_capture_info->mpdu_nbuf = qdf_nbuf_alloc(pdev->soc->osdev,
							    MAX_MONITOR_HEADER,
							    MAX_MONITOR_HEADER,
							    4, FALSE);
	else
		tx_capture_info->mpdu_nbuf = qdf_nbuf_alloc(pdev->soc->osdev,
							    MAX_MONITOR_HEADER +
							    MAX_DUMMY_FRM_BODY,
							    MAX_MONITOR_HEADER,
							    4, FALSE);
	return 0;
}

static uint32_t
dp_check_mgmt_ctrl_ppdu(struct dp_pdev *pdev,
			qdf_nbuf_t nbuf_ppdu_desc)
{
	struct cdp_tx_indication_info tx_capture_info;
	qdf_nbuf_t mgmt_ctl_nbuf;
	uint8_t type, subtype;
	bool is_sgen_pkt;
	struct cdp_tx_mgmt_comp_info *ptr_comp_info;
	qdf_nbuf_queue_t *retries_q;
	struct cdp_tx_completion_ppdu *ppdu_desc;
	uint32_t ppdu_id;
	size_t head_size;
	uint32_t status = 1;

	ppdu_desc = (struct cdp_tx_completion_ppdu *)
		qdf_nbuf_data(nbuf_ppdu_desc);
	/*
	 * only for host generated frame we do have
	 * timestamp and retries count.
	 */
	head_size = sizeof(struct cdp_tx_mgmt_comp_info);

	type = (ppdu_desc->frame_ctrl &
		IEEE80211_FC0_TYPE_MASK) >>
		IEEE80211_FC0_TYPE_SHIFT;
	subtype = (ppdu_desc->frame_ctrl &
		IEEE80211_FC0_SUBTYPE_MASK) >>
		IEEE80211_FC0_SUBTYPE_SHIFT;

	if (ppdu_desc->htt_frame_type == HTT_STATS_FTYPE_SGEN_NDP) {
		dp_update_frame_ctrl_from_frame_type(ppdu_desc);
		type = 0;
		subtype = 0;
	}

	retries_q = &pdev->tx_capture.retries_ctl_mgmt_q[type][subtype];
get_mgmt_pkt_from_queue:
	qdf_spin_lock_bh(
		&pdev->tx_capture.ctl_mgmt_lock[type][subtype]);
	mgmt_ctl_nbuf = qdf_nbuf_queue_remove(
		&pdev->tx_capture.ctl_mgmt_q[type][subtype]);
	qdf_spin_unlock_bh(&pdev->tx_capture.ctl_mgmt_lock[type][subtype]);

	if (mgmt_ctl_nbuf) {
		qdf_nbuf_t tmp_mgmt_ctl_nbuf;
		uint32_t start_tsf;

		ptr_comp_info = (struct cdp_tx_mgmt_comp_info *)
				qdf_nbuf_data(mgmt_ctl_nbuf);
		is_sgen_pkt = ptr_comp_info->is_sgen_pkt;
		ppdu_id = ptr_comp_info->ppdu_id;

		if (!is_sgen_pkt && ptr_comp_info->tx_tsf <
		    ppdu_desc->ppdu_start_timestamp) {
			/*
			 * free the older mgmt buffer from
			 * the queue and get new mgmt buffer
			 */
			qdf_nbuf_free(mgmt_ctl_nbuf);
			goto get_mgmt_pkt_from_queue;
		}

		QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE,
			  QDF_TRACE_LEVEL_INFO_HIGH,
			  "ppdu_id [%d 0x%x] type_subtype[%d %d] is_sgen[%d] h_sz[%d]",
			  ppdu_id, ppdu_id, type, subtype,
			  is_sgen_pkt, head_size);

		QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_TX_CAPTURE,
				   QDF_TRACE_LEVEL_INFO_HIGH,
				   qdf_nbuf_data(mgmt_ctl_nbuf), 32);

		/*
		 * for sgen frame we won't have, retries count
		 * and 64 bits tsf in the head.
		 */
		if (ppdu_id != ppdu_desc->ppdu_id) {
			if (is_sgen_pkt) {
				start_tsf = (ppdu_desc->ppdu_start_timestamp &
					     LOWER_32_MASK);
				if ((ptr_comp_info->tx_tsf <
				     (start_tsf + MAX_MGMT_ENQ_DELAY)) &&
				    ((ppdu_id & SCH_ID_MASK) <
				     (ppdu_desc->ppdu_id & SCH_ID_MASK))) {
					/*
					 * free the older mgmt buffer from
					 * the queue and get new mgmt buffer
					 */
					qdf_nbuf_free(mgmt_ctl_nbuf);
					goto get_mgmt_pkt_from_queue;
				}
			}

			if (ppdu_desc->user[0].completion_status ==
			    HTT_PPDU_STATS_USER_STATUS_FILTERED) {
				qdf_nbuf_free(nbuf_ppdu_desc);
				status = 0;
				goto free_ppdu_desc;
			}

			/*
			 * add the ppdu_desc into retry queue
			 */
			qdf_nbuf_queue_add(retries_q, nbuf_ppdu_desc);
			status = 0;

			/*
			 * insert the mgmt_ctl buffer back to
			 * the queue
			 */
			qdf_spin_lock_bh(
			&pdev->tx_capture.ctl_mgmt_lock[type][subtype]);
			qdf_nbuf_queue_insert_head(
			&pdev->tx_capture.ctl_mgmt_q[type][subtype],
			mgmt_ctl_nbuf);
			qdf_spin_unlock_bh(
			&pdev->tx_capture.ctl_mgmt_lock[type][subtype]);
		} else {
			qdf_nbuf_t nbuf_retry_ppdu;
			struct cdp_tx_completion_ppdu *tmp_ppdu_desc;
			uint16_t frame_ctrl_le;
			struct ieee80211_frame *wh;

			if (ppdu_desc->user[0].completion_status ==
			    HTT_PPDU_STATS_USER_STATUS_FILTERED) {
				qdf_nbuf_free(nbuf_ppdu_desc);
				qdf_nbuf_free(mgmt_ctl_nbuf);
				status = 0;
				goto free_ppdu_desc;
			}

			while (!!qdf_nbuf_queue_len(retries_q)) {
				/*
				 * send retried packet stored
				 * in queue
				 */
				nbuf_retry_ppdu =
					qdf_nbuf_queue_remove(retries_q);
				tmp_ppdu_desc =
					(struct cdp_tx_completion_ppdu *)
						qdf_nbuf_data(nbuf_retry_ppdu);
				tmp_mgmt_ctl_nbuf =
					skb_copy_expand(mgmt_ctl_nbuf,
							0, 0, GFP_ATOMIC);

				dp_update_tx_cap_info(pdev, nbuf_retry_ppdu,
						      &tx_capture_info, true);
				if (!tx_capture_info.mpdu_nbuf) {
					qdf_nbuf_free(nbuf_retry_ppdu);
					qdf_nbuf_free(tmp_mgmt_ctl_nbuf);
					continue;
				}

				/* pull head based on sgen pkt or mgmt pkt */
				qdf_nbuf_pull_head(tmp_mgmt_ctl_nbuf,
						   head_size);

				/*
				 * frame control from ppdu_desc has
				 * retry flag set
				 */
				frame_ctrl_le =
				qdf_cpu_to_le16(tmp_ppdu_desc->frame_ctrl);
				wh = (struct ieee80211_frame *)
					(qdf_nbuf_data(tmp_mgmt_ctl_nbuf));
				wh->i_fc[1] = (frame_ctrl_le & 0xFF00) >> 8;
				wh->i_fc[0] = (frame_ctrl_le & 0xFF);

				tx_capture_info.ppdu_desc = tmp_ppdu_desc;
				/*
				 * send MPDU to osif layer
				 */
				dp_send_mgmt_ctrl_to_stack(pdev,
							   nbuf_retry_ppdu,
							   &tx_capture_info,
							   tmp_mgmt_ctl_nbuf,
							   true);

				/* free retried queue nbuf ppdu_desc */
				qdf_nbuf_free(nbuf_retry_ppdu);
			}

			dp_update_tx_cap_info(pdev, nbuf_ppdu_desc,
					      &tx_capture_info, true);
			if (!tx_capture_info.mpdu_nbuf) {
				qdf_nbuf_free(mgmt_ctl_nbuf);
				goto free_ppdu_desc;
			}

			tx_capture_info.mpdu_info.ppdu_id =
				*(uint32_t *)qdf_nbuf_data(mgmt_ctl_nbuf);
			/* pull head based on sgen pkt or mgmt pkt */
			qdf_nbuf_pull_head(mgmt_ctl_nbuf, head_size);

			/* frame control from ppdu_desc has retry flag set */
			frame_ctrl_le = qdf_cpu_to_le16(ppdu_desc->frame_ctrl);
			wh = (struct ieee80211_frame *)
				(qdf_nbuf_data(mgmt_ctl_nbuf));
			wh->i_fc[1] = (frame_ctrl_le & 0xFF00) >> 8;
			wh->i_fc[0] = (frame_ctrl_le & 0xFF);

			tx_capture_info.ppdu_desc = ppdu_desc;
			/*
			 * send MPDU to osif layer
			 */
			dp_send_mgmt_ctrl_to_stack(pdev, nbuf_ppdu_desc,
						   &tx_capture_info,
						   mgmt_ctl_nbuf, true);
		}
	} else if ((ppdu_desc->frame_ctrl &
		   IEEE80211_FC0_TYPE_MASK) ==
		   IEEE80211_FC0_TYPE_CTL) {

		if (ppdu_desc->user[0].completion_status ==
		    HTT_PPDU_STATS_USER_STATUS_FILTERED) {
			qdf_nbuf_free(nbuf_ppdu_desc);
			status = 0;
			goto free_ppdu_desc;
		}

		dp_update_tx_cap_info(pdev, nbuf_ppdu_desc,
				      &tx_capture_info, false);
		if (!tx_capture_info.mpdu_nbuf)
			goto free_ppdu_desc;
		/*
		 * send MPDU to osif layer
		 */
		dp_send_mgmt_ctrl_to_stack(pdev, nbuf_ppdu_desc,
					   &tx_capture_info, NULL, false);
	}

free_ppdu_desc:
	return status;
}

/**
 * dp_tx_ppdu_stats_flush(): Function to flush pending retried ppdu desc
 * @pdev: DP pdev handle
 * @nbuf: ppdu_desc
 *
 * return: void
 */
static void
dp_tx_ppdu_stats_flush(struct dp_pdev *pdev,
		       struct cdp_tx_completion_ppdu *ppdu_desc)
{
	struct dp_peer *peer;

	peer = dp_peer_find_by_id(pdev->soc,
				  ppdu_desc->user[0].peer_id);

	if (!peer)
		return;

	/*
	 * for all drop reason we are invoking
	 * proc xretries
	 */
	dp_tx_mon_proc_xretries(pdev, peer, ppdu_desc->user[0].tid);

	dp_peer_unref_del_find_by_id(peer);
	return;
}

/**
 * dp_check_ppdu_and_deliver(): Check PPDUs for any holes and deliver
 * to upper layer if complete
 * @pdev: DP pdev handle
 * @nbuf_ppdu_desc_list: ppdu_desc_list per sched cmd id
 * @ppdu_desc_cnt: number of ppdu_desc_cnt
 *
 * return: status
 */
static void
dp_check_ppdu_and_deliver(struct dp_pdev *pdev,
			  qdf_nbuf_t nbuf_ppdu_desc_list[],
			  uint32_t ppdu_desc_cnt)
{
	uint32_t ppdu_id;
	uint32_t desc_cnt;
	qdf_nbuf_t tmp_nbuf;
	struct dp_tx_tid *tx_tid  = NULL;
	int i;

	for (desc_cnt = 0; desc_cnt < ppdu_desc_cnt; desc_cnt++) {
		struct cdp_tx_completion_ppdu *ppdu_desc;
		uint32_t num_mpdu;
		uint16_t start_seq, seq_no = 0;
		int i;
		uint32_t len;
		qdf_nbuf_t mpdu_nbuf;
		struct dp_peer *peer;
		uint8_t type;

		if (!nbuf_ppdu_desc_list[desc_cnt])
			continue;

		ppdu_desc = (struct cdp_tx_completion_ppdu *)
			qdf_nbuf_data(nbuf_ppdu_desc_list[desc_cnt]);

		ppdu_id = ppdu_desc->ppdu_id;
		type = (ppdu_desc->frame_ctrl & IEEE80211_FC0_TYPE_MASK) >>
			IEEE80211_FC0_TYPE_SHIFT;

		if (ppdu_desc->is_flush) {
			dp_tx_ppdu_stats_flush(pdev, ppdu_desc);
			tmp_nbuf = nbuf_ppdu_desc_list[desc_cnt];
			nbuf_ppdu_desc_list[desc_cnt] = NULL;
			qdf_nbuf_free(tmp_nbuf);
			continue;
		}

		if (ppdu_desc->frame_type == CDP_PPDU_FTYPE_CTRL ||
		    ppdu_desc->htt_frame_type ==
		    HTT_STATS_FTYPE_SGEN_QOS_NULL ||
		    type != FRAME_CTRL_TYPE_DATA) {
			qdf_nbuf_t nbuf_ppdu = nbuf_ppdu_desc_list[desc_cnt];

			if (dp_check_mgmt_ctrl_ppdu(pdev, nbuf_ppdu)) {
				tmp_nbuf = nbuf_ppdu_desc_list[desc_cnt];
				nbuf_ppdu_desc_list[desc_cnt] = NULL;
				qdf_nbuf_free(tmp_nbuf);
				continue;
			}
			nbuf_ppdu_desc_list[desc_cnt] = NULL;
			continue;
		}

		peer = dp_peer_find_by_id(pdev->soc,
					  ppdu_desc->user[0].peer_id);
		if (!peer) {
			tmp_nbuf = nbuf_ppdu_desc_list[desc_cnt];
			nbuf_ppdu_desc_list[desc_cnt] = NULL;
			qdf_nbuf_queue_free(&ppdu_desc->mpdu_q);
			qdf_nbuf_free(tmp_nbuf);
			continue;
		}

		tx_tid = &peer->tx_capture.tx_tid[ppdu_desc->user[0].tid];

		ppdu_id = ppdu_desc->ppdu_id;

		/* find mpdu tried is same as success mpdu */

		num_mpdu = ppdu_desc->user[0].mpdu_success;

		/* get length */
		len = qdf_nbuf_queue_len(&ppdu_desc->mpdu_q);

		/* ba_size is updated in BA bitmap TLVs, which are not received
		 * in case of non-QoS TID.
		 */
		if (qdf_unlikely(ppdu_desc->user[0].tid == DP_NON_QOS_TID)) {
			ppdu_desc->user[0].ba_size = 1;
			ppdu_desc->user[0].last_enq_seq =
				ppdu_desc->user[0].start_seq;
		}

		if (ppdu_desc->user[0].ba_size == 0)
			ppdu_desc->user[0].ba_size = 1;

		/* find list of missing sequence */
		ppdu_desc->mpdus = qdf_mem_malloc(sizeof(qdf_nbuf_t) *
			ppdu_desc->user[0].ba_size);

		if (ppdu_desc->frame_type == CDP_PPDU_FTYPE_BAR)
			dp_send_dummy_mpdu_info_to_stack(pdev,
							 ppdu_desc);

		if (qdf_unlikely(!ppdu_desc->mpdus)) {
			QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE,
				  QDF_TRACE_LEVEL_FATAL,
				  "%s: ppdu_desc->mpdus allocation failed",
				  __func__);
			tmp_nbuf = nbuf_ppdu_desc_list[desc_cnt];
			nbuf_ppdu_desc_list[desc_cnt] = NULL;
			qdf_nbuf_queue_free(&ppdu_desc->mpdu_q);
			qdf_nbuf_free(tmp_nbuf);
			dp_peer_unref_del_find_by_id(peer);
			continue;
		}

		if (qdf_unlikely(ppdu_desc->user[0].ba_size >
		    CDP_BA_256_BIT_MAP_SIZE_DWORDS *
		    SEQ_SEG_SZ_BITS(ppdu_desc->user[0].failed_bitmap))) {
			dp_peer_unref_del_find_by_id(peer);
			qdf_assert_always(0);
			return;
		}
		/* Fill seq holes within current schedule list */
		start_seq = ppdu_desc->user[0].start_seq;
		for (i = 0; i < ppdu_desc->user[0].ba_size; i++) {
			if (qdf_likely(ppdu_desc->user[0].tid !=
			    DP_NON_QOS_TID) &&
			    !(SEQ_BIT(ppdu_desc->user[0].enq_bitmap, i)))
				continue;
			/* missed seq number */
			seq_no = start_seq + i;

			/* Fill failed MPDUs in AMPDU if they're available in
			 * subsequent PPDUs in current burst schedule. This
			 * is not applicable for non-QoS TIDs (no AMPDUs)
			 */
			if (qdf_likely(ppdu_desc->user[0].tid !=
			    DP_NON_QOS_TID) &&
			    !(SEQ_BIT(ppdu_desc->user[0].failed_bitmap, i))) {
				QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE,
					  QDF_TRACE_LEVEL_DEBUG,
					  "%s:find seq %d in next ppdu %d",
					  __func__, seq_no,
					  ppdu_desc_cnt);

				mpdu_nbuf = get_mpdu_clone_from_next_ppdu(
						nbuf_ppdu_desc_list +
						desc_cnt,
						ppdu_desc_cnt -
						desc_cnt, seq_no,
						ppdu_desc->user[0].peer_id,
						ppdu_id);

				/* check mpdu_nbuf NULL */
				if (!mpdu_nbuf) {
					ppdu_desc->pending_retries++;
					continue;
				}
				ppdu_desc->mpdus[seq_no - start_seq] =
					mpdu_nbuf;
				SEQ_SEG(ppdu_desc->user[0].failed_bitmap, i) |=
				SEQ_SEG_MSK(ppdu_desc->user[0].failed_bitmap[0],
					    i);
			} else {
				/* any error case we need to handle */
				ppdu_desc->mpdus[seq_no - start_seq] =
					qdf_nbuf_queue_remove(
					&ppdu_desc->mpdu_q);
			}
		}
		dp_peer_unref_del_find_by_id(peer);

		if ((ppdu_desc->pending_retries == 0) &&
		    qdf_nbuf_is_queue_empty(&tx_tid->pending_ppdu_q)) {
			dp_send_data_to_stack(pdev, ppdu_desc);
			qdf_nbuf_queue_free(&ppdu_desc->mpdu_q);
			qdf_mem_free(ppdu_desc->mpdus);
			ppdu_desc->mpdus = NULL;
			tmp_nbuf = nbuf_ppdu_desc_list[desc_cnt];
			nbuf_ppdu_desc_list[desc_cnt] = NULL;
			qdf_nbuf_free(tmp_nbuf);
		}
	}

	for (i = 0; i < ppdu_desc_cnt; i++) {
		uint32_t pending_ppdus;
		struct cdp_tx_completion_ppdu *cur_ppdu_desc;
		struct dp_peer *peer;
		qdf_nbuf_queue_t head_ppdu;

		if (!nbuf_ppdu_desc_list[i])
			continue;
		cur_ppdu_desc = (struct cdp_tx_completion_ppdu *)qdf_nbuf_data(
			nbuf_ppdu_desc_list[i]);
		if (!cur_ppdu_desc)
			continue;

		peer = dp_peer_find_by_id(pdev->soc,
					  cur_ppdu_desc->user[0].peer_id);
		if (!peer) {
			tmp_nbuf = nbuf_ppdu_desc_list[i];
			nbuf_ppdu_desc_list[i] = NULL;
			qdf_nbuf_queue_free(&cur_ppdu_desc->mpdu_q);
			qdf_mem_free(cur_ppdu_desc->mpdus);
			qdf_nbuf_free(tmp_nbuf);
			continue;
		}
		tx_tid = &peer->tx_capture.tx_tid[cur_ppdu_desc->user[0].tid];
		qdf_nbuf_queue_init(&head_ppdu);
		dp_tx_mon_proc_pending_ppdus(pdev, tx_tid,
					     nbuf_ppdu_desc_list + i,
					     ppdu_desc_cnt - i, &head_ppdu,
					     cur_ppdu_desc->user[0].peer_id);

		if (qdf_nbuf_is_queue_empty(&tx_tid->pending_ppdu_q)) {
			while ((tmp_nbuf = qdf_nbuf_queue_first(&head_ppdu))) {
				cur_ppdu_desc =
				(struct cdp_tx_completion_ppdu *)
					qdf_nbuf_data(tmp_nbuf);
				if (cur_ppdu_desc->pending_retries)
					break;
				dp_send_data_to_stack(pdev, cur_ppdu_desc);
				qdf_nbuf_queue_free(&cur_ppdu_desc->mpdu_q);
				qdf_mem_free(cur_ppdu_desc->mpdus);
				cur_ppdu_desc->mpdus = NULL;
				qdf_nbuf_queue_remove(&head_ppdu);
				qdf_nbuf_free(tmp_nbuf);
			}
		}
		qdf_nbuf_queue_append(&tx_tid->pending_ppdu_q, &head_ppdu);

		dp_tx_mon_proc_xretries(pdev, peer, tx_tid->tid);

		dp_peer_unref_del_find_by_id(peer);
		pending_ppdus = qdf_nbuf_queue_len(&tx_tid->pending_ppdu_q);
		if (pending_ppdus > MAX_PENDING_PPDUS) {
			QDF_TRACE(QDF_MODULE_ID_TX_CAPTURE,
				  QDF_TRACE_LEVEL_FATAL,
				  "pending ppdus (%d, %d) : %d\n",
				  cur_ppdu_desc->user[0].peer_id,
				  tx_tid->tid, pending_ppdus);
		}
	}
}

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
	struct dp_pdev *pdev = (struct dp_pdev *)context;
	struct ppdu_info *ppdu_info, *tmp_ppdu_info = NULL;
	uint32_t now_ms = qdf_system_ticks_to_msecs(qdf_system_ticks());
	struct ppdu_info *sched_ppdu_info = NULL;

	STAILQ_HEAD(, ppdu_info) sched_ppdu_queue;

	struct ppdu_info *sched_ppdu_list_last_ptr;
	qdf_nbuf_t *nbuf_ppdu_desc_list;
	qdf_nbuf_t tmp_nbuf;
	struct dp_pdev_tx_capture *ptr_tx_cap = &pdev->tx_capture;
	qdf_nbuf_queue_t head_xretries;

	STAILQ_INIT(&sched_ppdu_queue);
	/* Move the PPDU entries to defer list */
	qdf_spin_lock_bh(&ptr_tx_cap->ppdu_stats_lock);
	STAILQ_CONCAT(&ptr_tx_cap->ppdu_stats_defer_queue,
		      &ptr_tx_cap->ppdu_stats_queue);
	ptr_tx_cap->ppdu_stats_defer_queue_depth +=
		ptr_tx_cap->ppdu_stats_queue_depth;
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
			sched_ppdu_list_last_ptr = ppdu_info;
			ppdu_cnt++;
		}
		if (ppdu_info && (curr_sched_cmdid == ppdu_info->sched_cmdid) &&
		    ptr_tx_cap->ppdu_stats_next_sched < now_ms)
			break;

		last_ppdu_id = sched_ppdu_list_last_ptr->ppdu_id;

		STAILQ_FIRST(&sched_ppdu_queue) =
			STAILQ_FIRST(&ptr_tx_cap->ppdu_stats_defer_queue);
		STAILQ_REMOVE_HEAD_UNTIL(&ptr_tx_cap->ppdu_stats_defer_queue,
					 sched_ppdu_list_last_ptr,
					 ppdu_info_queue_elem);
		STAILQ_NEXT(sched_ppdu_list_last_ptr,
			    ppdu_info_queue_elem) = NULL;

		ptr_tx_cap->ppdu_stats_defer_queue_depth -= ppdu_cnt;

		nbuf_ppdu_desc_list =
			(qdf_nbuf_t *) qdf_mem_malloc(sizeof(qdf_nbuf_t) *
						      ppdu_cnt);
		/*
		 * if there is no memory allocated we need to free sched ppdu
		 * list, no ppdu stats will be updated.
		 */
		if (!nbuf_ppdu_desc_list) {
			STAILQ_FOREACH_SAFE(sched_ppdu_info,
					    &sched_ppdu_queue,
					    ppdu_info_queue_elem,
					    tmp_ppdu_info) {
				ppdu_info = sched_ppdu_info;
				tmp_nbuf = ppdu_info->nbuf;
				qdf_mem_free(ppdu_info);
				qdf_nbuf_free(tmp_nbuf);
			}
			continue;
		}

		qdf_spin_lock(&ptr_tx_cap->config_lock);
		ppdu_desc_cnt = 0;
		STAILQ_FOREACH_SAFE(sched_ppdu_info,
				    &sched_ppdu_queue,
				    ppdu_info_queue_elem, tmp_ppdu_info) {
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
			qdf_nbuf_queue_init(&head_xretries);

			ppdu_info = sched_ppdu_info;
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
			if (pdev->tx_capture_enabled ==
			    CDP_TX_ENH_CAPTURE_DISABLED) {
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
			/**
			 * peer can be NULL
			 */
			if (!peer) {
				qdf_nbuf_free(nbuf);
				continue;
			}

			if ((ppdu_desc->frame_type == CDP_PPDU_FTYPE_DATA) ||
			    (ppdu_desc->num_mpdu &&
			     ppdu_desc->frame_type == CDP_PPDU_FTYPE_BAR)) {
				/**
				 * check whether it is bss peer,
				 * if bss_peer no need to process further
				 * check whether tx_capture feature is enabled
				 * for this peer or globally for all peers
				 */
				if (peer->bss_peer ||
				    !dp_peer_or_pdev_tx_cap_enabled(pdev,
				    peer)) {
					dp_peer_unref_del_find_by_id(peer);
					qdf_nbuf_free(nbuf);
					continue;
				}

				/* print the bit map */
				dp_tx_print_bitmap(pdev, ppdu_desc,
						   0, ppdu_desc->ppdu_id);
				if (ppdu_desc->user[0].tid > DP_MAX_TIDS) {
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

				/* Non-QOS frames are being indicated with TID 0
				 * in WBM completion path, an hence we should
				 * TID 0 to reap MSDUs from completion path
				 */
				if (qdf_unlikely(ppdu_desc->user[0].tid ==
				    DP_NON_QOS_TID))
					tid = 0;
				else
					tid = ppdu_desc->user[0].tid;

dequeue_msdu_again:
				num_msdu = ppdu_desc->user[0].num_msdu;
				start_tsf = ppdu_desc->ppdu_start_timestamp;
				end_tsf = ppdu_desc->ppdu_end_timestamp;
				/*
				 * retrieve msdu buffer based on ppdu_id & tid
				 * based msdu queue and store it in local queue
				 * sometimes, wbm comes later than per ppdu
				 * stats. Assumption: all packets are SU,
				 * and packets comes in order
				 */
				ret = dp_tx_msdu_dequeue(peer,
							 ppdu_desc->ppdu_id,
							 tid, num_msdu,
							 &head_msdu,
							 &head_xretries,
							 start_tsf, end_tsf);

				if (!ret && (++retries < 2)) {
					/* wait for wbm to complete */
					qdf_mdelay(2);
					goto dequeue_msdu_again;
				}

				if (!qdf_nbuf_is_queue_empty(&head_xretries)) {
					struct dp_tx_tid *tx_tid =
						&peer->tx_capture.tx_tid[tid];
					struct cdp_tx_completion_ppdu
						*xretry_ppdu =
						&tx_tid->xretry_ppdu;

					xretry_ppdu->ppdu_id =
					peer->tx_capture.tx_wifi_ppdu_id;
					/* Restitch MPDUs from xretry MSDUs */
					dp_tx_mon_restitch_mpdu(pdev, peer,
								xretry_ppdu,
								&head_xretries,
								&xretry_ppdu->mpdu_q);
				}
				if (!qdf_nbuf_is_queue_empty(&head_msdu)) {
					/*
					 * now head_msdu hold - msdu list for
					 * that particular ppdu_id, restitch
					 * mpdu from msdu and create a mpdu
					 * queue
					 */
					dp_tx_mon_restitch_mpdu(pdev, peer,
								ppdu_desc,
								&head_msdu,
								&ppdu_desc->mpdu_q);
					/*
					 * sanity: free local head msdu queue
					 * do we need this ?
					 */
					qdf_nbuf_queue_free(&head_msdu);
					qlen =
					qdf_nbuf_queue_len(&ppdu_desc->mpdu_q);
					if (!qlen) {
						qdf_nbuf_free(nbuf);
						dp_peer_unref_del_find_by_id(peer);
						continue;
					}
				}

				nbuf_ppdu_desc_list[ppdu_desc_cnt++] = nbuf;

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
			} else {
				/*
				 * other packet frame also added to
				 * descriptor list
				 */
				nbuf_ppdu_desc_list[ppdu_desc_cnt++] = nbuf;
			}

			dp_peer_unref_del_find_by_id(peer);
		}

		/*
		 * At this point we have mpdu queued per ppdu_desc
		 * based on packet capture flags send mpdu info to upper stack
		 */
		if (ppdu_desc_cnt) {
			dp_check_ppdu_and_deliver(pdev, nbuf_ppdu_desc_list,
						  ppdu_desc_cnt);
		}

		qdf_spin_unlock(&ptr_tx_cap->config_lock);
		qdf_mem_free(nbuf_ppdu_desc_list);
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

	qdf_spin_lock_bh(&pdev->tx_capture.ppdu_stats_lock);

	if (qdf_unlikely(!pdev->tx_capture_enabled &&
	    (pdev->tx_capture.ppdu_stats_queue_depth +
	    pdev->tx_capture.ppdu_stats_defer_queue_depth) >
	    DP_TX_PPDU_PROC_MAX_DEPTH)) {
		qdf_nbuf_free(ppdu_info->nbuf);
		qdf_mem_free(ppdu_info);
		pdev->tx_capture.ppdu_dropped++;
	} else {
		STAILQ_INSERT_TAIL(&pdev->tx_capture.ppdu_stats_queue,
				   ppdu_info, ppdu_info_queue_elem);
		pdev->tx_capture.ppdu_stats_queue_depth++;
	}
	qdf_spin_unlock_bh(&pdev->tx_capture.ppdu_stats_lock);

	if ((pdev->tx_capture.ppdu_stats_queue_depth >
	    DP_TX_PPDU_PROC_THRESHOLD) ||
	    (pdev->tx_capture.ppdu_stats_next_sched <= now_ms)) {
		qdf_queue_work(0, pdev->tx_capture.ppdu_stats_workqueue,
			       &pdev->tx_capture.ppdu_stats_work);
		pdev->tx_capture.ppdu_stats_next_sched =
			now_ms + DP_TX_PPDU_PROC_TIMEOUT;
	}
}

static void set_mpdu_info(
	struct cdp_tx_indication_info *tx_capture_info,
	struct mon_rx_status *rx_status,
	struct mon_rx_user_status *rx_user_status)
{
	struct cdp_tx_indication_mpdu_info *mpdu_info;

	qdf_mem_set(tx_capture_info,
		    sizeof(struct cdp_tx_indication_info), 0);

	mpdu_info = &tx_capture_info->mpdu_info;
	mpdu_info->ppdu_start_timestamp = rx_status->tsft + 16;
	mpdu_info->channel_num = rx_status->chan_num;
	mpdu_info->bw = 0;

	if  (mpdu_info->channel_num < 20)
		mpdu_info->preamble = DOT11_B;
	else
		mpdu_info->preamble = DOT11_A;
}

static void dp_gen_ack_frame(struct hal_rx_ppdu_info *ppdu_info,
			     struct dp_peer *peer,
			     qdf_nbuf_t mpdu_nbuf)
{
	struct ieee80211_frame_min_one *wh_addr1;

	wh_addr1 = (struct ieee80211_frame_min_one *)
		qdf_nbuf_data(mpdu_nbuf);

	wh_addr1->i_fc[0] = 0;
	wh_addr1->i_fc[1] = 0;
	wh_addr1->i_fc[0] =  IEEE80211_FC0_VERSION_0 |
		IEEE80211_FC0_TYPE_CTL |
		IEEE80211_FC0_SUBTYPE_ACK;
	if (peer) {
		qdf_mem_copy(wh_addr1->i_addr1,
			     &peer->mac_addr.raw[0],
			     QDF_MAC_ADDR_SIZE);
	} else {
		qdf_mem_copy(wh_addr1->i_addr1,
			     &ppdu_info->nac_info.mac_addr2[0],
			     QDF_MAC_ADDR_SIZE);
	}
	*(u_int16_t *)(&wh_addr1->i_dur) = qdf_cpu_to_le16(0x0000);
	qdf_nbuf_set_pktlen(mpdu_nbuf, sizeof(*wh_addr1));
}

static void dp_gen_block_ack_frame(
	struct mon_rx_user_status *rx_user_status,
	struct dp_peer *peer,
	qdf_nbuf_t mpdu_nbuf)
{
	struct dp_vdev *vdev = NULL;
	struct ieee80211_ctlframe_addr2 *wh_addr2;
	uint8_t *frm;

	wh_addr2 = (struct ieee80211_ctlframe_addr2 *)
			qdf_nbuf_data(mpdu_nbuf);

	qdf_mem_zero(wh_addr2, DP_BA_ACK_FRAME_SIZE);

	wh_addr2->i_fc[0] = 0;
	wh_addr2->i_fc[1] = 0;
	wh_addr2->i_fc[0] =  IEEE80211_FC0_VERSION_0 |
		IEEE80211_FC0_TYPE_CTL |
		IEEE80211_FC0_BLOCK_ACK;
	*(u_int16_t *)(&wh_addr2->i_aidordur) = qdf_cpu_to_le16(0x0000);

	vdev = peer->vdev;
	if (vdev)
		qdf_mem_copy(wh_addr2->i_addr2, vdev->mac_addr.raw,
			     QDF_MAC_ADDR_SIZE);
	qdf_mem_copy(wh_addr2->i_addr1, &peer->mac_addr.raw[0],
		     QDF_MAC_ADDR_SIZE);

	frm = (uint8_t *)&wh_addr2[1];
	*((uint16_t *)frm) =
		qdf_cpu_to_le16((rx_user_status->tid <<
		DP_IEEE80211_BAR_CTL_TID_S) |
		DP_IEEE80211_BAR_CTL_COMBA);
	frm += 2;
	*((uint16_t *)frm) =
		rx_user_status->first_data_seq_ctrl;
	frm += 2;
	if ((rx_user_status->mpdu_cnt_fcs_ok +
		rx_user_status->mpdu_cnt_fcs_err)
		> DP_MAX_MPDU_64) {
		qdf_mem_copy(frm,
			     rx_user_status->mpdu_fcs_ok_bitmap,
			     HAL_RX_NUM_WORDS_PER_PPDU_BITMAP *
			     sizeof(rx_user_status->mpdu_fcs_ok_bitmap[0]));
		frm += DP_NUM_BYTES_PER_PPDU_BITMAP;
	} else {
		qdf_mem_copy(frm,
			     rx_user_status->mpdu_fcs_ok_bitmap,
			     DP_NUM_WORDS_PER_PPDU_BITMAP_64 *
			     sizeof(rx_user_status->mpdu_fcs_ok_bitmap[0]));
		frm += DP_NUM_BYTES_PER_PPDU_BITMAP_64;
	}
	qdf_nbuf_set_pktlen(mpdu_nbuf,
			    (frm - (uint8_t *)qdf_nbuf_data(mpdu_nbuf)));
}

/**
 * dp_send_ack_frame_to_stack(): Function to generate BA or ACK frame and
 * send to upper layer on received unicast frame
 * @soc: core txrx main context
 * @pdev: DP pdev object
 * @ppdu_info: HAL RX PPDU info retrieved from status ring TLV
 *
 * return: status
 */
QDF_STATUS dp_send_ack_frame_to_stack(struct dp_soc *soc,
				      struct dp_pdev *pdev,
				      struct hal_rx_ppdu_info *ppdu_info)
{
	struct cdp_tx_indication_info tx_capture_info;
	struct dp_peer *peer;
	struct dp_ast_entry *ast_entry;
	uint32_t peer_id;
	struct mon_rx_status *rx_status;
	struct mon_rx_user_status *rx_user_status;
	uint32_t ast_index;
	uint32_t i;

	rx_status = &ppdu_info->rx_status;

	if (!rx_status->rxpcu_filter_pass)
		return QDF_STATUS_SUCCESS;

	if (ppdu_info->sw_frame_group_id ==
	    HAL_MPDU_SW_FRAME_GROUP_MGMT_BEACON)
		return QDF_STATUS_SUCCESS;

	for (i = 0; i < ppdu_info->com_info.num_users; i++) {
		if (i > OFDMA_NUM_USERS)
			return QDF_STATUS_E_FAULT;

		rx_user_status =  &ppdu_info->rx_user_status[i];

		ast_index = rx_user_status->ast_index;
		if (ast_index >=
		    wlan_cfg_get_max_ast_idx(soc->wlan_cfg_ctx)) {
			set_mpdu_info(&tx_capture_info,
				      rx_status, rx_user_status);
			tx_capture_info.mpdu_nbuf =
				qdf_nbuf_alloc(pdev->soc->osdev,
					       MAX_MONITOR_HEADER +
					       DP_BA_ACK_FRAME_SIZE,
					       MAX_MONITOR_HEADER,
					       4, FALSE);
			if (!tx_capture_info.mpdu_nbuf)
				continue;
			dp_gen_ack_frame(ppdu_info, NULL,
					 tx_capture_info.mpdu_nbuf);
			dp_wdi_event_handler(WDI_EVENT_TX_DATA, pdev->soc,
					     &tx_capture_info, HTT_INVALID_PEER,
					     WDI_NO_VAL, pdev->pdev_id);
			continue;
		}

		qdf_spin_lock_bh(&soc->ast_lock);
		ast_entry = soc->ast_table[ast_index];
		if (!ast_entry) {
			qdf_spin_unlock_bh(&soc->ast_lock);
			continue;
		}

		peer = ast_entry->peer;
		if (!peer || peer->peer_ids[0] == HTT_INVALID_PEER) {
			qdf_spin_unlock_bh(&soc->ast_lock);
			continue;
		}
		peer_id = peer->peer_ids[0];
		qdf_spin_unlock_bh(&soc->ast_lock);

		peer = dp_peer_find_by_id(soc, peer_id);
		if (!peer)
			continue;

		if (!dp_peer_or_pdev_tx_cap_enabled(pdev,
						    peer)) {
			dp_peer_unref_del_find_by_id(peer);
			continue;
		}

		set_mpdu_info(&tx_capture_info,
			      rx_status, rx_user_status);

		tx_capture_info.mpdu_nbuf =
			qdf_nbuf_alloc(pdev->soc->osdev,
				       MAX_MONITOR_HEADER +
				       DP_BA_ACK_FRAME_SIZE,
				       MAX_MONITOR_HEADER,
				       4, FALSE);

		if (!tx_capture_info.mpdu_nbuf) {
			dp_peer_unref_del_find_by_id(peer);
			return QDF_STATUS_E_NOMEM;
		}

		if (rx_status->rs_flags & IEEE80211_AMPDU_FLAG) {
			dp_gen_block_ack_frame(
					       rx_user_status, peer,
					       tx_capture_info.mpdu_nbuf);
			tx_capture_info.mpdu_info.tid = rx_user_status->tid;

		} else {
			dp_gen_ack_frame(ppdu_info, peer,
					 tx_capture_info.mpdu_nbuf);
		}
		dp_peer_unref_del_find_by_id(peer);
		dp_wdi_event_handler(WDI_EVENT_TX_DATA, pdev->soc,
				     &tx_capture_info, HTT_INVALID_PEER,
				     WDI_NO_VAL, pdev->pdev_id);

	}

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_peer_set_tx_capture_enabled: Set tx_cap_enabled bit in peer
 * @peer_handle: Peer handle
 * @value: Enable/disable setting for tx_cap_enabled
 *
 * Return: None
 */
void
dp_peer_set_tx_capture_enabled(struct cdp_peer *peer_handle, bool value)
{
	struct dp_peer *peer = (struct dp_peer *)peer_handle;

	peer->tx_cap_enabled = value;
	if (!value)
		dp_peer_tx_cap_tid_queue_flush(peer);
}
#endif
