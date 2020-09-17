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
#include "dp_types.h"

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

#if defined(WLAN_MAX_PDEVS) && (WLAN_MAX_PDEVS == 1)
static const int tx_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	WLAN_CFG_TX_RING_MASK_0, 0, 0, 0, 0, 0, 0};

#ifndef IPA_OFFLOAD
static const int rx_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0, WLAN_CFG_RX_RING_MASK_0, WLAN_CFG_RX_RING_MASK_1, WLAN_CFG_RX_RING_MASK_2, WLAN_CFG_RX_RING_MASK_3, 0, 0};
#else
static const int rx_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0, WLAN_CFG_RX_RING_MASK_0, WLAN_CFG_RX_RING_MASK_1, WLAN_CFG_RX_RING_MASK_2, 0, 0, 0};
#endif

static const int rx_mon_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0, WLAN_CFG_RX_MON_RING_MASK_0, WLAN_CFG_RX_MON_RING_MASK_1, 0, 0, 0, 0};

static const int host2rxdma_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0, 0, 0, 0, 0, 0, 0};

static const int rxdma2host_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0, 0, 0, 0, 0, WLAN_CFG_RXDMA2HOST_RING_MASK_0, WLAN_CFG_RXDMA2HOST_RING_MASK_1};

static const int host2rxdma_mon_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0, 0, 0, 0, 0, 0, 0};

static const int rxdma2host_mon_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0, 0, 0, 0, 0, 0, 0};

static const int rx_err_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0, 0, 0, 0, 0, 0, WLAN_CFG_RX_ERR_RING_MASK_0};

static const int rx_wbm_rel_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0, 0, 0, 0, 0, 0, WLAN_CFG_RX_WBM_REL_RING_MASK_0};

static const int reo_status_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0, 0, 0, 0, 0, 0, WLAN_CFG_REO_STATUS_RING_MASK_0};

static const int tx_ring_mask_integrated[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0, 0, 0, 0, 0, 0, 0};

static const int rx_ring_mask_integrated[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0, 0, 0, 0, 0, 0, 0};

static const int rx_mon_ring_mask_integrated[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0, 0, 0, 0, 0, 0, 0};

static const int host2rxdma_ring_mask_integrated[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0, 0, 0, 0, 0, 0, 0};

static const int rxdma2host_ring_mask_integrated[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0, 0, 0, 0, 0, 0, 0};

static const int host2rxdma_mon_ring_mask_integrated[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0, 0, 0, 0, 0, 0, 0};

static const int rxdma2host_mon_ring_mask_integrated[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0, 0, 0, 0, 0, 0, 0};

static const int rx_err_ring_mask_integrated[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0, 0, 0, 0, 0, 0, 0};

static const int rx_wbm_rel_ring_mask_integrated[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0, 0, 0, 0, 0, 0, 0};

static const int reo_status_ring_mask_integrated[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0, 0, 0, 0, 0, 0, 0};

#else

static const int tx_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
						WLAN_CFG_TX_RING_MASK_0,
						WLAN_CFG_TX_RING_MASK_1,
						WLAN_CFG_TX_RING_MASK_2,
						WLAN_CFG_TX_RING_MASK_3};

static const int rx_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
					0,
					0,
					0,
					0,
					WLAN_CFG_RX_RING_MASK_0,
					WLAN_CFG_RX_RING_MASK_1,
					WLAN_CFG_RX_RING_MASK_2,
					WLAN_CFG_RX_RING_MASK_3};

static const int rx_mon_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
					0,
					0,
					0,
					WLAN_CFG_RX_MON_RING_MASK_0,
					WLAN_CFG_RX_MON_RING_MASK_1,
					WLAN_CFG_RX_MON_RING_MASK_2};

static const int host2rxdma_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
					0,
					0,
					0,
					WLAN_CFG_HOST2RXDMA_RING_MASK_0,
					WLAN_CFG_HOST2RXDMA_RING_MASK_1,
					WLAN_CFG_HOST2RXDMA_RING_MASK_2,
					WLAN_CFG_HOST2RXDMA_RING_MASK_3};

