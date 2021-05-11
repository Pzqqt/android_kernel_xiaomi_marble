/*
 * Copyright (c) 2016-2021, The Linux Foundation. All rights reserved.

 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.

 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <dp_types.h>
#include "dp_rx.h"
#include "dp_peer.h"
#include <dp_htt.h>
#include <dp_rx_mon.h>
#include <dp_mon_filter.h>
#include <dp_mon.h>

#define RNG_ERR		"SRNG setup failed for"
#define mon_init_err(params...) QDF_TRACE_ERROR(QDF_MODULE_ID_DP_INIT, params)

QDF_STATUS dp_srng_alloc(struct dp_soc *soc, struct dp_srng *srng,
			 int ring_type, uint32_t num_entries,
			 bool cached);
void dp_srng_free(struct dp_soc *soc, struct dp_srng *srng);
QDF_STATUS dp_srng_init(struct dp_soc *soc, struct dp_srng *srng,
			int ring_type, int ring_num, int mac_id);
void dp_srng_deinit(struct dp_soc *soc, struct dp_srng *srng,
		    int ring_type, int ring_num);

QDF_STATUS dp_htt_ppdu_stats_attach(struct dp_pdev *pdev);
void dp_htt_ppdu_stats_detach(struct dp_pdev *pdev);
void dp_neighbour_peers_detach(struct dp_pdev *pdev);
void dp_pktlogmod_exit(struct dp_pdev *handle);

#if !defined(DISABLE_MON_CONFIG)
/**
 * dp_mon_rings_deinit() - Deinitialize monitor rings
 * @pdev: DP pdev handle
 *
 * Return: None
 *
 */
static void dp_mon_rings_deinit(struct dp_pdev *pdev)
{
	int mac_id = 0;
	struct wlan_cfg_dp_pdev_ctxt *pdev_cfg_ctx;
	struct dp_soc *soc = pdev->soc;

	pdev_cfg_ctx = pdev->wlan_cfg_ctx;

	for (mac_id = 0; mac_id < NUM_RXDMA_RINGS_PER_PDEV; mac_id++) {
		int lmac_id = dp_get_lmac_id_for_pdev_id(soc, mac_id,
							 pdev->pdev_id);

		dp_srng_deinit(soc, &soc->rxdma_mon_status_ring[lmac_id],
			       RXDMA_MONITOR_STATUS, 0);

		if (!soc->wlan_cfg_ctx->rxdma1_enable)
			continue;

		dp_srng_deinit(soc, &soc->rxdma_mon_buf_ring[lmac_id],
			       RXDMA_MONITOR_BUF, 0);
		dp_srng_deinit(soc, &soc->rxdma_mon_dst_ring[lmac_id],
			       RXDMA_MONITOR_DST, 0);
		dp_srng_deinit(soc, &soc->rxdma_mon_desc_ring[lmac_id],
			       RXDMA_MONITOR_DESC, 0);
	}
}

/**
 * dp_mon_rings_free() - free monitor rings
 * @pdev: Datapath pdev handle
 *
 * Return: None
 *
 */
static void dp_mon_rings_free(struct dp_pdev *pdev)
{
	int mac_id = 0;
	struct wlan_cfg_dp_pdev_ctxt *pdev_cfg_ctx;
	struct dp_soc *soc = pdev->soc;

	pdev_cfg_ctx = pdev->wlan_cfg_ctx;

	for (mac_id = 0; mac_id < NUM_RXDMA_RINGS_PER_PDEV; mac_id++) {
		int lmac_id = dp_get_lmac_id_for_pdev_id(soc, mac_id,
							 pdev->pdev_id);

		dp_srng_free(soc, &soc->rxdma_mon_status_ring[lmac_id]);

		if (!soc->wlan_cfg_ctx->rxdma1_enable)
			continue;

		dp_srng_free(soc, &soc->rxdma_mon_buf_ring[lmac_id]);
		dp_srng_free(soc, &soc->rxdma_mon_dst_ring[lmac_id]);
		dp_srng_free(soc, &soc->rxdma_mon_desc_ring[lmac_id]);
	}
}

/**
 * dp_mon_rings_init() - Initialize monitor srng rings
 * @pdev: Datapath pdev handle
 *
 * return: QDF_STATUS_SUCCESS on success
 *	   QDF_STATUS_E_NOMEM on failure
 */
