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

#include <osdep.h>
#include "a_types.h"
#include <athdefs.h>
#include "osapi_linux.h"
#include "hif.h"
#include "hif_io32.h"
#include "ce_api.h"
#include "ce_main.h"
#include "ce_internal.h"
#include "ce_reg.h"
#include "cdf_lock.h"
#include "regtable.h"
#include "epping_main.h"
#include "hif_main.h"
#include "hif_debug.h"
#include "cds_concurrency.h"

#ifdef IPA_OFFLOAD
#ifdef QCA_WIFI_3_0
#define CE_IPA_RING_INIT(ce_desc)                       \
	do {                                            \
		ce_desc->gather = 0;                    \
		ce_desc->enable_11h = 0;                \
		ce_desc->meta_data_low = 0;             \
		ce_desc->packet_result_offset = 64;     \
		ce_desc->toeplitz_hash_enable = 0;      \
		ce_desc->addr_y_search_disable = 0;     \
		ce_desc->addr_x_search_disable = 0;     \
		ce_desc->misc_int_disable = 0;          \
		ce_desc->target_int_disable = 0;        \
		ce_desc->host_int_disable = 0;          \
		ce_desc->dest_byte_swap = 0;            \
		ce_desc->byte_swap = 0;                 \
		ce_desc->type = 2;                      \
		ce_desc->tx_classify = 1;               \
		ce_desc->buffer_addr_hi = 0;            \
		ce_desc->meta_data = 0;                 \
		ce_desc->nbytes = 128;                  \
	} while (0)
#else
#define CE_IPA_RING_INIT(ce_desc)                       \
	do {                                            \
		ce_desc->byte_swap = 0;                 \
		ce_desc->nbytes = 60;                   \
		ce_desc->gather = 0;                    \
	} while (0)
#endif /* QCA_WIFI_3_0 */
#endif /* IPA_OFFLOAD */

static int war1_allow_sleep;
/* io32 write workaround */
static int hif_ce_war1;

/*
 * Support for Copy Engine hardware, which is mainly used for
 * communication between Host and Target over a PCIe interconnect.
 */

/*
 * A single CopyEngine (CE) comprises two "rings":
 *   a source ring
 *   a destination ring
 *
 * Each ring consists of a number of descriptors which specify
 * an address, length, and meta-data.
 *
 * Typically, one side of the PCIe interconnect (Host or Target)
 * controls one ring and the other side controls the other ring.
 * The source side chooses when to initiate a transfer and it
 * chooses what to send (buffer address, length). The destination
 * side keeps a supply of "anonymous receive buffers" available and
 * it handles incoming data as it arrives (when the destination
 * recieves an interrupt).
 *
 * The sender may send a simple buffer (address/length) or it may
 * send a small list of buffers.  When a small list is sent, hardware
 * "gathers" these and they end up in a single destination buffer
 * with a single interrupt.
 *
 * There are several "contexts" managed by this layer -- more, it
 * may seem -- than should be needed. These are provided mainly for
 * maximum flexibility and especially to facilitate a simpler HIF
 * implementation. There are per-CopyEngine recv, send, and watermark
 * contexts. These are supplied by the caller when a recv, send,
 * or watermark handler is established and they are echoed back to
 * the caller when the respective callbacks are invoked. There is
 * also a per-transfer context supplied by the caller when a buffer
 * (or sendlist) is sent and when a buffer is enqueued for recv.
 * These per-transfer contexts are echoed back to the caller when
 * the buffer is sent/received.
 * Target TX harsh result toeplitz_hash_result
 */

/*
 * Guts of ce_send, used by both ce_send and ce_sendlist_send.
 * The caller takes responsibility for any needed locking.
 */
int
ce_completed_send_next_nolock(struct CE_state *CE_state,
			      void **per_CE_contextp,
			      void **per_transfer_contextp,
			      cdf_dma_addr_t *bufferp,
			      unsigned int *nbytesp,
			      unsigned int *transfer_idp,
			      unsigned int *sw_idx, unsigned int *hw_idx,
			      uint32_t *toeplitz_hash_result);

void war_ce_src_ring_write_idx_set(struct ol_softc *scn,
				   u32 ctrl_addr, unsigned int write_index)
{
	if (hif_ce_war1) {
		void __iomem *indicator_addr;

		indicator_addr = scn->mem + ctrl_addr + DST_WATERMARK_ADDRESS;

		if (!war1_allow_sleep
		    && ctrl_addr == CE_BASE_ADDRESS(CDC_WAR_DATA_CE)) {
			hif_write32_mb(indicator_addr,
				      (CDC_WAR_MAGIC_STR | write_index));
		} else {
			unsigned long irq_flags;
			local_irq_save(irq_flags);
			hif_write32_mb(indicator_addr, 1);

			/*
			 * PCIE write waits for ACK in IPQ8K, there is no
			 * need to read back value.
			 */
			(void)hif_read32_mb(indicator_addr);
			(void)hif_read32_mb(indicator_addr); /* conservative */

			CE_SRC_RING_WRITE_IDX_SET(scn,
						  ctrl_addr, write_index);

			hif_write32_mb(indicator_addr, 0);
			local_irq_restore(irq_flags);
		}
	} else
		CE_SRC_RING_WRITE_IDX_SET(scn, ctrl_addr, write_index);
}

int
ce_send_nolock(struct CE_handle *copyeng,
			   void *per_transfer_context,
			   cdf_dma_addr_t buffer,
			   uint32_t nbytes,
			   uint32_t transfer_id,
			   uint32_t flags,
			   uint32_t user_flags)
{
	int status;
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	struct CE_ring_state *src_ring = CE_state->src_ring;
	uint32_t ctrl_addr = CE_state->ctrl_addr;
	unsigned int nentries_mask = src_ring->nentries_mask;
	unsigned int sw_index = src_ring->sw_index;
	unsigned int write_index = src_ring->write_index;
	uint64_t dma_addr = buffer;
	struct ol_softc *scn = CE_state->scn;

	A_TARGET_ACCESS_BEGIN_RET(scn);
	if (unlikely(CE_RING_DELTA(nentries_mask,
				write_index, sw_index - 1) <= 0)) {
		OL_ATH_CE_PKT_ERROR_COUNT_INCR(scn, CE_RING_DELTA_FAIL);
		status = CDF_STATUS_E_FAILURE;
		A_TARGET_ACCESS_END_RET(scn);
		return status;
	}
	{
		struct CE_src_desc *src_ring_base =
			(struct CE_src_desc *)src_ring->base_addr_owner_space;
		struct CE_src_desc *shadow_base =
			(struct CE_src_desc *)src_ring->shadow_base;
		struct CE_src_desc *src_desc =
			CE_SRC_RING_TO_DESC(src_ring_base, write_index);
		struct CE_src_desc *shadow_src_desc =
			CE_SRC_RING_TO_DESC(shadow_base, write_index);

		/* Update low 32 bits source descriptor address */
		shadow_src_desc->buffer_addr =
			(uint32_t)(dma_addr & 0xFFFFFFFF);
#ifdef QCA_WIFI_3_0
		shadow_src_desc->buffer_addr_hi =
			(uint32_t)((dma_addr >> 32) & 0x1F);
		user_flags |= shadow_src_desc->buffer_addr_hi;
		memcpy(&(((uint32_t *)shadow_src_desc)[1]), &user_flags,
			   sizeof(uint32_t));
#endif
		shadow_src_desc->meta_data = transfer_id;

		/*
		 * Set the swap bit if:
		 * typical sends on this CE are swapped (host is big-endian)
		 * and this send doesn't disable the swapping
		 * (data is not bytestream)
		 */
		shadow_src_desc->byte_swap =
			(((CE_state->attr_flags & CE_ATTR_BYTE_SWAP_DATA)
			 != 0) & ((flags & CE_SEND_FLAG_SWAP_DISABLE) == 0));
		shadow_src_desc->gather = ((flags & CE_SEND_FLAG_GATHER) != 0);
		shadow_src_desc->nbytes = nbytes;

		*src_desc = *shadow_src_desc;

		src_ring->per_transfer_context[write_index] =
			per_transfer_context;

		/* Update Source Ring Write Index */
		write_index = CE_RING_IDX_INCR(nentries_mask, write_index);

		/* WORKAROUND */
		if (!shadow_src_desc->gather) {
			war_ce_src_ring_write_idx_set(scn, ctrl_addr,
						      write_index);
		}

		src_ring->write_index = write_index;
		status = CDF_STATUS_SUCCESS;
	}
	A_TARGET_ACCESS_END_RET(scn);

	return status;
}

