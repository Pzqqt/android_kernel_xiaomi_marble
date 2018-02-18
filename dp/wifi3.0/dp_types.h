/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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

#ifndef _DP_TYPES_H_
#define _DP_TYPES_H_

#include <qdf_types.h>
#include <qdf_nbuf.h>
#include <qdf_lock.h>
#include <qdf_atomic.h>
#include <qdf_util.h>
#include <qdf_list.h>
#include <qdf_lro.h>
#include <queue.h>
#include <htt_common.h>

#include <cdp_txrx_cmn.h>
#ifdef CONFIG_MCL
#include <cds_ieee80211_common.h>
#else
#include <linux/ieee80211.h>
#endif

#ifndef CONFIG_WIN
#include <wdi_event_api.h>    /* WDI subscriber event list */
#endif

#include <hal_tx.h>
#include <hal_reo.h>
#include "wlan_cfg.h"
#include "hal_rx.h"
#include <hal_api.h>
#include <hal_api_mon.h>
#include "hal_rx.h"

#define MAX_BW 7
#define MAX_RETRIES 4
#define MAX_RECEPTION_TYPES 4

#ifndef REMOVE_PKT_LOG
#include <pktlog.h>
#endif

#define REPT_MU_MIMO 1
#define REPT_MU_OFDMA_MIMO 3
#define DP_VO_TID 6

#define DP_MAX_INTERRUPT_CONTEXTS 8
#define DP_MAX_TID_MAPS 16 /* MAX TID MAPS AVAILABLE PER PDEV*/
#define DSCP_TID_MAP_MAX    (64)
#define DP_IP_DSCP_SHIFT 2
#define DP_IP_DSCP_MASK 0x3f
#define DP_FC0_SUBTYPE_QOS 0x80
#define DP_QOS_TID 0x0f
#define DP_IPV6_PRIORITY_SHIFT 20
#define MAX_MON_LINK_DESC_BANKS 2

#if defined(CONFIG_MCL)
#define MAX_PDEV_CNT 1
#else
#define MAX_PDEV_CNT 3
#endif

#define MAX_LINK_DESC_BANKS 8
#define MAX_TXDESC_POOLS 4
#define MAX_RXDESC_POOLS 4
#define MAX_REO_DEST_RINGS 4
#define MAX_TCL_DATA_RINGS 4
#define MAX_IDLE_SCATTER_BUFS 16
#define DP_MAX_IRQ_PER_CONTEXT 12
#define DP_MAX_INTERRUPT_CONTEXTS 8
#define DEFAULT_HW_PEER_ID 0xffff

#define MAX_TX_HW_QUEUES MAX_TCL_DATA_RINGS

#define DP_MAX_INTERRUPT_CONTEXTS 8

#ifndef REMOVE_PKT_LOG
enum rx_pktlog_mode {
	DP_RX_PKTLOG_DISABLED = 0,
	DP_RX_PKTLOG_FULL,
	DP_RX_PKTLOG_LITE,
};
#endif

struct dp_soc_cmn;
struct dp_pdev;
struct dp_vdev;
struct dp_tx_desc_s;
struct dp_soc;
union dp_rx_desc_list_elem_t;

#define DP_PDEV_ITERATE_VDEV_LIST(_pdev, _vdev) \
	TAILQ_FOREACH((_vdev), &(_pdev)->vdev_list, vdev_list_elem)

#define DP_VDEV_ITERATE_PEER_LIST(_vdev, _peer) \
	TAILQ_FOREACH((_peer), &(_vdev)->peer_list, peer_list_elem)

#define DP_PEER_ITERATE_ASE_LIST(_peer, _ase, _temp_ase) \
	TAILQ_FOREACH_SAFE((_ase), &peer->ast_entry_list, ase_list_elem, (_temp_ase))

#define DP_MUTEX_TYPE qdf_spinlock_t

#define DP_FRAME_IS_MULTICAST(_a)  (*(_a) & 0x01)
#define DP_FRAME_IS_IPV4_MULTICAST(_a)  (*(_a) == 0x01)

#define DP_FRAME_IS_IPV6_MULTICAST(_a)         \
    ((_a)[0] == 0x33 &&                         \
     (_a)[1] == 0x33)

#define DP_FRAME_IS_BROADCAST(_a)              \
    ((_a)[0] == 0xff &&                         \
     (_a)[1] == 0xff &&                         \
     (_a)[2] == 0xff &&                         \
     (_a)[3] == 0xff &&                         \
     (_a)[4] == 0xff &&                         \
     (_a)[5] == 0xff)
#define IS_LLC_PRESENT(typeorlen) ((typeorlen) >= 0x600)
#define DP_FRAME_IS_SNAP(_llc) ((_llc)->llc_dsap == 0xaa && \
		(_llc)->llc_ssap == 0xaa && \
		(_llc)->llc_un.type_snap.control == 0x3)
#define DP_FRAME_FC0_TYPE_MASK 0x0c
#define DP_FRAME_FC0_TYPE_DATA 0x08
#define DP_FRAME_IS_DATA(_frame) \
	(((_frame)->i_fc[0] & DP_FRAME_FC0_TYPE_MASK) == DP_FRAME_FC0_TYPE_DATA)

/**
 * macros to convert hw mac id to sw mac id:
 * mac ids used by hardware start from a value of 1 while
 * those in host software start from a value of 0. Use the
 * macros below to convert between mac ids used by software and
 * hardware
 */
#define DP_SW2HW_MACID(id) ((id) + 1)
#define DP_HW2SW_MACID(id) ((id) > 0 ? ((id) - 1) : 0)
#define DP_MAC_ADDR_LEN 6

/**
 * enum dp_intr_mode
 * @DP_INTR_LEGACY: Legacy/Line interrupts, for WIN
 * @DP_INTR_MSI: MSI interrupts, for MCL
 * @DP_INTR_POLL: Polling
 */
enum dp_intr_mode {
	DP_INTR_LEGACY = 0,
	DP_INTR_MSI,
	DP_INTR_POLL,
};

