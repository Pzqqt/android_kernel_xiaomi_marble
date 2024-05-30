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

#include <wlan_iot_sim_utils_api.h>
#include <qdf_module.h>
#include "../../core/iot_sim_cmn_api_i.h"
#include <wlan_iot_sim_tgt_api.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>

#define IEEE80211_FRAME_BODY_OFFSET 0x18

QDF_STATUS iot_sim_cmd_handler(struct wlan_objmgr_vdev *vdev, qdf_nbuf_t nbuf,
			       struct beacon_tmpl_params *bcn_param, bool tx,
			       struct mgmt_rx_event_params *param)
{
	struct wlan_objmgr_pdev *pdev = vdev->vdev_objmgr.wlan_pdev;

	return iot_sim_frame_update(pdev, nbuf, bcn_param, tx, param);
}

QDF_STATUS iot_sim_register_callbacks(struct wlan_objmgr_pdev *pdev,
				      struct iot_sim_cbacks *cb)
{
	struct iot_sim_context *isc = NULL;

	isc = wlan_objmgr_pdev_get_comp_private_obj(pdev, WLAN_IOT_SIM_COMP);
	if (!isc)
		return QDF_STATUS_E_NULL_VALUE;

	isc->iot_sim_update_beacon_trigger = cb->update_beacon_trigger;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_iot_sim_init(void)
{
	if (wlan_objmgr_register_pdev_create_handler(
		WLAN_IOT_SIM_COMP,
		wlan_iot_sim_pdev_obj_create_handler,
		NULL) !=
	    QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}
	if (wlan_objmgr_register_pdev_destroy_handler(
		WLAN_IOT_SIM_COMP,
		wlan_iot_sim_pdev_obj_destroy_handler,
		NULL) !=
	    QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_iot_sim_deinit(void)
{
	if (wlan_objmgr_unregister_pdev_create_handler(
		WLAN_IOT_SIM_COMP,
		wlan_iot_sim_pdev_obj_create_handler,
		NULL) !=
	    QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}
	if (wlan_objmgr_unregister_pdev_destroy_handler(
		WLAN_IOT_SIM_COMP,
		wlan_iot_sim_pdev_obj_destroy_handler,
		NULL) !=
	    QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

void wlan_lmac_if_iot_sim_register_rx_ops(struct wlan_lmac_if_rx_ops *rx_ops)
{
	struct wlan_lmac_if_iot_sim_rx_ops *iot_sim_ops =
						&rx_ops->iot_sim_rx_ops;

	iot_sim_ops->iot_sim_cmd_handler = iot_sim_cmd_handler;
	iot_sim_ops->iot_sim_register_cb = iot_sim_register_callbacks;

}
