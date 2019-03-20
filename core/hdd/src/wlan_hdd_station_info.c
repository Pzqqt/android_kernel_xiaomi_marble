/*
 * Copyright (c) 2012-2019 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_hdd_station_info.c
 *
 * WLAN station info functions
 *
 */

#include "osif_sync.h"
#include <wlan_hdd_includes.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <linux/if_ether.h>
#include <wlan_cp_stats_mc_ucfg_api.h>
#include <wlan_hdd_stats.h>
#include <wlan_hdd_hostapd.h>
#include <wlan_hdd_station_info.h>
#include "wlan_mlme_ucfg_api.h"

/*
 * define short names for the global vendor params
 * used by __wlan_hdd_cfg80211_get_station_cmd()
 */
#define STATION_INVALID \
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INVALID
#define STATION_INFO \
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO
#define STATION_ASSOC_FAIL_REASON \
	QCA_WLAN_VENDOR_ATTR_GET_STATION_ASSOC_FAIL_REASON
#define STATION_REMOTE \
	QCA_WLAN_VENDOR_ATTR_GET_STATION_REMOTE
#define STATION_MAX \
	QCA_WLAN_VENDOR_ATTR_GET_STATION_MAX

/* define short names for get station info attributes */
#define LINK_INFO_STANDARD_NL80211_ATTR \
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_LINK_STANDARD_NL80211_ATTR
#define AP_INFO_STANDARD_NL80211_ATTR \
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_AP_STANDARD_NL80211_ATTR
#define INFO_ROAM_COUNT \
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_ROAM_COUNT
#define INFO_AKM \
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_AKM
#define WLAN802_11_MODE \
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_802_11_MODE
#define AP_INFO_HS20_INDICATION \
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_AP_HS20_INDICATION
#define HT_OPERATION \
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_HT_OPERATION
#define VHT_OPERATION \
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_VHT_OPERATION
#define INFO_ASSOC_FAIL_REASON \
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_ASSOC_FAIL_REASON
#define REMOTE_MAX_PHY_RATE \
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_MAX_PHY_RATE
#define REMOTE_TX_PACKETS \
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_TX_PACKETS
#define REMOTE_TX_BYTES \
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_TX_BYTES
#define REMOTE_RX_PACKETS \
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_RX_PACKETS
#define REMOTE_RX_BYTES \
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_RX_BYTES
#define REMOTE_LAST_TX_RATE \
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_LAST_TX_RATE
#define REMOTE_LAST_RX_RATE \
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_LAST_RX_RATE
#define REMOTE_WMM \
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_WMM
#define REMOTE_SUPPORTED_MODE \
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_SUPPORTED_MODE
#define REMOTE_AMPDU \
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_AMPDU
#define REMOTE_TX_STBC \
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_TX_STBC
#define REMOTE_RX_STBC \
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_RX_STBC
#define REMOTE_CH_WIDTH\
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_CH_WIDTH
#define REMOTE_SGI_ENABLE\
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_SGI_ENABLE
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
	#define REMOTE_PAD\
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_PAD
#endif

static const struct nla_policy
hdd_get_station_policy[STATION_MAX + 1] = {
	[STATION_INFO] = {.type = NLA_FLAG},
	[STATION_ASSOC_FAIL_REASON] = {.type = NLA_FLAG},
	[STATION_REMOTE] = {.type = NLA_BINARY, .len = QDF_MAC_ADDR_SIZE},
};

#ifdef QCA_SUPPORT_CP_STATS
static int hdd_get_sta_congestion(struct hdd_adapter *adapter,
				  uint32_t *congestion)
{
	QDF_STATUS status;
	struct cca_stats cca_stats;

	status = ucfg_mc_cp_stats_cca_stats_get(adapter->vdev, &cca_stats);
	if (QDF_IS_STATUS_ERROR(status))
		return -EINVAL;

	*congestion = cca_stats.congestion;
	return 0;
}
#else
static int hdd_get_sta_congestion(struct hdd_adapter *adapter,
				  uint32_t *congestion)
{
	struct hdd_station_ctx *hdd_sta_ctx;

	hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	*congestion = hdd_sta_ctx->conn_info.cca;
	return 0;
}
#endif

/**
 * hdd_get_station_assoc_fail() - Handle get station assoc fail
 * @hdd_ctx: HDD context within host driver
 * @wdev: wireless device
 *
 * Handles QCA_NL80211_VENDOR_SUBCMD_GET_STATION_ASSOC_FAIL.
 * Validate cmd attributes and send the station info to upper layers.
 *
 * Return: Success(0) or reason code for failure
 */
static int hdd_get_station_assoc_fail(struct hdd_context *hdd_ctx,
				      struct hdd_adapter *adapter)
{
	struct sk_buff *skb = NULL;
	uint32_t nl_buf_len;
	struct hdd_station_ctx *hdd_sta_ctx;
	uint32_t congestion;

	nl_buf_len = NLMSG_HDRLEN;
	nl_buf_len += sizeof(uint32_t);
	skb = cfg80211_vendor_cmd_alloc_reply_skb(hdd_ctx->wiphy, nl_buf_len);

	if (!skb) {
		hdd_err("cfg80211_vendor_cmd_alloc_reply_skb failed");
		return -ENOMEM;
	}

	hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	if (nla_put_u32(skb, INFO_ASSOC_FAIL_REASON,
			hdd_sta_ctx->conn_info.assoc_status_code)) {
		hdd_err("put fail");
		goto fail;
	}

	if (hdd_get_sta_congestion(adapter, &congestion))
		congestion = 0;

	hdd_info("congestion:%d", congestion);
	if (nla_put_u32(skb, NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY,
			congestion)) {
		hdd_err("put fail");
		goto fail;
	}

