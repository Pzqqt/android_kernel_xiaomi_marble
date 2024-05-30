/*
 * Copyright (c) 2018-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "wmi_unified_twt_param.h"
#include "wmi_unified_twt_api.h"

#if defined(WLAN_SUPPORT_TWT) && defined(WLAN_TWT_CONV_SUPPORTED)
static QDF_STATUS send_twt_enable_cmd_tlv(wmi_unified_t wmi_handle,
			struct twt_enable_param *params)
{
	wmi_twt_enable_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS status;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		wmi_err("Failed to allocate memory");
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_twt_enable_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
			WMITLV_TAG_STRUC_wmi_twt_enable_cmd_fixed_param,
			WMITLV_GET_STRUCT_TLVLEN
			(wmi_twt_enable_cmd_fixed_param));

	cmd->pdev_id =
		wmi_handle->ops->convert_pdev_id_host_to_target(
						wmi_handle,
						params->pdev_id);
	cmd->sta_cong_timer_ms =            params->sta_cong_timer_ms;
	cmd->mbss_support =                 params->mbss_support;
	cmd->default_slot_size =            params->default_slot_size;
	cmd->congestion_thresh_setup =      params->congestion_thresh_setup;
	cmd->congestion_thresh_teardown =   params->congestion_thresh_teardown;
	cmd->congestion_thresh_critical =   params->congestion_thresh_critical;
	cmd->interference_thresh_teardown =
					params->interference_thresh_teardown;
	cmd->interference_thresh_setup =    params->interference_thresh_setup;
	cmd->min_no_sta_setup =             params->min_no_sta_setup;
	cmd->min_no_sta_teardown =          params->min_no_sta_teardown;
	cmd->no_of_bcast_mcast_slots =      params->no_of_bcast_mcast_slots;
	cmd->min_no_twt_slots =             params->min_no_twt_slots;
	cmd->max_no_sta_twt =               params->max_no_sta_twt;
	cmd->mode_check_interval =          params->mode_check_interval;
	cmd->add_sta_slot_interval =        params->add_sta_slot_interval;
	cmd->remove_sta_slot_interval =     params->remove_sta_slot_interval;

	TWT_EN_DIS_FLAGS_SET_BTWT(cmd->flags, params->b_twt_enable);
	TWT_EN_DIS_FLAGS_SET_L_MBSSID(cmd->flags,
				      params->b_twt_legacy_mbss_enable);
	TWT_EN_DIS_FLAGS_SET_AX_MBSSID(cmd->flags,
				       params->b_twt_ax_mbss_enable);
	if (params->ext_conf_present) {
		TWT_EN_DIS_FLAGS_SET_SPLIT_CONFIG(cmd->flags, 1);
		TWT_EN_DIS_FLAGS_SET_REQ_RESP(cmd->flags, params->twt_role);
		TWT_EN_DIS_FLAGS_SET_I_B_TWT(cmd->flags, params->twt_oper);
	}

	status = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
				      WMI_TWT_ENABLE_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_err("Failed to send WMI_TWT_ENABLE_CMDID");
		wmi_buf_free(buf);
	}

	return status;
}

static WMI_DISABLE_TWT_REASON_T
wmi_convert_dis_reason_code(enum HOST_TWT_DISABLE_REASON reason)
{
	switch (reason) {
	case HOST_TWT_DISABLE_REASON_NONE:
		return WMI_DISABLE_TWT_REASON_NONE;
	case HOST_TWT_DISABLE_REASON_CONCURRENCY_SCC:
		return WMI_DISABLE_TWT_REASON_CONCURRENCY_SCC;
	case HOST_TWT_DISABLE_REASON_CONCURRENCY_MCC:
		return WMI_DISABLE_TWT_REASON_CONCURRENCY_MCC;
	case HOST_TWT_DISABLE_REASON_CHANGE_CONGESTION_TIMEOUT:
		return WMI_DISABLE_TWT_REASON_CHANGE_CONGESTION_TIMEOUT;
	case HOST_TWT_DISABLE_REASON_P2P_GO_NOA:
		return WMI_DISABLE_TWT_REASON_P2P_GO_NOA;
	default:
		return WMI_DISABLE_TWT_REASON_NONE;
	}
}

static QDF_STATUS send_twt_disable_cmd_tlv(wmi_unified_t wmi_handle,
			struct twt_disable_param *params)
{
	wmi_twt_disable_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS status;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		wmi_err("Failed to allocate memory");
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_twt_disable_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
			WMITLV_TAG_STRUC_wmi_twt_disable_cmd_fixed_param,
			WMITLV_GET_STRUCT_TLVLEN
			(wmi_twt_disable_cmd_fixed_param));

	cmd->pdev_id =
		wmi_handle->ops->convert_pdev_id_host_to_target(
						wmi_handle,
						params->pdev_id);
	if (params->ext_conf_present) {
		TWT_EN_DIS_FLAGS_SET_SPLIT_CONFIG(cmd->flags, 1);
		TWT_EN_DIS_FLAGS_SET_REQ_RESP(cmd->flags, params->twt_role);
		TWT_EN_DIS_FLAGS_SET_I_B_TWT(cmd->flags, params->twt_oper);
	}

	cmd->reason_code = wmi_convert_dis_reason_code(
					params->dis_reason_code);
	status = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
				      WMI_TWT_DISABLE_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_err("Failed to send WMI_TWT_DISABLE_CMDID");
		wmi_buf_free(buf);
	}

	return status;
}

#ifdef WLAN_SUPPORT_BCAST_TWT
static void
twt_add_dialog_set_bcast_twt_params(struct twt_add_dialog_param *params,
				    wmi_twt_add_dialog_cmd_fixed_param *cmd)
{
	TWT_FLAGS_SET_BTWT_ID0(cmd->flags, params->flag_b_twt_id0);
	cmd->b_twt_persistence = params->b_twt_persistence;
	cmd->b_twt_recommendation = params->b_twt_recommendation;
}
#else
static void
twt_add_dialog_set_bcast_twt_params(struct twt_add_dialog_param *params,
				    wmi_twt_add_dialog_cmd_fixed_param *cmd)
{
}
#endif

static QDF_STATUS
send_twt_add_dialog_cmd_tlv(wmi_unified_t wmi_handle,
			    struct twt_add_dialog_param *params)
{
	wmi_twt_add_dialog_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS status;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		wmi_err("Failed to allocate memory");
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_twt_add_dialog_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_twt_add_dialog_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_twt_add_dialog_cmd_fixed_param));

	cmd->vdev_id = params->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(params->peer_macaddr.bytes,
				   &cmd->peer_macaddr);
	cmd->dialog_id =         params->dialog_id;
	cmd->wake_intvl_us =     params->wake_intvl_us;
	cmd->wake_intvl_mantis = params->wake_intvl_mantis;
	cmd->wake_dura_us =      params->wake_dura_us;
	cmd->sp_offset_us =      params->sp_offset_us;
	cmd->min_wake_intvl_us = params->min_wake_intvl_us;
	cmd->max_wake_intvl_us = params->max_wake_intvl_us;
	cmd->min_wake_dura_us = params->min_wake_dura_us;
	cmd->max_wake_dura_us = params->max_wake_dura_us;
	cmd->sp_start_tsf_lo = (uint32_t)(params->wake_time_tsf & 0xFFFFFFFF);
	cmd->sp_start_tsf_hi = (uint32_t)(params->wake_time_tsf >> 32);
	cmd->announce_timeout_us = params->announce_timeout_us;
	TWT_FLAGS_SET_CMD(cmd->flags, params->twt_cmd);
	TWT_FLAGS_SET_BROADCAST(cmd->flags, params->flag_bcast);
	TWT_FLAGS_SET_TRIGGER(cmd->flags, params->flag_trigger);
	TWT_FLAGS_SET_FLOW_TYPE(cmd->flags, params->flag_flow_type);
	TWT_FLAGS_SET_PROTECTION(cmd->flags, params->flag_protection);

	twt_add_dialog_set_bcast_twt_params(params, cmd);

	status = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
				      WMI_TWT_ADD_DIALOG_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_err("Failed to send WMI_TWT_ADD_DIALOG_CMDID");
		wmi_buf_free(buf);
	}

	return status;
}

#ifdef WLAN_SUPPORT_BCAST_TWT
static void
twt_del_dialog_set_bcast_twt_params(struct twt_del_dialog_param *params,
				    wmi_twt_del_dialog_cmd_fixed_param *cmd)
{
	cmd->b_twt_persistence = params->b_twt_persistence;
}
#else
static void
twt_del_dialog_set_bcast_twt_params(struct twt_del_dialog_param *params,
				    wmi_twt_del_dialog_cmd_fixed_param *cmd)
{
}
#endif

static QDF_STATUS
send_twt_del_dialog_cmd_tlv(wmi_unified_t wmi_handle,
			    struct twt_del_dialog_param *params)
{
	wmi_twt_del_dialog_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS status;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		wmi_err("Failed to allocate memory");
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_twt_del_dialog_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_twt_del_dialog_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_twt_del_dialog_cmd_fixed_param));

	cmd->vdev_id = params->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(params->peer_macaddr.bytes,
				   &cmd->peer_macaddr);
	cmd->dialog_id = params->dialog_id;

	twt_del_dialog_set_bcast_twt_params(params, cmd);

	status = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
				      WMI_TWT_DEL_DIALOG_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_err("Failed to send WMI_TWT_DEL_DIALOG_CMDID");
		wmi_buf_free(buf);
	}

	return status;
}

static QDF_STATUS
send_twt_pause_dialog_cmd_tlv(wmi_unified_t wmi_handle,
			      struct twt_pause_dialog_cmd_param *params)
{
	wmi_twt_pause_dialog_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS status;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		wmi_err("Failed to allocate memory");
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_twt_pause_dialog_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_twt_pause_dialog_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_twt_pause_dialog_cmd_fixed_param));

	cmd->vdev_id = params->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(params->peer_macaddr.bytes,
				   &cmd->peer_macaddr);
	cmd->dialog_id = params->dialog_id;

	status = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
				      WMI_TWT_PAUSE_DIALOG_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_err("Failed to send WMI_TWT_PAUSE_DIALOG_CMDID");
		wmi_buf_free(buf);
	}

	return status;
}

static QDF_STATUS
send_twt_nudge_dialog_cmd_tlv(wmi_unified_t wmi_handle,
			      struct twt_nudge_dialog_cmd_param *params)
{
	wmi_twt_nudge_dialog_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS status;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	cmd = (wmi_twt_nudge_dialog_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_twt_nudge_dialog_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_twt_nudge_dialog_cmd_fixed_param));

	cmd->vdev_id = params->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(params->peer_macaddr.bytes,
				   &cmd->peer_macaddr);
	cmd->dialog_id = params->dialog_id;
	cmd->suspend_duration_ms = params->suspend_duration / 1000;
	cmd->next_twt_size = params->next_twt_size;

	wmi_debug("vdev_id: %d dialog_id: %d duration(in ms): %u next_twt_size: %d "
		  "peer_macaddr: "QDF_MAC_ADDR_FMT, cmd->vdev_id,
		  cmd->dialog_id, cmd->suspend_duration_ms, cmd->next_twt_size,
		  QDF_MAC_ADDR_REF(params->peer_macaddr.bytes));

	status = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
				      WMI_TWT_NUDGE_DIALOG_CMDID);
	if (QDF_IS_STATUS_ERROR(status))
		wmi_buf_free(buf);

	return status;
}

static QDF_STATUS send_twt_resume_dialog_cmd_tlv(wmi_unified_t wmi_handle,
			struct twt_resume_dialog_cmd_param *params)
{
	wmi_twt_resume_dialog_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS status;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		wmi_err("Failed to allocate memory");
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_twt_resume_dialog_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
			WMITLV_TAG_STRUC_wmi_twt_resume_dialog_cmd_fixed_param,
			WMITLV_GET_STRUCT_TLVLEN
			(wmi_twt_resume_dialog_cmd_fixed_param));

	cmd->vdev_id = params->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(params->peer_macaddr.bytes,
				   &cmd->peer_macaddr);
	cmd->dialog_id = params->dialog_id;
	cmd->sp_offset_us = params->sp_offset_us;
	cmd->next_twt_size = params->next_twt_size;

	status = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
						WMI_TWT_RESUME_DIALOG_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_err("Failed to send WMI_TWT_RESUME_DIALOG_CMDID");
		wmi_buf_free(buf);
	}

	return status;
}

#ifdef WLAN_SUPPORT_BCAST_TWT
static QDF_STATUS
send_twt_btwt_invite_sta_cmd_tlv(wmi_unified_t wmi_handle,
				 struct twt_btwt_invite_sta_cmd_param *params)
{
	wmi_twt_btwt_invite_sta_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS status;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		wmi_err("Failed to allocate memory");
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_twt_btwt_invite_sta_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_twt_btwt_invite_sta_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_twt_btwt_invite_sta_cmd_fixed_param));

	cmd->vdev_id = params->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(params->peer_macaddr.bytes,
				   &cmd->peer_macaddr);
	cmd->dialog_id = params->dialog_id;

	status = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
				      WMI_TWT_BTWT_INVITE_STA_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_buf_free(buf);
		wmi_err("Failed to send WMI_TWT_BTWT_INVITE_STA_CMDID");
	}

	return status;
}

static QDF_STATUS
send_twt_btwt_remove_sta_cmd_tlv(wmi_unified_t wmi_handle,
				 struct twt_btwt_remove_sta_cmd_param
				 *params)
{
	wmi_twt_btwt_remove_sta_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS status;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		wmi_err("Failed to allocate memory");
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_twt_btwt_remove_sta_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_twt_btwt_remove_sta_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_twt_btwt_remove_sta_cmd_fixed_param));

	cmd->vdev_id = params->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(params->peer_macaddr.bytes,
				   &cmd->peer_macaddr);
	cmd->dialog_id = params->dialog_id;

	status = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
				      WMI_TWT_BTWT_REMOVE_STA_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_buf_free(buf);
		wmi_err("Failed to send WMI_TWT_BTWT_REMOVE_STA_CMDID");
	}

	return status;
}
#endif

static enum HOST_TWT_ENABLE_STATUS
wmi_twt_enable_status_to_host_twt_status(WMI_ENABLE_TWT_STATUS_T status)
{
	switch (status) {
	case WMI_ENABLE_TWT_STATUS_OK:
		return HOST_TWT_ENABLE_STATUS_OK;
	case WMI_ENABLE_TWT_STATUS_ALREADY_ENABLED:
		return HOST_TWT_ENABLE_STATUS_ALREADY_ENABLED;
	case WMI_ENABLE_TWT_STATUS_NOT_READY:
		return HOST_TWT_ENABLE_STATUS_NOT_READY;
	case WMI_ENABLE_TWT_INVALID_PARAM:
		return HOST_TWT_ENABLE_INVALID_PARAM;
	default:
		return HOST_TWT_ENABLE_STATUS_UNKNOWN_ERROR;
	}
}

static QDF_STATUS extract_twt_enable_comp_event_tlv(wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct twt_enable_complete_event_param *params)
{
	WMI_TWT_ENABLE_COMPLETE_EVENTID_param_tlvs *param_buf;
	wmi_twt_enable_complete_event_fixed_param *ev;

	param_buf = (WMI_TWT_ENABLE_COMPLETE_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("evt_buf is NULL");
		return QDF_STATUS_E_INVAL;
	}

	ev = param_buf->fixed_param;

	params->pdev_id =
		wmi_handle->ops->convert_pdev_id_target_to_host(wmi_handle,
								ev->pdev_id);
	params->status = wmi_twt_enable_status_to_host_twt_status(ev->status);

	return QDF_STATUS_SUCCESS;
}

static enum HOST_TWT_DISABLE_STATUS
wmi_twt_disable_status_to_host_twt_status(WMI_DISABLE_TWT_STATUS_T status)
{
	switch (status) {
	case WMI_DISABLE_TWT_STATUS_OK:
		return HOST_TWT_DISABLE_STATUS_OK;
	case WMI_DISABLE_TWT_STATUS_ROAM_IN_PROGRESS:
		return HOST_TWT_DISABLE_STATUS_ROAM_IN_PROGRESS;
	case WMI_DISABLE_TWT_STATUS_CHAN_SW_IN_PROGRESS:
		return HOST_TWT_DISABLE_STATUS_CHAN_SW_IN_PROGRESS;
	case WMI_DISABLE_TWT_STATUS_SCAN_IN_PROGRESS:
		return HOST_TWT_DISABLE_STATUS_SCAN_IN_PROGRESS;
	default:
		return HOST_TWT_DISABLE_STATUS_UNKNOWN_ERROR;
	}
}

static QDF_STATUS extract_twt_disable_comp_event_tlv(wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct twt_disable_complete_event_param *params)
{
	WMI_TWT_DISABLE_COMPLETE_EVENTID_param_tlvs *param_buf;
	wmi_twt_disable_complete_event_fixed_param *ev;

	param_buf = (WMI_TWT_DISABLE_COMPLETE_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("evt_buf is NULL");
		return QDF_STATUS_E_INVAL;
	}

	ev = param_buf->fixed_param;

	params->pdev_id =
		wmi_handle->ops->convert_pdev_id_target_to_host(wmi_handle,
								ev->pdev_id);
	params->status = wmi_twt_disable_status_to_host_twt_status(ev->status);

	return QDF_STATUS_SUCCESS;
}

static enum HOST_TWT_ADD_STATUS
wmi_get_converted_twt_add_dialog_status(WMI_ADD_TWT_STATUS_T tgt_status)
{
	switch (tgt_status) {
	case WMI_ADD_TWT_STATUS_OK:
		return HOST_TWT_ADD_STATUS_OK;
	case WMI_ADD_TWT_STATUS_TWT_NOT_ENABLED:
		return HOST_TWT_ADD_STATUS_TWT_NOT_ENABLED;
	case WMI_ADD_TWT_STATUS_USED_DIALOG_ID:
		return HOST_TWT_ADD_STATUS_USED_DIALOG_ID;
	case WMI_ADD_TWT_STATUS_INVALID_PARAM:
		return HOST_TWT_ADD_STATUS_INVALID_PARAM;
	case WMI_ADD_TWT_STATUS_NOT_READY:
		return HOST_TWT_ADD_STATUS_NOT_READY;
	case WMI_ADD_TWT_STATUS_NO_RESOURCE:
		return HOST_TWT_ADD_STATUS_NO_RESOURCE;
	case WMI_ADD_TWT_STATUS_NO_ACK:
		return HOST_TWT_ADD_STATUS_NO_ACK;
	case WMI_ADD_TWT_STATUS_NO_RESPONSE:
		return HOST_TWT_ADD_STATUS_NO_RESPONSE;
	case WMI_ADD_TWT_STATUS_DENIED:
		return HOST_TWT_ADD_STATUS_DENIED;
	case WMI_ADD_TWT_STATUS_AP_PARAMS_NOT_IN_RANGE:
		return HOST_TWT_ADD_STATUS_AP_PARAMS_NOT_IN_RANGE;
	case WMI_ADD_TWT_STATUS_AP_IE_VALIDATION_FAILED:
		return HOST_TWT_ADD_STATUS_AP_IE_VALIDATION_FAILED;
	case WMI_ADD_TWT_STATUS_ROAM_IN_PROGRESS:
		return HOST_TWT_ADD_STATUS_ROAM_IN_PROGRESS;
	case WMI_ADD_TWT_STATUS_CHAN_SW_IN_PROGRESS:
		return HOST_TWT_ADD_STATUS_CHAN_SW_IN_PROGRESS;
	case WMI_ADD_TWT_STATUS_SCAN_IN_PROGRESS:
		return HOST_TWT_ADD_STATUS_SCAN_IN_PROGRESS;
	default:
		return HOST_TWT_ADD_STATUS_UNKNOWN_ERROR;
	}
}

/**
 * extract_twt_add_dialog_comp_event_tlv - Extacts twt add dialog complete wmi
 * event from firmware
 * @wmi_hande: WMI handle
 * @evt_buf: Pointer to wmi event buf of twt add dialog complete event
 * @params: Pointer to store the extracted parameters
 *
 * Return: QDF_STATUS_SUCCESS on success or QDF STATUS error values on failure
 */
