/*
 * Copyright (c) 2019-2020 The Linux Foundation. All rights reserved.
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
#include <wlan_vdev_mgr_tgt_if_tx_defs.h>
#include <wmi_unified_param.h>
#include <wlan_mlme_dbg.h>
#include <target_if.h>
#include <wlan_vdev_mlme_main.h>
#include <wmi_unified_vdev_api.h>

void target_if_vdev_mgr_rsp_timer_cb(struct vdev_response_timer *vdev_rsp)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;
	struct crash_inject param;
	struct wmi_unified *wmi_handle;
	struct vdev_start_response start_rsp = {0};
	struct vdev_stop_response stop_rsp = {0};
	struct vdev_delete_response del_rsp = {0};
	struct peer_delete_all_response peer_del_all_rsp = {0};
	uint8_t vdev_id;
	uint16_t rsp_pos = RESPONSE_BIT_MAX;

	if (!vdev_rsp) {
		mlme_err("Vdev response timer is NULL");
		return;
	}

	psoc = vdev_rsp->psoc;
	if (!psoc) {
		mlme_err("PSOC is NULL");
		return;
	}

	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);
	if (!rx_ops || !rx_ops->psoc_get_vdev_response_timer_info) {
		mlme_err("No Rx Ops");
		return;
	}

	if (!qdf_atomic_test_bit(START_RESPONSE_BIT, &vdev_rsp->rsp_status) &&
	    !qdf_atomic_test_bit(RESTART_RESPONSE_BIT, &vdev_rsp->rsp_status) &&
	    !qdf_atomic_test_bit(STOP_RESPONSE_BIT, &vdev_rsp->rsp_status) &&
	    !qdf_atomic_test_bit(DELETE_RESPONSE_BIT, &vdev_rsp->rsp_status) &&
	    !qdf_atomic_test_bit(
			PEER_DELETE_ALL_RESPONSE_BIT,
			&vdev_rsp->rsp_status)) {
		mlme_debug("No response bit is set, ignoring actions :%d",
			   vdev_rsp->vdev_id);
		return;
	}

	vdev_id = vdev_rsp->vdev_id;
	if (vdev_id >= WLAN_UMAC_PSOC_MAX_VDEVS) {
		mlme_err("Invalid VDEV_%d PSOC_%d", vdev_id,
			 wlan_psoc_get_id(psoc));
		return;
	}

	vdev_rsp->timer_status = QDF_STATUS_E_TIMEOUT;
	if (qdf_atomic_test_bit(START_RESPONSE_BIT,
				&vdev_rsp->rsp_status) ||
	    qdf_atomic_test_bit(RESTART_RESPONSE_BIT,
				&vdev_rsp->rsp_status)) {
		start_rsp.vdev_id = vdev_id;
		start_rsp.status = WLAN_MLME_HOST_VDEV_START_TIMEOUT;
		if (qdf_atomic_test_bit(START_RESPONSE_BIT,
					&vdev_rsp->rsp_status)) {
			start_rsp.resp_type =
				WMI_HOST_VDEV_START_RESP_EVENT;
			rsp_pos = START_RESPONSE_BIT;
		} else {
			start_rsp.resp_type =
				WMI_HOST_VDEV_RESTART_RESP_EVENT;
			rsp_pos = RESTART_RESPONSE_BIT;
		}

		target_if_vdev_mgr_rsp_timer_stop(psoc, vdev_rsp, rsp_pos);

		rx_ops->vdev_mgr_start_response(psoc, &start_rsp);
	} else if (qdf_atomic_test_bit(STOP_RESPONSE_BIT,
				       &vdev_rsp->rsp_status)) {
		rsp_pos = STOP_RESPONSE_BIT;
		stop_rsp.vdev_id = vdev_id;
		target_if_vdev_mgr_rsp_timer_stop(psoc, vdev_rsp, rsp_pos);

		rx_ops->vdev_mgr_stop_response(psoc, &stop_rsp);
	} else if (qdf_atomic_test_bit(DELETE_RESPONSE_BIT,
				       &vdev_rsp->rsp_status)) {
		del_rsp.vdev_id = vdev_id;
		rsp_pos = DELETE_RESPONSE_BIT;
		target_if_vdev_mgr_rsp_timer_stop(psoc, vdev_rsp, rsp_pos);

		rx_ops->vdev_mgr_delete_response(psoc, &del_rsp);
	} else if (qdf_atomic_test_bit(PEER_DELETE_ALL_RESPONSE_BIT,
				&vdev_rsp->rsp_status)) {
		peer_del_all_rsp.vdev_id = vdev_id;
		rsp_pos = PEER_DELETE_ALL_RESPONSE_BIT;
		target_if_vdev_mgr_rsp_timer_stop(psoc, vdev_rsp, rsp_pos);

		rx_ops->vdev_mgr_peer_delete_all_response(
				psoc,
				&peer_del_all_rsp);
	} else {
		mlme_err("PSOC_%d VDEV_%d: Unknown error",
			 wlan_psoc_get_id(psoc), vdev_id);
		return;
	}

	if (!target_if_vdev_mgr_is_panic_allowed()) {
		mlme_debug("PSOC_%d VDEV_%d: Panic not allowed",
			   wlan_psoc_get_id(psoc), vdev_id);
		return;
	}

	/* Trigger fw recovery to collect fw dump */
	wmi_handle = GET_WMI_HDL_FROM_PSOC(psoc);
	if (wmi_handle) {
		mlme_err("PSOC_%d VDEV_%d: Self recovery, %s rsp timeout",
			 wlan_psoc_get_id(psoc), vdev_id,
			 string_from_rsp_bit(rsp_pos));
		qdf_mem_set(&param, sizeof(param), 0);
		/* RECOVERY_SIM_ASSERT */
		param.type = 0x01;
		wmi_crash_inject(wmi_handle, &param);
	} else if (target_if_vdev_mgr_is_panic_on_bug()) {
		QDF_DEBUG_PANIC("PSOC_%d VDEV_%d: Panic, %s response timeout",
				wlan_psoc_get_id(psoc),
				vdev_id, string_from_rsp_bit(rsp_pos));
	}
}

