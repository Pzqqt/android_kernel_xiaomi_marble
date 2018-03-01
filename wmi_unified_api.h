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
 * This file contains the API definitions for the Unified Wireless Module
 * Interface (WMI).
 */

#ifndef _WMI_UNIFIED_API_H_
#define _WMI_UNIFIED_API_H_

#include <osdep.h>
#ifdef CONFIG_MCL
#include "wmi.h"
#endif
#include "htc_api.h"
#include "wmi_unified_param.h"
#include "service_ready_param.h"
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_mgmt_txrx_utils_api.h"
#ifdef WLAN_PMO_ENABLE
#include "wmi_unified_pmo_api.h"
#endif
#ifdef CONVERGED_P2P_ENABLE
#include "wlan_p2p_public_struct.h"
#endif
#include "wlan_scan_public_structs.h"
#ifdef WLAN_FEATURE_DISA
#include "wlan_disa_public_struct.h"
#endif

#ifdef WLAN_FEATURE_NAN_CONVERGENCE
#include "nan_public_structs.h"
#endif
#ifdef WLAN_SUPPORT_GREEN_AP
#include "wlan_green_ap_api.h"
#endif
#ifdef WLAN_FEATURE_DSRC
#include "wlan_ocb_public_structs.h"
#endif

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

#define PHYERROR_MAX_BUFFER_LENGTH 0x7F000000

struct wmi_soc;
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
	WMI_NON_TLV_TARGET,
	WMI_MAX_TARGET_TYPE
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
 *  @param target_type      : type of supported wmi command
 *  @param use_cookie       : flag to indicate cookie based allocation
 *  @param ops              : handle to wmi ops
 *  @psoc                   : objmgr psoc
 *  @return opaque handle.
 */
void *wmi_unified_attach(void *scn_handle,
			 osdev_t osdev, enum wmi_target_type target_type,
			 bool use_cookie, struct wmi_rx_ops *ops,
			 struct wlan_objmgr_psoc *psoc);



/**
 * wmi_mgmt_cmd_record() - Wrapper function for mgmt command logging macro
 *
 * @wmi_handle: wmi handle
 * @cmd: mgmt command
 * @header: pointer to 802.11 header
 * @vdev_id: vdev id
 * @chanfreq: channel frequency
 *
 * Return: none
 */
void wmi_mgmt_cmd_record(wmi_unified_t wmi_handle, uint32_t cmd,
			void *header, uint32_t vdev_id, uint32_t chanfreq);

/**
 * detach for unified WMI
 *
 *  @param wmi_handle      : handle to WMI.
 *  @return void.
 */
void wmi_unified_detach(struct wmi_unified *wmi_handle);

/**
 * API to sync time between host and firmware
 *
 *  @param wmi_handle      : handle to WMI.
 *  @return void.
 */
void wmi_send_time_stamp_sync_cmd_tlv(void *wmi_hdl);

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
 *  @param cmd_id          : WMI cmd id
 *  @return 0  on success and -ve on failure.
 */
QDF_STATUS
wmi_unified_cmd_send(wmi_unified_t wmi_handle, wmi_buf_t buf, uint32_t buflen,
			uint32_t cmd_id);

/**
 * wmi_unified_register_event() - WMI event handler
 * registration function for converged components
 *
 * @wmi_handle:   handle to WMI.
 * @event_id:     WMI event ID
 * @handler_func: Event handler call back function
 *
 *  @return 0  on success and -ve on failure.
 */
int
wmi_unified_register_event(wmi_unified_t wmi_handle,
				   uint32_t event_id,
				   wmi_unified_event_handler handler_func);

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
				   wmi_conv_event_id event_id,
				   wmi_unified_event_handler handler_func,
				   uint8_t rx_ctx);

/**
 * WMI event handler unregister function for converged componets
 *
 *  @param wmi_handle      : handle to WMI.
 *  @param event_id        : WMI event ID
 *  @return 0  on success and -ve on failure.
 */
int
wmi_unified_unregister_event(wmi_unified_t wmi_handle,
					 uint32_t event_id);

/**
 * WMI event handler unregister function
 *
 *  @param wmi_handle      : handle to WMI.
 *  @param event_id        : WMI event ID
 *  @return 0  on success and -ve on failure.
 */
int
wmi_unified_unregister_event_handler(wmi_unified_t wmi_handle,
					 wmi_conv_event_id event_id);

/**
 * request wmi to connet its htc service.
 *  @param wmi_handle      : handle to WMI.
 *  @param htc_handle      : handle to HTC.
 *  @return void
 */
QDF_STATUS
wmi_unified_connect_htc_service(struct wmi_unified *wmi_handle,
				void *htc_handle);

/*
 * WMI API to verify the host has enough credits to suspend
 *  @param wmi_handle      : handle to WMI.
 */

int wmi_is_suspend_ready(wmi_unified_t wmi_handle);

/**
 *  WMI API to get updated host_credits
 *  @param wmi_handle      : handle to WMI.
 */

int wmi_get_host_credits(wmi_unified_t wmi_handle);

/**
 *  WMI API to get WMI Pending Commands in the HTC queue
 *  @param wmi_handle      : handle to WMI.
 */

int wmi_get_pending_cmds(wmi_unified_t wmi_handle);

/**
 *  WMI API to set target suspend state
 *  @param wmi_handle      : handle to WMI.
 *  @param val             : suspend state boolean
 */
void wmi_set_target_suspend(wmi_unified_t wmi_handle, bool val);

/**
 * WMI API to set bus suspend state
 * @param wmi_handle:	handle to WMI.
 * @param val:		suspend state boolean
 */
void wmi_set_is_wow_bus_suspended(wmi_unified_t wmi_handle, A_BOOL val);

/**
 * WMI API to set crash injection state
 * @param wmi_handle:	handle to WMI.
 * @param val:		crash injection state boolean
 */
void wmi_tag_crash_inject(wmi_unified_t wmi_handle, A_BOOL flag);

/**
 * WMI API to set target assert
 * @param wmi_handle: 	handle to WMI.
 * @param val:		target assert config value.
 *
 * Return: 		none.
 */
void wmi_set_tgt_assert(wmi_unified_t wmi_handle, bool val);

/**
 * generic function to block unified WMI command
 * @param wmi_handle      : handle to WMI.
 * @return 0  on success and -ve on failure.
 */
int
wmi_stop(wmi_unified_t wmi_handle);

/**
 * API to flush all the previous packets  associated with the wmi endpoint
 *
 * @param wmi_handle      : handle to WMI.
 */
void
wmi_flush_endpoint(wmi_unified_t wmi_handle);

/**
 * wmi_pdev_id_conversion_enable() - API to enable pdev_id conversion in WMI
 *                     By default pdev_id conversion is not done in WMI.
 *                     This API can be used enable conversion in WMI.
 * @param wmi_handle   : handle to WMI
 * Return none
 */
void wmi_pdev_id_conversion_enable(wmi_unified_t wmi_handle);

/**
 * API to handle wmi rx event after UMAC has taken care of execution
 * context
 *
 * @param wmi_handle      : handle to WMI.
 * @param evt_buf         : wmi event buffer
 */
void __wmi_control_rx(struct wmi_unified *wmi_handle, wmi_buf_t evt_buf);
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

void *wmi_unified_get_soc_handle(struct wmi_unified *wmi_handle);

void *wmi_unified_get_pdev_handle(struct wmi_soc *soc, uint32_t pdev_idx);

/**
 * UMAC Callback to process fw event.
 * @param wmi_handle      : handle to WMI.
 * @param evt_buf         : wmi event buffer
 */
void wmi_process_fw_event(struct wmi_unified *wmi_handle, wmi_buf_t evt_buf);
uint16_t wmi_get_max_msg_len(wmi_unified_t wmi_handle);


QDF_STATUS wmi_unified_vdev_create_send(void *wmi_hdl,
				 uint8_t macaddr[IEEE80211_ADDR_LEN],
				 struct vdev_create_params *param);

QDF_STATUS wmi_unified_vdev_delete_send(void *wmi_hdl,
					  uint8_t if_id);

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

QDF_STATUS wmi_unified_vdev_start_send(void *wmi_hdl,
				struct vdev_start_params *req);
