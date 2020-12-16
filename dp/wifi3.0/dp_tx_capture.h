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
#ifndef _DP_TX_CAPTURE_H_
#define _DP_TX_CAPTURE_H_

#ifdef WLAN_TX_PKT_CAPTURE_ENH

#define DP_TX_PPDU_PROC_MAX_DEPTH 512

struct dp_soc;
struct dp_pdev;
struct dp_vdev;
struct dp_peer;
struct dp_tx_desc_s;

#define TX_CAP_HTT_MAX_FTYPE 19

#define TXCAP_MAX_TYPE \
	((IEEE80211_FC0_TYPE_DATA >> IEEE80211_FC0_TYPE_SHIFT) + 1)
#define TXCAP_MAX_SUBTYPE \
	((IEEE80211_FC0_SUBTYPE_MASK >> IEEE80211_FC0_SUBTYPE_SHIFT) + 1)

#define SIFS_INTERVAL 16

#define RTS_INTERVAL 40

#define MAX_MGMT_PEER_FILTER 16

/* Macro for Debugfs */
#define TX_CAP_DBG_FILE_PERM	(QDF_FILE_USR_READ | QDF_FILE_USR_WRITE | \
				 QDF_FILE_GRP_READ | \
				 QDF_FILE_OTH_READ)
#define NUM_TX_CAP_DEBUG_INFOS 2
#define MAX_TX_CAP_QUEUE_SIZE 30

#define PPDU_LOG_ENABLE_LIST 1
#define PPDU_LOG_DISPLAY_LIST 2

#define TX_CAPTURE_WORK_Q_TIMER_MS 10

/* stats */
enum CDP_PEER_MSDU_DESC {
	PEER_MSDU_SUCC,
	PEER_MSDU_ENQ,
	PEER_MSDU_DEQ,
	PEER_MSDU_FLUSH,
	PEER_MSDU_DROP,
	PEER_MSDU_XRETRY,
	PEER_MSDU_DESC_MAX,
};

enum CDP_PEER_MPDU_DESC {
	PEER_MPDU_TRI,
	PEER_MPDU_SUCC,
	PEER_MPDU_RESTITCH,
	PEER_MPDU_ARR,
	PEER_MPDU_CLONE,
	PEER_MPDU_TO_STACK,
	PEER_MPDU_DESC_MAX,
};

#ifdef WLAN_TX_PKT_CAPTURE_ENH_DEBUG
struct dp_peer_tx_capture_stats {
	/* mpdu success and restich count */
	uint32_t mpdu[PEER_MPDU_DESC_MAX];
	/*msdu success and restich count */
	uint32_t msdu[PEER_MSDU_DESC_MAX];
};
#endif

struct dp_peer_mgmt_list {
	uint8_t mac_addr[QDF_MAC_ADDR_SIZE];
	uint32_t mgmt_pkt_counter;
	uint16_t peer_id;
	bool avail;
};

struct dp_tx_cap_nbuf_list {
	qdf_nbuf_t nbuf_ppdu;
	uint8_t ref_cnt;
};

struct tx_cap_debugfs_info {
	const char *name;
	struct qdf_debugfs_fops ops;
};

struct tx_cap_debug_log_info {
	struct dentry *debugfs_de[NUM_DEBUG_INFOS];
	qdf_spinlock_t tx_cap_record_lock;
	qdf_dentry_t tx_cap_debugfs_dir;
	qdf_list_t ppdu_dbg_queue;
	qdf_spinlock_t dbg_log_lock;
	uint8_t ppdu_desc_log;
	uint8_t pause_dbg_log;
	uint8_t stop_seq;
	uint8_t ppdu_queue_size;
	struct tx_cap_debugfs_info tx_cap_debugfs_infos[NUM_TX_CAP_DEBUG_INFOS];
};

