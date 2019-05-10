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

#ifndef _DP_TX_CAPTURE_H_
#define _DP_TX_CAPTURE_H_

#ifdef WLAN_TX_PKT_CAPTURE_ENH

struct dp_soc;
struct dp_pdev;
struct dp_vdev;
struct dp_peer;
struct dp_tx_desc_s;

struct dp_pdev_tx_capture {
	/* For deferred PPDU status processing */
	qdf_spinlock_t ppdu_stats_lock;
	qdf_workqueue_t *ppdu_stats_workqueue;

	qdf_work_t ppdu_stats_work;

	STAILQ_HEAD(, ppdu_info) ppdu_stats_queue;

	STAILQ_HEAD(, ppdu_info) ppdu_stats_defer_queue;

	uint32_t ppdu_stats_queue_depth;
	uint32_t ppdu_stats_next_sched;
	qdf_spinlock_t msdu_comp_q_list_lock;
	uint32_t missed_ppdu_id;
	uint32_t last_msdu_id;
	qdf_event_t miss_ppdu_event;
};

/* Tx TID */
struct dp_tx_tid {
	/* TID */
	uint8_t tid;
	/* max ppdu_id in a tid */
	uint32_t max_ppdu_id;
	/* tx_tid lock */
	qdf_spinlock_t tid_lock;
	qdf_nbuf_queue_t msdu_comp_q;
};

struct dp_peer_tx_capture {
	struct dp_tx_tid tx_tid[DP_MAX_TIDS];

	/*
	 * for a ppdu, framectrl and qos ctrl is same
	 * for each ppdu, we update and used on rest of mpdu
	 */
	uint32_t tx_wifi_ppdu_id;
	struct ieee80211_frame tx_wifi_hdr;
	struct ieee80211_qoscntl tx_qoscntl;
};

/*
 * dp_peer_tid_queue_init() – Initialize ppdu stats queue per TID
 * @peer: Datapath peer
 *
 */
void dp_peer_tid_queue_init(struct dp_peer *peer);

/*
 * dp_peer_tid_queue_cleanup() – remove ppdu stats queue per TID
 * @peer: Datapath peer
 *
 */
void dp_peer_tid_queue_cleanup(struct dp_peer *peer);

/*
 * dp_peer_update_80211_hdr: update 80211 hdr
 * @vdev: DP VDEV
 * @peer: DP PEER
 *
 * return: void
 */
void dp_peer_update_80211_hdr(struct dp_vdev *vdev, struct dp_peer *peer);

/**
 * dp_tx_ppdu_stats_attach - Initialize Tx PPDU stats and enhanced capture
 * @pdev: DP PDEV
 *
 * Return: none
 */
void dp_tx_ppdu_stats_attach(struct dp_pdev *pdev);

/**
 * dp_tx_ppdu_stats_detach - Cleanup Tx PPDU stats and enhanced capture
 * @pdev: DP PDEV
 *
 * Return: none
 */
void dp_tx_ppdu_stats_detach(struct dp_pdev *pdev);

/**
 * dp_process_ppdu_stats_update_failed_bitmap(): Function to
 * get ppdu stats update
 * @pdev: dp_pdev
 * @data: tx completion ppdu desc
 * @ppdu_id: ppdu id
 * @size: bitmap size
 *
 * return: status
 */
void dp_process_ppdu_stats_update_failed_bitmap(struct dp_pdev *pdev,
						void *data,
						uint32_t ppdu_id,
						uint32_t size);

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
		       qdf_nbuf_t netbuf);

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
				   struct dp_peer *peer);

/*
 * dp_config_enh_tx_capture()- API to enable/disable enhanced tx capture
 * @pdev_handle: DP_PDEV handle
 * @val: user provided value
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
dp_config_enh_tx_capture(struct cdp_pdev *pdev_handle, int val);

/*
 * dp_deliver_mgmt_frm: Process
 * @pdev: DP PDEV handle
 * @nbuf: buffer containing the htt_ppdu_stats_tx_mgmtctrl_payload_tlv
 *
 * return: void
 */
void dp_deliver_mgmt_frm(struct dp_pdev *pdev, qdf_nbuf_t nbuf);

/**
 * dp_tx_ppdu_stats_attach(): Function to initialize tx ppdu stats on attach
 * @pdev: dp_pdev
 *
 * return: status
 */
void dp_tx_ppdu_stats_attach(struct dp_pdev *pdev);

/**
 * dp_tx_ppdu_stats_detach(): Function to deinit tx ppdu stats on detach
 * @pdev: dp_pdev
 *
 * return: status
 */
void dp_tx_ppdu_stats_detach(struct dp_pdev *pdev);

/**
 * dp_process_ppdu_stats_process(): workqueue function for ppdu stats
 * @context: context
 *
 * return: status
 */
void dp_tx_ppdu_stats_process(void *context);

/**
 * dp_ppdu_desc_deliver(): Function to deliver Tx PPDU status descriptor
 * to upper layer
 * @pdev: DP pdev handle
 * @ppdu_info: per PPDU TLV descriptor
 *
 * return: void
 */
void dp_ppdu_desc_deliver(struct dp_pdev *pdev,
			  struct ppdu_info *ppdu_info);

#endif
#endif
