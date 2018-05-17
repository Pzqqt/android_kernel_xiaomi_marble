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

#if defined(CONFIG_HL_SUPPORT)
#include "wlan_tgt_def_config_hl.h"
#else
#include "wlan_tgt_def_config.h"
#endif

#include "qdf_trace.h"
#include "qdf_mem.h"
#include "wlan_cfg.h"

/*
 * FIX THIS -
 * For now, all these configuration parameters are hardcoded.
 * Many of these should actually be coming from dts file/ini file
 */

#ifdef CONFIG_MCL
#define WLAN_CFG_PER_PDEV_RX_RING 0
#define WLAN_CFG_PER_PDEV_LMAC_RING 0
#define WLAN_LRO_ENABLE 1
#ifdef IPA_OFFLOAD
#define WLAN_CFG_TX_RING_SIZE 2048
#else
#define WLAN_CFG_TX_RING_SIZE 512
#endif
#define WLAN_CFG_TX_COMP_RING_SIZE 1024

/* Tx Descriptor and Tx Extension Descriptor pool sizes */
#define WLAN_CFG_NUM_TX_DESC  1024
#define WLAN_CFG_NUM_TX_EXT_DESC 1024

/* Interrupt Mitigation - Batch threshold in terms of number of frames */
#define WLAN_CFG_INT_BATCH_THRESHOLD_TX 1
#define WLAN_CFG_INT_BATCH_THRESHOLD_RX 1
#define WLAN_CFG_INT_BATCH_THRESHOLD_OTHER 1

/* Interrupt Mitigation - Timer threshold in us */
#define WLAN_CFG_INT_TIMER_THRESHOLD_TX 8
#define WLAN_CFG_INT_TIMER_THRESHOLD_RX 8
#define WLAN_CFG_INT_TIMER_THRESHOLD_OTHER 8
#endif

#ifdef CONFIG_WIN
#define WLAN_CFG_PER_PDEV_RX_RING 0
#define WLAN_CFG_PER_PDEV_LMAC_RING 1
#define WLAN_LRO_ENABLE 0

/* Tx Descriptor and Tx Extension Descriptor pool sizes */
#define WLAN_CFG_NUM_TX_DESC  (32 << 10)
#define WLAN_CFG_NUM_TX_EXT_DESC (8 << 10)


/* Interrupt Mitigation - Batch threshold in terms of number of frames */
#define WLAN_CFG_INT_BATCH_THRESHOLD_TX 256
#define WLAN_CFG_INT_BATCH_THRESHOLD_RX 128
#define WLAN_CFG_INT_BATCH_THRESHOLD_OTHER 1

/* Interrupt Mitigation - Timer threshold in us */
#define WLAN_CFG_INT_TIMER_THRESHOLD_TX 1000
#define WLAN_CFG_INT_TIMER_THRESHOLD_RX 500
#define WLAN_CFG_INT_TIMER_THRESHOLD_OTHER 1000

#define WLAN_CFG_TX_RING_SIZE 512

/* Size the completion ring using following 2 parameters
 *  - NAPI schedule latency (assuming 1 netdev competing for CPU) = 20 ms (2 jiffies)
 *  - Worst case PPS requirement = 400K PPS
 *
 * Ring size = 20 * 400 = 8000
 * 8192 is nearest power of 2
 */
#define WLAN_CFG_TX_COMP_RING_SIZE (8 << 10)
#endif

/*
 * The max allowed size for tx comp ring is 8191.
 * This is limitted by h/w ring max size.
 * As this is not a power of 2 it does not work with nss offload so the
 * nearest available size which is power of 2 is 4096 chosen for nss
 */
#define NSS_TX_COMP_RING_SIZE (4 << 10)

#define RXDMA_BUF_RING_SIZE 2048
#define RXDMA_MONITOR_BUF_RING_SIZE 2048
#define RXDMA_MONITOR_DEST_RING_SIZE 2048
#define RXDMA_MONITOR_STATUS_RING_SIZE 2048

#ifdef QCA_LL_TX_FLOW_CONTROL_V2

/* Per vdev pools */
#define WLAN_CFG_NUM_TX_DESC_POOL	3
#define WLAN_CFG_NUM_TXEXT_DESC_POOL	3

#else /* QCA_LL_TX_FLOW_CONTROL_V2 */

#ifdef TX_PER_PDEV_DESC_POOL
#define WLAN_CFG_NUM_TX_DESC_POOL	MAX_PDEV_CNT
#define WLAN_CFG_NUM_TXEXT_DESC_POOL	MAX_PDEV_CNT

#else /* TX_PER_PDEV_DESC_POOL */

#define WLAN_CFG_NUM_TX_DESC_POOL 3
#define WLAN_CFG_NUM_TXEXT_DESC_POOL 3

#endif /* TX_PER_PDEV_DESC_POOL */
#endif /* QCA_LL_TX_FLOW_CONTROL_V2 */

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

