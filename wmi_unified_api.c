/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
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
#include "athdefs.h"
#include "osapi_linux.h"
#include "a_types.h"
#include "a_debug.h"
#include "ol_if_athvar.h"
#include "ol_defines.h"
#include "wmi.h"
#include "wmi_unified_priv.h"
#include "wma_api.h"
#include "wmi_unified_param.h"

/**
 * wmi_unified_vdev_create_send() - send VDEV create command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold vdev create parameter
 * @macaddr: vdev mac address
 *
 * Return: 0 for success or error code
 */
CDF_STATUS wmi_unified_vdev_create_send(void *wmi_hdl,
				 uint8_t macaddr[IEEE80211_ADDR_LEN],
				 struct vdev_create_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_vdev_create_cmd)
		return wmi_handle->ops->send_vdev_create_cmd(wmi_handle,
			   macaddr, param);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_vdev_delete_send() - send VDEV delete command to fw
 * @wmi_handle: wmi handle
 * @if_id: vdev id
 *
 * Return: 0 for success or error code
 */
CDF_STATUS wmi_unified_vdev_delete_send(void *wmi_hdl,
					  uint8_t if_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_vdev_delete_cmd)
		return wmi_handle->ops->send_vdev_delete_cmd(wmi_handle,
			   if_id);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_vdev_stop_send() - send vdev stop command to fw
 * @wmi: wmi handle
 * @vdev_id: vdev id
 *
 * Return: 0 for success or erro code
 */
CDF_STATUS wmi_unified_vdev_stop_send(void *wmi_hdl,
					uint8_t vdev_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_vdev_stop_cmd)
		return wmi_handle->ops->send_vdev_stop_cmd(wmi_handle,
			   vdev_id);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_vdev_down_send() - send vdev down command to fw
 * @wmi: wmi handle
 * @vdev_id: vdev id
 *
 * Return: 0 for success or error code
 */
CDF_STATUS wmi_unified_vdev_down_send(void *wmi_hdl, uint8_t vdev_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_vdev_down_cmd)
		return wmi_handle->ops->send_vdev_down_cmd(wmi_handle, vdev_id);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_peer_flush_tids_send() - flush peer tids packets in fw
 * @wmi: wmi handle
 * @peer_addr: peer mac address
 * @param: pointer to hold peer flush tid parameter
 *
 * Return: 0 for sucess or error code
 */
CDF_STATUS wmi_unified_peer_flush_tids_send(void *wmi_hdl,
					 uint8_t peer_addr[IEEE80211_ADDR_LEN],
					 struct peer_flush_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_peer_flush_tids_cmd)
		return wmi_handle->ops->send_peer_flush_tids_cmd(wmi_handle,
				  peer_addr, param);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_peer_delete_send() - send PEER delete command to fw
 * @wmi: wmi handle
 * @peer_addr: peer mac addr
 * @vdev_id: vdev id
 *
 * Return: 0 for success or error code
 */
CDF_STATUS wmi_unified_peer_delete_send(void *wmi_hdl,
				    uint8_t
				    peer_addr[IEEE80211_ADDR_LEN],
				    uint8_t vdev_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_peer_delete_cmd)
		return wmi_handle->ops->send_peer_delete_cmd(wmi_handle,
				  peer_addr, vdev_id);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_set_peer_param() - set peer parameter in fw
 * @wmi_ctx: wmi handle
 * @peer_addr: peer mac address
 * @param    : pointer to hold peer set parameter
 *
 * Return: 0 for success or error code
 */
CDF_STATUS wmi_set_peer_param_send(void *wmi_hdl,
				uint8_t peer_addr[IEEE80211_ADDR_LEN],
				struct peer_set_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_peer_param_cmd)
		return wmi_handle->ops->send_peer_param_cmd(wmi_handle,
				peer_addr, param);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_vdev_up_send() - send vdev up command in fw
 * @wmi: wmi handle
 * @bssid: bssid
 * @vdev_up_params: pointer to hold vdev up parameter
 *
 * Return: 0 for success or error code
 */
CDF_STATUS wmi_unified_vdev_up_send(void *wmi_hdl,
			     uint8_t bssid[IEEE80211_ADDR_LEN],
				 struct vdev_up_params *params)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_vdev_up_cmd)
		return wmi_handle->ops->send_vdev_up_cmd(wmi_handle, bssid,
					params);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_peer_create_send() - send peer create command to fw
 * @wmi: wmi handle
 * @peer_addr: peer mac address
 * @peer_type: peer type
 * @vdev_id: vdev id
 *
 * Return: 0 for success or error code
 */
CDF_STATUS wmi_unified_peer_create_send(void *wmi_hdl,
					struct peer_create_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_peer_create_cmd)
		return wmi_handle->ops->send_peer_create_cmd(wmi_handle, param);

	return CDF_STATUS_E_FAILURE;
}

#ifdef FEATURE_GREEN_AP
/**
 * wmi_unified_green_ap_ps_send() - enable green ap powersave command
 * @wmi_handle: wmi handle
 * @value: value
 * @mac_id: mac id to have radio context
 *
 * Return: 0 for success or error code
 */
