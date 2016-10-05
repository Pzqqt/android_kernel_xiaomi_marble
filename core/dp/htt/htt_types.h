/*
 * Copyright (c) 2011, 2014-2016 The Linux Foundation. All rights reserved.
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

#ifndef _HTT_TYPES__H_
#define _HTT_TYPES__H_

#include <osdep.h>              /* uint16_t, dma_addr_t */
#include <qdf_types.h>          /* qdf_device_t */
#include <qdf_lock.h>           /* qdf_spinlock_t */
#include <qdf_timer.h>		/* qdf_timer_t */
#include <qdf_atomic.h>         /* qdf_atomic_inc */
#include <qdf_nbuf.h>           /* qdf_nbuf_t */
#include <htc_api.h>            /* HTC_PACKET */

#define DEBUG_DMA_DONE

#define HTT_TX_MUTEX_TYPE qdf_spinlock_t

#ifdef QCA_TX_HTT2_SUPPORT
#ifndef HTC_TX_HTT2_MAX_SIZE
/* Should sync to the target's implementation. */
#define HTC_TX_HTT2_MAX_SIZE    (120)
#endif
#endif /* QCA_TX_HTT2_SUPPORT */


struct htt_htc_pkt {
	void *pdev_ctxt;
	dma_addr_t nbuf_paddr;
	HTC_PACKET htc_pkt;
	uint16_t msdu_id;
};

struct htt_htc_pkt_union {
	union {
		struct htt_htc_pkt pkt;
		struct htt_htc_pkt_union *next;
	} u;
};

/*
 * HTT host descriptor:
 * Include the htt_tx_msdu_desc that gets downloaded to the target,
 * but also include the HTC_FRAME_HDR and alignment padding that
 * precede the htt_tx_msdu_desc.
 * htc_send_data_pkt expects this header space at the front of the
 * initial fragment (i.e. tx descriptor) that is downloaded.
 */
struct htt_host_tx_desc_t {
	uint8_t htc_header[HTC_HEADER_LEN];
	/* force the tx_desc field to begin on a 4-byte boundary */
	union {
		uint32_t dummy_force_align;
		struct htt_tx_msdu_desc_t tx_desc;
	} align32;
};

struct htt_tx_mgmt_desc_buf {
	qdf_nbuf_t msg_buf;
	A_BOOL is_inuse;
	qdf_nbuf_t mgmt_frm;
};

struct htt_tx_mgmt_desc_ctxt {
	struct htt_tx_mgmt_desc_buf *pool;
	A_UINT32 pending_cnt;
};

struct htt_list_node {
	struct htt_list_node *prev;
	struct htt_list_node *next;
};

struct htt_rx_hash_entry {
	A_UINT32 paddr;
	qdf_nbuf_t netbuf;
	A_UINT8 fromlist;
	struct htt_list_node listnode;
#ifdef RX_HASH_DEBUG
	A_UINT32 cookie;
#endif
};

struct htt_rx_hash_bucket {
	struct htt_list_node listhead;
	struct htt_rx_hash_entry *entries;
	struct htt_list_node freepool;
#ifdef RX_HASH_DEBUG
	A_UINT32 count;
#endif
};

/* IPA micro controller
   wlan host driver
   firmware shared memory structure */
struct uc_shared_mem_t {
	uint32_t *vaddr;
	qdf_dma_addr_t paddr;
	qdf_dma_mem_context(memctx);
};

/* Micro controller datapath offload
 * WLAN TX resources */
struct htt_ipa_uc_tx_resource_t {
	struct uc_shared_mem_t tx_ce_idx;
	struct uc_shared_mem_t tx_comp_base;

	uint32_t tx_comp_idx_paddr;
	void **tx_buf_pool_vaddr_strg;
	qdf_dma_addr_t *paddr_strg;
	uint32_t alloc_tx_buf_cnt;
};

/**
 * struct htt_ipa_uc_rx_resource_t
 * @rx_rdy_idx_paddr: rx ready index physical address
 * @rx_ind_ring_base: rx indication ring base memory info
 * @rx_ipa_prc_done_idx: rx process done index memory info
 * @rx_ind_ring_size: rx process done ring size
 * @rx2_rdy_idx_paddr: rx process done index physical address
 * @rx2_ind_ring_base: rx process done indication ring base memory info
 * @rx2_ipa_prc_done_idx: rx process done index memory info
 * @rx2_ind_ring_size: rx process done ring size
 */
struct htt_ipa_uc_rx_resource_t {
	qdf_dma_addr_t rx_rdy_idx_paddr;
	struct uc_shared_mem_t rx_ind_ring_base;
	struct uc_shared_mem_t rx_ipa_prc_done_idx;
	uint32_t rx_ind_ring_size;