	return cfg80211_vendor_cmd_reply(skb);
fail:
	if (skb)
		kfree_skb(skb);
	return -EINVAL;
}

/**
 * hdd_map_auth_type() - transform auth type specific to
 * vendor command
 * @auth_type: csr auth type
 *
 * Return: Success(0) or reason code for failure
 */
static int hdd_convert_auth_type(uint32_t auth_type)
{
	uint32_t ret_val;

	switch (auth_type) {
	case eCSR_AUTH_TYPE_OPEN_SYSTEM:
		ret_val = QCA_WLAN_AUTH_TYPE_OPEN;
		break;
	case eCSR_AUTH_TYPE_SHARED_KEY:
		ret_val = QCA_WLAN_AUTH_TYPE_SHARED;
		break;
	case eCSR_AUTH_TYPE_WPA:
		ret_val = QCA_WLAN_AUTH_TYPE_WPA;
		break;
	case eCSR_AUTH_TYPE_WPA_PSK:
		ret_val = QCA_WLAN_AUTH_TYPE_WPA_PSK;
		break;
	case eCSR_AUTH_TYPE_AUTOSWITCH:
		ret_val = QCA_WLAN_AUTH_TYPE_AUTOSWITCH;
		break;
	case eCSR_AUTH_TYPE_WPA_NONE:
		ret_val = QCA_WLAN_AUTH_TYPE_WPA_NONE;
		break;
	case eCSR_AUTH_TYPE_RSN:
		ret_val = QCA_WLAN_AUTH_TYPE_RSN;
		break;
	case eCSR_AUTH_TYPE_RSN_PSK:
		ret_val = QCA_WLAN_AUTH_TYPE_RSN_PSK;
		break;
	case eCSR_AUTH_TYPE_FT_RSN:
		ret_val = QCA_WLAN_AUTH_TYPE_FT;
		break;
	case eCSR_AUTH_TYPE_FT_RSN_PSK:
		ret_val = QCA_WLAN_AUTH_TYPE_FT_PSK;
		break;
	case eCSR_AUTH_TYPE_WAPI_WAI_CERTIFICATE:
		ret_val = QCA_WLAN_AUTH_TYPE_WAI;
		break;
	case eCSR_AUTH_TYPE_WAPI_WAI_PSK:
		ret_val = QCA_WLAN_AUTH_TYPE_WAI_PSK;
		break;
	case eCSR_AUTH_TYPE_CCKM_WPA:
		ret_val = QCA_WLAN_AUTH_TYPE_CCKM_WPA;
		break;
	case eCSR_AUTH_TYPE_CCKM_RSN:
		ret_val = QCA_WLAN_AUTH_TYPE_CCKM_RSN;
		break;
	case eCSR_AUTH_TYPE_RSN_PSK_SHA256:
		ret_val = QCA_WLAN_AUTH_TYPE_SHA256_PSK;
		break;
	case eCSR_AUTH_TYPE_RSN_8021X_SHA256:
		ret_val = QCA_WLAN_AUTH_TYPE_SHA256;
		break;
	case eCSR_AUTH_TYPE_FT_SAE:
		ret_val = QCA_WLAN_AUTH_TYPE_FT_SAE;
		break;
	case eCSR_AUTH_TYPE_FT_SUITEB_EAP_SHA384:
		ret_val = QCA_WLAN_AUTH_TYPE_FT_SUITEB_EAP_SHA384;
		break;
	case eCSR_NUM_OF_SUPPORT_AUTH_TYPE:
	case eCSR_AUTH_TYPE_FAILED:
	case eCSR_AUTH_TYPE_NONE:
	default:
		ret_val = QCA_WLAN_AUTH_TYPE_INVALID;
		break;
	}
	return ret_val;
}

/**
 * hdd_map_dot_11_mode() - transform dot11mode type specific to
 * vendor command
 * @dot11mode: dot11mode
 *
 * Return: Success(0) or reason code for failure
 */
static int hdd_convert_dot11mode(uint32_t dot11mode)
{
	uint32_t ret_val;

	switch (dot11mode) {
	case eCSR_CFG_DOT11_MODE_11A:
		ret_val = QCA_WLAN_802_11_MODE_11A;
		break;
	case eCSR_CFG_DOT11_MODE_11B:
		ret_val = QCA_WLAN_802_11_MODE_11B;
		break;
	case eCSR_CFG_DOT11_MODE_11G:
		ret_val = QCA_WLAN_802_11_MODE_11G;
		break;
	case eCSR_CFG_DOT11_MODE_11N:
		ret_val = QCA_WLAN_802_11_MODE_11N;
		break;
	case eCSR_CFG_DOT11_MODE_11AC:
		ret_val = QCA_WLAN_802_11_MODE_11AC;
		break;
	case eCSR_CFG_DOT11_MODE_AUTO:
	case eCSR_CFG_DOT11_MODE_ABG:
	default:
		ret_val = QCA_WLAN_802_11_MODE_INVALID;
	}
	return ret_val;
}

/**
 * hdd_add_tx_bitrate() - add tx bitrate attribute
 * @skb: pointer to sk buff
 * @hdd_sta_ctx: pointer to hdd station context
 * @idx: attribute index
 *
 * Return: Success(0) or reason code for failure
 */