int
ce_send(struct CE_handle *copyeng,
		void *per_transfer_context,
		cdf_dma_addr_t buffer,
		uint32_t nbytes,
		uint32_t transfer_id,
		uint32_t flags,
		uint32_t user_flag)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	int status;

	cdf_spin_lock_bh(&CE_state->scn->target_lock);
	status = ce_send_nolock(copyeng, per_transfer_context, buffer, nbytes,
			transfer_id, flags, user_flag);
	cdf_spin_unlock_bh(&CE_state->scn->target_lock);

	return status;
}

unsigned int ce_sendlist_sizeof(void)
{
	return sizeof(struct ce_sendlist);
}

void ce_sendlist_init(struct ce_sendlist *sendlist)
{
	struct ce_sendlist_s *sl = (struct ce_sendlist_s *)sendlist;
	sl->num_items = 0;
}

int
ce_sendlist_buf_add(struct ce_sendlist *sendlist,
					cdf_dma_addr_t buffer,
					uint32_t nbytes,
					uint32_t flags,
					uint32_t user_flags)
{
	struct ce_sendlist_s *sl = (struct ce_sendlist_s *)sendlist;
	unsigned int num_items = sl->num_items;
	struct ce_sendlist_item *item;

	if (num_items >= CE_SENDLIST_ITEMS_MAX) {
		CDF_ASSERT(num_items < CE_SENDLIST_ITEMS_MAX);
		return CDF_STATUS_E_RESOURCES;
	}

	item = &sl->item[num_items];
	item->send_type = CE_SIMPLE_BUFFER_TYPE;
	item->data = buffer;
	item->u.nbytes = nbytes;
	item->flags = flags;
	item->user_flags = user_flags;
	sl->num_items = num_items + 1;
	return CDF_STATUS_SUCCESS;
}

int
ce_sendlist_send(struct CE_handle *copyeng,
		 void *per_transfer_context,
		 struct ce_sendlist *sendlist, unsigned int transfer_id)
{
	int status = -ENOMEM;
	struct ce_sendlist_s *sl = (struct ce_sendlist_s *)sendlist;
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	struct CE_ring_state *src_ring = CE_state->src_ring;
	unsigned int nentries_mask = src_ring->nentries_mask;
	unsigned int num_items = sl->num_items;
	unsigned int sw_index;
	unsigned int write_index;

	CDF_ASSERT((num_items > 0) && (num_items < src_ring->nentries));

	cdf_spin_lock_bh(&CE_state->scn->target_lock);
	sw_index = src_ring->sw_index;
	write_index = src_ring->write_index;

	if (CE_RING_DELTA(nentries_mask, write_index, sw_index - 1) >=
	    num_items) {
		struct ce_sendlist_item *item;
		int i;

		/* handle all but the last item uniformly */
		for (i = 0; i < num_items - 1; i++) {
			item = &sl->item[i];
			/* TBDXXX: Support extensible sendlist_types? */
			CDF_ASSERT(item->send_type == CE_SIMPLE_BUFFER_TYPE);
			status = ce_send_nolock(copyeng, CE_SENDLIST_ITEM_CTXT,
				(cdf_dma_addr_t) item->data,
				item->u.nbytes, transfer_id,
				item->flags | CE_SEND_FLAG_GATHER,
				item->user_flags);
			CDF_ASSERT(status == CDF_STATUS_SUCCESS);
		}
		/* provide valid context pointer for final item */
		item = &sl->item[i];
		/* TBDXXX: Support extensible sendlist_types? */
		CDF_ASSERT(item->send_type == CE_SIMPLE_BUFFER_TYPE);
		status = ce_send_nolock(copyeng, per_transfer_context,
					(cdf_dma_addr_t) item->data,
					item->u.nbytes,
					transfer_id, item->flags,
					item->user_flags);
		CDF_ASSERT(status == CDF_STATUS_SUCCESS);
		NBUF_UPDATE_TX_PKT_COUNT((cdf_nbuf_t)per_transfer_context,
					NBUF_TX_PKT_CE);
		DPTRACE(cdf_dp_trace((cdf_nbuf_t)per_transfer_context,
			CDF_DP_TRACE_CE_PACKET_PTR_RECORD,
			(uint8_t *)(((cdf_nbuf_t)per_transfer_context)->data),
			sizeof(((cdf_nbuf_t)per_transfer_context)->data)));
	} else {
		/*
		 * Probably not worth the additional complexity to support
		 * partial sends with continuation or notification.  We expect
		 * to use large rings and small sendlists. If we can't handle
		 * the entire request at once, punt it back to the caller.
		 */
	}
	cdf_spin_unlock_bh(&CE_state->scn->target_lock);

	return status;
}

#ifdef WLAN_FEATURE_FASTPATH
#ifdef QCA_WIFI_3_0
static inline void
ce_buffer_addr_hi_set(struct CE_src_desc *shadow_src_desc,
		      uint64_t dma_addr,
		      uint32_t user_flags)
{
	shadow_src_desc->buffer_addr_hi =
			(uint32_t)((dma_addr >> 32) & 0x1F);
	user_flags |= shadow_src_desc->buffer_addr_hi;
	memcpy(&(((uint32_t *)shadow_src_desc)[1]), &user_flags,
			sizeof(uint32_t));
}
#else
static inline void
ce_buffer_addr_hi_set(struct CE_src_desc *shadow_src_desc,
		      uint64_t dma_addr,
		      uint32_t user_flags)
{
}
#endif

/**
 * ce_send_fast() CE layer Tx buffer posting function
 * @copyeng: copy engine handle
 * @msdus: iarray of msdu to be sent
 * @num_msdus: number of msdus in an array
 * @transfer_id: transfer_id
 *
 * Assumption : Called with an array of MSDU's
 * Function:
 * For each msdu in the array
 * 1. Check no. of available entries
 * 2. Create src ring entries (allocated in consistent memory
 * 3. Write index to h/w
 *
 * Return: No. of packets that could be sent
 */

