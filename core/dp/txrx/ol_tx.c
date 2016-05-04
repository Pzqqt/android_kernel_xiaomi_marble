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

/* OS abstraction libraries */
#include <qdf_nbuf.h>           /* qdf_nbuf_t, etc. */
#include <qdf_atomic.h>         /* qdf_atomic_read, etc. */
#include <qdf_util.h>           /* qdf_unlikely */

/* APIs for other modules */
#include <htt.h>                /* HTT_TX_EXT_TID_MGMT */
#include <ol_htt_tx_api.h>      /* htt_tx_desc_tid */

/* internal header files relevant for all systems */
#include <ol_txrx_internal.h>   /* TXRX_ASSERT1 */
#include <ol_tx_desc.h>         /* ol_tx_desc */
#include <ol_tx_send.h>         /* ol_tx_send */
#include <ol_txrx.h>

/* internal header files relevant only for HL systems */
#include <ol_tx_queue.h>        /* ol_tx_enqueue */

/* internal header files relevant only for specific systems (Pronto) */
#include <ol_txrx_encap.h>      /* OL_TX_ENCAP, etc */
#include <ol_tx.h>

#ifdef WLAN_FEATURE_FASTPATH
#include <hif.h>              /* HIF_DEVICE */
#include <htc_api.h>    /* Layering violation, but required for fast path */
#include <htt_internal.h>
#include <htt_types.h>        /* htc_endpoint */
#include <cdp_txrx_peer_ops.h>

int ce_send_fast(struct CE_handle *copyeng, qdf_nbuf_t *msdus,
		 unsigned int num_msdus, unsigned int transfer_id);
#endif  /* WLAN_FEATURE_FASTPATH */

/*
 * The TXRX module doesn't accept tx frames unless the target has
 * enough descriptors for them.
 * For LL, the TXRX descriptor pool is sized to match the target's
 * descriptor pool.  Hence, if the descriptor allocation in TXRX
 * succeeds, that guarantees that the target has room to accept
 * the new tx frame.
 */
#define ol_tx_prepare_ll(tx_desc, vdev, msdu, msdu_info)		\
	do {								\
		struct ol_txrx_pdev_t *pdev = vdev->pdev;		\
		(msdu_info)->htt.info.frame_type = pdev->htt_pkt_type;	\
		tx_desc = ol_tx_desc_ll(pdev, vdev, msdu, msdu_info);	\
		if (qdf_unlikely(!tx_desc)) {				\
			TXRX_STATS_MSDU_LIST_INCR(			\
				pdev, tx.dropped.host_reject, msdu);	\
			return msdu; /* the list of unaccepted MSDUs */	\
		}							\
	} while (0)

#if defined(FEATURE_TSO)
/**
 * ol_tx_prepare_tso() - Given a jumbo msdu, prepare the TSO
 * related information in the msdu_info meta data
 * @vdev: virtual device handle
 * @msdu: network buffer
 * @msdu_info: meta data associated with the msdu
 *
 * Return: 0 - success, >0 - error
 */
static inline uint8_t ol_tx_prepare_tso(ol_txrx_vdev_handle vdev,
	 qdf_nbuf_t msdu, struct ol_txrx_msdu_info_t *msdu_info)
{
	msdu_info->tso_info.curr_seg = NULL;
	if (qdf_nbuf_is_tso(msdu)) {
		int num_seg = qdf_nbuf_get_tso_num_seg(msdu);
		msdu_info->tso_info.tso_seg_list = NULL;
		msdu_info->tso_info.num_segs = num_seg;
		while (num_seg) {
			struct qdf_tso_seg_elem_t *tso_seg =
				ol_tso_alloc_segment(vdev->pdev);
			if (tso_seg) {
				tso_seg->next =
					msdu_info->tso_info.tso_seg_list;
				msdu_info->tso_info.tso_seg_list
					= tso_seg;
				num_seg--;
			} else {
				struct qdf_tso_seg_elem_t *next_seg;
				struct qdf_tso_seg_elem_t *free_seg =
					msdu_info->tso_info.tso_seg_list;
				qdf_print("TSO seg alloc failed!\n");
				while (free_seg) {
					next_seg = free_seg->next;
					ol_tso_free_segment(vdev->pdev,
						 free_seg);
					free_seg = next_seg;
				}
				return 1;
			}
		}
		qdf_nbuf_get_tso_info(vdev->pdev->osdev,
			msdu, &(msdu_info->tso_info));
		msdu_info->tso_info.curr_seg =
			msdu_info->tso_info.tso_seg_list;
		num_seg = msdu_info->tso_info.num_segs;
	} else {
		msdu_info->tso_info.is_tso = 0;
		msdu_info->tso_info.num_segs = 1;
	}
	return 0;
}
#endif

/**
 * ol_tx_data() - send data frame
 * @vdev: virtual device handle
 * @skb: skb
 *
 * Return: skb/NULL for success
 */
qdf_nbuf_t ol_tx_data(ol_txrx_vdev_handle vdev, qdf_nbuf_t skb)
{
	void *qdf_ctx = cds_get_context(QDF_MODULE_ID_QDF_DEVICE);
	struct ol_txrx_pdev_t *pdev;
	qdf_nbuf_t ret;
	QDF_STATUS status;

	if (qdf_unlikely(!vdev)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_WARN,
			"%s:vdev is null", __func__);
		return skb;
	} else {
		pdev = vdev->pdev;
	}

	if (qdf_unlikely(!pdev)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_WARN,
			"%s:pdev is null", __func__);
		return skb;
	}
	if (qdf_unlikely(!qdf_ctx)) {
		TXRX_PRINT(TXRX_PRINT_LEVEL_ERR,
			"%s:qdf_ctx is null", __func__);
		return skb;
	}

	status = qdf_nbuf_map_single(qdf_ctx, skb, QDF_DMA_TO_DEVICE);
	if (qdf_unlikely(status != QDF_STATUS_SUCCESS)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_WARN,
			"%s: nbuf map failed", __func__);
		return skb;
	}

	if ((ol_cfg_is_ip_tcp_udp_checksum_offload_enabled(pdev->ctrl_pdev))
		&& (qdf_nbuf_get_protocol(skb) == htons(ETH_P_IP))
		&& (qdf_nbuf_get_ip_summed(skb) == CHECKSUM_PARTIAL))
		qdf_nbuf_set_ip_summed(skb, CHECKSUM_COMPLETE);

	/* Terminate the (single-element) list of tx frames */
	qdf_nbuf_set_next(skb, NULL);
	ret = OL_TX_LL(vdev, skb);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_WARN,
			"%s: Failed to tx", __func__);
		qdf_nbuf_unmap_single(qdf_ctx, ret, QDF_DMA_TO_DEVICE);
		return ret;
	}

	return NULL;
}

#ifdef IPA_OFFLOAD
/**
 * ol_tx_send_ipa_data_frame() - send IPA data frame
 * @vdev: vdev
 * @skb: skb
 *
 * Return: skb/ NULL is for success
 */
qdf_nbuf_t ol_tx_send_ipa_data_frame(void *vdev,
			qdf_nbuf_t skb)
{
	ol_txrx_pdev_handle pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	qdf_nbuf_t ret;

	if (qdf_unlikely(!pdev)) {
		TXRX_PRINT(TXRX_PRINT_LEVEL_ERR,
			"%s: pdev is NULL", __func__);
		return skb;
	}

	if ((ol_cfg_is_ip_tcp_udp_checksum_offload_enabled(pdev->ctrl_pdev))
		&& (qdf_nbuf_get_protocol(skb) == htons(ETH_P_IP))
		&& (qdf_nbuf_get_ip_summed(skb) == CHECKSUM_PARTIAL))
		qdf_nbuf_set_ip_summed(skb, CHECKSUM_COMPLETE);

	/* Terminate the (single-element) list of tx frames */
	qdf_nbuf_set_next(skb, NULL);
	ret = OL_TX_LL((struct ol_txrx_vdev_t *)vdev, skb);
	if (ret) {
		TXRX_PRINT(TXRX_PRINT_LEVEL_WARN,
			"%s: Failed to tx", __func__);
		return ret;
	}

	return NULL;
}
#endif


