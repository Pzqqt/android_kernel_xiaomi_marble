/*
 * Copyright (c) 2013-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/*
 * This file contains the API definitions for the Unified Wireless
 * Module Interface (WMI).
 */
#ifndef _WMI_UNIFIED_PRIV_H_
#define _WMI_UNIFIED_PRIV_H_
#include <osdep.h>
#include "a_types.h"
#include "wmi.h"
#include "wmi_unified.h"
#include "cdf_atomic.h"

#define WMI_UNIFIED_MAX_EVENT 0x100
#define WMI_MAX_CMDS  1024

typedef cdf_nbuf_t wmi_buf_t;

#ifdef WMI_INTERFACE_EVENT_LOGGING

#define WMI_EVENT_DEBUG_MAX_ENTRY (1024)

struct wmi_command_debug {
	uint32_t command;
	uint32_t data[4]; /*16 bytes of WMI cmd excluding TLV and WMI headers */
	uint64_t time;
};

struct wmi_event_debug {
	uint32_t event;
	uint32_t data[4]; /*16 bytes of WMI event data excluding TLV header */
	uint64_t time;
};

#endif /*WMI_INTERFACE_EVENT_LOGGING */

#ifdef WLAN_OPEN_SOURCE
struct fwdebug {
	struct sk_buff_head fwlog_queue;
	struct completion fwlog_completion;
	A_BOOL fwlog_open;
};
#endif /* WLAN_OPEN_SOURCE */