static QDF_STATUS extract_twt_add_dialog_comp_event_tlv(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct twt_add_dialog_complete_event_param *params)
{
	WMI_TWT_ADD_DIALOG_COMPLETE_EVENTID_param_tlvs *param_buf;
	wmi_twt_add_dialog_complete_event_fixed_param *ev;

	param_buf = (WMI_TWT_ADD_DIALOG_COMPLETE_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("evt_buf is NULL");
		return QDF_STATUS_E_INVAL;
	}

	ev = param_buf->fixed_param;

	params->vdev_id = ev->vdev_id;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&ev->peer_macaddr,
				   params->peer_macaddr.bytes);
	params->status = wmi_get_converted_twt_add_dialog_status(ev->status);
	params->dialog_id = ev->dialog_id;
	params->num_additional_twt_params = param_buf->num_twt_params;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_twt_add_dialog_comp_additional_parameters() - Extracts additional twt
 * twt parameters, as part of add dialog completion event
 * @wmi_hdl: wmi handle
 * @evt_buf: Pointer event buffer
 * @evt_buf_len: length of the add dialog event buffer
 * @idx: index of num_twt_params
 * @additional_params: twt additional parameters to extract
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_INVAL for failure
 */
static QDF_STATUS extract_twt_add_dialog_comp_additional_parameters
(
	wmi_unified_t wmi_handle, uint8_t *evt_buf,
	uint32_t evt_buf_len, uint32_t idx,
	struct twt_add_dialog_additional_params *additional_params
)
{
	WMI_TWT_ADD_DIALOG_COMPLETE_EVENTID_param_tlvs *param_buf;
	wmi_twt_add_dialog_complete_event_fixed_param *ev;
	uint32_t flags = 0;
	uint32_t expected_len;

	param_buf = (WMI_TWT_ADD_DIALOG_COMPLETE_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("evt_buf is NULL");
		return QDF_STATUS_E_INVAL;
	}

	ev = param_buf->fixed_param;

	if (idx >= param_buf->num_twt_params) {
		wmi_err("Invalid idx %d while num_twt_params = %d",
			 idx, param_buf->num_twt_params);
		return QDF_STATUS_E_INVAL;
	}

	if (!param_buf->twt_params) {
		wmi_err("Unable to extract additional twt parameters");
		return QDF_STATUS_E_INVAL;
	}

	expected_len = (sizeof(wmi_twt_add_dialog_complete_event_fixed_param) +
			WMI_TLV_HDR_SIZE + (param_buf->num_twt_params *
			sizeof(wmi_twt_add_dialog_additional_params)));

	if (evt_buf_len != expected_len) {
		wmi_err("Got invalid len data from FW %d expected %d",
			 evt_buf_len, expected_len);
		return QDF_STATUS_E_INVAL;
	}

	flags = param_buf->twt_params[idx].flags;
	additional_params->twt_cmd = TWT_FLAGS_GET_CMD(flags);
	additional_params->bcast = TWT_FLAGS_GET_BROADCAST(flags);
	additional_params->trig_en = TWT_FLAGS_GET_TRIGGER(flags);
	additional_params->announce = TWT_FLAGS_GET_FLOW_TYPE(flags);
	additional_params->protection = TWT_FLAGS_GET_PROTECTION(flags);
	additional_params->b_twt_id0 = TWT_FLAGS_GET_BTWT_ID0(flags);
	additional_params->info_frame_disabled =
				TWT_FLAGS_GET_TWT_INFO_FRAME_DISABLED(flags);
	additional_params->wake_dur_us = param_buf->twt_params[idx].wake_dur_us;
	additional_params->wake_intvl_us =
				param_buf->twt_params[idx].wake_intvl_us;
	additional_params->sp_offset_us =
				param_buf->twt_params[idx].sp_offset_us;
	additional_params->sp_tsf_us_lo =
				param_buf->twt_params[idx].sp_tsf_us_lo;
	additional_params->sp_tsf_us_hi =
				param_buf->twt_params[idx].sp_tsf_us_hi;

	return QDF_STATUS_SUCCESS;
}