#if defined(FEATURE_TSO)
qdf_nbuf_t ol_tx_ll(ol_txrx_vdev_handle vdev, qdf_nbuf_t msdu_list)
{
	qdf_nbuf_t msdu = msdu_list;
	struct ol_txrx_msdu_info_t msdu_info;

	msdu_info.htt.info.l2_hdr_type = vdev->pdev->htt_pkt_type;
	msdu_info.htt.action.tx_comp_req = 0;
	/*
	 * The msdu_list variable could be used instead of the msdu var,
	 * but just to clarify which operations are done on a single MSDU
	 * vs. a list of MSDUs, use a distinct variable for single MSDUs
	 * within the list.
	 */
	while (msdu) {
		qdf_nbuf_t next;
		struct ol_tx_desc_t *tx_desc;
		int segments = 1;

		msdu_info.htt.info.ext_tid = qdf_nbuf_get_tid(msdu);
		msdu_info.peer = NULL;

		if (qdf_unlikely(ol_tx_prepare_tso(vdev, msdu, &msdu_info))) {
			qdf_print("ol_tx_prepare_tso failed\n");
			TXRX_STATS_MSDU_LIST_INCR(vdev->pdev,
				 tx.dropped.host_reject, msdu);
			return msdu;
		}

		segments = msdu_info.tso_info.num_segs;

		/*
		 * The netbuf may get linked into a different list inside the
		 * ol_tx_send function, so store the next pointer before the
		 * tx_send call.
		 */
		next = qdf_nbuf_next(msdu);
		/* init the current segment to the 1st segment in the list */
		while (segments) {

			if (msdu_info.tso_info.curr_seg)
				QDF_NBUF_CB_PADDR(msdu) =
					msdu_info.tso_info.curr_seg->
					seg.tso_frags[0].paddr_low_32;

			segments--;

			/**
			* if this is a jumbo nbuf, then increment the number
			* of nbuf users for each additional segment of the msdu.
			* This will ensure that the skb is freed only after
			* receiving tx completion for all segments of an nbuf
			*/
			if (segments)
				qdf_nbuf_inc_users(msdu);

			ol_tx_prepare_ll(tx_desc, vdev, msdu, &msdu_info);

			/*
			 * If debug display is enabled, show the meta-data being
			 * downloaded to the target via the HTT tx descriptor.
			 */
			htt_tx_desc_display(tx_desc->htt_tx_desc);

			ol_tx_send(vdev->pdev, tx_desc, msdu);

			if (msdu_info.tso_info.curr_seg) {
				msdu_info.tso_info.curr_seg =
					 msdu_info.tso_info.curr_seg->next;
			}

			qdf_nbuf_reset_num_frags(msdu);

			if (msdu_info.tso_info.is_tso) {
				TXRX_STATS_TSO_INC_SEG(vdev->pdev);
				TXRX_STATS_TSO_INC_SEG_IDX(vdev->pdev);
			}
		} /* while segments */

		msdu = next;
		if (msdu_info.tso_info.is_tso) {
			TXRX_STATS_TSO_INC_MSDU_IDX(vdev->pdev);
			TXRX_STATS_TSO_RESET_MSDU(vdev->pdev);
		}
	} /* while msdus */
	return NULL;            /* all MSDUs were accepted */
}
#else /* TSO */

qdf_nbuf_t ol_tx_ll(ol_txrx_vdev_handle vdev, qdf_nbuf_t msdu_list)
{
	qdf_nbuf_t msdu = msdu_list;
	struct ol_txrx_msdu_info_t msdu_info;

	msdu_info.htt.info.l2_hdr_type = vdev->pdev->htt_pkt_type;
	msdu_info.htt.action.tx_comp_req = 0;
	msdu_info.tso_info.is_tso = 0;
	/*
	 * The msdu_list variable could be used instead of the msdu var,
	 * but just to clarify which operations are done on a single MSDU
	 * vs. a list of MSDUs, use a distinct variable for single MSDUs
	 * within the list.
	 */
	while (msdu) {
		qdf_nbuf_t next;
		struct ol_tx_desc_t *tx_desc;

		msdu_info.htt.info.ext_tid = qdf_nbuf_get_tid(msdu);
		msdu_info.peer = NULL;
		ol_tx_prepare_ll(tx_desc, vdev, msdu, &msdu_info);

		/*
		 * If debug display is enabled, show the meta-data being
		 * downloaded to the target via the HTT tx descriptor.
		 */
		htt_tx_desc_display(tx_desc->htt_tx_desc);
		/*
		 * The netbuf may get linked into a different list inside the
		 * ol_tx_send function, so store the next pointer before the
		 * tx_send call.
		 */
		next = qdf_nbuf_next(msdu);
		ol_tx_send(vdev->pdev, tx_desc, msdu);
		msdu = next;
	}
	return NULL;            /* all MSDUs were accepted */
}
#endif /* TSO */

#ifdef WLAN_FEATURE_FASTPATH
/**
 * ol_tx_prepare_ll_fast() Alloc and prepare Tx descriptor
 *
 * Allocate and prepare Tx descriptor with msdu and fragment descritor
 * inforamtion.
 *
 * @pdev: pointer to ol pdev handle
 * @vdev: pointer to ol vdev handle
 * @msdu: linked list of msdu packets
 * @pkt_download_len: packet download length
 * @ep_id: endpoint ID
 * @msdu_info: Handle to msdu_info
 *
 * Return: Pointer to Tx descriptor
 */
static inline struct ol_tx_desc_t *
ol_tx_prepare_ll_fast(struct ol_txrx_pdev_t *pdev,
		      ol_txrx_vdev_handle vdev, qdf_nbuf_t msdu,
		      uint32_t pkt_download_len, uint32_t ep_id,
		      struct ol_txrx_msdu_info_t *msdu_info)
{
	struct ol_tx_desc_t *tx_desc = NULL;
	uint32_t *htt_tx_desc;
	void *htc_hdr_vaddr;
	u_int32_t num_frags, i;

	tx_desc = ol_tx_desc_alloc_wrapper(pdev, vdev, msdu_info);
	if (qdf_unlikely(!tx_desc))
		return NULL;

	tx_desc->netbuf = msdu;
	if (msdu_info->tso_info.is_tso) {
		tx_desc->tso_desc = msdu_info->tso_info.curr_seg;
		tx_desc->pkt_type = OL_TX_FRM_TSO;
		TXRX_STATS_MSDU_INCR(pdev, tx.tso.tso_pkts, msdu);
	} else {
		tx_desc->pkt_type = OL_TX_FRM_STD;
	}

	htt_tx_desc = tx_desc->htt_tx_desc;

	/* Make sure frags num is set to 0 */
	/*
	 * Do this here rather than in hardstart, so
	 * that we can hopefully take only one cache-miss while
	 * accessing skb->cb.
	 */

	/* HTT Header */
	/* TODO : Take care of multiple fragments */

