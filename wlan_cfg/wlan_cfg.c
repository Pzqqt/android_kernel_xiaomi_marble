/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
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

#if defined(CONFIG_HL_SUPPORT)
#include "wlan_tgt_def_config_hl.h"
#else
#include "wlan_tgt_def_config.h"
#endif

#include "qdf_trace.h"
#include "qdf_mem.h"
#include <cdp_txrx_ops.h>
#include "wlan_cfg.h"
#include "cfg_ucfg_api.h"
#include "hal_api.h"

/*
 * FIX THIS -
 * For now, all these configuration parameters are hardcoded.
 * Many of these should actually be coming from dts file/ini file
 */

/*
 * The max allowed size for tx comp ring is 8191.
 * This is limitted by h/w ring max size.
 * As this is not a power of 2 it does not work with nss offload so the
 * nearest available size which is power of 2 is 4096 chosen for nss
 */

#define WLAN_CFG_TX_RING_MASK_0 0x1
#define WLAN_CFG_TX_RING_MASK_1 0x2
#define WLAN_CFG_TX_RING_MASK_2 0x4
#define WLAN_CFG_TX_RING_MASK_3 0x0

#define WLAN_CFG_RX_RING_MASK_0 0x1
#define WLAN_CFG_RX_RING_MASK_1 0x2
#define WLAN_CFG_RX_RING_MASK_2 0x4
#define WLAN_CFG_RX_RING_MASK_3 0x8

#define WLAN_CFG_RX_MON_RING_MASK_0 0x1
#define WLAN_CFG_RX_MON_RING_MASK_1 0x2
#define WLAN_CFG_RX_MON_RING_MASK_2 0x4
#define WLAN_CFG_RX_MON_RING_MASK_3 0x0

#define WLAN_CFG_HOST2RXDMA_MON_RING_MASK_0 0x1
#define WLAN_CFG_HOST2RXDMA_MON_RING_MASK_1 0x2
#define WLAN_CFG_HOST2RXDMA_MON_RING_MASK_2 0x4

#define WLAN_CFG_RXDMA2HOST_MON_RING_MASK_0 0x1
#define WLAN_CFG_RXDMA2HOST_MON_RING_MASK_1 0x2
#define WLAN_CFG_RXDMA2HOST_MON_RING_MASK_2 0x4

#define WLAN_CFG_RX_ERR_RING_MASK_0 0x1
#define WLAN_CFG_RX_ERR_RING_MASK_1 0x0
#define WLAN_CFG_RX_ERR_RING_MASK_2 0x0
#define WLAN_CFG_RX_ERR_RING_MASK_3 0x0

#define WLAN_CFG_RX_WBM_REL_RING_MASK_0 0x1
#define WLAN_CFG_RX_WBM_REL_RING_MASK_1 0x0
#define WLAN_CFG_RX_WBM_REL_RING_MASK_2 0x0
#define WLAN_CFG_RX_WBM_REL_RING_MASK_3 0x0

#define WLAN_CFG_REO_STATUS_RING_MASK_0 0x1
#define WLAN_CFG_REO_STATUS_RING_MASK_1 0x0
#define WLAN_CFG_REO_STATUS_RING_MASK_2 0x0
#define WLAN_CFG_REO_STATUS_RING_MASK_3 0x0

#define WLAN_CFG_RXDMA2HOST_RING_MASK_0 0x1
#define WLAN_CFG_RXDMA2HOST_RING_MASK_1 0x2
#define WLAN_CFG_RXDMA2HOST_RING_MASK_2 0x4
#define WLAN_CFG_RXDMA2HOST_RING_MASK_3 0x0

#define WLAN_CFG_HOST2RXDMA_RING_MASK_0 0x1
#define WLAN_CFG_HOST2RXDMA_RING_MASK_1 0x2
#define WLAN_CFG_HOST2RXDMA_RING_MASK_2 0x4
#define WLAN_CFG_HOST2RXDMA_RING_MASK_3 0x0

#ifdef CONFIG_MCL
static const int tx_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS] = {
	WLAN_CFG_TX_RING_MASK_0, 0, 0, 0, 0, 0, 0};

#ifndef IPA_OFFLOAD
static const int rx_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0, WLAN_CFG_RX_RING_MASK_0, WLAN_CFG_RX_RING_MASK_1, WLAN_CFG_RX_RING_MASK_2, WLAN_CFG_RX_RING_MASK_3, 0, 0};
#else
static const int rx_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0, WLAN_CFG_RX_RING_MASK_0, WLAN_CFG_RX_RING_MASK_1, WLAN_CFG_RX_RING_MASK_2, 0, 0, 0};
#endif

static const int rx_mon_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0, WLAN_CFG_RX_MON_RING_MASK_0, WLAN_CFG_RX_MON_RING_MASK_1, 0, 0, 0, 0};

