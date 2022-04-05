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
#include <osif_twt_internal.h>
#include <wlan_osif_request_manager.h>
#include <wlan_osif_priv.h>
#include "wlan_cp_stats_mc_ucfg_api.h"
#include "wlan_mlme_ucfg_api.h"
#include "wlan_cp_stats_ucfg_api.h"

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

static const struct nla_policy
qca_wlan_vendor_twt_resume_dialog_policy[QCA_WLAN_VENDOR_ATTR_TWT_RESUME_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_TWT_RESUME_FLOW_ID] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_TWT_RESUME_NEXT_TWT] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_TWT_RESUME_NEXT_TWT_SIZE] = {.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_TWT_RESUME_NEXT2_TWT] = {.type = NLA_U32 },
};

static const struct nla_policy
qca_wlan_vendor_twt_nudge_dialog_policy[QCA_WLAN_VENDOR_ATTR_TWT_NUDGE_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_TWT_NUDGE_FLOW_ID] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_TWT_NUDGE_WAKE_TIME] = {.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_TWT_NUDGE_NEXT_TWT_SIZE] = {.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_TWT_NUDGE_MAC_ADDR] = VENDOR_NLA_POLICY_MAC_ADDR,
};

static const struct nla_policy
qca_wlan_vendor_twt_stats_dialog_policy[QCA_WLAN_VENDOR_ATTR_TWT_STATS_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_TWT_STATS_FLOW_ID] = {.type = NLA_U8 },
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

