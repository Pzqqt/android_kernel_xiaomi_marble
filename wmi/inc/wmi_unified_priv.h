/*
 * Copyright (c) 2013-2018 The Linux Foundation. All rights reserved.
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
#include "wmi_unified_api.h"
#include "wmi_unified_param.h"
#include "wlan_scan_ucfg_api.h"
#ifdef CONFIG_MCL
#include <wmi_unified.h>
#endif
#include "qdf_atomic.h"

#ifdef CONVERGED_P2P_ENABLE
#include <wlan_p2p_public_struct.h>
#endif

#ifdef DFS_COMPONENT_ENABLE
#include <wlan_dfs_public_struct.h>
#endif
#include <qdf_threads.h>
#ifdef WLAN_SUPPORT_GREEN_AP
#include "wlan_green_ap_api.h"
#endif

#ifdef WLAN_FEATURE_NAN_CONVERGENCE
#include "nan_public_structs.h"
#endif

#define WMI_UNIFIED_MAX_EVENT 0x100
#define WMI_MAX_CMDS 1024

#ifdef WMI_INTERFACE_EVENT_LOGGING

#define WMI_EVENT_DEBUG_MAX_ENTRY (1024)
#define WMI_EVENT_DEBUG_ENTRY_MAX_LENGTH (16)
/* wmi_mgmt commands */
#define WMI_MGMT_EVENT_DEBUG_MAX_ENTRY (256)

/**
 * struct wmi_command_debug - WMI command log buffer data type
 * @ command - Store WMI Command id
 * @ data - Stores WMI command data
 * @ time - Time of WMI command handling
 */
struct wmi_command_debug {
	uint32_t command;
	/*16 bytes of WMI cmd excluding TLV and WMI headers */
	uint32_t data[WMI_EVENT_DEBUG_ENTRY_MAX_LENGTH/sizeof(uint32_t)];
	uint64_t time;
};

/**
 * struct wmi_event_debug - WMI event log buffer data type
 * @ command - Store WMI Event id
 * @ data - Stores WMI Event data
 * @ time - Time of WMI Event handling
 */
struct wmi_event_debug {
	uint32_t event;
	/*16 bytes of WMI event data excluding TLV header */
	uint32_t data[WMI_EVENT_DEBUG_ENTRY_MAX_LENGTH/sizeof(uint32_t)];
	uint64_t time;
};

/**
 * struct wmi_command_header - Type for accessing frame data
 * @ type - 802.11 Frame type
 * @ subType - 802.11 Frame subtype
 * @ protVer - 802.11 Version
 */
struct wmi_command_header {
#ifndef ANI_LITTLE_BIT_ENDIAN

	uint32_t sub_type:4;
	uint32_t type:2;
	uint32_t prot_ver:2;

#else

	uint32_t prot_ver:2;
	uint32_t type:2;
	uint32_t sub_type:4;

#endif
};

/**
 * struct wmi_log_buf_t - WMI log buffer information type
 * @buf - Refernce to WMI log buffer
 * @ length - length of buffer
 * @ buf_tail_idx - Tail index of buffer
 * @ p_buf_tail_idx - refernce to buffer tail index. It is added to accommodate
 * unified design since MCL uses global variable for buffer tail index
 * @ size - the size of the buffer in number of entries
 */
struct wmi_log_buf_t {
	void *buf;
	uint32_t length;
	uint32_t buf_tail_idx;
	uint32_t *p_buf_tail_idx;
	uint32_t size;
};

/**
 * struct wmi_debug_log_info - Meta data to hold information of all buffers
 * used for WMI logging
 * @wmi_command_log_buf_info - Buffer info for WMI Command log
 * @wmi_command_tx_cmp_log_buf_info - Buffer info for WMI Command Tx completion
 * log
 * @wmi_event_log_buf_info - Buffer info for WMI Event log
 * @wmi_rx_event_log_buf_info - Buffer info for WMI event received log
 * @wmi_mgmt_command_log_buf_info - Buffer info for WMI Management Command log
 * @wmi_mgmt_command_tx_cmp_log_buf_info - Buffer info for WMI Management
 * Command Tx completion log
 * @wmi_mgmt_event_log_buf_info - Buffer info for WMI Management event log
 * @wmi_record_lock - Lock WMI recording
 * @wmi_logging_enable - Enable/Disable state for WMI logging
 * @buf_offset_command - Offset from where WMI command data should be logged
 * @buf_offset_event - Offset from where WMI event data should be logged
 * @is_management_record - Function refernce to check if command/event is
 *  management record
 * @wmi_id_to_name - Function refernce to API to convert Command id to
 * string name
 * @wmi_log_debugfs_dir - refernce to debugfs directory
 */
struct wmi_debug_log_info {
	struct wmi_log_buf_t wmi_command_log_buf_info;
	struct wmi_log_buf_t wmi_command_tx_cmp_log_buf_info;

	struct wmi_log_buf_t wmi_event_log_buf_info;
	struct wmi_log_buf_t wmi_rx_event_log_buf_info;

	struct wmi_log_buf_t wmi_mgmt_command_log_buf_info;
	struct wmi_log_buf_t wmi_mgmt_command_tx_cmp_log_buf_info;
	struct wmi_log_buf_t wmi_mgmt_event_log_buf_info;

	qdf_spinlock_t wmi_record_lock;
	bool wmi_logging_enable;
	uint32_t buf_offset_command;
	uint32_t buf_offset_event;
	struct dentry *wmi_log_debugfs_dir;
	uint8_t wmi_instance_id;
};

#endif /*WMI_INTERFACE_EVENT_LOGGING */

#ifdef WLAN_OPEN_SOURCE
struct fwdebug {
	struct sk_buff_head fwlog_queue;
	struct completion fwlog_completion;
	A_BOOL fwlog_open;
};
#endif /* WLAN_OPEN_SOURCE */

/**
 * struct wmi_wq_dbg_info - WMI WQ debug info
 * @ wd_msg_type_id - wmi event id
 * @ wmi_wq - WMI workqueue struct
 * @ task - WMI workqueue task struct
 */
struct wmi_wq_dbg_info {
	uint32_t wd_msg_type_id;
	qdf_workqueue_t *wmi_wq;
	qdf_thread_t *task;
};