int ce_send_fast(struct CE_handle *copyeng, cdf_nbuf_t *msdus,
		 unsigned int num_msdus, unsigned int transfer_id)
{
	struct CE_state *ce_state = (struct CE_state *)copyeng;
	struct ol_softc *scn = ce_state->scn;
	struct CE_ring_state *src_ring = ce_state->src_ring;
	u_int32_t ctrl_addr = ce_state->ctrl_addr;
	unsigned int nentries_mask = src_ring->nentries_mask;
	unsigned int write_index;
	unsigned int sw_index;
	unsigned int frag_len;
	cdf_nbuf_t msdu;
	int i;
	uint64_t dma_addr;
	uint32_t user_flags = 0;

	/*
	 * This lock could be more fine-grained, one per CE,
	 * TODO : Add this lock now.
	 * That is the next step of optimization.
	 */
	cdf_spin_lock_bh(&scn->target_lock);
	sw_index = src_ring->sw_index;
	write_index = src_ring->write_index;

	/* 2 msdus per packet */
	for (i = 0; i < num_msdus; i++) {
		struct CE_src_desc *src_ring_base =
			(struct CE_src_desc *)src_ring->base_addr_owner_space;
		struct CE_src_desc *shadow_base =
			(struct CE_src_desc *)src_ring->shadow_base;
		struct CE_src_desc *src_desc =
			CE_SRC_RING_TO_DESC(src_ring_base, write_index);
		struct CE_src_desc *shadow_src_desc =
			CE_SRC_RING_TO_DESC(shadow_base, write_index);

		msdu = msdus[i];

		/*
		 * First fill out the ring descriptor for the HTC HTT frame
		 * header. These are uncached writes. Should we use a local
		 * structure instead?
		 */
		/* HTT/HTC header can be passed as a argument */
		dma_addr = cdf_nbuf_get_frag_paddr_lo(msdu, 0);
		shadow_src_desc->buffer_addr = (uint32_t)(dma_addr &
							  0xFFFFFFFF);
		user_flags = cdf_nbuf_data_attr_get(msdu) & DESC_DATA_FLAG_MASK;
		ce_buffer_addr_hi_set(shadow_src_desc, dma_addr, user_flags);

		shadow_src_desc->meta_data = transfer_id;
		shadow_src_desc->nbytes = cdf_nbuf_get_frag_len(msdu, 0);

		/*
		 * HTC HTT header is a word stream, so byte swap if CE byte
		 * swap enabled
		 */
		shadow_src_desc->byte_swap = ((ce_state->attr_flags &
					CE_ATTR_BYTE_SWAP_DATA) != 0);
		/* For the first one, it still does not need to write */
		shadow_src_desc->gather = 1;
		*src_desc = *shadow_src_desc;

		/* By default we could initialize the transfer context to this
		 * value
		 */
		src_ring->per_transfer_context[write_index] =
			CE_SENDLIST_ITEM_CTXT;

		write_index = CE_RING_IDX_INCR(nentries_mask, write_index);

		src_desc = CE_SRC_RING_TO_DESC(src_ring_base, write_index);
		shadow_src_desc = CE_SRC_RING_TO_DESC(shadow_base, write_index);
		/*
		 * Now fill out the ring descriptor for the actual data
		 * packet
		 */
		dma_addr = cdf_nbuf_get_frag_paddr_lo(msdu, 1);
		shadow_src_desc->buffer_addr = (uint32_t)(dma_addr &
							  0xFFFFFFFF);
		/*
		 * Clear packet offset for all but the first CE desc.
		 */
		user_flags &= ~CDF_CE_TX_PKT_OFFSET_BIT_M;
		ce_buffer_addr_hi_set(shadow_src_desc, dma_addr, user_flags);
		shadow_src_desc->meta_data = transfer_id;

		/* get actual packet length */
		frag_len = cdf_nbuf_get_frag_len(msdu, 1);
		shadow_src_desc->nbytes = frag_len > ce_state->download_len ?
			ce_state->download_len : frag_len;

		/*  Data packet is a byte stream, so disable byte swap */
		shadow_src_desc->byte_swap = 0;
		/* For the last one, gather is not set */
		shadow_src_desc->gather    = 0;
		*src_desc = *shadow_src_desc;
		src_ring->per_transfer_context[write_index] = msdu;
		write_index = CE_RING_IDX_INCR(nentries_mask, write_index);
	}

	/* Write the final index to h/w one-shot */
	if (i) {
		src_ring->write_index = write_index;
		/* Don't call WAR_XXX from here
		 * Just call XXX instead, that has the reqd. intel
		 */
		war_ce_src_ring_write_idx_set(scn, ctrl_addr, write_index);
	}

	cdf_spin_unlock_bh(&scn->target_lock);

	/*
	 * If all packets in the array are transmitted,
	 * i = num_msdus
	 * Temporarily add an ASSERT
	 */
	ASSERT(i == num_msdus);
	return i;
}
#endif /* WLAN_FEATURE_FASTPATH */

int
ce_recv_buf_enqueue(struct CE_handle *copyeng,
		    void *per_recv_context, cdf_dma_addr_t buffer)
{
	int status;
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	struct CE_ring_state *dest_ring = CE_state->dest_ring;
	uint32_t ctrl_addr = CE_state->ctrl_addr;
	unsigned int nentries_mask = dest_ring->nentries_mask;
	unsigned int write_index;
	unsigned int sw_index;
	int val = 0;
	uint64_t dma_addr = buffer;
	struct ol_softc *scn = CE_state->scn;

	cdf_spin_lock_bh(&scn->target_lock);
	write_index = dest_ring->write_index;
	sw_index = dest_ring->sw_index;

	A_TARGET_ACCESS_BEGIN_RET_EXT(scn, val);
	if (val == -1) {
		cdf_spin_unlock_bh(&scn->target_lock);
		return val;
	}

	if (CE_RING_DELTA(nentries_mask, write_index, sw_index - 1) > 0) {
		struct CE_dest_desc *dest_ring_base =
			(struct CE_dest_desc *)dest_ring->
			    base_addr_owner_space;
		struct CE_dest_desc *dest_desc =
			CE_DEST_RING_TO_DESC(dest_ring_base, write_index);

		/* Update low 32 bit destination descriptor */
		dest_desc->buffer_addr = (uint32_t)(dma_addr & 0xFFFFFFFF);
#ifdef QCA_WIFI_3_0
		dest_desc->buffer_addr_hi =
			(uint32_t)((dma_addr >> 32) & 0x1F);
#endif
		dest_desc->nbytes = 0;

		dest_ring->per_transfer_context[write_index] =
			per_recv_context;

		/* Update Destination Ring Write Index */
		write_index = CE_RING_IDX_INCR(nentries_mask, write_index);
		CE_DEST_RING_WRITE_IDX_SET(scn, ctrl_addr, write_index);
		dest_ring->write_index = write_index;
		status = CDF_STATUS_SUCCESS;
	} else {
		status = CDF_STATUS_E_FAILURE;
	}
	A_TARGET_ACCESS_END_RET_EXT(scn, val);
	if (val == -1) {
		cdf_spin_unlock_bh(&scn->target_lock);
		return val;
	}

	cdf_spin_unlock_bh(&scn->target_lock);

	return status;
}

void
ce_send_watermarks_set(struct CE_handle *copyeng,
		       unsigned int low_alert_nentries,
		       unsigned int high_alert_nentries)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	uint32_t ctrl_addr = CE_state->ctrl_addr;
	struct ol_softc *scn = CE_state->scn;

	cdf_spin_lock(&scn->target_lock);
	CE_SRC_RING_LOWMARK_SET(scn, ctrl_addr, low_alert_nentries);
	CE_SRC_RING_HIGHMARK_SET(scn, ctrl_addr, high_alert_nentries);
	cdf_spin_unlock(&scn->target_lock);
}

void
ce_recv_watermarks_set(struct CE_handle *copyeng,
		       unsigned int low_alert_nentries,
		       unsigned int high_alert_nentries)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	uint32_t ctrl_addr = CE_state->ctrl_addr;
	struct ol_softc *scn = CE_state->scn;

	cdf_spin_lock(&scn->target_lock);
	CE_DEST_RING_LOWMARK_SET(scn, ctrl_addr,
				low_alert_nentries);
	CE_DEST_RING_HIGHMARK_SET(scn, ctrl_addr,
				high_alert_nentries);
	cdf_spin_unlock(&scn->target_lock);
}