static enum HOST_TWT_DEL_STATUS
wmi_get_converted_twt_del_dialog_status(WMI_DEL_TWT_STATUS_T tgt_status)
{
	switch (tgt_status) {
	case WMI_DEL_TWT_STATUS_OK:
		return HOST_TWT_DEL_STATUS_OK;
	case WMI_DEL_TWT_STATUS_DIALOG_ID_NOT_EXIST:
		return HOST_TWT_DEL_STATUS_DIALOG_ID_NOT_EXIST;
	case WMI_DEL_TWT_STATUS_INVALID_PARAM:
		return HOST_TWT_DEL_STATUS_INVALID_PARAM;
	case WMI_DEL_TWT_STATUS_DIALOG_ID_BUSY:
		return HOST_TWT_DEL_STATUS_DIALOG_ID_BUSY;
	case WMI_DEL_TWT_STATUS_NO_RESOURCE:
		return HOST_TWT_DEL_STATUS_NO_RESOURCE;
	case WMI_DEL_TWT_STATUS_NO_ACK:
		return HOST_TWT_DEL_STATUS_NO_ACK;
	case WMI_DEL_TWT_STATUS_PEER_INIT_TEARDOWN:
		return HOST_TWT_DEL_STATUS_PEER_INIT_TEARDOWN;
	case WMI_DEL_TWT_STATUS_ROAMING:
		return HOST_TWT_DEL_STATUS_ROAMING;
	case WMI_DEL_TWT_STATUS_CONCURRENCY:
		return HOST_TWT_DEL_STATUS_CONCURRENCY;
	case WMI_DEL_TWT_STATUS_CHAN_SW_IN_PROGRESS:
		return HOST_TWT_DEL_STATUS_CHAN_SW_IN_PROGRESS;
	case WMI_DEL_TWT_STATUS_SCAN_IN_PROGRESS:
		return HOST_TWT_DEL_STATUS_SCAN_IN_PROGRESS;
	default:
		return HOST_TWT_DEL_STATUS_UNKNOWN_ERROR;
	}
}

static QDF_STATUS extract_twt_del_dialog_comp_event_tlv(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct twt_del_dialog_complete_event_param *params)
{
	WMI_TWT_DEL_DIALOG_COMPLETE_EVENTID_param_tlvs *param_buf;
	wmi_twt_del_dialog_complete_event_fixed_param *ev;

	param_buf = (WMI_TWT_DEL_DIALOG_COMPLETE_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("evt_buf is NULL");
		return QDF_STATUS_E_INVAL;
	}

	ev = param_buf->fixed_param;

	params->vdev_id = ev->vdev_id;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&ev->peer_macaddr,
				   params->peer_macaddr.bytes);
	params->dialog_id = ev->dialog_id;
	params->status = wmi_get_converted_twt_del_dialog_status(ev->status);

	return QDF_STATUS_SUCCESS;
}

static enum HOST_TWT_PAUSE_STATUS
wmi_twt_pause_status_to_host_twt_status(WMI_PAUSE_TWT_STATUS_T status)
{
	switch (status) {
	case WMI_PAUSE_TWT_STATUS_OK:
		return HOST_TWT_PAUSE_STATUS_OK;
	case WMI_PAUSE_TWT_STATUS_DIALOG_ID_NOT_EXIST:
		return HOST_TWT_PAUSE_STATUS_DIALOG_ID_NOT_EXIST;
	case WMI_PAUSE_TWT_STATUS_INVALID_PARAM:
		return HOST_TWT_PAUSE_STATUS_INVALID_PARAM;
	case WMI_PAUSE_TWT_STATUS_DIALOG_ID_BUSY:
		return HOST_TWT_PAUSE_STATUS_DIALOG_ID_BUSY;
	case WMI_PAUSE_TWT_STATUS_NO_RESOURCE:
		return HOST_TWT_PAUSE_STATUS_NO_RESOURCE;
	case WMI_PAUSE_TWT_STATUS_NO_ACK:
		return HOST_TWT_PAUSE_STATUS_NO_ACK;
	case WMI_PAUSE_TWT_STATUS_ALREADY_PAUSED:
		return HOST_TWT_PAUSE_STATUS_ALREADY_PAUSED;
	case WMI_PAUSE_TWT_STATUS_CHAN_SW_IN_PROGRESS:
		return HOST_TWT_PAUSE_STATUS_CHAN_SW_IN_PROGRESS;
	case WMI_PAUSE_TWT_STATUS_ROAM_IN_PROGRESS:
		return HOST_TWT_PAUSE_STATUS_ROAM_IN_PROGRESS;
	case WMI_PAUSE_TWT_STATUS_SCAN_IN_PROGRESS:
		return HOST_TWT_PAUSE_STATUS_SCAN_IN_PROGRESS;
	default:
		return HOST_TWT_PAUSE_STATUS_UNKNOWN_ERROR;
	}
}

static QDF_STATUS extract_twt_pause_dialog_comp_event_tlv(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct twt_pause_dialog_complete_event_param *params)
{
	WMI_TWT_PAUSE_DIALOG_COMPLETE_EVENTID_param_tlvs *param_buf;
	wmi_twt_pause_dialog_complete_event_fixed_param *ev;

	param_buf = (WMI_TWT_PAUSE_DIALOG_COMPLETE_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("evt_buf is NULL");
		return QDF_STATUS_E_INVAL;
	}

	ev = param_buf->fixed_param;

	params->vdev_id = ev->vdev_id;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&ev->peer_macaddr,
				   params->peer_macaddr.bytes);
	params->status = wmi_twt_pause_status_to_host_twt_status(ev->status);
	params->dialog_id = ev->dialog_id;

	return QDF_STATUS_SUCCESS;
}

static enum HOST_TWT_NUDGE_STATUS
wmi_twt_nudge_status_to_host_twt_status(WMI_TWT_NUDGE_STATUS_T status)
{
	switch (status) {
	case WMI_NUDGE_TWT_STATUS_OK:
		return HOST_TWT_NUDGE_STATUS_OK;
	case WMI_NUDGE_TWT_STATUS_DIALOG_ID_NOT_EXIST:
		return HOST_TWT_NUDGE_STATUS_DIALOG_ID_NOT_EXIST;
	case WMI_NUDGE_TWT_STATUS_INVALID_PARAM:
		return HOST_TWT_NUDGE_STATUS_INVALID_PARAM;
	case WMI_NUDGE_TWT_STATUS_DIALOG_ID_BUSY:
		return HOST_TWT_NUDGE_STATUS_DIALOG_ID_BUSY;
	case WMI_NUDGE_TWT_STATUS_NO_RESOURCE:
		return HOST_TWT_NUDGE_STATUS_NO_RESOURCE;
	case WMI_NUDGE_TWT_STATUS_NO_ACK:
		return HOST_TWT_NUDGE_STATUS_NO_ACK;
	case WMI_NUDGE_TWT_STATUS_CHAN_SW_IN_PROGRESS:
		return HOST_TWT_NUDGE_STATUS_CHAN_SW_IN_PROGRESS;
	case WMI_NUDGE_TWT_STATUS_ROAM_IN_PROGRESS:
		return HOST_TWT_NUDGE_STATUS_ROAM_IN_PROGRESS;
	case WMI_NUDGE_TWT_STATUS_SCAN_IN_PROGRESS:
		return HOST_TWT_NUDGE_STATUS_SCAN_IN_PROGRESS;
	default:
		return HOST_TWT_NUDGE_STATUS_UNKNOWN_ERROR;
	}
}

static QDF_STATUS extract_twt_nudge_dialog_comp_event_tlv(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct twt_nudge_dialog_complete_event_param *params)
{
	WMI_TWT_NUDGE_DIALOG_COMPLETE_EVENTID_param_tlvs *param_buf;
	wmi_twt_nudge_dialog_complete_event_fixed_param *ev;

	param_buf = (WMI_TWT_NUDGE_DIALOG_COMPLETE_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("evt_buf is NULL");
		return QDF_STATUS_E_INVAL;
	}

	ev = param_buf->fixed_param;

	params->vdev_id = ev->vdev_id;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&ev->peer_macaddr,
				   params->peer_macaddr.bytes);
	params->status = wmi_twt_nudge_status_to_host_twt_status(ev->status);
	params->dialog_id = ev->dialog_id;
	params->next_twt_tsf_us_lo = ev->sp_tsf_us_lo;
	params->next_twt_tsf_us_hi = ev->sp_tsf_us_hi;

	wmi_debug("vdev_id: %d dialog_id: %d tsf hi : %x tsf lo: %x",
		  params->vdev_id, params->dialog_id,
		  params->next_twt_tsf_us_hi, params->next_twt_tsf_us_lo);

	return QDF_STATUS_SUCCESS;
}

static enum HOST_TWT_RESUME_STATUS
wmi_get_converted_twt_resume_dialog_status(WMI_RESUME_TWT_STATUS_T tgt_status)
{
	switch (tgt_status) {
	case WMI_RESUME_TWT_STATUS_OK:
		return HOST_TWT_RESUME_STATUS_OK;
	case WMI_RESUME_TWT_STATUS_DIALOG_ID_NOT_EXIST:
		return HOST_TWT_RESUME_STATUS_DIALOG_ID_NOT_EXIST;
	case WMI_RESUME_TWT_STATUS_INVALID_PARAM:
		return HOST_TWT_RESUME_STATUS_INVALID_PARAM;
	case WMI_RESUME_TWT_STATUS_DIALOG_ID_BUSY:
		return HOST_TWT_RESUME_STATUS_DIALOG_ID_BUSY;
	case WMI_RESUME_TWT_STATUS_NOT_PAUSED:
		return HOST_TWT_RESUME_STATUS_NOT_PAUSED;
	case WMI_RESUME_TWT_STATUS_NO_RESOURCE:
		return HOST_TWT_RESUME_STATUS_NO_RESOURCE;
	case WMI_RESUME_TWT_STATUS_NO_ACK:
		return HOST_TWT_RESUME_STATUS_NO_ACK;
	case WMI_RESUME_TWT_STATUS_CHAN_SW_IN_PROGRESS:
		return HOST_TWT_RESUME_STATUS_CHAN_SW_IN_PROGRESS;
	case WMI_RESUME_TWT_STATUS_ROAM_IN_PROGRESS:
		return HOST_TWT_RESUME_STATUS_ROAM_IN_PROGRESS;
	case WMI_RESUME_TWT_STATUS_SCAN_IN_PROGRESS:
		return HOST_TWT_RESUME_STATUS_SCAN_IN_PROGRESS;
	default:
		return HOST_TWT_RESUME_STATUS_UNKNOWN_ERROR;
	}
}

