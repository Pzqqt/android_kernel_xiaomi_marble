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

#ifndef _WLAN_MGMT_TXRX_RX_REO_UTILS_API_H_
#define _WLAN_MGMT_TXRX_RX_REO_UTILS_API_H_

/**
 * DOC:  wlan_mgmt_txrx_rx_reo_utils_api.h
 *
 * management rx-reorder public APIs and structures
 * for umac converged components.
 *
 */

#include <wlan_mgmt_txrx_utils_api.h>

struct mgmt_txrx_priv_pdev_context;

#ifdef WLAN_MGMT_RX_REO_SUPPORT
#define mgmt_rx_reo_alert mgmt_txrx_alert
#define mgmt_rx_reo_err mgmt_txrx_err
#define mgmt_rx_reo_warn mgmt_txrx_warn
#define mgmt_rx_reo_notice mgmt_txrx_notice
#define mgmt_rx_reo_info mgmt_txrx_info
#define mgmt_rx_reo_debug mgmt_txrx_debug

/**
 * wlan_mgmt_rx_reo_init() - Initializes the management rx-reorder module
 *
 * This function gets called from dispatcher init and initializes the management
 * rx-reorder module.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_mgmt_rx_reo_init(void);

/**
 * wlan_mgmt_rx_reo_deinit() - De initializes the management rx-reorder module
 *
 * This function gets called from dispatcher deinit and de initializes the
 * management rx-reorder module.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_mgmt_rx_reo_deinit(void);

/**
 * wlan_mgmt_rx_reo_pdev_obj_create_notification() - pdev create handler for
 * management rx-reorder module
 * @pdev: pointer to pdev object
 *
 * This function gets called from object manager when pdev is being created and
 * creates management rx-reorder pdev context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_mgmt_rx_reo_pdev_obj_create_notification(
			struct wlan_objmgr_pdev *pdev,
			struct mgmt_txrx_priv_pdev_context *mgmt_txrx_pdev_ctx);

/**
 * wlan_mgmt_rx_reo_pdev_obj_destroy_notification() - pdev destroy handler for
 * management rx-reorder feature
 * @pdev: pointer to pdev object
 *
 * This function gets called from object manager when pdev is being destroyed
 * and destroys management rx-reorder pdev context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_mgmt_rx_reo_pdev_obj_destroy_notification(
			struct wlan_objmgr_pdev *pdev,
			struct mgmt_txrx_priv_pdev_context *mgmt_txrx_pdev_ctx);

/**
 * wlan_mgmt_rx_reo_get_priv_object() - Get the pdev private object of
 * MGMT Rx REO module
 * @pdev: pointer to pdev object
 * Return: Pointer to pdev private object of MGMT Rx REO module on success,
 * else NULL
 */
struct mgmt_rx_reo_pdev_info *
wlan_mgmt_rx_reo_get_priv_object(struct wlan_objmgr_pdev *pdev);
#else
/**
 * wlan_mgmt_rx_reo_pdev_obj_create_notification() - pdev create handler for
 * management rx-reorder feature
 * @pdev: pointer to pdev object
 *
 * This function gets called from object manager when pdev is being created and
 * creates management rx-reorder pdev context
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS
wlan_mgmt_rx_reo_pdev_obj_create_notification(
			struct wlan_objmgr_pdev *pdev,
			struct mgmt_txrx_priv_pdev_context *mgmt_txrx_pdev_ctx)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_mgmt_rx_reo_pdev_obj_destroy_notification() - pdev destroy handler for
 * management rx-reorder feature
 * @pdev: pointer to pdev object
 *
 * This function gets called from object manager when pdev is being destroyed
 * and destroys management rx-reorder pdev context
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS
wlan_mgmt_rx_reo_pdev_obj_destroy_notification(
			struct wlan_objmgr_pdev *pdev,
			struct mgmt_txrx_priv_pdev_context *mgmt_txrx_pdev_ctx)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_mgmt_rx_reo_init() - Initializes the management rx-reorder module
 *
 * This function gets called from dispatcher init and initializes the management
 * rx-reorder module.
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS
wlan_mgmt_rx_reo_init(void)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_mgmt_rx_reo_deinit() - De initializes the management rx-reorder module
 *
 * This function gets called from dispatcher deinit and de initializes the
 * management rx-reorder module.
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS
wlan_mgmt_rx_reo_deinit(void)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_MGMT_RX_REO_SUPPORT */
#endif /* _WLAN_MGMT_TXRX_RX_REO_UTILS_API_H_ */
