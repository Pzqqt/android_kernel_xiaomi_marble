/*
 * Copyright (c) 2013-2020 The Linux Foundation. All rights reserved.
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
 * DOC: Implement API's specific to STA component.
 */
#include <wmi.h>
#include "wmi_unified_priv.h"
#include "wmi_unified_sta_api.h"

QDF_STATUS wmi_unified_set_sta_sa_query_param_cmd(wmi_unified_t wmi_handle,
						  uint8_t vdev_id,
						  uint32_t max_retries,
						  uint32_t retry_interval)
{
	if (wmi_handle->ops->send_set_sta_sa_query_param_cmd)
		return wmi_handle->ops->send_set_sta_sa_query_param_cmd(wmi_handle,
						vdev_id, max_retries,
						retry_interval);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_set_sta_keep_alive_cmd(wmi_unified_t wmi_handle,
				   struct sta_keep_alive_params *params)
{
	if (wmi_handle->ops->send_set_sta_keep_alive_cmd)
		return wmi_handle->ops->send_set_sta_keep_alive_cmd(wmi_handle,
								    params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_vdev_set_gtx_cfg_cmd(wmi_unified_t wmi_handle, uint32_t if_id,
				 struct wmi_gtx_config *gtx_info)
{
	if (wmi_handle->ops->send_vdev_set_gtx_cfg_cmd)
		return wmi_handle->ops->send_vdev_set_gtx_cfg_cmd(wmi_handle,
								  if_id,
								  gtx_info);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_process_dhcp_ind(
			wmi_unified_t wmi_handle,
			wmi_peer_set_param_cmd_fixed_param *ta_dhcp_ind)
{
	if (wmi_handle->ops->send_process_dhcp_ind_cmd)
		return wmi_handle->ops->send_process_dhcp_ind_cmd(wmi_handle,
								  ta_dhcp_ind);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_get_link_speed_cmd(wmi_unified_t wmi_handle,
					  wmi_mac_addr peer_macaddr)
{
	if (wmi_handle->ops->send_get_link_speed_cmd)
		return wmi_handle->ops->send_get_link_speed_cmd(wmi_handle,
								peer_macaddr);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_fw_profiling_data_cmd(wmi_unified_t wmi_handle,
					     uint32_t cmd,
					     uint32_t value1,
					     uint32_t value2)
{
	if (wmi_handle->ops->send_fw_profiling_cmd)
		return wmi_handle->ops->send_fw_profiling_cmd(wmi_handle,
							      cmd,
							      value1,
							      value2);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_nat_keepalive_en_cmd(wmi_unified_t wmi_handle,
					    uint8_t vdev_id)
{
	if (wmi_handle->ops->send_nat_keepalive_en_cmd)
		return wmi_handle->ops->send_nat_keepalive_en_cmd(wmi_handle,
								  vdev_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_wlm_latency_level_cmd(wmi_unified_t wmi_handle,
				  struct wlm_latency_level_param *param)
{
	if (wmi_handle->ops->send_wlm_latency_level_cmd)
		return wmi_handle->ops->send_wlm_latency_level_cmd(wmi_handle,
								   param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_process_set_ie_info_cmd(wmi_unified_t wmi_handle,
				    struct vdev_ie_info_param *ie_info)
{
	if (wmi_handle->ops->send_process_set_ie_info_cmd)
		return wmi_handle->ops->send_process_set_ie_info_cmd(wmi_handle,
								     ie_info);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_set_base_macaddr_indicate_cmd(wmi_unified_t wmi_handle,
						     uint8_t *custom_addr)
{
	if (wmi_handle->ops->send_set_base_macaddr_indicate_cmd)
		return wmi_handle->ops->send_set_base_macaddr_indicate_cmd(
						wmi_handle, custom_addr);

	return QDF_STATUS_E_FAILURE;
}

#ifdef FEATURE_WLAN_TDLS
QDF_STATUS wmi_unified_set_tdls_offchan_mode_cmd(
			wmi_unified_t wmi_handle,
			struct tdls_channel_switch_params *chan_switch_params)
{
	if (wmi_handle->ops->send_set_tdls_offchan_mode_cmd)
		return wmi_handle->ops->send_set_tdls_offchan_mode_cmd(
						wmi_handle, chan_switch_params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_update_fw_tdls_state_cmd(wmi_unified_t wmi_handle,
						struct tdls_info *tdls_param,
						enum wmi_tdls_state tdls_state)
{
	if (wmi_handle->ops->send_update_fw_tdls_state_cmd)
		return wmi_handle->ops->send_update_fw_tdls_state_cmd(
				wmi_handle, tdls_param, tdls_state);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_update_tdls_peer_state_cmd(wmi_unified_t wmi_handle,
				struct tdls_peer_update_state *peer_state,
				uint32_t *ch_mhz)
{
	if (wmi_handle->ops->send_update_tdls_peer_state_cmd)
		return wmi_handle->ops->send_update_tdls_peer_state_cmd(wmi_handle,
			    peer_state, ch_mhz);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_vdev_tdls_ev_param(wmi_unified_t wmi_handle,
					  void *evt_buf,
					  struct tdls_event_info *param)
{
	if (wmi_handle->ops->extract_vdev_tdls_ev_param)
		return wmi_handle->ops->extract_vdev_tdls_ev_param(wmi_handle,
				evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}
#endif /* FEATURE_WLAN_TDLS */

#if defined(WLAN_FEATURE_ROAM_OFFLOAD) && defined(FEATURE_BLACKLIST_MGR)
QDF_STATUS
wmi_unified_send_reject_ap_list(struct wmi_unified *wmi_handle,
				struct reject_ap_params *reject_params)
{
	if (wmi_handle->ops->send_reject_ap_list_cmd)
		return wmi_handle->ops->send_reject_ap_list_cmd(wmi_handle,
								reject_params);

	return QDF_STATUS_E_FAILURE;
}
#endif

QDF_STATUS wmi_unified_send_sar_limit_cmd(wmi_unified_t wmi_handle,
					  struct sar_limit_cmd_params *params)
{
	if (wmi_handle->ops->send_sar_limit_cmd)
		return wmi_handle->ops->send_sar_limit_cmd(
						wmi_handle,
						params);
	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_get_sar_limit_cmd(wmi_unified_t wmi_handle)
{
	if (wmi_handle->ops->get_sar_limit_cmd)
		return wmi_handle->ops->get_sar_limit_cmd(wmi_handle);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_extract_sar_limit_event(wmi_unified_t wmi_handle,
					       uint8_t *evt_buf,
					       struct sar_limit_event *event)
{
	if (wmi_handle->ops->extract_sar_limit_event)
		return wmi_handle->ops->extract_sar_limit_event(wmi_handle,
								evt_buf,
								event);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_extract_sar2_result_event(void *handle,
						 uint8_t *event, uint32_t len)
{
	wmi_unified_t wmi_handle = handle;

	if (wmi_handle->ops->extract_sar2_result_event)
		return wmi_handle->ops->extract_sar2_result_event(wmi_handle,
								  event,
								  len);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_set_del_pmkid_cache(wmi_unified_t wmi_handle,
				struct wmi_unified_pmk_cache *req_buf)
{
	if (wmi_handle->ops->send_set_del_pmkid_cache_cmd)
		return wmi_handle->ops->send_set_del_pmkid_cache_cmd(wmi_handle,
								     req_buf);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_del_ts_cmd(wmi_unified_t wmi_handle, uint8_t vdev_id,
				  uint8_t ac)
{
	if (wmi_handle->ops->send_del_ts_cmd)
		return wmi_handle->ops->send_del_ts_cmd(wmi_handle,
							vdev_id, ac);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_aggr_qos_cmd(
		wmi_unified_t wmi_handle,
		struct aggr_add_ts_param *aggr_qos_rsp_msg)
{
	if (wmi_handle->ops->send_aggr_qos_cmd)
		return wmi_handle->ops->send_aggr_qos_cmd(wmi_handle,
							  aggr_qos_rsp_msg);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_add_ts_cmd(wmi_unified_t wmi_handle,
				  struct add_ts_param *msg)
{
	if (wmi_handle->ops->send_add_ts_cmd)
		return wmi_handle->ops->send_add_ts_cmd(wmi_handle,
							msg);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_process_add_periodic_tx_ptrn_cmd(
					wmi_unified_t wmi_handle,
					struct periodic_tx_pattern *pattern,
					uint8_t vdev_id)
{
	if (wmi_handle->ops->send_process_add_periodic_tx_ptrn_cmd)
		return wmi_handle->ops->send_process_add_periodic_tx_ptrn_cmd(
						wmi_handle, pattern, vdev_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_process_del_periodic_tx_ptrn_cmd(wmi_unified_t wmi_handle,
					     uint8_t vdev_id,
					     uint8_t pattern_id)
{
	if (wmi_handle->ops->send_process_del_periodic_tx_ptrn_cmd)
		return wmi_handle->ops->send_process_del_periodic_tx_ptrn_cmd(
				wmi_handle,
				vdev_id,
				pattern_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_set_auto_shutdown_timer_cmd(wmi_unified_t wmi_handle,
						   uint32_t timer_val)
{
	if (wmi_handle->ops->send_set_auto_shutdown_timer_cmd)
		return wmi_handle->ops->send_set_auto_shutdown_timer_cmd(
				wmi_handle,
				timer_val);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_set_led_flashing_cmd(wmi_unified_t wmi_handle,
				 struct flashing_req_params *flashing)
{
	if (wmi_handle->ops->send_set_led_flashing_cmd)
		return wmi_handle->ops->send_set_led_flashing_cmd(wmi_handle,
								  flashing);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_process_ch_avoid_update_cmd(wmi_unified_t wmi_handle)
{
	if (wmi_handle->ops->send_process_ch_avoid_update_cmd)
		return wmi_handle->ops->send_process_ch_avoid_update_cmd(
				wmi_handle);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_pdev_set_pcl_cmd(wmi_unified_t wmi_handle,
					struct wmi_pcl_chan_weights *msg)
{
	if (wmi_handle->ops->send_pdev_set_pcl_cmd)
		return wmi_handle->ops->send_pdev_set_pcl_cmd(wmi_handle, msg);

	return QDF_STATUS_E_FAILURE;
}

#ifdef WLAN_POLICY_MGR_ENABLE
QDF_STATUS wmi_unified_pdev_set_dual_mac_config_cmd(
		wmi_unified_t wmi_handle,
		struct policy_mgr_dual_mac_config *msg)
{
	if (wmi_handle->ops->send_pdev_set_dual_mac_config_cmd)
		return wmi_handle->ops->send_pdev_set_dual_mac_config_cmd(
				wmi_handle,
				msg);

	return QDF_STATUS_E_FAILURE;
}
#endif /* WLAN_POLICY_MGR_ENABLE */

QDF_STATUS wmi_unified_send_adapt_dwelltime_params_cmd(
			wmi_unified_t wmi_handle,
			struct wmi_adaptive_dwelltime_params *dwelltime_params)
{
	if (wmi_handle->ops->send_adapt_dwelltime_params_cmd)
		return wmi_handle->ops->
			send_adapt_dwelltime_params_cmd(wmi_handle,
				  dwelltime_params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_send_dbs_scan_sel_params_cmd(
			wmi_unified_t wmi_handle,
			struct wmi_dbs_scan_sel_params *dbs_scan_params)
{
	if (wmi_handle->ops->send_dbs_scan_sel_params_cmd)
		return wmi_handle->ops->
			send_dbs_scan_sel_params_cmd(wmi_handle,
						     dbs_scan_params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_set_arp_stats_req(wmi_unified_t wmi_handle,
					 struct set_arp_stats *req_buf)
{
	if (wmi_handle->ops->send_set_arp_stats_req_cmd)
		return wmi_handle->ops->send_set_arp_stats_req_cmd(wmi_handle,
								   req_buf);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_get_arp_stats_req(wmi_unified_t wmi_handle,
					 struct get_arp_stats *req_buf)
{
	if (wmi_handle->ops->send_get_arp_stats_req_cmd)
		return wmi_handle->ops->send_get_arp_stats_req_cmd(wmi_handle,
								   req_buf);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_peer_unmap_conf_send(wmi_unified_t wmi_handle,
					    uint8_t vdev_id,
					    uint32_t peer_id_cnt,
					    uint16_t *peer_id_list)
{
	if (wmi_handle->ops->send_peer_unmap_conf_cmd)
		return wmi_handle->ops->send_peer_unmap_conf_cmd(wmi_handle,
				  vdev_id, peer_id_cnt, peer_id_list);

	return QDF_STATUS_E_FAILURE;
}

