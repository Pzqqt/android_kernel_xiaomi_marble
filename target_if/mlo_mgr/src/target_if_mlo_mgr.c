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
 * DOC: target_if_mlo_mgr.c
 *
 * This file provide definition for APIs registered through lmac Tx Ops
 */

#include <wmi_unified_11be_api.h>
#include <init_deinit_lmac.h>
#include "target_if_mlo_mgr.h"

/**
 * target_if_mlo_link_set_active_resp_handler() - function to handle mlo link
 *  set active response from firmware.
 * @scn: scn handle
 * @data: data buffer for event
 * @datalen: data length
 *
 * Return: 0 on success, else error on failure
 */
static int
target_if_mlo_link_set_active_resp_handler(ol_scn_t scn, uint8_t *data,
					   uint32_t datalen)
{
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc;
	struct wmi_unified *wmi_handle;
	struct wlan_lmac_if_mlo_rx_ops *rx_ops;
	struct mlo_link_set_active_resp resp;

	if (!scn || !data) {
		target_if_err("scn: 0x%pK, data: 0x%pK", scn, data);
		return -EINVAL;
	}

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("null psoc");
		return -EINVAL;
	}

	rx_ops = target_if_mlo_get_rx_ops(psoc);
	if (!rx_ops || !rx_ops->process_link_set_active_resp) {
		target_if_err("callback not registered");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null");
		return -EINVAL;
	}

	if (wmi_extract_mlo_link_set_active_resp(wmi_handle, data, &resp) !=
	    QDF_STATUS_SUCCESS) {
		target_if_err("Unable to extract mlo link set active resp");
		return -EINVAL;
	}

	status = rx_ops->process_link_set_active_resp(psoc, &resp);

	return qdf_status_to_os_return(status);
}

/**
 * target_if_mlo_register_event_handler() - function to register handler for
 *  mlo related wmi event from firmware.
 * @psoc: psoc pointer
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
target_if_mlo_register_event_handler(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;

	if (!psoc) {
		target_if_err("PSOC is NULL!");
		return QDF_STATUS_E_NULL_VALUE;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_register_event_handler(
			wmi_handle,
			wmi_mlo_link_set_active_resp_eventid,
			target_if_mlo_link_set_active_resp_handler,
			WMI_RX_WORK_CTX);
	if (QDF_IS_STATUS_ERROR(status))
		target_if_err("Failed to register mlo link set active resp cb");

	return status;
}

/**
 * target_if_mlo_unregister_event_handler() - function to unregister handler for
 *  mlo related wmi event from firmware.
 * @psoc: psoc pointer
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
target_if_mlo_unregister_event_handler(struct wlan_objmgr_psoc *psoc)
{
	struct wmi_unified *wmi_handle;

	if (!psoc) {
		target_if_err("PSOC is NULL!");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null");
		return QDF_STATUS_E_INVAL;
	}

	wmi_unified_unregister_event_handler(wmi_handle,
		wmi_mlo_link_set_active_resp_eventid);

	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_mlo_link_set_active() - Send WMI command for set mlo link active
 * @psoc: psoc pointer
 * @param: parameter for setting mlo link active
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
target_if_mlo_link_set_active(struct wlan_objmgr_psoc *psoc,
			      struct mlo_link_set_active_param *param)
{
	QDF_STATUS ret;
	struct wmi_unified *wmi_handle;

	if (!psoc) {
		target_if_err("null psoc");
		return QDF_STATUS_E_FAILURE;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("null handle");
		return QDF_STATUS_E_FAILURE;
	}

	ret = wmi_send_mlo_link_set_active_cmd(wmi_handle, param);
	if (QDF_IS_STATUS_ERROR(ret))
		target_if_err("wmi mlo link set active send failed: %d", ret);

	return ret;
}

/**
 * target_if_mlo_register_tx_ops() - lmac handler to register mlo tx ops
 *  callback functions
 * @tx_ops: wlan_lmac_if_tx_ops object
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
target_if_mlo_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	struct wlan_lmac_if_mlo_tx_ops *mlo_tx_ops;

	if (!tx_ops) {
		target_if_err("lmac tx ops is NULL!");
		return QDF_STATUS_E_INVAL;
	}

	mlo_tx_ops = &tx_ops->mlo_ops;
	if (!mlo_tx_ops) {
		target_if_err("lmac tx ops is NULL!");
		return QDF_STATUS_E_FAILURE;
	}

	mlo_tx_ops->register_events =
		target_if_mlo_register_event_handler;
	mlo_tx_ops->unregister_events =
		target_if_mlo_unregister_event_handler;
	mlo_tx_ops->link_set_active = target_if_mlo_link_set_active;

	return QDF_STATUS_SUCCESS;
}