unsigned int ce_send_entries_avail(struct CE_handle *copyeng)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	struct CE_ring_state *src_ring = CE_state->src_ring;
	unsigned int nentries_mask = src_ring->nentries_mask;
	unsigned int sw_index;
	unsigned int write_index;

	cdf_spin_lock(&CE_state->scn->target_lock);
	sw_index = src_ring->sw_index;
	write_index = src_ring->write_index;
	cdf_spin_unlock(&CE_state->scn->target_lock);

	return CE_RING_DELTA(nentries_mask, write_index, sw_index - 1);
}

unsigned int ce_recv_entries_avail(struct CE_handle *copyeng)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	struct CE_ring_state *dest_ring = CE_state->dest_ring;
	unsigned int nentries_mask = dest_ring->nentries_mask;
	unsigned int sw_index;
	unsigned int write_index;

	cdf_spin_lock(&CE_state->scn->target_lock);
	sw_index = dest_ring->sw_index;
	write_index = dest_ring->write_index;
	cdf_spin_unlock(&CE_state->scn->target_lock);

	return CE_RING_DELTA(nentries_mask, write_index, sw_index - 1);
}

/*
 * Guts of ce_send_entries_done.
 * The caller takes responsibility for any necessary locking.
 */
unsigned int
ce_send_entries_done_nolock(struct ol_softc *scn,
			    struct CE_state *CE_state)
{
	struct CE_ring_state *src_ring = CE_state->src_ring;
	uint32_t ctrl_addr = CE_state->ctrl_addr;
	unsigned int nentries_mask = src_ring->nentries_mask;
	unsigned int sw_index;
	unsigned int read_index;

	sw_index = src_ring->sw_index;
	read_index = CE_SRC_RING_READ_IDX_GET(scn, ctrl_addr);

	return CE_RING_DELTA(nentries_mask, sw_index, read_index);
}

unsigned int ce_send_entries_done(struct CE_handle *copyeng)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	unsigned int nentries;

	cdf_spin_lock(&CE_state->scn->target_lock);
	nentries = ce_send_entries_done_nolock(CE_state->scn, CE_state);
	cdf_spin_unlock(&CE_state->scn->target_lock);

	return nentries;
}

/*
 * Guts of ce_recv_entries_done.
 * The caller takes responsibility for any necessary locking.
 */
unsigned int
ce_recv_entries_done_nolock(struct ol_softc *scn,
			    struct CE_state *CE_state)
{
	struct CE_ring_state *dest_ring = CE_state->dest_ring;
	uint32_t ctrl_addr = CE_state->ctrl_addr;
	unsigned int nentries_mask = dest_ring->nentries_mask;
	unsigned int sw_index;
	unsigned int read_index;

	sw_index = dest_ring->sw_index;
	read_index = CE_DEST_RING_READ_IDX_GET(scn, ctrl_addr);

	return CE_RING_DELTA(nentries_mask, sw_index, read_index);
}

unsigned int ce_recv_entries_done(struct CE_handle *copyeng)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	unsigned int nentries;

	cdf_spin_lock(&CE_state->scn->target_lock);
	nentries = ce_recv_entries_done_nolock(CE_state->scn, CE_state);
	cdf_spin_unlock(&CE_state->scn->target_lock);

	return nentries;
}

/* Debug support */
void *ce_debug_cmplrn_context;  /* completed recv next context */
void *ce_debug_cnclsn_context;  /* cancel send next context */
void *ce_debug_rvkrn_context;   /* revoke receive next context */
void *ce_debug_cmplsn_context;  /* completed send next context */

/*
 * Guts of ce_completed_recv_next.
 * The caller takes responsibility for any necessary locking.
 */
int
ce_completed_recv_next_nolock(struct CE_state *CE_state,
			      void **per_CE_contextp,
			      void **per_transfer_contextp,
			      cdf_dma_addr_t *bufferp,
			      unsigned int *nbytesp,
			      unsigned int *transfer_idp,
			      unsigned int *flagsp)
{
	int status;
	struct CE_ring_state *dest_ring = CE_state->dest_ring;
	unsigned int nentries_mask = dest_ring->nentries_mask;
	unsigned int sw_index = dest_ring->sw_index;

	struct CE_dest_desc *dest_ring_base =
		(struct CE_dest_desc *)dest_ring->base_addr_owner_space;
	struct CE_dest_desc *dest_desc =
		CE_DEST_RING_TO_DESC(dest_ring_base, sw_index);
	int nbytes;
	struct CE_dest_desc dest_desc_info;
	/*
	 * By copying the dest_desc_info element to local memory, we could
	 * avoid extra memory read from non-cachable memory.
	 */
	dest_desc_info =  *dest_desc;
	nbytes = dest_desc_info.nbytes;
	if (nbytes == 0) {
		/*
		 * This closes a relatively unusual race where the Host
		 * sees the updated DRRI before the update to the
		 * corresponding descriptor has completed. We treat this
		 * as a descriptor that is not yet done.
		 */
		status = CDF_STATUS_E_FAILURE;
		goto done;
	}

	dest_desc->nbytes = 0;

	/* Return data from completed destination descriptor */
	*bufferp = HIF_CE_DESC_ADDR_TO_DMA(&dest_desc_info);
	*nbytesp = nbytes;
	*transfer_idp = dest_desc_info.meta_data;
	*flagsp = (dest_desc_info.byte_swap) ? CE_RECV_FLAG_SWAPPED : 0;

	if (per_CE_contextp) {
		*per_CE_contextp = CE_state->recv_context;
	}

	ce_debug_cmplrn_context = dest_ring->per_transfer_context[sw_index];
	if (per_transfer_contextp) {
		*per_transfer_contextp = ce_debug_cmplrn_context;
	}
	dest_ring->per_transfer_context[sw_index] = 0;  /* sanity */

	/* Update sw_index */
	sw_index = CE_RING_IDX_INCR(nentries_mask, sw_index);
	dest_ring->sw_index = sw_index;
	status = CDF_STATUS_SUCCESS;

done:
	return status;
}

int
ce_completed_recv_next(struct CE_handle *copyeng,
		       void **per_CE_contextp,
		       void **per_transfer_contextp,
		       cdf_dma_addr_t *bufferp,
		       unsigned int *nbytesp,
		       unsigned int *transfer_idp, unsigned int *flagsp)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	int status;

	cdf_spin_lock_bh(&CE_state->scn->target_lock);
	status =
		ce_completed_recv_next_nolock(CE_state, per_CE_contextp,
					      per_transfer_contextp, bufferp,
					      nbytesp, transfer_idp, flagsp);
	cdf_spin_unlock_bh(&CE_state->scn->target_lock);

	return status;
}

/* NB: Modeled after ce_completed_recv_next_nolock */
CDF_STATUS
ce_revoke_recv_next(struct CE_handle *copyeng,
		    void **per_CE_contextp,
		    void **per_transfer_contextp, cdf_dma_addr_t *bufferp)
{
	struct CE_state *CE_state;
	struct CE_ring_state *dest_ring;
	unsigned int nentries_mask;
	unsigned int sw_index;
	unsigned int write_index;
	CDF_STATUS status;
	struct ol_softc *scn;

	CE_state = (struct CE_state *)copyeng;
	dest_ring = CE_state->dest_ring;
	if (!dest_ring) {
		return CDF_STATUS_E_FAILURE;
	}

	scn = CE_state->scn;
	cdf_spin_lock(&scn->target_lock);
	nentries_mask = dest_ring->nentries_mask;
	sw_index = dest_ring->sw_index;
	write_index = dest_ring->write_index;
	if (write_index != sw_index) {
		struct CE_dest_desc *dest_ring_base =
			(struct CE_dest_desc *)dest_ring->
			    base_addr_owner_space;
		struct CE_dest_desc *dest_desc =
			CE_DEST_RING_TO_DESC(dest_ring_base, sw_index);

		/* Return data from completed destination descriptor */
		*bufferp = HIF_CE_DESC_ADDR_TO_DMA(dest_desc);

		if (per_CE_contextp) {
			*per_CE_contextp = CE_state->recv_context;
		}

		ce_debug_rvkrn_context =
			dest_ring->per_transfer_context[sw_index];
		if (per_transfer_contextp) {
			*per_transfer_contextp = ce_debug_rvkrn_context;
		}
		dest_ring->per_transfer_context[sw_index] = 0;  /* sanity */

		/* Update sw_index */
		sw_index = CE_RING_IDX_INCR(nentries_mask, sw_index);
		dest_ring->sw_index = sw_index;
		status = CDF_STATUS_SUCCESS;
	} else {
		status = CDF_STATUS_E_FAILURE;
	}
	cdf_spin_unlock(&scn->target_lock);

	return status;
}

