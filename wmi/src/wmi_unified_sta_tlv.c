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

#include <osdep.h>
#include "wmi.h"
#include "wmi_version.h"
#include "wmi_unified_priv.h"
#include "wmi_unified_sta_param.h"
#include "wmi_unified_sta_api.h"
#ifdef FEATURE_WLAN_TDLS
#include <wlan_tdls_public_structs.h>
#endif

/**
 * send_set_sta_sa_query_param_cmd_tlv() - set sta sa query parameters
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @max_retries: max retries
 * @retry_interval: retry interval
 * This function sets sta query related parameters in fw.
 *
 * Return: QDF_STATUS_SUCCESS for success otherwise failure
 */
static QDF_STATUS send_set_sta_sa_query_param_cmd_tlv(wmi_unified_t wmi_handle,
						      uint8_t vdev_id,
						      uint32_t max_retries,
						      uint32_t retry_interval)
{
	wmi_buf_t buf;
	WMI_PMF_OFFLOAD_SET_SA_QUERY_CMD_fixed_param *cmd;
	int len;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (WMI_PMF_OFFLOAD_SET_SA_QUERY_CMD_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_WMI_PMF_OFFLOAD_SET_SA_QUERY_CMD_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
			(WMI_PMF_OFFLOAD_SET_SA_QUERY_CMD_fixed_param));

	cmd->vdev_id = vdev_id;
	cmd->sa_query_max_retry_count = max_retries;
	cmd->sa_query_retry_interval = retry_interval;

	wmi_debug("STA sa query: vdev_id:%d interval:%u retry count:%d",
		 vdev_id, retry_interval, max_retries);

	wmi_mtrace(WMI_PMF_OFFLOAD_SET_SA_QUERY_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_PMF_OFFLOAD_SET_SA_QUERY_CMDID)) {
		wmi_err("Failed to offload STA SA Query");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	wmi_debug("Exit");
	return 0;
}

/**
 * send_set_sta_keep_alive_cmd_tlv() - set sta keep alive parameters
 * @wmi_handle: wmi handle
 * @params: sta keep alive parameter
 *
 * This function sets keep alive related parameters in fw.
 *
 * Return: CDF status
 */
static QDF_STATUS
send_set_sta_keep_alive_cmd_tlv(wmi_unified_t wmi_handle,
				struct sta_keep_alive_params *params)
{
	wmi_buf_t buf;
	WMI_STA_KEEPALIVE_CMD_fixed_param *cmd;
	WMI_STA_KEEPALVE_ARP_RESPONSE *arp_rsp;
	uint8_t *buf_ptr;
	int len;
	QDF_STATUS ret;

	wmi_debug("Enter");

	len = sizeof(*cmd) + sizeof(*arp_rsp);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (WMI_STA_KEEPALIVE_CMD_fixed_param *) wmi_buf_data(buf);
	buf_ptr = (uint8_t *) cmd;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_WMI_STA_KEEPALIVE_CMD_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (WMI_STA_KEEPALIVE_CMD_fixed_param));
	cmd->interval = params->timeperiod;
	cmd->enable = (params->timeperiod) ? 1 : 0;
	cmd->vdev_id = params->vdev_id;
	wmi_debug("Keep Alive: vdev_id:%d interval:%u method:%d",
		 params->vdev_id, params->timeperiod, params->method);
	arp_rsp = (WMI_STA_KEEPALVE_ARP_RESPONSE *) (buf_ptr + sizeof(*cmd));
	WMITLV_SET_HDR(&arp_rsp->tlv_header,
		       WMITLV_TAG_STRUC_WMI_STA_KEEPALVE_ARP_RESPONSE,
		       WMITLV_GET_STRUCT_TLVLEN(WMI_STA_KEEPALVE_ARP_RESPONSE));

	if ((params->method == WMI_KEEP_ALIVE_UNSOLICIT_ARP_RSP) ||
	    (params->method ==
	     WMI_STA_KEEPALIVE_METHOD_GRATUITOUS_ARP_REQUEST)) {
		cmd->method = params->method;
		qdf_mem_copy(&arp_rsp->sender_prot_addr, params->hostv4addr,
			     QDF_IPV4_ADDR_SIZE);
		qdf_mem_copy(&arp_rsp->target_prot_addr, params->destv4addr,
			     QDF_IPV4_ADDR_SIZE);
		WMI_CHAR_ARRAY_TO_MAC_ADDR(params->destmac,
					   &arp_rsp->dest_mac_addr);
	} else {
		cmd->method = WMI_STA_KEEPALIVE_METHOD_NULL_FRAME;
	}

	wmi_mtrace(WMI_STA_KEEPALIVE_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_STA_KEEPALIVE_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to set KeepAlive");
		wmi_buf_free(buf);
	}

	wmi_debug("Exit");
	return ret;
}

/**
 * send_vdev_set_gtx_cfg_cmd_tlv() - set GTX params
 * @wmi_handle: wmi handle
 * @if_id: vdev id
 * @gtx_info: GTX config params
 *
 * This function set GTX related params in firmware.
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_vdev_set_gtx_cfg_cmd_tlv(wmi_unified_t wmi_handle, uint32_t if_id,
				  struct wmi_gtx_config *gtx_info)
{
	wmi_vdev_set_gtx_params_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS ret;
	int len = sizeof(wmi_vdev_set_gtx_params_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_vdev_set_gtx_params_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_set_gtx_params_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_vdev_set_gtx_params_cmd_fixed_param));
	cmd->vdev_id = if_id;

	cmd->gtxRTMask[0] = gtx_info->gtx_rt_mask[0];
	cmd->gtxRTMask[1] = gtx_info->gtx_rt_mask[1];
	cmd->userGtxMask = gtx_info->gtx_usrcfg;
	cmd->gtxPERThreshold = gtx_info->gtx_threshold;
	cmd->gtxPERMargin = gtx_info->gtx_margin;
	cmd->gtxTPCstep = gtx_info->gtx_tpcstep;
	cmd->gtxTPCMin = gtx_info->gtx_tpcmin;
	cmd->gtxBWMask = gtx_info->gtx_bwmask;

	wmi_debug("Setting vdev%d GTX values:htmcs 0x%x, vhtmcs 0x%x, usermask 0x%x, \
		 gtxPERThreshold %d, gtxPERMargin %d, gtxTPCstep %d, gtxTPCMin %d, \
		 gtxBWMask 0x%x.", if_id, cmd->gtxRTMask[0], cmd->gtxRTMask[1],
		 cmd->userGtxMask, cmd->gtxPERThreshold, cmd->gtxPERMargin,
		 cmd->gtxTPCstep, cmd->gtxTPCMin, cmd->gtxBWMask);

	wmi_mtrace(WMI_VDEV_SET_GTX_PARAMS_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_VDEV_SET_GTX_PARAMS_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to set GTX PARAMS");
		wmi_buf_free(buf);
	}
	return ret;
}

/**
 * send_process_dhcp_ind_cmd_tlv() - process dhcp indication from SME
 * @wmi_handle: wmi handle
 * @ta_dhcp_ind: DHCP indication parameter
 *
 * Return: CDF Status
 */
static QDF_STATUS send_process_dhcp_ind_cmd_tlv(wmi_unified_t wmi_handle,
				wmi_peer_set_param_cmd_fixed_param *ta_dhcp_ind)
{
	QDF_STATUS status;
	wmi_buf_t buf = NULL;
	uint8_t *buf_ptr;
	wmi_peer_set_param_cmd_fixed_param *peer_set_param_fp;
	int len = sizeof(wmi_peer_set_param_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	peer_set_param_fp = (wmi_peer_set_param_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&peer_set_param_fp->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_set_param_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_peer_set_param_cmd_fixed_param));

	/* fill in values */
	peer_set_param_fp->vdev_id = ta_dhcp_ind->vdev_id;
	peer_set_param_fp->param_id = ta_dhcp_ind->param_id;
	peer_set_param_fp->param_value = ta_dhcp_ind->param_value;
	qdf_mem_copy(&peer_set_param_fp->peer_macaddr,
		     &ta_dhcp_ind->peer_macaddr,
		     sizeof(ta_dhcp_ind->peer_macaddr));

	wmi_mtrace(WMI_PEER_SET_PARAM_CMDID, NO_SESSION, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_PEER_SET_PARAM_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_err("wmi_unified_cmd_send WMI_PEER_SET_PARAM_CMD"
			 " returned Error %d", status);
		wmi_buf_free(buf);
	}

	return status;
}

/**
 * send_get_link_speed_cmd_tlv() -send command to get linkspeed
 * @wmi_handle: wmi handle
 * @pLinkSpeed: link speed info
 *
 * Return: CDF status
 */