/**
 * enum dp_tx_frm_type
 * @dp_tx_frm_std: Regular frame, no added header fragments
 * @dp_tx_frm_tso: TSO segment, with a modified IP header added
 * @dp_tx_frm_sg: SG segment
 * @dp_tx_frm_audio: Audio frames, a custom LLC/SNAP header added
 * @dp_tx_frm_me: Multicast to Unicast Converted frame
 * @dp_tx_frm_raw: Raw Frame
 */
enum dp_tx_frm_type {
	dp_tx_frm_std = 0,
	dp_tx_frm_tso,
	dp_tx_frm_sg,
	dp_tx_frm_audio,
	dp_tx_frm_me,
	dp_tx_frm_raw,
};

/**
 * enum dp_ast_type
 * @dp_ast_type_wds: WDS peer AST type
 * @dp_ast_type_static: static ast entry type
 * @dp_ast_type_mec: Multicast echo ast entry type
 */
enum dp_ast_type {
	dp_ast_type_wds = 0,
	dp_ast_type_static,
	dp_ast_type_mec,
};

/**
 * enum dp_nss_cfg
 * @dp_nss_cfg_default: No radios are offloaded
 * @dp_nss_cfg_first_radio: First radio offloaded
 * @dp_nss_cfg_second_radio: Second radio offloaded
 * @dp_nss_cfg_dbdc: Dual radios offloaded
 */
enum dp_nss_cfg {
	dp_nss_cfg_default,
	dp_nss_cfg_first_radio,
	dp_nss_cfg_second_radio,
	dp_nss_cfg_dbdc,
};

/**
 * struct rx_desc_pool
 * @pool_size: number of RX descriptor in the pool
 * @array: pointer to array of RX descriptor
 * @freelist: pointer to free RX descriptor link list
 */
struct rx_desc_pool {
	uint32_t pool_size;
	union dp_rx_desc_list_elem_t *array;
	union dp_rx_desc_list_elem_t *freelist;
};

/**
 * struct dp_tx_ext_desc_elem_s
 * @next: next extension descriptor pointer
 * @vaddr: hlos virtual address pointer
 * @paddr: physical address pointer for descriptor
 */
struct dp_tx_ext_desc_elem_s {
	struct dp_tx_ext_desc_elem_s *next;
	void *vaddr;
	qdf_dma_addr_t paddr;
};

/**
 * struct dp_tx_ext_desc_s - Tx Extension Descriptor Pool
 * @elem_count: Number of descriptors in the pool
 * @elem_size: Size of each descriptor
 * @num_free: Number of free descriptors
 * @msdu_ext_desc: MSDU extension descriptor
 * @desc_pages: multiple page allocation information for actual descriptors
 * @link_elem_size: size of the link descriptor in cacheable memory used for
 * 		    chaining the extension descriptors
 * @desc_link_pages: multiple page allocation information for link descriptors
 */
struct dp_tx_ext_desc_pool_s {
	uint16_t elem_count;
	int elem_size;
	uint16_t num_free;
	struct qdf_mem_multi_page_t desc_pages;
	int link_elem_size;
	struct qdf_mem_multi_page_t desc_link_pages;
	struct dp_tx_ext_desc_elem_s *freelist;
	qdf_spinlock_t lock;
	qdf_dma_mem_context(memctx);
};

/**
 * struct dp_tx_desc_s - Tx Descriptor
 * @next: Next in the chain of descriptors in freelist or in the completion list
 * @nbuf: Buffer Address
 * @msdu_ext_desc: MSDU extension descriptor
 * @id: Descriptor ID
 * @vdev: vdev over which the packet was transmitted
 * @pdev: Handle to pdev
 * @pool_id: Pool ID - used when releasing the descriptor
 * @flags: Flags to track the state of descriptor and special frame handling
 * @comp: Pool ID - used when releasing the descriptor
 * @tx_encap_type: Transmit encap type (i.e. Raw, Native Wi-Fi, Ethernet).
 * 		   This is maintained in descriptor to allow more efficient
 * 		   processing in completion event processing code.
 * 		    This field is filled in with the htt_pkt_type enum.
 * @frm_type: Frame Type - ToDo check if this is redundant
 * @pkt_offset: Offset from which the actual packet data starts
 * @me_buffer: Pointer to ME buffer - store this so that it can be freed on
 *		Tx completion of ME packet
 * @pool: handle to flow_pool this descriptor belongs to.
 */
struct dp_tx_desc_s {
	struct dp_tx_desc_s *next;
	qdf_nbuf_t nbuf;
	struct dp_tx_ext_desc_elem_s *msdu_ext_desc;
	uint32_t  id;
	struct dp_vdev *vdev;
	struct dp_pdev *pdev;
	uint8_t  pool_id;
	uint16_t flags;
	struct hal_tx_desc_comp_s comp;
	uint16_t tx_encap_type;
	uint8_t frm_type;
	uint8_t pkt_offset;
	void *me_buffer;
	void *tso_desc;
	void *tso_num_desc;
};

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
 * struct dp_tx_tso_seg_pool_s
 * @pool_size: total number of pool elements
 * @num_free: free element count
 * @freelist: first free element pointer
 * @lock: lock for accessing the pool
 */
struct dp_tx_tso_seg_pool_s {
	uint16_t pool_size;
	uint16_t num_free;
	struct qdf_tso_seg_elem_t *freelist;
	qdf_spinlock_t lock;
};

/**
 * struct dp_tx_tso_num_seg_pool_s {
 * @num_seg_pool_size: total number of pool elements
 * @num_free: free element count
 * @freelist: first free element pointer
 * @lock: lock for accessing the pool
 */

struct dp_tx_tso_num_seg_pool_s {
	uint16_t num_seg_pool_size;
	uint16_t num_free;
	struct qdf_tso_num_seg_elem_t *freelist;
	/*tso mutex */
	qdf_spinlock_t lock;
};

