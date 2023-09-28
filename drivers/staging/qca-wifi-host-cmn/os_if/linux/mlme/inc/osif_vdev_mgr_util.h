/*
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: osif_vdev_mgr_util.h
 *
 * This header file maintains declarations of osif APIs corresponding to vdev
 * manager.
 */

#ifndef __OSIF_VDEV_MGR_UTIL_H
#define __OSIF_VDEV_MGR_UTIL_H
/**
 * struct osif_vdev_mgr_ops - VDEV mgr legacy callbacks
 * @osif_vdev_mgr_set_mac_addr_response: Callback to indicate set MAC address
 *                                       response from FW
 */
struct osif_vdev_mgr_ops {
#ifdef WLAN_FEATURE_DYNAMIC_MAC_ADDR_UPDATE
	void (*osif_vdev_mgr_set_mac_addr_response)(uint8_t vdev_id,
						    uint8_t resp_status);
#endif
};

/**
 * osif_vdev_mgr_set_legacy_cb() - Sets legacy callbacks to osif
 * @osif_legacy_ops:  Function pointer to legacy ops structure
 *
 * API to set legacy callbacks to osif
 * Context: Any context.
 *
 * Return: void
 */
void osif_vdev_mgr_set_legacy_cb(struct osif_vdev_mgr_ops *osif_legacy_ops);

/**
 * osif_vdev_mgr_reset_legacy_cb() - Resets legacy callbacks to osif
 *
 * API to reset legacy callbacks to osif
 * Context: Any context.
 *
 * Return: void
 */
void osif_vdev_mgr_reset_legacy_cb(void);

/**
 * osif_vdev_mgr_register_cb() - Register VDEV manager legacy callbacks
 *
 * API to register legavy VDEV manager callbacks
 *
 * Return: QDF_STATUS
 */
QDF_STATUS osif_vdev_mgr_register_cb(void);
#endif /* __OSIF_CM_UTIL_H */
