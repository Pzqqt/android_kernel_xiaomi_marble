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
 *  DOC: osif_twt_ext_req.c
 *  This file contains twt component's osif API implementation
 */
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_twt_ucfg_api.h>
#include <wlan_twt_ucfg_ext_api.h>
#include <wlan_twt_ucfg_ext_cfg.h>
#include <osif_twt_req.h>
#include <osif_twt_ext_req.h>
#include <wlan_policy_mgr_api.h>
#include <wlan_cm_api.h>
#include <wlan_cfg80211.h>
#include <wlan_cm_roam_api.h>
#include <wlan_serialization_api.h>
#include <osif_twt_internal.h>
#include <wlan_osif_request_manager.h>

#define TWT_ACK_COMPLETE_TIMEOUT 1000

#define TWT_FLOW_TYPE_ANNOUNCED 0
#define TWT_FLOW_TYPE_UNANNOUNCED 1

#define TWT_SETUP_WAKE_INTVL_MANTISSA_MAX       0xFFFF
#define TWT_SETUP_WAKE_DURATION_MAX             0xFFFF
#define TWT_SETUP_WAKE_INTVL_EXP_MAX            31
#define TWT_MAX_NEXT_TWT_SIZE                   3

static const struct nla_policy
qca_wlan_vendor_twt_add_dialog_policy[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_INTVL_EXP] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_BCAST] = {.type = NLA_FLAG },
	[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_REQ_TYPE] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_TRIGGER] = {.type = NLA_FLAG },
	[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_ID] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_TYPE] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_PROTECTION] = {.type = NLA_FLAG },
	[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_TIME] = {.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_DURATION] = {.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_INTVL_MANTISSA] = {.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MIN_WAKE_DURATION] = {.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAX_WAKE_DURATION] = {.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MIN_WAKE_INTVL] = {.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAX_WAKE_INTVL] = {.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_INTVL2_MANTISSA] = {.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAC_ADDR] = VENDOR_NLA_POLICY_MAC_ADDR,
	[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_BCAST_ID] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_BCAST_RECOMMENDATION] = {
							.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_BCAST_PERSISTENCE] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_TIME_TSF] = {.type = NLA_U64 },
};

static int osif_is_twt_command_allowed(struct wlan_objmgr_vdev *vdev,
				       uint8_t vdev_id,
				       struct wlan_objmgr_psoc *psoc)
{
	enum QDF_OPMODE mode = wlan_vdev_mlme_get_opmode(vdev);

	if (mode != QDF_STA_MODE &&
	    mode != QDF_P2P_CLIENT_MODE)
		return -EOPNOTSUPP;

	if (!wlan_cm_is_vdev_connected(vdev)) {
		osif_err_rl("Not associated!, vdev %d mode %d", vdev_id, mode);
		return -EAGAIN;
	}

	if (wlan_cm_host_roam_in_progress(psoc, vdev_id))
		return -EBUSY;

	if (wlan_get_vdev_status(vdev)) {
		osif_err_rl("Scan in progress");
		return -EBUSY;
	}

	return 0;
}

static bool osif_twt_setup_conc_allowed(struct wlan_objmgr_psoc *psoc,
					uint8_t vdev_id)
{
	return policy_mgr_current_concurrency_is_mcc(psoc) ||
	       policy_mgr_is_scc_with_this_vdev_id(psoc, vdev_id);
}

/**
 * osif_twt_setup_req_type_to_cmd() - Converts twt setup request type to twt cmd
 * @req_type: twt setup request type
 * @twt_cmd: pointer to store twt command
 *
 * Return: QDF_STATUS_SUCCESS on success, else other qdf error values
 */
