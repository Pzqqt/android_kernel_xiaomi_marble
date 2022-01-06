/*
 * Copyright (c) 2013-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef __COPY_ENGINE_INTERNAL_H__
#define __COPY_ENGINE_INTERNAL_H__

#include <hif.h>                /* A_TARGET_WRITE */

/* Copy Engine operational state */
enum CE_op_state {
	CE_UNUSED,
	CE_PAUSED,
	CE_RUNNING,
	CE_PENDING,
};

enum ol_ath_hif_ce_ecodes {
	CE_RING_DELTA_FAIL = 0
};

struct CE_src_desc;

/* Copy Engine Ring internal state */
struct CE_ring_state {

	/* Number of entries in this ring; must be power of 2 */
	unsigned int nentries;
	unsigned int nentries_mask;

	/*
	 * For dest ring, this is the next index to be processed
	 * by software after it was/is received into.
	 *
	 * For src ring, this is the last descriptor that was sent
	 * and completion processed by software.
	 *
	 * Regardless of src or dest ring, this is an invariant
	 * (modulo ring size):
	 *     write index >= read index >= sw_index
	 */
	unsigned int sw_index;
	unsigned int write_index;       /* cached copy */
	/*
	 * For src ring, this is the next index not yet processed by HW.
	 * This is a cached copy of the real HW index (read index), used
	 * for avoiding reading the HW index register more often than
	 * necessary.
	 * This extends the invariant:
	 *     write index >= read index >= hw_index >= sw_index
	 *
	 * For dest ring, this is currently unused.
	 */
	unsigned int hw_index;  /* cached copy */

	/* Start of DMA-coherent area reserved for descriptors */
	void *base_addr_owner_space_unaligned;  /* Host address space */
	qdf_dma_addr_t base_addr_CE_space_unaligned; /* CE address space */

	/*
	 * Actual start of descriptors.
	 * Aligned to descriptor-size boundary.
	 * Points into reserved DMA-coherent area, above.
	 */
	void *base_addr_owner_space;    /* Host address space */
	qdf_dma_addr_t base_addr_CE_space;   /* CE address space */
	/*
	 * Start of shadow copy of descriptors, within regular memory.
	 * Aligned to descriptor-size boundary.
	 */
	char *shadow_base_unaligned;
	struct CE_src_desc *shadow_base;

	unsigned int low_water_mark_nentries;
	unsigned int high_water_mark_nentries;
	void *srng_ctx;
	void **per_transfer_context;

	/* HAL CE ring type */
	uint32_t hal_ring_type;
	/* ring memory prealloc */
	uint8_t is_ring_prealloc;

	OS_DMA_MEM_CONTEXT(ce_dmacontext); /* OS Specific DMA context */
};

/* Copy Engine internal state */
struct CE_state {
	struct hif_softc *scn;
	unsigned int id;
	unsigned int attr_flags;  /* CE_ATTR_* */
	uint32_t ctrl_addr;       /* relative to BAR */
	enum CE_op_state state;

#ifdef WLAN_FEATURE_FASTPATH
	fastpath_msg_handler fastpath_handler;
	void *context;
#endif /* WLAN_FEATURE_FASTPATH */
	qdf_work_t oom_allocation_work;

	ce_send_cb send_cb;
	void *send_context;

	CE_recv_cb recv_cb;
	void *recv_context;

	/* misc_cbs - are any callbacks besides send and recv enabled? */
	uint8_t misc_cbs;

	CE_watermark_cb watermark_cb;
	void *wm_context;

	/*Record the state of the copy compl interrupt */
	int disable_copy_compl_intr;

	unsigned int src_sz_max;
	struct CE_ring_state *src_ring;
	struct CE_ring_state *dest_ring;
	struct CE_ring_state *status_ring;
	atomic_t rx_pending;

	qdf_spinlock_t ce_index_lock;
	/* Flag to indicate whether to break out the DPC context */
	bool force_break;

