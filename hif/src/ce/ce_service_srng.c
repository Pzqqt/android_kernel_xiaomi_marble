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

#include "hif.h"
#include "hif_io32.h"
#include "reg_struct.h"
#include "ce_api.h"
#include "ce_main.h"
#include "ce_internal.h"
#include "ce_reg.h"
#include "qdf_lock.h"
#include "regtable.h"
#include "hif_main.h"
#include "hif_debug.h"
#include "hal_api.h"
#include "pld_common.h"

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

#define CE_ADDR_COPY(desc, dma_addr) do {\
		(desc)->buffer_addr_lo = (uint32_t)((dma_addr) &\
							  0xFFFFFFFF);\
		(desc)->buffer_addr_hi =\
			(uint32_t)(((dma_addr) >> 32) & 0xFF);\
	} while (0)

static int
ce_send_nolock_srng(struct CE_handle *copyeng,
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
	unsigned int nentries_mask = src_ring->nentries_mask;
	unsigned int write_index = src_ring->write_index;
	uint64_t dma_addr = buffer;
	struct hif_softc *scn = CE_state->scn;

	if (Q_TARGET_ACCESS_BEGIN(scn) < 0)
		return QDF_STATUS_E_FAILURE;
	if (unlikely(hal_srng_src_num_avail(scn->hal_soc, src_ring->srng_ctx,
					false) <= 0)) {
		OL_ATH_CE_PKT_ERROR_COUNT_INCR(scn, CE_RING_DELTA_FAIL);
		Q_TARGET_ACCESS_END(scn);
		return QDF_STATUS_E_FAILURE;
	}
	{
		enum hif_ce_event_type event_type = HIF_TX_GATHER_DESC_POST;
		struct ce_srng_src_desc *src_desc;

		if (hal_srng_access_start(scn->hal_soc, src_ring->srng_ctx)) {
			Q_TARGET_ACCESS_END(scn);
			return QDF_STATUS_E_FAILURE;
		}

		src_desc = hal_srng_src_get_next_reaped(scn->hal_soc,
				src_ring->srng_ctx);

		/* Update low 32 bits source descriptor address */
		src_desc->buffer_addr_lo =
			(uint32_t)(dma_addr & 0xFFFFFFFF);
		src_desc->buffer_addr_hi =
			(uint32_t)((dma_addr >> 32) & 0xFF);

		src_desc->meta_data = transfer_id;

		/*
		 * Set the swap bit if:
		 * typical sends on this CE are swapped (host is big-endian)
		 * and this send doesn't disable the swapping
		 * (data is not bytestream)
		 */
		src_desc->byte_swap =
			(((CE_state->attr_flags & CE_ATTR_BYTE_SWAP_DATA)
			  != 0) & ((flags & CE_SEND_FLAG_SWAP_DISABLE) == 0));
		src_desc->gather = ((flags & CE_SEND_FLAG_GATHER) != 0);
		src_desc->nbytes = nbytes;

		src_ring->per_transfer_context[write_index] =
			per_transfer_context;
		write_index = CE_RING_IDX_INCR(nentries_mask, write_index);

		hal_srng_access_end(scn->hal_soc, src_ring->srng_ctx);

		/* src_ring->write index hasn't been updated event though
		 * the register has allready been written to.
		 */
		hif_record_ce_desc_event(scn, CE_state->id, event_type,
			(union ce_desc *) src_desc, per_transfer_context,
			src_ring->write_index);

		src_ring->write_index = write_index;
		status = QDF_STATUS_SUCCESS;
	}
	Q_TARGET_ACCESS_END(scn);
	return status;
}

static int
ce_sendlist_send_srng(struct CE_handle *copyeng,
		 void *per_transfer_context,
		 struct ce_sendlist *sendlist, unsigned int transfer_id)
{
	int status = -ENOMEM;
	struct ce_sendlist_s *sl = (struct ce_sendlist_s *)sendlist;
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	struct CE_ring_state *src_ring = CE_state->src_ring;
	unsigned int num_items = sl->num_items;
	unsigned int sw_index;
	unsigned int write_index;
	struct hif_softc *scn = CE_state->scn;

