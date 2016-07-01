/*
 * Copyright (c) 2013-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/**
 * @file ol_txrx_types.h
 * @brief Define the major data types used internally by the host datapath SW.
 */
#ifndef _OL_TXRX_TYPES__H_
#define _OL_TXRX_TYPES__H_

#include <qdf_nbuf.h>           /* qdf_nbuf_t */
#include <qdf_mem.h>
#include <cds_queue.h>          /* TAILQ */
#include <a_types.h>            /* A_UINT8 */
#include <htt.h>                /* htt_sec_type, htt_pkt_type, etc. */
#include <qdf_atomic.h>         /* qdf_atomic_t */
#include <wdi_event_api.h>      /* wdi_event_subscribe */
#include <qdf_timer.h>		/* qdf_timer_t */
#include <qdf_lock.h>           /* qdf_spinlock */
#include <pktlog.h>             /* ol_pktlog_dev_handle */
#include <ol_txrx_stats.h>
#include <txrx.h>
#include "ol_txrx_htt_api.h"
#include "ol_htt_tx_api.h"
#include "ol_htt_rx_api.h"
#include "ol_txrx_ctrl_api.h" /* WLAN_MAX_STA_COUNT */
#include "ol_txrx_osif_api.h" /* ol_rx_callback_fp */
#include "cdp_txrx_flow_ctrl_v2.h"
#include "cdp_txrx_peer_ops.h"

/*
 * The target may allocate multiple IDs for a peer.
 * In particular, the target may allocate one ID to represent the
 * multicast key the peer uses, and another ID to represent the
 * unicast key the peer uses.
 */
#define MAX_NUM_PEER_ID_PER_PEER 8

#define OL_TXRX_INVALID_NUM_PEERS (-1)

#define OL_TXRX_MAC_ADDR_LEN 6

/* OL_TXRX_NUM_EXT_TIDS -
 * 16 "real" TIDs + 3 pseudo-TIDs for mgmt, mcast/bcast & non-QoS data
 */
#define OL_TXRX_NUM_EXT_TIDS 19

#define OL_TX_NUM_QOS_TIDS 16   /* 16 regular TIDs */
#define OL_TX_NON_QOS_TID 16
#define OL_TX_MGMT_TID    17
#define OL_TX_NUM_TIDS    18
#define OL_RX_MCAST_TID   18  /* Mcast TID only between f/w & host */

#define OL_TX_VDEV_MCAST_BCAST    0 /* HTT_TX_EXT_TID_MCAST_BCAST */
#define OL_TX_VDEV_DEFAULT_MGMT   1 /* HTT_TX_EXT_TID_DEFALT_MGMT */
#define OL_TX_VDEV_NUM_QUEUES     2

#define OL_TXRX_MGMT_TYPE_BASE htt_pkt_num_types
#define OL_TXRX_MGMT_NUM_TYPES 8

#define OL_TX_MUTEX_TYPE qdf_spinlock_t
#define OL_RX_MUTEX_TYPE qdf_spinlock_t

/* TXRX Histogram defines */
#define TXRX_DATA_HISTROGRAM_GRANULARITY      1000
#define TXRX_DATA_HISTROGRAM_NUM_INTERVALS    100

struct ol_txrx_pdev_t;
struct ol_txrx_vdev_t;
struct ol_txrx_peer_t;

/* rx filter related */
#define MAX_PRIVACY_FILTERS           4 /* max privacy filters */

enum privacy_filter {
	PRIVACY_FILTER_ALWAYS,
	PRIVACY_FILTER_KEY_UNAVAILABLE,
};

enum privacy_filter_packet_type {
	PRIVACY_FILTER_PACKET_UNICAST,
	PRIVACY_FILTER_PACKET_MULTICAST,
	PRIVACY_FILTER_PACKET_BOTH
};

struct privacy_exemption {
	/* ethertype -
	 * type of ethernet frames this filter applies to, in host byte order
	 */
	uint16_t ether_type;
	enum privacy_filter filter_type;
	enum privacy_filter_packet_type packet_type;
};

enum ol_tx_frm_type {
	OL_TX_FRM_STD = 0, /* regular frame - no added header fragments */
	OL_TX_FRM_TSO,     /* TSO segment, with a modified IP header added */
	OL_TX_FRM_AUDIO,   /* audio frames, with a custom LLC/SNAP hdr added */
	OL_TX_FRM_NO_FREE, /* frame requires special tx completion callback */
};

#if defined(CONFIG_HL_SUPPORT) && defined(QCA_BAD_PEER_TX_FLOW_CL)

#define MAX_NO_PEERS_IN_LIMIT (2*10 + 2)

enum ol_tx_peer_bal_state {
	ol_tx_peer_bal_enable = 0,
	ol_tx_peer_bal_disable,
};

enum ol_tx_peer_bal_timer_state {
	ol_tx_peer_bal_timer_disable = 0,
	ol_tx_peer_bal_timer_active,
	ol_tx_peer_bal_timer_inactive,
};

struct ol_tx_limit_peer_t {
	u_int16_t limit_flag;
	u_int16_t peer_id;
	u_int16_t limit;
};

enum tx_peer_level {
	TXRX_IEEE11_B = 0,
	TXRX_IEEE11_A_G,
	TXRX_IEEE11_N,
	TXRX_IEEE11_AC,
	TXRX_IEEE11_MAX,
};

struct tx_peer_threshold {
	u_int32_t tput_thresh;
	u_int32_t tx_limit;
};
#endif


struct ol_tx_desc_t {
	qdf_nbuf_t netbuf;
	void *htt_tx_desc;
	uint16_t id;
	qdf_dma_addr_t htt_tx_desc_paddr;
	void *htt_frag_desc; /* struct msdu_ext_desc_t * */
	qdf_dma_addr_t htt_frag_desc_paddr;
	qdf_atomic_t ref_cnt;
	enum htt_tx_status status;

#ifdef QCA_COMPUTE_TX_DELAY
	uint32_t entry_timestamp_ticks;
#endif
	/*
	 * Allow tx descriptors to be stored in (doubly-linked) lists.
	 * This is mainly used for HL tx queuing and scheduling, but is
	 * also used by LL+HL for batch processing of tx frames.
	 */
	TAILQ_ENTRY(ol_tx_desc_t) tx_desc_list_elem;