int osif_fill_peer_macaddr(struct wlan_objmgr_vdev *vdev, uint8_t *mac_addr)
{
	struct wlan_objmgr_peer *peer;

	peer = wlan_objmgr_vdev_try_get_bsspeer(vdev, WLAN_TWT_ID);
	if (!peer) {
		osif_err("peer is null");
		return -EINVAL;
	}
	wlan_peer_obj_lock(peer);
	qdf_mem_copy(mac_addr, wlan_peer_get_macaddr(peer), QDF_MAC_ADDR_SIZE);
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

/**
 * osif_send_twt_pause_req() - Send TWT pause dialog command to target
 * @vdev: vdev
 * @psoc: psoc
 * @twt_params: Pointer to pause dialog cmd params structure
 *
 * Return: 0 on success, negative value on failure
 */
static int
osif_send_twt_pause_req(struct wlan_objmgr_vdev *vdev,
			struct wlan_objmgr_psoc *psoc,
			struct twt_pause_dialog_cmd_param *twt_params)
{
	QDF_STATUS status;
	int ret = 0, twt_cmd;
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

	status = ucfg_twt_pause_req(psoc, twt_params, context);
	if (QDF_IS_STATUS_ERROR(status)) {
		osif_err("Failed to send pause dialog command");
		ret = qdf_status_to_os_return(status);
		goto cleanup;
	}

	twt_cmd = HOST_TWT_PAUSE_DIALOG_CMDID;
	status = osif_twt_ack_wait_response(psoc, request, twt_cmd);
	if (QDF_IS_STATUS_ERROR(status)) {
		ret = qdf_status_to_os_return(status);
		goto cleanup;
	}

	ack_priv = osif_request_priv(request);
	if (ack_priv->status != HOST_TWT_PAUSE_STATUS_OK) {
		osif_err("Received TWT ack error:%d. Reset twt command",
			 ack_priv->status);

		switch (ack_priv->status) {
		case HOST_TWT_PAUSE_STATUS_INVALID_PARAM:
		case HOST_TWT_PAUSE_STATUS_ALREADY_PAUSED:
		case HOST_TWT_PAUSE_STATUS_UNKNOWN_ERROR:
			ret = -EINVAL;
			break;
		case HOST_TWT_PAUSE_STATUS_DIALOG_ID_NOT_EXIST:
			ret = -EAGAIN;
			break;
		case HOST_TWT_PAUSE_STATUS_DIALOG_ID_BUSY:
			ret = -EINPROGRESS;
			break;
		case HOST_TWT_PAUSE_STATUS_NO_RESOURCE:
			ret = -ENOMEM;
			break;
		case HOST_TWT_PAUSE_STATUS_CHAN_SW_IN_PROGRESS:
		case HOST_TWT_PAUSE_STATUS_ROAM_IN_PROGRESS:
		case HOST_TWT_PAUSE_STATUS_SCAN_IN_PROGRESS:
			ret = -EBUSY;
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

/**
 * osif_send_twt_resume_req() - Send TWT resume dialog command to target
 * @vdev: vdev
 * @psoc: psoc
 * @twt_params: Pointer to resume dialog cmd params structure
 *
 * Return: 0 on success, negative value on failure
 */
static int
osif_send_twt_resume_req(struct wlan_objmgr_vdev *vdev,
			struct wlan_objmgr_psoc *psoc,
			struct twt_resume_dialog_cmd_param *twt_params)
{
	QDF_STATUS status;
	int ret = 0, twt_cmd;
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

	status = ucfg_twt_resume_req(psoc, twt_params, context);
	if (QDF_IS_STATUS_ERROR(status)) {
		osif_err("Failed to send resume dialog command");
		ret = qdf_status_to_os_return(status);
		goto cleanup;
	}

	twt_cmd = HOST_TWT_RESUME_DIALOG_CMDID;
	status = osif_twt_ack_wait_response(psoc, request, twt_cmd);
	if (QDF_IS_STATUS_ERROR(status)) {
		ret = qdf_status_to_os_return(status);
		goto cleanup;
	}

	ack_priv = osif_request_priv(request);
	if (ack_priv->status != HOST_TWT_RESUME_STATUS_OK) {
		osif_err("Received TWT ack error:%d. Reset twt command",
			 ack_priv->status);

		switch (ack_priv->status) {
		case HOST_TWT_RESUME_STATUS_INVALID_PARAM:
		case HOST_TWT_RESUME_STATUS_UNKNOWN_ERROR:
			ret = -EINVAL;
			break;
		case HOST_TWT_RESUME_STATUS_DIALOG_ID_NOT_EXIST:
		case HOST_TWT_RESUME_STATUS_NOT_PAUSED:
			ret = -EAGAIN;
			break;
		case HOST_TWT_RESUME_STATUS_DIALOG_ID_BUSY:
			ret = -EINPROGRESS;
			break;
		case HOST_TWT_RESUME_STATUS_NO_RESOURCE:
			ret = -ENOMEM;
			break;
		case HOST_TWT_RESUME_STATUS_CHAN_SW_IN_PROGRESS:
		case HOST_TWT_RESUME_STATUS_ROAM_IN_PROGRESS:
		case HOST_TWT_RESUME_STATUS_SCAN_IN_PROGRESS:
			ret = -EBUSY;
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

/**
 * osif_send_twt_nudge_req() - Send TWT nudge dialog command to target
 * @vdev: vdev
 * @psoc: psoc
 * @twt_params: Pointer to nudge dialog cmd params structure
 *
 * Return: 0 on success, negative value on failure
 */
static int
osif_send_twt_nudge_req(struct wlan_objmgr_vdev *vdev,
			struct wlan_objmgr_psoc *psoc,
			struct twt_nudge_dialog_cmd_param *twt_params)
{
	QDF_STATUS status;
	int ret = 0, twt_cmd;
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

	status = ucfg_twt_nudge_req(psoc, twt_params, context);
	if (QDF_IS_STATUS_ERROR(status)) {
		osif_err("Failed to send nudge dialog command");
		ret = qdf_status_to_os_return(status);
		goto cleanup;
	}

	twt_cmd = HOST_TWT_NUDGE_DIALOG_CMDID;

	status = osif_twt_ack_wait_response(psoc, request, twt_cmd);
	if (QDF_IS_STATUS_ERROR(status)) {
		ret = qdf_status_to_os_return(status);
		goto cleanup;
	}

	ack_priv = osif_request_priv(request);
	if (ack_priv->status != HOST_TWT_NUDGE_STATUS_OK) {
		osif_err("Received TWT ack error:%d. Reset twt command",
			 ack_priv->status);

		switch (ack_priv->status) {
		case HOST_TWT_NUDGE_STATUS_INVALID_PARAM:
		case HOST_TWT_NUDGE_STATUS_UNKNOWN_ERROR:
			ret = -EINVAL;
			break;
		case HOST_TWT_NUDGE_STATUS_DIALOG_ID_NOT_EXIST:
			ret = -EAGAIN;
			break;
		case HOST_TWT_NUDGE_STATUS_DIALOG_ID_BUSY:
			ret = -EINPROGRESS;
			break;
		case HOST_TWT_NUDGE_STATUS_NO_RESOURCE:
			ret = -ENOMEM;
			break;
		case HOST_TWT_NUDGE_STATUS_CHAN_SW_IN_PROGRESS:
		case HOST_TWT_NUDGE_STATUS_ROAM_IN_PROGRESS:
		case HOST_TWT_NUDGE_STATUS_SCAN_IN_PROGRESS:
			ret = -EBUSY;
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
					uint8_t pdev_id, uint32_t reason)
{
	struct twt_disable_param req = {0};

	req.pdev_id = pdev_id;
	req.ext_conf_present = true;
	req.dis_reason_code = reason;

	return osif_twt_requestor_disable(psoc, &req);
}

int osif_twt_send_responder_disable_cmd(struct wlan_objmgr_psoc *psoc,
					uint8_t pdev_id, uint32_t reason)
{
	struct twt_disable_param req = {0};

	req.pdev_id = pdev_id;
	req.ext_conf_present = true;
	req.dis_reason_code = reason;

	return osif_twt_responder_disable(psoc, &req);
}

void osif_twt_teardown_in_ps_disable(struct wlan_objmgr_psoc *psoc,
				     struct qdf_mac_addr *mac_addr,
				     uint8_t vdev_id)
{
	struct twt_del_dialog_param params = {0};
	int ret;
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id, WLAN_TWT_ID);
	if (!vdev) {
		osif_err("vdev is NULL");
		return;
	}

	params.dialog_id = TWT_ALL_SESSIONS_DIALOG_ID;
	params.vdev_id = vdev_id;
	qdf_copy_macaddr(&params.peer_macaddr, mac_addr);

	if (ucfg_twt_is_setup_done(psoc, mac_addr, params.dialog_id)) {
		osif_debug("vdev%d: Terminate existing TWT session %d due to ps disable",
			  params.vdev_id, params.dialog_id);
		ret = osif_send_sta_twt_teardown_req(vdev, psoc, &params);
		if (ret)
			osif_debug("TWT teardown is failed on vdev: %d",
				   vdev_id);
	}
	wlan_objmgr_vdev_release_ref(vdev, WLAN_TWT_ID);
}

int osif_twt_get_capabilities(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_psoc *psoc;
	enum QDF_OPMODE mode;
	QDF_STATUS status;
	uint8_t vdev_id;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc)
		return -EINVAL;

	vdev_id = wlan_vdev_get_id(vdev);
	mode = wlan_vdev_mlme_get_opmode(vdev);
	if (mode != QDF_STA_MODE && mode != QDF_P2P_CLIENT_MODE)
		return -EOPNOTSUPP;

	if (!wlan_cm_is_vdev_connected(vdev)) {
		osif_err_rl("Not associated!, vdev %d mode %d", vdev_id, mode);
		return -EAGAIN;
	}

	if (wlan_cm_host_roam_in_progress(psoc, vdev_id))
		return -EBUSY;

	status = osif_twt_send_get_capabilities_response(psoc, vdev);
	if (QDF_IS_STATUS_ERROR(status))
		osif_err_rl("TWT: Get capabilities failed");

	return qdf_status_to_os_return(status);
}

int osif_twt_setup_req(struct wlan_objmgr_vdev *vdev,
		       struct nlattr *twt_param_attr)
{
	struct nlattr *tb2[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAX + 1];
	struct wlan_objmgr_psoc *psoc;
	int ret = 0;
	uint8_t vdev_id, pdev_id;
	struct twt_add_dialog_param params = {0};
	uint32_t congestion_timeout = 0, reason;
	uint8_t peer_cap;
	QDF_STATUS qdf_status;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		osif_err("NULL psoc");
		return -EINVAL;
	}

	vdev_id = wlan_vdev_get_id(vdev);

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

	qdf_status = ucfg_twt_get_peer_capabilities(psoc, &params.peer_macaddr,
						    &peer_cap);
	if (QDF_IS_STATUS_ERROR(qdf_status))
		return -EINVAL;

	if (params.flag_bcast && !(peer_cap & WLAN_TWT_CAPA_BROADCAST)) {
		osif_err_rl("TWT setup reject: TWT Broadcast not supported");
		return -EOPNOTSUPP;
	}

	if (!params.flag_bcast && !(peer_cap & WLAN_TWT_CAPA_RESPONDER)) {
		osif_err_rl("TWT setup reject: TWT responder not supported");
		return -EOPNOTSUPP;
	}

	ret = osif_is_twt_command_allowed(vdev, vdev_id, psoc);
	if (ret)
		return ret;

	if (osif_twt_setup_conc_allowed(psoc, vdev_id)) {
		osif_err_rl("TWT setup reject: SCC or MCC concurrency exists");
		return -EAGAIN;
	}

	ucfg_twt_cfg_get_congestion_timeout(psoc, &congestion_timeout);

	if (congestion_timeout) {
		reason = HOST_TWT_DISABLE_REASON_CHANGE_CONGESTION_TIMEOUT;
		ret = osif_twt_send_requestor_disable_cmd(psoc, pdev_id,
							  reason);
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
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAX + 1];
	struct wlan_objmgr_psoc *psoc;
	int id, id1, ret = 0;
	uint8_t vdev_id;
	struct twt_del_dialog_param params = {0};
	QDF_STATUS status;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		osif_err("NULL psoc");
		return -EINVAL;
	}

	vdev_id = wlan_vdev_get_id(vdev);
	params.vdev_id = vdev_id;

	ret = wlan_cfg80211_nla_parse_nested(tb,
					 QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAX,
					 twt_param_attr,
					 qca_wlan_vendor_twt_add_dialog_policy);
	if (ret)
		return ret;

	id = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_ID;
	id1 = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAC_ADDR;
	if (tb[id] && tb[id1]) {
		params.dialog_id = nla_get_u8(tb[id]);
		nla_memcpy(params.peer_macaddr.bytes, tb[id1],
			   QDF_MAC_ADDR_SIZE);
	} else if (!tb[id] && !tb[id1]) {
		struct qdf_mac_addr bcast_addr = QDF_MAC_ADDR_BCAST_INIT;

		params.dialog_id = TWT_ALL_SESSIONS_DIALOG_ID;
		qdf_copy_macaddr(&params.peer_macaddr, &bcast_addr);
	} else {
		osif_err_rl("get_params dialog_id or mac_addr is missing");
		return -EINVAL;
	}

	if (!params.dialog_id)
		params.dialog_id = TWT_ALL_SESSIONS_DIALOG_ID;

	if (params.dialog_id != TWT_ALL_SESSIONS_DIALOG_ID &&
	    qdf_is_macaddr_broadcast(&params.peer_macaddr)) {
		osif_err("Bcast MAC valid with dlg_id:%d but here dlg_id is:%d",
			TWT_ALL_SESSIONS_DIALOG_ID, params.dialog_id);
		return -EINVAL;
	}

	osif_debug("vdev_id %d dialog_id %d peer mac_addr "
		  QDF_MAC_ADDR_FMT, params.vdev_id, params.dialog_id,
		  QDF_MAC_ADDR_REF(params.peer_macaddr.bytes));

	status = ucfg_twt_teardown_req(psoc, &params, NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		osif_err("Failed to send del dialog command");
		ret = qdf_status_to_os_return(status);
	}

	return ret;
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

static void
osif_twt_concurrency_update_on_scc(struct wlan_objmgr_pdev *pdev,
				   void *object, void *arg)
{
	struct wlan_objmgr_vdev *vdev = object;
	struct twt_conc_context *twt_arg = arg;
	QDF_STATUS status;
	uint8_t pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
	uint32_t reason;

	if (vdev->vdev_mlme.vdev_opmode == QDF_SAP_MODE &&
	    vdev->vdev_mlme.mlme_state == WLAN_VDEV_S_UP) {
		osif_debug("Concurrency exist on SAP vdev");
		reason = HOST_TWT_DISABLE_REASON_CONCURRENCY_SCC;
		status = osif_twt_send_responder_disable_cmd(twt_arg->psoc,
							     pdev_id, reason);
		if (QDF_IS_STATUS_ERROR(status)) {
			osif_err("TWT responder disable cmd to fw failed");
			return;
		}
		ucfg_twt_update_beacon_template();
	}

	if (vdev->vdev_mlme.vdev_opmode == QDF_STA_MODE &&
	    vdev->vdev_mlme.mlme_state == WLAN_VDEV_S_UP) {
		osif_debug("Concurrency exist on STA vdev");
		reason = HOST_TWT_DISABLE_REASON_CONCURRENCY_SCC;
		status = osif_twt_send_requestor_disable_cmd(twt_arg->psoc,
							     pdev_id, reason);
		if (QDF_IS_STATUS_ERROR(status)) {
			osif_err("TWT requestor disable cmd to fw failed");
			return;
		}
	}
}

static void
osif_twt_concurrency_update_on_mcc(struct wlan_objmgr_pdev *pdev,
				   void *object, void *arg)
{
	struct wlan_objmgr_vdev *vdev = object;
	struct twt_conc_context *twt_arg = arg;
	QDF_STATUS status;
	uint8_t pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
	uint32_t reason;

	if (vdev->vdev_mlme.vdev_opmode == QDF_SAP_MODE &&
	    vdev->vdev_mlme.mlme_state == WLAN_VDEV_S_UP) {
		osif_debug("Concurrency exist on SAP vdev");
		reason = HOST_TWT_DISABLE_REASON_CONCURRENCY_MCC;
		status = osif_twt_send_responder_disable_cmd(twt_arg->psoc,
							     pdev_id, reason);
		if (QDF_IS_STATUS_ERROR(status)) {
			osif_err("TWT responder disable cmd to fw failed");
			return;
		}
		ucfg_twt_update_beacon_template();
	}

	if (vdev->vdev_mlme.vdev_opmode == QDF_STA_MODE &&
	    vdev->vdev_mlme.mlme_state == WLAN_VDEV_S_UP) {
		osif_debug("Concurrency exist on STA vdev");
		reason = HOST_TWT_DISABLE_REASON_CONCURRENCY_MCC;
		status = osif_twt_send_requestor_disable_cmd(twt_arg->psoc,
							     pdev_id, reason);
		if (QDF_IS_STATUS_ERROR(status)) {
			osif_err("TWT requestor disable cmd to fw failed");
			return;
		}
	}
}

static void
osif_twt_concurrency_update_on_dbs(struct wlan_objmgr_pdev *pdev,
				   void *object, void *arg)
{
	struct wlan_objmgr_vdev *vdev = object;
	struct twt_conc_context *twt_arg = arg;
	QDF_STATUS status;
	uint8_t pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);

	if (vdev->vdev_mlme.vdev_opmode == QDF_SAP_MODE &&
	    vdev->vdev_mlme.mlme_state == WLAN_VDEV_S_UP) {
		osif_debug("SAP vdev exist");
		status = osif_twt_send_responder_enable_cmd(twt_arg->psoc,
							    pdev_id);
		if (QDF_IS_STATUS_ERROR(status)) {
			osif_err("TWT responder enable cmd to firmware failed");
			return;
		}
		ucfg_twt_update_beacon_template();
	}

	if (vdev->vdev_mlme.vdev_opmode == QDF_STA_MODE &&
	    vdev->vdev_mlme.mlme_state == WLAN_VDEV_S_UP) {
		osif_debug("STA vdev exist");
		status = osif_twt_send_requestor_enable_cmd(twt_arg->psoc,
							    pdev_id);
		if (QDF_IS_STATUS_ERROR(status)) {
			osif_err("TWT requestor enable cmd to firmware failed");
			return;
		}
	}
}

void osif_twt_concurrency_update_handler(struct wlan_objmgr_psoc *psoc,
					 struct wlan_objmgr_pdev *pdev)
{
	uint32_t num_connections, sap_count, sta_count;
	QDF_STATUS status;
	struct twt_conc_context twt_arg;
	uint8_t pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);

	num_connections = policy_mgr_get_connection_count(psoc);
	sta_count = policy_mgr_mode_specific_connection_count(psoc,
							      PM_STA_MODE,
							      NULL);
	sap_count = policy_mgr_mode_specific_connection_count(psoc,
							      PM_SAP_MODE,
							      NULL);
	twt_arg.psoc = psoc;

	osif_debug("Total connection %d, sta_count %d, sap_count %d",
		  num_connections, sta_count, sap_count);
	switch (num_connections) {
	case 1:
		if (sta_count == 1) {
			osif_twt_send_requestor_enable_cmd(psoc, pdev_id);
		} else if (sap_count == 1) {
			osif_twt_send_responder_enable_cmd(psoc, pdev_id);
			ucfg_twt_update_beacon_template();
		}
		break;
	case 2:
		if (policy_mgr_current_concurrency_is_scc(psoc)) {
			status = wlan_objmgr_pdev_iterate_obj_list(
					pdev,
					WLAN_VDEV_OP,
					osif_twt_concurrency_update_on_scc,
					&twt_arg, 0,
					WLAN_TWT_ID);
			if (QDF_IS_STATUS_ERROR(status)) {
				osif_err("2port conc: SAP/STA not in SCC");
				return;
			}
		} else if (policy_mgr_current_concurrency_is_mcc(psoc)) {
			status = wlan_objmgr_pdev_iterate_obj_list(
					pdev,
					WLAN_VDEV_OP,
					osif_twt_concurrency_update_on_mcc,
					&twt_arg, 0,
					WLAN_TWT_ID);
			if (QDF_IS_STATUS_ERROR(status)) {
				osif_err("2port conc: SAP/STA not in MCC");
				return;
			}
		} else if (policy_mgr_is_current_hwmode_dbs(psoc)) {
			status = wlan_objmgr_pdev_iterate_obj_list(
					pdev,
					WLAN_VDEV_OP,
					osif_twt_concurrency_update_on_dbs,
					&twt_arg, 0,
					WLAN_TWT_ID);
			if (QDF_IS_STATUS_ERROR(status)) {
				osif_err("SAP not in DBS case");
				return;
			}
		}
		break;
	case 3:
		if (policy_mgr_current_concurrency_is_scc(psoc)) {
			status = wlan_objmgr_pdev_iterate_obj_list(
					pdev,
					WLAN_VDEV_OP,
					osif_twt_concurrency_update_on_scc,
					&twt_arg, 0,
					WLAN_TWT_ID);
			if (QDF_IS_STATUS_ERROR(status)) {
				osif_err("3port conc: SAP/STA not in SCC");
				return;
			}
		} else if (policy_mgr_current_concurrency_is_mcc(psoc)) {
			status = wlan_objmgr_pdev_iterate_obj_list(
					pdev,
					WLAN_VDEV_OP,
					osif_twt_concurrency_update_on_mcc,
					&twt_arg, 0,
					WLAN_TWT_ID);
			if (QDF_IS_STATUS_ERROR(status)) {
				osif_err("3port conc: SAP/STA not in MCC");
				return;
			}
		}
		break;
	default:
		osif_err("Unexpected number of connections: %d",
			 num_connections);
		break;
	}
}

int osif_twt_pause_req(struct wlan_objmgr_vdev *vdev,
		       struct nlattr *twt_param_attr)
{
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAX + 1];
	struct wlan_objmgr_psoc *psoc;
	int ret = 0, id;
	uint32_t vdev_id;
	struct  twt_pause_dialog_cmd_param params = {0};

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		osif_err("NULL psoc");
		return -EINVAL;
	}

	vdev_id = wlan_vdev_get_id(vdev);
	ret = osif_is_twt_command_allowed(vdev, vdev_id, psoc);
	if (ret)
		return ret;

	ret = osif_fill_peer_macaddr(vdev, params.peer_macaddr.bytes);
	if (ret)
		return ret;

	if (twt_param_attr) {
		ret = wlan_cfg80211_nla_parse_nested(tb,
					QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAX,
					twt_param_attr,
					qca_wlan_vendor_twt_add_dialog_policy);
		if (ret)
			return ret;

		id = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_ID;
		if (tb[id])
			params.dialog_id = nla_get_u8(tb[id]);
		else
			osif_debug("TWT: FLOW_ID not specified. set to zero");
	} else {
		osif_debug("TWT param not present. flow id set to zero");
	}

	osif_debug("twt_pause: vdev_id %d dialog_id %d peer mac_addr "
		  QDF_MAC_ADDR_FMT, vdev_id, params.dialog_id,
		  QDF_MAC_ADDR_REF(params.peer_macaddr.bytes));

	return osif_send_twt_pause_req(vdev, psoc, &params);
}

int osif_twt_resume_req(struct wlan_objmgr_vdev *vdev,
		       struct nlattr *twt_param_attr)
{
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAX + 1];
	struct wlan_objmgr_psoc *psoc;
	int ret = 0;
	uint32_t vdev_id;
	int id, id2;
	struct  twt_resume_dialog_cmd_param params = {0};

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		osif_err("NULL psoc");
		return -EINVAL;
	}

	vdev_id = wlan_vdev_get_id(vdev);

	ret = osif_is_twt_command_allowed(vdev, vdev_id, psoc);
	if (ret)
		return ret;

	ret = osif_fill_peer_macaddr(vdev, params.peer_macaddr.bytes);
	if (ret)
		return ret;

	ret = wlan_cfg80211_nla_parse_nested(tb,
				QCA_WLAN_VENDOR_ATTR_TWT_RESUME_MAX,
				twt_param_attr,
				qca_wlan_vendor_twt_resume_dialog_policy);
	if (ret)
		return ret;

	id = QCA_WLAN_VENDOR_ATTR_TWT_RESUME_FLOW_ID;
	if (tb[id])
		params.dialog_id = nla_get_u8(tb[id]);
	else
		osif_debug("TWT_RESUME_FLOW_ID not specified. set to zero");

	id = QCA_WLAN_VENDOR_ATTR_TWT_RESUME_NEXT_TWT;
	id2 = QCA_WLAN_VENDOR_ATTR_TWT_RESUME_NEXT2_TWT;
	if (tb[id2])
		params.sp_offset_us = nla_get_u32(tb[id2]);
	else if (tb[id])
		params.sp_offset_us = nla_get_u8(tb[id]);
	else
		params.sp_offset_us = 0;

	id = QCA_WLAN_VENDOR_ATTR_TWT_RESUME_NEXT_TWT_SIZE;
	if (tb[id]) {
		params.next_twt_size = nla_get_u32(tb[id]);
	} else {
		osif_err_rl("TWT_RESUME NEXT_TWT_SIZE is must");
		return -EINVAL;
	}
	if (params.next_twt_size > TWT_MAX_NEXT_TWT_SIZE)
		return -EINVAL;

	osif_debug("twt_resume: vdev_id %d dialog_id %d peer mac_addr "
		   QDF_MAC_ADDR_FMT, vdev_id, params.dialog_id,
		   QDF_MAC_ADDR_REF(params.peer_macaddr.bytes));

	return osif_send_twt_resume_req(vdev, psoc, &params);
}