struct wmi_ops {
QDF_STATUS (*send_vdev_create_cmd)(wmi_unified_t wmi_handle,
				 uint8_t macaddr[IEEE80211_ADDR_LEN],
				 struct vdev_create_params *param);

QDF_STATUS (*send_vdev_delete_cmd)(wmi_unified_t wmi_handle,
					  uint8_t if_id);

QDF_STATUS (*send_vdev_stop_cmd)(wmi_unified_t wmi,
					uint8_t vdev_id);

QDF_STATUS (*send_vdev_down_cmd)(wmi_unified_t wmi,
			uint8_t vdev_id);

QDF_STATUS (*send_vdev_start_cmd)(wmi_unified_t wmi,
		struct vdev_start_params *req);

QDF_STATUS (*send_vdev_set_nac_rssi_cmd)(wmi_unified_t wmi,
		struct vdev_scan_nac_rssi_params *req);

QDF_STATUS (*send_hidden_ssid_vdev_restart_cmd)(wmi_unified_t wmi_handle,
		struct hidden_ssid_vdev_restart_params *restart_params);

QDF_STATUS (*send_peer_flush_tids_cmd)(wmi_unified_t wmi,
					 uint8_t peer_addr[IEEE80211_ADDR_LEN],
					 struct peer_flush_params *param);

QDF_STATUS (*send_peer_delete_cmd)(wmi_unified_t wmi,
				    uint8_t peer_addr[IEEE80211_ADDR_LEN],
				    uint8_t vdev_id);

QDF_STATUS (*send_peer_param_cmd)(wmi_unified_t wmi,
				uint8_t peer_addr[IEEE80211_ADDR_LEN],
				struct peer_set_params *param);

QDF_STATUS (*send_vdev_up_cmd)(wmi_unified_t wmi,
			     uint8_t bssid[IEEE80211_ADDR_LEN],
				 struct vdev_up_params *params);

QDF_STATUS (*send_peer_create_cmd)(wmi_unified_t wmi,
					struct peer_create_params *param);

#ifdef WLAN_SUPPORT_GREEN_AP
QDF_STATUS (*send_green_ap_ps_cmd)(wmi_unified_t wmi_handle,
				   uint32_t value, uint8_t pdev_id);

QDF_STATUS (*extract_green_ap_egap_status_info)(
		uint8_t *evt_buf,
		struct wlan_green_ap_egap_status_info *egap_status_info_params);
#endif

QDF_STATUS
(*send_pdev_utf_cmd)(wmi_unified_t wmi_handle,
				struct pdev_utf_params *param,
				uint8_t mac_id);

QDF_STATUS
(*send_pdev_param_cmd)(wmi_unified_t wmi_handle,
			   struct pdev_params *param,
				uint8_t mac_id);

QDF_STATUS (*send_suspend_cmd)(wmi_unified_t wmi_handle,
				struct suspend_params *param,
				uint8_t mac_id);

QDF_STATUS (*send_resume_cmd)(wmi_unified_t wmi_handle,
				uint8_t mac_id);

#ifdef FEATURE_WLAN_D0WOW
QDF_STATUS (*send_d0wow_enable_cmd)(wmi_unified_t wmi_handle,
				uint8_t mac_id);
QDF_STATUS (*send_d0wow_disable_cmd)(wmi_unified_t wmi_handle,
				uint8_t mac_id);
#endif

QDF_STATUS (*send_wow_enable_cmd)(wmi_unified_t wmi_handle,
				struct wow_cmd_params *param,
				uint8_t mac_id);

QDF_STATUS (*send_set_ap_ps_param_cmd)(wmi_unified_t wmi_handle,
					   uint8_t *peer_addr,
					   struct ap_ps_params *param);

QDF_STATUS (*send_set_sta_ps_param_cmd)(wmi_unified_t wmi_handle,
					   struct sta_ps_params *param);

QDF_STATUS (*send_crash_inject_cmd)(wmi_unified_t wmi_handle,
			 struct crash_inject *param);

QDF_STATUS
(*send_dbglog_cmd)(wmi_unified_t wmi_handle,
				struct dbglog_params *dbglog_param);

QDF_STATUS (*send_vdev_set_param_cmd)(wmi_unified_t wmi_handle,
				struct vdev_set_params *param);

QDF_STATUS (*send_stats_request_cmd)(wmi_unified_t wmi_handle,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct stats_request_params *param);

#ifdef CONFIG_WIN
QDF_STATUS (*send_packet_log_enable_cmd)(wmi_unified_t wmi_handle,
			WMI_HOST_PKTLOG_EVENT PKTLOG_EVENT, uint8_t mac_id);
#else
QDF_STATUS (*send_packet_log_enable_cmd)(wmi_unified_t wmi_handle,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct packet_enable_params *param);
#endif

QDF_STATUS (*send_packet_log_disable_cmd)(wmi_unified_t wmi_handle,
	uint8_t mac_id);

QDF_STATUS (*send_beacon_send_cmd)(wmi_unified_t wmi_handle,
				struct beacon_params *param);

QDF_STATUS (*send_beacon_tmpl_send_cmd)(wmi_unified_t wmi_handle,
				struct beacon_tmpl_params *param);

QDF_STATUS (*send_peer_assoc_cmd)(wmi_unified_t wmi_handle,
				struct peer_assoc_params *param);

QDF_STATUS (*send_scan_start_cmd)(wmi_unified_t wmi_handle,
				struct scan_req_params *param);

QDF_STATUS (*send_scan_stop_cmd)(wmi_unified_t wmi_handle,
				struct scan_cancel_param *param);

QDF_STATUS (*send_scan_chan_list_cmd)(wmi_unified_t wmi_handle,
				struct scan_chan_list_params *param);

QDF_STATUS (*send_mgmt_cmd)(wmi_unified_t wmi_handle,
				struct wmi_mgmt_params *param);

QDF_STATUS (*send_offchan_data_tx_cmd)(wmi_unified_t wmi_handle,
				struct wmi_offchan_data_tx_params *param);

QDF_STATUS (*send_modem_power_state_cmd)(wmi_unified_t wmi_handle,
		uint32_t param_value);

QDF_STATUS (*send_set_sta_ps_mode_cmd)(wmi_unified_t wmi_handle,
			       uint32_t vdev_id, uint8_t val);

QDF_STATUS (*send_get_temperature_cmd)(wmi_unified_t wmi_handle);

QDF_STATUS (*send_set_p2pgo_oppps_req_cmd)(wmi_unified_t wmi_handle,
		struct p2p_ps_params *oppps);

QDF_STATUS (*send_set_p2pgo_noa_req_cmd)(wmi_unified_t wmi_handle,
			struct p2p_ps_params *noa);

#ifdef CONVERGED_P2P_ENABLE
QDF_STATUS (*send_p2p_lo_start_cmd)(wmi_unified_t wmi_handle,
			struct p2p_lo_start *param);

QDF_STATUS (*send_p2p_lo_stop_cmd)(wmi_unified_t wmi_handle,
			uint8_t vdev_id);
#endif

QDF_STATUS (*send_set_smps_params_cmd)(wmi_unified_t wmi_handle,
			  uint8_t vdev_id,
			  int value);

QDF_STATUS (*send_set_mimops_cmd)(wmi_unified_t wmi_handle,
			uint8_t vdev_id, int value);

QDF_STATUS (*send_set_sta_uapsd_auto_trig_cmd)(wmi_unified_t wmi_handle,
				struct sta_uapsd_trig_params *param);

#ifdef WLAN_FEATURE_DSRC
QDF_STATUS (*send_ocb_set_utc_time_cmd)(wmi_unified_t wmi_handle,
				struct ocb_utc_param *utc);

QDF_STATUS (*send_ocb_get_tsf_timer_cmd)(wmi_unified_t wmi_handle,
			  uint8_t vdev_id);

QDF_STATUS (*send_ocb_start_timing_advert_cmd)(wmi_unified_t wmi_handle,
	struct ocb_timing_advert_param *timing_advert);

QDF_STATUS (*send_ocb_stop_timing_advert_cmd)(wmi_unified_t wmi_handle,
	struct ocb_timing_advert_param *timing_advert);

QDF_STATUS (*send_dcc_get_stats_cmd)(wmi_unified_t wmi_handle,
		     struct ocb_dcc_get_stats_param *get_stats_param);

QDF_STATUS (*send_dcc_clear_stats_cmd)(wmi_unified_t wmi_handle,
				uint32_t vdev_id, uint32_t dcc_stats_bitmap);

QDF_STATUS (*send_dcc_update_ndl_cmd)(wmi_unified_t wmi_handle,
		       struct ocb_dcc_update_ndl_param *update_ndl_param);

QDF_STATUS (*send_ocb_set_config_cmd)(wmi_unified_t wmi_handle,
		  struct ocb_config *config);
QDF_STATUS (*extract_ocb_chan_config_resp)(wmi_unified_t wmi_hdl,
					   void *evt_buf,
					   uint32_t *status);
QDF_STATUS (*extract_ocb_tsf_timer)(wmi_unified_t wmi_hdl,
				    void *evt_buf,
				    struct ocb_get_tsf_timer_response *resp);
QDF_STATUS (*extract_dcc_update_ndl_resp)(wmi_unified_t wmi_hdl,
		void *evt_buf, struct ocb_dcc_update_ndl_response *resp);
QDF_STATUS (*extract_dcc_stats)(wmi_unified_t wmi_hdl,
				void *evt_buf,
				struct ocb_dcc_get_stats_response **response);
#endif
QDF_STATUS (*send_lro_config_cmd)(wmi_unified_t wmi_handle,
	 struct wmi_lro_config_cmd_t *wmi_lro_cmd);

QDF_STATUS (*send_set_thermal_mgmt_cmd)(wmi_unified_t wmi_handle,
				struct thermal_cmd_params *thermal_info);

QDF_STATUS (*send_peer_rate_report_cmd)(wmi_unified_t wmi_handle,
	 struct wmi_peer_rate_report_params *rate_report_params);

QDF_STATUS (*send_set_mcc_channel_time_quota_cmd)
	(wmi_unified_t wmi_handle,
	uint32_t adapter_1_chan_freq,
	uint32_t adapter_1_quota, uint32_t adapter_2_chan_freq);

QDF_STATUS (*send_set_mcc_channel_time_latency_cmd)
	(wmi_unified_t wmi_handle,
	uint32_t mcc_channel_freq, uint32_t mcc_channel_time_latency);

QDF_STATUS (*send_set_enable_disable_mcc_adaptive_scheduler_cmd)(
		  wmi_unified_t wmi_handle, uint32_t mcc_adaptive_scheduler,
		  uint32_t pdev_id);

QDF_STATUS (*send_p2p_go_set_beacon_ie_cmd)(wmi_unified_t wmi_handle,
				    A_UINT32 vdev_id, uint8_t *p2p_ie);

QDF_STATUS (*send_probe_rsp_tmpl_send_cmd)(wmi_unified_t wmi_handle,
			     uint8_t vdev_id,
			     struct wmi_probe_resp_params *probe_rsp_info);

QDF_STATUS (*send_setup_install_key_cmd)(wmi_unified_t wmi_handle,
				struct set_key_params *key_params);

QDF_STATUS (*send_vdev_set_gtx_cfg_cmd)(wmi_unified_t wmi_handle,
				  uint32_t if_id,
				  struct wmi_gtx_config *gtx_info);

QDF_STATUS (*send_set_sta_keep_alive_cmd)(wmi_unified_t wmi_handle,
				struct sta_params *params);

QDF_STATUS (*send_set_sta_sa_query_param_cmd)(wmi_unified_t wmi_handle,
				       uint8_t vdev_id, uint32_t max_retries,
					   uint32_t retry_interval);

QDF_STATUS (*send_set_gateway_params_cmd)(wmi_unified_t wmi_handle,
					struct gateway_update_req_param *req);

QDF_STATUS (*send_set_rssi_monitoring_cmd)(wmi_unified_t wmi_handle,
					struct rssi_monitor_param *req);

QDF_STATUS (*send_scan_probe_setoui_cmd)(wmi_unified_t wmi_handle,
			  struct scan_mac_oui *psetoui);

QDF_STATUS (*send_reset_passpoint_network_list_cmd)(wmi_unified_t wmi_handle,
					struct wifi_passpoint_req_param *req);

QDF_STATUS (*send_roam_scan_offload_rssi_thresh_cmd)(wmi_unified_t wmi_handle,
				struct roam_offload_scan_rssi_params *roam_req);

QDF_STATUS (*send_roam_mawc_params_cmd)(wmi_unified_t wmi_handle,
		struct wmi_mawc_roam_params *params);

QDF_STATUS (*send_roam_scan_filter_cmd)(wmi_unified_t wmi_handle,
				struct roam_scan_filter_params *roam_req);

#if defined(WLAN_FEATURE_FILS_SK)
QDF_STATUS (*send_roam_scan_hlp_cmd) (wmi_unified_t wmi_handle,
				struct hlp_params *params);
#endif

QDF_STATUS (*send_set_passpoint_network_list_cmd)(wmi_unified_t wmi_handle,
					struct wifi_passpoint_req_param *req);

QDF_STATUS (*send_set_epno_network_list_cmd)(wmi_unified_t wmi_handle,
		struct wifi_enhanched_pno_params *req);

QDF_STATUS (*send_extscan_get_capabilities_cmd)(wmi_unified_t wmi_handle,
			  struct extscan_capabilities_params *pgetcapab);

QDF_STATUS (*send_extscan_get_cached_results_cmd)(wmi_unified_t wmi_handle,
			  struct extscan_cached_result_params *pcached_results);

QDF_STATUS (*send_extscan_stop_change_monitor_cmd)(wmi_unified_t wmi_handle,
			  struct extscan_capabilities_reset_params *reset_req);

QDF_STATUS (*send_extscan_start_change_monitor_cmd)(wmi_unified_t wmi_handle,
		struct extscan_set_sig_changereq_params *
		psigchange);

QDF_STATUS (*send_extscan_stop_hotlist_monitor_cmd)(wmi_unified_t wmi_handle,
		struct extscan_bssid_hotlist_reset_params *photlist_reset);

QDF_STATUS (*send_stop_extscan_cmd)(wmi_unified_t wmi_handle,
		  struct extscan_stop_req_params *pstopcmd);

QDF_STATUS (*send_start_extscan_cmd)(wmi_unified_t wmi_handle,
		    struct wifi_scan_cmd_req_params *pstart);

QDF_STATUS (*send_plm_stop_cmd)(wmi_unified_t wmi_handle,
		 const struct plm_req_params *plm);

QDF_STATUS (*send_wlm_latency_level_cmd)(wmi_unified_t wmi_handle,
				struct wlm_latency_level_param *param);

QDF_STATUS (*send_plm_start_cmd)(wmi_unified_t wmi_handle,
		  const struct plm_req_params *plm,
		  uint32_t *gchannel_list);

QDF_STATUS (*send_csa_offload_enable_cmd)(wmi_unified_t wmi_handle,
			uint8_t vdev_id);

QDF_STATUS (*send_pno_stop_cmd)(wmi_unified_t wmi_handle, uint8_t vdev_id);

QDF_STATUS (*send_pno_start_cmd)(wmi_unified_t wmi_handle,
		   struct pno_scan_req_params *pno);

QDF_STATUS (*send_nlo_mawc_cmd)(wmi_unified_t wmi_handle,
		struct nlo_mawc_params *params);

QDF_STATUS (*send_ipa_offload_control_cmd)(wmi_unified_t wmi_handle,
		struct ipa_offload_control_params *ipa_offload);

QDF_STATUS (*send_set_ric_req_cmd)(wmi_unified_t wmi_handle, void *msg,
			uint8_t is_add_ts);

QDF_STATUS (*send_process_ll_stats_clear_cmd)
	   (wmi_unified_t wmi_handle,
	   const struct ll_stats_clear_params *clear_req,
	   uint8_t addr[IEEE80211_ADDR_LEN]);

QDF_STATUS (*send_process_ll_stats_set_cmd)
	(wmi_unified_t wmi_handle, const struct ll_stats_set_params *set_req);

QDF_STATUS (*send_process_ll_stats_get_cmd)
	(wmi_unified_t wmi_handle, const struct ll_stats_get_params  *get_req,
		 uint8_t addr[IEEE80211_ADDR_LEN]);


QDF_STATUS (*send_congestion_cmd)(wmi_unified_t wmi_handle,
			A_UINT8 vdev_id);

QDF_STATUS (*send_snr_request_cmd)(wmi_unified_t wmi_handle);

QDF_STATUS (*send_snr_cmd)(wmi_unified_t wmi_handle, uint8_t vdev_id);

QDF_STATUS (*send_link_status_req_cmd)(wmi_unified_t wmi_handle,
				 struct link_status_params *link_status);
#ifdef WLAN_PMO_ENABLE
QDF_STATUS (*send_add_wow_wakeup_event_cmd)(wmi_unified_t wmi_handle,
					uint32_t vdev_id,
					uint32_t *bitmap,
					bool enable);

QDF_STATUS (*send_wow_patterns_to_fw_cmd)(wmi_unified_t wmi_handle,
				uint8_t vdev_id, uint8_t ptrn_id,
				const uint8_t *ptrn, uint8_t ptrn_len,
				uint8_t ptrn_offset, const uint8_t *mask,
				uint8_t mask_len, bool user,
				uint8_t default_patterns);

QDF_STATUS (*send_enable_arp_ns_offload_cmd)(wmi_unified_t wmi_handle,
			   struct pmo_arp_offload_params *arp_offload_req,
			   struct pmo_ns_offload_params *ns_offload_req,
			   uint8_t vdev_id);

QDF_STATUS (*send_conf_hw_filter_cmd)(wmi_unified_t wmi,
				      struct pmo_hw_filter_params *req);

QDF_STATUS (*send_enable_enhance_multicast_offload_cmd)(
		wmi_unified_t wmi_handle,
		uint8_t vdev_id, bool action);

QDF_STATUS (*send_add_clear_mcbc_filter_cmd)(wmi_unified_t wmi_handle,
				     uint8_t vdev_id,
				     struct qdf_mac_addr multicast_addr,
				     bool clearList);

QDF_STATUS (*send_multiple_add_clear_mcbc_filter_cmd)(wmi_unified_t wmi_handle,
				uint8_t vdev_id,
				struct pmo_mcast_filter_params *filter_param);

QDF_STATUS (*send_gtk_offload_cmd)(wmi_unified_t wmi_handle, uint8_t vdev_id,
					   struct pmo_gtk_req *params,
					   bool enable_offload,
					   uint32_t gtk_offload_opcode);

QDF_STATUS (*send_process_gtk_offload_getinfo_cmd)(wmi_unified_t wmi_handle,
				uint8_t vdev_id,
				uint64_t offload_req_opcode);

QDF_STATUS (*send_wow_sta_ra_filter_cmd)(wmi_unified_t wmi_handle,
				   uint8_t vdev_id, uint8_t default_pattern,
				   uint16_t rate_limit_interval);

QDF_STATUS (*send_action_frame_patterns_cmd)(wmi_unified_t wmi_handle,
			struct pmo_action_wakeup_set_params *action_params);

QDF_STATUS (*extract_gtk_rsp_event)(wmi_unified_t wmi_handle,
			void *evt_buf,
			struct pmo_gtk_rsp_params *gtk_rsp_param, uint32_t len);

QDF_STATUS (*send_lphb_config_hbenable_cmd)(wmi_unified_t wmi_handle,
				wmi_hb_set_enable_cmd_fixed_param *params);

QDF_STATUS (*send_lphb_config_tcp_params_cmd)(wmi_unified_t wmi_handle,
				    wmi_hb_set_tcp_params_cmd_fixed_param *lphb_conf_req);

QDF_STATUS (*send_lphb_config_tcp_pkt_filter_cmd)(wmi_unified_t wmi_handle,
				wmi_hb_set_tcp_pkt_filter_cmd_fixed_param *g_hb_tcp_filter_fp);

QDF_STATUS (*send_lphb_config_udp_params_cmd)(wmi_unified_t wmi_handle,
				    wmi_hb_set_udp_params_cmd_fixed_param *lphb_conf_req);

QDF_STATUS (*send_lphb_config_udp_pkt_filter_cmd)(wmi_unified_t wmi_handle,
					wmi_hb_set_udp_pkt_filter_cmd_fixed_param *lphb_conf_req);

QDF_STATUS (*send_enable_disable_packet_filter_cmd)(wmi_unified_t wmi_handle,
					uint8_t vdev_id, bool enable);

QDF_STATUS (*send_config_packet_filter_cmd)(wmi_unified_t wmi_handle,
		uint8_t vdev_id, struct pmo_rcv_pkt_fltr_cfg *rcv_filter_param,
		uint8_t filter_id, bool enable);
#endif /* end of WLAN_PMO_ENABLE */
#ifdef CONFIG_MCL
QDF_STATUS (*send_process_dhcp_ind_cmd)(wmi_unified_t wmi_handle,
				wmi_peer_set_param_cmd_fixed_param *ta_dhcp_ind);

QDF_STATUS (*send_get_link_speed_cmd)(wmi_unified_t wmi_handle,
			wmi_mac_addr peer_macaddr);

QDF_STATUS (*send_bcn_buf_ll_cmd)(wmi_unified_t wmi_handle,
			wmi_bcn_send_from_host_cmd_fixed_param * param);

QDF_STATUS (*send_roam_scan_offload_mode_cmd)(wmi_unified_t wmi_handle,
				wmi_start_scan_cmd_fixed_param * scan_cmd_fp,
				struct roam_offload_scan_params *roam_req);

QDF_STATUS (*send_roam_scan_offload_ap_profile_cmd)(wmi_unified_t wmi_handle,
				    struct ap_profile_params *ap_profile);

QDF_STATUS (*send_pktlog_wmi_send_cmd)(wmi_unified_t wmi_handle,
				   WMI_PKTLOG_EVENT pktlog_event,
				   WMI_CMD_ID cmd_id, uint8_t user_triggered);
#endif

#ifdef WLAN_SUPPORT_GREEN_AP
QDF_STATUS (*send_egap_conf_params_cmd)(wmi_unified_t wmi_handle,
			struct wlan_green_ap_egap_params *egap_params);
#endif

QDF_STATUS (*send_fw_profiling_cmd)(wmi_unified_t wmi_handle,
			uint32_t cmd, uint32_t value1, uint32_t value2);

QDF_STATUS (*send_nat_keepalive_en_cmd)(wmi_unified_t wmi_handle, uint8_t vdev_id);

#ifdef WLAN_FEATURE_CIF_CFR
QDF_STATUS (*send_oem_dma_cfg_cmd)(wmi_unified_t wmi_handle,
				   wmi_oem_dma_ring_cfg_req_fixed_param *cfg);
#endif

QDF_STATUS (*send_dbr_cfg_cmd)(wmi_unified_t wmi_handle,
				   struct direct_buf_rx_cfg_req *cfg);

QDF_STATUS (*send_start_oem_data_cmd)(wmi_unified_t wmi_handle,
			  uint32_t data_len,
			  uint8_t *data);

QDF_STATUS
(*send_dfs_phyerr_filter_offload_en_cmd)(wmi_unified_t wmi_handle,
			bool dfs_phyerr_filter_offload);

QDF_STATUS (*send_wow_delete_pattern_cmd)(wmi_unified_t wmi_handle, uint8_t ptrn_id,
					uint8_t vdev_id);

QDF_STATUS (*send_host_wakeup_ind_to_fw_cmd)(wmi_unified_t wmi_handle);

QDF_STATUS (*send_del_ts_cmd)(wmi_unified_t wmi_handle, uint8_t vdev_id,
				uint8_t ac);

QDF_STATUS (*send_aggr_qos_cmd)(wmi_unified_t wmi_handle,
		      struct aggr_add_ts_param *aggr_qos_rsp_msg);

QDF_STATUS (*send_add_ts_cmd)(wmi_unified_t wmi_handle,
		 struct add_ts_param *msg);

QDF_STATUS (*send_process_add_periodic_tx_ptrn_cmd)(wmi_unified_t wmi_handle,
						struct periodic_tx_pattern  *
						pAddPeriodicTxPtrnParams,
						uint8_t vdev_id);

QDF_STATUS (*send_process_del_periodic_tx_ptrn_cmd)(wmi_unified_t wmi_handle,
						uint8_t vdev_id,
						uint8_t pattern_id);

QDF_STATUS (*send_stats_ext_req_cmd)(wmi_unified_t wmi_handle,
			struct stats_ext_params *preq);

QDF_STATUS (*send_enable_ext_wow_cmd)(wmi_unified_t wmi_handle,
			struct ext_wow_params *params);

QDF_STATUS (*send_set_app_type2_params_in_fw_cmd)(wmi_unified_t wmi_handle,
					  struct app_type2_params *appType2Params);

QDF_STATUS (*send_set_auto_shutdown_timer_cmd)(wmi_unified_t wmi_handle,
						  uint32_t timer_val);

QDF_STATUS (*send_nan_req_cmd)(wmi_unified_t wmi_handle,
			struct nan_req_params *nan_req);

QDF_STATUS (*send_process_dhcpserver_offload_cmd)(wmi_unified_t wmi_handle,
				struct dhcp_offload_info_params *params);

QDF_STATUS (*send_process_ch_avoid_update_cmd)(wmi_unified_t wmi_handle);

QDF_STATUS (*send_regdomain_info_to_fw_cmd)(wmi_unified_t wmi_handle,
				   uint32_t reg_dmn, uint16_t regdmn2G,
				   uint16_t regdmn5G, uint8_t ctl2G,
				   uint8_t ctl5G);

QDF_STATUS (*send_set_tdls_offchan_mode_cmd)(wmi_unified_t wmi_handle,
			      struct tdls_channel_switch_params *chan_switch_params);

QDF_STATUS (*send_update_fw_tdls_state_cmd)(wmi_unified_t wmi_handle,
					 void *tdls_param, uint8_t tdls_state);

QDF_STATUS (*send_update_tdls_peer_state_cmd)(wmi_unified_t wmi_handle,
			       struct tdls_peer_state_params *peerStateParams,
				   uint32_t *ch_mhz);


QDF_STATUS (*send_process_fw_mem_dump_cmd)(wmi_unified_t wmi_handle,
					struct fw_dump_req_param *mem_dump_req);

QDF_STATUS (*send_process_set_ie_info_cmd)(wmi_unified_t wmi_handle,
				   struct vdev_ie_info_param *ie_info);

QDF_STATUS (*save_fw_version_cmd)(wmi_unified_t wmi_handle, void *evt_buf);

QDF_STATUS (*check_and_update_fw_version_cmd)(wmi_unified_t wmi_hdl, void *ev);

QDF_STATUS (*send_set_base_macaddr_indicate_cmd)(wmi_unified_t wmi_handle,
					 uint8_t *custom_addr);

QDF_STATUS (*send_log_supported_evt_cmd)(wmi_unified_t wmi_handle,
		uint8_t *event,
		uint32_t len);

QDF_STATUS (*send_enable_specific_fw_logs_cmd)(wmi_unified_t wmi_handle,
		struct wmi_wifi_start_log *start_log);

QDF_STATUS (*send_flush_logs_to_fw_cmd)(wmi_unified_t wmi_handle);

QDF_STATUS (*send_pdev_set_pcl_cmd)(wmi_unified_t wmi_handle,
				struct wmi_pcl_chan_weights *msg);

QDF_STATUS (*send_pdev_set_hw_mode_cmd)(wmi_unified_t wmi_handle,
				uint32_t hw_mode_index);

QDF_STATUS (*send_pdev_set_dual_mac_config_cmd)(wmi_unified_t wmi_handle,
		struct wmi_dual_mac_config *msg);

QDF_STATUS (*send_set_led_flashing_cmd)(wmi_unified_t wmi_handle,
				struct flashing_req_params *flashing);

QDF_STATUS (*send_app_type1_params_in_fw_cmd)(wmi_unified_t wmi_handle,
				   struct app_type1_params *app_type1_params);

QDF_STATUS (*send_set_ssid_hotlist_cmd)(wmi_unified_t wmi_handle,
		     struct ssid_hotlist_request_params *request);

QDF_STATUS (*send_process_roam_synch_complete_cmd)(wmi_unified_t wmi_handle,
		 uint8_t vdev_id);

QDF_STATUS (*send_unit_test_cmd)(wmi_unified_t wmi_handle,
				 struct wmi_unit_test_cmd *wmi_utest);

QDF_STATUS (*send_roam_invoke_cmd)(wmi_unified_t wmi_handle,
		struct wmi_roam_invoke_cmd *roaminvoke,
		uint32_t ch_hz);

QDF_STATUS (*send_roam_scan_offload_cmd)(wmi_unified_t wmi_handle,
				 uint32_t command, uint32_t vdev_id);

QDF_STATUS (*send_roam_scan_offload_scan_period_cmd)(wmi_unified_t wmi_handle,
				     uint32_t scan_period,
				     uint32_t scan_age,
				     uint32_t vdev_id);

QDF_STATUS (*send_roam_scan_offload_chan_list_cmd)(wmi_unified_t wmi_handle,
				   uint8_t chan_count,
				   uint32_t *chan_list,
				   uint8_t list_type, uint32_t vdev_id);

QDF_STATUS (*send_roam_scan_offload_rssi_change_cmd)(wmi_unified_t wmi_handle,
	uint32_t vdev_id,
	int32_t rssi_change_thresh,
	uint32_t bcn_rssi_weight,
	uint32_t hirssi_delay_btw_scans);

QDF_STATUS (*send_per_roam_config_cmd)(wmi_unified_t wmi_handle,
		struct wmi_per_roam_config_req *req_buf);

QDF_STATUS (*send_set_arp_stats_req_cmd)(wmi_unified_t wmi_handle,
					 struct set_arp_stats *req_buf);

QDF_STATUS (*send_get_arp_stats_req_cmd)(wmi_unified_t wmi_handle,
					 struct get_arp_stats *req_buf);

QDF_STATUS (*send_get_buf_extscan_hotlist_cmd)(wmi_unified_t wmi_handle,
				   struct ext_scan_setbssi_hotlist_params *
				   photlist, int *buf_len);

QDF_STATUS (*send_set_active_bpf_mode_cmd)(wmi_unified_t wmi_handle,
				uint8_t vdev_id,
				enum wmi_host_active_bpf_mode ucast_mode,
				enum wmi_host_active_bpf_mode mcast_bcast_mode);

QDF_STATUS (*send_pdev_get_tpc_config_cmd)(wmi_unified_t wmi_handle,
		uint32_t param);

QDF_STATUS (*send_set_bwf_cmd)(wmi_unified_t wmi_handle,
		struct set_bwf_params *param);

QDF_STATUS (*send_set_atf_cmd)(wmi_unified_t wmi_handle,
		struct set_atf_params *param);

QDF_STATUS (*send_pdev_fips_cmd)(wmi_unified_t wmi_handle,
		struct fips_params *param);

QDF_STATUS (*send_wlan_profile_enable_cmd)(wmi_unified_t wmi_handle,
		struct wlan_profile_params *param);

QDF_STATUS (*send_wlan_profile_trigger_cmd)(wmi_unified_t wmi_handle,
		struct wlan_profile_params *param);

QDF_STATUS (*send_pdev_set_chan_cmd)(wmi_unified_t wmi_handle,
		struct channel_param *param);

QDF_STATUS (*send_set_ht_ie_cmd)(wmi_unified_t wmi_handle,
		struct ht_ie_params *param);

QDF_STATUS (*send_set_vht_ie_cmd)(wmi_unified_t wmi_handle,
		struct vht_ie_params *param);

QDF_STATUS (*send_wmm_update_cmd)(wmi_unified_t wmi_handle,
		struct wmm_update_params *param);

QDF_STATUS (*send_process_update_edca_param_cmd)(wmi_unified_t wmi_handle,
		uint8_t vdev_id,
		struct wmi_host_wme_vparams wmm_vparams[WMI_MAX_NUM_AC]);

QDF_STATUS (*send_set_ant_switch_tbl_cmd)(wmi_unified_t wmi_handle,
		struct ant_switch_tbl_params *param);

QDF_STATUS (*send_set_ratepwr_table_cmd)(wmi_unified_t wmi_handle,
		struct ratepwr_table_params *param);

QDF_STATUS (*send_get_ratepwr_table_cmd)(wmi_unified_t wmi_handle);

QDF_STATUS (*send_set_ctl_table_cmd)(wmi_unified_t wmi_handle,
		struct ctl_table_params *param);

QDF_STATUS (*send_set_mimogain_table_cmd)(wmi_unified_t wmi_handle,
		struct mimogain_table_params *param);

QDF_STATUS (*send_set_ratepwr_chainmsk_cmd)(wmi_unified_t wmi_handle,
		struct ratepwr_chainmsk_params *param);

QDF_STATUS (*send_set_macaddr_cmd)(wmi_unified_t wmi_handle,
		struct macaddr_params *param);

QDF_STATUS (*send_pdev_scan_start_cmd)(wmi_unified_t wmi_handle);

QDF_STATUS (*send_pdev_scan_end_cmd)(wmi_unified_t wmi_handle);

QDF_STATUS (*send_set_acparams_cmd)(wmi_unified_t wmi_handle,
		struct acparams_params *param);

QDF_STATUS (*send_set_vap_dscp_tid_map_cmd)(wmi_unified_t wmi_handle,
		struct vap_dscp_tid_map_params *param);

QDF_STATUS (*send_proxy_ast_reserve_cmd)(wmi_unified_t wmi_handle,
		struct proxy_ast_reserve_params *param);

QDF_STATUS (*send_pdev_qvit_cmd)(wmi_unified_t wmi_handle,
		struct pdev_qvit_params *param);

QDF_STATUS (*send_mcast_group_update_cmd)(wmi_unified_t wmi_handle,
		struct mcast_group_update_params *param);

QDF_STATUS (*send_peer_add_wds_entry_cmd)(wmi_unified_t wmi_handle,
		struct peer_add_wds_entry_params *param);

QDF_STATUS (*send_peer_del_wds_entry_cmd)(wmi_unified_t wmi_handle,
		struct peer_del_wds_entry_params *param);

QDF_STATUS (*send_set_bridge_mac_addr_cmd)(wmi_unified_t wmi_handle,
		struct set_bridge_mac_addr_params *param);

QDF_STATUS (*send_peer_update_wds_entry_cmd)(wmi_unified_t wmi_handle,
		struct peer_update_wds_entry_params *param);

QDF_STATUS (*send_phyerr_enable_cmd)(wmi_unified_t wmi_handle);

QDF_STATUS (*send_phyerr_disable_cmd)(wmi_unified_t wmi_handle);

QDF_STATUS (*send_smart_ant_enable_cmd)(wmi_unified_t wmi_handle,
		struct smart_ant_enable_params *param);

QDF_STATUS (*send_smart_ant_set_rx_ant_cmd)(wmi_unified_t wmi_handle,
		struct smart_ant_rx_ant_params *param);

QDF_STATUS (*send_smart_ant_set_tx_ant_cmd)(wmi_unified_t wmi_handle,
		uint8_t macaddr[IEEE80211_ADDR_LEN],
		struct smart_ant_tx_ant_params *param);

QDF_STATUS (*send_smart_ant_set_training_info_cmd)(wmi_unified_t wmi_handle,
		uint8_t macaddr[IEEE80211_ADDR_LEN],
		struct smart_ant_training_info_params *param);

QDF_STATUS (*send_smart_ant_set_node_config_cmd)(wmi_unified_t wmi_handle,
		uint8_t macaddr[IEEE80211_ADDR_LEN],
		struct smart_ant_node_config_params *param);

QDF_STATUS (*send_smart_ant_enable_tx_feedback_cmd)(wmi_unified_t wmi_handle,
		struct smart_ant_enable_tx_feedback_params *param);

QDF_STATUS (*send_vdev_spectral_configure_cmd)(wmi_unified_t wmi_handle,
		struct vdev_spectral_configure_params *param);

QDF_STATUS (*send_vdev_spectral_enable_cmd)(wmi_unified_t wmi_handle,
		struct vdev_spectral_enable_params *param);
QDF_STATUS (*send_set_del_pmkid_cache_cmd) (wmi_unified_t wmi_handle,
		struct wmi_unified_pmk_cache *req_buf);

QDF_STATUS (*send_bss_chan_info_request_cmd)(wmi_unified_t wmi_handle,
		struct bss_chan_info_request_params *param);

QDF_STATUS (*send_thermal_mitigation_param_cmd)(wmi_unified_t wmi_handle,
		struct thermal_mitigation_params *param);

QDF_STATUS (*send_vdev_set_neighbour_rx_cmd)(wmi_unified_t wmi_handle,
		uint8_t macaddr[IEEE80211_ADDR_LEN],
		struct set_neighbour_rx_params *param);

QDF_STATUS (*send_vdev_set_fwtest_param_cmd)(wmi_unified_t wmi_handle,
		struct set_fwtest_params *param);

QDF_STATUS (*send_vdev_config_ratemask_cmd)(wmi_unified_t wmi_handle,
		struct config_ratemask_params *param);

QDF_STATUS (*send_vdev_set_custom_aggr_size_cmd)(wmi_unified_t wmi_handle,
		struct set_custom_aggr_size_params *param);

QDF_STATUS (*send_vdev_set_qdepth_thresh_cmd)(wmi_unified_t wmi_handle,
		struct set_qdepth_thresh_params *param);

QDF_STATUS (*send_wow_wakeup_cmd)(wmi_unified_t wmi_handle);

QDF_STATUS (*send_wow_add_wakeup_event_cmd)(wmi_unified_t wmi_handle,
		struct wow_add_wakeup_params *param);

QDF_STATUS (*send_wow_add_wakeup_pattern_cmd)(wmi_unified_t wmi_handle,
		struct wow_add_wakeup_pattern_params *param);

QDF_STATUS (*send_wow_remove_wakeup_pattern_cmd)(wmi_unified_t wmi_handle,
		struct wow_remove_wakeup_pattern_params *param);

QDF_STATUS (*send_pdev_set_regdomain_cmd)(wmi_unified_t wmi_handle,
		struct pdev_set_regdomain_params *param);

QDF_STATUS (*send_set_quiet_mode_cmd)(wmi_unified_t wmi_handle,
		struct set_quiet_mode_params *param);

QDF_STATUS (*send_set_beacon_filter_cmd)(wmi_unified_t wmi_handle,
		struct set_beacon_filter_params *param);

QDF_STATUS (*send_remove_beacon_filter_cmd)(wmi_unified_t wmi_handle,
		struct remove_beacon_filter_params *param);
/*
QDF_STATUS (*send_mgmt_cmd)(wmi_unified_t wmi_handle,
		uint8_t macaddr[IEEE80211_ADDR_LEN],
		struct mgmt_params *param);
		*/

QDF_STATUS (*send_addba_clearresponse_cmd)(wmi_unified_t wmi_handle,
		uint8_t macaddr[IEEE80211_ADDR_LEN],
		struct addba_clearresponse_params *param);

QDF_STATUS (*send_addba_send_cmd)(wmi_unified_t wmi_handle,
		uint8_t macaddr[IEEE80211_ADDR_LEN],
		struct addba_send_params *param);

QDF_STATUS (*send_delba_send_cmd)(wmi_unified_t wmi_handle,
		uint8_t macaddr[IEEE80211_ADDR_LEN],
		struct delba_send_params *param);

QDF_STATUS (*send_addba_setresponse_cmd)(wmi_unified_t wmi_handle,
		uint8_t macaddr[IEEE80211_ADDR_LEN],
		struct addba_setresponse_params *param);

QDF_STATUS (*send_singleamsdu_cmd)(wmi_unified_t wmi_handle,
		uint8_t macaddr[IEEE80211_ADDR_LEN],
		struct singleamsdu_params *param);

QDF_STATUS (*send_set_qboost_param_cmd)(wmi_unified_t wmi_handle,
		uint8_t macaddr[IEEE80211_ADDR_LEN],
		struct set_qboost_params *param);

QDF_STATUS (*send_mu_scan_cmd)(wmi_unified_t wmi_handle,
		struct mu_scan_params *param);

QDF_STATUS (*send_lteu_config_cmd)(wmi_unified_t wmi_handle,
		struct lteu_config_params *param);

QDF_STATUS (*send_set_ps_mode_cmd)(wmi_unified_t wmi_handle,
		       struct set_ps_mode_params *param);
QDF_STATUS (*save_service_bitmap)(wmi_unified_t wmi_handle,
		void *evt_buf,  void *bitmap_buf);
QDF_STATUS (*save_ext_service_bitmap)(wmi_unified_t wmi_handle,
		void *evt_buf,  void *bitmap_buf);
bool (*is_service_enabled)(wmi_unified_t wmi_handle,
	uint32_t service_id);
QDF_STATUS (*get_target_cap_from_service_ready)(wmi_unified_t wmi_handle,
	void *evt_buf, struct wlan_psoc_target_capability_info *ev);

QDF_STATUS (*extract_fw_version)(wmi_unified_t wmi_handle,
				void *ev, struct wmi_host_fw_ver *fw_ver);

QDF_STATUS (*extract_fw_abi_version)(wmi_unified_t wmi_handle,
				void *ev, struct wmi_host_fw_abi_ver *fw_ver);

QDF_STATUS (*extract_hal_reg_cap)(wmi_unified_t wmi_handle, void *evt_buf,
	struct wlan_psoc_hal_reg_capability *hal_reg_cap);

host_mem_req * (*extract_host_mem_req)(wmi_unified_t wmi_handle,
	void *evt_buf, uint8_t *num_entries);

QDF_STATUS (*init_cmd_send)(wmi_unified_t wmi_handle,
				struct wmi_init_cmd_param *param);

QDF_STATUS (*save_fw_version)(wmi_unified_t wmi_handle, void *evt_buf);
uint32_t (*ready_extract_init_status)(wmi_unified_t wmi_hdl, void *ev);
QDF_STATUS (*ready_extract_mac_addr)(wmi_unified_t wmi_hdl, void *ev,
		uint8_t *macaddr);
wmi_host_mac_addr * (*ready_extract_mac_addr_list)(wmi_unified_t wmi_hdl,
					void *ev, uint8_t *num_mac_addr);
QDF_STATUS (*extract_ready_event_params)(wmi_unified_t wmi_handle,
		void *evt_buf, struct wmi_host_ready_ev_param *ev_param);

QDF_STATUS (*check_and_update_fw_version)(wmi_unified_t wmi_hdl, void *ev);
uint8_t* (*extract_dbglog_data_len)(wmi_unified_t wmi_handle, void *evt_buf,
		uint32_t *len);
QDF_STATUS (*send_ext_resource_config)(wmi_unified_t wmi_handle,
		wmi_host_ext_resource_config *ext_cfg);

QDF_STATUS (*send_nf_dbr_dbm_info_get_cmd)(wmi_unified_t wmi_handle,
					   uint8_t mac_id);

QDF_STATUS (*send_packet_power_info_get_cmd)(wmi_unified_t wmi_handle,
		      struct packet_power_info_params *param);

QDF_STATUS (*send_gpio_config_cmd)(wmi_unified_t wmi_handle,
		      struct gpio_config_params *param);

QDF_STATUS (*send_gpio_output_cmd)(wmi_unified_t wmi_handle,
		      struct gpio_output_params *param);

QDF_STATUS (*send_rtt_meas_req_test_cmd)(wmi_unified_t wmi_handle,
		      struct rtt_meas_req_test_params *param);

QDF_STATUS (*send_rtt_meas_req_cmd)(wmi_unified_t wmi_handle,
		      struct rtt_meas_req_params *param);

QDF_STATUS (*send_rtt_keepalive_req_cmd)(wmi_unified_t wmi_handle,
		      struct rtt_keepalive_req_params *param);

QDF_STATUS (*send_lci_set_cmd)(wmi_unified_t wmi_handle,
		      struct lci_set_params *param);

QDF_STATUS (*send_lcr_set_cmd)(wmi_unified_t wmi_handle,
		      struct lcr_set_params *param);

QDF_STATUS (*send_periodic_chan_stats_config_cmd)(wmi_unified_t wmi_handle,
			struct periodic_chan_stats_params *param);

QDF_STATUS
(*send_atf_peer_request_cmd)(wmi_unified_t wmi_handle,
			struct atf_peer_request_params *param);

QDF_STATUS
(*send_set_atf_grouping_cmd)(wmi_unified_t wmi_handle,
			struct atf_grouping_params *param);

QDF_STATUS (*send_get_user_position_cmd)(wmi_unified_t wmi_handle,
			uint32_t value);

QDF_STATUS
(*send_reset_peer_mumimo_tx_count_cmd)(wmi_unified_t wmi_handle,
			uint32_t value);

QDF_STATUS (*send_get_peer_mumimo_tx_count_cmd)(wmi_unified_t wmi_handle,
			uint32_t value);

QDF_STATUS
(*send_pdev_caldata_version_check_cmd)(wmi_unified_t wmi_handle,
			uint32_t value);

QDF_STATUS
(*send_btcoex_wlan_priority_cmd)(wmi_unified_t wmi_handle,
			struct btcoex_cfg_params *param);

QDF_STATUS
(*send_start_11d_scan_cmd)(wmi_unified_t wmi_handle,
			struct reg_start_11d_scan_req *param);

QDF_STATUS
(*send_stop_11d_scan_cmd)(wmi_unified_t wmi_handle,
			struct reg_stop_11d_scan_req *param);

QDF_STATUS
(*send_btcoex_duty_cycle_cmd)(wmi_unified_t wmi_handle,
			struct btcoex_cfg_params *param);

QDF_STATUS
(*send_coex_ver_cfg_cmd)(wmi_unified_t wmi_handle, coex_ver_cfg_t *param);

QDF_STATUS
(*send_coex_config_cmd)(wmi_unified_t wmi_handle,
			struct coex_config_params *param);

QDF_STATUS (*send_bcn_offload_control_cmd)(wmi_unified_t wmi_handle,
			struct bcn_offload_control *bcn_ctrl_param);

QDF_STATUS (*extract_wds_addr_event)(wmi_unified_t wmi_handle,
	void *evt_buf, uint16_t len, wds_addr_event_t *wds_ev);

QDF_STATUS (*extract_dcs_interference_type)(wmi_unified_t wmi_handle,
	void *evt_buf, struct wmi_host_dcs_interference_param *param);

QDF_STATUS (*extract_dcs_cw_int)(wmi_unified_t wmi_handle, void *evt_buf,
	wmi_host_ath_dcs_cw_int *cw_int);

QDF_STATUS (*extract_dcs_im_tgt_stats)(wmi_unified_t wmi_handle, void *evt_buf,
	wmi_host_dcs_im_tgt_stats_t *wlan_stat);

QDF_STATUS (*extract_fips_event_data)(wmi_unified_t wmi_handle,
	void *evt_buf, struct wmi_host_fips_event_param *param);

QDF_STATUS (*extract_vdev_start_resp)(wmi_unified_t wmi_handle, void *evt_buf,
	wmi_host_vdev_start_resp *vdev_rsp);

QDF_STATUS (*extract_vdev_delete_resp)(wmi_unified_t wmi_handle, void *evt_buf,
	struct wmi_host_vdev_delete_resp *delete_rsp);

QDF_STATUS (*extract_tbttoffset_update_params)(void *wmi_hdl, void *evt_buf,
	uint8_t idx, struct tbttoffset_params *tbtt_param);

QDF_STATUS (*extract_ext_tbttoffset_update_params)(void *wmi_hdl, void *evt_buf,
	uint8_t idx, struct tbttoffset_params *tbtt_param);

QDF_STATUS (*extract_tbttoffset_num_vdevs)(void *wmi_hdl, void *evt_buf,
					   uint32_t *num_vdevs);

QDF_STATUS (*extract_ext_tbttoffset_num_vdevs)(void *wmi_hdl, void *evt_buf,
					       uint32_t *num_vdevs);

QDF_STATUS (*extract_mgmt_rx_params)(wmi_unified_t wmi_handle, void *evt_buf,
	struct mgmt_rx_event_params *hdr, uint8_t **bufp);

QDF_STATUS (*extract_vdev_stopped_param)(wmi_unified_t wmi_handle,
		void *evt_buf, uint32_t *vdev_id);

QDF_STATUS (*extract_vdev_roam_param)(wmi_unified_t wmi_handle, void *evt_buf,
	wmi_host_roam_event *param);

QDF_STATUS (*extract_vdev_scan_ev_param)(wmi_unified_t wmi_handle,
		void *evt_buf, struct scan_event *param);

#ifdef CONVERGED_TDLS_ENABLE
QDF_STATUS (*extract_vdev_tdls_ev_param)(wmi_unified_t wmi_handle,
		void *evt_buf, struct tdls_event_info *param);
#endif

QDF_STATUS (*extract_mu_ev_param)(wmi_unified_t wmi_handle, void *evt_buf,
	wmi_host_mu_report_event *param);

QDF_STATUS (*extract_mu_db_entry)(wmi_unified_t wmi_hdl, void *evt_buf,
	uint8_t idx, wmi_host_mu_db_entry *param);

QDF_STATUS (*extract_mumimo_tx_count_ev_param)(wmi_unified_t wmi_handle,
	void *evt_buf, wmi_host_peer_txmu_cnt_event *param);

QDF_STATUS (*extract_peer_gid_userpos_list_ev_param)(wmi_unified_t wmi_handle,
	void *evt_buf, wmi_host_peer_gid_userpos_list_event *param);

QDF_STATUS (*extract_pdev_caldata_version_check_ev_param)(
	wmi_unified_t wmi_handle,
	void *evt_buf, wmi_host_pdev_check_cal_version_event *param);

QDF_STATUS (*extract_pdev_tpc_config_ev_param)(wmi_unified_t wmi_handle,
		void *evt_buf, wmi_host_pdev_tpc_config_event *param);

QDF_STATUS (*extract_gpio_input_ev_param)(wmi_unified_t wmi_handle,
	void *evt_buf, uint32_t *gpio_num);

QDF_STATUS (*extract_pdev_reserve_ast_ev_param)(wmi_unified_t wmi_handle,
		void *evt_buf, struct wmi_host_proxy_ast_reserve_param *param);

QDF_STATUS (*extract_nfcal_power_ev_param)(wmi_unified_t wmi_handle,
		void *evt_buf,
		wmi_host_pdev_nfcal_power_all_channels_event *param);

QDF_STATUS (*extract_pdev_tpc_ev_param)(wmi_unified_t wmi_handle,
		void *evt_buf, wmi_host_pdev_tpc_event *param);

QDF_STATUS (*extract_pdev_generic_buffer_ev_param)(wmi_unified_t wmi_handle,
		void *evt_buf, wmi_host_pdev_generic_buffer_event *param);

QDF_STATUS (*extract_mgmt_tx_compl_param)(wmi_unified_t wmi_handle,
		void *evt_buf, wmi_host_mgmt_tx_compl_event *param);

QDF_STATUS (*extract_offchan_data_tx_compl_param)(wmi_unified_t wmi_handle,
		void *evt_buf,
		struct wmi_host_offchan_data_tx_compl_event *param);

QDF_STATUS (*extract_pdev_csa_switch_count_status)(wmi_unified_t wmi_handle,
		void *evt_buf, struct pdev_csa_switch_count_status *param);

QDF_STATUS (*extract_swba_num_vdevs)(wmi_unified_t wmi_handle, void *evt_buf,
	uint32_t *num_vdevs);

QDF_STATUS (*extract_swba_tim_info)(wmi_unified_t wmi_handle, void *evt_buf,
	uint32_t idx, wmi_host_tim_info *tim_info);

QDF_STATUS (*extract_swba_noa_info)(wmi_unified_t wmi_handle, void *evt_buf,
	    uint32_t idx, wmi_host_p2p_noa_info *p2p_desc);

#ifdef CONVERGED_P2P_ENABLE
QDF_STATUS (*extract_p2p_lo_stop_ev_param)(wmi_unified_t wmi_handle,
	void *evt_buf, struct p2p_lo_event *param);

QDF_STATUS (*extract_p2p_noa_ev_param)(wmi_unified_t wmi_handle,
	void *evt_buf, struct p2p_noa_info *param);
#endif

QDF_STATUS (*extract_peer_sta_ps_statechange_ev)(wmi_unified_t wmi_handle,
	void *evt_buf, wmi_host_peer_sta_ps_statechange_event *ev);

QDF_STATUS (*extract_peer_sta_kickout_ev)(wmi_unified_t wmi_handle,
		void *evt_buf, wmi_host_peer_sta_kickout_event *ev);

QDF_STATUS (*extract_peer_ratecode_list_ev)(wmi_unified_t wmi_handle,
		void *evt_buf, uint8_t *peer_mac, wmi_sa_rate_cap *rate_cap);

QDF_STATUS (*extract_comb_phyerr)(wmi_unified_t wmi_handle, void *evt_buf,
	uint16_t datalen, uint16_t *buf_offset, wmi_host_phyerr_t *phyerr);

QDF_STATUS (*extract_single_phyerr)(wmi_unified_t wmi_handle, void *evt_buf,
	uint16_t datalen, uint16_t *buf_offset, wmi_host_phyerr_t *phyerr);

QDF_STATUS (*extract_composite_phyerr)(wmi_unified_t wmi_handle, void *evt_buf,
	uint16_t datalen, wmi_host_phyerr_t *phyerr);

QDF_STATUS (*extract_rtt_hdr)(wmi_unified_t wmi_handle, void *evt_buf,
	wmi_host_rtt_event_hdr *ev);

QDF_STATUS (*extract_rtt_ev)(wmi_unified_t wmi_handle, void *evt_buf,
	wmi_host_rtt_meas_event *ev, uint8_t *hdump, uint16_t hdump_len);

QDF_STATUS (*extract_rtt_error_report_ev)(wmi_unified_t wmi_handle,
		void *evt_buf, wmi_host_rtt_error_report_event *ev);

QDF_STATUS (*extract_all_stats_count)(wmi_unified_t wmi_handle, void *evt_buf,
			   wmi_host_stats_event *stats_param);

QDF_STATUS (*extract_pdev_stats)(wmi_unified_t wmi_handle, void *evt_buf,
			 uint32_t index, wmi_host_pdev_stats *pdev_stats);

QDF_STATUS (*extract_unit_test)(wmi_unified_t wmi_handle, void *evt_buf,
		wmi_unit_test_event *unit_test, uint32_t maxspace);

QDF_STATUS (*extract_pdev_ext_stats)(wmi_unified_t wmi_handle, void *evt_buf,
		 uint32_t index, wmi_host_pdev_ext_stats *pdev_ext_stats);

QDF_STATUS (*extract_vdev_stats)(wmi_unified_t wmi_handle, void *evt_buf,
			 uint32_t index, wmi_host_vdev_stats *vdev_stats);

QDF_STATUS (*extract_peer_stats)(wmi_unified_t wmi_handle, void *evt_buf,
			 uint32_t index, wmi_host_peer_stats *peer_stats);

QDF_STATUS (*extract_bcnflt_stats)(wmi_unified_t wmi_handle, void *evt_buf,
			 uint32_t index, wmi_host_bcnflt_stats *bcnflt_stats);

QDF_STATUS (*extract_peer_extd_stats)(wmi_unified_t wmi_handle, void *evt_buf,
		 uint32_t index, wmi_host_peer_extd_stats *peer_extd_stats);

QDF_STATUS (*extract_chan_stats)(wmi_unified_t wmi_handle, void *evt_buf,
			 uint32_t index, wmi_host_chan_stats *chan_stats);

QDF_STATUS (*extract_thermal_stats)(wmi_unified_t wmi_handle, void *evt_buf,
	uint32_t *temp, uint32_t *level, uint32_t *pdev_id);

QDF_STATUS (*extract_thermal_level_stats)(wmi_unified_t wmi_handle,
		void *evt_buf, uint8_t idx, uint32_t *levelcount,
		uint32_t *dccount);

QDF_STATUS (*extract_profile_ctx)(wmi_unified_t wmi_handle, void *evt_buf,
				   wmi_host_wlan_profile_ctx_t *profile_ctx);

QDF_STATUS (*extract_profile_data)(wmi_unified_t wmi_handle, void *evt_buf,
				uint8_t idx,
				wmi_host_wlan_profile_t *profile_data);

QDF_STATUS (*extract_chan_info_event)(wmi_unified_t wmi_handle, void *evt_buf,
				   wmi_host_chan_info_event *chan_info);

QDF_STATUS (*extract_channel_hopping_event)(wmi_unified_t wmi_handle,
		void *evt_buf,
		wmi_host_pdev_channel_hopping_event *ch_hopping);

QDF_STATUS (*extract_bss_chan_info_event)(wmi_unified_t wmi_handle,
		void *evt_buf,
		wmi_host_pdev_bss_chan_info_event *bss_chan_info);

QDF_STATUS (*extract_inst_rssi_stats_event)(wmi_unified_t wmi_handle,
		void *evt_buf, wmi_host_inst_stats_resp *inst_rssi_resp);

QDF_STATUS (*extract_tx_data_traffic_ctrl_ev)(wmi_unified_t wmi_handle,
		void *evt_buf, wmi_host_tx_data_traffic_ctrl_event *ev);

QDF_STATUS (*extract_atf_peer_stats_ev)(wmi_unified_t wmi_handle,
		void *evt_buf, wmi_host_atf_peer_stats_event *ev);

QDF_STATUS (*extract_atf_token_info_ev)(wmi_unified_t wmi_handle,
		void *evt_buf,
		uint8_t idx,
		wmi_host_atf_peer_stats_info *atf_token_info);

QDF_STATUS (*extract_vdev_extd_stats)(wmi_unified_t wmi_handle, void *evt_buf,
		uint32_t index, wmi_host_vdev_extd_stats *vdev_extd_stats);

QDF_STATUS (*extract_vdev_nac_rssi_stats)(wmi_unified_t wmi_handle, void *evt_buf,
		struct wmi_host_vdev_nac_rssi_event *vdev_nac_rssi_stats);

QDF_STATUS (*extract_bcn_stats)(wmi_unified_t wmi_handle, void *evt_buf,
		uint32_t index, wmi_host_bcn_stats *bcn_stats);

QDF_STATUS (*send_power_dbg_cmd)(wmi_unified_t wmi_handle,
				struct wmi_power_dbg_params *param);

QDF_STATUS (*send_multiple_vdev_restart_req_cmd)(wmi_unified_t wmi_handle,
				struct multiple_vdev_restart_params *param);

QDF_STATUS (*send_adapt_dwelltime_params_cmd)(wmi_unified_t wmi_handle,
			struct wmi_adaptive_dwelltime_params *dwelltime_params);

QDF_STATUS (*send_dbs_scan_sel_params_cmd)(wmi_unified_t wmi_handle,
			struct wmi_dbs_scan_sel_params *dbs_scan_params);

QDF_STATUS (*send_fw_test_cmd)(wmi_unified_t wmi_handle,
			       struct set_fwtest_params *wmi_fwtest);

#ifdef WLAN_FEATURE_DISA
QDF_STATUS (*send_encrypt_decrypt_send_cmd)(wmi_unified_t wmi_handle,
				struct disa_encrypt_decrypt_req_params *params);

QDF_STATUS (*extract_encrypt_decrypt_resp_event)(wmi_unified_t wmi_handle,
			void *evt_buf,
			struct disa_encrypt_decrypt_resp_params *resp);
#endif

QDF_STATUS (*send_sar_limit_cmd)(wmi_unified_t wmi_handle,
				struct sar_limit_cmd_params *params);

QDF_STATUS (*get_sar_limit_cmd)(wmi_unified_t wmi_handle);

QDF_STATUS (*extract_sar_limit_event)(wmi_unified_t wmi_handle,
				      uint8_t *evt_buf,
				      struct sar_limit_event *event);

QDF_STATUS (*send_peer_rx_reorder_queue_setup_cmd)(wmi_unified_t wmi_handle,
		struct rx_reorder_queue_setup_params *param);

QDF_STATUS (*send_peer_rx_reorder_queue_remove_cmd)(wmi_unified_t wmi_handle,
		struct rx_reorder_queue_remove_params *param);

QDF_STATUS (*extract_service_ready_ext)(wmi_unified_t wmi_handle,
			uint8_t *evt_buf,
			struct wlan_psoc_host_service_ext_param *param);

QDF_STATUS (*extract_hw_mode_cap_service_ready_ext)(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf, uint8_t hw_mode_idx,
			struct wlan_psoc_host_hw_mode_caps *param);

QDF_STATUS (*extract_mac_phy_cap_service_ready_ext)(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf,
			uint8_t hw_mode_id,
			uint8_t phy_id,
			struct wlan_psoc_host_mac_phy_caps *param);

QDF_STATUS (*extract_reg_cap_service_ready_ext)(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf, uint8_t phy_idx,
			struct wlan_psoc_host_hal_reg_capabilities_ext *param);

QDF_STATUS (*extract_dbr_ring_cap_service_ready_ext)(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf, uint8_t idx,
			struct wlan_psoc_host_dbr_ring_caps *param);

QDF_STATUS (*extract_dbr_buf_release_fixed)(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf,
			struct direct_buf_rx_rsp *param);

QDF_STATUS (*extract_dbr_buf_release_entry)(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf, uint8_t idx,
			struct direct_buf_rx_entry *param);

QDF_STATUS (*extract_pdev_utf_event)(wmi_unified_t wmi_hdl,
				     uint8_t *evt_buf,
				     struct wmi_host_pdev_utf_event *param);

QDF_STATUS (*extract_pdev_qvit_event)(wmi_unified_t wmi_hdl,
				     uint8_t *evt_buf,
				     struct wmi_host_pdev_qvit_event *param);

uint16_t (*wmi_set_htc_tx_tag)(wmi_unified_t wmi_handle,
				wmi_buf_t buf, uint32_t cmd_id);

QDF_STATUS (*extract_peer_delete_response_event)(
			wmi_unified_t wmi_handle,
			void *evt_buf,
			struct wmi_host_peer_delete_response_event *param);

bool (*is_management_record)(uint32_t cmd_id);
uint8_t *(*wmi_id_to_name)(uint32_t cmd_id);
QDF_STATUS (*send_dfs_phyerr_offload_en_cmd)(wmi_unified_t wmi_handle,
		uint32_t pdev_id);
QDF_STATUS (*send_dfs_phyerr_offload_dis_cmd)(wmi_unified_t wmi_handle,
		uint32_t pdev_id);
QDF_STATUS (*extract_reg_chan_list_update_event)(wmi_unified_t wmi_handle,
						 uint8_t *evt_buf,
						 struct cur_regulatory_info
						 *reg_info,
						 uint32_t len);

QDF_STATUS (*extract_reg_11d_new_country_event)(wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct reg_11d_new_country *reg_11d_country,
		uint32_t len);

QDF_STATUS (*extract_reg_ch_avoid_event)(wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct ch_avoid_ind_type *ch_avoid_event,
		uint32_t len);

QDF_STATUS (*extract_chainmask_tables)(wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wlan_psoc_host_chainmask_table *chainmask_table);

QDF_STATUS (*send_get_rcpi_cmd)(wmi_unified_t wmi_handle,
				struct rcpi_req *get_rcpi_param);

QDF_STATUS (*extract_rcpi_response_event)(wmi_unified_t wmi_handle,
					  void *evt_buf,
					  struct rcpi_res *res);

#ifdef DFS_COMPONENT_ENABLE
QDF_STATUS (*extract_dfs_cac_complete_event)(wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		uint32_t *vdev_id,
		uint32_t len);
QDF_STATUS (*extract_dfs_radar_detection_event)(wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct radar_found_info *radar_found,
		uint32_t len);
QDF_STATUS (*extract_wlan_radar_event_info)(wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct radar_event_info *wlan_radar_event,
		uint32_t len);
#endif
QDF_STATUS (*send_set_country_cmd)(wmi_unified_t wmi_handle,
				struct set_country *param);

uint32_t (*convert_pdev_id_host_to_target)(uint32_t pdev_id);
uint32_t (*convert_pdev_id_target_to_host)(uint32_t pdev_id);

QDF_STATUS (*send_user_country_code_cmd)(wmi_unified_t wmi_handle,
		uint8_t pdev_id, struct cc_regdmn_s *rd);
QDF_STATUS (*send_limit_off_chan_cmd)(wmi_unified_t wmi_handle,
		struct wmi_limit_off_chan_param *limit_off_chan_param);

QDF_STATUS (*send_wow_timer_pattern_cmd)(wmi_unified_t wmi_handle,
			uint8_t vdev_id, uint32_t cookie, uint32_t time);
QDF_STATUS (*send_wds_entry_list_cmd)(wmi_unified_t wmi_handle);
QDF_STATUS (*extract_wds_entry)(wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wdsentry *wds_entry,
		u_int32_t idx);

#ifdef WLAN_FEATURE_NAN_CONVERGENCE
QDF_STATUS (*send_ndp_initiator_req_cmd)(wmi_unified_t wmi_handle,
				struct nan_datapath_initiator_req *req);
QDF_STATUS (*send_ndp_responder_req_cmd)(wmi_unified_t wmi_handle,
				struct nan_datapath_responder_req *req);
QDF_STATUS (*send_ndp_end_req_cmd)(wmi_unified_t wmi_handle,
				struct nan_datapath_end_req *req);

QDF_STATUS (*extract_ndp_initiator_rsp)(wmi_unified_t wmi_handle,
		uint8_t *data, struct nan_datapath_initiator_rsp **rsp);
QDF_STATUS (*extract_ndp_ind)(wmi_unified_t wmi_handle,
		uint8_t *data, struct nan_datapath_indication_event **ind);
QDF_STATUS (*extract_ndp_confirm)(wmi_unified_t wmi_handle,
		uint8_t *data, struct nan_datapath_confirm_event **ev);
QDF_STATUS (*extract_ndp_responder_rsp)(wmi_unified_t wmi_handle,
		uint8_t *data, struct nan_datapath_responder_rsp **rsp);
QDF_STATUS (*extract_ndp_end_rsp)(wmi_unified_t wmi_handle,
		uint8_t *data, struct nan_datapath_end_rsp_event **rsp);
QDF_STATUS (*extract_ndp_end_ind)(wmi_unified_t wmi_handle,
		uint8_t *data, struct nan_datapath_end_indication_event **ind);
#endif
QDF_STATUS (*send_btm_config)(wmi_unified_t wmi_handle,
			      struct wmi_btm_config *params);
QDF_STATUS (*send_obss_detection_cfg_cmd)(wmi_unified_t wmi_handle,
		struct wmi_obss_detection_cfg_param *obss_cfg_param);
QDF_STATUS (*extract_obss_detection_info)(uint8_t *evt_buf,
					  struct wmi_obss_detect_info *info);

#ifdef WLAN_SUPPORT_FILS
QDF_STATUS (*send_vdev_fils_enable_cmd)(wmi_unified_t wmi_handle,
					struct config_fils_params *param);
QDF_STATUS (*extract_swfda_vdev_id)(wmi_unified_t wmi_handle, void *evt_buf,
				    uint32_t *vdev_id);
QDF_STATUS (*send_fils_discovery_send_cmd)(wmi_unified_t wmi_handle,
					   struct fd_params *param);
#endif /* WLAN_SUPPORT_FILS */
QDF_STATUS (*send_offload_11k_cmd)(wmi_unified_t wmi_handle,
		struct wmi_11k_offload_params *params);

QDF_STATUS (*send_invoke_neighbor_report_cmd)(wmi_unified_t wmi_handle,
		struct wmi_invoke_neighbor_report_params *params);

void (*wmi_pdev_id_conversion_enable)(wmi_unified_t wmi_handle);
void (*send_time_stamp_sync_cmd)(wmi_unified_t wmi_handle);
void (*wmi_free_allocated_event)(A_UINT32 cmd_event_id,
				void **wmi_cmd_struct_ptr);
int (*wmi_check_and_pad_event)(void *os_handle, void *param_struc_ptr,
				A_UINT32 param_buf_len,
				A_UINT32 wmi_cmd_event_id,
				void **wmi_cmd_struct_ptr);
int (*wmi_check_command_params)(void *os_handle, void *param_struc_ptr,
				A_UINT32 param_buf_len,
				A_UINT32 wmi_cmd_event_id);
QDF_STATUS (*send_bss_color_change_enable_cmd)(wmi_unified_t wmi_handle,
					       uint32_t vdev_id,
					       bool enable);
QDF_STATUS (*send_obss_color_collision_cfg_cmd)(wmi_unified_t wmi_handle,
		struct wmi_obss_color_collision_cfg_param *cfg);
QDF_STATUS (*extract_obss_color_collision_info)(uint8_t *evt_buf,
		struct wmi_obss_color_collision_info *info);
};