static QDF_STATUS
osif_twt_setup_req_type_to_cmd(u8 req_type, enum HOST_TWT_COMMAND *twt_cmd)
{
	if (req_type == QCA_WLAN_VENDOR_TWT_SETUP_REQUEST) {
		*twt_cmd = HOST_TWT_COMMAND_REQUEST_TWT;
	} else if (req_type == QCA_WLAN_VENDOR_TWT_SETUP_SUGGEST) {
		*twt_cmd = HOST_TWT_COMMAND_SUGGEST_TWT;
	} else if (req_type == QCA_WLAN_VENDOR_TWT_SETUP_DEMAND) {
		*twt_cmd = HOST_TWT_COMMAND_DEMAND_TWT;
	} else {
		osif_err_rl("Invalid TWT_SETUP_REQ_TYPE %d", req_type);
		return QDF_STATUS_E_INVAL;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * osif_twt_parse_add_dialog_attrs() - Get TWT add dialog parameter
 * values from QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_PARAMS
 * @tb: nl attributes
 * @params: wmi twt add dialog parameters
 *
 * Handles QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAX
 *
 * Return: 0 or -EINVAL.
 */
static int
osif_twt_parse_add_dialog_attrs(struct nlattr **tb,
				struct twt_add_dialog_param *params)
{
	uint32_t wake_intvl_exp, result;
	int cmd_id;
	QDF_STATUS qdf_status;

	cmd_id = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_ID;
	if (tb[cmd_id]) {
		params->dialog_id = nla_get_u8(tb[cmd_id]);
		if (params->dialog_id > TWT_MAX_DIALOG_ID) {
			osif_err_rl("Flow id (%u) invalid", params->dialog_id);
			return -EINVAL;
		}
	} else {
		params->dialog_id = 0;
		osif_debug("TWT_SETUP_FLOW_ID not specified. set to zero");
	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_INTVL_EXP;
	if (!tb[cmd_id]) {
		osif_err_rl("TWT_SETUP_WAKE_INTVL_EXP is must");
		return -EINVAL;
	}
	wake_intvl_exp = nla_get_u8(tb[cmd_id]);
	if (wake_intvl_exp > TWT_SETUP_WAKE_INTVL_EXP_MAX) {
		osif_err_rl("Invalid wake_intvl_exp %u > %u",
			   wake_intvl_exp,
			   TWT_SETUP_WAKE_INTVL_EXP_MAX);
		return -EINVAL;
	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_BCAST;
	params->flag_bcast = nla_get_flag(tb[cmd_id]);

	cmd_id = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_BCAST_ID;
	if (tb[cmd_id]) {
		params->dialog_id = nla_get_u8(tb[cmd_id]);
		osif_debug("TWT_SETUP_BCAST_ID %d", params->dialog_id);
	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_BCAST_RECOMMENDATION;
	if (tb[cmd_id]) {
		params->b_twt_recommendation = nla_get_u8(tb[cmd_id]);
		osif_debug("TWT_SETUP_BCAST_RECOMM %d",
			  params->b_twt_recommendation);
	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_BCAST_PERSISTENCE;
	if (tb[cmd_id]) {
		params->b_twt_persistence = nla_get_u8(tb[cmd_id]);
		osif_debug("TWT_SETUP_BCAST_PERSIS %d",
			  params->b_twt_persistence);
	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_REQ_TYPE;
	if (!tb[cmd_id]) {
		osif_err_rl("TWT_SETUP_REQ_TYPE is must");
		return -EINVAL;
	}
	qdf_status = osif_twt_setup_req_type_to_cmd(nla_get_u8(tb[cmd_id]),
						   &params->twt_cmd);
	if (QDF_IS_STATUS_ERROR(qdf_status))
		return qdf_status_to_os_return(qdf_status);

	cmd_id = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_TRIGGER;
	params->flag_trigger = nla_get_flag(tb[cmd_id]);

	cmd_id = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_TYPE;
	if (!tb[cmd_id]) {
		osif_err_rl("TWT_SETUP_FLOW_TYPE is must");
		return -EINVAL;
	}
	params->flag_flow_type = nla_get_u8(tb[cmd_id]);
	if (params->flag_flow_type != TWT_FLOW_TYPE_ANNOUNCED &&
	    params->flag_flow_type != TWT_FLOW_TYPE_UNANNOUNCED)
		return -EINVAL;

	cmd_id = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_PROTECTION;
	params->flag_protection = nla_get_flag(tb[cmd_id]);

	cmd_id = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_TIME;
	if (tb[cmd_id])
		params->sp_offset_us = nla_get_u32(tb[cmd_id]);

	cmd_id = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_DURATION;
	if (!tb[cmd_id]) {
		osif_err_rl("TWT_SETUP_WAKE_DURATION is must");
		return -EINVAL;
	}
	params->wake_dura_us = TWT_WAKE_DURATION_MULTIPLICATION_FACTOR *
			       nla_get_u32(tb[cmd_id]);
	if (params->wake_dura_us > TWT_SETUP_WAKE_DURATION_MAX) {
		osif_err_rl("Invalid wake_dura_us %u",
			   params->wake_dura_us);
		return -EINVAL;
	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MIN_WAKE_DURATION;
	if (tb[cmd_id])
		params->min_wake_dura_us = nla_get_u32(tb[cmd_id]);

	cmd_id = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAX_WAKE_DURATION;
	if (tb[cmd_id])
		params->max_wake_dura_us = nla_get_u32(tb[cmd_id]);

	if (params->min_wake_dura_us > params->max_wake_dura_us) {
		osif_err_rl("Invalid wake duration range min:%d max:%d. Reset to zero",
			   params->min_wake_dura_us, params->max_wake_dura_us);
		params->min_wake_dura_us = 0;
		params->max_wake_dura_us = 0;
	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_INTVL_MANTISSA;
	if (!tb[cmd_id]) {
		osif_err_rl("SETUP_WAKE_INTVL_MANTISSA is must");
		return -EINVAL;
	}
	params->wake_intvl_mantis = nla_get_u32(tb[cmd_id]);

	/*
	 * If mantissa in microsecond is present then take precedence over
	 * mantissa in TU. And send mantissa in microsecond to firmware.
	 */
	cmd_id = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_INTVL2_MANTISSA;
	if (tb[cmd_id])
		params->wake_intvl_mantis = nla_get_u32(tb[cmd_id]);

	if (params->wake_intvl_mantis >
	    TWT_SETUP_WAKE_INTVL_MANTISSA_MAX) {
		osif_err_rl("Invalid wake_intvl_mantis %u",
			   params->wake_intvl_mantis);
		return -EINVAL;
	}

	if (wake_intvl_exp && params->wake_intvl_mantis) {
		result = 2 << (wake_intvl_exp - 1);
		if (result >
		    (UINT_MAX / params->wake_intvl_mantis)) {
			osif_err_rl("Invalid exp %d mantissa %d",
				   wake_intvl_exp,
				   params->wake_intvl_mantis);
			return -EINVAL;
		}
		params->wake_intvl_us =
			params->wake_intvl_mantis * result;
	} else {
		params->wake_intvl_us = params->wake_intvl_mantis;
	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MIN_WAKE_INTVL;
	if (tb[cmd_id])
		params->min_wake_intvl_us = nla_get_u32(tb[cmd_id]);

	cmd_id = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAX_WAKE_INTVL;
	if (tb[cmd_id])
		params->max_wake_intvl_us = nla_get_u32(tb[cmd_id]);

	if (params->min_wake_intvl_us > params->max_wake_intvl_us) {
		osif_err_rl("Invalid wake intvl range min:%d max:%d. Reset to zero",
			   params->min_wake_intvl_us,
			   params->max_wake_intvl_us);
		params->min_wake_dura_us = 0;
		params->max_wake_dura_us = 0;
	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_TIME_TSF;
	if (tb[cmd_id])
		params->wake_time_tsf = nla_get_u64(tb[cmd_id]);
	else
		params->wake_time_tsf = 0;

	osif_debug("twt: dialog_id %d, vdev %d, wake intvl_us %d, min %d, max %d, mantis %d",
		  params->dialog_id, params->vdev_id, params->wake_intvl_us,
		  params->min_wake_intvl_us, params->max_wake_intvl_us,
		  params->wake_intvl_mantis);

	osif_debug("twt: wake dura %d, min %d, max %d, sp_offset %d, cmd %d",
		  params->wake_dura_us, params->min_wake_dura_us,
		  params->max_wake_dura_us, params->sp_offset_us,
		  params->twt_cmd);
	osif_debug("twt: bcast %d, trigger %d, flow_type %d, prot %d wake_tsf 0x%llx",
		  params->flag_bcast, params->flag_trigger,
		  params->flag_flow_type,
		  params->flag_protection,
		  params->wake_time_tsf);
	osif_debug("twt: peer mac_addr "
		  QDF_MAC_ADDR_FMT,
		  QDF_MAC_ADDR_REF(params->peer_macaddr.bytes));

	return 0;
}

/**
 * osif_twt_parse_del_dialog_attrs() - Parse TWT del dialog parameters
 * values from QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_PARAMS
 * @tb: nl attributes
 * @params: twt del dialog parameters
 *
 * Handles QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAX
 *
 * Return: 0 or -EINVAL.
 */
static int
osif_twt_parse_del_dialog_attrs(struct nlattr **tb,
				struct twt_del_dialog_param *params)
{
	int cmd_id;

	cmd_id = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_ID;
	if (tb[cmd_id]) {
		params->dialog_id = nla_get_u8(tb[cmd_id]);
	} else {
		params->dialog_id = 0;
		osif_debug("TWT_TERMINATE_FLOW_ID not specified. set to zero");
	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_BCAST_ID;
	if (tb[cmd_id]) {
		params->dialog_id = nla_get_u8(tb[cmd_id]);
		osif_debug("TWT_SETUP_BCAST_ID %d", params->dialog_id);
	}

	osif_debug("twt: dialog_id %d vdev %d peer mac_addr "QDF_MAC_ADDR_FMT,
		   params->dialog_id, params->vdev_id,
		   QDF_MAC_ADDR_REF(params->peer_macaddr.bytes));

	return 0;
}

static int osif_fill_peer_macaddr(struct wlan_objmgr_vdev *vdev,
				  uint8_t *mac_addr)
{
	struct wlan_objmgr_peer *peer;

	peer = wlan_objmgr_vdev_try_get_bsspeer(vdev, WLAN_TWT_ID);
	if (!peer) {
		osif_err("peer is null");
		return -EINVAL;
	}
	wlan_peer_obj_lock(peer);
	qdf_mem_copy(mac_addr, wlan_peer_get_macaddr(peer),
				 QDF_MAC_ADDR_SIZE);
	wlan_peer_obj_unlock(peer);

	wlan_objmgr_peer_release_ref(peer, WLAN_TWT_ID);
	return 0;
}

/**
 * osif_twt_ack_wait_response: TWT wait for ack event if it's supported
 * @psoc: psoc context
 * @request: OSIF request cookie
 * @twt_cmd: TWT command for which ack event come
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
osif_twt_ack_wait_response(struct wlan_objmgr_psoc *psoc,
			   struct osif_request *request, int twt_cmd)
{
	struct twt_ack_context *ack_priv;
	int ret = 0;
	bool twt_ack_cap;

	ucfg_twt_get_twt_ack_supported(psoc, &twt_ack_cap);

	if (!twt_ack_cap) {
		osif_err("TWT ack is not supported. No need to wait");
		return QDF_STATUS_SUCCESS;
	}

	ack_priv = osif_request_priv(request);
	ack_priv->twt_cmd_ack = twt_cmd;

	ret = osif_request_wait_for_response(request);
	if (ret) {
		osif_err("TWT ack response timed out");
		return QDF_STATUS_E_TIMEOUT;
	}

	osif_debug("TWT ack info: vdev_id %d dialog_id %d twt_cmd %d status %d peer_macaddr "
			  QDF_MAC_ADDR_FMT, ack_priv->vdev_id, ack_priv->dialog_id,
			  ack_priv->twt_cmd_ack, ack_priv->status,
			  QDF_MAC_ADDR_REF(ack_priv->peer_macaddr.bytes));

	return QDF_STATUS_SUCCESS;
}

static int
osif_send_twt_setup_req(struct wlan_objmgr_vdev *vdev,
			struct wlan_objmgr_psoc *psoc,
			struct twt_add_dialog_param *twt_params)
{
	QDF_STATUS status;
	int twt_cmd, ret = 0;
	struct osif_request *request;
	struct twt_ack_context *ack_priv;
	void *context;
	static const struct osif_request_params params = {
				.priv_size = sizeof(*ack_priv),
				.timeout_ms = TWT_ACK_COMPLETE_TIMEOUT,
	};

	request = osif_request_alloc(&params);
	if (!request) {
		osif_err("Request allocation failure");
		return -ENOMEM;
	}

	context = osif_request_cookie(request);

	status = ucfg_twt_setup_req(psoc, twt_params, context);
	if (QDF_IS_STATUS_ERROR(status)) {
		ret = qdf_status_to_os_return(status);
		osif_err("Failed to send add dialog command");
		goto cleanup;
	}

	twt_cmd = HOST_TWT_ADD_DIALOG_CMDID;
	status = osif_twt_ack_wait_response(psoc, request, twt_cmd);
	if (QDF_IS_STATUS_ERROR(status)) {
		/*
		 * If the TWT ack event comes after the timeout or
		 * if the event is not received from the firmware, then
		 * initialize the context (reset the active command),
		 * otherwise future commands shall be blocked.
		 */
		ucfg_twt_init_context(psoc, &twt_params->peer_macaddr,
				      twt_params->dialog_id);
		ret = qdf_status_to_os_return(status);
		goto cleanup;
	}

	ack_priv = osif_request_priv(request);
	if (ack_priv->status) {
		osif_err("Received TWT ack error: %d. Reset twt command",
			 ack_priv->status);
		ucfg_twt_init_context(psoc, &twt_params->peer_macaddr,
				      twt_params->dialog_id);

		switch (ack_priv->status) {
		case HOST_ADD_TWT_STATUS_INVALID_PARAM:
		case HOST_ADD_TWT_STATUS_UNKNOWN_ERROR:
		case HOST_ADD_TWT_STATUS_USED_DIALOG_ID:
			ret = -EINVAL;
			break;
		case HOST_ADD_TWT_STATUS_ROAM_IN_PROGRESS:
		case HOST_ADD_TWT_STATUS_CHAN_SW_IN_PROGRESS:
		case HOST_ADD_TWT_STATUS_SCAN_IN_PROGRESS:
			ret = -EBUSY;
			break;
		case HOST_ADD_TWT_STATUS_TWT_NOT_ENABLED:
			ret = -EOPNOTSUPP;
			break;
		case HOST_ADD_TWT_STATUS_NOT_READY:
			ret = -EAGAIN;
			break;
		case HOST_ADD_TWT_STATUS_NO_RESOURCE:
			ret = -ENOMEM;
			break;
		default:
			ret = -EINVAL;
			break;
		}
	}

cleanup:
	osif_request_put(request);
	return ret;
}

static int
osif_send_sta_twt_teardown_req(struct wlan_objmgr_vdev *vdev,
			       struct wlan_objmgr_psoc *psoc,
			       struct twt_del_dialog_param *twt_params)
{
	QDF_STATUS status;
	int twt_cmd, ret = 0;
	struct osif_request *request;
	struct twt_ack_context *ack_priv;
	void *context;
	static const struct osif_request_params params = {
				.priv_size = sizeof(*ack_priv),
				.timeout_ms = TWT_ACK_COMPLETE_TIMEOUT,
	};

	request = osif_request_alloc(&params);
	if (!request) {
		osif_err("Request allocation failure");
		return -ENOMEM;
	}

	context = osif_request_cookie(request);

	status = ucfg_twt_teardown_req(psoc, twt_params, context);
	if (QDF_IS_STATUS_ERROR(status)) {
		ret = qdf_status_to_os_return(status);
		osif_err("Failed to send del dialog command");
		goto cleanup;
	}

	twt_cmd = HOST_TWT_DEL_DIALOG_CMDID;

	status = osif_twt_ack_wait_response(psoc, request, twt_cmd);
	if (QDF_IS_STATUS_ERROR(status)) {
		ucfg_twt_reset_active_command(psoc, &twt_params->peer_macaddr,
					      twt_params->dialog_id);
		ret = qdf_status_to_os_return(status);
		goto cleanup;
	}

	ack_priv = osif_request_priv(request);
	if (ack_priv->status != HOST_TWT_DEL_STATUS_OK) {
		osif_err("Received TWT ack error:%d. Reset twt command",
			  ack_priv->status);

		switch (ack_priv->status) {
		case HOST_TWT_DEL_STATUS_INVALID_PARAM:
		case HOST_TWT_DEL_STATUS_UNKNOWN_ERROR:
			ret = -EINVAL;
			break;
		case HOST_TWT_DEL_STATUS_DIALOG_ID_NOT_EXIST:
			ret = -EAGAIN;
			break;
		case HOST_TWT_DEL_STATUS_DIALOG_ID_BUSY:
			ret = -EINPROGRESS;
			break;
		case HOST_TWT_DEL_STATUS_NO_RESOURCE:
			ret = -ENOMEM;
			break;
		case HOST_TWT_DEL_STATUS_ROAMING:
		case HOST_TWT_DEL_STATUS_CHAN_SW_IN_PROGRESS:
		case HOST_TWT_DEL_STATUS_SCAN_IN_PROGRESS:
			ret = -EBUSY;
			break;
		case HOST_TWT_DEL_STATUS_CONCURRENCY:
			ret = -EAGAIN;
			break;
		default:
			ret = -EAGAIN;
			break;
		}
	}

cleanup:
	osif_request_put(request);
	return ret;
}

int osif_twt_send_requestor_enable_cmd(struct wlan_objmgr_psoc *psoc,
				       uint8_t pdev_id)
{
	struct twt_enable_param req = {0};

	req.pdev_id = pdev_id;
	req.ext_conf_present = true;

	return osif_twt_requestor_enable(psoc, &req);
}

int osif_twt_send_responder_enable_cmd(struct wlan_objmgr_psoc *psoc,
				       uint8_t pdev_id)
{
	struct twt_enable_param req = {0};

	req.pdev_id = pdev_id;
	req.ext_conf_present = true;

	return osif_twt_responder_enable(psoc, &req);
}

int osif_twt_send_requestor_disable_cmd(struct wlan_objmgr_psoc *psoc,
					uint8_t pdev_id)
{
	struct twt_disable_param req = {0};

	req.pdev_id = pdev_id;
	req.ext_conf_present = true;

	return osif_twt_requestor_disable(psoc, &req);
}

int osif_twt_send_responder_disable_cmd(struct wlan_objmgr_psoc *psoc,
					uint8_t pdev_id)
{
	struct twt_disable_param req = {0};

	req.pdev_id = pdev_id;
	req.ext_conf_present = true;

	return osif_twt_responder_disable(psoc, &req);
}

int osif_twt_setup_req(struct wlan_objmgr_vdev *vdev,
		       struct nlattr *twt_param_attr)
{
	struct nlattr *tb2[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAX + 1];
	struct wlan_objmgr_psoc *psoc;
	int ret = 0;
	uint8_t vdev_id, pdev_id;
	struct twt_add_dialog_param params = {0};
	uint32_t congestion_timeout = 0;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		osif_err("NULL psoc");
		return -EINVAL;
	}

	vdev_id = wlan_vdev_get_id(vdev);
	ret = osif_is_twt_command_allowed(vdev, vdev_id, psoc);
	if (ret)
		return ret;

	if (osif_twt_setup_conc_allowed(psoc, vdev_id)) {
		osif_err_rl("TWT setup reject: SCC or MCC concurrency exists");
		return -EAGAIN;
	}

	ret = wlan_cfg80211_nla_parse_nested(tb2,
					 QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAX,
					 twt_param_attr,
					 qca_wlan_vendor_twt_add_dialog_policy);
	if (ret)
		return ret;

	ret = osif_fill_peer_macaddr(vdev, params.peer_macaddr.bytes);
	if (ret)
		return ret;

	params.vdev_id = vdev_id;
	pdev_id = wlan_get_pdev_id_from_vdev_id(psoc, vdev_id, WLAN_TWT_ID);

	ret = osif_twt_parse_add_dialog_attrs(tb2, &params);
	if (ret)
		return ret;

	ucfg_twt_cfg_get_congestion_timeout(psoc, &congestion_timeout);

	if (congestion_timeout) {
		ret = osif_twt_send_requestor_disable_cmd(psoc, pdev_id);
		if (ret) {
			osif_err("Failed to disable TWT");
			return ret;
		}
	}

	ucfg_twt_cfg_set_congestion_timeout(psoc, 0);

	ret = osif_twt_send_requestor_enable_cmd(psoc, pdev_id);
	if (ret) {
		osif_err("Failed to Enable TWT");
		return ret;
	}

	return osif_send_twt_setup_req(vdev, psoc, &params);
}

/**
 * osif_twt_handle_renego_failure() - Upon re-nego failure send TWT teardown
 *
 * @adapter: Adapter pointer
 * @event: Pointer to Add dialog complete event structure
 *
 * Upon re-negotiation failure, this function constructs TWT teardown
 * message to the target.
 *
 * Return: None
 */
void
osif_twt_handle_renego_failure(struct wlan_objmgr_psoc *psoc,
		       struct twt_add_dialog_complete_event *event)
{
	struct twt_del_dialog_param params = {0};
	uint8_t pdev_id;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_vdev *vdev;
	uint32_t vdev_id;

	if (!event)
		return;

	vdev_id = event->params.vdev_id;
	pdev_id = wlan_get_pdev_id_from_vdev_id(psoc, vdev_id,
						WLAN_TWT_ID);
	if (pdev_id == WLAN_INVALID_PDEV_ID) {
		osif_err("Invalid pdev id");
		return;
	}

	pdev = wlan_objmgr_get_pdev_by_id(psoc, pdev_id, WLAN_TWT_ID);
	if (!pdev) {
		osif_err("Invalid pdev");
		return;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev, vdev_id,
						    WLAN_TWT_ID);
	if (!vdev) {
		osif_err("vdev object is NULL");
		goto end;
	}

	qdf_copy_macaddr(&params.peer_macaddr, &event->params.peer_macaddr);
	params.vdev_id = vdev_id;
	params.dialog_id = event->params.dialog_id;

	osif_debug("renego: twt_terminate: vdev_id:%d dialog_id:%d peer mac_addr "
		  QDF_MAC_ADDR_FMT, params.vdev_id, params.dialog_id,
		  QDF_MAC_ADDR_REF(params.peer_macaddr.bytes));

	osif_send_sta_twt_teardown_req(vdev, psoc, &params);

	wlan_objmgr_vdev_release_ref(vdev, WLAN_TWT_ID);

end:
	wlan_objmgr_pdev_release_ref(pdev, WLAN_TWT_ID);
}

int osif_twt_sap_teardown_req(struct wlan_objmgr_vdev *vdev,
			      struct nlattr *twt_param_attr)
{
	return 0;
}

int osif_twt_sta_teardown_req(struct wlan_objmgr_vdev *vdev,
			      struct nlattr *twt_param_attr)
{
	struct nlattr *tb2[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAX + 1];
	struct wlan_objmgr_psoc *psoc;
	int ret = 0;
	uint8_t vdev_id, pdev_id;
	struct twt_del_dialog_param params = {0};

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		osif_err("NULL psoc");
		return -EINVAL;
	}

	vdev_id = wlan_vdev_get_id(vdev);

	if (!wlan_cm_is_vdev_connected(vdev)) {
		osif_err_rl("Not associated!, vdev %d", vdev_id);
		/*
		 * Return success, since STA is not associated and there is
		 * no TWT session.
		 */
		return 0;
	}

	if (wlan_cm_host_roam_in_progress(psoc, vdev_id))
		return -EBUSY;

	if (wlan_get_vdev_status(vdev)) {
		osif_err_rl("Scan in progress");
		return -EBUSY;
	}

	ret = wlan_cfg80211_nla_parse_nested(tb2,
					 QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAX,
					 twt_param_attr,
					 qca_wlan_vendor_twt_add_dialog_policy);
	if (ret)
		return ret;

	ret = osif_fill_peer_macaddr(vdev, params.peer_macaddr.bytes);
	if (ret)
		return ret;

	params.vdev_id = vdev_id;
	pdev_id = wlan_get_pdev_id_from_vdev_id(psoc, vdev_id, WLAN_TWT_ID);

	ret = osif_twt_parse_del_dialog_attrs(tb2, &params);
	if (ret)
		return ret;

	return osif_send_sta_twt_teardown_req(vdev, psoc, &params);
}
