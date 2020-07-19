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

/**
 * DOC : wlan_hdd_twt.c
 *
 * WLAN Host Device Driver file for TWT (Target Wake Time) support.
 *
 */

#include "wmi.h"
#include "wmi_unified_priv.h"
#include "wmi_unified_twt_param.h"
#include "wlan_hdd_twt.h"
#include "wlan_hdd_main.h"
#include "wlan_hdd_cfg.h"
#include "sme_api.h"
#include "wma_twt.h"
#include "osif_sync.h"
#include "wlan_osif_request_manager.h"

#define TWT_SETUP_COMPLETE_TIMEOUT 1000
#define TWT_DISABLE_COMPLETE_TIMEOUT 1000
#define TWT_TERMINATE_COMPLETE_TIMEOUT 1000

/**
 * struct twt_add_dialog_complete_event - TWT add dialog complete event
 * @params: Fixed parameters for TWT add dialog complete event
 * @additional_params: additional parameters for TWT add dialog complete event
 *
 * Holds the fixed and additional parameters from add dialog
 * complete event
 */
struct twt_add_dialog_complete_event {
	struct wmi_twt_add_dialog_complete_event_param params;
	struct wmi_twt_add_dialog_additional_params additional_params;
};

/**
 * struct twt_add_dialog_comp_ev_priv - private struct for twt add dialog
 * @add_dialog_comp_ev_buf: buffer from TWT add dialog complete_event
 *
 * This TWT add dialog private structure is registered with os_if to
 * retrieve the TWT add dialog response event buffer.
 */
struct twt_add_dialog_comp_ev_priv {
	struct twt_add_dialog_complete_event add_dialog_comp_ev_buf;
};

/**
 * struct twt_del_dialog_comp_ev_priv - private struct for twt del dialog
 * @del_dialog_comp_ev_buf: buffer from TWT del dialog complete_event
 *
 * This TWT del dialog private structure is registered with os_if to
 * retrieve the TWT del dialog response event buffer.
 */
struct twt_del_dialog_comp_ev_priv {
	struct wmi_twt_del_dialog_complete_event_param del_dialog_comp_ev_buf;
};

const struct nla_policy
wlan_hdd_wifi_twt_config_policy[
	QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_MAX + 1] = {
		[QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_OPERATION] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_PARAMS] = {
			.type = NLA_NESTED},
};

static uint32_t hdd_get_twt_setup_event_len(void)
{
	uint32_t len = 0;

	len += NLMSG_HDRLEN;
	/* QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_ID */
	len += nla_total_size(sizeof(u8));
	/* QCA_WLAN_VENDOR_ATTR_TWT_SETUP_STATUS */
	len += nla_total_size(sizeof(u8));
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

	return len;
}

/**
 * wmi_twt_add_cmd_to_vendor_twt_resp_type() - convert from
 * WMI_HOST_TWT_COMMAND to qca_wlan_vendor_twt_setup_resp_type
 * @status: WMI_HOST_TWT_COMMAND value from firmare
 *
 * Return: qca_wlan_vendor_twt_setup_resp_type values for valid
 * WMI_HOST_TWT_COMMAND value and -EINVAL for invalid value
 */
static
int wmi_twt_add_cmd_to_vendor_twt_resp_type(enum WMI_HOST_TWT_COMMAND type)
{
	switch (type) {
	case WMI_HOST_TWT_COMMAND_ACCEPT_TWT:
		return QCA_WLAN_VENDOR_TWT_RESP_ACCEPT;
	case WMI_HOST_TWT_COMMAND_ALTERNATE_TWT:
		return QCA_WLAN_VENDOR_TWT_RESP_ALTERNATE;
	case WMI_HOST_TWT_COMMAND_DICTATE_TWT:
		return QCA_WLAN_VENDOR_TWT_RESP_DICTATE;
	case WMI_HOST_TWT_COMMAND_REJECT_TWT:
		return QCA_WLAN_VENDOR_TWT_RESP_REJECT;
	default:
		return -EINVAL;
	}
}

/**
 * wmi_twt_del_status_to_vendor_twt_status() - convert from
 * WMI_HOST_DEL_TWT_STATUS to qca_wlan_vendor_twt_status
 * @status: WMI_HOST_DEL_TWT_STATUS value from firmare
 *
 * Return: qca_wlan_vendor_twt_status values corresponsing
 * to the firmware failure status
 */
