/*
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
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

#include <wlan_objmgr_pdev_obj.h>
#include <dp_txrx.h>
#include <dp_internal.h>
#include <cdp_txrx_cmn.h>
#include <cdp_txrx_misc.h>

QDF_STATUS dp_txrx_init(ol_txrx_soc_handle soc, uint8_t pdev_id,
			struct dp_txrx_config *config)
{
	struct dp_txrx_handle *dp_ext_hdl;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	uint8_t num_dp_rx_threads;
	struct dp_pdev *pdev;

	if (qdf_unlikely(!soc)) {
		dp_err("soc is NULL");
		return 0;
	}

	pdev = dp_get_pdev_from_soc_pdev_id_wifi3(cdp_soc_t_to_dp_soc(soc),
						  pdev_id);
	if (!pdev) {
		dp_err("pdev is NULL");
		return 0;
	}

	dp_ext_hdl = qdf_mem_malloc(sizeof(*dp_ext_hdl));
	if (!dp_ext_hdl) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_NOMEM;
	}

	dp_info("dp_txrx_handle allocated");
	dp_ext_hdl->soc = soc;
	dp_ext_hdl->pdev = dp_pdev_to_cdp_pdev(pdev);
	cdp_soc_set_dp_txrx_handle(soc, dp_ext_hdl);
	qdf_mem_copy(&dp_ext_hdl->config, config, sizeof(*config));
	dp_ext_hdl->rx_tm_hdl.txrx_handle_cmn =
				dp_txrx_get_cmn_hdl_frm_ext_hdl(dp_ext_hdl);

	num_dp_rx_threads = cdp_get_num_rx_contexts(soc);

	if (dp_ext_hdl->config.enable_rx_threads) {
		qdf_status = dp_rx_tm_init(&dp_ext_hdl->rx_tm_hdl,
					   num_dp_rx_threads);
	}

	return qdf_status;
}

QDF_STATUS dp_txrx_deinit(ol_txrx_soc_handle soc)
{
	struct dp_txrx_handle *dp_ext_hdl;

	if (!soc)
		return QDF_STATUS_E_INVAL;

	dp_ext_hdl = cdp_soc_get_dp_txrx_handle(soc);
	if (!dp_ext_hdl)
		return QDF_STATUS_E_FAULT;

	if (dp_ext_hdl->config.enable_rx_threads)
		dp_rx_tm_deinit(&dp_ext_hdl->rx_tm_hdl);

	qdf_mem_free(dp_ext_hdl);
	dp_info("dp_txrx_handle_t de-allocated");

	cdp_soc_set_dp_txrx_handle(soc, NULL);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_rx_tm_get_pending() - get number of frame in thread
 * nbuf queue pending
 * @soc: ol_txrx_soc_handle object
 *
 * Return: number of frames
 */
#ifdef FEATURE_WLAN_DP_RX_THREADS
int dp_rx_tm_get_pending(ol_txrx_soc_handle soc)
{
	int i;
	int num_pending = 0;
	struct dp_rx_thread *rx_thread;
	struct dp_txrx_handle *dp_ext_hdl;
	struct dp_rx_tm_handle *rx_tm_hdl;

	if (!soc)
		return 0;

	dp_ext_hdl = cdp_soc_get_dp_txrx_handle(soc);
	if (!dp_ext_hdl)
		return 0;

	rx_tm_hdl = &dp_ext_hdl->rx_tm_hdl;

	for (i = 0; i < rx_tm_hdl->num_dp_rx_threads; i++) {
		rx_thread = rx_tm_hdl->rx_thread[i];
		if (!rx_thread)
			continue;
		num_pending += qdf_nbuf_queue_head_qlen(&rx_thread->nbuf_queue);
	}

	if (num_pending)
		dp_debug("pending frames in thread queue %d", num_pending);

	return num_pending;
}
#else
int dp_rx_tm_get_pending(ol_txrx_soc_handle soc)
{
	return 0;
}
#endif

