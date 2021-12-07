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
 * wlan_get_mlo_link_id_from_pdev() - Helper API to get the MLO HW link id
 * from the pdev object.
 * @pdev: Pointer to pdev object
 *
 * Return: On success returns the MLO HW link id corresponding to the pdev
 * object. On failure returns -1.
 */
int8_t
wlan_get_mlo_link_id_from_pdev(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_get_pdev_from_mlo_link_id() - Helper API to get the pdev
 * object from the link id.
 *
 * Return: On success returns the pdev object from the link_id.
 * On failure returns NULL.
 */
struct wlan_objmgr_pdev *
wlan_get_pdev_from_mlo_link_id(uint8_t mlo_link_id);

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

#ifdef WLAN_MGMT_RX_REO_SIM_SUPPORT
/**
 * wlan_mgmt_rx_reo_sim_start() - Helper API to start management Rx reorder
 * simulation
 *
 * This API starts the simulation framework which mimics the management frame
 * generation by target. MAC HW is modelled as a kthread. FW and host layers
 * are modelled as an ordered work queues.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_mgmt_rx_reo_sim_start(void);

/**
 * wlan_mgmt_rx_reo_sim_stop() - Helper API to stop management Rx reorder
 * simulation
 *
 * This API stops the simulation framework which mimics the management frame
 * generation by target. MAC HW is modelled as a kthread. FW and host layers
 * are modelled as an ordered work queues.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_mgmt_rx_reo_sim_stop(void);
#else
/**
 * wlan_mgmt_rx_reo_sim_start() - Helper API to start management Rx reorder
 * simulation
 *
 * Error print is added to indicate that simulation framework is not compiled.
 *
 * Return: QDF_STATUS_E_INVAL
 */
static inline QDF_STATUS
wlan_mgmt_rx_reo_sim_start(void)
{
	mgmt_txrx_err("Mgmt rx reo simulation is not compiled");

	return QDF_STATUS_E_INVAL;
}

/**
 * wlan_mgmt_rx_reo_sim_stop() - Helper API to stop management Rx reorder
 * simulation
 *
 * Error print is added to indicate that simulation framework is not compiled.
 *
 * Return: QDF_STATUS_E_INVAL
 */
static inline QDF_STATUS
wlan_mgmt_rx_reo_sim_stop(void)
{
	mgmt_txrx_err("Mgmt rx reo simulation is not compiled");

	return QDF_STATUS_E_INVAL;
}
#endif /* WLAN_MGMT_RX_REO_SIM_SUPPORT */

/**
 * wlan_mgmt_rx_reo_get_snapshot_address() - Get snapshot address
 * @pdev: pointer to pdev
 * @id: snapshot identifier
 * @address: pointer to snapshot address
 *
 * Helper API to get address of snapshot @id for pdev @pdev.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_mgmt_rx_reo_get_snapshot_address(
			struct wlan_objmgr_pdev *pdev,
			enum mgmt_rx_reo_shared_snapshot_id id,
			struct mgmt_rx_reo_snapshot **address);

/**
 * wlan_mgmt_txrx_process_rx_frame() - API to process the incoming management
 * frame
 * @pdev: pointer to pdev
 * @buf: pointer to buffer
 * @mgmt_rx_params: pointer to management rx params
 *
 * API to process the incoming management frame.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_mgmt_txrx_process_rx_frame(
			struct wlan_objmgr_pdev *pdev,
			qdf_nbuf_t buf,
			struct mgmt_rx_event_params *mgmt_rx_params);

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
