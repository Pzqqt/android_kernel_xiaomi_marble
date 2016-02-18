/*
 * Copyright (c) 2012-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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
 * DOC: wlan_hdd_ext_scan.c
 *
 * WLAN Host Device Driver EXT SCAN feature implementation
 *
 */

#ifdef FEATURE_WLAN_EXTSCAN

#include "wlan_hdd_ext_scan.h"
#include "cds_utils.h"
#include "cds_sched.h"

/* amount of time to wait for a synchronous request/response operation */
#define WLAN_WAIT_TIME_EXTSCAN  1000

/**
 * struct hdd_ext_scan_context - hdd ext scan context
 * @request_id: userspace-assigned ID associated with the request
 * @response_event: Ext scan wait event
 * @response_status: Status returned by FW in response to a request
 * @ignore_cached_results: Flag to ignore cached results or not
 * @context_lock: Spinlock to serialize all context accesses
 * @capability_response: Ext scan capability response data from target
 */
struct hdd_ext_scan_context {
	uint32_t request_id;
	int response_status;
	bool ignore_cached_results;
	struct completion response_event;
	spinlock_t context_lock;
	struct ext_scan_capabilities_response capability_response;
};
static struct hdd_ext_scan_context ext_scan_context;

static const struct nla_policy wlan_hdd_extscan_config_policy
[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_REQUEST_ID] = {
				.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_GET_VALID_CHANNELS_CONFIG_PARAM_WIFI_BAND] = {
				.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_CHANNEL_SPEC_CHANNEL] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_CHANNEL_SPEC_DWELL_TIME] = {
				.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_CHANNEL_SPEC_PASSIVE] = {.type = NLA_U8},
	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_CHANNEL_SPEC_CLASS] = {.type = NLA_U8},

	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_INDEX] = {.type = NLA_U8},
	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_BAND] = {.type = NLA_U8},
	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_PERIOD] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_REPORT_EVENTS] = {
				.type = NLA_U8},
	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_NUM_CHANNEL_SPECS] = {
				.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SCAN_CMD_PARAMS_BASE_PERIOD] = {
				.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SCAN_CMD_PARAMS_MAX_AP_PER_SCAN] = {
				.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SCAN_CMD_PARAMS_REPORT_THRESHOLD_PERCENT] = {
				.type = NLA_U8},
	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SCAN_CMD_PARAMS_REPORT_THRESHOLD_NUM_SCANS] = {
				.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SCAN_CMD_PARAMS_NUM_BUCKETS] = {
				.type = NLA_U8},
	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_GET_CACHED_SCAN_RESULTS_CONFIG_PARAM_FLUSH] = {
				.type = NLA_U8},

	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_GET_CACHED_SCAN_RESULTS_CONFIG_PARAM_MAX] = {
				.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_AP_THRESHOLD_PARAM_BSSID] = {
				.type = NLA_UNSPEC},
	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_AP_THRESHOLD_PARAM_RSSI_LOW] = {
				.type = NLA_S32},
	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_AP_THRESHOLD_PARAM_RSSI_HIGH] = {
				.type = NLA_S32},
	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_AP_THRESHOLD_PARAM_CHANNEL] = {
				.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_BSSID_HOTLIST_PARAMS_NUM_AP] = {
				.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SIGNIFICANT_CHANGE_PARAMS_RSSI_SAMPLE_SIZE] = {
				.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SIGNIFICANT_CHANGE_PARAMS_LOST_AP_SAMPLE_SIZE] = {
				.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SIGNIFICANT_CHANGE_PARAMS_MIN_BREACHING] = {
				.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SIGNIFICANT_CHANGE_PARAMS_NUM_AP] = {
				.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_NUM_NETWORKS] = {
				.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORK_SSID] = {
				.type = NLA_BINARY,
				.len = IEEE80211_MAX_SSID_LEN },
	[QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORK_RSSI_THRESHOLD] = {
				.type = NLA_S8 },
	[QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORK_FLAGS] = {
				.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORK_AUTH_BIT] = {
				.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SSID_THRESHOLD_PARAM_SSID] = {
				.type = NLA_BINARY,
				.len = IEEE80211_MAX_SSID_LEN + 1 },
	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SSID_HOTLIST_PARAMS_LOST_SSID_SAMPLE_SIZE] = {
				.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SSID_HOTLIST_PARAMS_NUM_SSID] = {
				.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SSID_THRESHOLD_PARAM_BAND] = {
				.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SSID_THRESHOLD_PARAM_RSSI_LOW] = {
				.type = NLA_S32 },
	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SSID_THRESHOLD_PARAM_RSSI_HIGH] = {
				.type = NLA_S32 },
	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_CONFIGURATION_FLAGS] = {
				.type = NLA_U32 },
};

static const struct nla_policy
wlan_hdd_extscan_results_policy[QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_BEACON_PERIOD] = {
				.type = NLA_U16},
	[QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_CAPABILITY] = {
				.type = NLA_U16},
};

/**
 * wlan_hdd_cfg80211_extscan_get_capabilities_rsp() - response from target
 * @ctx: Pointer to hdd context
 * @data: Pointer to ext scan capabilities response from fw
 *
 * Return: None
 */
static void
wlan_hdd_cfg80211_extscan_get_capabilities_rsp(void *ctx,
	struct ext_scan_capabilities_response *data)
{
	struct hdd_ext_scan_context *context;
	hdd_context_t *hdd_ctx  = ctx;

	ENTER();

	if (wlan_hdd_validate_context(hdd_ctx))
		return;
	if (!data) {
		hddLog(LOGE, FL("data is null"));
		return;
	}

	context = &ext_scan_context;

	spin_lock(&context->context_lock);
	/* validate response received from target*/
	if (context->request_id != data->requestId) {
		spin_unlock(&context->context_lock);
		hddLog(LOGE,
			FL("Target response id did not match: request_id %d response_id %d"),
			context->request_id, data->requestId);
		return;
	} else {
		context->capability_response = *data;
		complete(&context->response_event);
	}

	spin_unlock(&context->context_lock);

	return;
}

/*
 * define short names for the global vendor params
 * used by hdd_extscan_nl_fill_bss()
 */
#define PARAM_TIME_STAMP \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_TIME_STAMP
#define PARAM_SSID \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_SSID
#define PARAM_BSSID \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_BSSID
#define PARAM_CHANNEL \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_CHANNEL
#define PARAM_RSSI \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_RSSI
#define PARAM_RTT \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_RTT
#define PARAM_RTT_SD \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_RTT_SD
#define PARAM_BEACON_PERIOD \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_BEACON_PERIOD
#define PARAM_CAPABILITY \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_CAPABILITY
#define PARAM_IE_LENGTH \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_IE_LENGTH
#define PARAM_IE_DATA \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_IE_DATA

/** hdd_extscan_nl_fill_bss() - extscan nl fill bss
 * @skb: socket buffer
 * @ap: bss information
 * @idx: nesting index
 *
 * Return: 0 on success; error number otherwise
 */
static int hdd_extscan_nl_fill_bss(struct sk_buff *skb, tSirWifiScanResult *ap,
					int idx)
{
	struct nlattr *nla_ap;

	nla_ap = nla_nest_start(skb, idx);
	if (!nla_ap)
		return -EINVAL;

	if (nla_put_u64(skb, PARAM_TIME_STAMP, ap->ts) ||
	    nla_put(skb, PARAM_SSID, sizeof(ap->ssid), ap->ssid) ||
	    nla_put(skb, PARAM_BSSID, sizeof(ap->bssid), ap->bssid.bytes) ||
	    nla_put_u32(skb, PARAM_CHANNEL, ap->channel) ||
	    nla_put_s32(skb, PARAM_RSSI, ap->rssi) ||
	    nla_put_u32(skb, PARAM_RTT, ap->rtt) ||
	    nla_put_u32(skb, PARAM_RTT_SD, ap->rtt_sd) ||
	    nla_put_u16(skb, PARAM_BEACON_PERIOD, ap->beaconPeriod) ||
	    nla_put_u16(skb, PARAM_CAPABILITY, ap->capability) ||
	    nla_put_u16(skb, PARAM_IE_LENGTH, ap->ieLength)) {
		hddLog(LOGE, FL("put fail"));
		return -EINVAL;
	}

	if (ap->ieLength)
		if (nla_put(skb, PARAM_IE_DATA, ap->ieLength, ap->ieData)) {
			hddLog(LOGE, FL("put fail"));
			return -EINVAL;
		}

	nla_nest_end(skb, nla_ap);

	return 0;
}
/*
 * done with short names for the global vendor params
 * used by hdd_extscan_nl_fill_bss()
 */
#undef PARAM_TIME_STAMP
#undef PARAM_SSID
#undef PARAM_BSSID
#undef PARAM_CHANNEL
#undef PARAM_RSSI
#undef PARAM_RTT
#undef PARAM_RTT_SD
#undef PARAM_BEACON_PERIOD
#undef PARAM_CAPABILITY
#undef PARAM_IE_LENGTH
#undef PARAM_IE_DATA

/** wlan_hdd_cfg80211_extscan_cached_results_ind() - get cached results
 * @ctx: hdd global context
 * @data: cached results
 *
 * This function reads the cached results %data, populated the NL
 * attributes and sends the NL event to the upper layer.
 *
 * Return: none
 */
static void
wlan_hdd_cfg80211_extscan_cached_results_ind(void *ctx,
				struct extscan_cached_scan_results *data)
{
	hdd_context_t *pHddCtx = ctx;
	struct sk_buff *skb = NULL;
	struct hdd_ext_scan_context *context;
	struct extscan_cached_scan_result *result;
	tSirWifiScanResult *ap;
	uint32_t i, j, nl_buf_len;
	bool ignore_cached_results = false;

	ENTER();

	if (wlan_hdd_validate_context(pHddCtx))
		return;
	if (!data) {
		hddLog(LOGE, FL("data is null"));
		return;
	}

	context = &ext_scan_context;
	spin_lock(&context->context_lock);
	ignore_cached_results = context->ignore_cached_results;
	spin_unlock(&context->context_lock);

	if (ignore_cached_results) {
		hddLog(LOGE,
			FL("Ignore the cached results received after timeout"));
		return;
	}

#define EXTSCAN_CACHED_NEST_HDRLEN NLA_HDRLEN
#define EXTSCAN_CACHED_NL_FIXED_TLV \
		((sizeof(data->request_id) + NLA_HDRLEN) + \
		(sizeof(data->num_scan_ids) + NLA_HDRLEN) + \
		(sizeof(data->more_data) + NLA_HDRLEN))
#define EXTSCAN_CACHED_NL_SCAN_ID_TLV \
		((sizeof(result->scan_id) + NLA_HDRLEN) + \
		(sizeof(result->flags) + NLA_HDRLEN) + \
		(sizeof(result->num_results) + NLA_HDRLEN))
#define EXTSCAN_CACHED_NL_SCAN_RESULTS_TLV \
		((sizeof(ap->ts) + NLA_HDRLEN) + \
		(sizeof(ap->ssid) + NLA_HDRLEN) + \
		(sizeof(ap->bssid) + NLA_HDRLEN) + \
		(sizeof(ap->channel) + NLA_HDRLEN) + \
		(sizeof(ap->rssi) + NLA_HDRLEN) + \
		(sizeof(ap->rtt) + NLA_HDRLEN) + \
		(sizeof(ap->rtt_sd) + NLA_HDRLEN) + \
		(sizeof(ap->beaconPeriod) + NLA_HDRLEN) + \
		(sizeof(ap->capability) + NLA_HDRLEN) + \
		(sizeof(ap->ieLength) + NLA_HDRLEN))
#define EXTSCAN_CACHED_NL_SCAN_RESULTS_IE_DATA_TLV \
		(ap->ieLength + NLA_HDRLEN)

	nl_buf_len = NLMSG_HDRLEN;
	nl_buf_len += EXTSCAN_CACHED_NL_FIXED_TLV;
	if (data->num_scan_ids) {
		nl_buf_len += sizeof(result->scan_id) + NLA_HDRLEN;
		nl_buf_len += EXTSCAN_CACHED_NEST_HDRLEN;
		result = &data->result[0];
		for (i = 0; i < data->num_scan_ids; i++) {
			nl_buf_len += EXTSCAN_CACHED_NEST_HDRLEN;
			nl_buf_len += EXTSCAN_CACHED_NL_SCAN_ID_TLV;
			nl_buf_len += EXTSCAN_CACHED_NEST_HDRLEN;

			ap = &result->ap[0];
			for (j = 0; j < result->num_results; j++) {
				nl_buf_len += EXTSCAN_CACHED_NEST_HDRLEN;
				nl_buf_len +=
					EXTSCAN_CACHED_NL_SCAN_RESULTS_TLV;
				if (ap->ieLength)
					nl_buf_len +=
					EXTSCAN_CACHED_NL_SCAN_RESULTS_IE_DATA_TLV;
				ap++;
			}
			result++;
		}
	}

	hddLog(LOG1, FL("nl_buf_len = %u"), nl_buf_len);
	skb = cfg80211_vendor_cmd_alloc_reply_skb(pHddCtx->wiphy, nl_buf_len);

	if (!skb) {
		hddLog(LOGE, FL("cfg80211_vendor_cmd_alloc_reply_skb failed"));
		goto fail;
	}
	hddLog(LOG1, "Req Id %u Num_scan_ids %u More Data %u",
		data->request_id, data->num_scan_ids, data->more_data);

	result = &data->result[0];
	for (i = 0; i < data->num_scan_ids; i++) {
		hddLog(LOG1, "[i=%d] scan_id %u flags %u num_results %u",
			i, result->scan_id, result->flags, result->num_results);

		ap = &result->ap[0];
		for (j = 0; j < result->num_results; j++) {
			/*
			 * Firmware returns timestamp from ext scan start till
			 * BSSID was cached (in micro seconds). Add this with
			 * time gap between system boot up to ext scan start
			 * to derive the time since boot when the
			 * BSSID was cached.
			 */
			ap->ts += pHddCtx->ext_scan_start_since_boot;
			hddLog(LOG1, "Timestamp %llu "
				"Ssid: %s "
				"Bssid (" MAC_ADDRESS_STR ") "
				"Channel %u "
				"Rssi %d "
				"RTT %u "
				"RTT_SD %u "
				"Beacon Period %u "
				"Capability 0x%x "
				"Ie length %d",
				ap->ts,
				ap->ssid,
				MAC_ADDR_ARRAY(ap->bssid.bytes),
				ap->channel,
				ap->rssi,
				ap->rtt,
				ap->rtt_sd,
				ap->beaconPeriod,
				ap->capability,
				ap->ieLength);
			ap++;
		}
		result++;
	}

	if (nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_REQUEST_ID,
		data->request_id) ||
	    nla_put_u32(skb,
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_NUM_RESULTS_AVAILABLE,
		data->num_scan_ids) ||
	    nla_put_u8(skb,
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_MORE_DATA,
		data->more_data)) {
		hddLog(LOGE, FL("put fail"));
		goto fail;
	}

	if (data->num_scan_ids) {
		struct nlattr *nla_results;
		result = &data->result[0];

		if (nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_EXTSCAN_CACHED_RESULTS_SCAN_ID,
			result->scan_id)) {
			hddLog(LOGE, FL("put fail"));
			goto fail;
		}
		nla_results = nla_nest_start(skb,
			      QCA_WLAN_VENDOR_ATTR_EXTSCAN_CACHED_RESULTS_LIST);
		if (!nla_results)
			goto fail;

		for (i = 0; i < data->num_scan_ids; i++) {
			struct nlattr *nla_result;
			struct nlattr *nla_aps;

			nla_result = nla_nest_start(skb, i);
			if (!nla_result)
				goto fail;

			if (nla_put_u32(skb,
				QCA_WLAN_VENDOR_ATTR_EXTSCAN_CACHED_RESULTS_SCAN_ID,
				result->scan_id) ||
			    nla_put_u32(skb,
				QCA_WLAN_VENDOR_ATTR_EXTSCAN_CACHED_RESULTS_FLAGS,
				result->flags) ||
			    nla_put_u32(skb,
				QCA_WLAN_VENDOR_ATTR_EXTSCAN_NUM_RESULTS_AVAILABLE,
				result->num_results)) {
				hddLog(LOGE, FL("put fail"));
				goto fail;
			}

			nla_aps = nla_nest_start(skb,
				     QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_LIST);
			if (!nla_aps)
				goto fail;

			ap = &result->ap[0];
			for (j = 0; j < result->num_results; j++) {
				if (hdd_extscan_nl_fill_bss(skb, ap, j))
					goto fail;

				ap++;
			}
			nla_nest_end(skb, nla_aps);
			nla_nest_end(skb, nla_result);
			result++;
		}
		nla_nest_end(skb, nla_results);
	}

	cfg80211_vendor_cmd_reply(skb);

	if (!data->more_data) {
		spin_lock(&context->context_lock);
		context->response_status = 0;
		complete(&context->response_event);
		spin_unlock(&context->context_lock);
	}
	EXIT();
	return;

fail:
	if (skb)
		kfree_skb(skb);

	spin_lock(&context->context_lock);
	context->response_status = -EINVAL;
	spin_unlock(&context->context_lock);

	return;
}

/**
 * wlan_hdd_cfg80211_extscan_hotlist_match_ind() - hot list match ind
 * @ctx: Pointer to hdd context
 * @pData: Pointer to ext scan result event
 *
 * This callback execute in atomic context and must not invoke any
 * blocking calls.
 *
 * Return: none
 */
static void
wlan_hdd_cfg80211_extscan_hotlist_match_ind(void *ctx,
					    struct extscan_hotlist_match *data)
{
	hdd_context_t *pHddCtx = ctx;
	struct sk_buff *skb = NULL;
	uint32_t i, index;
	int flags = cds_get_gfp_flags();

	ENTER();

	if (wlan_hdd_validate_context(pHddCtx))
		return;
	if (!data) {
		hddLog(LOGE, FL("data is null"));
		return;
	}

	if (data->ap_found)
		index = QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_AP_FOUND_INDEX;
	else
		index = QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_AP_LOST_INDEX;

	skb = cfg80211_vendor_event_alloc(
		  pHddCtx->wiphy,
		  NULL,
		  EXTSCAN_EVENT_BUF_SIZE + NLMSG_HDRLEN,
		  index, flags);

	if (!skb) {
		hddLog(LOGE, FL("cfg80211_vendor_event_alloc failed"));
		return;
	}
	hdd_info("Req Id: %u Num_APs: %u MoreData: %u ap_found: %u",
			data->requestId, data->numOfAps, data->moreData,
			data->ap_found);

	for (i = 0; i < data->numOfAps; i++) {
		data->ap[i].ts = cdf_get_monotonic_boottime();

		hddLog(LOG1, "[i=%d] Timestamp %llu "
		       "Ssid: %s "
		       "Bssid (" MAC_ADDRESS_STR ") "
		       "Channel %u "
		       "Rssi %d "
		       "RTT %u "
		       "RTT_SD %u",
		       i,
		       data->ap[i].ts,
		       data->ap[i].ssid,
		       MAC_ADDR_ARRAY(data->ap[i].bssid.bytes),
		       data->ap[i].channel,
		       data->ap[i].rssi,
		       data->ap[i].rtt, data->ap[i].rtt_sd);
	}

