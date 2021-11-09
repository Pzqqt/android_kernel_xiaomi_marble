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

#ifndef _DP_MON_H_
#define _DP_MON_H_

#include "qdf_trace.h"
#include "dp_internal.h"
#include "dp_types.h"
#include "dp_htt.h"

#ifdef WLAN_TX_PKT_CAPTURE_ENH
#include "dp_tx_capture.h"
#endif

#define DP_INTR_POLL_TIMER_MS	5

#define MON_VDEV_TIMER_INIT 0x1
#define MON_VDEV_TIMER_RUNNING 0x2

/* Budget to reap monitor status ring */
#define DP_MON_REAP_BUDGET 1024
#define MON_BUF_MIN_ENTRIES 64

#define RNG_ERR		"SRNG setup failed for"
#define dp_mon_info(params...) \
	__QDF_TRACE_FL(QDF_TRACE_LEVEL_INFO_HIGH, QDF_MODULE_ID_MON, ## params)
#define dp_mon_err(params...) QDF_TRACE_ERROR(QDF_MODULE_ID_MON, params)
#define dp_mon_debug(params...) QDF_TRACE_DEBUG(QDF_MODULE_ID_MON, params)
#define dp_mon_warn(params...) QDF_TRACE_WARN(QDF_MODULE_ID_MON, params)

#ifndef WLAN_TX_PKT_CAPTURE_ENH
struct dp_pdev_tx_capture {
};

struct dp_peer_tx_capture {
};
#endif

/*
 * dp_mon_soc_attach() - DP monitor soc attach
 * @soc: Datapath SOC handle
 *
 * Return: QDF_STATUS_SUCCESS: Success
 *         QDF_STATUS_E_NOMEM: Error
 */
QDF_STATUS dp_mon_soc_attach(struct dp_soc *soc);

/*
 * dp_mon_soc_detach() - DP monitor soc detach
 * @soc: Datapath SOC handle
 *
 * Return: QDF_STATUS_SUCCESS: Success
 *         QDF_STATUS_E_FAILURE: Error
 */
QDF_STATUS dp_mon_soc_detach(struct dp_soc *soc);

/*
 * dp_mon_soc_cfg_init() - DP monitor soc config init
 * @soc: Datapath SOC handle
 *
 * Return: QDF_STATUS_SUCCESS: Success
 *         QDF_STATUS_E_FAILURE: Error
 */
QDF_STATUS dp_mon_soc_cfg_init(struct dp_soc *soc);

/*
 * dp_mon_pdev_attach() - DP monitor pdev attach
 * @pdev: Datapath pdev handle
 *
 * Return: QDF_STATUS_SUCCESS: Success
 *         QDF_STATUS_E_NOMEM: Error
 */
QDF_STATUS dp_mon_pdev_attach(struct dp_pdev *pdev);

/*
 * dp_mon_pdev_detach() - DP monitor pdev detach
 * @pdev: Datapath pdev handle
 *
 * Return: QDF_STATUS_SUCCESS: Success
 *         QDF_STATUS_E_FAILURE: Error
 */
QDF_STATUS dp_mon_pdev_detach(struct dp_pdev *pdev);

/*
 * dp_mon_pdev_init() - DP monitor pdev init
 * @pdev: Datapath pdev handle
 *
 * Return: QDF_STATUS_SUCCESS: Success
 *         QDF_STATUS_E_FAILURE: Error
 */
QDF_STATUS dp_mon_pdev_init(struct dp_pdev *pdev);

/*
 * dp_mon_pdev_deinit() - DP monitor pdev deinit
 * @pdev: Datapath pdev handle
 *
 * Return: QDF_STATUS_SUCCESS: Success
 *         QDF_STATUS_E_FAILURE: Error
 */
QDF_STATUS dp_mon_pdev_deinit(struct dp_pdev *pdev);

/*
 * dp_mon_vdev_attach() - DP monitor vdev attach
 * @vdev: Datapath vdev handle
 *
 * Return: QDF_STATUS_SUCCESS: Success
 *         QDF_STATUS_E_NOMEM: Error
 */
QDF_STATUS dp_mon_vdev_attach(struct dp_vdev *vdev);

/*
 * dp_mon_vdev_detach() - DP monitor vdev detach
 * @vdev: Datapath vdev handle
 *
 * Return: QDF_STATUS_SUCCESS: Success
 *         QDF_STATUS_E_FAILURE: Error
 */
QDF_STATUS dp_mon_vdev_detach(struct dp_vdev *vdev);

/*
 * dp_mon_peer_attach() - DP monitor peer attach
 * @peer: Datapath peer handle
 *
 * Return: QDF_STATUS_SUCCESS: Success
 *         QDF_STATUS_E_NOMEM: Error
 */
#if defined(WLAN_TX_PKT_CAPTURE_ENH) || defined(FEATURE_PERPKT_INFO)
QDF_STATUS dp_mon_peer_attach(struct dp_peer *peer);
#else
static inline
QDF_STATUS dp_mon_peer_attach(struct dp_peer *peer)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/*
 * dp_mon_peer_detach() - DP monitor peer detach
 * @peer: Datapath peer handle
 *
 * Return: QDF_STATUS_SUCCESS: Success
 *         QDF_STATUS_E_FAILURE: Error
 */
QDF_STATUS dp_mon_peer_detach(struct dp_peer *peer);

/*
 * dp_mon_cdp_ops_register() - Register monitor cdp ops
 * @soc: Datapath soc handle
 *
 */
void dp_mon_cdp_ops_register(struct dp_soc *soc);

/*
 * dp_mon_cdp_ops_deregister() - deregister monitor cdp ops
 * @soc: Datapath soc handle
 *
 */
void dp_mon_cdp_ops_deregister(struct dp_soc *soc);

/*
 * dp_mon_ops_register() - Register monitor ops
 * @soc: Datapath soc handle
 *
 */
void dp_mon_ops_register(struct dp_soc *soc);

/*
 * dp_mon_htt_srng_setup() - DP mon htt srng setup
 * @soc: Datapath soc handle
 * @pdev: Datapath pdev handle
 * @mac_id: mac id
 * @mac_for_pdev: mac id mapped pdev
 *
 * Return: QDF_STATUS_SUCCESS: Success
 *         QDF_STATUS_E_FAILURE: Error
 */
QDF_STATUS dp_mon_htt_srng_setup(struct dp_soc *soc,
				 struct dp_pdev *pdev,
				 int mac_id,
				 int mac_for_pdev);

/*
 * dp_config_debug_sniffer()- API to enable/disable debug sniffer
 * @pdev: DP_PDEV handle
 * @val: user provided value
 *
 * Return: 0 for success. nonzero for failure.
 */
#if defined(QCA_MCOPY_SUPPORT) || defined(QCA_TX_CAPTURE_SUPPORT)
QDF_STATUS
dp_config_debug_sniffer(struct dp_pdev *pdev, int val);
#else
static inline QDF_STATUS
dp_config_debug_sniffer(struct dp_pdev *pdev, int val) {
	return QDF_STATUS_E_INVAL;
}
#endif /* QCA_MCOPY_SUPPORT || QCA_TX_CAPTURE_SUPPORT */

/*
 * dp_htt_ppdu_stats_attach() - attach resources for HTT PPDU stats processing
 * @pdev: Datapath PDEV handle
 *
 * Return: QDF_STATUS_SUCCESS: Success
 *         QDF_STATUS_E_NOMEM: Error
 */
QDF_STATUS dp_htt_ppdu_stats_attach(struct dp_pdev *pdev);

/*
 * dp_htt_ppdu_stats_detach() - detach stats resources
 * @pdev: Datapath PDEV handle
 *
 * Return: void
 */
void dp_htt_ppdu_stats_detach(struct dp_pdev *pdev);

/*
 *dp_set_bpr_enable() - API to enable/disable bpr feature
 *@pdev_handle: DP_PDEV handle.
 *@val: Provided value.
 *
 *Return: 0 for success. nonzero for failure.
 */
#ifdef QCA_SUPPORT_BPR
QDF_STATUS
dp_set_bpr_enable(struct dp_pdev *pdev, int val);
#endif

#ifdef ATH_SUPPORT_NAC
int dp_set_filter_neigh_peers(struct dp_pdev *pdev,
				     bool val);
#endif /* ATH_SUPPORT_NAC */

#ifdef WLAN_ATF_ENABLE
void dp_set_atf_stats_enable(struct dp_pdev *pdev, bool value);
#endif

/**
 * dp_set_bsscolor() - sets bsscolor for tx capture
 * @pdev: Datapath PDEV handle
 * @bsscolor: new bsscolor
 */
void
dp_mon_set_bsscolor(struct dp_pdev *pdev, uint8_t bsscolor);

/**
 * dp_pdev_get_filter_ucast_data() - get DP PDEV monitor ucast filter
 * @soc : data path soc handle
 * @pdev_id : pdev_id
 * Return: true on ucast filter flag set
 */
bool dp_pdev_get_filter_ucast_data(struct cdp_pdev *pdev_handle);

/**
 * dp_pdev_get_filter_mcast_data() - get DP PDEV monitor mcast filter
 * @pdev_handle: Datapath PDEV handle
 * Return: true on mcast filter flag set
 */
bool dp_pdev_get_filter_mcast_data(struct cdp_pdev *pdev_handle);

/**
 * dp_pdev_get_filter_non_data() - get DP PDEV monitor non_data filter
 * @pdev_handle: Datapath PDEV handle
 * Return: true on non data filter flag set
 */
bool dp_pdev_get_filter_non_data(struct cdp_pdev *pdev_handle);

/*
 * dp_set_pktlog_wifi3() - attach txrx vdev
 * @pdev: Datapath PDEV handle
 * @event: which event's notifications are being subscribed to
 * @enable: WDI event subscribe or not. (True or False)
 *
 * Return: Success, NULL on failure
 */
#ifdef WDI_EVENT_ENABLE
int dp_set_pktlog_wifi3(struct dp_pdev *pdev, uint32_t event,
			bool enable);
#endif

/* MCL specific functions */
#if defined(DP_CON_MON) && !defined(REMOVE_PKT_LOG)
/**
 * dp_pktlogmod_exit() - API to cleanup pktlog info
 * @pdev: Pdev handle
 *
 * Return: none
 */
void dp_pktlogmod_exit(struct dp_pdev *pdev);
#else
static inline
void dp_pktlogmod_exit(struct dp_pdev *handle)
{
}
#endif

#ifdef QCA_MONITOR_PKT_SUPPORT
/**
 * dp_vdev_set_monitor_mode_buf_rings () - set monitor mode buf rings
 *
 * Allocate SW descriptor pool, buffers, link descriptor memory
 * Initialize monitor related SRNGs
 *
 * @pdev: DP pdev object
 *
 * Return: void
 */
void dp_vdev_set_monitor_mode_buf_rings(struct dp_pdev *pdev);

/**
 * dp_vdev_set_monitor_mode_rings () - set monitor mode rings
 *
 * Allocate SW descriptor pool, buffers, link descriptor memory
 * Initialize monitor related SRNGs
 *
 * @pdev: DP pdev object
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_vdev_set_monitor_mode_rings(struct dp_pdev *pdev,
					  uint8_t delayed_replenish);

#else
static inline void
dp_vdev_set_monitor_mode_buf_rings(struct dp_pdev *pdev)
{
}

static inline QDF_STATUS
dp_vdev_set_monitor_mode_rings(struct dp_pdev *pdev,
			       uint8_t delayed_replenish)
{
	return QDF_STATUS_SUCCESS;
}
#endif

struct dp_mon_ops {
	QDF_STATUS (*mon_soc_cfg_init)(struct dp_soc *soc);
	QDF_STATUS (*mon_soc_attach)(struct dp_soc *soc);
	QDF_STATUS (*mon_soc_detach)(struct dp_soc *soc);
	QDF_STATUS (*mon_soc_init)(struct dp_soc *soc);
	QDF_STATUS (*mon_soc_deinit)(struct dp_soc *soc);
	QDF_STATUS (*mon_pdev_alloc)(struct dp_pdev *pdev);
	void (*mon_pdev_free)(struct dp_pdev *pdev);
	QDF_STATUS (*mon_pdev_attach)(struct dp_pdev *pdev);
	QDF_STATUS (*mon_pdev_detach)(struct dp_pdev *pdev);
	QDF_STATUS (*mon_pdev_init)(struct dp_pdev *pdev);
	QDF_STATUS (*mon_pdev_deinit)(struct dp_pdev *pdev);
	QDF_STATUS (*mon_vdev_attach)(struct dp_vdev *vdev);
	QDF_STATUS (*mon_vdev_detach)(struct dp_vdev *vdev);
	QDF_STATUS (*mon_peer_attach)(struct dp_peer *peer);
	QDF_STATUS (*mon_peer_detach)(struct dp_peer *peer);
	QDF_STATUS (*mon_config_debug_sniffer)(struct dp_pdev *pdev, int val);
	void (*mon_flush_rings)(struct dp_soc *soc);
#if !defined(DISABLE_MON_CONFIG)
	QDF_STATUS (*mon_pdev_htt_srng_setup)(struct dp_soc *soc,
					      struct dp_pdev *pdev,
					      int mac_id,
					      int mac_for_pdev);
	QDF_STATUS (*mon_soc_htt_srng_setup)(struct dp_soc *soc);
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
	uint32_t (*mon_rx_process)(struct dp_soc *soc,
				   struct dp_intr *int_ctx,
				   uint32_t mac_id,
				   uint32_t quota);
	uint32_t (*mon_tx_process)(struct dp_soc *soc,
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
	QDF_STATUS (*mon_update_msdu_to_list)(struct dp_soc *soc,
					      struct dp_pdev *pdev,
					      struct dp_peer *peer,
					      struct hal_tx_completion_status *ts,
					      qdf_nbuf_t netbuf);
#endif
#if defined(WDI_EVENT_ENABLE) &&\
	(defined(QCA_ENHANCED_STATS_SUPPORT) || !defined(REMOVE_PKT_LOG))
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
#ifdef QCA_SUPPORT_BPR
	QDF_STATUS (*mon_set_bpr_enable)(struct dp_pdev *pdev, int val);
#endif
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
	QDF_STATUS (*mon_vdev_set_monitor_mode_rings)(struct dp_pdev *pdev,
						      uint8_t delayed_replenish);
	void (*mon_neighbour_peers_detach)(struct dp_pdev *pdev);
#ifdef FEATURE_NAC_RSSI
	QDF_STATUS (*mon_filter_neighbour_peer)(struct dp_pdev *pdev,
						uint8_t *rx_pkt_hdr);
#endif
	void (*mon_vdev_timer_init)(struct dp_soc *soc);
	void (*mon_vdev_timer_start)(struct dp_soc *soc);
	bool (*mon_vdev_timer_stop)(struct dp_soc *soc);
	void (*mon_vdev_timer_deinit)(struct dp_soc *soc);
	void (*mon_reap_timer_init)(struct dp_soc *soc);
	void (*mon_reap_timer_start)(struct dp_soc *soc);
	bool (*mon_reap_timer_stop)(struct dp_soc *soc);
	void (*mon_reap_timer_deinit)(struct dp_soc *soc);
#ifdef QCA_MCOPY_SUPPORT
	QDF_STATUS (*mon_mcopy_check_deliver)(struct dp_pdev *pdev,
					      uint16_t peer_id,
					      uint32_t ppdu_id,
					      uint8_t first_msdu);
#endif
	void (*mon_neighbour_peer_add_ast)(struct dp_pdev *pdev,
					   struct dp_peer *ta_peer,
					   uint8_t *mac_addr,
					   qdf_nbuf_t nbuf,
					   uint32_t flags);
#ifdef QCA_ENHANCED_STATS_SUPPORT
	void (*mon_filter_setup_enhanced_stats)(struct dp_pdev *pdev);
	void (*mon_filter_reset_enhanced_stats)(struct dp_pdev *pdev);
#endif
#ifdef QCA_MCOPY_SUPPORT
	void (*mon_filter_setup_mcopy_mode)(struct dp_pdev *pdev);
	void (*mon_filter_reset_mcopy_mode)(struct dp_pdev *pdev);
#endif
#if defined(ATH_SUPPORT_NAC_RSSI) || defined(ATH_SUPPORT_NAC)
	void (*mon_filter_setup_smart_monitor)(struct dp_pdev *pdev);
	void (*mon_filter_reset_smart_monitor)(struct dp_pdev *pdev);
#endif
#ifdef WLAN_RX_PKT_CAPTURE_ENH
	void (*mon_filter_setup_rx_enh_capture)(struct dp_pdev *pdev);
	void (*mon_filter_reset_rx_enh_capture)(struct dp_pdev *pdev);
#endif
	void (*mon_filter_setup_mon_mode)(struct dp_pdev *pdev);
	void (*mon_filter_reset_mon_mode)(struct dp_pdev *pdev);
#ifdef WDI_EVENT_ENABLE
	void (*mon_filter_setup_rx_pkt_log_full)(struct dp_pdev *pdev);
	void (*mon_filter_reset_rx_pkt_log_full)(struct dp_pdev *pdev);
	void (*mon_filter_setup_rx_pkt_log_lite)(struct dp_pdev *pdev);
	void (*mon_filter_reset_rx_pkt_log_lite)(struct dp_pdev *pdev);
	void (*mon_filter_setup_rx_pkt_log_cbf)(struct dp_pdev *pdev);
	void (*mon_filter_reset_rx_pkt_log_cbf)(struct dp_pdev *pdev);
#endif
	QDF_STATUS (*mon_filter_update)(struct dp_pdev *pdev);

	QDF_STATUS (*mon_rings_alloc)(struct dp_pdev *pdev);
	void (*mon_rings_free)(struct dp_pdev *pdev);
	QDF_STATUS (*mon_rings_init)(struct dp_pdev *pdev);
	void (*mon_rings_deinit)(struct dp_pdev *pdev);

	QDF_STATUS (*rx_mon_buffers_alloc)(struct dp_pdev *pdev);
	void (*rx_mon_buffers_free)(struct dp_pdev *pdev);
	void (*rx_mon_desc_pool_init)(struct dp_pdev *pdev);
	void (*rx_mon_desc_pool_deinit)(struct dp_pdev *pdev);
	QDF_STATUS (*rx_mon_desc_pool_alloc)(struct dp_pdev *pdev);
	void (*rx_mon_desc_pool_free)(struct dp_pdev *pdev);
	void (*tx_mon_desc_pool_init)(struct dp_pdev *pdev);
	void (*tx_mon_desc_pool_deinit)(struct dp_pdev *pdev);
	QDF_STATUS (*tx_mon_desc_pool_alloc)(struct dp_pdev *pdev);
	void (*tx_mon_desc_pool_free)(struct dp_pdev *pdev);
	void (*rx_packet_length_set)(uint32_t *msg_word,
				     struct htt_rx_ring_tlv_filter *tlv_filter);
	void (*rx_wmask_subscribe)(uint32_t *msg_word,
				   struct htt_rx_ring_tlv_filter *tlv_filter);
	void (*rx_enable_mpdu_logging)(uint32_t *msg_word,
				       struct htt_rx_ring_tlv_filter *tlv_filter);
};

struct dp_mon_soc {
	/* Holds all monitor related fields extracted from dp_soc */
	/* Holds pointer to monitor ops */
	/* monitor link descriptor pages */
	struct qdf_mem_multi_page_t mon_link_desc_pages[MAX_NUM_LMAC_HW];

	/* total link descriptors for monitor mode for each radio */
	uint32_t total_mon_link_descs[MAX_NUM_LMAC_HW];

	 /* Monitor Link descriptor memory banks */
	struct link_desc_bank
		mon_link_desc_banks[MAX_NUM_LMAC_HW][MAX_MON_LINK_DESC_BANKS];
	uint32_t num_mon_link_desc_banks[MAX_NUM_LMAC_HW];
	/* Smart monitor capability for HKv2 */
	uint8_t hw_nac_monitor_support;

	/* Full monitor mode support */
	bool full_mon_mode;

	/*interrupt timer*/
	qdf_timer_t mon_reap_timer;
	uint8_t reap_timer_init;

	qdf_timer_t mon_vdev_timer;
	uint8_t mon_vdev_timer_state;

	struct dp_mon_ops *mon_ops;
	bool monitor_mode_v2;
};

