/*
 * Copyright (c) 2013-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: Implement API's specific to ROAMING component.
 */

#ifndef _WMI_UNIFIED_ROAM_API_H_
#define _WMI_UNIFIED_ROAM_API_H_

#include <wmi_unified_roam_param.h>
#include "wlan_cm_roam_public_struct.h"

#ifdef FEATURE_LFR_SUBNET_DETECTION
/**
 * wmi_unified_set_gateway_params_cmd() - set gateway parameters
 * @wmi_handle: wmi handle
 * @req: gateway parameter update request structure
 *
 * This function reads the incoming @req and fill in the destination
 * WMI structure and sends down the gateway configs down to the firmware
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failures;
 *         error number otherwise
 */
QDF_STATUS
wmi_unified_set_gateway_params_cmd(wmi_unified_t wmi_handle,
				   struct gateway_update_req_param *req);
#endif

#ifdef FEATURE_RSSI_MONITOR
/**
 * wmi_unified_set_rssi_monitoring_cmd() - set rssi monitoring
 * @wmi_handle: wmi handle
 * @req: rssi monitoring request structure
 *
 * This function reads the incoming @req and fill in the destination
 * WMI structure and send down the rssi monitoring configs down to the firmware
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failures;
 *         error number otherwise
 */
QDF_STATUS
wmi_unified_set_rssi_monitoring_cmd(wmi_unified_t wmi_handle,
				    struct rssi_monitor_param *req);
#endif