int osif_twt_nudge_req(struct wlan_objmgr_vdev *vdev,
		       struct nlattr *twt_param_attr)
{
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAX + 1];
	struct wlan_objmgr_psoc *psoc;
	int ret = 0, id;
	uint32_t vdev_id;
	struct  twt_nudge_dialog_cmd_param params = {0};

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		osif_err("NULL psoc");
		return -EINVAL;
	}

	vdev_id = wlan_vdev_get_id(vdev);
	ret = osif_is_twt_command_allowed(vdev, vdev_id, psoc);
	if (ret)
		return ret;

	ret = wlan_cfg80211_nla_parse_nested(tb,
				QCA_WLAN_VENDOR_ATTR_TWT_NUDGE_MAX,
				twt_param_attr,
				qca_wlan_vendor_twt_nudge_dialog_policy);
	if (ret)
		return ret;

	id = QCA_WLAN_VENDOR_ATTR_TWT_NUDGE_MAC_ADDR;
	if (tb[id]) {
		nla_memcpy(params.peer_macaddr.bytes, tb[id],
			   QDF_MAC_ADDR_SIZE);
	} else {
		ret = osif_fill_peer_macaddr(vdev, params.peer_macaddr.bytes);
		if (ret)
			return ret;
	}

	id = QCA_WLAN_VENDOR_ATTR_TWT_NUDGE_FLOW_ID;
	if (!tb[id]) {
		osif_debug("TWT: FLOW_ID not specified");
		return -EINVAL;
	}
	params.dialog_id = nla_get_u8(tb[id]);

	id = QCA_WLAN_VENDOR_ATTR_TWT_NUDGE_WAKE_TIME;
	if (!tb[id]) {
		osif_debug("TWT: NEXT_TWT_SIZE not specified");
		return -EINVAL;
	}

	params.suspend_duration = nla_get_u32(tb[id]);

	id = QCA_WLAN_VENDOR_ATTR_TWT_NUDGE_NEXT_TWT_SIZE;
	if (!tb[id]) {
		osif_debug("TWT: NEXT_TWT_SIZE not specified");
		return -EINVAL;
	}
	params.next_twt_size = nla_get_u32(tb[id]);

	osif_debug("twt_nudge: vdev_id %d dialog_id %d ", params.vdev_id,
		   params.dialog_id);
	osif_debug("twt_nudge: suspend_duration %d next_twt_size %d",
		   params.suspend_duration, params.next_twt_size);
	osif_debug("peer mac_addr " QDF_MAC_ADDR_FMT,
		   QDF_MAC_ADDR_REF(params.peer_macaddr.bytes));

	return osif_send_twt_nudge_req(vdev, psoc, &params);
}