static int32_t hdd_add_tx_bitrate(struct sk_buff *skb,
				  struct hdd_station_ctx *hdd_sta_ctx,
				  int idx)
{
	struct nlattr *nla_attr;
	uint32_t bitrate, bitrate_compat;

	nla_attr = nla_nest_start(skb, idx);
	if (!nla_attr) {
		hdd_err("nla_nest_start failed");
		goto fail;
	}

	/* cfg80211_calculate_bitrate will return 0 for mcs >= 32 */
	bitrate = cfg80211_calculate_bitrate(&hdd_sta_ctx->
						cache_conn_info.txrate);

	/* report 16-bit bitrate only if we can */
	bitrate_compat = bitrate < (1UL << 16) ? bitrate : 0;

	if (bitrate > 0) {
		if (nla_put_u32(skb, NL80211_RATE_INFO_BITRATE32, bitrate)) {
			hdd_err("put fail bitrate: %u", bitrate);
			goto fail;
		}
	} else {
		hdd_err("Invalid bitrate: %u", bitrate);
	}

	if (bitrate_compat > 0) {
		if (nla_put_u16(skb, NL80211_RATE_INFO_BITRATE,
				bitrate_compat)) {
			hdd_err("put fail bitrate_compat: %u", bitrate_compat);
			goto fail;
		}
	} else {
		hdd_err("Invalid bitrate_compat: %u", bitrate_compat);
	}

	if (nla_put_u8(skb, NL80211_RATE_INFO_VHT_NSS,
		       hdd_sta_ctx->cache_conn_info.txrate.nss)) {
		hdd_err("put fail");
		goto fail;
	}
	nla_nest_end(skb, nla_attr);
	return 0;
fail:
	return -EINVAL;
}

/**
 * hdd_add_sta_info() - add station info attribute
 * @skb: pointer to sk buff
 * @hdd_sta_ctx: pointer to hdd station context
 * @idx: attribute index
 *
 * Return: Success(0) or reason code for failure
 */
static int32_t hdd_add_sta_info(struct sk_buff *skb,
				struct hdd_station_ctx *hdd_sta_ctx,
				int idx)
{
	struct nlattr *nla_attr;

	nla_attr = nla_nest_start(skb, idx);
	if (!nla_attr) {
		hdd_err("nla_nest_start failed");
		goto fail;
	}

	if (nla_put_u8(skb, NL80211_STA_INFO_SIGNAL,
		       (hdd_sta_ctx->cache_conn_info.signal + 100))) {
		hdd_err("put fail");
		goto fail;
	}
	if (hdd_add_tx_bitrate(skb, hdd_sta_ctx, NL80211_STA_INFO_TX_BITRATE)) {
		hdd_err("hdd_add_tx_bitrate failed");
		goto fail;
	}

	nla_nest_end(skb, nla_attr);
	return 0;
fail:
	return -EINVAL;
}

/**
 * hdd_add_survey_info() - add survey info attribute
 * @skb: pointer to sk buff
 * @hdd_sta_ctx: pointer to hdd station context
 * @idx: attribute index
 *
 * Return: Success(0) or reason code for failure
 */
static int32_t hdd_add_survey_info(struct sk_buff *skb,
				   struct hdd_station_ctx *hdd_sta_ctx,
				   int idx)
{
	struct nlattr *nla_attr;

	nla_attr = nla_nest_start(skb, idx);
	if (!nla_attr)
		goto fail;
	if (nla_put_u32(skb, NL80211_SURVEY_INFO_FREQUENCY,
			hdd_sta_ctx->cache_conn_info.freq) ||
	    nla_put_u8(skb, NL80211_SURVEY_INFO_NOISE,
		       (hdd_sta_ctx->cache_conn_info.noise + 100))) {
		hdd_err("put fail");
		goto fail;
	}
	nla_nest_end(skb, nla_attr);
	return 0;
fail:
	return -EINVAL;
}

/**
 * hdd_add_link_standard_info() - add link info attribute
 * @skb: pointer to sk buff
 * @hdd_sta_ctx: pointer to hdd station context
 * @idx: attribute index
 *
 * Return: Success(0) or reason code for failure
 */
static int32_t
hdd_add_link_standard_info(struct sk_buff *skb,
			   struct hdd_station_ctx *hdd_sta_ctx, int idx)
{
	struct nlattr *nla_attr;

	nla_attr = nla_nest_start(skb, idx);
	if (!nla_attr) {
		hdd_err("nla_nest_start failed");
		goto fail;
	}

	if (nla_put(skb,
		    NL80211_ATTR_SSID,
		    hdd_sta_ctx->cache_conn_info.last_ssid.SSID.length,
		    hdd_sta_ctx->cache_conn_info.last_ssid.SSID.ssId)) {
		hdd_err("put fail");
		goto fail;
	}
	if (nla_put(skb, NL80211_ATTR_MAC, QDF_MAC_ADDR_SIZE,
		    hdd_sta_ctx->cache_conn_info.bssid.bytes)) {
		hdd_err("put bssid failed");
		goto fail;
	}
	if (hdd_add_survey_info(skb, hdd_sta_ctx, NL80211_ATTR_SURVEY_INFO)) {
		hdd_err("hdd_add_survey_info failed");
		goto fail;
	}

	if (hdd_add_sta_info(skb, hdd_sta_ctx, NL80211_ATTR_STA_INFO)) {
		hdd_err("hdd_add_sta_info failed");
		goto fail;
	}
	nla_nest_end(skb, nla_attr);
	return 0;
fail:
	return -EINVAL;
}

/**
 * hdd_add_ap_standard_info() - add ap info attribute
 * @skb: pointer to sk buff
 * @hdd_sta_ctx: pointer to hdd station context
 * @idx: attribute index
 *
 * Return: Success(0) or reason code for failure
 */
static int32_t
hdd_add_ap_standard_info(struct sk_buff *skb,
			 struct hdd_station_ctx *hdd_sta_ctx, int idx)
{
	struct nlattr *nla_attr;