CDF_STATUS wmi_unified_green_ap_ps_send(void *wmi_hdl,
						uint32_t value, uint8_t mac_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_green_ap_ps_cmd)
		return wmi_handle->ops->send_green_ap_ps_cmd(wmi_handle, value,
				  mac_id);

	return CDF_STATUS_E_FAILURE;
}
#else
CDF_STATUS wmi_unified_green_ap_ps_send(void *wmi_hdl,
						uint32_t value, uint8_t mac_id)
{
	return 0;
}
#endif /* FEATURE_GREEN_AP */

/**
 * wmi_unified_pdev_utf_cmd() - send utf command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to pdev_utf_params
 * @mac_id: mac id to have radio context
 *
 * Return: 0 for success or error code
 */
CDF_STATUS
wmi_unified_pdev_utf_cmd_send(void *wmi_hdl,
				struct pdev_utf_params *param,
				uint8_t mac_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_pdev_utf_cmd)
		return wmi_handle->ops->send_pdev_utf_cmd(wmi_handle, param,
				  mac_id);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_pdev_set_param() - set pdev parameters
 * @wmi_handle: wmi handle
 * @param: pointer to pdev parameter
 * @mac_id: radio context
 *
 * Return: 0 on success, errno on failure
 */
CDF_STATUS
wmi_unified_pdev_param_send(void *wmi_hdl,
			   struct pdev_params *param,
				uint8_t mac_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_pdev_param_cmd)
		return wmi_handle->ops->send_pdev_param_cmd(wmi_handle, param,
				  mac_id);

	return CDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_suspend_send() - WMI suspend function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold suspend parameter
 *  @mac_id: radio context
 *
 *  Return: 0 on success and -ve on failure.
 */
CDF_STATUS wmi_unified_suspend_send(void *wmi_hdl,
				struct suspend_params *param,
				uint8_t mac_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_suspend_cmd)
		return wmi_handle->ops->send_suspend_cmd(wmi_handle, param,
				  mac_id);

	return CDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_resume_send - WMI resume function
 *  @param wmi_handle      : handle to WMI.
 *  @mac_id: radio context
 *
 *  Return: 0 on success and -ve on failure.
 */
CDF_STATUS wmi_unified_resume_send(void *wmi_hdl,
				uint8_t mac_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_resume_cmd)
		return wmi_handle->ops->send_resume_cmd(wmi_handle,
				  mac_id);

	return CDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_wow_enable_send() - WMI wow enable function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold wow enable parameter
 *  @mac_id: radio context
 *
 *  Return: 0 on success and -ve on failure.
 */
CDF_STATUS wmi_unified_wow_enable_send(void *wmi_hdl,
				struct wow_cmd_params *param,
				uint8_t mac_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_wow_enable_cmd)
		return wmi_handle->ops->send_wow_enable_cmd(wmi_handle, param,
				  mac_id);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_ap_ps_cmd_send() - set ap powersave parameters
 * @wma_ctx: wma context
 * @peer_addr: peer mac address
 * @param: pointer to ap_ps parameter structure
 *
 * Return: 0 for success or error code
 */
CDF_STATUS wmi_unified_ap_ps_cmd_send(void *wmi_hdl,
					   uint8_t *peer_addr,
					   struct ap_ps_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_ap_ps_param_cmd)
		return wmi_handle->ops->send_set_ap_ps_param_cmd(wmi_handle,
				  peer_addr,
				  param);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_sta_ps_cmd_send() - set sta powersave parameters
 * @wma_ctx: wma context
 * @peer_addr: peer mac address
 * @param: pointer to sta_ps parameter structure
 *
 * Return: 0 for success or error code
 */
CDF_STATUS wmi_unified_sta_ps_cmd_send(void *wmi_hdl,
					   struct sta_ps_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_sta_ps_param_cmd)
		return wmi_handle->ops->send_set_sta_ps_param_cmd(wmi_handle,
				  param);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_crash_inject() - inject fw crash
 * @wma_handle: wma handle
 * @param: ponirt to crash inject paramter structure
 *
 * Return: 0 for success or return error
 */
CDF_STATUS wmi_crash_inject(void *wmi_hdl,
			 struct crash_inject *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_crash_inject_cmd)
		return wmi_handle->ops->send_crash_inject_cmd(wmi_handle,
				  param);

	return CDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_dbglog_cmd_send() - set debug log level
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold dbglog level parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
CDF_STATUS
wmi_unified_dbglog_cmd_send(void *wmi_hdl,
				struct dbglog_params *dbglog_param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_dbglog_cmd)
		return wmi_handle->ops->send_dbglog_cmd(wmi_handle,
				  dbglog_param);

	return CDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_vdev_set_param_send() - WMI vdev set parameter function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold vdev set parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
CDF_STATUS wmi_unified_vdev_set_param_send(void *wmi_hdl,
				struct vdev_set_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_vdev_set_param_cmd)
		return wmi_handle->ops->send_vdev_set_param_cmd(wmi_handle,
				  param);

	return CDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_stats_request_send() - WMI request stats function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold stats request parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
CDF_STATUS wmi_unified_stats_request_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct stats_request_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_stats_request_cmd)
		return wmi_handle->ops->send_stats_request_cmd(wmi_handle,
				   macaddr, param);

	return CDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_stats_request_send() - WMI request stats function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold stats request parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