/**
 * struct dp_tx_desc_pool_s - Tx Descriptor pool information
 * @elem_size: Size of each descriptor in the pool
 * @pool_size: Total number of descriptors in the pool
 * @num_free: Number of free descriptors
 * @num_allocated: Number of used descriptors
 * @freelist: Chain of free descriptors
 * @desc_pages: multiple page allocation information for actual descriptors
 * @num_invalid_bin: Deleted pool with pending Tx completions.
 * @flow_pool_array_lock: Lock when operating on flow_pool_array.
 * @flow_pool_array: List of allocated flow pools
 * @lock- Lock for descriptor allocation/free from/to the pool
 */
struct dp_tx_desc_pool_s {
	uint16_t elem_size;
	uint32_t num_allocated;
	struct dp_tx_desc_s *freelist;
	struct qdf_mem_multi_page_t desc_pages;
#ifdef QCA_LL_TX_FLOW_CONTROL_V2
	uint16_t pool_size;
	uint8_t flow_pool_id;
	uint8_t num_invalid_bin;
	uint16_t avail_desc;
	enum flow_pool_status status;
	enum htt_flow_type flow_type;
	uint16_t stop_th;
	uint16_t start_th;
	uint16_t pkt_drop_no_desc;
	qdf_spinlock_t flow_pool_lock;
	void *pool_owner_ctx;
#else
	uint16_t elem_count;
	uint32_t num_free;
	qdf_spinlock_t lock;
#endif
};

/**
 * struct dp_txrx_pool_stats - flow pool related statistics
 * @pool_map_count: flow pool map received
 * @pool_unmap_count: flow pool unmap received
 * @pkt_drop_no_pool: packets dropped due to unavailablity of pool
 */
struct dp_txrx_pool_stats {
	uint16_t pool_map_count;
	uint16_t pool_unmap_count;
	uint16_t pkt_drop_no_pool;
};

struct dp_srng {
	void *hal_srng;
	void *base_vaddr_unaligned;
	qdf_dma_addr_t base_paddr_unaligned;
	uint32_t alloc_size;
	int irq;
	uint32_t num_entries;
};

struct dp_rx_reorder_array_elem {
	qdf_nbuf_t head;
	qdf_nbuf_t tail;
};

#define DP_RX_BA_INACTIVE 0
#define DP_RX_BA_ACTIVE 1
struct dp_reo_cmd_info {
	uint16_t cmd;
	enum hal_reo_cmd_type cmd_type;
	void *data;
	void (*handler)(struct dp_soc *, void *, union hal_reo_status *);
	TAILQ_ENTRY(dp_reo_cmd_info) reo_cmd_list_elem;
};

/* Rx TID */
struct dp_rx_tid {
	/* TID */
	int tid;

	/* Num of addba requests */
	uint32_t num_of_addba_req;

	/* Num of addba responses */
	uint32_t num_of_addba_resp;

	/* Num of delba requests */
	uint32_t num_of_delba_req;

	/* pn size */
	uint8_t pn_size;
	/* REO TID queue descriptors */
	void *hw_qdesc_vaddr_unaligned;
	qdf_dma_addr_t hw_qdesc_paddr_unaligned;
	qdf_dma_addr_t hw_qdesc_paddr;
	uint32_t hw_qdesc_alloc_size;

	/* RX ADDBA session state */
	int ba_status;

	/* RX BA window size */
	uint16_t ba_win_size;

	/* TODO: Check the following while adding defragmentation support */
	struct dp_rx_reorder_array_elem *array;
	/* base - single rx reorder element used for non-aggr cases */
	struct dp_rx_reorder_array_elem base;

	/* only used for defrag right now */
	TAILQ_ENTRY(dp_rx_tid) defrag_waitlist_elem;

	/* Store dst desc for reinjection */
	void *dst_ring_desc;

	/* Sequence and fragments that are being processed currently */
	uint32_t curr_seq_num;
	uint32_t curr_frag_num;

	uint32_t defrag_timeout_ms;
	uint16_t dialogtoken;
	uint16_t statuscode;
	/* user defined ADDBA response status code */
	uint16_t userstatuscode;
};

/* per interrupt context  */
struct dp_intr {
	uint8_t tx_ring_mask;   /* WBM Tx completion rings (0-2)
				associated with this napi context */
	uint8_t rx_ring_mask;   /* Rx REO rings (0-3) associated
				with this interrupt context */
	uint8_t rx_mon_ring_mask;  /* Rx monitor ring mask (0-2) */
	uint8_t rx_err_ring_mask; /* REO Exception Ring */
	uint8_t rx_wbm_rel_ring_mask; /* WBM2SW Rx Release Ring */
	uint8_t reo_status_ring_mask; /* REO command response ring */
	uint8_t rxdma2host_ring_mask; /* RXDMA to host destination ring */
	uint8_t host2rxdma_ring_mask; /* Host to RXDMA buffer ring */
	struct dp_soc *soc;    /* Reference to SoC structure ,
				to get DMA ring handles */
	qdf_lro_ctx_t lro_ctx;
	uint8_t dp_intr_id;
};

#define REO_DESC_FREELIST_SIZE 64
#define REO_DESC_FREE_DEFER_MS 1000
struct reo_desc_list_node {
	qdf_list_node_t node;
	unsigned long free_ts;
	struct dp_rx_tid rx_tid;
};

/* SoC level data path statistics */
struct dp_soc_stats {
	struct {
		uint32_t added;
		uint32_t deleted;
		uint32_t aged_out;
	} ast;

	/* SOC level TX stats */
	struct {
		/* packets dropped on tx because of no peer */
		struct cdp_pkt_info tx_invalid_peer;
		/* descriptors in each tcl ring */
		uint32_t tcl_ring_full[MAX_TCL_DATA_RINGS];
		/* Descriptors in use at soc */
		uint32_t desc_in_use;
		/* tqm_release_reason == FW removed */
		uint32_t dropped_fw_removed;

	} tx;