static const int rxdma2host_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
					0,
					0,
					0,
					WLAN_CFG_RXDMA2HOST_RING_MASK_0,
					WLAN_CFG_RXDMA2HOST_RING_MASK_1,
					WLAN_CFG_RXDMA2HOST_RING_MASK_2,
					WLAN_CFG_RXDMA2HOST_RING_MASK_3};

static const int host2rxdma_mon_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
					0,
					0,
					0,
					WLAN_CFG_HOST2RXDMA_MON_RING_MASK_0,
					WLAN_CFG_HOST2RXDMA_MON_RING_MASK_1,
					WLAN_CFG_HOST2RXDMA_MON_RING_MASK_2};

static const int rxdma2host_mon_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
					0,
					0,
					0,
					WLAN_CFG_RXDMA2HOST_MON_RING_MASK_0,
					WLAN_CFG_RXDMA2HOST_MON_RING_MASK_1,
					WLAN_CFG_RXDMA2HOST_MON_RING_MASK_2};

static const int rx_err_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
					0,
					0,
					0,
					WLAN_CFG_RX_ERR_RING_MASK_0,
					WLAN_CFG_RX_ERR_RING_MASK_1,
					WLAN_CFG_RX_ERR_RING_MASK_2,
					WLAN_CFG_RX_ERR_RING_MASK_3};

static const int rx_wbm_rel_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
					0,
					0,
					0,
					WLAN_CFG_RX_WBM_REL_RING_MASK_0,
					WLAN_CFG_RX_WBM_REL_RING_MASK_1,
					WLAN_CFG_RX_WBM_REL_RING_MASK_2,
					WLAN_CFG_RX_WBM_REL_RING_MASK_3};

static const int reo_status_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
					0,
					0,
					0,
					WLAN_CFG_REO_STATUS_RING_MASK_0,
					WLAN_CFG_REO_STATUS_RING_MASK_1,
					WLAN_CFG_REO_STATUS_RING_MASK_2,
					WLAN_CFG_REO_STATUS_RING_MASK_3};

static const int tx_ring_mask_integrated[WLAN_CFG_INT_NUM_CONTEXTS] = {
						WLAN_CFG_TX_RING_MASK_0,
						WLAN_CFG_TX_RING_MASK_1,
						WLAN_CFG_TX_RING_MASK_2,
						WLAN_CFG_TX_RING_MASK_3};