	/*
	 * Remember whether the tx frame is a regular packet, or whether
	 * the driver added extra header fragments (e.g. a modified IP header
	 * for TSO fragments, or an added LLC/SNAP header for audio interworking
	 * data) that need to be handled in a special manner.
	 * This field is filled in with the ol_tx_frm_type enum.
	 */
	uint8_t pkt_type;
#if defined(CONFIG_HL_SUPPORT)
	struct ol_txrx_vdev_t *vdev;
#endif
	void *txq;

#ifdef QCA_SUPPORT_SW_TXRX_ENCAP
	/* used by tx encap, to restore the os buf start offset
	   after tx complete */
	uint8_t orig_l2_hdr_bytes;
#endif

#ifdef QCA_LL_TX_FLOW_CONTROL_V2
	struct ol_tx_flow_pool_t *pool;
#endif
	void *tso_desc;
};

typedef TAILQ_HEAD(some_struct_name, ol_tx_desc_t) ol_tx_desc_list;

union ol_tx_desc_list_elem_t {
	union ol_tx_desc_list_elem_t *next;
	struct ol_tx_desc_t tx_desc;
};

union ol_txrx_align_mac_addr_t {
	uint8_t raw[OL_TXRX_MAC_ADDR_LEN];
	struct {
		uint16_t bytes_ab;
		uint16_t bytes_cd;
		uint16_t bytes_ef;
	} align2;
	struct {
		uint32_t bytes_abcd;
		uint16_t bytes_ef;
	} align4;
};

struct ol_rx_reorder_timeout_list_elem_t {
	TAILQ_ENTRY(ol_rx_reorder_timeout_list_elem_t)
	reorder_timeout_list_elem;
	uint32_t timestamp_ms;
	struct ol_txrx_peer_t *peer;
	uint8_t tid;
	uint8_t active;
};

#define TXRX_TID_TO_WMM_AC(_tid) ( \
		(((_tid) >> 1) == 3) ? TXRX_WMM_AC_VO :	\
		(((_tid) >> 1) == 2) ? TXRX_WMM_AC_VI :	\
		(((_tid) ^ ((_tid) >> 1)) & 0x1) ? TXRX_WMM_AC_BK : \
		TXRX_WMM_AC_BE)

enum {
	OL_TX_SCHED_WRR_ADV_CAT_BE,
	OL_TX_SCHED_WRR_ADV_CAT_BK,
	OL_TX_SCHED_WRR_ADV_CAT_VI,
	OL_TX_SCHED_WRR_ADV_CAT_VO,
	OL_TX_SCHED_WRR_ADV_CAT_NON_QOS_DATA,
	OL_TX_SCHED_WRR_ADV_CAT_UCAST_MGMT,
	OL_TX_SCHED_WRR_ADV_CAT_MCAST_DATA,
	OL_TX_SCHED_WRR_ADV_CAT_MCAST_MGMT,

	OL_TX_SCHED_WRR_ADV_NUM_CATEGORIES /* must be last */
};

struct ol_tx_reorder_cat_timeout_t {
	TAILQ_HEAD(, ol_rx_reorder_timeout_list_elem_t) virtual_timer_list;
	qdf_timer_t timer;
	uint32_t duration_ms;
	struct ol_txrx_pdev_t *pdev;
};

enum ol_tx_scheduler_status {
	ol_tx_scheduler_idle = 0,
	ol_tx_scheduler_running,
};

enum ol_tx_queue_status {
	ol_tx_queue_empty = 0,
	ol_tx_queue_active,
	ol_tx_queue_paused,
};

struct ol_txrx_msdu_info_t {
	struct htt_msdu_info_t htt;
	struct ol_txrx_peer_t *peer;
	struct qdf_tso_info_t tso_info;
};

enum {
	ol_tx_aggr_untried = 0,
	ol_tx_aggr_enabled,
	ol_tx_aggr_disabled,
	ol_tx_aggr_retry,
	ol_tx_aggr_in_progress,
};

#define OL_TX_MAX_GROUPS_PER_QUEUE 1
#define OL_TX_MAX_VDEV_ID 16
#define OL_TXQ_GROUP_VDEV_ID_MASK_GET(_membership)           \
	(((_membership) & 0xffff0000) >> 16)
#define OL_TXQ_GROUP_VDEV_ID_BIT_MASK_GET(_mask, _vdev_id)   \
	((_mask >> _vdev_id) & 0x01)
#define OL_TXQ_GROUP_AC_MASK_GET(_membership)           \
	((_membership) & 0x0000ffff)
#define OL_TXQ_GROUP_AC_BIT_MASK_GET(_mask, _ac_mask)   \
	((_mask >> _ac_mask) & 0x01)
#define OL_TXQ_GROUP_MEMBERSHIP_GET(_vdev_mask, _ac_mask)     \
	((_vdev_mask << 16) | _ac_mask)

struct ol_tx_frms_queue_t {
	/* list_elem -
	 * Allow individual tx frame queues to be linked together into
	 * scheduler queues of tx frame queues
	 */
	TAILQ_ENTRY(ol_tx_frms_queue_t) list_elem;
	uint8_t aggr_state;
	struct {
		uint8_t total;
		/* pause requested by ctrl SW rather than txrx SW */
		uint8_t by_ctrl;
	} paused_count;
	uint8_t ext_tid;
	uint16_t frms;
	uint32_t bytes;
	ol_tx_desc_list head;
	enum ol_tx_queue_status flag;
	struct ol_tx_queue_group_t *group_ptrs[OL_TX_MAX_GROUPS_PER_QUEUE];
#if defined(CONFIG_HL_SUPPORT) && defined(QCA_BAD_PEER_TX_FLOW_CL)
	struct ol_txrx_peer_t *peer;
#endif
};

