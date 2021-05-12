/*
 * Copyright (c) 2016-2021, The Linux Foundation. All rights reserved.

 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.

 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#define mon_rx_warn(params...) QDF_TRACE_WARN(QDF_MODULE_ID_DP_RX, params)
struct dp_mon_ops {
	QDF_STATUS (*mon_soc_cfg_init)(struct dp_soc *soc);
	QDF_STATUS (*mon_pdev_attach)(struct dp_pdev *pdev);
	QDF_STATUS (*mon_pdev_detach)(struct dp_pdev *pdev);
	QDF_STATUS (*mon_pdev_init)(struct dp_pdev *pdev);
	QDF_STATUS (*mon_pdev_deinit)(struct dp_pdev *pdev);
	QDF_STATUS (*mon_config_debug_sniffer)(struct dp_pdev *pdev, int val);
	void (*mon_flush_rings)(struct dp_soc *soc);
#if !defined(DISABLE_MON_CONFIG)
	QDF_STATUS (*mon_htt_srng_setup)(struct dp_soc *soc,
					 struct dp_pdev *pdev,
					 int mac_id,
					 int mac_for_pdev);
#endif
#if !defined(DISABLE_MON_CONFIG) && defined(MON_ENABLE_DROP_FOR_MAC)
	uint32_t (*mon_drop_packets_for_mac)(struct dp_pdev *pdev,
					     uint32_t mac_id,
					     uint32_t quota);
#endif
#if defined(DP_CON_MON)
	void (*mon_service_rings)(struct  dp_soc *soc, uint32_t quota);
#endif
#ifndef DISABLE_MON_CONFIG
	uint32_t (*mon_process)(struct dp_soc *soc,
				struct dp_intr *int_ctx,
				uint32_t mac_id,
				uint32_t quota);
#endif
	void (*mon_peer_tx_init)(struct dp_pdev *pdev, struct dp_peer *peer);
	void (*mon_peer_tx_cleanup)(struct dp_vdev *vdev,
				    struct dp_peer *peer);
#ifdef WLAN_TX_PKT_CAPTURE_ENH
	void (*mon_peer_tid_peer_id_update)(struct dp_peer *peer,
					    uint16_t peer_id);
	void (*mon_tx_ppdu_stats_attach)(struct dp_pdev *pdev);
	void (*mon_tx_ppdu_stats_detach)(struct dp_pdev *pdev);
	QDF_STATUS (*mon_tx_capture_debugfs_init)(struct dp_pdev *pdev);
	void (*mon_peer_tx_capture_filter_check)(struct dp_pdev *pdev,
						 struct dp_peer *peer);
	QDF_STATUS (*mon_tx_add_to_comp_queue)(struct dp_soc *soc,
					       struct dp_tx_desc_s *desc,
					       struct hal_tx_completion_status *ts,
					       struct dp_peer *peer);
#endif
#if defined(WDI_EVENT_ENABLE) && !defined(REMOVE_PKT_LOG)
	bool (*mon_ppdu_stats_ind_handler)(struct htt_soc *soc,
					   uint32_t *msg_word,
					   qdf_nbuf_t htt_t2h_msg);
#endif
	QDF_STATUS (*mon_htt_ppdu_stats_attach)(struct dp_pdev *pdev);
	void (*mon_htt_ppdu_stats_detach)(struct dp_pdev *pdev);
	void (*mon_print_pdev_rx_mon_stats)(struct dp_pdev *pdev);

#ifdef WLAN_TX_PKT_CAPTURE_ENH
	void (*mon_print_pdev_tx_capture_stats)(struct dp_pdev *pdev);
	QDF_STATUS (*mon_config_enh_tx_capture)(struct dp_pdev *pdev,
						uint8_t val);
#endif
#ifdef WLAN_RX_PKT_CAPTURE_ENH
	QDF_STATUS (*mon_config_enh_rx_capture)(struct dp_pdev *pdev,
						uint8_t val);
#endif
	QDF_STATUS (*mon_set_bpr_enable)(struct dp_pdev *pdev, int val);
#ifdef ATH_SUPPORT_NAC
	int (*mon_set_filter_neigh_peers)(struct dp_pdev *pdev, bool val);
#endif
#ifdef WLAN_ATF_ENABLE
	void (*mon_set_atf_stats_enable)(struct dp_pdev *pdev, bool value);
#endif
	void (*mon_set_bsscolor)(struct dp_pdev *pdev, uint8_t bsscolor);
	bool (*mon_pdev_get_filter_ucast_data)(struct cdp_pdev *pdev_handle);
	bool (*mon_pdev_get_filter_non_data)(struct cdp_pdev *pdev_handle);
	bool (*mon_pdev_get_filter_mcast_data)(struct cdp_pdev *pdev_handle);
#ifdef WDI_EVENT_ENABLE
	int (*mon_set_pktlog_wifi3)(struct dp_pdev *pdev, uint32_t event,
				    bool enable);
#endif
#if defined(DP_CON_MON) && !defined(REMOVE_PKT_LOG)
	void (*mon_pktlogmod_exit)(struct dp_pdev *pdev);
#endif
	void (*mon_vdev_set_monitor_mode_buf_rings)(struct dp_pdev *pdev);
	void (*mon_neighbour_peers_detach)(struct dp_pdev *pdev);
#ifdef FEATURE_NAC_RSSI
	QDF_STATUS (*mon_filter_neighbour_peer)(struct dp_pdev *pdev,
						uint8_t *rx_pkt_hdr);
#endif
};

struct dp_mon_soc {
	/* Holds all monitor related fields extracted from dp_soc */
	/* Holds pointer to monitor ops */

	struct dp_mon_ops *mon_ops;
};