static const int rx_ring_mask_integrated[WLAN_CFG_INT_NUM_CONTEXTS] = {
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

static const int rx_mon_ring_mask_integrated[WLAN_CFG_INT_NUM_CONTEXTS] = {
					0,
					0,
					0,
					0,
					WLAN_CFG_RX_MON_RING_MASK_0,
					WLAN_CFG_RX_MON_RING_MASK_1,
					WLAN_CFG_RX_MON_RING_MASK_2};

static const int host2rxdma_ring_mask_integrated[WLAN_CFG_INT_NUM_CONTEXTS] = {
					WLAN_CFG_HOST2RXDMA_RING_MASK_0,
					WLAN_CFG_HOST2RXDMA_RING_MASK_1,
					WLAN_CFG_HOST2RXDMA_RING_MASK_2,
					WLAN_CFG_HOST2RXDMA_RING_MASK_3};

static const int rxdma2host_ring_mask_integrated[WLAN_CFG_INT_NUM_CONTEXTS] = {
					WLAN_CFG_RXDMA2HOST_RING_MASK_0,
					WLAN_CFG_RXDMA2HOST_RING_MASK_1,
					WLAN_CFG_RXDMA2HOST_RING_MASK_2,
					WLAN_CFG_RXDMA2HOST_RING_MASK_3};

static const int host2rxdma_mon_ring_mask_integrated[WLAN_CFG_INT_NUM_CONTEXTS] = {
					0,
					0,
					0,
					0,
					WLAN_CFG_HOST2RXDMA_MON_RING_MASK_0,
					WLAN_CFG_HOST2RXDMA_MON_RING_MASK_1,
					WLAN_CFG_HOST2RXDMA_MON_RING_MASK_2};

static const int rxdma2host_mon_ring_mask_integrated[WLAN_CFG_INT_NUM_CONTEXTS] = {
					0,
					0,
					0,
					0,
					WLAN_CFG_RXDMA2HOST_MON_RING_MASK_0,
					WLAN_CFG_RXDMA2HOST_MON_RING_MASK_1,
					WLAN_CFG_RXDMA2HOST_MON_RING_MASK_2};

static const int rx_err_ring_mask_integrated[WLAN_CFG_INT_NUM_CONTEXTS] = {
					WLAN_CFG_RX_ERR_RING_MASK_0,
					WLAN_CFG_RX_ERR_RING_MASK_1,
					WLAN_CFG_RX_ERR_RING_MASK_2,
					WLAN_CFG_RX_ERR_RING_MASK_3};

static const int rx_wbm_rel_ring_mask_integrated[WLAN_CFG_INT_NUM_CONTEXTS] = {
					WLAN_CFG_RX_WBM_REL_RING_MASK_0,
					WLAN_CFG_RX_WBM_REL_RING_MASK_1,
					WLAN_CFG_RX_WBM_REL_RING_MASK_2,
					WLAN_CFG_RX_WBM_REL_RING_MASK_3};

static const int reo_status_ring_mask_integrated[WLAN_CFG_INT_NUM_CONTEXTS] = {
					WLAN_CFG_REO_STATUS_RING_MASK_0,
					WLAN_CFG_REO_STATUS_RING_MASK_1,
					WLAN_CFG_REO_STATUS_RING_MASK_2,
					WLAN_CFG_REO_STATUS_RING_MASK_3};
#endif /* MAX_PDEV_CNT == 1 */

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
	g_wlan_srng_cfg[TCL_CMD_CREDIT] = wlan_srng_default_cfg;
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

static const uint8_t rx_fst_toeplitz_key[WLAN_CFG_RX_FST_TOEPLITZ_KEYLEN] = {
	0x6d, 0x5a, 0x56, 0xda, 0x25, 0x5b, 0x0e, 0xc2,
	0x41, 0x67, 0x25, 0x3d, 0x43, 0xa3, 0x8f, 0xb0,
	0xd0, 0xca, 0x2b, 0xcb, 0xae, 0x7b, 0x30, 0xb4,
	0x77, 0xcb, 0x2d, 0xa3, 0x80, 0x30, 0xf2, 0x0c,
	0x6a, 0x42, 0xb7, 0x3b, 0xbe, 0xac, 0x01, 0xfa
};

void wlan_cfg_fill_interrupt_mask(struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx,
				     int interrupt_mode,
				     bool is_monitor_mode) {
	int i = 0;

	if (interrupt_mode == DP_INTR_INTEGRATED) {
		for (i = 0; i < WLAN_CFG_INT_NUM_CONTEXTS; i++) {
			wlan_cfg_ctx->int_tx_ring_mask[i] =
					tx_ring_mask_integrated[i];
			wlan_cfg_ctx->int_rx_ring_mask[i] =
					rx_ring_mask_integrated[i];
			wlan_cfg_ctx->int_rx_mon_ring_mask[i] =
				rx_mon_ring_mask_integrated[i];
			wlan_cfg_ctx->int_rx_err_ring_mask[i] =
				rx_err_ring_mask_integrated[i];
			wlan_cfg_ctx->int_rx_wbm_rel_ring_mask[i] =
					rx_wbm_rel_ring_mask_integrated[i];
			wlan_cfg_ctx->int_reo_status_ring_mask[i] =
					reo_status_ring_mask_integrated[i];
			wlan_cfg_ctx->int_rxdma2host_ring_mask[i] =
				rxdma2host_ring_mask_integrated[i];
			wlan_cfg_ctx->int_host2rxdma_ring_mask[i] =
				host2rxdma_ring_mask_integrated[i];
			wlan_cfg_ctx->int_host2rxdma_mon_ring_mask[i] =
				host2rxdma_mon_ring_mask_integrated[i];
			wlan_cfg_ctx->int_rxdma2host_mon_ring_mask[i] =
				rxdma2host_mon_ring_mask_integrated[i];
		}
	} else if (interrupt_mode == DP_INTR_MSI || interrupt_mode ==
		   DP_INTR_POLL) {
		for (i = 0; i < WLAN_CFG_INT_NUM_CONTEXTS; i++) {
			wlan_cfg_ctx->int_tx_ring_mask[i] = tx_ring_mask_msi[i];
			wlan_cfg_ctx->int_rx_mon_ring_mask[i] =
				rx_mon_ring_mask_msi[i];
			wlan_cfg_ctx->int_rx_err_ring_mask[i] =
				rx_err_ring_mask_msi[i];
			wlan_cfg_ctx->int_rx_wbm_rel_ring_mask[i] =
						rx_wbm_rel_ring_mask_msi[i];
			wlan_cfg_ctx->int_reo_status_ring_mask[i] =
						reo_status_ring_mask_msi[i];
			if (is_monitor_mode) {
				wlan_cfg_ctx->int_rx_ring_mask[i] = 0;
				wlan_cfg_ctx->int_rxdma2host_ring_mask[i] = 0;
			} else {
				wlan_cfg_ctx->int_rx_ring_mask[i] =
					rx_ring_mask_msi[i];
				wlan_cfg_ctx->int_rxdma2host_ring_mask[i] =
					rxdma2host_ring_mask_msi[i];
			}
			wlan_cfg_ctx->int_host2rxdma_ring_mask[i] =
				host2rxdma_ring_mask_msi[i];
			wlan_cfg_ctx->int_host2rxdma_mon_ring_mask[i] =
				host2rxdma_mon_ring_mask_msi[i];
			wlan_cfg_ctx->int_rxdma2host_mon_ring_mask[i] =
				rxdma2host_mon_ring_mask_msi[i];
		}
	} else {
		qdf_err("Interrupt mode %d", interrupt_mode);
	}
}

/**
 * wlan_cfg_soc_attach() - Allocate and prepare SoC configuration
 * @psoc - Object manager psoc
 * Return: wlan_cfg_ctx - Handle to Configuration context
 */
struct wlan_cfg_dp_soc_ctxt *
wlan_cfg_soc_attach(struct cdp_ctrl_objmgr_psoc *psoc)
{
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
	wlan_cfg_ctx->pktlog_buffer_size =
		cfg_get(psoc, CFG_DP_PKTLOG_BUFFER_SIZE);

	/* This is default mapping and can be overridden by HW config
	 * received from FW */
	wlan_cfg_set_hw_mac_idx(wlan_cfg_ctx, 0, 0);
	if (MAX_PDEV_CNT > 1)
		wlan_cfg_set_hw_mac_idx(wlan_cfg_ctx, 1, 2);
	if (MAX_PDEV_CNT > 2)
		wlan_cfg_set_hw_mac_idx(wlan_cfg_ctx, 2, 1);

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
	wlan_cfg_ctx->p2p_tcp_udp_checksumoffload =
			cfg_get(psoc, CFG_DP_P2P_TCP_UDP_CKSUM_OFFLOAD);
	wlan_cfg_ctx->nan_tcp_udp_checksumoffload =
			cfg_get(psoc, CFG_DP_NAN_TCP_UDP_CKSUM_OFFLOAD);
	wlan_cfg_ctx->tcp_udp_checksumoffload =
			cfg_get(psoc, CFG_DP_TCP_UDP_CKSUM_OFFLOAD);
	wlan_cfg_ctx->legacy_mode_checksumoffload_disable =
			cfg_get(psoc, CFG_DP_LEGACY_MODE_CSUM_DISABLE);
	wlan_cfg_ctx->per_pkt_trace = cfg_get(psoc, CFG_DP_PER_PKT_LOGGING);
	wlan_cfg_ctx->defrag_timeout_check =
			cfg_get(psoc, CFG_DP_DEFRAG_TIMEOUT_CHECK);
	wlan_cfg_ctx->rx_defrag_min_timeout =
			cfg_get(psoc, CFG_DP_RX_DEFRAG_TIMEOUT);

	wlan_cfg_ctx->wbm_release_ring = cfg_get(psoc,
						 CFG_DP_WBM_RELEASE_RING);
	wlan_cfg_ctx->tcl_cmd_credit_ring = cfg_get(psoc,
					     CFG_DP_TCL_CMD_CREDIT_RING);
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
	wlan_cfg_ctx->tx_desc_limit_0 = cfg_get(psoc,
						CFG_DP_TX_DESC_LIMIT_0);
	wlan_cfg_ctx->tx_desc_limit_1 = cfg_get(psoc,
						CFG_DP_TX_DESC_LIMIT_1);
	wlan_cfg_ctx->tx_desc_limit_2 = cfg_get(psoc,
						CFG_DP_TX_DESC_LIMIT_2);
	wlan_cfg_ctx->tx_device_limit = cfg_get(psoc,
						CFG_DP_TX_DEVICE_LIMIT);
	wlan_cfg_ctx->tx_sw_internode_queue = cfg_get(psoc,
						CFG_DP_TX_SW_INTERNODE_QUEUE);
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
	wlan_cfg_ctx->rx_sw_desc_weight = cfg_get(psoc,
						   CFG_DP_RX_SW_DESC_WEIGHT);
	wlan_cfg_ctx->rx_sw_desc_num = cfg_get(psoc,
						   CFG_DP_RX_SW_DESC_NUM);
	wlan_cfg_ctx->rx_toeplitz_hash_key = (uint8_t *)rx_fst_toeplitz_key;
	wlan_cfg_ctx->rx_flow_max_search = WLAN_CFG_RX_FST_MAX_SEARCH;
	wlan_cfg_ctx->is_rx_flow_tag_enabled =
			cfg_get(psoc, CFG_DP_RX_FLOW_TAG_ENABLE);
	wlan_cfg_ctx->is_rx_flow_search_table_per_pdev =
			cfg_get(psoc, CFG_DP_RX_FLOW_SEARCH_TABLE_PER_PDEV);
	wlan_cfg_ctx->rx_flow_search_table_size =
			cfg_get(psoc, CFG_DP_RX_FLOW_SEARCH_TABLE_SIZE);
	wlan_cfg_ctx->is_rx_mon_protocol_flow_tag_enabled =
			cfg_get(psoc, CFG_DP_RX_MON_PROTOCOL_FLOW_TAG_ENABLE);
	wlan_cfg_ctx->mon_drop_thresh =
		cfg_get(psoc, CFG_DP_RXDMA_MONITOR_RX_DROP_THRESHOLD);
	wlan_cfg_ctx->is_rx_fisa_enabled = cfg_get(psoc, CFG_DP_RX_FISA_ENABLE);
	wlan_cfg_ctx->reo_rings_mapping = cfg_get(psoc, CFG_DP_REO_RINGS_MAP);
	wlan_cfg_ctx->pext_stats_enabled = cfg_get(psoc, CFG_DP_PEER_EXT_STATS);
	wlan_cfg_ctx->is_rx_buff_pool_enabled =
			cfg_get(psoc, CFG_DP_RX_BUFF_POOL_ENABLE);
	wlan_cfg_ctx->rx_pending_high_threshold =
			cfg_get(psoc, CFG_DP_RX_PENDING_HL_THRESHOLD);
	wlan_cfg_ctx->rx_pending_low_threshold =
			cfg_get(psoc, CFG_DP_RX_PENDING_LO_THRESHOLD);
	wlan_cfg_ctx->is_poll_mode_enabled =
			cfg_get(psoc, CFG_DP_POLL_MODE_ENABLE);
	wlan_cfg_ctx->is_swlm_enabled = cfg_get(psoc, CFG_DP_SWLM_ENABLE);
	wlan_cfg_ctx->fst_in_cmem = cfg_get(psoc, CFG_DP_RX_FST_IN_CMEM);
	wlan_cfg_ctx->tx_per_pkt_vdev_id_check =
			cfg_get(psoc, CFG_DP_TX_PER_PKT_VDEV_ID_CHECK);

	return wlan_cfg_ctx;
}

void wlan_cfg_soc_detach(struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx)
{
	qdf_mem_free(wlan_cfg_ctx);
}

struct wlan_cfg_dp_pdev_ctxt *
wlan_cfg_pdev_attach(struct cdp_ctrl_objmgr_psoc *psoc)
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

void wlan_cfg_set_mon_delayed_replenish_entries(
					struct wlan_cfg_dp_soc_ctxt *cfg,
					uint32_t val)
{
	cfg->delayed_replenish_entries = val;
}

int wlan_cfg_get_mon_delayed_replenish_entries(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->delayed_replenish_entries;
}

void wlan_cfg_pdev_detach(struct wlan_cfg_dp_pdev_ctxt *wlan_cfg_ctx)
{
	if (wlan_cfg_ctx)
		qdf_mem_free(wlan_cfg_ctx);
}

int wlan_cfg_get_mon_drop_thresh(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->mon_drop_thresh;
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

void wlan_cfg_set_hw_mac_idx(struct wlan_cfg_dp_soc_ctxt *cfg, int pdev_idx,
			     int hw_macid)
{
	qdf_assert_always(pdev_idx < MAX_PDEV_CNT);
	cfg->hw_macid[pdev_idx] = hw_macid;
}

int wlan_cfg_get_hw_mac_idx(struct wlan_cfg_dp_soc_ctxt *cfg, int pdev_idx)
{
	qdf_assert_always(pdev_idx < MAX_PDEV_CNT);
	return cfg->hw_macid[pdev_idx];
}

int wlan_cfg_get_target_pdev_id(struct wlan_cfg_dp_soc_ctxt *cfg,
				int hw_macid)
{
	int idx;

	for (idx = 0; idx < MAX_PDEV_CNT; idx++) {
		if (cfg->hw_macid[idx] == hw_macid)
			return (idx + 1);
	}
	qdf_assert_always(idx < MAX_PDEV_CNT);
	return WLAN_INVALID_PDEV_ID;
}

void wlan_cfg_set_pdev_idx(struct wlan_cfg_dp_soc_ctxt *cfg, int pdev_idx,
			   int hw_macid)
{
	qdf_assert_always((pdev_idx < MAX_PDEV_CNT) ||
			  (pdev_idx == INVALID_PDEV_ID));
	qdf_assert_always(hw_macid < MAX_NUM_LMAC_HW);
	cfg->hw_macid_pdev_id_map[hw_macid] = pdev_idx;
}

int wlan_cfg_get_pdev_idx(struct wlan_cfg_dp_soc_ctxt *cfg, int hw_macid)
{
	qdf_assert_always(hw_macid < MAX_NUM_LMAC_HW);
	return cfg->hw_macid_pdev_id_map[hw_macid];
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

uint32_t
wlan_cfg_rx_pending_hl_threshold(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->rx_pending_high_threshold;
}

uint32_t
wlan_cfg_rx_pending_lo_threshold(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->rx_pending_low_threshold;
}

int wlan_cfg_per_pdev_lmac_ring(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->per_pdev_lmac_ring;
}

#ifdef DP_MEMORY_OPT
int wlan_cfg_num_tcl_data_rings(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return 1;
}
#else
int wlan_cfg_num_tcl_data_rings(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->num_tcl_data_rings;
}
#endif

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

int wlan_cfg_get_p2p_checksum_offload(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->p2p_tcp_udp_checksumoffload;
}

int wlan_cfg_get_nan_checksum_offload(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->nan_tcp_udp_checksumoffload;
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
wlan_cfg_get_dp_soc_tcl_cmd_credit_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->tcl_cmd_credit_ring;
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
wlan_cfg_get_dp_soc_tx_desc_limit_0(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->tx_desc_limit_0;
}

int
wlan_cfg_get_dp_soc_tx_desc_limit_1(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->tx_desc_limit_1;
}

int
wlan_cfg_get_dp_soc_tx_desc_limit_2(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->tx_desc_limit_2;
}

int
wlan_cfg_get_dp_soc_tx_device_limit(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->tx_device_limit;
}

int
wlan_cfg_get_dp_soc_tx_sw_internode_queue(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->tx_sw_internode_queue;
}

int
wlan_cfg_get_dp_soc_rxdma_err_dst_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->rxdma_err_dst_ring;
}

int
wlan_cfg_get_dp_soc_rx_sw_desc_weight(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->rx_sw_desc_weight;
}

int
wlan_cfg_get_dp_soc_rx_sw_desc_num(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->rx_sw_desc_num;
}

uint32_t
wlan_cfg_get_reo_rings_mapping(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->reo_rings_mapping;
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

void wlan_cfg_set_tso_desc_attach_defer(struct wlan_cfg_dp_soc_ctxt *cfg,
					bool val)
{
	cfg->is_tso_desc_attach_defer = val;
}

bool wlan_cfg_is_tso_desc_attach_defer(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->is_tso_desc_attach_defer;
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

void wlan_cfg_set_rx_flow_tag_enabled(struct wlan_cfg_dp_soc_ctxt *cfg,
				      bool val)
{
	cfg->is_rx_flow_tag_enabled = val;
}

uint8_t *wlan_cfg_rx_fst_get_hash_key(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->rx_toeplitz_hash_key;
}

uint8_t wlan_cfg_rx_fst_get_max_search(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->rx_flow_max_search;
}

bool wlan_cfg_is_rx_flow_tag_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->is_rx_flow_tag_enabled;
}

#ifdef WLAN_SUPPORT_RX_FISA
bool wlan_cfg_is_rx_fisa_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return (bool)(cfg->is_rx_fisa_enabled);
}
#else
bool wlan_cfg_is_rx_fisa_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return false;
}
#endif

bool wlan_cfg_is_poll_mode_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return (bool)(cfg->is_poll_mode_enabled);
}

void
wlan_cfg_set_rx_flow_search_table_per_pdev(struct wlan_cfg_dp_soc_ctxt *cfg,
					   bool val)
{
	cfg->is_rx_flow_search_table_per_pdev = val;
}

bool wlan_cfg_is_rx_flow_search_table_per_pdev(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->is_rx_flow_search_table_per_pdev;
}

void wlan_cfg_set_rx_flow_search_table_size(struct wlan_cfg_dp_soc_ctxt *cfg,
					    uint16_t val)
{
	cfg->rx_flow_search_table_size = val;
}

uint16_t
wlan_cfg_get_rx_flow_search_table_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return  cfg->rx_flow_search_table_size;
}

