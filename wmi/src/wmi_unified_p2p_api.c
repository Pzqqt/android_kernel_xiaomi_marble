/*
 * Copyright (c) 2013-2019 The Linux Foundation. All rights reserved.
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
 * DOC: Implement API's specific to P2P component.
 */

#include <wmi_unified_priv.h>
#include <wmi_unified_p2p_api.h>

QDF_STATUS wmi_unified_set_p2pgo_oppps_req(wmi_unified_t wmi_handle,
					   struct p2p_ps_params *oppps)
{
	if (wmi_handle->ops->send_set_p2pgo_oppps_req_cmd)
		return wmi_handle->ops->send_set_p2pgo_oppps_req_cmd(wmi_handle,
								     oppps);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_set_p2pgo_noa_req_cmd(wmi_unified_t wmi_handle,
					     struct p2p_ps_params *noa)
{
	if (wmi_handle->ops->send_set_p2pgo_noa_req_cmd)
		return wmi_handle->ops->send_set_p2pgo_noa_req_cmd(wmi_handle,
								   noa);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_p2p_noa_ev_param(wmi_unified_t wmi_handle,
					void *evt_buf,
					struct p2p_noa_info *param)
{
	if (!wmi_handle) {
		WMI_LOGE("wmi handle is null");
		return QDF_STATUS_E_INVAL;
	}

	if (wmi_handle->ops->extract_p2p_noa_ev_param)
		return wmi_handle->ops->extract_p2p_noa_ev_param(
				wmi_handle, evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_send_set_mac_addr_rx_filter_cmd(wmi_unified_t wmi_handle,
				    struct p2p_set_mac_filter *param)
{
	if (!wmi_handle) {
		WMI_LOGE("wmi handle is null");
		return QDF_STATUS_E_INVAL;
	}

	if (wmi_handle->ops->set_mac_addr_rx_filter)
		return wmi_handle->ops->set_mac_addr_rx_filter(
				wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_mac_addr_rx_filter_evt_param(wmi_unified_t wmi_handle,
					 void *evt_buf,
					 struct p2p_set_mac_filter_evt *param)
{
	if (!wmi_handle) {
		WMI_LOGE("wmi handle is null");
		return QDF_STATUS_E_INVAL;
	}

	if (wmi_handle->ops->extract_mac_addr_rx_filter_evt_param)
		return wmi_handle->ops->extract_mac_addr_rx_filter_evt_param(
				wmi_handle, evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}

#ifdef FEATURE_P2P_LISTEN_OFFLOAD
QDF_STATUS wmi_unified_p2p_lo_start_cmd(wmi_unified_t wmi_handle,
					struct p2p_lo_start *param)
{
	if (!wmi_handle) {
		WMI_LOGE("wmi handle is null");
		return QDF_STATUS_E_INVAL;
	}

	if (wmi_handle->ops->send_p2p_lo_start_cmd)
		return wmi_handle->ops->send_p2p_lo_start_cmd(wmi_handle,
							      param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_p2p_lo_stop_cmd(wmi_unified_t wmi_handle,
				       uint8_t vdev_id)
{
	if (!wmi_handle) {
		WMI_LOGE("wmi handle is null");
		return QDF_STATUS_E_INVAL;
	}

	if (wmi_handle->ops->send_p2p_lo_stop_cmd)
		return wmi_handle->ops->send_p2p_lo_stop_cmd(wmi_handle,
							     vdev_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_p2p_lo_stop_ev_param(wmi_unified_t wmi_handle,
					    void *evt_buf,
					    struct p2p_lo_event *param)
{
	if (!wmi_handle) {
		WMI_LOGE("wmi handle is null");
		return QDF_STATUS_E_INVAL;
	}

	if (wmi_handle->ops->extract_p2p_lo_stop_ev_param)
		return wmi_handle->ops->extract_p2p_lo_stop_ev_param(
				wmi_handle, evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}
#endif /* End of FEATURE_P2P_LISTEN_OFFLOAD*/

