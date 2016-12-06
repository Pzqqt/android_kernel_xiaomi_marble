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

#include "dp_types.h"
#include "dp_rx.h"
#include "dp_peer.h"
#include "hal_api.h"
#include "qdf_trace.h"
#include "qdf_nbuf.h"

/**
 * dp_rx_cookie_2_link_desc_va() - Converts cookie to a virtual address of
 *				   the MSDU Link Descriptor
 * @soc: core txrx main context
 * @cookie: cookie used to lookup virtual address of link descriptor
 *          Normally this is just an index into a per SOC array.
 *
 * This is the VA of the link descriptor, that HAL layer later uses to
 * retrieve the list of MSDU's for a given .
 *
 * Return: void *: Virtual Address of the Rx descriptor
 */
static inline
void *dp_rx_cookie_2_link_desc_va(struct dp_soc *soc,
				  struct hal_buf_info *buf_info)
{
	void *link_desc_va;

	/* TODO */
	/* Add sanity for  cookie */

	link_desc_va = soc->link_desc_banks[buf_info->sw_cookie].base_vaddr +
		(buf_info->paddr -
			soc->link_desc_banks[buf_info->sw_cookie].base_paddr);

	return link_desc_va;
}

/**
 * dp_rx_frag_handle() - Handles fragmented Rx frames
 *
 * @soc: core txrx main context
 * @ring_desc: opaque pointer to the REO error ring descriptor
 * @mpdu_desc_info: MPDU descriptor information from ring descriptor
 * @head: head of the local descriptor free-list
 * @tail: tail of the local descriptor free-list
 * @quota: No. of units (packets) that can be serviced in one shot.
 *
 * This function implements RX 802.11 fragmentation handling
 * The handling is mostly same as legacy fragmentation handling.
 * If required, this function can re-inject the frames back to
 * REO ring (with proper setting to by-pass fragmentation check
 * but use duplicate detection / re-ordering and routing these frames
 * to a different core.
 *
 * Return: uint32_t: No. of elements processed
 */
uint32_t
dp_rx_frag_handle(struct dp_soc *soc, void *ring_desc,
		  struct hal_rx_mpdu_desc_info *mpdu_desc_info,
		  union dp_rx_desc_list_elem_t **head,
		  union dp_rx_desc_list_elem_t **tail,
		  uint32_t quota)
{
	uint32_t rx_bufs_used = 0;

	return rx_bufs_used;
}

/**
 * dp_rx_msdus_drop() - Drops all MSDU's per MPDU
 *
 * @soc: core txrx main context
 * @ring_desc: opaque pointer to the REO error ring descriptor
 * @mpdu_desc_info: MPDU descriptor information from ring descriptor
 * @head: head of the local descriptor free-list
 * @tail: tail of the local descriptor free-list
 * @quota: No. of units (packets) that can be serviced in one shot.
 *
 * This function is used to drop all MSDU in an MPDU
 *
 * Return: uint32_t: No. of elements processed
 */
static uint32_t
dp_rx_msdus_drop(struct dp_soc *soc, void *ring_desc,
		 struct hal_rx_mpdu_desc_info *mpdu_desc_info,
		 union dp_rx_desc_list_elem_t **head,
		 union dp_rx_desc_list_elem_t **tail,
		 uint32_t quota)
{
	uint8_t num_msdus;
	uint32_t rx_bufs_used = 0;
	void *link_desc_va;
	struct hal_buf_info buf_info;
	struct hal_rx_msdu_list msdu_list; /* MSDU's per MPDU */
	int i;

	hal_rx_reo_buf_paddr_get(ring_desc, &buf_info);

	link_desc_va = dp_rx_cookie_2_link_desc_va(soc, &buf_info);

	qdf_assert(rx_msdu_link_desc);

	/* No UNMAP required -- this is "malloc_consistent" memory */

	hal_rx_msdu_list_get(link_desc_va, &msdu_list, &num_msdus);

	for (i = 0; (i < HAL_RX_NUM_MSDU_DESC) && quota--; i++) {
		struct dp_rx_desc *rx_desc =
			dp_rx_cookie_2_va(soc, msdu_list.sw_cookie[i]);

		qdf_assert(rx_desc);

		rx_bufs_used++;

		/* Just free the buffers */
		qdf_nbuf_free(rx_desc->nbuf);

		dp_rx_add_to_free_desc_list(head, tail, rx_desc);
	}

	return rx_bufs_used;
}