	QDF_ASSERT((num_items > 0) && (num_items < src_ring->nentries));

	qdf_spin_lock_bh(&CE_state->ce_index_lock);
	sw_index = src_ring->sw_index;
	write_index = src_ring->write_index;

	if (hal_srng_src_num_avail(scn->hal_soc, src_ring->srng_ctx, false) >=
	    num_items) {
		struct ce_sendlist_item *item;
		int i;

		/* handle all but the last item uniformly */
		for (i = 0; i < num_items - 1; i++) {
			item = &sl->item[i];
			/* TBDXXX: Support extensible sendlist_types? */
			QDF_ASSERT(item->send_type == CE_SIMPLE_BUFFER_TYPE);
			status = ce_send_nolock_srng(copyeng,
					CE_SENDLIST_ITEM_CTXT,
				(qdf_dma_addr_t) item->data,
				item->u.nbytes, transfer_id,
				item->flags | CE_SEND_FLAG_GATHER,
				item->user_flags);
			QDF_ASSERT(status == QDF_STATUS_SUCCESS);
		}
		/* provide valid context pointer for final item */
		item = &sl->item[i];
		/* TBDXXX: Support extensible sendlist_types? */
		QDF_ASSERT(item->send_type == CE_SIMPLE_BUFFER_TYPE);
		status = ce_send_nolock_srng(copyeng, per_transfer_context,
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
			(uint8_t *)(((qdf_nbuf_t)per_transfer_context)->data),
			sizeof(((qdf_nbuf_t)per_transfer_context)->data), QDF_TX));
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

#define SLOTS_PER_DATAPATH_TX 2

#ifndef AH_NEED_TX_DATA_SWAP
#define AH_NEED_TX_DATA_SWAP 0
#endif
/**
 * ce_recv_buf_enqueue_srng() - enqueue a recv buffer into a copy engine
 * @coyeng: copy engine handle
 * @per_recv_context: virtual address of the nbuf
 * @buffer: physical address of the nbuf
 *
 * Return: 0 if the buffer is enqueued
 */
static int
ce_recv_buf_enqueue_srng(struct CE_handle *copyeng,
		    void *per_recv_context, qdf_dma_addr_t buffer)
{
	int status;
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	struct CE_ring_state *dest_ring = CE_state->dest_ring;
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

	if (hal_srng_access_start(scn->hal_soc, dest_ring->srng_ctx)) {
		qdf_spin_unlock_bh(&CE_state->ce_index_lock);
		return QDF_STATUS_E_FAILURE;
	}

	if ((hal_srng_src_num_avail(scn->hal_soc,
					dest_ring->srng_ctx, false) > 0)) {
		struct ce_srng_dest_desc *dest_desc =
				hal_srng_src_get_next(scn->hal_soc,
							dest_ring->srng_ctx);

		if (dest_desc == NULL) {
			status = QDF_STATUS_E_FAILURE;
		} else {

			CE_ADDR_COPY(dest_desc, dma_addr);

			dest_ring->per_transfer_context[write_index] =
				per_recv_context;

			/* Update Destination Ring Write Index */
			write_index = CE_RING_IDX_INCR(nentries_mask,
								write_index);
			status = QDF_STATUS_SUCCESS;
		}
	} else
		status = QDF_STATUS_E_FAILURE;

	dest_ring->write_index = write_index;
	hal_srng_access_end(scn->hal_soc, dest_ring->srng_ctx);
	Q_TARGET_ACCESS_END(scn);
	qdf_spin_unlock_bh(&CE_state->ce_index_lock);
	return status;
}

/*
 * Guts of ce_recv_entries_done.
 * The caller takes responsibility for any necessary locking.
 */
static unsigned int
ce_recv_entries_done_nolock_srng(struct hif_softc *scn,
			    struct CE_state *CE_state)
{
	struct CE_ring_state *status_ring = CE_state->status_ring;