	/* time in nanoseconds to yield control of napi poll */
	unsigned long long ce_service_yield_time;
	/* CE service start time in nanoseconds */
	unsigned long long ce_service_start_time;
	/* Num Of Receive Buffers handled for one interrupt DPC routine */
	unsigned int receive_count;
	/* epping */
	bool timer_inited;
	qdf_timer_t poll_timer;

	/* datapath - for faster access, use bools instead of a bitmap */
	bool htt_tx_data;
	bool htt_rx_data;
	qdf_lro_ctx_t lro_data;

	void (*service)(struct hif_softc *scn, int CE_id);
#ifdef WLAN_TRACEPOINTS
	/* CE tasklet sched time in nanoseconds */
	unsigned long long ce_tasklet_sched_time;
#endif
};

/* Descriptor rings must be aligned to this boundary */
#define CE_DESC_RING_ALIGN 8
#define CLOCK_OVERRIDE 0x2

#ifdef QCA_WIFI_3_0
#define HIF_CE_DESC_ADDR_TO_DMA(desc) \
	(qdf_dma_addr_t)(((uint64_t)(desc)->buffer_addr + \
	((uint64_t)((desc)->buffer_addr_hi & 0x1F) << 32)))
#else
#define HIF_CE_DESC_ADDR_TO_DMA(desc) \
	(qdf_dma_addr_t)((desc)->buffer_addr)
#endif

#ifdef QCA_WIFI_3_0
struct CE_src_desc {
	uint32_t buffer_addr:32;
#if _BYTE_ORDER == _BIG_ENDIAN
	uint32_t gather:1,
		enable_11h:1,
		meta_data_low:2, /* fw_metadata_low */
		packet_result_offset:12,
		toeplitz_hash_enable:1,
		addr_y_search_disable:1,
		addr_x_search_disable:1,
		misc_int_disable:1,
		target_int_disable:1,
		host_int_disable:1,
		dest_byte_swap:1,
		byte_swap:1,
		type:2,
		tx_classify:1,
		buffer_addr_hi:5;
		uint32_t meta_data:16, /* fw_metadata_high */
		nbytes:16;       /* length in register map */
#else
	uint32_t buffer_addr_hi:5,
		tx_classify:1,
		type:2,
		byte_swap:1,          /* src_byte_swap */
		dest_byte_swap:1,
		host_int_disable:1,
		target_int_disable:1,
		misc_int_disable:1,
		addr_x_search_disable:1,
		addr_y_search_disable:1,
		toeplitz_hash_enable:1,
		packet_result_offset:12,
		meta_data_low:2, /* fw_metadata_low */
		enable_11h:1,
		gather:1;
		uint32_t nbytes:16, /* length in register map */
		meta_data:16; /* fw_metadata_high */
#endif
	uint32_t toeplitz_hash_result:32;
};

struct CE_dest_desc {
	uint32_t buffer_addr:32;
#if _BYTE_ORDER == _BIG_ENDIAN
	uint32_t gather:1,
		enable_11h:1,
		meta_data_low:2, /* fw_metadata_low */
		packet_result_offset:12,
		toeplitz_hash_enable:1,
		addr_y_search_disable:1,
		addr_x_search_disable:1,
		misc_int_disable:1,
		target_int_disable:1,
		host_int_disable:1,
		byte_swap:1,
		src_byte_swap:1,
		type:2,
		tx_classify:1,
		buffer_addr_hi:5;
		uint32_t meta_data:16, /* fw_metadata_high */
		nbytes:16;          /* length in register map */
#else
	uint32_t buffer_addr_hi:5,
		tx_classify:1,
		type:2,
		src_byte_swap:1,
		byte_swap:1,         /* dest_byte_swap */
		host_int_disable:1,
		target_int_disable:1,
		misc_int_disable:1,
		addr_x_search_disable:1,
		addr_y_search_disable:1,
		toeplitz_hash_enable:1,
		packet_result_offset:12,
		meta_data_low:2, /* fw_metadata_low */
		enable_11h:1,
		gather:1;
		uint32_t nbytes:16, /* length in register map */
		meta_data:16;    /* fw_metadata_high */
#endif
	uint32_t toeplitz_hash_result:32;
};
#else
struct CE_src_desc {
	uint32_t buffer_addr;
#if _BYTE_ORDER == _BIG_ENDIAN
	uint32_t  meta_data:12,
		  target_int_disable:1,
		  host_int_disable:1,
		  byte_swap:1,
		  gather:1,
		  nbytes:16;
#else