/**
 * wmi_unified_vdev_set_nac_rssi_send() - send NAC_RSSI command to fw
 * @param wmi_handle   : handle to WMI
 * @param req          : pointer to hold nac rssi request data
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_set_nac_rssi_send(void *wmi_hdl,
			struct vdev_scan_nac_rssi_params *req);

QDF_STATUS wmi_unified_hidden_ssid_vdev_restart_send(void *wmi_hdl,
		struct hidden_ssid_vdev_restart_params *restart_params);

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
					uint32_t value, uint8_t pdev_id);

#ifdef FEATURE_WLAN_D0WOW
QDF_STATUS wmi_unified_d0wow_enable_send(void *wmi_hdl,
				uint8_t mac_id);
QDF_STATUS wmi_unified_d0wow_disable_send(void *wmi_hdl,
				uint8_t mac_id);
#endif

QDF_STATUS wmi_unified_wow_enable_send(void *wmi_hdl,
				struct wow_cmd_params *param,
				uint8_t mac_id);

QDF_STATUS wmi_unified_wow_wakeup_send(void *wmi_hdl);

QDF_STATUS wmi_unified_wow_add_wakeup_event_send(void *wmi_hdl,
		struct wow_add_wakeup_params *param);

QDF_STATUS wmi_unified_wow_add_wakeup_pattern_send(void *wmi_hdl,
		struct wow_add_wakeup_pattern_params *param);

QDF_STATUS wmi_unified_wow_remove_wakeup_pattern_send(void *wmi_hdl,
		struct wow_remove_wakeup_pattern_params *param);

#ifndef CONFIG_MCL
QDF_STATUS wmi_unified_packet_log_enable_send(void *wmi_hdl,
			WMI_HOST_PKTLOG_EVENT PKTLOG_EVENT, uint8_t mac_id);
#else
QDF_STATUS wmi_unified_packet_log_enable_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct packet_enable_params *param);
#endif

QDF_STATUS wmi_unified_packet_log_disable_send(void *wmi_hdl, uint8_t mac_id);

QDF_STATUS wmi_unified_suspend_send(void *wmi_hdl,
				struct suspend_params *param,
				uint8_t mac_id);

QDF_STATUS wmi_unified_resume_send(void *wmi_hdl,
				uint8_t mac_id);

QDF_STATUS
wmi_unified_pdev_param_send(void *wmi_hdl,
			   struct pdev_params *param,
				uint8_t mac_id);

QDF_STATUS wmi_unified_beacon_tmpl_send_cmd(void *wmi_hdl,
				struct beacon_tmpl_params *param);


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
				struct scan_req_params *param);

QDF_STATUS wmi_unified_scan_stop_cmd_send(void *wmi_hdl,
				struct scan_cancel_param *param);

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

QDF_STATUS wmi_offchan_data_tx_cmd_send(void *wmi_hdl,
				struct wmi_offchan_data_tx_params *param);

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

#ifdef CONVERGED_P2P_ENABLE
QDF_STATUS wmi_unified_p2p_lo_start_cmd(void *wmi_hdl,
			struct p2p_lo_start *param);

QDF_STATUS wmi_unified_p2p_lo_stop_cmd(void *wmi_hdl, uint8_t vdev_id);
#endif

QDF_STATUS wmi_unified_set_smps_params(void *wmi_hdl, uint8_t vdev_id,
			       int value);

QDF_STATUS wmi_unified_set_mimops(void *wmi_hdl, uint8_t vdev_id, int value);

#ifdef WLAN_FEATURE_DSRC
/**
 * wmi_unified_ocb_start_timing_advert() - start sending the timing
 *  advertisement frames on a channel
 * @wmi_handle: pointer to the wmi handle
 * @timing_advert: pointer to the timing advertisement struct
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_ocb_start_timing_advert(struct wmi_unified *wmi_handle,
	struct ocb_timing_advert_param *timing_advert);

/**
 * wmi_unified_ocb_stop_timing_advert() - stop sending the timing
 *  advertisement frames on a channel
 * @wmi_handle: pointer to the wmi handle
 * @timing_advert: pointer to the timing advertisement struct
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_ocb_stop_timing_advert(struct wmi_unified *wmi_handle,
	struct ocb_timing_advert_param *timing_advert);

/**
 * wmi_unified_ocb_set_config() - send the OCB config to the FW
 * @wmi_handle: pointer to the wmi handle
 * @config: the OCB configuration
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failures
 */
QDF_STATUS wmi_unified_ocb_set_config(struct wmi_unified *wmi_handle,
				      struct ocb_config *config);

/**
 * wmi_unified_ocb_get_tsf_timer() - get ocb tsf timer val
 * @wmi_handle: pointer to the wmi handle
 * @req: request for tsf timer
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_ocb_get_tsf_timer(struct wmi_unified *wmi_handle,
					 struct ocb_get_tsf_timer_param *req);

/**
 * wmi_unified_ocb_set_utc_time_cmd() - get ocb tsf timer val
 * @wmi_handle: pointer to the wmi handle
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_ocb_set_utc_time_cmd(struct wmi_unified *wmi_handle,
					    struct ocb_utc_param *utc);

/**
 * wmi_unified_dcc_get_stats_cmd() - get the DCC channel stats
 * @wmi_handle: pointer to the wmi handle
 * @get_stats_param: pointer to the dcc stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_dcc_get_stats_cmd(struct wmi_unified *wmi_handle,
		     struct ocb_dcc_get_stats_param *get_stats_param);

/**
 * wmi_unified_dcc_clear_stats() - command to clear the DCC stats
 * @wmi_handle: pointer to the wmi handle
 * @clear_stats_param: parameters to the command
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_dcc_clear_stats(struct wmi_unified *wmi_handle,
			struct ocb_dcc_clear_stats_param *clear_stats_param);

/**
 * wmi_unified_dcc_update_ndl() - command to update the NDL data
 * @wmi_handle: pointer to the wmi handle
 * @update_ndl_param: pointer to the request parameters
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failures
 */
QDF_STATUS wmi_unified_dcc_update_ndl(struct wmi_unified *wmi_handle,
		       struct ocb_dcc_update_ndl_param *update_ndl_param);

