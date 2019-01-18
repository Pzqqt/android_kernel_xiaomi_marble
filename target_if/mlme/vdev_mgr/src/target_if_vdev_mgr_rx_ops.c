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

void target_if_vdev_mgr_rsp_timer_mgmt_cb(void *arg)
{
	struct wlan_objmgr_vdev *vdev = arg;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;
	struct vdev_response_timer *vdev_rsp;

	mlme_debug("Response timer expired for VDEV %d",
		   wlan_vdev_get_id(vdev));

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		QDF_ASSERT(0);
		return;
	}

	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);
	if (!rx_ops || !rx_ops->vdev_mgr_get_response_timer_info) {
		mlme_err("No Rx Ops");
		return;
	}

	if (qdf_is_recovering()) {
		mlme_debug("Recovery in progress");
		return;
	}

	vdev_rsp = rx_ops->vdev_mgr_get_response_timer_info(vdev);
	qdf_atomic_clear_bit(START_RESPONSE_BIT, &vdev_rsp->rsp_status);
	qdf_atomic_clear_bit(STOP_RESPONSE_BIT, &vdev_rsp->rsp_status);
	qdf_atomic_clear_bit(DELETE_RESPONSE_BIT, &vdev_rsp->rsp_status);

	if (rx_ops->vdev_mgr_response_timeout_cb)
		rx_ops->vdev_mgr_response_timeout_cb(vdev);

	/* Implementation need to be done based on build type */
	QDF_ASSERT(0);
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
		QDF_ASSERT(0);
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
		QDF_ASSERT(0);
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
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_psoc *psoc;
	struct wmi_unified *wmi_handle;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;
	struct vdev_delete_response rsp = {0};
	struct wmi_host_vdev_delete_resp vdev_del_resp;
	struct vdev_response_timer *vdev_rsp;

	if (!scn || !data) {
		mlme_err("scn: 0x%pK, data: 0x%pK", scn, data);
		return -EINVAL;
	}

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		QDF_ASSERT(0);
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

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc,
						    vdev_del_resp.vdev_id,
						    WLAN_MLME_SB_ID);
	if (!vdev) {
		QDF_ASSERT(0);
		return -EINVAL;
	}

	rsp.vdev_id = vdev_del_resp.vdev_id;
	status = rx_ops->vdev_mgr_delete_response(psoc, &rsp);

	vdev_rsp = rx_ops->vdev_mgr_get_response_timer_info(vdev);
	if (vdev_rsp && QDF_IS_STATUS_SUCCESS(status))
		target_if_vdev_mgr_rsp_timer_mgmt(vdev, &vdev_rsp->rsp_timer,
						  false);

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_SB_ID);
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
		QDF_ASSERT(0);
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
		QDF_ASSERT(0);
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
		QDF_ASSERT(0);
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
		QDF_ASSERT(0);
		return QDF_STATUS_E_NULL_VALUE;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		mlme_err("wmi_handle is null");
		return QDF_STATUS_E_INVAL;
	}

#if CMN_VDEV_MGR_TGT_IF_ENABLE_PHASE /* to be implemented in next phase */
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

	retval = wmi_unified_register_event_handler(
			wmi_handle,
			wmi_offload_bcn_tx_status_event_id,
			target_if_vdev_mgr_offload_bcn_tx_status_handler,
			WMI_RX_UMAC_CTX);
	if (retval)
		mlme_err("failed to register for bcn tx status response");

	retval = wmi_unified_register_event_handler(
				wmi_handle,
				wmi_tbttoffset_update_event_id,
				target_if_vdev_mgr_tbttoffset_update_handler,
				WMI_RX_UMAC_CTX);
	if (retval)
		mlme_err("failed to register for tbttoffset update");

	retval = wmi_unified_register_event_handler(
			wmi_handle,
			wmi_ext_tbttoffset_update_event_id,
			target_if_vdev_mgr_ext_tbttoffset_update_handler,
			WMI_RX_UMAC_CTX);
	if (retval)
		mlme_err("failed to register for ext tbttoffset update");
#endif

	return qdf_status_from_os_return(retval);
}

QDF_STATUS target_if_vdev_mgr_wmi_event_unregister(
					struct wlan_objmgr_psoc *psoc)
{
	struct wmi_unified *wmi_handle;

	if (!psoc) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		mlme_err("wmi_handle is null");
		return QDF_STATUS_E_INVAL;
	}

#if CMN_VDEV_MGR_TGT_IF_ENABLE_PHASE
	wmi_unified_unregister_event_handler(wmi_handle,
					     wmi_vdev_stopped_event_id);

	wmi_unified_unregister_event_handler(wmi_handle,
					     wmi_vdev_delete_resp_event_id);

	wmi_unified_unregister_event_handler(wmi_handle,
					     wmi_vdev_start_resp_event_id);

	wmi_unified_unregister_event_handler(wmi_handle,
					     wmi_tbttoffset_update_event_id);

	wmi_unified_unregister_event_handler(
				wmi_handle,
				wmi_ext_tbttoffset_update_event_id);

	wmi_unified_unregister_event_handler(
				wmi_handle,
				wmi_offload_bcn_tx_status_event_id);
#endif

	return QDF_STATUS_SUCCESS;
}