#ifdef SERIALIZE_VDEV_RESP
static QDF_STATUS target_if_vdev_mgr_rsp_flush_cb(struct scheduler_msg *msg)
{
	struct vdev_response_timer *vdev_rsp;
	struct wlan_objmgr_psoc *psoc;

	if (!msg->bodyptr) {
		mlme_err("Message bodyptr is NULL");
		return QDF_STATUS_E_INVAL;
	}

	vdev_rsp = msg->bodyptr;
	if (!vdev_rsp) {
		mlme_err("vdev response timer is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc = vdev_rsp->psoc;
	if (!psoc) {
		mlme_err("PSOC is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (vdev_rsp->rsp_status)
		wlan_objmgr_psoc_release_ref(psoc, WLAN_PSOC_TARGET_IF_ID);

	return QDF_STATUS_SUCCESS;
}

static void
target_if_vdev_mgr_rsp_cb_mc_ctx(void *arg)
{
	struct scheduler_msg msg = {0};
	struct vdev_response_timer *vdev_rsp = arg;
	struct wlan_objmgr_psoc *psoc;

	psoc = vdev_rsp->psoc;
	if (!psoc) {
		mlme_err("PSOC is NULL");
		return;
	}

	msg.type = SYS_MSG_ID_MC_TIMER;
	msg.reserved = SYS_MSG_COOKIE;
	msg.callback = target_if_vdev_mgr_rsp_timer_cb;
	msg.bodyptr = vdev_rsp;
	msg.bodyval = 0;
	msg.flush_callback = target_if_vdev_mgr_rsp_flush_cb;

	if (scheduler_post_message(QDF_MODULE_ID_TARGET_IF,
				   QDF_MODULE_ID_TARGET_IF,
				   QDF_MODULE_ID_SYS, &msg) ==
				   QDF_STATUS_SUCCESS)
		return;

	mlme_err("Could not enqueue timer to timer queue");
	if (psoc)
		wlan_objmgr_psoc_release_ref(psoc, WLAN_PSOC_TARGET_IF_ID);
}

void target_if_vdev_mgr_rsp_timer_mgmt_cb(void *arg)
{
	target_if_vdev_mgr_rsp_cb_mc_ctx(arg);
}

#define VDEV_RSP_RX_CTX WMI_RX_SERIALIZER_CTX
#else
void target_if_vdev_mgr_rsp_timer_mgmt_cb(void *arg)
{
	target_if_vdev_mgr_rsp_timer_cb(arg);
}

#define VDEV_RSP_RX_CTX WMI_RX_UMAC_CTX
#endif

static int target_if_vdev_mgr_start_response_handler(ol_scn_t scn,
						     uint8_t *data,
						     uint32_t datalen)
{
	QDF_STATUS status = QDF_STATUS_E_INVAL;
	struct wlan_objmgr_psoc *psoc;
	struct wmi_unified *wmi_handle;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;
	struct vdev_start_response rsp = {0};
	wmi_host_vdev_start_resp vdev_start_resp;
	uint8_t vdev_id;
	struct vdev_response_timer *vdev_rsp;

	if (!scn || !data) {
		mlme_err("Invalid input");
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

	vdev_id = vdev_start_resp.vdev_id;
	vdev_rsp = rx_ops->psoc_get_vdev_response_timer_info(psoc, vdev_id);
	if (!vdev_rsp) {
		mlme_err("vdev response timer is null VDEV_%d PSOC_%d",
			 vdev_id, wlan_psoc_get_id(psoc));
		return -EINVAL;
	}

	if (vdev_start_resp.resp_type == WMI_HOST_VDEV_RESTART_RESP_EVENT)
		status = target_if_vdev_mgr_rsp_timer_stop(
							psoc, vdev_rsp,
							RESTART_RESPONSE_BIT);
	else
		status = target_if_vdev_mgr_rsp_timer_stop(psoc, vdev_rsp,
							   START_RESPONSE_BIT);

	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("PSOC_%d VDEV_%d: VDE MGR RSP Timer stop failed",
			 psoc->soc_objmgr.psoc_id, vdev_id);
		goto err;
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

err:
	return qdf_status_to_os_return(status);
}

static int target_if_vdev_mgr_stop_response_handler(ol_scn_t scn,
						    uint8_t *data,
						    uint32_t datalen)
{
	QDF_STATUS status = QDF_STATUS_E_INVAL;
	struct wlan_objmgr_psoc *psoc;
	struct wmi_unified *wmi_handle;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;
	struct vdev_stop_response rsp = {0};
	uint32_t vdev_id;
	struct vdev_response_timer *vdev_rsp;

	if (!scn || !data) {
		mlme_err("Invalid input");
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

	vdev_rsp = rx_ops->psoc_get_vdev_response_timer_info(psoc, vdev_id);
	if (!vdev_rsp) {
		mlme_err("vdev response timer is null VDEV_%d PSOC_%d",
			 vdev_id, wlan_psoc_get_id(psoc));
		return -EINVAL;
	}

	status = target_if_vdev_mgr_rsp_timer_stop(psoc, vdev_rsp,
						   STOP_RESPONSE_BIT);

	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("PSOC_%d VDEV_%d: VDE MGR RSP Timer stop failed",
			 psoc->soc_objmgr.psoc_id, vdev_id);
		goto err;
	}

	rsp.vdev_id = vdev_id;
	status = rx_ops->vdev_mgr_stop_response(psoc, &rsp);

err:
	return qdf_status_to_os_return(status);
}

static int target_if_vdev_mgr_delete_response_handler(ol_scn_t scn,
						      uint8_t *data,
						      uint32_t datalen)
{
	QDF_STATUS status = QDF_STATUS_E_INVAL;
	struct wlan_objmgr_psoc *psoc;
	struct wmi_unified *wmi_handle;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;
	struct vdev_delete_response rsp = {0};
	struct wmi_host_vdev_delete_resp vdev_del_resp;
	struct vdev_response_timer *vdev_rsp;

	if (!scn || !data) {
		mlme_err("Invalid input");
		return -EINVAL;
	}

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		mlme_err("PSOC is NULL");
		return -EINVAL;
	}

	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);
	if (!rx_ops || !rx_ops->vdev_mgr_delete_response) {
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

	vdev_rsp = rx_ops->psoc_get_vdev_response_timer_info(psoc,
							 vdev_del_resp.vdev_id);
	if (!vdev_rsp) {
		mlme_err("vdev response timer is null VDEV_%d PSOC_%d",
			 vdev_del_resp.vdev_id, wlan_psoc_get_id(psoc));
		return -EINVAL;
	}

	status = target_if_vdev_mgr_rsp_timer_stop(
						psoc, vdev_rsp,
						DELETE_RESPONSE_BIT);

	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("PSOC_%d VDEV_%d: VDE MGR RSP Timer stop failed",
			 wlan_psoc_get_id(psoc), vdev_del_resp.vdev_id);
		goto err;
	}

	rsp.vdev_id = vdev_del_resp.vdev_id;
	status = rx_ops->vdev_mgr_delete_response(psoc, &rsp);

err:
	return qdf_status_to_os_return(status);
}

static int target_if_vdev_mgr_peer_delete_all_response_handler(
							ol_scn_t scn,
							uint8_t *data,
							uint32_t datalen)
{
	QDF_STATUS status = QDF_STATUS_E_INVAL;
	struct wlan_objmgr_psoc *psoc;
	struct wmi_unified *wmi_handle;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;
	struct peer_delete_all_response rsp = {0};
	struct wmi_host_vdev_peer_delete_all_response_event
						vdev_peer_del_all_resp;
	struct vdev_response_timer *vdev_rsp;

	if (!scn || !data) {
		mlme_err("Invalid input");
		return -EINVAL;
	}

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		mlme_err("PSOC is NULL");
		return -EINVAL;
	}

	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);
	if (!rx_ops || !rx_ops->vdev_mgr_peer_delete_all_response) {
		mlme_err("No Rx Ops");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		mlme_err("wmi_handle is null");
		return -EINVAL;
	}

	if (wmi_extract_vdev_peer_delete_all_response_event(
						wmi_handle, data,
						&vdev_peer_del_all_resp)) {
		mlme_err("WMI extract failed");
		return -EINVAL;
	}

	vdev_rsp = rx_ops->psoc_get_vdev_response_timer_info(psoc,
						vdev_peer_del_all_resp.vdev_id);
	if (!vdev_rsp) {
		mlme_err("vdev response timer is null VDEV_%d PSOC_%d",
			 vdev_peer_del_all_resp.vdev_id,
			 wlan_psoc_get_id(psoc));
		return -EINVAL;
	}

	status = target_if_vdev_mgr_rsp_timer_stop(
						psoc,
						vdev_rsp,
						PEER_DELETE_ALL_RESPONSE_BIT);

	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("PSOC_%d VDEV_%d: VDE MGR RSP Timer stop failed",
			 psoc->soc_objmgr.psoc_id,
			 vdev_peer_del_all_resp.vdev_id);
		goto err;
	}

	rsp.vdev_id = vdev_peer_del_all_resp.vdev_id;
	rsp.status = vdev_peer_del_all_resp.status;
	status = rx_ops->vdev_mgr_peer_delete_all_response(psoc, &rsp);

err:
	return qdf_status_to_os_return(status);
}

