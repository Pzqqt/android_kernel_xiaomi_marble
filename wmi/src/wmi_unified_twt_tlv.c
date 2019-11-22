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
		WMI_LOGE("Failed to allocate memory");
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
	cmd->flags =                        params->flags;

	status = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
			WMI_TWT_ENABLE_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("Failed to send WMI_TWT_ENABLE_CMDID");
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
		WMI_LOGE("Failed to allocate memory");
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

	status = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
			WMI_TWT_DISABLE_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("Failed to send WMI_TWT_DISABLE_CMDID");
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
		WMI_LOGE("Failed to allocate memory");
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
		WMI_LOGE("Failed to send WMI_TWT_ADD_DIALOG_CMDID");
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
		WMI_LOGE("Failed to allocate memory");
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
		WMI_LOGE("Failed to send WMI_TWT_DEL_DIALOG_CMDID");
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
		WMI_LOGE("Failed to allocate memory");
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
		WMI_LOGE("Failed to send WMI_TWT_PAUSE_DIALOG_CMDID");
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
		WMI_LOGE("Failed to allocate memory");
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
		WMI_LOGE("Failed to send WMI_TWT_RESUME_DIALOG_CMDID");
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
		WMI_LOGE("Failed to allocate memory");
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
		WMI_LOGE("Failed to allocate memory");
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
		WMI_LOGE("evt_buf is NULL");
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
		WMI_LOGE("evt_buf is NULL");
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

static QDF_STATUS extract_twt_add_dialog_comp_event_tlv(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wmi_twt_add_dialog_complete_event_param *params)
{
	WMI_TWT_ADD_DIALOG_COMPLETE_EVENTID_param_tlvs *param_buf;
	wmi_twt_add_dialog_complete_event_fixed_param *ev;

	param_buf = (WMI_TWT_ADD_DIALOG_COMPLETE_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		WMI_LOGE("evt_buf is NULL");
		return QDF_STATUS_E_INVAL;
	}

	ev = param_buf->fixed_param;

	params->vdev_id = ev->vdev_id;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&ev->peer_macaddr, params->peer_macaddr);
	params->status = ev->status;
	params->dialog_id = ev->dialog_id;

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
		WMI_LOGE("evt_buf is NULL");
		return QDF_STATUS_E_INVAL;
	}

	ev = param_buf->fixed_param;

	params->vdev_id = ev->vdev_id;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&ev->peer_macaddr, params->peer_macaddr);
	params->dialog_id = ev->dialog_id;

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
		WMI_LOGE("evt_buf is NULL");
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
		WMI_LOGE("evt_buf is NULL");
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
		WMI_LOGE("evt_buf is NULL");
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
		WMI_LOGE("evt_buf is NULL");
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
	ops->extract_twt_del_dialog_comp_event =
				extract_twt_del_dialog_comp_event_tlv;
	ops->extract_twt_pause_dialog_comp_event =
				extract_twt_pause_dialog_comp_event_tlv;
	ops->extract_twt_resume_dialog_comp_event =
				extract_twt_resume_dialog_comp_event_tlv;

	wmi_twt_attach_bcast_twt_tlv(ops);
}
