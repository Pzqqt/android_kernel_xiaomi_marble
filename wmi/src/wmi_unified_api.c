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

#include "wmi_unified_priv.h"
#include "wmi_unified_param.h"
#include "qdf_module.h"
#include "cdp_txrx_cmn_struct.h"
#include <wmi_unified_vdev_api.h>

static const wmi_host_channel_width mode_to_width[WMI_HOST_MODE_MAX] = {
	[WMI_HOST_MODE_11A]           = WMI_HOST_CHAN_WIDTH_20,
	[WMI_HOST_MODE_11G]           = WMI_HOST_CHAN_WIDTH_20,
	[WMI_HOST_MODE_11B]           = WMI_HOST_CHAN_WIDTH_20,
	[WMI_HOST_MODE_11GONLY]       = WMI_HOST_CHAN_WIDTH_20,
	[WMI_HOST_MODE_11NA_HT20]     = WMI_HOST_CHAN_WIDTH_20,
	[WMI_HOST_MODE_11NG_HT20]     = WMI_HOST_CHAN_WIDTH_20,
	[WMI_HOST_MODE_11AC_VHT20]    = WMI_HOST_CHAN_WIDTH_20,
	[WMI_HOST_MODE_11AC_VHT20_2G] = WMI_HOST_CHAN_WIDTH_20,
	[WMI_HOST_MODE_11NA_HT40]     = WMI_HOST_CHAN_WIDTH_40,
	[WMI_HOST_MODE_11NG_HT40]     = WMI_HOST_CHAN_WIDTH_40,
	[WMI_HOST_MODE_11AC_VHT40]    = WMI_HOST_CHAN_WIDTH_40,
	[WMI_HOST_MODE_11AC_VHT40_2G] = WMI_HOST_CHAN_WIDTH_40,
	[WMI_HOST_MODE_11AC_VHT80]    = WMI_HOST_CHAN_WIDTH_80,
	[WMI_HOST_MODE_11AC_VHT80_2G] = WMI_HOST_CHAN_WIDTH_80,
#if CONFIG_160MHZ_SUPPORT
	[WMI_HOST_MODE_11AC_VHT80_80] = WMI_HOST_CHAN_WIDTH_80P80,
	[WMI_HOST_MODE_11AC_VHT160]   = WMI_HOST_CHAN_WIDTH_160,
#endif

#if SUPPORT_11AX
	[WMI_HOST_MODE_11AX_HE20]     = WMI_HOST_CHAN_WIDTH_20,
	[WMI_HOST_MODE_11AX_HE40]     = WMI_HOST_CHAN_WIDTH_40,
	[WMI_HOST_MODE_11AX_HE80]     = WMI_HOST_CHAN_WIDTH_80,
	[WMI_HOST_MODE_11AX_HE80_80]  = WMI_HOST_CHAN_WIDTH_80P80,
	[WMI_HOST_MODE_11AX_HE160]    = WMI_HOST_CHAN_WIDTH_160,
	[WMI_HOST_MODE_11AX_HE20_2G]  = WMI_HOST_CHAN_WIDTH_20,
	[WMI_HOST_MODE_11AX_HE40_2G]  = WMI_HOST_CHAN_WIDTH_40,
	[WMI_HOST_MODE_11AX_HE80_2G]  = WMI_HOST_CHAN_WIDTH_80,
#endif
};