static
int wmi_twt_del_status_to_vendor_twt_status(enum WMI_HOST_DEL_TWT_STATUS status)
{
	switch (status) {
	case WMI_HOST_DEL_TWT_STATUS_OK:
		return QCA_WLAN_VENDOR_TWT_STATUS_OK;
	case WMI_HOST_DEL_TWT_STATUS_DIALOG_ID_NOT_EXIST:
		return QCA_WLAN_VENDOR_TWT_STATUS_SESSION_NOT_EXIST;
	case WMI_HOST_DEL_TWT_STATUS_INVALID_PARAM:
		return QCA_WLAN_VENDOR_TWT_STATUS_INVALID_PARAM;
	case WMI_HOST_DEL_TWT_STATUS_DIALOG_ID_BUSY:
		return QCA_WLAN_VENDOR_TWT_STATUS_SESSION_BUSY;
	case WMI_HOST_DEL_TWT_STATUS_NO_RESOURCE:
		return QCA_WLAN_VENDOR_TWT_STATUS_NO_RESOURCE;
	case WMI_HOST_DEL_TWT_STATUS_NO_ACK:
		return QCA_WLAN_VENDOR_TWT_STATUS_NO_ACK;
	case WMI_HOST_DEL_TWT_STATUS_UNKNOWN_ERROR:
		return QCA_WLAN_VENDOR_TWT_STATUS_UNKNOWN_ERROR;
	default:
		return QCA_WLAN_VENDOR_TWT_STATUS_UNKNOWN_ERROR;
	}
}

/**
 * wmi_twt_add_status_to_vendor_twt_status() - convert from
 * WMI_HOST_ADD_TWT_STATUS to qca_wlan_vendor_twt_status
 * @status: WMI_HOST_ADD_TWT_STATUS value from firmare
 *
 * Return: qca_wlan_vendor_twt_status values for valid
 * WMI_HOST_ADD_TWT_STATUS and -EINVAL for invalid value
 */
static
int wmi_twt_add_status_to_vendor_twt_status(enum WMI_HOST_ADD_TWT_STATUS status)
{
	switch (status) {
	case WMI_HOST_ADD_TWT_STATUS_OK:
		return QCA_WLAN_VENDOR_TWT_STATUS_OK;
	case WMI_HOST_ADD_TWT_STATUS_TWT_NOT_ENABLED:
		return QCA_WLAN_VENDOR_TWT_STATUS_TWT_NOT_ENABLED;
	case WMI_HOST_ADD_TWT_STATUS_USED_DIALOG_ID:
		return QCA_WLAN_VENDOR_TWT_STATUS_USED_DIALOG_ID;
	case WMI_HOST_ADD_TWT_STATUS_INVALID_PARAM:
		return QCA_WLAN_VENDOR_TWT_STATUS_INVALID_PARAM;
	case WMI_HOST_ADD_TWT_STATUS_NOT_READY:
		return QCA_WLAN_VENDOR_TWT_STATUS_NOT_READY;
	case WMI_HOST_ADD_TWT_STATUS_NO_RESOURCE:
		return QCA_WLAN_VENDOR_TWT_STATUS_NO_RESOURCE;
	case WMI_HOST_ADD_TWT_STATUS_NO_ACK:
		return QCA_WLAN_VENDOR_TWT_STATUS_NO_ACK;
	case WMI_HOST_ADD_TWT_STATUS_NO_RESPONSE:
		return QCA_WLAN_VENDOR_TWT_STATUS_NO_RESPONSE;
	case WMI_HOST_ADD_TWT_STATUS_DENIED:
		return QCA_WLAN_VENDOR_TWT_STATUS_DENIED;
	case WMI_HOST_ADD_TWT_STATUS_UNKNOWN_ERROR:
		return QCA_WLAN_VENDOR_TWT_STATUS_UNKNOWN_ERROR;
	default:
		return -EINVAL;
	}
}

static
QDF_STATUS hdd_twt_setup_pack_resp_nlmsg(
	 struct sk_buff *reply_skb,
	 struct twt_add_dialog_complete_event *event)
{
	uint64_t sp_offset_tsf;
	int vendor_status;
	int response_type;

	hdd_enter();

	sp_offset_tsf = event->additional_params.sp_tsf_us_hi;
	sp_offset_tsf = (sp_offset_tsf << 32) |
			 event->additional_params.sp_tsf_us_lo;

	if (nla_put_u8(reply_skb, QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_ID,
		       event->params.dialog_id)) {
		hdd_debug("TWT: Failed to put dialog_id");
		return QDF_STATUS_E_FAILURE;
	}