	uint32_t nbytes:16,
		 gather:1,
		 byte_swap:1,
		 host_int_disable:1,
		 target_int_disable:1,
		 meta_data:12;
#endif
};

struct CE_dest_desc {
	uint32_t buffer_addr;
#if _BYTE_ORDER == _BIG_ENDIAN
	uint32_t  meta_data:12,
		  target_int_disable:1,
		  host_int_disable:1,
		  byte_swap:1,
		  gather:1,
		  nbytes:16;
#else
	uint32_t nbytes:16,
		 gather:1,
		 byte_swap:1,
		 host_int_disable:1,
		 target_int_disable:1,
		 meta_data:12;
#endif
};
#endif /* QCA_WIFI_3_0 */

struct ce_srng_src_desc {
	uint32_t buffer_addr_lo;
#if _BYTE_ORDER == _BIG_ENDIAN
	uint32_t nbytes:16,
		 rsvd:4,
		 gather:1,
		 dest_swap:1,
		 byte_swap:1,
		 toeplitz_hash_enable:1,
		 buffer_addr_hi:8;
	uint32_t rsvd1:16,
		 meta_data:16;
	uint32_t loop_count:4,
		 ring_id:8,
		 rsvd3:20;
#else
	uint32_t buffer_addr_hi:8,
		 toeplitz_hash_enable:1,
		 byte_swap:1,
		 dest_swap:1,
		 gather:1,
		 rsvd:4,
		 nbytes:16;
	uint32_t meta_data:16,
		 rsvd1:16;
	uint32_t rsvd3:20,
		 ring_id:8,
		 loop_count:4;
#endif
};
struct ce_srng_dest_desc {
	uint32_t buffer_addr_lo;
#if _BYTE_ORDER == _BIG_ENDIAN
	uint32_t loop_count:4,
		 ring_id:8,
		 rsvd1:12,
		 buffer_addr_hi:8;
#else
	uint32_t buffer_addr_hi:8,
		 rsvd1:12,
		 ring_id:8,
		 loop_count:4;
#endif
};
struct ce_srng_dest_status_desc {
#if _BYTE_ORDER == _BIG_ENDIAN
	uint32_t nbytes:16,
		 rsvd:4,
		 gather:1,
		 dest_swap:1,
		 byte_swap:1,
		 toeplitz_hash_enable:1,
		 rsvd0:8;
	uint32_t rsvd1:16,
		 meta_data:16;
#else
	uint32_t rsvd0:8,
		 toeplitz_hash_enable:1,
		 byte_swap:1,
		 dest_swap:1,
		 gather:1,
		 rsvd:4,
		 nbytes:16;
	uint32_t meta_data:16,
		 rsvd1:16;
#endif
	uint32_t toeplitz_hash;
#if _BYTE_ORDER == _BIG_ENDIAN
	uint32_t loop_count:4,
		 ring_id:8,
		 rsvd3:20;
#else
	uint32_t rsvd3:20,
		 ring_id:8,
		 loop_count:4;
#endif
};

#define CE_SENDLIST_ITEMS_MAX 12

/**
 * union ce_desc - unified data type for ce descriptors
 *
 * Both src and destination descriptors follow the same format.
 * They use different data structures for different access symantics.
 * Here we provice a unifying data type.
 */
