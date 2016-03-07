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

#include "wmi_unified_non_tlv.h"
#include "wmi_unified_api.h"
#include "wmi.h"
#include "wmi_unified_priv.h"

/**
 * send_vdev_create_cmd_non_tlv() - send VDEV create command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold vdev create parameter
 * @macaddr: vdev mac address
 *
 * Return: 0 for success or error code
 */
CDF_STATUS send_vdev_create_cmd_non_tlv(wmi_unified_t wmi_handle,
				 uint8_t macaddr[IEEE80211_ADDR_LEN],
				 struct vdev_create_params *param)
{
	return 0;
}

/**
 * send_vdev_delete_cmd_non_tlv() - send VDEV delete command to fw
 * @wmi_handle: wmi handle
 * @if_id: vdev id
 *
 * Return: 0 for success or error code
 */
CDF_STATUS send_vdev_delete_cmd_non_tlv(wmi_unified_t wmi_handle,
					  uint8_t if_id)
{
	return 0;
}

/**
 * send_vdev_stop_cmd_non_tlv() - send vdev stop command to fw
 * @wmi: wmi handle
 * @vdev_id: vdev id
 *
 * Return: 0 for success or erro code
 */
CDF_STATUS send_vdev_stop_cmd_non_tlv(wmi_unified_t wmi,
					uint8_t vdev_id)
{
	return 0;
}

/**
 * send_vdev_down_cmd_non_tlv() - send vdev down command to fw
 * @wmi: wmi handle
 * @vdev_id: vdev id
 *
 * Return: 0 for success or error code
 */
CDF_STATUS send_vdev_down_cmd_non_tlv(wmi_unified_t wmi, uint8_t vdev_id)
{
	return 0;
}

/**
 * send_peer_flush_tids_cmd_non_tlv() - flush peer tids packets in fw
 * @wmi: wmi handle
 * @peer_addr: peer mac address
 * @param: pointer to hold peer flush tid parameter
 *
 * Return: 0 for sucess or error code
 */
CDF_STATUS send_peer_flush_tids_cmd_non_tlv(wmi_unified_t wmi,
					 uint8_t peer_addr[IEEE80211_ADDR_LEN],
					 struct peer_flush_params *param)
{
	return 0;
}

/**
 * send_peer_delete_cmd_non_tlv() - send PEER delete command to fw
 * @wmi: wmi handle
 * @peer_addr: peer mac addr
 * @vdev_id: vdev id
 *
 * Return: 0 for success or error code
 */
CDF_STATUS send_peer_delete_cmd_non_tlv(wmi_unified_t wmi,
					    uint8_t
					    peer_addr[IEEE80211_ADDR_LEN],
					    uint8_t vdev_id)
{
	return 0;
}

/**
 * send_peer_param_cmd_non_tlv() - set peer parameter in fw
 * @wma_ctx: wmi handle
 * @peer_addr: peer mac address
 * @param    : pointer to hold peer set parameter
 *
 * Return: 0 for success or error code
 */
CDF_STATUS send_peer_param_cmd_non_tlv(wmi_unified_t wmi,
				uint8_t peer_addr[IEEE80211_ADDR_LEN],
				struct peer_set_params *param)
{
	return 0;
}

/**
 * send_vdev_up_cmd_non_tlv() - send vdev up command in fw
 * @wmi: wmi handle
 * @bssid: bssid
 * @vdev_up_params: pointer to hold vdev up parameter
 *
 * Return: 0 for success or error code
 */
CDF_STATUS send_vdev_up_cmd_non_tlv(wmi_unified_t wmi,
			     uint8_t bssid[IEEE80211_ADDR_LEN],
				 struct vdev_up_params *params)
{
	return 0;
}

/**
 * send_peer_create_cmd_non_tlv() - send peer create command to fw
 * @wmi: wmi handle
 * @peer_addr: peer mac address
 * @peer_type: peer type
 * @vdev_id: vdev id
 *
 * Return: 0 for success or error code
 */
CDF_STATUS send_peer_create_cmd_non_tlv(wmi_unified_t wmi,
					struct peer_create_params *param)
{
	return 0;
}

/**
 * send_green_ap_ps_cmd_non_tlv() - enable green ap powersave command
 * @wmi_handle: wmi handle
 * @value: value
 * @mac_id: mac id to have radio context
 *
 * Return: 0 for success or error code
 */