	vendor_status = wmi_twt_add_status_to_vendor_twt_status(event->params.status);
	if (vendor_status == -EINVAL)
		return QDF_STATUS_E_FAILURE;
	if (nla_put_u8(reply_skb, QCA_WLAN_VENDOR_ATTR_TWT_SETUP_STATUS,
		       vendor_status)) {
		hdd_err("TWT: Failed to put QCA_WLAN_TWT_SET");
		return QDF_STATUS_E_FAILURE;
	}

	response_type = wmi_twt_add_cmd_to_vendor_twt_resp_type(event->additional_params.twt_cmd);
	if (response_type == -EINVAL)
		return QDF_STATUS_E_FAILURE;
	if (nla_put_u8(reply_skb, QCA_WLAN_VENDOR_ATTR_TWT_SETUP_RESP_TYPE,
		       response_type)) {
		hdd_err("TWT: Failed to put QCA_WLAN_TWT_SET");
		return QDF_STATUS_E_FAILURE;
	}

	if (nla_put_u8(reply_skb, QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_TYPE,
		       event->additional_params.announce)) {
		hdd_err("TWT: Failed to put QCA_WLAN_TWT_SET");
		return QDF_STATUS_E_FAILURE;
	}

	if (nla_put_u32(reply_skb, QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_DURATION,
			event->additional_params.wake_dur_us)) {
		hdd_err("TWT: Failed to put wake duration");
		return QDF_STATUS_E_FAILURE;
	}

	if (nla_put_u32(reply_skb,
			QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_INTVL_MANTISSA,
			event->additional_params.wake_intvl_us)) {
		hdd_err("TWT: Failed to put wake interval us");
		return QDF_STATUS_E_FAILURE;
	}

	if (nla_put_u8(reply_skb, QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_INTVL_EXP,
		       0)) {
		hdd_err("TWT: Failed to put wake interval exp");
		return QDF_STATUS_E_FAILURE;
	}

	if (wlan_cfg80211_nla_put_u64(reply_skb,
				      QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_TIME_TSF,
				      sp_offset_tsf)) {
		hdd_err("TWT: Failed to put sp_offset_tsf");
		return QDF_STATUS_E_FAILURE;
	}

	if (nla_put_u32(reply_skb, QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_TIME,
			event->additional_params.sp_offset_us)) {
		hdd_err("TWT: Failed to put sp_offset_us");
		return QDF_STATUS_E_FAILURE;
	}

	if (event->additional_params.trig_en) {
		if (nla_put_flag(reply_skb,
				 QCA_WLAN_VENDOR_ATTR_TWT_SETUP_TRIGGER)) {
			hdd_err("TWT: Failed to put trig type");
			return QDF_STATUS_E_FAILURE;
		}
	}

	if (event->additional_params.protection) {
		if (nla_put_flag(reply_skb,
				 QCA_WLAN_VENDOR_ATTR_TWT_SETUP_PROTECTION)) {
			hdd_err("TWT: Failed to put protection flag");
			return QDF_STATUS_E_FAILURE;
		}
	}

	if (event->additional_params.bcast) {
		if (nla_put_flag(reply_skb,
				 QCA_WLAN_VENDOR_ATTR_TWT_SETUP_BCAST)) {
			hdd_err("TWT: Failed to put bcast flag");
			return QDF_STATUS_E_FAILURE;
		}
	}

	if (!event->additional_params.info_frame_disabled) {
		if (nla_put_flag(reply_skb,
				 QCA_WLAN_VENDOR_ATTR_TWT_SETUP_TWT_INFO_ENABLED)) {
			hdd_err("TWT: Failed to put twt info enable flag");
			return QDF_STATUS_E_FAILURE;
		}
	}

	hdd_exit();

	return QDF_STATUS_SUCCESS;
}

static void
hdd_twt_add_dialog_comp_cb(void *context,
			   struct wmi_twt_add_dialog_complete_event_param *params,
			   struct wmi_twt_add_dialog_additional_params *additional_params)
{
	struct osif_request *request;
	struct twt_add_dialog_comp_ev_priv *priv;

	hdd_enter();

	request = osif_request_get(context);
	if (!request) {
		hdd_err("Obsolete request");
		return;
	}

	priv = osif_request_priv(request);

	qdf_mem_copy(&priv->add_dialog_comp_ev_buf.params, params,
		     sizeof(*params));
	qdf_mem_copy(&priv->add_dialog_comp_ev_buf.additional_params,
		     additional_params,
		     sizeof(*additional_params));
	osif_request_complete(request);
	osif_request_put(request);

	hdd_debug("TWT: add dialog_id:%d, status:%d vdev_id %d peer mac_addr"
		  QDF_MAC_ADDR_STR, params->dialog_id,
		  params->status, params->vdev_id,
		  QDF_MAC_ADDR_ARRAY(params->peer_macaddr));