static uint32_t
osif_twt_get_params_resp_len(struct twt_session_stats_info *params)
{
	uint32_t len = nla_total_size(0);

	/* QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAC_ADDR */
	len += nla_total_size(QDF_MAC_ADDR_SIZE);

	/* QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_ID */
	len += nla_total_size(sizeof(u8));

	/* QCA_WLAN_VENDOR_ATTR_TWT_SETUP_BCAST */
	len += nla_total_size(sizeof(u8));

	/* QCA_WLAN_VENDOR_ATTR_TWT_SETUP_TRIGGER */
	len += nla_total_size(sizeof(u8));

	/* QCA_WLAN_VENDOR_ATTR_TWT_SETUP_ANNOUNCE */
	len += nla_total_size(sizeof(u8));

	/* QCA_WLAN_VENDOR_ATTR_TWT_SETUP_PROTECTION */
	len += nla_total_size(sizeof(u8));

	/* QCA_WLAN_VENDOR_ATTR_TWT_SETUP_TWT_INFO_ENABLED */
	len += nla_total_size(sizeof(u8));

	/* QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_DURATION */
	len += nla_total_size(sizeof(u32));

	/* QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_INTVL_MANTISSA */
	len += nla_total_size(sizeof(u32));

	/*QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_INTVL_EXP*/
	len += nla_total_size(sizeof(u8));

	/* QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_TIME_TSF */
	len += nla_total_size(sizeof(u64));

	/* QCA_WLAN_VENDOR_ATTR_TWT_SETUP_STATE */
	len += nla_total_size(sizeof(u32));

	/* QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_TYPE */
	len += nla_total_size(sizeof(u8));

	/* QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_INTVL2_MANTISSA */
	len += nla_total_size(sizeof(u32));

	/* QCA_WLAN_VENDOR_ATTR_TWT_SETUP_RESPONDER_PM_MODE */
	if (params->pm_responder_bit_valid)
		len += nla_total_size(sizeof(u8));

	return len;
}