static QDF_STATUS extract_twt_resume_dialog_comp_event_tlv(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct twt_resume_dialog_complete_event_param *params)
{
	WMI_TWT_RESUME_DIALOG_COMPLETE_EVENTID_param_tlvs *param_buf;
	wmi_twt_resume_dialog_complete_event_fixed_param *ev;

	param_buf =
		(WMI_TWT_RESUME_DIALOG_COMPLETE_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("evt_buf is NULL");
		return QDF_STATUS_E_INVAL;
	}

	ev = param_buf->fixed_param;

	params->vdev_id = ev->vdev_id;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&ev->peer_macaddr,
				   params->peer_macaddr.bytes);
	params->status = wmi_get_converted_twt_resume_dialog_status(ev->status);
	params->dialog_id = ev->dialog_id;

	return QDF_STATUS_SUCCESS;
}

static enum HOST_TWT_NOTIFY_STATUS
wmi_get_converted_twt_notify_status(WMI_TWT_NOTIFICATION_ID_T tgt_status)
{
	switch (tgt_status) {
	case WMI_TWT_NOTIFY_EVENT_AP_TWT_REQ_BIT_SET:
		return HOST_TWT_NOTIFY_EVENT_AP_TWT_REQ_BIT_SET;
	case WMI_TWT_NOTIFY_EVENT_AP_TWT_REQ_BIT_CLEAR:
		return HOST_TWT_NOTIFY_EVENT_AP_TWT_REQ_BIT_CLEAR;
	default:
		return HOST_TWT_NOTIFY_EVENT_READY;
	}
}

static QDF_STATUS extract_twt_notify_event_tlv(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct twt_notify_event_param *params)
{
	WMI_TWT_NOTIFY_EVENTID_param_tlvs *param_buf;
	wmi_twt_notify_event_fixed_param *ev;

	param_buf =
		(WMI_TWT_NOTIFY_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("evt_buf is NULL");
		return QDF_STATUS_E_INVAL;
	}

	ev = param_buf->fixed_param;

	if (ev->event_id > WMI_TWT_NOTIFY_EVENT_AP_TWT_REQ_BIT_CLEAR) {
		wmi_debug("Incorrect TWT notify event vdev_id: %d, status: %d",
			  ev->vdev_id, ev->event_id);
		return QDF_STATUS_E_INVAL;
	}

	params->vdev_id = ev->vdev_id;
	params->status = wmi_get_converted_twt_notify_status(ev->event_id);

	wmi_debug("Extract notify event vdev_id: %d, status: %d",
		  params->vdev_id, params->status);

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_SUPPORT_BCAST_TWT
static QDF_STATUS
extract_twt_btwt_invite_sta_comp_event_tlv(
				   wmi_unified_t wmi_handle,
				   uint8_t *evt_buf,
				   struct
				   twt_btwt_invite_sta_complete_event_param
				   *params)
{
	WMI_TWT_BTWT_INVITE_STA_COMPLETE_EVENTID_param_tlvs *param_buf;
	wmi_twt_btwt_invite_sta_complete_event_fixed_param *ev;

	param_buf =
		(WMI_TWT_BTWT_INVITE_STA_COMPLETE_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("evt_buf is NULL");
		return QDF_STATUS_E_INVAL;
	}

	ev = param_buf->fixed_param;

	params->vdev_id = ev->vdev_id;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&ev->peer_macaddr,
				   params->peer_macaddr.bytes);
	params->status = ev->status;
	params->dialog_id = ev->dialog_id;

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
extract_twt_btwt_remove_sta_comp_event_tlv(
				   wmi_unified_t wmi_handle,
				   uint8_t *evt_buf,
				   struct
				   twt_btwt_remove_sta_complete_event_param
				   *params)
{
	WMI_TWT_BTWT_REMOVE_STA_COMPLETE_EVENTID_param_tlvs *param_buf;
	wmi_twt_btwt_remove_sta_complete_event_fixed_param *ev;

	param_buf =
		(WMI_TWT_BTWT_REMOVE_STA_COMPLETE_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("evt_buf is NULL");
		return QDF_STATUS_E_INVAL;
	}

	ev = param_buf->fixed_param;

	params->vdev_id = ev->vdev_id;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&ev->peer_macaddr,
				   params->peer_macaddr.bytes);
	params->status = ev->status;
	params->dialog_id = ev->dialog_id;

	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef WLAN_SUPPORT_BCAST_TWT
static void
wmi_twt_attach_bcast_twt_tlv(struct wmi_ops *ops)
{
	ops->send_twt_btwt_invite_sta_cmd = send_twt_btwt_invite_sta_cmd_tlv;
	ops->send_twt_btwt_remove_sta_cmd = send_twt_btwt_remove_sta_cmd_tlv;
	ops->extract_twt_btwt_invite_sta_comp_event =
				extract_twt_btwt_invite_sta_comp_event_tlv;
	ops->extract_twt_btwt_remove_sta_comp_event =
				extract_twt_btwt_remove_sta_comp_event_tlv;
}
#else
static void
wmi_twt_attach_bcast_twt_tlv(struct wmi_ops *ops)
{
}
#endif

static QDF_STATUS
extract_twt_session_stats_event_tlv(wmi_unified_t wmi_handle,
				    uint8_t *evt_buf,
				    struct twt_session_stats_event_param
				    *params)
{
	WMI_TWT_SESSION_STATS_EVENTID_param_tlvs *param_buf;
	wmi_pdev_twt_session_stats_event_fixed_param *ev;

	param_buf =
		(WMI_TWT_SESSION_STATS_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("evt_buf is NULL");
		return QDF_STATUS_E_INVAL;
	}

	ev = param_buf->fixed_param;
	params->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
							wmi_handle,
							ev->pdev_id);
	params->num_sessions = param_buf->num_twt_sessions;

	wmi_debug("pdev_id=%d, num of TWT sessions=%d",
		 params->pdev_id, params->num_sessions);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
extract_twt_session_stats_event_data(wmi_unified_t wmi_handle,
				     uint8_t *evt_buf,
				     struct twt_session_stats_event_param
				     *params,
				     struct twt_session_stats_info
				     *session,
				     uint32_t idx)
{
	WMI_TWT_SESSION_STATS_EVENTID_param_tlvs *param_buf;
	wmi_twt_session_stats_info *twt_session;
	uint32_t flags;
	wmi_mac_addr *m1;
	uint8_t *m2;

	param_buf =
		(WMI_TWT_SESSION_STATS_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("evt_buf is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (idx >= param_buf->num_twt_sessions) {
		wmi_err("wrong idx, idx=%d, num_sessions=%d",
			 idx, param_buf->num_twt_sessions);
		return QDF_STATUS_E_INVAL;
	}

	twt_session = &param_buf->twt_sessions[idx];

	session->vdev_id = twt_session->vdev_id;
	m1 = &twt_session->peer_mac;
	m2 = session->peer_mac.bytes;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(m1, m2);
	session->event_type = twt_session->event_type;
	flags = twt_session->flow_id_flags;
	session->flow_id = WMI_TWT_SESSION_FLAG_FLOW_ID_GET(flags);
	session->bcast = WMI_TWT_SESSION_FLAG_BCAST_TWT_GET(flags);
	session->trig = WMI_TWT_SESSION_FLAG_TRIGGER_TWT_GET(flags);
	session->announ = WMI_TWT_SESSION_FLAG_ANNOUN_TWT_GET(flags);
	session->protection = WMI_TWT_SESSION_FLAG_TWT_PROTECTION_GET(flags);
	session->info_frame_disabled =
			WMI_TWT_SESSION_FLAG_TWT_INFO_FRAME_DISABLED_GET(flags);
	session->dialog_id = twt_session->dialog_id;
	session->wake_dura_us = twt_session->wake_dura_us;
	session->wake_intvl_us = twt_session->wake_intvl_us;
	session->sp_offset_us = twt_session->sp_offset_us;
	session->sp_tsf_us_lo = twt_session->sp_tsf_us_lo;
	session->sp_tsf_us_hi = twt_session->sp_tsf_us_hi;
	wmi_debug("type=%d id=%d bcast=%d trig=%d announ=%d diagid=%d wake_dur=%ul wake_int=%ul offset=%ul",
		 session->event_type, session->flow_id,
		 session->bcast, session->trig,
		 session->announ, session->dialog_id, session->wake_dura_us,
		 session->wake_intvl_us, session->sp_offset_us);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS extract_twt_cap_service_ready_ext2_tlv(
				wmi_unified_t wmi_handle, uint8_t *event,
				struct wmi_twt_cap_bitmap_params *var)
{
	WMI_SERVICE_READY_EXT2_EVENTID_param_tlvs *param_buf;
	wmi_twt_caps_params *twt_caps;

	param_buf = (WMI_SERVICE_READY_EXT2_EVENTID_param_tlvs *)event;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	twt_caps = param_buf->twt_caps;
	if (!twt_caps)
		return QDF_STATUS_E_INVAL;

	var->twt_ack_support_cap = WMI_GET_BITS(twt_caps->twt_capability_bitmap,
						0, 1);

	return QDF_STATUS_SUCCESS;
}

static enum WMI_HOST_TWT_CMD_FOR_ACK_EVENT
wmi_get_converted_twt_command_for_ack_event(WMI_CMD_ID tgt_cmd)
{
	switch (tgt_cmd) {
	case WMI_TWT_ADD_DIALOG_CMDID:
		return WMI_HOST_TWT_ADD_DIALOG_CMDID;
	case WMI_TWT_DEL_DIALOG_CMDID:
		return WMI_HOST_TWT_DEL_DIALOG_CMDID;
	case WMI_TWT_PAUSE_DIALOG_CMDID:
		return WMI_HOST_TWT_PAUSE_DIALOG_CMDID;
	case WMI_TWT_RESUME_DIALOG_CMDID:
		return WMI_HOST_TWT_RESUME_DIALOG_CMDID;
	case WMI_TWT_NUDGE_DIALOG_CMDID:
		return WMI_HOST_TWT_NUDGE_DIALOG_CMDID;
	default:
		return WMI_HOST_TWT_UNKNOWN_CMDID;
	}
}

static QDF_STATUS
extract_twt_ack_comp_event_tlv(wmi_unified_t wmi_handle,
			       uint8_t *evt_buf,
			       struct twt_ack_complete_event_param *var)
{
	WMI_TWT_ACK_EVENTID_param_tlvs *param_buf;
	wmi_twt_ack_event_fixed_param *ack_event;

	param_buf = (WMI_TWT_ACK_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("evt_buf is NULL");
		return QDF_STATUS_E_INVAL;
	}

	ack_event = param_buf->fixed_param;

	var->vdev_id = ack_event->vdev_id;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&ack_event->peer_macaddr,
				   var->peer_macaddr.bytes);
	var->dialog_id = ack_event->dialog_id;
	var->twt_cmd_ack = wmi_get_converted_twt_command_for_ack_event(
						ack_event->twt_cmd);

	switch (ack_event->twt_cmd) {
	case WMI_TWT_ADD_DIALOG_CMDID:
		var->status = wmi_get_converted_twt_add_dialog_status(
						ack_event->status);
		break;
	case WMI_TWT_DEL_DIALOG_CMDID:
		var->status = wmi_get_converted_twt_del_dialog_status(
						ack_event->status);
		break;
	case WMI_TWT_PAUSE_DIALOG_CMDID:
		var->status = wmi_twt_pause_status_to_host_twt_status(
						ack_event->status);
		break;
	case WMI_TWT_RESUME_DIALOG_CMDID:
		var->status = wmi_get_converted_twt_resume_dialog_status(
						ack_event->status);
		break;
	case WMI_TWT_NUDGE_DIALOG_CMDID:
		var->status = wmi_twt_nudge_status_to_host_twt_status(
						ack_event->status);
		break;
	default:
		break;
	}
	return QDF_STATUS_SUCCESS;
}
#elif WLAN_SUPPORT_TWT
static QDF_STATUS send_twt_enable_cmd_tlv(wmi_unified_t wmi_handle,
			struct wmi_twt_enable_param *params)
{
	wmi_twt_enable_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS status;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		wmi_err("Failed to allocate memory");
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_twt_enable_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
			WMITLV_TAG_STRUC_wmi_twt_enable_cmd_fixed_param,
			WMITLV_GET_STRUCT_TLVLEN
			(wmi_twt_enable_cmd_fixed_param));

	cmd->pdev_id =
		wmi_handle->ops->convert_pdev_id_host_to_target(
						wmi_handle,
						params->pdev_id);
	cmd->sta_cong_timer_ms =            params->sta_cong_timer_ms;
	cmd->mbss_support =                 params->mbss_support;
	cmd->default_slot_size =            params->default_slot_size;
	cmd->congestion_thresh_setup =      params->congestion_thresh_setup;
	cmd->congestion_thresh_teardown =   params->congestion_thresh_teardown;
	cmd->congestion_thresh_critical =   params->congestion_thresh_critical;
	cmd->interference_thresh_teardown =
					params->interference_thresh_teardown;
	cmd->interference_thresh_setup =    params->interference_thresh_setup;
	cmd->min_no_sta_setup =             params->min_no_sta_setup;
	cmd->min_no_sta_teardown =          params->min_no_sta_teardown;
	cmd->no_of_bcast_mcast_slots =      params->no_of_bcast_mcast_slots;
	cmd->min_no_twt_slots =             params->min_no_twt_slots;
	cmd->max_no_sta_twt =               params->max_no_sta_twt;
	cmd->mode_check_interval =          params->mode_check_interval;
	cmd->add_sta_slot_interval =        params->add_sta_slot_interval;
	cmd->remove_sta_slot_interval =     params->remove_sta_slot_interval;

	TWT_EN_DIS_FLAGS_SET_BTWT(cmd->flags, params->b_twt_enable);
	TWT_EN_DIS_FLAGS_SET_L_MBSSID(cmd->flags,
				      params->b_twt_legacy_mbss_enable);
	TWT_EN_DIS_FLAGS_SET_AX_MBSSID(cmd->flags,
				       params->b_twt_ax_mbss_enable);
	if (params->ext_conf_present) {
		TWT_EN_DIS_FLAGS_SET_SPLIT_CONFIG(cmd->flags, 1);
		TWT_EN_DIS_FLAGS_SET_REQ_RESP(cmd->flags, params->twt_role);
		TWT_EN_DIS_FLAGS_SET_I_B_TWT(cmd->flags, params->twt_oper);
	}

	status = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
				      WMI_TWT_ENABLE_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_err("Failed to send WMI_TWT_ENABLE_CMDID");
		wmi_buf_free(buf);
	}

	return status;
}

