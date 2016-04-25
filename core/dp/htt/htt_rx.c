/*
 * Copyright (c) 2011-2016 The Linux Foundation. All rights reserved.
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
 * @file htt_rx.c
 * @brief Implement receive aspects of HTT.
 * @details
 *  This file contains three categories of HTT rx code:
 *  1.  An abstraction of the rx descriptor, to hide the
 *      differences between the HL vs. LL rx descriptor.
 *  2.  Functions for providing access to the (series of)
 *      rx descriptor(s) and rx frame(s) associated with
 *      an rx indication message.
 *  3.  Functions for setting up and using the MAC DMA
 *      rx ring (applies to LL only).
 */

#include <qdf_mem.h>         /* qdf_mem_malloc,free, etc. */
#include <qdf_types.h>          /* qdf_print, bool */
#include <qdf_nbuf.h>           /* qdf_nbuf_t, etc. */
#include <qdf_timer.h>		/* qdf_timer_free */

#include <htt.h>                /* HTT_HL_RX_DESC_SIZE */
#include <ol_cfg.h>
#include <ol_rx.h>
#include <ol_htt_rx_api.h>
#include <htt_internal.h>       /* HTT_ASSERT, htt_pdev_t, HTT_RX_BUF_SIZE */
#include "regtable.h"

#include <cds_ieee80211_common.h>   /* ieee80211_frame, ieee80211_qoscntl */
#include <cds_ieee80211_defines.h>  /* ieee80211_rx_status */
#include <cds_utils.h>
#include <cds_concurrency.h>

#ifdef DEBUG_DMA_DONE
#include <asm/barrier.h>
#include <wma_api.h>
#endif

#ifdef HTT_DEBUG_DATA
#define HTT_PKT_DUMP(x) x
#else
#define HTT_PKT_DUMP(x) /* no-op */
#endif

/* AR9888v1 WORKAROUND for EV#112367 */
/* FIX THIS - remove this WAR when the bug is fixed */
#define PEREGRINE_1_0_ZERO_LEN_PHY_ERR_WAR

/*--- setup / tear-down functions -------------------------------------------*/

#ifndef HTT_RX_RING_SIZE_MIN
#define HTT_RX_RING_SIZE_MIN 128        /* slightly > than one large A-MPDU */
#endif

#ifndef HTT_RX_RING_SIZE_MAX
#define HTT_RX_RING_SIZE_MAX 2048       /* ~20 ms @ 1 Gbps of 1500B MSDUs */
#endif

#ifndef HTT_RX_AVG_FRM_BYTES
#define HTT_RX_AVG_FRM_BYTES 1000
#endif

#ifndef HTT_RX_HOST_LATENCY_MAX_MS
#define HTT_RX_HOST_LATENCY_MAX_MS 20 /* ms */	/* very conservative */
#endif

 /* very conservative to ensure enough buffers are allocated */
#ifndef HTT_RX_HOST_LATENCY_WORST_LIKELY_MS
#define HTT_RX_HOST_LATENCY_WORST_LIKELY_MS 20
#endif

#ifndef HTT_RX_RING_REFILL_RETRY_TIME_MS
#define HTT_RX_RING_REFILL_RETRY_TIME_MS    50
#endif

/*--- RX In Order Definitions ------------------------------------------------*/

/* Number of buckets in the hash table */
#define RX_NUM_HASH_BUCKETS 1024        /* This should always be a power of 2 */
#define RX_NUM_HASH_BUCKETS_MASK (RX_NUM_HASH_BUCKETS - 1)

/* Number of hash entries allocated per bucket */
#define RX_ENTRIES_SIZE 10

#define RX_HASH_FUNCTION(a) (((a >> 14) ^ (a >> 4)) & RX_NUM_HASH_BUCKETS_MASK)

#ifdef RX_HASH_DEBUG_LOG
#define RX_HASH_LOG(x) x
#else
#define RX_HASH_LOG(x)          /* no-op */
#endif

/* De -initialization function of the rx buffer hash table. This function will
 *   free up the hash table which includes freeing all the pending rx buffers
 */
void htt_rx_hash_deinit(struct htt_pdev_t *pdev)
{

	uint32_t i;
	struct htt_rx_hash_entry *hash_entry;
	struct htt_list_node *list_iter = NULL;

	if (NULL == pdev->rx_ring.hash_table)
		return;
	for (i = 0; i < RX_NUM_HASH_BUCKETS; i++) {
		/* Free the hash entries in hash bucket i */
		list_iter = pdev->rx_ring.hash_table[i].listhead.next;
		while (list_iter != &pdev->rx_ring.hash_table[i].listhead) {
			hash_entry =
				(struct htt_rx_hash_entry *)((char *)list_iter -
							     pdev->rx_ring.
							     listnode_offset);
			if (hash_entry->netbuf) {
#ifdef DEBUG_DMA_DONE
				qdf_nbuf_unmap(pdev->osdev, hash_entry->netbuf,
					       QDF_DMA_BIDIRECTIONAL);
#else
				qdf_nbuf_unmap(pdev->osdev, hash_entry->netbuf,
					       QDF_DMA_FROM_DEVICE);
#endif
				qdf_nbuf_free(hash_entry->netbuf);
				hash_entry->paddr = 0;
			}
			list_iter = list_iter->next;

			if (!hash_entry->fromlist)
				qdf_mem_free(hash_entry);
		}

		qdf_mem_free(pdev->rx_ring.hash_table[i].entries);

	}
	qdf_mem_free(pdev->rx_ring.hash_table);
	pdev->rx_ring.hash_table = NULL;
}

static bool
htt_rx_msdu_first_msdu_flag_ll(htt_pdev_handle pdev, void *msdu_desc)
{
	struct htt_host_rx_desc_base *rx_desc =
		(struct htt_host_rx_desc_base *)msdu_desc;
	return (bool)
		(((*(((uint32_t *) &rx_desc->msdu_end) + 4)) &
		  RX_MSDU_END_4_FIRST_MSDU_MASK) >>
		 RX_MSDU_END_4_FIRST_MSDU_LSB);
}

static int htt_rx_ring_size(struct htt_pdev_t *pdev)
{
	int size;

	/*
	 * It is expected that the host CPU will typically be able to service
	 * the rx indication from one A-MPDU before the rx indication from
	 * the subsequent A-MPDU happens, roughly 1-2 ms later.
	 * However, the rx ring should be sized very conservatively, to
	 * accomodate the worst reasonable delay before the host CPU services
	 * a rx indication interrupt.
	 * The rx ring need not be kept full of empty buffers.  In theory,
	 * the htt host SW can dynamically track the low-water mark in the
	 * rx ring, and dynamically adjust the level to which the rx ring
	 * is filled with empty buffers, to dynamically meet the desired
	 * low-water mark.
	 * In contrast, it's difficult to resize the rx ring itself, once
	 * it's in use.
	 * Thus, the ring itself should be sized very conservatively, while
	 * the degree to which the ring is filled with empty buffers should
	 * be sized moderately conservatively.
	 */
	size =
		ol_cfg_max_thruput_mbps(pdev->ctrl_pdev) *
		1000 /* 1e6 bps/mbps / 1e3 ms per sec = 1000 */  /
		(8 * HTT_RX_AVG_FRM_BYTES) * HTT_RX_HOST_LATENCY_MAX_MS;

	if (size < HTT_RX_RING_SIZE_MIN)
		size = HTT_RX_RING_SIZE_MIN;
	else if (size > HTT_RX_RING_SIZE_MAX)
		size = HTT_RX_RING_SIZE_MAX;

	size = qdf_get_pwr2(size);
	return size;
}

static int htt_rx_ring_fill_level(struct htt_pdev_t *pdev)
{
	int size;

	size = ol_cfg_max_thruput_mbps(pdev->ctrl_pdev) *
		1000 /* 1e6 bps/mbps / 1e3 ms per sec = 1000 */  /
		(8 * HTT_RX_AVG_FRM_BYTES) * HTT_RX_HOST_LATENCY_WORST_LIKELY_MS;

	size = qdf_get_pwr2(size);
	/*
	 * Make sure the fill level is at least 1 less than the ring size.
	 * Leaving 1 element empty allows the SW to easily distinguish
	 * between a full ring vs. an empty ring.
	 */
	if (size >= pdev->rx_ring.size)
		size = pdev->rx_ring.size - 1;

	return size;
}

static void htt_rx_ring_refill_retry(void *arg)
{
	htt_pdev_handle pdev = (htt_pdev_handle) arg;
	htt_rx_msdu_buff_replenish(pdev);
}

void htt_rx_ring_fill_n(struct htt_pdev_t *pdev, int num)
{
	int idx;
	QDF_STATUS status;
	struct htt_host_rx_desc_base *rx_desc;

	idx = *(pdev->rx_ring.alloc_idx.vaddr);
	while (num > 0) {
		qdf_dma_addr_t paddr;
		qdf_nbuf_t rx_netbuf;
		int headroom;

		rx_netbuf =
			qdf_nbuf_alloc(pdev->osdev, HTT_RX_BUF_SIZE,
				       0, 4, false);
		if (!rx_netbuf) {
			qdf_timer_stop(&pdev->rx_ring.
						 refill_retry_timer);
			/*
			 * Failed to fill it to the desired level -
			 * we'll start a timer and try again next time.
			 * As long as enough buffers are left in the ring for
			 * another A-MPDU rx, no special recovery is needed.
			 */
#ifdef DEBUG_DMA_DONE
			pdev->rx_ring.dbg_refill_cnt++;
#endif
			qdf_timer_start(
				&pdev->rx_ring.refill_retry_timer,
				HTT_RX_RING_REFILL_RETRY_TIME_MS);
			goto fail;
		}

		/* Clear rx_desc attention word before posting to Rx ring */
		rx_desc = htt_rx_desc(rx_netbuf);
		*(uint32_t *) &rx_desc->attention = 0;

#ifdef DEBUG_DMA_DONE
		*(uint32_t *) &rx_desc->msdu_end = 1;

#define MAGIC_PATTERN 0xDEADBEEF
		*(uint32_t *) &rx_desc->msdu_start = MAGIC_PATTERN;

		/* To ensure that attention bit is reset and msdu_end is set
		   before calling dma_map */
		smp_mb();
#endif
		/*
		 * Adjust qdf_nbuf_data to point to the location in the buffer
		 * where the rx descriptor will be filled in.
		 */
		headroom = qdf_nbuf_data(rx_netbuf) - (uint8_t *) rx_desc;
		qdf_nbuf_push_head(rx_netbuf, headroom);

#ifdef DEBUG_DMA_DONE
		status =
			qdf_nbuf_map(pdev->osdev, rx_netbuf,
						QDF_DMA_BIDIRECTIONAL);
#else
		status =
			qdf_nbuf_map(pdev->osdev, rx_netbuf,
						QDF_DMA_FROM_DEVICE);
#endif
		if (status != QDF_STATUS_SUCCESS) {
			qdf_nbuf_free(rx_netbuf);
			goto fail;
		}
		paddr = qdf_nbuf_get_frag_paddr(rx_netbuf, 0);
		if (pdev->cfg.is_full_reorder_offload) {
			if (qdf_unlikely
				    (htt_rx_hash_list_insert(pdev, paddr,
							     rx_netbuf))) {
				qdf_print("%s: hash insert failed!\n",
					  __func__);
#ifdef DEBUG_DMA_DONE
				qdf_nbuf_unmap(pdev->osdev, rx_netbuf,
					       QDF_DMA_BIDIRECTIONAL);
#else
				qdf_nbuf_unmap(pdev->osdev, rx_netbuf,
					       QDF_DMA_FROM_DEVICE);
#endif
				qdf_nbuf_free(rx_netbuf);
				goto fail;
			}
			htt_rx_dbg_rxbuf_set(pdev, paddr, rx_netbuf);
		} else {
			pdev->rx_ring.buf.netbufs_ring[idx] = rx_netbuf;
		}
#if HTT_PADDR64
		paddr &= 0x1fffffffff; /* trim out higher than 37 bits */
#endif /* HTT_PADDR64 */
		pdev->rx_ring.buf.paddrs_ring[idx] = paddr;
		pdev->rx_ring.fill_cnt++;

		num--;
		idx++;
		idx &= pdev->rx_ring.size_mask;
	}

fail:
	*(pdev->rx_ring.alloc_idx.vaddr) = idx;
	return;
}

unsigned htt_rx_ring_elems(struct htt_pdev_t *pdev)
{
	return
		(*pdev->rx_ring.alloc_idx.vaddr -
		 pdev->rx_ring.sw_rd_idx.msdu_payld) & pdev->rx_ring.size_mask;
}

unsigned int htt_rx_in_order_ring_elems(struct htt_pdev_t *pdev)
{
	return
		(*pdev->rx_ring.alloc_idx.vaddr -
		 *pdev->rx_ring.target_idx.vaddr) &
		pdev->rx_ring.size_mask;
}

void htt_rx_detach(struct htt_pdev_t *pdev)
{
	qdf_timer_stop(&pdev->rx_ring.refill_retry_timer);
	qdf_timer_free(&pdev->rx_ring.refill_retry_timer);

	if (pdev->cfg.is_full_reorder_offload) {
		qdf_mem_free_consistent(pdev->osdev, pdev->osdev->dev,
					   sizeof(uint32_t),
					   pdev->rx_ring.target_idx.vaddr,
					   pdev->rx_ring.target_idx.paddr,
					   qdf_get_dma_mem_context((&pdev->
								    rx_ring.
								    target_idx),
								   memctx));
		htt_rx_hash_deinit(pdev);
	} else {
		int sw_rd_idx = pdev->rx_ring.sw_rd_idx.msdu_payld;

		while (sw_rd_idx != *(pdev->rx_ring.alloc_idx.vaddr)) {
#ifdef DEBUG_DMA_DONE
			qdf_nbuf_unmap(pdev->osdev,
				       pdev->rx_ring.buf.
				       netbufs_ring[sw_rd_idx],
				       QDF_DMA_BIDIRECTIONAL);
#else
			qdf_nbuf_unmap(pdev->osdev,
				       pdev->rx_ring.buf.
				       netbufs_ring[sw_rd_idx],
				       QDF_DMA_FROM_DEVICE);
#endif
			qdf_nbuf_free(pdev->rx_ring.buf.
				      netbufs_ring[sw_rd_idx]);
			sw_rd_idx++;
			sw_rd_idx &= pdev->rx_ring.size_mask;
		}
		qdf_mem_free(pdev->rx_ring.buf.netbufs_ring);
	}

	qdf_mem_free_consistent(pdev->osdev, pdev->osdev->dev,
				   sizeof(uint32_t),
				   pdev->rx_ring.alloc_idx.vaddr,
				   pdev->rx_ring.alloc_idx.paddr,
				   qdf_get_dma_mem_context((&pdev->rx_ring.
							    alloc_idx),
							   memctx));

	qdf_mem_free_consistent(pdev->osdev, pdev->osdev->dev,
				   pdev->rx_ring.size * sizeof(qdf_dma_addr_t),
				   pdev->rx_ring.buf.paddrs_ring,
				   pdev->rx_ring.base_paddr,
				   qdf_get_dma_mem_context((&pdev->rx_ring.buf),
							   memctx));
}

