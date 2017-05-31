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

#include "htt.h"
#include "dp_tx.h"
#include "dp_tx_desc.h"
#include "dp_peer.h"
#include "dp_types.h"
#include "hal_tx.h"
#include "qdf_mem.h"
#include "qdf_nbuf.h"
#include <wlan_cfg.h>
#ifdef MESH_MODE_SUPPORT
#include "if_meta_hdr.h"
#endif

#ifdef TX_PER_PDEV_DESC_POOL
	#define DP_TX_GET_DESC_POOL_ID(vdev) (vdev->pdev->pdev_id)
	#define DP_TX_GET_RING_ID(vdev) (vdev->pdev->pdev_id)
#else
	#ifdef TX_PER_VDEV_DESC_POOL
		#define DP_TX_GET_DESC_POOL_ID(vdev) (vdev->vdev_id)
		#define DP_TX_GET_RING_ID(vdev) (vdev->pdev->pdev_id)
	#else
		#define DP_TX_GET_DESC_POOL_ID(vdev) qdf_get_cpu()
		#define DP_TX_GET_RING_ID(vdev) qdf_get_cpu()
	#endif /* TX_PER_VDEV_DESC_POOL */
#endif /* TX_PER_PDEV_DESC_POOL */

/* TODO Add support in TSO */
#define DP_DESC_NUM_FRAG(x) 0

/* disable TQM_BYPASS */
#define TQM_BYPASS_WAR 0

/**
 * dp_tx_get_queue() - Returns Tx queue IDs to be used for this Tx frame
 * @vdev: DP Virtual device handle
 * @nbuf: Buffer pointer
 * @queue: queue ids container for nbuf
 *
 * TX packet queue has 2 instances, software descriptors id and dma ring id
 * Based on tx feature and hardware configuration queue id combination could be
 * different.
 * For example -
 * With XPS enabled,all TX descriptor pools and dma ring are assigned per cpu id
 * With no XPS,lock based resource protection, Descriptor pool ids are different
 * for each vdev, dma ring id will be same as single pdev id
 *
 * Return: None
 */
static inline void dp_tx_get_queue(struct dp_vdev *vdev,
		qdf_nbuf_t nbuf, struct dp_tx_queue *queue)
{
	queue->desc_pool_id = DP_TX_GET_DESC_POOL_ID(vdev);
	queue->ring_id = DP_TX_GET_RING_ID(vdev);

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			"%s, pool_id:%d ring_id: %d\n",
			__func__, queue->desc_pool_id, queue->ring_id);

	return;
}

#if defined(FEATURE_TSO)
/**
 * dp_tx_tso_desc_release() - Release the tso segment
 *                            after unmapping all the fragments
 *
 * @pdev - physical device handle
 * @tx_desc - Tx software descriptor
 */
static void dp_tx_tso_desc_release(struct dp_soc *soc,
		struct dp_tx_desc_s *tx_desc)
{
	TSO_DEBUG("%s: Free the tso descriptor", __func__);
	if (qdf_unlikely(tx_desc->tso_desc == NULL)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"%s %d TSO desc is NULL!",
			__func__, __LINE__);
		qdf_assert(0);
	} else if (qdf_unlikely(tx_desc->tso_num_desc == NULL)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s %d TSO common info is NULL!",
			__func__, __LINE__);
		qdf_assert(0);
	} else {
		struct qdf_tso_num_seg_elem_t *tso_num_desc =
			(struct qdf_tso_num_seg_elem_t *) tx_desc->tso_num_desc;

		if (tso_num_desc->num_seg.tso_cmn_num_seg > 1) {
			tso_num_desc->num_seg.tso_cmn_num_seg--;
			qdf_nbuf_unmap_tso_segment(soc->osdev,
					tx_desc->tso_desc, false);
		} else {
			tso_num_desc->num_seg.tso_cmn_num_seg--;
			qdf_assert(tso_num_desc->num_seg.tso_cmn_num_seg == 0);
			qdf_nbuf_unmap_tso_segment(soc->osdev,
					tx_desc->tso_desc, true);
			dp_tso_num_seg_free(soc, tx_desc->pool_id,
					tx_desc->tso_num_desc);
			tx_desc->tso_num_desc = NULL;
		}
		dp_tx_tso_desc_free(soc,
				tx_desc->pool_id, tx_desc->tso_desc);
		tx_desc->tso_desc = NULL;
	}
}
#else
static void dp_tx_tso_desc_release(struct dp_soc *soc,
		struct dp_tx_desc_s *tx_desc)
{
	return;
}
#endif
/**
 * dp_tx_desc_release() - Release Tx Descriptor
 * @tx_desc : Tx Descriptor
 * @desc_pool_id: Descriptor Pool ID
 *
 * Deallocate all resources attached to Tx descriptor and free the Tx
 * descriptor.
 *
 * Return:
 */
static void
dp_tx_desc_release(struct dp_tx_desc_s *tx_desc, uint8_t desc_pool_id)
{
	struct dp_pdev *pdev = tx_desc->pdev;
	struct dp_soc *soc;
	uint8_t comp_status = 0;

	qdf_assert(pdev);

	soc = pdev->soc;

	if (tx_desc->frm_type == dp_tx_frm_tso)
		dp_tx_tso_desc_release(soc, tx_desc);

	if (tx_desc->flags & DP_TX_DESC_FLAG_FRAG)
		dp_tx_ext_desc_free(soc, tx_desc->msdu_ext_desc, desc_pool_id);

	qdf_atomic_dec(&pdev->num_tx_outstanding);

	if (tx_desc->flags & DP_TX_DESC_FLAG_TO_FW)
		qdf_atomic_dec(&pdev->num_tx_exception);

	if (HAL_TX_COMP_RELEASE_SOURCE_TQM ==
				hal_tx_comp_get_buffer_source(&tx_desc->comp))
		comp_status = hal_tx_comp_get_release_reason(&tx_desc->comp);
	else
		comp_status = HAL_TX_COMP_RELEASE_REASON_FW;

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			"Tx Completion Release desc %d status %d outstanding %d\n",
			tx_desc->id, comp_status,
			qdf_atomic_read(&pdev->num_tx_outstanding));

	dp_tx_desc_free(soc, tx_desc, desc_pool_id);
	return;
}

/**
 * dp_tx_htt_metadata_prepare() - Prepare HTT metadata for special frames
 * @vdev: DP vdev Handle
 * @nbuf: skb
 *
 * Prepares and fills HTT metadata in the frame pre-header for special frames
 * that should be transmitted using varying transmit parameters.
 * There are 2 VDEV modes that currently needs this special metadata -
 *  1) Mesh Mode
 *  2) DSRC Mode
 *
 * Return: HTT metadata size
 *
 */
static uint8_t dp_tx_prepare_htt_metadata(struct dp_vdev *vdev, qdf_nbuf_t nbuf,
		uint32_t *meta_data)
{
	struct htt_tx_msdu_desc_ext2_t *desc_ext =
				(struct htt_tx_msdu_desc_ext2_t *) meta_data;

	uint8_t htt_desc_size;

	/* Size rounded of multiple of 8 bytes */
	uint8_t htt_desc_size_aligned;

	uint8_t *hdr = NULL;

	qdf_nbuf_unshare(nbuf);

	HTT_TX_TCL_METADATA_VALID_HTT_SET(vdev->htt_tcl_metadata, 1);

	/*
	 * Metadata - HTT MSDU Extension header
	 */
	htt_desc_size = sizeof(struct htt_tx_msdu_desc_ext2_t);
	htt_desc_size_aligned = (htt_desc_size + 7) & ~0x7;

	if (vdev->mesh_vdev) {

		/* Fill and add HTT metaheader */
		hdr = qdf_nbuf_push_head(nbuf, htt_desc_size_aligned);
		qdf_mem_copy(hdr, desc_ext, htt_desc_size);

	} else if (vdev->opmode == wlan_op_mode_ocb) {
		/* Todo - Add support for DSRC */
	}

	return htt_desc_size_aligned;
}

/**
 * dp_tx_prepare_tso_ext_desc() - Prepare MSDU extension descriptor for TSO
 * @tso_seg: TSO segment to process
 * @ext_desc: Pointer to MSDU extension descriptor
 *
 * Return: void
 */
#if defined(FEATURE_TSO)
static void dp_tx_prepare_tso_ext_desc(struct qdf_tso_seg_t *tso_seg,
		void *ext_desc)
{
	uint8_t num_frag;
	uint32_t tso_flags;

	/*
	 * Set tso_en, tcp_flags(NS, CWR, ECE, URG, ACK, PSH, RST, SYN, FIN),
	 * tcp_flag_mask
	 *
	 * Checksum enable flags are set in TCL descriptor and not in Extension
	 * Descriptor (H/W ignores checksum_en flags in MSDU ext descriptor)
	 */
	tso_flags = *(uint32_t *) &tso_seg->tso_flags;

	hal_tx_ext_desc_set_tso_flags(ext_desc, tso_flags);

	hal_tx_ext_desc_set_msdu_length(ext_desc, tso_seg->tso_flags.l2_len,
		tso_seg->tso_flags.ip_len);

	hal_tx_ext_desc_set_tcp_seq(ext_desc, tso_seg->tso_flags.tcp_seq_num);
	hal_tx_ext_desc_set_ip_id(ext_desc, tso_seg->tso_flags.ip_id);


	for (num_frag = 0; num_frag < tso_seg->num_frags; num_frag++) {
		uint32_t lo = 0;
		uint32_t hi = 0;

		qdf_dmaaddr_to_32s(
			tso_seg->tso_frags[num_frag].paddr, &lo, &hi);
		hal_tx_ext_desc_set_buffer(ext_desc, num_frag, lo, hi,
			tso_seg->tso_frags[num_frag].length);
	}

	return;
}
#else
static void dp_tx_prepare_tso_ext_desc(struct qdf_tso_seg_t *tso_seg,
		void *ext_desc)
{
	return;
}
#endif

#if defined(FEATURE_TSO)
/**
 * dp_tx_free_tso_seg() - Loop through the tso segments
 *                        allocated and free them
 *
 * @soc: soc handle
 * @free_seg: list of tso segments
 * @msdu_info: msdu descriptor
 *
 * Return - void
 */
static void dp_tx_free_tso_seg(struct dp_soc *soc,
	struct qdf_tso_seg_elem_t *free_seg,
	struct dp_tx_msdu_info_s *msdu_info)
{
	struct qdf_tso_seg_elem_t *next_seg;

	while (free_seg) {
		next_seg = free_seg->next;
		dp_tx_tso_desc_free(soc,
			msdu_info->tx_queue.desc_pool_id,
			free_seg);
		free_seg = next_seg;
	}
}

/**
 * dp_tx_free_tso_num_seg() - Loop through the tso num segments
 *                            allocated and free them
 *
 * @soc:  soc handle
 * @free_seg: list of tso segments
 * @msdu_info: msdu descriptor
 * Return - void
 */
static void dp_tx_free_tso_num_seg(struct dp_soc *soc,
	struct qdf_tso_num_seg_elem_t *free_seg,
	struct dp_tx_msdu_info_s *msdu_info)
{
	struct qdf_tso_num_seg_elem_t *next_seg;

	while (free_seg) {
		next_seg = free_seg->next;
		dp_tso_num_seg_free(soc,
			msdu_info->tx_queue.desc_pool_id,
			free_seg);
		free_seg = next_seg;
	}
}

/**
 * dp_tx_prepare_tso() - Given a jumbo msdu, prepare the TSO info
 * @vdev: virtual device handle
 * @msdu: network buffer
 * @msdu_info: meta data associated with the msdu
 *
 * Return: QDF_STATUS_SUCCESS success
 */
