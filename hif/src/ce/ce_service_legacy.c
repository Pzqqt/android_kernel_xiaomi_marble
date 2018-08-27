/*
 * Copyright (c) 2013-2018 The Linux Foundation. All rights reserved.
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

#include "ce_api.h"
#include "ce_internal.h"
#include "ce_main.h"
#include "ce_reg.h"
#include "hif.h"
#include "hif_debug.h"
#include "hif_io32.h"
#include "qdf_lock.h"
#include "hif_main.h"
#include "hif_napi.h"
#include "qdf_module.h"
#include "regtable.h"

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
 * receives an interrupt).
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

/* NB: Modeled after ce_completed_send_next */
/* Shift bits to convert IS_*_RING_*_WATERMARK_MASK to CE_WM_FLAG_*_* */
#define CE_WM_SHFT 1

#ifdef WLAN_FEATURE_FASTPATH
/**
 * ce_is_fastpath_enabled() - returns true if fastpath mode is enabled
 * @scn: Handle to HIF context
 *
 * Return: true if fastpath is enabled else false.
 */
static inline bool ce_is_fastpath_enabled(struct hif_softc *scn)
{
	return scn->fastpath_mode_on;
}
#else
static inline bool ce_is_fastpath_enabled(struct hif_softc *scn)
{
	return false;
}
#endif /* WLAN_FEATURE_FASTPATH */

static int
ce_send_nolock_legacy(struct CE_handle *copyeng,
		      void *per_transfer_context,
		      qdf_dma_addr_t buffer,
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
	struct hif_softc *scn = CE_state->scn;

	if (Q_TARGET_ACCESS_BEGIN(scn) < 0)
		return QDF_STATUS_E_FAILURE;
	if (unlikely(CE_RING_DELTA(nentries_mask,
				   write_index, sw_index - 1) <= 0)) {
		OL_ATH_CE_PKT_ERROR_COUNT_INCR(scn, CE_RING_DELTA_FAIL);
		Q_TARGET_ACCESS_END(scn);
		return QDF_STATUS_E_FAILURE;
	}
	{
		enum hif_ce_event_type event_type;
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
		shadow_src_desc->target_int_disable = 0;
		shadow_src_desc->host_int_disable = 0;

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
		ce_validate_nbytes(nbytes, CE_state);

		*src_desc = *shadow_src_desc;

		src_ring->per_transfer_context[write_index] =
			per_transfer_context;

		/* Update Source Ring Write Index */
		write_index = CE_RING_IDX_INCR(nentries_mask, write_index);

		/* WORKAROUND */
		if (shadow_src_desc->gather) {
			event_type = HIF_TX_GATHER_DESC_POST;
		} else if (qdf_unlikely(CE_state->state != CE_RUNNING)) {
			event_type = HIF_TX_DESC_SOFTWARE_POST;
			CE_state->state = CE_PENDING;
		} else {
			event_type = HIF_TX_DESC_POST;
			war_ce_src_ring_write_idx_set(scn, ctrl_addr,
						      write_index);
		}

		/* src_ring->write index hasn't been updated event though
		 * the register has allready been written to.
		 */
		hif_record_ce_desc_event(scn, CE_state->id, event_type,
			(union ce_desc *)shadow_src_desc, per_transfer_context,
			src_ring->write_index, nbytes);

		src_ring->write_index = write_index;
		status = QDF_STATUS_SUCCESS;
	}
	Q_TARGET_ACCESS_END(scn);
	return status;
}