struct  dp_mon_pdev {
};

struct  dp_mon_vdev {
};

struct dp_mon_peer {
};

#ifdef FEATURE_PERPKT_INFO
void dp_deliver_mgmt_frm(struct dp_pdev *pdev, qdf_nbuf_t nbuf);
#else
static inline
void dp_deliver_mgmt_frm(struct dp_pdev *pdev, qdf_nbuf_t nbuf)
{
}
#endif

#ifndef WLAN_TX_PKT_CAPTURE_ENH
/**
 * dp_peer_tid_queue_init() – Initialize ppdu stats queue per TID
 * @peer: Datapath peer
 *
 */
static inline void dp_peer_tid_queue_init(struct dp_peer *peer)
{
}

/**
 * dp_peer_tid_queue_cleanup() – remove ppdu stats queue per TID
 * @peer: Datapath peer
 *
 */
static inline void dp_peer_tid_queue_cleanup(struct dp_peer *peer)
{
}

/**
 * dp_peer_update_80211_hdr() – dp peer update 80211 hdr
 * @vdev: Datapath vdev
 * @peer: Datapath peer
 *
 */
static inline void
dp_peer_update_80211_hdr(struct dp_vdev *vdev, struct dp_peer *peer)
{
}

/**
 * dp_tx_ppdu_stats_attach - Initialize Tx PPDU stats and enhanced capture
 * @pdev: DP PDEV
 *
 * Return: none
 */
static inline void dp_tx_ppdu_stats_attach(struct dp_pdev *pdev)
{
}

/**
 * dp_tx_ppdu_stats_detach - Cleanup Tx PPDU stats and enhanced capture
 * @pdev: DP PDEV
 *
 * Return: none
 */