static QDF_STATUS dp_tx_prepare_tso(struct dp_vdev *vdev,
		qdf_nbuf_t msdu, struct dp_tx_msdu_info_s *msdu_info)
{
	struct qdf_tso_seg_elem_t *tso_seg;
	int num_seg = qdf_nbuf_get_tso_num_seg(msdu);
	struct dp_soc *soc = vdev->pdev->soc;
	struct qdf_tso_info_t *tso_info;
	struct qdf_tso_num_seg_elem_t *tso_num_seg;

	tso_info = &msdu_info->u.tso_info;
	tso_info->curr_seg = NULL;
	tso_info->tso_seg_list = NULL;
	tso_info->num_segs = num_seg;
	msdu_info->frm_type = dp_tx_frm_tso;
	tso_info->tso_num_seg_list = NULL;

	TSO_DEBUG(" %s: num_seg: %d", __func__, num_seg);

	while (num_seg) {
		tso_seg = dp_tx_tso_desc_alloc(
				soc, msdu_info->tx_queue.desc_pool_id);
		if (tso_seg) {
			tso_seg->next = tso_info->tso_seg_list;
			tso_info->tso_seg_list = tso_seg;
			num_seg--;
		} else {
			struct qdf_tso_seg_elem_t *free_seg =
				tso_info->tso_seg_list;

			dp_tx_free_tso_seg(soc, free_seg, msdu_info);

			return QDF_STATUS_E_NOMEM;
		}
	}

	TSO_DEBUG(" %s: num_seg: %d", __func__, num_seg);

	tso_num_seg = dp_tso_num_seg_alloc(soc,
			msdu_info->tx_queue.desc_pool_id);

	if (tso_num_seg) {
		tso_num_seg->next = tso_info->tso_num_seg_list;
		tso_info->tso_num_seg_list = tso_num_seg;
	} else {
		/* Bug: free tso_num_seg and tso_seg */
		/* Free the already allocated num of segments */
		struct qdf_tso_seg_elem_t *free_seg =
					tso_info->tso_seg_list;

		TSO_DEBUG(" %s: Failed alloc - Number of segs for a TSO packet",
			__func__);
		dp_tx_free_tso_seg(soc, free_seg, msdu_info);

		return QDF_STATUS_E_NOMEM;
	}

	msdu_info->num_seg =
		qdf_nbuf_get_tso_info(soc->osdev, msdu, tso_info);

	TSO_DEBUG(" %s: msdu_info->num_seg: %d", __func__,
			msdu_info->num_seg);

	if (!(msdu_info->num_seg)) {
		dp_tx_free_tso_seg(soc, tso_info->tso_seg_list, msdu_info);
		dp_tx_free_tso_num_seg(soc, tso_info->tso_num_seg_list,
					msdu_info);
		return QDF_STATUS_E_INVAL;
	}

	tso_info->curr_seg = tso_info->tso_seg_list;

	return QDF_STATUS_SUCCESS;
}
#else
static QDF_STATUS dp_tx_prepare_tso(struct dp_vdev *vdev,
		qdf_nbuf_t msdu, struct dp_tx_msdu_info_s *msdu_info)
{
	return QDF_STATUS_E_NOMEM;
}
#endif

/**
 * dp_tx_prepare_ext_desc() - Allocate and prepare MSDU extension descriptor
 * @vdev: DP Vdev handle
 * @msdu_info: MSDU info to be setup in MSDU extension descriptor
 * @desc_pool_id: Descriptor Pool ID
 *
 * Return:
 */
static
struct dp_tx_ext_desc_elem_s *dp_tx_prepare_ext_desc(struct dp_vdev *vdev,
		struct dp_tx_msdu_info_s *msdu_info, uint8_t desc_pool_id)
{
	uint8_t i;
	uint8_t cached_ext_desc[HAL_TX_EXT_DESC_WITH_META_DATA];
	struct dp_tx_seg_info_s *seg_info;
	struct dp_tx_ext_desc_elem_s *msdu_ext_desc;
	struct dp_soc *soc = vdev->pdev->soc;

	/* Allocate an extension descriptor */
	msdu_ext_desc = dp_tx_ext_desc_alloc(soc, desc_pool_id);
	qdf_mem_zero(&cached_ext_desc[0], HAL_TX_EXT_DESC_WITH_META_DATA);

	if (!msdu_ext_desc) {
		DP_STATS_INC(vdev, tx_i.dropped.desc_na, 1);
		return NULL;
	}

	if (qdf_unlikely(vdev->mesh_vdev)) {
		qdf_mem_copy(&cached_ext_desc[HAL_TX_EXTENSION_DESC_LEN_BYTES],
				&msdu_info->meta_data[0],
				sizeof(struct htt_tx_msdu_desc_ext2_t));
		qdf_atomic_inc(&vdev->pdev->num_tx_exception);
		HTT_TX_TCL_METADATA_VALID_HTT_SET(vdev->htt_tcl_metadata, 1);
	}

	switch (msdu_info->frm_type) {
	case dp_tx_frm_sg:
	case dp_tx_frm_me:
	case dp_tx_frm_raw:
		seg_info = msdu_info->u.sg_info.curr_seg;
		/* Update the buffer pointers in MSDU Extension Descriptor */
		for (i = 0; i < seg_info->frag_cnt; i++) {
			hal_tx_ext_desc_set_buffer(&cached_ext_desc[0], i,
				seg_info->frags[i].paddr_lo,
				seg_info->frags[i].paddr_hi,
				seg_info->frags[i].len);
		}

		break;

	case dp_tx_frm_tso:
		dp_tx_prepare_tso_ext_desc(&msdu_info->u.tso_info.curr_seg->seg,
				&cached_ext_desc[0]);
		break;


	default:
		break;
	}

	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			cached_ext_desc, HAL_TX_EXT_DESC_WITH_META_DATA);

	hal_tx_ext_desc_sync(&cached_ext_desc[0],
			msdu_ext_desc->vaddr);

	return msdu_ext_desc;
}

/**
 * dp_tx_desc_prepare_single - Allocate and prepare Tx descriptor
 * @vdev: DP vdev handle
 * @nbuf: skb
 * @desc_pool_id: Descriptor pool ID
 * Allocate and prepare Tx descriptor with msdu information.
 *
 * Return: Pointer to Tx Descriptor on success,
 *         NULL on failure
 */
static
struct dp_tx_desc_s *dp_tx_prepare_desc_single(struct dp_vdev *vdev,
		qdf_nbuf_t nbuf, uint8_t desc_pool_id,
		uint32_t *meta_data)
{
	QDF_STATUS status;
	uint8_t align_pad;
	uint8_t is_exception = 0;
	uint8_t htt_hdr_size;
	struct ether_header *eh;
	struct dp_tx_desc_s *tx_desc;
	struct dp_pdev *pdev = vdev->pdev;
	struct dp_soc *soc = pdev->soc;

	/* Flow control/Congestion Control processing */
	status = dp_tx_flow_control(vdev);
	if (QDF_STATUS_E_RESOURCES == status) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
				"%s Tx Resource Full\n", __func__);
		DP_STATS_INC(vdev, tx_i.dropped.res_full, 1);
		/* TODO Stop Tx Queues */
	}

	/* Allocate software Tx descriptor */
	tx_desc = dp_tx_desc_alloc(soc, desc_pool_id);

	if (qdf_unlikely(!tx_desc)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"%s Tx Desc Alloc Failed\n", __func__);
		DP_STATS_INC(vdev, tx_i.dropped.desc_na, 1);
		return NULL;
	}

	/* Flow control/Congestion Control counters */
	qdf_atomic_inc(&pdev->num_tx_outstanding);

	/* Initialize the SW tx descriptor */
	tx_desc->nbuf = nbuf;
	tx_desc->frm_type = dp_tx_frm_std;
	tx_desc->tx_encap_type = vdev->tx_encap_type;
	tx_desc->vdev = vdev;
	tx_desc->pdev = pdev;
	tx_desc->msdu_ext_desc = NULL;

	/**
	 * For non-scatter regular frames, buffer pointer is directly
	 * programmed in TCL input descriptor instead of using an MSDU
	 * extension descriptor.For this cass, HW requirement is that
	 * descriptor should always point to a 8-byte aligned address.
	 *
	 * So we add alignment pad to start of buffer, and specify the actual
	 * start of data through pkt_offset
	 */
	align_pad = ((unsigned long) qdf_nbuf_data(nbuf)) & 0x7;
	qdf_nbuf_push_head(nbuf, align_pad);
	tx_desc->pkt_offset = align_pad;

	/*
	 * For special modes (vdev_type == ocb or mesh), data frames should be
	 * transmitted using varying transmit parameters (tx spec) which include
	 * transmit rate, power, priority, channel, channel bandwidth , nss etc.
	 * These are filled in HTT MSDU descriptor and sent in frame pre-header.
	 * These frames are sent as exception packets to firmware.
	 *
	 *  HTT Metadata should be ensured to be multiple of 8-bytes,
	 *  to get 8-byte aligned start address along with align_pad added above
	 *
	 *  |-----------------------------|
	 *  |                             |
	 *  |-----------------------------| <-----Buffer Pointer Address given
	 *  |                             |  ^    in HW descriptor (aligned)
	 *  |       HTT Metadata          |  |
	 *  |                             |  |
	 *  |                             |  | Packet Offset given in descriptor
	 *  |                             |  |
	 *  |-----------------------------|  |
	 *  |       Alignment Pad         |  v
	 *  |-----------------------------| <----- Actual buffer start address
	 *  |        SKB Data             |           (Unaligned)
	 *  |                             |
	 *  |                             |
	 *  |                             |
	 *  |                             |
	 *  |                             |
	 *  |-----------------------------|
	 */
	if (qdf_unlikely(vdev->mesh_vdev ||
				(vdev->opmode == wlan_op_mode_ocb))) {
		htt_hdr_size = dp_tx_prepare_htt_metadata(vdev, nbuf,
				meta_data);
		tx_desc->pkt_offset += htt_hdr_size;
		tx_desc->flags |= DP_TX_DESC_FLAG_TO_FW;
		is_exception = 1;
	}

	if (qdf_unlikely(QDF_STATUS_SUCCESS !=
				qdf_nbuf_map(soc->osdev, nbuf,
					QDF_DMA_TO_DEVICE))) {
		/* Handle failure */
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"qdf_nbuf_map failed\n");
		DP_STATS_INC(vdev, tx_i.dropped.dma_error, 1);
		goto failure;
	}

	if (qdf_unlikely(vdev->nawds_enabled)) {
		eh = (struct ether_header *) qdf_nbuf_data(nbuf);
		if (DP_FRAME_IS_MULTICAST((eh)->ether_dhost)) {
			tx_desc->flags |= DP_TX_DESC_FLAG_TO_FW;
			is_exception = 1;
		}
	}

#if !TQM_BYPASS_WAR
	if (is_exception)
#endif
	{
		/* Temporary WAR due to TQM VP issues */
		tx_desc->flags |= DP_TX_DESC_FLAG_TO_FW;
		qdf_atomic_inc(&pdev->num_tx_exception);
	}

	return tx_desc;

failure:
	dp_tx_desc_release(tx_desc, desc_pool_id);
	return NULL;
}

/**
 * dp_tx_prepare_desc() - Allocate and prepare Tx descriptor for multisegment frame
 * @vdev: DP vdev handle
 * @nbuf: skb
 * @msdu_info: Info to be setup in MSDU descriptor and MSDU extension descriptor
 * @desc_pool_id : Descriptor Pool ID
 *
 * Allocate and prepare Tx descriptor with msdu and fragment descritor
 * information. For frames wth fragments, allocate and prepare
 * an MSDU extension descriptor
 *
 * Return: Pointer to Tx Descriptor on success,
 *         NULL on failure
 */
static struct dp_tx_desc_s *dp_tx_prepare_desc(struct dp_vdev *vdev,
		qdf_nbuf_t nbuf, struct dp_tx_msdu_info_s *msdu_info,
		uint8_t desc_pool_id)
{
	struct dp_tx_desc_s *tx_desc;
	QDF_STATUS status;
	struct dp_tx_ext_desc_elem_s *msdu_ext_desc;
	struct dp_pdev *pdev = vdev->pdev;
	struct dp_soc *soc = pdev->soc;

	/* Flow control/Congestion Control processing */
	status = dp_tx_flow_control(vdev);
	if (QDF_STATUS_E_RESOURCES == status) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
				"%s Tx Resource Full\n", __func__);
		DP_STATS_INC(vdev, tx_i.dropped.res_full, 1);
		/* TODO Stop Tx Queues */
	}

	/* Allocate software Tx descriptor */
	tx_desc = dp_tx_desc_alloc(soc, desc_pool_id);

	if (!tx_desc) {
		DP_STATS_INC(vdev, tx_i.dropped.desc_na, 1);
		return NULL;
	}

	/* Flow control/Congestion Control counters */
	qdf_atomic_inc(&pdev->num_tx_outstanding);

	/* Initialize the SW tx descriptor */
	tx_desc->nbuf = nbuf;
	tx_desc->frm_type = msdu_info->frm_type;
	tx_desc->tx_encap_type = vdev->tx_encap_type;
	tx_desc->vdev = vdev;
	tx_desc->pdev = pdev;
	tx_desc->pkt_offset = 0;
	tx_desc->tso_desc = msdu_info->u.tso_info.curr_seg;
	tx_desc->tso_num_desc = msdu_info->u.tso_info.tso_num_seg_list;

	/* Handle scattered frames - TSO/SG/ME */
	/* Allocate and prepare an extension descriptor for scattered frames */
	msdu_ext_desc = dp_tx_prepare_ext_desc(vdev, msdu_info, desc_pool_id);
	if (!msdu_ext_desc) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
				"%s Tx Extension Descriptor Alloc Fail\n",
				__func__);
		goto failure;
	}