	/* SOC level RX stats */
	struct {
		/* Rx errors */
		/* Total Packets in Rx Error ring */
		uint32_t err_ring_pkts;
		/* No of Fragments */
		uint32_t rx_frags;
		struct {
			/* Invalid RBM error count */
			uint32_t invalid_rbm;
			/* Invalid VDEV Error count */
			uint32_t invalid_vdev;
			/* Invalid PDEV error count */
			uint32_t invalid_pdev;
			/* Invalid PEER Error count */
			struct cdp_pkt_info rx_invalid_peer;
			/* HAL ring access Fail error count */
			uint32_t hal_ring_access_fail;
			/* RX DMA error count */
			uint32_t rxdma_error[HAL_RXDMA_ERR_MAX];
			/* REO Error count */
			uint32_t reo_error[HAL_REO_ERR_MAX];
			/* HAL REO ERR Count */
			uint32_t hal_reo_error[MAX_REO_DEST_RINGS];
		} err;

		/* packet count per core - per ring */
		uint64_t ring_packets[NR_CPUS][MAX_REO_DEST_RINGS];
	} rx;
};

#define DP_MAC_ADDR_LEN 6
union dp_align_mac_addr {
	uint8_t raw[DP_MAC_ADDR_LEN];
	struct {
		uint16_t bytes_ab;
		uint16_t bytes_cd;
		uint16_t bytes_ef;
	} align2;
	struct {
		uint32_t bytes_abcd;
		uint16_t bytes_ef;
	} align4;
	struct {
		uint16_t bytes_ab;
		uint32_t bytes_cdef;
	} align4_2;
};

/*
 * dp_ast_entry
 *
 * @ast_idx: Hardware AST Index
 * @mac_addr:  MAC Address for this AST entry
 * @peer: Next Hop peer (for non-WDS nodes, this will be point to
 *        associated peer with this MAC address)
 * @next_hop: Set to 1 if this is for a WDS node
 * @is_active: flag to indicate active data traffic on this node
 *             (used for aging out/expiry)
 * @ase_list_elem: node in peer AST list
 * @is_bss: flag to indicate if entry corresponds to bss peer
 * @pdev_id: pdev ID
 * @vdev_id: vdev ID
 * @type: flag to indicate type of the entry(static/WDS/MEC)
 * @hash_list_elem: node in soc AST hash list (mac address used as hash)
 */
struct dp_ast_entry {
	uint16_t ast_idx;
	/* MAC address */
	union dp_align_mac_addr mac_addr;
	struct dp_peer *peer;
	bool next_hop;
	bool is_active;
	bool is_bss;
	uint8_t pdev_id;
	uint8_t vdev_id;
	enum cdp_txrx_ast_entry_type type;
	TAILQ_ENTRY(dp_ast_entry) ase_list_elem;
	TAILQ_ENTRY(dp_ast_entry) hash_list_elem;
};

/* SOC level htt stats */
struct htt_t2h_stats {
	/* lock to protect htt_stats_msg update */
	qdf_spinlock_t lock;

	/* work queue to process htt stats */
	qdf_work_t work;

	/* T2H Ext stats message queue */
	qdf_nbuf_queue_t msg;

	/* number of completed stats in htt_stats_msg */
	uint32_t num_stats;
};

/* SOC level structure for data path */
struct dp_soc {
	/* Common base structure - Should be the first member */
	struct cdp_soc_t cdp_soc;

	/* SoC Obj */
	void *ctrl_psoc;

	/* OS device abstraction */
	qdf_device_t osdev;

	/* WLAN config context */
	struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx;

	/* HTT handle for host-fw interaction */
	void *htt_handle;

	/* Commint init done */
	qdf_atomic_t cmn_init_done;

	/* Opaque hif handle */
	struct hif_opaque_softc *hif_handle;

	/* PDEVs on this SOC */
	struct dp_pdev *pdev_list[MAX_PDEV_CNT];

	/* Number of PDEVs */
	uint8_t pdev_count;

	/*cce disable*/
	bool cce_disable;

	/* Link descriptor memory banks */
	struct {
		void *base_vaddr_unaligned;
		void *base_vaddr;
		qdf_dma_addr_t base_paddr_unaligned;
		qdf_dma_addr_t base_paddr;
		uint32_t size;
	} link_desc_banks[MAX_LINK_DESC_BANKS];

	/* Link descriptor Idle list for HW internal use (SRNG mode) */
	struct dp_srng wbm_idle_link_ring;

	/* Link descriptor Idle list for HW internal use (scatter buffer mode)
	 */
	qdf_dma_addr_t wbm_idle_scatter_buf_base_paddr[MAX_IDLE_SCATTER_BUFS];
	void *wbm_idle_scatter_buf_base_vaddr[MAX_IDLE_SCATTER_BUFS];
	uint32_t wbm_idle_scatter_buf_size;

#ifdef QCA_LL_TX_FLOW_CONTROL_V2
	qdf_spinlock_t flow_pool_array_lock;
	tx_pause_callback pause_cb;
	struct dp_txrx_pool_stats pool_stats;
#endif /* !QCA_LL_TX_FLOW_CONTROL_V2 */
	/* Tx SW descriptor pool */
	struct dp_tx_desc_pool_s tx_desc[MAX_TXDESC_POOLS];

	/* Tx MSDU Extension descriptor pool */
	struct dp_tx_ext_desc_pool_s tx_ext_desc[MAX_TXDESC_POOLS];

	/* Tx TSO descriptor pool */
	struct dp_tx_tso_seg_pool_s tx_tso_desc[MAX_TXDESC_POOLS];

	/* Tx TSO Num of segments pool */
	struct dp_tx_tso_num_seg_pool_s tx_tso_num_seg[MAX_TXDESC_POOLS];

	/* Tx H/W queues lock */
	qdf_spinlock_t tx_queue_lock[MAX_TX_HW_QUEUES];

	/* Rx SW descriptor pool for RXDMA buffer */
	struct rx_desc_pool rx_desc_buf[MAX_RXDESC_POOLS];

	/* Rx SW descriptor pool for RXDMA monitor buffer */
	struct rx_desc_pool rx_desc_mon[MAX_RXDESC_POOLS];

	/* Rx SW descriptor pool for RXDMA status buffer */
	struct rx_desc_pool rx_desc_status[MAX_RXDESC_POOLS];

