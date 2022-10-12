/*
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: defines driver functions interfacing with linux kernel
 */
#include <wmi_unified_param.h>
#include <wlan_osif_request_manager.h>
#include <osif_sync.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_hdd_main.h>
#include <wlan_coap_main.h>
#include <wlan_coap_ucfg_api.h>
#include <wlan_cfg80211_coap.h>

#define COAP_MATCH_DATA_BYTES_MAX 16
#define COAP_MSG_BYTES_MAX 1152
#define COAP_OFFLOAD_REPLY_CACHE_EXPTIME_MS 40000
#define COAP_OFFLOAD_CACHE_GET_TIMEOUT_MS 2000

#define COAP_ATTR(_name) QCA_WLAN_VENDOR_ATTR_COAP_OFFLOAD_ ## _name

static const struct nla_policy
coap_offload_filter_policy[COAP_ATTR(FILTER_MAX) + 1] = {
	[COAP_ATTR(FILTER_DEST_IPV4)] = {.type = NLA_U32},
	[COAP_ATTR(FILTER_DEST_IPV4_IS_BC)] = {.type = NLA_FLAG},
	[COAP_ATTR(FILTER_DEST_PORT)] = {.type = NLA_U16},
	[COAP_ATTR(FILTER_MATCH_OFFSET)] = {.type = NLA_U32},
	[COAP_ATTR(FILTER_MATCH_DATA)] = {
		.type = NLA_BINARY, .len = COAP_MATCH_DATA_BYTES_MAX},
};

static const struct nla_policy
coap_offload_tx_ipv4_policy[COAP_ATTR(TX_IPV4_MAX) + 1] = {
	[COAP_ATTR(TX_IPV4_SRC_ADDR)] = {.type = NLA_U32},
	[COAP_ATTR(TX_IPV4_SRC_PORT)] = {.type = NLA_U16},
	[COAP_ATTR(TX_IPV4_DEST_ADDR)] = {.type = NLA_U32},
	[COAP_ATTR(TX_IPV4_DEST_IS_BC)] = {.type = NLA_FLAG},
	[COAP_ATTR(TX_IPV4_DEST_PORT)] = {.type = NLA_U16},
};

static const struct nla_policy
coap_offload_reply_policy[COAP_ATTR(REPLY_MAX) + 1] = {
	[COAP_ATTR(REPLY_SRC_IPV4)] = {.type = NLA_U32},
	[COAP_ATTR(REPLY_FILTER)] =
		VENDOR_NLA_POLICY_NESTED(coap_offload_filter_policy),
	[COAP_ATTR(REPLY_MSG)] = {
		.type = NLA_BINARY, .len = COAP_MSG_BYTES_MAX},
	[COAP_ATTR(REPLY_CACHE_EXPTIME)] = {.type = NLA_U32},
};

static const struct nla_policy
coap_offload_periodic_tx_policy[COAP_ATTR(PERIODIC_TX_MAX) + 1] = {
	[COAP_ATTR(PERIODIC_TX_IPV4)] =
		VENDOR_NLA_POLICY_NESTED(coap_offload_tx_ipv4_policy),
	[COAP_ATTR(PERIODIC_TX_PERIOD)] = {.type = NLA_U32},
	[COAP_ATTR(PERIODIC_TX_MSG)] = {
		.type = NLA_BINARY, .len = COAP_MSG_BYTES_MAX},
};

const struct nla_policy
coap_offload_policy[COAP_ATTR(MAX) + 1] = {
	[COAP_ATTR(ACTION)] = {.type = NLA_U32 },
	[COAP_ATTR(REQ_ID)] = {.type = NLA_U32 },
	[COAP_ATTR(REPLY)] =
		VENDOR_NLA_POLICY_NESTED(coap_offload_reply_policy),
	[COAP_ATTR(PERIODIC_TX)] =
		VENDOR_NLA_POLICY_NESTED(coap_offload_periodic_tx_policy),
};

/**
 * wlan_cfg80211_coap_offload_reply_fill_filter() - fill filter for CoAP
 * offload reply.
 * @attr_filter: pointer to filter attribute
 * @params: pointer to parameters for CoAP offload reply
 *
 * Return: 0 on success; error number otherwise
 */