/**
 * wmi_extract_ocb_set_channel_config_resp() - extract status from wmi event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @status: status buffer
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS
wmi_extract_ocb_set_channel_config_resp(struct wmi_unified *wmi_handle,
					void *evt_buf,
					uint32_t *status);

/**
 * wmi_extract_ocb_tsf_timer() - extract tsf timer from wmi event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @resp: tsf timer
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS wmi_extract_ocb_tsf_timer(struct wmi_unified *wmi_handle,
				     void *evt_buf,
				     struct ocb_get_tsf_timer_response *resp);

/**
 * wmi_extract_dcc_update_ndl_resp() - extract NDL update from wmi event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @resp: ndl update status
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS wmi_extract_dcc_update_ndl_resp(struct wmi_unified *wmi_handle,
		void *evt_buf, struct ocb_dcc_update_ndl_response *resp);

/**
 * wmi_extract_dcc_stats() - extract DCC stats from wmi event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @resp: DCC stats
 *
 * Since length of the response is variable, response buffer will be allocated.
 * The caller must free the response buffer.
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS wmi_extract_dcc_stats(struct wmi_unified *wmi_handle,
				 void *evt_buf,
				 struct ocb_dcc_get_stats_response **response);
#endif

QDF_STATUS wmi_unified_lro_config_cmd(void *wmi_hdl,
	 struct wmi_lro_config_cmd_t *wmi_lro_cmd);

QDF_STATUS wmi_unified_set_thermal_mgmt_cmd(void *wmi_hdl,
				struct thermal_cmd_params *thermal_info);

QDF_STATUS wmi_unified_peer_rate_report_cmd(void *wmi_hdl,
		struct wmi_peer_rate_report_params *rate_report_params);

QDF_STATUS wmi_unified_set_mcc_channel_time_quota_cmd
	(void *wmi_hdl,
	uint32_t adapter_1_chan_freq,
	uint32_t adapter_1_quota, uint32_t adapter_2_chan_freq);

QDF_STATUS wmi_unified_set_mcc_channel_time_latency_cmd
	(void *wmi_hdl,
	uint32_t mcc_channel_freq, uint32_t mcc_channel_time_latency);

QDF_STATUS wmi_unified_set_enable_disable_mcc_adaptive_scheduler_cmd(
		   void *wmi_hdl, uint32_t mcc_adaptive_scheduler,
		   uint32_t pdev_id);

#ifdef CONFIG_MCL
QDF_STATUS wmi_unified_bcn_buf_ll_cmd(void *wmi_hdl,
			wmi_bcn_send_from_host_cmd_fixed_param *param);
#endif

QDF_STATUS wmi_unified_set_sta_sa_query_param_cmd(void *wmi_hdl,
				       uint8_t vdev_id, uint32_t max_retries,
					   uint32_t retry_interval);


QDF_STATUS wmi_unified_set_sta_keep_alive_cmd(void *wmi_hdl,
				struct sta_params *params);

QDF_STATUS wmi_unified_vdev_set_gtx_cfg_cmd(void *wmi_hdl, uint32_t if_id,
				  struct wmi_gtx_config *gtx_info);

QDF_STATUS wmi_unified_process_update_edca_param(void *wmi_hdl,
		     uint8_t vdev_id,
		     struct wmi_host_wme_vparams wmm_vparams[WMI_MAX_NUM_AC]);

QDF_STATUS wmi_unified_probe_rsp_tmpl_send_cmd(void *wmi_hdl,
		   uint8_t vdev_id,
		   struct wmi_probe_resp_params *probe_rsp_info);

QDF_STATUS wmi_unified_setup_install_key_cmd(void *wmi_hdl,
			struct set_key_params *key_params);

#ifdef WLAN_FEATURE_DISA
/**
 * wmi_unified_encrypt_decrypt_send_cmd() - send encryptdecrypt cmd to fw
 * @wmi_hdl: wmi handle
 * @params: encrypt/decrypt params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_encrypt_decrypt_send_cmd(void *wmi_hdl,
			struct disa_encrypt_decrypt_req_params *params);

/**
 * wmi_extract_encrypt_decrypt_resp_params() -
 *       extract encrypt decrypt resp params from event buffer
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @resp: encrypt decrypt resp params
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_extract_encrypt_decrypt_resp_params(void *wmi_hdl,
			uint8_t *evt_buf,
			struct disa_encrypt_decrypt_resp_params *resp);
#endif

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

#ifdef CONFIG_MCL
QDF_STATUS wmi_unified_roam_scan_offload_mode_cmd(void *wmi_hdl,
				wmi_start_scan_cmd_fixed_param *scan_cmd_fp,
				struct roam_offload_scan_params *roam_req);
#endif

/**
 * wmi_unified_roam_mawc_params_cmd() - configure roaming MAWC parameters
 * @wmi_hdl: wmi handle
 * @params: Parameters to be configured
 *
 * Pass the MAWC(Motion Aided wireless connectivity) related roaming
 * parameters from the host to the target
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_roam_mawc_params_cmd(void *wmi_hdl,
			struct wmi_mawc_roam_params *params);

QDF_STATUS wmi_unified_roam_scan_offload_rssi_thresh_cmd(void *wmi_hdl,
				struct roam_offload_scan_rssi_params *roam_req);

QDF_STATUS wmi_unified_roam_scan_filter_cmd(void *wmi_hdl,
				struct roam_scan_filter_params *roam_req);

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

#ifdef FEATURE_WLAN_SCAN_PNO
QDF_STATUS wmi_unified_pno_start_cmd(void *wmi_hdl,
		   struct pno_scan_req_params *pno);
#endif

QDF_STATUS wmi_unified_nlo_mawc_cmd(void *wmi_hdl,
		struct nlo_mawc_params *params);

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

/**
 * wmi_unified_congestion_request_cmd() - send request to fw to get CCA
 * @wmi_hdl: wma handle
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_congestion_request_cmd(void *wmi_hdl,
		uint8_t vdev_id);

QDF_STATUS wmi_unified_snr_request_cmd(void *wmi_hdl);

QDF_STATUS wmi_unified_snr_cmd(void *wmi_hdl, uint8_t vdev_id);

QDF_STATUS wmi_unified_link_status_req_cmd(void *wmi_hdl,
				 struct link_status_params *link_status);

#ifdef CONFIG_MCL
QDF_STATUS wmi_unified_process_dhcp_ind(void *wmi_hdl,
				wmi_peer_set_param_cmd_fixed_param *ta_dhcp_ind);

QDF_STATUS wmi_unified_get_link_speed_cmd(void *wmi_hdl,
					wmi_mac_addr peer_macaddr);
#endif

#ifdef WLAN_SUPPORT_GREEN_AP
QDF_STATUS wmi_unified_egap_conf_params_cmd(void *wmi_hdl,
		struct wlan_green_ap_egap_params *egap_params);
#endif

QDF_STATUS wmi_unified_fw_profiling_data_cmd(void *wmi_hdl,
			uint32_t cmd, uint32_t value1, uint32_t value2);

QDF_STATUS wmi_unified_wow_timer_pattern_cmd(void *wmi_hdl, uint8_t vdev_id,
					     uint32_t cookie, uint32_t time);

QDF_STATUS wmi_unified_nat_keepalive_en_cmd(void *wmi_hdl, uint8_t vdev_id);

/**
 * wmi_unified_set_latency_config_cmd()
 * @wmi_handle: wmi handle
 * @param: WLM parameters
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_wlm_latency_level_cmd(void *wmi_hdl,
					struct wlm_latency_level_param *param);

QDF_STATUS wmi_unified_csa_offload_enable(void *wmi_hdl, uint8_t vdev_id);

#ifdef WLAN_FEATURE_CIF_CFR
/**
 * wmi_unified_oem_dma_ring_cfg() - configure OEM DMA rings
 * @wmi_handle: wmi handle
 * @data_len: len of dma cfg req
 * @data: dma cfg req
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_oem_dma_ring_cfg(void *wmi_hdl,
				wmi_oem_dma_ring_cfg_req_fixed_param *cfg);
#endif

/**
 * wmi_unified_dbr_ring_cfg: Configure direct buffer rx rings
 * @wmi_hdl: WMI handle
 * @cfg: pointer to direct buffer rx config request
 *
 * Return: QDF status of operation
 */
QDF_STATUS wmi_unified_dbr_ring_cfg(void *wmi_hdl,
				struct direct_buf_rx_cfg_req *cfg);

QDF_STATUS wmi_unified_start_oem_data_cmd(void *wmi_hdl,
			  uint32_t data_len,
			  uint8_t *data);

QDF_STATUS wmi_unified_dfs_phyerr_filter_offload_en_cmd(void *wmi_hdl,
			bool dfs_phyerr_filter_offload);

#ifdef CONFIG_MCL
QDF_STATUS wmi_unified_pktlog_wmi_send_cmd(void *wmi_hdl,
				   WMI_PKTLOG_EVENT pktlog_event,
				   uint32_t cmd_id,
				   uint8_t user_triggered);
#endif

QDF_STATUS wmi_unified_wow_delete_pattern_cmd(void *wmi_hdl, uint8_t ptrn_id,
					uint8_t vdev_id);

QDF_STATUS wmi_unified_host_wakeup_ind_to_fw_cmd(void *wmi_hdl);
QDF_STATUS wmi_unified_del_ts_cmd(void *wmi_hdl, uint8_t vdev_id,
				uint8_t ac);

QDF_STATUS wmi_unified_aggr_qos_cmd(void *wmi_hdl,
		      struct aggr_add_ts_param *aggr_qos_rsp_msg);

QDF_STATUS wmi_unified_add_ts_cmd(void *wmi_hdl,
		 struct add_ts_param *msg);

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
				struct dhcp_offload_info_params *params);

QDF_STATUS wmi_unified_process_ch_avoid_update_cmd(void *wmi_hdl);

QDF_STATUS wmi_unified_send_regdomain_info_to_fw_cmd(void *wmi_hdl,
				   uint32_t reg_dmn, uint16_t regdmn2G,
				   uint16_t regdmn5G, uint8_t ctl2G,
				   uint8_t ctl5G);

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

QDF_STATUS wmi_unified_save_fw_version_cmd(void *wmi_hdl,
		void *evt_buf);

QDF_STATUS wmi_unified_set_base_macaddr_indicate_cmd(void *wmi_hdl,
					 uint8_t *custom_addr);

QDF_STATUS wmi_unified_log_supported_evt_cmd(void *wmi_hdl,
		uint8_t *event,
		uint32_t len);

QDF_STATUS wmi_unified_enable_specific_fw_logs_cmd(void *wmi_hdl,
		struct wmi_wifi_start_log *start_log);

QDF_STATUS wmi_unified_flush_logs_to_fw_cmd(void *wmi_hdl);

QDF_STATUS wmi_unified_pdev_set_pcl_cmd(void *wmi_hdl,
				struct wmi_pcl_chan_weights *msg);

QDF_STATUS wmi_unified_soc_set_hw_mode_cmd(void *wmi_hdl,
				uint32_t hw_mode_index);

QDF_STATUS wmi_unified_pdev_set_dual_mac_config_cmd(void *wmi_hdl,
		struct wmi_dual_mac_config *msg);

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

#ifdef CONFIG_MCL
QDF_STATUS wmi_unified_send_roam_scan_offload_ap_cmd(void *wmi_hdl,
				   struct ap_profile_params *ap_profile);
#endif

QDF_STATUS wmi_unified_roam_scan_offload_scan_period(void *wmi_hdl,
					     uint32_t scan_period,
					     uint32_t scan_age,
					     uint32_t vdev_id);

QDF_STATUS wmi_unified_roam_scan_offload_chan_list_cmd(void *wmi_hdl,
				   uint8_t chan_count,
				   uint32_t *chan_list,
				   uint8_t list_type, uint32_t vdev_id);

QDF_STATUS wmi_unified_roam_scan_offload_rssi_change_cmd(void *wmi_hdl,
			  uint32_t vdev_id,
			  int32_t rssi_change_thresh,
			  uint32_t bcn_rssi_weight,
			  uint32_t hirssi_delay_btw_scans);

/**
 * wmi_unified_set_per_roam_config() - set PER roam config in FW
 * @wmi_hdl: wmi handle
 * @req_buf: per roam config request buffer
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_per_roam_config(void *wmi_hdl,
		struct wmi_per_roam_config_req *req_buf);

QDF_STATUS wmi_unified_get_buf_extscan_hotlist_cmd(void *wmi_hdl,
				   struct ext_scan_setbssi_hotlist_params *
				   photlist, int *buf_len);

/**
 * wmi_unified_set_active_bpf_mode_cmd() - config active BPF mode in FW
 * @wmi_hdl: the WMI handle
 * @vdev_id: the Id of the vdev to apply the configuration to
 * @ucast_mode: the active BPF mode to configure for unicast packets
 * @mcast_bcast_mode: the active BPF mode to configure for multicast/broadcast
 *	packets
 */
