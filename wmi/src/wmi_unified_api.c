/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
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
#include "qdf_module.h"
#include "cdp_txrx_cmn_struct.h"

static const wmi_host_channel_width mode_to_width[WMI_HOST_MODE_MAX] = {
	[WMI_HOST_MODE_11A]           = WMI_HOST_CHAN_WIDTH_20,
	[WMI_HOST_MODE_11G]           = WMI_HOST_CHAN_WIDTH_20,
	[WMI_HOST_MODE_11B]           = WMI_HOST_CHAN_WIDTH_20,
	[WMI_HOST_MODE_11GONLY]       = WMI_HOST_CHAN_WIDTH_20,
	[WMI_HOST_MODE_11NA_HT20]     = WMI_HOST_CHAN_WIDTH_20,
	[WMI_HOST_MODE_11NG_HT20]     = WMI_HOST_CHAN_WIDTH_20,
	[WMI_HOST_MODE_11AC_VHT20]    = WMI_HOST_CHAN_WIDTH_20,
	[WMI_HOST_MODE_11AC_VHT20_2G] = WMI_HOST_CHAN_WIDTH_20,
	[WMI_HOST_MODE_11NA_HT40]     = WMI_HOST_CHAN_WIDTH_40,
	[WMI_HOST_MODE_11NG_HT40]     = WMI_HOST_CHAN_WIDTH_40,
	[WMI_HOST_MODE_11AC_VHT40]    = WMI_HOST_CHAN_WIDTH_40,
	[WMI_HOST_MODE_11AC_VHT40_2G] = WMI_HOST_CHAN_WIDTH_40,
	[WMI_HOST_MODE_11AC_VHT80]    = WMI_HOST_CHAN_WIDTH_80,
	[WMI_HOST_MODE_11AC_VHT80_2G] = WMI_HOST_CHAN_WIDTH_80,
#if CONFIG_160MHZ_SUPPORT
	[WMI_HOST_MODE_11AC_VHT80_80] = WMI_HOST_CHAN_WIDTH_80P80,
	[WMI_HOST_MODE_11AC_VHT160]   = WMI_HOST_CHAN_WIDTH_160,
#endif

#if SUPPORT_11AX
	[WMI_HOST_MODE_11AX_HE20]     = WMI_HOST_CHAN_WIDTH_20,
	[WMI_HOST_MODE_11AX_HE40]     = WMI_HOST_CHAN_WIDTH_40,
	[WMI_HOST_MODE_11AX_HE80]     = WMI_HOST_CHAN_WIDTH_80,
	[WMI_HOST_MODE_11AX_HE80_80]  = WMI_HOST_CHAN_WIDTH_80P80,
	[WMI_HOST_MODE_11AX_HE160]    = WMI_HOST_CHAN_WIDTH_160,
	[WMI_HOST_MODE_11AX_HE20_2G]  = WMI_HOST_CHAN_WIDTH_20,
	[WMI_HOST_MODE_11AX_HE40_2G]  = WMI_HOST_CHAN_WIDTH_40,
	[WMI_HOST_MODE_11AX_HE80_2G]  = WMI_HOST_CHAN_WIDTH_80,
#endif
};