static int
wlan_cfg80211_coap_offload_reply_fill_filter(struct nlattr *attr_filter,
	struct coap_offload_reply_param *params)
{
	struct nlattr *tb[COAP_ATTR(FILTER_MAX) + 1];

	if (!attr_filter) {
		coap_err("No ATTR filter");
		return -EINVAL;
	}

	if (!nla_data(attr_filter)) {
		coap_err("Invalid filter");
		return -EINVAL;
	}

	if (wlan_cfg80211_nla_parse_nested(tb, COAP_ATTR(FILTER_MAX),
					   attr_filter,
					   coap_offload_filter_policy)) {
		coap_err("Invalid ATTR");
		return -EINVAL;
	}

	if (!tb[COAP_ATTR(FILTER_DEST_IPV4)]) {
		coap_err("no ATTR dest IPv4");
		return -EINVAL;
	}

	params->dest_ip_v4 = nla_get_u32(tb[COAP_ATTR(FILTER_DEST_IPV4)]);
	params->dest_ip_v4_is_bc =
		nla_get_flag(tb[COAP_ATTR(FILTER_DEST_IPV4_IS_BC)]);

	if (!tb[COAP_ATTR(FILTER_DEST_PORT)]) {
		coap_err("no ATTR dest IPv4 port");
		return -EINVAL;
	}

	params->dest_udp_port = nla_get_u16(tb[COAP_ATTR(FILTER_DEST_PORT)]);

	if (!tb[COAP_ATTR(FILTER_MATCH_OFFSET)]) {
		coap_err("no ATTR match offset");
		return -EINVAL;
	}

	params->verify_offset =
		nla_get_u32(tb[COAP_ATTR(FILTER_MATCH_OFFSET)]);

	if (!tb[COAP_ATTR(FILTER_MATCH_DATA)]) {
		coap_err("no ATTR match data");
		return -EINVAL;
	}

	params->verify_len = nla_len(tb[COAP_ATTR(FILTER_MATCH_DATA)]);
	if (!params->verify_len) {
		coap_err("invalid match data len");
		return -EINVAL;
	}

	params->verify = nla_data(tb[COAP_ATTR(FILTER_MATCH_DATA)]);
	return 0;
}

/**
 * wlan_cfg80211_coap_offload_reply_enable() - enable CoAP offload reply
 * @vdev: pointer to vdev object
 * @req_id: request id
 * @attr_reply: pointer to CoAP offload reply attribute
 *
 * Return: 0 on success; error number otherwise
 */
static int
wlan_cfg80211_coap_offload_reply_enable(struct wlan_objmgr_vdev *vdev,
					uint32_t req_id,
					struct nlattr *attr_reply)
{
	struct nlattr *tb[COAP_ATTR(REPLY_MAX) + 1];
	struct coap_offload_reply_param params = {0};
	struct nlattr *attr;
	QDF_STATUS status;
	int ret;

	if (!attr_reply) {
		coap_err("No ATTR reply");
		return -EINVAL;
	}

	if (wlan_cfg80211_nla_parse_nested(tb, COAP_ATTR(REPLY_MAX),
					   attr_reply,
					   coap_offload_reply_policy)) {
		coap_err("Invalid ATTR");
		return -EINVAL;
	}

	attr = tb[COAP_ATTR(REPLY_SRC_IPV4)];
	if (!attr) {
		coap_err("No ATTR IPv4");
		return -EINVAL;
	}

	params.pattern_id = req_id;
	params.vdev_id = wlan_vdev_get_id(vdev);
	params.src_ip_v4 = nla_get_u32(attr);

	attr = tb[COAP_ATTR(REPLY_FILTER)];
	ret = wlan_cfg80211_coap_offload_reply_fill_filter(attr, &params);
	if (ret)
		return ret;

	attr = tb[COAP_ATTR(REPLY_MSG)];
	if (!attr) {
		coap_err("No ATTR msg");
		return -EINVAL;
	}

	params.coapmsg_len = nla_len(attr);
	params.coapmsg = nla_data(attr);

	attr = tb[COAP_ATTR(REPLY_CACHE_EXPTIME)];
	if (!attr)
		params.cache_timeout = COAP_OFFLOAD_REPLY_CACHE_EXPTIME_MS;
	else
		params.cache_timeout = nla_get_u32(attr);

