/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
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
#include <htt_stats.h>
#include <cdp_txrx_cmn.h>
#ifdef DP_MOB_DEFS
#include <cds_ieee80211_common.h>
#endif
#include <wdi_event_api.h>    /* WDI subscriber event list */

#include "hal_hw_headers.h"
#include <hal_tx.h>
#include <hal_reo.h>
#include "wlan_cfg.h"
#include "hal_rx.h"
#include <hal_api.h>
#include <hal_api_mon.h>
#include "hal_rx.h"
//#include "hal_rx_flow.h"

#define MAX_BW 7
#define MAX_RETRIES 4
#define MAX_RECEPTION_TYPES 4

#define MINIDUMP_STR_SIZE 25
#ifndef REMOVE_PKT_LOG
#include <pktlog.h>
#endif

#ifdef WLAN_TX_PKT_CAPTURE_ENH
#include "dp_tx_capture.h"
#endif

#define REPT_MU_MIMO 1
#define REPT_MU_OFDMA_MIMO 3
#define DP_VO_TID 6
 /** MAX TID MAPS AVAILABLE PER PDEV */
#define DP_MAX_TID_MAPS 16
/** pad DSCP_TID_MAP_MAX with 6 to fix oob issue */
#define DSCP_TID_MAP_MAX (64 + 6)
#define DP_IP_DSCP_SHIFT 2
#define DP_IP_DSCP_MASK 0x3f
#define DP_FC0_SUBTYPE_QOS 0x80
#define DP_QOS_TID 0x0f
#define DP_IPV6_PRIORITY_SHIFT 20
#define MAX_MON_LINK_DESC_BANKS 2
#define DP_VDEV_ALL 0xff

#if defined(WLAN_MAX_PDEVS) && (WLAN_MAX_PDEVS == 1)
#define MAX_PDEV_CNT 1
#else
#define MAX_PDEV_CNT 3
#endif

/* Max no. of VDEV per PSOC */
#ifdef WLAN_PSOC_MAX_VDEVS
#define MAX_VDEV_CNT WLAN_PSOC_MAX_VDEVS
#else
#define MAX_VDEV_CNT 51
#endif

/* Max no. of VDEVs, a PDEV can support */
#ifdef WLAN_PDEV_MAX_VDEVS
#define DP_PDEV_MAX_VDEVS WLAN_PDEV_MAX_VDEVS
#else
#define DP_PDEV_MAX_VDEVS 17
#endif

#define MAX_TXDESC_POOLS 4
#define MAX_RXDESC_POOLS 4
#define MAX_REO_DEST_RINGS 4
#define EXCEPTION_DEST_RING_ID 0
#define MAX_TCL_DATA_RINGS 4
#define MAX_IDLE_SCATTER_BUFS 16
#define DP_MAX_IRQ_PER_CONTEXT 12
#define DEFAULT_HW_PEER_ID 0xffff

#define WBM_INT_ERROR_ALL 0
#define WBM_INT_ERROR_REO_NULL_BUFFER 1
#define WBM_INT_ERROR_REO_NULL_LINK_DESC 2
#define WBM_INT_ERROR_REO_NULL_MSDU_BUFF 3
#define WBM_INT_ERROR_REO_BUFF_REAPED 4
#define MAX_WBM_INT_ERROR_REASONS 5

#define MAX_TX_HW_QUEUES MAX_TCL_DATA_RINGS
/* Maximum retries for Delba per tid per peer */
#define DP_MAX_DELBA_RETRY 3

#define PCP_TID_MAP_MAX 8
#define MAX_MU_USERS 37

#define REO_CMD_EVENT_HIST_MAX 64

/* 2G PHYB */
#define PHYB_2G_LMAC_ID 2
#define PHYB_2G_TARGET_PDEV_ID 2

/* Flags for skippig s/w tid classification */
#define DP_TX_HW_DSCP_TID_MAP_VALID 0x1
#define DP_TXRX_HLOS_TID_OVERRIDE_ENABLED 0x2
#define DP_TX_MESH_ENABLED 0x4

enum rx_pktlog_mode {
	DP_RX_PKTLOG_DISABLED = 0,
	DP_RX_PKTLOG_FULL,
	DP_RX_PKTLOG_LITE,
};

/* enum m_copy_mode - Available mcopy mode
 *
 */
enum m_copy_mode {
	M_COPY_DISABLED = 0,
	M_COPY = 2,
	M_COPY_EXTENDED = 4,
};

struct msdu_list {
	qdf_nbuf_t head;
	qdf_nbuf_t tail;
	uint32 sum_len;
};

struct dp_soc_cmn;
struct dp_pdev;
struct dp_vdev;
struct dp_tx_desc_s;
struct dp_soc;
union dp_rx_desc_list_elem_t;
struct cdp_peer_rate_stats_ctx;
struct cdp_soc_rate_stats_ctx;
struct dp_rx_fst;
struct dp_mon_filter;
struct dp_mon_mpdu;

/**
 * enum for DP peer state
 */
enum dp_peer_state {
	DP_PEER_STATE_NONE,
	DP_PEER_STATE_INIT,
	DP_PEER_STATE_ACTIVE,
	DP_PEER_STATE_LOGICAL_DELETE,
	DP_PEER_STATE_INACTIVE,
	DP_PEER_STATE_FREED,
	DP_PEER_STATE_INVALID,
};

/**
 * enum for modules ids of
 */
enum dp_mod_id {
	DP_MOD_ID_TX_COMP = 0,
	DP_MOD_ID_RX = 1,
	DP_MOD_ID_HTT_COMP = 2,
	DP_MOD_ID_RX_ERR = 3,
	DP_MOD_ID_TX_PPDU_STATS = 4,
	DP_MOD_ID_RX_PPDU_STATS = 5,
	DP_MOD_ID_CDP = 6,
	DP_MOD_ID_GENERIC_STATS = 7,
	DP_MOD_ID_TX_MULTIPASS = 8,
	DP_MOD_ID_TX_CAPTURE = 9,
	DP_MOD_ID_NSS_OFFLOAD = 10,
	DP_MOD_ID_CONFIG = 11,
	DP_MOD_ID_HTT = 12,
	DP_MOD_ID_IPA = 13,
	DP_MOD_ID_AST = 14,
	DP_MOD_ID_MCAST2UCAST = 15,
	DP_MOD_ID_CHILD = 16,
	DP_MOD_ID_MESH = 17,
	DP_MOD_ID_TX_EXCEPTION = 18,
	DP_MOD_ID_TDLS = 19,
	DP_MOD_ID_MISC = 20,
	DP_MOD_ID_MSCS = 21,
	DP_MOD_ID_TX = 22,
	DP_MOD_ID_MAX = 23,
};

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
#define DP_FRAME_IS_SNAP(_llc) ((_llc)->llc_dsap == 0xaa && \
		(_llc)->llc_ssap == 0xaa && \
		(_llc)->llc_un.type_snap.control == 0x3)
#define DP_FRAME_IS_LLC(typeorlen) ((typeorlen) >= 0x600)
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

/**
 * Number of Tx Queues
 * enum and macro to define how many threshold levels is used
 * for the AC based flow control
 */
#ifdef QCA_AC_BASED_FLOW_CONTROL
enum dp_fl_ctrl_threshold {
	DP_TH_BE_BK = 0,
	DP_TH_VI,
	DP_TH_VO,
	DP_TH_HI,
};

#define FL_TH_MAX (4)
#define FL_TH_VI_PERCENTAGE (80)
#define FL_TH_VO_PERCENTAGE (60)
#define FL_TH_HI_PERCENTAGE (40)
#endif

/**
 * enum dp_intr_mode
 * @DP_INTR_INTEGRATED: Line interrupts
 * @DP_INTR_MSI: MSI interrupts
 * @DP_INTR_POLL: Polling
 */
