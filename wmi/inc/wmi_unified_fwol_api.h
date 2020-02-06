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
 * DOC: Implement API's specific to fw offload component.
 */

#ifndef _WMI_UNIFIED_FWOL_API_H_
#define _WMI_UNIFIED_FWOL_API_H_
#include "wlan_fwol_public_structs.h"

#ifdef WLAN_FEATURE_ELNA
/**
 * wmi_unified_send_set_elna_bypass_cmd() - Send WMI set eLNA bypass cmd
 * @wmi_handle: wmi handle
 * @req: set eLNA bypass request
 *
 * Send WMI set eLNA bypass command to firmware.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_unified_send_set_elna_bypass_cmd(struct wmi_unified *wmi_handle,
				     struct set_elna_bypass_request *req);

/**
 * wmi_unified_send_get_elna_bypass_cmd() - Send WMI get eLNA bypass cmd
 * @wmi_handle: wmi handle
 * @req: get eLNA bypass request
 *
 * Send WMI get eLNA bypass command to firmware.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_unified_send_get_elna_bypass_cmd(struct wmi_unified *wmi_handle,
				     struct get_elna_bypass_request *req);

/**
 * wmi_extract_get_elna_bypass_resp() - Extract WMI get eLNA bypass response
 * @wmi_handle: wmi handle
 * @resp_buf: response buffer
 * @resp: get eLNA bypass response
 *
 * Extract WMI get eLNA bypass response from firmware.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_extract_get_elna_bypass_resp(struct wmi_unified *wmi_handle, void *resp_buf,
				 struct get_elna_bypass_response *resp);
#endif /* WLAN_FEATURE_ELNA */

#ifdef WLAN_SEND_DSCP_UP_MAP_TO_FW
/**
 * wmi_unified_send_dscp_tip_map_cmd() - Send dscp-to-tid map values cmd
 * @wmi_handle: wmi handle
 * @dscp_to_tid_map: array of dscp_tid map values
 *
 * Send dscp-to-tid map values to FW.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_unified_send_dscp_tip_map_cmd(struct wmi_unified *wmi_handle,
				  uint32_t *dscp_to_tid_map);
#else
static inline QDF_STATUS
wmi_unified_send_dscp_tip_map_cmd(struct wmi_unified *wmi_handle,
				  uint32_t *dscp_to_tid_map)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_SEND_DSCP_UP_MAP_TO_FW */

#endif /* _WMI_UNIFIED_FWOL_API_H_ */
