/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
#include "hal_rx.h"
#include "hal_api.h"
#include "qdf_nbuf.h"
#include <ieee80211.h>

#ifdef RXDMA_OPTIMIZATION
#define RX_BUFFER_ALIGNMENT	128
#else /* RXDMA_OPTIMIZATION */
#define RX_BUFFER_ALIGNMENT	4
#endif /* RXDMA_OPTIMIZATION */

#define RX_BUFFER_SIZE		2048
#define RX_BUFFER_RESERVATION	0

/*
 * dp_rx_buffers_replenish() - replenish rxdma ring with rx nbufs
 *			       called during dp rx initialization
 *			       and at the end of dp_rx_process.
 *
 * @soc: core txrx main context
 * @mac_id: mac_id which is one of 3 mac_ids
 * @desc_list: list of descs if called from dp_rx_process
 *	       or NULL during dp rx initialization or out of buffer
 *	       interrupt.
 * @owner: who owns the nbuf (host, NSS etc...)
 * Return: return success or failure
 */
QDF_STATUS dp_rx_buffers_replenish(struct dp_soc *dp_soc, uint32_t mac_id,
				 uint32_t num_req_buffers,
				 union dp_rx_desc_list_elem_t **desc_list,
				 union dp_rx_desc_list_elem_t **tail,
				 uint8_t owner)
{
	uint32_t num_alloc_desc;
	uint16_t num_desc_to_free = 0;
	struct dp_pdev *dp_pdev = dp_soc->pdev_list[mac_id];
	uint32_t num_entries_avail;
	uint32_t count;
	int sync_hw_ptr = 1;
	qdf_dma_addr_t paddr;
	qdf_nbuf_t rx_netbuf;
	void *rxdma_ring_entry;
	union dp_rx_desc_list_elem_t *next;
	struct dp_srng *dp_rxdma_srng = &dp_pdev->rx_refill_buf_ring;
	void *rxdma_srng = dp_rxdma_srng->hal_srng;

	if (!rxdma_srng) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"rxdma srng not initialized");
		return QDF_STATUS_E_FAILURE;
	}

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
		"requested %d buffers for replenish", num_req_buffers);

	/*
	 * if desc_list is NULL, allocate the descs from freelist
	 */
	if (!(*desc_list)) {
		num_alloc_desc = dp_rx_get_free_desc_list(dp_soc, mac_id,
							  num_req_buffers,
							  desc_list,
							  tail);

		if (!num_alloc_desc) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"no free rx_descs in freelist");
			return QDF_STATUS_E_NOMEM;
		}

		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"%d rx desc allocated", num_alloc_desc);
		num_req_buffers = num_alloc_desc;
	}

	hal_srng_access_start(dp_soc->hal_soc, rxdma_srng);
	num_entries_avail = hal_srng_src_num_avail(dp_soc->hal_soc,
						   rxdma_srng,
						   sync_hw_ptr);

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"no of availble entries in rxdma ring: %d",
			num_entries_avail);

	if (num_entries_avail < num_req_buffers) {
		num_desc_to_free = num_req_buffers - num_entries_avail;
		num_req_buffers = num_entries_avail;
	}

	for (count = 0; count < num_req_buffers; count++) {
		rxdma_ring_entry = hal_srng_src_get_next(dp_soc->hal_soc,
							 rxdma_srng);

		rx_netbuf = qdf_nbuf_alloc(dp_pdev->osif_pdev,
					RX_BUFFER_SIZE,
					RX_BUFFER_RESERVATION,
					RX_BUFFER_ALIGNMENT,
					FALSE);

		if (rx_netbuf == NULL)
			break;

		qdf_nbuf_map_single(dp_soc->osdev, rx_netbuf,
				    QDF_DMA_BIDIRECTIONAL);

		paddr = qdf_nbuf_get_frag_paddr(rx_netbuf, 0);

		next = (*desc_list)->next;

		(*desc_list)->rx_desc.nbuf = rx_netbuf;
		hal_rxdma_buff_addr_info_set(rxdma_ring_entry, paddr,
						(*desc_list)->rx_desc.cookie,
						owner);

		*desc_list = next;
	}

	hal_srng_access_end(dp_soc->hal_soc, rxdma_srng);

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
		"successfully replenished %d buffers", num_req_buffers);
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
		"%d rx desc added back to free list", num_desc_to_free);

	/*
	 * add any available free desc back to the free list
	 */
	if (*desc_list)
		dp_rx_add_desc_list_to_free_list(dp_soc, desc_list,
						 tail, mac_id);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_rx_intrabss_fwd() - Implements the Intra-BSS forwarding logic
 *
 * @soc: core txrx main context
 * @sa_peer	: source peer entry
 * @rx_tlv_hdr	: start address of rx tlvs
 * @nbuf	: nbuf that has to be intrabss forwarded
 *
 * Return: bool: true if it is forwarded else false
 */
static bool
dp_rx_intrabss_fwd(struct dp_soc *soc,
			struct dp_peer *sa_peer,
			uint8_t *rx_tlv_hdr,
			qdf_nbuf_t nbuf)
{
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
		FL("Intra-BSS forwarding not implemented"));
	return false;
}


