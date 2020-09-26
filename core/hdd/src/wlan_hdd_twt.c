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
#include <wlan_cp_stats_mc_ucfg_api.h>

#define TWT_SETUP_COMPLETE_TIMEOUT 4000
#define TWT_DISABLE_COMPLETE_TIMEOUT 4000
#define TWT_TERMINATE_COMPLETE_TIMEOUT 4000
#define TWT_PAUSE_COMPLETE_TIMEOUT 4000
#define TWT_RESUME_COMPLETE_TIMEOUT 4000

#define TWT_FLOW_TYPE_ANNOUNCED 0
#define TWT_FLOW_TYPE_UNANNOUNCED 1

#define TWT_SETUP_WAKE_INTVL_MANTISSA_MAX       0xFFFF
#define TWT_SETUP_WAKE_DURATION_MAX             0xFFFF
#define TWT_SETUP_WAKE_INTVL_EXP_MAX            31
#define TWT_WAKE_DURATION_MULTIPLICATION_FACTOR 256
#define TWT_MAX_NEXT_TWT_SIZE                   3

/**
 * struct twt_pause_dialog_comp_ev_priv - private struct for twt pause dialog
 * @pause_dialog_comp_ev_buf: buffer from TWT pause dialog complete_event
 *
 * This TWT pause dialog private structure is registered with os_if to
 * retrieve the TWT pause dialog response event buffer.
 */
struct twt_pause_dialog_comp_ev_priv {
	struct wmi_twt_pause_dialog_complete_event_param pause_dialog_comp_ev_buf;
};

/**
 * struct twt_resume_dialog_comp_ev_priv - private struct for twt resume dialog
 * @resume_dialog_comp_ev_buf: buffer from TWT resume dialog complete_event
 *
 * This TWT resume dialog private structure is registered with os_if to
 * retrieve the TWT resume dialog response event buffer.
 */
struct twt_resume_dialog_comp_ev_priv {
	struct wmi_twt_resume_dialog_complete_event_param res_dialog_comp_ev_buf;
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
};

static const struct nla_policy
qca_wlan_vendor_twt_resume_dialog_policy[QCA_WLAN_VENDOR_ATTR_TWT_RESUME_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_TWT_RESUME_FLOW_ID] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_TWT_RESUME_NEXT_TWT] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_TWT_RESUME_NEXT_TWT_SIZE] = {.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_TWT_RESUME_NEXT2_TWT] = {.type = NLA_U32 },
};

const struct nla_policy
wlan_hdd_wifi_twt_config_policy[
	QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_MAX + 1] = {
		[QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_OPERATION] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_PARAMS] = {
			.type = NLA_NESTED},
};

/**
 * hdd_twt_setup_req_type_to_cmd() - Converts twt setup request type to twt cmd
 * @req_type: twt setup request type
 * @twt_cmd: pointer to store twt command
 *
 * Return: QDF_STATUS_SUCCESS on success, else other qdf error values
 */