/*--- rx descriptor field access functions ----------------------------------*/
/*
 * These functions need to use bit masks and shifts to extract fields
 * from the rx descriptors, rather than directly using the bitfields.
 * For example, use
 *     (desc & FIELD_MASK) >> FIELD_LSB
 * rather than
 *     desc.field
 * This allows the functions to work correctly on either little-endian
 * machines (no endianness conversion needed) or big-endian machines
 * (endianness conversion provided automatically by the HW DMA's
 * byte-swizzling).
 */
/* FIX THIS: APPLIES TO LL ONLY */

/**
 * htt_rx_mpdu_desc_retry_ll() - Returns the retry bit from the Rx descriptor
 *                               for the Low Latency driver
 * @pdev:                          Handle (pointer) to HTT pdev.
 * @mpdu_desc:                     Void pointer to the Rx descriptor for MPDU
 *                                 before the beginning of the payload.
 *
 *  This function returns the retry bit of the 802.11 header for the
 *  provided rx MPDU descriptor.
 *
 * Return:        boolean -- true if retry is set, false otherwise
 */
bool
htt_rx_mpdu_desc_retry_ll(htt_pdev_handle pdev, void *mpdu_desc)
{
	struct htt_host_rx_desc_base *rx_desc =
		(struct htt_host_rx_desc_base *) mpdu_desc;

	return
		(bool)(((*((uint32_t *) &rx_desc->mpdu_start)) &
		RX_MPDU_START_0_RETRY_MASK) >>
		RX_MPDU_START_0_RETRY_LSB);
}

uint16_t htt_rx_mpdu_desc_seq_num_ll(htt_pdev_handle pdev, void *mpdu_desc)
{
	struct htt_host_rx_desc_base *rx_desc =
		(struct htt_host_rx_desc_base *)mpdu_desc;

	return
		(uint16_t) (((*((uint32_t *) &rx_desc->mpdu_start)) &
			     RX_MPDU_START_0_SEQ_NUM_MASK) >>
			    RX_MPDU_START_0_SEQ_NUM_LSB);
}

/* FIX THIS: APPLIES TO LL ONLY */
void
htt_rx_mpdu_desc_pn_ll(htt_pdev_handle pdev,
		       void *mpdu_desc, union htt_rx_pn_t *pn, int pn_len_bits)
{
	struct htt_host_rx_desc_base *rx_desc =
		(struct htt_host_rx_desc_base *)mpdu_desc;

	switch (pn_len_bits) {
	case 24:
		/* bits 23:0 */
		pn->pn24 = rx_desc->mpdu_start.pn_31_0 & 0xffffff;
		break;
	case 48:
		/* bits 31:0 */
		pn->pn48 = rx_desc->mpdu_start.pn_31_0;
		/* bits 47:32 */
		pn->pn48 |= ((uint64_t)
			     ((*(((uint32_t *) &rx_desc->mpdu_start) + 2))
			      & RX_MPDU_START_2_PN_47_32_MASK))
			<< (32 - RX_MPDU_START_2_PN_47_32_LSB);
		break;
	case 128:
		/* bits 31:0 */
		pn->pn128[0] = rx_desc->mpdu_start.pn_31_0;
		/* bits 47:32 */
		pn->pn128[0] |=
			((uint64_t) ((*(((uint32_t *)&rx_desc->mpdu_start) + 2))
				     & RX_MPDU_START_2_PN_47_32_MASK))
			<< (32 - RX_MPDU_START_2_PN_47_32_LSB);
		/* bits 63:48 */
		pn->pn128[0] |=
			((uint64_t) ((*(((uint32_t *) &rx_desc->msdu_end) + 2))
				     & RX_MSDU_END_1_EXT_WAPI_PN_63_48_MASK))
			<< (48 - RX_MSDU_END_1_EXT_WAPI_PN_63_48_LSB);
		/* bits 95:64 */
		pn->pn128[1] = rx_desc->msdu_end.ext_wapi_pn_95_64;
		/* bits 127:96 */
		pn->pn128[1] |=
			((uint64_t) rx_desc->msdu_end.ext_wapi_pn_127_96) << 32;
		break;
	default:
		qdf_print("Error: invalid length spec (%d bits) for PN\n",
			  pn_len_bits);
	};
}

/**
 * htt_rx_mpdu_desc_tid_ll() - Returns the TID value from the Rx descriptor
 *                             for Low Latency driver
 * @pdev:                        Handle (pointer) to HTT pdev.
 * @mpdu_desc:                   Void pointer to the Rx descriptor for the MPDU
 *                               before the beginning of the payload.
 *
 * This function returns the TID set in the 802.11 QoS Control for the MPDU
 * in the packet header, by looking at the mpdu_start of the Rx descriptor.
 * Rx descriptor gets a copy of the TID from the MAC.
 *
 * Return:        Actual TID set in the packet header.
 */
uint8_t
htt_rx_mpdu_desc_tid_ll(htt_pdev_handle pdev, void *mpdu_desc)
{
	struct htt_host_rx_desc_base *rx_desc =
		(struct htt_host_rx_desc_base *) mpdu_desc;

	return
		(uint8_t)(((*(((uint32_t *) &rx_desc->mpdu_start) + 2)) &
		RX_MPDU_START_2_TID_MASK) >>
		RX_MPDU_START_2_TID_LSB);
}

uint32_t htt_rx_mpdu_desc_tsf32(htt_pdev_handle pdev, void *mpdu_desc)
{
/* FIX THIS */
	return 0;
}

/* FIX THIS: APPLIES TO LL ONLY */
char *htt_rx_mpdu_wifi_hdr_retrieve(htt_pdev_handle pdev, void *mpdu_desc)
{
	struct htt_host_rx_desc_base *rx_desc =
		(struct htt_host_rx_desc_base *)mpdu_desc;
	return rx_desc->rx_hdr_status;
}

/* FIX THIS: APPLIES TO LL ONLY */
bool htt_rx_msdu_desc_completes_mpdu_ll(htt_pdev_handle pdev, void *msdu_desc)
{
	struct htt_host_rx_desc_base *rx_desc =
		(struct htt_host_rx_desc_base *)msdu_desc;
	return (bool)
		(((*(((uint32_t *) &rx_desc->msdu_end) + 4)) &
		  RX_MSDU_END_4_LAST_MSDU_MASK) >> RX_MSDU_END_4_LAST_MSDU_LSB);
}

/* FIX THIS: APPLIES TO LL ONLY */
int htt_rx_msdu_has_wlan_mcast_flag_ll(htt_pdev_handle pdev, void *msdu_desc)
{
	struct htt_host_rx_desc_base *rx_desc =
		(struct htt_host_rx_desc_base *)msdu_desc;
	/* HW rx desc: the mcast_bcast flag is only valid
	   if first_msdu is set */
	return
		((*(((uint32_t *) &rx_desc->msdu_end) + 4)) &
		 RX_MSDU_END_4_FIRST_MSDU_MASK) >> RX_MSDU_END_4_FIRST_MSDU_LSB;
}

/* FIX THIS: APPLIES TO LL ONLY */
bool htt_rx_msdu_is_wlan_mcast_ll(htt_pdev_handle pdev, void *msdu_desc)
{
	struct htt_host_rx_desc_base *rx_desc =
		(struct htt_host_rx_desc_base *)msdu_desc;
	return
		((*((uint32_t *) &rx_desc->attention)) &
		 RX_ATTENTION_0_MCAST_BCAST_MASK)
		>> RX_ATTENTION_0_MCAST_BCAST_LSB;
}

/* FIX THIS: APPLIES TO LL ONLY */
int htt_rx_msdu_is_frag_ll(htt_pdev_handle pdev, void *msdu_desc)
{
	struct htt_host_rx_desc_base *rx_desc =
		(struct htt_host_rx_desc_base *)msdu_desc;
	return
		((*((uint32_t *) &rx_desc->attention)) &
		 RX_ATTENTION_0_FRAGMENT_MASK) >> RX_ATTENTION_0_FRAGMENT_LSB;
}

static inline
uint8_t htt_rx_msdu_fw_desc_get(htt_pdev_handle pdev, void *msdu_desc)
{
	/*
	 * HL and LL use the same format for FW rx desc, but have the FW rx desc
	 * in different locations.
	 * In LL, the FW rx descriptor has been copied into the same
	 * htt_host_rx_desc_base struct that holds the HW rx desc.
	 * In HL, the FW rx descriptor, along with the MSDU payload,
	 * is in the same buffer as the rx indication message.
	 *
	 * Use the FW rx desc offset configured during startup to account for
	 * this difference between HL vs. LL.
	 *
	 * An optimization would be to define the LL and HL msdu_desc pointer
	 * in such a way that they both use the same offset to the FW rx desc.
	 * Then the following functions could be converted to macros, without
	 * needing to expose the htt_pdev_t definition outside HTT.
	 */
	return *(((uint8_t *) msdu_desc) + pdev->rx_fw_desc_offset);
}

int htt_rx_msdu_discard(htt_pdev_handle pdev, void *msdu_desc)
{
	return htt_rx_msdu_fw_desc_get(pdev, msdu_desc) & FW_RX_DESC_DISCARD_M;
}

int htt_rx_msdu_forward(htt_pdev_handle pdev, void *msdu_desc)
{
	return htt_rx_msdu_fw_desc_get(pdev, msdu_desc) & FW_RX_DESC_FORWARD_M;
}

int htt_rx_msdu_inspect(htt_pdev_handle pdev, void *msdu_desc)
{
	return htt_rx_msdu_fw_desc_get(pdev, msdu_desc) & FW_RX_DESC_INSPECT_M;
}

void
htt_rx_msdu_actions(htt_pdev_handle pdev,
		    void *msdu_desc, int *discard, int *forward, int *inspect)
{
	uint8_t rx_msdu_fw_desc = htt_rx_msdu_fw_desc_get(pdev, msdu_desc);
#ifdef HTT_DEBUG_DATA
	HTT_PRINT("act:0x%x ", rx_msdu_fw_desc);
#endif
	*discard = rx_msdu_fw_desc & FW_RX_DESC_DISCARD_M;
	*forward = rx_msdu_fw_desc & FW_RX_DESC_FORWARD_M;
	*inspect = rx_msdu_fw_desc & FW_RX_DESC_INSPECT_M;
}

static inline qdf_nbuf_t htt_rx_netbuf_pop(htt_pdev_handle pdev)
{
	int idx;
	qdf_nbuf_t msdu;

	HTT_ASSERT1(htt_rx_ring_elems(pdev) != 0);

#ifdef DEBUG_DMA_DONE
	pdev->rx_ring.dbg_ring_idx++;
	pdev->rx_ring.dbg_ring_idx &= pdev->rx_ring.size_mask;
#endif

	idx = pdev->rx_ring.sw_rd_idx.msdu_payld;
	msdu = pdev->rx_ring.buf.netbufs_ring[idx];
	idx++;
	idx &= pdev->rx_ring.size_mask;
	pdev->rx_ring.sw_rd_idx.msdu_payld = idx;
	pdev->rx_ring.fill_cnt--;
	return msdu;
}

static inline qdf_nbuf_t
htt_rx_in_order_netbuf_pop(htt_pdev_handle pdev, uint32_t paddr)
{
	HTT_ASSERT1(htt_rx_in_order_ring_elems(pdev) != 0);
	pdev->rx_ring.fill_cnt--;
	return htt_rx_hash_list_lookup(pdev, paddr);
}

/* FIX ME: this function applies only to LL rx descs.
   An equivalent for HL rx descs is needed. */
#ifdef CHECKSUM_OFFLOAD
static inline
void
htt_set_checksum_result_ll(htt_pdev_handle pdev, qdf_nbuf_t msdu,
			   struct htt_host_rx_desc_base *rx_desc)
{
#define MAX_IP_VER          2
#define MAX_PROTO_VAL       4
	struct rx_msdu_start *rx_msdu = &rx_desc->msdu_start;
	unsigned int proto = (rx_msdu->tcp_proto) | (rx_msdu->udp_proto << 1);

	/*
	 * HW supports TCP & UDP checksum offload for ipv4 and ipv6
	 */
	static const qdf_nbuf_l4_rx_cksum_type_t
		cksum_table[][MAX_PROTO_VAL][MAX_IP_VER] = {
		{
			/* non-fragmented IP packet */
			/* non TCP/UDP packet */
			{QDF_NBUF_RX_CKSUM_NONE, QDF_NBUF_RX_CKSUM_NONE},
			/* TCP packet */
			{QDF_NBUF_RX_CKSUM_TCP, QDF_NBUF_RX_CKSUM_TCPIPV6},
			/* UDP packet */
			{QDF_NBUF_RX_CKSUM_UDP, QDF_NBUF_RX_CKSUM_UDPIPV6},
			/* invalid packet type */
			{QDF_NBUF_RX_CKSUM_NONE, QDF_NBUF_RX_CKSUM_NONE},
		},
		{
			/* fragmented IP packet */
			{QDF_NBUF_RX_CKSUM_NONE, QDF_NBUF_RX_CKSUM_NONE},
			{QDF_NBUF_RX_CKSUM_NONE, QDF_NBUF_RX_CKSUM_NONE},
			{QDF_NBUF_RX_CKSUM_NONE, QDF_NBUF_RX_CKSUM_NONE},
			{QDF_NBUF_RX_CKSUM_NONE, QDF_NBUF_RX_CKSUM_NONE},
		}
	};

	qdf_nbuf_rx_cksum_t cksum = {
		cksum_table[rx_msdu->ip_frag][proto][rx_msdu->ipv6_proto],
		QDF_NBUF_RX_CKSUM_NONE,
		0
	};

	if (cksum.l4_type !=
	    (qdf_nbuf_l4_rx_cksum_type_t) QDF_NBUF_RX_CKSUM_NONE) {
		cksum.l4_result =
			((*(uint32_t *) &rx_desc->attention) &
			 RX_ATTENTION_0_TCP_UDP_CHKSUM_FAIL_MASK) ?
			QDF_NBUF_RX_CKSUM_NONE :
			QDF_NBUF_RX_CKSUM_TCP_UDP_UNNECESSARY;
	}
	qdf_nbuf_set_rx_cksum(msdu, &cksum);
#undef MAX_IP_VER
#undef MAX_PROTO_VAL
}
#else
#define htt_set_checksum_result_ll(pdev, msdu, rx_desc) /* no-op */
#endif