QDF_STATUS wmi_unified_set_active_bpf_mode_cmd(void *wmi_hdl,
				uint8_t vdev_id,
				enum wmi_host_active_bpf_mode ucast_mode,
				enum wmi_host_active_bpf_mode mcast_bcast_mode);

QDF_STATUS wmi_unified_stats_request_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct stats_request_params *param);

QDF_STATUS wmi_unified_pdev_get_tpc_config_cmd_send(void *wmi_hdl,
				uint32_t param);

QDF_STATUS wmi_unified_set_bwf_cmd_send(void *wmi_hdl,
				struct set_bwf_params *param);

QDF_STATUS wmi_send_get_user_position_cmd(void *wmi_hdl, uint32_t value);

QDF_STATUS wmi_send_get_peer_mumimo_tx_count_cmd(void *wmi_hdl, uint32_t value);

QDF_STATUS wmi_send_reset_peer_mumimo_tx_count_cmd(void *wmi_hdl,
				uint32_t value);

QDF_STATUS wmi_send_pdev_caldata_version_check_cmd(void *wmi_hdl,
				uint32_t value);

QDF_STATUS wmi_unified_send_btcoex_wlan_priority_cmd(void *wmi_hdl,
				struct btcoex_cfg_params *param);

QDF_STATUS wmi_unified_send_btcoex_duty_cycle_cmd(void *wmi_hdl,
				struct btcoex_cfg_params *param);

QDF_STATUS wmi_unified_send_coex_ver_cfg_cmd(void *wmi_hdl,
				coex_ver_cfg_t *param);

QDF_STATUS wmi_unified_send_coex_config_cmd(void *wmi_hdl,
					    struct coex_config_params *param);

QDF_STATUS wmi_unified_set_atf_cmd_send(void *wmi_hdl,
				struct set_atf_params *param);

QDF_STATUS wmi_unified_pdev_fips_cmd_send(void *wmi_hdl,
				struct fips_params *param);

QDF_STATUS wmi_unified_wlan_profile_enable_cmd_send(void *wmi_hdl,
				struct wlan_profile_params *param);

QDF_STATUS wmi_unified_wlan_profile_trigger_cmd_send(void *wmi_hdl,
				struct wlan_profile_params *param);

QDF_STATUS wmi_unified_set_chan_cmd_send(void *wmi_hdl,
				struct channel_param *param);

QDF_STATUS wmi_unified_set_ht_ie_cmd_send(void *wmi_hdl,
				struct ht_ie_params *param);

QDF_STATUS wmi_unified_set_vht_ie_cmd_send(void *wmi_hdl,
				struct vht_ie_params *param);

QDF_STATUS wmi_unified_wmm_update_cmd_send(void *wmi_hdl,
				struct wmm_update_params *param);

QDF_STATUS wmi_unified_set_ant_switch_tbl_cmd_send(void *wmi_hdl,
				struct ant_switch_tbl_params *param);

QDF_STATUS wmi_unified_set_ratepwr_table_cmd_send(void *wmi_hdl,
				struct ratepwr_table_params *param);

QDF_STATUS wmi_unified_get_ratepwr_table_cmd_send(void *wmi_hdl);

QDF_STATUS wmi_unified_set_ctl_table_cmd_send(void *wmi_hdl,
				struct ctl_table_params *param);

QDF_STATUS wmi_unified_set_mimogain_table_cmd_send(void *wmi_hdl,
				struct mimogain_table_params *param);

QDF_STATUS wmi_unified_set_ratepwr_chainmsk_cmd_send(void *wmi_hdl,
				struct ratepwr_chainmsk_params *param);

QDF_STATUS wmi_unified_set_macaddr_cmd_send(void *wmi_hdl,
				struct macaddr_params *param);

QDF_STATUS wmi_unified_pdev_scan_start_cmd_send(void *wmi_hdl);

QDF_STATUS wmi_unified_pdev_scan_end_cmd_send(void *wmi_hdl);

QDF_STATUS wmi_unified_set_acparams_cmd_send(void *wmi_hdl,
				struct acparams_params *param);

QDF_STATUS wmi_unified_set_vap_dscp_tid_map_cmd_send(void *wmi_hdl,
				struct vap_dscp_tid_map_params *param);

QDF_STATUS wmi_unified_proxy_ast_reserve_cmd_send(void *wmi_hdl,
				struct proxy_ast_reserve_params *param);

QDF_STATUS wmi_unified_pdev_qvit_cmd_send(void *wmi_hdl,
				struct pdev_qvit_params *param);

QDF_STATUS wmi_unified_mcast_group_update_cmd_send(void *wmi_hdl,
				struct mcast_group_update_params *param);

QDF_STATUS wmi_unified_peer_add_wds_entry_cmd_send(void *wmi_hdl,
				struct peer_add_wds_entry_params *param);

QDF_STATUS wmi_unified_peer_del_wds_entry_cmd_send(void *wmi_hdl,
				struct peer_del_wds_entry_params *param);

/**
 *  wmi_unified_set_bridge_mac_addr_cmd_send() - WMI set bridge mac addr cmd function
 *  @param wmi_hdl      : handle to WMI.
 *  @param param        : pointer to hold bridge mac addr param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_bridge_mac_addr_cmd_send(void *wmi_hdl,
				struct set_bridge_mac_addr_params *param);


QDF_STATUS wmi_unified_peer_update_wds_entry_cmd_send(void *wmi_hdl,
				struct peer_update_wds_entry_params *param);

QDF_STATUS wmi_unified_phyerr_enable_cmd_send(void *wmi_hdl);

QDF_STATUS wmi_unified_phyerr_enable_cmd_send(void *wmi_hdl);

QDF_STATUS wmi_unified_phyerr_disable_cmd_send(void *wmi_hdl);

QDF_STATUS wmi_unified_smart_ant_enable_cmd_send(void *wmi_hdl,
				struct smart_ant_enable_params *param);

QDF_STATUS wmi_unified_smart_ant_set_rx_ant_cmd_send(void *wmi_hdl,
				struct smart_ant_rx_ant_params *param);

QDF_STATUS wmi_unified_smart_ant_set_tx_ant_cmd_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct smart_ant_tx_ant_params *param);

QDF_STATUS wmi_unified_smart_ant_set_training_info_cmd_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct smart_ant_training_info_params *param);

QDF_STATUS wmi_unified_smart_ant_node_config_cmd_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct smart_ant_node_config_params *param);

QDF_STATUS wmi_unified_smart_ant_enable_tx_feedback_cmd_send(void *wmi_hdl,
			struct smart_ant_enable_tx_feedback_params *param);

QDF_STATUS wmi_unified_vdev_spectral_configure_cmd_send(void *wmi_hdl,
				struct vdev_spectral_configure_params *param);

QDF_STATUS wmi_unified_vdev_spectral_enable_cmd_send(void *wmi_hdl,
				struct vdev_spectral_enable_params *param);

QDF_STATUS wmi_unified_bss_chan_info_request_cmd_send(void *wmi_hdl,
				struct bss_chan_info_request_params *param);

QDF_STATUS wmi_unified_thermal_mitigation_param_cmd_send(void *wmi_hdl,
				struct thermal_mitigation_params *param);

QDF_STATUS wmi_unified_vdev_set_neighbour_rx_cmd_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct set_neighbour_rx_params *param);

QDF_STATUS wmi_unified_vdev_set_fwtest_param_cmd_send(void *wmi_hdl,
				struct set_fwtest_params *param);

QDF_STATUS wmi_unified_vdev_config_ratemask_cmd_send(void *wmi_hdl,
				struct config_ratemask_params *param);

/**
 *  wmi_unified_vdev_set_custom_aggr_size_cmd_send() - WMI set custom aggr
 *						       size command
 *  @param wmi_hdl      : handle to WMI.
 *  @param param        : pointer to hold custom aggr size param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_set_custom_aggr_size_cmd_send(void *wmi_hdl,
				struct set_custom_aggr_size_params *param);

/**
 *  wmi_unified_vdev_set_qdepth_thresh_cmd_send() - WMI set qdepth threshold
 *  @param wmi_hdl      : handle to WMI.
 *  @param param        : pointer to hold set qdepth thresh param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_set_qdepth_thresh_cmd_send(void *wmi_hdl,
				struct set_qdepth_thresh_params *param);

QDF_STATUS wmi_unified_pdev_set_regdomain_cmd_send(void *wmi_hdl,
				struct pdev_set_regdomain_params *param);

QDF_STATUS wmi_unified_set_quiet_mode_cmd_send(void *wmi_hdl,
				struct set_quiet_mode_params *param);

QDF_STATUS wmi_unified_set_beacon_filter_cmd_send(void *wmi_hdl,
				struct set_beacon_filter_params *param);

QDF_STATUS wmi_unified_remove_beacon_filter_cmd_send(void *wmi_hdl,
				struct remove_beacon_filter_params *param);

QDF_STATUS wmi_unified_addba_clearresponse_cmd_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct addba_clearresponse_params *param);

QDF_STATUS wmi_unified_addba_send_cmd_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct addba_send_params *param);

QDF_STATUS wmi_unified_delba_send_cmd_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct delba_send_params *param);

QDF_STATUS wmi_unified_addba_setresponse_cmd_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct addba_setresponse_params *param);

QDF_STATUS wmi_unified_singleamsdu_cmd_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct singleamsdu_params *param);

QDF_STATUS wmi_unified_set_qboost_param_cmd_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct set_qboost_params *param);

QDF_STATUS wmi_unified_mu_scan_cmd_send(void *wmi_hdl,
				struct mu_scan_params *param);

QDF_STATUS wmi_unified_lteu_config_cmd_send(void *wmi_hdl,
				struct lteu_config_params *param);

QDF_STATUS wmi_unified_set_psmode_cmd_send(void *wmi_hdl,
				struct set_ps_mode_params *param);

QDF_STATUS wmi_unified_init_cmd_send(void *wmi_hdl,
				struct wmi_init_cmd_param *param);

bool wmi_service_enabled(void *wmi_hdl, uint32_t service_id);

/**
 * wmi_save_service_bitmap() - save service bitmap
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS failure code
 */
