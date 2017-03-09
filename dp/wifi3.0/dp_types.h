/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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
#ifndef CONFIG_WIN
#include <wdi_event_api.h>    /* WDI subscriber event list */
#endif

#include <hal_tx.h>
#include <hal_reo.h>
#include "wlan_cfg.h"
#include "hal_rx.h"
#include <hal_api.h>
#include <hal_api_mon.h>

#define MAX_TCL_RING 3
#define MAX_RXDMA_ERRORS 32
#define SUPPORTED_BW 4
#define SUPPORTED_RECEPTION_TYPES 4
#define REPT_MU_MIMO 1
#define REPT_MU_OFDMA_MIMO 3
#define REO_ERROR_TYPE_MAX (HAL_REO_ERR_QUEUE_DESC_BLOCKED_SET+1)
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
#define DP_MAX_TX_RINGS 8
#define DP_MAX_RX_RINGS 8
#define MAX_IDLE_SCATTER_BUFS 16
#define DP_MAX_IRQ_PER_CONTEXT 12
#define DP_MAX_INTERRUPT_CONTEXTS 8
#define DEFAULT_HW_PEER_ID 0xffff

#define MAX_TX_HW_QUEUES 3

#define DP_MAX_INTERRUPT_CONTEXTS 8

struct dp_soc_cmn;
struct dp_pdev;
struct dp_vdev;
union dp_tx_desc_list_elem_t;
struct dp_soc;
union dp_rx_desc_list_elem_t;

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
 * @pool_id: Pool ID - used when releasing the descripto
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
 */
struct dp_tx_desc_s {
	struct dp_tx_desc_s *next;
	qdf_nbuf_t nbuf;
	struct dp_tx_ext_desc_elem_s *msdu_ext_desc;
	uint32_t  id;
	struct dp_vdev *vdev;
	struct dp_pdev *pdev;
	uint8_t  pool_id;
	uint8_t flags;
	struct hal_tx_desc_comp_s comp;
	uint16_t tx_encap_type;
	uint8_t frm_type;
	uint8_t pkt_offset;
	void *me_buffer;
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
 * struct dp_tx_desc_pool_s - Tx Descriptor pool information
 * @elem_size: Size of each descriptor in the pool
 * @elem_count: Total number of descriptors in the pool
 * @num_allocated: Number of used descriptors
 * @num_free: Number of free descriptors
 * @freelist: Chain of free descriptors
 * @desc_pages: multiple page allocation information for actual descriptors
 * @lock- Lock for descriptor allocation/free from/to the pool
 */
struct dp_tx_desc_pool_s {
	uint16_t elem_size;
	uint16_t elem_count;
	uint32_t num_allocated;
	uint32_t num_free;
	struct dp_tx_desc_s *freelist;
	struct qdf_mem_multi_page_t desc_pages;
	qdf_spinlock_t lock;
};


struct dp_srng {
	void *hal_srng;
	void *base_vaddr_unaligned;
	qdf_dma_addr_t base_paddr_unaligned;
	uint32_t alloc_size;
	int irq;
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
	uint32_t defrag_timeout_ms;
	uint16_t dialogtoken;
	uint16_t statuscode;
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
	struct dp_soc *soc;    /* Reference to SoC structure ,
				to get DMA ring handles */
	qdf_lro_ctx_t lro_ctx;
};

#define REO_DESC_FREELIST_SIZE 64
#define REO_DESC_FREE_DEFER_MS 1000
struct reo_desc_list_node {
	qdf_list_node_t node;
	unsigned long free_ts;
	struct dp_rx_tid rx_tid;
};

/* TODO: Proper comments have been added in the subsequesnt gerrit */
/* packet info */
struct dp_pkt_info {
	uint32_t num; /*no of packets*/
	uint32_t bytes; /* total no of bytes */
};

/* per pdev tx stats*/
struct dp_tx_pdev_stats {

	struct dp_pkt_info rcvd; /*total packets received for transmission */
	struct {
		/* Pkt Info for which completions were received */
		struct dp_pkt_info comp_pkt;
		uint32_t mcs_count[MAX_MCS + 1]; /* MCS Count */
	} comp; /* Tx completions received*/

	struct dp_pkt_info freed; /* Tx packets freed*/

	struct dp_pkt_info processed; /* Tx packets processed*/
	struct dp_pkt_info outstanding; /* Tx packets remaining for processing*/