	nla_attr = nla_nest_start(skb, idx);
	if (!nla_attr)
		goto fail;
	if (hdd_sta_ctx->cache_conn_info.conn_flag.vht_present)
		if (nla_put(skb, NL80211_ATTR_VHT_CAPABILITY,
			    sizeof(hdd_sta_ctx->cache_conn_info.vht_caps),
			    &hdd_sta_ctx->cache_conn_info.vht_caps)) {
			hdd_err("put fail");
			goto fail;
		}
	if (hdd_sta_ctx->cache_conn_info.conn_flag.ht_present)
		if (nla_put(skb, NL80211_ATTR_HT_CAPABILITY,
			    sizeof(hdd_sta_ctx->cache_conn_info.ht_caps),
			    &hdd_sta_ctx->cache_conn_info.ht_caps)) {
			hdd_err("put fail");
			goto fail;
		}
	nla_nest_end(skb, nla_attr);
	return 0;
fail:
	return -EINVAL;
}

/**
 * hdd_get_station_info() - send BSS information to supplicant
 * @hdd_ctx: pointer to hdd context
 * @adapter: pointer to adapter
 *
 * Return: 0 if success else error status
 */
static int hdd_get_station_info(struct hdd_context *hdd_ctx,
				struct hdd_adapter *adapter)
{
	struct sk_buff *skb = NULL;
	uint8_t *tmp_hs20 = NULL;
	uint32_t nl_buf_len;
	struct hdd_station_ctx *hdd_sta_ctx;

	hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	nl_buf_len = NLMSG_HDRLEN;
	nl_buf_len += sizeof(hdd_sta_ctx->
				cache_conn_info.last_ssid.SSID.length) +
		      QDF_MAC_ADDR_SIZE +
		      sizeof(hdd_sta_ctx->cache_conn_info.freq) +
		      sizeof(hdd_sta_ctx->cache_conn_info.noise) +
		      sizeof(hdd_sta_ctx->cache_conn_info.signal) +
		      (sizeof(uint32_t) * 2) +
		      sizeof(hdd_sta_ctx->cache_conn_info.txrate.nss) +
		      sizeof(hdd_sta_ctx->cache_conn_info.roam_count) +
		      sizeof(hdd_sta_ctx->cache_conn_info.last_auth_type) +
		      sizeof(hdd_sta_ctx->cache_conn_info.dot11mode);
	if (hdd_sta_ctx->cache_conn_info.conn_flag.vht_present)
		nl_buf_len += sizeof(hdd_sta_ctx->cache_conn_info.vht_caps);
	if (hdd_sta_ctx->cache_conn_info.conn_flag.ht_present)
		nl_buf_len += sizeof(hdd_sta_ctx->cache_conn_info.ht_caps);
	if (hdd_sta_ctx->cache_conn_info.conn_flag.hs20_present) {
		tmp_hs20 = (uint8_t *)&(hdd_sta_ctx->
						cache_conn_info.hs20vendor_ie);
		nl_buf_len += (sizeof(hdd_sta_ctx->
					cache_conn_info.hs20vendor_ie) - 1);
	}
	if (hdd_sta_ctx->cache_conn_info.conn_flag.ht_op_present)
		nl_buf_len += sizeof(hdd_sta_ctx->
						cache_conn_info.ht_operation);
	if (hdd_sta_ctx->cache_conn_info.conn_flag.vht_op_present)
		nl_buf_len += sizeof(hdd_sta_ctx->
						cache_conn_info.vht_operation);

	skb = cfg80211_vendor_cmd_alloc_reply_skb(hdd_ctx->wiphy, nl_buf_len);
	if (!skb) {
		hdd_err("cfg80211_vendor_cmd_alloc_reply_skb failed");
		return -ENOMEM;
	}

	if (hdd_add_link_standard_info(skb, hdd_sta_ctx,
				       LINK_INFO_STANDARD_NL80211_ATTR)) {
		hdd_err("put fail");
		goto fail;
	}
	if (hdd_add_ap_standard_info(skb, hdd_sta_ctx,
				     AP_INFO_STANDARD_NL80211_ATTR)) {
		hdd_err("put fail");
		goto fail;
	}
	if (nla_put_u32(skb, INFO_ROAM_COUNT,
			hdd_sta_ctx->cache_conn_info.roam_count) ||
	    nla_put_u32(skb, INFO_AKM,
			hdd_convert_auth_type(
			hdd_sta_ctx->cache_conn_info.last_auth_type)) ||
	    nla_put_u32(skb, WLAN802_11_MODE,
			hdd_convert_dot11mode(
			hdd_sta_ctx->cache_conn_info.dot11mode))) {
		hdd_err("put fail");
		goto fail;
	}
	if (hdd_sta_ctx->cache_conn_info.conn_flag.ht_op_present)
		if (nla_put(skb, HT_OPERATION,
			    (sizeof(hdd_sta_ctx->cache_conn_info.ht_operation)),
			    &hdd_sta_ctx->cache_conn_info.ht_operation)) {
			hdd_err("put fail");
			goto fail;
		}
	if (hdd_sta_ctx->cache_conn_info.conn_flag.vht_op_present)
		if (nla_put(skb, VHT_OPERATION,
			    (sizeof(hdd_sta_ctx->
					cache_conn_info.vht_operation)),
			    &hdd_sta_ctx->cache_conn_info.vht_operation)) {
			hdd_err("put fail");
			goto fail;
		}
	if (hdd_sta_ctx->cache_conn_info.conn_flag.hs20_present)
		if (nla_put(skb, AP_INFO_HS20_INDICATION,
			    (sizeof(hdd_sta_ctx->cache_conn_info.hs20vendor_ie)
			     - 1),
			    tmp_hs20 + 1)) {
			hdd_err("put fail");
			goto fail;
		}

