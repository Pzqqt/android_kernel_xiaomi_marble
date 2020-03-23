/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "dp_types.h"
#include "hal_rx.h"
#include "hal_api.h"
#include "qdf_trace.h"
#include "qdf_nbuf.h"
#include "hal_api_mon.h"
#include "dp_rx.h"
#include "dp_rx_mon.h"
#include "dp_internal.h"
#include "dp_htt.h"
#include "dp_full_mon.h"
#include "qdf_mem.h"

#ifdef QCA_SUPPORT_FULL_MON

uint32_t
dp_rx_mon_status_process(struct dp_soc *soc,
			 uint32_t mac_id,
			 uint32_t quota);

/*
 * dp_rx_mon_prepare_mon_mpdu () - API to prepare dp_mon_mpdu object
 *
 * @pdev: DP pdev object
 * @head_msdu: Head msdu
 * @tail_msdu: Tail msdu
 *
 */
static inline struct dp_mon_mpdu *
dp_rx_mon_prepare_mon_mpdu(struct dp_pdev *pdev,
			   qdf_nbuf_t head_msdu,
			   qdf_nbuf_t tail_msdu)
{
	struct dp_mon_mpdu *mon_mpdu = NULL;

	mon_mpdu = qdf_mem_malloc(sizeof(struct dp_mon_mpdu));

	if (!mon_mpdu) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  FL("Monitor MPDU object allocation failed -- %pK"),
			     pdev);
		qdf_assert_always(0);
	}

	mon_mpdu->head = head_msdu;
	mon_mpdu->tail = tail_msdu;
	mon_mpdu->rs_flags = pdev->ppdu_info.rx_status.rs_flags;
	mon_mpdu->ant_signal_db = pdev->ppdu_info.rx_status.ant_signal_db;
	mon_mpdu->is_stbc = pdev->ppdu_info.rx_status.is_stbc;
	mon_mpdu->sgi = pdev->ppdu_info.rx_status.sgi;
	mon_mpdu->beamformed = pdev->ppdu_info.rx_status.beamformed;

	return mon_mpdu;
}

/*
 * dp_rx_monitor_deliver_ppdu () - API to deliver all MPDU for a MPDU
 * to upper layer stack
 *
 * @soc: DP soc handle
 * @mac_id: lmac id
 */
