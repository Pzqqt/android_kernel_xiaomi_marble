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
#include "qdf_atomic.h"
#include <wbuff.h>

#ifdef WLAN_FW_OFFLOAD
#include "wlan_fwol_public_structs.h"
#endif

#ifdef DFS_COMPONENT_ENABLE
#include <wlan_dfs_public_struct.h>
#endif
#include <qdf_threads.h>
#ifdef WLAN_SUPPORT_GREEN_AP
#include "wlan_green_ap_api.h"
#endif

#ifdef WLAN_FEATURE_NAN
#include "nan_public_structs.h"
#endif

#ifdef WLAN_SUPPORT_TWT
#include "wmi_unified_twt_param.h"
#endif

#ifdef WMI_SMART_ANT_SUPPORT
#include "wmi_unified_smart_ant_param.h"
#endif

#ifdef WMI_DBR_SUPPORT
#include "wmi_unified_dbr_param.h"
#endif

#ifdef WMI_ATF_SUPPORT
#include "wmi_unified_atf_param.h"
#endif

#ifdef WLAN_FEATURE_INTEROP_ISSUES_AP
#include <wlan_interop_issues_ap_public_structs.h>
#endif

#define WMI_UNIFIED_MAX_EVENT 0x100

#ifdef WMI_EXT_DBG

#define WMI_EXT_DBG_DIR			"WMI_EXT_DBG"
#define WMI_EXT_DBG_FILE		"wmi_log"
#define WMI_EXT_DBG_FILE_PERM		(QDF_FILE_USR_READ | \
					 QDF_FILE_GRP_READ | \
					 QDF_FILE_OTH_READ)
#define WMI_EXT_DBG_QUEUE_SIZE		1024
#define WMI_EXT_DBG_DUMP_ROW_SIZE	16
#define WMI_EXT_DBG_DUMP_GROUP_SIZE	1


/**
 * enum WMI_MSG_TYPE - WMI message types
 * @ WMI_MSG_TYPE_CMD - Message is of type WMI command
 * @ WMI_MSG_TYPE_EVENT - Message is of type WMI event
 */
enum WMI_MSG_TYPE {
	WMI_MSG_TYPE_CMD = 0,
	WMI_MSG_TYPE_EVENT,
};

/**
 * struct wmi_ext_dbg_msg - WMI command/event msg details
 * @ node - qdf list node of wmi messages
 * @ len - command/event message length
 * @ ts - Time of WMI command/event handling
 * @ WMI_MSG_TYPE - message type
 * @ bug - command/event buffer
 */
struct wmi_ext_dbg_msg {
	qdf_list_node_t node;
	uint32_t len;
	uint64_t ts;
	enum WMI_MSG_TYPE type;
	uint8_t buf[0];
};
#endif /*WMI_EXT_DBG */

#ifdef WMI_INTERFACE_EVENT_LOGGING

#ifndef WMI_EVENT_DEBUG_MAX_ENTRY
#define WMI_EVENT_DEBUG_MAX_ENTRY (1024)
#endif

#define WMI_EVENT_DEBUG_ENTRY_MAX_LENGTH (16)

/* wmi_mgmt commands */
#ifndef WMI_MGMT_EVENT_DEBUG_MAX_ENTRY
#define WMI_MGMT_EVENT_DEBUG_MAX_ENTRY (256)
#endif
/* wmi diag rx events max buffer */
#ifndef WMI_DIAG_RX_EVENT_DEBUG_MAX_ENTRY
#define WMI_DIAG_RX_EVENT_DEBUG_MAX_ENTRY (256)
#endif

#define wmi_alert(params...) QDF_TRACE_FATAL(QDF_MODULE_ID_WMI, ## params)
#define wmi_err(params...) QDF_TRACE_ERROR(QDF_MODULE_ID_WMI, ## params)
#define wmi_warn(params...) QDF_TRACE_WARN(QDF_MODULE_ID_WMI, ## params)
#define wmi_info(params...) QDF_TRACE_INFO(QDF_MODULE_ID_WMI, ## params)
#define wmi_debug(params...) QDF_TRACE_DEBUG(QDF_MODULE_ID_WMI, ## params)

