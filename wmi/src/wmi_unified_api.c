/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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
#include "wmi_unified_priv.h"
#include "wmi_unified_param.h"
#include "qdf_module.h"

/**
 * wmi_unified_vdev_create_send() - send VDEV create command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold vdev create parameter
 * @macaddr: vdev mac address
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_create_send(void *wmi_hdl,
				 uint8_t macaddr[IEEE80211_ADDR_LEN],
				 struct vdev_create_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_vdev_create_cmd)
		return wmi_handle->ops->send_vdev_create_cmd(wmi_handle,
			   macaddr, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_vdev_delete_send() - send VDEV delete command to fw
 * @wmi_handle: wmi handle
 * @if_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_delete_send(void *wmi_hdl,
					  uint8_t if_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_vdev_delete_cmd)
		return wmi_handle->ops->send_vdev_delete_cmd(wmi_handle,
			   if_id);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_vdev_stop_send() - send vdev stop command to fw
 * @wmi: wmi handle
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_stop_send(void *wmi_hdl,
					uint8_t vdev_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_vdev_stop_cmd)
		return wmi_handle->ops->send_vdev_stop_cmd(wmi_handle,
			   vdev_id);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_vdev_down_send() - send vdev down command to fw
 * @wmi: wmi handle
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_down_send(void *wmi_hdl, uint8_t vdev_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_vdev_down_cmd)
		return wmi_handle->ops->send_vdev_down_cmd(wmi_handle, vdev_id);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_vdev_start_send() - send vdev start command to fw
 * @wmi: wmi handle
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_start_send(void *wmi_hdl,
			struct vdev_start_params *req)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_vdev_start_cmd)
		return wmi_handle->ops->send_vdev_start_cmd(wmi_handle, req);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_vdev_set_nac_rssi_send() - send NAC_RSSI command to fw
 * @wmi: wmi handle
 * @req: pointer to hold nac rssi request data
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_set_nac_rssi_send(void *wmi_hdl,
			struct vdev_scan_nac_rssi_params *req)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_vdev_set_nac_rssi_cmd)
		return wmi_handle->ops->send_vdev_set_nac_rssi_cmd(wmi_handle, req);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_hidden_ssid_vdev_restart_send() - restart vdev to set hidden ssid
 * @wmi: wmi handle
 * @restart_params: vdev restart params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_hidden_ssid_vdev_restart_send(void *wmi_hdl,
			struct hidden_ssid_vdev_restart_params *restart_params)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_hidden_ssid_vdev_restart_cmd)
		return wmi_handle->ops->send_hidden_ssid_vdev_restart_cmd(
			wmi_handle, restart_params);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_peer_flush_tids_send() - flush peer tids packets in fw
 * @wmi: wmi handle
 * @peer_addr: peer mac address
 * @param: pointer to hold peer flush tid parameter
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_peer_flush_tids_send(void *wmi_hdl,
					 uint8_t peer_addr[IEEE80211_ADDR_LEN],
					 struct peer_flush_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_peer_flush_tids_cmd)
		return wmi_handle->ops->send_peer_flush_tids_cmd(wmi_handle,
				  peer_addr, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_peer_delete_send() - send PEER delete command to fw
 * @wmi: wmi handle
 * @peer_addr: peer mac addr
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_peer_delete_send(void *wmi_hdl,
				    uint8_t
				    peer_addr[IEEE80211_ADDR_LEN],
				    uint8_t vdev_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_peer_delete_cmd)
		return wmi_handle->ops->send_peer_delete_cmd(wmi_handle,
				  peer_addr, vdev_id);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_set_peer_param() - set peer parameter in fw
 * @wmi_ctx: wmi handle
 * @peer_addr: peer mac address
 * @param    : pointer to hold peer set parameter
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_set_peer_param_send(void *wmi_hdl,
				uint8_t peer_addr[IEEE80211_ADDR_LEN],
				struct peer_set_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_peer_param_cmd)
		return wmi_handle->ops->send_peer_param_cmd(wmi_handle,
				peer_addr, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_vdev_up_send() - send vdev up command in fw
 * @wmi: wmi handle
 * @bssid: bssid
 * @vdev_up_params: pointer to hold vdev up parameter
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_up_send(void *wmi_hdl,
			     uint8_t bssid[IEEE80211_ADDR_LEN],
				 struct vdev_up_params *params)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_vdev_up_cmd)
		return wmi_handle->ops->send_vdev_up_cmd(wmi_handle, bssid,
					params);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_peer_create_send() - send peer create command to fw
 * @wmi: wmi handle
 * @peer_addr: peer mac address
 * @peer_type: peer type
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_peer_create_send(void *wmi_hdl,
					struct peer_create_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_peer_create_cmd)
		return wmi_handle->ops->send_peer_create_cmd(wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_peer_rx_reorder_queue_setup_send() - send rx reorder queue
 * 	setup command to fw
 * @wmi: wmi handle
 * @rx_reorder_queue_setup_params: Rx reorder queue setup parameters
 *
 * Return: QDF_STATUS for success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_peer_rx_reorder_queue_setup_send(void *wmi_hdl,
					struct rx_reorder_queue_setup_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_peer_rx_reorder_queue_setup_cmd)
		return wmi_handle->ops->send_peer_rx_reorder_queue_setup_cmd(
			wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_peer_rx_reorder_queue_remove_send() - send rx reorder queue
 * 	remove command to fw
 * @wmi: wmi handle
 * @rx_reorder_queue_remove_params: Rx reorder queue remove parameters
 *
 * Return: QDF_STATUS for success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_peer_rx_reorder_queue_remove_send(void *wmi_hdl,
					struct rx_reorder_queue_remove_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_peer_rx_reorder_queue_remove_cmd)
		return wmi_handle->ops->send_peer_rx_reorder_queue_remove_cmd(
			wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

#ifdef WLAN_SUPPORT_GREEN_AP
/**
 * wmi_unified_green_ap_ps_send() - enable green ap powersave command
 * @wmi_handle: wmi handle
 * @value: value
 * @pdev_id: pdev id to have radio context
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_green_ap_ps_send(void *wmi_hdl,
						uint32_t value, uint8_t pdev_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_green_ap_ps_cmd)
		return wmi_handle->ops->send_green_ap_ps_cmd(wmi_handle, value,
				  pdev_id);

	return QDF_STATUS_E_FAILURE;
}
#else
QDF_STATUS wmi_unified_green_ap_ps_send(void *wmi_hdl,
						uint32_t value, uint8_t pdev_id)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_SUPPORT_GREEN_AP */

/**
 * wmi_unified_pdev_utf_cmd() - send utf command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to pdev_utf_params
 * @mac_id: mac id to have radio context
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_pdev_utf_cmd_send(void *wmi_hdl,
				struct pdev_utf_params *param,
				uint8_t mac_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_pdev_utf_cmd)
		return wmi_handle->ops->send_pdev_utf_cmd(wmi_handle, param,
				  mac_id);

	return QDF_STATUS_E_FAILURE;
}

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
wmi_unified_pdev_param_send(void *wmi_hdl,
			   struct pdev_params *param,
				uint8_t mac_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_pdev_param_cmd)
		return wmi_handle->ops->send_pdev_param_cmd(wmi_handle, param,
				  mac_id);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_suspend_send() - WMI suspend function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold suspend parameter
 *  @mac_id: radio context
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_suspend_send(void *wmi_hdl,
				struct suspend_params *param,
				uint8_t mac_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_suspend_cmd)
		return wmi_handle->ops->send_suspend_cmd(wmi_handle, param,
				  mac_id);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_resume_send - WMI resume function
 *  @param wmi_handle      : handle to WMI.
 *  @mac_id: radio context
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_resume_send(void *wmi_hdl,
				uint8_t mac_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_resume_cmd)
		return wmi_handle->ops->send_resume_cmd(wmi_handle,
				  mac_id);

	return QDF_STATUS_E_FAILURE;
}

#ifdef FEATURE_WLAN_D0WOW
/**
 *  wmi_unified_d0wow_enable_send() - WMI d0 wow enable function
 *  @param wmi_handle: handle to WMI.
 *  @mac_id: radio context
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_d0wow_enable_send(void *wmi_hdl,
				uint8_t mac_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_d0wow_enable_cmd)
		return wmi_handle->ops->send_d0wow_enable_cmd(
					wmi_handle, mac_id);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_d0wow_disable_send() - WMI d0 wow disable function
 *  @param wmi_handle: handle to WMI.
 *  @mac_id: radio context
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_d0wow_disable_send(void *wmi_hdl,
				uint8_t mac_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_d0wow_disable_cmd)
		return wmi_handle->ops->send_d0wow_disable_cmd(
					wmi_handle, mac_id);

	return QDF_STATUS_E_FAILURE;
}
#endif

/**
 *  wmi_unified_wow_enable_send() - WMI wow enable function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold wow enable parameter
 *  @mac_id: radio context
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_wow_enable_send(void *wmi_hdl,
				struct wow_cmd_params *param,
				uint8_t mac_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_wow_enable_cmd)
		return wmi_handle->ops->send_wow_enable_cmd(wmi_handle, param,
				  mac_id);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_wow_wakeup_send() - WMI wow wakeup function
 *  @param wmi_hdl      : handle to WMI.
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_wow_wakeup_send(void *wmi_hdl)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_wow_wakeup_cmd)
		return wmi_handle->ops->send_wow_wakeup_cmd(wmi_handle);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_wow_add_wakeup_event_send() - WMI wow wakeup function
 *  @param wmi_handle      : handle to WMI.
 *  @param: pointer to wow wakeup event parameter structure
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_wow_add_wakeup_event_send(void *wmi_hdl,
		struct wow_add_wakeup_params *param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->send_wow_add_wakeup_event_cmd)
		return wmi->ops->send_wow_add_wakeup_event_cmd(wmi,
				param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_wow_add_wakeup_pattern_send() - WMI wow wakeup pattern function
 *  @param wmi_handle      : handle to WMI.
 *  @param: pointer to wow wakeup pattern parameter structure
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_wow_add_wakeup_pattern_send(void *wmi_hdl,
		struct wow_add_wakeup_pattern_params *param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->send_wow_add_wakeup_pattern_cmd)
		return wmi->ops->send_wow_add_wakeup_pattern_cmd(wmi, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_wow_remove_wakeup_pattern_send() - WMI wow wakeup pattern function
 *  @param wmi_handle      : handle to WMI.
 *  @param: pointer to wow wakeup pattern parameter structure
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_wow_remove_wakeup_pattern_send(void *wmi_hdl,
		struct wow_remove_wakeup_pattern_params *param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->send_wow_remove_wakeup_pattern_cmd)
		return wmi->ops->send_wow_remove_wakeup_pattern_cmd(wmi, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_ap_ps_cmd_send() - set ap powersave parameters
 * @wma_ctx: wma context
 * @peer_addr: peer mac address
 * @param: pointer to ap_ps parameter structure
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_ap_ps_cmd_send(void *wmi_hdl,
					   uint8_t *peer_addr,
					   struct ap_ps_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_ap_ps_param_cmd)
		return wmi_handle->ops->send_set_ap_ps_param_cmd(wmi_handle,
				  peer_addr,
				  param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_sta_ps_cmd_send() - set sta powersave parameters
 * @wma_ctx: wma context
 * @peer_addr: peer mac address
 * @param: pointer to sta_ps parameter structure
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_sta_ps_cmd_send(void *wmi_hdl,
					   struct sta_ps_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_sta_ps_param_cmd)
		return wmi_handle->ops->send_set_sta_ps_param_cmd(wmi_handle,
				  param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_crash_inject() - inject fw crash
 * @wma_handle: wma handle
 * @param: ponirt to crash inject paramter structure
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_crash_inject(void *wmi_hdl,
			 struct crash_inject *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_crash_inject_cmd)
		return wmi_handle->ops->send_crash_inject_cmd(wmi_handle,
				  param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_dbglog_cmd_send() - set debug log level
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold dbglog level parameter
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_dbglog_cmd_send(void *wmi_hdl,
				struct dbglog_params *dbglog_param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_dbglog_cmd)
		return wmi_handle->ops->send_dbglog_cmd(wmi_handle,
				  dbglog_param);

	return QDF_STATUS_E_FAILURE;
}
qdf_export_symbol(wmi_unified_dbglog_cmd_send);

/**
 *  wmi_unified_vdev_set_param_send() - WMI vdev set parameter function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold vdev set parameter
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_set_param_send(void *wmi_hdl,
				struct vdev_set_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_vdev_set_param_cmd)
		return wmi_handle->ops->send_vdev_set_param_cmd(wmi_handle,
				  param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_stats_request_send() - WMI request stats function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold stats request parameter
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_stats_request_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct stats_request_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_stats_request_cmd)
		return wmi_handle->ops->send_stats_request_cmd(wmi_handle,
				   macaddr, param);

	return QDF_STATUS_E_FAILURE;
}

#ifdef CONFIG_MCL
/**
 *  wmi_unified_packet_log_enable_send() - WMI request stats function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold stats request parameter
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_packet_log_enable_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct packet_enable_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_packet_log_enable_cmd)
		return wmi_handle->ops->send_packet_log_enable_cmd(wmi_handle,
				  macaddr, param);

	return QDF_STATUS_E_FAILURE;
}
#else
/**
 *  wmi_unified_packet_log_enable_send() - WMI request stats function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold stats request parameter
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_packet_log_enable_send(void *wmi_hdl,
			WMI_HOST_PKTLOG_EVENT PKTLOG_EVENT, uint8_t mac_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_packet_log_enable_cmd)
		return wmi_handle->ops->send_packet_log_enable_cmd(wmi_handle,
				  PKTLOG_EVENT, mac_id);

	return QDF_STATUS_E_FAILURE;
}

#endif
/**
 *  wmi_unified_packet_log_disable__send() - WMI pktlog disable function
 *  @param wmi_handle      : handle to WMI.
 *  @param PKTLOG_EVENT    : packet log event
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_packet_log_disable_send(void *wmi_hdl, uint8_t mac_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_packet_log_disable_cmd)
		return wmi_handle->ops->send_packet_log_disable_cmd(wmi_handle,
			mac_id);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_beacon_send_cmd() - WMI beacon send function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
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
 *  wmi_unified_beacon_tmpl_send_cmd() - WMI beacon send function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold beacon send cmd parameter
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_beacon_tmpl_send_cmd(void *wmi_hdl,
				struct beacon_tmpl_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_beacon_tmpl_send_cmd)
		return wmi_handle->ops->send_beacon_tmpl_send_cmd(wmi_handle,
				  param);

	return QDF_STATUS_E_FAILURE;
}
/**
 *  wmi_unified_peer_assoc_send() - WMI peer assoc function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to peer assoc parameter
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_peer_assoc_send(void *wmi_hdl,
				struct peer_assoc_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_peer_assoc_cmd)
		return wmi_handle->ops->send_peer_assoc_cmd(wmi_handle,
				  param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_scan_start_cmd_send() - WMI scan start function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold scan start cmd parameter
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_scan_start_cmd_send(void *wmi_hdl,
				struct scan_req_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_scan_start_cmd)
		return wmi_handle->ops->send_scan_start_cmd(wmi_handle,
				  param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_scan_stop_cmd_send() - WMI scan start function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold scan start cmd parameter
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_scan_stop_cmd_send(void *wmi_hdl,
				struct scan_cancel_param *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_scan_stop_cmd)
		return wmi_handle->ops->send_scan_stop_cmd(wmi_handle,
				  param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_scan_chan_list_cmd_send() - WMI scan channel list function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold scan channel list parameter
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_scan_chan_list_cmd_send(void *wmi_hdl,
				struct scan_chan_list_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_scan_chan_list_cmd)
		return wmi_handle->ops->send_scan_chan_list_cmd(wmi_handle,
				  param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_mgmt_unified_cmd_send() - management cmd over wmi layer
 *  @wmi_hdl      : handle to WMI.
 *  @param    : pointer to hold mgmt cmd parameter
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_mgmt_unified_cmd_send(void *wmi_hdl,
				struct wmi_mgmt_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_mgmt_cmd)
		return wmi_handle->ops->send_mgmt_cmd(wmi_handle,
				  param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_offchan_data_tx_cmd_send() - Send offchan data tx cmd over wmi layer
 *  @wmi_hdl      : handle to WMI.
 *  @param    : pointer to hold offchan data cmd parameter
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_offchan_data_tx_cmd_send(void *wmi_hdl,
				struct wmi_offchan_data_tx_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_offchan_data_tx_cmd)
		return wmi_handle->ops->send_offchan_data_tx_cmd(wmi_handle,
				  param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_modem_power_state() - set modem power state to fw
 * @wmi_hdl: wmi handle
 * @param_value: parameter value
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_modem_power_state(void *wmi_hdl,
		uint32_t param_value)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_modem_power_state_cmd)
		return wmi_handle->ops->send_modem_power_state_cmd(wmi_handle,
				  param_value);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_set_sta_ps_mode() - set sta powersave params in fw
 * @wmi_hdl: wmi handle
 * @vdev_id: vdev id
 * @val: value
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure.
 */
QDF_STATUS wmi_unified_set_sta_ps_mode(void *wmi_hdl,
			       uint32_t vdev_id, uint8_t val)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_sta_ps_mode_cmd)
		return wmi_handle->ops->send_set_sta_ps_mode_cmd(wmi_handle,
				  vdev_id, val);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_set_mimops() - set MIMO powersave
 * @wmi_hdl: wmi handle
 * @vdev_id: vdev id
 * @value: value
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure.
 */