enum {
	ol_tx_log_entry_type_invalid,
	ol_tx_log_entry_type_queue_state,
	ol_tx_log_entry_type_enqueue,
	ol_tx_log_entry_type_dequeue,
	ol_tx_log_entry_type_drop,
	ol_tx_log_entry_type_queue_free,

	ol_tx_log_entry_type_wrap,
};

struct ol_tx_log_queue_state_var_sz_t {
	uint32_t active_bitmap;
	uint16_t credit;
	uint8_t num_cats_active;
	uint8_t data[1];
};

struct ol_tx_log_queue_add_t {
	uint8_t num_frms;
	uint8_t tid;
	uint16_t peer_id;
	uint16_t num_bytes;
};

struct ol_mac_addr {
	uint8_t mac_addr[OL_TXRX_MAC_ADDR_LEN];
};

struct ol_tx_sched_t;


#ifndef OL_TXRX_NUM_LOCAL_PEER_IDS
#define OL_TXRX_NUM_LOCAL_PEER_IDS 33   /* default */
#endif

#ifndef ol_txrx_local_peer_id_t
#define ol_txrx_local_peer_id_t uint8_t /* default */
#endif

#ifdef QCA_COMPUTE_TX_DELAY
/*
 * Delay histogram bins: 16 bins of 10 ms each to count delays
 * from 0-160 ms, plus one overflow bin for delays > 160 ms.
 */
#define QCA_TX_DELAY_HIST_INTERNAL_BINS 17
#define QCA_TX_DELAY_HIST_INTERNAL_BIN_WIDTH_MS 10

struct ol_tx_delay_data {
	struct {
		uint64_t transmit_sum_ticks;
		uint64_t queue_sum_ticks;
		uint32_t transmit_num;
		uint32_t queue_num;
	} avgs;
	uint16_t hist_bins_queue[QCA_TX_DELAY_HIST_INTERNAL_BINS];
};

#endif /* QCA_COMPUTE_TX_DELAY */

/* Thermal Mitigation */

enum throttle_level {
	THROTTLE_LEVEL_0,
	THROTTLE_LEVEL_1,
	THROTTLE_LEVEL_2,
	THROTTLE_LEVEL_3,
	/* Invalid */
	THROTTLE_LEVEL_MAX,
};

enum throttle_phase {
	THROTTLE_PHASE_OFF,
	THROTTLE_PHASE_ON,
	/* Invalid */
	THROTTLE_PHASE_MAX,
};

#define THROTTLE_TX_THRESHOLD (100)

typedef void (*ipa_uc_op_cb_type)(uint8_t *op_msg, void *osif_ctxt);

struct ol_tx_queue_group_t {
	qdf_atomic_t credit;
	u_int32_t membership;
};
#define OL_TX_MAX_TXQ_GROUPS 2

#define OL_TX_GROUP_STATS_LOG_SIZE 128
struct ol_tx_group_credit_stats_t {
	struct {
		struct {
			u_int16_t member_vdevs;
			u_int16_t credit;
		} grp[OL_TX_MAX_TXQ_GROUPS];
	} stats[OL_TX_GROUP_STATS_LOG_SIZE];
	u_int16_t last_valid_index;
	u_int16_t wrap_around;
};

#ifdef QCA_LL_TX_FLOW_CONTROL_V2

/**
 * enum flow_pool_status - flow pool status
 * @FLOW_POOL_ACTIVE_UNPAUSED : pool is active (can take/put descriptors)
 *				and network queues are unpaused
 * @FLOW_POOL_ACTIVE_PAUSED: pool is active (can take/put descriptors)
 *			   and network queues are paused
 * @FLOW_POOL_INVALID: pool is invalid (put descriptor)
 * @FLOW_POOL_INACTIVE: pool is inactive (pool is free)
 */
enum flow_pool_status {
	FLOW_POOL_ACTIVE_UNPAUSED = 0,
	FLOW_POOL_ACTIVE_PAUSED = 1,
	FLOW_POOL_INVALID = 2,
	FLOW_POOL_INACTIVE = 3,
};

/**
 * struct ol_txrx_pool_stats - flow pool related statistics
 * @pool_map_count: flow pool map received
 * @pool_unmap_count: flow pool unmap received
 * @pkt_drop_no_pool: packets dropped due to unavailablity of pool
 */
struct ol_txrx_pool_stats {
	uint16_t pool_map_count;
	uint16_t pool_unmap_count;
	uint16_t pkt_drop_no_pool;
};

/**
 * struct ol_tx_flow_pool_t - flow_pool info
 * @flow_pool_list_elem: flow_pool_list element
 * @flow_pool_lock: flow_pool lock
 * @flow_pool_id: flow_pool id
 * @flow_pool_size: flow_pool size
 * @avail_desc: available descriptors
 * @deficient_desc: deficient descriptors
 * @status: flow pool status
 * @flow_type: flow pool type
 * @member_flow_id: member flow id
 * @stop_th: stop threshold
 * @start_th: start threshold
 * @freelist: tx descriptor freelist
 * @pkt_drop_no_desc: drop due to no descriptors
 */
struct ol_tx_flow_pool_t {
	TAILQ_ENTRY(ol_tx_flow_pool_t) flow_pool_list_elem;
	qdf_spinlock_t flow_pool_lock;
	uint8_t flow_pool_id;
	uint16_t flow_pool_size;
	uint16_t avail_desc;
	uint16_t deficient_desc;
	enum flow_pool_status status;
	enum htt_flow_type flow_type;
	uint8_t member_flow_id;
	uint16_t stop_th;
	uint16_t start_th;
	union ol_tx_desc_list_elem_t *freelist;
	uint16_t pkt_drop_no_desc;
};

#endif

struct ol_txrx_peer_id_map {
	struct ol_txrx_peer_t *peer;
	qdf_atomic_t peer_id_ref_cnt;
};