struct dp_pdev_tx_capture {
	/* For deferred PPDU status processing */
	qdf_spinlock_t ppdu_stats_lock;
	qdf_workqueue_t *ppdu_stats_workqueue;

	qdf_work_t ppdu_stats_work;

	STAILQ_HEAD(, ppdu_info) ppdu_stats_queue;

	STAILQ_HEAD(, ppdu_info) ppdu_stats_defer_queue;

	uint32_t ppdu_stats_queue_depth;
	uint32_t ppdu_stats_defer_queue_depth;
	uint32_t ppdu_stats_next_sched;
	qdf_spinlock_t msdu_comp_q_list_lock;
	qdf_timer_t work_q_timer;
	uint32_t missed_ppdu_id;
	uint32_t last_msdu_id;
	uint16_t last_peer_id;
	qdf_event_t miss_ppdu_event;
	uint32_t ppdu_dropped;
	uint32_t pend_ppdu_dropped;
	qdf_nbuf_queue_t ctl_mgmt_q[TXCAP_MAX_TYPE][TXCAP_MAX_SUBTYPE];
	qdf_nbuf_queue_t retries_ctl_mgmt_q[TXCAP_MAX_TYPE][TXCAP_MAX_SUBTYPE];
	qdf_spinlock_t ctl_mgmt_lock[TXCAP_MAX_TYPE][TXCAP_MAX_SUBTYPE];
	uint32_t htt_frame_type[TX_CAP_HTT_MAX_FTYPE];
	struct cdp_tx_completion_ppdu *dummy_ppdu_desc;
	struct dp_peer_mgmt_list *ptr_peer_mgmt_list;
	qdf_atomic_t tx_cap_usr_mode;
	struct tx_cap_debug_log_info log_info;
	struct dp_tx_cap_nbuf_list *last_nbuf_ppdu_list;
	uint32_t last_nbuf_ppdu_list_arr_sz;
	uint64_t peer_mismatch;
};

/* Tx TID */
struct dp_tx_tid {
	/* TID */
	uint8_t tid;
	/* peer id */
	uint16_t peer_id;
	/* max ppdu_id in a tid */
	uint32_t max_ppdu_id;
	/* tx_tid lock */
	qdf_spinlock_t tid_lock;
	qdf_spinlock_t tasklet_tid_lock;
	qdf_nbuf_queue_t defer_msdu_q;
	qdf_nbuf_queue_t msdu_comp_q;
	qdf_nbuf_queue_t pending_ppdu_q;
	struct cdp_tx_completion_ppdu *xretry_ppdu;
	uint16_t first_data_seq_ctrl;
	uint32_t mpdu_cnt;
	uint32_t mpdu_fcs_ok_bitmap[QDF_MON_STATUS_MPDU_FCS_BMAP_NWORDS];
	unsigned long tid_flags;
};

struct dp_peer_tx_capture {
	struct dp_tx_tid tx_tid[DP_MAX_TIDS];

	/*
	 * for a ppdu, framectrl and qos ctrl is same
	 * for each ppdu, we update and used on rest of mpdu
	 */
	uint32_t tx_wifi_ppdu_id;
	union {
		struct ieee80211_frame tx_wifi_hdr;
		struct ieee80211_qosframe tx_wifi_qos_hdr;
	};
	union {
		struct ieee80211_frame_addr4 tx_wifi_addr4_hdr;
		struct ieee80211_qosframe_addr4 tx_wifi_addr4_qos_hdr;
	};
#ifdef WLAN_TX_PKT_CAPTURE_ENH_DEBUG
	struct dp_peer_tx_capture_stats stats;
#endif

	/* TID is not initialized for a STA VAP */
	bool is_tid_initialized;
};