QDF_STATUS wmi_unified_set_mimops(void *wmi_hdl, uint8_t vdev_id, int value)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_mimops_cmd)
		return wmi_handle->ops->send_set_mimops_cmd(wmi_handle,
				  vdev_id, value);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_set_smps_params() - set smps params
 * @wmi_hdl: wmi handle
 * @vdev_id: vdev id
 * @value: value
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure.
 */
QDF_STATUS wmi_unified_set_smps_params(void *wmi_hdl, uint8_t vdev_id,
			       int value)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_smps_params_cmd)
		return wmi_handle->ops->send_set_smps_params_cmd(wmi_handle,
				  vdev_id, value);

	return QDF_STATUS_E_FAILURE;
}


/**
 * wmi_set_p2pgo_oppps_req() - send p2p go opp power save request to fw
 * @wmi_hdl: wmi handle
 * @opps: p2p opp power save parameters
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_p2pgo_oppps_req(void *wmi_hdl,
		struct p2p_ps_params *oppps)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_p2pgo_oppps_req_cmd)
		return wmi_handle->ops->send_set_p2pgo_oppps_req_cmd(wmi_handle,
				  oppps);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_set_p2pgo_noa_req_cmd() - send p2p go noa request to fw
 * @wmi_hdl: wmi handle
 * @noa: p2p power save parameters
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_p2pgo_noa_req_cmd(void *wmi_hdl,
			struct p2p_ps_params *noa)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_p2pgo_noa_req_cmd)
		return wmi_handle->ops->send_set_p2pgo_noa_req_cmd(wmi_handle,
				  noa);

	return QDF_STATUS_E_FAILURE;
}

#ifdef CONVERGED_P2P_ENABLE
/**
 * wmi_unified_p2p_lo_start_cmd() - send p2p lo start request to fw
 * @wmi_hdl: wmi handle
 * @param: p2p listen offload start parameters
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_p2p_lo_start_cmd(void *wmi_hdl,
	struct p2p_lo_start *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (!wmi_handle) {
		WMI_LOGE("wmi handle is null");
		return QDF_STATUS_E_INVAL;
	}

	if (wmi_handle->ops->send_p2p_lo_start_cmd)
		return wmi_handle->ops->send_p2p_lo_start_cmd(wmi_handle,
				  param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_p2p_lo_stop_cmd() - send p2p lo stop request to fw
 * @wmi_hdl: wmi handle
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_p2p_lo_stop_cmd(void *wmi_hdl, uint8_t vdev_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (!wmi_handle) {
		WMI_LOGE("wmi handle is null");
		return QDF_STATUS_E_INVAL;
	}

	if (wmi_handle->ops->send_p2p_lo_start_cmd)
		return wmi_handle->ops->send_p2p_lo_stop_cmd(wmi_handle,
				  vdev_id);

	return QDF_STATUS_E_FAILURE;
}
#endif /* End of CONVERGED_P2P_ENABLE */

/**
 * wmi_get_temperature() - get pdev temperature req
 * @wmi_hdl: wmi handle
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure.
 */
QDF_STATUS wmi_unified_get_temperature(void *wmi_hdl)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_get_temperature_cmd)
		return wmi_handle->ops->send_get_temperature_cmd(wmi_handle);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_set_sta_uapsd_auto_trig_cmd() - set uapsd auto trigger command
 * @wmi_hdl: wmi handle
 * @end_set_sta_ps_mode_cmd: cmd paramter strcture
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
wmi_unified_set_sta_uapsd_auto_trig_cmd(void *wmi_hdl,
				struct sta_uapsd_trig_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_sta_uapsd_auto_trig_cmd)
		return wmi_handle->ops->send_set_sta_uapsd_auto_trig_cmd(wmi_handle,
					param);

	return QDF_STATUS_E_FAILURE;
}

#ifdef WLAN_FEATURE_DSRC
QDF_STATUS wmi_unified_ocb_start_timing_advert(struct wmi_unified *wmi_hdl,
	struct ocb_timing_advert_param *timing_advert)
{
	if (wmi_hdl->ops->send_ocb_start_timing_advert_cmd)
		return wmi_hdl->ops->send_ocb_start_timing_advert_cmd(wmi_hdl,
							timing_advert);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_ocb_stop_timing_advert(struct wmi_unified *wmi_hdl,
	struct ocb_timing_advert_param *timing_advert)
{
	if (wmi_hdl->ops->send_ocb_stop_timing_advert_cmd)
		return wmi_hdl->ops->send_ocb_stop_timing_advert_cmd(wmi_hdl,
							timing_advert);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_ocb_set_utc_time_cmd(struct wmi_unified *wmi_hdl,
					    struct ocb_utc_param *utc)
{
	if (wmi_hdl->ops->send_ocb_set_utc_time_cmd)
		return wmi_hdl->ops->send_ocb_set_utc_time_cmd(wmi_hdl, utc);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_ocb_get_tsf_timer(struct wmi_unified *wmi_hdl,
					 struct ocb_get_tsf_timer_param *req)
{
	if (wmi_hdl->ops->send_ocb_get_tsf_timer_cmd)
		return wmi_hdl->ops->send_ocb_get_tsf_timer_cmd(wmi_hdl,
								req->vdev_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_dcc_get_stats_cmd(struct wmi_unified *wmi_hdl,
			struct ocb_dcc_get_stats_param *get_stats_param)
{
	if (wmi_hdl->ops->send_dcc_get_stats_cmd)
		return wmi_hdl->ops->send_dcc_get_stats_cmd(wmi_hdl,
							    get_stats_param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_dcc_clear_stats(struct wmi_unified *wmi_hdl,
		struct ocb_dcc_clear_stats_param *clear_stats_param)
{
	if (wmi_hdl->ops->send_dcc_clear_stats_cmd)
		return wmi_hdl->ops->send_dcc_clear_stats_cmd(wmi_hdl,
				clear_stats_param->vdev_id,
				clear_stats_param->dcc_stats_bitmap);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_dcc_update_ndl(struct wmi_unified *wmi_hdl,
			struct ocb_dcc_update_ndl_param *update_ndl_param)
{
	if (wmi_hdl->ops->send_dcc_update_ndl_cmd)
		return wmi_hdl->ops->send_dcc_update_ndl_cmd(wmi_hdl,
					update_ndl_param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_ocb_set_config(struct wmi_unified *wmi_hdl,
				      struct ocb_config *config)
{
	if (wmi_hdl->ops->send_ocb_set_config_cmd)
		return wmi_hdl->ops->send_ocb_set_config_cmd(wmi_hdl,
							     config);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_ocb_set_channel_config_resp(struct wmi_unified *wmi_hdl,
					void *evt_buf,
					uint32_t *status)
{
	if (wmi_hdl->ops->extract_ocb_chan_config_resp)
		return wmi_hdl->ops->extract_ocb_chan_config_resp(wmi_hdl,
								  evt_buf,
								  status);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_ocb_tsf_timer(struct wmi_unified *wmi_hdl,
				     void *evt_buf,
				     struct ocb_get_tsf_timer_response *resp)
{
	if (wmi_hdl->ops->extract_ocb_tsf_timer)
		return wmi_hdl->ops->extract_ocb_tsf_timer(wmi_hdl,
							   evt_buf,
							   resp);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_dcc_update_ndl_resp(struct wmi_unified *wmi_hdl,
		void *evt_buf, struct ocb_dcc_update_ndl_response *resp)
{
	if (wmi_hdl->ops->extract_dcc_update_ndl_resp)
		return wmi_hdl->ops->extract_dcc_update_ndl_resp(wmi_hdl,
								 evt_buf,
								 resp);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_dcc_stats(struct wmi_unified *wmi_hdl,
				 void *evt_buf,
				 struct ocb_dcc_get_stats_response **resp)
{
	if (wmi_hdl->ops->extract_dcc_stats)
		return wmi_hdl->ops->extract_dcc_stats(wmi_hdl,
						       evt_buf,
						       resp);

	return QDF_STATUS_E_FAILURE;
}
#endif

/**
 * wmi_unified_set_enable_disable_mcc_adaptive_scheduler_cmd() - control mcc scheduler
 * @wmi_handle: wmi handle
 * @mcc_adaptive_scheduler: enable/disable
 *
 * This function enable/disable mcc adaptive scheduler in fw.
 *
 * Return: QDF_STATUS_SUCCESS for sucess or error code
 */
QDF_STATUS wmi_unified_set_enable_disable_mcc_adaptive_scheduler_cmd(
		void *wmi_hdl, uint32_t mcc_adaptive_scheduler,
		uint32_t pdev_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_enable_disable_mcc_adaptive_scheduler_cmd)
		return wmi_handle->ops->send_set_enable_disable_mcc_adaptive_scheduler_cmd(wmi_handle,
					mcc_adaptive_scheduler, pdev_id);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_set_mcc_channel_time_latency_cmd() - set MCC channel time latency
 * @wmi: wmi handle
 * @mcc_channel: mcc channel
 * @mcc_channel_time_latency: MCC channel time latency.
 *
 * Currently used to set time latency for an MCC vdev/adapter using operating
 * channel of it and channel number. The info is provided run time using
 * iwpriv command: iwpriv <wlan0 | p2p0> setMccLatency <latency in ms>.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_mcc_channel_time_latency_cmd(void *wmi_hdl,
	uint32_t mcc_channel_freq, uint32_t mcc_channel_time_latency)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_mcc_channel_time_latency_cmd)
		return wmi_handle->ops->send_set_mcc_channel_time_latency_cmd(wmi_handle,
					mcc_channel_freq,
					mcc_channel_time_latency);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_set_mcc_channel_time_quota_cmd() - set MCC channel time quota
 * @wmi: wmi handle
 * @adapter_1_chan_number: adapter 1 channel number
 * @adapter_1_quota: adapter 1 quota
 * @adapter_2_chan_number: adapter 2 channel number
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_mcc_channel_time_quota_cmd(void *wmi_hdl,
			 uint32_t adapter_1_chan_freq,
			 uint32_t adapter_1_quota, uint32_t adapter_2_chan_freq)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_mcc_channel_time_quota_cmd)
		return wmi_handle->ops->send_set_mcc_channel_time_quota_cmd(wmi_handle,
						adapter_1_chan_freq,
						adapter_1_quota,
						adapter_2_chan_freq);

	return QDF_STATUS_E_FAILURE;
}

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
QDF_STATUS wmi_unified_set_thermal_mgmt_cmd(void *wmi_hdl,
				struct thermal_cmd_params *thermal_info)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_thermal_mgmt_cmd)
		return wmi_handle->ops->send_set_thermal_mgmt_cmd(wmi_handle,
					thermal_info);

	return QDF_STATUS_E_FAILURE;
}


/**
 * wmi_unified_lro_config_cmd() - process the LRO config command
 * @wmi: Pointer to wmi handle
 * @wmi_lro_cmd: Pointer to LRO configuration parameters
 *
 * This function sends down the LRO configuration parameters to
 * the firmware to enable LRO, sets the TCP flags and sets the
 * seed values for the toeplitz hash generation
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_lro_config_cmd(void *wmi_hdl,
	 struct wmi_lro_config_cmd_t *wmi_lro_cmd)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_lro_config_cmd)
		return wmi_handle->ops->send_lro_config_cmd(wmi_handle,
					wmi_lro_cmd);

	return QDF_STATUS_E_FAILURE;
}

#ifdef CONFIG_MCL
/**
 * wmi_unified_peer_rate_report_cmd() - process the peer rate report command
 * @wmi_hdl: Pointer to wmi handle
 * @rate_report_params: Pointer to peer rate report parameters
 *
 *
 * Return: QDF_STATUS_SUCCESS for success otherwise failure
 */
QDF_STATUS wmi_unified_peer_rate_report_cmd(void *wmi_hdl,
		struct wmi_peer_rate_report_params *rate_report_params)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_peer_rate_report_cmd)
		return wmi_handle->ops->send_peer_rate_report_cmd(wmi_handle,
					rate_report_params);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_bcn_buf_ll_cmd() - prepare and send beacon buffer to fw for LL
 * @wmi_hdl: wmi handle
 * @param: bcn ll cmd parameter
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_bcn_buf_ll_cmd(void *wmi_hdl,
			wmi_bcn_send_from_host_cmd_fixed_param *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_bcn_buf_ll_cmd)
		return wmi_handle->ops->send_bcn_buf_ll_cmd(wmi_handle,
						param);

	return QDF_STATUS_E_FAILURE;
}
#endif

/**
 * wmi_unified_set_sta_sa_query_param_cmd() - set sta sa query parameters
 * @wmi_hdl: wmi handle
 * @vdev_id: vdev id
 * @max_retries: max retries
 * @retry_interval: retry interval
 * This function sets sta query related parameters in fw.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */

QDF_STATUS wmi_unified_set_sta_sa_query_param_cmd(void *wmi_hdl,
					uint8_t vdev_id, uint32_t max_retries,
					uint32_t retry_interval)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_sta_sa_query_param_cmd)
		return wmi_handle->ops->send_set_sta_sa_query_param_cmd(wmi_handle,
						vdev_id, max_retries,
						retry_interval);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_set_sta_keep_alive_cmd() - set sta keep alive parameters
 * @wmi_hdl: wmi handle
 * @params: sta keep alive parameter
 *
 * This function sets keep alive related parameters in fw.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_sta_keep_alive_cmd(void *wmi_hdl,
				struct sta_params *params)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_sta_keep_alive_cmd)
		return wmi_handle->ops->send_set_sta_keep_alive_cmd(wmi_handle,
						params);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_vdev_set_gtx_cfg_cmd() - set GTX params
 * @wmi_hdl: wmi handle
 * @if_id: vdev id
 * @gtx_info: GTX config params
 *
 * This function set GTX related params in firmware.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_set_gtx_cfg_cmd(void *wmi_hdl, uint32_t if_id,
			struct wmi_gtx_config *gtx_info)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_vdev_set_gtx_cfg_cmd)
		return wmi_handle->ops->send_vdev_set_gtx_cfg_cmd(wmi_handle,
					if_id, gtx_info);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_process_update_edca_param() - update EDCA params
 * @wmi_hdl: wmi handle
 * @vdev_id: vdev id.
 * @wmm_vparams: edca parameters
 *
 * This function updates EDCA parameters to the target
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_process_update_edca_param(void *wmi_hdl,
				uint8_t vdev_id,
				struct wmi_host_wme_vparams wmm_vparams[WMI_MAX_NUM_AC])
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_process_update_edca_param_cmd)
		return wmi_handle->ops->send_process_update_edca_param_cmd(wmi_handle,
					 vdev_id, wmm_vparams);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_probe_rsp_tmpl_send_cmd() - send probe response template to fw
 * @wmi_hdl: wmi handle
 * @vdev_id: vdev id
 * @probe_rsp_info: probe response info
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_probe_rsp_tmpl_send_cmd(void *wmi_hdl,
				uint8_t vdev_id,
				struct wmi_probe_resp_params *probe_rsp_info)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_probe_rsp_tmpl_send_cmd)
		return wmi_handle->ops->send_probe_rsp_tmpl_send_cmd(wmi_handle,
						 vdev_id, probe_rsp_info);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_setup_install_key_cmd - send key to install to fw
 * @wmi_hdl: wmi handle
 * @key_params: key parameters
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_setup_install_key_cmd(void *wmi_hdl,
				struct set_key_params *key_params)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_setup_install_key_cmd)
		return wmi_handle->ops->send_setup_install_key_cmd(wmi_handle,
							key_params);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_p2p_go_set_beacon_ie_cmd() - set beacon IE for p2p go
 * @wma_handle: wma handle
 * @vdev_id: vdev id
 * @p2p_ie: p2p IE
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_p2p_go_set_beacon_ie_cmd(void *wmi_hdl,
				    A_UINT32 vdev_id, uint8_t *p2p_ie)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_p2p_go_set_beacon_ie_cmd)
		return wmi_handle->ops->send_p2p_go_set_beacon_ie_cmd(wmi_handle,
						 vdev_id, p2p_ie);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_set_gateway_params_cmd() - set gateway parameters
 * @wmi_hdl: wmi handle
 * @req: gateway parameter update request structure
 *
 * This function reads the incoming @req and fill in the destination
 * WMI structure and sends down the gateway configs down to the firmware
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failures;
 *         error number otherwise
 */
QDF_STATUS wmi_unified_set_gateway_params_cmd(void *wmi_hdl,
					struct gateway_update_req_param *req)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_gateway_params_cmd)
		return wmi_handle->ops->send_set_gateway_params_cmd(wmi_handle,
				  req);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_set_rssi_monitoring_cmd() - set rssi monitoring
 * @wmi_hdl: wmi handle
 * @req: rssi monitoring request structure
 *
 * This function reads the incoming @req and fill in the destination
 * WMI structure and send down the rssi monitoring configs down to the firmware
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failures;
 *         error number otherwise
 */