	hdd_exit();
}

/**
 * hdd_send_twt_add_dialog_cmd() - Send TWT add dialog command to target
 * @hdd_ctx: HDD Context
 * @twt_params: Pointer to Add dialog cmd params structure
 *
 * Return: QDF_STATUS
 */
static
int hdd_send_twt_add_dialog_cmd(struct hdd_context *hdd_ctx,
				struct wmi_twt_add_dialog_param *twt_params)
{
	struct twt_add_dialog_complete_event *add_dialog_comp_ev_params;
	struct twt_add_dialog_comp_ev_priv *priv;
	static const struct osif_request_params osif_req_params = {
		.priv_size = sizeof(*priv),
		.timeout_ms = TWT_SETUP_COMPLETE_TIMEOUT,
		.dealloc = NULL,
	};
	struct osif_request *request;
	struct sk_buff *reply_skb = NULL;
	void *cookie;
	QDF_STATUS status;
	int skb_len;
	int ret;

	request = osif_request_alloc(&osif_req_params);
	if (!request) {
		hdd_err("twt osif request allocation failure");
		ret = -ENOMEM;
		goto err;
	}

	cookie = osif_request_cookie(request);
	status = sme_add_dialog_cmd(hdd_ctx->mac_handle,
				    hdd_twt_add_dialog_comp_cb,
				    twt_params,
				    cookie);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("Failed to send add dialog command");
		ret = qdf_status_to_os_return(status);
		goto err;
	}

	ret = osif_request_wait_for_response(request);
	if (ret) {
		hdd_err("twt: add dialog req timedout");
		ret = -ETIMEDOUT;
		goto err;
	}

	priv = osif_request_priv(request);
	add_dialog_comp_ev_params = &priv->add_dialog_comp_ev_buf;

	skb_len = hdd_get_twt_setup_event_len();

	reply_skb = cfg80211_vendor_cmd_alloc_reply_skb(hdd_ctx->wiphy,
							skb_len);
	if (!reply_skb) {
		hdd_err("cfg80211_vendor_cmd_alloc_reply_skb failed");
		ret = -ENOMEM;
		goto err;
	}

	status = hdd_twt_setup_pack_resp_nlmsg(reply_skb,
					       add_dialog_comp_ev_params);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("Failed to pack nl add dialog response");
		ret = qdf_status_to_os_return(status);
		goto err;
	}
	ret = cfg80211_vendor_cmd_reply(reply_skb);

err:
	if (request)
		osif_request_put(request);

	if (ret && reply_skb)
		kfree_skb(reply_skb);
	return ret;
}

/**
 * hdd_twt_setup_session() - Process TWT setup operation in the
 * received vendor command and send it to firmare
 * @adapter: adapter pointer
 * @twt_param_attr: nl attributes
 *
 * Handles QCA_WLAN_TWT_SET
 *
 * Return: 0 for Success and negative value for failure
 */
static int hdd_twt_setup_session(struct hdd_adapter *adapter,
				 struct nlattr *twt_param_attr)
{
	struct hdd_station_ctx *hdd_sta_ctx = NULL;
	struct wmi_twt_add_dialog_param params = {0};
	struct nlattr *tb2[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAX + 1];
	uint32_t congestion_timeout = 0;
	int ret = 0;

	if (adapter->device_mode != QDF_STA_MODE &&
	    adapter->device_mode != QDF_P2P_CLIENT_MODE) {
		return -EOPNOTSUPP;
	}

	hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	if (hdd_sta_ctx->conn_info.conn_state != eConnectionState_Associated) {
		hdd_err_rl("Invalid state, vdev %d mode %d state %d",
			   adapter->vdev_id, adapter->device_mode,
			   hdd_sta_ctx->conn_info.conn_state);
		return -EINVAL;
	}

	qdf_mem_copy(params.peer_macaddr,
		     hdd_sta_ctx->conn_info.bssid.bytes,
		     QDF_MAC_ADDR_SIZE);
	params.vdev_id = adapter->vdev_id;

	ret = wlan_cfg80211_nla_parse_nested(tb2,
					     QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAX,
					     twt_param_attr,
					     qca_wlan_vendor_twt_add_dialog_policy);
	if (ret)
		return ret;

	ret = hdd_twt_get_add_dialog_values(tb2, &params);
	if (ret)
		return ret;

	ucfg_mlme_get_twt_congestion_timeout(adapter->hdd_ctx->psoc,
					     &congestion_timeout);

