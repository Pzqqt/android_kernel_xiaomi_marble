/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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
#ifdef QCA_LL_TX_FLOW_CONTROL_V2
#define DP_TX_GET_DESC_POOL_ID(vdev) (vdev->vdev_id)
#else /* QCA_LL_TX_FLOW_CONTROL_V2 */
#define DP_TX_GET_DESC_POOL_ID(vdev) (vdev->pdev->pdev_id)
#endif /* QCA_LL_TX_FLOW_CONTROL_V2 */
	#define DP_TX_GET_RING_ID(vdev) (vdev->pdev->pdev_id)
#else
	#ifdef TX_PER_VDEV_DESC_POOL
		#define DP_TX_GET_DESC_POOL_ID(vdev) (vdev->vdev_id)
		#define DP_TX_GET_RING_ID(vdev) (vdev->pdev->pdev_id)
	#else
		#define DP_TX_GET_DESC_POOL_ID(vdev) qdf_get_cpu()
		#define DP_TX_GET_RING_ID(vdev) vdev->pdev->soc->tx_ring_map[qdf_get_cpu()]
	#endif /* TX_PER_VDEV_DESC_POOL */
#endif /* TX_PER_PDEV_DESC_POOL */

/* TODO Add support in TSO */
#define DP_DESC_NUM_FRAG(x) 0

/* disable TQM_BYPASS */
#define TQM_BYPASS_WAR 0

/* invalid peer id for reinject*/
#define DP_INVALID_PEER 0XFFFE

/*mapping between hal encrypt type and cdp_sec_type*/
#define MAX_CDP_SEC_TYPE 12
static const uint8_t sec_type_map[MAX_CDP_SEC_TYPE] = {
					HAL_TX_ENCRYPT_TYPE_NO_CIPHER,
					HAL_TX_ENCRYPT_TYPE_WEP_128,
					HAL_TX_ENCRYPT_TYPE_WEP_104,
					HAL_TX_ENCRYPT_TYPE_WEP_40,
					HAL_TX_ENCRYPT_TYPE_TKIP_WITH_MIC,
					HAL_TX_ENCRYPT_TYPE_TKIP_NO_MIC,
					HAL_TX_ENCRYPT_TYPE_AES_CCMP_128,
					HAL_TX_ENCRYPT_TYPE_WAPI,
					HAL_TX_ENCRYPT_TYPE_AES_CCMP_256,
					HAL_TX_ENCRYPT_TYPE_AES_GCMP_128,
					HAL_TX_ENCRYPT_TYPE_AES_GCMP_256,
					HAL_TX_ENCRYPT_TYPE_WAPI_GCM_SM4};

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
	/* get flow id */
	queue->desc_pool_id = DP_TX_GET_DESC_POOL_ID(vdev);
	queue->ring_id = DP_TX_GET_RING_ID(vdev);

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"%s, pool_id:%d ring_id: %d",
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

	if (tx_desc->flags & DP_TX_DESC_FLAG_ME)
		dp_tx_me_free_buf(tx_desc->pdev, tx_desc->me_buffer);

	qdf_atomic_dec(&pdev->num_tx_outstanding);

	if (tx_desc->flags & DP_TX_DESC_FLAG_TO_FW)
		qdf_atomic_dec(&pdev->num_tx_exception);

	if (HAL_TX_COMP_RELEASE_SOURCE_TQM ==
				hal_tx_comp_get_buffer_source(&tx_desc->comp))
		comp_status = hal_tx_comp_get_release_reason(&tx_desc->comp);
	else
		comp_status = HAL_TX_COMP_RELEASE_REASON_FW;

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
		"Tx Completion Release desc %d status %d outstanding %d",
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

	HTT_TX_TCL_METADATA_VALID_HTT_SET(vdev->htt_tcl_metadata, 1);

	/*
	 * Metadata - HTT MSDU Extension header
	 */
	htt_desc_size = sizeof(struct htt_tx_msdu_desc_ext2_t);
	htt_desc_size_aligned = (htt_desc_size + 7) & ~0x7;

	if (vdev->mesh_vdev) {

		/* Fill and add HTT metaheader */
		hdr = qdf_nbuf_push_head(nbuf, htt_desc_size_aligned);
		if (hdr == NULL) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
					"Error in filling HTT metadata\n");

			return 0;
		}
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

	if (msdu_info->exception_fw &&
			qdf_unlikely(vdev->mesh_vdev)) {
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

	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
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
 * @meta_data: Metadata to the fw
 * @tx_exc_metadata: Handle that holds exception path metadata
 * Allocate and prepare Tx descriptor with msdu information.
 *
 * Return: Pointer to Tx Descriptor on success,
 *         NULL on failure
 */
static
struct dp_tx_desc_s *dp_tx_prepare_desc_single(struct dp_vdev *vdev,
		qdf_nbuf_t nbuf, uint8_t desc_pool_id,
		struct dp_tx_msdu_info_s *msdu_info,
		struct cdp_tx_exception_metadata *tx_exc_metadata)
{
	uint8_t align_pad;
	uint8_t is_exception = 0;
	uint8_t htt_hdr_size;
	struct ether_header *eh;
	struct dp_tx_desc_s *tx_desc;
	struct dp_pdev *pdev = vdev->pdev;
	struct dp_soc *soc = pdev->soc;

	/* Allocate software Tx descriptor */
	tx_desc = dp_tx_desc_alloc(soc, desc_pool_id);
	if (qdf_unlikely(!tx_desc)) {
		DP_STATS_INC(vdev, tx_i.dropped.desc_na, 1);
		return NULL;
	}

	/* Flow control/Congestion Control counters */
	qdf_atomic_inc(&pdev->num_tx_outstanding);

	/* Initialize the SW tx descriptor */
	tx_desc->nbuf = nbuf;
	tx_desc->frm_type = dp_tx_frm_std;
	tx_desc->tx_encap_type = (tx_exc_metadata ?
			tx_exc_metadata->tx_encap_type : vdev->tx_encap_type);
	tx_desc->vdev = vdev;
	tx_desc->pdev = pdev;
	tx_desc->msdu_ext_desc = NULL;
	tx_desc->pkt_offset = 0;

	/*
	 * For special modes (vdev_type == ocb or mesh), data frames should be
	 * transmitted using varying transmit parameters (tx spec) which include
	 * transmit rate, power, priority, channel, channel bandwidth , nss etc.
	 * These are filled in HTT MSDU descriptor and sent in frame pre-header.
	 * These frames are sent as exception packets to firmware.
	 *
	 * HW requirement is that metadata should always point to a
	 * 8-byte aligned address. So we add alignment pad to start of buffer.
	 *  HTT Metadata should be ensured to be multiple of 8-bytes,
	 *  to get 8-byte aligned start address along with align_pad added
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
	if (qdf_unlikely((msdu_info->exception_fw)) ||
				(vdev->opmode == wlan_op_mode_ocb)) {
		align_pad = ((unsigned long) qdf_nbuf_data(nbuf)) & 0x7;
		if (qdf_nbuf_push_head(nbuf, align_pad) == NULL) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
					"qdf_nbuf_push_head failed\n");
			goto failure;
		}

		htt_hdr_size = dp_tx_prepare_htt_metadata(vdev, nbuf,
				msdu_info->meta_data);
		if (htt_hdr_size == 0)
			goto failure;
		tx_desc->pkt_offset = align_pad + htt_hdr_size;
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
	if (is_exception || tx_exc_metadata)
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
	struct dp_tx_ext_desc_elem_s *msdu_ext_desc;
	struct dp_pdev *pdev = vdev->pdev;
	struct dp_soc *soc = pdev->soc;

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
	if (qdf_unlikely(msdu_info->exception_fw))
		tx_desc->flags |= DP_TX_DESC_FLAG_TO_FW;

	tx_desc->msdu_ext_desc = msdu_ext_desc;
	tx_desc->flags |= DP_TX_DESC_FLAG_FRAG;

	return tx_desc;
failure:
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
	qdf_dma_addr_t paddr;
	int32_t i;
	int32_t mapped_buf_num = 0;

	struct dp_tx_sg_info_s *sg_info = &msdu_info->u.sg_info;
	qdf_dot3_qosframe_t *qos_wh = (qdf_dot3_qosframe_t *) nbuf->data;

	DP_STATS_INC_PKT(vdev, tx_i.raw.raw_pkt, 1, qdf_nbuf_len(nbuf));

	/* SWAR for HW: Enable WEP bit in the AMSDU frames for RAW mode */
	if (qos_wh->i_fc[0] & IEEE80211_FC0_SUBTYPE_QOS)
		qos_wh->i_fc[1] |= IEEE80211_FC1_WEP;

	for (curr_nbuf = nbuf, i = 0; curr_nbuf;
			curr_nbuf = qdf_nbuf_next(curr_nbuf), i++) {

		if (QDF_STATUS_SUCCESS != qdf_nbuf_map(vdev->osdev, curr_nbuf,
					QDF_DMA_TO_DEVICE)) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"%s dma map error \n", __func__);
			DP_STATS_INC(vdev, tx_i.raw.dma_map_error, 1);
			mapped_buf_num = i;
			goto error;
		}

		paddr = qdf_nbuf_get_frag_paddr(curr_nbuf, 0);
		seg_info->frags[i].paddr_lo = paddr;
		seg_info->frags[i].paddr_hi = ((uint64_t)paddr >> 32);
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

error:
	i = 0;
	while (nbuf) {
		curr_nbuf = nbuf;
		if (i < mapped_buf_num) {
			qdf_nbuf_unmap(vdev->osdev, curr_nbuf, QDF_DMA_TO_DEVICE);
			i++;
		}
		nbuf = qdf_nbuf_next(nbuf);
		qdf_nbuf_free(curr_nbuf);
	}
	return NULL;

}