static QDF_STATUS send_get_link_speed_cmd_tlv(wmi_unified_t wmi_handle,
					      wmi_mac_addr peer_macaddr)
{
	wmi_peer_get_estimated_linkspeed_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint32_t len;
	uint8_t *buf_ptr;

	len = sizeof(wmi_peer_get_estimated_linkspeed_cmd_fixed_param);
	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		return QDF_STATUS_E_NOMEM;
	}
	buf_ptr = (uint8_t *) wmi_buf_data(wmi_buf);

	cmd = (wmi_peer_get_estimated_linkspeed_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
	       WMITLV_TAG_STRUC_wmi_peer_get_estimated_linkspeed_cmd_fixed_param,
	       WMITLV_GET_STRUCT_TLVLEN
			(wmi_peer_get_estimated_linkspeed_cmd_fixed_param));

	/* Copy the peer macaddress to the wma buffer */
	qdf_mem_copy(&cmd->peer_macaddr,
		     &peer_macaddr,
		     sizeof(peer_macaddr));

	wmi_mtrace(WMI_PEER_GET_ESTIMATED_LINKSPEED_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
				 WMI_PEER_GET_ESTIMATED_LINKSPEED_CMDID)) {
		wmi_err("Failed to send link speed command");
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * send_fw_profiling_cmd_tlv() - send FW profiling cmd to WLAN FW
 * @wmi_handl: wmi handle
 * @cmd: Profiling command index
 * @value1: parameter1 value
 * @value2: parameter2 value
 *
 * Return: QDF_STATUS_SUCCESS for success else error code
 */
static QDF_STATUS send_fw_profiling_cmd_tlv(wmi_unified_t wmi_handle,
			uint32_t cmd, uint32_t value1, uint32_t value2)
{
	wmi_buf_t buf;
	int32_t len = 0;
	int ret;
	wmi_wlan_profile_trigger_cmd_fixed_param *prof_trig_cmd;
	wmi_wlan_profile_set_hist_intvl_cmd_fixed_param *hist_intvl_cmd;
	wmi_wlan_profile_enable_profile_id_cmd_fixed_param *profile_enable_cmd;
	wmi_wlan_profile_get_prof_data_cmd_fixed_param *profile_getdata_cmd;

	switch (cmd) {
	case WMI_WLAN_PROFILE_TRIGGER_CMDID:
		len = sizeof(wmi_wlan_profile_trigger_cmd_fixed_param);
		buf = wmi_buf_alloc(wmi_handle, len);
		if (!buf) {
			return QDF_STATUS_E_NOMEM;
		}
		prof_trig_cmd =
			(wmi_wlan_profile_trigger_cmd_fixed_param *)
				wmi_buf_data(buf);
		WMITLV_SET_HDR(&prof_trig_cmd->tlv_header,
		     WMITLV_TAG_STRUC_wmi_wlan_profile_trigger_cmd_fixed_param,
		     WMITLV_GET_STRUCT_TLVLEN
				(wmi_wlan_profile_trigger_cmd_fixed_param));
		prof_trig_cmd->enable = value1;
		wmi_mtrace(WMI_WLAN_PROFILE_TRIGGER_CMDID, NO_SESSION, 0);
		ret = wmi_unified_cmd_send(wmi_handle, buf, len,
					   WMI_WLAN_PROFILE_TRIGGER_CMDID);
		if (ret) {
			wmi_err("PROFILE_TRIGGER cmd Failed with value %d",
				 value1);
			wmi_buf_free(buf);
			return ret;
		}
		break;

	case WMI_WLAN_PROFILE_GET_PROFILE_DATA_CMDID:
		len = sizeof(wmi_wlan_profile_get_prof_data_cmd_fixed_param);
		buf = wmi_buf_alloc(wmi_handle, len);
		if (!buf) {
			return QDF_STATUS_E_NOMEM;
		}
		profile_getdata_cmd =
			(wmi_wlan_profile_get_prof_data_cmd_fixed_param *)
				wmi_buf_data(buf);
		WMITLV_SET_HDR(&profile_getdata_cmd->tlv_header,
		      WMITLV_TAG_STRUC_wmi_wlan_profile_get_prof_data_cmd_fixed_param,
		      WMITLV_GET_STRUCT_TLVLEN
		      (wmi_wlan_profile_get_prof_data_cmd_fixed_param));
		wmi_mtrace(WMI_WLAN_PROFILE_GET_PROFILE_DATA_CMDID,
			   NO_SESSION, 0);
		ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				WMI_WLAN_PROFILE_GET_PROFILE_DATA_CMDID);
		if (ret) {
			wmi_err("PROFILE_DATA cmd Failed for id %d value %d",
				 value1, value2);
			wmi_buf_free(buf);
			return ret;
		}
		break;

	case WMI_WLAN_PROFILE_SET_HIST_INTVL_CMDID:
		len = sizeof(wmi_wlan_profile_set_hist_intvl_cmd_fixed_param);
		buf = wmi_buf_alloc(wmi_handle, len);
		if (!buf) {
			return QDF_STATUS_E_NOMEM;
		}
		hist_intvl_cmd =
			(wmi_wlan_profile_set_hist_intvl_cmd_fixed_param *)
				wmi_buf_data(buf);
		WMITLV_SET_HDR(&hist_intvl_cmd->tlv_header,
		      WMITLV_TAG_STRUC_wmi_wlan_profile_set_hist_intvl_cmd_fixed_param,
		      WMITLV_GET_STRUCT_TLVLEN
		      (wmi_wlan_profile_set_hist_intvl_cmd_fixed_param));
		hist_intvl_cmd->profile_id = value1;
		hist_intvl_cmd->value = value2;
		wmi_mtrace(WMI_WLAN_PROFILE_SET_HIST_INTVL_CMDID,
			   NO_SESSION, 0);
		ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				WMI_WLAN_PROFILE_SET_HIST_INTVL_CMDID);
		if (ret) {
			wmi_err("HIST_INTVL cmd Failed for id %d value %d",
				 value1, value2);
			wmi_buf_free(buf);
			return ret;
		}
		break;

	case WMI_WLAN_PROFILE_ENABLE_PROFILE_ID_CMDID:
		len =
		sizeof(wmi_wlan_profile_enable_profile_id_cmd_fixed_param);
		buf = wmi_buf_alloc(wmi_handle, len);
		if (!buf) {
			return QDF_STATUS_E_NOMEM;
		}
		profile_enable_cmd =
			(wmi_wlan_profile_enable_profile_id_cmd_fixed_param *)
				wmi_buf_data(buf);
		WMITLV_SET_HDR(&profile_enable_cmd->tlv_header,
		      WMITLV_TAG_STRUC_wmi_wlan_profile_enable_profile_id_cmd_fixed_param,
		      WMITLV_GET_STRUCT_TLVLEN
		      (wmi_wlan_profile_enable_profile_id_cmd_fixed_param));
		profile_enable_cmd->profile_id = value1;
		profile_enable_cmd->enable = value2;
		wmi_mtrace(WMI_WLAN_PROFILE_ENABLE_PROFILE_ID_CMDID,
			   NO_SESSION, 0);
		ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				WMI_WLAN_PROFILE_ENABLE_PROFILE_ID_CMDID);
		if (ret) {
			wmi_err("enable cmd Failed for id %d value %d",
				 value1, value2);
			wmi_buf_free(buf);
			return ret;
		}
		break;

	default:
		wmi_debug("Invalid profiling command: %u", cmd);
		break;
	}

	return 0;
}

/**
 * send_nat_keepalive_en_cmd_tlv() - enable NAT keepalive filter
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_nat_keepalive_en_cmd_tlv(wmi_unified_t wmi_handle, uint8_t vdev_id)
{
	WMI_VDEV_IPSEC_NATKEEPALIVE_FILTER_CMD_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	wmi_debug("vdev_id: %d", vdev_id);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (WMI_VDEV_IPSEC_NATKEEPALIVE_FILTER_CMD_fixed_param *)
		wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
	WMITLV_TAG_STRUC_WMI_VDEV_IPSEC_NATKEEPALIVE_FILTER_CMD_fixed_param,
		  WMITLV_GET_STRUCT_TLVLEN
		  (WMI_VDEV_IPSEC_NATKEEPALIVE_FILTER_CMD_fixed_param));
	cmd->vdev_id = vdev_id;
	cmd->action = IPSEC_NATKEEPALIVE_FILTER_ENABLE;
	wmi_mtrace(WMI_VDEV_IPSEC_NATKEEPALIVE_FILTER_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_VDEV_IPSEC_NATKEEPALIVE_FILTER_CMDID)) {
		wmi_err("Failed to send NAT keepalive enable command");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return 0;
}

static QDF_STATUS send_wlm_latency_level_cmd_tlv(wmi_unified_t wmi_handle,
				struct wlm_latency_level_param *params)
{
	wmi_wlm_config_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint32_t len = sizeof(*cmd);
	static uint32_t ll[4] = {100, 60, 40, 20};

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_wlm_config_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_wlm_config_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_wlm_config_cmd_fixed_param));
	cmd->vdev_id = params->vdev_id;
	cmd->latency_level = params->wlm_latency_level;
	cmd->ul_latency = ll[params->wlm_latency_level];
	cmd->dl_latency = ll[params->wlm_latency_level];
	cmd->flags = params->wlm_latency_flags;
	wmi_mtrace(WMI_WLM_CONFIG_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_WLM_CONFIG_CMDID)) {
		wmi_err("Failed to send setting latency config command");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return 0;
}

#ifdef FEATURE_WLAN_TDLS
/**
 * tdls_get_wmi_offchannel_mode - Get WMI tdls off channel mode
 * @tdls_sw_mode: tdls_sw_mode
 *
 * This function returns wmi tdls offchannel mode
 *
 * Return: enum value of wmi tdls offchannel mode
 */
static uint8_t tdls_get_wmi_offchannel_mode(uint8_t tdls_sw_mode)
{
	uint8_t off_chan_mode;

	switch (tdls_sw_mode) {
	case ENABLE_CHANSWITCH:
		off_chan_mode = WMI_TDLS_ENABLE_OFFCHANNEL;
		break;

	case DISABLE_CHANSWITCH:
		off_chan_mode = WMI_TDLS_DISABLE_OFFCHANNEL;
		break;

	default:
		wmi_debug("unknown tdls_sw_mode: %d", tdls_sw_mode);
		off_chan_mode = WMI_TDLS_DISABLE_OFFCHANNEL;
	}
	return off_chan_mode;
}

/**
 * tdls_get_wmi_offchannel_bw - Get WMI tdls off channel Bandwidth
 * @tdls_sw_mode: tdls_sw_mode
 *
 * This function returns wmi tdls offchannel bandwidth
 *
 * Return: TDLS offchannel bandwidth
 */
static uint8_t tdls_get_wmi_offchannel_bw(uint16_t tdls_off_ch_bw_offset)
{
	uint8_t off_chan_bw;

	switch (tdls_off_ch_bw_offset) {
	case BW20:
		off_chan_bw = WMI_TDLS_OFFCHAN_20MHZ;
		break;
	case BW40_LOW_PRIMARY:
	case BW40_HIGH_PRIMARY:
		off_chan_bw = WMI_TDLS_OFFCHAN_40MHZ;
		break;
	case BW80:
		off_chan_bw = WMI_TDLS_OFFCHAN_80MHZ;
		break;
	case BWALL:
		off_chan_bw = WMI_TDLS_OFFCHAN_160MHZ;
		break;
	default:
		wmi_debug("unknown tdls offchannel bw offset: %d",
			 tdls_off_ch_bw_offset);
		off_chan_bw = WMI_TDLS_OFFCHAN_20MHZ;
	}
	return off_chan_bw;
}

/**
 * send_set_tdls_offchan_mode_cmd_tlv() - set tdls off channel mode
 * @wmi_handle: wmi handle
 * @chan_switch_params: Pointer to tdls channel switch parameter structure
 *
 * This function sets tdls off channel mode
 *
 * Return: 0 on success; Negative errno otherwise
 */