	if (nla_put_u32(skb,
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_REQUEST_ID,
		data->requestId) ||
	    nla_put_u32(skb,
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_NUM_RESULTS_AVAILABLE,
		data->numOfAps)) {
		hddLog(LOGE, FL("put fail"));
		goto fail;
	}

	if (data->numOfAps) {
		struct nlattr *aps;

		aps = nla_nest_start(skb,
			       QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_LIST);
		if (!aps)
			goto fail;

		for (i = 0; i < data->numOfAps; i++) {
			struct nlattr *ap;

			ap = nla_nest_start(skb, i);
			if (!ap)
				goto fail;

			if (nla_put_u64(skb,
				QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_TIME_STAMP,
				data->ap[i].ts) ||
			    nla_put(skb,
				QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_SSID,
				sizeof(data->ap[i].ssid),
				data->ap[i].ssid) ||
			    nla_put(skb,
				QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_BSSID,
				sizeof(data->ap[i].bssid),
				data->ap[i].bssid.bytes) ||
			    nla_put_u32(skb,
				QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_CHANNEL,
				data->ap[i].channel) ||
			    nla_put_s32(skb,
				QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_RSSI,
				data->ap[i].rssi) ||
			    nla_put_u32(skb,
				QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_RTT,
				data->ap[i].rtt) ||
			    nla_put_u32(skb,
				QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_RTT_SD,
				data->ap[i].rtt_sd))
				goto fail;

			nla_nest_end(skb, ap);
		}
		nla_nest_end(skb, aps);

		if (nla_put_u8(skb,
		       QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_MORE_DATA,
		       data->moreData))
			goto fail;
	}

	cfg80211_vendor_event(skb, flags);
	EXIT();
	return;

fail:
	kfree_skb(skb);
	return;
}

/**
 * wlan_hdd_cfg80211_extscan_signif_wifi_change_results_ind() -
 *	significant wifi change results indication
 * @ctx: Pointer to hdd context
 * @pData: Pointer to signif wifi change event
 *
 * This callback execute in atomic context and must not invoke any
 * blocking calls.
 *
 * Return: none
 */
static void
wlan_hdd_cfg80211_extscan_signif_wifi_change_results_ind(
			void *ctx,
			tpSirWifiSignificantChangeEvent pData)
{
	hdd_context_t *pHddCtx = (hdd_context_t *) ctx;
	struct sk_buff *skb = NULL;
	tSirWifiSignificantChange *ap_info;
	int32_t *rssi;
	uint32_t i, j;
	int flags = cds_get_gfp_flags();

	ENTER();

	if (wlan_hdd_validate_context(pHddCtx))
		return;
	if (!pData) {
		hddLog(LOGE, FL("pData is null"));
		return;
	}

	skb = cfg80211_vendor_event_alloc(
		pHddCtx->wiphy,
		NULL,
		EXTSCAN_EVENT_BUF_SIZE + NLMSG_HDRLEN,
		QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SIGNIFICANT_CHANGE_INDEX,
		flags);

	if (!skb) {
		hddLog(LOGE, FL("cfg80211_vendor_event_alloc failed"));
		return;
	}
	hddLog(LOG1, "Req Id %u Num results %u More Data %u",
		pData->requestId, pData->numResults, pData->moreData);

	ap_info = &pData->ap[0];
	for (i = 0; i < pData->numResults; i++) {
		hddLog(LOG1, "[i=%d] "
		       "Bssid (" MAC_ADDRESS_STR ") "
		       "Channel %u "
		       "numOfRssi %d",
		       i,
		       MAC_ADDR_ARRAY(ap_info->bssid.bytes),
		       ap_info->channel, ap_info->numOfRssi);
		rssi = &(ap_info)->rssi[0];
		for (j = 0; j < ap_info->numOfRssi; j++)
			hddLog(LOG1, "Rssi %d", *rssi++);

		ap_info += ap_info->numOfRssi * sizeof(*rssi);
	}

	if (nla_put_u32(skb,
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_REQUEST_ID,
		pData->requestId) ||
	    nla_put_u32(skb,
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_NUM_RESULTS_AVAILABLE,
		pData->numResults)) {
		hddLog(LOGE, FL("put fail"));
		goto fail;
	}

	if (pData->numResults) {
		struct nlattr *aps;

		aps = nla_nest_start(skb,
			       QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_LIST);
		if (!aps)
			goto fail;

		ap_info = &pData->ap[0];
		for (i = 0; i < pData->numResults; i++) {
			struct nlattr *ap;

			ap = nla_nest_start(skb, i);
			if (!ap)
				goto fail;

			if (nla_put(skb,
				QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SIGNIFICANT_CHANGE_RESULT_BSSID,
				CDF_MAC_ADDR_SIZE, ap_info->bssid.bytes) ||
			    nla_put_u32(skb,
				QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SIGNIFICANT_CHANGE_RESULT_CHANNEL,
				ap_info->channel) ||
			    nla_put_u32(skb,
				QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SIGNIFICANT_CHANGE_RESULT_NUM_RSSI,
				ap_info->numOfRssi) ||
			    nla_put(skb,
				QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SIGNIFICANT_CHANGE_RESULT_RSSI_LIST,
				sizeof(s32) * ap_info->numOfRssi,
				&(ap_info)->rssi[0]))
				goto fail;

			nla_nest_end(skb, ap);

			ap_info += ap_info->numOfRssi * sizeof(*rssi);
		}
		nla_nest_end(skb, aps);

		if (nla_put_u8(skb,
		     QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_MORE_DATA,
		     pData->moreData))
			goto fail;
	}

	cfg80211_vendor_event(skb, flags);
	return;

fail:
	kfree_skb(skb);
	return;

}

/**
 * wlan_hdd_cfg80211_extscan_full_scan_result_event() - full scan result event
 * @ctx: Pointer to hdd context
 * @pData: Pointer to full scan result event
 *
 * This callback execute in atomic context and must not invoke any
 * blocking calls.
 *
 * Return: none
 */
static void
wlan_hdd_cfg80211_extscan_full_scan_result_event(void *ctx,
						 tpSirWifiFullScanResultEvent
						 pData)
{
	hdd_context_t *pHddCtx = (hdd_context_t *) ctx;
	struct sk_buff *skb = NULL;
#ifdef CONFIG_CNSS
	struct timespec ts;
#endif
	int flags = cds_get_gfp_flags();

	ENTER();

	if (wlan_hdd_validate_context(pHddCtx))
		return;
	if (!pData) {
		hddLog(LOGE, FL("pData is null"));
		return;
	}

	if ((sizeof(*pData) + pData->ap.ieLength) >= EXTSCAN_EVENT_BUF_SIZE) {
		hddLog(LOGE,
		       FL("Frame exceeded NL size limitation, drop it!!"));
		return;
	}
	skb = cfg80211_vendor_event_alloc(
		  pHddCtx->wiphy,
		  NULL,
		  EXTSCAN_EVENT_BUF_SIZE + NLMSG_HDRLEN,
		  QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_FULL_SCAN_RESULT_INDEX,
		  flags);

	if (!skb) {
		hddLog(LOGE, FL("cfg80211_vendor_event_alloc failed"));
		return;
	}

	pData->ap.channel = cds_chan_to_freq(pData->ap.channel);
#ifdef CONFIG_CNSS
	/* Android does not want the time stamp from the frame.
	   Instead it wants a monotonic increasing value since boot */
	cnss_get_monotonic_boottime(&ts);
	pData->ap.ts = ((u64)ts.tv_sec * 1000000) + (ts.tv_nsec / 1000);
#endif
	hddLog(LOG1, "Req Id %u More Data %u", pData->requestId,
	       pData->moreData);
	hddLog(LOG1, "AP Info: Timestamp %llu Ssid: %s "
	       "Bssid (" MAC_ADDRESS_STR ") "
	       "Channel %u "
	       "Rssi %d "
	       "RTT %u "
	       "RTT_SD %u "
	       "Bcn Period %d "
	       "Capability 0x%X "
	       "IE Length %d",
	       pData->ap.ts,
	       pData->ap.ssid,
	       MAC_ADDR_ARRAY(pData->ap.bssid.bytes),
	       pData->ap.channel,
	       pData->ap.rssi,
	       pData->ap.rtt,
	       pData->ap.rtt_sd,
	       pData->ap.beaconPeriod,
	       pData->ap.capability, pData->ap.ieLength);

	if (nla_put_u32(skb,
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_REQUEST_ID,
		pData->requestId) ||
	    nla_put_u64(skb,
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_TIME_STAMP,
		pData->ap.ts) ||
	    nla_put(skb,
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_SSID,
		sizeof(pData->ap.ssid),
		pData->ap.ssid) ||
	    nla_put(skb, QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_BSSID,
		sizeof(pData->ap.bssid),
		pData->ap.bssid.bytes) ||
	    nla_put_u32(skb,
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_CHANNEL,
		pData->ap.channel) ||
	    nla_put_s32(skb,
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_RSSI,
		pData->ap.rssi) ||
	    nla_put_u32(skb,
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_RTT,
		pData->ap.rtt) ||
	    nla_put_u32(skb,
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_RTT_SD,
		pData->ap.rtt_sd) ||
	    nla_put_u16(skb,
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_BEACON_PERIOD,
		pData->ap.beaconPeriod) ||
	    nla_put_u16(skb,
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_CAPABILITY,
		pData->ap.capability) ||
	    nla_put_u32(skb,
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_IE_LENGTH,
		pData->ap.ieLength) ||
	    nla_put_u8(skb,
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_MORE_DATA,
		pData->moreData)) {
		hddLog(LOGE, FL("nla put fail"));
		goto nla_put_failure;
	}

	if (pData->ap.ieLength) {
		if (nla_put(skb,
		    QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_IE_DATA,
		    pData->ap.ieLength, pData->ap.ieData))
			goto nla_put_failure;
	}

	cfg80211_vendor_event(skb, flags);
	EXIT();
	return;

nla_put_failure:
	kfree_skb(skb);
	return;
}

/**
 * wlan_hdd_cfg80211_extscan_scan_res_available_event() - scan result event
 * @ctx: Pointer to hdd context
 * @pData: Pointer to scan results available indication param
 *
 * This callback execute in atomic context and must not invoke any
 * blocking calls.
 *
 * Return: none
 */
static void
wlan_hdd_cfg80211_extscan_scan_res_available_event(
			void *ctx,
			tpSirExtScanResultsAvailableIndParams pData)
{
	hdd_context_t *pHddCtx = (hdd_context_t *) ctx;
	struct sk_buff *skb = NULL;
	int flags = cds_get_gfp_flags();

	ENTER();

	if (wlan_hdd_validate_context(pHddCtx))
		return;
	if (!pData) {
		hddLog(LOGE, FL("pData is null"));
		return;
	}

	skb = cfg80211_vendor_event_alloc(
		 pHddCtx->wiphy,
		 NULL,
		 EXTSCAN_EVENT_BUF_SIZE + NLMSG_HDRLEN,
		 QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SCAN_RESULTS_AVAILABLE_INDEX,
		 flags);

	if (!skb) {
		hddLog(LOGE, FL("cfg80211_vendor_event_alloc failed"));
		return;
	}

	hddLog(LOG1, "Req Id %u Num results %u",
	       pData->requestId, pData->numResultsAvailable);
	if (nla_put_u32(skb,
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_REQUEST_ID,
		pData->requestId) ||
	    nla_put_u32(skb,
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_NUM_RESULTS_AVAILABLE,
		pData->numResultsAvailable)) {
		hddLog(LOGE, FL("nla put fail"));
		goto nla_put_failure;
	}

	cfg80211_vendor_event(skb, flags);
	EXIT();
	return;

nla_put_failure:
	kfree_skb(skb);
	return;
}

/**
 * wlan_hdd_cfg80211_extscan_scan_progress_event() - scan progress event
 * @ctx: Pointer to hdd context
 * @pData: Pointer to scan event indication param
 *
 * This callback execute in atomic context and must not invoke any
 * blocking calls.
 *
 * Return: none
 */
static void
wlan_hdd_cfg80211_extscan_scan_progress_event(void *ctx,
					      tpSirExtScanOnScanEventIndParams
					      pData)
{
	hdd_context_t *pHddCtx = (hdd_context_t *) ctx;
	struct sk_buff *skb = NULL;
	int flags = cds_get_gfp_flags();

	ENTER();

	if (wlan_hdd_validate_context(pHddCtx))
		return;
	if (!pData) {
		hddLog(LOGE, FL("pData is null"));
		return;
	}

	skb = cfg80211_vendor_event_alloc(
			pHddCtx->wiphy,
			NULL,
			EXTSCAN_EVENT_BUF_SIZE + NLMSG_HDRLEN,
			QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SCAN_EVENT_INDEX,
			flags);

	if (!skb) {
		hddLog(LOGE, FL("cfg80211_vendor_event_alloc failed"));
		return;
	}
	hddLog(LOG1, "Req Id %u Scan event type %u Scan event status %u",
		pData->requestId, pData->scanEventType, pData->status);

	if (nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_REQUEST_ID,
			pData->requestId) ||
	    nla_put_u8(skb,
		       QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_EVENT_TYPE,
		       pData->scanEventType) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_EVENT_STATUS,
			pData->status)) {
		hddLog(LOGE, FL("nla put fail"));
		goto nla_put_failure;
	}

	cfg80211_vendor_event(skb, flags);
	EXIT();
	return;

nla_put_failure:
	kfree_skb(skb);
	return;
}

/**
 * wlan_hdd_cfg80211_extscan_epno_match_found() - pno match found
 * @hddctx: HDD context
 * @data: matched network data
 *
 * This function reads the matched network data and fills NL vendor attributes
 * and send it to upper layer.
 * This callback execute in atomic context and must not invoke any
 * blocking calls.
 *
 * Return: 0 on success, error number otherwise
 */
static void
wlan_hdd_cfg80211_extscan_epno_match_found(void *ctx,
					struct pno_match_found *data)
{
	hdd_context_t *pHddCtx  = (hdd_context_t *)ctx;
	struct sk_buff *skb     = NULL;
	uint32_t len, i;
	int flags = cds_get_gfp_flags();

	ENTER();

	if (wlan_hdd_validate_context(pHddCtx))
		return;
	if (!data) {
		hddLog(LOGE, FL("data is null"));
		return;
	}

	/*
	 * If the number of match found APs including IE data exceeds NL 4K size
	 * limitation, drop that beacon/probe rsp frame.
	 */
	len = sizeof(*data) +
			(data->num_results + sizeof(tSirWifiScanResult));
	for (i = 0; i < data->num_results; i++)
		len += data->ap[i].ieLength;

	if (len >= EXTSCAN_EVENT_BUF_SIZE) {
		hddLog(LOGE, FL("Frame exceeded NL size limitation, drop it!"));
		return;
	}

	skb = cfg80211_vendor_event_alloc(pHddCtx->wiphy,
		  NULL,
		  EXTSCAN_EVENT_BUF_SIZE + NLMSG_HDRLEN,
		QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_NETWORK_FOUND_INDEX,
		  flags);

	if (!skb) {
		hddLog(LOGE, FL("cfg80211_vendor_event_alloc failed"));
		return;
	}

	hddLog(LOG1, "Req Id %u More Data %u num_results %d",
		data->request_id, data->more_data, data->num_results);
	for (i = 0; i < data->num_results; i++) {
		data->ap[i].channel = cds_chan_to_freq(data->ap[i].channel);
		hddLog(LOG1, "AP Info: Timestamp %llu) Ssid: %s "
					"Bssid (" MAC_ADDRESS_STR ") "
					"Channel %u "
					"Rssi %d "
					"RTT %u "
					"RTT_SD %u "
					"Bcn Period %d "
					"Capability 0x%X "
					"IE Length %d",
					data->ap[i].ts,
					data->ap[i].ssid,
					MAC_ADDR_ARRAY(data->ap[i].bssid.bytes),
					data->ap[i].channel,
					data->ap[i].rssi,
					data->ap[i].rtt,
					data->ap[i].rtt_sd,
					data->ap[i].beaconPeriod,
					data->ap[i].capability,
					data->ap[i].ieLength);
	}

	if (nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_REQUEST_ID,
		data->request_id) ||
	    nla_put_u32(skb,
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_NUM_RESULTS_AVAILABLE,
		data->num_results) ||
	    nla_put_u8(skb,
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_MORE_DATA,
		data->more_data)) {
		hddLog(LOGE, FL("nla put fail"));
		goto fail;
	}

	if (data->num_results) {
		struct nlattr *nla_aps;
		nla_aps = nla_nest_start(skb,
			QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_LIST);
		if (!nla_aps)
			goto fail;

		for (i = 0; i < data->num_results; i++) {
			if (hdd_extscan_nl_fill_bss(skb, &data->ap[i], i))
				goto fail;
		}
		nla_nest_end(skb, nla_aps);
	}

	cfg80211_vendor_event(skb, flags);
	return;

fail:
	kfree_skb(skb);
	return;
}

/**
 * wlan_hdd_cfg80211_passpoint_match_found() - passpoint match found
 * @hddctx: HDD context
 * @data: matched network data
 *
 * This function reads the match network %data and fill in the skb with
 * NL attributes and send up the NL event
 * This callback execute in atomic context and must not invoke any
 * blocking calls.
 *
 * Return: none
 */
static void
wlan_hdd_cfg80211_passpoint_match_found(void *ctx,
					struct wifi_passpoint_match *data)
{
	hdd_context_t *pHddCtx  = ctx;
	struct sk_buff *skb     = NULL;
	uint32_t len, i, num_matches = 1, more_data = 0;
	struct nlattr *nla_aps, *nla_bss;
	int flags = cds_get_gfp_flags();

	ENTER();

	if (wlan_hdd_validate_context(pHddCtx))
		return;
	if (!data) {
		hddLog(LOGE, FL("data is null"));
		return;
	}

	len = sizeof(*data) + data->ap.ieLength + data->anqp_len;
	if (len >= EXTSCAN_EVENT_BUF_SIZE) {
		hddLog(LOGE, FL("Result exceeded NL size limitation, drop it"));
		return;
	}

	skb = cfg80211_vendor_event_alloc(pHddCtx->wiphy,
		  NULL,
		  EXTSCAN_EVENT_BUF_SIZE + NLMSG_HDRLEN,
		  QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_PASSPOINT_NETWORK_FOUND_INDEX,
		  flags);

	if (!skb) {
		hddLog(LOGE, FL("cfg80211_vendor_event_alloc failed"));
		return;
	}

	hddLog(LOG1, "Req Id %u Id %u ANQP length %u num_matches %u",
		data->request_id, data->id, data->anqp_len, num_matches);
	for (i = 0; i < num_matches; i++) {
		hddLog(LOG1, "AP Info: Timestamp %llu Ssid: %s "
					"Bssid (" MAC_ADDRESS_STR ") "
					"Channel %u "
					"Rssi %d "
					"RTT %u "
					"RTT_SD %u "
					"Bcn Period %d "
					"Capability 0x%X "
					"IE Length %d",
					data->ap.ts,
					data->ap.ssid,
					MAC_ADDR_ARRAY(data->ap.bssid.bytes),
					data->ap.channel,
					data->ap.rssi,
					data->ap.rtt,
					data->ap.rtt_sd,
					data->ap.beaconPeriod,
					data->ap.capability,
					data->ap.ieLength);
	}