#ifdef DEBUG_DMA_DONE
void htt_rx_print_rx_indication(qdf_nbuf_t rx_ind_msg, htt_pdev_handle pdev)
{
	uint32_t *msg_word;
	int byte_offset;
	int mpdu_range, num_mpdu_range;

	msg_word = (uint32_t *) qdf_nbuf_data(rx_ind_msg);

	qdf_print
		("------------------HTT RX IND-----------------------------\n");
	qdf_print("alloc idx paddr %x (*vaddr) %d\n",
		  pdev->rx_ring.alloc_idx.paddr,
		  *pdev->rx_ring.alloc_idx.vaddr);

	qdf_print("sw_rd_idx msdu_payld %d msdu_desc %d\n",
		  pdev->rx_ring.sw_rd_idx.msdu_payld,
		  pdev->rx_ring.sw_rd_idx.msdu_desc);

	qdf_print("dbg_ring_idx %d\n", pdev->rx_ring.dbg_ring_idx);

	qdf_print("fill_level %d fill_cnt %d\n", pdev->rx_ring.fill_level,
		  pdev->rx_ring.fill_cnt);

	qdf_print("initial msdu_payld %d curr mpdu range %d curr mpdu cnt %d\n",
		  pdev->rx_ring.dbg_initial_msdu_payld,
		  pdev->rx_ring.dbg_mpdu_range, pdev->rx_ring.dbg_mpdu_count);

	/* Print the RX_IND contents */

	qdf_print("peer id %x RV %x FV %x ext_tid %x msg_type %x\n",
		  HTT_RX_IND_PEER_ID_GET(*msg_word),
		  HTT_RX_IND_REL_VALID_GET(*msg_word),
		  HTT_RX_IND_FLUSH_VALID_GET(*msg_word),
		  HTT_RX_IND_EXT_TID_GET(*msg_word),
		  HTT_T2H_MSG_TYPE_GET(*msg_word));

	qdf_print("num_mpdu_ranges %x rel_seq_num_end %x rel_seq_num_start %x\n"
		  " flush_seq_num_end %x flush_seq_num_start %x\n",
		  HTT_RX_IND_NUM_MPDU_RANGES_GET(*(msg_word + 1)),
		  HTT_RX_IND_REL_SEQ_NUM_END_GET(*(msg_word + 1)),
		  HTT_RX_IND_REL_SEQ_NUM_START_GET(*(msg_word + 1)),
		  HTT_RX_IND_FLUSH_SEQ_NUM_END_GET(*(msg_word + 1)),
		  HTT_RX_IND_FLUSH_SEQ_NUM_START_GET(*(msg_word + 1)));

	qdf_print("fw_rx_desc_bytes %x\n",
		  HTT_RX_IND_FW_RX_DESC_BYTES_GET(*
						  (msg_word + 2 +
						   HTT_RX_PPDU_DESC_SIZE32)));

	/* receive MSDU desc for current frame */
	byte_offset =
		HTT_ENDIAN_BYTE_IDX_SWAP(HTT_RX_IND_FW_RX_DESC_BYTE_OFFSET +
					 pdev->rx_ind_msdu_byte_idx);

	qdf_print("msdu byte idx %x msdu desc %x\n", pdev->rx_ind_msdu_byte_idx,
		  HTT_RX_IND_FW_RX_DESC_BYTES_GET(*
						  (msg_word + 2 +
						   HTT_RX_PPDU_DESC_SIZE32)));

	num_mpdu_range = HTT_RX_IND_NUM_MPDU_RANGES_GET(*(msg_word + 1));

	for (mpdu_range = 0; mpdu_range < num_mpdu_range; mpdu_range++) {
		enum htt_rx_status status;
		int num_mpdus;

		htt_rx_ind_mpdu_range_info(pdev, rx_ind_msg, mpdu_range,
					   &status, &num_mpdus);

		qdf_print("mpdu_range %x status %x num_mpdus %x\n",
			  pdev->rx_ind_msdu_byte_idx, status, num_mpdus);
	}
	qdf_print
		("---------------------------------------------------------\n");
}
#endif

#ifdef DEBUG_DMA_DONE
#define MAX_DONE_BIT_CHECK_ITER 5
#endif

int
htt_rx_amsdu_pop_ll(htt_pdev_handle pdev,
		    qdf_nbuf_t rx_ind_msg,
		    qdf_nbuf_t *head_msdu, qdf_nbuf_t *tail_msdu)
{
	int msdu_len, msdu_chaining = 0;
	qdf_nbuf_t msdu;
	struct htt_host_rx_desc_base *rx_desc;
	uint8_t *rx_ind_data;
	uint32_t *msg_word, num_msdu_bytes;
	enum htt_t2h_msg_type msg_type;
	uint8_t pad_bytes = 0;

	HTT_ASSERT1(htt_rx_ring_elems(pdev) != 0);
	rx_ind_data = qdf_nbuf_data(rx_ind_msg);
	msg_word = (uint32_t *) rx_ind_data;

	msg_type = HTT_T2H_MSG_TYPE_GET(*msg_word);

	if (qdf_unlikely(HTT_T2H_MSG_TYPE_RX_FRAG_IND == msg_type)) {
		num_msdu_bytes = HTT_RX_FRAG_IND_FW_RX_DESC_BYTES_GET(
			*(msg_word + HTT_RX_FRAG_IND_HDR_PREFIX_SIZE32));
	} else {
		num_msdu_bytes = HTT_RX_IND_FW_RX_DESC_BYTES_GET(
			*(msg_word
			  + HTT_RX_IND_HDR_PREFIX_SIZE32
			  + HTT_RX_PPDU_DESC_SIZE32));
	}
	msdu = *head_msdu = htt_rx_netbuf_pop(pdev);
	while (1) {
		int last_msdu, msdu_len_invalid, msdu_chained;
		int byte_offset;

		/*
		 * Set the netbuf length to be the entire buffer length
		 * initially, so the unmap will unmap the entire buffer.
		 */
		qdf_nbuf_set_pktlen(msdu, HTT_RX_BUF_SIZE);
#ifdef DEBUG_DMA_DONE
		qdf_nbuf_unmap(pdev->osdev, msdu, QDF_DMA_BIDIRECTIONAL);
#else
		qdf_nbuf_unmap(pdev->osdev, msdu, QDF_DMA_FROM_DEVICE);
#endif

		/* cache consistency has been taken care of by qdf_nbuf_unmap */

		/*
		 * Now read the rx descriptor.
		 * Set the length to the appropriate value.
		 * Check if this MSDU completes a MPDU.
		 */
		rx_desc = htt_rx_desc(msdu);
#if defined(HELIUMPLUS_PADDR64)
		if (HTT_WIFI_IP(pdev, 2, 0))
			pad_bytes = rx_desc->msdu_end.l3_header_padding;
#endif /* defined(HELIUMPLUS_PADDR64) */
		/*
		 * Make the netbuf's data pointer point to the payload rather
		 * than the descriptor.
		 */

		qdf_nbuf_pull_head(msdu,
				   HTT_RX_STD_DESC_RESERVATION + pad_bytes);

		/*
		 * Sanity check - confirm the HW is finished filling in
		 * the rx data.
		 * If the HW and SW are working correctly, then it's guaranteed
		 * that the HW's MAC DMA is done before this point in the SW.
		 * To prevent the case that we handle a stale Rx descriptor,
		 * just assert for now until we have a way to recover.
		 */

#ifdef DEBUG_DMA_DONE
		if (qdf_unlikely(!((*(uint32_t *) &rx_desc->attention)
				   & RX_ATTENTION_0_MSDU_DONE_MASK))) {

			int dbg_iter = MAX_DONE_BIT_CHECK_ITER;

			qdf_print("malformed frame\n");

			while (dbg_iter &&
			       (!((*(uint32_t *) &rx_desc->attention) &
				  RX_ATTENTION_0_MSDU_DONE_MASK))) {
				qdf_mdelay(1);

				qdf_invalidate_range((void *)rx_desc,
						     (void *)((char *)rx_desc +
						 HTT_RX_STD_DESC_RESERVATION));

				qdf_print("debug iter %d success %d\n",
					  dbg_iter,
					  pdev->rx_ring.dbg_sync_success);

				dbg_iter--;
			}

			if (qdf_unlikely(!((*(uint32_t *) &rx_desc->attention)
					   & RX_ATTENTION_0_MSDU_DONE_MASK))) {

#ifdef HTT_RX_RESTORE
				qdf_print("RX done bit error detected!\n");
				qdf_nbuf_set_next(msdu, NULL);
				*tail_msdu = msdu;
				pdev->rx_ring.rx_reset = 1;
				return msdu_chaining;
#else
				wma_cli_set_command(0, GEN_PARAM_CRASH_INJECT,
						    0, GEN_CMD);
				HTT_ASSERT_ALWAYS(0);
#endif
			}
			pdev->rx_ring.dbg_sync_success++;
			qdf_print("debug iter %d success %d\n", dbg_iter,
				  pdev->rx_ring.dbg_sync_success);
		}
#else
		HTT_ASSERT_ALWAYS((*(uint32_t *) &rx_desc->attention) &
				  RX_ATTENTION_0_MSDU_DONE_MASK);
#endif
		/*
		 * Copy the FW rx descriptor for this MSDU from the rx
		 * indication message into the MSDU's netbuf.
		 * HL uses the same rx indication message definition as LL, and
		 * simply appends new info (fields from the HW rx desc, and the
		 * MSDU payload itself).
		 * So, the offset into the rx indication message only has to
		 * account for the standard offset of the per-MSDU FW rx
		 * desc info within the message, and how many bytes of the
		 * per-MSDU FW rx desc info have already been consumed.
		 * (And the endianness of the host,
		 * since for a big-endian host, the rx ind message contents,
		 * including the per-MSDU rx desc bytes, were byteswapped during
		 * upload.)
		 */
		if (pdev->rx_ind_msdu_byte_idx < num_msdu_bytes) {
			if (qdf_unlikely
				    (HTT_T2H_MSG_TYPE_RX_FRAG_IND == msg_type))
				byte_offset =
					HTT_ENDIAN_BYTE_IDX_SWAP
					(HTT_RX_FRAG_IND_FW_DESC_BYTE_OFFSET);
			else
				byte_offset =
					HTT_ENDIAN_BYTE_IDX_SWAP
					(HTT_RX_IND_FW_RX_DESC_BYTE_OFFSET +
						pdev->rx_ind_msdu_byte_idx);

			*((uint8_t *) &rx_desc->fw_desc.u.val) =
				rx_ind_data[byte_offset];
			/*
			 * The target is expected to only provide the basic
			 * per-MSDU rx descriptors.  Just to be sure,
			 * verify that the target has not attached
			 * extension data (e.g. LRO flow ID).
			 */
			/*
			 * The assertion below currently doesn't work for
			 * RX_FRAG_IND messages, since their format differs
			 * from the RX_IND format (no FW rx PPDU desc in
			 * the current RX_FRAG_IND message).
			 * If the RX_FRAG_IND message format is updated to match
			 * the RX_IND message format, then the following
			 * assertion can be restored.
			 */
			/* qdf_assert((rx_ind_data[byte_offset] &
			   FW_RX_DESC_EXT_M) == 0); */
			pdev->rx_ind_msdu_byte_idx += 1;
			/* or more, if there's ext data */
		} else {
			/*
			 * When an oversized AMSDU happened, FW will lost some
			 * of MSDU status - in this case, the FW descriptors
			 * provided will be less than the actual MSDUs
			 * inside this MPDU.
			 * Mark the FW descriptors so that it will still
			 * deliver to upper stack, if no CRC error for the MPDU.
			 *
			 * FIX THIS - the FW descriptors are actually for MSDUs
			 * in the end of this A-MSDU instead of the beginning.
			 */
			*((uint8_t *) &rx_desc->fw_desc.u.val) = 0;
		}

		/*
		 *  TCP/UDP checksum offload support
		 */
		htt_set_checksum_result_ll(pdev, msdu, rx_desc);

		msdu_len_invalid = (*(uint32_t *) &rx_desc->attention) &
				   RX_ATTENTION_0_MPDU_LENGTH_ERR_MASK;
		msdu_chained = (((*(uint32_t *) &rx_desc->frag_info) &
				 RX_FRAG_INFO_0_RING2_MORE_COUNT_MASK) >>
				RX_FRAG_INFO_0_RING2_MORE_COUNT_LSB);
		msdu_len =
			((*((uint32_t *) &rx_desc->msdu_start)) &
			 RX_MSDU_START_0_MSDU_LENGTH_MASK) >>
			RX_MSDU_START_0_MSDU_LENGTH_LSB;

		do {
			if (!msdu_len_invalid && !msdu_chained) {
#if defined(PEREGRINE_1_0_ZERO_LEN_PHY_ERR_WAR)
				if (msdu_len > 0x3000)
					break;
#endif
				qdf_nbuf_trim_tail(msdu,
						   HTT_RX_BUF_SIZE -
						   (RX_STD_DESC_SIZE +
						    msdu_len));
			}
		} while (0);

		while (msdu_chained--) {
			qdf_nbuf_t next = htt_rx_netbuf_pop(pdev);
			qdf_nbuf_set_pktlen(next, HTT_RX_BUF_SIZE);
			msdu_len -= HTT_RX_BUF_SIZE;
			qdf_nbuf_set_next(msdu, next);
			msdu = next;
			msdu_chaining = 1;

			if (msdu_chained == 0) {
				/* Trim the last one to the correct size -
				 * accounting for inconsistent HW lengths
				 * causing length overflows and underflows
				 */
				if (((unsigned)msdu_len) >
				    ((unsigned)
				     (HTT_RX_BUF_SIZE - RX_STD_DESC_SIZE))) {
					msdu_len =
						(HTT_RX_BUF_SIZE -
						 RX_STD_DESC_SIZE);
				}

				qdf_nbuf_trim_tail(next,
						   HTT_RX_BUF_SIZE -
						   (RX_STD_DESC_SIZE +
						    msdu_len));
			}
		}

		last_msdu =
			((*(((uint32_t *) &rx_desc->msdu_end) + 4)) &
			 RX_MSDU_END_4_LAST_MSDU_MASK) >>
			RX_MSDU_END_4_LAST_MSDU_LSB;

		if (last_msdu) {
			qdf_nbuf_set_next(msdu, NULL);
			break;
		} else {
			qdf_nbuf_t next = htt_rx_netbuf_pop(pdev);
			qdf_nbuf_set_next(msdu, next);
			msdu = next;
		}
	}
	*tail_msdu = msdu;

	/*
	 * Don't refill the ring yet.
	 * First, the elements popped here are still in use - it is
	 * not safe to overwrite them until the matching call to
	 * mpdu_desc_list_next.
	 * Second, for efficiency it is preferable to refill the rx ring
	 * with 1 PPDU's worth of rx buffers (something like 32 x 3 buffers),
	 * rather than one MPDU's worth of rx buffers (sth like 3 buffers).
	 * Consequently, we'll rely on the txrx SW to tell us when it is done
	 * pulling all the PPDU's rx buffers out of the rx ring, and then
	 * refill it just once.
	 */
	return msdu_chaining;
}