/**
 * wmi_unified_vdev_create_send() - send VDEV create command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold vdev create parameter
 * @macaddr: vdev mac address
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_vdev_create_send(void *wmi_hdl,
				 uint8_t macaddr[QDF_MAC_ADDR_SIZE],
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

QDF_STATUS
wmi_unified_vdev_nss_chain_params_send(void *wmi_hdl,
				       uint8_t vdev_id,
				       struct vdev_nss_chains *user_cfg)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->send_vdev_nss_chain_params_cmd)
		return wmi_handle->ops->send_vdev_nss_chain_params_cmd(
							wmi_handle,
							vdev_id,
							user_cfg);

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
					 uint8_t peer_addr[QDF_MAC_ADDR_SIZE],
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
				    peer_addr[QDF_MAC_ADDR_SIZE],
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
				uint8_t peer_addr[QDF_MAC_ADDR_SIZE],
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
			     uint8_t bssid[QDF_MAC_ADDR_SIZE],
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
 * @param: ponirt to crash inject parameter structure
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

#ifdef FEATURE_FW_LOG_PARSING
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
#endif

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
 *  wmi_unified_sifs_trigger_send() - WMI vdev sifs trigger parameter function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold sifs trigger parameter
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_sifs_trigger_send(void *wmi_hdl,
					 struct sifs_trigger_param *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->send_vdev_sifs_trigger_cmd)
		return wmi_handle->ops->send_vdev_sifs_trigger_cmd(wmi_handle,
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
QDF_STATUS wmi_unified_stats_request_send(wmi_unified_t wmi_handle,
					  uint8_t macaddr[QDF_MAC_ADDR_SIZE],
					  struct stats_request_params *param)
{
	if (wmi_handle->ops->send_stats_request_cmd)
		return wmi_handle->ops->send_stats_request_cmd(wmi_handle,
				   macaddr, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_packet_log_enable_send(void *wmi_hdl,
			WMI_HOST_PKTLOG_EVENT PKTLOG_EVENT, uint8_t mac_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_packet_log_enable_cmd)
		return wmi_handle->ops->send_packet_log_enable_cmd(wmi_handle,
				  PKTLOG_EVENT, mac_id);

	return QDF_STATUS_E_FAILURE;
}

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
					      uint8_t enb_dsb)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->send_peer_based_pktlog_cmd)
		return wmi_handle->ops->send_peer_based_pktlog_cmd
			(wmi_handle, macaddr, mac_id, enb_dsb);

	return QDF_STATUS_E_FAILURE;
}

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

QDF_STATUS
wmi_unified_send_idle_trigger_monitor(wmi_unified_t wmi_handle, uint8_t val)
{
	if (wmi_handle->ops->send_idle_roam_monitor_cmd)
		return wmi_handle->ops->send_idle_roam_monitor_cmd(wmi_handle,
								   val);

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
 * @end_set_sta_ps_mode_cmd: cmd parameter strcture
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
 * wmi_unified_process_update_edca_param() - update EDCA params
 * @wmi_hdl: wmi handle
 * @vdev_id: vdev id.
 * @mu_edca_param: mu_edca_param.
 * @wmm_vparams: edca parameters
 *
 * This function updates EDCA parameters to the target
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_process_update_edca_param(void *wmi_hdl,
				uint8_t vdev_id, bool mu_edca_param,
				struct wmi_host_wme_vparams wmm_vparams[WMI_MAX_NUM_AC])
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_process_update_edca_param_cmd)
		return wmi_handle->ops->send_process_update_edca_param_cmd(wmi_handle,
					 vdev_id, mu_edca_param, wmm_vparams);

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
				    uint32_t vdev_id, uint8_t *p2p_ie)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_p2p_go_set_beacon_ie_cmd)
		return wmi_handle->ops->send_p2p_go_set_beacon_ie_cmd(wmi_handle,
						 vdev_id, p2p_ie);

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

#ifdef IPA_OFFLOAD
/** wmi_unified_ipa_offload_control_cmd() - ipa offload control parameter
 * @wmi_hdl: wmi handle
 * @ipa_offload: ipa offload control parameter
 *
 * Returns: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failures,
 *          error number otherwise
 */
QDF_STATUS  wmi_unified_ipa_offload_control_cmd(void *wmi_hdl,
		struct ipa_uc_offload_control_params *ipa_offload)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (!wmi_handle)
		return QDF_STATUS_E_FAILURE;

	if (wmi_handle->ops->send_ipa_offload_control_cmd)
		return wmi_handle->ops->send_ipa_offload_control_cmd(wmi_handle,
			    ipa_offload);

	return QDF_STATUS_E_FAILURE;
}
#endif

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

