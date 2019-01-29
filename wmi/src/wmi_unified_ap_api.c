/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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

#include "wmi_unified_priv.h"
#include "wmi_unified_param.h"
#include "wmi_unified_ap_api.h"
#include "qdf_module.h"

/**
 *  wmi_unified_peer_add_wds_entry_cmd_send() - WMI add wds entry cmd function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold wds entry param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_peer_add_wds_entry_cmd_send(void *wmi_hdl,
				struct peer_add_wds_entry_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_peer_add_wds_entry_cmd)
		return wmi_handle->ops->send_peer_add_wds_entry_cmd(wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_peer_del_wds_entry_cmd_send() - WMI del wds entry cmd function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold wds entry param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_peer_del_wds_entry_cmd_send(void *wmi_hdl,
				struct peer_del_wds_entry_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_peer_del_wds_entry_cmd)
		return wmi_handle->ops->send_peer_del_wds_entry_cmd(wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_peer_update_wds_entry_cmd_send() - WMI update wds entry cmd function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold wds entry param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_peer_update_wds_entry_cmd_send(void *wmi_hdl,
				struct peer_update_wds_entry_params *param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->send_peer_update_wds_entry_cmd)
		return wmi->ops->send_peer_update_wds_entry_cmd(wmi, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_pdev_get_tpc_config_cmd_send() - WMI get tpc config function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : tpc config param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_pdev_get_tpc_config_cmd_send(void *wmi_hdl,
				uint32_t param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_pdev_get_tpc_config_cmd)
		return wmi_handle->ops->send_pdev_get_tpc_config_cmd(wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_beacon_send_cmd() - WMI beacon send function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr	: MAC address
 *  @param param    : pointer to hold beacon send cmd parameter
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_beacon_send_cmd(void *wmi_hdl,
				struct beacon_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_beacon_send_cmd)
		return wmi_handle->ops->send_beacon_send_cmd(wmi_handle,
				  param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_set_ctl_table_cmd_send() - WMI ctl table cmd function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold ctl table param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_ctl_table_cmd_send(void *wmi_hdl,
				struct ctl_table_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_ctl_table_cmd)
		return wmi_handle->ops->send_set_ctl_table_cmd(wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_set_mimogain_table_cmd_send() - WMI set mimogain cmd function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold mimogain param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_mimogain_table_cmd_send(void *wmi_hdl,
				struct mimogain_table_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_mimogain_table_cmd)
		return wmi_handle->ops->send_set_mimogain_table_cmd(wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_packet_power_info_get_cmd_send() - WMI get packet power info function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold packet power info param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_packet_power_info_get_cmd_send(void *wmi_hdl,
				struct packet_power_info_params *param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->send_packet_power_info_get_cmd)
		return wmi->ops->send_packet_power_info_get_cmd(wmi, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_vdev_config_ratemask_cmd_send() - WMI config ratemask function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold config ratemask param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_config_ratemask_cmd_send(void *wmi_hdl,
				struct config_ratemask_params *param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->send_vdev_config_ratemask_cmd)
		return wmi->ops->send_vdev_config_ratemask_cmd(wmi, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_vdev_set_neighbour_rx_cmd_send() - WMI set neighbour rx function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr	: MAC address
 *  @param param    : pointer to hold neighbour rx parameter
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_set_neighbour_rx_cmd_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct set_neighbour_rx_params *param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->send_vdev_set_neighbour_rx_cmd)
		return wmi->ops->send_vdev_set_neighbour_rx_cmd(wmi,
				  macaddr, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_nf_dbr_dbm_info_get_cmd_send() - WMI request nf info function
 *  @param wmi_handle	  : handle to WMI.
 *  @mac_id: mac_id
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_nf_dbr_dbm_info_get_cmd_send(void *wmi_hdl,
						    uint8_t mac_id)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->send_nf_dbr_dbm_info_get_cmd)
		return wmi->ops->send_nf_dbr_dbm_info_get_cmd(wmi, mac_id);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_set_ht_ie_cmd_send() - WMI set channel cmd function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold channel param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_ht_ie_cmd_send(void *wmi_hdl,
				struct ht_ie_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_ht_ie_cmd)
		return wmi_handle->ops->send_set_ht_ie_cmd(wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_set_vht_ie_cmd_send() - WMI set channel cmd function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold channel param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_vht_ie_cmd_send(void *wmi_hdl,
				struct vht_ie_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_vht_ie_cmd)
		return wmi_handle->ops->send_set_vht_ie_cmd(wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_set_quiet_mode_cmd_send() - WMI set quiet mode function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold quiet mode param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_quiet_mode_cmd_send(void *wmi_hdl,
				struct set_quiet_mode_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_quiet_mode_cmd)
		return wmi_handle->ops->send_set_quiet_mode_cmd(wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_set_bcn_offload_quiet_mode_cmd_send - WMI set quiet mode
 *	function in beacon offload case
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold quiet mode param in bcn offload
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_bcn_offload_quiet_mode_cmd_send(void *wmi_hdl,
			struct set_bcn_offload_quiet_mode_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->send_set_bcn_offload_quiet_mode_cmd)
		return wmi_handle->ops->send_set_bcn_offload_quiet_mode_cmd(
				wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_send_bcn_offload_control_cmd - send beacon ofload control cmd to fw
 * @wmi_hdl: wmi handle
 * @bcn_ctrl_param: pointer to bcn_offload_control param
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_send_bcn_offload_control_cmd(void *wmi_hdl,
			struct bcn_offload_control *bcn_ctrl_param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_bcn_offload_control_cmd)
		return wmi_handle->ops->send_bcn_offload_control_cmd(wmi_handle,
				bcn_ctrl_param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_tbttoffset_num_vdevs() - extract tbtt offset num vdev
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param vdev_map: Pointer to hold num vdev
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_tbttoffset_num_vdevs(void *wmi_hdl, void *evt_buf,
					    uint32_t *num_vdevs)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->extract_tbttoffset_num_vdevs)
		return wmi->ops->extract_tbttoffset_num_vdevs(wmi,
			evt_buf, num_vdevs);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_ext_tbttoffset_num_vdevs() - extract ext tbtt offset num vdev
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param vdev_map: Pointer to hold num vdev
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_ext_tbttoffset_num_vdevs(void *wmi_hdl, void *evt_buf,
					    uint32_t *num_vdevs)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->extract_ext_tbttoffset_num_vdevs)
		return wmi->ops->extract_ext_tbttoffset_num_vdevs(wmi,
			evt_buf, num_vdevs);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_tbttoffset_update_params() - extract tbtt offset update param
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param idx: Index referring to a vdev
 * @param tbtt_param: Pointer to tbttoffset event param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_tbttoffset_update_params(void *wmi_hdl, void *evt_buf,
	uint8_t idx, struct tbttoffset_params *tbtt_param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->extract_tbttoffset_update_params)
		return wmi->ops->extract_tbttoffset_update_params(wmi,
			evt_buf, idx, tbtt_param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_ext_tbttoffset_update_params() - extract tbtt offset update param
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param idx: Index referring to a vdev
 * @param tbtt_param: Pointer to tbttoffset event param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_ext_tbttoffset_update_params(void *wmi_hdl,
	void *evt_buf, uint8_t idx, struct tbttoffset_params *tbtt_param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->extract_ext_tbttoffset_update_params)
		return wmi->ops->extract_ext_tbttoffset_update_params(wmi,
			evt_buf, idx, tbtt_param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_offchan_data_tx_compl_param() -
 *	    extract offchan data tx completion param from event
 * @wmi_hdl: wmi handle
 * @evt_buf: pointer to event buffer
 * @param: Pointer to offchan data tx completion param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_offchan_data_tx_compl_param(void *wmi_hdl, void *evt_buf,
	struct wmi_host_offchan_data_tx_compl_event *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_offchan_data_tx_compl_param)
		return wmi_handle->ops->extract_offchan_data_tx_compl_param(
				wmi_handle, evt_buf, param);


	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_pdev_csa_switch_count_status() - extract CSA switch count status
 * from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to CSA switch count status param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_pdev_csa_switch_count_status(void *wmi_hdl,
	void *evt_buf,
	struct pdev_csa_switch_count_status *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_pdev_csa_switch_count_status)
		return wmi_handle->ops->extract_pdev_csa_switch_count_status(
				wmi_handle,
				evt_buf,
				param);

	return QDF_STATUS_E_FAILURE;
}


/**
 * wmi_extract_swba_num_vdevs() - extract swba num vdevs from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param num_vdevs: Pointer to hold num vdevs
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_swba_num_vdevs(void *wmi_hdl, void *evt_buf,
		uint32_t *num_vdevs)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_swba_num_vdevs)
		return wmi_handle->ops->extract_swba_num_vdevs(wmi_handle,
					evt_buf, num_vdevs);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_swba_tim_info() - extract swba tim info from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param idx: Index to bcn info
 * @param tim_info: Pointer to hold tim info
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_swba_tim_info(void *wmi_hdl, void *evt_buf,
	    uint32_t idx, wmi_host_tim_info *tim_info)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_swba_tim_info)
		return wmi_handle->ops->extract_swba_tim_info(wmi_handle,
			evt_buf, idx, tim_info);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_swba_quiet_info() - extract swba quiet info from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param idx: Index to bcn info
 * @param quiet_info: Pointer to hold quiet info
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_swba_quiet_info(void *wmi_hdl, void *evt_buf,
				       uint32_t idx,
				       wmi_host_quiet_info *quiet_info)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->extract_swba_quiet_info)
		return wmi_handle->ops->extract_swba_quiet_info(wmi_handle,
								evt_buf, idx,
								quiet_info);
	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_swba_noa_info() - extract swba NoA information from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param idx: Index to bcn info
 * @param p2p_desc: Pointer to hold p2p NoA info
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_swba_noa_info(void *wmi_hdl, void *evt_buf,
	    uint32_t idx, wmi_host_p2p_noa_info *p2p_desc)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_swba_noa_info)
		return wmi_handle->ops->extract_swba_noa_info(wmi_handle,
			evt_buf, idx, p2p_desc);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_peer_sta_kickout_ev() - extract peer sta kickout event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param ev: Pointer to hold peer param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_peer_sta_kickout_ev(void *wmi_hdl, void *evt_buf,
	wmi_host_peer_sta_kickout_event *ev)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_peer_sta_kickout_ev)
		return wmi_handle->ops->extract_peer_sta_kickout_ev(wmi_handle,
			evt_buf, ev);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_send_multiple_vdev_restart_req_cmd() - send multiple vdev restart
 * @wmi_handle: wmi handle
 * @param: multiple vdev restart parameter
 *
 * Send WMI_PDEV_MULTIPLE_VDEV_RESTART_REQUEST_CMDID parameters to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wmi_unified_send_multiple_vdev_restart_req_cmd(void *wmi_hdl,
				struct multiple_vdev_restart_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_multiple_vdev_restart_req_cmd)
		return wmi_handle->ops->send_multiple_vdev_restart_req_cmd(
					wmi_handle,
					param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_extract_wds_addr_event - Extract WDS addr WMI event
 *
 *  @param wmi_handle      : handle to WMI.
 *  @param evt_buf    : pointer to event buffer
 *  @param len : length of the event buffer
 *  @param wds_ev: pointer to strct to extract
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_wds_addr_event(void *wmi_hdl, void *evt_buf,
	uint16_t len, wds_addr_event_t *wds_ev)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_wds_addr_event) {
		return wmi_handle->ops->extract_wds_addr_event(wmi_handle,
			evt_buf, len, wds_ev);
	}
	return QDF_STATUS_E_FAILURE;
}
qdf_export_symbol(wmi_extract_wds_addr_event);

/**
 * wmi_extract_dcs_interference_type() - extract dcs interference type
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold dcs interference param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_dcs_interference_type(void *wmi_hdl,
	void *evt_buf, struct wmi_host_dcs_interference_param *param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->extract_dcs_interference_type) {
		return wmi->ops->extract_dcs_interference_type(wmi,
			evt_buf, param);
	}
	return QDF_STATUS_E_FAILURE;
}

/*
 * wmi_extract_dcs_cw_int() - extract dcs cw interference from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param cw_int: Pointer to hold cw interference
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_dcs_cw_int(void *wmi_hdl, void *evt_buf,
	wmi_host_ath_dcs_cw_int *cw_int)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_dcs_cw_int) {
		return wmi_handle->ops->extract_dcs_cw_int(wmi_handle,
			evt_buf, cw_int);
	}
	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_dcs_im_tgt_stats() - extract dcs im target stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param wlan_stat: Pointer to hold wlan stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_dcs_im_tgt_stats(void *wmi_hdl, void *evt_buf,
	wmi_host_dcs_im_tgt_stats_t *wlan_stat)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_dcs_im_tgt_stats) {
		return wmi_handle->ops->extract_dcs_im_tgt_stats(wmi_handle,
			evt_buf, wlan_stat);
	}
	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_peer_delete_response_event() -
 *       extract vdev id and peer mac addresse from peer delete response event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold evt buf
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_extract_peer_delete_response_event(
			void *wmi_hdl,
			uint8_t *evt_buf,
			struct wmi_host_peer_delete_response_event *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_peer_delete_response_event)
		return wmi_handle->ops->extract_peer_delete_response_event(
				wmi_handle,
				evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_pdev_tpc_ev_param() - extract tpc param from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold tpc param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_pdev_tpc_ev_param(void *wmi_hdl, void *evt_buf,
	wmi_host_pdev_tpc_event *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_pdev_tpc_ev_param)
		return wmi_handle->ops->extract_pdev_tpc_ev_param(wmi_handle,
				evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_pdev_tpc_config_ev_param() - extract pdev tpc configuration
 * param from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold tpc configuration
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_pdev_tpc_config_ev_param(void *wmi_hdl, void *evt_buf,
	wmi_host_pdev_tpc_config_event *param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->extract_pdev_tpc_config_ev_param)
		return wmi->ops->extract_pdev_tpc_config_ev_param(wmi,
			evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_nfcal_power_ev_param() - extract noise floor calibration
 * power param from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold nf cal power param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_nfcal_power_ev_param(void *wmi_hdl, void *evt_buf,
	wmi_host_pdev_nfcal_power_all_channels_event *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_nfcal_power_ev_param)
		return wmi_handle->ops->extract_nfcal_power_ev_param(wmi_handle,
				evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_peer_sta_ps_statechange_ev() - extract peer sta ps state
 * from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param ev: Pointer to hold peer param and ps state
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_peer_sta_ps_statechange_ev(void *wmi_hdl, void *evt_buf,
	wmi_host_peer_sta_ps_statechange_event *ev)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->extract_peer_sta_ps_statechange_ev)
		return wmi->ops->extract_peer_sta_ps_statechange_ev(wmi,
			evt_buf, ev);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_inst_rssi_stats_event() - extract inst rssi stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param inst_rssi_resp: Pointer to hold inst rssi response
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_inst_rssi_stats_event(void *wmi_hdl, void *evt_buf,
			   wmi_host_inst_stats_resp *inst_rssi_resp)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->extract_inst_rssi_stats_event)
		return wmi->ops->extract_inst_rssi_stats_event(wmi,
			evt_buf, inst_rssi_resp);

	return QDF_STATUS_E_FAILURE;
}


/**
 * wmi_send_pdev_caldata_version_check_cmd() - send reset peer mumimo
 *                                             tx count to fw
 * @wmi_handle: wmi handle
 * @value: value
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_send_pdev_caldata_version_check_cmd(void *wmi_hdl, uint32_t value)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->send_pdev_caldata_version_check_cmd)
		return wmi->ops->send_pdev_caldata_version_check_cmd(wmi,
								value);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_pdev_caldata_version_check_ev_param() - extract caldata
 *                                                     from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold caldata version data
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_pdev_caldata_version_check_ev_param(void *wmi_hdl,
	void *evt_buf,
	wmi_host_pdev_check_cal_version_event *param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->extract_pdev_caldata_version_check_ev_param)
		return wmi->ops->extract_pdev_caldata_version_check_ev_param(
			wmi, evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}

#ifdef WLAN_SUPPORT_FILS
QDF_STATUS
wmi_unified_fils_discovery_send_cmd(void *wmi_hdl, struct fd_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->send_fils_discovery_send_cmd)
		return wmi_handle->ops->send_fils_discovery_send_cmd(wmi_handle,
								     param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_fils_vdev_config_send_cmd(void *wmi_hdl,
				      struct config_fils_params *param)
{
	wmi_unified_t wmi = (wmi_unified_t)wmi_hdl;

	if (wmi->ops->send_vdev_fils_enable_cmd)
		return wmi->ops->send_vdev_fils_enable_cmd(wmi, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_swfda_vdev_id(void *wmi_hdl, void *evt_buf,
			  uint32_t *vdev_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->extract_swfda_vdev_id)
		return wmi_handle->ops->extract_swfda_vdev_id(wmi_handle,
							      evt_buf, vdev_id);

	return QDF_STATUS_E_FAILURE;
}
#endif /* WLAN_SUPPORT_FILS */