/**
 * dp_rx_pn_error_handle() - Handles PN check errors
 *
 * @soc: core txrx main context
 * @ring_desc: opaque pointer to the REO error ring descriptor
 * @mpdu_desc_info: MPDU descriptor information from ring descriptor
 * @head: head of the local descriptor free-list
 * @tail: tail of the local descriptor free-list
 * @quota: No. of units (packets) that can be serviced in one shot.
 *
 * This function implements PN error handling
 * If the peer is configured to ignore the PN check errors
 * or if DP feels, that this frame is still OK, the frame can be
 * re-injected back to REO to use some of the other features
 * of REO e.g. duplicate detection/routing to other cores
 *
 * Return: uint32_t: No. of elements processed
 */
uint32_t
dp_rx_pn_error_handle(struct dp_soc *soc, void *ring_desc,
		      struct hal_rx_mpdu_desc_info *mpdu_desc_info,
		      union dp_rx_desc_list_elem_t **head,
		      union dp_rx_desc_list_elem_t **tail,
		      uint32_t quota)
{
	uint16_t peer_id;
	uint32_t rx_bufs_used = 0;
	struct dp_peer *peer;
	bool peer_pn_policy = false;

	peer_id = DP_PEER_METADATA_PEER_ID_GET(
				mpdu_desc_info->peer_meta_data);

	peer = dp_peer_find_by_id(soc, peer_id);

	if (qdf_likely(peer)) {
		/*
		 * TODO: Check for peer specific policies & set peer_pn_policy
		 */
	}


	/* No peer PN policy -- definitely drop */
	if (!peer_pn_policy)
		rx_bufs_used = dp_rx_msdus_drop(soc, ring_desc,
						mpdu_desc_info,
						head, tail, quota);

	return rx_bufs_used;
}

/**
 * dp_rx_2k_jump_handle() - Handles Sequence Number Jump by 2K
 *
 * @soc: core txrx main context
 * @ring_desc: opaque pointer to the REO error ring descriptor
 * @mpdu_desc_info: MPDU descriptor information from ring descriptor
 * @head: head of the local descriptor free-list
 * @tail: tail of the local descriptor free-list
 * @quota: No. of units (packets) that can be serviced in one shot.
 *
 * This function implements the error handling when sequence number
 * of the MPDU jumps suddenly by 2K.Today there are 2 cases that
 * need to be handled:
 *	A) CSN (Current Sequence Number) = Last Valid SN (LSN) + 2K
 *	B) CSN = LSN + 2K, but falls within a "BA sized window" of the SSN
 * For case A) the protocol stack is invoked to generate DELBA/DEAUTH frame
 * For case B), the frame is normally dropped, no more action is taken
 *
 * Return: uint32_t: No. of elements processed
 */
uint32_t
dp_rx_2k_jump_handle(struct dp_soc *soc, void *ring_desc,
		     struct hal_rx_mpdu_desc_info *mpdu_desc_info,
		     union dp_rx_desc_list_elem_t **head,
		     union dp_rx_desc_list_elem_t **tail,
		     uint32_t quota)
{
	return dp_rx_msdus_drop(soc, ring_desc, mpdu_desc_info,
				head, tail, quota);
}

/**
 * dp_rx_link_desc_return() - Return a MPDU link descriptor to HW
 *			      (WBM), following error handling
 *
 * @soc: core DP main context
 * @ring_desc: opaque pointer to the REO error ring descriptor
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
dp_rx_link_desc_return(struct dp_soc *soc, void *ring_desc)
{
	void *buf_addr_info = HAL_RX_REO_BUF_ADDR_INFO_GET(ring_desc);
	struct dp_srng *wbm_desc_rel_ring = &soc->wbm_desc_rel_ring;
	void *wbm_rel_srng = wbm_desc_rel_ring->hal_srng;
	void *hal_soc = soc->hal_soc;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	void *src_srng_desc;

	if (!wbm_rel_srng) {
		qdf_print("WBM RELEASE RING not initialized\n");
		return status;
	}

	if (qdf_unlikely(hal_srng_access_start(hal_soc, wbm_rel_srng))) {

		/* TODO */
		/*
		 * Need API to convert from hal_ring pointer to
		 * Ring Type / Ring Id combo
		 */
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s %d : "
			"HAL RING Access For WBM Release SRNG Failed -- %p\n",
			__func__, __LINE__, wbm_rel_srng);
		goto done;
	}
	src_srng_desc = hal_srng_src_get_next(hal_soc, wbm_rel_srng);
	if (qdf_likely(src_srng_desc)) {
		/* Return link descriptor through WBM ring (SW2WBM)*/
		hal_rx_msdu_link_desc_set(hal_soc,
				src_srng_desc, buf_addr_info);
		status = QDF_STATUS_SUCCESS;
	} else {
		struct hal_srng *srng = (struct hal_srng *)wbm_rel_srng;
		qdf_print("%s %d -- WBM Release Ring (Id %d) Full\n",
			__func__, __LINE__, srng->ring_id);
		qdf_print("HP 0x%x Reap HP 0x%x TP 0x%x Cached TP 0x%x\n",
			*srng->u.src_ring.hp_addr, srng->u.src_ring.reap_hp,
			*srng->u.src_ring.tp_addr, srng->u.src_ring.cached_tp);
	}