QDF_STATUS wmi_unified_soc_set_hw_mode_cmd(wmi_unified_t wmi_handle,
					   uint32_t hw_mode_index)
{
	if (wmi_handle->ops->send_pdev_set_hw_mode_cmd)
		return wmi_handle->ops->send_pdev_set_hw_mode_cmd(
								wmi_handle,
								hw_mode_index);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_vdev_create_send(wmi_unified_t wmi_handle,
					uint8_t macaddr[QDF_MAC_ADDR_SIZE],
					struct vdev_create_params *param)
{
	if (wmi_handle->ops->send_vdev_create_cmd)
		return wmi_handle->ops->send_vdev_create_cmd(wmi_handle,
			   macaddr, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_vdev_delete_send(wmi_unified_t wmi_handle,
					uint8_t if_id)
{
	if (wmi_handle->ops->send_vdev_delete_cmd)
		return wmi_handle->ops->send_vdev_delete_cmd(wmi_handle,
			   if_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_vdev_nss_chain_params_send(wmi_unified_t wmi_handle,
				       uint8_t vdev_id,
				       struct vdev_nss_chains *user_cfg)
{
	if (wmi_handle->ops->send_vdev_nss_chain_params_cmd)
		return wmi_handle->ops->send_vdev_nss_chain_params_cmd(
							wmi_handle,
							vdev_id,
							user_cfg);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_vdev_stop_send(wmi_unified_t  wmi_handle,
				      uint8_t vdev_id)
{
	if (wmi_handle->ops->send_vdev_stop_cmd)
		return wmi_handle->ops->send_vdev_stop_cmd(wmi_handle,
			   vdev_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_vdev_down_send(wmi_unified_t wmi_handle,
				      uint8_t vdev_id)
{
	if (wmi_handle->ops->send_vdev_down_cmd)
		return wmi_handle->ops->send_vdev_down_cmd(wmi_handle, vdev_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_vdev_start_send(wmi_unified_t wmi_handle,
				       struct vdev_start_params *req)
{
	if (wmi_handle->ops->send_vdev_start_cmd)
		return wmi_handle->ops->send_vdev_start_cmd(wmi_handle, req);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_vdev_set_nac_rssi_send(wmi_unified_t wmi_handle,
				   struct vdev_scan_nac_rssi_params *req)
{
	if (wmi_handle->ops->send_vdev_set_nac_rssi_cmd)
		return wmi_handle->ops->send_vdev_set_nac_rssi_cmd(wmi_handle, req);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_peer_flush_tids_send(wmi_unified_t wmi_handle,
				 uint8_t peer_addr[QDF_MAC_ADDR_SIZE],
				 struct peer_flush_params *param)
{
	if (wmi_handle->ops->send_peer_flush_tids_cmd)
		return wmi_handle->ops->send_peer_flush_tids_cmd(wmi_handle,
				  peer_addr, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_peer_delete_send(wmi_unified_t wmi_handle,
					uint8_t peer_addr[QDF_MAC_ADDR_SIZE],
					uint8_t vdev_id)
{
	if (wmi_handle->ops->send_peer_delete_cmd)
		return wmi_handle->ops->send_peer_delete_cmd(wmi_handle,
				  peer_addr, vdev_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_peer_delete_all_send(
				wmi_unified_t wmi_hdl,
				struct peer_delete_all_params *param)
{
	if (wmi_hdl->ops->send_peer_delete_all_cmd)
		return wmi_hdl->ops->send_peer_delete_all_cmd(wmi_hdl, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_set_peer_param_send(wmi_unified_t wmi_handle,
				   uint8_t peer_addr[QDF_MAC_ADDR_SIZE],
				   struct peer_set_params *param)
{
	if (wmi_handle->ops->send_peer_param_cmd)
		return wmi_handle->ops->send_peer_param_cmd(wmi_handle,
				peer_addr, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_vdev_up_send(wmi_unified_t wmi_handle,
				    uint8_t bssid[QDF_MAC_ADDR_SIZE],
				    struct vdev_up_params *params)
{
	if (wmi_handle->ops->send_vdev_up_cmd)
		return wmi_handle->ops->send_vdev_up_cmd(wmi_handle, bssid,
					params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_peer_create_send(wmi_unified_t wmi_handle,
					struct peer_create_params *param)
{
	if (wmi_handle->ops->send_peer_create_cmd)
		return wmi_handle->ops->send_peer_create_cmd(wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_peer_rx_reorder_queue_setup_send(
		wmi_unified_t wmi_handle,
		struct rx_reorder_queue_setup_params *param)
{
	if (wmi_handle->ops->send_peer_rx_reorder_queue_setup_cmd)
		return wmi_handle->ops->send_peer_rx_reorder_queue_setup_cmd(
			wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_peer_rx_reorder_queue_remove_send(
		wmi_unified_t wmi_handle,
		struct rx_reorder_queue_remove_params *param)
{
	if (wmi_handle->ops->send_peer_rx_reorder_queue_remove_cmd)
		return wmi_handle->ops->send_peer_rx_reorder_queue_remove_cmd(
			wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

#ifdef WLAN_SUPPORT_GREEN_AP
QDF_STATUS wmi_unified_green_ap_ps_send(wmi_unified_t wmi_handle,
					uint32_t value, uint8_t pdev_id)
{
	if (wmi_handle->ops->send_green_ap_ps_cmd)
		return wmi_handle->ops->send_green_ap_ps_cmd(wmi_handle, value,
				  pdev_id);

	return QDF_STATUS_E_FAILURE;
}
#else
QDF_STATUS wmi_unified_green_ap_ps_send(wmi_unified_t wmi_handle,
					uint32_t value, uint8_t pdev_id)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_SUPPORT_GREEN_AP */

QDF_STATUS
wmi_unified_pdev_utf_cmd_send(wmi_unified_t wmi_handle,
			      struct pdev_utf_params *param,
			      uint8_t mac_id)
{
	if (wmi_handle->ops->send_pdev_utf_cmd)
		return wmi_handle->ops->send_pdev_utf_cmd(wmi_handle, param,
				  mac_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_pdev_param_send(wmi_unified_t wmi_handle,
			    struct pdev_params *param,
			    uint8_t mac_id)
{
	if (wmi_handle->ops->send_pdev_param_cmd)
		return wmi_handle->ops->send_pdev_param_cmd(wmi_handle, param,
				  mac_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_suspend_send(wmi_unified_t wmi_handle,
				    struct suspend_params *param,
				    uint8_t mac_id)
{
	if (wmi_handle->ops->send_suspend_cmd)
		return wmi_handle->ops->send_suspend_cmd(wmi_handle, param,
				  mac_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_resume_send(wmi_unified_t wmi_handle,
				   uint8_t mac_id)
{
	if (wmi_handle->ops->send_resume_cmd)
		return wmi_handle->ops->send_resume_cmd(wmi_handle,
				  mac_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_wow_enable_send(wmi_unified_t wmi_handle,
				       struct wow_cmd_params *param,
				       uint8_t mac_id)
{
	if (wmi_handle->ops->send_wow_enable_cmd)
		return wmi_handle->ops->send_wow_enable_cmd(wmi_handle, param,
				  mac_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_wow_wakeup_send(wmi_unified_t wmi_handle)
{
	if (wmi_handle->ops->send_wow_wakeup_cmd)
		return wmi_handle->ops->send_wow_wakeup_cmd(wmi_handle);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_wow_add_wakeup_event_send(wmi_unified_t wmi_handle,
				      struct wow_add_wakeup_params *param)
{
	if (wmi_handle->ops->send_wow_add_wakeup_event_cmd)
		return wmi_handle->ops->send_wow_add_wakeup_event_cmd(
				wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_wow_add_wakeup_pattern_send(
		wmi_unified_t wmi_handle,
		struct wow_add_wakeup_pattern_params *param)
{
	if (wmi_handle->ops->send_wow_add_wakeup_pattern_cmd)
		return wmi_handle->ops->send_wow_add_wakeup_pattern_cmd(
				wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_wow_remove_wakeup_pattern_send(
			wmi_unified_t wmi_handle,
			struct wow_remove_wakeup_pattern_params *param)
{
	if (wmi_handle->ops->send_wow_remove_wakeup_pattern_cmd)
		return wmi_handle->ops->send_wow_remove_wakeup_pattern_cmd(
				wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_ap_ps_cmd_send(wmi_unified_t wmi_handle,
				      uint8_t *peer_addr,
				      struct ap_ps_params *param)
{
	if (wmi_handle->ops->send_set_ap_ps_param_cmd)
		return wmi_handle->ops->send_set_ap_ps_param_cmd(wmi_handle,
				  peer_addr,
				  param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_sta_ps_cmd_send(wmi_unified_t wmi_handle,
				       struct sta_ps_params *param)
{
	if (wmi_handle->ops->send_set_sta_ps_param_cmd)
		return wmi_handle->ops->send_set_sta_ps_param_cmd(wmi_handle,
				  param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_crash_inject(wmi_unified_t wmi_handle,
			    struct crash_inject *param)
{
	if (wmi_handle->ops->send_crash_inject_cmd)
		return wmi_handle->ops->send_crash_inject_cmd(wmi_handle,
				  param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_dbglog_cmd_send(wmi_unified_t wmi_handle,
			    struct dbglog_params *dbglog_param)
{
	if (wmi_handle->ops->send_dbglog_cmd)
		return wmi_handle->ops->send_dbglog_cmd(wmi_handle,
				  dbglog_param);

	return QDF_STATUS_E_FAILURE;
}
qdf_export_symbol(wmi_unified_dbglog_cmd_send);

QDF_STATUS
wmi_unified_vdev_set_param_send(wmi_unified_t wmi_handle,
				struct vdev_set_params *param)
{
	if (wmi_handle->ops->send_vdev_set_param_cmd)
		return wmi_handle->ops->send_vdev_set_param_cmd(wmi_handle,
				  param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_sifs_trigger_send(wmi_unified_t wmi_handle,
					 struct sifs_trigger_param *param)
{
	if (wmi_handle->ops->send_vdev_sifs_trigger_cmd)
		return wmi_handle->ops->send_vdev_sifs_trigger_cmd(wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_packet_log_enable_send(wmi_unified_t wmi_handle,
				   WMI_HOST_PKTLOG_EVENT PKTLOG_EVENT,
				   uint8_t mac_id)
{
	if (wmi_handle->ops->send_packet_log_enable_cmd)
		return wmi_handle->ops->send_packet_log_enable_cmd(wmi_handle,
				  PKTLOG_EVENT, mac_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_peer_based_pktlog_send(wmi_unified_t wmi_handle,
					      uint8_t *macaddr,
					      uint8_t mac_id,
					      uint8_t enb_dsb)
{
	if (wmi_handle->ops->send_peer_based_pktlog_cmd)
		return wmi_handle->ops->send_peer_based_pktlog_cmd
			(wmi_handle, macaddr, mac_id, enb_dsb);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_packet_log_disable_send(wmi_unified_t wmi_handle,
					       uint8_t mac_id)
{
	if (wmi_handle->ops->send_packet_log_disable_cmd)
		return wmi_handle->ops->send_packet_log_disable_cmd(wmi_handle,
			mac_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_fd_tmpl_send_cmd(wmi_unified_t wmi_handle,
			     struct fils_discovery_tmpl_params *param)
{
	if (wmi_handle->ops->send_fd_tmpl_cmd)
		return wmi_handle->ops->send_fd_tmpl_cmd(wmi_handle,
				  param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_beacon_tmpl_send_cmd(wmi_unified_t wmi_handle,
				 struct beacon_tmpl_params *param)
{
	if (wmi_handle->ops->send_beacon_tmpl_send_cmd)
		return wmi_handle->ops->send_beacon_tmpl_send_cmd(wmi_handle,
				  param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_peer_assoc_send(wmi_unified_t wmi_handle,
			    struct peer_assoc_params *param)
{
	if (wmi_handle->ops->send_peer_assoc_cmd)
		return wmi_handle->ops->send_peer_assoc_cmd(wmi_handle,
				  param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_scan_start_cmd_send(wmi_unified_t wmi_handle,
				struct scan_req_params *param)
{
	if (wmi_handle->ops->send_scan_start_cmd)
		return wmi_handle->ops->send_scan_start_cmd(wmi_handle,
				  param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_scan_stop_cmd_send(wmi_unified_t wmi_handle,
			       struct scan_cancel_param *param)
{
	if (wmi_handle->ops->send_scan_stop_cmd)
		return wmi_handle->ops->send_scan_stop_cmd(wmi_handle,
				  param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_scan_chan_list_cmd_send(wmi_unified_t wmi_handle,
				    struct scan_chan_list_params *param)
{
	if (wmi_handle->ops->send_scan_chan_list_cmd)
		return wmi_handle->ops->send_scan_chan_list_cmd(wmi_handle,
				  param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_mgmt_unified_cmd_send(wmi_unified_t wmi_handle,
				     struct wmi_mgmt_params *param)
{
	if (wmi_handle->ops->send_mgmt_cmd)
		return wmi_handle->ops->send_mgmt_cmd(wmi_handle,
				  param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_offchan_data_tx_cmd_send(wmi_unified_t wmi_handle,
			     struct wmi_offchan_data_tx_params *param)
{
	if (wmi_handle->ops->send_offchan_data_tx_cmd)
		return wmi_handle->ops->send_offchan_data_tx_cmd(wmi_handle,
				  param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_modem_power_state(wmi_unified_t wmi_handle,
					 uint32_t param_value)
{
	if (wmi_handle->ops->send_modem_power_state_cmd)
		return wmi_handle->ops->send_modem_power_state_cmd(wmi_handle,
				  param_value);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_set_sta_ps_mode(wmi_unified_t wmi_handle,
				       uint32_t vdev_id, uint8_t val)
{
	if (wmi_handle->ops->send_set_sta_ps_mode_cmd)
		return wmi_handle->ops->send_set_sta_ps_mode_cmd(wmi_handle,
				  vdev_id, val);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_send_idle_trigger_monitor(wmi_unified_t wmi_handle, uint8_t val)
{
	if (wmi_handle->ops->send_idle_roam_monitor_cmd)
		return wmi_handle->ops->send_idle_roam_monitor_cmd(wmi_handle,
								   val);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_set_mimops(wmi_unified_t wmi_handle, uint8_t vdev_id,
				  int value)
{
	if (wmi_handle->ops->send_set_mimops_cmd)
		return wmi_handle->ops->send_set_mimops_cmd(wmi_handle,
				  vdev_id, value);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_set_smps_params(wmi_unified_t wmi_handle,
				       uint8_t vdev_id,
				       int value)
{
	if (wmi_handle->ops->send_set_smps_params_cmd)
		return wmi_handle->ops->send_set_smps_params_cmd(wmi_handle,
				  vdev_id, value);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_get_temperature(wmi_unified_t wmi_handle)
{
	if (wmi_handle->ops->send_get_temperature_cmd)
		return wmi_handle->ops->send_get_temperature_cmd(wmi_handle);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_set_sta_uapsd_auto_trig_cmd(wmi_unified_t wmi_handle,
					struct sta_uapsd_trig_params *param)
{
	if (wmi_handle->ops->send_set_sta_uapsd_auto_trig_cmd)
		return wmi_handle->ops->send_set_sta_uapsd_auto_trig_cmd(wmi_handle,
					param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_set_thermal_mgmt_cmd(wmi_unified_t wmi_handle,
				 struct thermal_cmd_params *thermal_info)
{
	if (wmi_handle->ops->send_set_thermal_mgmt_cmd)
		return wmi_handle->ops->send_set_thermal_mgmt_cmd(wmi_handle,
					thermal_info);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_lro_config_cmd(wmi_unified_t wmi_handle,
			   struct wmi_lro_config_cmd_t *wmi_lro_cmd)
{
	if (wmi_handle->ops->send_lro_config_cmd)
		return wmi_handle->ops->send_lro_config_cmd(wmi_handle,
					wmi_lro_cmd);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_peer_rate_report_cmd(
		wmi_unified_t wmi_handle,
		struct wmi_peer_rate_report_params *rate_report_params)
{
	if (wmi_handle->ops->send_peer_rate_report_cmd)
		return wmi_handle->ops->send_peer_rate_report_cmd(wmi_handle,
					rate_report_params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_process_update_edca_param(
		wmi_unified_t wmi_handle,
		uint8_t vdev_id,
		bool mu_edca_param,
		struct wmi_host_wme_vparams wmm_vparams[WMI_MAX_NUM_AC])
{
	if (wmi_handle->ops->send_process_update_edca_param_cmd)
		return wmi_handle->ops->send_process_update_edca_param_cmd(wmi_handle,
					 vdev_id, mu_edca_param, wmm_vparams);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_probe_rsp_tmpl_send_cmd(
		wmi_unified_t wmi_handle,
		uint8_t vdev_id,
		struct wmi_probe_resp_params *probe_rsp_info)
{
	if (wmi_handle->ops->send_probe_rsp_tmpl_send_cmd)
		return wmi_handle->ops->send_probe_rsp_tmpl_send_cmd(wmi_handle,
						 vdev_id, probe_rsp_info);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_setup_install_key_cmd(wmi_unified_t wmi_handle,
					     struct set_key_params *key_params)
{
	if (wmi_handle->ops->send_setup_install_key_cmd)
		return wmi_handle->ops->send_setup_install_key_cmd(wmi_handle,
							key_params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_p2p_go_set_beacon_ie_cmd(wmi_unified_t wmi_handle,
						uint32_t vdev_id,
						uint8_t *p2p_ie)
{
	if (wmi_handle->ops->send_p2p_go_set_beacon_ie_cmd)
		return wmi_handle->ops->send_p2p_go_set_beacon_ie_cmd(wmi_handle,
						 vdev_id, p2p_ie);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_scan_probe_setoui_cmd(wmi_unified_t wmi_handle,
					     struct scan_mac_oui *psetoui)
{
	if (wmi_handle->ops->send_scan_probe_setoui_cmd)
		return wmi_handle->ops->send_scan_probe_setoui_cmd(wmi_handle,
			    psetoui);

	return QDF_STATUS_E_FAILURE;
}

#ifdef IPA_OFFLOAD
QDF_STATUS
wmi_unified_ipa_offload_control_cmd(
		wmi_unified_t wmi_handle,
		struct ipa_uc_offload_control_params *ipa_offload)
{
	if (!wmi_handle)
		return QDF_STATUS_E_FAILURE;

	if (wmi_handle->ops->send_ipa_offload_control_cmd)
		return wmi_handle->ops->send_ipa_offload_control_cmd(wmi_handle,
			    ipa_offload);

	return QDF_STATUS_E_FAILURE;
}
#endif

QDF_STATUS wmi_unified_pno_stop_cmd(wmi_unified_t wmi_handle, uint8_t vdev_id)
{
	if (wmi_handle->ops->send_pno_stop_cmd)
		return wmi_handle->ops->send_pno_stop_cmd(wmi_handle,
			    vdev_id);

	return QDF_STATUS_E_FAILURE;
}

#ifdef FEATURE_WLAN_SCAN_PNO
QDF_STATUS wmi_unified_pno_start_cmd(wmi_unified_t wmi_handle,
				     struct pno_scan_req_params *pno)
{
	if (wmi_handle->ops->send_pno_start_cmd)
		return wmi_handle->ops->send_pno_start_cmd(wmi_handle,
			    pno);

	return QDF_STATUS_E_FAILURE;
}
#endif

QDF_STATUS wmi_unified_nlo_mawc_cmd(wmi_unified_t wmi_handle,
				    struct nlo_mawc_params *params)
{
	if (wmi_handle->ops->send_nlo_mawc_cmd)
		return wmi_handle->ops->send_nlo_mawc_cmd(wmi_handle, params);

	return QDF_STATUS_E_FAILURE;
}

#ifdef WLAN_FEATURE_LINK_LAYER_STATS
QDF_STATUS wmi_unified_process_ll_stats_clear_cmd(wmi_unified_t wmi_handle,
				 const struct ll_stats_clear_params *clear_req)
{
	if (wmi_handle->ops->send_process_ll_stats_clear_cmd)
		return wmi_handle->ops->send_process_ll_stats_clear_cmd(wmi_handle,
									clear_req);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_process_ll_stats_set_cmd(wmi_unified_t wmi_handle,
				 const struct ll_stats_set_params *set_req)
{
	if (wmi_handle->ops->send_process_ll_stats_set_cmd)
		return wmi_handle->ops->send_process_ll_stats_set_cmd(wmi_handle,
								      set_req);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_process_ll_stats_get_cmd(wmi_unified_t wmi_handle,
				 const struct ll_stats_get_params *get_req)
{
	if (wmi_handle->ops->send_process_ll_stats_get_cmd)
		return wmi_handle->ops->send_process_ll_stats_get_cmd(wmi_handle,
								      get_req);

	return QDF_STATUS_E_FAILURE;
}

#ifdef FEATURE_CLUB_LL_STATS_AND_GET_STATION
QDF_STATUS wmi_process_unified_ll_stats_get_sta_cmd(
				wmi_unified_t wmi_handle,
				const struct ll_stats_get_params *get_req,
				bool is_always_over_qmi)
{
	if (wmi_handle->ops->send_unified_ll_stats_get_sta_cmd)
		return wmi_handle->ops->send_unified_ll_stats_get_sta_cmd(
						wmi_handle, get_req,
						is_always_over_qmi);

	return QDF_STATUS_E_FAILURE;
}
#endif
#endif /* WLAN_FEATURE_LINK_LAYER_STATS */

QDF_STATUS wmi_unified_congestion_request_cmd(wmi_unified_t wmi_handle,
					      uint8_t vdev_id)
{
	if (wmi_handle->ops->send_congestion_cmd)
		return wmi_handle->ops->send_congestion_cmd(wmi_handle,
			   vdev_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_snr_request_cmd(wmi_unified_t wmi_handle)
{
	if (wmi_handle->ops->send_snr_request_cmd)
		return wmi_handle->ops->send_snr_request_cmd(wmi_handle);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_snr_cmd(wmi_unified_t wmi_handle, uint8_t vdev_id)
{
	if (wmi_handle->ops->send_snr_cmd)
		return wmi_handle->ops->send_snr_cmd(wmi_handle,
			    vdev_id);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_link_status_req_cmd() - process link status request from UMAC
 * @wmi_handle: wmi handle
 * @params: get link status params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_link_status_req_cmd(wmi_unified_t wmi_handle,
					   struct link_status_params *params)
{
	if (wmi_handle->ops->send_link_status_req_cmd)
		return wmi_handle->ops->send_link_status_req_cmd(wmi_handle,
								 params);

	return QDF_STATUS_E_FAILURE;
}

#ifdef WLAN_SUPPORT_GREEN_AP
QDF_STATUS
wmi_unified_egap_conf_params_cmd(wmi_unified_t wmi_handle,
				 struct wlan_green_ap_egap_params *egap_params)
{
	if (wmi_handle->ops->send_egap_conf_params_cmd)
		return wmi_handle->ops->send_egap_conf_params_cmd(wmi_handle,
			    egap_params);

	return QDF_STATUS_E_FAILURE;
}
#endif

QDF_STATUS wmi_unified_csa_offload_enable(wmi_unified_t wmi_handle,
					  uint8_t vdev_id)
{
	if (wmi_handle->ops->send_csa_offload_enable_cmd)
		return wmi_handle->ops->send_csa_offload_enable_cmd(wmi_handle,
			    vdev_id);

	return QDF_STATUS_E_FAILURE;
}

#ifdef WLAN_FEATURE_CIF_CFR
QDF_STATUS
wmi_unified_oem_dma_ring_cfg(wmi_unified_t wmi_handle,
			     wmi_oem_dma_ring_cfg_req_fixed_param *cfg)
{
	if (wmi_handle->ops->send_oem_dma_cfg_cmd)
		return wmi_handle->ops->send_oem_dma_cfg_cmd(wmi_handle, cfg);

	return QDF_STATUS_E_FAILURE;
}
#endif

QDF_STATUS wmi_unified_start_oem_data_cmd(wmi_unified_t wmi_handle,
					  uint32_t data_len,
					  uint8_t *data)
{
	if (wmi_handle->ops->send_start_oem_data_cmd)
		return wmi_handle->ops->send_start_oem_data_cmd(wmi_handle,
								data_len,
								data);

	return QDF_STATUS_E_FAILURE;
}

#ifdef FEATURE_OEM_DATA
QDF_STATUS wmi_unified_start_oemv2_data_cmd(wmi_unified_t wmi_handle,
					    struct oem_data *params)
{
	if (wmi_handle->ops->send_start_oemv2_data_cmd)
		return wmi_handle->ops->send_start_oemv2_data_cmd(wmi_handle,
								  params);

	return QDF_STATUS_E_FAILURE;
}
#endif

QDF_STATUS
wmi_unified_dfs_phyerr_filter_offload_en_cmd(wmi_unified_t wmi_handle,
					     bool dfs_phyerr_filter_offload)
{
	if (wmi_handle->ops->send_dfs_phyerr_filter_offload_en_cmd)
		return wmi_handle->ops->send_dfs_phyerr_filter_offload_en_cmd(wmi_handle,
			    dfs_phyerr_filter_offload);

	return QDF_STATUS_E_FAILURE;
}

#if !defined(REMOVE_PKT_LOG) && defined(FEATURE_PKTLOG)
QDF_STATUS wmi_unified_pktlog_wmi_send_cmd(wmi_unified_t wmi_handle,
					   WMI_PKTLOG_EVENT pktlog_event,
					   uint32_t cmd_id,
					   uint8_t user_triggered)
{
	if (wmi_handle->ops->send_pktlog_wmi_send_cmd)
		return wmi_handle->ops->send_pktlog_wmi_send_cmd(wmi_handle,
			    pktlog_event, cmd_id, user_triggered);

	return QDF_STATUS_E_FAILURE;
}
#endif /* !REMOVE_PKT_LOG && FEATURE_PKTLOG */

QDF_STATUS wmi_unified_stats_ext_req_cmd(wmi_unified_t wmi_handle,
					 struct stats_ext_params *preq)
{
	if (wmi_handle->ops->send_stats_ext_req_cmd)
		return wmi_handle->ops->send_stats_ext_req_cmd(wmi_handle,
			    preq);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_process_dhcpserver_offload_cmd(
		wmi_unified_t wmi_handle,
		struct dhcp_offload_info_params *params)
{
	if (wmi_handle->ops->send_process_dhcpserver_offload_cmd)
		return wmi_handle->ops->send_process_dhcpserver_offload_cmd(wmi_handle,
			    params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_send_regdomain_info_to_fw_cmd(wmi_unified_t wmi_handle,
						     uint32_t reg_dmn,
						     uint16_t regdmn2G,
						     uint16_t regdmn5G,
						     uint8_t ctl2G,
						     uint8_t ctl5G)
{
	if (wmi_handle->ops->send_regdomain_info_to_fw_cmd)
		return wmi_handle->ops->send_regdomain_info_to_fw_cmd(wmi_handle,
			    reg_dmn, regdmn2G,
				regdmn5G, ctl2G,
				ctl5G);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_cfg_action_frm_tb_ppdu_cmd(
		wmi_unified_t wmi_handle,
		struct cfg_action_frm_tb_ppdu_param *cfg_info)
{
	if (wmi_handle->ops->send_cfg_action_frm_tb_ppdu_cmd)
		return wmi_handle->ops->send_cfg_action_frm_tb_ppdu_cmd(
						wmi_handle, cfg_info);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_save_fw_version_cmd(wmi_unified_t wmi_handle,
					   void *evt_buf)
{
	if (wmi_handle->ops->save_fw_version_cmd)
		return wmi_handle->ops->save_fw_version_cmd(wmi_handle,
			    evt_buf);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_log_supported_evt_cmd(wmi_unified_t wmi_handle,
					     uint8_t *event,
					     uint32_t len)
{
	if (wmi_handle->ops->send_log_supported_evt_cmd)
		return wmi_handle->ops->send_log_supported_evt_cmd(wmi_handle,
			    event, len);

	return QDF_STATUS_E_FAILURE;
}

void wmi_send_time_stamp_sync_cmd_tlv(wmi_unified_t wmi_handle)
{
	if (wmi_handle->ops->send_time_stamp_sync_cmd)
		wmi_handle->ops->send_time_stamp_sync_cmd(wmi_handle);

}

QDF_STATUS
wmi_unified_enable_specific_fw_logs_cmd(wmi_unified_t wmi_handle,
					struct wmi_wifi_start_log *start_log)
{
	if (wmi_handle->ops->send_enable_specific_fw_logs_cmd)
		return wmi_handle->ops->send_enable_specific_fw_logs_cmd(wmi_handle,
			    start_log);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_flush_logs_to_fw_cmd(wmi_unified_t wmi_handle)
{
	if (wmi_handle->ops->send_flush_logs_to_fw_cmd)
		return wmi_handle->ops->send_flush_logs_to_fw_cmd(wmi_handle);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_fw_test_cmd(wmi_unified_t wmi_handle,
				   struct set_fwtest_params *wmi_fwtest)
{
	if (wmi_handle->ops->send_fw_test_cmd)
		return wmi_handle->ops->send_fw_test_cmd(wmi_handle,
				  wmi_fwtest);

	return QDF_STATUS_E_FAILURE;

}

QDF_STATUS wmi_unified_wfa_test_cmd(wmi_unified_t wmi_handle,
				    struct set_wfatest_params *wmi_wfatest)
{
	if (wmi_handle->ops->send_wfa_test_cmd)
		return wmi_handle->ops->send_wfa_test_cmd(wmi_handle,
							  wmi_wfatest);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_unit_test_cmd(wmi_unified_t wmi_handle,
				     struct wmi_unit_test_cmd *wmi_utest)
{
	if (wmi_handle->ops->send_unit_test_cmd)
		return wmi_handle->ops->send_unit_test_cmd(wmi_handle,
				  wmi_utest);

	return QDF_STATUS_E_FAILURE;
}

#ifdef FEATURE_WLAN_APF
QDF_STATUS
wmi_unified_set_active_apf_mode_cmd(wmi_unified_t wmi, uint8_t vdev_id,
				    enum wmi_host_active_apf_mode ucast_mode,
				    enum wmi_host_active_apf_mode
							       mcast_bcast_mode)
{
	if (wmi->ops->send_set_active_apf_mode_cmd)
		return wmi->ops->send_set_active_apf_mode_cmd(wmi, vdev_id,
							      ucast_mode,
							      mcast_bcast_mode);
	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_send_apf_enable_cmd(wmi_unified_t wmi,
				uint32_t vdev_id, bool enable)
{
	if (wmi->ops->send_apf_enable_cmd)
		return wmi->ops->send_apf_enable_cmd(wmi, vdev_id, enable);
	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_send_apf_write_work_memory_cmd(wmi_unified_t wmi,
					   struct wmi_apf_write_memory_params
								  *write_params)
{
	if (wmi->ops->send_apf_write_work_memory_cmd)
		return wmi->ops->send_apf_write_work_memory_cmd(wmi,
								write_params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_send_apf_read_work_memory_cmd(wmi_unified_t wmi,
					  struct wmi_apf_read_memory_params
								   *read_params)
{
	if (wmi->ops->send_apf_read_work_memory_cmd)
		return wmi->ops->send_apf_read_work_memory_cmd(wmi,
							       read_params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_apf_read_memory_resp_event(wmi_unified_t wmi, void *evt_buf,
				struct wmi_apf_read_memory_resp_event_params
								*read_mem_evt)
{
	if (wmi->ops->extract_apf_read_memory_resp_event)
		return wmi->ops->extract_apf_read_memory_resp_event(wmi,
								evt_buf,
								read_mem_evt);

	return QDF_STATUS_E_FAILURE;
}
#endif /* FEATURE_WLAN_APF */

QDF_STATUS
wmi_unified_pdev_fips_cmd_send(wmi_unified_t wmi_handle,
			       struct fips_params *param)
{
	if (wmi_handle->ops->send_pdev_fips_cmd)
		return wmi_handle->ops->send_pdev_fips_cmd(wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

#ifdef WLAN_FEATURE_DISA
QDF_STATUS
wmi_unified_encrypt_decrypt_send_cmd(void *wmi_hdl,
				     struct disa_encrypt_decrypt_req_params
				     *params)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->send_encrypt_decrypt_send_cmd)
		return wmi_handle->ops->send_encrypt_decrypt_send_cmd(wmi_handle
								      , params);

	return QDF_STATUS_E_FAILURE;
}
#endif /* WLAN_FEATURE_DISA */

QDF_STATUS
wmi_unified_wlan_profile_enable_cmd_send(wmi_unified_t wmi_handle,
					 struct wlan_profile_params *param)
{
	if (wmi_handle->ops->send_wlan_profile_enable_cmd)
		return wmi_handle->ops->send_wlan_profile_enable_cmd(wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_wlan_profile_trigger_cmd_send(wmi_unified_t wmi_handle,
					  struct wlan_profile_params *param)
{
	if (wmi_handle->ops->send_wlan_profile_trigger_cmd)
		return wmi_handle->ops->send_wlan_profile_trigger_cmd(
					wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_wlan_profile_hist_intvl_cmd_send(wmi_unified_t wmi_handle,
					     struct wlan_profile_params *param)
{
	if (wmi_handle->ops->send_wlan_profile_hist_intvl_cmd)
		return wmi_handle->ops->send_wlan_profile_hist_intvl_cmd(
					wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_set_chan_cmd_send(wmi_unified_t wmi_handle,
			      struct channel_param *param)
{
	if (wmi_handle->ops->send_pdev_set_chan_cmd)
		return wmi_handle->ops->send_pdev_set_chan_cmd(wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_set_ratepwr_table_cmd_send(wmi_unified_t wmi_handle,
				       struct ratepwr_table_params *param)
{
	if (wmi_handle->ops->send_set_ratepwr_table_cmd)
		return wmi_handle->ops->send_set_ratepwr_table_cmd(wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_get_ratepwr_table_cmd_send(wmi_unified_t wmi_handle)
{
	if (wmi_handle->ops->send_get_ratepwr_table_cmd)
		return wmi_handle->ops->send_get_ratepwr_table_cmd(wmi_handle);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_set_ratepwr_chainmsk_cmd_send(
				wmi_unified_t wmi_handle,
				struct ratepwr_chainmsk_params *param)
{
	if (wmi_handle->ops->send_set_ratepwr_chainmsk_cmd)
		return wmi_handle->ops->send_set_ratepwr_chainmsk_cmd(
						wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_set_macaddr_cmd_send(wmi_unified_t wmi_handle,
					    struct macaddr_params *param)
{
	if (wmi_handle->ops->send_set_macaddr_cmd)
		return wmi_handle->ops->send_set_macaddr_cmd(wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_pdev_scan_start_cmd_send(wmi_unified_t wmi_handle)
{
	if (wmi_handle->ops->send_pdev_scan_start_cmd)
		return wmi_handle->ops->send_pdev_scan_start_cmd(wmi_handle);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_pdev_scan_end_cmd_send(wmi_unified_t wmi_handle)
{
	if (wmi_handle->ops->send_pdev_scan_end_cmd)
		return wmi_handle->ops->send_pdev_scan_end_cmd(wmi_handle);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_set_acparams_cmd_send(wmi_unified_t wmi_handle,
				  struct acparams_params *param)
{
	if (wmi_handle->ops->send_set_acparams_cmd)
		return wmi_handle->ops->send_set_acparams_cmd(wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_set_vap_dscp_tid_map_cmd_send(
			wmi_unified_t wmi_handle,
			struct vap_dscp_tid_map_params *param)
{
	if (wmi_handle->ops->send_set_vap_dscp_tid_map_cmd)
		return wmi_handle->ops->send_set_vap_dscp_tid_map_cmd(
						wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_proxy_ast_reserve_cmd_send(wmi_unified_t wmi_handle,
				       struct proxy_ast_reserve_params *param)
{
	if (wmi_handle->ops->send_proxy_ast_reserve_cmd)
		return wmi_handle->ops->send_proxy_ast_reserve_cmd(wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_set_bridge_mac_addr_cmd_send(
			wmi_unified_t wmi_handle,
			struct set_bridge_mac_addr_params *param)
{
	if (wmi_handle->ops->send_set_bridge_mac_addr_cmd)
		return wmi_handle->ops->send_set_bridge_mac_addr_cmd(wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_phyerr_enable_cmd_send(wmi_unified_t wmi_handle)
{
	if (wmi_handle->ops->send_phyerr_enable_cmd)
		return wmi_handle->ops->send_phyerr_enable_cmd(wmi_handle);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_phyerr_disable_cmd_send(wmi_unified_t wmi_handle)
{
	if (wmi_handle->ops->send_phyerr_disable_cmd)
		return wmi_handle->ops->send_phyerr_disable_cmd(wmi_handle);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_smart_ant_enable_tx_feedback_cmd_send(
		wmi_unified_t wmi_handle,
		struct smart_ant_enable_tx_feedback_params *param)
{
	if (wmi_handle->ops->send_smart_ant_enable_tx_feedback_cmd)
		return wmi_handle->ops->send_smart_ant_enable_tx_feedback_cmd(
							wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}
qdf_export_symbol(wmi_unified_smart_ant_enable_tx_feedback_cmd_send);

#ifdef WLAN_IOT_SIM_SUPPORT
QDF_STATUS
wmi_unified_simulation_test_cmd_send(
		wmi_unified_t wmi_handle,
		struct simulation_test_params *param)
{
	if (wmi_handle->ops->send_simulation_test_cmd)
		return wmi_handle->ops->send_simulation_test_cmd(
							wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}
qdf_export_symbol(wmi_unified_simulation_test_cmd_send);
#endif

QDF_STATUS
wmi_unified_vdev_spectral_configure_cmd_send(
		wmi_unified_t wmi_handle,
		struct vdev_spectral_configure_params *param)
{
	if (wmi_handle->ops->send_vdev_spectral_configure_cmd)
		return wmi_handle->ops->send_vdev_spectral_configure_cmd(
							wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_vdev_spectral_enable_cmd_send(
		wmi_unified_t wmi_handle,
		struct vdev_spectral_enable_params *param)
{
	if (wmi_handle->ops->send_vdev_spectral_enable_cmd)
		return wmi_handle->ops->send_vdev_spectral_enable_cmd(
						wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_bss_chan_info_request_cmd_send(
			wmi_unified_t wmi_handle,
			struct bss_chan_info_request_params *param)
{
	if (wmi_handle->ops->send_bss_chan_info_request_cmd)
		return wmi_handle->ops->send_bss_chan_info_request_cmd(
						wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_thermal_mitigation_param_cmd_send(
		wmi_unified_t wmi_handle,
		struct thermal_mitigation_params *param)
{
	if (wmi_handle->ops->send_thermal_mitigation_param_cmd)
		return wmi_handle->ops->send_thermal_mitigation_param_cmd(
						wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_vdev_set_fwtest_param_cmd_send(wmi_unified_t wmi_handle,
					   struct set_fwtest_params *param)
{
	if (wmi_handle->ops->send_vdev_set_fwtest_param_cmd)
		return wmi_handle->ops->send_vdev_set_fwtest_param_cmd(
						wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_vdev_set_custom_aggr_size_cmd_send(
		wmi_unified_t wmi_handle,
		struct set_custom_aggr_size_params *param)
{
	if (wmi_handle->ops->send_vdev_set_custom_aggr_size_cmd)
		return wmi_handle->ops->send_vdev_set_custom_aggr_size_cmd(
						wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_vdev_set_qdepth_thresh_cmd_send(
		wmi_unified_t wmi_handle,
		struct set_qdepth_thresh_params *param)
{
	if (wmi_handle->ops->send_vdev_set_qdepth_thresh_cmd)
		return wmi_handle->ops->send_vdev_set_qdepth_thresh_cmd(
						wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_pdev_set_regdomain_cmd_send(
			wmi_unified_t wmi_handle,
			struct pdev_set_regdomain_params *param)
{
	if (wmi_handle->ops->send_pdev_set_regdomain_cmd)
		return wmi_handle->ops->send_pdev_set_regdomain_cmd(wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_set_beacon_filter_cmd_send(
			wmi_unified_t wmi_handle,
			struct set_beacon_filter_params *param)
{
	if (wmi_handle->ops->send_set_beacon_filter_cmd)
		return wmi_handle->ops->send_set_beacon_filter_cmd(wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_remove_beacon_filter_cmd_send(
			wmi_unified_t wmi_handle,
			struct remove_beacon_filter_params *param)
{
	if (wmi_handle->ops->send_remove_beacon_filter_cmd)
		return wmi_handle->ops->send_remove_beacon_filter_cmd(
						wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_get_pn_send_cmd() - send command to get PN for peer
 * @wmi_hdl: wmi handle
 * @wmi_peer_tx_pn_request_cmd_fixed_param: pn request params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_get_pn_send_cmd(wmi_unified_t wmi_hdl,
				       struct peer_request_pn_param *pn_params)
{
	if (wmi_hdl->ops->send_pdev_get_pn_cmd)
		return wmi_hdl->ops->send_pdev_get_pn_cmd(wmi_hdl,
							     pn_params);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_mgmt_cmd_send() - WMI mgmt cmd function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold mgmt parameter
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
#if 0
QDF_STATUS wmi_unified_mgmt_cmd_send(void *wmi_hdl,
				uint8_t macaddr[QDF_MAC_ADDR_SIZE],
				struct mgmt_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_mgmt_cmd)
		return wmi_handle->ops->send_mgmt_cmd(wmi_handle,
				  macaddr, param);

	return QDF_STATUS_E_FAILURE;
}
#endif

QDF_STATUS wmi_unified_addba_clearresponse_cmd_send(
			wmi_unified_t wmi_handle,
			uint8_t macaddr[QDF_MAC_ADDR_SIZE],
			struct addba_clearresponse_params *param)
{
	if (wmi_handle->ops->send_addba_clearresponse_cmd)
		return wmi_handle->ops->send_addba_clearresponse_cmd(wmi_handle,
				  macaddr, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_addba_send_cmd_send(wmi_unified_t wmi_handle,
				uint8_t macaddr[QDF_MAC_ADDR_SIZE],
				struct addba_send_params *param)
{
	if (wmi_handle->ops->send_addba_send_cmd)
		return wmi_handle->ops->send_addba_send_cmd(wmi_handle,
				  macaddr, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_delba_send_cmd_send(wmi_unified_t wmi_handle,
				uint8_t macaddr[QDF_MAC_ADDR_SIZE],
				struct delba_send_params *param)
{
	if (wmi_handle->ops->send_delba_send_cmd)
		return wmi_handle->ops->send_delba_send_cmd(wmi_handle,
				  macaddr, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_addba_setresponse_cmd_send(wmi_unified_t wmi_handle,
				       uint8_t macaddr[QDF_MAC_ADDR_SIZE],
				       struct addba_setresponse_params *param)
{
	if (wmi_handle->ops->send_addba_setresponse_cmd)
		return wmi_handle->ops->send_addba_setresponse_cmd(wmi_handle,
				  macaddr, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_singleamsdu_cmd_send(wmi_unified_t wmi_handle,
				 uint8_t macaddr[QDF_MAC_ADDR_SIZE],
				 struct singleamsdu_params *param)
{
	if (wmi_handle->ops->send_singleamsdu_cmd)
		return wmi_handle->ops->send_singleamsdu_cmd(wmi_handle,
				  macaddr, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_mu_scan_cmd_send(wmi_unified_t wmi_handle,
			     struct mu_scan_params *param)
{
	if (wmi_handle->ops->send_mu_scan_cmd)
		return wmi_handle->ops->send_mu_scan_cmd(wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_lteu_config_cmd_send(wmi_unified_t wmi_handle,
				 struct lteu_config_params *param)
{
	if (wmi_handle->ops->send_lteu_config_cmd)
		return wmi_handle->ops->send_lteu_config_cmd(wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_set_psmode_cmd_send(wmi_unified_t wmi_handle,
				struct set_ps_mode_params *param)
{
	if (wmi_handle->ops->send_set_ps_mode_cmd)
		return wmi_handle->ops->send_set_ps_mode_cmd(wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_init_cmd_send(wmi_unified_t wmi_handle,
			  struct wmi_init_cmd_param *param)
{
	if (wmi_handle->ops->init_cmd_send)
		return wmi_handle->ops->init_cmd_send(wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_save_service_bitmap(wmi_unified_t wmi_handle, void *evt_buf,
				   void *bitmap_buf)
{
	if (wmi_handle->ops->save_service_bitmap) {
		return wmi_handle->ops->save_service_bitmap(wmi_handle, evt_buf,
						     bitmap_buf);
	}
	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_save_ext_service_bitmap(wmi_unified_t wmi_handle, void *evt_buf,
				       void *bitmap_buf)
{
	if (wmi_handle->ops->save_ext_service_bitmap) {
		return wmi_handle->ops->save_ext_service_bitmap(wmi_handle,
				evt_buf, bitmap_buf);
	}
	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_save_fw_version(wmi_unified_t wmi_handle, void *evt_buf)
{
	if (wmi_handle->ops->save_fw_version) {
		wmi_handle->ops->save_fw_version(wmi_handle, evt_buf);
		return 0;
	}
	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_check_and_update_fw_version(wmi_unified_t wmi_handle, void *evt_buf)
{
	if (wmi_handle->ops->check_and_update_fw_version)
		return wmi_handle->ops->check_and_update_fw_version(wmi_handle,
				evt_buf);

	return QDF_STATUS_E_FAILURE;
}

bool wmi_service_enabled(wmi_unified_t wmi_handle, uint32_t service_id)
{
	if ((service_id < wmi_services_max) &&
		(wmi_handle->services[service_id] != WMI_SERVICE_UNAVAILABLE)) {
		if (wmi_handle->ops->is_service_enabled) {
			return wmi_handle->ops->is_service_enabled(wmi_handle,
				wmi_handle->services[service_id]);
		}
	} else {
		wmi_info("Service %d not supported", service_id);
	}

	return false;
}

QDF_STATUS
wmi_get_target_cap_from_service_ready(
		wmi_unified_t wmi_handle, void *evt_buf,
		struct wlan_psoc_target_capability_info *ev)
{
	if (wmi_handle->ops->get_target_cap_from_service_ready)
		return wmi_handle->ops->get_target_cap_from_service_ready(
						wmi_handle, evt_buf, ev);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_fw_version(wmi_unified_t wmi_handle, void *evt_buf,
				  struct wmi_host_fw_ver *fw_ver)
{
	if (wmi_handle->ops->extract_fw_version)
		return wmi_handle->ops->extract_fw_version(wmi_handle,
				evt_buf, fw_ver);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_fw_abi_version(wmi_unified_t wmi_handle, void *evt_buf,
				      struct wmi_host_fw_abi_ver *fw_ver)
{
	if (wmi_handle->ops->extract_fw_abi_version)
		return wmi_handle->ops->extract_fw_abi_version(wmi_handle,
		evt_buf, fw_ver);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_hal_reg_cap(wmi_unified_t wmi_handle, void *evt_buf,
			struct wlan_psoc_hal_reg_capability *hal_reg_cap)
{
	if (wmi_handle->ops->extract_hal_reg_cap)
		return wmi_handle->ops->extract_hal_reg_cap(wmi_handle,
			evt_buf, hal_reg_cap);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_hal_reg_cap_ext2(
		wmi_unified_t wmi_handle, void *evt_buf, uint8_t phy_idx,
		struct wlan_psoc_host_hal_reg_capabilities_ext2 *hal_reg_cap)
{
	if (wmi_handle->ops->extract_hal_reg_cap_ext2)
		return wmi_handle->ops->extract_hal_reg_cap_ext2(
			wmi_handle, evt_buf, phy_idx, hal_reg_cap);

	return QDF_STATUS_E_FAILURE;
}

uint32_t
wmi_extract_num_mem_reqs_from_service_ready(
		wmi_unified_t wmi_handle,
		void *evt_buf)
{
	if (wmi_handle->ops->extract_num_mem_reqs)
		return wmi_handle->ops->extract_num_mem_reqs(wmi_handle,
				evt_buf);

	return 0;
}

QDF_STATUS
wmi_extract_host_mem_req_from_service_ready(wmi_unified_t wmi_handle,
					    void *evt_buf,
					    host_mem_req *mem_reqs,
					    uint32_t num_active_peers,
					    uint32_t num_peers,
					    enum wmi_fw_mem_prio fw_prio,
					    uint16_t idx)
{
	if (wmi_handle->ops->extract_host_mem_req)
		return wmi_handle->ops->extract_host_mem_req(wmi_handle,
				evt_buf, mem_reqs, num_active_peers,
				num_peers, fw_prio, idx);

	return QDF_STATUS_E_FAILURE;
}

uint32_t wmi_ready_extract_init_status(wmi_unified_t wmi_handle, void *ev)
{
	if (wmi_handle->ops->ready_extract_init_status)
		return wmi_handle->ops->ready_extract_init_status(wmi_handle,
			ev);


	return 1;

}

QDF_STATUS wmi_ready_extract_mac_addr(wmi_unified_t wmi_handle,
				      void *ev, uint8_t *macaddr)
{
	if (wmi_handle->ops->ready_extract_mac_addr)
		return wmi_handle->ops->ready_extract_mac_addr(wmi_handle,
			ev, macaddr);


	return QDF_STATUS_E_FAILURE;
}

wmi_host_mac_addr
*wmi_ready_extract_mac_addr_list(wmi_unified_t wmi_handle, void *ev,
				 uint8_t *num_mac_addr)
{
	if (wmi_handle->ops->ready_extract_mac_addr_list)
		return wmi_handle->ops->ready_extract_mac_addr_list(wmi_handle,
			ev, num_mac_addr);

	*num_mac_addr = 0;

	return NULL;
}

QDF_STATUS wmi_extract_ready_event_params(
		wmi_unified_t wmi_handle, void *evt_buf,
		struct wmi_host_ready_ev_param *ev_param)
{
	if (wmi_handle->ops->extract_ready_event_params)
		return wmi_handle->ops->extract_ready_event_params(wmi_handle,
			evt_buf, ev_param);

	return QDF_STATUS_E_FAILURE;
}

uint8_t *wmi_extract_dbglog_data_len(wmi_unified_t wmi_handle, void *evt_buf,
				     uint32_t *len)
{
	if (wmi_handle->ops->extract_dbglog_data_len)
		return wmi_handle->ops->extract_dbglog_data_len(wmi_handle,
			evt_buf, len);


	return NULL;
}
qdf_export_symbol(wmi_extract_dbglog_data_len);

QDF_STATUS wmi_send_ext_resource_config(wmi_unified_t wmi_handle,
					wmi_host_ext_resource_config *ext_cfg)
{
	if (wmi_handle->ops->send_ext_resource_config)
		return wmi_handle->ops->send_ext_resource_config(wmi_handle,
				ext_cfg);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_rtt_meas_req_test_cmd_send(wmi_unified_t wmi_handle,
				       struct rtt_meas_req_test_params *param)
{
	if (wmi_handle->ops->send_rtt_meas_req_test_cmd)
		return wmi_handle->ops->send_rtt_meas_req_test_cmd(wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_rtt_meas_req_cmd_send(wmi_unified_t wmi_handle,
				  struct rtt_meas_req_params *param)
{
	if (wmi_handle->ops->send_rtt_meas_req_cmd)
		return wmi_handle->ops->send_rtt_meas_req_cmd(wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_lci_set_cmd_send(wmi_unified_t wmi_handle,
					struct lci_set_params *param)
{
	if (wmi_handle->ops->send_lci_set_cmd)
		return wmi_handle->ops->send_lci_set_cmd(wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_lcr_set_cmd_send(wmi_unified_t wmi_handle,
					struct lcr_set_params *param)
{
	if (wmi_handle->ops->send_lcr_set_cmd)
		return wmi_handle->ops->send_lcr_set_cmd(wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_rtt_keepalive_req_cmd_send(wmi_unified_t wmi_handle,
				       struct rtt_keepalive_req_params *param)
{
	if (wmi_handle->ops->send_rtt_keepalive_req_cmd)
		return wmi_handle->ops->send_rtt_keepalive_req_cmd(wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_send_periodic_chan_stats_config_cmd(
		wmi_unified_t wmi_handle,
		struct periodic_chan_stats_params *param)
{
	if (wmi_handle->ops->send_periodic_chan_stats_config_cmd)
		return wmi_handle->ops->send_periodic_chan_stats_config_cmd(
						wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_send_get_user_position_cmd(wmi_unified_t wmi_handle, uint32_t value)
{
	if (wmi_handle->ops->send_get_user_position_cmd)
		return wmi_handle->ops->send_get_user_position_cmd(wmi_handle,
								value);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_send_get_peer_mumimo_tx_count_cmd(wmi_unified_t wmi_handle, uint32_t value)
{
	if (wmi_handle->ops->send_get_peer_mumimo_tx_count_cmd)
		return wmi_handle->ops->send_get_peer_mumimo_tx_count_cmd(
							wmi_handle, value);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_send_reset_peer_mumimo_tx_count_cmd(wmi_unified_t wmi_handle,
					uint32_t value)
{
	if (wmi_handle->ops->send_reset_peer_mumimo_tx_count_cmd)
		return wmi_handle->ops->send_reset_peer_mumimo_tx_count_cmd(
						wmi_handle, value);

	return QDF_STATUS_E_FAILURE;
}

/* Extract - APIs */

QDF_STATUS wmi_extract_ctl_failsafe_check_ev_param(
		wmi_unified_t wmi_handle,
		void *evt_buf,
		struct wmi_host_pdev_ctl_failsafe_event *param)
{
	if (wmi_handle->ops->extract_ctl_failsafe_check_ev_param)
		return wmi_handle->ops->extract_ctl_failsafe_check_ev_param(
			wmi_handle, evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_fips_event_data(wmi_unified_t wmi_handle, void *evt_buf,
			    struct wmi_host_fips_event_param *param)
{
	if (wmi_handle->ops->extract_fips_event_data) {
		return wmi_handle->ops->extract_fips_event_data(wmi_handle,
			evt_buf, param);
	}
	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_extract_pn() - extract pn event data
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: pointer to get pn event param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_extract_pn(wmi_unified_t wmi_hdl, void *evt_buf,
				  struct wmi_host_get_pn_event *param)
{
	if (wmi_hdl->ops->extract_get_pn_data)
		return wmi_hdl->ops->extract_get_pn_data(wmi_hdl,
							 evt_buf, param);
	return QDF_STATUS_E_FAILURE;
}

#ifdef WLAN_FEATURE_DISA
QDF_STATUS
wmi_extract_encrypt_decrypt_resp_params(void *wmi_hdl, void *evt_buf,
					struct disa_encrypt_decrypt_resp_params
					*param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->extract_encrypt_decrypt_resp_event)
		return
		wmi_handle->ops->extract_encrypt_decrypt_resp_event(wmi_handle,
								    evt_buf,
								    param);

	return QDF_STATUS_E_FAILURE;
}
#endif /* WLAN_FEATURE_DISA */

QDF_STATUS
wmi_extract_mgmt_rx_params(wmi_unified_t wmi_handle, void *evt_buf,
			   struct mgmt_rx_event_params *hdr, uint8_t **bufp)
{
	if (wmi_handle->ops->extract_mgmt_rx_params)
		return wmi_handle->ops->extract_mgmt_rx_params(wmi_handle,
				evt_buf, hdr, bufp);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_vdev_roam_param(wmi_unified_t wmi_handle, void *evt_buf,
			    wmi_host_roam_event *param)
{
	if (wmi_handle->ops->extract_vdev_roam_param)
		return wmi_handle->ops->extract_vdev_roam_param(wmi_handle,
				evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_vdev_scan_ev_param(wmi_unified_t wmi_handle, void *evt_buf,
			       struct scan_event *param)
{
	if (wmi_handle->ops->extract_vdev_scan_ev_param)
		return wmi_handle->ops->extract_vdev_scan_ev_param(wmi_handle,
				evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}

#ifdef FEATURE_WLAN_SCAN_PNO
QDF_STATUS
wmi_extract_nlo_match_ev_param(wmi_unified_t wmi_handle, void *evt_buf,
			       struct scan_event *param)
{
	if (wmi_handle->ops->extract_nlo_match_ev_param)
		return wmi_handle->ops->extract_nlo_match_ev_param(wmi_handle,
			evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_nlo_complete_ev_param(wmi_unified_t wmi_handle, void *evt_buf,
				  struct scan_event *param)
{
	if (wmi_handle->ops->extract_nlo_complete_ev_param)
		return wmi_handle->ops->extract_nlo_complete_ev_param(
			wmi_handle, evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}
#endif

QDF_STATUS
wmi_extract_mu_ev_param(wmi_unified_t wmi_handle, void *evt_buf,
			wmi_host_mu_report_event *param)
{
	if (wmi_handle->ops->extract_mu_ev_param)
		return wmi_handle->ops->extract_mu_ev_param(wmi_handle, evt_buf,
			param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_mu_db_entry(wmi_unified_t wmi_handle, void *evt_buf,
			uint8_t idx, wmi_host_mu_db_entry *param)
{
	if (wmi_handle->ops->extract_mu_db_entry)
		return wmi_handle->ops->extract_mu_db_entry(wmi_handle, evt_buf,
			idx, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_mumimo_tx_count_ev_param(wmi_unified_t wmi_handle, void *evt_buf,
				     wmi_host_peer_txmu_cnt_event *param)
{
	if (wmi_handle->ops->extract_mumimo_tx_count_ev_param)
		return wmi_handle->ops->extract_mumimo_tx_count_ev_param(
						wmi_handle, evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_peer_gid_userpos_list_ev_param(
		wmi_unified_t wmi_handle,
		void *evt_buf,
		wmi_host_peer_gid_userpos_list_event *param)
{
	if (wmi_handle->ops->extract_peer_gid_userpos_list_ev_param)
		return wmi_handle->ops->extract_peer_gid_userpos_list_ev_param(
						wmi_handle, evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_esp_estimate_ev_param(wmi_unified_t wmi_handle, void *evt_buf,
				  struct esp_estimation_event *param)
{
	if (wmi_handle->ops->extract_esp_estimation_ev_param)
		return wmi_handle->ops->extract_esp_estimation_ev_param(
				wmi_handle, evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_gpio_input_ev_param(wmi_unified_t wmi_handle, void *evt_buf,
				uint32_t *gpio_num)
{
	if (wmi_handle->ops->extract_gpio_input_ev_param)
		return wmi_handle->ops->extract_gpio_input_ev_param(wmi_handle,
			evt_buf, gpio_num);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_pdev_reserve_ast_ev_param(
		wmi_unified_t wmi_handle, void *evt_buf,
		struct wmi_host_proxy_ast_reserve_param *param)
{
	if (wmi_handle->ops->extract_pdev_reserve_ast_ev_param)
		return wmi_handle->ops->extract_pdev_reserve_ast_ev_param(
						wmi_handle, evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_pdev_generic_buffer_ev_param(
		wmi_unified_t wmi_handle, void *evt_buf,
		wmi_host_pdev_generic_buffer_event *param)
{
	if (wmi_handle->ops->extract_pdev_generic_buffer_ev_param)
		return wmi_handle->ops->extract_pdev_generic_buffer_ev_param(
						wmi_handle, evt_buf, param);

	return QDF_STATUS_E_FAILURE;

}

QDF_STATUS wmi_extract_peer_ratecode_list_ev(
		wmi_unified_t wmi_handle, void *evt_buf,
		uint8_t *peer_mac, uint32_t *pdev_id, wmi_sa_rate_cap *rate_cap)
{
	if (wmi_handle->ops->extract_peer_ratecode_list_ev)
		return wmi_handle->ops->extract_peer_ratecode_list_ev(
						wmi_handle, evt_buf,
						peer_mac, pdev_id, rate_cap);

	return QDF_STATUS_E_FAILURE;

}

QDF_STATUS
wmi_extract_comb_phyerr(wmi_unified_t wmi_handle, void *evt_buf,
			uint16_t datalen, uint16_t *buf_offset,
			wmi_host_phyerr_t *phyerr)
{
	if (wmi_handle->ops->extract_comb_phyerr)
		return wmi_handle->ops->extract_comb_phyerr(wmi_handle,
		evt_buf, datalen, buf_offset, phyerr);

	return QDF_STATUS_E_FAILURE;

}

QDF_STATUS
wmi_extract_single_phyerr(wmi_unified_t wmi_handle, void *evt_buf,
			  uint16_t datalen, uint16_t *buf_offset,
			  wmi_host_phyerr_t *phyerr)
{
	if (wmi_handle->ops->extract_single_phyerr)
		return wmi_handle->ops->extract_single_phyerr(wmi_handle,
			evt_buf, datalen, buf_offset, phyerr);

	return QDF_STATUS_E_FAILURE;

}

QDF_STATUS
wmi_extract_composite_phyerr(wmi_unified_t wmi_handle, void *evt_buf,
			     uint16_t datalen, wmi_host_phyerr_t *phyerr)
{
	if (wmi_handle->ops->extract_composite_phyerr)
		return wmi_handle->ops->extract_composite_phyerr(wmi_handle,
			evt_buf, datalen, phyerr);

	return QDF_STATUS_E_FAILURE;

}

QDF_STATUS
wmi_extract_pmf_bcn_protect_stats(wmi_unified_t wmi_handle, void *evt_buf,
				  wmi_host_pmf_bcn_protect_stats *bcn_stats)
{
	if (wmi_handle->ops->extract_pmf_bcn_protect_stats)
		return wmi_handle->ops->extract_pmf_bcn_protect_stats(
				wmi_handle, evt_buf, bcn_stats);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_unit_test(wmi_unified_t wmi_handle, void *evt_buf,
		      wmi_unit_test_event *unit_test, uint32_t maxspace)
{
	if (wmi_handle->ops->extract_unit_test)
		return wmi_handle->ops->extract_unit_test(wmi_handle,
			evt_buf, unit_test, maxspace);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_pdev_ext_stats(wmi_unified_t wmi_handle, void *evt_buf,
			   uint32_t index,
			   wmi_host_pdev_ext_stats *pdev_ext_stats)
{
	if (wmi_handle->ops->extract_pdev_ext_stats)
		return wmi_handle->ops->extract_pdev_ext_stats(wmi_handle,
			evt_buf, index, pdev_ext_stats);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_rtt_hdr(wmi_unified_t wmi_handle, void *evt_buf,
			       wmi_host_rtt_event_hdr *ev)
{
	if (wmi_handle->ops->extract_rtt_hdr)
		return wmi_handle->ops->extract_rtt_hdr(wmi_handle,
			evt_buf, ev);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_bcnflt_stats(
		wmi_unified_t wmi_handle, void *evt_buf,
		uint32_t index, wmi_host_bcnflt_stats *bcnflt_stats)
{
	if (wmi_handle->ops->extract_bcnflt_stats)
		return wmi_handle->ops->extract_bcnflt_stats(wmi_handle,
			evt_buf, index, bcnflt_stats);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_rtt_ev(wmi_unified_t wmi_handle, void *evt_buf,
			      wmi_host_rtt_meas_event *ev,
			      uint8_t *hdump, uint16_t hdump_len)
{
	if (wmi_handle->ops->extract_rtt_ev)
		return wmi_handle->ops->extract_rtt_ev(wmi_handle,
			evt_buf, ev, hdump, hdump_len);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_peer_retry_stats(
	wmi_unified_t wmi_handle, void *evt_buf,
	uint32_t index, struct wmi_host_peer_retry_stats *peer_retry_stats)
{
	if (wmi_handle->ops->extract_peer_retry_stats)
		return wmi_handle->ops->extract_peer_retry_stats(wmi_handle,
			evt_buf, index, peer_retry_stats);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_rtt_error_report_ev(wmi_unified_t wmi_handle, void *evt_buf,
				wmi_host_rtt_error_report_event *ev)
{
	if (wmi_handle->ops->extract_rtt_error_report_ev)
		return wmi_handle->ops->extract_rtt_error_report_ev(wmi_handle,
			evt_buf, ev);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_chan_stats(wmi_unified_t wmi_handle, void *evt_buf,
		       uint32_t index, wmi_host_chan_stats *chan_stats)
{
	if (wmi_handle->ops->extract_chan_stats)
		return wmi_handle->ops->extract_chan_stats(wmi_handle,
			evt_buf, index, chan_stats);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_thermal_stats(wmi_unified_t wmi_handle, void *evt_buf,
				     uint32_t *temp,
				     enum thermal_throttle_level *level,
				     uint32_t *pdev_id)
{
	if (wmi_handle->ops->extract_thermal_stats)
		return wmi_handle->ops->extract_thermal_stats(wmi_handle,
			evt_buf, temp, level, pdev_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_profile_ctx(wmi_unified_t wmi_handle, void *evt_buf,
			wmi_host_wlan_profile_ctx_t *profile_ctx)
{
	if (wmi_handle->ops->extract_profile_ctx)
		return wmi_handle->ops->extract_profile_ctx(wmi_handle,
			evt_buf, profile_ctx);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_thermal_level_stats(wmi_unified_t wmi_handle, void *evt_buf,
				uint8_t idx, uint32_t *levelcount,
				uint32_t *dccount)
{
	if (wmi_handle->ops->extract_thermal_level_stats)
		return wmi_handle->ops->extract_thermal_level_stats(wmi_handle,
			evt_buf, idx, levelcount, dccount);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_profile_data(wmi_unified_t wmi_handle, void *evt_buf, uint8_t idx,
			 wmi_host_wlan_profile_t *profile_data)
{
	if (wmi_handle->ops->extract_profile_data)
		return wmi_handle->ops->extract_profile_data(wmi_handle,
			evt_buf, idx, profile_data);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_bss_chan_info_event(
		wmi_unified_t wmi_handle, void *evt_buf,
		wmi_host_pdev_bss_chan_info_event *bss_chan_info)
{
	if (wmi_handle->ops->extract_bss_chan_info_event)
		return wmi_handle->ops->extract_bss_chan_info_event(wmi_handle,
		evt_buf, bss_chan_info);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_tx_data_traffic_ctrl_ev(wmi_unified_t wmi_handle, void *evt_buf,
				    wmi_host_tx_data_traffic_ctrl_event *ev)
{
	if (wmi_handle->ops->extract_tx_data_traffic_ctrl_ev)
		return wmi_handle->ops->extract_tx_data_traffic_ctrl_ev(
						wmi_handle, evt_buf, ev);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_vdev_extd_stats(wmi_unified_t wmi_handle, void *evt_buf,
			    uint32_t index,
			    wmi_host_vdev_extd_stats *vdev_extd_stats)
{
	if (wmi_handle->ops->extract_vdev_extd_stats)
		return wmi_handle->ops->extract_vdev_extd_stats(wmi_handle,
				evt_buf, index, vdev_extd_stats);
	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_bcn_stats(wmi_unified_t wmi_handle, void *evt_buf,
		      uint32_t index, wmi_host_bcn_stats *vdev_bcn_stats)
{
	if (wmi_handle->ops->extract_bcn_stats)
		return wmi_handle->ops->extract_bcn_stats(wmi_handle,
				evt_buf, index, vdev_bcn_stats);
	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_vdev_nac_rssi_stats(
		wmi_unified_t wmi_handle, void *evt_buf,
		struct wmi_host_vdev_nac_rssi_event *vdev_nac_rssi_stats)
{
	if (wmi_handle->ops->extract_vdev_nac_rssi_stats)
		return wmi_handle->ops->extract_vdev_nac_rssi_stats(wmi_handle,
				evt_buf, vdev_nac_rssi_stats);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_vdev_prb_fils_stats(
		wmi_unified_t wmi_handle, void *evt_buf,
		uint32_t index,
		struct wmi_host_vdev_prb_fils_stats *vdev_prb_fils_stats)
{
	if (wmi_handle->ops->extract_vdev_prb_fils_stats)
		return wmi_handle->ops->extract_vdev_prb_fils_stats(wmi_handle,
				evt_buf, index, vdev_prb_fils_stats);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_send_power_dbg_cmd(wmi_unified_t wmi_handle,
					  struct wmi_power_dbg_params *param)
{
	if (wmi_handle->ops->send_power_dbg_cmd)
		return wmi_handle->ops->send_power_dbg_cmd(wmi_handle,
				  param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_send_btcoex_wlan_priority_cmd(wmi_unified_t wmi_handle,
					  struct btcoex_cfg_params *param)
{
	if (wmi_handle->ops->send_btcoex_wlan_priority_cmd)
		return wmi_handle->ops->send_btcoex_wlan_priority_cmd(
						wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_send_btcoex_duty_cycle_cmd(wmi_unified_t wmi_handle,
				       struct btcoex_cfg_params *param)
{
	if (wmi_handle->ops->send_btcoex_duty_cycle_cmd)
		return wmi_handle->ops->send_btcoex_duty_cycle_cmd(
						wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_service_ready_ext(
		wmi_unified_t wmi_handle, uint8_t *evt_buf,
		struct wlan_psoc_host_service_ext_param *param)
{
	if (wmi_handle->ops->extract_service_ready_ext)
		return wmi_handle->ops->extract_service_ready_ext(wmi_handle,
				evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_service_ready_ext2(
		struct wmi_unified *wmi_handle, uint8_t *evt_buf,
		struct wlan_psoc_host_service_ext2_param *param)
{
	if (wmi_handle->ops->extract_service_ready_ext2)
		return wmi_handle->ops->extract_service_ready_ext2(wmi_handle,
				evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_sar_cap_service_ready_ext(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf,
			struct wlan_psoc_host_service_ext_param *ext_param)
{
	if (wmi_handle->ops->extract_sar_cap_service_ready_ext)
		return wmi_handle->ops->extract_sar_cap_service_ready_ext(
				wmi_handle,
				evt_buf, ext_param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_hw_mode_cap_service_ready_ext(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf, uint8_t hw_mode_idx,
			struct wlan_psoc_host_hw_mode_caps *param)
{
	if (wmi_handle->ops->extract_hw_mode_cap_service_ready_ext)
		return wmi_handle->ops->extract_hw_mode_cap_service_ready_ext(
				wmi_handle,
				evt_buf, hw_mode_idx, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_mac_phy_cap_service_ready_ext(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf,
			uint8_t hw_mode_id,
			uint8_t phy_id,
			struct wlan_psoc_host_mac_phy_caps *param)
{
	if (wmi_handle->ops->extract_mac_phy_cap_service_ready_ext)
		return wmi_handle->ops->extract_mac_phy_cap_service_ready_ext(
				wmi_handle,
				evt_buf, hw_mode_id, phy_id, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_mac_phy_cap_service_ready_ext2(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf,
			uint8_t hw_mode_id,
			uint8_t phy_id,
			uint8_t phy_idx,
			struct wlan_psoc_host_mac_phy_caps_ext2 *mac_phy_cap)
{
	if (wmi_handle->ops->extract_mac_phy_cap_service_ready_ext2)
		return wmi_handle->ops->extract_mac_phy_cap_service_ready_ext2(
				wmi_handle, evt_buf, hw_mode_id, phy_id,
				phy_idx, mac_phy_cap);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_reg_cap_service_ready_ext(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf, uint8_t phy_idx,
			struct wlan_psoc_host_hal_reg_capabilities_ext *param)
{
	if (wmi_handle->ops->extract_reg_cap_service_ready_ext)
		return wmi_handle->ops->extract_reg_cap_service_ready_ext(
				wmi_handle,
				evt_buf, phy_idx, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_dbr_ring_cap_service_ready_ext(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf, uint8_t idx,
			struct wlan_psoc_host_dbr_ring_caps *param)
{
	if (wmi_handle->ops->extract_dbr_ring_cap_service_ready_ext)
		return wmi_handle->ops->extract_dbr_ring_cap_service_ready_ext(
				wmi_handle,
				evt_buf, idx, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_dbr_ring_cap_service_ready_ext2(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf, uint8_t idx,
			struct wlan_psoc_host_dbr_ring_caps *param)
{
	if (wmi_handle->ops->extract_dbr_ring_cap_service_ready_ext2)
		return wmi_handle->ops->extract_dbr_ring_cap_service_ready_ext2(
				wmi_handle,
				evt_buf, idx, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_scan_radio_cap_service_ready_ext2(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf, uint8_t idx,
			struct wlan_psoc_host_scan_radio_caps *param)
{
	if (wmi_handle->ops->extract_scan_radio_cap_service_ready_ext2)
		return wmi_handle->ops->
		       extract_scan_radio_cap_service_ready_ext2(
				wmi_handle,
				evt_buf, idx, param);

	return QDF_STATUS_E_FAILURE;
}

#ifdef WLAN_CONV_SPECTRAL_ENABLE
QDF_STATUS wmi_extract_pdev_sscan_fw_cmd_fixed_param(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf,
			struct spectral_startscan_resp_params *param)
{
	if (wmi_handle->ops->extract_pdev_sscan_fw_cmd_fixed_param)
		return wmi_handle->ops->extract_pdev_sscan_fw_cmd_fixed_param(
				wmi_handle,
				evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_pdev_sscan_fft_bin_index(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf,
			struct spectral_fft_bin_markers_160_165mhz *param)
{
	if (wmi_handle->ops->extract_pdev_sscan_fft_bin_index)
		return wmi_handle->ops->extract_pdev_sscan_fft_bin_index(
				wmi_handle,
				evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}
#endif /* WLAN_CONV_SPECTRAL_ENABLE */

QDF_STATUS wmi_extract_spectral_scaling_params_service_ready_ext(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf, uint8_t idx,
			struct wlan_psoc_host_spectral_scaling_params *param)
{
	if (wmi_handle->ops->extract_scaling_params_service_ready_ext)
		return wmi_handle->ops->extract_scaling_params_service_ready_ext
				(wmi_handle, evt_buf, idx, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_pdev_utf_event(wmi_unified_t wmi_handle,
				      uint8_t *evt_buf,
				      struct wmi_host_pdev_utf_event *param)
{
	if (wmi_handle->ops->extract_pdev_utf_event)
		return wmi_handle->ops->extract_pdev_utf_event(
				wmi_handle,
				evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_pdev_qvit_event(wmi_unified_t wmi_handle,
				       uint8_t *evt_buf,
				       struct wmi_host_pdev_qvit_event *param)
{
	if (wmi_handle->ops->extract_pdev_qvit_event)
		return wmi_handle->ops->extract_pdev_qvit_event(
				wmi_handle,
				evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_send_coex_ver_cfg_cmd(wmi_unified_t wmi_handle,
				  coex_ver_cfg_t *param)
{
	if (wmi_handle->ops->send_coex_ver_cfg_cmd)
		return wmi_handle->ops->send_coex_ver_cfg_cmd(wmi_handle,
			param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_send_coex_config_cmd(wmi_unified_t wmi_handle,
				 struct coex_config_params *param)
{
	if (wmi_handle->ops->send_coex_config_cmd)
		return wmi_handle->ops->send_coex_config_cmd(wmi_handle,
			param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_send_request_get_rcpi_cmd(wmi_unified_t wmi_handle,
				      struct rcpi_req *get_rcpi_param)
{
	if (wmi_handle->ops->send_get_rcpi_cmd)
		return wmi_handle->ops->send_get_rcpi_cmd(wmi_handle,
			   get_rcpi_param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_rcpi_response_event(wmi_unified_t wmi_handle, void *evt_buf,
				struct rcpi_res *res)
{
	struct wmi_ops *ops = wmi_handle->ops;

	if (ops->extract_rcpi_response_event)
		return ops->extract_rcpi_response_event(wmi_handle, evt_buf,
							res);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_dfs_phyerr_offload_en_cmd(wmi_unified_t wmi_handle,
				      uint32_t pdev_id)
{
	if (wmi_handle->ops->send_dfs_phyerr_offload_en_cmd)
		return wmi_handle->ops->send_dfs_phyerr_offload_en_cmd(
				wmi_handle, pdev_id);

	return QDF_STATUS_E_FAILURE;
}

#ifdef QCA_SUPPORT_AGILE_DFS
QDF_STATUS
wmi_unified_send_vdev_adfs_ch_cfg_cmd(wmi_unified_t wmi_handle,
				      struct vdev_adfs_ch_cfg_params *param)
{
	if (wmi_handle->ops->send_adfs_ch_cfg_cmd)
		return wmi_handle->ops->send_adfs_ch_cfg_cmd(
				wmi_handle,
				param);
	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_send_vdev_adfs_ocac_abort_cmd(wmi_unified_t wmi_handle,
					  struct vdev_adfs_abort_params *param)
{
	if (wmi_handle->ops->send_adfs_ocac_abort_cmd)
		return wmi_handle->ops->send_adfs_ocac_abort_cmd(
				wmi_handle,
				param);
	return QDF_STATUS_E_FAILURE;
}
#endif

QDF_STATUS
wmi_unified_dfs_phyerr_offload_dis_cmd(wmi_unified_t wmi_handle,
				       uint32_t pdev_id)
{
	if (wmi_handle->ops->send_dfs_phyerr_offload_dis_cmd)
		return wmi_handle->ops->send_dfs_phyerr_offload_dis_cmd(
				wmi_handle, pdev_id);

	return QDF_STATUS_E_FAILURE;
}

#ifdef WLAN_SUPPORT_RF_CHARACTERIZATION
QDF_STATUS wmi_extract_num_rf_characterization_entries(wmi_unified_t wmi_hdl,
				uint8_t *evt_buf,
				uint32_t *num_rf_characterization_entries)
{
	if (wmi_hdl->ops->extract_num_rf_characterization_entries)
		return wmi_hdl->ops->extract_num_rf_characterization_entries(wmi_hdl,
				evt_buf, num_rf_characterization_entries);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_rf_characterization_entries(wmi_unified_t wmi_hdl,
	uint8_t *evt_buf,
	uint32_t num_rf_characterization_entries,
	struct wmi_host_rf_characterization_event_param *rf_characterization_entries)
{
	if (wmi_hdl->ops->extract_rf_characterization_entries)
		return wmi_hdl->ops->extract_rf_characterization_entries(wmi_hdl,
				evt_buf, num_rf_characterization_entries,
				rf_characterization_entries);

	return QDF_STATUS_E_FAILURE;
}
#endif

QDF_STATUS wmi_extract_chainmask_tables(
		wmi_unified_t wmi_handle, uint8_t *evt_buf,
		struct wlan_psoc_host_chainmask_table *chainmask_table)
{
	if (wmi_handle->ops->extract_chainmask_tables)
		return wmi_handle->ops->extract_chainmask_tables(wmi_handle,
				evt_buf, chainmask_table);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_set_country_cmd_send(wmi_unified_t wmi_handle,
					    struct set_country *param)
{
	if (wmi_handle->ops->send_set_country_cmd)
		return wmi_handle->ops->send_set_country_cmd(wmi_handle,
				  param);

	return QDF_STATUS_E_FAILURE;
}

#ifdef WLAN_FEATURE_ACTION_OUI
QDF_STATUS
wmi_unified_send_action_oui_cmd(wmi_unified_t wmi_handle,
				struct action_oui_request *req)
{
	if (wmi_handle->ops->send_action_oui_cmd)
		return wmi_handle->ops->send_action_oui_cmd(wmi_handle,
							    req);

	return QDF_STATUS_E_FAILURE;
}
#endif

QDF_STATUS wmi_unified_send_dump_wds_table_cmd(wmi_unified_t wmi_handle)
{
	if (wmi_handle->ops->send_wds_entry_list_cmd)
		return wmi_handle->ops->send_wds_entry_list_cmd(wmi_handle);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_wds_entry(wmi_unified_t wmi_handle, uint8_t *evt_buf,
		      struct wdsentry *wds_entry,
		      u_int32_t idx)
{
	if (wmi_handle->ops->extract_wds_entry)
		return wmi_handle->ops->extract_wds_entry(wmi_handle,
						evt_buf, wds_entry, idx);

	return QDF_STATUS_E_FAILURE;
}
qdf_export_symbol(wmi_extract_wds_entry);

QDF_STATUS wmi_unified_send_obss_detection_cfg_cmd(
		wmi_unified_t wmi_handle,
		struct wmi_obss_detection_cfg_param *obss_cfg_param)
{
	if (wmi_handle->ops->send_obss_detection_cfg_cmd)
		return wmi_handle->ops->send_obss_detection_cfg_cmd(wmi_handle,
				obss_cfg_param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_extract_obss_detection_info(
			wmi_unified_t wmi_handle,
			uint8_t *data,
			struct wmi_obss_detect_info *info)
{
	if (wmi_handle->ops->extract_obss_detection_info)
		return wmi_handle->ops->extract_obss_detection_info(data, info);

	return QDF_STATUS_E_FAILURE;
}

#if defined(WLAN_SUPPORT_FILS) || defined(CONFIG_BAND_6GHZ)
QDF_STATUS
wmi_unified_vdev_fils_enable_cmd_send(struct wmi_unified *wmi_handle,
				      struct config_fils_params *param)
{
	if (wmi_handle->ops->send_vdev_fils_enable_cmd)
		return wmi_handle->ops->send_vdev_fils_enable_cmd(
							wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}
#endif

#ifdef WLAN_SUPPORT_GREEN_AP
QDF_STATUS wmi_extract_green_ap_egap_status_info(
	wmi_unified_t wmi_handle, uint8_t *evt_buf,
	struct wlan_green_ap_egap_status_info *egap_status_info_params)
{
	if (wmi_handle->ops->extract_green_ap_egap_status_info)
		return wmi_handle->ops->extract_green_ap_egap_status_info(
				evt_buf, egap_status_info_params);

	return QDF_STATUS_E_FAILURE;
}
#endif

wmi_host_channel_width wmi_get_ch_width_from_phy_mode(
	wmi_unified_t wmi_handle, WMI_HOST_WLAN_PHY_MODE phymode)
{
	/*
	 * this API does translation between host only strcutres, hence
	 * does not need separate TLV, non-TLV definitions
	 */

	if (phymode >= WMI_HOST_MODE_11A && phymode < WMI_HOST_MODE_MAX)
		return mode_to_width[phymode];
	else
		return WMI_HOST_CHAN_WIDTH_20;
}

#ifdef QCA_SUPPORT_CP_STATS
QDF_STATUS wmi_extract_cca_stats(wmi_unified_t wmi_handle, void *evt_buf,
				 struct wmi_host_congestion_stats *stats)
{
	if (wmi_handle->ops->extract_cca_stats)
		return wmi_handle->ops->extract_cca_stats(wmi_handle, evt_buf,
							  stats);

	return QDF_STATUS_E_FAILURE;
}
#endif /* QCA_SUPPORT_CP_STATS */

#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(HOST_DFS_SPOOF_TEST)
QDF_STATUS
wmi_unified_dfs_send_avg_params_cmd(wmi_unified_t wmi_handle,
				    struct dfs_radar_found_params *params)
{
	if (wmi_handle->ops->send_dfs_average_radar_params_cmd)
		return wmi_handle->ops->send_dfs_average_radar_params_cmd(
			wmi_handle, params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_dfs_status_from_fw(wmi_unified_t wmi_handle, void *evt_buf,
			       uint32_t *dfs_status_check)
{
	if (wmi_handle->ops->extract_dfs_status_from_fw)
		return wmi_handle->ops->extract_dfs_status_from_fw(wmi_handle,
				evt_buf, dfs_status_check);

	return QDF_STATUS_E_FAILURE;
}
#endif

#ifdef OL_ATH_SMART_LOGGING
QDF_STATUS wmi_unified_send_smart_logging_enable_cmd(wmi_unified_t wmi_handle,
						     uint32_t param)
{
	if (wmi_handle->ops->send_smart_logging_enable_cmd)
		return wmi_handle->ops->send_smart_logging_enable_cmd(
				wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_send_smart_logging_fatal_cmd(wmi_unified_t wmi_handle,
					 struct wmi_debug_fatal_events *param)
{
	if (wmi_handle->ops->send_smart_logging_fatal_cmd)
		return wmi_handle->ops->send_smart_logging_fatal_cmd(wmi_handle,
			param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_smartlog_ev(wmi_unified_t wmi_handle,
				   void *evt_buf,
				   struct wmi_debug_fatal_events *ev)
{
	if (wmi_handle->ops->extract_smartlog_event)
		return wmi_handle->ops->extract_smartlog_event(
				wmi_handle, evt_buf, ev);

	return QDF_STATUS_E_FAILURE;
}

qdf_export_symbol(wmi_extract_smartlog_ev);
#endif /* OL_ATH_SMART_LOGGING */

QDF_STATUS
wmi_unified_send_roam_scan_stats_cmd(wmi_unified_t wmi_handle,
				     struct wmi_roam_scan_stats_req *params)
{
	if (wmi_handle->ops->send_roam_scan_stats_cmd)
		return wmi_handle->ops->send_roam_scan_stats_cmd(wmi_handle,
								 params);

	return QDF_STATUS_E_FAILURE;
}

#ifdef CRYPTO_SET_KEY_CONVERGED
uint8_t wlan_crypto_cipher_to_wmi_cipher(
		enum wlan_crypto_cipher_type crypto_cipher)
{
	switch (crypto_cipher) {
	case WLAN_CRYPTO_CIPHER_NONE:
		return WMI_CIPHER_NONE;
	case WLAN_CRYPTO_CIPHER_WEP:
	case WLAN_CRYPTO_CIPHER_WEP_40:
	case WLAN_CRYPTO_CIPHER_WEP_104:
		return WMI_CIPHER_WEP;
	case WLAN_CRYPTO_CIPHER_TKIP:
		return WMI_CIPHER_TKIP;
	case WLAN_CRYPTO_CIPHER_WAPI_SMS4:
	case WLAN_CRYPTO_CIPHER_WAPI_GCM4:
		return WMI_CIPHER_WAPI;
	case WLAN_CRYPTO_CIPHER_AES_CCM:
	case WLAN_CRYPTO_CIPHER_AES_CCM_256:
		return WMI_CIPHER_AES_CCM;
	case WLAN_CRYPTO_CIPHER_AES_CMAC:
		return WMI_CIPHER_AES_CMAC;
	case WLAN_CRYPTO_CIPHER_AES_GMAC:
	case WLAN_CRYPTO_CIPHER_AES_GMAC_256:
		return WMI_CIPHER_AES_GMAC;
	case WLAN_CRYPTO_CIPHER_AES_GCM:
	case WLAN_CRYPTO_CIPHER_AES_GCM_256:
		return WMI_CIPHER_AES_GCM;
	default:
		return 0;
	}
}

enum cdp_sec_type wlan_crypto_cipher_to_cdp_sec_type(
		enum wlan_crypto_cipher_type crypto_cipher)
{
	switch (crypto_cipher) {
	case WLAN_CRYPTO_CIPHER_NONE:
		return cdp_sec_type_none;
	case WLAN_CRYPTO_CIPHER_WEP:
	case WLAN_CRYPTO_CIPHER_WEP_40:
	case WLAN_CRYPTO_CIPHER_WEP_104:
		return cdp_sec_type_wep104;
	case WLAN_CRYPTO_CIPHER_TKIP:
		return cdp_sec_type_tkip;
	case WLAN_CRYPTO_CIPHER_WAPI_SMS4:
	case WLAN_CRYPTO_CIPHER_WAPI_GCM4:
		return cdp_sec_type_wapi;
	case WLAN_CRYPTO_CIPHER_AES_CCM:
		return cdp_sec_type_aes_ccmp;
	case WLAN_CRYPTO_CIPHER_AES_CCM_256:
		return cdp_sec_type_aes_ccmp_256;
	case WLAN_CRYPTO_CIPHER_AES_GCM:
		return cdp_sec_type_aes_gcmp;
	case WLAN_CRYPTO_CIPHER_AES_GCM_256:
		return cdp_sec_type_aes_gcmp_256;
	default:
		return cdp_sec_type_none;
	}
}
#endif /* CRYPTO_SET_KEY_CONVERGED */

QDF_STATUS
wmi_extract_roam_scan_stats_res_evt(wmi_unified_t wmi, void *evt_buf,
				    uint32_t *vdev_id,
				    struct wmi_roam_scan_stats_res **res_param)
{
	if (wmi->ops->extract_roam_scan_stats_res_evt)
		return wmi->ops->extract_roam_scan_stats_res_evt(wmi,
							evt_buf,
							vdev_id, res_param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_offload_bcn_tx_status_evt(wmi_unified_t wmi_handle, void *evt_buf,
				      uint32_t *vdev_id, uint32_t *tx_status)
{
	if (wmi_handle->ops->extract_offload_bcn_tx_status_evt)
		return wmi_handle->ops->extract_offload_bcn_tx_status_evt(
						wmi_handle, evt_buf,
						vdev_id, tx_status);

	return QDF_STATUS_E_FAILURE;
}

#ifdef OBSS_PD
QDF_STATUS wmi_unified_send_obss_spatial_reuse_set_cmd(
	wmi_unified_t wmi_handle,
	struct wmi_host_obss_spatial_reuse_set_param *obss_spatial_reuse_param)
{
	if (wmi_handle->ops->send_obss_spatial_reuse_set)
		return wmi_handle->ops->send_obss_spatial_reuse_set(wmi_handle,
				obss_spatial_reuse_param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_send_obss_spatial_reuse_set_def_thresh_cmd(
		wmi_unified_t wmi_handle,
		struct wmi_host_obss_spatial_reuse_set_def_thresh *thresh)
{
	if (wmi_handle->ops->send_obss_spatial_reuse_set_def_thresh)
		return wmi_handle->ops->send_obss_spatial_reuse_set_def_thresh(
						wmi_handle, thresh);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_send_self_srg_bss_color_bitmap_set_cmd(
	wmi_unified_t wmi_handle,  uint32_t bitmap_0,
	uint32_t bitmap_1, uint8_t pdev_id)
{
	if (wmi_handle->ops->send_self_srg_bss_color_bitmap_set)
		return wmi_handle->ops->send_self_srg_bss_color_bitmap_set(
				wmi_handle, bitmap_0, bitmap_1, pdev_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_send_self_srg_partial_bssid_bitmap_set_cmd(
	wmi_unified_t wmi_handle,  uint32_t bitmap_0,
	uint32_t bitmap_1, uint8_t pdev_id)
{
	if (wmi_handle->ops->send_self_srg_partial_bssid_bitmap_set)
		return wmi_handle->ops->send_self_srg_partial_bssid_bitmap_set(
				wmi_handle, bitmap_0, bitmap_1, pdev_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_send_self_srg_obss_color_enable_bitmap_cmd(
	wmi_unified_t wmi_handle,  uint32_t bitmap_0,
	uint32_t bitmap_1, uint8_t pdev_id)
{
	if (wmi_handle->ops->send_self_srg_obss_color_enable_bitmap)
		return wmi_handle->ops->
			send_self_srg_obss_color_enable_bitmap(
				wmi_handle, bitmap_0, bitmap_1, pdev_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_send_self_srg_obss_bssid_enable_bitmap_cmd(
	wmi_unified_t wmi_handle,  uint32_t bitmap_0,
	uint32_t bitmap_1, uint8_t pdev_id)
{
	if (wmi_handle->ops->send_self_srg_obss_bssid_enable_bitmap)
		return wmi_handle->ops->
			send_self_srg_obss_bssid_enable_bitmap(
				wmi_handle, bitmap_0, bitmap_1, pdev_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_send_self_non_srg_obss_color_enable_bitmap_cmd(
	wmi_unified_t wmi_handle,  uint32_t bitmap_0,
	uint32_t bitmap_1, uint8_t pdev_id)
{
	if (wmi_handle->ops->send_self_non_srg_obss_color_enable_bitmap)
		return wmi_handle->ops->
			send_self_non_srg_obss_color_enable_bitmap(
				wmi_handle, bitmap_0, bitmap_1, pdev_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_send_self_non_srg_obss_bssid_enable_bitmap_cmd(
	wmi_unified_t wmi_handle,  uint32_t bitmap_0,
	uint32_t bitmap_1, uint8_t pdev_id)
{
	if (wmi_handle->ops->send_self_non_srg_obss_bssid_enable_bitmap)
		return wmi_handle->ops->
			send_self_non_srg_obss_bssid_enable_bitmap(
				wmi_handle, bitmap_0, bitmap_1, pdev_id);

	return QDF_STATUS_E_FAILURE;
}
#endif

QDF_STATUS wmi_convert_pdev_id_host_to_target(wmi_unified_t wmi_handle,
					      uint32_t host_pdev_id,
					      uint32_t *target_pdev_id)
{
	if (wmi_handle->ops->convert_pdev_id_host_to_target) {
		*target_pdev_id =
			wmi_handle->ops->convert_pdev_id_host_to_target(
					wmi_handle,
					host_pdev_id);
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_FAILURE;
}

#ifndef CNSS_GENL
QDF_STATUS wmi_convert_pdev_id_target_to_host(wmi_unified_t wmi_handle,
					      uint32_t target_pdev_id,
					      uint32_t *host_pdev_id)
{
	if (wmi_handle->ops->convert_pdev_id_target_to_host) {
		*host_pdev_id =
			wmi_handle->ops->convert_pdev_id_target_to_host(
					wmi_handle,
					target_pdev_id);
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_FAILURE;
}
#endif

QDF_STATUS
wmi_unified_send_bss_color_change_enable_cmd(wmi_unified_t wmi_handle,
					     uint32_t vdev_id,
					     bool enable)
{
	if (wmi_handle->ops->send_bss_color_change_enable_cmd)
		return wmi_handle->ops->send_bss_color_change_enable_cmd(
				wmi_handle, vdev_id, enable);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_send_obss_color_collision_cfg_cmd(
		wmi_unified_t wmi_handle,
		struct wmi_obss_color_collision_cfg_param *cfg)
{
	if (wmi_handle->ops->send_obss_color_collision_cfg_cmd)
		return wmi_handle->ops->send_obss_color_collision_cfg_cmd(
				wmi_handle, cfg);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_extract_obss_color_collision_info(
		wmi_unified_t wmi_handle,
		uint8_t *data, struct wmi_obss_color_collision_info *info)
{
	if (wmi_handle->ops->extract_obss_color_collision_info)
		return wmi_handle->ops->extract_obss_color_collision_info(data,
									  info);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_send_mws_coex_req_cmd(struct wmi_unified *wmi_handle,
					     uint32_t vdev_id,
					     uint32_t cmd_id)
{

	if (wmi_handle->ops->send_mws_coex_status_req_cmd)
		return wmi_handle->ops->send_mws_coex_status_req_cmd(wmi_handle,
				vdev_id, cmd_id);

	return QDF_STATUS_E_FAILURE;
}

#ifdef WIFI_POS_CONVERGED
QDF_STATUS
wmi_extract_oem_response_param(wmi_unified_t wmi_hdl, void *resp_buf,
			       struct wmi_oem_response_param *oem_resp_param)
{
	if (wmi_hdl->ops->extract_oem_response_param)
		return wmi_hdl->ops->extract_oem_response_param(wmi_hdl,
								resp_buf,
								oem_resp_param);

	return QDF_STATUS_E_FAILURE;
}
#endif /* WIFI_POS_CONVERGED */

QDF_STATUS wmi_unified_extract_hw_mode_resp(wmi_unified_t wmi,
					    void *evt_buf,
					    uint32_t *cmd_status)
{
	if (wmi->ops->extract_hw_mode_resp_event)
		return wmi->ops->extract_hw_mode_resp_event(wmi,
							    evt_buf,
							    cmd_status);

	return QDF_STATUS_E_FAILURE;
}

#ifdef FEATURE_ANI_LEVEL_REQUEST
QDF_STATUS wmi_unified_ani_level_cmd_send(wmi_unified_t wmi_handle,
					  uint32_t *freqs,
					  uint8_t num_freqs)
{
	if (wmi_handle->ops->send_ani_level_cmd)
		return wmi_handle->ops->send_ani_level_cmd(wmi_handle, freqs,
							   num_freqs);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_extract_ani_level(wmi_unified_t wmi_handle,
					 uint8_t *data,
					 struct wmi_host_ani_level_event **info,
					 uint32_t *num_channels)
{
	if (wmi_handle->ops->extract_ani_level)
		return wmi_handle->ops->extract_ani_level(data, info,
							  num_channels);

	return QDF_STATUS_E_FAILURE;
}
#endif /* FEATURE_ANI_LEVEL_REQUEST */

QDF_STATUS
wmi_unified_extract_roam_trigger_stats(wmi_unified_t wmi,
				       void *evt_buf,
				       struct wmi_roam_trigger_info *trig,
				       uint8_t idx)
{
	if (wmi->ops->extract_roam_trigger_stats)
		return wmi->ops->extract_roam_trigger_stats(wmi, evt_buf, trig,
							    idx);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_extract_roam_scan_stats(wmi_unified_t wmi, void *evt_buf,
				    struct wmi_roam_scan_data *dst, uint8_t idx,
				    uint8_t chan_idx, uint8_t ap_idx)
{
	if (wmi->ops->extract_roam_scan_stats)
		return wmi->ops->extract_roam_scan_stats(wmi, evt_buf, dst,
							 idx, chan_idx, ap_idx);

	return QDF_STATUS_E_FAILURE;
}

#ifdef WLAN_FEATURE_PKT_CAPTURE
QDF_STATUS
wmi_unified_extract_vdev_mgmt_offload_event(
				wmi_unified_t wmi, void *evt_buf,
				struct mgmt_offload_event_params *params)
{
	if (wmi->ops->extract_vdev_mgmt_offload_event)
		return wmi->ops->extract_vdev_mgmt_offload_event(wmi, evt_buf,
								 params);

	return QDF_STATUS_E_FAILURE;
}
#endif /* WLAN_FEATURE_PKT_CAPTURE */

QDF_STATUS
wmi_unified_extract_roam_result_stats(wmi_unified_t wmi, void *buf,
				      struct wmi_roam_result *dst,
				      uint8_t idx)
{
	if (wmi->ops->extract_roam_result_stats)
		return wmi->ops->extract_roam_result_stats(wmi, buf, dst, idx);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_extract_roam_11kv_stats(wmi_unified_t wmi, void *evt_buf,
				    struct wmi_neighbor_report_data *dst,
				    uint8_t idx, uint8_t rpt_idx)
{
	if (wmi->ops->extract_roam_11kv_stats)
		return wmi->ops->extract_roam_11kv_stats(wmi, evt_buf, dst, idx,
							 rpt_idx);

	return QDF_STATUS_E_FAILURE;
}

#ifdef FEATURE_WLAN_TIME_SYNC_FTM
QDF_STATUS wmi_unified_send_wlan_time_sync_ftm_trigger(wmi_unified_t wmi_handle,
						       uint32_t vdev_id,
						       bool burst_mode)
{
	if (wmi_handle->ops->send_wlan_time_sync_ftm_trigger_cmd)
		return wmi_handle->ops->send_wlan_time_sync_ftm_trigger_cmd
				(wmi_handle, vdev_id, burst_mode);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_send_wlan_time_sync_qtime(wmi_unified_t wmi_handle,
						 uint32_t vdev_id,
						 uint64_t lpass_ts)
{
	if (wmi_handle->ops->send_wlan_ts_qtime_cmd)
		return wmi_handle->ops->send_wlan_ts_qtime_cmd(wmi_handle,
					vdev_id, lpass_ts);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_extract_time_sync_ftm_start_stop_params(
				wmi_unified_t wmi_handle, void *evt_buf,
				struct ftm_time_sync_start_stop_params *param)
{
	if (wmi_handle->ops->extract_time_sync_ftm_start_stop_event)
		return
		wmi_handle->ops->extract_time_sync_ftm_start_stop_event(
						wmi_handle, evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_extract_time_sync_ftm_offset(
			wmi_unified_t wmi_handle, void *evt_buf,
			struct ftm_time_sync_offset *param)
{
	if (wmi_handle->ops->extract_time_sync_ftm_offset_event)
		return
		wmi_handle->ops->extract_time_sync_ftm_offset_event(
						wmi_handle, evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}
#endif /* FEATURE_WLAN_TIME_SYNC_FTM */

QDF_STATUS
wmi_unified_send_injector_frame_config_cmd(wmi_unified_t wmi_handle,
				 struct wmi_host_injector_frame_params *param)
{
	if (wmi_handle->ops->send_injector_config_cmd) {
		return wmi_handle->ops->send_injector_config_cmd(wmi_handle,
			param);
	}

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_send_cp_stats_cmd(wmi_unified_t wmi_handle,
					 void *buf_ptr, uint32_t buf_len)
{
	if (wmi_handle->ops->send_cp_stats_cmd)
		return wmi_handle->ops->send_cp_stats_cmd(wmi_handle, buf_ptr,
							  buf_len);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_extract_cp_stats_more_pending(wmi_unified_t wmi_handle,
					  void *evt_buf, uint32_t *more_flag)
{
	if (wmi_handle->ops->extract_cp_stats_more_pending)
		return wmi_handle->ops->extract_cp_stats_more_pending(wmi_handle,
								      evt_buf,
								      more_flag);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_send_vdev_tsf_tstamp_action_cmd() - send vdev tsf action command
 * @wmi: wmi handle
 * @vdev_id: vdev id
 *
 * TSF_TSTAMP_READ_VALUE is the only operation supported
 * Return: QDF_STATUS_SUCCESS for success or erro code
 */
QDF_STATUS wmi_unified_send_vdev_tsf_tstamp_action_cmd(wmi_unified_t wmi_hdl,
						       uint8_t vdev_id)
{
	wmi_unified_t wmi_handle = wmi_hdl;

	if (wmi_handle->ops->send_vdev_tsf_tstamp_action_cmd)
		return wmi_handle->ops->send_vdev_tsf_tstamp_action_cmd(
						wmi_handle, vdev_id);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_vdev_tsf_report_event() - extract vdev tsf report from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @wmi_host_tsf_event param: Pointer to hold event info
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_vdev_tsf_report_event(wmi_unified_t wmi_hdl,
					     uint8_t *evt_buf,
					     struct wmi_host_tsf_event *param)
{
	wmi_unified_t wmi_handle = wmi_hdl;

	if (wmi_handle->ops->extract_vdev_tsf_report_event)
		return wmi_handle->ops->extract_vdev_tsf_report_event(
						wmi_handle, evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_pdev_csa_switch_count_status(
		wmi_unified_t wmi_handle,
		void *evt_buf,
		struct pdev_csa_switch_count_status *param)
{
	if (wmi_handle->ops->extract_pdev_csa_switch_count_status)
		return wmi_handle->ops->extract_pdev_csa_switch_count_status(
				wmi_handle,
				evt_buf,
				param);

	return QDF_STATUS_E_FAILURE;
}