static int
ce_sendlist_send_legacy(struct CE_handle *copyeng,
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
	struct hif_softc *scn = CE_state->scn;

	QDF_ASSERT((num_items > 0) && (num_items < src_ring->nentries));

	qdf_spin_lock_bh(&CE_state->ce_index_lock);

	if (CE_state->scn->fastpath_mode_on && CE_state->htt_tx_data &&
	    Q_TARGET_ACCESS_BEGIN(scn) == 0) {
		src_ring->sw_index = CE_SRC_RING_READ_IDX_GET_FROM_DDR(
					       scn, CE_state->ctrl_addr);
		Q_TARGET_ACCESS_END(scn);
	}

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
			QDF_ASSERT(item->send_type == CE_SIMPLE_BUFFER_TYPE);
			status = ce_send_nolock_legacy(copyeng,
				CE_SENDLIST_ITEM_CTXT,
				(qdf_dma_addr_t)item->data,
				item->u.nbytes, transfer_id,
				item->flags | CE_SEND_FLAG_GATHER,
				item->user_flags);
			QDF_ASSERT(status == QDF_STATUS_SUCCESS);
		}
		/* provide valid context pointer for final item */
		item = &sl->item[i];
		/* TBDXXX: Support extensible sendlist_types? */
		QDF_ASSERT(item->send_type == CE_SIMPLE_BUFFER_TYPE);
		status = ce_send_nolock_legacy(copyeng, per_transfer_context,
					       (qdf_dma_addr_t) item->data,
					       item->u.nbytes,
					       transfer_id, item->flags,
					       item->user_flags);
		QDF_ASSERT(status == QDF_STATUS_SUCCESS);
		QDF_NBUF_UPDATE_TX_PKT_COUNT((qdf_nbuf_t)per_transfer_context,
					     QDF_NBUF_TX_PKT_CE);
		DPTRACE(qdf_dp_trace((qdf_nbuf_t)per_transfer_context,
			QDF_DP_TRACE_CE_PACKET_PTR_RECORD,
			QDF_TRACE_DEFAULT_PDEV_ID,
			(uint8_t *)&(((qdf_nbuf_t)per_transfer_context)->data),
			sizeof(((qdf_nbuf_t)per_transfer_context)->data),
			QDF_TX));
	} else {
		/*
		 * Probably not worth the additional complexity to support
		 * partial sends with continuation or notification.  We expect
		 * to use large rings and small sendlists. If we can't handle
		 * the entire request at once, punt it back to the caller.
		 */
	}
	qdf_spin_unlock_bh(&CE_state->ce_index_lock);

	return status;
}

/**
 * ce_recv_buf_enqueue_legacy() - enqueue a recv buffer into a copy engine
 * @coyeng: copy engine handle
 * @per_recv_context: virtual address of the nbuf
 * @buffer: physical address of the nbuf
 *
 * Return: 0 if the buffer is enqueued
 */
static int
ce_recv_buf_enqueue_legacy(struct CE_handle *copyeng,
			   void *per_recv_context, qdf_dma_addr_t buffer)
{
	int status;
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	struct CE_ring_state *dest_ring = CE_state->dest_ring;
	uint32_t ctrl_addr = CE_state->ctrl_addr;
	unsigned int nentries_mask = dest_ring->nentries_mask;
	unsigned int write_index;
	unsigned int sw_index;
	uint64_t dma_addr = buffer;
	struct hif_softc *scn = CE_state->scn;

	qdf_spin_lock_bh(&CE_state->ce_index_lock);
	write_index = dest_ring->write_index;
	sw_index = dest_ring->sw_index;

	if (Q_TARGET_ACCESS_BEGIN(scn) < 0) {
		qdf_spin_unlock_bh(&CE_state->ce_index_lock);
		return -EIO;
	}

	if ((CE_RING_DELTA(nentries_mask, write_index, sw_index - 1) > 0) ||
	    (ce_is_fastpath_enabled(scn) && CE_state->htt_rx_data)) {
		struct CE_dest_desc *dest_ring_base =
			(struct CE_dest_desc *)dest_ring->base_addr_owner_space;
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

		hif_record_ce_desc_event(scn, CE_state->id, HIF_RX_DESC_POST,
				(union ce_desc *)dest_desc, per_recv_context,
				write_index, 0);

		/* Update Destination Ring Write Index */
		write_index = CE_RING_IDX_INCR(nentries_mask, write_index);
		if (write_index != sw_index) {
			CE_DEST_RING_WRITE_IDX_SET(scn, ctrl_addr, write_index);
			dest_ring->write_index = write_index;
		}
		status = QDF_STATUS_SUCCESS;
	} else
		status = QDF_STATUS_E_FAILURE;

	Q_TARGET_ACCESS_END(scn);
	qdf_spin_unlock_bh(&CE_state->ce_index_lock);
	return status;
}

