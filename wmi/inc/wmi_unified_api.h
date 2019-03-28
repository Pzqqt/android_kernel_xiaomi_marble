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
#ifdef WLAN_POWER_MANAGEMENT_OFFLOAD
#include "wmi_unified_pmo_api.h"
#endif
#include "wlan_scan_public_structs.h"
#ifdef WLAN_FEATURE_DISA
#include "wlan_disa_public_struct.h"
#endif
#ifdef WLAN_FEATURE_ACTION_OUI
#include "wlan_action_oui_public_struct.h"
#endif
#ifdef WLAN_SUPPORT_GREEN_AP
#include "wlan_green_ap_api.h"
#endif
#ifdef WLAN_FEATURE_DSRC
#include "wlan_ocb_public_structs.h"
#endif
#ifdef WLAN_SUPPORT_TWT
#include "wmi_unified_twt_param.h"
#include "wmi_unified_twt_api.h"
#endif

#ifdef FEATURE_WLAN_EXTSCAN
#include "wmi_unified_extscan_api.h"
#endif

#ifdef IPA_OFFLOAD
#include "wlan_ipa_public_struct.h"
#endif

#ifdef WMI_SMART_ANT_SUPPORT
#include "wmi_unified_smart_ant_api.h"
#endif

#ifdef WMI_DBR_SUPPORT
#include "wmi_unified_dbr_api.h"
#endif

#ifdef WMI_ATF_SUPPORT
#include "wmi_unified_atf_api.h"
#endif

#ifdef WMI_AP_SUPPORT
#include "wmi_unified_ap_api.h"
#endif

#ifdef WLAN_FEATURE_DSRC
#include "wmi_unified_ocb_api.h"
#endif

#ifdef WLAN_FEATURE_NAN
#include "wmi_unified_nan_api.h"
#endif

#ifdef CONVERGED_P2P_ENABLE
#include <wmi_unified_p2p_api.h>
#endif

#ifdef WMI_ROAM_SUPPORT
#include "wmi_unified_roam_api.h"
#endif

#ifdef WMI_CONCURRENCY_SUPPORT
#include "wmi_unified_concurrency_api.h"
#endif

#ifdef WMI_STA_SUPPORT
#include "wmi_unified_sta_api.h"
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

/* Number of bits to shift to combine 32 bit integer to 64 bit */
#define WMI_LOWER_BITS_SHIFT_32	0x20

#define PHYERROR_MAX_BUFFER_LENGTH 0x7F000000

struct wmi_soc;
struct policy_mgr_dual_mac_config;
/**
 * struct wmi_rx_ops - handle to wmi rx ops
 * @scn_handle: handle to scn
 * @ev: event buffer
 * @rx_ctx: rx execution context
 * @wma_process_fw_event_handler_cbk: generic event handler callback
 */
struct wmi_rx_ops {