	return cfg80211_vendor_cmd_reply(skb);
fail:
	if (skb)
		kfree_skb(skb);
	return -EINVAL;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
static inline int32_t remote_station_put_u64(struct sk_buff *skb,
					     int32_t attrtype,
					     uint64_t value)
{
	return nla_put_u64_64bit(skb, attrtype, value, REMOTE_PAD);
}
#else
static inline int32_t remote_station_put_u64(struct sk_buff *skb,
					     int32_t attrtype,
					     uint64_t value)
{
	return nla_put_u64(skb, attrtype, value);
}
#endif

/**
 * hdd_add_survey_info_sap_get_len - get data length used in
 * hdd_add_survey_info_sap()
 *
 * This function calculates the data length used in hdd_add_survey_info_sap()
 *
 * Return: total data length used in hdd_add_survey_info_sap()
 */
static uint32_t hdd_add_survey_info_sap_get_len(void)
{
	return ((NLA_HDRLEN) + (sizeof(uint32_t) + NLA_HDRLEN));
}

/**
 * hdd_add_survey_info - add survey info attribute
 * @skb: pointer to response skb buffer
 * @stainfo: station information
 * @idx: attribute type index for nla_next_start()
 *
 * This function adds survey info attribute to response skb buffer
 *
 * Return : 0 on success and errno on failure
 */
static int32_t hdd_add_survey_info_sap(struct sk_buff *skb,
				       struct hdd_station_info *stainfo,
				       int idx)
{
	struct nlattr *nla_attr;

	nla_attr = nla_nest_start(skb, idx);
	if (!nla_attr)
		goto fail;
	if (nla_put_u32(skb, NL80211_SURVEY_INFO_FREQUENCY,
			stainfo->freq)) {
		hdd_err("put fail");
		goto fail;
	}
	nla_nest_end(skb, nla_attr);
	return 0;
fail:
	return -EINVAL;
}

/**
 * hdd_add_tx_bitrate_sap_get_len - get data length used in
 * hdd_add_tx_bitrate_sap()
 *
 * This function calculates the data length used in hdd_add_tx_bitrate_sap()
 *
 * Return: total data length used in hdd_add_tx_bitrate_sap()
 */
static uint32_t hdd_add_tx_bitrate_sap_get_len(void)
{
	return ((NLA_HDRLEN) + (sizeof(uint8_t) + NLA_HDRLEN));
}

static uint32_t hdd_add_sta_capability_get_len(void)
{
	return nla_total_size(sizeof(uint16_t));
}

/**
 * hdd_add_tx_bitrate_sap - add vhs nss info attribute
 * @skb: pointer to response skb buffer
 * @stainfo: station information
 * @idx: attribute type index for nla_next_start()
 *
 * This function adds vht nss attribute to response skb buffer
 *
 * Return : 0 on success and errno on failure
 */
static int hdd_add_tx_bitrate_sap(struct sk_buff *skb,
				  struct hdd_station_info *stainfo,
				  int idx)
{
	struct nlattr *nla_attr;

	nla_attr = nla_nest_start(skb, idx);
	if (!nla_attr)
		goto fail;

	if (nla_put_u8(skb, NL80211_RATE_INFO_VHT_NSS,
		       stainfo->nss)) {
		hdd_err("put fail");
		goto fail;
	}
	nla_nest_end(skb, nla_attr);
	return 0;
fail:
	return -EINVAL;
}

/**
 * hdd_add_sta_info_sap_get_len - get data length used in
 * hdd_add_sta_info_sap()
 *
 * This function calculates the data length used in hdd_add_sta_info_sap()
 *
 * Return: total data length used in hdd_add_sta_info_sap()
 */
static uint32_t hdd_add_sta_info_sap_get_len(void)
{
	return ((NLA_HDRLEN) + (sizeof(uint8_t) + NLA_HDRLEN) +
		hdd_add_tx_bitrate_sap_get_len() +
		hdd_add_sta_capability_get_len());
}

/**
 * hdd_add_sta_info_sap - add sta signal info attribute
 * @skb: pointer to response skb buffer
 * @stainfo: station information
 * @idx: attribute type index for nla_next_start()
 *
 * This function adds sta signal attribute to response skb buffer
 *
 * Return : 0 on success and errno on failure
 */
static int32_t hdd_add_sta_info_sap(struct sk_buff *skb, int8_t rssi,
				    struct hdd_station_info *stainfo, int idx)
{
	struct nlattr *nla_attr;

	nla_attr = nla_nest_start(skb, idx);
	if (!nla_attr)
		goto fail;

	if (nla_put_u8(skb, NL80211_STA_INFO_SIGNAL, rssi)) {
		hdd_err("put fail");
		goto fail;
	}
	if (hdd_add_tx_bitrate_sap(skb, stainfo, NL80211_STA_INFO_TX_BITRATE))
		goto fail;

