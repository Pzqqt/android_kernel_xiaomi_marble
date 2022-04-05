/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: osif_twt_ext_rsp.c
 *
 */
#include <wlan_cfg80211.h>
#include <osif_twt_ext_req.h>
#include <osif_twt_rsp.h>
#include <osif_twt_ext_rsp.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_osif_priv.h>
#include <wlan_osif_request_manager.h>
#include <wlan_cm_api.h>
#include <wlan_twt_ucfg_api.h>
#include <wlan_cm_ucfg_api.h>
#include <wlan_reg_ucfg_api.h>
#include <wlan_twt_ucfg_ext_api.h>
#include <wlan_twt_ucfg_ext_cfg.h>
#include <wlan_cp_stats_ucfg_api.h>

/**
 * osif_twt_get_setup_event_len() - Calculates the length of twt
 * setup nl response
 * @additional_params_present: if true, then length required for
 * fixed and additional parameters is returned. if false,
 * then length required for fixed parameters is returned.
 *
 * Return: Length of twt setup nl response
 */
static
uint32_t osif_twt_get_setup_event_len(bool additional_params_present)
{
	uint32_t len = 0;

	len += NLMSG_HDRLEN;

	/* Length of attribute QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_PARAMS */
	len += NLA_HDRLEN;

	/* QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_OPERATION */
	len += nla_total_size(sizeof(u8));

	/* QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_ID */
	len += nla_total_size(sizeof(u8));
	/* QCA_WLAN_VENDOR_ATTR_TWT_SETUP_STATUS */
	len += nla_total_size(sizeof(u8));

	if (!additional_params_present)
		return len;

	/* QCA_WLAN_VENDOR_ATTR_TWT_SETUP_RESP_TYPE */
	len += nla_total_size(sizeof(u8));
	/*QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_TYPE*/
	len += nla_total_size(sizeof(u8));
	/*QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_DURATION*/
	len += nla_total_size(sizeof(u32));
	/*QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_INTVL_MANTISSA*/
	len += nla_total_size(sizeof(u32));
	/*QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_INTVL_EXP*/
	len += nla_total_size(sizeof(u8));
	/*QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_TIME_TSF*/
	len += nla_total_size(sizeof(u64));
	/*QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_TIME*/
	len += nla_total_size(sizeof(u32));
	/*QCA_WLAN_VENDOR_ATTR_TWT_SETUP_TRIGGER*/
	len += nla_total_size(sizeof(u8));
	/*QCA_WLAN_VENDOR_ATTR_TWT_SETUP_PROTECTION*/
	len += nla_total_size(sizeof(u8));
	/*QCA_WLAN_VENDOR_ATTR_TWT_SETUP_BCAST*/
	len += nla_total_size(sizeof(u8));
	/*QCA_WLAN_VENDOR_ATTR_TWT_SETUP_TWT_INFO_ENABLED*/
	len += nla_total_size(sizeof(u8));
	/*QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAC_ADDR*/
	len += nla_total_size(QDF_MAC_ADDR_SIZE);

	return len;
}

/**
 * osif_twt_get_event_len() - calculate length of skb
 * required for sending twt terminate, pause and resume
 * command responses.
 *
 * Return: length of skb
 */
static uint32_t osif_twt_get_event_len(void)
{
	uint32_t len = 0;

	len += NLMSG_HDRLEN;
	/* QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_ID */
	len += nla_total_size(sizeof(u8));
	/* QCA_WLAN_VENDOR_ATTR_TWT_SETUP_STATUS */
	len += nla_total_size(sizeof(u8));
	/* QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAC_ADDR*/
	len += nla_total_size(QDF_MAC_ADDR_SIZE);

	return len;
}

/**
 * twt_add_status_to_vendor_twt_status() - convert from
 * HOST_ADD_TWT_STATUS to qca_wlan_vendor_twt_status
 * @status: HOST_ADD_TWT_STATUS value from firmare
 *
 * Return: qca_wlan_vendor_twt_status values corresponding
 * to HOST_ADD_TWT_STATUS.
 */
static enum qca_wlan_vendor_twt_status
twt_add_status_to_vendor_twt_status(enum HOST_ADD_TWT_STATUS status)
{
	switch (status) {
	case HOST_ADD_TWT_STATUS_OK:
		return QCA_WLAN_VENDOR_TWT_STATUS_OK;
	case HOST_ADD_TWT_STATUS_TWT_NOT_ENABLED:
		return QCA_WLAN_VENDOR_TWT_STATUS_TWT_NOT_ENABLED;
	case HOST_ADD_TWT_STATUS_USED_DIALOG_ID:
		return QCA_WLAN_VENDOR_TWT_STATUS_USED_DIALOG_ID;
	case HOST_ADD_TWT_STATUS_INVALID_PARAM:
		return QCA_WLAN_VENDOR_TWT_STATUS_INVALID_PARAM;
	case HOST_ADD_TWT_STATUS_NOT_READY:
		return QCA_WLAN_VENDOR_TWT_STATUS_NOT_READY;
	case HOST_ADD_TWT_STATUS_NO_RESOURCE:
		return QCA_WLAN_VENDOR_TWT_STATUS_NO_RESOURCE;
	case HOST_ADD_TWT_STATUS_NO_ACK:
		return QCA_WLAN_VENDOR_TWT_STATUS_NO_ACK;
	case HOST_ADD_TWT_STATUS_NO_RESPONSE:
		return QCA_WLAN_VENDOR_TWT_STATUS_NO_RESPONSE;
	case HOST_ADD_TWT_STATUS_DENIED:
		return QCA_WLAN_VENDOR_TWT_STATUS_DENIED;
	case HOST_ADD_TWT_STATUS_UNKNOWN_ERROR:
		return QCA_WLAN_VENDOR_TWT_STATUS_UNKNOWN_ERROR;
	case HOST_ADD_TWT_STATUS_AP_PARAMS_NOT_IN_RANGE:
		return QCA_WLAN_VENDOR_TWT_STATUS_PARAMS_NOT_IN_RANGE;
	case HOST_ADD_TWT_STATUS_AP_IE_VALIDATION_FAILED:
		return QCA_WLAN_VENDOR_TWT_STATUS_IE_INVALID;
	case HOST_ADD_TWT_STATUS_ROAM_IN_PROGRESS:
		return QCA_WLAN_VENDOR_TWT_STATUS_ROAMING_IN_PROGRESS;
	case HOST_ADD_TWT_STATUS_CHAN_SW_IN_PROGRESS:
		return QCA_WLAN_VENDOR_TWT_STATUS_CHANNEL_SWITCH_IN_PROGRESS;
	case HOST_ADD_TWT_STATUS_SCAN_IN_PROGRESS:
		return QCA_WLAN_VENDOR_TWT_STATUS_SCAN_IN_PROGRESS;
	default:
		return QCA_WLAN_VENDOR_TWT_STATUS_UNKNOWN_ERROR;
	}
}

/**
 * twt_del_status_to_vendor_twt_status() - convert from
 * HOST_DEL_TWT_STATUS to qca_wlan_vendor_twt_status
 * @status: HOST_DEL_TWT_STATUS value from firmare
 *
 * Return: qca_wlan_vendor_twt_status values corresponding
 * to HOST_DEL_TWT_STATUS.
 */