done:
	hal_srng_access_end(hal_soc, wbm_rel_srng);
	return status;
}

/**
 * dp_rx_err_process() - Processes error frames routed to REO error ring
 *
 * @soc: core txrx main context
 * @hal_ring: opaque pointer to the HAL Rx Error Ring, which will be serviced
 * @quota: No. of units (packets) that can be serviced in one shot.
 *
 * This function implements error processing and top level demultiplexer
 * for all the frames routed to REO error ring.
 *
 * Return: uint32_t: No. of elements processed
 */
uint32_t
dp_rx_err_process(struct dp_soc *soc, void *hal_ring, uint32_t quota)
{
	void *hal_soc;
	void *ring_desc;
	union dp_rx_desc_list_elem_t *head = NULL;
	union dp_rx_desc_list_elem_t *tail = NULL;
	uint32_t rx_bufs_used = 0;
	uint8_t buf_type;
	uint8_t error, rbm;
	struct hal_rx_mpdu_desc_info mpdu_desc_info;
	struct hal_buf_info hbi;

	/* Debug -- Remove later */
	qdf_assert(soc && hal_ring);

	hal_soc = soc->hal_soc;

	/* Debug -- Remove later */
	qdf_assert(hal_soc);

	if (qdf_unlikely(hal_srng_access_start(hal_soc, hal_ring))) {

		/* TODO */
		/*
		 * Need API to convert from hal_ring pointer to
		 * Ring Type / Ring Id combo
		 */
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s %d : HAL RING Access Failed -- %p\n",
			__func__, __LINE__, hal_ring);
		goto done;
	}

	while (qdf_likely((ring_desc =
				hal_srng_dst_get_next(hal_soc, hal_ring))
				&& quota--)) {

		error = HAL_RX_ERROR_STATUS_GET(ring_desc);

		qdf_assert(error == HAL_REO_ERROR_DETECTED);

		/*
		 * Check if the buffer is to be processed on this processor
		 */
		rbm = hal_rx_ret_buf_manager_get(ring_desc);

		if (qdf_unlikely(rbm != HAL_RX_BUF_RBM_SW3_BM)) {
			/* TODO */
			/* Call appropriate handler */

			qdf_print("%s %d: Invalid RBM %d\n",
				__func__, __LINE__, rbm);
			continue;
		}

		buf_type = HAL_RX_REO_BUF_TYPE_GET(ring_desc);
		/*
		 * For REO error ring, expect only MSDU LINK DESC
		 */
		qdf_assert(buf_type == HAL_RX_REO_MSDU_LINK_DESC_TYPE);

		hal_rx_reo_buf_paddr_get(ring_desc, &hbi);

		/* Get the MPDU DESC info */
		hal_rx_mpdu_info_get(ring_desc, &mpdu_desc_info);

		if (mpdu_desc_info.mpdu_flags & HAL_MPDU_F_FRAGMENT) {
			/* TODO */
			rx_bufs_used += dp_rx_frag_handle(soc,
					ring_desc, &mpdu_desc_info,
					&head, &tail, quota);
			continue;
		}

		if (hal_rx_reo_is_pn_error(ring_desc)) {
			/* TOD0 */
			rx_bufs_used += dp_rx_pn_error_handle(soc,
					ring_desc, &mpdu_desc_info,
					&head, &tail, quota);
			continue;
		}

		if (hal_rx_reo_is_2k_jump(ring_desc)) {
			/* TOD0 */
			rx_bufs_used += dp_rx_2k_jump_handle(soc,
					ring_desc, &mpdu_desc_info,
					&head, &tail, quota);
			continue;
		}

		/* Return link descriptor through WBM ring (SW2WBM)*/
		dp_rx_link_desc_return(soc, ring_desc);
	}

done:
	hal_srng_access_end(hal_soc, hal_ring);

	/* Assume MAC id = 0, owner = 0 */
	dp_rx_buffers_replenish(soc, 0, rx_bufs_used, &head, &tail,
					HAL_RX_BUF_RBM_SW3_BM);

	return rx_bufs_used; /* Assume no scale factor for now */
}

/**
 * dp_rx_wbm_err_process() - Processes error frames routed to WBM release ring
 *
 * @soc: core txrx main context
 * @hal_ring: opaque pointer to the HAL Rx Error Ring, which will be serviced
 * @quota: No. of units (packets) that can be serviced in one shot.
 *
 * This function implements error processing and top level demultiplexer
 * for all the frames routed to WBM2HOST sw release ring.
 *
 * Return: uint32_t: No. of elements processed
 */