#ifdef WLAN_FEATURE_LINK_LAYER_STATS
QDF_STATUS wmi_unified_process_ll_stats_clear_cmd(wmi_unified_t wmi_handle,
				 const struct ll_stats_clear_params *clear_req)
{
	if (wmi_handle->ops->send_process_ll_stats_clear_cmd)
		return wmi_handle->ops->send_process_ll_stats_clear_cmd(wmi_handle,
									clear_req);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_process_ll_stats_set_cmd(wmi_unified_t wmi_handle,
				 const struct ll_stats_set_params *set_req)
{
	if (wmi_handle->ops->send_process_ll_stats_set_cmd)
		return wmi_handle->ops->send_process_ll_stats_set_cmd(wmi_handle,
								      set_req);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_process_ll_stats_get_cmd(wmi_unified_t wmi_handle,
				 const struct ll_stats_get_params *get_req)
{
	if (wmi_handle->ops->send_process_ll_stats_get_cmd)
		return wmi_handle->ops->send_process_ll_stats_get_cmd(wmi_handle,
								      get_req);

	return QDF_STATUS_E_FAILURE;
}
#endif /* WLAN_FEATURE_LINK_LAYER_STATS */

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
 * @params: get link status params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_link_status_req_cmd(wmi_unified_t wmi_handle,
					   struct link_status_params *params)
{
	if (wmi_handle->ops->send_link_status_req_cmd)
		return wmi_handle->ops->send_link_status_req_cmd(wmi_handle,
								 params);

	return QDF_STATUS_E_FAILURE;
}

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

#if !defined(REMOVE_PKT_LOG) && defined(FEATURE_PKTLOG)
/**
 * wmi_unified_pktlog_wmi_send_cmd() - send pktlog enable/disable command to target
 * @wmi_handle: wmi handle
 * @pktlog_event: pktlog event
 * @cmd_id: pktlog cmd id
 * @user_triggered: user triggered input for PKTLOG enable mode
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
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
#endif /* !REMOVE_PKT_LOG && FEATURE_PKTLOG */

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
 * wmi_unified_cfg_action_frm_tb_ppdu_cmd()-send action frame TB PPDU cfg to FW
 * @wmi_handle:    Pointer to WMi handle
 * @cfg_info:      Pointer to cfg msg
 *
 * This function sends action frame TB PPDU cfg to firmware
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 *
 */
QDF_STATUS
wmi_unified_cfg_action_frm_tb_ppdu_cmd(void *wmi_hdl,
				       struct cfg_action_frm_tb_ppdu_param
				       *cfg_info)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_cfg_action_frm_tb_ppdu_cmd)
		return wmi_handle->ops->send_cfg_action_frm_tb_ppdu_cmd(
						wmi_handle, cfg_info);

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

#ifdef FEATURE_WLAN_APF
QDF_STATUS
wmi_unified_set_active_apf_mode_cmd(wmi_unified_t wmi, uint8_t vdev_id,
				    enum wmi_host_active_apf_mode ucast_mode,
				    enum wmi_host_active_apf_mode
							       mcast_bcast_mode)
{
	if (wmi->ops->send_set_active_apf_mode_cmd)
		return wmi->ops->send_set_active_apf_mode_cmd(wmi, vdev_id,
							      ucast_mode,
							      mcast_bcast_mode);
	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_send_apf_enable_cmd(wmi_unified_t wmi,
				uint32_t vdev_id, bool enable)
{
	if (wmi->ops->send_apf_enable_cmd)
		return wmi->ops->send_apf_enable_cmd(wmi, vdev_id, enable);
	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_send_apf_write_work_memory_cmd(wmi_unified_t wmi,
					   struct wmi_apf_write_memory_params
								  *write_params)
{
	if (wmi->ops->send_apf_write_work_memory_cmd)
		return wmi->ops->send_apf_write_work_memory_cmd(wmi,
								write_params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_send_apf_read_work_memory_cmd(wmi_unified_t wmi,
					  struct wmi_apf_read_memory_params
								   *read_params)
{
	if (wmi->ops->send_apf_read_work_memory_cmd)
		return wmi->ops->send_apf_read_work_memory_cmd(wmi,
							       read_params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_apf_read_memory_resp_event(wmi_unified_t wmi, void *evt_buf,
				struct wmi_apf_read_memory_resp_event_params
								*read_mem_evt)
{
	if (wmi->ops->extract_apf_read_memory_resp_event)
		return wmi->ops->extract_apf_read_memory_resp_event(wmi,
								evt_buf,
								read_mem_evt);

	return QDF_STATUS_E_FAILURE;
}
#endif /* FEATURE_WLAN_APF */

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
				uint8_t macaddr[QDF_MAC_ADDR_SIZE],
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
				uint8_t macaddr[QDF_MAC_ADDR_SIZE],
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
				uint8_t macaddr[QDF_MAC_ADDR_SIZE],
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
				uint8_t macaddr[QDF_MAC_ADDR_SIZE],
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
				uint8_t macaddr[QDF_MAC_ADDR_SIZE],
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
				uint8_t macaddr[QDF_MAC_ADDR_SIZE],
				struct singleamsdu_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_singleamsdu_cmd)
		return wmi_handle->ops->send_singleamsdu_cmd(wmi_handle,
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
		WMI_LOGI("Service %d not supported", service_id);
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
 *					     command to fw
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
 *					       tx count to fw
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

/* Extract - APIs */
/**
 * wmi_extract_ctl_failsafe_check_ev_param() - extract ctl data
 * from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold ctl data
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_ctl_failsafe_check_ev_param(void *wmi_hdl,
					void *evt_buf,
					struct wmi_host_pdev_ctl_failsafe_event
					*param)
{
	wmi_unified_t wmi = (wmi_unified_t)wmi_hdl;

	if (wmi->ops->extract_ctl_failsafe_check_ev_param)
		return wmi->ops->extract_ctl_failsafe_check_ev_param(
			wmi, evt_buf, param);

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
 *                                                from event
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
 * wmi_extract_esp_estimate_ev_param() - extract air time from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @param: Pointer to hold esp event
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_esp_estimate_ev_param(void *wmi_hdl, void *evt_buf,
				  struct esp_estimation_event *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->extract_esp_estimation_ev_param)
		return wmi_handle->ops->extract_esp_estimation_ev_param(
				wmi_handle, evt_buf, param);

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
 * wmi_extract_per_chain_rssi_stats() - extract rssi stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into rssi stats
 * @param rssi_stats: Pointer to hold rssi stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_per_chain_rssi_stats(void *wmi_hdl, void *evt_buf,
	uint32_t index, struct wmi_host_per_chain_rssi_stats *rssi_stats)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_per_chain_rssi_stats)
		return wmi_handle->ops->extract_per_chain_rssi_stats(wmi_handle,
			evt_buf, index, rssi_stats);

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

QDF_STATUS wmi_extract_peer_retry_stats(void *wmi_hdl, void *evt_buf,
	uint32_t index, struct wmi_host_peer_retry_stats *peer_retry_stats)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->extract_peer_retry_stats)
		return wmi_handle->ops->extract_peer_retry_stats(wmi_handle,
			evt_buf, index, peer_retry_stats);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_peer_adv_stats() - extract advance (extd2) peer stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param peer_adv_stats: Pointer to hold extended peer stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_peer_adv_stats(wmi_unified_t wmi_handle, void *evt_buf,
				      struct wmi_host_peer_adv_stats
				      *peer_adv_stats)
{
	if (wmi_handle->ops->extract_peer_adv_stats)
		return wmi_handle->ops->extract_peer_adv_stats(wmi_handle,
			evt_buf, peer_adv_stats);

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
 * wmi_extract_sar_cap_service_ready_ext() -
 *	 extract sar cap from service ready event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @ext_param: extended target info
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_extract_sar_cap_service_ready_ext(
			void *wmi_hdl,
			uint8_t *evt_buf,
			struct wlan_psoc_host_service_ext_param *ext_param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->extract_sar_cap_service_ready_ext)
		return wmi_handle->ops->extract_sar_cap_service_ready_ext(
				wmi_handle,
				evt_buf, ext_param);

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

QDF_STATUS wmi_extract_spectral_scaling_params_service_ready_ext(
			void *wmi_hdl,
			uint8_t *evt_buf, uint8_t idx,
			struct wlan_psoc_host_spectral_scaling_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->extract_scaling_params_service_ready_ext)
		return wmi_handle->ops->extract_scaling_params_service_ready_ext
				(wmi_handle, evt_buf, idx, param);

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

#ifdef QCA_SUPPORT_AGILE_DFS
QDF_STATUS
wmi_unified_send_vdev_adfs_ch_cfg_cmd(void *wmi_hdl,
				      struct vdev_adfs_ch_cfg_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->send_adfs_ch_cfg_cmd)
		return wmi_handle->ops->send_adfs_ch_cfg_cmd(
				wmi_handle,
				param);
	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_send_vdev_adfs_ocac_abort_cmd(void *wmi_hdl,
					  struct vdev_adfs_abort_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->send_adfs_ocac_abort_cmd)
		return wmi_handle->ops->send_adfs_ocac_abort_cmd(
				wmi_handle,
				param);
	return QDF_STATUS_E_FAILURE;
}
#endif

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

#ifdef WLAN_SUPPORT_RF_CHARACTERIZATION
QDF_STATUS wmi_extract_rf_characterization_entries(wmi_unified_t wmi_hdl,
	uint8_t *evt_buf,
	struct wlan_psoc_host_rf_characterization_entry *rf_characterization_entries)
{
	if (wmi_hdl->ops->extract_rf_characterization_entries)
		return wmi_hdl->ops->extract_rf_characterization_entries(wmi_hdl,
					evt_buf, rf_characterization_entries);

	return QDF_STATUS_E_FAILURE;
}
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

#ifdef WLAN_FEATURE_ACTION_OUI
QDF_STATUS
wmi_unified_send_action_oui_cmd(void *wmi_hdl,
				struct action_oui_request *req)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->send_action_oui_cmd)
		return wmi_handle->ops->send_action_oui_cmd(wmi_handle,
							    req);

	return QDF_STATUS_E_FAILURE;
}
#endif

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

wmi_host_channel_width wmi_get_ch_width_from_phy_mode(void *wmi_hdl,
					WMI_HOST_WLAN_PHY_MODE phymode)
{
	/*
	 * this API does translation between host only strcutres, hence
	 * does not need separate TLV, non-TLV definitions
	 */

	if (phymode >= WMI_HOST_MODE_11A && phymode < WMI_HOST_MODE_MAX)
		return mode_to_width[phymode];
	else
		return WMI_HOST_CHAN_WIDTH_20;
}

#ifdef QCA_SUPPORT_CP_STATS
QDF_STATUS wmi_extract_cca_stats(wmi_unified_t wmi_handle, void *evt_buf,
				 struct wmi_host_congestion_stats *stats)
{
	if (wmi_handle->ops->extract_cca_stats)
		return wmi_handle->ops->extract_cca_stats(wmi_handle, evt_buf,
							  stats);

	return QDF_STATUS_E_FAILURE;
}
#endif /* QCA_SUPPORT_CP_STATS */

#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(HOST_DFS_SPOOF_TEST)
QDF_STATUS
wmi_unified_dfs_send_avg_params_cmd(void *wmi_hdl,
				    struct dfs_radar_found_params *params)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->send_dfs_average_radar_params_cmd)
		return wmi_handle->ops->send_dfs_average_radar_params_cmd(
			wmi_handle, params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_dfs_status_from_fw(void *wmi_hdl, void *evt_buf,
					  uint32_t *dfs_status_check)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->extract_dfs_status_from_fw)
		return wmi_handle->ops->extract_dfs_status_from_fw(wmi_handle,
				evt_buf, dfs_status_check);

	return QDF_STATUS_E_FAILURE;
}
#endif

#ifdef OL_ATH_SMART_LOGGING
QDF_STATUS wmi_unified_send_smart_logging_enable_cmd(void *wmi_hdl,
						     uint32_t param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->send_smart_logging_enable_cmd)
		return wmi_handle->ops->send_smart_logging_enable_cmd(
				wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_send_smart_logging_fatal_cmd(void *wmi_hdl,
					 struct wmi_debug_fatal_events *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->send_smart_logging_fatal_cmd)
		return wmi_handle->ops->send_smart_logging_fatal_cmd(wmi_handle,
			param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_smartlog_ev(void *wmi_hdl,
				   void *evt_buf,
				   struct wmi_debug_fatal_events *ev)
{
	wmi_unified_t wmi = (wmi_unified_t)wmi_hdl;

	if (wmi->ops->extract_smartlog_event)
		return wmi->ops->extract_smartlog_event(wmi, evt_buf, ev);

	return QDF_STATUS_E_FAILURE;
}

qdf_export_symbol(wmi_extract_smartlog_ev);
#endif /* OL_ATH_SMART_LOGGING */

QDF_STATUS
wmi_unified_send_roam_scan_stats_cmd(void *wmi_hdl,
				     struct wmi_roam_scan_stats_req *params)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->send_roam_scan_stats_cmd)
		return wmi_handle->ops->send_roam_scan_stats_cmd(wmi_handle,
								 params);

	return QDF_STATUS_E_FAILURE;
}

#ifdef CRYPTO_SET_KEY_CONVERGED
uint8_t wlan_crypto_cipher_to_wmi_cipher(
		enum wlan_crypto_cipher_type crypto_cipher)
{
	switch (crypto_cipher) {
	case WLAN_CRYPTO_CIPHER_NONE:
		return WMI_CIPHER_NONE;
	case WLAN_CRYPTO_CIPHER_WEP:
	case WLAN_CRYPTO_CIPHER_WEP_40:
	case WLAN_CRYPTO_CIPHER_WEP_104:
		return WMI_CIPHER_WEP;
	case WLAN_CRYPTO_CIPHER_TKIP:
		return WMI_CIPHER_TKIP;
	case WLAN_CRYPTO_CIPHER_WAPI_SMS4:
	case WLAN_CRYPTO_CIPHER_WAPI_GCM4:
		return WMI_CIPHER_WAPI;
	case WLAN_CRYPTO_CIPHER_AES_CCM:
	case WLAN_CRYPTO_CIPHER_AES_CCM_256:
		return WMI_CIPHER_AES_CCM;
	case WLAN_CRYPTO_CIPHER_AES_CMAC:
		return WMI_CIPHER_AES_CMAC;
	case WLAN_CRYPTO_CIPHER_AES_GMAC:
	case WLAN_CRYPTO_CIPHER_AES_GMAC_256:
		return WMI_CIPHER_AES_GMAC;
	case WLAN_CRYPTO_CIPHER_AES_GCM:
	case WLAN_CRYPTO_CIPHER_AES_GCM_256:
		return WMI_CIPHER_AES_GCM;
	default:
		return 0;
	}
}

enum cdp_sec_type wlan_crypto_cipher_to_cdp_sec_type(
		enum wlan_crypto_cipher_type crypto_cipher)
{
	switch (crypto_cipher) {
	case WLAN_CRYPTO_CIPHER_NONE:
		return cdp_sec_type_none;
	case WLAN_CRYPTO_CIPHER_WEP:
	case WLAN_CRYPTO_CIPHER_WEP_40:
	case WLAN_CRYPTO_CIPHER_WEP_104:
		return cdp_sec_type_wep104;
	case WLAN_CRYPTO_CIPHER_TKIP:
		return cdp_sec_type_tkip;
	case WLAN_CRYPTO_CIPHER_WAPI_SMS4:
	case WLAN_CRYPTO_CIPHER_WAPI_GCM4:
		return cdp_sec_type_wapi;
	case WLAN_CRYPTO_CIPHER_AES_CCM:
		return cdp_sec_type_aes_ccmp;
	case WLAN_CRYPTO_CIPHER_AES_CCM_256:
		return cdp_sec_type_aes_ccmp_256;
	case WLAN_CRYPTO_CIPHER_AES_GCM:
		return cdp_sec_type_aes_gcmp;
	case WLAN_CRYPTO_CIPHER_AES_GCM_256:
		return cdp_sec_type_aes_gcmp_256;
	default:
		return cdp_sec_type_none;
	}
}
#endif /* CRYPTO_SET_KEY_CONVERGED */

QDF_STATUS
wmi_extract_roam_scan_stats_res_evt(wmi_unified_t wmi, void *evt_buf,
				    uint32_t *vdev_id,
				    struct wmi_roam_scan_stats_res **res_param)
{
	if (wmi->ops->extract_roam_scan_stats_res_evt)
		return wmi->ops->extract_roam_scan_stats_res_evt(wmi,
							evt_buf,
							vdev_id, res_param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_offload_bcn_tx_status_evt(void *wmi_hdl, void *evt_buf,
					uint32_t *vdev_id,
					uint32_t *tx_status) {
	wmi_unified_t wmi = (wmi_unified_t)wmi_hdl;

	if (wmi->ops->extract_offload_bcn_tx_status_evt)
		return wmi->ops->extract_offload_bcn_tx_status_evt(wmi,
								   evt_buf,
								   vdev_id, tx_status);

	return QDF_STATUS_E_FAILURE;
}

#ifdef OBSS_PD
QDF_STATUS
wmi_unified_send_obss_spatial_reuse_set_cmd(void *wmi_hdl,
				    struct wmi_host_obss_spatial_reuse_set_param
				    *obss_spatial_reuse_param)
{
	wmi_unified_t wmi = (wmi_unified_t)wmi_hdl;

	if (wmi->ops->send_obss_spatial_reuse_set)
		return wmi->ops->send_obss_spatial_reuse_set(wmi,
				obss_spatial_reuse_param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_send_obss_spatial_reuse_set_def_thresh_cmd(void *wmi_hdl,
		struct wmi_host_obss_spatial_reuse_set_def_thresh *thresh)
{
	wmi_unified_t wmi = (wmi_unified_t)wmi_hdl;

	if (wmi->ops->send_obss_spatial_reuse_set_def_thresh)
		return wmi->ops->send_obss_spatial_reuse_set_def_thresh(wmi,
									thresh);

	return QDF_STATUS_E_FAILURE;
}
#endif

QDF_STATUS wmi_convert_pdev_id_host_to_target(void *wmi_hdl,
					      uint32_t host_pdev_id,
					      uint32_t *target_pdev_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->convert_pdev_id_host_to_target) {
		*target_pdev_id =
			wmi_handle->ops->convert_pdev_id_host_to_target(
					host_pdev_id);
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_FAILURE;
}

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

#ifdef WLAN_CFR_ENABLE
QDF_STATUS wmi_unified_send_peer_cfr_capture_cmd(void *wmi_hdl,
						 struct peer_cfr_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->send_peer_cfr_capture_cmd)
		return wmi_handle->ops->send_peer_cfr_capture_cmd(wmi_hdl,
								  param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_cfr_peer_tx_event_param() - extract tx event params from event
 */
QDF_STATUS
wmi_extract_cfr_peer_tx_event_param(void *wmi_hdl, void *evt_buf,
				    wmi_cfr_peer_tx_event_param *peer_tx_event)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->extract_cfr_peer_tx_event_param)
		return wmi_handle->ops->extract_cfr_peer_tx_event_param(wmi_hdl,
									evt_buf,
									peer_tx_event);
	return QDF_STATUS_E_FAILURE;
}
#endif /* WLAN_CFR_ENABLE */

QDF_STATUS wmi_unified_send_mws_coex_req_cmd(struct wmi_unified *wmi_handle,
					     uint32_t vdev_id,
					     uint32_t cmd_id)
{

	if (wmi_handle->ops->send_mws_coex_status_req_cmd)
		return wmi_handle->ops->send_mws_coex_status_req_cmd(wmi_handle,
				vdev_id, cmd_id);

	return QDF_STATUS_E_FAILURE;
}
