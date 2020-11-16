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

/*
 * This file contains the API definitions for the Unified Wireless Module
 * Interface (WMI).
 */

#ifndef _WMI_UNIFIED_API_H_
#define _WMI_UNIFIED_API_H_

#include <osdep.h>
#include "htc_api.h"
#include "wmi_unified_param.h"
#include "service_ready_param.h"
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_mgmt_txrx_utils_api.h"
#include <wlan_dfs_public_struct.h>
#include <wlan_crypto_global_def.h>
#include "wlan_thermal_public_struct.h"
#ifdef WLAN_POWER_MANAGEMENT_OFFLOAD
#include "wmi_unified_pmo_api.h"
#endif
#ifdef WLAN_FEATURE_MIB_STATS
#include "wlan_cp_stats_mc_defs.h"
#endif
#include "wlan_scan_public_structs.h"
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

#ifdef WLAN_FW_OFFLOAD
#include "wmi_unified_fwol_api.h"
#endif

#ifdef WLAN_FEATURE_PKT_CAPTURE
#include "wlan_pkt_capture_public_structs.h"
#endif

#ifdef WLAN_IOT_SIM_SUPPORT
#include "wlan_iot_sim_public_structs.h"
#endif

#include "wmi_unified_cp_stats_api.h"

typedef qdf_nbuf_t wmi_buf_t;
#define wmi_buf_data(_buf) qdf_nbuf_data(_buf)

/* Number of bits to shift to combine 32 bit integer to 64 bit */
#define WMI_LOWER_BITS_SHIFT_32	0x20

#define PHYERROR_MAX_BUFFER_LENGTH 0x7F000000

struct wmi_soc;
struct policy_mgr_dual_mac_config;

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
 * @WMI_RX_SERIALIZER_CTX: Execution context is serialized thread context
 * @WMI_RX_DIAG_WORK_CTX: work queue execution context for FW diag events
 *
 */
enum wmi_rx_exec_ctx {
	WMI_RX_WORK_CTX,
	WMI_RX_UMAC_CTX,
	WMI_RX_TASKLET_CTX = WMI_RX_UMAC_CTX,
	WMI_RX_SERIALIZER_CTX = 2,
	WMI_RX_DIAG_WORK_CTX
};

/**
 * enum wmi_rx_buff_type - wmi rx event buffer type
 * @WMI_RX_PROCESSED_BUFF: processed event buffer provided by WMI layer
 * @WMI_RX_RAW_BUFF:       raw event buffer provided by WMI layer
 *
 */
enum wmi_rx_buff_type {
	WMI_RX_PROCESSED_BUFF,
	WMI_RX_RAW_BUFF
};

/**
 * enum wmi_fw_mem_prio - defines FW Memory requirement type
 * @WMI_FW_MEM_HIGH_PRIORITY:   Memory requires contiguous memory allocation
 * @WMI_FW_MEM_LOW_PRIORITY:    Memory can be fragmented
 * @WMI_FW_PRIORITY_MAX:        Invalid type
 */
enum wmi_fw_mem_prio {
	WMI_FW_MEM_HIGH_PRIORITY = 0,
	WMI_FW_MEM_LOW_PRIORITY,
	WMI_FW_PRIORITY_MAX
};

/**
 * struct wmi_unified_attach_params - wmi init parameters
 *  @osdev: NIC device
 *  @target_type: type of supported wmi command
 *  @use_cookie: flag to indicate cookie based allocation
 *  @is_async_ep: queueing interrupt or non-interrupt endpoint
 *  @rx_ops: handle to wmi ops
 *  @psoc: objmgr psoc
 *  @max_commands: max commands
 *  @soc_id: SoC device instance id
 */
struct wmi_unified_attach_params {
	osdev_t osdev;
	enum wmi_target_type target_type;
	bool use_cookie;
	bool is_async_ep;
	struct wlan_objmgr_psoc *psoc;
	uint16_t max_commands;
	uint32_t soc_id;
};

/**
 *  struct wmi_unified_exec_ctx - wmi execution ctx and handler buff
 *  @exec_ctx:  execution context of event
 *  @buff_type: buffer type for event handler
 */