/*
 * As depicted in the diagram below, the pdev contains an array of
 * NUM_EXT_TID ol_tx_active_queues_in_tid_t elements.
 * Each element identifies all the tx queues that are active for
 * the TID, from the different peers.
 *
 * Each peer contains an array of NUM_EXT_TID ol_tx_frms_queue_t elements.
 * Each element identifies the tx frames for the TID that need to be sent
 * to the peer.
 *
 *
 *  pdev: ol_tx_active_queues_in_tid_t active_in_tids[NUM_EXT_TIDS]
 *                                TID
 *       0            1            2                     17
 *  +============+============+============+==    ==+============+
 *  | active (y) | active (n) | active (n) |        | active (y) |
 *  |------------+------------+------------+--    --+------------|
 *  | queues     | queues     | queues     |        | queues     |
 *  +============+============+============+==    ==+============+
 *       |                                               |
 *    .--+-----------------------------------------------'
 *    |  |
 *    |  |     peer X:                            peer Y:
 *    |  |     ol_tx_frms_queue_t                 ol_tx_frms_queue_t
 *    |  |     tx_queues[NUM_EXT_TIDS]            tx_queues[NUM_EXT_TIDS]
 *    |  | TID +======+                       TID +======+
 *    |  `---->| next |-------------------------->| next |--X
 *    |     0  | prev |   .------.   .------.  0  | prev |   .------.
 *    |        | txq  |-->|txdesc|-->|txdesc|     | txq  |-->|txdesc|
 *    |        +======+   `------'   `------'     +======+   `------'
 *    |        | next |      |          |      1  | next |      |
 *    |     1  | prev |      v          v         | prev |      v
 *    |        | txq  |   .------.   .------.     | txq  |   .------.
 *    |        +======+   |netbuf|   |netbuf|     +======+   |netbuf|
 *    |        | next |   `------'   `------'     | next |   `------'
 *    |     2  | prev |                        2  | prev |
 *    |        | txq  |                           | txq  |
 *    |        +======+                           +======+
 *    |        |      |                           |      |
 *    |
 *    |
 *    |        |      |                           |      |
 *    |        +======+                           +======+
 *    `------->| next |--X                        | next |
 *          17 | prev |   .------.             17 | prev |
 *             | txq  |-->|txdesc|                | txq  |
 *             +======+   `------'                +======+
 *                           |
 *                           v
 *                        .------.
 *                        |netbuf|
 *                        `------'
 */
struct ol_txrx_pdev_t {
	/* ctrl_pdev - handle for querying config info */
	ol_pdev_handle ctrl_pdev;

	/* osdev - handle for mem alloc / free, map / unmap */
	qdf_device_t osdev;

	htt_pdev_handle htt_pdev;

#ifdef WLAN_FEATURE_FASTPATH
	struct CE_handle    *ce_tx_hdl; /* Handle to Tx packet posting CE */
	struct CE_handle    *ce_htt_msg_hdl; /* Handle to TxRx completion CE */
#endif /* WLAN_FEATURE_FASTPATH */

	struct {
		int is_high_latency;
		int host_addba;
		int ll_pause_txq_limit;
		int default_tx_comp_req;
	} cfg;

	/* WDI subscriber's event list */
	wdi_event_subscribe **wdi_event_list;

#if !defined(REMOVE_PKT_LOG) && !defined(QVIT)
	bool pkt_log_init;
	/* Pktlog pdev */
	struct ol_pktlog_dev_t *pl_dev;
#endif /* #ifndef REMOVE_PKT_LOG */

	enum ol_sec_type sec_types[htt_num_sec_types];
	/* standard frame type */
	enum wlan_frm_fmt frame_format;
	enum htt_pkt_type htt_pkt_type;

#ifdef QCA_SUPPORT_SW_TXRX_ENCAP
	/* txrx encap/decap   */
	uint8_t sw_tx_encap;
	uint8_t sw_rx_decap;
	uint8_t target_tx_tran_caps;
	uint8_t target_rx_tran_caps;
	/* llc process */
	uint8_t sw_tx_llc_proc_enable;
	uint8_t sw_rx_llc_proc_enable;
	/* A-MSDU */
	uint8_t sw_subfrm_hdr_recovery_enable;
	/* Protected Frame bit handling */
	uint8_t sw_pf_proc_enable;
#endif
	/*
	 * target tx credit -
	 * not needed for LL, but used for HL download scheduler to keep
	 * track of roughly how much space is available in the target for
	 * tx frames
	 */
	qdf_atomic_t target_tx_credit;
	qdf_atomic_t orig_target_tx_credit;

	/* Peer mac address to staid mapping */
	struct ol_mac_addr mac_to_staid[WLAN_MAX_STA_COUNT + 3];

	/* ol_txrx_vdev list */
	TAILQ_HEAD(, ol_txrx_vdev_t) vdev_list;

	/* peer ID to peer object map (array of pointers to peer objects) */
	struct ol_txrx_peer_id_map *peer_id_to_obj_map;

	struct {
		unsigned mask;
		unsigned idx_bits;
		TAILQ_HEAD(, ol_txrx_peer_t) * bins;
	} peer_hash;

	/* rx specific processing */
	struct {
		struct {
			TAILQ_HEAD(, ol_rx_reorder_t) waitlist;
			uint32_t timeout_ms;
		} defrag;
		struct {
			int defrag_timeout_check;
			int dup_check;
		} flags;

		struct {
			struct ol_tx_reorder_cat_timeout_t
				access_cats[TXRX_NUM_WMM_AC];
		} reorder_timeout;
		qdf_spinlock_t mutex;
	} rx;

	/* rx proc function */
	void (*rx_opt_proc)(struct ol_txrx_vdev_t *vdev,
			    struct ol_txrx_peer_t *peer,
			    unsigned tid, qdf_nbuf_t msdu_list);