int target_if_vdev_mgr_offload_bcn_tx_status_handler(
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
		mlme_err("Invalid input");
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

int target_if_vdev_mgr_tbttoffset_update_handler(
						ol_scn_t scn, uint8_t *data,
						uint32_t datalen)
{
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc;
	struct wmi_unified *wmi_handle;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;
	uint32_t num_vdevs = 0;

	if (!scn || !data) {
		mlme_err("Invalid input");
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

	status = rx_ops->vdev_mgr_tbttoffset_update_handle(num_vdevs,
							   false);

	return qdf_status_to_os_return(status);
}

int target_if_vdev_mgr_ext_tbttoffset_update_handler(
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
		mlme_err("Invalid input");
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

	status = rx_ops->vdev_mgr_tbttoffset_update_handle(num_vdevs,
							   true);

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
				VDEV_RSP_RX_CTX);
	if (retval)
		mlme_err("failed to register for stop response");

	retval = wmi_unified_register_event_handler(
				wmi_handle,
				wmi_vdev_delete_resp_event_id,
				target_if_vdev_mgr_delete_response_handler,
				VDEV_RSP_RX_CTX);
	if (retval)
		mlme_err("failed to register for delete response");

	retval = wmi_unified_register_event_handler(
				wmi_handle,
				wmi_vdev_start_resp_event_id,
				target_if_vdev_mgr_start_response_handler,
				VDEV_RSP_RX_CTX);
	if (retval)
		mlme_err("failed to register for start response");

	retval = wmi_unified_register_event_handler(
			wmi_handle,
			wmi_peer_delete_all_response_event_id,
			target_if_vdev_mgr_peer_delete_all_response_handler,
			VDEV_RSP_RX_CTX);
	if (retval)
		mlme_err("failed to register for peer delete all response");

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

	wmi_unified_unregister_event_handler(
					wmi_handle,
					wmi_peer_delete_all_response_event_id);
	return QDF_STATUS_SUCCESS;
}
