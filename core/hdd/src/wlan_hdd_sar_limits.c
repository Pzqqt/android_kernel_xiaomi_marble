/*
 * Copyright (c) 2012-2020 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_hdd_vendor_sar_limits.c
 *
 * WLAN SAR limits functions
 *
 */

#include "osif_sync.h"
#include <wlan_hdd_includes.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <linux/if_ether.h>
#include <wlan_osif_request_manager.h>
#include <wlan_hdd_sar_limits.h>

#define WLAN_WAIT_TIME_SAR 5000
/**
 * hdd_sar_context - hdd sar context
 * @event: sar limit event
 */
struct hdd_sar_context {
	struct sar_limit_event event;
};

static u32 hdd_sar_wmi_to_nl_enable(uint32_t wmi_value)
{
	switch (wmi_value) {
	default:
	case WMI_SAR_FEATURE_OFF:
		return QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_NONE;
	case WMI_SAR_FEATURE_ON_SET_0:
		return QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_BDF0;
	case WMI_SAR_FEATURE_ON_SET_1:
		return QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_BDF1;
	case WMI_SAR_FEATURE_ON_SET_2:
		return QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_BDF2;
	case WMI_SAR_FEATURE_ON_SET_3:
		return QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_BDF3;
	case WMI_SAR_FEATURE_ON_SET_4:
		return QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_BDF4;
	case WMI_SAR_FEATURE_ON_USER_DEFINED:
		return QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_USER;
	case WMI_SAR_FEATURE_ON_SAR_V2_0:
		return QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_V2_0;
	}
}

static u32 hdd_sar_wmi_to_nl_band(uint32_t wmi_value)
{
	switch (wmi_value) {
	default:
	case WMI_SAR_2G_ID:
		return HDD_NL80211_BAND_2GHZ;
	case WMI_SAR_5G_ID:
		return HDD_NL80211_BAND_5GHZ;
	}
}

static u32 hdd_sar_wmi_to_nl_modulation(uint32_t wmi_value)
{
	switch (wmi_value) {
	default:
	case WMI_SAR_MOD_CCK:
		return QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_MODULATION_CCK;
	case WMI_SAR_MOD_OFDM:
		return QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_MODULATION_OFDM;
	}
}

/**
 * hdd_sar_cb () - sar response message handler
 * @cookie: hdd request cookie
 * @event: sar response event
 *
 * Return: none
 */
static void hdd_sar_cb(void *cookie,
		       struct sar_limit_event *event)
{
	struct osif_request *request;
	struct hdd_sar_context *context;

	hdd_enter();

	if (!event) {
		hdd_err("response is NULL");
		return;
	}

	request = osif_request_get(cookie);
	if (!request) {
		hdd_debug("Obsolete request");
		return;
	}

	context = osif_request_priv(request);
	context->event = *event;
	osif_request_complete(request);
	osif_request_put(request);

	hdd_exit();
}

static uint32_t hdd_sar_get_response_len(const struct sar_limit_event *event)
{
	uint32_t len;
	uint32_t row_len;

	len = NLMSG_HDRLEN;
	/* QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SAR_ENABLE */
	len += NLA_HDRLEN + sizeof(u32);
	/* QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_NUM_SPECS */
	len += NLA_HDRLEN + sizeof(u32);

	/* nest */
	row_len = NLA_HDRLEN;
	/* QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_BAND */
	row_len += NLA_HDRLEN + sizeof(u32);
	/* QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_CHAIN */
	row_len += NLA_HDRLEN + sizeof(u32);
	/* QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_MODULATION */
	row_len += NLA_HDRLEN + sizeof(u32);
	/* QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_POWER_LIMIT */
	row_len += NLA_HDRLEN + sizeof(u32);

	/* QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC */
	len += NLA_HDRLEN + (row_len * event->num_limit_rows);

	return len;
}