enum dp_intr_mode {
	DP_INTR_INTEGRATED = 0,
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
 * @dp_nss_cfg_dbtc: Three radios offloaded
 */
enum dp_nss_cfg {
	dp_nss_cfg_default = 0x0,
	dp_nss_cfg_first_radio = 0x1,
	dp_nss_cfg_second_radio = 0x2,
	dp_nss_cfg_dbdc = 0x3,
	dp_nss_cfg_dbtc = 0x7,
	dp_nss_cfg_max
};

#ifdef WLAN_TX_PKT_CAPTURE_ENH
#define DP_CPU_RING_MAP_1 1
#endif

/**
 * dp_cpu_ring_map_type - dp tx cpu ring map
 * @DP_NSS_DEFAULT_MAP: Default mode with no NSS offloaded
 * @DP_NSS_FIRST_RADIO_OFFLOADED_MAP: Only First Radio is offloaded
 * @DP_NSS_SECOND_RADIO_OFFLOADED_MAP: Only second radio is offloaded
 * @DP_NSS_DBDC_OFFLOADED_MAP: Both radios are offloaded
 * @DP_NSS_DBTC_OFFLOADED_MAP: All three radios are offloaded
 * @DP_SINGLE_TX_RING_MAP: to avoid out of order all cpu mapped to single ring
 * @DP_NSS_CPU_RING_MAP_MAX: Max cpu ring map val
 */
enum dp_cpu_ring_map_types {
	DP_NSS_DEFAULT_MAP,
	DP_NSS_FIRST_RADIO_OFFLOADED_MAP,
	DP_NSS_SECOND_RADIO_OFFLOADED_MAP,
	DP_NSS_DBDC_OFFLOADED_MAP,
	DP_NSS_DBTC_OFFLOADED_MAP,
#ifdef WLAN_TX_PKT_CAPTURE_ENH
	DP_SINGLE_TX_RING_MAP,
#endif
	DP_NSS_CPU_RING_MAP_MAX
};

/**
 * dp_rx_nbuf_frag_info - Hold vaddr and paddr for a buffer
 *
 * paddr: Physical address of buffer allocated.
 * nbuf: Allocated nbuf in case of nbuf approach.
 * vaddr: Virtual address of frag allocated in case of frag approach.
 */
struct dp_rx_nbuf_frag_info {
	qdf_dma_addr_t paddr;
	union {
		qdf_nbuf_t nbuf;
		qdf_frag_t vaddr;
	} virt_addr;
};

/**
 * enum dp_desc_type - source type for multiple pages allocation
 * @DP_TX_DESC_TYPE: DP SW TX descriptor
 * @DP_TX_EXT_DESC_TYPE: DP TX msdu extension descriptor
 * @DP_TX_EXT_DESC_LINK_TYPE: DP link descriptor for msdu ext_desc
 * @DP_TX_TSO_DESC_TYPE: DP TX TSO descriptor
 * @DP_TX_TSO_NUM_SEG_TYPE: DP TX number of segments
 * @DP_RX_DESC_BUF_TYPE: DP RX SW descriptor
 * @DP_RX_DESC_STATUS_TYPE: DP RX SW descriptor for monitor status
 * @DP_HW_LINK_DESC_TYPE: DP HW link descriptor
 */
enum dp_desc_type {
	DP_TX_DESC_TYPE,
	DP_TX_EXT_DESC_TYPE,
	DP_TX_EXT_DESC_LINK_TYPE,
	DP_TX_TSO_DESC_TYPE,
	DP_TX_TSO_NUM_SEG_TYPE,
	DP_RX_DESC_BUF_TYPE,
	DP_RX_DESC_STATUS_TYPE,
	DP_HW_LINK_DESC_TYPE,
};

/**
 * struct rx_desc_pool
 * @pool_size: number of RX descriptor in the pool
 * @elem_size: Element size
 * @desc_pages: Multi page descriptors
 * @array: pointer to array of RX descriptor
 * @freelist: pointer to free RX descriptor link list
 * @lock: Protection for the RX descriptor pool
 * @owner: owner for nbuf
 * @buf_size: Buffer size
 * @buf_alignment: Buffer alignment
 * @rx_mon_dest_frag_enable: Enable frag processing for mon dest buffer
 * @desc_type: type of desc this pool serves
 */
struct rx_desc_pool {
	uint32_t pool_size;
#ifdef RX_DESC_MULTI_PAGE_ALLOC
	uint16_t elem_size;
	struct qdf_mem_multi_page_t desc_pages;
#else
	union dp_rx_desc_list_elem_t *array;
#endif
	union dp_rx_desc_list_elem_t *freelist;
	qdf_spinlock_t lock;
	uint8_t owner;
	uint16_t buf_size;
	uint8_t buf_alignment;
	bool rx_mon_dest_frag_enable;
	enum dp_desc_type desc_type;
};

/**
 * struct dp_tx_ext_desc_elem_s
 * @next: next extension descriptor pointer
 * @vaddr: hlos virtual address pointer
 * @paddr: physical address pointer for descriptor
 * @flags: mark features for extension descriptor
 */
struct dp_tx_ext_desc_elem_s {
	struct dp_tx_ext_desc_elem_s *next;
	void *vaddr;
	qdf_dma_addr_t paddr;
	uint16_t flags;
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
 * @vdev_id: vdev_id of vdev over which the packet was transmitted
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
	uint16_t length;
	uint16_t flags;
	uint32_t id;
	qdf_dma_addr_t dma_addr;
	uint8_t vdev_id;
	uint8_t tx_status;
	uint16_t peer_id;
	struct dp_pdev *pdev;
	uint8_t tx_encap_type;
	uint8_t frm_type;
	uint8_t pkt_offset;
	uint8_t  pool_id;
	struct dp_tx_ext_desc_elem_s *msdu_ext_desc;
	void *me_buffer;
	void *tso_desc;
	void *tso_num_desc;
	uint64_t timestamp;
	struct hal_tx_desc_comp_s comp;
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
	FLOW_POOL_BE_BK_PAUSED = 2,
	FLOW_POOL_VI_PAUSED = 3,
	FLOW_POOL_VO_PAUSED = 4,
	FLOW_POOL_INVALID = 5,
	FLOW_POOL_INACTIVE = 6,
};

/**
 * struct dp_tx_tso_seg_pool_s
 * @pool_size: total number of pool elements
 * @num_free: free element count
 * @freelist: first free element pointer
 * @desc_pages: multiple page allocation information for actual descriptors
 * @lock: lock for accessing the pool
 */
struct dp_tx_tso_seg_pool_s {
	uint16_t pool_size;
	uint16_t num_free;
	struct qdf_tso_seg_elem_t *freelist;
	struct qdf_mem_multi_page_t desc_pages;
	qdf_spinlock_t lock;
};

/**
 * struct dp_tx_tso_num_seg_pool_s {
 * @num_seg_pool_size: total number of pool elements
 * @num_free: free element count
 * @freelist: first free element pointer
 * @desc_pages: multiple page allocation information for actual descriptors
 * @lock: lock for accessing the pool
 */

struct dp_tx_tso_num_seg_pool_s {
	uint16_t num_seg_pool_size;
	uint16_t num_free;
	struct qdf_tso_num_seg_elem_t *freelist;
	struct qdf_mem_multi_page_t desc_pages;
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
#ifdef QCA_AC_BASED_FLOW_CONTROL
	uint16_t stop_th[FL_TH_MAX];
	uint16_t start_th[FL_TH_MAX];
	qdf_time_t max_pause_time[FL_TH_MAX];
	qdf_time_t latest_pause_time[FL_TH_MAX];
#else
	uint16_t stop_th;
	uint16_t start_th;
#endif
	uint16_t pkt_drop_no_desc;
	qdf_spinlock_t flow_pool_lock;
	uint8_t pool_create_cnt;
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

/**
 * struct dp_srng - DP srng structure
 * @hal_srng: hal_srng handle
 * @base_vaddr_unaligned: un-aligned virtual base address of the srng ring
 * @base_vaddr_aligned: aligned virtual base address of the srng ring
 * @base_paddr_unaligned: un-aligned physical base address of the srng ring
 * @base_paddr_aligned: aligned physical base address of the srng ring
 * @alloc_size: size of the srng ring
 * @cached: is the srng ring memory cached or un-cached memory
 * @irq: irq number of the srng ring
 * @num_entries: number of entries in the srng ring
 */
struct dp_srng {
	hal_ring_handle_t hal_srng;
	void *base_vaddr_unaligned;
	void *base_vaddr_aligned;
	qdf_dma_addr_t base_paddr_unaligned;
	qdf_dma_addr_t base_paddr_aligned;
	uint32_t alloc_size;
	uint8_t cached;
	int irq;
	uint32_t num_entries;
#ifdef DP_MEM_PRE_ALLOC
	uint8_t is_mem_prealloc;
#endif
};

struct dp_rx_reorder_array_elem {
	qdf_nbuf_t head;
	qdf_nbuf_t tail;
};

#define DP_RX_BA_INACTIVE 0
#define DP_RX_BA_ACTIVE 1
#define DP_RX_BA_IN_PROGRESS 2
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

	/* Num of addba responses successful */
	uint32_t num_addba_rsp_success;

	/* Num of addba responses failed */
	uint32_t num_addba_rsp_failed;

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

	/* Starting sequence number in Addba request */
	uint16_t startseqnum;

	/* TODO: Check the following while adding defragmentation support */
	struct dp_rx_reorder_array_elem *array;
	/* base - single rx reorder element used for non-aggr cases */
	struct dp_rx_reorder_array_elem base;

	/* only used for defrag right now */
	TAILQ_ENTRY(dp_rx_tid) defrag_waitlist_elem;

	/* Store dst desc for reinjection */
	hal_ring_desc_t dst_ring_desc;
	struct dp_rx_desc *head_frag_desc;

	/* rx_tid lock */
	qdf_spinlock_t tid_lock;

	/* Sequence and fragments that are being processed currently */
	uint32_t curr_seq_num;
	uint32_t curr_frag_num;

	/* head PN number */
	uint64_t pn128[2];

	uint32_t defrag_timeout_ms;
	uint16_t dialogtoken;
	uint16_t statuscode;
	/* user defined ADDBA response status code */
	uint16_t userstatuscode;

	/* Store ppdu_id when 2k exception is received */
	uint32_t ppdu_id_2k;

	/* Delba Tx completion status */
	uint8_t delba_tx_status;

	/* Delba Tx retry count */
	uint8_t delba_tx_retry;

	/* Delba stats */
	uint32_t delba_tx_success_cnt;
	uint32_t delba_tx_fail_cnt;

	/* Delba reason code for retries */
	uint8_t delba_rcode;

	/* Coex Override preserved windows size 1 based */
	uint16_t rx_ba_win_size_override;

	/* Peer TID statistics */
	struct cdp_peer_tid_stats stats;
};

/**
 * struct dp_intr_stats - DP Interrupt Stats for an interrupt context
 * @num_tx_ring_masks: interrupts with tx_ring_mask set
 * @num_rx_ring_masks: interrupts with rx_ring_mask set
 * @num_rx_mon_ring_masks: interrupts with rx_mon_ring_mask set
 * @num_rx_err_ring_masks: interrupts with rx_err_ring_mask set
 * @num_rx_wbm_rel_ring_masks: interrupts with rx_wbm_rel_ring_mask set
 * @num_reo_status_ring_masks: interrupts with reo_status_ring_mask set
 * @num_rxdma2host_ring_masks: interrupts with rxdma2host_ring_mask set
 * @num_host2rxdma_ring_masks: interrupts with host2rxdma_ring_mask set
 * @num_host2rxdma_ring_masks: interrupts with host2rxdma_ring_mask set
 * @num_masks: total number of times the interrupt was received
 *
 * Counter for individual masks are incremented only if there are any packets
 * on that ring.
 */
struct dp_intr_stats {
	uint32_t num_tx_ring_masks[MAX_TCL_DATA_RINGS];
	uint32_t num_rx_ring_masks[MAX_REO_DEST_RINGS];
	uint32_t num_rx_mon_ring_masks;
	uint32_t num_rx_err_ring_masks;
	uint32_t num_rx_wbm_rel_ring_masks;
	uint32_t num_reo_status_ring_masks;
	uint32_t num_rxdma2host_ring_masks;
	uint32_t num_host2rxdma_ring_masks;
	uint32_t num_masks;
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
	/* Host to RXDMA monitor  buffer ring */
	uint8_t host2rxdma_mon_ring_mask;
	struct dp_soc *soc;    /* Reference to SoC structure ,
				to get DMA ring handles */
	qdf_lro_ctx_t lro_ctx;
	uint8_t dp_intr_id;

	/* Interrupt Stats for individual masks */
	struct dp_intr_stats intr_stats;
};

#define REO_DESC_FREELIST_SIZE 64
#define REO_DESC_FREE_DEFER_MS 1000
struct reo_desc_list_node {
	qdf_list_node_t node;
	unsigned long free_ts;
	struct dp_rx_tid rx_tid;
	bool resend_update_reo_cmd;
	uint32_t pending_ext_desc_size;
};

#ifdef WLAN_FEATURE_DP_EVENT_HISTORY
/**
 * struct reo_cmd_event_record: Elements to record for each reo command
 * @cmd_type: reo command type
 * @cmd_return_status: reo command post status
 * @timestamp: record timestamp for the reo command
 */
struct reo_cmd_event_record {
	enum hal_reo_cmd_type cmd_type;
	uint8_t cmd_return_status;
	uint32_t timestamp;
};

/**
 * struct reo_cmd_event_history: Account for reo cmd events
 * @index: record number
 * @cmd_record: list of records
 */
struct reo_cmd_event_history {
	qdf_atomic_t index;
	struct reo_cmd_event_record cmd_record[REO_CMD_EVENT_HIST_MAX];
};
#endif /* WLAN_FEATURE_DP_EVENT_HISTORY */

/* SoC level data path statistics */
struct dp_soc_stats {
	struct {
		uint32_t added;
		uint32_t deleted;
		uint32_t aged_out;
		uint32_t map_err;
		uint32_t ast_mismatch;
	} ast;

	/* SOC level TX stats */
	struct {
		/* Total packets transmitted */
		struct cdp_pkt_info egress;
		/* packets dropped on tx because of no peer */
		struct cdp_pkt_info tx_invalid_peer;
		/* descriptors in each tcl ring */
		uint32_t tcl_ring_full[MAX_TCL_DATA_RINGS];
		/* Descriptors in use at soc */
		uint32_t desc_in_use;
		/* tqm_release_reason == FW removed */
		uint32_t dropped_fw_removed;
		/* tx completion release_src != TQM or FW */
		uint32_t invalid_release_source;
		/* tx completion wbm_internal_error */
		uint32_t wbm_internal_error[MAX_WBM_INT_ERROR_REASONS];
		/* tx completion non_wbm_internal_error */
		uint32_t non_wbm_internal_err;
		/* TX Comp loop packet limit hit */
		uint32_t tx_comp_loop_pkt_limit_hit;
		/* Head pointer Out of sync at the end of dp_tx_comp_handler */
		uint32_t hp_oos2;
	} tx;