#ifdef DP_MEM_PRE_ALLOC
/* Num elements in REO ring */
#define REO_DST_RING_SIZE 1024

/* Num elements in TCL Data ring */
#define TCL_DATA_RING_SIZE 3072

/* Num elements in WBM2SW ring */
#define WBM2SW_RELEASE_RING_SIZE 4096

/* Num elements in WBM Idle Link */
#define WBM_IDLE_LINK_RING_SIZE (32 * 1024)

/**
 * struct dp_consistent_prealloc - element representing DP pre-alloc memory
 * @ring_type: HAL ring type
 * @size: size of pre-alloc memory
 * @in_use: whether this element is in use (occupied)
 * @va_unaligned: Unaligned virtual address
 * @va_aligned: aligned virtual address.
 * @pa_unaligned: Unaligned physical address.
 * @pa_aligned: Aligned physical address.
 */

struct dp_consistent_prealloc {
	enum hal_ring_type ring_type;
	uint32_t size;
	uint8_t in_use;
	void *va_unaligned;
	void *va_aligned;
	qdf_dma_addr_t pa_unaligned;
	qdf_dma_addr_t pa_aligned;
};

static struct  dp_consistent_prealloc g_dp_consistent_allocs[] = {
	/* 5 REO DST rings */
	{REO_DST, (sizeof(struct reo_destination_ring)) * REO_DST_RING_SIZE, 0, NULL, NULL, 0, 0},
	{REO_DST, (sizeof(struct reo_destination_ring)) * REO_DST_RING_SIZE, 0, NULL, NULL, 0, 0},
	{REO_DST, (sizeof(struct reo_destination_ring)) * REO_DST_RING_SIZE, 0, NULL, NULL, 0, 0},
	{REO_DST, (sizeof(struct reo_destination_ring)) * REO_DST_RING_SIZE, 0, NULL, NULL, 0, 0},
	{REO_DST, (sizeof(struct reo_destination_ring)) * REO_DST_RING_SIZE, 0, NULL, NULL, 0, 0},
	/* 3 TCL data rings */
	{TCL_DATA, (sizeof(struct tlv_32_hdr) + sizeof(struct tcl_data_cmd)) * TCL_DATA_RING_SIZE, 0, NULL, NULL, 0, 0},
	{TCL_DATA, (sizeof(struct tlv_32_hdr) + sizeof(struct tcl_data_cmd)) * TCL_DATA_RING_SIZE, 0, NULL, NULL, 0, 0},
	{TCL_DATA, (sizeof(struct tlv_32_hdr) + sizeof(struct tcl_data_cmd)) * TCL_DATA_RING_SIZE, 0, NULL, NULL, 0, 0},
	/* 4 WBM2SW rings */
	{WBM2SW_RELEASE, (sizeof(struct wbm_release_ring)) * WBM2SW_RELEASE_RING_SIZE, 0, NULL, NULL, 0, 0},
	{WBM2SW_RELEASE, (sizeof(struct wbm_release_ring)) * WBM2SW_RELEASE_RING_SIZE, 0, NULL, NULL, 0, 0},
	{WBM2SW_RELEASE, (sizeof(struct wbm_release_ring)) * WBM2SW_RELEASE_RING_SIZE, 0, NULL, NULL, 0, 0},
	{WBM2SW_RELEASE, (sizeof(struct wbm_release_ring)) * WBM2SW_RELEASE_RING_SIZE, 0, NULL, 0, 0},
	/* 1 WBM idle link desc ring */
	{WBM_IDLE_LINK, (sizeof(struct wbm_link_descriptor_ring)) * WBM_IDLE_LINK_RING_SIZE, 0, NULL, NULL, 0, 0},
};

