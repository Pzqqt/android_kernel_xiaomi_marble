/*
 * Copyright (c) 2012-2015, 2020 The Linux Foundation. All rights reserved.
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
 * DOC: osif_cm_util.h
 *
 * This header file maintains declarations of connect, disconnect, roam
 * common apis.
 */

#ifndef __OSIF_CM_UTIL_H
#define __OSIF_CM_UTIL_H

#include <qca_vendor.h>
#include "wlan_cm_ucfg_api.h"
#include "wlan_cm_public_struct.h"
#ifdef CONN_MGR_ADV_FEATURE
#include <cdp_txrx_mob_def.h>
#endif

#ifdef FEATURE_CM_ENABLE
/**
 * osif_cm_qca_reason_to_str() - return string conversion of qca reason code
 * @reason: enum qca_disconnect_reason_codes
 *
 * This utility function helps log string conversion of qca reason code.
 *
 * Return: string conversion of reason code, if match found;
 *         "Unknown" otherwise.
 */
const char *
osif_cm_qca_reason_to_str(enum qca_disconnect_reason_codes reason);

/**
 * osif_cm_mac_to_qca_reason() - Convert to qca internal disconnect reason
 * @internal_reason: Mac reason code of type @wlan_reason_code
 *
 * Check if it is internal reason code and convert it to the
 * enum qca_disconnect_reason_codes.
 *
 * Return: Reason code of type enum qca_disconnect_reason_codes
 */
enum qca_disconnect_reason_codes
osif_cm_mac_to_qca_reason(enum wlan_reason_code internal_reason);

/**
 * osif_cm_register_cb() - API to register connection manager
 * callbacks.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS osif_cm_register_cb(void);

/**
 * osif_cm_osif_priv_init() - API to init osif priv data for connection manager
 * @vdev: vdev pointer
 *
 * Return: QDF_STATUS
 */
QDF_STATUS osif_cm_osif_priv_init(struct wlan_objmgr_vdev *vdev);

/**
 * osif_cm_osif_priv_deinit() - API to deinit osif priv data for connection
 * manager
 * @vdev: vdev pointer
 *
 * Return: QDF_STATUS
 */
QDF_STATUS osif_cm_osif_priv_deinit(struct wlan_objmgr_vdev *vdev);

/**
 * osif_cm_reset_id_and_src_no_lock() - Function to resets last
 * connection manager command id and source in osif
 * @osif_priv: Pointer to vdev osif priv
 *
 * This function resets the last connection manager command id
 * and source.
 *
 * Context: Any context. This function should be called by holding
 * cmd id spinlock
 * Return: None
 */

void osif_cm_reset_id_and_src_no_lock(struct vdev_osif_priv *osif_priv);

/**
 * osif_cm_reset_id_and_src() - Function to resets last
 * connection manager command id and source in osif
 * @vdev: vdev pointer
 *
 * This function resets the last connection manager command id
 * and source.
 *
 * Context: Any context. Takes and release cmd id spinlock
 * Return: None
 */
QDF_STATUS osif_cm_reset_id_and_src(struct wlan_objmgr_vdev *vdev);

/**
 * enum osif_cb_type - Type of the update from osif to legacy module
 * @OSIF_POST_USERSPACE_UPDATE: Indicates that when this update is received
 * userspace is already updated.
 * @OSIF_PRE_USERSPACE_UPDATE: Indicates that when this update is received
 * userspace is not yet updated.
 * @OSIF_NOT_HANDLED: Indicates that last command is not handled
 */

enum osif_cb_type {
	OSIF_POST_USERSPACE_UPDATE,
	OSIF_PRE_USERSPACE_UPDATE,
	OSIF_NOT_HANDLED,
};

/**
 * typedef osif_cm_connect_comp_cb  - Connect complete callback
 * @vdev: vdev pointer
 * @rsp: connect response
 * @type: indicates update type
 *
 * This callback indicates connect complete to the legacy module
 *
 * Context: Any context.
 * Return: QDF_STATUS
 */
typedef QDF_STATUS
	(*osif_cm_connect_comp_cb)(struct wlan_objmgr_vdev *vdev,
				   struct wlan_cm_connect_rsp *rsp,
				   enum osif_cb_type type);