static enum qca_wlan_vendor_twt_status
twt_del_status_to_vendor_twt_status(enum HOST_TWT_DEL_STATUS status)
{
	switch (status) {
	case HOST_TWT_DEL_STATUS_OK:
		return QCA_WLAN_VENDOR_TWT_STATUS_OK;
	case HOST_TWT_DEL_STATUS_DIALOG_ID_NOT_EXIST:
		return QCA_WLAN_VENDOR_TWT_STATUS_SESSION_NOT_EXIST;
	case HOST_TWT_DEL_STATUS_INVALID_PARAM:
		return QCA_WLAN_VENDOR_TWT_STATUS_INVALID_PARAM;
	case HOST_TWT_DEL_STATUS_DIALOG_ID_BUSY:
		return QCA_WLAN_VENDOR_TWT_STATUS_SESSION_BUSY;
	case HOST_TWT_DEL_STATUS_NO_RESOURCE:
		return QCA_WLAN_VENDOR_TWT_STATUS_NO_RESOURCE;
	case HOST_TWT_DEL_STATUS_NO_ACK:
		return QCA_WLAN_VENDOR_TWT_STATUS_NO_ACK;
	case HOST_TWT_DEL_STATUS_UNKNOWN_ERROR:
		return QCA_WLAN_VENDOR_TWT_STATUS_UNKNOWN_ERROR;
	case HOST_TWT_DEL_STATUS_PEER_INIT_TEARDOWN:
		return QCA_WLAN_VENDOR_TWT_STATUS_PEER_INITIATED_TERMINATE;
	case HOST_TWT_DEL_STATUS_ROAMING:
		return QCA_WLAN_VENDOR_TWT_STATUS_ROAM_INITIATED_TERMINATE;
	case HOST_TWT_DEL_STATUS_CONCURRENCY:
		return QCA_WLAN_VENDOR_TWT_STATUS_SCC_MCC_CONCURRENCY_TERMINATE;
	case HOST_TWT_DEL_STATUS_CHAN_SW_IN_PROGRESS:
		return QCA_WLAN_VENDOR_TWT_STATUS_CHANNEL_SWITCH_IN_PROGRESS;
	case HOST_TWT_DEL_STATUS_SCAN_IN_PROGRESS:
		return QCA_WLAN_VENDOR_TWT_STATUS_SCAN_IN_PROGRESS;
	case HOST_TWT_DEL_STATUS_PS_DISABLE_TEARDOWN:
		return QCA_WLAN_VENDOR_TWT_STATUS_POWER_SAVE_EXIT_TERMINATE;
	default:
		return QCA_WLAN_VENDOR_TWT_STATUS_UNKNOWN_ERROR;
	}
}

/**
 * twt_resume_status_to_vendor_twt_status() - convert from
 * HOST_TWT_RESUME_STATUS to qca_wlan_vendor_twt_status
 * @status: HOST_TWT_RESUME_STATUS value from firmware
 *
 * Return: qca_wlan_vendor_twt_status values corresponding
 * to the firmware failure status
 */
static int
twt_resume_status_to_vendor_twt_status(enum HOST_TWT_RESUME_STATUS status)
{
	switch (status) {
	case HOST_TWT_RESUME_STATUS_OK:
		return QCA_WLAN_VENDOR_TWT_STATUS_OK;
	case HOST_TWT_RESUME_STATUS_DIALOG_ID_NOT_EXIST:
		return QCA_WLAN_VENDOR_TWT_STATUS_SESSION_NOT_EXIST;
	case HOST_TWT_RESUME_STATUS_INVALID_PARAM:
		return QCA_WLAN_VENDOR_TWT_STATUS_INVALID_PARAM;
	case HOST_TWT_RESUME_STATUS_DIALOG_ID_BUSY:
		return QCA_WLAN_VENDOR_TWT_STATUS_SESSION_BUSY;
	case HOST_TWT_RESUME_STATUS_NOT_PAUSED:
		return QCA_WLAN_VENDOR_TWT_STATUS_NOT_SUSPENDED;
	case HOST_TWT_RESUME_STATUS_NO_RESOURCE:
		return QCA_WLAN_VENDOR_TWT_STATUS_NO_RESOURCE;
	case HOST_TWT_RESUME_STATUS_NO_ACK:
		return QCA_WLAN_VENDOR_TWT_STATUS_NO_ACK;
	case HOST_TWT_RESUME_STATUS_UNKNOWN_ERROR:
		return QCA_WLAN_VENDOR_TWT_STATUS_UNKNOWN_ERROR;
	case HOST_TWT_RESUME_STATUS_CHAN_SW_IN_PROGRESS:
		return QCA_WLAN_VENDOR_TWT_STATUS_CHANNEL_SWITCH_IN_PROGRESS;
	case HOST_TWT_RESUME_STATUS_ROAM_IN_PROGRESS:
		return QCA_WLAN_VENDOR_TWT_STATUS_ROAMING_IN_PROGRESS;
	case HOST_TWT_RESUME_STATUS_SCAN_IN_PROGRESS:
		return QCA_WLAN_VENDOR_TWT_STATUS_SCAN_IN_PROGRESS;
	default:
		return QCA_WLAN_VENDOR_TWT_STATUS_UNKNOWN_ERROR;
	}
}

/**
 * twt_nudge_status_to_vendor_twt_status() - convert from
 * HOST_TWT_NUDGE_STATUS to qca_wlan_vendor_twt_status
 * @status: HOST_TWT_NUDGE_STATUS value from firmware
 *
 * Return: qca_wlan_vendor_twt_status values corresponding
 * to the firmware failure status
 */
static int
twt_nudge_status_to_vendor_twt_status(enum HOST_TWT_NUDGE_STATUS status)
{
	switch (status) {
	case HOST_TWT_NUDGE_STATUS_OK:
		return QCA_WLAN_VENDOR_TWT_STATUS_OK;
	case HOST_TWT_NUDGE_STATUS_DIALOG_ID_NOT_EXIST:
		return QCA_WLAN_VENDOR_TWT_STATUS_SESSION_NOT_EXIST;
	case HOST_TWT_NUDGE_STATUS_INVALID_PARAM:
		return QCA_WLAN_VENDOR_TWT_STATUS_INVALID_PARAM;
	case HOST_TWT_NUDGE_STATUS_DIALOG_ID_BUSY:
		return QCA_WLAN_VENDOR_TWT_STATUS_SESSION_BUSY;
	case HOST_TWT_NUDGE_STATUS_NO_RESOURCE:
		return QCA_WLAN_VENDOR_TWT_STATUS_NO_RESOURCE;
	case HOST_TWT_NUDGE_STATUS_NO_ACK:
		return QCA_WLAN_VENDOR_TWT_STATUS_NO_ACK;
	case HOST_TWT_NUDGE_STATUS_UNKNOWN_ERROR:
		return QCA_WLAN_VENDOR_TWT_STATUS_UNKNOWN_ERROR;
	case HOST_TWT_NUDGE_STATUS_CHAN_SW_IN_PROGRESS:
		return QCA_WLAN_VENDOR_TWT_STATUS_CHANNEL_SWITCH_IN_PROGRESS;
	default:
		return QCA_WLAN_VENDOR_TWT_STATUS_UNKNOWN_ERROR;
	}
}

/**
 * twt_add_cmd_to_vendor_twt_resp_type() - convert from
 * HOST_TWT_COMMAND to qca_wlan_vendor_twt_setup_resp_type
 * @status: HOST_TWT_COMMAND value from firmare
 *
 * Return: qca_wlan_vendor_twt_setup_resp_type values for valid
 * HOST_TWT_COMMAND value and -EINVAL for invalid value
 */
static
int twt_add_cmd_to_vendor_twt_resp_type(enum HOST_TWT_COMMAND type)
{
	switch (type) {
	case HOST_TWT_COMMAND_ACCEPT_TWT:
		return QCA_WLAN_VENDOR_TWT_RESP_ACCEPT;
	case HOST_TWT_COMMAND_ALTERNATE_TWT:
		return QCA_WLAN_VENDOR_TWT_RESP_ALTERNATE;
	case HOST_TWT_COMMAND_DICTATE_TWT:
		return QCA_WLAN_VENDOR_TWT_RESP_DICTATE;
	case HOST_TWT_COMMAND_REJECT_TWT:
		return QCA_WLAN_VENDOR_TWT_RESP_REJECT;
	default:
		return -EINVAL;
	}
}