#define WLAN_CFG_DP_TX_NUM_POOLS 3
/* Change this to a lower value to enforce scattered idle list mode */
#define WLAN_CFG_MAX_ALLOC_SIZE (2 << 20)

#define WLAN_CFG_MAX_CLIENTS 64

#ifdef CONFIG_MCL
#ifdef IPA_OFFLOAD
#define WLAN_CFG_PER_PDEV_TX_RING 0
#else
#define WLAN_CFG_PER_PDEV_TX_RING 1
#endif
#else
#define WLAN_CFG_PER_PDEV_TX_RING 0
#endif

#define WLAN_CFG_NUM_TCL_DATA_RINGS 3
#define WLAN_CFG_NUM_REO_DEST_RING 4

#define WLAN_CFG_HTT_PKT_TYPE 2
#define WLAN_CFG_MAX_PEER_ID 64

#define WLAN_CFG_RX_DEFRAG_TIMEOUT 100

#ifdef CONFIG_MCL
static const int tx_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS] = {
						0,
						WLAN_CFG_TX_RING_MASK_0,
						0,
						0,
						0,
						0,
						0};

static const int rx_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS] = {
					0,
					0,
					WLAN_CFG_RX_RING_MASK_0,
					0,
					WLAN_CFG_RX_RING_MASK_1,
					WLAN_CFG_RX_RING_MASK_2,
					WLAN_CFG_RX_RING_MASK_3};

static const int rx_mon_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS] = {
					0,
					0,
					0,
					WLAN_CFG_RX_MON_RING_MASK_0,
					WLAN_CFG_RX_MON_RING_MASK_1,
					WLAN_CFG_RX_MON_RING_MASK_2,
					WLAN_CFG_RX_MON_RING_MASK_3};
#else
static const int tx_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS] = {
						WLAN_CFG_TX_RING_MASK_0,
						WLAN_CFG_TX_RING_MASK_1,
						WLAN_CFG_TX_RING_MASK_2,
						WLAN_CFG_TX_RING_MASK_3};

static const int rx_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS] = {
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

#endif

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

static const int rxdma2host_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS] = {
					WLAN_CFG_RXDMA2HOST_RING_MASK_0,
					WLAN_CFG_RXDMA2HOST_RING_MASK_1,
					WLAN_CFG_RXDMA2HOST_RING_MASK_2,
					WLAN_CFG_RXDMA2HOST_RING_MASK_3};

static const int host2rxdma_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS] = {
					WLAN_CFG_HOST2RXDMA_RING_MASK_0,
					WLAN_CFG_HOST2RXDMA_RING_MASK_1,
					WLAN_CFG_HOST2RXDMA_RING_MASK_2,
					WLAN_CFG_HOST2RXDMA_RING_MASK_3};

/**
 * struct wlan_cfg_dp_pdev_ctxt - Configuration parameters for pdev (radio)
 * @rx_dma_buf_ring_size - Size of RxDMA buffer ring
 * @dma_mon_buf_ring_size - Size of RxDMA Monitor buffer ring
 * @dma_mon_dest_ring_size - Size of RxDMA Monitor Destination ring
 * @dma_mon_status_ring_size - Size of RxDMA Monitor Status ring
 */
struct wlan_cfg_dp_pdev_ctxt {
	int rx_dma_buf_ring_size;
	int dma_mon_buf_ring_size;
	int dma_mon_dest_ring_size;
	int dma_mon_status_ring_size;
	int num_mac_rings;
	int nss_enabled;
};

/**
 * wlan_cfg_soc_attach() - Allocate and prepare SoC configuration
 *
 * Return: wlan_cfg_ctx - Handle to Configuration context
 */
struct wlan_cfg_dp_soc_ctxt *wlan_cfg_soc_attach()
{
	int i = 0;

	struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx =
		qdf_mem_malloc(sizeof(struct wlan_cfg_dp_soc_ctxt));

	if (wlan_cfg_ctx == NULL)
		return NULL;

	wlan_cfg_ctx->num_int_ctxts = WLAN_CFG_INT_NUM_CONTEXTS;
	wlan_cfg_ctx->max_clients = WLAN_CFG_MAX_CLIENTS;
	wlan_cfg_ctx->max_alloc_size = WLAN_CFG_MAX_ALLOC_SIZE;
	wlan_cfg_ctx->per_pdev_tx_ring = WLAN_CFG_PER_PDEV_TX_RING;
	wlan_cfg_ctx->num_tcl_data_rings = WLAN_CFG_NUM_TCL_DATA_RINGS;
	wlan_cfg_ctx->per_pdev_rx_ring = WLAN_CFG_PER_PDEV_RX_RING;
	wlan_cfg_ctx->per_pdev_lmac_ring = WLAN_CFG_PER_PDEV_LMAC_RING;
	wlan_cfg_ctx->num_reo_dest_rings = WLAN_CFG_NUM_REO_DEST_RING;
	wlan_cfg_ctx->num_tx_desc_pool = MAX_TXDESC_POOLS;
	wlan_cfg_ctx->num_tx_ext_desc_pool = WLAN_CFG_NUM_TXEXT_DESC_POOL;
	wlan_cfg_ctx->num_tx_desc = WLAN_CFG_NUM_TX_DESC;
	wlan_cfg_ctx->num_tx_ext_desc = WLAN_CFG_NUM_TX_EXT_DESC;
	wlan_cfg_ctx->htt_packet_type = WLAN_CFG_HTT_PKT_TYPE;
	wlan_cfg_ctx->max_peer_id = WLAN_CFG_MAX_PEER_ID;