static QDF_STATUS send_twt_disable_cmd_tlv(wmi_unified_t wmi_handle,
			struct wmi_twt_disable_param *params)
{
	wmi_twt_disable_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS status;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		wmi_err("Failed to allocate memory");
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_twt_disable_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
			WMITLV_TAG_STRUC_wmi_twt_disable_cmd_fixed_param,
			WMITLV_GET_STRUCT_TLVLEN
			(wmi_twt_disable_cmd_fixed_param));

	cmd->pdev_id =
		wmi_handle->ops->convert_pdev_id_host_to_target(
						wmi_handle,
						params->pdev_id);
	if (params->ext_conf_present) {
		TWT_EN_DIS_FLAGS_SET_SPLIT_CONFIG(cmd->flags, 1);
		TWT_EN_DIS_FLAGS_SET_REQ_RESP(cmd->flags, params->twt_role);
		TWT_EN_DIS_FLAGS_SET_I_B_TWT(cmd->flags, params->twt_oper);
	}

	status = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
				      WMI_TWT_DISABLE_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_err("Failed to send WMI_TWT_DISABLE_CMDID");
		wmi_buf_free(buf);
	}

	return status;
}

#ifdef WLAN_SUPPORT_BCAST_TWT
static void
twt_add_dialog_set_bcast_twt_params(struct wmi_twt_add_dialog_param *params,
                wmi_twt_add_dialog_cmd_fixed_param *cmd)
{
	TWT_FLAGS_SET_BTWT_ID0(cmd->flags, params->flag_b_twt_id0);
	cmd->b_twt_persistence = params->b_twt_persistence;
	cmd->b_twt_recommendation = params->b_twt_recommendation;
}
#else
static void
twt_add_dialog_set_bcast_twt_params(struct wmi_twt_add_dialog_param *params,
                wmi_twt_add_dialog_cmd_fixed_param *cmd)
{
}
#endif

static QDF_STATUS
send_twt_add_dialog_cmd_tlv(wmi_unified_t wmi_handle,
			    struct wmi_twt_add_dialog_param *params)
{
	wmi_twt_add_dialog_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS status;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		wmi_err("Failed to allocate memory");
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_twt_add_dialog_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_twt_add_dialog_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_twt_add_dialog_cmd_fixed_param));

	cmd->vdev_id = params->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(params->peer_macaddr, &cmd->peer_macaddr);
	cmd->dialog_id =         params->dialog_id;
	cmd->wake_intvl_us =     params->wake_intvl_us;
	cmd->wake_intvl_mantis = params->wake_intvl_mantis;
	cmd->wake_dura_us =      params->wake_dura_us;
	cmd->sp_offset_us =      params->sp_offset_us;
	cmd->min_wake_intvl_us = params->min_wake_intvl_us;
	cmd->max_wake_intvl_us = params->max_wake_intvl_us;
	cmd->min_wake_dura_us = params->min_wake_dura_us;
	cmd->max_wake_dura_us = params->max_wake_dura_us;
	cmd->sp_start_tsf_lo = (uint32_t)(params->wake_time_tsf & 0xFFFFFFFF);
	cmd->sp_start_tsf_hi = (uint32_t)(params->wake_time_tsf >> 32);
	cmd->announce_timeout_us = params->announce_timeout_us;
	TWT_FLAGS_SET_CMD(cmd->flags, params->twt_cmd);
	TWT_FLAGS_SET_BROADCAST(cmd->flags, params->flag_bcast);
	TWT_FLAGS_SET_TRIGGER(cmd->flags, params->flag_trigger);
	TWT_FLAGS_SET_FLOW_TYPE(cmd->flags, params->flag_flow_type);
	TWT_FLAGS_SET_PROTECTION(cmd->flags, params->flag_protection);

	twt_add_dialog_set_bcast_twt_params(params, cmd);

	status = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
				      WMI_TWT_ADD_DIALOG_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_err("Failed to send WMI_TWT_ADD_DIALOG_CMDID");
		wmi_buf_free(buf);
	}

	return status;
}

#ifdef WLAN_SUPPORT_BCAST_TWT
static void
twt_del_dialog_set_bcast_twt_params(struct wmi_twt_del_dialog_param *params,
                wmi_twt_del_dialog_cmd_fixed_param *cmd)
{
	cmd->b_twt_persistence = params->b_twt_persistence;
}
#else
static void
twt_del_dialog_set_bcast_twt_params(struct wmi_twt_del_dialog_param *params,
                wmi_twt_del_dialog_cmd_fixed_param *cmd)
{
}
#endif

static QDF_STATUS
send_twt_del_dialog_cmd_tlv(wmi_unified_t wmi_handle,
			    struct wmi_twt_del_dialog_param *params)
{
	wmi_twt_del_dialog_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS status;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		wmi_err("Failed to allocate memory");
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_twt_del_dialog_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_twt_del_dialog_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_twt_del_dialog_cmd_fixed_param));

	cmd->vdev_id = params->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(params->peer_macaddr, &cmd->peer_macaddr);
	cmd->dialog_id = params->dialog_id;

	twt_del_dialog_set_bcast_twt_params(params, cmd);

	status = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
				      WMI_TWT_DEL_DIALOG_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_err("Failed to send WMI_TWT_DEL_DIALOG_CMDID");
		wmi_buf_free(buf);
	}

	return status;
}

static QDF_STATUS
send_twt_pause_dialog_cmd_tlv(wmi_unified_t wmi_handle,
			      struct wmi_twt_pause_dialog_cmd_param *params)
{
	wmi_twt_pause_dialog_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS status;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		wmi_err("Failed to allocate memory");
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_twt_pause_dialog_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_twt_pause_dialog_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_twt_pause_dialog_cmd_fixed_param));

	cmd->vdev_id = params->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(params->peer_macaddr, &cmd->peer_macaddr);
	cmd->dialog_id = params->dialog_id;

	status = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
				      WMI_TWT_PAUSE_DIALOG_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_err("Failed to send WMI_TWT_PAUSE_DIALOG_CMDID");
		wmi_buf_free(buf);
	}

	return status;
}

static QDF_STATUS
send_twt_nudge_dialog_cmd_tlv(wmi_unified_t wmi_handle,
			      struct wmi_twt_nudge_dialog_cmd_param *params)
{
	wmi_twt_nudge_dialog_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS status;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	cmd = (wmi_twt_nudge_dialog_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_twt_nudge_dialog_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_twt_nudge_dialog_cmd_fixed_param));

	cmd->vdev_id = params->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(params->peer_macaddr, &cmd->peer_macaddr);
	cmd->dialog_id = params->dialog_id;
	cmd->suspend_duration_ms = params->suspend_duration / 1000;
	cmd->next_twt_size = params->next_twt_size;

	wmi_debug("vdev_id: %d dialog_id: %d duration(in ms): %u next_twt_size: %d "
		  "peer_macaddr: "QDF_MAC_ADDR_FMT, cmd->vdev_id,
		  cmd->dialog_id, cmd->suspend_duration_ms, cmd->next_twt_size,
		  QDF_MAC_ADDR_REF(params->peer_macaddr));

	status = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
				      WMI_TWT_NUDGE_DIALOG_CMDID);
	if (QDF_IS_STATUS_ERROR(status))
		wmi_buf_free(buf);

	return status;
}

static QDF_STATUS send_twt_resume_dialog_cmd_tlv(wmi_unified_t wmi_handle,
			struct wmi_twt_resume_dialog_cmd_param *params)
{
	wmi_twt_resume_dialog_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS status;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		wmi_err("Failed to allocate memory");
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_twt_resume_dialog_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
			WMITLV_TAG_STRUC_wmi_twt_resume_dialog_cmd_fixed_param,
			WMITLV_GET_STRUCT_TLVLEN
			(wmi_twt_resume_dialog_cmd_fixed_param));

	cmd->vdev_id = params->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(params->peer_macaddr, &cmd->peer_macaddr);
	cmd->dialog_id = params->dialog_id;
	cmd->sp_offset_us = params->sp_offset_us;
	cmd->next_twt_size = params->next_twt_size;

	status = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
						WMI_TWT_RESUME_DIALOG_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_err("Failed to send WMI_TWT_RESUME_DIALOG_CMDID");
		wmi_buf_free(buf);
	}

	return status;
}

