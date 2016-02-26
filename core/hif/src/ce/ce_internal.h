/*
 * Copyright (c) 2013-2015 The Linux Foundation. All rights reserved.
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
#ifndef __COPY_ENGINE_INTERNAL_H__
#define __COPY_ENGINE_INTERNAL_H__

#include <hif.h>                /* A_TARGET_WRITE */

/* Copy Engine operational state */
enum CE_op_state {
	CE_UNUSED,
	CE_PAUSED,
	CE_RUNNING,
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
	cdf_dma_addr_t base_addr_CE_space_unaligned; /* CE address space */

	/*
	 * Actual start of descriptors.
	 * Aligned to descriptor-size boundary.
	 * Points into reserved DMA-coherent area, above.
	 */
	void *base_addr_owner_space;    /* Host address space */
	cdf_dma_addr_t base_addr_CE_space;   /* CE address space */
	/*
	 * Start of shadow copy of descriptors, within regular memory.
	 * Aligned to descriptor-size boundary.
	 */
	char *shadow_base_unaligned;
	struct CE_src_desc *shadow_base;

	unsigned int low_water_mark_nentries;
	unsigned int high_water_mark_nentries;
	void **per_transfer_context;
	OS_DMA_MEM_CONTEXT(ce_dmacontext) /* OS Specific DMA context */
};

/* Copy Engine internal state */
struct CE_state {
	struct ol_softc *scn;
	unsigned int id;
	unsigned int attr_flags;  /* CE_ATTR_* */
	uint32_t ctrl_addr;       /* relative to BAR */
	enum CE_op_state state;

#ifdef WLAN_FEATURE_FASTPATH
	u_int32_t download_len; /* pkt download length for source ring */
#endif /* WLAN_FEATURE_FASTPATH */

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
	atomic_t rx_pending;

	cdf_spinlock_t ce_index_lock;
	bool force_break;	/* Flag to indicate whether to
				 * break out the DPC context */

	unsigned int receive_count;	/* count Num Of Receive Buffers
					 * handled for one interrupt
					 * DPC routine */
	/* epping */
	bool timer_inited;
	cdf_softirq_timer_t poll_timer;
	void (*lro_flush_cb)(void *);
	void *lro_data;
};

/* Descriptor rings must be aligned to this boundary */
#define CE_DESC_RING_ALIGN 8

#ifdef QCA_WIFI_3_0
#define HIF_CE_DESC_ADDR_TO_DMA(desc) \
	(cdf_dma_addr_t)(((uint64_t)(desc)->buffer_addr + \
	((uint64_t)((desc)->buffer_addr_hi & 0x1F) << 32)))
#else
#define HIF_CE_DESC_ADDR_TO_DMA(desc) \
	(cdf_dma_addr_t)((desc)->buffer_addr)
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
	uint32_t  meta_data:14,
		byte_swap:1,
		gather:1,
		nbytes:16;
#else

		uint32_t nbytes:16,
		gather:1,
		byte_swap:1,
		meta_data:14;
#endif
};

struct CE_dest_desc {
	uint32_t buffer_addr;
#if _BYTE_ORDER == _BIG_ENDIAN
	uint32_t  meta_data:14,
		byte_swap:1,
		gather:1,
		nbytes:16;
#else
	uint32_t nbytes:16,
		gather:1,
		byte_swap:1,
		meta_data:14;
#endif
};
#endif /* QCA_WIFI_3_0 */

#define CE_SENDLIST_ITEMS_MAX 12

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
		 * OR-ed with internal flags */
		uint32_t flags;
		uint32_t user_flags;
	} item[CE_SENDLIST_ITEMS_MAX];
};

#ifdef WLAN_FEATURE_FASTPATH
void ce_h2t_tx_ce_cleanup(struct CE_handle *ce_hdl);
#endif

/* which ring of a CE? */
#define CE_RING_SRC  0
#define CE_RING_DEST 1

#define CDC_WAR_MAGIC_STR   0xceef0000
#define CDC_WAR_DATA_CE     4

/* Additional internal-only ce_send flags */
#define CE_SEND_FLAG_GATHER             0x00010000      /* Use Gather */
#endif /* __COPY_ENGINE_INTERNAL_H__ */