static QDF_STATUS send_set_tdls_offchan_mode_cmd_tlv(wmi_unified_t wmi_handle,
	      struct tdls_channel_switch_params *chan_switch_params)
{
	wmi_tdls_set_offchan_mode_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	u_int16_t len = sizeof(wmi_tdls_set_offchan_mode_cmd_fixed_param);

	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (wmi_tdls_set_offchan_mode_cmd_fixed_param *)
		wmi_buf_data(wmi_buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_tdls_set_offchan_mode_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			wmi_tdls_set_offchan_mode_cmd_fixed_param));

	WMI_CHAR_ARRAY_TO_MAC_ADDR(chan_switch_params->peer_mac_addr,
				&cmd->peer_macaddr);
	cmd->vdev_id = chan_switch_params->vdev_id;
	cmd->offchan_mode =
		tdls_get_wmi_offchannel_mode(chan_switch_params->tdls_sw_mode);
	cmd->is_peer_responder = chan_switch_params->is_responder;
	cmd->offchan_freq = chan_switch_params->tdls_off_chan_freq;
	cmd->offchan_num = chan_switch_params->tdls_off_ch;
	cmd->offchan_bw_bitmap =
		tdls_get_wmi_offchannel_bw(
			chan_switch_params->tdls_off_ch_bw_offset);
	cmd->offchan_oper_class = chan_switch_params->oper_class;

	wmi_debug("Peer MAC Addr mac_addr31to0: 0x%x, mac_addr47to32: 0x%x",
		 cmd->peer_macaddr.mac_addr31to0,
		 cmd->peer_macaddr.mac_addr47to32);

	wmi_debug("vdev_id: %d, off channel mode: %d, off channel Num: %d, "
		 "off channel frequency: %u off channel offset: 0x%x, "
		 "is_peer_responder: %d, operating class: %d",
		 cmd->vdev_id,
		 cmd->offchan_mode,
		 cmd->offchan_num,
		 cmd->offchan_freq,
		 cmd->offchan_bw_bitmap,
		 cmd->is_peer_responder,
		 cmd->offchan_oper_class);

	wmi_mtrace(WMI_TDLS_SET_OFFCHAN_MODE_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
		WMI_TDLS_SET_OFFCHAN_MODE_CMDID)) {
		wmi_err("failed to send tdls off chan command");
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_update_fw_tdls_state_cmd_tlv() - send enable/disable tdls for a vdev
 * @wmi_handle: wmi handle
 * @pwmaTdlsparams: TDLS params
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_update_fw_tdls_state_cmd_tlv(wmi_unified_t wmi_handle,
				  struct tdls_info *tdls_param,
				  enum wmi_tdls_state tdls_state)
{
	wmi_tdls_set_state_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;

	uint16_t len = sizeof(wmi_tdls_set_state_cmd_fixed_param);

	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (wmi_tdls_set_state_cmd_fixed_param *) wmi_buf_data(wmi_buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		  WMITLV_TAG_STRUC_wmi_tdls_set_state_cmd_fixed_param,
		  WMITLV_GET_STRUCT_TLVLEN
			(wmi_tdls_set_state_cmd_fixed_param));
	cmd->vdev_id = tdls_param->vdev_id;
	cmd->state = (A_UINT32)tdls_state;
	cmd->notification_interval_ms = tdls_param->notification_interval_ms;
	cmd->tx_discovery_threshold = tdls_param->tx_discovery_threshold;
	cmd->tx_teardown_threshold = tdls_param->tx_teardown_threshold;
	cmd->rssi_teardown_threshold = tdls_param->rssi_teardown_threshold;
	cmd->rssi_delta = tdls_param->rssi_delta;
	cmd->tdls_options = tdls_param->tdls_options;
	cmd->tdls_peer_traffic_ind_window = tdls_param->peer_traffic_ind_window;
	cmd->tdls_peer_traffic_response_timeout_ms =
		tdls_param->peer_traffic_response_timeout;
	cmd->tdls_puapsd_mask = tdls_param->puapsd_mask;
	cmd->tdls_puapsd_inactivity_time_ms =
		tdls_param->puapsd_inactivity_time;
	cmd->tdls_puapsd_rx_frame_threshold =
		tdls_param->puapsd_rx_frame_threshold;
	cmd->teardown_notification_ms =
		tdls_param->teardown_notification_ms;
	cmd->tdls_peer_kickout_threshold =
		tdls_param->tdls_peer_kickout_threshold;
	cmd->tdls_discovery_wake_timeout =
		tdls_param->tdls_discovery_wake_timeout;

	wmi_debug("vdev %d tdls_state: %d, state: %d, "
		 "notification_interval_ms: %d, "
		 "tx_discovery_threshold: %d, "
		 "tx_teardown_threshold: %d, "
		 "rssi_teardown_threshold: %d, "
		 "rssi_delta: %d, "
		 "tdls_options: 0x%x, "
		 "tdls_peer_traffic_ind_window: %d, "
		 "tdls_peer_traffic_response_timeout: %d, "
		 "tdls_puapsd_mask: 0x%x, "
		 "tdls_puapsd_inactivity_time: %d, "
		 "tdls_puapsd_rx_frame_threshold: %d, "
		 "teardown_notification_ms: %d, "
		 "tdls_peer_kickout_threshold: %d, "
		 "tdls_discovery_wake_timeout: %d",
		 tdls_param->vdev_id, tdls_state, cmd->state,
		 cmd->notification_interval_ms,
		 cmd->tx_discovery_threshold,
		 cmd->tx_teardown_threshold,
		 cmd->rssi_teardown_threshold,
		 cmd->rssi_delta,
		 cmd->tdls_options,
		 cmd->tdls_peer_traffic_ind_window,
		 cmd->tdls_peer_traffic_response_timeout_ms,
		 cmd->tdls_puapsd_mask,
		 cmd->tdls_puapsd_inactivity_time_ms,
		 cmd->tdls_puapsd_rx_frame_threshold,
		 cmd->teardown_notification_ms,
		 cmd->tdls_peer_kickout_threshold,
		 cmd->tdls_discovery_wake_timeout);

	wmi_mtrace(WMI_TDLS_SET_STATE_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
				 WMI_TDLS_SET_STATE_CMDID)) {
		wmi_err("Failed to send tdls set state command");
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_update_tdls_peer_state_cmd_tlv() - update TDLS peer state
 * @wmi_handle: wmi handle
 * @peer_state: TDLS peer state params
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS
send_update_tdls_peer_state_cmd_tlv(wmi_unified_t wmi_handle,
				    struct tdls_peer_update_state *peer_state,
				    uint32_t *ch_mhz)
{
	struct tdls_peer_params *in_peer_cap;
	struct tdls_ch_params *in_chan_info;
	wmi_tdls_peer_update_cmd_fixed_param *cmd;
	wmi_tdls_peer_capabilities *peer_cap;
	wmi_channel *chan_info;
	wmi_buf_t wmi_buf;
	uint8_t *buf_ptr;
	uint32_t i;
	int32_t len = sizeof(wmi_tdls_peer_update_cmd_fixed_param) +
		      sizeof(wmi_tdls_peer_capabilities);

	in_peer_cap = &peer_state->peer_cap;
	len += WMI_TLV_HDR_SIZE +
	       sizeof(wmi_channel) * in_peer_cap->peer_chanlen;

	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		return QDF_STATUS_E_FAILURE;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(wmi_buf);
	cmd = (wmi_tdls_peer_update_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_tdls_peer_update_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_tdls_peer_update_cmd_fixed_param));

	cmd->vdev_id = peer_state->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(peer_state->peer_macaddr,
				   &cmd->peer_macaddr);

	cmd->peer_state = peer_state->peer_state;

	wmi_debug("vdev_id: %d, peermac: "QDF_MAC_ADDR_FMT", "
		 "peer_macaddr.mac_addr31to0: 0x%x, "
		 "peer_macaddr.mac_addr47to32: 0x%x, peer_state: %d",
		 cmd->vdev_id,
		 QDF_MAC_ADDR_REF(peer_state->peer_macaddr),
		 cmd->peer_macaddr.mac_addr31to0,
		 cmd->peer_macaddr.mac_addr47to32, cmd->peer_state);

	buf_ptr += sizeof(wmi_tdls_peer_update_cmd_fixed_param);
	peer_cap = (wmi_tdls_peer_capabilities *) buf_ptr;
	WMITLV_SET_HDR(&peer_cap->tlv_header,
		       WMITLV_TAG_STRUC_wmi_tdls_peer_capabilities,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_tdls_peer_capabilities));

	if ((in_peer_cap->peer_uapsd_queue & 0x08) >> 3)
		WMI_SET_TDLS_PEER_VO_UAPSD(peer_cap);
	if ((in_peer_cap->peer_uapsd_queue & 0x04) >> 2)
		WMI_SET_TDLS_PEER_VI_UAPSD(peer_cap);
	if ((in_peer_cap->peer_uapsd_queue & 0x02) >> 1)
		WMI_SET_TDLS_PEER_BK_UAPSD(peer_cap);
	if (in_peer_cap->peer_uapsd_queue & 0x01)
		WMI_SET_TDLS_PEER_BE_UAPSD(peer_cap);

	/* Ack and More Data Ack are sent as 0, so no need to set
	 * but fill SP
	 */
	WMI_SET_TDLS_PEER_SP_UAPSD(peer_cap, in_peer_cap->peer_max_sp);

	peer_cap->buff_sta_support = in_peer_cap->peer_buff_sta_support;
	peer_cap->off_chan_support = in_peer_cap->peer_off_chan_support;
	peer_cap->peer_curr_operclass = in_peer_cap->peer_curr_operclass;
	/* self curr operclass is not being used and so pass op class for
	 * preferred off chan in it.
	 */
	peer_cap->self_curr_operclass = in_peer_cap->opclass_for_prefoffchan;
	peer_cap->peer_chan_len = in_peer_cap->peer_chanlen;
	peer_cap->peer_operclass_len = in_peer_cap->peer_oper_classlen;

	wmi_debug("peer_operclass_len: %d", peer_cap->peer_operclass_len);
	for (i = 0; i < WMI_TDLS_MAX_SUPP_OPER_CLASSES; i++) {
		peer_cap->peer_operclass[i] = in_peer_cap->peer_oper_class[i];
	}
	qdf_trace_hex_dump(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_DEBUG,
			   (uint8_t *)peer_cap->peer_operclass,
			   WMI_TDLS_MAX_SUPP_OPER_CLASSES);

	peer_cap->is_peer_responder = in_peer_cap->is_peer_responder;
	peer_cap->pref_offchan_freq = in_peer_cap->pref_offchan_freq;
	peer_cap->pref_offchan_num = in_peer_cap->pref_off_channum;
	peer_cap->pref_offchan_bw = in_peer_cap->pref_off_chan_bandwidth;

	wmi_debug("peer_qos: 0x%x, buff_sta_support: %d, off_chan_support: %d, "
		 "peer_curr_operclass: %d, self_curr_operclass: %d, peer_chan_len: "
		 "%d, peer_operclass_len: %d, is_peer_responder: %d, pref_offchan_num:"
		 " %d, pref_offchan_bw: %d, pref_offchan_freq: %u",
		 peer_cap->peer_qos, peer_cap->buff_sta_support,
		 peer_cap->off_chan_support, peer_cap->peer_curr_operclass,
		 peer_cap->self_curr_operclass, peer_cap->peer_chan_len,
		 peer_cap->peer_operclass_len, peer_cap->is_peer_responder,
		 peer_cap->pref_offchan_num, peer_cap->pref_offchan_bw,
		 peer_cap->pref_offchan_freq);

	/* next fill variable size array of peer chan info */
	buf_ptr += sizeof(wmi_tdls_peer_capabilities);
	WMITLV_SET_HDR(buf_ptr,
		       WMITLV_TAG_ARRAY_STRUC,
		       sizeof(wmi_channel) *
		       in_peer_cap->peer_chanlen);

	chan_info = (wmi_channel *) (buf_ptr + WMI_TLV_HDR_SIZE);
	in_chan_info = in_peer_cap->peer_chan;

	for (i = 0; i < in_peer_cap->peer_chanlen; ++i) {
		WMITLV_SET_HDR(&chan_info->tlv_header,
			       WMITLV_TAG_STRUC_wmi_channel,
			       WMITLV_GET_STRUCT_TLVLEN(wmi_channel));
		chan_info->mhz = ch_mhz[i];
		chan_info->band_center_freq1 = chan_info->mhz;
		chan_info->band_center_freq2 = 0;

		wmi_debug("chan[%d] = %u", i, chan_info->mhz);

		if (in_chan_info->dfs_set) {
			WMI_SET_CHANNEL_FLAG(chan_info, WMI_CHAN_FLAG_PASSIVE);
			wmi_debug("chan[%d] DFS[%d]",
				 in_chan_info->chan_id,
				 in_chan_info->dfs_set);
		}

		if (chan_info->mhz < WMI_2_4_GHZ_MAX_FREQ)
			WMI_SET_CHANNEL_MODE(chan_info, MODE_11G);
		else
			WMI_SET_CHANNEL_MODE(chan_info, MODE_11A);

		WMI_SET_CHANNEL_MAX_TX_POWER(chan_info, in_chan_info->pwr);
		WMI_SET_CHANNEL_REG_POWER(chan_info, in_chan_info->pwr);
		wmi_debug("Channel TX power[%d] = %u: %d", i, chan_info->mhz,
			 in_chan_info->pwr);

		chan_info++;
		in_chan_info++;
	}

	wmi_mtrace(WMI_TDLS_PEER_UPDATE_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
				 WMI_TDLS_PEER_UPDATE_CMDID)) {
		wmi_err("Failed to send tdls peer update state command");
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_vdev_tdls_ev_param_tlv() - extract vdev tdls param from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold vdev tdls param
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_vdev_tdls_ev_param_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, struct tdls_event_info *param)
{
	WMI_TDLS_PEER_EVENTID_param_tlvs *param_buf;
	wmi_tdls_peer_event_fixed_param *evt;

	param_buf = (WMI_TDLS_PEER_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("NULL param_buf");
		return QDF_STATUS_E_NULL_VALUE;
	}

	evt = param_buf->fixed_param;

	qdf_mem_zero(param, sizeof(*param));

	param->vdev_id = evt->vdev_id;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&evt->peer_macaddr,
				   param->peermac.bytes);
	switch (evt->peer_status) {
	case WMI_TDLS_SHOULD_DISCOVER:
		param->message_type = TDLS_SHOULD_DISCOVER;
		break;
	case WMI_TDLS_SHOULD_TEARDOWN:
		param->message_type = TDLS_SHOULD_TEARDOWN;
		break;
	case WMI_TDLS_PEER_DISCONNECTED:
		param->message_type = TDLS_PEER_DISCONNECTED;
		break;
	case WMI_TDLS_CONNECTION_TRACKER_NOTIFICATION:
		param->message_type = TDLS_CONNECTION_TRACKER_NOTIFY;
		break;
	default:
		wmi_err("Discarding unknown tdls event %d from target",
			evt->peer_status);
		return QDF_STATUS_E_INVAL;
	};

	switch (evt->peer_reason) {
	case WMI_TDLS_TEARDOWN_REASON_TX:
		param->peer_reason = TDLS_TEARDOWN_TX;
		break;
	case WMI_TDLS_TEARDOWN_REASON_RSSI:
		param->peer_reason = TDLS_TEARDOWN_RSSI;
		break;
	case WMI_TDLS_TEARDOWN_REASON_SCAN:
		param->peer_reason = TDLS_TEARDOWN_SCAN;
		break;
	case WMI_TDLS_DISCONNECTED_REASON_PEER_DELETE:
		param->peer_reason = TDLS_DISCONNECTED_PEER_DELETE;
		break;
	case WMI_TDLS_TEARDOWN_REASON_PTR_TIMEOUT:
		param->peer_reason = TDLS_TEARDOWN_PTR_TIMEOUT;
		break;
	case WMI_TDLS_TEARDOWN_REASON_BAD_PTR:
		param->peer_reason = TDLS_TEARDOWN_BAD_PTR;
		break;
	case WMI_TDLS_TEARDOWN_REASON_NO_RESPONSE:
		param->peer_reason = TDLS_TEARDOWN_NO_RSP;
		break;
	case WMI_TDLS_ENTER_BUF_STA:
		param->peer_reason = TDLS_PEER_ENTER_BUF_STA;
		break;
	case WMI_TDLS_EXIT_BUF_STA:
		param->peer_reason = TDLS_PEER_EXIT_BUF_STA;
		break;
	case WMI_TDLS_ENTER_BT_BUSY_MODE:
		param->peer_reason = TDLS_ENTER_BT_BUSY;
		break;
	case WMI_TDLS_EXIT_BT_BUSY_MODE:
		param->peer_reason = TDLS_EXIT_BT_BUSY;
		break;
	case WMI_TDLS_SCAN_STARTED_EVENT:
		param->peer_reason = TDLS_SCAN_STARTED;
		break;
	case WMI_TDLS_SCAN_COMPLETED_EVENT:
		param->peer_reason = TDLS_SCAN_COMPLETED;
		break;

	default:
		wmi_err("Unknown reason %d in tdls event %d from target",
			evt->peer_reason, evt->peer_status);
		return QDF_STATUS_E_INVAL;
	};

	wmi_debug("tdls event, peer: "QDF_MAC_ADDR_FMT", type: 0x%x, reason: %d, vdev: %d",
		 QDF_MAC_ADDR_REF(param->peermac.bytes),
		 param->message_type,
		 param->peer_reason, param->vdev_id);

	return QDF_STATUS_SUCCESS;
}