CDF_STATUS wmi_unified_packet_log_enable_send(void *wmi_hdl,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct packet_enable_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_packet_log_enable_cmd)
		return wmi_handle->ops->send_packet_log_enable_cmd(wmi_handle,
				  macaddr, param);

	return CDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_beacon_send_cmd() - WMI beacon send function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold beacon send cmd parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
CDF_STATUS wmi_unified_beacon_send_cmd(void *wmi_hdl,
				struct beacon_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_beacon_send_cmd)
		return wmi_handle->ops->send_beacon_send_cmd(wmi_handle,
				  param);

	return CDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_peer_assoc_send() - WMI peer assoc function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to peer assoc parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
CDF_STATUS wmi_unified_peer_assoc_send(void *wmi_hdl,
				struct peer_assoc_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_peer_assoc_cmd)
		return wmi_handle->ops->send_peer_assoc_cmd(wmi_handle,
				  param);

	return CDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_scan_start_cmd_send() - WMI scan start function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold scan start cmd parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
CDF_STATUS wmi_unified_scan_start_cmd_send(void *wmi_hdl,
				struct scan_start_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_scan_start_cmd)
		return wmi_handle->ops->send_scan_start_cmd(wmi_handle,
				  param);

	return CDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_scan_stop_cmd_send() - WMI scan start function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold scan start cmd parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
CDF_STATUS wmi_unified_scan_stop_cmd_send(void *wmi_hdl,
				struct scan_stop_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_scan_stop_cmd)
		return wmi_handle->ops->send_scan_stop_cmd(wmi_handle,
				  param);

	return CDF_STATUS_E_FAILURE;
}

/**
 *  wmi_unified_scan_chan_list_cmd_send() - WMI scan channel list function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold scan channel list parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
CDF_STATUS wmi_unified_scan_chan_list_cmd_send(void *wmi_hdl,
				struct scan_chan_list_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_scan_chan_list_cmd)
		return wmi_handle->ops->send_scan_chan_list_cmd(wmi_handle,
				  param);

	return CDF_STATUS_E_FAILURE;
}

/**
 *  wmi_mgmt_unified_cmd_send() - management cmd over wmi layer
 *  @wmi_hdl      : handle to WMI.
 *  @param    : pointer to hold mgmt cmd parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
CDF_STATUS wmi_mgmt_unified_cmd_send(void *wmi_hdl,
				struct wmi_mgmt_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_mgmt_cmd)
		return wmi_handle->ops->send_mgmt_cmd(wmi_handle,
				  param);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_modem_power_state() - set modem power state to fw
 * @wmi_hdl: wmi handle
 * @param_value: parameter value
 *
 * Return: 0 for success or error code
 */
CDF_STATUS wmi_unified_modem_power_state(void *wmi_hdl,
		uint32_t param_value)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_modem_power_state_cmd)
		return wmi_handle->ops->send_modem_power_state_cmd(wmi_handle,
				  param_value);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_set_sta_ps_mode() - set sta powersave params in fw
 * @wmi_hdl: wmi handle
 * @vdev_id: vdev id
 * @val: value
 *
 * Return: 0 for success or error code.
 */
CDF_STATUS wmi_unified_set_sta_ps_mode(void *wmi_hdl,
			       uint32_t vdev_id, uint8_t val)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_sta_ps_mode_cmd)
		return wmi_handle->ops->send_set_sta_ps_mode_cmd(wmi_handle,
				  vdev_id, val);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_set_mimops() - set MIMO powersave
 * @wmi_hdl: wmi handle
 * @vdev_id: vdev id
 * @value: value
 *
 * Return: CDF_STATUS_SUCCESS for success or error code.
 */
CDF_STATUS wmi_unified_set_mimops(void *wmi_hdl, uint8_t vdev_id, int value)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_mimops_cmd)
		return wmi_handle->ops->send_set_mimops_cmd(wmi_handle,
				  vdev_id, value);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_set_smps_params() - set smps params
 * @wmi_hdl: wmi handle
 * @vdev_id: vdev id
 * @value: value
 *
 * Return: CDF_STATUS_SUCCESS for success or error code.
 */
CDF_STATUS wmi_unified_set_smps_params(void *wmi_hdl, uint8_t vdev_id,
			       int value)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_smps_params_cmd)
		return wmi_handle->ops->send_set_smps_params_cmd(wmi_handle,
				  vdev_id, value);

	return CDF_STATUS_E_FAILURE;
}


/**
 * wmi_set_p2pgo_oppps_req() - send p2p go opp power save request to fw
 * @wmi_hdl: wmi handle
 * @opps: p2p opp power save parameters
 *
 * Return: none
 */
CDF_STATUS wmi_unified_set_p2pgo_oppps_req(void *wmi_hdl,
		struct p2p_ps_params *oppps)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_p2pgo_oppps_req_cmd)
		return wmi_handle->ops->send_set_p2pgo_oppps_req_cmd(wmi_handle,
				  oppps);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_set_p2pgo_noa_req_cmd() - send p2p go noa request to fw
 * @wmi_hdl: wmi handle
 * @noa: p2p power save parameters
 *
 * Return: none
 */