	if (congestion_timeout) {
		ret = qdf_status_to_os_return(
			hdd_send_twt_disable_cmd(adapter->hdd_ctx));
		if (ret) {
			hdd_err("Failed to disable TWT");
			return ret;
		}
		ucfg_mlme_set_twt_congestion_timeout(adapter->hdd_ctx->psoc, 0);
		hdd_send_twt_enable_cmd(adapter->hdd_ctx);
	}
	ret = hdd_send_twt_add_dialog_cmd(adapter->hdd_ctx, &params);
	return ret;
}

/**
 * hdd_get_twt_terminate_event_len() - calculate length of skb
 * required for sending twt terminate response.
 *
 * Return: length of skb
 */
static uint32_t hdd_get_twt_terminate_event_len(void)
{
	uint32_t len = 0;

	len += NLMSG_HDRLEN;
	/* QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_ID */
	len += nla_total_size(sizeof(u8));
	/* QCA_WLAN_VENDOR_ATTR_TWT_SETUP_STATUS */
	len += nla_total_size(sizeof(u8));

	return len;
}

/**
 * hdd_twt_terminate_pack_resp_nlmsg() - pack the skb with
 * firmware response for twt terminate command
 * @reply_skb: skb to store the response
 * @params: Pointer to del dialog complete event buffer
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
hdd_twt_terminate_pack_resp_nlmsg(struct sk_buff *reply_skb,
				  struct wmi_twt_del_dialog_complete_event_param *params)
{
	int vendor_status;

	if (nla_put_u8(reply_skb, QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_ID,
		       params->dialog_id)) {
		hdd_debug("TWT: Failed to put dialog_id");
		return QDF_STATUS_E_FAILURE;
	}

	vendor_status = wmi_twt_del_status_to_vendor_twt_status(params->status);
	if (nla_put_u8(reply_skb, QCA_WLAN_VENDOR_ATTR_TWT_SETUP_STATUS,
		       vendor_status)) {
		hdd_err("TWT: Failed to put QCA_WLAN_TWT_TERMINATE");
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * hdd_twt_del_dialog_comp_cb() - callback function
 * to get twt terminate command complete event
 * @context: private context
 * @params: Pointer to del dialog complete event buffer
 *
 * Return: None
 */
static void
hdd_twt_del_dialog_comp_cb(void *context,
			   struct wmi_twt_del_dialog_complete_event_param *params)
{
	struct osif_request *request;
	struct twt_del_dialog_comp_ev_priv *priv;

	hdd_enter();

	request = osif_request_get(context);
	if (!request) {
		hdd_err("Obsolete request");
		return;
	}

	priv = osif_request_priv(request);
	qdf_mem_copy(&priv->del_dialog_comp_ev_buf, params,
		     sizeof(*params));
	osif_request_complete(request);
	osif_request_put(request);

	hdd_debug("TWT: del dialog_id:%d, status:%d vdev_id %d peer mac_addr "
		  QDF_MAC_ADDR_FMT, params->dialog_id,
		  params->status, params->vdev_id,
		  QDF_MAC_ADDR_REF(params->peer_macaddr));

	hdd_exit();
}

/**
 * hdd_send_twt_del_dialog_cmd() - Send TWT del dialog command to target
 * @hdd_ctx: HDD Context
 * @twt_params: Pointer to del dialog cmd params structure
 *
 * Return: QDF_STATUS
 */
static
int hdd_send_twt_del_dialog_cmd(struct hdd_context *hdd_ctx,
				struct wmi_twt_del_dialog_param *twt_params)
{
	struct wmi_twt_del_dialog_complete_event_param *del_dialog_comp_ev_params;
	struct twt_del_dialog_comp_ev_priv *priv;
	static const struct osif_request_params osif_req_params = {
		.priv_size = sizeof(*priv),
		.timeout_ms = TWT_TERMINATE_COMPLETE_TIMEOUT,
		.dealloc = NULL,
	};
	struct osif_request *request;
	struct sk_buff *reply_skb = NULL;
	QDF_STATUS status;
	void *cookie;
	int skb_len;
	int ret = 0;

	request = osif_request_alloc(&osif_req_params);
	if (!request) {
		hdd_err("twt osif request allocation failure");
		ret = -ENOMEM;
		goto err;
	}

	cookie = osif_request_cookie(request);

