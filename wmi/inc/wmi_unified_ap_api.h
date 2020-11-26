/*
 * Copyright (c) 2013-2018,2020 The Linux Foundation. All rights reserved.
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
 * This file contains the API definitions for the generic AP WMIs
 */

#ifndef _WMI_UNIFIED_AP_API_H_
#define _WMI_UNIFIED_AP_API_H_

#include "wmi_unified_param.h"

/**
 *  wmi_unified_beacon_send_cmd() - WMI beacon send function
 *  @wmi_handle: handle to WMI.
 *  @macaddr: MAC address
 *  @param: pointer to hold beacon send cmd parameter
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_beacon_send_cmd(wmi_unified_t wmi_handle,
				       struct beacon_params *param);

/**
 *  wmi_unified_pdev_get_tpc_config_cmd_send() - WMI get tpc config function
 *  @wmi_handle: handle to WMI.
 *  @param: tpc config param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_pdev_get_tpc_config_cmd_send(wmi_unified_t wmi_handle,
						    uint32_t param);

/**
 * wmi_send_pdev_caldata_version_check_cmd() - send reset peer mumimo
 *                                             tx count to fw
 * @wmi_handle: wmi handle
 * @value: value
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_send_pdev_caldata_version_check_cmd(wmi_unified_t wmi_handle,
						   uint32_t value);

/**
 *  wmi_unified_set_ht_ie_cmd_send() - WMI set channel cmd function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold channel param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_ht_ie_cmd_send(wmi_unified_t wmi_handle,
					  struct ht_ie_params *param);

/**
 *  wmi_unified_set_vht_ie_cmd_send() - WMI set channel cmd function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold channel param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_vht_ie_cmd_send(wmi_unified_t wmi_handle,
					   struct vht_ie_params *param);

/**
 *  wmi_unified_set_ctl_table_cmd_send() - WMI ctl table cmd function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold ctl table param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_ctl_table_cmd_send(wmi_unified_t wmi_handle,
					      struct ctl_table_params *param);

/**
 *  wmi_unified_set_mimogain_table_cmd_send() - WMI set mimogain cmd function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold mimogain param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_mimogain_table_cmd_send(
		wmi_unified_t wmi_handle,
		struct mimogain_table_params *param);

/**
 *  wmi_unified_peer_add_wds_entry_cmd_send() - WMI add wds entry cmd function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold wds entry param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_peer_add_wds_entry_cmd_send(
		wmi_unified_t wmi_handle,
		struct peer_add_wds_entry_params *param);

/**
 *  wmi_unified_peer_del_wds_entry_cmd_send() - WMI del wds entry cmd function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold wds entry param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_peer_del_wds_entry_cmd_send(
		wmi_unified_t wmi_handle,
		struct peer_del_wds_entry_params *param);

/**
 *  wmi_unified_peer_update_wds_entry_cmd_send() - WMI update wds entry
 *  cmd function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold wds entry param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_peer_update_wds_entry_cmd_send(
		wmi_unified_t wmi_handle,
		struct peer_update_wds_entry_params *param);


/**
 * wmi_unified_peer_ft_roam_send() - reset BA window in fw
 * @wmi_handle: wmi handle
 * @peer_addr: peer mac address
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_peer_ft_roam_send(wmi_unified_t wmi_handle,
			      uint8_t peer_addr[QDF_MAC_ADDR_SIZE],
			      uint8_t vdev_id);

/**
 *  wmi_unified_vdev_set_neighbour_rx_cmd_send() - WMI set neighbour rx function
 *  @wmi_handle: handle to WMI.
 *  @macaddr: MAC address
 *  @param: pointer to hold neighbour rx parameter
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_set_neighbour_rx_cmd_send(
		wmi_unified_t wmi_handle,
		uint8_t macaddr[QDF_MAC_ADDR_SIZE],
		struct set_neighbour_rx_params *param);

/**
 *  wmi_unified_vdev_config_ratemask_cmd_send() - WMI config ratemask function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold config ratemask param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_config_ratemask_cmd_send(
		wmi_unified_t wmi_handle,
		struct config_ratemask_params *param);

/**
 *  wmi_unified_set_quiet_mode_cmd_send() - WMI set quiet mode function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold quiet mode param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_quiet_mode_cmd_send(
		wmi_unified_t wmi_handle,
		struct set_quiet_mode_params *param);

/**
 *  wmi_unified_set_bcn_offload_quiet_mode_cmd_send - WMI set quiet mode
 *      function in beacon offload case
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold quiet mode param in bcn offload
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_bcn_offload_quiet_mode_cmd_send(
		wmi_unified_t wmi_handle,
		struct set_bcn_offload_quiet_mode_params *param);

/**
 *  wmi_unified_nf_dbr_dbm_info_get_cmd_send() - WMI request nf info function
 *  @wmi_handle: handle to WMI.
 *  @mac_id: mac_id
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_nf_dbr_dbm_info_get_cmd_send(wmi_unified_t wmi_handle,
						    uint8_t mac_id);

/**
 *  wmi_unified_packet_power_info_get_cmd_send() - WMI get packet power
 *	info function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold packet power info param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_packet_power_info_get_cmd_send(
		wmi_unified_t wmi_handle,
		struct packet_power_info_params *param);

/**
 *  wmi_extract_wds_addr_event - Extract WDS addr WMI event
 *  @wmi_handle: handle to WMI.
 *  @evt_buf: pointer to event buffer
 *  @len: length of the event buffer
 *  @wds_ev: pointer to strct to extract
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_wds_addr_event(
		wmi_unified_t wmi_handle,
		void *evt_buf, uint16_t len, wds_addr_event_t *wds_ev);

/**
 * wmi_extract_dcs_interference_type() - extract dcs interference type
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @param: Pointer to hold dcs interference param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_dcs_interference_type(
		wmi_unified_t wmi_handle,
		void *evt_buf, struct wmi_host_dcs_interference_param *param);

/*
 * wmi_extract_dcs_cw_int() - extract dcs cw interference from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @cw_int: Pointer to hold cw interference
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_dcs_cw_int(wmi_unified_t wmi_handle, void *evt_buf,
				  wmi_host_ath_dcs_cw_int *cw_int);

/**
 * wmi_extract_dcs_im_tgt_stats() - extract dcs im target stats from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @wlan_stat: Pointer to hold wlan stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_dcs_im_tgt_stats(
		wmi_unified_t wmi_handle, void *evt_buf,
		wmi_host_dcs_im_tgt_stats_t *wlan_stat);

/**
 * wmi_extract_tbttoffset_update_params() - extract tbtt offset update param
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @idx: Index referring to a vdev
 * @tbtt_param: Pointer to tbttoffset event param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_tbttoffset_update_params(
		wmi_unified_t wmi_handle, void *evt_buf,
		uint8_t idx, struct tbttoffset_params *tbtt_param);

/**
 * wmi_extract_ext_tbttoffset_update_params() - extract tbtt offset update param
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @idx: Index referring to a vdev
 * @tbtt_param: Pointer to tbttoffset event param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_ext_tbttoffset_update_params(
		wmi_unified_t wmi_handle,
		void *evt_buf, uint8_t idx,
		struct tbttoffset_params *tbtt_param);

/**
 * wmi_extract_tbttoffset_num_vdevs() - extract tbtt offset num vdev
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @vdev_map: Pointer to hold num vdev
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_tbttoffset_num_vdevs(wmi_unified_t wmi_handle,
					    void *evt_buf,
					    uint32_t *num_vdevs);

/**
 * wmi_extract_ext_tbttoffset_num_vdevs() - extract ext tbtt offset num vdev
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @vdev_map: Pointer to hold num vdev
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_ext_tbttoffset_num_vdevs(wmi_unified_t wmi_handle,
						void *evt_buf,
						uint32_t *num_vdevs);

/**
 * wmi_extract_pdev_caldata_version_check_ev_param() - extract caldata
 *                                                     from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @param: Pointer to hold caldata version data
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_pdev_caldata_version_check_ev_param(
		wmi_unified_t wmi_handle,
		void *evt_buf, wmi_host_pdev_check_cal_version_event *param);

/**
 * wmi_extract_pdev_tpc_config_ev_param() - extract pdev tpc configuration
 * param from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @param: Pointer to hold tpc configuration
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_pdev_tpc_config_ev_param(
		wmi_unified_t wmi_handle, void *evt_buf,
		wmi_host_pdev_tpc_config_event *param);

/**
 * wmi_extract_nfcal_power_ev_param() - extract noise floor calibration
 * power param from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @param: Pointer to hold nf cal power param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_nfcal_power_ev_param(
		wmi_unified_t wmi_handle, void *evt_buf,
		wmi_host_pdev_nfcal_power_all_channels_event *param);

/**
 * wmi_extract_pdev_tpc_ev_param() - extract tpc param from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @param: Pointer to hold tpc param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_pdev_tpc_ev_param(
		wmi_unified_t wmi_handle, void *evt_buf,
		wmi_host_pdev_tpc_event *param);

/**
 * wmi_extract_offchan_data_tx_compl_param() -
 *          extract offchan data tx completion param from event
 * @wmi_hdl: wmi handle
 * @evt_buf: pointer to event buffer
 * @param: Pointer to offchan data tx completion param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_offchan_data_tx_compl_param(
		wmi_unified_t wmi_handle, void *evt_buf,
		struct wmi_host_offchan_data_tx_compl_event *param);

/**
 * wmi_extract_swba_num_vdevs() - extract swba num vdevs from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @num_vdevs: Pointer to hold num vdevs
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_swba_num_vdevs(wmi_unified_t wmi_handle, void *evt_buf,
				      uint32_t *num_vdevs);

/**
 * wmi_extract_swba_tim_info() - extract swba tim info from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @idx: Index to bcn info
 * @tim_info: Pointer to hold tim info
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_swba_tim_info(
		wmi_unified_t wmi_handle, void *evt_buf,
		uint32_t idx, wmi_host_tim_info *tim_info);

/**
 * wmi_extract_swba_quiet_info() - extract swba quiet info from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @idx: Index to bcn info
 * @quiet_info: Pointer to hold quiet info
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_swba_quiet_info(wmi_unified_t wmi_handle, void *evt_buf,
				       uint32_t idx,
				       wmi_host_quiet_info *quiet_info);

/**
 * wmi_extract_swba_noa_info() - extract swba NoA information from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @idx: Index to bcn info
 * @p2p_desc: Pointer to hold p2p NoA info
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_swba_noa_info(wmi_unified_t wmi_handle, void *evt_buf,
				     uint32_t idx,
				     wmi_host_p2p_noa_info *p2p_desc);

/**
 * wmi_extract_peer_sta_ps_statechange_ev() - extract peer sta ps state
 * from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @ev: Pointer to hold peer param and ps state
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_peer_sta_ps_statechange_ev(
		wmi_unified_t wmi_handle,
		void *evt_buf, wmi_host_peer_sta_ps_statechange_event *ev);

/**
 * wmi_extract_peer_sta_kickout_ev() - extract peer sta kickout event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @ev: Pointer to hold peer param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_peer_sta_kickout_ev(
		wmi_unified_t wmi_handle, void *evt_buf,
		wmi_host_peer_sta_kickout_event *ev);

/**
 * wmi_extract_inst_rssi_stats_event() - extract inst rssi stats from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @inst_rssi_resp: Pointer to hold inst rssi response
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_inst_rssi_stats_event(
		wmi_unified_t wmi_handle, void *evt_buf,
		wmi_host_inst_stats_resp *inst_rssi_resp);

/**
 * wmi_unified_send_multiple_vdev_restart_req_cmd() - send multiple vdev restart
 * @wmi_handle: wmi handle
 * @param: multiple vdev restart parameter
 *
 * Send WMI_PDEV_MULTIPLE_VDEV_RESTART_REQUEST_CMDID parameters to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wmi_unified_send_multiple_vdev_restart_req_cmd(
		wmi_unified_t wmi_handle,
		struct multiple_vdev_restart_params *param);

/**
 * wmi_extract_peer_create_response_event() -
 * extract vdev id and peer mac address and status from peer create
 * response event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @param: Pointer to hold evt buf
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_extract_peer_create_response_event(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wmi_host_peer_create_response_event *param);

/**
 * wmi_extract_peer_delete_response_event() -
 *       extract vdev id and peer mac addresse from peer delete response event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @param: Pointer to hold evt buf
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_extract_peer_delete_response_event(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wmi_host_peer_delete_response_event *param);

/**
 * wmi_send_bcn_offload_control_cmd - send beacon ofload control cmd to fw
 * @wmi_hdl: wmi handle
 * @bcn_ctrl_param: pointer to bcn_offload_control param
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_send_bcn_offload_control_cmd(
		wmi_unified_t wmi_handle,
		struct bcn_offload_control *bcn_ctrl_param);

#ifdef WLAN_SUPPORT_FILS
/**
 * wmi_unified_fils_vdev_config_send_cmd() - send FILS config cmd to fw
 * @wmi_hdl: wmi handle
 * @param: fils config params
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS
wmi_unified_fils_vdev_config_send_cmd(wmi_unified_t wmi_handle,
				      struct config_fils_params *param);

/**
 * wmi_extract_swfda_vdev_id() - api to extract vdev id
 * @wmi_hdl: wmi handle
 * @evt_buf: pointer to event buffer
 * @vdev_id: pointer to vdev id
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_extract_swfda_vdev_id(wmi_unified_t wmi_handle, void *evt_buf,
				     uint32_t *vdev_id);

/**
 * wmi_unified_fils_discovery_send_cmd() - send FILS discovery cmd to fw
 * @wmi_hdl: wmi handle
 * @param: fils discovery params
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_unified_fils_discovery_send_cmd(wmi_unified_t wmi_handle,
					       struct fd_params *param);
#endif /* WLAN_SUPPORT_FILS */