	/* TODO: Precompute and store paddr in ol_tx_desc_t */
	/* Virtual address of the HTT/HTC header, added by driver */
	htc_hdr_vaddr = (char *)htt_tx_desc - HTC_HEADER_LEN;
	htt_tx_desc_init(pdev->htt_pdev, htt_tx_desc,
			 tx_desc->htt_tx_desc_paddr, tx_desc->id, msdu,
			 &msdu_info->htt, &msdu_info->tso_info,
			NULL, vdev->opmode == wlan_op_mode_ocb);

	num_frags = qdf_nbuf_get_num_frags(msdu);
	/* num_frags are expected to be 2 max */
	num_frags = (num_frags > QDF_NBUF_CB_TX_MAX_EXTRA_FRAGS)
		? QDF_NBUF_CB_TX_MAX_EXTRA_FRAGS
		: num_frags;
#if defined(HELIUMPLUS_PADDR64)
	/*
	 * Use num_frags - 1, since 1 frag is used to store
	 * the HTT/HTC descriptor
	 * Refer to htt_tx_desc_init()
	 */
	htt_tx_desc_num_frags(pdev->htt_pdev, tx_desc->htt_frag_desc,
			      num_frags - 1);
#else /* ! defined(HELIUMPLUSPADDR64) */
	htt_tx_desc_num_frags(pdev->htt_pdev, tx_desc->htt_tx_desc,
			      num_frags-1);
#endif /* defined(HELIUMPLUS_PADDR64) */
	if (msdu_info->tso_info.is_tso) {
		htt_tx_desc_fill_tso_info(pdev->htt_pdev,
			 tx_desc->htt_frag_desc, &msdu_info->tso_info);
		TXRX_STATS_TSO_SEG_UPDATE(pdev,
			 msdu_info->tso_info.curr_seg->seg);
	} else {
		for (i = 1; i < num_frags; i++) {
			qdf_size_t frag_len;
			qdf_dma_addr_t frag_paddr;

			frag_len = qdf_nbuf_get_frag_len(msdu, i);
			frag_paddr = qdf_nbuf_get_frag_paddr(msdu, i);
#if defined(HELIUMPLUS_PADDR64)
			htt_tx_desc_frag(pdev->htt_pdev, tx_desc->htt_frag_desc,
					 i - 1, frag_paddr, frag_len);
#if defined(HELIUMPLUS_DEBUG)
			qdf_print("%s:%d: htt_fdesc=%p frag=%d frag_paddr=0x%0llx len=%zu",
				  __func__, __LINE__, tx_desc->htt_frag_desc,
				  i-1, frag_paddr, frag_len);
			dump_pkt(netbuf, frag_paddr, 64);
#endif /* HELIUMPLUS_DEBUG */
#else /* ! defined(HELIUMPLUSPADDR64) */
			htt_tx_desc_frag(pdev->htt_pdev, tx_desc->htt_tx_desc,
					 i - 1, frag_paddr, frag_len);
#endif /* defined(HELIUMPLUS_PADDR64) */
		}
	}

	/*
	 *  Do we want to turn on word_stream bit-map here ? For linux, non-TSO
	 *  this is not required. We still have to mark the swap bit correctly,
	 *  when posting to the ring
	 */
	/* Check to make sure, data download length is correct */

	/*
	 * TODO : Can we remove this check and always download a fixed length ?
	 * */
	if (qdf_unlikely(qdf_nbuf_len(msdu) < pkt_download_len))
		pkt_download_len = qdf_nbuf_len(msdu);

	/* Fill the HTC header information */
	/*
	 * Passing 0 as the seq_no field, we can probably get away
	 * with it for the time being, since this is not checked in f/w
	 */
	/* TODO : Prefill this, look at multi-fragment case */
	HTC_TX_DESC_FILL(htc_hdr_vaddr, pkt_download_len, ep_id, 0);

	return tx_desc;
}
#if defined(FEATURE_TSO)
/**
 * ol_tx_ll_fast() Update metadata information and send msdu to HIF/CE
 *
 * @vdev: handle to ol_txrx_vdev_t
 * @msdu_list: msdu list to be sent out.
 *
 * Return: on success return NULL, pointer to nbuf when it fails to send.
 */