/**
 * osif_twt_setup_pack_resp_nlmsg() - pack nlmsg response for setup
 * @reply_skb: pointer to the response skb structure
 * @event: twt event buffer with firmware response
 *
 * Pack the nl response with parameters and additional parameters
 * received from firmware.
 * Firmware sends additional parameters only for 2 conditions
 * 1) TWT Negotiation is accepted by AP - Firmware sends
 * QCA_WLAN_VENDOR_TWT_STATUS_OK with appropriate response type
 * in additional parameters
 * 2) AP has proposed Alternate values - In this case firmware sends
 * QCA_WLAN_VENDOR_TWT_STATUS_DENIED with appropriate response type
 * in additional parameters
 *
 * Return: QDF_STATUS_SUCCESS on Success, other QDF_STATUS error codes
 * on failure
 */
static QDF_STATUS
osif_twt_setup_pack_resp_nlmsg(struct sk_buff *reply_skb,
			       struct twt_add_dialog_complete_event *event)
{
	struct nlattr *config_attr;
	uint64_t sp_offset_tsf;
	enum qca_wlan_vendor_twt_status vendor_status;
	int response_type, attr;
	uint32_t wake_duration;
	uint32_t wake_intvl_mantis_us, wake_intvl_mantis_tu;

	if (nla_put_u8(reply_skb, QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_OPERATION,
		       QCA_WLAN_TWT_SET)) {
		osif_err("Failed to put TWT operation");
		return QDF_STATUS_E_FAILURE;
	}

	config_attr = nla_nest_start(reply_skb,
				     QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_PARAMS);
	if (!config_attr) {
		osif_err("nla_nest_start error");
		return QDF_STATUS_E_INVAL;
	}

	sp_offset_tsf = event->additional_params.sp_tsf_us_hi;
	sp_offset_tsf = (sp_offset_tsf << 32) |
			 event->additional_params.sp_tsf_us_lo;

	attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_ID;
	if (nla_put_u8(reply_skb, attr, event->params.dialog_id)) {
		osif_err("Failed to put dialog_id");
		return QDF_STATUS_E_FAILURE;
	}

	attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_STATUS;
	vendor_status = twt_add_status_to_vendor_twt_status(
							event->params.status);
	if (nla_put_u8(reply_skb, attr, vendor_status)) {
		osif_err("Failed to put setup status");
		return QDF_STATUS_E_FAILURE;
	}

	if (event->params.num_additional_twt_params == 0) {
		nla_nest_end(reply_skb, config_attr);
		return QDF_STATUS_SUCCESS;
	}

	response_type = twt_add_cmd_to_vendor_twt_resp_type(
					event->additional_params.twt_cmd);
	if (response_type == -EINVAL) {
		osif_err("Invalid response type from firmware");
		return QDF_STATUS_E_FAILURE;
	}

	attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_RESP_TYPE;
	if (nla_put_u8(reply_skb, attr, response_type)) {
		osif_err("Failed to put setup response type");
		return QDF_STATUS_E_FAILURE;
	}

	attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_TYPE;
	if (nla_put_u8(reply_skb, attr, event->additional_params.announce)) {
		osif_err("Failed to put setup flow type");
		return QDF_STATUS_E_FAILURE;
	}

	osif_debug("wake_dur_us %d", event->additional_params.wake_dur_us);
	wake_duration = (event->additional_params.wake_dur_us /
			 TWT_WAKE_DURATION_MULTIPLICATION_FACTOR);

	attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_DURATION;
	if (nla_put_u32(reply_skb, attr, wake_duration)) {
		osif_err("Failed to put wake duration");
		return QDF_STATUS_E_FAILURE;
	}

	wake_intvl_mantis_us = event->additional_params.wake_intvl_us;
	if (nla_put_u32(reply_skb,
			QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_INTVL2_MANTISSA,
			wake_intvl_mantis_us)) {
		osif_err("Failed to put wake interval mantissa in us");
		return QDF_STATUS_E_FAILURE;
	}

	wake_intvl_mantis_tu = (event->additional_params.wake_intvl_us /
				 TWT_WAKE_INTVL_MULTIPLICATION_FACTOR);

	attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_INTVL_MANTISSA;
	if (nla_put_u32(reply_skb, attr, wake_intvl_mantis_tu)) {
		osif_err("Failed to put wake interval mantissa in tu");
		return QDF_STATUS_E_FAILURE;
	}
	osif_debug("Send mantissa_us:%d, mantissa_tu:%d to userspace",
		  wake_intvl_mantis_us, wake_intvl_mantis_tu);

	attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_INTVL_EXP;
	if (nla_put_u8(reply_skb, attr, 0)) {
		osif_err("Failed to put wake interval exp");
		return QDF_STATUS_E_FAILURE;
	}

	attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_TIME_TSF;
	if (wlan_cfg80211_nla_put_u64(reply_skb, attr, sp_offset_tsf)) {
		osif_err("Failed to put sp_offset_tsf");
		return QDF_STATUS_E_FAILURE;
	}

	attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_TIME;
	if (nla_put_u32(reply_skb, attr,
			event->additional_params.sp_offset_us)) {
		osif_err("Failed to put sp_offset_us");
		return QDF_STATUS_E_FAILURE;
	}

	if (event->additional_params.trig_en) {
		attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_TRIGGER;
		if (nla_put_flag(reply_skb, attr)) {
			osif_err("Failed to put trig type");
			return QDF_STATUS_E_FAILURE;
		}
	}

	if (event->additional_params.protection) {
		attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_PROTECTION;
		if (nla_put_flag(reply_skb, attr)) {
			osif_err("Failed to put protection flag");
			return QDF_STATUS_E_FAILURE;
		}
	}

	if (event->additional_params.bcast) {
		attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_BCAST;
		if (nla_put_flag(reply_skb, attr)) {
			osif_err("Failed to put bcast flag");
			return QDF_STATUS_E_FAILURE;
		}
	}

	if (!event->additional_params.info_frame_disabled) {
		attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_TWT_INFO_ENABLED;
		if (nla_put_flag(reply_skb, attr)) {
			osif_err("Failed to put twt info enable flag");
			return QDF_STATUS_E_FAILURE;
		}
	}

	attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAC_ADDR;
	if (nla_put(reply_skb, attr, QDF_MAC_ADDR_SIZE,
		    event->params.peer_macaddr.bytes)) {
		osif_err("Failed to put mac_addr");
		return QDF_STATUS_E_INVAL;
	}

	nla_nest_end(reply_skb, config_attr);

	return QDF_STATUS_SUCCESS;
}

/**
 * twt_notify_status_to_vendor_twt_status() - convert from
 * HOST_NOTIFY_TWT_STATUS to qca_wlan_vendor_twt_notify_status
 * @status: HOST_TWT_NOTIFY_STATUS value from firmware
 *
 * Return: qca_wlan_vendor_twt_status values corresponding
 * to the firmware failure status
 */
static enum qca_wlan_vendor_twt_status
twt_notify_status_to_vendor_twt_status(enum HOST_TWT_NOTIFY_STATUS status)
{
	switch (status) {
	case HOST_TWT_NOTIFY_EVENT_AP_TWT_REQ_BIT_SET:
		return QCA_WLAN_VENDOR_TWT_STATUS_TWT_REQUIRED;
	case HOST_TWT_NOTIFY_EVENT_AP_TWT_REQ_BIT_CLEAR:
		return QCA_WLAN_VENDOR_TWT_STATUS_TWT_NOT_REQUIRED;
	default:
		return QCA_WLAN_VENDOR_TWT_STATUS_TWT_NOT_REQUIRED;
	}
}

/**
 * osif_twt_notify_pack_nlmsg() - pack nlmsg response for TWT notify
 * @reply_skb: pointer to the response skb structure
 * @event: twt event buffer with firmware response
 *
 * Return: QDF_STATUS_SUCCESS on Success, other QDF_STATUS error codes
 * on failure
 */
static QDF_STATUS
osif_twt_notify_pack_nlmsg(struct sk_buff *reply_skb,
			   struct twt_notify_event_param *event)
{
	int attr;
	enum qca_wlan_vendor_twt_status vendor_status;
	enum qca_wlan_twt_operation twt_op;

