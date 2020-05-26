/*
 * Copyright (c) 2012-2020 The Linux Foundation. All rights reserved.
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

#if !defined(HDD_CONFIG_H__)
#define HDD_CONFIG_H__

/**
 *
 * DOC: wlan_hdd_config.h
 *
 * WLAN Adapter Configuration functions
 */

/* $HEADER$ */

/* Include files */
#include <wlan_hdd_includes.h>
#include <wlan_hdd_wmm.h>
#include <qdf_types.h>
#include <csr_api.h>
#include <sap_api.h>
#include "osapi_linux.h"
#include <wmi_unified.h>
#include "wlan_pmo_hw_filter_public_struct.h"
#include "wlan_action_oui_public_struct.h"
#include "hdd_config.h"

struct hdd_context;

#define CFG_DP_RPS_RX_QUEUE_CPU_MAP_LIST_LEN 30

#define FW_MODULE_LOG_LEVEL_STRING_LENGTH  (512)
#define TX_SCHED_WRR_PARAMS_NUM            (5)

/* Defines for all of the things we read from the configuration (registry). */

#ifdef CONFIG_DP_TRACE
/* Max length of gDptraceConfig string. e.g.- "1, 6, 1, 62" */
#define DP_TRACE_CONFIG_STRING_LENGTH		(20)

/* At max 4 DP Trace config parameters are allowed. Refer - gDptraceConfig */
#define DP_TRACE_CONFIG_NUM_PARAMS		(4)

/*
 * Default value of live mode in case it cannot be determined from cfg string
 * gDptraceConfig
 */
#define DP_TRACE_CONFIG_DEFAULT_LIVE_MODE	(1)

/*
 * Default value of thresh (packets/second) beyond which DP Trace is disabled.
 * Use this default in case the value cannot be determined from cfg string
 * gDptraceConfig
 */
#define DP_TRACE_CONFIG_DEFAULT_THRESH		(6)

/*
 * Number of intervals of BW timer to wait before enabling/disabling DP Trace.
 * Since throughput threshold to disable live logging for DP Trace is very low,
 * we calculate throughput based on # packets received in a second.
 * For example assuming bandwidth timer interval is 100ms, and if more than 6
 * prints are received in 10 * 100 ms interval, we want to disable DP Trace
 * live logging. DP_TRACE_CONFIG_DEFAULT_THRESH_TIME_LIMIT is the default
 * value, to be used in case the real value cannot be derived from
 * bw timer interval
 */
#define DP_TRACE_CONFIG_DEFAULT_THRESH_TIME_LIMIT (10)

/* Default proto bitmap in case its missing in gDptraceConfig string */
#define DP_TRACE_CONFIG_DEFAULT_BITMAP \
			(QDF_NBUF_PKT_TRAC_TYPE_EAPOL |\
			QDF_NBUF_PKT_TRAC_TYPE_DHCP |\
			QDF_NBUF_PKT_TRAC_TYPE_MGMT_ACTION |\
			QDF_NBUF_PKT_TRAC_TYPE_ARP |\
			QDF_NBUF_PKT_TRAC_TYPE_ICMP |\
			QDF_NBUF_PKT_TRAC_TYPE_ICMPv6)\

/* Default verbosity, in case its missing in gDptraceConfig string*/
#define DP_TRACE_CONFIG_DEFAULT_VERBOSTY QDF_DP_TRACE_VERBOSITY_LOW

#endif

/*
 * Type declarations
 */

struct hdd_config {
	/* Config parameters */
	enum hdd_dot11_mode dot11Mode;

#ifdef FEATURE_WLAN_DYNAMIC_CVM
	/* Bitmap for operating voltage corner mode */
	uint32_t vc_mode_cfg_bitmap;
#endif
#ifdef ENABLE_MTRACE_LOG
	bool enable_mtrace;
#endif
	bool advertise_concurrent_operation;
#ifdef DHCP_SERVER_OFFLOAD
	struct dhcp_server dhcp_server_ip;
#endif /* DHCP_SERVER_OFFLOAD */
	bool apf_enabled;
	uint16_t sap_tx_leakage_threshold;
	bool sap_internal_restart;
	bool tx_orphan_enable;
	bool is_11k_offload_supported;
	bool action_oui_enable;
	uint8_t action_oui_str[ACTION_OUI_MAXIMUM_ID][ACTION_OUI_MAX_STR_LEN];
	bool is_unit_test_framework_enabled;
	bool disable_channel;

