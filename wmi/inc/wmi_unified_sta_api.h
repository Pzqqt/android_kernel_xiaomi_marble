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

/**
 * DOC: Implement API's specific to STA component.
 */

#ifndef _WMI_UNIFIED_STA_API_H_
#define _WMI_UNIFIED_STA_API_H_

#include "wlan_disa_public_struct.h"
#include "wlan_tdls_public_structs.h"
#include "wlan_policy_mgr_public_struct.h"
#include "wmi_unified_sta_param.h"

struct policy_mgr_dual_mac_config;

/**
 * wmi_unified_set_sta_sa_query_param_cmd() - set sta sa query parameters
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @max_retries: max retries
 * @retry_interval: retry interval
 * This function sets sta query related parameters in fw.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_sta_sa_query_param_cmd(wmi_unified_t wmi_handle,
						  uint8_t vdev_id,
						  uint32_t max_retries,
						  uint32_t retry_interval);

/**
 * wmi_unified_set_sta_keep_alive_cmd() - set sta keep alive parameters
 * @wmi_handle: wmi handle
 * @params: sta keep alive parameter
 *
 * This function sets keep alive related parameters in fw.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_set_sta_keep_alive_cmd(wmi_unified_t wmi_handle,
				   struct sta_keep_alive_params *params);

/**
 * wmi_unified_vdev_set_gtx_cfg_cmd() - set GTX params
 * @wmi_handle: wmi handle
 * @if_id: vdev id
 * @gtx_info: GTX config params
 *
 * This function set GTX related params in firmware.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_vdev_set_gtx_cfg_cmd(wmi_unified_t wmi_handle, uint32_t if_id,
				 struct wmi_gtx_config *gtx_info);

#ifdef FEATURE_BLACKLIST_MGR
/**
 * wmi_unified_send_reject_ap_list() - send the reject ap list maintained by
 * BLM to FW for roaming cases.
 * @wmi_handle: wmi handle
 * @reject_params: This contains the reject ap list, and the num of BSSIDs.
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS
wmi_unified_send_reject_ap_list(struct wmi_unified *wmi_handle,
				struct reject_ap_params *reject_params);
#endif

/**
 * wmi_unified_process_dhcp_ind() - process dhcp indication from SME
 * @wmi_handle: wmi handle
 * @ta_dhcp_ind: DHCP indication parameter
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_process_dhcp_ind(
			wmi_unified_t wmi_handle,
			wmi_peer_set_param_cmd_fixed_param *ta_dhcp_ind);

/**
 * wmi_unified_get_link_speed_cmd() -send command to get linkspeed
 * @wmi_handle: wmi handle
 * @peer_macaddr: peer MAC address
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_get_link_speed_cmd(wmi_unified_t wmi_handle,
					  wmi_mac_addr peer_macaddr);

/**
 * wmi_unified_fw_profiling_data_cmd() - send FW profiling cmd to WLAN FW
 * @wmi_handle: wmi handle
 * @cmd: Profiling command index
 * @value1: parameter1 value
 * @value2: parameter2 value
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_fw_profiling_data_cmd(wmi_unified_t wmi_handle,
					     uint32_t cmd,
					     uint32_t value1,
					     uint32_t value2);

/**
 * wmi_unified_nat_keepalive_en_cmd() - enable NAT keepalive filter
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_nat_keepalive_en_cmd(wmi_unified_t wmi_handle,
					    uint8_t vdev_id);

/**
 * wmi_unified_wlm_latency_level_cmd() - set latency level
 * @wmi_handle: wmi handle
 * @param: WLM parameters
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_wlm_latency_level_cmd(wmi_unified_t wmi_handle,
				  struct wlm_latency_level_param *param);

/**
 * wmi_unified_process_set_ie_info_cmd() - Function to send IE info to firmware
 * @wmi_handle:    Pointer to WMi handle
 * @ie_data:       Pointer for ie data
 *
 * This function sends IE information to firmware
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_process_set_ie_info_cmd(wmi_unified_t wmi_handle,
				    struct vdev_ie_info_param *ie_info);

/**
 * wmi_unified_set_base_macaddr_indicate_cmd() - set base mac address in fw
 * @wmi_handle: wmi handle
 * @custom_addr: base mac address
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_base_macaddr_indicate_cmd(wmi_unified_t wmi_handle,
						     uint8_t *custom_addr);

#ifdef FEATURE_WLAN_TDLS
/**
 * wmi_unified_set_tdls_offchan_mode_cmd() - set tdls off channel mode
 * @wmi_handle: wmi handle
 * @chan_switch_params: Pointer to tdls channel switch parameter structure
 *
 * This function sets tdls off channel mode
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failures;
 *         Negative errno otherwise
 */