/**
 * dp_tx_hw_enqueue() - Enqueue to TCL HW for transmit
 * @soc: DP Soc Handle
 * @vdev: DP vdev handle
 * @tx_desc: Tx Descriptor Handle
 * @tid: TID from HLOS for overriding default DSCP-TID mapping
 * @fw_metadata: Metadata to send to Target Firmware along with frame
 * @ring_id: Ring ID of H/W ring to which we enqueue the packet
 * @tx_exc_metadata: Handle that holds exception path meta data
 *
 *  Gets the next free TCL HW DMA descriptor and sets up required parameters
 *  from software Tx descriptor
 *
 * Return:
 */
static QDF_STATUS dp_tx_hw_enqueue(struct dp_soc *soc, struct dp_vdev *vdev,
				   struct dp_tx_desc_s *tx_desc, uint8_t tid,
				   uint16_t fw_metadata, uint8_t ring_id,
				   struct cdp_tx_exception_metadata
					*tx_exc_metadata)
{
	uint8_t type;
	uint16_t length;
	void *hal_tx_desc, *hal_tx_desc_cached;
	qdf_dma_addr_t dma_addr;
	uint8_t cached_desc[HAL_TX_DESC_LEN_BYTES];

	enum cdp_sec_type sec_type = (tx_exc_metadata ?
			tx_exc_metadata->sec_type : vdev->sec_type);

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
	hal_tx_desc_set_encrypt_type(hal_tx_desc_cached,
			sec_type_map[sec_type]);

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"%s length:%d , type = %d, dma_addr %llx, offset %d desc id %u",
			__func__, length, type, (uint64_t)dma_addr,
			tx_desc->pkt_offset, tx_desc->id);

	if (tx_desc->flags & DP_TX_DESC_FLAG_TO_FW)
		hal_tx_desc_set_to_fw(hal_tx_desc_cached, 1);

	hal_tx_desc_set_addr_search_flags(hal_tx_desc_cached,
			vdev->hal_desc_addr_search_flags);

	/* verify checksum offload configuration*/
	if ((wlan_cfg_get_checksum_offload(soc->wlan_cfg_ctx)) &&
		((qdf_nbuf_get_tx_cksum(tx_desc->nbuf) == QDF_NBUF_TX_CKSUM_TCP_UDP)
		|| qdf_nbuf_is_tso(tx_desc->nbuf)))  {
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
		return QDF_STATUS_E_RESOURCES;
	}

	tx_desc->flags |= DP_TX_DESC_FLAG_QUEUED_TX;

	hal_tx_desc_sync(hal_tx_desc_cached, hal_tx_desc);
	DP_STATS_INC_PKT(vdev, tx_i.processed, 1, length);

	/*
	 * If one packet is enqueued in HW, PM usage count needs to be
	 * incremented by one to prevent future runtime suspend. This
	 * should be tied with the success of enqueuing. It will be
	 * decremented after the packet has been sent.
	 */
	hif_pm_runtime_get_noresume(soc->hif_handle);

	return QDF_STATUS_SUCCESS;
}


/**
 * dp_cce_classify() - Classify the frame based on CCE rules
 * @vdev: DP vdev handle
 * @nbuf: skb
 *
 * Classify frames based on CCE rules
 * Return: bool( true if classified,
 *               else false)
 */
static bool dp_cce_classify(struct dp_vdev *vdev, qdf_nbuf_t nbuf)
{
	struct ether_header *eh = NULL;
	uint16_t   ether_type;
	qdf_llc_t *llcHdr;
	qdf_nbuf_t nbuf_clone = NULL;
	qdf_dot3_qosframe_t *qos_wh = NULL;

	/* for mesh packets don't do any classification */
	if (qdf_unlikely(vdev->mesh_vdev))
		return false;

	if (qdf_likely(vdev->tx_encap_type != htt_cmn_pkt_type_raw)) {
		eh = (struct ether_header *) qdf_nbuf_data(nbuf);
		ether_type = eh->ether_type;
		llcHdr = (qdf_llc_t *)(nbuf->data +
					sizeof(struct ether_header));
	} else {
		qos_wh = (qdf_dot3_qosframe_t *) nbuf->data;

		if (qdf_unlikely(qos_wh->i_fc[0] & QDF_IEEE80211_FC0_SUBTYPE_QOS)) {
			if (qdf_unlikely(
				qos_wh->i_fc[1] & QDF_IEEE80211_FC1_TODS &&
				qos_wh->i_fc[1] & QDF_IEEE80211_FC1_FROMDS)) {

				ether_type = *(uint16_t *)(nbuf->data
						+ QDF_IEEE80211_4ADDR_HDR_LEN
						+ sizeof(qdf_llc_t)
						- sizeof(ether_type));
				llcHdr = (qdf_llc_t *)(nbuf->data +
						QDF_IEEE80211_4ADDR_HDR_LEN);
			} else {
				ether_type = *(uint16_t *)(nbuf->data
						+ QDF_IEEE80211_3ADDR_HDR_LEN
						+ sizeof(qdf_llc_t)
						- sizeof(ether_type));
				llcHdr = (qdf_llc_t *)(nbuf->data +
					QDF_IEEE80211_3ADDR_HDR_LEN);
			}

			if (qdf_unlikely(DP_FRAME_IS_SNAP(llcHdr)
				&& (ether_type ==
				qdf_htons(QDF_NBUF_TRAC_EAPOL_ETH_TYPE)))) {

				DP_STATS_INC(vdev, tx_i.cce_classified_raw, 1);
				return true;
			}
		}

		return false;
	}

	if (qdf_unlikely(DP_FRAME_IS_SNAP(llcHdr))) {
		ether_type = *(uint16_t *)(nbuf->data + 2*ETHER_ADDR_LEN +
				sizeof(*llcHdr));
		nbuf_clone = qdf_nbuf_clone(nbuf);
		qdf_nbuf_pull_head(nbuf_clone, sizeof(*llcHdr));

		if (ether_type == htons(ETHERTYPE_8021Q)) {
			qdf_nbuf_pull_head(nbuf_clone,
						sizeof(qdf_net_vlanhdr_t));
		}
	} else {
		if (ether_type == htons(ETHERTYPE_8021Q)) {
			nbuf_clone = qdf_nbuf_clone(nbuf);
			qdf_nbuf_pull_head(nbuf_clone,
					sizeof(qdf_net_vlanhdr_t));
		}
	}

	if (qdf_unlikely(nbuf_clone))
		nbuf = nbuf_clone;


	if (qdf_unlikely(qdf_nbuf_is_ipv4_eapol_pkt(nbuf)
		|| qdf_nbuf_is_ipv4_arp_pkt(nbuf)
		|| qdf_nbuf_is_ipv4_wapi_pkt(nbuf)
		|| qdf_nbuf_is_ipv4_tdls_pkt(nbuf)
		|| (qdf_nbuf_is_ipv4_pkt(nbuf)
			&& qdf_nbuf_is_ipv4_dhcp_pkt(nbuf))
		|| (qdf_nbuf_is_ipv6_pkt(nbuf) &&
			qdf_nbuf_is_ipv6_dhcp_pkt(nbuf)))) {
		if (qdf_unlikely(nbuf_clone != NULL))
			qdf_nbuf_free(nbuf_clone);
		return true;
	}

	if (qdf_unlikely(nbuf_clone != NULL))
		qdf_nbuf_free(nbuf_clone);

	return false;
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

#ifdef CONVERGED_TDLS_ENABLE
/**
 * dp_tx_update_tdls_flags() - Update descriptor flags for TDLS frame
 * @tx_desc: TX descriptor
 *
 * Return: None
 */
static void dp_tx_update_tdls_flags(struct dp_tx_desc_s *tx_desc)
{
	if (tx_desc->vdev) {
		if (tx_desc->vdev->is_tdls_frame)
			tx_desc->flags |= DP_TX_DESC_FLAG_TDLS_FRAME;
			tx_desc->vdev->is_tdls_frame = false;
	}
}

/**
 * dp_non_std_tx_comp_free_buff() - Free the non std tx packet buffer
 * @tx_desc: TX descriptor
 * @vdev: datapath vdev handle
 *
 * Return: None
 */
static void dp_non_std_tx_comp_free_buff(struct dp_tx_desc_s *tx_desc,
				  struct dp_vdev *vdev)
{
	struct hal_tx_completion_status ts = {0};
	qdf_nbuf_t nbuf = tx_desc->nbuf;

	hal_tx_comp_get_status(&tx_desc->comp, &ts);
	if (vdev->tx_non_std_data_callback.func) {
		qdf_nbuf_set_next(tx_desc->nbuf, NULL);
		vdev->tx_non_std_data_callback.func(
				vdev->tx_non_std_data_callback.ctxt,
				nbuf, ts.status);
		return;
	}
}
#endif

/**
 * dp_tx_send_msdu_single() - Setup descriptor and enqueue single MSDU to TCL
 * @vdev: DP vdev handle
 * @nbuf: skb
 * @tid: TID from HLOS for overriding default DSCP-TID mapping
 * @meta_data: Metadata to the fw
 * @tx_q: Tx queue to be used for this Tx frame
 * @peer_id: peer_id of the peer in case of NAWDS frames
 * @tx_exc_metadata: Handle that holds exception path metadata
 *
 * Return: NULL on success,
 *         nbuf when it fails to send
 */
static qdf_nbuf_t dp_tx_send_msdu_single(struct dp_vdev *vdev, qdf_nbuf_t nbuf,
		struct dp_tx_msdu_info_s *msdu_info, uint16_t peer_id,
		struct cdp_tx_exception_metadata *tx_exc_metadata)
{
	struct dp_pdev *pdev = vdev->pdev;
	struct dp_soc *soc = pdev->soc;
	struct dp_tx_desc_s *tx_desc;
	QDF_STATUS status;
	struct dp_tx_queue *tx_q = &(msdu_info->tx_queue);
	void *hal_srng = soc->tcl_data_ring[tx_q->ring_id].hal_srng;
	uint16_t htt_tcl_metadata = 0;
	uint8_t tid = msdu_info->tid;

	HTT_TX_TCL_METADATA_VALID_HTT_SET(htt_tcl_metadata, 0);
	/* Setup Tx descriptor for an MSDU, and MSDU extension descriptor */
	tx_desc = dp_tx_prepare_desc_single(vdev, nbuf, tx_q->desc_pool_id,
			msdu_info, tx_exc_metadata);
	if (!tx_desc) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "%s Tx_desc prepare Fail vdev %pK queue %d\n",
			  __func__, vdev, tx_q->desc_pool_id);
		return nbuf;
	}

	if (qdf_unlikely(soc->cce_disable)) {
		if (dp_cce_classify(vdev, nbuf) == true) {
			DP_STATS_INC(vdev, tx_i.cce_classified, 1);
			tid = DP_VO_TID;
			tx_desc->flags |= DP_TX_DESC_FLAG_TO_FW;
		}
	}

	dp_tx_update_tdls_flags(tx_desc);

	if (qdf_unlikely(hal_srng_access_start(soc->hal_soc, hal_srng))) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s %d : HAL RING Access Failed -- %pK\n",
				__func__, __LINE__, hal_srng);
		DP_STATS_INC(vdev, tx_i.dropped.ring_full, 1);
		dp_tx_desc_release(tx_desc, tx_q->desc_pool_id);
		goto fail_return;
	}

	if (qdf_unlikely(peer_id == DP_INVALID_PEER)) {
		htt_tcl_metadata = vdev->htt_tcl_metadata;
		HTT_TX_TCL_METADATA_HOST_INSPECTED_SET(htt_tcl_metadata, 1);
	} else if (qdf_unlikely(peer_id != HTT_INVALID_PEER)) {
		HTT_TX_TCL_METADATA_TYPE_SET(htt_tcl_metadata,
				HTT_TCL_METADATA_TYPE_PEER_BASED);
		HTT_TX_TCL_METADATA_PEER_ID_SET(htt_tcl_metadata,
				peer_id);
	} else
		htt_tcl_metadata = vdev->htt_tcl_metadata;

	/* Enqueue the Tx MSDU descriptor to HW for transmit */
	status = dp_tx_hw_enqueue(soc, vdev, tx_desc, tid,
			htt_tcl_metadata, tx_q->ring_id, tx_exc_metadata);

	if (status != QDF_STATUS_SUCCESS) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "%s Tx_hw_enqueue Fail tx_desc %pK queue %d\n",
			  __func__, tx_desc, tx_q->ring_id);
		dp_tx_desc_release(tx_desc, tx_q->desc_pool_id);
		goto fail_return;
	}

	nbuf = NULL;