	/* HDD converged ini items are listed below this*/
	bool bug_on_reinit_failure;
	bool is_ramdump_enabled;
	uint32_t iface_change_wait_time;
	uint8_t multicast_host_fw_msgs;
	enum hdd_wext_control private_wext_control;
	bool enablefwprint;
	uint8_t enable_fw_log;

#ifdef WLAN_LOGGING_SOCK_SVC_ENABLE
	/* WLAN Logging */
	bool wlan_logging_enable;
	bool wlan_logging_to_console;
	uint8_t host_log_custom_nl_proto;
#endif /* WLAN_LOGGING_SOCK_SVC_ENABLE */

#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
	uint32_t wlan_auto_shutdown;
#endif

#ifndef REMOVE_PKT_LOG
	bool enable_packet_log;
#endif
	uint32_t rx_mode;
	uint32_t tx_comp_loop_pkt_limit;
	uint32_t rx_reap_loop_pkt_limit;
	uint32_t rx_hp_oos_update_limit;
	uint64_t rx_softirq_max_yield_duration_ns;
#ifdef WLAN_FEATURE_DP_BUS_BANDWIDTH
	/* bandwidth threshold for very high bandwidth */
	uint32_t bus_bw_very_high_threshold;
	/* bandwidth threshold for high bandwidth */
	uint32_t bus_bw_high_threshold;
	/* bandwidth threshold for medium bandwidth */
	uint32_t bus_bw_medium_threshold;
	/* bandwidth threshold for low bandwidth */
	uint32_t bus_bw_low_threshold;
	uint32_t bus_bw_compute_interval;
	uint32_t enable_tcp_delack;
	bool     enable_tcp_limit_output;
	uint32_t enable_tcp_adv_win_scale;
	uint32_t tcp_delack_thres_high;
	uint32_t tcp_delack_thres_low;
	uint32_t tcp_tx_high_tput_thres;
	uint32_t tcp_delack_timer_count;
	bool     enable_tcp_param_update;
	uint32_t bus_low_cnt_threshold;
	bool enable_latency_crit_clients;
#endif /*WLAN_FEATURE_DP_BUS_BANDWIDTH*/

#ifdef QCA_SUPPORT_TXRX_DRIVER_TCP_DEL_ACK
	bool del_ack_enable;
	uint32_t del_ack_threshold_high;
	uint32_t del_ack_threshold_low;
	uint16_t del_ack_timer_value;
	uint16_t del_ack_pkt_count;
#endif

#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL
	uint32_t tx_flow_low_watermark;
	uint32_t tx_flow_hi_watermark_offset;
	uint32_t tx_flow_max_queue_depth;
	uint32_t tx_lbw_flow_low_watermark;
	uint32_t tx_lbw_flow_hi_watermark_offset;
	uint32_t tx_lbw_flow_max_queue_depth;
	uint32_t tx_hbw_flow_low_watermark;
	uint32_t tx_hbw_flow_hi_watermark_offset;
	uint32_t tx_hbw_flow_max_queue_depth;
#endif /* QCA_LL_LEGACY_TX_FLOW_CONTROL */
	uint32_t napi_cpu_affinity_mask;
	/* CPU affinity mask for rx_thread */
	uint32_t rx_thread_ul_affinity_mask;
	uint32_t rx_thread_affinity_mask;
	uint8_t cpu_map_list[CFG_DP_RPS_RX_QUEUE_CPU_MAP_LIST_LEN];
	bool multicast_replay_filter;
	uint32_t rx_wakelock_timeout;
	uint8_t num_dp_rx_threads;
#ifdef CONFIG_DP_TRACE
	bool enable_dp_trace;
	uint8_t dp_trace_config[DP_TRACE_CONFIG_STRING_LENGTH];
#endif
	uint8_t enable_nud_tracking;
	uint32_t operating_chan_freq;
	uint8_t num_vdevs;
	uint8_t enable_concurrent_sta[CFG_CONCURRENT_IFACE_MAX_LEN];
	uint8_t dbs_scan_selection[CFG_DBS_SCAN_PARAM_LENGTH];
#ifdef FEATURE_RUNTIME_PM
	uint8_t runtime_pm;
#endif
	uint8_t inform_bss_rssi_raw;