static const int host2rxdma_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0, 0, 0, 0, 0, 0, 0};

static const int rxdma2host_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0, 0, 0, 0, 0, WLAN_CFG_RXDMA2HOST_RING_MASK_0, WLAN_CFG_RXDMA2HOST_RING_MASK_1};

static const int host2rxdma_mon_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0, 0, 0, 0, 0, 0, 0};

static const int rxdma2host_mon_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0, 0, 0, 0, 0, 0, 0};

static const int rx_err_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0, 0, 0, 0, 0, 0, WLAN_CFG_RX_ERR_RING_MASK_0};

static const int rx_wbm_rel_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0, 0, 0, 0, 0, 0, WLAN_CFG_RX_WBM_REL_RING_MASK_0};

static const int reo_status_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0, 0, 0, 0, 0, 0, WLAN_CFG_REO_STATUS_RING_MASK_0};
#else
static const int tx_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS] = {
						WLAN_CFG_TX_RING_MASK_0,
						WLAN_CFG_TX_RING_MASK_1,
						WLAN_CFG_TX_RING_MASK_2,
						WLAN_CFG_TX_RING_MASK_3};

static const int rx_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS] = {
					0,
					0,
					0,
					0,
					0,
					0,
					0,
					WLAN_CFG_RX_RING_MASK_0,
					WLAN_CFG_RX_RING_MASK_1,
					WLAN_CFG_RX_RING_MASK_2,
					WLAN_CFG_RX_RING_MASK_3};

static const int rx_mon_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS] = {
					0,
					0,
					0,
					0,
					WLAN_CFG_RX_MON_RING_MASK_0,
					WLAN_CFG_RX_MON_RING_MASK_1,
					WLAN_CFG_RX_MON_RING_MASK_2};

static const int host2rxdma_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS] = {
					WLAN_CFG_HOST2RXDMA_RING_MASK_0,
					WLAN_CFG_HOST2RXDMA_RING_MASK_1,
					WLAN_CFG_HOST2RXDMA_RING_MASK_2,
					WLAN_CFG_HOST2RXDMA_RING_MASK_3};

static const int rxdma2host_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS] = {
					WLAN_CFG_RXDMA2HOST_RING_MASK_0,
					WLAN_CFG_RXDMA2HOST_RING_MASK_1,
					WLAN_CFG_RXDMA2HOST_RING_MASK_2,
					WLAN_CFG_RXDMA2HOST_RING_MASK_3};

static const int host2rxdma_mon_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS] = {
					0,
					0,
					0,
					0,
					WLAN_CFG_HOST2RXDMA_MON_RING_MASK_0,
					WLAN_CFG_HOST2RXDMA_MON_RING_MASK_1,
					WLAN_CFG_HOST2RXDMA_MON_RING_MASK_2};

static const int rxdma2host_mon_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS] = {
					0,
					0,
					0,
					0,
					WLAN_CFG_RXDMA2HOST_MON_RING_MASK_0,
					WLAN_CFG_RXDMA2HOST_MON_RING_MASK_1,
					WLAN_CFG_RXDMA2HOST_MON_RING_MASK_2};

static const int rx_err_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS] = {
					WLAN_CFG_RX_ERR_RING_MASK_0,
					WLAN_CFG_RX_ERR_RING_MASK_1,
					WLAN_CFG_RX_ERR_RING_MASK_2,
					WLAN_CFG_RX_ERR_RING_MASK_3};

static const int rx_wbm_rel_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS] = {
					WLAN_CFG_RX_WBM_REL_RING_MASK_0,
					WLAN_CFG_RX_WBM_REL_RING_MASK_1,
					WLAN_CFG_RX_WBM_REL_RING_MASK_2,
					WLAN_CFG_RX_WBM_REL_RING_MASK_3};

static const int reo_status_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS] = {
					WLAN_CFG_REO_STATUS_RING_MASK_0,
					WLAN_CFG_REO_STATUS_RING_MASK_1,
					WLAN_CFG_REO_STATUS_RING_MASK_2,
					WLAN_CFG_REO_STATUS_RING_MASK_3};
#endif /*CONFIG_MCL*/

/**
 * g_wlan_srng_cfg[] - Per ring_type specific configuration
 *
 */
struct wlan_srng_cfg g_wlan_srng_cfg[MAX_RING_TYPES];

/* REO_DST ring configuration */
struct wlan_srng_cfg wlan_srng_reo_cfg = {
	.timer_threshold = WLAN_CFG_INT_TIMER_THRESHOLD_REO_RING,
	.batch_count_threshold = 0,
	.low_threshold = 0,
};

/* WBM2SW_RELEASE ring configuration */
struct wlan_srng_cfg wlan_srng_wbm_release_cfg = {
	.timer_threshold = WLAN_CFG_INT_TIMER_THRESHOLD_WBM_RELEASE_RING,
	.batch_count_threshold = 0,
	.low_threshold = 0,
};

