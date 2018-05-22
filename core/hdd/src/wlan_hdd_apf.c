/*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_hdd_apf.c
 *
 * Android Packet Filter support and implementation
 */

#include "wlan_hdd_apf.h"
#include "qca_vendor.h"
#include "wlan_hdd_request_manager.h"

/*
 * define short names for the global vendor params
 * used by __wlan_hdd_cfg80211_apf_offload()
 */
#define APF_INVALID \
	QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_INVALID
#define APF_SET_RESET \
	QCA_WLAN_VENDOR_ATTR_SET_RESET_PACKET_FILTER
#define APF_VERSION \
	QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_VERSION
#define APF_FILTER_ID \
	QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_ID
#define APF_PACKET_SIZE \
	QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_SIZE
#define APF_CURRENT_OFFSET \
	QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_CURRENT_OFFSET
#define APF_PROGRAM \
	QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_PROGRAM
#define APF_MAX \
	QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_MAX

static const struct nla_policy
wlan_hdd_apf_offload_policy[APF_MAX + 1] = {
	[APF_SET_RESET] = {.type = NLA_U32},
	[APF_VERSION] = {.type = NLA_U32},
	[APF_FILTER_ID] = {.type = NLA_U32},
	[APF_PACKET_SIZE] = {.type = NLA_U32},
	[APF_CURRENT_OFFSET] = {.type = NLA_U32},
	[APF_PROGRAM] = {.type = NLA_U8},
};

struct apf_offload_priv {
	struct sir_apf_get_offload apf_get_offload;
};

static void hdd_get_apf_offload_cb(void *context,
				   struct sir_apf_get_offload *data)
{
	struct hdd_request *request;
	struct apf_offload_priv *priv;

	hdd_enter();

	request = hdd_request_get(context);
	if (!request) {
		hdd_err("Obsolete request");
		return;
	}

	priv = hdd_request_priv(request);
	priv->apf_get_offload = *data;
	hdd_request_complete(request);
	hdd_request_put(request);
}

/**
 * hdd_post_get_apf_capabilities_rsp() - Callback function to APF Offload
 * @hdd_context: hdd_context
 * @apf_get_offload: struct for get offload
 *
 * Return: 0 on success, error number otherwise.
 */
static int
hdd_post_get_apf_capabilities_rsp(struct hdd_context *hdd_ctx,
				  struct sir_apf_get_offload *apf_get_offload)
{
	struct sk_buff *skb;
	uint32_t nl_buf_len;

	hdd_enter();

	nl_buf_len = NLMSG_HDRLEN;
	nl_buf_len +=
		(sizeof(apf_get_offload->max_bytes_for_apf_inst) + NLA_HDRLEN) +
		(sizeof(apf_get_offload->apf_version) + NLA_HDRLEN);

	skb = cfg80211_vendor_cmd_alloc_reply_skb(hdd_ctx->wiphy, nl_buf_len);
	if (!skb) {
		hdd_err("cfg80211_vendor_cmd_alloc_reply_skb failed");
		return -ENOMEM;
	}

	hdd_debug("APF Version: %u APF max bytes: %u",
		  apf_get_offload->apf_version,
		  apf_get_offload->max_bytes_for_apf_inst);

	if (nla_put_u32(skb, APF_PACKET_SIZE,
			apf_get_offload->max_bytes_for_apf_inst) ||
	    nla_put_u32(skb, APF_VERSION, apf_get_offload->apf_version)) {
		hdd_err("nla put failure");
		goto nla_put_failure;
	}

	cfg80211_vendor_cmd_reply(skb);
	hdd_exit();
	return 0;

nla_put_failure:
	kfree_skb(skb);
	return -EINVAL;
}

/**
 * hdd_get_apf_offload - Get APF offload Capabilities
 * @hdd_ctx: Hdd context
 *
 * Return: 0 on success, errno on failure
 */
static int hdd_get_apf_offload(struct hdd_context *hdd_ctx)
{
	QDF_STATUS status;
	int ret;
	void *cookie;
	struct hdd_request *request;
	struct apf_offload_priv *priv;
	static const struct hdd_request_params params = {
		.priv_size = sizeof(*priv),
		.timeout_ms = WLAN_WAIT_TIME_APF,
	};

	hdd_enter();

	request = hdd_request_alloc(&params);
	if (!request) {
		hdd_err("Unable to allocate request");
		return -EINVAL;
	}
	cookie = hdd_request_cookie(request);

	status = sme_get_apf_offload_capabilities(hdd_ctx->hHal,
						  hdd_get_apf_offload_cb,
						  cookie);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("Unable to retrieve APF caps");
		ret = qdf_status_to_os_return(status);
		goto cleanup;
	}
	ret = hdd_request_wait_for_response(request);
	if (ret) {
		hdd_err("Target response timed out");
		goto cleanup;
	}
	priv = hdd_request_priv(request);
	ret = hdd_post_get_apf_capabilities_rsp(hdd_ctx,
						&priv->apf_get_offload);
	if (ret)
		hdd_err("Failed to post get apf capabilities");

cleanup:
	/*
	 * either we never sent a request to SME, we sent a request to
	 * SME and timed out, or we sent a request to SME, received a
	 * response from SME, and posted the response to userspace.
	 * regardless we are done with the request.
	 */
	hdd_request_put(request);
	hdd_exit();

	return ret;
}