CDF_STATUS wmi_unified_set_p2pgo_noa_req_cmd(void *wmi_hdl,
			struct p2p_ps_params *noa)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_p2pgo_noa_req_cmd)
		return wmi_handle->ops->send_set_p2pgo_noa_req_cmd(wmi_handle,
				  noa);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_get_temperature() - get pdev temperature req
 * @wmi_hdl: wmi handle
 *
 * Return: CDF_STATUS_SUCCESS for success or error code.
 */
CDF_STATUS wmi_unified_get_temperature(void *wmi_hdl)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_get_temperature_cmd)
		return wmi_handle->ops->send_get_temperature_cmd(wmi_handle);

	return CDF_STATUS_E_FAILURE;
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
 * Return: 0 for success or error code.
 */
CDF_STATUS
wmi_unified_set_sta_uapsd_auto_trig_cmd(void *wmi_hdl,
				struct sta_uapsd_trig_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_sta_uapsd_auto_trig_cmd)
		return wmi_handle->ops->send_set_sta_uapsd_auto_trig_cmd(wmi_handle,
					param);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_ocb_start_timing_advert() - start sending the timing advertisement
 *			   frames on a channel
 * @wmi_handle: pointer to the wmi handle
 * @timing_advert: pointer to the timing advertisement struct
 *
 * Return: 0 on succes
 */
CDF_STATUS wmi_unified_ocb_start_timing_advert(void *wmi_hdl,
	struct ocb_timing_advert_param *timing_advert)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_ocb_start_timing_advert_cmd)
		return wmi_handle->ops->send_ocb_start_timing_advert_cmd(wmi_handle,
				timing_advert);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_ocb_stop_timing_advert() - stop sending the timing advertisement
 *			frames on a channel
 * @wmi_handle: pointer to the wmi handle
 * @timing_advert: pointer to the timing advertisement struct
 *
 * Return: 0 on succes
 */
CDF_STATUS wmi_unified_ocb_stop_timing_advert(void *wmi_hdl,
	struct ocb_timing_advert_param *timing_advert)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_ocb_stop_timing_advert_cmd)
		return wmi_handle->ops->send_ocb_stop_timing_advert_cmd(wmi_handle,
					timing_advert);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_ocb_set_utc_time_cmd() - get ocb tsf timer val
 * @wmi_handle: pointer to the wmi handle
 * @vdev_id: vdev id
 *
 * Return: 0 on succes
 */
CDF_STATUS wmi_unified_ocb_set_utc_time_cmd(void *wmi_hdl,
			struct ocb_utc_param *utc)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_ocb_set_utc_time_cmd)
		return wmi_handle->ops->send_ocb_set_utc_time_cmd(wmi_handle,
				utc);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_ocb_get_tsf_timer() - get ocb tsf timer val
 * @wmi_handle: pointer to the wmi handle
 * @vdev_id: vdev id
 *
 * Return: 0 on succes
 */
CDF_STATUS wmi_unified_ocb_get_tsf_timer(void *wmi_hdl,
			uint8_t vdev_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_ocb_get_tsf_timer_cmd)
		return wmi_handle->ops->send_ocb_get_tsf_timer_cmd(wmi_handle,
					vdev_id);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_dcc_get_stats_cmd() - get the DCC channel stats
 * @wmi_handle: pointer to the wmi handle
 * @get_stats_param: pointer to the dcc stats
 *
 * Return: 0 on succes
 */
CDF_STATUS wmi_unified_dcc_get_stats_cmd(void *wmi_hdl,
			struct dcc_get_stats_param *get_stats_param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_dcc_get_stats_cmd)
		return wmi_handle->ops->send_dcc_get_stats_cmd(wmi_handle,
					get_stats_param);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_dcc_clear_stats() - command to clear the DCC stats
 * @wmi_handle: pointer to the wmi handle
 * @clear_stats_param: parameters to the command
 *
 * Return: 0 on succes
 */
CDF_STATUS wmi_unified_dcc_clear_stats(void *wmi_hdl,
			uint32_t vdev_id, uint32_t dcc_stats_bitmap)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_dcc_clear_stats_cmd)
		return wmi_handle->ops->send_dcc_clear_stats_cmd(wmi_handle,
					vdev_id, dcc_stats_bitmap);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_dcc_update_ndl() - command to update the NDL data
 * @wmi_handle: pointer to the wmi handle
 * @update_ndl_param: pointer to the request parameters
 *
 * Return: 0 on success
 */
CDF_STATUS wmi_unified_dcc_update_ndl(void *wmi_hdl,
			struct dcc_update_ndl_param *update_ndl_param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_dcc_update_ndl_cmd)
		return wmi_handle->ops->send_dcc_update_ndl_cmd(wmi_handle,
					update_ndl_param);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_ocb_set_config() - send the OCB config to the FW
 * @wmi_handle: pointer to the wmi handle
 * @config: the OCB configuration
 *
 * Return: 0 on success
 */