QDF_STATUS wmi_unified_set_tdls_offchan_mode_cmd(
		wmi_unified_t wmi_handle,
		struct tdls_channel_switch_params *chan_switch_params);

/**
 * wmi_unified_update_fw_tdls_state_cmd() - send enable/disable tdls for a vdev
 * @wmi_handle: wmi handle
 * @tdls_param: TDLS params
 * @tdls_state: TDLS state
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_update_fw_tdls_state_cmd(wmi_unified_t wmi_handle,
						struct tdls_info *tdls_param,
						enum wmi_tdls_state tdls_state);

/**
 * wmi_unified_update_tdls_peer_state_cmd() - update TDLS peer state
 * @wmi_handle: wmi handle
 * @peer_state: TDLS peer state params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_update_tdls_peer_state_cmd(wmi_unified_t wmi_handle,
				struct tdls_peer_update_state *peer_state,
				uint32_t *ch_mhz);

/**
 * wmi_extract_vdev_tdls_ev_param - extract vdev tdls param from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @param: Pointer to hold vdev tdls param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_vdev_tdls_ev_param(wmi_unified_t wmi_handle,
					  void *evt_buf,
					  struct tdls_event_info *param);
#endif /* FEATURE_WLAN_TDLS */

/**
 * wmi_unified_send_sar_limit_cmd() - send sar limit cmd to fw
 * @wmi_handle: wmi handle
 * @params: sar limit command params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_send_sar_limit_cmd(wmi_unified_t wmi_handle,
					  struct sar_limit_cmd_params *params);

/**
 * wmi_unified_get_sar_limit_cmd() - request current SAR limits from FW
 * @wmi_handle: wmi handle
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_unified_get_sar_limit_cmd(wmi_unified_t wmi_handle);

/**
 * wmi_unified_extract_sar_limit_event() - extract SAR limits from FW event
 * @wmi_handle: wmi handle
 * @evt_buf: event buffer received from firmware
 * @event: SAR limit event which is to be populated by data extracted from
 *         the @evt_buf buffer
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_unified_extract_sar_limit_event(wmi_unified_t wmi_handle,
					       uint8_t *evt_buf,
					       struct sar_limit_event *event);

/**
 * wmi_unified_extract_sar2_result_event() - extract SAR limits from FW event
 * @handle: wmi handle
 * @event: event buffer received from firmware
 * @len: length of the event buffer
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_unified_extract_sar2_result_event(void *handle,
						 uint8_t *event, uint32_t len);

/*
 * wmi_unified_set_del_pmkid_cache() - set delete PMKID
 * @wmi_handle: wma handle
 * @pmksa: pointer to pmk cache entry
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_set_del_pmkid_cache(wmi_unified_t wmi_handle,
				struct wmi_unified_pmk_cache *pmksa);

/**
 * wmi_unified_del_ts_cmd() - send DELTS request to fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @ac: ac param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_del_ts_cmd(wmi_unified_t wmi_handle, uint8_t vdev_id,
				  uint8_t ac);

/**
 * wmi_unified_aggr_qos_cmd() - send aggr qos request to fw
 * @wmi_handle: handle to wmi
 * @aggr_qos_rsp_msg: combined struct for all ADD_TS requests.
 *
 * A function to handle WMI_AGGR_QOS_REQ. This will send out
 * ADD_TS requestes to firmware in loop for all the ACs with
 * active flow.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_aggr_qos_cmd(
		wmi_unified_t wmi_handle,
		struct aggr_add_ts_param *aggr_qos_rsp_msg);

/**
 * wmi_unified_add_ts_cmd() - send ADDTS request to fw
 * @wmi_handle: wmi handle
 * @msg: ADDTS params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_add_ts_cmd(wmi_unified_t wmi_handle,
				  struct add_ts_param *msg);

/**
 * wmi_unified_process_add_periodic_tx_ptrn_cmd() - add periodic tx pattern
 * @wmi_handle: wmi handle
 * @pattern: tx pattern parameters
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_process_add_periodic_tx_ptrn_cmd(
					wmi_unified_t wmi_handle,
					struct periodic_tx_pattern *pattern,
					uint8_t vdev_id);

/**
 * wmi_unified_process_del_periodic_tx_ptrn_cmd() - del periodic tx ptrn
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @pattern_id: pattern id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_process_del_periodic_tx_ptrn_cmd(wmi_unified_t wmi_handle,
					     uint8_t vdev_id,
					     uint8_t pattern_id);

/**
 * wmi_unified_set_auto_shutdown_timer_cmd() - sets auto shutdown
 *					       timer in firmware
 * @wmi_handle: wmi handle
 * @timer_val: auto shutdown timer value
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_auto_shutdown_timer_cmd(wmi_unified_t wmi_handle,
						   uint32_t timer_val);

/**
 * wmi_unified_set_led_flashing_cmd() - set led flashing in fw
 * @wmi_handle: wmi handle
 * @flashing: flashing request
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_set_led_flashing_cmd(wmi_unified_t wmi_handle,
				 struct flashing_req_params *flashing);

/**
 * wmi_unified_process_ch_avoid_update_cmd() - handles channel avoid
 *					       update request
 * @wmi_handle: wmi handle
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_process_ch_avoid_update_cmd(wmi_unified_t wmi_handle);

/**
 * wmi_unified_pdev_set_pcl_cmd() - Send WMI_SOC_SET_PCL_CMDID to FW
 * @wmi_handle: wmi handle
 * @msg: PCL structure containing the PCL and the number of channels
 *
 * WMI_SOC_SET_PCL_CMDID provides a Preferred Channel List (PCL) to the WLAN
 * firmware. The DBS Manager is the consumer of this information in the WLAN
 * firmware. The channel list will be used when a Virtual DEVice (VDEV) needs
 * to migrate to a new channel without host driver involvement. An example of
 * this behavior is Legacy Fast Roaming (LFR 3.0). Generally, the host will
 * manage the channel selection without firmware involvement.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_pdev_set_pcl_cmd(wmi_unified_t wmi_handle,
					struct wmi_pcl_chan_weights *msg);

#ifdef WLAN_POLICY_MGR_ENABLE
/**
 * wmi_unified_pdev_set_dual_mac_config_cmd() - Set dual mac config to FW
 * @wmi_handle: wmi handle
 * @msg: Dual MAC config parameters
 *
 * Configures WLAN firmware with the dual MAC features
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failures.
 */
