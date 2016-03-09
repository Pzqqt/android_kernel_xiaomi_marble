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
 * This file contains the API definitions for the Unified Wireless Module
 * Interface (WMI).
 */

#ifndef _WMI_UNIFIED_API_H_
#define _WMI_UNIFIED_API_H_

#include <osdep.h>
#include "a_types.h"
#include "ol_defines.h"
#include "wmi.h"
#include "htc_api.h"
#include "wmi_unified_param.h"

typedef qdf_nbuf_t wmi_buf_t;
#define wmi_buf_data(_buf) qdf_nbuf_data(_buf)

#define WMI_LOGD(args ...) \
	QDF_TRACE(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_DEBUG, ## args)
#define WMI_LOGI(args ...) \
	QDF_TRACE(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_INFO, ## args)
#define WMI_LOGW(args ...) \
	QDF_TRACE(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_WARN, ## args)
#define WMI_LOGE(args ...) \
	QDF_TRACE(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_ERROR, ## args)
#define WMI_LOGP(args ...) \
	QDF_TRACE(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_FATAL, ## args)

#define WMI_DEBUG_ALWAYS

#ifdef WMI_DEBUG_ALWAYS
#define WMI_LOGA(args ...) \
	QDF_TRACE(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_FATAL, ## args)
#else
#define WMI_LOGA(args ...)
#endif

/**
 * struct wmi_ops - service callbacks to upper layer
 * @service_ready_cbk: service ready callback
 * @service_ready_ext_cbk: service ready ext callback
 * @ready_cbk: ready calback
 * @wma_process_fw_event_handler_cbk: generic event handler callback
 */
struct wmi_rx_ops {

	int (*wma_process_fw_event_handler_cbk)(void *ctx,
				  void *ev, uint8_t rx_ctx);
};

/**
 * enum wmi_target_type - type of supported wmi command
 * @WMI_TLV_TARGET: tlv based target
 * @WMI_NON_TLV_TARGET: non-tlv based target
 *
 */
enum wmi_target_type {
	WMI_TLV_TARGET,
	WMI_NON_TLV_TARGET
};

/**
 * enum wmi_rx_exec_ctx - wmi rx execution context
 * @WMI_RX_WORK_CTX: work queue context execution provided by WMI layer
 * @WMI_RX_UMAC_CTX: execution context provided by umac layer
 *
 */
enum wmi_rx_exec_ctx {
	WMI_RX_WORK_CTX,
	WMI_RX_UMAC_CTX
};

/**
 * attach for unified WMI
 *
 *  @param scn_handle      : handle to SCN.
 *  @return opaque handle.
 */
void *wmi_unified_attach(void *scn_handle,
			 osdev_t osdev, enum wmi_target_type target_type,
			 bool use_cookie, struct wmi_rx_ops *ops);

/**
 * detach for unified WMI
 *
 *  @param wmi_handle      : handle to WMI.
 *  @return void.
 */
void wmi_unified_detach(struct wmi_unified *wmi_handle);

void
wmi_unified_remove_work(struct wmi_unified *wmi_handle);

/**
 * generic function to allocate WMI buffer
 *
 *  @param wmi_handle      : handle to WMI.
 *  @param len             : length of the buffer
 *  @return wmi_buf_t.
 */
#ifdef MEMORY_DEBUG
#define wmi_buf_alloc(h, l) wmi_buf_alloc_debug(h, l, __FILE__, __LINE__)
wmi_buf_t
wmi_buf_alloc_debug(wmi_unified_t wmi_handle, uint16_t len,
		    uint8_t *file_name, uint32_t line_num);
#else
wmi_buf_t wmi_buf_alloc(wmi_unified_t wmi_handle, uint16_t len);
#endif

/**
 * generic function frees WMI net buffer
 *
 *  @param net_buf : Pointer ot net_buf to be freed
 */
void wmi_buf_free(wmi_buf_t net_buf);

/**
 * generic function to send unified WMI command
 *
 *  @param wmi_handle      : handle to WMI.
 *  @param buf             : wmi command buffer
 *  @param buflen          : wmi command buffer length
 *  @return 0  on success and -ve on failure.
 */
int
wmi_unified_cmd_send(wmi_unified_t wmi_handle, wmi_buf_t buf, uint32_t buflen,
		     WMI_CMD_ID cmd_id);

/**
 * wmi_unified_register_event_handler() - WMI event handler
 * registration function
 *
 * @wmi_handle:   handle to WMI.
 * @event_id:     WMI event ID
 * @handler_func: Event handler call back function
 * @rx_ctx: rx event processing context
 *
 *  @return 0  on success and -ve on failure.
 */
int
wmi_unified_register_event_handler(wmi_unified_t wmi_handle,
				   WMI_EVT_ID event_id,
				   wmi_unified_event_handler handler_func,
				   uint8_t rx_ctx);

/**
 * WMI event handler unregister function
 *
 *  @param wmi_handle      : handle to WMI.
 *  @param event_id        : WMI event ID
 *  @return 0  on success and -ve on failure.
 */
int
wmi_unified_unregister_event_handler(wmi_unified_t wmi_handle,
				     WMI_EVT_ID event_id);

/**
 * request wmi to connet its htc service.
 *  @param wmi_handle      : handle to WMI.
 *  @return void
 */
int
wmi_unified_connect_htc_service(struct wmi_unified *wmi_handle,
				void *htc_handle);

/*
 * WMI API to verify the host has enough credits to suspend
 */

int wmi_is_suspend_ready(wmi_unified_t wmi_handle);

/**
   WMI API to get updated host_credits
 */

int wmi_get_host_credits(wmi_unified_t wmi_handle);

/**
   WMI API to get WMI Pending Commands in the HTC queue
 */

int wmi_get_pending_cmds(wmi_unified_t wmi_handle);

/**
   WMI API to set target suspend state
 */
void wmi_set_target_suspend(wmi_unified_t wmi_handle, bool val);

#ifdef FEATURE_RUNTIME_PM
void
wmi_set_runtime_pm_inprogress(wmi_unified_t wmi_handle, bool val);
bool wmi_get_runtime_pm_inprogress(wmi_unified_t wmi_handle);
#else
static inline void
wmi_set_runtime_pm_inprogress(wmi_unified_t wmi_handle, bool val)
{
	return;
}
static inline bool wmi_get_runtime_pm_inprogress(wmi_unified_t wmi_handle)
{
	return false;
}
#endif


/**
 * UMAC Callback to process fw event.
 */
void wmi_process_fw_event(struct wmi_unified *wmi_handle, wmi_buf_t evt_buf);
uint16_t wmi_get_max_msg_len(wmi_unified_t wmi_handle);


QDF_STATUS wmi_unified_vdev_create_send(void *wmi_hdl,
				 uint8_t macaddr[IEEE80211_ADDR_LEN],
				 struct vdev_create_params *param);

QDF_STATUS wmi_unified_vdev_delete_send(void *wmi_hdl,
					  uint8_t if_id);

QDF_STATUS wmi_unified_vdev_start_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct vdev_start_params *param);

QDF_STATUS wmi_unified_vdev_restart_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct vdev_start_params *param);

QDF_STATUS wmi_unified_vdev_stop_send(void *wmi_hdl,
					uint8_t vdev_id);

QDF_STATUS wmi_unified_vdev_up_send(void *wmi_hdl,
			     uint8_t bssid[IEEE80211_ADDR_LEN],
				 struct vdev_up_params *params);

QDF_STATUS wmi_unified_vdev_down_send(void *wmi_hdl,
				uint8_t vdev_id);

QDF_STATUS wmi_unified_vdev_set_param_send(void *wmi_hdl,
				struct vdev_set_params *param);

QDF_STATUS wmi_unified_peer_delete_send(void *wmi_hdl,
				    uint8_t
				    peer_addr[IEEE80211_ADDR_LEN],
				    uint8_t vdev_id);

QDF_STATUS wmi_unified_peer_flush_tids_send(void *wmi_hdl,
					 uint8_t peer_addr[IEEE80211_ADDR_LEN],
					 struct peer_flush_params *param);

QDF_STATUS wmi_set_peer_param_send(void *wmi_hdl,
				uint8_t peer_addr[IEEE80211_ADDR_LEN],
				struct peer_set_params *param);

QDF_STATUS wmi_unified_peer_create_send(void *wmi_hdl,
					struct peer_create_params *param);

QDF_STATUS wmi_unified_stats_request_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct stats_request_params *param);

QDF_STATUS wmi_unified_green_ap_ps_send(void *wmi_hdl,
						uint32_t value, uint8_t mac_id);


QDF_STATUS wmi_unified_wow_enable_send(void *wmi_hdl,
				struct wow_cmd_params *param,
				uint8_t mac_id);

QDF_STATUS wmi_unified_packet_log_enable_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct packet_enable_params *param);


QDF_STATUS wmi_unified_suspend_send(void *wmi_hdl,
				struct suspend_params *param,
				uint8_t mac_id);

QDF_STATUS wmi_unified_resume_send(void *wmi_hdl,
				uint8_t mac_id);

QDF_STATUS
wmi_unified_pdev_param_send(void *wmi_hdl,
			   struct pdev_params *param,
				uint8_t mac_id);

QDF_STATUS wmi_unified_beacon_send_cmd(void *wmi_hdl,
				struct beacon_params *param);

QDF_STATUS wmi_unified_peer_assoc_send(void *wmi_hdl,
				struct peer_assoc_params *param);

QDF_STATUS wmi_unified_sta_ps_cmd_send(void *wmi_hdl,
				struct sta_ps_params *param);

QDF_STATUS wmi_unified_ap_ps_cmd_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct ap_ps_params *param);

QDF_STATUS wmi_unified_scan_start_cmd_send(void *wmi_hdl,
				struct scan_start_params *param);

QDF_STATUS wmi_unified_scan_stop_cmd_send(void *wmi_hdl,
				struct scan_stop_params *param);

QDF_STATUS wmi_unified_scan_chan_list_cmd_send(void *wmi_hdl,
				struct scan_chan_list_params *param);


QDF_STATUS wmi_crash_inject(void *wmi_hdl,
				struct crash_inject *param);

QDF_STATUS wmi_unified_pdev_utf_cmd_send(void *wmi_hdl,
				struct pdev_utf_params *param,
				uint8_t mac_id);

QDF_STATUS wmi_unified_dbglog_cmd_send(void *wmi_hdl,
				struct dbglog_params *param);

QDF_STATUS wmi_mgmt_unified_cmd_send(void *wmi_hdl,
				struct wmi_mgmt_params *param);

QDF_STATUS wmi_unified_modem_power_state(void *wmi_hdl,
		uint32_t param_value);

QDF_STATUS wmi_unified_set_sta_ps_mode(void *wmi_hdl,
			       uint32_t vdev_id, uint8_t val);
QDF_STATUS
wmi_unified_set_sta_uapsd_auto_trig_cmd(void *wmi_hdl,
				struct sta_uapsd_trig_params *param);

QDF_STATUS wmi_unified_get_temperature(void *wmi_hdl);

QDF_STATUS wmi_unified_set_p2pgo_oppps_req(void *wmi_hdl,
		struct p2p_ps_params *oppps);

QDF_STATUS wmi_unified_set_p2pgo_noa_req_cmd(void *wmi_hdl,
			struct p2p_ps_params *noa);

QDF_STATUS wmi_unified_set_smps_params(void *wmi_hdl, uint8_t vdev_id,
			       int value);

QDF_STATUS wmi_unified_set_mimops(void *wmi_hdl, uint8_t vdev_id, int value);

QDF_STATUS wmi_unified_ocb_set_utc_time(void *wmi_hdl,
				struct ocb_utc_param *utc);

QDF_STATUS wmi_unified_ocb_start_timing_advert(void *wmi_hdl,
	struct ocb_timing_advert_param *timing_advert);

QDF_STATUS wmi_unified_ocb_stop_timing_advert(void *wmi_hdl,
	struct ocb_timing_advert_param *timing_advert);

QDF_STATUS wmi_unified_ocb_set_config(void *wmi_hdl,
		   struct ocb_config_param *config, uint32_t *ch_mhz);

QDF_STATUS wmi_unified_ocb_get_tsf_timer(void *wmi_hdl,
			  uint8_t vdev_id);

QDF_STATUS wmi_unified_lro_config_cmd(void *wmi_hdl,
	 struct wmi_lro_config_cmd_t *wmi_lro_cmd);

QDF_STATUS wmi_unified_set_thermal_mgmt_cmd(void *wmi_hdl,
				struct thermal_cmd_params *thermal_info);

QDF_STATUS wmi_unified_set_mcc_channel_time_quota_cmd
	(void *wmi_hdl,
	uint32_t adapter_1_chan_freq,
	uint32_t adapter_1_quota, uint32_t adapter_2_chan_freq);

QDF_STATUS wmi_unified_set_mcc_channel_time_latency_cmd
	(void *wmi_hdl,
	uint32_t mcc_channel_freq, uint32_t mcc_channel_time_latency);

QDF_STATUS wmi_unified_set_enable_disable_mcc_adaptive_scheduler_cmd(
		   void *wmi_hdl, uint32_t mcc_adaptive_scheduler);

QDF_STATUS wmi_unified_bcn_buf_ll_cmd(void *wmi_hdl,
			wmi_bcn_send_from_host_cmd_fixed_param *param);

QDF_STATUS wmi_unified_set_sta_sa_query_param_cmd(void *wmi_hdl,
				       uint8_t vdev_id, uint32_t max_retries,
					   uint32_t retry_interval);


QDF_STATUS wmi_unified_set_sta_keep_alive_cmd(void *wmi_hdl,
				struct sta_params *params);

QDF_STATUS wmi_unified_vdev_set_gtx_cfg_cmd(void *wmi_hdl, uint32_t if_id,
				  struct wmi_gtx_config *gtx_info);


QDF_STATUS wmi_unified_process_update_edca_param(void *wmi_hdl,
		     uint8_t vdev_id,
		     wmi_wmm_vparams gwmm_param[WMI_MAX_NUM_AC]);


QDF_STATUS wmi_unified_probe_rsp_tmpl_send_cmd(void *wmi_hdl,
		   uint8_t vdev_id,
		   struct wmi_probe_resp_params *probe_rsp_info,
		   uint8_t *frm);

QDF_STATUS wmi_unified_p2p_go_set_beacon_ie_cmd(void *wmi_hdl,
				    A_UINT32 vdev_id, uint8_t *p2p_ie);


QDF_STATUS wmi_unified_set_gateway_params_cmd(void *wmi_hdl,
					struct gateway_update_req_param *req);

QDF_STATUS wmi_unified_set_rssi_monitoring_cmd(void *wmi_hdl,
					struct rssi_monitor_param *req);

QDF_STATUS wmi_unified_scan_probe_setoui_cmd(void *wmi_hdl,
			  struct scan_mac_oui *psetoui);

QDF_STATUS wmi_unified_reset_passpoint_network_list_cmd(void *wmi_hdl,
					struct wifi_passpoint_req_param *req);

QDF_STATUS wmi_unified_set_passpoint_network_list_cmd(void *wmi_hdl,
					struct wifi_passpoint_req_param *req);

QDF_STATUS wmi_unified_set_epno_network_list_cmd(void *wmi_hdl,
		struct wifi_enhanched_pno_params *req);

QDF_STATUS  wmi_unified_ipa_offload_control_cmd(void *wmi_hdl,
		struct ipa_offload_control_params *ipa_offload);

QDF_STATUS wmi_unified_extscan_get_capabilities_cmd(void *wmi_hdl,
			  struct extscan_capabilities_params *pgetcapab);

QDF_STATUS wmi_unified_extscan_get_cached_results_cmd(void *wmi_hdl,
			  struct extscan_cached_result_params *pcached_results);


QDF_STATUS wmi_unified_extscan_stop_change_monitor_cmd(void *wmi_hdl,
			  struct extscan_capabilities_reset_params *reset_req);


QDF_STATUS wmi_unified_extscan_start_change_monitor_cmd(void *wmi_hdl,
				   struct extscan_set_sig_changereq_params *
					   psigchange);

QDF_STATUS wmi_unified_extscan_stop_hotlist_monitor_cmd(void *wmi_hdl,
		  struct extscan_bssid_hotlist_reset_params *photlist_reset);

QDF_STATUS wmi_unified_stop_extscan_cmd(void *wmi_hdl,
			  struct extscan_stop_req_params *pstopcmd);

QDF_STATUS wmi_unified_start_extscan_cmd(void *wmi_hdl,
			  struct wifi_scan_cmd_req_params *pstart);

QDF_STATUS wmi_unified_plm_stop_cmd(void *wmi_hdl,
			  const struct plm_req_params *plm);

QDF_STATUS wmi_unified_plm_start_cmd(void *wmi_hdl,
			  const struct plm_req_params *plm,
			  uint32_t *gchannel_list);

QDF_STATUS wmi_unified_pno_stop_cmd(void *wmi_hdl, uint8_t vdev_id);

QDF_STATUS wmi_unified_pno_start_cmd(void *wmi_hdl,
		   struct pno_scan_req_params *pno,
		   uint32_t *gchannel_freq_list);

QDF_STATUS wmi_unified_set_ric_req_cmd(void *wmi_hdl, void *msg,
			uint8_t is_add_ts);

QDF_STATUS wmi_unified_process_ll_stats_clear_cmd
	(void *wmi_hdl, const struct ll_stats_clear_params *clear_req,
	 uint8_t addr[IEEE80211_ADDR_LEN]);

QDF_STATUS wmi_unified_process_ll_stats_set_cmd
	(void *wmi_hdl, const struct ll_stats_set_params *set_req);

QDF_STATUS wmi_unified_process_ll_stats_get_cmd
	(void *wmi_hdl, const struct ll_stats_get_params  *get_req,
		 uint8_t addr[IEEE80211_ADDR_LEN]);

QDF_STATUS wmi_unified_get_stats_cmd(void *wmi_hdl,
		       struct pe_stats_req  *get_stats_param,
			   uint8_t addr[IEEE80211_ADDR_LEN]);

QDF_STATUS wmi_unified_snr_request_cmd(void *wmi_hdl);

QDF_STATUS wmi_unified_snr_cmd(void *wmi_hdl, uint8_t vdev_id);

QDF_STATUS wmi_unified_link_status_req_cmd(void *wmi_hdl,
				 struct link_status_params *link_status);

QDF_STATUS wmi_unified_lphb_config_hbenable_cmd(void *wmi_hdl,
				wmi_hb_set_enable_cmd_fixed_param *params);

QDF_STATUS wmi_unified_lphb_config_tcp_params_cmd(void *wmi_hdl,
				    wmi_hb_set_tcp_params_cmd_fixed_param *lphb_conf_req);

QDF_STATUS wmi_unified_lphb_config_tcp_pkt_filter_cmd(void *wmi_hdl,
					wmi_hb_set_tcp_pkt_filter_cmd_fixed_param *g_hb_tcp_filter_fp);

QDF_STATUS wmi_unified_lphb_config_udp_params_cmd(void *wmi_hdl,
				    wmi_hb_set_udp_params_cmd_fixed_param *lphb_conf_req);

QDF_STATUS wmi_unified_lphb_config_udp_pkt_filter_cmd(void *wmi_hdl,
					wmi_hb_set_udp_pkt_filter_cmd_fixed_param *lphb_conf_req);

QDF_STATUS wmi_unified_process_dhcp_ind(void *wmi_hdl,
				wmi_peer_set_param_cmd_fixed_param *ta_dhcp_ind);

QDF_STATUS wmi_unified_get_link_speed_cmd(void *wmi_hdl,
			wmi_mac_addr peer_macaddr);

QDF_STATUS wmi_unified_egap_conf_params_cmd(void *wmi_hdl,
				     wmi_ap_ps_egap_param_cmd_fixed_param *egap_params);

QDF_STATUS wmi_unified_fw_profiling_data_cmd(void *wmi_hdl,
			uint32_t cmd, uint32_t value1, uint32_t value2);

QDF_STATUS wmi_unified_wow_sta_ra_filter_cmd(void *wmi_hdl,
			  uint8_t vdev_id, uint8_t default_pattern,
			  uint16_t rate_limit_interval);

QDF_STATUS wmi_unified_nat_keepalive_en_cmd(void *wmi_hdl, uint8_t vdev_id);

QDF_STATUS wmi_unified_csa_offload_enable(void *wmi_hdl, uint8_t vdev_id);

QDF_STATUS wmi_unified_start_oem_data_cmd(void *wmi_hdl,
			  uint8_t data_len,
			  uint8_t *data);

QDF_STATUS wmi_unified_dfs_phyerr_filter_offload_en_cmd(void *wmi_hdl,
			bool dfs_phyerr_filter_offload);

QDF_STATUS wmi_unified_pktlog_wmi_send_cmd(void *wmi_hdl,
				   WMI_PKTLOG_EVENT pktlog_event,
				   WMI_CMD_ID cmd_id);

QDF_STATUS wmi_unified_add_wow_wakeup_event_cmd(void *wmi_hdl,
					uint32_t vdev_id,
					uint32_t bitmap,
					bool enable);

QDF_STATUS wmi_unified_wow_patterns_to_fw_cmd(void *wmi_hdl,
				uint8_t vdev_id, uint8_t ptrn_id,
				const uint8_t *ptrn, uint8_t ptrn_len,
				uint8_t ptrn_offset, const uint8_t *mask,
				uint8_t mask_len, bool user,
				uint8_t default_patterns);

QDF_STATUS wmi_unified_wow_delete_pattern_cmd(void *wmi_hdl, uint8_t ptrn_id,
					uint8_t vdev_id);

QDF_STATUS wmi_unified_host_wakeup_ind_to_fw_cmd(void *wmi_hdl);
QDF_STATUS wmi_unified_del_ts_cmd(void *wmi_hdl, uint8_t vdev_id,
				uint8_t ac);

QDF_STATUS wmi_unified_aggr_qos_cmd(void *wmi_hdl,
		      struct aggr_add_ts_param *aggr_qos_rsp_msg);

QDF_STATUS wmi_unified_add_ts_cmd(void *wmi_hdl,
		 struct add_ts_param *msg);

QDF_STATUS wmi_unified_enable_disable_packet_filter_cmd(void *wmi_hdl,
					uint8_t vdev_id, bool enable);

QDF_STATUS wmi_unified_config_packet_filter_cmd(void *wmi_hdl,
		uint8_t vdev_id, struct rcv_pkt_filter_config *rcv_filter_param,
		uint8_t filter_id, bool enable);

QDF_STATUS wmi_unified_add_clear_mcbc_filter_cmd(void *wmi_hdl,
				     uint8_t vdev_id,
				     struct qdf_mac_addr multicast_addr,
				     bool clearList);

QDF_STATUS wmi_unified_send_gtk_offload_cmd(void *wmi_hdl, uint8_t vdev_id,
					   struct gtk_offload_params *params,
					   bool enable_offload,
					   uint32_t gtk_offload_opcode);

QDF_STATUS wmi_unified_process_gtk_offload_getinfo_cmd(void *wmi_hdl,
				uint8_t vdev_id,
				uint64_t offload_req_opcode);

QDF_STATUS wmi_unified_process_add_periodic_tx_ptrn_cmd(void *wmi_hdl,
						struct periodic_tx_pattern  *
						pAddPeriodicTxPtrnParams,
						uint8_t vdev_id);

QDF_STATUS wmi_unified_process_del_periodic_tx_ptrn_cmd(void *wmi_hdl,
						uint8_t vdev_id,
						uint8_t pattern_id);

QDF_STATUS wmi_unified_stats_ext_req_cmd(void *wmi_hdl,
			struct stats_ext_params *preq);

QDF_STATUS wmi_unified_enable_ext_wow_cmd(void *wmi_hdl,
			struct ext_wow_params *params);

QDF_STATUS wmi_unified_set_app_type2_params_in_fw_cmd(void *wmi_hdl,
					  struct app_type2_params *appType2Params);

QDF_STATUS wmi_unified_set_auto_shutdown_timer_cmd(void *wmi_hdl,
						  uint32_t timer_val);

QDF_STATUS wmi_unified_nan_req_cmd(void *wmi_hdl,
			struct nan_req_params *nan_req);

QDF_STATUS wmi_unified_process_dhcpserver_offload_cmd(void *wmi_hdl,
				struct dhcp_offload_info_params *pDhcpSrvOffloadInfo);

QDF_STATUS wmi_unified_process_ch_avoid_update_cmd(void *wmi_hdl);

QDF_STATUS wmi_unified_send_regdomain_info_to_fw_cmd(void *wmi_hdl,
				   uint32_t reg_dmn, uint16_t regdmn2G,
				   uint16_t regdmn5G, int8_t ctl2G,
				   int8_t ctl5G);

QDF_STATUS wmi_unified_set_tdls_offchan_mode_cmd(void *wmi_hdl,
			      struct tdls_channel_switch_params *chan_switch_params);

QDF_STATUS wmi_unified_update_fw_tdls_state_cmd(void *wmi_hdl,
					 void *tdls_param, uint8_t tdls_state);

QDF_STATUS wmi_unified_update_tdls_peer_state_cmd(void *wmi_hdl,
			       struct tdls_peer_state_params *peerStateParams,
				   uint32_t *ch_mhz);

QDF_STATUS wmi_unified_process_fw_mem_dump_cmd(void *wmi_hdl,
					struct fw_dump_req_param *mem_dump_req);

QDF_STATUS wmi_unified_process_set_ie_info_cmd(void *wmi_hdl,
				   struct vdev_ie_info_param *ie_info);

QDF_STATUS wmi_unified_ocb_set_utc_time_cmd(void *wmi_hdl,
			  struct ocb_utc_param *utc);

QDF_STATUS wmi_unified_dcc_get_stats_cmd(void *wmi_hdl,
		     struct dcc_get_stats_param *get_stats_param);

QDF_STATUS wmi_unified_dcc_clear_stats(void *wmi_hdl,
				uint32_t vdev_id, uint32_t dcc_stats_bitmap);

QDF_STATUS wmi_unified_dcc_update_ndl(void *wmi_hdl,
		       struct dcc_update_ndl_param *update_ndl_param);

QDF_STATUS wmi_unified_save_fw_version_cmd(void *wmi_hdl,
		void *evt_buf);

QDF_STATUS wmi_unified_send_init_cmd(void *wmi_hdl,
		wmi_resource_config *res_cfg,
		uint8_t num_mem_chunks, struct wmi_host_mem_chunk *mem_chunk,
		bool action);

QDF_STATUS wmi_unified_send_saved_init_cmd(void *wmi_hdl);

QDF_STATUS wmi_unified_set_base_macaddr_indicate_cmd(void *wmi_hdl,
					 uint8_t *custom_addr);

QDF_STATUS wmi_unified_log_supported_evt_cmd(void *wmi_hdl,
		uint8_t *event,
		uint32_t len);

QDF_STATUS wmi_unified_enable_specific_fw_logs_cmd(void *wmi_hdl,
		struct wmi_wifi_start_log *start_log);

QDF_STATUS wmi_unified_flush_logs_to_fw_cmd(void *wmi_hdl);

QDF_STATUS wmi_unified_soc_set_pcl_cmd(void *wmi_hdl,
				struct wmi_pcl_list *msg);

QDF_STATUS wmi_unified_soc_set_hw_mode_cmd(void *wmi_hdl,
				uint32_t hw_mode_index);

QDF_STATUS wmi_unified_soc_set_dual_mac_config_cmd(void *wmi_hdl,
		struct wmi_dual_mac_config *msg);

QDF_STATUS wmi_unified_enable_arp_ns_offload_cmd(void *wmi_hdl,
			   struct host_offload_req_param *param, bool arp_only,
			   uint8_t vdev_id);

QDF_STATUS wmi_unified_set_led_flashing_cmd(void *wmi_hdl,
				struct flashing_req_params *flashing);

QDF_STATUS wmi_unified_app_type1_params_in_fw_cmd(void *wmi_hdl,
				   struct app_type1_params *app_type1_params);

QDF_STATUS wmi_unified_set_ssid_hotlist_cmd(void *wmi_hdl,
		     struct ssid_hotlist_request_params *request);

QDF_STATUS wmi_unified_roam_synch_complete_cmd(void *wmi_hdl,
		 uint8_t vdev_id);

QDF_STATUS wmi_unified_unit_test_cmd(void *wmi_hdl,
			       struct wmi_unit_test_cmd *wmi_utest);

QDF_STATUS wmi_unified_roam_invoke_cmd(void *wmi_hdl,
		struct wmi_roam_invoke_cmd *roaminvoke,
		uint32_t ch_hz);

QDF_STATUS wmi_unified_roam_scan_offload_cmd(void *wmi_hdl,
					 uint32_t command, uint32_t vdev_id);

QDF_STATUS wmi_unified_send_roam_scan_offload_ap_cmd(void *wmi_hdl,
					    wmi_ap_profile *ap_profile_p,
					    uint32_t vdev_id);

QDF_STATUS wmi_unified_roam_scan_offload_scan_period(void *wmi_hdl,
					     uint32_t scan_period,
					     uint32_t scan_age,
					     uint32_t vdev_id);

QDF_STATUS wmi_unified_roam_scan_offload_chan_list_cmd(void *wmi_hdl,
				   uint8_t chan_count,
				   uint8_t *chan_list,
				   uint8_t list_type, uint32_t vdev_id);

QDF_STATUS wmi_unified_roam_scan_offload_rssi_change_cmd(void *wmi_hdl,
			  uint32_t vdev_id,
			  int32_t rssi_change_thresh,
			  uint32_t bcn_rssi_weight,
			  uint32_t hirssi_delay_btw_scans);

QDF_STATUS wmi_unified_get_buf_extscan_hotlist_cmd(void *wmi_hdl,
				   struct ext_scan_setbssi_hotlist_params *
				   photlist, int *buf_len);
#endif /* _WMI_UNIFIED_API_H_ */