CDF_STATUS send_green_ap_ps_cmd_non_tlv(wmi_unified_t wmi_handle,
						uint32_t value, uint8_t mac_id)
{
	return 0;
}

/**
 * send_pdev_utf_cmd_non_tlv() - send utf command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to pdev_utf_params
 * @mac_id: mac id to have radio context
 *
 * Return: 0 for success or error code
 */
CDF_STATUS
send_pdev_utf_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct pdev_utf_params *param,
				uint8_t mac_id)
{
	return 0;
}

/**
 * send_pdev_param_cmd_non_tlv() - set pdev parameters
 * @wmi_handle: wmi handle
 * @param: pointer to pdev parameter
 * @mac_id: radio context
 *
 * Return: 0 on success, errno on failure
 */
CDF_STATUS
send_pdev_param_cmd_non_tlv(wmi_unified_t wmi_handle,
			   struct pdev_params *param,
				uint8_t mac_id)
{
	return 0;
}

/**
 *  send_suspend_cmd_non_tlv() - WMI suspend function
 *  @wmi_handle      : handle to WMI.
 *  @param    : pointer to hold suspend parameter
 *  @mac_id: radio context
 *
 *  Return: 0  on success and -ve on failure.
 */
CDF_STATUS send_suspend_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct suspend_params *param,
				uint8_t mac_id)
{

	return 0;
}

/**
 *  send_resume_cmd_non_tlv() - WMI resume function
 *  @param wmi_handle      : handle to WMI.
 *  @mac_id: radio context
 *
 *  Return: 0  on success and -ve on failure.
 */
CDF_STATUS send_resume_cmd_non_tlv(wmi_unified_t wmi_handle,
				uint8_t mac_id)
{
	return 0;
}

/**
 *  send_wow_enable_cmd_non_tlv() - WMI wow enable function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold wow enable parameter
 *  @mac_id: radio context
 *
 *  Return: 0  on success and -ve on failure.
 */
CDF_STATUS send_wow_enable_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct wow_cmd_params *param,
				uint8_t mac_id)
{
	return 0;
}

/**
 *  send_set_ap_ps_param_cmd_non_tlv() - set ap powersave parameters
 *  @wma_ctx: wma context
 *  @peer_addr: peer mac address
 *  @param: pointer to ap_ps parameter structure
 *
 *  Return: 0 for success or error code
 */
CDF_STATUS send_set_ap_ps_param_cmd_non_tlv(wmi_unified_t wmi_handle,
					   uint8_t *peer_addr,
					   struct ap_ps_params *param)
{
	return 0;
}

/**
 *  send_set_sta_ps_param_cmd_non_tlv() - set sta powersave parameters
 *  @wma_ctx: wma context
 *  @peer_addr: peer mac address
 *  @param: pointer to sta_ps parameter structure
 *
 *  Return: 0 for success or error code
 */
CDF_STATUS send_set_sta_ps_param_cmd_non_tlv(wmi_unified_t wmi_handle,
					   struct sta_ps_params *param)
{
	return 0;
}

/**
 *  send_crash_inject_cmd_non_tlv() - inject fw crash
 *  @wma_handle: wma handle
 *  @param: ponirt to crash inject paramter structure
 *
 *  Return: 0 for success or return error
 */
CDF_STATUS send_crash_inject_cmd_non_tlv(wmi_unified_t wmi_handle,
			 struct crash_inject *param)
{
	return 0;
}