/**
 *  wmi_unified_set_qboost_param_cmd_send() - WMI set_qboost function
 *  @wmi_handle: handle to WMI.
 *  @macaddr: MAC address
 *  @param: pointer to hold set_qboost parameter
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_set_qboost_param_cmd_send(wmi_unified_t wmi_handle,
				      uint8_t macaddr[QDF_MAC_ADDR_SIZE],
				      struct set_qboost_params *param);

/**
 *  wmi_unified_mcast_group_update_cmd_send() - WMI mcast grp update cmd function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold mcast grp param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_mcast_group_update_cmd_send(wmi_unified_t wmi_handle,
					struct mcast_group_update_params *param);

/**
 *  wmi_unified_pdev_qvit_cmd_send() - WMI pdev qvit cmd function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold qvit param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_pdev_qvit_cmd_send(wmi_unified_t wmi_handle,
					  struct pdev_qvit_params *param);

/**
 *  wmi_unified_wmm_update_cmd_send() - WMI wmm update cmd function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold wmm param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_wmm_update_cmd_send(wmi_unified_t wmi_handle,
					   struct wmm_update_params *param);

/**
 * wmi_extract_vdev_start_resp() - extract vdev start response
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @vdev_rsp: Pointer to hold vdev response
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_vdev_start_resp(
		wmi_unified_t wmi_handle, void *evt_buf,
		struct vdev_start_response *vdev_rsp);

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
QDF_STATUS wmi_extract_vdev_delete_resp(
		wmi_unified_t wmi_handle, void *evt_buf,
		struct vdev_delete_response *delele_rsp);

/**
 * wmi_extract_vdev_stopped_param() - extract vdev stop param from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @vdev_id: Pointer to hold vdev identifier
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_vdev_stopped_param(wmi_unified_t wmi_handle,
					  void *evt_buf,
					  uint32_t *vdev_id);

/**
 * wmi_extract_mgmt_tx_compl_param() - extract mgmt tx completion param
 * from event
 * @wmi_hdl: wmi handle
 * @evt_buf: pointer to event buffer
 * @param: Pointer to mgmt tx completion param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_mgmt_tx_compl_param(
		wmi_unified_t wmi_handle, void *evt_buf,
		wmi_host_mgmt_tx_compl_event *param);

/**
 * wmi_extract_chan_info_event() - extract chan information from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @chan_info: Pointer to hold chan information
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_chan_info_event(
		wmi_unified_t wmi_handle, void *evt_buf,
		wmi_host_chan_info_event *chan_info);

/**
 * wmi_extract_channel_hopping_event() - extract channel hopping param
 * from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @ch_hopping: Pointer to hold channel hopping param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_channel_hopping_event(
		wmi_unified_t wmi_handle, void *evt_buf,
		wmi_host_pdev_channel_hopping_event *ch_hopping);

/**
 * wmi_unified_peer_chan_width_switch_cmd_send() - WMI send peer chan width
 * @wmi_hdl: handle to WMI
 * @param: pointer to hold peer capability param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_peer_chan_width_switch_cmd_send(
		wmi_unified_t wmi_handle,
		struct peer_chan_width_switch_params *param);

/**
 * wmi_unified_peer_del_all_wds_entries_cmd_send() - send delete
 * all wds entries cmd to fw
 * @wmi_hdl: wmi handle
 * @param: delete all wds entries params
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_unified_peer_del_all_wds_entries_cmd_send(
		wmi_unified_t wmi_handle,
		struct peer_del_all_wds_entries_params *param);

/**
 *  wmi_unified_vdev_pcp_tid_map_cmd_send() - WMI set vap pcp
 *  tid map cmd function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold pcp param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_pcp_tid_map_cmd_send(
		wmi_unified_t wmi_handle,
		struct vap_pcp_tid_map_params *param);


/**
 *  wmi_unified_vdev_tidmap_prec_cmd_send() - WMI set vap tidmap precedence
 *  cmd function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold precedence param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_tidmap_prec_cmd_send(
		wmi_unified_t wmi_handle,
		struct vap_tidmap_prec_params *param);

#ifdef WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG
/**
 * wmi_unified_set_rx_pkt_type_routing_tag() - api to add/delete
 * the protocols to be tagged by CCE
 * @wmi_hdl: wmi handle
 * @param: Packet routing/tagging info
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_unified_set_rx_pkt_type_routing_tag(
		wmi_unified_t wmi_handle,
		struct wmi_rx_pkt_protocol_routing_info *param);
#endif /* WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG */

