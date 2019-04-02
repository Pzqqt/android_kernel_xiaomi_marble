/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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
 * DOC: target_if_vdev_mgr_rx_ops.c
 *
 * This file provide definition for APIs registered through events received
 * from FW
 */
#include <target_if_vdev_mgr_rx_ops.h>
#include <target_if_vdev_mgr_tx_ops.h>
#include <wlan_vdev_mgr_tgt_if_rx_defs.h>
#include <wmi_unified_param.h>
#include <wlan_mlme_dbg.h>
#include <target_if.h>
#include <qdf_platform.h>
#include <wlan_vdev_mlme_main.h>

void target_if_vdev_mgr_rsp_timer_mgmt_cb(void *arg)
{
	struct wlan_objmgr_vdev *vdev = arg;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;
	struct vdev_response_timer *vdev_rsp;
	struct crash_inject param;
	struct wmi_unified *wmi_handle;
	struct vdev_start_response start_rsp = {0};
	struct vdev_stop_response stop_rsp = {0};
	struct vdev_delete_response del_rsp = {0};
	uint8_t vdev_id;

	vdev_id = wlan_vdev_get_id(vdev);
	mlme_debug("Response timer expired for VDEV %d", vdev_id);

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		mlme_err("PSOC is NULL");
		return;
	}

	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);
	if (!rx_ops || !rx_ops->vdev_mgr_get_response_timer_info) {
		mlme_err("No Rx Ops");
		return;
	}

	vdev_rsp = rx_ops->vdev_mgr_get_response_timer_info(vdev);
	if (!qdf_atomic_test_bit(START_RESPONSE_BIT, &vdev_rsp->rsp_status) &&
	    !qdf_atomic_test_bit(RESTART_RESPONSE_BIT, &vdev_rsp->rsp_status) &&
	    !qdf_atomic_test_bit(STOP_RESPONSE_BIT, &vdev_rsp->rsp_status) &&
	    !qdf_atomic_test_bit(DELETE_RESPONSE_BIT, &vdev_rsp->rsp_status)) {
		mlme_debug("No response bit is set, ignoring actions");
		return;
	}

	if (target_if_vdev_mgr_is_driver_unloading() || qdf_is_recovering() ||
	    qdf_is_fw_down()) {
		/* this ensures stop timer will not be done in target_if */
		vdev_rsp->timer_status = QDF_STATUS_E_TIMEOUT;
		if (qdf_atomic_test_bit(START_RESPONSE_BIT,
					&vdev_rsp->rsp_status) ||
		    qdf_atomic_test_bit(RESTART_RESPONSE_BIT,
					&vdev_rsp->rsp_status)) {
			start_rsp.vdev_id = wlan_vdev_get_id(vdev);
			start_rsp.status = WMI_HOST_VDEV_START_TIMEOUT;
			if (qdf_atomic_test_bit(START_RESPONSE_BIT,
						&vdev_rsp->rsp_status))
				start_rsp.resp_type =
					WMI_HOST_VDEV_START_RESP_EVENT;
			else
				start_rsp.resp_type =
					WMI_HOST_VDEV_RESTART_RESP_EVENT;

			rx_ops->vdev_mgr_start_response(psoc, &start_rsp);
		}

		if (qdf_atomic_test_bit(STOP_RESPONSE_BIT,
					&vdev_rsp->rsp_status)) {
			stop_rsp.vdev_id = wlan_vdev_get_id(vdev);
			rx_ops->vdev_mgr_stop_response(psoc, &stop_rsp);
		}

		if (qdf_atomic_test_bit(DELETE_RESPONSE_BIT,
					&vdev_rsp->rsp_status)) {
			del_rsp.vdev_id = wlan_vdev_get_id(vdev);
			rx_ops->vdev_mgr_delete_response(psoc, &del_rsp);
		}

		return;
	}

	if (target_if_vdev_mgr_is_panic_on_bug()) {
		QDF_DEBUG_PANIC("PSOC_%d VDEV_%d: Panic on bug, rsp status:%d",
				wlan_psoc_get_id(psoc),
			        vdev_id, vdev_rsp->rsp_status);
	} else {
		mlme_err("PSOC_%d VDEV_%d: Trigger Self recovery, rsp status%d",
			 wlan_psoc_get_id(psoc),
			 vdev_id, vdev_rsp->rsp_status);
		wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);

		qdf_mem_set(&param, sizeof(param), 0);
		/* RECOVERY_SIM_SELF_RECOVERY*/
		param.type = 0x08;
		wmi_crash_inject(wmi_handle, &param);
	}
}

