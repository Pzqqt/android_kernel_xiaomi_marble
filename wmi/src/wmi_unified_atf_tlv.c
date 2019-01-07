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

#include <osdep.h>
#include "wmi.h"
#include "wmi_unified_priv.h"
#include "wmi_unified_atf_param.h"
#include "wmi_unified_atf_api.h"

#ifdef WLAN_ATF_ENABLE
/**
 * send_set_atf_cmd_tlv() - send set atf command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to set atf param
 *
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS
send_set_atf_cmd_tlv(wmi_unified_t wmi_handle,
		     struct set_atf_params *param)
{
	wmi_atf_peer_info *peer_info;
	wmi_peer_atf_request_fixed_param *cmd;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	int i;
	int32_t len = 0;
	QDF_STATUS retval;

	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE;
	len += param->num_peers * sizeof(wmi_atf_peer_info);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	cmd = (wmi_peer_atf_request_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_atf_request_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
				wmi_peer_atf_request_fixed_param));
	cmd->num_peers = param->num_peers;

	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       sizeof(wmi_atf_peer_info) *
		       cmd->num_peers);
	buf_ptr += WMI_TLV_HDR_SIZE;
	peer_info = (wmi_atf_peer_info *)buf_ptr;

	for (i = 0; i < cmd->num_peers; i++) {
		WMITLV_SET_HDR(&peer_info->tlv_header,
			    WMITLV_TAG_STRUC_wmi_atf_peer_info,
			    WMITLV_GET_STRUCT_TLVLEN(
				wmi_atf_peer_info));
		qdf_mem_copy(&(peer_info->peer_macaddr),
				&(param->peer_info[i].peer_macaddr),
				sizeof(wmi_mac_addr));
		peer_info->atf_units = param->peer_info[i].percentage_peer;
		peer_info->vdev_id = param->peer_info[i].vdev_id;
		peer_info->pdev_id =
			wmi_handle->ops->convert_pdev_id_host_to_target(
				param->peer_info[i].pdev_id);
		/*
		 * TLV definition for peer atf request fixed param combines
		 * extension stats. Legacy FW for WIN (Non-TLV) has peer atf
		 * stats and atf extension stats as two different
		 * implementations.
		 * Need to discuss with FW on this.
		 *
		 * peer_info->atf_groupid = param->peer_ext_info[i].group_index;
		 * peer_info->atf_units_reserved =
		 *		param->peer_ext_info[i].atf_index_reserved;
		 */
		peer_info++;
	}

	wmi_mtrace(WMI_PEER_ATF_REQUEST_CMDID, NO_SESSION, 0);
	retval = wmi_unified_cmd_send(wmi_handle, buf, len,
		WMI_PEER_ATF_REQUEST_CMDID);

	if (retval != QDF_STATUS_SUCCESS) {
		WMI_LOGE("%s : WMI Failed", __func__);
		wmi_buf_free(buf);
	}

	return retval;
}
#endif

/**
 * send_set_bwf_cmd_tlv() - send set bwf command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to set bwf param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_set_bwf_cmd_tlv(wmi_unified_t wmi_handle,
		     struct set_bwf_params *param)
{
	wmi_bwf_peer_info *peer_info;
	wmi_peer_bwf_request_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS retval;
	int32_t len;
	uint8_t *buf_ptr;
	int i;

	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE;
	len += param->num_peers * sizeof(wmi_bwf_peer_info);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	cmd = (wmi_peer_bwf_request_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_bwf_request_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
				wmi_peer_bwf_request_fixed_param));
	cmd->num_peers = param->num_peers;

	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       sizeof(wmi_bwf_peer_info) *
		       cmd->num_peers);
	buf_ptr += WMI_TLV_HDR_SIZE;
	peer_info = (wmi_bwf_peer_info *)buf_ptr;

	for (i = 0; i < cmd->num_peers; i++) {
		WMITLV_SET_HDR(&peer_info->tlv_header,
			       WMITLV_TAG_STRUC_wmi_bwf_peer_info,
			       WMITLV_GET_STRUCT_TLVLEN(wmi_bwf_peer_info));
		peer_info->bwf_guaranteed_bandwidth =
				param->peer_info[i].throughput;
		peer_info->bwf_max_airtime =
				param->peer_info[i].max_airtime;
		peer_info->bwf_peer_priority =
				param->peer_info[i].priority;
		qdf_mem_copy(&peer_info->peer_macaddr,
			     &param->peer_info[i].peer_macaddr,
			     sizeof(param->peer_info[i].peer_macaddr));
		peer_info->vdev_id =
				param->peer_info[i].vdev_id;
		peer_info->pdev_id =
			wmi_handle->ops->convert_pdev_id_host_to_target(
				param->peer_info[i].pdev_id);
		peer_info++;
	}

	wmi_mtrace(WMI_PEER_BWF_REQUEST_CMDID, NO_SESSION, 0);
	retval = wmi_unified_cmd_send(wmi_handle, buf, len,
				      WMI_PEER_BWF_REQUEST_CMDID);

	if (retval != QDF_STATUS_SUCCESS) {
		WMI_LOGE("%s : WMI Failed", __func__);
		wmi_buf_free(buf);
	}

	return retval;
}

void wmi_atf_attach_tlv(wmi_unified_t wmi_handle)
{
	struct wmi_ops *ops = wmi_handle->ops;

#ifdef WLAN_ATF_ENABLE
	ops->send_set_atf_cmd = send_set_atf_cmd_tlv;
#endif
	ops->send_set_bwf_cmd = send_set_bwf_cmd_tlv;
}