	/* tx data delivery notification callback function */
	struct {
		ol_txrx_data_tx_cb func;
		void *ctxt;
	} tx_data_callback;

	/* tx management delivery notification callback functions */
	struct {
		struct {
			ol_txrx_mgmt_tx_cb download_cb;
			ol_txrx_mgmt_tx_cb ota_ack_cb;
			void *ctxt;
		} callbacks[OL_TXRX_MGMT_NUM_TYPES];
	} tx_mgmt;

	struct {
		uint16_t pool_size;
		uint16_t num_free;
		union ol_tx_desc_list_elem_t *array;
		union ol_tx_desc_list_elem_t *freelist;
#ifdef QCA_LL_TX_FLOW_CONTROL_V2
		uint8_t num_invalid_bin;
		qdf_spinlock_t flow_pool_list_lock;
		TAILQ_HEAD(flow_pool_list_t, ol_tx_flow_pool_t) flow_pool_list;
#endif
		uint32_t page_size;
		uint16_t desc_reserved_size;
		uint8_t page_divider;
		uint32_t offset_filter;
		struct qdf_mem_multi_page_t desc_pages;
#ifdef DESC_DUP_DETECT_DEBUG
		uint32_t *free_list_bitmap;
#endif
	} tx_desc;

	uint8_t is_mgmt_over_wmi_enabled;
#if defined(QCA_LL_TX_FLOW_CONTROL_V2)
	struct ol_txrx_pool_stats pool_stats;
	uint32_t num_msdu_desc;
#ifdef QCA_LL_TX_FLOW_GLOBAL_MGMT_POOL
	struct ol_tx_flow_pool_t *mgmt_pool;
#endif
#endif

	struct {
		int (*cmp)(union htt_rx_pn_t *new,
			   union htt_rx_pn_t *old,
			   int is_unicast, int opmode);
		int len;
	} rx_pn[htt_num_sec_types];

	/* tx mutex */
	OL_TX_MUTEX_TYPE tx_mutex;

	/*
	 * peer ref mutex:
	 * 1. Protect peer object lookups until the returned peer object's
	 *    reference count is incremented.
	 * 2. Provide mutex when accessing peer object lookup structures.
	 */
	OL_RX_MUTEX_TYPE peer_ref_mutex;

	/*
	 * last_real_peer_mutex:
	 * Protect lookups of any vdev's last_real_peer pointer until the
	 * reference count for the pointed-to peer object is incremented.
	 * This mutex could be in the vdev struct, but it's slightly simpler
	 * to have a single lock in the pdev struct.  Since the lock is only
	 * held for an extremely short time, and since it's very unlikely for
	 * two vdev's to concurrently access the lock, there's no real
	 * benefit to having a per-vdev lock.
	 */
	OL_RX_MUTEX_TYPE last_real_peer_mutex;

	qdf_spinlock_t peer_map_unmap_lock;

	struct {
		struct {
			struct {
				struct {
					uint64_t ppdus;
					uint64_t mpdus;
				} normal;
				struct {
					/*
					 * mpdu_bad is general -
					 * replace it with the specific counters
					 * below
					 */
					uint64_t mpdu_bad;
					/* uint64_t mpdu_fcs; */
					/* uint64_t mpdu_duplicate; */
					/* uint64_t mpdu_pn_replay; */
					/* uint64_t mpdu_bad_sender; */
					/* ^ comment: peer not found */
					/* uint64_t mpdu_flushed; */
					/* uint64_t msdu_defrag_mic_err; */
					uint64_t msdu_mc_dup_drop;
				} err;
			} rx;
		} priv;
		struct ol_txrx_stats pub;
	} stats;

#if defined(ENABLE_RX_REORDER_TRACE)
	struct {
		uint32_t mask;
		uint32_t idx;
		uint64_t cnt;
#define TXRX_RX_REORDER_TRACE_SIZE_LOG2 8       /* 256 entries */
		struct {
			uint16_t reorder_idx;
			uint16_t seq_num;
			uint8_t num_mpdus;
			uint8_t tid;
		} *data;
	} rx_reorder_trace;
#endif /* ENABLE_RX_REORDER_TRACE */

#if defined(ENABLE_RX_PN_TRACE)
	struct {
		uint32_t mask;
		uint32_t idx;
		uint64_t cnt;
#define TXRX_RX_PN_TRACE_SIZE_LOG2 5    /* 32 entries */
		struct {
			struct ol_txrx_peer_t *peer;
			uint32_t pn32;
			uint16_t seq_num;
			uint8_t unicast;
			uint8_t tid;
		} *data;
	} rx_pn_trace;
#endif /* ENABLE_RX_PN_TRACE */

#if defined(PERE_IP_HDR_ALIGNMENT_WAR)
	bool host_80211_enable;
#endif

	/*
	 * tx_sched only applies for HL, but is defined unconditionally
	 * rather than  only if defined(CONFIG_HL_SUPPORT).
	 * This is because the struct only
	 * occupies a few bytes, and to avoid the complexity of
	 * wrapping references
	 * to the struct members in "defined(CONFIG_HL_SUPPORT)" conditional
	 * compilation.
	 * If this struct gets expanded to a non-trivial size,
	 * then it should be
	 * conditionally compiled to only apply if defined(CONFIG_HL_SUPPORT).
	 */
	qdf_spinlock_t tx_queue_spinlock;
	struct {
		enum ol_tx_scheduler_status tx_sched_status;
		struct ol_tx_sched_t *scheduler;
		struct ol_tx_frms_queue_t *last_used_txq;
	} tx_sched;
	/*
	 * tx_queue only applies for HL, but is defined unconditionally to avoid
	 * wrapping references to tx_queue in "defined(CONFIG_HL_SUPPORT)"
	 * conditional compilation.
	 */
	struct {
		qdf_atomic_t rsrc_cnt;
		/* threshold_lo - when to start tx desc margin replenishment */
		uint16_t rsrc_threshold_lo;
		/* threshold_hi - where to stop during tx desc margin
		   replenishment */
		uint16_t rsrc_threshold_hi;
	} tx_queue;

#if defined(DEBUG_HL_LOGGING) && defined(CONFIG_HL_SUPPORT)
#define OL_TXQ_LOG_SIZE 512
	qdf_spinlock_t txq_log_spinlock;
	struct {
		int size;
		int oldest_record_offset;
		int offset;
		int allow_wrap;
		u_int32_t wrapped;
		/* aligned to u_int32_t boundary */
		u_int8_t data[OL_TXQ_LOG_SIZE];
	} txq_log;
#endif

#ifdef QCA_ENABLE_OL_TXRX_PEER_STATS
	qdf_spinlock_t peer_stat_mutex;
#endif