static int target_if_vdev_mgr_start_response_handler(
					ol_scn_t scn,
					uint8_t *data,
					uint32_t datalen)
{
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc;
	struct wmi_unified *wmi_handle;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;
	struct vdev_start_response rsp = {0};
	wmi_host_vdev_start_resp vdev_start_resp;

	if (!scn || !data) {
		mlme_err("scn: 0x%pK, data: 0x%pK", scn, data);
		return -EINVAL;
	}

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		mlme_err("PSOC is NULL");
		return -EINVAL;
	}

	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);
	if (!rx_ops || !rx_ops->vdev_mgr_start_response) {
		mlme_err("No Rx Ops");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		mlme_err("wmi_handle is null");
		return -EINVAL;
	}

	if (wmi_extract_vdev_start_resp(wmi_handle, data, &vdev_start_resp)) {
		mlme_err("WMI extract failed");
		return -EINVAL;
	}

	rsp.vdev_id = vdev_start_resp.vdev_id;
	rsp.requestor_id = vdev_start_resp.requestor_id;
	rsp.status = vdev_start_resp.status;
	rsp.resp_type = vdev_start_resp.resp_type;
	rsp.chain_mask = vdev_start_resp.chain_mask;
	rsp.smps_mode = vdev_start_resp.smps_mode;
	rsp.mac_id = vdev_start_resp.mac_id;
	rsp.cfgd_tx_streams = vdev_start_resp.cfgd_tx_streams;
	rsp.cfgd_rx_streams = vdev_start_resp.cfgd_rx_streams;

	status = rx_ops->vdev_mgr_start_response(psoc, &rsp);

	return qdf_status_to_os_return(status);
}

static int target_if_vdev_mgr_stop_response_handler(
						ol_scn_t scn,
						uint8_t *data,
						uint32_t datalen)
{
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc;
	struct wmi_unified *wmi_handle;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;
	struct vdev_stop_response rsp = {0};
	uint32_t vdev_id;

	if (!scn || !data) {
		mlme_err("scn: 0x%pK, data: 0x%pK", scn, data);
		return -EINVAL;
	}

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		mlme_err("PSOC is NULL");
		return -EINVAL;
	}

	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);
	if (!rx_ops || !rx_ops->vdev_mgr_stop_response) {
		mlme_err("No Rx Ops");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		mlme_err("wmi_handle is null");
		return -EINVAL;
	}

	if (wmi_extract_vdev_stopped_param(wmi_handle, data, &vdev_id)) {
		mlme_err("WMI extract failed");
		return -EINVAL;
	}

	rsp.vdev_id = vdev_id;
	status = rx_ops->vdev_mgr_stop_response(psoc, &rsp);

	return qdf_status_to_os_return(status);
}

static int target_if_vdev_mgr_delete_response_handler(
						ol_scn_t scn,
						uint8_t *data,
						uint32_t datalen)
{
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc;
	struct wmi_unified *wmi_handle;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;
	struct vdev_delete_response rsp = {0};
	struct wmi_host_vdev_delete_resp vdev_del_resp;

	if (!scn || !data) {
		mlme_err("scn: 0x%pK, data: 0x%pK", scn, data);
		return -EINVAL;
	}

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		mlme_err("PSOC is NULL");
		return -EINVAL;
	}

	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);
	if (!rx_ops || !rx_ops->vdev_mgr_stop_response ||
	    !rx_ops->vdev_mgr_get_response_timer_info) {
		mlme_err("No Rx Ops");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		mlme_err("wmi_handle is null");
		return -EINVAL;
	}

	if (wmi_extract_vdev_delete_resp(wmi_handle, data, &vdev_del_resp)) {
		mlme_err("WMI extract failed");
		return -EINVAL;
	}

	rsp.vdev_id = vdev_del_resp.vdev_id;
	status = rx_ops->vdev_mgr_delete_response(psoc, &rsp);

	return qdf_status_to_os_return(status);
}

static int target_if_vdev_mgr_offload_bcn_tx_status_handler(
							ol_scn_t scn,
							uint8_t *data,
							uint32_t datalen)
{
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc;
	struct wmi_unified *wmi_handle;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;
	uint32_t vdev_id, tx_status;

	if (!scn || !data) {
		mlme_err("scn: 0x%pK, data: 0x%pK", scn, data);
		return -EINVAL;
	}
	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		mlme_err("PSOC is NULL");
		return -EINVAL;
	}

	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);
	if (!rx_ops || !rx_ops->vdev_mgr_offload_bcn_tx_status_event_handle) {
		mlme_err("No Rx Ops");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		mlme_err("wmi_handle is null");
		return -EINVAL;
	}

	if (wmi_extract_offload_bcn_tx_status_evt(wmi_handle, data,
						  &vdev_id, &tx_status)) {
		mlme_err("WMI extract failed");
		return -EINVAL;
	}

	status = rx_ops->vdev_mgr_offload_bcn_tx_status_event_handle(
								vdev_id,
								tx_status);

	return qdf_status_to_os_return(status);
}

