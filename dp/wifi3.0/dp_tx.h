/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
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
#ifndef __DP_TX_H
#define __DP_TX_H

#include <qdf_types.h>
#include <qdf_nbuf.h>
#include "dp_types.h"


#define DP_TX_MAX_NUM_FRAGS 6

#define DP_TX_DESC_FLAG_ALLOCATED	0x1
#define DP_TX_DESC_FLAG_TO_FW		0x2
#define DP_TX_DESC_FLAG_FRAG		0x4
#define DP_TX_DESC_FLAG_RAW		0x8
#define DP_TX_DESC_FLAG_MESH		0x10
#define DP_TX_DESC_FLAG_QUEUED_TX	0x20
#define DP_TX_DESC_FLAG_COMPLETED_TX	0x40
#define DP_TX_DESC_FLAG_ME		0x80
#define DP_TX_DESC_FLAG_TDLS_FRAME	0x100

#define DP_TX_FREE_SINGLE_BUF(soc, buf)                  \
do {                                                           \
	qdf_nbuf_unmap(soc->osdev, buf, QDF_DMA_TO_DEVICE);  \
	qdf_nbuf_free(buf);                                    \
} while (0)

#define OCB_HEADER_VERSION	 1

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
	#endif /* TX_PER_VDEV_DESC_POOL */
#endif /* TX_PER_PDEV_DESC_POOL */
#define DP_TX_QUEUE_MASK 0x3
/**
 * struct dp_tx_frag_info_s
 * @vaddr: hlos vritual address for buffer
 * @paddr_lo: physical address lower 32bits
 * @paddr_hi: physical address higher bits
 * @len: length of the buffer
 */
struct dp_tx_frag_info_s {
	uint8_t  *vaddr;
	uint32_t paddr_lo;
	uint16_t paddr_hi;
	uint16_t len;
};

/**
 * struct dp_tx_seg_info_s - Segmentation Descriptor
 * @nbuf: NBUF pointer if segment corresponds to separate nbuf
 * @frag_cnt: Fragment count in this segment
 * @total_len: Total length of segment
 * @frags: per-Fragment information
 * @next: pointer to next MSDU segment
 */
struct dp_tx_seg_info_s  {
	qdf_nbuf_t nbuf;
	uint16_t frag_cnt;
	uint16_t total_len;
	struct dp_tx_frag_info_s frags[DP_TX_MAX_NUM_FRAGS];
	struct dp_tx_seg_info_s *next;
};

/**
 * struct dp_tx_sg_info_s - Scatter Gather Descriptor
 * @num_segs: Number of segments (TSO/ME) in the frame
 * @total_len: Total length of the frame
 * @curr_seg: Points to current segment descriptor to be processed. Chain of
 * 	      descriptors for SG frames/multicast-unicast converted packets.
 *
 * Used for SG (802.3 or Raw) frames and Multicast-Unicast converted frames to
 * carry fragmentation information
 * Raw Frames will be handed over to driver as an SKB chain with MPDU boundaries
 * indicated through flags in SKB CB (first_msdu and last_msdu). This will be
 * converted into set of skb sg (nr_frags) structures.
 */
struct dp_tx_sg_info_s {
	uint32_t num_segs;
	uint32_t total_len;
	struct dp_tx_seg_info_s *curr_seg;
};

/**
 * struct dp_tx_queue - Tx queue
 * @desc_pool_id: Descriptor Pool to be used for the tx queue
 * @ring_id: TCL descriptor ring ID corresponding to the tx queue
 *
 * Tx queue contains information of the software (Descriptor pool)
 * and hardware resources (TCL ring id) to be used for a particular
 * transmit queue (obtained from skb_queue_mapping in case of linux)
 */
struct dp_tx_queue {
	uint8_t desc_pool_id;
	uint8_t ring_id;
};

/**
 * struct dp_tx_msdu_info_s - MSDU Descriptor
 * @frm_type: Frame type - Regular/TSO/SG/Multicast enhancement
 * @tx_queue: Tx queue on which this MSDU should be transmitted
 * @num_seg: Number of segments (TSO)
 * @tid: TID (override) that is sent from HLOS
 * @u.tso_info: TSO information for TSO frame types
 * 	     (chain of the TSO segments, number of segments)
 * @u.sg_info: Scatter Gather information for non-TSO SG frames
 * @meta_data: Mesh meta header information
 * @exception_fw: Duplicate frame to be sent to firmware
 * @ppdu_cookie: 16-bit ppdu_cookie that has to be replayed back in completions
 * @ix_tx_sniffer: Indicates if the packet has to be sniffed
 *
 * This structure holds the complete MSDU information needed to program the
 * Hardware TCL and MSDU extension descriptors for different frame types
 *
 */