#ifdef WLAN_SUPPORT_BCAST_TWT
static QDF_STATUS
send_twt_btwt_invite_sta_cmd_tlv(wmi_unified_t wmi_handle,
				 struct wmi_twt_btwt_invite_sta_cmd_param
				 *params)
{
	wmi_twt_btwt_invite_sta_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS status;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		wmi_err("Failed to allocate memory");
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_twt_btwt_invite_sta_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_twt_btwt_invite_sta_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_twt_btwt_invite_sta_cmd_fixed_param));

	cmd->vdev_id = params->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(params->peer_macaddr, &cmd->peer_macaddr);
	cmd->dialog_id = params->dialog_id;

	status = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
				      WMI_TWT_BTWT_INVITE_STA_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_buf_free(buf);
	}

	return status;
}

static QDF_STATUS
send_twt_btwt_remove_sta_cmd_tlv(wmi_unified_t wmi_handle,
				 struct wmi_twt_btwt_remove_sta_cmd_param
				 *params)
{
	wmi_twt_btwt_remove_sta_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS status;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		wmi_err("Failed to allocate memory");
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_twt_btwt_remove_sta_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_twt_btwt_remove_sta_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_twt_btwt_remove_sta_cmd_fixed_param));

	cmd->vdev_id = params->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(params->peer_macaddr, &cmd->peer_macaddr);
	cmd->dialog_id = params->dialog_id;

	status = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
				      WMI_TWT_BTWT_REMOVE_STA_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_buf_free(buf);
	}

	return status;
}
#endif

static QDF_STATUS extract_twt_enable_comp_event_tlv(wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wmi_twt_enable_complete_event_param *params)
{
	WMI_TWT_ENABLE_COMPLETE_EVENTID_param_tlvs *param_buf;
	wmi_twt_enable_complete_event_fixed_param *ev;

	param_buf = (WMI_TWT_ENABLE_COMPLETE_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("evt_buf is NULL");
		return QDF_STATUS_E_INVAL;
	}

	ev = param_buf->fixed_param;

	params->pdev_id =
		wmi_handle->ops->convert_pdev_id_target_to_host(wmi_handle,
								ev->pdev_id);
	params->status = ev->status;

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS extract_twt_disable_comp_event_tlv(wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wmi_twt_disable_complete_event *params)
{
	WMI_TWT_DISABLE_COMPLETE_EVENTID_param_tlvs *param_buf;
	wmi_twt_disable_complete_event_fixed_param *ev;

	param_buf = (WMI_TWT_DISABLE_COMPLETE_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("evt_buf is NULL");
		return QDF_STATUS_E_INVAL;
	}

	ev = param_buf->fixed_param;

#if 0
	params->pdev_id =
		wmi_handle->ops->convert_pdev_id_target_to_host(wmi_handle,
								ev->pdev_id);
	params->status = ev->status;
#endif

	return QDF_STATUS_SUCCESS;
}

static enum WMI_HOST_ADD_TWT_STATUS
wmi_get_converted_twt_add_dialog_status(WMI_ADD_TWT_STATUS_T tgt_status)
{
	switch (tgt_status) {
	case WMI_ADD_TWT_STATUS_OK:
		return WMI_HOST_ADD_TWT_STATUS_OK;
	case WMI_ADD_TWT_STATUS_TWT_NOT_ENABLED:
		return WMI_HOST_ADD_TWT_STATUS_TWT_NOT_ENABLED;
	case WMI_ADD_TWT_STATUS_USED_DIALOG_ID:
		return WMI_HOST_ADD_TWT_STATUS_USED_DIALOG_ID;
	case WMI_ADD_TWT_STATUS_INVALID_PARAM:
		return WMI_HOST_ADD_TWT_STATUS_INVALID_PARAM;
	case WMI_ADD_TWT_STATUS_NOT_READY:
		return WMI_HOST_ADD_TWT_STATUS_NOT_READY;
	case WMI_ADD_TWT_STATUS_NO_RESOURCE:
		return WMI_HOST_ADD_TWT_STATUS_NO_RESOURCE;
	case WMI_ADD_TWT_STATUS_NO_ACK:
		return WMI_HOST_ADD_TWT_STATUS_NO_ACK;
	case WMI_ADD_TWT_STATUS_NO_RESPONSE:
		return WMI_HOST_ADD_TWT_STATUS_NO_RESPONSE;
	case WMI_ADD_TWT_STATUS_DENIED:
		return WMI_HOST_ADD_TWT_STATUS_DENIED;
	case WMI_ADD_TWT_STATUS_AP_PARAMS_NOT_IN_RANGE:
		return WMI_HOST_ADD_TWT_STATUS_AP_PARAMS_NOT_IN_RANGE;
	case WMI_ADD_TWT_STATUS_AP_IE_VALIDATION_FAILED:
		return WMI_HOST_ADD_TWT_STATUS_AP_IE_VALIDATION_FAILED;
	case WMI_ADD_TWT_STATUS_ROAM_IN_PROGRESS:
		return WMI_HOST_ADD_TWT_STATUS_ROAM_IN_PROGRESS;
	case WMI_ADD_TWT_STATUS_CHAN_SW_IN_PROGRESS:
		return WMI_HOST_ADD_TWT_STATUS_CHAN_SW_IN_PROGRESS;
	case WMI_ADD_TWT_STATUS_SCAN_IN_PROGRESS:
		return WMI_HOST_ADD_TWT_STATUS_SCAN_IN_PROGRESS;
	default:
		return WMI_HOST_ADD_TWT_STATUS_UNKNOWN_ERROR;
	}
}

/**
 * extract_twt_add_dialog_comp_event_tlv - Extacts twt add dialog complete wmi
 * event from firmware
 * @wmi_hande: WMI handle
 * @evt_buf: Pointer to wmi event buf of twt add dialog complete event
 * @params: Pointer to store the extracted parameters
 *
 * Return: QDF_STATUS_SUCCESS on success or QDF STATUS error values on failure
 */
static QDF_STATUS extract_twt_add_dialog_comp_event_tlv(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wmi_twt_add_dialog_complete_event_param *params)
{
	WMI_TWT_ADD_DIALOG_COMPLETE_EVENTID_param_tlvs *param_buf;
	wmi_twt_add_dialog_complete_event_fixed_param *ev;

	param_buf = (WMI_TWT_ADD_DIALOG_COMPLETE_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("evt_buf is NULL");
		return QDF_STATUS_E_INVAL;
	}

	ev = param_buf->fixed_param;

	params->vdev_id = ev->vdev_id;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&ev->peer_macaddr, params->peer_macaddr);
	params->status = wmi_get_converted_twt_add_dialog_status(ev->status);
	params->dialog_id = ev->dialog_id;
	params->num_additional_twt_params = param_buf->num_twt_params;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_twt_add_dialog_comp_additional_parameters() - Extracts additional twt
 * twt parameters, as part of add dialog completion event
 * @wmi_hdl: wmi handle
 * @evt_buf: Pointer event buffer
 * @evt_buf_len: length of the add dialog event buffer
 * @idx: index of num_twt_params
 * @additional_params: twt additional parameters to extract
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_INVAL for failure
 */
static QDF_STATUS extract_twt_add_dialog_comp_additional_parameters
(
	wmi_unified_t wmi_handle, uint8_t *evt_buf,
	uint32_t evt_buf_len, uint32_t idx,
	struct wmi_twt_add_dialog_additional_params *additional_params
)
{
	WMI_TWT_ADD_DIALOG_COMPLETE_EVENTID_param_tlvs *param_buf;
	wmi_twt_add_dialog_complete_event_fixed_param *ev;
	uint32_t flags = 0;
	uint32_t expected_len;

	param_buf = (WMI_TWT_ADD_DIALOG_COMPLETE_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("evt_buf is NULL");
		return QDF_STATUS_E_INVAL;
	}

	ev = param_buf->fixed_param;

	if (idx >= param_buf->num_twt_params) {
		wmi_err("Invalid idx %d while num_twt_params = %d",
			 idx, param_buf->num_twt_params);
		return QDF_STATUS_E_INVAL;
	}

	if (!param_buf->twt_params) {
		wmi_err("Unable to extract additional twt parameters");
		return QDF_STATUS_E_INVAL;
	}

	expected_len = (sizeof(wmi_twt_add_dialog_complete_event_fixed_param) +
			WMI_TLV_HDR_SIZE + (param_buf->num_twt_params *
			sizeof(wmi_twt_add_dialog_additional_params)));

	if (evt_buf_len != expected_len) {
		wmi_err("Got invalid len data from FW %d expected %d",
			 evt_buf_len, expected_len);
		return QDF_STATUS_E_INVAL;
	}

	flags = param_buf->twt_params[idx].flags;
	additional_params->twt_cmd = TWT_FLAGS_GET_CMD(flags);
	additional_params->bcast = TWT_FLAGS_GET_BROADCAST(flags);
	additional_params->trig_en = TWT_FLAGS_GET_TRIGGER(flags);
	additional_params->announce = TWT_FLAGS_GET_FLOW_TYPE(flags);
	additional_params->protection = TWT_FLAGS_GET_PROTECTION(flags);
	additional_params->b_twt_id0 = TWT_FLAGS_GET_BTWT_ID0(flags);
	additional_params->info_frame_disabled =
				TWT_FLAGS_GET_TWT_INFO_FRAME_DISABLED(flags);
	additional_params->wake_dur_us = param_buf->twt_params[idx].wake_dur_us;
	additional_params->wake_intvl_us =
				param_buf->twt_params[idx].wake_intvl_us;
	additional_params->sp_offset_us =
				param_buf->twt_params[idx].sp_offset_us;
	additional_params->sp_tsf_us_lo =
				param_buf->twt_params[idx].sp_tsf_us_lo;
	additional_params->sp_tsf_us_hi =
				param_buf->twt_params[idx].sp_tsf_us_hi;
	additional_params->pm_responder_bit_valid =
				TWT_FLAGS_GET_PM_RESPONDER_MODE_VALID(flags);
	additional_params->pm_responder_bit =
				TWT_FLAGS_GET_PM_RESPONDER_MODE(flags);

	return QDF_STATUS_SUCCESS;
}

static enum WMI_HOST_DEL_TWT_STATUS
wmi_get_converted_twt_del_dialog_status(WMI_DEL_TWT_STATUS_T tgt_status)
{
	switch (tgt_status) {
	case WMI_DEL_TWT_STATUS_OK:
		return WMI_HOST_DEL_TWT_STATUS_OK;
	case WMI_DEL_TWT_STATUS_DIALOG_ID_NOT_EXIST:
		return WMI_HOST_DEL_TWT_STATUS_DIALOG_ID_NOT_EXIST;
	case WMI_DEL_TWT_STATUS_INVALID_PARAM:
		return WMI_HOST_DEL_TWT_STATUS_INVALID_PARAM;
	case WMI_DEL_TWT_STATUS_DIALOG_ID_BUSY:
		return WMI_HOST_DEL_TWT_STATUS_DIALOG_ID_BUSY;
	case WMI_DEL_TWT_STATUS_NO_RESOURCE:
		return WMI_HOST_DEL_TWT_STATUS_NO_RESOURCE;
	case WMI_DEL_TWT_STATUS_NO_ACK:
		return WMI_HOST_DEL_TWT_STATUS_NO_ACK;
	case WMI_DEL_TWT_STATUS_UNKNOWN_ERROR:
		return WMI_HOST_DEL_TWT_STATUS_UNKNOWN_ERROR;
	case WMI_DEL_TWT_STATUS_PEER_INIT_TEARDOWN:
		return WMI_HOST_DEL_TWT_STATUS_PEER_INIT_TEARDOWN;
	case WMI_DEL_TWT_STATUS_ROAMING:
		return WMI_HOST_DEL_TWT_STATUS_ROAMING;
	case WMI_DEL_TWT_STATUS_CONCURRENCY:
		return WMI_HOST_DEL_TWT_STATUS_CONCURRENCY;
	case WMI_DEL_TWT_STATUS_CHAN_SW_IN_PROGRESS:
		return WMI_HOST_DEL_TWT_STATUS_CHAN_SW_IN_PROGRESS;
	case WMI_DEL_TWT_STATUS_SCAN_IN_PROGRESS:
		return WMI_HOST_DEL_TWT_STATUS_SCAN_IN_PROGRESS;
	default:
		return WMI_HOST_DEL_TWT_STATUS_UNKNOWN_ERROR;
	}

	return WMI_HOST_DEL_TWT_STATUS_UNKNOWN_ERROR;
}

static QDF_STATUS extract_twt_del_dialog_comp_event_tlv(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wmi_twt_del_dialog_complete_event_param *params)
{
	WMI_TWT_DEL_DIALOG_COMPLETE_EVENTID_param_tlvs *param_buf;
	wmi_twt_del_dialog_complete_event_fixed_param *ev;

	param_buf = (WMI_TWT_DEL_DIALOG_COMPLETE_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("evt_buf is NULL");
		return QDF_STATUS_E_INVAL;
	}

	ev = param_buf->fixed_param;

	params->vdev_id = ev->vdev_id;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&ev->peer_macaddr, params->peer_macaddr);
	params->dialog_id = ev->dialog_id;
	params->status = wmi_get_converted_twt_del_dialog_status(ev->status);

	return QDF_STATUS_SUCCESS;
}

