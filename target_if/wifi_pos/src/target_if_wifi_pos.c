/*
 * Copyright (c) 2013-2017 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/**
 * DOC: target_if_wifi_pos.c
 * This file defines the functions pertinent to wifi positioning component's
 * target if layer.
 */
#include "../../../../umac/wifi_pos/src/wifi_pos_utils_i.h"
#include "wmi_unified_api.h"
#include "wlan_lmac_if_def.h"
#include "target_if_wifi_pos.h"
#include "../../../../umac/wifi_pos/src/wifi_pos_main_i.h"
#include "target_if.h"

/**
 * wifi_pos_oem_rsp_ev_handler: handler registered with WMI_OEM_RESPONSE_EVENTID
 * @scn: scn handle
 * @data_buf: event buffer
 * @data_len: event buffer length
 *
 * Return: status of operation
 */
static int wifi_pos_oem_rsp_ev_handler(ol_scn_t scn,
					uint8_t *data_buf,
					uint32_t data_len)
{
	int ret;
	struct oem_data_rsp oem_rsp = {0};
	struct wifi_pos_psoc_priv_obj *wifi_pos_psoc;
	struct wlan_objmgr_psoc *psoc = wifi_pos_get_psoc();
	struct wlan_lmac_if_wifi_pos_rx_ops *wifi_pos_rx_ops = NULL;
	WMI_OEM_RESPONSE_EVENTID_param_tlvs *param_buf =
		(WMI_OEM_RESPONSE_EVENTID_param_tlvs *)data_buf;

	if (!psoc) {
		wifi_pos_err("psoc is null");
		return QDF_STATUS_NOT_INITIALIZED;
	}
	wifi_pos_psoc = wifi_pos_get_psoc_priv_obj(psoc);
	if (!wifi_pos_psoc) {
		wifi_pos_err("wifi_pos_psoc is null");
		return QDF_STATUS_NOT_INITIALIZED;
	}
	qdf_spin_lock_bh(&wifi_pos_psoc->wifi_pos_lock);
	wlan_objmgr_psoc_get_ref(psoc, WLAN_WIFI_POS_ID);

	wifi_pos_rx_ops = target_if_wifi_pos_get_rxops(psoc);
	/* this will be implemented later */
	if (!wifi_pos_rx_ops || !wifi_pos_rx_ops->oem_rsp_event_rx) {
		wifi_pos_err("lmac callbacks not registered");
		ret = QDF_STATUS_NOT_INITIALIZED;
		goto release_psoc_ref;
	}

	oem_rsp.rsp_len = param_buf->num_data;
	oem_rsp.data = param_buf->data;

	ret = wifi_pos_rx_ops->oem_rsp_event_rx(psoc, &oem_rsp);

release_psoc_ref:
	wlan_objmgr_psoc_release_ref(psoc, WLAN_WIFI_POS_ID);
	qdf_spin_unlock_bh(&wifi_pos_psoc->wifi_pos_lock);

	return ret;
}

/**
 * wifi_pos_oem_cap_ev_handler: handler registered with wmi_oem_cap_event_id
 * @scn: scn handle
 * @buf: event buffer
 * @len: event buffer length
 *
 * Return: status of operation
 */
static int wifi_pos_oem_cap_ev_handler(ol_scn_t scn, uint8_t *buf, uint32_t len)
{
	/* TBD */
	return 0;
}

/**
 * wifi_pos_oem_meas_rpt_ev_handler: handler registered with
 * wmi_oem_meas_report_event_id
 * @scn: scn handle
 * @buf: event buffer
 * @len: event buffer length
 *
 * Return: status of operation
 */
static int wifi_pos_oem_meas_rpt_ev_handler(ol_scn_t scn, uint8_t *buf,
					    uint32_t len)
{
	/* TBD */
	return 0;
}

/**
 * wifi_pos_oem_err_rpt_ev_handler: handler registered with
 * wmi_oem_err_report_event_id
 * @scn: scn handle
 * @buf: event buffer
 * @len: event buffer length
 *
 * Return: status of operation
 */
static int wifi_pos_oem_err_rpt_ev_handler(ol_scn_t scn, uint8_t *buf,
					    uint32_t len)
{
	/* TBD */
	return 0;
}

