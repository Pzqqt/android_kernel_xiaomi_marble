/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 *  DOC: target_if_mgmt_txrx_rx_reo.h
 *  This file contains declarations of management rx re-ordering related APIs.
 */

#ifndef _TARGET_IF_MGMT_TXRX_RX_REO_H_
#define _TARGET_IF_MGMT_TXRX_RX_REO_H_

#include <qdf_types.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_mgmt_txrx_rx_reo_utils_api.h>
#include <wlan_mgmt_txrx_rx_reo_tgt_api.h>
#include <wlan_lmac_if_api.h>
#include <wlan_lmac_if_def.h>
#include <wmi_unified_param.h>

#ifdef WLAN_MGMT_RX_REO_SUPPORT

#define MGMT_RX_REO_SNAPSHOT_READ_RETRY_LIMIT (5)

/**
 * target_if_get_mgmt_rx_reo_low_level_ops() - Get low-level ops of management
 * rx-reorder module
 * @psoc: Pointer to psoc object
 *
 * Return: Pointer to management rx-reorder low-level ops
 */
static inline struct wlan_lmac_if_mgmt_rx_reo_low_level_ops *
target_if_get_mgmt_rx_reo_low_level_ops(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_mgmt_rx_reo_tx_ops *mgmt_rx_reo_tx_ops;

	mgmt_rx_reo_tx_ops = wlan_psoc_get_mgmt_rx_reo_txops(psoc);
	if (!mgmt_rx_reo_tx_ops) {
		mgmt_txrx_err("txops is null for mgmt rx reo module");
		return NULL;
	}

	return &mgmt_rx_reo_tx_ops->low_level_ops;
}

/**
 * target_if_mgmt_rx_reo_register_event_handlers() - Register management
 * rx-reordering related event handlers.
 * @psoc: psoc object
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
target_if_mgmt_rx_reo_register_event_handlers(struct wlan_objmgr_psoc *psoc);

/**
 * target_if_mgmt_rx_reo_unregister_event_handlers() - Unregister management
 * rx-reordering related event handlers.
 * @psoc: psoc object
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
target_if_mgmt_rx_reo_unregister_event_handlers(struct wlan_objmgr_psoc *psoc);

/**
 * target_if_mgmt_rx_reo_tx_ops_register() - Register management rx-reordring
 * txops
 * @mgmt_txrx_tx_ops: Pointer to mgmt_txrx module's txops
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
target_if_mgmt_rx_reo_tx_ops_register(
		struct wlan_lmac_if_mgmt_txrx_tx_ops *mgmt_txrx_tx_ops);

/**
 * target_if_mgmt_rx_reo_get_rx_ops() - Retrieve rx_ops of MGMT Rx REO module
 * @psoc:psoc context
 *
 * Return: Pointer to rx_ops of MGMT Rx REO module
 */
static struct wlan_lmac_if_mgmt_rx_reo_rx_ops *
target_if_mgmt_rx_reo_get_rx_ops(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_mgmt_txrx_rx_ops *mgmt_rx_ops;

	mgmt_rx_ops = wlan_lmac_if_get_mgmt_txrx_rx_ops(psoc);
	if (!mgmt_rx_ops) {
		mgmt_rx_reo_err("MGMT TxRx rx_ops is NULL");
		return NULL;
	}

	return &mgmt_rx_ops->mgmt_rx_reo_rx_ops;
}

/**
 * target_if_mgmt_rx_reo_extract_reo_params() - Extract MGMT Rx REO params from
 * MGMT_RX_EVENT_ID
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @params: Pointer to MGMT Rx event parameters
 *
 * Return: QDF_STATUS of operation
 */
QDF_STATUS
target_if_mgmt_rx_reo_extract_reo_params(wmi_unified_t wmi_handle,
					 void *evt_buf,
					 struct mgmt_rx_event_params *params);
/**
 * target_if_mgmt_rx_reo_host_drop_handler() - MGMT Rx REO handler for the
 * management Rx frames that gets dropped in the Host before entering
 * MGMT Rx REO algorithm
 * @pdev: pdev for which this frame was intended
 * @params: MGMT Rx event parameters
 *
 * Return: QDF_STATUS of operation
 */
QDF_STATUS
target_if_mgmt_rx_reo_host_drop_handler(struct wlan_objmgr_pdev *pdev,
					struct mgmt_rx_event_params *params);
#else
/**
 * target_if_mgmt_rx_reo_register_event_handlers() - Register management
 * rx-reordering related event handlers.
 * @psoc: psoc object
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS
target_if_mgmt_rx_reo_register_event_handlers(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_mgmt_rx_reo_unregister_event_handlers() - Unregister management
 * rx-reordering related event handlers.
 * @psoc: psoc object
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS
target_if_mgmt_rx_reo_unregister_event_handlers(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_mgmt_rx_reo_tx_ops_register() - Register management rx-reordring
 * txops
 * @mgmt_txrx_tx_ops: Pointer to mgmt_txrx module's txops
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS
target_if_mgmt_rx_reo_tx_ops_register(
		struct wlan_lmac_if_mgmt_txrx_tx_ops *mgmt_txrx_tx_ops)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_mgmt_rx_reo_extract_reo_params() - Extract MGMT Rx REO params from
 * MGMT_RX_EVENT_ID
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @hdr: Pointer to MGMT Rx event parameters
 *
 * Return: QDF_STATUS of operation
 */
static inline QDF_STATUS
target_if_mgmt_rx_reo_extract_reo_params(wmi_unified_t wmi_handle,
					 void *evt_buf,
					 struct mgmt_rx_event_params *hdr)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_mgmt_rx_reo_host_drop_handler() - MGMT Rx REO handler for the
 * management Rx frames that gets dropped in the Host before entering
 * MGMT Rx REO algorithm
 * @pdev: pdev for which this frame was intended
 * @params: MGMT Rx event parameters
 *
 * Return: QDF_STATUS of operation
 */
static inline QDF_STATUS
target_if_mgmt_rx_reo_host_drop_handler(struct wlan_objmgr_pdev *pdev,
					struct mgmt_rx_event_params *params)
{
	/* Nothing to do when REO is compiled off */
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_MGMT_RX_REO_SUPPORT */
#endif /* _TARGET_IF_MGMT_TXRX_RX_REO_H_ */