int
htt_rx_offload_msdu_pop_ll(htt_pdev_handle pdev,
			   qdf_nbuf_t offload_deliver_msg,
			   int *vdev_id,
			   int *peer_id,
			   int *tid,
			   uint8_t *fw_desc,
			   qdf_nbuf_t *head_buf, qdf_nbuf_t *tail_buf)
{
	qdf_nbuf_t buf;
	uint32_t *msdu_hdr, msdu_len;

	*head_buf = *tail_buf = buf = htt_rx_netbuf_pop(pdev);
	/* Fake read mpdu_desc to keep desc ptr in sync */
	htt_rx_mpdu_desc_list_next(pdev, NULL);
	qdf_nbuf_set_pktlen(buf, HTT_RX_BUF_SIZE);
#ifdef DEBUG_DMA_DONE
	qdf_nbuf_unmap(pdev->osdev, buf, QDF_DMA_BIDIRECTIONAL);
#else
	qdf_nbuf_unmap(pdev->osdev, buf, QDF_DMA_FROM_DEVICE);
#endif
	msdu_hdr = (uint32_t *) qdf_nbuf_data(buf);

	/* First dword */
	msdu_len = HTT_RX_OFFLOAD_DELIVER_IND_MSDU_LEN_GET(*msdu_hdr);
	*peer_id = HTT_RX_OFFLOAD_DELIVER_IND_MSDU_PEER_ID_GET(*msdu_hdr);

	/* Second dword */
	msdu_hdr++;
	*vdev_id = HTT_RX_OFFLOAD_DELIVER_IND_MSDU_VDEV_ID_GET(*msdu_hdr);
	*tid = HTT_RX_OFFLOAD_DELIVER_IND_MSDU_TID_GET(*msdu_hdr);
	*fw_desc = HTT_RX_OFFLOAD_DELIVER_IND_MSDU_DESC_GET(*msdu_hdr);

	qdf_nbuf_pull_head(buf, HTT_RX_OFFLOAD_DELIVER_IND_MSDU_HDR_BYTES);
	qdf_nbuf_set_pktlen(buf, msdu_len);
	return 0;
}

int
htt_rx_offload_paddr_msdu_pop_ll(htt_pdev_handle pdev,
				 uint32_t *msg_word,
				 int msdu_iter,
				 int *vdev_id,
				 int *peer_id,
				 int *tid,
				 uint8_t *fw_desc,
				 qdf_nbuf_t *head_buf, qdf_nbuf_t *tail_buf)
{
	qdf_nbuf_t buf;
	uint32_t *msdu_hdr, msdu_len;
	uint32_t *curr_msdu;
	uint32_t paddr;

	curr_msdu =
		msg_word + (msdu_iter * HTT_RX_IN_ORD_PADDR_IND_MSDU_DWORDS);
	paddr = HTT_RX_IN_ORD_PADDR_IND_PADDR_GET(*curr_msdu);
	*head_buf = *tail_buf = buf = htt_rx_in_order_netbuf_pop(pdev, paddr);

	if (qdf_unlikely(NULL == buf)) {
		qdf_print("%s: netbuf pop failed!\n", __func__);
		return 0;
	}
	qdf_nbuf_set_pktlen(buf, HTT_RX_BUF_SIZE);
#ifdef DEBUG_DMA_DONE
	qdf_nbuf_unmap(pdev->osdev, buf, QDF_DMA_BIDIRECTIONAL);
#else
	qdf_nbuf_unmap(pdev->osdev, buf, QDF_DMA_FROM_DEVICE);
#endif
	msdu_hdr = (uint32_t *) qdf_nbuf_data(buf);

	/* First dword */
	msdu_len = HTT_RX_OFFLOAD_DELIVER_IND_MSDU_LEN_GET(*msdu_hdr);
	*peer_id = HTT_RX_OFFLOAD_DELIVER_IND_MSDU_PEER_ID_GET(*msdu_hdr);

	/* Second dword */
	msdu_hdr++;
	*vdev_id = HTT_RX_OFFLOAD_DELIVER_IND_MSDU_VDEV_ID_GET(*msdu_hdr);
	*tid = HTT_RX_OFFLOAD_DELIVER_IND_MSDU_TID_GET(*msdu_hdr);
	*fw_desc = HTT_RX_OFFLOAD_DELIVER_IND_MSDU_DESC_GET(*msdu_hdr);

	qdf_nbuf_pull_head(buf, HTT_RX_OFFLOAD_DELIVER_IND_MSDU_HDR_BYTES);
	qdf_nbuf_set_pktlen(buf, msdu_len);
	return 0;
}

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#if HTT_PADDR64
#define NEXT_FIELD_OFFSET_IN32 2
#else /* ! HTT_PADDR64 */
#define NEXT_FIELD_OFFSET_IN32 1
#endif /* HTT_PADDR64 */

/**
 * htt_mon_rx_handle_amsdu_packet() - Handle consecutive fragments of amsdu
 * @msdu: pointer to first msdu of amsdu
 * @pdev: Handle to htt_pdev_handle
 * @msg_word: Input and output variable, so pointer to HTT msg pointer
 * @amsdu_len: remaining length of all N-1 msdu msdu's
 *
 * This function handles the (N-1) msdu's of amsdu, N'th msdu is already
 * handled by calling function. N-1 msdu's are tied using frags_list.
 * msdu_info field updated by FW indicates if this is last msdu. All the
 * msdu's before last msdu will be of MAX payload.
 *
 * Return: 1 on success and 0 on failure.
 */
int htt_mon_rx_handle_amsdu_packet(qdf_nbuf_t msdu, htt_pdev_handle pdev,
				   uint32_t **msg_word, uint32_t amsdu_len)
{
	qdf_nbuf_t frag_nbuf;
	qdf_nbuf_t prev_frag_nbuf;
	uint32_t len;
	uint32_t last_frag;

	*msg_word += HTT_RX_IN_ORD_PADDR_IND_MSDU_DWORDS;
	frag_nbuf = htt_rx_in_order_netbuf_pop(pdev,
				HTT_RX_IN_ORD_PADDR_IND_PADDR_GET(**msg_word));
	if (qdf_unlikely(NULL == frag_nbuf)) {
		qdf_print("%s: netbuf pop failed!\n", __func__);
		return 0;
	}
	last_frag = ((struct htt_rx_in_ord_paddr_ind_msdu_t *)*msg_word)->
		msdu_info;
	qdf_nbuf_append_ext_list(msdu, frag_nbuf, amsdu_len);
	qdf_nbuf_set_pktlen(frag_nbuf, HTT_RX_BUF_SIZE);
	qdf_nbuf_unmap(pdev->osdev, frag_nbuf, QDF_DMA_FROM_DEVICE);
	/* For msdu's other than parent will not have htt_host_rx_desc_base */
	len = MIN(amsdu_len, HTT_RX_BUF_SIZE);
	amsdu_len -= len;
	qdf_nbuf_trim_tail(frag_nbuf, HTT_RX_BUF_SIZE - len);

	HTT_PKT_DUMP(qdf_trace_hex_dump(QDF_MODULE_ID_TXRX,
					QDF_TRACE_LEVEL_FATAL,
					qdf_nbuf_data(frag_nbuf),
					qdf_nbuf_len(frag_nbuf)));
	prev_frag_nbuf = frag_nbuf;
	while (!last_frag) {
		*msg_word += HTT_RX_IN_ORD_PADDR_IND_MSDU_DWORDS;
		frag_nbuf = htt_rx_in_order_netbuf_pop(pdev,
				HTT_RX_IN_ORD_PADDR_IND_PADDR_GET(**msg_word));
		last_frag = ((struct htt_rx_in_ord_paddr_ind_msdu_t *)
			     *msg_word)->msdu_info;

		if (qdf_unlikely(NULL == frag_nbuf)) {
			qdf_print("%s: netbuf pop failed!\n", __func__);
			prev_frag_nbuf->next = NULL;
			return 0;
		}
		qdf_nbuf_set_pktlen(frag_nbuf, HTT_RX_BUF_SIZE);
		qdf_nbuf_unmap(pdev->osdev, frag_nbuf, QDF_DMA_FROM_DEVICE);

		len = MIN(amsdu_len, HTT_RX_BUF_SIZE);
		amsdu_len -= len;
		qdf_nbuf_trim_tail(frag_nbuf, HTT_RX_BUF_SIZE - len);
		HTT_PKT_DUMP(qdf_trace_hex_dump(QDF_MODULE_ID_TXRX,
						QDF_TRACE_LEVEL_FATAL,
						qdf_nbuf_data(frag_nbuf),
						qdf_nbuf_len(frag_nbuf)));

		qdf_nbuf_set_next(prev_frag_nbuf, frag_nbuf);
		prev_frag_nbuf = frag_nbuf;
	}
	qdf_nbuf_set_next(prev_frag_nbuf, NULL);
	return 1;
}

/**
 * htt_mon_rx_get_phy_info() - Get rate interms of 500Kbps.
 * @rx_desc: Pointer to struct htt_host_rx_desc_base
 * @rx_status: Return variable updated with phy_info in rx_status
 *
 * If l_sig_rate_select is 0:
 * 0x8: OFDM 48 Mbps
 * 0x9: OFDM 24 Mbps
 * 0xA: OFDM 12 Mbps
 * 0xB: OFDM 6 Mbps
 * 0xC: OFDM 54 Mbps
 * 0xD: OFDM 36 Mbps
 * 0xE: OFDM 18 Mbps
 * 0xF: OFDM 9 Mbps
 * If l_sig_rate_select is 1:
 * 0x8: CCK 11 Mbps long preamble
 * 0x9: CCK 5.5 Mbps long preamble
 * 0xA: CCK 2 Mbps long preamble
 * 0xB: CCK 1 Mbps long preamble
 * 0xC: CCK 11 Mbps short preamble
 * 0xD: CCK 5.5 Mbps short preamble
 * 0xE: CCK 2 Mbps short preamble
 *
 * Return: None
 */
static void htt_mon_rx_get_phy_info(struct htt_host_rx_desc_base *rx_desc,
				    struct mon_rx_status *rx_status)
{
#define SHORT_PREAMBLE 1
#define LONG_PREAMBLE  0
	char rate = 0x0;
	uint8_t preamble = SHORT_PREAMBLE;
	uint8_t preamble_type = rx_desc->ppdu_start.preamble_type;
	uint8_t mcs = 0, nss = 0, sgi = 0, bw = 0, beamformed = 0;
	uint16_t vht_flags = 0;
	uint32_t l_sig_rate_select = rx_desc->ppdu_start.l_sig_rate_select;
	uint32_t l_sig_rate = rx_desc->ppdu_start.l_sig_rate;
	bool is_stbc = 0, ldpc = 0;

	if (l_sig_rate_select == 0) {
		switch (l_sig_rate) {
		case 0x8:
			rate = 0x60;
			break;
		case 0x9:
			rate = 0x30;
			break;
		case 0xA:
			rate = 0x18;
			break;
		case 0xB:
			rate = 0x0c;
			break;
		case 0xC:
			rate = 0x6c;
			break;
		case 0xD:
			rate = 0x48;
			break;
		case 0xE:
			rate = 0x24;
			break;
		case 0xF:
			rate = 0x12;
			break;
		default:
			break;
		}
	} else if (l_sig_rate_select == 1) {
		switch (l_sig_rate) {
		case 0x8:
			rate = 0x16;
			preamble = LONG_PREAMBLE;
			break;
		case 0x9:
			rate = 0x0B;
			preamble = LONG_PREAMBLE;
			break;
		case 0xA:
			rate = 0x04;
			preamble = LONG_PREAMBLE;
			break;
		case 0xB:
			rate = 0x02;
			preamble = LONG_PREAMBLE;
			break;
		case 0xC:
			rate = 0x16;
			break;
		case 0xD:
			rate = 0x0B;
			break;
		case 0xE:
			rate = 0x04;
			break;
		default:
			break;
		}
	} else {
		qdf_print("Invalid rate info\n");
	}

	switch (preamble_type) {
	case 8:
		is_stbc = ((VHT_SIG_A_2(rx_desc) >> 4) & 3);
		/* fallthrough */
	case 9:
		vht_flags = 1;
		sgi = (VHT_SIG_A_2(rx_desc) >> 7) & 0x01;
		bw = (VHT_SIG_A_1(rx_desc) >> 7) & 0x01;
		mcs = (VHT_SIG_A_1(rx_desc) & 0x7f);
		nss = mcs>>3;
		beamformed =
			(VHT_SIG_A_2(rx_desc) >> 8) & 0x1;
		break;
	case 0x0c:
		vht_flags = 1;
		is_stbc = (VHT_SIG_A_2(rx_desc) >> 3) & 1;
		ldpc = (VHT_SIG_A_2(rx_desc) >> 2) & 1;
		/* fallthrough */
	case 0x0d:
	{
		uint8_t gid_in_sig = ((VHT_SIG_A_1(rx_desc) >> 4) & 0x3f);

		vht_flags = 1;
		sgi = VHT_SIG_A_2(rx_desc) & 0x01;
		bw = (VHT_SIG_A_1(rx_desc) & 0x03);
		if (gid_in_sig == 0 || gid_in_sig == 63) {
			/* SU case */
			mcs = (VHT_SIG_A_2(rx_desc) >> 4) &
				0xf;
			nss = (VHT_SIG_A_1(rx_desc) >> 10) &
				0x7;
		} else {
			/* SU case */
			uint8_t sta_user_pos =
				(uint8_t)((rx_desc->ppdu_start.reserved_4a >> 8)
					  & 0x3);
			mcs = (rx_desc->ppdu_start.vht_sig_b >> 16);
			if (bw >= 2)
				mcs >>= 3;
			else if (bw > 0)
				mcs >>= 1;
			mcs &= 0xf;
			nss = (((VHT_SIG_A_1(rx_desc) >> 10) +
				sta_user_pos * 3) & 0x7);
		}
		beamformed = (VHT_SIG_A_2(rx_desc) >> 8) & 0x1;
	}
		/* fallthrough */
	default:
		break;
	}

	rx_status->mcs = mcs;
	rx_status->nr_ant = nss;
	rx_status->is_stbc = is_stbc;
	rx_status->sgi = sgi;
	rx_status->ldpc = ldpc;
	rx_status->beamformed = beamformed;
	rx_status->vht_flag_values3[0] = mcs << 0x4;
	rx_status->rate = rate;
	rx_status->vht_flags = vht_flags;
	rx_status->rtap_flags |= ((preamble == SHORT_PREAMBLE) ? BIT(1) : 0);
	rx_status->vht_flag_values2 = 0x01 < bw;
}

/**
 * htt_mon_rx_get_rtap_flags() - Get radiotap flags
 * @rx_desc: Pointer to struct htt_host_rx_desc_base
 *
 * Return: Bitmapped radiotap flags.
 */
static uint8_t htt_mon_rx_get_rtap_flags(struct htt_host_rx_desc_base *rx_desc)
{
	uint8_t rtap_flags = 0;

	/* WEP40 || WEP104 || WEP128 */
	if (rx_desc->mpdu_start.encrypt_type == 0 ||
	    rx_desc->mpdu_start.encrypt_type == 1 ||
	    rx_desc->mpdu_start.encrypt_type == 3)
		rtap_flags |= BIT(2);

	/* IEEE80211_RADIOTAP_F_FRAG */
	if (rx_desc->attention.fragment)
		rtap_flags |= BIT(3);

	/* IEEE80211_RADIOTAP_F_FCS */
	rtap_flags |= BIT(4);

	/* IEEE80211_RADIOTAP_F_BADFCS */
	if (rx_desc->mpdu_end.fcs_err)
		rtap_flags |= BIT(6);

	return rtap_flags;
}

/**
 * htt_rx_mon_get_rx_status() - Update information about the rx status,
 * which is used later for radiotap updation.
 * @rx_desc: Pointer to struct htt_host_rx_desc_base
 * @rx_status: Return variable updated with rx_status
 *
 * Return: None
 */