qdf_nbuf_t
ol_tx_ll_fast(ol_txrx_vdev_handle vdev, qdf_nbuf_t msdu_list)
{
	qdf_nbuf_t msdu = msdu_list;
	struct ol_txrx_pdev_t *pdev = vdev->pdev;
	uint32_t pkt_download_len =
		((struct htt_pdev_t *)(pdev->htt_pdev))->download_len;
	uint32_t ep_id = HTT_EPID_GET(pdev->htt_pdev);
	struct ol_txrx_msdu_info_t msdu_info;

	msdu_info.htt.info.l2_hdr_type = vdev->pdev->htt_pkt_type;
	msdu_info.htt.action.tx_comp_req = 0;
	/*
	 * The msdu_list variable could be used instead of the msdu var,
	 * but just to clarify which operations are done on a single MSDU
	 * vs. a list of MSDUs, use a distinct variable for single MSDUs
	 * within the list.
	 */
	while (msdu) {
		qdf_nbuf_t next;
		struct ol_tx_desc_t *tx_desc;
		int segments = 1;

		msdu_info.htt.info.ext_tid = qdf_nbuf_get_tid(msdu);
		msdu_info.peer = NULL;

		if (qdf_unlikely(ol_tx_prepare_tso(vdev, msdu, &msdu_info))) {
			qdf_print("ol_tx_prepare_tso failed\n");
			TXRX_STATS_MSDU_LIST_INCR(vdev->pdev,
				 tx.dropped.host_reject, msdu);
			return msdu;
		}

		segments = msdu_info.tso_info.num_segs;

		/*
		 * The netbuf may get linked into a different list
		 * inside the ce_send_fast function, so store the next
		 * pointer before the ce_send call.
		 */
		next = qdf_nbuf_next(msdu);
		/* init the current segment to the 1st segment in the list */
		while (segments) {

			if (msdu_info.tso_info.curr_seg)
				QDF_NBUF_CB_PADDR(msdu) = msdu_info.tso_info.
					curr_seg->seg.tso_frags[0].paddr_low_32;

			segments--;

			/**
			* if this is a jumbo nbuf, then increment the number
			* of nbuf users for each additional segment of the msdu.
			* This will ensure that the skb is freed only after
			* receiving tx completion for all segments of an nbuf
			*/
			if (segments)
				qdf_nbuf_inc_users(msdu);

			msdu_info.htt.info.frame_type = pdev->htt_pkt_type;
			msdu_info.htt.info.vdev_id = vdev->vdev_id;
			msdu_info.htt.action.cksum_offload =
				qdf_nbuf_get_tx_cksum(msdu);
			switch (qdf_nbuf_get_exemption_type(msdu)) {
			case QDF_NBUF_EXEMPT_NO_EXEMPTION:
			case QDF_NBUF_EXEMPT_ON_KEY_MAPPING_KEY_UNAVAILABLE:
				/* We want to encrypt this frame */
				msdu_info.htt.action.do_encrypt = 1;
				break;
			case QDF_NBUF_EXEMPT_ALWAYS:
				/* We don't want to encrypt this frame */
				msdu_info.htt.action.do_encrypt = 0;
				break;
			default:
				msdu_info.htt.action.do_encrypt = 1;
				qdf_assert(0);
				break;
			}

			tx_desc = ol_tx_prepare_ll_fast(pdev, vdev, msdu,
						  pkt_download_len, ep_id,
						  &msdu_info);

			if (qdf_likely(tx_desc)) {
				/*
				 * If debug display is enabled, show the meta
				 * data being downloaded to the target via the
				 * HTT tx descriptor.
				 */
				htt_tx_desc_display(tx_desc->htt_tx_desc);
				if ((0 == ce_send_fast(pdev->ce_tx_hdl, &msdu,
						       1, ep_id))) {
					/*
					 * The packet could not be sent.
					 * Free the descriptor, return the
					 * packet to the caller.
					 */
					ol_tx_desc_free(pdev, tx_desc);
					return msdu;
				}
				if (msdu_info.tso_info.curr_seg) {
					msdu_info.tso_info.curr_seg =
					msdu_info.tso_info.curr_seg->next;
				}

				if (msdu_info.tso_info.is_tso) {
					qdf_nbuf_reset_num_frags(msdu);
					TXRX_STATS_TSO_INC_SEG(vdev->pdev);
					TXRX_STATS_TSO_INC_SEG_IDX(vdev->pdev);
				}
			} else {
				TXRX_STATS_MSDU_LIST_INCR(
					pdev, tx.dropped.host_reject, msdu);
				/* the list of unaccepted MSDUs */
				return msdu;
			}
		} /* while segments */

		msdu = next;
		if (msdu_info.tso_info.is_tso) {
			TXRX_STATS_TSO_INC_MSDU_IDX(vdev->pdev);
			TXRX_STATS_TSO_RESET_MSDU(vdev->pdev);
		}
	} /* while msdus */
	return NULL; /* all MSDUs were accepted */
}
#else
qdf_nbuf_t
ol_tx_ll_fast(ol_txrx_vdev_handle vdev, qdf_nbuf_t msdu_list)
{
	qdf_nbuf_t msdu = msdu_list;
	struct ol_txrx_pdev_t *pdev = vdev->pdev;
	uint32_t pkt_download_len =
		((struct htt_pdev_t *)(pdev->htt_pdev))->download_len;
	uint32_t ep_id = HTT_EPID_GET(pdev->htt_pdev);
	struct ol_txrx_msdu_info_t msdu_info;

	msdu_info.htt.info.l2_hdr_type = vdev->pdev->htt_pkt_type;
	msdu_info.htt.action.tx_comp_req = 0;
	msdu_info.tso_info.is_tso = 0;
	/*
	 * The msdu_list variable could be used instead of the msdu var,
	 * but just to clarify which operations are done on a single MSDU
	 * vs. a list of MSDUs, use a distinct variable for single MSDUs
	 * within the list.
	 */
	while (msdu) {
		qdf_nbuf_t next;
		struct ol_tx_desc_t *tx_desc;

		msdu_info.htt.info.ext_tid = qdf_nbuf_get_tid(msdu);
		msdu_info.peer = NULL;

		msdu_info.htt.info.frame_type = pdev->htt_pkt_type;
		msdu_info.htt.info.vdev_id = vdev->vdev_id;
		msdu_info.htt.action.cksum_offload =
			qdf_nbuf_get_tx_cksum(msdu);
		switch (qdf_nbuf_get_exemption_type(msdu)) {
		case QDF_NBUF_EXEMPT_NO_EXEMPTION:
		case QDF_NBUF_EXEMPT_ON_KEY_MAPPING_KEY_UNAVAILABLE:
			/* We want to encrypt this frame */
			msdu_info.htt.action.do_encrypt = 1;
			break;
		case QDF_NBUF_EXEMPT_ALWAYS:
			/* We don't want to encrypt this frame */
			msdu_info.htt.action.do_encrypt = 0;
			break;
		default:
			msdu_info.htt.action.do_encrypt = 1;
			qdf_assert(0);
			break;
		}

		tx_desc = ol_tx_prepare_ll_fast(pdev, vdev, msdu,
					  pkt_download_len, ep_id,
					  &msdu_info);

		if (qdf_likely(tx_desc)) {
			/*
			 * If debug display is enabled, show the meta-data being
			 * downloaded to the target via the HTT tx descriptor.
			 */
			htt_tx_desc_display(tx_desc->htt_tx_desc);
			/*
			 * The netbuf may get linked into a different list
			 * inside the ce_send_fast function, so store the next
			 * pointer before the ce_send call.
			 */
			next = qdf_nbuf_next(msdu);
			if ((0 == ce_send_fast(pdev->ce_tx_hdl, &msdu, 1,
					       ep_id))) {
				/* The packet could not be sent */
				/* Free the descriptor, return the packet to the
				 * caller */
				ol_tx_desc_free(pdev, tx_desc);
				return msdu;
			}
			msdu = next;
		} else {
			TXRX_STATS_MSDU_LIST_INCR(
				pdev, tx.dropped.host_reject, msdu);
			return msdu; /* the list of unaccepted MSDUs */
		}
	}

	return NULL; /* all MSDUs were accepted */
}
#endif /* FEATURE_TSO */
#endif /* WLAN_FEATURE_FASTPATH */

#ifdef WLAN_FEATURE_FASTPATH
/**
 * ol_tx_ll_wrapper() wrapper to ol_tx_ll
 *
 */
qdf_nbuf_t
ol_tx_ll_wrapper(ol_txrx_vdev_handle vdev, qdf_nbuf_t msdu_list)
{
	struct hif_opaque_softc *hif_device =
		(struct hif_opaque_softc *)cds_get_context(QDF_MODULE_ID_HIF);

	if (qdf_likely(hif_device && hif_is_fastpath_mode_enabled(hif_device)))
		msdu_list = ol_tx_ll_fast(vdev, msdu_list);
	else
		msdu_list = ol_tx_ll(vdev, msdu_list);

	return msdu_list;
}
#else
qdf_nbuf_t
ol_tx_ll_wrapper(ol_txrx_vdev_handle vdev, qdf_nbuf_t msdu_list)
{
	return ol_tx_ll(vdev, msdu_list);
}
#endif  /* WLAN_FEATURE_FASTPATH */

#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL

#define OL_TX_VDEV_PAUSE_QUEUE_SEND_MARGIN 400
#define OL_TX_VDEV_PAUSE_QUEUE_SEND_PERIOD_MS 5
static void ol_tx_vdev_ll_pause_queue_send_base(struct ol_txrx_vdev_t *vdev)
{
	int max_to_accept;

	qdf_spin_lock_bh(&vdev->ll_pause.mutex);
	if (vdev->ll_pause.paused_reason) {
		qdf_spin_unlock_bh(&vdev->ll_pause.mutex);
		return;
	}

	/*
	 * Send as much of the backlog as possible, but leave some margin
	 * of unallocated tx descriptors that can be used for new frames
	 * being transmitted by other vdevs.
	 * Ideally there would be a scheduler, which would not only leave
	 * some margin for new frames for other vdevs, but also would
	 * fairly apportion the tx descriptors between multiple vdevs that
	 * have backlogs in their pause queues.
	 * However, the fairness benefit of having a scheduler for frames
	 * from multiple vdev's pause queues is not sufficient to outweigh
	 * the extra complexity.
	 */
	max_to_accept = vdev->pdev->tx_desc.num_free -
		OL_TX_VDEV_PAUSE_QUEUE_SEND_MARGIN;
	while (max_to_accept > 0 && vdev->ll_pause.txq.depth) {
		qdf_nbuf_t tx_msdu;
		max_to_accept--;
		vdev->ll_pause.txq.depth--;
		tx_msdu = vdev->ll_pause.txq.head;
		if (tx_msdu) {
			vdev->ll_pause.txq.head = qdf_nbuf_next(tx_msdu);
			if (NULL == vdev->ll_pause.txq.head)
				vdev->ll_pause.txq.tail = NULL;
			qdf_nbuf_set_next(tx_msdu, NULL);
			QDF_NBUF_UPDATE_TX_PKT_COUNT(tx_msdu,
						QDF_NBUF_TX_PKT_TXRX_DEQUEUE);
			tx_msdu = ol_tx_ll_wrapper(vdev, tx_msdu);
			/*
			 * It is unexpected that ol_tx_ll would reject the frame
			 * since we checked that there's room for it, though
			 * there's an infinitesimal possibility that between the
			 * time we checked the room available and now, a
			 * concurrent batch of tx frames used up all the room.
			 * For simplicity, just drop the frame.
			 */
			if (tx_msdu) {
				qdf_nbuf_unmap(vdev->pdev->osdev, tx_msdu,
					       QDF_DMA_TO_DEVICE);
				qdf_nbuf_tx_free(tx_msdu, QDF_NBUF_PKT_ERROR);
			}
		}
	}
	if (vdev->ll_pause.txq.depth) {
		qdf_timer_stop(&vdev->ll_pause.timer);
		qdf_timer_start(&vdev->ll_pause.timer,
					OL_TX_VDEV_PAUSE_QUEUE_SEND_PERIOD_MS);
		vdev->ll_pause.is_q_timer_on = true;
		if (vdev->ll_pause.txq.depth >= vdev->ll_pause.max_q_depth)
			vdev->ll_pause.q_overflow_cnt++;
	}

	qdf_spin_unlock_bh(&vdev->ll_pause.mutex);
}

