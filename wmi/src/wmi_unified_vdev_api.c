/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
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
 * This file contains the API definitions for the Unified Wireless Module
 * Interface (WMI).
 */
#include <htc_api.h>
#include <htc_api.h>
#include <wmi_unified_priv.h>
#include <wmi_unified_api.h>
#include <qdf_module.h>
#include <qdf_list.h>
#include <qdf_platform.h>
#ifdef WMI_EXT_DBG
#include <qdf_list.h>
#endif

#include <linux/debugfs.h>
#include <wmi_unified_param.h>
#include <cdp_txrx_cmn_struct.h>
#include <wmi_unified_vdev_api.h>

QDF_STATUS
wmi_extract_tbttoffset_num_vdevs(struct wmi_unified *wmi_handle, void *evt_buf,
				 uint32_t *num_vdevs)
{
	if (wmi_handle->ops->extract_tbttoffset_num_vdevs)
		return wmi_handle->ops->extract_tbttoffset_num_vdevs(wmi_handle,
								     evt_buf,
								     num_vdevs);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_send_multiple_vdev_restart_req_cmd(
				struct wmi_unified *wmi_handle,
				struct multiple_vdev_restart_params *param)
{
	if (wmi_handle->ops->send_multiple_vdev_restart_req_cmd)
		return wmi_handle->ops->send_multiple_vdev_restart_req_cmd(
							wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_send_multiple_vdev_set_param_cmd(
				struct wmi_unified *wmi_handle,
				struct multiple_vdev_set_param *param)
{
	if (wmi_handle->ops->send_multiple_vdev_set_param_cmd)
		return wmi_handle->ops->send_multiple_vdev_set_param_cmd(
							wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_beacon_send_cmd(struct wmi_unified *wmi_handle,
				       struct beacon_params *param)
{
	if (wmi_handle->ops->send_beacon_send_cmd)
		return wmi_handle->ops->send_beacon_send_cmd(wmi_handle,
							     param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_vdev_start_resp(struct wmi_unified *wmi_handle, void *evt_buf,
			    struct vdev_start_response *vdev_rsp)
{
	if (wmi_handle->ops->extract_vdev_start_resp)
		return wmi_handle->ops->extract_vdev_start_resp(wmi_handle,
								evt_buf,
								vdev_rsp);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_vdev_delete_resp(struct wmi_unified *wmi_handle, void *evt_buf,
			     struct vdev_delete_response *delete_rsp)
{
	if (wmi_handle->ops->extract_vdev_delete_resp)
		return wmi_handle->ops->extract_vdev_delete_resp(wmi_handle,
								 evt_buf,
								 delete_rsp);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_vdev_peer_delete_all_response_event(
	struct wmi_unified *wmi_handle,
	void *evt_buf,
	struct peer_delete_all_response *delete_rsp)
{
	if (wmi_handle->ops->extract_vdev_peer_delete_all_response_event)
		return
			wmi_handle->ops->extract_vdev_peer_delete_all_response_event(
								wmi_handle,
								 evt_buf,
								 delete_rsp);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_vdev_stopped_param(struct wmi_unified *wmi_handle, void *evt_buf,
			       uint32_t *vdev_id)
{
	if (wmi_handle->ops->extract_vdev_stopped_param)
		return wmi_handle->ops->extract_vdev_stopped_param(wmi_handle,
								   evt_buf,
								   vdev_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_ext_tbttoffset_num_vdevs(struct wmi_unified *wmi_handle,
				     void *evt_buf, uint32_t *num_vdevs)
{
	if (wmi_handle->ops->extract_ext_tbttoffset_num_vdevs)
		return wmi_handle->ops->extract_ext_tbttoffset_num_vdevs(
							wmi_handle,
							evt_buf, num_vdevs);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_vdev_config_ratemask_cmd_send(struct wmi_unified *wmi_handle,
					  struct config_ratemask_params *param)
{
	if (wmi_handle->ops->send_vdev_config_ratemask_cmd)
		return wmi_handle->ops->send_vdev_config_ratemask_cmd(
							wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_vdev_set_neighbour_rx_cmd_send(
				struct wmi_unified *wmi_handle,
				uint8_t macaddr[QDF_MAC_ADDR_SIZE],
				struct set_neighbour_rx_params *param)
{
	if (wmi_handle->ops->send_vdev_set_neighbour_rx_cmd)
		return wmi_handle->ops->send_vdev_set_neighbour_rx_cmd(
							wmi_handle,
							macaddr, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_multi_vdev_restart_resp_event(
	struct wmi_unified *wmi_handle,
	void *evt_buf,
	struct multi_vdev_restart_resp *restart_rsp)
{
	if (wmi_handle->ops->extract_multi_vdev_restart_resp_event)
		return wmi_handle->ops->extract_multi_vdev_restart_resp_event(
				wmi_handle, evt_buf, restart_rsp);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_multisoc_tbtt_sync_cmd(wmi_unified_t wmi_handle,
			struct rnr_tbtt_multisoc_sync_param *param)
{
	if (wmi_handle->ops->multisoc_tbtt_sync_cmd)
		return wmi_handle->ops->multisoc_tbtt_sync_cmd(wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}