fail_return:
	if (hif_pm_runtime_get(soc->hif_handle) == 0) {
		hal_srng_access_end(soc->hal_soc, hal_srng);
		hif_pm_runtime_put(soc->hif_handle);
	} else {
		hal_srng_access_end_reap(soc->hal_soc, hal_srng);
	}

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
	bool is_cce_classified = false;
	QDF_STATUS status;

	struct dp_tx_queue *tx_q = &msdu_info->tx_queue;
	void *hal_srng = soc->tcl_data_ring[tx_q->ring_id].hal_srng;

	if (qdf_unlikely(hal_srng_access_start(soc->hal_soc, hal_srng))) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s %d : HAL RING Access Failed -- %pK\n",
				__func__, __LINE__, hal_srng);
		DP_STATS_INC(vdev, tx_i.dropped.ring_full, 1);
		return nbuf;
	}

	if (qdf_unlikely(soc->cce_disable)) {
		is_cce_classified = dp_cce_classify(vdev, nbuf);
		if (is_cce_classified) {
			DP_STATS_INC(vdev, tx_i.cce_classified, 1);
			msdu_info->tid = DP_VO_TID;
		}
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

		if (!tx_desc) {
			if (msdu_info->frm_type == dp_tx_frm_me) {
				dp_tx_me_free_buf(pdev,
					(void *)(msdu_info->u.sg_info
						.curr_seg->frags[0].vaddr));
			}
			goto done;
		}

		if (msdu_info->frm_type == dp_tx_frm_me) {
			tx_desc->me_buffer =
				msdu_info->u.sg_info.curr_seg->frags[0].vaddr;
			tx_desc->flags |= DP_TX_DESC_FLAG_ME;
		}

		if (is_cce_classified)
			tx_desc->flags |= DP_TX_DESC_FLAG_TO_FW;

		/*
		 * Enqueue the Tx MSDU descriptor to HW for transmit
		 */
		status = dp_tx_hw_enqueue(soc, vdev, tx_desc, msdu_info->tid,
			vdev->htt_tcl_metadata, tx_q->ring_id, NULL);

		if (status != QDF_STATUS_SUCCESS) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				  "%s Tx_hw_enqueue Fail tx_desc %pK queue %d\n",
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
	if (hif_pm_runtime_get(soc->hif_handle) == 0) {
		hal_srng_access_end(soc->hal_soc, hal_srng);
		hif_pm_runtime_put(soc->hif_handle);
	} else {
		hal_srng_access_end_reap(soc->hal_soc, hal_srng);
	}

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

	paddr = qdf_nbuf_get_frag_paddr(nbuf, 0);
	seg_info->frags[0].paddr_lo = paddr;
	seg_info->frags[0].paddr_hi = ((uint64_t) paddr) >> 32;
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
 * Return: NULL on failure,
 *         nbuf when extracted successfully
 */
static
qdf_nbuf_t dp_tx_extract_mesh_meta_data(struct dp_vdev *vdev, qdf_nbuf_t nbuf,
				struct dp_tx_msdu_info_s *msdu_info)
{
	struct meta_hdr_s *mhdr;
	struct htt_tx_msdu_desc_ext2_t *meta_data =
				(struct htt_tx_msdu_desc_ext2_t *)&msdu_info->meta_data[0];

	mhdr = (struct meta_hdr_s *)qdf_nbuf_data(nbuf);

	if (CB_FTYPE_MESH_TX_INFO != qdf_nbuf_get_tx_ftype(nbuf)) {
		msdu_info->exception_fw = 0;
		goto remove_meta_hdr;
	}

	msdu_info->exception_fw = 1;

	qdf_mem_set(meta_data, sizeof(struct htt_tx_msdu_desc_ext2_t), 0);

	meta_data->host_tx_desc_pool = 1;
	meta_data->update_peer_cache = 1;
	meta_data->learning_frame = 1;

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
		meta_data->learning_frame = 0;
	}

	meta_data->valid_key_flags = 1;
	meta_data->key_flags = (mhdr->keyix & 0x3);

remove_meta_hdr:
	if (qdf_nbuf_pull_head(nbuf, sizeof(struct meta_hdr_s)) == NULL) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"qdf_nbuf_pull_head failed\n");
		qdf_nbuf_free(nbuf);
		return NULL;
	}

	if (mhdr->flags & METAHDR_FLAG_NOQOS)
		msdu_info->tid = HTT_TX_EXT_TID_NON_QOS_MCAST_BCAST;
	else
		msdu_info->tid = qdf_nbuf_get_priority(nbuf);

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_HIGH,
			"%s , Meta hdr %0x %0x %0x %0x %0x %0x"
			" tid %d to_fw %d\n",
			__func__, msdu_info->meta_data[0],
			msdu_info->meta_data[1],
			msdu_info->meta_data[2],
			msdu_info->meta_data[3],
			msdu_info->meta_data[4],
			msdu_info->meta_data[5],
			msdu_info->tid, msdu_info->exception_fw);

	return nbuf;
}
#else
static
qdf_nbuf_t dp_tx_extract_mesh_meta_data(struct dp_vdev *vdev, qdf_nbuf_t nbuf,
				struct dp_tx_msdu_info_s *msdu_info)
{
	return nbuf;
}