#if TQM_BYPASS_WAR
	/* Temporary WAR due to TQM VP issues */
	tx_desc->flags |= DP_TX_DESC_FLAG_TO_FW;
	qdf_atomic_inc(&pdev->num_tx_exception);
#endif
	if (qdf_unlikely(vdev->mesh_vdev))
		tx_desc->flags |= DP_TX_DESC_FLAG_TO_FW;

	tx_desc->msdu_ext_desc = msdu_ext_desc;
	tx_desc->flags |= DP_TX_DESC_FLAG_FRAG;

	return tx_desc;
failure:
	if (qdf_unlikely(tx_desc->flags & DP_TX_DESC_FLAG_ME))
		dp_tx_me_free_buf(pdev, tx_desc->me_buffer);
	dp_tx_desc_release(tx_desc, desc_pool_id);
	return NULL;
}

/**
 * dp_tx_prepare_raw() - Prepare RAW packet TX
 * @vdev: DP vdev handle
 * @nbuf: buffer pointer
 * @seg_info: Pointer to Segment info Descriptor to be prepared
 * @msdu_info: MSDU info to be setup in MSDU descriptor and MSDU extension
 *     descriptor
 *
 * Return:
 */
static qdf_nbuf_t dp_tx_prepare_raw(struct dp_vdev *vdev, qdf_nbuf_t nbuf,
	struct dp_tx_seg_info_s *seg_info, struct dp_tx_msdu_info_s *msdu_info)
{
	qdf_nbuf_t curr_nbuf = NULL;
	uint16_t total_len = 0;
	int32_t i;

	struct dp_tx_sg_info_s *sg_info = &msdu_info->u.sg_info;
	qdf_dot3_qosframe_t *qos_wh = (qdf_dot3_qosframe_t *) nbuf->data;

	DP_STATS_INC_PKT(vdev, tx_i.raw.raw_pkt, 1, qdf_nbuf_len(nbuf));

	/* SWAR for HW: Enable WEP bit in the AMSDU frames for RAW mode */
	if ((qos_wh->i_fc[0] & IEEE80211_FC0_SUBTYPE_QOS)
			&& (qos_wh->i_qos[0] & IEEE80211_QOS_AMSDU)) {
		qos_wh->i_fc[1] |= IEEE80211_FC1_WEP;
	}

	if (QDF_STATUS_SUCCESS != qdf_nbuf_map(vdev->osdev, nbuf,
				QDF_DMA_TO_DEVICE)) {
		qdf_print("dma map error\n");
		DP_STATS_INC(vdev, tx_i.raw.dma_map_error, 1);
		qdf_nbuf_free(nbuf);
		return NULL;
	}

	for (curr_nbuf = nbuf, i = 0; curr_nbuf;
				curr_nbuf = qdf_nbuf_next(curr_nbuf), i++) {
		seg_info->frags[i].paddr_lo =
			qdf_nbuf_get_frag_paddr(curr_nbuf, 0);
		seg_info->frags[i].paddr_hi = 0x0;
		seg_info->frags[i].len = qdf_nbuf_len(curr_nbuf);
		seg_info->frags[i].vaddr = (void *) curr_nbuf;
		total_len += qdf_nbuf_len(curr_nbuf);
	}

	seg_info->frag_cnt = i;
	seg_info->total_len = total_len;
	seg_info->next = NULL;

	sg_info->curr_seg = seg_info;

	msdu_info->frm_type = dp_tx_frm_raw;
	msdu_info->num_seg = 1;

	return nbuf;
}

/**
 * dp_tx_hw_enqueue() - Enqueue to TCL HW for transmit
 * @soc: DP Soc Handle
 * @vdev: DP vdev handle
 * @tx_desc: Tx Descriptor Handle
 * @tid: TID from HLOS for overriding default DSCP-TID mapping
 * @fw_metadata: Metadata to send to Target Firmware along with frame
 * @ring_id: Ring ID of H/W ring to which we enqueue the packet
 *
 *  Gets the next free TCL HW DMA descriptor and sets up required parameters
 *  from software Tx descriptor
 *
 * Return:
 */
static QDF_STATUS dp_tx_hw_enqueue(struct dp_soc *soc, struct dp_vdev *vdev,
				   struct dp_tx_desc_s *tx_desc, uint8_t tid,
				   uint16_t fw_metadata, uint8_t ring_id)
{
	uint8_t type;
	uint16_t length;
	void *hal_tx_desc, *hal_tx_desc_cached;
	qdf_dma_addr_t dma_addr;
	uint8_t cached_desc[HAL_TX_DESC_LEN_BYTES];

	/* Return Buffer Manager ID */
	uint8_t bm_id = ring_id;
	void *hal_srng = soc->tcl_data_ring[ring_id].hal_srng;

	hal_tx_desc_cached = (void *) cached_desc;
	qdf_mem_zero_outline(hal_tx_desc_cached, HAL_TX_DESC_LEN_BYTES);

	if (tx_desc->flags & DP_TX_DESC_FLAG_FRAG) {
		length = HAL_TX_EXT_DESC_WITH_META_DATA;
		type = HAL_TX_BUF_TYPE_EXT_DESC;
		dma_addr = tx_desc->msdu_ext_desc->paddr;
	} else {
		length = qdf_nbuf_len(tx_desc->nbuf) - tx_desc->pkt_offset;
		type = HAL_TX_BUF_TYPE_BUFFER;
		dma_addr = qdf_nbuf_mapped_paddr_get(tx_desc->nbuf);
	}

	hal_tx_desc_set_fw_metadata(hal_tx_desc_cached, fw_metadata);
	hal_tx_desc_set_buf_addr(hal_tx_desc_cached,
			dma_addr , bm_id, tx_desc->id, type);
	hal_tx_desc_set_buf_length(hal_tx_desc_cached, length);
	hal_tx_desc_set_buf_offset(hal_tx_desc_cached, tx_desc->pkt_offset);
	hal_tx_desc_set_encap_type(hal_tx_desc_cached, tx_desc->tx_encap_type);
	hal_tx_desc_set_dscp_tid_table_id(hal_tx_desc_cached,
			vdev->dscp_tid_map_id);

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			"%s length:%d , type = %d, dma_addr %llx, offset %d desc id %u\n",
			__func__, length, type, (uint64_t)dma_addr,
			tx_desc->pkt_offset, tx_desc->id);

	if (tx_desc->flags & DP_TX_DESC_FLAG_TO_FW)
		hal_tx_desc_set_to_fw(hal_tx_desc_cached, 1);

	hal_tx_desc_set_addr_search_flags(hal_tx_desc_cached,
			vdev->hal_desc_addr_search_flags);

	if ((qdf_nbuf_get_tx_cksum(tx_desc->nbuf) == QDF_NBUF_TX_CKSUM_TCP_UDP)
		|| qdf_nbuf_is_tso(tx_desc->nbuf))  {
		hal_tx_desc_set_l3_checksum_en(hal_tx_desc_cached, 1);
		hal_tx_desc_set_l4_checksum_en(hal_tx_desc_cached, 1);
	}

	if (tid != HTT_TX_EXT_TID_INVALID)
		hal_tx_desc_set_hlos_tid(hal_tx_desc_cached, tid);

	if (tx_desc->flags & DP_TX_DESC_FLAG_MESH)
		hal_tx_desc_set_mesh_en(hal_tx_desc_cached, 1);


	/* Sync cached descriptor with HW */
	hal_tx_desc = hal_srng_src_get_next(soc->hal_soc, hal_srng);

	if (!hal_tx_desc) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "%s TCL ring full ring_id:%d\n", __func__, ring_id);
		DP_STATS_INC(soc, tx.tcl_ring_full[ring_id], 1);
		DP_STATS_INC(vdev, tx_i.dropped.enqueue_fail, 1);
		hal_srng_access_end(soc->hal_soc,
				soc->tcl_data_ring[ring_id].hal_srng);
		return QDF_STATUS_E_RESOURCES;
	}

	tx_desc->flags |= DP_TX_DESC_FLAG_QUEUED_TX;

	hal_tx_desc_sync(hal_tx_desc_cached, hal_tx_desc);
	DP_STATS_INC_PKT(vdev, tx_i.processed, 1, length);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_tx_classify_tid() - Obtain TID to be used for this frame
 * @vdev: DP vdev handle
 * @nbuf: skb
 *
 * Extract the DSCP or PCP information from frame and map into TID value.
 * Software based TID classification is required when more than 2 DSCP-TID
 * mapping tables are needed.
 * Hardware supports 2 DSCP-TID mapping tables
 *
 * Return: void
 */
static void dp_tx_classify_tid(struct dp_vdev *vdev, qdf_nbuf_t nbuf,
		struct dp_tx_msdu_info_s *msdu_info)
{
	uint8_t tos = 0, dscp_tid_override = 0;
	uint8_t *hdr_ptr, *L3datap;
	uint8_t is_mcast = 0;
	struct ether_header *eh = NULL;
	qdf_ethervlan_header_t *evh = NULL;
	uint16_t   ether_type;
	qdf_llc_t *llcHdr;
	struct dp_pdev *pdev = (struct dp_pdev *)vdev->pdev;

	/* for mesh packets don't do any classification */
	if (qdf_unlikely(vdev->mesh_vdev))
		return;

	if (qdf_likely(vdev->tx_encap_type != htt_cmn_pkt_type_raw)) {
		eh = (struct ether_header *) nbuf->data;
		hdr_ptr = eh->ether_dhost;
		L3datap = hdr_ptr + sizeof(struct ether_header);
	} else {
		qdf_dot3_qosframe_t *qos_wh =
			(qdf_dot3_qosframe_t *) nbuf->data;
		msdu_info->tid = qos_wh->i_fc[0] & DP_FC0_SUBTYPE_QOS ?
			qos_wh->i_qos[0] & DP_QOS_TID : 0;
		return;
	}

	is_mcast = DP_FRAME_IS_MULTICAST(hdr_ptr);
	ether_type = eh->ether_type;
	/*
	 * Check if packet is dot3 or eth2 type.
	 */
	if (IS_LLC_PRESENT(ether_type)) {
		ether_type = (uint16_t)*(nbuf->data + 2*ETHER_ADDR_LEN +
				sizeof(*llcHdr));

		if (ether_type == htons(ETHERTYPE_8021Q)) {
			L3datap = hdr_ptr + sizeof(qdf_ethervlan_header_t) +
				sizeof(*llcHdr);
			ether_type = (uint16_t)*(nbuf->data + 2*ETHER_ADDR_LEN
					+ sizeof(*llcHdr) +
					sizeof(qdf_net_vlanhdr_t));
		} else {
			L3datap = hdr_ptr + sizeof(struct ether_header) +
				sizeof(*llcHdr);
		}

	} else {
		if (ether_type == htons(ETHERTYPE_8021Q)) {
			evh = (qdf_ethervlan_header_t *) eh;
			ether_type = evh->ether_type;
			L3datap = hdr_ptr + sizeof(qdf_ethervlan_header_t);
		}
	}
	/*
	 * Find priority from IP TOS DSCP field
	 */
	if (qdf_nbuf_is_ipv4_pkt(nbuf)) {
		qdf_net_iphdr_t *ip = (qdf_net_iphdr_t *) L3datap;
		if (qdf_nbuf_is_ipv4_dhcp_pkt(nbuf)) {
			/* Only for unicast frames */
			if (!is_mcast) {
				/* send it on VO queue */
				msdu_info->tid = DP_VO_TID;
			}
		} else {
			/*
			 * IP frame: exclude ECN bits 0-1 and map DSCP bits 2-7
			 * from TOS byte.
			 */
			tos = ip->ip_tos;
			dscp_tid_override = 1;

		}
	} else if (qdf_nbuf_is_ipv6_pkt(nbuf)) {
		/* TODO
		 * use flowlabel
		 *igmpmld cases to be handled in phase 2
		 */
		unsigned long ver_pri_flowlabel;
		unsigned long pri;
		ver_pri_flowlabel = *(unsigned long *) L3datap;
		pri = (ntohl(ver_pri_flowlabel) & IPV6_FLOWINFO_PRIORITY) >>
			DP_IPV6_PRIORITY_SHIFT;
		tos = pri;
		dscp_tid_override = 1;
	} else if (qdf_nbuf_is_ipv4_eapol_pkt(nbuf))
		msdu_info->tid = DP_VO_TID;
	else if (qdf_nbuf_is_ipv4_arp_pkt(nbuf)) {
		/* Only for unicast frames */
		if (!is_mcast) {
			/* send ucast arp on VO queue */
			msdu_info->tid = DP_VO_TID;
		}
	}