static enum qca_wlan_twt_setup_state
osif_get_converted_twt_state(enum wlan_twt_session_state state)
{
	switch (state) {
	case WLAN_TWT_SETUP_STATE_NOT_ESTABLISHED:
		return QCA_WLAN_TWT_SETUP_STATE_NOT_ESTABLISHED;
	case WLAN_TWT_SETUP_STATE_ACTIVE:
		return QCA_WLAN_TWT_SETUP_STATE_ACTIVE;
	case WLAN_TWT_SETUP_STATE_SUSPEND:
		return QCA_WLAN_TWT_SETUP_STATE_SUSPEND;
	default:
		return QCA_WLAN_TWT_SETUP_STATE_NOT_ESTABLISHED;
	}
}

static QDF_STATUS
osif_twt_pack_get_params_resp_nlmsg(struct wlan_objmgr_psoc *psoc,
				    struct sk_buff *reply_skb,
				    struct twt_session_stats_info *params,
				    int num_twt_session)
{
	struct nlattr *config_attr, *nla_params;
	enum wlan_twt_session_state state;
	enum qca_wlan_twt_setup_state converted_state;
	uint64_t tsf_val;
	uint32_t wake_duration;
	uint32_t wake_intvl_mantis_us, wake_intvl_mantis_tu;
	int i, attr;

	config_attr = nla_nest_start(reply_skb,
				     QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_PARAMS);
	if (!config_attr) {
		osif_err("TWT: get_params nla_nest_start error");
		return QDF_STATUS_E_INVAL;
	}

	for (i = 0; i < num_twt_session; i++) {
		if (params[i].event_type != HOST_TWT_SESSION_SETUP &&
		    params[i].event_type != HOST_TWT_SESSION_UPDATE)
			continue;

		nla_params = nla_nest_start(reply_skb, i);
		if (!nla_params) {
			osif_err("TWT: get_params nla_nest_start error");
			return QDF_STATUS_E_INVAL;
		}

		attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAC_ADDR;
		if (nla_put(reply_skb, attr, QDF_MAC_ADDR_SIZE,
			    params[i].peer_mac.bytes)) {
			osif_err("TWT: get_params failed to put mac_addr");
			return QDF_STATUS_E_INVAL;
		}

		osif_debug("TWT: get_params peer mac_addr " QDF_MAC_ADDR_FMT,
			   QDF_MAC_ADDR_REF(params[i].peer_mac.bytes));
		attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_ID;
		if (nla_put_u8(reply_skb, attr, params[i].dialog_id)) {
			osif_err("TWT: get_params failed to put dialog_id");
			return QDF_STATUS_E_INVAL;
		}

		if (params[i].bcast) {
			attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_BCAST;
			if (nla_put_flag(reply_skb, attr)) {
				osif_err("TWT: get_params fail to put bcast");
				return QDF_STATUS_E_INVAL;
			}
		}

		if (params[i].trig) {
			attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_TRIGGER;
			if (nla_put_flag(reply_skb, attr)) {
				osif_err("TWT: get_params fail to put Trigger");
				return QDF_STATUS_E_INVAL;
			}
		}

		if (params[i].announ) {
			attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_TYPE;
			if (nla_put_flag(reply_skb, attr)) {
				osif_err("TWT: get_params fail to put Announce");
				return QDF_STATUS_E_INVAL;
			}
		}

		if (params[i].protection) {
			attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_PROTECTION;
			if (nla_put_flag(reply_skb, attr)) {
				osif_err("TWT: get_params fail to put Protect");
				return QDF_STATUS_E_INVAL;
			}
		}

		if (params[i].pm_responder_bit_valid) {
			attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_RESPONDER_PM_MODE;
			if (nla_put_u8(reply_skb, attr,
				       params[i].pm_responder_bit)) {
				osif_err("TWT: fail to put pm responder mode");
				return QDF_STATUS_E_INVAL;
			}
		}

		if (!params[i].info_frame_disabled) {
			attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_TWT_INFO_ENABLED;
			if (nla_put_flag(reply_skb, attr)) {
				osif_err("TWT: get_params put Info Enable fail");
				return QDF_STATUS_E_INVAL;
			}
		}

		attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_DURATION;
		wake_duration = (params[i].wake_dura_us /
				TWT_WAKE_DURATION_MULTIPLICATION_FACTOR);
		if (nla_put_u32(reply_skb, attr, wake_duration)) {
			osif_err("TWT: get_params failed to put Wake duration");
			return QDF_STATUS_E_INVAL;
		}

		wake_intvl_mantis_us = params[i].wake_intvl_us;
		attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_INTVL2_MANTISSA;
		if (nla_put_u32(reply_skb, attr, wake_intvl_mantis_us)) {
			osif_err("TWT: get_params failed to put Wake Interval in us");
			return QDF_STATUS_E_INVAL;
		}

		wake_intvl_mantis_tu = params[i].wake_intvl_us /
					TWT_WAKE_INTVL_MULTIPLICATION_FACTOR;
		attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_INTVL_MANTISSA;
		if (nla_put_u32(reply_skb, attr, wake_intvl_mantis_tu)) {
			osif_err("TWT: get_params failed to put Wake Interval");
			return QDF_STATUS_E_INVAL;
		}

		osif_debug("TWT: Send mantissa_us:%d, mantissa_tu:%d to userspace",
			   wake_intvl_mantis_us, wake_intvl_mantis_tu);
		attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_INTVL_EXP;
		if (nla_put_u8(reply_skb, attr, 0)) {
			osif_err("TWT: get_params put Wake Interval Exp failed");
			return QDF_STATUS_E_INVAL;
		}

		tsf_val = ((uint64_t)params[i].sp_tsf_us_hi << 32) |
			   params[i].sp_tsf_us_lo;
		osif_debug("TWT: get_params dialog_id %d TSF = 0x%llx",
			   params[i].dialog_id, tsf_val);
		attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_TIME_TSF;
		if (wlan_cfg80211_nla_put_u64(reply_skb, attr, tsf_val)) {
			osif_err("TWT: get_params failed to put TSF Value");
			return QDF_STATUS_E_INVAL;
		}
		attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_STATE;
		state = ucfg_twt_get_session_state(psoc,
				   &params[i].peer_mac,
				   params[i].dialog_id);
		converted_state = osif_get_converted_twt_state(state);
		if (nla_put_u32(reply_skb, attr, converted_state)) {
			osif_err("TWT: get_params failed to put TWT state");
			return QDF_STATUS_E_INVAL;
		}

		nla_nest_end(reply_skb, nla_params);
	}
	nla_nest_end(reply_skb, config_attr);
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
osif_twt_send_get_params_resp(struct wlan_objmgr_vdev *vdev,
			      struct twt_session_stats_info *params,
			      int num_twt_session)
{
	struct wlan_objmgr_psoc *psoc;
	struct vdev_osif_priv *osif_priv;
	struct sk_buff *reply_skb;
	uint32_t skb_len = NLMSG_HDRLEN, i;
	QDF_STATUS qdf_status;
	struct wireless_dev *wdev;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc)
		return QDF_STATUS_E_INVAL;

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
	/* Length of attribute QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_PARAMS */
	skb_len += NLA_HDRLEN;

	/* Length of twt session parameters */
	for (i = 0; i < num_twt_session; i++) {
		if (params[i].event_type == HOST_TWT_SESSION_SETUP ||
		    params[i].event_type == HOST_TWT_SESSION_UPDATE)
			skb_len += osif_twt_get_params_resp_len(params + i);
	}

	reply_skb = wlan_cfg80211_vendor_cmd_alloc_reply_skb(wdev->wiphy,
							     skb_len);
	if (!reply_skb) {
		osif_err("TWT: get_params alloc reply skb failed");
		return QDF_STATUS_E_NOMEM;
	}

	qdf_status = osif_twt_pack_get_params_resp_nlmsg(psoc, reply_skb,
							 params,
							 num_twt_session);
	if (QDF_IS_STATUS_ERROR(qdf_status))
		goto fail;

	if (cfg80211_vendor_cmd_reply(reply_skb))
		qdf_status = QDF_STATUS_E_INVAL;

	return qdf_status;
fail:
	kfree_skb(reply_skb);
	return qdf_status;
}