static unsigned int
ce_send_entries_done_nolock_legacy(struct hif_softc *scn,
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

static unsigned int
ce_recv_entries_done_nolock_legacy(struct hif_softc *scn,
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

static int
ce_completed_recv_next_nolock_legacy(struct CE_state *CE_state,
				     void **per_CE_contextp,
				     void **per_transfer_contextp,
				     qdf_dma_addr_t *bufferp,
				     unsigned int *nbytesp,
				     unsigned int *transfer_idp,
				     unsigned int *flagsp)
{
	int status;
	struct CE_ring_state *dest_ring = CE_state->dest_ring;
	unsigned int nentries_mask = dest_ring->nentries_mask;
	unsigned int sw_index = dest_ring->sw_index;
	struct hif_softc *scn = CE_state->scn;
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
		status = QDF_STATUS_E_FAILURE;
		goto done;
	}

	hif_record_ce_desc_event(scn, CE_state->id, HIF_RX_DESC_COMPLETION,
				 (union ce_desc *)dest_desc,
				 dest_ring->per_transfer_context[sw_index],
				 sw_index, 0);

	dest_desc->nbytes = 0;

	/* Return data from completed destination descriptor */
	*bufferp = HIF_CE_DESC_ADDR_TO_DMA(&dest_desc_info);
	*nbytesp = nbytes;
	*transfer_idp = dest_desc_info.meta_data;
	*flagsp = (dest_desc_info.byte_swap) ? CE_RECV_FLAG_SWAPPED : 0;

	if (per_CE_contextp)
		*per_CE_contextp = CE_state->recv_context;

	if (per_transfer_contextp) {
		*per_transfer_contextp =
			dest_ring->per_transfer_context[sw_index];
	}
	dest_ring->per_transfer_context[sw_index] = 0;  /* sanity */

	/* Update sw_index */
	sw_index = CE_RING_IDX_INCR(nentries_mask, sw_index);
	dest_ring->sw_index = sw_index;
	status = QDF_STATUS_SUCCESS;

done:
	return status;
}

/* NB: Modeled after ce_completed_recv_next_nolock */
static QDF_STATUS
ce_revoke_recv_next_legacy(struct CE_handle *copyeng,
			   void **per_CE_contextp,
			   void **per_transfer_contextp,
			   qdf_dma_addr_t *bufferp)
{
	struct CE_state *CE_state;
	struct CE_ring_state *dest_ring;
	unsigned int nentries_mask;
	unsigned int sw_index;
	unsigned int write_index;
	QDF_STATUS status;
	struct hif_softc *scn;

	CE_state = (struct CE_state *)copyeng;
	dest_ring = CE_state->dest_ring;
	if (!dest_ring)
		return QDF_STATUS_E_FAILURE;

	scn = CE_state->scn;
	qdf_spin_lock(&CE_state->ce_index_lock);
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

		if (per_CE_contextp)
			*per_CE_contextp = CE_state->recv_context;

		if (per_transfer_contextp) {
			*per_transfer_contextp =
				dest_ring->per_transfer_context[sw_index];
		}
		dest_ring->per_transfer_context[sw_index] = 0;  /* sanity */

		/* Update sw_index */
		sw_index = CE_RING_IDX_INCR(nentries_mask, sw_index);
		dest_ring->sw_index = sw_index;
		status = QDF_STATUS_SUCCESS;
	} else {
		status = QDF_STATUS_E_FAILURE;
	}
	qdf_spin_unlock(&CE_state->ce_index_lock);

	return status;
}

/*
 * Guts of ce_completed_send_next.
 * The caller takes responsibility for any necessary locking.
 */