/* per user ppdu desc structure for debugfs */
struct dbg_tx_comp_ppdu_user {
	uint32_t completion_status:8,
		tid:8,
		peer_id:16;
	uint8_t mac_addr[6];
	uint32_t frame_ctrl:16,
		 qos_ctrl:16;
	uint32_t mpdu_tried;
	uint16_t mpdu_success:16;
	uint32_t ltf_size:2,
		 stbc:1,
		 he_re:1,
		 txbf:4,
		 bw:4,
		 nss:4,
		 mcs:4,
		 preamble:4,
		 gi:4,
		 dcm:1,
		 ldpc:1,
		 delayed_ba:1,
		 ack_ba_tlv:1;
	uint32_t ba_seq_no;
	uint32_t ba_bitmap[CDP_BA_256_BIT_MAP_SIZE_DWORDS];
	uint32_t start_seq;
	uint32_t enq_bitmap[CDP_BA_256_BIT_MAP_SIZE_DWORDS];
	uint32_t num_mpdu:9,
		 num_msdu:16;
	bool is_mcast;
	uint32_t tlv_bitmap;
};

/* ppdu desc structure for debugfs */
struct dbg_tx_comp_ppdu {
	qdf_list_node_t node;
	uint32_t ppdu_id;
	uint32_t bar_ppdu_id;
	uint16_t vdev_id;
	uint16_t bar_num_users;
	uint32_t num_users;
	uint16_t sched_cmdid;
	uint32_t tlv_bitmap;
	uint16_t htt_frame_type;
	uint16_t frame_ctrl;
	uint64_t ppdu_start_timestamp;
	uint64_t ppdu_end_timestamp;
	uint32_t bar_ppdu_start_timestamp;
	uint32_t bar_ppdu_end_timestamp;

	struct dbg_tx_comp_ppdu_user user[];
};

struct tid_q_len {
	uint64_t defer_msdu_len;
	uint64_t tasklet_msdu_len;
	uint64_t pending_q_len;
};
/*
 * dp_peer_tid_peer_id_update() – update peer_id to tid structure
 * @peer: Datapath peer
 * @peer_id: peer_id
 *
 */
void dp_peer_tid_peer_id_update(struct dp_peer *peer, uint16_t peer_id);

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
dp_config_enh_tx_capture(struct dp_pdev *pdev_handle, uint8_t val);

/*
 * dp_deliver_mgmt_frm: Process
 * @pdev: DP PDEV handle
 * @nbuf: buffer containing the htt_ppdu_stats_tx_mgmtctrl_payload_tlv
 *
 * return: void
 */
void dp_deliver_mgmt_frm(struct dp_pdev *pdev, qdf_nbuf_t nbuf);

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

/*
 * dp_tx_capture_htt_frame_counter: increment counter for htt_frame_type
 * pdev: DP pdev handle
 * htt_frame_type: htt frame type received from fw
 *
 * return: void
 */
void dp_tx_capture_htt_frame_counter(struct dp_pdev *pdev,
				     uint32_t htt_frame_type);

/*
 * dp_print_pdev_tx_capture_stats: print tx capture stats
 * @pdev: DP PDEV handle
 *
 * return: void
 */
void dp_print_pdev_tx_capture_stats(struct dp_pdev *pdev);

/*
 * dp_iterate_print_tid_qlen_per_peer()- API to print peer tid msdu queue
 * @pdev_handle: DP_PDEV handle
 * @consolidated: consolidated flag
 *
 * Return: void
 */
void dp_print_tid_qlen_per_peer(void *pdev_hdl, uint8_t consolidated);

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
				      struct hal_rx_ppdu_info *ppdu_info);

/**
 * dp_handle_tx_capture_from_dest: Handle any TX capture frames from
 * monitor destination path.
 * @soc: SoC handle
 * @pdev: PDEV pointer
 * @mon_mpdu: mpdu from monitor destination path
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
dp_handle_tx_capture_from_dest(struct dp_soc *soc, struct dp_pdev *pdev,
			       qdf_nbuf_t mon_mpdu);

/**
 * dp_bar_send_ack_frm_to_stack(): send  ACK frame
 * to upper layers on received BAR packet for tx capture feature
 *
 * @soc: soc handle
 * @pdev: pdev handle
 * @nbuf: received packet
 *
 * Return: QDF_STATUS_SUCCESS on success
 *         others on error
 */