	/* SOC level RX stats */
	struct {
		/* Total rx packets count */
		struct cdp_pkt_info ingress;
		/* Rx errors */
		/* Total Packets in Rx Error ring */
		uint32_t err_ring_pkts;
		/* No of Fragments */
		uint32_t rx_frags;
		/* No of incomplete fragments in waitlist */
		uint32_t rx_frag_wait;
		/* Fragments dropped due to errors */
		uint32_t rx_frag_err;
		/* Fragments received OOR causing sequence num mismatch */
		uint32_t rx_frag_oor;
		/* Fragments dropped due to len errors in skb */
		uint32_t rx_frag_err_len_error;
		/* Fragments dropped due to no peer found */
		uint32_t rx_frag_err_no_peer;
		/* No of reinjected packets */
		uint32_t reo_reinject;
		/* Reap loop packet limit hit */
		uint32_t reap_loop_pkt_limit_hit;
		/* Head pointer Out of sync at the end of dp_rx_process */
		uint32_t hp_oos2;
		/* Rx ring near full */
		uint32_t near_full;
		/* Break ring reaping as not all scattered msdu received */
		uint32_t msdu_scatter_wait_break;

		struct {
			/* Invalid RBM error count */
			uint32_t invalid_rbm;
			/* Invalid VDEV Error count */
			uint32_t invalid_vdev;
			/* Invalid PDEV error count */
			uint32_t invalid_pdev;

			/* Packets delivered to stack that no related peer */
			uint32_t pkt_delivered_no_peer;
			/* Defrag peer uninit error count */
			uint32_t defrag_peer_uninit;
			/* Invalid sa_idx or da_idx*/
			uint32_t invalid_sa_da_idx;
			/* MSDU DONE failures */
			uint32_t msdu_done_fail;
			/* Invalid PEER Error count */
			struct cdp_pkt_info rx_invalid_peer;
			/* Invalid PEER ID count */
			struct cdp_pkt_info rx_invalid_peer_id;
			/* Invalid packet length */
			struct cdp_pkt_info rx_invalid_pkt_len;
			/* HAL ring access Fail error count */
			uint32_t hal_ring_access_fail;
			/* HAL ring access full Fail error count */
			uint32_t hal_ring_access_full_fail;
			/* RX DMA error count */
			uint32_t rxdma_error[HAL_RXDMA_ERR_MAX];
			/* RX REO DEST Desc Invalid Magic count */
			uint32_t rx_desc_invalid_magic;
			/* REO Error count */
			uint32_t reo_error[HAL_REO_ERR_MAX];
			/* HAL REO ERR Count */
			uint32_t hal_reo_error[MAX_REO_DEST_RINGS];
			/* HAL REO DEST Duplicate count */
			uint32_t hal_reo_dest_dup;
			/* HAL WBM RELEASE Duplicate count */
			uint32_t hal_wbm_rel_dup;
			/* HAL RXDMA error Duplicate count */
			uint32_t hal_rxdma_err_dup;
			/* ipa smmu map duplicate count */
			uint32_t ipa_smmu_map_dup;
			/* ipa smmu unmap duplicate count */
			uint32_t ipa_smmu_unmap_dup;
			/* ipa smmu unmap while ipa pipes is disabled */
			uint32_t ipa_unmap_no_pipe;
			/* REO cmd send fail/requeue count */
			uint32_t reo_cmd_send_fail;
			/* REO cmd send drain count */
			uint32_t reo_cmd_send_drain;
			/* RX msdu drop count due to scatter */
			uint32_t scatter_msdu;
			/* RX msdu drop count due to invalid cookie */
			uint32_t invalid_cookie;
			/* Count of stale cookie read in RX path */
			uint32_t stale_cookie;
			/* Delba sent count due to RX 2k jump */
			uint32_t rx_2k_jump_delba_sent;
			/* RX 2k jump msdu indicated to stack count */
			uint32_t rx_2k_jump_to_stack;
			/* RX 2k jump msdu dropped count */
			uint32_t rx_2k_jump_drop;
			/* REO OOR msdu drop count */
			uint32_t reo_err_oor_drop;
			/* REO OOR msdu indicated to stack count */
			uint32_t reo_err_oor_to_stack;
			/* REO OOR scattered msdu count */
			uint32_t reo_err_oor_sg_count;
			/* RX msdu rejected count on delivery to vdev stack_fn*/
			uint32_t rejected;
			/* Incorrect msdu count in MPDU desc info */
			uint32_t msdu_count_mismatch;
			/* RX raw frame dropped count */
			uint32_t raw_frm_drop;
			/* Stale link desc cookie count*/
			uint32_t invalid_link_cookie;
			/* Nbuf sanity failure */
			uint32_t nbuf_sanity_fail;
			/* Duplicate link desc refilled */
			uint32_t dup_refill_link_desc;
		} err;

		/* packet count per core - per ring */
		uint64_t ring_packets[NR_CPUS][MAX_REO_DEST_RINGS];
	} rx;

#ifdef WLAN_FEATURE_DP_EVENT_HISTORY
	struct reo_cmd_event_history cmd_event_history;
#endif /* WLAN_FEATURE_DP_EVENT_HISTORY */
};

union dp_align_mac_addr {
	uint8_t raw[QDF_MAC_ADDR_SIZE];
	struct {
		uint16_t bytes_ab;
		uint16_t bytes_cd;
		uint16_t bytes_ef;
	} align2;
	struct {
		uint32_t bytes_abcd;
		uint16_t bytes_ef;
	} align4;
	struct __attribute__((__packed__)) {
		uint16_t bytes_ab;
		uint32_t bytes_cdef;
	} align4_2;
};

/**
 * struct dp_ast_free_cb_params - HMWDS free callback cookie
 * @mac_addr: ast mac address
 * @peer_mac_addr: mac address of peer
 * @type: ast entry type
 * @vdev_id: vdev_id
 * @flags: ast flags
 */
struct dp_ast_free_cb_params {
	union dp_align_mac_addr mac_addr;
	union dp_align_mac_addr peer_mac_addr;
	enum cdp_txrx_ast_entry_type type;
	uint8_t vdev_id;
	uint32_t flags;
};

/*
 * dp_ast_entry
 *
 * @ast_idx: Hardware AST Index
 * @peer_id: Next Hop peer_id (for non-WDS nodes, this will be point to
 *           associated peer with this MAC address)
 * @mac_addr:  MAC Address for this AST entry
 * @next_hop: Set to 1 if this is for a WDS node
 * @is_active: flag to indicate active data traffic on this node
 *             (used for aging out/expiry)
 * @ase_list_elem: node in peer AST list
 * @is_bss: flag to indicate if entry corresponds to bss peer
 * @is_mapped: flag to indicate that we have mapped the AST entry
 *             in ast_table
 * @pdev_id: pdev ID
 * @vdev_id: vdev ID
 * @ast_hash_value: hast value in HW
 * @ref_cnt: reference count
 * @type: flag to indicate type of the entry(static/WDS/MEC)
 * @delete_in_progress: Flag to indicate that delete commands send to FW
 *                      and host is waiting for response from FW
 * @callback: ast free/unmap callback
 * @cookie: argument to callback
 * @hash_list_elem: node in soc AST hash list (mac address used as hash)
 */
struct dp_ast_entry {
	uint16_t ast_idx;
	uint16_t peer_id;
	union dp_align_mac_addr mac_addr;
	bool next_hop;
	bool is_active;
	bool is_mapped;
	uint8_t pdev_id;
	uint8_t vdev_id;
	uint16_t ast_hash_value;
	qdf_atomic_t ref_cnt;
	enum cdp_txrx_ast_entry_type type;
	bool delete_in_progress;
	txrx_ast_free_cb callback;
	void *cookie;
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

struct link_desc_bank {
	void *base_vaddr_unaligned;
	void *base_vaddr;
	qdf_dma_addr_t base_paddr_unaligned;
	qdf_dma_addr_t base_paddr;
	uint32_t size;
};

struct rx_buff_pool {
	qdf_nbuf_queue_head_t emerg_nbuf_q;
	uint32_t nbuf_fail_cnt;
	bool is_initialized;
};

/*
 * The logic for get current index of these history is dependent on this
 * value being power of 2.
 */
#define DP_RX_HIST_MAX 2048
#define DP_RX_ERR_HIST_MAX 4096
#define DP_RX_REINJECT_HIST_MAX 1024

QDF_COMPILE_TIME_ASSERT(rx_history_size,
			(DP_RX_HIST_MAX &
			 (DP_RX_HIST_MAX - 1)) == 0);
QDF_COMPILE_TIME_ASSERT(rx_err_history_size,
			(DP_RX_ERR_HIST_MAX &
			 (DP_RX_ERR_HIST_MAX - 1)) == 0);
QDF_COMPILE_TIME_ASSERT(rx_reinject_history_size,
			(DP_RX_REINJECT_HIST_MAX &
			 (DP_RX_REINJECT_HIST_MAX - 1)) == 0);

/**
 * struct dp_buf_info_record - ring buffer info
 * @hbi: HW ring buffer info
 * @timestamp: timestamp when this entry was recorded
 */
struct dp_buf_info_record {
	struct hal_buf_info hbi;
	uint64_t timestamp;
};

/* struct dp_rx_history - rx ring hisotry
 * @index: Index where the last entry is written
 * @entry: history entries
 */
struct dp_rx_history {
	qdf_atomic_t index;
	struct dp_buf_info_record entry[DP_RX_HIST_MAX];
};

/* struct dp_rx_err_history - rx err ring hisotry
 * @index: Index where the last entry is written
 * @entry: history entries
 */
struct dp_rx_err_history {
	qdf_atomic_t index;
	struct dp_buf_info_record entry[DP_RX_ERR_HIST_MAX];
};

/* struct dp_rx_reinject_history - rx reinject ring hisotry
 * @index: Index where the last entry is written
 * @entry: history entries
 */
struct dp_rx_reinject_history {
	qdf_atomic_t index;
	struct dp_buf_info_record entry[DP_RX_REINJECT_HIST_MAX];
};

/* structure to record recent operation related variable */
struct dp_last_op_info {
	/* last link desc buf info through WBM release ring */
	struct hal_buf_info wbm_rel_link_desc;
	/* last link desc buf info through REO reinject ring */
	struct hal_buf_info reo_reinject_link_desc;
};

#ifdef WLAN_DP_FEATURE_SW_LATENCY_MGR

/**
 * struct dp_swlm_tcl_data - params for tcl register write coalescing
 *			     descision making
 * @nbuf: TX packet
 * @tid: tid for transmitting the current packet
 * @num_ll_connections: Number of low latency connections on this vdev
 *
 * This structure contains the information required by the software
 * latency manager to decide on whether to coalesce the current TCL
 * register write or not.
 */
struct dp_swlm_tcl_data {
	qdf_nbuf_t nbuf;
	uint8_t tid;
	uint8_t num_ll_connections;
};

/**
 * union swlm_data - SWLM query data
 * @tcl_data: data for TCL query in SWLM
 */
union swlm_data {
	struct dp_swlm_tcl_data *tcl_data;
};

/**
 * struct dp_swlm_ops - SWLM ops
 * @tcl_wr_coalesce_check: handler to check if the current TCL register
 *			   write can be coalesced or not
 */
struct dp_swlm_ops {
	int (*tcl_wr_coalesce_check)(struct dp_soc *soc,
				     struct dp_swlm_tcl_data *tcl_data);
};

/**
 * struct dp_swlm_stats - Stats for Software Latency manager.
 * @tcl.timer_flush_success: Num TCL HP writes success from timer context
 * @tcl.timer_flush_fail: Num TCL HP writes failure from timer context
 * @tcl.tid_fail: Num TCL register write coalescing skips, since the pkt
 *		 was being transmitted on a TID above coalescing threshold
 * @tcl.sp_frames: Num TCL register write coalescing skips, since the pkt
 *		  being transmitted was a special frame
 * @tcl.ll_connection: Num TCL register write coalescing skips, since the
 *		       vdev has low latency connections
 * @tcl.bytes_thresh_reached: Num TCL HP writes flush after the coalescing
 *			     bytes threshold was reached
 * @tcl.time_thresh_reached: Num TCL HP writes flush after the coalescing
 *			    session time expired
 * @tcl.tput_criteria_fail: Num TCL HP writes coalescing fails, since the
 *			   throughput did not meet session threshold
 * @tcl.coalesce_success: Num of TCL HP writes coalesced successfully.
 * @tcl.coalesce_fail: Num of TCL HP writes coalesces failed
 */
struct dp_swlm_stats {
	struct {
		uint32_t timer_flush_success;
		uint32_t timer_flush_fail;
		uint32_t tid_fail;
		uint32_t sp_frames;
		uint32_t ll_connection;
		uint32_t bytes_thresh_reached;
		uint32_t time_thresh_reached;
		uint32_t tput_criteria_fail;
		uint32_t coalesce_success;
		uint32_t coalesce_fail;
	} tcl;
};

/**
 * struct dp_swlm_params: Parameters for different modules in the
 *			  Software latency manager.
 * @tcl.flush_timer: Timer for flushing the coalesced TCL HP writes
 * @tcl.rx_traffic_thresh: Threshold for RX traffic, to begin TCL register
 *			   write coalescing
 * @tcl.tx_traffic_thresh: Threshold for TX traffic, to begin TCL register
 *			   write coalescing
 * @tcl.sampling_time: Sampling time to test the throughput threshold
 * @tcl.sampling_session_tx_bytes: Num bytes transmitted in the sampling time
 * @tcl.bytes_flush_thresh: Bytes threshold to flush the TCL HP register write
 * @tcl.time_flush_thresh: Time threshold to flush the TCL HP register write
 * @tcl.tx_thresh_multiplier: Multiplier to deduce the bytes threshold after
 *			      which the TCL HP register is written, thereby
 *			      ending the coalescing.
 * @tcl.coalesce_end_time: End timestamp for current coalescing session
 * @tcl.bytes_coalesced: Num bytes coalesced in the current session
 */
struct dp_swlm_params {
	struct {
		qdf_timer_t flush_timer;
		uint32_t rx_traffic_thresh;
		uint32_t tx_traffic_thresh;
		uint32_t sampling_time;
		uint32_t sampling_session_tx_bytes;
		uint32_t bytes_flush_thresh;
		uint32_t time_flush_thresh;
		uint32_t tx_thresh_multiplier;
		uint64_t coalesce_end_time;
		uint32_t bytes_coalesced;
	} tcl;
};

/**
 * struct dp_swlm - Software latency manager context
 * @ops: SWLM ops pointers
 * @is_enabled: SWLM enabled/disabled
 * @is_init: SWLM module initialized
 * @stats: SWLM stats
 * @params: SWLM SRNG params
 * @tcl_flush_timer: flush timer for TCL register writes
 */
struct dp_swlm {
	struct dp_swlm_ops *ops;
	uint8_t is_enabled:1,
		is_init:1;
	struct dp_swlm_stats stats;
	struct dp_swlm_params params;
};
#endif

/* SOC level structure for data path */
struct dp_soc {
	/**
	 * re-use memory section starts
	 */