void wmi_tdls_attach_tlv(struct wmi_unified *wmi_handle)
{
	struct wmi_ops *ops = wmi_handle->ops;

	ops->send_set_tdls_offchan_mode_cmd =
		send_set_tdls_offchan_mode_cmd_tlv;
	ops->send_update_fw_tdls_state_cmd =
		send_update_fw_tdls_state_cmd_tlv;
	ops->send_update_tdls_peer_state_cmd =
		send_update_tdls_peer_state_cmd_tlv;
	ops->extract_vdev_tdls_ev_param = extract_vdev_tdls_ev_param_tlv;
}
#endif /* FEATURE_WLAN_TDLS */

/*
 * send_process_set_ie_info_cmd_tlv() - Function to send IE info to firmware
 * @wmi_handle:    Pointer to WMi handle
 * @ie_data:       Pointer for ie data
 *
 * This function sends IE information to firmware
 *
 * Return: QDF_STATUS_SUCCESS for success otherwise failure
 *
 */
static QDF_STATUS send_process_set_ie_info_cmd_tlv(wmi_unified_t wmi_handle,
				   struct vdev_ie_info_param *ie_info)
{
	wmi_vdev_set_ie_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	uint32_t len, ie_len_aligned;
	QDF_STATUS ret;

	ie_len_aligned = roundup(ie_info->length, sizeof(uint32_t));
	/* Allocate memory for the WMI command */
	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE + ie_len_aligned;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = wmi_buf_data(buf);
	qdf_mem_zero(buf_ptr, len);

	/* Populate the WMI command */
	cmd = (wmi_vdev_set_ie_cmd_fixed_param *)buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_set_ie_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
				wmi_vdev_set_ie_cmd_fixed_param));
	cmd->vdev_id = ie_info->vdev_id;
	cmd->ie_id = ie_info->ie_id;
	cmd->ie_len = ie_info->length;
	cmd->band = ie_info->band;

	wmi_debug("IE:%d of size:%d sent for vdev:%d", ie_info->ie_id,
		 ie_info->length, ie_info->vdev_id);

	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, ie_len_aligned);
	buf_ptr += WMI_TLV_HDR_SIZE;

	qdf_mem_copy(buf_ptr, ie_info->data, cmd->ie_len);

	wmi_mtrace(WMI_VDEV_SET_IE_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_VDEV_SET_IE_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to send set IE command ret = %d", ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_set_base_macaddr_indicate_cmd_tlv() - set base mac address in fw
 * @wmi_handle: wmi handle
 * @custom_addr: base mac address
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_set_base_macaddr_indicate_cmd_tlv(wmi_unified_t wmi_handle,
					 uint8_t *custom_addr)
{
	wmi_pdev_set_base_macaddr_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int err;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_pdev_set_base_macaddr_cmd_fixed_param *) wmi_buf_data(buf);
	qdf_mem_zero(cmd, sizeof(*cmd));

	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_pdev_set_base_macaddr_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_pdev_set_base_macaddr_cmd_fixed_param));
	WMI_CHAR_ARRAY_TO_MAC_ADDR(custom_addr, &cmd->base_macaddr);
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
							wmi_handle,
							WMI_HOST_PDEV_ID_SOC);
	wmi_mtrace(WMI_PDEV_SET_BASE_MACADDR_CMDID, NO_SESSION, 0);
	err = wmi_unified_cmd_send(wmi_handle, buf,
				   sizeof(*cmd),
				   WMI_PDEV_SET_BASE_MACADDR_CMDID);
	if (err) {
		wmi_err("Failed to send set_base_macaddr cmd");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return 0;
}

#if defined(WLAN_FEATURE_ROAM_OFFLOAD) && defined(FEATURE_BLACKLIST_MGR)

static WMI_BSSID_DISALLOW_LIST_TYPE
wmi_get_wmi_reject_ap_type(enum blm_reject_ap_type reject_ap_type)
{
	switch (reject_ap_type) {
	case USERSPACE_AVOID_TYPE:
		return WMI_BSSID_DISALLOW_USER_SPACE_AVOID_LIST;
	case DRIVER_AVOID_TYPE:
		return WMI_BSSID_DISALLOW_DRIVER_AVOID_LIST;
	case USERSPACE_BLACKLIST_TYPE:
		return WMI_BSSID_DISALLOW_USER_SPACE_BLACK_LIST;
	case DRIVER_BLACKLIST_TYPE:
		return WMI_BSSID_DISALLOW_DRIVER_BLACK_LIST;
	case DRIVER_RSSI_REJECT_TYPE:
		return WMI_BSSID_DISALLOW_RSSI_REJECT_LIST;
	default:
		return WMI_BSSID_DISALLOW_DRIVER_AVOID_LIST;
	}
}

static WMI_BLACKLIST_REASON_ID
wmi_get_reject_reason(enum blm_reject_ap_reason reject_reason)
{
	switch(reject_reason) {
	case REASON_NUD_FAILURE:
		return WMI_BL_REASON_NUD_FAILURE;
	case REASON_STA_KICKOUT:
		return WMI_BL_REASON_STA_KICKOUT;
	case REASON_ROAM_HO_FAILURE:
		return WMI_BL_REASON_ROAM_HO_FAILURE;
	case REASON_ASSOC_REJECT_POOR_RSSI:
		return WMI_BL_REASON_ASSOC_REJECT_POOR_RSSI;
	case REASON_ASSOC_REJECT_OCE:
		return WMI_BL_REASON_ASSOC_REJECT_OCE;
	case REASON_USERSPACE_BL:
		return WMI_BL_REASON_USERSPACE_BL;
	case REASON_USERSPACE_AVOID_LIST:
		return WMI_BL_REASON_USERSPACE_AVOID_LIST;
	case REASON_BTM_DISASSOC_IMMINENT:
		return WMI_BL_REASON_BTM_DIASSOC_IMMINENT;
	case REASON_BTM_BSS_TERMINATION:
		return WMI_BL_REASON_BTM_BSS_TERMINATION;
	case REASON_BTM_MBO_RETRY:
		return WMI_BL_REASON_BTM_MBO_RETRY;
	case REASON_REASSOC_RSSI_REJECT:
		return WMI_BL_REASON_REASSOC_RSSI_REJECT;
	case REASON_REASSOC_NO_MORE_STAS:
		return WMI_BL_REASON_REASSOC_NO_MORE_STAS;
	default:
		return 0;
	}
}

static QDF_STATUS
send_reject_ap_list_cmd_tlv(wmi_unified_t wmi_handle,
			    struct reject_ap_params *reject_params)
{
	wmi_buf_t buf;
	QDF_STATUS status;
	uint32_t len, list_tlv_len;
	int i;
	uint8_t *buf_ptr;
	wmi_pdev_dsm_filter_fixed_param *chan_list_fp;
	wmi_pdev_bssid_disallow_list_config_param *chan_list;
	struct reject_ap_config_params *reject_list = reject_params->bssid_list;
	uint8_t num_of_reject_bssid = reject_params->num_of_reject_bssid;

	list_tlv_len = sizeof(*chan_list) * num_of_reject_bssid;

	len = sizeof(*chan_list_fp) + list_tlv_len + WMI_TLV_HDR_SIZE;
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	wmi_debug("num of reject BSSIDs %d", num_of_reject_bssid);

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	chan_list_fp = (wmi_pdev_dsm_filter_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&chan_list_fp->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_dsm_filter_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_pdev_dsm_filter_fixed_param));

	buf_ptr += sizeof(wmi_pdev_dsm_filter_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, list_tlv_len);

	buf_ptr += WMI_TLV_HDR_SIZE;
	chan_list = (wmi_pdev_bssid_disallow_list_config_param *)buf_ptr;
	for (i = 0; i < num_of_reject_bssid; i++) {

		WMITLV_SET_HDR(&chan_list->tlv_header,
		     WMITLV_TAG_STRUC_wmi_pdev_bssid_disallow_list_config_param,
			       WMITLV_GET_STRUCT_TLVLEN
				  (wmi_pdev_bssid_disallow_list_config_param));
		WMI_CHAR_ARRAY_TO_MAC_ADDR(reject_list[i].bssid.bytes,
					   &chan_list->bssid);
		chan_list->bssid_type =
		    wmi_get_wmi_reject_ap_type(reject_list[i].reject_ap_type);
		chan_list->expected_rssi = reject_list[i].expected_rssi;
		chan_list->remaining_disallow_duration =
					reject_list[i].reject_duration;
		chan_list->reason =
			wmi_get_reject_reason(reject_list[i].reject_reason);
		chan_list->original_timeout = reject_list[i].original_timeout;
		chan_list->timestamp = reject_list[i].received_time;
		chan_list->source = reject_list[i].source;
		chan_list++;
	}

	wmi_mtrace(WMI_PDEV_DSM_FILTER_CMDID, NO_SESSION, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_PDEV_DSM_FILTER_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_err("wmi_unified_cmd_send WMI_PDEV_DSM_FILTER_CMDID returned Error %d",
			 status);
		goto error;
	}

	return QDF_STATUS_SUCCESS;