union ce_desc {
	struct CE_src_desc src_desc;
	struct CE_dest_desc dest_desc;
};

/**
 * union ce_srng_desc - unified data type for ce srng descriptors
 * @src_desc: ce srng Source ring descriptor
 * @dest_desc: ce srng destination ring descriptor
 * @dest_status_desc: ce srng status ring descriptor
 */
union ce_srng_desc {
	struct ce_srng_src_desc src_desc;
	struct ce_srng_dest_desc dest_desc;
	struct ce_srng_dest_status_desc dest_status_desc;
};

/**
 * enum hif_ce_event_type - HIF copy engine event type
 * @HIF_RX_DESC_POST: event recorded before updating write index of RX ring.
 * @HIF_RX_DESC_COMPLETION: event recorded before updating sw index of RX ring.
 * @HIF_TX_GATHER_DESC_POST: post gather desc. (no write index update)
 * @HIF_TX_DESC_POST: event recorded before updating write index of TX ring.
 * @HIF_TX_DESC_SOFTWARE_POST: event recorded when dropping a write to the write
 *	index in a normal tx
 * @HIF_TX_DESC_COMPLETION: event recorded before updating sw index of TX ring.
 * @FAST_RX_WRITE_INDEX_UPDATE: event recorded before updating the write index
 *	of the RX ring in fastpath
 * @FAST_RX_SOFTWARE_INDEX_UPDATE: event recorded before updating the software
 *	index of the RX ring in fastpath
 * @FAST_TX_WRITE_INDEX_UPDATE: event recorded before updating the write index
 *	of the TX ring in fastpath
 * @FAST_TX_WRITE_INDEX_SOFTWARE_UPDATE: recored when dropping a write to
 *	the write index in fastpath
 * @FAST_TX_SOFTWARE_INDEX_UPDATE: event recorded before updating the software
 *	index of the RX ring in fastpath
 * @HIF_IRQ_EVENT: event recorded in the irq before scheduling the bh
 * @HIF_CE_TASKLET_ENTRY: records the start of the ce_tasklet
 * @HIF_CE_TASKLET_RESCHEDULE: records the rescheduling of the wlan_tasklet
 * @HIF_CE_TASKLET_EXIT: records the exit of the wlan tasklet without reschedule
 * @HIF_CE_REAP_ENTRY: records when we process completion outside of a bh
 * @HIF_CE_REAP_EXIT:  records when we process completion outside of a bh
 * @NAPI_SCHEDULE: records when napi is scheduled from the irq context
 * @NAPI_POLL_ENTER: records the start of the napi poll function
 * @NAPI_COMPLETE: records when interrupts are reenabled
 * @NAPI_POLL_EXIT: records when the napi poll function returns
 * @HIF_RX_NBUF_ALLOC_FAILURE: record the packet when nbuf fails to allocate
 * @HIF_RX_NBUF_MAP_FAILURE: record the packet when dma map fails
 * @HIF_RX_NBUF_ENQUEUE_FAILURE: record the packet when enqueue to ce fails
 * @HIF_CE_SRC_RING_BUFFER_POST: record the packet when buffer is posted to ce src ring
 * @HIF_CE_DEST_RING_BUFFER_POST: record the packet when buffer is posted to ce dst ring
 * @HIF_CE_DEST_RING_BUFFER_REAP: record the packet when buffer is reaped from ce dst ring
 * @HIF_CE_DEST_STATUS_RING_REAP: record the packet when status ring is reaped
 * @HIF_RX_DESC_PRE_NBUF_ALLOC: record the packet before nbuf allocation
 * @HIF_RX_DESC_PRE_NBUF_MAP: record the packet before nbuf map
 * @HIF_RX_DESC_POST_NBUF_MAP: record the packet after nbuf map
 */