/*
 * Guts of ce_completed_send_next.
 * The caller takes responsibility for any necessary locking.
 */
int
ce_completed_send_next_nolock(struct CE_state *CE_state,
			      void **per_CE_contextp,
			      void **per_transfer_contextp,
			      cdf_dma_addr_t *bufferp,
			      unsigned int *nbytesp,
			      unsigned int *transfer_idp,
			      unsigned int *sw_idx,
			      unsigned int *hw_idx,
			      uint32_t *toeplitz_hash_result)
{
	int status = CDF_STATUS_E_FAILURE;
	struct CE_ring_state *src_ring = CE_state->src_ring;
	uint32_t ctrl_addr = CE_state->ctrl_addr;
	unsigned int nentries_mask = src_ring->nentries_mask;
	unsigned int sw_index = src_ring->sw_index;
	unsigned int read_index;
	struct ol_softc *scn = CE_state->scn;

	if (src_ring->hw_index == sw_index) {
		/*
		 * The SW completion index has caught up with the cached
		 * version of the HW completion index.
		 * Update the cached HW completion index to see whether
		 * the SW has really caught up to the HW, or if the cached
		 * value of the HW index has become stale.
		 */
		A_TARGET_ACCESS_BEGIN_RET(scn);
		src_ring->hw_index =
			CE_SRC_RING_READ_IDX_GET(scn, ctrl_addr);
		A_TARGET_ACCESS_END_RET(scn);
	}
	read_index = src_ring->hw_index;

	if (sw_idx)
		*sw_idx = sw_index;

	if (hw_idx)
		*hw_idx = read_index;

	if ((read_index != sw_index) && (read_index != 0xffffffff)) {
		struct CE_src_desc *shadow_base =
			(struct CE_src_desc *)src_ring->shadow_base;
		struct CE_src_desc *shadow_src_desc =
			CE_SRC_RING_TO_DESC(shadow_base, sw_index);
#ifdef QCA_WIFI_3_0
		struct CE_src_desc *src_ring_base =
			(struct CE_src_desc *)src_ring->base_addr_owner_space;
		struct CE_src_desc *src_desc =
			CE_SRC_RING_TO_DESC(src_ring_base, sw_index);
#endif
		/* Return data from completed source descriptor */
		*bufferp = HIF_CE_DESC_ADDR_TO_DMA(shadow_src_desc);
		*nbytesp = shadow_src_desc->nbytes;
		*transfer_idp = shadow_src_desc->meta_data;
#ifdef QCA_WIFI_3_0
		*toeplitz_hash_result = src_desc->toeplitz_hash_result;
#else
		*toeplitz_hash_result = 0;
#endif
		if (per_CE_contextp) {
			*per_CE_contextp = CE_state->send_context;
		}

		ce_debug_cmplsn_context =
			src_ring->per_transfer_context[sw_index];
		if (per_transfer_contextp) {
			*per_transfer_contextp = ce_debug_cmplsn_context;
		}
		src_ring->per_transfer_context[sw_index] = 0;   /* sanity */

		/* Update sw_index */
		sw_index = CE_RING_IDX_INCR(nentries_mask, sw_index);
		src_ring->sw_index = sw_index;
		status = CDF_STATUS_SUCCESS;
	}

	return status;
}

/* NB: Modeled after ce_completed_send_next */
CDF_STATUS
ce_cancel_send_next(struct CE_handle *copyeng,
		void **per_CE_contextp,
		void **per_transfer_contextp,
		cdf_dma_addr_t *bufferp,
		unsigned int *nbytesp,
		unsigned int *transfer_idp,
		uint32_t *toeplitz_hash_result)
{
	struct CE_state *CE_state;
	struct CE_ring_state *src_ring;
	unsigned int nentries_mask;
	unsigned int sw_index;
	unsigned int write_index;
	CDF_STATUS status;
	struct ol_softc *scn;

	CE_state = (struct CE_state *)copyeng;
	src_ring = CE_state->src_ring;
	if (!src_ring) {
		return CDF_STATUS_E_FAILURE;
	}

	scn = CE_state->scn;
	cdf_spin_lock(&CE_state->scn->target_lock);
	nentries_mask = src_ring->nentries_mask;
	sw_index = src_ring->sw_index;
	write_index = src_ring->write_index;

	if (write_index != sw_index) {
		struct CE_src_desc *src_ring_base =
			(struct CE_src_desc *)src_ring->base_addr_owner_space;
		struct CE_src_desc *src_desc =
			CE_SRC_RING_TO_DESC(src_ring_base, sw_index);

		/* Return data from completed source descriptor */
		*bufferp = HIF_CE_DESC_ADDR_TO_DMA(src_desc);
		*nbytesp = src_desc->nbytes;
		*transfer_idp = src_desc->meta_data;
#ifdef QCA_WIFI_3_0
		*toeplitz_hash_result = src_desc->toeplitz_hash_result;
#else
		*toeplitz_hash_result = 0;
#endif

		if (per_CE_contextp) {
			*per_CE_contextp = CE_state->send_context;
		}

		ce_debug_cnclsn_context =
			src_ring->per_transfer_context[sw_index];
		if (per_transfer_contextp) {
			*per_transfer_contextp = ce_debug_cnclsn_context;
		}
		src_ring->per_transfer_context[sw_index] = 0;   /* sanity */

		/* Update sw_index */
		sw_index = CE_RING_IDX_INCR(nentries_mask, sw_index);
		src_ring->sw_index = sw_index;
		status = CDF_STATUS_SUCCESS;
	} else {
		status = CDF_STATUS_E_FAILURE;
	}
	cdf_spin_unlock(&CE_state->scn->target_lock);

	return status;
}

/* Shift bits to convert IS_*_RING_*_WATERMARK_MASK to CE_WM_FLAG_*_* */
#define CE_WM_SHFT 1

int
ce_completed_send_next(struct CE_handle *copyeng,
		       void **per_CE_contextp,
		       void **per_transfer_contextp,
		       cdf_dma_addr_t *bufferp,
		       unsigned int *nbytesp,
		       unsigned int *transfer_idp,
		       unsigned int *sw_idx,
		       unsigned int *hw_idx,
		       unsigned int *toeplitz_hash_result)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	int status;

	cdf_spin_lock_bh(&CE_state->scn->target_lock);
	status =
		ce_completed_send_next_nolock(CE_state, per_CE_contextp,
					      per_transfer_contextp, bufferp,
					      nbytesp, transfer_idp, sw_idx,
					      hw_idx, toeplitz_hash_result);
	cdf_spin_unlock_bh(&CE_state->scn->target_lock);

	return status;
}

#ifdef ATH_11AC_TXCOMPACT
/* CE engine descriptor reap
 * Similar to ce_per_engine_service , Only difference is ce_per_engine_service
 * does recieve and reaping of completed descriptor ,
 * This function only handles reaping of Tx complete descriptor.
 * The Function is called from threshold reap  poll routine
 * hif_send_complete_check so should not countain recieve functionality
 * within it .
 */