	/*
	 * Assign all MCAST packets to BE
	 */
	if (qdf_unlikely(vdev->tx_encap_type != htt_cmn_pkt_type_raw)) {
		if (is_mcast) {
			tos = 0;
			dscp_tid_override = 1;
		}
	}

	if (dscp_tid_override == 1) {
		tos = (tos >> DP_IP_DSCP_SHIFT) & DP_IP_DSCP_MASK;
		msdu_info->tid = pdev->dscp_tid_map[vdev->dscp_tid_map_id][tos];
	}
	return;
}

/**
 * dp_tx_send_msdu_single() - Setup descriptor and enqueue single MSDU to TCL
 * @vdev: DP vdev handle
 * @nbuf: skb
 * @tid: TID from HLOS for overriding default DSCP-TID mapping
 * @tx_q: Tx queue to be used for this Tx frame
 * @peer_id: peer_id of the peer in case of NAWDS frames
 *
 * Return: NULL on success,
 *         nbuf when it fails to send
 */
static qdf_nbuf_t dp_tx_send_msdu_single(struct dp_vdev *vdev, qdf_nbuf_t nbuf,
		uint8_t tid, struct dp_tx_queue *tx_q,
		uint32_t *meta_data, uint16_t peer_id)
{
	struct dp_pdev *pdev = vdev->pdev;
	struct dp_soc *soc = pdev->soc;
	struct dp_tx_desc_s *tx_desc;
	QDF_STATUS status;
	void *hal_srng = soc->tcl_data_ring[tx_q->ring_id].hal_srng;
	uint16_t htt_tcl_metadata = 0;

	HTT_TX_TCL_METADATA_VALID_HTT_SET(htt_tcl_metadata, 0);
	/* Setup Tx descriptor for an MSDU, and MSDU extension descriptor */
	tx_desc = dp_tx_prepare_desc_single(vdev, nbuf, tx_q->desc_pool_id, meta_data);
	if (!tx_desc) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "%s Tx_desc prepare Fail vdev %p queue %d\n",
			  __func__, vdev, tx_q->desc_pool_id);
		goto fail_return;
	}

	if (qdf_unlikely(hal_srng_access_start(soc->hal_soc, hal_srng))) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s %d : HAL RING Access Failed -- %p\n",
				__func__, __LINE__, hal_srng);
		DP_STATS_INC(vdev, tx_i.dropped.ring_full, 1);
		goto fail_return;
	}

	if (qdf_unlikely(peer_id != HTT_INVALID_PEER)) {
		HTT_TX_TCL_METADATA_TYPE_SET(htt_tcl_metadata,
				HTT_TCL_METADATA_TYPE_PEER_BASED);
		HTT_TX_TCL_METADATA_PEER_ID_SET(htt_tcl_metadata,
				peer_id);
	} else
		htt_tcl_metadata = vdev->htt_tcl_metadata;

	/* Enqueue the Tx MSDU descriptor to HW for transmit */
	status = dp_tx_hw_enqueue(soc, vdev, tx_desc, tid,
			htt_tcl_metadata, tx_q->ring_id);

	if (status != QDF_STATUS_SUCCESS) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "%s Tx_hw_enqueue Fail tx_desc %p queue %d\n",
			  __func__, tx_desc, tx_q->ring_id);
		dp_tx_desc_release(tx_desc, tx_q->desc_pool_id);
		goto fail_return;
	}

	hal_srng_access_end(soc->hal_soc, hal_srng);

	return NULL;

fail_return:
	return nbuf;
}

/**
 * dp_tx_send_msdu_multiple() - Enqueue multiple MSDUs
 * @vdev: DP vdev handle
 * @nbuf: skb
 * @msdu_info: MSDU info to be setup in MSDU extension descriptor
 *
 * Prepare descriptors for multiple MSDUs (TSO segments) and enqueue to TCL
 *
 * Return: NULL on success,
 *         nbuf when it fails to send
 */
#if QDF_LOCK_STATS
static noinline
#else
static
#endif
qdf_nbuf_t dp_tx_send_msdu_multiple(struct dp_vdev *vdev, qdf_nbuf_t nbuf,
				    struct dp_tx_msdu_info_s *msdu_info)
{
	uint8_t i;
	struct dp_pdev *pdev = vdev->pdev;
	struct dp_soc *soc = pdev->soc;
	struct dp_tx_desc_s *tx_desc;
	QDF_STATUS status;

	struct dp_tx_queue *tx_q = &msdu_info->tx_queue;
	void *hal_srng = soc->tcl_data_ring[tx_q->ring_id].hal_srng;

	if (qdf_unlikely(hal_srng_access_start(soc->hal_soc, hal_srng))) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s %d : HAL RING Access Failed -- %p\n",
				__func__, __LINE__, hal_srng);
		DP_STATS_INC(vdev, tx_i.dropped.ring_full, 1);
		return nbuf;
	}

	if (msdu_info->frm_type == dp_tx_frm_me)
		nbuf = msdu_info->u.sg_info.curr_seg->nbuf;

	i = 0;
	/* Print statement to track i and num_seg */
	/*
	 * For each segment (maps to 1 MSDU) , prepare software and hardware
	 * descriptors using information in msdu_info
	 */
	while (i < msdu_info->num_seg) {
		/*
		 * Setup Tx descriptor for an MSDU, and MSDU extension
		 * descriptor
		 */
		tx_desc = dp_tx_prepare_desc(vdev, nbuf, msdu_info,
				tx_q->desc_pool_id);

		if (msdu_info->frm_type == dp_tx_frm_me) {
			tx_desc->me_buffer =
				msdu_info->u.sg_info.curr_seg->frags[0].vaddr;
			tx_desc->flags |= DP_TX_DESC_FLAG_ME;
		}

		if (!tx_desc) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				  "%s Tx_desc prepare Fail vdev %p queue %d\n",
				  __func__, vdev, tx_q->desc_pool_id);

			if (tx_desc->flags & DP_TX_DESC_FLAG_ME)
				dp_tx_me_free_buf(pdev, tx_desc->me_buffer);
			dp_tx_desc_release(tx_desc, tx_q->desc_pool_id);
			goto done;
		}

		/*
		 * Enqueue the Tx MSDU descriptor to HW for transmit
		 */
		status = dp_tx_hw_enqueue(soc, vdev, tx_desc, msdu_info->tid,
			vdev->htt_tcl_metadata, tx_q->ring_id);

		if (status != QDF_STATUS_SUCCESS) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				  "%s Tx_hw_enqueue Fail tx_desc %p queue %d\n",
				  __func__, tx_desc, tx_q->ring_id);

			if (tx_desc->flags & DP_TX_DESC_FLAG_ME)
				dp_tx_me_free_buf(pdev, tx_desc->me_buffer);

			dp_tx_desc_release(tx_desc, tx_q->desc_pool_id);
			goto done;
		}

		/*
		 * TODO
		 * if tso_info structure can be modified to have curr_seg
		 * as first element, following 2 blocks of code (for TSO and SG)
		 * can be combined into 1
		 */

		/*
		 * For frames with multiple segments (TSO, ME), jump to next
		 * segment.
		 */
		if (msdu_info->frm_type == dp_tx_frm_tso) {
			if (msdu_info->u.tso_info.curr_seg->next) {
				msdu_info->u.tso_info.curr_seg =
					msdu_info->u.tso_info.curr_seg->next;

				/*
				 * If this is a jumbo nbuf, then increment the number of
				 * nbuf users for each additional segment of the msdu.
				 * This will ensure that the skb is freed only after
				 * receiving tx completion for all segments of an nbuf
				 */
				qdf_nbuf_inc_users(nbuf);

				/* Check with MCL if this is needed */
				/* nbuf = msdu_info->u.tso_info.curr_seg->nbuf; */
			}
		}

		/*
		 * For Multicast-Unicast converted packets,
		 * each converted frame (for a client) is represented as
		 * 1 segment
		 */
		if ((msdu_info->frm_type == dp_tx_frm_sg) ||
				(msdu_info->frm_type == dp_tx_frm_me)) {
			if (msdu_info->u.sg_info.curr_seg->next) {
				msdu_info->u.sg_info.curr_seg =
					msdu_info->u.sg_info.curr_seg->next;
				nbuf = msdu_info->u.sg_info.curr_seg->nbuf;
			}
		}
		i++;
	}

	nbuf = NULL;

done:
	hal_srng_access_end(soc->hal_soc, hal_srng);

	return nbuf;
}

/**
 * dp_tx_prepare_sg()- Extract SG info from NBUF and prepare msdu_info
 *                     for SG frames
 * @vdev: DP vdev handle
 * @nbuf: skb
 * @seg_info: Pointer to Segment info Descriptor to be prepared
 * @msdu_info: MSDU info to be setup in MSDU descriptor and MSDU extension desc.
 *
 * Return: NULL on success,
 *         nbuf when it fails to send
 */
static qdf_nbuf_t dp_tx_prepare_sg(struct dp_vdev *vdev, qdf_nbuf_t nbuf,
	struct dp_tx_seg_info_s *seg_info, struct dp_tx_msdu_info_s *msdu_info)
{
	uint32_t cur_frag, nr_frags;
	qdf_dma_addr_t paddr;
	struct dp_tx_sg_info_s *sg_info;

	sg_info = &msdu_info->u.sg_info;
	nr_frags = qdf_nbuf_get_nr_frags(nbuf);

	if (QDF_STATUS_SUCCESS != qdf_nbuf_map(vdev->osdev, nbuf,
				QDF_DMA_TO_DEVICE)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"dma map error\n");
		DP_STATS_INC(vdev, tx_i.sg.dma_map_error, 1);

		qdf_nbuf_free(nbuf);
		return NULL;
	}

	seg_info->frags[0].paddr_lo = qdf_nbuf_get_frag_paddr(nbuf, 0);
	seg_info->frags[0].paddr_hi = 0;
	seg_info->frags[0].len = qdf_nbuf_headlen(nbuf);
	seg_info->frags[0].vaddr = (void *) nbuf;

	for (cur_frag = 0; cur_frag < nr_frags; cur_frag++) {
		if (QDF_STATUS_E_FAILURE == qdf_nbuf_frag_map(vdev->osdev,
					nbuf, 0, QDF_DMA_TO_DEVICE, cur_frag)) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
					"frag dma map error\n");
			DP_STATS_INC(vdev, tx_i.sg.dma_map_error, 1);
			qdf_nbuf_free(nbuf);
			return NULL;
		}

		paddr = qdf_nbuf_get_frag_paddr(nbuf, 0);
		seg_info->frags[cur_frag + 1].paddr_lo = paddr;
		seg_info->frags[cur_frag + 1].paddr_hi =
			((uint64_t) paddr) >> 32;
		seg_info->frags[cur_frag + 1].len =
			qdf_nbuf_get_frag_size(nbuf, cur_frag);
	}

	seg_info->frag_cnt = (cur_frag + 1);
	seg_info->total_len = qdf_nbuf_len(nbuf);
	seg_info->next = NULL;

	sg_info->curr_seg = seg_info;

	msdu_info->frm_type = dp_tx_frm_sg;
	msdu_info->num_seg = 1;

	return nbuf;
}

#ifdef MESH_MODE_SUPPORT

/**
 * dp_tx_extract_mesh_meta_data()- Extract mesh meta hdr info from nbuf
				and prepare msdu_info for mesh frames.
 * @vdev: DP vdev handle
 * @nbuf: skb
 * @msdu_info: MSDU info to be setup in MSDU descriptor and MSDU extension desc.
 *
 * Return: void
 */
static
void dp_tx_extract_mesh_meta_data(struct dp_vdev *vdev, qdf_nbuf_t nbuf,
				struct dp_tx_msdu_info_s *msdu_info)
{
	struct meta_hdr_s *mhdr;
	struct htt_tx_msdu_desc_ext2_t *meta_data =
				(struct htt_tx_msdu_desc_ext2_t *)&msdu_info->meta_data[0];