static int hdd_sar_fill_response(struct sk_buff *skb,
				 const struct sar_limit_event *event)
{
	int errno;
	u32 value;
	u32 attr;
	struct nlattr *nla_spec_attr;
	struct nlattr *nla_row_attr;
	uint32_t row;
	const struct sar_limit_event_row *event_row;

	attr = QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SAR_ENABLE;
	value = hdd_sar_wmi_to_nl_enable(event->sar_enable);
	errno = nla_put_u32(skb, attr, value);
	if (errno)
		return errno;

	attr = QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_NUM_SPECS;
	value = event->num_limit_rows;
	errno = nla_put_u32(skb, attr, value);
	if (errno)
		return errno;

	attr = QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC;
	nla_spec_attr = nla_nest_start(skb, attr);
	if (!nla_spec_attr)
		return -EINVAL;

	for (row = 0, event_row = event->sar_limit_row;
	     row < event->num_limit_rows;
	     row++, event_row++) {
		nla_row_attr = nla_nest_start(skb, attr);
		if (!nla_row_attr)
			return -EINVAL;

		attr = QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_BAND;
		value = hdd_sar_wmi_to_nl_band(event_row->band_id);
		errno = nla_put_u32(skb, attr, value);
		if (errno)
			return errno;

		attr = QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_CHAIN;
		value = event_row->chain_id;
		errno = nla_put_u32(skb, attr, value);
		if (errno)
			return errno;

		attr = QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_MODULATION;
		value = hdd_sar_wmi_to_nl_modulation(event_row->mod_id);
		errno = nla_put_u32(skb, attr, value);
		if (errno)
			return errno;

		attr = QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_POWER_LIMIT;
		value = event_row->limit_value;
		errno = nla_put_u32(skb, attr, value);
		if (errno)
			return errno;

		nla_nest_end(skb, nla_row_attr);
	}
	nla_nest_end(skb, nla_spec_attr);

	return 0;
}

static int hdd_sar_send_response(struct wiphy *wiphy,
				 const struct sar_limit_event *event)
{
	uint32_t len;
	struct sk_buff *skb;
	int errno;

	len = hdd_sar_get_response_len(event);
	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, len);
	if (!skb) {
		hdd_err("cfg80211_vendor_cmd_alloc_reply_skb failed");
		return -ENOMEM;
	}

	errno = hdd_sar_fill_response(skb, event);
	if (errno) {
		kfree_skb(skb);
		return errno;
	}

	return cfg80211_vendor_cmd_reply(skb);
}

/**
 * __wlan_hdd_get_sar_power_limits() - Get SAR power limits
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Length of @data
 *
 * This function is used to retrieve Specific Absorption Rate limit specs.
 *
 * Return: 0 on success, negative errno on failure
 */
static int __wlan_hdd_get_sar_power_limits(struct wiphy *wiphy,
					   struct wireless_dev *wdev,
					   const void *data, int data_len)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct osif_request *request;
	struct hdd_sar_context *context;
	mac_handle_t mac_handle;
	void *cookie;
	QDF_STATUS status;
	int ret;
	static const struct osif_request_params params = {
		.priv_size = sizeof(*context),
		.timeout_ms = WLAN_WAIT_TIME_SAR,
	};

	hdd_enter();

	if (hdd_get_conparam() == QDF_GLOBAL_FTM_MODE) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	if (wlan_hdd_validate_context(hdd_ctx))
		return -EINVAL;

	request = osif_request_alloc(&params);
	if (!request) {
		hdd_err("Request allocation failure");
		return -ENOMEM;
	}

	cookie = osif_request_cookie(request);

	mac_handle = hdd_ctx->mac_handle;
	status = sme_get_sar_power_limits(mac_handle, hdd_sar_cb, cookie);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("Unable to post sar message");
		ret = -EINVAL;
		goto cleanup;
	}

	ret = osif_request_wait_for_response(request);
	if (ret) {
		hdd_err("Target response timed out");
		goto cleanup;
	}

	context = osif_request_priv(request);
	ret = hdd_sar_send_response(wiphy, &context->event);

cleanup:
	osif_request_put(request);

	return ret;
}

int wlan_hdd_cfg80211_get_sar_power_limits(struct wiphy *wiphy,
					   struct wireless_dev *wdev,
					   const void *data,
					   int data_len)
{
	struct osif_psoc_sync *psoc_sync;
	int errno;

	errno = osif_psoc_sync_op_start(wiphy_dev(wiphy), &psoc_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_get_sar_power_limits(wiphy, wdev, data, data_len);

	osif_psoc_sync_op_stop(psoc_sync);

	return errno;
}

#ifdef SAR_SAFETY_FEATURE
void hdd_disable_sar(struct hdd_context *hdd_ctx)
{
	struct sar_limit_cmd_params *sar_limit_cmd;
	struct sar_limit_cmd_row *row;
	QDF_STATUS status;

	if (hdd_ctx->sar_version != SAR_VERSION_2) {
		hdd_nofl_debug("FW SAR version: %d", hdd_ctx->sar_version);
		return;
	}

	sar_limit_cmd = qdf_mem_malloc(sizeof(struct sar_limit_cmd_params));
	if (!sar_limit_cmd)
		return;

	/*
	 * Need two rows to hold the per-chain V2 power index
	 */
	row = qdf_mem_malloc(2 * sizeof(*row));
	if (!row)
		goto config_sar_failed;

	sar_limit_cmd->sar_enable = WMI_SAR_FEATURE_OFF;
	sar_limit_cmd->commit_limits = 1;
	sar_limit_cmd->num_limit_rows = 2;
	sar_limit_cmd->sar_limit_row_list = row;
	row[0].limit_value = 0;
	row[1].limit_value = 0;
	row[0].chain_id = 0;
	row[1].chain_id = 1;
	row[0].validity_bitmap = WMI_SAR_CHAIN_ID_VALID_MASK;
	row[1].validity_bitmap = WMI_SAR_CHAIN_ID_VALID_MASK;

	hdd_nofl_debug("Disable the SAR limit index for both the chains");

	status = sme_set_sar_power_limits(hdd_ctx->mac_handle, sar_limit_cmd);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_nofl_err("Failed to set sar power limits");
		goto config_sar_failed;
	}

	/* After SSR, the SAR configuration is lost. As SSR is hidden from
	 * userland, this command will not come from userspace after a SSR. To
	 * restore this configuration, save this in hdd context and restore
	 * after re-init.
	 */
	hdd_store_sar_config(hdd_ctx, sar_limit_cmd);
	return;

config_sar_failed:

	if (sar_limit_cmd) {
		qdf_mem_free(sar_limit_cmd->sar_limit_row_list);
		qdf_mem_free(sar_limit_cmd);
	}
}

