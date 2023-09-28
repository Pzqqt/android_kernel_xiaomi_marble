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
 * DOC: This file contains the API definitions for the Unified Wireless Module
 * Interface (WMI) which are specific to DFS module.
 */

#ifndef _WMI_UNIFIED_DCS_API_H_
#define _WMI_UNIFIED_DCS_API_H_

#include <qdf_status.h>
#include <wmi_unified_api.h>
#include <wmi_unified_priv.h>
#include <wlan_objmgr_vdev_obj.h>

/**
 * wmi_extract_dcs_interference_type() - extract dcs interference type
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @param: Pointer to hold dcs interference param
 *
 * This function gets called to extract dcs interference type from dcs FW event
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_dcs_interference_type(
	void *wmi_hdl,
	void *evt_buf,
	struct wlan_host_dcs_interference_param *param);

/**
 * wmi_extract_dcs_im_tgt_stats() - extract dcs im target stats
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @wlan_stat: Pointer to hold wlan stats
 *
 * This function gets called to extract dcs im target stats from event
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_dcs_im_tgt_stats(
	void *wmi_hdl,
	void *evt_buf,
	struct wlan_host_dcs_im_tgt_stats *wlan_stat);

/*
 * wmi_extract_dcs_awgn_info() - extract DCS AWGN interference info from event
 * @wmi_handle: WMI handle
 * @evt_buf: Pointer to event buffer
 * @awgn_info: Pointer to hold AWGN interference info
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_dcs_awgn_info(wmi_unified_t wmi_hdl, void *evt_buf,
			  struct wlan_host_dcs_awgn_info *awgn_info);

/**
 * wmi_send_dcs_pdev_param() - send dcs pdev param
 * @wmi_handle: wmi handle
 * @pdev_idx: pdev id
 * @is_host_pdev_id: host pdev_id or not
 * @dcs_enable: value of dcs enable
 *
 * This functions gets called to send dcs pdev param
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wmi_send_dcs_pdev_param(wmi_unified_t wmi_handle,
				   uint32_t pdev_idx,
				   bool is_host_pdev_id,
				   uint32_t dcs_enable);
#endif /* _WMI_UNIFIED_DCS_API_H_ */