	if (event->status == HOST_TWT_NOTIFY_EVENT_READY)
		twt_op = QCA_WLAN_TWT_SETUP_READY_NOTIFY;
	else
		twt_op = QCA_WLAN_TWT_NOTIFY;

	if (nla_put_u8(reply_skb, QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_OPERATION,
		       twt_op)) {
		osif_err("Failed to put TWT notify operation");
		return QDF_STATUS_E_FAILURE;
	}

	if (event->status != HOST_TWT_NOTIFY_EVENT_READY) {
		attr = QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_NOTIFY_STATUS;
		vendor_status = twt_notify_status_to_vendor_twt_status(
								event->status);
		if (nla_put_u8(reply_skb, attr, vendor_status)) {
			osif_err("Failed to put notify status");
			return QDF_STATUS_E_FAILURE;
		}
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * osif_twt_teardown_pack_resp_nlmsg() - pack nlmsg response for teardown
 * @reply_skb: pointer to the response skb structure
 * @event: twt event buffer with firmware response
 *
 * Return: QDF_STATUS_SUCCESS on Success, other QDF_STATUS error codes
 * on failure
 */
static QDF_STATUS
osif_twt_teardown_pack_resp_nlmsg(struct sk_buff *reply_skb,
			     struct twt_del_dialog_complete_event_param *event)
{
	struct nlattr *config_attr;
	enum qca_wlan_vendor_twt_status vendor_status;
	int attr;

	if (nla_put_u8(reply_skb, QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_OPERATION,
		       QCA_WLAN_TWT_TERMINATE)) {
		osif_err("Failed to put TWT operation");
		return QDF_STATUS_E_FAILURE;
	}

	config_attr = nla_nest_start(reply_skb,
				     QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_PARAMS);
	if (!config_attr) {
		osif_err("nla_nest_start error");
		return QDF_STATUS_E_INVAL;
	}

	attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_ID;
	if (nla_put_u8(reply_skb, attr, event->dialog_id)) {
		osif_debug("Failed to put dialog_id");
		return QDF_STATUS_E_FAILURE;
	}

	attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_STATUS;
	vendor_status = twt_del_status_to_vendor_twt_status(event->status);
	if (nla_put_u8(reply_skb, attr, vendor_status)) {
		osif_err("Failed to put QCA_WLAN_TWT_TERMINATE");
		return QDF_STATUS_E_FAILURE;
	}

	attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAC_ADDR;
	if (nla_put(reply_skb, attr, QDF_MAC_ADDR_SIZE,
		    event->peer_macaddr.bytes)) {
		osif_err("Failed to put mac_addr");
		return QDF_STATUS_E_INVAL;
	}

	nla_nest_end(reply_skb, config_attr);

	return QDF_STATUS_SUCCESS;
}

/**
 * osif_twt_resume_pack_resp_nlmsg() - pack the skb with
 * firmware response for twt resume command
 * @reply_skb: skb to store the response
 * @event: Pointer to resume dialog complete event buffer
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
osif_twt_resume_pack_resp_nlmsg(struct sk_buff *reply_skb,
			   struct twt_resume_dialog_complete_event_param *event)
{
	struct nlattr *config_attr;
	int vendor_status, attr;

	if (nla_put_u8(reply_skb, QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_OPERATION,
		       QCA_WLAN_TWT_RESUME)) {
		osif_err("Failed to put TWT operation");
		return QDF_STATUS_E_FAILURE;
	}

	config_attr = nla_nest_start(reply_skb,
				     QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_PARAMS);
	if (!config_attr) {
		osif_err("nla_nest_start error");
		return QDF_STATUS_E_INVAL;
	}

	attr = QCA_WLAN_VENDOR_ATTR_TWT_RESUME_FLOW_ID;
	if (nla_put_u8(reply_skb, attr, event->dialog_id)) {
		osif_debug("Failed to put dialog_id");
		return QDF_STATUS_E_FAILURE;
	}

	attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_STATUS;
	vendor_status = twt_resume_status_to_vendor_twt_status(event->status);
	if (nla_put_u8(reply_skb, attr, vendor_status)) {
		osif_err("Failed to put QCA_WLAN_TWT_RESUME status");
		return QDF_STATUS_E_FAILURE;
	}

	attr = QCA_WLAN_VENDOR_ATTR_TWT_RESUME_MAC_ADDR;
	if (nla_put(reply_skb, attr, QDF_MAC_ADDR_SIZE,
		    event->peer_macaddr.bytes)) {
		osif_err("Failed to put mac_addr");
		return QDF_STATUS_E_INVAL;
	}

	nla_nest_end(reply_skb, config_attr);

	return QDF_STATUS_SUCCESS;
}

/**
 * osif_twt_nudge_pack_resp_nlmsg() - pack the skb with
 * firmware response for twt nudge command
 * @reply_skb: skb to store the response
 * @event: Pointer to nudge dialog complete event buffer
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
osif_twt_nudge_pack_resp_nlmsg(struct sk_buff *reply_skb,
			      struct twt_nudge_dialog_complete_event_param *event)
{
	struct nlattr *config_attr;
	int vendor_status, attr;
	uint64_t tsf_val;

	if (nla_put_u8(reply_skb, QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_OPERATION,
		       QCA_WLAN_TWT_NUDGE)) {
		osif_err("Failed to put TWT operation");
		return QDF_STATUS_E_FAILURE;
	}

	config_attr = nla_nest_start(reply_skb,
				     QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_PARAMS);
	if (!config_attr) {
		osif_err("nla_nest_start error");
		return QDF_STATUS_E_INVAL;
	}

	attr = QCA_WLAN_VENDOR_ATTR_TWT_NUDGE_FLOW_ID;
	if (nla_put_u8(reply_skb, attr, event->dialog_id)) {
		osif_debug("Failed to put dialog_id");
		return QDF_STATUS_E_FAILURE;
	}

	tsf_val = event->next_twt_tsf_us_hi;
	tsf_val = (tsf_val << 32) | event->next_twt_tsf_us_lo;
	if (wlan_cfg80211_nla_put_u64(reply_skb,
				 QCA_WLAN_VENDOR_ATTR_TWT_NUDGE_WAKE_TIME_TSF,
				 tsf_val)) {
		osif_err("get_params failed to put TSF Value");
		return QDF_STATUS_E_INVAL;
	}

	attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_STATUS;
	vendor_status = twt_nudge_status_to_vendor_twt_status(event->status);
	if (nla_put_u8(reply_skb, attr, vendor_status)) {
		osif_err("Failed to put QCA_WLAN_TWT_NUDGE status");
		return QDF_STATUS_E_FAILURE;
	}

	attr = QCA_WLAN_VENDOR_ATTR_TWT_NUDGE_MAC_ADDR;
	if (nla_put(reply_skb, attr, QDF_MAC_ADDR_SIZE,
		    event->peer_macaddr.bytes)) {
		osif_err("Failed to put mac_addr");
		return QDF_STATUS_E_INVAL;
	}

	nla_nest_end(reply_skb, config_attr);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
osif_twt_send_get_capabilities_response(struct wlan_objmgr_psoc *psoc,
					struct wlan_objmgr_vdev *vdev)
{
	struct vdev_osif_priv *osif_priv;
	struct nlattr *config_attr;
	struct sk_buff *reply_skb;
	size_t skb_len = NLMSG_HDRLEN;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	enum band_info connected_band;
	uint8_t peer_cap = 0, self_cap = 0;
	bool twt_req = false, twt_bcast_req = false;
	bool is_twt_24ghz_allowed = true, val;
	struct qdf_mac_addr peer_mac;
	int ret;

	/*
	 * Userspace will query the TWT get capabilities before
	 * issuing a get capabilities request. If the STA is
	 * connected, then check the "enable_twt_24ghz" ini
	 * value to advertise the TWT requestor capability.
	 */
	connected_band = ucfg_cm_get_connected_band(vdev);
	ucfg_twt_cfg_get_24ghz_enabled(psoc, &val);

	osif_debug("connected_band: %d val: %d", connected_band, val);
	if (connected_band == BAND_2G && !val)
		is_twt_24ghz_allowed = false;

	/* fill the self_capability bitmap  */
	ucfg_twt_cfg_get_requestor(psoc, &twt_req);
	osif_debug("is_twt_24ghz_allowed: %d twt_req: %d",
		   is_twt_24ghz_allowed, twt_req);
	if (twt_req && is_twt_24ghz_allowed)
		self_cap |= QCA_WLAN_TWT_CAPA_REQUESTOR;

	ucfg_twt_cfg_get_bcast_requestor(psoc, &twt_bcast_req);
	osif_debug("twt_bcast_req: %d", twt_bcast_req);
	self_cap |= (twt_bcast_req ? QCA_WLAN_TWT_CAPA_BROADCAST : 0);

	ucfg_twt_cfg_get_flex_sched(psoc, &val);
	osif_debug("flex sched: %d", val);
	if (val)
		self_cap |= QCA_WLAN_TWT_CAPA_FLEXIBLE;

	ret = osif_fill_peer_macaddr(vdev, peer_mac.bytes);
	if (ret)
		return QDF_STATUS_E_INVAL;

	qdf_status = ucfg_twt_get_peer_capabilities(psoc, &peer_mac, &peer_cap);
	if (QDF_IS_STATUS_ERROR(qdf_status))
		return qdf_status;

	osif_debug("self_cap: 0x%x peer_cap: 0x%x", self_cap, peer_cap);
	osif_priv = wlan_vdev_get_ospriv(vdev);
	/*
	 * Length of attribute QCA_WLAN_VENDOR_ATTR_TWT_CAPABILITIES_SELF &
	 * QCA_WLAN_VENDOR_ATTR_TWT_CAPABILITIES_PEER
	 */
	skb_len += 2 * nla_total_size(sizeof(u16)) + NLA_HDRLEN;

	reply_skb = wlan_cfg80211_vendor_cmd_alloc_reply_skb(
							osif_priv->wdev->wiphy,
							skb_len);
	if (!reply_skb) {
		osif_err("TWT: get_caps alloc reply skb failed");
		return QDF_STATUS_E_NOMEM;
	}

	config_attr = nla_nest_start(reply_skb,
				     QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_PARAMS);
	if (!config_attr) {
		osif_err("TWT: nla_nest_start error");
		qdf_status = QDF_STATUS_E_FAILURE;
		goto free_skb;
	}

	if (nla_put_u16(reply_skb, QCA_WLAN_VENDOR_ATTR_TWT_CAPABILITIES_SELF,
	    self_cap)) {
		osif_err("TWT: Failed to fill capabilities");
		qdf_status = QDF_STATUS_E_FAILURE;
		goto free_skb;
	}

	if (nla_put_u16(reply_skb, QCA_WLAN_VENDOR_ATTR_TWT_CAPABILITIES_PEER,
	    peer_cap)) {
		osif_err("TWT: Failed to fill capabilities");
		qdf_status = QDF_STATUS_E_FAILURE;
		goto free_skb;
	}

	nla_nest_end(reply_skb, config_attr);

	if (cfg80211_vendor_cmd_reply(reply_skb))
		qdf_status = QDF_STATUS_E_INVAL;

free_skb:
	if (QDF_IS_STATUS_ERROR(qdf_status) && reply_skb)
		kfree_skb(reply_skb);

	return qdf_status;
}

static void
osif_twt_setup_response(struct wlan_objmgr_psoc *psoc,
			struct twt_add_dialog_complete_event *event)
{
	struct sk_buff *twt_vendor_event;
	struct wireless_dev *wdev;
	struct wlan_objmgr_vdev *vdev;
	struct vdev_osif_priv *osif_priv;
	size_t data_len;
	QDF_STATUS status;
	bool additional_params_present = false;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc,
						event->params.vdev_id,
						WLAN_TWT_ID);
	if (!vdev) {
		osif_err("vdev is null");
		return;
	}