static inline void dp_tx_ppdu_stats_detach(struct dp_pdev *pdev)
{
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
static inline
QDF_STATUS dp_tx_add_to_comp_queue(struct dp_soc *soc,
				   struct dp_tx_desc_s *desc,
				   struct hal_tx_completion_status *ts,
				   struct dp_peer *peer)
{
	return QDF_STATUS_E_FAILURE;
}

/*
 * dp_peer_tx_capture_filter_check: check filter is enable for the filter
 * and update tx_cap_enabled flag
 * @pdev: DP PDEV handle
 * @peer: DP PEER handle
 *
 * return: void
 */
static inline
void dp_peer_tx_capture_filter_check(struct dp_pdev *pdev,
				     struct dp_peer *peer)
{
}

/*
 * dp_tx_capture_debugfs_init: tx capture debugfs init
 * @pdev: DP PDEV handle
 *
 * return: QDF_STATUS
 */
static inline
QDF_STATUS dp_tx_capture_debugfs_init(struct dp_pdev *pdev)
{
	return QDF_STATUS_E_FAILURE;
}
#endif

#ifdef WLAN_TX_PKT_CAPTURE_ENH
extern uint8_t
dp_cpu_ring_map[DP_NSS_CPU_RING_MAP_MAX][WLAN_CFG_INT_NUM_CONTEXTS_MAX];
#endif

#ifdef WDI_EVENT_ENABLE
void dp_pkt_log_init(struct cdp_soc_t *soc_hdl, uint8_t pdev_id, void *scn);
#else
static inline void
dp_pkt_log_init(struct cdp_soc_t *soc_hdl, uint8_t pdev_id, void *scn)
{
}
#endif

#ifdef WDI_EVENT_ENABLE
QDF_STATUS dp_peer_stats_notify(struct dp_pdev *pdev, struct dp_peer *peer);
#else
static inline QDF_STATUS dp_peer_stats_notify(struct dp_pdev *pdev,
					      struct dp_peer *peer)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#if defined(DP_CON_MON) && !defined(REMOVE_PKT_LOG)
static void dp_pktlogmod_exit(struct dp_pdev *handle);
#else
static void dp_pktlogmod_exit(struct dp_pdev *handle) { }
#endif

#ifndef WLAN_TX_PKT_CAPTURE_ENH
/**
 * dp_tx_ppdu_stats_process - Deferred PPDU stats handler
 * @context: Opaque work context (PDEV)
 *
 * Return: none
 */
static  inline void dp_tx_ppdu_stats_process(void *context)
{
}

/*
 * dp_tx_capture_htt_frame_counter: increment counter for htt_frame_type
 * pdev: DP pdev handle
 * htt_frame_type: htt frame type received from fw
 *
 * return: void
 */
static inline
void dp_tx_capture_htt_frame_counter(struct dp_pdev *pdev,
				     uint32_t htt_frame_type)
{
}

/*
 * dp_tx_cature_stats: print tx capture stats
 * @pdev: DP PDEV handle
 *
 * return: void
 */
static inline
void dp_print_pdev_tx_capture_stats(struct dp_pdev *pdev)
{
}
#endif

/*
 * dp_is_enable_reap_timer_non_pkt() - check if mon reap timer is
 * enabled by non-pkt log or not
 * @pdev: point to dp pdev
 *
 * Return: true if mon reap timer is enabled by non-pkt log
 */
static inline bool dp_is_enable_reap_timer_non_pkt(struct dp_pdev *pdev)
{
	if (!pdev) {
		dp_err("null pdev");
		return false;
	}

	return pdev->enable_reap_timer_non_pkt;
}

#ifdef FEATURE_NAC_RSSI
static inline QDF_STATUS monitor_drop_inv_peer_pkts(struct dp_vdev *vdev,
						    struct ieee80211_frame *wh)
{
	struct dp_pdev *pdev = vdev->pdev;
	struct dp_soc *soc = pdev->soc;

	if (!soc->hw_nac_monitor_support &&
	    pdev->filter_neighbour_peers &&
	    vdev->opmode == wlan_op_mode_sta) {
		mon_rx_warn("%pK: Drop inv peer pkts with STA RA:%pm",
			    soc, wh->i_addr1);
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_FAILURE;
}
#endif

static inline QDF_STATUS monitor_pdev_attach(struct dp_pdev *pdev)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	/*
	 * mon_soc uninitialized modular support enabled
	 * monitor related attach/detach/init/deinit
	 * will be done while monitor insmod
	 */
	if (!mon_soc)
		return QDF_STATUS_SUCCESS;

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_pdev_attach) {
		qdf_err("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_pdev_attach(pdev);
}

static inline QDF_STATUS monitor_pdev_detach(struct dp_pdev *pdev)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	/*
	 * mon_soc uninitialized modular support enabled
	 * monitor related attach/detach/init/deinit
	 * will be done while monitor insmod
	 */
	if (!mon_soc)
		return QDF_STATUS_SUCCESS;

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_pdev_detach) {
		qdf_err("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_pdev_detach(pdev);
}

static inline QDF_STATUS monitor_pdev_init(struct dp_pdev *pdev)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	/*
	 * mon_soc uninitialized modular support enabled
	 * monitor related attach/detach/init/deinit
	 * will be done while monitor insmod
	 */
	if (!mon_soc)
		return QDF_STATUS_SUCCESS;

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_pdev_init) {
		qdf_err("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_pdev_init(pdev);
}

static inline QDF_STATUS monitor_pdev_deinit(struct dp_pdev *pdev)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	/*
	 * mon_soc uninitialized modular support enabled
	 * monitor related attach/detach/init/deinit
	 * will be done while monitor insmod
	 */
	if (!mon_soc)
		return QDF_STATUS_SUCCESS;

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_pdev_deinit) {
		qdf_err("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_pdev_deinit(pdev);
}

static inline QDF_STATUS monitor_soc_cfg_init(struct dp_soc *soc)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

	/*
	 * this API is getting call from dp_soc_init,
	 * mon_soc will be uninitialized for WIN here
	 * So returning QDF_STATUS_SUCCESS.
	 * For WIN, soc cfg init is done while monitor insmod.
	 */
	if (!mon_soc)
		return QDF_STATUS_SUCCESS;

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_soc_cfg_init) {
		qdf_err("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_soc_cfg_init(soc);
}

static inline QDF_STATUS monitor_config_debug_sniffer(struct dp_pdev *pdev,
						      int val)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc)
		return QDF_STATUS_E_FAILURE;

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_config_debug_sniffer) {
		qdf_err("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_config_debug_sniffer(pdev, val);
}

static inline void monitor_flush_rings(struct dp_soc *soc)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

	if (!mon_soc) {
		qdf_err("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_flush_rings) {
		qdf_err("callback not registered");
		return;
	}

	return monitor_ops->mon_flush_rings(soc);
}

#if !defined(DISABLE_MON_CONFIG)
static inline QDF_STATUS monitor_htt_srng_setup(struct dp_soc *soc,
						struct dp_pdev *pdev,
						int mac_id,
						int mac_for_pdev)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

	if (!mon_soc) {
		qdf_err("monitor soc is NULL");
		return QDF_STATUS_SUCCESS;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_htt_srng_setup) {
		qdf_err("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_htt_srng_setup(soc, pdev, mac_id,
					       mac_for_pdev);
}
#else
static inline QDF_STATUS monitor_htt_srng_setup(struct dp_soc *soc,
						struct dp_pdev *pdev,
						int mac_id,
						int mac_for_pdev)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#if defined(DP_CON_MON)
static inline void monitor_service_mon_rings(struct dp_soc *soc, uint32_t quota)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

	if (!mon_soc) {
		qdf_err("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_service_rings) {
		qdf_err("callback not registered");
		return;
	}

	return monitor_ops->mon_service_rings(soc, quota);
}
#endif

#ifndef DISABLE_MON_CONFIG
static inline
uint32_t monitor_process(struct dp_soc *soc, struct dp_intr *int_ctx,
			 uint32_t mac_id, uint32_t quota)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

	if (!mon_soc) {
		qdf_err("monitor soc is NULL");
		return 0;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_process) {
		qdf_err("callback not registered");
		return 0;
	}

	return monitor_ops->mon_process(soc, int_ctx, mac_id, quota);
}
#else
static inline
uint32_t monitor_process(struct dp_soc *soc, struct dp_intr *int_ctx,
			 uint32_t mac_id, uint32_t quota)
{
	return 0;
}
#endif