QDF_STATUS wmi_unified_set_rssi_monitoring_cmd(void *wmi_hdl,
					struct rssi_monitor_param *req)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_rssi_monitoring_cmd)
		return wmi_handle->ops->send_set_rssi_monitoring_cmd(wmi_handle,
			    req);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_scan_probe_setoui_cmd() - set scan probe OUI
 * @wmi_hdl: wmi handle
 * @psetoui: OUI parameters
 *
 * set scan probe OUI parameters in firmware
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_scan_probe_setoui_cmd(void *wmi_hdl,
			  struct scan_mac_oui *psetoui)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_scan_probe_setoui_cmd)
		return wmi_handle->ops->send_scan_probe_setoui_cmd(wmi_handle,
			    psetoui);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_reset_passpoint_network_list_cmd() - reset passpoint network list
 * @wmi_hdl: wmi handle
 * @req: passpoint network request structure
 *
 * This function sends down WMI command with network id set to wildcard id.
 * firmware shall clear all the config entries
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_reset_passpoint_network_list_cmd(void *wmi_hdl,
					struct wifi_passpoint_req_param *req)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_reset_passpoint_network_list_cmd)
		return wmi_handle->ops->send_reset_passpoint_network_list_cmd(wmi_handle,
			    req);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_set_passpoint_network_list_cmd() - set passpoint network list
 * @wmi_hdl: wmi handle
 * @req: passpoint network request structure
 *
 * This function reads the incoming @req and fill in the destination
 * WMI structure and send down the passpoint configs down to the firmware
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_passpoint_network_list_cmd(void *wmi_hdl,
					struct wifi_passpoint_req_param *req)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_passpoint_network_list_cmd)
		return wmi_handle->ops->send_set_passpoint_network_list_cmd(wmi_handle,
			    req);

	return QDF_STATUS_E_FAILURE;
}

/** wmi_unified_set_epno_network_list_cmd() - set epno network list
 * @wmi_hdl: wmi handle
 * @req: epno config params request structure
 *
 * This function reads the incoming epno config request structure
 * and constructs the WMI message to the firmware.
 *
 * Returns: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failures,
 *          error number otherwise
 */
QDF_STATUS wmi_unified_set_epno_network_list_cmd(void *wmi_hdl,
		struct wifi_enhanched_pno_params *req)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_epno_network_list_cmd)
		return wmi_handle->ops->send_set_epno_network_list_cmd(wmi_handle,
			    req);

	return QDF_STATUS_E_FAILURE;
}

#ifdef CONFIG_MCL
/**
 * wmi_unified_roam_scan_offload_mode_cmd() - set roam scan parameters
 * @wmi_hdl: wmi handle
 * @scan_cmd_fp: scan related parameters
 * @roam_req: roam related parameters
 *
 * This function reads the incoming @roam_req and fill in the destination
 * WMI structure and send down the roam scan configs down to the firmware
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_roam_scan_offload_mode_cmd(void *wmi_hdl,
				wmi_start_scan_cmd_fixed_param *scan_cmd_fp,
				struct roam_offload_scan_params *roam_req)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_roam_scan_offload_mode_cmd)
		return wmi_handle->ops->send_roam_scan_offload_mode_cmd(
			wmi_handle, scan_cmd_fp, roam_req);

	return QDF_STATUS_E_FAILURE;
}
#endif

/**
 * wmi_unified_roam_scan_offload_rssi_thresh_cmd() - set roam scan rssi
 *							parameters
 * @wmi_hdl: wmi handle
 * @roam_req: roam rssi related parameters
 *
 * This function reads the incoming @roam_req and fill in the destination
 * WMI structure and send down the roam scan rssi configs down to the firmware
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_roam_scan_offload_rssi_thresh_cmd(void *wmi_hdl,
					struct roam_offload_scan_rssi_params
					*roam_req)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_roam_scan_offload_rssi_thresh_cmd)
		return wmi_handle->ops->send_roam_scan_offload_rssi_thresh_cmd(
				wmi_handle, roam_req);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_roam_mawc_params_cmd(
			void *wmi_hdl, struct wmi_mawc_roam_params *params)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_roam_mawc_params_cmd)
		return wmi_handle->ops->send_roam_mawc_params_cmd(
				wmi_handle, params);

	return QDF_STATUS_E_FAILURE;
}
/**
 * wmi_unified_roam_scan_filter_cmd() - send roam scan whitelist,
 *                                      blacklist and preferred list
 * @wmi_hdl: wmi handle
 * @roam_req: roam scan lists related parameters
 *
 * This function reads the incoming @roam_req and fill in the destination
 * WMI structure and send down the different roam scan lists down to the fw
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_roam_scan_filter_cmd(void *wmi_hdl,
				struct roam_scan_filter_params *roam_req)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_roam_scan_filter_cmd)
		return wmi_handle->ops->send_roam_scan_filter_cmd(
				wmi_handle, roam_req);

	return QDF_STATUS_E_FAILURE;
}

/** wmi_unified_ipa_offload_control_cmd() - ipa offload control parameter
 * @wmi_hdl: wmi handle
 * @ipa_offload: ipa offload control parameter
 *
 * Returns: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failures,
 *          error number otherwise
 */
QDF_STATUS  wmi_unified_ipa_offload_control_cmd(void *wmi_hdl,
		struct ipa_offload_control_params *ipa_offload)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_ipa_offload_control_cmd)
		return wmi_handle->ops->send_ipa_offload_control_cmd(wmi_handle,
			    ipa_offload);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_extscan_get_capabilities_cmd() - extscan get capabilities
 * @wmi_hdl: wmi handle
 * @pgetcapab: get capabilities params
 *
 * This function send request to fw to get extscan capabilities.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_extscan_get_capabilities_cmd(void *wmi_hdl,
			  struct extscan_capabilities_params *pgetcapab)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_extscan_get_capabilities_cmd)
		return wmi_handle->ops->send_extscan_get_capabilities_cmd(wmi_handle,
			    pgetcapab);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_extscan_get_cached_results_cmd() - extscan get cached results
 * @wmi_hdl: wmi handle
 * @pcached_results: cached results parameters
 *
 * This function send request to fw to get cached results.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_extscan_get_cached_results_cmd(void *wmi_hdl,
			  struct extscan_cached_result_params *pcached_results)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_extscan_get_cached_results_cmd)
		return wmi_handle->ops->send_extscan_get_cached_results_cmd(wmi_handle,
			    pcached_results);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_extscan_stop_change_monitor_cmd() - send stop change monitor cmd
 * @wmi_hdl: wmi handle
 * @reset_req: Reset change request params
 *
 * This function sends stop change monitor request to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_extscan_stop_change_monitor_cmd(void *wmi_hdl,
			  struct extscan_capabilities_reset_params *reset_req)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_extscan_stop_change_monitor_cmd)
		return wmi_handle->ops->send_extscan_stop_change_monitor_cmd(wmi_handle,
			    reset_req);

	return QDF_STATUS_E_FAILURE;
}



/**
 * wmi_unified_extscan_start_change_monitor_cmd() - start change monitor cmd
 * @wmi_hdl: wmi handle
 * @psigchange: change monitor request params
 *
 * This function sends start change monitor request to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_extscan_start_change_monitor_cmd(void *wmi_hdl,
				   struct extscan_set_sig_changereq_params *
				   psigchange)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_extscan_start_change_monitor_cmd)
		return wmi_handle->ops->send_extscan_start_change_monitor_cmd(wmi_handle,
			    psigchange);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_extscan_stop_hotlist_monitor_cmd() - stop hotlist monitor
 * @wmi_hdl: wmi handle
 * @photlist_reset: hotlist reset params
 *
 * This function configures hotlist monitor to stop in fw.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_extscan_stop_hotlist_monitor_cmd(void *wmi_hdl,
		  struct extscan_bssid_hotlist_reset_params *photlist_reset)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_extscan_stop_hotlist_monitor_cmd)
		return wmi_handle->ops->send_extscan_stop_hotlist_monitor_cmd(wmi_handle,
			    photlist_reset);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_stop_extscan_cmd() - stop extscan command to fw.
 * @wmi_hdl: wmi handle
 * @pstopcmd: stop scan command request params
 *
 * This function sends stop extscan request to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure.
 */
QDF_STATUS wmi_unified_stop_extscan_cmd(void *wmi_hdl,
			  struct extscan_stop_req_params *pstopcmd)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_stop_extscan_cmd)
		return wmi_handle->ops->send_stop_extscan_cmd(wmi_handle,
			    pstopcmd);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_start_extscan_cmd() - start extscan command to fw.
 * @wmi_hdl: wmi handle
 * @pstart: scan command request params
 *
 * This function sends start extscan request to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure.
 */
QDF_STATUS wmi_unified_start_extscan_cmd(void *wmi_hdl,
			  struct wifi_scan_cmd_req_params *pstart)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_start_extscan_cmd)
		return wmi_handle->ops->send_start_extscan_cmd(wmi_handle,
			    pstart);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_plm_stop_cmd() - plm stop request
 * @wmi_hdl: wmi handle
 * @plm: plm request parameters
 *
 * This function request FW to stop PLM.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_plm_stop_cmd(void *wmi_hdl,
			  const struct plm_req_params *plm)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_plm_stop_cmd)
		return wmi_handle->ops->send_plm_stop_cmd(wmi_handle,
			    plm);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_plm_start_cmd() - plm start request
 * @wmi_hdl: wmi handle
 * @plm: plm request parameters
 *
 * This function request FW to start PLM.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_plm_start_cmd(void *wmi_hdl,
			  const struct plm_req_params *plm,
			  uint32_t *gchannel_list)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_plm_start_cmd)
		return wmi_handle->ops->send_plm_start_cmd(wmi_handle,
			    plm, gchannel_list);

	return QDF_STATUS_E_FAILURE;
}

/**
 * send_pno_stop_cmd() - PNO stop request
 * @wmi_hdl: wmi handle
 * @vdev_id: vdev id
 *
 * This function request FW to stop ongoing PNO operation.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_pno_stop_cmd(void *wmi_hdl, uint8_t vdev_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_pno_stop_cmd)
		return wmi_handle->ops->send_pno_stop_cmd(wmi_handle,
			    vdev_id);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_pno_start_cmd() - PNO start request
 * @wmi_hdl: wmi handle
 * @pno: PNO request
 *
 * This function request FW to start PNO request.
 * Request: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
#ifdef FEATURE_WLAN_SCAN_PNO
QDF_STATUS wmi_unified_pno_start_cmd(void *wmi_hdl,
		   struct pno_scan_req_params *pno)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_pno_start_cmd)
		return wmi_handle->ops->send_pno_start_cmd(wmi_handle,
			    pno);

	return QDF_STATUS_E_FAILURE;
}
#endif

/**
 * wmi_unified_nlo_mawc_cmd() - NLO MAWC cmd configuration
 * @wmi_hdl: wmi handle
 * @params: Configuration parameters
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_nlo_mawc_cmd(void *wmi_hdl,
		struct nlo_mawc_params *params)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_nlo_mawc_cmd)
		return wmi_handle->ops->send_nlo_mawc_cmd(wmi_handle, params);

	return QDF_STATUS_E_FAILURE;
}

/* wmi_unified_set_ric_req_cmd() - set ric request element
 * @wmi_hdl: wmi handle
 * @msg: message
 * @is_add_ts: is addts required
 *
 * This function sets ric request element for 11r roaming.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_ric_req_cmd(void *wmi_hdl, void *msg,
		uint8_t is_add_ts)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_ric_req_cmd)
		return wmi_handle->ops->send_set_ric_req_cmd(wmi_handle, msg,
			    is_add_ts);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_process_ll_stats_clear_cmd() - clear link layer stats
 * @wmi_hdl: wmi handle
 * @clear_req: ll stats clear request command params
 * @addr: mac address
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_process_ll_stats_clear_cmd(void *wmi_hdl,
	 const struct ll_stats_clear_params *clear_req,
	 uint8_t addr[IEEE80211_ADDR_LEN])
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_process_ll_stats_clear_cmd)
		return wmi_handle->ops->send_process_ll_stats_clear_cmd(wmi_handle,
			   clear_req,  addr);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_process_ll_stats_get_cmd() - link layer stats get request
 * @wmi_hdl:wmi handle
 * @get_req:ll stats get request command params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_process_ll_stats_get_cmd(void *wmi_hdl,
		 const struct ll_stats_get_params  *get_req,
		 uint8_t addr[IEEE80211_ADDR_LEN])
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_process_ll_stats_get_cmd)
		return wmi_handle->ops->send_process_ll_stats_get_cmd(wmi_handle,
			   get_req,  addr);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_congestion_request_cmd() - send request to fw to get CCA
 * @wmi_hdl: wma handle
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_congestion_request_cmd(void *wmi_hdl,
		uint8_t vdev_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_congestion_cmd)
		return wmi_handle->ops->send_congestion_cmd(wmi_handle,
			   vdev_id);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_process_ll_stats_set_cmd() - link layer stats set request
 * @wmi_handle:       wmi handle
 * @set_req:  ll stats set request command params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_process_ll_stats_set_cmd(void *wmi_hdl,
		const struct ll_stats_set_params *set_req)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_process_ll_stats_set_cmd)
		return wmi_handle->ops->send_process_ll_stats_set_cmd(wmi_handle,
			   set_req);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_snr_request_cmd() - send request to fw to get RSSI stats
 * @wmi_handle: wmi handle
 * @rssi_req: get RSSI request
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_snr_request_cmd(void *wmi_hdl)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_snr_request_cmd)
		return wmi_handle->ops->send_snr_request_cmd(wmi_handle);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_snr_cmd() - get RSSI from fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_snr_cmd(void *wmi_hdl, uint8_t vdev_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_snr_cmd)
		return wmi_handle->ops->send_snr_cmd(wmi_handle,
			    vdev_id);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_link_status_req_cmd() - process link status request from UMAC
 * @wmi_handle: wmi handle
 * @link_status: get link params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_link_status_req_cmd(void *wmi_hdl,
				 struct link_status_params *link_status)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_link_status_req_cmd)
		return wmi_handle->ops->send_link_status_req_cmd(wmi_handle,
			    link_status);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_process_dhcp_ind() - process dhcp indication from SME
 * @wmi_handle: wmi handle
 * @ta_dhcp_ind: DHCP indication parameter
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
#ifdef CONFIG_MCL
QDF_STATUS wmi_unified_process_dhcp_ind(void *wmi_hdl,
				wmi_peer_set_param_cmd_fixed_param *ta_dhcp_ind)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_process_dhcp_ind_cmd)
		return wmi_handle->ops->send_process_dhcp_ind_cmd(wmi_handle,
			    ta_dhcp_ind);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_get_link_speed_cmd() -send command to get linkspeed
 * @wmi_handle: wmi handle
 * @pLinkSpeed: link speed info
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_get_link_speed_cmd(void *wmi_hdl,
			wmi_mac_addr peer_macaddr)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_get_link_speed_cmd)
		return wmi_handle->ops->send_get_link_speed_cmd(wmi_handle,
			    peer_macaddr);

	return QDF_STATUS_E_FAILURE;
}
#endif

#ifdef WLAN_SUPPORT_GREEN_AP
/**
 * wmi_unified_egap_conf_params_cmd() - send wmi cmd of egap configuration params
 * @wmi_handle:	 wmi handler
 * @egap_params: pointer to egap_params
 *
 * Return:	 0 for success, otherwise appropriate error code
 */
QDF_STATUS wmi_unified_egap_conf_params_cmd(void *wmi_hdl,
				struct wlan_green_ap_egap_params *egap_params)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_egap_conf_params_cmd)
		return wmi_handle->ops->send_egap_conf_params_cmd(wmi_handle,
			    egap_params);

	return QDF_STATUS_E_FAILURE;
}
#endif

/**
 * wmi_unified_fw_profiling_data_cmd() - send FW profiling cmd to WLAN FW
 * @wmi_handl: wmi handle
 * @cmd: Profiling command index
 * @value1: parameter1 value
 * @value2: parameter2 value
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_fw_profiling_data_cmd(void *wmi_hdl,
			uint32_t cmd, uint32_t value1, uint32_t value2)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_fw_profiling_cmd)
		return wmi_handle->ops->send_fw_profiling_cmd(wmi_handle,
			    cmd, value1, value2);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_wow_timer_pattern_cmd() - set timer pattern tlv, so that firmware
 * will wake up host after specified time is elapsed
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @cookie: value to identify reason why host set up wake call.
 * @time: time in ms
 *
 * Return: QDF status
 */
QDF_STATUS wmi_unified_wow_timer_pattern_cmd(void *wmi_hdl, uint8_t vdev_id,
					     uint32_t cookie, uint32_t time)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_wow_timer_pattern_cmd)
		return wmi_handle->ops->send_wow_timer_pattern_cmd(wmi_handle,
							vdev_id, cookie, time);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_nat_keepalive_en_cmd() - enable NAT keepalive filter
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_nat_keepalive_en_cmd(void *wmi_hdl, uint8_t vdev_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_nat_keepalive_en_cmd)
		return wmi_handle->ops->send_nat_keepalive_en_cmd(wmi_handle,
			    vdev_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_wlm_latency_level_cmd(void *wmi_hdl,
					struct wlm_latency_level_param *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_wlm_latency_level_cmd)
		return wmi_handle->ops->send_wlm_latency_level_cmd(wmi_handle,
								   param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_csa_offload_enable() - send CSA offload enable command
 * @wmi_hdl: wmi handle
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_csa_offload_enable(void *wmi_hdl, uint8_t vdev_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_csa_offload_enable_cmd)
		return wmi_handle->ops->send_csa_offload_enable_cmd(wmi_handle,
			    vdev_id);

	return QDF_STATUS_E_FAILURE;
}

#ifdef WLAN_FEATURE_CIF_CFR
QDF_STATUS wmi_unified_oem_dma_ring_cfg(void *wmi_hdl,
				wmi_oem_dma_ring_cfg_req_fixed_param *cfg)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_start_oem_data_cmd)
		return wmi_handle->ops->send_oem_dma_cfg_cmd(wmi_handle, cfg);

	return QDF_STATUS_E_FAILURE;
}
#endif