	bool mac_provision;
	uint32_t provisioned_intf_pool;
	uint32_t derived_intf_pool;
	uint32_t cfg_wmi_credit_cnt;
	uint32_t enable_sar_conversion;
	bool is_wow_disabled;
#ifdef WLAN_FEATURE_TSF_PLUS
	uint8_t tsf_ptp_options;
#endif /* WLAN_FEATURE_TSF_PLUS */

#ifdef WLAN_SUPPORT_TXRX_HL_BUNDLE
	uint32_t pkt_bundle_threshold_high;
	uint32_t pkt_bundle_threshold_low;
	uint16_t pkt_bundle_timer_value;
	uint16_t pkt_bundle_size;
#endif
	uint32_t dp_proto_event_bitmap;

#ifdef SAR_SAFETY_FEATURE
	uint32_t sar_safety_timeout;
	uint32_t sar_safety_unsolicited_timeout;
	uint32_t sar_safety_req_resp_timeout;
	uint32_t sar_safety_req_resp_retry;
	uint32_t sar_safety_index;
	uint32_t sar_safety_sleep_index;
	bool enable_sar_safety;
	bool config_sar_safety_sleep_index;
#endif
	bool get_roam_chan_from_fw;
	uint32_t fisa_enable;

#ifdef WLAN_FEATURE_PERIODIC_STA_STATS
	/* Periodicity of logging */
	uint32_t periodic_stats_timer_interval;
	/* Duration for which periodic logging should be done */
	uint32_t periodic_stats_timer_duration;
#endif /* WLAN_FEATURE_PERIODIC_STA_STATS */
};

/**
 * hdd_to_csr_wmm_mode() - Utility function to convert HDD to CSR WMM mode
 *
 * @uint8_t mode - hdd WMM user mode
 *
 * Return: CSR WMM mode
 */
eCsrRoamWmmUserModeType hdd_to_csr_wmm_mode(uint8_t mode);

QDF_STATUS hdd_update_mac_config(struct hdd_context *hdd_ctx);
QDF_STATUS hdd_set_sme_config(struct hdd_context *hdd_ctx);
QDF_STATUS hdd_set_policy_mgr_user_cfg(struct hdd_context *hdd_ctx);
QDF_STATUS hdd_set_sme_chan_list(struct hdd_context *hdd_ctx);
bool hdd_update_config_cfg(struct hdd_context *hdd_ctx);
void hdd_cfg_get_global_config(struct hdd_context *hdd_ctx, char *buf,
			       int buflen);

eCsrPhyMode hdd_cfg_xlate_to_csr_phy_mode(enum hdd_dot11_mode dot11Mode);

QDF_STATUS hdd_set_idle_ps_config(struct hdd_context *hdd_ctx, bool val);
void hdd_get_pmkid_modes(struct hdd_context *hdd_ctx,
			 struct pmkid_mode_bits *pmkid_modes);

int hdd_update_tgt_cfg(hdd_handle_t hdd_handle, struct wma_tgt_cfg *cfg);

/**
 * hdd_string_to_u8_array() - used to convert decimal string into u8 array
 * @str: Decimal string
 * @array: Array where converted value is stored
 * @len: Length of the populated array
 * @array_max_len: Maximum length of the array
 *
 * This API is called to convert decimal string (each byte separated by
 * a comma) into an u8 array
 *
 * Return: QDF_STATUS
 */
QDF_STATUS hdd_string_to_u8_array(char *str, uint8_t *array,
				  uint8_t *len, uint16_t array_max_len);

QDF_STATUS hdd_hex_string_to_u16_array(char *str, uint16_t *int_array,
				uint8_t *len, uint8_t int_array_max_len);

void hdd_cfg_print_global_config(struct hdd_context *hdd_ctx);

QDF_STATUS hdd_update_nss(struct hdd_adapter *adapter, uint8_t nss);

/**
 * hdd_dfs_indicate_radar() - Block tx as radar found on the channel
 * @hdd_ctxt: HDD context pointer
 *
 * This function is invoked in atomic context when a radar
 * is found on the SAP current operating channel and Data Tx
 * from netif has to be stopped to honor the DFS regulations.
 * Actions: Stop the netif Tx queues,Indicate Radar present
 * in HDD context for future usage.
 *
 * Return: true on success, else false
 */
bool hdd_dfs_indicate_radar(struct hdd_context *hdd_ctx);

/**
 * hdd_override_all_ps() - overrides to disables all the powersave features.
 * @hdd_ctx: Pointer to HDD context.
 * Overrides below powersave ini configurations.
 * gEnableImps=0
 * gEnableBmps=0
 * gRuntimePM=0
 * gWlanAutoShutdown = 0
 * gEnableSuspend=0
 * gEnableWoW=0
 *
 * Return: None
 */
void hdd_override_all_ps(struct hdd_context *hdd_ctx);
#endif
