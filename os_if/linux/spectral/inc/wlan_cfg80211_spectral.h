/*
 * Copyright (c) 2017, 2019-2020 The Linux Foundation. All rights reserved.
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
 * DOC: declares driver functions interfacing with linux kernel
 */

#ifndef _WLAN_CFG80211_SPECTRAL_H_
#define _WLAN_CFG80211_SPECTRAL_H_

#include <linux/version.h>
#include <linux/netdevice.h>
#include <net/cfg80211.h>
#include <qca_vendor.h>
#include <qdf_list.h>
#include <qdf_types.h>
#include <spectral_ioctl.h>
#include <wlan_spectral_public_structs.h>

extern const struct nla_policy
	spectral_scan_policy
	[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_MAX + 1];

#define CONFIG_REQUESTED(type)    ((type == \
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_REQUEST_TYPE_SCAN_AND_CONFIG) || \
	(type == QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_REQUEST_TYPE_CONFIG))

#define SCAN_REQUESTED(type)    ((type == \
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_REQUEST_TYPE_SCAN_AND_CONFIG) || \
	(type == QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_REQUEST_TYPE_SCAN))

/**
 * wlan_cfg80211_register_spectral_cmd_handler() - Registration api for spectral
 * @pdev:    Pointer to pdev
 * @idx:     Index in function table
 * @handler: Pointer to handler
 *
 * Return: 0 on success, negative value on failure
 */
void wlan_cfg80211_register_spectral_cmd_handler(
			struct wlan_objmgr_pdev *pdev,
			struct spectral_cfg80211_vendor_cmd_handlers *handlers);

/**
 * wlan_cfg80211_spectral_scan_config_and_start() - Start spectral scan
 * @wiphy:    Pointer to wiphy
 * @pdev:     Pointer to pdev
 * @vdev:     Pointer to vdev
 * @data:     Reference to data
 * @data_len: Length of @data
 *
 * Return: 0 on success, negative value on failure
 */
int wlan_cfg80211_spectral_scan_config_and_start(struct wiphy *wiphy,
						 struct wlan_objmgr_pdev *pdev,
						 struct wlan_objmgr_vdev *vdev,
						 const void *data,
						 int data_len);

/**
 * wlan_cfg80211_spectral_scan_stop() - Stop spectral scan
 * @wiphy:    Pointer to wiphy
 * @pdev:     Pointer to pdev
 * @vdev:     Pointer to vdev
 * @data:     Reference to data
 * @data_len: Length of @data
 *
 * Return: 0 on success, negative value on failure
 */
int wlan_cfg80211_spectral_scan_stop(struct wiphy *wiphy,
				     struct wlan_objmgr_pdev *pdev,
				     struct wlan_objmgr_vdev *vdev,
				     const void *data,
				     int data_len);

/**
 * wlan_cfg80211_spectral_scan_get_config() - Get spectral scan config
 * @wiphy:    Pointer to wiphy
 * @pdev:     Pointer to pdev
 * @vdev:     Pointer to vdev
 * @data:     Reference to data
 * @data_len: Length of @data
 *
 * Return: 0 on success, negative value on failure
 */
int wlan_cfg80211_spectral_scan_get_config(struct wiphy *wiphy,
					   struct wlan_objmgr_pdev *pdev,
					   struct wlan_objmgr_vdev *vdev,
					   const void *data,
					   int data_len);

/**
 * wlan_cfg80211_spectral_scan_get_cap() - Get spectral system capabilities
 * @wiphy:    Pointer to wiphy
 * @pdev:     Pointer to pdev
 * @vdev:     Pointer to vdev
 * @data:     Reference to data
 * @data_len: Length of @data
 *
 * Return: 0 on success, negative value on failure
 */
int wlan_cfg80211_spectral_scan_get_cap(struct wiphy *wiphy,
					struct wlan_objmgr_pdev *pdev,
					struct wlan_objmgr_vdev *vdev,
					const void *data,
					int data_len);

/**
 * wlan_cfg80211_spectral_scan_get_diag_stats() - Get spectral diag stats
 * @wiphy:    Pointer to wiphy
 * @pdev:     Pointer to pdev
 * @vdev:     Pointer to vdev
 * @data:     Reference to data
 * @data_len: Length of @data
 *
 * Return: 0 on success, negative value on failure
 */
int wlan_cfg80211_spectral_scan_get_diag_stats(struct wiphy *wiphy,
					       struct wlan_objmgr_pdev *pdev,
					       struct wlan_objmgr_vdev *vdev,
					       const void *data,
					       int data_len);

/**
 * wlan_cfg80211_spectral_scan_get_status() - Get spectral scan status
 * @wiphy:    Pointer to wiphy
 * @pdev:     Pointer to pdev
 * @vdev:     Pointer to vdev
 * @data:     Reference to data
 * @data_len: Length of @data
 *
 * Return: 0 on success, negative value on failure
 */
int wlan_cfg80211_spectral_scan_get_status(struct wiphy *wiphy,
					   struct wlan_objmgr_pdev *pdev,
					   struct wlan_objmgr_vdev *vdev,
					   const void *data,
					   int data_len);

/**
 * wlan_cfg80211_spectral_scan_dma_debug_config() - configure DMA debug
 * @pdev:       Pointer to pdev
 * @vdev:       Pointer to vdev
 * @tb:         Pointer to Spectral Scan config attribute
 * @sscan_mode: Spectral scan mode
 *
 * Return QDF_STATUS_SUCCESS on success, QDF_STATUS_E_FAILURE on failure
 */
QDF_STATUS wlan_cfg80211_spectral_scan_dma_debug_config(
		struct wlan_objmgr_pdev *pdev,
		struct wlan_objmgr_vdev *vdev,
		struct nlattr **tb,
		enum spectral_scan_mode sscan_mode);
#endif