	return hal_srng_dst_num_valid(scn->hal_soc,
				status_ring->srng_ctx, false);
}

/*
 * Guts of ce_send_entries_done.
 * The caller takes responsibility for any necessary locking.
 */
static unsigned int
ce_send_entries_done_nolock_srng(struct hif_softc *scn,
					struct CE_state *CE_state)
{

	struct CE_ring_state *src_ring = CE_state->src_ring;
	int count = 0;

	if (hal_srng_access_start(scn->hal_soc, src_ring->srng_ctx))
		return 0;

	count = hal_srng_src_done_val(scn->hal_soc, src_ring->srng_ctx);

	hal_srng_access_end(scn->hal_soc, src_ring->srng_ctx);

	return count;
}

/*
 * Guts of ce_completed_recv_next.
 * The caller takes responsibility for any necessary locking.
 */
static int
ce_completed_recv_next_nolock_srng(struct CE_state *CE_state,
			      void **per_CE_contextp,
			      void **per_transfer_contextp,
			      qdf_dma_addr_t *bufferp,
			      unsigned int *nbytesp,
			      unsigned int *transfer_idp,
			      unsigned int *flagsp)
{
	int status;
	struct CE_ring_state *dest_ring = CE_state->dest_ring;
	struct CE_ring_state *status_ring = CE_state->status_ring;
	unsigned int nentries_mask = dest_ring->nentries_mask;
	unsigned int sw_index = dest_ring->sw_index;
	struct hif_softc *scn = CE_state->scn;
	struct ce_srng_dest_status_desc *dest_status;
	int nbytes;
	struct ce_srng_dest_status_desc dest_status_info;

	if (hal_srng_access_start(scn->hal_soc, status_ring->srng_ctx)) {
		status = QDF_STATUS_E_FAILURE;
		goto done;
	}

	dest_status = hal_srng_dst_get_next(scn->hal_soc,
						status_ring->srng_ctx);

	if (dest_status == NULL) {
		status = QDF_STATUS_E_FAILURE;
		goto done;
	}
	/*
	 * By copying the dest_desc_info element to local memory, we could
	 * avoid extra memory read from non-cachable memory.
	 */
	dest_status_info = *dest_status;
	nbytes = dest_status_info.nbytes;
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

	dest_status->nbytes = 0;

	*nbytesp = nbytes;
	*transfer_idp = dest_status_info.meta_data;
	*flagsp = (dest_status_info.byte_swap) ? CE_RECV_FLAG_SWAPPED : 0;

	if (per_CE_contextp)
		*per_CE_contextp = CE_state->recv_context;

	/* NOTE: sw_index is more like a read_index in this context. It has a
	 * one-to-one mapping with status ring.
	 * Get the per trasnfer context from dest_ring.
	 */
	if (per_transfer_contextp)
		*per_transfer_contextp =
			dest_ring->per_transfer_context[sw_index];

	dest_ring->per_transfer_context[sw_index] = 0;  /* sanity */

	/* Update sw_index */
	sw_index = CE_RING_IDX_INCR(nentries_mask, sw_index);
	dest_ring->sw_index = sw_index;
	status = QDF_STATUS_SUCCESS;

done:
	hal_srng_access_end(scn->hal_soc, status_ring->srng_ctx);

	return status;
}

static QDF_STATUS
ce_revoke_recv_next_srng(struct CE_handle *copyeng,
		    void **per_CE_contextp,
		    void **per_transfer_contextp, qdf_dma_addr_t *bufferp)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	struct CE_ring_state *dest_ring = CE_state->dest_ring;
	unsigned int sw_index;

	if (!dest_ring)
		return QDF_STATUS_E_FAILURE;

	sw_index = dest_ring->sw_index;

	if (per_CE_contextp)
		*per_CE_contextp = CE_state->recv_context;