	nla_nest_end(skb, nla_attr);
	return 0;
fail:
	return -EINVAL;
}

/**
 * hdd_add_link_standard_info_sap_get_len - get data length used in
 * hdd_add_link_standard_info_sap()
 *
 * This function calculates the data length used in
 * hdd_add_link_standard_info_sap()
 *
 * Return: total data length used in hdd_add_link_standard_info_sap()
 */
static uint32_t hdd_add_link_standard_info_sap_get_len(void)
{
	return ((NLA_HDRLEN) +
		hdd_add_survey_info_sap_get_len() +
		hdd_add_sta_info_sap_get_len() +
		(sizeof(uint32_t) + NLA_HDRLEN));
}

/**
 * hdd_add_link_standard_info_sap - add add link info attribut
 * @skb: pointer to response skb buffer
 * @stainfo: station information
 * @idx: attribute type index for nla_next_start()
 *
 * This function adds link info attribut to response skb buffer
 *
 * Return : 0 on success and errno on failure
 */
static int hdd_add_link_standard_info_sap(struct sk_buff *skb, int8_t rssi,
					  struct hdd_station_info *stainfo,
					  int idx)
{
	struct nlattr *nla_attr;

	nla_attr = nla_nest_start(skb, idx);
	if (!nla_attr)
		goto fail;
	if (hdd_add_survey_info_sap(skb, stainfo, NL80211_ATTR_SURVEY_INFO))
		goto fail;
	if (hdd_add_sta_info_sap(skb, rssi, stainfo, NL80211_ATTR_STA_INFO))
		goto fail;

	if (nla_put_u32(skb, NL80211_ATTR_REASON_CODE, stainfo->reason_code)) {
		hdd_err("Reason code put fail");
		goto fail;
	}
	if (nla_put_u16(skb, NL80211_ATTR_STA_CAPABILITY,
			stainfo->capability)) {
		hdd_err("put fail");
		goto fail;
	}
	nla_nest_end(skb, nla_attr);
	return 0;
fail:
	return -EINVAL;
}

/**
 * hdd_add_ap_standard_info_sap_get_len - get data length used in
 * hdd_add_ap_standard_info_sap()
 * @stainfo: station information
 *
 * This function calculates the data length used in
 * hdd_add_ap_standard_info_sap()
 *
 * Return: total data length used in hdd_add_ap_standard_info_sap()
 */
static uint32_t hdd_add_ap_standard_info_sap_get_len(
				struct hdd_station_info *stainfo)
{
	uint32_t len;

	len = NLA_HDRLEN;
	if (stainfo->vht_present)
		len += (sizeof(stainfo->vht_caps) + NLA_HDRLEN);
	if (stainfo->ht_present)
		len += (sizeof(stainfo->ht_caps) + NLA_HDRLEN);

	return len;
}

/**
 * hdd_add_ap_standard_info_sap - add HT and VHT info attributes
 * @skb: pointer to response skb buffer
 * @stainfo: station information
 * @idx: attribute type index for nla_next_start()
 *
 * This function adds HT and VHT info attributes to response skb buffer
 *
 * Return : 0 on success and errno on failure
 */
static int hdd_add_ap_standard_info_sap(struct sk_buff *skb,
					struct hdd_station_info *stainfo,
					int idx)
{
	struct nlattr *nla_attr;

	nla_attr = nla_nest_start(skb, idx);
	if (!nla_attr)
		goto fail;

	if (stainfo->vht_present) {
		if (nla_put(skb, NL80211_ATTR_VHT_CAPABILITY,
			    sizeof(stainfo->vht_caps),
			    &stainfo->vht_caps)) {
			hdd_err("put fail");
			goto fail;
		}
	}
	if (stainfo->ht_present) {
		if (nla_put(skb, NL80211_ATTR_HT_CAPABILITY,
			    sizeof(stainfo->ht_caps),
			    &stainfo->ht_caps)) {
			hdd_err("put fail");
			goto fail;
		}
	}
	nla_nest_end(skb, nla_attr);
	return 0;
fail:
	return -EINVAL;
}

/**
 * hdd_decode_ch_width - decode channel band width based
 * @ch_width: encoded enum value holding channel band width
 *
 * This function decodes channel band width from the given encoded enum value.
 *
 * Returns: decoded channel band width.
 */
static uint8_t hdd_decode_ch_width(tSirMacHTChannelWidth ch_width)
{
	switch (ch_width) {
	case 0:
		return 20;
	case 1:
		return 40;
	case 2:
		return 80;
	case 3:
	case 4:
		return 160;
	default:
		hdd_debug("invalid enum: %d", ch_width);
		return 20;
	}
}

/**
 * hdd_get_cached_station_remote() - get cached(deleted) peer's info
 * @hdd_ctx: hdd context
 * @adapter: hostapd interface
 * @mac_addr: mac address of requested peer
 *
 * This function collect and indicate the cached(deleted) peer's info
 *
 * Return: 0 on success, otherwise error value
 */

static int hdd_get_cached_station_remote(struct hdd_context *hdd_ctx,
					 struct hdd_adapter *adapter,
					 struct qdf_mac_addr mac_addr)
{
	struct hdd_station_info *stainfo = hdd_get_stainfo(
						adapter->cache_sta_info,
						mac_addr);
	struct sk_buff *skb = NULL;
	uint32_t nl_buf_len = NLMSG_HDRLEN;
	uint8_t channel_width;

	if (!stainfo) {
		hdd_err("peer " MAC_ADDRESS_STR " not found",
			MAC_ADDR_ARRAY(mac_addr.bytes));
		return -EINVAL;
	}

	nl_buf_len += hdd_add_link_standard_info_sap_get_len() +
			hdd_add_ap_standard_info_sap_get_len(stainfo) +
			(sizeof(stainfo->dot11_mode) + NLA_HDRLEN) +
			(sizeof(stainfo->ch_width) + NLA_HDRLEN) +
			(sizeof(stainfo->tx_rate) + NLA_HDRLEN) +
			(sizeof(stainfo->rx_rate) + NLA_HDRLEN) +
			(sizeof(stainfo->support_mode) + NLA_HDRLEN);

	skb = cfg80211_vendor_cmd_alloc_reply_skb(hdd_ctx->wiphy, nl_buf_len);
	if (!skb) {
		hdd_err("cfg80211_vendor_cmd_alloc_reply_skb failed");
		return -ENOMEM;
	}