	int (*wma_process_fw_event_handler_cbk)(ol_scn_t scn_handle,
						void *ev,
						uint8_t rx_ctx);
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
 * struct wmi_unified_attach_params - wmi init parameters
 *  @param osdev            : NIC device
 *  @param target_type      : type of supported wmi command
 *  @param use_cookie       : flag to indicate cookie based allocation
 *  @param enable_vdev_pdev_param_conversion : To enable pdev vdev parametric
 *                                             id conversion from host type to
 *                                             target type
 *  @param ops              : handle to wmi ops
 *  @psoc                   : objmgr psoc
 *  @max_commands           : max commands
 */
struct wmi_unified_attach_params {
	osdev_t osdev;
	enum wmi_target_type target_type;
	bool use_cookie;
	bool is_async_ep;
	bool enable_vdev_pdev_param_conversion;
	struct wmi_rx_ops *rx_ops;
	struct wlan_objmgr_psoc *psoc;
	uint16_t max_commands;
	uint32_t soc_id;
};

/**
 * attach for unified WMI
 *
 *  @param scn_handle      : handle to SCN.
 *  @param params          : attach params for WMI
 *
 */
void *wmi_unified_attach(void *scn_handle,
			 struct wmi_unified_attach_params *params);



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
#ifdef NBUF_MEMORY_DEBUG
#define wmi_buf_alloc(h, l) wmi_buf_alloc_debug(h, l, __func__, __LINE__)
wmi_buf_t
wmi_buf_alloc_debug(wmi_unified_t wmi_handle, uint32_t len,
		    const char *func_name, uint32_t line_num);
#else
/**
 * wmi_buf_alloc() - generic function to allocate WMI buffer
 * @wmi_handle: handle to WMI.
 * @len: length of the buffer
 *
 * Return: return wmi_buf_t or null if memory alloc fails
 */
#define wmi_buf_alloc(wmi_handle, len) \
	wmi_buf_alloc_fl(wmi_handle, len, __func__, __LINE__)

wmi_buf_t wmi_buf_alloc_fl(wmi_unified_t wmi_handle, uint32_t len,
			   const char *func, uint32_t line);
#endif

/**
 * generic function frees WMI net buffer
 *
 *  @param net_buf : Pointer ot net_buf to be freed
 */
void wmi_buf_free(wmi_buf_t net_buf);

/**
 * wmi_unified_cmd_send() -  generic function to send unified WMI command
 * @wmi_handle: handle to WMI.
 * @buf: wmi command buffer
 * @buflen: wmi command buffer length
 * @cmd_id: WMI cmd id
 *
 * Note, it is NOT safe to access buf after calling this function!
 *
 * Return: QDF_STATUS
 */
#define wmi_unified_cmd_send(wmi_handle, buf, buflen, cmd_id) \
	wmi_unified_cmd_send_fl(wmi_handle, buf, buflen, \
				cmd_id, __func__, __LINE__)

QDF_STATUS
wmi_unified_cmd_send_fl(wmi_unified_t wmi_handle, wmi_buf_t buf,
			uint32_t buflen, uint32_t cmd_id,
			const char *func, uint32_t line);

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
 * wmi_is_target_suspended() - WMI API to check target suspend state
 * @wmi_handle: handle to WMI.
 *
 * WMI API to check target suspend state
 *
 * Return: true if target is suspended, else false.
 */
bool wmi_is_target_suspended(struct wmi_unified *wmi_handle);

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
				 uint8_t macaddr[QDF_MAC_ADDR_SIZE],
				 struct vdev_create_params *param);

QDF_STATUS wmi_unified_vdev_delete_send(void *wmi_hdl,
					  uint8_t if_id);