QDF_STATUS wmi_unified_dbr_ring_cfg(void *wmi_hdl,
				struct direct_buf_rx_cfg_req *cfg)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_dbr_cfg_cmd)
		return wmi_handle->ops->send_dbr_cfg_cmd(wmi_handle, cfg);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_start_oem_data_cmd() - start OEM data request to target
 * @wmi_handle: wmi handle
 * @startOemDataReq: start request params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_start_oem_data_cmd(void *wmi_hdl,
			  uint32_t data_len,
			  uint8_t *data)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_start_oem_data_cmd)
		return wmi_handle->ops->send_start_oem_data_cmd(wmi_handle,
			    data_len, data);

	return QDF_STATUS_E_FAILURE;
}

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
wmi_unified_dfs_phyerr_filter_offload_en_cmd(void *wmi_hdl,
			bool dfs_phyerr_filter_offload)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_dfs_phyerr_filter_offload_en_cmd)
		return wmi_handle->ops->send_dfs_phyerr_filter_offload_en_cmd(wmi_handle,
			    dfs_phyerr_filter_offload);

	return QDF_STATUS_E_FAILURE;
}

#if !defined(REMOVE_PKT_LOG)
/**
 * wmi_unified_pktlog_wmi_send_cmd() - send pktlog enable/disable command to target
 * @wmi_handle: wmi handle
 * @pktlog_event: pktlog event
 * @cmd_id: pktlog cmd id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
#ifdef CONFIG_MCL
QDF_STATUS wmi_unified_pktlog_wmi_send_cmd(void *wmi_hdl,
				   WMI_PKTLOG_EVENT pktlog_event,
				   uint32_t cmd_id,
				   uint8_t user_triggered)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_pktlog_wmi_send_cmd)
		return wmi_handle->ops->send_pktlog_wmi_send_cmd(wmi_handle,
			    pktlog_event, cmd_id, user_triggered);

	return QDF_STATUS_E_FAILURE;
}
#endif
#endif /* REMOVE_PKT_LOG */

/**
 * wmi_unified_wow_delete_pattern_cmd() - delete wow pattern in target
 * @wmi_handle: wmi handle
 * @ptrn_id: pattern id
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_wow_delete_pattern_cmd(void *wmi_hdl, uint8_t ptrn_id,
					uint8_t vdev_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_wow_delete_pattern_cmd)
		return wmi_handle->ops->send_wow_delete_pattern_cmd(wmi_handle,
			    ptrn_id, vdev_id);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_host_wakeup_ind_to_fw_cmd() - send wakeup ind to fw
 * @wmi_handle: wmi handle
 *
 * Sends host wakeup indication to FW. On receiving this indication,
 * FW will come out of WOW.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_host_wakeup_ind_to_fw_cmd(void *wmi_hdl)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_host_wakeup_ind_to_fw_cmd)
		return wmi_handle->ops->send_host_wakeup_ind_to_fw_cmd(wmi_handle);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_del_ts_cmd() - send DELTS request to fw
 * @wmi_handle: wmi handle
 * @msg: delts params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_del_ts_cmd(void *wmi_hdl, uint8_t vdev_id,
				uint8_t ac)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_del_ts_cmd)
		return wmi_handle->ops->send_del_ts_cmd(wmi_handle,
			    vdev_id, ac);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_aggr_qos_cmd() - send aggr qos request to fw
 * @wmi_handle: handle to wmi
 * @aggr_qos_rsp_msg - combined struct for all ADD_TS requests.
 *
 * A function to handle WMI_AGGR_QOS_REQ. This will send out
 * ADD_TS requestes to firmware in loop for all the ACs with
 * active flow.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_aggr_qos_cmd(void *wmi_hdl,
		      struct aggr_add_ts_param *aggr_qos_rsp_msg)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_aggr_qos_cmd)
		return wmi_handle->ops->send_aggr_qos_cmd(wmi_handle,
			    aggr_qos_rsp_msg);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_add_ts_cmd() - send ADDTS request to fw
 * @wmi_handle: wmi handle
 * @msg: ADDTS params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_add_ts_cmd(void *wmi_hdl,
		 struct add_ts_param *msg)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_add_ts_cmd)
		return wmi_handle->ops->send_add_ts_cmd(wmi_handle,
			    msg);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_process_add_periodic_tx_ptrn_cmd - add periodic tx ptrn
 * @wmi_handle: wmi handle
 * @pAddPeriodicTxPtrnParams: tx ptrn params
 *
 * Retrun: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_process_add_periodic_tx_ptrn_cmd(void *wmi_hdl,
						struct periodic_tx_pattern  *
						pAddPeriodicTxPtrnParams,
						uint8_t vdev_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_process_add_periodic_tx_ptrn_cmd)
		return wmi_handle->ops->send_process_add_periodic_tx_ptrn_cmd(wmi_handle,
			    pAddPeriodicTxPtrnParams,
				vdev_id);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_process_del_periodic_tx_ptrn_cmd - del periodic tx ptrn
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @pattern_id: pattern id
 *
 * Retrun: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_process_del_periodic_tx_ptrn_cmd(void *wmi_hdl,
						uint8_t vdev_id,
						uint8_t pattern_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_process_del_periodic_tx_ptrn_cmd)
		return wmi_handle->ops->send_process_del_periodic_tx_ptrn_cmd(wmi_handle,
			    vdev_id,
				pattern_id);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_stats_ext_req_cmd() - request ext stats from fw
 * @wmi_handle: wmi handle
 * @preq: stats ext params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_stats_ext_req_cmd(void *wmi_hdl,
			struct stats_ext_params *preq)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_stats_ext_req_cmd)
		return wmi_handle->ops->send_stats_ext_req_cmd(wmi_handle,
			    preq);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_enable_ext_wow_cmd() - enable ext wow in fw
 * @wmi_handle: wmi handle
 * @params: ext wow params
 *
 * Return:QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_enable_ext_wow_cmd(void *wmi_hdl,
			struct ext_wow_params *params)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_enable_ext_wow_cmd)
		return wmi_handle->ops->send_enable_ext_wow_cmd(wmi_handle,
			    params);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_set_app_type2_params_in_fw_cmd() - set app type2 params in fw
 * @wmi_handle: wmi handle
 * @appType2Params: app type2 params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_app_type2_params_in_fw_cmd(void *wmi_hdl,
					  struct app_type2_params *appType2Params)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_app_type2_params_in_fw_cmd)
		return wmi_handle->ops->send_set_app_type2_params_in_fw_cmd(wmi_handle,
			     appType2Params);

	return QDF_STATUS_E_FAILURE;

}

/**
 * wmi_unified_set_auto_shutdown_timer_cmd() - sets auto shutdown timer in firmware
 * @wmi_handle: wmi handle
 * @timer_val: auto shutdown timer value
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_auto_shutdown_timer_cmd(void *wmi_hdl,
						  uint32_t timer_val)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_auto_shutdown_timer_cmd)
		return wmi_handle->ops->send_set_auto_shutdown_timer_cmd(wmi_handle,
			    timer_val);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_nan_req_cmd() - to send nan request to target
 * @wmi_handle: wmi handle
 * @nan_req: request data which will be non-null
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_nan_req_cmd(void *wmi_hdl,
			struct nan_req_params *nan_req)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_nan_req_cmd)
		return wmi_handle->ops->send_nan_req_cmd(wmi_handle,
			    nan_req);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_process_dhcpserver_offload_cmd() - enable DHCP server offload
 * @wmi_handle: wmi handle
 * @pDhcpSrvOffloadInfo: DHCP server offload info
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_process_dhcpserver_offload_cmd(void *wmi_hdl,
				struct dhcp_offload_info_params *params)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_process_dhcpserver_offload_cmd)
		return wmi_handle->ops->send_process_dhcpserver_offload_cmd(wmi_handle,
			    params);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_process_ch_avoid_update_cmd() - handles channel avoid update request
 * @wmi_handle: wmi handle
 * @ch_avoid_update_req: channel avoid update params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_process_ch_avoid_update_cmd(void *wmi_hdl)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_process_ch_avoid_update_cmd)
		return wmi_handle->ops->send_process_ch_avoid_update_cmd(wmi_handle);

	return QDF_STATUS_E_FAILURE;
}

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
QDF_STATUS wmi_unified_send_regdomain_info_to_fw_cmd(void *wmi_hdl,
				   uint32_t reg_dmn, uint16_t regdmn2G,
				   uint16_t regdmn5G, uint8_t ctl2G,
				   uint8_t ctl5G)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_regdomain_info_to_fw_cmd)
		return wmi_handle->ops->send_regdomain_info_to_fw_cmd(wmi_handle,
			    reg_dmn, regdmn2G,
				regdmn5G, ctl2G,
				ctl5G);

	return QDF_STATUS_E_FAILURE;
}


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
QDF_STATUS wmi_unified_set_tdls_offchan_mode_cmd(void *wmi_hdl,
			      struct tdls_channel_switch_params *chan_switch_params)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_tdls_offchan_mode_cmd)
		return wmi_handle->ops->send_set_tdls_offchan_mode_cmd(wmi_handle,
			    chan_switch_params);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_update_fw_tdls_state_cmd() - send enable/disable tdls for a vdev
 * @wmi_handle: wmi handle
 * @pwmaTdlsparams: TDLS params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_update_fw_tdls_state_cmd(void *wmi_hdl,
					 void *tdls_param, uint8_t tdls_state)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_update_fw_tdls_state_cmd)
		return wmi_handle->ops->send_update_fw_tdls_state_cmd(wmi_handle,
			    tdls_param, tdls_state);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_update_tdls_peer_state_cmd() - update TDLS peer state
 * @wmi_handle: wmi handle
 * @peerStateParams: TDLS peer state params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_update_tdls_peer_state_cmd(void *wmi_hdl,
			       struct tdls_peer_state_params *peerStateParams,
				   uint32_t *ch_mhz)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_update_tdls_peer_state_cmd)
		return wmi_handle->ops->send_update_tdls_peer_state_cmd(wmi_handle,
			    peerStateParams, ch_mhz);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_process_set_ie_info_cmd() - Function to send IE info to firmware
 * @wmi_handle:    Pointer to WMi handle
 * @ie_data:       Pointer for ie data
 *
 * This function sends IE information to firmware
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 *
 */
QDF_STATUS wmi_unified_process_set_ie_info_cmd(void *wmi_hdl,
				   struct vdev_ie_info_param *ie_info)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_process_set_ie_info_cmd)
		return wmi_handle->ops->send_process_set_ie_info_cmd(wmi_handle,
			    ie_info);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_save_fw_version_cmd() - save fw version
 * @wmi_handle:      pointer to wmi handle
 * @res_cfg:         resource config
 * @num_mem_chunks:  no of mem chunck
 * @mem_chunk:       pointer to mem chunck structure
 *
 * This function sends IE information to firmware
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 *
 */
QDF_STATUS wmi_unified_save_fw_version_cmd(void *wmi_hdl,
		void *evt_buf)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->save_fw_version_cmd)
		return wmi_handle->ops->save_fw_version_cmd(wmi_handle,
			    evt_buf);

	return QDF_STATUS_E_FAILURE;
}

/**
 * send_set_base_macaddr_indicate_cmd() - set base mac address in fw
 * @wmi_hdl: wmi handle
 * @custom_addr: base mac address
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_base_macaddr_indicate_cmd(void *wmi_hdl,
					 uint8_t *custom_addr)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_base_macaddr_indicate_cmd)
		return wmi_handle->ops->send_set_base_macaddr_indicate_cmd(wmi_handle,
			    custom_addr);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_log_supported_evt_cmd() - Enable/Disable FW diag/log events
 * @wmi_hdl: wmi handle
 * @event:  Event received from FW
 * @len:    Length of the event
 *
 * Enables the low frequency events and disables the high frequency
 * events. Bit 17 indicates if the event if low/high frequency.
 * 1 - high frequency, 0 - low frequency
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failures
 */
QDF_STATUS wmi_unified_log_supported_evt_cmd(void *wmi_hdl,
		uint8_t *event,
		uint32_t len)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_log_supported_evt_cmd)
		return wmi_handle->ops->send_log_supported_evt_cmd(wmi_handle,
			    event, len);

	return QDF_STATUS_E_FAILURE;
}

void wmi_send_time_stamp_sync_cmd_tlv(void *wmi_hdl)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;
	if (wmi_handle->ops->send_time_stamp_sync_cmd)
		wmi_handle->ops->send_time_stamp_sync_cmd(wmi_handle);

}
/**
 * wmi_unified_enable_specific_fw_logs_cmd() - Start/Stop logging of diag log id
 * @wmi_hdl: wmi handle
 * @start_log: Start logging related parameters
 *
 * Send the command to the FW based on which specific logging of diag
 * event/log id can be started/stopped
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_enable_specific_fw_logs_cmd(void *wmi_hdl,
		struct wmi_wifi_start_log *start_log)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_enable_specific_fw_logs_cmd)
		return wmi_handle->ops->send_enable_specific_fw_logs_cmd(wmi_handle,
			    start_log);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_flush_logs_to_fw_cmd() - Send log flush command to FW
 * @wmi_hdl: WMI handle
 *
 * This function is used to send the flush command to the FW,
 * that will flush the fw logs that are residue in the FW
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_flush_logs_to_fw_cmd(void *wmi_hdl)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_flush_logs_to_fw_cmd)
		return wmi_handle->ops->send_flush_logs_to_fw_cmd(wmi_handle);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_pdev_set_pcl_cmd() - Send WMI_SOC_SET_PCL_CMDID to FW
 * @wmi_hdl: wmi handle
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
QDF_STATUS wmi_unified_pdev_set_pcl_cmd(void *wmi_hdl,
				struct wmi_pcl_chan_weights *msg)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_pdev_set_pcl_cmd)
		return wmi_handle->ops->send_pdev_set_pcl_cmd(wmi_handle, msg);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_soc_set_hw_mode_cmd() - Send WMI_SOC_SET_HW_MODE_CMDID to FW
 * @wmi_hdl: wmi handle
 * @msg: Structure containing the following parameters
 *
 * - hw_mode_index: The HW_Mode field is a enumerated type that is selected
 * from the HW_Mode table, which is returned in the WMI_SERVICE_READY_EVENTID.
 *
 * Provides notification to the WLAN firmware that host driver is requesting a
 * HardWare (HW) Mode change. This command is needed to support iHelium in the
 * configurations that include the Dual Band Simultaneous (DBS) feature.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_soc_set_hw_mode_cmd(void *wmi_hdl,
				uint32_t hw_mode_index)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_pdev_set_hw_mode_cmd)
		return wmi_handle->ops->send_pdev_set_hw_mode_cmd(wmi_handle,
				  hw_mode_index);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_pdev_set_dual_mac_config_cmd() - Set dual mac config to FW
 * @wmi_hdl: wmi handle
 * @msg: Dual MAC config parameters
 *
 * Configures WLAN firmware with the dual MAC features
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failures.
 */
QDF_STATUS wmi_unified_pdev_set_dual_mac_config_cmd(void *wmi_hdl,
		struct wmi_dual_mac_config *msg)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_pdev_set_dual_mac_config_cmd)
		return wmi_handle->ops->send_pdev_set_dual_mac_config_cmd(wmi_handle,
				  msg);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_set_led_flashing_cmd() - set led flashing in fw
 * @wmi_hdl: wmi handle
 * @flashing: flashing request
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_led_flashing_cmd(void *wmi_hdl,
				struct flashing_req_params *flashing)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_led_flashing_cmd)
		return wmi_handle->ops->send_set_led_flashing_cmd(wmi_handle,
				  flashing);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_app_type1_params_in_fw_cmd() - set app type1 params in fw
 * @wmi_hdl: wmi handle
 * @appType1Params: app type1 params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_app_type1_params_in_fw_cmd(void *wmi_hdl,
				   struct app_type1_params *app_type1_params)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_app_type1_params_in_fw_cmd)
		return wmi_handle->ops->send_app_type1_params_in_fw_cmd(wmi_handle,
				  app_type1_params);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_set_ssid_hotlist_cmd() - Handle an SSID hotlist set request
 * @wmi_hdl: wmi handle
 * @request: SSID hotlist set request
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_set_ssid_hotlist_cmd(void *wmi_hdl,
		     struct ssid_hotlist_request_params *request)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_ssid_hotlist_cmd)
		return wmi_handle->ops->send_set_ssid_hotlist_cmd(wmi_handle,
				  request);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_roam_synch_complete_cmd() - roam synch complete command to fw.
 * @wmi_hdl: wmi handle
 * @vdev_id: vdev id
 *
 * This function sends roam synch complete event to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_roam_synch_complete_cmd(void *wmi_hdl,
		 uint8_t vdev_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_process_roam_synch_complete_cmd)
		return wmi_handle->ops->send_process_roam_synch_complete_cmd(wmi_handle,
				  vdev_id);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_fw_test_cmd() - send fw test command to fw.
 * @wmi_hdl: wmi handle
 * @wmi_fwtest: fw test command
 *
 * This function sends fw test command to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_fw_test_cmd(void *wmi_hdl,
				   struct set_fwtest_params *wmi_fwtest)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_fw_test_cmd)
		return wmi_handle->ops->send_fw_test_cmd(wmi_handle,
				  wmi_fwtest);

	return QDF_STATUS_E_FAILURE;

}