static inline QDF_STATUS
dp_rx_monitor_deliver_ppdu(struct dp_soc *soc, uint32_t mac_id)
{
	struct dp_pdev *pdev = dp_get_pdev_for_lmac_id(soc, mac_id);
	struct dp_mon_mpdu *mpdu = NULL;
	struct dp_mon_mpdu *temp_mpdu = NULL;

	if (!TAILQ_EMPTY(&pdev->mon_mpdu_q)) {
		TAILQ_FOREACH_SAFE(mpdu,
				   &pdev->mon_mpdu_q,
				   mpdu_list_elem,
				   temp_mpdu) {
			TAILQ_REMOVE(&pdev->mon_mpdu_q,
				     mpdu, mpdu_list_elem);

			pdev->ppdu_info.rx_status.rs_flags = mpdu->rs_flags;
			pdev->ppdu_info.rx_status.ant_signal_db =
				mpdu->ant_signal_db;
			pdev->ppdu_info.rx_status.is_stbc = mpdu->is_stbc;
			pdev->ppdu_info.rx_status.sgi = mpdu->sgi;
			pdev->ppdu_info.rx_status.beamformed = mpdu->beamformed;

			dp_rx_mon_deliver(soc, mac_id,
					  mpdu->head, mpdu->tail);

			qdf_mem_free(mpdu);
		}
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_rx_mon_reap_status_ring () - Reap status_buf_count of status buffers for
 * status ring.
 *
 * @soc: DP soc handle
 * @mac_id: mac id on which interrupt is received
 * @quota: number of status ring entries to be reaped
 * @desc_info: Rx ppdu desc info
 */
static inline uint32_t
dp_rx_mon_reap_status_ring(struct dp_soc *soc,
			   uint32_t mac_id,
			   uint32_t quota,
			   struct hal_rx_mon_desc_info *desc_info)
{
	struct dp_pdev *pdev = dp_get_pdev_for_lmac_id(soc, mac_id);
	uint8_t status_buf_count;
	uint32_t work_done;

	status_buf_count = desc_info->status_buf_count;

	qdf_mem_copy(&pdev->mon_desc, desc_info,
		     sizeof(struct hal_rx_mon_desc_info));

	work_done = dp_rx_mon_status_process(soc, mac_id, status_buf_count);

	if (desc_info->ppdu_id != pdev->ppdu_info.com_info.ppdu_id) {
		qdf_err("DEBUG: count: %d quota: %d", status_buf_count, quota);
		dp_print_ring_stats(pdev);
		qdf_assert_always(0);
	}

	/* DEBUG */
	if (work_done != status_buf_count) {
		qdf_err("Reaped status ring buffers are not equal to "
			"status buf count from destination ring work_done:"
			" %d status_buf_count: %d",
			work_done, status_buf_count);

		dp_print_ring_stats(pdev);
		qdf_assert_always(0);
	}

	return work_done;
}

/**
 * dp_rx_mon_mpdu_reap () - This API reaps a mpdu from mon dest ring descriptor
 * and returns link descriptor to HW (WBM)
 *
 * @soc: DP soc handle
 * @mac_id: lmac id
 * @ring_desc: SW monitor ring desc
 * @head_msdu: nbuf pointing to first msdu in a chain
 * @tail_msdu: nbuf pointing to last msdu in a chain
 * @head_desc: head pointer to free desc list
 * @tail_desc: tail pointer to free desc list
 *
 * Return: number of reaped buffers
 */
static inline uint32_t
dp_rx_mon_mpdu_reap(struct dp_soc *soc, uint32_t mac_id, void *ring_desc,
		    qdf_nbuf_t *head_msdu, qdf_nbuf_t *tail_msdu,
		    union dp_rx_desc_list_elem_t **head_desc,
		    union dp_rx_desc_list_elem_t **tail_desc)
{
	struct dp_pdev *pdev = dp_get_pdev_for_lmac_id(soc, mac_id);
	struct dp_rx_desc *rx_desc = NULL;
	struct hal_rx_msdu_list msdu_list;
	uint32_t rx_buf_reaped = 0;
	uint16_t num_msdus = 0, msdu_index, rx_hdr_tlv_len, l3_hdr_pad;
	uint32_t total_frag_len = 0, frag_len = 0;
	bool drop_mpdu = false;
	bool msdu_frag = false;
	void *link_desc_va;
	uint8_t *rx_tlv_hdr;
	qdf_nbuf_t msdu = NULL, last_msdu = NULL;
	uint32_t rx_link_buf_info[HAL_RX_BUFFINFO_NUM_DWORDS];
	struct hal_rx_mon_desc_info *desc_info;

	desc_info = pdev->mon_desc;

	qdf_mem_zero(desc_info, sizeof(struct hal_rx_mon_desc_info));

	/* Read SW Mon ring descriptor */
	hal_rx_sw_mon_desc_info_get((struct hal_soc *)soc->hal_soc,
				    ring_desc,
				    (void *)desc_info);

	/* If end_of_ppdu is 1, return*/
	if (desc_info->end_of_ppdu)
		return rx_buf_reaped;

	/* If there is rxdma error, drop mpdu */
	if (qdf_unlikely(dp_rx_mon_is_rxdma_error(desc_info)
			== QDF_STATUS_SUCCESS)) {
		drop_mpdu = true;
		pdev->rx_mon_stats.dest_mpdu_drop++;
	}

	/*
	 * while loop iterates through all link descriptors and
	 * reaps msdu_count number of msdus for one SW_MONITOR_RING descriptor
	 * and forms nbuf queue.
	 */
	while (desc_info->msdu_count && desc_info->link_desc.paddr) {
		link_desc_va = dp_rx_cookie_2_mon_link_desc(pdev,
							    desc_info->link_desc,
							    mac_id);

		qdf_assert_always(link_desc_va);

		hal_rx_msdu_list_get(soc->hal_soc,
				     link_desc_va,
				     &msdu_list,
				     &num_msdus);

		for (msdu_index = 0; msdu_index < num_msdus; msdu_index++) {
			rx_desc = dp_rx_get_mon_desc(soc,
						     msdu_list.sw_cookie[msdu_index]);

			qdf_assert_always(rx_desc);

			msdu = rx_desc->nbuf;

			if (rx_desc->unmapped == 0) {
				qdf_nbuf_unmap_single(soc->osdev,
						      msdu,
						      QDF_DMA_FROM_DEVICE);
				rx_desc->unmapped = 1;
			}

			if (drop_mpdu) {
				qdf_nbuf_free(msdu);
				msdu = NULL;
				desc_info->msdu_count--;
				goto next_msdu;
			}

			rx_tlv_hdr = qdf_nbuf_data(msdu);

			if (hal_rx_desc_is_first_msdu(soc->hal_soc,
						      rx_tlv_hdr))
				hal_rx_mon_hw_desc_get_mpdu_status(soc->hal_soc,
								   rx_tlv_hdr,
								   &pdev->ppdu_info.rx_status);

			/** If msdu is fragmented, spread across multiple
			 *  buffers
			 *   a. calculate len of each fragmented buffer
			 *   b. calculate the number of fragmented buffers for
			 *      a msdu and decrement one msdu_count
			 */
			if (msdu_list.msdu_info[msdu_index].msdu_flags
			    & HAL_MSDU_F_MSDU_CONTINUATION) {
				if (!msdu_frag) {
					total_frag_len = msdu_list.msdu_info[msdu_index].msdu_len;
					msdu_frag = true;
				}
				dp_mon_adjust_frag_len(&total_frag_len,
						       &frag_len);
			} else {
				if (msdu_frag)
					dp_mon_adjust_frag_len(&total_frag_len,
							       &frag_len);
				else
					frag_len = msdu_list.msdu_info[msdu_index].msdu_len;
				msdu_frag = false;
				desc_info->msdu_count--;
			}

			rx_hdr_tlv_len = SIZE_OF_MONITOR_TLV;

			/*
			 * HW structures call this L3 header padding.
			 * this is actually the offset
			 * from the buffer beginning where the L2
			 * header begins.
			 */

			l3_hdr_pad = hal_rx_msdu_end_l3_hdr_padding_get(
								soc->hal_soc,
								rx_tlv_hdr);

			/*******************************************************
			 *                    RX_PACKET                        *
			 * ----------------------------------------------------*
			 |   RX_PKT_TLVS  |   L3 Padding header  |  msdu data| |
			 * ----------------------------------------------------*
			 ******************************************************/

			qdf_nbuf_set_pktlen(msdu,
					    rx_hdr_tlv_len +
					    l3_hdr_pad +
					    frag_len);

			if (head_msdu && !*head_msdu)
				*head_msdu = msdu;
			else if (last_msdu)
				qdf_nbuf_set_next(last_msdu, msdu);

			last_msdu = msdu;

next_msdu:
			rx_buf_reaped++;
			dp_rx_add_to_free_desc_list(head_desc,
						    tail_desc,
						    rx_desc);

			QDF_TRACE(QDF_MODULE_ID_DP,
				  QDF_TRACE_LEVEL_DEBUG,
				  FL("%s total_len %u frag_len %u flags %u"),
				  total_frag_len, frag_len,
				  msdu_list.msdu_info[msdu_index].msdu_flags);
		}

		hal_rxdma_buff_addr_info_set(rx_link_buf_info,
					     desc_info->link_desc.paddr,
					     desc_info->link_desc.sw_cookie,
					     desc_info->link_desc.rbm);

		/* Get next link desc VA from current link desc */
		hal_rx_mon_next_link_desc_get(link_desc_va,
					      &desc_info->link_desc);

		/* return msdu link descriptor to WBM */
		if (dp_rx_monitor_link_desc_return(pdev,
						   (hal_buff_addrinfo_t)rx_link_buf_info,
						   mac_id,
						   HAL_BM_ACTION_PUT_IN_IDLE_LIST)
				!= QDF_STATUS_SUCCESS) {
			dp_print_ring_stats(pdev);
			qdf_assert_always(0);
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				  "dp_rx_monitor_link_desc_return failed");
		}
	}

	if (last_msdu)
		qdf_nbuf_set_next(last_msdu, NULL);

	*tail_msdu = msdu;

	return rx_buf_reaped;
}

/**
 * dp_rx_mon_process () - Core brain processing for monitor mode
 *
 * This API processes monitor destination ring followed by monitor status ring
 * Called from bottom half (tasklet/NET_RX_SOFTIRQ)
 *
 * @soc: datapath soc context
 * @mac_id: mac_id on which interrupt is received
 * @quota: Number of status ring entry that can be serviced in one shot.
 *
 * @Return: Number of reaped status ring entries
 */
uint32_t dp_rx_mon_process(struct dp_soc *soc, uint32_t mac_id, uint32_t quota)
{
	struct dp_pdev *pdev = dp_get_pdev_for_lmac_id(soc, mac_id);
	union dp_rx_desc_list_elem_t *head_desc = NULL;
	union dp_rx_desc_list_elem_t *tail_desc = NULL;
	uint32_t rx_bufs_reaped = 0;
	struct dp_mon_mpdu *mon_mpdu;
	struct cdp_pdev_mon_stats *rx_mon_stats = &pdev->rx_mon_stats;
	hal_rxdma_desc_t ring_desc;
	hal_soc_handle_t hal_soc;
	hal_ring_handle_t mon_dest_srng;
	qdf_nbuf_t head_msdu = NULL;
	qdf_nbuf_t tail_msdu = NULL;
	struct hal_rx_mon_desc_info *desc_info;
	int mac_for_pdev = mac_id;
	QDF_STATUS status;

	if (qdf_unlikely(!dp_soc_is_full_mon_enable(pdev)))
		return quota;

	mon_dest_srng = dp_rxdma_get_mon_dst_ring(pdev, mac_for_pdev);

	if (qdf_unlikely(!mon_dest_srng ||
			 !hal_srng_initialized(mon_dest_srng))) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  FL("HAL Monitor Destination Ring Init Failed -- %pK"),
			  mon_dest_srng);
		goto done;
	}

	hal_soc = soc->hal_soc;

	qdf_assert_always(hal_soc && pdev);

	qdf_spin_lock_bh(&pdev->mon_lock);

	desc_info = pdev->mon_desc;

	if (qdf_unlikely(hal_srng_access_start(hal_soc, mon_dest_srng))) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  FL("HAL Monitor Destination Ring access Failed -- %pK"),
			  mon_dest_srng);
		goto done1;
	}

	/* Each entry in mon dest ring carries mpdu data
	 * reap all msdus for a mpdu and form skb chain
	 */
	while (qdf_likely(ring_desc =
			  hal_srng_dst_peek(hal_soc, mon_dest_srng))) {
		head_msdu = NULL;
		tail_msdu = NULL;
		rx_bufs_reaped = dp_rx_mon_mpdu_reap(soc, mac_id,
						     ring_desc, &head_msdu,
						     &tail_msdu, &head_desc,
						     &tail_desc);

		/* Assert if end_of_ppdu is zero and number of reaped buffers
		 * are zero.
		 */
		if (qdf_unlikely(!desc_info->end_of_ppdu && !rx_bufs_reaped)) {
			dp_print_ring_stats(pdev);
			qdf_assert_always(0);
		}

		rx_mon_stats->mon_rx_bufs_reaped_dest += rx_bufs_reaped;

		/* replenish rx_bufs_reaped buffers back to
		 * RxDMA Monitor buffer ring
		 */
		if (rx_bufs_reaped) {
			status = dp_rx_buffers_replenish(soc, mac_id,
							 dp_rxdma_get_mon_buf_ring(pdev,
										   mac_for_pdev),
							 dp_rx_get_mon_desc_pool(soc, mac_id,
										 pdev->pdev_id),
										 rx_bufs_reaped,
										 &head_desc, &tail_desc);
			if (status != QDF_STATUS_SUCCESS)
				qdf_assert_always(0);

			rx_mon_stats->mon_rx_bufs_replenished_dest += rx_bufs_reaped;
		}

		head_desc = NULL;
		tail_desc = NULL;

		/* If end_of_ppdu is zero, it is a valid data mpdu
		 *    a. Add head_msdu and tail_msdu to mpdu list
		 *    b. continue reaping next SW_MONITOR_RING descriptor
		 */

		if (!desc_info->end_of_ppdu) {
			/*
			 * In case of rxdma error, MPDU is dropped
			 * from sw_monitor_ring descriptor.
			 * in this case, head_msdu remains NULL.
			 * move srng to next and continue reaping next entry
			 */
			if (!head_msdu) {
				ring_desc = hal_srng_dst_get_next(hal_soc,
								  mon_dest_srng);
				continue;
			}

			/*
			 * Prepare a MPDU object which holds chain of msdus
			 * and MPDU specific status and add this is to
			 * monitor mpdu queue
			 */
			mon_mpdu = dp_rx_mon_prepare_mon_mpdu(pdev,
							      head_msdu,
							      tail_msdu);

			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				  FL("Dest_srng: %pK MPDU_OBJ: %pK "
				  "head_msdu: %pK tail_msdu: %pK -- "),
				  mon_dest_srng,
				  mon_mpdu,
				  head_msdu,
				  tail_msdu);

			TAILQ_INSERT_TAIL(&pdev->mon_mpdu_q,
					  mon_mpdu,
					  mpdu_list_elem);

			head_msdu = NULL;
			tail_msdu = NULL;
			ring_desc = hal_srng_dst_get_next(hal_soc,
							  mon_dest_srng);
			continue;
		}

		/*
		 * end_of_ppdu is one,
		 *  a. update ppdu_done stattistics
		 *  b. Replenish buffers back to mon buffer ring
		 *  c. reap status ring for a PPDU and deliver all mpdus
		 *     to upper layer
		 */
		rx_mon_stats->dest_ppdu_done++;

		if (pdev->ppdu_info.com_info.ppdu_id !=
		    pdev->mon_desc->ppdu_id) {
			pdev->rx_mon_stats.ppdu_id_mismatch++;
			qdf_err("PPDU id mismatch, status_ppdu_id: %d"
				"dest_ppdu_id: %d status_ppdu_done: %d "
				"dest_ppdu_done: %d ppdu_id_mismatch_cnt: %u"
				"dest_mpdu_drop: %u",
				pdev->ppdu_info.com_info.ppdu_id,
				pdev->mon_desc->ppdu_id,
				pdev->rx_mon_stats.status_ppdu_done,
				pdev->rx_mon_stats.dest_ppdu_done,
				pdev->rx_mon_stats.ppdu_id_mismatch,
				pdev->rx_mon_stats.dest_mpdu_drop);

		/* WAR: It is observed that in some cases, status ring ppdu_id
		 *     and destination ring ppdu_id doesn't match.
		 *     Following WAR is added to fix it.
		 *     a. If status ppdu_id is less than destination ppdu_id,
		 *        hold onto destination ring until ppdu_id matches
		 *     b. If status ppdu_id is greater than destination ring
		 *        ppdu_Id, move tp in destination ring.
		 */
			if (pdev->ppdu_info.com_info.ppdu_id <
			    pdev->mon_desc->ppdu_id) {
				break;
			} else {
				ring_desc = hal_srng_dst_get_next(hal_soc,
								  mon_dest_srng);
				continue;
			}
		}

		/*
		 * At this point, end_of_ppdu is one here,
		 * When 'end_of_ppdu' is one, status buffer_count and
		 * status_buf_addr must be valid.
		 *
		 *  Assert if
		 *  a. status_buf_count is zero
		 *  b. status_buf.paddr is NULL
		 */
		if (!pdev->mon_desc->status_buf_count ||
		    !pdev->mon_desc->status_buf.paddr) {
			qdf_assert_always(0);
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				  FL("Status buffer info is NULL"
				  "status_buf_count: %d"
				  "status_buf_addr: %pK"
				  "ring_desc: %pK-- "),
				  pdev->mon_desc->status_buf_count,
				  pdev->mon_desc->status_buf.paddr,
				  ring_desc);
				goto done2;
		}

		/* Deliver all MPDUs for a PPDU */
		dp_rx_monitor_deliver_ppdu(soc, mac_id);

		hal_srng_dst_get_next(hal_soc, mon_dest_srng);
		break;
	}