/* RXDMA_BUF ring configuration */
struct wlan_srng_cfg wlan_srng_rxdma_buf_cfg = {
	.timer_threshold = WLAN_CFG_INT_TIMER_THRESHOLD_RX,
	.batch_count_threshold = 0,
	.low_threshold = WLAN_CFG_RXDMA_REFILL_RING_SIZE >> 3,
};

/* RXDMA_MONITOR_BUF ring configuration */
struct wlan_srng_cfg wlan_srng_rxdma_monitor_buf_cfg = {
	.timer_threshold = WLAN_CFG_INT_TIMER_THRESHOLD_RX,
	.batch_count_threshold = 0,
	.low_threshold = WLAN_CFG_RXDMA_MONITOR_BUF_RING_SIZE >> 3,
};

/* RXDMA_MONITOR_STATUS ring configuration */
struct wlan_srng_cfg wlan_srng_rxdma_monitor_status_cfg = {
	.timer_threshold = WLAN_CFG_INT_TIMER_THRESHOLD_RX,
	.batch_count_threshold = 0,
	.low_threshold = WLAN_CFG_RXDMA_MONITOR_STATUS_RING_SIZE >> 3,
};

/* DEFAULT_CONFIG ring configuration */
struct wlan_srng_cfg wlan_srng_default_cfg = {
	.timer_threshold = WLAN_CFG_INT_TIMER_THRESHOLD_OTHER,
	.batch_count_threshold = WLAN_CFG_INT_BATCH_THRESHOLD_OTHER,
	.low_threshold = 0,
};

void wlan_set_srng_cfg(struct wlan_srng_cfg **wlan_cfg)
{
	g_wlan_srng_cfg[REO_DST] = wlan_srng_reo_cfg;
	g_wlan_srng_cfg[WBM2SW_RELEASE] = wlan_srng_wbm_release_cfg;
	g_wlan_srng_cfg[REO_EXCEPTION] = wlan_srng_default_cfg;
	g_wlan_srng_cfg[REO_REINJECT] = wlan_srng_default_cfg;
	g_wlan_srng_cfg[REO_CMD] = wlan_srng_default_cfg;
	g_wlan_srng_cfg[REO_STATUS] = wlan_srng_default_cfg;
	g_wlan_srng_cfg[TCL_DATA] = wlan_srng_default_cfg;
	g_wlan_srng_cfg[TCL_CMD] = wlan_srng_default_cfg;
	g_wlan_srng_cfg[TCL_STATUS] = wlan_srng_default_cfg;
	g_wlan_srng_cfg[WBM_IDLE_LINK] = wlan_srng_default_cfg;
	g_wlan_srng_cfg[SW2WBM_RELEASE] = wlan_srng_default_cfg;
	g_wlan_srng_cfg[RXDMA_BUF] = wlan_srng_rxdma_buf_cfg;
	g_wlan_srng_cfg[RXDMA_DST] = wlan_srng_default_cfg;
	g_wlan_srng_cfg[RXDMA_MONITOR_BUF] =
			wlan_srng_rxdma_monitor_buf_cfg;
	g_wlan_srng_cfg[RXDMA_MONITOR_STATUS] =
			wlan_srng_rxdma_monitor_status_cfg;
	g_wlan_srng_cfg[RXDMA_MONITOR_DST] = wlan_srng_default_cfg;
	g_wlan_srng_cfg[RXDMA_MONITOR_DESC] = wlan_srng_default_cfg;
	g_wlan_srng_cfg[DIR_BUF_RX_DMA_SRC] = wlan_srng_default_cfg;
#ifdef WLAN_FEATURE_CIF_CFR
	g_wlan_srng_cfg[WIFI_POS_SRC] = wlan_srng_default_cfg;
#endif
	*wlan_cfg = g_wlan_srng_cfg;
}

/**
 * wlan_cfg_soc_attach() - Allocate and prepare SoC configuration
 * @psoc - Object manager psoc
 * Return: wlan_cfg_ctx - Handle to Configuration context
 */
struct wlan_cfg_dp_soc_ctxt *wlan_cfg_soc_attach(void *psoc)
{
	int i = 0;

	struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx =
		qdf_mem_malloc(sizeof(struct wlan_cfg_dp_soc_ctxt));

	if (!wlan_cfg_ctx)
		return NULL;