QDF_STATUS wmi_save_service_bitmap(void *wmi_hdl, void *evt_buf,
				   void *bitmap_buf);

/**
 * wmi_save_ext_service_bitmap() - save extended service bitmap
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS failure code
 */
QDF_STATUS wmi_save_ext_service_bitmap(void *wmi_hdl, void *evt_buf,
				   void *bitmap_buf);

QDF_STATUS wmi_save_fw_version(void *wmi_hdl, void *evt_buf);

QDF_STATUS wmi_get_target_cap_from_service_ready(void *wmi_hdl,
				void *evt_buf,
				struct wlan_psoc_target_capability_info *ev);

QDF_STATUS wmi_extract_hal_reg_cap(void *wmi_hdl, void *evt_buf,
			struct wlan_psoc_hal_reg_capability *hal_reg_cap);

host_mem_req *wmi_extract_host_mem_req_from_service_ready(void *wmi_hdl,
				void *evt_buf, uint8_t *num_entries);

uint32_t wmi_ready_extract_init_status(void *wmi_hdl, void *ev);

QDF_STATUS wmi_ready_extract_mac_addr(void *wmi_hdl,
				void *ev, uint8_t *macaddr);

wmi_host_mac_addr *wmi_ready_extract_mac_addr_list(void *wmi_hdl, void *ev,
					      uint8_t *num_mac_addr);

/**
 * wmi_extract_ready_params() - Extract data from ready event apart from
 *                     status, macaddr and version.
 * @wmi_handle: Pointer to WMI handle.
 * @evt_buf: Pointer to Ready event buffer.
 * @ev_param: Pointer to host defined struct to copy the data from event.
 *
 * Return: QDF_STATUS_SUCCESS on success.
 */
QDF_STATUS wmi_extract_ready_event_params(void *wmi_hdl,
		void *evt_buf, struct wmi_host_ready_ev_param *ev_param);

QDF_STATUS wmi_extract_fw_version(void *wmi_hdl,
				void *ev, struct wmi_host_fw_ver *fw_ver);

QDF_STATUS wmi_extract_fw_abi_version(void *wmi_hdl,
				void *ev, struct wmi_host_fw_abi_ver *fw_ver);

QDF_STATUS wmi_check_and_update_fw_version(void *wmi_hdl, void *ev);

uint8_t *wmi_extract_dbglog_data_len(void *wmi_hdl,
				void *evt_b, uint32_t *len);

QDF_STATUS wmi_send_ext_resource_config(void *wmi_hdl,
				wmi_host_ext_resource_config *ext_cfg);

QDF_STATUS wmi_unified_nf_dbr_dbm_info_get_cmd_send(void *wmi_hdl,
						    uint8_t mac_id);

QDF_STATUS wmi_unified_packet_power_info_get_cmd_send(void *wmi_hdl,
				struct packet_power_info_params *param);

QDF_STATUS wmi_unified_gpio_config_cmd_send(void *wmi_hdl,
				struct gpio_config_params *param);

QDF_STATUS wmi_unified_gpio_output_cmd_send(void *wmi_hdl,
				struct gpio_output_params *param);

QDF_STATUS wmi_unified_rtt_meas_req_test_cmd_send(void *wmi_hdl,
				struct rtt_meas_req_test_params *param);

QDF_STATUS wmi_unified_rtt_meas_req_cmd_send(void *wmi_hdl,
				struct rtt_meas_req_params *param);

QDF_STATUS wmi_unified_rtt_keepalive_req_cmd_send(void *wmi_hdl,
				struct rtt_keepalive_req_params *param);

QDF_STATUS wmi_unified_lci_set_cmd_send(void *wmi_hdl,
				struct lci_set_params *param);

QDF_STATUS wmi_unified_lcr_set_cmd_send(void *wmi_hdl,
				struct lcr_set_params *param);

QDF_STATUS wmi_unified_send_periodic_chan_stats_config_cmd(void *wmi_hdl,
			struct periodic_chan_stats_params *param);

QDF_STATUS
wmi_send_atf_peer_request_cmd(void *wmi_hdl,
			struct atf_peer_request_params *param);

QDF_STATUS
wmi_send_set_atf_grouping_cmd(void *wmi_hdl,
			struct atf_grouping_params *param);
/* Extract APIs */

QDF_STATUS wmi_extract_wds_addr_event(void *wmi_hdl,
		void *evt_buf, uint16_t len, wds_addr_event_t *wds_ev);

QDF_STATUS wmi_extract_dcs_interference_type(void *wmi_hdl,
		void *evt_buf, struct wmi_host_dcs_interference_param *param);

QDF_STATUS wmi_extract_dcs_cw_int(void *wmi_hdl, void *evt_buf,
		wmi_host_ath_dcs_cw_int *cw_int);

QDF_STATUS wmi_extract_dcs_im_tgt_stats(void *wmi_hdl, void *evt_buf,
		wmi_host_dcs_im_tgt_stats_t *wlan_stat);

QDF_STATUS wmi_extract_fips_event_data(void *wmi_hdl, void *evt_buf,
		struct wmi_host_fips_event_param *param);

QDF_STATUS wmi_extract_vdev_start_resp(void *wmi_hdl, void *evt_buf,
		wmi_host_vdev_start_resp *vdev_rsp);

/**
 * wmi_extract_vdev_delete_resp - api to extract vdev delete
 * response event params
 * @wmi_handle: wma handle
 * @evt_buf: pointer to event buffer
 * @delele_rsp: pointer to hold delete response from firmware
 *
 * Return: QDF_STATUS_SUCCESS for successful event parse
 *         else QDF_STATUS_E_INVAL or QDF_STATUS_E_FAILURE
 */
QDF_STATUS wmi_extract_vdev_delete_resp(void *wmi_hdl, void *evt_buf,
		struct wmi_host_vdev_delete_resp *delele_rsp);

QDF_STATUS wmi_extract_tbttoffset_update_params(void *wmi_hdl, void *evt_buf,
		uint8_t idx, struct tbttoffset_params *tbtt_param);

QDF_STATUS wmi_extract_ext_tbttoffset_update_params(void *wmi_hdl,
		void *evt_buf, uint8_t idx,
		struct tbttoffset_params *tbtt_param);

QDF_STATUS wmi_extract_tbttoffset_num_vdevs(void *wmi_hdl, void *evt_buf,
					    uint32_t *num_vdevs);

QDF_STATUS wmi_extract_ext_tbttoffset_num_vdevs(void *wmi_hdl, void *evt_buf,
						uint32_t *num_vdevs);

QDF_STATUS wmi_extract_mgmt_rx_params(void *wmi_hdl, void *evt_buf,
		struct mgmt_rx_event_params *hdr, uint8_t **bufp);

QDF_STATUS wmi_extract_vdev_stopped_param(void *wmi_hdl, void *evt_buf,
		uint32_t *vdev_id);

QDF_STATUS wmi_extract_vdev_roam_param(void *wmi_hdl, void *evt_buf,
		wmi_host_roam_event *ev);

QDF_STATUS wmi_extract_vdev_scan_ev_param(void *wmi_hdl, void *evt_buf,
		struct scan_event *param);