static int
ce_completed_send_next_nolock_legacy(struct CE_state *CE_state,
				     void **per_CE_contextp,
				     void **per_transfer_contextp,
				     qdf_dma_addr_t *bufferp,
				     unsigned int *nbytesp,
				     unsigned int *transfer_idp,
				     unsigned int *sw_idx,
				     unsigned int *hw_idx,
				     uint32_t *toeplitz_hash_result)
{
	int status = QDF_STATUS_E_FAILURE;
	struct CE_ring_state *src_ring = CE_state->src_ring;
	uint32_t ctrl_addr = CE_state->ctrl_addr;
	unsigned int nentries_mask = src_ring->nentries_mask;
	unsigned int sw_index = src_ring->sw_index;
	unsigned int read_index;
	struct hif_softc *scn = CE_state->scn;

	if (src_ring->hw_index == sw_index) {
		/*
		 * The SW completion index has caught up with the cached
		 * version of the HW completion index.
		 * Update the cached HW completion index to see whether
		 * the SW has really caught up to the HW, or if the cached
		 * value of the HW index has become stale.
		 */
		if (Q_TARGET_ACCESS_BEGIN(scn) < 0)
			return QDF_STATUS_E_FAILURE;
		src_ring->hw_index =
			CE_SRC_RING_READ_IDX_GET_FROM_DDR(scn, ctrl_addr);
		if (Q_TARGET_ACCESS_END(scn) < 0)
			return QDF_STATUS_E_FAILURE;
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
		hif_record_ce_desc_event(scn, CE_state->id,
				HIF_TX_DESC_COMPLETION,
				(union ce_desc *)shadow_src_desc,
				src_ring->per_transfer_context[sw_index],
				sw_index, shadow_src_desc->nbytes);

		/* Return data from completed source descriptor */
		*bufferp = HIF_CE_DESC_ADDR_TO_DMA(shadow_src_desc);
		*nbytesp = shadow_src_desc->nbytes;
		*transfer_idp = shadow_src_desc->meta_data;
#ifdef QCA_WIFI_3_0
		*toeplitz_hash_result = src_desc->toeplitz_hash_result;
#else
		*toeplitz_hash_result = 0;
#endif
		if (per_CE_contextp)
			*per_CE_contextp = CE_state->send_context;

		if (per_transfer_contextp) {
			*per_transfer_contextp =
				src_ring->per_transfer_context[sw_index];
		}
		src_ring->per_transfer_context[sw_index] = 0;   /* sanity */

		/* Update sw_index */
		sw_index = CE_RING_IDX_INCR(nentries_mask, sw_index);
		src_ring->sw_index = sw_index;
		status = QDF_STATUS_SUCCESS;
	}

	return status;
}

static QDF_STATUS
ce_cancel_send_next_legacy(struct CE_handle *copyeng,
			   void **per_CE_contextp,
			   void **per_transfer_contextp,
			   qdf_dma_addr_t *bufferp,
			   unsigned int *nbytesp,
			   unsigned int *transfer_idp,
			   uint32_t *toeplitz_hash_result)
{
	struct CE_state *CE_state;
	struct CE_ring_state *src_ring;
	unsigned int nentries_mask;
	unsigned int sw_index;
	unsigned int write_index;
	QDF_STATUS status;
	struct hif_softc *scn;

	CE_state = (struct CE_state *)copyeng;
	src_ring = CE_state->src_ring;
	if (!src_ring)
		return QDF_STATUS_E_FAILURE;

	scn = CE_state->scn;
	qdf_spin_lock(&CE_state->ce_index_lock);
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

		if (per_CE_contextp)
			*per_CE_contextp = CE_state->send_context;

		if (per_transfer_contextp) {
			*per_transfer_contextp =
				src_ring->per_transfer_context[sw_index];
		}
		src_ring->per_transfer_context[sw_index] = 0;   /* sanity */

		/* Update sw_index */
		sw_index = CE_RING_IDX_INCR(nentries_mask, sw_index);
		src_ring->sw_index = sw_index;
		status = QDF_STATUS_SUCCESS;
	} else {
		status = QDF_STATUS_E_FAILURE;
	}
	qdf_spin_unlock(&CE_state->ce_index_lock);

	return status;
}

/*
 * Adjust interrupts for the copy complete handler.
 * If it's needed for either send or recv, then unmask
 * this interrupt; otherwise, mask it.
 *
 * Called with target_lock held.
 */