	if (nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_REQUEST_ID,
		data->request_id) ||
	    nla_put_u32(skb,
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_PNO_RESULTS_PASSPOINT_NETWORK_FOUND_NUM_MATCHES,
		num_matches) ||
	    nla_put_u8(skb,
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_MORE_DATA,
		more_data)) {
		hddLog(LOGE, FL("nla put fail"));
		goto fail;
	}

	nla_aps = nla_nest_start(skb,
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_PNO_RESULTS_PASSPOINT_MATCH_RESULT_LIST);
	if (!nla_aps)
		goto fail;

	for (i = 0; i < num_matches; i++) {
		struct nlattr *nla_ap;

		nla_ap = nla_nest_start(skb, i);
		if (!nla_ap)
			goto fail;

		if (nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_EXTSCAN_PNO_RESULTS_PASSPOINT_MATCH_ID,
			data->id) ||
		    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_EXTSCAN_PNO_RESULTS_PASSPOINT_MATCH_ANQP_LEN,
			data->anqp_len)) {
			goto fail;
		}

		if (data->anqp_len)
			if (nla_put(skb,
				QCA_WLAN_VENDOR_ATTR_EXTSCAN_PNO_RESULTS_PASSPOINT_MATCH_ANQP,
				data->anqp_len, data->anqp))
				goto fail;

		nla_bss = nla_nest_start(skb,
				QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_LIST);
		if (!nla_bss)
			goto fail;

		if (hdd_extscan_nl_fill_bss(skb, &data->ap, 0))
			goto fail;

		nla_nest_end(skb, nla_bss);
		nla_nest_end(skb, nla_ap);
	}
	nla_nest_end(skb, nla_aps);

	cfg80211_vendor_event(skb, flags);
	return;

fail:
	kfree_skb(skb);
	return;
}

/**
 * wlan_hdd_cfg80211_extscan_hotlist_ssid_match_ind() -
 *	Handle an SSID hotlist match event
 * @ctx: HDD context registered with SME
 * @event: The SSID hotlist match event
 *
 * This function will take an SSID match event that was generated by
 * firmware and will convert it into a cfg80211 vendor event which is
 * sent to userspace.
 * This callback execute in atomic context and must not invoke any
 * blocking calls.
 *
 * Return: none
 */
static void
wlan_hdd_cfg80211_extscan_hotlist_ssid_match_ind(void *ctx,
						 tpSirWifiScanResultEvent event)
{
	hdd_context_t *hdd_ctx = ctx;
	struct sk_buff *skb;
	uint32_t i, index;
	int flags = cds_get_gfp_flags();

	ENTER();

	if (wlan_hdd_validate_context(hdd_ctx))
		return;
	if (!event) {
		hddLog(LOGE, FL("event is null"));
		return;
	}

	if (event->ap_found) {
		index = QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_SSID_FOUND_INDEX;
		hddLog(LOG1, "SSID hotlist found");
	} else {
		index = QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_SSID_LOST_INDEX;
		hddLog(LOG1, "SSID hotlist lost");
	}

	skb = cfg80211_vendor_event_alloc(hdd_ctx->wiphy,
		NULL,
		EXTSCAN_EVENT_BUF_SIZE + NLMSG_HDRLEN,
		index, flags);

	if (!skb) {
		hddLog(LOGE, FL("cfg80211_vendor_event_alloc failed"));
		return;
	}
	hddLog(LOG1, "Req Id %u, Num results %u, More Data %u",
	       event->requestId, event->numOfAps, event->moreData);

	for (i = 0; i < event->numOfAps; i++) {
		hddLog(LOG1, "[i=%d] Timestamp %llu "
		       "Ssid: %s "
		       "Bssid (" MAC_ADDRESS_STR ") "
		       "Channel %u "
		       "Rssi %d "
		       "RTT %u "
		       "RTT_SD %u",
		       i,
		       event->ap[i].ts,
		       event->ap[i].ssid,
		       MAC_ADDR_ARRAY(event->ap[i].bssid.bytes),
		       event->ap[i].channel,
		       event->ap[i].rssi,
		       event->ap[i].rtt,
		       event->ap[i].rtt_sd);
	}

	if (nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_REQUEST_ID,
			event->requestId) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_EXTSCAN_NUM_RESULTS_AVAILABLE,
			event->numOfAps)) {
		hddLog(LOGE, FL("put fail"));
		goto fail;
	}

	if (event->numOfAps) {
		struct nlattr *aps;
		aps = nla_nest_start(skb,
				     QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_LIST);
		if (!aps) {
			hddLog(LOGE, FL("nest fail"));
			goto fail;
		}

		for (i = 0; i < event->numOfAps; i++) {
			struct nlattr *ap;

			ap = nla_nest_start(skb, i);
			if (!ap) {
				hddLog(LOGE, FL("nest fail"));
				goto fail;
			}

			if (nla_put_u64(skb,
					QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_TIME_STAMP,
					event->ap[i].ts) ||
			    nla_put(skb,
				    QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_SSID,
				    sizeof(event->ap[i].ssid),
				    event->ap[i].ssid) ||
			    nla_put(skb,
				    QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_BSSID,
				    sizeof(event->ap[i].bssid),
				    event->ap[i].bssid.bytes) ||
			    nla_put_u32(skb,
					QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_CHANNEL,
					event->ap[i].channel) ||
			    nla_put_s32(skb,
					QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_RSSI,
					event->ap[i].rssi) ||
			    nla_put_u32(skb,
					QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_RTT,
					event->ap[i].rtt) ||
			    nla_put_u32(skb,
					QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_RTT_SD,
					event->ap[i].rtt_sd)) {
				hddLog(LOGE, FL("put fail"));
				goto fail;
			}
			nla_nest_end(skb, ap);
		}
		nla_nest_end(skb, aps);

		if (nla_put_u8(skb,
			       QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_MORE_DATA,
			       event->moreData)) {
			hddLog(LOGE, FL("put fail"));
			goto fail;
		}
	}

	cfg80211_vendor_event(skb, flags);
	return;

fail:
	kfree_skb(skb);
	return;
}

/**
 * wlan_hdd_cfg80211_extscan_generic_rsp() -
 *	Handle a generic ExtScan Response message
 * @ctx: HDD context registered with SME
 * @response: The ExtScan response from firmware
 *
 * This function will handle a generic ExtScan response message from
 * firmware and will communicate the result to the userspace thread
 * that is waiting for the response.
 *
 * Return: none
 */
static void
wlan_hdd_cfg80211_extscan_generic_rsp
	(void *ctx,
	 struct sir_extscan_generic_response *response)
{
	hdd_context_t *hdd_ctx = ctx;
	struct hdd_ext_scan_context *context;

	ENTER();

	if (wlan_hdd_validate_context(hdd_ctx) || !response) {
		hddLog(LOGE,
		       FL("HDD context is not valid or response(%p) is null"),
		       response);
		return;
	}

	hddLog(LOG1, FL("request %u status %u"),
	       response->request_id, response->status);

	context = &ext_scan_context;
	spin_lock(&context->context_lock);
	if (context->request_id == response->request_id) {
		context->response_status = response->status ? -EINVAL : 0;
		complete(&context->response_event);
	}
	spin_unlock(&context->context_lock);

	return;
}

/**
 * wlan_hdd_cfg80211_extscan_callback() - ext scan callback
 * @ctx: Pointer to hdd context
 * @evType: Event type
 * @pMag: Pointer to message
 *
 * Return: none
 */
void wlan_hdd_cfg80211_extscan_callback(void *ctx, const uint16_t evType,
					void *pMsg)
{
	hdd_context_t *pHddCtx = (hdd_context_t *) ctx;

	ENTER();

	if (wlan_hdd_validate_context(pHddCtx))
		return;

	hddLog(LOG1, FL("Rcvd Event %d"), evType);

	switch (evType) {
	case eSIR_EXTSCAN_CACHED_RESULTS_RSP:
		/* There is no need to send this response to upper layer
		   Just log the message */
		hddLog(LOG1,
		       FL("Rcvd eSIR_EXTSCAN_CACHED_RESULTS_RSP"));
		break;

	case eSIR_EXTSCAN_GET_CAPABILITIES_IND:
		wlan_hdd_cfg80211_extscan_get_capabilities_rsp(ctx,
			(struct ext_scan_capabilities_response *) pMsg);
		break;

	case eSIR_EXTSCAN_HOTLIST_MATCH_IND:
		wlan_hdd_cfg80211_extscan_hotlist_match_ind(ctx, pMsg);
		break;

	case eSIR_EXTSCAN_SIGNIFICANT_WIFI_CHANGE_RESULTS_IND:
		wlan_hdd_cfg80211_extscan_signif_wifi_change_results_ind(ctx,
					(tpSirWifiSignificantChangeEvent) pMsg);
		break;

	case eSIR_EXTSCAN_CACHED_RESULTS_IND:
		wlan_hdd_cfg80211_extscan_cached_results_ind(ctx, pMsg);
		break;

	case eSIR_EXTSCAN_SCAN_RES_AVAILABLE_IND:
		wlan_hdd_cfg80211_extscan_scan_res_available_event(ctx,
			   (tpSirExtScanResultsAvailableIndParams) pMsg);
		break;

	case eSIR_EXTSCAN_FULL_SCAN_RESULT_IND:
		wlan_hdd_cfg80211_extscan_full_scan_result_event(ctx,
					 (tpSirWifiFullScanResultEvent) pMsg);
		break;

	case eSIR_EPNO_NETWORK_FOUND_IND:
		wlan_hdd_cfg80211_extscan_epno_match_found(ctx,
					(struct pno_match_found *)pMsg);
		break;

	case eSIR_EXTSCAN_HOTLIST_SSID_MATCH_IND:
		wlan_hdd_cfg80211_extscan_hotlist_ssid_match_ind(ctx,
					(tpSirWifiScanResultEvent)pMsg);
		break;

	case eSIR_EXTSCAN_SCAN_PROGRESS_EVENT_IND:
		wlan_hdd_cfg80211_extscan_scan_progress_event(ctx,
			      (tpSirExtScanOnScanEventIndParams) pMsg);
		break;

	case eSIR_PASSPOINT_NETWORK_FOUND_IND:
		wlan_hdd_cfg80211_passpoint_match_found(ctx,
					(struct wifi_passpoint_match *) pMsg);
		break;

	case eSIR_EXTSCAN_START_RSP:
	case eSIR_EXTSCAN_STOP_RSP:
	case eSIR_EXTSCAN_SET_BSSID_HOTLIST_RSP:
	case eSIR_EXTSCAN_RESET_BSSID_HOTLIST_RSP:
	case eSIR_EXTSCAN_SET_SIGNIFICANT_WIFI_CHANGE_RSP:
	case eSIR_EXTSCAN_RESET_SIGNIFICANT_WIFI_CHANGE_RSP:
	case eSIR_EXTSCAN_SET_SSID_HOTLIST_RSP:
	case eSIR_EXTSCAN_RESET_SSID_HOTLIST_RSP:
		wlan_hdd_cfg80211_extscan_generic_rsp(ctx, pMsg);
		break;

	default:
		hddLog(LOGE, FL("Unknown event type %u"), evType);
		break;
	}
	EXIT();
}

/*
 * define short names for the global vendor params
 * used by wlan_hdd_send_ext_scan_capability()
 */
#define PARAM_REQUEST_ID \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_REQUEST_ID
#define PARAM_STATUS \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_STATUS
#define MAX_SCAN_CACHE_SIZE \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_SCAN_CACHE_SIZE
#define MAX_SCAN_BUCKETS \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_SCAN_BUCKETS
#define MAX_AP_CACHE_PER_SCAN \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_AP_CACHE_PER_SCAN
#define MAX_RSSI_SAMPLE_SIZE \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_RSSI_SAMPLE_SIZE
#define MAX_SCAN_RPT_THRHOLD \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_SCAN_REPORTING_THRESHOLD
#define MAX_HOTLIST_BSSIDS \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_HOTLIST_BSSIDS
#define MAX_SIGNIFICANT_WIFI_CHANGE_APS \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_SIGNIFICANT_WIFI_CHANGE_APS
#define MAX_BSSID_HISTORY_ENTRIES \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_BSSID_HISTORY_ENTRIES
#define MAX_HOTLIST_SSIDS \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_HOTLIST_SSIDS
#define MAX_NUM_EPNO_NETS \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_NUM_EPNO_NETS
#define MAX_NUM_EPNO_NETS_BY_SSID \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_NUM_EPNO_NETS_BY_SSID
#define MAX_NUM_WHITELISTED_SSID \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_NUM_WHITELISTED_SSID

/**
 * wlan_hdd_send_ext_scan_capability - send ext scan capability to user space
 * @hdd_ctx: Pointer to hdd context
 *
 * Return: 0 for success, non-zero for failure
 */
static int wlan_hdd_send_ext_scan_capability(hdd_context_t *hdd_ctx)
{
	int ret;
	struct sk_buff *skb;
	struct ext_scan_capabilities_response *data;
	uint32_t nl_buf_len;

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	data = &(ext_scan_context.capability_response);

	nl_buf_len = NLMSG_HDRLEN;
	nl_buf_len += (sizeof(data->requestId) + NLA_HDRLEN) +
	(sizeof(data->status) + NLA_HDRLEN) +
	(sizeof(data->max_scan_cache_size) + NLA_HDRLEN) +
	(sizeof(data->max_scan_buckets) + NLA_HDRLEN) +
	(sizeof(data->max_ap_cache_per_scan) + NLA_HDRLEN) +
	(sizeof(data->max_rssi_sample_size) + NLA_HDRLEN) +
	(sizeof(data->max_scan_reporting_threshold) + NLA_HDRLEN) +
	(sizeof(data->max_hotlist_bssids) + NLA_HDRLEN) +
	(sizeof(data->max_significant_wifi_change_aps) + NLA_HDRLEN) +
	(sizeof(data->max_bssid_history_entries) + NLA_HDRLEN) +
	(sizeof(data->max_hotlist_ssids) + NLA_HDRLEN) +
	(sizeof(data->max_number_epno_networks) + NLA_HDRLEN) +
	(sizeof(data->max_number_epno_networks_by_ssid) + NLA_HDRLEN) +
	(sizeof(data->max_number_of_white_listed_ssid) + NLA_HDRLEN);

	skb = cfg80211_vendor_cmd_alloc_reply_skb(hdd_ctx->wiphy, nl_buf_len);

	if (!skb) {
		hddLog(LOGE, FL("cfg80211_vendor_cmd_alloc_reply_skb failed"));
		return -ENOMEM;
	}


	hddLog(LOG1, "Req Id %u", data->requestId);
	hddLog(LOG1, "Status %u", data->status);
	hddLog(LOG1, "Scan cache size %u",
	       data->max_scan_cache_size);
	hddLog(LOG1, "Scan buckets %u", data->max_scan_buckets);
	hddLog(LOG1, "Max AP per scan %u",
	       data->max_ap_cache_per_scan);
	hddLog(LOG1, "max_rssi_sample_size %u",
	       data->max_rssi_sample_size);
	hddLog(LOG1, "max_scan_reporting_threshold %u",
	       data->max_scan_reporting_threshold);
	hddLog(LOG1, "max_hotlist_bssids %u",
	       data->max_hotlist_bssids);
	hddLog(LOG1, "max_significant_wifi_change_aps %u",
	       data->max_significant_wifi_change_aps);
	hddLog(LOG1, "max_bssid_history_entries %u",
	       data->max_bssid_history_entries);
	hddLog(LOG1, "max_hotlist_ssids %u", data->max_hotlist_ssids);
	hddLog(LOG1, "max_number_epno_networks %u",
					data->max_number_epno_networks);
	hddLog(LOG1, "max_number_epno_networks_by_ssid %u",
					data->max_number_epno_networks_by_ssid);
	hddLog(LOG1, "max_number_of_white_listed_ssid %u",
					data->max_number_of_white_listed_ssid);

	if (nla_put_u32(skb, PARAM_REQUEST_ID, data->requestId) ||
	    nla_put_u32(skb, PARAM_STATUS, data->status) ||
	    nla_put_u32(skb, MAX_SCAN_CACHE_SIZE, data->max_scan_cache_size) ||
	    nla_put_u32(skb, MAX_SCAN_BUCKETS, data->max_scan_buckets) ||
	    nla_put_u32(skb, MAX_AP_CACHE_PER_SCAN,
			data->max_ap_cache_per_scan) ||
	    nla_put_u32(skb, MAX_RSSI_SAMPLE_SIZE,
			data->max_rssi_sample_size) ||
	    nla_put_u32(skb, MAX_SCAN_RPT_THRHOLD,
			data->max_scan_reporting_threshold) ||
	    nla_put_u32(skb, MAX_HOTLIST_BSSIDS, data->max_hotlist_bssids) ||
	    nla_put_u32(skb, MAX_SIGNIFICANT_WIFI_CHANGE_APS,
			data->max_significant_wifi_change_aps) ||
	    nla_put_u32(skb, MAX_BSSID_HISTORY_ENTRIES,
			data->max_bssid_history_entries) ||
	    nla_put_u32(skb, MAX_HOTLIST_SSIDS,	data->max_hotlist_ssids) ||
	    nla_put_u32(skb, MAX_NUM_EPNO_NETS,
			data->max_number_epno_networks) ||
	    nla_put_u32(skb, MAX_NUM_EPNO_NETS_BY_SSID,
			data->max_number_epno_networks_by_ssid) ||
	    nla_put_u32(skb, MAX_NUM_WHITELISTED_SSID,
		data->max_number_of_white_listed_ssid)) {
			hddLog(LOGE, FL("nla put fail"));
			goto nla_put_failure;
	}

	cfg80211_vendor_cmd_reply(skb);
	return 0;

nla_put_failure:
	kfree_skb(skb);
	return -EINVAL;
}
/*
 * done with short names for the global vendor params
 * used by wlan_hdd_send_ext_scan_capability()
 */
#undef PARAM_REQUEST_ID
#undef PARAM_STATUS
#undef MAX_SCAN_CACHE_SIZE
#undef MAX_SCAN_BUCKETS
#undef MAX_AP_CACHE_PER_SCAN
#undef MAX_RSSI_SAMPLE_SIZE
#undef MAX_SCAN_RPT_THRHOLD
#undef MAX_HOTLIST_BSSIDS
#undef MAX_SIGNIFICANT_WIFI_CHANGE_APS
#undef MAX_BSSID_HISTORY_ENTRIES
#undef MAX_HOTLIST_SSIDS
#undef MAX_NUM_EPNO_NETS
#undef MAX_NUM_EPNO_NETS_BY_SSID
#undef MAX_NUM_WHITELISTED_SSID

/**
 * __wlan_hdd_cfg80211_extscan_get_capabilities() - get ext scan capabilities
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: none
 */