	/* Common base structure - Should be the first member */
	struct cdp_soc_t cdp_soc;

	/* SoC Obj */
	struct cdp_ctrl_objmgr_psoc *ctrl_psoc;

	/* OS device abstraction */
	qdf_device_t osdev;

	/*cce disable*/
	bool cce_disable;

	/* WLAN config context */
	struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx;

	/* HTT handle for host-fw interaction */
	struct htt_soc *htt_handle;

	/* Commint init done */
	qdf_atomic_t cmn_init_done;

	/* Opaque hif handle */
	struct hif_opaque_softc *hif_handle;

	/* PDEVs on this SOC */
	struct dp_pdev *pdev_list[MAX_PDEV_CNT];

	/* Ring used to replenish rx buffers (maybe to the firmware of MAC) */
	struct dp_srng rx_refill_buf_ring[MAX_PDEV_CNT];

	struct dp_srng rxdma_mon_desc_ring[MAX_NUM_LMAC_HW];

	/* RXDMA error destination ring */
	struct dp_srng rxdma_err_dst_ring[MAX_NUM_LMAC_HW];

	/* RXDMA monitor buffer replenish ring */
	struct dp_srng rxdma_mon_buf_ring[MAX_NUM_LMAC_HW];

	/* RXDMA monitor destination ring */
	struct dp_srng rxdma_mon_dst_ring[MAX_NUM_LMAC_HW];

	/* RXDMA monitor status ring. TBD: Check format of this ring */
	struct dp_srng rxdma_mon_status_ring[MAX_NUM_LMAC_HW];

	/* Number of PDEVs */
	uint8_t pdev_count;

	/*ast override support in HW*/
	bool ast_override_support;

	/*number of hw dscp tid map*/
	uint8_t num_hw_dscp_tid_map;

	/* HAL SOC handle */
	hal_soc_handle_t hal_soc;

	/* Device ID coming from Bus sub-system */
	uint32_t device_id;

	/* Link descriptor pages */
	struct qdf_mem_multi_page_t link_desc_pages;

	/* total link descriptors for regular RX and TX */
	uint32_t total_link_descs;

	/* monitor link descriptor pages */
	struct qdf_mem_multi_page_t mon_link_desc_pages[MAX_NUM_LMAC_HW];

	/* total link descriptors for monitor mode for each radio */
	uint32_t total_mon_link_descs[MAX_NUM_LMAC_HW];

	/* Monitor Link descriptor memory banks */
	struct link_desc_bank
		mon_link_desc_banks[MAX_NUM_LMAC_HW][MAX_MON_LINK_DESC_BANKS];
	uint32_t num_mon_link_desc_banks[MAX_NUM_LMAC_HW];

	/* Link descriptor Idle list for HW internal use (SRNG mode) */
	struct dp_srng wbm_idle_link_ring;

	/* Link descriptor Idle list for HW internal use (scatter buffer mode)
	 */
	qdf_dma_addr_t wbm_idle_scatter_buf_base_paddr[MAX_IDLE_SCATTER_BUFS];
	void *wbm_idle_scatter_buf_base_vaddr[MAX_IDLE_SCATTER_BUFS];
	uint32_t num_scatter_bufs;

	/* Tx SW descriptor pool */
	struct dp_tx_desc_pool_s tx_desc[MAX_TXDESC_POOLS];

	/* Tx MSDU Extension descriptor pool */
	struct dp_tx_ext_desc_pool_s tx_ext_desc[MAX_TXDESC_POOLS];

	/* Tx TSO descriptor pool */
	struct dp_tx_tso_seg_pool_s tx_tso_desc[MAX_TXDESC_POOLS];

	/* Tx TSO Num of segments pool */
	struct dp_tx_tso_num_seg_pool_s tx_tso_num_seg[MAX_TXDESC_POOLS];

	/* REO destination rings */
	struct dp_srng reo_dest_ring[MAX_REO_DEST_RINGS];

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

	/* TCL data ring */
	struct dp_srng tcl_data_ring[MAX_TCL_DATA_RINGS];

	/* Number of TCL data rings */
	uint8_t num_tcl_data_rings;

	/* TCL CMD_CREDIT ring */
	/* It is used as credit based ring on QCN9000 else command ring */
	struct dp_srng tcl_cmd_credit_ring;

	/* TCL command status ring */
	struct dp_srng tcl_status_ring;

	/* WBM Tx completion rings */
	struct dp_srng tx_comp_ring[MAX_TCL_DATA_RINGS];

	/* Common WBM link descriptor release ring (SW to WBM) */
	struct dp_srng wbm_desc_rel_ring;

	/* DP Interrupts */
	struct dp_intr intr_ctx[WLAN_CFG_INT_NUM_CONTEXTS];

	/* Monitor mode mac id to dp_intr_id map */
	int mon_intr_id_lmac_map[MAX_NUM_LMAC_HW];
	/* Rx SW descriptor pool for RXDMA monitor buffer */
	struct rx_desc_pool rx_desc_mon[MAX_RXDESC_POOLS];

	/* Rx SW descriptor pool for RXDMA status buffer */
	struct rx_desc_pool rx_desc_status[MAX_RXDESC_POOLS];

	/* Rx SW descriptor pool for RXDMA buffer */
	struct rx_desc_pool rx_desc_buf[MAX_RXDESC_POOLS];

	/* Number of REO destination rings */
	uint8_t num_reo_dest_rings;

#ifdef QCA_LL_TX_FLOW_CONTROL_V2
	/* lock to control access to soc TX descriptors */
	qdf_spinlock_t flow_pool_array_lock;

	/* pause callback to pause TX queues as per flow control */
	tx_pause_callback pause_cb;

	/* flow pool related statistics */
	struct dp_txrx_pool_stats pool_stats;
#endif /* !QCA_LL_TX_FLOW_CONTROL_V2 */

	uint32_t wbm_idle_scatter_buf_size;

	/* VDEVs on this SOC */
	struct dp_vdev *vdev_id_map[MAX_VDEV_CNT];

	/* Tx H/W queues lock */
	qdf_spinlock_t tx_queue_lock[MAX_TX_HW_QUEUES];

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
			uint32_t next_flush_ms;
			qdf_spinlock_t defrag_lock;
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

	/* Protect peer hash table */
	DP_MUTEX_TYPE peer_hash_lock;
	/* Protect peer_id_to_objmap */
	DP_MUTEX_TYPE peer_map_lock;

	/* maximum value for peer_id */
	uint32_t max_peers;

	/* SoC level data path statistics */
	struct dp_soc_stats stats;

	/* Enable processing of Tx completion status words */
	bool process_tx_status;
	bool process_rx_status;
	struct dp_ast_entry **ast_table;
	struct {
		unsigned mask;
		unsigned idx_bits;
		TAILQ_HEAD(, dp_ast_entry) * bins;
	} ast_hash;

	struct dp_rx_history *rx_ring_history[MAX_REO_DEST_RINGS];
	struct dp_rx_err_history *rx_err_ring_history;
	struct dp_rx_reinject_history *rx_reinject_ring_history;

	qdf_spinlock_t ast_lock;
	/*Timer for AST entry ageout maintainance */
	qdf_timer_t ast_aging_timer;

	/*Timer counter for WDS AST entry ageout*/
	uint8_t wds_ast_aging_timer_cnt;

	/*interrupt timer*/
	qdf_timer_t mon_reap_timer;
	uint8_t reap_timer_init;
	qdf_timer_t lmac_reap_timer;
	uint8_t lmac_timer_init;
	qdf_timer_t int_timer;
	uint8_t intr_mode;
	uint8_t lmac_polled_mode;