	/* NOTE: sw_index is more like a read_index in this context. It has a
	 * one-to-one mapping with status ring.
	 * Get the per trasnfer context from dest_ring.
	 */
	if (per_transfer_contextp)
		*per_transfer_contextp =
			dest_ring->per_transfer_context[sw_index];

	if (dest_ring->per_transfer_context[sw_index] == NULL)
		return QDF_STATUS_E_FAILURE;

	/* provide end condition */
	dest_ring->per_transfer_context[sw_index] = NULL;

	/* Update sw_index */
	sw_index = CE_RING_IDX_INCR(dest_ring->nentries_mask, sw_index);
	dest_ring->sw_index = sw_index;
	return QDF_STATUS_SUCCESS;
}

/*
 * Guts of ce_completed_send_next.
 * The caller takes responsibility for any necessary locking.
 */
static int
ce_completed_send_next_nolock_srng(struct CE_state *CE_state,
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
	unsigned int nentries_mask = src_ring->nentries_mask;
	unsigned int sw_index = src_ring->sw_index;
	struct hif_softc *scn = CE_state->scn;
	struct ce_srng_src_desc *src_desc;

	if (hal_srng_access_start(scn->hal_soc, src_ring->srng_ctx)) {
		status = QDF_STATUS_E_FAILURE;
		return status;
	}

	src_desc = hal_srng_src_reap_next(scn->hal_soc, src_ring->srng_ctx);
	if (src_desc) {

		/* Return data from completed source descriptor */
		*bufferp = (qdf_dma_addr_t)
			(((uint64_t)(src_desc)->buffer_addr_lo +
			  ((uint64_t)((src_desc)->buffer_addr_hi &
				  0xFF) << 32)));
		*nbytesp = src_desc->nbytes;
		*transfer_idp = src_desc->meta_data;
		*toeplitz_hash_result = 0; /*src_desc->toeplitz_hash_result;*/

		if (per_CE_contextp)
			*per_CE_contextp = CE_state->send_context;

		/* sw_index is used more like read index */
		if (per_transfer_contextp)
			*per_transfer_contextp =
				src_ring->per_transfer_context[sw_index];

		src_ring->per_transfer_context[sw_index] = 0;   /* sanity */

		/* Update sw_index */
		sw_index = CE_RING_IDX_INCR(nentries_mask, sw_index);
		src_ring->sw_index = sw_index;
		status = QDF_STATUS_SUCCESS;
	}
	hal_srng_access_end_reap(scn->hal_soc, src_ring->srng_ctx);

	return status;
}

/* NB: Modelled after ce_completed_send_next */
static QDF_STATUS
ce_cancel_send_next_srng(struct CE_handle *copyeng,
		void **per_CE_contextp,
		void **per_transfer_contextp,
		qdf_dma_addr_t *bufferp,
		unsigned int *nbytesp,
		unsigned int *transfer_idp,
		uint32_t *toeplitz_hash_result)
{
	return QDF_STATUS_E_INVAL;
}

/* Shift bits to convert IS_*_RING_*_WATERMARK_MASK to CE_WM_FLAG_*_* */
#define CE_WM_SHFT 1

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
 * Adjust interrupts for the copy complete handler.
 * If it's needed for either send or recv, then unmask
 * this interrupt; otherwise, mask it.
 *
 * Called with target_lock held.
 */
static void
ce_per_engine_handler_adjust_srng(struct CE_state *CE_state,
			     int disable_copy_compl_intr)
{
}

static bool ce_check_int_watermark_srng(struct CE_state *CE_state,
					unsigned int *flags)
{
	/*TODO*/
	return false;
}

static uint32_t ce_get_desc_size_srng(uint8_t ring_type)
{
	switch (ring_type) {
	case CE_RING_SRC:
		return sizeof(struct ce_srng_src_desc);
	case CE_RING_DEST:
		return sizeof(struct ce_srng_dest_desc);
	case CE_RING_STATUS:
		return sizeof(struct ce_srng_dest_status_desc);
	default:
		return 0;
	}
	return 0;
}