/**
 * wmi_unified_peer_vlan_config_send() - WMI function to send vlan command
 *
 * @wmi_hdl: WMI handle
 * @peer_addr: Peer mac address
 * @param: struct peer_vlan_config_param *
 *
 * Return: QDF_STATUS_SUCCESS if success, else returns proper error code.
 */
QDF_STATUS wmi_unified_peer_vlan_config_send(wmi_unified_t wmi_handle,
		uint8_t peer_addr[QDF_MAC_ADDR_SIZE],
		struct peer_vlan_config_param *param);

/**
 * wmi_extract_muedca_params_handler() - WMI function to extract Muedca params
 *
 * @wmi_handle: WMI handle
 * @evt_buf: Event data buffer
 * @muedca_param_list: struct muedca_params
 *
 * Return: QDF_STATUS_SUCCESS if success, else returns proper error code.
 */
QDF_STATUS wmi_extract_muedca_params_handler(wmi_unified_t wmi_handle,
		void *evt_buf, struct muedca_params *muedca_param_list);

/**
 *  wmi_unified_set_radio_tx_mode_select_cmd_send() - WMI ant switch tbl cmd function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold tx mode selection param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_radio_tx_mode_select_cmd_send(
		wmi_unified_t wmi_handle,
		struct wmi_pdev_enable_tx_mode_selection *tx_mode_select_param);

#endif /* _WMI_UNIFIED_AP_API_H_ */