/**
 * wmi_unified_roam_scan_offload_rssi_thresh_cmd() - set roam scan rssi
 *							parameters
 * @wmi_handle: wmi handle
 * @roam_req: roam rssi related parameters
 *
 * This function reads the incoming @roam_req and fill in the destination
 * WMI structure and send down the roam scan rssi configs down to the firmware
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_roam_scan_offload_rssi_thresh_cmd(
		wmi_unified_t wmi_handle,
		struct wlan_roam_offload_scan_rssi_params *roam_req);

/**
 * wmi_unified_roam_scan_offload_scan_period() - set roam offload scan period
 * @wmi_handle: wmi handle
 * @param: pointer to roam scan period params to be sent to fw
 *
 * Send WMI_ROAM_SCAN_PERIOD parameters to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_roam_scan_offload_scan_period(
	wmi_unified_t wmi_handle, struct wlan_roam_scan_period_params *param);

/**
 * wmi_unified_roam_mawc_params_cmd() - configure roaming MAWC parameters
 * @wmi_handle: wmi handle
 * @params: Parameters to be configured
 *
 * Pass the MAWC(Motion Aided wireless connectivity) related roaming
 * parameters from the host to the target
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_roam_mawc_params_cmd(wmi_unified_t wmi_handle,
				 struct wlan_roam_mawc_params *params);

/**
 * wmi_unified_roam_scan_filter_cmd() - send roam scan whitelist,
 *                                      blacklist and preferred list
 * @wmi_handle: wmi handle
 * @roam_req: roam scan lists related parameters
 *
 * This function reads the incoming @roam_req and fill in the destination
 * WMI structure and send down the different roam scan lists down to the fw
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_roam_scan_filter_cmd(wmi_unified_t wmi_handle,
				 struct roam_scan_filter_params *roam_req);

#ifdef FEATURE_WLAN_ESE
/**
 * wmi_unified_plm_stop_cmd() - plm stop request
 * @wmi_handle: wmi handle
 * @plm: plm request parameters
 *
 * This function request FW to stop PLM.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_plm_stop_cmd(wmi_unified_t wmi_handle,
				    const struct plm_req_params *plm);

/**
 * wmi_unified_plm_start_cmd() - plm start request
 * @wmi_handle: wmi handle
 * @plm: plm request parameters
 *
 * This function request FW to start PLM.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_plm_start_cmd(wmi_unified_t wmi_handle,
				     const struct plm_req_params *plm);
#endif /* FEATURE_WLAN_ESE */

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/* wmi_unified_set_ric_req_cmd() - set ric request element
 * @wmi_handle: wmi handle
 * @msg: message
 * @is_add_ts: is addts required
 *
 * This function sets ric request element for 11r roaming.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_ric_req_cmd(wmi_unified_t wmi_handle, void *msg,
				       uint8_t is_add_ts);

/**
 * wmi_unified_roam_synch_complete_cmd() - roam synch complete command to fw.
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 *
 * This function sends roam synch complete event to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_roam_synch_complete_cmd(wmi_unified_t wmi_handle,
					       uint8_t vdev_id);

/**
 * wmi_unified__roam_invoke_cmd() - send roam invoke command to fw.
 * @wmi_handle: wmi handle
 * @roaminvoke: roam invoke command
 *
 * Send roam invoke command to fw for fastreassoc.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_roam_invoke_cmd(wmi_unified_t wmi_handle,
			    struct roam_invoke_req *roaminvoke);

/**
 * wmi_unified_set_roam_triggers() - send roam trigger bitmap
 * @wmi_handle: wmi handle
 * @triggers: Roam trigger bitmap params as defined @roam_control_trigger_reason
 *
 * This function passes the roam trigger bitmap to fw
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_roam_triggers(wmi_unified_t wmi_handle,
					 struct wlan_roam_triggers *triggers);

/**
 * wmi_unified_send_disconnect_roam_params() - Send disconnect roam trigger
 * parameters to firmware
 * @wmi_hdl:  wmi handle
 * @params: pointer to wlan_roam_disconnect_params
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_unified_send_disconnect_roam_params(wmi_unified_t wmi_handle,
				struct wlan_roam_disconnect_params *req);

/**
 * wmi_unified_send_idle_roam_params() - Send idle roam trigger params to fw
 * @wmi_hdl:  wmi handle
 * @params: pointer to wlan_roam_idle_params
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_unified_send_idle_roam_params(wmi_unified_t wmi_handle,
				  struct wlan_roam_idle_params *req);

/**
 * wmi_unified_send_roam_preauth_status() - Send roam preauthentication status
 * to target.
 * @wmi_handle: wmi handle
 * @param: Roam auth status params
 *
 * This function passes preauth status of WPA3 SAE auth to firmware. It is
 * called when external_auth_status event is received from userspace.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_unified_send_roam_preauth_status(wmi_unified_t wmi_handle,
				     struct wmi_roam_auth_status_params *param);

/**
 * wmi_unified_vdev_set_pcl_cmd  - Send Vdev PCL command to fw
 * @wmi_handle: WMI handle
 * @params: Set VDEV pcl parameters
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wmi_unified_vdev_set_pcl_cmd(wmi_unified_t wmi_handle,
					struct set_pcl_cmd_params *params);

/**
 * wmi_extract_roam_sync_event  - Extract roam sync event
 * @wmi_handle: WMI handle
 * @evt_buf: Event buffer
 * @len: evt buffer data len
 * @synd_ind: roam sync ptr
 *
 * This api will allocate memory for roam sync info, extract
 * the information sent by FW and pass to CM.The memory will be
 * freed by target_if_cm_roam_sync_event.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_extract_roam_sync_event(wmi_unified_t wmi_handle, void *evt_buf,
			    uint32_t len,
			    struct roam_offload_synch_ind **sync_ind);

/**
 * wmi_extract_roam_sync_frame_event  - Extract roam sync frame event
 * @wmi_handle: WMI handle
 * @event: Event buffer
 * @len: evt buffer data len
 * @frame_ptr: roam sync frame ptr
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_extract_roam_sync_frame_event(wmi_unified_t wmi_handle, void *event,
				  uint32_t len,
				  struct roam_synch_frame_ind *frame_ptr);

/**
 * wmi_extract_roam_event  - Extract roam event
 * @wmi_handle: WMI handle
 * @event: Event data received from firmware
 * @data_len: Event data length received from firmware
 * @roam_event: Extract the event and fill in roam_event
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_extract_roam_event(wmi_unified_t wmi_handle, uint8_t *event,
		       uint32_t data_len,
		       struct roam_offload_roam_event *roam_event);

/**
 * wmi_extract_btm_blacklist_event - Extract btm blacklist event
 * @wmi_handle: WMI handle
 * @event: Event data received from firmware
 * @data_len: Event data length received from firmware
 * @dst_list: Extract the event and fill in dst_list
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_extract_btm_blacklist_event(wmi_unified_t wmi_handle,
				uint8_t *event, uint32_t data_len,
				struct roam_blacklist_event **dst_list);

/**
 * wmi_extract_vdev_disconnect_event - Extract disconnect event data
 * @wmi_handle: WMI handle
 * @event: Event data received from firmware
 * @data_len: Event data length received from firmware
 * @data: Extract the event and fill in data
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_extract_vdev_disconnect_event(wmi_unified_t wmi_handle,
				  uint8_t *event, uint32_t data_len,
				  struct vdev_disconnect_event_data *data);

/**
 * wmi_extract_roam_scan_chan_list - Extract roam scan chan list
 * @wmi_handle: WMI handle
 * @event: Event data received from firmware
 * @data_len: Event data length received from firmware
 * @data: Extract the event and fill in data
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_extract_roam_scan_chan_list(wmi_unified_t wmi_handle,
				uint8_t *event, uint32_t data_len,
				struct cm_roam_scan_ch_resp **data);

/**
 * wmi_unified_extract_roam_btm_response() - Extract BTM response
 * @wmi:       wmi handle
 * @evt_buf:   Pointer to the event buffer
 * @dst:       Pointer to destination structure to fill data
 * @idx:       TLV id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_unified_extract_roam_btm_response(wmi_unified_t wmi, void *evt_buf,
				      struct roam_btm_response_data *dst,
				      uint8_t idx);

/**
 * wmi_unified_extract_roam_initial_info() - Extract initial info
 * @wmi:       wmi handle
 * @evt_buf:   Pointer to the event buffer
 * @dst:       Pointer to destination structure to fill data
 * @idx:       TLV id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_unified_extract_roam_initial_info(wmi_unified_t wmi, void *evt_buf,
				      struct roam_initial_data *dst,
				      uint8_t idx);

/**
 * wmi_unified_extract_roam_msg_info() - Extract roam msg info
 * @wmi:       wmi handle
 * @evt_buf:   Pointer to the event buffer
 * @dst:       Pointer to destination structure to fill data
 * @idx:       TLV id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_unified_extract_roam_msg_info(wmi_unified_t wmi, void *evt_buf,
				  struct roam_msg_info *dst, uint8_t idx);

/**
 * wmi_extract_roam_stats_event  - Extract roam stats event
 * @wmi_handle: WMI handle
 * @event: Event data received from firmware
 * @data_len: Event data length received from firmware
 * @stats_info: Extract the event and fill in stats_info
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_extract_roam_stats_event(wmi_unified_t wmi_handle,
			     uint8_t *event, uint32_t data_len,
			     struct roam_stats_event **stats_info);

/**
 * wmi_unified_extract_roam_extract_frame_info() - Extract the roam frame
 * info TLV from roam stats event
 * @wmi:        wmi handle
 * @evt_buf:    Pointer to the event buffer
 * @dst:        Pointer to destination structure to fill data
 * @idx:        Index of the TLV to read
 * @num_frames: Number of TLV to read
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_unified_extract_roam_extract_frame_info(wmi_unified_t wmi, void *evt_buf,
					    struct roam_frame_stats *dst,
					    uint8_t idx, uint8_t num_frames);

/**
 * wmi_extract_auth_offload_event  - Extract auth offload event
 * @wmi_handle: WMI handle
 * @event: Event data received from firmware
 * @data_len: Event data length received from firmware
 * @roam_event: Extract the event and fill in auth_event
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_extract_auth_offload_event(wmi_unified_t wmi_handle,
			       uint8_t *event, uint32_t data_len,
			       struct auth_offload_event *auth_event);

/**
 * wmi_extract_roam_pmkid_request - Extract roam pmkid list
 * @wmi_handle: WMI handle
 * @event: Event data received from firmware
 * @data_len: Event data length received from firmware
 * @data: Extract the event and fill in data
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_extract_roam_pmkid_request(wmi_unified_t wmi_handle,
			       uint8_t *event, uint32_t data_len,
			       struct roam_pmkid_req_event **data);

/**
 * wmi_extract_roam_candidate_frame_event() - Extract the roam candidate
 * scan entry and update the scan db
 * @wmi_handle: wmi handle
 * @event: Event data received from firmware
 * @len: Event data length received from firmware
 * @data: Extract the event and fill in data
 */