static qdf_nbuf_t
ol_tx_vdev_pause_queue_append(struct ol_txrx_vdev_t *vdev,
			      qdf_nbuf_t msdu_list, uint8_t start_timer)
{
	qdf_spin_lock_bh(&vdev->ll_pause.mutex);
	while (msdu_list &&
	       vdev->ll_pause.txq.depth < vdev->ll_pause.max_q_depth) {
		qdf_nbuf_t next = qdf_nbuf_next(msdu_list);
		QDF_NBUF_UPDATE_TX_PKT_COUNT(msdu_list,
					     QDF_NBUF_TX_PKT_TXRX_ENQUEUE);
		DPTRACE(qdf_dp_trace(msdu_list,
				QDF_DP_TRACE_TXRX_QUEUE_PACKET_PTR_RECORD,
				(uint8_t *)(qdf_nbuf_data(msdu_list)),
				sizeof(qdf_nbuf_data(msdu_list))));

		vdev->ll_pause.txq.depth++;
		if (!vdev->ll_pause.txq.head) {
			vdev->ll_pause.txq.head = msdu_list;
			vdev->ll_pause.txq.tail = msdu_list;
		} else {
			qdf_nbuf_set_next(vdev->ll_pause.txq.tail, msdu_list);
		}
		vdev->ll_pause.txq.tail = msdu_list;

		msdu_list = next;
	}
	if (vdev->ll_pause.txq.tail)
		qdf_nbuf_set_next(vdev->ll_pause.txq.tail, NULL);

	if (start_timer) {
		qdf_timer_stop(&vdev->ll_pause.timer);
		qdf_timer_start(&vdev->ll_pause.timer,
					OL_TX_VDEV_PAUSE_QUEUE_SEND_PERIOD_MS);
		vdev->ll_pause.is_q_timer_on = true;
	}
	qdf_spin_unlock_bh(&vdev->ll_pause.mutex);

	return msdu_list;
}

/*
 * Store up the tx frame in the vdev's tx queue if the vdev is paused.
 * If there are too many frames in the tx queue, reject it.
 */
qdf_nbuf_t ol_tx_ll_queue(ol_txrx_vdev_handle vdev, qdf_nbuf_t msdu_list)
{
	uint16_t eth_type;
	uint32_t paused_reason;

	if (msdu_list == NULL)
		return NULL;

	paused_reason = vdev->ll_pause.paused_reason;
	if (paused_reason) {
		if (qdf_unlikely((paused_reason &
				  OL_TXQ_PAUSE_REASON_PEER_UNAUTHORIZED) ==
				 paused_reason)) {
			eth_type = (((struct ethernet_hdr_t *)
				     qdf_nbuf_data(msdu_list))->
				    ethertype[0] << 8) |
				   (((struct ethernet_hdr_t *)
				     qdf_nbuf_data(msdu_list))->ethertype[1]);
			if (ETHERTYPE_IS_EAPOL_WAPI(eth_type)) {
				msdu_list = ol_tx_ll_wrapper(vdev, msdu_list);
				return msdu_list;
			}
		}
		msdu_list = ol_tx_vdev_pause_queue_append(vdev, msdu_list, 1);
	} else {
		if (vdev->ll_pause.txq.depth > 0 ||
		    vdev->pdev->tx_throttle.current_throttle_level !=
		    THROTTLE_LEVEL_0) {
			/* not paused, but there is a backlog of frms
			   from a prior pause or throttle off phase */
			msdu_list = ol_tx_vdev_pause_queue_append(
				vdev, msdu_list, 0);
			/* if throttle is disabled or phase is "on",
			   send the frame */
			if (vdev->pdev->tx_throttle.current_throttle_level ==
			    THROTTLE_LEVEL_0 ||
			    vdev->pdev->tx_throttle.current_throttle_phase ==
			    THROTTLE_PHASE_ON) {
				/* send as many frames as possible
				   from the vdevs backlog */
				ol_tx_vdev_ll_pause_queue_send_base(vdev);
			}
		} else {
			/* not paused, no throttle and no backlog -
			   send the new frames */
			msdu_list = ol_tx_ll_wrapper(vdev, msdu_list);
		}
	}
	return msdu_list;
}

/*
 * Run through the transmit queues for all the vdevs and
 * send the pending frames
 */
void ol_tx_pdev_ll_pause_queue_send_all(struct ol_txrx_pdev_t *pdev)
{
	int max_to_send;        /* tracks how many frames have been sent */
	qdf_nbuf_t tx_msdu;
	struct ol_txrx_vdev_t *vdev = NULL;
	uint8_t more;

	if (NULL == pdev)
		return;

	if (pdev->tx_throttle.current_throttle_phase == THROTTLE_PHASE_OFF)
		return;

	/* ensure that we send no more than tx_threshold frames at once */
	max_to_send = pdev->tx_throttle.tx_threshold;

	/* round robin through the vdev queues for the given pdev */

	/* Potential improvement: download several frames from the same vdev
	   at a time, since it is more likely that those frames could be
	   aggregated together, remember which vdev was serviced last,
	   so the next call this function can resume the round-robin
	   traversing where the current invocation left off */
	do {
		more = 0;
		TAILQ_FOREACH(vdev, &pdev->vdev_list, vdev_list_elem) {

			qdf_spin_lock_bh(&vdev->ll_pause.mutex);
			if (vdev->ll_pause.txq.depth) {
				if (vdev->ll_pause.paused_reason) {
					qdf_spin_unlock_bh(&vdev->ll_pause.
							   mutex);
					continue;
				}

				tx_msdu = vdev->ll_pause.txq.head;
				if (NULL == tx_msdu) {
					qdf_spin_unlock_bh(&vdev->ll_pause.
							   mutex);
					continue;
				}

				max_to_send--;
				vdev->ll_pause.txq.depth--;

				vdev->ll_pause.txq.head =
					qdf_nbuf_next(tx_msdu);

				if (NULL == vdev->ll_pause.txq.head)
					vdev->ll_pause.txq.tail = NULL;

				qdf_nbuf_set_next(tx_msdu, NULL);
				tx_msdu = ol_tx_ll_wrapper(vdev, tx_msdu);
				/*
				 * It is unexpected that ol_tx_ll would reject
				 * the frame, since we checked that there's
				 * room for it, though there's an infinitesimal
				 * possibility that between the time we checked
				 * the room available and now, a concurrent
				 * batch of tx frames used up all the room.
				 * For simplicity, just drop the frame.
				 */
				if (tx_msdu) {
					qdf_nbuf_unmap(pdev->osdev, tx_msdu,
						       QDF_DMA_TO_DEVICE);
					qdf_nbuf_tx_free(tx_msdu,
							 QDF_NBUF_PKT_ERROR);
				}
			}
			/*check if there are more msdus to transmit */
			if (vdev->ll_pause.txq.depth)
				more = 1;
			qdf_spin_unlock_bh(&vdev->ll_pause.mutex);
		}
	} while (more && max_to_send);

	vdev = NULL;
	TAILQ_FOREACH(vdev, &pdev->vdev_list, vdev_list_elem) {
		qdf_spin_lock_bh(&vdev->ll_pause.mutex);
		if (vdev->ll_pause.txq.depth) {
			qdf_timer_stop(&pdev->tx_throttle.tx_timer);
			qdf_timer_start(
				&pdev->tx_throttle.tx_timer,
				OL_TX_VDEV_PAUSE_QUEUE_SEND_PERIOD_MS);
			qdf_spin_unlock_bh(&vdev->ll_pause.mutex);
			return;
		}
		qdf_spin_unlock_bh(&vdev->ll_pause.mutex);
	}
}