static void
ce_per_engine_handler_adjust_legacy(struct CE_state *CE_state,
				    int disable_copy_compl_intr)
{
	uint32_t ctrl_addr = CE_state->ctrl_addr;
	struct hif_softc *scn = CE_state->scn;

	CE_state->disable_copy_compl_intr = disable_copy_compl_intr;

	if (Q_TARGET_ACCESS_BEGIN(scn) < 0)
		return;

	if (!TARGET_REGISTER_ACCESS_ALLOWED(scn)) {
		hif_err_rl("%s: target access is not allowed", __func__);
		return;
	}

	if ((!disable_copy_compl_intr) &&
	    (CE_state->send_cb || CE_state->recv_cb))
		CE_COPY_COMPLETE_INTR_ENABLE(scn, ctrl_addr);
	else
		CE_COPY_COMPLETE_INTR_DISABLE(scn, ctrl_addr);

	if (CE_state->watermark_cb)
		CE_WATERMARK_INTR_ENABLE(scn, ctrl_addr);
	else
		CE_WATERMARK_INTR_DISABLE(scn, ctrl_addr);
	Q_TARGET_ACCESS_END(scn);
}

static void ce_legacy_src_ring_setup(struct hif_softc *scn, uint32_t ce_id,
				     struct CE_ring_state *src_ring,
				     struct CE_attr *attr)
{
	uint32_t ctrl_addr;
	uint64_t dma_addr;

	QDF_ASSERT(ce_id < scn->ce_count);
	ctrl_addr = CE_BASE_ADDRESS(ce_id);

	src_ring->hw_index =
		CE_SRC_RING_READ_IDX_GET_FROM_REGISTER(scn, ctrl_addr);
	src_ring->sw_index = src_ring->hw_index;
	src_ring->write_index =
		CE_SRC_RING_WRITE_IDX_GET_FROM_REGISTER(scn, ctrl_addr);
	dma_addr = src_ring->base_addr_CE_space;
	CE_SRC_RING_BASE_ADDR_SET(scn, ctrl_addr,
				  (uint32_t)(dma_addr & 0xFFFFFFFF));

	/* if SR_BA_ADDRESS_HIGH register exists */
	if (is_register_supported(SR_BA_ADDRESS_HIGH)) {
		uint32_t tmp;

		tmp = CE_SRC_RING_BASE_ADDR_HIGH_GET(
				scn, ctrl_addr);
		tmp &= ~0x1F;
		dma_addr = ((dma_addr >> 32) & 0x1F) | tmp;
		CE_SRC_RING_BASE_ADDR_HIGH_SET(scn,
					ctrl_addr, (uint32_t)dma_addr);
	}
	CE_SRC_RING_SZ_SET(scn, ctrl_addr, src_ring->nentries);
	CE_SRC_RING_DMAX_SET(scn, ctrl_addr, attr->src_sz_max);
#ifdef BIG_ENDIAN_HOST
	/* Enable source ring byte swap for big endian host */
	CE_SRC_RING_BYTE_SWAP_SET(scn, ctrl_addr, 1);
#endif
	CE_SRC_RING_LOWMARK_SET(scn, ctrl_addr, 0);
	CE_SRC_RING_HIGHMARK_SET(scn, ctrl_addr, src_ring->nentries);
}

static void ce_legacy_dest_ring_setup(struct hif_softc *scn, uint32_t ce_id,
				struct CE_ring_state *dest_ring,
				struct CE_attr *attr)
{
	uint32_t ctrl_addr;
	uint64_t dma_addr;

	QDF_ASSERT(ce_id < scn->ce_count);
	ctrl_addr = CE_BASE_ADDRESS(ce_id);
	dest_ring->sw_index =
		CE_DEST_RING_READ_IDX_GET_FROM_REGISTER(scn, ctrl_addr);
	dest_ring->write_index =
		CE_DEST_RING_WRITE_IDX_GET_FROM_REGISTER(scn, ctrl_addr);
	dma_addr = dest_ring->base_addr_CE_space;
	CE_DEST_RING_BASE_ADDR_SET(scn, ctrl_addr,
				   (uint32_t)(dma_addr & 0xFFFFFFFF));

	/* if DR_BA_ADDRESS_HIGH exists */
	if (is_register_supported(DR_BA_ADDRESS_HIGH)) {
		uint32_t tmp;

		tmp = CE_DEST_RING_BASE_ADDR_HIGH_GET(scn,
						      ctrl_addr);
		tmp &= ~0x1F;
		dma_addr = ((dma_addr >> 32) & 0x1F) | tmp;
		CE_DEST_RING_BASE_ADDR_HIGH_SET(scn,
				ctrl_addr, (uint32_t)dma_addr);
	}