	int rssi_update_shift;
	int rssi_new_weight;
#ifdef QCA_SUPPORT_TXRX_LOCAL_PEER_ID
	struct {
		ol_txrx_local_peer_id_t pool[OL_TXRX_NUM_LOCAL_PEER_IDS + 1];
		ol_txrx_local_peer_id_t freelist;
		qdf_spinlock_t lock;
		ol_txrx_peer_handle map[OL_TXRX_NUM_LOCAL_PEER_IDS];
	} local_peer_ids;
#endif

#ifdef QCA_COMPUTE_TX_DELAY
#ifdef QCA_COMPUTE_TX_DELAY_PER_TID
#define QCA_TX_DELAY_NUM_CATEGORIES \
	(OL_TX_NUM_TIDS + OL_TX_VDEV_NUM_QUEUES)
#else
#define QCA_TX_DELAY_NUM_CATEGORIES 1
#endif
	struct {
		qdf_spinlock_t mutex;
		struct {
			struct ol_tx_delay_data copies[2]; /* ping-pong */
			int in_progress_idx;
			uint32_t avg_start_time_ticks;
		} cats[QCA_TX_DELAY_NUM_CATEGORIES];
		uint32_t tx_compl_timestamp_ticks;
		uint32_t avg_period_ticks;
		uint32_t hist_internal_bin_width_mult;
		uint32_t hist_internal_bin_width_shift;
	} tx_delay;

	uint16_t packet_count[QCA_TX_DELAY_NUM_CATEGORIES];
	uint16_t packet_loss_count[QCA_TX_DELAY_NUM_CATEGORIES];

#endif /* QCA_COMPUTE_TX_DELAY */

	struct {
		qdf_spinlock_t mutex;
		/* timer used to monitor the throttle "on" phase and
		   "off" phase */
		qdf_timer_t phase_timer;
		/* timer used to send tx frames */
		qdf_timer_t tx_timer;
		/* This is the time in ms of the throttling window, it will
		 * include an "on" phase and an "off" phase */
		uint32_t throttle_period_ms;
		/* Current throttle level set by the client ex. level 0,
		   level 1, etc */
		enum throttle_level current_throttle_level;
		/* Index that points to the phase within the throttle period */
		enum throttle_phase current_throttle_phase;
		/* Maximum number of frames to send to the target at one time */
		uint32_t tx_threshold;
		/* stores time in ms of on/off phase for each throttle level */
		int throttle_time_ms[THROTTLE_LEVEL_MAX][THROTTLE_PHASE_MAX];
		/* mark true if traffic is paused due to thermal throttling */
		bool is_paused;
	} tx_throttle;

#ifdef IPA_OFFLOAD
	ipa_uc_op_cb_type ipa_uc_op_cb;
	void *osif_dev;
#endif /* IPA_UC_OFFLOAD */

#if defined(FEATURE_TSO)
	struct {
		uint16_t pool_size;
		uint16_t num_free;
		struct qdf_tso_seg_elem_t *freelist;
		/* tso mutex */
		OL_TX_MUTEX_TYPE tso_mutex;
	} tso_seg_pool;
#endif

#if defined(CONFIG_HL_SUPPORT) && defined(QCA_BAD_PEER_TX_FLOW_CL)
	struct {
		enum ol_tx_peer_bal_state enabled;
		qdf_spinlock_t mutex;
		/* timer used to trigger more frames for bad peers */
		qdf_timer_t peer_bal_timer;
		/*This is the time in ms of the peer balance timer period */
		u_int32_t peer_bal_period_ms;
		/*This is the txq limit */
		u_int32_t peer_bal_txq_limit;
		/*This is the state of the peer balance timer */
		enum ol_tx_peer_bal_timer_state peer_bal_timer_state;
		/*This is the counter about active peers which are under
		 *tx flow control */
		u_int32_t peer_num;
		/*This is peer list which are under tx flow control */
		struct ol_tx_limit_peer_t limit_list[MAX_NO_PEERS_IN_LIMIT];
		/*This is threshold configurationl */
		struct tx_peer_threshold ctl_thresh[TXRX_IEEE11_MAX];
	} tx_peer_bal;
#endif /* CONFIG_Hl_SUPPORT && QCA_BAD_PEER_TX_FLOW_CL */

	struct ol_tx_queue_group_t txq_grps[OL_TX_MAX_TXQ_GROUPS];
#ifdef DEBUG_HL_LOGGING
		qdf_spinlock_t grp_stat_spinlock;
		struct ol_tx_group_credit_stats_t grp_stats;
#endif
	int tid_to_ac[OL_TX_NUM_TIDS + OL_TX_VDEV_NUM_QUEUES];
	uint8_t ocb_peer_valid;
	struct ol_txrx_peer_t *ocb_peer;
	ol_tx_pause_callback_fp pause_cb;

	struct {
		void *lro_data;
		void (*lro_flush_cb)(void *);
	} lro_info;
	struct ol_txrx_peer_t *self_peer;
};

struct ol_txrx_ocb_chan_info {
	uint32_t chan_freq;
	uint16_t disable_rx_stats_hdr:1;
};

