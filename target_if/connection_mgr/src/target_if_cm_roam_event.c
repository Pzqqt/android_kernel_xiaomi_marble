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
 * DOC: This file contains definitions for target_if roaming events.
 */
#include "qdf_types.h"
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_objmgr_pdev_obj.h"
#include "wlan_objmgr_vdev_obj.h"
#include "wmi_unified_api.h"
#include "scheduler_api.h"
#include <wmi_unified.h>
#include "target_if_cm_roam_event.h"
#include "wlan_psoc_mlme_api.h"
#include "wlan_mlme_main.h"
#include <../../core/src/wlan_cm_roam_i.h>
#include "wlan_cm_roam_api.h"

static inline struct wlan_cm_roam_rx_ops *
target_if_cm_get_roam_rx_ops(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_mlme_psoc_ext_obj *psoc_ext_priv;
	struct wlan_cm_roam_rx_ops *rx_ops;

	if (!psoc) {
		target_if_err("psoc object is NULL");
		return NULL;
	}
	psoc_ext_priv = wlan_psoc_mlme_get_ext_hdl(psoc);
	if (!psoc_ext_priv) {
		target_if_err("psoc legacy private object is NULL");
		return NULL;
	}

	rx_ops = &psoc_ext_priv->rso_rx_ops;
	return rx_ops;
}

void
target_if_cm_roam_register_rx_ops(struct wlan_cm_roam_rx_ops *rx_ops)
{
#ifdef ROAM_TARGET_IF_CONVERGENCE
	rx_ops->roam_sync_event = cm_roam_sync_event_handler;
	rx_ops->roam_sync_frame_event = cm_roam_sync_frame_event_handler;
	rx_ops->roam_event_rx = cm_roam_event_handler;
	rx_ops->btm_blacklist_event = cm_btm_blacklist_event_handler;
	rx_ops->vdev_disconnect_event = cm_vdev_disconnect_event_handler;
	rx_ops->roam_scan_chan_list_event = cm_roam_scan_ch_list_event_handler;
#endif
}

#ifdef ROAM_TARGET_IF_CONVERGENCE
int
target_if_cm_roam_sync_frame_event(ol_scn_t scn,
				   uint8_t *event,
				   uint32_t len)
{
	QDF_STATUS qdf_status;
	struct roam_synch_frame_ind *frame_ind_ptr;
	struct wmi_unified *wmi_handle;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_cm_roam_rx_ops *roam_rx_ops;
	int status = 0;

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null");
		return -EINVAL;
	}

	frame_ind_ptr = qdf_mem_malloc(sizeof(*frame_ind_ptr));

	if (!frame_ind_ptr)
		return -ENOMEM;

	qdf_status = wmi_extract_roam_sync_frame_event(wmi_handle, event,
						       len,
						       frame_ind_ptr);
	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		target_if_err("parsing of event failed, %d", qdf_status);
		status = -EINVAL;
		goto err;
	}

	roam_rx_ops = target_if_cm_get_roam_rx_ops(psoc);

	if (!roam_rx_ops || !roam_rx_ops->roam_sync_frame_event) {
		target_if_err("No valid roam rx ops");
		status = -EINVAL;
		goto err;
	}

	qdf_status = roam_rx_ops->roam_sync_frame_event(psoc,
						    frame_ind_ptr);

	if (QDF_IS_STATUS_ERROR(qdf_status))
		status = -EINVAL;

err:
	qdf_mem_free(frame_ind_ptr);
	return status;
}