static
QDF_STATUS dp_mon_rings_init(struct dp_soc *soc, struct dp_pdev *pdev)
{
	int mac_id = 0;
	struct wlan_cfg_dp_pdev_ctxt *pdev_cfg_ctx;

	pdev_cfg_ctx = pdev->wlan_cfg_ctx;

	for (mac_id = 0; mac_id < NUM_RXDMA_RINGS_PER_PDEV; mac_id++) {
		int lmac_id = dp_get_lmac_id_for_pdev_id(soc, mac_id,
							 pdev->pdev_id);

		if (dp_srng_init(soc, &soc->rxdma_mon_status_ring[lmac_id],
				 RXDMA_MONITOR_STATUS, 0, lmac_id)) {
			mon_init_err("%pK: " RNG_ERR "rxdma_mon_status_ring",
				     soc);
			goto fail1;
		}

		if (!soc->wlan_cfg_ctx->rxdma1_enable)
			continue;

		if (dp_srng_init(soc, &soc->rxdma_mon_buf_ring[lmac_id],
				 RXDMA_MONITOR_BUF, 0, lmac_id)) {
			mon_init_err("%pK: " RNG_ERR "rxdma_mon_buf_ring ",
				     soc);
			goto fail1;
		}

		if (dp_srng_init(soc, &soc->rxdma_mon_dst_ring[lmac_id],
				 RXDMA_MONITOR_DST, 0, lmac_id)) {
			mon_init_err("%pK: " RNG_ERR "rxdma_mon_dst_ring", soc);
			goto fail1;
		}

		if (dp_srng_init(soc, &soc->rxdma_mon_desc_ring[lmac_id],
				 RXDMA_MONITOR_DESC, 0, lmac_id)) {
			mon_init_err("%pK: " RNG_ERR "rxdma_mon_desc_ring",
				     soc);
			goto fail1;
		}
	}
	return QDF_STATUS_SUCCESS;

fail1:
	dp_mon_rings_deinit(pdev);
	return QDF_STATUS_E_NOMEM;
}

/**
 * dp_mon_rings_alloc() - Allocate memory for monitor srng rings
 * @soc: Datapath soc handle
 * @pdev: Datapath pdev handle
 *
 * return: QDF_STATUS_SUCCESS on success
 *	   QDF_STATUS_E_NOMEM on failure
 */
static
QDF_STATUS dp_mon_rings_alloc(struct dp_soc *soc, struct dp_pdev *pdev)
{
	int mac_id = 0;
	int entries;
	struct wlan_cfg_dp_pdev_ctxt *pdev_cfg_ctx;

	pdev_cfg_ctx = pdev->wlan_cfg_ctx;

	for (mac_id = 0; mac_id < NUM_RXDMA_RINGS_PER_PDEV; mac_id++) {
		int lmac_id =
		dp_get_lmac_id_for_pdev_id(soc, mac_id, pdev->pdev_id);
		entries = wlan_cfg_get_dma_mon_stat_ring_size(pdev_cfg_ctx);
		if (dp_srng_alloc(soc, &soc->rxdma_mon_status_ring[lmac_id],
				  RXDMA_MONITOR_STATUS, entries, 0)) {
			mon_init_err("%pK: " RNG_ERR "rxdma_mon_status_ring",
				     soc);
			goto fail1;
		}

		if (!soc->wlan_cfg_ctx->rxdma1_enable)
			continue;

		entries = wlan_cfg_get_dma_mon_buf_ring_size(pdev_cfg_ctx);
		if (dp_srng_alloc(soc, &soc->rxdma_mon_buf_ring[lmac_id],
				  RXDMA_MONITOR_BUF, entries, 0)) {
			mon_init_err("%pK: " RNG_ERR "rxdma_mon_buf_ring ",
				     soc);
			goto fail1;
		}

		entries = wlan_cfg_get_dma_mon_dest_ring_size(pdev_cfg_ctx);
		if (dp_srng_alloc(soc, &soc->rxdma_mon_dst_ring[lmac_id],
				  RXDMA_MONITOR_DST, entries, 0)) {
			mon_init_err("%pK: " RNG_ERR "rxdma_mon_dst_ring", soc);
			goto fail1;
		}

		entries = wlan_cfg_get_dma_mon_desc_ring_size(pdev_cfg_ctx);
		if (dp_srng_alloc(soc, &soc->rxdma_mon_desc_ring[lmac_id],
				  RXDMA_MONITOR_DESC, entries, 0)) {
			mon_init_err("%pK: " RNG_ERR "rxdma_mon_desc_ring",
				     soc);
			goto fail1;
		}
	}
	return QDF_STATUS_SUCCESS;

fail1:
	dp_mon_rings_free(pdev);
	return QDF_STATUS_E_NOMEM;
}
#else
static void dp_mon_rings_free(struct dp_pdev *pdev)
{
}

static void dp_mon_rings_deinit(struct dp_pdev *pdev)
{
}

