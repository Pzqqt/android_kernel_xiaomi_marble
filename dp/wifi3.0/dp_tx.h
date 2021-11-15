/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
#if defined(MESH_MODE_SUPPORT) || defined(FEATURE_PERPKT_INFO)
#include "if_meta_hdr.h"
#endif
#include "dp_internal.h"
#include "hal_tx.h"
#include <qdf_tracepoint.h>

#define DP_INVALID_VDEV_ID 0xFF

#define DP_TX_MAX_NUM_FRAGS 6

/*
 * DP_TX_DESC_FLAG_FRAG flags should always be defined to 0x1
 * please do not change this flag's definition
 */
#define DP_TX_DESC_FLAG_FRAG		0x1
#define DP_TX_DESC_FLAG_TO_FW		0x2
#define DP_TX_DESC_FLAG_SIMPLE		0x4
#define DP_TX_DESC_FLAG_RAW		0x8
#define DP_TX_DESC_FLAG_MESH		0x10
#define DP_TX_DESC_FLAG_QUEUED_TX	0x20
#define DP_TX_DESC_FLAG_COMPLETED_TX	0x40
#define DP_TX_DESC_FLAG_ME		0x80
#define DP_TX_DESC_FLAG_TDLS_FRAME	0x100
#define DP_TX_DESC_FLAG_ALLOCATED	0x200
#define DP_TX_DESC_FLAG_MESH_MODE	0x400
#define DP_TX_DESC_FLAG_UNMAP_DONE	0x800
#define DP_TX_DESC_FLAG_TX_COMP_ERR	0x1000
#define DP_TX_DESC_FLAG_FLUSH		0x2000

#define DP_TX_EXT_DESC_FLAG_METADATA_VALID 0x1

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

#define MAX_CDP_SEC_TYPE 12

/* number of dwords for htt_tx_msdu_desc_ext2_t */
#define DP_TX_MSDU_INFO_META_DATA_DWORDS 7