/**
 * dp_rx_process() - Brain of the Rx processing functionality
 *		     Called from the bottom half (tasklet/NET_RX_SOFTIRQ)
 * @soc: core txrx main context
 * @hal_ring: opaque pointer to the HAL Rx Ring, which will be serviced
 * @quota: No. of units (packets) that can be serviced in one shot.
 *
 * This function implements the core of Rx functionality. This is
 * expected to handle only non-error frames.
 *
 * Return: uint32_t: No. of elements processed
 */
uint32_t
dp_rx_process(struct dp_soc *soc, void *hal_ring, uint32_t quota)
{
	void *hal_soc;
	void *ring_desc;
	struct dp_rx_desc *rx_desc;
	qdf_nbuf_t nbuf;
	qdf_nbuf_t deliver_list_head = NULL;
	qdf_nbuf_t deliver_list_tail = NULL;
	union dp_rx_desc_list_elem_t *head = NULL;
	union dp_rx_desc_list_elem_t *tail = NULL;
	uint32_t rx_bufs_used = 0, rx_buf_cookie, l2_hdr_offset;
	uint16_t msdu_len;
	uint16_t peer_id;
	struct dp_peer *peer = NULL;
	struct dp_vdev *vdev = NULL;
	struct hal_rx_mpdu_desc_info mpdu_desc_info;
	enum hal_reo_error_status error;
	uint32_t pkt_len;
	uint32_t peer_mdata;
	uint8_t *rx_tlv_hdr;
	uint32_t rx_bufs_reaped = 0;
	struct dp_pdev *pdev;

	/* Debug -- Remove later */
	qdf_assert(soc && hal_ring);

	hal_soc = soc->hal_soc;

	/* Debug -- Remove later */
	qdf_assert(hal_soc);

	if (qdf_unlikely(hal_srng_access_start(hal_soc, hal_ring))) {

		/*
		 * Need API to convert from hal_ring pointer to
		 * Ring Type / Ring Id combo
		 */
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			FL("HAL RING Access Failed -- %p"), hal_ring);
		hal_srng_access_end(hal_soc, hal_ring);
		goto done;
	}

	/*
	 * start reaping the buffers from reo ring and queue
	 * them in per vdev queue.
	 * Process the received pkts in a different per vdev loop.
	 */
	while (qdf_likely((ring_desc =
				hal_srng_dst_get_next(hal_soc, hal_ring))
				&& quota--)) {

		error = HAL_RX_ERROR_STATUS_GET(ring_desc);

		if (qdf_unlikely(error == HAL_REO_ERROR_DETECTED)) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("HAL RING 0x%p:error %d"), hal_ring, error);
			/* Don't know how to deal with this -- assert */
			qdf_assert(0);
		}

		rx_buf_cookie = HAL_RX_REO_BUF_COOKIE_GET(ring_desc);

		rx_desc = dp_rx_cookie_2_va(soc, rx_buf_cookie);

		qdf_assert(rx_desc);
		rx_bufs_reaped++;

		/* TODO */
		/*
		 * Need a separate API for unmapping based on
		 * phyiscal address
		 */
		qdf_nbuf_unmap_single(soc->osdev, rx_desc->nbuf,
					QDF_DMA_BIDIRECTIONAL);

		/* Get MPDU DESC info */
		hal_rx_mpdu_desc_info_get(ring_desc, &mpdu_desc_info);
		peer_id = DP_PEER_METADATA_PEER_ID_GET(
				mpdu_desc_info.peer_meta_data);

		peer = dp_peer_find_by_id(soc, peer_id);
		if (!peer) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				 FL("peer look-up failed peer id %d"), peer_id);

			/* Drop & free packet */
			qdf_nbuf_free(rx_desc->nbuf);
			goto fail;
		}

		vdev = peer->vdev;
		if (!vdev) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				FL("vdev is NULL"));
			qdf_nbuf_free(rx_desc->nbuf);
			goto fail;
		}

		qdf_nbuf_queue_add(&vdev->rxq, rx_desc->nbuf);
fail:
		dp_rx_add_to_free_desc_list(&head, &tail, rx_desc);
	}