/**
 * wmi_unified_vdev_nss_chain_params_send() - send VDEV nss chain params to fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @nss_chains_user_cfg: user configured params to send
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_nss_chain_params_send(void *wmi_hdl,
			uint8_t vdev_id,
			struct vdev_nss_chains *nss_chains_user_cfg);

QDF_STATUS wmi_unified_vdev_stop_send(void *wmi_hdl,
					uint8_t vdev_id);

QDF_STATUS wmi_unified_vdev_up_send(void *wmi_hdl,
			     uint8_t bssid[QDF_MAC_ADDR_SIZE],
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

QDF_STATUS wmi_unified_sifs_trigger_send(void *wmi_hdl,
					 struct sifs_trigger_param *param);

QDF_STATUS wmi_unified_peer_delete_send(void *wmi_hdl,
				    uint8_t
				    peer_addr[QDF_MAC_ADDR_SIZE],
				    uint8_t vdev_id);

QDF_STATUS wmi_unified_peer_flush_tids_send(void *wmi_hdl,
					 uint8_t peer_addr[QDF_MAC_ADDR_SIZE],
					 struct peer_flush_params *param);

QDF_STATUS wmi_set_peer_param_send(void *wmi_hdl,
				uint8_t peer_addr[QDF_MAC_ADDR_SIZE],
				struct peer_set_params *param);

QDF_STATUS wmi_unified_peer_create_send(void *wmi_hdl,
					struct peer_create_params *param);

QDF_STATUS wmi_unified_stats_request_send(wmi_unified_t wmi_handle,
					  uint8_t macaddr[QDF_MAC_ADDR_SIZE],
					  struct stats_request_params *param);

QDF_STATUS wmi_unified_green_ap_ps_send(void *wmi_hdl,
					uint32_t value, uint8_t pdev_id);

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

/**
 *  wmi_unified_peer_based_pktlog_send() - WMI request enable peer
 *  based filtering
 *  @wmi_handle: handle to WMI.
 *  @macaddr: PEER mac address to be filtered
 *  @mac_id: Mac id
 *  @enb_dsb: Enable or Disable peer based pktlog
 *            filtering
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_peer_based_pktlog_send(void *wmi_hdl,
					      uint8_t *macaddr,
					      uint8_t mac_id,
					      uint8_t enb_dsb);
#else
QDF_STATUS wmi_unified_packet_log_enable_send(void *wmi_hdl,
				uint8_t macaddr[QDF_MAC_ADDR_SIZE],
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


QDF_STATUS wmi_unified_peer_assoc_send(void *wmi_hdl,
				struct peer_assoc_params *param);

QDF_STATUS wmi_unified_sta_ps_cmd_send(void *wmi_hdl,
				struct sta_ps_params *param);

QDF_STATUS wmi_unified_ap_ps_cmd_send(void *wmi_hdl,
				uint8_t macaddr[QDF_MAC_ADDR_SIZE],
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

#ifdef FEATURE_FW_LOG_PARSING
QDF_STATUS wmi_unified_dbglog_cmd_send(void *wmi_hdl,
				       struct dbglog_params *param);
#else
static inline QDF_STATUS
wmi_unified_dbglog_cmd_send(void *wmi_hdl,
			    struct dbglog_params *param)
{
	return QDF_STATUS_SUCCESS;
}
#endif

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

QDF_STATUS wmi_unified_set_smps_params(void *wmi_hdl, uint8_t vdev_id,
			       int value);

QDF_STATUS wmi_unified_set_mimops(void *wmi_hdl, uint8_t vdev_id, int value);

QDF_STATUS wmi_unified_lro_config_cmd(void *wmi_hdl,
	 struct wmi_lro_config_cmd_t *wmi_lro_cmd);

QDF_STATUS wmi_unified_set_thermal_mgmt_cmd(void *wmi_hdl,
				struct thermal_cmd_params *thermal_info);

QDF_STATUS wmi_unified_peer_rate_report_cmd(void *wmi_hdl,
		struct wmi_peer_rate_report_params *rate_report_params);

QDF_STATUS wmi_unified_process_update_edca_param(void *wmi_hdl,
		     uint8_t vdev_id, bool mu_edca_param,
		     struct wmi_host_wme_vparams wmm_vparams[WMI_MAX_NUM_AC]);

QDF_STATUS wmi_unified_probe_rsp_tmpl_send_cmd(void *wmi_hdl,
		   uint8_t vdev_id,
		   struct wmi_probe_resp_params *probe_rsp_info);

QDF_STATUS wmi_unified_setup_install_key_cmd(void *wmi_hdl,
			struct set_key_params *key_params);

QDF_STATUS wmi_unified_p2p_go_set_beacon_ie_cmd(void *wmi_hdl,
				    uint32_t vdev_id, uint8_t *p2p_ie);

QDF_STATUS wmi_unified_scan_probe_setoui_cmd(void *wmi_hdl,
			  struct scan_mac_oui *psetoui);

#ifdef IPA_OFFLOAD
QDF_STATUS  wmi_unified_ipa_offload_control_cmd(void *wmi_hdl,
		struct ipa_uc_offload_control_params *ipa_offload);
#endif

QDF_STATUS wmi_unified_pno_stop_cmd(void *wmi_hdl, uint8_t vdev_id);

#ifdef FEATURE_WLAN_SCAN_PNO
QDF_STATUS wmi_unified_pno_start_cmd(void *wmi_hdl,
		   struct pno_scan_req_params *pno);
#endif

QDF_STATUS wmi_unified_nlo_mawc_cmd(void *wmi_hdl,
		struct nlo_mawc_params *params);

#ifdef WLAN_FEATURE_LINK_LAYER_STATS
/**
 * wmi_unified_process_ll_stats_clear_cmd() - clear link layer stats
 * @wmi_handle: wmi handle
 * @clear_req: ll stats clear request command params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_process_ll_stats_clear_cmd(wmi_unified_t wmi_handle,
				 const struct ll_stats_clear_params *clear_req);

/**
 * wmi_unified_process_ll_stats_set_cmd() - link layer stats set request
 * @wmi_handle: wmi handle
 * @set_req: ll stats set request command params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_process_ll_stats_set_cmd(wmi_unified_t wmi_handle,
				 const struct ll_stats_set_params *set_req);

/**
 * wmi_unified_process_ll_stats_get_cmd() - link layer stats get request
 * @wmi_handle: wmi handle
 * @get_req: ll stats get request command params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_process_ll_stats_get_cmd(wmi_unified_t wmi_handle,
				 const struct ll_stats_get_params *get_req);
#endif /* WLAN_FEATURE_LINK_LAYER_STATS */

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