#endif

#ifdef DP_FEATURE_NAWDS_TX
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
		struct dp_tx_msdu_info_s *msdu_info)
{
	struct dp_peer *peer = NULL;
	struct dp_soc *soc = vdev->pdev->soc;
	struct dp_ast_entry *ast_entry = NULL;
	struct ether_header *eh = (struct ether_header *)qdf_nbuf_data(nbuf);
	uint16_t peer_id = HTT_INVALID_PEER;

	struct dp_peer *sa_peer = NULL;
	qdf_nbuf_t nbuf_copy;

	qdf_spin_lock_bh(&(soc->ast_lock));
	ast_entry = dp_peer_ast_hash_find(soc, (uint8_t *)(eh->ether_shost));

	if (ast_entry)
		sa_peer = ast_entry->peer;

	qdf_spin_unlock_bh(&(soc->ast_lock));

	TAILQ_FOREACH(peer, &vdev->peer_list, peer_list_elem) {
		if ((peer->peer_ids[0] != HTT_INVALID_PEER) &&
				(peer->nawds_enabled)) {
			if (sa_peer == peer) {
				QDF_TRACE(QDF_MODULE_ID_DP,
						QDF_TRACE_LEVEL_DEBUG,
						" %s: broadcast multicast packet",
						 __func__);
				DP_STATS_INC(peer, tx.nawds_mcast_drop, 1);
				continue;
			}

			nbuf_copy = qdf_nbuf_copy(nbuf);
			if (!nbuf_copy) {
				QDF_TRACE(QDF_MODULE_ID_DP,
						QDF_TRACE_LEVEL_ERROR,
						"nbuf copy failed");
			}

			peer_id = peer->peer_ids[0];
			nbuf_copy = dp_tx_send_msdu_single(vdev, nbuf_copy,
					msdu_info, peer_id, NULL);
			if (nbuf_copy != NULL) {
				qdf_nbuf_free(nbuf_copy);
				continue;
			}
			DP_STATS_INC_PKT(peer, tx.nawds_mcast,
						1, qdf_nbuf_len(nbuf));
		}
	}
	if (peer_id == HTT_INVALID_PEER)
		return nbuf;

	return NULL;
}
#endif

/**
 * dp_check_exc_metadata() - Checks if parameters are valid
 * @tx_exc - holds all exception path parameters
 *
 * Returns true when all the parameters are valid else false
 *
 */
static bool dp_check_exc_metadata(struct cdp_tx_exception_metadata *tx_exc)
{
	if ((tx_exc->tid > DP_MAX_TIDS && tx_exc->tid != HTT_INVALID_TID) ||
	    tx_exc->tx_encap_type > htt_cmn_pkt_num_types ||
	    tx_exc->sec_type > cdp_num_sec_types) {
		return false;
	}

	return true;
}

/**
 * dp_tx_send_exception() - Transmit a frame on a given VAP in exception path
 * @vap_dev: DP vdev handle
 * @nbuf: skb
 * @tx_exc_metadata: Handle that holds exception path meta data
 *
 * Entry point for Core Tx layer (DP_TX) invoked from
 * hard_start_xmit in OSIF/HDD to transmit frames through fw
 *
 * Return: NULL on success,
 *         nbuf when it fails to send
 */
qdf_nbuf_t dp_tx_send_exception(void *vap_dev, qdf_nbuf_t nbuf,
		struct cdp_tx_exception_metadata *tx_exc_metadata)
{
	struct ether_header *eh = NULL;
	struct dp_vdev *vdev = (struct dp_vdev *) vap_dev;
	struct dp_tx_msdu_info_s msdu_info;

	qdf_mem_set(&msdu_info, sizeof(msdu_info), 0x0);

	msdu_info.tid = tx_exc_metadata->tid;

	eh = (struct ether_header *)qdf_nbuf_data(nbuf);
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"%s , skb %pM",
			__func__, nbuf->data);

	DP_STATS_INC_PKT(vdev, tx_i.rcvd, 1, qdf_nbuf_len(nbuf));

	if (qdf_unlikely(!dp_check_exc_metadata(tx_exc_metadata))) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"Invalid parameters in exception path");
		goto fail;
	}

	/* Basic sanity checks for unsupported packets */

	/* MESH mode */
	if (qdf_unlikely(vdev->mesh_vdev)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"Mesh mode is not supported in exception path");
		goto fail;
	}

	/* TSO or SG */
	if (qdf_unlikely(qdf_nbuf_is_tso(nbuf)) ||
	    qdf_unlikely(qdf_nbuf_is_nonlinear(nbuf))) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "TSO and SG are not supported in exception path");

		goto fail;
	}

	/* RAW */
	if (qdf_unlikely(tx_exc_metadata->tx_encap_type == htt_cmn_pkt_type_raw)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "Raw frame is not supported in exception path");
		goto fail;
	}


	/* Mcast enhancement*/
	if (qdf_unlikely(vdev->mcast_enhancement_en > 0)) {
		if (DP_FRAME_IS_MULTICAST((eh)->ether_dhost)) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
					  "Ignoring mcast_enhancement_en which is set and sending the mcast packet to the FW\n");
		}
	}

	/*
	 * Get HW Queue to use for this frame.
	 * TCL supports upto 4 DMA rings, out of which 3 rings are
	 * dedicated for data and 1 for command.
	 * "queue_id" maps to one hardware ring.
	 *  With each ring, we also associate a unique Tx descriptor pool
	 *  to minimize lock contention for these resources.
	 */
	dp_tx_get_queue(vdev, nbuf, &msdu_info.tx_queue);

	/* Reset the control block */
	qdf_nbuf_reset_ctxt(nbuf);

	/*  Single linear frame */
	/*
	 * If nbuf is a simple linear frame, use send_single function to
	 * prepare direct-buffer type TCL descriptor and enqueue to TCL
	 * SRNG. There is no need to setup a MSDU extension descriptor.
	 */
	nbuf = dp_tx_send_msdu_single(vdev, nbuf, &msdu_info,
			tx_exc_metadata->peer_id, tx_exc_metadata);

	return nbuf;

fail:
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"pkt send failed");
	return nbuf;
}

/**
 * dp_tx_send_mesh() - Transmit mesh frame on a given VAP
 * @vap_dev: DP vdev handle
 * @nbuf: skb
 *
 * Entry point for Core Tx layer (DP_TX) invoked from
 * hard_start_xmit in OSIF/HDD
 *
 * Return: NULL on success,
 *         nbuf when it fails to send
 */
#ifdef MESH_MODE_SUPPORT
qdf_nbuf_t dp_tx_send_mesh(void *vap_dev, qdf_nbuf_t nbuf)
{
	struct meta_hdr_s *mhdr;
	qdf_nbuf_t nbuf_mesh = NULL;
	qdf_nbuf_t nbuf_clone = NULL;
	struct dp_vdev *vdev = (struct dp_vdev *) vap_dev;
	uint8_t no_enc_frame = 0;

	nbuf_mesh = qdf_nbuf_unshare(nbuf);
	if (nbuf_mesh == NULL) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"qdf_nbuf_unshare failed\n");
		return nbuf;
	}
	nbuf = nbuf_mesh;

	mhdr = (struct meta_hdr_s *)qdf_nbuf_data(nbuf);

	if ((vdev->sec_type != cdp_sec_type_none) &&
			(mhdr->flags & METAHDR_FLAG_NOENCRYPT))
		no_enc_frame = 1;

	if ((mhdr->flags & METAHDR_FLAG_INFO_UPDATED) &&
		       !no_enc_frame) {
		nbuf_clone = qdf_nbuf_clone(nbuf);
		if (nbuf_clone == NULL) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"qdf_nbuf_clone failed\n");
			return nbuf;
		}
		qdf_nbuf_set_tx_ftype(nbuf_clone, CB_FTYPE_MESH_TX_INFO);
	}

	if (nbuf_clone) {
		if (!dp_tx_send(vap_dev, nbuf_clone)) {
			DP_STATS_INC(vdev, tx_i.mesh.exception_fw, 1);
		} else
			qdf_nbuf_free(nbuf_clone);
	}

	if (no_enc_frame)
		qdf_nbuf_set_tx_ftype(nbuf, CB_FTYPE_MESH_TX_INFO);
	else
		qdf_nbuf_set_tx_ftype(nbuf, CB_FTYPE_INVALID);

	nbuf = dp_tx_send(vap_dev, nbuf);
	if ((nbuf == NULL) && no_enc_frame) {
		DP_STATS_INC(vdev, tx_i.mesh.exception_fw, 1);
	}

	return nbuf;
}

#else

qdf_nbuf_t dp_tx_send_mesh(void *vap_dev, qdf_nbuf_t nbuf)
{
	return dp_tx_send(vap_dev, nbuf);
}