static int target_if_vdev_mgr_tbttoffset_update_handler(
						ol_scn_t scn,
						uint8_t *data,
						uint32_t datalen)
{
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc;
	struct wmi_unified *wmi_handle;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;
	uint32_t num_vdevs = 0;

	if (!scn || !data) {
		mlme_err("scn: 0x%pK, data: 0x%pK", scn, data);
		return -EINVAL;
	}
	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		mlme_err("PSOC is NULL");
		return -EINVAL;
	}

	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);
	if (!rx_ops || !rx_ops->vdev_mgr_tbttoffset_update_handle) {
		mlme_err("No Rx Ops");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		mlme_err("wmi_handle is null");
		return -EINVAL;
	}

	if (wmi_extract_tbttoffset_num_vdevs(wmi_handle, data, &num_vdevs)) {
		mlme_err("WMI extract failed");
		return -EINVAL;
	}

	status = rx_ops->vdev_mgr_tbttoffset_update_handle(num_vdevs, false);

	return qdf_status_to_os_return(status);
}

static int target_if_vdev_mgr_ext_tbttoffset_update_handler(
						ol_scn_t scn,
						uint8_t *data,
						uint32_t datalen)
{
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc;
	struct wmi_unified *wmi_handle;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;
	uint32_t num_vdevs = 0;

	if (!scn || !data) {
		mlme_err("scn: 0x%pK, data: 0x%pK", scn, data);
		return -EINVAL;
	}
	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		mlme_err("PSOC is NULL");
		return -EINVAL;
	}

	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);
	if (!rx_ops || !rx_ops->vdev_mgr_tbttoffset_update_handle) {
		mlme_err("No Rx Ops");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		mlme_err("wmi_handle is null");
		return -EINVAL;
	}

	if (wmi_extract_ext_tbttoffset_num_vdevs(wmi_handle, data,
						 &num_vdevs)) {
		mlme_err("WMI extract failed");
		return -EINVAL;
	}

	status = rx_ops->vdev_mgr_tbttoffset_update_handle(num_vdevs, true);

	return qdf_status_to_os_return(status);
}

QDF_STATUS target_if_vdev_mgr_wmi_event_register(
				struct wlan_objmgr_psoc *psoc)
{
	int retval = 0;
	struct wmi_unified *wmi_handle;

	if (!psoc) {
		mlme_err("PSOC is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		mlme_err("wmi_handle is null");
		return QDF_STATUS_E_INVAL;
	}

	retval = wmi_unified_register_event_handler(
				wmi_handle,
				wmi_vdev_stopped_event_id,
				target_if_vdev_mgr_stop_response_handler,
				WMI_RX_UMAC_CTX);
	if (retval)
		mlme_err("failed to register for stop response");

	retval = wmi_unified_register_event_handler(
				wmi_handle,
				wmi_vdev_delete_resp_event_id,
				target_if_vdev_mgr_delete_response_handler,
				WMI_RX_UMAC_CTX);
	if (retval)
		mlme_err("failed to register for delete response");

	retval = wmi_unified_register_event_handler(
				wmi_handle,
				wmi_vdev_start_resp_event_id,
				target_if_vdev_mgr_start_response_handler,
				WMI_RX_UMAC_CTX);
	if (retval)
		mlme_err("failed to register for start response");

	return qdf_status_from_os_return(retval);
}

QDF_STATUS target_if_vdev_mgr_wmi_event_unregister(
					struct wlan_objmgr_psoc *psoc)
{
	struct wmi_unified *wmi_handle;

	if (!psoc) {
		mlme_err("PSOC is NULL");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		mlme_err("wmi_handle is null");
		return QDF_STATUS_E_INVAL;
	}

	wmi_unified_unregister_event_handler(wmi_handle,
					     wmi_vdev_stopped_event_id);

	wmi_unified_unregister_event_handler(wmi_handle,
					     wmi_vdev_delete_resp_event_id);

	wmi_unified_unregister_event_handler(wmi_handle,
					     wmi_vdev_start_resp_event_id);

	return QDF_STATUS_SUCCESS;
}