	CE_DEST_RING_SZ_SET(scn, ctrl_addr, dest_ring->nentries);
#ifdef BIG_ENDIAN_HOST
	/* Enable Dest ring byte swap for big endian host */
	CE_DEST_RING_BYTE_SWAP_SET(scn, ctrl_addr, 1);
#endif
	CE_DEST_RING_LOWMARK_SET(scn, ctrl_addr, 0);
	CE_DEST_RING_HIGHMARK_SET(scn, ctrl_addr, dest_ring->nentries);
}

static uint32_t ce_get_desc_size_legacy(uint8_t ring_type)
{
	switch (ring_type) {
	case CE_RING_SRC:
		return sizeof(struct CE_src_desc);
	case CE_RING_DEST:
		return sizeof(struct CE_dest_desc);
	case CE_RING_STATUS:
		qdf_assert(0);
		return 0;
	default:
		return 0;
	}

	return 0;
}

static int ce_ring_setup_legacy(struct hif_softc *scn, uint8_t ring_type,
				uint32_t ce_id, struct CE_ring_state *ring,
				struct CE_attr *attr)
{
	int status = Q_TARGET_ACCESS_BEGIN(scn);

	if (status < 0)
		goto out;

	switch (ring_type) {
	case CE_RING_SRC:
		ce_legacy_src_ring_setup(scn, ce_id, ring, attr);
		break;
	case CE_RING_DEST:
		ce_legacy_dest_ring_setup(scn, ce_id, ring, attr);
		break;
	case CE_RING_STATUS:
	default:
		qdf_assert(0);
		break;
	}

	Q_TARGET_ACCESS_END(scn);
out:
	return status;
}

static void ce_prepare_shadow_register_v2_cfg_legacy(struct hif_softc *scn,
			    struct pld_shadow_reg_v2_cfg **shadow_config,
			    int *num_shadow_registers_configured)
{
	*num_shadow_registers_configured = 0;
	*shadow_config = NULL;
}

static bool ce_check_int_watermark(struct CE_state *CE_state,
				   unsigned int *flags)
{
	uint32_t ce_int_status;
	uint32_t ctrl_addr = CE_state->ctrl_addr;
	struct hif_softc *scn = CE_state->scn;

	ce_int_status = CE_ENGINE_INT_STATUS_GET(scn, ctrl_addr);
	if (ce_int_status & CE_WATERMARK_MASK) {
		/* Convert HW IS bits to software flags */
		*flags =
			(ce_int_status & CE_WATERMARK_MASK) >>
			CE_WM_SHFT;
		return true;
	}

	return false;
}

struct ce_ops ce_service_legacy = {
	.ce_get_desc_size = ce_get_desc_size_legacy,
	.ce_ring_setup = ce_ring_setup_legacy,
	.ce_sendlist_send = ce_sendlist_send_legacy,
	.ce_completed_recv_next_nolock = ce_completed_recv_next_nolock_legacy,
	.ce_revoke_recv_next = ce_revoke_recv_next_legacy,
	.ce_cancel_send_next = ce_cancel_send_next_legacy,
	.ce_recv_buf_enqueue = ce_recv_buf_enqueue_legacy,
	.ce_per_engine_handler_adjust = ce_per_engine_handler_adjust_legacy,
	.ce_send_nolock = ce_send_nolock_legacy,
	.watermark_int = ce_check_int_watermark,
	.ce_completed_send_next_nolock = ce_completed_send_next_nolock_legacy,
	.ce_recv_entries_done_nolock = ce_recv_entries_done_nolock_legacy,
	.ce_send_entries_done_nolock = ce_send_entries_done_nolock_legacy,
	.ce_prepare_shadow_register_v2_cfg =
		ce_prepare_shadow_register_v2_cfg_legacy,
};

struct ce_ops *ce_services_legacy()
{
	return &ce_service_legacy;
}

qdf_export_symbol(ce_services_legacy);

void ce_service_legacy_init(void)
{
	ce_service_register_module(CE_SVC_LEGACY, &ce_services_legacy);
}