/* Forward declartion for psoc*/
struct wlan_objmgr_psoc;

/**
 * struct wmi_init_cmd - Saved wmi INIT command
 * @buf: Buffer containing the wmi INIT command
 * @buf_len: Length of the buffer
 */
struct wmi_cmd_init {
	wmi_buf_t buf;
	uint32_t buf_len;
};

/**
 * @abi_version_0: WMI Major and Minor versions
 * @abi_version_1: WMI change revision
 * @abi_version_ns_0: ABI version namespace first four dwords
 * @abi_version_ns_1: ABI version namespace second four dwords
 * @abi_version_ns_2: ABI version namespace third four dwords
 * @abi_version_ns_3: ABI version namespace fourth four dwords
 */
struct wmi_host_abi_version {
	uint32_t abi_version_0;
	uint32_t abi_version_1;
	uint32_t abi_version_ns_0;
	uint32_t abi_version_ns_1;
	uint32_t abi_version_ns_2;
	uint32_t abi_version_ns_3;
};

struct wmi_unified {
	void *scn_handle;    /* handle to device */
	osdev_t  osdev; /* handle to use OS-independent services */
	qdf_atomic_t pending_cmds;
	HTC_ENDPOINT_ID wmi_endpoint_id;
	uint16_t max_msg_len;
	uint32_t *event_id;
	wmi_unified_event_handler *event_handler;
	enum wmi_rx_exec_ctx *ctx;
	void *htc_handle;
	qdf_spinlock_t eventq_lock;
	qdf_nbuf_queue_t event_queue;
	qdf_work_t rx_event_work;
	qdf_workqueue_t *wmi_rx_work_queue;
	int wmi_stop_in_progress;
	struct wmi_host_abi_version fw_abi_version;
	struct wmi_host_abi_version final_abi_vers;
	uint32_t num_of_diag_events_logs;
	uint32_t *events_logs_list;
#ifdef WLAN_OPEN_SOURCE
	struct fwdebug dbglog;
	struct dentry *debugfs_phy;
#endif /* WLAN_OPEN_SOURCE */

#ifdef WMI_INTERFACE_EVENT_LOGGING
	struct wmi_debug_log_info log_info;
#endif /*WMI_INTERFACE_EVENT_LOGGING */