	osif_priv = wlan_vdev_get_ospriv(vdev);
	if (!osif_priv) {
		osif_err("osif_priv is null");
		goto fail;
	}

	wdev = osif_priv->wdev;
	if (!wdev) {
		osif_err("wireless dev is null");
		goto fail;
	}

	if (event->params.num_additional_twt_params != 0)
		additional_params_present = true;

	data_len = osif_twt_get_setup_event_len(additional_params_present);
	twt_vendor_event = wlan_cfg80211_vendor_event_alloc(
				wdev->wiphy, wdev, data_len,
				QCA_NL80211_VENDOR_SUBCMD_CONFIG_TWT_INDEX,
				GFP_KERNEL);
	if (!twt_vendor_event) {
		osif_err("TWT: Alloc setup resp skb fail");
		goto fail;
	}

	status = osif_twt_setup_pack_resp_nlmsg(twt_vendor_event, event);
	if (QDF_IS_STATUS_ERROR(status)) {
		osif_err("Failed to pack nl add dialog response");
		wlan_cfg80211_vendor_free_skb(twt_vendor_event);
		goto fail;
	}

	wlan_cfg80211_vendor_event(twt_vendor_event, GFP_KERNEL);

fail:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_TWT_ID);
}

static void
osif_twt_teardown_response(struct wlan_objmgr_psoc *psoc,
			   struct twt_del_dialog_complete_event_param *event)
{
	struct sk_buff *twt_vendor_event;
	struct wireless_dev *wdev;
	struct wlan_objmgr_vdev *vdev;
	struct vdev_osif_priv *osif_priv;
	size_t data_len;
	QDF_STATUS status;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc,
						event->vdev_id, WLAN_TWT_ID);
	if (!vdev) {
		osif_err("vdev is null");
		return;
	}

	osif_priv = wlan_vdev_get_ospriv(vdev);
	if (!osif_priv) {
		osif_err("osif_priv is null");
		goto fail;
	}

	wdev = osif_priv->wdev;
	if (!wdev) {
		osif_err("wireless dev is null");
		goto fail;
	}

	data_len = osif_twt_get_event_len() + nla_total_size(sizeof(u8));
	data_len += NLA_HDRLEN;
	twt_vendor_event = wlan_cfg80211_vendor_event_alloc(
				wdev->wiphy, wdev, data_len,
				QCA_NL80211_VENDOR_SUBCMD_CONFIG_TWT_INDEX,
				GFP_KERNEL);
	if (!twt_vendor_event) {
		osif_err("TWT: Alloc teardown resp skb fail");
		goto fail;
	}

	status = osif_twt_teardown_pack_resp_nlmsg(twt_vendor_event, event);
	if (QDF_IS_STATUS_ERROR(status)) {
		osif_err("Failed to pack nl del dialog response");
		wlan_cfg80211_vendor_free_skb(twt_vendor_event);
		goto fail;
	}

	wlan_cfg80211_vendor_event(twt_vendor_event, GFP_KERNEL);

fail:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_TWT_ID);
}

