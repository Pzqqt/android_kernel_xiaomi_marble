/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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

/**
 * DOC: offload lmac interface APIs definitions for FTM
 */

#include <qdf_status.h>
#include <target_if_ftm.h>
#include <wmi_unified_priv.h>
#include <wlan_objmgr_psoc_obj.h>
#include <target_if.h>
#include <wlan_lmac_if_def.h>
#include <wlan_ftm_ucfg_api.h>

static inline struct wlan_lmac_if_ftm_rx_ops *
target_if_ftm_get_rx_ops(struct wlan_objmgr_psoc *psoc)
{
	return &psoc->soc_cb.rx_ops.ftm_rx_ops;
}

static int
target_if_ftm_process_utf_event(ol_scn_t sc, uint8_t *event_buf, uint32_t len)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev;
	struct wmi_host_pdev_utf_event event;
	struct wlan_lmac_if_ftm_rx_ops *ftm_rx_ops;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	psoc = target_if_get_psoc_from_scn_hdl(sc);
	if (!psoc) {
		ftm_err("null psoc");
		return QDF_STATUS_E_INVAL;
	}

	status = wlan_objmgr_psoc_try_get_ref(psoc, WLAN_FTM_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		ftm_err("unable to get psoc reference");
		return QDF_STATUS_E_INVAL;
	}

	event.datalen = len;
	if (wmi_extract_pdev_utf_event(GET_WMI_HDL_FROM_PSOC(psoc),
				event_buf, &event) != QDF_STATUS_SUCCESS) {
		ftm_err("Extracting utf event failed");
		wlan_objmgr_psoc_release_ref(psoc, WLAN_FTM_ID);
		return QDF_STATUS_E_INVAL;
	}

	pdev = wlan_objmgr_get_pdev_by_id(psoc, event.pdev_id, WLAN_FTM_ID);
	if (!pdev) {
		ftm_err("null pdev");
		wlan_objmgr_psoc_release_ref(psoc, WLAN_FTM_ID);
		return QDF_STATUS_E_INVAL;
	}

	ftm_rx_ops = target_if_ftm_get_rx_ops(psoc);

	if (ftm_rx_ops->ftm_ev_handler) {
		status = ftm_rx_ops->ftm_ev_handler(pdev,
				event.data, event.datalen);
		if (QDF_IS_STATUS_ERROR(status))
			status = QDF_STATUS_E_INVAL;
	} else {
		status = QDF_STATUS_E_INVAL;
	}

	wlan_objmgr_pdev_release_ref(pdev, WLAN_FTM_ID);
	wlan_objmgr_psoc_release_ref(psoc, WLAN_FTM_ID);

	return status;
}

QDF_STATUS target_if_ftm_cmd_send(struct wlan_objmgr_pdev *pdev,
				  uint8_t *buf, uint32_t len,
				  uint8_t pdev_id)
{
	QDF_STATUS ret;
	wmi_unified_t handle;
	struct pdev_utf_params param;

	if (!pdev) {
		target_if_err("null pdev");
		return QDF_STATUS_E_FAILURE;
	}

	handle = GET_WMI_HDL_FROM_PDEV(pdev);
	if (!handle) {
		target_if_err("null handle");
		return QDF_STATUS_E_FAILURE;
	}
	param.utf_payload = buf;
	param.len = len;

	ret = wmi_unified_pdev_utf_cmd_send(handle, &param, pdev_id);
	if (QDF_IS_STATUS_ERROR(ret))
		ftm_err("wmi utf cmd send failed, ret: %d", ret);

	return ret;
}

QDF_STATUS target_if_ftm_attach(struct wlan_objmgr_psoc *psoc)
{
	int ret;
	wmi_unified_t handle;

	if (!psoc) {
		target_if_err("null psoc");
		return QDF_STATUS_E_FAILURE;
	}

	handle = GET_WMI_HDL_FROM_PSOC(psoc);
	if (!handle) {
		target_if_err("null handle");
		return QDF_STATUS_E_FAILURE;
	}
	ret = wmi_unified_register_event_handler(handle,
			wmi_pdev_utf_event_id,
			target_if_ftm_process_utf_event,
			WMI_RX_UMAC_CTX);
	if (ret) {
		ftm_err("wmi event registration failed, ret: %d", ret);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_ftm_detach(struct wlan_objmgr_psoc *psoc)

{
	int ret;
	wmi_unified_t handle;

	if (!psoc) {
		target_if_err("null psoc");
		return QDF_STATUS_E_FAILURE;
	}

	handle = GET_WMI_HDL_FROM_PSOC(psoc);
	if (!handle) {
		target_if_err("null handle");
		return QDF_STATUS_E_FAILURE;
	}
	ret = wmi_unified_unregister_event_handler(handle,
						   wmi_pdev_utf_event_id);

	if (ret) {
		ftm_err("wmi event deregistration failed, ret: %d", ret);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_ftm_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	struct wlan_lmac_if_ftm_tx_ops *ftm_tx_ops;

	if (!tx_ops) {
		ftm_err("invalid tx_ops");
		return QDF_STATUS_E_FAILURE;
	}

	ftm_tx_ops = &tx_ops->ftm_tx_ops;
	ftm_tx_ops->ftm_attach = target_if_ftm_attach;
	ftm_tx_ops->ftm_detach = target_if_ftm_detach;
	ftm_tx_ops->ftm_cmd_send = target_if_ftm_cmd_send;

	return QDF_STATUS_SUCCESS;
}