	status = ucfg_coap_offload_reply_enable(vdev, &params);
	ret = qdf_status_to_os_return(status);
	return ret;
}

/**
 * wlan_cfg80211_coap_offload_fill_tx_ipv4() - fill IPv4 source/destination
 * address/port for offload transmitting.
 * @attr_ipv4: pointer to TX IPv4 attribute
 * @params: pointer to parameters for CoAP offload reply
 *
 * Return: 0 on success; error number otherwise
 */
static int
wlan_cfg80211_coap_offload_fill_tx_ipv4(struct nlattr *attr_ipv4,
			struct coap_offload_periodic_tx_param *params)
{
	struct nlattr *tb[COAP_ATTR(TX_IPV4_MAX) + 1];

	if (!attr_ipv4) {
		coap_err("No ATTR TX IPv4");
		return -EINVAL;
	}

	if (wlan_cfg80211_nla_parse_nested(tb, COAP_ATTR(TX_IPV4_MAX),
					   attr_ipv4,
					   coap_offload_tx_ipv4_policy)) {
		coap_err("Invalid ATTR");
		return -EINVAL;
	}

	if (!tb[COAP_ATTR(TX_IPV4_SRC_ADDR)]) {
		coap_err("no ATTR src addr");
		return -EINVAL;
	}

	params->src_ip_v4 = nla_get_u32(tb[COAP_ATTR(TX_IPV4_SRC_ADDR)]);
	if (tb[COAP_ATTR(TX_IPV4_SRC_PORT)])
		params->src_udp_port =
			nla_get_u32(tb[COAP_ATTR(TX_IPV4_SRC_PORT)]);

	if (!tb[COAP_ATTR(TX_IPV4_DEST_ADDR)]) {
		coap_err("no ATTR IPv4 dest addr");
		return -EINVAL;
	}

	params->dest_ip_v4 = nla_get_u32(tb[COAP_ATTR(TX_IPV4_DEST_ADDR)]);
	params->dest_ip_v4_is_bc =
		nla_get_flag(tb[COAP_ATTR(TX_IPV4_DEST_IS_BC)]);

	if (!tb[COAP_ATTR(TX_IPV4_DEST_PORT)]) {
		coap_err("no ATTR dest IPv4 port");
		return -EINVAL;
	}

	params->dest_udp_port =
		nla_get_u32(tb[COAP_ATTR(TX_IPV4_DEST_PORT)]);
	return 0;
}

/**
 * wlan_cfg80211_coap_offload_periodic_tx_enable() - enable CoAP offload
 * periodic transmitting
 * @vdev: pointer to vdev object
 * @req_id: request id
 * @attr_reply: pointer to CoAP offload periodic TX attribute
 *
 * Return: 0 on success; error number otherwise
 */
static int
wlan_cfg80211_coap_offload_periodic_tx_enable(struct wlan_objmgr_vdev *vdev,
					      uint32_t req_id,
					      struct nlattr *attr_periodic_tx)
{
	struct nlattr *tb[COAP_ATTR(PERIODIC_TX_MAX) + 1];
	struct coap_offload_periodic_tx_param param = {0};
	struct nlattr *attr_ipv4;
	QDF_STATUS status;
	int ret;

	if (!attr_periodic_tx) {
		coap_err("No ATTR periodic tx");
		return -EINVAL;
	}

	if (wlan_cfg80211_nla_parse_nested(tb, COAP_ATTR(PERIODIC_TX_MAX),
					   attr_periodic_tx,
					   coap_offload_periodic_tx_policy)) {
		coap_err("Invalid ATTR");
		return -EINVAL;
	}

	if (!tb[COAP_ATTR(PERIODIC_TX_PERIOD)]) {
		coap_err("no ATTR period");
		return -EINVAL;
	}

	param.timeout = nla_get_u32(tb[COAP_ATTR(PERIODIC_TX_PERIOD)]);
	attr_ipv4 = tb[COAP_ATTR(PERIODIC_TX_IPV4)];
	ret = wlan_cfg80211_coap_offload_fill_tx_ipv4(attr_ipv4, &param);
	if (ret)
		return ret;

