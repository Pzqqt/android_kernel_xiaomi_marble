/*
 * Copyright (c) 2019-2021 The Linux Foundation. All rights reserved.
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

#include "wmi_unified_priv.h"
#include "wlan_fwol_public_structs.h"
#include "wmi_unified_fwol_api.h"

#ifdef WLAN_FEATURE_ELNA
QDF_STATUS
wmi_unified_send_set_elna_bypass_cmd(struct wmi_unified *wmi_handle,
				     struct set_elna_bypass_request *req)
{
	if (wmi_handle->ops->send_set_elna_bypass_cmd)
		return wmi_handle->ops->send_set_elna_bypass_cmd(wmi_handle,
								 req);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_send_get_elna_bypass_cmd(struct wmi_unified *wmi_handle,
				     struct get_elna_bypass_request *req)
{
	if (wmi_handle->ops->send_get_elna_bypass_cmd)
		return wmi_handle->ops->send_get_elna_bypass_cmd(wmi_handle,
								 req);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_get_elna_bypass_resp(struct wmi_unified *wmi_handle, void *resp_buf,
				 struct get_elna_bypass_response *resp)
{
	if (wmi_handle->ops->extract_get_elna_bypass_resp)
		return wmi_handle->ops->extract_get_elna_bypass_resp(wmi_handle,
								     resp_buf,
								     resp);

	return QDF_STATUS_E_FAILURE;
}
#endif /* WLAN_FEATURE_ELNA */

#ifdef WLAN_SEND_DSCP_UP_MAP_TO_FW
QDF_STATUS
wmi_unified_send_dscp_tip_map_cmd(struct wmi_unified *wmi_handle,
				  uint32_t *dscp_to_tid_map)
{
	if (wmi_handle->ops->send_dscp_tid_map_cmd)
		return wmi_handle->ops->send_dscp_tid_map_cmd(wmi_handle,
							      dscp_to_tid_map);

	return QDF_STATUS_E_FAILURE;
}
#endif /* WLAN_SEND_DSCP_UP_MAP_TO_FW */

#ifdef WLAN_FEATURE_MDNS_OFFLOAD
QDF_STATUS
wmi_unified_send_set_mdns_config_cmd(struct wmi_unified *wmi_handle,
				     struct mdns_config_info *mdns_info)
{
	if (wmi_handle->ops->send_set_mdns_config_cmd)
		return wmi_handle->ops->send_set_mdns_config_cmd(wmi_handle,
								 mdns_info);

	return QDF_STATUS_E_FAILURE;
}
#endif /* WLAN_FEATURE_MDNS_OFFLOAD */

#ifdef THERMAL_STATS_SUPPORT
QDF_STATUS
wmi_unified_send_get_thermal_stats_cmd(struct wmi_unified *wmi_handle,
				       enum thermal_stats_request_type req_type,
				       uint8_t temp_offset)
{
	if (wmi_handle->ops->send_get_thermal_stats_cmd)
		return wmi_handle->ops->send_get_thermal_stats_cmd(wmi_handle,
								   req_type,
								   temp_offset);

	return QDF_STATUS_E_FAILURE;
}
#endif /* THERMAL_STATS_SUPPORT */