void ce_per_engine_servicereap(struct ol_softc *scn, unsigned int CE_id)
{
	void *CE_context;
	void *transfer_context;
	cdf_dma_addr_t buf;
	unsigned int nbytes;
	unsigned int id;
	unsigned int sw_idx, hw_idx;
	uint32_t toeplitz_hash_result;
	struct CE_state *CE_state = scn->ce_id_to_state[CE_id];

	A_TARGET_ACCESS_BEGIN(scn);

	/* Since this function is called from both user context and
	 * tasklet context the spinlock has to lock the bottom halves.
	 * This fix assumes that ATH_11AC_TXCOMPACT flag is always
	 * enabled in TX polling mode. If this is not the case, more
	 * bottom halve spin lock changes are needed. Due to data path
	 * performance concern, after internal discussion we've decided
	 * to make minimum change, i.e., only address the issue occured
	 * in this function. The possible negative effect of this minimum
	 * change is that, in the future, if some other function will also
	 * be opened to let the user context to use, those cases need to be
	 * addressed by change spin_lock to spin_lock_bh also.
	 */

	cdf_spin_lock_bh(&scn->target_lock);

	if (CE_state->send_cb) {
		{
			/* Pop completed send buffers and call the
			 * registered send callback for each
			 */
			while (ce_completed_send_next_nolock
				 (CE_state, &CE_context,
				  &transfer_context, &buf,
				  &nbytes, &id, &sw_idx, &hw_idx,
				  &toeplitz_hash_result) ==
				  CDF_STATUS_SUCCESS) {
				if (CE_id != CE_HTT_H2T_MSG) {
					cdf_spin_unlock_bh(&scn->target_lock);
					CE_state->
					send_cb((struct CE_handle *)
						CE_state, CE_context,
						transfer_context, buf,
						nbytes, id, sw_idx, hw_idx,
						toeplitz_hash_result);
					cdf_spin_lock_bh(&scn->target_lock);
				} else {
					struct HIF_CE_pipe_info *pipe_info =
						(struct HIF_CE_pipe_info *)
						CE_context;

					cdf_spin_lock_bh(&pipe_info->
						 completion_freeq_lock);
					pipe_info->num_sends_allowed++;
					cdf_spin_unlock_bh(&pipe_info->
						   completion_freeq_lock);
				}
			}
		}
	}

	cdf_spin_unlock_bh(&scn->target_lock);
	A_TARGET_ACCESS_END(scn);
}

#endif /*ATH_11AC_TXCOMPACT */

/*
 * Number of times to check for any pending tx/rx completion on
 * a copy engine, this count should be big enough. Once we hit
 * this threashold we'll not check for any Tx/Rx comlpetion in same
 * interrupt handling. Note that this threashold is only used for
 * Rx interrupt processing, this can be used tor Tx as well if we
 * suspect any infinite loop in checking for pending Tx completion.
 */
#define CE_TXRX_COMP_CHECK_THRESHOLD 20

/*
 * Guts of interrupt handler for per-engine interrupts on a particular CE.
 *
 * Invokes registered callbacks for recv_complete,
 * send_complete, and watermarks.
 *
 * Returns: number of messages processed
 */

int ce_per_engine_service(struct ol_softc *scn, unsigned int CE_id)
{
	struct CE_state *CE_state = scn->ce_id_to_state[CE_id];
	uint32_t ctrl_addr = CE_state->ctrl_addr;
	void *CE_context;
	void *transfer_context;
	cdf_dma_addr_t buf;
	unsigned int nbytes;
	unsigned int id;
	unsigned int flags;
	uint32_t CE_int_status;
	unsigned int more_comp_cnt = 0;
	unsigned int more_snd_comp_cnt = 0;
	unsigned int sw_idx, hw_idx;
	uint32_t toeplitz_hash_result;

	if (Q_TARGET_ACCESS_BEGIN(scn) < 0) {
		HIF_ERROR("[premature rc=0]\n");
		return 0; /* no work done */
	}

	cdf_spin_lock(&scn->target_lock);

	/* Clear force_break flag and re-initialize receive_count to 0 */

	/* NAPI: scn variables- thread/multi-processing safety? */
	scn->receive_count = 0;
	CE_state->force_break = 0;
more_completions:
	if (CE_state->recv_cb) {

		/* Pop completed recv buffers and call
		 * the registered recv callback for each
		 */
		while (ce_completed_recv_next_nolock
				(CE_state, &CE_context, &transfer_context,
				&buf, &nbytes, &id, &flags) ==
				CDF_STATUS_SUCCESS) {
			cdf_spin_unlock(&scn->target_lock);
			CE_state->recv_cb((struct CE_handle *)CE_state,
					  CE_context, transfer_context, buf,
					  nbytes, id, flags);

			/*
			 * EV #112693 -
			 * [Peregrine][ES1][WB342][Win8x86][Performance]
			 * BSoD_0x133 occurred in VHT80 UDP_DL
			 * Break out DPC by force if number of loops in
			 * hif_pci_ce_recv_data reaches MAX_NUM_OF_RECEIVES
			 * to avoid spending too long time in
			 * DPC for each interrupt handling. Schedule another
			 * DPC to avoid data loss if we had taken
			 * force-break action before apply to Windows OS
			 * only currently, Linux/MAC os can expand to their
			 * platform if necessary
			 */

			/* Break the receive processes by
			 * force if force_break set up
			 */
			if (cdf_unlikely(CE_state->force_break)) {
				cdf_atomic_set(&CE_state->rx_pending, 1);
				CE_ENGINE_INT_STATUS_CLEAR(scn, ctrl_addr,
					HOST_IS_COPY_COMPLETE_MASK);
				if (Q_TARGET_ACCESS_END(scn) < 0)
					HIF_ERROR("<--[premature rc=%d]\n",
						  scn->receive_count);
				return scn->receive_count;
			}
			cdf_spin_lock(&scn->target_lock);
		}
	}

	/*
	 * Attention: We may experience potential infinite loop for below
	 * While Loop during Sending Stress test.
	 * Resolve the same way as Receive Case (Refer to EV #112693)
	 */

	if (CE_state->send_cb) {
		/* Pop completed send buffers and call
		 * the registered send callback for each
		 */

#ifdef ATH_11AC_TXCOMPACT
		while (ce_completed_send_next_nolock
			 (CE_state, &CE_context,
			 &transfer_context, &buf, &nbytes,
			 &id, &sw_idx, &hw_idx,
			 &toeplitz_hash_result) == CDF_STATUS_SUCCESS) {

			if (CE_id != CE_HTT_H2T_MSG ||
			    WLAN_IS_EPPING_ENABLED(cds_get_conparam())) {
				cdf_spin_unlock(&scn->target_lock);
				CE_state->send_cb((struct CE_handle *)CE_state,
						  CE_context, transfer_context,
						  buf, nbytes, id, sw_idx,
						  hw_idx, toeplitz_hash_result);
				cdf_spin_lock(&scn->target_lock);
			} else {
				struct HIF_CE_pipe_info *pipe_info =
					(struct HIF_CE_pipe_info *)CE_context;

				cdf_spin_lock(&pipe_info->
					      completion_freeq_lock);
				pipe_info->num_sends_allowed++;
				cdf_spin_unlock(&pipe_info->
						completion_freeq_lock);
			}
		}
#else                           /*ATH_11AC_TXCOMPACT */
		while (ce_completed_send_next_nolock
			 (CE_state, &CE_context,
			  &transfer_context, &buf, &nbytes,
			  &id, &sw_idx, &hw_idx,
			  &toeplitz_hash_result) == CDF_STATUS_SUCCESS) {
			cdf_spin_unlock(&scn->target_lock);
			CE_state->send_cb((struct CE_handle *)CE_state,
				  CE_context, transfer_context, buf,
				  nbytes, id, sw_idx, hw_idx,
				  toeplitz_hash_result);
			cdf_spin_lock(&scn->target_lock);
		}
#endif /*ATH_11AC_TXCOMPACT */
	}

more_watermarks:
	if (CE_state->misc_cbs) {
		CE_int_status = CE_ENGINE_INT_STATUS_GET(scn, ctrl_addr);
		if (CE_int_status & CE_WATERMARK_MASK) {
			if (CE_state->watermark_cb) {

				cdf_spin_unlock(&scn->target_lock);
				/* Convert HW IS bits to software flags */
				flags =
					(CE_int_status & CE_WATERMARK_MASK) >>
					CE_WM_SHFT;

				CE_state->
				watermark_cb((struct CE_handle *)CE_state,
					     CE_state->wm_context, flags);
				cdf_spin_lock(&scn->target_lock);
			}
		}
	}

	/*
	 * Clear the misc interrupts (watermark) that were handled above,
	 * and that will be checked again below.
	 * Clear and check for copy-complete interrupts again, just in case
	 * more copy completions happened while the misc interrupts were being
	 * handled.
	 */
	CE_ENGINE_INT_STATUS_CLEAR(scn, ctrl_addr,
				   CE_WATERMARK_MASK |
				   HOST_IS_COPY_COMPLETE_MASK);

	/*
	 * Now that per-engine interrupts are cleared, verify that
	 * no recv interrupts arrive while processing send interrupts,
	 * and no recv or send interrupts happened while processing
	 * misc interrupts.Go back and check again.Keep checking until
	 * we find no more events to process.
	 */
	if (CE_state->recv_cb && ce_recv_entries_done_nolock(scn, CE_state)) {
		if (WLAN_IS_EPPING_ENABLED(cds_get_conparam()) ||
		    more_comp_cnt++ < CE_TXRX_COMP_CHECK_THRESHOLD) {
			goto more_completions;
		} else {
			HIF_ERROR(
				"%s:Potential infinite loop detected during Rx processing nentries_mask:0x%x sw read_idx:0x%x hw read_idx:0x%x",
				__func__, CE_state->dest_ring->nentries_mask,
				CE_state->dest_ring->sw_index,
				CE_DEST_RING_READ_IDX_GET(scn,
							  CE_state->ctrl_addr));
		}
	}

	if (CE_state->send_cb && ce_send_entries_done_nolock(scn, CE_state)) {
		if (WLAN_IS_EPPING_ENABLED(cds_get_conparam()) ||
		    more_snd_comp_cnt++ < CE_TXRX_COMP_CHECK_THRESHOLD) {
			goto more_completions;
		} else {
			HIF_ERROR(
				"%s:Potential infinite loop detected during send completion nentries_mask:0x%x sw read_idx:0x%x hw read_idx:0x%x",
				__func__, CE_state->src_ring->nentries_mask,
				CE_state->src_ring->sw_index,
				CE_SRC_RING_READ_IDX_GET(scn,
							 CE_state->ctrl_addr));
		}
	}

	if (CE_state->misc_cbs) {
		CE_int_status = CE_ENGINE_INT_STATUS_GET(scn, ctrl_addr);
		if (CE_int_status & CE_WATERMARK_MASK) {
			if (CE_state->watermark_cb) {
				goto more_watermarks;
			}
		}
	}

	cdf_spin_unlock(&scn->target_lock);
	cdf_atomic_set(&CE_state->rx_pending, 0);

	if (Q_TARGET_ACCESS_END(scn) < 0)
		HIF_ERROR("<--[premature rc=%d]\n", scn->receive_count);
	return scn->receive_count;
}

