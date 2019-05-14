/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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
#include <wmi.h>
#include <wmi_unified_priv.h>
#include <wmi_unified_interop_issues_ap_api.h>

/**
 * extract_interop_issues_ap_ev_param_tlv() - extract info from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold interop issues ap info
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS
extract_interop_issues_ap_ev_param_tlv(wmi_unified_t wmi_handle, void *evt_buf,
				     struct wlan_interop_issues_ap_event *data)
{
	wmi_pdev_rap_info_event_fixed_param *fixed_param;
	WMI_PDEV_RAP_INFO_EVENTID_param_tlvs *param_buf =
		(WMI_PDEV_RAP_INFO_EVENTID_param_tlvs *)evt_buf;

	if (!param_buf) {
		wmi_err_rl("Invalid param_buf");
		return -EINVAL;
	}

	fixed_param = param_buf->fixed_param;
	if (!fixed_param) {
		wmi_err_rl("Invalid fixed_praram");
		return -EINVAL;
	}

	if (fixed_param->type != WMI_ROGUE_AP_ON_STA_PS) {
		wmi_err_rl("Invalid type");
		return -EINVAL;
	}

	data->pdev_id = fixed_param->pdev_id;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&fixed_param->bssid, data->rap_addr.bytes);

	return QDF_STATUS_SUCCESS;
}

/**
 * send_set_rap_ps_cmd_tlv() - set interop issues ap mac address in fw
 * @wmi_handle: wmi handle
 * @rap: interop issues ap info
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS
send_set_rap_ps_cmd_tlv(wmi_unified_t wmi_handle,
			struct wlan_interop_issues_ap_info *rap)
{
	wmi_pdev_set_rap_config_fixed_param *cmd;
	wmi_pdev_set_rap_config_on_sta_ps_tlv_param *param;
	uint8_t *buf_ptr;
	wmi_buf_t buf;
	uint32_t ret;
	uint32_t len, count;
	qdf_size_t i;

	count = rap->count;
	len  = sizeof(*cmd) + WMI_TLV_HDR_SIZE + sizeof(*param) * count;
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	buf_ptr = wmi_buf_data(buf);
	cmd = (wmi_pdev_set_rap_config_fixed_param *)buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_set_rap_config_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_pdev_set_rap_config_fixed_param));
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
						      WMI_HOST_PDEV_ID_SOC);

	cmd->type = WMI_ROGUE_AP_ON_STA_PS;
	if (count)
		cmd->sta_ps_detection_enabled = 1;
	else
		cmd->sta_ps_detection_enabled = 0;

	buf_ptr += sizeof(*cmd);

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		  sizeof(wmi_pdev_set_rap_config_on_sta_ps_tlv_param) * count);
	buf_ptr += WMI_TLV_HDR_SIZE;

	for (i = 0; i < count; i++) {
		param = (wmi_pdev_set_rap_config_on_sta_ps_tlv_param *)buf_ptr;
		WMITLV_SET_HDR(&param->tlv_header,
		  WMITLV_TAG_STRUC_wmi_pdev_set_rap_config_on_sta_ps_tlv_param,
		  WMITLV_GET_STRUCT_TLVLEN
				(wmi_pdev_set_rap_config_on_sta_ps_tlv_param));
		WMI_CHAR_ARRAY_TO_MAC_ADDR(rap->rap_items[i].bytes,
					   &param->bssid);
		buf_ptr += sizeof(*param);
	}

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_PDEV_SET_RAP_CONFIG_CMDID);
	if (ret) {
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

void wmi_interop_issues_ap_attach_tlv(wmi_unified_t wmi_handle)
{
	struct wmi_ops *ops = wmi_handle->ops;

	ops->extract_interop_issues_ap_ev_param =
					extract_interop_issues_ap_ev_param_tlv;
	ops->send_set_rap_ps_cmd = send_set_rap_ps_cmd_tlv;
}