#endif

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
	qdf_nbuf_t nbuf_mesh = NULL;

	qdf_mem_set(&msdu_info, sizeof(msdu_info), 0x0);
	qdf_mem_set(&seg_info, sizeof(seg_info), 0x0);

	eh = (struct ether_header *)qdf_nbuf_data(nbuf);

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"%s , skb %pM",
			__func__, nbuf->data);

	/*
	 * Set Default Host TID value to invalid TID
	 * (TID override disabled)
	 */
	msdu_info.tid = HTT_TX_EXT_TID_INVALID;
	DP_STATS_INC_PKT(vdev, tx_i.rcvd, 1, qdf_nbuf_len(nbuf));

	if (qdf_unlikely(vdev->mesh_vdev)) {
		nbuf_mesh = dp_tx_extract_mesh_meta_data(vdev, nbuf,
								&msdu_info);
		if (nbuf_mesh == NULL) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
					"Extracting mesh metadata failed\n");
			return nbuf;
		}
		nbuf = nbuf_mesh;
	}

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
			  "%s TSO frame %pK\n", __func__, vdev);
		DP_STATS_INC_PKT(vdev, tx_i.tso.tso_pkt, 1,
				qdf_nbuf_len(nbuf));

		if (dp_tx_prepare_tso(vdev, nbuf, &msdu_info)) {
			DP_STATS_INC(vdev, tx_i.tso.dropped_host, 1);
			return nbuf;
		}

		goto send_multiple;
	}

	/* SG */
	if (qdf_unlikely(qdf_nbuf_is_nonlinear(nbuf))) {
		nbuf = dp_tx_prepare_sg(vdev, nbuf, &seg_info, &msdu_info);

		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			 "%s non-TSO SG frame %pK\n", __func__, vdev);

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
				  "%s Mcast frm for ME %pK\n", __func__, vdev);

			DP_STATS_INC_PKT(vdev,
					tx_i.mcast_en.mcast_pkt, 1,
					qdf_nbuf_len(nbuf));
			if (dp_tx_prepare_send_me(vdev, nbuf) ==
					QDF_STATUS_SUCCESS) {
				return NULL;
			}
		}
	}
#endif

	/* RAW */
	if (qdf_unlikely(vdev->tx_encap_type == htt_cmn_pkt_type_raw)) {
		nbuf = dp_tx_prepare_raw(vdev, nbuf, &seg_info, &msdu_info);
		if (nbuf == NULL)
			return NULL;

		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "%s Raw frame %pK\n", __func__, vdev);

		goto send_multiple;

	}

	/*  Single linear frame */
	/*
	 * If nbuf is a simple linear frame, use send_single function to
	 * prepare direct-buffer type TCL descriptor and enqueue to TCL
	 * SRNG. There is no need to setup a MSDU extension descriptor.
	 */
	nbuf = dp_tx_send_msdu_single(vdev, nbuf, &msdu_info, peer_id, NULL);

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
	struct dp_peer *sa_peer = NULL;
	struct dp_ast_entry *ast_entry = NULL;
	struct dp_soc *soc = NULL;
	struct ether_header *eh = (struct ether_header *)qdf_nbuf_data(nbuf);
#ifdef WDS_VENDOR_EXTENSION
	int is_mcast = 0, is_ucast = 0;
	int num_peers_3addr = 0;
	struct ether_header *eth_hdr = (struct ether_header *)(qdf_nbuf_data(nbuf));
	struct ieee80211_frame_addr4 *wh = (struct ieee80211_frame_addr4 *)(qdf_nbuf_data(nbuf));
#endif

	vdev = tx_desc->vdev;
	soc = vdev->pdev->soc;

	qdf_assert(vdev);

	qdf_mem_set(&msdu_info, sizeof(msdu_info), 0x0);

	dp_tx_get_queue(vdev, nbuf, &msdu_info.tx_queue);

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"%s Tx reinject path\n", __func__);

	DP_STATS_INC_PKT(vdev, tx_i.reinject_pkts, 1,
			qdf_nbuf_len(tx_desc->nbuf));

	qdf_spin_lock_bh(&(soc->ast_lock));

	ast_entry = dp_peer_ast_hash_find(soc, (uint8_t *)(eh->ether_shost));

	if (ast_entry)
		sa_peer = ast_entry->peer;

	qdf_spin_unlock_bh(&(soc->ast_lock));

#ifdef WDS_VENDOR_EXTENSION
	if (qdf_unlikely(vdev->tx_encap_type != htt_cmn_pkt_type_raw)) {
		is_mcast = (IS_MULTICAST(wh->i_addr1)) ? 1 : 0;
	} else {
		is_mcast = (IS_MULTICAST(eth_hdr->ether_dhost)) ? 1 : 0;
	}
	is_ucast = !is_mcast;

	TAILQ_FOREACH(peer, &vdev->peer_list, peer_list_elem) {
		if (peer->bss_peer)
			continue;

		/* Detect wds peers that use 3-addr framing for mcast.
		 * if there are any, the bss_peer is used to send the
		 * the mcast frame using 3-addr format. all wds enabled
		 * peers that use 4-addr framing for mcast frames will
		 * be duplicated and sent as 4-addr frames below.
		 */
		if (!peer->wds_enabled || !peer->wds_ecm.wds_tx_mcast_4addr) {
			num_peers_3addr = 1;
			break;
		}
	}
#endif

	if (qdf_unlikely(vdev->mesh_vdev)) {
		DP_TX_FREE_SINGLE_BUF(vdev->pdev->soc, tx_desc->nbuf);
	} else {
		TAILQ_FOREACH(peer, &vdev->peer_list, peer_list_elem) {
			if ((peer->peer_ids[0] != HTT_INVALID_PEER) &&
#ifdef WDS_VENDOR_EXTENSION
			/*
			 * . if 3-addr STA, then send on BSS Peer
			 * . if Peer WDS enabled and accept 4-addr mcast,
			 * send mcast on that peer only
			 * . if Peer WDS enabled and accept 4-addr ucast,
			 * send ucast on that peer only
			 */
			((peer->bss_peer && num_peers_3addr && is_mcast) ||
			 (peer->wds_enabled &&
				  ((is_mcast && peer->wds_ecm.wds_tx_mcast_4addr) ||
				   (is_ucast && peer->wds_ecm.wds_tx_ucast_4addr))))) {
#else
			((peer->bss_peer &&
			  !(vdev->osif_proxy_arp(vdev->osif_vdev, nbuf))) ||
				 peer->nawds_enabled)) {
#endif
				peer_id = DP_INVALID_PEER;

				if (peer->nawds_enabled) {
					peer_id = peer->peer_ids[0];
					if (sa_peer == peer) {
						QDF_TRACE(
							QDF_MODULE_ID_DP,
							QDF_TRACE_LEVEL_DEBUG,
							" %s: multicast packet",
							__func__);
						DP_STATS_INC(peer,
							tx.nawds_mcast_drop, 1);
						continue;
					}
				}

				nbuf_copy = qdf_nbuf_copy(nbuf);

				if (!nbuf_copy) {
					QDF_TRACE(QDF_MODULE_ID_DP,
						QDF_TRACE_LEVEL_DEBUG,
						FL("nbuf copy failed"));
					break;
				}

				nbuf_copy = dp_tx_send_msdu_single(vdev,
						nbuf_copy,
						&msdu_info,
						peer_id,
						NULL);

				if (nbuf_copy) {
					QDF_TRACE(QDF_MODULE_ID_DP,
						QDF_TRACE_LEVEL_DEBUG,
						FL("pkt send failed"));
					qdf_nbuf_free(nbuf_copy);
				} else {
					if (peer_id != DP_INVALID_PEER)
						DP_STATS_INC_PKT(peer,
							tx.nawds_mcast,
							1, qdf_nbuf_len(nbuf));
				}
			}
		}
	}

	if (vdev->nawds_enabled) {
		peer_id = DP_INVALID_PEER;

		DP_STATS_INC_PKT(vdev, tx_i.nawds_mcast,
					1, qdf_nbuf_len(nbuf));

		nbuf = dp_tx_send_msdu_single(vdev,
				nbuf,
				&msdu_info,
				peer_id, NULL);

		if (nbuf) {
			QDF_TRACE(QDF_MODULE_ID_DP,
				QDF_TRACE_LEVEL_DEBUG,
				FL("pkt send failed"));
			qdf_nbuf_free(nbuf);
		}
	} else
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
	dp_tx_desc_release(tx_desc, tx_desc->pool_id);
}