	/* 2nd RX ring */
	qdf_dma_addr_t rx2_rdy_idx_paddr;
	struct uc_shared_mem_t rx2_ind_ring_base;
	struct uc_shared_mem_t rx2_ipa_prc_done_idx;
	uint32_t rx2_ind_ring_size;
};

/**
 * struct ipa_uc_rx_ring_elem_t
 * @rx_packet_paddr: rx packet physical address
 * @vdev_id: virtual interface id
 * @rx_packet_leng: packet length
 */
struct ipa_uc_rx_ring_elem_t {
	qdf_dma_addr_t rx_packet_paddr;
	uint32_t vdev_id;
	uint32_t rx_packet_leng;
};

struct htt_tx_credit_t {
	qdf_atomic_t bus_delta;
	qdf_atomic_t target_delta;
};

#if defined(HELIUMPLUS_PADDR64)
/**
 * msdu_ext_frag_desc:
 * semantically, this is an array of 6 of 2-tuples of
 * a 48-bit physical address and a 16 bit len field
 * with the following layout:
 * 31               16       8       0
 * |        p t r - l o w 3 2         |
 * | len             | ptr-7/16       |
 */
struct msdu_ext_frag_desc {
	union {
		uint64_t desc64;
		struct {
			uint32_t ptr_low;
			uint32_t ptr_hi:16,
				len:16;
		} frag32;
	} u;
};

struct msdu_ext_desc_t {
	struct qdf_tso_flags_t tso_flags;
	struct msdu_ext_frag_desc frags[6];
/*
	u_int32_t frag_ptr0;
	u_int32_t frag_len0;
	u_int32_t frag_ptr1;
	u_int32_t frag_len1;
	u_int32_t frag_ptr2;
	u_int32_t frag_len2;
	u_int32_t frag_ptr3;
	u_int32_t frag_len3;
	u_int32_t frag_ptr4;
	u_int32_t frag_len4;
	u_int32_t frag_ptr5;
	u_int32_t frag_len5;
*/
};
#endif  /* defined(HELIUMPLUS_PADDR64) */

/**
 * struct mon_channel
 * @ch_num: Monitor mode capture channel number
 * @ch_freq: channel frequency.
 */
struct mon_channel {
	uint32_t ch_num;
	uint32_t ch_freq;
};

struct htt_pdev_t {
	ol_pdev_handle ctrl_pdev;
	ol_txrx_pdev_handle txrx_pdev;
	HTC_HANDLE htc_pdev;
	qdf_device_t osdev;

	HTC_ENDPOINT_ID htc_tx_endpoint;

#ifdef QCA_TX_HTT2_SUPPORT
	HTC_ENDPOINT_ID htc_tx_htt2_endpoint;
	uint16_t htc_tx_htt2_max_size;
#endif /* QCA_TX_HTT2_SUPPORT */

#ifdef ATH_11AC_TXCOMPACT
	HTT_TX_MUTEX_TYPE txnbufq_mutex;
	qdf_nbuf_queue_t txnbufq;
	struct htt_htc_pkt_union *htt_htc_pkt_misclist;
#endif

	struct htt_htc_pkt_union *htt_htc_pkt_freelist;
	struct {
		int is_high_latency;
		int is_full_reorder_offload;
		int default_tx_comp_req;
		int ce_classify_enabled;
		uint8_t is_first_wakeup_packet;
	} cfg;
	struct {
		uint8_t major;
		uint8_t minor;
	} tgt_ver;
#if defined(HELIUMPLUS_PADDR64)
	struct {
		u_int8_t major;
		u_int8_t minor;
	} wifi_ip_ver;
#endif /* defined(HELIUMPLUS_PADDR64) */
	struct {
		struct {
			/*
			 * Ring of network buffer objects -
			 * This ring is used exclusively by the host SW.
			 * This ring mirrors the dev_addrs_ring that is shared
			 * between the host SW and the MAC HW.
			 * The host SW uses this netbufs ring to locate the nw
			 * buffer objects whose data buffers the HW has filled.
			 */
			qdf_nbuf_t *netbufs_ring;
			/*
			 * Ring of buffer addresses -
			 * This ring holds the "physical" device address of the
			 * rx buffers the host SW provides for MAC HW to fill.
			 */
#if HTT_PADDR64
			uint64_t *paddrs_ring;
#else   /* ! HTT_PADDR64 */
			uint32_t *paddrs_ring;
#endif
			qdf_dma_mem_context(memctx);
		} buf;
		/*
		 * Base address of ring, as a "physical" device address rather
		 * than a CPU address.
		 */
		qdf_dma_addr_t base_paddr;
		int32_t  size;       /* how many elems in the ring (power of 2) */
		uint32_t size_mask;  /* size - 1, at least 16 bits long */