	mhdr = (struct meta_hdr_s *)qdf_nbuf_data(nbuf);

	qdf_mem_set(meta_data, 0, sizeof(struct htt_tx_msdu_desc_ext2_t));

	if (!(mhdr->flags & METAHDR_FLAG_AUTO_RATE)) {
		meta_data->power = mhdr->power;

		meta_data->mcs_mask = 1 << mhdr->rate_info[0].mcs;
		meta_data->nss_mask = 1 << mhdr->rate_info[0].nss;
		meta_data->pream_type = mhdr->rate_info[0].preamble_type;
		meta_data->retry_limit = mhdr->rate_info[0].max_tries;

		meta_data->dyn_bw = 1;

		meta_data->valid_pwr = 1;
		meta_data->valid_mcs_mask = 1;
		meta_data->valid_nss_mask = 1;
		meta_data->valid_preamble_type  = 1;
		meta_data->valid_retries = 1;
		meta_data->valid_bw_info = 1;
	}

	if (mhdr->flags & METAHDR_FLAG_NOENCRYPT) {
		meta_data->encrypt_type = 0;
		meta_data->valid_encrypt_type = 1;
	}

	if (mhdr->flags & METAHDR_FLAG_NOQOS)
		msdu_info->tid = HTT_TX_EXT_TID_NON_QOS_MCAST_BCAST;
	else
		msdu_info->tid = qdf_nbuf_get_priority(nbuf);

	meta_data->valid_key_flags = 1;
	meta_data->key_flags = (mhdr->keyix & 0x3);

	qdf_nbuf_pull_head(nbuf, sizeof(struct meta_hdr_s));

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"%s , Meta hdr %0x %0x %0x %0x %0x\n",
			__func__, msdu_info->meta_data[0],
			msdu_info->meta_data[1],
			msdu_info->meta_data[2],
			msdu_info->meta_data[3],
			msdu_info->meta_data[4]);

	return;
}
#else
static
void dp_tx_extract_mesh_meta_data(struct dp_vdev *vdev, qdf_nbuf_t nbuf,
				struct dp_tx_msdu_info_s *msdu_info)
{
}

#endif

/**
 * dp_tx_prepare_nawds(): Tramit NAWDS frames
 * @vdev: dp_vdev handle
 * @nbuf: skb
 * @tid: TID from HLOS for overriding default DSCP-TID mapping
 * @tx_q: Tx queue to be used for this Tx frame
 * @meta_data: Meta date for mesh
 * @peer_id: peer_id of the peer in case of NAWDS frames
 *
 * return: NULL on success nbuf on failure
 */
static qdf_nbuf_t dp_tx_prepare_nawds(struct dp_vdev *vdev, qdf_nbuf_t nbuf,
		uint8_t tid, struct dp_tx_queue *tx_q, uint32_t *meta_data,
		uint32_t peer_id)
{
	struct dp_peer *peer = NULL;
	qdf_nbuf_t nbuf_copy;
	TAILQ_FOREACH(peer, &vdev->peer_list, peer_list_elem) {
		if ((peer->peer_ids[0] != HTT_INVALID_PEER) &&
				(peer->nawds_enabled || peer->bss_peer)) {
			nbuf_copy = qdf_nbuf_copy(nbuf);
			if (!nbuf_copy) {
				QDF_TRACE(QDF_MODULE_ID_DP,
						QDF_TRACE_LEVEL_ERROR,
						"nbuf copy failed");
			}

			peer_id = peer->peer_ids[0];
			nbuf_copy = dp_tx_send_msdu_single(vdev, nbuf_copy, tid,
					tx_q, meta_data, peer_id);
			if (nbuf_copy != NULL) {
				qdf_nbuf_free(nbuf);
				return nbuf_copy;
			}
		}
	}
	if (peer_id == HTT_INVALID_PEER)
		return nbuf;

	qdf_nbuf_free(nbuf);
	return NULL;
}

/**
 * dp_tx_send() - Transmit a frame on a given VAP
 * @vap_dev: DP vdev handle
 * @nbuf: skb
 *
 * Entry point for Core Tx layer (DP_TX) invoked from
 * hard_start_xmit in OSIF/HDD or from dp_rx_process for intravap forwarding
 * cases
 *
 * Return: NULL on success,
 *         nbuf when it fails to send
 */
qdf_nbuf_t dp_tx_send(void *vap_dev, qdf_nbuf_t nbuf)
{
	struct ether_header *eh = NULL;
	struct dp_tx_msdu_info_s msdu_info;
	struct dp_tx_seg_info_s seg_info;
	struct dp_vdev *vdev = (struct dp_vdev *) vap_dev;
	uint16_t peer_id = HTT_INVALID_PEER;

	qdf_mem_set(&msdu_info, sizeof(msdu_info), 0x0);
	qdf_mem_set(&seg_info, sizeof(seg_info), 0x0);

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"%s , skb %0x:%0x:%0x:%0x:%0x:%0x\n",
			__func__, nbuf->data[0], nbuf->data[1], nbuf->data[2],
			nbuf->data[3], nbuf->data[4], nbuf->data[5]);
	/*
	 * Set Default Host TID value to invalid TID
	 * (TID override disabled)
	 */
	msdu_info.tid = HTT_TX_EXT_TID_INVALID;
	DP_STATS_INC_PKT(vdev, tx_i.rcvd, 1, qdf_nbuf_len(nbuf));

	if (qdf_unlikely(vdev->mesh_vdev))
		dp_tx_extract_mesh_meta_data(vdev, nbuf, &msdu_info);

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"%s , skb %0x:%0x:%0x:%0x:%0x:%0x\n",
			__func__, nbuf->data[0], nbuf->data[1], nbuf->data[2],
			nbuf->data[3], nbuf->data[4], nbuf->data[5]);
	/*
	 * Get HW Queue to use for this frame.
	 * TCL supports upto 4 DMA rings, out of which 3 rings are
	 * dedicated for data and 1 for command.
	 * "queue_id" maps to one hardware ring.
	 *  With each ring, we also associate a unique Tx descriptor pool
	 *  to minimize lock contention for these resources.
	 */
	dp_tx_get_queue(vdev, nbuf, &msdu_info.tx_queue);

	/*
	 * TCL H/W supports 2 DSCP-TID mapping tables.
	 *  Table 1 - Default DSCP-TID mapping table
	 *  Table 2 - 1 DSCP-TID override table
	 *
	 * If we need a different DSCP-TID mapping for this vap,
	 * call tid_classify to extract DSCP/ToS from frame and
	 * map to a TID and store in msdu_info. This is later used
	 * to fill in TCL Input descriptor (per-packet TID override).
	 */
	if (vdev->dscp_tid_map_id > 1)
		dp_tx_classify_tid(vdev, nbuf, &msdu_info);

	/* Reset the control block */
	qdf_nbuf_reset_ctxt(nbuf);

	/*
	 * Classify the frame and call corresponding
	 * "prepare" function which extracts the segment (TSO)
	 * and fragmentation information (for TSO , SG, ME, or Raw)
	 * into MSDU_INFO structure which is later used to fill
	 * SW and HW descriptors.
	 */
	if (qdf_nbuf_is_tso(nbuf)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "%s TSO frame %p\n", __func__, vdev);
		DP_STATS_INC_PKT(vdev, tx_i.tso.tso_pkt, 1,
				qdf_nbuf_len(nbuf));

		if (dp_tx_prepare_tso(vdev, nbuf, &msdu_info)) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
					"%s tso_prepare fail vdev_id:%d\n",
					__func__, vdev->vdev_id);
			DP_STATS_INC(vdev, tx_i.tso.dropped_host, 1);
			return nbuf;
		}

		goto send_multiple;
	}

	/* SG */
	if (qdf_unlikely(qdf_nbuf_is_nonlinear(nbuf))) {
		nbuf = dp_tx_prepare_sg(vdev, nbuf, &seg_info, &msdu_info);

		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			 "%s non-TSO SG frame %p\n", __func__, vdev);

		DP_STATS_INC_PKT(vdev, tx_i.sg.sg_pkt, 1,
				qdf_nbuf_len(nbuf));

		goto send_multiple;
	}

#ifdef ATH_SUPPORT_IQUE
	/* Mcast to Ucast Conversion*/
	if (qdf_unlikely(vdev->mcast_enhancement_en > 0)) {
		eh = (struct ether_header *)qdf_nbuf_data(nbuf);
		if (DP_FRAME_IS_MULTICAST((eh)->ether_dhost)) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
				  "%s Mcast frm for ME %p\n", __func__, vdev);

			DP_STATS_INC_PKT(vdev,
					tx_i.mcast_en.mcast_pkt, 1,
					qdf_nbuf_len(nbuf));
			if (dp_tx_prepare_send_me(vdev, nbuf)) {
				qdf_nbuf_free(nbuf);
				return NULL;
			}
			return nbuf;
		}
	}
#endif

	/* RAW */
	if (qdf_unlikely(vdev->tx_encap_type == htt_cmn_pkt_type_raw)) {
		nbuf = dp_tx_prepare_raw(vdev, nbuf, &seg_info, &msdu_info);
		if (nbuf == NULL)
			return NULL;

		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "%s Raw frame %p\n", __func__, vdev);

		goto send_multiple;

	}

	if (vdev->nawds_enabled) {
		eh = (struct ether_header *)qdf_nbuf_data(nbuf);
		if (DP_FRAME_IS_MULTICAST((eh)->ether_dhost)) {
			nbuf = dp_tx_prepare_nawds(vdev, nbuf, msdu_info.tid,
					&msdu_info.tx_queue,
					msdu_info.meta_data, peer_id);
			return nbuf;
		}
	}

	/*  Single linear frame */
	/*
	 * If nbuf is a simple linear frame, use send_single function to
	 * prepare direct-buffer type TCL descriptor and enqueue to TCL
	 * SRNG. There is no need to setup a MSDU extension descriptor.
	 */
	nbuf = dp_tx_send_msdu_single(vdev, nbuf, msdu_info.tid,
			&msdu_info.tx_queue, msdu_info.meta_data, peer_id);

	return nbuf;

send_multiple:
	nbuf = dp_tx_send_msdu_multiple(vdev, nbuf, &msdu_info);

	return nbuf;
}

/**
 * dp_tx_reinject_handler() - Tx Reinject Handler
 * @tx_desc: software descriptor head pointer
 * @status : Tx completion status from HTT descriptor
 *
 * This function reinjects frames back to Target.
 * Todo - Host queue needs to be added
 *
 * Return: none
 */
static
void dp_tx_reinject_handler(struct dp_tx_desc_s *tx_desc, uint8_t *status)
{
	struct dp_vdev *vdev;
	struct dp_peer *peer = NULL;
	uint32_t peer_id = HTT_INVALID_PEER;
	qdf_nbuf_t nbuf = tx_desc->nbuf;
	qdf_nbuf_t nbuf_copy = NULL;
	struct dp_tx_msdu_info_s msdu_info;

	vdev = tx_desc->vdev;

	qdf_assert(vdev);

	qdf_mem_set(&msdu_info, sizeof(msdu_info), 0x0);

	dp_tx_get_queue(vdev, nbuf, &msdu_info.tx_queue);

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			"%s Tx reinject path\n", __func__);

	DP_STATS_INC_PKT(vdev, tx_i.reinject_pkts, 1,
			qdf_nbuf_len(tx_desc->nbuf));

	if (!vdev->osif_proxy_arp) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"function pointer to proxy arp not present\n");
		return;
	}

	if (qdf_unlikely(vdev->mesh_vdev)) {
		DP_TX_FREE_SINGLE_BUF(vdev->pdev->soc, tx_desc->nbuf);
	} else {
		TAILQ_FOREACH(peer, &vdev->peer_list, peer_list_elem) {
			if ((peer->peer_ids[0] != HTT_INVALID_PEER) &&
					(peer->bss_peer || peer->nawds_enabled)
					&& !(vdev->osif_proxy_arp(
							vdev->osif_vdev,
							nbuf))) {
				nbuf_copy = qdf_nbuf_copy(nbuf);

				if (!nbuf_copy) {
					QDF_TRACE(QDF_MODULE_ID_DP,
							QDF_TRACE_LEVEL_ERROR,
							FL("nbuf copy failed"));
					break;
				}

				if (peer->nawds_enabled)
					peer_id = peer->peer_ids[0];
				else
					peer_id = HTT_INVALID_PEER;

				nbuf_copy = dp_tx_send_msdu_single(vdev,
						nbuf_copy, msdu_info.tid,
						&msdu_info.tx_queue,
						msdu_info.meta_data, peer_id);

				if (nbuf_copy) {
					QDF_TRACE(QDF_MODULE_ID_DP,
							QDF_TRACE_LEVEL_ERROR,
							FL("pkt send failed"));
					qdf_nbuf_free(nbuf_copy);
				}
			}
		}
	}

	qdf_nbuf_free(nbuf);
	dp_tx_desc_release(tx_desc, tx_desc->pool_id);
}

