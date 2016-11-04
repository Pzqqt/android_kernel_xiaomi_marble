/*
 * Copyright (c) 2013-2016 The Linux Foundation. All rights reserved.
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

#ifndef __WLAN_CFG_H
#define __WLAN_CFG_H

struct wlan_cfg_dp_pdev_ctxt;
struct wlan_cfg_dp_soc_ctxt;

/**
 * wlan_cfg_soc_attach() - Attach configuration interface for SoC
 *
 * Allocates context for Soc configuration parameters,
 * Read configuration information from device tree/ini file and
 * returns back handle
 *
 * Return: Handle to configuration context
 */
struct wlan_cfg_dp_soc_ctxt *wlan_cfg_soc_attach(void);

/**
 * wlan_cfg_soc_detach() - Detach soc configuration handle
 * @wlan_cfg_ctx: soc configuration handle
 *
 * De-allocates memory allocated for SoC configuration
 *
 * Return:none
 */
void wlan_cfg_soc_detach(struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx);

/**
 * wlan_cfg_pdev_attach() Attach configuration interface for pdev
 *
 * Allocates context for pdev configuration parameters,
 * Read configuration information from device tree/ini file and
 * returns back handle
 *
 * Return: Handle to configuration context
 */
struct wlan_cfg_dp_pdev_ctxt *wlan_cfg_pdev_attach(void);

/**
 * wlan_cfg_pdev_detach() Detach and free pdev configuration handle
 * @wlan_cfg_pdev_ctx - PDEV Configuration Handle
 *
 * Return: void
 */
void wlan_cfg_pdev_detach(struct wlan_cfg_dp_pdev_ctxt *wlan_cfg_pdev_ctx);

/**
 * wlan_cfg_get_num_contexts()  Number of interrupt contexts to be registered
 * @wlan_cfg_ctx - Configuration Handle
 *
 * For WIN,  DP_NUM_INTERRUPT_CONTEXTS will be equal to  number of CPU cores.
 * Each context (for linux it is a NAPI context) will have a tx_ring_mask,
 * rx_ring_mask ,and rx_monitor_ring mask  to indicate the rings
 * that are processed by the handler.
 *
 * Return: num_contexts
 */
int wlan_cfg_get_num_contexts(struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx);

/**
 * wlan_cfg_get_intr_tx_ring_mask() - Return Tx interrupt mask mapped to an
 *				      interrupt context
 * @wlan_cfg_ctx - Configuration Handle
 * @context - Numerical ID identifying the Interrupt/NAPI context
 *
 * Return: int_tx_ring_mask[context]
 */
int wlan_cfg_get_tx_ring_mask(struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx,
		int context);

/**
 * wlan_cfg_get_intr_rx_ring_mask() - Return Rx interrupt mask mapped to an
 *				      interrupt context
 * @wlan_cfg_ctx - Configuration Handle
 * @context - Numerical ID identifying the Interrupt/NAPI context
 *
 * Return: int_rx_ring_mask[context]
 */
int wlan_cfg_get_rx_ring_mask(struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx,
		int context);

/**
 * wlan_cfg_get_intr_rx_mon_ring_mask() - Return Rx monitor ring interrupt mask
 *					   mapped to an interrupt context
 * @wlan_cfg_ctx - Configuration Handle
 * @context - Numerical ID identifying the Interrupt/NAPI context
 *
 * Return: int_rx_mon_ring_mask[context]
 */
int wlan_cfg_get_rx_mon_ring_mask(struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx,
		int context);

/**
 * wlan_cfg_get_intr_ce_ring_mask() - Return CE ring interrupt mask
 *				mapped to an interrupt context
 * @wlan_cfg_ctx - Configuration Handle
 * @context - Numerical ID identifying the Interrupt/NAPI context
 *
 * Return: int_ce_ring_mask[context]
 */
int wlan_cfg_get_ce_ring_mask(struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx,
		int context);

/**
 * wlan_cfg_get_num_tx_pools() - Return Number of Tx Descriptor pools for SoC
 * @wlan_cfg_ctx - Configuration Handle
 *
 * Return: num_tx_pools
 */
int wlan_cfg_get_num_tx_pools(struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx);

/**
 * wlan_cfg_get_max_clients() Return maximum number of peers/stations supported
 *			      by device
 * @wlan_cfg_ctx - Configuration Handle
 *
 * Return: max_clients
 */
uint32_t wlan_cfg_get_max_clients(struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx);