QDF_STATUS
osif_twt_setup_complete_cb(struct wlan_objmgr_psoc *psoc,
			   struct twt_add_dialog_complete_event *event,
			   bool renego_fail)
{
	uint32_t vdev_id = event->params.vdev_id;

	osif_debug("TWT: add dialog_id:%d, status:%d vdev_id:%d renego_fail:%d peer mac_addr "
		  QDF_MAC_ADDR_FMT, event->params.dialog_id,
		  event->params.status, vdev_id, renego_fail,
		  QDF_MAC_ADDR_REF(event->params.peer_macaddr.bytes));

	osif_twt_setup_response(psoc, event);

	if (renego_fail)
		osif_twt_handle_renego_failure(psoc, event);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
osif_twt_teardown_complete_cb(struct wlan_objmgr_psoc *psoc,
			      struct twt_del_dialog_complete_event_param *event)
{
	uint32_t vdev_id = event->vdev_id;

	osif_debug("TWT: del dialog_id:%d status:%d vdev_id:%d peer mac_addr "
		  QDF_MAC_ADDR_FMT, event->dialog_id,
		  event->status, vdev_id,
		  QDF_MAC_ADDR_REF(event->peer_macaddr.bytes));

	osif_twt_teardown_response(psoc, event);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
osif_twt_resume_complete_cb(struct wlan_objmgr_psoc *psoc,
			   struct twt_resume_dialog_complete_event_param *event)
{
	struct wireless_dev *wdev;
	struct vdev_osif_priv *osif_priv;
	struct wlan_objmgr_vdev *vdev;
	uint32_t vdev_id = event->vdev_id;
	struct sk_buff *twt_vendor_event;
	size_t data_len;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, event->vdev_id,
						    WLAN_TWT_ID);
	if (!vdev) {
		osif_err("vdev is null");
		return status;
	}

	osif_priv = wlan_vdev_get_ospriv(vdev);
	if (!osif_priv) {
		osif_err("osif_priv is null");
		goto fail;
	}

	wdev = osif_priv->wdev;
	if (!wdev) {
		osif_err("wireless dev is null");
		goto fail;
	}

	osif_debug("TWT: resume dialog_id:%d status:%d vdev_id:%d peer macaddr "
		   QDF_MAC_ADDR_FMT, event->dialog_id,
		   event->status, vdev_id,
		   QDF_MAC_ADDR_REF(event->peer_macaddr.bytes));

	data_len = osif_twt_get_event_len() + nla_total_size(sizeof(u8));
	data_len += NLA_HDRLEN;

	twt_vendor_event = wlan_cfg80211_vendor_event_alloc(
				wdev->wiphy, wdev, data_len,
				QCA_NL80211_VENDOR_SUBCMD_CONFIG_TWT_INDEX,
				GFP_KERNEL);
	if (!twt_vendor_event) {
		osif_err("TWT: Alloc resume resp skb fail");
		goto fail;
	}

	status = osif_twt_resume_pack_resp_nlmsg(twt_vendor_event, event);
	if (QDF_IS_STATUS_ERROR(status)) {
		osif_err("Failed to pack nl resume dialog response");
		wlan_cfg80211_vendor_free_skb(twt_vendor_event);
		goto fail;
	}
	wlan_cfg80211_vendor_event(twt_vendor_event, GFP_KERNEL);

fail:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_TWT_ID);
	return status;
}

QDF_STATUS
osif_twt_nudge_complete_cb(struct wlan_objmgr_psoc *psoc,
			   struct twt_nudge_dialog_complete_event_param *event)
{
	struct wireless_dev *wdev;
	struct vdev_osif_priv *osif_priv;
	struct wlan_objmgr_vdev *vdev;
	uint32_t vdev_id = event->vdev_id;
	struct sk_buff *twt_vendor_event;
	size_t data_len;
	QDF_STATUS  status = QDF_STATUS_E_FAILURE;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, event->vdev_id,
						    WLAN_TWT_ID);
	if (!vdev) {
		osif_err("vdev is null");
		return status;
	}

	osif_priv = wlan_vdev_get_ospriv(vdev);
	if (!osif_priv) {
		osif_err("osif_priv is null");
		goto fail;
	}

	wdev = osif_priv->wdev;
	if (!wdev) {
		osif_err("wireless dev is null");
		goto fail;
	}

	osif_debug("TWT: nudge dialog_id:%d status:%d vdev_id:%d peer macaddr "
		   QDF_MAC_ADDR_FMT, event->dialog_id,
		   event->status, vdev_id,
		   QDF_MAC_ADDR_REF(event->peer_macaddr.bytes));

	data_len = osif_twt_get_event_len() + nla_total_size(sizeof(u8)) +
		   nla_total_size(sizeof(u64));
	data_len += NLA_HDRLEN;

	twt_vendor_event = wlan_cfg80211_vendor_event_alloc(
				wdev->wiphy, wdev, data_len,
				QCA_NL80211_VENDOR_SUBCMD_CONFIG_TWT_INDEX,
				GFP_KERNEL);
	if (!twt_vendor_event) {
		osif_err("TWT: Alloc nudge resp skb fail");
		goto fail;
	}

	status = osif_twt_nudge_pack_resp_nlmsg(twt_vendor_event, event);
	if (QDF_IS_STATUS_ERROR(status)) {
		osif_err("Failed to pack nl add dialog response");
		wlan_cfg80211_vendor_free_skb(twt_vendor_event);
		goto fail;
	}
	wlan_cfg80211_vendor_event(twt_vendor_event, GFP_KERNEL);

fail:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_TWT_ID);
	return status;

}

/**
 * osif_twt_get_notify_event_len() - calculates the length of twt
 * notify nl response
 *
 * Return: Length of twt notify nl response
 */
static
uint32_t osif_twt_get_notify_event_len(void)
{
	uint32_t len = 0;

	len += NLMSG_HDRLEN;

	/* QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_OPERATION */
	len += nla_total_size(sizeof(u8));

	/* QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_NOTIFY_STATUS */
	len += nla_total_size(sizeof(u8));

	return len;
}

QDF_STATUS
osif_twt_notify_complete_cb(struct wlan_objmgr_psoc *psoc,
			    struct twt_notify_event_param *event)
{
	struct wireless_dev *wdev;
	struct sk_buff *twt_vendor_event;
	size_t data_len;
	QDF_STATUS status;
	struct vdev_osif_priv *osif_priv;
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, event->vdev_id,
						    WLAN_TWT_ID);
	if (!vdev) {
		osif_err("vdev is null");
		return QDF_STATUS_E_INVAL;
	}

	osif_priv = wlan_vdev_get_ospriv(vdev);
	if (!osif_priv) {
		osif_err("osif_priv is null");
		status = QDF_STATUS_E_INVAL;
		goto end;
	}

	wdev = osif_priv->wdev;
	if (!wdev) {
		osif_err("wireless dev is null");
		status = QDF_STATUS_E_INVAL;
		goto end;
	}

	data_len = osif_twt_get_notify_event_len();
	data_len += NLA_HDRLEN;

	twt_vendor_event = wlan_cfg80211_vendor_event_alloc(
				wdev->wiphy, wdev, data_len,
				QCA_NL80211_VENDOR_SUBCMD_CONFIG_TWT_INDEX,
				GFP_KERNEL);
	if (!twt_vendor_event) {
		osif_err("Notify skb alloc failed");
		status = QDF_STATUS_E_INVAL;
		goto end;
	}

	osif_debug("TWT: twt Notify vdev_id: %d, status: %d", event->vdev_id,
		   event->status);

	status = osif_twt_notify_pack_nlmsg(twt_vendor_event, event);
	if (QDF_IS_STATUS_ERROR(status)) {
		osif_err("Failed to pack nl notify event");
		wlan_cfg80211_vendor_free_skb(twt_vendor_event);
		status = QDF_STATUS_E_INVAL;
		goto end;
	}

	wlan_cfg80211_vendor_event(twt_vendor_event, GFP_KERNEL);
	status = QDF_STATUS_SUCCESS;

end:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_TWT_ID);
	return status;
}

/**
 * twt_pause_status_to_vendor_twt_status() - convert from
 * HOST_TWT_PAUSE_STATUS to qca_wlan_vendor_twt_status
 * @status: HOST_TWT_PAUSE_STATUS value from firmware
 *
 * Return: qca_wlan_vendor_twt_status values corresponding
 * to the firmware failure status
 */