	if (hdd_add_link_standard_info_sap(skb, stainfo->rssi, stainfo,
					   LINK_INFO_STANDARD_NL80211_ATTR)) {
		hdd_err("link standard put fail");
		goto fail;
	}

	if (hdd_add_ap_standard_info_sap(skb, stainfo,
					 AP_INFO_STANDARD_NL80211_ATTR)) {
		hdd_err("ap standard put fail");
		goto fail;
	}

	/* upper layer expects decoded channel BW */
	channel_width = hdd_decode_ch_width(stainfo->ch_width);

	if (nla_put_u32(skb, REMOTE_SUPPORTED_MODE,
			stainfo->support_mode) ||
	    nla_put_u8(skb, REMOTE_CH_WIDTH, channel_width)) {
		hdd_err("remote ch put fail");
		goto fail;
	}
	if (nla_put_u32(skb, REMOTE_LAST_TX_RATE, stainfo->tx_rate)) {
		hdd_err("tx rate put fail");
		goto fail;
	}
	if (nla_put_u32(skb, REMOTE_LAST_RX_RATE, stainfo->rx_rate)) {
		hdd_err("rx rate put fail");
		goto fail;
	}
	if (nla_put_u32(skb, WLAN802_11_MODE, stainfo->dot11_mode)) {
		hdd_err("dot11 mode put fail");
		goto fail;
	}

	qdf_mem_zero(stainfo, sizeof(*stainfo));

	return cfg80211_vendor_cmd_reply(skb);
fail:
	if (skb)
		kfree_skb(skb);

	return -EINVAL;
}

/**
 * hdd_get_cached_station_remote() - get connected peer's info
 * @hdd_ctx: hdd context
 * @adapter: hostapd interface
 * @mac_addr: mac address of requested peer
 *
 * This function collect and indicate the connected peer's info
 *
 * Return: 0 on success, otherwise error value
 */
static int hdd_get_connected_station_info(struct hdd_context *hdd_ctx,
					  struct hdd_adapter *adapter,
					  struct qdf_mac_addr mac_addr,
					  struct hdd_station_info *stainfo)
{
	struct sk_buff *skb = NULL;
	uint32_t nl_buf_len;
	struct sir_peer_info_ext peer_info;
	bool txrx_rate = true;
	bool value;
	QDF_STATUS status;

	nl_buf_len = NLMSG_HDRLEN;
	nl_buf_len += (sizeof(stainfo->max_phy_rate) + NLA_HDRLEN) +
		(sizeof(stainfo->tx_packets) + NLA_HDRLEN) +
		(sizeof(stainfo->tx_bytes) + NLA_HDRLEN) +
		(sizeof(stainfo->rx_packets) + NLA_HDRLEN) +
		(sizeof(stainfo->rx_bytes) + NLA_HDRLEN) +
		(sizeof(stainfo->is_qos_enabled) + NLA_HDRLEN) +
		(sizeof(stainfo->mode) + NLA_HDRLEN);

	status = ucfg_mlme_get_sap_get_peer_info(hdd_ctx->psoc, &value);
	if (status != QDF_STATUS_SUCCESS)
		hdd_err("Unable to fetch sap ger peer info");
	if (!value ||
	    wlan_hdd_get_peer_info(adapter, mac_addr, &peer_info)) {
		hdd_err("fail to get tx/rx rate");
		txrx_rate = false;
	} else {
		stainfo->tx_rate = peer_info.tx_rate;
		stainfo->rx_rate = peer_info.rx_rate;
		nl_buf_len += (sizeof(stainfo->tx_rate) + NLA_HDRLEN) +
			(sizeof(stainfo->rx_rate) + NLA_HDRLEN);
	}

	/* below info is only valid for HT/VHT mode */
	if (stainfo->mode > SIR_SME_PHY_MODE_LEGACY)
		nl_buf_len += (sizeof(stainfo->ampdu) + NLA_HDRLEN) +
			(sizeof(stainfo->tx_stbc) + NLA_HDRLEN) +
			(sizeof(stainfo->rx_stbc) + NLA_HDRLEN) +
			(sizeof(stainfo->ch_width) + NLA_HDRLEN) +
			(sizeof(stainfo->sgi_enable) + NLA_HDRLEN);

	hdd_info("buflen %d hdrlen %d", nl_buf_len, NLMSG_HDRLEN);

	skb = cfg80211_vendor_cmd_alloc_reply_skb(hdd_ctx->wiphy,
						  nl_buf_len);
	if (!skb) {
		hdd_err("cfg80211_vendor_cmd_alloc_reply_skb failed");
		goto fail;
	}

	hdd_info("stainfo");
	hdd_info("maxrate %x tx_pkts %x tx_bytes %llx",
		 stainfo->max_phy_rate, stainfo->tx_packets,
		 stainfo->tx_bytes);
	hdd_info("rx_pkts %x rx_bytes %llx mode %x",
		 stainfo->rx_packets, stainfo->rx_bytes,
		 stainfo->mode);
	if (stainfo->mode > SIR_SME_PHY_MODE_LEGACY) {
		hdd_info("ampdu %d tx_stbc %d rx_stbc %d",
			 stainfo->ampdu, stainfo->tx_stbc,
			 stainfo->rx_stbc);
		hdd_info("wmm %d chwidth %d sgi %d",
			 stainfo->is_qos_enabled,
			 stainfo->ch_width,
			 stainfo->sgi_enable);
	}