	status = sme_del_dialog_cmd(hdd_ctx->mac_handle,
				    hdd_twt_del_dialog_comp_cb,
				    twt_params,
				    cookie);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("Failed to send del dialog command");
		ret = qdf_status_to_os_return(status);
		goto err;
	}

	ret = osif_request_wait_for_response(request);
	if (ret) {
		hdd_err("twt: del dialog req timedout");
		ret = -ETIMEDOUT;
		goto err;
	}

	priv = osif_request_priv(request);
	del_dialog_comp_ev_params = &priv->del_dialog_comp_ev_buf;

	skb_len = hdd_get_twt_terminate_event_len();
	reply_skb = wlan_cfg80211_vendor_cmd_alloc_reply_skb(hdd_ctx->wiphy,
							     skb_len);
	if (!reply_skb) {
		hdd_err("cfg80211_vendor_cmd_alloc_reply_skb failed");
		ret = -ENOMEM;
		goto err;
	}

	status = hdd_twt_terminate_pack_resp_nlmsg(reply_skb,
						   del_dialog_comp_ev_params);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("Failed to pack nl del dialog response");
		ret = qdf_status_to_os_return(status);
		goto err;
	}

	ret = wlan_cfg80211_vendor_cmd_reply(reply_skb);

err:
	if (request)
		osif_request_put(request);
	if (ret && reply_skb)
		kfree_skb(reply_skb);
	return ret;
}

/**
 * hdd_twt_terminate_session - Process TWT terminate
 * operation in the recevied vendor command and
 * send it to firmare
 * @adapter: adapter pointer
 * @twt_param_attr: nl attributes
 *
 * Handles QCA_WLAN_TWT_TERMINATE
 *
 * Return: 0 on success, negative value on failure
 */
static int hdd_twt_terminate_session(struct hdd_adapter *adapter,
				     struct nlattr *twt_param_attr)
{
	struct hdd_station_ctx *hdd_sta_ctx = NULL;
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAX + 1];
	struct wmi_twt_del_dialog_param params = {0};
	int id;
	int ret;

	if (adapter->device_mode != QDF_STA_MODE &&
	    adapter->device_mode != QDF_P2P_CLIENT_MODE) {
		return -EOPNOTSUPP;
	}

	hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	if (hdd_sta_ctx->conn_info.conn_state != eConnectionState_Associated) {
		hdd_err_rl("Invalid state, vdev %d mode %d state %d",
			   adapter->vdev_id, adapter->device_mode,
			   hdd_sta_ctx->conn_info.conn_state);
		return -EINVAL;
	}

	qdf_mem_copy(params.peer_macaddr,
		     hdd_sta_ctx->conn_info.bssid.bytes,
		     QDF_MAC_ADDR_SIZE);
	params.vdev_id = adapter->vdev_id;

	ret = wlan_cfg80211_nla_parse_nested(tb,
					     QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAX,
					     twt_param_attr,
					     qca_wlan_vendor_twt_add_dialog_policy);
	if (ret)
		return ret;

	id = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_ID;
	if (tb[id]) {
		params.dialog_id = nla_get_u8(tb[id]);
	} else {
		params.dialog_id = 0;
		hdd_debug("TWT_TERMINATE_FLOW_ID not specified. set to zero");
	}

	hdd_debug("twt_terminate: vdev_id %d dialog_id %d peer mac_addr "
		  QDF_MAC_ADDR_FMT, params.vdev_id, params.dialog_id,
		  QDF_MAC_ADDR_REF(params.peer_macaddr));

	ret = hdd_send_twt_del_dialog_cmd(adapter->hdd_ctx, &params);

	return ret;
}

/**
 * hdd_twt_configure - Process the TWT
 * operation in the recevied vendor command
 * @adapter: adapter pointer
 * @tb: nl attributes
 *
 * Handles QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_OPERATION
 *
 * Return: 0 for Success and negative value for failure
 */
static int hdd_twt_configure(struct hdd_adapter *adapter,
			     struct nlattr **tb)
{
	enum qca_wlan_twt_operation twt_oper;
	struct nlattr *twt_oper_attr;
	struct nlattr *twt_param_attr;
	uint32_t id;
	int ret = 0;

	id = QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_OPERATION;
	twt_oper_attr = tb[id];

	if (!twt_oper_attr) {
		hdd_err("TWT parameters NOT specified");
		return -EINVAL;
	}

	id = QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_PARAMS;
	twt_param_attr = tb[id];

	if (!twt_param_attr) {
		hdd_err("TWT parameters NOT specified");
		return -EINVAL;
	}

	twt_oper = nla_get_u8(twt_oper_attr);
	hdd_debug("twt: TWT Operation 0x%x", twt_oper);