done2:
	hal_srng_access_end(hal_soc, mon_dest_srng);

done1:
	qdf_spin_unlock_bh(&pdev->mon_lock);

done:
	return quota;
}

/**
 * dp_full_mon_attach() - attach full monitor mode
 *              resources
 * @pdev: Datapath PDEV handle
 *
 * Return: void
 */
void dp_full_mon_attach(struct dp_pdev *pdev)
{
	struct dp_soc *soc = pdev->soc;

	if (!soc->full_mon_mode) {
		qdf_debug("Full monitor is not enabled");
		return;
	}

	pdev->mon_desc = qdf_mem_malloc(sizeof(struct hal_rx_mon_desc_info));

	if (!pdev->mon_desc) {
		qdf_err("Memory allocation failed for hal_rx_mon_desc_info ");
		return;
	}
	TAILQ_INIT(&pdev->mon_mpdu_q);
}

/**
 * dp_full_mon_detach() - detach full monitor mode
 *              resources
 * @pdev: Datapath PDEV handle
 *
 * Return: void
 *
 */
void dp_full_mon_detach(struct dp_pdev *pdev)
{
	struct dp_soc *soc = pdev->soc;
	struct dp_mon_mpdu *mpdu = NULL;
	struct dp_mon_mpdu *temp_mpdu = NULL;

	if (!soc->full_mon_mode) {
		qdf_debug("Full monitor is not enabled");
		return;
	}

	if (pdev->mon_desc)
		qdf_mem_free(pdev->mon_desc);

	if (!TAILQ_EMPTY(&pdev->mon_mpdu_q)) {
		TAILQ_FOREACH_SAFE(mpdu,
				   &pdev->mon_mpdu_q,
				   mpdu_list_elem,
				   temp_mpdu) {
			qdf_mem_free(mpdu);
		}
	}
}
#endif