static int __wlan_hdd_cfg80211_extscan_get_capabilities(struct wiphy *wiphy,
					       struct wireless_dev *wdev,
					       const void *data, int data_len)
{
	int ret;
	unsigned long rc;
	struct hdd_ext_scan_context *context;
	tpSirGetExtScanCapabilitiesReqParams pReqMsg = NULL;
	struct net_device *dev = wdev->netdev;
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *pHddCtx = wiphy_priv(wiphy);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_MAX +
			  1];
	QDF_STATUS status;

	ENTER();

	if (CDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	ret = wlan_hdd_validate_context(pHddCtx);
	if (0 != ret)
		return -EINVAL;

	if (nla_parse(tb, QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_MAX,
		      data, data_len, wlan_hdd_extscan_config_policy)) {
		hddLog(LOGE, FL("Invalid ATTR"));
		return -EINVAL;
	}

	pReqMsg = cdf_mem_malloc(sizeof(*pReqMsg));
	if (!pReqMsg) {
		hddLog(LOGE, FL("cdf_mem_malloc failed"));
		return -ENOMEM;
	}

	/* Parse and fetch request Id */
	if (!tb[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_REQUEST_ID]) {
		hddLog(LOGE, FL("attr request id failed"));
		goto fail;
	}

	pReqMsg->requestId =
		nla_get_u32(tb
		 [QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_REQUEST_ID]);
	pReqMsg->sessionId = pAdapter->sessionId;
	hddLog(LOG1, FL("Req Id %d Session Id %d"),
		pReqMsg->requestId, pReqMsg->sessionId);

	context = &ext_scan_context;
	spin_lock(&context->context_lock);
	context->request_id = pReqMsg->requestId;
	INIT_COMPLETION(context->response_event);
	spin_unlock(&context->context_lock);

	status = sme_ext_scan_get_capabilities(pHddCtx->hHal, pReqMsg);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hddLog(LOGE, FL("sme_ext_scan_get_capabilities failed(err=%d)"),
			status);
		goto fail;
	}

	rc = wait_for_completion_timeout(&context->response_event,
		msecs_to_jiffies(WLAN_WAIT_TIME_EXTSCAN));
	if (!rc) {
		hddLog(LOGE, FL("Target response timed out"));
		return -ETIMEDOUT;
	}

	ret = wlan_hdd_send_ext_scan_capability(pHddCtx);
	if (ret)
		hddLog(LOGE, FL("Failed to send ext scan capability to user space"));
	EXIT();
	return ret;
fail:
	cdf_mem_free(pReqMsg);
	return -EINVAL;
}

/**
 * wlan_hdd_cfg80211_extscan_get_capabilities() - get ext scan capabilities
 * @wiphy: Pointer to wiphy
 * @wdev: Pointer to wdev
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: 0 for success, non-zero for failure
 */