struct  dp_mon_pdev {
	/* monitor */
	bool monitor_configured;

	struct dp_mon_filter **filter;	/* Monitor Filter pointer */

	/* advance filter mode and type*/
	uint8_t mon_filter_mode;
	uint16_t fp_mgmt_filter;
	uint16_t fp_ctrl_filter;
	uint16_t fp_data_filter;
	uint16_t mo_mgmt_filter;
	uint16_t mo_ctrl_filter;
	uint16_t mo_data_filter;
	uint16_t md_data_filter;

#ifdef WLAN_TX_PKT_CAPTURE_ENH
	struct dp_pdev_tx_capture tx_capture;
	bool stop_tx_capture_work_q_timer;
#endif

	/* tx packet capture enhancement */
	enum cdp_tx_enh_capture_mode tx_capture_enabled;
	/* Stuck count on monitor destination ring MPDU process */
	uint32_t mon_dest_ring_stuck_cnt;
	/* monitor mode lock */
	qdf_spinlock_t mon_lock;

	/* Monitor mode operation channel */
	int mon_chan_num;

	/* Monitor mode operation frequency */
	qdf_freq_t mon_chan_freq;

	/* Monitor mode band */
	enum reg_wifi_band mon_chan_band;

	uint32_t mon_ppdu_status;
	/* monitor mode status/destination ring PPDU and MPDU count */
	struct cdp_pdev_mon_stats rx_mon_stats;
	/* Monitor mode interface and status storage */
	struct dp_vdev *mvdev;
	struct cdp_mon_status rx_mon_recv_status;
	/* to track duplicate link descriptor indications by HW for a WAR */
	uint64_t mon_last_linkdesc_paddr;
	/* to track duplicate buffer indications by HW for a WAR */
	uint32_t mon_last_buf_cookie;

#ifdef QCA_SUPPORT_FULL_MON
	/* List to maintain all MPDUs for a PPDU in monitor mode */
	TAILQ_HEAD(, dp_mon_mpdu) mon_mpdu_q;