static int
twt_pause_status_to_vendor_twt_status(enum HOST_TWT_PAUSE_STATUS status)
{
	switch (status) {
	case HOST_TWT_PAUSE_STATUS_OK:
		return QCA_WLAN_VENDOR_TWT_STATUS_OK;
	case HOST_TWT_PAUSE_STATUS_DIALOG_ID_NOT_EXIST:
		return QCA_WLAN_VENDOR_TWT_STATUS_SESSION_NOT_EXIST;
	case HOST_TWT_PAUSE_STATUS_INVALID_PARAM:
		return QCA_WLAN_VENDOR_TWT_STATUS_INVALID_PARAM;
	case HOST_TWT_PAUSE_STATUS_DIALOG_ID_BUSY:
		return QCA_WLAN_VENDOR_TWT_STATUS_SESSION_BUSY;
	case HOST_TWT_PAUSE_STATUS_ALREADY_PAUSED:
		return QCA_WLAN_VENDOR_TWT_STATUS_ALREADY_SUSPENDED;
	case HOST_TWT_PAUSE_STATUS_NO_RESOURCE:
		return QCA_WLAN_VENDOR_TWT_STATUS_NO_RESOURCE;
	case HOST_TWT_PAUSE_STATUS_NO_ACK:
		return QCA_WLAN_VENDOR_TWT_STATUS_NO_ACK;
	case HOST_TWT_PAUSE_STATUS_UNKNOWN_ERROR:
		return QCA_WLAN_VENDOR_TWT_STATUS_UNKNOWN_ERROR;
	case HOST_TWT_PAUSE_STATUS_CHAN_SW_IN_PROGRESS:
		return QCA_WLAN_VENDOR_TWT_STATUS_CHANNEL_SWITCH_IN_PROGRESS;
	case HOST_TWT_PAUSE_STATUS_ROAM_IN_PROGRESS:
		return QCA_WLAN_VENDOR_TWT_STATUS_ROAMING_IN_PROGRESS;
	default:
		return QCA_WLAN_VENDOR_TWT_STATUS_UNKNOWN_ERROR;
	}
}

/**
 * osif_twt_pause_pack_resp_nlmsg() - pack the skb with
 * firmware response for twt pause command
 * @reply_skb: skb to store the response
 * @event: Pointer to pause dialog complete event buffer
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
osif_twt_pause_pack_resp_nlmsg(struct sk_buff *reply_skb,
			struct twt_pause_dialog_complete_event_param *event)
{
	struct nlattr *config_attr;
	int vendor_status, attr;

	if (nla_put_u8(reply_skb, QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_OPERATION,
		       QCA_WLAN_TWT_SUSPEND)) {
		osif_err("Failed to put TWT operation");
		return QDF_STATUS_E_FAILURE;
	}

	config_attr = nla_nest_start(reply_skb,
				     QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_PARAMS);
	if (!config_attr) {
		osif_err("nla_nest_start error");
		return QDF_STATUS_E_INVAL;
	}

	attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_ID;
	if (nla_put_u8(reply_skb, attr, event->dialog_id)) {
		osif_debug("Failed to put dialog_id");
		return QDF_STATUS_E_FAILURE;
	}

	attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_STATUS;
	vendor_status = twt_pause_status_to_vendor_twt_status(event->status);
	if (nla_put_u8(reply_skb, attr, vendor_status)) {
		osif_err("Failed to put QCA_WLAN_TWT_PAUSE status");
		return QDF_STATUS_E_FAILURE;
	}

	attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAC_ADDR;
	if (nla_put(reply_skb, attr, QDF_MAC_ADDR_SIZE,
		    event->peer_macaddr.bytes)) {
		osif_err("Failed to put mac_addr");
		return QDF_STATUS_E_INVAL;
	}

	nla_nest_end(reply_skb, config_attr);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
osif_twt_pause_complete_cb(struct wlan_objmgr_psoc *psoc,
			   struct twt_pause_dialog_complete_event_param *event)

{
	struct wireless_dev *wdev;
	struct vdev_osif_priv *osif_priv;
	struct wlan_objmgr_vdev *vdev;
	uint32_t vdev_id = event->vdev_id;
	struct sk_buff *twt_vendor_event;
	size_t data_len;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, event->vdev_id,
						    WLAN_TWT_ID);
	if (!vdev) {
		osif_err("vdev is null");
		return status;
	}

	osif_priv = wlan_vdev_get_ospriv(vdev);
	if (!osif_priv) {
		osif_err("osif_priv is null");
		goto fail;
	}

	wdev = osif_priv->wdev;
	if (!wdev) {
		osif_err("wireless dev is null");
		goto fail;
	}

	osif_debug("TWT: pause dialog_id:%d status:%d vdev_id:%d peer macaddr "
		   QDF_MAC_ADDR_FMT, event->dialog_id,
		   event->status, vdev_id,
		   QDF_MAC_ADDR_REF(event->peer_macaddr.bytes));

	data_len = osif_twt_get_event_len() + nla_total_size(sizeof(u8));
	data_len += NLA_HDRLEN;

	twt_vendor_event = wlan_cfg80211_vendor_event_alloc(
				wdev->wiphy, wdev, data_len,
				QCA_NL80211_VENDOR_SUBCMD_CONFIG_TWT_INDEX,
				GFP_KERNEL);
	if (!twt_vendor_event) {
		osif_err("TWT: Alloc pause resp skb fail");
		goto fail;
	}

	status = osif_twt_pause_pack_resp_nlmsg(twt_vendor_event, event);
	if (QDF_IS_STATUS_ERROR(status)) {
		osif_err("Failed to pack nl add dialog response");
		wlan_cfg80211_vendor_free_skb(twt_vendor_event);
		goto fail;
	}
	wlan_cfg80211_vendor_event(twt_vendor_event, GFP_KERNEL);

fail:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_TWT_ID);
	return status;
}

QDF_STATUS
osif_twt_ack_complete_cb(struct wlan_objmgr_psoc *psoc,
			 struct twt_ack_complete_event_param *params,
			 void *context)
{
	struct osif_request *request = NULL;
	struct twt_ack_context *status_priv;

	request = osif_request_get(context);
	if (!request) {
		osif_err("obsolete request");
		return QDF_STATUS_E_FAILURE;
	}

	status_priv = osif_request_priv(request);
	if (!status_priv) {
		osif_err("obsolete status_priv");
		return QDF_STATUS_E_FAILURE;
	}

	if (status_priv->twt_cmd_ack == params->twt_cmd_ack) {
		status_priv->vdev_id = params->vdev_id;
		qdf_copy_macaddr(&status_priv->peer_macaddr,
				 &params->peer_macaddr);
		status_priv->dialog_id = params->dialog_id;
		status_priv->status = params->status;
		osif_request_complete(request);
	} else {
		osif_err("Invalid TWT ack. Expected cmd: %d Actual cmd: %d",
				status_priv->twt_cmd_ack, params->twt_cmd_ack);
	}

	osif_request_put(request);
	return QDF_STATUS_SUCCESS;
}

static uint32_t
osif_get_session_wake_duration(struct wlan_objmgr_vdev *vdev,
			       uint32_t dialog_id,
			       struct qdf_mac_addr *peer_macaddr)
{
	struct wlan_objmgr_psoc *psoc;
	struct twt_session_stats_info params = {0};
	int num_twt_session = 0;

	psoc = wlan_vdev_get_psoc(vdev);
	params.dialog_id = dialog_id;
	qdf_copy_macaddr(&params.peer_mac, peer_macaddr);

	osif_debug("Get_params peer mac_addr " QDF_MAC_ADDR_FMT,
		   QDF_MAC_ADDR_REF(params.peer_mac.bytes));

	num_twt_session = ucfg_cp_stats_twt_get_peer_session_params(psoc,
								    &params);
	if (num_twt_session)
		return params.wake_dura_us;

	return 0;
}

static int
twt_get_stats_status_to_vendor_twt_status(enum HOST_TWT_GET_STATS_STATUS status)
{
	switch (status) {
	case HOST_TWT_GET_STATS_STATUS_OK:
		return QCA_WLAN_VENDOR_TWT_STATUS_OK;
	case HOST_TWT_GET_STATS_STATUS_DIALOG_ID_NOT_EXIST:
		return QCA_WLAN_VENDOR_TWT_STATUS_SESSION_NOT_EXIST;
	case HOST_TWT_GET_STATS_STATUS_INVALID_PARAM:
		return QCA_WLAN_VENDOR_TWT_STATUS_INVALID_PARAM;
	default:
		return QCA_WLAN_VENDOR_TWT_STATUS_UNKNOWN_ERROR;
	}
}

/**
 * osif_twt_pack_get_stats_resp_nlmsg() - Packs and sends twt get stats response
 * @vdev: vdev
 * @reply_skb: pointer to response skb buffer
 * @params: Pointer to twt session parameter buffer
 * @num_session_stats: number of twt statistics
 *
 * Return: QDF_STATUS_SUCCESS on success, else other qdf error values
 */