#define dp_tx_alert(params...) QDF_TRACE_FATAL(QDF_MODULE_ID_DP_TX, params)
#define dp_tx_err(params...) QDF_TRACE_ERROR(QDF_MODULE_ID_DP_TX, params)
#define dp_tx_err_rl(params...) QDF_TRACE_ERROR_RL(QDF_MODULE_ID_DP_TX, params)
#define dp_tx_warn(params...) QDF_TRACE_WARN(QDF_MODULE_ID_DP_TX, params)
#define dp_tx_info(params...) \
	__QDF_TRACE_FL(QDF_TRACE_LEVEL_INFO_HIGH, QDF_MODULE_ID_DP_TX, ## params)
#define dp_tx_debug(params...) QDF_TRACE_DEBUG(QDF_MODULE_ID_DP_TX, params)

#define dp_tx_comp_alert(params...) QDF_TRACE_FATAL(QDF_MODULE_ID_DP_TX_COMP, params)
#define dp_tx_comp_err(params...) QDF_TRACE_ERROR(QDF_MODULE_ID_DP_TX_COMP, params)
#define dp_tx_comp_warn(params...) QDF_TRACE_WARN(QDF_MODULE_ID_DP_TX_COMP, params)
#define dp_tx_comp_info(params...) \
	__QDF_TRACE_FL(QDF_TRACE_LEVEL_INFO_HIGH, QDF_MODULE_ID_DP_TX_COMP, ## params)
#define dp_tx_comp_info_rl(params...) \
	__QDF_TRACE_RL(QDF_TRACE_LEVEL_INFO_HIGH, QDF_MODULE_ID_DP_TX_COMP, ## params)
#define dp_tx_comp_debug(params...) QDF_TRACE_DEBUG(QDF_MODULE_ID_DP_TX_COMP, params)

#ifndef QCA_HOST_MODE_WIFI_DISABLED

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

#endif /* QCA_HOST_MODE_WIFI_DISABLED */

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
 * @skip_hp_update : Skip HP update for TSO segments and update in last segment
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
	uint8_t exception_fw;
	uint8_t is_tx_sniffer;
	union {
		struct qdf_tso_info_t tso_info;
		struct dp_tx_sg_info_s sg_info;
	} u;
	uint32_t meta_data[DP_TX_MSDU_INFO_META_DATA_DWORDS];
	uint16_t ppdu_cookie;
#ifdef WLAN_DP_FEATURE_SW_LATENCY_MGR
	uint8_t skip_hp_update;
#endif
};

#ifndef QCA_HOST_MODE_WIFI_DISABLED
/**
 * dp_tx_deinit_pair_by_index() - Deinit TX rings based on index
 * @soc: core txrx context
 * @index: index of ring to deinit
 *
 * Deinit 1 TCL and 1 WBM2SW release ring on as needed basis using
 * index of the respective TCL/WBM2SW release in soc structure.
 * For example, if the index is 2 then &soc->tcl_data_ring[2]
 * and &soc->tx_comp_ring[2] will be deinitialized.
 *
 * Return: none
 */
void dp_tx_deinit_pair_by_index(struct dp_soc *soc, int index);
#endif /* QCA_HOST_MODE_WIFI_DISABLED */

void dp_tx_tso_cmn_desc_pool_deinit(struct dp_soc *soc, uint8_t num_pool);
void dp_tx_tso_cmn_desc_pool_free(struct dp_soc *soc, uint8_t num_pool);
QDF_STATUS dp_tx_tso_cmn_desc_pool_alloc(struct dp_soc *soc,
					 uint8_t num_pool,
					 uint16_t num_desc);
QDF_STATUS dp_tx_tso_cmn_desc_pool_init(struct dp_soc *soc,
					uint8_t num_pool,
					uint16_t num_desc);

void dp_tx_tso_cmn_desc_pool_deinit(struct dp_soc *soc, uint8_t num_pool);
void dp_tx_tso_cmn_desc_pool_free(struct dp_soc *soc, uint8_t num_pool);
QDF_STATUS dp_tx_tso_cmn_desc_pool_alloc(struct dp_soc *soc,
					 uint8_t num_pool,
					 uint16_t num_desc);
QDF_STATUS dp_tx_tso_cmn_desc_pool_init(struct dp_soc *soc,
					uint8_t num_pool,
					uint16_t num_desc);
void dp_tx_comp_free_buf(struct dp_soc *soc, struct dp_tx_desc_s *desc);
void dp_tx_desc_release(struct dp_tx_desc_s *tx_desc, uint8_t desc_pool_id);
void dp_tx_compute_delay(struct dp_vdev *vdev, struct dp_tx_desc_s *tx_desc,
			 uint8_t tid, uint8_t ring_id);
void dp_tx_comp_process_tx_status(struct dp_soc *soc,
				  struct dp_tx_desc_s *tx_desc,
				  struct hal_tx_completion_status *ts,
				  struct dp_peer *peer, uint8_t ring_id);
void dp_tx_comp_process_desc(struct dp_soc *soc,
			     struct dp_tx_desc_s *desc,
			     struct hal_tx_completion_status *ts,
			     struct dp_peer *peer);
void dp_tx_reinject_handler(struct dp_soc *soc,
			    struct dp_vdev *vdev,
			    struct dp_tx_desc_s *tx_desc,
			    uint8_t *status);
void dp_tx_inspect_handler(struct dp_soc *soc,
			   struct dp_vdev *vdev,
			   struct dp_tx_desc_s *tx_desc,
			   uint8_t *status);
void dp_tx_update_peer_basic_stats(struct dp_peer *peer, uint32_t length,
				   uint8_t tx_status, bool update);

#ifndef QCA_HOST_MODE_WIFI_DISABLED
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

qdf_nbuf_t dp_tx_send(struct cdp_soc_t *soc, uint8_t vdev_id, qdf_nbuf_t nbuf);

qdf_nbuf_t dp_tx_send_vdev_id_check(struct cdp_soc_t *soc, uint8_t vdev_id,
				    qdf_nbuf_t nbuf);

qdf_nbuf_t dp_tx_send_exception(struct cdp_soc_t *soc, uint8_t vdev_id,
				qdf_nbuf_t nbuf,
				struct cdp_tx_exception_metadata *tx_exc);

qdf_nbuf_t dp_tx_send_exception_vdev_id_check(struct cdp_soc_t *soc,
					      uint8_t vdev_id,
					      qdf_nbuf_t nbuf,
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

QDF_STATUS
dp_tx_prepare_send_igmp_me(struct dp_vdev *vdev, qdf_nbuf_t nbuf);

#endif /* QCA_HOST_MODE_WIFI_DISABLED */

#if defined(QCA_HOST_MODE_WIFI_DISABLED) || !defined(ATH_SUPPORT_IQUE)
static inline void dp_tx_me_exit(struct dp_pdev *pdev)
{
	return;
}
#endif

/**
 * dp_tx_pdev_init() - dp tx pdev init
 * @pdev: physical device instance
 *
 * Return: QDF_STATUS_SUCCESS: success
 *         QDF_STATUS_E_RESOURCES: Error return
 */
static inline QDF_STATUS dp_tx_pdev_init(struct dp_pdev *pdev)
{
	struct dp_soc *soc = pdev->soc;

	/* Initialize Flow control counters */
	qdf_atomic_init(&pdev->num_tx_outstanding);
	pdev->tx_descs_max = 0;
	if (wlan_cfg_per_pdev_tx_ring(soc->wlan_cfg_ctx)) {
		/* Initialize descriptors in TCL Ring */
		hal_tx_init_data_ring(soc->hal_soc,
				soc->tcl_data_ring[pdev->pdev_id].hal_srng);
	}

	return QDF_STATUS_SUCCESS;
}


#ifndef FEATURE_WDS
static inline void dp_tx_mec_handler(struct dp_vdev *vdev, uint8_t *status)
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
 * dp_tx_hw_to_qdf()- convert hw status to qdf status
 * @status: hw status
 *
 * Return: qdf tx rx status
 */
static inline enum qdf_dp_tx_rx_status dp_tx_hw_to_qdf(uint16_t status)
{
	switch (status) {
	case HAL_TX_TQM_RR_FRAME_ACKED:
		return QDF_TX_RX_STATUS_OK;
	case HAL_TX_TQM_RR_REM_CMD_TX:
		return QDF_TX_RX_STATUS_NO_ACK;
	case HAL_TX_TQM_RR_REM_CMD_REM:
	case HAL_TX_TQM_RR_REM_CMD_NOTX:
	case HAL_TX_TQM_RR_REM_CMD_AGED:
		return QDF_TX_RX_STATUS_FW_DISCARD;
	default:
		return QDF_TX_RX_STATUS_DEFAULT;
	}
}

#ifndef QCA_HOST_MODE_WIFI_DISABLED
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
	queue->ring_id = qdf_get_cpu();

	dp_tx_debug("pool_id:%d ring_id: %d",
		    queue->desc_pool_id, queue->ring_id);

}

/*
 * dp_tx_get_hal_ring_hdl()- Get the hal_tx_ring_hdl for data transmission
 * @dp_soc - DP soc structure pointer
 * @ring_id - Transmit Queue/ring_id to be used when XPS is enabled
 *
 * Return - HAL ring handle
 */
static inline hal_ring_handle_t dp_tx_get_hal_ring_hdl(struct dp_soc *soc,
						       uint8_t ring_id)
{
	if (ring_id == soc->num_tcl_data_rings)
		return soc->tcl_cmd_credit_ring.hal_srng;

	return soc->tcl_data_ring[ring_id].hal_srng;
}

#else /* QCA_OL_TX_MULTIQ_SUPPORT */

#ifdef TX_MULTI_TCL
#ifdef IPA_OFFLOAD
static inline void dp_tx_get_queue(struct dp_vdev *vdev,
				   qdf_nbuf_t nbuf, struct dp_tx_queue *queue)
{
	/* get flow id */
	queue->desc_pool_id = DP_TX_GET_DESC_POOL_ID(vdev);
	if (vdev->pdev->soc->wlan_cfg_ctx->ipa_enabled)
		queue->ring_id = DP_TX_GET_RING_ID(vdev);
	else
		queue->ring_id = (qdf_nbuf_get_queue_mapping(nbuf) %
					vdev->pdev->soc->num_tcl_data_rings);
}
#else
static inline void dp_tx_get_queue(struct dp_vdev *vdev,
				   qdf_nbuf_t nbuf, struct dp_tx_queue *queue)
{
	/* get flow id */
	queue->desc_pool_id = DP_TX_GET_DESC_POOL_ID(vdev);
	queue->ring_id = (qdf_nbuf_get_queue_mapping(nbuf) %
				vdev->pdev->soc->num_tcl_data_rings);
}
#endif
#else
static inline void dp_tx_get_queue(struct dp_vdev *vdev,
				   qdf_nbuf_t nbuf, struct dp_tx_queue *queue)
{
	/* get flow id */
	queue->desc_pool_id = DP_TX_GET_DESC_POOL_ID(vdev);
	queue->ring_id = DP_TX_GET_RING_ID(vdev);
}
#endif

static inline hal_ring_handle_t dp_tx_get_hal_ring_hdl(struct dp_soc *soc,
						       uint8_t ring_id)
{
	return soc->tcl_data_ring[ring_id].hal_srng;
}
#endif

#ifdef QCA_OL_TX_LOCK_LESS_ACCESS
/*
 * dp_tx_hal_ring_access_start()- hal_tx_ring access for data transmission
 * @dp_soc - DP soc structure pointer
 * @hal_ring_hdl - HAL ring handle
 *
 * Return - None
 */
static inline int dp_tx_hal_ring_access_start(struct dp_soc *soc,
					      hal_ring_handle_t hal_ring_hdl)
{
	return hal_srng_access_start_unlocked(soc->hal_soc, hal_ring_hdl);
}

/*
 * dp_tx_hal_ring_access_end()- hal_tx_ring access for data transmission
 * @dp_soc - DP soc structure pointer
 * @hal_ring_hdl - HAL ring handle
 *
 * Return - None
 */
static inline void dp_tx_hal_ring_access_end(struct dp_soc *soc,
					     hal_ring_handle_t hal_ring_hdl)
{
	hal_srng_access_end_unlocked(soc->hal_soc, hal_ring_hdl);
}

/*
 * dp_tx_hal_ring_access_reap()- hal_tx_ring access for data transmission
 * @dp_soc - DP soc structure pointer
 * @hal_ring_hdl - HAL ring handle
 *
 * Return - None
 */
static inline void dp_tx_hal_ring_access_end_reap(struct dp_soc *soc,
						  hal_ring_handle_t
						  hal_ring_hdl)
{
}

#else
static inline int dp_tx_hal_ring_access_start(struct dp_soc *soc,
					      hal_ring_handle_t hal_ring_hdl)
{
	return hal_srng_access_start(soc->hal_soc, hal_ring_hdl);
}

static inline void dp_tx_hal_ring_access_end(struct dp_soc *soc,
					     hal_ring_handle_t hal_ring_hdl)
{
	hal_srng_access_end(soc->hal_soc, hal_ring_hdl);
}

static inline void dp_tx_hal_ring_access_end_reap(struct dp_soc *soc,
						  hal_ring_handle_t
						  hal_ring_hdl)
{
	hal_srng_access_end_reap(soc->hal_soc, hal_ring_hdl);
}
#endif

#ifdef ATH_TX_PRI_OVERRIDE
#define DP_TX_TID_OVERRIDE(_msdu_info, _nbuf) \
	((_msdu_info)->tid = qdf_nbuf_get_priority(_nbuf))
#else
#define DP_TX_TID_OVERRIDE(_msdu_info, _nbuf)
#endif

/* TODO TX_FEATURE_NOT_YET */
static inline void dp_tx_comp_process_exception(struct dp_tx_desc_s *tx_desc)
{
	return;
}
/* TODO TX_FEATURE_NOT_YET */

void dp_tx_desc_flush(struct dp_pdev *pdev, struct dp_vdev *vdev,
		      bool force_free);
QDF_STATUS dp_tx_vdev_attach(struct dp_vdev *vdev);
QDF_STATUS dp_tx_vdev_detach(struct dp_vdev *vdev);
void dp_tx_vdev_update_search_flags(struct dp_vdev *vdev);
QDF_STATUS dp_soc_tx_desc_sw_pools_alloc(struct dp_soc *soc);
QDF_STATUS dp_soc_tx_desc_sw_pools_init(struct dp_soc *soc);
void dp_soc_tx_desc_sw_pools_free(struct dp_soc *soc);
void dp_soc_tx_desc_sw_pools_deinit(struct dp_soc *soc);
void
dp_handle_wbm_internal_error(struct dp_soc *soc, void *hal_desc,
			     uint32_t buf_type);
#else /* QCA_HOST_MODE_WIFI_DISABLED */

static inline
QDF_STATUS dp_soc_tx_desc_sw_pools_alloc(struct dp_soc *soc)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS dp_soc_tx_desc_sw_pools_init(struct dp_soc *soc)
{
	return QDF_STATUS_SUCCESS;
}

static inline void dp_soc_tx_desc_sw_pools_free(struct dp_soc *soc)
{
}

static inline void dp_soc_tx_desc_sw_pools_deinit(struct dp_soc *soc)
{
}

static inline
void dp_tx_desc_flush(struct dp_pdev *pdev, struct dp_vdev *vdev,
		      bool force_free)
{
}

static inline QDF_STATUS dp_tx_vdev_attach(struct dp_vdev *vdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS dp_tx_vdev_detach(struct dp_vdev *vdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline void dp_tx_vdev_update_search_flags(struct dp_vdev *vdev)
{
}

#endif /* QCA_HOST_MODE_WIFI_DISABLED */

#ifdef FEATURE_PERPKT_INFO
QDF_STATUS
dp_get_completion_indication_for_stack(struct dp_soc *soc,
				       struct dp_pdev *pdev,
				       struct dp_peer *peer,
				       struct hal_tx_completion_status *ts,
				       qdf_nbuf_t netbuf,
				       uint64_t time_latency);

void dp_send_completion_to_stack(struct dp_soc *soc,  struct dp_pdev *pdev,
			    uint16_t peer_id, uint32_t ppdu_id,
			    qdf_nbuf_t netbuf);
#else
static inline
QDF_STATUS dp_get_completion_indication_for_stack(struct dp_soc *soc,
				       struct dp_pdev *pdev,
				       struct dp_peer *peer,
				       struct hal_tx_completion_status *ts,
				       qdf_nbuf_t netbuf,
				       uint64_t time_latency)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline
void dp_send_completion_to_stack(struct dp_soc *soc,  struct dp_pdev *pdev,
			    uint16_t peer_id, uint32_t ppdu_id,
			    qdf_nbuf_t netbuf)
{
}
#endif

#ifdef WLAN_FEATURE_PKT_CAPTURE_V2
void dp_send_completion_to_pkt_capture(struct dp_soc *soc,
				       struct dp_tx_desc_s *desc,
				       struct hal_tx_completion_status *ts);
#else
static inline void
dp_send_completion_to_pkt_capture(struct dp_soc *soc,
				  struct dp_tx_desc_s *desc,
				  struct hal_tx_completion_status *ts)
{
}
#endif

#ifndef QCA_HOST_MODE_WIFI_DISABLED
#ifdef WLAN_DP_FEATURE_SW_LATENCY_MGR
/**
 * dp_tx_update_stats() - Update soc level tx stats
 * @soc: DP soc handle
 * @tx_desc: TX descriptor reference
 * @ring_id: TCL ring id
 *
 * Returns: none
 */
void dp_tx_update_stats(struct dp_soc *soc,
			struct dp_tx_desc_s *tx_desc,
			uint8_t ring_id);

/**
 * dp_tx_attempt_coalescing() - Check and attempt TCL register write coalescing
 * @soc: Datapath soc handle
 * @tx_desc: tx packet descriptor
 * @tid: TID for pkt transmission
 * @msdu_info: MSDU info of tx packet
 * @ring_id: TCL ring id
 *
 * Returns: 1, if coalescing is to be done
 *	    0, if coalescing is not to be done
 */
int
dp_tx_attempt_coalescing(struct dp_soc *soc, struct dp_vdev *vdev,
			 struct dp_tx_desc_s *tx_desc,
			 uint8_t tid,
			 struct dp_tx_msdu_info_s *msdu_info,
			 uint8_t ring_id);

/**
 * dp_tx_ring_access_end() - HAL ring access end for data transmission
 * @soc: Datapath soc handle
 * @hal_ring_hdl: HAL ring handle
 * @coalesce: Coalesce the current write or not
 *
 * Returns: none
 */
void
dp_tx_ring_access_end(struct dp_soc *soc, hal_ring_handle_t hal_ring_hdl,
		      int coalesce);
#else
/**
 * dp_tx_update_stats() - Update soc level tx stats
 * @soc: DP soc handle
 * @tx_desc: TX descriptor reference
 * @ring_id: TCL ring id
 *
 * Returns: none
 */
static inline void dp_tx_update_stats(struct dp_soc *soc,
				      struct dp_tx_desc_s *tx_desc,
				      uint8_t ring_id){ }

static inline void
dp_tx_ring_access_end(struct dp_soc *soc, hal_ring_handle_t hal_ring_hdl,
		      int coalesce)
{
	dp_tx_hal_ring_access_end(soc, hal_ring_hdl);
}

static inline int
dp_tx_attempt_coalescing(struct dp_soc *soc, struct dp_vdev *vdev,
			 struct dp_tx_desc_s *tx_desc,
			 uint8_t tid,
			 struct dp_tx_msdu_info_s *msdu_info,
			 uint8_t ring_id)
{
	return 0;
}

#endif /* WLAN_DP_FEATURE_SW_LATENCY_MGR */

#ifdef FEATURE_RUNTIME_PM
/**
 * dp_set_rtpm_tput_policy_requirement() - Update RTPM throughput policy
 * @soc_hdl: DP soc handle
 * @is_high_tput: flag to indicate whether throughput is high
 *
 * Returns: none
 */
static inline
void dp_set_rtpm_tput_policy_requirement(struct cdp_soc_t *soc_hdl,
					 bool is_high_tput)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);

	qdf_atomic_set(&soc->rtpm_high_tput_flag, is_high_tput);
}

void
dp_tx_ring_access_end_wrapper(struct dp_soc *soc,
			      hal_ring_handle_t hal_ring_hdl,
			      int coalesce);
#else
#ifdef DP_POWER_SAVE
void
dp_tx_ring_access_end_wrapper(struct dp_soc *soc,
			      hal_ring_handle_t hal_ring_hdl,
			      int coalesce);
#else
static inline void
dp_tx_ring_access_end_wrapper(struct dp_soc *soc,
			      hal_ring_handle_t hal_ring_hdl,
			      int coalesce)
{
	dp_tx_ring_access_end(soc, hal_ring_hdl, coalesce);
}
#endif

static inline void
dp_set_rtpm_tput_policy_requirement(struct cdp_soc_t *soc_hdl,
				    bool is_high_tput)
{ }
#endif
#endif /* QCA_HOST_MODE_WIFI_DISABLED */

#ifdef DP_TX_HW_DESC_HISTORY
static inline void
dp_tx_hw_desc_update_evt(uint8_t *hal_tx_desc_cached,
			 hal_ring_handle_t hal_ring_hdl,
			 struct dp_soc *soc)
{
	struct dp_tx_hw_desc_evt *evt;
	uint64_t idx = 0;

	if (!soc->tx_hw_desc_history)
		return;

	idx = ++soc->tx_hw_desc_history->index;
	if (idx == DP_TX_HW_DESC_HIST_MAX)
		soc->tx_hw_desc_history->index = 0;
	idx = qdf_do_div_rem(idx, DP_TX_HW_DESC_HIST_MAX);

	evt = &soc->tx_hw_desc_history->entry[idx];
	qdf_mem_copy(evt->tcl_desc, hal_tx_desc_cached, HAL_TX_DESC_LEN_BYTES);
	evt->posted = qdf_get_log_timestamp();
	hal_get_sw_hptp(soc->hal_soc, hal_ring_hdl, &evt->tp, &evt->hp);
}
#else
static inline void
dp_tx_hw_desc_update_evt(uint8_t *hal_tx_desc_cached,
			 hal_ring_handle_t hal_ring_hdl,
			 struct dp_soc *soc)
{
}
#endif

#ifdef WLAN_FEATURE_TSF_UPLINK_DELAY
/**
 * dp_set_delta_tsf() - Set delta_tsf to dp_soc structure
 * @soc_hdl: cdp soc pointer
 * @vdev_id: vdev id
 * @delta_tsf: difference between TSF clock and qtimer
 *
 * Return: None
 */
void dp_set_delta_tsf(struct cdp_soc_t *soc_hdl, uint8_t vdev_id,
		      uint32_t delta_tsf);

/**
 * dp_set_tsf_report_ul_delay() - Enable or disable reporting uplink delay
 * @soc_hdl: cdp soc pointer
 * @vdev_id: vdev id
 * @enable: true to enable and false to disable
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_set_tsf_ul_delay_report(struct cdp_soc_t *soc_hdl,
				      uint8_t vdev_id, bool enable);

/**
 * dp_get_uplink_delay() - Get uplink delay value
 * @soc_hdl: cdp soc pointer
 * @vdev_id: vdev id
 * @val: pointer to save uplink delay value
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_get_uplink_delay(struct cdp_soc_t *soc_hdl, uint8_t vdev_id,
			       uint32_t *val);
#endif /* WLAN_FEATURE_TSF_UPLINK_TSF */

/**
 * dp_tx_pkt_tracepoints_enabled() - Get the state of tx pkt tracepoint
 *
 * Return: True if any tx pkt tracepoint is enabled else false
 */
static inline
bool dp_tx_pkt_tracepoints_enabled(void)
{
	return (qdf_trace_dp_tx_comp_tcp_pkt_enabled() ||
		qdf_trace_dp_tx_comp_udp_pkt_enabled() ||
		qdf_trace_dp_tx_comp_pkt_enabled());
}

#ifdef DP_TX_TRACKING
/**
 * dp_tx_desc_set_timestamp() - set timestamp in tx descriptor
 * @tx_desc - tx descriptor
 *
 * Return: None
 */
static inline
void dp_tx_desc_set_timestamp(struct dp_tx_desc_s *tx_desc)
{
	tx_desc->timestamp = qdf_system_ticks();
}

/**
 * dp_tx_desc_check_corruption() - Verify magic pattern in tx descriptor
 * @tx_desc: tx descriptor
 *
 * Check for corruption in tx descriptor, if magic pattern is not matching
 * trigger self recovery
 *
 * Return: none
 */
void dp_tx_desc_check_corruption(struct dp_tx_desc_s *tx_desc);
#else
static inline
void dp_tx_desc_set_timestamp(struct dp_tx_desc_s *tx_desc)
{
}

static inline
void dp_tx_desc_check_corruption(struct dp_tx_desc_s *tx_desc)
{
}
#endif

#ifdef HW_TX_DELAY_STATS_ENABLE
/**
 * dp_tx_desc_set_ktimestamp() - set kernel timestamp in tx descriptor
 * @vdev: DP vdev handle
 * @tx_desc: tx descriptor
 *
 * Return: true when descriptor is timestamped, false otherwise
 */
static inline
bool dp_tx_desc_set_ktimestamp(struct dp_vdev *vdev,
			       struct dp_tx_desc_s *tx_desc)
{
	if (qdf_unlikely(vdev->pdev->delay_stats_flag) ||
	    qdf_unlikely(vdev->pdev->soc->wlan_cfg_ctx->pext_stats_enabled) ||
	    qdf_unlikely(dp_tx_pkt_tracepoints_enabled()) ||
	    qdf_unlikely(vdev->pdev->soc->rdkstats_enabled) ||
	    qdf_unlikely(dp_is_vdev_tx_delay_stats_enabled(vdev))) {
		tx_desc->timestamp = qdf_ktime_to_ms(qdf_ktime_real_get());
		return true;
	}
	return false;
}
#else
static inline
bool dp_tx_desc_set_ktimestamp(struct dp_vdev *vdev,
			       struct dp_tx_desc_s *tx_desc)
{
	if (qdf_unlikely(vdev->pdev->delay_stats_flag) ||
	    qdf_unlikely(vdev->pdev->soc->wlan_cfg_ctx->pext_stats_enabled) ||
	    qdf_unlikely(dp_tx_pkt_tracepoints_enabled()) ||
	    qdf_unlikely(vdev->pdev->soc->rdkstats_enabled)) {
		tx_desc->timestamp = qdf_ktime_to_ms(qdf_ktime_real_get());
		return true;
	}
	return false;
}
#endif
#endif
