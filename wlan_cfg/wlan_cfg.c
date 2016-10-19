/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
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
#define WLAN_CFG_INT_NUM_CONTEXTS 4
#define RXDMA_BUF_RING_SIZE 8192
#define RXDMA_MONITOR_BUF_RING_SIZE 8192
#define RXDMA_MONITOR_DEST_RING_SIZE 8192
#define RXDMA_MONITOR_STATUS_RING_SIZE 2048

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

#define WLAN_CFG_DP_TX_NUM_POOLS 3
/* Change this to a lower value to enforce scattered idle list mode */
#define WLAN_CFG_MAX_ALLOC_SIZE (32 << 20)

# define WLAN_CFG_MAX_CLIENTS 32

#define WLAN_CFG_PER_PDEV_TX_RING 16
#define WLAN_CFG_NUM_TCL_DATA_RINGS 16
#define WLAN_CFG_PER_PDEV_RX_RING 16
#define WLAN_CFG_NUM_REO_DEST_RING 16

#define WLAN_CFG_NUM_TX_DESC  8192
#define WLAN_CFG_NUM_TX_EXT_DESC 8192
#define WLAN_CFG_NUM_TX_DESC_POOL 16
#define WLAN_CFG_NUM_TXEXT_DESC_POOL 16
#define WLAN_CFG_HTT_PKT_TYPE 2
#define WLAN_CFG_MAX_PEER_ID 16

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
					WLAN_CFG_RX_MON_RING_MASK_0,
					WLAN_CFG_RX_MON_RING_MASK_1,
					WLAN_CFG_RX_MON_RING_MASK_2,
					WLAN_CFG_RX_MON_RING_MASK_3};

/**
 * struct wlan_cfg_dp_soc_ctxt - Configuration parameters for SoC (core TxRx)
 * @num_int_ctxts - Number of NAPI/Interrupt contexts to be registered for DP
 * @max_clients - Maximum number of peers/stations supported by device
 * @max_alloc_size - Maximum allocation size for any dynamic memory
 *			allocation request for this device
 * @per_pdev_tx_ring - 0 - TCL ring is not mapped per radio
 *		       1 - Each TCL ring is mapped to one radio/pdev
 * @num_tcl_data_rings - Number of TCL Data rings supported by device
 * @per_pdev_rx_ring - 0 - REO ring is not mapped per radio
 *		       1 - Each REO ring is mapped to one radio/pdev
 * @num_tx_desc_pool - Number of Tx Descriptor pools
 * @num_tx_ext_desc_pool - Number of Tx MSDU extension Descriptor pools
 * @num_tx_desc - Number of Tx Descriptors per pool
 * @num_tx_ext_desc - Number of Tx MSDU extension Descriptors per pool
 * @max_peer_id - Maximum value of peer id that FW can assign for a client
 * @htt_packet_type - Default 802.11 encapsulation type for any VAP created
 * @int_tx_ring_mask - Bitmap of Tx interrupts mapped to each NAPI/Intr context
 * @int_rx_ring_mask - Bitmap of Rx interrupts mapped to each NAPI/Intr context
 * @int_rx_mon_ring_mask - Bitmap of Rx monitor ring interrupts mapped to each
 *			  NAPI/Intr context
 * @int_ce_ring_mask - Bitmap of CE interrupts mapped to each NAPI/Intr context
 *
 */
struct wlan_cfg_dp_soc_ctxt {
	int num_int_ctxts;
	int max_clients;
	int max_alloc_size;
	int per_pdev_tx_ring;
	int num_tcl_data_rings;
	int per_pdev_rx_ring;
	int num_reo_dest_rings;
	int num_tx_desc_pool;
	int num_tx_ext_desc_pool;
	int num_tx_desc;
	int num_tx_ext_desc;
	int max_peer_id;
	int htt_packet_type;
	int int_tx_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS];
	int int_rx_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS];
	int int_rx_mon_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS];
	int int_ce_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS];
};

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
};

/**
 * wlan_cfg_soc_attach() - Allocate and prepare SoC configuration
 *
 * Return: wlan_cfg_ctx - Handle to Configuration context
 */
struct wlan_cfg_dp_soc_ctxt *wlan_cfg_soc_attach(void)
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
	wlan_cfg_ctx->num_reo_dest_rings = WLAN_CFG_NUM_REO_DEST_RING;
	wlan_cfg_ctx->num_tx_desc_pool = WLAN_CFG_NUM_TX_DESC_POOL;
	wlan_cfg_ctx->num_tx_ext_desc_pool = WLAN_CFG_NUM_TXEXT_DESC_POOL;
	wlan_cfg_ctx->num_tx_desc = WLAN_CFG_NUM_TX_DESC;
	wlan_cfg_ctx->num_tx_ext_desc = WLAN_CFG_NUM_TX_EXT_DESC;
	wlan_cfg_ctx->max_peer_id = WLAN_CFG_MAX_PEER_ID;
	wlan_cfg_ctx->htt_packet_type = WLAN_CFG_HTT_PKT_TYPE;

	for (i = 0; i < WLAN_CFG_INT_NUM_CONTEXTS; i++) {
		wlan_cfg_ctx->int_tx_ring_mask[i] = tx_ring_mask[i];
		wlan_cfg_ctx->int_rx_ring_mask[i] = rx_ring_mask[i];
		wlan_cfg_ctx->int_rx_mon_ring_mask[i] = rx_mon_ring_mask[i];
	}

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

int wlan_cfg_num_tcl_data_rings(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->num_tcl_data_rings;
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

int wlan_cfg_get_num_tx_ext_desc_pool(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->num_tx_ext_desc_pool;
}

int wlan_cfg_get_num_tx_desc(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->num_tx_desc;
}

int wlan_cfg_get_num_tx_ext_desc(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->num_tx_ext_desc;
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