void hdd_configure_sar_index(struct hdd_context *hdd_ctx, uint32_t sar_index)
{
	struct sar_limit_cmd_params *sar_limit_cmd;
	struct sar_limit_cmd_row *row;
	QDF_STATUS status;

	if (hdd_ctx->sar_version != SAR_VERSION_2) {
		hdd_nofl_debug("FW SAR version: %d", hdd_ctx->sar_version);
		return;
	}

	sar_limit_cmd = qdf_mem_malloc(sizeof(struct sar_limit_cmd_params));
	if (!sar_limit_cmd)
		return;

	/*
	 * Need two rows to hold the per-chain V2 power index
	 */
	row = qdf_mem_malloc(2 * sizeof(*row));
	if (!row)
		goto config_sar_failed;

	sar_limit_cmd->sar_enable = WMI_SAR_FEATURE_ON_SAR_V2_0;
	sar_limit_cmd->commit_limits = 1;
	sar_limit_cmd->num_limit_rows = 2;
	sar_limit_cmd->sar_limit_row_list = row;
	row[0].limit_value = sar_index;
	row[1].limit_value = sar_index;
	row[0].chain_id = 0;
	row[1].chain_id = 1;
	row[0].validity_bitmap = WMI_SAR_CHAIN_ID_VALID_MASK;
	row[1].validity_bitmap = WMI_SAR_CHAIN_ID_VALID_MASK;

	hdd_nofl_debug("Configure POW_Limit Index: %d for both the chains",
		       row->limit_value);

	status = sme_set_sar_power_limits(hdd_ctx->mac_handle, sar_limit_cmd);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_nofl_err("Failed to set sar power limits");
		goto config_sar_failed;
	}

	/*
	 * After SSR, the SAR configuration is lost. As SSR is hidden from
	 * userland, this command will not come from userspace after a SSR. To
	 * restore this configuration, save this in hdd context and restore
	 * after re-init.
	 */
	hdd_store_sar_config(hdd_ctx, sar_limit_cmd);
	return;

config_sar_failed:

	if (sar_limit_cmd) {
		qdf_mem_free(sar_limit_cmd->sar_limit_row_list);
		qdf_mem_free(sar_limit_cmd);
	}
}

void hdd_configure_sar_sleep_index(struct hdd_context *hdd_ctx)
{
	if (!hdd_ctx->config->enable_sar_safety)
		return;

	if (hdd_ctx->config->config_sar_safety_sleep_index) {
		hdd_nofl_debug("Configure SAR sleep index %d",
			       hdd_ctx->config->sar_safety_sleep_index);
		hdd_configure_sar_index(
				hdd_ctx,
				hdd_ctx->config->sar_safety_sleep_index);
	} else {
		hdd_nofl_debug("Disable SAR");
		hdd_disable_sar(hdd_ctx);
	}
}

void hdd_configure_sar_resume_index(struct hdd_context *hdd_ctx)
{
	if (!hdd_ctx->config->enable_sar_safety)
		return;

	hdd_nofl_debug("Configure SAR safety index %d on wlan resume",
		       hdd_ctx->config->sar_safety_index);
	hdd_configure_sar_index(hdd_ctx,
				hdd_ctx->config->sar_safety_index);
}