	param.vdev_id = wlan_vdev_get_id(vdev);
	param.pattern_id = req_id;
	if (!tb[COAP_ATTR(PERIODIC_TX_MSG)]) {
		coap_err("no ATTR msg");
		return -EINVAL;
	}

	param.coapmsg_len = nla_len(tb[COAP_ATTR(PERIODIC_TX_MSG)]);
	param.coapmsg = nla_data(tb[COAP_ATTR(PERIODIC_TX_MSG)]);
	status = ucfg_coap_offload_periodic_tx_enable(vdev, &param);
	return qdf_status_to_os_return(status);
}

/**
 * wlan_cfg80211_coap_offload_periodic_tx_enable() - disable CoAP offload
 * periodic transmitting
 * @vdev: pointer to vdev object
 * @req_id: request id
 *
 * Return: 0 on success; error number otherwise
 */
static int
wlan_cfg80211_coap_offload_periodic_tx_disable(struct wlan_objmgr_vdev *vdev,
					       uint32_t req_id)
{
	QDF_STATUS status;

	status = ucfg_coap_offload_periodic_tx_disable(vdev, req_id);
	return qdf_status_to_os_return(status);
}

/**
 * wlan_cfg80211_dealloc_coap_buf_info() - Callback to free priv
 * allocations for CoAP buffer info
 * @priv: Pointer to priv data statucture
 *
 * Return: None
 */
static void wlan_cfg80211_dealloc_coap_buf_info(void *priv)
{
	struct coap_buf_info *info = priv;
	struct coap_buf_node *cur, *next;

	if (!info)
		return;

	qdf_list_for_each_del(&info->info_list, cur, next, node) {
		qdf_list_remove_node(&info->info_list, &cur->node);
		qdf_mem_free(cur->payload);
		qdf_mem_free(cur);
	}

	qdf_list_destroy(&info->info_list);
}

static void
wlan_cfg80211_coap_cache_get_cbk(void *context, struct coap_buf_info *info)
{
	struct osif_request *request;
	struct coap_buf_info *priv_info;

	if (!context || !info)
		return;

	request = osif_request_get(context);
	if (!request)
		return;

	priv_info = osif_request_priv(request);
	if (info->req_id == priv_info->req_id) {
		qdf_list_join(&priv_info->info_list, &info->info_list);
		if (!info->more_info)
			osif_request_complete(request);
	}

	osif_request_put(request);
}

/**
 * wlan_cfg80211_coap_fill_buf_info() - Fill cache get response buffer
 * @reply_skb : pointer to reply_skb
 * @info : information of cached CoAP messages
 * @index : attribute type index for nla_next_start()
 *
 * Return : 0 on success and errno on failure
 */
static int
wlan_cfg80211_coap_fill_buf_info(struct sk_buff *reply_skb,
				 struct coap_buf_node *info, int index)
{
	struct nlattr *attr;

	attr = nla_nest_start(reply_skb, index);
	if (!attr) {
		coap_err("nla_nest_start failed");
		return -EINVAL;
	}

	if (hdd_wlan_nla_put_u64(reply_skb, COAP_ATTR(CACHE_INFO_TS),
				 info->tsf) ||
	    nla_put_u32(reply_skb, COAP_ATTR(CACHE_INFO_SRC_IPV4),
			info->src_ip) ||
	    nla_put(reply_skb, COAP_ATTR(CACHE_INFO_MSG),
		    info->len, info->payload)) {
		coap_err("nla_put failed");
		return -EINVAL;
	}

	nla_nest_end(reply_skb, attr);
	return 0;
}

/**
 * wlan_cfg80211_coap_offload_cache_deliver() - deliver cached CoAP messages
 * @wiphy: pointer to wireless wiphy structure.
 * @cache_list: list of cached CoAP messages
 *
 * Return: 0 on success; error number otherwise
 */