#ifdef CONVERGED_TDLS_ENABLE
/**
 * wmi_extract_vdev_tdls_ev_param - extract vdev tdls param from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold vdev tdls param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_vdev_tdls_ev_param(void *wmi_hdl, void *evt_buf,
					  struct tdls_event_info *param);
#endif

QDF_STATUS wmi_extract_mu_ev_param(void *wmi_hdl, void *evt_buf,
		wmi_host_mu_report_event *param);

QDF_STATUS wmi_extract_mu_db_entry(void *wmi_hdl, void *evt_buf,
		uint8_t idx, wmi_host_mu_db_entry *param);

QDF_STATUS wmi_extract_mumimo_tx_count_ev_param(void *wmi_hdl, void *evt_buf,
		wmi_host_peer_txmu_cnt_event *param);

QDF_STATUS wmi_extract_peer_gid_userpos_list_ev_param(void *wmi_hdl,
		void *evt_buf, wmi_host_peer_gid_userpos_list_event *param);

QDF_STATUS wmi_extract_pdev_caldata_version_check_ev_param(void *wmi_hdl,
		void *evt_buf, wmi_host_pdev_check_cal_version_event *param);

QDF_STATUS wmi_extract_pdev_tpc_config_ev_param(void *wmi_hdl, void *evt_buf,
		wmi_host_pdev_tpc_config_event *param);

QDF_STATUS wmi_extract_gpio_input_ev_param(void *wmi_hdl,
		void *evt_buf, uint32_t *gpio_num);

QDF_STATUS wmi_extract_pdev_reserve_ast_ev_param(void *wmi_hdl,
		void *evt_buf, struct wmi_host_proxy_ast_reserve_param *param);

QDF_STATUS wmi_extract_nfcal_power_ev_param(void *wmi_hdl, void *evt_buf,
		wmi_host_pdev_nfcal_power_all_channels_event *param);

QDF_STATUS wmi_extract_pdev_tpc_ev_param(void *wmi_hdl, void *evt_buf,
		wmi_host_pdev_tpc_event *param);

QDF_STATUS wmi_extract_pdev_generic_buffer_ev_param(void *wmi_hdl,
		void *evt_buf,
		wmi_host_pdev_generic_buffer_event *param);

QDF_STATUS wmi_extract_mgmt_tx_compl_param(void *wmi_hdl, void *evt_buf,
		wmi_host_mgmt_tx_compl_event *param);

QDF_STATUS wmi_extract_offchan_data_tx_compl_param(void *wmi_hdl, void *evt_buf,
		struct wmi_host_offchan_data_tx_compl_event *param);

QDF_STATUS wmi_extract_pdev_csa_switch_count_status(void *wmi_hdl,
		void *evt_buf,
		struct pdev_csa_switch_count_status *param);

QDF_STATUS wmi_extract_swba_num_vdevs(void *wmi_hdl, void *evt_buf,
		uint32_t *num_vdevs);

QDF_STATUS wmi_extract_swba_tim_info(void *wmi_hdl, void *evt_buf,
		 uint32_t idx, wmi_host_tim_info *tim_info);

QDF_STATUS wmi_extract_swba_noa_info(void *wmi_hdl, void *evt_buf,
			uint32_t idx, wmi_host_p2p_noa_info *p2p_desc);

#ifdef CONVERGED_P2P_ENABLE
QDF_STATUS wmi_extract_p2p_lo_stop_ev_param(void *wmi_hdl,
		void *evt_buf, struct p2p_lo_event *param);

QDF_STATUS wmi_extract_p2p_noa_ev_param(void *wmi_hdl,
		void *evt_buf, struct p2p_noa_info *param);
#endif

QDF_STATUS wmi_extract_peer_sta_ps_statechange_ev(void *wmi_hdl,
		void *evt_buf, wmi_host_peer_sta_ps_statechange_event *ev);

QDF_STATUS wmi_extract_peer_sta_kickout_ev(void *wmi_hdl, void *evt_buf,
		wmi_host_peer_sta_kickout_event *ev);

QDF_STATUS wmi_extract_peer_ratecode_list_ev(void *wmi_hdl, void *evt_buf,
		uint8_t *peer_mac, wmi_sa_rate_cap *rate_cap);

QDF_STATUS wmi_extract_bcnflt_stats(void *wmi_hdl, void *evt_buf,
		 uint32_t index, wmi_host_bcnflt_stats *bcnflt_stats);

QDF_STATUS wmi_extract_rtt_hdr(void *wmi_hdl, void *evt_buf,
		wmi_host_rtt_event_hdr *ev);

QDF_STATUS wmi_extract_rtt_ev(void *wmi_hdl, void *evt_buf,
		wmi_host_rtt_meas_event *ev, uint8_t *hdump,
		uint16_t hdump_len);

QDF_STATUS wmi_extract_rtt_error_report_ev(void *wmi_hdl, void *evt_buf,
		wmi_host_rtt_error_report_event *ev);

QDF_STATUS wmi_extract_chan_stats(void *wmi_hdl, void *evt_buf,
		uint32_t index, wmi_host_chan_stats *chan_stats);

QDF_STATUS wmi_extract_thermal_stats(void *wmi_hdl, void *evt_buf,
		uint32_t *temp, uint32_t *level, uint32_t *pdev_id);

QDF_STATUS wmi_extract_thermal_level_stats(void *wmi_hdl, void *evt_buf,
		uint8_t idx, uint32_t *levelcount, uint32_t *dccount);

QDF_STATUS wmi_extract_comb_phyerr(void *wmi_hdl, void *evt_buf,
			uint16_t datalen, uint16_t *buf_offset,
			wmi_host_phyerr_t *phyerr);

QDF_STATUS wmi_extract_single_phyerr(void *wmi_hdl, void *evt_buf,
			uint16_t datalen, uint16_t *buf_offset,
			wmi_host_phyerr_t *phyerr);

QDF_STATUS wmi_extract_composite_phyerr(void *wmi_hdl, void *evt_buf,
			uint16_t datalen, wmi_host_phyerr_t *phyerr);

QDF_STATUS wmi_extract_profile_ctx(void *wmi_hdl, void *evt_buf,
			wmi_host_wlan_profile_ctx_t *profile_ctx);

QDF_STATUS wmi_extract_profile_data(void *wmi_hdl, void *evt_buf, uint8_t idx,
			wmi_host_wlan_profile_t *profile_data);

QDF_STATUS wmi_extract_chan_info_event(void *wmi_hdl, void *evt_buf,
			wmi_host_chan_info_event *chan_info);

QDF_STATUS wmi_extract_channel_hopping_event(void *wmi_hdl, void *evt_buf,
			wmi_host_pdev_channel_hopping_event *ch_hopping);

QDF_STATUS wmi_extract_stats_param(void *wmi_hdl, void *evt_buf,
					   wmi_host_stats_event *stats_param);

QDF_STATUS wmi_extract_pdev_stats(void *wmi_hdl, void *evt_buf,
					 uint32_t index,
					 wmi_host_pdev_stats *pdev_stats);

QDF_STATUS wmi_extract_unit_test(void *wmi_hdl, void *evt_buf,
			wmi_unit_test_event *unit_test, uint32_t maxspace);

QDF_STATUS wmi_extract_pdev_ext_stats(void *wmi_hdl, void *evt_buf,
			uint32_t index,
			wmi_host_pdev_ext_stats *pdev_ext_stats);

QDF_STATUS wmi_extract_peer_extd_stats(void *wmi_hdl, void *evt_buf,
			uint32_t index,
			wmi_host_peer_extd_stats *peer_extd_stats);

QDF_STATUS wmi_extract_bss_chan_info_event(void *wmi_hdl, void *evt_buf,
			wmi_host_pdev_bss_chan_info_event *bss_chan_info);

QDF_STATUS wmi_extract_inst_rssi_stats_event(void *wmi_hdl, void *evt_buf,
			wmi_host_inst_stats_resp *inst_rssi_resp);

QDF_STATUS wmi_extract_peer_stats(void *wmi_hdl, void *evt_buf,
		uint32_t index, wmi_host_peer_stats *peer_stats);

QDF_STATUS wmi_extract_tx_data_traffic_ctrl_ev(void *wmi_hdl, void *evt_buf,
		wmi_host_tx_data_traffic_ctrl_event *ev);

QDF_STATUS wmi_extract_atf_peer_stats_ev(void *wmi_hdl, void *evt_buf,
		wmi_host_atf_peer_stats_event *ev);

QDF_STATUS wmi_extract_atf_token_info_ev(void *wmi_hdl, void *evt_buf,
		uint8_t idx, wmi_host_atf_peer_stats_info *atf_token_info);

QDF_STATUS wmi_extract_vdev_stats(void *wmi_hdl, void *evt_buf,
		uint32_t index, wmi_host_vdev_stats *vdev_stats);

QDF_STATUS wmi_extract_vdev_extd_stats(void *wmi_hdl, void *evt_buf,
		uint32_t index, wmi_host_vdev_extd_stats *vdev_extd_stats);

QDF_STATUS wmi_extract_bcn_stats(void *wmi_hdl, void *evt_buf,
		uint32_t index, wmi_host_bcn_stats *vdev_bcn_stats);

/**
 * wmi_extract_vdev_nac_rssi_stats() - extract NAC_RSSI stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param vdev_extd_stats: Pointer to hold nac rssi stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_vdev_nac_rssi_stats(void *wmi_hdl, void *evt_buf,
		struct wmi_host_vdev_nac_rssi_event *vdev_nac_rssi_stats);

QDF_STATUS wmi_unified_send_power_dbg_cmd(void *wmi_hdl,
				struct wmi_power_dbg_params *param);

QDF_STATUS wmi_unified_send_multiple_vdev_restart_req_cmd(void *wmi_hdl,
				struct multiple_vdev_restart_params *param);

/**
 * wmi_unified_send_sar_limit_cmd() - send sar limit cmd to fw
 * @wmi_hdl: wmi handle
 * @params: sar limit command params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_send_sar_limit_cmd(void *wmi_hdl,
					  struct sar_limit_cmd_params *params);

/**
 * wmi_unified_get_sar_limit_cmd() - request current SAR limits from FW
 * @wmi_hdl: wmi handle
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_unified_get_sar_limit_cmd(void *wmi_hdl);

/**
 * wmi_unified_extract_sar_limit_event() - extract SAR limits from FW event
 * @wmi_hdl: wmi handle
 * @evt_buf: event buffer received from firmware
 * @event: SAR limit event which is to be populated by data extracted from
 *         the @evt_buf buffer
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_unified_extract_sar_limit_event(void *wmi_hdl,
					       uint8_t *evt_buf,
					       struct sar_limit_event *event);

QDF_STATUS wmi_unified_send_adapt_dwelltime_params_cmd(void *wmi_hdl,
				   struct wmi_adaptive_dwelltime_params *
				   wmi_param);
QDF_STATUS wmi_unified_fw_test_cmd(void *wmi_hdl,
				   struct set_fwtest_params *wmi_fwtest);

QDF_STATUS wmi_unified_peer_rx_reorder_queue_setup_send(void *wmi_hdl,
					struct rx_reorder_queue_setup_params *param);
QDF_STATUS wmi_unified_peer_rx_reorder_queue_remove_send(void *wmi_hdl,
					struct rx_reorder_queue_remove_params *param);

QDF_STATUS wmi_extract_service_ready_ext(void *wmi_hdl, uint8_t *evt_buf,
		struct wlan_psoc_host_service_ext_param *param);
QDF_STATUS wmi_extract_hw_mode_cap_service_ready_ext(
			void *wmi_hdl,
			uint8_t *evt_buf, uint8_t hw_mode_idx,
			struct wlan_psoc_host_hw_mode_caps *param);
QDF_STATUS wmi_extract_mac_phy_cap_service_ready_ext(
			void *wmi_hdl,
			uint8_t *evt_buf,
			uint8_t hw_mode_id,
			uint8_t phy_id,
			struct wlan_psoc_host_mac_phy_caps *param);
QDF_STATUS wmi_extract_reg_cap_service_ready_ext(
			void *wmi_hdl,
			uint8_t *evt_buf, uint8_t phy_idx,
			struct wlan_psoc_host_hal_reg_capabilities_ext *param);

/**
 * wmi_extract_dbr_ring_cap_service_ready_ext: Extract direct buffer rx
 *                                             capability received through
 *                                             extended service ready event
 * @wmi_hdl: WMI handle
 * @evt_buf: Event buffer
 * @idx: Index of the module for which capability is received
 * @param: Pointer to direct buffer rx ring cap struct
 *
 * Return: QDF status of operation
 */