	qdf_list_t reo_desc_freelist;
	qdf_spinlock_t reo_desc_freelist_lock;

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

	qdf_atomic_t ipa_pipes_enabled;
	bool ipa_first_tx_db_access;
#endif

#ifdef WLAN_FEATURE_STATS_EXT
	struct {
		uint32_t rx_mpdu_received;
		uint32_t rx_mpdu_missed;
	} ext_stats;
	qdf_event_t rx_hw_stats_event;
	qdf_spinlock_t rx_hw_stats_lock;
	bool is_last_stats_ctx_init;
#endif /* WLAN_FEATURE_STATS_EXT */

	/* Smart monitor capability for HKv2 */
	uint8_t hw_nac_monitor_support;
	/* Flag to indicate if HTT v2 is enabled*/
	bool is_peer_map_unmap_v2;
	/* Per peer per Tid ba window size support */
	uint8_t per_tid_basize_max_tid;
	/* Soc level flag to enable da_war */
	uint8_t da_war_enabled;
	/* number of active ast entries */
	uint32_t num_ast_entries;
	/* rdk rate statistics context at soc level*/
	struct cdp_soc_rate_stats_ctx *rate_stats_ctx;
	/* rdk rate statistics control flag */
	bool rdkstats_enabled;

	/* 8021p PCP-TID map values */
	uint8_t pcp_tid_map[PCP_TID_MAP_MAX];
	/* TID map priority value */
	uint8_t tidmap_prty;
	/* Pointer to global per ring type specific configuration table */
	struct wlan_srng_cfg *wlan_srng_cfg;
	/* Num Tx outstanding on device */
	qdf_atomic_t num_tx_outstanding;
	/* Num Tx exception on device */
	qdf_atomic_t num_tx_exception;
	/* Num Tx allowed */
	uint32_t num_tx_allowed;
	/* Preferred HW mode */
	uint8_t preferred_hw_mode;

	/**
	 * Flag to indicate whether WAR to address single cache entry
	 * invalidation bug is enabled or not
	 */
	bool is_rx_fse_full_cache_invalidate_war_enabled;
#if defined(WLAN_SUPPORT_RX_FLOW_TAG) || defined(WLAN_SUPPORT_RX_FISA)
	/**
	 * Pointer to DP RX Flow FST at SOC level if
	 * is_rx_flow_search_table_per_pdev is false
	 * TBD: rx_fst[num_macs] if we decide to have per mac FST
	 */
	struct dp_rx_fst *rx_fst;
#ifdef WLAN_SUPPORT_RX_FISA
	uint8_t fisa_enable;

	/**
	 * Params used for controlling the fisa aggregation dynamically
	 */
	struct {
		qdf_atomic_t skip_fisa;
		uint8_t fisa_force_flush[MAX_REO_DEST_RINGS];
	} skip_fisa_param;
#endif
#endif /* WLAN_SUPPORT_RX_FLOW_TAG || WLAN_SUPPORT_RX_FISA */
	/* Full monitor mode support */
	bool full_mon_mode;
	/* SG supported for msdu continued packets from wbm release ring */
	bool wbm_release_desc_rx_sg_support;
	bool peer_map_attach_success;
	/* Flag to disable mac1 ring interrupts */
	bool disable_mac1_intr;
	/* Flag to disable mac2 ring interrupts */
	bool disable_mac2_intr;

	struct {
		/* 1st msdu in sg for msdu continued packets in wbm rel ring */
		bool wbm_is_first_msdu_in_sg;
		/* Wbm sg list head */
		qdf_nbuf_t wbm_sg_nbuf_head;
		/* Wbm sg list tail */
		qdf_nbuf_t wbm_sg_nbuf_tail;
		uint32_t wbm_sg_desc_msdu_len;
	} wbm_sg_param;
	/* Number of msdu exception descriptors */
	uint32_t num_msdu_exception_desc;

	/* RX buffer params */
	struct rx_buff_pool rx_buff_pool[MAX_PDEV_CNT];
	/* Save recent operation related variable */
	struct dp_last_op_info last_op_info;
	TAILQ_HEAD(, dp_peer) inactive_peer_list;
	qdf_spinlock_t inactive_peer_list_lock;
	TAILQ_HEAD(, dp_vdev) inactive_vdev_list;
	qdf_spinlock_t inactive_vdev_list_lock;
	/* lock to protect vdev_id_map table*/
	qdf_spinlock_t vdev_map_lock;

	/* Flow Search Table is in CMEM */
	bool fst_in_cmem;

#ifdef WLAN_DP_FEATURE_SW_LATENCY_MGR
	struct dp_swlm swlm;
#endif
};

#ifdef IPA_OFFLOAD
/**
 * dp_ipa_resources - Resources needed for IPA
 */
struct dp_ipa_resources {
	qdf_shared_mem_t tx_ring;
	uint32_t tx_num_alloc_buffer;

	qdf_shared_mem_t tx_comp_ring;
	qdf_shared_mem_t rx_rdy_ring;
	qdf_shared_mem_t rx_refill_ring;

	/* IPA UC doorbell registers paddr */
	qdf_dma_addr_t tx_comp_doorbell_paddr;
	uint32_t *tx_comp_doorbell_vaddr;
	qdf_dma_addr_t rx_ready_doorbell_paddr;

	bool is_db_ddr_mapped;
};
#endif

#define MAX_RX_MAC_RINGS 2
/* Same as NAC_MAX_CLENT */
#define DP_NAC_MAX_CLIENT  24

/*
 * 24 bits cookie size
 * 10 bits page id 0 ~ 1023 for MCL
 * 3 bits page id 0 ~ 7 for WIN
 * WBM Idle List Desc size = 128,
 * Num descs per page = 4096/128 = 32 for MCL
 * Num descs per page = 2MB/128 = 16384 for WIN
 */
/*
 * Macros to setup link descriptor cookies - for link descriptors, we just
 * need first 3 bits to store bank/page ID for WIN. The
 * remaining bytes will be used to set a unique ID, which will
 * be useful in debugging
 */
#ifdef MAX_ALLOC_PAGE_SIZE
#define LINK_DESC_PAGE_ID_MASK  0x007FE0
#define LINK_DESC_ID_SHIFT      5
#define LINK_DESC_COOKIE(_desc_id, _page_id) \
	((((_page_id) + LINK_DESC_ID_START) << LINK_DESC_ID_SHIFT) | (_desc_id))
#define LINK_DESC_COOKIE_PAGE_ID(_cookie) \
	(((_cookie) & LINK_DESC_PAGE_ID_MASK) >> LINK_DESC_ID_SHIFT)
#else
#define LINK_DESC_PAGE_ID_MASK  0x7
#define LINK_DESC_ID_SHIFT      3
#define LINK_DESC_COOKIE(_desc_id, _page_id) \
	((((_desc_id) + LINK_DESC_ID_START) << LINK_DESC_ID_SHIFT) | (_page_id))
#define LINK_DESC_COOKIE_PAGE_ID(_cookie) \
	((_cookie) & LINK_DESC_PAGE_ID_MASK)
#endif
#define LINK_DESC_ID_START 0x8000

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
 * @ast_entry: ast_entry for neighbour peer
 * @rssi: rssi value
 */
struct dp_neighbour_peer {
	/* MAC address of neighbour's peer */
	union dp_align_mac_addr neighbour_peers_macaddr;
	struct dp_vdev *vdev;
	struct dp_ast_entry *ast_entry;
	uint8_t rssi;
	/* node in the list of neighbour's peer */
	TAILQ_ENTRY(dp_neighbour_peer) neighbour_peer_list_elem;
};

#ifdef WLAN_TX_PKT_CAPTURE_ENH
#define WLAN_TX_PKT_CAPTURE_ENH 1
#define DP_TX_PPDU_PROC_THRESHOLD 8
#define DP_TX_PPDU_PROC_TIMEOUT 10
#endif

/**
 * struct ppdu_info - PPDU Status info descriptor
 * @ppdu_id: Unique ppduid assigned by firmware for every tx packet
 * @sched_cmdid: schedule command id, which will be same in a burst
 * @max_ppdu_id: wrap around for ppdu id
 * @last_tlv_cnt: Keep track for missing ppdu tlvs
 * @last_user: last ppdu processed for user
 * @is_ampdu: set if Ampdu aggregate
 * @nbuf: ppdu descriptor payload
 * @ppdu_desc: ppdu descriptor
 * @ppdu_info_list_elem: linked list of ppdu tlvs
 * @ppdu_info_queue_elem: Singly linked list (queue) of ppdu tlvs
 * @mpdu_compltn_common_tlv: Successful tlv counter from COMPLTN COMMON tlv
 * @mpdu_ack_ba_tlv: Successful tlv counter from ACK BA tlv
 */
struct ppdu_info {
	uint32_t ppdu_id;
	uint32_t sched_cmdid;
	uint32_t max_ppdu_id;
	uint32_t tsf_l32;
	uint16_t tlv_bitmap;
	uint16_t last_tlv_cnt;
	uint16_t last_user:8,
		 is_ampdu:1;
	qdf_nbuf_t nbuf;
	struct cdp_tx_completion_ppdu *ppdu_desc;
#ifdef WLAN_TX_PKT_CAPTURE_ENH
	union {
		TAILQ_ENTRY(ppdu_info) ppdu_info_dlist_elem;
		STAILQ_ENTRY(ppdu_info) ppdu_info_slist_elem;
	} ulist;
#define ppdu_info_list_elem ulist.ppdu_info_dlist_elem
#define ppdu_info_queue_elem ulist.ppdu_info_slist_elem
#else
	TAILQ_ENTRY(ppdu_info) ppdu_info_list_elem;
#endif
	uint8_t compltn_common_tlv;
	uint8_t ack_ba_tlv;
	bool done;
};

/**
 * struct msdu_completion_info - wbm msdu completion info
 * @ppdu_id            - Unique ppduid assigned by firmware for every tx packet
 * @peer_id            - peer_id
 * @tid                - tid which used during transmit
 * @first_msdu         - first msdu indication
 * @last_msdu          - last msdu indication
 * @msdu_part_of_amsdu - msdu part of amsdu
 * @transmit_cnt       - retried count
 * @status             - transmit status
 * @tsf                - timestamp which it transmitted
 */
struct msdu_completion_info {
	uint32_t ppdu_id;
	uint16_t peer_id;
	uint8_t tid;
	uint8_t first_msdu:1,
		last_msdu:1,
		msdu_part_of_amsdu:1;
	uint8_t transmit_cnt;
	uint8_t status;
	uint32_t tsf;
};

#ifdef WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG
struct rx_protocol_tag_map {
	/* This is the user configured tag for the said protocol type */
	uint16_t tag;
};

#ifdef WLAN_SUPPORT_RX_TAG_STATISTICS
struct rx_protocol_tag_stats {
	uint32_t tag_ctr;
};
#endif /* WLAN_SUPPORT_RX_TAG_STATISTICS */

#endif /* WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG */

#ifndef WLAN_TX_PKT_CAPTURE_ENH
struct dp_pdev_tx_capture {
};

struct dp_peer_tx_capture {
};
#endif
#ifdef WLAN_RX_PKT_CAPTURE_ENH
/* Template data to be set for Enhanced RX Monitor packets */
#define RX_MON_CAP_ENH_TRAILER 0xdeadc0dedeadda7a

/**
 * struct dp_rx_mon_enh_trailer_data - Data structure to set a known pattern
 * at end of each MSDU in monitor-lite mode
 * @reserved1: reserved for future use
 * @reserved2: reserved for future use
 * @flow_tag: flow tag value read from skb->cb
 * @protocol_tag: protocol tag value read from skb->cb
 */
struct dp_rx_mon_enh_trailer_data {
	uint16_t reserved1;
	uint16_t reserved2;
	uint16_t flow_tag;
	uint16_t protocol_tag;
};
#endif /* WLAN_RX_PKT_CAPTURE_ENH */

#ifdef HTT_STATS_DEBUGFS_SUPPORT
/* Number of debugfs entries created for HTT stats */
#define PDEV_HTT_STATS_DBGFS_SIZE HTT_DBG_NUM_EXT_STATS

/* struct pdev_htt_stats_dbgfs_priv - Structure to maintain debugfs information
 * of HTT stats
 * @pdev: dp pdev of debugfs entry
 * @stats_id: stats id of debugfs entry
 */
struct pdev_htt_stats_dbgfs_priv {
	struct dp_pdev *pdev;
	uint16_t stats_id;
};

/* struct pdev_htt_stats_dbgfs_cfg - PDEV level data structure for debugfs
 * support for HTT stats
 * @debugfs_entry: qdf_debugfs directory entry
 * @m: qdf debugfs file handler
 * @pdev_htt_stats_dbgfs_ops: File operations of entry created
 * @priv: HTT stats debugfs private object
 * @htt_stats_dbgfs_event: HTT stats event for debugfs support
 * @lock: HTT stats debugfs lock
 * @htt_stats_dbgfs_msg_process: Function callback to print HTT stats
 */
struct pdev_htt_stats_dbgfs_cfg {
	qdf_dentry_t debugfs_entry[PDEV_HTT_STATS_DBGFS_SIZE];
	qdf_debugfs_file_t m;
	struct qdf_debugfs_fops
			pdev_htt_stats_dbgfs_ops[PDEV_HTT_STATS_DBGFS_SIZE - 1];
	struct pdev_htt_stats_dbgfs_priv priv[PDEV_HTT_STATS_DBGFS_SIZE - 1];
	qdf_event_t htt_stats_dbgfs_event;
	qdf_mutex_t lock;
	void (*htt_stats_dbgfs_msg_process)(void *data, A_INT32 len);
};
#endif /* HTT_STATS_DEBUGFS_SUPPORT */

/* PDEV level structure for data path */
struct dp_pdev {
	/**
	 * Re-use Memory Section Starts
	 */