CDF_STATUS wmi_unified_ocb_set_config(void *wmi_hdl,
			struct ocb_config_param *config, uint32_t *ch_mhz)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_ocb_set_config_cmd)
		return wmi_handle->ops->send_ocb_set_config_cmd(wmi_handle,
					config, ch_mhz);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_set_enable_disable_mcc_adaptive_scheduler_cmd() - control mcc scheduler
 * @wmi_handle: wmi handle
 * @mcc_adaptive_scheduler: enable/disable
 *
 * This function enable/disable mcc adaptive scheduler in fw.
 *
 * Return: CDF_STATUS_SUCCESS for sucess or error code
 */
CDF_STATUS wmi_unified_set_enable_disable_mcc_adaptive_scheduler_cmd(
		void *wmi_hdl, uint32_t mcc_adaptive_scheduler)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_enable_disable_mcc_adaptive_scheduler_cmd)
		return wmi_handle->ops->send_set_enable_disable_mcc_adaptive_scheduler_cmd(wmi_handle,
					mcc_adaptive_scheduler);

	return CDF_STATUS_E_FAILURE;
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
 * Return: CDF status
 */
CDF_STATUS wmi_unified_set_mcc_channel_time_latency_cmd(void *wmi_hdl,
	uint32_t mcc_channel_freq, uint32_t mcc_channel_time_latency)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_mcc_channel_time_latency_cmd)
		return wmi_handle->ops->send_set_mcc_channel_time_latency_cmd(wmi_handle,
					mcc_channel_freq,
					mcc_channel_time_latency);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_set_mcc_channel_time_quota_cmd() - set MCC channel time quota
 * @wmi: wmi handle
 * @adapter_1_chan_number: adapter 1 channel number
 * @adapter_1_quota: adapter 1 quota
 * @adapter_2_chan_number: adapter 2 channel number
 *
 * Return: CDF status
 */
CDF_STATUS wmi_unified_set_mcc_channel_time_quota_cmd(void *wmi_hdl,
			 uint32_t adapter_1_chan_freq,
			 uint32_t adapter_1_quota, uint32_t adapter_2_chan_freq)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_mcc_channel_time_quota_cmd)
		return wmi_handle->ops->send_set_mcc_channel_time_quota_cmd(wmi_handle,
						adapter_1_chan_freq,
						adapter_1_quota,
						adapter_2_chan_freq);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_set_thermal_mgmt_cmd() - set thermal mgmt command to fw
 * @wmi_handle: Pointer to wmi handle
 * @thermal_info: Thermal command information
 *
 * This function sends the thermal management command
 * to the firmware
 *
 * Return: CDF_STATUS_SUCCESS for success otherwise failure
 */
CDF_STATUS wmi_unified_set_thermal_mgmt_cmd(void *wmi_hdl,
				struct thermal_cmd_params *thermal_info)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_thermal_mgmt_cmd)
		return wmi_handle->ops->send_set_thermal_mgmt_cmd(wmi_handle,
					thermal_info);

	return CDF_STATUS_E_FAILURE;
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
 * Return: CDF_STATUS_SUCCESS for success otherwise failure
 */
CDF_STATUS wmi_unified_lro_config_cmd(void *wmi_hdl,
	 struct wmi_lro_config_cmd_t *wmi_lro_cmd)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_lro_config_cmd)
		return wmi_handle->ops->send_lro_config_cmd(wmi_handle,
					wmi_lro_cmd);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_bcn_buf_ll_cmd() - prepare and send beacon buffer to fw for LL
 * @wmi_hdl: wmi handle
 * @param: bcn ll cmd parameter
 *
 * Return: CDF_STATUS_SUCCESS for success otherwise failure
 */
CDF_STATUS wmi_unified_bcn_buf_ll_cmd(void *wmi_hdl,
			wmi_bcn_send_from_host_cmd_fixed_param *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_bcn_buf_ll_cmd)
		return wmi_handle->ops->send_bcn_buf_ll_cmd(wmi_handle,
						param);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_set_sta_sa_query_param_cmd() - set sta sa query parameters
 * @wmi_hdl: wmi handle
 * @vdev_id: vdev id
 * @max_retries: max retries
 * @retry_interval: retry interval
 * This function sets sta query related parameters in fw.
 *
 * Return: CDF_STATUS_SUCCESS for success otherwise failure
 */

CDF_STATUS wmi_unified_set_sta_sa_query_param_cmd(void *wmi_hdl,
					uint8_t vdev_id, uint32_t max_retries,
					uint32_t retry_interval)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_sta_sa_query_param_cmd)
		return wmi_handle->ops->send_set_sta_sa_query_param_cmd(wmi_handle,
						vdev_id, max_retries,
						retry_interval);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_set_sta_keep_alive_cmd() - set sta keep alive parameters
 * @wmi_hdl: wmi handle
 * @params: sta keep alive parameter
 *
 * This function sets keep alive related parameters in fw.
 *
 * Return: none
 */
CDF_STATUS wmi_unified_set_sta_keep_alive_cmd(void *wmi_hdl,
				struct sta_params *params)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_sta_keep_alive_cmd)
		return wmi_handle->ops->send_set_sta_keep_alive_cmd(wmi_handle,
						params);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_vdev_set_gtx_cfg_cmd() - set GTX params
 * @wmi_hdl: wmi handle
 * @if_id: vdev id
 * @gtx_info: GTX config params
 *
 * This function set GTX related params in firmware.
 *
 * Return: 0 for success or error code
 */
