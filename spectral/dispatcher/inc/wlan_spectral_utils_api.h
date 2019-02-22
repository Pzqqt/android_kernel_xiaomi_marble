/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
 *
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

#ifndef _WLAN_SPECTRAL_UTILS_API_H_
#define _WLAN_SPECTRAL_UTILS_API_H_

#include <wlan_objmgr_cmn.h>
#include <wlan_lmac_if_def.h>

/* Forward declaration */
struct direct_buf_rx_data;
struct wmi_spectral_cmd_ops;

/**
 * wlan_spectral_is_feature_disabled() - Check if spectral feature is disabled
 * @psoc - the physical device object.
 *
 * Return : true if spectral is disabled, else false.
 */
bool wlan_spectral_is_feature_disabled(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_spectral_init() - API to init spectral component
 *
 * This API is invoked from dispatcher init during all component init.
 * This API will register all required handlers for pdev and peer object
 * create/delete notification.
 *
 * Return: SUCCESS,
 *         Failure
 */
QDF_STATUS wlan_spectral_init(void);

/**
 * wlan_spectral_deinit() - API to deinit spectral component
 *
 * This API is invoked from dispatcher deinit during all component deinit.
 * This API will unregister all registered handlers for pdev and peer object
 * create/delete notification.
 *
 * Return: SUCCESS,
 *         Failure
 */
QDF_STATUS wlan_spectral_deinit(void);

/**
 * wlan_lmac_if_sptrl_register_rx_ops() - Register lmac interface Rx operations
 * @rx_ops: Pointer to lmac interface Rx operations structure
 *
 * API to register spectral related lmac interface Rx operations
 *
 * Return: None
 */
void
wlan_lmac_if_sptrl_register_rx_ops(struct wlan_lmac_if_rx_ops *rx_ops);

/**
* wlan_register_wmi_spectral_cmd_ops() - Register operations related to wmi
* commands on spectral parameters
* @pdev    - the physical device object
* @cmd_ops - pointer to the structure holding the operations
*	     related to wmi commands on spectral parameters
*
* API to register operations related to wmi commands on spectral parameters
*
* Return: None
*/
void
wlan_register_wmi_spectral_cmd_ops(struct wlan_objmgr_pdev *pdev,
				   struct wmi_spectral_cmd_ops *cmd_ops);

/**
 * struct spectral_legacy_cbacks - Spectral legacy callbacks
 * @vdev_get_chan_freq:          Get channel frequency
 * @vdev_get_ch_width:           Get channel width
 * @vdev_get_sec20chan_freq_mhz: Get seconadry 20 frequency
 */
struct spectral_legacy_cbacks {
	int16_t (*vdev_get_chan_freq)(struct wlan_objmgr_vdev *vdev);
	enum phy_ch_width (*vdev_get_ch_width)(struct wlan_objmgr_vdev *vdev);
	int (*vdev_get_sec20chan_freq_mhz)(struct wlan_objmgr_vdev *vdev,
					   uint16_t *sec20chan_freq);
};

/**
 * spectral_vdev_get_chan_freq - Get vdev channel frequency
 * @vdev:          vdev object
 *
 * Return: vdev operating frequency
 */
int16_t spectral_vdev_get_chan_freq(struct wlan_objmgr_vdev *vdev);

/**
 * spectral_vdev_get_sec20chan_freq_mhz - Get vdev secondary channel frequency
 * @vdev:   vdev object
 * @sec20chan_freq: secondary channel frequency
 *
 * Return: secondary channel freq
 */
int spectral_vdev_get_sec20chan_freq_mhz(struct wlan_objmgr_vdev *vdev,
					uint16_t *sec20chan_freq);

/**
 * spectral_register_legacy_cb() - Register spectral legacy callbacks
 * commands on spectral parameters
 * @psoc    - the physical device object
 * @legacy_cbacks - Reference to struct spectral_legacy_cbacks from which
 * function pointers need to be copied
 *
 * API to register spectral related legacy callbacks
 *
 * Return: QDF_STATUS_SUCCESS upon successful registration,
 *         QDF_STATUS_E_FAILURE upon failure
 */
QDF_STATUS spectral_register_legacy_cb(
	struct wlan_objmgr_psoc *psoc,
	struct spectral_legacy_cbacks *legacy_cbacks);

/**
 * spectral_vdev_get_ch_width() - Get the channel bandwidth
 * @vdev    - Pointer to vdev
 *
 * API to get the channel bandwidth of a given vdev
 *
 * Return: Enumeration corresponding to the channel bandwidth
 */
enum phy_ch_width
spectral_vdev_get_ch_width(struct wlan_objmgr_vdev *vdev);

/**
 * spectral_pdev_open() - Spectral pdev open handler
 * @pdev:  pointer to pdev object
 *
 * API to execute operations on pdev open
 *
 * Return: QDF_STATUS_SUCCESS upon successful registration,
 *         QDF_STATUS_E_FAILURE upon failure
 */
QDF_STATUS spectral_pdev_open(struct wlan_objmgr_pdev *pdev);

/**
 * spectral_register_dbr() - register Spectral event handler with DDMA
 * @pdev:  pointer to pdev object
 *
 * API to register event handler with Direct DMA
 *
 * Return: QDF_STATUS_SUCCESS upon successful registration,
 *         QDF_STATUS_E_FAILURE upon failure
 */

QDF_STATUS spectral_register_dbr(struct wlan_objmgr_pdev *pdev);

/**
 * spectral_unregister_dbr() - unregister Spectral event handler with DDMA
 * @pdev:  pointer to pdev object
 *
 * API to unregister event handler with Direct DMA
 *
 * Return: QDF_STATUS_SUCCESS upon successful unregistration,
 *         QDF_STATUS_E_FAILURE upon failure
 */
QDF_STATUS spectral_unregister_dbr(struct wlan_objmgr_pdev *pdev);

#ifdef DIRECT_BUF_RX_ENABLE
/**
 * spectral_dbr_event_handler() - Spectral dbr event handler
 * @pdev:  pointer to pdev object
 * @payload: dbr event buffer
 *
 * API to handle spectral dbr event
 *
 * Return: true to release buf
 */
bool spectral_dbr_event_handler(struct wlan_objmgr_pdev *pdev,
				struct direct_buf_rx_data *payload);
#endif
#endif /* _WLAN_SPECTRAL_UTILS_API_H_*/
