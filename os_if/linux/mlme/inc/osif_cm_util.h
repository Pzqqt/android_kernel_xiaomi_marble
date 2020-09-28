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

#endif /* __OSIF_CM_UTIL_H */
