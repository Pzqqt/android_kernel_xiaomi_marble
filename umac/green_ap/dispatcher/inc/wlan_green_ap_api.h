/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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
 * DOC: Contains green ap north bound interface definitions
 */

#ifndef _WLAN_GREEN_AP_API_H_
#define _WLAN_GREEN_AP_API_H_

#include <wlan_objmgr_cmn.h>
#include <wlan_objmgr_pdev_obj.h>
#include <qdf_status.h>

/* Green ap mode of operation */
#define WLAN_GREEN_AP_MODE_NO_STA       1 /* PS if no sta connected */
#define WLAN_GREEN_AP_MODE_NUM_STREAM   2 /* PS if 1x1 clients only connected */

/**
 * struct wlan_green_ap_egap_params - enhance green ap params
 * @fw_egap_support: fw enhance green ap support
 * @host_enable_egap: HOST enhance green ap support
 * @egap_inactivity_time: inactivity time
 * @egap_wait_time: wait time
 * @egap_feature_flags: feature flags
 */
struct wlan_green_ap_egap_params {
	bool fw_egap_support;
	bool host_enable_egap;
	uint32_t egap_inactivity_time;
	uint32_t egap_wait_time;
	uint32_t egap_feature_flags;
};

/**
 * struct wlan_green_ap_egap_status_info - enhance green ap params
 * @status: egap status
 * @mac_id: mac id
 * @tx_chainmask: tx chainmask
 * @rx_chainmask: rx chainmask
 */
struct wlan_green_ap_egap_status_info {
	uint32_t status;
	uint32_t mac_id;
	uint32_t tx_chainmask;
	uint32_t rx_chainmask;
};

/**
 * wlan_green_ap_init() - initialize green ap component
 *
 * Return: Success or Failure
 */
QDF_STATUS wlan_green_ap_init(void);

/**
 * wlan_green_ap_deinit() - De-initialize green ap component
 *
 * Return: Success or Failure
 */
QDF_STATUS wlan_green_ap_deinit(void);

/**
 * wlan_green_ap_start() - Start green ap
 * @pdev: pdev pointer
 *
 * Call this function when the first SAP comes up
 *
 * Return: Success or Failure
 */
QDF_STATUS wlan_green_ap_start(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_green_ap_stop() - Stop green ap
 * @pdev: pdev pointer
 *
 * Call this function when the last SAP goes down
 *
 * Return: Success or Failure
 */
QDF_STATUS wlan_green_ap_stop(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_green_ap_pdev_open() - open component and update config params
 * @pdev: pdev pointer
 *
 * Return: Success or Failure
 */
QDF_STATUS wlan_green_ap_pdev_open(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_green_ap_add_sta() - On association
 * @pdev: pdev pointer
 *
 * Call this function when new node is associated
 *
 * Return: Success or Failure
 */
QDF_STATUS wlan_green_ap_add_sta(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_green_ap_add_multistream_sta() - On association
 * @pdev: pdev pointer
 *
 * Call this function when new multistream node is associated
 *
 * Return: Success or Failure
 */
QDF_STATUS wlan_green_ap_add_multistream_sta(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_green_ap_del_sta() - On disassociation
 * @pdev: pdev pointer
 *
 * Call this function when new node is disassociated
 *
 * Return: Success or Failure
 */
QDF_STATUS wlan_green_ap_del_sta(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_green_ap_del_multistream_sta() - On disassociation
 * @pdev: pdev pointer
 *
 * Call this function when new multistream node is disassociated
 *
 * Return: Success or Failure
 */
QDF_STATUS wlan_green_ap_del_multistream_sta(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_green_ap_is_ps_enabled() - is power save enabled
 * @pdev: pdev pointer
 *
 * Check if power save is enabled in FW
 *
 * Return: Success or Failure
 */
bool wlan_green_ap_is_ps_enabled(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_green_ap_suspend_handle() - handle driver suspend
 * @pdev: pdev pointer
 *
 * Return: None
 */
void wlan_green_ap_suspend_handle(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_green_ap_get_capab() - get lmac capability
 * @pdev: pdev pointer
 *
 * Return: Appropriate status
 */
QDF_STATUS wlan_green_ap_get_capab(struct wlan_objmgr_pdev *pdev);
#endif /* _WLAN_GREEN_AP_API_H_ */