CDF_STATUS wmi_unified_vdev_set_gtx_cfg_cmd(void *wmi_hdl, uint32_t if_id,
			struct wmi_gtx_config *gtx_info)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_vdev_set_gtx_cfg_cmd)
		return wmi_handle->ops->send_vdev_set_gtx_cfg_cmd(wmi_handle,
					if_id, gtx_info);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_process_update_edca_param() - update EDCA params
 * @wmi_hdl: wmi handle
 * @edca_params: edca parameters
 *
 * This function updates EDCA parameters to the target
 *
 * Return: CDF Status
 */
CDF_STATUS wmi_unified_process_update_edca_param(void *wmi_hdl,
				uint8_t vdev_id,
				wmi_wmm_vparams gwmm_param[WMI_MAX_NUM_AC])
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_process_update_edca_param_cmd)
		return wmi_handle->ops->send_process_update_edca_param_cmd(wmi_handle,
					 vdev_id, gwmm_param);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_probe_rsp_tmpl_send_cmd() - send probe response template to fw
 * @wmi_hdl: wmi handle
 * @vdev_id: vdev id
 * @probe_rsp_info: probe response info
 *
 * Return: 0 for success or error code
 */
CDF_STATUS wmi_unified_probe_rsp_tmpl_send_cmd(void *wmi_hdl,
				uint8_t vdev_id,
				struct wmi_probe_resp_params *probe_rsp_info,
				uint8_t *frm)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_probe_rsp_tmpl_send_cmd)
		return wmi_handle->ops->send_probe_rsp_tmpl_send_cmd(wmi_handle,
						 vdev_id, probe_rsp_info,
						 frm);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_p2p_go_set_beacon_ie_cmd() - set beacon IE for p2p go
 * @wma_handle: wma handle
 * @vdev_id: vdev id
 * @p2p_ie: p2p IE
 *
 * Return: 0 for success or error code
 */
CDF_STATUS wmi_unified_p2p_go_set_beacon_ie_cmd(void *wmi_hdl,
				    A_UINT32 vdev_id, uint8_t *p2p_ie)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_p2p_go_set_beacon_ie_cmd)
		return wmi_handle->ops->send_p2p_go_set_beacon_ie_cmd(wmi_handle,
						 vdev_id, p2p_ie);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_set_gateway_params_cmd() - set gateway parameters
 * @wmi_hdl: wmi handle
 * @req: gateway parameter update request structure
 *
 * This function reads the incoming @req and fill in the destination
 * WMI structure and sends down the gateway configs down to the firmware
 *
 * Return: CDF_STATUS
 */
CDF_STATUS wmi_unified_set_gateway_params_cmd(void *wmi_hdl,
					struct gateway_update_req_param *req)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_gateway_params_cmd)
		return wmi_handle->ops->send_set_gateway_params_cmd(wmi_handle,
				  req);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_set_rssi_monitoring_cmd() - set rssi monitoring
 * @wmi_hdl: wmi handle
 * @req: rssi monitoring request structure
 *
 * This function reads the incoming @req and fill in the destination
 * WMI structure and send down the rssi monitoring configs down to the firmware
 *
 * Return: 0 on success; error number otherwise
 */
CDF_STATUS wmi_unified_set_rssi_monitoring_cmd(void *wmi_hdl,
					struct rssi_monitor_param *req)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_pno_start_cmd)
		return wmi_handle->ops->send_set_rssi_monitoring_cmd(wmi_handle,
			    req);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_scan_probe_setoui_cmd() - set scan probe OUI
 * @wmi_hdl: wmi handle
 * @psetoui: OUI parameters
 *
 * set scan probe OUI parameters in firmware
 *
 * Return: CDF status
 */
CDF_STATUS wmi_unified_scan_probe_setoui_cmd(void *wmi_hdl,
			  struct scan_mac_oui *psetoui)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_scan_probe_setoui_cmd)
		return wmi_handle->ops->send_scan_probe_setoui_cmd(wmi_handle,
			    psetoui);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_reset_passpoint_network_list_cmd() - reset passpoint network list
 * @wmi_hdl: wmi handle
 * @req: passpoint network request structure
 *
 * This function sends down WMI command with network id set to wildcard id.
 * firmware shall clear all the config entries
 *
 * Return: CDF_STATUS enumeration
 */
CDF_STATUS wmi_unified_reset_passpoint_network_list_cmd(void *wmi_hdl,
					struct wifi_passpoint_req_param *req)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_reset_passpoint_network_list_cmd)
		return wmi_handle->ops->send_reset_passpoint_network_list_cmd(wmi_handle,
			    req);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_set_passpoint_network_list_cmd() - set passpoint network list
 * @wmi_hdl: wmi handle
 * @req: passpoint network request structure
 *
 * This function reads the incoming @req and fill in the destination
 * WMI structure and send down the passpoint configs down to the firmware
 *
 * Return: CDF_STATUS enumeration
 */
CDF_STATUS wmi_unified_set_passpoint_network_list_cmd(void *wmi_hdl,
					struct wifi_passpoint_req_param *req)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_passpoint_network_list_cmd)
		return wmi_handle->ops->send_set_passpoint_network_list_cmd(wmi_handle,
			    req);

	return CDF_STATUS_E_FAILURE;
}

