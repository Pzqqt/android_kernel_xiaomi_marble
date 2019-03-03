/*
 * Copyright (c) 2013-2018 The Linux Foundation. All rights reserved.
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

QDF_STATUS wmi_unified_beacon_send_cmd(void *wmi_hdl,
				struct beacon_params *param);

QDF_STATUS wmi_unified_pdev_get_tpc_config_cmd_send(void *wmi_hdl,
				uint32_t param);

QDF_STATUS wmi_send_pdev_caldata_version_check_cmd(void *wmi_hdl,
				uint32_t value);

QDF_STATUS wmi_unified_set_ht_ie_cmd_send(void *wmi_hdl,
				struct ht_ie_params *param);

QDF_STATUS wmi_unified_set_vht_ie_cmd_send(void *wmi_hdl,
				struct vht_ie_params *param);

QDF_STATUS wmi_unified_set_ctl_table_cmd_send(void *wmi_hdl,
				struct ctl_table_params *param);

QDF_STATUS wmi_unified_set_mimogain_table_cmd_send(void *wmi_hdl,
				struct mimogain_table_params *param);

QDF_STATUS wmi_unified_peer_add_wds_entry_cmd_send(void *wmi_hdl,
				struct peer_add_wds_entry_params *param);

QDF_STATUS wmi_unified_peer_del_wds_entry_cmd_send(void *wmi_hdl,
				struct peer_del_wds_entry_params *param);

QDF_STATUS wmi_unified_peer_update_wds_entry_cmd_send(void *wmi_hdl,
				struct peer_update_wds_entry_params *param);

QDF_STATUS wmi_unified_vdev_set_neighbour_rx_cmd_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct set_neighbour_rx_params *param);

QDF_STATUS wmi_unified_vdev_config_ratemask_cmd_send(void *wmi_hdl,
				struct config_ratemask_params *param);

QDF_STATUS wmi_unified_set_quiet_mode_cmd_send(void *wmi_hdl,
				struct set_quiet_mode_params *param);

QDF_STATUS wmi_unified_set_bcn_offload_quiet_mode_cmd_send(void *wmi_hdl,
			struct set_bcn_offload_quiet_mode_params *param);

QDF_STATUS wmi_unified_nf_dbr_dbm_info_get_cmd_send(void *wmi_hdl,
						    uint8_t mac_id);

QDF_STATUS wmi_unified_packet_power_info_get_cmd_send(void *wmi_hdl,
				struct packet_power_info_params *param);

QDF_STATUS wmi_extract_wds_addr_event(void *wmi_hdl,
		void *evt_buf, uint16_t len, wds_addr_event_t *wds_ev);

QDF_STATUS wmi_extract_dcs_interference_type(void *wmi_hdl,
		void *evt_buf, struct wmi_host_dcs_interference_param *param);

QDF_STATUS wmi_extract_dcs_cw_int(void *wmi_hdl, void *evt_buf,
		wmi_host_ath_dcs_cw_int *cw_int);

QDF_STATUS wmi_extract_dcs_im_tgt_stats(void *wmi_hdl, void *evt_buf,
		wmi_host_dcs_im_tgt_stats_t *wlan_stat);

QDF_STATUS wmi_extract_tbttoffset_update_params(void *wmi_hdl, void *evt_buf,
		uint8_t idx, struct tbttoffset_params *tbtt_param);

QDF_STATUS wmi_extract_ext_tbttoffset_update_params(void *wmi_hdl,
		void *evt_buf, uint8_t idx,
		struct tbttoffset_params *tbtt_param);

QDF_STATUS wmi_extract_tbttoffset_num_vdevs(void *wmi_hdl, void *evt_buf,
					    uint32_t *num_vdevs);

QDF_STATUS wmi_extract_ext_tbttoffset_num_vdevs(void *wmi_hdl, void *evt_buf,
						uint32_t *num_vdevs);

QDF_STATUS wmi_extract_pdev_caldata_version_check_ev_param(void *wmi_hdl,
		void *evt_buf, wmi_host_pdev_check_cal_version_event *param);

QDF_STATUS wmi_extract_pdev_tpc_config_ev_param(void *wmi_hdl, void *evt_buf,
		wmi_host_pdev_tpc_config_event *param);

QDF_STATUS wmi_extract_nfcal_power_ev_param(void *wmi_hdl, void *evt_buf,
		wmi_host_pdev_nfcal_power_all_channels_event *param);

QDF_STATUS wmi_extract_pdev_tpc_ev_param(void *wmi_hdl, void *evt_buf,
		wmi_host_pdev_tpc_event *param);

QDF_STATUS wmi_extract_offchan_data_tx_compl_param(void *wmi_hdl, void *evt_buf,
		struct wmi_host_offchan_data_tx_compl_event *param);

QDF_STATUS wmi_extract_pdev_csa_switch_count_status(void *wmi_hdl,
		void *evt_buf,
		struct pdev_csa_switch_count_status *param);

QDF_STATUS wmi_extract_swba_num_vdevs(void *wmi_hdl, void *evt_buf,
		uint32_t *num_vdevs);

QDF_STATUS wmi_extract_swba_tim_info(void *wmi_hdl, void *evt_buf,
		 uint32_t idx, wmi_host_tim_info *tim_info);

QDF_STATUS wmi_extract_swba_quiet_info(void *wmi_hdl, void *evt_buf,
				       uint32_t idx,
				       wmi_host_quiet_info *quiet_info);

QDF_STATUS wmi_extract_swba_noa_info(void *wmi_hdl, void *evt_buf,
			uint32_t idx, wmi_host_p2p_noa_info *p2p_desc);

QDF_STATUS wmi_extract_peer_sta_ps_statechange_ev(void *wmi_hdl,
		void *evt_buf, wmi_host_peer_sta_ps_statechange_event *ev);

QDF_STATUS wmi_extract_peer_sta_kickout_ev(void *wmi_hdl, void *evt_buf,
		wmi_host_peer_sta_kickout_event *ev);

QDF_STATUS wmi_extract_inst_rssi_stats_event(void *wmi_hdl, void *evt_buf,
			wmi_host_inst_stats_resp *inst_rssi_resp);

QDF_STATUS wmi_unified_send_multiple_vdev_restart_req_cmd(void *wmi_hdl,
				struct multiple_vdev_restart_params *param);

QDF_STATUS wmi_extract_peer_delete_response_event(void *wmi_hdl,
		uint8_t *evt_buf,
		struct wmi_host_peer_delete_response_event *param);

/**
 * wmi_send_bcn_offload_control_cmd - send beacon ofload control cmd to fw
 * @wmi_hdl: wmi handle
 * @bcn_ctrl_param: pointer to bcn_offload_control param
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_send_bcn_offload_control_cmd(void *wmi_hdl,
			struct bcn_offload_control *bcn_ctrl_param);

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

QDF_STATUS wmi_unified_set_qboost_param_cmd_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct set_qboost_params *param);

QDF_STATUS wmi_unified_gpio_config_cmd_send(void *wmi_hdl,
				struct gpio_config_params *param);

QDF_STATUS wmi_unified_gpio_output_cmd_send(void *wmi_hdl,
				struct gpio_output_params *param);

QDF_STATUS wmi_unified_mcast_group_update_cmd_send(void *wmi_hdl,
				struct mcast_group_update_params *param);

QDF_STATUS wmi_unified_pdev_qvit_cmd_send(void *wmi_hdl,
				struct pdev_qvit_params *param);

QDF_STATUS wmi_unified_wmm_update_cmd_send(void *wmi_hdl,
				struct wmm_update_params *param);

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

QDF_STATUS wmi_extract_vdev_stopped_param(void *wmi_hdl, void *evt_buf,
		uint32_t *vdev_id);

QDF_STATUS wmi_extract_mgmt_tx_compl_param(void *wmi_hdl, void *evt_buf,
		wmi_host_mgmt_tx_compl_event *param);

QDF_STATUS wmi_extract_chan_info_event(void *wmi_hdl, void *evt_buf,
			wmi_host_chan_info_event *chan_info);

QDF_STATUS wmi_extract_channel_hopping_event(void *wmi_hdl, void *evt_buf,
			wmi_host_pdev_channel_hopping_event *ch_hopping);

QDF_STATUS wmi_unified_peer_chan_width_switch_cmd_send(void *wmi_hdl,
			struct peer_chan_width_switch_params *param);

/**
 * wmi_unified_peer_del_all_wds_entries_cmd_send() - send delete
 * all wds entries cmd to fw
 * @wmi_hdl: wmi handle
 * @param:   delete all wds entries params
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_unified_peer_del_all_wds_entries_cmd_send(void *wmi_hdl,
		struct peer_del_all_wds_entries_params *param);

/**
 *  wmi_unified_vdev_pcp_tid_map_cmd_send() - WMI set vap pcp
 *  tid map cmd function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold pcp param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_pcp_tid_map_cmd_send(void *wmi_hdl,
					 struct vap_pcp_tid_map_params *param);


/**
 *  wmi_unified_vdev_tidmap_prec_cmd_send() - WMI set vap tidmap precedence
 *  cmd function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold precedence param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_tidmap_prec_cmd_send(void *wmi_hdl,
					struct vap_tidmap_prec_params *param);
#endif /* _WMI_UNIFIED_AP_API_H_ */