int target_if_cm_roam_sync_event(ol_scn_t scn, uint8_t *event,
				 uint32_t len)
{
	QDF_STATUS qdf_status;
	struct wmi_unified *wmi_handle;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_cm_roam_rx_ops *roam_rx_ops;
	struct roam_offload_synch_ind *sync_ind = NULL;
	int status = 0;

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null");
		return -EINVAL;
	}

	status = wmi_extract_roam_sync_event(wmi_handle, event,
					     len, &sync_ind);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("parsing of event failed, %d", status);
		status = -EINVAL;
		goto err;
	}

	roam_rx_ops = target_if_cm_get_roam_rx_ops(psoc);

	if (!roam_rx_ops || !roam_rx_ops->roam_sync_event) {
		target_if_err("No valid roam rx ops");
		status = -EINVAL;
		goto err;
	}

	qdf_status = roam_rx_ops->roam_sync_event(psoc,
						  event,
						  len,
						  sync_ind);

	if (QDF_IS_STATUS_ERROR(qdf_status))
		status = -EINVAL;

err:
	if (sync_ind && sync_ind->ric_tspec_data)
		qdf_mem_free(sync_ind->ric_tspec_data);
	if (sync_ind)
		qdf_mem_free(sync_ind);
	return status;
}

int target_if_cm_roam_event(ol_scn_t scn, uint8_t *event, uint32_t len)
{
	QDF_STATUS qdf_status;
	int status = 0;
	struct wmi_unified *wmi_handle;
	struct roam_offload_roam_event roam_event = {0};
	struct wlan_objmgr_psoc *psoc;
	struct wlan_cm_roam_rx_ops *roam_rx_ops;

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null");
		return -EINVAL;
	}

	qdf_status = wmi_extract_roam_event(wmi_handle, event, len,
					    &roam_event);
	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		target_if_err("parsing of event failed, %d", qdf_status);
		return -EINVAL;
	}

	roam_rx_ops = target_if_cm_get_roam_rx_ops(psoc);
	if (!roam_rx_ops || !roam_rx_ops->roam_event_rx) {
		target_if_err("No valid roam rx ops");
		status = -EINVAL;
		goto err;
	}
	qdf_status = roam_rx_ops->roam_event_rx(roam_event);
	if (QDF_IS_STATUS_ERROR(qdf_status))
		status = -EINVAL;

err:
	return status;
}

static int
target_if_cm_btm_blacklist_event(ol_scn_t scn, uint8_t *event, uint32_t len)
{
	QDF_STATUS qdf_status;
	int status = 0;
	struct roam_blacklist_event *dst_list = NULL;
	struct wmi_unified *wmi_handle;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_cm_roam_rx_ops *roam_rx_ops;

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null");
		return -EINVAL;
	}

	qdf_status = wmi_extract_btm_blacklist_event(wmi_handle, event, len,
						     &dst_list);
	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		target_if_err("parsing of event failed, %d", qdf_status);
		return -EINVAL;
	}

	roam_rx_ops = target_if_cm_get_roam_rx_ops(psoc);
	if (!roam_rx_ops || !roam_rx_ops->btm_blacklist_event) {
		target_if_err("No valid roam rx ops");
		qdf_mem_free(dst_list);
		return -EINVAL;
	}
	qdf_status = roam_rx_ops->btm_blacklist_event(psoc, dst_list);
	if (QDF_IS_STATUS_ERROR(qdf_status))
		status = -EINVAL;

	return status;
}

int
target_if_cm_roam_vdev_disconnect_event_handler(ol_scn_t scn, uint8_t *event,
						uint32_t len)
{
	QDF_STATUS qdf_status;
	int status = 0;
	struct wmi_unified *wmi_handle;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_cm_roam_rx_ops *roam_rx_ops;
	struct vdev_disconnect_event_data data = {0};

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null");
		return -EINVAL;
	}

	qdf_status = wmi_extract_vdev_disconnect_event(wmi_handle, event, len,
						       &data);
	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		target_if_err("parsing of event failed, %d", qdf_status);
		return -EINVAL;
	}

	roam_rx_ops = target_if_cm_get_roam_rx_ops(psoc);
	if (!roam_rx_ops || !roam_rx_ops->vdev_disconnect_event) {
		target_if_err("No valid roam rx ops");
		status = -EINVAL;
		goto err;
	}
	qdf_status = roam_rx_ops->vdev_disconnect_event(&data);
	if (QDF_IS_STATUS_ERROR(qdf_status))
		status = -EINVAL;