void dp_prealloc_deinit(void)
{
	int i;
	struct dp_consistent_prealloc *p;
	qdf_device_t qdf_ctx = cds_get_context(QDF_MODULE_ID_QDF_DEVICE);

	if (!qdf_ctx) {
		dp_warn("qdf_ctx is NULL");
		return;
	}

	for (i = 0; i < QDF_ARRAY_SIZE(g_dp_consistent_allocs); i++) {
		p = &g_dp_consistent_allocs[i];

		if (p->in_use)
			dp_warn("i %d: in use while free", i);

		if (p->va_aligned) {
			dp_debug("i %d: va aligned %pK pa aligned %llx size %d",
				i, p->va_aligned, p->pa_aligned, p->size);
			qdf_mem_free_consistent(qdf_ctx, qdf_ctx->dev,
						p->size,
						p->va_unaligned,
						p->pa_unaligned, 0);
			qdf_mem_zero(p, sizeof(*p));
		}
	}
}

QDF_STATUS dp_prealloc_init(void)
{
	int i;
	struct dp_consistent_prealloc *p;
	qdf_device_t qdf_ctx = cds_get_context(QDF_MODULE_ID_QDF_DEVICE);

	if (!qdf_ctx) {
		dp_err("qdf_ctx is NULL");
		QDF_BUG(0);
		return QDF_STATUS_E_FAILURE;
	}

	for (i = 0; i < QDF_ARRAY_SIZE(g_dp_consistent_allocs); i++) {
		p = &g_dp_consistent_allocs[i];
		p->in_use = 0;
		p->va_aligned =
			qdf_aligned_mem_alloc_consistent(qdf_ctx,
							 &p->size,
							 &p->va_unaligned,
							 &p->pa_unaligned,
							 &p->pa_aligned,
							 DP_RING_BASE_ALIGN);
		if (!p->va_unaligned) {
			dp_warn("i %d: unable to preallocate %d bytes memory!",
				i, p->size);
			break;
		}

		dp_debug("i %d: va aligned %pK pa aligned %llx size %d", i,
			p->va_aligned, p->pa_aligned, p->size);
	}

	if (i != QDF_ARRAY_SIZE(g_dp_consistent_allocs)) {
		dp_err("unable to allocate memory!");
		dp_prealloc_deinit();
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

void *dp_prealloc_get_coherent(uint32_t *size, void **base_vaddr_unaligned,
			       qdf_dma_addr_t *paddr_unaligned,
			       qdf_dma_addr_t *paddr_aligned,
			       uint32_t align,
			       uint32_t ring_type)
{
	int i;
	struct dp_consistent_prealloc *p;
	void *va_aligned = NULL;

	for (i = 0; i < QDF_ARRAY_SIZE(g_dp_consistent_allocs); i++) {
		p = &g_dp_consistent_allocs[i];
		if (p->ring_type == ring_type && !p->in_use &&
		    p->va_unaligned && *size <= p->size) {
			p->in_use = 1;
			*base_vaddr_unaligned = p->va_unaligned;
			*paddr_unaligned = p->pa_unaligned;
			*paddr_aligned = p->pa_aligned;
			va_aligned = p->va_aligned;
			*size = p->size;
			dp_debug("index %i -> ring type %s va-aligned %pK", i,
				dp_srng_get_str_from_hal_ring_type(ring_type),
				va_aligned);
			break;
		}
	}

	if (i == QDF_ARRAY_SIZE(g_dp_consistent_allocs))
		dp_err("unable to allocate memory for ring type %s (%d) size %d",
			dp_srng_get_str_from_hal_ring_type(ring_type),
			ring_type, p->size);
	return va_aligned;
}

void dp_prealloc_put_coherent(qdf_size_t size, void *vaddr_unligned,
			      qdf_dma_addr_t paddr)
{
	int i;
	struct dp_consistent_prealloc *p;

	for (i = 0; i < QDF_ARRAY_SIZE(g_dp_consistent_allocs); i++) {
		p = &g_dp_consistent_allocs[i];
		if (p->va_unaligned == vaddr_unligned) {
			dp_debug("index %d, returned", i);
			p->in_use = 0;
			qdf_mem_zero(p->va_unaligned, p->size);
			break;
		}
	}

	if (i == QDF_ARRAY_SIZE(g_dp_consistent_allocs))
		dp_err("unable to find vaddr %pK", vaddr_unligned);
}
#endif