	struct {
		struct dp_pkt_info dropped_pkt; /* Total packets dropped */
		uint32_t desc_total;  /* total descriptors dropped */
		uint32_t dma_map_error; /* Dropped due to Dma Error */
		uint32_t ring_full;    /* dropped due to ring full */
		uint32_t fw_discard;   /* Discarded bu firmware */
		uint32_t fw_discard_retired; /* fw_discard_retired */
		/* firmware_discard_untransmitted */
		uint32_t firmware_discard_untransmitted;
		uint32_t mpdu_age_out; /* mpdu_age_out */
		uint32_t firmware_discard_reason1; /*firmware_discard_reason1*/
		uint32_t firmware_discard_reason2; /*firmware_discard_reason2*/
		uint32_t firmware_discard_reason3; /*firmware_discard_reason3*/
	} dropped; /* Packets dropped on the Tx side */

	struct {
		struct dp_pkt_info sg_pkt; /* total scatter gather packets */
		uint32_t dropped_host; /* SG packets dropped by host */
		uint32_t dropped_target; /* SG packets dropped by target */
	} sg; /* Scatter Gather packet info */

	struct {
		uint32_t num_seg;  /* No of segments in TSO packets */
		struct dp_pkt_info tso_pkt; /* total no of TSO packets */
		uint32_t dropped_host; /* TSO packets dropped by host */
		uint32_t dropped_target; /* TSO packets dropped by target */
	} tso; /* TSO packets info */

	struct {
		/* total no of multicast conversion packets */
		struct dp_pkt_info mcast_pkt;
		/* packets dropped due to map error */
		uint32_t dropped_map_error;
		/* packets dropped due to self Mac address */
		uint32_t dropped_self_mac;
		/* Packets dropped due to send fail */
		uint32_t dropped_send_fail;
		/* total unicast packets transmitted */
		uint32_t ucast;
	} mcast_en; /* Multicast Enhancement packets info */

	/* Total packets passed Reinject handler */
	struct dp_pkt_info reinject_pkts;
	/*  Total packets passed to inspect handler */
	struct dp_pkt_info inspect_pkts;
	/* Total Raw packets */
	struct dp_pkt_info raw_pkt;
};

/* Per pdev RX stats */
struct dp_rx_pdev_stats {
	struct dp_pkt_info rcvd_reo; /* packets received on the reo ring */
	struct {
		/* packets dropped because of no peer */
		struct dp_pkt_info no_peer;
		/* packets dropped because nsdu_done bit not set */
		struct dp_pkt_info msdu_not_done;
	} dropped; /* packets dropped on rx */
	struct dp_pkt_info replenished; /* total packets replnished */
	struct dp_pkt_info to_stack;    /* total packets sent up the stack */
	struct dp_pkt_info intra_bss;   /* Intra BSS packets received */
	struct dp_pkt_info wds;         /* WDS packets received */
	struct dp_pkt_info desc;
	struct dp_pkt_info buff;
	struct dp_pkt_info raw;         /* Raw Pakets received */
	struct {
		uint32_t rxdma_unitialized; /* rxdma_unitialized errors */
		uint32_t desc_alloc_fail; /* desc alloc failed errors */
	} err;                          /* Rx errors */
	uint32_t buf_freelist;         /* buffers added back in freelist */
	uint32_t mcs_count[MAX_MCS + 1]; /* packets in different MCS rates */
	uint32_t sgi_count[MAX_MCS + 1]; /* SGI count */
	/*  Number of MSDUs with no MPDU level aggregation */
	uint32_t non_ampdu_cnt;
	/* Number of MSDUs part of AMSPU */
	uint32_t ampdu_cnt;
	/* Number of MSDUs with no MSDU level aggregation */
	uint32_t non_amsdu_cnt;
	/* Number of MSDUs part of AMSDU*/
	uint32_t amsdu_cnt;
	/* Packet count in spatiel Streams */
	uint32_t nss[SS_COUNT];
	/* Packet count in different Bandwidths */
	uint32_t bw[SUPPORTED_BW];
	/* reception type os packets */
	uint32_t reception_type[SUPPORTED_RECEPTION_TYPES];
};

struct dp_ast_entry {
	uint16_t ast_idx;
	uint8_t mac_addr[DP_MAC_ADDR_LEN];
	uint8_t next_hop;
	struct dp_peer *peer;
	TAILQ_ENTRY(dp_ast_entry) ast_entry_elem;
};

/* SOC level structure for data path */
struct dp_soc {
	/* Common base structure - Should be the first member */
	struct cdp_soc_t cdp_soc;

	/* SoC/softc handle from OSIF layer */
	void *osif_soc;

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