QDF_STATUS dp_bar_send_ack_frm_to_stack(struct dp_soc *soc,
			      struct dp_pdev *pdev,
			      qdf_nbuf_t nbuf);

/**
 * dp_peer_set_tx_capture_enabled: Set tx_cap_enabled bit in peer
 * @pdev: DP PDEV handle
 * @peer: Peer handle
 * @value: Enable/disable setting for tx_cap_enabled
 * @peer_mac: peer_mac Enable/disable setting for tx_cap_enabled
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
dp_peer_set_tx_capture_enabled(struct dp_pdev *pdev,
			       struct dp_peer *peer, uint8_t value,
			       uint8_t *peer_mac);

/*
 * dp_peer_tx_cap_add_filter: add peer filter mgmt pkt based on peer
 * and mac address
 * @pdev: DP PDEV handle
 * @peer_id: DP PEER ID
 * @mac_addr: pointer to mac address
 *
 * return: true on added and false on not failed
 */
bool dp_pdev_tx_cap_add_filter(struct dp_pdev *pdev,
			       uint16_t peer_id, uint8_t *mac_addr);

/*
 * dp_peer_tx_cap_del_filter: delete peer filter mgmt pkt based on peer
 * and mac address
 * @pdev: DP PDEV handle
 * @peer_id: DP PEER ID
 * @mac_addr: pointer to mac address
 *
 * return: true on added and false on not failed
 */
bool dp_peer_tx_cap_del_filter(struct dp_pdev *pdev,
			       uint16_t peer_id, uint8_t *mac_addr);

/*
 * dp_peer_tx_cap_print_mgmt_filter: pradd peer filter mgmt pkt based on peer
 * and mac address
 * @pdev: DP PDEV handle
 * @peer_id: DP PEER ID
 * @mac_addr: pointer to mac address
 *
 * return: true on added and false on not failed
 */
void dp_peer_tx_cap_print_mgmt_filter(struct dp_pdev *pdev,
				      uint16_t peer_id, uint8_t *mac_addr);

/*
 * dp_peer_mgmt_pkt_filter: filter mgmt pkt based on peer and mac address
 * @pdev: DP PDEV handle
 * @nbuf: buffer containing the ppdu_desc
 *
 * return: status
 */
bool is_dp_peer_mgmt_pkt_filter(struct dp_pdev *pdev,
				uint32_t peer_id, uint8_t *mac_addr);

/*
 * dp_peer_tx_capture_filter_check: check filter is enable for the filter
 * and update tx_cap_enabled flag
 * @pdev: DP PDEV handle
 * @peer: DP PEER handle
 *
 * return: void
 */
void dp_peer_tx_capture_filter_check(struct dp_pdev *pdev,
				     struct dp_peer *peer);

/*
 * dp_tx_capture_debugfs_init: tx capture debugfs init
 * @pdev: DP PDEV handle
 *
 * return: QDF_STATUS
 */
QDF_STATUS dp_tx_capture_debugfs_init(struct dp_pdev *pdev);

/*
 * dp_tx_capture_debugfs_deinit: tx capture debugfs deinit
 * @pdev: DP PDEV handle
 *
 * return: void
 */
void dp_tx_capture_debugfs_deinit(struct dp_pdev *pdev);

/*
 * tx_cap_debugfs_log_ppdu_desc: tx capture logging ppdu desc
 * @pdev: DP PDEV handle
 * @nbuf_ppdu: nbuf ppdu
 *
 * return: void
 */
void tx_cap_debugfs_log_ppdu_desc(struct dp_pdev *pdev, qdf_nbuf_t nbuf_ppdu);
#endif
#endif