/**
 * dp_tx_inspect_handler() - Tx Inspect Handler
 * @tx_desc: software descriptor head pointer
 * @status : Tx completion status from HTT descriptor
 *
 * Handles Tx frames sent back to Host for inspection
 * (ProxyARP)
 *
 * Return: none
 */
static void dp_tx_inspect_handler(struct dp_tx_desc_s *tx_desc, uint8_t *status)
{

	struct dp_soc *soc;
	struct dp_pdev *pdev = tx_desc->pdev;

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			"%s Tx inspect path\n",
			__func__);

	qdf_assert(pdev);

	soc = pdev->soc;

	DP_STATS_INC_PKT(tx_desc->vdev, tx_i.inspect_pkts, 1,
			qdf_nbuf_len(tx_desc->nbuf));

	DP_TX_FREE_SINGLE_BUF(soc, tx_desc->nbuf);
}

/**
 * dp_tx_process_htt_completion() - Tx HTT Completion Indication Handler
 * @tx_desc: software descriptor head pointer
 * @status : Tx completion status from HTT descriptor
 *
 * This function will process HTT Tx indication messages from Target
 *
 * Return: none
 */
static
void dp_tx_process_htt_completion(struct dp_tx_desc_s *tx_desc, uint8_t *status)
{
	uint8_t tx_status;
	struct dp_pdev *pdev;
	struct dp_soc *soc;
	uint32_t *htt_status_word = (uint32_t *) status;

	qdf_assert(tx_desc->pdev);

	pdev = tx_desc->pdev;
	soc = pdev->soc;

	tx_status = HTT_TX_WBM_COMPLETION_TX_STATUS_GET(htt_status_word[0]);

	switch (tx_status) {
	case HTT_TX_FW2WBM_TX_STATUS_OK:
	{
		qdf_atomic_dec(&pdev->num_tx_exception);
		DP_TX_FREE_SINGLE_BUF(soc, tx_desc->nbuf);
		break;
	}
	case HTT_TX_FW2WBM_TX_STATUS_DROP:
	case HTT_TX_FW2WBM_TX_STATUS_TTL:
	{
		qdf_atomic_dec(&pdev->num_tx_exception);
		DP_TX_FREE_SINGLE_BUF(soc, tx_desc->nbuf);
		break;
	}
	case HTT_TX_FW2WBM_TX_STATUS_REINJECT:
	{
		dp_tx_reinject_handler(tx_desc, status);
		break;
	}
	case HTT_TX_FW2WBM_TX_STATUS_INSPECT:
	{
		dp_tx_inspect_handler(tx_desc, status);
		break;
	}
	default:
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"%s Invalid HTT tx_status %d\n",
				__func__, tx_status);
		break;
	}
}

#ifdef MESH_MODE_SUPPORT
/**
 * dp_tx_comp_fill_tx_completion_stats() - Fill per packet Tx completion stats
 *                                         in mesh meta header
 * @tx_desc: software descriptor head pointer
 * @ts: pointer to tx completion stats
 * Return: none
 */
static
void dp_tx_comp_fill_tx_completion_stats(struct dp_tx_desc_s *tx_desc,
		struct hal_tx_completion_status *ts)
{
	struct meta_hdr_s *mhdr;
	qdf_nbuf_t netbuf = tx_desc->nbuf;

	if (!tx_desc->msdu_ext_desc) {
		qdf_nbuf_pull_head(netbuf, tx_desc->pkt_offset);
	}
	qdf_nbuf_push_head(netbuf, sizeof(struct meta_hdr_s));

	mhdr = (struct meta_hdr_s *)qdf_nbuf_data(netbuf);
	mhdr->rssi = ts->ack_frame_rssi;
	mhdr->channel = tx_desc->pdev->operating_channel;
}

#else
static
void dp_tx_comp_fill_tx_completion_stats(struct dp_tx_desc_s *tx_desc,
		struct hal_tx_completion_status *ts)
{
}

#endif


/**
 * dp_tx_comp_process_tx_status() - Parse and Dump Tx completion status info
 * @tx_desc: software descriptor head pointer
 * @length: packet length
 *
 * Return: none
 */
static inline void dp_tx_comp_process_tx_status(struct dp_tx_desc_s *tx_desc,
		uint32_t length)
{
	struct hal_tx_completion_status ts;
	struct dp_soc *soc = NULL;
	struct dp_vdev *vdev = tx_desc->vdev;
	struct dp_peer *peer = NULL;
	uint8_t comp_status = 0;
	qdf_mem_zero(&ts, sizeof(struct hal_tx_completion_status));
	hal_tx_comp_get_status(&tx_desc->comp, &ts);
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
				"-------------------- \n"
				"Tx Completion Stats: \n"
				"-------------------- \n"
				"ack_frame_rssi = %d \n"
				"first_msdu = %d \n"
				"last_msdu = %d \n"
				"msdu_part_of_amsdu = %d \n"
				"rate_stats valid = %d \n"
				"bw = %d \n"
				"pkt_type = %d \n"
				"stbc = %d \n"
				"ldpc = %d \n"
				"sgi = %d \n"
				"mcs = %d \n"
				"ofdma = %d \n"
				"tones_in_ru = %d \n"
				"tsf = %d \n"
				"ppdu_id = %d \n"
				"transmit_cnt = %d \n"
				"tid = %d \n"
				"peer_id = %d \n",
				ts.ack_frame_rssi, ts.first_msdu, ts.last_msdu,
				ts.msdu_part_of_amsdu, ts.valid, ts.bw,
				ts.pkt_type, ts.stbc, ts.ldpc, ts.sgi,
				ts.mcs, ts.ofdma, ts.tones_in_ru, ts.tsf,
				ts.ppdu_id, ts.transmit_cnt, ts.tid,
				ts.peer_id);

	if (qdf_unlikely(tx_desc->vdev->mesh_vdev))
		dp_tx_comp_fill_tx_completion_stats(tx_desc, &ts);

	if (!vdev) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
				"invalid peer");
		goto fail;
	}

	soc = tx_desc->vdev->pdev->soc;
	peer = dp_peer_find_by_id(soc, ts.peer_id);
	if (!peer) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
				"invalid peer");
		DP_STATS_INC_PKT(soc, tx.tx_invalid_peer, 1, length);
		goto out;
	}

	DP_STATS_INC_PKT(peer, tx.comp_pkt, 1, length);

	if (HAL_TX_COMP_RELEASE_SOURCE_TQM ==
				hal_tx_comp_get_buffer_source(&tx_desc->comp)) {
		comp_status = hal_tx_comp_get_release_reason(&tx_desc->comp);

		DP_STATS_INCC(peer, tx.dropped.mpdu_age_out, 1,
				(comp_status == HAL_TX_TQM_RR_REM_CMD_AGED));
		DP_STATS_INCC(peer, tx.dropped.fw_discard_reason1, 1,
				(comp_status == HAL_TX_TQM_RR_FW_REASON1));
		DP_STATS_INCC(peer, tx.dropped.fw_discard_reason2, 1,
				(comp_status == HAL_TX_TQM_RR_FW_REASON2));
		DP_STATS_INCC(peer, tx.dropped.fw_discard_reason3, 1,
				(comp_status == HAL_TX_TQM_RR_FW_REASON3));
		DP_STATS_INCC(peer, tx.tx_failed, 1,
				comp_status != HAL_TX_TQM_RR_FRAME_ACKED);

		if (comp_status == HAL_TX_TQM_RR_FRAME_ACKED) {
			DP_STATS_INCC(peer, tx.pkt_type[ts.pkt_type].
					mcs_count[MAX_MCS], 1,
					((ts.mcs >= MAX_MCS_11A) && (ts.pkt_type
						== DOT11_A)));
			DP_STATS_INCC(peer, tx.pkt_type[ts.pkt_type].
					mcs_count[ts.mcs], 1,
					((ts.mcs <= MAX_MCS_11A) && (ts.pkt_type
						== DOT11_A)));
			DP_STATS_INCC(peer, tx.pkt_type[ts.pkt_type].
					mcs_count[MAX_MCS], 1,
					((ts.mcs >= MAX_MCS_11B)
					 && (ts.pkt_type == DOT11_B)));
			DP_STATS_INCC(peer, tx.pkt_type[ts.pkt_type].
					mcs_count[ts.mcs], 1,
					((ts.mcs <= MAX_MCS_11B)
					 && (ts.pkt_type == DOT11_B)));
			DP_STATS_INCC(peer, tx.pkt_type[ts.pkt_type].
					mcs_count[MAX_MCS], 1,
					((ts.mcs >= MAX_MCS_11A)
					 && (ts.pkt_type == DOT11_N)));
			DP_STATS_INCC(peer, tx.pkt_type[ts.pkt_type].
					mcs_count[ts.mcs], 1,
					((ts.mcs <= MAX_MCS_11A)
					 && (ts.pkt_type == DOT11_N)));
			DP_STATS_INCC(peer, tx.pkt_type[ts.pkt_type].
					mcs_count[MAX_MCS], 1,
					((ts.mcs >= MAX_MCS_11AC)
					 && (ts.pkt_type == DOT11_AC)));
			DP_STATS_INCC(peer, tx.pkt_type[ts.pkt_type].
					mcs_count[ts.mcs], 1,
					((ts.mcs <= MAX_MCS_11AC)
					 && (ts.pkt_type == DOT11_AC)));
			DP_STATS_INCC(peer, tx.pkt_type[ts.pkt_type].
					mcs_count[MAX_MCS], 1,
					((ts.mcs >= MAX_MCS)
					 && (ts.pkt_type == DOT11_AX)));
			DP_STATS_INCC(peer, tx.pkt_type[ts.pkt_type].
					mcs_count[ts.mcs], 1,
					((ts.mcs <= MAX_MCS)
					 && (ts.pkt_type == DOT11_AX)));

			DP_STATS_INC(peer, tx.sgi_count[ts.sgi], 1);
			DP_STATS_INC(peer, tx.bw[ts.bw], 1);
			DP_STATS_UPD(peer, tx.last_ack_rssi, ts.ack_frame_rssi);
			DP_STATS_INC(peer, tx.wme_ac_type[TID_TO_WME_AC(ts.tid)]
					, 1);
			DP_STATS_INC_PKT(peer, tx.tx_success, 1, length);
			DP_STATS_INCC(peer, tx.stbc, 1, ts.stbc);
			DP_STATS_INCC(peer, tx.ofdma, 1, ts.ofdma);
			DP_STATS_INCC(peer, tx.ldpc, 1, ts.ldpc);
			DP_STATS_INCC(peer, tx.non_amsdu_cnt, 1,
					(ts.first_msdu && ts.last_msdu));
			DP_STATS_INCC(peer, tx.amsdu_cnt, 1,
					!(ts.first_msdu && ts.last_msdu));
			DP_STATS_INCC(peer, tx.retries, 1, ts.transmit_cnt > 1);
		}
	}

	/* TODO: This call is temporary.
	 * Stats update has to be attached to the HTT PPDU message
	 */
	if (soc->cdp_soc.ol_ops->update_dp_stats)
		soc->cdp_soc.ol_ops->update_dp_stats(vdev->pdev->osif_pdev,
				&peer->stats, ts.peer_id, UPDATE_PEER_STATS);

out:
	dp_aggregate_vdev_stats(tx_desc->vdev);
	if (soc->cdp_soc.ol_ops->update_dp_stats)
		soc->cdp_soc.ol_ops->update_dp_stats(vdev->pdev->osif_pdev,
				&vdev->stats, vdev->vdev_id, UPDATE_VDEV_STATS);
fail:
	return;
}


/**
 * dp_tx_comp_process_desc() - Tx complete software descriptor handler
 * @soc: core txrx main context
 * @comp_head: software descriptor head pointer
 *
 * This function will process batch of descriptors reaped by dp_tx_comp_handler
 * and release the software descriptors after processing is complete
 *
 * Return: none
 */