static enum WMI_HOST_PAUSE_TWT_STATUS
wmi_twt_pause_status_to_host_twt_status(WMI_PAUSE_TWT_STATUS_T status)
{
	switch (status) {
	case WMI_PAUSE_TWT_STATUS_OK:
		return WMI_HOST_PAUSE_TWT_STATUS_OK;
	case WMI_PAUSE_TWT_STATUS_DIALOG_ID_NOT_EXIST:
		return WMI_HOST_PAUSE_TWT_STATUS_DIALOG_ID_NOT_EXIST;
	case WMI_PAUSE_TWT_STATUS_INVALID_PARAM:
		return WMI_HOST_PAUSE_TWT_STATUS_INVALID_PARAM;
	case WMI_PAUSE_TWT_STATUS_DIALOG_ID_BUSY:
		return WMI_HOST_PAUSE_TWT_STATUS_DIALOG_ID_BUSY;
	case WMI_PAUSE_TWT_STATUS_NO_RESOURCE:
		return WMI_HOST_PAUSE_TWT_STATUS_NO_RESOURCE;
	case WMI_PAUSE_TWT_STATUS_NO_ACK:
		return WMI_HOST_PAUSE_TWT_STATUS_NO_ACK;
	case WMI_PAUSE_TWT_STATUS_UNKNOWN_ERROR:
		return WMI_HOST_PAUSE_TWT_STATUS_UNKNOWN_ERROR;
	case WMI_PAUSE_TWT_STATUS_ALREADY_PAUSED:
		return WMI_HOST_PAUSE_TWT_STATUS_ALREADY_PAUSED;
	case WMI_PAUSE_TWT_STATUS_CHAN_SW_IN_PROGRESS:
		return WMI_HOST_PAUSE_TWT_STATUS_CHAN_SW_IN_PROGRESS;
	case WMI_PAUSE_TWT_STATUS_ROAM_IN_PROGRESS:
		return WMI_HOST_PAUSE_TWT_STATUS_ROAM_IN_PROGRESS;
	case WMI_PAUSE_TWT_STATUS_SCAN_IN_PROGRESS:
		return WMI_HOST_PAUSE_TWT_STATUS_SCAN_IN_PROGRESS;
	default:
		return WMI_HOST_PAUSE_TWT_STATUS_UNKNOWN_ERROR;
	}
}

static QDF_STATUS extract_twt_pause_dialog_comp_event_tlv(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wmi_twt_pause_dialog_complete_event_param *params)
{
	WMI_TWT_PAUSE_DIALOG_COMPLETE_EVENTID_param_tlvs *param_buf;
	wmi_twt_pause_dialog_complete_event_fixed_param *ev;

	param_buf = (WMI_TWT_PAUSE_DIALOG_COMPLETE_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("evt_buf is NULL");
		return QDF_STATUS_E_INVAL;
	}

	ev = param_buf->fixed_param;

	params->vdev_id = ev->vdev_id;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&ev->peer_macaddr, params->peer_macaddr);
	params->status = wmi_twt_pause_status_to_host_twt_status(ev->status);
	params->dialog_id = ev->dialog_id;

	return QDF_STATUS_SUCCESS;
}

static enum WMI_HOST_NUDGE_TWT_STATUS
wmi_twt_nudge_status_to_host_twt_status(WMI_TWT_NUDGE_STATUS_T status)
{
	switch (status) {
	case WMI_NUDGE_TWT_STATUS_OK:
		return WMI_HOST_NUDGE_TWT_STATUS_OK;
	case WMI_NUDGE_TWT_STATUS_DIALOG_ID_NOT_EXIST:
		return WMI_HOST_NUDGE_TWT_STATUS_DIALOG_ID_NOT_EXIST;
	case WMI_NUDGE_TWT_STATUS_INVALID_PARAM:
		return WMI_HOST_NUDGE_TWT_STATUS_INVALID_PARAM;
	case WMI_NUDGE_TWT_STATUS_DIALOG_ID_BUSY:
		return WMI_HOST_NUDGE_TWT_STATUS_DIALOG_ID_BUSY;
	case WMI_NUDGE_TWT_STATUS_NO_RESOURCE:
		return WMI_HOST_NUDGE_TWT_STATUS_NO_RESOURCE;
	case WMI_NUDGE_TWT_STATUS_NO_ACK:
		return WMI_HOST_NUDGE_TWT_STATUS_NO_ACK;
	case WMI_NUDGE_TWT_STATUS_UNKNOWN_ERROR:
		return WMI_HOST_NUDGE_TWT_STATUS_UNKNOWN_ERROR;
	case WMI_NUDGE_TWT_STATUS_CHAN_SW_IN_PROGRESS:
		return WMI_HOST_NUDGE_TWT_STATUS_CHAN_SW_IN_PROGRESS;
	case WMI_NUDGE_TWT_STATUS_ROAM_IN_PROGRESS:
		return WMI_HOST_NUDGE_TWT_STATUS_ROAM_IN_PROGRESS;
	case WMI_NUDGE_TWT_STATUS_SCAN_IN_PROGRESS:
		return WMI_HOST_NUDGE_TWT_STATUS_SCAN_IN_PROGRESS;
	default:
		return WMI_HOST_NUDGE_TWT_STATUS_UNKNOWN_ERROR;
	}
}

static QDF_STATUS extract_twt_nudge_dialog_comp_event_tlv(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wmi_twt_nudge_dialog_complete_event_param *params)
{
	WMI_TWT_NUDGE_DIALOG_COMPLETE_EVENTID_param_tlvs *param_buf;
	wmi_twt_nudge_dialog_complete_event_fixed_param *ev;

	param_buf = (WMI_TWT_NUDGE_DIALOG_COMPLETE_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("evt_buf is NULL");
		return QDF_STATUS_E_INVAL;
	}

	ev = param_buf->fixed_param;

	params->vdev_id = ev->vdev_id;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&ev->peer_macaddr, params->peer_macaddr);
	params->status = wmi_twt_nudge_status_to_host_twt_status(ev->status);
	params->dialog_id = ev->dialog_id;
	params->next_twt_tsf_us_lo = ev->sp_tsf_us_lo;
	params->next_twt_tsf_us_hi = ev->sp_tsf_us_hi;

	wmi_debug("vdev_id: %d dialog_id: %d tsf hi : %x tsf lo: %x",
		  params->vdev_id, params->dialog_id,
		  params->next_twt_tsf_us_hi, params->next_twt_tsf_us_lo);

	return QDF_STATUS_SUCCESS;
}

static enum WMI_HOST_RESUME_TWT_STATUS
wmi_get_converted_twt_resume_dialog_status(WMI_RESUME_TWT_STATUS_T tgt_status)
{
	switch (tgt_status) {
	case WMI_RESUME_TWT_STATUS_OK:
		return WMI_HOST_RESUME_TWT_STATUS_OK;
	case WMI_RESUME_TWT_STATUS_DIALOG_ID_NOT_EXIST:
		return WMI_HOST_RESUME_TWT_STATUS_DIALOG_ID_NOT_EXIST;
	case WMI_RESUME_TWT_STATUS_INVALID_PARAM:
		return WMI_HOST_RESUME_TWT_STATUS_INVALID_PARAM;
	case WMI_RESUME_TWT_STATUS_DIALOG_ID_BUSY:
		return WMI_HOST_RESUME_TWT_STATUS_DIALOG_ID_BUSY;
	case WMI_RESUME_TWT_STATUS_NOT_PAUSED:
		return WMI_HOST_RESUME_TWT_STATUS_NOT_PAUSED;
	case WMI_RESUME_TWT_STATUS_NO_RESOURCE:
		return WMI_HOST_RESUME_TWT_STATUS_NO_RESOURCE;
	case WMI_RESUME_TWT_STATUS_NO_ACK:
		return WMI_HOST_RESUME_TWT_STATUS_NO_ACK;
	case WMI_RESUME_TWT_STATUS_CHAN_SW_IN_PROGRESS:
		return WMI_HOST_RESUME_TWT_STATUS_CHAN_SW_IN_PROGRESS;
	case WMI_RESUME_TWT_STATUS_ROAM_IN_PROGRESS:
		return WMI_HOST_RESUME_TWT_STATUS_ROAM_IN_PROGRESS;
	case WMI_RESUME_TWT_STATUS_SCAN_IN_PROGRESS:
		return WMI_HOST_RESUME_TWT_STATUS_SCAN_IN_PROGRESS;
	default:
		return WMI_HOST_RESUME_TWT_STATUS_UNKNOWN_ERROR;
	}
}

