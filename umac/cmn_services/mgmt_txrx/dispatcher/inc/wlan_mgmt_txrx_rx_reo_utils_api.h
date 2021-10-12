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

/**
 * TODO: Dummy function to get the MLO link ID from the pdev.
 * This is added only as a place holder for the time being.
 * Remove this once this API is implemented in MLO manager.
 */
static inline uint8_t
wlan_get_mlo_link_id_from_pdev(struct wlan_objmgr_pdev *pdev)
{
	return 0;
}

/**
 * TODO: Dummy function to get pdev handle from MLO link ID.
 * This is added only as a place holder for the time being.
 * Remove this once this API is implemented in MLO manager.
 */
static inline struct wlan_objmgr_pdev *
wlan_get_pdev_from_mlo_link_id(uint8_t mlo_link_id)
{
	return NULL;
}

#ifdef WLAN_MGMT_RX_REO_SUPPORT

#define mgmt_rx_reo_alert(params...) \
	QDF_TRACE_FATAL(QDF_MODULE_ID_MGMT_RX_REO, params)
#define mgmt_rx_reo_err(params...) \
	QDF_TRACE_ERROR(QDF_MODULE_ID_MGMT_RX_REO, params)
#define mgmt_rx_reo_warn(params...) \
	QDF_TRACE_WARN(QDF_MODULE_ID_MGMT_RX_REO, params)
#define mgmt_rx_reo_notice(params...) \
	QDF_TRACE_INFO(QDF_MODULE_ID_MGMT_RX_REO, params)
#define mgmt_rx_reo_info(params...) \
	QDF_TRACE_INFO(QDF_MODULE_ID_MGMT_RX_REO, params)
#define mgmt_rx_reo_debug(params...) \
	QDF_TRACE_DEBUG(QDF_MODULE_ID_MGMT_RX_REO, params)

#define mgmt_rx_reo_alert_rl(params...) \
	QDF_TRACE_FATAL_RL(QDF_MODULE_ID_MGMT_RX_REO, params)
#define mgmt_rx_reo_err_rl(params...) \
	QDF_TRACE_ERROR_RL(QDF_MODULE_ID_MGMT_RX_REO, params)
#define mgmt_rx_reo_warn_rl(params...) \
	QDF_TRACE_WARN_RL(QDF_MODULE_ID_MGMT_RX_REO, params)
#define mgmt_rx_reo_notice_rl(params...) \
	QDF_TRACE_INFO_RL(QDF_MODULE_ID_MGMT_RX_REO, params)
#define mgmt_rx_reo_info_rl(params...) \
	QDF_TRACE_INFO_RL(QDF_MODULE_ID_MGMT_RX_REO, params)
#define mgmt_rx_reo_debug_rl(params...) \
	QDF_TRACE_DEBUG_RL(QDF_MODULE_ID_MGMT_RX_REO, params)

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
 * @mgmt_txrx_pdev_ctx: pdev private object of mgmt txrx module
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
 * @mgmt_txrx_pdev_ctx: pdev private object of mgmt txrx module
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
 * wlan_mgmt_rx_reo_is_feature_enabled_at_psoc() - Check if MGMT Rx REO feature
 * is enabled on a given psoc
 * @psoc: pointer to psoc object
 *
 * Return: true if the feature is enabled, else false
 */
bool
wlan_mgmt_rx_reo_is_feature_enabled_at_psoc(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_mgmt_rx_reo_is_feature_enabled_at_pdev() - Check if MGMT Rx REO feature
 * is enabled on a given pdev
 * @psoc: pointer to pdev object
 *
 * Return: true if the feature is enabled, else false
 */
bool
wlan_mgmt_rx_reo_is_feature_enabled_at_pdev(struct wlan_objmgr_pdev *pdev);
#else
/**
 * wlan_mgmt_rx_reo_pdev_obj_create_notification() - pdev create handler for
 * management rx-reorder feature
 * @pdev: pointer to pdev object
 * @mgmt_txrx_pdev_ctx: pdev private object of mgmt txrx module
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
 * @mgmt_txrx_pdev_ctx: pdev private object of mgmt txrx module
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