static void ce_srng_msi_ring_params_setup(struct hif_softc *scn, uint32_t ce_id,
			      struct hal_srng_params *ring_params)
{
	uint32_t addr_low;
	uint32_t addr_high;
	uint32_t msi_data_start;
	uint32_t msi_data_count;
	uint32_t msi_irq_start;
	int ret;

	ret = pld_get_user_msi_assignment(scn->qdf_dev->dev, "CE",
					  &msi_data_count, &msi_data_start,
					  &msi_irq_start);

	/* msi config not found */
	if (ret)
		return;

	HIF_INFO("%s: ce_id %d, msi_start: %d, msi_count %d", __func__, ce_id,
		  msi_data_start, msi_data_count);

	pld_get_msi_address(scn->qdf_dev->dev, &addr_low, &addr_high);

	ring_params->msi_addr = addr_low;
	ring_params->msi_addr |= (qdf_dma_addr_t)(((uint64_t)addr_high) << 32);
	ring_params->msi_data = (ce_id % msi_data_count) + msi_data_start;
	ring_params->flags |= HAL_SRNG_MSI_INTR;

	HIF_INFO("%s: ce_id %d, msi_addr %p, msi_data %d", __func__, ce_id,
		  (void *)ring_params->msi_addr, ring_params->msi_data);
}

static void ce_srng_src_ring_setup(struct hif_softc *scn, uint32_t ce_id,
			struct CE_ring_state *src_ring)
{
	struct hal_srng_params ring_params = {0};

	HIF_INFO("%s: ce_id %d", __func__, ce_id);

	ce_srng_msi_ring_params_setup(scn, ce_id, &ring_params);

	ring_params.ring_base_paddr = src_ring->base_addr_CE_space;
	ring_params.ring_base_vaddr = src_ring->base_addr_owner_space;
	ring_params.num_entries = src_ring->nentries;
	/*
	 * The minimum increment for the timer is 8us
	 * A default value of 0 disables the timer
	 * A valid default value caused continuous interrupts to
	 * fire with MSI enabled. Need to revisit usage of the timer
	 */
	ring_params.intr_timer_thres_us = 0;
	ring_params.intr_batch_cntr_thres_entries = 1;

	/* TODO
	 * ring_params.msi_addr = XXX;
	 * ring_params.msi_data = XXX;
	 * ring_params.flags = XXX;
	 */

	src_ring->srng_ctx = hal_srng_setup(scn->hal_soc, CE_SRC, ce_id, 0,
			&ring_params);
}

static void ce_srng_dest_ring_setup(struct hif_softc *scn, uint32_t ce_id,
				struct CE_ring_state *dest_ring,
				struct CE_attr *attr)
{
	struct hal_srng_params ring_params = {0};

	HIF_INFO("%s: ce_id %d", __func__, ce_id);

	ce_srng_msi_ring_params_setup(scn, ce_id, &ring_params);

	ring_params.ring_base_paddr = dest_ring->base_addr_CE_space;
	ring_params.ring_base_vaddr = dest_ring->base_addr_owner_space;
	ring_params.num_entries = dest_ring->nentries;
	ring_params.intr_timer_thres_us = 0;
	ring_params.intr_batch_cntr_thres_entries = 1;
	ring_params.max_buffer_length = attr->src_sz_max;

	/* TODO
	 * ring_params.msi_addr = XXX;
	 * ring_params.msi_data = XXX;
	 * ring_params.flags = XXX;
	 */

	/*Dest ring is also source ring*/
	dest_ring->srng_ctx = hal_srng_setup(scn->hal_soc, CE_DST, ce_id, 0,
			&ring_params);
}

static void ce_srng_status_ring_setup(struct hif_softc *scn, uint32_t ce_id,
				struct CE_ring_state *status_ring)
{
	struct hal_srng_params ring_params = {0};

	HIF_INFO("%s: ce_id %d", __func__, ce_id);

	ce_srng_msi_ring_params_setup(scn, ce_id, &ring_params);