struct ol_txrx_vdev_t {
	struct ol_txrx_pdev_t *pdev; /* pdev - the physical device that is
					the parent of this virtual device */
	uint8_t vdev_id;             /* ID used to specify a particular vdev
					to the target */
	void *osif_dev;
	union ol_txrx_align_mac_addr_t mac_addr; /* MAC address */
	/* tx paused - NO LONGER NEEDED? */
	TAILQ_ENTRY(ol_txrx_vdev_t) vdev_list_elem; /* node in the pdev's list
						       of vdevs */
	TAILQ_HEAD(peer_list_t, ol_txrx_peer_t) peer_list;
	struct ol_txrx_peer_t *last_real_peer; /* last real peer created for
						  this vdev (not "self"
						  pseudo-peer) */
	ol_txrx_rx_fp rx; /* receive function used by this vdev */

	struct {
		/*
		 * If the vdev object couldn't be deleted immediately because
		 * it still had some peer objects left, remember that a delete
		 * was requested, so it can be deleted once all its peers have
		 * been deleted.
		 */
		int pending;
		/*
		 * Store a function pointer and a context argument to provide a
		 * notification for when the vdev is deleted.
		 */
		ol_txrx_vdev_delete_cb callback;
		void *context;
	} delete;

	/* safe mode control to bypass the encrypt and decipher process */
	uint32_t safemode;

	/* rx filter related */
	uint32_t drop_unenc;
	struct privacy_exemption privacy_filters[MAX_PRIVACY_FILTERS];
	uint32_t num_filters;

	enum wlan_op_mode opmode;

#ifdef QCA_IBSS_SUPPORT
	/* ibss mode related */
	int16_t ibss_peer_num;  /* the number of active peers */
	int16_t ibss_peer_heart_beat_timer; /* for detecting peer departure */
#endif

#if defined(CONFIG_HL_SUPPORT)
	struct ol_tx_frms_queue_t txqs[OL_TX_VDEV_NUM_QUEUES];
#endif

	struct {
		struct {
			qdf_nbuf_t head;
			qdf_nbuf_t tail;
			int depth;
		} txq;
		uint32_t paused_reason;
		qdf_spinlock_t mutex;
		qdf_timer_t timer;
		int max_q_depth;
		bool is_q_paused;
		bool is_q_timer_on;
		uint32_t q_pause_cnt;
		uint32_t q_unpause_cnt;
		uint32_t q_overflow_cnt;
	} ll_pause;
	bool disable_intrabss_fwd;
	qdf_atomic_t os_q_paused;
	uint16_t tx_fl_lwm;
	uint16_t tx_fl_hwm;
	qdf_spinlock_t flow_control_lock;
	ol_txrx_tx_flow_control_fp osif_flow_control_cb;
	void *osif_fc_ctx;

#if defined(CONFIG_HL_SUPPORT) && defined(FEATURE_WLAN_TDLS)
	union ol_txrx_align_mac_addr_t hl_tdls_ap_mac_addr;
	bool hlTdlsFlag;
#endif

#if defined(CONFIG_PER_VDEV_TX_DESC_POOL)
	qdf_atomic_t tx_desc_count;
#endif

	uint16_t wait_on_peer_id;
	qdf_event_t wait_delete_comp;
#if defined(FEATURE_TSO)
	struct {
		int pool_elems; /* total number of elements in the pool */
		int alloc_cnt; /* number of allocated elements */
		uint32_t *freelist; /* free list of qdf_tso_seg_elem_t */
	} tso_pool_t;
#endif

	/* last channel change event recieved */
	struct {
		bool is_valid;  /* whether the rest of the members are valid */
		uint16_t mhz;
		uint16_t band_center_freq1;
		uint16_t band_center_freq2;
		WLAN_PHY_MODE phy_mode;
	} ocb_channel_event;

	/* Information about the schedules in the schedule */
	struct ol_txrx_ocb_chan_info *ocb_channel_info;
	uint32_t ocb_channel_count;

#ifdef QCA_LL_TX_FLOW_CONTROL_V2
	struct ol_tx_flow_pool_t *pool;
#endif
	/* intra bss forwarded tx and rx packets count */
	uint64_t fwd_tx_packets;
	uint64_t fwd_rx_packets;
	bool is_wisa_mode_enable;
	uint8_t mac_id;
};

struct ol_rx_reorder_array_elem_t {
	qdf_nbuf_t head;
	qdf_nbuf_t tail;
};

struct ol_rx_reorder_t {
	uint8_t win_sz;
	uint8_t win_sz_mask;
	uint8_t num_mpdus;
	struct ol_rx_reorder_array_elem_t *array;
	/* base - single rx reorder element used for non-aggr cases */
	struct ol_rx_reorder_array_elem_t base;
#if defined(QCA_SUPPORT_OL_RX_REORDER_TIMEOUT)
	struct ol_rx_reorder_timeout_list_elem_t timeout;
#endif
	/* only used for defrag right now */
	TAILQ_ENTRY(ol_rx_reorder_t) defrag_waitlist_elem;
	uint32_t defrag_timeout_ms;
	/* get back to parent ol_txrx_peer_t when ol_rx_reorder_t is in a
	 * waitlist */
	uint16_t tid;
};

enum {
	txrx_sec_mcast = 0,
	txrx_sec_ucast
};

typedef A_STATUS (*ol_tx_filter_func)(struct ol_txrx_msdu_info_t *
				      tx_msdu_info);

#define OL_TXRX_PEER_SECURITY_MULTICAST  0
#define OL_TXRX_PEER_SECURITY_UNICAST    1
#define OL_TXRX_PEER_SECURITY_MAX        2

struct ol_txrx_peer_t {
	struct ol_txrx_vdev_t *vdev;

	qdf_atomic_t ref_cnt;
	qdf_atomic_t delete_in_progress;
	qdf_atomic_t flush_in_progress;