	/* TODO: define per-user mpdu list
	 * struct dp_mon_mpdu_list mpdu_list[MAX_MU_USERS];
	 */
	struct hal_rx_mon_desc_info *mon_desc;
#endif
	/* Flag to hold on to monitor destination ring */
	bool hold_mon_dest_ring;

	/* Flag to inidicate monitor rings are initialized */
	uint8_t pdev_mon_init;
#ifndef REMOVE_PKT_LOG
	bool pkt_log_init;
	struct pktlog_dev_t *pl_dev; /* Pktlog pdev */
#endif /* #ifndef REMOVE_PKT_LOG */

	/* Smart Mesh */
	bool filter_neighbour_peers;

	/*flag to indicate neighbour_peers_list not empty */
	bool neighbour_peers_added;
	/* smart mesh mutex */
	qdf_spinlock_t neighbour_peer_mutex;
	/* Neighnour peer list */
	TAILQ_HEAD(, dp_neighbour_peer) neighbour_peers_list;
	/* Enhanced Stats is enabled */
	bool enhanced_stats_en;
	qdf_nbuf_queue_t rx_status_q;

	/* 128 bytes mpdu header queue per user for ppdu */
	qdf_nbuf_queue_t mpdu_q[MAX_MU_USERS];

	/* is this a mpdu header TLV and not msdu header TLV */
	bool is_mpdu_hdr[MAX_MU_USERS];

	/* per user 128 bytes msdu header list for MPDU */
	struct msdu_list msdu_list[MAX_MU_USERS];

	/* RX enhanced capture mode */
	uint8_t rx_enh_capture_mode;
	/* Rx per peer enhanced capture mode */
	bool rx_enh_capture_peer;
	struct dp_vdev *rx_enh_monitor_vdev;
	/* RX enhanced capture trailer enable/disable flag */
	bool is_rx_enh_capture_trailer_enabled;
#ifdef WLAN_RX_PKT_CAPTURE_ENH
	/* RX per MPDU/PPDU information */
	struct cdp_rx_indication_mpdu mpdu_ind;
#endif

	/* Packet log mode */
	uint8_t rx_pktlog_mode;
	/* Enable pktlog logging cbf */
	bool rx_pktlog_cbf;

	bool tx_sniffer_enable;
	/* mirror copy mode */
	enum m_copy_mode mcopy_mode;
	bool enable_reap_timer_non_pkt;
	bool bpr_enable;
	/* Pdev level flag to check peer based pktlog enabled or
	 * disabled
	 */
	uint8_t dp_peer_based_pktlog;

#ifdef WLAN_ATF_ENABLE
	/* ATF stats enable */
	bool dp_atf_stats_enable;
#endif

	/* Maintains first status buffer's paddr of a PPDU */
	uint64_t status_buf_addr;
	struct hal_rx_ppdu_info ppdu_info;

	/* ppdu_id of last received HTT TX stats */
	uint32_t last_ppdu_id;
	struct {
		uint8_t last_user;
		qdf_nbuf_t buf;
	} tx_ppdu_info;

	struct {
		uint32_t tx_ppdu_id;
		uint16_t tx_peer_id;
		uint32_t rx_ppdu_id;
	} m_copy_id;

	/* To check if PPDU Tx stats are enabled for Pktlog */
	bool pktlog_ppdu_stats;

#ifdef ATH_SUPPORT_NAC_RSSI
	bool nac_rssi_filtering;
#endif

	/* ppdu_stats lock for queue concurrency between cores*/
	qdf_spinlock_t ppdu_stats_lock;

	/* list of ppdu tlvs */
	TAILQ_HEAD(, ppdu_info) ppdu_info_list;
	TAILQ_HEAD(, ppdu_info) sched_comp_ppdu_list;

	uint32_t sched_comp_list_depth;
	uint16_t delivered_sched_cmdid;
	uint16_t last_sched_cmdid;
	uint32_t tlv_count;
	uint32_t list_depth;

	struct {
		qdf_nbuf_t last_nbuf; /*Ptr to mgmt last buf */
		uint8_t *mgmt_buf; /* Ptr to mgmt. payload in HTT ppdu stats */
		uint32_t mgmt_buf_len; /* Len of mgmt. payload in ppdu stats */
		uint32_t ppdu_id;
	} mgmtctrl_frm_info;
	/* Context of cal client timer */
	struct cdp_cal_client *cal_client_ctx;
	uint32_t *ppdu_tlv_buf; /* Buffer to hold HTT ppdu stats TLVs*/

	qdf_nbuf_t mcopy_status_nbuf;
	bool is_dp_mon_pdev_initialized;
	/* indicates if spcl vap is configured */
	bool scan_spcl_vap_configured;
#ifdef QCA_SUPPORT_SCAN_SPCL_VAP_STATS
	/* enable spcl vap stats reset on ch change */
	bool reset_scan_spcl_vap_stats_enable;
#endif
};

struct  dp_mon_vdev {
	/* callback to hand rx monitor 802.11 MPDU to the OS shim */
	ol_txrx_rx_mon_fp osif_rx_mon;
#ifdef QCA_SUPPORT_SCAN_SPCL_VAP_STATS
	struct cdp_scan_spcl_vap_stats *scan_spcl_vap_stats;
#endif
};

struct dp_mon_peer {
#ifdef WLAN_TX_PKT_CAPTURE_ENH
	struct dp_peer_tx_capture tx_capture;
#endif
#ifdef FEATURE_PERPKT_INFO
	/* delayed ba ppdu stats handling */
	struct cdp_delayed_tx_completion_ppdu_user delayed_ba_ppdu_stats;
	/* delayed ba flag */
	bool last_delayed_ba;
	/* delayed ba ppdu id */
	uint32_t last_delayed_ba_ppduid;
#endif
};

#if defined(QCA_TX_CAPTURE_SUPPORT) || defined(QCA_ENHANCED_STATS_SUPPORT)
void dp_deliver_mgmt_frm(struct dp_pdev *pdev, qdf_nbuf_t nbuf);
#else
static inline
void dp_deliver_mgmt_frm(struct dp_pdev *pdev, qdf_nbuf_t nbuf)
{
}
#endif

#if defined(WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG) ||\
	defined(WLAN_SUPPORT_RX_FLOW_TAG)
/**
 * dp_rx_mon_update_protocol_flow_tag() - Performs necessary checks for monitor
 *                                       mode and then tags appropriate packets
 * @soc: core txrx main context
 * @vdev: pdev on which packet is received
 * @msdu: QDF packet buffer on which the protocol tag should be set
 * @rx_desc: base address where the RX TLVs start
 * Return: void
 */
void dp_rx_mon_update_protocol_flow_tag(struct dp_soc *soc,
					struct dp_pdev *dp_pdev,
					qdf_nbuf_t msdu, void *rx_desc);
#endif /* WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG || WLAN_SUPPORT_RX_FLOW_TAG */

#if !defined(WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG) &&\
	!defined(WLAN_SUPPORT_RX_FLOW_TAG)
/**
 * dp_rx_mon_update_protocol_flow_tag() - Performs necessary checks for monitor
 *                                       mode and then tags appropriate packets
 * @soc: core txrx main context
 * @vdev: pdev on which packet is received
 * @msdu: QDF packet buffer on which the protocol tag should be set
 * @rx_desc: base address where the RX TLVs start
 * Return: void
 */
static inline
void dp_rx_mon_update_protocol_flow_tag(struct dp_soc *soc,
					struct dp_pdev *dp_pdev,
					qdf_nbuf_t msdu, void *rx_desc)
{
}
#endif /* WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG || WLAN_SUPPORT_RX_FLOW_TAG */

#ifndef WLAN_TX_PKT_CAPTURE_ENH
static inline
QDF_STATUS dp_peer_set_tx_capture_enabled(struct dp_pdev *pdev,
					  struct dp_peer *peer_handle,
					  uint8_t value, uint8_t *peer_mac)
{
	return QDF_STATUS_SUCCESS;
}

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

/**
 * dp_update_msdu_to_list(): Function to queue msdu from wbm
 * @pdev: dp_pdev
 * @peer: dp_peer
 * @ts: hal tx completion status
 * @netbuf: msdu
 *
 * return: status
 */
static inline
QDF_STATUS dp_update_msdu_to_list(struct dp_soc *soc,
				  struct dp_pdev *pdev,
				  struct dp_peer *peer,
				  struct hal_tx_completion_status *ts,
				  qdf_nbuf_t netbuf)
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

int
dp_htt_get_ppdu_sniffer_ampdu_tlv_bitmap(uint32_t bitmap);

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