QDF_STATUS
wmi_extract_roam_candidate_frame_event(wmi_unified_t wmi_handle, uint8_t *event,
				       uint32_t len,
				       struct roam_scan_candidate_frame *data);
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */

/**
 * wmi_unified_roam_scan_offload_mode_cmd() - set roam scan parameters
 * @wmi_handle: wmi handle
 * @scan_cmd_fp: scan related parameters
 * @rso_cfg: roam scan offload parameters
 *
 * This function reads the incoming @rso_cfg and fill in the destination
 * WMI structure and send down the roam scan configs down to the firmware
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_roam_scan_offload_mode_cmd(
			wmi_unified_t wmi_handle,
			struct wlan_roam_scan_offload_params *rso_cfg);

/**
 * wmi_unified_send_roam_scan_offload_ap_cmd() - set roam ap profile in fw
 * @wmi_handle: wmi handle
 * @ap_profile: ap profile params
 *
 * Send WMI_ROAM_AP_PROFILE to firmware
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_send_roam_scan_offload_ap_cmd(
				wmi_unified_t wmi_handle,
				struct ap_profile_params *ap_profile);

/**
 * wmi_unified_roam_scan_offload_cmd() - set roam offload command
 * @wmi_handle: wmi handle
 * @command: command
 * @vdev_id: vdev id
 *
 * This function set roam offload command to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_roam_scan_offload_cmd(wmi_unified_t wmi_handle,
					     uint32_t command,
					     uint32_t vdev_id);

/**
 * wmi_unified_roam_scan_offload_chan_list_cmd  - Roam scan offload channel
 * list command
 * @wmi_handle: wmi handle
 * @rso_ch_info: roam scan offload channel info
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wmi_unified_roam_scan_offload_chan_list_cmd(wmi_unified_t wmi_handle,
			struct wlan_roam_scan_channel_list *rso_ch_info);

/**
 * wmi_unified_roam_scan_offload_rssi_change_cmd() - set roam offload RSSI
 * threshold
 * @wmi_handle: wmi handle
 * @params: RSSI change params
 *
 * Send WMI_ROAM_SCAN_RSSI_CHANGE_THRESHOLD parameters to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_roam_scan_offload_rssi_change_cmd(
		wmi_unified_t wmi_handle,
		struct wlan_roam_rssi_change_params *params);

/**
 * wmi_unified_set_per_roam_config() - set PER roam config in FW
 * @wmi_handle: wmi handle
 * @req_buf: per roam config request buffer
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_set_per_roam_config(wmi_unified_t wmi_handle,
				struct wlan_per_roam_config_req *req_buf);

/**
 * wmi_unified_send_limit_off_chan_cmd() - send wmi cmd of limit off channel
 * configuration params
 * @wmi_handle:  wmi handler
 * @wmi_param: pointer to wmi_limit_off_chan_param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF failure reason code on failure
 */