	switch (twt_oper) {
	case QCA_WLAN_TWT_SET:
		ret = hdd_twt_setup_session(adapter, twt_param_attr);
		break;
	case QCA_WLAN_TWT_GET:
		break;
	case QCA_WLAN_TWT_TERMINATE:
		ret = hdd_twt_terminate_session(adapter, twt_param_attr);
		break;
	case QCA_WLAN_TWT_SUSPEND:
		break;
	case QCA_WLAN_TWT_RESUME:
		break;
	default:
		hdd_err("Invalid TWT Operation");
		ret = -EINVAL;
		break;
	}

	return ret;
}

/**
 * __wlan_hdd_cfg80211_wifi_twt_config() - Wifi TWT configuration
 * vendor command
 * @wiphy: wiphy device pointer
 * @wdev: wireless device pointer
 * @data: Vendor command data buffer
 * @data_len: Buffer length
 *
 * Handles QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_MAX.
 *
 * Return: 0 for Success and negative value for failure
 */
static int
__wlan_hdd_cfg80211_wifi_twt_config(struct wiphy *wiphy,
				    struct wireless_dev *wdev,
				    const void *data, int data_len)
{
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx  = wiphy_priv(wiphy);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_MAX + 1];
	int errno;

	if (hdd_get_conparam() == QDF_GLOBAL_FTM_MODE) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	errno = wlan_hdd_validate_context(hdd_ctx);
	if (errno)
		return errno;

	errno = hdd_validate_adapter(adapter);
	if (errno)
		return errno;

	if (wlan_cfg80211_nla_parse(tb, QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_MAX,
				    data,
				    data_len,
				    wlan_hdd_wifi_twt_config_policy)) {
		hdd_err("invalid twt attr");
		return -EINVAL;
	}

	errno = hdd_twt_configure(adapter, tb);
	return errno;
}

int wlan_hdd_cfg80211_wifi_twt_config(struct wiphy *wiphy,
				      struct wireless_dev *wdev,
				      const void *data,
				      int data_len)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_wifi_twt_config(wiphy, wdev, data,
						    data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}

void hdd_update_tgt_twt_cap(struct hdd_context *hdd_ctx,
			    struct wma_tgt_cfg *cfg)
{
	struct wma_tgt_services *services = &cfg->services;
	bool enable_twt = false;

	ucfg_mlme_get_enable_twt(hdd_ctx->psoc, &enable_twt);
	hdd_debug("TWT: enable_twt=%d, tgt Req=%d, Res=%d",
		  enable_twt, services->twt_requestor,
		  services->twt_responder);

	ucfg_mlme_set_twt_requestor(hdd_ctx->psoc,
				    QDF_MIN(services->twt_requestor,
					    enable_twt));

	ucfg_mlme_set_twt_responder(hdd_ctx->psoc,
				    QDF_MIN(services->twt_responder,
					    enable_twt));

	/*
	 * Currently broadcast TWT is not supported
	 */
	ucfg_mlme_set_bcast_twt(hdd_ctx->psoc,
				QDF_MIN(0, enable_twt));
}

void hdd_send_twt_enable_cmd(struct hdd_context *hdd_ctx)
{
	uint8_t pdev_id = hdd_ctx->pdev->pdev_objmgr.wlan_pdev_id;
	bool req_val = 0, resp_val = 0, bcast_val = 0;
	uint32_t congestion_timeout = 0;

	ucfg_mlme_get_twt_requestor(hdd_ctx->psoc, &req_val);
	ucfg_mlme_get_twt_responder(hdd_ctx->psoc, &resp_val);
	ucfg_mlme_get_bcast_twt(hdd_ctx->psoc, &bcast_val);
	ucfg_mlme_get_twt_congestion_timeout(hdd_ctx->psoc,
					     &congestion_timeout);

	hdd_debug("TWT cfg req:%d, responder:%d, bcast:%d, pdev:%d, cong:%d",
		  req_val, resp_val, bcast_val, pdev_id, congestion_timeout);

	if (req_val || resp_val || bcast_val)
		wma_send_twt_enable_cmd(pdev_id, congestion_timeout, bcast_val);
}

QDF_STATUS hdd_send_twt_disable_cmd(struct hdd_context *hdd_ctx)
{
	uint8_t pdev_id = hdd_ctx->pdev->pdev_objmgr.wlan_pdev_id;

	hdd_debug("TWT disable cmd :pdev:%d", pdev_id);

	wma_send_twt_disable_cmd(pdev_id);

	return qdf_wait_single_event(&hdd_ctx->twt_disable_comp_evt,
				     TWT_DISABLE_COMPLETE_TIMEOUT);
}