struct wmi_ops {
CDF_STATUS (*send_vdev_create_cmd)(wmi_unified_t wmi_handle,
				 uint8_t macaddr[IEEE80211_ADDR_LEN],
				 struct vdev_create_params *param);

CDF_STATUS (*send_vdev_delete_cmd)(wmi_unified_t wmi_handle,
					  uint8_t if_id);

CDF_STATUS (*send_vdev_stop_cmd)(wmi_unified_t wmi,
					uint8_t vdev_id);

CDF_STATUS (*send_vdev_down_cmd)(wmi_unified_t wmi,
			uint8_t vdev_id);

CDF_STATUS (*send_peer_flush_tids_cmd)(wmi_unified_t wmi,
					 uint8_t peer_addr[IEEE80211_ADDR_LEN],
					 struct peer_flush_params *param);

CDF_STATUS (*send_peer_delete_cmd)(wmi_unified_t wmi,
				    uint8_t peer_addr[IEEE80211_ADDR_LEN],
				    uint8_t vdev_id);

CDF_STATUS (*send_peer_param_cmd)(wmi_unified_t wmi,
				uint8_t peer_addr[IEEE80211_ADDR_LEN],
				struct peer_set_params *param);

CDF_STATUS (*send_vdev_up_cmd)(wmi_unified_t wmi,
			     uint8_t bssid[IEEE80211_ADDR_LEN],
				 struct vdev_up_params *params);

CDF_STATUS (*send_peer_create_cmd)(wmi_unified_t wmi,
					struct peer_create_params *param);

CDF_STATUS (*send_green_ap_ps_cmd)(wmi_unified_t wmi_handle,
						uint32_t value, uint8_t mac_id);

CDF_STATUS
(*send_pdev_utf_cmd)(wmi_unified_t wmi_handle,
				struct pdev_utf_params *param,
				uint8_t mac_id);

CDF_STATUS
(*send_pdev_param_cmd)(wmi_unified_t wmi_handle,
			   struct pdev_params *param,
				uint8_t mac_id);

CDF_STATUS (*send_suspend_cmd)(wmi_unified_t wmi_handle,
				struct suspend_params *param,
				uint8_t mac_id);

CDF_STATUS (*send_resume_cmd)(wmi_unified_t wmi_handle,
				uint8_t mac_id);

CDF_STATUS (*send_wow_enable_cmd)(wmi_unified_t wmi_handle,
				struct wow_cmd_params *param,
				uint8_t mac_id);

CDF_STATUS (*send_set_ap_ps_param_cmd)(wmi_unified_t wmi_handle,
					   uint8_t *peer_addr,
					   struct ap_ps_params *param);

CDF_STATUS (*send_set_sta_ps_param_cmd)(wmi_unified_t wmi_handle,
					   struct sta_ps_params *param);

CDF_STATUS (*send_crash_inject_cmd)(wmi_unified_t wmi_handle,
			 struct crash_inject *param);

CDF_STATUS
(*send_dbglog_cmd)(wmi_unified_t wmi_handle,
				struct dbglog_params *dbglog_param);

CDF_STATUS (*send_vdev_set_param_cmd)(wmi_unified_t wmi_handle,
				struct vdev_set_params *param);

CDF_STATUS (*send_stats_request_cmd)(wmi_unified_t wmi_handle,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct stats_request_params *param);

CDF_STATUS (*send_packet_log_enable_cmd)(wmi_unified_t wmi_handle,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct packet_enable_params *param);

CDF_STATUS (*send_beacon_send_cmd)(wmi_unified_t wmi_handle,
				struct beacon_params *param);

CDF_STATUS (*send_peer_assoc_cmd)(wmi_unified_t wmi_handle,
				struct peer_assoc_params *param);

CDF_STATUS (*send_scan_start_cmd)(wmi_unified_t wmi_handle,
				struct scan_start_params *param);

CDF_STATUS (*send_scan_stop_cmd)(wmi_unified_t wmi_handle,
				struct scan_stop_params *param);

CDF_STATUS (*send_scan_chan_list_cmd)(wmi_unified_t wmi_handle,
				struct scan_chan_list_params *param);

CDF_STATUS (*send_mgmt_cmd)(wmi_unified_t wmi_handle,
				struct wmi_mgmt_params *param);

CDF_STATUS (*send_modem_power_state_cmd)(wmi_unified_t wmi_handle,
		uint32_t param_value);

CDF_STATUS (*send_set_sta_ps_mode_cmd)(wmi_unified_t wmi_handle,
			       uint32_t vdev_id, uint8_t val);

CDF_STATUS (*send_get_temperature_cmd)(wmi_unified_t wmi_handle);

CDF_STATUS (*send_set_p2pgo_oppps_req_cmd)(wmi_unified_t wmi_handle,
		struct p2p_ps_params *oppps);

CDF_STATUS (*send_set_p2pgo_noa_req_cmd)(wmi_unified_t wmi_handle,
			struct p2p_ps_params *noa);

CDF_STATUS (*send_set_smps_params_cmd)(wmi_unified_t wmi_handle,
			  uint8_t vdev_id,
			  int value);

CDF_STATUS (*send_set_mimops_cmd)(wmi_unified_t wmi_handle,
			uint8_t vdev_id, int value);

CDF_STATUS (*send_set_sta_uapsd_auto_trig_cmd)(wmi_unified_t wmi_handle,
				struct sta_uapsd_trig_params *param);

CDF_STATUS (*send_ocb_set_utc_time_cmd)(wmi_unified_t wmi_handle,
				struct ocb_utc_param *utc);

CDF_STATUS (*send_ocb_get_tsf_timer_cmd)(wmi_unified_t wmi_handle,
			  uint8_t vdev_id);

CDF_STATUS (*send_ocb_start_timing_advert_cmd)(wmi_unified_t wmi_handle,
	struct ocb_timing_advert_param *timing_advert);

CDF_STATUS (*send_ocb_stop_timing_advert_cmd)(wmi_unified_t wmi_handle,
	struct ocb_timing_advert_param *timing_advert);

CDF_STATUS (*send_dcc_get_stats_cmd)(wmi_unified_t wmi_handle,
		     struct dcc_get_stats_param *get_stats_param);

CDF_STATUS (*send_dcc_clear_stats_cmd)(wmi_unified_t wmi_handle,
				uint32_t vdev_id, uint32_t dcc_stats_bitmap);

CDF_STATUS (*send_dcc_update_ndl_cmd)(wmi_unified_t wmi_handle,
		       struct dcc_update_ndl_param *update_ndl_param);

CDF_STATUS (*send_ocb_set_config_cmd)(wmi_unified_t wmi_handle,
		  struct ocb_config_param *config, uint32_t *ch_mhz);

CDF_STATUS (*send_lro_config_cmd)(wmi_unified_t wmi_handle,
	 struct wmi_lro_config_cmd_t *wmi_lro_cmd);

CDF_STATUS (*send_set_thermal_mgmt_cmd)(wmi_unified_t wmi_handle,
				struct thermal_cmd_params *thermal_info);

CDF_STATUS (*send_set_mcc_channel_time_quota_cmd)
	(wmi_unified_t wmi_handle,
	uint32_t adapter_1_chan_freq,
	uint32_t adapter_1_quota, uint32_t adapter_2_chan_freq);

CDF_STATUS (*send_set_mcc_channel_time_latency_cmd)
	(wmi_unified_t wmi_handle,
	uint32_t mcc_channel_freq, uint32_t mcc_channel_time_latency);

CDF_STATUS (*send_set_enable_disable_mcc_adaptive_scheduler_cmd)(
		   wmi_unified_t wmi_handle, uint32_t mcc_adaptive_scheduler);

CDF_STATUS (*send_p2p_go_set_beacon_ie_cmd)(wmi_unified_t wmi_handle,
				    A_UINT32 vdev_id, uint8_t *p2p_ie);

CDF_STATUS (*send_probe_rsp_tmpl_send_cmd)(wmi_unified_t wmi_handle,
			     uint8_t vdev_id,
			     struct wmi_probe_resp_params *probe_rsp_info,
			     uint8_t *frm);

CDF_STATUS (*send_process_update_edca_param_cmd)(wmi_unified_t wmi_handle,
			     uint8_t vdev_id,
			     wmi_wmm_vparams gwmm_param[WMI_MAX_NUM_AC]);

CDF_STATUS (*send_vdev_set_gtx_cfg_cmd)(wmi_unified_t wmi_handle,
				  uint32_t if_id,
				  struct wmi_gtx_config *gtx_info);

CDF_STATUS (*send_set_sta_keep_alive_cmd)(wmi_unified_t wmi_handle,
				struct sta_params *params);

CDF_STATUS (*send_set_sta_sa_query_param_cmd)(wmi_unified_t wmi_handle,
				       uint8_t vdev_id, uint32_t max_retries,
					   uint32_t retry_interval);

CDF_STATUS (*send_bcn_buf_ll_cmd)(wmi_unified_t wmi_handle,
			wmi_bcn_send_from_host_cmd_fixed_param *param);

CDF_STATUS (*send_set_gateway_params_cmd)(wmi_unified_t wmi_handle,
					struct gateway_update_req_param *req);

CDF_STATUS (*send_set_rssi_monitoring_cmd)(wmi_unified_t wmi_handle,
					struct rssi_monitor_param *req);

CDF_STATUS (*send_scan_probe_setoui_cmd)(wmi_unified_t wmi_handle,
			  struct scan_mac_oui *psetoui);

CDF_STATUS (*send_reset_passpoint_network_list_cmd)(wmi_unified_t wmi_handle,
					struct wifi_passpoint_req_param *req);

CDF_STATUS (*send_set_passpoint_network_list_cmd)(wmi_unified_t wmi_handle,
					struct wifi_passpoint_req_param *req);

CDF_STATUS (*send_set_epno_network_list_cmd)(wmi_unified_t wmi_handle,
		struct wifi_enhanched_pno_params *req);

CDF_STATUS (*send_extscan_get_capabilities_cmd)(wmi_unified_t wmi_handle,
			  struct extscan_capabilities_params *pgetcapab);

CDF_STATUS (*send_extscan_get_cached_results_cmd)(wmi_unified_t wmi_handle,
			  struct extscan_cached_result_params *pcached_results);

CDF_STATUS (*send_extscan_stop_change_monitor_cmd)(wmi_unified_t wmi_handle,
			  struct extscan_capabilities_reset_params *reset_req);

CDF_STATUS (*send_extscan_start_change_monitor_cmd)(wmi_unified_t wmi_handle,
		struct extscan_set_sig_changereq_params *
		psigchange);

CDF_STATUS (*send_extscan_stop_hotlist_monitor_cmd)(wmi_unified_t wmi_handle,
		struct extscan_bssid_hotlist_reset_params *photlist_reset);

CDF_STATUS (*send_stop_extscan_cmd)(wmi_unified_t wmi_handle,
		  struct extscan_stop_req_params *pstopcmd);

CDF_STATUS (*send_start_extscan_cmd)(wmi_unified_t wmi_handle,
		    struct wifi_scan_cmd_req_params *pstart);

CDF_STATUS (*send_plm_stop_cmd)(wmi_unified_t wmi_handle,
		 const struct plm_req_params *plm);

CDF_STATUS (*send_plm_start_cmd)(wmi_unified_t wmi_handle,
		  const struct plm_req_params *plm,
		  uint32_t *gchannel_list);

CDF_STATUS (*send_csa_offload_enable_cmd)(wmi_unified_t wmi_handle,
			uint8_t vdev_id);

CDF_STATUS (*send_pno_stop_cmd)(wmi_unified_t wmi_handle, uint8_t vdev_id);

CDF_STATUS (*send_pno_start_cmd)(wmi_unified_t wmi_handle,
		   struct pno_scan_req_params *pno,
		   uint32_t *gchannel_freq_list);

CDF_STATUS (*send_ipa_offload_control_cmd)(wmi_unified_t wmi_handle,
		struct ipa_offload_control_params *ipa_offload);

CDF_STATUS (*send_set_ric_req_cmd)(wmi_unified_t wmi_handle, void *msg,
			uint8_t is_add_ts);

CDF_STATUS (*send_process_ll_stats_clear_cmd)
	   (wmi_unified_t wmi_handle,
	   const struct ll_stats_clear_params *clear_req,
	   uint8_t addr[IEEE80211_ADDR_LEN]);

CDF_STATUS (*send_process_ll_stats_set_cmd)
	(wmi_unified_t wmi_handle, const struct ll_stats_set_params *set_req);

CDF_STATUS (*send_process_ll_stats_get_cmd)
	(wmi_unified_t wmi_handle, const struct ll_stats_get_params  *get_req,
		 uint8_t addr[IEEE80211_ADDR_LEN]);

CDF_STATUS (*send_get_stats_cmd)(wmi_unified_t wmi_handle,
		       struct pe_stats_req  *get_stats_param,
			   uint8_t addr[IEEE80211_ADDR_LEN]);

CDF_STATUS (*send_snr_request_cmd)(wmi_unified_t wmi_handle);

CDF_STATUS (*send_snr_cmd)(wmi_unified_t wmi_handle, uint8_t vdev_id);

CDF_STATUS (*send_link_status_req_cmd)(wmi_unified_t wmi_handle,
				 struct link_status_params *link_status);

CDF_STATUS (*send_lphb_config_hbenable_cmd)(wmi_unified_t wmi_handle,
				wmi_hb_set_enable_cmd_fixed_param *params);

CDF_STATUS (*send_lphb_config_tcp_params_cmd)(wmi_unified_t wmi_handle,
				    wmi_hb_set_tcp_params_cmd_fixed_param *lphb_conf_req);

CDF_STATUS (*send_lphb_config_tcp_pkt_filter_cmd)(wmi_unified_t wmi_handle,
				wmi_hb_set_tcp_pkt_filter_cmd_fixed_param *g_hb_tcp_filter_fp);

CDF_STATUS (*send_lphb_config_udp_params_cmd)(wmi_unified_t wmi_handle,
				    wmi_hb_set_udp_params_cmd_fixed_param *lphb_conf_req);

CDF_STATUS (*send_lphb_config_udp_pkt_filter_cmd)(wmi_unified_t wmi_handle,
					wmi_hb_set_udp_pkt_filter_cmd_fixed_param *lphb_conf_req);

CDF_STATUS (*send_process_dhcp_ind_cmd)(wmi_unified_t wmi_handle,
				wmi_peer_set_param_cmd_fixed_param *ta_dhcp_ind);

CDF_STATUS (*send_get_link_speed_cmd)(wmi_unified_t wmi_handle,
			wmi_mac_addr peer_macaddr);

CDF_STATUS (*send_egap_conf_params_cmd)(wmi_unified_t wmi_handle,
				     wmi_ap_ps_egap_param_cmd_fixed_param *egap_params);

CDF_STATUS (*send_fw_profiling_cmd)(wmi_unified_t wmi_handle,
			uint32_t cmd, uint32_t value1, uint32_t value2);

CDF_STATUS (*send_wow_sta_ra_filter_cmd)(wmi_unified_t wmi_handle,
				   uint8_t vdev_id, uint8_t default_pattern,
				   uint16_t rate_limit_interval);

CDF_STATUS (*send_nat_keepalive_en_cmd)(wmi_unified_t wmi_handle, uint8_t vdev_id);

CDF_STATUS (*send_start_oem_data_cmd)(wmi_unified_t wmi_handle,
			  uint8_t data_len,
			  uint8_t *data);

CDF_STATUS
(*send_dfs_phyerr_filter_offload_en_cmd)(wmi_unified_t wmi_handle,
			bool dfs_phyerr_filter_offload);

CDF_STATUS (*send_pktlog_wmi_send_cmd)(wmi_unified_t wmi_handle,
				   WMI_PKTLOG_EVENT pktlog_event,
				   WMI_CMD_ID cmd_id);

CDF_STATUS (*send_add_wow_wakeup_event_cmd)(wmi_unified_t wmi_handle,
					uint32_t vdev_id,
					uint32_t bitmap,
					bool enable);

CDF_STATUS (*send_wow_patterns_to_fw_cmd)(wmi_unified_t wmi_handle,
				uint8_t vdev_id, uint8_t ptrn_id,
				const uint8_t *ptrn, uint8_t ptrn_len,
				uint8_t ptrn_offset, const uint8_t *mask,
				uint8_t mask_len, bool user,
				uint8_t default_patterns);

CDF_STATUS (*send_wow_delete_pattern_cmd)(wmi_unified_t wmi_handle, uint8_t ptrn_id,
					uint8_t vdev_id);

CDF_STATUS (*send_host_wakeup_ind_to_fw_cmd)(wmi_unified_t wmi_handle);

CDF_STATUS (*send_del_ts_cmd)(wmi_unified_t wmi_handle, uint8_t vdev_id,
				uint8_t ac);

CDF_STATUS (*send_aggr_qos_cmd)(wmi_unified_t wmi_handle,
		      struct aggr_add_ts_param *aggr_qos_rsp_msg);

CDF_STATUS (*send_add_ts_cmd)(wmi_unified_t wmi_handle,
		 struct add_ts_param *msg);

CDF_STATUS (*send_enable_disable_packet_filter_cmd)(wmi_unified_t wmi_handle,
					uint8_t vdev_id, bool enable);

CDF_STATUS (*send_config_packet_filter_cmd)(wmi_unified_t wmi_handle,
		uint8_t vdev_id, struct rcv_pkt_filter_config *rcv_filter_param,
		uint8_t filter_id, bool enable);

CDF_STATUS (*send_add_clear_mcbc_filter_cmd)(wmi_unified_t wmi_handle,
				     uint8_t vdev_id,
				     struct cdf_mac_addr multicast_addr,
				     bool clearList);

CDF_STATUS (*send_gtk_offload_cmd)(wmi_unified_t wmi_handle, uint8_t vdev_id,
					   struct gtk_offload_params *params,
					   bool enable_offload,
					   uint32_t gtk_offload_opcode);

CDF_STATUS (*send_process_gtk_offload_getinfo_cmd)(wmi_unified_t wmi_handle,
				uint8_t vdev_id,
				uint64_t offload_req_opcode);

CDF_STATUS (*send_process_add_periodic_tx_ptrn_cmd)(wmi_unified_t wmi_handle,
						struct periodic_tx_pattern  *
						pAddPeriodicTxPtrnParams,
						uint8_t vdev_id);

CDF_STATUS (*send_process_del_periodic_tx_ptrn_cmd)(wmi_unified_t wmi_handle,
						uint8_t vdev_id,
						uint8_t pattern_id);

CDF_STATUS (*send_stats_ext_req_cmd)(wmi_unified_t wmi_handle,
			struct stats_ext_params *preq);

CDF_STATUS (*send_enable_ext_wow_cmd)(wmi_unified_t wmi_handle,
			struct ext_wow_params *params);

CDF_STATUS (*send_set_app_type2_params_in_fw_cmd)(wmi_unified_t wmi_handle,
					  struct app_type2_params *appType2Params);

CDF_STATUS (*send_set_auto_shutdown_timer_cmd)(wmi_unified_t wmi_handle,
						  uint32_t timer_val);

CDF_STATUS (*send_nan_req_cmd)(wmi_unified_t wmi_handle,
			struct nan_req_params *nan_req);

CDF_STATUS (*send_process_dhcpserver_offload_cmd)(wmi_unified_t wmi_handle,
				struct dhcp_offload_info_params *pDhcpSrvOffloadInfo);

CDF_STATUS (*send_process_ch_avoid_update_cmd)(wmi_unified_t wmi_handle);

CDF_STATUS (*send_regdomain_info_to_fw_cmd)(wmi_unified_t wmi_handle,
				   uint32_t reg_dmn, uint16_t regdmn2G,
				   uint16_t regdmn5G, int8_t ctl2G,
				   int8_t ctl5G);

CDF_STATUS (*send_set_tdls_offchan_mode_cmd)(wmi_unified_t wmi_handle,
			      struct tdls_channel_switch_params *chan_switch_params);

CDF_STATUS (*send_update_fw_tdls_state_cmd)(wmi_unified_t wmi_handle,
					 void *tdls_param, uint8_t tdls_state);

CDF_STATUS (*send_update_tdls_peer_state_cmd)(wmi_unified_t wmi_handle,
			       struct tdls_peer_state_params *peerStateParams,
				   uint32_t *ch_mhz);


CDF_STATUS (*send_process_fw_mem_dump_cmd)(wmi_unified_t wmi_handle,
					struct fw_dump_req_param *mem_dump_req);

CDF_STATUS (*send_process_set_ie_info_cmd)(wmi_unified_t wmi_handle,
				   struct vdev_ie_info_param *ie_info);

CDF_STATUS (*send_init_cmd)(wmi_unified_t wmi_handle,
		wmi_resource_config *res_cfg,
		uint8_t num_mem_chunks, struct wmi_host_mem_chunk *mem_chunk,
		bool action);

CDF_STATUS (*save_fw_version_cmd)(wmi_unified_t wmi_handle, void *evt_buf);

CDF_STATUS (*check_and_update_fw_version_cmd)(wmi_unified_t wmi_hdl, void *ev);

CDF_STATUS (*send_saved_init_cmd)(wmi_unified_t wmi_handle);

CDF_STATUS (*send_set_base_macaddr_indicate_cmd)(wmi_unified_t wmi_handle,
					 uint8_t *custom_addr);

CDF_STATUS (*send_log_supported_evt_cmd)(wmi_unified_t wmi_handle,
		uint8_t *event,
		uint32_t len);

CDF_STATUS (*send_enable_specific_fw_logs_cmd)(wmi_unified_t wmi_handle,
		struct wmi_wifi_start_log *start_log);

CDF_STATUS (*send_flush_logs_to_fw_cmd)(wmi_unified_t wmi_handle);

CDF_STATUS (*send_soc_set_pcl_cmd)(wmi_unified_t wmi_handle,
				struct wmi_pcl_list *msg);

CDF_STATUS (*send_soc_set_hw_mode_cmd)(wmi_unified_t wmi_handle,
				uint32_t hw_mode_index);

CDF_STATUS (*send_soc_set_dual_mac_config_cmd)(wmi_unified_t wmi_handle,
		struct wmi_dual_mac_config *msg);

CDF_STATUS (*send_enable_arp_ns_offload_cmd)(wmi_unified_t wmi_handle,
			   struct host_offload_req_param *param, bool arp_only,
			   uint8_t vdev_id);

CDF_STATUS (*send_set_led_flashing_cmd)(wmi_unified_t wmi_handle,
				struct flashing_req_params *flashing);

CDF_STATUS (*send_app_type1_params_in_fw_cmd)(wmi_unified_t wmi_handle,
				   struct app_type1_params *app_type1_params);

CDF_STATUS (*send_set_ssid_hotlist_cmd)(wmi_unified_t wmi_handle,
		     struct ssid_hotlist_request_params *request);

CDF_STATUS (*send_process_roam_synch_complete_cmd)(wmi_unified_t wmi_handle,
		 uint8_t vdev_id);

CDF_STATUS (*send_unit_test_cmd)(wmi_unified_t wmi_handle,
			       struct wmi_unit_test_cmd *wmi_utest);

CDF_STATUS (*send_roam_invoke_cmd)(wmi_unified_t wmi_handle,
		struct wmi_roam_invoke_cmd *roaminvoke,
		uint32_t ch_hz);

CDF_STATUS (*send_roam_scan_offload_cmd)(wmi_unified_t wmi_handle,
				 uint32_t command, uint32_t vdev_id);

CDF_STATUS (*send_roam_scan_offload_ap_profile_cmd)(wmi_unified_t wmi_handle,
				    wmi_ap_profile *ap_profile_p,
				    uint32_t vdev_id);

CDF_STATUS (*send_roam_scan_offload_scan_period_cmd)(wmi_unified_t wmi_handle,
				     uint32_t scan_period,
				     uint32_t scan_age,
				     uint32_t vdev_id);

CDF_STATUS (*send_roam_scan_offload_chan_list_cmd)(wmi_unified_t wmi_handle,
				   uint8_t chan_count,
				   uint8_t *chan_list,
				   uint8_t list_type, uint32_t vdev_id);

CDF_STATUS (*send_roam_scan_offload_rssi_change_cmd)(wmi_unified_t wmi_handle,
	uint32_t vdev_id,
	int32_t rssi_change_thresh,
	uint32_t bcn_rssi_weight,
	uint32_t hirssi_delay_btw_scans);

CDF_STATUS (*send_get_buf_extscan_hotlist_cmd)(wmi_unified_t wmi_handle,
				   struct ext_scan_setbssi_hotlist_params *
				   photlist, int *buf_len);
};