static QDF_STATUS
hdd_twt_setup_req_type_to_cmd(u8 req_type, enum WMI_HOST_TWT_COMMAND *twt_cmd)
{
	if (req_type == QCA_WLAN_VENDOR_TWT_SETUP_REQUEST) {
		*twt_cmd = WMI_HOST_TWT_COMMAND_REQUEST_TWT;
	} else if (req_type == QCA_WLAN_VENDOR_TWT_SETUP_SUGGEST) {
		*twt_cmd = WMI_HOST_TWT_COMMAND_SUGGEST_TWT;
	} else if (req_type == QCA_WLAN_VENDOR_TWT_SETUP_DEMAND) {
		*twt_cmd = WMI_HOST_TWT_COMMAND_DEMAND_TWT;
	} else {
		hdd_err_rl("Invalid TWT_SETUP_REQ_TYPE %d", req_type);
		return QDF_STATUS_E_INVAL;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * hdd_twt_get_add_dialog_values() - Get TWT add dialog parameter
 * values from QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_PARAMS
 * @tb: nl attributes
 * @params: wmi twt add dialog parameters
 *
 * Handles QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAX
 *
 * Return: 0 or -EINVAL.
 */
static
int hdd_twt_get_add_dialog_values(struct nlattr **tb,
				  struct wmi_twt_add_dialog_param *params)
{
	uint32_t wake_intvl_exp, result;
	int cmd_id;
	QDF_STATUS qdf_status;

	cmd_id = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_ID;
	if (tb[cmd_id]) {
		params->dialog_id = nla_get_u8(tb[cmd_id]);
	} else {
		params->dialog_id = 0;
		hdd_debug("TWT_SETUP_FLOW_ID not specified. set to zero");
	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_INTVL_EXP;
	if (!tb[cmd_id]) {
		hdd_err_rl("TWT_SETUP_WAKE_INTVL_EXP is must");
		return -EINVAL;
	}
	wake_intvl_exp = nla_get_u8(tb[cmd_id]);
	if (wake_intvl_exp > TWT_SETUP_WAKE_INTVL_EXP_MAX) {
		hdd_err_rl("Invalid wake_intvl_exp %u > %u",
			   wake_intvl_exp,
			   TWT_SETUP_WAKE_INTVL_EXP_MAX);
		return -EINVAL;
	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_BCAST;
	params->flag_bcast = nla_get_flag(tb[cmd_id]);

	cmd_id = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_REQ_TYPE;
	if (!tb[cmd_id]) {
		hdd_err_rl("TWT_SETUP_REQ_TYPE is must");
		return -EINVAL;
	}
	qdf_status = hdd_twt_setup_req_type_to_cmd(nla_get_u8(tb[cmd_id]),
						   &params->twt_cmd);
	if (QDF_IS_STATUS_ERROR(qdf_status))
		return qdf_status_to_os_return(qdf_status);

	cmd_id = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_TRIGGER;
	params->flag_trigger = nla_get_flag(tb[cmd_id]);

	cmd_id = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_TYPE;
	if (!tb[cmd_id]) {
		hdd_err_rl("TWT_SETUP_FLOW_TYPE is must");
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
		hdd_err_rl("TWT_SETUP_WAKE_DURATION is must");
		return -EINVAL;
	}
	params->wake_dura_us = TWT_WAKE_DURATION_MULTIPLICATION_FACTOR *
			       nla_get_u32(tb[cmd_id]);
	if (params->wake_dura_us > TWT_SETUP_WAKE_DURATION_MAX) {
		hdd_err_rl("Invalid wake_dura_us %u",
			   params->wake_dura_us);
		return -EINVAL;
	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_INTVL_MANTISSA;
	if (!tb[cmd_id]) {
		hdd_err_rl("SETUP_WAKE_INTVL_MANTISSA is must");
		return -EINVAL;
	}
	params->wake_intvl_mantis = nla_get_u32(tb[cmd_id]);
	if (params->wake_intvl_mantis >
	    TWT_SETUP_WAKE_INTVL_MANTISSA_MAX) {
		hdd_err_rl("Invalid wake_intvl_mantis %u",
			   params->wake_dura_us);
		return -EINVAL;
	}

	if (wake_intvl_exp && params->wake_intvl_mantis) {
		result = 2 << (wake_intvl_exp - 1);
		if (result >
		    (UINT_MAX / params->wake_intvl_mantis)) {
			hdd_err_rl("Invalid exp %d mantissa %d",
				   wake_intvl_exp,
				   params->wake_intvl_mantis);
			return -EINVAL;
		}
		params->wake_intvl_us =
			params->wake_intvl_mantis * result;
	} else {
		params->wake_intvl_us = params->wake_intvl_mantis;
	}

	hdd_debug("twt: dialog_id %d, vdev %d, wake intvl_us %d, mantis %d",
		  params->dialog_id, params->vdev_id, params->wake_intvl_us,
		  params->wake_intvl_mantis);
	hdd_debug("twt: wake dura %d, sp_offset %d, cmd %d",
		  params->wake_dura_us, params->sp_offset_us,
		  params->twt_cmd);
	hdd_debug("twt: bcast %d, trigger %d, flow_type %d, prot %d",
		  params->flag_bcast, params->flag_trigger,
		  params->flag_flow_type,
		  params->flag_protection);
	hdd_debug("twt: peer mac_addr "
		  QDF_MAC_ADDR_FMT,
		  QDF_MAC_ADDR_REF(params->peer_macaddr));

	return 0;
}

int hdd_test_config_twt_setup_session(struct hdd_adapter *adapter,
				      struct nlattr **tb)
{
	struct nlattr *twt_session;
	int tmp, rc;
	struct hdd_station_ctx *hdd_sta_ctx = NULL;
	struct wmi_twt_add_dialog_param params = {0};
	struct nlattr *tb2[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAX + 1];
	uint32_t congestion_timeout = 0;
	int ret = 0;
	int cmd_id;

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

	qdf_mem_copy(params.peer_macaddr, hdd_sta_ctx->conn_info.bssid.bytes,
		     QDF_MAC_ADDR_SIZE);
	params.vdev_id = adapter->vdev_id;

	cmd_id = QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_TWT_SETUP;
	nla_for_each_nested(twt_session, tb[cmd_id], tmp) {
		rc = wlan_cfg80211_nla_parse(tb2,
					     QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAX,
					     nla_data(twt_session),
					     nla_len(twt_session),
					     qca_wlan_vendor_twt_add_dialog_policy);
		if (rc) {
			hdd_err_rl("Invalid twt ATTR");
			return -EINVAL;
		}

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

		ret = qdf_status_to_os_return(sme_test_config_twt_setup(&params));
	}
	return ret;
}

int hdd_test_config_twt_terminate_session(struct hdd_adapter *adapter,
					  struct nlattr **tb)
{
	struct hdd_station_ctx *hdd_sta_ctx = NULL;
	struct wmi_twt_del_dialog_param params = {0};
	int ret_val;

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
	params.dialog_id = 0;
	hdd_debug("twt_terminate: vdev_id %d", params.vdev_id);

	ret_val = qdf_status_to_os_return(sme_test_config_twt_terminate(&params));
	return ret_val;
}

/**
 * hdd_twt_get_params_resp_len() - Calculates the length
 * of twt get_params nl response
 *
 * Return: Length of get params nl response
 */
static uint32_t hdd_twt_get_params_resp_len(void)
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

	return len;
}

/**
 * hdd_twt_pack_get_params_resp_nlmsg()- Packs and sends twt get_params response
 * @reply_skb: pointer to response skb buffer
 * @params: Ponter to twt peer session parameters
 *
 * Return: QDF_STATUS_SUCCESS on success, else other qdf error values
 */
static QDF_STATUS
hdd_twt_pack_get_params_resp_nlmsg(struct sk_buff *reply_skb,
				   struct wmi_host_twt_session_stats_info *params)
{
	struct nlattr *config_attr, *nla_params;
	uint64_t tsf_val;
	int i, attr;

	config_attr = nla_nest_start(reply_skb,
				     QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_PARAMS);
	if (!config_attr) {
		hdd_err("TWT: get_params nla_nest_start error");
		return QDF_STATUS_E_INVAL;
	}

	for (i = 0; i < TWT_PSOC_MAX_SESSIONS; i++) {
		if (params[i].event_type != HOST_TWT_SESSION_SETUP &&
		    params[i].event_type != HOST_TWT_SESSION_UPDATE)
			continue;

		nla_params = nla_nest_start(reply_skb, i);
		if (!nla_params) {
			hdd_err("TWT: get_params nla_nest_start error");
			return QDF_STATUS_E_INVAL;
		}

		attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAC_ADDR;
		if (nla_put(reply_skb, attr, QDF_MAC_ADDR_SIZE,
			    params[i].peer_mac)) {
			hdd_err("TWT: get_params failed to put mac_addr");
			return QDF_STATUS_E_INVAL;
		}
		hdd_debug("TWT: get_params peer mac_addr " QDF_MAC_ADDR_FMT,
			  QDF_MAC_ADDR_REF(params[i].peer_mac));

		attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_ID;
		if (nla_put_u8(reply_skb, attr, params[i].dialog_id)) {
			hdd_err("TWT: get_params failed to put dialog_id");
			return QDF_STATUS_E_INVAL;
		}

		if (params[i].bcast) {
			attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_BCAST;
			if (nla_put_flag(reply_skb, attr)) {
				hdd_err("TWT: get_params fail to put bcast");
				return QDF_STATUS_E_INVAL;
			}
		}

		if (params[i].trig) {
			attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_TRIGGER;
			if (nla_put_flag(reply_skb, attr)) {
				hdd_err("TWT: get_params fail to put Trigger");
				return QDF_STATUS_E_INVAL;
			}
		}

		if (params[i].announ) {
			attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_TYPE;
			if (nla_put_flag(reply_skb, attr)) {
				hdd_err("TWT: get_params fail to put Announce");
				return QDF_STATUS_E_INVAL;
			}
		}

		if (params[i].protection) {
			attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_PROTECTION;
			if (nla_put_flag(reply_skb, attr)) {
				hdd_err("TWT: get_params fail to put Protect");
				return QDF_STATUS_E_INVAL;
			}
		}

		if (params[i].info_frame_disabled) {
			attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_TWT_INFO_ENABLED;
			if (nla_put_flag(reply_skb, attr)) {
				hdd_err("TWT: get_params put Info Enable fail");
				return QDF_STATUS_E_INVAL;
			}
		}

		attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_DURATION;
		if (nla_put_u32(reply_skb, attr, params[i].wake_dura_us)) {
			hdd_err("TWT: get_params failed to put Wake duration");
			return QDF_STATUS_E_INVAL;
		}

		attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_INTVL_MANTISSA;
		if (nla_put_u32(reply_skb, attr, params[i].wake_intvl_us)) {
			hdd_err("TWT: get_params failed to put Wake Interval");
			return QDF_STATUS_E_INVAL;
		}

		attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_INTVL_EXP;
		if (nla_put_u8(reply_skb, attr, 0)) {
			hdd_err("TWT: get_params put Wake Interval Exp failed");
			return QDF_STATUS_E_INVAL;
		}

		tsf_val = ((uint64_t)params[i].sp_tsf_us_hi << 32) |
			   params[i].sp_tsf_us_lo;

		hdd_debug("TWT: get_params dialog_id %d TSF = 0x%llx",
			  params[i].dialog_id, tsf_val);

		attr = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_TIME_TSF;
		if (hdd_wlan_nla_put_u64(reply_skb, attr, tsf_val)) {
			hdd_err("TWT: get_params failed to put TSF Value");
			return QDF_STATUS_E_INVAL;
		}

		nla_nest_end(reply_skb, nla_params);
	}
	nla_nest_end(reply_skb, config_attr);

	return QDF_STATUS_SUCCESS;
}

/**
 * hdd_twt_pack_get_params_resp()- Obtains twt session parameters of a peer
 * and sends response to the user space via nl layer
 * @hdd_ctx: hdd context
 * @params: Pointer to store twt peer session parameters
 *
 * Return: QDF_STATUS_SUCCESS on success, else other qdf error values
 */
static QDF_STATUS
hdd_twt_pack_get_params_resp(struct hdd_context *hdd_ctx,
			     struct wmi_host_twt_session_stats_info *params)
{
	struct sk_buff *reply_skb;
	uint32_t skb_len = NLMSG_HDRLEN, i;
	QDF_STATUS qdf_status;

	/* Length of attribute QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_PARAMS */
	skb_len += NLA_HDRLEN;

	/* Length of twt session parameters */
	for (i = 0; i < TWT_PSOC_MAX_SESSIONS; i++) {
		if (params[i].event_type == HOST_TWT_SESSION_SETUP ||
		    params[i].event_type == HOST_TWT_SESSION_UPDATE)
			skb_len += hdd_twt_get_params_resp_len();
	}

	reply_skb = wlan_cfg80211_vendor_cmd_alloc_reply_skb(hdd_ctx->wiphy,
							     skb_len);
	if (!reply_skb) {
		hdd_err("TWT: get_params alloc reply skb failed");
		return QDF_STATUS_E_NOMEM;
	}

	qdf_status = hdd_twt_pack_get_params_resp_nlmsg(reply_skb, params);
	if (QDF_IS_STATUS_ERROR(qdf_status))
		goto fail;

	if (cfg80211_vendor_cmd_reply(reply_skb))
		qdf_status = QDF_STATUS_E_INVAL;
fail:
	if (QDF_IS_STATUS_ERROR(qdf_status) && reply_skb)
		kfree_skb(reply_skb);

	return qdf_status;
}

/**
 * hdd_twt_get_peer_session_params() - Obtains twt session parameters of a peer
 * and sends response to the user space
 * @hdd_ctx: hdd context
 * @params: Pointer to store twt peer session parameters
 *
 * Return: QDF_STATUS_SUCCESS on success, else other qdf error values
 */
static QDF_STATUS
hdd_twt_get_peer_session_params(struct hdd_context *hdd_ctx,
				struct wmi_host_twt_session_stats_info *params)
{
	QDF_STATUS qdf_status = QDF_STATUS_E_INVAL;

	if (!hdd_ctx || !params)
		return qdf_status;

	qdf_status = ucfg_twt_get_peer_session_params(hdd_ctx->psoc, params);
	if (QDF_IS_STATUS_SUCCESS(qdf_status))
		qdf_status = hdd_twt_pack_get_params_resp(hdd_ctx, params);

	return qdf_status;
}

/**
 * hdd_twt_get_session_params() - Parses twt nl attrributes, obtains twt
 * session parameters based on dialog_id and returns to user via nl layer
 * @adapter: hdd_adapter
 * @twt_param_attr: twt nl attributes
 *
 * Return: 0 on success, negative value on failure
 */
static int hdd_twt_get_session_params(struct hdd_adapter *adapter,
				      struct nlattr *twt_param_attr)
{
	struct hdd_station_ctx *hdd_sta_ctx =
				WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAX + 1];
	struct wmi_host_twt_session_stats_info
				params[TWT_PSOC_MAX_SESSIONS] = { {0} };
	int ret, id;
	QDF_STATUS qdf_status;

	ret = wlan_cfg80211_nla_parse_nested(tb,
					     QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAX,
					     twt_param_attr,
					     qca_wlan_vendor_twt_add_dialog_policy);
	if (ret)
		return ret;

	/*
	 * Currently twt_get_params nl cmd is sending only dialog_id(STA), fill
	 * mac_addr of STA in params and call hdd_twt_get_peer_session_params.
	 * When twt_get_params passes mac_addr and dialog_id of STA/SAP, update
	 * both mac_addr and dialog_id in params before calling
	 * hdd_twt_get_peer_session_params. dialog_id if not received,
	 * dialog_id of value 0 will be used as default.
	 */
	id = QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_ID;
	if (tb[id])
		params[0].dialog_id = (uint32_t)nla_get_u8(tb[id]);
	else
		params[0].dialog_id = 0;

	hdd_debug("TWT: get_params dialog_id %d", params[0].dialog_id);

	if (params[0].dialog_id <= TWT_MAX_DIALOG_ID) {
		qdf_mem_copy(params[0].peer_mac,
			     hdd_sta_ctx->conn_info.bssid.bytes,
			     QDF_MAC_ADDR_SIZE);
		hdd_debug("TWT: get_params peer mac_addr " QDF_MAC_ADDR_FMT,
			  QDF_MAC_ADDR_REF(params[0].peer_mac));
	}

	qdf_status = hdd_twt_get_peer_session_params(adapter->hdd_ctx,
						     &params[0]);

	return qdf_status_to_os_return(qdf_status);
}

/**
 * hdd_get_twt_setup_event_len() - Calculates the length of twt
 * setup nl response
 * @additional_params_present: if true, then length required for
 * fixed and additional parameters is returned. if false,
 * then length required for fixed parameters is returned.
 *
 * Return: Length of twt setup nl response
 */
static
uint32_t hdd_get_twt_setup_event_len(bool additional_params_present)
{
	uint32_t len = 0;

	len += NLMSG_HDRLEN;
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

	return len;
}

/**
 * wmi_twt_resume_status_to_vendor_twt_status() - convert from
 * WMI_HOST_RESUME_TWT_STATUS to qca_wlan_vendor_twt_status
 * @status: WMI_HOST_RESUME_TWT_STATUS value from firmware
 *
 * Return: qca_wlan_vendor_twt_status values corresponding
 * to the firmware failure status
 */
static int
wmi_twt_resume_status_to_vendor_twt_status(enum WMI_HOST_RESUME_TWT_STATUS status)
{
	switch (status) {
	case WMI_HOST_RESUME_TWT_STATUS_OK:
		return QCA_WLAN_VENDOR_TWT_STATUS_OK;
	case WMI_HOST_RESUME_TWT_STATUS_DIALOG_ID_NOT_EXIST:
		return QCA_WLAN_VENDOR_TWT_STATUS_SESSION_NOT_EXIST;
	case WMI_HOST_RESUME_TWT_STATUS_INVALID_PARAM:
		return QCA_WLAN_VENDOR_TWT_STATUS_INVALID_PARAM;
	case WMI_HOST_RESUME_TWT_STATUS_DIALOG_ID_BUSY:
		return QCA_WLAN_VENDOR_TWT_STATUS_SESSION_BUSY;
	case WMI_HOST_RESUME_TWT_STATUS_NOT_PAUSED:
		return QCA_WLAN_VENDOR_TWT_STATUS_NOT_SUSPENDED;
	case WMI_HOST_RESUME_TWT_STATUS_NO_RESOURCE:
		return QCA_WLAN_VENDOR_TWT_STATUS_NO_RESOURCE;
	case WMI_HOST_RESUME_TWT_STATUS_NO_ACK:
		return QCA_WLAN_VENDOR_TWT_STATUS_NO_ACK;
	case WMI_HOST_RESUME_TWT_STATUS_UNKNOWN_ERROR:
		return QCA_WLAN_VENDOR_TWT_STATUS_UNKNOWN_ERROR;
	default:
		return QCA_WLAN_VENDOR_TWT_STATUS_UNKNOWN_ERROR;
	}
}

/**
 * wmi_twt_pause_status_to_vendor_twt_status() - convert from
 * WMI_HOST_PAUSE_TWT_STATUS to qca_wlan_vendor_twt_status
 * @status: WMI_HOST_PAUSE_TWT_STATUS value from firmware
 *
 * Return: qca_wlan_vendor_twt_status values corresponding
 * to the firmware failure status
 */
static int
wmi_twt_pause_status_to_vendor_twt_status(enum WMI_HOST_PAUSE_TWT_STATUS status)
{
	switch (status) {
	case WMI_HOST_PAUSE_TWT_STATUS_OK:
		return QCA_WLAN_VENDOR_TWT_STATUS_OK;
	case WMI_HOST_PAUSE_TWT_STATUS_DIALOG_ID_NOT_EXIST:
		return QCA_WLAN_VENDOR_TWT_STATUS_SESSION_NOT_EXIST;
	case WMI_HOST_PAUSE_TWT_STATUS_INVALID_PARAM:
		return QCA_WLAN_VENDOR_TWT_STATUS_INVALID_PARAM;
	case WMI_HOST_PAUSE_TWT_STATUS_DIALOG_ID_BUSY:
		return QCA_WLAN_VENDOR_TWT_STATUS_SESSION_BUSY;
	case WMI_HOST_PAUSE_TWT_STATUS_NO_RESOURCE:
		return QCA_WLAN_VENDOR_TWT_STATUS_NO_RESOURCE;
	case WMI_HOST_PAUSE_TWT_STATUS_NO_ACK:
		return QCA_WLAN_VENDOR_TWT_STATUS_NO_ACK;
	case WMI_HOST_PAUSE_TWT_STATUS_UNKNOWN_ERROR:
		return QCA_WLAN_VENDOR_TWT_STATUS_UNKNOWN_ERROR;
	default:
		return QCA_WLAN_VENDOR_TWT_STATUS_UNKNOWN_ERROR;
	}
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
 * Return: qca_wlan_vendor_twt_status values corresponding
 * to WMI_HOST_ADD_TWT_STATUS.
 */
static enum qca_wlan_vendor_twt_status
wmi_twt_add_status_to_vendor_twt_status(enum WMI_HOST_ADD_TWT_STATUS status)
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
		return QCA_WLAN_VENDOR_TWT_STATUS_UNKNOWN_ERROR;
	}
}

/**
 * hdd_twt_setup_pack_resp_nlmsg() - pack nlmsg response for setup
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
hdd_twt_setup_pack_resp_nlmsg(struct sk_buff *reply_skb,
			      struct twt_add_dialog_complete_event *event)
{
	uint64_t sp_offset_tsf;
	enum qca_wlan_vendor_twt_status vendor_status;
	int response_type;

	hdd_enter();

	sp_offset_tsf = event->additional_params.sp_tsf_us_hi;
	sp_offset_tsf = (sp_offset_tsf << 32) |
			 event->additional_params.sp_tsf_us_lo;

	if (nla_put_u8(reply_skb, QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_ID,
		       event->params.dialog_id)) {
		hdd_err("TWT: Failed to put dialog_id");
		return QDF_STATUS_E_FAILURE;
	}

	vendor_status = wmi_twt_add_status_to_vendor_twt_status(event->params.status);
	if (nla_put_u8(reply_skb, QCA_WLAN_VENDOR_ATTR_TWT_SETUP_STATUS,
		       vendor_status)) {
		hdd_err("TWT: Failed to put setup status");
		return QDF_STATUS_E_FAILURE;
	}

	if (event->params.num_additional_twt_params == 0)
		return QDF_STATUS_SUCCESS;

	response_type = wmi_twt_add_cmd_to_vendor_twt_resp_type(event->additional_params.twt_cmd);
	if (response_type == -EINVAL) {
		hdd_err("TWT: Invalid response type from firmware");
		return QDF_STATUS_E_FAILURE;
	}
	if (nla_put_u8(reply_skb, QCA_WLAN_VENDOR_ATTR_TWT_SETUP_RESP_TYPE,
		       response_type)) {
		hdd_err("TWT: Failed to put setup response type");
		return QDF_STATUS_E_FAILURE;
	}

	if (nla_put_u8(reply_skb, QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_TYPE,
		       event->additional_params.announce)) {
		hdd_err("TWT: Failed to put setup flow type");
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

/**
 * hdd_twt_add_dialog_comp_cb() - HDD callback for twt add dialog
 * complete event
 * @hdd_ctx: HDD Context
 * @add_dialog_event: Pointer to Add dialog complete event structure
 *
 * Return: None
 */
static void
hdd_twt_add_dialog_comp_cb(void *context,
			   struct twt_add_dialog_complete_event *add_dialog_event)
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

	qdf_mem_copy(&priv->add_dialog_comp_ev_buf, add_dialog_event,
		     sizeof(*add_dialog_event));
	osif_request_complete(request);
	osif_request_put(request);

	hdd_debug("TWT: add dialog_id:%d, status:%d vdev_id %d peer mac_addr "
		  QDF_MAC_ADDR_FMT, add_dialog_event->params.dialog_id,
		  add_dialog_event->params.status,
		  add_dialog_event->params.vdev_id,
		  QDF_MAC_ADDR_REF(add_dialog_event->params.peer_macaddr));

	hdd_exit();
}

/**
 * hdd_send_twt_add_dialog_cmd() - Send TWT add dialog command to target
 * @hdd_ctx: HDD Context
 * @twt_params: Pointer to Add dialog cmd params structure
 *
 * Return: 0 for Success and negative value for failure
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
	bool additional_params_present = false;
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
	if (add_dialog_comp_ev_params->params.num_additional_twt_params != 0)
		additional_params_present = true;

	skb_len = hdd_get_twt_setup_event_len(additional_params_present);

	reply_skb = cfg80211_vendor_cmd_alloc_reply_skb(hdd_ctx->wiphy,
							skb_len);
	if (!reply_skb) {
		hdd_err("cfg80211_vendor_cmd_alloc_reply_skb failed");
		ret = -ENOMEM;
		goto err;
	}

	status = hdd_twt_setup_pack_resp_nlmsg(reply_skb,
					       add_dialog_comp_ev_params);
	if (QDF_IS_STATUS_ERROR(status)) {
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
 * hdd_get_twt_event_len() - calculate length of skb
 * required for sending twt terminate, pause and resume
 * command responses.
 *
 * Return: length of skb
 */
static uint32_t hdd_get_twt_event_len(void)
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
 * Return: QDF_STATUS_SUCCESS on Success, QDF_STATUS_E_FAILURE
 * on failure
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
 * Return: 0 on success, negative value on failure
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

	skb_len = hdd_get_twt_event_len();
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
 * operation in the received vendor command and
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
 * hdd_twt_pause_dialog_comp_cb() - callback function
 * to get twt pause command complete event
 * @context: private context
 * @params: Pointer to pause dialog complete event buffer
 *
 * Return: None
 */
static void
hdd_twt_pause_dialog_comp_cb(void *context,
			     struct wmi_twt_pause_dialog_complete_event_param *params)
{
	struct osif_request *request;
	struct twt_pause_dialog_comp_ev_priv *priv;

	hdd_enter();

	request = osif_request_get(context);
	if (!request) {
		hdd_err("Obsolete request");
		return;
	}

	priv = osif_request_priv(request);

	qdf_mem_copy(&priv->pause_dialog_comp_ev_buf, params, sizeof(*params));
	osif_request_complete(request);
	osif_request_put(request);

	hdd_debug("TWT: pause dialog_id:%d, status:%d vdev_id %d peer mac_addr "
		  QDF_MAC_ADDR_FMT, params->dialog_id,
		  params->status, params->vdev_id,
		  QDF_MAC_ADDR_REF(params->peer_macaddr));

	hdd_exit();
}

/**
 * hdd_twt_pause_pack_resp_nlmsg() - pack the skb with
 * firmware response for twt pause command
 * @reply_skb: skb to store the response
 * @params: Pointer to pause dialog complete event buffer
 *
 * Return: QDF_STATUS_SUCCESS on Success, QDF_STATUS_E_FAILURE
 * on failure
 */
static QDF_STATUS
hdd_twt_pause_pack_resp_nlmsg(struct sk_buff *reply_skb,
			      struct wmi_twt_pause_dialog_complete_event_param *params)
{
	int vendor_status;

	if (nla_put_u8(reply_skb, QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_ID,
		       params->dialog_id)) {
		hdd_debug("TWT: Failed to put dialog_id");
		return QDF_STATUS_E_FAILURE;
	}

	vendor_status = wmi_twt_pause_status_to_vendor_twt_status(params->status);
	if (nla_put_u8(reply_skb, QCA_WLAN_VENDOR_ATTR_TWT_SETUP_STATUS,
		       vendor_status)) {
		hdd_err("TWT: Failed to put QCA_WLAN_TWT_PAUSE status");
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * hdd_send_twt_pause_dialog_cmd() - Send TWT pause dialog command to target
 * @hdd_ctx: HDD Context
 * @twt_params: Pointer to pause dialog cmd params structure
 *
 * Return: 0 on success, negative value on failure
 */
static
int hdd_send_twt_pause_dialog_cmd(struct hdd_context *hdd_ctx,
				  struct wmi_twt_pause_dialog_cmd_param *twt_params)
{
	struct wmi_twt_pause_dialog_complete_event_param *comp_ev_params;
	struct twt_pause_dialog_comp_ev_priv *priv;
	static const struct osif_request_params osif_req_params = {
		.priv_size = sizeof(*priv),
		.timeout_ms = TWT_PAUSE_COMPLETE_TIMEOUT,
		.dealloc = NULL,
	};
	struct osif_request *request;
	struct sk_buff *reply_skb = NULL;
	QDF_STATUS status;
	void *cookie;
	int skb_len;
	int ret;

	request = osif_request_alloc(&osif_req_params);
	if (!request) {
		hdd_err("twt osif request allocation failure");
		ret = -ENOMEM;
		goto err;
	}

	cookie = osif_request_cookie(request);

	status = sme_pause_dialog_cmd(hdd_ctx->mac_handle,
				      hdd_twt_pause_dialog_comp_cb,
				      twt_params, cookie);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("Failed to send pause dialog command");
		ret = qdf_status_to_os_return(status);
		goto err;
	}

	ret = osif_request_wait_for_response(request);
	if (ret) {
		hdd_err("twt: pause dialog req timedout");
		ret = -ETIMEDOUT;
		goto err;
	}

	priv = osif_request_priv(request);
	comp_ev_params = &priv->pause_dialog_comp_ev_buf;

	skb_len = hdd_get_twt_event_len();
	reply_skb = wlan_cfg80211_vendor_cmd_alloc_reply_skb(hdd_ctx->wiphy,
							     skb_len);
	if (!reply_skb) {
		hdd_err("cfg80211_vendor_cmd_alloc_reply_skb failed");
		ret = -ENOMEM;
		goto err;
	}

	status = hdd_twt_pause_pack_resp_nlmsg(reply_skb, comp_ev_params);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("Failed to pack nl pause dialog response");
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
 * hdd_twt_pause_session - Process TWT pause operation
 * in the received vendor command and send it to firmware
 * @adapter: adapter pointer
 * @twt_param_attr: nl attributes
 *
 * Handles QCA_WLAN_TWT_SUSPEND
 *
 * Return: 0 on success, negative value on failure
 */
static int hdd_twt_pause_session(struct hdd_adapter *adapter,
				 struct nlattr *twt_param_attr)
{
	struct hdd_station_ctx *hdd_sta_ctx;
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAX + 1];
	struct wmi_twt_pause_dialog_cmd_param params = {0};
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

	qdf_mem_copy(params.peer_macaddr, hdd_sta_ctx->conn_info.bssid.bytes,
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
		hdd_debug("TWT: FLOW_ID not specified. set to zero");
	}

	hdd_debug("twt_pause: vdev_id %d dialog_id %d peer mac_addr "
		  QDF_MAC_ADDR_FMT, params.vdev_id, params.dialog_id,
		  QDF_MAC_ADDR_REF(params.peer_macaddr));

	ret = hdd_send_twt_pause_dialog_cmd(adapter->hdd_ctx, &params);

	return ret;
}

/**
 * hdd_twt_resume_dialog_comp_cb() - callback function
 * to get twt resume command complete event
 * @context: private context
 * @params: Pointer to resume dialog complete event buffer
 *
 * Return: None
 */
static void
hdd_twt_resume_dialog_comp_cb(void *context,
			      struct wmi_twt_resume_dialog_complete_event_param *params)
{
	struct osif_request *request;
	struct twt_resume_dialog_comp_ev_priv *priv;

	hdd_enter();

	request = osif_request_get(context);
	if (!request) {
		hdd_err("Obsolete request");
		return;
	}

	priv = osif_request_priv(request);

	qdf_mem_copy(&priv->res_dialog_comp_ev_buf, params, sizeof(*params));
	osif_request_complete(request);
	osif_request_put(request);

	hdd_debug("TWT: resume dialog_id:%d status:%d vdev_id %d peer mac_addr "
		  QDF_MAC_ADDR_FMT, params->dialog_id,
		  params->status, params->vdev_id,
		  QDF_MAC_ADDR_REF(params->peer_macaddr));

	hdd_exit();
}

/**
 * hdd_twt_resume_pack_resp_nlmsg() - pack the skb with
 * firmware response for twt resume command
 * @reply_skb: skb to store the response
 * @params: Pointer to resume dialog complete event buffer
 *
 * Return: QDF_STATUS_SUCCESS on Success, QDF_STATUS_E_FAILURE
 * on failure
 */
static QDF_STATUS
hdd_twt_resume_pack_resp_nlmsg(struct sk_buff *reply_skb,
			       struct wmi_twt_resume_dialog_complete_event_param *params)
{
	int vendor_status;

	if (nla_put_u8(reply_skb, QCA_WLAN_VENDOR_ATTR_TWT_RESUME_FLOW_ID,
		       params->dialog_id)) {
		hdd_debug("TWT: Failed to put dialog_id");
		return QDF_STATUS_E_FAILURE;
	}

	vendor_status = wmi_twt_resume_status_to_vendor_twt_status(params->status);
	if (nla_put_u8(reply_skb, QCA_WLAN_VENDOR_ATTR_TWT_SETUP_STATUS,
		       vendor_status)) {
		hdd_err("TWT: Failed to put QCA_WLAN_TWT_RESUME status");
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * hdd_send_twt_resume_dialog_cmd() - Send TWT resume dialog command to target
 * @hdd_ctx: HDD Context
 * @twt_params: Pointer to resume dialog cmd params structure
 *
 * Return: 0 on success, negative value on failure
 */
static int
hdd_send_twt_resume_dialog_cmd(struct hdd_context *hdd_ctx,
			       struct wmi_twt_resume_dialog_cmd_param *twt_params)
{
	struct wmi_twt_resume_dialog_complete_event_param *comp_ev_params;
	struct twt_resume_dialog_comp_ev_priv *priv;
	static const struct osif_request_params osif_req_params = {
		.priv_size = sizeof(*priv),
		.timeout_ms = TWT_RESUME_COMPLETE_TIMEOUT,
		.dealloc = NULL,
	};
	struct osif_request *request;
	struct sk_buff *reply_skb = NULL;
	QDF_STATUS status;
	void *cookie;
	int skb_len;
	int ret;

	request = osif_request_alloc(&osif_req_params);
	if (!request) {
		hdd_err("twt osif request allocation failure");
		ret = -ENOMEM;
		goto err;
	}

	cookie = osif_request_cookie(request);

	status = sme_resume_dialog_cmd(hdd_ctx->mac_handle,
				       hdd_twt_resume_dialog_comp_cb,
				       twt_params, cookie);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("Failed to send resume dialog command");
		ret = qdf_status_to_os_return(status);
		goto err;
	}

	ret = osif_request_wait_for_response(request);
	if (ret) {
		hdd_err("twt: resume dialog req timedout");
		ret = -ETIMEDOUT;
		goto err;
	}

	priv = osif_request_priv(request);
	comp_ev_params = &priv->res_dialog_comp_ev_buf;

	skb_len = hdd_get_twt_event_len();
	reply_skb = wlan_cfg80211_vendor_cmd_alloc_reply_skb(hdd_ctx->wiphy,
							     skb_len);
	if (!reply_skb) {
		hdd_err("cfg80211_vendor_cmd_alloc_reply_skb failed");
		ret = -ENOMEM;
		goto err;
	}

	status = hdd_twt_resume_pack_resp_nlmsg(reply_skb, comp_ev_params);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("Failed to pack nl resume dialog response");
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
 * hdd_twt_resume_session - Process TWT resume operation
 * in the received vendor command and send it to firmware
 * @adapter: adapter pointer
 * @twt_param_attr: nl attributes
 *
 * Handles QCA_WLAN_TWT_RESUME
 *
 * Return: 0 on success, negative value on failure
 */
static int hdd_twt_resume_session(struct hdd_adapter *adapter,
				  struct nlattr *twt_param_attr)
{
	struct hdd_station_ctx *hdd_sta_ctx;
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_TWT_RESUME_MAX + 1];
	struct wmi_twt_resume_dialog_cmd_param params = {0};
	int id, id2;
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

	qdf_mem_copy(params.peer_macaddr, hdd_sta_ctx->conn_info.bssid.bytes,
		     QDF_MAC_ADDR_SIZE);
	params.vdev_id = adapter->vdev_id;

	ret = wlan_cfg80211_nla_parse_nested(tb,
					     QCA_WLAN_VENDOR_ATTR_TWT_RESUME_MAX,
					     twt_param_attr,
					     qca_wlan_vendor_twt_resume_dialog_policy);
	if (ret)
		return ret;

	id = QCA_WLAN_VENDOR_ATTR_TWT_RESUME_FLOW_ID;
	if (tb[id]) {
		params.dialog_id = nla_get_u8(tb[id]);
	} else {
		params.dialog_id = 0;
		hdd_debug("TWT_RESUME_FLOW_ID not specified. set to zero");
	}

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
		hdd_err_rl("TWT_RESUME NEXT_TWT_SIZE is must");
		return -EINVAL;
	}
	if (params.next_twt_size > TWT_MAX_NEXT_TWT_SIZE)
		return -EINVAL;

	hdd_debug("twt_resume: vdev_id %d dialog_id %d peer mac_addr "
		  QDF_MAC_ADDR_FMT, params.vdev_id, params.dialog_id,
		  QDF_MAC_ADDR_REF(params.peer_macaddr));

	ret = hdd_send_twt_resume_dialog_cmd(adapter->hdd_ctx, &params);

	return ret;
}

/**
 * hdd_twt_configure - Process the TWT
 * operation in the received vendor command
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
		ret = hdd_twt_get_session_params(adapter, twt_param_attr);
		break;
	case QCA_WLAN_TWT_TERMINATE:
		ret = hdd_twt_terminate_session(adapter, twt_param_attr);
		break;
	case QCA_WLAN_TWT_SUSPEND:
		ret = hdd_twt_pause_session(adapter, twt_param_attr);
		break;
	case QCA_WLAN_TWT_RESUME:
		ret = hdd_twt_resume_session(adapter, twt_param_attr);
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

	sme_init_twt_complete_cb(hdd_ctx->mac_handle);
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