/**
 * typedef  osif_cm_disconnect_comp_cb: Disonnect complete callback
 * @vdev: vdev pointer
 * @rsp: disconnect response
 * @type: indicates update type
 *
 * This callback indicates disconnect complete to the legacy module
 *
 * Context: Any context.
 * Return: QDF_STATUS
 */
typedef QDF_STATUS
	(*osif_cm_disconnect_comp_cb)(struct wlan_objmgr_vdev *vdev,
				      struct wlan_cm_discon_rsp *rsp,
				      enum osif_cb_type type);

#ifdef CONN_MGR_ADV_FEATURE
/**
 * typedef osif_cm_netif_queue_ctrl_cb: Callback to update netif queue
 * @vdev: vdev pointer
 * @action: Action to take on netif queue
 * @reason: netif reason type
 *
 * This callback indicates legacy modules to take the actions related to netif
 * queue
 *
 * Context: Any context.
 * Return: QDF_STATUS
 */
typedef QDF_STATUS
	(*osif_cm_netif_queue_ctrl_cb)(struct wlan_objmgr_vdev *vdev,
				       enum netif_action_type action,
				       enum netif_reason_type reason);
#endif

/**
 * osif_cm_ops: connection manager legacy callbacks
 * osif_cm_connect_comp_cb: callback for connect complete to legacy
 * modules
 *  osif_cm_disconnect_comp_cb: callback for disconnect complete to
 * legacy modules
 * osif_cm_netif_queue_ctrl_cb: callback to legacy module to take
 * actions on netif queue
 */
struct osif_cm_ops {
	osif_cm_connect_comp_cb connect_complete_cb;
	osif_cm_disconnect_comp_cb disconnect_complete_cb;
#ifdef CONN_MGR_ADV_FEATURE
	osif_cm_netif_queue_ctrl_cb netif_queue_control_cb;
#endif
};

/**
 * osif_cm_connect_comp_ind() - Function to indicate connect
 * complete to legacy module
 * @vdev: vdev pointer
 * @rsp: connect response
 * @type: indicates update type
 *
 * This function indicates connect complete to the legacy module
 *
 * Context: Any context.
 * Return: QDF_STATUS
 */
QDF_STATUS osif_cm_connect_comp_ind(struct wlan_objmgr_vdev *vdev,
				    struct wlan_cm_connect_rsp *rsp,
				    enum osif_cb_type type);

/**
 * osif_cm_disconnect_comp_ind() - Function to indicate disconnect
 * complete to legacy module
 * @vdev: vdev pointer
 * @rsp: disconnect response
 * @type: indicates update type
 *
 * This function indicates disconnect complete to the legacy module
 *
 * Context: Any context.
 * Return: QDF_STATUS
 */
QDF_STATUS osif_cm_disconnect_comp_ind(struct wlan_objmgr_vdev *vdev,
				       struct wlan_cm_discon_rsp *rsp,
				       enum osif_cb_type type);

#ifdef CONN_MGR_ADV_FEATURE
/**
 * osif_cm_netif_queue_ind() - Function to indicate netif queue update
 * complete to legacy module
 * @vdev: vdev pointer
 * @action: Action to take on netif queue
 * @reason: netif reason type
 *
 * This function indicates to take the actions related to netif queue
 *
 * Context: Any context.
 * Return: QDF_STATUS
 */
QDF_STATUS osif_cm_netif_queue_ind(struct wlan_objmgr_vdev *vdev,
				   enum netif_action_type action,
				   enum netif_reason_type reason);
#endif
/**
 * osif_cm_set_legacy_cb() - Sets legacy callbacks to osif
 * @osif_legacy_ops:  Function pointer to legacy ops structure
 *
 * API to set legacy callbacks to osif
 * Context: Any context.
 *
 * Return: void
 */
void osif_cm_set_legacy_cb(struct osif_cm_ops *osif_legacy_ops);

/**
 * osif_cm_reset_legacy_cb() - Resets legacy callbacks to osif
 *
 * API to reset legacy callbacks to osif
 * Context: Any context.
 *
 * Return: void
 */
void osif_cm_reset_legacy_cb(void);

#else
static inline QDF_STATUS osif_cm_osif_priv_init(struct wlan_objmgr_vdev *vdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS osif_cm_osif_priv_deinit(struct wlan_objmgr_vdev *vdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS osif_cm_register_cb(void)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#endif /* __OSIF_CM_UTIL_H */