#if !defined(DISABLE_MON_CONFIG) && defined(MON_ENABLE_DROP_FOR_MAC)
static inline
uint32_t monitor_drop_packets_for_mac(struct dp_pdev *pdev,
				      uint32_t mac_id, uint32_t quota)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		qdf_err("monitor soc is NULL");
		return 0;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_drop_packets_for_mac) {
		qdf_err("callback not registered");
		return 0;
	}

	return monitor_ops->mon_drop_packets_for_mac(pdev,
						     mac_id, quota);
}
#else
static inline
uint32_t monitor_drop_packets_for_mac(struct dp_pdev *pdev,
				      uint32_t mac_id, uint32_t quota)
{
	return 0;
}
#endif

static inline void monitor_peer_tx_init(struct dp_pdev *pdev,
					struct dp_peer *peer)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		qdf_err("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_peer_tx_init) {
		qdf_err("callback not registered");
		return;
	}

	return monitor_ops->mon_peer_tx_init(pdev, peer);
}

static inline void monitor_peer_tx_cleanup(struct dp_vdev *vdev,
					   struct dp_peer *peer)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = vdev->pdev->soc->monitor_soc;

	if (!mon_soc) {
		qdf_err("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_peer_tx_cleanup) {
		qdf_err("callback not registered");
		return;
	}

	return monitor_ops->mon_peer_tx_cleanup(vdev, peer);
}