/**
 * wmi_unified_link_status_req_cmd() - process link status request from UMAC
 * @wmi_handle: wmi handle
 * @params: get link status params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_link_status_req_cmd(wmi_unified_t wmi_handle,
					   struct link_status_params *params);

#ifdef WLAN_SUPPORT_GREEN_AP
QDF_STATUS wmi_unified_egap_conf_params_cmd(void *wmi_hdl,
		struct wlan_green_ap_egap_params *egap_params);
#endif

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

QDF_STATUS wmi_unified_stats_ext_req_cmd(void *wmi_hdl,
			struct stats_ext_params *preq);

QDF_STATUS wmi_unified_process_dhcpserver_offload_cmd(void *wmi_hdl,
				struct dhcp_offload_info_params *params);

QDF_STATUS wmi_unified_send_regdomain_info_to_fw_cmd(void *wmi_hdl,
				   uint32_t reg_dmn, uint16_t regdmn2G,
				   uint16_t regdmn5G, uint8_t ctl2G,
				   uint8_t ctl5G);

QDF_STATUS wmi_unified_process_fw_mem_dump_cmd(void *wmi_hdl,
					struct fw_dump_req_param *mem_dump_req);

QDF_STATUS wmi_unified_cfg_action_frm_tb_ppdu_cmd(void *wmi_hdl,
				struct cfg_action_frm_tb_ppdu_param *cfg_info);

QDF_STATUS wmi_unified_save_fw_version_cmd(void *wmi_hdl,
		void *evt_buf);

QDF_STATUS wmi_unified_log_supported_evt_cmd(void *wmi_hdl,
		uint8_t *event,
		uint32_t len);

QDF_STATUS wmi_unified_enable_specific_fw_logs_cmd(void *wmi_hdl,
		struct wmi_wifi_start_log *start_log);

QDF_STATUS wmi_unified_flush_logs_to_fw_cmd(void *wmi_hdl);

QDF_STATUS wmi_unified_unit_test_cmd(void *wmi_hdl,
			       struct wmi_unit_test_cmd *wmi_utest);

#ifdef FEATURE_WLAN_APF
/**
 * wmi_unified_set_active_apf_mode_cmd() - config active APF mode in FW
 * @wmi: the WMI handle
 * @vdev_id: the Id of the vdev to apply the configuration to
 * @ucast_mode: the active APF mode to configure for unicast packets
 * @mcast_bcast_mode: the active APF mode to configure for multicast/broadcast
 *	packets
 */
QDF_STATUS
wmi_unified_set_active_apf_mode_cmd(wmi_unified_t wmi, uint8_t vdev_id,
				    enum wmi_host_active_apf_mode ucast_mode,
				    enum wmi_host_active_apf_mode
							      mcast_bcast_mode);

/**
 * wmi_unified_send_apf_enable_cmd() - send apf enable/disable cmd
 * @wmi: wmi handle
 * @vdev_id: VDEV id
 * @enable: true: enable, false: disable
 *
 * This function passes the apf enable command to fw
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_send_apf_enable_cmd(wmi_unified_t wmi,
					   uint32_t vdev_id, bool enable);

/**
 * wmi_unified_send_apf_write_work_memory_cmd() - send cmd to write into the APF
 *	work memory.
 * @wmi: wmi handle
 * @write_params: parameters and buffer pointer for the write
 *
 * This function passes the write apf work mem command to fw
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_send_apf_write_work_memory_cmd(wmi_unified_t wmi,
			struct wmi_apf_write_memory_params *write_params);

/**
 * wmi_unified_send_apf_read_work_memory_cmd() - send cmd to read part of APF
 *	work memory
 * @wmi: wmi handle
 * @read_params: contains relative address and length to read from
 *
 * This function passes the read apf work mem command to fw
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_send_apf_read_work_memory_cmd(wmi_unified_t wmi,
				struct wmi_apf_read_memory_params *read_params);

/**
 * wmi_extract_apf_read_memory_resp_event() - exctract read mem resp event
 * @wmi: wmi handle
 * @evt_buf: Pointer to the event buffer
 * @resp: pointer to memory to extract event parameters into
 *
 * This function exctracts read mem response event into the given structure ptr
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_apf_read_memory_resp_event(wmi_unified_t wmi, void *evt_buf,
				struct wmi_apf_read_memory_resp_event_params
								*read_mem_evt);
#endif /* FEATURE_WLAN_APF */

QDF_STATUS wmi_send_get_user_position_cmd(void *wmi_hdl, uint32_t value);

QDF_STATUS wmi_send_get_peer_mumimo_tx_count_cmd(void *wmi_hdl, uint32_t value);

QDF_STATUS wmi_send_reset_peer_mumimo_tx_count_cmd(void *wmi_hdl,
				uint32_t value);

QDF_STATUS wmi_unified_send_btcoex_wlan_priority_cmd(void *wmi_hdl,
				struct btcoex_cfg_params *param);

QDF_STATUS wmi_unified_send_btcoex_duty_cycle_cmd(void *wmi_hdl,
				struct btcoex_cfg_params *param);

QDF_STATUS wmi_unified_send_coex_ver_cfg_cmd(void *wmi_hdl,
				coex_ver_cfg_t *param);