static int
wlan_cfg80211_coap_offload_cache_deliver(struct wiphy *wiphy,
					 qdf_list_t *cache_list)
{
	struct sk_buff *skb;
	uint32_t skb_len = NLMSG_HDRLEN;
	struct coap_buf_node *cur, *next;
	struct nlattr *attr;
	int i = 0, ret;

	/* QCA_WLAN_VENDOR_ATTR_COAP_OFFLOAD_CACHES */
	skb_len += nla_total_size(0);
	qdf_list_for_each_del(cache_list, cur, next, node) {
		if (!cur->len || !cur->payload)
			continue;

		/* nest attribute */
		skb_len += nla_total_size(0);

		/* QCA_WLAN_VENDOR_ATTR_COAP_OFFLOAD_CACHE_INFO_TS */
		skb_len += nla_total_size(sizeof(uint64_t));

		/* QCA_WLAN_VENDOR_ATTR_COAP_OFFLOAD_CACHE_INFO_SRC_IPV4 */
		skb_len += nla_total_size(sizeof(uint32_t));

		/* QCA_WLAN_VENDOR_ATTR_COAP_OFFLOAD_CACHE_INFO_MSG */
		skb_len += nla_total_size(cur->len);
	}

	skb = wlan_cfg80211_vendor_cmd_alloc_reply_skb(wiphy, skb_len);
	attr = nla_nest_start(skb, COAP_ATTR(CACHES));
	if (!attr) {
		hdd_err("nla_nest_start failed");
		wlan_cfg80211_vendor_free_skb(skb);
		return -EINVAL;
	}

	qdf_list_for_each_del(cache_list, cur, next, node) {
		if (!cur->len || !cur->payload)
			continue;

		qdf_list_remove_node(cache_list, &cur->node);
		ret = wlan_cfg80211_coap_fill_buf_info(skb, cur, i++);
		if (ret) {
			wlan_cfg80211_vendor_free_skb(skb);
			return -EINVAL;
		}

		qdf_mem_free(cur->payload);
		qdf_mem_free(cur);
	}

	nla_nest_end(skb, attr);
	return wlan_cfg80211_vendor_cmd_reply(skb);
}

/**
 * wlan_cfg80211_coap_offload_cache_get() - get cached CoAP messages
 * @wiphy: pointer to wireless wiphy structure.
 * @vdev: pointer to vdev object
 * @req_id: request id
 *
 * Return: 0 on success; error number otherwise
 */
static int
wlan_cfg80211_coap_offload_cache_get(struct wiphy *wiphy,
				     struct wlan_objmgr_vdev *vdev,
				     uint32_t req_id)
{
	void *cookie;
	QDF_STATUS status;
	struct osif_request *request;
	struct coap_buf_info *buf_info;
	int ret;
	static const struct osif_request_params params = {
		.priv_size = sizeof(*buf_info),
		.timeout_ms = COAP_OFFLOAD_CACHE_GET_TIMEOUT_MS,
		.dealloc = wlan_cfg80211_dealloc_coap_buf_info,
	};

	request = osif_request_alloc(&params);
	if (!request) {
		coap_err("Request allocation failure");
		status = QDF_STATUS_E_NOMEM;
		goto out;
	}

	buf_info = osif_request_priv(request);
	qdf_list_create(&buf_info->info_list, 0);
	buf_info->req_id = req_id;
	buf_info->vdev_id = wlan_vdev_get_id(vdev);

	cookie = osif_request_cookie(request);
	status = ucfg_coap_offload_cache_get(vdev, req_id,
					     wlan_cfg80211_coap_cache_get_cbk,
					     cookie);
	if (QDF_IS_STATUS_ERROR(status)) {
		coap_err("Unable to get cache");
		goto out;
	}

	ret = osif_request_wait_for_response(request);
	if (ret) {
		coap_err("Target response timed out");
		status = qdf_status_from_os_return(ret);
		goto out;
	}

	ret = wlan_cfg80211_coap_offload_cache_deliver(wiphy,
						       &buf_info->info_list);
	if (ret) {
		coap_err("Failed to deliver buf info");
		status = qdf_status_from_os_return(ret);
		goto out;
	}

out:
	if (request)
		osif_request_put(request);
	return qdf_status_to_os_return(status);
}

/**
 * wlan_cfg80211_coap_offload_reply_disable() - disable CoAP offload reply
 * @wiphy: pointer to wireless wiphy structure.
 * @vdev: pointer to vdev object
 * @req_id: request id
 *
 * Return: 0 on success; error number otherwise
 */