/**
 * wmi_unified_unit_test_cmd() - send unit test command to fw.
 * @wmi_hdl: wmi handle
 * @wmi_utest: unit test command
 *
 * This function send unit test command to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_unit_test_cmd(void *wmi_hdl,
			       struct wmi_unit_test_cmd *wmi_utest)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_unit_test_cmd)
		return wmi_handle->ops->send_unit_test_cmd(wmi_handle,
				  wmi_utest);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified__roam_invoke_cmd() - send roam invoke command to fw.
 * @wmi_hdl: wmi handle
 * @roaminvoke: roam invoke command
 *
 * Send roam invoke command to fw for fastreassoc.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_roam_invoke_cmd(void *wmi_hdl,
		struct wmi_roam_invoke_cmd *roaminvoke,
		uint32_t ch_hz)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_roam_invoke_cmd)
		return wmi_handle->ops->send_roam_invoke_cmd(wmi_handle,
				  roaminvoke, ch_hz);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_roam_scan_offload_cmd() - set roam offload command
 * @wmi_hdl: wmi handle
 * @command: command
 * @vdev_id: vdev id
 *
 * This function set roam offload command to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_roam_scan_offload_cmd(void *wmi_hdl,
					 uint32_t command, uint32_t vdev_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_roam_scan_offload_cmd)
		return wmi_handle->ops->send_roam_scan_offload_cmd(wmi_handle,
				  command, vdev_id);

	return QDF_STATUS_E_FAILURE;
}
#ifdef CONFIG_MCL
/**
 * wmi_unified_send_roam_scan_offload_ap_cmd() - set roam ap profile in fw
 * @wmi_hdl: wmi handle
 * @ap_profile: ap profile params
 *
 * Send WMI_ROAM_AP_PROFILE to firmware
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_send_roam_scan_offload_ap_cmd(void *wmi_hdl,
					   struct ap_profile_params *ap_profile)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_roam_scan_offload_ap_profile_cmd)
		return wmi_handle->ops->send_roam_scan_offload_ap_profile_cmd(
				  wmi_handle, ap_profile);

	return QDF_STATUS_E_FAILURE;
}
#endif
/**
 * wmi_unified_roam_scan_offload_scan_period() - set roam offload scan period
 * @wmi_handle: wmi handle
 * @scan_period: scan period
 * @scan_age: scan age
 * @vdev_id: vdev id
 *
 * Send WMI_ROAM_SCAN_PERIOD parameters to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_roam_scan_offload_scan_period(void *wmi_hdl,
					     uint32_t scan_period,
					     uint32_t scan_age,
					     uint32_t vdev_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_roam_scan_offload_scan_period_cmd)
		return wmi_handle->ops->send_roam_scan_offload_scan_period_cmd(wmi_handle,
				  scan_period, scan_age, vdev_id);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_roam_scan_offload_chan_list_cmd() - set roam offload channel list
 * @wmi_handle: wmi handle
 * @chan_count: channel count
 * @chan_list: channel list
 * @list_type: list type
 * @vdev_id: vdev id
 *
 * Set roam offload channel list.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_roam_scan_offload_chan_list_cmd(void *wmi_hdl,
				   uint8_t chan_count,
				   uint32_t *chan_list,
				   uint8_t list_type, uint32_t vdev_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_roam_scan_offload_chan_list_cmd)
		return wmi_handle->ops->send_roam_scan_offload_chan_list_cmd(wmi_handle,
				  chan_count, chan_list,
				  list_type, vdev_id);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_roam_scan_offload_rssi_change_cmd() - set roam offload RSSI th
 * @wmi_hdl: wmi handle
 * @rssi_change_thresh: RSSI Change threshold
 * @bcn_rssi_weight: beacon RSSI weight
 * @vdev_id: vdev id
 *
 * Send WMI_ROAM_SCAN_RSSI_CHANGE_THRESHOLD parameters to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_roam_scan_offload_rssi_change_cmd(void *wmi_hdl,
	uint32_t vdev_id,
	int32_t rssi_change_thresh,
	uint32_t bcn_rssi_weight,
	uint32_t hirssi_delay_btw_scans)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_roam_scan_offload_rssi_change_cmd)
		return wmi_handle->ops->send_roam_scan_offload_rssi_change_cmd(wmi_handle,
				  vdev_id, rssi_change_thresh,
				  bcn_rssi_weight, hirssi_delay_btw_scans);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_set_per_roam_config(void *wmi_hdl,
		struct wmi_per_roam_config_req *req_buf)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_per_roam_config_cmd)
		return wmi_handle->ops->send_per_roam_config_cmd(wmi_handle,
				req_buf);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_set_arp_stats_req() - set arp stats request
 * @wmi_hdl: wmi handle
 * @req_buf: pointer to set_arp_stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_arp_stats_req(void *wmi_hdl,
					 struct set_arp_stats *req_buf)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_arp_stats_req_cmd)
		return wmi_handle->ops->send_set_arp_stats_req_cmd(wmi_handle,
								   req_buf);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_get_arp_stats_req() - get arp stats request
 * @wmi_hdl: wmi handle
 * @req_buf: pointer to get_arp_stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_get_arp_stats_req(void *wmi_hdl,
					 struct get_arp_stats *req_buf)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_get_arp_stats_req_cmd)
		return wmi_handle->ops->send_get_arp_stats_req_cmd(wmi_handle,
								   req_buf);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_set_del_pmkid_cache(void *wmi_hdl,
					struct wmi_unified_pmk_cache *req_buf)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_del_pmkid_cache_cmd)
		return wmi_handle->ops->send_set_del_pmkid_cache_cmd(wmi_handle,
								     req_buf);

	return QDF_STATUS_E_FAILURE;
}

#if defined(WLAN_FEATURE_FILS_SK)
QDF_STATUS wmi_unified_roam_send_hlp_cmd(void *wmi_hdl,
					 struct hlp_params *req_buf)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_roam_scan_hlp_cmd)
		return wmi_handle->ops->send_roam_scan_hlp_cmd(wmi_handle,
								    req_buf);

	return QDF_STATUS_E_FAILURE;
}
#endif

/**
 * wmi_unified_get_buf_extscan_hotlist_cmd() - prepare hotlist command
 * @wmi_hdl: wmi handle
 * @photlist: hotlist command params
 * @buf_len: buffer length
 *
 * This function fills individual elements for  hotlist request and
 * TLV for bssid entries
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure.
 */
QDF_STATUS wmi_unified_get_buf_extscan_hotlist_cmd(void *wmi_hdl,
				   struct ext_scan_setbssi_hotlist_params *
				   photlist, int *buf_len)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_get_buf_extscan_hotlist_cmd)
		return wmi_handle->ops->send_get_buf_extscan_hotlist_cmd(wmi_handle,
				  photlist, buf_len);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_set_active_bpf_mode_cmd(void *wmi_hdl,
				uint8_t vdev_id,
				enum wmi_host_active_bpf_mode ucast_mode,
				enum wmi_host_active_bpf_mode mcast_bcast_mode)
{
	wmi_unified_t wmi = (wmi_unified_t)wmi_hdl;