	/* DP rx desc lock */
	DP_MUTEX_TYPE rx_desc_mutex[MAX_RXDESC_POOLS];

	/* HAL SOC handle */
	void *hal_soc;

	/* DP Interrupts */
	struct dp_intr intr_ctx[DP_MAX_INTERRUPT_CONTEXTS];

	/* REO destination rings */
	struct dp_srng reo_dest_ring[MAX_REO_DEST_RINGS];

	/* Number of REO destination rings */
	uint8_t num_reo_dest_rings;

	/* REO exception ring - See if should combine this with reo_dest_ring */
	struct dp_srng reo_exception_ring;

	/* REO reinjection ring */
	struct dp_srng reo_reinject_ring;

	/* REO command ring */
	struct dp_srng reo_cmd_ring;

	/* REO command status ring */
	struct dp_srng reo_status_ring;

	/* WBM Rx release ring */
	struct dp_srng rx_rel_ring;

	/* Number of TCL data rings */
	uint8_t num_tcl_data_rings;

	/* TCL data ring */
	struct dp_srng tcl_data_ring[MAX_TCL_DATA_RINGS];

	/* TCL command ring */
	struct dp_srng tcl_cmd_ring;

	/* TCL command status ring */
	struct dp_srng tcl_status_ring;

	/* WBM Tx completion rings */
	struct dp_srng tx_comp_ring[MAX_TCL_DATA_RINGS];

	/* Common WBM link descriptor release ring (SW to WBM) */
	struct dp_srng wbm_desc_rel_ring;

	/* Tx ring map for interrupt processing */
	uint8_t tx_ring_map[WLAN_CFG_INT_NUM_CONTEXTS];

	/* Rx ring map for interrupt processing */
	uint8_t rx_ring_map[WLAN_CFG_INT_NUM_CONTEXTS];

	/* peer ID to peer object map (array of pointers to peer objects) */
	struct dp_peer **peer_id_to_obj_map;

	struct {
		unsigned mask;
		unsigned idx_bits;
		TAILQ_HEAD(, dp_peer) * bins;
	} peer_hash;

	/* rx defrag state – TBD: do we need this per radio? */
	struct {
		struct {
			TAILQ_HEAD(, dp_rx_tid) waitlist;
			uint32_t timeout_ms;
		} defrag;
		struct {
			int defrag_timeout_check;
			int dup_check;
		} flags;
		TAILQ_HEAD(, dp_reo_cmd_info) reo_cmd_list;
		qdf_spinlock_t reo_cmd_lock;
	} rx;

	/* optional rx processing function */
	void (*rx_opt_proc)(
		struct dp_vdev *vdev,
		struct dp_peer *peer,
		unsigned tid,
		qdf_nbuf_t msdu_list);

	/* pool addr for mcast enhance buff */
	struct {
		int size;
		uint32_t paddr;
		uint32_t *vaddr;
		struct dp_tx_me_buf_t *freelist;
		int buf_in_use;
		qdf_dma_mem_context(memctx);
	} me_buf;

	/**
	 * peer ref mutex:
	 * 1. Protect peer object lookups until the returned peer object's
	 *	reference count is incremented.
	 * 2. Provide mutex when accessing peer object lookup structures.
	 */
	DP_MUTEX_TYPE peer_ref_mutex;

	/* maximum value for peer_id */
	int max_peers;

	/* SoC level data path statistics */
	struct dp_soc_stats stats;

	/* Enable processing of Tx completion status words */
	bool process_tx_status;
	bool process_rx_status;
	struct dp_ast_entry *ast_table[WLAN_UMAC_PSOC_MAX_PEERS * 2];
	struct {
		unsigned mask;
		unsigned idx_bits;
		TAILQ_HEAD(, dp_ast_entry) * bins;
	} ast_hash;

	qdf_spinlock_t ast_lock;
	qdf_timer_t wds_aging_timer;

	/*interrupt timer*/
	qdf_timer_t mon_reap_timer;
	uint8_t reap_timer_init;
	qdf_timer_t int_timer;
	uint8_t intr_mode;

	qdf_list_t reo_desc_freelist;
	qdf_spinlock_t reo_desc_freelist_lock;

#ifdef QCA_SUPPORT_SON
	/* The timer to check station's inactivity status */
	os_timer_t pdev_bs_inact_timer;
	/* The current inactivity count reload value
	   based on overload condition */
	u_int16_t pdev_bs_inact_reload;

	/* The inactivity timer value when not overloaded */
	u_int16_t pdev_bs_inact_normal;

	/* The inactivity timer value when overloaded */
	u_int16_t pdev_bs_inact_overload;

	/* The inactivity timer check interval */
	u_int16_t pdev_bs_inact_interval;
	/* Inactivity timer */
#endif /* QCA_SUPPORT_SON */

	/* htt stats */
	struct htt_t2h_stats htt_stats;

	void *external_txrx_handle; /* External data path handle */
#ifdef IPA_OFFLOAD
	/* IPA uC datapath offload Wlan Tx resources */
	struct {
		/* Resource info to be passed to IPA */
		qdf_dma_addr_t ipa_tcl_ring_base_paddr;
		void *ipa_tcl_ring_base_vaddr;
		uint32_t ipa_tcl_ring_size;
		qdf_dma_addr_t ipa_tcl_hp_paddr;
		uint32_t alloc_tx_buf_cnt;

		qdf_dma_addr_t ipa_wbm_ring_base_paddr;
		void *ipa_wbm_ring_base_vaddr;
		uint32_t ipa_wbm_ring_size;
		qdf_dma_addr_t ipa_wbm_tp_paddr;

		/* TX buffers populated into the WBM ring */
		void **tx_buf_pool_vaddr_unaligned;
		qdf_dma_addr_t *tx_buf_pool_paddr_unaligned;
	} ipa_uc_tx_rsc;