	ring_params.ring_base_paddr = status_ring->base_addr_CE_space;
	ring_params.ring_base_vaddr = status_ring->base_addr_owner_space;
	ring_params.num_entries = status_ring->nentries;
	ring_params.intr_timer_thres_us = 0;
	ring_params.intr_batch_cntr_thres_entries = 1;

	/* TODO
	 * ring_params.msi_addr = XXX;
	 * ring_params.msi_data = XXX;
	 * ring_params.flags = XXX;
	 */

	status_ring->srng_ctx = hal_srng_setup(scn->hal_soc, CE_DST_STATUS,
			ce_id, 0, &ring_params);
}

static void ce_ring_setup_srng(struct hif_softc *scn, uint8_t ring_type,
		uint32_t ce_id, struct CE_ring_state *ring,
		struct CE_attr *attr)
{
	switch (ring_type) {
	case CE_RING_SRC:
		ce_srng_src_ring_setup(scn, ce_id, ring);
		break;
	case CE_RING_DEST:
		ce_srng_dest_ring_setup(scn, ce_id, ring, attr);
		break;
	case CE_RING_STATUS:
		ce_srng_status_ring_setup(scn, ce_id, ring);
		break;
	default:
		qdf_assert(0);
		break;
	}
}

static void ce_construct_shadow_config_srng(struct hif_softc *scn)
{
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);
	int ce_id;

	for (ce_id = 0; ce_id < scn->ce_count; ce_id++) {
		if (hif_state->host_ce_config[ce_id].src_nentries)
			hal_set_one_shadow_config(scn->hal_soc,
						  CE_SRC, ce_id);

		if (hif_state->host_ce_config[ce_id].dest_nentries) {
			hal_set_one_shadow_config(scn->hal_soc,
						  CE_DST, ce_id);

			hal_set_one_shadow_config(scn->hal_soc,
						  CE_DST_STATUS, ce_id);
		}
	}
}

static void ce_prepare_shadow_register_v2_cfg_srng(struct hif_softc *scn,
		struct pld_shadow_reg_v2_cfg **shadow_config,
		int *num_shadow_registers_configured)
{
	if (scn->hal_soc == NULL) {
		HIF_ERROR("%s: hal not initialized: not initializing shadow config",
			  __func__);
		return;
	}

	hal_get_shadow_config(scn->hal_soc, shadow_config,
			      num_shadow_registers_configured);

	if (*num_shadow_registers_configured != 0) {
		HIF_ERROR("%s: hal shadow register configuration allready constructed",
			  __func__);

		/* return with original configuration*/
		return;
	}

	hal_construct_shadow_config(scn->hal_soc);
	ce_construct_shadow_config_srng(scn);

	/* get updated configuration */
	hal_get_shadow_config(scn->hal_soc, shadow_config,
			      num_shadow_registers_configured);
}

static struct ce_ops ce_service_srng = {
	.ce_get_desc_size = ce_get_desc_size_srng,
	.ce_ring_setup = ce_ring_setup_srng,
	.ce_sendlist_send = ce_sendlist_send_srng,
	.ce_completed_recv_next_nolock = ce_completed_recv_next_nolock_srng,
	.ce_revoke_recv_next = ce_revoke_recv_next_srng,
	.ce_cancel_send_next = ce_cancel_send_next_srng,
	.ce_recv_buf_enqueue = ce_recv_buf_enqueue_srng,
	.ce_per_engine_handler_adjust = ce_per_engine_handler_adjust_srng,
	.ce_send_nolock = ce_send_nolock_srng,
	.watermark_int = ce_check_int_watermark_srng,
	.ce_completed_send_next_nolock = ce_completed_send_next_nolock_srng,
	.ce_recv_entries_done_nolock = ce_recv_entries_done_nolock_srng,
	.ce_send_entries_done_nolock = ce_send_entries_done_nolock_srng,
	.ce_prepare_shadow_register_v2_cfg =
		ce_prepare_shadow_register_v2_cfg_srng,
};

struct ce_ops *ce_services_srng()
{
	return &ce_service_srng;
}