/*
 * Handler for per-engine interrupts on ALL active CEs.
 * This is used in cases where the system is sharing a
 * single interrput for all CEs
 */

void ce_per_engine_service_any(int irq, struct ol_softc *scn)
{
	int CE_id;
	uint32_t intr_summary;

	A_TARGET_ACCESS_BEGIN(scn);
	if (!cdf_atomic_read(&scn->tasklet_from_intr)) {
		for (CE_id = 0; CE_id < scn->ce_count; CE_id++) {
			struct CE_state *CE_state = scn->ce_id_to_state[CE_id];
			if (cdf_atomic_read(&CE_state->rx_pending)) {
				cdf_atomic_set(&CE_state->rx_pending, 0);
				ce_per_engine_service(scn, CE_id);
			}
		}

		A_TARGET_ACCESS_END(scn);
		return;
	}

	intr_summary = CE_INTERRUPT_SUMMARY(scn);

	for (CE_id = 0; intr_summary && (CE_id < scn->ce_count); CE_id++) {
		if (intr_summary & (1 << CE_id)) {
			intr_summary &= ~(1 << CE_id);
		} else {
			continue;       /* no intr pending on this CE */
		}

		ce_per_engine_service(scn, CE_id);
	}

	A_TARGET_ACCESS_END(scn);
}

/*
 * Adjust interrupts for the copy complete handler.
 * If it's needed for either send or recv, then unmask
 * this interrupt; otherwise, mask it.
 *
 * Called with target_lock held.
 */
static void
ce_per_engine_handler_adjust(struct CE_state *CE_state,
			     int disable_copy_compl_intr)
{
	uint32_t ctrl_addr = CE_state->ctrl_addr;
	struct ol_softc *scn = CE_state->scn;

	CE_state->disable_copy_compl_intr = disable_copy_compl_intr;
	A_TARGET_ACCESS_BEGIN(scn);
	if ((!disable_copy_compl_intr) &&
	    (CE_state->send_cb || CE_state->recv_cb)) {
		CE_COPY_COMPLETE_INTR_ENABLE(scn, ctrl_addr);
	} else {
		CE_COPY_COMPLETE_INTR_DISABLE(scn, ctrl_addr);
	}

	if (CE_state->watermark_cb) {
		CE_WATERMARK_INTR_ENABLE(scn, ctrl_addr);
	} else {
		CE_WATERMARK_INTR_DISABLE(scn, ctrl_addr);
	}
	A_TARGET_ACCESS_END(scn);

}

/*Iterate the CE_state list and disable the compl interrupt
 * if it has been registered already.
 */
void ce_disable_any_copy_compl_intr_nolock(struct ol_softc *scn)
{
	int CE_id;

	A_TARGET_ACCESS_BEGIN(scn);
	for (CE_id = 0; CE_id < scn->ce_count; CE_id++) {
		struct CE_state *CE_state = scn->ce_id_to_state[CE_id];
		uint32_t ctrl_addr = CE_state->ctrl_addr;

		/* if the interrupt is currently enabled, disable it */
		if (!CE_state->disable_copy_compl_intr
		    && (CE_state->send_cb || CE_state->recv_cb)) {
			CE_COPY_COMPLETE_INTR_DISABLE(scn, ctrl_addr);
		}

		if (CE_state->watermark_cb) {
			CE_WATERMARK_INTR_DISABLE(scn, ctrl_addr);
		}
	}
	A_TARGET_ACCESS_END(scn);
}

void ce_enable_any_copy_compl_intr_nolock(struct ol_softc *scn)
{
	int CE_id;

	A_TARGET_ACCESS_BEGIN(scn);
	for (CE_id = 0; CE_id < scn->ce_count; CE_id++) {
		struct CE_state *CE_state = scn->ce_id_to_state[CE_id];
		uint32_t ctrl_addr = CE_state->ctrl_addr;

		/*
		 * If the CE is supposed to have copy complete interrupts
		 * enabled (i.e. there a callback registered, and the
		 * "disable" flag is not set), then re-enable the interrupt.
		 */
		if (!CE_state->disable_copy_compl_intr
		    && (CE_state->send_cb || CE_state->recv_cb)) {
			CE_COPY_COMPLETE_INTR_ENABLE(scn, ctrl_addr);
		}

		if (CE_state->watermark_cb) {
			CE_WATERMARK_INTR_ENABLE(scn, ctrl_addr);
		}
	}
	A_TARGET_ACCESS_END(scn);
}