QDF_STATUS wmi_unified_send_coex_config_cmd(void *wmi_hdl,
					    struct coex_config_params *param);

QDF_STATUS wmi_unified_pdev_fips_cmd_send(void *wmi_hdl,
				struct fips_params *param);

QDF_STATUS wmi_unified_wlan_profile_enable_cmd_send(void *wmi_hdl,
				struct wlan_profile_params *param);

QDF_STATUS wmi_unified_wlan_profile_trigger_cmd_send(void *wmi_hdl,
				struct wlan_profile_params *param);

QDF_STATUS wmi_unified_set_chan_cmd_send(void *wmi_hdl,
				struct channel_param *param);

QDF_STATUS wmi_unified_set_ratepwr_table_cmd_send(void *wmi_hdl,
				struct ratepwr_table_params *param);

QDF_STATUS wmi_unified_get_ratepwr_table_cmd_send(void *wmi_hdl);

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

/**
 *  wmi_unified_set_bridge_mac_addr_cmd_send() - WMI set bridge mac addr cmd function
 *  @param wmi_hdl      : handle to WMI.
 *  @param param        : pointer to hold bridge mac addr param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_bridge_mac_addr_cmd_send(void *wmi_hdl,
				struct set_bridge_mac_addr_params *param);


QDF_STATUS wmi_unified_phyerr_enable_cmd_send(void *wmi_hdl);

QDF_STATUS wmi_unified_phyerr_enable_cmd_send(void *wmi_hdl);

QDF_STATUS wmi_unified_phyerr_disable_cmd_send(void *wmi_hdl);

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

QDF_STATUS wmi_unified_vdev_set_fwtest_param_cmd_send(void *wmi_hdl,
				struct set_fwtest_params *param);

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

QDF_STATUS wmi_unified_set_beacon_filter_cmd_send(void *wmi_hdl,
				struct set_beacon_filter_params *param);

QDF_STATUS wmi_unified_remove_beacon_filter_cmd_send(void *wmi_hdl,
				struct remove_beacon_filter_params *param);

QDF_STATUS wmi_unified_addba_clearresponse_cmd_send(void *wmi_hdl,
				uint8_t macaddr[QDF_MAC_ADDR_SIZE],
				struct addba_clearresponse_params *param);

QDF_STATUS wmi_unified_addba_send_cmd_send(void *wmi_hdl,
				uint8_t macaddr[QDF_MAC_ADDR_SIZE],
				struct addba_send_params *param);

QDF_STATUS wmi_unified_delba_send_cmd_send(void *wmi_hdl,
				uint8_t macaddr[QDF_MAC_ADDR_SIZE],
				struct delba_send_params *param);

QDF_STATUS wmi_unified_addba_setresponse_cmd_send(void *wmi_hdl,
				uint8_t macaddr[QDF_MAC_ADDR_SIZE],
				struct addba_setresponse_params *param);

QDF_STATUS wmi_unified_singleamsdu_cmd_send(void *wmi_hdl,
				uint8_t macaddr[QDF_MAC_ADDR_SIZE],
				struct singleamsdu_params *param);

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

/* Extract APIs */

QDF_STATUS wmi_extract_fips_event_data(void *wmi_hdl, void *evt_buf,
		struct wmi_host_fips_event_param *param);

QDF_STATUS wmi_extract_mgmt_rx_params(void *wmi_hdl, void *evt_buf,
		struct mgmt_rx_event_params *hdr, uint8_t **bufp);

QDF_STATUS wmi_extract_vdev_roam_param(void *wmi_hdl, void *evt_buf,
		wmi_host_roam_event *ev);

QDF_STATUS wmi_extract_vdev_scan_ev_param(void *wmi_hdl, void *evt_buf,
		struct scan_event *param);

QDF_STATUS wmi_extract_mu_ev_param(void *wmi_hdl, void *evt_buf,
		wmi_host_mu_report_event *param);

QDF_STATUS wmi_extract_mu_db_entry(void *wmi_hdl, void *evt_buf,
		uint8_t idx, wmi_host_mu_db_entry *param);

QDF_STATUS wmi_extract_mumimo_tx_count_ev_param(void *wmi_hdl, void *evt_buf,
		wmi_host_peer_txmu_cnt_event *param);

QDF_STATUS wmi_extract_peer_gid_userpos_list_ev_param(void *wmi_hdl,
		void *evt_buf, wmi_host_peer_gid_userpos_list_event *param);

QDF_STATUS
wmi_extract_esp_estimate_ev_param(void *wmi_hdl, void *evt_buf,
				  struct esp_estimation_event *param);