/**
 * hdd_twt_enable_comp_cb() - TWT enable complete event callback
 * @hdd_handle: opaque handle for the global HDD Context
 * @twt_event: TWT event data received from the target
 *
 * Return: None
 */
static void
hdd_twt_enable_comp_cb(hdd_handle_t hdd_handle,
		       struct wmi_twt_enable_complete_event_param *params)
{
	struct hdd_context *hdd_ctx = hdd_handle_to_context(hdd_handle);
	enum twt_status prev_state;

	if (!hdd_ctx) {
		hdd_err("TWT: Invalid HDD Context");
		return;
	}
	prev_state = hdd_ctx->twt_state;
	if (params->status == WMI_HOST_ENABLE_TWT_STATUS_OK ||
	    params->status == WMI_HOST_ENABLE_TWT_STATUS_ALREADY_ENABLED) {
		switch (prev_state) {
		case TWT_FW_TRIGGER_ENABLE_REQUESTED:
			hdd_ctx->twt_state = TWT_FW_TRIGGER_ENABLED;
			break;
		case TWT_HOST_TRIGGER_ENABLE_REQUESTED:
			hdd_ctx->twt_state = TWT_HOST_TRIGGER_ENABLED;
			break;
		default:
			break;
		}
	}
	if (params->status == WMI_HOST_ENABLE_TWT_INVALID_PARAM ||
	    params->status == WMI_HOST_ENABLE_TWT_STATUS_UNKNOWN_ERROR)
		hdd_ctx->twt_state = TWT_INIT;

	hdd_debug("TWT: pdev ID:%d, status:%d State transitioned from %d to %d",
		  params->pdev_id, params->status,
		  prev_state, hdd_ctx->twt_state);
}

/**
 * hdd_twt_disable_comp_cb() - TWT disable complete event callback
 * @hdd_handle: opaque handle for the global HDD Context
 *
 * Return: None
 */
static void
hdd_twt_disable_comp_cb(hdd_handle_t hdd_handle)
{
	struct hdd_context *hdd_ctx = hdd_handle_to_context(hdd_handle);
	enum twt_status prev_state;
	QDF_STATUS status;

	if (!hdd_ctx) {
		hdd_err("TWT: Invalid HDD Context");
		return;
	}
	prev_state = hdd_ctx->twt_state;
	hdd_ctx->twt_state = TWT_DISABLED;

	hdd_debug("TWT: State transitioned from %d to %d",
		  prev_state, hdd_ctx->twt_state);

	status = qdf_event_set(&hdd_ctx->twt_disable_comp_evt);
	if (!QDF_IS_STATUS_SUCCESS(status))
		hdd_err("Failed to set twt_disable_comp_evt");
}

void wlan_hdd_twt_init(struct hdd_context *hdd_ctx)
{
	QDF_STATUS status;

	hdd_ctx->twt_state = TWT_INIT;
	status = sme_register_twt_enable_complete_cb(hdd_ctx->mac_handle,
						     hdd_twt_enable_comp_cb);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("Register twt enable complete failed");
		return;
	}

	status = sme_register_twt_disable_complete_cb(hdd_ctx->mac_handle,
						      hdd_twt_disable_comp_cb);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_deregister_twt_enable_complete_cb(hdd_ctx->mac_handle);
		hdd_err("Register twt disable complete failed");
		return;
	}

	status = qdf_event_create(&hdd_ctx->twt_disable_comp_evt);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_deregister_twt_disable_complete_cb(hdd_ctx->mac_handle);
		sme_deregister_twt_enable_complete_cb(hdd_ctx->mac_handle);
		hdd_err("twt_disable_comp_evt init failed");
		return;
	}

	hdd_send_twt_enable_cmd(hdd_ctx);
}

void wlan_hdd_twt_deinit(struct hdd_context *hdd_ctx)
{
	QDF_STATUS status;

	status  = sme_deregister_twt_disable_complete_cb(hdd_ctx->mac_handle);
	if (!QDF_IS_STATUS_SUCCESS(status))
		hdd_err("De-register of twt disable cb failed: %d", status);
	status  = sme_deregister_twt_enable_complete_cb(hdd_ctx->mac_handle);
	if (!QDF_IS_STATUS_SUCCESS(status))
		hdd_err("De-register of twt enable cb failed: %d", status);

	if (!QDF_IS_STATUS_SUCCESS(qdf_event_destroy(
				   &hdd_ctx->twt_disable_comp_evt)))
		hdd_err("Failed to destroy twt_disable_comp_evt");

	hdd_ctx->twt_state = TWT_CLOSED;
}