void ce_disable_any_copy_compl_intr(struct ol_softc *scn)
{
	cdf_spin_lock(&scn->target_lock);
	ce_disable_any_copy_compl_intr_nolock(scn);
	cdf_spin_unlock(&scn->target_lock);
}

/*Re-enable the copy compl interrupt if it has not been disabled before.*/
void ce_enable_any_copy_compl_intr(struct ol_softc *scn)
{
	cdf_spin_lock(&scn->target_lock);
	ce_enable_any_copy_compl_intr_nolock(scn);
	cdf_spin_unlock(&scn->target_lock);
}

void
ce_send_cb_register(struct CE_handle *copyeng,
		    ce_send_cb fn_ptr,
		    void *ce_send_context, int disable_interrupts)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;

	if (CE_state == NULL) {
		pr_err("%s: Error CE state = NULL\n", __func__);
		return;
	}
	cdf_spin_lock(&CE_state->scn->target_lock);
	CE_state->send_cb = fn_ptr;
	CE_state->send_context = ce_send_context;
	ce_per_engine_handler_adjust(CE_state, disable_interrupts);
	cdf_spin_unlock(&CE_state->scn->target_lock);
}

void
ce_recv_cb_register(struct CE_handle *copyeng,
		    CE_recv_cb fn_ptr,
		    void *CE_recv_context, int disable_interrupts)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;

	if (CE_state == NULL) {
		pr_err("%s: ERROR CE state = NULL\n", __func__);
		return;
	}
	cdf_spin_lock(&CE_state->scn->target_lock);
	CE_state->recv_cb = fn_ptr;
	CE_state->recv_context = CE_recv_context;
	ce_per_engine_handler_adjust(CE_state, disable_interrupts);
	cdf_spin_unlock(&CE_state->scn->target_lock);
}

void
ce_watermark_cb_register(struct CE_handle *copyeng,
			 CE_watermark_cb fn_ptr, void *CE_wm_context)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;

	cdf_spin_lock(&CE_state->scn->target_lock);
	CE_state->watermark_cb = fn_ptr;
	CE_state->wm_context = CE_wm_context;
	ce_per_engine_handler_adjust(CE_state, 0);
	if (fn_ptr) {
		CE_state->misc_cbs = 1;
	}
	cdf_spin_unlock(&CE_state->scn->target_lock);
}

#ifdef WLAN_FEATURE_FASTPATH
/**
 * ce_pkt_dl_len_set() set the HTT packet download length
 * @hif_sc: HIF context
 * @pkt_download_len: download length
 *
 * Return: None
 */
void ce_pkt_dl_len_set(void *hif_sc, u_int32_t pkt_download_len)
{
	struct ol_softc *sc = (struct ol_softc *)(hif_sc);
	struct CE_state *ce_state = sc->ce_id_to_state[CE_HTT_H2T_MSG];

	cdf_assert_always(ce_state);

	cdf_spin_lock_bh(&sc->target_lock);
	ce_state->download_len = pkt_download_len;
	cdf_spin_unlock_bh(&sc->target_lock);

	cdf_print("%s CE %d Pkt download length %d\n", __func__,
		  ce_state->id, ce_state->download_len);
}
#else
void ce_pkt_dl_len_set(void *hif_sc, u_int32_t pkt_download_len)
{
}
#endif /* WLAN_FEATURE_FASTPATH */

bool ce_get_rx_pending(struct ol_softc *scn)
{
	int CE_id;

	for (CE_id = 0; CE_id < scn->ce_count; CE_id++) {
		struct CE_state *CE_state = scn->ce_id_to_state[CE_id];
		if (cdf_atomic_read(&CE_state->rx_pending))
			return true;
	}

	return false;
}

/**
 * ce_check_rx_pending() - ce_check_rx_pending
 * @scn: ol_softc
 * @ce_id: ce_id
 *
 * Return: bool
 */
bool ce_check_rx_pending(struct ol_softc *scn, int ce_id)
{
	struct CE_state *CE_state = scn->ce_id_to_state[ce_id];
	if (cdf_atomic_read(&CE_state->rx_pending))
		return true;
	else
		return false;
}
void ce_enable_msi(struct ol_softc *scn, unsigned int CE_id,
				   uint32_t msi_addr_lo, uint32_t msi_addr_hi,
				   uint32_t msi_data)
{
#ifdef WLAN_ENABLE_QCA6180
	struct CE_state *CE_state;
	A_target_id_t targid;
	u_int32_t ctrl_addr;
	uint32_t tmp;

	adf_os_spin_lock(&scn->target_lock);
	CE_state = scn->ce_id_to_state[CE_id];
	if (!CE_state) {
		HIF_ERROR("%s: error - CE_state = NULL", __func__);
		adf_os_spin_unlock(&scn->target_lock);
		return;
	}
	targid = TARGID(sc);
	ctrl_addr = CE_state->ctrl_addr;
	CE_MSI_ADDR_LOW_SET(scn, ctrl_addr, msi_addr_lo);
	CE_MSI_ADDR_HIGH_SET(scn, ctrl_addr, msi_addr_hi);
	CE_MSI_DATA_SET(scn, ctrl_addr, msi_data);
	tmp = CE_CTRL_REGISTER1_GET(scn, ctrl_addr);
	tmp |= (1 << CE_MSI_ENABLE_BIT);
	CE_CTRL_REGISTER1_SET(scn, ctrl_addr, tmp);
	adf_os_spin_unlock(&scn->target_lock);
#endif
}

#ifdef IPA_OFFLOAD
/*
 * Copy engine should release resource to micro controller
 * Micro controller needs
   - Copy engine source descriptor base address
   - Copy engine source descriptor size
   - PCI BAR address to access copy engine regiser
 */
void ce_ipa_get_resource(struct CE_handle *ce,
			 uint32_t *ce_sr_base_paddr,
			 uint32_t *ce_sr_ring_size,
			 cdf_dma_addr_t *ce_reg_paddr)
{
	struct CE_state *CE_state = (struct CE_state *)ce;
	uint32_t ring_loop;
	struct CE_src_desc *ce_desc;
	cdf_dma_addr_t phy_mem_base;
	struct ol_softc *scn = CE_state->scn;

	if (CE_RUNNING != CE_state->state) {
		*ce_sr_base_paddr = 0;
		*ce_sr_ring_size = 0;
		return;
	}

	/* Update default value for descriptor */
	for (ring_loop = 0; ring_loop < CE_state->src_ring->nentries;
	     ring_loop++) {
		ce_desc = (struct CE_src_desc *)
			  ((char *)CE_state->src_ring->base_addr_owner_space +
			   ring_loop * (sizeof(struct CE_src_desc)));
		CE_IPA_RING_INIT(ce_desc);
	}

	/* Get BAR address */
	hif_read_phy_mem_base(CE_state->scn, &phy_mem_base);

	*ce_sr_base_paddr = (uint32_t) CE_state->src_ring->base_addr_CE_space;
	*ce_sr_ring_size = (uint32_t) CE_state->src_ring->nentries;
	*ce_reg_paddr = phy_mem_base + CE_BASE_ADDRESS(CE_state->id) +
			SR_WR_INDEX_ADDRESS;
	return;
}
#endif /* IPA_OFFLOAD */