static QDF_STATUS
osif_twt_get_peer_session_params(struct wlan_objmgr_vdev *vdev,
				 struct twt_session_stats_info *params)
{
	struct wlan_objmgr_psoc *psoc;
	int num_twt_session = 0;
	QDF_STATUS qdf_status = QDF_STATUS_E_INVAL;

	psoc = wlan_vdev_get_psoc(vdev);

	if (!psoc)
		return qdf_status;

	num_twt_session = ucfg_cp_stats_twt_get_peer_session_params(psoc,
								    params);

	if (num_twt_session)
		qdf_status = osif_twt_send_get_params_resp(vdev, params,
							   num_twt_session);

	return qdf_status;
}

static QDF_STATUS
osif_send_inactive_session_reply(struct wlan_objmgr_vdev *vdev,
				 struct twt_session_stats_info *params)
{
	QDF_STATUS qdf_status;
	int num_twt_session = 0;

	params[num_twt_session].event_type = HOST_TWT_SESSION_UPDATE;
	num_twt_session++;

	qdf_status = osif_twt_send_get_params_resp(vdev, params,
						   num_twt_session);

	return qdf_status;
}

static int
osif_twt_sap_get_session_params(struct wlan_objmgr_vdev *vdev,
				struct nlattr *twt_param_attr)
{
	struct wlan_objmgr_psoc *psoc;
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAX + 1];
	uint16_t num_peer;
	struct twt_session_stats_info *params;
	int ret, id, id1;
	QDF_STATUS qdf_status = QDF_STATUS_E_INVAL;
	uint8_t vdev_id;

	ret = wlan_cfg80211_nla_parse_nested(
			tb, QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAX,
			twt_param_attr,
			qca_wlan_vendor_twt_add_dialog_policy);

	if (ret)
		return ret;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc)
		return -EINVAL;

	num_peer = wlan_vdev_get_peer_count(vdev);
	params = qdf_mem_malloc(TWT_PEER_MAX_SESSIONS * num_peer *
				sizeof(*params));

	if (!params)
		return -ENOMEM;

	vdev_id = wlan_vdev_get_id(vdev);
	params[0].vdev_id = vdev_id;
	id = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_ID;
	id1 = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAC_ADDR;

	if (!tb[id] || !tb[id1]) {
		osif_err_rl("TWT: get_params dialog_id or mac_addr is missing");
		goto done;
	}

	params[0].dialog_id = nla_get_u8(tb[id]);
	nla_memcpy(params[0].peer_mac.bytes, tb[id1], QDF_MAC_ADDR_SIZE);

	if (qdf_is_macaddr_broadcast(&params[0].peer_mac) &&
	    params[0].dialog_id != TWT_ALL_SESSIONS_DIALOG_ID) {
		osif_err_rl("Bcast MAC valid with dlg_id:%d but here dlg_id is:%d",
			TWT_ALL_SESSIONS_DIALOG_ID, params[0].dialog_id);
		goto done;
	}

	if (!params[0].dialog_id)
		params[0].dialog_id = TWT_ALL_SESSIONS_DIALOG_ID;

	osif_debug("TWT: get_params dialog_id %d and mac_addr "QDF_MAC_ADDR_FMT,
		   params[0].dialog_id,
		   QDF_MAC_ADDR_REF(params[0].peer_mac.bytes));
	qdf_status = osif_twt_get_peer_session_params(vdev, params);