error:
	wmi_buf_free(buf);
	return status;
}

void wmi_blacklist_mgr_attach_tlv(struct wmi_unified *wmi_handle)
{
	struct wmi_ops *ops = wmi_handle->ops;

	ops->send_reject_ap_list_cmd = send_reject_ap_list_cmd_tlv;
}
#endif

/**
 * send_sar_limit_cmd_tlv() - send sar limit cmd to fw
 * @wmi_handle: wmi handle
 * @params: sar limit params
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_sar_limit_cmd_tlv(wmi_unified_t wmi_handle,
		struct sar_limit_cmd_params *sar_limit_params)
{
	wmi_buf_t buf;
	QDF_STATUS qdf_status;
	wmi_sar_limits_cmd_fixed_param *cmd;
	int i;
	uint8_t *buf_ptr;
	wmi_sar_limit_cmd_row *wmi_sar_rows_list;
	struct sar_limit_cmd_row *sar_rows_list;
	uint32_t len = sizeof(*cmd) + WMI_TLV_HDR_SIZE;

	len += sizeof(wmi_sar_limit_cmd_row) * sar_limit_params->num_limit_rows;
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_status = QDF_STATUS_E_NOMEM;
		goto end;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_sar_limits_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_sar_limits_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
				(wmi_sar_limits_cmd_fixed_param));
	cmd->sar_enable = sar_limit_params->sar_enable;
	cmd->commit_limits = sar_limit_params->commit_limits;
	cmd->num_limit_rows = sar_limit_params->num_limit_rows;

	wmi_debug("no of sar rows = %d, len = %d",
		 sar_limit_params->num_limit_rows, len);
	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       sizeof(wmi_sar_limit_cmd_row) *
			      sar_limit_params->num_limit_rows);
	if (cmd->num_limit_rows == 0)
		goto send_sar_limits;

	wmi_sar_rows_list = (wmi_sar_limit_cmd_row *)
				(buf_ptr + WMI_TLV_HDR_SIZE);
	sar_rows_list = sar_limit_params->sar_limit_row_list;

	for (i = 0; i < sar_limit_params->num_limit_rows; i++) {
		WMITLV_SET_HDR(&wmi_sar_rows_list->tlv_header,
			       WMITLV_TAG_STRUC_wmi_sar_limit_cmd_row,
			       WMITLV_GET_STRUCT_TLVLEN(wmi_sar_limit_cmd_row));
		wmi_sar_rows_list->band_id = sar_rows_list->band_id;
		wmi_sar_rows_list->chain_id = sar_rows_list->chain_id;
		wmi_sar_rows_list->mod_id = sar_rows_list->mod_id;
		wmi_sar_rows_list->limit_value = sar_rows_list->limit_value;
		wmi_sar_rows_list->validity_bitmap =
						sar_rows_list->validity_bitmap;
		wmi_debug("row %d, band_id = %d, chain_id = %d, mod_id = %d, limit_value = %d, validity_bitmap = %d",
			 i, wmi_sar_rows_list->band_id,
			 wmi_sar_rows_list->chain_id,
			 wmi_sar_rows_list->mod_id,
			 wmi_sar_rows_list->limit_value,
			 wmi_sar_rows_list->validity_bitmap);
		sar_rows_list++;
		wmi_sar_rows_list++;
	}
send_sar_limits:
	wmi_mtrace(WMI_SAR_LIMITS_CMDID, NO_SESSION, 0);
	qdf_status = wmi_unified_cmd_send(wmi_handle, buf, len,
					  WMI_SAR_LIMITS_CMDID);

	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		wmi_err("Failed to send WMI_SAR_LIMITS_CMDID");
		wmi_buf_free(buf);
	}

end:
	return qdf_status;
}

static QDF_STATUS get_sar_limit_cmd_tlv(wmi_unified_t wmi_handle)
{
	wmi_sar_get_limits_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint32_t len;
	QDF_STATUS status;

	wmi_debug("Enter");

	len = sizeof(*cmd);
	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_sar_get_limits_cmd_fixed_param *)wmi_buf_data(wmi_buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_sar_get_limits_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
				(wmi_sar_get_limits_cmd_fixed_param));

	cmd->reserved = 0;

	wmi_mtrace(WMI_SAR_GET_LIMITS_CMDID, NO_SESSION, 0);
	status = wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
				      WMI_SAR_GET_LIMITS_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_err("Failed to send get SAR limit cmd: %d", status);
		wmi_buf_free(wmi_buf);
	}

	wmi_debug("Exit");

	return status;
}

/**
 * wmi_sar2_result_string() - return string conversion of sar2 result
 * @result: sar2 result value
 *
 * This utility function helps log string conversion of sar2 result.
 *
 * Return: string conversion of sar 2 result, if match found;
 *	   "Unknown response" otherwise.
 */
static const char *wmi_sar2_result_string(uint32_t result)
{
	switch (result) {
	CASE_RETURN_STRING(WMI_SAR2_SUCCESS);
	CASE_RETURN_STRING(WMI_SAR2_INVALID_ANTENNA_INDEX);
	CASE_RETURN_STRING(WMI_SAR2_INVALID_TABLE_INDEX);
	CASE_RETURN_STRING(WMI_SAR2_STATE_ERROR);
	CASE_RETURN_STRING(WMI_SAR2_BDF_NO_TABLE);
	default:
		return "Unknown response";
	}
}

/**
 * extract_sar2_result_event_tlv() -  process sar response event from FW.
 * @handle: wma handle
 * @event: event buffer
 * @len: buffer length
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_sar2_result_event_tlv(void *handle,
						uint8_t *event,
						uint32_t len)
{
	wmi_sar2_result_event_fixed_param *sar2_fixed_param;

	WMI_SAR2_RESULT_EVENTID_param_tlvs *param_buf =
		(WMI_SAR2_RESULT_EVENTID_param_tlvs *)event;

	if (!param_buf) {
		wmi_err("Invalid sar2 result event buffer");
		return QDF_STATUS_E_INVAL;
	}

	sar2_fixed_param = param_buf->fixed_param;
	if (!sar2_fixed_param) {
		wmi_err("Invalid sar2 result event fixed param buffer");
		return QDF_STATUS_E_INVAL;
	}

	wmi_debug("SAR2 result: %s",
		 wmi_sar2_result_string(sar2_fixed_param->result));

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS extract_sar_limit_event_tlv(wmi_unified_t wmi_handle,
					      uint8_t *evt_buf,
					      struct sar_limit_event *event)
{
	wmi_sar_get_limits_event_fixed_param *fixed_param;
	WMI_SAR_GET_LIMITS_EVENTID_param_tlvs *param_buf;
	wmi_sar_get_limit_event_row *row_in;
	struct sar_limit_event_row *row_out;
	uint32_t row;

	if (!evt_buf) {
		wmi_err("input event is NULL");
		return QDF_STATUS_E_INVAL;
	}
	if (!event) {
		wmi_err("output event is NULL");
		return QDF_STATUS_E_INVAL;
	}

	param_buf = (WMI_SAR_GET_LIMITS_EVENTID_param_tlvs *)evt_buf;

	fixed_param = param_buf->fixed_param;
	if (!fixed_param) {
		wmi_err("Invalid fixed param");
		return QDF_STATUS_E_INVAL;
	}

	event->sar_enable = fixed_param->sar_enable;
	event->num_limit_rows = fixed_param->num_limit_rows;

	if (event->num_limit_rows > param_buf->num_sar_get_limits) {
		wmi_err("Num rows %d exceeds sar_get_limits rows len %d",
			 event->num_limit_rows, param_buf->num_sar_get_limits);
		return QDF_STATUS_E_INVAL;
	}

	if (event->num_limit_rows > MAX_SAR_LIMIT_ROWS_SUPPORTED) {
		QDF_ASSERT(0);
		wmi_err("Num rows %d exceeds max of %d",
			 event->num_limit_rows,
			 MAX_SAR_LIMIT_ROWS_SUPPORTED);
		event->num_limit_rows = MAX_SAR_LIMIT_ROWS_SUPPORTED;
	}

	row_in = param_buf->sar_get_limits;
	if (!row_in) {
		wmi_debug("sar_get_limits is NULL");
	} else {
		row_out = &event->sar_limit_row[0];
		for (row = 0; row < event->num_limit_rows; row++) {
			row_out->band_id = row_in->band_id;
			row_out->chain_id = row_in->chain_id;
			row_out->mod_id = row_in->mod_id;
			row_out->limit_value = row_in->limit_value;
			row_out++;
			row_in++;
		}
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_set_del_pmkid_cache_cmd_tlv() - send wmi cmd of set del pmkid
 * @wmi_handle: wmi handler
 * @pmk_info: pointer to PMK cache entry
 * @vdev_id: vdev id
 *
 * Return: 0 for success and non zero for failure
 */
static QDF_STATUS send_set_del_pmkid_cache_cmd_tlv(wmi_unified_t wmi_handle,
				struct wmi_unified_pmk_cache *pmk_info)
{
	wmi_pdev_update_pmk_cache_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS status;
	uint8_t *buf_ptr;
	wmi_pmk_cache *pmksa;
	uint32_t len = sizeof(*cmd);

	if (!pmk_info)
		return QDF_STATUS_E_INVAL;

	if (!pmk_info->is_flush_all)
		len += WMI_TLV_HDR_SIZE + sizeof(*pmksa);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_pdev_update_pmk_cache_cmd_fixed_param *) buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		 WMITLV_TAG_STRUC_wmi_pdev_update_pmk_cache_cmd_fixed_param,
		 WMITLV_GET_STRUCT_TLVLEN(
			wmi_pdev_update_pmk_cache_cmd_fixed_param));

	cmd->vdev_id = pmk_info->vdev_id;

	/* If pmk_info->is_flush_all is true, this is a flush request */
	if (pmk_info->is_flush_all) {
		cmd->op_flag = WMI_PMK_CACHE_OP_FLAG_FLUSH_ALL;
		cmd->num_cache = 0;
		goto send_cmd;
	}

	cmd->num_cache = 1;
	buf_ptr += sizeof(*cmd);

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
			sizeof(*pmksa));
	buf_ptr += WMI_TLV_HDR_SIZE;

	pmksa = (wmi_pmk_cache *)buf_ptr;
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_STRUC_wmi_pmk_cache,
			WMITLV_GET_STRUCT_TLVLEN
				(wmi_pmk_cache));
	pmksa->pmk_len = pmk_info->pmk_len;
	qdf_mem_copy(pmksa->pmk, pmk_info->pmk, pmksa->pmk_len);
	pmksa->pmkid_len = pmk_info->pmkid_len;
	qdf_mem_copy(pmksa->pmkid, pmk_info->pmkid, pmksa->pmkid_len);
	qdf_mem_copy(&(pmksa->bssid), &(pmk_info->bssid), sizeof(wmi_mac_addr));
	pmksa->ssid.ssid_len = pmk_info->ssid.length;
	qdf_mem_copy(&(pmksa->ssid.ssid), &(pmk_info->ssid.ssid),
		     pmksa->ssid.ssid_len);
	pmksa->cache_id = pmk_info->cache_id;
	pmksa->cat_flag = pmk_info->cat_flag;
	pmksa->action_flag = pmk_info->action_flag;

