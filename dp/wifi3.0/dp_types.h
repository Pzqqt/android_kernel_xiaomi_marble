/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
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
#include <queue.h>

#include <cdp_txrx_cmn.h>
#include <wdi_event_api.h>    /* WDI subscriber event list */

#include <hal_tx.h>
#define MAX_PDEV_CNT 3
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

#define MAX_TCL_DATA_RINGS 3
#define MAX_TXDESC_POOLS 3
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
 * @msdu_ext_desc: MSDU extension descriptor
 * @desc_pages: multiple page allocation information for actual descriptors
 * @link_elem_size: size of the link descriptor in cacheable memory used for
 * 		    chaining the extension descriptors
 * @desc_link_pages: multiple page allocation information for link descriptors
 */
struct dp_tx_ext_desc_pool_s {
	uint16_t elem_count;
	int elem_size;
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
	uint8_t  pool_id;
	uint8_t flags;
	struct hal_tx_desc_comp_s comp;
	uint16_t tx_encap_type;
	uint8_t frm_type;
	uint8_t pkt_offset;
	void *me_buffer;
};

/**
 * struct dp_tx_desc_pool_s - Tx Descriptor pool information
 * @desc_reserved_size: Size to be reserved for housekeeping info
 * 			in allocated memory for each descriptor
 * @page_divider: Number of bits to shift to get page number from descriptor ID
 * @offset_filter: Bit mask to get offset from descriptor ID
 * @num_allocated: Number of allocated (in use) descriptors in the pool
 * @elem_size: Size of each descriptor in the pool
 * @elem_count: Total number of descriptors in the pool
 * @freelist: Chain of free descriptors
 * @desc_pages: multiple page allocation information
 * @lock- Lock for descriptor allocation/free from/to the pool
 */
struct dp_tx_desc_pool_s {
	uint16_t desc_reserved_size;
	uint8_t page_divider;
	uint32_t offset_filter;
	uint32_t num_allocated;
	uint16_t elem_size;
	uint16_t elem_count;
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

struct ol_if_ops {
	void (*peer_set_default_routing)(void *scn_handle,
		uint8_t *peer_macaddr, uint8_t vdev_id, bool hash_based,
		uint8_t ring_num);
	int (*peer_rx_reorder_queue_setup)(void *ol_soc_handle,
		uint8_t vdev_id, uint8_t *peer_mac, qdf_dma_addr_t hw_qdesc,
		int tid, uint16_t queue_num);
	int (*peer_rx_reorder_queue_remove)(void *ol_soc_handle,
		uint8_t vdev_id, uint8_t *peer_macaddr, uint32_t tid_mask);
	/* TODO: Add any other control path calls required to OL_IF/WMA layer */
};


/* per interrupt context  */
struct dp_intr {
	uint8_t tx_ring_mask;   /* WBM Tx completion rings (0-2)
				associated with this napi context */
	uint8_t rx_ring_mask;   /* Rx REO rings (0-3) associated
				with this interrupt context */
	uint8_t rx_mon_ring_mask;  /* Rx monitor ring mask (0-2) */
	struct dp_soc *soc;    /* Reference to SoC structure ,
				to get DMA ring handles */
};

/* SOC level structure for data path */
struct dp_soc {
	/* Common base structure - Should be the first member */
#ifdef notyet /* TODO: dp_soc_cmn should be defined in cmn headers */
	struct dp_soc_cmn soc_cmn;
#endif
	/* Callbacks to OL_IF layer */
	struct ol_if_ops *ol_ops;

	/* SoC/softc handle from OSIF layer */
	void *osif_soc;

	/* OS device abstraction */
	qdf_device_t osdev;

	/* WLAN config context */
	void *wlan_cfg_ctx;

	/* HTT handle for host-fw interaction */
	void *htt_handle;

	/* Commint init done */
	bool cmn_init_done;

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
	/* Tx SW descriptor pool */
	struct dp_tx_desc_pool_s tx_desc[MAX_TXDESC_POOLS];

	/* Tx MSDU Extension descriptor pool */
	struct dp_tx_ext_desc_pool_s tx_ext_desc[MAX_TXDESC_POOLS];

	/* Tx H/W queues lock */
	qdf_spinlock_t tx_queue_lock[MAX_TX_HW_QUEUES];