#ifdef WLAN_TX_PKT_CAPTURE_ENH
static inline
void monitor_peer_tid_peer_id_update(struct dp_soc *soc,
				     struct dp_peer *peer,
				     uint16_t peer_id)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

	if (!mon_soc) {
		qdf_err("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_peer_tid_peer_id_update) {
		qdf_err("callback not registered");
		return;
	}

	return monitor_ops->mon_peer_tid_peer_id_update(peer, peer_id);
}

static inline void monitor_tx_ppdu_stats_attach(struct dp_pdev *pdev)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		qdf_err("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_tx_ppdu_stats_attach) {
		qdf_err("callback not registered");
		return;
	}

	return monitor_ops->mon_tx_ppdu_stats_attach(pdev);
}

static inline void monitor_tx_ppdu_stats_detach(struct dp_pdev *pdev)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		qdf_err("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_tx_ppdu_stats_detach) {
		qdf_err("callback not registered");
		return;
	}

	return monitor_ops->mon_tx_ppdu_stats_detach(pdev);
}

static inline QDF_STATUS monitor_tx_capture_debugfs_init(struct dp_pdev *pdev)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		qdf_err("monitor soc is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_tx_capture_debugfs_init) {
		qdf_err("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_tx_capture_debugfs_init(pdev);
}

static inline void monitor_peer_tx_capture_filter_check(struct dp_pdev *pdev,
							struct dp_peer *peer)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		qdf_err("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_peer_tx_capture_filter_check) {
		qdf_err("callback not registered");
		return;
	}

	return monitor_ops->mon_peer_tx_capture_filter_check(pdev, peer);
}

static inline
QDF_STATUS monitor_tx_add_to_comp_queue(struct dp_soc *soc,
					struct dp_tx_desc_s *desc,
					struct hal_tx_completion_status *ts,
					struct dp_peer *peer)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

	if (!mon_soc) {
		qdf_err("monitor soc is NULL");
		return QDF_STATUS_SUCCESS;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_tx_add_to_comp_queue) {
		qdf_err("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_tx_add_to_comp_queue(soc, desc, ts, peer);
}

#else
static inline void monitor_peer_tid_peer_id_update(struct dp_soc *soc,
						   struct dp_peer *peer,
						   uint16_t peer_id)
{
}

static inline void monitor_tx_ppdu_stats_attach(struct dp_pdev *pdev)
{
}

static inline void monitor_tx_ppdu_stats_detach(struct dp_pdev *pdev)
{
}

static inline QDF_STATUS monitor_tx_capture_debugfs_init(struct dp_pdev *pdev)
{
	return QDF_STATUS_E_FAILURE;
}

static inline void monitor_peer_tx_capture_filter_check(struct dp_pdev *pdev,
							struct dp_peer *peer)
{
}

static inline
QDF_STATUS monitor_tx_add_to_comp_queue(struct dp_soc *soc,
					struct dp_tx_desc_s *desc,
					struct hal_tx_completion_status *ts,
					struct dp_peer *peer)
{
	return QDF_STATUS_E_FAILURE;
}

#endif

#if defined(WDI_EVENT_ENABLE) && !defined(REMOVE_PKT_LOG)
static inline bool monitor_ppdu_stats_ind_handler(struct htt_soc *soc,
						  uint32_t *msg_word,
						  qdf_nbuf_t htt_t2h_msg)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = soc->dp_soc->monitor_soc;

	if (!mon_soc) {
		qdf_err("monitor soc is NULL");
		return true;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_ppdu_stats_ind_handler) {
		qdf_err("callback not registered");
		return true;
	}

	return monitor_ops->mon_ppdu_stats_ind_handler(soc, msg_word,
						       htt_t2h_msg);
}
#else
static inline bool monitor_ppdu_stats_ind_handler(struct htt_soc *soc,
						  uint32_t *msg_word,
						  qdf_nbuf_t htt_t2h_msg)
{
	return true;
}
#endif