	qdf_atomic_t is_target_suspended;

#ifdef FEATURE_RUNTIME_PM
	qdf_atomic_t runtime_pm_inprogress;
#endif
	qdf_atomic_t is_wow_bus_suspended;
	bool tag_crash_inject;
	bool tgt_force_assert_enable;
	enum wmi_target_type target_type;
	struct wmi_rx_ops rx_ops;
	struct wmi_ops *ops;
	bool use_cookie;
	bool wmi_stopinprogress;
	uint32_t *wmi_events;
#ifndef CONFIG_MCL
	uint32_t *pdev_param;
	uint32_t *vdev_param;
#endif
	uint32_t *services;
	struct wmi_soc *soc;
};

#define WMI_MAX_RADIOS 3
struct wmi_soc {
	struct wlan_objmgr_psoc *wmi_psoc;
	void *scn_handle;    /* handle to device */
	qdf_atomic_t num_pdevs;
	enum wmi_target_type target_type;
	void *htc_handle;
	uint32_t event_id[WMI_UNIFIED_MAX_EVENT];
	wmi_unified_event_handler event_handler[WMI_UNIFIED_MAX_EVENT];
	uint32_t max_event_idx;
	enum wmi_rx_exec_ctx ctx[WMI_UNIFIED_MAX_EVENT];
	qdf_spinlock_t ctx_lock;
	struct wmi_unified *wmi_pdev[WMI_MAX_RADIOS];
	HTC_ENDPOINT_ID wmi_endpoint_id[WMI_MAX_RADIOS];
	uint16_t max_msg_len[WMI_MAX_RADIOS];
	struct wmi_ops *ops;
	uint32_t wmi_events[wmi_events_max];
	/* WMI service bitmap recieved from target */
	uint32_t *wmi_service_bitmap;
	uint32_t *wmi_ext_service_bitmap;
#ifndef CONFIG_MCL
	uint32_t pdev_param[wmi_pdev_param_max];
	uint32_t vdev_param[wmi_vdev_param_max];
#endif
	uint32_t services[wmi_services_max];

};