static void dp_tx_comp_process_desc(struct dp_soc *soc,
				    struct dp_tx_desc_s *comp_head)
{
	struct dp_tx_desc_s *desc;
	struct dp_tx_desc_s *next;
	struct hal_tx_completion_status ts = {0};
	uint32_t length;
	struct dp_peer *peer;

	DP_HIST_INIT();
	desc = comp_head;

	while (desc) {

		hal_tx_comp_get_status(&desc->comp, &ts);
		peer = dp_peer_find_by_id(soc, ts.peer_id);
		length = qdf_nbuf_len(desc->nbuf);
		/* Error Handling */
		if (hal_tx_comp_get_buffer_source(&desc->comp) ==
				HAL_TX_COMP_RELEASE_SOURCE_FW) {
			dp_tx_comp_process_exception(desc);
			desc = desc->next;
			continue;
		}

		/* Process Tx status in descriptor */
		if (soc->process_tx_status ||
				(desc->vdev && desc->vdev->mesh_vdev))
			dp_tx_comp_process_tx_status(desc, length);

		/* 0 : MSDU buffer, 1 : MLE */
		if (desc->msdu_ext_desc) {
			/* TSO free */
			if (hal_tx_ext_desc_get_tso_enable(
				desc->msdu_ext_desc->vaddr)) {
				/* If remaining number of segment is 0
				 * actual TSO may unmap and free */
				if (!DP_DESC_NUM_FRAG(desc)) {
					qdf_nbuf_unmap(soc->osdev, desc->nbuf,
							QDF_DMA_TO_DEVICE);
					qdf_nbuf_free(desc->nbuf);
				}
			} else {
				/* SG free */
				/* Free buffer */
				DP_TX_FREE_DMA_TO_DEVICE(soc, desc->vdev,
								desc->nbuf);
			}
		} else {
			/* Free buffer */
			DP_TX_FREE_DMA_TO_DEVICE(soc, desc->vdev, desc->nbuf);
		}

		DP_HIST_PACKET_COUNT_INC(desc->pdev->pdev_id);
		next = desc->next;

		if (desc->flags & DP_TX_DESC_FLAG_ME)
			dp_tx_me_free_buf(desc->pdev, desc->me_buffer);

		dp_tx_desc_release(desc, desc->pool_id);
		desc = next;
	}
	DP_TX_HIST_STATS_PER_PDEV();
}

/**
 * dp_tx_comp_handler() - Tx completion handler
 * @soc: core txrx main context
 * @ring_id: completion ring id
 * @budget: No. of packets/descriptors that can be serviced in one loop
 *
 * This function will collect hardware release ring element contents and
 * handle descriptor contents. Based on contents, free packet or handle error
 * conditions
 *
 * Return: none
 */
uint32_t dp_tx_comp_handler(struct dp_soc *soc, uint32_t ring_id,
			uint32_t budget)
{
	void *tx_comp_hal_desc;
	uint8_t buffer_src;
	uint8_t pool_id;
	uint32_t tx_desc_id;
	struct dp_tx_desc_s *tx_desc = NULL;
	struct dp_tx_desc_s *head_desc = NULL;
	struct dp_tx_desc_s *tail_desc = NULL;
	uint32_t num_processed;
	void *hal_srng = soc->tx_comp_ring[ring_id].hal_srng;

	if (qdf_unlikely(hal_srng_access_start(soc->hal_soc, hal_srng))) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s %d : HAL RING Access Failed -- %p\n",
				__func__, __LINE__, hal_srng);
		return 0;
	}

	num_processed = 0;

	/* Find head descriptor from completion ring */
	while (qdf_likely(tx_comp_hal_desc =
			hal_srng_dst_get_next(soc->hal_soc, hal_srng))) {

		buffer_src = hal_tx_comp_get_buffer_source(tx_comp_hal_desc);

		/* If this buffer was not released by TQM or FW, then it is not
		 * Tx completion indication, skip to next descriptor */
		if ((buffer_src != HAL_TX_COMP_RELEASE_SOURCE_TQM) &&
				(buffer_src != HAL_TX_COMP_RELEASE_SOURCE_FW)) {

			QDF_TRACE(QDF_MODULE_ID_DP,
					QDF_TRACE_LEVEL_ERROR,
					"Tx comp release_src != TQM | FW");

			/* TODO Handle Freeing of the buffer in descriptor */
			continue;
		}

		/* Get descriptor id */
		tx_desc_id = hal_tx_comp_get_desc_id(tx_comp_hal_desc);
		pool_id = (tx_desc_id & DP_TX_DESC_ID_POOL_MASK) >>
			DP_TX_DESC_ID_POOL_OS;

		/* Pool ID is out of limit. Error */
		if (pool_id > wlan_cfg_get_num_tx_desc_pool(
					soc->wlan_cfg_ctx)) {
			QDF_TRACE(QDF_MODULE_ID_DP,
					QDF_TRACE_LEVEL_FATAL,
					"TX COMP pool id %d not valid",
					pool_id);

			/* Check if assert aborts execution, if not handle
			 * return here */
			QDF_ASSERT(0);
		}

		/* Find Tx descriptor */
		tx_desc = dp_tx_desc_find(soc, pool_id,
				(tx_desc_id & DP_TX_DESC_ID_PAGE_MASK) >>
				DP_TX_DESC_ID_PAGE_OS,
				(tx_desc_id & DP_TX_DESC_ID_OFFSET_MASK) >>
				DP_TX_DESC_ID_OFFSET_OS);

		/* Pool id is not matching. Error */
		if (tx_desc && (tx_desc->pool_id != pool_id)) {
			QDF_TRACE(QDF_MODULE_ID_DP,
					QDF_TRACE_LEVEL_FATAL,
					"Tx Comp pool id %d not matched %d",
					pool_id, tx_desc->pool_id);

			/* Check if assert aborts execution, if not handle
			 * return here */
			QDF_ASSERT(0);
		}

		if (!(tx_desc->flags & DP_TX_DESC_FLAG_ALLOCATED) ||
				!(tx_desc->flags & DP_TX_DESC_FLAG_QUEUED_TX)) {
			QDF_TRACE(QDF_MODULE_ID_DP,
					QDF_TRACE_LEVEL_FATAL,
					"Txdesc invalid, flgs = %x,id = %d",
					tx_desc->flags,	tx_desc_id);

			/* TODO Handle Freeing of the buffer in this invalid
			 * descriptor */
			continue;
		}

		/*
		 * If the release source is FW, process the HTT
		 * status
		 */
		if (qdf_unlikely(buffer_src ==
					HAL_TX_COMP_RELEASE_SOURCE_FW)) {
			uint8_t htt_tx_status[HAL_TX_COMP_HTT_STATUS_LEN];
			hal_tx_comp_get_htt_desc(tx_comp_hal_desc,
					htt_tx_status);
			dp_tx_process_htt_completion(tx_desc,
					htt_tx_status);
		} else {
			tx_desc->next = NULL;

			/* First ring descriptor on the cycle */
			if (!head_desc) {
				head_desc = tx_desc;
			} else {
				tail_desc->next = tx_desc;
			}

			tail_desc = tx_desc;

			/* Collect hw completion contents */
			hal_tx_comp_desc_sync(tx_comp_hal_desc,
					&tx_desc->comp, soc->process_tx_status);

		}

		num_processed++;

		/*
		 * Processed packet count is more than given quota
		 * stop to processing
		 */
		if (num_processed >= budget)
			break;

	}

	hal_srng_access_end(soc->hal_soc, hal_srng);

	/* Process the reaped descriptors */
	if (head_desc)
		dp_tx_comp_process_desc(soc, head_desc);

	return num_processed;
}

/**
 * dp_tx_vdev_attach() - attach vdev to dp tx
 * @vdev: virtual device instance
 *
 * Return: QDF_STATUS_SUCCESS: success
 *         QDF_STATUS_E_RESOURCES: Error return
 */
QDF_STATUS dp_tx_vdev_attach(struct dp_vdev *vdev)
{
	/*
	 * Fill HTT TCL Metadata with Vdev ID and MAC ID
	 */
	HTT_TX_TCL_METADATA_TYPE_SET(vdev->htt_tcl_metadata,
			HTT_TCL_METADATA_TYPE_VDEV_BASED);

	HTT_TX_TCL_METADATA_VDEV_ID_SET(vdev->htt_tcl_metadata,
			vdev->vdev_id);

	HTT_TX_TCL_METADATA_PDEV_ID_SET(vdev->htt_tcl_metadata,
			DP_SW2HW_MACID(vdev->pdev->pdev_id));

	/*
	 * Set HTT Extension Valid bit to 0 by default
	 */
	HTT_TX_TCL_METADATA_VALID_HTT_SET(vdev->htt_tcl_metadata, 0);

	dp_tx_vdev_update_search_flags(vdev);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_tx_vdev_update_search_flags() - Update vdev flags as per opmode
 * @vdev: virtual device instance
 *
 * Return: void
 *
 */
void dp_tx_vdev_update_search_flags(struct dp_vdev *vdev)
{
	/*
	 * Enable both AddrY (SA based search) and AddrX (Da based search)
	 * for TDLS link
	 *
	 * Enable AddrY (SA based search) only for non-WDS STA and
	 * ProxySTA VAP modes.
	 *
	 * In all other VAP modes, only DA based search should be
	 * enabled
	 */
	if (vdev->opmode == wlan_op_mode_sta &&
	    vdev->tdls_link_connected)
		vdev->hal_desc_addr_search_flags =
			(HAL_TX_DESC_ADDRX_EN | HAL_TX_DESC_ADDRY_EN);
	else if ((vdev->opmode == wlan_op_mode_sta &&
				(!vdev->wds_enabled || vdev->proxysta_vdev)))
		vdev->hal_desc_addr_search_flags = HAL_TX_DESC_ADDRY_EN;
	else
		vdev->hal_desc_addr_search_flags = HAL_TX_DESC_ADDRX_EN;
}

/**
 * dp_tx_vdev_detach() - detach vdev from dp tx
 * @vdev: virtual device instance
 *
 * Return: QDF_STATUS_SUCCESS: success
 *         QDF_STATUS_E_RESOURCES: Error return
 */
QDF_STATUS dp_tx_vdev_detach(struct dp_vdev *vdev)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * dp_tx_pdev_attach() - attach pdev to dp tx
 * @pdev: physical device instance
 *
 * Return: QDF_STATUS_SUCCESS: success
 *         QDF_STATUS_E_RESOURCES: Error return
 */
QDF_STATUS dp_tx_pdev_attach(struct dp_pdev *pdev)
{
	struct dp_soc *soc = pdev->soc;

	/* Initialize Flow control counters */
	qdf_atomic_init(&pdev->num_tx_exception);
	qdf_atomic_init(&pdev->num_tx_outstanding);

	if (wlan_cfg_per_pdev_tx_ring(soc->wlan_cfg_ctx)) {
		/* Initialize descriptors in TCL Ring */
		hal_tx_init_data_ring(soc->hal_soc,
				soc->tcl_data_ring[pdev->pdev_id].hal_srng);
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_tx_pdev_detach() - detach pdev from dp tx
 * @pdev: physical device instance
 *
 * Return: QDF_STATUS_SUCCESS: success
 *         QDF_STATUS_E_RESOURCES: Error return
 */
QDF_STATUS dp_tx_pdev_detach(struct dp_pdev *pdev)
{
	/* What should do here? */
	return QDF_STATUS_SUCCESS;
}

/**
 * dp_tx_soc_detach() - detach soc from dp tx
 * @soc: core txrx main context
 *
 * This function will detach dp tx into main device context
 * will free dp tx resource and initialize resources
 *
 * Return: QDF_STATUS_SUCCESS: success
 *         QDF_STATUS_E_RESOURCES: Error return
 */
QDF_STATUS dp_tx_soc_detach(struct dp_soc *soc)
{
	uint8_t num_pool;
	uint16_t num_desc;
	uint16_t num_ext_desc;
	uint8_t i;

	num_pool = wlan_cfg_get_num_tx_desc_pool(soc->wlan_cfg_ctx);
	num_desc = wlan_cfg_get_num_tx_desc(soc->wlan_cfg_ctx);
	num_ext_desc = wlan_cfg_get_num_tx_ext_desc(soc->wlan_cfg_ctx);

	for (i = 0; i < num_pool; i++) {
		if (dp_tx_desc_pool_free(soc, i)) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
					"%s Tx Desc Pool Free failed\n",
					__func__);
			return QDF_STATUS_E_RESOURCES;
		}
	}

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			"%s Tx Desc Pool Free num_pool = %d, descs = %d\n",
			__func__, num_pool, num_desc);

	for (i = 0; i < num_pool; i++) {
		if (dp_tx_ext_desc_pool_free(soc, i)) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
					"%s Tx Ext Desc Pool Free failed\n",
					__func__);
			return QDF_STATUS_E_RESOURCES;
		}
	}

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			"%s MSDU Ext Desc Pool %d Free descs = %d\n",
			__func__, num_pool, num_ext_desc);

	for (i = 0; i < num_pool; i++) {
		dp_tx_tso_desc_pool_free(soc, i);
	}

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			"%s TSO Desc Pool %d Free descs = %d\n",
			__func__, num_pool, num_desc);


	for (i = 0; i < num_pool; i++)
		dp_tx_tso_num_seg_pool_free(soc, i);


	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
		"%s TSO Num of seg Desc Pool %d Free descs = %d\n",
		__func__, num_pool, num_desc);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_tx_soc_attach() - attach soc to dp tx
 * @soc: core txrx main context
 *
 * This function will attach dp tx into main device context
 * will allocate dp tx resource and initialize resources
 *
 * Return: QDF_STATUS_SUCCESS: success
 *         QDF_STATUS_E_RESOURCES: Error return
 */