	if (!wmi->ops->send_set_active_bpf_mode_cmd) {
		WMI_LOGD("send_set_active_bpf_mode_cmd op is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	return wmi->ops->send_set_active_bpf_mode_cmd(wmi, vdev_id,
						      ucast_mode,
						      mcast_bcast_mode);
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
 *  wmi_unified_set_bwf_cmd_send() - WMI set bwf function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to set bwf param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_bwf_cmd_send(void *wmi_hdl,
				struct set_bwf_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_bwf_cmd)
		return wmi_handle->ops->send_set_bwf_cmd(wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_set_atf_cmd_send() - WMI set atf function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to set atf param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_atf_cmd_send(void *wmi_hdl,
				struct set_atf_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_atf_cmd)
		return wmi_handle->ops->send_set_atf_cmd(wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_pdev_fips_cmd_send() - WMI pdev fips cmd function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold pdev fips param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_pdev_fips_cmd_send(void *wmi_hdl,
				struct fips_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_pdev_fips_cmd)
		return wmi_handle->ops->send_pdev_fips_cmd(wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_wlan_profile_enable_cmd_send() - WMI wlan profile enable cmd function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold wlan profile param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_wlan_profile_enable_cmd_send(void *wmi_hdl,
				struct wlan_profile_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_wlan_profile_enable_cmd)
		return wmi_handle->ops->send_wlan_profile_enable_cmd(wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_wlan_profile_trigger_cmd_send() - WMI wlan profile trigger cmd function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold wlan profile param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_wlan_profile_trigger_cmd_send(void *wmi_hdl,
				struct wlan_profile_params *param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->send_wlan_profile_trigger_cmd)
		return wmi->ops->send_wlan_profile_trigger_cmd(wmi,
				param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_set_chan_cmd_send() - WMI set channel cmd function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold channel param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_chan_cmd_send(void *wmi_hdl,
				struct channel_param *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_pdev_set_chan_cmd)
		return wmi_handle->ops->send_pdev_set_chan_cmd(wmi_handle,
				param);

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
 *  wmi_unified_set_ant_switch_tbl_cmd_send() - WMI ant switch tbl cmd function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold ant switch tbl param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_ant_switch_tbl_cmd_send(void *wmi_hdl,
				struct ant_switch_tbl_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_ant_switch_tbl_cmd)
		return wmi_handle->ops->send_set_ant_switch_tbl_cmd(wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_set_ratepwr_table_cmd_send() - WMI ratepwr table cmd function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold ratepwr table param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_ratepwr_table_cmd_send(void *wmi_hdl,
				struct ratepwr_table_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_ratepwr_table_cmd)
		return wmi_handle->ops->send_set_ratepwr_table_cmd(wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_get_ratepwr_table_cmd_send() - WMI ratepwr table cmd function
 *  @param wmi_handle      : handle to WMI.
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_get_ratepwr_table_cmd_send(void *wmi_hdl)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_get_ratepwr_table_cmd)
		return wmi_handle->ops->send_get_ratepwr_table_cmd(wmi_handle);

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
 *  wmi_unified_set_ratepwr_chainmsk_cmd_send() - WMI ratepwr
 *  chainmsk cmd function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold ratepwr chainmsk param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_ratepwr_chainmsk_cmd_send(void *wmi_hdl,
				struct ratepwr_chainmsk_params *param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->send_set_ratepwr_chainmsk_cmd)
		return wmi->ops->send_set_ratepwr_chainmsk_cmd(wmi, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_set_macaddr_cmd_send() - WMI set macaddr cmd function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold macaddr param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_macaddr_cmd_send(void *wmi_hdl,
				struct macaddr_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_macaddr_cmd)
		return wmi_handle->ops->send_set_macaddr_cmd(wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_pdev_scan_start_cmd_send() - WMI pdev scan start cmd function
 *  @param wmi_handle      : handle to WMI.
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_pdev_scan_start_cmd_send(void *wmi_hdl)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_pdev_scan_start_cmd)
		return wmi_handle->ops->send_pdev_scan_start_cmd(wmi_handle);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_pdev_scan_end_cmd_send() - WMI pdev scan end cmd function
 *  @param wmi_handle      : handle to WMI.
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_pdev_scan_end_cmd_send(void *wmi_hdl)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_pdev_scan_end_cmd)
		return wmi_handle->ops->send_pdev_scan_end_cmd(wmi_handle);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_set_acparams_cmd_send() - WMI set acparams cmd function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold acparams param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_acparams_cmd_send(void *wmi_hdl,
				struct acparams_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_acparams_cmd)
		return wmi_handle->ops->send_set_acparams_cmd(wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_set_vap_dscp_tid_map_cmd_send() - WMI set vap dscp
 *  tid map cmd function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold dscp param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_vap_dscp_tid_map_cmd_send(void *wmi_hdl,
				struct vap_dscp_tid_map_params *param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->send_set_vap_dscp_tid_map_cmd)
		return wmi->ops->send_set_vap_dscp_tid_map_cmd(wmi, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_proxy_ast_reserve_cmd_send() - WMI proxy ast
 *  reserve cmd function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold ast param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_proxy_ast_reserve_cmd_send(void *wmi_hdl,
				struct proxy_ast_reserve_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_proxy_ast_reserve_cmd)
		return wmi_handle->ops->send_proxy_ast_reserve_cmd(wmi_handle,
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
 *  wmi_unified_set_bridge_mac_addr_cmd_send() - WMI set bridge mac addr cmd function
 *  @param wmi_hdl      : handle to WMI.
 *  @param param        : pointer to hold bridge mac addr param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_bridge_mac_addr_cmd_send(void *wmi_hdl,
				struct set_bridge_mac_addr_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_bridge_mac_addr_cmd)
		return wmi_handle->ops->send_set_bridge_mac_addr_cmd(wmi_handle,
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
 *  wmi_unified_phyerr_enable_cmd_send() - WMI phyerr enable cmd function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold phyerr enable param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_phyerr_enable_cmd_send(void *wmi_hdl)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_phyerr_enable_cmd)
		return wmi_handle->ops->send_phyerr_enable_cmd(wmi_handle);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_phyerr_disable_cmd_send() - WMI phyerr disable cmd function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold phyerr disable param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_phyerr_disable_cmd_send(void *wmi_hdl)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_phyerr_disable_cmd)
		return wmi_handle->ops->send_phyerr_disable_cmd(wmi_handle);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_smart_ant_enable_cmd_send() - WMI smart ant enable function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold antenna param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_smart_ant_enable_cmd_send(void *wmi_hdl,
				struct smart_ant_enable_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_smart_ant_enable_cmd)
		return wmi_handle->ops->send_smart_ant_enable_cmd(wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_smart_ant_set_rx_ant_cmd_send() - WMI set rx antenna function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold antenna param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_smart_ant_set_rx_ant_cmd_send(void *wmi_hdl,
				struct smart_ant_rx_ant_params *param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->send_smart_ant_set_rx_ant_cmd)
		return wmi->ops->send_smart_ant_set_rx_ant_cmd(wmi, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_smart_ant_set_tx_ant_cmd_send() - WMI set tx antenna function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold antenna param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_smart_ant_set_tx_ant_cmd_send(void *wmi_hdl,
		uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct smart_ant_tx_ant_params *param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->send_smart_ant_set_tx_ant_cmd)
		return wmi->ops->send_smart_ant_set_tx_ant_cmd(wmi, macaddr,
				param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_smart_ant_set_training_info_cmd_send() - WMI set tx antenna function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold antenna param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_smart_ant_set_training_info_cmd_send(void *wmi_hdl,
		uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct smart_ant_training_info_params *param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->send_smart_ant_set_training_info_cmd)
		return wmi->ops->send_smart_ant_set_training_info_cmd(wmi,
				macaddr, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_smart_ant_node_config_cmd_send() - WMI set node config function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold node parameter
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_smart_ant_node_config_cmd_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct smart_ant_node_config_params *param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->send_smart_ant_set_node_config_cmd)
		return wmi->ops->send_smart_ant_set_node_config_cmd(wmi,
				  macaddr, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_smart_ant_enable_tx_feedback_cmd_send() - WMI set tx antenna function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold antenna param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_smart_ant_enable_tx_feedback_cmd_send(void *wmi_hdl,
			struct smart_ant_enable_tx_feedback_params *param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->send_smart_ant_enable_tx_feedback_cmd)
		return wmi->ops->send_smart_ant_enable_tx_feedback_cmd(wmi,
				param);

	return QDF_STATUS_E_FAILURE;
}
qdf_export_symbol(wmi_unified_smart_ant_enable_tx_feedback_cmd_send);

/**
 *  wmi_unified_vdev_spectral_configure_cmd_send() - WMI set spectral config function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold spectral config param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_spectral_configure_cmd_send(void *wmi_hdl,
				struct vdev_spectral_configure_params *param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->send_vdev_spectral_configure_cmd)
		return wmi->ops->send_vdev_spectral_configure_cmd(wmi, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_vdev_spectral_enable_cmd_send() - WMI enable spectral function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold enable spectral param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_spectral_enable_cmd_send(void *wmi_hdl,
				struct vdev_spectral_enable_params *param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->send_vdev_spectral_enable_cmd)
		return wmi->ops->send_vdev_spectral_enable_cmd(wmi, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_bss_chan_info_request_cmd_send() - WMI bss chan info request function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold chan info param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_bss_chan_info_request_cmd_send(void *wmi_hdl,
				struct bss_chan_info_request_params *param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->send_bss_chan_info_request_cmd)
		return wmi->ops->send_bss_chan_info_request_cmd(wmi, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_thermal_mitigation_param_cmd_send() - WMI thermal mitigation function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold thermal mitigation param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_thermal_mitigation_param_cmd_send(void *wmi_hdl,
				struct thermal_mitigation_params *param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->send_thermal_mitigation_param_cmd)
		return wmi->ops->send_thermal_mitigation_param_cmd(wmi, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_vdev_set_neighbour_rx_cmd_send() - WMI set neighbour rx function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
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
 *  wmi_unified_vdev_set_fwtest_param_cmd_send() - WMI set fwtest function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold fwtest param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_set_fwtest_param_cmd_send(void *wmi_hdl,
				struct set_fwtest_params *param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->send_vdev_set_fwtest_param_cmd)
		return wmi->ops->send_vdev_set_fwtest_param_cmd(wmi, param);

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
 * wmi_unified_vdev_set_custom_aggr_size_cmd_send() - WMI set custom aggr
 * size function
 * @param wmi_handle	: handle to WMI
 * @param param		: pointer to hold custom aggr size param
 *
 * @return QDF_STATUS_SUCCESS on success and QDF_STATUS_R_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_set_custom_aggr_size_cmd_send(void *wmi_hdl,
				struct set_custom_aggr_size_params *param)
{
	wmi_unified_t wmi = (wmi_unified_t)wmi_hdl;

	if (wmi->ops->send_vdev_set_custom_aggr_size_cmd)
		return wmi->ops->send_vdev_set_custom_aggr_size_cmd(wmi, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_vdev_set_qdepth_thresh_cmd_send() - WMI set qdepth threshold
 * @param wmi_handle	: handle to WMI
 * @param param		: pointer to hold qdepth threshold params
 *
 * @return QDF_STATUS_SUCCESS on success and QDF_STATUS_R_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_set_qdepth_thresh_cmd_send(void *wmi_hdl,
				struct set_qdepth_thresh_params *param)
{
	wmi_unified_t wmi = (wmi_unified_t)wmi_hdl;

	if (wmi->ops->send_vdev_set_qdepth_thresh_cmd)
		return wmi->ops->send_vdev_set_qdepth_thresh_cmd(wmi, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_pdev_set_regdomain_params_cmd_send() - WMI set regdomain function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold regdomain param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_pdev_set_regdomain_cmd_send(void *wmi_hdl,
				struct pdev_set_regdomain_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_pdev_set_regdomain_cmd)
		return wmi_handle->ops->send_pdev_set_regdomain_cmd(wmi_handle,
				param);

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
 *  wmi_unified_set_beacon_filter_cmd_send() - WMI set beacon filter function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold beacon filter param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_beacon_filter_cmd_send(void *wmi_hdl,
				struct set_beacon_filter_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_beacon_filter_cmd)
		return wmi_handle->ops->send_set_beacon_filter_cmd(wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_remove_beacon_filter_cmd_send() - WMI set beacon filter function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold beacon filter param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_remove_beacon_filter_cmd_send(void *wmi_hdl,
				struct remove_beacon_filter_params *param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->send_remove_beacon_filter_cmd)
		return wmi->ops->send_remove_beacon_filter_cmd(wmi, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_mgmt_cmd_send() - WMI mgmt cmd function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold mgmt parameter
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
#if 0
QDF_STATUS wmi_unified_mgmt_cmd_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct mgmt_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_mgmt_cmd)
		return wmi_handle->ops->send_mgmt_cmd(wmi_handle,
				  macaddr, param);

	return QDF_STATUS_E_FAILURE;
}
#endif

/**
 *  wmi_unified_addba_clearresponse_cmd_send() - WMI addba resp cmd function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold addba resp parameter
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_addba_clearresponse_cmd_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct addba_clearresponse_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_addba_clearresponse_cmd)
		return wmi_handle->ops->send_addba_clearresponse_cmd(wmi_handle,
				  macaddr, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_addba_send_cmd_send() - WMI addba send function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold addba parameter
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_addba_send_cmd_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct addba_send_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_addba_send_cmd)
		return wmi_handle->ops->send_addba_send_cmd(wmi_handle,
				  macaddr, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_delba_send_cmd_send() - WMI delba cmd function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold delba parameter
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_delba_send_cmd_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct delba_send_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_delba_send_cmd)
		return wmi_handle->ops->send_delba_send_cmd(wmi_handle,
				  macaddr, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_addba_setresponse_cmd_send() - WMI addba set resp cmd function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold addba set resp parameter
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_addba_setresponse_cmd_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct addba_setresponse_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_addba_setresponse_cmd)
		return wmi_handle->ops->send_addba_setresponse_cmd(wmi_handle,
				  macaddr, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_singleamsdu_cmd_send() - WMI singleamsdu function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold singleamsdu parameter
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_singleamsdu_cmd_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct singleamsdu_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_singleamsdu_cmd)
		return wmi_handle->ops->send_singleamsdu_cmd(wmi_handle,
				  macaddr, param);

	return QDF_STATUS_E_FAILURE;
}

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
 *  wmi_unified_mu_scan_cmd_send() - WMI set mu scan function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold mu scan param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_mu_scan_cmd_send(void *wmi_hdl,
				struct mu_scan_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_mu_scan_cmd)
		return wmi_handle->ops->send_mu_scan_cmd(wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_lteu_config_cmd_send() - WMI set mu scan function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold mu scan param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_lteu_config_cmd_send(void *wmi_hdl,
				struct lteu_config_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_lteu_config_cmd)
		return wmi_handle->ops->send_lteu_config_cmd(wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_set_psmode_cmd_send() - WMI set mu scan function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold mu scan param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_psmode_cmd_send(void *wmi_hdl,
				struct set_ps_mode_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_ps_mode_cmd)
		return wmi_handle->ops->send_set_ps_mode_cmd(wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_init_cmd_send() - send initialization cmd to fw
 * @wmi_handle: wmi handle
 * @param param: pointer to wmi init param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_init_cmd_send(void *wmi_hdl,
				struct wmi_init_cmd_param *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->init_cmd_send)
		return wmi_handle->ops->init_cmd_send(wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_save_service_bitmap() - save service bitmap
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_save_service_bitmap(void *wmi_hdl, void *evt_buf,
				   void *bitmap_buf)
{
	struct wmi_unified *wmi_handle = (struct wmi_unified *) wmi_hdl;

	if (wmi_handle->ops->save_service_bitmap) {
		return wmi_handle->ops->save_service_bitmap(wmi_handle, evt_buf,
						     bitmap_buf);
	}
	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_save_ext_service_bitmap() - save extended service bitmap
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_save_ext_service_bitmap(void *wmi_hdl, void *evt_buf,
				   void *bitmap_buf)
{
	struct wmi_unified *wmi_handle = (struct wmi_unified *) wmi_hdl;

	if (wmi_handle->ops->save_ext_service_bitmap) {
		return wmi_handle->ops->save_ext_service_bitmap(wmi_handle,
				evt_buf, bitmap_buf);
	}
	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_save_fw_version() - Save fw version
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_save_fw_version(void *wmi_hdl, void *evt_buf)
{
	struct wmi_unified *wmi_handle = (struct wmi_unified *) wmi_hdl;

	if (wmi_handle->ops->save_fw_version) {
		wmi_handle->ops->save_fw_version(wmi_handle, evt_buf);
		return 0;
	}
	return QDF_STATUS_E_FAILURE;

}

/**
 * wmi_check_and_update_fw_version() - Ready and fw version check
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_check_and_update_fw_version(void *wmi_hdl, void *evt_buf)
{
	struct wmi_unified *wmi_handle = (struct wmi_unified *) wmi_hdl;

	if (wmi_handle->ops->check_and_update_fw_version)
		return wmi_handle->ops->check_and_update_fw_version(wmi_handle,
				evt_buf);

	return QDF_STATUS_E_FAILURE;

}

/**
 * wmi_service_enabled() - Check if service enabled
 * @param wmi_handle: wmi handle
 * @param service_id: service identifier
 *
 * Return: 1 enabled, 0 disabled
 */
bool wmi_service_enabled(void *wmi_hdl, uint32_t service_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if ((service_id < wmi_services_max) &&
		(wmi_handle->services[service_id] != WMI_SERVICE_UNAVAILABLE)) {
		if (wmi_handle->ops->is_service_enabled) {
			return wmi_handle->ops->is_service_enabled(wmi_handle,
				wmi_handle->services[service_id]);
		}
	} else {
		qdf_print("Support not added yet for Service %d\n", service_id);
	}

	return false;
}

/**
 * wmi_get_target_cap_from_service_ready() - extract service ready event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to received event buffer
 * @param ev: pointer to hold target capability information extracted from even
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_get_target_cap_from_service_ready(void *wmi_hdl,
	void *evt_buf, struct wlan_psoc_target_capability_info *ev)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->get_target_cap_from_service_ready)
		return wmi->ops->get_target_cap_from_service_ready(wmi,
				evt_buf, ev);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_fw_version() - extract fw version
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param fw_ver: Pointer to hold fw version
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_fw_version(void *wmi_hdl,
				void *evt_buf, struct wmi_host_fw_ver *fw_ver)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_fw_version)
		return wmi_handle->ops->extract_fw_version(wmi_handle,
				evt_buf, fw_ver);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_fw_abi_version() - extract fw abi version
 * @wmi_handle: wmi handle
 * @param evt_buf: Pointer to event buffer
 * @param fw_ver: Pointer to hold fw abi version
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_fw_abi_version(void *wmi_hdl,
			void *evt_buf, struct wmi_host_fw_abi_ver *fw_ver)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_fw_abi_version)
		return wmi_handle->ops->extract_fw_abi_version(wmi_handle,
		evt_buf, fw_ver);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_hal_reg_cap() - extract HAL registered capabilities
 * @wmi_handle: wmi handle
 * @param evt_buf: Pointer to event buffer
 * @param hal_reg_cap: pointer to hold HAL reg capabilities
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_hal_reg_cap(void *wmi_hdl, void *evt_buf,
	struct wlan_psoc_hal_reg_capability *hal_reg_cap)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_hal_reg_cap)
		return wmi_handle->ops->extract_hal_reg_cap(wmi_handle,
			evt_buf, hal_reg_cap);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_host_mem_req_from_service_ready() - Extract host memory
 *                                                 request event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param num_entries: pointer to hold number of entries requested
 *
 * Return: Number of entries requested
 */
host_mem_req *wmi_extract_host_mem_req_from_service_ready(void *wmi_hdl,
	void *evt_buf, uint8_t *num_entries)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_host_mem_req)
		return wmi_handle->ops->extract_host_mem_req(wmi_handle,
			evt_buf, num_entries);

	*num_entries = 0;
	return NULL;
}

/**
 * wmi_ready_extract_init_status() - Extract init status from ready event
 * @wmi_handle: wmi handle
 * @param ev: Pointer to event buffer
 *
 * Return: ready status
 */
uint32_t wmi_ready_extract_init_status(void *wmi_hdl, void *ev)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->ready_extract_init_status)
		return wmi_handle->ops->ready_extract_init_status(wmi_handle,
			ev);


	return 1;

}

/**
 * wmi_ready_extract_mac_addr() - extract mac address from ready event
 * @wmi_handle: wmi handle
 * @param ev: pointer to event buffer
 * @param macaddr: Pointer to hold MAC address
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_ready_extract_mac_addr(void *wmi_hdl, void *ev, uint8_t *macaddr)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->ready_extract_mac_addr)
		return wmi_handle->ops->ready_extract_mac_addr(wmi_handle,
			ev, macaddr);


	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_ready_extract_mac_addr() - extract MAC address list from ready event
 * @wmi_handle: wmi handle
 * @param ev: pointer to event buffer
 * @param num_mac_addr: Pointer to number of entries
 *
 * Return: address to start of mac addr list
 */
wmi_host_mac_addr *wmi_ready_extract_mac_addr_list(void *wmi_hdl, void *ev,
					      uint8_t *num_mac_addr)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->ready_extract_mac_addr_list)
		return wmi_handle->ops->ready_extract_mac_addr_list(wmi_handle,
			ev, num_mac_addr);

	*num_mac_addr = 0;

	return NULL;
}

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
		void *evt_buf, struct wmi_host_ready_ev_param *ev_param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_ready_event_params)
		return wmi_handle->ops->extract_ready_event_params(wmi_handle,
			evt_buf, ev_param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_dbglog_data_len() - extract debuglog data length
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param len:  length of buffer
 *
 * Return: length
 */
uint8_t *wmi_extract_dbglog_data_len(void *wmi_hdl, void *evt_buf,
			uint32_t *len)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_dbglog_data_len)
		return wmi_handle->ops->extract_dbglog_data_len(wmi_handle,
			evt_buf, len);


	return NULL;
}
qdf_export_symbol(wmi_extract_dbglog_data_len);

/**
 * wmi_send_ext_resource_config() - send extended resource configuration
 * @wmi_handle: wmi handle
 * @param ext_cfg: pointer to extended resource configuration
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_send_ext_resource_config(void *wmi_hdl,
			wmi_host_ext_resource_config *ext_cfg)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_ext_resource_config)
		return wmi_handle->ops->send_ext_resource_config(wmi_handle,
				ext_cfg);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_nf_dbr_dbm_info_get_cmd_send() - WMI request nf info function
 *  @param wmi_handle	  : handle to WMI.
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_nf_dbr_dbm_info_get_cmd_send(void *wmi_hdl)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->send_nf_dbr_dbm_info_get_cmd)
		return wmi->ops->send_nf_dbr_dbm_info_get_cmd(wmi);

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
 *  wmi_unified_rtt_meas_req_test_cmd_send() - WMI rtt meas req test function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold rtt meas req test param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_rtt_meas_req_test_cmd_send(void *wmi_hdl,
				struct rtt_meas_req_test_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_rtt_meas_req_test_cmd)
		return wmi_handle->ops->send_rtt_meas_req_test_cmd(wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_rtt_meas_req_cmd_send() - WMI rtt meas req function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold rtt meas req param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_rtt_meas_req_cmd_send(void *wmi_hdl,
				struct rtt_meas_req_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_rtt_meas_req_cmd)
		return wmi_handle->ops->send_rtt_meas_req_cmd(wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_lci_set_cmd_send() - WMI lci set function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold lci param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_lci_set_cmd_send(void *wmi_hdl,
				struct lci_set_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_lci_set_cmd)
		return wmi_handle->ops->send_lci_set_cmd(wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_lcr_set_cmd_send() - WMI lcr set function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold lcr param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_lcr_set_cmd_send(void *wmi_hdl,
				struct lcr_set_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_lcr_set_cmd)
		return wmi_handle->ops->send_lcr_set_cmd(wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_rtt_keepalive_req_cmd_send() - WMI rtt meas req test function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold rtt meas req test param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_rtt_keepalive_req_cmd_send(void *wmi_hdl,
				struct rtt_keepalive_req_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_rtt_keepalive_req_cmd)
		return wmi_handle->ops->send_rtt_keepalive_req_cmd(wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_send_periodic_chan_stats_config_cmd() - send periodic chan stats cmd
 * to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold periodic chan stats param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_send_periodic_chan_stats_config_cmd(void *wmi_hdl,
			struct periodic_chan_stats_params *param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->send_periodic_chan_stats_config_cmd)
		return wmi->ops->send_periodic_chan_stats_config_cmd(wmi,
					param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_send_atf_peer_request_cmd() - send atf peer request command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to atf peer request param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_send_atf_peer_request_cmd(void *wmi_hdl,
		struct atf_peer_request_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_atf_peer_request_cmd)
		return wmi_handle->ops->send_atf_peer_request_cmd(wmi_handle,
					param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_send_set_atf_grouping_cmd() - send set atf grouping command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to set atf grouping param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_send_set_atf_grouping_cmd(void *wmi_hdl,
		struct atf_grouping_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_atf_grouping_cmd)
		return wmi_handle->ops->send_set_atf_grouping_cmd(wmi_handle,
					param);

	return QDF_STATUS_E_FAILURE;

}

/**
 * wmi_send_get_user_position_cmd() - send get user position command to fw
 * @wmi_handle: wmi handle
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_send_get_user_position_cmd(void *wmi_hdl, uint32_t value)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_get_user_position_cmd)
		return wmi_handle->ops->send_get_user_position_cmd(wmi_handle,
								value);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_send_get_peer_mumimo_tx_count_cmd() - send get mumio tx count
 * 					     command to fw
 * @wmi_handle: wmi handle
 * @value: user pos value
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_send_get_peer_mumimo_tx_count_cmd(void *wmi_hdl, uint32_t value)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->send_get_peer_mumimo_tx_count_cmd)
		return wmi->ops->send_get_peer_mumimo_tx_count_cmd(wmi, value);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_send_reset_peer_mumimo_tx_count_cmd() - send reset peer mumimo
 * 					       tx count to fw
 * @wmi_handle: wmi handle
 * @value:	reset tx count value
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_send_reset_peer_mumimo_tx_count_cmd(void *wmi_hdl, uint32_t value)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->send_reset_peer_mumimo_tx_count_cmd)
		return wmi->ops->send_reset_peer_mumimo_tx_count_cmd(wmi,
								value);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_send_pdev_caldata_version_check_cmd() - send reset peer mumimo
 * 					       tx count to fw
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

/* Extract - APIs */
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
 * wmi_extract_fips_event_data() - extract fips event data
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: pointer to FIPS event param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_fips_event_data(void *wmi_hdl, void *evt_buf,
		struct wmi_host_fips_event_param *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_fips_event_data) {
		return wmi_handle->ops->extract_fips_event_data(wmi_handle,
			evt_buf, param);
	}
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
 * @param idx: Index refering to a vdev
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
 * @param idx: Index refering to a vdev
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
 * wmi_extract_mgmt_rx_params() - extract management rx params from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param hdr: Pointer to hold header
 * @param bufp: Pointer to hold pointer to rx param buffer
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_mgmt_rx_params(void *wmi_hdl, void *evt_buf,
	struct mgmt_rx_event_params *hdr, uint8_t **bufp)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_mgmt_rx_params)
		return wmi_handle->ops->extract_mgmt_rx_params(wmi_handle,
				evt_buf, hdr, bufp);

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
 * wmi_extract_vdev_roam_param() - extract vdev roam param from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold roam param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_vdev_roam_param(void *wmi_hdl, void *evt_buf,
	wmi_host_roam_event *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_vdev_roam_param)
		return wmi_handle->ops->extract_vdev_roam_param(wmi_handle,
				evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_vdev_scan_ev_param() - extract vdev scan param from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold vdev scan param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_vdev_scan_ev_param(void *wmi_hdl, void *evt_buf,
	struct scan_event *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_vdev_scan_ev_param)
		return wmi_handle->ops->extract_vdev_scan_ev_param(wmi_handle,
				evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}

#ifdef CONVERGED_TDLS_ENABLE
QDF_STATUS wmi_extract_vdev_tdls_ev_param(void *wmi_hdl, void *evt_buf,
					  struct tdls_event_info *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->extract_vdev_tdls_ev_param)
		return wmi_handle->ops->extract_vdev_tdls_ev_param(wmi_handle,
				evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}
#endif

/**
 * wmi_extract_mu_ev_param() - extract mu param from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold mu report
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_mu_ev_param(void *wmi_hdl, void *evt_buf,
	wmi_host_mu_report_event *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_mu_ev_param)
		return wmi_handle->ops->extract_mu_ev_param(wmi_handle, evt_buf,
			param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_mu_db_entry() - extract mu db entry from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold mu db entry
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_mu_db_entry(void *wmi_hdl, void *evt_buf,
	uint8_t idx, wmi_host_mu_db_entry *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_mu_db_entry)
		return wmi_handle->ops->extract_mu_db_entry(wmi_handle, evt_buf,
			idx, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_mumimo_tx_count_ev_param() - extract mumimo tx count from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold mumimo tx count
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_mumimo_tx_count_ev_param(void *wmi_hdl, void *evt_buf,
	wmi_host_peer_txmu_cnt_event *param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->extract_mumimo_tx_count_ev_param)
		return wmi->ops->extract_mumimo_tx_count_ev_param(wmi,
			 evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_peer_gid_userpos_list_ev_param() - extract peer userpos list
 * 						  from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold peer gid userposition list
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_peer_gid_userpos_list_ev_param(void *wmi_hdl,
	void *evt_buf,
	wmi_host_peer_gid_userpos_list_event *param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->extract_peer_gid_userpos_list_ev_param)
		return wmi->ops->extract_peer_gid_userpos_list_ev_param(wmi,
			 evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_pdev_caldata_version_check_ev_param() - extract caldata
 * 						       from event
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
 * wmi_extract_gpio_input_ev_param() - extract gpio input param from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param gpio_num: Pointer to hold gpio number
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_gpio_input_ev_param(void *wmi_hdl,
	void *evt_buf, uint32_t *gpio_num)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_gpio_input_ev_param)
		return wmi_handle->ops->extract_gpio_input_ev_param(wmi_handle,
			evt_buf, gpio_num);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_pdev_reserve_ast_ev_param() - extract reserve ast entry
 * param from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold reserve ast entry param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_pdev_reserve_ast_ev_param(void *wmi_hdl,
		void *evt_buf, struct wmi_host_proxy_ast_reserve_param *param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->extract_pdev_reserve_ast_ev_param)
		return wmi->ops->extract_pdev_reserve_ast_ev_param(wmi,
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
 * wmi_extract_pdev_generic_buffer_ev_param() - extract pdev generic buffer
 * from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to generic buffer param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_pdev_generic_buffer_ev_param(void *wmi_hdl,
		void *evt_buf, wmi_host_pdev_generic_buffer_event *param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->extract_pdev_generic_buffer_ev_param)
		return wmi->ops->extract_pdev_generic_buffer_ev_param(wmi,
				evt_buf, param);

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
 * wmi_extract_offchan_data_tx_compl_param() -
 *            extract offchan data tx completion param from event
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

#ifdef CONVERGED_P2P_ENABLE
/**
 * wmi_extract_p2p_lo_stop_ev_param() - extract p2p lo stop param from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @param: Pointer to hold listen offload stop param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_p2p_lo_stop_ev_param(void *wmi_hdl, void *evt_buf,
	struct p2p_lo_event *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (!wmi_handle) {
		WMI_LOGE("wmi handle is null");
		return QDF_STATUS_E_INVAL;
	}

	if (wmi_handle->ops->extract_p2p_lo_stop_ev_param)
		return wmi_handle->ops->extract_p2p_lo_stop_ev_param(
				wmi_handle, evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_p2p_noa_ev_param() - extract p2p noa param from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @param: Pointer to hold p2p noa param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_p2p_noa_ev_param(void *wmi_hdl, void *evt_buf,
	struct p2p_noa_info *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (!wmi_handle) {
		WMI_LOGE("wmi handle is null");
		return QDF_STATUS_E_INVAL;
	}

	if (wmi_handle->ops->extract_p2p_noa_ev_param)
		return wmi_handle->ops->extract_p2p_noa_ev_param(
				wmi_handle, evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}
#endif

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
 * wmi_extract_peer_ratecode_list_ev() - extract peer ratecode from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param peer_mac: Pointer to hold peer mac address
 * @param rate_cap: Pointer to hold ratecode
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_peer_ratecode_list_ev(void *wmi_hdl, void *evt_buf,
	uint8_t *peer_mac, wmi_sa_rate_cap *rate_cap)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->extract_peer_ratecode_list_ev)
		return wmi->ops->extract_peer_ratecode_list_ev(wmi,
			evt_buf, peer_mac, rate_cap);

	return QDF_STATUS_E_FAILURE;

}

/**
 * wmi_extract_comb_phyerr() - extract comb phy error from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param datalen: data length of event buffer
 * @param buf_offset: Pointer to hold value of current event buffer offset
 * post extraction
 * @param phyer: Pointer to hold phyerr
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_comb_phyerr(void *wmi_hdl, void *evt_buf,
	uint16_t datalen, uint16_t *buf_offset, wmi_host_phyerr_t *phyerr)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_comb_phyerr)
		return wmi_handle->ops->extract_comb_phyerr(wmi_handle,
		evt_buf, datalen, buf_offset, phyerr);

	return QDF_STATUS_E_FAILURE;

}

/**
 * wmi_extract_single_phyerr() - extract single phy error from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param datalen: data length of event buffer
 * @param buf_offset: Pointer to hold value of current event buffer offset
 * post extraction
 * @param phyerr: Pointer to hold phyerr
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_single_phyerr(void *wmi_hdl, void *evt_buf,
	uint16_t datalen, uint16_t *buf_offset, wmi_host_phyerr_t *phyerr)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_single_phyerr)
		return wmi_handle->ops->extract_single_phyerr(wmi_handle,
			evt_buf, datalen, buf_offset, phyerr);

	return QDF_STATUS_E_FAILURE;

}

/**
 * wmi_extract_composite_phyerr() - extract composite phy error from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param datalen: Length of event buffer
 * @param phyerr: Pointer to hold phy error
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_composite_phyerr(void *wmi_hdl, void *evt_buf,
	uint16_t datalen, wmi_host_phyerr_t *phyerr)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_composite_phyerr)
		return wmi_handle->ops->extract_composite_phyerr(wmi_handle,
			evt_buf, datalen, phyerr);

	return QDF_STATUS_E_FAILURE;

}

/**
 * wmi_extract_stats_param() - extract all stats count from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param stats_param: Pointer to hold stats count
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_stats_param(void *wmi_hdl, void *evt_buf,
		   wmi_host_stats_event *stats_param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_all_stats_count)
		return wmi_handle->ops->extract_all_stats_count(wmi_handle,
			evt_buf, stats_param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_pdev_stats() - extract pdev stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into pdev stats
 * @param pdev_stats: Pointer to hold pdev stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_pdev_stats(void *wmi_hdl, void *evt_buf,
		 uint32_t index, wmi_host_pdev_stats *pdev_stats)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_pdev_stats)
		return wmi_handle->ops->extract_pdev_stats(wmi_handle,
			evt_buf, index, pdev_stats);

	return QDF_STATUS_E_FAILURE;
}

/**
 * extract_unit_test() - extract unit test from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param unit_test: Pointer to hold unit-test header
 * @param maxspace: The amount of space in evt_buf
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_unit_test(void *wmi_hdl, void *evt_buf,
		wmi_unit_test_event *unit_test, uint32_t maxspace)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_unit_test)
		return wmi_handle->ops->extract_unit_test(wmi_handle,
			evt_buf, unit_test, maxspace);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_pdev_ext_stats() - extract extended pdev stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into extended pdev stats
 * @param pdev_ext_stats: Pointer to hold extended pdev stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_pdev_ext_stats(void *wmi_hdl, void *evt_buf,
		 uint32_t index, wmi_host_pdev_ext_stats *pdev_ext_stats)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_pdev_ext_stats)
		return wmi_handle->ops->extract_pdev_ext_stats(wmi_handle,
			evt_buf, index, pdev_ext_stats);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_peer_stats() - extract peer stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into peer stats
 * @param peer_stats: Pointer to hold peer stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_peer_stats(void *wmi_hdl, void *evt_buf,
		 uint32_t index, wmi_host_peer_stats *peer_stats)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_peer_stats)
		return wmi_handle->ops->extract_peer_stats(wmi_handle,
			evt_buf, index, peer_stats);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_vdev_stats() - extract vdev stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into vdev stats
 * @param vdev_stats: Pointer to hold vdev stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_vdev_stats(void *wmi_hdl, void *evt_buf,
		 uint32_t index, wmi_host_vdev_stats *vdev_stats)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_vdev_stats)
		return wmi_handle->ops->extract_vdev_stats(wmi_handle,
			evt_buf, index, vdev_stats);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_rtt_hdr() - extract rtt header from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param ev: Pointer to hold rtt header
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_rtt_hdr(void *wmi_hdl, void *evt_buf,
	wmi_host_rtt_event_hdr *ev)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_rtt_hdr)
		return wmi_handle->ops->extract_rtt_hdr(wmi_handle,
			evt_buf, ev);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_bcnflt_stats() - extract bcn fault stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into bcn fault stats
 * @param bcnflt_stats: Pointer to hold bcn fault stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_bcnflt_stats(void *wmi_hdl, void *evt_buf,
		 uint32_t index, wmi_host_bcnflt_stats *bcnflt_stats)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_bcnflt_stats)
		return wmi_handle->ops->extract_bcnflt_stats(wmi_handle,
			evt_buf, index, bcnflt_stats);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_rtt_ev() - extract rtt event
 * @wmi_handle: wmi handle
 * @param evt_buf: Pointer to event buffer
 * @param ev: Pointer to hold rtt event
 * @param hdump: Pointer to hold hex dump
 * @param hdump_len: hex dump length
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_rtt_ev(void *wmi_hdl, void *evt_buf,
	wmi_host_rtt_meas_event *ev, uint8_t *hdump, uint16_t hdump_len)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_rtt_ev)
		return wmi_handle->ops->extract_rtt_ev(wmi_handle,
			evt_buf, ev, hdump, hdump_len);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_peer_extd_stats() - extract extended peer stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into extended peer stats
 * @param peer_extd_stats: Pointer to hold extended peer stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_peer_extd_stats(void *wmi_hdl, void *evt_buf,
		 uint32_t index, wmi_host_peer_extd_stats *peer_extd_stats)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_peer_extd_stats)
		return wmi_handle->ops->extract_peer_extd_stats(wmi_handle,
			evt_buf, index, peer_extd_stats);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_rtt_error_report_ev() - extract rtt error report from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param wds_ev: Pointer to hold rtt error report
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_rtt_error_report_ev(void *wmi_hdl, void *evt_buf,
	wmi_host_rtt_error_report_event *ev)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_rtt_error_report_ev)
		return wmi_handle->ops->extract_rtt_error_report_ev(wmi_handle,
			evt_buf, ev);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_chan_stats() - extract chan stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into chan stats
 * @param chanstats: Pointer to hold chan stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_chan_stats(void *wmi_hdl, void *evt_buf,
		 uint32_t index, wmi_host_chan_stats *chan_stats)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_chan_stats)
		return wmi_handle->ops->extract_chan_stats(wmi_handle,
			evt_buf, index, chan_stats);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_thermal_stats() - extract thermal stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: Pointer to event buffer
 * @param temp: Pointer to hold extracted temperature
 * @param level: Pointer to hold extracted level
 * @param pdev_id: Pointer to hold extracted pdev_id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_thermal_stats(void *wmi_hdl, void *evt_buf,
	uint32_t *temp, uint32_t *level, uint32_t *pdev_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_thermal_stats)
		return wmi_handle->ops->extract_thermal_stats(wmi_handle,
			evt_buf, temp, level, pdev_id);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_profile_ctx() - extract profile context from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param profile_ctx: Pointer to hold profile context
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_profile_ctx(void *wmi_hdl, void *evt_buf,
			    wmi_host_wlan_profile_ctx_t *profile_ctx)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_profile_ctx)
		return wmi_handle->ops->extract_profile_ctx(wmi_handle,
			evt_buf, profile_ctx);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_thermal_level_stats() - extract thermal level stats from
 * event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param idx: Index to level stats
 * @param levelcount: Pointer to hold levelcount
 * @param dccount: Pointer to hold dccount
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_thermal_level_stats(void *wmi_hdl, void *evt_buf,
	uint8_t idx, uint32_t *levelcount, uint32_t *dccount)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_thermal_level_stats)
		return wmi_handle->ops->extract_thermal_level_stats(wmi_handle,
			evt_buf, idx, levelcount, dccount);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_profile_data() - extract profile data from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @idx index: index of profile data
 * @param profile_data: Pointer to hold profile data
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_profile_data(void *wmi_hdl, void *evt_buf, uint8_t idx,
			       wmi_host_wlan_profile_t *profile_data)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_profile_data)
		return wmi_handle->ops->extract_profile_data(wmi_handle,
			evt_buf, idx, profile_data);

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
 * wmi_extract_bss_chan_info_event() - extract bss channel information
 * from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param bss_chan_info: Pointer to hold bss channel information
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_bss_chan_info_event(void *wmi_hdl, void *evt_buf,
		    wmi_host_pdev_bss_chan_info_event *bss_chan_info)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_bss_chan_info_event)
		return wmi_handle->ops->extract_bss_chan_info_event(wmi_handle,
		evt_buf, bss_chan_info);

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
 * wmi_extract_tx_data_traffic_ctrl_ev() - extract tx data traffic control
 * from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param ev: Pointer to hold data traffic control
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_tx_data_traffic_ctrl_ev(void *wmi_hdl, void *evt_buf,
			wmi_host_tx_data_traffic_ctrl_event *ev)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->extract_tx_data_traffic_ctrl_ev)
		return wmi->ops->extract_tx_data_traffic_ctrl_ev(wmi,
				evt_buf, ev);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_atf_peer_stats_ev() - extract atf peer stats
 * from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param ev: Pointer to hold atf peer stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_atf_peer_stats_ev(void *wmi_hdl, void *evt_buf,
			wmi_host_atf_peer_stats_event *ev)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->extract_atf_peer_stats_ev)
		return wmi->ops->extract_atf_peer_stats_ev(wmi,
				evt_buf, ev);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_atf_token_info_ev() - extract atf token info
 * from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param idx: Index indicating the peer number
 * @param ev: Pointer to hold atf token info
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_atf_token_info_ev(void *wmi_hdl, void *evt_buf,
			uint8_t idx, wmi_host_atf_peer_stats_info *ev)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->extract_atf_token_info_ev)
		return wmi->ops->extract_atf_token_info_ev(wmi,
				evt_buf, idx, ev);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_vdev_extd_stats() - extract extended vdev stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into extended vdev stats
 * @param vdev_extd_stats: Pointer to hold extended vdev stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_vdev_extd_stats(void *wmi_hdl, void *evt_buf,
		uint32_t index, wmi_host_vdev_extd_stats *vdev_extd_stats)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_vdev_extd_stats)
		return wmi_handle->ops->extract_vdev_extd_stats(wmi_handle,
				evt_buf, index, vdev_extd_stats);
	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_bcn_stats() - extract beacon stats from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @index: Index into beacon stats
 * @vdev_bcn_stats: Pointer to hold beacon stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_bcn_stats(void *wmi_hdl, void *evt_buf,
		uint32_t index, wmi_host_bcn_stats *vdev_bcn_stats)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_bcn_stats)
		return wmi_handle->ops->extract_bcn_stats(wmi_handle,
				evt_buf, index, vdev_bcn_stats);
	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_vdev_nac_rssi_stats() - extract NAC_RSSI stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param vdev_extd_stats: Pointer to hold nac rssi stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_vdev_nac_rssi_stats(void *wmi_hdl, void *evt_buf,
		 struct wmi_host_vdev_nac_rssi_event *vdev_nac_rssi_stats)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_vdev_nac_rssi_stats)
		return wmi_handle->ops->extract_vdev_nac_rssi_stats(wmi_handle,
				evt_buf, vdev_nac_rssi_stats);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_send_adapt_dwelltime_params_cmd() - send wmi cmd of
 * adaptive dwelltime configuration params
 * @wma_handle:  wma handler
 * @dwelltime_params: pointer to dwelltime_params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF failure reason code for failure
 */
QDF_STATUS wmi_unified_send_adapt_dwelltime_params_cmd(void *wmi_hdl,
			struct wmi_adaptive_dwelltime_params *dwelltime_params)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_adapt_dwelltime_params_cmd)
		return wmi_handle->ops->
			send_adapt_dwelltime_params_cmd(wmi_handle,
				  dwelltime_params);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_send_power_dbg_cmd() - send power debug commands
 * @wmi_handle: wmi handle
 * @param: wmi power debug parameter
 *
 * Send WMI_POWER_DEBUG_CMDID parameters to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wmi_unified_send_power_dbg_cmd(void *wmi_hdl,
				struct wmi_power_dbg_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_power_dbg_cmd)
		return wmi_handle->ops->send_power_dbg_cmd(wmi_handle,
				  param);

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

QDF_STATUS wmi_unified_send_sar_limit_cmd(void *wmi_hdl,
				struct sar_limit_cmd_params *params)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_sar_limit_cmd)
		return wmi_handle->ops->send_sar_limit_cmd(
						wmi_handle,
						params);
	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_get_sar_limit_cmd(void *wmi_hdl)
{
	wmi_unified_t wmi_handle = wmi_hdl;

	if (wmi_handle->ops->get_sar_limit_cmd)
		return wmi_handle->ops->get_sar_limit_cmd(wmi_handle);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_extract_sar_limit_event(void *wmi_hdl,
					       uint8_t *evt_buf,
					       struct sar_limit_event *event)
{
	wmi_unified_t wmi_handle = wmi_hdl;

	if (wmi_handle->ops->extract_sar_limit_event)
		return wmi_handle->ops->extract_sar_limit_event(wmi_handle,
								evt_buf,
								event);

	return QDF_STATUS_E_FAILURE;
}


#ifdef WLAN_FEATURE_DISA
QDF_STATUS wmi_unified_encrypt_decrypt_send_cmd(void *wmi_hdl,
				struct disa_encrypt_decrypt_req_params *params)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_encrypt_decrypt_send_cmd)
		return wmi_handle->ops->send_encrypt_decrypt_send_cmd(
						wmi_handle,
						params);
	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_encrypt_decrypt_resp_params(void *wmi_hdl,
			uint8_t *evt_buf,
			struct disa_encrypt_decrypt_resp_params *resp)
{
	struct wmi_unified *wmi_handle = (struct wmi_unified *)wmi_hdl;

	if (wmi_handle->ops->extract_encrypt_decrypt_resp_event)
		return wmi_handle->ops->extract_encrypt_decrypt_resp_event(
				wmi_handle, evt_buf, resp);

	return QDF_STATUS_E_FAILURE;
}

#endif

/*
 * wmi_unified_send_btcoex_wlan_priority_cmd() - send btcoex priority commands
 * @wmi_handle: wmi handle
 * @param :     wmi btcoex cfg params
 *
 * Send WMI_BTCOEX_CFG_CMDID parameters to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wmi_unified_send_btcoex_wlan_priority_cmd(void *wmi_hdl,
				struct btcoex_cfg_params *param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->send_btcoex_wlan_priority_cmd)
		return wmi->ops->send_btcoex_wlan_priority_cmd(wmi,
				  param);

	return QDF_STATUS_E_FAILURE;
}
/**
 *  wmi_unified_send_btcoex_duty_cycle_cmd() - send btcoex duty cycle commands
 * @wmi_handle: wmi handle
 * @param:      wmi btcoex cfg params
 *
 * Send WMI_BTCOEX_CFG_CMDID parameters to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wmi_unified_send_btcoex_duty_cycle_cmd(void *wmi_hdl,
				struct btcoex_cfg_params *param)
{
	wmi_unified_t wmi = (wmi_unified_t) wmi_hdl;

	if (wmi->ops->send_btcoex_duty_cycle_cmd)
		return wmi->ops->send_btcoex_duty_cycle_cmd(wmi,
				param);

	return QDF_STATUS_E_FAILURE;
}

/*
 * wmi_extract_service_ready_ext() - extract extended service ready
 * @wmi_handle: wmi handle
 * @param: wmi power debug parameter
 *
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wmi_extract_service_ready_ext(void *wmi_hdl, uint8_t *evt_buf,
		struct wlan_psoc_host_service_ext_param *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_service_ready_ext)
		return wmi_handle->ops->extract_service_ready_ext(wmi_handle,
				evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_hw_mode_cap_service_ready_ext() -
 *       extract HW mode cap from service ready event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold evt buf
 * @param hw_mode_idx: hw mode idx should be less than num_mode
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_extract_hw_mode_cap_service_ready_ext(
			void *wmi_hdl,
			uint8_t *evt_buf, uint8_t hw_mode_idx,
			struct wlan_psoc_host_hw_mode_caps *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_hw_mode_cap_service_ready_ext)
		return wmi_handle->ops->extract_hw_mode_cap_service_ready_ext(
				wmi_handle,
				evt_buf, hw_mode_idx, param);

	return QDF_STATUS_E_FAILURE;
}
/**
 * wmi_extract_mac_phy_cap_service_ready_ext() -
 *       extract MAC phy cap from service ready event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param hw_mode_id: hw mode id of hw_mode_caps
 * @param phy_id: phy_id within hw_mode_cap
 * @param param: pointer to mac phy caps structure to hold the values from event
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_extract_mac_phy_cap_service_ready_ext(
			void *wmi_hdl,
			uint8_t *evt_buf,
			uint8_t hw_mode_id,
			uint8_t phy_id,
			struct wlan_psoc_host_mac_phy_caps *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_mac_phy_cap_service_ready_ext)
		return wmi_handle->ops->extract_mac_phy_cap_service_ready_ext(
				wmi_handle,
				evt_buf, hw_mode_id, phy_id, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_reg_cap_service_ready_ext() -
 *       extract REG cap from service ready event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold evt buf
 * @param phy_idx: phy idx should be less than num_mode
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_extract_reg_cap_service_ready_ext(
			void *wmi_hdl,
			uint8_t *evt_buf, uint8_t phy_idx,
			struct wlan_psoc_host_hal_reg_capabilities_ext *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_reg_cap_service_ready_ext)
		return wmi_handle->ops->extract_reg_cap_service_ready_ext(
				wmi_handle,
				evt_buf, phy_idx, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_dbr_ring_cap_service_ready_ext(
			void *wmi_hdl,
			uint8_t *evt_buf, uint8_t idx,
			struct wlan_psoc_host_dbr_ring_caps *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_dbr_ring_cap_service_ready_ext)
		return wmi_handle->ops->extract_dbr_ring_cap_service_ready_ext(
				wmi_handle,
				evt_buf, idx, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_dbr_buf_release_fixed(
			void *wmi_hdl,
			uint8_t *evt_buf,
			struct direct_buf_rx_rsp *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_dbr_buf_release_fixed)
		return wmi_handle->ops->extract_dbr_buf_release_fixed(
				wmi_handle,
				evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_dbr_buf_release_entry(
			void *wmi_hdl,
			uint8_t *evt_buf, uint8_t idx,
			struct direct_buf_rx_entry *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_dbr_buf_release_entry)
		return wmi_handle->ops->extract_dbr_buf_release_entry(
				wmi_handle,
				evt_buf, idx, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_pdev_utf_event() -
 *       extract UTF data from pdev utf event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold evt buf
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_extract_pdev_utf_event(void *wmi_hdl,
				      uint8_t *evt_buf,
				      struct wmi_host_pdev_utf_event *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_pdev_utf_event)
		return wmi_handle->ops->extract_pdev_utf_event(
				wmi_handle,
				evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_pdev_qvit_event() -
 *       extract UTF data from pdev qvit event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold evt buf
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_extract_pdev_qvit_event(void *wmi_hdl,
				      uint8_t *evt_buf,
				      struct wmi_host_pdev_qvit_event *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_pdev_qvit_event)
		return wmi_handle->ops->extract_pdev_qvit_event(
				wmi_handle,
				evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_send_coex_ver_cfg_cmd() - send coex ver cfg command
 * @wmi_handle: wmi handle
 * @param:      wmi coex ver cfg params
 *
 * Send WMI_COEX_VERSION_CFG_CMID parameters to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wmi_unified_send_coex_ver_cfg_cmd(void *wmi_hdl,
				coex_ver_cfg_t *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_coex_ver_cfg_cmd)
		return wmi_handle->ops->send_coex_ver_cfg_cmd(wmi_handle,
			param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_send_coex_config_cmd() - send coex ver cfg command
 * @wmi_handle: wmi handle
 * @param:      wmi coex cfg cmd params
 *
 * Send WMI_COEX_CFG_CMD parameters to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wmi_unified_send_coex_config_cmd(void *wmi_hdl,
					    struct coex_config_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_coex_config_cmd)
		return wmi_handle->ops->send_coex_config_cmd(wmi_handle,
			param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_send_request_get_rcpi_cmd(void *wmi_hdl,
					struct rcpi_req *get_rcpi_param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_get_rcpi_cmd)
		return wmi_handle->ops->send_get_rcpi_cmd(wmi_handle,
			   get_rcpi_param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_rcpi_response_event(void *wmi_hdl, void *evt_buf,
					   struct rcpi_res *res)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;
	struct wmi_ops *ops = wmi_handle->ops;

	if (ops->extract_rcpi_response_event)
		return ops->extract_rcpi_response_event(wmi_handle, evt_buf,
							res);

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

QDF_STATUS
wmi_unified_dfs_phyerr_offload_en_cmd(void *wmi_hdl,
				      uint32_t pdev_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_dfs_phyerr_offload_en_cmd)
		return wmi_handle->ops->send_dfs_phyerr_offload_en_cmd(
				wmi_handle, pdev_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_dfs_phyerr_offload_dis_cmd(void *wmi_hdl,
				       uint32_t pdev_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_dfs_phyerr_offload_dis_cmd)
		return wmi_handle->ops->send_dfs_phyerr_offload_dis_cmd(
				wmi_handle, pdev_id);

	return QDF_STATUS_E_FAILURE;
}

/*
 * wmi_extract_chainmask_tables_tlv() - extract chain mask tables
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer.
 * @chainmask_table: pointer to struct wlan_psoc_host_chainmask_table
 *
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wmi_extract_chainmask_tables(void *wmi_hdl, uint8_t *evt_buf,
		struct wlan_psoc_host_chainmask_table *chainmask_table)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_chainmask_tables)
		return wmi_handle->ops->extract_chainmask_tables(wmi_handle,
				evt_buf, chainmask_table);

	return QDF_STATUS_E_FAILURE;
}
/**
 *  wmi_unified_set_country_cmd_send() - WMI set country function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold set country cmd parameter
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_country_cmd_send(void *wmi_hdl,
				struct set_country *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_country_cmd)
		return wmi_handle->ops->send_set_country_cmd(wmi_handle,
				  param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_send_dbs_scan_sel_params_cmd() - send wmi cmd of
 * DBS scan selection configuration params
 * @wma_handle:  wma handler
 * @dbs_scan_params: pointer to wmi_dbs_scan_sel_params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF failure reason code for failure
 */
QDF_STATUS wmi_unified_send_dbs_scan_sel_params_cmd(void *wmi_hdl,
			struct wmi_dbs_scan_sel_params *dbs_scan_params)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_dbs_scan_sel_params_cmd)
		return wmi_handle->ops->
			send_dbs_scan_sel_params_cmd(wmi_handle,
						     dbs_scan_params);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_send_limit_off_chan_cmd() - send wmi cmd of limit off channel
 * configuration params
 * @wmi_hdl:  wmi handler
 * @limit_off_chan_param: pointer to wmi_limit_off_chan_param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF failure reason code on failure
 */
QDF_STATUS wmi_unified_send_limit_off_chan_cmd(void *wmi_hdl,
		struct wmi_limit_off_chan_param *limit_off_chan_param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_limit_off_chan_cmd)
		return wmi_handle->ops->send_limit_off_chan_cmd(wmi_handle,
				limit_off_chan_param);

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
 * wmi_unified_send_wds_entry_list_cmd() - WMI function to get list of
 *  wds entries from FW
 * @wmi_handle: wmi handle
 *
 * Send WMI_PDEV_WDS_ENTRY_LIST_CMDID parameters to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wmi_unified_send_dump_wds_table_cmd(void *wmi_hdl)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_wds_entry_list_cmd)
		return wmi_handle->ops->send_wds_entry_list_cmd(wmi_handle);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_wds_entry - api to extract wds entry
 * @wmi_handle: wma handle
 * @evt_buf: pointer to event buffer
 * @wds_entry: wds entry
 * @idx: index to point wds entry in event buffer
 *
 * Return: QDF_STATUS_SUCCESS for successful event parse
 *         else QDF_STATUS_E_INVAL or QDF_STATUS_E_FAILURE
 */
QDF_STATUS wmi_extract_wds_entry(void *wmi_hdl, uint8_t *evt_buf,
			struct wdsentry *wds_entry,
			u_int32_t idx)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_wds_entry)
		return wmi_handle->ops->extract_wds_entry(wmi_handle,
						evt_buf, wds_entry, idx);

	return QDF_STATUS_E_FAILURE;
}
qdf_export_symbol(wmi_extract_wds_entry);