static
QDF_STATUS dp_mon_rings_init(struct dp_soc *soc, struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static
QDF_STATUS dp_mon_rings_alloc(struct dp_soc *soc, struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}
#endif

QDF_STATUS dp_mon_soc_cfg_init(struct dp_soc *soc)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dp_mon_pdev_attach(struct dp_pdev *pdev)
{
	struct dp_soc *soc = pdev->soc;
	struct dp_mon_pdev *mon_pdev;

	mon_pdev = (struct dp_mon_pdev *)qdf_mem_malloc(sizeof(*mon_pdev));
	if (!mon_pdev) {
		mon_init_err("%pK: MONITOR pdev allocation failed", pdev);
		goto fail0;
	}

	if (dp_mon_rings_alloc(soc, pdev)) {
		mon_init_err("%pK: MONITOR rings setup failed", soc);
		goto fail1;
	}

	/* Rx monitor mode specific init */
	if (dp_rx_pdev_mon_desc_pool_alloc(pdev)) {
		mon_init_err("%pK: dp_rx_pdev_mon_attach failed", soc);
		goto fail2;
	}

	pdev->monitor_pdev = mon_pdev;

	return QDF_STATUS_SUCCESS;
fail2:
	dp_mon_rings_free(pdev);
fail1:
	pdev->monitor_pdev = NULL;
	qdf_mem_free(mon_pdev);
fail0:
	return QDF_STATUS_E_NOMEM;
}

QDF_STATUS dp_mon_pdev_detach(struct dp_pdev *pdev)
{
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;

	dp_rx_pdev_mon_desc_pool_free(pdev);
	dp_mon_rings_free(pdev);
	pdev->monitor_pdev = NULL;
	qdf_mem_free(mon_pdev);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dp_mon_pdev_init(struct dp_pdev *pdev)
{
	struct dp_soc *soc = pdev->soc;

	pdev->filter = dp_mon_filter_alloc(pdev);
	if (!pdev->filter) {
		mon_init_err("%pK: Memory allocation failed for monitor filter",
			     soc);
		return QDF_STATUS_E_NOMEM;
	}

	qdf_spinlock_create(&pdev->ppdu_stats_lock);
	qdf_spinlock_create(&pdev->neighbour_peer_mutex);
	pdev->monitor_configured = false;
	pdev->mon_chan_band = REG_BAND_UNKNOWN;
	/* Monitor filter init */
	pdev->mon_filter_mode = MON_FILTER_ALL;

	TAILQ_INIT(&pdev->neighbour_peers_list);
	pdev->neighbour_peers_added = false;
	pdev->monitor_configured = false;
	/* Monitor filter init */
	pdev->mon_filter_mode = MON_FILTER_ALL;
	pdev->fp_mgmt_filter = FILTER_MGMT_ALL;
	pdev->fp_ctrl_filter = FILTER_CTRL_ALL;
	pdev->fp_data_filter = FILTER_DATA_ALL;
	pdev->mo_mgmt_filter = FILTER_MGMT_ALL;
	pdev->mo_ctrl_filter = FILTER_CTRL_ALL;
	pdev->mo_data_filter = FILTER_DATA_ALL;

	if (dp_htt_ppdu_stats_attach(pdev) != QDF_STATUS_SUCCESS)
		goto fail0;

	if (dp_mon_rings_init(soc, pdev)) {
		mon_init_err("%pK: MONITOR rings setup failed", soc);
		goto fail1;
	}
	/* initialize sw monitor rx descriptors */
	dp_rx_pdev_mon_desc_pool_init(pdev);
	/* allocate buffers and replenish the monitor RxDMA ring */
	dp_rx_pdev_mon_buffers_alloc(pdev);
	dp_tx_ppdu_stats_attach(pdev);

	return QDF_STATUS_SUCCESS;
fail1:
	dp_htt_ppdu_stats_detach(pdev);
fail0:
	qdf_spinlock_destroy(&pdev->neighbour_peer_mutex);
	qdf_spinlock_destroy(&pdev->ppdu_stats_lock);
	dp_mon_filter_dealloc(pdev);
	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS dp_mon_pdev_deinit(struct dp_pdev *pdev)
{
	dp_tx_ppdu_stats_detach(pdev);
	dp_rx_pdev_mon_buffers_free(pdev);
	dp_rx_pdev_mon_desc_pool_deinit(pdev);
	dp_mon_rings_deinit(pdev);
	dp_htt_ppdu_stats_detach(pdev);
	qdf_spinlock_destroy(&pdev->ppdu_stats_lock);
	dp_neighbour_peers_detach(pdev);
	dp_pktlogmod_exit(pdev);

	if (pdev->filter)
		dp_mon_filter_dealloc(pdev);

	return QDF_STATUS_SUCCESS;
}

static struct dp_mon_ops monitor_ops = {
	.mon_soc_cfg_init = dp_mon_soc_cfg_init,
	.mon_pdev_attach = dp_mon_pdev_attach,
	.mon_pdev_detach = dp_mon_pdev_detach,
	.mon_pdev_init = dp_mon_pdev_init,
	.mon_pdev_deinit = dp_mon_pdev_deinit,
};

static inline void dp_mon_ops_register(struct dp_mon_soc *mon_soc)
{
	mon_soc->mon_ops = &monitor_ops;
}

QDF_STATUS dp_mon_soc_attach(struct dp_soc *soc)
{
	struct dp_mon_soc *mon_soc;

	mon_soc = (struct dp_mon_soc *)qdf_mem_malloc(sizeof(*mon_soc));
	if (!mon_soc) {
		mon_init_err("%pK: mem allocation failed", soc);
		return QDF_STATUS_E_NOMEM;
	}
	/* register monitor ops */
	dp_mon_ops_register(mon_soc);
	soc->monitor_soc = mon_soc;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dp_mon_soc_detach(struct dp_soc *soc)
{
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

	soc->monitor_soc = NULL;
	qdf_mem_free(mon_soc);
	return QDF_STATUS_SUCCESS;
}
