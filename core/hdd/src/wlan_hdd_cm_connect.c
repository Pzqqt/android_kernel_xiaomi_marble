/*
 * Copyright (c) 2012-2020, The Linux Foundation. All rights reserved.
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
 * DOC: hdd_cm_connect.c
 *
 * WLAN Host Device Driver connect APIs implementation
 *
 */

#include "wlan_hdd_main.h"
#include "wlan_hdd_cm_api.h"
#include "wlan_hdd_trace.h"
#include "wlan_hdd_object_manager.h"
#include "wlan_hdd_power.h"
#include <osif_cm_req.h>

static void hdd_update_scan_ie_for_connect(struct hdd_adapter *adapter,
					   struct osif_connect_params *params)
{
	if (adapter->device_mode == QDF_P2P_CLIENT_MODE) {
		params->scan_ie.ptr =
			&adapter->scan_info.scan_add_ie.addIEdata[0];
		params->scan_ie.len = adapter->scan_info.scan_add_ie.length;
	} else if (adapter->scan_info.default_scan_ies) {
		params->scan_ie.ptr = adapter->scan_info.default_scan_ies;
		params->scan_ie.len = adapter->scan_info.default_scan_ies_len;
	} else if (adapter->scan_info.scan_add_ie.length) {
		params->scan_ie.ptr = adapter->scan_info.scan_add_ie.addIEdata;
		params->scan_ie.len = adapter->scan_info.scan_add_ie.length;
	}
}

/**
 * hdd_get_dot11mode_filter() - Get dot11 mode filter
 * @hdd_ctx: HDD context
 *
 * This function is used to get the dot11 mode filter
 *
 * Context: Any Context.
 * Return: dot11_mode_filter
 */
static enum dot11_mode_filter
hdd_get_dot11mode_filter(struct hdd_context *hdd_ctx)
{
	struct hdd_config *config = hdd_ctx->config;

	if (config->dot11Mode == eHDD_DOT11_MODE_11n_ONLY)
		return ALLOW_11N_ONLY;
	else if (config->dot11Mode == eHDD_DOT11_MODE_11ac_ONLY)
		return ALLOW_11AC_ONLY;
	else if (config->dot11Mode == eHDD_DOT11_MODE_11ax_ONLY)
		return ALLOW_11AX_ONLY;
	else
		return ALLOW_ALL;
}

int wlan_hdd_cm_connect(struct wiphy *wiphy,
			struct net_device *ndev,
			struct cfg80211_connect_params *req)
{
	int status;
	struct wlan_objmgr_vdev *vdev;
	struct osif_connect_params params;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(ndev);
	struct hdd_context *hdd_ctx;

	hdd_enter();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_vdev_id(adapter->vdev_id))
		return -EINVAL;

	qdf_mtrace(QDF_MODULE_ID_HDD, QDF_MODULE_ID_HDD,
		   TRACE_CODE_HDD_CFG80211_CONNECT,
		   adapter->vdev_id, adapter->device_mode);

	if (adapter->device_mode != QDF_STA_MODE &&
	    adapter->device_mode != QDF_P2P_CLIENT_MODE) {
		hdd_err("Device_mode %s(%d) is not supported",
			qdf_opmode_str(adapter->device_mode),
			adapter->device_mode);
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	status = wlan_hdd_validate_context(hdd_ctx);
	if (status)
		return status;

	vdev = hdd_objmgr_get_vdev(adapter);

	ucfg_pmo_flush_gtk_offload_req(vdev);

	qdf_runtime_pm_prevent_suspend(&hdd_ctx->runtime_context.connect);
	hdd_prevent_suspend_timeout(HDD_WAKELOCK_CONNECT_COMPLETE,
				    WIFI_POWER_EVENT_WAKELOCK_CONNECT);

	params.force_rsne_override = hdd_ctx->force_rsne_override;
	params.dot11mode_filter = hdd_get_dot11mode_filter(hdd_ctx);

	hdd_update_scan_ie_for_connect(adapter, &params);

	status = osif_cm_connect(ndev, vdev, req, &params);

	if (status) {
		hdd_err("Vdev %d connect failed status %d",
			adapter->vdev_id, status);
		qdf_runtime_pm_allow_suspend(&hdd_ctx->runtime_context.connect);
		hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_CONNECT);
	}

	hdd_objmgr_put_vdev(vdev);
	return status;
}

QDF_STATUS hdd_cm_connect_complete(struct wlan_objmgr_vdev *vdev,
				   struct wlan_cm_connect_resp *rsp,
				   enum osif_cb_type type)
{
	return QDF_STATUS_SUCCESS;
}