	/* PDEV Id */
	int pdev_id;

	/* LMAC Id */
	int lmac_id;

	/* Target pdev  Id */
	int target_pdev_id;

	/* TXRX SOC handle */
	struct dp_soc *soc;

	/* Stuck count on monitor destination ring MPDU process */
	uint32_t mon_dest_ring_stuck_cnt;

	bool pdev_deinit;

	/* pdev status down or up required to handle dynamic hw
	 * mode switch between DBS and DBS_SBS.
	 * 1 = down
	 * 0 = up
	 */
	bool is_pdev_down;

	/* Second ring used to replenish rx buffers */
	struct dp_srng rx_refill_buf_ring2;

	/* Empty ring used by firmware to post rx buffers to the MAC */
	struct dp_srng rx_mac_buf_ring[MAX_RX_MAC_RINGS];

	int ch_band_lmac_id_mapping[REG_BAND_UNKNOWN];

	/* wlan_cfg pdev ctxt*/
	 struct wlan_cfg_dp_pdev_ctxt *wlan_cfg_ctx;

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

	/* vdev list lock */
	qdf_spinlock_t vdev_list_lock;

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

	/* Monitor mode operation channel */
	int mon_chan_num;

	/* Monitor mode operation frequency */
	qdf_freq_t mon_chan_freq;

	/* Monitor mode band */
	enum reg_wifi_band mon_chan_band;

	/* monitor mode lock */
	qdf_spinlock_t mon_lock;

	/*tx_mutex for me*/
	DP_MUTEX_TYPE tx_mutex;

	/* monitor */
	bool monitor_configured;

	/* Smart Mesh */
	bool filter_neighbour_peers;

	/*flag to indicate neighbour_peers_list not empty */
	bool neighbour_peers_added;
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
	uint16_t md_data_filter;

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
	struct {
		uint8_t num;
		uint8_t band;
		uint16_t freq;
	} operating_channel;

	qdf_nbuf_queue_t rx_status_q;
	uint32_t mon_ppdu_status;
	struct cdp_mon_status rx_mon_recv_status;
	/* monitor mode status/destination ring PPDU and MPDU count */
	struct cdp_pdev_mon_stats rx_mon_stats;
	/* to track duplicate link descriptor indications by HW for a WAR */
	uint64_t mon_last_linkdesc_paddr;
	/* to track duplicate buffer indications by HW for a WAR */
	uint32_t mon_last_buf_cookie;
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
	/* pool addr for mcast enhance buff */
	struct {
		int size;
		uint32_t paddr;
		char *vaddr;
		struct dp_tx_me_buf_t *freelist;
		int buf_in_use;
		qdf_dma_mem_context(memctx);
	} me_buf;

	bool hmmc_tid_override_en;
	uint8_t hmmc_tid;

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

	/* Packet log mode */
	uint8_t rx_pktlog_mode;

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
	enum m_copy_mode mcopy_mode;
	bool cfr_rcc_mode;
	bool enable_reap_timer_non_pkt;
	bool bpr_enable;

	/* enable time latency check for tx completion */
	bool latency_capture_enable;

	/* enable calculation of delay stats*/
	bool delay_stats_flag;
	struct {
		uint32_t tx_ppdu_id;
		uint16_t tx_peer_id;
		uint32_t rx_ppdu_id;
	} m_copy_id;

	/* To check if PPDU Tx stats are enabled for Pktlog */
	bool pktlog_ppdu_stats;

	void *dp_txrx_handle; /* Advanced data path handle */

#ifdef ATH_SUPPORT_NAC_RSSI
	bool nac_rssi_filtering;
#endif
	/* list of ppdu tlvs */
	TAILQ_HEAD(, ppdu_info) ppdu_info_list;
	TAILQ_HEAD(, ppdu_info) sched_comp_ppdu_list;

	uint32_t sched_comp_list_depth;
	uint16_t delivered_sched_cmdid;
	uint16_t last_sched_cmdid;
	uint32_t tlv_count;
	uint32_t list_depth;
	uint32_t ppdu_id;
	bool first_nbuf;
	struct {
		qdf_nbuf_t last_nbuf; /*Ptr to mgmt last buf */
		uint8_t *mgmt_buf; /* Ptr to mgmt. payload in HTT ppdu stats */
		uint32_t mgmt_buf_len; /* Len of mgmt. payload in ppdu stats */
		uint32_t ppdu_id;
	} mgmtctrl_frm_info;

	/* Current noise-floor reading for the pdev channel */
	int16_t chan_noise_floor;

	/*
	 * For multiradio device, this flag indicates if
	 * this radio is primary or secondary.
	 *
	 * For HK 1.0, this is used for WAR for the AST issue.
	 * HK 1.x mandates creation of only 1 AST entry with same MAC address
	 * across 2 radios. is_primary indicates the radio on which DP should
	 * install HW AST entry if there is a request to add 2 AST entries
	 * with same MAC address across 2 radios
	 */
	uint8_t is_primary;
	/* Context of cal client timer */
	struct cdp_cal_client *cal_client_ctx;
	struct cdp_tx_sojourn_stats sojourn_stats;
	qdf_nbuf_t sojourn_buf;

	/* peer pointer for collecting invalid peer stats */
	struct dp_peer *invalid_peer;

	union dp_rx_desc_list_elem_t *free_list_head;
	union dp_rx_desc_list_elem_t *free_list_tail;
	/* Pdev level flag to check peer based pktlog enabled or
	 * disabled
	 */
	uint8_t dp_peer_based_pktlog;

	/* Cached peer_id from htt_peer_details_tlv */
	uint16_t fw_stats_peer_id;

	/* qdf_event for fw_peer_stats */
	qdf_event_t fw_peer_stats_event;

	/* User configured max number of tx buffers */
	uint32_t num_tx_allowed;

	/* unique cookie required for peer session */
	uint32_t next_peer_cookie;

	/*
	 * Run time enabled when the first protocol tag is added,
	 * run time disabled when the last protocol tag is deleted
	 */
	bool  is_rx_protocol_tagging_enabled;

#ifdef WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG
	/*
	 * The protocol type is used as array index to save
	 * user provided tag info
	 */
	struct rx_protocol_tag_map rx_proto_tag_map[RX_PROTOCOL_TAG_MAX];

#ifdef WLAN_SUPPORT_RX_TAG_STATISTICS
	/*
	 * Track msdus received from each reo ring separately to avoid
	 * simultaneous writes from different core
	 */
	struct rx_protocol_tag_stats
		reo_proto_tag_stats[MAX_REO_DEST_RINGS][RX_PROTOCOL_TAG_MAX];
	/* Track msdus received from expection ring separately */
	struct rx_protocol_tag_stats
		rx_err_proto_tag_stats[RX_PROTOCOL_TAG_MAX];
#endif /* WLAN_SUPPORT_RX_TAG_STATISTICS */
#endif /* WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG */

	/* tx packet capture enhancement */
	enum cdp_tx_enh_capture_mode tx_capture_enabled;
	struct dp_pdev_tx_capture tx_capture;

	uint32_t *ppdu_tlv_buf; /* Buffer to hold HTT ppdu stats TLVs*/

#ifdef WLAN_SUPPORT_RX_FLOW_TAG
	/**
	 * Pointer to DP Flow FST at SOC level if
	 * is_rx_flow_search_table_per_pdev is true
	 */
	struct dp_rx_fst *rx_fst;
#endif /* WLAN_SUPPORT_RX_FLOW_TAG */

#ifdef FEATURE_TSO_STATS
	/* TSO Id to index into TSO packet information */
	qdf_atomic_t tso_idx;
#endif /* FEATURE_TSO_STATS */

#ifdef WLAN_SUPPORT_DATA_STALL
	data_stall_detect_cb data_stall_detect_callback;
#endif /* WLAN_SUPPORT_DATA_STALL */

	struct dp_mon_filter **filter;	/* Monitor Filter pointer */

#ifdef QCA_SUPPORT_FULL_MON
	/* List to maintain all MPDUs for a PPDU in monitor mode */
	TAILQ_HEAD(, dp_mon_mpdu) mon_mpdu_q;

	/* TODO: define per-user mpdu list
	 * struct dp_mon_mpdu_list mpdu_list[MAX_MU_USERS];
	 */
	struct hal_rx_mon_desc_info *mon_desc;
#endif
	qdf_nbuf_t mcopy_status_nbuf;

	/* Flag to hold on to monitor destination ring */
	bool hold_mon_dest_ring;

#ifdef WLAN_ATF_ENABLE
	/* ATF stats enable */
	bool dp_atf_stats_enable;
#endif

	/* Maintains first status buffer's paddr of a PPDU */
	uint64_t status_buf_addr;
#ifdef HTT_STATS_DEBUGFS_SUPPORT
	/* HTT stats debugfs params */
	struct pdev_htt_stats_dbgfs_cfg *dbgfs_cfg;
#endif
};

struct dp_peer;

/* VDEV structure for data path state */
struct dp_vdev {
	/* OS device abstraction */
	qdf_device_t osdev;