	if (nla_put_u32(skb, REMOTE_MAX_PHY_RATE, stainfo->max_phy_rate) ||
	    nla_put_u32(skb, REMOTE_TX_PACKETS, stainfo->tx_packets) ||
	    remote_station_put_u64(skb, REMOTE_TX_BYTES, stainfo->tx_bytes) ||
	    nla_put_u32(skb, REMOTE_RX_PACKETS, stainfo->rx_packets) ||
	    remote_station_put_u64(skb, REMOTE_RX_BYTES, stainfo->rx_bytes) ||
	    nla_put_u8(skb, REMOTE_WMM, stainfo->is_qos_enabled) ||
	    nla_put_u8(skb, REMOTE_SUPPORTED_MODE, stainfo->mode)) {
		hdd_err("put fail");
		goto fail;
	}

	if (txrx_rate) {
		if (nla_put_u32(skb, REMOTE_LAST_TX_RATE, stainfo->tx_rate) ||
		    nla_put_u32(skb, REMOTE_LAST_RX_RATE, stainfo->rx_rate)) {
			hdd_err("put fail");
			goto fail;
		} else {
			hdd_info("tx_rate %x rx_rate %x",
				 stainfo->tx_rate, stainfo->rx_rate);
		}
	}

	if (stainfo->mode > SIR_SME_PHY_MODE_LEGACY) {
		if (nla_put_u8(skb, REMOTE_AMPDU, stainfo->ampdu) ||
		    nla_put_u8(skb, REMOTE_TX_STBC, stainfo->tx_stbc) ||
		    nla_put_u8(skb, REMOTE_RX_STBC, stainfo->rx_stbc) ||
		    nla_put_u8(skb, REMOTE_CH_WIDTH, stainfo->ch_width) ||
		    nla_put_u8(skb, REMOTE_SGI_ENABLE, stainfo->sgi_enable)) {
			hdd_err("put fail");
			goto fail;
		}
	}

	return cfg80211_vendor_cmd_reply(skb);

fail:
	if (skb)
		kfree_skb(skb);

	return -EINVAL;
}

/**
 * hdd_get_station_remote() - get remote peer's info
 * @hdd_ctx: hdd context
 * @adapter: hostapd interface
 * @mac_addr: mac address of requested peer
 *
 * This function collect and indicate the remote peer's info
 *
 * Return: 0 on success, otherwise error value
 */
static int hdd_get_station_remote(struct hdd_context *hdd_ctx,
				  struct hdd_adapter *adapter,
				  struct qdf_mac_addr mac_addr)
{
	struct hdd_station_info *stainfo = hdd_get_stainfo(adapter->sta_info,
							   mac_addr);
	int status = 0;
	bool is_associated = false;

	if (!stainfo) {
		status = hdd_get_cached_station_remote(hdd_ctx, adapter,
						       mac_addr);
		return status;
	}

	is_associated = hdd_is_peer_associated(adapter, &mac_addr);
	if (!is_associated) {
		status = hdd_get_cached_station_remote(hdd_ctx, adapter,
						       mac_addr);
		return status;
	}

	status = hdd_get_connected_station_info(hdd_ctx, adapter,
						mac_addr, stainfo);
	return status;
}

/**
 * __hdd_cfg80211_get_station_cmd() - Handle get station vendor cmd
 * @wiphy: corestack handler
 * @wdev: wireless device
 * @data: data
 * @data_len: data length
 *
 * Handles QCA_NL80211_VENDOR_SUBCMD_GET_STATION.
 * Validate cmd attributes and send the station info to upper layers.
 *
 * Return: Success(0) or reason code for failure
 */
static int
__hdd_cfg80211_get_station_cmd(struct wiphy *wiphy,
			       struct wireless_dev *wdev,
			       const void *data,
			       int data_len)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_GET_STATION_MAX + 1];
	int32_t status;

	hdd_enter_dev(dev);
	if (hdd_get_conparam() == QDF_GLOBAL_FTM_MODE) {
		hdd_err("Command not allowed in FTM mode");
		status = -EPERM;
		goto out;
	}

	status = wlan_hdd_validate_context(hdd_ctx);
	if (status != 0)
		goto out;

	status = wlan_cfg80211_nla_parse(tb,
					 QCA_WLAN_VENDOR_ATTR_GET_STATION_MAX,
					 data, data_len,
					 hdd_get_station_policy);
	if (status) {
		hdd_err("Invalid ATTR");
		goto out;
	}

	/* Parse and fetch Command Type*/
	if (tb[STATION_INFO]) {
		status = hdd_get_station_info(hdd_ctx, adapter);
	} else if (tb[STATION_ASSOC_FAIL_REASON]) {
		status = hdd_get_station_assoc_fail(hdd_ctx, adapter);
	} else if (tb[STATION_REMOTE]) {
		struct qdf_mac_addr mac_addr;

		if (adapter->device_mode != QDF_SAP_MODE &&
		    adapter->device_mode != QDF_P2P_GO_MODE) {
			hdd_err("invalid device_mode:%d", adapter->device_mode);
			status = -EINVAL;
			goto out;
		}

		nla_memcpy(mac_addr.bytes, tb[STATION_REMOTE],
			   QDF_MAC_ADDR_SIZE);

		hdd_debug("STATION_REMOTE " MAC_ADDRESS_STR,
			  MAC_ADDR_ARRAY(mac_addr.bytes));

		status = hdd_get_station_remote(hdd_ctx, adapter, mac_addr);
	} else {
		hdd_err("get station info cmd type failed");
		status = -EINVAL;
		goto out;
	}
	hdd_exit();
out:
	return status;
}

int32_t hdd_cfg80211_get_station_cmd(struct wiphy *wiphy,
				     struct wireless_dev *wdev,
				     const void *data,
				     int data_len)
{
	struct osif_vdev_sync *vdev_sync;
	int errno;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __hdd_cfg80211_get_station_cmd(wiphy, wdev, data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}