void ol_tx_vdev_ll_pause_queue_send(void *context)
{
	struct ol_txrx_vdev_t *vdev = (struct ol_txrx_vdev_t *)context;
	struct ol_txrx_pdev_t *pdev = vdev->pdev;

	if (pdev->tx_throttle.current_throttle_level != THROTTLE_LEVEL_0 &&
	    pdev->tx_throttle.current_throttle_phase == THROTTLE_PHASE_OFF)
		return;
	ol_tx_vdev_ll_pause_queue_send_base(vdev);
}
#endif /* QCA_LL_LEGACY_TX_FLOW_CONTROL */

static inline int ol_txrx_tx_is_raw(enum ol_tx_spec tx_spec)
{
	return
		tx_spec &
		(OL_TX_SPEC_RAW | OL_TX_SPEC_NO_AGGR | OL_TX_SPEC_NO_ENCRYPT);
}

static inline uint8_t ol_txrx_tx_raw_subtype(enum ol_tx_spec tx_spec)
{
	uint8_t sub_type = 0x1; /* 802.11 MAC header present */

	if (tx_spec & OL_TX_SPEC_NO_AGGR)
		sub_type |= 0x1 << HTT_TX_MSDU_DESC_RAW_SUBTYPE_NO_AGGR_S;
	if (tx_spec & OL_TX_SPEC_NO_ENCRYPT)
		sub_type |= 0x1 << HTT_TX_MSDU_DESC_RAW_SUBTYPE_NO_ENCRYPT_S;
	if (tx_spec & OL_TX_SPEC_NWIFI_NO_ENCRYPT)
		sub_type |= 0x1 << HTT_TX_MSDU_DESC_RAW_SUBTYPE_NO_ENCRYPT_S;
	return sub_type;
}

qdf_nbuf_t
ol_tx_non_std_ll(ol_txrx_vdev_handle vdev,
		 enum ol_tx_spec tx_spec, qdf_nbuf_t msdu_list)
{
	qdf_nbuf_t msdu = msdu_list;
	htt_pdev_handle htt_pdev = vdev->pdev->htt_pdev;
	struct ol_txrx_msdu_info_t msdu_info;

	msdu_info.htt.info.l2_hdr_type = vdev->pdev->htt_pkt_type;
	msdu_info.htt.action.tx_comp_req = 0;

	/*
	 * The msdu_list variable could be used instead of the msdu var,
	 * but just to clarify which operations are done on a single MSDU
	 * vs. a list of MSDUs, use a distinct variable for single MSDUs
	 * within the list.
	 */
	while (msdu) {
		qdf_nbuf_t next;
		struct ol_tx_desc_t *tx_desc;

		msdu_info.htt.info.ext_tid = qdf_nbuf_get_tid(msdu);
		msdu_info.peer = NULL;
		msdu_info.tso_info.is_tso = 0;

		ol_tx_prepare_ll(tx_desc, vdev, msdu, &msdu_info);

		/*
		 * The netbuf may get linked into a different list inside the
		 * ol_tx_send function, so store the next pointer before the
		 * tx_send call.
		 */
		next = qdf_nbuf_next(msdu);

		if (tx_spec != OL_TX_SPEC_STD) {
			if (tx_spec & OL_TX_SPEC_NO_FREE) {
				tx_desc->pkt_type = OL_TX_FRM_NO_FREE;
			} else if (tx_spec & OL_TX_SPEC_TSO) {
				tx_desc->pkt_type = OL_TX_FRM_TSO;
			} else if (tx_spec & OL_TX_SPEC_NWIFI_NO_ENCRYPT) {
				uint8_t sub_type =
					ol_txrx_tx_raw_subtype(tx_spec);
				htt_tx_desc_type(htt_pdev, tx_desc->htt_tx_desc,
						 htt_pkt_type_native_wifi,
						 sub_type);
			} else if (ol_txrx_tx_is_raw(tx_spec)) {
				/* different types of raw frames */
				uint8_t sub_type =
					ol_txrx_tx_raw_subtype(tx_spec);
				htt_tx_desc_type(htt_pdev, tx_desc->htt_tx_desc,
						 htt_pkt_type_raw, sub_type);
			}
		}
		/*
		 * If debug display is enabled, show the meta-data being
		 * downloaded to the target via the HTT tx descriptor.
		 */
		htt_tx_desc_display(tx_desc->htt_tx_desc);
		ol_tx_send(vdev->pdev, tx_desc, msdu);
		msdu = next;
	}
	return NULL;            /* all MSDUs were accepted */
}

#ifdef QCA_SUPPORT_SW_TXRX_ENCAP
#define OL_TX_ENCAP_WRAPPER(pdev, vdev, tx_desc, msdu, tx_msdu_info) \
	do { \
		if (OL_TX_ENCAP(vdev, tx_desc, msdu, &tx_msdu_info) != A_OK) { \
			qdf_atomic_inc(&pdev->tx_queue.rsrc_cnt); \
			ol_tx_desc_frame_free_nonstd(pdev, tx_desc, 1);	\
			if (tx_msdu_info.peer) { \
				/* remove the peer reference added above */ \
				ol_txrx_peer_unref_delete(tx_msdu_info.peer); \
			} \
			goto MSDU_LOOP_BOTTOM; \
		} \
	} while (0)
#else
#define OL_TX_ENCAP_WRAPPER(pdev, vdev, tx_desc, msdu, tx_msdu_info) /* no-op */
#endif

/* tx filtering is handled within the target FW */
#define TX_FILTER_CHECK(tx_msdu_info) 0 /* don't filter */

/**
 * parse_ocb_tx_header() - Function to check for OCB
 * TX control header on a packet and extract it if present
 *
 * @msdu:   Pointer to OS packet (qdf_nbuf_t)
 */
#define OCB_HEADER_VERSION     1
bool parse_ocb_tx_header(qdf_nbuf_t msdu,
			struct ocb_tx_ctrl_hdr_t *tx_ctrl)
{
	struct ether_header *eth_hdr_p;
	struct ocb_tx_ctrl_hdr_t *tx_ctrl_hdr;

	/* Check if TX control header is present */
	eth_hdr_p = (struct ether_header *) qdf_nbuf_data(msdu);
	if (eth_hdr_p->ether_type != QDF_SWAP_U16(ETHERTYPE_OCB_TX))
		/* TX control header is not present. Nothing to do.. */
		return true;