static void hdd_send_sar_unsolicited_event(struct hdd_context *hdd_ctx)
{
	struct sk_buff *vendor_event;
	uint32_t len;

	if (!hdd_ctx) {
		hdd_err_rl("hdd context is null");
		return;
	}

	len = NLMSG_HDRLEN;
	vendor_event =
		cfg80211_vendor_event_alloc(
			hdd_ctx->wiphy, NULL, len,
			QCA_NL80211_VENDOR_SUBCMD_REQUEST_SAR_LIMITS_INDEX,
			GFP_KERNEL);

	if (!vendor_event) {
		hdd_err("cfg80211_vendor_event_alloc failed");
		return;
	}

	cfg80211_vendor_event(vendor_event, GFP_KERNEL);
}

static void hdd_sar_unsolicited_timer_cb(void *user_data)
{
	struct hdd_context *hdd_ctx = (struct hdd_context *)user_data;
	uint8_t i = 0;
	QDF_STATUS status;

	hdd_nofl_debug("Sar unsolicited timer expired");

	for (i = 0; i < hdd_ctx->config->sar_safety_req_resp_retry; i++) {
		hdd_send_sar_unsolicited_event(hdd_ctx);
		status = qdf_wait_for_event_completion(
				&hdd_ctx->sar_safety_req_resp_event,
				hdd_ctx->config->sar_safety_req_resp_timeout);
		if (QDF_IS_STATUS_SUCCESS(status))
			break;
	}

	if (i >= hdd_ctx->config->sar_safety_req_resp_retry)
		hdd_configure_sar_index(hdd_ctx,
					hdd_ctx->config->sar_safety_index);
}

static void hdd_sar_safety_timer_cb(void *user_data)
{
	struct hdd_context *hdd_ctx = (struct hdd_context *)user_data;

	hdd_nofl_debug("Sar safety timer expires");
	hdd_configure_sar_index(hdd_ctx, hdd_ctx->config->sar_safety_index);
}

void wlan_hdd_sar_unsolicited_timer_start(struct hdd_context *hdd_ctx)
{
	if (!hdd_ctx->config->enable_sar_safety)
		return;

	if (QDF_TIMER_STATE_RUNNING !=
		qdf_mc_timer_get_current_state(
				&hdd_ctx->sar_safety_unsolicited_timer))
		qdf_mc_timer_start(
			&hdd_ctx->sar_safety_unsolicited_timer,
			hdd_ctx->config->sar_safety_unsolicited_timeout);
}

void wlan_hdd_sar_timers_reset(struct hdd_context *hdd_ctx)
{
	if (!hdd_ctx->config->enable_sar_safety)
		return;

	if (hdd_ctx->sar_version != SAR_VERSION_2)
		return;

	if (QDF_TIMER_STATE_RUNNING ==
		qdf_mc_timer_get_current_state(&hdd_ctx->sar_safety_timer))
		qdf_mc_timer_stop(&hdd_ctx->sar_safety_timer);

	qdf_mc_timer_start(&hdd_ctx->sar_safety_timer,
			   hdd_ctx->config->sar_safety_timeout);

	if (QDF_TIMER_STATE_RUNNING ==
		qdf_mc_timer_get_current_state(
				&hdd_ctx->sar_safety_unsolicited_timer))
		qdf_mc_timer_stop(&hdd_ctx->sar_safety_unsolicited_timer);

	qdf_event_set(&hdd_ctx->sar_safety_req_resp_event);
}

void wlan_hdd_sar_timers_init(struct hdd_context *hdd_ctx)
{
	if (!hdd_ctx->config->enable_sar_safety)
		return;

	qdf_mc_timer_init(&hdd_ctx->sar_safety_timer, QDF_TIMER_TYPE_SW,
			  hdd_sar_safety_timer_cb, hdd_ctx);

	qdf_mc_timer_init(&hdd_ctx->sar_safety_unsolicited_timer,
			  QDF_TIMER_TYPE_SW,
			  hdd_sar_unsolicited_timer_cb, hdd_ctx);

	qdf_event_create(&hdd_ctx->sar_safety_req_resp_event);
}

void wlan_hdd_sar_timers_deinit(struct hdd_context *hdd_ctx)
{
	if (!hdd_ctx->config->enable_sar_safety)
		return;

	if (QDF_TIMER_STATE_RUNNING ==
		qdf_mc_timer_get_current_state(&hdd_ctx->sar_safety_timer))
		qdf_mc_timer_stop(&hdd_ctx->sar_safety_timer);

	qdf_mc_timer_destroy(&hdd_ctx->sar_safety_timer);

	if (QDF_TIMER_STATE_RUNNING ==
		qdf_mc_timer_get_current_state(
				&hdd_ctx->sar_safety_unsolicited_timer))
		qdf_mc_timer_stop(&hdd_ctx->sar_safety_unsolicited_timer);

	qdf_mc_timer_destroy(&hdd_ctx->sar_safety_unsolicited_timer);

	qdf_event_destroy(&hdd_ctx->sar_safety_req_resp_event);
}
#endif

