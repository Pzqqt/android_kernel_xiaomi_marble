/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
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
 * This file contains the API declarations for the Unified Wireless Module
 * Interface (WMI).
 */

#ifndef _WMI_UNIFIED_VDEV_API_H_
#define _WMI_UNIFIED_VDEV_API_H_

#include <wmi_unified_api.h>

/**
 *  wmi_unified_vdev_set_neighbour_rx_cmd_send() - WMI set neighbour rx function
 *  @param wmi_handle: handle to WMI.
 *  @param macaddr: MAC address
 *  @param param: pointer to hold neighbour rx parameter
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_set_neighbour_rx_cmd_send(
				struct wmi_unified *wmi_handle,
				uint8_t macaddr[QDF_MAC_ADDR_SIZE],
				struct set_neighbour_rx_params *param);

/**
 *  wmi_unified_vdev_config_ratemask_cmd_send() - WMI config ratemask function
 *  @param wmi_handle: handle to WMI.
 *  @param param: pointer to hold config ratemask param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_vdev_config_ratemask_cmd_send(struct wmi_unified *wmi_handle,
					  struct config_ratemask_params *param);

/**
 * wmi_unified_send_multiple_vdev_restart_req_cmd() - send multiple vdev restart
 * @wmi_handle: wmi handle
 * @param: multiple vdev restart parameter
 *
 * Send WMI_PDEV_MULTIPLE_VDEV_RESTART_REQUEST_CMDID parameters to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wmi_unified_send_multiple_vdev_restart_req_cmd(
				struct wmi_unified *wmi_handle,
				struct multiple_vdev_restart_params *param);

/**
 *  wmi_unified_beacon_send_cmd() - WMI beacon send function
 *  @param wmi_handle: handle to WMI.
 *  @param macaddr: MAC address
 *  @param param: pointer to hold beacon send cmd parameter
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_beacon_send_cmd(struct wmi_unified *wmi_handle,
				       struct beacon_params *param);

/**
 * wmi_extract_vdev_start_resp() - extract vdev start response
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param vdev_rsp: Pointer to hold vdev response
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_vdev_start_resp(struct wmi_unified *wmi_handle, void *evt_buf,
			    wmi_host_vdev_start_resp *vdev_start_resp);

/**
 * wmi_extract_vdev_stopped_param() - extract vdev stop param from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param vdev_id: Pointer to hold vdev identifier
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_vdev_stopped_param(struct wmi_unified *wmi_handle, void *evt_buf,
			       uint32_t *vdev_id);

/**
 * wmi_extract_vdev_delete_resp() - extract vdev delete response
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param delete_rsp: Pointer to hold vdev delete response
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_vdev_delete_resp(struct wmi_unified *wmi_handle, void *evt_buf,
			     struct wmi_host_vdev_delete_resp *vdev_del_resp);

/**
 * wmi_extract_vdev_peer_delete_all_response_event() - extract peer delete all
 * response
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param delete_rsp: Pointer to hold peer delete al response
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_vdev_peer_delete_all_response_event(
	struct wmi_unified *wmi_handle,
	void *evt_buf,
	struct wmi_host_vdev_peer_delete_all_response_event *delete_rsp);

/**
 * wmi_extract_ext_tbttoffset_num_vdevs() - extract ext tbtt offset num vdev
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param vdev_map: Pointer to hold num vdev
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_ext_tbttoffset_num_vdevs(struct wmi_unified *wmi_handle,
				     void *evt_buf, uint32_t *num_vdevs);

/**
 * wmi_extract_tbttoffset_num_vdevs() - extract tbtt offset num vdev
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param vdev_map: Pointer to hold num vdev
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_tbttoffset_num_vdevs(struct wmi_unified *wmi_handle, void *evt_buf,
				 uint32_t *num_vdevs);

/**
 * wmi_extract_multi_vdev_restart_resp_event() - extract multi vdev restart
 * response
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @restart_rsp: Pointer to hold multi vdev restart response
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_multi_vdev_restart_resp_event(
		struct wmi_unified *wmi_handle,
		void *evt_buf,
		struct multi_vdev_restart_resp *restart_rsp);
#endif