/**
 *  wmi_unified_set_qboost_param_cmd_send() - WMI set_qboost function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold set_qboost parameter
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_qboost_param_cmd_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct set_qboost_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_qboost_param_cmd)
		return wmi_handle->ops->send_set_qboost_param_cmd(wmi_handle,
				  macaddr, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_gpio_config_cmd_send() - WMI gpio config function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold gpio config param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_gpio_config_cmd_send(void *wmi_hdl,
				struct gpio_config_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_gpio_config_cmd)
		return wmi_handle->ops->send_gpio_config_cmd(wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_gpio_output_cmd_send() - WMI gpio config function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold gpio config param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_gpio_output_cmd_send(void *wmi_hdl,
				struct gpio_output_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_gpio_output_cmd)
		return wmi_handle->ops->send_gpio_output_cmd(wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_mcast_group_update_cmd_send() - WMI mcast grp update cmd function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold mcast grp param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_mcast_group_update_cmd_send(void *wmi_hdl,
				struct mcast_group_update_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_mcast_group_update_cmd)
		return wmi_handle->ops->send_mcast_group_update_cmd(wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_pdev_qvit_cmd_send() - WMI pdev qvit cmd function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold qvit param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_pdev_qvit_cmd_send(void *wmi_hdl,
				struct pdev_qvit_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_pdev_qvit_cmd)
		return wmi_handle->ops->send_pdev_qvit_cmd(wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_wmm_update_cmd_send() - WMI wmm update cmd function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold wmm param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_wmm_update_cmd_send(void *wmi_hdl,
				struct wmm_update_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_wmm_update_cmd)
		return wmi_handle->ops->send_wmm_update_cmd(wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_vdev_start_resp() - extract vdev start response
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param vdev_rsp: Pointer to hold vdev response
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_vdev_start_resp(void *wmi_hdl, void *evt_buf,
	wmi_host_vdev_start_resp *vdev_rsp)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_vdev_start_resp)
		return wmi_handle->ops->extract_vdev_start_resp(wmi_handle,
				evt_buf, vdev_rsp);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_vdev_delete_resp() - extract vdev delete response
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param delete_rsp: Pointer to hold vdev delete response
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_vdev_delete_resp(void *wmi_hdl, void *evt_buf,
	struct wmi_host_vdev_delete_resp *delete_rsp)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_vdev_delete_resp)
		return wmi_handle->ops->extract_vdev_delete_resp(wmi_handle,
				evt_buf, delete_rsp);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_vdev_stopped_param() - extract vdev stop param from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param vdev_id: Pointer to hold vdev identifier
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_vdev_stopped_param(void *wmi_hdl, void *evt_buf,
	uint32_t *vdev_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_vdev_stopped_param)
		return wmi_handle->ops->extract_vdev_stopped_param(wmi_handle,
				evt_buf, vdev_id);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_mgmt_tx_compl_param() - extract mgmt tx completion param
 * from event
 * @wmi_hdl: wmi handle
 * @evt_buf: pointer to event buffer
 * @param: Pointer to mgmt tx completion param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_mgmt_tx_compl_param(void *wmi_hdl, void *evt_buf,
	wmi_host_mgmt_tx_compl_event *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_mgmt_tx_compl_param)
		return wmi_handle->ops->extract_mgmt_tx_compl_param(wmi_handle,
				evt_buf, param);


	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_chan_info_event() - extract chan information from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param chan_info: Pointer to hold chan information
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_chan_info_event(void *wmi_hdl, void *evt_buf,
			       wmi_host_chan_info_event *chan_info)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_chan_info_event)
		return wmi_handle->ops->extract_chan_info_event(wmi_handle,
			evt_buf, chan_info);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_channel_hopping_event() - extract channel hopping param
 * from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param ch_hopping: Pointer to hold channel hopping param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_channel_hopping_event(void *wmi_hdl, void *evt_buf,
	     wmi_host_pdev_channel_hopping_event *ch_hopping)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->extract_channel_hopping_event)
		return wmi->ops->extract_channel_hopping_event(wmi,
			evt_buf, ch_hopping);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_peer_chan_width_switch_cmd_send() - WMI send peer chan width
 * @wmi_hdl: handle to WMI
 * @param: pointer to hold peer capability param
 *
 * @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_peer_chan_width_switch_cmd_send(void *wmi_hdl,
				struct peer_chan_width_switch_params *param)
{
	wmi_unified_t wmi = (wmi_unified_t)wmi_hdl;

	if (wmi->ops->send_peer_chan_width_switch_cmd)
		return wmi->ops->send_peer_chan_width_switch_cmd(wmi, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_peer_del_all_wds_entries_cmd_send() - WMI del wds
 *  entries cmd function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold wds entry param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_peer_del_all_wds_entries_cmd_send(void *wmi_hdl,
				struct peer_del_all_wds_entries_params *param)
{
	wmi_unified_t wmi = (wmi_unified_t)wmi_hdl;

	if (wmi->ops->send_peer_del_all_wds_entries_cmd)
		return wmi->ops->send_peer_del_all_wds_entries_cmd(wmi,
				param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_vdev_pcp_tid_map_cmd_send() - WMI set vap pcp
 *  tid map cmd function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold pcp param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_vdev_pcp_tid_map_cmd_send(void *wmi_hdl,
				      struct vap_pcp_tid_map_params *param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->send_vdev_pcp_tid_map_cmd)
		return wmi->ops->send_vdev_pcp_tid_map_cmd(wmi, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_vdev_tidmap_prec_cmd_send() - WMI set vap tidmap precedence
 *  cmd function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold precedence param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_vdev_tidmap_prec_cmd_send(void *wmi_hdl,
				       struct vap_tidmap_prec_params *param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->send_vdev_tidmap_prec_cmd)
		return wmi->ops->send_vdev_tidmap_prec_cmd(wmi, param);

	return QDF_STATUS_E_FAILURE;
}