void htt_rx_mon_get_rx_status(htt_pdev_handle pdev,
			      struct htt_host_rx_desc_base *rx_desc,
			      struct mon_rx_status *rx_status)
{
	uint16_t channel_flags = 0;
	struct mon_channel *ch_info = &pdev->mon_ch_info;

	rx_status->tsft = (u_int64_t)TSF_TIMESTAMP(rx_desc);
	rx_status->chan_freq = ch_info->ch_freq;
	rx_status->chan_num = ch_info->ch_num;
	htt_mon_rx_get_phy_info(rx_desc, rx_status);
	rx_status->rtap_flags |= htt_mon_rx_get_rtap_flags(rx_desc);
	channel_flags |= rx_desc->ppdu_start.l_sig_rate_select ?
		IEEE80211_CHAN_CCK : IEEE80211_CHAN_OFDM;
	channel_flags |=
		(cds_chan_to_band(ch_info->ch_num) == CDS_BAND_2GHZ ?
		IEEE80211_CHAN_2GHZ : IEEE80211_CHAN_5GHZ);

	rx_status->chan_flags = channel_flags;
	rx_status->ant_signal_db = rx_desc->ppdu_start.rssi_comb;
}

#ifdef RX_HASH_DEBUG
#define HTT_RX_CHECK_MSDU_COUNT(msdu_count) HTT_ASSERT_ALWAYS(msdu_count)
#else
#define HTT_RX_CHECK_MSDU_COUNT(msdu_count)     /* no-op */
#endif

/**
 * htt_rx_mon_amsdu_rx_in_order_pop_ll() - Monitor mode HTT Rx in order pop
 * function
 * @pdev: Handle to htt_pdev_handle
 * @rx_ind_msg: In order indication message.
 * @head_msdu: Return variable pointing to head msdu.
 * @tail_msdu: Return variable pointing to tail msdu.
 *
 * This function pops the msdu based on paddr:length of inorder indication
 * message.
 *
 * Return: 1 for success, 0 on failure.
 */
int htt_rx_mon_amsdu_rx_in_order_pop_ll(htt_pdev_handle pdev,
					qdf_nbuf_t rx_ind_msg,
					qdf_nbuf_t *head_msdu,
					qdf_nbuf_t *tail_msdu)
{
	qdf_nbuf_t msdu, next;
	uint8_t *rx_ind_data;
	uint32_t *msg_word;
	uint32_t msdu_count;
	struct htt_host_rx_desc_base *rx_desc;
	struct mon_rx_status rx_status = {0};
	uint32_t amsdu_len;
	uint32_t len;
	uint32_t last_frag;

	HTT_ASSERT1(htt_rx_in_order_ring_elems(pdev) != 0);

	rx_ind_data = qdf_nbuf_data(rx_ind_msg);
	msg_word = (uint32_t *)rx_ind_data;

	HTT_PKT_DUMP(qdf_trace_hex_dump(QDF_MODULE_ID_TXRX,
					QDF_TRACE_LEVEL_FATAL,
					(void *)rx_ind_data,
					(int)qdf_nbuf_len(rx_ind_msg)));

	/* Get the total number of MSDUs */
	msdu_count = HTT_RX_IN_ORD_PADDR_IND_MSDU_CNT_GET(*(msg_word + 1));
	HTT_RX_CHECK_MSDU_COUNT(msdu_count);

	msg_word = (uint32_t *)(rx_ind_data +
				 HTT_RX_IN_ORD_PADDR_IND_HDR_BYTES);

	(*head_msdu) = msdu = htt_rx_in_order_netbuf_pop(pdev,
				HTT_RX_IN_ORD_PADDR_IND_PADDR_GET(*msg_word));

	if (qdf_unlikely(NULL == msdu)) {
		qdf_print("%s: netbuf pop failed!\n", __func__);
		*tail_msdu = NULL;
		return 0;
	}
	while (msdu_count > 0) {

		msdu_count--;
		/*
		 * Set the netbuf length to be the entire buffer length
		 * initially, so the unmap will unmap the entire buffer.
		 */
		qdf_nbuf_set_pktlen(msdu, HTT_RX_BUF_SIZE);
		qdf_nbuf_unmap(pdev->osdev, msdu, QDF_DMA_FROM_DEVICE);

		/*
		 * cache consistency has been taken care of by the
		 * qdf_nbuf_unmap
		 */
		rx_desc = htt_rx_desc(msdu);
		HTT_PKT_DUMP(htt_print_rx_desc(rx_desc));
		/*
		 * Make the netbuf's data pointer point to the payload rather
		 * than the descriptor.
		 */
		htt_rx_mon_get_rx_status(pdev, rx_desc, &rx_status);
		/*
		 * 350 bytes of RX_STD_DESC size should be sufficient for
		 * radiotap.
		 */
		qdf_nbuf_update_radiotap(&rx_status, msdu,
						  HTT_RX_STD_DESC_RESERVATION);
		amsdu_len = HTT_RX_IN_ORD_PADDR_IND_MSDU_LEN_GET(*(msg_word +
						NEXT_FIELD_OFFSET_IN32));

		/*
		 * MAX_RX_PAYLOAD_SZ when we have AMSDU packet. amsdu_len in
		 * which case is the total length of sum of all AMSDU's
		 */
		len = MIN(amsdu_len, MAX_RX_PAYLOAD_SZ);
		amsdu_len -= len;
		qdf_nbuf_trim_tail(msdu,
			   HTT_RX_BUF_SIZE -
			   (RX_STD_DESC_SIZE + len));


		HTT_PKT_DUMP(qdf_trace_hex_dump(QDF_MODULE_ID_TXRX,
						QDF_TRACE_LEVEL_FATAL,
						qdf_nbuf_data(msdu),
						qdf_nbuf_len(msdu)));
		last_frag = ((struct htt_rx_in_ord_paddr_ind_msdu_t *)
			     msg_word)->msdu_info;

#undef NEXT_FIELD_OFFSET_IN32
		/* Handle amsdu packet */
		if (!last_frag) {
			/*
			 * For AMSDU packet msdu->len is sum of all the msdu's
			 * length, msdu->data_len is sum of length's of
			 * remaining msdu's other than parent.
			 */
			if (!htt_mon_rx_handle_amsdu_packet(msdu, pdev,
							    &msg_word,
							    amsdu_len)) {
				qdf_print("%s: failed to handle amsdu packet\n",
					     __func__);
				return 0;
			}
		}
		/* check if this is the last msdu */
		if (msdu_count) {
			msg_word += HTT_RX_IN_ORD_PADDR_IND_MSDU_DWORDS;
			next = htt_rx_in_order_netbuf_pop(pdev,
				HTT_RX_IN_ORD_PADDR_IND_PADDR_GET(*msg_word));
			if (qdf_unlikely(NULL == next)) {
				qdf_print("%s: netbuf pop failed!\n",
					     __func__);
				*tail_msdu = NULL;
				return 0;
			}
			qdf_nbuf_set_next(msdu, next);
			msdu = next;
		} else {
			*tail_msdu = msdu;
			qdf_nbuf_set_next(msdu, NULL);
		}
	}

	return 1;
}

/**
 * htt_rx_mon_note_capture_channel() - Make note of channel to update in
 * radiotap
 * @pdev: handle to htt_pdev
 * @mon_ch: capture channel number.
 *
 * Return: None
 */
void htt_rx_mon_note_capture_channel(htt_pdev_handle pdev, int mon_ch)
{
	struct mon_channel *ch_info = &pdev->mon_ch_info;

	ch_info->ch_num = mon_ch;
	ch_info->ch_freq = cds_chan_to_freq(mon_ch);
}

extern void
dump_pkt(qdf_nbuf_t nbuf, uint32_t nbuf_paddr, int len);

/* Return values: 1 - success, 0 - failure */
int
htt_rx_amsdu_rx_in_order_pop_ll(htt_pdev_handle pdev,
				qdf_nbuf_t rx_ind_msg,
				qdf_nbuf_t *head_msdu, qdf_nbuf_t *tail_msdu)
{
	qdf_nbuf_t msdu, next, prev = NULL;
	uint8_t *rx_ind_data;
	uint32_t *msg_word;
	unsigned int msdu_count = 0;
	uint8_t offload_ind;
	struct htt_host_rx_desc_base *rx_desc;

	HTT_ASSERT1(htt_rx_in_order_ring_elems(pdev) != 0);

	rx_ind_data = qdf_nbuf_data(rx_ind_msg);
	msg_word = (uint32_t *) rx_ind_data;

	offload_ind = HTT_RX_IN_ORD_PADDR_IND_OFFLOAD_GET(*msg_word);

	/* Get the total number of MSDUs */
	msdu_count = HTT_RX_IN_ORD_PADDR_IND_MSDU_CNT_GET(*(msg_word + 1));
	HTT_RX_CHECK_MSDU_COUNT(msdu_count);
	ol_rx_update_histogram_stats(msdu_count);

	msg_word =
		(uint32_t *) (rx_ind_data + HTT_RX_IN_ORD_PADDR_IND_HDR_BYTES);
	if (offload_ind) {
		ol_rx_offload_paddr_deliver_ind_handler(pdev, msdu_count,
							msg_word);
		*head_msdu = *tail_msdu = NULL;
		return 0;
	}

	(*head_msdu) = msdu = htt_rx_in_order_netbuf_pop(
		pdev,
		HTT_RX_IN_ORD_PADDR_IND_PADDR_GET(*msg_word));

	if (qdf_unlikely(NULL == msdu)) {
		qdf_print("%s: netbuf pop failed!\n", __func__);
		*tail_msdu = NULL;
		return 0;
	}

	while (msdu_count > 0) {

		/*
		 * Set the netbuf length to be the entire buffer length
		 * initially, so the unmap will unmap the entire buffer.
		 */
		qdf_nbuf_set_pktlen(msdu, HTT_RX_BUF_SIZE);
#ifdef DEBUG_DMA_DONE
		qdf_nbuf_unmap(pdev->osdev, msdu, QDF_DMA_BIDIRECTIONAL);
#else
		qdf_nbuf_unmap(pdev->osdev, msdu, QDF_DMA_FROM_DEVICE);
#endif

		/* cache consistency has been taken care of by qdf_nbuf_unmap */
		rx_desc = htt_rx_desc(msdu);

		htt_rx_extract_lro_info(msdu, rx_desc);

		/*
		 * Make the netbuf's data pointer point to the payload rather
		 * than the descriptor.
		 */
		qdf_nbuf_pull_head(msdu, HTT_RX_STD_DESC_RESERVATION);
#if HTT_PADDR64
#define NEXT_FIELD_OFFSET_IN32 2
#else /* ! HTT_PADDR64 */
#define NEXT_FIELD_OFFSET_IN32 1
#endif /* HTT_PADDR64 */
#
		qdf_nbuf_trim_tail(msdu,
				   HTT_RX_BUF_SIZE -
				   (RX_STD_DESC_SIZE +
				    HTT_RX_IN_ORD_PADDR_IND_MSDU_LEN_GET(
					    *(msg_word + NEXT_FIELD_OFFSET_IN32))));
#if defined(HELIUMPLUS_DEBUG)
		dump_pkt(msdu, 0, 64);
#endif
		*((uint8_t *) &rx_desc->fw_desc.u.val) =
			HTT_RX_IN_ORD_PADDR_IND_FW_DESC_GET(*(msg_word + NEXT_FIELD_OFFSET_IN32));
#undef NEXT_FIELD_OFFSET_IN32

		msdu_count--;

		if (qdf_unlikely((*((u_int8_t *) &rx_desc->fw_desc.u.val)) &
				    FW_RX_DESC_MIC_ERR_M)) {
			u_int8_t tid =
				HTT_RX_IN_ORD_PADDR_IND_EXT_TID_GET(
					*(u_int32_t *)rx_ind_data);
			u_int16_t peer_id =
				HTT_RX_IN_ORD_PADDR_IND_PEER_ID_GET(
					*(u_int32_t *)rx_ind_data);
			ol_rx_mic_error_handler(pdev->txrx_pdev, tid, peer_id,
						rx_desc, msdu);

			htt_rx_desc_frame_free(pdev, msdu);
			/* if this is the last msdu */
			if (!msdu_count) {
				/* if this is the only msdu */
				if (!prev) {
					*head_msdu = *tail_msdu = NULL;
					return 0;
				} else {
					*tail_msdu = prev;
					qdf_nbuf_set_next(prev, NULL);
					return 1;
				}
			} else { /* if this is not the last msdu */
				/* get the next msdu */
				msg_word += HTT_RX_IN_ORD_PADDR_IND_MSDU_DWORDS;
				next = htt_rx_in_order_netbuf_pop(
					pdev,
					HTT_RX_IN_ORD_PADDR_IND_PADDR_GET(
						*msg_word));
				if (qdf_unlikely(NULL == next)) {
					qdf_print("%s: netbuf pop failed!\n",
								 __func__);
					*tail_msdu = NULL;
					return 0;
				}

				/* if this is not the first msdu, update the
				 * next pointer of the preceding msdu
				 */
				if (prev) {
					qdf_nbuf_set_next(prev, next);
				} else {
				/* if this is the first msdu, update the
				 * head pointer
				 */
					*head_msdu = next;
				}
				msdu = next;
				continue;
			}
		}

		/* Update checksum result */
		htt_set_checksum_result_ll(pdev, msdu, rx_desc);

		/* check if this is the last msdu */
		if (msdu_count) {
			msg_word += HTT_RX_IN_ORD_PADDR_IND_MSDU_DWORDS;
			next = htt_rx_in_order_netbuf_pop(
				pdev,
				HTT_RX_IN_ORD_PADDR_IND_PADDR_GET(*msg_word));
			if (qdf_unlikely(NULL == next)) {
				qdf_print("%s: netbuf pop failed!\n",
					  __func__);
				*tail_msdu = NULL;
				return 0;
			}
			qdf_nbuf_set_next(msdu, next);
			prev = msdu;
			msdu = next;
		} else {
			*tail_msdu = msdu;
			qdf_nbuf_set_next(msdu, NULL);
		}
	}

	return 1;
}

/* Util fake function that has same prototype as qdf_nbuf_clone that just
 * retures the same nbuf
 */
qdf_nbuf_t htt_rx_qdf_noclone_buf(qdf_nbuf_t buf)
{
	return buf;
}

/* FIXME: This is a HW definition not provded by HW, where does it go ? */
enum {
	HW_RX_DECAP_FORMAT_RAW = 0,
	HW_RX_DECAP_FORMAT_NWIFI,
	HW_RX_DECAP_FORMAT_8023,
	HW_RX_DECAP_FORMAT_ETH2,
};

#define HTT_FCS_LEN (4)

static void
htt_rx_parse_ppdu_start_status(struct htt_host_rx_desc_base *rx_desc,
			       struct ieee80211_rx_status *rs)
{

	struct rx_ppdu_start *ppdu_start = &rx_desc->ppdu_start;

	/* RSSI */
	rs->rs_rssi = ppdu_start->rssi_comb;