/**
 * struct wmi_init_cmd - Saved wmi INIT command
 * @buf: Buffer containing the wmi INIT command
 * @buf_len: Length of the buffer
 */
struct wmi_init_cmd {
	wmi_buf_t buf;
	uint32_t buf_len;
};

struct wmi_unified {
	ol_scn_t scn_handle;    /* handle to device */
	osdev_t  osdev; /* handle to use OS-independent services */
	cdf_atomic_t pending_cmds;
	HTC_ENDPOINT_ID wmi_endpoint_id;
	uint16_t max_msg_len;
	WMI_EVT_ID event_id[WMI_UNIFIED_MAX_EVENT];
	wmi_unified_event_handler event_handler[WMI_UNIFIED_MAX_EVENT];
	enum wmi_rx_exec_ctx ctx[WMI_UNIFIED_MAX_EVENT];
	uint32_t max_event_idx;
	void *htc_handle;
	cdf_spinlock_t eventq_lock;
	cdf_nbuf_queue_t event_queue;
	struct work_struct rx_event_work;
	int wmi_stop_in_progress;
	struct _wmi_abi_version fw_abi_version;
	struct _wmi_abi_version final_abi_vers;
	struct wmi_init_cmd saved_wmi_init_cmd;
	uint32_t num_of_diag_events_logs;
	uint32_t *events_logs_list;
	struct host_offload_req_param arp_info;
#ifdef WLAN_OPEN_SOURCE
	struct fwdebug dbglog;
	struct dentry *debugfs_phy;
#endif /* WLAN_OPEN_SOURCE */

#ifdef WMI_INTERFACE_EVENT_LOGGING
	cdf_spinlock_t wmi_record_lock;
#endif /*WMI_INTERFACE_EVENT_LOGGING */

	cdf_atomic_t is_target_suspended;

#ifdef FEATURE_RUNTIME_PM
	cdf_atomic_t runtime_pm_inprogress;
#endif

	struct wmi_rx_ops rx_ops;
	struct wmi_ops *ops;
	void *event_handler_cookie[WMI_UNIFIED_MAX_EVENT];
	bool use_cookie;
};
struct wmi_ops *wmi_get_tlv_ops(void);
struct wmi_ops *wmi_get_non_tlv_ops(void);
#endif