enum hif_ce_event_type {
	HIF_RX_DESC_POST,
	HIF_RX_DESC_COMPLETION,
	HIF_TX_GATHER_DESC_POST,
	HIF_TX_DESC_POST,
	HIF_TX_DESC_SOFTWARE_POST,
	HIF_TX_DESC_COMPLETION,
	FAST_RX_WRITE_INDEX_UPDATE,
	FAST_RX_SOFTWARE_INDEX_UPDATE,
	FAST_TX_WRITE_INDEX_UPDATE,
	FAST_TX_WRITE_INDEX_SOFTWARE_UPDATE,
	FAST_TX_SOFTWARE_INDEX_UPDATE,
	RESUME_WRITE_INDEX_UPDATE,

	HIF_IRQ_EVENT = 0x10,
	HIF_CE_TASKLET_ENTRY,
	HIF_CE_TASKLET_RESCHEDULE,
	HIF_CE_TASKLET_EXIT,
	HIF_CE_REAP_ENTRY,
	HIF_CE_REAP_EXIT,
	NAPI_SCHEDULE,
	NAPI_POLL_ENTER,
	NAPI_COMPLETE,
	NAPI_POLL_EXIT,

	HIF_RX_NBUF_ALLOC_FAILURE = 0x20,
	HIF_RX_NBUF_MAP_FAILURE,
	HIF_RX_NBUF_ENQUEUE_FAILURE,

	HIF_CE_SRC_RING_BUFFER_POST,
	HIF_CE_DEST_RING_BUFFER_POST,
	HIF_CE_DEST_RING_BUFFER_REAP,
	HIF_CE_DEST_STATUS_RING_REAP,

	HIF_RX_DESC_PRE_NBUF_ALLOC,
	HIF_RX_DESC_PRE_NBUF_MAP,
	HIF_RX_DESC_POST_NBUF_MAP,

	HIF_EVENT_TYPE_MAX,
};

void ce_init_ce_desc_event_log(struct hif_softc *scn, int ce_id, int size);
void ce_deinit_ce_desc_event_log(struct hif_softc *scn, int ce_id);
void hif_record_ce_desc_event(struct hif_softc *scn, int ce_id,
			      enum hif_ce_event_type type,
			      union ce_desc *descriptor, void *memory,
			      int index, int len);

enum ce_sendlist_type_e {
	CE_SIMPLE_BUFFER_TYPE,
	/* TBDXXX: CE_RX_DESC_LIST, */
};

/*
 * There's a public "ce_sendlist" and a private "ce_sendlist_s".
 * The former is an opaque structure with sufficient space
 * to hold the latter.  The latter is the actual structure
 * definition and it is only used internally.  The opaque version
 * of the structure allows callers to allocate an instance on the
 * run-time stack without knowing any of the details of the
 * structure layout.
 */
struct ce_sendlist_s {
	unsigned int num_items;
	struct ce_sendlist_item {
		enum ce_sendlist_type_e send_type;
		dma_addr_t data;        /* e.g. buffer or desc list */
		union {
			unsigned int nbytes;    /* simple buffer */
			unsigned int ndesc;     /* Rx descriptor list */
		} u;
		/* flags: externally-specified flags;
		 * OR-ed with internal flags
		 */
		uint32_t flags;
		uint32_t user_flags;
	} item[CE_SENDLIST_ITEMS_MAX];
};

bool hif_ce_service_should_yield(struct hif_softc *scn, struct CE_state
				 *ce_state);

#ifdef WLAN_FEATURE_FASTPATH
void ce_h2t_tx_ce_cleanup(struct CE_handle *ce_hdl);
void ce_t2h_msg_ce_cleanup(struct CE_handle *ce_hdl);
#else
static inline void ce_h2t_tx_ce_cleanup(struct CE_handle *ce_hdl)
{
}

static inline void ce_t2h_msg_ce_cleanup(struct CE_handle *ce_hdl)
{
}
#endif