	wlan_cfg_ctx->tx_ring_size = WLAN_CFG_TX_RING_SIZE;
	wlan_cfg_ctx->tx_comp_ring_size = WLAN_CFG_TX_COMP_RING_SIZE;

	wlan_cfg_ctx->int_batch_threshold_tx = WLAN_CFG_INT_BATCH_THRESHOLD_TX;
	wlan_cfg_ctx->int_timer_threshold_tx =  WLAN_CFG_INT_TIMER_THRESHOLD_TX;
	wlan_cfg_ctx->int_batch_threshold_rx = WLAN_CFG_INT_BATCH_THRESHOLD_RX;
	wlan_cfg_ctx->int_timer_threshold_rx = WLAN_CFG_INT_TIMER_THRESHOLD_RX;
	wlan_cfg_ctx->int_batch_threshold_other =
		WLAN_CFG_INT_BATCH_THRESHOLD_OTHER;
	wlan_cfg_ctx->int_timer_threshold_other =
		WLAN_CFG_INT_TIMER_THRESHOLD_OTHER;

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
	}

	/* This is default mapping and can be overridden by HW config
	 * received from FW */
	wlan_cfg_set_hw_macid(wlan_cfg_ctx, 0, 1);
	if (MAX_PDEV_CNT > 1)
		wlan_cfg_set_hw_macid(wlan_cfg_ctx, 1, 3);
	if (MAX_PDEV_CNT > 2)
		wlan_cfg_set_hw_macid(wlan_cfg_ctx, 2, 2);

	wlan_cfg_ctx->base_hw_macid = 1;
	/*Enable checksum offload by default*/
	wlan_cfg_ctx->tcp_udp_checksumoffload = 1;

	wlan_cfg_ctx->defrag_timeout_check = 1;
	wlan_cfg_ctx->rx_defrag_min_timeout = WLAN_CFG_RX_DEFRAG_TIMEOUT;

	return wlan_cfg_ctx;
}

void wlan_cfg_soc_detach(struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx)
{
	qdf_mem_free(wlan_cfg_ctx);
}

struct wlan_cfg_dp_pdev_ctxt *wlan_cfg_pdev_attach(void)
{
	struct wlan_cfg_dp_pdev_ctxt *wlan_cfg_ctx =
		qdf_mem_malloc(sizeof(struct wlan_cfg_dp_pdev_ctxt));

	if (wlan_cfg_ctx == NULL)
		return NULL;

	wlan_cfg_ctx->rx_dma_buf_ring_size = RXDMA_BUF_RING_SIZE;
	wlan_cfg_ctx->dma_mon_buf_ring_size = RXDMA_MONITOR_BUF_RING_SIZE;
	wlan_cfg_ctx->dma_mon_dest_ring_size = RXDMA_MONITOR_DEST_RING_SIZE;
	wlan_cfg_ctx->dma_mon_status_ring_size = RXDMA_MONITOR_STATUS_RING_SIZE;
	wlan_cfg_ctx->num_mac_rings = NUM_RXDMA_RINGS_PER_PDEV;

	return wlan_cfg_ctx;
}

void wlan_cfg_pdev_detach(struct wlan_cfg_dp_pdev_ctxt *wlan_cfg_ctx)
{
	qdf_mem_free(wlan_cfg_ctx);
}

void wlan_cfg_set_num_contexts(struct wlan_cfg_dp_soc_ctxt *cfg, int num)
{
	cfg->num_int_ctxts = num;
}

void wlan_cfg_set_max_peer_id(struct wlan_cfg_dp_soc_ctxt *cfg, uint32_t val)
{
	cfg->max_peer_id = val;;
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

int wlan_cfg_get_num_tx_desc(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->num_tx_desc;
}

void wlan_cfg_set_num_tx_desc(struct wlan_cfg_dp_soc_ctxt *cfg, int num_desc)
{
	cfg->num_tx_desc = num_desc;
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

int wlan_cfg_get_rx_dma_buf_ring_size(struct wlan_cfg_dp_pdev_ctxt *cfg)
{
	return  cfg->rx_dma_buf_ring_size;
}

int wlan_cfg_get_num_mac_rings(struct wlan_cfg_dp_pdev_ctxt *cfg)
{
	return  cfg->num_mac_rings;
}

bool wlan_cfg_is_lro_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return  cfg->lro_enabled;
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
		cfg->tx_comp_ring_size = NSS_TX_COMP_RING_SIZE;
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