void wmi_unified_register_module(enum wmi_target_type target_type,
			void (*wmi_attach)(wmi_unified_t wmi_handle));
void wmi_tlv_init(void);
void wmi_non_tlv_init(void);
#ifdef WMI_NON_TLV_SUPPORT
/* ONLY_NON_TLV_TARGET:TLV attach dummy function defintion for case when
 * driver supports only NON-TLV target (WIN mainline) */
#define wmi_tlv_attach(x) qdf_print("TLV Unavailable\n")
#else
void wmi_tlv_attach(wmi_unified_t wmi_handle);
#endif
void wmi_non_tlv_attach(wmi_unified_t wmi_handle);

/**
 * wmi_align() - provides word aligned parameter
 * @param: parameter to be aligned
 *
 * Return: word aligned parameter
 */
static inline uint32_t wmi_align(uint32_t param)
{
	return roundup(param, sizeof(uint32_t));
}

/**
 * wmi_vdev_map_to_vdev_id() - Provides vdev id corresponding to idx
 *                             from vdev map
 * @vdev_map: Bitmask containing information of active vdev ids
 * @idx: Index referring to the i'th bit set from LSB in vdev map
 *
 * This API returns the vdev id for the i'th bit set from LSB in vdev map.
 * Index runs through 1 from maximum number of vdevs set in the vdev map
 *
 * Return: vdev id of the vdev object
 */
static inline uint32_t wmi_vdev_map_to_vdev_id(uint32_t vdev_map,
					       uint32_t idx)
{
	uint32_t vdev_count = 0, vdev_set = 0, vdev_id = WLAN_INVALID_VDEV_ID;

	while (vdev_map) {
		vdev_set += (vdev_map & 0x1);
		if (vdev_set == (idx+1)) {
			vdev_id = vdev_count;
			break;
		}
		vdev_map >>= 1;
		vdev_count++;
	}

	return vdev_id;
}

/**
 * wmi_vdev_map_to_num_vdevs() - Provides number of vdevs active based on the
 *                               vdev map received from FW
 * @vdev_map: Bitmask containing information of active vdev ids
 *
 * Return: Number of vdevs set in the vdev bit mask
 */
static inline uint32_t wmi_vdev_map_to_num_vdevs(uint32_t vdev_map)
{
	uint32_t num_vdevs = 0;

	while (vdev_map) {
		num_vdevs += (vdev_map & 0x1);
		vdev_map >>= 1;
	}

	return num_vdevs;
}
#endif