struct dp_tx_msdu_info_s {
	enum dp_tx_frm_type frm_type;
	struct dp_tx_queue tx_queue;
	uint32_t num_seg;
	uint8_t tid;
	union {
		struct qdf_tso_info_t tso_info;
		struct dp_tx_sg_info_s sg_info;
	} u;
	uint32_t meta_data[7];
	uint8_t exception_fw;
	uint16_t ppdu_cookie;
	uint8_t is_tx_sniffer;
};

QDF_STATUS dp_tx_vdev_attach(struct dp_vdev *vdev);
QDF_STATUS dp_tx_vdev_detach(struct dp_vdev *vdev);
void dp_tx_vdev_update_search_flags(struct dp_vdev *vdev);

QDF_STATUS dp_tx_soc_attach(struct dp_soc *soc);
QDF_STATUS dp_tx_soc_detach(struct dp_soc *soc);

/**
 * dp_tso_attach() - TSO Attach handler
 * @txrx_soc: Opaque Dp handle
 *
 * Reserve TSO descriptor buffers
 *
 * Return: QDF_STATUS_E_FAILURE on failure or
 * QDF_STATUS_SUCCESS on success
 */
QDF_STATUS dp_tso_soc_attach(struct cdp_soc_t *txrx_soc);

/**
 * dp_tso_detach() - TSO Detach handler
 * @txrx_soc: Opaque Dp handle
 *
 * Deallocate TSO descriptor buffers
 *
 * Return: QDF_STATUS_E_FAILURE on failure or
 * QDF_STATUS_SUCCESS on success
 */
QDF_STATUS dp_tso_soc_detach(struct cdp_soc_t *txrx_soc);

QDF_STATUS dp_tx_pdev_detach(struct dp_pdev *pdev);
QDF_STATUS dp_tx_pdev_attach(struct dp_pdev *pdev);

qdf_nbuf_t dp_tx_send(struct cdp_soc_t *soc, uint8_t vdev_id, qdf_nbuf_t nbuf);
qdf_nbuf_t dp_tx_send_exception(struct cdp_vdev *data_vdev, qdf_nbuf_t nbuf,
				struct cdp_tx_exception_metadata *tx_exc);
qdf_nbuf_t dp_tx_send_mesh(struct cdp_soc_t *soc, uint8_t vdev_id,
			   qdf_nbuf_t nbuf);
qdf_nbuf_t
dp_tx_send_msdu_single(struct dp_vdev *vdev, qdf_nbuf_t nbuf,
		       struct dp_tx_msdu_info_s *msdu_info, uint16_t peer_id,
		       struct cdp_tx_exception_metadata *tx_exc_metadata);

#if QDF_LOCK_STATS
noinline qdf_nbuf_t
dp_tx_send_msdu_multiple(struct dp_vdev *vdev, qdf_nbuf_t nbuf,
			 struct dp_tx_msdu_info_s *msdu_info);
#else
qdf_nbuf_t dp_tx_send_msdu_multiple(struct dp_vdev *vdev, qdf_nbuf_t nbuf,
				    struct dp_tx_msdu_info_s *msdu_info);
#endif
#ifdef FEATURE_WLAN_TDLS
/**
 * dp_tx_non_std() - Allow the control-path SW to send data frames
 * @soc_hdl: Datapath soc handle
 * @vdev_id: id of vdev
 * @tx_spec: what non-standard handling to apply to the tx data frames
 * @msdu_list: NULL-terminated list of tx MSDUs
 *
 * Return: NULL on success,
 *         nbuf when it fails to send
 */
qdf_nbuf_t dp_tx_non_std(struct cdp_soc_t *soc_hdl, uint8_t vdev_id,
			 enum ol_tx_spec tx_spec, qdf_nbuf_t msdu_list);
#endif
int dp_tx_frame_is_drop(struct dp_vdev *vdev, uint8_t *srcmac, uint8_t *dstmac);