struct wmi_unified_exec_ctx {
	enum wmi_rx_exec_ctx exec_ctx;
	enum wmi_rx_buff_type buff_type;
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
 * @wmi_handle: handle to WMI.
 *
 * Return: none
 */
void wmi_send_time_stamp_sync_cmd_tlv(wmi_unified_t wmi_handle);

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

#ifdef WLAN_FEATURE_WMI_SEND_RECV_QMI
/**
 * wmi_unified_cmd_send_over_qmi() -  generic function to send unified WMI command
 *                               over QMI
 * @wmi_handle: handle to WMI.
 * @buf: wmi command buffer
 * @buflen: wmi command buffer length
 * @cmd_id: WMI cmd id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wmi_unified_cmd_send_over_qmi(struct wmi_unified *wmi_handle,
				    wmi_buf_t buf, uint32_t buflen,
				    uint32_t cmd_id);

/**
 * wmi_process_qmi_fw_event() - Process WMI event received over QMI
 * @wmi_cb_ctx: WMI handle received as call back context
 * @buf: Pointer to WMI event buffer
 * @len: Len of WMI buffer received
 *
 * Return: None
 */
int wmi_process_qmi_fw_event(void *wmi_cb_ctx, void *buf, int len);
#else
static inline
QDF_STATUS wmi_unified_cmd_send_over_qmi(struct wmi_unified *wmi_handle,
				    wmi_buf_t buf, uint32_t buflen,
				    uint32_t cmd_id)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline int wmi_process_qmi_fw_event(void *wmi_cb_ctx, void *buf, int len)
{
	return -EINVAL;
}
#endif

/**
 * wmi_unified_cmd_send_pm_chk() - send unified WMI command with PM check,
 * if target is in suspended state, WMI command will be sent over QMI.
 * @wmi_handle: handle to WMI.
 * @buf: wmi command buffer
 * @buflen: wmi command buffer length
 * @cmd_id: WMI cmd id
 *
 * Note, it is NOT safe to access buf after calling this function!
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wmi_unified_cmd_send_pm_chk(struct wmi_unified *wmi_handle,
				       wmi_buf_t buf, uint32_t buflen,
				       uint32_t cmd_id);

/**
 * wmi_unified_register_event() - WMI event handler
 * registration function for converged components
 * @wmi_handle:   handle to WMI.
 * @event_id:     WMI event ID
 * @handler_func: Event handler call back function
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_unified_register_event(wmi_unified_t wmi_handle,
				   uint32_t event_id,
				   wmi_unified_event_handler handler_func);

/**
 * wmi_unified_register_event_handler() - WMI event handler
 * registration function
 * @wmi_handle:   handle to WMI.
 * @event_id:     WMI event ID
 * @handler_func: Event handler call back function
 * @rx_ctx: rx event processing context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_unified_register_event_handler(wmi_unified_t wmi_handle,
				   wmi_conv_event_id event_id,
				   wmi_unified_event_handler handler_func,
				   uint8_t rx_ctx);

/**
 * wmi_unified_unregister_event() - WMI event handler unregister function
 * for converged componets
 * @wmi_handle:    handle to WMI.
 * @event_id:      WMI event ID
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_unified_unregister_event(wmi_unified_t wmi_handle,
			     uint32_t event_id);

/**
 * wmi_unified_register_raw_event_handler() - WMI event handler
 * registration function.
 * @wmi_handle:   handle to WMI.
 * @event_id:     WMI event ID
 * @handler_func: Event handler call back function
 * @rx_ctx:       rx event processing context
 *
 * Register event handler to get struct wmi_raw_event_buffer as arg
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_unified_register_raw_event_handler(wmi_unified_t wmi_handle,
				       wmi_conv_event_id event_id,
				       wmi_unified_event_handler handler_func,
				       enum wmi_rx_exec_ctx rx_ctx);

/**
 * wmi_unified_unregister_event_handler() - WMI event handler unregister
 * function
 * wmi_handle:  handle to WMI.
 * event_id:    WMI event ID
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_unified_unregister_event_handler(wmi_unified_t wmi_handle,
				     wmi_conv_event_id event_id);

/**
 * wmi_unified_connect_htc_service() -  WMI API to get connect to HTC service
 * @wmi_handle: handle to WMI.
 * @htc_handle: handle to HTC.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAULT for failure
 */
QDF_STATUS
wmi_unified_connect_htc_service(struct wmi_unified *wmi_handle,
				HTC_HANDLE htc_handle);

#ifdef WLAN_FEATURE_WMI_DIAG_OVER_CE7
/**
 * wmi_diag_connect_pdev_htc_service()
 * WMI DIAG API to get connect to HTC service
 * @wmi_handle: handle to WMI.
 * @htc_handle: handle to HTC.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAULT for failure
 */
QDF_STATUS
wmi_diag_connect_pdev_htc_service(struct wmi_unified *wmi_handle,
				  HTC_HANDLE htc_handle);
#else
static inline QDF_STATUS
wmi_diag_connect_pdev_htc_service(struct wmi_unified *wmi_handle,
				  HTC_HANDLE htc_handle)
{
	return QDF_STATUS_SUCCESS;
}
#endif

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

#ifdef WLAN_FEATURE_WMI_SEND_RECV_QMI
/**
 *  wmi_set_qmi_stats() - WMI API to set qmi stats enabled/disabled
 *  @wmi_handle: handle to WMI.
 *  @val: suspend state boolean
 */
void wmi_set_qmi_stats(wmi_unified_t wmi_handle, bool val);

/**
 * wmi_is_qmi_stats_enabled() - WMI API to check if periodic stats
 * over qmi is enableid
 * @wmi_handle: handle to WMI.
 *
 * WMI API to check if periodic stats over qmi is enabled
 *
 * Return: true if qmi stats is enabled, else false.
 */
bool wmi_is_qmi_stats_enabled(struct wmi_unified *wmi_handle);
#else
static inline
void wmi_set_qmi_stats(wmi_unified_t wmi_handle, bool val)
{}

static inline
bool wmi_is_qmi_stats_enabled(struct wmi_unified *wmi_handle)
{
	return false;
}
#endif /* end if of WLAN_FEATURE_WMI_SEND_RECV_QMI */

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
 * generic function to start unified WMI command
 * @param wmi_handle      : handle to WMI.
 * @return 0  on success and -ve on failure.
 */
int
wmi_start(wmi_unified_t wmi_handle);

/**
 * wmi_is_blocked() - generic function to check if WMI is blocked
 * @wmi_handle: handle to WMI.
 *
 * @Return: true, if blocked, false if not blocked
 */
bool
wmi_is_blocked(wmi_unified_t wmi_handle);

/**
 * API to flush all the previous packets  associated with the wmi endpoint
 *
 * @param wmi_handle      : handle to WMI.
 */
void
wmi_flush_endpoint(wmi_unified_t wmi_handle);

/**
 * API to stop wmi sequence check
 *
 * @param wmi_handle      : handle to WMI.
 */
void
wmi_interface_sequence_stop(wmi_unified_t wmi_handle);

/**
 * wmi_pdev_id_conversion_enable() - API to enable pdev_id and phy_id
 *                     conversion in WMI. By default pdev_id and
 *                     phyid conversion is not done in WMI.
 *                     This API can be used enable conversion in WMI.
 * @param wmi_handle   : handle to WMI
 * @param *pdev_id_map : pdev conversion map
 * @param size         : size of pdev_id_map
 * Return none
 */
void wmi_pdev_id_conversion_enable(wmi_unified_t wmi_handle,
				   uint32_t *pdev_id_map,
				   uint8_t size);

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

/**
 * wmi_unified_soc_set_hw_mode_cmd() - Send WMI_SOC_SET_HW_MODE_CMDID to FW
 * @wmi_handle: wmi handle
 * @hw_mode_index: The HW_Mode field is a enumerated type that is selected
 * from the HW_Mode table, which is returned in the WMI_SERVICE_READY_EVENTID.
 *
 * Request HardWare (HW) Mode change to WLAN firmware
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_soc_set_hw_mode_cmd(wmi_unified_t wmi_handle,
					   uint32_t hw_mode_index);

/**
 * wmi_extract_hw_mode_resp() - function to extract HW mode change response
 * @wmi_hdl: WMI handle
 * @evt_buf: Buffer holding event data
 * @cmd_status: command status
 *
 * Return: QDF_STATUS_SUCCESS if success, else returns proper error code.
 */
QDF_STATUS
wmi_unified_extract_hw_mode_resp(wmi_unified_t wmi,
				 void *evt_buf,
				 uint32_t *cmd_status);

/**
 * wmi_unified_extract_roam_trigger_stats() - Extract roam trigger related
 * stats
 * @wmi:        wmi handle
 * @evt_buf:    Pointer to the event buffer
 * @trig:       Pointer to destination structure to fill data
 * @idx:        TLV id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_unified_extract_roam_trigger_stats(wmi_unified_t wmi, void *evt_buf,
				       struct wmi_roam_trigger_info *trig,
				       uint8_t idx);

/**
 * wmi_unified_extract_roam_scan_stats() - Extract roam scan stats from
 * firmware
 * @wmi:        wmi handle
 * @evt_buf:    Pointer to the event buffer
 * @dst:        Pointer to destination structure to fill data
 * @idx:        TLV id
 * @chan_idx:   Index of the channel frequency for this roam trigger
 * @ap_idx:     Index of the candidate AP for this roam trigger
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_unified_extract_roam_scan_stats(wmi_unified_t wmi, void *evt_buf,
				    struct wmi_roam_scan_data *dst, uint8_t idx,
				    uint8_t chan_idx, uint8_t ap_idx);

/**
 * wmi_unified_extract_roam_result_stats() - Extract roam result related stats
 * @wmi:        wmi handle
 * @evt_buf:    Pointer to the event buffer
 * @dst:        Pointer to destination structure to fill data
 * @idx:        TLV id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_unified_extract_roam_result_stats(wmi_unified_t wmi, void *evt_buf,
				      struct wmi_roam_result *dst,
				      uint8_t idx);

/**
 * wmi_unified_extract_roam_11kv_stats() - Extract BTM/Neigh report stats
 * @wmi:       wmi handle
 * @evt_buf:   Pointer to the event buffer
 * @dst:       Pointer to destination structure to fill data
 * @idx:       TLV id
 * @rpt_idx:   index of the current channel
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_unified_extract_roam_11kv_stats(wmi_unified_t wmi, void *evt_buf,
				    struct wmi_neighbor_report_data *dst,
				    uint8_t idx, uint8_t rpt_idx);

/**
 * wmi_unified_vdev_create_send() - send VDEV create command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold vdev create parameter
 * @macaddr: vdev mac address
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_create_send(wmi_unified_t wmi_handle,
					uint8_t macaddr[QDF_MAC_ADDR_SIZE],
					struct vdev_create_params *param);

/**
 * wmi_unified_vdev_delete_send() - send VDEV delete command to fw
 * @wmi_handle: wmi handle
 * @if_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_delete_send(wmi_unified_t wmi_handle,
					uint8_t if_id);

/**
 * wmi_unified_vdev_nss_chain_params_send() - send VDEV nss chain params to fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @nss_chains_user_cfg: user configured params to send
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_vdev_nss_chain_params_send(
		wmi_unified_t wmi_handle,
		uint8_t vdev_id,
		struct vdev_nss_chains *nss_chains_user_cfg);

/**
 * wmi_unified_vdev_stop_send() - send vdev stop command to fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_stop_send(wmi_unified_t wmi_handle,
				      uint8_t vdev_id);

/**
 * wmi_unified_vdev_up_send() - send vdev up command in fw
 * @wmi_handle: wmi handle
 * @bssid: bssid
 * @params: pointer to hold vdev up parameter
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_up_send(wmi_unified_t wmi_handle,
				    uint8_t bssid[QDF_MAC_ADDR_SIZE],
				    struct vdev_up_params *params);

/**
 * wmi_unified_vdev_down_send() - send vdev down command to fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_down_send(wmi_unified_t wmi_handle,
				      uint8_t vdev_id);

/**
 * wmi_unified_vdev_start_send() - send vdev start command to fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_start_send(wmi_unified_t wmi_handle,
				       struct vdev_start_params *req);

/**
 * wmi_unified_vdev_set_nac_rssi_send() - send NAC_RSSI command to fw
 * @wmi_handle: handle to WMI
 * @req: pointer to hold nac rssi request data
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_vdev_set_nac_rssi_send(wmi_unified_t wmi_handle,
				   struct vdev_scan_nac_rssi_params *req);

/**
 * wmi_unified_vdev_set_param_send() - WMI vdev set parameter function
 * @wmi_handle: handle to WMI.
 * @macaddr: MAC address
 * @param: pointer to hold vdev set parameter
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_vdev_set_param_send(wmi_unified_t wmi_handle,
				struct vdev_set_params *param);

/**
 * wmi_unified_sifs_trigger_send() - WMI vdev sifs trigger parameter function
 * @wmi_handle: handle to WMI.
 * @param: pointer to hold sifs trigger parameter
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_sifs_trigger_send(wmi_unified_t wmi_handle,
					 struct sifs_trigger_param *param);

/**
 * wmi_unified_peer_delete_send() - send PEER delete command to fw
 * @wmi_handle: wmi handle
 * @peer_addr: peer mac addr
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_peer_delete_send(wmi_unified_t wmi_handle,
			     uint8_t peer_addr[QDF_MAC_ADDR_SIZE],
			     uint8_t vdev_id);

/**
 * wmi_unified_peer_flush_tids_send() - flush peer tids packets in fw
 * @wmi_handle: wmi handle
 * @peer_addr: peer mac address
 * @param: pointer to hold peer flush tid parameter
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_peer_flush_tids_send(wmi_unified_t wmi_handle,
				 uint8_t peer_addr[QDF_MAC_ADDR_SIZE],
				 struct peer_flush_params *param);

/**
 * wmi_unified_peer_delete_all_send() - send PEER delete all command to fw
 * @wmi_hdl: wmi handle
 * @param: pointer to hold peer delete all parameters
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_peer_delete_all_send(
					wmi_unified_t wmi_hdl,
					struct peer_delete_all_params *param);

/**
 * wmi_set_peer_param() - set peer parameter in fw
 * @wmi_handle: wmi handle
 * @peer_addr: peer mac address
 * @param: pointer to hold peer set parameter
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_set_peer_param_send(wmi_unified_t wmi_handle,
			uint8_t peer_addr[QDF_MAC_ADDR_SIZE],
			struct peer_set_params *param);

/**
 * wmi_unified_peer_create_send() - send peer create command to fw
 * @wmi_handle: wmi handle
 * @peer_addr: peer mac address
 * @peer_type: peer type
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_peer_create_send(wmi_unified_t wmi_handle,
					struct peer_create_params *param);

/**
 * wmi_unified_green_ap_ps_send() - enable green ap powersave command
 * @wmi_handle: wmi handle
 * @value: value
 * @pdev_id: pdev id to have radio context
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_green_ap_ps_send(wmi_unified_t wmi_handle,
					uint32_t value, uint8_t pdev_id);

/**
 * wmi_unified_wow_enable_send() - WMI wow enable function
 * @wmi_handle: handle to WMI.
 * @param: pointer to hold wow enable parameter
 * @mac_id: radio context
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_wow_enable_send(wmi_unified_t wmi_handle,
				       struct wow_cmd_params *param,
				       uint8_t mac_id);

/**
 * wmi_unified_wow_wakeup_send() - WMI wow wakeup function
 * @wmi_handle: handle to WMI.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_wow_wakeup_send(wmi_unified_t wmi_handle);

/**
 * wmi_unified_wow_add_wakeup_event_send() - WMI wow wakeup function
 * @wmi_handle: handle to WMI.
 * @param: pointer to wow wakeup event parameter structure
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_wow_add_wakeup_event_send(wmi_unified_t wmi_handle,
				      struct wow_add_wakeup_params *param);

/**
 * wmi_unified_wow_add_wakeup_pattern_send() - WMI wow wakeup pattern function
 * @wmi_handle: handle to WMI.
 * @param: pointer to wow wakeup pattern parameter structure
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_wow_add_wakeup_pattern_send(
		wmi_unified_t wmi_handle,
		struct wow_add_wakeup_pattern_params *param);

/**
 * wmi_unified_wow_remove_wakeup_pattern_send() - wow wakeup pattern function
 * @wmi_handle: handle to WMI.
 * @param: pointer to wow wakeup pattern parameter structure
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_wow_remove_wakeup_pattern_send(
		wmi_unified_t wmi_handle,
		struct wow_remove_wakeup_pattern_params *param);

/**
 * wmi_unified_packet_log_enable_send() - WMI request stats function
 * @wmi_handle : handle to WMI.
 * @PKTLOG_EVENT : PKTLOG Event
 * @mac_id : MAC id corresponds to pdev id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_packet_log_enable_send(wmi_unified_t wmi_handle,
				   WMI_HOST_PKTLOG_EVENT PKTLOG_EVENT,
				   uint8_t mac_id);

/**
 * wmi_unified_peer_based_pktlog_send() - WMI request enable peer
 * based filtering
 * @wmi_handle: handle to WMI.
 * @macaddr: PEER mac address to be filtered
 * @mac_id: Mac id
 * @enb_dsb: Enable or Disable peer based pktlog
 *           filtering
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_peer_based_pktlog_send(wmi_unified_t wmi_handle,
					      uint8_t *macaddr,
					      uint8_t mac_id,
					      uint8_t enb_dsb);

/**
 * wmi_unified_packet_log_disable__send() - WMI pktlog disable function
 * @wmi_handle: handle to WMI.
 * @PKTLOG_EVENT: packet log event
 *
 * @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_packet_log_disable_send(wmi_unified_t wmi_handle,
					       uint8_t mac_id);

/**
 * wmi_unified_suspend_send() - WMI suspend function
 * @wmi_handle: handle to WMI.
 * @param: pointer to hold suspend parameter
 * @mac_id: radio context
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_suspend_send(wmi_unified_t wmi_handle,
				    struct suspend_params *param,
				    uint8_t mac_id);

/**
 * wmi_unified_resume_send - WMI resume function
 * @wmi_handle      : handle to WMI.
 * @mac_id: radio context
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_resume_send(wmi_unified_t wmi_handle,
				   uint8_t mac_id);

/**
 * wmi_unified_pdev_param_send() - set pdev parameters
 * @wmi_handle: wmi handle
 * @param: pointer to pdev parameter
 * @mac_id: radio context
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failures,
 *         errno on failure
 */
QDF_STATUS
wmi_unified_pdev_param_send(wmi_unified_t wmi_handle,
			    struct pdev_params *param,
			    uint8_t mac_id);

/**
 * wmi_unified_fd_tmpl_send_cmd() - WMI FILS Discovery send function
 * @wmi_handle: handle to WMI.
 * @param: pointer to hold FILS Discovery send cmd parameter
 *
 * @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_fd_tmpl_send_cmd(wmi_unified_t wmi_handle,
			     struct fils_discovery_tmpl_params *param);

/**
 * wmi_unified_beacon_tmpl_send_cmd() - WMI beacon send function
 * @wmi_handle: handle to WMI.
 * @macaddr: MAC address
 * @param: pointer to hold beacon send cmd parameter
 *
 * @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_beacon_tmpl_send_cmd(wmi_unified_t wmi_handle,
				 struct beacon_tmpl_params *param);

/**
 * wmi_unified_peer_assoc_send() - WMI peer assoc function
 * @wmi_handle: handle to WMI.
 * @macaddr: MAC address
 * @param: pointer to peer assoc parameter
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_peer_assoc_send(wmi_unified_t wmi_handle,
				       struct peer_assoc_params *param);

/**
 * wmi_unified_sta_ps_cmd_send() - set sta powersave parameters
 * @wmi_handle: wmi handle
 * @peer_addr: peer mac address
 * @param: pointer to sta_ps parameter structure
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_sta_ps_cmd_send(wmi_unified_t wmi_handle,
				       struct sta_ps_params *param);

/**
 * wmi_unified_ap_ps_cmd_send() - set ap powersave parameters
 * @wmi_handle: wmi handle
 * @peer_addr: peer mac address
 * @param: pointer to ap_ps parameter structure
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_ap_ps_cmd_send(wmi_unified_t wmi_handle,
				      uint8_t macaddr[QDF_MAC_ADDR_SIZE],
				      struct ap_ps_params *param);

/**
 * wmi_unified_scan_start_cmd_send() - WMI scan start function
 * @wmi_handle: handle to WMI.
 * @macaddr: MAC address
 * @param: pointer to hold scan start cmd parameter
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_scan_start_cmd_send(wmi_unified_t wmi_handle,
				struct scan_req_params *param);

/**
 * wmi_unified_scan_stop_cmd_send() - WMI scan start function
 * @wmi_handle: handle to WMI.
 * @macaddr: MAC address
 * @param: pointer to hold scan start cmd parameter
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_scan_stop_cmd_send(wmi_unified_t wmi_handle,
			       struct scan_cancel_param *param);

/**
 * wmi_unified_scan_chan_list_cmd_send() - WMI scan channel list function
 * @wmi_handle: handle to WMI.
 * @macaddr: MAC address
 * @param: pointer to hold scan channel list parameter
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_scan_chan_list_cmd_send(wmi_unified_t wmi_handle,
				    struct scan_chan_list_params *param);


/**
 * wmi_crash_inject() - inject fw crash
 * @wmi_handle: wmi handle
 * @param: ponirt to crash inject parameter structure
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_crash_inject(wmi_unified_t wmi_handle,
			    struct crash_inject *param);

/**
 * wmi_unified_pdev_utf_cmd() - send utf command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to pdev_utf_params
 * @mac_id: mac id to have radio context
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_pdev_utf_cmd_send(wmi_unified_t wmi_handle,
					 struct pdev_utf_params *param,
					 uint8_t mac_id);

/**
 * wmi_unified_dbglog_cmd_send() - set debug log level
 * @wmi_handle: handle to WMI.
 * @param: pointer to hold dbglog level parameter
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_dbglog_cmd_send(wmi_unified_t wmi_handle,
				       struct dbglog_params *param);

/**
 *  wmi_mgmt_unified_cmd_send() - management cmd over wmi layer
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold mgmt cmd parameter
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_mgmt_unified_cmd_send(wmi_unified_t wmi_handle,
				     struct wmi_mgmt_params *param);

/**
 *  wmi_offchan_data_tx_cmd_send() - Send offchan data tx cmd over wmi layer
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold offchan data cmd parameter
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_offchan_data_tx_cmd_send(
		wmi_unified_t wmi_handle,
		struct wmi_offchan_data_tx_params *param);

/**
 * wmi_unified_modem_power_state() - set modem power state to fw
 * @wmi_handle: wmi handle
 * @param_value: parameter value
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_modem_power_state(wmi_unified_t wmi_handle,
					 uint32_t param_value);

/**
 * wmi_unified_set_sta_ps_mode() - set sta powersave params in fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @val: value
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure.
 */
QDF_STATUS wmi_unified_set_sta_ps_mode(wmi_unified_t wmi_handle,
				       uint32_t vdev_id,
				       uint8_t val);

/**
 * wmi_unified_set_sta_uapsd_auto_trig_cmd() - set uapsd auto trigger command
 * @wmi_handle: wmi handle
 * @param: uapsd cmd parameter strcture
 *
 * This function sets the trigger
 * uapsd params such as service interval, delay interval
 * and suspend interval which will be used by the firmware
 * to send trigger frames periodically when there is no
 * traffic on the transmit side.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure.
 */
QDF_STATUS
wmi_unified_set_sta_uapsd_auto_trig_cmd(wmi_unified_t wmi_handle,
					struct sta_uapsd_trig_params *param);

/**
 * wmi_get_temperature() - get pdev temperature req
 * @wmi_handle: wmi handle
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure.
 */
QDF_STATUS wmi_unified_get_temperature(wmi_unified_t wmi_handle);

/**
 * wmi_set_smps_params() - set smps params
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @value: value
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure.
 */
QDF_STATUS wmi_unified_set_smps_params(wmi_unified_t wmi_handle,
				       uint8_t vdev_id,
				       int value);

/**
 * wmi_set_mimops() - set MIMO powersave
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @value: value
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure.
 */
QDF_STATUS wmi_unified_set_mimops(wmi_unified_t wmi_handle,
				  uint8_t vdev_id, int value);

/**
 * wmi_unified_lro_config_cmd() - process the LRO config command
 * @wmi_handle: Pointer to wmi handle
 * @wmi_lro_cmd: Pointer to LRO configuration parameters
 *
 * This function sends down the LRO configuration parameters to
 * the firmware to enable LRO, sets the TCP flags and sets the
 * seed values for the toeplitz hash generation
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_lro_config_cmd(wmi_unified_t wmi_handle,
			   struct wmi_lro_config_cmd_t *wmi_lro_cmd);

/**
 * wmi_unified_set_thermal_mgmt_cmd() - set thermal mgmt command to fw
 * @wmi_handle: Pointer to wmi handle
 * @thermal_info: Thermal command information
 *
 * This function sends the thermal management command
 * to the firmware
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_set_thermal_mgmt_cmd(wmi_unified_t wmi_handle,
				 struct thermal_cmd_params *thermal_info);

/**
 * wmi_unified_peer_rate_report_cmd() - process the peer rate report command
 * @wmi_handle: Pointer to wmi handle
 * @rate_report_params: Pointer to peer rate report parameters
 *
 *
 * Return: QDF_STATUS_SUCCESS for success otherwise failure
 */
QDF_STATUS
wmi_unified_peer_rate_report_cmd(
		wmi_unified_t wmi_handle,
		struct wmi_peer_rate_report_params *rate_report_params);

/**
 * wmi_unified_process_update_edca_param() - update EDCA params
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id.
 * @mu_edca_param: mu_edca_param.
 * @wmm_vparams: edca parameters
 *
 * This function updates EDCA parameters to the target
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_process_update_edca_param(
		wmi_unified_t wmi_handle,
		uint8_t vdev_id,
		bool mu_edca_param,
		struct wmi_host_wme_vparams wmm_vparams[WMI_MAX_NUM_AC]);

/**
 * wmi_unified_probe_rsp_tmpl_send_cmd() - send probe response template to fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @probe_rsp_info: probe response info
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_probe_rsp_tmpl_send_cmd(
		wmi_unified_t wmi_handle,
		uint8_t vdev_id,
		struct wmi_probe_resp_params *probe_rsp_info);

/**
 * wmi_unified_setup_install_key_cmd - send key to install to fw
 * @wmi_handle: wmi handle
 * @key_params: key parameters
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_setup_install_key_cmd(wmi_unified_t wmi_handle,
				  struct set_key_params *key_params);

/**
 * wmi_unified_get_pn_send_cmd() - send command to fw get PN for peer
 * @wmi_handle: wmi handle
 * @pn_params: PN parameters
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_get_pn_send_cmd(wmi_unified_t wmi_hdl,
			    struct peer_request_pn_param *pn_params);

/**
 * wmi_unified_p2p_go_set_beacon_ie_cmd() - set beacon IE for p2p go
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @p2p_ie: p2p IE
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_p2p_go_set_beacon_ie_cmd(wmi_unified_t wmi_hdl,
						uint32_t vdev_id,
						uint8_t *p2p_ie);

/**
 * wmi_unified_scan_probe_setoui_cmd() - set scan probe OUI
 * @wmi_handle: wmi handle
 * @psetoui: OUI parameters
 *
 * set scan probe OUI parameters in firmware
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_scan_probe_setoui_cmd(wmi_unified_t wmi_handle,
					     struct scan_mac_oui *psetoui);

#ifdef IPA_OFFLOAD
/** wmi_unified_ipa_offload_control_cmd() - ipa offload control parameter
 * @wmi_handle: wmi handle
 * @ipa_offload: ipa offload control parameter
 *
 * Returns: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failures,
 *          error number otherwise
 */
QDF_STATUS
wmi_unified_ipa_offload_control_cmd(
		wmi_unified_t wmi_handle,
		struct ipa_uc_offload_control_params *ipa_offload);
#endif

/**
 * wmi_unified_pno_stop_cmd() - PNO stop request
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 *
 * This function request FW to stop ongoing PNO operation.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_pno_stop_cmd(wmi_unified_t wmi_handle, uint8_t vdev_id);

#ifdef FEATURE_WLAN_SCAN_PNO
/**
 * wmi_unified_pno_start_cmd() - PNO start request
 * @wmi_handle: wmi handle
 * @pno: PNO request
 *
 * This function request FW to start PNO request.
 * Request: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_pno_start_cmd(wmi_unified_t wmi_handle,
				     struct pno_scan_req_params *pno);
#endif

/**
 * wmi_unified_nlo_mawc_cmd() - NLO MAWC cmd configuration
 * @wmi_handle: wmi handle
 * @params: Configuration parameters
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_nlo_mawc_cmd(wmi_unified_t wmi_handle,
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

#ifdef FEATURE_CLUB_LL_STATS_AND_GET_STATION
/**
 * wmi_process_unified_ll_stats_get_sta_cmd() - unified link layer stats and
 *                                              get station request
 * @wmi_handle: wmi handle
 * @get_req: unified ll stats and get station request command params
 * @is_always_over_qmi: flag to send stats request always over qmi
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_process_unified_ll_stats_get_sta_cmd(
				wmi_unified_t wmi_handle,
				const struct ll_stats_get_params *get_req,
				bool is_always_over_qmi);
#endif /* FEATURE_CLUB_LL_STATS_AND_GET_STATION */
#endif /* WLAN_FEATURE_LINK_LAYER_STATS */

/**
 * wmi_unified_congestion_request_cmd() - send request to fw to get CCA
 * @wmi_handle: wma handle
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_congestion_request_cmd(wmi_unified_t wmi_handle,
					      uint8_t vdev_id);

/**
 * wmi_unified_snr_request_cmd() - send request to fw to get RSSI stats
 * @wmi_handle: wmi handle
 * @rssi_req: get RSSI request
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_snr_request_cmd(wmi_unified_t wmi_handle);

/**
 * wmi_unified_snr_cmd() - get RSSI from fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_snr_cmd(wmi_unified_t wmi_handle, uint8_t vdev_id);

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
/**
 * wmi_unified_egap_conf_params_cmd() - send wmi cmd of egap config params
 * @wmi_handle: wmi handler
 * @egap_params: pointer to egap_params
 *
 * Return:	 0 for success, otherwise appropriate error code
 */
QDF_STATUS
wmi_unified_egap_conf_params_cmd(
		wmi_unified_t wmi_handle,
		struct wlan_green_ap_egap_params *egap_params);
#endif

/**
 * wmi_unified_csa_offload_enable() - send CSA offload enable command
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_csa_offload_enable(wmi_unified_t wmi_handle,
					  uint8_t vdev_id);

#ifdef WLAN_FEATURE_CIF_CFR
/**
 * wmi_unified_oem_dma_ring_cfg() - configure OEM DMA rings
 * @wmi_handle: wmi handle
 * @data_len: len of dma cfg req
 * @data: dma cfg req
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_oem_dma_ring_cfg(wmi_unified_t wmi_handle,
			     wmi_oem_dma_ring_cfg_req_fixed_param *cfg);
#endif

/**
 * wmi_unified_start_oem_data_cmd() - start oem data request to target
 * @wmi_handle: wmi handle
 * @data_len: the length of @data
 * @data: the pointer to data buf
 *
 * This is legacy api for oem data request, using wmi command
 * WMI_OEM_REQ_CMDID.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_start_oem_data_cmd(wmi_unified_t wmi_handle,
					  uint32_t data_len,
					  uint8_t *data);

#ifdef FEATURE_OEM_DATA
/**
 * wmi_unified_start_oemv2_data_cmd() - start oem data cmd to target
 * @wmi_handle: wmi handle
 * @params: oem data params
 *
 * This is common api for oem data, using wmi command WMI_OEM_DATA_CMDID.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_start_oemv2_data_cmd(wmi_unified_t wmi_handle,
					    struct oem_data *params);
#endif

/**
 * wmi_unified_dfs_phyerr_filter_offload_en_cmd() - enable dfs phyerr filter
 * @wmi_handle: wmi handle
 * @dfs_phyerr_filter_offload: is dfs phyerr filter offload
 *
 * Send WMI_DFS_PHYERR_FILTER_ENA_CMDID or
 * WMI_DFS_PHYERR_FILTER_DIS_CMDID command
 * to firmware based on phyerr filtering
 * offload status.
 *
 * Return: 1 success, 0 failure
 */
QDF_STATUS
wmi_unified_dfs_phyerr_filter_offload_en_cmd(wmi_unified_t wmi_handle,
					     bool dfs_phyerr_filter_offload);

#if !defined(REMOVE_PKT_LOG) && defined(FEATURE_PKTLOG)
/**
 * wmi_unified_pktlog_wmi_send_cmd() - send pktlog event command to target
 * @wmi_handle: wmi handle
 * @pktlog_event: pktlog event
 * @cmd_id: pktlog cmd id
 * @user_triggered: user triggered input for PKTLOG enable mode
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_pktlog_wmi_send_cmd(wmi_unified_t wmi_handle,
					   WMI_PKTLOG_EVENT pktlog_event,
					   uint32_t cmd_id,
					   uint8_t user_triggered);
#endif

/**
 * wmi_unified_stats_ext_req_cmd() - request ext stats from fw
 * @wmi_handle: wmi handle
 * @preq: stats ext params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_stats_ext_req_cmd(wmi_unified_t wmi_handle,
					 struct stats_ext_params *preq);

/**
 * wmi_unified_process_dhcpserver_offload_cmd() - enable DHCP server offload
 * @wmi_handle: wmi handle
 * @pDhcpSrvOffloadInfo: DHCP server offload info
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_process_dhcpserver_offload_cmd(
		wmi_unified_t wmi_handle,
		struct dhcp_offload_info_params *params);

/**
 * wmi_unified_send_regdomain_info_to_fw_cmd() - send regdomain info to fw
 * @wmi_handle: wmi handle
 * @reg_dmn: reg domain
 * @regdmn2G: 2G reg domain
 * @regdmn5G: 5G reg domain
 * @ctl2G: 2G test limit
 * @ctl5G: 5G test limit
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_send_regdomain_info_to_fw_cmd(wmi_unified_t wmi_handle,
						     uint32_t reg_dmn,
						     uint16_t regdmn2G,
						     uint16_t regdmn5G,
						     uint8_t ctl2G,
						     uint8_t ctl5G);

QDF_STATUS
wmi_unified_process_fw_mem_dump_cmd(wmi_unified_t wmi_hdl,
				    struct fw_dump_req_param *mem_dump_req);

/**
 * wmi_unified_cfg_action_frm_tb_ppdu_cmd()-send action frame TB PPDU cfg to FW
 * @wmi_handle: Pointer to WMi handle
 * @cfg_info: Pointer to cfg msg
 *
 * This function sends action frame TB PPDU cfg to firmware
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 *
 */
QDF_STATUS
wmi_unified_cfg_action_frm_tb_ppdu_cmd(
		wmi_unified_t wmi_handle,
		struct cfg_action_frm_tb_ppdu_param *cfg_info);

/**
 * wmi_unified_save_fw_version_cmd() - save fw version
 * @wmi_handle: pointer to wmi handle
 * @evt_buf: Event buffer
 *
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 *
 */
QDF_STATUS wmi_unified_save_fw_version_cmd(wmi_unified_t wmi_handle,
					   void *evt_buf);

/**
 * wmi_unified_log_supported_evt_cmd() - Enable/Disable FW diag/log events
 * @wmi_handle: wmi handle
 * @event: Event received from FW
 * @len: Length of the event
 *
 * Enables the low frequency events and disables the high frequency
 * events. Bit 17 indicates if the event if low/high frequency.
 * 1 - high frequency, 0 - low frequency
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failures
 */
QDF_STATUS wmi_unified_log_supported_evt_cmd(wmi_unified_t wmi_handle,
					     uint8_t *event,
					     uint32_t len);

/**
 * wmi_unified_enable_specific_fw_logs_cmd() - Start/Stop logging of diag log id
 * @wmi_handle: wmi handle
 * @start_log: Start logging related parameters
 *
 * Send the command to the FW based on which specific logging of diag
 * event/log id can be started/stopped
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_enable_specific_fw_logs_cmd(wmi_unified_t wmi_handle,
					struct wmi_wifi_start_log *start_log);

/**
 * wmi_unified_flush_logs_to_fw_cmd() - Send log flush command to FW
 * @wmi_handle: WMI handle
 *
 * This function is used to send the flush command to the FW,
 * that will flush the fw logs that are residue in the FW
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_flush_logs_to_fw_cmd(wmi_unified_t wmi_handle);

/**
 * wmi_unified_unit_test_cmd() - send unit test command to fw.
 * @wmi_handle: wmi handle
 * @wmi_utest: unit test command
 *
 * This function send unit test command to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_unit_test_cmd(wmi_unified_t wmi_handle,
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

/**
 * wmi_send_get_user_position_cmd() - send get user position command to fw
 * @wmi_handle: wmi handle
 * @value: user pos value
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_send_get_user_position_cmd(wmi_unified_t wmi_handle, uint32_t value);

/**
 * wmi_send_get_peer_mumimo_tx_count_cmd() - send get mumio tx count
 *                                           command to fw
 * @wmi_handle: wmi handle
 * @value: user pos value
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_send_get_peer_mumimo_tx_count_cmd(wmi_unified_t wmi_handle,
				      uint32_t value);

/**
 * wmi_send_reset_peer_mumimo_tx_count_cmd() - send reset peer mumimo
 *                                             tx count to fw
 * @wmi_handle: wmi handle
 * @value: reset tx count value
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_send_reset_peer_mumimo_tx_count_cmd(wmi_unified_t wmi_handle,
					uint32_t value);

/*
 * wmi_unified_send_btcoex_wlan_priority_cmd() - send btcoex priority commands
 * @wmi_handle: wmi handle
 * @param: wmi btcoex cfg params
 *
 * Send WMI_BTCOEX_CFG_CMDID parameters to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS
wmi_unified_send_btcoex_wlan_priority_cmd(wmi_unified_t wmi_handle,
					  struct btcoex_cfg_params *param);

/**
 *  wmi_unified_send_btcoex_duty_cycle_cmd() - send btcoex duty cycle commands
 * @wmi_handle: wmi handle
 * @param: wmi btcoex cfg params
 *
 * Send WMI_BTCOEX_CFG_CMDID parameters to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS
wmi_unified_send_btcoex_duty_cycle_cmd(wmi_unified_t wmi_handle,
				       struct btcoex_cfg_params *param);

/**
 * wmi_unified_send_coex_ver_cfg_cmd() - send coex ver cfg command
 * @wmi_handle: wmi handle
 * @param: wmi coex ver cfg params
 *
 * Send WMI_COEX_VERSION_CFG_CMID parameters to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS
wmi_unified_send_coex_ver_cfg_cmd(wmi_unified_t wmi_handle,
				  coex_ver_cfg_t *param);

/**
 * wmi_unified_send_coex_config_cmd() - send coex ver cfg command
 * @wmi_handle: wmi handle
 * @param: wmi coex cfg cmd params
 *
 * Send WMI_COEX_CFG_CMD parameters to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS
wmi_unified_send_coex_config_cmd(wmi_unified_t wmi_handle,
				 struct coex_config_params *param);

/**
 *  wmi_unified_pdev_fips_cmd_send() - WMI pdev fips cmd function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold pdev fips param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_pdev_fips_cmd_send(wmi_unified_t wmi_handle,
			       struct fips_params *param);

#ifdef WLAN_FEATURE_DISA
/**
 * wmi_unified_encrypt_decrypt_send_cmd() - send encryptdecrypt cmd to fw
 * @wmi_handle: wmi handle
 * @params: encrypt/decrypt params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_encrypt_decrypt_send_cmd(void *wmi_hdl,
				     struct disa_encrypt_decrypt_req_params
				     *params);
#endif /* WLAN_FEATURE_DISA */

/**
 *  wmi_unified_wlan_profile_enable_cmd_send() - WMI wlan profile enable
 *						 cmd function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold wlan profile param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_wlan_profile_enable_cmd_send(wmi_unified_t wmi_handle,
					 struct wlan_profile_params *param);

/**
 *  wmi_unified_wlan_profile_trigger_cmd_send() - WMI wlan profile trigger
 *						  cmd function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold wlan profile param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_wlan_profile_trigger_cmd_send(wmi_unified_t wmi_handle,
					  struct wlan_profile_params *param);

/**
 *  wmi_unified_wlan_profile_hist_intvl_cmd_send() - WMI wlan profile history
 *						  cmd function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold wlan profile param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_wlan_profile_hist_intvl_cmd_send(wmi_unified_t wmi_handle,
					     struct wlan_profile_params *param);

/**
 *  wmi_unified_set_chan_cmd_send() - WMI set channel cmd function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold channel param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_set_chan_cmd_send(wmi_unified_t wmi_handle,
			      struct channel_param *param);

/**
 *  wmi_unified_set_ratepwr_table_cmd_send() - WMI ratepwr table cmd function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold ratepwr table param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_set_ratepwr_table_cmd_send(wmi_unified_t wmi_handle,
				       struct ratepwr_table_params *param);

/**
 *  wmi_unified_get_ratepwr_table_cmd_send() - WMI ratepwr table cmd function
 *  @wmi_handle: handle to WMI.
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_get_ratepwr_table_cmd_send(wmi_unified_t wmi_handle);

/**
 *  wmi_unified_set_ratepwr_chainmsk_cmd_send() - WMI ratepwr
 *  chainmsk cmd function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold ratepwr chainmsk param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_set_ratepwr_chainmsk_cmd_send(wmi_unified_t wmi_handle,
					  struct ratepwr_chainmsk_params
					  *param);

/**
 *  wmi_unified_set_macaddr_cmd_send() - WMI set macaddr cmd function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold macaddr param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_macaddr_cmd_send(wmi_unified_t wmi_handle,
					    struct macaddr_params *param);

/**
 *  wmi_unified_pdev_scan_start_cmd_send() - WMI pdev scan start cmd function
 *  @wmi_handle: handle to WMI.
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_pdev_scan_start_cmd_send(wmi_unified_t wmi_handle);

/**
 *  wmi_unified_pdev_scan_end_cmd_send() - WMI pdev scan end cmd function
 *  @wmi_handle: handle to WMI.
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_pdev_scan_end_cmd_send(wmi_unified_t wmi_handle);

/**
 *  wmi_unified_set_acparams_cmd_send() - WMI set acparams cmd function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold acparams param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_set_acparams_cmd_send(wmi_unified_t wmi_handle,
				  struct acparams_params *param);

/**
 *  wmi_unified_set_vap_dscp_tid_map_cmd_send() - WMI set vap dscp
 *  tid map cmd function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold dscp param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_set_vap_dscp_tid_map_cmd_send(
			wmi_unified_t wmi_handle,
			struct vap_dscp_tid_map_params *param);

/**
 *  wmi_unified_proxy_ast_reserve_cmd_send() - WMI proxy ast
 *  reserve cmd function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold ast param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_proxy_ast_reserve_cmd_send(wmi_unified_t wmi_handle,
				       struct proxy_ast_reserve_params *param);

/**
 *  wmi_unified_set_bridge_mac_addr_cmd_send() - WMI set bridge mac
 *  addr cmd function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold bridge mac addr param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_bridge_mac_addr_cmd_send(
			wmi_unified_t wmi_handle,
			struct set_bridge_mac_addr_params *param);

/**
 *  wmi_unified_phyerr_enable_cmd_send() - WMI phyerr enable cmd function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold phyerr enable param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_phyerr_enable_cmd_send(wmi_unified_t wmi_handle);

/**
 *  wmi_unified_phyerr_disable_cmd_send() - WMI phyerr disable cmd function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold phyerr disable param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_phyerr_disable_cmd_send(wmi_unified_t wmi_handle);

/**
 *  wmi_unified_smart_ant_enable_tx_feedback_cmd_send() -
 *					WMI set tx antenna function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold antenna param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_smart_ant_enable_tx_feedback_cmd_send(
			wmi_unified_t wmi_handle,
			struct smart_ant_enable_tx_feedback_params *param);

#ifdef WLAN_IOT_SIM_SUPPORT
/**
 *  wmi_unified_simulation_test_cmd_send() -
 *  WMI simulation test command
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold simulation test param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_simulation_test_cmd_send(wmi_unified_t wmi_handle,
						struct simulation_test_params
						*param);
#endif

/**
 *  wmi_unified_vdev_spectral_configure_cmd_send() -
 *					WMI set spectral config function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold spectral config param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_spectral_configure_cmd_send(
			wmi_unified_t wmi_handle,
			struct vdev_spectral_configure_params *param);

/**
 *  wmi_unified_vdev_spectral_enable_cmd_send() - WMI enable spectral function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold enable spectral param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_spectral_enable_cmd_send(
			wmi_unified_t wmi_handle,
			struct vdev_spectral_enable_params *param);

#ifdef WLAN_CONV_SPECTRAL_ENABLE
/**
 *  wmi_extract_pdev_sscan_fw_cmd_fixed_param() - Extract fixed params
 *  from start scan response event
 *  @wmi_handle: handle to WMI.
 *  @evt_buf: Event buffer
 *  @param: pointer to hold fixed params from fw params event
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_pdev_sscan_fw_cmd_fixed_param(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf,
			struct spectral_startscan_resp_params *param);

/**
 *  wmi_extract_pdev_sscan_fft_bin_index() - Extract FFT bin indexes
 *  from start scan response event
 *  @wmi_handle: handle to WMI.
 *  @evt_buf: Event buffer
 *  @param: pointer to hold FFT bin indexes from fw params event
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_pdev_sscan_fft_bin_index(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf,
			struct spectral_fft_bin_markers_160_165mhz *param);
#endif /* WLAN_CONV_SPECTRAL_ENABLE */

#if defined(WLAN_SUPPORT_FILS) || defined(CONFIG_BAND_6GHZ)
/**
 *  wmi_unified_vdev_fils_enable_cmd_send() - WMI send fils enable command
 *  @param wmi_handle: handle to WMI.
 *  @param config_fils_params: fils enable parameters
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_vdev_fils_enable_cmd_send(struct wmi_unified *wmi_handle,
				      struct config_fils_params *param);
#endif

/**
 *  wmi_unified_bss_chan_info_request_cmd_send() - WMI bss chan info
 *  request function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold chan info param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_bss_chan_info_request_cmd_send(
			wmi_unified_t wmi_handle,
			struct bss_chan_info_request_params *param);

/**
 *  wmi_unified_thermal_mitigation_param_cmd_send() -
 *					WMI thermal mitigation function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold thermal mitigation param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_thermal_mitigation_param_cmd_send(
			wmi_unified_t wmi_handle,
			struct thermal_mitigation_params *param);

/**
 *  wmi_unified_vdev_set_fwtest_param_cmd_send() - WMI set fwtest function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold fwtest param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_set_fwtest_param_cmd_send(
			wmi_unified_t wmi_handle,
			struct set_fwtest_params *param);

/**
 *  wmi_unified_vdev_set_custom_aggr_size_cmd_send() - WMI set custom aggr
 *						       size command
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold custom aggr size param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_set_custom_aggr_size_cmd_send(
		wmi_unified_t wmi_handle,
		struct set_custom_aggr_size_params *param);

/**
 *  wmi_unified_vdev_set_qdepth_thresh_cmd_send() - WMI set qdepth threshold
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold set qdepth thresh param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_set_qdepth_thresh_cmd_send(
		wmi_unified_t wmi_handle,
		struct set_qdepth_thresh_params *param);

/**
 *  wmi_unified_pdev_set_regdomain_params_cmd_send() - WMI set regdomain
 *  function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold regdomain param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_pdev_set_regdomain_cmd_send(
			wmi_unified_t wmi_handle,
			struct pdev_set_regdomain_params *param);

/**
 *  wmi_unified_set_beacon_filter_cmd_send() - WMI set beacon filter function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold beacon filter param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_beacon_filter_cmd_send(
			wmi_unified_t wmi_handle,
			struct set_beacon_filter_params *param);

/**
 *  wmi_unified_remove_beacon_filter_cmd_send() - WMI set beacon filter function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold beacon filter param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_remove_beacon_filter_cmd_send(
			wmi_unified_t wmi_handle,
			struct remove_beacon_filter_params *param);

/**
 *  wmi_unified_addba_clearresponse_cmd_send() - WMI addba resp cmd function
 *  @wmi_handle: handle to WMI.
 *  @macaddr: MAC address
 *  @param: pointer to hold addba resp parameter
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_addba_clearresponse_cmd_send(
			wmi_unified_t wmi_handle,
			uint8_t macaddr[QDF_MAC_ADDR_SIZE],
			struct addba_clearresponse_params *param);

/**
 *  wmi_unified_addba_send_cmd_send() - WMI addba send function
 *  @wmi_handle: handle to WMI.
 *  @macaddr: MAC address
 *  @param: pointer to hold addba parameter
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_addba_send_cmd_send(wmi_unified_t wmi_handle,
				uint8_t macaddr[QDF_MAC_ADDR_SIZE],
				struct addba_send_params *param);

/**
 *  wmi_unified_delba_send_cmd_send() - WMI delba cmd function
 *  @wmi_handle: handle to WMI.
 *  @macaddr: MAC address
 *  @param: pointer to hold delba parameter
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_delba_send_cmd_send(wmi_unified_t wmi_handle,
				uint8_t macaddr[QDF_MAC_ADDR_SIZE],
				struct delba_send_params *param);

/**
 *  wmi_unified_addba_setresponse_cmd_send() - WMI addba set resp cmd function
 *  @wmi_handle: handle to WMI.
 *  @macaddr: MAC address
 *  @param: pointer to hold addba set resp parameter
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_addba_setresponse_cmd_send(wmi_unified_t wmi_handle,
				       uint8_t macaddr[QDF_MAC_ADDR_SIZE],
				       struct addba_setresponse_params *param);

/**
 *  wmi_unified_singleamsdu_cmd_send() - WMI singleamsdu function
 *  @wmi_handle: handle to WMI.
 *  @macaddr: MAC address
 *  @param: pointer to hold singleamsdu parameter
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_singleamsdu_cmd_send(wmi_unified_t wmi_handle,
				 uint8_t macaddr[QDF_MAC_ADDR_SIZE],
				 struct singleamsdu_params *param);

/**
 *  wmi_unified_mu_scan_cmd_send() - WMI set mu scan function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold mu scan param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_mu_scan_cmd_send(wmi_unified_t wmi_handle,
			     struct mu_scan_params *param);

/**
 *  wmi_unified_lteu_config_cmd_send() - WMI set mu scan function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold mu scan param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_lteu_config_cmd_send(wmi_unified_t wmi_handle,
				 struct lteu_config_params *param);

/**
 *  wmi_unified_set_psmode_cmd_send() - WMI set mu scan function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold mu scan param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_set_psmode_cmd_send(wmi_unified_t wmi_handle,
				struct set_ps_mode_params *param);

/**
 * wmi_unified_init_cmd_send() - send initialization cmd to fw
 * @wmi_handle: wmi handle
 * @param: pointer to wmi init param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_init_cmd_send(wmi_unified_t wmi_handle,
			  struct wmi_init_cmd_param *param);

/**
 * wmi_service_enabled() - Check if service enabled
 * @wmi_handle: wmi handle
 * @service_id: service identifier
 *
 * Return: 1 enabled, 0 disabled
 */
bool wmi_service_enabled(wmi_unified_t wmi_handle, uint32_t service_id);

/**
 * wmi_save_service_bitmap() - save service bitmap
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @bitmap_buf: bitmap buffer
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS failure code
 */
QDF_STATUS wmi_save_service_bitmap(wmi_unified_t wmi_handle, void *evt_buf,
				   void *bitmap_buf);

/**
 * wmi_save_ext_service_bitmap() - save extended service bitmap
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS failure code
 */
QDF_STATUS wmi_save_ext_service_bitmap(wmi_unified_t wmi_handle, void *evt_buf,
				       void *bitmap_buf);

/**
 * wmi_save_fw_version() - Save fw version
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_save_fw_version(wmi_unified_t wmi_handle, void *evt_buf);

/**
 * wmi_get_target_cap_from_service_ready() - extract service ready event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to received event buffer
 * @ev: pointer to hold target capability information extracted from even
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_get_target_cap_from_service_ready(
		wmi_unified_t wmi_handle, void *evt_buf,
		struct wlan_psoc_target_capability_info *ev);

/**
 * wmi_extract_hal_reg_cap() - extract HAL registered capabilities
 * @wmi_handle: wmi handle
 * @evt_buf: Pointer to event buffer
 * @hal_reg_cap: pointer to hold HAL reg capabilities
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_hal_reg_cap(wmi_unified_t wmi_handle, void *evt_buf,
			struct wlan_psoc_hal_reg_capability *hal_reg_cap);

/**
 * wmi_extract_hal_reg_cap_ext2() - Extract HAL reg capabilities from service
 * ready ext2 event
 * @wmi_handle: wmi handle
 * @evt_buf: Pointer to event buffer
 * @phy_idx: Phy id
 * @wireless_modes: 11AX wireless modes
 * @hal_reg_cap: pointer to hold HAL reg capabilities ext2 structure
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_hal_reg_cap_ext2(
		wmi_unified_t wmi_handle, void *evt_buf, uint8_t phy_idx,
		struct wlan_psoc_host_hal_reg_capabilities_ext2 *hal_reg_cap);

/**
 * wmi_extract_num_mem_reqs_from_service_ready() - Extract number of memory
 *                                                 entries requested
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 *
 * Return: Number of entries requested
 */
uint32_t wmi_extract_num_mem_reqs_from_service_ready(
		wmi_unified_t wmi_handle,
		void *evt_buf);

/**
 * wmi_extract_host_mem_req_from_service_ready() - Extract host memory
 *                                                 request event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @mem_reqs: pointer to host memory request structure
 * @num_active_peers: number of active peers for peer cache
 * @num_peers: number of peers
 * @fw_prio: FW priority
 * @idx: Index for memory request
 *
 * Return: Host memory request parameters requested by target
 */
QDF_STATUS wmi_extract_host_mem_req_from_service_ready(
		wmi_unified_t wmi_handle, void *evt_buf, host_mem_req *mem_reqs,
		uint32_t num_active_peers, uint32_t num_peers,
		enum wmi_fw_mem_prio fw_prio, uint16_t idx);

/**
 * wmi_ready_extract_init_status() - Extract init status from ready event
 * @wmi_handle: wmi handle
 * @ev: Pointer to event buffer
 *
 * Return: ready status
 */
uint32_t wmi_ready_extract_init_status(wmi_unified_t wmi_handle, void *ev);

/**
 * wmi_ready_extract_mac_addr() - extract mac address from ready event
 * @wmi_handle: wmi handle
 * @ev: pointer to event buffer
 * @macaddr: Pointer to hold MAC address
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_ready_extract_mac_addr(wmi_unified_t wmi_handle,
				      void *ev, uint8_t *macaddr);

/**
 * wmi_ready_extract_mac_addr() - extract MAC address list from ready event
 * @wmi_handle: wmi handle
 * @ev: pointer to event buffer
 * @num_mac_addr: Pointer to number of entries
 *
 * Return: address to start of mac addr list
 */
wmi_host_mac_addr
*wmi_ready_extract_mac_addr_list(wmi_unified_t wmi_handle, void *ev,
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
QDF_STATUS wmi_extract_ready_event_params(
		wmi_unified_t wmi_handle, void *evt_buf,
		struct wmi_host_ready_ev_param *ev_param);

/**
 * wmi_extract_fw_version() - extract fw version
 * @wmi_handle: wmi handle
 * @ev: pointer to event buffer
 * @fw_ver: Pointer to hold fw version
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_fw_version(wmi_unified_t wmi_handle, void *ev,
				  struct wmi_host_fw_ver *fw_ver);

/**
 * wmi_extract_fw_abi_version() - extract fw abi version
 * @wmi_handle: wmi handle
 * @ev: Pointer to event buffer
 * @fw_ver: Pointer to hold fw abi version
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_fw_abi_version(wmi_unified_t wmi_handle, void *ev,
				      struct wmi_host_fw_abi_ver *fw_ver);

/**
 * wmi_check_and_update_fw_version() - Ready and fw version check
 * @wmi_handle: wmi handle
 * @ev: pointer to event buffer
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_check_and_update_fw_version(wmi_unified_t wmi_handle, void *ev);

/**
 * wmi_extract_dbglog_data_len() - extract debuglog data length
 * @wmi_handle: wmi handle
 * @evt_b: pointer to event buffer
 * @len:  length of buffer
 *
 * Return: length
 */
uint8_t *wmi_extract_dbglog_data_len(wmi_unified_t wmi_handle,
				     void *evt_b, uint32_t *len);

/**
 * wmi_send_ext_resource_config() - send extended resource configuration
 * @wmi_handle: wmi handle
 * @ext_cfg: pointer to extended resource configuration
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_send_ext_resource_config(wmi_unified_t wmi_handle,
					wmi_host_ext_resource_config *ext_cfg);

/**
 *  wmi_unified_rtt_meas_req_test_cmd_send() - WMI rtt meas req test function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold rtt meas req test param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_rtt_meas_req_test_cmd_send(wmi_unified_t wmi_handle,
				       struct rtt_meas_req_test_params *param);

/**
 *  wmi_unified_rtt_meas_req_cmd_send() - WMI rtt meas req function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold rtt meas req param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_rtt_meas_req_cmd_send(wmi_unified_t wmi_handle,
				  struct rtt_meas_req_params *param);

/**
 *  wmi_unified_rtt_keepalive_req_cmd_send() - WMI rtt meas req test function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold rtt meas req test param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_rtt_keepalive_req_cmd_send(wmi_unified_t wmi_handle,
				       struct rtt_keepalive_req_params *param);

/**
 *  wmi_unified_lci_set_cmd_send() - WMI lci set function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold lci param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_lci_set_cmd_send(wmi_unified_t wmi_handle,
					struct lci_set_params *param);

/**
 *  wmi_unified_lcr_set_cmd_send() - WMI lcr set function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold lcr param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_lcr_set_cmd_send(wmi_unified_t wmi_handle,
					struct lcr_set_params *param);

/**
 * wmi_unified_extract_pn() - extract pn event data
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @param: pointer to get pn event param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_extract_pn(wmi_unified_t wmi_hdl, void *evt_buf,
			  struct wmi_host_get_pn_event *param);

/**
 * wmi_unified_send_periodic_chan_stats_config_cmd() - send periodic chan
 * stats cmd to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold periodic chan stats param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_send_periodic_chan_stats_config_cmd(
		wmi_unified_t wmi_handle,
		struct periodic_chan_stats_params *param);

/* Extract APIs */

/**
 * wmi_extract_fips_event_data() - extract fips event data
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @param: pointer to FIPS event param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_fips_event_data(wmi_unified_t wmi_handle, void *evt_buf,
			    struct wmi_host_fips_event_param *param);

#ifdef WLAN_FEATURE_DISA
/**
 * wmi_extract_encrypt_decrypt_resp_params() -
 *       extract encrypt decrypt resp params from event buffer
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @resp: encrypt decrypt resp params
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS
wmi_extract_encrypt_decrypt_resp_params(void *wmi_hdl, void *evt_buf,
					struct disa_encrypt_decrypt_resp_params
					*param);
#endif /* WLAN_FEATURE_DISA */

/**
 * wmi_extract_mgmt_rx_params() - extract management rx params from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @hdr: Pointer to hold header
 * @bufp: Pointer to hold pointer to rx param buffer
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_mgmt_rx_params(wmi_unified_t wmi_handle, void *evt_buf,
			   struct mgmt_rx_event_params *hdr, uint8_t **bufp);

/**
 * wmi_extract_vdev_roam_param() - extract vdev roam param from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @ev: Pointer to hold roam param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_vdev_roam_param(wmi_unified_t wmi_handle, void *evt_buf,
			    wmi_host_roam_event *ev);

/**
 * wmi_extract_vdev_scan_ev_param() - extract vdev scan param from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @param: Pointer to hold vdev scan param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_vdev_scan_ev_param(wmi_unified_t wmi_handle, void *evt_buf,
			       struct scan_event *param);

#ifdef FEATURE_WLAN_SCAN_PNO
/**
 * wmi_extract_nlo_match_ev_param() - extract NLO match param from event
 * @wmi_handle: pointer to WMI handle
 * @evt_buf: pointer to WMI event buffer
 * @param: pointer to scan event param for NLO match
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS
wmi_extract_nlo_match_ev_param(wmi_unified_t wmi_handle, void *evt_buf,
			       struct scan_event *param);

/**
 * wmi_extract_nlo_complete_ev_param() - extract NLO complete param from event
 * @wmi_handle: pointer to WMI handle
 * @evt_buf: pointer to WMI event buffer
 * @param: pointer to scan event param for NLO complete
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS
wmi_extract_nlo_complete_ev_param(wmi_unified_t wmi_handle, void *evt_buf,
				  struct scan_event *param);
#endif

/**
 * wmi_extract_mu_ev_param() - extract mu param from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @param: Pointer to hold mu report
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_mu_ev_param(wmi_unified_t wmi_handle, void *evt_buf,
			wmi_host_mu_report_event *param);

/**
 * wmi_extract_mu_db_entry() - extract mu db entry from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @idx: index
 * @param: Pointer to hold mu db entry
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_mu_db_entry(wmi_unified_t wmi_handle, void *evt_buf,
			uint8_t idx, wmi_host_mu_db_entry *param);

/**
 * wmi_extract_mumimo_tx_count_ev_param() - extract mumimo tx count from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @param: Pointer to hold mumimo tx count
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_mumimo_tx_count_ev_param(wmi_unified_t wmi_handle, void *evt_buf,
				     wmi_host_peer_txmu_cnt_event *param);

/**
 * wmi_extract_peer_gid_userpos_list_ev_param() - extract peer userpos list
 *                                                from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @param: Pointer to hold peer gid userposition list
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_peer_gid_userpos_list_ev_param(
		wmi_unified_t wmi_handle,
		void *evt_buf,
		wmi_host_peer_gid_userpos_list_event *param);

/**
 * wmi_extract_esp_estimate_ev_param() - extract air time from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @param: Pointer to hold esp event
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_esp_estimate_ev_param(wmi_unified_t wmi_handle, void *evt_buf,
				  struct esp_estimation_event *param);

/**
 * wmi_extract_gpio_input_ev_param() - extract gpio input param from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @gpio_num: Pointer to hold gpio number
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_gpio_input_ev_param(wmi_unified_t wmi_handle,
					   void *evt_buf, uint32_t *gpio_num);

/**
 * wmi_extract_pdev_reserve_ast_ev_param() - extract reserve ast entry
 * param from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @param: Pointer to hold reserve ast entry param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_pdev_reserve_ast_ev_param(
		wmi_unified_t wmi_handle, void *evt_buf,
		struct wmi_host_proxy_ast_reserve_param *param);
/**
 * wmi_extract_pdev_generic_buffer_ev_param() - extract pdev generic buffer
 * from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @param: Pointer to generic buffer param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_pdev_generic_buffer_ev_param(
		wmi_unified_t wmi_handle, void *evt_buf,
		wmi_host_pdev_generic_buffer_event *param);

/**
 * wmi_extract_peer_ratecode_list_ev() - extract peer ratecode from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @peer_mac: Pointer to hold peer mac address
 * @pdev_id: Pointer to hold pdev_id
 * @rate_cap: Pointer to hold ratecode
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_peer_ratecode_list_ev(
		wmi_unified_t wmi_handle, void *evt_buf,
		uint8_t *peer_mac, uint32_t *pdev_id,
		wmi_sa_rate_cap *rate_cap);

/**
 * wmi_extract_bcnflt_stats() - extract bcn fault stats from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @index: Index into bcn fault stats
 * @bcnflt_stats: Pointer to hold bcn fault stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_bcnflt_stats(
		wmi_unified_t wmi_handle, void *evt_buf,
		uint32_t index, wmi_host_bcnflt_stats *bcnflt_stats);

/**
 * wmi_extract_rtt_hdr() - extract rtt header from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @ev: Pointer to hold rtt header
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_rtt_hdr(wmi_unified_t wmi_handle, void *evt_buf,
			       wmi_host_rtt_event_hdr *ev);

/**
 * wmi_extract_rtt_ev() - extract rtt event
 * @wmi_handle: wmi handle
 * @evt_buf: Pointer to event buffer
 * @ev: Pointer to hold rtt event
 * @hdump: Pointer to hold hex dump
 * @hdump_len: hex dump length
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_rtt_ev(wmi_unified_t wmi_handle, void *evt_buf,
			      wmi_host_rtt_meas_event *ev,
			      uint8_t *hdump, uint16_t hdump_len);

/**
 * wmi_extract_rtt_error_report_ev() - extract rtt error report from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @ev: Pointer to hold rtt error report
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_rtt_error_report_ev(wmi_unified_t wmi_handle, void *evt_buf,
				wmi_host_rtt_error_report_event *ev);

/**
 * wmi_extract_chan_stats() - extract chan stats from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @index: Index into chan stats
 * @chan_stats: Pointer to hold chan stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_chan_stats(wmi_unified_t wmi_handle, void *evt_buf,
		       uint32_t index, wmi_host_chan_stats *chan_stats);

/**
 * wmi_extract_thermal_stats() - extract thermal stats from event
 * @wmi_handle: wmi handle
 * @evt_buf: Pointer to event buffer
 * @temp: Pointer to hold extracted temperature
 * @level: Pointer to hold extracted level in host enum
 * @pdev_id: Pointer to hold extracted pdev_id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_thermal_stats(wmi_unified_t wmi_handle, void *evt_buf,
				     uint32_t *temp,
				     enum thermal_throttle_level *level,
				     uint32_t *pdev_id);

/**
 * wmi_extract_thermal_level_stats() - extract thermal level stats from
 * event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @idx: Index to level stats
 * @levelcount: Pointer to hold levelcount
 * @dccount: Pointer to hold dccount
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_thermal_level_stats(wmi_unified_t wmi_handle, void *evt_buf,
				uint8_t idx, uint32_t *levelcount,
				uint32_t *dccount);

/**
 * wmi_extract_comb_phyerr() - extract comb phy error from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @datalen: data length of event buffer
 * @buf_offset: Pointer to hold value of current event buffer offset
 * post extraction
 * @phyerr: Pointer to hold phyerr
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_comb_phyerr(wmi_unified_t wmi_handle, void *evt_buf,
			uint16_t datalen, uint16_t *buf_offset,
			wmi_host_phyerr_t *phyerr);

/**
 * wmi_extract_single_phyerr() - extract single phy error from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @datalen: data length of event buffer
 * @buf_offset: Pointer to hold value of current event buffer offset
 * post extraction
 * @phyerr: Pointer to hold phyerr
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_single_phyerr(wmi_unified_t wmi_handle, void *evt_buf,
			  uint16_t datalen, uint16_t *buf_offset,
			  wmi_host_phyerr_t *phyerr);

/**
 * wmi_extract_composite_phyerr() - extract composite phy error from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @datalen: Length of event buffer
 * @phyerr: Pointer to hold phy error
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_composite_phyerr(wmi_unified_t wmi_handle, void *evt_buf,
			     uint16_t datalen, wmi_host_phyerr_t *phyerr);

/**
 * wmi_extract_profile_ctx() - extract profile context from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @profile_ctx: Pointer to hold profile context
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_profile_ctx(wmi_unified_t wmi_handle, void *evt_buf,
			wmi_host_wlan_profile_ctx_t *profile_ctx);

/**
 * wmi_extract_profile_data() - extract profile data from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @idx: index of profile data
 * @profile_data: Pointer to hold profile data
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_profile_data(wmi_unified_t wmi_handle, void *evt_buf, uint8_t idx,
			 wmi_host_wlan_profile_t *profile_data);

/**
 * wmi_extract_pmf_bcn_protect_stats() - extract pmf bcn stats from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @bcn_stats: Pointer to hold pmf bcn protect stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_pmf_bcn_protect_stats(wmi_unified_t wmi_handle, void *evt_buf,
				  wmi_host_pmf_bcn_protect_stats *bcn_stats);

/**
 * extract_unit_test() - extract unit test from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @unit_test: Pointer to hold unit-test header
 * @maxspace: The amount of space in evt_buf
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_unit_test(wmi_unified_t wmi_handle, void *evt_buf,
		      wmi_unit_test_event *unit_test, uint32_t maxspace);

/**
 * wmi_extract_pdev_ext_stats() - extract extended pdev stats from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @index: Index into extended pdev stats
 * @pdev_ext_stats: Pointer to hold extended pdev stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_pdev_ext_stats(wmi_unified_t wmi_handle, void *evt_buf,
			   uint32_t index,
			   wmi_host_pdev_ext_stats *pdev_ext_stats);

/**
 * wmi_extract_bss_chan_info_event() - extract bss channel information
 * from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @bss_chan_info: Pointer to hold bss channel information
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_bss_chan_info_event(
		wmi_unified_t wmi_handle, void *evt_buf,
		wmi_host_pdev_bss_chan_info_event *bss_chan_info);

/**
 * wmi_extract_tx_data_traffic_ctrl_ev() - extract tx data traffic control
 * from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @ev: Pointer to hold data traffic control
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_tx_data_traffic_ctrl_ev(wmi_unified_t wmi_handle, void *evt_buf,
				    wmi_host_tx_data_traffic_ctrl_event *ev);

/**
 * wmi_extract_vdev_extd_stats() - extract extended vdev stats from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @index: Index into extended vdev stats
 * @vdev_extd_stats: Pointer to hold extended vdev stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_vdev_extd_stats(wmi_unified_t wmi_handle, void *evt_buf,
			    uint32_t index,
			    wmi_host_vdev_extd_stats *vdev_extd_stats);

/**
 * wmi_extract_bcn_stats() - extract beacon stats from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @index: Index into beacon stats
 * @vdev_bcn_stats: Pointer to hold beacon stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_bcn_stats(wmi_unified_t wmi_handle, void *evt_buf,
		      uint32_t index, wmi_host_bcn_stats *vdev_bcn_stats);

/**
 * wmi_extract_vdev_nac_rssi_stats() - extract NAC_RSSI stats from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @vdev_extd_stats: Pointer to hold nac rssi stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_vdev_nac_rssi_stats(
		wmi_unified_t wmi_handle, void *evt_buf,
		struct wmi_host_vdev_nac_rssi_event *vdev_nac_rssi_stats);

/**
 * wmi_extract_vdev_prb_fils_stats() - extract probe and fils vdev
 * stats from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @index: Index into extended vdev stats
 * @vdev_prb_fils_stats: Pointer to hold probe and fils vdev stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_vdev_prb_fils_stats(
		wmi_unified_t wmi_handle, void *evt_buf,
		uint32_t index,
		struct wmi_host_vdev_prb_fils_stats *vdev_prb_fils_stats);

/**
 * wmi_extract_peer_retry_stats() - extract peer retry stats from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @index: Index into peer retry stats
 * @peer_retry_stats: Pointer to hold  peer retry stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_peer_retry_stats(
	wmi_unified_t wmi_handle, void *evt_buf,
	uint32_t index, struct wmi_host_peer_retry_stats *peer_retry_stats);

/**
 * wmi_unified_send_power_dbg_cmd() - send power debug commands
 * @wmi_handle: wmi handle
 * @param: wmi power debug parameter
 *
 * Send WMI_POWER_DEBUG_CMDID parameters to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wmi_unified_send_power_dbg_cmd(wmi_unified_t wmi_handle,
					  struct wmi_power_dbg_params *param);

/**
 * wmi_extract_sar_cap_service_ready_ext() - extract SAR cap from
 *					     FW service ready event
 * @wmi_handle: wmi handle
 * @evt_buf: event buffer received from firmware
 * @ext_param: extended target info
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_extract_sar_cap_service_ready_ext(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf,
			struct wlan_psoc_host_service_ext_param *ext_param);

/**
 * wmi_unified_fw_test_cmd() - send fw test command to fw.
 * @wmi_handle: wmi handle
 * @wmi_fwtest: fw test command
 *
 * This function sends fw test command to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_fw_test_cmd(wmi_unified_t wmi_handle,
				   struct set_fwtest_params *wmi_fwtest);

/**
 * wmi_unified_wfa_test_cmd() - send wfa test command to fw.
 * @handle: wmi handle
 * @wmi_fwtest: wfa test param
 *
 * This function send wfa test command to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_wfa_test_cmd(wmi_unified_t wmi_handle,
				    struct set_wfatest_params *wmi_wfatest);

/**
 * wmi_unified_peer_rx_reorder_queue_setup_send() - send rx reorder queue
 *      setup command to fw
 * @wmi_handle: wmi handle
 * @param: Rx reorder queue setup parameters
 *
 * Return: QDF_STATUS for success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_peer_rx_reorder_queue_setup_send(
		wmi_unified_t wmi_handle,
		struct rx_reorder_queue_setup_params *param);

/**
 * wmi_unified_peer_rx_reorder_queue_remove_send() - send rx reorder queue
 *      remove command to fw
 * @wmi_handle: wmi handle
 * @param: Rx reorder queue remove parameters
 *
 * Return: QDF_STATUS for success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_peer_rx_reorder_queue_remove_send(
		wmi_unified_t wmi_handle,
		struct rx_reorder_queue_remove_params *param);

/*
 * wmi_extract_service_ready_ext() - extract extended service ready
 * @wmi_handle: wmi handle
 * @param: wmi power debug parameter
 *
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wmi_extract_service_ready_ext(
		wmi_unified_t wmi_handle, uint8_t *evt_buf,
		struct wlan_psoc_host_service_ext_param *param);

/*
 * wmi_extract_service_ready_ext2() - extract extended2 service ready
 * @wmi_handle: wmi handle
 * @evt_buff: pointer to event buffer
 * @param: wmi ext2 base parameters
 *
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wmi_extract_service_ready_ext2(
		struct wmi_unified *wmi_handle, uint8_t *evt_buf,
		struct wlan_psoc_host_service_ext2_param *param);

/**
 * wmi_extract_hw_mode_cap_service_ready_ext() -
 *       extract HW mode cap from service ready event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @hw_mode_idx: hw mode idx should be less than num_mode
 * @param: Pointer to hold evt buf
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_extract_hw_mode_cap_service_ready_ext(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf, uint8_t hw_mode_idx,
			struct wlan_psoc_host_hw_mode_caps *param);

/**
 * wmi_extract_mac_phy_cap_service_ready_ext() -
 *       extract MAC phy cap from service ready event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @hw_mode_id: hw mode id of hw_mode_caps
 * @phy_id: phy_id within hw_mode_cap
 * @param: pointer to mac phy caps structure to hold the values from event
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_extract_mac_phy_cap_service_ready_ext(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf,
			uint8_t hw_mode_id,
			uint8_t phy_id,
			struct wlan_psoc_host_mac_phy_caps *param);

/**
 * wmi_extract_mac_phy_cap_service_ready_ext2() - Extract MAC phy cap from
 * service ready ext2 event.
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @hw_mode_id: hw mode id of hw_mode_caps
 * @phy_id: phy_id within hw_mode_cap
 * @phy_idx: index to hw_mode_cap for the given hw_mode_id and phy_id
 * @mac_phy_cap: Pointer to mac_phy_cap_ext2 structure
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_extract_mac_phy_cap_service_ready_ext2(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf,
			uint8_t hw_mode_id,
			uint8_t phy_id,
			uint8_t phy_idx,
			struct wlan_psoc_host_mac_phy_caps_ext2 *mac_phy_cap);

/**
 * wmi_extract_reg_cap_service_ready_ext() -
 *       extract REG cap from service ready event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @phy_idx: phy idx should be less than num_mode
 * @param: Pointer to hold evt buf
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS
wmi_extract_reg_cap_service_ready_ext(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf, uint8_t phy_idx,
			struct wlan_psoc_host_hal_reg_capabilities_ext *param);

/**
 * wmi_extract_dbr_ring_cap_service_ready_ext: Extract direct buffer rx
 *                                             capability received through
 *                                             extended service ready event
 * @wmi_handle: WMI handle
 * @evt_buf: Event buffer
 * @idx: Index of the module for which capability is received
 * @param: Pointer to direct buffer rx ring cap struct
 *
 * Return: QDF status of operation
 */
QDF_STATUS wmi_extract_dbr_ring_cap_service_ready_ext(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf, uint8_t idx,
			struct wlan_psoc_host_dbr_ring_caps *param);

/**
 * wmi_extract_dbr_ring_cap_service_ready_ext2: Extract direct buffer rx
 *                                              capability received through
 *                                              extended service ready2 event
 * @wmi_handle: WMI handle
 * @evt_buf: Event buffer
 * @idx: Index of the module for which capability is received
 * @param: Pointer to direct buffer rx ring cap struct
 *
 * Return: QDF status of operation
 */
QDF_STATUS wmi_extract_dbr_ring_cap_service_ready_ext2(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf, uint8_t idx,
			struct wlan_psoc_host_dbr_ring_caps *param);

/**
 * wmi_extract_scan_radio_cap_service_ready_ext2: Extract scan radio capability
 * received through extended service ready2 event
 * @wmi_handle: WMI handle
 * @evt_buf: Event buffer
 * @idx: Index of the module for which capability is received
 * @param: Pointer to scan radio cap struct
 *
 * Return: QDF status of operation
 */
QDF_STATUS wmi_extract_scan_radio_cap_service_ready_ext2(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf, uint8_t idx,
			struct wlan_psoc_host_scan_radio_caps *param);

/**
 * wmi_extract_spectral_scaling_params_service_ready_ext: Extract Spectral
 *                                             scaling params received through
 *                                             extended service ready event
 * @wmi_handle: WMI handle
 * @evt_buf: Event buffer
 * @idx: Index
 * @param: Pointer to Spectral scaling params
 *
 * Return: QDF status of operation
 */
QDF_STATUS wmi_extract_spectral_scaling_params_service_ready_ext(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf, uint8_t idx,
			struct wlan_psoc_host_spectral_scaling_params *param);

/**
 * wmi_extract_pdev_utf_event() -
 *       extract UTF data from pdev utf event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @param: Pointer to hold evt buf
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_extract_pdev_utf_event(wmi_unified_t wmi_handle,
				      uint8_t *evt_buf,
				      struct wmi_host_pdev_utf_event *param);

/**
 * wmi_extract_pdev_qvit_event() -
 *       extract UTF data from pdev qvit event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @param: Pointer to hold evt buf
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_extract_pdev_qvit_event(wmi_unified_t wmi_handle,
				       uint8_t *evt_buf,
				       struct wmi_host_pdev_qvit_event *param);

#ifdef WLAN_SUPPORT_RF_CHARACTERIZATION
/**
 * wmi_extract_num_rf_characterziation_entries - Extract number of RF
 * characterization metrics received from the RF characterization event.
 * @wmi_hdl: WMI handle
 * @evt_buf: Event buffer
 * @num_rf_characterization_entries: Number of RF characterization metrics
 *
 * Return: QDF status of operation
 */
QDF_STATUS wmi_extract_num_rf_characterization_entries(wmi_unified_t wmi_hdl,
				uint8_t *evt_buf,
				uint32_t *num_rf_characterization_entries);

/**
 * wmi_extract_rf_characterziation_entries - Extract RF characterization metrics
 * received from the RF characterization event.
 * @wmi_hdl: WMI handle
 * @evt_buf: Event buffer
 * @num_rf_characterization_entries: Number of RF characterization metrics
 * @rf_characterization_entries: Pointer to RF characterization metrics
 *
 * Return: QDF status of operation
 */
QDF_STATUS wmi_extract_rf_characterization_entries(wmi_unified_t wmi_hdl,
	uint8_t *evt_buf,
	uint32_t num_rf_characterization_entries,
	struct wmi_host_rf_characterization_event_param *rf_characterization_entries);
#endif

/*
 * wmi_extract_chainmask_tables_tlv() - extract chain mask tables
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer.
 * @chainmask_table: pointer to struct wlan_psoc_host_chainmask_table
 *
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wmi_extract_chainmask_tables(
		wmi_unified_t wmi_handle, uint8_t *evt_buf,
		struct wlan_psoc_host_chainmask_table *chainmask_table);

/**
 * wmi_unified_dfs_phyerr_offload_en_cmd() - enable dfs phyerr offload
 * @wmi_handle: wmi handle
 * @pdev_id: pdev id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wmi_unified_dfs_phyerr_offload_en_cmd(wmi_unified_t wmi_handle,
						 uint32_t pdev_id);

/**
 * wmi_unified_dfs_phyerr_offload_dis_cmd() - disable dfs phyerr offload
 * @wmi_handle: wmi handle
 * @pdev_id: pdev id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wmi_unified_dfs_phyerr_offload_dis_cmd(wmi_unified_t wmi_handle,
						  uint32_t pdev_id);

#ifdef QCA_SUPPORT_AGILE_DFS
/**
 * wmi_unified_send_vdev_adfs_ch_cfg_cmd() - send adfs channel config command
 * @wmi_handle: wmi handle
 * @param: adfs channel config params
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_unified_send_vdev_adfs_ch_cfg_cmd(wmi_unified_t wmi_handle,
				      struct vdev_adfs_ch_cfg_params *param);

/**
 * wmi_unified_send_vdev_adfs_ocac_abort_cmd() - send adfs o-cac abort command
 * @wmi_handle: wmi handle
 * @param: adfs channel o-cac abort params
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_unified_send_vdev_adfs_ocac_abort_cmd(wmi_unified_t wmi_handle,
					  struct vdev_adfs_abort_params *param);
#endif

/**
 *  wmi_unified_set_country_cmd_send() - WMI set country function
 *  @wmi_handle      : handle to WMI.
 *  @param    : pointer to hold set country cmd parameter
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_country_cmd_send(wmi_unified_t wmi_handle,
					    struct set_country *param);

#ifdef WLAN_FEATURE_ACTION_OUI
/**
 * wmi_unified_send_action_oui_cmd() - send action oui cmd to fw
 * @wmi_handle: wma handle
 * @req: wmi action oui message to be send
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_send_action_oui_cmd(wmi_unified_t wmi_handle,
				struct action_oui_request *req);
#endif /* WLAN_FEATURE_ACTION_OUI */

/**
 * wmi_unified_send_request_get_rcpi_cmd() - command to request rcpi value
 * @wmi_handle: wma handle
 * @get_rcpi_param: rcpi params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_send_request_get_rcpi_cmd(wmi_unified_t wmi_handle,
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
QDF_STATUS
wmi_extract_rcpi_response_event(wmi_unified_t wmi_handle, void *evt_buf,
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
 * @wmi_handle: wmi handle
 *
 * Send WMI_PDEV_WDS_ENTRY_LIST_CMDID parameters to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wmi_unified_send_dump_wds_table_cmd(wmi_unified_t wmi_handle);

/**
 * wmi_extract_wds_entry - api to extract wds entry
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @wds_entry: wds entry
 * @idx: index to point wds entry in event buffer
 *
 * Return: QDF_STATUS_SUCCESS for successful event parse
 *         else QDF_STATUS_E_INVAL or QDF_STATUS_E_FAILURE
 */
QDF_STATUS
wmi_extract_wds_entry(wmi_unified_t wmi_handle, uint8_t *evt_buf,
		      struct wdsentry *wds_entry, u_int32_t idx);

/**
 * wmi_unified_send_obss_detection_cfg_cmd() - WMI function to send obss
 *  detection configuration to FW.
 * @wmi_handle: wmi handle
 * @cfg: obss detection configuration
 *
 * Send WMI_SAP_OBSS_DETECTION_CFG_CMDID parameters to fw.
 *
 * Return: QDF_STATUS
 */

QDF_STATUS wmi_unified_send_obss_detection_cfg_cmd(
		wmi_unified_t wmi_handle,
		struct wmi_obss_detection_cfg_param *cfg);

/**
 * wmi_unified_extract_obss_detection_info() - WMI function to extract obss
 *  detection info from FW.
 * @wmi_handle: wmi handle
 * @data: event data from firmware
 * @info: Pointer to hold obss detection info
 *
 * This function is used to extract obss info from firmware.
 *
 * Return: QDF_STATUS
 */

QDF_STATUS wmi_unified_extract_obss_detection_info(
			wmi_unified_t wmi_handle,
			uint8_t *data,
			struct wmi_obss_detect_info *info);

#ifdef WLAN_SUPPORT_GREEN_AP
QDF_STATUS wmi_extract_green_ap_egap_status_info(
	wmi_unified_t wmi_hdl, uint8_t *evt_buf,
	struct wlan_green_ap_egap_status_info *egap_status_info_params);
#endif

/**
 * wmi_unified_send_roam_scan_stats_cmd() - Wrapper to request roam scan stats
 * @wmi_handle: wmi handle
 * @params: request params
 *
 * This function is used to send the roam scan stats request command to
 * firmware.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_send_roam_scan_stats_cmd(wmi_unified_t wmi_handle,
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
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to the event buffer
 * @vdev_id: output pointer to hold vdev id
 * @tx_status: output pointer to hold bcn tx status
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_extract_offload_bcn_tx_status_evt(wmi_unified_t wmi_handle, void *evt_buf,
				      uint32_t *vdev_id, uint32_t *tx_status);

/* wmi_get_ch_width_from_phy_mode() - convert phy mode to channel width
 * @wmi_handle: wmi handle
 * @phymode: phy mode
 *
 * Return: wmi channel width
 */
wmi_host_channel_width wmi_get_ch_width_from_phy_mode(
	wmi_unified_t wmi_handle, WMI_HOST_WLAN_PHY_MODE phymode);

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
 * @wmi_handle: wmi handle
 * @params: radar found params
 *
 * This function passes the average radar parameters to fw
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_dfs_send_avg_params_cmd(wmi_unified_t wmi_handle,
				    struct dfs_radar_found_params *params);

/**
 * wmi_extract_dfs_status_from_fw() - extract host dfs status from fw.
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @dfs_status_check: pointer to the host dfs status
 *
 * This function extracts the result of host dfs from fw
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_dfs_status_from_fw(wmi_unified_t wmi_handle,
					  void *evt_buf,
					  uint32_t  *dfs_status_check);
#endif
#ifdef OL_ATH_SMART_LOGGING
/**
 * wmi_unified_send_smart_logging_enable_cmd() - send smart logging enable cmd
 * @wmi_handle: wmi handle
 * @param: enable/disable
 *
 * This function enables/disable the smart logging feature
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_send_smart_logging_enable_cmd(wmi_unified_t wmi_handle,
						     uint32_t param);

/**
 * wmi_unified_send_smart_logging_fatal_cmd() - send smart logging fatal cmd
 * @wmi_handle: wmi handle
 * @param:  Fatal event
 *
 * This function sends the smart log fatal events to the FW
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_send_smart_logging_fatal_cmd(wmi_unified_t wmi_handle,
					 struct wmi_debug_fatal_events *param);

/**
 * wmi_extract_smartlog_ev() - extract smartlog event info from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @ev: Pointer to hold fatal events
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_smartlog_ev(wmi_unified_t wmi_handle, void *evt_buf,
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
 * @evt_buf: pointer to event buffer
 * @ev: Pointer to hold ctl status
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_ctl_failsafe_check_ev_param(
		wmi_unified_t wmi_handle,
		void *evt_buf,
		struct wmi_host_pdev_ctl_failsafe_event *param);

#ifdef OBSS_PD
/**
 * wmi_unified_send_obss_spatial_reuse_set_cmd() - send obss pd offset
 * @wmi_handle: wmi handle
 * @oobss_spatial_reuse_param: Pointer to obsspd min max offset
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_send_obss_spatial_reuse_set_cmd(
	wmi_unified_t wmi_handle,
	struct wmi_host_obss_spatial_reuse_set_param *obss_spatial_reuse_param);

/**
 * wmi_unified_send_obss_spatial_reuse_set_def_thresh_cmd() - send def thresh
 * @wmi_handle: wmi handle
 * @thresh: Pointer to def thresh
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_send_obss_spatial_reuse_set_def_thresh_cmd(
	wmi_unified_t wmi_handle,
	struct wmi_host_obss_spatial_reuse_set_def_thresh *thresh);

/**
 * wmi_unified_send_self_srg_bss_color_bitmap_set_cmd() - Send 64-bit BSS color
 * bitmap to be used by SRG based Spatial Reuse feature
 * @wmi_handle: wmi handle
 * @bitmap_0: lower 32 bits in BSS color bitmap
 * @bitmap_1: upper 32 bits in BSS color bitmap
 * @pdev_id: pdev ID
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_send_self_srg_bss_color_bitmap_set_cmd(
	wmi_unified_t wmi_handle,  uint32_t bitmap_0,
	uint32_t bitmap_1, uint8_t pdev_id);

/**
 * wmi_unified_send_self_srg_partial_bssid_bitmap_set_cmd() - Send 64-bit
 * partial BSSID bitmap to be used by SRG based Spatial Reuse feature
 * @wmi_handle: wmi handle
 * @bitmap_0: lower 32 bits in partial BSSID bitmap
 * @bitmap_1: upper 32 bits in partial BSSID bitmap
 * @pdev_id: pdev ID
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_send_self_srg_partial_bssid_bitmap_set_cmd(
	wmi_unified_t wmi_handle,  uint32_t bitmap_0,
	uint32_t bitmap_1, uint8_t pdev_id);

/**
 * wmi_unified_send_self_srg_obss_color_enable_bitmap_cmd() - Send 64-bit BSS
 * color enable bitmap to be used by SRG based Spatial Reuse feature to the FW
 * @wmi_handle: wmi handle
 * @bitmap_0: lower 32 bits in BSS color enable bitmap
 * @bitmap_1: upper 32 bits in BSS color enable bitmap
 * @pdev_id: pdev ID
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_send_self_srg_obss_color_enable_bitmap_cmd(
	wmi_unified_t wmi_handle,  uint32_t bitmap_0,
	uint32_t bitmap_1, uint8_t pdev_id);

/**
 * wmi_unified_send_self_srg_obss_bssid_enable_bitmap_cmd() - Send 64-bit OBSS
 * BSSID enable bitmap to be used by SRG based Spatial Reuse feature to the FW
 * @wmi_handle: wmi handle
 * @bitmap_0: lower 32 bits in BSSID enable bitmap
 * @bitmap_1: upper 32 bits in BSSID enable bitmap
 * @pdev_id: pdev ID
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_send_self_srg_obss_bssid_enable_bitmap_cmd(
	wmi_unified_t wmi_handle,  uint32_t bitmap_0,
	uint32_t bitmap_1, uint8_t pdev_id);

/**
 * wmi_unified_send_self_non_srg_obss_color_enable_bitmap_cmd() - Send 64-bit
 * BSS color enable bitmap to be used by Non-SRG based Spatial Reuse
 * feature to the FW
 * @wmi_handle: wmi handle
 * @bitmap_0: lower 32 bits in BSS color enable bitmap
 * @bitmap_1: upper 32 bits in BSS color enable bitmap
 * @pdev_id: pdev ID
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_send_self_non_srg_obss_color_enable_bitmap_cmd(
	wmi_unified_t wmi_handle,  uint32_t bitmap_0,
	uint32_t bitmap_1, uint8_t pdev_id);

/**
 * wmi_unified_send_self_non_srg_obss_bssid_enable_bitmap_cmd() - Send 64-bit
 * OBSS BSSID enable bitmap to be used by Non-SRG based Spatial Reuse
 * feature to the FW
 * @wmi_handle: wmi handle
 * @bitmap_0: lower 32 bits in BSSID enable bitmap
 * @bitmap_1: upper 32 bits in BSSID enable bitmap
 * @pdev_id: pdev ID
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_send_self_non_srg_obss_bssid_enable_bitmap_cmd(
	wmi_unified_t wmi_handle,  uint32_t bitmap_0,
	uint32_t bitmap_1, uint8_t pdev_id);
#endif /* OBSS_PD */

/**
 * wmi_convert_pdev_id_host_to_target() - Convert pdev_id from host to target
 * defines. For legacy there is not conversion required. Just return pdev_id as
 * it is.
 * @wmi_handle: wmi handle
 * @host_pdev_id: host pdev_id to be converted.
 * @target_pdev_id: Output target pdev id.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wmi_convert_pdev_id_host_to_target(wmi_unified_t wmi_handle,
					      uint32_t host_pdev_id,
					      uint32_t *target_pdev_id);

#ifndef CNSS_GENL
/**
 * wmi_convert_pdev_id_target_to_host() - Convert pdev_id from target to host
 * defines. For legacy there is not conversion required. Just return pdev_id as
 * it is.
 * @wmi_handle: wmi handle
 * @target_pdev_id: target pdev_id to be converted.
 * @host_pdev_id: Output host pdev id.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wmi_convert_pdev_id_target_to_host(wmi_unified_t wmi_handle,
					      uint32_t target_pdev_id,
					      uint32_t *host_pdev_id);
#endif

/**
 * wmi_unified_send_bss_color_change_enable_cmd() - WMI function to send bss
 *  color change enable to FW.
 * @wmi_handle: wmi handle
 * @vdev_id: vdev ID
 * @enable: enable or disable color change handeling within firmware
 *
 * Send WMI_BSS_COLOR_CHANGE_ENABLE_CMDID parameters to fw,
 * thereby firmware updates bss color when AP announces bss color change.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_unified_send_bss_color_change_enable_cmd(wmi_unified_t wmi_handle,
					     uint32_t vdev_id,
					     bool enable);

/**
 * wmi_unified_send_obss_color_collision_cfg_cmd() - WMI function to send bss
 *  color collision detection configuration to FW.
 * @wmi_handle: wmi handle
 * @cfg: obss color collision detection configuration
 *
 * Send WMI_OBSS_COLOR_COLLISION_DET_CONFIG_CMDID parameters to fw.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wmi_unified_send_obss_color_collision_cfg_cmd(
		wmi_unified_t wmi_handle,
		struct wmi_obss_color_collision_cfg_param *cfg);

/**
 * wmi_unified_extract_obss_color_collision_info() - WMI function to extract
 *  obss color collision info from FW.
 * @wmi_handle: wmi handle
 * @data: event data from firmware
 * @info: Pointer to hold bss color collision info
 *
 * This function is used to extract bss collision info from firmware.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wmi_unified_extract_obss_color_collision_info(
		wmi_unified_t wmi_handle,
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

/**
 * wmi_unified_send_idle_trigger_monitor() - send idle trigger monitor command
 * @wmi_handle: WMI handle
 * @val: idle trigger monitor value - 1 for idle monitor on, 0 for idle monitor
 * off
 *
 * Return: QDF_STATUS_SUCCESS if success, else returns proper error code.
 */
QDF_STATUS
wmi_unified_send_idle_trigger_monitor(wmi_unified_t wmi_handle, uint8_t val);


#ifdef WIFI_POS_CONVERGED
/**
 * wmi_extract_oem_response_param() - WMI function to extract OEM response param
 * @wmi_hdl: WMI handle
 * @resp_buf: Buffer holding response data
 * @oem_resp_param: zero-filled structure pointer to hold oem response data
 *
 * Return: QDF_STATUS_SUCCESS if success, else returns proper error code.
 */
QDF_STATUS
wmi_extract_oem_response_param(wmi_unified_t wmi_hdl, void *resp_buf,
			       struct wmi_oem_response_param *oem_resp_param);
#endif /* WIFI_POS_CONVERGED */
/**
 * wmi_critical_events_in_flight() - get the number of critical events in flight
 *
 * @wmi_hdl: WMI handle
 *
 * Return: the number of critical events in flight.
 */
uint32_t wmi_critical_events_in_flight(struct wmi_unified *wmi);


#ifdef FEATURE_ANI_LEVEL_REQUEST
/**
 * wmi_unified_ani_level_cmd_send() - WMI function to send get ani level cmd
 * @wmi_hdl: WMI handle
 * @freqs: pointer to list of freqs for which ANI levels are to be fetched
 * @num_freqs: number of freqs in the above parameter
 *
 * Return: QDF_STATUS_SUCCESS if success, else returns proper error code.
 */
QDF_STATUS wmi_unified_ani_level_cmd_send(wmi_unified_t wmi_handle,
					  uint32_t *freqs,
					  uint8_t num_freqs);

/**
 * wmi_unified_extract_ani_level() - WMI function to receive ani level cmd
 * @wmi_hdl: WMI handle
 * @info: pointer to ANI data received from the FW and stored in HOST
 * @num_freqs: number of freqs in the above parameter
 *
 * Return: QDF_STATUS_SUCCESS if success, else returns proper error code.
 */
QDF_STATUS wmi_unified_extract_ani_level(wmi_unified_t wmi_handle,
					 uint8_t *data,
					 struct wmi_host_ani_level_event **info,
					 uint32_t *num_freqs);
#endif /* FEATURE_ANI_LEVEL_REQUEST */

#ifdef WLAN_FEATURE_PKT_CAPTURE
/**
 * wmi_unified_extract_vdev_mgmt_offload_event() - Extract mgmt offload params
 * @wmi: WMI handle
 * @evt_buf: Event buffer
 * @params: Management offload event params
 *
 * WMI function to extract management offload event params
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_unified_extract_vdev_mgmt_offload_event(wmi_unified_t wmi, void *evt_buf,
				struct mgmt_offload_event_params *params);
#endif

#ifdef FEATURE_WLAN_TIME_SYNC_FTM
/**
 * wmi_unified_send_wlan_time_sync_ftm_trigger() - send ftm timesync trigger cmd
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @burst_mode: mode reg getting time sync relation from FW
 *
 * This function indicates the FW to trigger wlan time sync using FTM
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_send_wlan_time_sync_ftm_trigger(wmi_unified_t wmi_handle,
					    uint32_t vdev_id,
					    bool burst_mode);

/**
 * wmi_unified_send_wlan_time_sync_qtime() - send ftm time sync qtime cmd.
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @lpass_ts: audio qtime
 *
 * This function sends the wmi cmd to FW having audio qtime
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_send_wlan_time_sync_qtime(wmi_unified_t wmi_handle,
				      uint32_t vdev_id, uint64_t lpass_ts);

/**
 * wmi_unified_extract_time_sync_ftm_start_stop_params() - extract FTM time sync
 *							   params
 * @wmi_handle: wmi handle
 * @evt_buf: event buffer
 * @param: params received in start stop ftm timesync event
 *
 * This function extracts the params from ftm timesync start stop event
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_extract_time_sync_ftm_start_stop_params(
				wmi_unified_t wmi_handle, void *evt_buf,
				struct ftm_time_sync_start_stop_params *param);

/**
 * wmi_unified_extract_time_sync_ftm_offset() - extract timesync FTM offset
 * @wmi_handle: wmi handle
 * @evt_buf: event buffer
 * @param: params received in ftm timesync offset event
 *
 * This function extracts the params from ftm timesync offset event
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_extract_time_sync_ftm_offset(wmi_unified_t wmi_handle,
					 void *evt_buf,
					 struct ftm_time_sync_offset *param);
#endif /* FEATURE_WLAN_TIME_SYNC_FTM */

/**
 * wmi_unified_send_injector_frame_config_cmd() - configure injector frame
 * @wmi_handle: wmi handle
 * @param: params received in the injector frame configure command
 *
 * This function configures the AP to send out injector frames
 *
 * Return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_send_injector_frame_config_cmd(wmi_unified_t wmi_handle,
				struct wmi_host_injector_frame_params *param);

/**
 * wmi_unified_send_cp_stats_cmd() - Send cp stats command
 * @wmi_handle: wmi handle
 * @buf_ptr: buf_ptr received from wifistats
 * @buf_len: length of buffer received from wifistats
 *
 * This function sends cp stats cmd to get cp stats.
 *
 * Return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_send_cp_stats_cmd(wmi_unified_t wmi_handle,
					 void *buf_ptr, uint32_t buf_len);

/**
 * wmi_unified_extract_cp_stats_more_pending() - extract more flag
 * @wmi_handle: wmi handle
 * @evt_buf: event buffer
 * @more_flag: more flag
 *
 * This function extracts the more_flag from fixed param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_extract_cp_stats_more_pending(wmi_unified_t wmi_handle,
					  void *evt_buf, uint32_t *more_flag);

/**
 * wmi_unified_send_vdev_tsf_tstamp_action_cmd() - send vdev tsf action command
 * @wmi: wmi handle
 * @vdev_id: vdev id
 *
 * TSF_TSTAMP_READ_VALUE is the only operation supported
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_unified_send_vdev_tsf_tstamp_action_cmd(wmi_unified_t wmi_hdl,
						       uint8_t vdev_id);

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
					     struct wmi_host_tsf_event *param);
#endif /* _WMI_UNIFIED_API_H_ */