/**
 * dp_rx_cookie_2_mon_link_desc_va() - Converts cookie to a virtual address of
 *				   the MSDU Link Descriptor
 * @pdev: core txrx pdev context
 * @buf_info: buf_info includes cookie that used to lookup virtual address of
 * link descriptor. Normally this is just an index into a per pdev array.
 *
 * This is the VA of the link descriptor in monitor mode destination ring,
 * that HAL layer later uses to retrieve the list of MSDU's for a given MPDU.
 *
 * Return: void *: Virtual Address of the Rx descriptor
 */
static inline
void *dp_rx_cookie_2_mon_link_desc_va(struct dp_pdev *pdev,
				      struct hal_buf_info *buf_info,
				      int mac_id)
{
	void *link_desc_va;
	struct qdf_mem_multi_page_t *pages;
	uint16_t page_id = LINK_DESC_COOKIE_PAGE_ID(buf_info->sw_cookie);
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc)
		return NULL;

	pages = &mon_soc->mon_link_desc_pages[mac_id];
	if (!pages)
		return NULL;

	if (qdf_unlikely(page_id >= pages->num_pages))
		return NULL;

	link_desc_va = pages->dma_pages[page_id].page_v_addr_start +
		(buf_info->paddr - pages->dma_pages[page_id].page_p_addr);

	return link_desc_va;
}

/**
 * dp_soc_is_full_mon_enable () - Return if full monitor mode is enabled
 * @soc: DP soc handle
 *
 * Return: Full monitor mode status
 */
static inline bool dp_soc_is_full_mon_enable(struct dp_pdev *pdev)
{
	return (pdev->soc->monitor_soc->full_mon_mode &&
		pdev->monitor_pdev->monitor_configured) ? true : false;
}

/*
 * dp_mon_is_enable_reap_timer_non_pkt() - check if mon reap timer is
 * enabled by non-pkt log or not
 * @pdev: point to dp pdev
 *
 * Return: true if mon reap timer is enabled by non-pkt log
 */
static inline bool dp_mon_is_enable_reap_timer_non_pkt(struct dp_pdev *pdev)
{
	if (qdf_unlikely(!pdev || !pdev->monitor_pdev))
		return false;

	return pdev->monitor_pdev->enable_reap_timer_non_pkt;
}

/*
 * dp_monitor_is_enable_mcopy_mode() - check if mcopy mode is enabled
 * @pdev: point to dp pdev
 *
 * Return: true if mcopy mode is enabled
 */
static inline bool dp_monitor_is_enable_mcopy_mode(struct dp_pdev *pdev)
{
	if (qdf_unlikely(!pdev || !pdev->monitor_pdev))
		return false;

	return pdev->monitor_pdev->mcopy_mode;
}

/*
 * dp_monitor_is_enable_tx_sniffer() - check if tx sniffer is enabled
 * @pdev: point to dp pdev
 *
 * Return: true if tx sniffer is enabled
 */
static inline bool dp_monitor_is_enable_tx_sniffer(struct dp_pdev *pdev)
{
	if (qdf_unlikely(!pdev || !pdev->monitor_pdev))
		return false;

	return pdev->monitor_pdev->tx_sniffer_enable;
}

/*
 * dp_monitor_is_set_monitor_configured() - check if monitor configured is set
 * @pdev: point to dp pdev
 *
 * Return: true if monitor configured is set
 */
static inline bool dp_monitor_is_configured(struct dp_pdev *pdev)
{
	if (qdf_unlikely(!pdev || !pdev->monitor_pdev))
		return false;

	return pdev->monitor_pdev->monitor_configured;
}

/*
 * dp_monitor_check_com_info_ppdu_id() - check if msdu ppdu_id matches with
 * com info ppdu_id
 * @pdev: point to dp pdev
 * @rx_desc: point to rx_desc
 *
 * Return: success if ppdu_id matches
 */