	/* Remove the ethernet header */
	qdf_nbuf_pull_head(msdu, sizeof(struct ether_header));

	/* Parse the TX control header */
	tx_ctrl_hdr = (struct ocb_tx_ctrl_hdr_t *) qdf_nbuf_data(msdu);

	if (tx_ctrl_hdr->version == OCB_HEADER_VERSION) {
		if (tx_ctrl)
			qdf_mem_copy(tx_ctrl, tx_ctrl_hdr,
				sizeof(*tx_ctrl_hdr));
	} else {
		/* The TX control header is invalid. */
		return false;
	}

	/* Remove the TX control header */
	qdf_nbuf_pull_head(msdu, tx_ctrl_hdr->length);
	return true;
}

/**
 * ol_tx_non_std - Allow the control-path SW to send data frames
 *
 * @data_vdev - which vdev should transmit the tx data frames
 * @tx_spec - what non-standard handling to apply to the tx data frames
 * @msdu_list - NULL-terminated list of tx MSDUs
 *
 * Generally, all tx data frames come from the OS shim into the txrx layer.
 * However, there are rare cases such as TDLS messaging where the UMAC
 * control-path SW creates tx data frames.
 *  This UMAC SW can call this function to provide the tx data frames to
 *  the txrx layer.
 *  The UMAC SW can request a callback for these data frames after their
 *  transmission completes, by using the ol_txrx_data_tx_cb_set function
 *  to register a tx completion callback, and by specifying
 *  ol_tx_spec_no_free as the tx_spec arg when giving the frames to
 *  ol_tx_non_std.
 *  The MSDUs need to have the appropriate L2 header type (802.3 vs. 802.11),
 *  as specified by ol_cfg_frame_type().
 *
 *  Return: null - success, skb - failure
 */
qdf_nbuf_t
ol_tx_non_std(ol_txrx_vdev_handle vdev,
	      enum ol_tx_spec tx_spec, qdf_nbuf_t msdu_list)
{
	return ol_tx_non_std_ll(vdev, tx_spec, msdu_list);
}

void
ol_txrx_data_tx_cb_set(ol_txrx_vdev_handle vdev,
		       ol_txrx_data_tx_cb callback, void *ctxt)
{
	struct ol_txrx_pdev_t *pdev = vdev->pdev;
	pdev->tx_data_callback.func = callback;
	pdev->tx_data_callback.ctxt = ctxt;
}

/**
 * ol_txrx_mgmt_tx_cb_set() - Store a callback for delivery
 * notifications for management frames.
 *
 * @pdev - the data physical device object
 * @type - the type of mgmt frame the callback is used for
 * @download_cb - the callback for notification of delivery to the target
 * @ota_ack_cb - the callback for notification of delivery to the peer
 * @ctxt - context to use with the callback
 *
 * When the txrx SW receives notifications from the target that a tx frame
 * has been delivered to its recipient, it will check if the tx frame
 * is a management frame.  If so, the txrx SW will check the management
 * frame type specified when the frame was submitted for transmission.
 * If there is a callback function registered for the type of managment
 * frame in question, the txrx code will invoke the callback to inform
 * the management + control SW that the mgmt frame was delivered.
 * This function is used by the control SW to store a callback pointer
 * for a given type of management frame.
 */
void
ol_txrx_mgmt_tx_cb_set(ol_txrx_pdev_handle pdev,
		       uint8_t type,
		       ol_txrx_mgmt_tx_cb download_cb,
		       ol_txrx_mgmt_tx_cb ota_ack_cb, void *ctxt)
{
	TXRX_ASSERT1(type < OL_TXRX_MGMT_NUM_TYPES);
	pdev->tx_mgmt.callbacks[type].download_cb = download_cb;
	pdev->tx_mgmt.callbacks[type].ota_ack_cb = ota_ack_cb;
	pdev->tx_mgmt.callbacks[type].ctxt = ctxt;
}

#if defined(HELIUMPLUS_PADDR64)
void dump_frag_desc(char *msg, struct ol_tx_desc_t *tx_desc)
{
	uint32_t                *frag_ptr_i_p;
	int                     i;

	qdf_print("OL TX Descriptor 0x%p msdu_id %d\n",
		 tx_desc, tx_desc->id);
	qdf_print("HTT TX Descriptor vaddr: 0x%p paddr: 0x%llx",
		 tx_desc->htt_tx_desc, tx_desc->htt_tx_desc_paddr);
	qdf_print("%s %d: Fragment Descriptor 0x%p (paddr=0x%llx)",
		 __func__, __LINE__, tx_desc->htt_frag_desc, tx_desc->htt_frag_desc_paddr);

	/* it looks from htt_tx_desc_frag() that tx_desc->htt_frag_desc
	   is already de-referrable (=> in virtual address space) */
	frag_ptr_i_p = tx_desc->htt_frag_desc;

	/* Dump 6 words of TSO flags */
	print_hex_dump(KERN_DEBUG, "MLE Desc:TSO Flags:  ",
		       DUMP_PREFIX_NONE, 8, 4,
		       frag_ptr_i_p, 24, true);

	frag_ptr_i_p += 6; /* Skip 6 words of TSO flags */

	i = 0;
	while (*frag_ptr_i_p) {
		print_hex_dump(KERN_DEBUG, "MLE Desc:Frag Ptr:  ",
			       DUMP_PREFIX_NONE, 8, 4,
			       frag_ptr_i_p, 8, true);
		i++;
		if (i > 5) /* max 6 times: frag_ptr0 to frag_ptr5 */
			break;
		else  /* jump to next  pointer - skip length */
			frag_ptr_i_p += 2;
	}
	return;
}
#endif /* HELIUMPLUS_PADDR64 */

/**
 * ol_txrx_mgmt_send_ext() - Transmit a management frame
 *
 * @vdev - virtual device transmitting the frame
 * @tx_mgmt_frm - management frame to transmit
 * @type - the type of managment frame (determines what callback to use)
 * @use_6mbps - specify whether management frame to transmit should
 * use 6 Mbps rather than 1 Mbps min rate(for 5GHz band or P2P)
 * @chanfreq - channel to transmit the frame on
 *
 * Send the specified management frame from the specified virtual device.
 * The type is used for determining whether to invoke a callback to inform
 * the sender that the tx mgmt frame was delivered, and if so, which
 * callback to use.
 *
 * Return: 0 - the frame is accepted for transmission
 *         1 - the frame was not accepted
 */