	/* IPA uC datapath offload Wlan Rx resources */
	struct {
		/* Resource info to be passed to IPA */
		qdf_dma_addr_t ipa_reo_ring_base_paddr;
		void *ipa_reo_ring_base_vaddr;
		uint32_t ipa_reo_ring_size;
		qdf_dma_addr_t ipa_reo_tp_paddr;

		/* Resource info to be passed to firmware and IPA */
		qdf_dma_addr_t ipa_rx_refill_buf_ring_base_paddr;
		void *ipa_rx_refill_buf_ring_base_vaddr;
		uint32_t ipa_rx_refill_buf_ring_size;
		qdf_dma_addr_t ipa_rx_refill_buf_hp_paddr;
	} ipa_uc_rx_rsc;
#endif
};

#ifdef IPA_OFFLOAD
/**
 * dp_ipa_resources - Resources needed for IPA
 */
struct dp_ipa_resources {
	qdf_dma_addr_t tx_ring_base_paddr;
	uint32_t tx_ring_size;
	uint32_t tx_num_alloc_buffer;

	qdf_dma_addr_t tx_comp_ring_base_paddr;
	uint32_t tx_comp_ring_size;

	qdf_dma_addr_t rx_rdy_ring_base_paddr;
	uint32_t rx_rdy_ring_size;

	qdf_dma_addr_t rx_refill_ring_base_paddr;
	uint32_t rx_refill_ring_size;

	/* IPA UC doorbell registers paddr */
	qdf_dma_addr_t tx_comp_doorbell_paddr;
	uint32_t *tx_comp_doorbell_vaddr;
	qdf_dma_addr_t rx_ready_doorbell_paddr;
};
#endif

#define MAX_RX_MAC_RINGS 2
/* Same as NAC_MAX_CLENT */
#define DP_NAC_MAX_CLIENT  24

/*
 * Macros to setup link descriptor cookies - for link descriptors, we just
 * need first 3 bits to store bank ID. The remaining bytes will be used set a
 * unique ID, which will be useful in debugging
 */
#define LINK_DESC_BANK_ID_MASK 0x7
#define LINK_DESC_ID_SHIFT 3
#define LINK_DESC_ID_START 0x8000

#define LINK_DESC_COOKIE(_desc_id, _bank_id) \
	((((_desc_id) + LINK_DESC_ID_START) << LINK_DESC_ID_SHIFT) | (_bank_id))

#define LINK_DESC_COOKIE_BANK_ID(_cookie) \
	((_cookie) & LINK_DESC_BANK_ID_MASK)

/* same as ieee80211_nac_param */
enum dp_nac_param_cmd {
	/* IEEE80211_NAC_PARAM_ADD */
	DP_NAC_PARAM_ADD = 1,
	/* IEEE80211_NAC_PARAM_DEL */
	DP_NAC_PARAM_DEL,
	/* IEEE80211_NAC_PARAM_LIST */
	DP_NAC_PARAM_LIST,
};

/**
 * struct dp_neighbour_peer - neighbour peer list type for smart mesh
 * @neighbour_peers_macaddr: neighbour peer's mac address
 * @neighbour_peer_list_elem: neighbour peer list TAILQ element
 */
struct dp_neighbour_peer {
	/* MAC address of neighbour's peer */
	union dp_align_mac_addr neighbour_peers_macaddr;
	/* node in the list of neighbour's peer */
	TAILQ_ENTRY(dp_neighbour_peer) neighbour_peer_list_elem;
};

/* PDEV level structure for data path */
struct dp_pdev {
	/* PDEV handle from OSIF layer TBD: see if we really need osif_pdev */
	void *osif_pdev;

	/* PDEV Id */
	int pdev_id;

	/* TXRX SOC handle */
	struct dp_soc *soc;

	/* Ring used to replenish rx buffers (maybe to the firmware of MAC) */
	struct dp_srng rx_refill_buf_ring;

	/* Second ring used to replenish rx buffers */
	struct dp_srng rx_refill_buf_ring2;

	/* Empty ring used by firmware to post rx buffers to the MAC */
	struct dp_srng rx_mac_buf_ring[MAX_RX_MAC_RINGS];

	/* wlan_cfg pdev ctxt*/
	 struct wlan_cfg_dp_pdev_ctxt *wlan_cfg_ctx;

	/* RXDMA monitor buffer replenish ring */
	struct dp_srng rxdma_mon_buf_ring;

	/* RXDMA monitor destination ring */
	struct dp_srng rxdma_mon_dst_ring;

	/* RXDMA monitor status ring. TBD: Check format of this ring */
	struct dp_srng rxdma_mon_status_ring;

	struct dp_srng rxdma_mon_desc_ring;

	/* RXDMA error destination ring */
	struct dp_srng rxdma_err_dst_ring[MAX_RX_MAC_RINGS];

	/* Link descriptor memory banks */
	struct {
		void *base_vaddr_unaligned;
		void *base_vaddr;
		qdf_dma_addr_t base_paddr_unaligned;
		qdf_dma_addr_t base_paddr;
		uint32_t size;
	} link_desc_banks[MAX_MON_LINK_DESC_BANKS];


	/**
	 * TODO: See if we need a ring map here for LMAC rings.
	 * 1. Monitor rings are currently planning to be processed on receiving
	 * PPDU end interrupts and hence wont need ring based interrupts.
	 * 2. Rx buffer rings will be replenished during REO destination
	 * processing and doesn't require regular interrupt handling - we will
	 * only handle low water mark interrupts which is not expected
	 * frequently
	 */

	/* VDEV list */
	TAILQ_HEAD(, dp_vdev) vdev_list;

	/* Number of vdevs this device have */
	uint16_t vdev_count;

	/* PDEV transmit lock */
	qdf_spinlock_t tx_lock;

#ifndef REMOVE_PKT_LOG
	bool pkt_log_init;
	/* Pktlog pdev */
	struct pktlog_dev_t *pl_dev;
#endif /* #ifndef REMOVE_PKT_LOG */

	/* Monitor mode interface and status storage */
	struct dp_vdev *monitor_vdev;

	/* monitor mode mutex */
	qdf_spinlock_t mon_mutex;