static inline QDF_STATUS dp_monitor_check_com_info_ppdu_id(struct dp_pdev *pdev,
							   void *rx_desc)
{
	struct cdp_mon_status *rs;
	struct dp_mon_pdev *mon_pdev;
	uint32_t msdu_ppdu_id = 0;

	if (qdf_unlikely(!pdev || !pdev->monitor_pdev))
		return QDF_STATUS_E_FAILURE;

	mon_pdev = pdev->monitor_pdev;
	if (qdf_likely(1 != mon_pdev->ppdu_info.rx_status.rxpcu_filter_pass))
		return QDF_STATUS_E_FAILURE;

	rs = &pdev->monitor_pdev->rx_mon_recv_status;
	if (!rs || rs->cdp_rs_rxdma_err)
		return QDF_STATUS_E_FAILURE;

	msdu_ppdu_id = hal_rx_get_ppdu_id(pdev->soc->hal_soc, rx_desc);
	if (msdu_ppdu_id != mon_pdev->ppdu_info.com_info.ppdu_id) {
		QDF_TRACE(QDF_MODULE_ID_DP,
			  QDF_TRACE_LEVEL_ERROR,
			  "msdu_ppdu_id=%x,com_info.ppdu_id=%x",
			  msdu_ppdu_id,
			  mon_pdev->ppdu_info.com_info.ppdu_id);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/*
 * dp_monitor_get_rx_status() - get rx status
 * @pdev: point to dp pdev
 *
 * Return: return rx status pointer
 */
static inline struct mon_rx_status*
dp_monitor_get_rx_status(struct dp_pdev *pdev)
{
	if (qdf_unlikely(!pdev || !pdev->monitor_pdev))
		return NULL;

	return &pdev->monitor_pdev->ppdu_info.rx_status;
}

/*
 * dp_monitor_is_chan_band_known() - check if monitor chan band known
 * @pdev: point to dp pdev
 *
 * Return: true if chan band known
 */
static inline bool dp_monitor_is_chan_band_known(struct dp_pdev *pdev)
{
	if (qdf_unlikely(!pdev || !pdev->monitor_pdev))
		return false;

	if (pdev->monitor_pdev->mon_chan_band != REG_BAND_UNKNOWN)
		return true;

	return false;
}

/*
 * dp_monitor_get_chan_band() - get chan band
 * @pdev: point to dp pdev
 *
 * Return: wifi channel band
 */
static inline enum reg_wifi_band
dp_monitor_get_chan_band(struct dp_pdev *pdev)
{
	return pdev->monitor_pdev->mon_chan_band;
}

/*
 * dp_monitor_print_tx_stats() - print tx stats from monitor pdev
 * @pdev: point to dp pdev
 *
 */
static inline void dp_monitor_print_tx_stats(struct dp_pdev *pdev)
{
	if (qdf_unlikely(!pdev || !pdev->monitor_pdev))
		return;

	DP_PRINT_STATS("ppdu info schedule completion list depth: %d",
		       pdev->monitor_pdev->sched_comp_list_depth);
	DP_PRINT_STATS("delivered sched cmdid: %d",
		       pdev->monitor_pdev->delivered_sched_cmdid);
	DP_PRINT_STATS("cur sched cmdid: %d",
		       pdev->monitor_pdev->last_sched_cmdid);
	DP_PRINT_STATS("ppdu info list depth: %d",
		       pdev->monitor_pdev->list_depth);
}

/*
 * dp_monitor_set_chan_num() - set channel number
 * @pdev: point to dp pdev
 * @chan_num: channel number
 *
 */
static inline void dp_monitor_set_chan_num(struct dp_pdev *pdev, int chan_num)
{
	if (qdf_unlikely(!pdev || !pdev->monitor_pdev))
		return;

	pdev->monitor_pdev->mon_chan_num = chan_num;
}

/*
 * dp_monitor_set_chan_freq() - set channel frequency
 * @pdev: point to dp pdev
 * @chan_freq: channel frequency
 *
 */
static inline void
dp_monitor_set_chan_freq(struct dp_pdev *pdev, qdf_freq_t chan_freq)
{
	if (qdf_unlikely(!pdev || !pdev->monitor_pdev))
		return;

	pdev->monitor_pdev->mon_chan_freq = chan_freq;
}

/*
 * dp_monitor_set_chan_band() - set channel band
 * @pdev: point to dp pdev
 * @chan_band: channel band
 *
 */
static inline void
dp_monitor_set_chan_band(struct dp_pdev *pdev, enum reg_wifi_band chan_band)
{
	if (qdf_unlikely(!pdev || !pdev->monitor_pdev))
		return;

	pdev->monitor_pdev->mon_chan_band = chan_band;
}

/*
 * dp_monitor_get_mpdu_status() - get mpdu status
 * @pdev: point to dp pdev
 * @soc: point to dp soc
 * @rx_tlv_hdr: point to rx tlv header
 *
 */
static inline void dp_monitor_get_mpdu_status(struct dp_pdev *pdev,
					      struct dp_soc *soc,
					      uint8_t *rx_tlv_hdr)
{
	struct dp_mon_pdev *mon_pdev;

	if (qdf_unlikely(!pdev || !pdev->monitor_pdev))
		return;

	mon_pdev = pdev->monitor_pdev;
	hal_rx_mon_hw_desc_get_mpdu_status(soc->hal_soc, rx_tlv_hdr,
					   &mon_pdev->ppdu_info.rx_status);
}

#ifdef FEATURE_NAC_RSSI
/*
 * dp_monitor_drop_inv_peer_pkts() - drop invalid peer pkts
 * @vdev: point to dp vdev
 *
 * Return: success if sta mode and filter for neighbour peers enabled
 */
static inline QDF_STATUS dp_monitor_drop_inv_peer_pkts(struct dp_vdev *vdev)
{
	struct dp_pdev *pdev = vdev->pdev;
	struct dp_soc *soc = pdev->soc;

	if (!soc->monitor_soc)
		return QDF_STATUS_E_FAILURE;

	if (!soc->monitor_soc->hw_nac_monitor_support &&
	    pdev->monitor_pdev->filter_neighbour_peers &&
	    vdev->opmode == wlan_op_mode_sta)
		return QDF_STATUS_SUCCESS;

	return QDF_STATUS_E_FAILURE;
}
#endif

/*
 * dp_peer_ppdu_delayed_ba_init() - Initialize ppdu in peer
 * @peer: Datapath peer
 *
 * return: void
 */
#ifdef FEATURE_PERPKT_INFO
static inline void dp_peer_ppdu_delayed_ba_init(struct dp_peer *peer)
{
	struct dp_mon_peer *mon_peer = peer->monitor_peer;

	if (!mon_peer)
		return;

	qdf_mem_zero(&mon_peer->delayed_ba_ppdu_stats,
		     sizeof(struct cdp_delayed_tx_completion_ppdu_user));
	mon_peer->last_delayed_ba = false;
	mon_peer->last_delayed_ba_ppduid = 0;
}
#else
static inline void dp_peer_ppdu_delayed_ba_init(struct dp_peer *peer)
{
}
#endif

/*
 * dp_monitor_vdev_register_osif() - Register osif rx mon
 * @vdev: point to vdev
 * @txrx_ops: point to ol txrx ops
 *
 * Return: void
 */
static inline void dp_monitor_vdev_register_osif(struct dp_vdev *vdev,
						 struct ol_txrx_ops *txrx_ops)
{
	if (!vdev->monitor_vdev)
		return;

	vdev->monitor_vdev->osif_rx_mon = txrx_ops->rx.mon;
}

/*
 * dp_monitor_get_monitor_vdev_from_pdev() - Get monitor vdev
 * @pdev: point to pdev
 *
 * Return: pointer to vdev
 */
static inline struct dp_vdev*
dp_monitor_get_monitor_vdev_from_pdev(struct dp_pdev *pdev)
{
	if (!pdev || !pdev->monitor_pdev || !pdev->monitor_pdev->mvdev)
		return NULL;

	return pdev->monitor_pdev->mvdev;
}

/*
 * dp_monitor_is_vdev_timer_running() - Get vdev timer status
 * @soc: point to soc
 *
 * Return: true if timer running
 */
static inline bool dp_monitor_is_vdev_timer_running(struct dp_soc *soc)
{
	if (qdf_unlikely(!soc || !soc->monitor_soc))
		return false;

	return !!(soc->monitor_soc->mon_vdev_timer_state &
		  MON_VDEV_TIMER_RUNNING);
}

/*
 * dp_monitor_get_link_desc_pages() - Get link desc pages
 * @soc: point to soc
 * @mac_id: mac id
 *
 * Return: return point to link desc pages
 */
static inline struct qdf_mem_multi_page_t*
dp_monitor_get_link_desc_pages(struct dp_soc *soc, uint32_t mac_id)
{
	if (qdf_unlikely(!soc || !soc->monitor_soc))
		return NULL;

	return &soc->monitor_soc->mon_link_desc_pages[mac_id];
}

/*
 * dp_monitor_get_total_link_descs() - Get total link descs
 * @soc: point to soc
 * @mac_id: mac id
 *
 * Return: return point total link descs
 */
static inline uint32_t *
dp_monitor_get_total_link_descs(struct dp_soc *soc, uint32_t mac_id)
{
	return &soc->monitor_soc->total_mon_link_descs[mac_id];
}

/*
 * dp_monitor_pdev_attach() - Monitor pdev attach
 * @pdev: point to pdev
 *
 * Return: return QDF_STATUS
 */
static inline QDF_STATUS dp_monitor_pdev_attach(struct dp_pdev *pdev)
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
		dp_mon_debug("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_pdev_attach(pdev);
}

/*
 * dp_monitor_pdev_detach() - Monitor pdev detach
 * @pdev: point to pdev
 *
 * Return: return QDF_STATUS
 */
static inline QDF_STATUS dp_monitor_pdev_detach(struct dp_pdev *pdev)
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
		dp_mon_debug("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_pdev_detach(pdev);
}

/*
 * dp_monitor_vdev_attach() - Monitor vdev attach
 * @vdev: point to vdev
 *
 * Return: return QDF_STATUS
 */
static inline QDF_STATUS dp_monitor_vdev_attach(struct dp_vdev *vdev)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = vdev->pdev->soc->monitor_soc;

	if (!mon_soc)
		return QDF_STATUS_E_FAILURE;

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_vdev_attach) {
		dp_mon_debug("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_vdev_attach(vdev);
}

/*
 * dp_monitor_vdev_detach() - Monitor vdev detach
 * @vdev: point to vdev
 *
 * Return: return QDF_STATUS
 */
static inline QDF_STATUS dp_monitor_vdev_detach(struct dp_vdev *vdev)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = vdev->pdev->soc->monitor_soc;

	if (!mon_soc)
		return QDF_STATUS_E_FAILURE;

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_vdev_detach) {
		dp_mon_debug("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_vdev_detach(vdev);
}

/*
 * dp_monitor_peer_attach() - Monitor peer attach
 * @soc: point to soc
 * @peer: point to peer
 *
 * Return: return QDF_STATUS
 */
static inline QDF_STATUS dp_monitor_peer_attach(struct dp_soc *soc,
						struct dp_peer *peer)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

	if (!mon_soc)
		return QDF_STATUS_E_FAILURE;

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_peer_attach) {
		dp_mon_debug("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_peer_attach(peer);
}

/*
 * dp_monitor_peer_detach() - Monitor peer detach
 * @soc: point to soc
 * @peer: point to peer
 *
 * Return: return QDF_STATUS
 */
static inline QDF_STATUS dp_monitor_peer_detach(struct dp_soc *soc,
						struct dp_peer *peer)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

	if (!mon_soc)
		return QDF_STATUS_E_FAILURE;

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_peer_detach) {
		dp_mon_debug("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_peer_detach(peer);
}

/*
 * dp_monitor_pdev_init() - Monitor pdev init
 * @pdev: point to pdev
 *
 * Return: return QDF_STATUS
 */
static inline QDF_STATUS dp_monitor_pdev_init(struct dp_pdev *pdev)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	/*
	 * mon_soc uninitialized when modular support enabled
	 * monitor related attach/detach/init/deinit
	 * will be done while monitor insmod
	 */
	if (!mon_soc)
		return QDF_STATUS_SUCCESS;

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_pdev_init) {
		dp_mon_debug("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_pdev_init(pdev);
}

/*
 * dp_monitor_pdev_deinit() - Monitor pdev deinit
 * @pdev: point to pdev
 *
 * Return: return QDF_STATUS
 */
static inline QDF_STATUS dp_monitor_pdev_deinit(struct dp_pdev *pdev)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	/*
	 * mon_soc uninitialized modular when support enabled
	 * monitor related attach/detach/init/deinit
	 * will be done while monitor insmod
	 */
	if (!mon_soc)
		return QDF_STATUS_SUCCESS;

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_pdev_deinit) {
		dp_mon_debug("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_pdev_deinit(pdev);
}

/*
 * dp_monitor_soc_cfg_init() - Monitor sco cfg init
 * @soc: point to soc
 *
 * Return: return QDF_STATUS
 */
static inline QDF_STATUS dp_monitor_soc_cfg_init(struct dp_soc *soc)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

	/*
	 * this API is getting call from dp_soc_init,
	 * mon_soc will be uninitialized when monitor support enabled
	 * So returning QDF_STATUS_SUCCESS.
	 * soc cfg init will be done while monitor insmod.
	 */
	if (!mon_soc)
		return QDF_STATUS_SUCCESS;

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_soc_cfg_init) {
		dp_mon_debug("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_soc_cfg_init(soc);
}

/*
 * dp_monitor_config_debug_sniffer() - Monitor config debug sniffer
 * @pdev: point to pdev
 * @val: val
 *
 * Return: return QDF_STATUS
 */
static inline QDF_STATUS dp_monitor_config_debug_sniffer(struct dp_pdev *pdev,
							 int val)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc)
		return QDF_STATUS_E_FAILURE;

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_config_debug_sniffer) {
		dp_mon_debug("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_config_debug_sniffer(pdev, val);
}

/*
 * dp_monitor_flush_rings() - Flush monitor rings
 * @soc: point to soc
 *
 * Return: None
 */
static inline void dp_monitor_flush_rings(struct dp_soc *soc)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_flush_rings) {
		dp_mon_debug("callback not registered");
		return;
	}

	return monitor_ops->mon_flush_rings(soc);
}

/*
 * dp_monitor_htt_srng_setup() - Setup htt srng
 * @soc: point to soc
 * @pdev: point to pdev
 * @mac_id: lmac id
 * @mac for pdev: pdev id
 *
 * Return: QDF_STATUS
 */
#if !defined(DISABLE_MON_CONFIG)
static inline QDF_STATUS dp_monitor_htt_srng_setup(struct dp_soc *soc,
						   struct dp_pdev *pdev,
						   int mac_id,
						   int mac_for_pdev)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return QDF_STATUS_SUCCESS;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_pdev_htt_srng_setup) {
		dp_mon_debug("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_pdev_htt_srng_setup(soc, pdev, mac_id,
						    mac_for_pdev);
}

static inline QDF_STATUS dp_monitor_soc_htt_srng_setup(struct dp_soc *soc)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return QDF_STATUS_SUCCESS;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_soc_htt_srng_setup) {
		dp_mon_debug("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_soc_htt_srng_setup(soc);
}
#else
static inline QDF_STATUS dp_monitor_htt_srng_setup(struct dp_soc *soc,
						   struct dp_pdev *pdev,
						   int mac_id,
						   int mac_for_pdev)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/*
 * dp_monitor_service_mon_rings() - service monitor rings
 * @soc: point to soc
 * @quota: reap budget
 *
 * Return: None
 */
#if defined(DP_CON_MON)
static inline
void dp_monitor_service_mon_rings(struct dp_soc *soc, uint32_t quota)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_service_rings) {
		dp_mon_debug("callback not registered");
		return;
	}

	return monitor_ops->mon_service_rings(soc, quota);
}
#endif

/*
 * dp_monitor_process() - Process monitor
 * @soc: point to soc
 * @int_ctx: interrupt ctx
 * @mac_id: lma
 * @quota:
 *
 * Return: None
 */
#ifndef DISABLE_MON_CONFIG
static inline
uint32_t dp_monitor_process(struct dp_soc *soc, struct dp_intr *int_ctx,
			    uint32_t mac_id, uint32_t quota)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return 0;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_rx_process) {
		dp_mon_debug("callback not registered");
		return 0;
	}

	return monitor_ops->mon_rx_process(soc, int_ctx, mac_id, quota);
}

static inline uint32_t
dp_tx_mon_process(struct dp_soc *soc, struct dp_intr *int_ctx,
		  uint32_t mac_id, uint32_t quota)
{
	return 0;
}
#else
static inline
uint32_t dp_monitor_process(struct dp_soc *soc, struct dp_intr *int_ctx,
			    uint32_t mac_id, uint32_t quota)
{
	return 0;
}