done:
	qdf_mem_free(params);
	return qdf_status_to_os_return(qdf_status);
}

static int
osif_twt_sta_get_session_params(struct wlan_objmgr_vdev *vdev,
				struct nlattr *twt_param_attr)
{
	struct wlan_objmgr_psoc *psoc;
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAX + 1];
	struct twt_session_stats_info
		params[TWT_PSOC_MAX_SESSIONS] = { {0} };
	int ret, id;
	QDF_STATUS qdf_status;
	struct qdf_mac_addr bcast_addr = QDF_MAC_ADDR_BCAST_INIT;
	struct qdf_mac_addr peer_mac;
	uint8_t vdev_id;

	ret = wlan_cfg80211_nla_parse_nested(tb,
					 QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAX,
					 twt_param_attr,
					 qca_wlan_vendor_twt_add_dialog_policy);
	if (ret)
		return ret;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc)
		return -EINVAL;

	vdev_id = wlan_vdev_get_id(vdev);
	params[0].vdev_id = vdev_id;

	/*
	 * Currently twt_get_params nl cmd is sending only dialog_id(STA), fill
	 * mac_addr of STA in params and call osif_twt_get_peer_session_params.
	 * When twt_get_params passes mac_addr and dialog_id of STA/SAP, update
	 * both mac_addr and dialog_id in params before calling
	 * osif_twt_get_peer_session_params. dialog_id if not received,
	 * dialog_id of value 0 will be used as default.
	 */

	id = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_ID;
	if (tb[id])
		params[0].dialog_id = (uint32_t)nla_get_u8(tb[id]);
	else
		params[0].dialog_id = 0;

	if (osif_fill_peer_macaddr(vdev, peer_mac.bytes))
		return -EINVAL;

	if (params[0].dialog_id <= TWT_MAX_DIALOG_ID) {
		qdf_copy_macaddr(&params[0].peer_mac, &peer_mac);
		osif_debug("TWT: get_params peer mac_addr " QDF_MAC_ADDR_FMT,
			   QDF_MAC_ADDR_REF(params[0].peer_mac.bytes));
	} else {
		qdf_copy_macaddr(&params[0].peer_mac, &bcast_addr);
	}

	if (!ucfg_twt_is_setup_done(psoc, &peer_mac,
				    params[0].dialog_id)) {
		osif_debug("vdev%d: TWT session %d setup incomplete", vdev_id,
			   params[0].dialog_id);
		qdf_status = osif_send_inactive_session_reply(vdev, params);
		return qdf_status_to_os_return(qdf_status);
	}

	osif_debug("TWT: get_params dialog_id %d and mac_addr "QDF_MAC_ADDR_FMT,
		   params[0].dialog_id,
		   QDF_MAC_ADDR_REF(params[0].peer_mac.bytes));

	qdf_status = osif_twt_get_peer_session_params(vdev, params);

	return qdf_status_to_os_return(qdf_status);
}

int osif_twt_get_session_req(struct wlan_objmgr_vdev *vdev,
			     struct nlattr *twt_param_attr)
{
	enum QDF_OPMODE device_mode;

	device_mode = wlan_vdev_mlme_get_opmode(vdev);

	switch (device_mode) {
	case QDF_STA_MODE:
		return osif_twt_sta_get_session_params(vdev, twt_param_attr);
	case QDF_SAP_MODE:
		return osif_twt_sap_get_session_params(vdev, twt_param_attr);
	default:
		osif_err_rl("TWT get session params is not supported on %s",
			    qdf_opmode_str(device_mode));
	}

