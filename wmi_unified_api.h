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

typedef cdf_nbuf_t wmi_buf_t;
#define wmi_buf_data(_buf) cdf_nbuf_data(_buf)

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
 * WMA Callback to process fw event.
 */
void wmi_process_fw_event(struct wmi_unified *wmi_handle, wmi_buf_t evt_buf);
uint16_t wmi_get_max_msg_len(wmi_unified_t wmi_handle);


int32_t wmi_unified_vdev_create_send(void *wmi_hdl,
				 uint8_t macaddr[IEEE80211_ADDR_LEN],
				 struct vdev_create_params *param);

int32_t wmi_unified_vdev_delete_send(void *wmi_hdl,
					  uint8_t if_id);

int32_t wmi_unified_vdev_start_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct vdev_start_params *param);

int32_t wmi_unified_vdev_restart_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct vdev_start_params *param);

int32_t wmi_unified_vdev_stop_send(void *wmi_hdl,
					uint8_t vdev_id);

int32_t wmi_unified_vdev_up_send(void *wmi_hdl,
			     uint8_t bssid[IEEE80211_ADDR_LEN],
				 struct vdev_up_params *params);

int32_t wmi_unified_vdev_down_send(void *wmi_hdl,
				uint8_t vdev_id);

int32_t wmi_unified_vdev_set_param_send(void *wmi_hdl,
				struct vdev_set_params *param);

int32_t wmi_unified_peer_delete_send(void *wmi_hdl,
				    uint8_t
				    peer_addr[IEEE80211_ADDR_LEN],
				    uint8_t vdev_id);

int32_t wmi_unified_peer_flush_tids_send(void *wmi_hdl,
					 uint8_t peer_addr[IEEE80211_ADDR_LEN],
					 struct peer_flush_params *param);

int32_t wmi_set_peer_param_send(void *wmi_hdl,
				uint8_t peer_addr[IEEE80211_ADDR_LEN],
				struct peer_set_params *param);

int32_t wmi_unified_peer_create_send(void *wmi_hdl,
					struct peer_create_params *param);

int32_t wmi_unified_stats_request_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct stats_request_params *param);

int32_t wmi_unified_green_ap_ps_send(void *wmi_hdl,
						uint32_t value, uint8_t mac_id);


int32_t wmi_unified_wow_enable_send(void *wmi_hdl,
				struct wow_cmd_params *param,
				uint8_t mac_id);

int32_t wmi_unified_packet_log_enable_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct packet_enable_params *param);


int32_t wmi_unified_suspend_send(void *wmi_hdl,
				struct suspend_params *param,
				uint8_t mac_id);

int32_t wmi_unified_resume_send(void *wmi_hdl,
				uint8_t mac_id);

int32_t
wmi_unified_pdev_param_send(void *wmi_hdl,
			   struct pdev_params *param,
				uint8_t mac_id);

int32_t wmi_unified_beacon_send_cmd(void *wmi_hdl,
				struct beacon_params *param);

int32_t wmi_unified_peer_assoc_send(void *wmi_hdl,
				struct peer_assoc_params *param);

int32_t wmi_unified_sta_ps_cmd_send(void *wmi_hdl,
				struct sta_ps_params *param);

int32_t wmi_unified_ap_ps_cmd_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct ap_ps_params *param);

int32_t wmi_unified_scan_start_cmd_send(void *wmi_hdl,
				struct scan_start_params *param);

int32_t wmi_unified_scan_stop_cmd_send(void *wmi_hdl,
				struct scan_stop_params *param);

int32_t wmi_unified_scan_chan_list_cmd_send(void *wmi_hdl,
				struct scan_chan_list_params *param);


int32_t wmi_crash_inject(void *wmi_hdl,
				struct crash_inject *param);

int32_t wmi_unified_pdev_utf_cmd_send(void *wmi_hdl,
				struct pdev_utf_params *param,
				uint8_t mac_id);

int32_t wmi_unified_dbglog_cmd_send(void *wmi_hdl,
				struct dbglog_params *param);

int32_t wmi_mgmt_unified_cmd_send(void *wmi_hdl,
				struct wmi_mgmt_params *param);

int32_t wmi_unified_modem_power_state(void *wmi_hdl,
		uint32_t param_value);

int32_t wmi_unified_set_sta_ps_mode(void *wmi_hdl,
			       uint32_t vdev_id, uint8_t val);
int32_t
wmi_unified_set_sta_uapsd_auto_trig_cmd(void *wmi_hdl,
				struct sta_uapsd_trig_params *param);

int32_t wmi_unified_get_temperature(void *wmi_hdl);

int32_t wmi_unified_set_p2pgo_oppps_req(void *wmi_hdl,
		struct p2p_ps_params *oppps);

int32_t wmi_unified_set_p2pgo_noa_req_cmd(void *wmi_hdl,
			struct p2p_ps_params *noa);

int32_t wmi_unified_set_smps_params(void *wmi_hdl, uint8_t vdev_id,
			       int value);

int32_t wmi_unified_set_mimops(void *wmi_hdl, uint8_t vdev_id, int value);
#endif /* _WMI_UNIFIED_API_H_ */