#define wmi_nofl_alert(params...) \
	QDF_TRACE_FATAL_NO_FL(QDF_MODULE_ID_WMI, ## params)
#define wmi_nofl_err(params...) \
	QDF_TRACE_ERROR_NO_FL(QDF_MODULE_ID_WMI, ## params)
#define wmi_nofl_warn(params...) \
	QDF_TRACE_WARN_NO_FL(QDF_MODULE_ID_WMI, ## params)
#define wmi_nofl_info(params...) \
	QDF_TRACE_INFO_NO_FL(QDF_MODULE_ID_WMI, ## params)
#define wmi_nofl_debug(params...) \
	QDF_TRACE_DEBUG_NO_FL(QDF_MODULE_ID_WMI, ## params)

#define wmi_alert_rl(params...) QDF_TRACE_FATAL_RL(QDF_MODULE_ID_WMI, params)
#define wmi_err_rl(params...) QDF_TRACE_ERROR_RL(QDF_MODULE_ID_WMI, params)
#define wmi_warn_rl(params...) QDF_TRACE_WARN_RL(QDF_MODULE_ID_WMI, params)
#define wmi_info_rl(params...) QDF_TRACE_INFO_RL(QDF_MODULE_ID_WMI, params)
#define wmi_debug_rl(params...) QDF_TRACE_DEBUG_RL(QDF_MODULE_ID_WMI, params)

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
 * @wmi_diag_event_log_buf_info - Buffer info for WMI diag event log
 * @wmi_record_lock - Lock WMI recording
 * @wmi_logging_enable - Enable/Disable state for WMI logging
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
	struct wmi_log_buf_t wmi_diag_event_log_buf_info;

	qdf_spinlock_t wmi_record_lock;
	bool wmi_logging_enable;
	struct dentry *wmi_log_debugfs_dir;
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
				 uint8_t macaddr[QDF_MAC_ADDR_SIZE],
				 struct vdev_create_params *param);

QDF_STATUS (*send_vdev_delete_cmd)(wmi_unified_t wmi_handle,
					  uint8_t if_id);

QDF_STATUS (*send_vdev_nss_chain_params_cmd)(wmi_unified_t wmi_handle,
					 uint8_t vdev_id,
					 struct vdev_nss_chains *user_cfg);

QDF_STATUS (*send_vdev_stop_cmd)(wmi_unified_t wmi,
					uint8_t vdev_id);

QDF_STATUS (*send_vdev_down_cmd)(wmi_unified_t wmi,
			uint8_t vdev_id);

QDF_STATUS (*send_vdev_start_cmd)(wmi_unified_t wmi,
		struct vdev_start_params *req);

QDF_STATUS (*send_vdev_set_nac_rssi_cmd)(wmi_unified_t wmi,
		struct vdev_scan_nac_rssi_params *req);

QDF_STATUS (*send_peer_flush_tids_cmd)(wmi_unified_t wmi,
					 uint8_t peer_addr[QDF_MAC_ADDR_SIZE],
					 struct peer_flush_params *param);

QDF_STATUS (*send_peer_delete_cmd)(wmi_unified_t wmi,
				    uint8_t peer_addr[QDF_MAC_ADDR_SIZE],
				    uint8_t vdev_id);

QDF_STATUS (*send_peer_delete_all_cmd)(
				wmi_unified_t wmi,
				struct peer_delete_all_params *param);

QDF_STATUS
(*send_peer_unmap_conf_cmd)(wmi_unified_t wmi,
			    uint8_t vdev_id,
			    uint32_t peer_id_cnt,
			    uint16_t *peer_id_list);

QDF_STATUS (*send_peer_param_cmd)(wmi_unified_t wmi,
				uint8_t peer_addr[QDF_MAC_ADDR_SIZE],
				struct peer_set_params *param);

QDF_STATUS (*send_vdev_up_cmd)(wmi_unified_t wmi,
			     uint8_t bssid[QDF_MAC_ADDR_SIZE],
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

QDF_STATUS
(*send_pdev_set_hw_mode_cmd)(wmi_unified_t wmi_handle,
			     uint32_t hw_mode_index);

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

#ifdef FEATURE_BLACKLIST_MGR
QDF_STATUS
(*send_reject_ap_list_cmd)(struct wmi_unified *wmi_handle,
			   struct reject_ap_params *reject_params);
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

QDF_STATUS (*send_vdev_sifs_trigger_cmd)(wmi_unified_t wmi_handle,
					 struct sifs_trigger_param *param);

QDF_STATUS (*send_stats_request_cmd)(wmi_unified_t wmi_handle,
				uint8_t macaddr[QDF_MAC_ADDR_SIZE],
				struct stats_request_params *param);

QDF_STATUS (*send_packet_log_enable_cmd)(wmi_unified_t wmi_handle,
			WMI_HOST_PKTLOG_EVENT PKTLOG_EVENT, uint8_t mac_id);

QDF_STATUS (*send_packet_log_disable_cmd)(wmi_unified_t wmi_handle,
	uint8_t mac_id);

QDF_STATUS (*send_fd_tmpl_cmd)(wmi_unified_t wmi_handle,
				struct fils_discovery_tmpl_params *param);

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

QDF_STATUS (*send_idle_roam_monitor_cmd)(wmi_unified_t wmi_handle,
					 uint8_t val);

QDF_STATUS (*send_get_temperature_cmd)(wmi_unified_t wmi_handle);

#ifdef CONVERGED_P2P_ENABLE
QDF_STATUS (*send_set_p2pgo_oppps_req_cmd)(wmi_unified_t wmi_handle,
		struct p2p_ps_params *oppps);

QDF_STATUS (*send_set_p2pgo_noa_req_cmd)(wmi_unified_t wmi_handle,
			struct p2p_ps_params *noa);

#ifdef FEATURE_P2P_LISTEN_OFFLOAD
QDF_STATUS (*send_p2p_lo_start_cmd)(wmi_unified_t wmi_handle,
			struct p2p_lo_start *param);

QDF_STATUS (*send_p2p_lo_stop_cmd)(wmi_unified_t wmi_handle,
			uint8_t vdev_id);
#endif /* FEATURE_P2P_LISTEN_OFFLOAD */
#endif /* CONVERGED_P2P_ENABLE */

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
#endif /* WLAN_FEATURE_DSRC */
QDF_STATUS (*send_lro_config_cmd)(wmi_unified_t wmi_handle,
	 struct wmi_lro_config_cmd_t *wmi_lro_cmd);

QDF_STATUS (*send_set_thermal_mgmt_cmd)(wmi_unified_t wmi_handle,
				struct thermal_cmd_params *thermal_info);

QDF_STATUS (*send_peer_rate_report_cmd)(wmi_unified_t wmi_handle,
	 struct wmi_peer_rate_report_params *rate_report_params);

#ifdef WMI_CONCURRENCY_SUPPORT
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
#endif /* WMI_CONCURRENCY_SUPPORT */

QDF_STATUS (*send_p2p_go_set_beacon_ie_cmd)(wmi_unified_t wmi_handle,
				    uint32_t vdev_id, uint8_t *p2p_ie);

QDF_STATUS (*send_probe_rsp_tmpl_send_cmd)(wmi_unified_t wmi_handle,
			     uint8_t vdev_id,
			     struct wmi_probe_resp_params *probe_rsp_info);

QDF_STATUS (*send_setup_install_key_cmd)(wmi_unified_t wmi_handle,
				struct set_key_params *key_params);

QDF_STATUS (*send_reset_passpoint_network_list_cmd)(wmi_unified_t wmi_handle,
					struct wifi_passpoint_req_param *req);

#ifdef WMI_ROAM_SUPPORT
#ifdef FEATURE_LFR_SUBNET_DETECTION
QDF_STATUS (*send_set_gateway_params_cmd)(wmi_unified_t wmi_handle,
					struct gateway_update_req_param *req);
#endif /* FEATURE_LFR_SUBNET_DETECTION */

#ifdef FEATURE_RSSI_MONITOR
QDF_STATUS (*send_set_rssi_monitoring_cmd)(wmi_unified_t wmi_handle,
					struct rssi_monitor_param *req);
#endif /* FEATURE_RSSI_MONITOR */

QDF_STATUS (*send_roam_scan_offload_rssi_thresh_cmd)(wmi_unified_t wmi_handle,
				struct roam_offload_scan_rssi_params *roam_req);

QDF_STATUS (*send_roam_mawc_params_cmd)(wmi_unified_t wmi_handle,
		struct wmi_mawc_roam_params *params);

QDF_STATUS (*send_roam_scan_filter_cmd)(wmi_unified_t wmi_handle,
				struct roam_scan_filter_params *roam_req);

QDF_STATUS (*send_roam_scan_offload_mode_cmd)(wmi_unified_t wmi_handle,
				wmi_start_scan_cmd_fixed_param *scan_cmd_fp,
				struct roam_offload_scan_params *roam_req);

QDF_STATUS (*send_roam_scan_offload_ap_profile_cmd)(wmi_unified_t wmi_handle,
				    struct ap_profile_params *ap_profile);

QDF_STATUS (*send_roam_scan_offload_cmd)(wmi_unified_t wmi_handle,
				 uint32_t command, uint32_t vdev_id);

QDF_STATUS (*send_roam_scan_offload_scan_period_cmd)(wmi_unified_t wmi_handle,
					struct roam_scan_period_params *params);

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

QDF_STATUS (*send_offload_11k_cmd)(wmi_unified_t wmi_handle,
		struct wmi_11k_offload_params *params);

QDF_STATUS (*send_invoke_neighbor_report_cmd)(wmi_unified_t wmi_handle,
		struct wmi_invoke_neighbor_report_params *params);

QDF_STATUS (*send_roam_bss_load_config)(wmi_unified_t wmi_handle,
					struct wmi_bss_load_config *params);

QDF_STATUS (*send_disconnect_roam_params)(
			wmi_unified_t wmi_handle,
			struct wmi_disconnect_roam_params *req);

QDF_STATUS (*send_idle_roam_params)(wmi_unified_t wmi_handle,
				    struct wmi_idle_roam_params *req);

QDF_STATUS (*send_roam_preauth_status)(wmi_unified_t wmi_handle,
				struct wmi_roam_auth_status_params *params);

QDF_STATUS (*send_btm_config)(wmi_unified_t wmi_handle,
			      struct wmi_btm_config *params);

QDF_STATUS (*send_limit_off_chan_cmd)(wmi_unified_t wmi_handle,
		struct wmi_limit_off_chan_param *limit_off_chan_param);

#ifdef WLAN_FEATURE_FILS_SK
QDF_STATUS (*send_roam_scan_hlp_cmd) (wmi_unified_t wmi_handle,
				struct hlp_params *params);
#endif /* WLAN_FEATURE_FILS_SK */

#ifdef FEATURE_WLAN_ESE
QDF_STATUS (*send_plm_stop_cmd)(wmi_unified_t wmi_handle,
		 const struct plm_req_params *plm);

QDF_STATUS (*send_plm_start_cmd)(wmi_unified_t wmi_handle,
				 const struct plm_req_params *plm);
#endif /* FEATURE_WLAN_ESE */

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
QDF_STATUS (*send_set_ric_req_cmd)(wmi_unified_t wmi_handle, void *msg,
			uint8_t is_add_ts);

QDF_STATUS (*send_process_roam_synch_complete_cmd)(wmi_unified_t wmi_handle,
		 uint8_t vdev_id);

QDF_STATUS (*send_roam_invoke_cmd)(wmi_unified_t wmi_handle,
		struct wmi_roam_invoke_cmd *roaminvoke,
		uint32_t ch_hz);
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */
#endif /* WMI_ROAM_SUPPORT */

QDF_STATUS (*send_scan_probe_setoui_cmd)(wmi_unified_t wmi_handle,
			  struct scan_mac_oui *psetoui);

QDF_STATUS (*send_set_passpoint_network_list_cmd)(wmi_unified_t wmi_handle,
					struct wifi_passpoint_req_param *req);

QDF_STATUS (*send_set_epno_network_list_cmd)(wmi_unified_t wmi_handle,
		struct wifi_enhanced_pno_params *req);

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

QDF_STATUS (*send_extscan_start_hotlist_monitor_cmd)(wmi_unified_t wmi_handle,
		struct extscan_bssid_hotlist_set_params *params);

QDF_STATUS (*send_stop_extscan_cmd)(wmi_unified_t wmi_handle,
		  struct extscan_stop_req_params *pstopcmd);

QDF_STATUS (*send_start_extscan_cmd)(wmi_unified_t wmi_handle,
		    struct wifi_scan_cmd_req_params *pstart);

QDF_STATUS (*send_csa_offload_enable_cmd)(wmi_unified_t wmi_handle,
			uint8_t vdev_id);

QDF_STATUS (*send_pno_stop_cmd)(wmi_unified_t wmi_handle, uint8_t vdev_id);

QDF_STATUS (*send_pno_start_cmd)(wmi_unified_t wmi_handle,
		   struct pno_scan_req_params *pno);

QDF_STATUS (*send_nlo_mawc_cmd)(wmi_unified_t wmi_handle,
		struct nlo_mawc_params *params);

#ifdef IPA_OFFLOAD
QDF_STATUS (*send_ipa_offload_control_cmd)(wmi_unified_t wmi_handle,
		struct ipa_uc_offload_control_params *ipa_offload);
#endif

#ifdef WLAN_FEATURE_LINK_LAYER_STATS
QDF_STATUS (*send_process_ll_stats_clear_cmd)(wmi_unified_t wmi_handle,
				const struct ll_stats_clear_params *clear_req);

QDF_STATUS (*send_process_ll_stats_set_cmd)(wmi_unified_t wmi_handle,
				const struct ll_stats_set_params *set_req);

QDF_STATUS (*send_process_ll_stats_get_cmd)(wmi_unified_t wmi_handle,
				const struct ll_stats_get_params *get_req);
#endif

QDF_STATUS (*send_congestion_cmd)(wmi_unified_t wmi_handle,
				  uint8_t vdev_id);

QDF_STATUS (*send_snr_request_cmd)(wmi_unified_t wmi_handle);

QDF_STATUS (*send_snr_cmd)(wmi_unified_t wmi_handle, uint8_t vdev_id);

QDF_STATUS (*send_link_status_req_cmd)(wmi_unified_t wmi_handle,
				 struct link_status_params *link_status);
#ifdef WLAN_POWER_MANAGEMENT_OFFLOAD
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

QDF_STATUS (*send_wow_delete_pattern_cmd)(wmi_unified_t wmi_handle,
					  uint8_t ptrn_id,
					  uint8_t vdev_id);

QDF_STATUS (*send_host_wakeup_ind_to_fw_cmd)(wmi_unified_t wmi_handle);

QDF_STATUS (*send_wow_timer_pattern_cmd)(wmi_unified_t wmi_handle,
			uint8_t vdev_id, uint32_t cookie, uint32_t time);
#ifdef FEATURE_WLAN_LPHB
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
#endif /* FEATURE_WLAN_LPHB */
#ifdef WLAN_FEATURE_PACKET_FILTERING
QDF_STATUS (*send_enable_disable_packet_filter_cmd)(wmi_unified_t wmi_handle,
					uint8_t vdev_id, bool enable);

QDF_STATUS (*send_config_packet_filter_cmd)(wmi_unified_t wmi_handle,
		uint8_t vdev_id, struct pmo_rcv_pkt_fltr_cfg *rcv_filter_param,
		uint8_t filter_id, bool enable);
#endif
#endif /* end of WLAN_POWER_MANAGEMENT_OFFLOAD */
#ifdef WLAN_WMI_BCN
QDF_STATUS (*send_bcn_buf_ll_cmd)(wmi_unified_t wmi_handle,
			wmi_bcn_send_from_host_cmd_fixed_param * param);
#endif

#if !defined(REMOVE_PKT_LOG) && defined(FEATURE_PKTLOG)
QDF_STATUS (*send_pktlog_wmi_send_cmd)(wmi_unified_t wmi_handle,
				   WMI_PKTLOG_EVENT pktlog_event,
				   WMI_CMD_ID cmd_id, uint8_t user_triggered);
#endif

#ifdef WLAN_SUPPORT_GREEN_AP
QDF_STATUS (*send_egap_conf_params_cmd)(wmi_unified_t wmi_handle,
			struct wlan_green_ap_egap_params *egap_params);
#endif

#ifdef WLAN_FEATURE_CIF_CFR
QDF_STATUS (*send_oem_dma_cfg_cmd)(wmi_unified_t wmi_handle,
				   wmi_oem_dma_ring_cfg_req_fixed_param *cfg);
#endif

QDF_STATUS (*send_start_oem_data_cmd)(wmi_unified_t wmi_handle,
				      uint32_t data_len,
				      uint8_t *data);

#ifdef FEATURE_OEM_DATA
QDF_STATUS (*send_start_oemv2_data_cmd)(wmi_unified_t wmi_handle,
					struct oem_data *params);
#endif

QDF_STATUS
(*send_dfs_phyerr_filter_offload_en_cmd)(wmi_unified_t wmi_handle,
			bool dfs_phyerr_filter_offload);

QDF_STATUS (*send_bss_color_change_enable_cmd)(wmi_unified_t wmi_handle,
					       uint32_t vdev_id,
					       bool enable);

QDF_STATUS (*send_obss_color_collision_cfg_cmd)(wmi_unified_t wmi_handle,
		struct wmi_obss_color_collision_cfg_param *cfg);

QDF_STATUS (*extract_obss_color_collision_info)(uint8_t *evt_buf,
		struct wmi_obss_color_collision_info *info);

QDF_STATUS (*send_peer_based_pktlog_cmd)(wmi_unified_t wmi_handle,
					 uint8_t *macaddr,
					 uint8_t mac_id,
					 uint8_t enb_dsb);

#ifdef WMI_STA_SUPPORT
QDF_STATUS (*send_del_ts_cmd)(wmi_unified_t wmi_handle, uint8_t vdev_id,
				uint8_t ac);

QDF_STATUS (*send_aggr_qos_cmd)(wmi_unified_t wmi_handle,
		      struct aggr_add_ts_param *aggr_qos_rsp_msg);

QDF_STATUS (*send_add_ts_cmd)(wmi_unified_t wmi_handle,
		 struct add_ts_param *msg);

QDF_STATUS (*send_process_add_periodic_tx_ptrn_cmd)(
					wmi_unified_t wmi_handle,
					struct periodic_tx_pattern *pattern,
					uint8_t vdev_id);

QDF_STATUS (*send_process_del_periodic_tx_ptrn_cmd)(wmi_unified_t wmi_handle,
						    uint8_t vdev_id,
						    uint8_t pattern_id);

QDF_STATUS (*send_set_auto_shutdown_timer_cmd)(wmi_unified_t wmi_handle,
						  uint32_t timer_val);

#ifdef WLAN_FEATURE_NAN
QDF_STATUS (*send_nan_req_cmd)(wmi_unified_t wmi_handle,
			struct nan_msg_params *nan_req);

QDF_STATUS (*send_nan_disable_req_cmd)(wmi_unified_t wmi_handle,
				       struct nan_disable_req *nan_msg);

QDF_STATUS (*extract_nan_event_rsp)(wmi_unified_t wmi_handle, void *evt_buf,
				    struct nan_event_params *evt_params,
				    uint8_t **msg_buf);
#endif

QDF_STATUS (*send_process_ch_avoid_update_cmd)(wmi_unified_t wmi_handle);

QDF_STATUS (*send_process_set_ie_info_cmd)(wmi_unified_t wmi_handle,
				   struct vdev_ie_info_param *ie_info);

QDF_STATUS (*send_set_base_macaddr_indicate_cmd)(wmi_unified_t wmi_handle,
					 uint8_t *custom_addr);

QDF_STATUS (*send_pdev_set_pcl_cmd)(wmi_unified_t wmi_handle,
				struct wmi_pcl_chan_weights *msg);

#ifdef WLAN_POLICY_MGR_ENABLE
QDF_STATUS (*send_pdev_set_dual_mac_config_cmd)(wmi_unified_t wmi_handle,
		struct policy_mgr_dual_mac_config *msg);
#endif

QDF_STATUS (*send_set_led_flashing_cmd)(wmi_unified_t wmi_handle,
				struct flashing_req_params *flashing);

QDF_STATUS (*send_set_arp_stats_req_cmd)(wmi_unified_t wmi_handle,
					 struct set_arp_stats *req_buf);

QDF_STATUS (*send_get_arp_stats_req_cmd)(wmi_unified_t wmi_handle,
					 struct get_arp_stats *req_buf);

QDF_STATUS (*send_set_del_pmkid_cache_cmd) (wmi_unified_t wmi_handle,
		struct wmi_unified_pmk_cache *req_buf);

QDF_STATUS (*send_adapt_dwelltime_params_cmd)(wmi_unified_t wmi_handle,
			struct wmi_adaptive_dwelltime_params *dwelltime_params);

QDF_STATUS (*send_dbs_scan_sel_params_cmd)(wmi_unified_t wmi_handle,
			struct wmi_dbs_scan_sel_params *dbs_scan_params);

QDF_STATUS (*send_vdev_set_gtx_cfg_cmd)(wmi_unified_t wmi_handle,
				  uint32_t if_id,
				  struct wmi_gtx_config *gtx_info);

QDF_STATUS (*send_set_sta_keep_alive_cmd)(wmi_unified_t wmi_handle,
					  struct sta_keep_alive_params *params);

QDF_STATUS (*send_set_sta_sa_query_param_cmd)(wmi_unified_t wmi_handle,
					uint8_t vdev_id, uint32_t max_retries,
					uint32_t retry_interval);

QDF_STATUS (*send_fw_profiling_cmd)(wmi_unified_t wmi_handle,
			uint32_t cmd, uint32_t value1, uint32_t value2);

QDF_STATUS (*send_nat_keepalive_en_cmd)(wmi_unified_t wmi_handle,
					uint8_t vdev_id);

QDF_STATUS (*send_process_dhcp_ind_cmd)(wmi_unified_t wmi_handle,
			wmi_peer_set_param_cmd_fixed_param *ta_dhcp_ind);

QDF_STATUS (*send_get_link_speed_cmd)(wmi_unified_t wmi_handle,
			wmi_mac_addr peer_macaddr);

QDF_STATUS (*send_wlm_latency_level_cmd)(wmi_unified_t wmi_handle,
				struct wlm_latency_level_param *param);

QDF_STATUS (*send_sar_limit_cmd)(wmi_unified_t wmi_handle,
				struct sar_limit_cmd_params *params);

QDF_STATUS (*get_sar_limit_cmd)(wmi_unified_t wmi_handle);

QDF_STATUS (*extract_sar_limit_event)(wmi_unified_t wmi_handle,
				      uint8_t *evt_buf,
				      struct sar_limit_event *event);

QDF_STATUS (*extract_sar2_result_event)(void *handle,
					uint8_t *event,
					uint32_t len);

#ifdef FEATURE_WLAN_TDLS
QDF_STATUS (*send_set_tdls_offchan_mode_cmd)(wmi_unified_t wmi_handle,
			      struct tdls_channel_switch_params *chan_switch_params);

QDF_STATUS (*send_update_fw_tdls_state_cmd)(wmi_unified_t wmi_handle,
					    struct tdls_info *tdls_param,
					    enum wmi_tdls_state tdls_state);

QDF_STATUS (*send_update_tdls_peer_state_cmd)(wmi_unified_t wmi_handle,
				struct tdls_peer_update_state *peer_state,
				uint32_t *ch_mhz);

QDF_STATUS (*extract_vdev_tdls_ev_param)(wmi_unified_t wmi_handle,
		void *evt_buf, struct tdls_event_info *param);
#endif /* FEATURE_WLAN_TDLS */
#endif /* WMI_STA_SUPPORT */

QDF_STATUS (*send_stats_ext_req_cmd)(wmi_unified_t wmi_handle,
			struct stats_ext_params *preq);

#ifdef WLAN_FEATURE_EXTWOW_SUPPORT
QDF_STATUS (*send_enable_ext_wow_cmd)(wmi_unified_t wmi_handle,
			struct ext_wow_params *params);

QDF_STATUS (*send_set_app_type2_params_in_fw_cmd)(wmi_unified_t wmi_handle,
				struct app_type2_params *appType2Params);

QDF_STATUS (*send_app_type1_params_in_fw_cmd)(wmi_unified_t wmi_handle,
				struct app_type1_params *app_type1_params);
#endif /* WLAN_FEATURE_EXTWOW_SUPPORT */

QDF_STATUS (*send_process_dhcpserver_offload_cmd)(wmi_unified_t wmi_handle,
				struct dhcp_offload_info_params *params);

QDF_STATUS (*send_regdomain_info_to_fw_cmd)(wmi_unified_t wmi_handle,
				   uint32_t reg_dmn, uint16_t regdmn2G,
				   uint16_t regdmn5G, uint8_t ctl2G,
				   uint8_t ctl5G);

QDF_STATUS (*send_process_fw_mem_dump_cmd)(wmi_unified_t wmi_handle,
					struct fw_dump_req_param *mem_dump_req);

QDF_STATUS (*send_cfg_action_frm_tb_ppdu_cmd)(wmi_unified_t wmi_handle,
				struct cfg_action_frm_tb_ppdu_param *cfg_info);

QDF_STATUS (*save_fw_version_cmd)(wmi_unified_t wmi_handle, void *evt_buf);

QDF_STATUS (*check_and_update_fw_version_cmd)(wmi_unified_t wmi_hdl, void *ev);

QDF_STATUS (*send_log_supported_evt_cmd)(wmi_unified_t wmi_handle,
		uint8_t *event,
		uint32_t len);

QDF_STATUS (*send_enable_specific_fw_logs_cmd)(wmi_unified_t wmi_handle,
		struct wmi_wifi_start_log *start_log);

QDF_STATUS (*send_flush_logs_to_fw_cmd)(wmi_unified_t wmi_handle);

QDF_STATUS (*send_unit_test_cmd)(wmi_unified_t wmi_handle,
				 struct wmi_unit_test_cmd *wmi_utest);

#ifdef FEATURE_WLAN_APF
QDF_STATUS
(*send_set_active_apf_mode_cmd)(wmi_unified_t wmi_handle, uint8_t vdev_id,
				enum wmi_host_active_apf_mode ucast_mode,
				enum wmi_host_active_apf_mode mcast_bcast_mode);

QDF_STATUS (*send_apf_enable_cmd)(wmi_unified_t wmi_handle, uint32_t vdev_id,
				  bool enable);

QDF_STATUS (*send_apf_write_work_memory_cmd)(wmi_unified_t wmi_handle,
			struct wmi_apf_write_memory_params *apf_write_params);

QDF_STATUS (*send_apf_read_work_memory_cmd)(wmi_unified_t wmi_handle,
			struct wmi_apf_read_memory_params *apf_read_params);

QDF_STATUS (*extract_apf_read_memory_resp_event)(wmi_unified_t wmi_handle,
			void *evt_buf,
			struct wmi_apf_read_memory_resp_event_params *resp);
#endif /* FEATURE_WLAN_APF */

QDF_STATUS (*send_pdev_get_tpc_config_cmd)(wmi_unified_t wmi_handle,
		uint32_t param);

#ifdef WMI_ATF_SUPPORT
QDF_STATUS (*send_set_bwf_cmd)(wmi_unified_t wmi_handle,
		struct set_bwf_params *param);
#endif

QDF_STATUS (*send_pdev_fips_cmd)(wmi_unified_t wmi_handle,
		struct fips_params *param);

QDF_STATUS (*send_wlan_profile_enable_cmd)(wmi_unified_t wmi_handle,
		struct wlan_profile_params *param);

#ifdef WLAN_FEATURE_DISA
QDF_STATUS
(*send_encrypt_decrypt_send_cmd)(wmi_unified_t wmi_handle,
				 struct disa_encrypt_decrypt_req_params
				 *params);
#endif

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
		uint8_t vdev_id, bool mu_edca_param,
		struct wmi_host_wme_vparams wmm_vparams[WMI_MAX_NUM_AC]);

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

#ifdef WMI_SMART_ANT_SUPPORT
QDF_STATUS (*send_set_ant_switch_tbl_cmd)(wmi_unified_t wmi_handle,
		struct ant_switch_tbl_params *param);

QDF_STATUS (*send_smart_ant_enable_cmd)(wmi_unified_t wmi_handle,
		struct smart_ant_enable_params *param);

QDF_STATUS (*send_smart_ant_set_rx_ant_cmd)(wmi_unified_t wmi_handle,
		struct smart_ant_rx_ant_params *param);

QDF_STATUS (*send_smart_ant_set_tx_ant_cmd)(wmi_unified_t wmi_handle,
		uint8_t macaddr[QDF_MAC_ADDR_SIZE],
		struct smart_ant_tx_ant_params *param);

QDF_STATUS (*send_smart_ant_set_training_info_cmd)(wmi_unified_t wmi_handle,
		uint8_t macaddr[QDF_MAC_ADDR_SIZE],
		struct smart_ant_training_info_params *param);

QDF_STATUS (*send_smart_ant_set_node_config_cmd)(wmi_unified_t wmi_handle,
		uint8_t macaddr[QDF_MAC_ADDR_SIZE],
		struct smart_ant_node_config_params *param);
#endif

QDF_STATUS (*send_smart_ant_enable_tx_feedback_cmd)(wmi_unified_t wmi_handle,
		struct smart_ant_enable_tx_feedback_params *param);

QDF_STATUS (*send_vdev_spectral_configure_cmd)(wmi_unified_t wmi_handle,
		struct vdev_spectral_configure_params *param);

QDF_STATUS (*send_vdev_spectral_enable_cmd)(wmi_unified_t wmi_handle,
		struct vdev_spectral_enable_params *param);

QDF_STATUS (*send_bss_chan_info_request_cmd)(wmi_unified_t wmi_handle,
		struct bss_chan_info_request_params *param);

QDF_STATUS (*send_thermal_mitigation_param_cmd)(wmi_unified_t wmi_handle,
		struct thermal_mitigation_params *param);

QDF_STATUS (*send_vdev_set_neighbour_rx_cmd)(wmi_unified_t wmi_handle,
		uint8_t macaddr[QDF_MAC_ADDR_SIZE],
		struct set_neighbour_rx_params *param);

QDF_STATUS (*send_vdev_set_fwtest_param_cmd)(wmi_unified_t wmi_handle,
		struct set_fwtest_params *param);

QDF_STATUS (*send_vdev_config_ratemask_cmd)(wmi_unified_t wmi_handle,
		struct config_ratemask_params *param);

QDF_STATUS (*send_vdev_set_custom_aggr_size_cmd)(wmi_unified_t wmi_handle,
		struct set_custom_aggr_size_params *param);

QDF_STATUS (*send_vdev_set_qdepth_thresh_cmd)(wmi_unified_t wmi_handle,
		struct set_qdepth_thresh_params *param);

QDF_STATUS (*send_peer_chan_width_switch_cmd)(wmi_unified_t wmi_handle,
		struct peer_chan_width_switch_params *param);

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

QDF_STATUS (*send_set_bcn_offload_quiet_mode_cmd)(wmi_unified_t wmi_handle,
		struct set_bcn_offload_quiet_mode_params *param);

QDF_STATUS (*send_set_beacon_filter_cmd)(wmi_unified_t wmi_handle,
		struct set_beacon_filter_params *param);

QDF_STATUS (*send_remove_beacon_filter_cmd)(wmi_unified_t wmi_handle,
		struct remove_beacon_filter_params *param);
/*
QDF_STATUS (*send_mgmt_cmd)(wmi_unified_t wmi_handle,
		uint8_t macaddr[QDF_MAC_ADDR_SIZE],
		struct mgmt_params *param);
		*/

QDF_STATUS (*send_addba_clearresponse_cmd)(wmi_unified_t wmi_handle,
		uint8_t macaddr[QDF_MAC_ADDR_SIZE],
		struct addba_clearresponse_params *param);

QDF_STATUS (*send_addba_send_cmd)(wmi_unified_t wmi_handle,
		uint8_t macaddr[QDF_MAC_ADDR_SIZE],
		struct addba_send_params *param);

QDF_STATUS (*send_delba_send_cmd)(wmi_unified_t wmi_handle,
		uint8_t macaddr[QDF_MAC_ADDR_SIZE],
		struct delba_send_params *param);

QDF_STATUS (*send_addba_setresponse_cmd)(wmi_unified_t wmi_handle,
		uint8_t macaddr[QDF_MAC_ADDR_SIZE],
		struct addba_setresponse_params *param);

QDF_STATUS (*send_singleamsdu_cmd)(wmi_unified_t wmi_handle,
		uint8_t macaddr[QDF_MAC_ADDR_SIZE],
		struct singleamsdu_params *param);

QDF_STATUS (*send_set_qboost_param_cmd)(wmi_unified_t wmi_handle,
		uint8_t macaddr[QDF_MAC_ADDR_SIZE],
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

#ifdef WLAN_ATF_ENABLE
QDF_STATUS (*send_set_atf_cmd)(wmi_unified_t wmi_handle,
			       struct set_atf_params *param);

QDF_STATUS
(*send_atf_peer_request_cmd)(wmi_unified_t wmi_handle,
			     struct atf_peer_request_params *param);

QDF_STATUS
(*send_set_atf_grouping_cmd)(wmi_unified_t wmi_handle,
			     struct atf_grouping_params *param);

QDF_STATUS
(*send_set_atf_group_ac_cmd)(wmi_unified_t wmi_handle,
			     struct atf_group_ac_params *param);

QDF_STATUS (*extract_atf_peer_stats_ev)(wmi_unified_t wmi_handle,
					void *evt_buf,
					wmi_host_atf_peer_stats_event *ev);

QDF_STATUS (*extract_atf_token_info_ev)(wmi_unified_t wmi_handle,
					void *evt_buf, uint8_t idx,
					wmi_host_atf_peer_stats_info *atf_info);
#endif

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
#ifdef OL_ATH_SMART_LOGGING
QDF_STATUS
(*send_smart_logging_enable_cmd)(wmi_unified_t wmi_handle, uint32_t param);

QDF_STATUS
(*send_smart_logging_fatal_cmd)(wmi_unified_t wmi_handle,
				struct wmi_debug_fatal_events *param);
#endif /* OL_ATH_SMART_LOGGING */
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

#ifdef WLAN_FEATURE_DISA
QDF_STATUS
(*extract_encrypt_decrypt_resp_event)(wmi_unified_t wmi_handle,
				      void *evt_buf,
				      struct disa_encrypt_decrypt_resp_params
				      *resp);
#endif

QDF_STATUS (*extract_vdev_start_resp)(wmi_unified_t wmi_handle, void *evt_buf,
	wmi_host_vdev_start_resp *vdev_rsp);

QDF_STATUS (*extract_vdev_delete_resp)(wmi_unified_t wmi_handle, void *evt_buf,
	struct wmi_host_vdev_delete_resp *delete_rsp);

QDF_STATUS (*extract_tbttoffset_update_params)(wmi_unified_t wmi_hdl,
					void *evt_buf, uint8_t idx,
					struct tbttoffset_params *tbtt_param);

QDF_STATUS (*extract_ext_tbttoffset_update_params)(wmi_unified_t wmi_hdl,
					void *evt_buf, uint8_t idx,
					struct tbttoffset_params *tbtt_param);

QDF_STATUS (*extract_tbttoffset_num_vdevs)(wmi_unified_t wmi_hdl, void *evt_buf,
					   uint32_t *num_vdevs);

QDF_STATUS (*extract_ext_tbttoffset_num_vdevs)(wmi_unified_t wmi_hdl,
					       void *evt_buf,
					       uint32_t *num_vdevs);

QDF_STATUS (*extract_mgmt_rx_params)(wmi_unified_t wmi_handle, void *evt_buf,
	struct mgmt_rx_event_params *hdr, uint8_t **bufp);

QDF_STATUS (*extract_vdev_stopped_param)(wmi_unified_t wmi_handle,
		void *evt_buf, uint32_t *vdev_id);

QDF_STATUS (*extract_vdev_roam_param)(wmi_unified_t wmi_handle, void *evt_buf,
	wmi_host_roam_event *param);

QDF_STATUS (*extract_vdev_scan_ev_param)(wmi_unified_t wmi_handle,
		void *evt_buf, struct scan_event *param);

#ifdef FEATURE_WLAN_SCAN_PNO
QDF_STATUS (*extract_nlo_match_ev_param)(wmi_unified_t wmi_handle,
					 void *evt_buf,
					 struct scan_event *param);

QDF_STATUS (*extract_nlo_complete_ev_param)(wmi_unified_t wmi_handle,
					    void *evt_buf,
					    struct scan_event *param);
#endif

QDF_STATUS (*extract_mu_ev_param)(wmi_unified_t wmi_handle, void *evt_buf,
	wmi_host_mu_report_event *param);

QDF_STATUS (*extract_mu_db_entry)(wmi_unified_t wmi_hdl, void *evt_buf,
	uint8_t idx, wmi_host_mu_db_entry *param);

QDF_STATUS (*extract_mumimo_tx_count_ev_param)(wmi_unified_t wmi_handle,
	void *evt_buf, wmi_host_peer_txmu_cnt_event *param);

QDF_STATUS (*extract_peer_gid_userpos_list_ev_param)(wmi_unified_t wmi_handle,
	void *evt_buf, wmi_host_peer_gid_userpos_list_event *param);

QDF_STATUS
(*extract_esp_estimation_ev_param)(wmi_unified_t wmi_handle, void *evt_buf,
				   struct esp_estimation_event *param);

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

QDF_STATUS (*extract_swba_quiet_info)(wmi_unified_t wmi_handle, void *evt_buf,
				      uint32_t idx,
				      wmi_host_quiet_info *quiet_info);

#ifdef CONVERGED_P2P_ENABLE
#ifdef FEATURE_P2P_LISTEN_OFFLOAD
QDF_STATUS (*extract_p2p_lo_stop_ev_param)(wmi_unified_t wmi_handle,
	void *evt_buf, struct p2p_lo_event *param);
#endif

QDF_STATUS (*extract_p2p_noa_ev_param)(wmi_unified_t wmi_handle,
	void *evt_buf, struct p2p_noa_info *param);

QDF_STATUS (*set_mac_addr_rx_filter)(wmi_unified_t wmi_handle,
				     struct p2p_set_mac_filter *param);
QDF_STATUS
(*extract_mac_addr_rx_filter_evt_param)(wmi_unified_t wmi_handle,
					void *evt_buf,
					struct p2p_set_mac_filter_evt *param);
#endif

#ifdef WLAN_FEATURE_INTEROP_ISSUES_AP
QDF_STATUS
(*extract_interop_issues_ap_ev_param)(wmi_unified_t wmi_handle, void *evt_buf,
				  struct wlan_interop_issues_ap_event *param);
QDF_STATUS
(*send_set_rap_ps_cmd)(wmi_unified_t wmi_handle,
		       struct wlan_interop_issues_ap_info *interop_issues_ap);
#endif

QDF_STATUS (*extract_peer_sta_ps_statechange_ev)(wmi_unified_t wmi_handle,
	void *evt_buf, wmi_host_peer_sta_ps_statechange_event *ev);

QDF_STATUS (*extract_peer_sta_kickout_ev)(wmi_unified_t wmi_handle,
		void *evt_buf, wmi_host_peer_sta_kickout_event *ev);

QDF_STATUS (*extract_peer_ratecode_list_ev)(wmi_unified_t wmi_handle,
					    void *evt_buf, uint8_t *peer_mac,
					    uint32_t *pdev_id,
					    wmi_sa_rate_cap *rate_cap);

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

QDF_STATUS (*extract_per_chain_rssi_stats)(wmi_unified_t wmi_handle,
			void *evt_buf, uint32_t index,
			struct wmi_host_per_chain_rssi_stats *rssi_stats);

QDF_STATUS (*extract_peer_stats)(wmi_unified_t wmi_handle, void *evt_buf,
			 uint32_t index, wmi_host_peer_stats *peer_stats);

QDF_STATUS (*extract_bcnflt_stats)(wmi_unified_t wmi_handle, void *evt_buf,
			 uint32_t index, wmi_host_bcnflt_stats *bcnflt_stats);

QDF_STATUS (*extract_peer_extd_stats)(wmi_unified_t wmi_handle, void *evt_buf,
		 uint32_t index, wmi_host_peer_extd_stats *peer_extd_stats);

QDF_STATUS (*extract_peer_retry_stats)(wmi_unified_t wmi_handle, void *evt_buf,
	    uint32_t index,
	    struct wmi_host_peer_retry_stats *peer_retry_stats);

QDF_STATUS (*extract_peer_adv_stats)(wmi_unified_t wmi_handle, void *evt_buf,
				     struct wmi_host_peer_adv_stats
				     *peer_adv_stats);

QDF_STATUS (*extract_chan_stats)(wmi_unified_t wmi_handle, void *evt_buf,
			 uint32_t index, wmi_host_chan_stats *chan_stats);

#ifdef WLAN_FEATURE_MIB_STATS
QDF_STATUS (*extract_mib_stats)(wmi_unified_t wmi_handle, void *evt_buf,
				struct mib_stats_metrics *mib_stats);
#endif

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

QDF_STATUS (*extract_vdev_extd_stats)(wmi_unified_t wmi_handle, void *evt_buf,
		uint32_t index, wmi_host_vdev_extd_stats *vdev_extd_stats);

QDF_STATUS (*extract_vdev_nac_rssi_stats)(wmi_unified_t wmi_handle, void *evt_buf,
		struct wmi_host_vdev_nac_rssi_event *vdev_nac_rssi_stats);

QDF_STATUS (*extract_bcn_stats)(wmi_unified_t wmi_handle, void *evt_buf,
		uint32_t index, wmi_host_bcn_stats *bcn_stats);

#ifdef OL_ATH_SMART_LOGGING
QDF_STATUS (*extract_smartlog_event)(wmi_unified_t wmi_handle, void *evt_buf,
				     struct wmi_debug_fatal_events *event);
#endif /* OL_ATH_SMART_LOGGING */
QDF_STATUS (*send_power_dbg_cmd)(wmi_unified_t wmi_handle,
				struct wmi_power_dbg_params *param);

QDF_STATUS (*send_multiple_vdev_restart_req_cmd)(wmi_unified_t wmi_handle,
				struct multiple_vdev_restart_params *param);
#ifdef QCA_SUPPORT_AGILE_DFS
QDF_STATUS
(*send_adfs_ocac_abort_cmd)(wmi_unified_t wmi_handle,
			    struct vdev_adfs_abort_params *param);

QDF_STATUS (*send_adfs_ch_cfg_cmd)(wmi_unified_t wmi_handle,
				   struct vdev_adfs_ch_cfg_params *param);
#endif
QDF_STATUS (*send_fw_test_cmd)(wmi_unified_t wmi_handle,
			       struct set_fwtest_params *wmi_fwtest);

#ifdef WLAN_FEATURE_ACTION_OUI
QDF_STATUS (*send_action_oui_cmd)(wmi_unified_t wmi_handle,
				  struct action_oui_request *req);
#endif /* WLAN_FEATURE_ACTION_OUI */

QDF_STATUS (*send_peer_rx_reorder_queue_setup_cmd)(wmi_unified_t wmi_handle,
		struct rx_reorder_queue_setup_params *param);

QDF_STATUS (*send_peer_rx_reorder_queue_remove_cmd)(wmi_unified_t wmi_handle,
		struct rx_reorder_queue_remove_params *param);

QDF_STATUS (*extract_service_ready_ext)(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf,
			struct wlan_psoc_host_service_ext_param *param);

QDF_STATUS (*extract_service_ready_ext2)(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf,
			struct wlan_psoc_host_service_ext2_param *param);

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

QDF_STATUS (*extract_dbr_ring_cap_service_ready_ext2)(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf, uint8_t idx,
			struct wlan_psoc_host_dbr_ring_caps *param);

QDF_STATUS (*extract_scaling_params_service_ready_ext)(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf, uint8_t idx,
			struct wlan_psoc_host_spectral_scaling_params *param);

QDF_STATUS (*extract_sar_cap_service_ready_ext)(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wlan_psoc_host_service_ext_param *ext_param);

#ifdef WMI_DBR_SUPPORT
QDF_STATUS (*send_dbr_cfg_cmd)(wmi_unified_t wmi_handle,
				   struct direct_buf_rx_cfg_req *cfg);

QDF_STATUS (*extract_dbr_buf_release_fixed)(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf,
			struct direct_buf_rx_rsp *param);

QDF_STATUS (*extract_dbr_buf_release_entry)(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf, uint8_t idx,
			struct direct_buf_rx_entry *param);

QDF_STATUS (*extract_dbr_buf_metadata)(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf, uint8_t idx,
			struct direct_buf_rx_metadata *param);
#endif

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

QDF_STATUS (*extract_vdev_peer_delete_all_resp)(
		wmi_unified_t wmi_handle,
		void *evt_buf,
		struct wmi_host_vdev_peer_delete_all_response_event
						*peer_delete_all_rsp);

QDF_STATUS (*extract_vdev_peer_delete_all_response_event)(
		wmi_unified_t wmi_handle,
		void *evt_buf,
		struct wmi_host_vdev_peer_delete_all_response_event *param);

bool (*is_management_record)(uint32_t cmd_id);
bool (*is_diag_event)(uint32_t event_id);
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

#ifdef WLAN_SUPPORT_RF_CHARACTERIZATION
QDF_STATUS (*extract_num_rf_characterization_entries)(wmi_unified_t wmi_hdl,
				uint8_t *evt_buf,
				uint32_t *num_rf_characterization_entries);


QDF_STATUS (*extract_rf_characterization_entries)(wmi_unified_t wmi_handle,
	uint8_t *evt_buf,
	uint32_t num_rf_characterization_entries,
	struct wmi_host_rf_characterization_event_param *rf_characterization_entries);
#endif

QDF_STATUS (*extract_chainmask_tables)(wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wlan_psoc_host_chainmask_table *chainmask_table);

QDF_STATUS (*send_get_rcpi_cmd)(wmi_unified_t wmi_handle,
				struct rcpi_req *get_rcpi_param);

QDF_STATUS (*extract_rcpi_response_event)(wmi_unified_t wmi_handle,
					  void *evt_buf,
					  struct rcpi_res *res);

QDF_STATUS (*extract_dfs_cac_complete_event)(wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		uint32_t *vdev_id,
		uint32_t len);
QDF_STATUS
(*extract_dfs_ocac_complete_event)(wmi_unified_t wmi_handle,
				   uint8_t *evt_buf,
				   struct vdev_adfs_complete_status *oca_stats);

QDF_STATUS (*extract_dfs_radar_detection_event)(wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct radar_found_info *radar_found,
		uint32_t len);
QDF_STATUS (*extract_wlan_radar_event_info)(wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct radar_event_info *wlan_radar_event,
		uint32_t len);

QDF_STATUS (*send_set_country_cmd)(wmi_unified_t wmi_handle,
				struct set_country *param);

uint32_t (*convert_pdev_id_host_to_target)(wmi_unified_t wmi_handle,
					   uint32_t pdev_id);
uint32_t (*convert_pdev_id_target_to_host)(wmi_unified_t wmi_handle,
					   uint32_t pdev_id);

/*
 * For MCL, convert_pdev_id_host_to_target returns legacy pdev id value.
 * But in converged firmware, WMI_SET_CURRENT_COUNTRY_CMDID expects target
 * mapping of pdev_id to give only one WMI_REG_CHAN_LIST_CC_EVENTID.
 * wmi_pdev_id_conversion_enable cannot be used since it overwrites
 * convert_pdev_id_host_to_target which effects legacy cases.
 * Below two commands: convert_host_pdev_id_to_target and
 * convert_target_pdev_id_to_host should be used for any WMI
 * command/event where FW expects target/host mapping of pdev_id respectively.
 */
uint32_t (*convert_host_pdev_id_to_target)(wmi_unified_t wmi_handle,
					   uint32_t pdev_id);
uint32_t (*convert_target_pdev_id_to_host)(wmi_unified_t wmi_handle,
					   uint32_t pdev_id);

QDF_STATUS (*send_user_country_code_cmd)(wmi_unified_t wmi_handle,
		uint8_t pdev_id, struct cc_regdmn_s *rd);

QDF_STATUS (*send_wds_entry_list_cmd)(wmi_unified_t wmi_handle);
QDF_STATUS (*extract_wds_entry)(wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wdsentry *wds_entry,
		u_int32_t idx);

#ifdef WLAN_FEATURE_NAN
QDF_STATUS (*send_ndp_initiator_req_cmd)(wmi_unified_t wmi_handle,
				struct nan_datapath_initiator_req *req);
QDF_STATUS (*send_ndp_responder_req_cmd)(wmi_unified_t wmi_handle,
				struct nan_datapath_responder_req *req);
QDF_STATUS (*send_ndp_end_req_cmd)(wmi_unified_t wmi_handle,
				struct nan_datapath_end_req *req);
QDF_STATUS (*send_terminate_all_ndps_req_cmd)(wmi_unified_t wmi_handle,
					      uint32_t vdev_id);

QDF_STATUS (*extract_ndp_initiator_rsp)(wmi_unified_t wmi_handle,
		uint8_t *data, struct nan_datapath_initiator_rsp *rsp);
QDF_STATUS (*extract_ndp_ind)(wmi_unified_t wmi_handle,
		uint8_t *data, struct nan_datapath_indication_event *ind);
QDF_STATUS (*extract_ndp_confirm)(wmi_unified_t wmi_handle,
		uint8_t *data, struct nan_datapath_confirm_event *ev);
QDF_STATUS (*extract_ndp_responder_rsp)(wmi_unified_t wmi_handle,
		uint8_t *data, struct nan_datapath_responder_rsp *rsp);
QDF_STATUS (*extract_ndp_end_rsp)(wmi_unified_t wmi_handle,
		uint8_t *data, struct nan_datapath_end_rsp_event *rsp);
QDF_STATUS (*extract_ndp_end_ind)(wmi_unified_t wmi_handle,
		uint8_t *data, struct nan_datapath_end_indication_event **ind);
QDF_STATUS (*extract_ndp_sch_update)(wmi_unified_t wmi_handle,
		uint8_t *data, struct nan_datapath_sch_update_event *ind);
QDF_STATUS (*extract_ndp_host_event)(wmi_unified_t wmi_handle, uint8_t *data,
		struct nan_datapath_host_event *evt);
#endif /* WLAN_FEATURE_NAN */

QDF_STATUS (*send_obss_detection_cfg_cmd)(wmi_unified_t wmi_handle,
		struct wmi_obss_detection_cfg_param *obss_cfg_param);
QDF_STATUS (*extract_obss_detection_info)(uint8_t *evt_buf,
					  struct wmi_obss_detect_info *info);
#if defined(WLAN_SUPPORT_FILS) || defined(CONFIG_BAND_6GHZ)
QDF_STATUS (*send_vdev_fils_enable_cmd)(wmi_unified_t wmi_handle,
					struct config_fils_params *param);
#endif
#ifdef WLAN_SUPPORT_FILS
QDF_STATUS (*extract_swfda_vdev_id)(wmi_unified_t wmi_handle, void *evt_buf,
				    uint32_t *vdev_id);
QDF_STATUS (*send_fils_discovery_send_cmd)(wmi_unified_t wmi_handle,
					   struct fd_params *param);
#endif /* WLAN_SUPPORT_FILS */

QDF_STATUS
(*send_roam_scan_stats_cmd)(wmi_unified_t wmi_handle,
			    struct wmi_roam_scan_stats_req *params);

QDF_STATUS
(*extract_roam_scan_stats_res_evt)(wmi_unified_t wmi_handle,
				   void *evt_buf,
				   uint32_t *vdev_id,
				   struct wmi_roam_scan_stats_res **res_param);
QDF_STATUS
(*extract_offload_bcn_tx_status_evt)(wmi_unified_t wmi_handle,
				     void *evt_buf, uint32_t *vdev_id,
				     uint32_t *tx_status);

QDF_STATUS
(*extract_roam_trigger_stats)(wmi_unified_t wmi_handle,
			      void *evt_buf,
			      struct wmi_roam_trigger_info *trig,
			      uint8_t idx);

QDF_STATUS
(*extract_roam_scan_stats)(wmi_unified_t wmi_handle,
			   void *evt_buf,
			   struct wmi_roam_scan_data *dst, uint8_t idx,
			   uint8_t chan_idx, uint8_t ap_idx);

QDF_STATUS
(*extract_roam_result_stats)(wmi_unified_t wmi_handle,
			     void *evt_buf,
			     struct wmi_roam_result *dst,
			     uint8_t idx);

QDF_STATUS
(*extract_roam_11kv_stats)(wmi_unified_t wmi_handle,
			   void *evt_buf,
			   struct wmi_neighbor_report_data *dst,
			   uint8_t idx, uint8_t rpt_idx);

void (*wmi_pdev_id_conversion_enable)(wmi_unified_t wmi_handle,
				      uint32_t *pdev_map, uint8_t size);
void (*send_time_stamp_sync_cmd)(wmi_unified_t wmi_handle);
void (*wmi_free_allocated_event)(uint32_t cmd_event_id,
				void **wmi_cmd_struct_ptr);
int (*wmi_check_and_pad_event)(void *os_handle, void *param_struc_ptr,
				uint32_t param_buf_len,
				uint32_t wmi_cmd_event_id,
				void **wmi_cmd_struct_ptr);
int (*wmi_check_command_params)(void *os_handle, void *param_struc_ptr,
				uint32_t param_buf_len,
				uint32_t wmi_cmd_event_id);

#ifdef WLAN_SUPPORT_TWT
QDF_STATUS (*send_twt_enable_cmd)(wmi_unified_t wmi_handle,
			struct wmi_twt_enable_param *params);

QDF_STATUS (*send_twt_disable_cmd)(wmi_unified_t wmi_handle,
			struct wmi_twt_disable_param *params);

QDF_STATUS (*send_twt_add_dialog_cmd)(wmi_unified_t wmi_handle,
			struct wmi_twt_add_dialog_param *params);

QDF_STATUS (*send_twt_del_dialog_cmd)(wmi_unified_t wmi_handle,
			struct wmi_twt_del_dialog_param *params);

QDF_STATUS (*send_twt_pause_dialog_cmd)(wmi_unified_t wmi_handle,
			struct wmi_twt_pause_dialog_cmd_param *params);

QDF_STATUS (*send_twt_resume_dialog_cmd)(wmi_unified_t wmi_handle,
			struct wmi_twt_resume_dialog_cmd_param *params);

QDF_STATUS (*extract_twt_enable_comp_event)(wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wmi_twt_enable_complete_event_param *params);

QDF_STATUS (*extract_twt_disable_comp_event)(wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wmi_twt_disable_complete_event *params);

QDF_STATUS (*extract_twt_add_dialog_comp_event)(wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wmi_twt_add_dialog_complete_event_param *params);

QDF_STATUS (*extract_twt_del_dialog_comp_event)(wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wmi_twt_del_dialog_complete_event_param *params);

QDF_STATUS (*extract_twt_pause_dialog_comp_event)(wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wmi_twt_pause_dialog_complete_event_param *params);

QDF_STATUS (*extract_twt_resume_dialog_comp_event)(wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wmi_twt_resume_dialog_complete_event_param *params);
#endif

#ifdef QCA_SUPPORT_CP_STATS
QDF_STATUS (*extract_cca_stats)(wmi_unified_t wmi_handle, void *evt_buf,
				struct wmi_host_congestion_stats *stats);
#endif /* QCA_SUPPORT_CP_STATS */

#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(HOST_DFS_SPOOF_TEST)
QDF_STATUS (*send_dfs_average_radar_params_cmd)(
		wmi_unified_t wmi_handle,
		struct dfs_radar_found_params *params);

QDF_STATUS (*extract_dfs_status_from_fw)(wmi_unified_t wmi_handle,
					 void *evt_buf,
					 uint32_t *dfs_status_check);
#endif

QDF_STATUS
(*extract_cfr_peer_tx_event_param)(wmi_unified_t wmi_handle, void *evt_buf,
				   wmi_cfr_peer_tx_event_param * peer_tx_event);

#ifdef OBSS_PD
QDF_STATUS (*send_obss_spatial_reuse_set)(wmi_unified_t wmi_handle,
		struct wmi_host_obss_spatial_reuse_set_param
		*obss_spatial_reuse_param);

QDF_STATUS (*send_obss_spatial_reuse_set_def_thresh)(wmi_unified_t wmi_handle,
		struct wmi_host_obss_spatial_reuse_set_def_thresh
		*obss_spatial_reuse_param);
#endif

QDF_STATUS
(*extract_ctl_failsafe_check_ev_param)(
		wmi_unified_t wmi_handle,
		void *evt_buf,
		struct wmi_host_pdev_ctl_failsafe_event *param);

QDF_STATUS (*send_peer_del_all_wds_entries_cmd)(wmi_unified_t wmi_handle,
		struct peer_del_all_wds_entries_params *param);

#ifdef WLAN_CFR_ENABLE
QDF_STATUS (*send_peer_cfr_capture_cmd)(wmi_unified_t wmi_handle,
					struct peer_cfr_params *param);
#endif
#ifdef WMI_AP_SUPPORT
QDF_STATUS (*send_vdev_pcp_tid_map_cmd)(wmi_unified_t wmi_handle,
					struct vap_pcp_tid_map_params *param);
QDF_STATUS (*send_vdev_tidmap_prec_cmd)(wmi_unified_t wmi_handle,
					struct vap_tidmap_prec_params *param);
QDF_STATUS (*send_peer_ft_roam_cmd)(wmi_unified_t wmi_handle,
				    uint8_t peer_addr[QDF_MAC_ADDR_SIZE],
				    uint8_t vdev_id);
#endif
QDF_STATUS (*send_mws_coex_status_req_cmd)(wmi_unified_t wmi_handle,
					   uint32_t vdev_id, uint32_t cmd_id);

#ifdef WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG
QDF_STATUS (*set_rx_pkt_type_routing_tag_cmd)(
	wmi_unified_t wmi_hdl, struct wmi_rx_pkt_protocol_routing_info *param);
#endif /* WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG */
#ifdef WIFI_POS_CONVERGED
QDF_STATUS (*extract_oem_response_param)
		(wmi_unified_t wmi_hdl, void *resp_buf,
		 struct wmi_oem_response_param *oem_resp_param);
#endif /* WIFI_POS_CONVERGED */

QDF_STATUS (*extract_hw_mode_resp_event)(wmi_unified_t wmi_handle,
					 void *evt_buf, uint32_t *cmd_status);

QDF_STATUS (*send_set_roam_trigger_cmd)(wmi_unified_t wmi_handle,
					uint32_t vdev_id,
					uint32_t trigger_bitmap);

#ifdef WLAN_FEATURE_ELNA
QDF_STATUS (*send_set_elna_bypass_cmd)(wmi_unified_t wmi_handle,
				       struct set_elna_bypass_request *req);
QDF_STATUS (*send_get_elna_bypass_cmd)(wmi_unified_t wmi_handle,
				       struct get_elna_bypass_request *req);
QDF_STATUS (*extract_get_elna_bypass_resp)(wmi_unified_t wmi_handle,
					 void *resp_buf,
					 struct get_elna_bypass_response *resp);
#endif /* WLAN_FEATURE_ELNA */

QDF_STATUS (*send_pdev_get_pn_cmd)(wmi_unified_t wmi_handle,
				   struct peer_request_pn_param *pn_params);
QDF_STATUS (*extract_get_pn_data)(wmi_unified_t wmi_handle,
				  void *evt_buf,
				  struct wmi_host_get_pn_event *param);
#ifdef FEATURE_ANI_LEVEL_REQUEST
QDF_STATUS (*send_ani_level_cmd)(wmi_unified_t wmi_handle, uint32_t *freqs,
				 uint8_t num_freqs);

QDF_STATUS (*extract_ani_level)(uint8_t *evt_buf,
				struct wmi_host_ani_level_event **info,
				uint32_t *num_freqs);
#endif /* FEATURE_ANI_LEVEL_REQUEST */
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

#define NUM_DEBUG_INFOS 9
struct wmi_unified {
	void *scn_handle;    /* handle to device */
	osdev_t  osdev; /* handle to use OS-independent services */
	struct wbuff_mod_handle *wbuff_handle; /* handle to wbuff */
	qdf_atomic_t pending_cmds;
	HTC_ENDPOINT_ID wmi_endpoint_id;
	uint16_t max_msg_len;
	uint32_t *event_id;
	wmi_unified_event_handler *event_handler;
	enum wmi_rx_exec_ctx *ctx;
	HTC_HANDLE htc_handle;
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
	struct wmi_ops *ops;
	bool use_cookie;
	bool wmi_stopinprogress;
	uint32_t *wmi_events;
	uint32_t *services;
	struct wmi_soc *soc;
	uint16_t wmi_max_cmds;
	struct dentry *debugfs_de[NUM_DEBUG_INFOS];
	qdf_atomic_t critical_events_in_flight;
#ifdef WMI_EXT_DBG
	int wmi_ext_dbg_msg_queue_size;
	qdf_list_t wmi_ext_dbg_msg_queue;
	qdf_spinlock_t wmi_ext_dbg_msg_queue_lock;
	qdf_dentry_t wmi_ext_dbg_dentry;
#endif /*WMI_EXT_DBG*/
	uint32_t *cmd_pdev_id_map;
	uint32_t *evt_pdev_id_map;
	qdf_atomic_t num_stats_over_qmi;
};

#define WMI_MAX_RADIOS 3
struct wmi_soc {
	struct wlan_objmgr_psoc *wmi_psoc;
	void *scn_handle;    /* handle to device */
	qdf_atomic_t num_pdevs;
	enum wmi_target_type target_type;
	bool is_async_ep;
	HTC_HANDLE htc_handle;
	uint32_t event_id[WMI_UNIFIED_MAX_EVENT];
	wmi_unified_event_handler event_handler[WMI_UNIFIED_MAX_EVENT];
	uint32_t max_event_idx;
	enum wmi_rx_exec_ctx ctx[WMI_UNIFIED_MAX_EVENT];
	qdf_spinlock_t ctx_lock;
	struct wmi_unified *wmi_pdev[WMI_MAX_RADIOS];
	HTC_ENDPOINT_ID wmi_endpoint_id[WMI_MAX_RADIOS];
	uint16_t max_msg_len[WMI_MAX_RADIOS];
	struct wmi_ops *ops;
	const uint32_t *svc_ids;
	uint32_t wmi_events[wmi_events_max];
	/* WMI service bitmap received from target */
	uint32_t *wmi_service_bitmap;
	uint32_t *wmi_ext_service_bitmap;
	uint32_t services[wmi_services_max];
	uint16_t wmi_max_cmds;
	uint32_t soc_idx;
	uint32_t cmd_pdev_id_map[WMI_MAX_RADIOS];
	uint32_t evt_pdev_id_map[WMI_MAX_RADIOS];
	bool is_pdev_is_map_enable;
#ifdef WMI_INTERFACE_EVENT_LOGGING
	uint32_t buf_offset_command;
	uint32_t buf_offset_event;
#endif /*WMI_INTERFACE_EVENT_LOGGING */
};

/**
 * struct wmi_process_fw_event_params - fw event parameters
 * @wmi_handle: wmi handle
 * @evt_buf: event buffer
 */
struct wmi_process_fw_event_params {
	void *wmi_handle;
	void *evt_buf;
};

/**
 * wmi_mtrace() - Wrappper function for qdf_mtrace api
 * @message_id: 32-Bit Wmi message ID
 * @vdev_id: Vdev ID
 * @data: Actual message contents
 *
 * This function converts the 32-bit WMI message ID in 15-bit message ID
 * format for qdf_mtrace as in qdf_mtrace message there are only 15
 * bits reserved for message ID.
 * out of these 15-bits, 8-bits (From MSB) specifies the WMI_GRP_ID
 * and remaining 7-bits specifies the actual WMI command. With this
 * notation there can be maximum 256 groups and each group can have
 * max 128 commands can be supported.
 *
 * Return: None
 */
void wmi_mtrace(uint32_t message_id, uint16_t vdev_id, uint32_t data);

void wmi_unified_register_module(enum wmi_target_type target_type,
			void (*wmi_attach)(wmi_unified_t wmi_handle));
void wmi_tlv_init(void);
void wmi_non_tlv_init(void);
#ifdef WMI_NON_TLV_SUPPORT
/* ONLY_NON_TLV_TARGET:TLV attach dummy function definition for case when
 * driver supports only NON-TLV target (WIN mainline) */
#define wmi_tlv_attach(x) qdf_print("TLV Unavailable")
#else
void wmi_tlv_attach(wmi_unified_t wmi_handle);
#endif
void wmi_non_tlv_attach(wmi_unified_t wmi_handle);

#ifdef FEATURE_WLAN_EXTSCAN
void wmi_extscan_attach_tlv(struct wmi_unified *wmi_handle);
#else
static inline void wmi_extscan_attach_tlv(struct wmi_unified *wmi_handle)
{
}
#endif

#ifdef WMI_SMART_ANT_SUPPORT
void wmi_smart_ant_attach_tlv(struct wmi_unified *wmi_handle);
#else
static inline void wmi_smart_ant_attach_tlv(struct wmi_unified *wmi_handle)
{
}
#endif

#ifdef WMI_DBR_SUPPORT
void wmi_dbr_attach_tlv(struct wmi_unified *wmi_handle);
#else
static inline void wmi_dbr_attach_tlv(struct wmi_unified *wmi_handle)
{
}
#endif

#ifdef WMI_ATF_SUPPORT
void wmi_atf_attach_tlv(struct wmi_unified *wmi_handle);
#else
static inline void wmi_atf_attach_tlv(struct wmi_unified *wmi_handle)
{
}
#endif

#ifdef WMI_AP_SUPPORT
void wmi_ap_attach_tlv(struct wmi_unified *wmi_handle);
#else
static inline void wmi_ap_attach_tlv(struct wmi_unified *wmi_handle)
{
}
#endif

#ifdef WLAN_FEATURE_DSRC
void wmi_ocb_attach_tlv(wmi_unified_t wmi_handle);
#else
static inline void wmi_ocb_attach_tlv(wmi_unified_t wmi_handle)
{
}
#endif

#ifdef WLAN_FEATURE_NAN
void wmi_nan_attach_tlv(wmi_unified_t wmi_handle);
#else
static inline void wmi_nan_attach_tlv(wmi_unified_t wmi_handle)
{
}
#endif

#ifdef FEATURE_P2P_LISTEN_OFFLOAD
void wmi_p2p_listen_offload_attach_tlv(wmi_unified_t wmi_handle);
#else
static inline
void wmi_p2p_listen_offload_attach_tlv(wmi_unified_t wmi_handle)
{
}
#endif

#ifdef CONVERGED_P2P_ENABLE
void wmi_p2p_attach_tlv(wmi_unified_t wmi_handle);
#else
static inline void wmi_p2p_attach_tlv(struct wmi_unified *wmi_handle)
{
}
#endif

#ifdef WLAN_FEATURE_INTEROP_ISSUES_AP
void wmi_interop_issues_ap_attach_tlv(wmi_unified_t wmi_handle);
#else
static inline void
wmi_interop_issues_ap_attach_tlv(struct wmi_unified *wmi_handle)
{
}
#endif

#ifdef FEATURE_LFR_SUBNET_DETECTION
void wmi_lfr_subnet_detection_attach_tlv(wmi_unified_t wmi_handle);
#else
static inline
void wmi_lfr_subnet_detection_attach_tlv(struct wmi_unified *wmi_handle)
{
}
#endif

#ifdef FEATURE_RSSI_MONITOR
void wmi_rssi_monitor_attach_tlv(wmi_unified_t wmi_handle);
#else
static inline
void wmi_rssi_monitor_attach_tlv(struct wmi_unified *wmi_handle)
{
}
#endif

#ifdef FEATURE_WLAN_ESE
void wmi_ese_attach_tlv(wmi_unified_t wmi_handle);
#else
static inline void wmi_ese_attach_tlv(struct wmi_unified *wmi_handle)
{
}
#endif

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
void wmi_roam_offload_attach_tlv(wmi_unified_t wmi_handle);
#else
static inline
void wmi_roam_offload_attach_tlv(struct wmi_unified *wmi_handle)
{
}
#endif

#ifdef WLAN_FEATURE_FILS_SK
void wmi_fils_sk_attach_tlv(wmi_unified_t wmi_handle);
#else
static inline void wmi_fils_sk_attach_tlv(struct wmi_unified *wmi_handle)
{
}
#endif

#ifdef WMI_ROAM_SUPPORT
void wmi_roam_attach_tlv(struct wmi_unified *wmi_handle);
#else
static inline void wmi_roam_attach_tlv(struct wmi_unified *wmi_handle)
{
}
#endif

#ifdef WMI_CONCURRENCY_SUPPORT
void wmi_concurrency_attach_tlv(struct wmi_unified *wmi_handle);
#else
static inline void wmi_concurrency_attach_tlv(struct wmi_unified *wmi_handle)
{
}
#endif

#ifdef FEATURE_WLAN_D0WOW
void wmi_d0wow_attach_tlv(struct wmi_unified *wmi_handle);
#else
static inline void wmi_d0wow_attach_tlv(struct wmi_unified *wmi_handle)
{
}
#endif

#ifdef FEATURE_WLAN_RA_FILTERING
void wmi_ra_filtering_attach_tlv(struct wmi_unified *wmi_handle);
#else
static inline
void wmi_ra_filtering_attach_tlv(struct wmi_unified *wmi_handle)
{
}
#endif

#ifdef FEATURE_WLAN_LPHB
void wmi_lphb_attach_tlv(struct wmi_unified *wmi_handle);
#else
static inline void wmi_lphb_attach_tlv(struct wmi_unified *wmi_handle)
{
}
#endif

#ifdef WLAN_FEATURE_PACKET_FILTERING
void wmi_packet_filtering_attach_tlv(struct wmi_unified *wmi_handle);
#else
static inline
void wmi_packet_filtering_attach_tlv(struct wmi_unified *wmi_handle)
{
}
#endif

#ifdef WLAN_FEATURE_EXTWOW_SUPPORT
void wmi_extwow_attach_tlv(struct wmi_unified *wmi_handle);
#else
static inline void wmi_extwow_attach_tlv(struct wmi_unified *wmi_handle)
{
}
#endif

#ifdef WLAN_POWER_MANAGEMENT_OFFLOAD
void wmi_pmo_attach_tlv(struct wmi_unified *wmi_handle);
#else
static inline void wmi_pmo_attach_tlv(struct wmi_unified *wmi_handle)
{
}
#endif

#ifdef FEATURE_WLAN_TDLS
void wmi_tdls_attach_tlv(struct wmi_unified *wmi_handle);
#else
static inline void wmi_tdls_attach_tlv(struct wmi_unified *wmi_handle)
{
}
#endif

#ifdef WLAN_POLICY_MGR_ENABLE
void wmi_policy_mgr_attach_tlv(struct wmi_unified *wmi_handle);
#else
static inline
void wmi_policy_mgr_attach_tlv(struct wmi_unified *wmi_handle)
{
}
#endif

#ifdef FEATURE_BLACKLIST_MGR
void wmi_blacklist_mgr_attach_tlv(struct wmi_unified *wmi_handle);
#else
static inline
void wmi_blacklist_mgr_attach_tlv(struct wmi_unified *wmi_handle)
{
}
#endif

#ifdef WMI_STA_SUPPORT
void wmi_sta_attach_tlv(struct wmi_unified *wmi_handle);
#else
static inline void wmi_sta_attach_tlv(struct wmi_unified *wmi_handle)
{
}
#endif

#ifdef WLAN_WMI_BCN
void wmi_bcn_attach_tlv(wmi_unified_t wmi_handle);
#else
static inline void wmi_bcn_attach_tlv(wmi_unified_t wmi_handle)
{
}
#endif

/**
 * wmi_fwol_attach_tlv() - attach fw offload tlv handlers
 * @wmi_handle: wmi handle
 *
 * Return: void
 */
#ifdef WLAN_FW_OFFLOAD
void wmi_fwol_attach_tlv(wmi_unified_t wmi_handle);
#else
static inline void wmi_fwol_attach_tlv(wmi_unified_t wmi_handle)
{
}
#endif

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

#ifdef WMI_EXT_DBG

/**
 * wmi_ext_dbg_msg_get() - Allocate memory for wmi debug msg
 *
 * @buflen: Length of WMI message buffer
 *
 * Return: Allocated msg buffer else NULL on failure.
 */
static inline struct wmi_ext_dbg_msg *wmi_ext_dbg_msg_get(uint32_t buflen)
{
	return qdf_mem_malloc(sizeof(struct wmi_ext_dbg_msg) + buflen);
}

/**
 * wmi_ext_dbg_msg_put() - Free wmi debug msg buffer
 *
 * @msg: wmi message buffer to be freed
 *
 * Return: none
 */
static inline void wmi_ext_dbg_msg_put(struct wmi_ext_dbg_msg *msg)
{
	qdf_mem_free(msg);
}

#else

static inline QDF_STATUS wmi_ext_dbg_msg_cmd_record(struct wmi_unified
						    *wmi_handle,
						    uint8_t *buf, uint32_t len)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS wmi_ext_dbg_msg_event_record(struct wmi_unified
						      *wmi_handle,
						      uint8_t *buf,
						      uint32_t len)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS wmi_ext_dbgfs_init(struct wmi_unified *wmi_handle)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS wmi_ext_dbgfs_deinit(struct wmi_unified *wmi_handle)
{
	return QDF_STATUS_SUCCESS;
}

#endif /*WMI_EXT_DBG */
#endif
