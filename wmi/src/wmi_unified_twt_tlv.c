/*
 * Copyright (c) 2018-2020 The Linux Foundation. All rights reserved.
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

	return;
}
#else
static void
twt_add_dialog_set_bcast_twt_params(struct wmi_twt_add_dialog_param *params,
                wmi_twt_add_dialog_cmd_fixed_param *cmd)
{
	return;
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
	return;
}
#else
static void
twt_del_dialog_set_bcast_twt_params(struct wmi_twt_del_dialog_param *params,
                wmi_twt_del_dialog_cmd_fixed_param *cmd)
{
	return;
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
		wmi_handle->ops->convert_pdev_id_target_to_host(ev->pdev_id);
	params->status = ev->status;
#endif

	return QDF_STATUS_SUCCESS;
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
	params->status = ev->status;
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

	/*
	 * For Alternate values from AP, Firmware sends additional params
	 * with WMI_HOST_ADD_TWT_STATUS_DENIED
	 */
	if (ev->status != WMI_HOST_ADD_TWT_STATUS_OK &&
	    ev->status != WMI_HOST_ADD_TWT_STATUS_DENIED) {
		wmi_err("Status of add dialog complete is not success");
		return QDF_STATUS_E_INVAL;
	}

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
	params->status = ev->status;

	return QDF_STATUS_SUCCESS;
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
	params->status = ev->status;
	params->dialog_id = ev->dialog_id;

	return QDF_STATUS_SUCCESS;
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
	params->status = ev->status;
	params->dialog_id = ev->dialog_id;

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

	return;
}
#else
static void
wmi_twt_attach_bcast_twt_tlv(struct wmi_ops *ops)
{
	return;
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

void wmi_twt_attach_tlv(wmi_unified_t wmi_handle)
{
	struct wmi_ops *ops = wmi_handle->ops;

	ops->send_twt_enable_cmd = send_twt_enable_cmd_tlv;
	ops->send_twt_disable_cmd = send_twt_disable_cmd_tlv;
	ops->send_twt_add_dialog_cmd = send_twt_add_dialog_cmd_tlv;
	ops->send_twt_del_dialog_cmd = send_twt_del_dialog_cmd_tlv;
	ops->send_twt_pause_dialog_cmd = send_twt_pause_dialog_cmd_tlv;
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
	ops->extract_twt_resume_dialog_comp_event =
				extract_twt_resume_dialog_comp_event_tlv;
	ops->extract_twt_session_stats_event =
				extract_twt_session_stats_event_tlv;
	ops->extract_twt_session_stats_data =
				extract_twt_session_stats_event_data;

	wmi_twt_attach_bcast_twt_tlv(ops);
}
