/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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

#ifndef _WLAN_IOT_SIM_UTILS_API_H_
#define _WLAN_IOT_SIM_UTILS_API_H_

#include <wlan_objmgr_cmn.h>
#include <wlan_lmac_if_def.h>
#include <wmi_unified_param.h>
#include "include/wlan_pdev_mlme.h"
#include "wlan_pdev_mlme_api.h"

/* Forward Declarations */
struct wmi_iot_sim_cmd_ops;

/**
 * struct iot_sim_cbacks - IOT Sim callbacks
 * @reg_beacon_trigger_handler: reg_beacon_trigger_handler
 */
struct iot_sim_cbacks {
	void (*update_beacon_trigger)(mlme_pdev_ext_t *);
};

/**
 * iot_sim_cmd_handler() - IOT SIM frame handler function
 * @vdev - vdev object.
 * @buf - skb
 * @tx - TRUE in case of Tx
 * @rx_param - mgmt_rx_event_params
 *
 * Return : QDF_STATUS_E_SUCCESS/QDF_STATUS_E_FAILURE.
 */
QDF_STATUS iot_sim_cmd_handler(struct wlan_objmgr_vdev *vdev, qdf_nbuf_t buf,
			       struct beacon_tmpl_params *bcn_param, bool tx,
			       struct mgmt_rx_event_params *param);

/**
 * wlan_iot_sim_init() - API to init iot_sim component
 *
 * This API is invoked from dispatcher init during all component init.
 * This API will register all required handlers for pdev and peer object
 * create/delete notification.
 *
 * Return: SUCCESS,
 *         Failure
 */
QDF_STATUS wlan_iot_sim_init(void);

/*
 * wlan_iot_sim_deinit() - API to deinit iot_sim component
 *
 * This API is invoked from dispatcher deinit during all component deinit.
 * This API will unregister all registered handlers for pdev and peer object
 * create/delete notification.
 *
 * Return: SUCCESS,
 *         Failure
 */
QDF_STATUS wlan_iot_sim_deinit(void);

/**
 * wlan_lmac_if_iot_sim_register_rx_ops() - Register lmac interface Rx
 * operations
 * @rx_ops: Pointer to lmac interface Rx operations structure
 *
 * API to register iot_sim related lmac interface Rx operations
 *
 * Return: None
 */
void wlan_lmac_if_iot_sim_register_rx_ops(struct wlan_lmac_if_rx_ops *rx_ops);

/**
 * wlan_register_wmi_iot_sim_cmd_ops() - Register operations related to wmi
 * commands on iot_sim parameters
 * @pdev    - the physical device object
 * @cmd_ops - pointer to the structure holding the operations
 *	     related to wmi commands on iot_sim parameters
 *
 * API to register operations related to wmi commands on iot_sim parameters
 *
 * Return: None
 */
void
wlan_register_wmi_iot_sim_cmd_ops(struct wlan_objmgr_pdev *pdev,
				  struct wmi_iot_sim_cmd_ops *cmd_ops);

#endif /* _WLAN_IOT_SIM_UTILS_API_H_*/