	/* PHY rate */
	/* rs_ratephy coding
	   [b3 - b0]
	   0 -> OFDM
	   1 -> CCK
	   2 -> HT
	   3 -> VHT
	   OFDM / CCK
	   [b7  - b4 ] => LSIG rate
	   [b23 - b8 ] => service field
	   (b'12 static/dynamic,
	   b'14..b'13 BW for VHT)
	   [b31 - b24 ] => Reserved
	   HT / VHT
	   [b15 - b4 ] => SIG A_2 12 LSBs
	   [b31 - b16] => SIG A_1 16 LSBs

	 */
	if (ppdu_start->preamble_type == 0x4) {
		rs->rs_ratephy = ppdu_start->l_sig_rate_select;
		rs->rs_ratephy |= ppdu_start->l_sig_rate << 4;
		rs->rs_ratephy |= ppdu_start->service << 8;
	} else {
		rs->rs_ratephy = (ppdu_start->preamble_type & 0x4) ? 3 : 2;
#ifdef HELIUMPLUS
		rs->rs_ratephy |=
			(ppdu_start->ht_sig_vht_sig_ah_sig_a_2 & 0xFFF) << 4;
		rs->rs_ratephy |=
			(ppdu_start->ht_sig_vht_sig_ah_sig_a_1 & 0xFFFF) << 16;
#else
		rs->rs_ratephy |= (ppdu_start->ht_sig_vht_sig_a_2 & 0xFFF) << 4;
		rs->rs_ratephy |=
			(ppdu_start->ht_sig_vht_sig_a_1 & 0xFFFF) << 16;
#endif
	}

	return;
}

/* This function is used by montior mode code to restitch an MSDU list
 * corresponding to an MPDU back into an MPDU by linking up the skbs.
 */
qdf_nbuf_t
htt_rx_restitch_mpdu_from_msdus(htt_pdev_handle pdev,
				qdf_nbuf_t head_msdu,
				struct ieee80211_rx_status *rx_status,
				unsigned clone_not_reqd)
{

	qdf_nbuf_t msdu, mpdu_buf, prev_buf, msdu_orig, head_frag_list_cloned;
	qdf_nbuf_t (*clone_nbuf_fn)(qdf_nbuf_t buf);
	unsigned decap_format, wifi_hdr_len, sec_hdr_len, msdu_llc_len,
		 mpdu_buf_len, decap_hdr_pull_bytes, frag_list_sum_len, dir,
		 is_amsdu, is_first_frag, amsdu_pad, msdu_len;
	struct htt_host_rx_desc_base *rx_desc;
	char *hdr_desc;
	unsigned char *dest;
	struct ieee80211_frame *wh;
	struct ieee80211_qoscntl *qos;

	/* If this packet does not go up the normal stack path we dont need to
	 * waste cycles cloning the packets
	 */
	clone_nbuf_fn =
		clone_not_reqd ? htt_rx_qdf_noclone_buf : qdf_nbuf_clone;

	/* The nbuf has been pulled just beyond the status and points to the
	 * payload
	 */
	msdu_orig = head_msdu;
	rx_desc = htt_rx_desc(msdu_orig);

	/* Fill out the rx_status from the PPDU start and end fields */
	if (rx_desc->attention.first_mpdu) {
		htt_rx_parse_ppdu_start_status(rx_desc, rx_status);

		/* The timestamp is no longer valid - It will be valid only for
		 * the last MPDU
		 */
		rx_status->rs_tstamp.tsf = ~0;
	}

	decap_format =
		GET_FIELD(&rx_desc->msdu_start, RX_MSDU_START_2_DECAP_FORMAT);

	head_frag_list_cloned = NULL;

	/* Easy case - The MSDU status indicates that this is a non-decapped
	 * packet in RAW mode.
	 * return
	 */
	if (decap_format == HW_RX_DECAP_FORMAT_RAW) {
		/* Note that this path might suffer from headroom unavailabilty,
		 * but the RX status is usually enough
		 */
		mpdu_buf = clone_nbuf_fn(head_msdu);

		prev_buf = mpdu_buf;

		frag_list_sum_len = 0;
		is_first_frag = 1;
		msdu_len = qdf_nbuf_len(mpdu_buf);

		/* Drop the zero-length msdu */
		if (!msdu_len)
			goto mpdu_stitch_fail;

		msdu_orig = qdf_nbuf_next(head_msdu);

		while (msdu_orig) {

			/* TODO: intra AMSDU padding - do we need it ??? */
			msdu = clone_nbuf_fn(msdu_orig);
			if (!msdu)
				goto mpdu_stitch_fail;

			if (is_first_frag) {
				is_first_frag = 0;
				head_frag_list_cloned = msdu;
			}

			msdu_len = qdf_nbuf_len(msdu);
			/* Drop the zero-length msdu */
			if (!msdu_len)
				goto mpdu_stitch_fail;

			frag_list_sum_len += msdu_len;

			/* Maintain the linking of the cloned MSDUS */
			qdf_nbuf_set_next_ext(prev_buf, msdu);

			/* Move to the next */
			prev_buf = msdu;
			msdu_orig = qdf_nbuf_next(msdu_orig);
		}

		/* The last msdu length need be larger than HTT_FCS_LEN */
		if (msdu_len < HTT_FCS_LEN)
			goto mpdu_stitch_fail;

		qdf_nbuf_trim_tail(prev_buf, HTT_FCS_LEN);

		/* If there were more fragments to this RAW frame */
		if (head_frag_list_cloned) {
			qdf_nbuf_append_ext_list(mpdu_buf,
						 head_frag_list_cloned,
						 frag_list_sum_len);
		}

		goto mpdu_stitch_done;
	}

	/* Decap mode:
	 * Calculate the amount of header in decapped packet to knock off based
	 * on the decap type and the corresponding number of raw bytes to copy
	 * status header
	 */

	hdr_desc = &rx_desc->rx_hdr_status[0];

	/* Base size */
	wifi_hdr_len = sizeof(struct ieee80211_frame);
	wh = (struct ieee80211_frame *)hdr_desc;

	dir = wh->i_fc[1] & IEEE80211_FC1_DIR_MASK;
	if (dir == IEEE80211_FC1_DIR_DSTODS)
		wifi_hdr_len += 6;

	is_amsdu = 0;
	if (wh->i_fc[0] & IEEE80211_FC0_SUBTYPE_QOS) {
		qos = (struct ieee80211_qoscntl *)
		      (hdr_desc + wifi_hdr_len);
		wifi_hdr_len += 2;

		is_amsdu = (qos->i_qos[0] & IEEE80211_QOS_AMSDU);
	}

	/* TODO: Any security headers associated with MPDU */
	sec_hdr_len = 0;

	/* MSDU related stuff LLC - AMSDU subframe header etc */
	msdu_llc_len = is_amsdu ? (14 + 8) : 8;

	mpdu_buf_len = wifi_hdr_len + sec_hdr_len + msdu_llc_len;

	/* "Decap" header to remove from MSDU buffer */
	decap_hdr_pull_bytes = 14;

	/* Allocate a new nbuf for holding the 802.11 header retrieved from the
	 * status of the now decapped first msdu. Leave enough headroom for
	 * accomodating any radio-tap /prism like PHY header
	 */
#define HTT_MAX_MONITOR_HEADER (512)
	mpdu_buf = qdf_nbuf_alloc(pdev->osdev,
				  HTT_MAX_MONITOR_HEADER + mpdu_buf_len,
				  HTT_MAX_MONITOR_HEADER, 4, false);

	if (!mpdu_buf)
		goto mpdu_stitch_fail;

	/* Copy the MPDU related header and enc headers into the first buffer
	 * - Note that there can be a 2 byte pad between heaader and enc header
	 */

	prev_buf = mpdu_buf;
	dest = qdf_nbuf_put_tail(prev_buf, wifi_hdr_len);
	if (!dest)
		goto mpdu_stitch_fail;
	qdf_mem_copy(dest, hdr_desc, wifi_hdr_len);
	hdr_desc += wifi_hdr_len;

	/* NOTE - This padding is present only in the RAW header status - not
	 * when the MSDU data payload is in RAW format.
	 */
	/* Skip the "IV pad" */
	if (wifi_hdr_len & 0x3)
		hdr_desc += 2;

	/* The first LLC len is copied into the MPDU buffer */
	frag_list_sum_len = 0;
	frag_list_sum_len -= msdu_llc_len;

	msdu_orig = head_msdu;
	is_first_frag = 1;
	amsdu_pad = 0;

	while (msdu_orig) {

		/* TODO: intra AMSDU padding - do we need it ??? */

		msdu = clone_nbuf_fn(msdu_orig);
		if (!msdu)
			goto mpdu_stitch_fail;

		if (is_first_frag) {
			is_first_frag = 0;
			head_frag_list_cloned = msdu;
		} else {

			/* Maintain the linking of the cloned MSDUS */
			qdf_nbuf_set_next_ext(prev_buf, msdu);

			/* Reload the hdr ptr only on non-first MSDUs */
			rx_desc = htt_rx_desc(msdu_orig);
			hdr_desc = &rx_desc->rx_hdr_status[0];

		}

		/* Copy this buffers MSDU related status into the prev buffer */
		dest = qdf_nbuf_put_tail(prev_buf, msdu_llc_len + amsdu_pad);
		dest += amsdu_pad;
		qdf_mem_copy(dest, hdr_desc, msdu_llc_len);

		/* Push the MSDU buffer beyond the decap header */
		qdf_nbuf_pull_head(msdu, decap_hdr_pull_bytes);
		frag_list_sum_len +=
			msdu_llc_len + qdf_nbuf_len(msdu) + amsdu_pad;

		/* Set up intra-AMSDU pad to be added to start of next buffer -
		 * AMSDU pad is 4 byte pad on AMSDU subframe */
		amsdu_pad = (msdu_llc_len + qdf_nbuf_len(msdu)) & 0x3;
		amsdu_pad = amsdu_pad ? (4 - amsdu_pad) : 0;

		/* TODO FIXME How do we handle MSDUs that have fraglist - Should
		 * probably iterate all the frags cloning them along the way and
		 * and also updating the prev_buf pointer
		 */

		/* Move to the next */
		prev_buf = msdu;
		msdu_orig = qdf_nbuf_next(msdu_orig);

	}

	/* TODO: Convert this to suitable qdf routines */
	qdf_nbuf_append_ext_list(mpdu_buf, head_frag_list_cloned,
				 frag_list_sum_len);

mpdu_stitch_done:
	/* Check if this buffer contains the PPDU end status for TSF */
	if (rx_desc->attention.last_mpdu)
#ifdef HELIUMPLUS
		rx_status->rs_tstamp.tsf =
			rx_desc->ppdu_end.rx_pkt_end.phy_timestamp_1_lower_32;
#else
		rx_status->rs_tstamp.tsf = rx_desc->ppdu_end.tsf_timestamp;
#endif
	/* All the nbufs have been linked into the ext list and
	   then unlink the nbuf list */
	if (clone_not_reqd) {
		msdu = head_msdu;
		while (msdu) {
			msdu_orig = msdu;
			msdu = qdf_nbuf_next(msdu);
			qdf_nbuf_set_next(msdu_orig, NULL);
		}
	}

	return mpdu_buf;

mpdu_stitch_fail:
	/* Free these alloced buffers and the orig buffers in non-clone case */
	if (!clone_not_reqd) {
		/* Free the head buffer */
		if (mpdu_buf)
			qdf_nbuf_free(mpdu_buf);

		/* Free the partial list */
		while (head_frag_list_cloned) {
			msdu = head_frag_list_cloned;
			head_frag_list_cloned =
				qdf_nbuf_next_ext(head_frag_list_cloned);
			qdf_nbuf_free(msdu);
		}
	} else {
		/* Free the alloced head buffer */
		if (decap_format != HW_RX_DECAP_FORMAT_RAW)
			if (mpdu_buf)
				qdf_nbuf_free(mpdu_buf);

		/* Free the orig buffers */
		msdu = head_msdu;
		while (msdu) {
			msdu_orig = msdu;
			msdu = qdf_nbuf_next(msdu);
			qdf_nbuf_free(msdu_orig);
		}
	}

	return NULL;
}

int16_t htt_rx_mpdu_desc_rssi_dbm(htt_pdev_handle pdev, void *mpdu_desc)
{
	/*
	 * Currently the RSSI is provided only as a field in the
	 * HTT_T2H_RX_IND message, rather than in each rx descriptor.
	 */
	return HTT_RSSI_INVALID;
}

/*
 * htt_rx_amsdu_pop -
 * global function pointer that is programmed during attach to point
 * to either htt_rx_amsdu_pop_ll or htt_rx_amsdu_rx_in_order_pop_ll.
 */
int (*htt_rx_amsdu_pop)(htt_pdev_handle pdev,
			qdf_nbuf_t rx_ind_msg,
			qdf_nbuf_t *head_msdu, qdf_nbuf_t *tail_msdu);

/*
 * htt_rx_frag_pop -
 * global function pointer that is programmed during attach to point
 * to either htt_rx_amsdu_pop_ll
 */
int (*htt_rx_frag_pop)(htt_pdev_handle pdev,
		       qdf_nbuf_t rx_ind_msg,
		       qdf_nbuf_t *head_msdu, qdf_nbuf_t *tail_msdu);

int
(*htt_rx_offload_msdu_pop)(htt_pdev_handle pdev,
			   qdf_nbuf_t offload_deliver_msg,
			   int *vdev_id,
			   int *peer_id,
			   int *tid,
			   uint8_t *fw_desc,
			   qdf_nbuf_t *head_buf, qdf_nbuf_t *tail_buf);

void * (*htt_rx_mpdu_desc_list_next)(htt_pdev_handle pdev,
				    qdf_nbuf_t rx_ind_msg);

bool (*htt_rx_mpdu_desc_retry)(
    htt_pdev_handle pdev, void *mpdu_desc);

uint16_t (*htt_rx_mpdu_desc_seq_num)(htt_pdev_handle pdev, void *mpdu_desc);

void (*htt_rx_mpdu_desc_pn)(htt_pdev_handle pdev,
			    void *mpdu_desc,
			    union htt_rx_pn_t *pn, int pn_len_bits);

uint8_t (*htt_rx_mpdu_desc_tid)(
    htt_pdev_handle pdev, void *mpdu_desc);

bool (*htt_rx_msdu_desc_completes_mpdu)(htt_pdev_handle pdev, void *msdu_desc);

bool (*htt_rx_msdu_first_msdu_flag)(htt_pdev_handle pdev, void *msdu_desc);

int (*htt_rx_msdu_has_wlan_mcast_flag)(htt_pdev_handle pdev, void *msdu_desc);

bool (*htt_rx_msdu_is_wlan_mcast)(htt_pdev_handle pdev, void *msdu_desc);

int (*htt_rx_msdu_is_frag)(htt_pdev_handle pdev, void *msdu_desc);

void * (*htt_rx_msdu_desc_retrieve)(htt_pdev_handle pdev, qdf_nbuf_t msdu);

bool (*htt_rx_mpdu_is_encrypted)(htt_pdev_handle pdev, void *mpdu_desc);

bool (*htt_rx_msdu_desc_key_id)(htt_pdev_handle pdev,
				void *mpdu_desc, uint8_t *key_id);