void
wlan_cfg_set_rx_mon_protocol_flow_tag_enabled(struct wlan_cfg_dp_soc_ctxt *cfg,
					      bool val)
{
	cfg->is_rx_mon_protocol_flow_tag_enabled = val;
}

bool
wlan_cfg_is_rx_mon_protocol_flow_tag_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->is_rx_mon_protocol_flow_tag_enabled;
}

void
wlan_cfg_set_tx_per_pkt_vdev_id_check(struct wlan_cfg_dp_soc_ctxt *cfg,
				      bool val)
{
	cfg->tx_per_pkt_vdev_id_check = val;
}

bool
wlan_cfg_is_tx_per_pkt_vdev_id_check_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->tx_per_pkt_vdev_id_check;
}

void
wlan_cfg_set_peer_ext_stats(struct wlan_cfg_dp_soc_ctxt *cfg,
			    bool val)
{
	cfg->pext_stats_enabled = val;
}

bool
wlan_cfg_is_peer_ext_stats_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->pext_stats_enabled;
}

bool wlan_cfg_is_fst_in_cmem_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->fst_in_cmem;
}

#ifdef WLAN_FEATURE_RX_PREALLOC_BUFFER_POOL
bool wlan_cfg_is_rx_buffer_pool_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->is_rx_buff_pool_enabled;
}
#else
bool wlan_cfg_is_rx_buffer_pool_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return false;
}
#endif /* WLAN_FEATURE_RX_PREALLOC_BUFFER_POOL */

#ifdef WLAN_DP_FEATURE_SW_LATENCY_MGR
bool wlan_cfg_is_swlm_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return (bool)(cfg->is_swlm_enabled);
}
#else
bool wlan_cfg_is_swlm_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return false;
}
#endif