#ifdef FEATURE_PERPKT_INFO
/**
 * dp_get_completion_indication_for_stack() - send completion to stack
 * @soc :  dp_soc handle
 * @pdev:  dp_pdev handle
 * @peer_id: peer_id of the peer for which completion came
 * @ppdu_id: ppdu_id
 * @first_msdu: first msdu
 * @last_msdu: last msdu
 * @netbuf: Buffer pointer for free
 *
 * This function is used for indication whether buffer needs to be
 * send to stack for free or not
*/
QDF_STATUS
dp_get_completion_indication_for_stack(struct dp_soc *soc,  struct dp_pdev *pdev,
		      uint16_t peer_id, uint32_t ppdu_id, uint8_t first_msdu,
		      uint8_t last_msdu, qdf_nbuf_t netbuf)
{
	struct tx_capture_hdr *ppdu_hdr;
	struct dp_peer *peer = NULL;

	if (qdf_unlikely(!pdev->tx_sniffer_enable && !pdev->mcopy_mode))
		return QDF_STATUS_E_NOSUPPORT;

	peer = (peer_id == HTT_INVALID_PEER) ? NULL :
			dp_peer_find_by_id(soc, peer_id);

	if (!peer) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				FL("Peer Invalid"));
		return QDF_STATUS_E_INVAL;
	}

	if (pdev->mcopy_mode) {
		if ((pdev->am_copy_id.tx_ppdu_id == ppdu_id) &&
			(pdev->am_copy_id.tx_peer_id == peer_id)) {
			return QDF_STATUS_E_INVAL;
		}

		pdev->am_copy_id.tx_ppdu_id = ppdu_id;
		pdev->am_copy_id.tx_peer_id = peer_id;
	}

	if (!qdf_nbuf_push_head(netbuf, sizeof(struct tx_capture_hdr))) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				FL("No headroom"));
		return QDF_STATUS_E_NOMEM;
	}

	ppdu_hdr = (struct tx_capture_hdr *)qdf_nbuf_data(netbuf);
	qdf_mem_copy(ppdu_hdr->ta, peer->vdev->mac_addr.raw,
					IEEE80211_ADDR_LEN);
	ppdu_hdr->ppdu_id = ppdu_id;
	qdf_mem_copy(ppdu_hdr->ra, peer->mac_addr.raw,
			IEEE80211_ADDR_LEN);
	ppdu_hdr->peer_id = peer_id;
	ppdu_hdr->first_msdu = first_msdu;
	ppdu_hdr->last_msdu = last_msdu;

	return QDF_STATUS_SUCCESS;
}


/**
 * dp_send_completion_to_stack() - send completion to stack
 * @soc :  dp_soc handle
 * @pdev:  dp_pdev handle
 * @peer_id: peer_id of the peer for which completion came
 * @ppdu_id: ppdu_id
 * @netbuf: Buffer pointer for free
 *
 * This function is used to send completion to stack
 * to free buffer
*/
void  dp_send_completion_to_stack(struct dp_soc *soc,  struct dp_pdev *pdev,
					uint16_t peer_id, uint32_t ppdu_id,
					qdf_nbuf_t netbuf)
{
	dp_wdi_event_handler(WDI_EVENT_TX_DATA, soc,
				netbuf, peer_id,
				WDI_NO_VAL, pdev->pdev_id);
}
#else
static QDF_STATUS
dp_get_completion_indication_for_stack(struct dp_soc *soc,  struct dp_pdev *pdev,
		      uint16_t peer_id, uint32_t ppdu_id, uint8_t first_msdu,
		      uint8_t last_msdu, qdf_nbuf_t netbuf)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static void
dp_send_completion_to_stack(struct dp_soc *soc,  struct dp_pdev *pdev,
		      uint16_t peer_id, uint32_t ppdu_id, qdf_nbuf_t netbuf)
{
}
#endif

/**
 * dp_tx_comp_free_buf() - Free nbuf associated with the Tx Descriptor
 * @soc: Soc handle
 * @desc: software Tx descriptor to be processed
 *
 * Return: none
 */
static inline void dp_tx_comp_free_buf(struct dp_soc *soc,
		struct dp_tx_desc_s *desc)
{
	struct dp_vdev *vdev = desc->vdev;
	qdf_nbuf_t nbuf = desc->nbuf;

	/* If it is TDLS mgmt, don't unmap or free the frame */
	if (desc->flags & DP_TX_DESC_FLAG_TDLS_FRAME)
		return dp_non_std_tx_comp_free_buff(desc, vdev);

	/* 0 : MSDU buffer, 1 : MLE */
	if (desc->msdu_ext_desc) {
		/* TSO free */
		if (hal_tx_ext_desc_get_tso_enable(
					desc->msdu_ext_desc->vaddr)) {
			/* If remaining number of segment is 0
			 * actual TSO may unmap and free */
			if (qdf_nbuf_get_users(nbuf) == 1)
				__qdf_nbuf_unmap_single(soc->osdev,
						nbuf,
						QDF_DMA_TO_DEVICE);

			qdf_nbuf_free(nbuf);
			return;
		}
	}

	qdf_nbuf_unmap(soc->osdev, nbuf, QDF_DMA_TO_DEVICE);

	if (qdf_likely(!vdev->mesh_vdev))
		qdf_nbuf_free(nbuf);
	else {
		if (desc->flags & DP_TX_DESC_FLAG_TO_FW) {
			qdf_nbuf_free(nbuf);
			DP_STATS_INC(vdev, tx_i.mesh.completion_fw, 1);
		} else
			vdev->osif_tx_free_ext((nbuf));
	}
}

/**
 * dp_tx_mec_handler() - Tx  MEC Notify Handler
 * @vdev: pointer to dp dev handler
 * @status : Tx completion status from HTT descriptor
 *
 * Handles MEC notify event sent from fw to Host
 *
 * Return: none
 */
#ifdef FEATURE_WDS
void dp_tx_mec_handler(struct dp_vdev *vdev, uint8_t *status)
{

	struct dp_soc *soc;
	uint32_t flags = IEEE80211_NODE_F_WDS_HM;
	struct dp_peer *peer;
	uint8_t mac_addr[DP_MAC_ADDR_LEN], i;

	if (!vdev->wds_enabled)
		return;

	soc = vdev->pdev->soc;
	qdf_spin_lock_bh(&soc->peer_ref_mutex);
	peer = TAILQ_FIRST(&vdev->peer_list);
	qdf_spin_unlock_bh(&soc->peer_ref_mutex);

	if (!peer) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
				FL("peer is NULL"));
		return;
	}

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"%s Tx MEC Handler\n",
			__func__);

	for (i = 0; i < DP_MAC_ADDR_LEN; i++)
		mac_addr[(DP_MAC_ADDR_LEN - 1) - i] =
					status[(DP_MAC_ADDR_LEN - 2) + i];

	if (qdf_mem_cmp(mac_addr, vdev->mac_addr.raw, DP_MAC_ADDR_LEN))
		dp_peer_add_ast(soc,
				peer,
				mac_addr,
				CDP_TXRX_AST_TYPE_MEC,
				flags);
}
#else
static void dp_tx_mec_handler(struct dp_vdev *vdev, uint8_t *status)
{
}
#endif

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
	struct dp_vdev *vdev;
	struct dp_soc *soc;
	uint32_t *htt_status_word = (uint32_t *) status;

	qdf_assert(tx_desc->pdev);

	pdev = tx_desc->pdev;
	vdev = tx_desc->vdev;
	soc = pdev->soc;

	tx_status = HTT_TX_WBM_COMPLETION_V2_TX_STATUS_GET(htt_status_word[0]);

	switch (tx_status) {
	case HTT_TX_FW2WBM_TX_STATUS_OK:
	case HTT_TX_FW2WBM_TX_STATUS_DROP:
	case HTT_TX_FW2WBM_TX_STATUS_TTL:
	{
		dp_tx_comp_free_buf(soc, tx_desc);
		dp_tx_desc_release(tx_desc, tx_desc->pool_id);
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
	case HTT_TX_FW2WBM_TX_STATUS_MEC_NOTIFY:
	{
		dp_tx_mec_handler(vdev, status);
		break;
	}
	default:
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
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
		if (qdf_nbuf_pull_head(netbuf, tx_desc->pkt_offset) == NULL) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"netbuf %pK offset %d\n",
				netbuf, tx_desc->pkt_offset);
			return;
		}
	}
	if (qdf_nbuf_push_head(netbuf, sizeof(struct meta_hdr_s)) == NULL) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"netbuf %pK offset %d\n", netbuf,
			sizeof(struct meta_hdr_s));
		return;
	}

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
 * dp_tx_update_peer_stats() - Update peer stats from Tx completion indications
 * @peer: Handle to DP peer
 * @ts: pointer to HAL Tx completion stats
 * @length: MSDU length
 *
 * Return: None
 */
static void dp_tx_update_peer_stats(struct dp_peer *peer,
		struct hal_tx_completion_status *ts, uint32_t length)
{
	struct dp_pdev *pdev = peer->vdev->pdev;
	struct dp_soc *soc = pdev->soc;
	uint8_t mcs, pkt_type;

	mcs = ts->mcs;
	pkt_type = ts->pkt_type;


	if (!ts->release_src == HAL_TX_COMP_RELEASE_SOURCE_TQM)
		return;

	DP_STATS_INCC(peer, tx.dropped.age_out, 1,
			(ts->status == HAL_TX_TQM_RR_REM_CMD_AGED));

	DP_STATS_INCC(peer, tx.dropped.fw_rem, 1,
			(ts->status == HAL_TX_TQM_RR_REM_CMD_REM));

	DP_STATS_INCC(peer, tx.dropped.fw_rem_notx, 1,
			(ts->status == HAL_TX_TQM_RR_REM_CMD_NOTX));

	DP_STATS_INCC(peer, tx.dropped.fw_rem_tx, 1,
			(ts->status == HAL_TX_TQM_RR_REM_CMD_TX));

