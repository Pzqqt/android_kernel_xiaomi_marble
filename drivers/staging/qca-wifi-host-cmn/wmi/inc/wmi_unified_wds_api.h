/*
 * Copyright (c) 2021 The Linux Foundation. All rights reserved.
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
 * This file contains the API declaration for the WDS WMI APIs.
 */

#ifndef _WMI_UNIFIED_WDS_API_H_
#define _WMI_UNIFIED_WDS_API_H_

#ifdef WDS_CONV_TARGET_IF_OPS_ENABLE
#include <wmi_unified_api.h>
#include <wmi_unified_wds_param.h>

/**
 *  wmi_unified_peer_add_wds_entry_cmd() - WMI add wds entry cmd function
 *  @wmi_hdl: handle to WMI
 *  @param: pointer to hold wds entry param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_peer_add_wds_entry_cmd(wmi_unified_t wmi_handle,
				   struct peer_add_wds_entry_params *param);

/**
 *  wmi_unified_peer_del_wds_entry_cmd() - WMI del wds entry cmd function
 *  @wmi_hdl: handle to WMI
 *  @param: pointer to hold wds entry param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_peer_del_wds_entry_cmd(wmi_unified_t wmi_handle,
				   struct peer_del_wds_entry_params *param);

/**
 *  wmi_unified_update_wds_entry_cmd() - WMI update wds entry
 *  cmd function
 *  @wmi_hdl: handle to WMI
 *  @param: pointer to hold wds entry param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_update_wds_entry_cmd(wmi_unified_t wmi_handle,
				 struct peer_update_wds_entry_params *param);

/**
 * wmi_wds_attach_tlv() - attach wds tlv handlers
 * @wmi_handle: wmi handle
 *
 * Return: void
 */
void wmi_wds_attach_tlv(wmi_unified_t wmi_handle);
#else
/**
 * wmi_wds_attach_tlv() - attach wds tlv handlers
 * @wmi_handle: wmi handle
 *
 * Return: void
 */
static inline void
wmi_wds_attach_tlv(struct wmi_unified *wmi_handle)
{
}
#endif /*WDS_CONV_TARGET_IF_OPS*/
#endif /*_WMI_UNIFIED_WDS_API_H_*/