	/* physical device that is the parent of this virtual device */
	struct dp_pdev *pdev;

	/* VDEV operating mode */
	enum wlan_op_mode opmode;

	/* VDEV subtype */
	enum wlan_op_subtype subtype;

	/* Tx encapsulation type for this VAP */
	enum htt_cmn_pkt_type tx_encap_type;

	/* Rx Decapsulation type for this VAP */
	enum htt_cmn_pkt_type rx_decap_type;

	/* WDS enabled */
	bool wds_enabled;

	/* MEC enabled */
	bool mec_enabled;

#ifdef QCA_SUPPORT_WDS_EXTENDED
	bool wds_ext_enabled;
#endif /* QCA_SUPPORT_WDS_EXTENDED */

	/* WDS Aging timer period */
	uint32_t wds_aging_timer_val;

	/* NAWDS enabled */
	bool nawds_enabled;

	/* Multicast enhancement enabled */
	uint8_t mcast_enhancement_en;

	/* IGMP multicast enhancement enabled */
	uint8_t igmp_mcast_enhanc_en;

	/* HW TX Checksum Enabled Flag */
	uint8_t csum_enabled;

	/* vdev_id - ID used to specify a particular vdev to the target */
	uint8_t vdev_id;

	/* Default HTT meta data for this VDEV */
	/* TBD: check alignment constraints */
	uint16_t htt_tcl_metadata;

	/* Mesh mode vdev */
	uint32_t mesh_vdev;

	/* Mesh mode rx filter setting */
	uint32_t mesh_rx_filter;

	/* DSCP-TID mapping table ID */
	uint8_t dscp_tid_map_id;

	/* Address search type to be set in TX descriptor */
	uint8_t search_type;

	/*
	 * Flag to indicate if s/w tid classification should be
	 * skipped
	 */
	uint8_t skip_sw_tid_classification;

	/* AST hash value for BSS peer in HW valid for STA VAP*/
	uint16_t bss_ast_hash;

	/* vdev lmac_id */
	int lmac_id;

	bool multipass_en;

	/* Address search flags to be configured in HAL descriptor */
	uint8_t hal_desc_addr_search_flags;

	/* Handle to the OS shim SW's virtual device */
	ol_osif_vdev_handle osif_vdev;

	/* MAC address */
	union dp_align_mac_addr mac_addr;

	/* node in the pdev's list of vdevs */
	TAILQ_ENTRY(dp_vdev) vdev_list_elem;

	/* dp_peer list */
	TAILQ_HEAD(, dp_peer) peer_list;
	/* to protect peer_list */
	DP_MUTEX_TYPE peer_list_lock;

	/* RX call back function to flush GRO packets*/
	ol_txrx_rx_gro_flush_ind_fp osif_gro_flush;
	/* default RX call back function called by dp */
	ol_txrx_rx_fp osif_rx;
	/* callback to deliver rx frames to the OS */
	ol_txrx_rx_fp osif_rx_stack;
	/* Callback to handle rx fisa frames */
	ol_txrx_fisa_rx_fp osif_fisa_rx;
	ol_txrx_fisa_flush_fp osif_fisa_flush;

	/* call back function to flush out queued rx packets*/
	ol_txrx_rx_flush_fp osif_rx_flush;
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

	/* completion function used by this vdev*/
	ol_txrx_completion_fp tx_comp;

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

	/* per vdev rx nbuf queue */
	qdf_nbuf_queue_t rxq;

	uint8_t tx_ring_id;
	struct dp_tx_desc_pool_s *tx_desc;
	struct dp_tx_ext_desc_pool_s *tx_ext_desc;

	/* VDEV Stats */
	struct cdp_vdev_stats stats;

	/* Is this a proxySTA VAP */
	bool proxysta_vdev;
	/* Is isolation mode enabled */
	bool isolation_vdev;

#ifdef QCA_LL_TX_FLOW_CONTROL_V2
	struct dp_tx_desc_pool_s *pool;
#endif
	/* AP BRIDGE enabled */
	bool ap_bridge_enabled;

	enum cdp_sec_type  sec_type;

	/* SWAR for HW: Enable WEP bit in the AMSDU frames for RAW mode */
	bool raw_mode_war;


	/* AST hash index for BSS peer in HW valid for STA VAP*/
	uint16_t bss_ast_idx;

	/* Capture timestamp of previous tx packet enqueued */
	uint64_t prev_tx_enq_tstamp;

	/* Capture timestamp of previous rx packet delivered */
	uint64_t prev_rx_deliver_tstamp;

	/* 8021p PCP-TID mapping table ID */
	uint8_t tidmap_tbl_id;

	/* 8021p PCP-TID map values */
	uint8_t pcp_tid_map[PCP_TID_MAP_MAX];

	/* TIDmap priority */
	uint8_t tidmap_prty;

#ifdef QCA_MULTIPASS_SUPPORT
	uint16_t *iv_vlan_map;

	/* dp_peer special list */
	TAILQ_HEAD(, dp_peer) mpass_peer_list;
	DP_MUTEX_TYPE mpass_peer_mutex;
#endif
	/* Extended data path handle */
	struct cdp_ext_vdev *vdev_dp_ext_handle;
#ifdef VDEV_PEER_PROTOCOL_COUNT
	/*
	 * Rx-Ingress and Tx-Egress are in the lower level DP layer
	 * Rx-Egress and Tx-ingress are handled in osif layer for DP
	 * So
	 * Rx-Egress and Tx-ingress mask definitions are in OSIF layer
	 * Rx-Ingress and Tx-Egress definitions are here below
	 */
#define VDEV_PEER_PROTOCOL_RX_INGRESS_MASK 1
#define VDEV_PEER_PROTOCOL_TX_INGRESS_MASK 2
#define VDEV_PEER_PROTOCOL_RX_EGRESS_MASK 4
#define VDEV_PEER_PROTOCOL_TX_EGRESS_MASK 8
	bool peer_protocol_count_track;
	int peer_protocol_count_dropmask;
#endif
	/* callback to collect connectivity stats */
	ol_txrx_stats_rx_fp stats_cb;
	uint32_t num_peers;
	/* entry to inactive_list*/
	TAILQ_ENTRY(dp_vdev) inactive_list_elem;

#ifdef WLAN_SUPPORT_RX_FISA
	/**
	 * Params used for controlling the fisa aggregation dynamically
	 */
	uint8_t fisa_disallowed[MAX_REO_DEST_RINGS];
	uint8_t fisa_force_flushed[MAX_REO_DEST_RINGS];
#endif
	/*
	 * Refcount for VDEV currently incremented when
	 * peer is created for VDEV
	 */
	qdf_atomic_t ref_cnt;
	qdf_atomic_t mod_refs[DP_MOD_ID_MAX];
	uint8_t num_latency_critical_conn;
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

/*
 * struct dp_peer_cached_bufq - cached_bufq to enqueue rx packets
 * @cached_bufq: nbuff list to enqueue rx packets
 * @bufq_lock: spinlock for nbuff list access
 * @thres: maximum threshold for number of rx buff to enqueue
 * @entries: number of entries
 * @dropped: number of packets dropped
 */
struct dp_peer_cached_bufq {
	qdf_list_t cached_bufq;
	qdf_spinlock_t bufq_lock;
	uint32_t thresh;
	uint32_t entries;
	uint32_t dropped;
};

/**
 * enum dp_peer_ast_flowq
 * @DP_PEER_AST_FLOWQ_HI_PRIO: Hi Priority flow queue
 * @DP_PEER_AST_FLOWQ_LOW_PRIO: Low priority flow queue
 * @DP_PEER_AST_FLOWQ_UDP: flow queue type is UDP
 * @DP_PEER_AST_FLOWQ_NON_UDP: flow queue type is Non UDP
 */
enum dp_peer_ast_flowq {
	DP_PEER_AST_FLOWQ_HI_PRIO,
	DP_PEER_AST_FLOWQ_LOW_PRIO,
	DP_PEER_AST_FLOWQ_UDP,
	DP_PEER_AST_FLOWQ_NON_UDP,
	DP_PEER_AST_FLOWQ_MAX,
};

/*
 * struct dp_ast_flow_override_info - ast override info
 * @ast_index - ast indexes in peer map message
 * @ast_valid_mask - ast valid mask for each ast index
 * @ast_flow_mask - ast flow mask for each ast index
 * @tid_valid_low_pri_mask - per tid mask for low priority flow
 * @tid_valid_hi_pri_mask - per tid mask for hi priority flow
 */
struct dp_ast_flow_override_info {
	uint16_t ast_idx[DP_PEER_AST_FLOWQ_MAX];
	uint8_t ast_valid_mask;
	uint8_t ast_flow_mask[DP_PEER_AST_FLOWQ_MAX];
	uint8_t tid_valid_low_pri_mask;
	uint8_t tid_valid_hi_pri_mask;
};

/*
 * struct dp_peer_ast_params - ast parameters for a msdu flow-queue
 * @ast_index - ast index populated by FW
 * @is_valid - ast flow valid mask
 * @valid_tid_mask - per tid mask for this ast index
 * @flowQ - flow queue id associated with this ast index
 */
struct dp_peer_ast_params {
	uint16_t ast_idx;
	uint8_t is_valid;
	uint8_t valid_tid_mask;
	uint8_t flowQ;
};

#ifdef WLAN_SUPPORT_MSCS
/*MSCS Procedure based macros */
#define IEEE80211_MSCS_MAX_ELEM_SIZE    5
#define IEEE80211_TCLAS_MASK_CLA_TYPE_4  4
/*
 * struct dp_peer_mscs_parameter - MSCS database obtained from
 * MSCS Request and Response in the control path. This data is used
 * by the AP to find out what priority to set based on the tuple
 * classification during packet processing.
 * @user_priority_bitmap - User priority bitmap obtained during
 * handshake
 * @user_priority_limit - User priority limit obtained during
 * handshake
 * @classifier_mask - params to be compared during processing
 */
struct dp_peer_mscs_parameter {
	uint8_t user_priority_bitmap;
	uint8_t user_priority_limit;
	uint8_t classifier_mask;
};
#endif

#ifdef QCA_SUPPORT_WDS_EXTENDED
#define WDS_EXT_PEER_INIT_BIT 0

/**
 * struct dp_wds_ext_peer - wds ext peer structure
 * This is used when wds extended feature is enabled
 * both compile time and run time. It is created
 * when 1st 4 address frame is received from
 * wds backhaul.
 * @osif_vdev: Handle to the OS shim SW's virtual device
 * @init: wds ext netdev state
 */
struct dp_wds_ext_peer {
	ol_osif_peer_handle osif_peer;
	unsigned long init;
};
#endif /* QCA_SUPPORT_WDS_EXTENDED */

/* Peer structure for data path state */
struct dp_peer {
	/* VDEV to which this peer is associated */
	struct dp_vdev *vdev;

	struct dp_ast_entry *self_ast_entry;

	qdf_atomic_t ref_cnt;

	/* peer ID for this peer */
	uint16_t peer_id;

	union dp_align_mac_addr mac_addr;

	/* node in the vdev's list of peers */
	TAILQ_ENTRY(dp_peer) peer_list_elem;
	/* node in the hash table bin's list of peers */
	TAILQ_ENTRY(dp_peer) hash_list_elem;

	/* TID structures */
	struct dp_rx_tid rx_tid[DP_MAX_TIDS];
	struct dp_peer_tx_capture tx_capture;


