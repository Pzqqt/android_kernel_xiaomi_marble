/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
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
 * DOC: target_if_dcs.c
 *
 * This file provide definition for APIs registered through lmac Tx Ops
 */

#include <wmi_unified_api.h>
#include <wmi_unified_priv.h>
#include <wmi_unified_dcs_api.h>
#include <init_deinit_lmac.h>
#include "wlan_dcs_tgt_api.h"
#include "target_if_dcs.h"

/**
 * target_if_dcs_interference_event_handler() - function to handle dcs event
 * from firmware.
 * @scn: scn handle
 * @data: data buffer for event
 * @datalen: data length
 *
 * Return: status of operation.
 */
static int target_if_dcs_interference_event_handler(ol_scn_t scn,
						    uint8_t *data,
						    uint32_t datalen)
{
	QDF_STATUS status;
	struct wlan_host_dcs_event ev;
	struct wlan_objmgr_psoc *psoc;
	struct wmi_unified *wmi_handle;
	struct wlan_target_if_dcs_rx_ops *rx_ops;

	if (!scn || !data) {
		target_if_err("scn: 0x%pK, data: 0x%pK", scn, data);
		return -EINVAL;
	}
	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("null psoc");
		return -EINVAL;
	}

	rx_ops = target_if_dcs_get_rx_ops(psoc);
	if (!rx_ops || !rx_ops->process_dcs_event) {
		target_if_err("callback not registered");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null");
		return -EINVAL;
	}

	if (wmi_extract_dcs_interference_type(wmi_handle, data,
					      &ev.dcs_param) !=
	    QDF_STATUS_SUCCESS) {
		target_if_err("Unable to extract dcs interference type");
		return -EINVAL;
	}

	if (ev.dcs_param.interference_type == WLAN_HOST_DCS_WLANIM &&
	    wmi_extract_dcs_im_tgt_stats(wmi_handle, data, &ev.wlan_stat) !=
	    QDF_STATUS_SUCCESS) {
		target_if_err("Unable to extract WLAN IM stats");
		return -EINVAL;
	}

	if (ev.dcs_param.interference_type == WLAN_HOST_DCS_AWGNIM &&
	    wmi_extract_dcs_awgn_info(wmi_handle, data, &ev.awgn_info) !=
	    QDF_STATUS_SUCCESS) {
		target_if_err("Unable to extract AWGN info");
		return -EINVAL;
	}

	status = rx_ops->process_dcs_event(psoc, &ev);

	return qdf_status_to_os_return(status);
}

static QDF_STATUS
target_if_dcs_register_event_handler(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS ret_val;
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

	ret_val = wmi_unified_register_event_handler(
			wmi_handle,
			wmi_dcs_interference_event_id,
			target_if_dcs_interference_event_handler,
			WMI_RX_WORK_CTX);
	if (QDF_IS_STATUS_ERROR(ret_val))
		target_if_err("Failed to register dcs interference event cb");

	return ret_val;
}

static QDF_STATUS
target_if_dcs_unregister_event_handler(struct wlan_objmgr_psoc *psoc)
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
					     wmi_dcs_interference_event_id);

	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_dcs_cmd_send() - Send WMI command for dcs requests
 * @psoc: psoc pointer
 * @pdev_id: pdev_id
 * @is_host_pdev_id: pdev_id is host pdev_id or not
 * @dcs_enable: dcs enable or not
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
static QDF_STATUS
target_if_dcs_cmd_send(struct wlan_objmgr_psoc *psoc, uint32_t pdev_id,
		       bool is_host_pdev_id, uint32_t dcs_enable)
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

	ret = wmi_send_dcs_pdev_param(wmi_handle, pdev_id,
				      is_host_pdev_id, dcs_enable);
	if (QDF_IS_STATUS_ERROR(ret))
		target_if_err("wmi dcs cmd send failed, ret: %d", ret);

	return ret;
}

QDF_STATUS
target_if_dcs_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	struct wlan_target_if_dcs_tx_ops *dcs_tx_ops;

	if (!tx_ops) {
		target_if_err("lmac tx ops is NULL!");
		return QDF_STATUS_E_INVAL;
	}

	dcs_tx_ops = &tx_ops->dcs_tx_ops;
	if (!dcs_tx_ops) {
		target_if_err("lmac tx ops is NULL!");
		return QDF_STATUS_E_FAILURE;
	}

	dcs_tx_ops->dcs_attach =
		target_if_dcs_register_event_handler;
	dcs_tx_ops->dcs_detach =
		target_if_dcs_unregister_event_handler;
	dcs_tx_ops->dcs_cmd_send = target_if_dcs_cmd_send;

	return QDF_STATUS_SUCCESS;
}