int wlan_hdd_cfg80211_extscan_get_capabilities(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data, int data_len)
{
	int ret = 0;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_extscan_get_capabilities(wiphy, wdev, data,
		data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/*
 * define short names for the global vendor params
 * used by wlan_hdd_cfg80211_extscan_get_cached_results()
 */
#define PARAM_MAX \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_MAX
#define PARAM_REQUEST_ID \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_REQUEST_ID
#define PARAM_FLUSH \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_GET_CACHED_SCAN_RESULTS_CONFIG_PARAM_FLUSH
/**
 * __wlan_hdd_cfg80211_extscan_get_cached_results() - extscan get cached results
 * @wiphy: wiphy pointer
 * @wdev: pointer to struct wireless_dev
 * @data: pointer to incoming NL vendor data
 * @data_len: length of @data
 *
 * This function parses the incoming NL vendor command data attributes and
 * invokes the SME Api and blocks on a completion variable.
 * Each WMI event with cached scan results data chunk results in
 * function call wlan_hdd_cfg80211_extscan_cached_results_ind and each
 * data chunk is sent up the layer in cfg80211_vendor_cmd_alloc_reply_skb.
 *
 * If timeout happens before receiving all of the data, this function sets
 * a context variable @ignore_cached_results to %true, all of the next data
 * chunks are checked against this variable and dropped.
 *
 * Return: 0 on success; error number otherwise.
 */
static int __wlan_hdd_cfg80211_extscan_get_cached_results(struct wiphy *wiphy,
						 struct wireless_dev
						 *wdev, const void *data,
						 int data_len)
{
	tpSirExtScanGetCachedResultsReqParams pReqMsg = NULL;
	struct net_device *dev = wdev->netdev;
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *pHddCtx = wiphy_priv(wiphy);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_MAX +
			  1];
	struct hdd_ext_scan_context *context;
	QDF_STATUS status;
	int retval = 0;
	unsigned long rc;

	ENTER();

	if (CDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	retval = wlan_hdd_validate_context(pHddCtx);
	if (0 != retval)
		return -EINVAL;

	if (nla_parse(tb, PARAM_MAX, data, data_len,
		wlan_hdd_extscan_config_policy)) {
		hddLog(LOGE, FL("Invalid ATTR"));
		return -EINVAL;
	}

	pReqMsg = cdf_mem_malloc(sizeof(*pReqMsg));
	if (!pReqMsg) {
		hddLog(LOGE, FL("cdf_mem_malloc failed"));
		return -ENOMEM;
	}

	/* Parse and fetch request Id */
	if (!tb[PARAM_REQUEST_ID]) {
		hddLog(LOGE, FL("attr request id failed"));
		goto fail;
	}

	pReqMsg->requestId = nla_get_u32(tb[PARAM_REQUEST_ID]);
	pReqMsg->sessionId = pAdapter->sessionId;
	hddLog(LOG1, FL("Req Id %d Session Id %d"),
		pReqMsg->requestId, pReqMsg->sessionId);

	/* Parse and fetch flush parameter */
	if (!tb[PARAM_FLUSH]) {
		hddLog(LOGE, FL("attr flush failed"));
		goto fail;
	}
	pReqMsg->flush = nla_get_u8(tb[PARAM_FLUSH]);
	hddLog(LOG1, FL("Flush %d"), pReqMsg->flush);

	context = &ext_scan_context;
	spin_lock(&context->context_lock);
	context->request_id = pReqMsg->requestId;
	context->ignore_cached_results = false;
	INIT_COMPLETION(context->response_event);
	spin_unlock(&context->context_lock);

	status = sme_get_cached_results(pHddCtx->hHal, pReqMsg);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hddLog(LOGE,
		       FL("sme_get_cached_results failed(err=%d)"), status);
		goto fail;
	}

	rc = wait_for_completion_timeout(&context->response_event,
			msecs_to_jiffies(WLAN_WAIT_TIME_EXTSCAN));
	if (!rc) {
		hddLog(LOGE, FL("Target response timed out"));
		retval = -ETIMEDOUT;
		spin_lock(&context->context_lock);
		context->ignore_cached_results = true;
		spin_unlock(&context->context_lock);
	} else {
		spin_lock(&context->context_lock);
		retval = context->response_status;
		spin_unlock(&context->context_lock);
	}
	EXIT();
	return retval;

fail:
	cdf_mem_free(pReqMsg);
	return -EINVAL;
}
/*
 * done with short names for the global vendor params
 * used by wlan_hdd_cfg80211_extscan_get_cached_results()
 */
#undef PARAM_MAX
#undef PARAM_REQUEST_ID
#undef PARAM_FLUSH

/**
 * wlan_hdd_cfg80211_extscan_get_cached_results() - extscan get cached results
 * @wiphy: wiphy pointer
 * @wdev: pointer to struct wireless_dev
 * @data: pointer to incoming NL vendor data
 * @data_len: length of @data
 *
 * This function parses the incoming NL vendor command data attributes and
 * invokes the SME Api and blocks on a completion variable.
 * Each WMI event with cached scan results data chunk results in
 * function call wlan_hdd_cfg80211_extscan_cached_results_ind and each
 * data chunk is sent up the layer in cfg80211_vendor_cmd_alloc_reply_skb.
 *
 * If timeout happens before receiving all of the data, this function sets
 * a context variable @ignore_cached_results to %true, all of the next data
 * chunks are checked against this variable and dropped.
 *
 * Return: 0 on success; error number otherwise.
 */
int wlan_hdd_cfg80211_extscan_get_cached_results(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					const void *data, int data_len)
{
	int ret = 0;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_extscan_get_cached_results(wiphy, wdev, data,
								data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __wlan_hdd_cfg80211_extscan_set_bssid_hotlist() - set bssid hot list
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: none
 */
static int
__wlan_hdd_cfg80211_extscan_set_bssid_hotlist(struct wiphy *wiphy,
						struct wireless_dev
						*wdev, const void *data,
						int data_len)
{
	tpSirExtScanSetBssidHotListReqParams pReqMsg = NULL;
	struct net_device *dev = wdev->netdev;
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *pHddCtx = wiphy_priv(wiphy);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_MAX +
			  1];
	struct nlattr *tb2[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_MAX
			   + 1];
	struct nlattr *apTh;
	struct hdd_ext_scan_context *context;
	uint32_t request_id;
	QDF_STATUS status;
	uint8_t i;
	int rem, retval;
	unsigned long rc;

	ENTER();

	if (CDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	retval = wlan_hdd_validate_context(pHddCtx);
	if (0 != retval)
		return -EINVAL;

	if (nla_parse(tb, QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_MAX,
		      data, data_len, wlan_hdd_extscan_config_policy)) {
		hddLog(LOGE, FL("Invalid ATTR"));
		return -EINVAL;
	}

	pReqMsg = cdf_mem_malloc(sizeof(*pReqMsg));
	if (!pReqMsg) {
		hddLog(LOGE, FL("cdf_mem_malloc failed"));
		return -ENOMEM;
	}

	/* Parse and fetch request Id */
	if (!tb[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_REQUEST_ID]) {
		hddLog(LOGE, FL("attr request id failed"));
		goto fail;
	}

	pReqMsg->requestId =
		nla_get_u32(tb
		 [QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_REQUEST_ID]);
	hddLog(LOG1, FL("Req Id %d"), pReqMsg->requestId);

	/* Parse and fetch number of APs */
	if (!tb[QCA_WLAN_VENDOR_ATTR_EXTSCAN_BSSID_HOTLIST_PARAMS_NUM_AP]) {
		hddLog(LOGE, FL("attr number of AP failed"));
		goto fail;
	}
	pReqMsg->numAp =
		nla_get_u32(tb
		    [QCA_WLAN_VENDOR_ATTR_EXTSCAN_BSSID_HOTLIST_PARAMS_NUM_AP]);
	pReqMsg->sessionId = pAdapter->sessionId;
	hddLog(LOG1, FL("Number of AP %d Session Id %d"),
		pReqMsg->numAp, pReqMsg->sessionId);

	/* Parse and fetch lost ap sample size */
	if (!tb[QCA_WLAN_VENDOR_ATTR_EXTSCAN_BSSID_HOTLIST_PARAMS_LOST_AP_SAMPLE_SIZE]) {
		hddLog(LOGE, FL("attr lost ap sample size failed"));
		goto fail;
	}

	pReqMsg->lost_ap_sample_size = nla_get_u32(
		tb[QCA_WLAN_VENDOR_ATTR_EXTSCAN_BSSID_HOTLIST_PARAMS_LOST_AP_SAMPLE_SIZE]);
	hddLog(LOG1, FL("Lost ap sample size %d"),
			pReqMsg->lost_ap_sample_size);

	i = 0;
	nla_for_each_nested(apTh,
			    tb[QCA_WLAN_VENDOR_ATTR_EXTSCAN_AP_THRESHOLD_PARAM],
			    rem) {
		if (nla_parse
		    (tb2, QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_MAX,
		    nla_data(apTh), nla_len(apTh),
		    wlan_hdd_extscan_config_policy)) {
			hddLog(LOGE, FL("nla_parse failed"));
			goto fail;
		}

		/* Parse and fetch MAC address */
		if (!tb2[QCA_WLAN_VENDOR_ATTR_EXTSCAN_AP_THRESHOLD_PARAM_BSSID]) {
			hddLog(LOGE, FL("attr mac address failed"));
			goto fail;
		}
		nla_memcpy(pReqMsg->ap[i].bssid.bytes,
			tb2
			[QCA_WLAN_VENDOR_ATTR_EXTSCAN_AP_THRESHOLD_PARAM_BSSID],
			   CDF_MAC_ADDR_SIZE);
		hddLog(LOG1, MAC_ADDRESS_STR,
		       MAC_ADDR_ARRAY(pReqMsg->ap[i].bssid.bytes));

		/* Parse and fetch low RSSI */
		if (!tb2
		    [QCA_WLAN_VENDOR_ATTR_EXTSCAN_AP_THRESHOLD_PARAM_RSSI_LOW]) {
			hddLog(LOGE, FL("attr low RSSI failed"));
			goto fail;
		}
		pReqMsg->ap[i].low =
			nla_get_s32(tb2
			    [QCA_WLAN_VENDOR_ATTR_EXTSCAN_AP_THRESHOLD_PARAM_RSSI_LOW]);
		hddLog(LOG1, FL("RSSI low %d"), pReqMsg->ap[i].low);

		/* Parse and fetch high RSSI */
		if (!tb2
		    [QCA_WLAN_VENDOR_ATTR_EXTSCAN_AP_THRESHOLD_PARAM_RSSI_HIGH]) {
			hddLog(LOGE, FL("attr high RSSI failed"));
			goto fail;
		}
		pReqMsg->ap[i].high =
			nla_get_s32(tb2
			    [QCA_WLAN_VENDOR_ATTR_EXTSCAN_AP_THRESHOLD_PARAM_RSSI_HIGH]);
		hddLog(LOG1, FL("RSSI High %d"), pReqMsg->ap[i].high);

		i++;
	}

	context = &ext_scan_context;
	spin_lock(&context->context_lock);
	INIT_COMPLETION(context->response_event);
	context->request_id = request_id = pReqMsg->requestId;
	spin_unlock(&context->context_lock);

	status = sme_set_bss_hotlist(pHddCtx->hHal, pReqMsg);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hddLog(LOGE, FL("sme_set_bss_hotlist failed(err=%d)"), status);
		goto fail;
	}

	/* request was sent -- wait for the response */
	rc = wait_for_completion_timeout
		(&context->response_event,
		 msecs_to_jiffies(WLAN_WAIT_TIME_EXTSCAN));

	if (!rc) {
		hddLog(LOGE, FL("sme_set_bss_hotlist timed out"));
		retval = -ETIMEDOUT;
	} else {
		spin_lock(&context->context_lock);
		if (context->request_id == request_id)
			retval = context->response_status;
		else
			retval = -EINVAL;
		spin_unlock(&context->context_lock);
	}
	EXIT();
	return retval;

fail:
	cdf_mem_free(pReqMsg);
	return -EINVAL;
}

/**
 * wlan_hdd_cfg80211_extscan_set_bssid_hotlist() - set ext scan bssid hotlist
 * @wiphy: Pointer to wiphy
 * @wdev: Pointer to wdev
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: 0 for success, non-zero for failure
 */
int wlan_hdd_cfg80211_extscan_set_bssid_hotlist(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					const void *data, int data_len)
{
	int ret = 0;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_extscan_set_bssid_hotlist(wiphy, wdev, data,
					data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}


/**
 * __wlan_hdd_cfg80211_extscan_set_significant_change () - set significant change
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: none
 */
static int
__wlan_hdd_cfg80211_extscan_set_significant_change(struct wiphy *wiphy,
						     struct wireless_dev
						     *wdev, const void *data,
						     int data_len)
{
	tpSirExtScanSetSigChangeReqParams pReqMsg = NULL;
	struct net_device *dev = wdev->netdev;
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *pHddCtx = wiphy_priv(wiphy);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_MAX +
			  1];
	struct nlattr *tb2[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_MAX
			   + 1];
	struct nlattr *apTh;
	struct hdd_ext_scan_context *context;
	uint32_t request_id;
	QDF_STATUS status;
	uint8_t i;
	int rem, retval;
	unsigned long rc;

	ENTER();

	if (CDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	retval = wlan_hdd_validate_context(pHddCtx);
	if (0 != retval)
		return -EINVAL;

	if (nla_parse(tb, QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_MAX,
		      data, data_len, wlan_hdd_extscan_config_policy)) {
		hddLog(LOGE, FL("Invalid ATTR"));
		return -EINVAL;
	}

	pReqMsg = cdf_mem_malloc(sizeof(*pReqMsg));
	if (!pReqMsg) {
		hddLog(LOGE, FL("cdf_mem_malloc failed"));
		return -ENOMEM;
	}

	/* Parse and fetch request Id */
	if (!tb[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_REQUEST_ID]) {
		hddLog(LOGE, FL("attr request id failed"));
		goto fail;
	}

	pReqMsg->requestId =
		nla_get_u32(tb
		 [QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_REQUEST_ID]);
	hddLog(LOG1, FL("Req Id %d"), pReqMsg->requestId);

	/* Parse and fetch RSSI sample size */
	if (!tb
	    [QCA_WLAN_VENDOR_ATTR_EXTSCAN_SIGNIFICANT_CHANGE_PARAMS_RSSI_SAMPLE_SIZE]) {
		hddLog(LOGE, FL("attr RSSI sample size failed"));
		goto fail;
	}
	pReqMsg->rssiSampleSize =
		nla_get_u32(tb
		    [QCA_WLAN_VENDOR_ATTR_EXTSCAN_SIGNIFICANT_CHANGE_PARAMS_RSSI_SAMPLE_SIZE]);
	hddLog(LOG1, FL("RSSI sample size %u"), pReqMsg->rssiSampleSize);

	/* Parse and fetch lost AP sample size */
	if (!tb
	    [QCA_WLAN_VENDOR_ATTR_EXTSCAN_SIGNIFICANT_CHANGE_PARAMS_LOST_AP_SAMPLE_SIZE]) {
		hddLog(LOGE, FL("attr lost AP sample size failed"));
		goto fail;
	}
	pReqMsg->lostApSampleSize =
		nla_get_u32(tb
		    [QCA_WLAN_VENDOR_ATTR_EXTSCAN_SIGNIFICANT_CHANGE_PARAMS_LOST_AP_SAMPLE_SIZE]);
	hddLog(LOG1, FL("Lost AP sample size %u"), pReqMsg->lostApSampleSize);

	/* Parse and fetch AP min breacing */
	if (!tb
	    [QCA_WLAN_VENDOR_ATTR_EXTSCAN_SIGNIFICANT_CHANGE_PARAMS_MIN_BREACHING]) {
		hddLog(LOGE, FL("attr AP min breaching"));
		goto fail;
	}
	pReqMsg->minBreaching =
		nla_get_u32(tb
		    [QCA_WLAN_VENDOR_ATTR_EXTSCAN_SIGNIFICANT_CHANGE_PARAMS_MIN_BREACHING]);
	hddLog(LOG1, FL("AP min breaching %u"), pReqMsg->minBreaching);

	/* Parse and fetch number of APs */
	if (!tb[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SIGNIFICANT_CHANGE_PARAMS_NUM_AP]) {
		hddLog(LOGE, FL("attr number of AP failed"));
		goto fail;
	}
	pReqMsg->numAp =
		nla_get_u32(tb
		    [QCA_WLAN_VENDOR_ATTR_EXTSCAN_SIGNIFICANT_CHANGE_PARAMS_NUM_AP]);
	pReqMsg->sessionId = pAdapter->sessionId;
	hddLog(LOG1, FL("Number of AP %d Session Id %d"),
		pReqMsg->numAp, pReqMsg->sessionId);

	i = 0;
	nla_for_each_nested(apTh,
			    tb[QCA_WLAN_VENDOR_ATTR_EXTSCAN_AP_THRESHOLD_PARAM],
			    rem) {
		if (nla_parse
		    (tb2, QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_MAX,
		    nla_data(apTh), nla_len(apTh),
		    wlan_hdd_extscan_config_policy)) {
			hddLog(LOGE, FL("nla_parse failed"));
			goto fail;
		}

		/* Parse and fetch MAC address */
		if (!tb2[QCA_WLAN_VENDOR_ATTR_EXTSCAN_AP_THRESHOLD_PARAM_BSSID]) {
			hddLog(LOGE, FL("attr mac address failed"));
			goto fail;
		}
		nla_memcpy(pReqMsg->ap[i].bssid.bytes,
			   tb2
			   [QCA_WLAN_VENDOR_ATTR_EXTSCAN_AP_THRESHOLD_PARAM_BSSID],
			   CDF_MAC_ADDR_SIZE);
		hddLog(LOG1, MAC_ADDRESS_STR,
		       MAC_ADDR_ARRAY(pReqMsg->ap[i].bssid.bytes));

		/* Parse and fetch low RSSI */
		if (!tb2
		    [QCA_WLAN_VENDOR_ATTR_EXTSCAN_AP_THRESHOLD_PARAM_RSSI_LOW]) {
			hddLog(LOGE, FL("attr low RSSI failed"));
			goto fail;
		}
		pReqMsg->ap[i].low =
			nla_get_s32(tb2
			    [QCA_WLAN_VENDOR_ATTR_EXTSCAN_AP_THRESHOLD_PARAM_RSSI_LOW]);
		hddLog(LOG1, FL("RSSI low %d"), pReqMsg->ap[i].low);

		/* Parse and fetch high RSSI */
		if (!tb2
		    [QCA_WLAN_VENDOR_ATTR_EXTSCAN_AP_THRESHOLD_PARAM_RSSI_HIGH]) {
			hddLog(LOGE, FL("attr high RSSI failed"));
			goto fail;
		}
		pReqMsg->ap[i].high =
			nla_get_s32(tb2
			    [QCA_WLAN_VENDOR_ATTR_EXTSCAN_AP_THRESHOLD_PARAM_RSSI_HIGH]);
		hddLog(LOG1, FL("RSSI High %d"), pReqMsg->ap[i].high);

		i++;
	}

	context = &ext_scan_context;
	spin_lock(&context->context_lock);
	INIT_COMPLETION(context->response_event);
	context->request_id = request_id = pReqMsg->requestId;
	spin_unlock(&context->context_lock);

	status = sme_set_significant_change(pHddCtx->hHal, pReqMsg);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hddLog(LOGE,
		       FL("sme_set_significant_change failed(err=%d)"), status);
		cdf_mem_free(pReqMsg);
		return -EINVAL;
	}

	/* request was sent -- wait for the response */
	rc = wait_for_completion_timeout(&context->response_event,
				 msecs_to_jiffies(WLAN_WAIT_TIME_EXTSCAN));

	if (!rc) {
		hddLog(LOGE, FL("sme_set_significant_change timed out"));
		retval = -ETIMEDOUT;
	} else {
		spin_lock(&context->context_lock);
		if (context->request_id == request_id)
			retval = context->response_status;
		else
			retval = -EINVAL;
		spin_unlock(&context->context_lock);
	}
	EXIT();
	return retval;

fail:
	cdf_mem_free(pReqMsg);
	return -EINVAL;
}

/**
 * wlan_hdd_cfg80211_extscan_set_significant_change() - set significant change
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: 0 on success, negative errno on failure
 */
int wlan_hdd_cfg80211_extscan_set_significant_change(struct wiphy *wiphy,
				struct wireless_dev *wdev,
				const void *data, int data_len)
{
	int ret = 0;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_extscan_set_significant_change(wiphy, wdev,
					data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * hdd_remove_dsrc_channels () - remove dsrc chanels
 * @wiphy: Pointer to wireless phy
 * @chan_list: channel list
 * @num_channels: number of channels
 *
 * Return: none
 */
void hdd_remove_dsrc_channels(struct wiphy *wiphy, uint32_t *chan_list,
				 uint8_t *num_channels)
{
	uint8_t num_chan_temp = 0;
	int i;

	for (i = 0; i < *num_channels; i++) {
		if (!cds_is_dsrc_channel(chan_list[i])) {
			chan_list[num_chan_temp] = chan_list[i];
			num_chan_temp++;
		}
	}

	*num_channels = num_chan_temp;
}

/**
 * hdd_remove_indoor_channels () - remove indoor channels
 * @wiphy: Pointer to wireless phy
 * @chan_list: channel list
 * @num_channels: number of channels
 *
 * Return: none
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
void hdd_remove_indoor_channels(struct wiphy *wiphy, uint32_t *chan_list,
				 uint8_t *num_channels)
{
	uint8_t num_chan_temp = 0;
	int i, j, k;

	for (i = 0; i < *num_channels; i++)
		for (j = 0; j < IEEE80211_NUM_BANDS; j++) {
			if (wiphy->bands[j] == NULL)
				continue;
			for (k = 0; k < wiphy->bands[j]->n_channels; k++) {
				if ((chan_list[i] ==
				     wiphy->bands[j]->channels[k].center_freq)
				    && (!(wiphy->bands[j]->channels[k].flags &
				       IEEE80211_CHAN_INDOOR_ONLY))
				) {
					chan_list[num_chan_temp] = chan_list[i];
					num_chan_temp++;
				}
			}
		}

	*num_channels = num_chan_temp;
}
#else
void hdd_remove_indoor_channels(struct wiphy *wiphy, uint32_t *chan_list,
				 uint8_t *num_channels)
{
	*num_channels = 0;
}
#endif

/**
 * __wlan_hdd_cfg80211_extscan_get_valid_channels () - get valid channels
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: none
 */
static int
__wlan_hdd_cfg80211_extscan_get_valid_channels(struct wiphy *wiphy,
						 struct wireless_dev
						 *wdev, const void *data,
						 int data_len)
{
	hdd_context_t *pHddCtx = wiphy_priv(wiphy);
	struct net_device *dev = wdev->netdev;
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	uint32_t chan_list[WNI_CFG_VALID_CHANNEL_LIST_LEN] = {0};
	uint8_t num_channels  = 0;
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_MAX +
			  1];
	uint32_t requestId, maxChannels;
	tWifiBand wifiBand;
	QDF_STATUS status;
	struct sk_buff *reply_skb;
	uint8_t i;
	int ret;

	ENTER();

	if (CDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	ret = wlan_hdd_validate_context(pHddCtx);
	if (0 != ret)
		return -EINVAL;

	if (nla_parse(tb, QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_MAX,
		      data, data_len, wlan_hdd_extscan_config_policy)) {
		hddLog(LOGE, FL("Invalid ATTR"));
		return -EINVAL;
	}

	/* Parse and fetch request Id */
	if (!tb[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_REQUEST_ID]) {
		hddLog(LOGE, FL("attr request id failed"));
		return -EINVAL;
	}
	requestId =
		nla_get_u32(tb
		 [QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_REQUEST_ID]);
	hddLog(LOG1, FL("Req Id %d"), requestId);

	/* Parse and fetch wifi band */
	if (!tb
	    [QCA_WLAN_VENDOR_ATTR_EXTSCAN_GET_VALID_CHANNELS_CONFIG_PARAM_WIFI_BAND]) {
		hddLog(LOGE, FL("attr wifi band failed"));
		return -EINVAL;
	}
	wifiBand =
		nla_get_u32(tb
		    [QCA_WLAN_VENDOR_ATTR_EXTSCAN_GET_VALID_CHANNELS_CONFIG_PARAM_WIFI_BAND]);
	hddLog(LOG1, FL("Wifi band %d"), wifiBand);

	if (!tb
	    [QCA_WLAN_VENDOR_ATTR_EXTSCAN_GET_VALID_CHANNELS_CONFIG_PARAM_MAX_CHANNELS]) {
		hddLog(LOGE, FL("attr max channels failed"));
		return -EINVAL;
	}
	maxChannels =
		nla_get_u32(tb
		    [QCA_WLAN_VENDOR_ATTR_EXTSCAN_GET_VALID_CHANNELS_CONFIG_PARAM_MAX_CHANNELS]);
	hddLog(LOG1, FL("Max channels %d"), maxChannels);
	status = sme_get_valid_channels_by_band((tHalHandle) (pHddCtx->hHal),
						wifiBand, chan_list,
						&num_channels);
	if (QDF_STATUS_SUCCESS != status) {
		hddLog(LOGE,
		       FL("sme_get_valid_channels_by_band failed (err=%d)"),
		       status);
		return -EINVAL;
	}

	num_channels = CDF_MIN(num_channels, maxChannels);

	hdd_remove_dsrc_channels(wiphy, chan_list, &num_channels);

	if ((WLAN_HDD_SOFTAP == pAdapter->device_mode) ||
	    !strncmp(hdd_get_fwpath(), "ap", 2))
		hdd_remove_indoor_channels(wiphy, chan_list, &num_channels);

	hddLog(LOG1, FL("Number of channels %d"), num_channels);
	for (i = 0; i < num_channels; i++)
		hddLog(LOG1, "Channel: %u ", chan_list[i]);

	reply_skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, sizeof(u32) +
							sizeof(u32) *
							num_channels +
							NLMSG_HDRLEN);

	if (reply_skb) {
		if (nla_put_u32(reply_skb,
			QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_NUM_CHANNELS,
			num_channels) ||
		    nla_put(reply_skb,
			QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CHANNELS,
			sizeof(u32) * num_channels, chan_list)) {
			hddLog(LOGE, FL("nla put fail"));
			kfree_skb(reply_skb);
			return -EINVAL;
		}
		ret = cfg80211_vendor_cmd_reply(reply_skb);
		EXIT();
		return ret;
	}

	hddLog(LOGE, FL("valid channels: buffer alloc fail"));
	return -EINVAL;
}

/**
 * wlan_hdd_cfg80211_extscan_get_valid_channels() - get ext scan valid channels
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: 0 on success, negative errno on failure
 */
int wlan_hdd_cfg80211_extscan_get_valid_channels(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					const void *data, int data_len)
{
	int ret = 0;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_extscan_get_valid_channels(wiphy, wdev, data,
			data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * hdd_extscan_update_dwell_time_limits() - update dwell times
 * @req_msg: Pointer to request message
 * @bkt_idx: Index of current bucket being processed
 * @active_min: minimum active dwell time
 * @active_max: maximum active dwell time
 * @passive_min: minimum passive dwell time
 * @passive_max: maximum passive dwell time
 *
 * Return: none
 */
static void hdd_extscan_update_dwell_time_limits(
			tpSirWifiScanCmdReqParams req_msg, uint32_t bkt_idx,
			uint32_t active_min, uint32_t active_max,
			uint32_t passive_min, uint32_t passive_max)
{
	/* update per-bucket dwell times */
	if (req_msg->buckets[bkt_idx].min_dwell_time_active >
			active_min) {
		req_msg->buckets[bkt_idx].min_dwell_time_active =
			active_min;
	}
	if (req_msg->buckets[bkt_idx].max_dwell_time_active <
			active_max) {
		req_msg->buckets[bkt_idx].max_dwell_time_active =
			active_max;
	}
	if (req_msg->buckets[bkt_idx].min_dwell_time_passive >
			passive_min) {
		req_msg->buckets[bkt_idx].min_dwell_time_passive =
			passive_min;
	}
	if (req_msg->buckets[bkt_idx].max_dwell_time_passive <
			passive_max) {
		req_msg->buckets[bkt_idx].max_dwell_time_passive =
			passive_max;
	}
	/* update dwell-time across all buckets */
	if (req_msg->min_dwell_time_active >
			req_msg->buckets[bkt_idx].min_dwell_time_active) {
		req_msg->min_dwell_time_active =
			req_msg->buckets[bkt_idx].min_dwell_time_active;
	}
	if (req_msg->max_dwell_time_active <
			req_msg->buckets[bkt_idx].max_dwell_time_active) {
		req_msg->max_dwell_time_active =
			req_msg->buckets[bkt_idx].max_dwell_time_active;
	}
	if (req_msg->min_dwell_time_passive >
			req_msg->buckets[bkt_idx].min_dwell_time_passive) {
		req_msg->min_dwell_time_passive =
			req_msg->buckets[bkt_idx].min_dwell_time_passive;
	}
	if (req_msg->max_dwell_time_passive >
			req_msg->buckets[bkt_idx].max_dwell_time_passive) {
		req_msg->max_dwell_time_passive =
			req_msg->buckets[bkt_idx].max_dwell_time_passive;
	}
}

/**
 * hdd_extscan_channel_max_reached() - channel max reached
 * @req: extscan request structure
 * @total_channels: total number of channels
 *
 * Return: true if total channels reached max, false otherwise
 */
static bool hdd_extscan_channel_max_reached(tSirWifiScanCmdReqParams *req,
					    uint8_t total_channels)
{
	if (total_channels == WLAN_EXTSCAN_MAX_CHANNELS) {
		hdd_warn(
		   "max #of channels %d reached, take only first %d bucket(s)",
		   total_channels, req->numBuckets);
		return true;
	}
	return false;
}

/**
 * hdd_extscan_start_fill_bucket_channel_spec() - fill bucket channel spec
 * @hdd_ctx: HDD global context
 * @req_msg: Pointer to request structure
 * @tb: pointer to NL attributes
 *
 * Return: 0 on success; error number otherwise
 */
static int hdd_extscan_start_fill_bucket_channel_spec(
			hdd_context_t *hdd_ctx,
			tpSirWifiScanCmdReqParams req_msg,
			struct nlattr **tb)
{
	struct nlattr *bucket[
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_MAX + 1];
	struct nlattr *channel[
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_MAX + 1];
	struct nlattr *buckets;
	struct nlattr *channels;
	int rem1, rem2;
	QDF_STATUS status;
	uint8_t bkt_index, j, num_channels, total_channels = 0;
	uint32_t chan_list[WNI_CFG_VALID_CHANNEL_LIST_LEN] = {0};

	uint32_t min_dwell_time_active_bucket =
		hdd_ctx->config->extscan_active_max_chn_time;
	uint32_t max_dwell_time_active_bucket =
		hdd_ctx->config->extscan_active_max_chn_time;
	uint32_t min_dwell_time_passive_bucket =
		hdd_ctx->config->extscan_passive_max_chn_time;
	uint32_t max_dwell_time_passive_bucket =
		hdd_ctx->config->extscan_passive_max_chn_time;

	bkt_index = 0;
	req_msg->min_dwell_time_active =
		req_msg->max_dwell_time_active =
			hdd_ctx->config->extscan_active_max_chn_time;

	req_msg->min_dwell_time_passive =
		req_msg->max_dwell_time_passive =
			hdd_ctx->config->extscan_passive_max_chn_time;
	req_msg->numBuckets = 0;

	nla_for_each_nested(buckets,
			tb[QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC], rem1) {
		if (nla_parse(bucket,
			QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_MAX,
			nla_data(buckets), nla_len(buckets), NULL)) {
			hddLog(LOGE, FL("nla_parse failed"));
			return -EINVAL;
		}

		/* Parse and fetch bucket spec */
		if (!bucket[QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_INDEX]) {
			hddLog(LOGE, FL("attr bucket index failed"));
			return -EINVAL;
		}
		req_msg->buckets[bkt_index].bucket = nla_get_u8(
			bucket[QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_INDEX]);
		hddLog(LOG1, FL("Bucket spec Index %d"),
				req_msg->buckets[bkt_index].bucket);

		/* Parse and fetch wifi band */
		if (!bucket[QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_BAND]) {
			hddLog(LOGE, FL("attr wifi band failed"));
			return -EINVAL;
		}
		req_msg->buckets[bkt_index].band = nla_get_u8(
			bucket[QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_BAND]);
		hddLog(LOG1, FL("Wifi band %d"),
			req_msg->buckets[bkt_index].band);

		/* Parse and fetch period */
		if (!bucket[QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_PERIOD]) {
			hddLog(LOGE, FL("attr period failed"));
			return -EINVAL;
		}
		req_msg->buckets[bkt_index].period = nla_get_u32(
		bucket[QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_PERIOD]);
		hddLog(LOG1, FL("period %d"),
			req_msg->buckets[bkt_index].period);

		/* Parse and fetch report events */
		if (!bucket[
			QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_REPORT_EVENTS]) {
			hddLog(LOGE, FL("attr report events failed"));
			return -EINVAL;
		}
		req_msg->buckets[bkt_index].reportEvents = nla_get_u8(
			bucket[
			QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_REPORT_EVENTS]);
		hddLog(LOG1, FL("report events %d"),
				req_msg->buckets[bkt_index].reportEvents);

		/* Parse and fetch max period */
		if (!bucket[QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_MAX_PERIOD]) {
			hddLog(LOGE, FL("attr max period failed"));
			return -EINVAL;
		}
		req_msg->buckets[bkt_index].max_period = nla_get_u32(
			bucket[QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_MAX_PERIOD]);
		hddLog(LOG1, FL("max period %u"),
			req_msg->buckets[bkt_index].max_period);

		/* Parse and fetch exponent */
		if (!bucket[QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_EXPONENT]) {
			hddLog(LOGE, FL("attr exponent failed"));
			return -EINVAL;
		}
		req_msg->buckets[bkt_index].exponent = nla_get_u32(
			bucket[QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_EXPONENT]);
		hddLog(LOG1, FL("exponent %u"),
			req_msg->buckets[bkt_index].exponent);

		/* Parse and fetch step count */
		if (!bucket[QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_STEP_COUNT]) {
			hddLog(LOGE, FL("attr step count failed"));
			return -EINVAL;
		}
		req_msg->buckets[bkt_index].step_count = nla_get_u32(
			bucket[QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_STEP_COUNT]);
		hddLog(LOG1, FL("Step count %u"),
			req_msg->buckets[bkt_index].step_count);

		/* start with known good values for bucket dwell times */
		req_msg->buckets[bkt_index].min_dwell_time_active =
		req_msg->buckets[bkt_index].max_dwell_time_active =
			hdd_ctx->config->extscan_active_max_chn_time;

		req_msg->buckets[bkt_index].min_dwell_time_passive =
		req_msg->buckets[bkt_index].max_dwell_time_passive =
			hdd_ctx->config->extscan_passive_max_chn_time;

		/* Framework shall pass the channel list if the input WiFi band
		 * is WIFI_BAND_UNSPECIFIED.
		 * If the input WiFi band is specified (any value other than
		 * WIFI_BAND_UNSPECIFIED) then driver populates the channel list
		 */
		if (req_msg->buckets[bkt_index].band != WIFI_BAND_UNSPECIFIED) {
			if (hdd_extscan_channel_max_reached(req_msg,
							    total_channels))
				return 0;

			num_channels = 0;
			hddLog(LOG1, "WiFi band is specified, driver to fill channel list");
			status = sme_get_valid_channels_by_band(hdd_ctx->hHal,
						req_msg->buckets[bkt_index].band,
						chan_list, &num_channels);
			if (!QDF_IS_STATUS_SUCCESS(status)) {
				hddLog(LOGE,
				       FL("sme_GetValidChannelsByBand failed (err=%d)"),
				       status);
				return -EINVAL;
			}
			hddLog(LOG1, FL("before trimming, num_channels: %d"),
				num_channels);

			req_msg->buckets[bkt_index].numChannels =
				CDF_MIN(num_channels,
					(WLAN_EXTSCAN_MAX_CHANNELS -
						total_channels));
			hdd_info("Adj Num channels/bucket: %d total_channels: %d",
				req_msg->buckets[bkt_index].numChannels,
				total_channels);
			total_channels +=
				req_msg->buckets[bkt_index].numChannels;

			for (j = 0; j < req_msg->buckets[bkt_index].numChannels;
				j++) {
				req_msg->buckets[bkt_index].channels[j].channel =
							chan_list[j];
				req_msg->buckets[bkt_index].channels[j].
							chnlClass = 0;
				if (CDS_IS_PASSIVE_OR_DISABLE_CH(
					cds_freq_to_chan(chan_list[j]))) {
					req_msg->buckets[bkt_index].channels[j].
								passive = 1;
					req_msg->buckets[bkt_index].channels[j].
					dwellTimeMs =
						hdd_ctx->config->
						extscan_passive_max_chn_time;
					/* reconfigure per-bucket dwell time */
					if (min_dwell_time_passive_bucket >
							req_msg->buckets[bkt_index].channels[j].dwellTimeMs) {
						min_dwell_time_passive_bucket =
							req_msg->buckets[bkt_index].channels[j].dwellTimeMs;
					}
					if (max_dwell_time_passive_bucket <
							req_msg->buckets[bkt_index].channels[j].dwellTimeMs) {
						max_dwell_time_passive_bucket =
							req_msg->buckets[bkt_index].channels[j].dwellTimeMs;
					}

				} else {
					req_msg->buckets[bkt_index].channels[j].
							passive = 0;
					req_msg->buckets[bkt_index].channels[j].
					dwellTimeMs =
						hdd_ctx->config->extscan_active_max_chn_time;
					/* reconfigure per-bucket dwell times */
					if (min_dwell_time_active_bucket >
							req_msg->buckets[bkt_index].channels[j].dwellTimeMs) {
						min_dwell_time_active_bucket =
							req_msg->buckets[bkt_index].channels[j].dwellTimeMs;
					}
					if (max_dwell_time_active_bucket <
							req_msg->buckets[bkt_index].channels[j].dwellTimeMs) {
						max_dwell_time_active_bucket =
							req_msg->buckets[bkt_index].channels[j].dwellTimeMs;
					}

				}

				hddLog(LOG1,
					"Channel: %u Passive: %u Dwell time: %u ms Class: %u",
					req_msg->buckets[bkt_index].channels[j].channel,
					req_msg->buckets[bkt_index].channels[j].passive,
					req_msg->buckets[bkt_index].channels[j].dwellTimeMs,
					req_msg->buckets[bkt_index].channels[j].chnlClass);
			}

			hdd_extscan_update_dwell_time_limits(
					req_msg, bkt_index,
					min_dwell_time_active_bucket,
					max_dwell_time_active_bucket,
					min_dwell_time_passive_bucket,
					max_dwell_time_passive_bucket);

			hddLog(LOG1, FL("bkt_index:%d actv_min:%d actv_max:%d pass_min:%d pass_max:%d"),
					bkt_index,
					req_msg->buckets[bkt_index].min_dwell_time_active,
					req_msg->buckets[bkt_index].max_dwell_time_active,
					req_msg->buckets[bkt_index].min_dwell_time_passive,
					req_msg->buckets[bkt_index].max_dwell_time_passive);

			bkt_index++;
			req_msg->numBuckets++;
			continue;
		}

		/* Parse and fetch number of channels */
		if (!bucket[
			QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_NUM_CHANNEL_SPECS]) {
			hddLog(LOGE, FL("attr num channels failed"));
			return -EINVAL;
		}
		req_msg->buckets[bkt_index].numChannels =
		nla_get_u32(bucket[
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_NUM_CHANNEL_SPECS]);
		hdd_info("before trimming: num channels %d",
			req_msg->buckets[bkt_index].numChannels);

		req_msg->buckets[bkt_index].numChannels =
			CDF_MIN(req_msg->buckets[bkt_index].numChannels,
				(WLAN_EXTSCAN_MAX_CHANNELS - total_channels));
		hdd_info("Num channels/bucket: %d total_channels: %d",
			req_msg->buckets[bkt_index].numChannels,
			total_channels);
		if (hdd_extscan_channel_max_reached(req_msg, total_channels))
			return 0;

		if (!bucket[QCA_WLAN_VENDOR_ATTR_EXTSCAN_CHANNEL_SPEC]) {
			hddLog(LOGE, FL("attr channel spec failed"));
			return -EINVAL;
		}

		j = 0;
		nla_for_each_nested(channels,
			bucket[QCA_WLAN_VENDOR_ATTR_EXTSCAN_CHANNEL_SPEC], rem2) {
			if ((j >= req_msg->buckets[bkt_index].numChannels) ||
			    hdd_extscan_channel_max_reached(req_msg,
							    total_channels))
				break;

			if (nla_parse(channel,
				QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_MAX,
				nla_data(channels), nla_len(channels),
				wlan_hdd_extscan_config_policy)) {
				hddLog(LOGE, FL("nla_parse failed"));
				return -EINVAL;
			}

			/* Parse and fetch channel */
			if (!channel[
				QCA_WLAN_VENDOR_ATTR_EXTSCAN_CHANNEL_SPEC_CHANNEL]) {
				hddLog(LOGE, FL("attr channel failed"));
				return -EINVAL;
			}
			req_msg->buckets[bkt_index].channels[j].channel =
				nla_get_u32(channel[
				QCA_WLAN_VENDOR_ATTR_EXTSCAN_CHANNEL_SPEC_CHANNEL]);
			hddLog(LOG1, FL("channel %u"),
				req_msg->buckets[bkt_index].channels[j].channel);

			/* Parse and fetch dwell time */
			if (!channel[
				QCA_WLAN_VENDOR_ATTR_EXTSCAN_CHANNEL_SPEC_DWELL_TIME]) {
				hddLog(LOGE, FL("attr dwelltime failed"));
				return -EINVAL;
			}
			req_msg->buckets[bkt_index].channels[j].dwellTimeMs =
				nla_get_u32(channel[
				QCA_WLAN_VENDOR_ATTR_EXTSCAN_CHANNEL_SPEC_DWELL_TIME]);

			/* Override dwell time if required */
			if (req_msg->buckets[bkt_index].channels[j].dwellTimeMs <
				hdd_ctx->config->extscan_active_min_chn_time ||
				req_msg->buckets[bkt_index].channels[j].dwellTimeMs >
				hdd_ctx->config->extscan_active_max_chn_time) {
				hddLog(LOG1, FL("WiFi band is unspecified, dwellTime:%d"),
						req_msg->buckets[bkt_index].channels[j].dwellTimeMs);

				if (CDS_IS_PASSIVE_OR_DISABLE_CH(
					cds_freq_to_chan(
						req_msg->buckets[bkt_index].channels[j].channel))) {
					req_msg->buckets[bkt_index].channels[j].dwellTimeMs =
						hdd_ctx->config->extscan_passive_max_chn_time;
				} else {
					req_msg->buckets[bkt_index].channels[j].dwellTimeMs =
						hdd_ctx->config->extscan_active_max_chn_time;
				}
			}

			hddLog(LOG1, FL("New Dwell time %u ms"),
				req_msg->buckets[bkt_index].channels[j].dwellTimeMs);

			if (CDS_IS_PASSIVE_OR_DISABLE_CH(
						cds_freq_to_chan(
						req_msg->buckets[bkt_index].channels[j].channel))) {
				if (min_dwell_time_passive_bucket >
						req_msg->buckets[bkt_index].channels[j].dwellTimeMs) {
					min_dwell_time_passive_bucket =
						req_msg->buckets[bkt_index].channels[j].dwellTimeMs;
				}
				if (max_dwell_time_passive_bucket <
						req_msg->buckets[bkt_index].channels[j].dwellTimeMs) {
					max_dwell_time_passive_bucket =
						req_msg->buckets[bkt_index].channels[j].dwellTimeMs;
				}
			} else {
				if (min_dwell_time_active_bucket >
						req_msg->buckets[bkt_index].channels[j].dwellTimeMs) {
					min_dwell_time_active_bucket =
						req_msg->buckets[bkt_index].channels[j].dwellTimeMs;
				}
				if (max_dwell_time_active_bucket <
						req_msg->buckets[bkt_index].channels[j].dwellTimeMs) {
					max_dwell_time_active_bucket =
						req_msg->buckets[bkt_index].channels[j].dwellTimeMs;
				}
			}

			/* Parse and fetch channel spec passive */
			if (!channel[
				QCA_WLAN_VENDOR_ATTR_EXTSCAN_CHANNEL_SPEC_PASSIVE]) {
				hddLog(LOGE,
					FL("attr channel spec passive failed"));
				return -EINVAL;
			}
			req_msg->buckets[bkt_index].channels[j].passive =
				nla_get_u8(channel[
				QCA_WLAN_VENDOR_ATTR_EXTSCAN_CHANNEL_SPEC_PASSIVE]);
			hddLog(LOG1, FL("Chnl spec passive %u"),
				req_msg->buckets[bkt_index].channels[j].passive);
			/* Override scan type if required */
			if (CDS_IS_PASSIVE_OR_DISABLE_CH(
				cds_freq_to_chan(
					req_msg->buckets[bkt_index].channels[j].channel))) {
				req_msg->buckets[bkt_index].channels[j].passive = true;
			} else {
				req_msg->buckets[bkt_index].channels[j].passive = false;
			}
			j++;
			total_channels++;
		}

		hdd_extscan_update_dwell_time_limits(
					req_msg, bkt_index,
					min_dwell_time_active_bucket,
					max_dwell_time_active_bucket,
					min_dwell_time_passive_bucket,
					max_dwell_time_passive_bucket);

		hddLog(LOG1, FL("bktIndex:%d actv_min:%d actv_max:%d pass_min:%d pass_max:%d"),
				bkt_index,
				req_msg->buckets[bkt_index].min_dwell_time_active,
				req_msg->buckets[bkt_index].max_dwell_time_active,
				req_msg->buckets[bkt_index].min_dwell_time_passive,
				req_msg->buckets[bkt_index].max_dwell_time_passive);

		bkt_index++;
		req_msg->numBuckets++;
	}

	hddLog(LOG1, FL("Global: actv_min:%d actv_max:%d pass_min:%d pass_max:%d"),
				req_msg->min_dwell_time_active,
				req_msg->max_dwell_time_active,
				req_msg->min_dwell_time_passive,
				req_msg->max_dwell_time_passive);

	return 0;
}

/*
 * hdd_extscan_map_usr_drv_config_flags() - map userspace to driver config flags
 * @config_flags - [input] configuration flags.
 *
 * This function maps user space received configuration flags to
 * driver representation.
 *
 * Return: configuration flags
 */
static uint32_t hdd_extscan_map_usr_drv_config_flags(uint32_t config_flags)
{
	uint32_t configuration_flags = 0;

	if (config_flags & EXTSCAN_LP_EXTENDED_BATCHING)
		configuration_flags |= EXTSCAN_LP_EXTENDED_BATCHING;

	return configuration_flags;
}

/*
 * define short names for the global vendor params
 * used by __wlan_hdd_cfg80211_extscan_start()
 */
#define PARAM_MAX \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_MAX
#define PARAM_REQUEST_ID \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_REQUEST_ID
#define PARAM_BASE_PERIOD \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SCAN_CMD_PARAMS_BASE_PERIOD
#define PARAM_MAX_AP_PER_SCAN \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SCAN_CMD_PARAMS_MAX_AP_PER_SCAN
#define PARAM_RPT_THRHLD_PERCENT \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SCAN_CMD_PARAMS_REPORT_THRESHOLD_PERCENT
#define PARAM_RPT_THRHLD_NUM_SCANS \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SCAN_CMD_PARAMS_REPORT_THRESHOLD_NUM_SCANS
#define PARAM_NUM_BUCKETS \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SCAN_CMD_PARAMS_NUM_BUCKETS
#define PARAM_CONFIG_FLAGS \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_CONFIGURATION_FLAGS

/**
 * __wlan_hdd_cfg80211_extscan_start() - ext scan start
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Length of @data
 *
 * Return: 0 on success; error number otherwise
 */
static int
__wlan_hdd_cfg80211_extscan_start(struct wiphy *wiphy,
				    struct wireless_dev *wdev,
				    const void *data,
				    int data_len)
{
	tpSirWifiScanCmdReqParams pReqMsg       = NULL;
	struct net_device *dev                  = wdev->netdev;
	hdd_adapter_t *pAdapter                 = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *pHddCtx                  = wiphy_priv(wiphy);
	struct nlattr *tb[PARAM_MAX + 1];
	struct hdd_ext_scan_context *context;
	uint32_t request_id, num_buckets;
	QDF_STATUS status;
	int retval;
	unsigned long rc;

	ENTER();

	if (CDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	retval = wlan_hdd_validate_context(pHddCtx);
	if (0 != retval)
		return -EINVAL;

	if (nla_parse(tb, PARAM_MAX, data, data_len,
		wlan_hdd_extscan_config_policy)) {
		hddLog(LOGE, FL("Invalid ATTR"));
		return -EINVAL;
	}

	pReqMsg = cdf_mem_malloc(sizeof(*pReqMsg));
	if (!pReqMsg) {
		hddLog(LOGE, FL("memory allocation failed"));
		return -ENOMEM;
	}

	/* Parse and fetch request Id */
	if (!tb[PARAM_REQUEST_ID]) {
		hddLog(LOGE, FL("attr request id failed"));
		goto fail;
	}

	pReqMsg->requestId = nla_get_u32(tb[PARAM_REQUEST_ID]);
	pReqMsg->sessionId = pAdapter->sessionId;
	hddLog(LOG1, FL("Req Id %d Session Id %d"),
			pReqMsg->requestId,
			pReqMsg->sessionId);

	/* Parse and fetch base period */
	if (!tb[PARAM_BASE_PERIOD]) {
		hddLog(LOGE, FL("attr base period failed"));
		goto fail;
	}
	pReqMsg->basePeriod = nla_get_u32(tb[PARAM_BASE_PERIOD]);
	hddLog(LOG1, FL("Base Period %d"),
				pReqMsg->basePeriod);

	/* Parse and fetch max AP per scan */
	if (!tb[PARAM_MAX_AP_PER_SCAN]) {
		hddLog(LOGE, FL("attr max_ap_per_scan failed"));
		goto fail;
	}
	pReqMsg->maxAPperScan = nla_get_u32(tb[PARAM_MAX_AP_PER_SCAN]);
	hddLog(LOG1, FL("Max AP per Scan %d"), pReqMsg->maxAPperScan);

	/* Parse and fetch report threshold percent */
	if (!tb[PARAM_RPT_THRHLD_PERCENT]) {
		hddLog(LOGE, FL("attr report_threshold percent failed"));
		goto fail;
	}
	pReqMsg->report_threshold_percent = nla_get_u8(tb[PARAM_RPT_THRHLD_PERCENT]);
	hddLog(LOG1, FL("Report Threshold percent %d"),
			pReqMsg->report_threshold_percent);

	/* Parse and fetch report threshold num scans */
	if (!tb[PARAM_RPT_THRHLD_NUM_SCANS]) {
		hddLog(LOGE, FL("attr report_threshold num scans failed"));
		goto fail;
	}
	pReqMsg->report_threshold_num_scans = nla_get_u8(tb[PARAM_RPT_THRHLD_NUM_SCANS]);
	hddLog(LOG1, FL("Report Threshold num scans %d"),
		pReqMsg->report_threshold_num_scans);

	/* Parse and fetch number of buckets */
	if (!tb[PARAM_NUM_BUCKETS]) {
		hddLog(LOGE, FL("attr number of buckets failed"));
		goto fail;
	}
	num_buckets = nla_get_u8(tb[PARAM_NUM_BUCKETS]);
	if (num_buckets > WLAN_EXTSCAN_MAX_BUCKETS) {
		hdd_warn("Exceeded MAX number of buckets: %d",
				WLAN_EXTSCAN_MAX_BUCKETS);
	}
	hdd_info("Input: Number of Buckets %d", num_buckets);

	/* This is optional attribute, if not present set it to 0 */
	if (!tb[PARAM_CONFIG_FLAGS])
		pReqMsg->configuration_flags = 0;
	else
		pReqMsg->configuration_flags =
			hdd_extscan_map_usr_drv_config_flags(
				nla_get_u32(tb[PARAM_CONFIG_FLAGS]));

	hddLog(LOG1, FL("Configuration flags: %u"),
				pReqMsg->configuration_flags);

	if (!tb[QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC]) {
		hddLog(LOGE, FL("attr bucket spec failed"));
		goto fail;
	}

	if (hdd_extscan_start_fill_bucket_channel_spec(pHddCtx, pReqMsg, tb))
		goto fail;

	context = &ext_scan_context;
	spin_lock(&context->context_lock);
	INIT_COMPLETION(context->response_event);
	context->request_id = request_id = pReqMsg->requestId;
	spin_unlock(&context->context_lock);

	status = sme_ext_scan_start(pHddCtx->hHal, pReqMsg);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hddLog(LOGE,
			FL("sme_ext_scan_start failed(err=%d)"), status);
		goto fail;
	}

	pHddCtx->ext_scan_start_since_boot = cdf_get_monotonic_boottime();
	hddLog(LOG1, FL("Timestamp since boot: %llu"),
			pHddCtx->ext_scan_start_since_boot);

	/* request was sent -- wait for the response */
	rc = wait_for_completion_timeout(&context->response_event,
				msecs_to_jiffies(WLAN_WAIT_TIME_EXTSCAN));

	if (!rc) {
		hddLog(LOGE, FL("sme_ext_scan_start timed out"));
		retval = -ETIMEDOUT;
	} else {
		spin_lock(&context->context_lock);
		if (context->request_id == request_id)
			retval = context->response_status;
		else
			retval = -EINVAL;
		spin_unlock(&context->context_lock);
	}
	EXIT();
	return retval;

fail:
	cdf_mem_free(pReqMsg);
	return -EINVAL;
}
/*
 * done with short names for the global vendor params
 * used by __wlan_hdd_cfg80211_extscan_start()
 */
#undef PARAM_MAX
#undef PARAM_REQUEST_ID
#undef PARAM_BASE_PERIOD
#undef PARAMS_MAX_AP_PER_SCAN
#undef PARAMS_RPT_THRHLD_PERCENT
#undef PARAMS_RPT_THRHLD_NUM_SCANS
#undef PARAMS_NUM_BUCKETS
#undef PARAM_CONFIG_FLAGS

/**
 * wlan_hdd_cfg80211_extscan_start() - start extscan
 * @wiphy: Pointer to wireless phy.
 * @wdev: Pointer to wireless device.
 * @data: Pointer to input data.
 * @data_len: Length of @data.
 *
 * Return: 0 on success, negative errno on failure
 */
int wlan_hdd_cfg80211_extscan_start(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					const void *data, int data_len)
{
	int ret = 0;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_extscan_start(wiphy, wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}


/*
 * define short names for the global vendor params
 * used by __wlan_hdd_cfg80211_extscan_stop()
 */
#define PARAM_MAX \
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_MAX
#define PARAM_REQUEST_ID \
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_REQUEST_ID

/**
 * __wlan_hdd_cfg80211_extscan_stop() - ext scan stop
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: none
 */
static int
__wlan_hdd_cfg80211_extscan_stop(struct wiphy *wiphy,
				   struct wireless_dev *wdev,
				   const void *data, int data_len)
{
	tpSirExtScanStopReqParams pReqMsg = NULL;
	struct net_device *dev = wdev->netdev;
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *pHddCtx = wiphy_priv(wiphy);
	struct nlattr *tb[PARAM_MAX + 1];
	struct hdd_ext_scan_context *context;
	QDF_STATUS status;
	uint32_t request_id;
	int retval;
	unsigned long rc;

	ENTER();

	if (CDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	retval = wlan_hdd_validate_context(pHddCtx);
	if (0 != retval)
		return -EINVAL;

	if (nla_parse(tb, PARAM_MAX, data, data_len,
			wlan_hdd_extscan_config_policy)) {
		hddLog(LOGE, FL("Invalid ATTR"));
		return -EINVAL;
	}

	pReqMsg = cdf_mem_malloc(sizeof(*pReqMsg));
	if (!pReqMsg) {
		hddLog(LOGE, FL("cdf_mem_malloc failed"));
		return -ENOMEM;
	}

	/* Parse and fetch request Id */
	if (!tb[PARAM_REQUEST_ID]) {
		hddLog(LOGE, FL("attr request id failed"));
		goto fail;
	}

	pReqMsg->requestId = nla_get_u32(tb[PARAM_REQUEST_ID]);
	pReqMsg->sessionId = pAdapter->sessionId;
	hddLog(LOG1, FL("Req Id %d Session Id %d"),
		pReqMsg->requestId, pReqMsg->sessionId);

	context = &ext_scan_context;
	spin_lock(&context->context_lock);
	INIT_COMPLETION(context->response_event);
	context->request_id = request_id = pReqMsg->requestId;
	spin_unlock(&context->context_lock);

	status = sme_ext_scan_stop(pHddCtx->hHal, pReqMsg);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hddLog(LOGE,
		       FL("sme_ext_scan_stop failed(err=%d)"), status);
		goto fail;
	}

	/* request was sent -- wait for the response */
	rc = wait_for_completion_timeout(&context->response_event,
				msecs_to_jiffies(WLAN_WAIT_TIME_EXTSCAN));

	if (!rc) {
		hddLog(LOGE, FL("sme_ext_scan_stop timed out"));
		retval = -ETIMEDOUT;
	} else {
		spin_lock(&context->context_lock);
		if (context->request_id == request_id)
			retval = context->response_status;
		else
			retval = -EINVAL;
		spin_unlock(&context->context_lock);
	}
	EXIT();
	return retval;

fail:
	cdf_mem_free(pReqMsg);
	return -EINVAL;
}
/*
 * done with short names for the global vendor params
 * used by wlan_hdd_cfg80211_extscan_stop()
 */
#undef PARAM_MAX
#undef PARAM_REQUEST_ID


/**
 * wlan_hdd_cfg80211_extscan_stop() - stop extscan
 * @wiphy: Pointer to wireless phy.
 * @wdev: Pointer to wireless device.
 * @data: Pointer to input data.
 * @data_len: Length of @data.
 *
 * Return: 0 on success, negative errno on failure
 */
int wlan_hdd_cfg80211_extscan_stop(struct wiphy *wiphy,
				struct wireless_dev *wdev,
				const void *data, int data_len)
{
	int ret = 0;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_extscan_stop(wiphy, wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}


/**
 * __wlan_hdd_cfg80211_extscan_reset_bssid_hotlist() - reset bssid hotlist
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: none
 */
static int
__wlan_hdd_cfg80211_extscan_reset_bssid_hotlist(struct wiphy *wiphy,
						  struct wireless_dev
						  *wdev, const void *data,
						  int data_len)
{
	tpSirExtScanResetBssidHotlistReqParams pReqMsg = NULL;
	struct net_device *dev = wdev->netdev;
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *pHddCtx = wiphy_priv(wiphy);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_MAX +
			  1];
	struct hdd_ext_scan_context *context;
	uint32_t request_id;
	QDF_STATUS status;
	int retval;
	unsigned long rc;

	ENTER();

	if (CDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	retval = wlan_hdd_validate_context(pHddCtx);
	if (0 != retval)
		return -EINVAL;

	if (nla_parse(tb, QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_MAX,
		      data, data_len, wlan_hdd_extscan_config_policy)) {
		hddLog(LOGE, FL("Invalid ATTR"));
		return -EINVAL;
	}

	pReqMsg = cdf_mem_malloc(sizeof(*pReqMsg));
	if (!pReqMsg) {
		hddLog(LOGE, FL("cdf_mem_malloc failed"));
		return -ENOMEM;
	}

	/* Parse and fetch request Id */
	if (!tb[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_REQUEST_ID]) {
		hddLog(LOGE, FL("attr request id failed"));
		goto fail;
	}

	pReqMsg->requestId =
		nla_get_u32(tb
		 [QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_REQUEST_ID]);
	pReqMsg->sessionId = pAdapter->sessionId;
	hddLog(LOG1, FL("Req Id %d Session Id %d"),
		pReqMsg->requestId, pReqMsg->sessionId);

	context = &ext_scan_context;
	spin_lock(&context->context_lock);
	INIT_COMPLETION(context->response_event);
	context->request_id = request_id = pReqMsg->requestId;
	spin_unlock(&context->context_lock);

	status = sme_reset_bss_hotlist(pHddCtx->hHal, pReqMsg);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hddLog(LOGE,
		       FL("sme_reset_bss_hotlist failed(err=%d)"), status);
		goto fail;
	}

	/* request was sent -- wait for the response */
	rc = wait_for_completion_timeout
		(&context->response_event,
		 msecs_to_jiffies(WLAN_WAIT_TIME_EXTSCAN));
	if (!rc) {
		hddLog(LOGE, FL("sme_reset_bss_hotlist timed out"));
		retval = -ETIMEDOUT;
	} else {
		spin_lock(&context->context_lock);
		if (context->request_id == request_id)
			retval = context->response_status;
		else
			retval = -EINVAL;
		spin_unlock(&context->context_lock);
	}
	EXIT();
	return retval;

fail:
	cdf_mem_free(pReqMsg);
	return -EINVAL;
}

/**
 * wlan_hdd_cfg80211_extscan_reset_bssid_hotlist() - reset bssid hot list
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: 0 on success, negative errno on failure
 */
int wlan_hdd_cfg80211_extscan_reset_bssid_hotlist(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					const void *data, int data_len)
{
	int ret = 0;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_extscan_reset_bssid_hotlist(wiphy, wdev,
								data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __wlan_hdd_cfg80211_extscan_reset_significant_change() -
 *		reset significant change
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: none
 */
static int
__wlan_hdd_cfg80211_extscan_reset_significant_change(struct wiphy
						       *wiphy,
						       struct
						       wireless_dev
						       *wdev, const void *data,
						       int data_len)
{
	tpSirExtScanResetSignificantChangeReqParams pReqMsg = NULL;
	struct net_device *dev = wdev->netdev;
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *pHddCtx = wiphy_priv(wiphy);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_MAX +
			  1];
	struct hdd_ext_scan_context *context;
	uint32_t request_id;
	QDF_STATUS status;
	int retval;
	unsigned long rc;

	ENTER();

	if (CDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	retval = wlan_hdd_validate_context(pHddCtx);
	if (0 != retval)
		return -EINVAL;

	if (nla_parse(tb, QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_MAX,
		      data, data_len, wlan_hdd_extscan_config_policy)) {
		hddLog(LOGE, FL("Invalid ATTR"));
		return -EINVAL;
	}

	pReqMsg = cdf_mem_malloc(sizeof(*pReqMsg));
	if (!pReqMsg) {
		hddLog(LOGE, FL("cdf_mem_malloc failed"));
		return -ENOMEM;
	}

	/* Parse and fetch request Id */
	if (!tb[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_REQUEST_ID]) {
		hddLog(LOGE, FL("attr request id failed"));
		goto fail;
	}

	pReqMsg->requestId =
		nla_get_u32(tb
		 [QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_REQUEST_ID]);
	pReqMsg->sessionId = pAdapter->sessionId;
	hddLog(LOG1, FL("Req Id %d Session Id %d"),
		pReqMsg->requestId, pReqMsg->sessionId);

	context = &ext_scan_context;
	spin_lock(&context->context_lock);
	INIT_COMPLETION(context->response_event);
	context->request_id = request_id = pReqMsg->requestId;
	spin_unlock(&context->context_lock);

	status = sme_reset_significant_change(pHddCtx->hHal, pReqMsg);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hddLog(LOGE, FL("sme_reset_significant_change failed(err=%d)"),
			status);
		cdf_mem_free(pReqMsg);
		return -EINVAL;
	}

	/* request was sent -- wait for the response */
	rc = wait_for_completion_timeout(&context->response_event,
				msecs_to_jiffies(WLAN_WAIT_TIME_EXTSCAN));

	if (!rc) {
		hddLog(LOGE, FL("sme_ResetSignificantChange timed out"));
		retval = -ETIMEDOUT;
	} else {
		spin_lock(&context->context_lock);
		if (context->request_id == request_id)
			retval = context->response_status;
		else
			retval = -EINVAL;
		spin_unlock(&context->context_lock);
	}
	EXIT();
	return retval;

fail:
	cdf_mem_free(pReqMsg);
	return -EINVAL;
}

/**
 * wlan_hdd_cfg80211_extscan_reset_significant_change() - reset significant
 *							change
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: 0 on success, negative errno on failure
 */
int wlan_hdd_cfg80211_extscan_reset_significant_change(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data, int data_len)
{
	int ret = 0;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_extscan_reset_significant_change(wiphy, wdev,
						data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}


/**
 * hdd_extscan_epno_fill_network_list() - epno fill network list
 * @hddctx: HDD context
 * @req_msg: request message
 * @tb: vendor attribute table
 *
 * This function reads the network block NL vendor attributes from %tb and
 * fill in the epno request message.
 *
 * Return: 0 on success, error number otherwise
 */
static int hdd_extscan_epno_fill_network_list(
			hdd_context_t *hddctx,
			struct wifi_epno_params *req_msg,
			struct nlattr **tb)
{
	struct nlattr *network[
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_MAX + 1];
	struct nlattr *networks;
	int rem1, ssid_len;
	uint8_t index, *ssid;

	index = 0;
	nla_for_each_nested(networks,
		tb[QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORKS_LIST],
		rem1) {
		if (nla_parse(network,
			QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_MAX,
			nla_data(networks), nla_len(networks), NULL)) {
			hddLog(LOGE, FL("nla_parse failed"));
			return -EINVAL;
		}

		/* Parse and fetch ssid */
		if (!network[QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORK_SSID]) {
			hddLog(LOGE, FL("attr network ssid failed"));
			return -EINVAL;
		}
		ssid_len = nla_len(
			network[QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORK_SSID]);

		/* Decrement by 1, don't count null character */
		ssid_len--;

		req_msg->networks[index].ssid.length = ssid_len;
		hddLog(LOG1, FL("network ssid length %d"), ssid_len);
		ssid = nla_data(network[QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORK_SSID]);
		cdf_mem_copy(req_msg->networks[index].ssid.ssId,
				ssid, ssid_len);
		hddLog(LOG1, FL("Ssid (%.*s)"),
			req_msg->networks[index].ssid.length,
			req_msg->networks[index].ssid.ssId);

		/* Parse and fetch rssi threshold */
		if (!network[QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORK_RSSI_THRESHOLD]) {
			hddLog(LOGE, FL("attr rssi threshold failed"));
			return -EINVAL;
		}
		req_msg->networks[index].rssi_threshold = nla_get_s8(
			network[QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORK_RSSI_THRESHOLD]);
		hddLog(LOG1, FL("rssi threshold %d"),
			req_msg->networks[index].rssi_threshold);

		/* Parse and fetch epno flags */
		if (!network[QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORK_FLAGS]) {
			hddLog(LOGE, FL("attr epno flags failed"));
			return -EINVAL;
		}
		req_msg->networks[index].flags = nla_get_u8(
			network[QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORK_FLAGS]);
		hddLog(LOG1, FL("flags %u"), req_msg->networks[index].flags);

		/* Parse and fetch auth bit */
		if (!network[QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORK_AUTH_BIT]) {
			hddLog(LOGE, FL("attr auth bit failed"));
			return -EINVAL;
		}
		req_msg->networks[index].auth_bit_field = nla_get_u8(
			network[QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORK_AUTH_BIT]);
		hddLog(LOG1, FL("auth bit %u"),
			req_msg->networks[index].auth_bit_field);

		index++;
	}
	return 0;
}

/**
 * __wlan_hdd_cfg80211_set_epno_list() - epno set network list
 * @wiphy: wiphy
 * @wdev: pointer to wireless dev
 * @data: data pointer
 * @data_len: data length
 *
 * This function reads the NL vendor attributes from %tb and
 * fill in the epno request message.
 *
 * Return: 0 on success, error number otherwise
 */
static int __wlan_hdd_cfg80211_set_epno_list(struct wiphy *wiphy,
					     struct wireless_dev *wdev,
					     const void *data,
					     int data_len)
{
	struct wifi_epno_params *req_msg = NULL;
	struct net_device *dev           = wdev->netdev;
	hdd_adapter_t *adapter           = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx           = wiphy_priv(wiphy);
	struct nlattr *tb[
		QCA_WLAN_VENDOR_ATTR_PNO_MAX + 1];
	QDF_STATUS status;
	uint32_t num_networks, len;
	int ret_val;

	ENTER();

	ret_val = wlan_hdd_validate_context(hdd_ctx);
	if (ret_val)
		return ret_val;

	if (CDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	if (nla_parse(tb, QCA_WLAN_VENDOR_ATTR_PNO_MAX,
		data, data_len,
		wlan_hdd_extscan_config_policy)) {
		hddLog(LOGE, FL("Invalid ATTR"));
		return -EINVAL;
	}

	/* Parse and fetch number of networks */
	if (!tb[QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_NUM_NETWORKS]) {
		hddLog(LOGE, FL("attr num networks failed"));
		return -EINVAL;
	}
	num_networks = nla_get_u32(
		tb[QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_NUM_NETWORKS]);
	hddLog(LOG1, FL("num networks %u"), num_networks);

	len = sizeof(*req_msg) +
		(num_networks * sizeof(struct wifi_epno_network));
	req_msg = cdf_mem_malloc(len);
	if (!req_msg) {
		hddLog(LOGE, FL("cdf_mem_malloc failed"));
		return -ENOMEM;
	}
	cdf_mem_zero(req_msg, len);
	req_msg->num_networks = num_networks;

	/* Parse and fetch request Id */
	if (!tb[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_REQUEST_ID]) {
		hddLog(LOGE, FL("attr request id failed"));
		goto fail;
	}
	req_msg->request_id = nla_get_u32(
	    tb[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_REQUEST_ID]);
	hddLog(LOG1, FL("Req Id %u"), req_msg->request_id);

	req_msg->session_id = adapter->sessionId;
	hddLog(LOG1, FL("Session Id %d"), req_msg->session_id);

	if (hdd_extscan_epno_fill_network_list(hdd_ctx, req_msg, tb))
		goto fail;

	status = sme_set_epno_list(hdd_ctx->hHal, req_msg);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hddLog(LOGE, FL("sme_set_epno_list failed(err=%d)"), status);
		goto fail;
	}

	EXIT();
	cdf_mem_free(req_msg);
	return 0;

fail:
	cdf_mem_free(req_msg);
	return -EINVAL;
}

 /**
 * wlan_hdd_cfg80211_set_epno_list() - epno set network list
 * @wiphy: wiphy
 * @wdev: pointer to wireless dev
 * @data: data pointer
 * @data_len: data length
 *
 * This function reads the NL vendor attributes from %tb and
 * fill in the epno request message.
 *
 * Return: 0 on success, error number otherwise
 */
int wlan_hdd_cfg80211_set_epno_list(struct wiphy *wiphy,
				    struct wireless_dev *wdev,
				    const void *data,
				    int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_set_epno_list(wiphy, wdev,
						data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * hdd_extscan_passpoint_fill_network_list() - passpoint fill network list
 * @hddctx: HDD context
 * @req_msg: request message
 * @tb: vendor attribute table
 *
 * This function reads the network block NL vendor attributes from %tb and
 * fill in the passpoint request message.
 *
 * Return: 0 on success, error number otherwise
 */
static int hdd_extscan_passpoint_fill_network_list(
			hdd_context_t *hddctx,
			struct wifi_passpoint_req *req_msg,
			struct nlattr **tb)
{
	struct nlattr *network[QCA_WLAN_VENDOR_ATTR_PNO_MAX + 1];
	struct nlattr *networks;
	int rem1, len;
	uint8_t index;

	index = 0;
	nla_for_each_nested(networks,
		tb[QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_LIST_PARAM_NETWORK_ARRAY],
		rem1) {
		if (nla_parse(network,
			QCA_WLAN_VENDOR_ATTR_PNO_MAX,
			nla_data(networks), nla_len(networks), NULL)) {
			hddLog(LOGE, FL("nla_parse failed"));
			return -EINVAL;
		}

		/* Parse and fetch identifier */
		if (!network[QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_NETWORK_PARAM_ID]) {
			hddLog(LOGE, FL("attr passpoint id failed"));
			return -EINVAL;
		}
		req_msg->networks[index].id = nla_get_u32(
			network[QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_NETWORK_PARAM_ID]);
		hddLog(LOG1, FL("Id %u"), req_msg->networks[index].id);

		/* Parse and fetch realm */
		if (!network[QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_NETWORK_PARAM_REALM]) {
			hddLog(LOGE, FL("attr realm failed"));
			return -EINVAL;
		}
		len = nla_len(
			network[QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_NETWORK_PARAM_REALM]);
		if (len < 0 || len > SIR_PASSPOINT_REALM_LEN) {
			hddLog(LOGE, FL("Invalid realm size %d"), len);
			return -EINVAL;
		}
		cdf_mem_copy(req_msg->networks[index].realm,
				nla_data(network[QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_NETWORK_PARAM_REALM]),
				len);
		hddLog(LOG1, FL("realm len %d"), len);
		hddLog(LOG1, FL("realm: %s"), req_msg->networks[index].realm);

		/* Parse and fetch roaming consortium ids */
		if (!network[QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_NETWORK_PARAM_ROAM_CNSRTM_ID]) {
			hddLog(LOGE, FL("attr roaming consortium ids failed"));
			return -EINVAL;
		}
		nla_memcpy(&req_msg->networks[index].roaming_consortium_ids,
			network[QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_NETWORK_PARAM_ROAM_CNSRTM_ID],
			sizeof(req_msg->networks[0].roaming_consortium_ids));
		hddLog(LOG1, FL("roaming consortium ids"));

		/* Parse and fetch plmn */
		if (!network[QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_NETWORK_PARAM_ROAM_PLMN]) {
			hddLog(LOGE, FL("attr plmn failed"));
			return -EINVAL;
		}
		nla_memcpy(&req_msg->networks[index].plmn,
			network[QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_NETWORK_PARAM_ROAM_PLMN],
			SIR_PASSPOINT_PLMN_LEN);
		hddLog(LOG1, FL("plmn %02x:%02x:%02x)"),
			req_msg->networks[index].plmn[0],
			req_msg->networks[index].plmn[1],
			req_msg->networks[index].plmn[2]);

		index++;
	}
	return 0;
}

/**
 * __wlan_hdd_cfg80211_set_passpoint_list() - set passpoint network list
 * @wiphy: wiphy
 * @wdev: pointer to wireless dev
 * @data: data pointer
 * @data_len: data length
 *
 * This function reads the NL vendor attributes from %tb and
 * fill in the passpoint request message.
 *
 * Return: 0 on success, error number otherwise
 */
static int __wlan_hdd_cfg80211_set_passpoint_list(struct wiphy *wiphy,
						  struct wireless_dev *wdev,
						  const void *data,
						  int data_len)
{
	struct wifi_passpoint_req *req_msg = NULL;
	struct net_device *dev             = wdev->netdev;
	hdd_adapter_t *adapter             = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx             = wiphy_priv(wiphy);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_PNO_MAX + 1];
	QDF_STATUS status;
	uint32_t num_networks = 0;
	int ret;

	ENTER();

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	if (CDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	if (nla_parse(tb, QCA_WLAN_VENDOR_ATTR_PNO_MAX, data, data_len,
		wlan_hdd_extscan_config_policy)) {
		hddLog(LOGE, FL("Invalid ATTR"));
		return -EINVAL;
	}

	/* Parse and fetch number of networks */
	if (!tb[QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_LIST_PARAM_NUM]) {
		hddLog(LOGE, FL("attr num networks failed"));
		return -EINVAL;
	}
	num_networks = nla_get_u32(
		tb[QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_LIST_PARAM_NUM]);
	hddLog(LOG1, FL("num networks %u"), num_networks);

	req_msg = cdf_mem_malloc(sizeof(*req_msg) +
			(num_networks * sizeof(req_msg->networks[0])));
	if (!req_msg) {
		hddLog(LOGE, FL("cdf_mem_malloc failed"));
		return -ENOMEM;
	}
	req_msg->num_networks = num_networks;

	/* Parse and fetch request Id */
	if (!tb[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_REQUEST_ID]) {
		hddLog(LOGE, FL("attr request id failed"));
		goto fail;
	}
	req_msg->request_id = nla_get_u32(
	    tb[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_REQUEST_ID]);

	req_msg->session_id = adapter->sessionId;
	hddLog(LOG1, FL("Req Id %u Session Id %d"), req_msg->request_id,
			req_msg->session_id);

	if (hdd_extscan_passpoint_fill_network_list(hdd_ctx, req_msg, tb))
		goto fail;

	status = sme_set_passpoint_list(hdd_ctx->hHal, req_msg);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hddLog(LOGE,
			FL("sme_set_passpoint_list failed(err=%d)"), status);
		goto fail;
	}

	EXIT();
	cdf_mem_free(req_msg);
	return 0;

fail:
	cdf_mem_free(req_msg);
	return -EINVAL;
}

/**
 * wlan_hdd_cfg80211_set_passpoint_list() - set passpoint network list
 * @wiphy: wiphy
 * @wdev: pointer to wireless dev
 * @data: data pointer
 * @data_len: data length
 *
 * This function reads the NL vendor attributes from %tb and
 * fill in the passpoint request message.
 *
 * Return: 0 on success, error number otherwise
 */
int wlan_hdd_cfg80211_set_passpoint_list(struct wiphy *wiphy,
					 struct wireless_dev *wdev,
					 const void *data,
					 int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_set_passpoint_list(wiphy, wdev,
						     data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __wlan_hdd_cfg80211_reset_passpoint_list() - reset passpoint network list
 * @wiphy: wiphy
 * @wdev: pointer to wireless dev
 * @data: data pointer
 * @data_len: data length
 *
 * This function resets passpoint networks list
 *
 * Return: 0 on success, error number otherwise
 */
static int __wlan_hdd_cfg80211_reset_passpoint_list(struct wiphy *wiphy,
						    struct wireless_dev *wdev,
						    const void *data,
						    int data_len)
{
	struct wifi_passpoint_req *req_msg = NULL;
	struct net_device *dev             = wdev->netdev;
	hdd_adapter_t *adapter             = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx             = wiphy_priv(wiphy);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_PNO_MAX + 1];
	QDF_STATUS status;
	int ret;

	ENTER();

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	if (CDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	if (nla_parse(tb, QCA_WLAN_VENDOR_ATTR_PNO_MAX, data, data_len,
		wlan_hdd_extscan_config_policy)) {
		hddLog(LOGE, FL("Invalid ATTR"));
		return -EINVAL;
	}

	req_msg = cdf_mem_malloc(sizeof(*req_msg));
	if (!req_msg) {
		hddLog(LOGE, FL("cdf_mem_malloc failed"));
		return -ENOMEM;
	}

	/* Parse and fetch request Id */
	if (!tb[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_REQUEST_ID]) {
		hddLog(LOGE, FL("attr request id failed"));
		goto fail;
	}
	req_msg->request_id = nla_get_u32(
	    tb[QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_REQUEST_ID]);

	req_msg->session_id = adapter->sessionId;
	hddLog(LOG1, FL("Req Id %u Session Id %d"),
			req_msg->request_id, req_msg->session_id);

	status = sme_reset_passpoint_list(hdd_ctx->hHal, req_msg);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hddLog(LOGE,
			FL("sme_reset_passpoint_list failed(err=%d)"), status);
		goto fail;
	}

	EXIT();
	cdf_mem_free(req_msg);
	return 0;

fail:
	cdf_mem_free(req_msg);
	return -EINVAL;
}

/**
 * wlan_hdd_cfg80211_reset_passpoint_list() - reset passpoint network list
 * @wiphy: wiphy
 * @wdev: pointer to wireless dev
 * @data: data pointer
 * @data_len: data length
 *
 * This function resets passpoint networks list
 *
 * Return: 0 on success, error number otherwise
 */
int wlan_hdd_cfg80211_reset_passpoint_list(struct wiphy *wiphy,
					   struct wireless_dev *wdev,
					   const void *data,
					   int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_reset_passpoint_list(wiphy, wdev,
						       data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/*
 * define short names for the global vendor params
 * used by __wlan_hdd_cfg80211_extscan_set_ssid_hotlist()
 */
#define PARAM_MAX \
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_MAX
#define PARAM_REQUEST_ID \
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_REQUEST_ID
#define PARAMS_LOST_SSID_SAMPLE_SIZE \
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_SSID_HOTLIST_PARAMS_LOST_SSID_SAMPLE_SIZE
#define PARAMS_NUM_SSID \
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_SSID_HOTLIST_PARAMS_NUM_SSID
#define THRESHOLD_PARAM \
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_SSID_THRESHOLD_PARAM
#define PARAM_SSID \
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_SSID_THRESHOLD_PARAM_SSID
#define PARAM_BAND \
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_SSID_THRESHOLD_PARAM_BAND
#define PARAM_RSSI_LOW \
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_SSID_THRESHOLD_PARAM_RSSI_LOW
#define PARAM_RSSI_HIGH \
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_SSID_THRESHOLD_PARAM_RSSI_HIGH

/**
 * __wlan_hdd_cfg80211_extscan_set_ssid_hotlist() - set ssid hot list
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: 0 on success, negative errno on failure
 */
static int
__wlan_hdd_cfg80211_extscan_set_ssid_hotlist(struct wiphy *wiphy,
					   struct wireless_dev *wdev,
					   const void *data,
					   int data_len)
{
	struct sir_set_ssid_hotlist_request *request;
	struct net_device *dev = wdev->netdev;
	hdd_adapter_t *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx = wiphy_priv(wiphy);
	struct nlattr *tb[PARAM_MAX + 1];
	struct nlattr *tb2[PARAM_MAX + 1];
	struct nlattr *ssids;
	struct hdd_ext_scan_context *context;
	uint32_t request_id;
	char ssid_string[SIR_MAC_MAX_SSID_LENGTH + 1];
	int ssid_len, i, rem;
	QDF_STATUS status;
	int retval;
	unsigned long rc;

	ENTER();

	if (CDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	retval = wlan_hdd_validate_context(hdd_ctx);
	if (0 != retval)
		return -EINVAL;

	if (nla_parse(tb, PARAM_MAX,
		      data, data_len,
		      wlan_hdd_extscan_config_policy)) {
		hddLog(LOGE, FL("Invalid ATTR"));
		return -EINVAL;
	}

	request = cdf_mem_malloc(sizeof(*request));
	if (!request) {
		hddLog(LOGE, FL("cdf_mem_malloc failed"));
		return -ENOMEM;
	}

	/* Parse and fetch request Id */
	if (!tb[PARAM_REQUEST_ID]) {
		hddLog(LOGE, FL("attr request id failed"));
		goto fail;
	}

	request->request_id = nla_get_u32(tb[PARAM_REQUEST_ID]);
	hddLog(LOG1, FL("Request Id %d"), request->request_id);

	/* Parse and fetch lost SSID sample size */
	if (!tb[PARAMS_LOST_SSID_SAMPLE_SIZE]) {
		hddLog(LOGE, FL("attr number of Ssid failed"));
		goto fail;
	}
	request->lost_ssid_sample_size =
		nla_get_u32(tb[PARAMS_LOST_SSID_SAMPLE_SIZE]);
	hddLog(LOG1, FL("Lost SSID Sample Size %d"),
	       request->lost_ssid_sample_size);

	/* Parse and fetch number of hotlist SSID */
	if (!tb[PARAMS_NUM_SSID]) {
		hddLog(LOGE, FL("attr number of Ssid failed"));
		goto fail;
	}
	request->ssid_count = nla_get_u32(tb[PARAMS_NUM_SSID]);
	hddLog(LOG1, FL("Number of SSID %d"), request->ssid_count);

	request->session_id = adapter->sessionId;
	hddLog(LOG1, FL("Session Id %d"), request->session_id);

	i = 0;
	nla_for_each_nested(ssids, tb[THRESHOLD_PARAM], rem) {
		if (i >= WLAN_EXTSCAN_MAX_HOTLIST_SSIDS) {
			hddLog(LOGE,
			       FL("Too Many SSIDs, %d exceeds %d"),
			       i, WLAN_EXTSCAN_MAX_HOTLIST_SSIDS);
			break;
		}
		if (nla_parse(tb2, PARAM_MAX,
			      nla_data(ssids), nla_len(ssids),
			      wlan_hdd_extscan_config_policy)) {
			hddLog(LOGE, FL("nla_parse failed"));
			goto fail;
		}

		/* Parse and fetch SSID */
		if (!tb2[PARAM_SSID]) {
			hddLog(LOGE, FL("attr ssid failed"));
			goto fail;
		}
		nla_memcpy(ssid_string,
			   tb2[PARAM_SSID],
			   sizeof(ssid_string));
		hddLog(LOG1, FL("SSID %s"),
		       ssid_string);
		ssid_len = strlen(ssid_string);
		memcpy(request->ssids[i].ssid.ssId, ssid_string, ssid_len);
		request->ssids[i].ssid.length = ssid_len;

		/* Parse and fetch low RSSI */
		if (!tb2[PARAM_BAND]) {
			hddLog(LOGE, FL("attr band failed"));
			goto fail;
		}
		request->ssids[i].band = nla_get_u8(tb2[PARAM_BAND]);
		hddLog(LOG1, FL("band %d"), request->ssids[i].band);

		/* Parse and fetch low RSSI */
		if (!tb2[PARAM_RSSI_LOW]) {
			hddLog(LOGE, FL("attr low RSSI failed"));
			goto fail;
		}
		request->ssids[i].rssi_low = nla_get_s32(tb2[PARAM_RSSI_LOW]);
		hddLog(LOG1, FL("RSSI low %d"), request->ssids[i].rssi_low);

		/* Parse and fetch high RSSI */
		if (!tb2[PARAM_RSSI_HIGH]) {
			hddLog(LOGE, FL("attr high RSSI failed"));
			goto fail;
		}
		request->ssids[i].rssi_high = nla_get_u32(tb2[PARAM_RSSI_HIGH]);
		hddLog(LOG1, FL("RSSI high %d"), request->ssids[i].rssi_high);
		i++;
	}

	context = &ext_scan_context;
	spin_lock(&context->context_lock);
	INIT_COMPLETION(context->response_event);
	context->request_id = request_id = request->request_id;
	spin_unlock(&context->context_lock);

	status = sme_set_ssid_hotlist(hdd_ctx->hHal, request);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hddLog(LOGE,
		       FL("sme_set_ssid_hotlist failed(err=%d)"), status);
		goto fail;
	}

	cdf_mem_free(request);

	/* request was sent -- wait for the response */
	rc = wait_for_completion_timeout(&context->response_event,
					 msecs_to_jiffies
						(WLAN_WAIT_TIME_EXTSCAN));
	if (!rc) {
		hddLog(LOGE, FL("sme_set_ssid_hotlist timed out"));
		retval = -ETIMEDOUT;
	} else {
		spin_lock(&context->context_lock);
		if (context->request_id == request_id)
			retval = context->response_status;
		else
			retval = -EINVAL;
		spin_unlock(&context->context_lock);
	}

	return retval;

fail:
	cdf_mem_free(request);
	return -EINVAL;
}

/*
 * done with short names for the global vendor params
 * used by wlan_hdd_cfg80211_extscan_set_ssid_hotlist()
 */
#undef PARAM_MAX
#undef PARAM_REQUEST_ID
#undef PARAMS_NUM_SSID
#undef THRESHOLD_PARAM
#undef PARAM_SSID
#undef PARAM_BAND
#undef PARAM_RSSI_LOW
#undef PARAM_RSSI_HIGH

/**
 * wlan_hdd_cfg80211_extscan_set_ssid_hotlist() - set ssid hot list
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: 0 on success, negative errno on failure
 */
int
wlan_hdd_cfg80211_extscan_set_ssid_hotlist(struct wiphy *wiphy,
					   struct wireless_dev *wdev,
					   const void *data,
					   int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_extscan_set_ssid_hotlist(wiphy, wdev, data,
							data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/*
 * define short names for the global vendor params
 * used by __wlan_hdd_cfg80211_extscan_reset_ssid_hotlist()
 */
#define PARAM_MAX \
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_MAX
#define PARAM_REQUEST_ID \
		QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_REQUEST_ID

/**
 * __wlan_hdd_cfg80211_extscan_reset_ssid_hotlist() - reset ssid hot list
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: 0 on success, negative errno on failure
 */
static int
__wlan_hdd_cfg80211_extscan_reset_ssid_hotlist(struct wiphy *wiphy,
					     struct wireless_dev *wdev,
					     const void *data,
					     int data_len)
{
	struct sir_set_ssid_hotlist_request *request;
	struct net_device *dev = wdev->netdev;
	hdd_adapter_t *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx = wiphy_priv(wiphy);
	struct nlattr *tb[PARAM_MAX + 1];
	struct hdd_ext_scan_context *context;
	uint32_t request_id;
	QDF_STATUS status;
	int retval;
	unsigned long rc;

	ENTER();

	if (CDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	retval = wlan_hdd_validate_context(hdd_ctx);
	if (0 != retval)
		return -EINVAL;

	if (nla_parse(tb, PARAM_MAX,
		      data, data_len,
		      wlan_hdd_extscan_config_policy)) {
		hddLog(LOGE, FL("Invalid ATTR"));
		return -EINVAL;
	}

	request = cdf_mem_malloc(sizeof(*request));
	if (!request) {
		hddLog(LOGE, FL("cdf_mem_malloc failed"));
		return -ENOMEM;
	}

	/* Parse and fetch request Id */
	if (!tb[PARAM_REQUEST_ID]) {
		hddLog(LOGE, FL("attr request id failed"));
		goto fail;
	}

	request->request_id = nla_get_u32(tb[PARAM_REQUEST_ID]);
	hddLog(LOG1, FL("Request Id %d"), request->request_id);

	request->session_id = adapter->sessionId;
	hddLog(LOG1, FL("Session Id %d"), request->session_id);

	request->lost_ssid_sample_size = 0;
	request->ssid_count = 0;

	context = &ext_scan_context;
	spin_lock(&context->context_lock);
	INIT_COMPLETION(context->response_event);
	context->request_id = request_id = request->request_id;
	spin_unlock(&context->context_lock);

	status = sme_set_ssid_hotlist(hdd_ctx->hHal, request);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hddLog(LOGE,
		       FL("sme_reset_ssid_hotlist failed(err=%d)"), status);
		goto fail;
	}

	cdf_mem_free(request);

	/* request was sent -- wait for the response */
	rc = wait_for_completion_timeout(&context->response_event,
					 msecs_to_jiffies
						(WLAN_WAIT_TIME_EXTSCAN));
	if (!rc) {
		hddLog(LOGE, FL("sme_reset_ssid_hotlist timed out"));
		retval = -ETIMEDOUT;
	} else {
		spin_lock(&context->context_lock);
		if (context->request_id == request_id)
			retval = context->response_status;
		else
			retval = -EINVAL;
		spin_unlock(&context->context_lock);
	}

	return retval;

fail:
	cdf_mem_free(request);
	return -EINVAL;
}

/*
 * done with short names for the global vendor params
 * used by wlan_hdd_cfg80211_extscan_reset_ssid_hotlist()
 */
#undef PARAM_MAX
#undef PARAM_REQUEST_ID

/**
 * wlan_hdd_cfg80211_extscan_reset_ssid_hotlist() - reset ssid hot list
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: 0 on success, negative errno on failure
 */
int
wlan_hdd_cfg80211_extscan_reset_ssid_hotlist(struct wiphy *wiphy,
					     struct wireless_dev *wdev,
					     const void *data,
					     int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_extscan_reset_ssid_hotlist(wiphy, wdev,
							data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * wlan_hdd_init_completion_extwow() - Initialize ext wow variable
 * @hdd_ctx: Global HDD context
 *
 * Return: none
 */
#ifdef WLAN_FEATURE_EXTWOW_SUPPORT
static inline void wlan_hdd_init_completion_extwow(hdd_context_t *pHddCtx)
{
	init_completion(&pHddCtx->ready_to_extwow);
}
#else
static inline void wlan_hdd_init_completion_extwow(hdd_context_t *pHddCtx)
{
	return;
}
#endif

/**
 * wlan_hdd_cfg80211_extscan_init() - Initialize the ExtScan feature
 * @hdd_ctx: Global HDD context
 *
 * Return: none
 */
void wlan_hdd_cfg80211_extscan_init(hdd_context_t *hdd_ctx)
{
	wlan_hdd_init_completion_extwow(hdd_ctx);
	init_completion(&ext_scan_context.response_event);
	spin_lock_init(&ext_scan_context.context_lock);
}

#endif /* FEATURE_WLAN_EXTSCAN */