/* which ring of a CE? */
#define CE_RING_SRC  0
#define CE_RING_DEST 1
#define CE_RING_STATUS 2

#define CDC_WAR_MAGIC_STR   0xceef0000
#define CDC_WAR_DATA_CE     4

/* Additional internal-only ce_send flags */
#define CE_SEND_FLAG_GATHER             0x00010000      /* Use Gather */

/**
 * hif_get_wake_ce_id() - gets the copy engine id used for waking up
 * @scn: The hif context to use
 * @ce_id: a pointer where the copy engine Id should be populated
 *
 * Return: errno
 */
int hif_get_wake_ce_id(struct hif_softc *scn, uint8_t *ce_id);

/**
 * hif_get_fw_diag_ce_id() - gets the copy engine id used for FW diag
 * @scn: The hif context to use
 * @ce_id: a pointer where the copy engine Id should be populated
 *
 * Return: errno
 */
int hif_get_fw_diag_ce_id(struct hif_softc *scn, uint8_t *ce_id);

#if defined(HIF_CONFIG_SLUB_DEBUG_ON) || defined(HIF_CE_DEBUG_DATA_BUF)

#ifndef HIF_CE_HISTORY_MAX
#define HIF_CE_HISTORY_MAX 1024
#endif

#define CE_DEBUG_MAX_DATA_BUF_SIZE 64

/**
 * struct hif_ce_desc_event - structure for detailing a ce event
 * @index: location of the descriptor in the ce ring;
 * @type: what the event was
 * @time: when it happened
 * @current_hp: holds the current ring hp value
 * @current_tp: holds the current ring tp value
 * @descriptor: descriptor enqueued or dequeued
 * @memory: virtual address that was used
 * @dma_addr: physical/iova address based on smmu status
 * @dma_to_phy: physical address from iova address
 * @virt_to_phy: physical address from virtual address
 * @actual_data_len: length of the data
 * @data: data pointed by descriptor
 */
struct hif_ce_desc_event {
	int index;
	enum hif_ce_event_type type;
	uint64_t time;
	int cpu_id;
#ifdef HELIUMPLUS
	union ce_desc descriptor;
#else
	uint32_t current_hp;
	uint32_t current_tp;
	union ce_srng_desc descriptor;
#endif
	void *memory;

#ifdef HIF_RECORD_PADDR
	/* iova/pa based on smmu status */
	qdf_dma_addr_t dma_addr;
	/* store pa from iova address */
	qdf_dma_addr_t dma_to_phy;
	/* store pa */
	qdf_dma_addr_t virt_to_phy;
#endif /* HIF_RECORD_ADDR */

#ifdef HIF_CE_DEBUG_DATA_BUF
	size_t actual_data_len;
	uint8_t *data;
#endif /* HIF_CE_DEBUG_DATA_BUF */
};
#else
struct hif_ce_desc_event;
#endif /*#if defined(HIF_CONFIG_SLUB_DEBUG_ON)||defined(HIF_CE_DEBUG_DATA_BUF)*/

/**
 * get_next_record_index() - get the next record index
 * @table_index: atomic index variable to increment
 * @array_size: array size of the circular buffer
 *
 * Increment the atomic index and reserve the value.
 * Takes care of buffer wrap.
 * Guaranteed to be thread safe as long as fewer than array_size contexts
 * try to access the array.  If there are more than array_size contexts
 * trying to access the array, full locking of the recording process would
 * be needed to have sane logging.
 */
int get_next_record_index(qdf_atomic_t *table_index, int array_size);

#if defined(HIF_CONFIG_SLUB_DEBUG_ON) || defined(HIF_CE_DEBUG_DATA_BUF)
/**
 * hif_record_ce_srng_desc_event() - Record data pointed by the CE descriptor
 * @scn: structure detailing a ce event
 * @ce_id: length of the data
 * @type: event_type
 * @descriptor: ce src/dest/status ring descriptor
 * @memory: nbuf
 * @index: current sw/write index
 * @len: len of the buffer
 * @hal_ring: ce hw ring
 *
 * Return: None
 */