#ifdef WLAN_FEATURE_NAN_CONVERGENCE
QDF_STATUS wmi_unified_ndp_initiator_req_cmd_send(void *wmi_hdl,
					struct nan_datapath_initiator_req *req)
{
	wmi_unified_t wmi_handle = wmi_hdl;

	if (wmi_handle->ops->send_ndp_initiator_req_cmd)
		return wmi_handle->ops->send_ndp_initiator_req_cmd(wmi_handle,
								    req);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_ndp_responder_req_cmd_send(void *wmi_hdl,
				struct nan_datapath_responder_req *req)
{
	wmi_unified_t wmi_handle = wmi_hdl;

	if (wmi_handle->ops->send_ndp_responder_req_cmd)
		return wmi_handle->ops->send_ndp_responder_req_cmd(wmi_handle,
								    req);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_ndp_end_req_cmd_send(void *wmi_hdl,
					    struct nan_datapath_end_req *req)
{
	wmi_unified_t wmi_handle = wmi_hdl;

	if (wmi_handle->ops->send_ndp_end_req_cmd)
		return wmi_handle->ops->send_ndp_end_req_cmd(wmi_handle,
								    req);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_ndp_initiator_rsp(wmi_unified_t wmi_handle,
			uint8_t *data, struct nan_datapath_initiator_rsp **rsp)
{
	if (wmi_handle->ops->extract_ndp_initiator_rsp)
		return wmi_handle->ops->extract_ndp_initiator_rsp(wmi_handle,
								data, rsp);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_ndp_ind(wmi_unified_t wmi_handle, uint8_t *data,
			       struct nan_datapath_indication_event **ind)
{
	if (wmi_handle->ops->extract_ndp_ind)
		return wmi_handle->ops->extract_ndp_ind(wmi_handle,
							data, ind);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_ndp_confirm(wmi_unified_t wmi_handle, uint8_t *data,
				   struct nan_datapath_confirm_event **ev)
{
	if (wmi_handle->ops->extract_ndp_confirm)
		return wmi_handle->ops->extract_ndp_confirm(wmi_handle,
							    data, ev);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_ndp_responder_rsp(wmi_unified_t wmi_handle,
			uint8_t *data, struct nan_datapath_responder_rsp **rsp)
{
	if (wmi_handle->ops->extract_ndp_responder_rsp)
		return wmi_handle->ops->extract_ndp_responder_rsp(wmi_handle,
								  data, rsp);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_ndp_end_rsp(wmi_unified_t wmi_handle, uint8_t *data,
				   struct nan_datapath_end_rsp_event **rsp)
{
	if (wmi_handle->ops->extract_ndp_end_rsp)
		return wmi_handle->ops->extract_ndp_end_rsp(wmi_handle,
							    data, rsp);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_ndp_end_ind(wmi_unified_t wmi_handle, uint8_t *data,
				struct nan_datapath_end_indication_event **ind)
{
	if (wmi_handle->ops->extract_ndp_end_ind)
		return wmi_handle->ops->extract_ndp_end_ind(wmi_handle,
							    data, ind);

	return QDF_STATUS_E_FAILURE;
}
#endif
QDF_STATUS wmi_unified_send_btm_config(void *wmi_hdl,
				       struct wmi_btm_config *params)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_btm_config)
		return wmi_handle->ops->send_btm_config(wmi_handle,
							params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_send_obss_detection_cfg_cmd(void *wmi_hdl,
		struct wmi_obss_detection_cfg_param *obss_cfg_param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->send_obss_detection_cfg_cmd)
		return wmi_handle->ops->send_obss_detection_cfg_cmd(wmi_handle,
				obss_cfg_param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_extract_obss_detection_info(void *wmi_hdl,
						   uint8_t *data,
						   struct wmi_obss_detect_info
						   *info)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->extract_obss_detection_info)
		return wmi_handle->ops->extract_obss_detection_info(data, info);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_offload_11k_cmd(void *wmi_hdl,
				struct wmi_11k_offload_params *params)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_offload_11k_cmd)
		return wmi_handle->ops->send_offload_11k_cmd(
				wmi_handle, params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_invoke_neighbor_report_cmd(void *wmi_hdl,
			struct wmi_invoke_neighbor_report_params *params)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_invoke_neighbor_report_cmd)
		return wmi_handle->ops->send_invoke_neighbor_report_cmd(
				wmi_handle, params);

	return QDF_STATUS_E_FAILURE;
}

#ifdef WLAN_SUPPORT_GREEN_AP
QDF_STATUS wmi_extract_green_ap_egap_status_info(
		void *wmi_hdl, uint8_t *evt_buf,
		struct wlan_green_ap_egap_status_info *egap_status_info_params)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->extract_green_ap_egap_status_info)
		return wmi_handle->ops->extract_green_ap_egap_status_info(
				evt_buf, egap_status_info_params);

	return QDF_STATUS_E_FAILURE;
}
#endif

QDF_STATUS wmi_unified_send_bss_color_change_enable_cmd(void *wmi_hdl,
							uint32_t vdev_id,
							bool enable)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->send_bss_color_change_enable_cmd)
		return wmi_handle->ops->send_bss_color_change_enable_cmd(
				wmi_handle, vdev_id, enable);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_send_obss_color_collision_cfg_cmd(void *wmi_hdl,
		struct wmi_obss_color_collision_cfg_param *cfg)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->send_obss_color_collision_cfg_cmd)
		return wmi_handle->ops->send_obss_color_collision_cfg_cmd(
				wmi_handle, cfg);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_extract_obss_color_collision_info(void *wmi_hdl,
		uint8_t *data, struct wmi_obss_color_collision_info *info)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->extract_obss_color_collision_info)
		return wmi_handle->ops->extract_obss_color_collision_info(data,
									  info);

	return QDF_STATUS_E_FAILURE;
}