static inline QDF_STATUS monitor_htt_ppdu_stats_attach(struct dp_pdev *pdev)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		qdf_err("monitor soc is NULL");
		return QDF_STATUS_SUCCESS;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_htt_ppdu_stats_attach) {
		qdf_err("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_htt_ppdu_stats_attach(pdev);
}

static inline void monitor_htt_ppdu_stats_detach(struct dp_pdev *pdev)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		qdf_err("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_htt_ppdu_stats_detach) {
		qdf_err("callback not registered");
		return;
	}

	return monitor_ops->mon_htt_ppdu_stats_detach(pdev);
}

static inline void monitor_print_pdev_rx_mon_stats(struct dp_pdev *pdev)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		qdf_err("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_print_pdev_rx_mon_stats) {
		qdf_err("callback not registered");
		return;
	}

	return monitor_ops->mon_print_pdev_rx_mon_stats(pdev);
}

#ifdef WLAN_TX_PKT_CAPTURE_ENH
static inline void monitor_print_pdev_tx_capture_stats(struct dp_pdev *pdev)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		qdf_err("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_print_pdev_tx_capture_stats) {
		qdf_err("callback not registered");
		return;
	}

	return monitor_ops->mon_print_pdev_tx_capture_stats(pdev);
}

static inline QDF_STATUS monitor_config_enh_tx_capture(struct dp_pdev *pdev,
						       uint32_t val)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		qdf_err("monitor soc is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_config_enh_tx_capture) {
		qdf_err("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_config_enh_tx_capture(pdev, val);
}
#else
static inline void monitor_print_pdev_tx_capture_stats(struct dp_pdev *pdev)
{
}

static inline QDF_STATUS monitor_config_enh_tx_capture(struct dp_pdev *pdev,
						       uint32_t val)
{
	return QDF_STATUS_E_INVAL;
}
#endif

#ifdef WLAN_RX_PKT_CAPTURE_ENH
static inline QDF_STATUS monitor_config_enh_rx_capture(struct dp_pdev *pdev,
						       uint32_t val)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		qdf_err("monitor soc is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_config_enh_rx_capture) {
		qdf_err("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_config_enh_rx_capture(pdev, val);
}
#else
static inline QDF_STATUS monitor_config_enh_rx_capture(struct dp_pdev *pdev,
						       uint32_t val)
{
	return QDF_STATUS_E_INVAL;
}
#endif

static inline QDF_STATUS monitor_set_bpr_enable(struct dp_pdev *pdev,
						uint32_t val)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		qdf_err("monitor soc is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_set_bpr_enable) {
		qdf_err("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_set_bpr_enable(pdev, val);
}

#ifdef ATH_SUPPORT_NAC
static inline int monitor_set_filter_neigh_peers(struct dp_pdev *pdev, bool val)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		qdf_err("monitor soc is NULL");
		return 0;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_set_filter_neigh_peers) {
		qdf_err("callback not registered");
		return 0;
	}

	return monitor_ops->mon_set_filter_neigh_peers(pdev, val);
}
#else
static inline int monitor_set_filter_neigh_peers(struct dp_pdev *pdev, bool val)
{
	return 0;
}
#endif

#ifdef WLAN_ATF_ENABLE
static inline
void monitor_set_atf_stats_enable(struct dp_pdev *pdev, bool value)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		qdf_err("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_set_atf_stats_enable) {
		qdf_err("callback not registered");
		return;
	}

	return monitor_ops->mon_set_atf_stats_enable(pdev, value);
}
#else
static inline
void monitor_set_atf_stats_enable(struct dp_pdev *pdev, bool value)
{
}
#endif