static int
wlan_cfg80211_coap_offload_reply_disable(struct wiphy *wiphy,
					 struct wlan_objmgr_vdev *vdev,
					 uint32_t req_id)
{
	void *cookie;
	QDF_STATUS status;
	struct osif_request *request;
	struct coap_buf_info *buf_info;
	int ret;
	static const struct osif_request_params params = {
		.priv_size = sizeof(*buf_info),
		.timeout_ms = COAP_OFFLOAD_CACHE_GET_TIMEOUT_MS,
		.dealloc = wlan_cfg80211_dealloc_coap_buf_info,
	};

	request = osif_request_alloc(&params);
	if (!request) {
		coap_err("Request allocation failure");
		status = QDF_STATUS_E_NOMEM;
		goto out;
	}

	buf_info = osif_request_priv(request);
	qdf_list_create(&buf_info->info_list, 0);
	buf_info->req_id = req_id;
	buf_info->vdev_id = wlan_vdev_get_id(vdev);

	cookie = osif_request_cookie(request);
	status = ucfg_coap_offload_reply_disable(vdev, req_id,
			wlan_cfg80211_coap_cache_get_cbk, cookie);
	if (QDF_IS_STATUS_ERROR(status)) {
		coap_err("Failed to disable offload reply");
		goto out;
	}

	ret = osif_request_wait_for_response(request);
	if (ret) {
		coap_err("Target response timed out");
		status = qdf_status_from_os_return(ret);
		goto out;
	}

	ret = wlan_cfg80211_coap_offload_cache_deliver(wiphy,
						       &buf_info->info_list);
	if (ret) {
		coap_err("Failed to deliver buf info");
		status = qdf_status_from_os_return(ret);
		goto out;
	}

out:
	if (request)
		osif_request_put(request);
	return qdf_status_to_os_return(status);
}

int
wlan_cfg80211_coap_offload(struct wiphy *wiphy, struct wlan_objmgr_vdev *vdev,
			   const void *data, int data_len)
{
	struct nlattr *tb[COAP_ATTR(MAX) + 1];
	struct nlattr *attr;
	uint32_t action, req_id;
	int ret;

	if (wlan_cfg80211_nla_parse(tb, COAP_ATTR(MAX),
				    data, data_len, coap_offload_policy)) {
		coap_err("Invalid ATTR");
		return -EINVAL;
	}

	if (!tb[COAP_ATTR(ACTION)]) {
		coap_err("no attr action");
		return -EINVAL;
	}

	if (!tb[COAP_ATTR(REQ_ID)]) {
		coap_err("no attr req id");
		return -EINVAL;
	}

	action = nla_get_u32(tb[COAP_ATTR(ACTION)]);
	req_id = nla_get_u32(tb[COAP_ATTR(REQ_ID)]);
	switch (action) {
	case QCA_WLAN_VENDOR_COAP_OFFLOAD_ACTION_REPLY_ENABLE:
		attr = tb[COAP_ATTR(REPLY)];
		ret = wlan_cfg80211_coap_offload_reply_enable(vdev, req_id,
							      attr);
		break;
	case QCA_WLAN_VENDOR_COAP_OFFLOAD_ACTION_REPLY_DISABLE:
		ret = wlan_cfg80211_coap_offload_reply_disable(wiphy, vdev,
							       req_id);
		break;
	case QCA_WLAN_VENDOR_COAP_OFFLOAD_ACTION_PERIODIC_TX_ENABLE:
		attr = tb[COAP_ATTR(PERIODIC_TX)];
		ret = wlan_cfg80211_coap_offload_periodic_tx_enable(vdev,
								    req_id,
								    attr);
		break;
	case QCA_WLAN_VENDOR_COAP_OFFLOAD_ACTION_PERIODIC_TX_DISABLE:
		ret = wlan_cfg80211_coap_offload_periodic_tx_disable(vdev,
								     req_id);
		break;
	case QCA_WLAN_VENDOR_COAP_OFFLOAD_ACTION_CACHE_GET:
		ret = wlan_cfg80211_coap_offload_cache_get(wiphy, vdev,
							   req_id);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	coap_debug("vdev_id %u action %u req id %u ret %d",
		   wlan_vdev_get_id(vdev), action, req_id, ret);
	return ret;
}