/** wmi_unified_set_epno_network_list_cmd() - set epno network list
 * @wmi_hdl: wmi handle
 * @req: epno config params request structure
 *
 * This function reads the incoming epno config request structure
 * and constructs the WMI message to the firmware.
 *
 * Returns: 0 on success, error number otherwise
 */
CDF_STATUS wmi_unified_set_epno_network_list_cmd(void *wmi_hdl,
		struct wifi_enhanched_pno_params *req)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_epno_network_list_cmd)
		return wmi_handle->ops->send_set_epno_network_list_cmd(wmi_handle,
			    req);

	return CDF_STATUS_E_FAILURE;
}

/** wmi_unified_ipa_offload_control_cmd() - ipa offload control parameter
 * @wmi_hdl: wmi handle
 * @ipa_offload: ipa offload control parameter
 *
 * Returns: 0 on success, error number otherwise
 */
CDF_STATUS  wmi_unified_ipa_offload_control_cmd(void *wmi_hdl,
		struct ipa_offload_control_params *ipa_offload)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_ipa_offload_control_cmd)
		return wmi_handle->ops->send_ipa_offload_control_cmd(wmi_handle,
			    ipa_offload);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_extscan_get_capabilities_cmd() - extscan get capabilities
 * @wmi_hdl: wmi handle
 * @pgetcapab: get capabilities params
 *
 * This function send request to fw to get extscan capabilities.
 *
 * Return: CDF status
 */
CDF_STATUS wmi_unified_extscan_get_capabilities_cmd(void *wmi_hdl,
			  struct extscan_capabilities_params *pgetcapab)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_extscan_get_capabilities_cmd)
		return wmi_handle->ops->send_extscan_get_capabilities_cmd(wmi_handle,
			    pgetcapab);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_extscan_get_cached_results_cmd() - extscan get cached results
 * @wmi_hdl: wmi handle
 * @pcached_results: cached results parameters
 *
 * This function send request to fw to get cached results.
 *
 * Return: CDF status
 */
CDF_STATUS wmi_unified_extscan_get_cached_results_cmd(void *wmi_hdl,
			  struct extscan_cached_result_params *pcached_results)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_extscan_get_cached_results_cmd)
		return wmi_handle->ops->send_extscan_get_cached_results_cmd(wmi_handle,
			    pcached_results);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_extscan_stop_change_monitor_cmd() - send stop change monitor cmd
 * @wmi_hdl: wmi handle
 * @reset_req: Reset change request params
 *
 * This function sends stop change monitor request to fw.
 *
 * Return: CDF status
 */
CDF_STATUS wmi_unified_extscan_stop_change_monitor_cmd(void *wmi_hdl,
			  struct extscan_capabilities_reset_params *reset_req)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_extscan_stop_change_monitor_cmd)
		return wmi_handle->ops->send_extscan_stop_change_monitor_cmd(wmi_handle,
			    reset_req);

	return CDF_STATUS_E_FAILURE;
}



/**
 * wmi_unified_extscan_start_change_monitor_cmd() - start change monitor cmd
 * @wmi_hdl: wmi handle
 * @psigchange: change monitor request params
 *
 * This function sends start change monitor request to fw.
 *
 * Return: CDF status
 */
CDF_STATUS wmi_unified_extscan_start_change_monitor_cmd(void *wmi_hdl,
				   struct extscan_set_sig_changereq_params *
				   psigchange)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_extscan_start_change_monitor_cmd)
		return wmi_handle->ops->send_extscan_start_change_monitor_cmd(wmi_handle,
			    psigchange);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_extscan_stop_hotlist_monitor_cmd() - stop hotlist monitor
 * @wmi_hdl: wmi handle
 * @photlist_reset: hotlist reset params
 *
 * This function configures hotlist monitor to stop in fw.
 *
 * Return: CDF status
 */
CDF_STATUS wmi_unified_extscan_stop_hotlist_monitor_cmd(void *wmi_hdl,
		  struct extscan_bssid_hotlist_reset_params *photlist_reset)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_extscan_stop_hotlist_monitor_cmd)
		return wmi_handle->ops->send_extscan_stop_hotlist_monitor_cmd(wmi_handle,
			    photlist_reset);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_stop_extscan_cmd() - stop extscan command to fw.
 * @wmi_hdl: wmi handle
 * @pstopcmd: stop scan command request params
 *
 * This function sends stop extscan request to fw.
 *
 * Return: CDF Status.
 */
CDF_STATUS wmi_unified_stop_extscan_cmd(void *wmi_hdl,
			  struct extscan_stop_req_params *pstopcmd)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_stop_extscan_cmd)
		return wmi_handle->ops->send_stop_extscan_cmd(wmi_handle,
			    pstopcmd);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_start_extscan_cmd() - start extscan command to fw.
 * @wmi_hdl: wmi handle
 * @pstart: scan command request params
 *
 * This function sends start extscan request to fw.
 *
 * Return: CDF Status.
 */