QDF_STATUS wmi_extract_gpio_input_ev_param(void *wmi_hdl,
		void *evt_buf, uint32_t *gpio_num);

QDF_STATUS wmi_extract_pdev_reserve_ast_ev_param(void *wmi_hdl,
		void *evt_buf, struct wmi_host_proxy_ast_reserve_param *param);

QDF_STATUS wmi_extract_pdev_generic_buffer_ev_param(void *wmi_hdl,
		void *evt_buf,
		wmi_host_pdev_generic_buffer_event *param);

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

QDF_STATUS wmi_extract_peer_adv_stats(wmi_unified_t wmi_handle, void *evt_buf,
				      struct wmi_host_peer_adv_stats
				      *peer_adv_stats);

QDF_STATUS wmi_extract_bss_chan_info_event(void *wmi_hdl, void *evt_buf,
			wmi_host_pdev_bss_chan_info_event *bss_chan_info);

QDF_STATUS wmi_extract_peer_stats(void *wmi_hdl, void *evt_buf,
		uint32_t index, wmi_host_peer_stats *peer_stats);

QDF_STATUS wmi_extract_tx_data_traffic_ctrl_ev(void *wmi_hdl, void *evt_buf,
		wmi_host_tx_data_traffic_ctrl_event *ev);

QDF_STATUS wmi_extract_vdev_stats(void *wmi_hdl, void *evt_buf,
		uint32_t index, wmi_host_vdev_stats *vdev_stats);

QDF_STATUS wmi_extract_per_chain_rssi_stats(void *wmi_hdl, void *evt_buf,
	uint32_t index, struct wmi_host_per_chain_rssi_stats *rssi_stats);

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

/**
 * wmi_extract_peer_retry_stats() - extract peer retry stats from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @index: Index into peer retry stats
 * @peer_retry_stats: Pointer to hold  peer retry stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_peer_retry_stats(void *wmi_hdl, void *evt_buf,
	uint32_t index, struct wmi_host_peer_retry_stats *peer_retry_stats);

QDF_STATUS wmi_unified_send_power_dbg_cmd(void *wmi_hdl,
				struct wmi_power_dbg_params *param);

/**
 * wmi_extract_sar_cap_service_ready_ext() - extract SAR cap from
 *					     FW service ready event
 * @wmi_hdl: wmi handle
 * @evt_buf: event buffer received from firmware
 * @ext_param: extended target info
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_extract_sar_cap_service_ready_ext(
			void *wmi_hdl,
			uint8_t *evt_buf,
			struct wlan_psoc_host_service_ext_param *ext_param);

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
 * wmi_extract_spectral_scaling_params_service_ready_ext: Extract Spectral
 *                                             scaling params received through
 *                                             extended service ready event
 * @wmi_hdl: WMI handle
 * @evt_buf: Event buffer
 * @idx: Index
 * @param: Pointer to Spectral scaling params
 *
 * Return: QDF status of operation
 */
QDF_STATUS wmi_extract_spectral_scaling_params_service_ready_ext(
			void *wmi_hdl,
			uint8_t *evt_buf, uint8_t idx,
			struct wlan_psoc_host_spectral_scaling_params *param);

QDF_STATUS wmi_extract_pdev_utf_event(void *wmi_hdl,
				      uint8_t *evt_buf,
				      struct wmi_host_pdev_utf_event *param);

QDF_STATUS wmi_extract_pdev_qvit_event(void *wmi_hdl,
				      uint8_t *evt_buf,
				      struct wmi_host_pdev_qvit_event *param);

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

#ifdef QCA_SUPPORT_AGILE_DFS
/**
 * wmi_unified_send_vdev_adfs_ch_cfg_cmd() - send adfs channel config command
 * @wmi_handle: wmi handle
 * @vdev_adfs_ch_cfg_params: adfs channel config params
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_unified_send_vdev_adfs_ch_cfg_cmd(void *wmi_hdl,
				      struct vdev_adfs_ch_cfg_params *param);

/**
 * wmi_unified_send_vdev_adfs_ocac_abort_cmd() - send adfs o-cac abort command
 * @wmi_handle: wmi handle
 * @vdev_adfs_abort_params: adfs channel o-cac abort params
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_unified_send_vdev_adfs_ocac_abort_cmd(void *wmi_hdl,
					  struct vdev_adfs_abort_params *param);
#endif

QDF_STATUS wmi_unified_set_country_cmd_send(void *wmi_hdl,
				struct set_country *param);

#ifdef WLAN_FEATURE_ACTION_OUI
/**
 * wmi_unified_send_action_oui_cmd() - send action oui cmd to fw
 * @wmi_hdl: wma handle
 * @req: wmi action oui message to be send
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_send_action_oui_cmd(void *wmi_hdl,
				struct action_oui_request *req);
#endif /* WLAN_FEATURE_ACTION_OUI */

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