	/* Rx SW descriptor pool */
	struct {
		uint32_t pool_size;
		union dp_rx_desc_list_elem_t *array;
		union dp_rx_desc_list_elem_t *freelist;
	} rx_desc[MAX_RXDESC_POOLS];

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

	/* WDI event handlers */
	struct wdi_event_subscribe_t **wdi_event_list;

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
		char *vaddr;
		struct dp_tx_me_buf_t *freelist;
		int buf_in_use;
		int nonpool_buf_in_use;
		qdf_dma_mem_context(memctx);
	} me_buf;

	/**
	 * peer ref mutex:
	 * 1. Protect peer object lookups until the returned peer object's
	 *	reference count is incremented.
	 * 2. Provide mutex when accessing peer object lookup structures.
	 */
	DP_MUTEX_TYPE peer_ref_mutex;

	/* Number of VAPs with mcast enhancement enabled */
	atomic_t mc_num_vap_attached;

	/* maximum value for peer_id */
	int max_peers;

	/* SoC level data path statistics */
	struct {
		/* TBD */
	} stats;

	/* Enable processing of Tx completion status words */
	bool process_tx_status;
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
	struct dp_srng rx_mac_buf_ring;

	/* wlan_cfg pdev ctxt*/
	 struct wlan_cfg_dp_pdev_ctxt *wlan_cfg_ctx;

	/* RXDMA monitor buffer replenish ring */
	struct dp_srng rxdma_mon_buf_ring;

	/* RXDMA monitor destination ring */
	struct dp_srng rxdma_mon_dst_ring;

	/* RXDMA monitor status ring. TBD: Check format of this ring */
	struct dp_srng rxdma_mon_status_ring;

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

	/* Band steering  */
	/* TBD */

	/* PDEV level data path statistics */
	struct {
		/* TBD */
	} stats;

	/* Global RX decap mode for the device */
	enum htt_pkt_type rx_decap_mode;

	/* Enhanced Stats is enabled */
	bool ap_stats_tx_cal_enable;

	uint32_t num_tx_outstanding;

	uint32_t num_tx_exception;
	/* TBD */
};

struct dp_peer;

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

#define MAX_HTT_METADATA_LEN 32

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

#ifdef notyet
	/* callback to check if the msdu is an WAI (WAPI) frame */
	ol_rx_check_wai_fp osif_check_wai;
#endif

	/* proxy arp function */
	ol_txrx_proxy_arp_fp osif_proxy_arp;

	/* callback to hand rx monitor 802.11 MPDU to the OS shim */
	ol_txrx_rx_mon_fp osif_rx_mon;

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
#if notyet
	privacy_exemption privacy_filters[MAX_PRIVACY_FILTERS];
	uint32_t filters_num;
#endif

	/* VDEV operating mode */
	enum wlan_op_mode opmode;

	/* Tx encapsulation type for this VAP */
	enum htt_pkt_type tx_encap_type;
	/* Rx Decapsulation type for this VAP */
	enum htt_pkt_type rx_decap_type;

	/* BSS peer */
	struct dp_peer *vap_bss_peer;

	/* NAWDS enabled */
	bool nawds_enabled;

	/* Default HTT meta data for this VDEV */
	/* TBD: check alignment constraints */
	uint16_t htt_tcl_metadata;

	/* Mesh mode vdev */
	uint32_t mesh_vdev;

	/* DSCP-TID mapping table ID */
	uint8_t dscp_tid_map_id;

	/* Multicast enhancement enabled */
	uint8_t mcast_enhancement_en;

	uint32_t num_tx_outstanding;

	uint8_t tx_ring_id;
	struct dp_tx_desc_pool_s *tx_desc;
	struct dp_tx_ext_desc_pool_s *tx_ext_desc;

	/* TBD */
};


enum {
	dp_sec_mcast = 0,
	dp_sec_ucast
};

#define MAX_NUM_PEER_ID_PER_PEER 8
#define DP_MAX_TIDS 17
#define DP_NON_QOS_TID 16
/* Peer structure for data path state */
struct dp_peer {
	/* VDEV to which this peer is associated */
	struct dp_vdev *vdev;

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
#if notyet /* TODO: See if this is required for defrag support */
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

	/* TBD */
};

#endif /* _DP_TYPES_H_ */