	wlan_cfg_ctx->rxdma1_enable = WLAN_CFG_RXDMA1_ENABLE;
	wlan_cfg_ctx->num_int_ctxts = WLAN_CFG_INT_NUM_CONTEXTS;
	wlan_cfg_ctx->max_clients = cfg_get(psoc, CFG_DP_MAX_CLIENTS);
	wlan_cfg_ctx->max_alloc_size = cfg_get(psoc, CFG_DP_MAX_ALLOC_SIZE);
	wlan_cfg_ctx->per_pdev_tx_ring = cfg_get(psoc, CFG_DP_PDEV_TX_RING);
	wlan_cfg_ctx->num_tcl_data_rings = cfg_get(psoc, CFG_DP_TCL_DATA_RINGS);
	wlan_cfg_ctx->per_pdev_rx_ring = cfg_get(psoc, CFG_DP_PDEV_RX_RING);
	wlan_cfg_ctx->per_pdev_lmac_ring = cfg_get(psoc, CFG_DP_PDEV_LMAC_RING);
	wlan_cfg_ctx->num_reo_dest_rings = cfg_get(psoc, CFG_DP_REO_DEST_RINGS);
	wlan_cfg_ctx->num_tx_desc_pool = MAX_TXDESC_POOLS;
	wlan_cfg_ctx->num_tx_ext_desc_pool = cfg_get(psoc,
						     CFG_DP_TX_EXT_DESC_POOLS);
	wlan_cfg_ctx->num_tx_desc = cfg_get(psoc, CFG_DP_TX_DESC);
	wlan_cfg_ctx->min_tx_desc = WLAN_CFG_NUM_TX_DESC_MIN;
	wlan_cfg_ctx->num_tx_ext_desc = cfg_get(psoc, CFG_DP_TX_EXT_DESC);
	wlan_cfg_ctx->htt_packet_type = cfg_get(psoc, CFG_DP_HTT_PACKET_TYPE);
	wlan_cfg_ctx->max_peer_id = cfg_get(psoc, CFG_DP_MAX_PEER_ID);

	wlan_cfg_ctx->tx_ring_size = cfg_get(psoc, CFG_DP_TX_RING_SIZE);
	wlan_cfg_ctx->tx_comp_ring_size = cfg_get(psoc,
						  CFG_DP_TX_COMPL_RING_SIZE);

	wlan_cfg_ctx->tx_comp_ring_size_nss =
		cfg_get(psoc, CFG_DP_NSS_COMP_RING_SIZE);

	wlan_cfg_ctx->int_batch_threshold_tx =
			cfg_get(psoc, CFG_DP_INT_BATCH_THRESHOLD_TX);
	wlan_cfg_ctx->int_timer_threshold_tx =
			cfg_get(psoc, CFG_DP_INT_TIMER_THRESHOLD_TX);
	wlan_cfg_ctx->int_batch_threshold_rx =
			cfg_get(psoc, CFG_DP_INT_BATCH_THRESHOLD_RX);
	wlan_cfg_ctx->int_timer_threshold_rx =
			cfg_get(psoc, CFG_DP_INT_TIMER_THRESHOLD_RX);
	wlan_cfg_ctx->int_batch_threshold_other =
		cfg_get(psoc, CFG_DP_INT_BATCH_THRESHOLD_OTHER);
	wlan_cfg_ctx->int_timer_threshold_other =
		cfg_get(psoc, CFG_DP_INT_TIMER_THRESHOLD_OTHER);

	for (i = 0; i < WLAN_CFG_INT_NUM_CONTEXTS; i++) {
		wlan_cfg_ctx->int_tx_ring_mask[i] = tx_ring_mask[i];
		wlan_cfg_ctx->int_rx_ring_mask[i] = rx_ring_mask[i];
		wlan_cfg_ctx->int_rx_mon_ring_mask[i] = rx_mon_ring_mask[i];
		wlan_cfg_ctx->int_rx_err_ring_mask[i] = rx_err_ring_mask[i];
		wlan_cfg_ctx->int_rx_wbm_rel_ring_mask[i] =
					rx_wbm_rel_ring_mask[i];
		wlan_cfg_ctx->int_reo_status_ring_mask[i] =
					reo_status_ring_mask[i];
		wlan_cfg_ctx->int_rxdma2host_ring_mask[i] =
			rxdma2host_ring_mask[i];
		wlan_cfg_ctx->int_host2rxdma_ring_mask[i] =
			host2rxdma_ring_mask[i];
		wlan_cfg_ctx->int_host2rxdma_mon_ring_mask[i] =
			host2rxdma_mon_ring_mask[i];
		wlan_cfg_ctx->int_rxdma2host_mon_ring_mask[i] =
			rxdma2host_mon_ring_mask[i];
	}

	/* This is default mapping and can be overridden by HW config
	 * received from FW */
	wlan_cfg_set_hw_macid(wlan_cfg_ctx, 0, 1);
	if (MAX_PDEV_CNT > 1)
		wlan_cfg_set_hw_macid(wlan_cfg_ctx, 1, 3);
	if (MAX_PDEV_CNT > 2)
		wlan_cfg_set_hw_macid(wlan_cfg_ctx, 2, 2);

	wlan_cfg_ctx->base_hw_macid = cfg_get(psoc, CFG_DP_BASE_HW_MAC_ID);

