/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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

/**
 * DOC: target_if_wifi_pos.h
 * This file declares the functions pertinent to wifi positioning component's
 * target if layer.
 */
#ifndef _WIFI_POS_TGT_IF_H_
#define _WIFI_POS_TGT_IF_H_

#include "qdf_types.h"
#include "qdf_status.h"
struct oem_data_req;
struct oem_data_rsp;
struct wlan_objmgr_psoc;
struct wlan_soc_southbound_cb;
struct wlan_lmac_if_tx_ops;
struct wlan_lmac_if_rx_ops;

#ifdef WIFI_POS_CONVERGED

/**
 * target_if_wifi_pos_get_txops: api to get tx ops
 * @psoc: pointer to psoc object
 *
 * Return: tx ops
 */
struct wlan_lmac_if_wifi_pos_tx_ops *target_if_wifi_pos_get_txops(
						struct wlan_objmgr_psoc *psoc);

/**
 * target_if_wifi_pos_get_rxops: api to get rx ops
 * @psoc: pointer to psoc object
 *
 * Return: rx ops
 */
struct wlan_lmac_if_wifi_pos_rx_ops *target_if_wifi_pos_get_rxops(
						struct wlan_objmgr_psoc *psoc);

/**
 * target_if_wifi_pos_register_events: function to register with wmi event
 * @psoc: pointer to psoc object
 *
 * Return: status of operation
 */
QDF_STATUS target_if_wifi_pos_register_events(struct wlan_objmgr_psoc *psoc);

/**
 * target_if_wifi_pos_deregister_events: function to deregister wmi event
 * @psoc: pointer to psoc object
 *
 * Return: status of operation
 */
QDF_STATUS target_if_wifi_pos_deregister_events(struct wlan_objmgr_psoc *psoc);


/**
 * target_if_wifi_pos_register_tx_ops: function to register with lmac tx ops
 * @tx_ops: lmac tx ops struct object
 *
 * Return: none
 */
void target_if_wifi_pos_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops);

/**
 * target_if_wifi_pos_register_rx_ops: function to register with lmac rx ops
 * @rx_ops: lmac rx ops struct object
 *
 * Return: none
 */
void target_if_wifi_pos_register_rx_ops(struct wlan_lmac_if_rx_ops *rx_ops);

#else
static inline struct wlan_lmac_if_wifi_pos_tx_ops *target_if_wifi_pos_get_txops(
						struct wlan_objmgr_psoc *psoc)
{
	return NULL;
}

static inline struct wlan_lmac_if_wifi_pos_rx_ops *target_if_wifi_pos_get_rxops(
						struct wlan_objmgr_psoc *psoc)
{
	return NULL;
}

static inline void target_if_wifi_pos_register_tx_ops(
					struct wlan_lmac_if_tx_ops *tx_ops)
{
}

static inline void target_if_wifi_pos_register_rx_ops(
					struct wlan_lmac_if_rx_ops *rx_ops)
{
}
#endif

#if defined(WLAN_FEATURE_CIF_CFR) && defined(WIFI_POS_CONVERGED)
/**
 * target_if_wifi_pos_init_cir_cfr_rings: set DMA ring cap in psoc private
 * object
 * @psoc: pointer to psoc object
 * @hal_soc: pointer to hal soc
 * @num_mac: number of mac
 * @buf: buffer containing DMA ring cap
 *
 * Return: status of operation
 */
QDF_STATUS target_if_wifi_pos_init_cir_cfr_rings(struct wlan_objmgr_psoc *psoc,
					     void *hal_soc, uint8_t num_mac,
					     void *buf);
/**
 * target_if_wifi_pos_deinit_dma_rings: frees up DMA rings
 * @psoc: pointer to psoc
 *
 * Return: status of operation
 */
QDF_STATUS target_if_wifi_pos_deinit_dma_rings(struct wlan_objmgr_psoc *psoc);
#else
static inline QDF_STATUS target_if_wifi_pos_init_cir_cfr_rings(
				struct wlan_objmgr_psoc *psoc, void *hal_soc,
				uint8_t num_mac, void *buf)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS target_if_wifi_pos_deinit_dma_rings(
				struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#endif /* _WIFI_POS_TGT_IF_H_ */