CDF_STATUS wmi_unified_start_extscan_cmd(void *wmi_hdl,
			  struct wifi_scan_cmd_req_params *pstart)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_start_extscan_cmd)
		return wmi_handle->ops->send_start_extscan_cmd(wmi_handle,
			    pstart);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_plm_stop_cmd() - plm stop request
 * @wmi_hdl: wmi handle
 * @plm: plm request parameters
 *
 * This function request FW to stop PLM.
 *
 * Return: CDF status
 */
CDF_STATUS wmi_unified_plm_stop_cmd(void *wmi_hdl,
			  const struct plm_req_params *plm)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_plm_stop_cmd)
		return wmi_handle->ops->send_plm_stop_cmd(wmi_handle,
			    plm);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_plm_start_cmd() - plm start request
 * @wmi_hdl: wmi handle
 * @plm: plm request parameters
 *
 * This function request FW to start PLM.
 *
 * Return: CDF status
 */
CDF_STATUS wmi_unified_plm_start_cmd(void *wmi_hdl,
			  const struct plm_req_params *plm,
			  uint32_t *gchannel_list)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_plm_start_cmd)
		return wmi_handle->ops->send_plm_start_cmd(wmi_handle,
			    plm, gchannel_list);

	return CDF_STATUS_E_FAILURE;
}

/**
 * send_pno_stop_cmd() - PNO stop request
 * @wmi_hdl: wmi handle
 * @vdev_id: vdev id
 *
 * This function request FW to stop ongoing PNO operation.
 *
 * Return: CDF status
 */
CDF_STATUS wmi_unified_pno_stop_cmd(void *wmi_hdl, uint8_t vdev_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_pno_stop_cmd)
		return wmi_handle->ops->send_pno_stop_cmd(wmi_handle,
			    vdev_id);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_pno_start_cmd() - PNO start request
 * @wmi_hdl: wmi handle
 * @pno: PNO request
 * @gchannel_freq_list: channel frequency list
 *
 * This function request FW to start PNO request.
 * Request: CDF status
 */
CDF_STATUS wmi_unified_pno_start_cmd(void *wmi_hdl,
		   struct pno_scan_req_params *pno,
		   uint32_t *gchannel_freq_list)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_pno_start_cmd)
		return wmi_handle->ops->send_pno_start_cmd(wmi_handle,
			    pno, gchannel_freq_list);

	return CDF_STATUS_E_FAILURE;
}

/* wmi_unified_set_ric_req_cmd() - set ric request element
 * @wmi_hdl: wmi handle
 * @msg: message
 * @is_add_ts: is addts required
 *
 * This function sets ric request element for 11r roaming.
 *
 * Return: none
 */
CDF_STATUS wmi_unified_set_ric_req_cmd(void *wmi_hdl, void *msg,
		uint8_t is_add_ts)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_ric_req_cmd)
		return wmi_handle->ops->send_set_ric_req_cmd(wmi_handle, msg,
			    is_add_ts);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_process_ll_stats_clear_cmd() - clear link layer stats
 * @wmi_hdl: wmi handle
 * @clear_req: ll stats clear request command params
 * @addr: mac address
 *
 * Return: CDF_STATUS_SUCCESS for success or error code
 */
CDF_STATUS wmi_unified_process_ll_stats_clear_cmd(void *wmi_hdl,
	 const struct ll_stats_clear_params *clear_req,
	 uint8_t addr[IEEE80211_ADDR_LEN])
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_process_ll_stats_clear_cmd)
		return wmi_handle->ops->send_process_ll_stats_clear_cmd(wmi_handle,
			   clear_req,  addr);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_process_ll_stats_get_cmd() - link layer stats get request
 * @wmi_hdl:wmi handle
 * @get_req:ll stats get request command params
 *
 * Return: CDF_STATUS_SUCCESS for success or error code
 */
CDF_STATUS wmi_unified_process_ll_stats_get_cmd(void *wmi_hdl,
		 const struct ll_stats_get_params  *get_req,
		 uint8_t addr[IEEE80211_ADDR_LEN])
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_process_ll_stats_get_cmd)
		return wmi_handle->ops->send_process_ll_stats_get_cmd(wmi_handle,
			   get_req,  addr);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_get_stats_cmd() - get stats request
 * @wmi_hdl: wma handle
 * @get_stats_param: stats params
 * @addr: mac address
 *
 * Return: none
 */
CDF_STATUS wmi_unified_get_stats_cmd(void *wmi_hdl,
		       struct pe_stats_req  *get_stats_param,
			   uint8_t addr[IEEE80211_ADDR_LEN])
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_get_stats_cmd)
		return wmi_handle->ops->send_get_stats_cmd(wmi_handle,
			   get_stats_param,  addr);

	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_process_ll_stats_set_cmd() - link layer stats set request
 * @wmi_handle:       wmi handle
 * @set_req:  ll stats set request command params
 *
 * Return: CDF_STATUS_SUCCESS for success or error code
 */
CDF_STATUS wmi_unified_process_ll_stats_set_cmd(void *wmi_hdl,
		const struct ll_stats_set_params *set_req)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_process_ll_stats_set_cmd)
		return wmi_handle->ops->send_process_ll_stats_set_cmd(wmi_handle,
			   set_req);

	return CDF_STATUS_E_FAILURE;
}