	wlan_cfg_ctx->rx_hash = cfg_get(psoc, CFG_DP_RX_HASH);
	wlan_cfg_ctx->tso_enabled = cfg_get(psoc, CFG_DP_TSO);
	wlan_cfg_ctx->lro_enabled = cfg_get(psoc, CFG_DP_LRO);
	wlan_cfg_ctx->sg_enabled = cfg_get(psoc, CFG_DP_SG);
	wlan_cfg_ctx->gro_enabled = cfg_get(psoc, CFG_DP_GRO);
	wlan_cfg_ctx->ol_tx_csum_enabled = cfg_get(psoc, CFG_DP_OL_TX_CSUM);
	wlan_cfg_ctx->ol_rx_csum_enabled = cfg_get(psoc, CFG_DP_OL_RX_CSUM);
	wlan_cfg_ctx->rawmode_enabled = cfg_get(psoc, CFG_DP_RAWMODE);
	wlan_cfg_ctx->peer_flow_ctrl_enabled =
			cfg_get(psoc, CFG_DP_PEER_FLOW_CTRL);
	wlan_cfg_ctx->napi_enabled = cfg_get(psoc, CFG_DP_NAPI);
	/*Enable checksum offload by default*/
	wlan_cfg_ctx->tcp_udp_checksumoffload =
			cfg_get(psoc, CFG_DP_TCP_UDP_CKSUM_OFFLOAD);
	wlan_cfg_ctx->per_pkt_trace = cfg_get(psoc, CFG_DP_PER_PKT_LOGGING);
	wlan_cfg_ctx->defrag_timeout_check =
			cfg_get(psoc, CFG_DP_DEFRAG_TIMEOUT_CHECK);
	wlan_cfg_ctx->rx_defrag_min_timeout =
			cfg_get(psoc, CFG_DP_RX_DEFRAG_TIMEOUT);

	wlan_cfg_ctx->wbm_release_ring = cfg_get(psoc,
						 CFG_DP_WBM_RELEASE_RING);
	wlan_cfg_ctx->tcl_cmd_ring = cfg_get(psoc,
					     CFG_DP_TCL_CMD_RING);
	wlan_cfg_ctx->tcl_status_ring = cfg_get(psoc,
						CFG_DP_TCL_STATUS_RING);
	wlan_cfg_ctx->reo_reinject_ring = cfg_get(psoc,
						  CFG_DP_REO_REINJECT_RING);
	wlan_cfg_ctx->rx_release_ring = cfg_get(psoc,
						CFG_DP_RX_RELEASE_RING);
	wlan_cfg_ctx->reo_exception_ring = cfg_get(psoc,
						   CFG_DP_REO_EXCEPTION_RING);
	wlan_cfg_ctx->reo_cmd_ring = cfg_get(psoc,
					     CFG_DP_REO_CMD_RING);
	wlan_cfg_ctx->reo_status_ring = cfg_get(psoc,
						CFG_DP_REO_STATUS_RING);
	wlan_cfg_ctx->rxdma_refill_ring = cfg_get(psoc,
						  CFG_DP_RXDMA_REFILL_RING);
	wlan_cfg_ctx->rxdma_err_dst_ring = cfg_get(psoc,
						   CFG_DP_RXDMA_ERR_DST_RING);
	wlan_cfg_ctx->enable_data_stall_detection =
		cfg_get(psoc, CFG_DP_ENABLE_DATA_STALL_DETECTION);
	wlan_cfg_ctx->tx_flow_start_queue_offset =
		cfg_get(psoc, CFG_DP_TX_FLOW_START_QUEUE_OFFSET);
	wlan_cfg_ctx->tx_flow_stop_queue_threshold =
		cfg_get(psoc, CFG_DP_TX_FLOW_STOP_QUEUE_TH);
	wlan_cfg_ctx->disable_intra_bss_fwd =
		cfg_get(psoc, CFG_DP_AP_STA_SECURITY_SEPERATION);

	return wlan_cfg_ctx;
}

void wlan_cfg_soc_detach(struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx)
{
	qdf_mem_free(wlan_cfg_ctx);
}

struct wlan_cfg_dp_pdev_ctxt *wlan_cfg_pdev_attach(void *psoc)
{
	struct wlan_cfg_dp_pdev_ctxt *wlan_cfg_ctx =
		qdf_mem_malloc(sizeof(struct wlan_cfg_dp_pdev_ctxt));

	if (!wlan_cfg_ctx)
		return NULL;