	/*tx_mutex for me*/
	DP_MUTEX_TYPE tx_mutex;

	/* Smart Mesh */
	bool filter_neighbour_peers;
	/* smart mesh mutex */
	qdf_spinlock_t neighbour_peer_mutex;
	/* Neighnour peer list */
	TAILQ_HEAD(, dp_neighbour_peer) neighbour_peers_list;
	/* msdu chain head & tail */
	qdf_nbuf_t invalid_peer_head_msdu;
	qdf_nbuf_t invalid_peer_tail_msdu;

	/* Band steering  */
	/* TBD */

	/* PDEV level data path statistics */
	struct cdp_pdev_stats stats;

	/* Global RX decap mode for the device */
	enum htt_pkt_type rx_decap_mode;

	/* Enhanced Stats is enabled */
	bool enhanced_stats_en;

	/* advance filter mode and type*/
	uint8_t mon_filter_mode;
	uint16_t fp_mgmt_filter;
	uint16_t fp_ctrl_filter;
	uint16_t fp_data_filter;
	uint16_t mo_mgmt_filter;
	uint16_t mo_ctrl_filter;
	uint16_t mo_data_filter;

	qdf_atomic_t num_tx_outstanding;

	qdf_atomic_t num_tx_exception;

	/* MCL specific local peer handle */
	struct {
		uint8_t pool[OL_TXRX_NUM_LOCAL_PEER_IDS + 1];
		uint8_t freelist;
		qdf_spinlock_t lock;
		struct dp_peer *map[OL_TXRX_NUM_LOCAL_PEER_IDS];
	} local_peer_ids;

	/* dscp_tid_map_*/
	uint8_t dscp_tid_map[DP_MAX_TID_MAPS][DSCP_TID_MAP_MAX];

	struct hal_rx_ppdu_info ppdu_info;

	/* operating channel */
	uint8_t operating_channel;

	qdf_nbuf_queue_t rx_status_q;
	uint32_t mon_ppdu_status;
	struct cdp_mon_status rx_mon_recv_status;

	/* pool addr for mcast enhance buff */
	struct {
		int size;
		uint32_t paddr;
		char *vaddr;
		struct dp_tx_me_buf_t *freelist;
		int buf_in_use;
		qdf_dma_mem_context(memctx);
	} me_buf;

	/* Number of VAPs with mcast enhancement enabled */
	qdf_atomic_t mc_num_vap_attached;

	qdf_atomic_t stats_cmd_complete;

#ifdef IPA_OFFLOAD
	ipa_uc_op_cb_type ipa_uc_op_cb;
	void *usr_ctxt;
	struct dp_ipa_resources ipa_resource;
#endif

	/* TBD */

	/* map this pdev to a particular Reo Destination ring */
	enum cdp_host_reo_dest_ring reo_dest;

#ifndef REMOVE_PKT_LOG
	/* Packet log mode */
	uint8_t rx_pktlog_mode;
#endif

	/* WDI event handlers */
	struct wdi_event_subscribe_t **wdi_event_list;

	/* ppdu_id of last received HTT TX stats */
	uint32_t last_ppdu_id;
	struct {
		uint8_t last_user;
		qdf_nbuf_t buf;
	} tx_ppdu_info;

	bool tx_sniffer_enable;
	/* mirror copy mode */
	bool mcopy_mode;

	struct {
		uint16_t tx_ppdu_id;
		uint16_t tx_peer_id;
		uint16_t rx_ppdu_id;
	} am_copy_id;

	/* To check if PPDU Tx stats are enabled for Pktlog */
	bool pktlog_ppdu_stats;

	void *dp_txrx_handle; /* Advanced data path handle */

#ifdef ATH_SUPPORT_NAC_RSSI
	bool nac_rssi_filtering;
#endif
};

struct dp_peer;

/* VDEV structure for data path state */
struct dp_vdev {
	/* OS device abstraction */
	qdf_device_t osdev;
	/* physical device that is the parent of this virtual device */
	struct dp_pdev *pdev;

	/* Handle to the OS shim SW's virtual device */
	ol_osif_vdev_handle osif_vdev;

	/* vdev_id - ID used to specify a particular vdev to the target */
	uint8_t vdev_id;

	/* MAC address */
	union dp_align_mac_addr mac_addr;

	/* node in the pdev's list of vdevs */
	TAILQ_ENTRY(dp_vdev) vdev_list_elem;

	/* dp_peer list */
	TAILQ_HEAD(, dp_peer) peer_list;

	/* callback to hand rx frames to the OS shim */
	ol_txrx_rx_fp osif_rx;
	ol_txrx_rsim_rx_decap_fp osif_rsim_rx_decap;
	ol_txrx_get_key_fp osif_get_key;
	ol_txrx_tx_free_ext_fp osif_tx_free_ext;

#ifdef notyet
	/* callback to check if the msdu is an WAI (WAPI) frame */
	ol_rx_check_wai_fp osif_check_wai;
#endif

	/* proxy arp function */
	ol_txrx_proxy_arp_fp osif_proxy_arp;

	/* callback to hand rx monitor 802.11 MPDU to the OS shim */
	ol_txrx_rx_mon_fp osif_rx_mon;

	ol_txrx_mcast_me_fp me_convert;
	/* deferred vdev deletion state */
	struct {
		/* VDEV delete pending */
		int pending;
		/*
		* callback and a context argument to provide a
		* notification for when the vdev is deleted.
		*/
		ol_txrx_vdev_delete_cb callback;
		void *context;
	} delete;

	/* tx data delivery notification callback function */
	struct {
		ol_txrx_data_tx_cb func;
		void *ctxt;
	} tx_non_std_data_callback;


	/* safe mode control to bypass the encrypt and decipher process*/
	uint32_t safemode;

	/* rx filter related */
	uint32_t drop_unenc;
#ifdef notyet
	privacy_exemption privacy_filters[MAX_PRIVACY_FILTERS];
	uint32_t filters_num;
#endif
	/* TDLS Link status */
	bool tdls_link_connected;
	bool is_tdls_frame;


