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

/**
 * osif_get_twt_event_len() - calculate length of skb
 * required for sending twt terminate, pause and resume
 * command responses.
 *
 * Return: length of skb
 */
static uint32_t osif_get_twt_event_len(void)
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

	data_len = osif_get_twt_event_len() + nla_total_size(sizeof(u8));
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
osif_twt_pause_complete_cb(struct wlan_objmgr_psoc *psoc,
			   struct twt_pause_dialog_complete_event_param *event)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
osif_twt_resume_complete_cb(struct wlan_objmgr_psoc *psoc,
			   struct twt_resume_dialog_complete_event_param *event)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
osif_twt_nudge_complete_cb(struct wlan_objmgr_psoc *psoc,
			   struct twt_nudge_dialog_complete_event_param *event)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
osif_twt_notify_complete_cb(struct wlan_objmgr_psoc *psoc,
			    struct twt_notify_event_param *event)
{
	return QDF_STATUS_SUCCESS;
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