	/* Tx SW descriptor pool */
	struct dp_tx_desc_pool_s tx_desc[MAX_TXDESC_POOLS];

	/* Tx MSDU Extension descriptor pool */
	struct dp_tx_ext_desc_pool_s tx_ext_desc[MAX_TXDESC_POOLS];

	/* Tx TSO descriptor pool */
	struct dp_tx_tso_seg_pool_s tx_tso_desc[MAX_TXDESC_POOLS];

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
	struct dp_srng *tx_ring_map[DP_MAX_TX_RINGS];

	/* Rx ring map for interrupt processing */
	struct dp_srng *rx_ring_map[DP_MAX_RX_RINGS];

#ifndef CONFIG_WIN
	/* WDI event handlers */
	struct wdi_event_subscribe_t **wdi_event_list;
#endif

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
	struct {
		/* SOC level TX stats */
		struct {
			/* packets dropped on tx because of no peer */
			struct cdp_pkt_info tx_invalid_peer;
			/* descriptors in each tcl ring */
			uint32_t tcl_ring_full[MAX_TCL_RING];
			/* Descriptors in use at soc */
			uint32_t desc_in_use;
		} tx;
		/* SOC level RX stats */
		struct {
		/* Rx errors */
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
				uint32_t rxdma_error[MAX_RXDMA_ERRORS];
				/* REO Error count */
				uint32_t reo_error[REO_ERROR_TYPE_MAX];
			} err;
		} rx;
	} stats;

	/* Enable processing of Tx completion status words */
	bool process_tx_status;

	struct dp_ast_entry *ast_table[WLAN_UMAC_PSOC_MAX_PEERS];

#ifdef DP_INTR_POLL_BASED
	/*interrupt timer*/
	qdf_timer_t int_timer;
#endif
	qdf_list_t reo_desc_freelist;
	qdf_spinlock_t reo_desc_freelist_lock;

	/* Obj Mgr SoC */
	struct wlan_objmgr_psoc *psoc;
};

#define MAX_RX_MAC_RINGS 2
/* Same as NAC_MAX_CLENT */
#define DP_NAC_MAX_CLIENT  24

/* same as ieee80211_nac_param */
enum dp_nac_param_cmd {
	/* IEEE80211_NAC_PARAM_ADD */
	DP_NAC_PARAM_ADD = 1,
	/* IEEE80211_NAC_PARAM_DEL */
	DP_NAC_PARAM_DEL,
	/* IEEE80211_NAC_PARAM_LIST */
	DP_NAC_PARAM_LIST,
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

#ifdef notyet
	/* Pktlog pdev */
	ol_pktlog_dev_t *pl_dev;
#endif

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

	/* Band steering  */
	/* TBD */

	/* PDEV level data path statistics */
	struct cdp_pdev_stats stats;

	/* Global RX decap mode for the device */
	enum htt_pkt_type rx_decap_mode;

	/* Enhanced Stats is enabled */
	bool ap_stats_tx_cal_enable;

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

	qdf_nbuf_queue_t rx_status_q;
	uint32_t mon_ppdu_id;
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

	/* TBD */

	/* map this pdev to a particular Reo Destination ring */
	enum cdp_host_reo_dest_ring reo_dest;
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

	/* safe mode control to bypass the encrypt and decipher process*/
	uint32_t safemode;

	/* rx filter related */
	uint32_t drop_unenc;
#ifdef notyet
	privacy_exemption privacy_filters[MAX_PRIVACY_FILTERS];
	uint32_t filters_num;
#endif

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

	/* Address search flags to be configured in HAL descriptor */
	uint8_t hal_desc_addr_search_flags;
};


enum {
	dp_sec_mcast = 0,
	dp_sec_ucast
};

/* Peer structure for data path state */
struct dp_peer {
	/* VDEV to which this peer is associated */
	struct dp_vdev *vdev;

	struct dp_ast_entry self_ast_entry;

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
#ifdef notyet /* TODO: See if this is required for defrag support */
		u_int32_t michael_key[2]; /* relevant for TKIP */
#endif
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

	/* NAWDS Flag and Bss Peer bit */
	uint8_t nawds_enabled:1,
				bss_peer:1,
				wapi:1;

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
 * data: Destination Mac address
 * next: pointer to next buffer
 */
struct dp_tx_me_buf_t {
	uint8_t data[DP_MAC_ADDR_LEN];
	struct dp_tx_me_buf_t *next;
};
#endif /* _DP_TYPES_H_ */