#ifdef WLAN_SUPPORT_GREEN_AP
QDF_STATUS wmi_extract_green_ap_egap_status_info(
		void *wmi_hdl, uint8_t *evt_buf,
		struct wlan_green_ap_egap_status_info *egap_status_info_params);
#endif

/**
 * wmi_unified_send_roam_scan_stats_cmd() - Wrapper to request roam scan stats
 * @wmi_hdl: wmi handle
 * @params: request params
 *
 * This function is used to send the roam scan stats request command to
 * firmware.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_send_roam_scan_stats_cmd(void *wmi_hdl,
				     struct wmi_roam_scan_stats_req *params);

/**
 * wmi_extract_roam_scan_stats_res_evt() - API to extract roam scan stats res
 * @wmi: wmi handle
 * @evt_buf: pointer to the event buffer
 * @vdev_id: output pointer to hold vdev id
 * @res_param: output pointer to hold extracted memory
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_extract_roam_scan_stats_res_evt(wmi_unified_t wmi, void *evt_buf,
				    uint32_t *vdev_id,
				    struct wmi_roam_scan_stats_res **res_param);

/**
 * wmi_extract_offload_bcn_tx_status_evt() - API to extract bcn tx status event
 * @wmi: wmi handle
 * @evt_buf: pointer to the event buffer
 * @vdev_id: output pointer to hold vdev id
 * @tx_status: output pointer to hold bcn tx status
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_extract_offload_bcn_tx_status_evt(void *wmi_hdl, void *evt_buf,
				      uint32_t *vdev_id, uint32_t *tx_status);

/* wmi_get_ch_width_from_phy_mode() - convert phy mode to channel width
 * @wmi_hdl: wmi handle
 * @phymode: phy mode
 *
 * Return: wmi channel width
 */
wmi_host_channel_width wmi_get_ch_width_from_phy_mode(void *wmi_hdl,
					WMI_HOST_WLAN_PHY_MODE phymode);

#ifdef QCA_SUPPORT_CP_STATS
/**
 * wmi_extract_cca_stats() - api to extract congestion stats from event buffer
 * @wmi_handle: wma handle
 * @evt_buf: event buffer
 * @datalen: length of buffer
 * @stats: buffer to populated after stats extraction
 *
 * Return: status of operation
 */
QDF_STATUS wmi_extract_cca_stats(wmi_unified_t wmi_handle, void *evt_buf,
				 struct wmi_host_congestion_stats *stats);
#endif /* QCA_SUPPORT_CP_STATS */

#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(HOST_DFS_SPOOF_TEST)
/**
 * wmi_unified_dfs_send_avg_params_cmd() - send average radar parameters cmd.
 * @wmi_hdl: wmi handle
 * @params: radar found params
 *
 * This function passes the average radar parameters to fw
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_dfs_send_avg_params_cmd(void *wmi_hdl,
				    struct dfs_radar_found_params *params);

/**
 * wmi_extract_dfs_status_from_fw() - extract host dfs status from fw.
 * @wmi_hdl: wmi handle
 * @evt_buf: pointer to event buffer
 * @dfs_status_check: pointer to the host dfs status
 *
 * This function extracts the result of host dfs from fw
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_dfs_status_from_fw(void *wmi_hdl, void *evt_buf,
					  uint32_t  *dfs_status_check);
#endif
#ifdef OL_ATH_SMART_LOGGING
/**
 * wmi_unified_send_smart_logging_enable_cmd() - send smart logging enable cmd
 * @wmi_hdl: wmi handle
 * @params: enable/disable
 *
 * This function enables/disable the smart logging feature
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_send_smart_logging_enable_cmd(void *wmi_hdl,
						     uint32_t param);

/**
 * wmi_unified_send_smart_logging_fatal_cmd() - send smart logging fatal cmd
 * @wmi_hdl: wmi handle
 * @param:  Fatal event
 *
 * This function sends the smart log fatal events to the FW
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_send_smart_logging_fatal_cmd
					(void *wmi_hdl,
					struct wmi_debug_fatal_events *param);

/**
 * wmi_extract_smartlog_ev() - extract smartlog event info from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param ev: Pointer to hold fatal events
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_smartlog_ev
				(void *wmi_hdl, void *evt_buf,
				struct wmi_debug_fatal_events *ev);

#endif /* OL_ATH_SMART_LOGGING */