	/* VDEV operating mode */
	enum wlan_op_mode opmode;

	/* Tx encapsulation type for this VAP */
	enum htt_cmn_pkt_type tx_encap_type;
	/* Rx Decapsulation type for this VAP */
	enum htt_cmn_pkt_type rx_decap_type;

	/* BSS peer */
	struct dp_peer *vap_bss_peer;

	/* WDS enabled */
	bool wds_enabled;

	/* WDS Aging timer period */
	uint32_t wds_aging_timer_val;

	/* NAWDS enabled */
	bool nawds_enabled;

	/* Default HTT meta data for this VDEV */
	/* TBD: check alignment constraints */
	uint16_t htt_tcl_metadata;

	/* Mesh mode vdev */
	uint32_t mesh_vdev;

	/* Mesh mode rx filter setting */
	uint32_t mesh_rx_filter;

	/* DSCP-TID mapping table ID */
	uint8_t dscp_tid_map_id;

	/* Multicast enhancement enabled */
	uint8_t mcast_enhancement_en;

	/* per vdev rx nbuf queue */
	qdf_nbuf_queue_t rxq;

	uint8_t tx_ring_id;
	struct dp_tx_desc_pool_s *tx_desc;
	struct dp_tx_ext_desc_pool_s *tx_ext_desc;

	/* VDEV Stats */
	struct cdp_vdev_stats stats;
	bool lro_enable;

	/* Is this a proxySTA VAP */
	bool proxysta_vdev;
	/* Is isolation mode enabled */
	bool isolation_vdev;

	/* Address search flags to be configured in HAL descriptor */
	uint8_t hal_desc_addr_search_flags;
#ifdef QCA_LL_TX_FLOW_CONTROL_V2
	struct dp_tx_desc_pool_s *pool;
#endif
	/* AP BRIDGE enabled */
	uint32_t ap_bridge_enabled;

	enum cdp_sec_type  sec_type;

#ifdef ATH_SUPPORT_NAC_RSSI
	bool cdp_nac_rssi_enabled;
	struct {
		uint8_t bssid_mac[6];
		uint8_t client_mac[6];
		uint8_t  chan_num;
		uint8_t client_rssi_valid;
		uint8_t client_rssi;
		uint8_t vdev_id;
	} cdp_nac_rssi;
#endif
};


enum {
	dp_sec_mcast = 0,
	dp_sec_ucast
};

#ifdef WDS_VENDOR_EXTENSION
typedef struct {
	uint8_t	wds_tx_mcast_4addr:1,
		wds_tx_ucast_4addr:1,
		wds_rx_filter:1,      /* enforce rx filter */
		wds_rx_ucast_4addr:1, /* when set, accept 4addr unicast frames    */
		wds_rx_mcast_4addr:1;  /* when set, accept 4addr multicast frames  */

} dp_ecm_policy;
#endif

/* Peer structure for data path state */
struct dp_peer {
	/* VDEV to which this peer is associated */
	struct dp_vdev *vdev;

	struct dp_ast_entry *self_ast_entry;

	qdf_atomic_t ref_cnt;

	/* TODO: See if multiple peer IDs are required in wifi3.0 */
	/* peer ID(s) for this peer */
	uint16_t peer_ids[MAX_NUM_PEER_ID_PER_PEER];

	union dp_align_mac_addr mac_addr;

	/* node in the vdev's list of peers */
	TAILQ_ENTRY(dp_peer) peer_list_elem;
	/* node in the hash table bin's list of peers */
	TAILQ_ENTRY(dp_peer) hash_list_elem;

	/* TID structures */
	struct dp_rx_tid rx_tid[DP_MAX_TIDS];

	/* TBD: No transmit TID state required? */

	struct {
		enum htt_sec_type sec_type;
		u_int32_t michael_key[2]; /* relevant for TKIP */
	} security[2]; /* 0 -> multicast, 1 -> unicast */

	/*
	* rx proc function: this either is a copy of pdev's rx_opt_proc for
	* regular rx processing, or has been redirected to a /dev/null discard
	* function when peer deletion is in progress.
	*/
	void (*rx_opt_proc)(struct dp_vdev *vdev, struct dp_peer *peer,
		unsigned tid, qdf_nbuf_t msdu_list);

	/* set when node is authorized */
	uint8_t authorize:1;

	/* Band steering: Set when node is inactive */
	uint8_t peer_bs_inact_flag:1;
	u_int16_t peer_bs_inact; /* inactivity mark count */

	/* NAWDS Flag and Bss Peer bit */
	uint8_t nawds_enabled:1,
				bss_peer:1,
				wapi:1,
				wds_enabled:1;

	/* MCL specific peer local id */
	uint16_t local_id;
	enum ol_txrx_peer_state state;
	qdf_spinlock_t peer_info_lock;

	qdf_time_t last_assoc_rcvd;
	qdf_time_t last_disassoc_rcvd;
	qdf_time_t last_deauth_rcvd;
	/* Peer Stats */
	struct cdp_peer_stats stats;

	TAILQ_HEAD(, dp_ast_entry) ast_entry_list;
	/* TBD */

#ifdef WDS_VENDOR_EXTENSION
	dp_ecm_policy wds_ecm;
#endif
	bool delete_in_progress;
};

#ifdef CONFIG_WIN
/*
 * dp_invalid_peer_msg
 * @nbuf: data buffer
 * @wh: 802.11 header
 * @vdev_id: id of vdev
 */
struct dp_invalid_peer_msg {
	qdf_nbuf_t nbuf;
	struct ieee80211_frame *wh;
	uint8_t vdev_id;
};
#endif

/*
 * dp_tx_me_buf_t: ME buffer
 * next: pointer to next buffer
 * data: Destination Mac address
 */
struct dp_tx_me_buf_t {
	/* Note: ME buf pool initialization logic expects next pointer to
	 * be the first element. Dont add anything before next */
	struct dp_tx_me_buf_t *next;
	uint8_t data[DP_MAC_ADDR_LEN];
};

#endif /* _DP_TYPES_H_ */