done:
	hal_srng_access_end(hal_soc, hal_ring);

	if (!rx_bufs_reaped)
		return 0;
	else
		/* Replenish buffers */
		/* Assume MAC id = 0, owner = 0 */
		dp_rx_buffers_replenish(soc, 0, rx_bufs_reaped, &head, &tail,
					HAL_RX_BUF_RBM_SW3_BM);

	pdev = soc->pdev_list[0];

	if (qdf_unlikely(!pdev)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("pdev is NULL"));
		goto fail1;
	}

	TAILQ_FOREACH(vdev, &pdev->vdev_list, vdev_list_elem) {

		while ((nbuf = qdf_nbuf_queue_remove(&vdev->rxq))) {
			rx_tlv_hdr = qdf_nbuf_data(nbuf);

			/*
			 * Check if DMA completed -- msdu_done is the last bit
			 * to be written
			 */
			if (!hal_rx_attn_msdu_done_get(rx_tlv_hdr)) {

				QDF_TRACE(QDF_MODULE_ID_DP,
						QDF_TRACE_LEVEL_ERROR,
						FL("HAL RING 0x%p"), hal_ring);

				print_hex_dump(KERN_ERR,
				       "\t Pkt Desc:", DUMP_PREFIX_NONE, 32, 4,
					rx_tlv_hdr, 128, false);

				qdf_assert(0);
			}

			peer_mdata = hal_rx_mpdu_peer_meta_data_get(rx_tlv_hdr);
			peer_id = DP_PEER_METADATA_PEER_ID_GET(peer_mdata);
			peer = dp_peer_find_by_id(soc, peer_id);

			/* TODO */
			/*
			 * In case of roaming peer object may not be
			 * immediately available -- need to handle this
			 * Cannot drop these packets right away.
			 */
			/* Peer lookup failed */
			if (!peer) {

				/* Drop & free packet */
				qdf_nbuf_free(nbuf);

				/* Statistics */
				continue;
			}

			/*
			 * HW structures call this L3 header padding --
			 * even though this is actually the offset from
			 * the buffer beginning where the L2 header
			 * begins.
			 */
			l2_hdr_offset =
				hal_rx_msdu_end_l3_hdr_padding_get(rx_tlv_hdr);

			msdu_len = hal_rx_msdu_start_msdu_len_get(rx_tlv_hdr);
			pkt_len = msdu_len + l2_hdr_offset + RX_PKT_TLVS_LEN;

			/* Set length in nbuf */
			qdf_nbuf_set_pktlen(nbuf, pkt_len);

			/*
			 * Advance the packet start pointer by total size of
			 * pre-header TLV's
			 */
			qdf_nbuf_pull_head(nbuf,
					   RX_PKT_TLVS_LEN + l2_hdr_offset);

#ifdef QCA_WIFI_NAPIER_EMULATION /* Debug code, remove later */
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"p_id %d msdu_len %d hdr_off %d",
				peer_id, msdu_len, l2_hdr_offset);

			print_hex_dump(KERN_ERR,
				       "\t Pkt Data:", DUMP_PREFIX_NONE, 32, 4,
					qdf_nbuf_data(nbuf), 128, false);
#endif /* NAPIER_EMULATION */

			/* TODO */
			/* WDS Source Port Learning */

			/* Intrabss-fwd */
			if (dp_rx_intrabss_fwd(soc, peer, rx_tlv_hdr, nbuf))
				continue; /* Get next descriptor */

			rx_bufs_used++;
			DP_RX_LIST_APPEND(deliver_list_head,
						deliver_list_tail,
						nbuf);
		}

		if (qdf_likely(vdev->osif_rx) && deliver_list_head)
			vdev->osif_rx(vdev->osif_vdev, deliver_list_head);

	}
	return rx_bufs_used; /* Assume no scale factor for now */
fail1:
	qdf_assert(0);
	return 0;
}

/**
 * dp_rx_detach() - detach dp rx
 * @soc: core txrx main context
 *
 * This function will detach DP RX into main device context
 * will free DP Rx resources.
 *
 * Return: void
 */
void
dp_rx_pdev_detach(struct dp_pdev *pdev)
{
	uint8_t pdev_id = pdev->pdev_id;
	struct dp_soc *soc = pdev->soc;

	dp_rx_desc_pool_free(soc, pdev_id);
	qdf_spinlock_destroy(&soc->rx_desc_mutex[pdev_id]);

	return;
}

/**
 * dp_rx_attach() - attach DP RX
 * @soc: core txrx main context
 *
 * This function will attach a DP RX instance into the main
 * device (SOC) context. Will allocate dp rx resource and
 * initialize resources.
 *
 * Return: QDF_STATUS_SUCCESS: success
 *         QDF_STATUS_E_RESOURCES: Error return
 */
QDF_STATUS
dp_rx_pdev_attach(struct dp_pdev *pdev)
{
	uint8_t pdev_id = pdev->pdev_id;
	struct dp_soc *soc = pdev->soc;
	struct dp_srng rxdma_srng;
	uint32_t rxdma_entries;
	union dp_rx_desc_list_elem_t *desc_list = NULL;
	union dp_rx_desc_list_elem_t *tail = NULL;

	qdf_spinlock_create(&soc->rx_desc_mutex[pdev_id]);
	pdev = soc->pdev_list[pdev_id];
	rxdma_srng = pdev->rx_refill_buf_ring;

	rxdma_entries = rxdma_srng.alloc_size/hal_srng_get_entrysize(
						     soc->hal_soc, RXDMA_BUF);
	dp_rx_desc_pool_alloc(soc, pdev_id);

	/* For Rx buffers, WBM release ring is SW RING 3,for all pdev's */
	dp_rx_buffers_replenish(soc, pdev_id, rxdma_entries,
				&desc_list, &tail, HAL_RX_BUF_RBM_SW3_BM);

	return QDF_STATUS_SUCCESS;
}