/**
 *  send_dbglog_cmd_non_tlv() - set debug log level
 *  @wmi_handle      : handle to WMI.
 *  @param    : pointer to hold dbglog level parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
CDF_STATUS
send_dbglog_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct dbglog_params *dbglog_param)
{
	return 0;
}

/**
 *  send_vdev_set_param_cmd_non_tlv() - WMI vdev set parameter function
 *  @wmi_handle      : handle to WMI.
 *  @macaddr        : MAC address
 *  @param    : pointer to hold vdev set parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
CDF_STATUS send_vdev_set_param_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct vdev_set_params *param)
{
	return 0;
}

/**
 *  send_stats_request_cmd_non_tlv() - WMI request stats function
 *  @wmi_handle      : handle to WMI.
 *  @macaddr        : MAC address
 *  @param    : pointer to hold stats request parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
CDF_STATUS send_stats_request_cmd_non_tlv(wmi_unified_t wmi_handle,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct stats_request_params *param)
{
	return 0;
}

/**
 *  send_packet_log_enable_cmd_non_tlv() - WMI request stats function
 *  @wmi_handle      : handle to WMI.
 *  @macaddr        : MAC address
 *  @param    : pointer to hold stats request parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
CDF_STATUS send_packet_log_enable_cmd_non_tlv(wmi_unified_t wmi_handle,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct packet_enable_params *param)
{
	return 0;
}

/**
 *  send_beacon_send_cmd_non_tlv() - WMI beacon send function
 *  @wmi_handle      : handle to WMI.
 *  @macaddr        : MAC address
 *  @param    : pointer to hold beacon send cmd parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
CDF_STATUS send_beacon_send_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct beacon_params *param)
{
	return 0;
}

/**
 *  send_peer_assoc_cmd_non_tlv() - WMI peer assoc function
 *  @wmi_handle      : handle to WMI.
 *  @macaddr        : MAC address
 *  @param    : pointer to peer assoc parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
CDF_STATUS send_peer_assoc_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct peer_assoc_params *param)
{
	return 0;
}

/**
 *  send_scan_start_cmd_non_tlv() - WMI scan start function
 *  @wmi_handle      : handle to WMI.
 *  @macaddr        : MAC address
 *  @param    : pointer to hold scan start cmd parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
CDF_STATUS send_scan_start_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct scan_start_params *param)
{
	return 0;
}

/**
 *  send_scan_stop_cmd_non_tlv() - WMI scan start function
 *  @wmi_handle      : handle to WMI.
 *  @macaddr        : MAC address
 *  @param    : pointer to hold scan start cmd parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
CDF_STATUS send_scan_stop_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct scan_stop_params *param)
{
	return 0;
}

/**
 *  send_scan_chan_list_cmd_non_tlv() - WMI scan channel list function
 *  @wmi_handle      : handle to WMI.
 *  @macaddr        : MAC address
 *  @param    : pointer to hold scan channel list parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
CDF_STATUS send_scan_chan_list_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct scan_chan_list_params *param)
{
	return 0;
}

struct wmi_ops non_tlv_ops =  {
	.send_vdev_create_cmd = send_vdev_create_cmd_non_tlv,
	.send_vdev_delete_cmd = send_vdev_delete_cmd_non_tlv,
	.send_vdev_down_cmd = send_vdev_down_cmd_non_tlv,
	.send_peer_flush_tids_cmd = send_peer_flush_tids_cmd_non_tlv,
	.send_peer_param_cmd = send_peer_param_cmd_non_tlv,
	.send_vdev_up_cmd = send_vdev_up_cmd_non_tlv,
	.send_peer_create_cmd = send_peer_create_cmd_non_tlv,
	.send_green_ap_ps_cmd = send_green_ap_ps_cmd_non_tlv,
	.send_pdev_utf_cmd = send_pdev_utf_cmd_non_tlv,
	.send_pdev_param_cmd = send_pdev_param_cmd_non_tlv,
	.send_suspend_cmd = send_suspend_cmd_non_tlv,
	.send_resume_cmd = send_resume_cmd_non_tlv,
	.send_wow_enable_cmd = send_wow_enable_cmd_non_tlv,
	.send_set_ap_ps_param_cmd = send_set_ap_ps_param_cmd_non_tlv,
	.send_set_sta_ps_param_cmd = send_set_sta_ps_param_cmd_non_tlv,
	.send_crash_inject_cmd = send_crash_inject_cmd_non_tlv,
	.send_dbglog_cmd = send_dbglog_cmd_non_tlv,
	.send_vdev_set_param_cmd = send_vdev_set_param_cmd_non_tlv,
	.send_stats_request_cmd = send_stats_request_cmd_non_tlv,
	.send_packet_log_enable_cmd = send_packet_log_enable_cmd_non_tlv,
	.send_beacon_send_cmd = send_beacon_send_cmd_non_tlv,
	.send_peer_assoc_cmd = send_peer_assoc_cmd_non_tlv,
	.send_scan_start_cmd = send_scan_start_cmd_non_tlv,
	.send_scan_stop_cmd = send_scan_stop_cmd_non_tlv,
	.send_scan_chan_list_cmd = send_scan_chan_list_cmd_non_tlv

};

/**
 * wmi_get_non_tlv_ops() - gives pointer to wmi tlv ops
 *
 * Return: pointer to wmi tlv ops
 */
struct wmi_ops *wmi_get_non_tlv_ops(void)
{
	return &non_tlv_ops;
}