	wlan_cfg_ctx->rx_dma_buf_ring_size = cfg_get(psoc,
					CFG_DP_RXDMA_BUF_RING);
	wlan_cfg_ctx->dma_mon_buf_ring_size = cfg_get(psoc,
					CFG_DP_RXDMA_MONITOR_BUF_RING);
	wlan_cfg_ctx->dma_mon_dest_ring_size = cfg_get(psoc,
					CFG_DP_RXDMA_MONITOR_DST_RING);
	wlan_cfg_ctx->dma_mon_status_ring_size = cfg_get(psoc,
					CFG_DP_RXDMA_MONITOR_STATUS_RING);
	wlan_cfg_ctx->rxdma_monitor_desc_ring = cfg_get(psoc,
					CFG_DP_RXDMA_MONITOR_DESC_RING);
	wlan_cfg_ctx->num_mac_rings = NUM_RXDMA_RINGS_PER_PDEV;

	return wlan_cfg_ctx;
}

void wlan_cfg_pdev_detach(struct wlan_cfg_dp_pdev_ctxt *wlan_cfg_ctx)
{
	if (wlan_cfg_ctx)
		qdf_mem_free(wlan_cfg_ctx);
}

void wlan_cfg_set_num_contexts(struct wlan_cfg_dp_soc_ctxt *cfg, int num)
{
	cfg->num_int_ctxts = num;
}

void wlan_cfg_set_max_peer_id(struct wlan_cfg_dp_soc_ctxt *cfg, uint32_t val)
{
	cfg->max_peer_id = val;
}

void wlan_cfg_set_max_ast_idx(struct wlan_cfg_dp_soc_ctxt *cfg, uint32_t val)
{
	cfg->max_ast_idx = val;
}

int wlan_cfg_get_max_ast_idx(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->max_ast_idx;
}

void wlan_cfg_set_tx_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
		int context, int mask)
{
	cfg->int_tx_ring_mask[context] = mask;
}

void wlan_cfg_set_rx_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
			       int context, int mask)
{
	cfg->int_rx_ring_mask[context] = mask;
}

void wlan_cfg_set_rx_mon_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
		int context, int mask)
{
	cfg->int_rx_mon_ring_mask[context] = mask;
}

int wlan_cfg_get_host2rxdma_mon_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
					  int context)
{
	return cfg->int_host2rxdma_mon_ring_mask[context];
}

void wlan_cfg_set_host2rxdma_mon_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
					   int context, int mask)
{
	cfg->int_host2rxdma_mon_ring_mask[context] = mask;
}

int wlan_cfg_get_rxdma2host_mon_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
					  int context)
{
	return cfg->int_rxdma2host_mon_ring_mask[context];
}

void wlan_cfg_set_rxdma2host_mon_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
					   int context, int mask)
{
	cfg->int_rxdma2host_mon_ring_mask[context] = mask;
}

void wlan_cfg_set_rxdma2host_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
	int context, int mask)
{
	cfg->int_rxdma2host_ring_mask[context] = mask;
}

int wlan_cfg_get_rxdma2host_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
	int context)
{
	return cfg->int_rxdma2host_ring_mask[context];
}

void wlan_cfg_set_host2rxdma_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
	int context, int mask)
{
	cfg->int_host2rxdma_ring_mask[context] = mask;
}

int wlan_cfg_get_host2rxdma_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
	int context)
{
	return cfg->int_host2rxdma_ring_mask[context];
}

void wlan_cfg_set_hw_macid(struct wlan_cfg_dp_soc_ctxt *cfg, int pdev_idx,
	int hw_macid)
{
	qdf_assert_always(pdev_idx < MAX_PDEV_CNT);
	cfg->hw_macid[pdev_idx] = hw_macid;
}

int wlan_cfg_get_hw_macid(struct wlan_cfg_dp_soc_ctxt *cfg, int pdev_idx)
{
	qdf_assert_always(pdev_idx < MAX_PDEV_CNT);
	return cfg->hw_macid[pdev_idx];
}

int wlan_cfg_get_hw_mac_idx(struct wlan_cfg_dp_soc_ctxt *cfg, int pdev_idx)
{
	qdf_assert_always(pdev_idx < MAX_PDEV_CNT);
	return cfg->hw_macid[pdev_idx] - cfg->base_hw_macid;
}

void wlan_cfg_set_ce_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
		int context, int mask)
{
	cfg->int_ce_ring_mask[context] = mask;
}

void wlan_cfg_set_rxbuf_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg, int context,
		int mask)
{
	cfg->int_rx_ring_mask[context] = mask;
}

int wlan_cfg_set_rx_err_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
		int context, int mask)
{
	return cfg->int_rx_err_ring_mask[context] = mask;
}

int wlan_cfg_set_rx_wbm_rel_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
		int context, int mask)
{
	return cfg->int_rx_wbm_rel_ring_mask[context] = mask;
}

int wlan_cfg_set_reo_status_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
		int context, int mask)
{
	return cfg->int_reo_status_ring_mask[context] = mask;
}

int wlan_cfg_get_num_contexts(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->num_int_ctxts;
}

int wlan_cfg_get_tx_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg, int context)
{
	return cfg->int_tx_ring_mask[context];
}

int wlan_cfg_get_rx_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg, int context)
{
	return cfg->int_rx_ring_mask[context];
}