/**
 * wlan_cfg_max_alloc_size() Return Maximum allocation size for any dynamic
 *			    memory allocation request for this device
 * @wlan_cfg_ctx - Configuration Handle
 *
 * Return: max_alloc_size
 */
uint32_t wlan_cfg_max_alloc_size(struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx);

/*
 * wlan_cfg_per_pdev_tx_ring() Return true if Tx rings are mapped as
 *			       one per radio
 * @wlan_cfg_ctx - Configuration Handle
 *
 * Return: per_pdev_tx_ring
 */
int wlan_cfg_per_pdev_tx_ring(struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx);

/*
 * wlan_cfg_num_tcl_data_rings() Number of TCL Data rings supported by device
 * @wlan_cfg_ctx
 *
 * Return: num_tcl_data_rings
 */
int wlan_cfg_num_tcl_data_rings(struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx);

/*
 * wlan_cfg_per_pdev_rx_ring()  Return true if Rx rings are mapped as
 *                              one per radio
 * @wlan_cfg_ctx
 *
 * Return: per_pdev_rx_ring
 */
int wlan_cfg_per_pdev_rx_ring(struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx);

/*
 * wlan_cfg_num_reo_dest_rings() Number of REO Data rings supported by device
 * @wlan_cfg_ctx - Configuration Handle
 *
 * Return: num_reo_dest_rings
 */
int wlan_cfg_num_reo_dest_rings(struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx);

/*
 * wlan_cfg_pkt_type() Default 802.11 encapsulation type
 * @wlan_cfg_ctx - Configuration Handle
 *
 * Return: htt_pkt_type_ethernet
 */
int wlan_cfg_pkt_type(struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx);

/*
 * wlan_cfg_get_num_tx_desc_pool() Number of Tx Descriptor pools for the device
 * @wlan_cfg_ctx - Configuration Handle
 *
 * Return: num_tx_desc_pool
 */
int wlan_cfg_get_num_tx_desc_pool(struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx);

/*
 * wlan_cfg_get_num_tx_ext_desc_pool Number of Tx MSDU ext Descriptor pools
 * @wlan_cfg_ctx - Configuration Handle
 *
 * Return: num_tx_ext_desc_pool
 */
int wlan_cfg_get_num_tx_ext_desc_pool(
		struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx);

/*
 * wlan_cfg_get_num_tx_desc Number of Tx Descriptors per pool
 * @wlan_cfg_ctx - Configuration Handle
 *
 * Return: num_tx_desc
 */
int wlan_cfg_get_num_tx_desc(struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx);

/*
 * wlan_cfg_get_num_tx_ext_desc Number of Tx MSDU extension Descriptors per pool
 * @wlan_cfg_ctx - Configuration Handle
 *
 * Return: num_tx_ext_desc
 */
int wlan_cfg_get_num_tx_ext_desc(struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx);

/*
 * wlan_cfg_get_dma_mon_buf_ring_size() - Return Size of monitor buffer ring
 * @wlan_cfg_pdev_ctx
 *
 * Return: dma_mon_buf_ring_size
 */
int wlan_cfg_get_dma_mon_buf_ring_size(
		struct wlan_cfg_dp_pdev_ctxt *wlan_cfg_pdev_ctx);

/*
 * wlan_cfg_get_dma_mon_dest_ring_size() - Return Size of RxDMA Monitor
 *					   Destination ring
 * @wlan_cfg_pdev_ctx
 *
 * Return: dma_mon_dest_size
 */
int wlan_cfg_get_dma_mon_dest_ring_size(
		struct wlan_cfg_dp_pdev_ctxt *wlan_cfg_pdev_ctx);

/*
 * wlan_cfg_get_dma_mon_stat_ring_size - Return size of Monitor Status ring
 * @wlan_cfg_pdev_ctx
 *
 * Return: dma_mon_stat_ring_size
 */
int wlan_cfg_get_dma_mon_stat_ring_size(
		struct wlan_cfg_dp_pdev_ctxt *wlan_cfg_pdev_ctx);

/*
 * wlan_cfg_get_rx_dma_buf_ring_size - Return Size of RxDMA buffer ring
 * @wlan_cfg_pdev_ctx
 *
 * Return: rx_dma_buf_ring_size
 */
int wlan_cfg_get_rx_dma_buf_ring_size(
		struct wlan_cfg_dp_pdev_ctxt *wlan_cfg_pdev_ctx);

#endif