QDF_STATUS wmi_unified_send_limit_off_chan_cmd(
		wmi_unified_t wmi_handle,
		struct wmi_limit_off_chan_param *wmi_param);

#ifdef WLAN_FEATURE_FILS_SK
/*
 * wmi_unified_roam_send_hlp_cmd() -send HLP command info
 * @wmi_handle: wma handle
 * @req_buf: Pointer to HLP params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_roam_send_hlp_cmd(wmi_unified_t wmi_handle,
					 struct hlp_params *req_buf);
#endif /* WLAN_FEATURE_FILS_SK */

/**
 * wmi_unified_send_btm_config() - Send BTM config to fw
 * @wmi_handle:  wmi handle
 * @params: pointer to wlan_roam_btm_config
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wmi_unified_send_btm_config(wmi_unified_t wmi_handle,
				       struct wlan_roam_btm_config *params);

/**
 * wmi_unified_send_bss_load_config() - Send bss load trigger params to fw
 * @wmi_handle:  wmi handle
 * @params: pointer to wlan_roam_bss_load_config
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wmi_unified_send_bss_load_config(
				wmi_unified_t wmi_handle,
				struct wlan_roam_bss_load_config *params);

/**
 * wmi_unified_offload_11k_cmd() - send 11k offload command
 * @wmi_handle: wmi handle
 * @params: 11k offload params
 *
 * This function passes the 11k offload command params to FW
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_offload_11k_cmd(wmi_unified_t wmi_handle,
			    struct wlan_roam_11k_offload_params *params);
/**
 * wmi_unified_invoke_neighbor_report_cmd() - send invoke neighbor report cmd
 * @wmi_handle: wmi handle
 * @params: invoke neighbor report params
 *
 * This function passes the invoke neighbor report command to fw
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_invoke_neighbor_report_cmd(
			wmi_unified_t wmi_handle,
			struct wmi_invoke_neighbor_report_params *params);

/**
 * wmi_unified_get_roam_scan_ch_list() - send roam scan channel list get cmd
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 *
 * This function sends roam scan channel list get command to firmware.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_get_roam_scan_ch_list(wmi_unified_t wmi_handle,
					     uint8_t vdev_id);

#endif /* _WMI_UNIFIED_ROAM_API_H_ */