/**
 * hdd_set_reset_apf_offload - Post set/reset apf to SME
 * @hdd_ctx: Hdd context
 * @tb: Length of @data
 * @adapter: pointer to adapter struct
 *
 * Return: 0 on success; errno on failure
 */
static int hdd_set_reset_apf_offload(struct hdd_context *hdd_ctx,
				     struct nlattr **tb,
				     struct hdd_adapter *adapter)
{
	struct sir_apf_set_offload *apf_set_offload;
	QDF_STATUS status;
	int prog_len;
	int ret = 0;

	hdd_enter();

	if (adapter->device_mode == QDF_STA_MODE ||
	    adapter->device_mode == QDF_P2P_CLIENT_MODE) {
		if (!hdd_conn_is_connected(
		    WLAN_HDD_GET_STATION_CTX_PTR(adapter))) {
			hdd_err("Not in Connected state!");
			return -ENOTSUPP;
		}
	}

	apf_set_offload = qdf_mem_malloc(sizeof(*apf_set_offload));
	if (!apf_set_offload) {
		hdd_err("qdf_mem_malloc failed for apf_set_offload");
		return -ENOMEM;
	}

	/* Parse and fetch apf packet size */
	if (!tb[APF_PACKET_SIZE]) {
		hdd_err("attr apf packet size failed");
		ret = -EINVAL;
		goto fail;
	}
	apf_set_offload->total_length = nla_get_u32(tb[APF_PACKET_SIZE]);

	if (!apf_set_offload->total_length) {
		hdd_debug("APF reset packet filter received");
		goto post_sme;
	}

	/* Parse and fetch apf program */
	if (!tb[APF_PROGRAM]) {
		hdd_err("attr apf program failed");
		ret = -EINVAL;
		goto fail;
	}

	prog_len = nla_len(tb[APF_PROGRAM]);
	apf_set_offload->program = qdf_mem_malloc(sizeof(uint8_t) * prog_len);

	if (!apf_set_offload->program) {
		hdd_err("qdf_mem_malloc failed for apf offload program");
		ret = -ENOMEM;
		goto fail;
	}

	apf_set_offload->current_length = prog_len;
	nla_memcpy(apf_set_offload->program, tb[APF_PROGRAM], prog_len);
	apf_set_offload->session_id = adapter->session_id;

	hdd_debug("APF set instructions");
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_DEBUG,
			   apf_set_offload->program, prog_len);

	/* Parse and fetch filter Id */
	if (!tb[APF_FILTER_ID]) {
		hdd_err("attr filter id failed");
		ret = -EINVAL;
		goto fail;
	}
	apf_set_offload->filter_id = nla_get_u32(tb[APF_FILTER_ID]);

	/* Parse and fetch current offset */
	if (!tb[APF_CURRENT_OFFSET]) {
		hdd_err("attr current offset failed");
		ret = -EINVAL;
		goto fail;
	}
	apf_set_offload->current_offset = nla_get_u32(tb[APF_CURRENT_OFFSET]);

post_sme:
	hdd_debug("Posting APF SET/RESET to SME, session_id: %d APF Version: %d filter ID: %d total_length: %d current_length: %d current offset: %d",
		  apf_set_offload->session_id, apf_set_offload->version,
		  apf_set_offload->filter_id, apf_set_offload->total_length,
		  apf_set_offload->current_length,
		  apf_set_offload->current_offset);

	status = sme_set_apf_instructions(hdd_ctx->hHal, apf_set_offload);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("sme_set_apf_instructions failed(err=%d)", status);
		ret = -EINVAL;
		goto fail;
	}
	hdd_exit();

fail:
	if (apf_set_offload->current_length)
		qdf_mem_free(apf_set_offload->program);
	qdf_mem_free(apf_set_offload);
	return ret;
}

/**
 * wlan_hdd_cfg80211_apf_offload() - Set/Reset to APF Offload
 * @wiphy:    wiphy structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of @data
 *
 * Return: 0 on success; errno on failure
 */
static int
__wlan_hdd_cfg80211_apf_offload(struct wiphy *wiphy,
				struct wireless_dev *wdev,
				const void *data, int data_len)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter =  WLAN_HDD_GET_PRIV_PTR(dev);
	struct nlattr *tb[APF_MAX + 1];
	int ret_val, packet_filter_subcmd;

	hdd_enter();

	ret_val = wlan_hdd_validate_context(hdd_ctx);
	if (ret_val)
		return ret_val;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (!hdd_ctx->apf_enabled) {
		hdd_err("APF offload is not supported/enabled");
		return -ENOTSUPP;
	}

	if (wlan_cfg80211_nla_parse(tb, APF_MAX, data, data_len,
				    wlan_hdd_apf_offload_policy)) {
		hdd_err("Invalid ATTR");
		return -EINVAL;
	}

	if (!tb[APF_SET_RESET]) {
		hdd_err("attr apf set reset failed");
		return -EINVAL;
	}

	packet_filter_subcmd = nla_get_u32(tb[APF_SET_RESET]);

	if (packet_filter_subcmd == QCA_WLAN_GET_PACKET_FILTER)
		return hdd_get_apf_offload(hdd_ctx);
	else
		return hdd_set_reset_apf_offload(hdd_ctx, tb,
						 adapter);
}

int
wlan_hdd_cfg80211_apf_offload(struct wiphy *wiphy, struct wireless_dev *wdev,
			      const void *data, int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_apf_offload(wiphy, wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