	if (!ts->status == HAL_TX_TQM_RR_FRAME_ACKED)
		return;

	DP_STATS_INCC(peer, tx.ofdma, 1, ts->ofdma);

	DP_STATS_INCC(peer, tx.amsdu_cnt, 1, ts->msdu_part_of_amsdu);

	if (!(soc->process_tx_status))
		return;

	DP_STATS_INCC(peer, tx.pkt_type[pkt_type].mcs_count[MAX_MCS - 1], 1,
			((mcs >= MAX_MCS_11A) && (pkt_type == DOT11_A)));
	DP_STATS_INCC(peer, tx.pkt_type[pkt_type].mcs_count[mcs], 1,
			((mcs < (MAX_MCS_11A)) && (pkt_type == DOT11_A)));
	DP_STATS_INCC(peer, tx.pkt_type[pkt_type].mcs_count[MAX_MCS - 1], 1,
			((mcs >= MAX_MCS_11B) && (pkt_type == DOT11_B)));
	DP_STATS_INCC(peer, tx.pkt_type[pkt_type].mcs_count[mcs], 1,
			((mcs < MAX_MCS_11B) && (pkt_type == DOT11_B)));
	DP_STATS_INCC(peer, tx.pkt_type[pkt_type].mcs_count[MAX_MCS - 1], 1,
			((mcs >= MAX_MCS_11A) && (pkt_type == DOT11_N)));
	DP_STATS_INCC(peer, tx.pkt_type[pkt_type].mcs_count[mcs], 1,
			((mcs < MAX_MCS_11A) && (pkt_type == DOT11_N)));
	DP_STATS_INCC(peer, tx.pkt_type[pkt_type].mcs_count[MAX_MCS - 1], 1,
			((mcs >= MAX_MCS_11AC) && (pkt_type == DOT11_AC)));
	DP_STATS_INCC(peer, tx.pkt_type[pkt_type].mcs_count[mcs], 1,
			((mcs < MAX_MCS_11AC) && (pkt_type == DOT11_AC)));
	DP_STATS_INCC(peer, tx.pkt_type[pkt_type].mcs_count[MAX_MCS - 1], 1,
			((mcs >= (MAX_MCS - 1)) && (pkt_type == DOT11_AX)));
	DP_STATS_INCC(peer, tx.pkt_type[pkt_type].mcs_count[mcs], 1,
			((mcs < (MAX_MCS - 1)) && (pkt_type == DOT11_AX)));
	DP_STATS_INC(peer, tx.sgi_count[ts->sgi], 1);
	DP_STATS_INC(peer, tx.bw[ts->bw], 1);
	DP_STATS_UPD(peer, tx.last_ack_rssi, ts->ack_frame_rssi);
	DP_STATS_INC(peer, tx.wme_ac_type[TID_TO_WME_AC(ts->tid)], 1);
	DP_STATS_INCC(peer, tx.stbc, 1, ts->stbc);
	DP_STATS_INCC(peer, tx.ldpc, 1, ts->ldpc);
	DP_STATS_INC_PKT(peer, tx.tx_success, 1, length);
	DP_STATS_INCC(peer, tx.retries, 1, ts->transmit_cnt > 1);

	if (soc->cdp_soc.ol_ops->update_dp_stats) {
		soc->cdp_soc.ol_ops->update_dp_stats(pdev->osif_pdev,
				&peer->stats, ts->peer_id,
				UPDATE_PEER_STATS);
	}
}

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
	struct ether_header *eh =
		(struct ether_header *)qdf_nbuf_data(tx_desc->nbuf);
	bool isBroadcast;

	hal_tx_comp_get_status(&tx_desc->comp, &ts);

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
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

	if (!vdev) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
				"invalid vdev");
		goto out;
	}

	soc = vdev->pdev->soc;

	/* Update SoC level stats */
	DP_STATS_INCC(soc, tx.dropped_fw_removed, 1,
			(ts.status == HAL_TX_TQM_RR_REM_CMD_REM));

	/* Update per-packet stats */
	if (qdf_unlikely(vdev->mesh_vdev) &&
			!(tx_desc->flags & DP_TX_DESC_FLAG_TO_FW))
		dp_tx_comp_fill_tx_completion_stats(tx_desc, &ts);

	/* Update peer level stats */
	peer = dp_peer_find_by_id(soc, ts.peer_id);
	if (!peer) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
				"invalid peer");
		DP_STATS_INC_PKT(soc, tx.tx_invalid_peer, 1, length);
		goto out;
	}
	if (qdf_likely(vdev->tx_encap_type == htt_cmn_pkt_type_ethernet)) {
		isBroadcast = (IEEE80211_IS_BROADCAST(eh->ether_dhost)) ? 1 : 0 ;

		if (isBroadcast) {
			DP_STATS_INC_PKT(peer, tx.bcast, 1,
					qdf_nbuf_len(tx_desc->nbuf));
		}
	}

	dp_tx_update_peer_stats(peer, &ts, length);

out:
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

		dp_tx_comp_process_tx_status(desc, length);

		/*currently m_copy/tx_capture is not supported for scatter gather packets*/
		if (!(desc->msdu_ext_desc) && (dp_get_completion_indication_for_stack(soc,
					desc->pdev, ts.peer_id, ts.ppdu_id,
					ts.first_msdu, ts.last_msdu,
					desc->nbuf) == QDF_STATUS_SUCCESS)) {
			qdf_nbuf_unmap(soc->osdev, desc->nbuf,
						QDF_DMA_TO_DEVICE);

			dp_send_completion_to_stack(soc, desc->pdev, ts.peer_id,
				ts.ppdu_id, desc->nbuf);
		} else {
			dp_tx_comp_free_buf(soc, desc);
		}

		DP_HIST_PACKET_COUNT_INC(desc->pdev->pdev_id);

		next = desc->next;
		dp_tx_desc_release(desc, desc->pool_id);
		desc = next;
	}
	DP_TX_HIST_STATS_PER_PDEV();
}

/**
 * dp_tx_comp_handler() - Tx completion handler
 * @soc: core txrx main context
 * @ring_id: completion ring id
 * @quota: No. of packets/descriptors that can be serviced in one loop
 *
 * This function will collect hardware release ring element contents and
 * handle descriptor contents. Based on contents, free packet or handle error
 * conditions
 *
 * Return: none
 */
uint32_t dp_tx_comp_handler(struct dp_soc *soc, void *hal_srng, uint32_t quota)
{
	void *tx_comp_hal_desc;
	uint8_t buffer_src;
	uint8_t pool_id;
	uint32_t tx_desc_id;
	struct dp_tx_desc_s *tx_desc = NULL;
	struct dp_tx_desc_s *head_desc = NULL;
	struct dp_tx_desc_s *tail_desc = NULL;
	uint32_t num_processed;
	uint32_t count;

	if (qdf_unlikely(hal_srng_access_start(soc->hal_soc, hal_srng))) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s %d : HAL RING Access Failed -- %pK\n",
				__func__, __LINE__, hal_srng);
		return 0;
	}

	num_processed = 0;
	count = 0;

	/* Find head descriptor from completion ring */
	while (qdf_likely(tx_comp_hal_desc =
			hal_srng_dst_get_next(soc->hal_soc, hal_srng))) {

		buffer_src = hal_tx_comp_get_buffer_source(tx_comp_hal_desc);

		/* If this buffer was not released by TQM or FW, then it is not
		 * Tx completion indication, assert */
		if ((buffer_src != HAL_TX_COMP_RELEASE_SOURCE_TQM) &&
				(buffer_src != HAL_TX_COMP_RELEASE_SOURCE_FW)) {

			QDF_TRACE(QDF_MODULE_ID_DP,
					QDF_TRACE_LEVEL_FATAL,
					"Tx comp release_src != TQM | FW");

			qdf_assert_always(0);
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
					"Tx Comp pool id %d not valid",
					pool_id);

			qdf_assert_always(0);
		}

		/* Find Tx descriptor */
		tx_desc = dp_tx_desc_find(soc, pool_id,
				(tx_desc_id & DP_TX_DESC_ID_PAGE_MASK) >>
				DP_TX_DESC_ID_PAGE_OS,
				(tx_desc_id & DP_TX_DESC_ID_OFFSET_MASK) >>
				DP_TX_DESC_ID_OFFSET_OS);

		/*
		 * If the release source is FW, process the HTT status
		 */
		if (qdf_unlikely(buffer_src ==
					HAL_TX_COMP_RELEASE_SOURCE_FW)) {
			uint8_t htt_tx_status[HAL_TX_COMP_HTT_STATUS_LEN];
			hal_tx_comp_get_htt_desc(tx_comp_hal_desc,
					htt_tx_status);
			dp_tx_process_htt_completion(tx_desc,
					htt_tx_status);
		} else {
			/* Pool id is not matching. Error */
			if (tx_desc->pool_id != pool_id) {
				QDF_TRACE(QDF_MODULE_ID_DP,
					QDF_TRACE_LEVEL_FATAL,
					"Tx Comp pool id %d not matched %d",
					pool_id, tx_desc->pool_id);

				qdf_assert_always(0);
			}

			if (!(tx_desc->flags & DP_TX_DESC_FLAG_ALLOCATED) ||
				!(tx_desc->flags & DP_TX_DESC_FLAG_QUEUED_TX)) {
				QDF_TRACE(QDF_MODULE_ID_DP,
					QDF_TRACE_LEVEL_FATAL,
					"Txdesc invalid, flgs = %x,id = %d",
					tx_desc->flags,	tx_desc_id);
				qdf_assert_always(0);
			}

			/* First ring descriptor on the cycle */
			if (!head_desc) {
				head_desc = tx_desc;
				tail_desc = tx_desc;
			}

			tail_desc->next = tx_desc;
			tx_desc->next = NULL;
			tail_desc = tx_desc;

			/* Collect hw completion contents */
			hal_tx_comp_desc_sync(tx_comp_hal_desc,
					&tx_desc->comp, 1);

		}

		num_processed += !(count & DP_TX_NAPI_BUDGET_DIV_MASK);
		/* Decrement PM usage count if the packet has been sent.*/
		hif_pm_runtime_put(soc->hif_handle);

		/*
		 * Processed packet count is more than given quota
		 * stop to processing
		 */
		if ((num_processed >= quota))
			break;

		count++;
	}

	hal_srng_access_end(soc->hal_soc, hal_srng);

	/* Process the reaped descriptors */
	if (head_desc)
		dp_tx_comp_process_desc(soc, head_desc);

	return num_processed;
}