static QDF_STATUS
osif_twt_pack_get_stats_resp_nlmsg(struct wlan_objmgr_vdev *vdev,
				   struct sk_buff *reply_skb,
				   struct twt_infra_cp_stats_event *params,
				   uint32_t num_session_stats)
{
	struct nlattr *config_attr, *nla_params;
	int i, attr;
	int vendor_status;
	uint32_t duration;

	config_attr = nla_nest_start(reply_skb,
				     QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_PARAMS);

	if (!config_attr) {
		osif_err("get_params nla_nest_start error");
		return QDF_STATUS_E_INVAL;
	}

	for (i = 0; i < num_session_stats; i++) {
		nla_params = nla_nest_start(reply_skb, i);
		if (!nla_params) {
			osif_err("get_stats nla_nest_start error");
			return QDF_STATUS_E_INVAL;
		}

		attr = QCA_WLAN_VENDOR_ATTR_TWT_STATS_MAC_ADDR;
		if (nla_put(reply_skb, attr, QDF_MAC_ADDR_SIZE,
			    params[i].peer_macaddr.bytes)) {
			osif_err("get_stats failed to put mac_addr");
			return QDF_STATUS_E_INVAL;
		}

		osif_debug("get_stats peer mac_addr " QDF_MAC_ADDR_FMT,
			   QDF_MAC_ADDR_REF(params[i].peer_macaddr.bytes));

		attr = QCA_WLAN_VENDOR_ATTR_TWT_STATS_FLOW_ID;
		if (nla_put_u8(reply_skb, attr, params[i].dialog_id)) {
			osif_err("get_stats failed to put dialog_id");
			return QDF_STATUS_E_INVAL;
		}

		duration = osif_get_session_wake_duration(vdev,
						params[i].dialog_id,
						&params[i].peer_macaddr);
		attr = QCA_WLAN_VENDOR_ATTR_TWT_STATS_SESSION_WAKE_DURATION;
		if (nla_put_u32(reply_skb, attr, duration)) {
			osif_err("get_params failed to put Wake duration");
			return QDF_STATUS_E_INVAL;
		}

		osif_debug("dialog_id %d wake duration %d num sp cycles %d",
			   params[i].dialog_id, duration,
			   params[i].num_sp_cycles);

		attr = QCA_WLAN_VENDOR_ATTR_TWT_STATS_NUM_SP_ITERATIONS;
		if (nla_put_u32(reply_skb, attr, params[i].num_sp_cycles)) {
			osif_err("get_params failed to put num_sp_cycles");
			return QDF_STATUS_E_INVAL;
		}

		attr = QCA_WLAN_VENDOR_ATTR_TWT_STATS_AVG_WAKE_DURATION;
		if (nla_put_u32(reply_skb, attr, params[i].avg_sp_dur_us)) {
			osif_err("get_params failed to put avg_sp_dur_us");
			return QDF_STATUS_E_INVAL;
		}

		attr = QCA_WLAN_VENDOR_ATTR_TWT_STATS_MIN_WAKE_DURATION;
		if (nla_put_u32(reply_skb, attr, params[i].min_sp_dur_us)) {
			osif_err("get_params failed to put min_sp_dur_us");
			return QDF_STATUS_E_INVAL;
		}

		attr = QCA_WLAN_VENDOR_ATTR_TWT_STATS_MAX_WAKE_DURATION;
		if (nla_put_u32(reply_skb, attr, params[i].max_sp_dur_us)) {
			osif_err("get_params failed to put max_sp_dur_us");
			return QDF_STATUS_E_INVAL;
		}

		attr = QCA_WLAN_VENDOR_ATTR_TWT_STATS_AVERAGE_TX_MPDU;
		if (nla_put_u32(reply_skb, attr, params[i].tx_mpdu_per_sp)) {
			osif_err("get_params failed to put tx_mpdu_per_sp");
			return QDF_STATUS_E_INVAL;
		}

		attr = QCA_WLAN_VENDOR_ATTR_TWT_STATS_AVERAGE_RX_MPDU;
		if (nla_put_u32(reply_skb, attr, params[i].rx_mpdu_per_sp)) {
			osif_err("get_params failed to put rx_mpdu_per_sp");
			return QDF_STATUS_E_INVAL;
		}

		attr = QCA_WLAN_VENDOR_ATTR_TWT_STATS_AVERAGE_TX_PACKET_SIZE;
		if (nla_put_u32(reply_skb, attr, params[i].tx_bytes_per_sp)) {
			osif_err("get_params failed to put tx_bytes_per_sp");
			return QDF_STATUS_E_INVAL;
		}

		attr = QCA_WLAN_VENDOR_ATTR_TWT_STATS_AVERAGE_RX_PACKET_SIZE;
		if (nla_put_u32(reply_skb, attr, params[i].rx_bytes_per_sp)) {
			osif_err("get_params failed to put rx_bytes_per_sp");
			return QDF_STATUS_E_INVAL;
		}

		attr = QCA_WLAN_VENDOR_ATTR_TWT_STATS_STATUS;
		vendor_status =
		    twt_get_stats_status_to_vendor_twt_status(params[i].status);
		if (nla_put_u32(reply_skb, attr, vendor_status)) {
			osif_err("get_params failed to put status");
			return QDF_STATUS_E_INVAL;
		}

		nla_nest_end(reply_skb, nla_params);
	}

	nla_nest_end(reply_skb, config_attr);

	return QDF_STATUS_SUCCESS;
}

/**
 * osif_get_twt_get_stats_event_len() - calculate length of skb
 * required for sending twt get statistics command responses.
 *
 * Return: length of skb
 */
static uint32_t osif_get_twt_get_stats_event_len(void)
{
	uint32_t len = 0;

	len += NLMSG_HDRLEN;

	/* QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_ID */
	len += nla_total_size(sizeof(u8));

	/* QCA_WLAN_VENDOR_ATTR_TWT_SETUP_STATUS */
	len += nla_total_size(sizeof(u8));

	return len;
}

QDF_STATUS osif_twt_get_stats_response(struct wlan_objmgr_vdev *vdev,
				       struct twt_infra_cp_stats_event *params,
				       uint32_t num_session_stats)
{
	int skb_len;
	struct vdev_osif_priv *osif_priv;
	struct wireless_dev *wdev;
	QDF_STATUS status = QDF_STATUS_E_INVAL;
	struct sk_buff *reply_skb;

	osif_priv = wlan_vdev_get_ospriv(vdev);
	if (!osif_priv) {
		osif_err("osif_priv is null");
		return QDF_STATUS_E_INVAL;
	}

	wdev = osif_priv->wdev;
	if (!wdev) {
		osif_err("wireless dev is null");
		return QDF_STATUS_E_INVAL;
	}

	skb_len = osif_get_twt_get_stats_event_len();
	reply_skb = wlan_cfg80211_vendor_cmd_alloc_reply_skb(wdev->wiphy,
							     skb_len);
	if (!reply_skb) {
		osif_err("Get stats - alloc reply_skb failed");
		return QDF_STATUS_E_NOMEM;
	}

	status = osif_twt_pack_get_stats_resp_nlmsg(vdev, reply_skb, params,
						    num_session_stats);
	if (QDF_IS_STATUS_ERROR(status)) {
		osif_err("Get stats - Failed to pack nl response");
		wlan_cfg80211_vendor_free_skb(reply_skb);
		return qdf_status_to_os_return(status);
	}

	return wlan_cfg80211_vendor_cmd_reply(reply_skb);
}