uint32_t
dp_rx_wbm_err_process(struct dp_soc *soc, void *hal_ring, uint32_t quota)
{
	void *hal_soc;
	void *ring_desc;
	struct dp_rx_desc *rx_desc;
	union dp_rx_desc_list_elem_t *head = NULL;
	union dp_rx_desc_list_elem_t *tail = NULL;
	uint32_t rx_bufs_used = 0;
	uint8_t buf_type, rbm;
	uint8_t wbm_err_src;
	uint32_t rx_buf_cookie;

	/* Debug -- Remove later */
	qdf_assert(soc && hal_ring);

	hal_soc = soc->hal_soc;

	/* Debug -- Remove later */
	qdf_assert(hal_soc);

	if (qdf_unlikely(hal_srng_access_start(hal_soc, hal_ring))) {

		/* TODO */
		/*
		 * Need API to convert from hal_ring pointer to
		 * Ring Type / Ring Id combo
		 */
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s %d : HAL RING Access Failed -- %p\n",
			__func__, __LINE__, hal_ring);
		goto done;
	}

	while (qdf_likely((ring_desc =
				hal_srng_dst_get_next(hal_soc, hal_ring))
				&& quota--)) {

		/* XXX */
		wbm_err_src = HAL_RX_WBM_ERR_SRC_GET(ring_desc);

		qdf_assert((wbm_err_src == HAL_RX_WBM_ERR_SRC_RXDMA) ||
				(wbm_err_src == HAL_RX_WBM_ERR_SRC_REO));

		/*
		 * Check if the buffer is to be processed on this processor
		 */
		rbm = hal_rx_ret_buf_manager_get(ring_desc);

		if (qdf_unlikely(rbm != HAL_RX_BUF_RBM_SW3_BM)) {
			/* TODO */
			/* Call appropriate handler */

			qdf_print("%s %d: Invalid RBM %d\n",
				__func__, __LINE__, rbm);
			continue;
		}

		/* XXX */
		buf_type = HAL_RX_WBM_BUF_TYPE_GET(ring_desc);
		/*
		 * For WBM ring, expect only MSDU buffers
		 */
		qdf_assert(buf_type == HAL_RX_WBM_BUF_TYPE_REL_BUF);

		if (wbm_err_src == HAL_RX_WBM_ERR_SRC_REO) {

			uint8_t push_reason =
				HAL_RX_WBM_REO_PUSH_REASON_GET(ring_desc);

			if (push_reason == HAL_RX_WBM_REO_PSH_RSN_ERROR) {

				uint8_t reo_error_code =
				   HAL_RX_WBM_REO_ERROR_CODE_GET(ring_desc);

				switch (reo_error_code) {

				/* TODO */
				/* Add per error code accounting */

				default:
					qdf_print(
					"%s %d: REO error %d detected\n",
					__func__, __LINE__, reo_error_code);
				}
			}
		} else if (wbm_err_src == HAL_RX_WBM_ERR_SRC_RXDMA) {
			uint8_t push_reason =
				HAL_RX_WBM_RXDMA_PUSH_REASON_GET(ring_desc);

			if (push_reason == HAL_RX_WBM_RXDMA_PSH_RSN_ERROR) {

				uint8_t rxdma_error_code =
				   HAL_RX_WBM_RXDMA_ERROR_CODE_GET(ring_desc);

				switch (rxdma_error_code) {

				/* TODO */
				/* Add per error code accounting */

				default:
					qdf_print(
					"%s %d: RXDMA error %d detected\n",
					__func__, __LINE__, rxdma_error_code);
				}
			}
		} else {
			/* Should not come here */
			qdf_assert(0);
		}
		rx_buf_cookie = HAL_RX_WBM_BUF_COOKIE_GET(ring_desc);

		rx_desc = dp_rx_cookie_2_va(soc, rx_buf_cookie);

		qdf_assert(rx_desc);

		rx_bufs_used++;

		qdf_nbuf_unmap_single(soc->osdev, rx_desc->nbuf,
				QDF_DMA_BIDIRECTIONAL);

		qdf_nbuf_free(rx_desc->nbuf);

		dp_rx_add_to_free_desc_list(&head, &tail, rx_desc);
	}

done:
	hal_srng_access_end(hal_soc, hal_ring);

	/* Assume MAC id = 0, owner = 0 */
	dp_rx_buffers_replenish(soc, 0, rx_bufs_used, &head, &tail,
					HAL_RX_BUF_RBM_SW3_BM);

	return rx_bufs_used; /* Assume no scale factor for now */
}