static QDF_STATUS extract_twt_resume_dialog_comp_event_tlv(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wmi_twt_resume_dialog_complete_event_param *params)
{
	WMI_TWT_RESUME_DIALOG_COMPLETE_EVENTID_param_tlvs *param_buf;
	wmi_twt_resume_dialog_complete_event_fixed_param *ev;

	param_buf =
		(WMI_TWT_RESUME_DIALOG_COMPLETE_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("evt_buf is NULL");
		return QDF_STATUS_E_INVAL;
	}

	ev = param_buf->fixed_param;

	params->vdev_id = ev->vdev_id;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&ev->peer_macaddr, params->peer_macaddr);
	params->status = wmi_get_converted_twt_resume_dialog_status(ev->status);
	params->dialog_id = ev->dialog_id;

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS extract_twt_notify_event_tlv(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wmi_twt_notify_event_param *params)
{
	WMI_TWT_NOTIFY_EVENTID_param_tlvs *param_buf;
	wmi_twt_notify_event_fixed_param *ev;

	param_buf =
		(WMI_TWT_NOTIFY_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("evt_buf is NULL");
		return QDF_STATUS_E_INVAL;
	}

	ev = param_buf->fixed_param;

	params->vdev_id = ev->vdev_id;

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_SUPPORT_BCAST_TWT
static QDF_STATUS
extract_twt_btwt_invite_sta_comp_event_tlv(
					   wmi_unified_t wmi_handle,
					   uint8_t *evt_buf,
					   struct
					   wmi_twt_btwt_invite_sta_complete_event_param
					   *params)
{
	WMI_TWT_BTWT_INVITE_STA_COMPLETE_EVENTID_param_tlvs *param_buf;
	wmi_twt_btwt_invite_sta_complete_event_fixed_param *ev;

	param_buf =
		(WMI_TWT_BTWT_INVITE_STA_COMPLETE_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("evt_buf is NULL");
		return QDF_STATUS_E_INVAL;
	}

	ev = param_buf->fixed_param;

	params->vdev_id = ev->vdev_id;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&ev->peer_macaddr, params->peer_macaddr);
	params->status = ev->status;
	params->dialog_id = ev->dialog_id;

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
extract_twt_btwt_remove_sta_comp_event_tlv(
					   wmi_unified_t wmi_handle,
					   uint8_t *evt_buf,
					   struct
					   wmi_twt_btwt_remove_sta_complete_event_param
					   *params)
{
	WMI_TWT_BTWT_REMOVE_STA_COMPLETE_EVENTID_param_tlvs *param_buf;
	wmi_twt_btwt_remove_sta_complete_event_fixed_param *ev;

	param_buf =
		(WMI_TWT_BTWT_REMOVE_STA_COMPLETE_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("evt_buf is NULL");
		return QDF_STATUS_E_INVAL;
	}

	ev = param_buf->fixed_param;

	params->vdev_id = ev->vdev_id;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&ev->peer_macaddr, params->peer_macaddr);
	params->status = ev->status;
	params->dialog_id = ev->dialog_id;

	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef WLAN_SUPPORT_BCAST_TWT
static void
wmi_twt_attach_bcast_twt_tlv(struct wmi_ops *ops)
{
	ops->send_twt_btwt_invite_sta_cmd = send_twt_btwt_invite_sta_cmd_tlv;
	ops->send_twt_btwt_remove_sta_cmd = send_twt_btwt_remove_sta_cmd_tlv;
	ops->extract_twt_btwt_invite_sta_comp_event =
				extract_twt_btwt_invite_sta_comp_event_tlv;
	ops->extract_twt_btwt_remove_sta_comp_event =
				extract_twt_btwt_remove_sta_comp_event_tlv;
}
#else
static void
wmi_twt_attach_bcast_twt_tlv(struct wmi_ops *ops)
{
}
#endif

static QDF_STATUS
extract_twt_session_stats_event_tlv(wmi_unified_t wmi_handle,
				    uint8_t *evt_buf,
				    struct wmi_twt_session_stats_event_param
				    *params)
{
	WMI_TWT_SESSION_STATS_EVENTID_param_tlvs *param_buf;
	wmi_pdev_twt_session_stats_event_fixed_param *ev;

	param_buf =
		(WMI_TWT_SESSION_STATS_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("evt_buf is NULL");
		return QDF_STATUS_E_INVAL;
	}

	ev = param_buf->fixed_param;
	params->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
							wmi_handle,
							ev->pdev_id);
	params->num_sessions = param_buf->num_twt_sessions;

	wmi_debug("pdev_id=%d, num of TWT sessions=%d",
		 params->pdev_id, params->num_sessions);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
extract_twt_session_stats_event_data(wmi_unified_t wmi_handle,
				     uint8_t *evt_buf,
				     struct wmi_twt_session_stats_event_param
				     *params,
				     struct wmi_host_twt_session_stats_info
				     *session,
				     uint32_t idx)
{
	WMI_TWT_SESSION_STATS_EVENTID_param_tlvs *param_buf;
	wmi_twt_session_stats_info *twt_session;
	uint32_t flags;
	wmi_mac_addr *m1;
	uint8_t *m2;

	param_buf =
		(WMI_TWT_SESSION_STATS_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("evt_buf is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (idx >= param_buf->num_twt_sessions) {
		wmi_err("wrong idx, idx=%d, num_sessions=%d",
			 idx, param_buf->num_twt_sessions);
		return QDF_STATUS_E_INVAL;
	}

	twt_session = &param_buf->twt_sessions[idx];

	session->vdev_id = twt_session->vdev_id;
	m1 = &twt_session->peer_mac;
	m2 = session->peer_mac;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(m1, m2);
	session->event_type = twt_session->event_type;
	flags = twt_session->flow_id_flags;
	session->flow_id = WMI_TWT_SESSION_FLAG_FLOW_ID_GET(flags);
	session->bcast = WMI_TWT_SESSION_FLAG_BCAST_TWT_GET(flags);
	session->trig = WMI_TWT_SESSION_FLAG_TRIGGER_TWT_GET(flags);
	session->announ = WMI_TWT_SESSION_FLAG_ANNOUN_TWT_GET(flags);
	session->protection = WMI_TWT_SESSION_FLAG_TWT_PROTECTION_GET(flags);
	session->info_frame_disabled =
			WMI_TWT_SESSION_FLAG_TWT_INFO_FRAME_DISABLED_GET(flags);
	session->pm_responder_bit =
			WMI_TWT_SESSION_FLAG_TWT_PM_RESPONDER_MODE_GET(flags);
	session->pm_responder_bit_valid =
		WMI_TWT_SESSION_FLAG_TWT_PM_RESPONDER_MODE_VALID_GET(flags);
	session->dialog_id = twt_session->dialog_id;
	session->wake_dura_us = twt_session->wake_dura_us;
	session->wake_intvl_us = twt_session->wake_intvl_us;
	session->sp_offset_us = twt_session->sp_offset_us;
	session->sp_tsf_us_lo = twt_session->sp_tsf_us_lo;
	session->sp_tsf_us_hi = twt_session->sp_tsf_us_hi;
	wmi_debug("type=%d id=%d bcast=%d trig=%d announ=%d diagid=%d wake_dur=%ul wake_int=%ul offset=%ul",
		 session->event_type, session->flow_id,
		 session->bcast, session->trig,
		 session->announ, session->dialog_id, session->wake_dura_us,
		 session->wake_intvl_us, session->sp_offset_us);
	wmi_debug("resp_pm_valid=%d resp_pm=%d",
		  session->pm_responder_bit_valid, session->pm_responder_bit);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS extract_twt_cap_service_ready_ext2_tlv(
				wmi_unified_t wmi_handle, uint8_t *event,
				struct wmi_twt_cap_bitmap_params *var)
{
	WMI_SERVICE_READY_EXT2_EVENTID_param_tlvs *param_buf;
	wmi_twt_caps_params *twt_caps;

	param_buf = (WMI_SERVICE_READY_EXT2_EVENTID_param_tlvs *)event;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	twt_caps = param_buf->twt_caps;
	if (!twt_caps)
		return QDF_STATUS_E_INVAL;

	var->twt_ack_support_cap = WMI_GET_BITS(twt_caps->twt_capability_bitmap,
						0, 1);

	return QDF_STATUS_SUCCESS;
}

static enum WMI_HOST_TWT_CMD_FOR_ACK_EVENT
wmi_get_converted_twt_command_for_ack_event(WMI_CMD_ID tgt_cmd)
{
	switch (tgt_cmd) {
	case WMI_TWT_ADD_DIALOG_CMDID:
		return WMI_HOST_TWT_ADD_DIALOG_CMDID;
	case WMI_TWT_DEL_DIALOG_CMDID:
		return WMI_HOST_TWT_DEL_DIALOG_CMDID;
	case WMI_TWT_PAUSE_DIALOG_CMDID:
		return WMI_HOST_TWT_PAUSE_DIALOG_CMDID;
	case WMI_TWT_RESUME_DIALOG_CMDID:
		return WMI_HOST_TWT_RESUME_DIALOG_CMDID;
	case WMI_TWT_NUDGE_DIALOG_CMDID:
		return WMI_HOST_TWT_NUDGE_DIALOG_CMDID;
	default:
		return WMI_HOST_TWT_UNKNOWN_CMDID;
	}
}

static QDF_STATUS
extract_twt_ack_comp_event_tlv(wmi_unified_t wmi_handle,
			       uint8_t *evt_buf,
			       struct wmi_twt_ack_complete_event_param *var)
{
	WMI_TWT_ACK_EVENTID_param_tlvs *param_buf;
	wmi_twt_ack_event_fixed_param *ack_event;

	param_buf = (WMI_TWT_ACK_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("evt_buf is NULL");
		return QDF_STATUS_E_INVAL;
	}

	ack_event = param_buf->fixed_param;

	var->vdev_id = ack_event->vdev_id;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&ack_event->peer_macaddr,
				   var->peer_macaddr.bytes);
	var->dialog_id = ack_event->dialog_id;
	var->twt_cmd_ack = wmi_get_converted_twt_command_for_ack_event(
						ack_event->twt_cmd);

	switch (ack_event->twt_cmd) {
	case WMI_TWT_ADD_DIALOG_CMDID:
		var->status = wmi_get_converted_twt_add_dialog_status(
						ack_event->status);
		break;
	case WMI_TWT_DEL_DIALOG_CMDID:
		var->status = wmi_get_converted_twt_del_dialog_status(
						ack_event->status);
		break;
	case WMI_TWT_PAUSE_DIALOG_CMDID:
		var->status = wmi_twt_pause_status_to_host_twt_status(
						ack_event->status);
		break;
	case WMI_TWT_RESUME_DIALOG_CMDID:
		var->status = wmi_get_converted_twt_resume_dialog_status(
						ack_event->status);
		break;
	case WMI_TWT_NUDGE_DIALOG_CMDID:
		var->status = wmi_twt_nudge_status_to_host_twt_status(
						ack_event->status);
		break;
	default:
		break;
	}
	return QDF_STATUS_SUCCESS;
}
#endif

void wmi_twt_attach_tlv(wmi_unified_t wmi_handle)
{
	struct wmi_ops *ops = wmi_handle->ops;

	ops->send_twt_enable_cmd = send_twt_enable_cmd_tlv;
	ops->send_twt_disable_cmd = send_twt_disable_cmd_tlv;
	ops->send_twt_add_dialog_cmd = send_twt_add_dialog_cmd_tlv;
	ops->send_twt_del_dialog_cmd = send_twt_del_dialog_cmd_tlv;
	ops->send_twt_pause_dialog_cmd = send_twt_pause_dialog_cmd_tlv;
	ops->send_twt_nudge_dialog_cmd = send_twt_nudge_dialog_cmd_tlv;
	ops->send_twt_resume_dialog_cmd = send_twt_resume_dialog_cmd_tlv;
	ops->extract_twt_enable_comp_event = extract_twt_enable_comp_event_tlv;
	ops->extract_twt_disable_comp_event =
				extract_twt_disable_comp_event_tlv;
	ops->extract_twt_add_dialog_comp_event =
				extract_twt_add_dialog_comp_event_tlv;
	ops->extract_twt_add_dialog_comp_additional_params =
			extract_twt_add_dialog_comp_additional_parameters;
	ops->extract_twt_del_dialog_comp_event =
				extract_twt_del_dialog_comp_event_tlv;
	ops->extract_twt_pause_dialog_comp_event =
				extract_twt_pause_dialog_comp_event_tlv;
	ops->extract_twt_nudge_dialog_comp_event =
				extract_twt_nudge_dialog_comp_event_tlv;
	ops->extract_twt_resume_dialog_comp_event =
				extract_twt_resume_dialog_comp_event_tlv;
	ops->extract_twt_session_stats_event =
				extract_twt_session_stats_event_tlv;
	ops->extract_twt_session_stats_data =
				extract_twt_session_stats_event_data;
	ops->extract_twt_notify_event =
				extract_twt_notify_event_tlv;
	ops->extract_twt_cap_service_ready_ext2 =
				extract_twt_cap_service_ready_ext2_tlv,
	ops->extract_twt_ack_comp_event = extract_twt_ack_comp_event_tlv;
	wmi_twt_attach_bcast_twt_tlv(ops);
}