QDF_STATUS dp_tx_soc_attach(struct dp_soc *soc)
{
	uint8_t num_pool;
	uint32_t num_desc;
	uint32_t num_ext_desc;
	uint8_t i;

	num_pool = wlan_cfg_get_num_tx_desc_pool(soc->wlan_cfg_ctx);
	num_desc = wlan_cfg_get_num_tx_desc(soc->wlan_cfg_ctx);
	num_ext_desc = wlan_cfg_get_num_tx_ext_desc(soc->wlan_cfg_ctx);

	/* Allocate software Tx descriptor pools */
	for (i = 0; i < num_pool; i++) {
		if (dp_tx_desc_pool_alloc(soc, i, num_desc)) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
					"%s Tx Desc Pool alloc %d failed %p\n",
					__func__, i, soc);
			goto fail;
		}
	}

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			"%s Tx Desc Alloc num_pool = %d, descs = %d\n",
			__func__, num_pool, num_desc);

	/* Allocate extension tx descriptor pools */
	for (i = 0; i < num_pool; i++) {
		if (dp_tx_ext_desc_pool_alloc(soc, i, num_ext_desc)) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"MSDU Ext Desc Pool alloc %d failed %p\n",
				i, soc);

			goto fail;
		}
	}

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			"%s MSDU Ext Desc Alloc %d, descs = %d\n",
			__func__, num_pool, num_ext_desc);

	for (i = 0; i < num_pool; i++) {
		if (dp_tx_tso_desc_pool_alloc(soc, i, num_desc)) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"TSO Desc Pool alloc %d failed %p\n",
				i, soc);

			goto fail;
		}
	}

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			"%s TSO Desc Alloc %d, descs = %d\n",
			__func__, num_pool, num_desc);

	for (i = 0; i < num_pool; i++) {
		if (dp_tx_tso_num_seg_pool_alloc(soc, i, num_desc)) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"TSO Num of seg Pool alloc %d failed %p\n",
				i, soc);

			goto fail;
		}
	}

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			"%s TSO Num of seg pool Alloc %d, descs = %d\n",
			__func__, num_pool, num_desc);

	/* Initialize descriptors in TCL Rings */
	if (!wlan_cfg_per_pdev_tx_ring(soc->wlan_cfg_ctx)) {
		for (i = 0; i < soc->num_tcl_data_rings; i++) {
			hal_tx_init_data_ring(soc->hal_soc,
					soc->tcl_data_ring[i].hal_srng);
		}
	}

	/*
	 * todo - Add a runtime config option to enable this.
	 */
	/*
	 * Due to multiple issues on NPR EMU, enable it selectively
	 * only for NPR EMU, should be removed, once NPR platforms
	 * are stable.
	 */
	soc->process_tx_status = 1;

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			"%s HAL Tx init Success\n", __func__);

	return QDF_STATUS_SUCCESS;

fail:
	/* Detach will take care of freeing only allocated resources */
	dp_tx_soc_detach(soc);
	return QDF_STATUS_E_RESOURCES;
}

/*
 * dp_tx_me_mem_free(): Function to free allocated memory in mcast enahncement
 * pdev: pointer to DP PDEV structure
 * seg_info_head: Pointer to the head of list
 *
 * return: void
 */
static inline void dp_tx_me_mem_free(struct dp_pdev *pdev,
		struct dp_tx_seg_info_s *seg_info_head)
{
	struct dp_tx_me_buf_t *mc_uc_buf;
	struct dp_tx_seg_info_s *seg_info_new = NULL;
	qdf_nbuf_t nbuf = NULL;
	uint64_t phy_addr;

	while (seg_info_head) {
		nbuf = seg_info_head->nbuf;
		mc_uc_buf = (struct dp_tx_me_buf_t *)
			seg_info_new->frags[0].vaddr;
		phy_addr = seg_info_head->frags[0].paddr_hi;
		phy_addr =  (phy_addr << 32) | seg_info_head->frags[0].paddr_lo;
		qdf_mem_unmap_nbytes_single(pdev->soc->osdev,
				phy_addr,
				QDF_DMA_TO_DEVICE , DP_MAC_ADDR_LEN);
		dp_tx_me_free_buf(pdev, mc_uc_buf);
		qdf_nbuf_free(nbuf);
		seg_info_new = seg_info_head;
		seg_info_head = seg_info_head->next;
		qdf_mem_free(seg_info_new);
	}
}

/**
 * dp_tx_me_send_convert_ucast(): fuction to convert multicast to unicast
 * @vdev: DP VDEV handle
 * @nbuf: Multicast nbuf
 * @newmac: Table of the clients to which packets have to be sent
 * @new_mac_cnt: No of clients
 *
 * return: no of converted packets
 */
uint16_t
dp_tx_me_send_convert_ucast(struct cdp_vdev *vdev_handle, qdf_nbuf_t nbuf,
		uint8_t newmac[][DP_MAC_ADDR_LEN], uint8_t new_mac_cnt)
{
	struct dp_vdev *vdev = (struct dp_vdev *) vdev_handle;
	struct dp_pdev *pdev = vdev->pdev;
	struct ether_header *eh;
	uint8_t *data;
	uint16_t len;

	/* reference to frame dst addr */
	uint8_t *dstmac;
	/* copy of original frame src addr */
	uint8_t srcmac[DP_MAC_ADDR_LEN];

	/* local index into newmac */
	uint8_t new_mac_idx = 0;
	struct dp_tx_me_buf_t *mc_uc_buf;
	qdf_nbuf_t  nbuf_clone;
	struct dp_tx_msdu_info_s msdu_info;
	struct dp_tx_seg_info_s *seg_info_head = NULL;
	struct dp_tx_seg_info_s *seg_info_tail = NULL;
	struct dp_tx_seg_info_s *seg_info_new;
	struct dp_tx_frag_info_s data_frag;
	qdf_dma_addr_t paddr_data;
	qdf_dma_addr_t paddr_mcbuf = 0;
	uint8_t empty_entry_mac[DP_MAC_ADDR_LEN] = {0};
	QDF_STATUS status;

	dp_tx_get_queue(vdev, nbuf, &msdu_info.tx_queue);

	eh = (struct ether_header *) nbuf;
	qdf_mem_copy(srcmac, eh->ether_shost, DP_MAC_ADDR_LEN);

	len = qdf_nbuf_len(nbuf);

	data = qdf_nbuf_data(nbuf);

	status = qdf_nbuf_map(vdev->osdev, nbuf,
			QDF_DMA_TO_DEVICE);

	if (status) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"Mapping failure Error:%d", status);
		DP_STATS_INC(vdev, tx_i.mcast_en.dropped_map_error, 1);
		return 0;
	}

	paddr_data = qdf_nbuf_get_frag_paddr(nbuf, 0) + IEEE80211_ADDR_LEN;

	/*preparing data fragment*/
	data_frag.vaddr = qdf_nbuf_data(nbuf) + IEEE80211_ADDR_LEN;
	data_frag.paddr_lo = (uint32_t)paddr_data;
	data_frag.paddr_hi = ((uint64_t)paddr_data & 0xffffffff00000000) >> 32;
	data_frag.len = len - DP_MAC_ADDR_LEN;

	for (new_mac_idx = 0; new_mac_idx < new_mac_cnt; new_mac_idx++) {
		dstmac = newmac[new_mac_idx];
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
				"added mac addr (%pM)", dstmac);

		/* Check for NULL Mac Address */
		if (!qdf_mem_cmp(dstmac, empty_entry_mac, DP_MAC_ADDR_LEN))
			continue;

		/* frame to self mac. skip */
		if (!qdf_mem_cmp(dstmac, srcmac, DP_MAC_ADDR_LEN))
			continue;

		/*
		 * TODO: optimize to avoid malloc in per-packet path
		 * For eg. seg_pool can be made part of vdev structure
		 */
		seg_info_new = qdf_mem_malloc(sizeof(*seg_info_new));

		if (!seg_info_new) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
					"alloc failed");
			DP_STATS_INC(vdev, tx_i.mcast_en.fail_seg_alloc, 1);
			goto fail_seg_alloc;
		}

		mc_uc_buf = dp_tx_me_alloc_buf(pdev);
		if (mc_uc_buf == NULL)
			goto fail_buf_alloc;

		/*
		 * TODO: Check if we need to clone the nbuf
		 * Or can we just use the reference for all cases
		 */
		if (new_mac_idx < (new_mac_cnt - 1)) {
			nbuf_clone = qdf_nbuf_clone((qdf_nbuf_t)nbuf);
			if (nbuf_clone == NULL) {
				DP_STATS_INC(vdev, tx_i.mcast_en.clone_fail, 1);
				goto fail_clone;
			}
		} else {
			/*
			 * Update the ref
			 * to account for frame sent without cloning
			 */
			qdf_nbuf_ref(nbuf);
			nbuf_clone = nbuf;
		}

		qdf_mem_copy(mc_uc_buf->data, dstmac, DP_MAC_ADDR_LEN);

		status = qdf_mem_map_nbytes_single(vdev->osdev, mc_uc_buf->data,
				QDF_DMA_TO_DEVICE, DP_MAC_ADDR_LEN,
				&paddr_mcbuf);

		if (status) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
					"Mapping failure Error:%d", status);
			DP_STATS_INC(vdev, tx_i.mcast_en.dropped_map_error, 1);
			goto fail_map;
		}

		seg_info_new->frags[0].vaddr =  (uint8_t *)mc_uc_buf;
		seg_info_new->frags[0].paddr_lo = (uint32_t) paddr_mcbuf;
		seg_info_new->frags[0].paddr_hi =
			((u64)paddr_mcbuf & 0xffffffff00000000) >> 32;
		seg_info_new->frags[0].len = DP_MAC_ADDR_LEN;

		seg_info_new->frags[1] = data_frag;
		seg_info_new->nbuf = nbuf_clone;
		seg_info_new->frag_cnt = 2;
		seg_info_new->total_len = len;

		seg_info_new->next = NULL;

		if (seg_info_head == NULL)
			seg_info_head = seg_info_new;
		else
			seg_info_tail->next = seg_info_new;

		seg_info_tail = seg_info_new;
	}

	if (!seg_info_head)
		return 0;

	msdu_info.u.sg_info.curr_seg = seg_info_head;
	msdu_info.num_seg = new_mac_cnt;
	msdu_info.frm_type = dp_tx_frm_me;

	DP_STATS_INC(vdev, tx_i.mcast_en.ucast, new_mac_cnt);
	dp_tx_send_msdu_multiple(vdev, nbuf, &msdu_info);

	while (seg_info_head->next) {
		seg_info_new = seg_info_head;
		seg_info_head = seg_info_head->next;
		qdf_mem_free(seg_info_new);
	}
	qdf_mem_free(seg_info_head);

	return new_mac_cnt;

fail_map:
	qdf_nbuf_free(nbuf_clone);

fail_clone:
	dp_tx_me_free_buf(pdev, mc_uc_buf);

fail_buf_alloc:
	qdf_mem_free(seg_info_new);

fail_seg_alloc:
	dp_tx_me_mem_free(pdev, seg_info_head);
	qdf_nbuf_unmap(pdev->soc->osdev, nbuf, QDF_DMA_TO_DEVICE);
	return 0;
}
