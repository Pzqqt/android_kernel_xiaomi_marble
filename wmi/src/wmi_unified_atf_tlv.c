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
		wmi_err("wmi_buf_alloc failed");
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
				wmi_handle,
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
		wmi_err("WMI Failed");
		wmi_buf_free(buf);
	}

	return retval;
}

/**
 * send_set_atf_grouping_cmd_tlv() - send set atf grouping command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to set atf grouping param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_set_atf_grouping_cmd_tlv(wmi_unified_t wmi_handle,
			      struct atf_grouping_params *param)
{
	wmi_atf_group_info *group_info;
	wmi_atf_ssid_grp_request_fixed_param *cmd;
	wmi_buf_t buf;
	uint8_t i;
	QDF_STATUS retval;
	uint32_t len = 0;
	uint8_t *buf_ptr = 0;

	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE;
	len += param->num_groups * sizeof(wmi_atf_group_info);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		wmi_err("wmi_buf_alloc failed");
		return QDF_STATUS_E_FAILURE;
	}

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	cmd = (wmi_atf_ssid_grp_request_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_atf_ssid_grp_request_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
					wmi_atf_ssid_grp_request_fixed_param));
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
				wmi_handle,
				param->pdev_id);

	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       sizeof(wmi_atf_group_info) *
		       param->num_groups);
	buf_ptr += WMI_TLV_HDR_SIZE;
	group_info = (wmi_atf_group_info *)buf_ptr;

	for (i = 0; i < param->num_groups; i++)	{
		WMITLV_SET_HDR(&group_info->tlv_header,
			       WMITLV_TAG_STRUC_wmi_atf_group_info,
			       WMITLV_GET_STRUCT_TLVLEN(
			       wmi_atf_group_info));
		group_info->atf_group_id = i;
		group_info->atf_group_units =
			param->group_info[i].percentage_group;
		WMI_ATF_GROUP_SET_GROUP_SCHED_POLICY(
			group_info->atf_group_flags,
			param->group_info[i].atf_group_units_reserved);
		group_info++;
	}

	wmi_mtrace(WMI_ATF_SSID_GROUPING_REQUEST_CMDID, NO_SESSION, 0);
	retval = wmi_unified_cmd_send(wmi_handle, buf, len,
				      WMI_ATF_SSID_GROUPING_REQUEST_CMDID);
	if (QDF_IS_STATUS_ERROR(retval)) {
		wmi_err("WMI Failed");
		wmi_buf_free(buf);
	}

	return retval;
}

/**
 * send_set_atf_group_ac_cmd_tlv() - send set atf AC command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to set atf AC group param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_set_atf_group_ac_cmd_tlv(wmi_unified_t wmi_handle,
			      struct atf_group_ac_params *param)
{
	wmi_atf_group_wmm_ac_info *ac_info;
	wmi_atf_grp_wmm_ac_cfg_request_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS ret;
	uint8_t i;
	uint32_t len = 0;
	uint8_t *buf_ptr = 0;

	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE;
	len += param->num_groups * sizeof(wmi_atf_group_wmm_ac_info);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		wmi_err("wmi_buf_alloc failed");
		return QDF_STATUS_E_FAILURE;
	}

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	cmd = (wmi_atf_grp_wmm_ac_cfg_request_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_atf_grp_wmm_ac_cfg_request_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			wmi_atf_grp_wmm_ac_cfg_request_fixed_param));
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
				wmi_handle,
				param->pdev_id);

	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       sizeof(wmi_atf_group_wmm_ac_info) * param->num_groups);
	buf_ptr += WMI_TLV_HDR_SIZE;
	ac_info = (wmi_atf_group_wmm_ac_info *)buf_ptr;
	for (i = 0; i < param->num_groups; i++)	{
		WMITLV_SET_HDR(&ac_info->tlv_header,
			       WMITLV_TAG_STRUC_wmi_atf_group_wmm_ac_info,
			       WMITLV_GET_STRUCT_TLVLEN(
			       wmi_atf_group_wmm_ac_info));
		ac_info->atf_group_id = i;
		ac_info->atf_units_be =
			param->group_info[i].atf_config_ac_be;
		ac_info->atf_units_bk =
			param->group_info[i].atf_config_ac_bk;
		ac_info->atf_units_vi =
			param->group_info[i].atf_config_ac_vi;
		ac_info->atf_units_vo =
			param->group_info[i].atf_config_ac_vo;
		ac_info++;
	}

	wmi_mtrace(WMI_ATF_GROUP_WMM_AC_CONFIG_REQUEST_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_ATF_GROUP_WMM_AC_CONFIG_REQUEST_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("WMI Failed");
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_atf_peer_request_cmd_tlv() - send atf peer request command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to atf peer request param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_atf_peer_request_cmd_tlv(wmi_unified_t wmi_handle,
			      struct atf_peer_request_params *param)
{
	wmi_peer_atf_ext_info *peer_ext_info;
	wmi_peer_atf_ext_request_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS retval;
	uint32_t len = 0;
	uint8_t i = 0;
	uint8_t *buf_ptr = 0;

	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE;
	len += param->num_peers * sizeof(wmi_peer_atf_ext_info);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		wmi_err("wmi_buf_alloc failed");
		return QDF_STATUS_E_FAILURE;
	}

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	cmd = (wmi_peer_atf_ext_request_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_atf_ext_request_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
		       wmi_peer_atf_ext_request_fixed_param));
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
				wmi_handle,
				param->pdev_id);

	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       sizeof(wmi_peer_atf_ext_info) * param->num_peers);
	buf_ptr += WMI_TLV_HDR_SIZE;
	peer_ext_info = (wmi_peer_atf_ext_info *)buf_ptr;
	for (i = 0; i < param->num_peers; i++) {
		WMITLV_SET_HDR(&peer_ext_info->tlv_header,
			       WMITLV_TAG_STRUC_wmi_peer_atf_ext_info,
			       WMITLV_GET_STRUCT_TLVLEN(
			       wmi_peer_atf_ext_info));
		qdf_mem_copy(&(peer_ext_info->peer_macaddr),
			     &param->peer_ext_info[i].peer_macaddr,
			     sizeof(wmi_mac_addr));
		peer_ext_info->atf_group_id =
				param->peer_ext_info[i].group_index;
		WMI_ATF_GROUP_SET_CFG_PEER_BIT(peer_ext_info->atf_peer_flags,
				param->peer_ext_info[i].atf_index_reserved);
		peer_ext_info++;
	}

	wmi_mtrace(WMI_PEER_ATF_EXT_REQUEST_CMDID, NO_SESSION, 0);
	retval = wmi_unified_cmd_send(wmi_handle, buf, len,
				      WMI_PEER_ATF_EXT_REQUEST_CMDID);
	if (QDF_IS_STATUS_ERROR(retval)) {
		wmi_err("WMI Failed");
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
		wmi_err("wmi_buf_alloc failed");
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
				wmi_handle,
				param->peer_info[i].pdev_id);
		peer_info++;
	}

	wmi_mtrace(WMI_PEER_BWF_REQUEST_CMDID, NO_SESSION, 0);
	retval = wmi_unified_cmd_send(wmi_handle, buf, len,
				      WMI_PEER_BWF_REQUEST_CMDID);

	if (retval != QDF_STATUS_SUCCESS) {
		wmi_err("WMI Failed");
		wmi_buf_free(buf);
	}

	return retval;
}

void wmi_atf_attach_tlv(wmi_unified_t wmi_handle)
{
	struct wmi_ops *ops = wmi_handle->ops;

#ifdef WLAN_ATF_ENABLE
	ops->send_set_atf_cmd = send_set_atf_cmd_tlv;
	ops->send_set_atf_grouping_cmd = send_set_atf_grouping_cmd_tlv;
	ops->send_set_atf_group_ac_cmd = send_set_atf_group_ac_cmd_tlv;
	ops->send_atf_peer_request_cmd = send_atf_peer_request_cmd_tlv;
#endif
	ops->send_set_bwf_cmd = send_set_bwf_cmd_tlv;
}

