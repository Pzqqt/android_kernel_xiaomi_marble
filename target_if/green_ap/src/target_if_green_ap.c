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
	green_ap_tx_ops->reset_dev = NULL;
	green_ap_tx_ops->get_current_channel = NULL;
	green_ap_tx_ops->get_current_channel_flags = NULL;
	green_ap_tx_ops->get_capab = NULL;

	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_green_ap_egap_status_info_event() - egap status info event
 * @scn: pointer to scn handle
 * @evt_buf: pointer to event buffer
 * @data_len: data len of the event buffer
 *
 * Return: 0 for success, otherwise appropriate error code
 */
static int target_if_green_ap_egap_status_info_event(
		ol_scn_t scn, uint8_t *evt_buf, uint32_t data_len)
{
	struct wlan_objmgr_pdev *pdev;
	struct wlan_green_ap_egap_status_info egap_status_info_params;
	void *wmi_hdl;

	pdev = target_if_get_pdev_from_scn_hdl(scn);
	if (!pdev) {
		green_ap_err("pdev is null");
		return QDF_STATUS_E_FAILURE;
	}

	wmi_hdl = GET_WMI_HDL_FROM_PDEV(pdev);
	if (!wmi_hdl) {
		green_ap_err("null wmi_hdl");
		return QDF_STATUS_E_FAILURE;
	}

	if (wmi_extract_green_ap_egap_status_info(wmi_hdl,
						  evt_buf,
						  &egap_status_info_params) !=
						  QDF_STATUS_SUCCESS) {
		green_ap_err("unable to extract green ap egap status info");
		return QDF_STATUS_E_FAILURE;
	}

	green_ap_debug("mac_id: %d, status: %d, tx_mask: %x, rx_mask: %d",
		       egap_status_info_params.mac_id,
		       egap_status_info_params.status,
		       egap_status_info_params.tx_chainmask,
		       egap_status_info_params.rx_chainmask);

	return 0;
}

QDF_STATUS target_if_green_ap_register_egap_event_handler(
			struct wlan_objmgr_pdev *pdev)
{
	struct wlan_pdev_green_ap_ctx *green_ap_ctx;
	struct wlan_green_ap_egap_params *egap_params;
	int ret;
	void *wmi_hdl;

	if (!pdev) {
		green_ap_err("pdev is null");
		return QDF_STATUS_E_INVAL;
	}

	wmi_hdl = GET_WMI_HDL_FROM_PDEV(pdev);
	if (!wmi_hdl) {
		green_ap_err("null wmi_hdl");
		return QDF_STATUS_E_FAILURE;
	}

	green_ap_ctx = wlan_objmgr_pdev_get_comp_private_obj(
			pdev, WLAN_UMAC_COMP_GREEN_AP);
	if (!green_ap_ctx) {
		green_ap_err("green ap context obtained is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	egap_params = &green_ap_ctx->egap_params;

	ret = wmi_unified_register_event_handler(
			wmi_hdl,
			wmi_ap_ps_egap_info_event_id,
			target_if_green_ap_egap_status_info_event,
			WMI_RX_UMAC_CTX);
	if (ret < 0) {
		green_ap_err("Failed to register Enhance Green AP event");
		egap_params->fw_egap_support = false;
	} else {
		green_ap_info("Set the Enhance Green AP event handler");
		egap_params->fw_egap_support = true;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_green_ap_enable_egap(
		struct wlan_objmgr_pdev *pdev,
		struct wlan_green_ap_egap_params *egap_params)
{
	struct wlan_pdev_green_ap_ctx *green_ap_ctx;
	wmi_unified_t wmi_hdl;

	if (!pdev) {
		green_ap_err("pdev context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	wmi_hdl = GET_WMI_HDL_FROM_PDEV(pdev);
	if (!wmi_hdl) {
		green_ap_err("null wmi_hdl");
		return QDF_STATUS_E_FAILURE;
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

	return wmi_unified_egap_conf_params_cmd(wmi_hdl,
							egap_params);
}

QDF_STATUS target_if_green_ap_set_ps_on_off(struct wlan_objmgr_pdev *pdev,
					    bool value, uint8_t pdev_id)
{
	wmi_unified_t wmi_hdl;

	if (!pdev) {
		green_ap_err("pdev context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	wmi_hdl = GET_WMI_HDL_FROM_PDEV(pdev);
	if (!wmi_hdl) {
		green_ap_err("null wmi_hdl");
		return QDF_STATUS_E_FAILURE;
	}

	return wmi_unified_green_ap_ps_send(wmi_hdl,
					    value, pdev_id);
}