err:
	return status;
}

int
target_if_cm_roam_scan_chan_list_event_handler(ol_scn_t scn, uint8_t *event,
					       uint32_t len)
{
	QDF_STATUS qdf_status;
	int status = 0;
	struct wmi_unified *wmi_handle;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_cm_roam_rx_ops *roam_rx_ops;
	struct cm_roam_scan_ch_resp *data = NULL;

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null");
		return -EINVAL;
	}

	qdf_status = wmi_extract_roam_scan_chan_list(wmi_handle, event, len,
						     &data);
	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		target_if_err("parsing of event failed, %d", qdf_status);
		return -EINVAL;
	}

	roam_rx_ops = target_if_cm_get_roam_rx_ops(psoc);
	if (!roam_rx_ops || !roam_rx_ops->roam_scan_chan_list_event) {
		target_if_err("No valid roam rx ops");
		qdf_mem_free(data);
		return -EINVAL;
	}
	qdf_status = roam_rx_ops->roam_scan_chan_list_event(data);
	if (QDF_IS_STATUS_ERROR(qdf_status))
		status = -EINVAL;

	return status;
}

QDF_STATUS
target_if_roam_offload_register_events(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS ret;
	wmi_unified_t handle = get_wmi_unified_hdl_from_psoc(psoc);

	if (!handle) {
		target_if_err("handle is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	/* Register for roam offload event */
	ret = wmi_unified_register_event_handler(handle,
						 wmi_roam_synch_event_id,
						 target_if_cm_roam_sync_event,
						 WMI_RX_SERIALIZER_CTX);
	if (QDF_IS_STATUS_ERROR(ret)) {
		target_if_err("wmi event registration failed, ret: %d", ret);
		return QDF_STATUS_E_FAILURE;
	}

	/* Register for roam offload event */
	ret = wmi_unified_register_event_handler(handle,
						 wmi_roam_synch_frame_event_id,
						 target_if_cm_roam_sync_frame_event,
						 WMI_RX_SERIALIZER_CTX);
	if (QDF_IS_STATUS_ERROR(ret)) {
		target_if_err("wmi event registration failed, ret: %d", ret);
		return QDF_STATUS_E_FAILURE;
	}
	ret = wmi_unified_register_event_handler(handle, wmi_roam_event_id,
						 target_if_cm_roam_event,
						 WMI_RX_SERIALIZER_CTX);
	if (QDF_IS_STATUS_ERROR(ret)) {
		target_if_err("wmi event registration failed, ret: %d", ret);
		return QDF_STATUS_E_FAILURE;
	}

	ret = wmi_unified_register_event_handler(handle,
					wmi_roam_blacklist_event_id,
					target_if_cm_btm_blacklist_event,
					WMI_RX_SERIALIZER_CTX);
	if (QDF_IS_STATUS_ERROR(ret)) {
		target_if_err("wmi event(%u) registration failed, ret: %d",
			      wmi_roam_blacklist_event_id, ret);
		return QDF_STATUS_E_FAILURE;
	}

	ret = wmi_unified_register_event_handler(handle,
				wmi_vdev_disconnect_event_id,
				target_if_cm_roam_vdev_disconnect_event_handler,
				WMI_RX_SERIALIZER_CTX);
	if (QDF_IS_STATUS_ERROR(ret)) {
		target_if_err("wmi event(%u) registration failed, ret: %d",
			      wmi_vdev_disconnect_event_id, ret);
		return QDF_STATUS_E_FAILURE;
	}

	ret = wmi_unified_register_event_handler(handle,
				wmi_roam_scan_chan_list_id,
				target_if_cm_roam_scan_chan_list_event_handler,
				WMI_RX_SERIALIZER_CTX);
	if (QDF_IS_STATUS_ERROR(ret)) {
		target_if_err("wmi event(%u) registration failed, ret: %d",
			      wmi_roam_scan_chan_list_id, ret);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

#endif