#ifdef CONVERGED_TDLS_ENABLE
/**
 * dp_tx_non_std() - Allow the control-path SW to send data frames
 *
 * @data_vdev - which vdev should transmit the tx data frames
 * @tx_spec - what non-standard handling to apply to the tx data frames
 * @msdu_list - NULL-terminated list of tx MSDUs
 *
 * Return: NULL on success,
 *         nbuf when it fails to send
 */
qdf_nbuf_t dp_tx_non_std(struct cdp_vdev *vdev_handle,
			enum ol_tx_spec tx_spec, qdf_nbuf_t msdu_list)
{
	struct dp_vdev *vdev = (struct dp_vdev *) vdev_handle;

	if (tx_spec & OL_TX_SPEC_NO_FREE)
		vdev->is_tdls_frame = true;
	return dp_tx_send(vdev_handle, msdu_list);
}
#endif

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

#ifdef QCA_LL_TX_FLOW_CONTROL_V2
/* Pools will be allocated dynamically */
static int dp_tx_alloc_static_pools(struct dp_soc *soc, int num_pool,
					int num_desc)
{
	uint8_t i;

	for (i = 0; i < num_pool; i++) {
		qdf_spinlock_create(&soc->tx_desc[i].flow_pool_lock);
		soc->tx_desc[i].status = FLOW_POOL_INACTIVE;
	}

	return 0;
}

static void dp_tx_delete_static_pools(struct dp_soc *soc, int num_pool)
{
	uint8_t i;

	for (i = 0; i < num_pool; i++)
		qdf_spinlock_destroy(&soc->tx_desc[i].flow_pool_lock);
}

static void dp_tx_desc_flush(struct dp_pdev *pdev)
{
}
#else /* QCA_LL_TX_FLOW_CONTROL_V2! */
static int dp_tx_alloc_static_pools(struct dp_soc *soc, int num_pool,
					int num_desc)
{
	uint8_t i;

	/* Allocate software Tx descriptor pools */
	for (i = 0; i < num_pool; i++) {
		if (dp_tx_desc_pool_alloc(soc, i, num_desc)) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
					"%s Tx Desc Pool alloc %d failed %pK\n",
					__func__, i, soc);
			return ENOMEM;
		}
	}
	return 0;
}

static void dp_tx_delete_static_pools(struct dp_soc *soc, int num_pool)
{
	uint8_t i;

	for (i = 0; i < num_pool; i++) {
		if (dp_tx_desc_pool_free(soc, i)) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
				"%s Tx Desc Pool Free failed\n", __func__);
		}
	}
}

/* dp_tx_desc_flush() - release resources associated
 *                      to tx_desc
 * @pdev: physical device instance
 *
 * This function will free all outstanding Tx buffers,
 * including ME buffer for which either free during
 * completion didn't happened or completion is not
 * received.
*/
static void dp_tx_desc_flush(struct dp_pdev *pdev)
{
	uint8_t i, num_pool;
	uint32_t j;
	uint32_t num_desc;
	struct dp_soc *soc = pdev->soc;
	struct dp_tx_desc_s *tx_desc = NULL;

	num_desc = wlan_cfg_get_num_tx_desc(soc->wlan_cfg_ctx);
	num_pool = wlan_cfg_get_num_tx_desc_pool(soc->wlan_cfg_ctx);

	for (i = 0; i < num_pool; i++) {
		for (j = 0; j < num_desc; j++) {
			tx_desc = dp_tx_desc_find(soc, i,
					(j & DP_TX_DESC_ID_PAGE_MASK) >>
					DP_TX_DESC_ID_PAGE_OS,
					(j & DP_TX_DESC_ID_OFFSET_MASK) >>
					DP_TX_DESC_ID_OFFSET_OS);

			if (tx_desc && (tx_desc->pdev == pdev) &&
				(tx_desc->flags & DP_TX_DESC_FLAG_ALLOCATED)) {
				dp_tx_comp_free_buf(soc, tx_desc);
				dp_tx_desc_release(tx_desc, i);
			}
		}
	}
}
#endif /* !QCA_LL_TX_FLOW_CONTROL_V2 */

/**
 * dp_tx_pdev_detach() - detach pdev from dp tx
 * @pdev: physical device instance
 *
 * Return: QDF_STATUS_SUCCESS: success
 *         QDF_STATUS_E_RESOURCES: Error return
 */
QDF_STATUS dp_tx_pdev_detach(struct dp_pdev *pdev)
{
	dp_tx_desc_flush(pdev);
	dp_tx_me_exit(pdev);
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

	dp_tx_flow_control_deinit(soc);
	dp_tx_delete_static_pools(soc, num_pool);

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
	uint8_t i;
	uint8_t num_pool;
	uint32_t num_desc;
	uint32_t num_ext_desc;

	num_pool = wlan_cfg_get_num_tx_desc_pool(soc->wlan_cfg_ctx);
	num_desc = wlan_cfg_get_num_tx_desc(soc->wlan_cfg_ctx);
	num_ext_desc = wlan_cfg_get_num_tx_ext_desc(soc->wlan_cfg_ctx);

	if (dp_tx_alloc_static_pools(soc, num_pool, num_desc))
		goto fail;

	dp_tx_flow_control_init(soc);

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			"%s Tx Desc Alloc num_pool = %d, descs = %d\n",
			__func__, num_pool, num_desc);

	/* Allocate extension tx descriptor pools */
	for (i = 0; i < num_pool; i++) {
		if (dp_tx_ext_desc_pool_alloc(soc, i, num_ext_desc)) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"MSDU Ext Desc Pool alloc %d failed %pK\n",
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
				"TSO Desc Pool alloc %d failed %pK\n",
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
				"TSO Num of seg Pool alloc %d failed %pK\n",
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
	soc->process_tx_status = CONFIG_PROCESS_TX_STATUS;

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
static void dp_tx_me_mem_free(struct dp_pdev *pdev,
		struct dp_tx_seg_info_s *seg_info_head)
{
	struct dp_tx_me_buf_t *mc_uc_buf;
	struct dp_tx_seg_info_s *seg_info_new = NULL;
	qdf_nbuf_t nbuf = NULL;
	uint64_t phy_addr;

	while (seg_info_head) {
		nbuf = seg_info_head->nbuf;
		mc_uc_buf = (struct dp_tx_me_buf_t *)
			seg_info_head->frags[0].vaddr;
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

	qdf_mem_set(&msdu_info, sizeof(msdu_info), 0x0);

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
		qdf_nbuf_free(nbuf);
		return 1;
	}

	paddr_data = qdf_nbuf_get_frag_paddr(nbuf, 0) + IEEE80211_ADDR_LEN;

	/*preparing data fragment*/
	data_frag.vaddr = qdf_nbuf_data(nbuf) + IEEE80211_ADDR_LEN;
	data_frag.paddr_lo = (uint32_t)paddr_data;
	data_frag.paddr_hi = (((uint64_t) paddr_data)  >> 32);
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
			((uint64_t) paddr_mcbuf >> 32);
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

	if (!seg_info_head) {
		goto free_return;
	}

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

	qdf_nbuf_unmap(pdev->soc->osdev, nbuf, QDF_DMA_TO_DEVICE);
	qdf_nbuf_free(nbuf);
	return new_mac_cnt;

fail_map:
	qdf_nbuf_free(nbuf_clone);

fail_clone:
	dp_tx_me_free_buf(pdev, mc_uc_buf);

fail_buf_alloc:
	qdf_mem_free(seg_info_new);

fail_seg_alloc:
	dp_tx_me_mem_free(pdev, seg_info_head);

free_return:
	qdf_nbuf_unmap(pdev->soc->osdev, nbuf, QDF_DMA_TO_DEVICE);
	qdf_nbuf_free(nbuf);
	return 1;
}