QDF_STATUS wmi_extract_dbr_ring_cap_service_ready_ext(
			void *wmi_hdl,
			uint8_t *evt_buf, uint8_t idx,
			struct wlan_psoc_host_dbr_ring_caps *param);

/**
 * wmi_extract_dbr_buf_release_fixed : Extract direct buffer rx fixed param
 *                                     from buffer release event
 * @wmi_hdl: WMI handle
 * @evt_buf: Event buffer
 * @param: Pointer to direct buffer rx response struct
 *
 * Return: QDF status of operation
 */
QDF_STATUS wmi_extract_dbr_buf_release_fixed(
			void *wmi_hdl,
			uint8_t *evt_buf,
			struct direct_buf_rx_rsp *param);

/**
 * wmi_extract_dbr_buf_release_entry: Extract direct buffer rx buffer tlv
 *
 * @wmi_hdl: WMI handle
 * @evt_buf: Event buffer
 * @idx: Index of the module for which capability is received
 * @param: Pointer to direct buffer rx entry
 *
 * Return: QDF status of operation
 */
QDF_STATUS wmi_extract_dbr_buf_release_entry(
			void *wmi_hdl,
			uint8_t *evt_buf, uint8_t idx,
			struct direct_buf_rx_entry *param);
QDF_STATUS wmi_extract_pdev_utf_event(void *wmi_hdl,
				      uint8_t *evt_buf,
				      struct wmi_host_pdev_utf_event *param);

QDF_STATUS wmi_extract_pdev_qvit_event(void *wmi_hdl,
				      uint8_t *evt_buf,
				      struct wmi_host_pdev_qvit_event *param);

QDF_STATUS wmi_extract_peer_delete_response_event(void *wmi_hdl,
		uint8_t *evt_buf,
		struct wmi_host_peer_delete_response_event *param);

QDF_STATUS wmi_extract_chainmask_tables(void *wmi_hdl, uint8_t *evt_buf,
		struct wlan_psoc_host_chainmask_table *chainmask_table);
/**
 * wmi_unified_dfs_phyerr_offload_en_cmd() - enable dfs phyerr offload
 * @wmi_handle: wmi handle
 * @pdev_id: pdev id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wmi_unified_dfs_phyerr_offload_en_cmd(void *wmi_hdl,
		uint32_t pdev_id);

/**
 * wmi_unified_dfs_phyerr_offload_dis_cmd() - disable dfs phyerr offload
 * @wmi_handle: wmi handle
 * @pdev_id: pdev id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wmi_unified_dfs_phyerr_offload_dis_cmd(void *wmi_hdl,
		uint32_t pdev_id);

QDF_STATUS wmi_unified_set_country_cmd_send(void *wmi_hdl,
				struct set_country *param);
/*
 * wmi_unified_set_del_pmkid_cache() - set delete PMKID
 * @wmi_hdl: wma handle
 * @pmksa: pointer to pmk cache entry
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_del_pmkid_cache(void *wmi_hdl,
					struct wmi_unified_pmk_cache *pmksa);

#if defined(WLAN_FEATURE_FILS_SK)
/*
 * wmi_unified_roam_send_hlp_cmd() -send HLP command info
 * @wmi_hdl: wma handle
 * @req_buf: Pointer to HLP params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_roam_send_hlp_cmd(void *wmi_hdl,
					struct hlp_params *req_buf);
#endif

/**
 * wmi_unified_send_request_get_rcpi_cmd() - command to request rcpi value
 * @wmi_hdl: wma handle
 * @get_rcpi_param: rcpi params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_send_request_get_rcpi_cmd(void *wmi_hdl,
					struct rcpi_req *get_rcpi_param);

/**
 * wmi_extract_rcpi_response_event - api to extract RCPI event params
 * @wmi_handle: wma handle
 * @evt_buf: pointer to event buffer
 * @res: pointer to hold rcpi response from firmware
 *
 * Return: QDF_STATUS_SUCCESS for successful event parse
 *         else QDF_STATUS_E_INVAL or QDF_STATUS_E_FAILURE
 */
QDF_STATUS wmi_extract_rcpi_response_event(void *wmi_hdl, void *evt_buf,
					   struct rcpi_res *res);

#ifdef WMI_INTERFACE_EVENT_LOGGING
void wmi_print_cmd_log(wmi_unified_t wmi, uint32_t count,
		       qdf_abstract_print *print, void *print_priv);

void wmi_print_cmd_tx_cmp_log(wmi_unified_t wmi, uint32_t count,
			      qdf_abstract_print *print, void *print_priv);

void wmi_print_mgmt_cmd_log(wmi_unified_t wmi, uint32_t count,
			    qdf_abstract_print *print, void *print_priv);

void wmi_print_mgmt_cmd_tx_cmp_log(wmi_unified_t wmi, uint32_t count,
				   qdf_abstract_print *print, void *print_priv);

void wmi_print_event_log(wmi_unified_t wmi, uint32_t count,
			 qdf_abstract_print *print, void *print_priv);

void wmi_print_rx_event_log(wmi_unified_t wmi, uint32_t count,
			    qdf_abstract_print *print, void *print_priv);

void wmi_print_mgmt_event_log(wmi_unified_t wmi, uint32_t count,
			      qdf_abstract_print *print, void *print_priv);

#endif /* WMI_INTERFACE_EVENT_LOGGING */

QDF_STATUS wmi_unified_send_dbs_scan_sel_params_cmd(void *wmi_hdl,
				struct wmi_dbs_scan_sel_params *wmi_param);

QDF_STATUS wmi_unified_send_limit_off_chan_cmd(void *wmi_hdl,
		struct wmi_limit_off_chan_param *wmi_param);
QDF_STATUS wmi_unified_set_arp_stats_req(void *wmi_hdl,
					 struct set_arp_stats *req_buf);
QDF_STATUS wmi_unified_get_arp_stats_req(void *wmi_hdl,
					 struct get_arp_stats *req_buf);

/**
 * wmi_send_bcn_offload_control_cmd - send beacon ofload control cmd to fw
 * @wmi_hdl: wmi handle
 * @bcn_ctrl_param: pointer to bcn_offload_control param
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_send_bcn_offload_control_cmd(void *wmi_hdl,
			struct bcn_offload_control *bcn_ctrl_param);
/**
 * wmi_unified_send_wds_entry_list_cmd() - WMI function to get list of
 *  wds entries from FW
 * @wmi_hdl: wmi handle
 *
 * Send WMI_PDEV_WDS_ENTRY_LIST_CMDID parameters to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */

QDF_STATUS wmi_unified_send_dump_wds_table_cmd(void *wmi_hdl);

