
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

#include <osdep.h>
#include <wmi.h>
#include <wmi_unified_priv.h>
#include <wmi_unified_roam_param.h>
#include <wmi_unified_roam_api.h>

#ifdef FEATURE_LFR_SUBNET_DETECTION
QDF_STATUS wmi_unified_set_gateway_params_cmd(void *wmi_hdl,
					struct gateway_update_req_param *req)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_gateway_params_cmd)
		return wmi_handle->ops->send_set_gateway_params_cmd(wmi_handle,
								    req);

	return QDF_STATUS_E_FAILURE;
}
#endif /* FEATURE_LFR_SUBNET_DETECTION */

#ifdef FEATURE_RSSI_MONITOR
QDF_STATUS wmi_unified_set_rssi_monitoring_cmd(void *wmi_hdl,
					struct rssi_monitor_param *req)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_rssi_monitoring_cmd)
		return wmi_handle->ops->send_set_rssi_monitoring_cmd(wmi_handle,
								     req);

	return QDF_STATUS_E_FAILURE;
}
#endif /* FEATURE_RSSI_MONITOR */

QDF_STATUS wmi_unified_roam_scan_offload_rssi_thresh_cmd(void *wmi_hdl,
					struct roam_offload_scan_rssi_params
					*roam_req)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_roam_scan_offload_rssi_thresh_cmd)
		return wmi_handle->ops->send_roam_scan_offload_rssi_thresh_cmd(
				wmi_handle, roam_req);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_roam_mawc_params_cmd(
			void *wmi_hdl, struct wmi_mawc_roam_params *params)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_roam_mawc_params_cmd)
		return wmi_handle->ops->send_roam_mawc_params_cmd(wmi_handle,
								  params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_roam_scan_filter_cmd(void *wmi_hdl,
				struct roam_scan_filter_params *roam_req)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_roam_scan_filter_cmd)
		return wmi_handle->ops->send_roam_scan_filter_cmd(wmi_handle,
								  roam_req);

	return QDF_STATUS_E_FAILURE;
}

#ifdef FEATURE_WLAN_ESE
QDF_STATUS wmi_unified_plm_stop_cmd(void *wmi_hdl,
			  const struct plm_req_params *plm)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_plm_stop_cmd)
		return wmi_handle->ops->send_plm_stop_cmd(wmi_handle, plm);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_plm_start_cmd(void *wmi_hdl,
			  const struct plm_req_params *plm,
			  uint32_t *gchannel_list)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_plm_start_cmd)
		return wmi_handle->ops->send_plm_start_cmd(wmi_handle,
							   plm,
							   gchannel_list);

	return QDF_STATUS_E_FAILURE;
}
#endif /* FEATURE_WLAN_ESE */

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
QDF_STATUS wmi_unified_set_ric_req_cmd(void *wmi_hdl, void *msg,
		uint8_t is_add_ts)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_ric_req_cmd)
		return wmi_handle->ops->send_set_ric_req_cmd(wmi_handle, msg,
							     is_add_ts);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_roam_synch_complete_cmd(void *wmi_hdl,
		 uint8_t vdev_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_process_roam_synch_complete_cmd)
		return wmi_handle->ops->send_process_roam_synch_complete_cmd(
				wmi_handle, vdev_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_roam_invoke_cmd(void *wmi_hdl,
				       struct wmi_roam_invoke_cmd *roaminvoke,
				       uint32_t ch_hz)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_roam_invoke_cmd)
		return wmi_handle->ops->send_roam_invoke_cmd(wmi_handle,
							     roaminvoke,
							     ch_hz);

	return QDF_STATUS_E_FAILURE;
}
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */

QDF_STATUS wmi_unified_roam_scan_offload_mode_cmd(void *wmi_hdl,
				wmi_start_scan_cmd_fixed_param *scan_cmd_fp,
				struct roam_offload_scan_params *roam_req)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_roam_scan_offload_mode_cmd)
		return wmi_handle->ops->send_roam_scan_offload_mode_cmd(
				wmi_handle, scan_cmd_fp, roam_req);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_send_roam_scan_offload_ap_cmd(void *wmi_hdl,
					   struct ap_profile_params *ap_profile)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_roam_scan_offload_ap_profile_cmd)
		return wmi_handle->ops->send_roam_scan_offload_ap_profile_cmd(
				  wmi_handle, ap_profile);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_roam_scan_offload_cmd(void *wmi_hdl,
					 uint32_t command, uint32_t vdev_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_roam_scan_offload_cmd)
		return wmi_handle->ops->send_roam_scan_offload_cmd(wmi_handle,
								   command,
								   vdev_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_roam_scan_offload_scan_period(void *wmi_hdl,
					     uint32_t scan_period,
					     uint32_t scan_age,
					     uint32_t vdev_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_roam_scan_offload_scan_period_cmd)
		return wmi_handle->ops->send_roam_scan_offload_scan_period_cmd(wmi_handle,
				  scan_period, scan_age, vdev_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_roam_scan_offload_chan_list_cmd(void *wmi_hdl,
				   uint8_t chan_count,
				   uint32_t *chan_list,
				   uint8_t list_type, uint32_t vdev_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_roam_scan_offload_chan_list_cmd)
		return wmi_handle->ops->send_roam_scan_offload_chan_list_cmd(wmi_handle,
				  chan_count, chan_list,
				  list_type, vdev_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_roam_scan_offload_rssi_change_cmd(void *wmi_hdl,
	uint32_t vdev_id,
	int32_t rssi_change_thresh,
	uint32_t bcn_rssi_weight,
	uint32_t hirssi_delay_btw_scans)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_roam_scan_offload_rssi_change_cmd)
		return wmi_handle->ops->send_roam_scan_offload_rssi_change_cmd(wmi_handle,
				  vdev_id, rssi_change_thresh,
				  bcn_rssi_weight, hirssi_delay_btw_scans);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_set_per_roam_config(void *wmi_hdl,
		struct wmi_per_roam_config_req *req_buf)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_per_roam_config_cmd)
		return wmi_handle->ops->send_per_roam_config_cmd(wmi_handle,
								 req_buf);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_send_limit_off_chan_cmd(void *wmi_hdl,
		struct wmi_limit_off_chan_param *limit_off_chan_param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_limit_off_chan_cmd)
		return wmi_handle->ops->send_limit_off_chan_cmd(wmi_handle,
				limit_off_chan_param);

	return QDF_STATUS_E_FAILURE;
}

#ifdef WLAN_FEATURE_FILS_SK
QDF_STATUS wmi_unified_roam_send_hlp_cmd(void *wmi_hdl,
					 struct hlp_params *req_buf)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_roam_scan_hlp_cmd)
		return wmi_handle->ops->send_roam_scan_hlp_cmd(wmi_handle,
							       req_buf);

	return QDF_STATUS_E_FAILURE;
}
#endif /* WLAN_FEATURE_FILS_SK */

QDF_STATUS wmi_unified_send_btm_config(void *wmi_hdl,
				       struct wmi_btm_config *params)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_btm_config)
		return wmi_handle->ops->send_btm_config(wmi_handle,
							params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_send_bss_load_config(void *wmi_hdl,
					    struct wmi_bss_load_config *params)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->send_roam_bss_load_config)
		return wmi_handle->ops->send_roam_bss_load_config(wmi_handle,
								  params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_send_disconnect_roam_params(wmi_unified_t wmi_handle,
					struct wmi_disconnect_roam_params *req)
{
	if (wmi_handle->ops->send_disconnect_roam_params)
		return wmi_handle->ops->send_disconnect_roam_params(wmi_handle,
								    req);
	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_send_idle_roam_params(wmi_unified_t wmi_handle,
				  struct wmi_idle_roam_params *req)
{
	if (wmi_handle->ops->send_idle_roam_params)
		return wmi_handle->ops->send_idle_roam_params(wmi_handle,
							      req);
	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_offload_11k_cmd(void *wmi_hdl,
				struct wmi_11k_offload_params *params)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_offload_11k_cmd)
		return wmi_handle->ops->send_offload_11k_cmd(wmi_handle,
							     params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_invoke_neighbor_report_cmd(void *wmi_hdl,
			struct wmi_invoke_neighbor_report_params *params)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_invoke_neighbor_report_cmd)
		return wmi_handle->ops->send_invoke_neighbor_report_cmd(
				wmi_handle, params);

	return QDF_STATUS_E_FAILURE;
}