static inline uint32_t
dp_tx_mon_process(struct dp_soc *soc, struct dp_intr *int_ctx,
		  uint32_t mac_id, uint32_t quota)
{
	return 0;
}
#endif

/*
 * dp_monitor_drop_packets_for_mac() - monitor_drop_packets_for_mac
 * @pdev: point to pdev
 * @mac_id:
 * @quota:
 *
 * Return:
 */
#if !defined(DISABLE_MON_CONFIG) && defined(MON_ENABLE_DROP_FOR_MAC)
static inline
uint32_t dp_monitor_drop_packets_for_mac(struct dp_pdev *pdev,
					 uint32_t mac_id, uint32_t quota)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return 0;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_drop_packets_for_mac) {
		dp_mon_debug("callback not registered");
		return 0;
	}

	return monitor_ops->mon_drop_packets_for_mac(pdev,
						     mac_id, quota);
}
#else
static inline
uint32_t dp_monitor_drop_packets_for_mac(struct dp_pdev *pdev,
					 uint32_t mac_id, uint32_t quota)
{
	return 0;
}
#endif

/*
 * dp_monitor_peer_tx_init() - peer tx init
 * @pdev: point to pdev
 * @peer: point to peer
 *
 * Return: None
 */
static inline void dp_monitor_peer_tx_init(struct dp_pdev *pdev,
					   struct dp_peer *peer)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_peer_tx_init) {
		dp_mon_debug("callback not registered");
		return;
	}

	return monitor_ops->mon_peer_tx_init(pdev, peer);
}

/*
 * dp_monitor_peer_tx_cleanup() - peer tx cleanup
 * @vdev: point to vdev
 * @peer: point to peer
 *
 * Return: None
 */
static inline void dp_monitor_peer_tx_cleanup(struct dp_vdev *vdev,
					      struct dp_peer *peer)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = vdev->pdev->soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_peer_tx_cleanup) {
		dp_mon_debug("callback not registered");
		return;
	}

	return monitor_ops->mon_peer_tx_cleanup(vdev, peer);
}

#ifdef WLAN_TX_PKT_CAPTURE_ENH
/*
 * dp_monitor_peer_tid_peer_id_update() - peer tid update
 * @soc: point to soc
 * @peer: point to peer
 * @peer_id: peer id
 *
 * Return: None
 */
static inline
void dp_monitor_peer_tid_peer_id_update(struct dp_soc *soc,
					struct dp_peer *peer,
					uint16_t peer_id)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_peer_tid_peer_id_update) {
		dp_mon_debug("callback not registered");
		return;
	}

	return monitor_ops->mon_peer_tid_peer_id_update(peer, peer_id);
}

/*
 * dp_monitor_tx_ppdu_stats_attach() - Attach tx ppdu stats
 * @pdev: point to pdev
 *
 * Return: None
 */
static inline void dp_monitor_tx_ppdu_stats_attach(struct dp_pdev *pdev)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_tx_ppdu_stats_attach) {
		dp_mon_debug("callback not registered");
		return;
	}

	return monitor_ops->mon_tx_ppdu_stats_attach(pdev);
}

/*
 * dp_monitor_tx_ppdu_stats_detach() - Detach tx ppdu stats
 * @pdev: point to pdev
 *
 * Return: None
 */
static inline void dp_monitor_tx_ppdu_stats_detach(struct dp_pdev *pdev)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_tx_ppdu_stats_detach) {
		dp_mon_debug("callback not registered");
		return;
	}

	return monitor_ops->mon_tx_ppdu_stats_detach(pdev);
}

/*
 * dp_monitor_tx_capture_debugfs_init() - Init tx capture debugfs
 * @pdev: point to pdev
 *
 * Return: QDF_STATUS_SUCCESS
 */
static inline
QDF_STATUS dp_monitor_tx_capture_debugfs_init(struct dp_pdev *pdev)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_tx_capture_debugfs_init) {
		dp_mon_debug("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_tx_capture_debugfs_init(pdev);
}

/*
 * dp_monitor_peer_tx_capture_filter_check() - Check tx capture filter
 * @pdev: point to pdev
 * @peer: point to peer
 *
 * Return: None
 */
static inline void dp_monitor_peer_tx_capture_filter_check(struct dp_pdev *pdev,
							   struct dp_peer *peer)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_peer_tx_capture_filter_check) {
		dp_mon_debug("callback not registered");
		return;
	}

	return monitor_ops->mon_peer_tx_capture_filter_check(pdev, peer);
}

/*
 * dp_monitor_tx_add_to_comp_queue() - add completion msdu to queue
 * @soc: point to soc
 * @desc: point to tx desc
 * @ts: Tx completion status from HAL/HTT descriptor
 * @peer: DP peer
 *
 * Return: None
 */
static inline
QDF_STATUS dp_monitor_tx_add_to_comp_queue(struct dp_soc *soc,
					   struct dp_tx_desc_s *desc,
					   struct hal_tx_completion_status *ts,
					   struct dp_peer *peer)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return QDF_STATUS_SUCCESS;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_tx_add_to_comp_queue) {
		dp_mon_debug("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_tx_add_to_comp_queue(soc, desc, ts, peer);
}

static inline
QDF_STATUS monitor_update_msdu_to_list(struct dp_soc *soc,
				       struct dp_pdev *pdev,
				       struct dp_peer *peer,
				       struct hal_tx_completion_status *ts,
				       qdf_nbuf_t netbuf)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return QDF_STATUS_SUCCESS;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_update_msdu_to_list) {
		dp_mon_debug("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_update_msdu_to_list(soc, pdev,
						    peer, ts, netbuf);
}

#else
static inline
void dp_monitor_peer_tid_peer_id_update(struct dp_soc *soc,
					struct dp_peer *peer,
					uint16_t peer_id)
{
}

static inline void dp_monitor_tx_ppdu_stats_attach(struct dp_pdev *pdev)
{
}

static inline void dp_monitor_tx_ppdu_stats_detach(struct dp_pdev *pdev)
{
}

static inline
QDF_STATUS dp_monitor_tx_capture_debugfs_init(struct dp_pdev *pdev)
{
	return QDF_STATUS_E_FAILURE;
}

static inline void dp_monitor_peer_tx_capture_filter_check(struct dp_pdev *pdev,
							   struct dp_peer *peer)
{
}

static inline
QDF_STATUS dp_monitor_tx_add_to_comp_queue(struct dp_soc *soc,
					   struct dp_tx_desc_s *desc,
					   struct hal_tx_completion_status *ts,
					   struct dp_peer *peer)
{
	return QDF_STATUS_E_FAILURE;
}

static inline
QDF_STATUS monitor_update_msdu_to_list(struct dp_soc *soc,
				       struct dp_pdev *pdev,
				       struct dp_peer *peer,
				       struct hal_tx_completion_status *ts,
				       qdf_nbuf_t netbuf)
{
	return QDF_STATUS_E_FAILURE;
}
#endif

/*
 * dp_monitor_ppdu_stats_ind_handler() - PPDU stats msg handler
 * @htt_soc:     HTT SOC handle
 * @msg_word:    Pointer to payload
 * @htt_t2h_msg: HTT msg nbuf
 *
 * Return: True if buffer should be freed by caller.
 */
#if defined(WDI_EVENT_ENABLE) &&\
	(defined(QCA_ENHANCED_STATS_SUPPORT) || !defined(REMOVE_PKT_LOG))
static inline bool dp_monitor_ppdu_stats_ind_handler(struct htt_soc *soc,
						     uint32_t *msg_word,
						     qdf_nbuf_t htt_t2h_msg)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = soc->dp_soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return true;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_ppdu_stats_ind_handler) {
		dp_mon_debug("callback not registered");
		return true;
	}

	return monitor_ops->mon_ppdu_stats_ind_handler(soc, msg_word,
						       htt_t2h_msg);
}
#else
static inline bool dp_monitor_ppdu_stats_ind_handler(struct htt_soc *soc,
						     uint32_t *msg_word,
						     qdf_nbuf_t htt_t2h_msg)
{
	return true;
}
#endif

/*
 * dp_monitor_htt_ppdu_stats_attach() - attach resources for HTT PPDU
 * stats processing
 * @pdev: Datapath PDEV handle
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS dp_monitor_htt_ppdu_stats_attach(struct dp_pdev *pdev)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return QDF_STATUS_SUCCESS;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_htt_ppdu_stats_attach) {
		dp_mon_debug("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_htt_ppdu_stats_attach(pdev);
}

/*
 * dp_monitor_htt_ppdu_stats_detach() - detach stats resources
 * @pdev: Datapath PDEV handle
 *
 * Return: void
 */
static inline void dp_monitor_htt_ppdu_stats_detach(struct dp_pdev *pdev)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_htt_ppdu_stats_detach) {
		dp_mon_debug("callback not registered");
		return;
	}

	return monitor_ops->mon_htt_ppdu_stats_detach(pdev);
}

/*
 * dp_monitor_print_pdev_rx_mon_stats() - print rx mon stats
 * @pdev: Datapath PDEV handle
 *
 * Return: void
 */
static inline void dp_monitor_print_pdev_rx_mon_stats(struct dp_pdev *pdev)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_print_pdev_rx_mon_stats) {
		dp_mon_debug("callback not registered");
		return;
	}

	return monitor_ops->mon_print_pdev_rx_mon_stats(pdev);
}

/*
 * dp_monitor_print_pdev_tx_capture_stats() - print tx capture stats
 * @pdev: Datapath PDEV handle
 *
 * Return: void
 */
#ifdef WLAN_TX_PKT_CAPTURE_ENH
static inline void dp_monitor_print_pdev_tx_capture_stats(struct dp_pdev *pdev)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_print_pdev_tx_capture_stats) {
		dp_mon_debug("callback not registered");
		return;
	}

	return monitor_ops->mon_print_pdev_tx_capture_stats(pdev);
}