	/* The peer state tracking is used for HL systems
	 * that don't support tx and rx filtering within the target.
	 * In such systems, the peer's state determines what kind of
	 * tx and rx filtering, if any, is done.
	 * This variable doesn't apply to LL systems, or to HL systems for
	 * which the target handles tx and rx filtering. However, it is
	 * simplest to declare and update this variable unconditionally,
	 * for all systems.
	 */
	enum ol_txrx_peer_state state;
	qdf_spinlock_t peer_info_lock;
	qdf_spinlock_t bufq_lock;
	struct list_head cached_bufq;

	ol_tx_filter_func tx_filter;

	/* peer ID(s) for this peer */
	uint16_t peer_ids[MAX_NUM_PEER_ID_PER_PEER];
#ifdef QCA_SUPPORT_TXRX_LOCAL_PEER_ID
	uint16_t local_id;
#endif

	union ol_txrx_align_mac_addr_t mac_addr;

	/* node in the vdev's list of peers */
	TAILQ_ENTRY(ol_txrx_peer_t) peer_list_elem;
	/* node in the hash table bin's list of peers */
	TAILQ_ENTRY(ol_txrx_peer_t) hash_list_elem;

	/*
	 * per TID info -
	 * stored in separate arrays to avoid alignment padding mem overhead
	 */
	struct ol_rx_reorder_t tids_rx_reorder[OL_TXRX_NUM_EXT_TIDS];
	union htt_rx_pn_t tids_last_pn[OL_TXRX_NUM_EXT_TIDS];
	uint8_t tids_last_pn_valid[OL_TXRX_NUM_EXT_TIDS];
	uint16_t tids_next_rel_idx[OL_TXRX_NUM_EXT_TIDS];
	uint16_t tids_last_seq[OL_TXRX_NUM_EXT_TIDS];
	uint16_t tids_mcast_last_seq[OL_TXRX_NUM_EXT_TIDS];

	struct {
		enum htt_sec_type sec_type;
		uint32_t michael_key[2];        /* relevant for TKIP */
	} security[2];          /* 0 -> multicast, 1 -> unicast */

	/*
	 * rx proc function: this either is a copy of pdev's rx_opt_proc for
	 * regular rx processing, or has been redirected to a /dev/null discard
	 * function when peer deletion is in progress.
	 */
	void (*rx_opt_proc)(struct ol_txrx_vdev_t *vdev,
			    struct ol_txrx_peer_t *peer,
			    unsigned tid, qdf_nbuf_t msdu_list);

#if defined(CONFIG_HL_SUPPORT)
	struct ol_tx_frms_queue_t txqs[OL_TX_NUM_TIDS];
#endif

#ifdef QCA_ENABLE_OL_TXRX_PEER_STATS
	ol_txrx_peer_stats_t stats;
#endif
	int16_t rssi_dbm;

	/* NAWDS Flag and Bss Peer bit */
	uint16_t nawds_enabled:1, bss_peer:1, valid:1;

	/* QoS info */
	uint8_t qos_capable;
	/* U-APSD tid mask */
	uint8_t uapsd_mask;
	/*flag indicating key installed */
	uint8_t keyinstalled;

	/* Bit to indicate if PN check is done in fw */
	qdf_atomic_t fw_pn_check;

#ifdef WLAN_FEATURE_11W
	/* PN counter for Robust Management Frames */
	uint64_t last_rmf_pn;
	uint32_t rmf_pn_replays;
	uint8_t last_rmf_pn_valid;
#endif

	/* Properties of the last received PPDU */
	int16_t last_pkt_rssi_cmb;
	int16_t last_pkt_rssi[4];
	uint8_t last_pkt_legacy_rate;
	uint8_t last_pkt_legacy_rate_sel;
	uint32_t last_pkt_timestamp_microsec;
	uint8_t last_pkt_timestamp_submicrosec;
	uint32_t last_pkt_tsf;
	uint8_t last_pkt_tid;
	uint16_t last_pkt_center_freq;
#if defined(CONFIG_HL_SUPPORT) && defined(QCA_BAD_PEER_TX_FLOW_CL)
	u_int16_t tx_limit;
	u_int16_t tx_limit_flag;
	u_int16_t tx_pause_flag;
#endif
	qdf_time_t last_assoc_rcvd;
	qdf_time_t last_disassoc_rcvd;
	qdf_time_t last_deauth_rcvd;
	qdf_atomic_t exists_in_fw;
};

enum ol_rx_err_type {
	OL_RX_ERR_DEFRAG_MIC,
	OL_RX_ERR_PN,
	OL_RX_ERR_UNKNOWN_PEER,
	OL_RX_ERR_MALFORMED,
	OL_RX_ERR_TKIP_MIC,
	OL_RX_ERR_DECRYPT,
	OL_RX_ERR_MPDU_LENGTH,
	OL_RX_ERR_ENCRYPT_REQUIRED,
	OL_RX_ERR_DUP,
	OL_RX_ERR_UNKNOWN,
	OL_RX_ERR_FCS,
	OL_RX_ERR_PRIVACY,
	OL_RX_ERR_NONE_FRAG,
	OL_RX_ERR_NONE = 0xFF
};

/**
 * ol_mic_error_info - carries the information associated with
 * a MIC error
 * @vdev_id: virtual device ID
 * @key_id: Key ID
 * @pn: packet number
 * @sa: source address
 * @da: destination address
 * @ta: transmitter address
 */
struct ol_mic_error_info {
	uint8_t vdev_id;
	uint32_t key_id;
	uint64_t pn;
	uint8_t sa[OL_TXRX_MAC_ADDR_LEN];
	uint8_t da[OL_TXRX_MAC_ADDR_LEN];
	uint8_t ta[OL_TXRX_MAC_ADDR_LEN];
};

/**
 * ol_error_info - carries the information associated with an
 * error indicated by the firmware
 * @mic_err: MIC error information
 */
struct ol_error_info {
	union {
		struct ol_mic_error_info mic_err;
	} u;
};

struct ol_rx_remote_data {
	qdf_nbuf_t msdu;
	uint8_t mac_id;
};

#endif /* _OL_TXRX_TYPES__H_ */