		int fill_level; /* how many rx buffers to keep in the ring */
		int fill_cnt;   /* # of rx buffers (full+empty) in the ring */

		/*
		 * target_idx -
		 * Without reorder offload:
		 * not used
		 * With reorder offload:
		 * points to the location in the rx ring from which rx buffers
		 * are available to copy into the MAC DMA ring
		 */
		struct {
			uint32_t *vaddr;
			qdf_dma_addr_t paddr;
			qdf_dma_mem_context(memctx);
		} target_idx;

		/*
		 * alloc_idx/host_idx -
		 * Without reorder offload:
		 * where HTT SW has deposited empty buffers
		 * This is allocated in consistent mem, so that the FW can read
		 * this variable, and program the HW's FW_IDX reg with the value
		 * of this shadow register
		 * With reorder offload:
		 * points to the end of the available free rx buffers
		 */
		struct {
			uint32_t *vaddr;
			qdf_dma_addr_t paddr;
			qdf_dma_mem_context(memctx);
		} alloc_idx;

		/* sw_rd_idx -
		 * where HTT SW has processed bufs filled by rx MAC DMA */
		struct {
			unsigned msdu_desc;
			unsigned msdu_payld;
		} sw_rd_idx;

		/*
		 * refill_retry_timer - timer triggered when the ring is not
		 * refilled to the level expected
		 */
		qdf_timer_t refill_retry_timer;

		/*
		 * refill_ref_cnt - ref cnt for Rx buffer replenishment - this
		 * variable is used to guarantee that only one thread tries
		 * to replenish Rx ring.
		 */
		qdf_atomic_t refill_ref_cnt;
#ifdef DEBUG_DMA_DONE
		uint32_t dbg_initial_msdu_payld;
		uint32_t dbg_mpdu_range;
		uint32_t dbg_mpdu_count;
		uint32_t dbg_ring_idx;
		uint32_t dbg_refill_cnt;
		uint32_t dbg_sync_success;
#endif
#ifdef HTT_RX_RESTORE
		int rx_reset;
		uint8_t htt_rx_restore;
#endif
		qdf_spinlock_t rx_hash_lock;
		struct htt_rx_hash_bucket **hash_table;
		uint32_t listnode_offset;
	} rx_ring;
#ifdef CONFIG_HL_SUPPORT
	int rx_desc_size_hl;
#endif
	long rx_fw_desc_offset;
	int rx_mpdu_range_offset_words;
	int rx_ind_msdu_byte_idx;

	struct {
		int size;       /* of each HTT tx desc */
		uint16_t pool_elems;
		uint16_t alloc_cnt;
		struct qdf_mem_multi_page_t desc_pages;
		uint32_t *freelist;
		qdf_dma_mem_context(memctx);
	} tx_descs;
#if defined(HELIUMPLUS_PADDR64)
	struct {
		int size; /* of each Fragment/MSDU-Ext descriptor */
		int pool_elems;
		struct qdf_mem_multi_page_t desc_pages;
		qdf_dma_mem_context(memctx);
	} frag_descs;
#endif /* defined(HELIUMPLUS_PADDR64) */

	int download_len;
	void (*tx_send_complete_part2)(void *pdev, A_STATUS status,
				       qdf_nbuf_t msdu, uint16_t msdu_id);

	HTT_TX_MUTEX_TYPE htt_tx_mutex;

	struct {
		int htc_err_cnt;
	} stats;
#ifdef CONFIG_HL_SUPPORT
	int cur_seq_num_hl;
#endif
	struct htt_tx_mgmt_desc_ctxt tx_mgmt_desc_ctxt;
	struct targetdef_s *targetdef;
	struct ce_reg_def *target_ce_def;

	struct htt_ipa_uc_tx_resource_t ipa_uc_tx_rsc;
	struct htt_ipa_uc_rx_resource_t ipa_uc_rx_rsc;

	struct htt_tx_credit_t htt_tx_credit;

#ifdef DEBUG_RX_RING_BUFFER
	struct rx_buf_debug *rx_buff_list;
	int rx_buff_index;
#endif
	struct mon_channel mon_ch_info;
};

#define HTT_EPID_GET(_htt_pdev_hdl)  \
	(((struct htt_pdev_t *)(_htt_pdev_hdl))->htc_tx_endpoint)

#if defined(HELIUMPLUS_PADDR64)
#define HTT_WIFI_IP(pdev, x, y) (((pdev)->wifi_ip_ver.major == (x)) &&	\
				 ((pdev)->wifi_ip_ver.minor == (y)))

#define HTT_SET_WIFI_IP(pdev, x, y) (((pdev)->wifi_ip_ver.major = (x)) && \
				     ((pdev)->wifi_ip_ver.minor = (y)))
#endif /* defined(HELIUMPLUS_PADDR64) */

#endif /* _HTT_TYPES__H_ */