QDF_STATUS wmi_unified_pdev_set_dual_mac_config_cmd(
		wmi_unified_t wmi_handle,
		struct policy_mgr_dual_mac_config *msg);
#endif /* WLAN_POLICY_MGR_ENABLE */

/**
 * wmi_unified_send_adapt_dwelltime_params_cmd() - send wmi cmd of
 * adaptive dwelltime configuration params
 * @wma_handle:  wma handler
 * @wmi_param: pointer to dwelltime_params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF failure reason code for failure
 */
QDF_STATUS wmi_unified_send_adapt_dwelltime_params_cmd(
		wmi_unified_t wmi_handle,
		struct wmi_adaptive_dwelltime_params *wmi_param);

/**
 * wmi_unified_send_dbs_scan_sel_params_cmd() - send wmi cmd of
 * DBS scan selection configuration params
 * @wma_handle:  wma handler
 * @wmi_param: pointer to wmi_dbs_scan_sel_params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF failure reason code for failure
 */
QDF_STATUS wmi_unified_send_dbs_scan_sel_params_cmd(
			wmi_unified_t wmi_handle,
			struct wmi_dbs_scan_sel_params *wmi_param);

/**
 * wmi_unified_set_arp_stats_req() - set arp stats request
 * @wmi_handle: wmi handle
 * @req_buf: pointer to set_arp_stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_arp_stats_req(wmi_unified_t wmi_handle,
					 struct set_arp_stats *req_buf);

/**
 * wmi_unified_get_arp_stats_req() - get arp stats request
 * @wmi_handle: wmi handle
 * @req_buf: pointer to get_arp_stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_get_arp_stats_req(wmi_unified_t wmi_handle,
					 struct get_arp_stats *req_buf);

/**
 * wmi_unified_peer_unmap_conf_send() - send PEER unmap conf command to fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @peer_id_cnt: number of peer id
 * @peer_id_list: list of peer ids
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_peer_unmap_conf_send(wmi_unified_t wmi_handle,
					    uint8_t vdev_id,
					    uint32_t peer_id_cnt,
					    uint16_t *peer_id_list);

#endif /* _WMI_UNIFIED_STA_API_H_ */