void *htt_rx_mpdu_desc_list_next_ll(htt_pdev_handle pdev, qdf_nbuf_t rx_ind_msg)
{
	int idx = pdev->rx_ring.sw_rd_idx.msdu_desc;
	qdf_nbuf_t netbuf = pdev->rx_ring.buf.netbufs_ring[idx];
	pdev->rx_ring.sw_rd_idx.msdu_desc = pdev->rx_ring.sw_rd_idx.msdu_payld;
	return (void *)htt_rx_desc(netbuf);
}

bool (*htt_rx_msdu_chan_info_present)(
	htt_pdev_handle pdev,
	void *mpdu_desc);

bool (*htt_rx_msdu_center_freq)(
	htt_pdev_handle pdev,
	struct ol_txrx_peer_t *peer,
	void *mpdu_desc,
	uint16_t *primary_chan_center_freq_mhz,
	uint16_t *contig_chan1_center_freq_mhz,
	uint16_t *contig_chan2_center_freq_mhz,
	uint8_t *phy_mode);

void *htt_rx_in_ord_mpdu_desc_list_next_ll(htt_pdev_handle pdev,
					   qdf_nbuf_t netbuf)
{
	return (void *)htt_rx_desc(netbuf);
}

void *htt_rx_msdu_desc_retrieve_ll(htt_pdev_handle pdev, qdf_nbuf_t msdu)
{
	return htt_rx_desc(msdu);
}

bool htt_rx_mpdu_is_encrypted_ll(htt_pdev_handle pdev, void *mpdu_desc)
{
	struct htt_host_rx_desc_base *rx_desc =
		(struct htt_host_rx_desc_base *)mpdu_desc;

	return (((*((uint32_t *) &rx_desc->mpdu_start)) &
		 RX_MPDU_START_0_ENCRYPTED_MASK) >>
		RX_MPDU_START_0_ENCRYPTED_LSB) ? true : false;
}

bool htt_rx_msdu_chan_info_present_ll(htt_pdev_handle pdev, void *mpdu_desc)
{
	return false;
}

bool htt_rx_msdu_center_freq_ll(htt_pdev_handle pdev,
	struct ol_txrx_peer_t *peer,
	void *mpdu_desc,
	uint16_t *primary_chan_center_freq_mhz,
	uint16_t *contig_chan1_center_freq_mhz,
	uint16_t *contig_chan2_center_freq_mhz,
	uint8_t *phy_mode)
{
	if (primary_chan_center_freq_mhz)
		*primary_chan_center_freq_mhz = 0;
	if (contig_chan1_center_freq_mhz)
		*contig_chan1_center_freq_mhz = 0;
	if (contig_chan2_center_freq_mhz)
		*contig_chan2_center_freq_mhz = 0;
	if (phy_mode)
		*phy_mode = 0;
	return false;
}

bool
htt_rx_msdu_desc_key_id_ll(htt_pdev_handle pdev, void *mpdu_desc,
			   uint8_t *key_id)
{
	struct htt_host_rx_desc_base *rx_desc = (struct htt_host_rx_desc_base *)
						mpdu_desc;

	if (!htt_rx_msdu_first_msdu_flag_ll(pdev, mpdu_desc))
		return false;

	*key_id = ((*(((uint32_t *) &rx_desc->msdu_end) + 1)) &
		   (RX_MSDU_END_1_KEY_ID_OCT_MASK >>
		    RX_MSDU_END_1_KEY_ID_OCT_LSB));

	return true;
}

void htt_rx_desc_frame_free(htt_pdev_handle htt_pdev, qdf_nbuf_t msdu)
{
	qdf_nbuf_free(msdu);
}

void htt_rx_msdu_desc_free(htt_pdev_handle htt_pdev, qdf_nbuf_t msdu)
{
	/*
	 * The rx descriptor is in the same buffer as the rx MSDU payload,
	 * and does not need to be freed separately.
	 */
}

void htt_rx_msdu_buff_replenish(htt_pdev_handle pdev)
{
	if (qdf_atomic_dec_and_test(&pdev->rx_ring.refill_ref_cnt)) {
		int num_to_fill;
		num_to_fill = pdev->rx_ring.fill_level -
			pdev->rx_ring.fill_cnt;

		htt_rx_ring_fill_n(pdev,
				   num_to_fill /* okay if <= 0 */);
	}
	qdf_atomic_inc(&pdev->rx_ring.refill_ref_cnt);
}

#define AR600P_ASSEMBLE_HW_RATECODE(_rate, _nss, _pream)     \
	(((_pream) << 6) | ((_nss) << 4) | (_rate))

enum AR600P_HW_RATECODE_PREAM_TYPE {
	AR600P_HW_RATECODE_PREAM_OFDM,
	AR600P_HW_RATECODE_PREAM_CCK,
	AR600P_HW_RATECODE_PREAM_HT,
	AR600P_HW_RATECODE_PREAM_VHT,
};

/*--- RX In Order Hash Code --------------------------------------------------*/

/* Initializes the circular linked list */
static inline void htt_list_init(struct htt_list_node *head)
{
	head->prev = head;
	head->next = head;
}

/* Adds entry to the end of the linked list */
static inline void htt_list_add_tail(struct htt_list_node *head,
				     struct htt_list_node *node)
{
	head->prev->next = node;
	node->prev = head->prev;
	node->next = head;
	head->prev = node;
}

/* Removes the entry corresponding to the input node from the linked list */
static inline void htt_list_remove(struct htt_list_node *node)
{
	node->prev->next = node->next;
	node->next->prev = node->prev;
}

/* Helper macro to iterate through the linked list */
#define HTT_LIST_ITER_FWD(iter, head) for (iter = (head)->next;		\
					   (iter) != (head);		\
					   (iter) = (iter)->next)	\

#ifdef RX_HASH_DEBUG
/* Hash cookie related macros */
#define HTT_RX_HASH_COOKIE 0xDEED

#define HTT_RX_HASH_COOKIE_SET(hash_element) \
	((hash_element)->cookie = HTT_RX_HASH_COOKIE)

#define HTT_RX_HASH_COOKIE_CHECK(hash_element) \
	HTT_ASSERT_ALWAYS((hash_element)->cookie == HTT_RX_HASH_COOKIE)

/* Hash count related macros */
#define HTT_RX_HASH_COUNT_INCR(hash_bucket) \
	((hash_bucket).count++)

#define HTT_RX_HASH_COUNT_DECR(hash_bucket) \
	((hash_bucket).count--)

#define HTT_RX_HASH_COUNT_RESET(hash_bucket) ((hash_bucket).count = 0)

#define HTT_RX_HASH_COUNT_PRINT(hash_bucket) \
	RX_HASH_LOG(qdf_print(" count %d\n", (hash_bucket).count))
#else                           /* RX_HASH_DEBUG */
/* Hash cookie related macros */
#define HTT_RX_HASH_COOKIE_SET(hash_element)    /* no-op */
#define HTT_RX_HASH_COOKIE_CHECK(hash_element)  /* no-op */
/* Hash count related macros */
#define HTT_RX_HASH_COUNT_INCR(hash_bucket)     /* no-op */
#define HTT_RX_HASH_COUNT_DECR(hash_bucket)     /* no-op */
#define HTT_RX_HASH_COUNT_PRINT(hash_bucket)    /* no-op */
#define HTT_RX_HASH_COUNT_RESET(hash_bucket)    /* no-op */
#endif /* RX_HASH_DEBUG */

/* Inserts the given "physical address - network buffer" pair into the
   hash table for the given pdev. This function will do the following:
   1. Determine which bucket to insert the pair into
   2. First try to allocate the hash entry for this pair from the pre-allocated
      entries list
   3. If there are no more entries in the pre-allocated entries list, allocate
      the hash entry from the hash memory pool
   Note: this function is not thread-safe
   Returns 0 - success, 1 - failure */
int
htt_rx_hash_list_insert(struct htt_pdev_t *pdev, uint32_t paddr,
			qdf_nbuf_t netbuf)
{
	int i;
	struct htt_rx_hash_entry *hash_element = NULL;

	i = RX_HASH_FUNCTION(paddr);

	/* Check if there are any entries in the pre-allocated free list */
	if (pdev->rx_ring.hash_table[i].freepool.next !=
	    &pdev->rx_ring.hash_table[i].freepool) {

		hash_element =
			(struct htt_rx_hash_entry *)(
				(char *)
				pdev->rx_ring.hash_table[i].freepool.next -
				pdev->rx_ring.listnode_offset);
		if (qdf_unlikely(NULL == hash_element)) {
			HTT_ASSERT_ALWAYS(0);
			return 1;
		}

		htt_list_remove(pdev->rx_ring.hash_table[i].freepool.next);
	} else {
		hash_element = qdf_mem_malloc(sizeof(struct htt_rx_hash_entry));
		if (qdf_unlikely(NULL == hash_element)) {
			HTT_ASSERT_ALWAYS(0);
			return 1;
		}
		hash_element->fromlist = 0;
	}

	hash_element->netbuf = netbuf;
	hash_element->paddr = paddr;
	HTT_RX_HASH_COOKIE_SET(hash_element);

	htt_list_add_tail(&pdev->rx_ring.hash_table[i].listhead,
			  &hash_element->listnode);

	RX_HASH_LOG(qdf_print("rx hash: %s: paddr 0x%x netbuf %p bucket %d\n",
			      __func__, paddr, netbuf, (int)i));

	HTT_RX_HASH_COUNT_INCR(pdev->rx_ring.hash_table[i]);
	HTT_RX_HASH_COUNT_PRINT(pdev->rx_ring.hash_table[i]);

	return 0;
}

/* Given a physical address this function will find the corresponding network
   buffer from the hash table.
   Note: this function is not thread-safe */
qdf_nbuf_t htt_rx_hash_list_lookup(struct htt_pdev_t *pdev, uint32_t paddr)
{
	uint32_t i;
	struct htt_list_node *list_iter = NULL;
	qdf_nbuf_t netbuf = NULL;
	struct htt_rx_hash_entry *hash_entry;

	i = RX_HASH_FUNCTION(paddr);

	HTT_LIST_ITER_FWD(list_iter, &pdev->rx_ring.hash_table[i].listhead) {
		hash_entry = (struct htt_rx_hash_entry *)
			     ((char *)list_iter -
			      pdev->rx_ring.listnode_offset);

		HTT_RX_HASH_COOKIE_CHECK(hash_entry);

		if (hash_entry->paddr == paddr) {
			/* Found the entry corresponding to paddr */
			netbuf = hash_entry->netbuf;
			htt_list_remove(&hash_entry->listnode);
			HTT_RX_HASH_COUNT_DECR(pdev->rx_ring.hash_table[i]);
			/* if the rx entry is from the pre-allocated list,
			   return it */
			if (hash_entry->fromlist)
				htt_list_add_tail(&pdev->rx_ring.hash_table[i].
						  freepool,
						  &hash_entry->listnode);
			else
				qdf_mem_free(hash_entry);

			htt_rx_dbg_rxbuf_reset(pdev, netbuf);
			break;
		}
	}

	RX_HASH_LOG(qdf_print("rx hash: %s: paddr 0x%x, netbuf %p, bucket %d\n",
			      __func__, paddr, netbuf, (int)i));
	HTT_RX_HASH_COUNT_PRINT(pdev->rx_ring.hash_table[i]);

	if (netbuf == NULL) {
		qdf_print("rx hash: %s: no entry found for 0x%x!!!\n",
			  __func__, paddr);
		HTT_ASSERT_ALWAYS(0);
	}

	return netbuf;
}

/* Initialization function of the rx buffer hash table. This function will
   allocate a hash table of a certain pre-determined size and initialize all
   the elements */
int htt_rx_hash_init(struct htt_pdev_t *pdev)
{
	int i, j;

	HTT_ASSERT2(QDF_IS_PWR2(RX_NUM_HASH_BUCKETS));

	pdev->rx_ring.hash_table =
		qdf_mem_malloc(RX_NUM_HASH_BUCKETS *
			       sizeof(struct htt_rx_hash_bucket));

	if (NULL == pdev->rx_ring.hash_table) {
		qdf_print("rx hash table allocation failed!\n");
		return 1;
	}

	for (i = 0; i < RX_NUM_HASH_BUCKETS; i++) {
		HTT_RX_HASH_COUNT_RESET(pdev->rx_ring.hash_table[i]);

		/* initialize the hash table buckets */
		htt_list_init(&pdev->rx_ring.hash_table[i].listhead);

		/* initialize the hash table free pool per bucket */
		htt_list_init(&pdev->rx_ring.hash_table[i].freepool);

		/* pre-allocate a pool of entries for this bucket */
		pdev->rx_ring.hash_table[i].entries =
			qdf_mem_malloc(RX_ENTRIES_SIZE *
				       sizeof(struct htt_rx_hash_entry));

		if (NULL == pdev->rx_ring.hash_table[i].entries) {
			qdf_print("rx hash bucket %d entries alloc failed\n",
				(int)i);
			while (i) {
				i--;
				qdf_mem_free(pdev->rx_ring.hash_table[i].
					     entries);
			}
			qdf_mem_free(pdev->rx_ring.hash_table);
			pdev->rx_ring.hash_table = NULL;
			return 1;
		}

		/* initialize the free list with pre-allocated entries */
		for (j = 0; j < RX_ENTRIES_SIZE; j++) {
			pdev->rx_ring.hash_table[i].entries[j].fromlist = 1;
			htt_list_add_tail(&pdev->rx_ring.hash_table[i].freepool,
					  &pdev->rx_ring.hash_table[i].
					  entries[j].listnode);
		}
	}

	pdev->rx_ring.listnode_offset =
		qdf_offsetof(struct htt_rx_hash_entry, listnode);

	return 0;
}

void htt_rx_hash_dump_table(struct htt_pdev_t *pdev)
{
	uint32_t i;
	struct htt_rx_hash_entry *hash_entry;
	struct htt_list_node *list_iter = NULL;

	for (i = 0; i < RX_NUM_HASH_BUCKETS; i++) {
		HTT_LIST_ITER_FWD(list_iter,
				  &pdev->rx_ring.hash_table[i].listhead) {
			hash_entry =
				(struct htt_rx_hash_entry *)((char *)list_iter -
							     pdev->rx_ring.
							     listnode_offset);
			qdf_print("hash_table[%d]: netbuf %p paddr 0x%x\n", i,
				  hash_entry->netbuf, hash_entry->paddr);
		}
	}
}

/*--- RX In Order Hash Code --------------------------------------------------*/

/* move the function to the end of file
 * to omit ll/hl pre-declaration
 */