static inline QDF_STATUS dp_monitor_config_enh_tx_capture(struct dp_pdev *pdev,
							  uint32_t val)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_config_enh_tx_capture) {
		dp_mon_debug("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_config_enh_tx_capture(pdev, val);
}
#else
static inline void dp_monitor_print_pdev_tx_capture_stats(struct dp_pdev *pdev)
{
}

static inline QDF_STATUS dp_monitor_config_enh_tx_capture(struct dp_pdev *pdev,
							  uint32_t val)
{
	return QDF_STATUS_E_INVAL;
}
#endif

#ifdef WLAN_RX_PKT_CAPTURE_ENH
static inline QDF_STATUS dp_monitor_config_enh_rx_capture(struct dp_pdev *pdev,
							  uint32_t val)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_config_enh_rx_capture) {
		dp_mon_debug("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_config_enh_rx_capture(pdev, val);
}
#else
static inline QDF_STATUS dp_monitor_config_enh_rx_capture(struct dp_pdev *pdev,
							  uint32_t val)
{
	return QDF_STATUS_E_INVAL;
}
#endif

#ifdef QCA_SUPPORT_BPR
static inline QDF_STATUS dp_monitor_set_bpr_enable(struct dp_pdev *pdev,
						   uint32_t val)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_set_bpr_enable) {
		dp_mon_debug("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_set_bpr_enable(pdev, val);
}
#else
static inline QDF_STATUS dp_monitor_set_bpr_enable(struct dp_pdev *pdev,
						   uint32_t val)
{
	return QDF_STATUS_E_FAILURE;
}
#endif

#ifdef ATH_SUPPORT_NAC
static inline
int dp_monitor_set_filter_neigh_peers(struct dp_pdev *pdev, bool val)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return 0;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_set_filter_neigh_peers) {
		dp_mon_debug("callback not registered");
		return 0;
	}

	return monitor_ops->mon_set_filter_neigh_peers(pdev, val);
}
#else
static inline
int dp_monitor_set_filter_neigh_peers(struct dp_pdev *pdev, bool val)
{
	return 0;
}
#endif

#ifdef WLAN_ATF_ENABLE
static inline
void dp_monitor_set_atf_stats_enable(struct dp_pdev *pdev, bool value)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_set_atf_stats_enable) {
		dp_mon_debug("callback not registered");
		return;
	}

	return monitor_ops->mon_set_atf_stats_enable(pdev, value);
}
#else
static inline
void dp_monitor_set_atf_stats_enable(struct dp_pdev *pdev, bool value)
{
}
#endif

static inline
void dp_monitor_set_bsscolor(struct dp_pdev *pdev, uint8_t bsscolor)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_set_bsscolor) {
		dp_mon_debug("callback not registered");
		return;
	}

	return monitor_ops->mon_set_bsscolor(pdev, bsscolor);
}

static inline
bool dp_monitor_pdev_get_filter_mcast_data(struct cdp_pdev *pdev_handle)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return false;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_pdev_get_filter_mcast_data) {
		dp_mon_debug("callback not registered");
		return false;
	}

	return monitor_ops->mon_pdev_get_filter_mcast_data(pdev_handle);
}

static inline
bool dp_monitor_pdev_get_filter_non_data(struct cdp_pdev *pdev_handle)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return false;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_pdev_get_filter_non_data) {
		dp_mon_debug("callback not registered");
		return false;
	}

	return monitor_ops->mon_pdev_get_filter_non_data(pdev_handle);
}

static inline
bool dp_monitor_pdev_get_filter_ucast_data(struct cdp_pdev *pdev_handle)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return false;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_pdev_get_filter_ucast_data) {
		dp_mon_debug("callback not registered");
		return false;
	}

	return monitor_ops->mon_pdev_get_filter_ucast_data(pdev_handle);
}

#ifdef WDI_EVENT_ENABLE
static inline
int dp_monitor_set_pktlog_wifi3(struct dp_pdev *pdev, uint32_t event,
				bool enable)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return 0;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_set_pktlog_wifi3) {
		dp_mon_debug("callback not registered");
		return 0;
	}

	return monitor_ops->mon_set_pktlog_wifi3(pdev, event, enable);
}
#else
static inline
int dp_monitor_set_pktlog_wifi3(struct dp_pdev *pdev, uint32_t event,
				bool enable)
{
	return 0;
}
#endif

#if defined(DP_CON_MON) && !defined(REMOVE_PKT_LOG)
static inline void dp_monitor_pktlogmod_exit(struct dp_pdev *pdev)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_pktlogmod_exit) {
		dp_mon_debug("callback not registered");
		return;
	}

	return monitor_ops->mon_pktlogmod_exit(pdev);
}
#else
static inline void dp_monitor_pktlogmod_exit(struct dp_pdev *pdev) {}
#endif

static inline
void dp_monitor_vdev_set_monitor_mode_buf_rings(struct dp_pdev *pdev)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_vdev_set_monitor_mode_buf_rings) {
		dp_mon_debug("callback not registered");
		return;
	}

	return monitor_ops->mon_vdev_set_monitor_mode_buf_rings(pdev);
}

static inline
void dp_monitor_neighbour_peers_detach(struct dp_pdev *pdev)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_neighbour_peers_detach) {
		dp_mon_debug("callback not registered");
		return;
	}

	return monitor_ops->mon_neighbour_peers_detach(pdev);
}

#ifdef FEATURE_NAC_RSSI
static inline QDF_STATUS dp_monitor_filter_neighbour_peer(struct dp_pdev *pdev,
							  uint8_t *rx_pkt_hdr)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_filter_neighbour_peer) {
		dp_mon_debug("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_filter_neighbour_peer(pdev, rx_pkt_hdr);
}
#endif

static inline
void dp_monitor_reap_timer_init(struct dp_soc *soc)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_reap_timer_init) {
		dp_mon_debug("callback not registered");
		return;
	}

	monitor_ops->mon_reap_timer_init(soc);
}

static inline
void dp_monitor_reap_timer_deinit(struct dp_soc *soc)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_reap_timer_deinit) {
		dp_mon_debug("callback not registered");
		return;
	}

	monitor_ops->mon_reap_timer_deinit(soc);
}

static inline
void dp_monitor_reap_timer_start(struct dp_soc *soc)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_reap_timer_start) {
		dp_mon_debug("callback not registered");
		return;
	}

	monitor_ops->mon_reap_timer_start(soc);
}

static inline
bool dp_monitor_reap_timer_stop(struct dp_soc *soc)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return false;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_reap_timer_stop) {
		dp_mon_debug("callback not registered");
		return false;
	}

	return monitor_ops->mon_reap_timer_stop(soc);
}

static inline
void dp_monitor_vdev_timer_init(struct dp_soc *soc)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_vdev_timer_init) {
		dp_mon_debug("callback not registered");
		return;
	}

	monitor_ops->mon_vdev_timer_init(soc);
}

static inline
void dp_monitor_vdev_timer_deinit(struct dp_soc *soc)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_vdev_timer_deinit) {
		dp_mon_debug("callback not registered");
		return;
	}

	monitor_ops->mon_vdev_timer_deinit(soc);
}

static inline
void dp_monitor_vdev_timer_start(struct dp_soc *soc)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_vdev_timer_start) {
		dp_mon_debug("callback not registered");
		return;
	}

	monitor_ops->mon_vdev_timer_start(soc);
}

static inline
bool dp_monitor_vdev_timer_stop(struct dp_soc *soc)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return false;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_vdev_timer_stop) {
		dp_mon_debug("callback not registered");
		return false;
	}

	return monitor_ops->mon_vdev_timer_stop(soc);
}

#ifdef QCA_MCOPY_SUPPORT
static inline
QDF_STATUS dp_monitor_mcopy_check_deliver(struct dp_pdev *pdev,
					  uint16_t peer_id, uint32_t ppdu_id,
					  uint8_t first_msdu)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_mcopy_check_deliver) {
		dp_mon_debug("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_mcopy_check_deliver(pdev, peer_id,
						    ppdu_id, first_msdu);
}
#else
static inline
QDF_STATUS dp_monitor_mcopy_check_deliver(struct dp_pdev *pdev,
					  uint16_t peer_id, uint32_t ppdu_id,
					  uint8_t first_msdu)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/*
 * dp_monitor_neighbour_peer_add_ast() - Add ast entry
 * @pdev: point to dp pdev
 * @ta_peer: point to peer
 * @mac_addr: mac address
 * @nbuf: point to nbuf
 * @flags: flags
 *
 * Return: void
 */
static inline void
dp_monitor_neighbour_peer_add_ast(struct dp_pdev *pdev,
				  struct dp_peer *ta_peer,
				  uint8_t *mac_addr,
				  qdf_nbuf_t nbuf,
				  uint32_t flags)
{
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;
	struct dp_mon_ops *monitor_ops;

	if (!mon_soc) {
		dp_mon_debug("monitor soc is NULL");
		return;
	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_neighbour_peer_add_ast) {
		dp_mon_debug("callback not registered");
		return;
	}

	return monitor_ops->mon_neighbour_peer_add_ast(pdev, ta_peer, mac_addr,
						       nbuf, flags);
}

/*
 * dp_monitor_vdev_delete() - delete monitor vdev
 * @soc: point to dp soc
 * @vdev: point to dp vdev
 *
 * Return: void
 */
static inline void dp_monitor_vdev_delete(struct dp_soc *soc,
					  struct dp_vdev *vdev)
{
	if (soc->intr_mode == DP_INTR_POLL) {
		qdf_timer_sync_cancel(&soc->int_timer);
		dp_monitor_flush_rings(soc);
	} else if (soc->intr_mode == DP_INTR_MSI) {
		if (dp_monitor_vdev_timer_stop(soc))
			dp_monitor_flush_rings(soc);
	}

	dp_monitor_vdev_detach(vdev);
}

#ifdef DP_POWER_SAVE
/*
 * dp_monitor_pktlog_reap_pending_frames() - reap pending frames
 * @pdev: point to dp pdev
 *
 * Return: void
 */
static inline void dp_monitor_pktlog_reap_pending_frames(struct dp_pdev *pdev)
{
	struct dp_soc *soc;

	if (qdf_unlikely(!pdev || !pdev->monitor_pdev))
		return;

	soc = pdev->soc;

	if (((pdev->monitor_pdev->rx_pktlog_mode != DP_RX_PKTLOG_DISABLED) ||
	     dp_mon_is_enable_reap_timer_non_pkt(pdev))) {
		if (dp_monitor_reap_timer_stop(soc))
			dp_monitor_service_mon_rings(soc, DP_MON_REAP_BUDGET);
	}
}

/*
 * dp_monitor_pktlog_start_reap_timer() - start reap timer
 * @pdev: point to dp pdev
 *
 * Return: void
 */