send_cmd:
	wmi_mtrace(WMI_PDEV_UPDATE_PMK_CACHE_CMDID, cmd->vdev_id, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf, len,
				      WMI_PDEV_UPDATE_PMK_CACHE_CMDID);
	if (status != QDF_STATUS_SUCCESS) {
		wmi_err("Failed to send set del pmkid cache command %d",
			status);
		wmi_buf_free(buf);
	}

	return status;
}

/**
 * send_del_ts_cmd_tlv() - send DELTS request to fw
 * @wmi_handle: wmi handle
 * @msg: delts params
 *
 * Return: CDF status
 */
static QDF_STATUS send_del_ts_cmd_tlv(wmi_unified_t wmi_handle, uint8_t vdev_id,
				uint8_t ac)
{
	wmi_vdev_wmm_delts_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_vdev_wmm_delts_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_wmm_delts_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_vdev_wmm_delts_cmd_fixed_param));
	cmd->vdev_id = vdev_id;
	cmd->ac = ac;

	wmi_debug("Delts vdev:%d, ac:%d", cmd->vdev_id, cmd->ac);
	wmi_mtrace(WMI_VDEV_WMM_DELTS_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_VDEV_WMM_DELTS_CMDID)) {
		wmi_err("Failed to send vdev DELTS command");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_aggr_qos_cmd_tlv() - send aggr qos request to fw
 * @wmi_handle: handle to wmi
 * @aggr_qos_rsp_msg - combined struct for all ADD_TS requests.
 *
 * A function to handle WMI_AGGR_QOS_REQ. This will send out
 * ADD_TS requests to firmware in loop for all the ACs with
 * active flow.
 *
 * Return: CDF status
 */
static QDF_STATUS send_aggr_qos_cmd_tlv(wmi_unified_t wmi_handle,
		      struct aggr_add_ts_param *aggr_qos_rsp_msg)
{
	int i = 0;
	wmi_vdev_wmm_addts_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	for (i = 0; i < WMI_QOS_NUM_AC_MAX; i++) {
		/* if flow in this AC is active */
		if (((1 << i) & aggr_qos_rsp_msg->tspecIdx)) {
			/*
			 * as per implementation of wma_add_ts_req() we
			 * are not waiting any response from firmware so
			 * apart from sending ADDTS to firmware just send
			 * success to upper layers
			 */
			aggr_qos_rsp_msg->status[i] = QDF_STATUS_SUCCESS;

			buf = wmi_buf_alloc(wmi_handle, len);
			if (!buf) {
				return QDF_STATUS_E_NOMEM;
			}
			cmd = (wmi_vdev_wmm_addts_cmd_fixed_param *)
				wmi_buf_data(buf);
			WMITLV_SET_HDR(&cmd->tlv_header,
			       WMITLV_TAG_STRUC_wmi_vdev_wmm_addts_cmd_fixed_param,
			       WMITLV_GET_STRUCT_TLVLEN
				       (wmi_vdev_wmm_addts_cmd_fixed_param));
			cmd->vdev_id = aggr_qos_rsp_msg->vdev_id;
			cmd->ac =
				WMI_TID_TO_AC(aggr_qos_rsp_msg->tspec[i].tsinfo.
					      traffic.userPrio);
			cmd->medium_time_us =
				aggr_qos_rsp_msg->tspec[i].mediumTime * 32;
			cmd->downgrade_type = WMM_AC_DOWNGRADE_DEPRIO;
			wmi_debug("Addts vdev:%d, ac:%d, mediumTime:%d downgrade_type:%d",
				 cmd->vdev_id, cmd->ac,
				 cmd->medium_time_us, cmd->downgrade_type);
			wmi_mtrace(WMI_VDEV_WMM_ADDTS_CMDID, cmd->vdev_id, 0);
			if (wmi_unified_cmd_send(wmi_handle, buf, len,
						 WMI_VDEV_WMM_ADDTS_CMDID)) {
				wmi_err("Failed to send vdev ADDTS command");
				aggr_qos_rsp_msg->status[i] =
							QDF_STATUS_E_FAILURE;
				wmi_buf_free(buf);
				return QDF_STATUS_E_FAILURE;
			}
		}
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_add_ts_cmd_tlv() - send ADDTS request to fw
 * @wmi_handle: wmi handle
 * @msg: ADDTS params
 *
 * Return: CDF status
 */
static QDF_STATUS send_add_ts_cmd_tlv(wmi_unified_t wmi_handle,
		 struct add_ts_param *msg)
{
	wmi_vdev_wmm_addts_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	msg->status = QDF_STATUS_SUCCESS;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_vdev_wmm_addts_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_wmm_addts_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_vdev_wmm_addts_cmd_fixed_param));
	cmd->vdev_id = msg->vdev_id;
	cmd->ac = msg->tspec.tsinfo.traffic.userPrio;
	cmd->medium_time_us = msg->tspec.mediumTime * 32;
	cmd->downgrade_type = WMM_AC_DOWNGRADE_DROP;
	wmi_debug("Addts vdev:%d, ac:%d, mediumTime:%d, downgrade_type:%d",
		 cmd->vdev_id, cmd->ac, cmd->medium_time_us,
		 cmd->downgrade_type);
	wmi_mtrace(WMI_VDEV_WMM_ADDTS_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_VDEV_WMM_ADDTS_CMDID)) {
		wmi_err("Failed to send vdev ADDTS command");
		msg->status = QDF_STATUS_E_FAILURE;
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_process_add_periodic_tx_ptrn_cmd_tlv() - add periodic tx pattern
 * @wmi_handle: wmi handle
 * @pattern: tx pattern params
 * @vdev_id: vdev id
 *
 * Return: QDF status
 */
static QDF_STATUS send_process_add_periodic_tx_ptrn_cmd_tlv(
					wmi_unified_t wmi_handle,
					struct periodic_tx_pattern *pattern,
					uint8_t vdev_id)
{
	WMI_ADD_PROACTIVE_ARP_RSP_PATTERN_CMD_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint32_t len;
	uint8_t *buf_ptr;
	uint32_t ptrn_len, ptrn_len_aligned;
	int j;

	ptrn_len = pattern->ucPtrnSize;
	ptrn_len_aligned = roundup(ptrn_len, sizeof(uint32_t));
	len = sizeof(WMI_ADD_PROACTIVE_ARP_RSP_PATTERN_CMD_fixed_param) +
	      WMI_TLV_HDR_SIZE + ptrn_len_aligned;

	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(wmi_buf);

	cmd = (WMI_ADD_PROACTIVE_ARP_RSP_PATTERN_CMD_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
	       WMITLV_TAG_STRUC_WMI_ADD_PROACTIVE_ARP_RSP_PATTERN_CMD_fixed_param,
	       WMITLV_GET_STRUCT_TLVLEN
	       (WMI_ADD_PROACTIVE_ARP_RSP_PATTERN_CMD_fixed_param));

	/* Pass the pattern id to delete for the corresponding vdev id */
	cmd->vdev_id = vdev_id;
	cmd->pattern_id = pattern->ucPtrnId;
	cmd->timeout = pattern->usPtrnIntervalMs;
	cmd->length = pattern->ucPtrnSize;

	/* Pattern info */
	buf_ptr += sizeof(WMI_ADD_PROACTIVE_ARP_RSP_PATTERN_CMD_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, ptrn_len_aligned);
	buf_ptr += WMI_TLV_HDR_SIZE;
	qdf_mem_copy(buf_ptr, pattern->ucPattern, ptrn_len);
	for (j = 0; j < pattern->ucPtrnSize; j++)
		wmi_debug("Add Ptrn: %02x", buf_ptr[j] & 0xff);

	wmi_debug("Add ptrn id: %d vdev_id: %d",
		 cmd->pattern_id, cmd->vdev_id);

	wmi_mtrace(WMI_ADD_PROACTIVE_ARP_RSP_PATTERN_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
				 WMI_ADD_PROACTIVE_ARP_RSP_PATTERN_CMDID)) {
		wmi_err("Failed to add pattern set state command");
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * send_process_del_periodic_tx_ptrn_cmd_tlv() - del periodic tx pattern
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @pattern_id: pattern id
 *
 * Return: QDF status
 */
static QDF_STATUS send_process_del_periodic_tx_ptrn_cmd_tlv(
						wmi_unified_t wmi_handle,
						uint8_t vdev_id,
						uint8_t pattern_id)
{
	WMI_DEL_PROACTIVE_ARP_RSP_PATTERN_CMD_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint32_t len =
		sizeof(WMI_DEL_PROACTIVE_ARP_RSP_PATTERN_CMD_fixed_param);

	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (WMI_DEL_PROACTIVE_ARP_RSP_PATTERN_CMD_fixed_param *)
		wmi_buf_data(wmi_buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
	       WMITLV_TAG_STRUC_WMI_DEL_PROACTIVE_ARP_RSP_PATTERN_CMD_fixed_param,
	       WMITLV_GET_STRUCT_TLVLEN
	       (WMI_DEL_PROACTIVE_ARP_RSP_PATTERN_CMD_fixed_param));

	/* Pass the pattern id to delete for the corresponding vdev id */
	cmd->vdev_id = vdev_id;
	cmd->pattern_id = pattern_id;
	wmi_debug("Del ptrn id: %d vdev_id: %d",
		 cmd->pattern_id, cmd->vdev_id);

	wmi_mtrace(WMI_DEL_PROACTIVE_ARP_RSP_PATTERN_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
				 WMI_DEL_PROACTIVE_ARP_RSP_PATTERN_CMDID)) {
		wmi_err("Failed to send del pattern command");
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * send_set_auto_shutdown_timer_cmd_tlv() - sets auto shutdown timer in firmware
 * @wmi_handle: wmi handle
 * @timer_val: auto shutdown timer value
 *
 * Return: CDF status
 */
static QDF_STATUS send_set_auto_shutdown_timer_cmd_tlv(wmi_unified_t wmi_handle,
						  uint32_t timer_val)
{
	QDF_STATUS status;
	wmi_buf_t buf = NULL;
	uint8_t *buf_ptr;
	wmi_host_auto_shutdown_cfg_cmd_fixed_param *wmi_auto_sh_cmd;
	int len = sizeof(wmi_host_auto_shutdown_cfg_cmd_fixed_param);

	wmi_debug("Set WMI_HOST_AUTO_SHUTDOWN_CFG_CMDID:TIMER_VAL=%d",
		 timer_val);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	wmi_auto_sh_cmd =
		(wmi_host_auto_shutdown_cfg_cmd_fixed_param *) buf_ptr;
	wmi_auto_sh_cmd->timer_value = timer_val;

	WMITLV_SET_HDR(&wmi_auto_sh_cmd->tlv_header,
	       WMITLV_TAG_STRUC_wmi_host_auto_shutdown_cfg_cmd_fixed_param,
	       WMITLV_GET_STRUCT_TLVLEN
	       (wmi_host_auto_shutdown_cfg_cmd_fixed_param));

	wmi_mtrace(WMI_HOST_AUTO_SHUTDOWN_CFG_CMDID, NO_SESSION, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_HOST_AUTO_SHUTDOWN_CFG_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_err("WMI_HOST_AUTO_SHUTDOWN_CFG_CMDID Err %d", status);
		wmi_buf_free(buf);
	}

	return status;
}

/**
 * send_set_led_flashing_cmd_tlv() - set led flashing in fw
 * @wmi_handle: wmi handle
 * @flashing: flashing request
 *
 * Return: CDF status
 */
static QDF_STATUS send_set_led_flashing_cmd_tlv(wmi_unified_t wmi_handle,
				struct flashing_req_params *flashing)
{
	wmi_set_led_flashing_cmd_fixed_param *cmd;
	QDF_STATUS status;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	int32_t len = sizeof(wmi_set_led_flashing_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}
	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_set_led_flashing_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_set_led_flashing_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_set_led_flashing_cmd_fixed_param));
	cmd->pattern_id = flashing->pattern_id;
	cmd->led_x0 = flashing->led_x0;
	cmd->led_x1 = flashing->led_x1;

	wmi_mtrace(WMI_PDEV_SET_LED_FLASHING_CMDID, NO_SESSION, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf, len,
				      WMI_PDEV_SET_LED_FLASHING_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_err("wmi_unified_cmd_send WMI_PEER_SET_PARAM_CMD"
			 " returned Error %d", status);
		wmi_buf_free(buf);
	}

	return status;
}

/**
 * send_process_ch_avoid_update_cmd_tlv() - handles channel avoid update request
 * @wmi_handle: wmi handle
 * @ch_avoid_update_req: channel avoid update params
 *
 * Return: CDF status
 */
static QDF_STATUS send_process_ch_avoid_update_cmd_tlv(wmi_unified_t wmi_handle)
{
	QDF_STATUS status;
	wmi_buf_t buf = NULL;
	uint8_t *buf_ptr;
	wmi_chan_avoid_update_cmd_param *ch_avoid_update_fp;
	int len = sizeof(wmi_chan_avoid_update_cmd_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	ch_avoid_update_fp = (wmi_chan_avoid_update_cmd_param *) buf_ptr;
	WMITLV_SET_HDR(&ch_avoid_update_fp->tlv_header,
		       WMITLV_TAG_STRUC_wmi_chan_avoid_update_cmd_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_chan_avoid_update_cmd_param));

	wmi_mtrace(WMI_CHAN_AVOID_UPDATE_CMDID, NO_SESSION, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_CHAN_AVOID_UPDATE_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_err("wmi_unified_cmd_send"
			" WMITLV_TABLE_WMI_CHAN_AVOID_UPDATE"
			" returned Error %d", status);
		wmi_buf_free(buf);
	}

	return status;
}

/**
 * send_pdev_set_pcl_cmd_tlv() - Send WMI_SOC_SET_PCL_CMDID to FW
 * @wmi_handle: wmi handle
 * @msg: PCL structure containing the PCL and the number of channels
 *
 * WMI_PDEV_SET_PCL_CMDID provides a Preferred Channel List (PCL) to the WLAN
 * firmware. The DBS Manager is the consumer of this information in the WLAN
 * firmware. The channel list will be used when a Virtual DEVice (VDEV) needs
 * to migrate to a new channel without host driver involvement. An example of
 * this behavior is Legacy Fast Roaming (LFR 3.0). Generally, the host will
 * manage the channel selection without firmware involvement.
 *
 * WMI_PDEV_SET_PCL_CMDID will carry only the weight list and not the actual
 * channel list. The weights corresponds to the channels sent in
 * WMI_SCAN_CHAN_LIST_CMDID. The channels from PCL would be having a higher
 * weightage compared to the non PCL channels.
 *
 * Return: Success if the cmd is sent successfully to the firmware
 */
static QDF_STATUS send_pdev_set_pcl_cmd_tlv(wmi_unified_t wmi_handle,
				struct wmi_pcl_chan_weights *msg)
{
	wmi_pdev_set_pcl_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	uint32_t *cmd_args, i, len;
	uint32_t chan_len;

	chan_len = msg->saved_num_chan;

	len = sizeof(*cmd) +
		WMI_TLV_HDR_SIZE + (chan_len * sizeof(uint32_t));

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_pdev_set_pcl_cmd_fixed_param *) wmi_buf_data(buf);
	buf_ptr = (uint8_t *) cmd;
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_pdev_set_pcl_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(wmi_pdev_set_pcl_cmd_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
							wmi_handle,
							WMI_HOST_PDEV_ID_SOC);
	cmd->num_chan = chan_len;
	buf_ptr += sizeof(wmi_pdev_set_pcl_cmd_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32,
			(chan_len * sizeof(uint32_t)));
	cmd_args = (uint32_t *) (buf_ptr + WMI_TLV_HDR_SIZE);
	for (i = 0; i < chan_len ; i++)
		cmd_args[i] = msg->weighed_valid_list[i];
	wmi_mtrace(WMI_PDEV_SET_PCL_CMDID, NO_SESSION, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_PDEV_SET_PCL_CMDID)) {
		wmi_err("Failed to send WMI_PDEV_SET_PCL_CMDID");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_POLICY_MGR_ENABLE
/**
 * send_pdev_set_dual_mac_config_cmd_tlv() - Set dual mac config to FW
 * @wmi_handle: wmi handle
 * @msg: Dual MAC config parameters
 *
 * Configures WLAN firmware with the dual MAC features
 *
 * Return: QDF_STATUS. 0 on success.
 */
static
QDF_STATUS send_pdev_set_dual_mac_config_cmd_tlv(wmi_unified_t wmi_handle,
		struct policy_mgr_dual_mac_config *msg)
{
	wmi_pdev_set_mac_config_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint32_t len;

	len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_pdev_set_mac_config_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_pdev_set_mac_config_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			wmi_pdev_set_mac_config_cmd_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
							wmi_handle,
							WMI_HOST_PDEV_ID_SOC);
	cmd->concurrent_scan_config_bits = msg->scan_config;
	cmd->fw_mode_config_bits = msg->fw_mode_config;
	wmi_debug("scan_config:%x fw_mode_config:%x",
		 msg->scan_config, msg->fw_mode_config);

	wmi_mtrace(WMI_PDEV_SET_MAC_CONFIG_CMDID, NO_SESSION, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_PDEV_SET_MAC_CONFIG_CMDID)) {
		wmi_err("Failed to send WMI_PDEV_SET_MAC_CONFIG_CMDID");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

void wmi_policy_mgr_attach_tlv(struct wmi_unified *wmi_handle)
{
	struct wmi_ops *ops = wmi_handle->ops;

	ops->send_pdev_set_dual_mac_config_cmd =
		send_pdev_set_dual_mac_config_cmd_tlv;
}
#endif /* WLAN_POLICY_MGR_ENABLE */

/**
 * send_adapt_dwelltime_params_cmd_tlv() - send wmi cmd of adaptive dwelltime
 * configuration params
 * @wma_handle:  wma handler
 * @dwelltime_params: pointer to dwelltime_params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF failure reason code for failure
 */
static
QDF_STATUS send_adapt_dwelltime_params_cmd_tlv(wmi_unified_t wmi_handle,
		struct wmi_adaptive_dwelltime_params *dwelltime_params)
{
	wmi_scan_adaptive_dwell_config_fixed_param *dwell_param;
	wmi_scan_adaptive_dwell_parameters_tlv *cmd;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	int32_t err;
	int len;

	len = sizeof(wmi_scan_adaptive_dwell_config_fixed_param);
	len += WMI_TLV_HDR_SIZE; /* TLV for ext_thresholds*/
	len += sizeof(wmi_scan_adaptive_dwell_parameters_tlv);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}
	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	dwell_param = (wmi_scan_adaptive_dwell_config_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&dwell_param->tlv_header,
		WMITLV_TAG_STRUC_wmi_scan_adaptive_dwell_config_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
		(wmi_scan_adaptive_dwell_config_fixed_param));

	dwell_param->enable = dwelltime_params->is_enabled;
	buf_ptr += sizeof(wmi_scan_adaptive_dwell_config_fixed_param);
	WMITLV_SET_HDR(buf_ptr,
		       WMITLV_TAG_ARRAY_STRUC,
		       sizeof(wmi_scan_adaptive_dwell_parameters_tlv));
	buf_ptr += WMI_TLV_HDR_SIZE;

	cmd = (wmi_scan_adaptive_dwell_parameters_tlv *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_scan_adaptive_dwell_parameters_tlv,
		       WMITLV_GET_STRUCT_TLVLEN(
		       wmi_scan_adaptive_dwell_parameters_tlv));

	cmd->default_adaptive_dwell_mode = dwelltime_params->dwelltime_mode;
	cmd->adapative_lpf_weight = dwelltime_params->lpf_weight;
	cmd->passive_monitor_interval_ms = dwelltime_params->passive_mon_intval;
	cmd->wifi_activity_threshold_pct = dwelltime_params->wifi_act_threshold;
	wmi_mtrace(WMI_SCAN_ADAPTIVE_DWELL_CONFIG_CMDID, NO_SESSION, 0);
	err = wmi_unified_cmd_send(wmi_handle, buf,
				   len, WMI_SCAN_ADAPTIVE_DWELL_CONFIG_CMDID);
	if (err) {
		wmi_err("Failed to send adapt dwelltime cmd err=%d", err);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_dbs_scan_sel_params_cmd_tlv() - send wmi cmd of DBS scan selection
 * configuration params
 * @wmi_handle: wmi handler
 * @dbs_scan_params: pointer to wmi_dbs_scan_sel_params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF failure reason code for failure
 */
static QDF_STATUS send_dbs_scan_sel_params_cmd_tlv(wmi_unified_t wmi_handle,
			struct wmi_dbs_scan_sel_params *dbs_scan_params)
{
	wmi_scan_dbs_duty_cycle_fixed_param *dbs_scan_param;
	wmi_scan_dbs_duty_cycle_tlv_param *cmd;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	QDF_STATUS err;
	uint32_t i;
	int len;

	len = sizeof(*dbs_scan_param);
	len += WMI_TLV_HDR_SIZE;
	len += dbs_scan_params->num_clients * sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	dbs_scan_param = (wmi_scan_dbs_duty_cycle_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&dbs_scan_param->tlv_header,
		       WMITLV_TAG_STRUC_wmi_scan_dbs_duty_cycle_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
				(wmi_scan_dbs_duty_cycle_fixed_param));

	dbs_scan_param->num_clients = dbs_scan_params->num_clients;
	dbs_scan_param->pdev_id = dbs_scan_params->pdev_id;
	buf_ptr += sizeof(*dbs_scan_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       (sizeof(*cmd) * dbs_scan_params->num_clients));
	buf_ptr = buf_ptr + (uint8_t) WMI_TLV_HDR_SIZE;

	for (i = 0; i < dbs_scan_params->num_clients; i++) {
		cmd = (wmi_scan_dbs_duty_cycle_tlv_param *) buf_ptr;
		WMITLV_SET_HDR(&cmd->tlv_header,
			WMITLV_TAG_STRUC_wmi_scan_dbs_duty_cycle_param_tlv,
			WMITLV_GET_STRUCT_TLVLEN(
					wmi_scan_dbs_duty_cycle_tlv_param));
		cmd->module_id = dbs_scan_params->module_id[i];
		cmd->num_dbs_scans = dbs_scan_params->num_dbs_scans[i];
		cmd->num_non_dbs_scans = dbs_scan_params->num_non_dbs_scans[i];
		buf_ptr = buf_ptr + (uint8_t) sizeof(*cmd);
	}

	wmi_mtrace(WMI_SET_SCAN_DBS_DUTY_CYCLE_CMDID, NO_SESSION, 0);
	err = wmi_unified_cmd_send(wmi_handle, buf,
				   len, WMI_SET_SCAN_DBS_DUTY_CYCLE_CMDID);
	if (QDF_IS_STATUS_ERROR(err)) {
		wmi_err("Failed to send dbs scan selection cmd err=%d", err);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_set_arp_stats_req_cmd_tlv() - send wmi cmd to set arp stats request
 * @wmi_handle: wmi handler
 * @req_buf: set arp stats request buffer
 *
 * Return: 0 for success and non zero for failure
 */
static QDF_STATUS send_set_arp_stats_req_cmd_tlv(wmi_unified_t wmi_handle,
					  struct set_arp_stats *req_buf)
{
	wmi_buf_t buf = NULL;
	QDF_STATUS status;
	int len;
	uint8_t *buf_ptr;
	wmi_vdev_set_arp_stats_cmd_fixed_param *wmi_set_arp;

	len = sizeof(wmi_vdev_set_arp_stats_cmd_fixed_param);
	if (req_buf->pkt_type_bitmap) {
		len += WMI_TLV_HDR_SIZE;
		len += sizeof(wmi_vdev_set_connectivity_check_stats);
	}
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	wmi_set_arp =
		(wmi_vdev_set_arp_stats_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&wmi_set_arp->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_set_arp_stats_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
				(wmi_vdev_set_arp_stats_cmd_fixed_param));

	/* fill in per roam config values */
	wmi_set_arp->vdev_id = req_buf->vdev_id;

	wmi_set_arp->set_clr = req_buf->flag;
	wmi_set_arp->pkt_type = req_buf->pkt_type;
	wmi_set_arp->ipv4 = req_buf->ip_addr;

	wmi_debug("NUD Stats: vdev_id %u set_clr %u pkt_type:%u ipv4 %u",
		 wmi_set_arp->vdev_id, wmi_set_arp->set_clr,
		 wmi_set_arp->pkt_type, wmi_set_arp->ipv4);

	/*
	 * pkt_type_bitmap should be non-zero to ensure
	 * presence of additional stats.
	 */
	if (req_buf->pkt_type_bitmap) {
		wmi_vdev_set_connectivity_check_stats *wmi_set_connect_stats;

		buf_ptr += sizeof(wmi_vdev_set_arp_stats_cmd_fixed_param);
		WMITLV_SET_HDR(buf_ptr,
			       WMITLV_TAG_ARRAY_STRUC,
			       sizeof(wmi_vdev_set_connectivity_check_stats));
		buf_ptr += WMI_TLV_HDR_SIZE;
		wmi_set_connect_stats =
			(wmi_vdev_set_connectivity_check_stats *)buf_ptr;
		WMITLV_SET_HDR(&wmi_set_connect_stats->tlv_header,
			WMITLV_TAG_STRUC_wmi_vdev_set_connectivity_check_stats,
			WMITLV_GET_STRUCT_TLVLEN(
					wmi_vdev_set_connectivity_check_stats));
		wmi_set_connect_stats->pkt_type_bitmap =
						req_buf->pkt_type_bitmap;
		wmi_set_connect_stats->tcp_src_port = req_buf->tcp_src_port;
		wmi_set_connect_stats->tcp_dst_port = req_buf->tcp_dst_port;
		wmi_set_connect_stats->icmp_ipv4 = req_buf->icmp_ipv4;

		wmi_debug("Connectivity Stats: pkt_type_bitmap %u tcp_src_port:%u tcp_dst_port %u icmp_ipv4 %u",
			 wmi_set_connect_stats->pkt_type_bitmap,
			 wmi_set_connect_stats->tcp_src_port,
			 wmi_set_connect_stats->tcp_dst_port,
			 wmi_set_connect_stats->icmp_ipv4);
	}

	/* Send per roam config parameters */
	wmi_mtrace(WMI_VDEV_SET_ARP_STAT_CMDID, NO_SESSION, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_VDEV_SET_ARP_STAT_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_err("WMI_SET_ARP_STATS_CMDID failed, Error %d", status);
		goto error;
	}

	wmi_debug("set arp stats flag=%d, vdev=%d",
		 req_buf->flag, req_buf->vdev_id);
	return QDF_STATUS_SUCCESS;
error:
	wmi_buf_free(buf);

	return status;
}

/**
 * send_get_arp_stats_req_cmd_tlv() - send wmi cmd to get arp stats request
 * @wmi_handle: wmi handler
 * @req_buf: get arp stats request buffer
 *
 * Return: 0 for success and non zero for failure
 */
static QDF_STATUS send_get_arp_stats_req_cmd_tlv(wmi_unified_t wmi_handle,
					  struct get_arp_stats *req_buf)
{
	wmi_buf_t buf = NULL;
	QDF_STATUS status;
	int len;
	uint8_t *buf_ptr;
	wmi_vdev_get_arp_stats_cmd_fixed_param *get_arp_stats;

	len = sizeof(wmi_vdev_get_arp_stats_cmd_fixed_param);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	get_arp_stats =
		(wmi_vdev_get_arp_stats_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&get_arp_stats->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_get_arp_stats_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
				(wmi_vdev_get_arp_stats_cmd_fixed_param));

	/* fill in arp stats req cmd values */
	get_arp_stats->vdev_id = req_buf->vdev_id;

	wmi_debug("vdev=%d", req_buf->vdev_id);
	/* Send per roam config parameters */
	wmi_mtrace(WMI_VDEV_GET_ARP_STAT_CMDID, NO_SESSION, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_VDEV_GET_ARP_STAT_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_err("WMI_GET_ARP_STATS_CMDID failed, Error %d", status);
		goto error;
	}

	return QDF_STATUS_SUCCESS;
error:
	wmi_buf_free(buf);

	return status;
}

/**
 * send_peer_unmap_conf_cmd_tlv() - send PEER UNMAP conf command to fw
 * @wmi: wmi handle
 * @vdev_id: vdev id
 * @peer_id_cnt: no. of peer ids
 * @peer_id_list: list of peer ids
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_peer_unmap_conf_cmd_tlv(wmi_unified_t wmi,
					       uint8_t vdev_id,
					       uint32_t peer_id_cnt,
					       uint16_t *peer_id_list)
{
	int i;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	A_UINT32 *peer_ids;
	wmi_peer_unmap_response_cmd_fixed_param *cmd;
	uint32_t peer_id_list_len;
	uint32_t len = sizeof(*cmd);
	QDF_STATUS status;

	if (!peer_id_cnt || !peer_id_list)
		return QDF_STATUS_E_FAILURE;

	len += WMI_TLV_HDR_SIZE;

	peer_id_list_len = peer_id_cnt * sizeof(A_UINT32);

	len += peer_id_list_len;

	buf = wmi_buf_alloc(wmi, len);

	if (!buf) {
		wmi_err("wmi_buf_alloc failed");
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_peer_unmap_response_cmd_fixed_param *)wmi_buf_data(buf);
	buf_ptr = (uint8_t *)wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_unmap_response_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_peer_unmap_response_cmd_fixed_param));

	buf_ptr += sizeof(wmi_peer_unmap_response_cmd_fixed_param);

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32,
		       peer_id_list_len);

	peer_ids = (A_UINT32 *)(buf_ptr + WMI_TLV_HDR_SIZE);

	for (i = 0; i < peer_id_cnt; i++)
		peer_ids[i] = peer_id_list[i];

	wmi_debug("vdev_id %d peer_id_cnt %d", vdev_id, peer_id_cnt);
	wmi_mtrace(WMI_PEER_UNMAP_RESPONSE_CMDID, vdev_id, 0);
	status = wmi_unified_cmd_send(wmi, buf, len,
				      WMI_PEER_UNMAP_RESPONSE_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_err("Failed to send peer unmap conf command: Err[%d]",
			 status);
		wmi_buf_free(buf);
		return status;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_ocl_cmd_tlv() - send ocl command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to coex config param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_ocl_cmd_tlv(wmi_unified_t wmi_handle, struct ocl_cmd_params *param)
{
	wmi_set_ocl_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS ret;
	int32_t len;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	cmd = (wmi_set_ocl_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_set_ocl_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
		       wmi_set_ocl_cmd_fixed_param));

	cmd->vdev_id = param->vdev_id;
	cmd->en_dis_chain = param->en_dis_chain;

	wmi_mtrace(WMI_SET_OCL_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_SET_OCL_CMDID);

	if (ret != 0) {
		wmi_err("Sending OCL CMD failed");
		wmi_buf_free(buf);
	}

	return ret;
}

void wmi_sta_attach_tlv(wmi_unified_t wmi_handle)
{
	struct wmi_ops *ops = wmi_handle->ops;

	ops->send_set_sta_sa_query_param_cmd =
		send_set_sta_sa_query_param_cmd_tlv;
	ops->send_set_sta_keep_alive_cmd = send_set_sta_keep_alive_cmd_tlv;
	ops->send_vdev_set_gtx_cfg_cmd = send_vdev_set_gtx_cfg_cmd_tlv;
	ops->send_process_dhcp_ind_cmd = send_process_dhcp_ind_cmd_tlv;
	ops->send_get_link_speed_cmd = send_get_link_speed_cmd_tlv;
	ops->send_fw_profiling_cmd = send_fw_profiling_cmd_tlv;
	ops->send_nat_keepalive_en_cmd = send_nat_keepalive_en_cmd_tlv;
	ops->send_wlm_latency_level_cmd = send_wlm_latency_level_cmd_tlv;
	ops->send_process_set_ie_info_cmd = send_process_set_ie_info_cmd_tlv;
	ops->send_set_base_macaddr_indicate_cmd =
		 send_set_base_macaddr_indicate_cmd_tlv;
	ops->send_sar_limit_cmd = send_sar_limit_cmd_tlv;
	ops->get_sar_limit_cmd = get_sar_limit_cmd_tlv;
	ops->extract_sar_limit_event = extract_sar_limit_event_tlv;
	ops->extract_sar2_result_event = extract_sar2_result_event_tlv;
	ops->send_set_del_pmkid_cache_cmd = send_set_del_pmkid_cache_cmd_tlv;
	ops->send_del_ts_cmd = send_del_ts_cmd_tlv;
	ops->send_aggr_qos_cmd = send_aggr_qos_cmd_tlv;
	ops->send_add_ts_cmd = send_add_ts_cmd_tlv;
	ops->send_process_add_periodic_tx_ptrn_cmd =
		send_process_add_periodic_tx_ptrn_cmd_tlv;
	ops->send_process_del_periodic_tx_ptrn_cmd =
		send_process_del_periodic_tx_ptrn_cmd_tlv;
	ops->send_set_auto_shutdown_timer_cmd =
		send_set_auto_shutdown_timer_cmd_tlv;
	ops->send_set_led_flashing_cmd = send_set_led_flashing_cmd_tlv;
	ops->send_process_ch_avoid_update_cmd =
		send_process_ch_avoid_update_cmd_tlv;
	ops->send_pdev_set_pcl_cmd = send_pdev_set_pcl_cmd_tlv;
	ops->send_adapt_dwelltime_params_cmd =
		send_adapt_dwelltime_params_cmd_tlv;
	ops->send_dbs_scan_sel_params_cmd =
		send_dbs_scan_sel_params_cmd_tlv;
	ops->send_set_arp_stats_req_cmd = send_set_arp_stats_req_cmd_tlv;
	ops->send_get_arp_stats_req_cmd = send_get_arp_stats_req_cmd_tlv;
	ops->send_peer_unmap_conf_cmd = send_peer_unmap_conf_cmd_tlv;
	ops->send_ocl_cmd = send_ocl_cmd_tlv;

	wmi_tdls_attach_tlv(wmi_handle);
	wmi_policy_mgr_attach_tlv(wmi_handle);
	wmi_blacklist_mgr_attach_tlv(wmi_handle);
}