	return -EOPNOTSUPP;
}

/**
 * osif_twt_request_session_traffic_stats() - Obtains twt session traffic
 * statistics and sends response to the user space
 * @vdev: vdev
 * @dialog_id: dialog id of the twt session
 * @peer_mac: Mac address of the peer
 *
 * Return: QDF_STATUS_SUCCESS on success, else other qdf error values
 */
static QDF_STATUS
osif_twt_request_session_traffic_stats(struct wlan_objmgr_vdev *vdev,
				       uint32_t dialog_id, uint8_t *peer_mac)
{
	int errno;
	QDF_STATUS status = QDF_STATUS_E_INVAL;
	struct infra_cp_stats_event *event;

	if (!peer_mac)
		return status;
	event = wlan_cfg80211_mc_twt_get_infra_cp_stats(vdev, dialog_id,
							peer_mac, &errno);

	if (!event)
		return errno;

	status = osif_twt_get_stats_response(vdev, event->twt_infra_cp_stats,
					     event->num_twt_infra_cp_stats);
	if (QDF_IS_STATUS_ERROR(status))
		osif_err("TWT: Get_traffic_stats failed status: %d", status);

	qdf_mem_free(event->twt_infra_cp_stats);
	qdf_mem_free(event);

	return status;
}

int osif_twt_get_session_traffic_stats(struct wlan_objmgr_vdev *vdev,
				       struct nlattr *twt_param_attr)
{
	struct wlan_objmgr_psoc *psoc;
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_TWT_STATS_MAX + 1];
	int ret, id;
	QDF_STATUS qdf_status;
	uint32_t dialog_id;
	bool is_stats_tgt_cap_enabled;
	struct qdf_mac_addr peer_mac;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc)
		return -EINVAL;

	ret = wlan_cfg80211_nla_parse_nested(tb,
					     QCA_WLAN_VENDOR_ATTR_TWT_STATS_MAX,
					     twt_param_attr,
					     qca_wlan_vendor_twt_stats_dialog_policy);

	if (ret)
		return ret;

	ucfg_twt_get_twt_stats_enabled(psoc, &is_stats_tgt_cap_enabled);
	if (!is_stats_tgt_cap_enabled) {
		osif_debug("TWT Stats not supported by target");
		return -EOPNOTSUPP;
	}

	if (osif_fill_peer_macaddr(vdev, peer_mac.bytes))
		return -EINVAL;

	if (ucfg_twt_is_command_in_progress(psoc, &peer_mac,
					    TWT_ALL_SESSIONS_DIALOG_ID,
					    WLAN_TWT_STATISTICS,
					    NULL) ||
	    ucfg_twt_is_command_in_progress(psoc, &peer_mac,
					    TWT_ALL_SESSIONS_DIALOG_ID,
					    WLAN_TWT_CLEAR_STATISTICS,
					    NULL)) {
		osif_warn("Already TWT statistics or clear statistics exists");
		return -EALREADY;
	}

	id = QCA_WLAN_VENDOR_ATTR_TWT_STATS_FLOW_ID;
	if (tb[id])
		dialog_id = (uint32_t)nla_get_u8(tb[id]);
	else
		dialog_id = 0;

	osif_debug("get_stats dialog_id %d", dialog_id);
	osif_debug("get_stats peer mac_addr " QDF_MAC_ADDR_FMT,
		   QDF_MAC_ADDR_REF(peer_mac.bytes));

	if (!ucfg_twt_is_setup_done(psoc, &peer_mac, dialog_id)) {
		osif_debug("TWT session %d setup incomplete", dialog_id);
		return -EAGAIN;
	}

	ucfg_twt_set_command_in_progress(psoc, &peer_mac, dialog_id,
					 WLAN_TWT_STATISTICS);

	qdf_status = osif_twt_request_session_traffic_stats(vdev, dialog_id,
							    peer_mac.bytes);
	ucfg_twt_set_command_in_progress(psoc, &peer_mac,
					 dialog_id, WLAN_TWT_NONE);

	return qdf_status_to_os_return(qdf_status);
}

int osif_twt_clear_session_traffic_stats(struct wlan_objmgr_vdev *vdev,
					 struct nlattr *twt_param_attr)
{
	struct wlan_objmgr_psoc *psoc;
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_TWT_STATS_MAX + 1];
	int ret, id;
	uint32_t dialog_id;
	bool is_stats_tgt_cap_enabled;
	QDF_STATUS status;
	struct qdf_mac_addr peer_mac;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc)
		return -EINVAL;

	ret = wlan_cfg80211_nla_parse_nested(tb,
				QCA_WLAN_VENDOR_ATTR_TWT_STATS_MAX,
				twt_param_attr,
				qca_wlan_vendor_twt_stats_dialog_policy);

	if (ret)
		return ret;

	ucfg_twt_get_twt_stats_enabled(psoc, &is_stats_tgt_cap_enabled);
	if (!is_stats_tgt_cap_enabled) {
		osif_debug("TWT Stats not supported by target");
		return -EOPNOTSUPP;
	}

	id = QCA_WLAN_VENDOR_ATTR_TWT_STATS_FLOW_ID;
	if (!tb[id]) {
		osif_err_rl("TWT Clear stats - dialog id param is must");
		return -EINVAL;
	}

	if (osif_fill_peer_macaddr(vdev, peer_mac.bytes))
		return -EINVAL;

	dialog_id = (uint32_t)nla_get_u8(tb[id]);
	osif_debug("dialog_id %d peer mac_addr "QDF_MAC_ADDR_FMT,
		   dialog_id, QDF_MAC_ADDR_REF(peer_mac.bytes));

	status = ucfg_twt_check_all_twt_support(psoc, dialog_id);
	if (QDF_IS_STATUS_ERROR(status)) {
		osif_debug("All TWT sessions not supported by target");
		return -EOPNOTSUPP;
	}

	if (ucfg_twt_is_command_in_progress(psoc, &peer_mac,
					    TWT_ALL_SESSIONS_DIALOG_ID,
					    WLAN_TWT_STATISTICS, NULL) ||
	    ucfg_twt_is_command_in_progress(psoc, &peer_mac,
					    TWT_ALL_SESSIONS_DIALOG_ID,
					    WLAN_TWT_CLEAR_STATISTICS,
					    NULL)) {
		osif_warn("Already TWT statistics or clear statistics exists");
		return -EALREADY;
	}

	if (!ucfg_twt_is_setup_done(psoc, &peer_mac, dialog_id)) {
		osif_debug("TWT session %d setup incomplete", dialog_id);
		return -EAGAIN;
	}

	ret = wlan_cfg80211_mc_twt_clear_infra_cp_stats(vdev, dialog_id,
							peer_mac.bytes);

	return ret;
}