void hif_record_ce_srng_desc_event(struct hif_softc *scn, int ce_id,
				   enum hif_ce_event_type type,
				   union ce_srng_desc *descriptor,
				   void *memory, int index,
				   int len, void *hal_ring);

/**
 * hif_clear_ce_desc_debug_data() - Clear the contents of hif_ce_desc_event
 * upto data field before reusing it.
 *
 * @event: record every CE event
 *
 * Return: None
 */
void hif_clear_ce_desc_debug_data(struct hif_ce_desc_event *event);
#else
static inline
void hif_record_ce_srng_desc_event(struct hif_softc *scn, int ce_id,
				   enum hif_ce_event_type type,
				   union ce_srng_desc *descriptor,
				   void *memory, int index,
				   int len, void *hal_ring)
{
}

static inline
void hif_clear_ce_desc_debug_data(struct hif_ce_desc_event *event)
{
}
#endif /* HIF_CONFIG_SLUB_DEBUG_ON || HIF_CE_DEBUG_DATA_BUF */

#ifdef HIF_CE_DEBUG_DATA_BUF
/**
 * hif_ce_desc_data_record() - Record data pointed by the CE descriptor
 * @event: structure detailing a ce event
 * @len: length of the data
 * Return:
 */
void hif_ce_desc_data_record(struct hif_ce_desc_event *event, int len);

QDF_STATUS alloc_mem_ce_debug_hist_data(struct hif_softc *scn, uint32_t ce_id);
void free_mem_ce_debug_hist_data(struct hif_softc *scn, uint32_t ce_id);
#else
static inline
QDF_STATUS alloc_mem_ce_debug_hist_data(struct hif_softc *scn, uint32_t ce_id)
{
	return QDF_STATUS_SUCCESS;
}

static inline
void free_mem_ce_debug_hist_data(struct hif_softc *scn, uint32_t ce_id) { }

static inline
void hif_ce_desc_data_record(struct hif_ce_desc_event *event, int len)
{
}
#endif /*HIF_CE_DEBUG_DATA_BUF*/

#ifdef HIF_CONFIG_SLUB_DEBUG_ON
/**
 * ce_validate_nbytes() - validate nbytes for slub builds on tx descriptors
 * @nbytes: nbytes value being written into a send descriptor
 * @ce_state: context of the copy engine

 * nbytes should be non-zero and less than max configured for the copy engine
 *
 * Return: none
 */
static inline void ce_validate_nbytes(uint32_t nbytes,
				      struct CE_state *ce_state)
{
	if (nbytes <= 0 || nbytes > ce_state->src_sz_max)
		QDF_BUG(0);
}
#else
static inline void ce_validate_nbytes(uint32_t nbytes,
				      struct CE_state *ce_state)
{
}
#endif /* HIF_CONFIG_SLUB_DEBUG_ON */

#if defined(HIF_RECORD_PADDR)
/**
 * hif_ce_desc_record_rx_paddr() - record physical address for IOMMU
 * IOVA addr and MMU virtual addr for Rx
 * @scn: hif_softc
 * @nbuf: buffer posted to fw
 *
 * record physical address for ce_event_type HIF_RX_DESC_POST and
 * HIF_RX_DESC_COMPLETION
 *
 * Return: none
 */
void hif_ce_desc_record_rx_paddr(struct hif_softc *scn,
				 struct hif_ce_desc_event *event,
				 qdf_nbuf_t nbuf);
#else
static inline
void hif_ce_desc_record_rx_paddr(struct hif_softc *scn,
				 struct hif_ce_desc_event *event,
				 qdf_nbuf_t nbuf)
{
}
#endif /* HIF_RECORD_PADDR */
#endif /* __COPY_ENGINE_INTERNAL_H__ */