int wlan_cfg_get_rx_err_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
						int context)
{
	return cfg->int_rx_err_ring_mask[context];
}

int wlan_cfg_get_rx_wbm_rel_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
					int context)
{
	return cfg->int_rx_wbm_rel_ring_mask[context];
}

int wlan_cfg_get_reo_status_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
					int context)
{
	return cfg->int_reo_status_ring_mask[context];
}

int wlan_cfg_get_rx_mon_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg, int context)
{
	return cfg->int_rx_mon_ring_mask[context];
}

int wlan_cfg_get_ce_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg, int context)
{
	return cfg->int_ce_ring_mask[context];
}

uint32_t wlan_cfg_get_max_clients(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->max_clients;
}

uint32_t wlan_cfg_max_alloc_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->max_alloc_size;
}

int wlan_cfg_per_pdev_tx_ring(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->per_pdev_tx_ring;
}

int wlan_cfg_per_pdev_lmac_ring(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->per_pdev_lmac_ring;
}

int wlan_cfg_num_tcl_data_rings(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->num_tcl_data_rings;
}

int wlan_cfg_tx_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->tx_ring_size;
}

int wlan_cfg_tx_comp_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->tx_comp_ring_size;
}

int wlan_cfg_per_pdev_rx_ring(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->per_pdev_rx_ring;
}

int wlan_cfg_num_reo_dest_rings(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->num_reo_dest_rings;
}

int wlan_cfg_pkt_type(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->htt_packet_type;            /*htt_pkt_type_ethernet*/
}

int wlan_cfg_get_num_tx_desc_pool(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->num_tx_desc_pool;
}

void wlan_cfg_set_num_tx_desc_pool(struct wlan_cfg_dp_soc_ctxt *cfg, int num_pool)
{
	cfg->num_tx_desc_pool = num_pool;
}

int wlan_cfg_get_num_tx_ext_desc_pool(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->num_tx_ext_desc_pool;
}

void wlan_cfg_set_num_tx_ext_desc_pool(struct wlan_cfg_dp_soc_ctxt *cfg, int num_pool)
{
	cfg->num_tx_ext_desc_pool = num_pool;
}

int wlan_cfg_get_reo_dst_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->reo_dst_ring_size;
}

void wlan_cfg_set_reo_dst_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg,
				    int reo_dst_ring_size)
{
	cfg->reo_dst_ring_size = reo_dst_ring_size;
}

void wlan_cfg_set_raw_mode_war(struct wlan_cfg_dp_soc_ctxt *cfg,
			       bool raw_mode_war)
{
	cfg->raw_mode_war = raw_mode_war;
}

bool wlan_cfg_get_raw_mode_war(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->raw_mode_war;
}

int wlan_cfg_get_num_tx_desc(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->num_tx_desc;
}

void wlan_cfg_set_num_tx_desc(struct wlan_cfg_dp_soc_ctxt *cfg, int num_desc)
{
	cfg->num_tx_desc = num_desc;
}

int wlan_cfg_get_min_tx_desc(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->min_tx_desc;
}

int wlan_cfg_get_num_tx_ext_desc(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->num_tx_ext_desc;
}

void wlan_cfg_set_num_tx_ext_desc(struct wlan_cfg_dp_soc_ctxt *cfg, int num_ext_desc)
{
	cfg->num_tx_ext_desc = num_ext_desc;
}

uint32_t wlan_cfg_max_peer_id(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	/* TODO: This should be calculated based on target capabilities */
	return cfg->max_peer_id;
}

int wlan_cfg_get_dma_mon_buf_ring_size(struct wlan_cfg_dp_pdev_ctxt *cfg)
{
	return  cfg->dma_mon_buf_ring_size;
}

int wlan_cfg_get_dma_mon_dest_ring_size(struct wlan_cfg_dp_pdev_ctxt *cfg)
{
	return  cfg->dma_mon_dest_ring_size;
}

int wlan_cfg_get_dma_mon_stat_ring_size(struct wlan_cfg_dp_pdev_ctxt *cfg)
{
	return  cfg->dma_mon_status_ring_size;
}

int
wlan_cfg_get_dma_mon_desc_ring_size(struct wlan_cfg_dp_pdev_ctxt *cfg)
{
	return cfg->rxdma_monitor_desc_ring;
}

int wlan_cfg_get_rx_dma_buf_ring_size(struct wlan_cfg_dp_pdev_ctxt *cfg)
{
	return  cfg->rx_dma_buf_ring_size;
}

int wlan_cfg_get_num_mac_rings(struct wlan_cfg_dp_pdev_ctxt *cfg)
{
	return  cfg->num_mac_rings;
}

bool wlan_cfg_is_gro_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return  cfg->gro_enabled;
}

bool wlan_cfg_is_lro_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return  cfg->lro_enabled;
}