/**
 * wifi_pos_oem_data_req() - start OEM data request to target
 * @wma_handle: wma handle
 * @req: start request params
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS wifi_pos_oem_data_req(struct wlan_objmgr_psoc *psoc,
					struct oem_data_req *req)
{
	QDF_STATUS status;
	void *wmi_hdl = GET_WMI_HDL_FROM_PSOC(psoc);

	wifi_pos_debug("Send oem data req to target");

	if (!req || !req->data) {
		wifi_pos_err("oem_data_req is null");
		return QDF_STATUS_E_INVAL;
	}

	if (!wmi_hdl) {
		wifi_pos_err(FL("WMA closed, can't send oem data req cmd"));
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_start_oem_data_cmd(wmi_hdl, req->data_len,
						req->data);

	if (!QDF_IS_STATUS_SUCCESS(status))
		wifi_pos_err("wmi cmd send failed");

	return status;
}

void target_if_wifi_pos_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	struct wlan_lmac_if_wifi_pos_tx_ops *wifi_pos_tx_ops;
	wifi_pos_tx_ops = &tx_ops->wifi_pos_tx_ops;
	wifi_pos_tx_ops->data_req_tx = wifi_pos_oem_data_req;
}

void target_if_wifi_pos_register_rx_ops(struct wlan_lmac_if_rx_ops *rx_ops)
{
	struct wlan_lmac_if_wifi_pos_rx_ops *wifi_pos_rx_ops;
	wifi_pos_rx_ops = &rx_ops->wifi_pos_rx_ops;
	wifi_pos_rx_ops->oem_rsp_event_rx = wifi_pos_oem_rsp_handler;
}

inline struct wlan_lmac_if_wifi_pos_tx_ops *target_if_wifi_pos_get_txops(
						struct wlan_objmgr_psoc *psoc)
{
	if (!psoc) {
		wifi_pos_err("passed psoc is NULL");
		return NULL;
	}

	return &psoc->soc_cb.tx_ops.wifi_pos_tx_ops;
}

inline struct wlan_lmac_if_wifi_pos_rx_ops *target_if_wifi_pos_get_rxops(
						struct wlan_objmgr_psoc *psoc)
{
	if (!psoc) {
		wifi_pos_err("passed psoc is NULL");
		return NULL;
	}

	return &psoc->soc_cb.rx_ops.wifi_pos_rx_ops;
}

QDF_STATUS target_if_wifi_pos_register_events(struct wlan_objmgr_psoc *psoc)
{
	int ret;

	if (!psoc || !psoc->tgt_if_handle) {
		wifi_pos_err("psoc or psoc->tgt_if_handle is null");
		return QDF_STATUS_E_INVAL;
	}

	ret = wmi_unified_register_event_handler(psoc->tgt_if_handle,
					WMI_OEM_RESPONSE_EVENTID,
					wifi_pos_oem_rsp_ev_handler,
					WMI_RX_UMAC_CTX);
	if (ret) {
		wifi_pos_err("register_event_handler failed: err %d", ret);
		return QDF_STATUS_E_INVAL;
	}

	ret = wmi_unified_register_event_handler(psoc->tgt_if_handle,
					wmi_oem_cap_event_id,
					wifi_pos_oem_cap_ev_handler,
					WMI_RX_UMAC_CTX);
	if (ret) {
		wifi_pos_err("register_event_handler failed: err %d", ret);
		return QDF_STATUS_E_INVAL;
	}

	ret = wmi_unified_register_event_handler(psoc->tgt_if_handle,
					wmi_oem_meas_report_event_id,
					wifi_pos_oem_meas_rpt_ev_handler,
					WMI_RX_UMAC_CTX);
	if (ret) {
		wifi_pos_err("register_event_handler failed: err %d", ret);
		return QDF_STATUS_E_INVAL;
	}

	ret = wmi_unified_register_event_handler(psoc->tgt_if_handle,
					wmi_oem_report_event_id,
					wifi_pos_oem_err_rpt_ev_handler,
					WMI_RX_UMAC_CTX);
	if (ret) {
		wifi_pos_err("register_event_handler failed: err %d", ret);
		return QDF_STATUS_E_INVAL;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_wifi_pos_deregister_events(struct wlan_objmgr_psoc *psoc)
{
	if (!psoc || !psoc->tgt_if_handle) {
		wifi_pos_err("psoc or psoc->tgt_if_handle is null");
		return QDF_STATUS_E_INVAL;
	}

	wmi_unified_unregister_event_handler(psoc->tgt_if_handle,
					WMI_OEM_RESPONSE_EVENTID);
	wmi_unified_unregister_event_handler(psoc->tgt_if_handle,
					wmi_oem_cap_event_id);
	wmi_unified_unregister_event_handler(psoc->tgt_if_handle,
					wmi_oem_meas_report_event_id);
	wmi_unified_unregister_event_handler(psoc->tgt_if_handle,
					wmi_oem_report_event_id);

	return QDF_STATUS_SUCCESS;
}