static inline
void monitor_set_bsscolor(struct dp_pdev *pdev, uint8_t bsscolor)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		qdf_err("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_set_bsscolor) {
		qdf_err("callback not registered");
		return;
	}

	return monitor_ops->mon_set_bsscolor(pdev, bsscolor);
}

static inline
bool monitor_pdev_get_filter_mcast_data(struct cdp_pdev *pdev_handle)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		qdf_err("monitor soc is NULL");
		return false;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_pdev_get_filter_mcast_data) {
		qdf_err("callback not registered");
		return false;
	}

	return monitor_ops->mon_pdev_get_filter_mcast_data(pdev_handle);
}

static inline
bool monitor_pdev_get_filter_non_data(struct cdp_pdev *pdev_handle)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		qdf_err("monitor soc is NULL");
		return false;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_pdev_get_filter_non_data) {
		qdf_err("callback not registered");
		return false;
	}

	return monitor_ops->mon_pdev_get_filter_non_data(pdev_handle);
}

static inline
bool monitor_pdev_get_filter_ucast_data(struct cdp_pdev *pdev_handle)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		qdf_err("monitor soc is NULL");
		return false;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_pdev_get_filter_ucast_data) {
		qdf_err("callback not registered");
		return false;
	}

	return monitor_ops->mon_pdev_get_filter_ucast_data(pdev_handle);
}

#ifdef WDI_EVENT_ENABLE
static inline
int monitor_set_pktlog_wifi3(struct dp_pdev *pdev, uint32_t event, bool enable)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		qdf_err("monitor soc is NULL");
		return 0;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_set_pktlog_wifi3) {
		qdf_err("callback not registered");
		return 0;
	}

	return monitor_ops->mon_set_pktlog_wifi3(pdev, event, enable);
}
#else
static inline int dp_set_pktlog_wifi3(struct dp_pdev *pdev, uint32_t event,
				      bool enable)
{
	return 0;
}
#endif

#if defined(DP_CON_MON) && !defined(REMOVE_PKT_LOG)
static inline void monitor_pktlogmod_exit(struct dp_pdev *pdev)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		qdf_err("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_pktlogmod_exit) {
		qdf_err("callback not registered");
		return;
	}

	return monitor_ops->mon_pktlogmod_exit(pdev);
}
#else
static inline void monitor_pktlogmod_exit(struct dp_pdev *pdev) {}
#endif

static inline
void monitor_vdev_set_monitor_mode_buf_rings(struct dp_pdev *pdev)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		qdf_err("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_vdev_set_monitor_mode_buf_rings) {
		qdf_err("callback not registered");
		return;
	}

	return monitor_ops->mon_vdev_set_monitor_mode_buf_rings(pdev);
}

static inline
void monitor_neighbour_peers_detach(struct dp_pdev *pdev)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		qdf_err("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_neighbour_peers_detach) {
		qdf_err("callback not registered");
		return;
	}

	return monitor_ops->mon_neighbour_peers_detach(pdev);
}

#ifdef FEATURE_NAC_RSSI
static inline QDF_STATUS monitor_filter_neighbour_peer(struct dp_pdev *pdev,
						       uint8_t *rx_pkt_hdr)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		qdf_err("monitor soc is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_filter_neighbour_peer) {
		qdf_err("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_filter_neighbour_peer(pdev, rx_pkt_hdr);
}
#endif