int htt_rx_attach(struct htt_pdev_t *pdev)
{
	qdf_dma_addr_t paddr;
	uint32_t ring_elem_size = sizeof(qdf_dma_addr_t);

	pdev->rx_ring.size = htt_rx_ring_size(pdev);
	HTT_ASSERT2(QDF_IS_PWR2(pdev->rx_ring.size));
	pdev->rx_ring.size_mask = pdev->rx_ring.size - 1;

	/*
	* Set the initial value for the level to which the rx ring
	* should be filled, based on the max throughput and the worst
	* likely latency for the host to fill the rx ring.
	* In theory, this fill level can be dynamically adjusted from
	* the initial value set here to reflect the actual host latency
	* rather than a conservative assumption.
	*/
	pdev->rx_ring.fill_level = htt_rx_ring_fill_level(pdev);

	if (pdev->cfg.is_full_reorder_offload) {
		if (htt_rx_hash_init(pdev))
			goto fail1;

		/* allocate the target index */
		pdev->rx_ring.target_idx.vaddr =
			 qdf_mem_alloc_consistent(pdev->osdev, pdev->osdev->dev,
				 sizeof(uint32_t),
				 &paddr);

		if (!pdev->rx_ring.target_idx.vaddr)
			goto fail1;

		pdev->rx_ring.target_idx.paddr = paddr;
		*pdev->rx_ring.target_idx.vaddr = 0;
	} else {
		pdev->rx_ring.buf.netbufs_ring =
			qdf_mem_malloc(pdev->rx_ring.size * sizeof(qdf_nbuf_t));
		if (!pdev->rx_ring.buf.netbufs_ring)
			goto fail1;

		pdev->rx_ring.sw_rd_idx.msdu_payld = 0;
		pdev->rx_ring.sw_rd_idx.msdu_desc = 0;
	}

	pdev->rx_ring.buf.paddrs_ring =
		qdf_mem_alloc_consistent(
			pdev->osdev, pdev->osdev->dev,
			 pdev->rx_ring.size * ring_elem_size,
			 &paddr);
	if (!pdev->rx_ring.buf.paddrs_ring)
		goto fail2;

	pdev->rx_ring.base_paddr = paddr;
	pdev->rx_ring.alloc_idx.vaddr =
		 qdf_mem_alloc_consistent(
			pdev->osdev, pdev->osdev->dev,
			 sizeof(uint32_t), &paddr);

	if (!pdev->rx_ring.alloc_idx.vaddr)
		goto fail3;

	pdev->rx_ring.alloc_idx.paddr = paddr;
	*pdev->rx_ring.alloc_idx.vaddr = 0;

	/*
	* Initialize the Rx refill reference counter to be one so that
	* only one thread is allowed to refill the Rx ring.
	*/
	qdf_atomic_init(&pdev->rx_ring.refill_ref_cnt);
	qdf_atomic_inc(&pdev->rx_ring.refill_ref_cnt);

	/* Initialize the Rx refill retry timer */
	qdf_timer_init(pdev->osdev,
		 &pdev->rx_ring.refill_retry_timer,
		 htt_rx_ring_refill_retry, (void *)pdev,
		 QDF_TIMER_TYPE_SW);

	pdev->rx_ring.fill_cnt = 0;
#ifdef DEBUG_DMA_DONE
	pdev->rx_ring.dbg_ring_idx = 0;
	pdev->rx_ring.dbg_refill_cnt = 0;
	pdev->rx_ring.dbg_sync_success = 0;
#endif
#ifdef HTT_RX_RESTORE
	pdev->rx_ring.rx_reset = 0;
	pdev->rx_ring.htt_rx_restore = 0;
#endif
	htt_rx_dbg_rxbuf_init(pdev);
	htt_rx_ring_fill_n(pdev, pdev->rx_ring.fill_level);

	if (pdev->cfg.is_full_reorder_offload) {
		qdf_print("HTT: full reorder offload enabled\n");
		htt_rx_amsdu_pop = htt_rx_amsdu_rx_in_order_pop_ll;
		htt_rx_frag_pop = htt_rx_amsdu_rx_in_order_pop_ll;
		htt_rx_mpdu_desc_list_next =
			 htt_rx_in_ord_mpdu_desc_list_next_ll;
	} else {
		htt_rx_amsdu_pop = htt_rx_amsdu_pop_ll;
		htt_rx_frag_pop = htt_rx_amsdu_pop_ll;
		htt_rx_mpdu_desc_list_next = htt_rx_mpdu_desc_list_next_ll;
	}

	if (cds_get_conparam() == QDF_GLOBAL_MONITOR_MODE)
		htt_rx_amsdu_pop = htt_rx_mon_amsdu_rx_in_order_pop_ll;

	htt_rx_offload_msdu_pop = htt_rx_offload_msdu_pop_ll;
	htt_rx_mpdu_desc_retry = htt_rx_mpdu_desc_retry_ll;
	htt_rx_mpdu_desc_seq_num = htt_rx_mpdu_desc_seq_num_ll;
	htt_rx_mpdu_desc_pn = htt_rx_mpdu_desc_pn_ll;
	htt_rx_mpdu_desc_tid = htt_rx_mpdu_desc_tid_ll;
	htt_rx_msdu_desc_completes_mpdu = htt_rx_msdu_desc_completes_mpdu_ll;
	htt_rx_msdu_first_msdu_flag = htt_rx_msdu_first_msdu_flag_ll;
	htt_rx_msdu_has_wlan_mcast_flag = htt_rx_msdu_has_wlan_mcast_flag_ll;
	htt_rx_msdu_is_wlan_mcast = htt_rx_msdu_is_wlan_mcast_ll;
	htt_rx_msdu_is_frag = htt_rx_msdu_is_frag_ll;
	htt_rx_msdu_desc_retrieve = htt_rx_msdu_desc_retrieve_ll;
	htt_rx_mpdu_is_encrypted = htt_rx_mpdu_is_encrypted_ll;
	htt_rx_msdu_desc_key_id = htt_rx_msdu_desc_key_id_ll;
	htt_rx_msdu_chan_info_present = htt_rx_msdu_chan_info_present_ll;
	htt_rx_msdu_center_freq = htt_rx_msdu_center_freq_ll;

	return 0;               /* success */

fail3:
	qdf_mem_free_consistent(pdev->osdev, pdev->osdev->dev,
				   pdev->rx_ring.size * sizeof(qdf_dma_addr_t),
				   pdev->rx_ring.buf.paddrs_ring,
				   pdev->rx_ring.base_paddr,
				   qdf_get_dma_mem_context((&pdev->rx_ring.buf),
							   memctx));

fail2:
	if (pdev->cfg.is_full_reorder_offload) {
		qdf_mem_free_consistent(pdev->osdev, pdev->osdev->dev,
					   sizeof(uint32_t),
					   pdev->rx_ring.target_idx.vaddr,
					   pdev->rx_ring.target_idx.paddr,
					   qdf_get_dma_mem_context((&pdev->
								    rx_ring.
								    target_idx),
								   memctx));
		htt_rx_hash_deinit(pdev);
	} else {
		qdf_mem_free(pdev->rx_ring.buf.netbufs_ring);
	}

fail1:
	return 1;               /* failure */
}

#ifdef IPA_OFFLOAD
#ifdef QCA_WIFI_3_0
/**
 * htt_rx_ipa_uc_alloc_wdi2_rsc() - Allocate WDI2.0 resources
 * @pdev: htt context
 * @rx_ind_ring_elements: rx ring elements
 *
 * Return: 0 success
 */
int htt_rx_ipa_uc_alloc_wdi2_rsc(struct htt_pdev_t *pdev,
			 unsigned int rx_ind_ring_elements)
{
	/* Allocate RX2 indication ring */
	/* RX2 IND ring element
	 *   4bytes: pointer
	 *   2bytes: VDEV ID
	 *   2bytes: length */
	pdev->ipa_uc_rx_rsc.rx2_ind_ring_base.vaddr =
		qdf_mem_alloc_consistent(
			pdev->osdev, pdev->osdev->dev,
			rx_ind_ring_elements *
			sizeof(qdf_dma_addr_t),
			&pdev->ipa_uc_rx_rsc.rx2_ind_ring_base.paddr);
	if (!pdev->ipa_uc_rx_rsc.rx2_ind_ring_base.vaddr) {
		qdf_print("%s: RX IND RING alloc fail", __func__);
		return -ENOBUFS;
	}

	/* RX indication ring size, by bytes */
	pdev->ipa_uc_rx_rsc.rx2_ind_ring_size =
		rx_ind_ring_elements * sizeof(qdf_dma_addr_t);
	qdf_mem_zero(pdev->ipa_uc_rx_rsc.rx2_ind_ring_base.vaddr,
		pdev->ipa_uc_rx_rsc.rx2_ind_ring_size);

	/* Allocate RX process done index */
	pdev->ipa_uc_rx_rsc.rx2_ipa_prc_done_idx.vaddr =
		qdf_mem_alloc_consistent(
			pdev->osdev, pdev->osdev->dev, 4,
			&pdev->ipa_uc_rx_rsc.rx2_ipa_prc_done_idx.paddr);
	if (!pdev->ipa_uc_rx_rsc.rx2_ipa_prc_done_idx.vaddr) {
		qdf_print("%s: RX PROC DONE IND alloc fail", __func__);
		qdf_mem_free_consistent(
			pdev->osdev, pdev->osdev->dev,
			pdev->ipa_uc_rx_rsc.rx2_ind_ring_size,
			pdev->ipa_uc_rx_rsc.rx2_ind_ring_base.vaddr,
			pdev->ipa_uc_rx_rsc.rx2_ind_ring_base.paddr,
			qdf_get_dma_mem_context((&pdev->ipa_uc_rx_rsc.
						 rx2_ind_ring_base),
						memctx));
		return -ENOBUFS;
	}
	qdf_mem_zero(pdev->ipa_uc_rx_rsc.rx2_ipa_prc_done_idx.vaddr, 4);
	return 0;
}
#else
int htt_rx_ipa_uc_alloc_wdi2_rsc(struct htt_pdev_t *pdev,
			 unsigned int rx_ind_ring_elements)
{
	return 0;
}
#endif

/**
 * htt_rx_ipa_uc_attach() - attach htt ipa uc rx resource
 * @pdev: htt context
 * @rx_ind_ring_size: rx ring size
 *
 * Return: 0 success
 */
int htt_rx_ipa_uc_attach(struct htt_pdev_t *pdev,
			 unsigned int rx_ind_ring_elements)
{
	int ret = 0;
	/* Allocate RX indication ring */
	/* RX IND ring element
	 *   4bytes: pointer
	 *   2bytes: VDEV ID
	 *   2bytes: length */
	pdev->ipa_uc_rx_rsc.rx_ind_ring_base.vaddr =
		qdf_mem_alloc_consistent(
			pdev->osdev,
			pdev->osdev->dev,
			rx_ind_ring_elements *
			sizeof(struct ipa_uc_rx_ring_elem_t),
			&pdev->ipa_uc_rx_rsc.rx_ind_ring_base.paddr);
	if (!pdev->ipa_uc_rx_rsc.rx_ind_ring_base.vaddr) {
		qdf_print("%s: RX IND RING alloc fail", __func__);
		return -ENOBUFS;
	}

	/* RX indication ring size, by bytes */
	pdev->ipa_uc_rx_rsc.rx_ind_ring_size =
		rx_ind_ring_elements * sizeof(struct ipa_uc_rx_ring_elem_t);
	qdf_mem_zero(pdev->ipa_uc_rx_rsc.rx_ind_ring_base.vaddr,
		pdev->ipa_uc_rx_rsc.rx_ind_ring_size);

	/* Allocate RX process done index */
	pdev->ipa_uc_rx_rsc.rx_ipa_prc_done_idx.vaddr =
		qdf_mem_alloc_consistent(
			pdev->osdev, pdev->osdev->dev, 4,
			&pdev->ipa_uc_rx_rsc.rx_ipa_prc_done_idx.paddr);
	if (!pdev->ipa_uc_rx_rsc.rx_ipa_prc_done_idx.vaddr) {
		qdf_print("%s: RX PROC DONE IND alloc fail", __func__);
		qdf_mem_free_consistent(
			pdev->osdev, pdev->osdev->dev,
			pdev->ipa_uc_rx_rsc.rx_ind_ring_size,
			pdev->ipa_uc_rx_rsc.rx_ind_ring_base.vaddr,
			pdev->ipa_uc_rx_rsc.rx_ind_ring_base.paddr,
			qdf_get_dma_mem_context((&pdev->ipa_uc_rx_rsc.
						 rx_ind_ring_base),
						memctx));
		return -ENOBUFS;
	}
	qdf_mem_zero(pdev->ipa_uc_rx_rsc.rx_ipa_prc_done_idx.vaddr, 4);

	ret = htt_rx_ipa_uc_alloc_wdi2_rsc(pdev, rx_ind_ring_elements);
	return ret;
}

#ifdef QCA_WIFI_3_0
/**
 * htt_rx_ipa_uc_free_wdi2_rsc() - Free WDI2.0 resources
 * @pdev: htt context
 *
 * Return: None
 */
void htt_rx_ipa_uc_free_wdi2_rsc(struct htt_pdev_t *pdev)
{
	if (pdev->ipa_uc_rx_rsc.rx2_ind_ring_base.vaddr) {
		qdf_mem_free_consistent(
			pdev->osdev, pdev->osdev->dev,
			pdev->ipa_uc_rx_rsc.rx2_ind_ring_size,
			pdev->ipa_uc_rx_rsc.rx2_ind_ring_base.vaddr,
			pdev->ipa_uc_rx_rsc.rx2_ind_ring_base.paddr,
			qdf_get_dma_mem_context((&pdev->ipa_uc_rx_rsc.
						 rx2_ind_ring_base),
						memctx));
	}

	if (pdev->ipa_uc_rx_rsc.rx2_ipa_prc_done_idx.vaddr) {
		qdf_mem_free_consistent(
			pdev->osdev, pdev->osdev->dev,
			4,
			pdev->ipa_uc_rx_rsc.
			rx_ipa_prc_done_idx.vaddr,
			pdev->ipa_uc_rx_rsc.rx2_ipa_prc_done_idx.paddr,
			qdf_get_dma_mem_context((&pdev->ipa_uc_rx_rsc.
						 rx_ipa_prc_done_idx),
						memctx));
	}
}
#else
void htt_rx_ipa_uc_free_wdi2_rsc(struct htt_pdev_t *pdev)
{
	return;
}
#endif

int htt_rx_ipa_uc_detach(struct htt_pdev_t *pdev)
{
	if (pdev->ipa_uc_rx_rsc.rx_ind_ring_base.vaddr) {
		qdf_mem_free_consistent(
			pdev->osdev, pdev->osdev->dev,
			pdev->ipa_uc_rx_rsc.rx_ind_ring_size,
			pdev->ipa_uc_rx_rsc.rx_ind_ring_base.vaddr,
			pdev->ipa_uc_rx_rsc.rx_ind_ring_base.paddr,
			qdf_get_dma_mem_context((&pdev->ipa_uc_rx_rsc.
						 rx_ind_ring_base),
						memctx));
	}

	if (pdev->ipa_uc_rx_rsc.rx_ipa_prc_done_idx.vaddr) {
		qdf_mem_free_consistent(
			pdev->osdev, pdev->osdev->dev,
			4,
			pdev->ipa_uc_rx_rsc.
			rx_ipa_prc_done_idx.vaddr,
			pdev->ipa_uc_rx_rsc.rx_ipa_prc_done_idx.paddr,
			qdf_get_dma_mem_context((&pdev->ipa_uc_rx_rsc.
						 rx2_ipa_prc_done_idx),
						memctx));
	}

	htt_rx_ipa_uc_free_wdi2_rsc(pdev);
	return 0;
}
#endif /* IPA_OFFLOAD */