/**
 * dp_tx_comp_handler() - Tx completion handler
 * @int_ctx: pointer to DP interrupt context
 * @soc: core txrx main context
 * @hal_srng: Opaque HAL SRNG pointer
 * @ring_id: completion ring id
 * @quota: No. of packets/descriptors that can be serviced in one loop
 *
 * This function will collect hardware release ring element contents and
 * handle descriptor contents. Based on contents, free packet or handle error
 * conditions
 *
 * Return: Number of TX completions processed
 */
uint32_t dp_tx_comp_handler(struct dp_intr *int_ctx, struct dp_soc *soc,
			    hal_ring_handle_t hal_srng, uint8_t ring_id,
			    uint32_t quota);

QDF_STATUS
dp_tx_prepare_send_me(struct dp_vdev *vdev, qdf_nbuf_t nbuf);

#ifndef FEATURE_WDS
static inline void dp_tx_mec_handler(struct dp_vdev *vdev, uint8_t *status)
{
	return;
}
#endif

#ifndef ATH_SUPPORT_IQUE
static inline void dp_tx_me_exit(struct dp_pdev *pdev)
{
	return;
}
#endif

#ifndef QCA_MULTIPASS_SUPPORT
static inline
bool dp_tx_multipass_process(struct dp_soc *soc, struct dp_vdev *vdev,
			     qdf_nbuf_t nbuf,
			     struct dp_tx_msdu_info_s *msdu_info)
{
	return true;
}

static inline
void dp_tx_vdev_multipass_deinit(struct dp_vdev *vdev)
{
}

#else
bool dp_tx_multipass_process(struct dp_soc *soc, struct dp_vdev *vdev,
			     qdf_nbuf_t nbuf,
			     struct dp_tx_msdu_info_s *msdu_info);

void dp_tx_vdev_multipass_deinit(struct dp_vdev *vdev);
#endif

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
#ifdef QCA_OL_TX_MULTIQ_SUPPORT
static inline void dp_tx_get_queue(struct dp_vdev *vdev,
				   qdf_nbuf_t nbuf, struct dp_tx_queue *queue)
{
	uint16_t queue_offset = qdf_nbuf_get_queue_mapping(nbuf) &
				DP_TX_QUEUE_MASK;

	queue->desc_pool_id = queue_offset;
	queue->ring_id = vdev->pdev->soc->tx_ring_map[queue_offset];

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
		  "%s, pool_id:%d ring_id: %d",
		  __func__, queue->desc_pool_id, queue->ring_id);
}
#else /* QCA_OL_TX_MULTIQ_SUPPORT */
static inline void dp_tx_get_queue(struct dp_vdev *vdev,
				   qdf_nbuf_t nbuf, struct dp_tx_queue *queue)
{
	/* get flow id */
	queue->desc_pool_id = DP_TX_GET_DESC_POOL_ID(vdev);
	queue->ring_id = DP_TX_GET_RING_ID(vdev);

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
		  "%s, pool_id:%d ring_id: %d",
		  __func__, queue->desc_pool_id, queue->ring_id);
}
#endif
#ifdef FEATURE_PERPKT_INFO
QDF_STATUS
dp_get_completion_indication_for_stack(struct dp_soc *soc,
				       struct dp_pdev *pdev,
				       struct dp_peer *peer,
				       struct hal_tx_completion_status *ts,
				       qdf_nbuf_t netbuf,
				       uint64_t time_latency);

void  dp_send_completion_to_stack(struct dp_soc *soc,  struct dp_pdev *pdev,
		uint16_t peer_id, uint32_t ppdu_id,
		qdf_nbuf_t netbuf);
#endif

void  dp_iterate_update_peer_list(struct cdp_pdev *pdev_hdl);

#ifdef ATH_TX_PRI_OVERRIDE
#define DP_TX_TID_OVERRIDE(_msdu_info, _nbuf) \
	((_msdu_info)->tid = qdf_nbuf_get_priority(_nbuf))
#else
#define DP_TX_TID_OVERRIDE(_msdu_info, _nbuf)
#endif

void
dp_handle_wbm_internal_error(struct dp_soc *soc, void *hal_desc,
			     uint32_t buf_type);

/* TODO TX_FEATURE_NOT_YET */
static inline void dp_tx_comp_process_exception(struct dp_tx_desc_s *tx_desc)
{
	return;
}
/* TODO TX_FEATURE_NOT_YET */

#ifndef WLAN_TX_PKT_CAPTURE_ENH
static inline
void dp_peer_set_tx_capture_enabled(struct cdp_peer *peer_handle, bool value)
{
}
#endif
#endif