	/* TBD: No transmit TID state required? */

	struct {
		enum cdp_sec_type sec_type;
		u_int32_t michael_key[2]; /* relevant for TKIP */
	} security[2]; /* 0 -> multicast, 1 -> unicast */

	/* NAWDS Flag and Bss Peer bit */
	uint16_t nawds_enabled:1, /* NAWDS flag */
		bss_peer:1, /* set for bss peer */
		wds_enabled:1, /* WDS peer */
		authorize:1, /* Set when authorized */
		nac:1, /* NAC Peer*/
		tx_cap_enabled:1, /* Peer's tx-capture is enabled */
		rx_cap_enabled:1, /* Peer's rx-capture is enabled */
		valid:1, /* valid bit */
		in_twt:1, /* in TWT session */
		delete_in_progress:1, /* Indicate kickout sent */
		sta_self_peer:1; /* Indicate STA self peer */

#ifdef QCA_SUPPORT_PEER_ISOLATION
	bool isolation; /* enable peer isolation for this peer */
#endif

	/* MCL specific peer local id */
	uint16_t local_id;
	enum ol_txrx_peer_state state;
	qdf_spinlock_t peer_info_lock;

	/* Peer Stats */
	struct cdp_peer_stats stats;

	/* Peer extended stats */
	struct cdp_peer_ext_stats *pext_stats;

	TAILQ_HEAD(, dp_ast_entry) ast_entry_list;
	/* TBD */

#ifdef WDS_VENDOR_EXTENSION
	dp_ecm_policy wds_ecm;
#endif

	/* Active Block ack sessions */
	uint16_t active_ba_session_cnt;

	/* Current HW buffersize setting */
	uint16_t hw_buffer_size;

	/*
	 * Flag to check if sessions with 256 buffersize
	 * should be terminated.
	 */
	uint8_t kill_256_sessions;
	qdf_atomic_t is_default_route_set;
	/* Peer level flag to check peer based pktlog enabled or
	 * disabled
	 */
	uint8_t peer_based_pktlog_filter;

	/* rdk statistics context */
	struct cdp_peer_rate_stats_ctx *rdkstats_ctx;
	/* average sojourn time */
	qdf_ewma_tx_lag avg_sojourn_msdu[CDP_DATA_TID_MAX];

#ifdef QCA_MULTIPASS_SUPPORT
	/* node in the special peer list element */
	TAILQ_ENTRY(dp_peer) mpass_peer_list_elem;
	/* vlan id for key */
	uint16_t vlan_id;
#endif

#ifdef PEER_CACHE_RX_PKTS
	qdf_atomic_t flush_in_progress;
	struct dp_peer_cached_bufq bufq_info;
#endif
#ifdef FEATURE_PERPKT_INFO
	/* delayed ba ppdu stats handling */
	struct cdp_delayed_tx_completion_ppdu_user delayed_ba_ppdu_stats;
	/* delayed ba flag */
	bool last_delayed_ba;
	/* delayed ba ppdu id */
	uint32_t last_delayed_ba_ppduid;
#endif
#ifdef QCA_PEER_MULTIQ_SUPPORT
	struct dp_peer_ast_params peer_ast_flowq_idx[DP_PEER_AST_FLOWQ_MAX];
#endif
	/* entry to inactive_list*/
	TAILQ_ENTRY(dp_peer) inactive_list_elem;

	qdf_atomic_t mod_refs[DP_MOD_ID_MAX];

	uint8_t peer_state;
	qdf_spinlock_t peer_state_lock;
#ifdef WLAN_SUPPORT_MSCS
	struct dp_peer_mscs_parameter mscs_ipv4_parameter, mscs_ipv6_parameter;
	bool mscs_active;
#endif
#ifdef QCA_SUPPORT_WDS_EXTENDED
	struct dp_wds_ext_peer wds_ext;
	ol_txrx_rx_fp osif_rx;
#endif
};

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

/*
 * dp_tx_me_buf_t: ME buffer
 * next: pointer to next buffer
 * data: Destination Mac address
 * paddr_macbuf: physical address for dest_mac
 */
struct dp_tx_me_buf_t {
	/* Note: ME buf pool initialization logic expects next pointer to
	 * be the first element. Dont add anything before next */
	struct dp_tx_me_buf_t *next;
	uint8_t data[QDF_MAC_ADDR_SIZE];
	qdf_dma_addr_t paddr_macbuf;
};

#if defined(WLAN_SUPPORT_RX_FLOW_TAG) || defined(WLAN_SUPPORT_RX_FISA)
struct hal_rx_fst;

#ifdef WLAN_SUPPORT_RX_FLOW_TAG
struct dp_rx_fse {
	/* HAL Rx Flow Search Entry which matches HW definition */
	void *hal_rx_fse;
	/* Toeplitz hash value */
	uint32_t flow_hash;
	/* Flow index, equivalent to hash value truncated to FST size */
	uint32_t flow_id;
	/* Stats tracking for this flow */
	struct cdp_flow_stats stats;
	/* Flag indicating whether flow is IPv4 address tuple */
	uint8_t is_ipv4_addr_entry;
	/* Flag indicating whether flow is valid */
	uint8_t is_valid;
};

struct dp_rx_fst {
	/* Software (DP) FST */
	uint8_t *base;
	/* Pointer to HAL FST */
	struct hal_rx_fst *hal_rx_fst;
	/* Base physical address of HAL RX HW FST */
	uint64_t hal_rx_fst_base_paddr;
	/* Maximum number of flows FSE supports */
	uint16_t max_entries;
	/* Num entries in flow table */
	uint16_t num_entries;
	/* SKID Length */
	uint16_t max_skid_length;
	/* Hash mask to obtain legitimate hash entry */
	uint32_t hash_mask;
	/* Timer for bundling of flows */
	qdf_timer_t cache_invalidate_timer;
	/**
	 * Flag which tracks whether cache update
	 * is needed on timer expiry
	 */
	qdf_atomic_t is_cache_update_pending;
	/* Flag to indicate completion of FSE setup in HW/FW */
	bool fse_setup_done;
};

#define DP_RX_GET_SW_FT_ENTRY_SIZE sizeof(struct dp_rx_fse)
#elif WLAN_SUPPORT_RX_FISA

struct dp_fisa_stats {
	/* flow index invalid from RX HW TLV */
	uint32_t invalid_flow_index;
	uint32_t reo_mismatch;
};

enum fisa_aggr_ret {
	FISA_AGGR_DONE,
	FISA_AGGR_NOT_ELIGIBLE,
	FISA_FLUSH_FLOW
};

struct dp_fisa_rx_sw_ft {
	/* HAL Rx Flow Search Entry which matches HW definition */
	void *hw_fse;
	/* Toeplitz hash value */
	uint32_t flow_hash;
	/* Flow index, equivalent to hash value truncated to FST size */
	uint32_t flow_id;
	/* Stats tracking for this flow */
	struct cdp_flow_stats stats;
	/* Flag indicating whether flow is IPv4 address tuple */
	uint8_t is_ipv4_addr_entry;
	/* Flag indicating whether flow is valid */
	uint8_t is_valid;
	uint8_t is_populated;
	uint8_t is_flow_udp;
	uint8_t is_flow_tcp;
	qdf_nbuf_t head_skb;
	uint16_t cumulative_l4_checksum;
	uint16_t adjusted_cumulative_ip_length;
	uint16_t cur_aggr;
	uint16_t napi_flush_cumulative_l4_checksum;
	uint16_t napi_flush_cumulative_ip_length;
	qdf_nbuf_t last_skb;
	uint32_t head_skb_ip_hdr_offset;
	uint32_t head_skb_l4_hdr_offset;
	struct cdp_rx_flow_tuple_info rx_flow_tuple_info;
	uint8_t napi_id;
	struct dp_vdev *vdev;
	uint64_t bytes_aggregated;
	uint32_t flush_count;
	uint32_t aggr_count;
	uint8_t do_not_aggregate;
	uint16_t hal_cumultive_ip_len;
	struct dp_soc *soc_hdl;
	/* last aggregate count fetched from RX PKT TLV */
	uint32_t last_hal_aggr_count;
	uint32_t cur_aggr_gso_size;
	struct udphdr *head_skb_udp_hdr;
	uint16_t frags_cumulative_len;
	/* CMEM parameters */
	uint32_t cmem_offset;
	uint32_t metadata;
};

#define DP_RX_GET_SW_FT_ENTRY_SIZE sizeof(struct dp_fisa_rx_sw_ft)
#define MAX_FSE_CACHE_FL_HST 10
/**
 * struct fse_cache_flush_history - Debug history cache flush
 * @timestamp: Entry update timestamp
 * @flows_added: Number of flows added for this flush
 * @flows_deleted: Number of flows deleted for this flush
 */
struct fse_cache_flush_history {
	uint64_t timestamp;
	uint32_t flows_added;
	uint32_t flows_deleted;
};

struct dp_rx_fst {
	/* Software (DP) FST */
	uint8_t *base;
	/* Pointer to HAL FST */
	struct hal_rx_fst *hal_rx_fst;
	/* Base physical address of HAL RX HW FST */
	uint64_t hal_rx_fst_base_paddr;
	/* Maximum number of flows FSE supports */
	uint16_t max_entries;
	/* Num entries in flow table */
	uint16_t num_entries;
	/* SKID Length */
	uint16_t max_skid_length;
	/* Hash mask to obtain legitimate hash entry */
	uint32_t hash_mask;
	/* Lock for adding/deleting entries of FST */
	qdf_spinlock_t dp_rx_fst_lock;
	uint32_t add_flow_count;
	uint32_t del_flow_count;
	uint32_t hash_collision_cnt;
	struct dp_soc *soc_hdl;
	qdf_atomic_t fse_cache_flush_posted;
	qdf_timer_t fse_cache_flush_timer;
	struct fse_cache_flush_history cache_fl_rec[MAX_FSE_CACHE_FL_HST];
	/* FISA DP stats */
	struct dp_fisa_stats stats;

	/* CMEM params */
	qdf_work_t fst_update_work;
	qdf_workqueue_t *fst_update_wq;
	qdf_list_t fst_update_list;
	uint32_t meta_counter;
	uint32_t cmem_ba;
	qdf_spinlock_t dp_rx_sw_ft_lock[MAX_REO_DEST_RINGS];
	qdf_event_t cmem_resp_event;
	bool flow_deletion_supported;
	bool fst_in_cmem;
};

#endif /* WLAN_SUPPORT_RX_FISA */
#endif /* WLAN_SUPPORT_RX_FLOW_TAG || WLAN_SUPPORT_RX_FISA */

#ifdef WLAN_FEATURE_STATS_EXT
/*
 * dp_req_rx_hw_stats_t: RX peer HW stats query structure
 * @pending_tid_query_cnt: pending tid stats count which waits for REO status
 * @is_query_timeout: flag to show is stats query timeout
 */
struct dp_req_rx_hw_stats_t {
	qdf_atomic_t pending_tid_stats_cnt;
	bool is_query_timeout;
};
#endif

void dp_hw_link_desc_pool_banks_free(struct dp_soc *soc, uint32_t mac_id);
QDF_STATUS dp_hw_link_desc_pool_banks_alloc(struct dp_soc *soc,
					    uint32_t mac_id);
void dp_link_desc_ring_replenish(struct dp_soc *soc, uint32_t mac_id);

#endif /* _DP_TYPES_H_ */