/**
 * wmi_process_fw_event_worker_thread_ctx() - process in worker thread context
 * @wmi_handle: handle to wmi
 * @evt_buf: pointer to event buffer
 *
 * Event process by below function will be in worker thread context.
 * Use this method for events which are not critical and not
 * handled in protocol stack.
 *
 * Return: none
 */
void wmi_process_fw_event_worker_thread_ctx(struct wmi_unified *wmi_handle,
					    void *evt_buf);

/**
 * wmi_extract_ctl_failsafe_check_ev_param() - extract ctl failsafe
 * status from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param ev: Pointer to hold ctl status
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_ctl_failsafe_check_ev_param(void *wmi_hdl,
					void *evt_buf,
					struct wmi_host_pdev_ctl_failsafe_event
					*param);

#ifdef OBSS_PD
/**
 * wmi_unified_send_obss_spatial_reuse_set_cmd() - send obss pd offset
 * @wmi_handle: wmi handle
 * @oobss_spatial_reuse_param: Pointer to obsspd min max offset
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_send_obss_spatial_reuse_set_cmd(void *wmi_handle,
	struct wmi_host_obss_spatial_reuse_set_param *obss_spatial_reuse_param);

/**
 * wmi_unified_send_obss_spatial_reuse_set_def_thresh_cmd() - send def thresh
 * @wmi_handle: wmi handle
 * @thresh: Pointer to def thresh
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_send_obss_spatial_reuse_set_def_thresh_cmd(void *wmi_hdl,
	struct wmi_host_obss_spatial_reuse_set_def_thresh *thresh);

#endif /* OBSS_PD */

/**
 * wmi_convert_pdev_id_host_to_target() - Convert pdev_id from host to target
 * defines. For legacy there is not conversion required. Just return pdev_id as
 * it is.
 * @wmi_hdl: wmi handle
 * @host_pdev_id: host pdev_id to be converted.
 * @target_pdev_id: Output target pdev id.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wmi_convert_pdev_id_host_to_target(void *wmi_hdl,
					      uint32_t host_pdev_id,
					      uint32_t *target_pdev_id);

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

#ifdef CRYPTO_SET_KEY_CONVERGED
/**
 * wlan_crypto_cipher_to_wmi_cipher() - Convert crypto cipher to WMI cipher
 * @crypto_cipher: cipher type in crypto format
 *
 * Return: cipher type in WMI cipher type
 */
uint8_t wlan_crypto_cipher_to_wmi_cipher(
		enum wlan_crypto_cipher_type crypto_cipher);

/**
 * wlan_crypto_cipher_to_cdp_sec_type() - Convert crypto cipher to CDP type
 * @crypto_cipher: cipher type in crypto format
 *
 * Return: security type in cdp_sec_type data format type
 */
enum cdp_sec_type wlan_crypto_cipher_to_cdp_sec_type(
		enum wlan_crypto_cipher_type crypto_cipher);

#endif

/**
 * wmi_unified_send_mws_coex_req_cmd() - WMI function to send coex req cmd
 * @wmi_hdl: wmi handle
 * @vdev_id: Vdev Id
 * @cmd_id: Coex cmd for which info is required
 *
 * Send wmi coex command to fw.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wmi_unified_send_mws_coex_req_cmd(struct wmi_unified *wmi_handle,
					     uint32_t vdev_id, uint32_t cmd_id);

#ifdef WLAN_CFR_ENABLE
/**
 * wmi_unified_send_peer_cfr_capture_cmd() - WMI function to start CFR capture
 * for a peer
 * @wmi_hdl: WMI handle
 * @param: configuration params for capture
 *
 * Return: QDF_STATUS_SUCCESS if success, else returns proper error code.
 */
QDF_STATUS
wmi_unified_send_peer_cfr_capture_cmd(void *wmi_hdl,
				      struct peer_cfr_params *param);
/**
 * wmi_extract_cfr_peer_tx_event_param() - WMI function to extract cfr tx event
 * for a peer
 * @wmi_hdl: WMI handle
 * @evt_buf: Buffer holding event data
 * @peer_tx_event: pointer to hold tx event data
 *
 * Return: QDF_STATUS_SUCCESS if success, else returns proper error code.
 */
QDF_STATUS
wmi_extract_cfr_peer_tx_event_param(void *wmi_hdl, void *evt_buf,
				    wmi_cfr_peer_tx_event_param *peer_tx_event);
#endif /* WLAN_CFR_ENABLE */
#endif /* _WMI_UNIFIED_API_H_ */