bool wlan_cfg_is_ipa_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return  cfg->ipa_enabled;
}

void wlan_cfg_set_rx_hash(struct wlan_cfg_dp_soc_ctxt *cfg, bool val)
{
	cfg->rx_hash = val;
}

bool wlan_cfg_is_rx_hash_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return  cfg->rx_hash;
}

int wlan_cfg_get_dp_pdev_nss_enabled(struct wlan_cfg_dp_pdev_ctxt *cfg)
{
	return  cfg->nss_enabled;
}

void wlan_cfg_set_dp_pdev_nss_enabled(struct wlan_cfg_dp_pdev_ctxt *cfg, int nss_enabled)
{
	cfg->nss_enabled = nss_enabled;
}

int wlan_cfg_get_dp_soc_nss_cfg(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return  cfg->nss_cfg;
}

void wlan_cfg_set_dp_soc_nss_cfg(struct wlan_cfg_dp_soc_ctxt *cfg, int nss_cfg)
{
	cfg->nss_cfg = nss_cfg;
	if (cfg->nss_cfg)
		cfg->tx_comp_ring_size = cfg->tx_comp_ring_size_nss;
}

int wlan_cfg_get_int_batch_threshold_tx(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->int_batch_threshold_tx;
}

int wlan_cfg_get_int_timer_threshold_tx(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->int_timer_threshold_tx;
}

int wlan_cfg_get_int_batch_threshold_rx(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->int_batch_threshold_rx;
}

int wlan_cfg_get_int_timer_threshold_rx(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->int_timer_threshold_rx;
}

int wlan_cfg_get_int_batch_threshold_other(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->int_batch_threshold_other;
}

int wlan_cfg_get_int_timer_threshold_other(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->int_timer_threshold_other;
}

int wlan_cfg_get_int_timer_threshold_mon(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->int_timer_threshold_mon;
}

int wlan_cfg_get_checksum_offload(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->tcp_udp_checksumoffload;
}

int wlan_cfg_get_rx_defrag_min_timeout(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->rx_defrag_min_timeout;
}

int wlan_cfg_get_defrag_timeout_check(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->defrag_timeout_check;
}

int
wlan_cfg_get_dp_soc_wbm_release_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->wbm_release_ring;
}

int
wlan_cfg_get_dp_soc_tcl_cmd_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->tcl_cmd_ring;
}

int
wlan_cfg_get_dp_soc_tcl_status_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->tcl_status_ring;
}

int
wlan_cfg_get_dp_soc_reo_reinject_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->reo_reinject_ring;
}

int
wlan_cfg_get_dp_soc_rx_release_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->rx_release_ring;
}

int
wlan_cfg_get_dp_soc_reo_exception_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->reo_exception_ring;
}

int
wlan_cfg_get_dp_soc_reo_cmd_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->reo_cmd_ring;
}

int
wlan_cfg_get_dp_soc_reo_status_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->reo_status_ring;
}

int
wlan_cfg_get_dp_soc_rxdma_refill_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->rxdma_refill_ring;
}

int
wlan_cfg_get_dp_soc_rxdma_err_dst_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->rxdma_err_dst_ring;
}

bool
wlan_cfg_get_dp_caps(struct wlan_cfg_dp_soc_ctxt *cfg,
		     enum cdp_capabilities dp_caps)
{
	switch (dp_caps) {
	case CDP_CFG_DP_TSO:
		return cfg->tso_enabled;
	case CDP_CFG_DP_LRO:
		return cfg->lro_enabled;
	case CDP_CFG_DP_SG:
		return cfg->sg_enabled;
	case CDP_CFG_DP_GRO:
		return cfg->gro_enabled;
	case CDP_CFG_DP_OL_TX_CSUM:
		return cfg->ol_tx_csum_enabled;
	case CDP_CFG_DP_OL_RX_CSUM:
		return cfg->ol_rx_csum_enabled;
	case CDP_CFG_DP_RAWMODE:
		return cfg->rawmode_enabled;
	case CDP_CFG_DP_PEER_FLOW_CTRL:
		return cfg->peer_flow_ctrl_enabled;
	default:
		return false;
	}
}

#ifdef QCA_LL_TX_FLOW_CONTROL_V2
/**
 * wlan_cfg_get_tx_flow_stop_queue_th() - Get flow control stop threshold
 * @cfg: config context
 *
 * Return: stop threshold
 */
int wlan_cfg_get_tx_flow_stop_queue_th(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->tx_flow_stop_queue_threshold;
}

/**
 * wlan_cfg_get_tx_flow_start_queue_offset() - Get flow control start offset
 *					for TX to resume
 * @cfg: config context
 *
 * Return: stop threshold
 */
int wlan_cfg_get_tx_flow_start_queue_offset(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->tx_flow_start_queue_offset;
}
#endif /* QCA_LL_TX_FLOW_CONTROL_V2 */
