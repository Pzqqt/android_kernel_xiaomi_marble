/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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
 * DOC: offload lmac interface APIs definitions for Green ap
 */

#include <target_if_green_ap.h>
#include <wlan_green_ap_api.h>
#include <../../core/src/wlan_green_ap_main_i.h>
#include <target_if.h>
#include <wmi_unified_api.h>

QDF_STATUS target_if_register_green_ap_tx_ops(
		struct wlan_lmac_if_tx_ops *tx_ops)
{
	struct wlan_lmac_if_green_ap_tx_ops *green_ap_tx_ops;

	if (!tx_ops) {
		target_if_err("invalid tx_ops");
		return QDF_STATUS_E_FAILURE;
	}

	green_ap_tx_ops = &tx_ops->green_ap_tx_ops;

	green_ap_tx_ops->enable_egap = target_if_green_ap_enable_egap;
	green_ap_tx_ops->ps_on_off_send = target_if_green_ap_set_ps_on_off;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_green_ap_enable_egap(
		struct wlan_objmgr_pdev *pdev,
		struct wlan_green_ap_egap_params *egap_params)
{
	struct wlan_pdev_green_ap_ctx *green_ap_ctx;

	if (!pdev) {
		green_ap_err("pdev context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	green_ap_ctx = wlan_objmgr_pdev_get_comp_private_obj(
			pdev, WLAN_UMAC_COMP_GREEN_AP);
	if (!green_ap_ctx) {
		green_ap_err("green ap context obtained is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_spin_lock_bh(&green_ap_ctx->lock);
	if (!wlan_is_egap_enabled(green_ap_ctx)) {
		green_ap_info("enhanced green ap support is not present");
		qdf_spin_unlock_bh(&green_ap_ctx->lock);
		return QDF_STATUS_SUCCESS;
	}
	qdf_spin_unlock_bh(&green_ap_ctx->lock);

	return wmi_unified_egap_conf_params_cmd(GET_WMI_HDL_FROM_PDEV(pdev),
							egap_params);
}

QDF_STATUS target_if_green_ap_set_ps_on_off(struct wlan_objmgr_pdev *pdev,
					    bool value, uint8_t pdev_id)
{
	if (!pdev) {
		green_ap_err("pdev context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	return wmi_unified_green_ap_ps_send(GET_WMI_HDL_FROM_PDEV(pdev),
					    value, pdev_id);
}