int
ol_txrx_mgmt_send_ext(ol_txrx_vdev_handle vdev,
		  qdf_nbuf_t tx_mgmt_frm,
		  uint8_t type, uint8_t use_6mbps, uint16_t chanfreq)
{
	struct ol_txrx_pdev_t *pdev = vdev->pdev;
	struct ol_tx_desc_t *tx_desc;
	struct ol_txrx_msdu_info_t tx_msdu_info;

	tx_msdu_info.tso_info.is_tso = 0;

	tx_msdu_info.htt.action.use_6mbps = use_6mbps;
	tx_msdu_info.htt.info.ext_tid = HTT_TX_EXT_TID_MGMT;
	tx_msdu_info.htt.info.vdev_id = vdev->vdev_id;
	tx_msdu_info.htt.action.do_tx_complete =
		pdev->tx_mgmt.callbacks[type].ota_ack_cb ? 1 : 0;

	/*
	 * FIX THIS: l2_hdr_type should only specify L2 header type
	 * The Peregrine/Rome HTT layer provides the FW with a "pkt type"
	 * that is a combination of L2 header type and 802.11 frame type.
	 * If the 802.11 frame type is "mgmt", then the HTT pkt type is "mgmt".
	 * But if the 802.11 frame type is "data", then the HTT pkt type is
	 * the L2 header type (more or less): 802.3 vs. Native WiFi
	 * (basic 802.11).
	 * (Or the header type can be "raw", which is any version of the 802.11
	 * header, and also implies that some of the offloaded tx data
	 * processing steps may not apply.)
	 * For efficiency, the Peregrine/Rome HTT uses the msdu_info's
	 * l2_hdr_type field to program the HTT pkt type.  Thus, this txrx SW
	 * needs to overload the l2_hdr_type to indicate whether the frame is
	 * data vs. mgmt, as well as 802.3 L2 header vs. 802.11 L2 header.
	 * To fix this, the msdu_info's l2_hdr_type should be left specifying
	 * just the L2 header type.  For mgmt frames, there should be a
	 * separate function to patch the HTT pkt type to store a "mgmt" value
	 * rather than the L2 header type.  Then the HTT pkt type can be
	 * programmed efficiently for data frames, and the msdu_info's
	 * l2_hdr_type field won't be confusingly overloaded to hold the 802.11
	 * frame type rather than the L2 header type.
	 */
	/*
	 * FIX THIS: remove duplication of htt_frm_type_mgmt and
	 * htt_pkt_type_mgmt
	 * The htt module expects a "enum htt_pkt_type" value.
	 * The htt_dxe module expects a "enum htt_frm_type" value.
	 * This needs to be cleaned up, so both versions of htt use a
	 * consistent method of specifying the frame type.
	 */
#ifdef QCA_SUPPORT_INTEGRATED_SOC
	/* tx mgmt frames always come with a 802.11 header */
	tx_msdu_info.htt.info.l2_hdr_type = htt_pkt_type_native_wifi;
	tx_msdu_info.htt.info.frame_type = htt_frm_type_mgmt;
#else
	tx_msdu_info.htt.info.l2_hdr_type = htt_pkt_type_mgmt;
	tx_msdu_info.htt.info.frame_type = htt_pkt_type_mgmt;
#endif

	tx_msdu_info.peer = NULL;

	qdf_nbuf_map_single(pdev->osdev, tx_mgmt_frm, QDF_DMA_TO_DEVICE);
	/* For LL tx_comp_req is not used so initialized to 0 */
	tx_msdu_info.htt.action.tx_comp_req = 0;
	tx_desc = ol_tx_desc_ll(pdev, vdev, tx_mgmt_frm, &tx_msdu_info);
	/* FIX THIS -
	 * The FW currently has trouble using the host's fragments table
	 * for management frames.  Until this is fixed, rather than
	 * specifying the fragment table to the FW, specify just the
	 * address of the initial fragment.
	 */
#if defined(HELIUMPLUS_PADDR64)
	/* dump_frag_desc("ol_txrx_mgmt_send(): after ol_tx_desc_ll",
	   tx_desc); */
#endif /* defined(HELIUMPLUS_PADDR64) */
	if (tx_desc) {
		/*
		 * Following the call to ol_tx_desc_ll, frag 0 is the
		 * HTT tx HW descriptor, and the frame payload is in
		 * frag 1.
		 */
		htt_tx_desc_frags_table_set(
			pdev->htt_pdev,
			tx_desc->htt_tx_desc,
			qdf_nbuf_get_frag_paddr(tx_mgmt_frm, 1),
			0, 0);
#if defined(HELIUMPLUS_PADDR64) && defined(HELIUMPLUS_DEBUG)
		dump_frag_desc(
			"after htt_tx_desc_frags_table_set",
			tx_desc);
#endif /* defined(HELIUMPLUS_PADDR64) */
	}
	if (!tx_desc) {
		qdf_nbuf_unmap_single(pdev->osdev, tx_mgmt_frm,
				      QDF_DMA_TO_DEVICE);
		return -EINVAL;       /* can't accept the tx mgmt frame */
	}
	TXRX_STATS_MSDU_INCR(pdev, tx.mgmt, tx_mgmt_frm);
	TXRX_ASSERT1(type < OL_TXRX_MGMT_NUM_TYPES);
	tx_desc->pkt_type = type + OL_TXRX_MGMT_TYPE_BASE;

	htt_tx_desc_set_chanfreq(tx_desc->htt_tx_desc, chanfreq);
	QDF_NBUF_CB_TX_PACKET_TRACK(tx_desc->netbuf) =
				QDF_NBUF_TX_PKT_MGMT_TRACK;
	ol_tx_send_nonstd(pdev, tx_desc, tx_mgmt_frm,
			  htt_pkt_type_mgmt);

	return 0;               /* accepted the tx mgmt frame */
}

void ol_txrx_sync(ol_txrx_pdev_handle pdev, uint8_t sync_cnt)
{
	htt_h2t_sync_msg(pdev->htt_pdev, sync_cnt);
}

qdf_nbuf_t ol_tx_reinject(struct ol_txrx_vdev_t *vdev,
			  qdf_nbuf_t msdu, uint16_t peer_id)
{
	struct ol_tx_desc_t *tx_desc;
	struct ol_txrx_msdu_info_t msdu_info;

	msdu_info.htt.info.l2_hdr_type = vdev->pdev->htt_pkt_type;
	msdu_info.htt.info.ext_tid = HTT_TX_EXT_TID_INVALID;
	msdu_info.peer = NULL;
	msdu_info.htt.action.tx_comp_req = 0;
	msdu_info.tso_info.is_tso = 0;

	ol_tx_prepare_ll(tx_desc, vdev, msdu, &msdu_info);
	HTT_TX_DESC_POSTPONED_SET(*((uint32_t *) (tx_desc->htt_tx_desc)), true);

	htt_tx_desc_set_peer_id(tx_desc->htt_tx_desc, peer_id);

	ol_tx_send(vdev->pdev, tx_desc, msdu);

	return NULL;
}

#if defined(FEATURE_TSO)
void ol_tso_seg_list_init(struct ol_txrx_pdev_t *pdev, uint32_t num_seg)
{
	int i;
	struct qdf_tso_seg_elem_t *c_element;

	c_element = qdf_mem_malloc(sizeof(struct qdf_tso_seg_elem_t));
	pdev->tso_seg_pool.freelist = c_element;
	for (i = 0; i < (num_seg - 1); i++) {
		c_element->next =
			qdf_mem_malloc(sizeof(struct qdf_tso_seg_elem_t));
		c_element = c_element->next;
		c_element->next = NULL;
	}
	pdev->tso_seg_pool.pool_size = num_seg;
	qdf_spinlock_create(&pdev->tso_seg_pool.tso_mutex);
}

void ol_tso_seg_list_deinit(struct ol_txrx_pdev_t *pdev)
{
	int i;
	struct qdf_tso_seg_elem_t *c_element;
	struct qdf_tso_seg_elem_t *temp;

	qdf_spin_lock_bh(&pdev->tso_seg_pool.tso_mutex);
	c_element = pdev->tso_seg_pool.freelist;
	for (i = 0; i < pdev->tso_seg_pool.pool_size; i++) {
		temp = c_element->next;
		qdf_mem_free(c_element);
		c_element = temp;
		if (!c_element)
			break;
	}

	pdev->tso_seg_pool.freelist = NULL;
	pdev->tso_seg_pool.num_free = 0;
	pdev->tso_seg_pool.pool_size = 0;
	qdf_spin_unlock_bh(&pdev->tso_seg_pool.tso_mutex);
	qdf_spinlock_destroy(&pdev->tso_seg_pool.tso_mutex);
}
#endif /* FEATURE_TSO */