static inline void dp_monitor_pktlog_start_reap_timer(struct dp_pdev *pdev)
{
	struct dp_soc *soc;

	if (qdf_unlikely(!pdev || !pdev->monitor_pdev))
		return;

	soc = pdev->soc;
	if (((pdev->monitor_pdev->rx_pktlog_mode != DP_RX_PKTLOG_DISABLED) ||
	     dp_mon_is_enable_reap_timer_non_pkt(pdev)))
		dp_monitor_reap_timer_start(soc);
}
#endif

/*
 * dp_monitor_neighbour_peer_list_remove() - remove neighbour peer list
 * @pdev: point to dp pdev
 * @vdev: point to dp vdev
 * @peer: point to dp_neighbour_peer
 *
 * Return: void
 */
static inline
void dp_monitor_neighbour_peer_list_remove(struct dp_pdev *pdev,
					   struct dp_vdev *vdev,
					   struct dp_neighbour_peer *peer)
{
	struct dp_mon_pdev *mon_pdev;
	struct dp_neighbour_peer *temp_peer = NULL;

	if (qdf_unlikely(!pdev || !pdev->monitor_pdev))
		return;

	mon_pdev = pdev->monitor_pdev;
	qdf_spin_lock_bh(&mon_pdev->neighbour_peer_mutex);
	if (!pdev->soc->monitor_soc->hw_nac_monitor_support) {
		TAILQ_FOREACH(peer, &mon_pdev->neighbour_peers_list,
			      neighbour_peer_list_elem) {
				QDF_ASSERT(peer->vdev != vdev);
			}
	} else {
		TAILQ_FOREACH_SAFE(peer, &mon_pdev->neighbour_peers_list,
				   neighbour_peer_list_elem, temp_peer) {
			if (peer->vdev == vdev) {
				TAILQ_REMOVE(&mon_pdev->neighbour_peers_list,
					     peer,
					     neighbour_peer_list_elem);
				qdf_mem_free(peer);
			}
		}
	}
	qdf_spin_unlock_bh(&mon_pdev->neighbour_peer_mutex);
}

static inline
void dp_monitor_pdev_set_mon_vdev(struct dp_vdev *vdev)
{
	if (!vdev->pdev->monitor_pdev)
		return;

	vdev->pdev->monitor_pdev->mvdev = vdev;
}

static inline
void dp_monitor_pdev_config_scan_spcl_vap(struct dp_pdev *pdev, bool val)
{
	if (!pdev || !pdev->monitor_pdev)
		return;

	pdev->monitor_pdev->scan_spcl_vap_configured = val;
}

#ifdef QCA_SUPPORT_SCAN_SPCL_VAP_STATS
static inline
void dp_monitor_pdev_reset_scan_spcl_vap_stats_enable(struct dp_pdev *pdev,
						      bool val)
{
	if (!pdev || !pdev->monitor_pdev)
		return;

	pdev->monitor_pdev->reset_scan_spcl_vap_stats_enable = val;
}
#else
static inline
void dp_monitor_pdev_reset_scan_spcl_vap_stats_enable(struct dp_pdev *pdev,
						      bool val)
{
}
#endif

static inline void
dp_mon_rx_wmask_subscribe(struct dp_soc *soc, uint32_t *msg_word,
			  struct htt_rx_ring_tlv_filter *tlv_filter)
{
	struct dp_mon_soc *mon_soc = soc->monitor_soc;
	struct dp_mon_ops *monitor_ops;

	if (!mon_soc) {
		dp_mon_debug("mon soc is NULL");
		return;

	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->rx_wmask_subscribe) {
		dp_mon_debug("callback not registered");
		return;
	}

	monitor_ops->rx_wmask_subscribe(msg_word, tlv_filter);
}

static inline void
dp_mon_rx_enable_mpdu_logging(struct dp_soc *soc, uint32_t *msg_word,
			      struct htt_rx_ring_tlv_filter *tlv_filter)
{
	struct dp_mon_soc *mon_soc = soc->monitor_soc;
	struct dp_mon_ops *monitor_ops;

	if (!mon_soc) {
		dp_mon_debug("mon soc is NULL");
		return;

	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->rx_enable_mpdu_logging) {
		dp_mon_debug("callback not registered");
		return;
	}

	monitor_ops->rx_enable_mpdu_logging(msg_word, tlv_filter);
}

static inline void
dp_mon_rx_packet_length_set(struct dp_soc *soc, uint32_t *msg_word,
			    struct htt_rx_ring_tlv_filter *tlv_filter)
{
	struct dp_mon_soc *mon_soc = soc->monitor_soc;
	struct dp_mon_ops *monitor_ops;

	if (!mon_soc) {
		dp_mon_debug("mon soc is NULL");
		return;

	}

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->rx_packet_length_set) {
		dp_mon_debug("callback not registered");
		return;
	}

	monitor_ops->rx_packet_length_set(msg_word, tlv_filter);
}

#ifdef QCA_ENHANCED_STATS_SUPPORT
QDF_STATUS dp_peer_qos_stats_notify(struct dp_pdev *dp_pdev,
				    struct cdp_rx_stats_ppdu_user *ppdu_user);
#endif

/*
 * dp_print_pdev_rx_mon_stats() - print rx mon stats
 * @pdev: device object
 *
 * Return: void
 */
void
dp_print_pdev_rx_mon_stats(struct dp_pdev *pdev);

/**
 * dp_mcopy_check_deliver() - mcopy check deliver
 * @pdev: DP pdev handle
 * @peer_id: peer id
 * @ppdu_id: ppdu
 * @first_msdu: flag to indicate first msdu of ppdu
 * Return: 0 on success, not 0 on failure
 */
QDF_STATUS dp_mcopy_check_deliver(struct dp_pdev *pdev,
				  uint16_t peer_id,
				  uint32_t ppdu_id,
				  uint8_t first_msdu);

/**
 * dp_pdev_set_advance_monitor_filter() - Set DP PDEV monitor filter
 * @soc: soc handle
 * @pdev_id: id of Datapath PDEV handle
 * @filter_val: Flag to select Filter for monitor mode
 * Return: 0 on success, not 0 on failure
 */
#ifdef QCA_ADVANCE_MON_FILTER_SUPPORT
QDF_STATUS
dp_pdev_set_advance_monitor_filter(struct cdp_soc_t *soc_hdl, uint8_t pdev_id,
				   struct cdp_monitor_filter *filter_val);
#else
static inline QDF_STATUS
dp_pdev_set_advance_monitor_filter(struct cdp_soc_t *soc_hdl, uint8_t pdev_id,
				   struct cdp_monitor_filter *filter_val)
{
	return QDF_STATUS_E_INVAL;
}
#endif /* QCA_ADVANCE_MON_FILTER_SUPPORT */

/**
 * dp_deliver_tx_mgmt() - Deliver mgmt frame for tx capture
 * @cdp_soc : data path soc handle
 * @pdev_id : pdev_id
 * @nbuf: Management frame buffer
 */
QDF_STATUS
dp_deliver_tx_mgmt(struct cdp_soc_t *cdp_soc, uint8_t pdev_id, qdf_nbuf_t nbuf);

/**
 * dp_filter_neighbour_peer() - API to filter neighbour peer
 * @pdev : DP pdev handle
 * @rx_pkt_hdr : packet header
 *
 * return: QDF_STATUS_SUCCESS on success
 *	   QDF_STATUS_E_FAILURE on failure
 */
#ifdef FEATURE_NAC_RSSI
QDF_STATUS dp_filter_neighbour_peer(struct dp_pdev *pdev,
				    uint8_t *rx_pkt_hdr);
#else
static inline
QDF_STATUS dp_filter_neighbour_peer(struct dp_pdev *pdev,
				    uint8_t *rx_pkt_hdr)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* FEATURE_NAC_RSSI */

/*
 * dp_neighbour_peers_detach() - Detach neighbour peers(nac clients)
 * @pdev: device object
 *
 * Return: void
 */
void dp_neighbour_peers_detach(struct dp_pdev *pdev);

/**
 * dp_reset_monitor_mode() - Disable monitor mode
 * @soc_hdl: Datapath soc handle
 * @pdev_id: id of datapath PDEV handle
 * @smart_monitor: smart monitor flag
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_reset_monitor_mode(struct cdp_soc_t *soc_hdl,
				 uint8_t pdev_id,
				 uint8_t smart_monitor);

static inline
struct dp_mon_ops *dp_mon_ops_get(struct dp_soc *soc)
{
	if (soc && soc->monitor_soc)
		return soc->monitor_soc->mon_ops;

	return NULL;
}

static inline
struct cdp_mon_ops *dp_mon_cdp_ops_get(struct dp_soc *soc)
{
	struct cdp_ops *ops = soc->cdp_soc.ops;

	return ops->mon_ops;
}

/**
 * dp_ppdu_desc_user_stats_update(): Function to update TX user stats
 * @pdev: DP pdev handle
 * @ppdu_info: per PPDU TLV descriptor
 *
 * return: void
 */
#ifdef QCA_ENHANCED_STATS_SUPPORT
void
dp_ppdu_desc_user_stats_update(struct dp_pdev *pdev,
			       struct ppdu_info *ppdu_info);
#else
static inline void
dp_ppdu_desc_user_stats_update(struct dp_pdev *pdev,
			       struct ppdu_info *ppdu_info)
{
}
#endif /* QCA_ENHANCED_STATS_SUPPORT */

/**
 * dp_mon_ops_get_1_0(): Get legacy monitor ops
 *
 * return: Pointer to dp_mon_ops
 */
struct dp_mon_ops *dp_mon_ops_get_1_0(void);

/**
 * dp_mon_cdp_ops_get_1_0(): Get legacy monitor cdp ops
 *
 * return: Pointer to dp_mon_cdp_ops
 */
struct cdp_mon_ops *dp_mon_cdp_ops_get_1_0(void);

#ifdef QCA_MONITOR_2_0_SUPPORT
/**
 * dp_mon_ops_get_2_0(): Get BE monitor ops
 *
 * return: Pointer to dp_mon_ops
 */
struct dp_mon_ops *dp_mon_ops_get_2_0(void);

/**
 * dp_mon_cdp_ops_get_2_0(): Get BE monitor cdp ops
 *
 * return: Pointer to dp_mon_cdp_ops
 */
struct cdp_mon_ops *dp_mon_cdp_ops_get_2_0(void);
#endif

#endif /* _DP_MON_H_ */