/**
 * wmi_extract_wds_entry - api to extract wds entry
 * @wmi_hdl: wmi handle
 * @evt_buf: pointer to event buffer
 * @wds_entry: wds entry
 * @idx: index to point wds entry in event buffer
 *
 * Return: QDF_STATUS_SUCCESS for successful event parse
 *         else QDF_STATUS_E_INVAL or QDF_STATUS_E_FAILURE
 */

QDF_STATUS wmi_extract_wds_entry(void *wmi_hdl, uint8_t *evt_buf,
			struct wdsentry *wds_entry, u_int32_t idx);

#ifdef WLAN_FEATURE_NAN_CONVERGENCE
/**
 * wmi_unified_ndp_initiator_req_cmd_send - api to send initiator request to FW
 * @wmi_hdl: wma handle
 * @req: pointer to request buffer
 *
 * Return: status of operation
 */
QDF_STATUS wmi_unified_ndp_initiator_req_cmd_send(void *wmi_hdl,
				struct nan_datapath_initiator_req *req);

/**
 * wmi_unified_ndp_responder_req_cmd_send - api to send responder request to FW
 * @wmi_hdl: wma handle
 * @req: pointer to request buffer
 *
 * Return: status of operation
 */
QDF_STATUS wmi_unified_ndp_responder_req_cmd_send(void *wmi_hdl,
				struct nan_datapath_responder_req *req);

/**
 * wmi_unified_ndp_end_req_cmd_send - api to send end request to FW
 * @wmi_hdl: wma handle
 * @req: pointer to request buffer
 *
 * Return: status of operation
 */
QDF_STATUS wmi_unified_ndp_end_req_cmd_send(void *wmi_hdl,
				struct nan_datapath_end_req *req);

/**
 * wmi_extract_ndp_initiator_rsp - api to extract initiator rsp from even buffer
 * @wmi_hdl: wma handle
 * @data: event buffer
 * @rsp: buffer to populate
 *
 * Return: status of operation
 */
QDF_STATUS wmi_extract_ndp_initiator_rsp(wmi_unified_t wmi_handle,
			uint8_t *data, struct nan_datapath_initiator_rsp **rsp);

/**
 * wmi_extract_ndp_ind - api to extract ndp indication struct from even buffer
 * @wmi_hdl: wma handle
 * @data: event buffer
 * @ind: buffer to populate
 *
 * Return: status of operation
 */
QDF_STATUS wmi_extract_ndp_ind(wmi_unified_t wmi_handle, uint8_t *data,
			struct nan_datapath_indication_event **ind);

/**
 * wmi_extract_ndp_confirm - api to extract ndp confim struct from even buffer
 * @wmi_hdl: wma handle
 * @data: event buffer
 * @ev: buffer to populate
 *
 * Return: status of operation
 */
QDF_STATUS wmi_extract_ndp_confirm(wmi_unified_t wmi_handle, uint8_t *data,
			struct nan_datapath_confirm_event **ev);

/**
 * wmi_extract_ndp_responder_rsp - api to extract responder rsp from even buffer
 * @wmi_hdl: wma handle
 * @data: event buffer
 * @rsp: buffer to populate
 *
 * Return: status of operation
 */
QDF_STATUS wmi_extract_ndp_responder_rsp(wmi_unified_t wmi_handle,
			uint8_t *data, struct nan_datapath_responder_rsp **rsp);

/**
 * wmi_extract_ndp_end_rsp - api to extract ndp end rsp from even buffer
 * @wmi_hdl: wma handle
 * @data: event buffer
 * @rsp: buffer to populate
 *
 * Return: status of operation
 */
QDF_STATUS wmi_extract_ndp_end_rsp(wmi_unified_t wmi_handle, uint8_t *data,
			struct nan_datapath_end_rsp_event **rsp);

/**
 * wmi_extract_ndp_end_ind - api to extract ndp end indication from even buffer
 * @wmi_hdl: wma handle
 * @data: event buffer
 * @ind: buffer to populate
 *
 * Return: status of operation
 */
QDF_STATUS wmi_extract_ndp_end_ind(wmi_unified_t wmi_handle, uint8_t *data,
			struct nan_datapath_end_indication_event **ind);

#endif

/**
 * wmi_unified_send_btm_config() - Send BTM config to fw
 * @wmi_hdl:  wmi handle
 * @params: pointer to wmi_btm_config
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wmi_unified_send_btm_config(void *wmi_hdl,
				       struct wmi_btm_config *params);

/**
 * wmi_unified_send_obss_detection_cfg_cmd() - WMI function to send obss
 *  detection configuration to FW.
 * @wmi_hdl: wmi handle
 * @cfg: obss detection configuration
 *
 * Send WMI_SAP_OBSS_DETECTION_CFG_CMDID parameters to fw.
 *
 * Return: QDF_STATUS
 */

QDF_STATUS wmi_unified_send_obss_detection_cfg_cmd(void *wmi_hdl,
			struct wmi_obss_detection_cfg_param *cfg);

/**
 * wmi_unified_extract_obss_detection_info() - WMI function to extract obss
 *  detection info from FW.
 * @wmi_hdl: wmi handle
 * @data: event data from firmware
 * @info: Pointer to hold obss detection info
 *
 * This function is used to extract obss info from firmware.
 *
 * Return: QDF_STATUS
 */

QDF_STATUS wmi_unified_extract_obss_detection_info(void *wmi_hdl,
						   uint8_t *data,
						   struct wmi_obss_detect_info
						   *info);
/**
 * wmi_unified_send_bss_color_change_enable_cmd() - WMI function to send bss
 *  color change enable to FW.
 * @wmi_hdl: wmi handle
 * @vdev_id: vdev ID
 * @enable: enable or disable color change handeling within firmware
 *
 * Send WMI_BSS_COLOR_CHANGE_ENABLE_CMDID parameters to fw,
 * thereby firmware updates bss color when AP announces bss color change.
 *
 * Return: QDF_STATUS
 */

QDF_STATUS wmi_unified_send_bss_color_change_enable_cmd(void *wmi_hdl,
							uint32_t vdev_id,
							bool enable);

/**
 * wmi_unified_send_obss_color_collision_cfg_cmd() - WMI function to send bss
 *  color collision detection configuration to FW.
 * @wmi_hdl: wmi handle
 * @cfg: obss color collision detection configuration
 *
 * Send WMI_OBSS_COLOR_COLLISION_DET_CONFIG_CMDID parameters to fw.
 *
 * Return: QDF_STATUS
 */

QDF_STATUS wmi_unified_send_obss_color_collision_cfg_cmd(void *wmi_hdl,
		struct wmi_obss_color_collision_cfg_param *cfg);

/**
 * wmi_unified_extract_obss_color_collision_info() - WMI function to extract
 *  obss color collision info from FW.
 * @wmi_hdl: wmi handle
 * @data: event data from firmware
 * @info: Pointer to hold bss color collision info
 *
 * This function is used to extract bss collision info from firmware.
 *
 * Return: QDF_STATUS
 */

QDF_STATUS wmi_unified_extract_obss_color_collision_info(void *wmi_hdl,
		uint8_t *data, struct wmi_obss_color_collision_info *info);

#ifdef WLAN_SUPPORT_GREEN_AP
QDF_STATUS wmi_extract_green_ap_egap_status_info(
		void *wmi_hdl, uint8_t *evt_buf,
		struct wlan_green_ap_egap_status_info *egap_status_info_params);
#endif

#ifdef WLAN_SUPPORT_FILS
/**
 * wmi_unified_fils_vdev_config_send_cmd() - send FILS config cmd to fw
 * @wmi_hdl: wmi handle
 * @param:   fils config params
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS
wmi_unified_fils_vdev_config_send_cmd(void *wmi_hdl,
				      struct config_fils_params *param);

/**
 * wmi_extract_swfda_vdev_id() - api to extract vdev id
 * @wmi_hdl: wmi handle
 * @evt_buf: pointer to event buffer
 * @vdev_id: pointer to vdev id
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_extract_swfda_vdev_id(void *wmi_hdl, void *evt_buf,
				     uint32_t *vdev_id);

/**
 * wmi_unified_fils_discovery_send_cmd() - send FILS discovery cmd to fw
 * @wmi_hdl: wmi handle
 * @param:   fils discovery params
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_unified_fils_discovery_send_cmd(void *wmi_hdl,
					       struct fd_params *param);
#endif /* WLAN_SUPPORT_FILS */

/**
 * wmi_unified_offload_11k_cmd() - send 11k offload command
 * @wmi_hdl: wmi handle
 * @params: 11k offload params
 *
 * This function passes the 11k offload command params to FW
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_offload_11k_cmd(void *wmi_hdl,
				struct wmi_11k_offload_params *params);
/**
 * wmi_unified_invoke_neighbor_report_cmd() - send invoke neighbor report cmd
 * @wmi_hdl: wmi handle
 * @params: invoke neighbor report params
 *
 * This function passes the invoke neighbor report command to fw
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_invoke_neighbor_report_cmd(void *wmi_hdl,
			struct wmi_invoke_neighbor_report_params *params);
#endif /* _WMI_UNIFIED_API_H_ */
