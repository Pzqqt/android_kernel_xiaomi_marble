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
 * DOC : wlan_hdd_stats.c
 *
 * WLAN Host Device Driver statistics related implementation
 *
 */

#include "wlan_hdd_stats.h"
#include "sme_api.h"
#include "cds_sched.h"
#include "osif_sync.h"
#include "wlan_hdd_trace.h"
#include "wlan_hdd_lpass.h"
#include "hif.h"
#include <qca_vendor.h>
#include "wma_api.h"
#include "wlan_hdd_hostapd.h"
#include "wlan_osif_request_manager.h"
#include "wlan_hdd_debugfs_llstat.h"
#include "wlan_reg_services_api.h"
#include <wlan_cfg80211_mc_cp_stats.h>
#include "wlan_mlme_ucfg_api.h"
#include "wlan_mlme_ucfg_api.h"

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)) && !defined(WITH_BACKPORTS)
#define HDD_INFO_SIGNAL                 STATION_INFO_SIGNAL
#define HDD_INFO_SIGNAL_AVG             STATION_INFO_SIGNAL_AVG
#define HDD_INFO_TX_PACKETS             STATION_INFO_TX_PACKETS
#define HDD_INFO_TX_RETRIES             STATION_INFO_TX_RETRIES
#define HDD_INFO_TX_FAILED              STATION_INFO_TX_FAILED
#define HDD_INFO_TX_BITRATE             STATION_INFO_TX_BITRATE
#define HDD_INFO_RX_BITRATE             STATION_INFO_RX_BITRATE
#define HDD_INFO_TX_BYTES               STATION_INFO_TX_BYTES
#define HDD_INFO_CHAIN_SIGNAL_AVG       STATION_INFO_CHAIN_SIGNAL_AVG
#define HDD_INFO_RX_BYTES               STATION_INFO_RX_BYTES
#define HDD_INFO_RX_PACKETS             STATION_INFO_RX_PACKETS
#define HDD_INFO_TX_BYTES64             0
#define HDD_INFO_RX_BYTES64             0
#define HDD_INFO_INACTIVE_TIME          0
#define HDD_INFO_CONNECTED_TIME         0
#else
#define HDD_INFO_SIGNAL                 BIT(NL80211_STA_INFO_SIGNAL)
#define HDD_INFO_SIGNAL_AVG             BIT(NL80211_STA_INFO_SIGNAL_AVG)
#define HDD_INFO_TX_PACKETS             BIT(NL80211_STA_INFO_TX_PACKETS)
#define HDD_INFO_TX_RETRIES             BIT(NL80211_STA_INFO_TX_RETRIES)
#define HDD_INFO_TX_FAILED              BIT(NL80211_STA_INFO_TX_FAILED)
#define HDD_INFO_TX_BITRATE             BIT(NL80211_STA_INFO_TX_BITRATE)
#define HDD_INFO_RX_BITRATE             BIT(NL80211_STA_INFO_RX_BITRATE)
#define HDD_INFO_TX_BYTES               BIT(NL80211_STA_INFO_TX_BYTES)
#define HDD_INFO_CHAIN_SIGNAL_AVG       BIT(NL80211_STA_INFO_CHAIN_SIGNAL_AVG)
#define HDD_INFO_RX_BYTES               BIT(NL80211_STA_INFO_RX_BYTES)
#define HDD_INFO_RX_PACKETS             BIT(NL80211_STA_INFO_RX_PACKETS)
#define HDD_INFO_TX_BYTES64             BIT(NL80211_STA_INFO_TX_BYTES64)
#define HDD_INFO_RX_BYTES64             BIT(NL80211_STA_INFO_RX_BYTES64)
#define HDD_INFO_INACTIVE_TIME          BIT(NL80211_STA_INFO_INACTIVE_TIME)
#define HDD_INFO_CONNECTED_TIME         BIT(NL80211_STA_INFO_CONNECTED_TIME)
#endif /* kernel version less than 4.0.0 && no_backport */

/* 11B, 11G Rate table include Basic rate and Extended rate
 * The IDX field is the rate index
 * The HI field is the rate when RSSI is strong or being ignored
 *  (in this case we report actual rate)
 * The MID field is the rate when RSSI is moderate
 * (in this case we cap 11b rates at 5.5 and 11g rates at 24)
 * The LO field is the rate when RSSI is low
 *  (in this case we don't report rates, actual current rate used)
 */
static const struct index_data_rate_type supported_data_rate[] = {
	/* IDX     HI  HM  LM LO (RSSI-based index */
	{2,   { 10,  10, 10, 0} },
	{4,   { 20,  20, 10, 0} },
	{11,  { 55,  20, 10, 0} },
	{12,  { 60,  55, 20, 0} },
	{18,  { 90,  55, 20, 0} },
	{22,  {110,  55, 20, 0} },
	{24,  {120,  90, 60, 0} },
	{36,  {180, 120, 60, 0} },
	{44,  {220, 180, 60, 0} },
	{48,  {240, 180, 90, 0} },
	{66,  {330, 180, 90, 0} },
	{72,  {360, 240, 90, 0} },
	{96,  {480, 240, 120, 0} },
	{108, {540, 240, 120, 0} }
};
/* MCS Based rate table HT MCS parameters with Nss = 1 */
static struct index_data_rate_type supported_mcs_rate_nss1[] = {
/* MCS  L20   L40   S20  S40 */
	{0, {65, 135, 72, 150} },
	{1, {130, 270, 144, 300} },
	{2, {195, 405, 217, 450} },
	{3, {260, 540, 289, 600} },
	{4, {390, 810, 433, 900} },
	{5, {520, 1080, 578, 1200} },
	{6, {585, 1215, 650, 1350} },
	{7, {650, 1350, 722, 1500} }
};

/* HT MCS parameters with Nss = 2 */
static struct index_data_rate_type supported_mcs_rate_nss2[] = {
/* MCS  L20    L40   S20   S40 */
	{0, {130, 270, 144, 300} },
	{1, {260, 540, 289, 600} },
	{2, {390, 810, 433, 900} },
	{3, {520, 1080, 578, 1200} },
	{4, {780, 1620, 867, 1800} },
	{5, {1040, 2160, 1156, 2400} },
	{6, {1170, 2430, 1300, 2700} },
	{7, {1300, 2700, 1444, 3000} }
};

/* MCS Based VHT rate table MCS parameters with Nss = 1*/
static struct index_vht_data_rate_type supported_vht_mcs_rate_nss1[] = {
/* MCS  L80    S80     L40   S40    L20   S40*/
	{0, {293, 325}, {135, 150}, {65, 72} },
	{1, {585, 650}, {270, 300}, {130, 144} },
	{2, {878, 975}, {405, 450}, {195, 217} },
	{3, {1170, 1300}, {540, 600}, {260, 289} },
	{4, {1755, 1950}, {810, 900}, {390, 433} },
	{5, {2340, 2600}, {1080, 1200}, {520, 578} },
	{6, {2633, 2925}, {1215, 1350}, {585, 650} },
	{7, {2925, 3250}, {1350, 1500}, {650, 722} },
	{8, {3510, 3900}, {1620, 1800}, {780, 867} },
	{9, {3900, 4333}, {1800, 2000}, {780, 867} }
};

/*MCS parameters with Nss = 2*/
static struct index_vht_data_rate_type supported_vht_mcs_rate_nss2[] = {
/* MCS  L80    S80     L40   S40    L20   S40*/
	{0, {585, 650}, {270, 300}, {130, 144} },
	{1, {1170, 1300}, {540, 600}, {260, 289} },
	{2, {1755, 1950}, {810, 900}, {390, 433} },
	{3, {2340, 2600}, {1080, 1200}, {520, 578} },
	{4, {3510, 3900}, {1620, 1800}, {780, 867} },
	{5, {4680, 5200}, {2160, 2400}, {1040, 1156} },
	{6, {5265, 5850}, {2430, 2700}, {1170, 1300} },
	{7, {5850, 6500}, {2700, 3000}, {1300, 1444} },
	{8, {7020, 7800}, {3240, 3600}, {1560, 1733} },
	{9, {7800, 8667}, {3600, 4000}, {1560, 1733} }
};

/*array index ponints to MCS and array value points respective rssi*/
static int rssi_mcs_tbl[][10] = {
/*MCS 0   1     2   3    4    5    6    7    8    9*/
	{-82, -79, -77, -74, -70, -66, -65, -64, -59, -57},     /* 20 */
	{-79, -76, -74, -71, -67, -63, -62, -61, -56, -54},     /* 40 */
	{-76, -73, -71, -68, -64, -60, -59, -58, -53, -51} /* 80 */
};


#ifdef WLAN_FEATURE_LINK_LAYER_STATS

/**
 * struct hdd_ll_stats_priv - hdd link layer stats private
 * @request_id: userspace-assigned link layer stats request id
 * @request_bitmap: userspace-assigned link layer stats request bitmap
 */
struct hdd_ll_stats_priv {
	uint32_t request_id;
	uint32_t request_bitmap;
};

/*
 * Used to allocate the size of 4096 for the link layer stats.
 * The size of 4096 is considered assuming that all data per
 * respective event fit with in the limit.Please take a call
 * on the limit based on the data requirements on link layer
 * statistics.
 */
#define LL_STATS_EVENT_BUF_SIZE 4096

/**
 * put_wifi_rate_stat() - put wifi rate stats
 * @stats: Pointer to stats context
 * @vendor_event: Pointer to vendor event
 *
 * Return: bool
 */
static bool put_wifi_rate_stat(tpSirWifiRateStat stats,
			       struct sk_buff *vendor_event)
{
	if (nla_put_u8(vendor_event,
		       QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_PREAMBLE,
		       stats->rate.preamble) ||
	    nla_put_u8(vendor_event,
		       QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_NSS,
		       stats->rate.nss) ||
	    nla_put_u8(vendor_event,
		       QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_BW,
		       stats->rate.bw) ||
	    nla_put_u8(vendor_event,
		       QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_MCS_INDEX,
		       stats->rate.rateMcsIdx) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_BIT_RATE,
			stats->rate.bitrate) ||
	    nla_put_u32(vendor_event,
			   QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_TX_MPDU,
			   stats->txMpdu) ||
	    nla_put_u32(vendor_event,
			   QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_RX_MPDU,
			   stats->rxMpdu) ||
	    nla_put_u32(vendor_event,
			   QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_MPDU_LOST,
			   stats->mpduLost) ||
	    nla_put_u32(vendor_event,
			   QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_RETRIES,
			   stats->retries) ||
	    nla_put_u32(vendor_event,
			   QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_RETRIES_SHORT,
			   stats->retriesShort) ||
	    nla_put_u32(vendor_event,
			   QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_RETRIES_LONG,
			   stats->retriesLong)) {
		hdd_err("QCA_WLAN_VENDOR_ATTR put fail");
		return false;
	}

	return true;
}

/**
 * put_wifi_peer_info() - put wifi peer info
 * @stats: Pointer to stats context
 * @vendor_event: Pointer to vendor event
 *
 * Return: bool
 */
static bool put_wifi_peer_info(tpSirWifiPeerInfo stats,
			       struct sk_buff *vendor_event)
{
	u32 i = 0;
	tpSirWifiRateStat rate_stat;

	if (nla_put_u32
		    (vendor_event, QCA_WLAN_VENDOR_ATTR_LL_STATS_PEER_INFO_TYPE,
		    wmi_to_sir_peer_type(stats->type)) ||
	    nla_put(vendor_event,
		       QCA_WLAN_VENDOR_ATTR_LL_STATS_PEER_INFO_MAC_ADDRESS,
		       QDF_MAC_ADDR_SIZE, &stats->peerMacAddress.bytes[0]) ||
	    nla_put_u32(vendor_event,
			   QCA_WLAN_VENDOR_ATTR_LL_STATS_PEER_INFO_CAPABILITIES,
			   stats->capabilities) ||
	    nla_put_u32(vendor_event,
			   QCA_WLAN_VENDOR_ATTR_LL_STATS_PEER_INFO_NUM_RATES,
			   stats->numRate)) {
		hdd_err("QCA_WLAN_VENDOR_ATTR put fail");
		goto error;
	}

	if (stats->numRate) {
		struct nlattr *rateInfo;
		struct nlattr *rates;

		rateInfo = nla_nest_start(vendor_event,
					  QCA_WLAN_VENDOR_ATTR_LL_STATS_PEER_INFO_RATE_INFO);
		if (!rateInfo)
			goto error;

		for (i = 0; i < stats->numRate; i++) {
			rate_stat = (tpSirWifiRateStat) ((uint8_t *)
							  stats->rateStats +
							  (i *
							   sizeof
							   (tSirWifiRateStat)));
			rates = nla_nest_start(vendor_event, i);
			if (!rates)
				goto error;

			if (!put_wifi_rate_stat(rate_stat, vendor_event)) {
				hdd_err("QCA_WLAN_VENDOR_ATTR put fail");
				return false;
			}
			nla_nest_end(vendor_event, rates);
		}
		nla_nest_end(vendor_event, rateInfo);
	}

	return true;
error:
	return false;
}

/**
 * put_wifi_wmm_ac_stat() - put wifi wmm ac stats
 * @stats: Pointer to stats context
 * @vendor_event: Pointer to vendor event
 *
 * Return: bool
 */
static bool put_wifi_wmm_ac_stat(wmi_wmm_ac_stats *stats,
				 struct sk_buff *vendor_event)
{
	if (nla_put_u32(vendor_event, QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_AC,
			stats->ac_type) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_TX_MPDU,
			stats->tx_mpdu) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_RX_MPDU,
			stats->rx_mpdu) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_TX_MCAST,
			stats->tx_mcast) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_RX_MCAST,
			stats->rx_mcast) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_RX_AMPDU,
			stats->rx_ampdu) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_TX_AMPDU,
			stats->tx_ampdu) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_MPDU_LOST,
			stats->mpdu_lost) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_RETRIES,
			stats->retries) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_RETRIES_SHORT,
			stats->retries_short) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_RETRIES_LONG,
			stats->retries_long) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_CONTENTION_TIME_MIN,
			stats->contention_time_min) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_CONTENTION_TIME_MAX,
			stats->contention_time_max) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_CONTENTION_TIME_AVG,
			stats->contention_time_avg) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_CONTENTION_NUM_SAMPLES,
			stats->contention_num_samples)) {
		hdd_err("QCA_WLAN_VENDOR_ATTR put fail");
		return false;
	}

	return true;
}

/**
 * put_wifi_interface_info() - put wifi interface info
 * @stats: Pointer to stats context
 * @vendor_event: Pointer to vendor event
 *
 * Return: bool
 */
static bool put_wifi_interface_info(struct wifi_interface_info *stats,
				    struct sk_buff *vendor_event)
{
	if (nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_MODE,
			stats->mode) ||
	    nla_put(vendor_event,
		    QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_MAC_ADDR,
		    QDF_MAC_ADDR_SIZE, stats->macAddr.bytes) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_STATE,
			stats->state) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_ROAMING,
			stats->roaming) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_CAPABILITIES,
			stats->capabilities) ||
	    nla_put(vendor_event,
		    QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_SSID,
		    strlen(stats->ssid), stats->ssid) ||
	    nla_put(vendor_event,
		    QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_BSSID,
		    QDF_MAC_ADDR_SIZE, stats->bssid.bytes) ||
	    nla_put(vendor_event,
		    QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_AP_COUNTRY_STR,
		    CFG_COUNTRY_CODE_LEN, stats->apCountryStr) ||
	    nla_put(vendor_event,
		    QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_COUNTRY_STR,
		    CFG_COUNTRY_CODE_LEN, stats->countryStr)) {
		hdd_err("QCA_WLAN_VENDOR_ATTR put fail");
		return false;
	}

	return true;
}

/**
 * put_wifi_iface_stats() - put wifi interface stats
 * @if_stat: Pointer to interface stats context
 * @num_peer: Number of peers
 * @vendor_event: Pointer to vendor event
 *
 * Return: bool
 */
static bool put_wifi_iface_stats(tpSirWifiIfaceStat if_stat,
				 u32 num_peers, struct sk_buff *vendor_event)
{
	int i = 0;
	struct nlattr *wmm_info;
	struct nlattr *wmm_stats;
	u64 average_tsf_offset;
	wmi_iface_link_stats *link_stats = &if_stat->link_stats;

	if (!put_wifi_interface_info(&if_stat->info, vendor_event)) {
		hdd_err("QCA_WLAN_VENDOR_ATTR put fail");
		return false;

	}

	average_tsf_offset =  link_stats->avg_bcn_spread_offset_high;
	average_tsf_offset =  (average_tsf_offset << 32) |
		link_stats->avg_bcn_spread_offset_low;

	if (nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE_IFACE) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_NUM_PEERS,
			num_peers) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_BEACON_RX,
			link_stats->beacon_rx) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_MGMT_RX,
			link_stats->mgmt_rx) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_MGMT_ACTION_RX,
			link_stats->mgmt_action_rx) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_MGMT_ACTION_TX,
			link_stats->mgmt_action_tx) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_RSSI_MGMT,
			link_stats->rssi_mgmt) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_RSSI_DATA,
			link_stats->rssi_data) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_RSSI_ACK,
			link_stats->rssi_ack) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_LEAKY_AP_DETECTED,
			link_stats->is_leaky_ap) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_LEAKY_AP_AVG_NUM_FRAMES_LEAKED,
			link_stats->avg_rx_frms_leaked) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_LEAKY_AP_GUARD_TIME,
			link_stats->rx_leak_window) ||
	    hdd_wlan_nla_put_u64(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_AVERAGE_TSF_OFFSET,
			average_tsf_offset) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_RTS_SUCC_CNT,
			if_stat->rts_succ_cnt) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_RTS_FAIL_CNT,
			if_stat->rts_fail_cnt) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_PPDU_SUCC_CNT,
			if_stat->ppdu_succ_cnt) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_PPDU_FAIL_CNT,
			if_stat->ppdu_fail_cnt)) {
		hdd_err("QCA_WLAN_VENDOR_ATTR put fail");
		return false;
	}

	wmm_info = nla_nest_start(vendor_event,
				  QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_INFO);
	if (!wmm_info)
		return false;

	for (i = 0; i < WIFI_AC_MAX; i++) {
		wmm_stats = nla_nest_start(vendor_event, i);
		if (!wmm_stats)
			return false;

		if (!put_wifi_wmm_ac_stat(&if_stat->ac_stats[i],
					  vendor_event)) {
			hdd_err("put_wifi_wmm_ac_stat Fail");
			return false;
		}

		nla_nest_end(vendor_event, wmm_stats);
	}
	nla_nest_end(vendor_event, wmm_info);
	return true;
}

/**
 * hdd_map_device_to_ll_iface_mode() - map device to link layer interface mode
 * @deviceMode: Device mode
 *
 * Return: interface mode
 */
static tSirWifiInterfaceMode hdd_map_device_to_ll_iface_mode(int deviceMode)
{
	switch (deviceMode) {
	case QDF_STA_MODE:
		return WIFI_INTERFACE_STA;
	case QDF_SAP_MODE:
		return WIFI_INTERFACE_SOFTAP;
	case QDF_P2P_CLIENT_MODE:
		return WIFI_INTERFACE_P2P_CLIENT;
	case QDF_P2P_GO_MODE:
		return WIFI_INTERFACE_P2P_GO;
	case QDF_IBSS_MODE:
		return WIFI_INTERFACE_IBSS;
	default:
		/* Return Interface Mode as STA for all the unsupported modes */
		return WIFI_INTERFACE_STA;
	}
}

bool hdd_get_interface_info(struct hdd_adapter *adapter,
			    struct wifi_interface_info *pInfo)
{
	uint8_t *staMac = NULL;
	struct hdd_station_ctx *sta_ctx;
	mac_handle_t mac_handle = adapter->hdd_ctx->mac_handle;
	/* pre-existing layering violation */
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	pInfo->mode = hdd_map_device_to_ll_iface_mode(adapter->device_mode);

	qdf_copy_macaddr(&pInfo->macAddr, &adapter->mac_addr);

	if (((QDF_STA_MODE == adapter->device_mode) ||
	     (QDF_P2P_CLIENT_MODE == adapter->device_mode) ||
	     (QDF_P2P_DEVICE_MODE == adapter->device_mode))) {
		sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
		if (eConnectionState_NotConnected ==
		    sta_ctx->conn_info.conn_state) {
			pInfo->state = WIFI_DISCONNECTED;
		}
		if (eConnectionState_Connecting ==
		    sta_ctx->conn_info.conn_state) {
			hdd_err("Session ID %d, Connection is in progress",
				adapter->vdev_id);
			pInfo->state = WIFI_ASSOCIATING;
		}
		if ((eConnectionState_Associated ==
		     sta_ctx->conn_info.conn_state) &&
		    (!sta_ctx->conn_info.is_authenticated)) {
			staMac =
				(uint8_t *) &(adapter->mac_addr.
					      bytes[0]);
			hdd_err("client " MAC_ADDRESS_STR
				" is in the middle of WPS/EAPOL exchange.",
				MAC_ADDR_ARRAY(staMac));
			pInfo->state = WIFI_AUTHENTICATING;
		}
		if (eConnectionState_Associated ==
		    sta_ctx->conn_info.conn_state) {
			pInfo->state = WIFI_ASSOCIATED;
			qdf_copy_macaddr(&pInfo->bssid,
					 &sta_ctx->conn_info.bssid);
			qdf_mem_copy(pInfo->ssid,
				     sta_ctx->conn_info.ssid.SSID.ssId,
				     sta_ctx->conn_info.ssid.SSID.length);
			/*
			 * NULL Terminate the string
			 */
			pInfo->ssid[sta_ctx->conn_info.ssid.SSID.length] = 0;
		}
	}

	qdf_mem_copy(pInfo->countryStr,
		     mac->scan.countryCodeCurrent, CFG_COUNTRY_CODE_LEN);

	qdf_mem_copy(pInfo->apCountryStr,
		     mac->scan.countryCodeCurrent, CFG_COUNTRY_CODE_LEN);

	return true;
}

/**
 * hdd_link_layer_process_peer_stats() - This function is called after
 * @adapter: Pointer to device adapter
 * @more_data: More data
 * @pData: Pointer to stats data
 *
 * Receiving Link Layer Peer statistics from FW.This function converts
 * the firmware data to the NL data and sends the same to the kernel/upper
 * layers.
 *
 * Return: None
 */
static void hdd_link_layer_process_peer_stats(struct hdd_adapter *adapter,
					      u32 more_data,
					      tpSirWifiPeerStat pData)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	tpSirWifiPeerStat pWifiPeerStat;
	tpSirWifiPeerInfo peer_info;
	struct sk_buff *vendor_event;
	int status, i;
	struct nlattr *peers;
	int numRate;

	hdd_enter();

	pWifiPeerStat = pData;

	status = wlan_hdd_validate_context(hdd_ctx);
	if (0 != status)
		return;

	hdd_debug("LL_STATS_PEER_ALL : numPeers %u, more data = %u",
		   pWifiPeerStat->numPeers, more_data);

	/*
	 * Allocate a size of 4096 for the peer stats comprising
	 * each of size = sizeof (tSirWifiPeerInfo) + numRate *
	 * sizeof (tSirWifiRateStat).Each field is put with an
	 * NL attribute.The size of 4096 is considered assuming
	 * that number of rates shall not exceed beyond 50 with
	 * the sizeof (tSirWifiRateStat) being 32.
	 */
	vendor_event = cfg80211_vendor_cmd_alloc_reply_skb(hdd_ctx->wiphy,
				LL_STATS_EVENT_BUF_SIZE);

	if (!vendor_event) {
		hdd_err("cfg80211_vendor_cmd_alloc_reply_skb failed");
		return;
	}

	if (nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE_PEER) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_RESULTS_MORE_DATA,
			more_data) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_NUM_PEERS,
			pWifiPeerStat->numPeers)) {
		hdd_err("QCA_WLAN_VENDOR_ATTR put fail");

		kfree_skb(vendor_event);
		return;
	}

	peer_info = (tpSirWifiPeerInfo) ((uint8_t *)
					     pWifiPeerStat->peerInfo);

	if (pWifiPeerStat->numPeers) {
		struct nlattr *peerInfo;

		peerInfo = nla_nest_start(vendor_event,
					  QCA_WLAN_VENDOR_ATTR_LL_STATS_PEER_INFO);
		if (!peerInfo) {
			hdd_err("nla_nest_start failed");
			kfree_skb(vendor_event);
			return;
		}

		for (i = 1; i <= pWifiPeerStat->numPeers; i++) {
			peers = nla_nest_start(vendor_event, i);
			if (!peers) {
				hdd_err("nla_nest_start failed");
				kfree_skb(vendor_event);
				return;
			}

			numRate = peer_info->numRate;

			if (!put_wifi_peer_info(peer_info, vendor_event)) {
				hdd_err("put_wifi_peer_info fail");
				kfree_skb(vendor_event);
				return;
			}

			peer_info = (tpSirWifiPeerInfo) ((uint8_t *)
							     pWifiPeerStat->
							     peerInfo +
							     (i *
							      sizeof
							      (tSirWifiPeerInfo))
							     +
							     (numRate *
							      sizeof
							      (tSirWifiRateStat)));
			nla_nest_end(vendor_event, peers);
		}
		nla_nest_end(vendor_event, peerInfo);
	}

	cfg80211_vendor_cmd_reply(vendor_event);
	hdd_exit();
}

/**
 * hdd_link_layer_process_iface_stats() - This function is called after
 * @adapter: Pointer to device adapter
 * @pData: Pointer to stats data
 * @num_peers: Number of peers
 *
 * Receiving Link Layer Interface statistics from FW.This function converts
 * the firmware data to the NL data and sends the same to the kernel/upper
 * layers.
 *
 * Return: None
 */
static void hdd_link_layer_process_iface_stats(struct hdd_adapter *adapter,
					       tpSirWifiIfaceStat pData,
					       u32 num_peers)
{
	tpSirWifiIfaceStat if_stat;
	struct sk_buff *vendor_event;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	int status;

	hdd_enter();

	if_stat = pData;

	status = wlan_hdd_validate_context(hdd_ctx);
	if (0 != status)
		return;

	/*
	 * Allocate a size of 4096 for the interface stats comprising
	 * sizeof (tpSirWifiIfaceStat).The size of 4096 is considered
	 * assuming that all these fit with in the limit.Please take
	 * a call on the limit based on the data requirements on
	 * interface statistics.
	 */
	vendor_event = cfg80211_vendor_cmd_alloc_reply_skb(hdd_ctx->wiphy,
				LL_STATS_EVENT_BUF_SIZE);

	if (!vendor_event) {
		hdd_err("cfg80211_vendor_cmd_alloc_reply_skb failed");
		return;
	}

	hdd_debug("WMI_LINK_STATS_IFACE Data");

	if (!hdd_get_interface_info(adapter, &if_stat->info)) {
		hdd_err("hdd_get_interface_info get fail");
		kfree_skb(vendor_event);
		return;
	}

	if (!put_wifi_iface_stats(if_stat, num_peers, vendor_event)) {
		hdd_err("put_wifi_iface_stats fail");
		kfree_skb(vendor_event);
		return;
	}

	cfg80211_vendor_cmd_reply(vendor_event);
	hdd_exit();
}

/**
 * hdd_llstats_radio_fill_channels() - radio stats fill channels
 * @adapter: Pointer to device adapter
 * @radiostat: Pointer to stats data
 * @vendor_event: vendor event
 *
 * Return: 0 on success; errno on failure
 */
static int hdd_llstats_radio_fill_channels(struct hdd_adapter *adapter,
					   tSirWifiRadioStat *radiostat,
					   struct sk_buff *vendor_event)
{
	tSirWifiChannelStats *channel_stats;
	struct nlattr *chlist;
	struct nlattr *chinfo;
	int i;

	chlist = nla_nest_start(vendor_event,
				QCA_WLAN_VENDOR_ATTR_LL_STATS_CH_INFO);
	if (!chlist) {
		hdd_err("nla_nest_start failed");
		return -EINVAL;
	}

	for (i = 0; i < radiostat->numChannels; i++) {
		channel_stats = (tSirWifiChannelStats *) ((uint8_t *)
				     radiostat->channels +
				     (i * sizeof(tSirWifiChannelStats)));

		chinfo = nla_nest_start(vendor_event, i);
		if (!chinfo) {
			hdd_err("nla_nest_start failed");
			return -EINVAL;
		}

		if (nla_put_u32(vendor_event,
				QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_INFO_WIDTH,
				channel_stats->channel.width) ||
		    nla_put_u32(vendor_event,
				QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_INFO_CENTER_FREQ,
				channel_stats->channel.centerFreq) ||
		    nla_put_u32(vendor_event,
				QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_INFO_CENTER_FREQ0,
				channel_stats->channel.centerFreq0) ||
		    nla_put_u32(vendor_event,
				QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_INFO_CENTER_FREQ1,
				channel_stats->channel.centerFreq1) ||
		    nla_put_u32(vendor_event,
				QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_ON_TIME,
				channel_stats->onTime) ||
		    nla_put_u32(vendor_event,
				QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_CCA_BUSY_TIME,
				channel_stats->ccaBusyTime)) {
			hdd_err("nla_put failed");
			return -EINVAL;
		}
		nla_nest_end(vendor_event, chinfo);
	}
	nla_nest_end(vendor_event, chlist);

	return 0;
}

/**
 * hdd_llstats_post_radio_stats() - post radio stats
 * @adapter: Pointer to device adapter
 * @more_data: More data
 * @radiostat: Pointer to stats data
 * @num_radio: Number of radios
 *
 * Return: 0 on success; errno on failure
 */
static int hdd_llstats_post_radio_stats(struct hdd_adapter *adapter,
					u32 more_data,
					tSirWifiRadioStat *radiostat,
					u32 num_radio)
{
	struct sk_buff *vendor_event;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	int ret;

	/*
	 * Allocate a size of 4096 for the Radio stats comprising
	 * sizeof (tSirWifiRadioStat) + numChannels * sizeof
	 * (tSirWifiChannelStats).Each channel data is put with an
	 * NL attribute.The size of 4096 is considered assuming that
	 * number of channels shall not exceed beyond  60 with the
	 * sizeof (tSirWifiChannelStats) being 24 bytes.
	 */

	vendor_event = cfg80211_vendor_cmd_alloc_reply_skb(
					hdd_ctx->wiphy,
					LL_STATS_EVENT_BUF_SIZE);

	if (!vendor_event) {
		hdd_err("cfg80211_vendor_cmd_alloc_reply_skb failed");
		return -ENOMEM;
	}

	if (nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE_RADIO) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_RESULTS_MORE_DATA,
			more_data) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_NUM_RADIOS,
			num_radio) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ID,
			radiostat->radio) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME,
			radiostat->onTime) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_TX_TIME,
			radiostat->txTime) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_RX_TIME,
			radiostat->rxTime) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME_SCAN,
			radiostat->onTimeScan) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME_NBD,
			radiostat->onTimeNbd) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME_GSCAN,
			radiostat->onTimeGscan) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME_ROAM_SCAN,
			radiostat->onTimeRoamScan) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME_PNO_SCAN,
			radiostat->onTimePnoScan) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME_HS20,
			radiostat->onTimeHs20) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_NUM_TX_LEVELS,
			radiostat->total_num_tx_power_levels)    ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_NUM_CHANNELS,
			radiostat->numChannels)) {
		hdd_err("QCA_WLAN_VENDOR_ATTR put fail");
		goto failure;
	}

	if (radiostat->total_num_tx_power_levels) {
		if (nla_put(vendor_event,
			    QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_TX_TIME_PER_LEVEL,
			    sizeof(u32) *
			    radiostat->total_num_tx_power_levels,
			    radiostat->tx_time_per_power_level)) {
			hdd_err("nla_put fail");
			goto failure;
		}
	}

	if (radiostat->numChannels) {
		ret = hdd_llstats_radio_fill_channels(adapter, radiostat,
						      vendor_event);
		if (ret)
			goto failure;
	}

	cfg80211_vendor_cmd_reply(vendor_event);
	return 0;

failure:
	kfree_skb(vendor_event);
	return -EINVAL;
}

/**
 * hdd_link_layer_process_radio_stats() - This function is called after
 * @adapter: Pointer to device adapter
 * @more_data: More data
 * @pData: Pointer to stats data
 * @num_radios: Number of radios
 *
 * Receiving Link Layer Radio statistics from FW.This function converts
 * the firmware data to the NL data and sends the same to the kernel/upper
 * layers.
 *
 * Return: None
 */
static void hdd_link_layer_process_radio_stats(struct hdd_adapter *adapter,
					       u32 more_data,
					       tpSirWifiRadioStat pData,
					       u32 num_radio)
{
	int status, i, nr, ret;
	tSirWifiRadioStat *pWifiRadioStat = pData;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	hdd_enter();

	status = wlan_hdd_validate_context(hdd_ctx);
	if (0 != status)
		return;

	hdd_debug("LL_STATS_RADIO: number of radios: %u", num_radio);

	for (i = 0; i < num_radio; i++) {
		hdd_debug("LL_STATS_RADIO"
		       " radio: %u onTime: %u txTime: %u rxTime: %u"
		       " onTimeScan: %u onTimeNbd: %u"
		       " onTimeGscan: %u onTimeRoamScan: %u"
		       " onTimePnoScan: %u  onTimeHs20: %u"
		       " numChannels: %u total_num_tx_pwr_levels: %u"
		       " on_time_host_scan: %u, on_time_lpi_scan: %u",
		       pWifiRadioStat->radio, pWifiRadioStat->onTime,
		       pWifiRadioStat->txTime, pWifiRadioStat->rxTime,
		       pWifiRadioStat->onTimeScan, pWifiRadioStat->onTimeNbd,
		       pWifiRadioStat->onTimeGscan,
		       pWifiRadioStat->onTimeRoamScan,
		       pWifiRadioStat->onTimePnoScan,
		       pWifiRadioStat->onTimeHs20, pWifiRadioStat->numChannels,
		       pWifiRadioStat->total_num_tx_power_levels,
		       pWifiRadioStat->on_time_host_scan,
		       pWifiRadioStat->on_time_lpi_scan);
		pWifiRadioStat++;
	}

	pWifiRadioStat = pData;
	for (nr = 0; nr < num_radio; nr++) {
		ret = hdd_llstats_post_radio_stats(adapter, more_data,
						   pWifiRadioStat, num_radio);
		if (ret)
			return;

		pWifiRadioStat++;
	}

	hdd_exit();
}

/**
 * hdd_ll_process_radio_stats() - Wrapper function for cfg80211/debugfs
 * @adapter: Pointer to device adapter
 * @more_data: More data
 * @data: Pointer to stats data
 * @num_radios: Number of radios
 * @resp_id: Response ID from FW
 *
 * Receiving Link Layer Radio statistics from FW. This function is a wrapper
 * function which calls cfg80211/debugfs functions based on the response ID.
 *
 * Return: None
 */
static void hdd_ll_process_radio_stats(struct hdd_adapter *adapter,
		uint32_t more_data, void *data, uint32_t num_radio,
		uint32_t resp_id)
{
	if (DEBUGFS_LLSTATS_REQID == resp_id)
		hdd_debugfs_process_radio_stats(adapter, more_data,
			(tpSirWifiRadioStat)data, num_radio);
	else
		hdd_link_layer_process_radio_stats(adapter, more_data,
			(tpSirWifiRadioStat)data, num_radio);
}

/**
 * hdd_ll_process_iface_stats() - Wrapper function for cfg80211/debugfs
 * @adapter: Pointer to device adapter
 * @data: Pointer to stats data
 * @num_peers: Number of peers
 * @resp_id: Response ID from FW
 *
 * Receiving Link Layer Radio statistics from FW. This function is a wrapper
 * function which calls cfg80211/debugfs functions based on the response ID.
 *
 * Return: None
 */
static void hdd_ll_process_iface_stats(struct hdd_adapter *adapter,
			void *data, uint32_t num_peers, uint32_t resp_id)
{
	if (DEBUGFS_LLSTATS_REQID == resp_id)
		hdd_debugfs_process_iface_stats(adapter,
				(tpSirWifiIfaceStat) data, num_peers);
	else
		hdd_link_layer_process_iface_stats(adapter,
				(tpSirWifiIfaceStat) data, num_peers);
}

/**
 * hdd_ll_process_peer_stats() - Wrapper function for cfg80211/debugfs
 * @adapter: Pointer to device adapter
 * @more_data: More data
 * @data: Pointer to stats data
 * @resp_id: Response ID from FW
 *
 * Receiving Link Layer Radio statistics from FW. This function is a wrapper
 * function which calls cfg80211/debugfs functions based on the response ID.
 *
 * Return: None
 */
static void hdd_ll_process_peer_stats(struct hdd_adapter *adapter,
		uint32_t more_data, void *data, uint32_t resp_id)
{
	if (DEBUGFS_LLSTATS_REQID == resp_id)
		hdd_debugfs_process_peer_stats(adapter, data);
	else
		hdd_link_layer_process_peer_stats(adapter, more_data,
						  (tpSirWifiPeerStat) data);
}

void wlan_hdd_cfg80211_link_layer_stats_callback(hdd_handle_t hdd_handle,
						 int indication_type,
						 tSirLLStatsResults *results,
						 void *cookie)
{
	struct hdd_context *hdd_ctx = hdd_handle_to_context(hdd_handle);
	struct hdd_ll_stats_priv *priv;
	struct hdd_adapter *adapter = NULL;
	int status;
	struct osif_request *request;

	status = wlan_hdd_validate_context(hdd_ctx);
	if (status)
		return;

	adapter = hdd_get_adapter_by_vdev(hdd_ctx,
					   results->ifaceId);

	if (!adapter) {
		hdd_err("vdev_id %d does not exist with host",
			results->ifaceId);
		return;
	}

	hdd_debug("Link Layer Indication Type: %d", indication_type);

	switch (indication_type) {
	case SIR_HAL_LL_STATS_RESULTS_RSP:
	{
		hdd_debug("LL_STATS RESP paramID = 0x%x, ifaceId = %u, respId= %u , moreResultToFollow = %u, num radio = %u result = %pK",
			results->paramId,
			results->ifaceId,
			results->rspId,
			results->moreResultToFollow,
			results->num_radio,
			results->results);

		request = osif_request_get(cookie);
		if (!request) {
			hdd_err("Obsolete request");
			return;
		}

		priv = osif_request_priv(request);

		/* validate response received from target */
		if ((priv->request_id != results->rspId) ||
		    !(priv->request_bitmap & results->paramId)) {
			hdd_err("Request id %d response id %d request bitmap 0x%x response bitmap 0x%x",
				priv->request_id, results->rspId,
				priv->request_bitmap, results->paramId);
			osif_request_put(request);
			return;
		}

		if (results->paramId & WMI_LINK_STATS_RADIO) {
			hdd_ll_process_radio_stats(adapter,
				results->moreResultToFollow,
				results->results,
				results->num_radio,
				results->rspId);

			if (!results->moreResultToFollow)
				priv->request_bitmap &= ~(WMI_LINK_STATS_RADIO);

		} else if (results->paramId &
				WMI_LINK_STATS_IFACE) {
			hdd_ll_process_iface_stats(adapter,
				results->results,
				results->num_peers,
				results->rspId);

			/* Firmware doesn't send peerstats event if no peers are
			 * connected. HDD should not wait for any peerstats in
			 * this case and return the status to middleware after
			 * receiving iface stats
			 */
			if (!results->num_peers)
				priv->request_bitmap &=
					~(WMI_LINK_STATS_ALL_PEER);
			priv->request_bitmap &= ~(WMI_LINK_STATS_IFACE);

		} else if (results->
			   paramId & WMI_LINK_STATS_ALL_PEER) {
			hdd_ll_process_peer_stats(adapter,
				results->moreResultToFollow,
				results->results,
				results->rspId);

			if (!results->moreResultToFollow)
				priv->request_bitmap &=
						~(WMI_LINK_STATS_ALL_PEER);

		} else {
			hdd_err("INVALID LL_STATS_NOTIFY RESPONSE");
		}

		/* complete response event if all requests are completed */
		if (!priv->request_bitmap)
			osif_request_complete(request);

		osif_request_put(request);
		break;
	}
	default:
		hdd_warn("invalid event type %d", indication_type);
		break;
	}
}

void hdd_lost_link_info_cb(hdd_handle_t hdd_handle,
			   struct sir_lost_link_info *lost_link_info)
{
	struct hdd_context *hdd_ctx = hdd_handle_to_context(hdd_handle);
	int status;
	struct hdd_adapter *adapter;

	status = wlan_hdd_validate_context(hdd_ctx);
	if (status)
		return;

	if (!lost_link_info) {
		hdd_err("lost_link_info is NULL");
		return;
	}

	adapter = hdd_get_adapter_by_vdev(hdd_ctx, lost_link_info->vdev_id);
	if (!adapter) {
		hdd_err("invalid adapter");
		return;
	}

	adapter->rssi_on_disconnect = lost_link_info->rssi;
	hdd_debug("rssi on disconnect %d", adapter->rssi_on_disconnect);
}

const struct
nla_policy
	qca_wlan_vendor_ll_set_policy[QCA_WLAN_VENDOR_ATTR_LL_STATS_SET_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_SET_CONFIG_MPDU_SIZE_THRESHOLD] = {
						.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_SET_CONFIG_AGGRESSIVE_STATS_GATHERING] = {
						.type = NLA_U32},
};

/**
 * __wlan_hdd_cfg80211_ll_stats_set() - set link layer stats
 * @wiphy: Pointer to wiphy
 * @wdev: Pointer to wdev
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: int
 */
static int
__wlan_hdd_cfg80211_ll_stats_set(struct wiphy *wiphy,
				   struct wireless_dev *wdev,
				   const void *data,
				   int data_len)
{
	int status;
	struct nlattr *tb_vendor[QCA_WLAN_VENDOR_ATTR_LL_STATS_SET_MAX + 1];
	tSirLLStatsSetReq req;
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);

	hdd_enter_dev(dev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	status = wlan_hdd_validate_context(hdd_ctx);
	if (0 != status)
		return -EINVAL;

	if (hdd_validate_adapter(adapter))
		return -EINVAL;

	if (adapter->device_mode != QDF_STA_MODE) {
		hdd_debug("Cannot set LL_STATS for device mode %d",
			  adapter->device_mode);
		return -EINVAL;
	}

	if (wlan_cfg80211_nla_parse(tb_vendor,
				    QCA_WLAN_VENDOR_ATTR_LL_STATS_SET_MAX,
				    (struct nlattr *)data, data_len,
				    qca_wlan_vendor_ll_set_policy)) {
		hdd_err("maximum attribute not present");
		return -EINVAL;
	}

	if (!tb_vendor
	    [QCA_WLAN_VENDOR_ATTR_LL_STATS_SET_CONFIG_MPDU_SIZE_THRESHOLD]) {
		hdd_err("MPDU size Not present");
		return -EINVAL;
	}

	if (!tb_vendor
	    [QCA_WLAN_VENDOR_ATTR_LL_STATS_SET_CONFIG_AGGRESSIVE_STATS_GATHERING]) {
		hdd_err("Stats Gathering Not Present");
		return -EINVAL;
	}

	/* Shall take the request Id if the Upper layers pass. 1 For now. */
	req.reqId = 1;

	req.mpduSizeThreshold =
		nla_get_u32(tb_vendor
			    [QCA_WLAN_VENDOR_ATTR_LL_STATS_SET_CONFIG_MPDU_SIZE_THRESHOLD]);

	req.aggressiveStatisticsGathering =
		nla_get_u32(tb_vendor
			    [QCA_WLAN_VENDOR_ATTR_LL_STATS_SET_CONFIG_AGGRESSIVE_STATS_GATHERING]);

	req.staId = adapter->vdev_id;

	hdd_debug("LL_STATS_SET reqId = %d, staId = %d, mpduSizeThreshold = %d, Statistics Gathering = %d",
		req.reqId, req.staId,
		req.mpduSizeThreshold,
		req.aggressiveStatisticsGathering);

	if (QDF_STATUS_SUCCESS != sme_ll_stats_set_req(hdd_ctx->mac_handle,
						       &req)) {
		hdd_err("sme_ll_stats_set_req Failed");
		return -EINVAL;
	}

	adapter->is_link_layer_stats_set = true;
	hdd_exit();
	return 0;
}

/**
 * wlan_hdd_cfg80211_ll_stats_set() - set ll stats
 * @wiphy: Pointer to wiphy
 * @wdev: Pointer to wdev
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: 0 if success, non-zero for failure
 */
int wlan_hdd_cfg80211_ll_stats_set(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					const void *data,
					int data_len)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_ll_stats_set(wiphy, wdev, data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}

const struct
nla_policy
	qca_wlan_vendor_ll_get_policy[QCA_WLAN_VENDOR_ATTR_LL_STATS_GET_MAX + 1] = {
	/* Unsigned 32bit value provided by the caller issuing the GET stats
	 * command. When reporting
	 * the stats results, the driver uses the same value to indicate
	 * which GET request the results
	 * correspond to.
	 */
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_GET_CONFIG_REQ_ID] = {.type = NLA_U32},

	/* Unsigned 32bit value . bit mask to identify what statistics are
	 * requested for retrieval
	 */
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_GET_CONFIG_REQ_MASK] = {.type = NLA_U32}
};

static int wlan_hdd_send_ll_stats_req(struct hdd_context *hdd_ctx,
				      tSirLLStatsGetReq *req)
{
	int ret;
	struct hdd_ll_stats_priv *priv;
	struct osif_request *request;
	void *cookie;
	static const struct osif_request_params params = {
		.priv_size = sizeof(*priv),
		.timeout_ms = WLAN_WAIT_TIME_LL_STATS,
	};

	hdd_enter();

	request = osif_request_alloc(&params);
	if (!request) {
		hdd_err("Request Allocation Failure");
		return -ENOMEM;
	}

	cookie = osif_request_cookie(request);

	priv = osif_request_priv(request);

	priv->request_id = req->reqId;
	priv->request_bitmap = req->paramIdMask;

	if (QDF_STATUS_SUCCESS !=
			sme_ll_stats_get_req(hdd_ctx->mac_handle, req,
					     cookie)) {
		hdd_err("sme_ll_stats_get_req Failed");
		ret = -EINVAL;
		goto exit;
	}

	ret = osif_request_wait_for_response(request);
	if (ret) {
		hdd_err("Target response timed out request id %d request bitmap 0x%x",
			priv->request_id, priv->request_bitmap);
		ret = -ETIMEDOUT;
		goto exit;
	}
	hdd_exit();

exit:
	osif_request_put(request);
	return ret;
}

int wlan_hdd_ll_stats_get(struct hdd_adapter *adapter, uint32_t req_id,
			  uint32_t req_mask)
{
	int errno;
	tSirLLStatsGetReq get_req;
	struct hdd_station_ctx *hddstactx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	hdd_enter();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_warn("Command not allowed in FTM mode");
		return -EPERM;
	}

	if (hddstactx->hdd_reassoc_scenario) {
		hdd_err("Roaming in progress, cannot process the request");
		return -EBUSY;
	}

	if (!adapter->is_link_layer_stats_set) {
		hdd_info("LL_STATs not set");
		return -EINVAL;
	}

	get_req.reqId = req_id;
	get_req.paramIdMask = req_mask;
	get_req.staId = adapter->vdev_id;

	rtnl_lock();
	errno = wlan_hdd_send_ll_stats_req(hdd_ctx, &get_req);
	rtnl_unlock();
	if (errno)
		hdd_err("Send LL stats req failed, id:%u, mask:%d, session:%d",
			req_id, req_mask, adapter->vdev_id);

	hdd_exit();

	return errno;
}

/**
 * __wlan_hdd_cfg80211_ll_stats_get() - get link layer stats
 * @wiphy: Pointer to wiphy
 * @wdev: Pointer to wdev
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: int
 */
static int
__wlan_hdd_cfg80211_ll_stats_get(struct wiphy *wiphy,
				   struct wireless_dev *wdev,
				   const void *data,
				   int data_len)
{
	int ret;
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct nlattr *tb_vendor[QCA_WLAN_VENDOR_ATTR_LL_STATS_GET_MAX + 1];
	tSirLLStatsGetReq LinkLayerStatsGetReq;
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_station_ctx *hddstactx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	/* ENTER() intentionally not used in a frequently invoked API */

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return -EINVAL;

	if (!adapter->is_link_layer_stats_set) {
		hdd_warn("is_link_layer_stats_set: %d",
			 adapter->is_link_layer_stats_set);
		return -EINVAL;
	}

	if (hddstactx->hdd_reassoc_scenario) {
		hdd_err("Roaming in progress, cannot process the request");
		return -EBUSY;
	}

	if (wlan_cfg80211_nla_parse(tb_vendor,
				    QCA_WLAN_VENDOR_ATTR_LL_STATS_GET_MAX,
				    (struct nlattr *)data, data_len,
				    qca_wlan_vendor_ll_get_policy)) {
		hdd_err("max attribute not present");
		return -EINVAL;
	}

	if (!tb_vendor[QCA_WLAN_VENDOR_ATTR_LL_STATS_GET_CONFIG_REQ_ID]) {
		hdd_err("Request Id Not present");
		return -EINVAL;
	}

	if (!tb_vendor[QCA_WLAN_VENDOR_ATTR_LL_STATS_GET_CONFIG_REQ_MASK]) {
		hdd_err("Req Mask Not present");
		return -EINVAL;
	}

	LinkLayerStatsGetReq.reqId =
		nla_get_u32(tb_vendor
			    [QCA_WLAN_VENDOR_ATTR_LL_STATS_GET_CONFIG_REQ_ID]);
	LinkLayerStatsGetReq.paramIdMask =
		nla_get_u32(tb_vendor
			    [QCA_WLAN_VENDOR_ATTR_LL_STATS_GET_CONFIG_REQ_MASK]);

	LinkLayerStatsGetReq.staId = adapter->vdev_id;

	if (wlan_hdd_validate_vdev_id(adapter->vdev_id))
		return -EINVAL;

	ret = wlan_hdd_send_ll_stats_req(hdd_ctx, &LinkLayerStatsGetReq);
	if (0 != ret) {
		hdd_err("Failed to send LL stats request (id:%u)",
			LinkLayerStatsGetReq.reqId);
		return ret;
	}

	hdd_exit();
	return 0;
}

/**
 * wlan_hdd_cfg80211_ll_stats_get() - get ll stats
 * @wiphy: Pointer to wiphy
 * @wdev: Pointer to wdev
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: 0 if success, non-zero for failure
 */
int wlan_hdd_cfg80211_ll_stats_get(struct wiphy *wiphy,
				   struct wireless_dev *wdev,
				   const void *data,
				   int data_len)
{
	struct osif_vdev_sync *vdev_sync;
	int errno;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_ll_stats_get(wiphy, wdev, data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}

const struct
nla_policy
	qca_wlan_vendor_ll_clr_policy[QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_CONFIG_REQ_MASK] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_CONFIG_STOP_REQ] = {.type = NLA_U8},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_CONFIG_RSP_MASK] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_CONFIG_STOP_RSP] = {.type = NLA_U8},
};

/**
 * __wlan_hdd_cfg80211_ll_stats_clear() - clear link layer stats
 * @wiphy: Pointer to wiphy
 * @wdev: Pointer to wdev
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: int
 */
static int
__wlan_hdd_cfg80211_ll_stats_clear(struct wiphy *wiphy,
				    struct wireless_dev *wdev,
				    const void *data,
				    int data_len)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct nlattr *tb_vendor[QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_MAX + 1];
	tSirLLStatsClearReq LinkLayerStatsClearReq;
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	u32 statsClearReqMask;
	u8 stopReq;
	int errno;
	QDF_STATUS status;
	struct sk_buff *skb;

	hdd_enter_dev(dev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	errno = wlan_hdd_validate_context(hdd_ctx);
	if (errno)
		return -EINVAL;

	if (!adapter->is_link_layer_stats_set) {
		hdd_warn("is_link_layer_stats_set : %d",
			  adapter->is_link_layer_stats_set);
		return -EINVAL;
	}

	if (wlan_cfg80211_nla_parse(tb_vendor,
				    QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_MAX,
				    (struct nlattr *)data, data_len,
				    qca_wlan_vendor_ll_clr_policy)) {
		hdd_err("STATS_CLR_MAX is not present");
		return -EINVAL;
	}

	if (!tb_vendor[QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_CONFIG_REQ_MASK] ||
	    !tb_vendor[QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_CONFIG_STOP_REQ]) {
		hdd_err("Error in LL_STATS CLR CONFIG PARA");
		return -EINVAL;
	}

	statsClearReqMask = LinkLayerStatsClearReq.statsClearReqMask =
				    nla_get_u32(tb_vendor
						[QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_CONFIG_REQ_MASK]);

	stopReq = LinkLayerStatsClearReq.stopReq =
			  nla_get_u8(tb_vendor
				     [QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_CONFIG_STOP_REQ]);

	/*
	 * Shall take the request Id if the Upper layers pass. 1 For now.
	 */
	LinkLayerStatsClearReq.reqId = 1;

	LinkLayerStatsClearReq.staId = adapter->vdev_id;

	hdd_debug("LL_STATS_CLEAR reqId = %d, staId = %d, statsClearReqMask = 0x%X, stopReq = %d",
		LinkLayerStatsClearReq.reqId,
		LinkLayerStatsClearReq.staId,
		LinkLayerStatsClearReq.statsClearReqMask,
		LinkLayerStatsClearReq.stopReq);

	status = sme_ll_stats_clear_req(hdd_ctx->mac_handle,
					&LinkLayerStatsClearReq);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("stats clear request failed, %d", status);
		return -EINVAL;
	}

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy,
						  2 * sizeof(u32) +
						  2 * NLMSG_HDRLEN);
	if (!skb) {
		hdd_err("skb allocation failed");
		return -ENOMEM;
	}

	if (nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_CONFIG_RSP_MASK,
			statsClearReqMask) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_CONFIG_STOP_RSP,
			stopReq)) {
		hdd_err("LL_STATS_CLR put fail");
		kfree_skb(skb);
		return -EINVAL;
	}

	/* If the ask is to stop the stats collection
	 * as part of clear (stopReq = 1), ensure
	 * that no further requests of get go to the
	 * firmware by having is_link_layer_stats_set set
	 * to 0.  However it the stopReq as part of
	 * the clear request is 0, the request to get
	 * the statistics are honoured as in this case
	 * the firmware is just asked to clear the
	 * statistics.
	 */
	if (stopReq == 1)
		adapter->is_link_layer_stats_set = false;

	hdd_exit();

	return cfg80211_vendor_cmd_reply(skb);
}

/**
 * wlan_hdd_cfg80211_ll_stats_clear() - clear ll stats
 * @wiphy: Pointer to wiphy
 * @wdev: Pointer to wdev
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: 0 if success, non-zero for failure
 */
int wlan_hdd_cfg80211_ll_stats_clear(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					const void *data,
					int data_len)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_ll_stats_clear(wiphy, wdev, data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}

/**
 * wlan_hdd_clear_link_layer_stats() - clear link layer stats
 * @adapter: pointer to adapter
 *
 * Wrapper function to clear link layer stats.
 * return - void
 */
void wlan_hdd_clear_link_layer_stats(struct hdd_adapter *adapter)
{
	tSirLLStatsClearReq link_layer_stats_clear_req;
	mac_handle_t mac_handle = adapter->hdd_ctx->mac_handle;

	link_layer_stats_clear_req.statsClearReqMask = WIFI_STATS_IFACE_AC |
		WIFI_STATS_IFACE_ALL_PEER;
	link_layer_stats_clear_req.stopReq = 0;
	link_layer_stats_clear_req.reqId = 1;
	link_layer_stats_clear_req.staId = adapter->vdev_id;
	sme_ll_stats_clear_req(mac_handle, &link_layer_stats_clear_req);
}

/**
 * hdd_populate_per_peer_ps_info() - populate per peer sta's PS info
 * @wifi_peer_info: peer information
 * @vendor_event: buffer for vendor event
 *
 * Return: 0 success
 */
static inline int
hdd_populate_per_peer_ps_info(tSirWifiPeerInfo *wifi_peer_info,
			      struct sk_buff *vendor_event)
{
	if (!wifi_peer_info) {
		hdd_err("Invalid pointer to peer info.");
		return -EINVAL;
	}

	if (nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_PS_STATE,
			wifi_peer_info->power_saving) ||
	    nla_put(vendor_event,
		    QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_MAC_ADDRESS,
		    QDF_MAC_ADDR_SIZE, &wifi_peer_info->peerMacAddress)) {
		hdd_err("QCA_WLAN_VENDOR_ATTR put fail.");
		return -EINVAL;
	}
	return 0;
}

/**
 * hdd_populate_wifi_peer_ps_info() - populate peer sta's power state
 * @data: stats for peer STA
 * @vendor_event: buffer for vendor event
 *
 * Return: 0 success
 */
static int hdd_populate_wifi_peer_ps_info(tSirWifiPeerStat *data,
					  struct sk_buff *vendor_event)
{
	uint32_t peer_num, i;
	tSirWifiPeerInfo *wifi_peer_info;
	struct nlattr *peer_info, *peers;

	if (!data) {
		hdd_err("Invalid pointer to Wifi peer stat.");
		return -EINVAL;
	}

	peer_num = data->numPeers;
	if (peer_num == 0) {
		hdd_err("Peer number is zero.");
		return -EINVAL;
	}

	if (nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_NUM,
			peer_num)) {
		hdd_err("QCA_WLAN_VENDOR_ATTR put fail");
		return -EINVAL;
	}

	peer_info = nla_nest_start(vendor_event,
			       QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_PS_CHG);
	if (!peer_info) {
		hdd_err("nla_nest_start failed");
		return -EINVAL;
	}

	for (i = 0; i < peer_num; i++) {
		wifi_peer_info = &data->peerInfo[i];
		peers = nla_nest_start(vendor_event, i);

		if (!peers) {
			hdd_err("nla_nest_start failed");
			return -EINVAL;
		}

		if (hdd_populate_per_peer_ps_info(wifi_peer_info, vendor_event))
			return -EINVAL;

		nla_nest_end(vendor_event, peers);
	}
	nla_nest_end(vendor_event, peer_info);

	return 0;
}

/**
 * hdd_populate_tx_failure_info() - populate TX failure info
 * @tx_fail: TX failure info
 * @skb: buffer for vendor event
 *
 * Return: 0 Success
 */
static inline int
hdd_populate_tx_failure_info(struct sir_wifi_iface_tx_fail *tx_fail,
			     struct sk_buff *skb)
{
	int status = 0;

	if (!tx_fail || !skb)
		return -EINVAL;

	if (nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TID,
			tx_fail->tid) ||
	    nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_NUM_MSDU,
			tx_fail->msdu_num) ||
	    nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_STATUS,
			tx_fail->status)) {
		hdd_err("QCA_WLAN_VENDOR_ATTR put fail");
		status = -EINVAL;
	}

	return status;
}

/**
 * hdd_populate_wifi_channel_cca_info() - put channel cca info to vendor event
 * @info: cca info array for all channels
 * @vendor_event: vendor event buffer
 *
 * Return: 0 Success, EINVAL failure
 */
static int
hdd_populate_wifi_channel_cca_info(struct sir_wifi_chan_cca_stats *cca,
				   struct sk_buff *vendor_event)
{
	/* There might be no CCA info for a channel */
	if (!cca)
		return 0;

	if (nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_IDLE_TIME,
			cca->idle_time) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_TIME,
			cca->tx_time) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_IN_BSS_TIME,
			cca->rx_in_bss_time) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_OUT_BSS_TIME,
			cca->rx_out_bss_time) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_BUSY,
			cca->rx_busy_time) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_BAD,
			cca->rx_in_bad_cond_time) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_BAD,
			cca->tx_in_bad_cond_time) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_NO_AVAIL,
			cca->wlan_not_avail_time) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_IFACE_ID,
			cca->vdev_id)) {
		hdd_err("QCA_WLAN_VENDOR_ATTR put fail");
		return -EINVAL;
	}
	return 0;
}

/**
 * hdd_populate_wifi_signal_info - put chain signal info
 * @info: RF chain signal info
 * @skb: vendor event buffer
 *
 * Return: 0 Success, EINVAL failure
 */
static int
hdd_populate_wifi_signal_info(struct sir_wifi_peer_signal_stats *peer_signal,
			      struct sk_buff *skb)
{
	uint32_t i, chain_count;
	struct nlattr *chains, *att;

	/* There might be no signal info for a peer */
	if (!peer_signal)
		return 0;

	chain_count = peer_signal->num_chain < WIFI_MAX_CHAINS ?
		      peer_signal->num_chain : WIFI_MAX_CHAINS;
	if (nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_ANT_NUM,
			chain_count)) {
		hdd_err("QCA_WLAN_VENDOR_ATTR put fail");
		return -EINVAL;
	}

	att = nla_nest_start(skb,
			     QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_SIGNAL);
	if (!att) {
		hdd_err("nla_nest_start failed");
		return -EINVAL;
	}

	for (i = 0; i < chain_count; i++) {
		chains = nla_nest_start(skb, i);

		if (!chains) {
			hdd_err("nla_nest_start failed");
			return -EINVAL;
		}

		hdd_debug("SNR=%d, NF=%d, Rx=%d, Tx=%d",
			  peer_signal->per_ant_snr[i],
			  peer_signal->nf[i],
			  peer_signal->per_ant_rx_mpdus[i],
			  peer_signal->per_ant_tx_mpdus[i]);
		if (nla_put_u32(skb,
				QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_ANT_SNR,
				peer_signal->per_ant_snr[i]) ||
		    nla_put_u32(skb,
				QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_ANT_NF,
				peer_signal->nf[i]) ||
		    nla_put_u32(skb,
				QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU,
				peer_signal->per_ant_rx_mpdus[i]) ||
		    nla_put_u32(skb,
				QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_MPDU,
				peer_signal->per_ant_tx_mpdus[i])) {
			hdd_err("QCA_WLAN_VENDOR_ATTR put fail");
			return -EINVAL;
		}
		nla_nest_end(skb, chains);
	}
	nla_nest_end(skb, att);

	return 0;
}

/**
 * hdd_populate_wifi_wmm_ac_tx_info() - put AC TX info
 * @info: tx info
 * @skb: vendor event buffer
 *
 * Return: 0 Success, EINVAL failure
 */
static int
hdd_populate_wifi_wmm_ac_tx_info(struct sir_wifi_tx *tx_stats,
				 struct sk_buff *skb)
{
	uint32_t *agg_size, *succ_mcs, *fail_mcs, *delay;

	/* There might be no TX info for a peer */
	if (!tx_stats)
		return 0;

	agg_size = tx_stats->mpdu_aggr_size;
	succ_mcs = tx_stats->success_mcs;
	fail_mcs = tx_stats->fail_mcs;
	delay = tx_stats->delay;

	if (nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_MSDU,
			tx_stats->msdus) ||
	    nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_MPDU,
			tx_stats->mpdus) ||
	    nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_PPDU,
			tx_stats->ppdus) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_BYTES,
			tx_stats->bytes) ||
	    nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_DROP,
			tx_stats->drops) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_DROP_BYTES,
			tx_stats->drop_bytes) ||
	    nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_RETRY,
			tx_stats->retries) ||
	    nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_NO_ACK,
			tx_stats->failed) ||
	    nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_AGGR_NUM,
			tx_stats->aggr_len) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_SUCC_MCS_NUM,
			tx_stats->success_mcs_len) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_FAIL_MCS_NUM,
			tx_stats->fail_mcs_len) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_DELAY_ARRAY_SIZE,
			tx_stats->delay_len))
		goto put_attr_fail;

	if (agg_size) {
		if (nla_put(skb,
			    QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_AGGR,
			    tx_stats->aggr_len, agg_size))
			goto put_attr_fail;
	}

	if (succ_mcs) {
		if (nla_put(skb,
			    QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_SUCC_MCS,
			    tx_stats->success_mcs_len, succ_mcs))
			goto put_attr_fail;
	}

	if (fail_mcs) {
		if (nla_put(skb,
			    QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_FAIL_MCS,
			    tx_stats->fail_mcs_len, fail_mcs))
			goto put_attr_fail;
	}

	if (delay) {
		if (nla_put(skb,
			    QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_DELAY,
			    tx_stats->delay_len, delay))
			goto put_attr_fail;
	}
	return 0;

put_attr_fail:
	hdd_err("QCA_WLAN_VENDOR_ATTR put fail");
	return -EINVAL;
}

/**
 * hdd_populate_wifi_wmm_ac_rx_info() - put AC RX info
 * @info: rx info
 * @skb: vendor event buffer
 *
 * Return: 0 Success, EINVAL failure
 */
static int
hdd_populate_wifi_wmm_ac_rx_info(struct sir_wifi_rx *rx_stats,
				 struct sk_buff *skb)
{
	uint32_t *mcs, *aggr;

	/* There might be no RX info for a peer */
	if (!rx_stats)
		return 0;

	aggr = rx_stats->mpdu_aggr;
	mcs = rx_stats->mcs;

	if (nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU,
			rx_stats->mpdus) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_BYTES,
			rx_stats->bytes) ||
	    nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_PPDU,
			rx_stats->ppdus) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_PPDU_BYTES,
			rx_stats->ppdu_bytes) ||
	    nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_LOST,
			rx_stats->mpdu_lost) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_RETRY,
			rx_stats->mpdu_retry) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_DUP,
			rx_stats->mpdu_dup) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_DISCARD,
			rx_stats->mpdu_discard) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_AGGR_NUM,
			rx_stats->aggr_len) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MCS_NUM,
			rx_stats->mcs_len))
		goto put_attr_fail;

	if (aggr) {
		if (nla_put(skb, QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_AGGR,
			    rx_stats->aggr_len, aggr))
			goto put_attr_fail;
	}

	if (mcs) {
		if (nla_put(skb, QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MCS,
			    rx_stats->mcs_len, mcs))
			goto put_attr_fail;
	}

	return 0;

put_attr_fail:
	hdd_err("QCA_WLAN_VENDOR_ATTR put fail");
	return -EINVAL;
}

/**
 * hdd_populate_wifi_wmm_ac_info() - put WMM AC info
 * @info: per AC stats
 * @skb: vendor event buffer
 *
 * Return: 0 Success, EINVAL failure
 */
static int
hdd_populate_wifi_wmm_ac_info(struct sir_wifi_ll_ext_wmm_ac_stats *ac_stats,
			      struct sk_buff *skb)
{
	struct nlattr *wmm;

	wmm = nla_nest_start(skb, ac_stats->type);
	if (!wmm)
		goto nest_start_fail;

	if (hdd_populate_wifi_wmm_ac_tx_info(ac_stats->tx_stats, skb) ||
	    hdd_populate_wifi_wmm_ac_rx_info(ac_stats->rx_stats, skb))
		goto put_attr_fail;

	nla_nest_end(skb, wmm);
	return 0;

nest_start_fail:
	hdd_err("nla_nest_start failed");
	return -EINVAL;

put_attr_fail:
	hdd_err("QCA_WLAN_VENDOR_ATTR put fail");
	return -EINVAL;
}

/**
 * hdd_populate_wifi_ll_ext_peer_info() - put per peer info
 * @info: peer stats
 * @skb: vendor event buffer
 *
 * Return: 0 Success, EINVAL failure
 */
static int
hdd_populate_wifi_ll_ext_peer_info(struct sir_wifi_ll_ext_peer_stats *peers,
				   struct sk_buff *skb)
{
	uint32_t i;
	struct nlattr *wmm_ac;

	if (nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_ID,
			peers->peer_id) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_IFACE_ID,
			peers->vdev_id) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_PS_TIMES,
			peers->sta_ps_inds) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_PS_DURATION,
			peers->sta_ps_durs) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_PROBE_REQ,
			peers->rx_probe_reqs) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MGMT,
			peers->rx_oth_mgmts) ||
	    nla_put(skb, QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_MAC_ADDRESS,
		    QDF_MAC_ADDR_SIZE, peers->mac_address) ||
	    hdd_populate_wifi_signal_info(&peers->peer_signal_stats, skb)) {
		hdd_err("put peer signal attr failed");
		return -EINVAL;
	}

	wmm_ac = nla_nest_start(skb,
				QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_STATUS);
	if (!wmm_ac) {
		hdd_err("nla_nest_start failed");
		return -EINVAL;
	}

	for (i = 0; i < WLAN_MAX_AC; i++) {
		if (hdd_populate_wifi_wmm_ac_info(&peers->ac_stats[i], skb)) {
			hdd_err("put WMM AC attr failed");
			return -EINVAL;
		}
	}

	nla_nest_end(skb, wmm_ac);
	return 0;
}

/**
 * hdd_populate_wifi_ll_ext_stats() - put link layer extension stats
 * @info: link layer stats
 * @skb: vendor event buffer
 *
 * Return: 0 Success, EINVAL failure
 */
static int
hdd_populate_wifi_ll_ext_stats(struct sir_wifi_ll_ext_stats *stats,
			       struct sk_buff *skb)
{
	uint32_t i;
	struct nlattr *peer, *peer_info, *channels, *channel_info;

	if (nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_EVENT_MODE,
			stats->trigger_cond_id) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_CCA_BSS_BITMAP,
			stats->cca_chgd_bitmap) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_SIGNAL_BITMAP,
			stats->sig_chgd_bitmap) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_BITMAP,
			stats->tx_chgd_bitmap) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_BITMAP,
			stats->rx_chgd_bitmap) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_CHANNEL_NUM,
			stats->channel_num) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_NUM,
			stats->peer_num)) {
		goto put_attr_fail;
	}

	channels = nla_nest_start(skb,
				  QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_CCA_BSS);
	if (!channels) {
		hdd_err("nla_nest_start failed");
		return -EINVAL;
	}

	for (i = 0; i < stats->channel_num; i++) {
		channel_info = nla_nest_start(skb, i);
		if (!channel_info) {
			hdd_err("nla_nest_start failed");
			return -EINVAL;
		}

		if (hdd_populate_wifi_channel_cca_info(&stats->cca[i], skb))
			goto put_attr_fail;
		nla_nest_end(skb, channel_info);
	}
	nla_nest_end(skb, channels);

	peer_info = nla_nest_start(skb,
				   QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER);
	if (!peer_info) {
		hdd_err("nla_nest_start failed");
		return -EINVAL;
	}

	for (i = 0; i < stats->peer_num; i++) {
		peer = nla_nest_start(skb, i);
		if (!peer) {
			hdd_err("nla_nest_start failed");
			return -EINVAL;
		}

		if (hdd_populate_wifi_ll_ext_peer_info(&stats->peer_stats[i],
						       skb))
			goto put_attr_fail;
		nla_nest_end(skb, peer);
	}

	nla_nest_end(skb, peer_info);
	return 0;

put_attr_fail:
	hdd_err("QCA_WLAN_VENDOR_ATTR put fail");
	return -EINVAL;
}

/**
 * wlan_hdd_cfg80211_link_layer_stats_ext_callback() - Callback for LL ext
 * @ctx: HDD context
 * @rsp: msg from FW
 *
 * This function is an extension of
 * wlan_hdd_cfg80211_link_layer_stats_callback. It converts
 * monitoring parameters offloaded to NL data and send the same to the
 * kernel/upper layers.
 *
 * Return: None
 */
void wlan_hdd_cfg80211_link_layer_stats_ext_callback(hdd_handle_t ctx,
						     tSirLLStatsResults *rsp)
{
	struct hdd_context *hdd_ctx;
	struct sk_buff *skb = NULL;
	uint32_t param_id, index;
	struct hdd_adapter *adapter = NULL;
	tSirLLStatsResults *linkLayer_stats_results;
	tSirWifiPeerStat *peer_stats;
	uint8_t *results;
	int status;

	hdd_enter();

	if (!rsp) {
		hdd_err("Invalid result.");
		return;
	}

	hdd_ctx = hdd_handle_to_context(ctx);
	linkLayer_stats_results = rsp;

	status = wlan_hdd_validate_context(hdd_ctx);
	if (0 != status)
		return;

	adapter = hdd_get_adapter_by_vdev(hdd_ctx,
					  linkLayer_stats_results->ifaceId);

	if (!adapter) {
		hdd_err("vdev_id %d does not exist with host.",
			linkLayer_stats_results->ifaceId);
		return;
	}

	index = QCA_NL80211_VENDOR_SUBCMD_LL_STATS_EXT_INDEX;
	skb = cfg80211_vendor_event_alloc(hdd_ctx->wiphy,
			NULL, LL_STATS_EVENT_BUF_SIZE + NLMSG_HDRLEN,
			index, GFP_KERNEL);
	if (!skb) {
		hdd_err("cfg80211_vendor_event_alloc failed.");
		return;
	}

	results = linkLayer_stats_results->results;
	param_id = linkLayer_stats_results->paramId;
	hdd_info("LL_STATS RESP paramID = 0x%x, ifaceId = %u, result = %pK",
		 linkLayer_stats_results->paramId,
		 linkLayer_stats_results->ifaceId,
		 linkLayer_stats_results->results);
	if (param_id & WMI_LL_STATS_EXT_PS_CHG) {
		peer_stats = (tSirWifiPeerStat *)results;
		status = hdd_populate_wifi_peer_ps_info(peer_stats, skb);
	} else if (param_id & WMI_LL_STATS_EXT_TX_FAIL) {
		struct sir_wifi_iface_tx_fail *tx_fail;

		tx_fail = (struct sir_wifi_iface_tx_fail *)results;
		status = hdd_populate_tx_failure_info(tx_fail, skb);
	} else if (param_id & WMI_LL_STATS_EXT_MAC_COUNTER) {
		hdd_info("MAC counters stats");
		status = hdd_populate_wifi_ll_ext_stats(
				(struct sir_wifi_ll_ext_stats *)
				rsp->results, skb);
	} else {
		hdd_info("Unknown link layer stats");
		status = -EINVAL;
	}

	if (status == 0)
		cfg80211_vendor_event(skb, GFP_KERNEL);
	else
		kfree_skb(skb);
	hdd_exit();
}

static const struct nla_policy
qca_wlan_vendor_ll_ext_policy[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_CFG_PERIOD] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_GLOBAL] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_CFG_THRESHOLD] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_BITMAP] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_BITMAP] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_CCA_BSS_BITMAP] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_SIGNAL_BITMAP] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_MSDU] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_MPDU] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_PPDU] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_BYTES] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_DROP] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_DROP_BYTES] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_RETRY] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_NO_ACK] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_NO_BACK] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_AGGR] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_SUCC_MCS] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_FAIL_MCS] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_DELAY] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_BYTES] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_PPDU] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_PPDU_BYTES] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_LOST] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_RETRY] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_DUP] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_DISCARD] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MCS] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_AGGR] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_PS_TIMES] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_PS_DURATION] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_PROBE_REQ] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MGMT] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_IDLE_TIME] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_TIME] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_BUSY] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_BAD] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_BAD] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_NO_AVAIL] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_IN_BSS_TIME] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_OUT_BSS_TIME] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_ANT_SNR] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_ANT_NF] = {
		.type = NLA_U32
	},
};

/**
 * __wlan_hdd_cfg80211_ll_stats_ext_set_param - config monitor parameters
 * @wiphy: wiphy handle
 * @wdev: wdev handle
 * @data: user layer input
 * @data_len: length of user layer input
 *
 * this function is called in ssr protected environment.
 *
 * return: 0 success, none zero for failure
 */
static int __wlan_hdd_cfg80211_ll_stats_ext_set_param(struct wiphy *wiphy,
						      struct wireless_dev *wdev,
						      const void *data,
						      int data_len)
{
	QDF_STATUS status;
	int errno;
	uint32_t period;
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct sir_ll_ext_stats_threshold thresh = {0,};
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_MAX + 1];

	hdd_enter_dev(dev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_warn("command not allowed in ftm mode");
		return -EPERM;
	}

	errno = wlan_hdd_validate_context(hdd_ctx);
	if (errno)
		return -EPERM;

	if (wlan_cfg80211_nla_parse(tb, QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_MAX,
				    (struct nlattr *)data, data_len,
				    qca_wlan_vendor_ll_ext_policy)) {
		hdd_err("maximum attribute not present");
		return -EPERM;
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_CFG_PERIOD]) {
		period = nla_get_u32(tb[
				QCA_WLAN_VENDOR_ATTR_LL_STATS_CFG_PERIOD]);

		if (period != 0 && period < LL_STATS_MIN_PERIOD)
			period = LL_STATS_MIN_PERIOD;

		/*
		 * Only enable/disbale counters.
		 * Keep the last threshold settings.
		 */
		goto set_period;
	}

	/* global thresh is not enabled */
	if (!tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_CFG_THRESHOLD]) {
		thresh.global = false;
		hdd_warn("global thresh is not set");
	} else {
		thresh.global_threshold = nla_get_u32(tb[
				QCA_WLAN_VENDOR_ATTR_LL_STATS_CFG_THRESHOLD]);
		thresh.global = true;
		hdd_debug("globle thresh is %d", thresh.global_threshold);
	}

	if (!tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_GLOBAL]) {
		thresh.global = false;
		hdd_warn("global thresh is not enabled");
	} else {
		thresh.global = nla_get_u32(tb[
				QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_GLOBAL]);
		hdd_debug("global is %d", thresh.global);
	}

	thresh.enable_bitmap = false;
	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_BITMAP]) {
		thresh.tx_bitmap = nla_get_u32(tb[
				QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_BITMAP]);
		thresh.enable_bitmap = true;
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_BITMAP]) {
		thresh.rx_bitmap = nla_get_u32(tb[
				QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_BITMAP]);
		thresh.enable_bitmap = true;
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_CCA_BSS_BITMAP]) {
		thresh.cca_bitmap = nla_get_u32(tb[
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_CCA_BSS_BITMAP]);
		thresh.enable_bitmap = true;
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_SIGNAL_BITMAP]) {
		thresh.signal_bitmap = nla_get_u32(tb[
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_SIGNAL_BITMAP]);
		thresh.enable_bitmap = true;
	}

	if (!thresh.global && !thresh.enable_bitmap) {
		hdd_warn("threshold will be disabled.");
		thresh.enable = false;

		/* Just disable threshold */
		goto set_thresh;
	} else {
		thresh.enable = true;
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_MSDU]) {
		thresh.tx.msdu = nla_get_u32(tb[
				QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_MSDU]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_MPDU]) {
		thresh.tx.mpdu = nla_get_u32(tb[
				QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_MPDU]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_PPDU]) {
		thresh.tx.ppdu = nla_get_u32(tb[
				QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_PPDU]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_BYTES]) {
		thresh.tx.bytes = nla_get_u32(tb[
				QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_BYTES]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_DROP]) {
		thresh.tx.msdu_drop = nla_get_u32(
			tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_DROP]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_DROP_BYTES]) {
		thresh.tx.byte_drop = nla_get_u32(tb[
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_DROP_BYTES]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_RETRY]) {
		thresh.tx.mpdu_retry = nla_get_u32(tb[
				QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_RETRY]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_NO_ACK]) {
		thresh.tx.mpdu_fail = nla_get_u32(tb[
				QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_NO_ACK]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_NO_BACK]) {
		thresh.tx.ppdu_fail = nla_get_u32(tb[
				QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_NO_BACK]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_AGGR]) {
		thresh.tx.aggregation = nla_get_u32(tb[
				  QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_AGGR]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_SUCC_MCS]) {
		thresh.tx.succ_mcs = nla_get_u32(tb[
				QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_SUCC_MCS]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_FAIL_MCS]) {
		thresh.tx.fail_mcs = nla_get_u32(tb[
				QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_FAIL_MCS]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_DELAY]) {
		thresh.tx.delay = nla_get_u32(tb[
				QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_DELAY]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU]) {
		thresh.rx.mpdu = nla_get_u32(tb[
				QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_BYTES]) {
		thresh.rx.bytes = nla_get_u32(tb[
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_BYTES]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_PPDU]) {
		thresh.rx.ppdu = nla_get_u32(tb[
				QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_PPDU]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_PPDU_BYTES]) {
		thresh.rx.ppdu_bytes = nla_get_u32(tb[
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_PPDU_BYTES]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_LOST]) {
		thresh.rx.mpdu_lost = nla_get_u32(tb[
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_LOST]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_RETRY]) {
		thresh.rx.mpdu_retry = nla_get_u32(tb[
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_RETRY]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_DUP]) {
		thresh.rx.mpdu_dup = nla_get_u32(tb[
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_DUP]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_DISCARD]) {
		thresh.rx.mpdu_discard = nla_get_u32(tb[
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_DISCARD]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_AGGR]) {
		thresh.rx.aggregation = nla_get_u32(tb[
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_AGGR]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MCS]) {
		thresh.rx.mcs = nla_get_u32(tb[
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MCS]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_PS_TIMES]) {
		thresh.rx.ps_inds = nla_get_u32(tb[
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_PS_TIMES]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_PS_DURATION]) {
		thresh.rx.ps_durs = nla_get_u32(tb[
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_PS_DURATION]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_PROBE_REQ]) {
		thresh.rx.probe_reqs = nla_get_u32(tb[
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_PROBE_REQ]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MGMT]) {
		thresh.rx.other_mgmt = nla_get_u32(tb[
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MGMT]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_IDLE_TIME]) {
		thresh.cca.idle_time = nla_get_u32(tb[
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_IDLE_TIME]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_TIME]) {
		thresh.cca.tx_time = nla_get_u32(tb[
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_TIME]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_IN_BSS_TIME]) {
		thresh.cca.rx_in_bss_time = nla_get_u32(tb[
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_IN_BSS_TIME]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_OUT_BSS_TIME]) {
		thresh.cca.rx_out_bss_time = nla_get_u32(tb[
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_OUT_BSS_TIME]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_BUSY]) {
		thresh.cca.rx_busy_time = nla_get_u32(tb[
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_BUSY]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_BAD]) {
		thresh.cca.rx_in_bad_cond_time = nla_get_u32(tb[
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_BAD]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_BAD]) {
		thresh.cca.tx_in_bad_cond_time = nla_get_u32(tb[
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_BAD]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_NO_AVAIL]) {
		thresh.cca.wlan_not_avail_time = nla_get_u32(tb[
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_NO_AVAIL]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_ANT_SNR]) {
		thresh.signal.snr = nla_get_u32(tb[
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_ANT_SNR]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_ANT_NF]) {
		thresh.signal.nf = nla_get_u32(tb[
			QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_ANT_NF]);
	}

set_thresh:
	hdd_info("send thresh settings to target");
	status = sme_ll_stats_set_thresh(hdd_ctx->mac_handle, &thresh);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("sme_ll_stats_set_thresh failed.");
		return -EINVAL;
	}
	return 0;

set_period:
	hdd_info("send period to target");
	errno = wma_cli_set_command(adapter->vdev_id,
				    WMI_PDEV_PARAM_STATS_OBSERVATION_PERIOD,
				    period, PDEV_CMD);
	if (errno) {
		hdd_err("wma_cli_set_command set_period failed.");
		return -EINVAL;
	}
	return 0;
}

/**
 * wlan_hdd_cfg80211_ll_stats_ext_set_param - config monitor parameters
 * @wiphy: wiphy handle
 * @wdev: wdev handle
 * @data: user layer input
 * @data_len: length of user layer input
 *
 * return: 0 success, einval failure
 */
int wlan_hdd_cfg80211_ll_stats_ext_set_param(struct wiphy *wiphy,
					     struct wireless_dev *wdev,
					     const void *data,
					     int data_len)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_ll_stats_ext_set_param(wiphy, wdev,
							   data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}
#endif /* WLAN_FEATURE_LINK_LAYER_STATS */

#ifdef WLAN_FEATURE_STATS_EXT
/**
 * __wlan_hdd_cfg80211_stats_ext_request() - ext stats request
 * @wiphy: Pointer to wiphy
 * @wdev: Pointer to wdev
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: int
 */
static int __wlan_hdd_cfg80211_stats_ext_request(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data,
						 int data_len)
{
	tStatsExtRequestReq stats_ext_req;
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	int ret_val;
	QDF_STATUS status;
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);

	hdd_enter_dev(dev);

	ret_val = wlan_hdd_validate_context(hdd_ctx);
	if (ret_val)
		return ret_val;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	stats_ext_req.request_data_len = data_len;
	stats_ext_req.request_data = (void *)data;

	status = sme_stats_ext_request(adapter->vdev_id, &stats_ext_req);

	if (QDF_STATUS_SUCCESS != status)
		ret_val = -EINVAL;

	return ret_val;
}

/**
 * wlan_hdd_cfg80211_stats_ext_request() - ext stats request
 * @wiphy: Pointer to wiphy
 * @wdev: Pointer to wdev
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: int
 */
int wlan_hdd_cfg80211_stats_ext_request(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					const void *data,
					int data_len)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_stats_ext_request(wiphy, wdev,
						      data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}

void wlan_hdd_cfg80211_stats_ext_callback(hdd_handle_t hdd_handle,
					  struct stats_ext_event *data)
{
	struct hdd_context *hdd_ctx = hdd_handle_to_context(hdd_handle);
	struct sk_buff *vendor_event;
	int status;
	int ret_val;
	struct hdd_adapter *adapter;

	status = wlan_hdd_validate_context(hdd_ctx);
	if (status)
		return;

	adapter = hdd_get_adapter_by_vdev(hdd_ctx, data->vdev_id);
	if (!adapter) {
		hdd_err("vdev_id %d does not exist with host", data->vdev_id);
		return;
	}

	vendor_event = cfg80211_vendor_event_alloc(hdd_ctx->wiphy,
						   NULL,
						   data->event_data_len +
						   sizeof(uint32_t) +
						   NLMSG_HDRLEN + NLMSG_HDRLEN,
						   QCA_NL80211_VENDOR_SUBCMD_STATS_EXT_INDEX,
						   GFP_KERNEL);

	if (!vendor_event) {
		hdd_err("cfg80211_vendor_event_alloc failed");
		return;
	}

	ret_val = nla_put_u32(vendor_event, QCA_WLAN_VENDOR_ATTR_IFINDEX,
			      adapter->dev->ifindex);
	if (ret_val) {
		hdd_err("QCA_WLAN_VENDOR_ATTR_IFINDEX put fail");
		kfree_skb(vendor_event);

		return;
	}

	ret_val = nla_put(vendor_event, QCA_WLAN_VENDOR_ATTR_STATS_EXT,
			  data->event_data_len, data->event_data);

	if (ret_val) {
		hdd_err("QCA_WLAN_VENDOR_ATTR_STATS_EXT put fail");
		kfree_skb(vendor_event);

		return;
	}

	cfg80211_vendor_event(vendor_event, GFP_KERNEL);

}

void
wlan_hdd_cfg80211_stats_ext2_callback(hdd_handle_t hdd_handle,
				      struct sir_sme_rx_aggr_hole_ind *pmsg)
{
	struct hdd_context *hdd_ctx = hdd_handle_to_context(hdd_handle);
	int status;
	uint32_t data_size, hole_info_size;
	struct sk_buff *vendor_event;

	status = wlan_hdd_validate_context(hdd_ctx);
	if (0 != status)
		return;

	if (!pmsg) {
		hdd_err("msg received here is null");
		return;
	}

	hole_info_size = (pmsg->hole_cnt)*sizeof(pmsg->hole_info_array[0]);
	data_size = sizeof(struct sir_sme_rx_aggr_hole_ind) + hole_info_size;

	vendor_event = cfg80211_vendor_event_alloc(hdd_ctx->wiphy,
			NULL,
			data_size + NLMSG_HDRLEN + NLMSG_HDRLEN,
			QCA_NL80211_VENDOR_SUBCMD_STATS_EXT_INDEX,
			GFP_KERNEL);

	if (!vendor_event) {
		hdd_err("vendor_event_alloc failed for STATS_EXT2");
		return;
	}

	if (nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_RX_AGGREGATION_STATS_HOLES_NUM,
			pmsg->hole_cnt)) {
		hdd_err("%s put fail",
			"QCA_WLAN_VENDOR_ATTR_RX_AGGREGATION_STATS_HOLES_NUM");
		kfree_skb(vendor_event);
		return;
	}
	if (nla_put(vendor_event,
		    QCA_WLAN_VENDOR_ATTR_RX_AGGREGATION_STATS_HOLES_INFO,
		    hole_info_size,
		    (void *)(pmsg->hole_info_array))) {
		hdd_err("%s put fail",
			"QCA_WLAN_VENDOR_ATTR_RX_AGGREGATION_STATS_HOLES_INFO");
		kfree_skb(vendor_event);
		return;
	}

	cfg80211_vendor_event(vendor_event, GFP_KERNEL);
}

#endif /* End of WLAN_FEATURE_STATS_EXT */

#ifdef LINKSPEED_DEBUG_ENABLED
#define linkspeed_dbg(format, args...) pr_info(format, ## args)
#else
#define linkspeed_dbg(format, args...)
#endif /* LINKSPEED_DEBUG_ENABLED */

/**
 * wlan_hdd_fill_summary_stats() - populate station_info summary stats
 * @stats: summary stats to use as a source
 * @info: kernel station_info struct to use as a destination
 *
 * Return: None
 */
static void wlan_hdd_fill_summary_stats(tCsrSummaryStatsInfo *stats,
					struct station_info *info)
{
	int i;

	info->rx_packets = stats->rx_frm_cnt;
	info->tx_packets = 0;
	info->tx_retries = 0;
	info->tx_failed = 0;

	for (i = 0; i < WIFI_MAX_AC; ++i) {
		info->tx_packets += stats->tx_frm_cnt[i];
		info->tx_retries += stats->multiple_retry_cnt[i];
		info->tx_failed += stats->fail_cnt[i];
	}

	info->filled |= HDD_INFO_TX_PACKETS |
			HDD_INFO_TX_RETRIES |
			HDD_INFO_TX_FAILED  |
			HDD_INFO_RX_PACKETS;
}

/**
 * wlan_hdd_get_sap_stats() - get aggregate SAP stats
 * @adapter: sap adapter to get stats for
 * @info: kernel station_info struct to populate
 *
 * Fetch the vdev-level aggregate stats for the given SAP adapter. This is to
 * support "station dump" and "station get" for SAP vdevs, even though they
 * aren't technically stations.
 *
 * Return: errno
 */
static int
wlan_hdd_get_sap_stats(struct hdd_adapter *adapter, struct station_info *info)
{
	int ret;

	ret = wlan_hdd_get_station_stats(adapter);
	if (ret) {
		hdd_err("Failed to get SAP stats; status:%d", ret);
		return ret;
	}

	wlan_hdd_fill_summary_stats(&adapter->hdd_stats.summary_stat, info);

	return 0;
}

/**
 * hdd_get_max_rate_legacy() - get max rate for legacy mode
 * @stainfo: stainfo pointer
 * @rssidx: rssi index
 *
 * This function will get max rate for legacy mode
 *
 * Return: max rate on success, otherwise 0
 */
static uint32_t hdd_get_max_rate_legacy(struct hdd_station_info *stainfo,
					uint8_t rssidx)
{
	uint32_t maxrate = 0;
	/*Minimum max rate, 6Mbps*/
	int maxidx = 12;
	int i;

	/* check supported rates */
	if (stainfo->max_supp_idx != 0xff &&
	    maxidx < stainfo->max_supp_idx)
		maxidx = stainfo->max_supp_idx;

	/* check extended rates */
	if (stainfo->max_ext_idx != 0xff &&
	    maxidx < stainfo->max_ext_idx)
		maxidx = stainfo->max_ext_idx;

	for (i = 0; QDF_ARRAY_SIZE(supported_data_rate); i++) {
		if (supported_data_rate[i].beacon_rate_index == maxidx)
			maxrate =
				supported_data_rate[i].supported_rate[rssidx];
	}

	hdd_debug("maxrate %d", maxrate);

	return maxrate;
}

/**
 * hdd_get_max_rate_ht() - get max rate for ht mode
 * @stainfo: stainfo pointer
 * @stats: fw txrx status pointer
 * @rate_flags: rate flags
 * @nss: number of streams
 * @maxrate: returned max rate buffer pointer
 * @max_mcs_idx: max mcs idx
 * @report_max: report max rate or actual rate
 *
 * This function will get max rate for ht mode
 *
 * Return: None
 */
static void hdd_get_max_rate_ht(struct hdd_station_info *stainfo,
				struct hdd_fw_txrx_stats *stats,
				uint32_t rate_flags,
				uint8_t nss,
				uint32_t *maxrate,
				uint8_t *max_mcs_idx,
				bool report_max)
{
	struct index_data_rate_type *supported_mcs_rate;
	uint32_t tmprate;
	uint8_t flag = 0, mcsidx;
	int8_t rssi = stats->rssi;
	int mode;
	int i;

	if (rate_flags & TX_RATE_HT40)
		mode = 1;
	else
		mode = 0;

	if (rate_flags & TX_RATE_HT40)
		flag |= 1;
	if (rate_flags & TX_RATE_SGI)
		flag |= 2;

	supported_mcs_rate = (struct index_data_rate_type *)
		((nss == 1) ? &supported_mcs_rate_nss1 :
		 &supported_mcs_rate_nss2);

	if (stainfo->max_mcs_idx == 0xff) {
		hdd_err("invalid max_mcs_idx");
		/* report real mcs idx */
		mcsidx = stats->tx_rate.mcs;
	} else {
		mcsidx = stainfo->max_mcs_idx;
	}

	if (!report_max) {
		for (i = 0; i < mcsidx; i++) {
			if (rssi <= rssi_mcs_tbl[mode][i]) {
				mcsidx = i;
				break;
			}
		}
		if (mcsidx < stats->tx_rate.mcs)
			mcsidx = stats->tx_rate.mcs;
	}

	tmprate = supported_mcs_rate[mcsidx].supported_rate[flag];

	hdd_debug("tmprate %d mcsidx %d", tmprate, mcsidx);

	*maxrate = tmprate;
	*max_mcs_idx = mcsidx;
}

/**
 * hdd_get_max_rate_vht() - get max rate for vht mode
 * @stainfo: stainfo pointer
 * @stats: fw txrx status pointer
 * @rate_flags: rate flags
 * @nss: number of streams
 * @maxrate: returned max rate buffer pointer
 * @max_mcs_idx: max mcs idx
 * @report_max: report max rate or actual rate
 *
 * This function will get max rate for vht mode
 *
 * Return: None
 */
static void hdd_get_max_rate_vht(struct hdd_station_info *stainfo,
				 struct hdd_fw_txrx_stats *stats,
				 uint32_t rate_flags,
				 uint8_t nss,
				 uint32_t *maxrate,
				 uint8_t *max_mcs_idx,
				 bool report_max)
{
	struct index_vht_data_rate_type *supported_vht_mcs_rate;
	uint32_t tmprate = 0;
	uint32_t vht_max_mcs;
	uint8_t flag = 0, mcsidx = INVALID_MCS_IDX;
	int8_t rssi = stats->rssi;
	int mode;
	int i;

	supported_vht_mcs_rate = (struct index_vht_data_rate_type *)
		((nss == 1) ?
		 &supported_vht_mcs_rate_nss1 :
		 &supported_vht_mcs_rate_nss2);

	if (rate_flags & TX_RATE_VHT80)
		mode = 2;
	else if (rate_flags & TX_RATE_VHT40)
		mode = 1;
	else
		mode = 0;

	if (rate_flags &
	    (TX_RATE_VHT20 | TX_RATE_VHT40 | TX_RATE_VHT80)) {
		vht_max_mcs =
			(enum data_rate_11ac_max_mcs)
			(stainfo->tx_mcs_map & DATA_RATE_11AC_MCS_MASK);
		if (rate_flags & TX_RATE_SGI)
			flag |= 1;

		if (vht_max_mcs == DATA_RATE_11AC_MAX_MCS_7) {
			mcsidx = 7;
		} else if (vht_max_mcs == DATA_RATE_11AC_MAX_MCS_8) {
			mcsidx = 8;
		} else if (vht_max_mcs == DATA_RATE_11AC_MAX_MCS_9) {
			/*
			 * 'IEEE_P802.11ac_2013.pdf' page 325, 326
			 * - MCS9 is valid for VHT20 when Nss = 3 or Nss = 6
			 * - MCS9 is not valid for VHT20 when Nss = 1,2,4,5,7,8
			 */
			if ((rate_flags & TX_RATE_VHT20) &&
			    (nss != 3 && nss != 6))
				mcsidx = 8;
			else
				mcsidx = 9;
		} else {
			hdd_err("invalid vht_max_mcs");
			/* report real mcs idx */
			mcsidx = stats->tx_rate.mcs;
		}

		if (!report_max) {
			for (i = 0; i <= mcsidx; i++) {
				if (rssi <= rssi_mcs_tbl[mode][i]) {
					mcsidx = i;
					break;
				}
			}
			if (mcsidx < stats->tx_rate.mcs)
				mcsidx = stats->tx_rate.mcs;
		}

		if (rate_flags & TX_RATE_VHT80)
			tmprate =
		    supported_vht_mcs_rate[mcsidx].supported_VHT80_rate[flag];
		else if (rate_flags & TX_RATE_VHT40)
			tmprate =
		    supported_vht_mcs_rate[mcsidx].supported_VHT40_rate[flag];
		else if (rate_flags & TX_RATE_VHT20)
			tmprate =
		    supported_vht_mcs_rate[mcsidx].supported_VHT20_rate[flag];
	}

	hdd_debug("tmprate %d mcsidx %d", tmprate, mcsidx);

	*maxrate = tmprate;
	*max_mcs_idx = mcsidx;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
/**
 * hdd_fill_bw_mcs() - fill ch width and mcs flags
 * @stainfo: stainfo pointer
 * @rate_flags: HDD rate flags
 * @mcsidx: mcs index
 * @nss: number of streams
 * @vht: vht mode or not
 *
 * This function will fill ch width and mcs flags
 *
 * Return: None
 */
static void hdd_fill_bw_mcs(struct station_info *sinfo,
			    uint8_t rate_flags,
			    uint8_t mcsidx,
			    uint8_t nss,
			    bool vht)
{
	if (vht) {
		sinfo->txrate.nss = nss;
		sinfo->txrate.mcs = mcsidx;
		sinfo->txrate.flags |= RATE_INFO_FLAGS_VHT_MCS;
		if (rate_flags & TX_RATE_VHT80)
			sinfo->txrate.bw = RATE_INFO_BW_80;
		else if (rate_flags & TX_RATE_VHT40)
			sinfo->txrate.bw = RATE_INFO_BW_40;
		else if (rate_flags & TX_RATE_VHT20)
			sinfo->txrate.flags |= RATE_INFO_FLAGS_VHT_MCS;
	} else {
		sinfo->txrate.mcs = (nss - 1) << 3;
		sinfo->txrate.mcs |= mcsidx;
		sinfo->txrate.flags |= RATE_INFO_FLAGS_MCS;
		if (rate_flags & TX_RATE_HT40)
			sinfo->txrate.bw = RATE_INFO_BW_40;
	}
}
#else
/**
 * hdd_fill_bw_mcs() - fill ch width and mcs flags
 * @stainfo: stainfo pointer
 * @rate_flags: HDD rate flags
 * @mcsidx: mcs index
 * @nss: number of streams
 * @vht: vht mode or not
 *
 * This function will fill ch width and mcs flags
 *
 * Return: None
 */
static void hdd_fill_bw_mcs(struct station_info *sinfo,
			    uint8_t rate_flags,
			    uint8_t mcsidx,
			    uint8_t nss,
			    bool vht)
{
	if (vht) {
		sinfo->txrate.nss = nss;
		sinfo->txrate.mcs = mcsidx;
		sinfo->txrate.flags |= RATE_INFO_FLAGS_VHT_MCS;
		if (rate_flags & TX_RATE_VHT80)
			sinfo->txrate.flags |= RATE_INFO_FLAGS_80_MHZ_WIDTH;
		else if (rate_flags & TX_RATE_VHT40)
			sinfo->txrate.flags |= RATE_INFO_FLAGS_40_MHZ_WIDTH;
		else if (rate_flags & TX_RATE_VHT20)
			sinfo->txrate.flags |= RATE_INFO_FLAGS_VHT_MCS;
	} else {
		sinfo->txrate.mcs = (nss - 1) << 3;
		sinfo->txrate.mcs |= mcsidx;
		sinfo->txrate.flags |= RATE_INFO_FLAGS_MCS;
		if (rate_flags & TX_RATE_HT40)
			sinfo->txrate.flags |= RATE_INFO_FLAGS_40_MHZ_WIDTH;
	}
}
#endif

/**
 * hdd_fill_bw_mcs_vht() - fill ch width and mcs flags for VHT mode
 * @stainfo: stainfo pointer
 * @rate_flags: HDD rate flags
 * @mcsidx: mcs index
 * @nss: number of streams
 *
 * This function will fill ch width and mcs flags for VHT mode
 *
 * Return: None
 */
static void hdd_fill_bw_mcs_vht(struct station_info *sinfo,
				uint8_t rate_flags,
				uint8_t mcsidx,
				uint8_t nss)
{
	hdd_fill_bw_mcs(sinfo, rate_flags, mcsidx, nss, true);
}

/**
 * hdd_fill_sinfo_rate_info() - fill rate info of sinfo struct
 * @sinfo: station_info struct pointer
 * @rate_flags: HDD rate flags
 * @mcsidx: mcs index
 * @nss: number of streams
 * @maxrate: data rate (kbps)
 *
 * This function will fill rate info of sinfo struct
 *
 * Return: None
 */
static void hdd_fill_sinfo_rate_info(struct station_info *sinfo,
				     uint32_t rate_flags,
				     uint8_t mcsidx,
				     uint8_t nss,
				     uint32_t maxrate)
{
	if (rate_flags & TX_RATE_LEGACY) {
		/* provide to the UI in units of 100kbps */
		sinfo->txrate.legacy = maxrate;
	} else {
		/* must be MCS */
		if (rate_flags &
				(TX_RATE_VHT80 |
				 TX_RATE_VHT40 |
				 TX_RATE_VHT20))
			hdd_fill_bw_mcs_vht(sinfo, rate_flags, mcsidx, nss);

		if (rate_flags & (TX_RATE_HT20 | TX_RATE_HT40))
			hdd_fill_bw_mcs(sinfo, rate_flags, mcsidx, nss, false);

		if (rate_flags & TX_RATE_SGI) {
			if (!(sinfo->txrate.flags & RATE_INFO_FLAGS_VHT_MCS))
				sinfo->txrate.flags |= RATE_INFO_FLAGS_MCS;
			sinfo->txrate.flags |= RATE_INFO_FLAGS_SHORT_GI;
		}
	}

	hdd_info("flag %x mcs %d legacy %d nss %d",
		 sinfo->txrate.flags,
		 sinfo->txrate.mcs,
		 sinfo->txrate.legacy,
		 sinfo->txrate.nss);
}

/**
 * hdd_fill_station_info_flags() - fill flags of sinfo struct
 * @sinfo: station_info struct pointer
 *
 * This function will fill flags of sinfo struct
 *
 * Return: None
 */
static void hdd_fill_station_info_flags(struct station_info *sinfo)
{
	sinfo->filled |= HDD_INFO_SIGNAL        |
			 HDD_INFO_TX_BYTES      |
			 HDD_INFO_TX_BYTES64    |
			 HDD_INFO_TX_BITRATE    |
			 HDD_INFO_TX_PACKETS    |
			 HDD_INFO_TX_RETRIES    |
			 HDD_INFO_TX_FAILED     |
			 HDD_INFO_RX_BYTES      |
			 HDD_INFO_RX_BYTES64    |
			 HDD_INFO_RX_PACKETS    |
			 HDD_INFO_INACTIVE_TIME |
			 HDD_INFO_CONNECTED_TIME;
}

/**
 * hdd_fill_rate_info() - fill rate info of sinfo
 * @psoc: psoc context
 * @sinfo: station_info struct pointer
 * @stainfo: stainfo pointer
 * @stats: fw txrx status pointer
 *
 * This function will fill rate info of sinfo
 *
 * Return: None
 */
static void hdd_fill_rate_info(struct wlan_objmgr_psoc *psoc,
			       struct station_info *sinfo,
			       struct hdd_station_info *stainfo,
			       struct hdd_fw_txrx_stats *stats)
{
	uint8_t rate_flags;
	uint8_t mcsidx = 0xff;
	uint32_t myrate, maxrate, tmprate;
	int rssidx;
	int nss = 1;
	int link_speed_rssi_high = 0;
	int link_speed_rssi_mid = 0;
	int link_speed_rssi_low = 0;
	uint32_t link_speed_rssi_report = 0;

	ucfg_mlme_stats_get_cfg_values(psoc,
				       &link_speed_rssi_high,
				       &link_speed_rssi_mid,
				       &link_speed_rssi_low,
				       &link_speed_rssi_report);

	hdd_info("reportMaxLinkSpeed %d", link_speed_rssi_report);
	/* convert to 100kbps expected in rate table */
	myrate = stats->tx_rate.rate / 100;
	rate_flags = stainfo->rate_flags;
	if (!(rate_flags & TX_RATE_LEGACY)) {
		nss = stainfo->nss;
		if (ucfg_mlme_stats_is_link_speed_report_actual(psoc)) {
			/* Get current rate flags if report actual */
			if (stats->tx_rate.rate_flags)
				rate_flags =
					stats->tx_rate.rate_flags;
			nss = stats->tx_rate.nss;
		}

		if (stats->tx_rate.mcs == INVALID_MCS_IDX)
			rate_flags = TX_RATE_LEGACY;
	}

	if (!ucfg_mlme_stats_is_link_speed_report_actual(psoc)) {
		/* we do not want to necessarily report the current speed */
		if (ucfg_mlme_stats_is_link_speed_report_max(psoc)) {
			/* report the max possible speed */
			rssidx = 0;
		} else if (ucfg_mlme_stats_is_link_speed_report_max_scaled(
					psoc)) {
			/* report the max possible speed with RSSI scaling */
			if (stats->rssi >= link_speed_rssi_high) {
				/* report the max possible speed */
				rssidx = 0;
			} else if (stats->rssi >= link_speed_rssi_mid) {
				/* report middle speed */
				rssidx = 1;
			} else if (stats->rssi >= link_speed_rssi_low) {
				/* report middle speed */
				rssidx = 2;
			} else {
				/* report actual speed */
				rssidx = 3;
			}
		} else {
			/* unknown, treat as eHDD_LINK_SPEED_REPORT_MAX */
			hdd_err("Invalid value for reportMaxLinkSpeed: %u",
				link_speed_rssi_report);
			rssidx = 0;
		}

		maxrate = hdd_get_max_rate_legacy(stainfo, rssidx);

		/*
		 * Get MCS Rate Set --
		 * Only if we are connected in non legacy mode and not
		 * reporting actual speed
		 */
		if ((rssidx != 3) &&
		    !(rate_flags & TX_RATE_LEGACY)) {
			hdd_get_max_rate_vht(stainfo,
					     stats,
					     rate_flags,
					     nss,
					     &tmprate,
					     &mcsidx,
					     rssidx == 0);

			if (maxrate < tmprate &&
			    mcsidx != INVALID_MCS_IDX)
				maxrate = tmprate;

			if (mcsidx == INVALID_MCS_IDX)
				hdd_get_max_rate_ht(stainfo,
						    stats,
						    rate_flags,
						    nss,
						    &tmprate,
						    &mcsidx,
						    rssidx == 0);

			if (maxrate < tmprate &&
			    mcsidx != INVALID_MCS_IDX)
				maxrate = tmprate;
		} else if (!(rate_flags & TX_RATE_LEGACY)) {
			maxrate = myrate;
			mcsidx = stats->tx_rate.mcs;
		}

		/*
		 * make sure we report a value at least as big as our
		 * current rate
		 */
		if ((maxrate < myrate) || (maxrate == 0)) {
			maxrate = myrate;
			if (!(rate_flags & TX_RATE_LEGACY)) {
				mcsidx = stats->tx_rate.mcs;
				/*
				 * 'IEEE_P802.11ac_2013.pdf' page 325, 326
				 * - MCS9 is valid for VHT20 when Nss = 3 or
				 *   Nss = 6
				 * - MCS9 is not valid for VHT20 when
				 *   Nss = 1,2,4,5,7,8
				 */
				if ((rate_flags & TX_RATE_VHT20) &&
				    (mcsidx > 8) &&
				    (nss != 3 && nss != 6))
					mcsidx = 8;
			}
		}
	} else {
		/* report current rate instead of max rate */
		maxrate = myrate;
		if (!(rate_flags & TX_RATE_LEGACY))
			mcsidx = stats->tx_rate.mcs;
	}

	hdd_fill_sinfo_rate_info(sinfo,
				 rate_flags,
				 mcsidx,
				 nss,
				 maxrate);
}

/**
 * wlan_hdd_fill_station_info() - fill station_info struct
 * @psoc: psoc context
 * @sinfo: station_info struct pointer
 * @stainfo: stainfo pointer
 * @stats: fw txrx status pointer
 *
 * This function will fill station_info struct
 *
 * Return: None
 */
static void wlan_hdd_fill_station_info(struct wlan_objmgr_psoc *psoc,
				       struct station_info *sinfo,
				       struct hdd_station_info *stainfo,
				       struct hdd_fw_txrx_stats *stats)
{
	qdf_time_t curr_time, dur;

	curr_time = qdf_system_ticks();
	dur = curr_time - stainfo->assoc_ts;
	sinfo->connected_time = qdf_system_ticks_to_msecs(dur) / 1000;
	dur = curr_time - stainfo->last_tx_rx_ts;
	sinfo->inactive_time = qdf_system_ticks_to_msecs(dur);
	sinfo->signal = stats->rssi;
	sinfo->tx_bytes = stats->tx_bytes;
	sinfo->tx_packets = stats->tx_packets;
	sinfo->rx_bytes = stats->rx_bytes;
	sinfo->rx_packets = stats->rx_packets;
	sinfo->tx_failed = stats->tx_failed;
	sinfo->tx_retries = stats->tx_retries;

	/* tx rate info */
	hdd_fill_rate_info(psoc, sinfo, stainfo, stats);

	hdd_fill_station_info_flags(sinfo);

	/* dump sta info*/
	hdd_info("dump stainfo");
	hdd_info("con_time %d inact_time %d tx_pkts %d rx_pkts %d",
		 sinfo->connected_time, sinfo->inactive_time,
		 sinfo->tx_packets, sinfo->rx_packets);
	hdd_info("failed %d retries %d tx_bytes %lld rx_bytes %lld",
		 sinfo->tx_failed, sinfo->tx_retries,
		 sinfo->tx_bytes, sinfo->rx_bytes);
	hdd_info("rssi %d mcs %d legacy %d nss %d flags %x",
		 sinfo->signal, sinfo->txrate.mcs,
		 sinfo->txrate.legacy, sinfo->txrate.nss,
		 sinfo->txrate.flags);
}

/**
 * hdd_get_rate_flags_ht() - get HT rate flags based on rate, nss and mcs
 * @rate: Data rate (100 kbps)
 * @nss: Number of streams
 * @mcs: HT mcs index
 *
 * This function is used to construct HT rate flag with rate, nss and mcs
 *
 * Return: rate flags for success, 0 on failure.
 */
static uint8_t hdd_get_rate_flags_ht(uint32_t rate,
				     uint8_t nss,
				     uint8_t mcs)
{
	struct index_data_rate_type *mcs_rate;
	uint8_t flags = 0;

	mcs_rate = (struct index_data_rate_type *)
		((nss == 1) ? &supported_mcs_rate_nss1 :
		 &supported_mcs_rate_nss2);

	if (rate == mcs_rate[mcs].supported_rate[0]) {
		flags |= TX_RATE_HT20;
	} else if (rate == mcs_rate[mcs].supported_rate[1]) {
		flags |= TX_RATE_HT40;
	} else if (rate == mcs_rate[mcs].supported_rate[2]) {
		flags |= TX_RATE_HT20;
		flags |= TX_RATE_SGI;
	} else if (rate == mcs_rate[mcs].supported_rate[3]) {
		flags |= TX_RATE_HT40;
		flags |= TX_RATE_SGI;
	} else {
		hdd_err("invalid params rate %d nss %d mcs %d",
			rate, nss, mcs);
	}

	return flags;
}

/**
 * hdd_get_rate_flags_vht() - get VHT rate flags based on rate, nss and mcs
 * @rate: Data rate (100 kbps)
 * @nss: Number of streams
 * @mcs: VHT mcs index
 *
 * This function is used to construct VHT rate flag with rate, nss and mcs
 *
 * Return: rate flags for success, 0 on failure.
 */
static uint8_t hdd_get_rate_flags_vht(uint32_t rate,
				      uint8_t nss,
				      uint8_t mcs)
{
	struct index_vht_data_rate_type *mcs_rate;
	uint8_t flags = 0;

	mcs_rate = (struct index_vht_data_rate_type *)
		((nss == 1) ?
		 &supported_vht_mcs_rate_nss1 :
		 &supported_vht_mcs_rate_nss2);

	if (rate == mcs_rate[mcs].supported_VHT80_rate[0]) {
		flags |= TX_RATE_VHT80;
	} else if (rate == mcs_rate[mcs].supported_VHT80_rate[1]) {
		flags |= TX_RATE_VHT80;
		flags |= TX_RATE_SGI;
	} else if (rate == mcs_rate[mcs].supported_VHT40_rate[0]) {
		flags |= TX_RATE_VHT40;
	} else if (rate == mcs_rate[mcs].supported_VHT40_rate[1]) {
		flags |= TX_RATE_VHT40;
		flags |= TX_RATE_SGI;
	} else if (rate == mcs_rate[mcs].supported_VHT20_rate[0]) {
		flags |= TX_RATE_VHT20;
	} else if (rate == mcs_rate[mcs].supported_VHT20_rate[1]) {
		flags |= TX_RATE_VHT20;
		flags |= TX_RATE_SGI;
	} else {
		hdd_err("invalid params rate %d nss %d mcs %d",
			rate, nss, mcs);
	}

	return flags;
}

/**
 * hdd_get_rate_flags() - get HT/VHT rate flags based on rate, nss and mcs
 * @rate: Data rate (100 kbps)
 * @mode: Tx/Rx mode
 * @nss: Number of streams
 * @mcs: Mcs index
 *
 * This function is used to construct rate flag with rate, nss and mcs
 *
 * Return: rate flags for success, 0 on failure.
 */
static uint8_t hdd_get_rate_flags(uint32_t rate,
				  uint8_t mode,
				  uint8_t nss,
				  uint8_t mcs)
{
	uint8_t flags = 0;

	if (mode == SIR_SME_PHY_MODE_HT)
		flags = hdd_get_rate_flags_ht(rate, nss, mcs);
	else if (mode == SIR_SME_PHY_MODE_VHT)
		flags = hdd_get_rate_flags_vht(rate, nss, mcs);
	else
		hdd_err("invalid mode param %d", mode);

	return flags;
}

/**
 * wlan_hdd_fill_rate_info() - fill HDD rate info from SIR peer info
 * @ap_ctx: AP Context
 * @peer_info: SIR peer info pointer
 *
 * This function is used to fill HDD rate info rom SIR peer info
 *
 * Return: None
 */
static void wlan_hdd_fill_rate_info(struct hdd_ap_ctx *ap_ctx,
				    struct sir_peer_info_ext *peer_info)
{
	uint8_t flags;
	uint32_t rate_code;

	/* tx rate info */
	ap_ctx->txrx_stats.tx_rate.rate = peer_info->tx_rate;
	rate_code = peer_info->tx_rate_code;

	if ((WMI_GET_HW_RATECODE_PREAM_V1(rate_code)) ==
			WMI_RATE_PREAMBLE_HT)
		ap_ctx->txrx_stats.tx_rate.mode = SIR_SME_PHY_MODE_HT;
	else if ((WMI_GET_HW_RATECODE_PREAM_V1(rate_code)) ==
			WMI_RATE_PREAMBLE_VHT)
		ap_ctx->txrx_stats.tx_rate.mode = SIR_SME_PHY_MODE_VHT;
	else
		ap_ctx->txrx_stats.tx_rate.mode = SIR_SME_PHY_MODE_LEGACY;

	ap_ctx->txrx_stats.tx_rate.nss =
		WMI_GET_HW_RATECODE_NSS_V1(rate_code) + 1;
	ap_ctx->txrx_stats.tx_rate.mcs =
		WMI_GET_HW_RATECODE_RATE_V1(rate_code);

	flags = hdd_get_rate_flags(ap_ctx->txrx_stats.tx_rate.rate / 100,
				   ap_ctx->txrx_stats.tx_rate.mode,
				   ap_ctx->txrx_stats.tx_rate.nss,
				   ap_ctx->txrx_stats.tx_rate.mcs);

	ap_ctx->txrx_stats.tx_rate.rate_flags = flags;

	hdd_debug("tx: mode %d nss %d mcs %d rate_flags %x flags %x",
		  ap_ctx->txrx_stats.tx_rate.mode,
		  ap_ctx->txrx_stats.tx_rate.nss,
		  ap_ctx->txrx_stats.tx_rate.mcs,
		  ap_ctx->txrx_stats.tx_rate.rate_flags,
		  flags);

	/* rx rate info */
	ap_ctx->txrx_stats.rx_rate.rate = peer_info->rx_rate;
	rate_code = peer_info->rx_rate_code;

	if ((WMI_GET_HW_RATECODE_PREAM_V1(rate_code)) ==
			WMI_RATE_PREAMBLE_HT)
		ap_ctx->txrx_stats.rx_rate.mode = SIR_SME_PHY_MODE_HT;
	else if ((WMI_GET_HW_RATECODE_PREAM_V1(rate_code)) ==
			WMI_RATE_PREAMBLE_VHT)
		ap_ctx->txrx_stats.rx_rate.mode = SIR_SME_PHY_MODE_VHT;
	else
		ap_ctx->txrx_stats.rx_rate.mode = SIR_SME_PHY_MODE_LEGACY;

	ap_ctx->txrx_stats.rx_rate.nss =
		WMI_GET_HW_RATECODE_NSS_V1(rate_code) + 1;
	ap_ctx->txrx_stats.rx_rate.mcs =
		WMI_GET_HW_RATECODE_RATE_V1(rate_code);

	flags = hdd_get_rate_flags(ap_ctx->txrx_stats.rx_rate.rate / 100,
				   ap_ctx->txrx_stats.rx_rate.mode,
				   ap_ctx->txrx_stats.rx_rate.nss,
				   ap_ctx->txrx_stats.rx_rate.mcs);

	ap_ctx->txrx_stats.rx_rate.rate_flags = flags;

	hdd_info("rx: mode %d nss %d mcs %d rate_flags %x flags %x",
		 ap_ctx->txrx_stats.rx_rate.mode,
		 ap_ctx->txrx_stats.rx_rate.nss,
		 ap_ctx->txrx_stats.rx_rate.mcs,
		 ap_ctx->txrx_stats.rx_rate.rate_flags,
		 flags);
}

int wlan_hdd_get_station_remote(struct wiphy *wiphy,
				struct net_device *dev,
				const u8 *mac,
				struct station_info *sinfo);

/**
 * wlan_hdd_get_station_remote() - NL80211_CMD_GET_STATION handler for SoftAP
 * @wiphy: pointer to wiphy
 * @dev: pointer to net_device structure
 * @mac: request peer mac address
 * @sinfo: pointer to station_info struct
 *
 * This function will get remote peer info from fw and fill sinfo struct
 *
 * Return: 0 on success, otherwise error value
 */
int wlan_hdd_get_station_remote(struct wiphy *wiphy,
				struct net_device *dev,
				const u8 *mac,
				struct station_info *sinfo)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hddctx = wiphy_priv(wiphy);
	struct hdd_ap_ctx *ap_ctx = WLAN_HDD_GET_AP_CTX_PTR(adapter);
	struct hdd_station_info *stainfo = NULL;
	struct hdd_config *cfg;
	struct qdf_mac_addr macaddr;
	struct sir_peer_info_ext peer_info;
	int status;
	int i;

	status = wlan_hdd_validate_context(hddctx);
	if (status != 0)
		return status;

	cfg = hddctx->config;

	hdd_debug("get peer %pM info", mac);

	for (i = 0; i < WLAN_MAX_STA_COUNT; i++) {
		if (!qdf_mem_cmp(adapter->sta_info[i].sta_mac.bytes,
				 mac,
				 QDF_MAC_ADDR_SIZE)) {
			stainfo = &adapter->sta_info[i];
			break;
		}
	}

	if (!stainfo) {
		hdd_err("peer %pM not found", mac);
		return -EINVAL;
	}

	qdf_mem_copy(macaddr.bytes, mac, QDF_MAC_ADDR_SIZE);
	status = wlan_hdd_get_peer_info(adapter, macaddr, &peer_info);
	if (status) {
		hdd_err("fail to get peer info from fw");
		return -EPERM;
	}

	qdf_mem_zero(&ap_ctx->txrx_stats, sizeof(ap_ctx->txrx_stats));
	ap_ctx->txrx_stats.tx_packets = peer_info.tx_packets;
	ap_ctx->txrx_stats.tx_bytes = peer_info.tx_bytes;
	ap_ctx->txrx_stats.rx_packets = peer_info.rx_packets;
	ap_ctx->txrx_stats.rx_bytes = peer_info.rx_bytes;
	ap_ctx->txrx_stats.tx_retries = peer_info.tx_retries;
	ap_ctx->txrx_stats.tx_failed = peer_info.tx_failed;
	ap_ctx->txrx_stats.rssi =
		peer_info.rssi + WLAN_HDD_TGT_NOISE_FLOOR_DBM;
	wlan_hdd_fill_rate_info(ap_ctx, &peer_info);

	wlan_hdd_fill_station_info(hddctx->psoc, sinfo, stainfo,
				   &ap_ctx->txrx_stats);

	return status;
}

/**
 * hdd_report_max_rate() - Fill the max rate stats in the station info structure
 * to be sent to the userspace.
 *
 * @mac_handle: The mac handle
 * @sinfo: The station_info struct to be filled
 * @tx_rate_flags: The TX rate flags computed from tx rate
 * @tx_mcs_index; The TX mcs index computed from tx rate
 * @my_tx_rate: The tx_rate from fw stats
 * @tx_nss: The TX NSS from fw stats
 *
 * Return: 0 for success
 */
static int hdd_report_max_rate(mac_handle_t mac_handle,
			       struct station_info *sinfo,
			       uint8_t tx_rate_flags,
			       uint8_t tx_mcs_index,
			       uint16_t my_tx_rate, uint8_t tx_nss)
{
	uint8_t i, j, rssidx;
	uint16_t max_rate = 0;
	uint32_t vht_mcs_map;
	uint16_t current_rate = 0;
	qdf_size_t or_leng = CSR_DOT11_SUPPORTED_RATES_MAX;
	uint8_t operational_rates[CSR_DOT11_SUPPORTED_RATES_MAX];
	uint8_t extended_rates[CSR_DOT11_EXTENDED_SUPPORTED_RATES_MAX];
	qdf_size_t er_leng = CSR_DOT11_EXTENDED_SUPPORTED_RATES_MAX;
	uint8_t mcs_rates[SIZE_OF_BASIC_MCS_SET];
	qdf_size_t mcs_leng = SIZE_OF_BASIC_MCS_SET;
	struct index_vht_data_rate_type *supported_vht_mcs_rate;
	struct index_data_rate_type *supported_mcs_rate;
	enum data_rate_11ac_max_mcs vht_max_mcs;
	uint8_t max_speed_mcs = 0;
	uint8_t max_mcs_idx = 0;
	uint8_t rate_flag = 1;
	int mode = 0, max_ht_idx;
	QDF_STATUS stat = QDF_STATUS_E_FAILURE;
	struct hdd_context *hdd_ctx;
	int link_speed_rssi_high = 0;
	int link_speed_rssi_mid = 0;
	int link_speed_rssi_low = 0;
	uint32_t link_speed_rssi_report = 0;

	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	if (!hdd_ctx) {
		hdd_err("HDD context is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	ucfg_mlme_stats_get_cfg_values(hdd_ctx->psoc,
				       &link_speed_rssi_high,
				       &link_speed_rssi_mid,
				       &link_speed_rssi_low,
				       &link_speed_rssi_report);

	/* we do not want to necessarily report the current speed */
	if (ucfg_mlme_stats_is_link_speed_report_max(hdd_ctx->psoc)) {
		/* report the max possible speed */
		rssidx = 0;
	} else if (ucfg_mlme_stats_is_link_speed_report_max_scaled(
				hdd_ctx->psoc)) {
		/* report the max possible speed with RSSI scaling */
		if (sinfo->signal >= link_speed_rssi_high) {
			/* report the max possible speed */
			rssidx = 0;
		} else if (sinfo->signal >= link_speed_rssi_mid) {
			/* report middle speed */
			rssidx = 1;
		} else if (sinfo->signal >= link_speed_rssi_low) {
			/* report middle speed */
			rssidx = 2;
		} else {
			/* report actual speed */
			rssidx = 3;
		}
	} else {
		/* unknown, treat as eHDD_LINK_SPEED_REPORT_MAX */
		hdd_err("Invalid value for reportMaxLinkSpeed: %u",
			link_speed_rssi_report);
		rssidx = 0;
	}

	max_rate = 0;

	/* Get Basic Rate Set */
	if (0 != ucfg_mlme_get_opr_rate_set(hdd_ctx->psoc,
					    operational_rates, &or_leng)) {
		hdd_err("cfg get returned failure");
		/*To keep GUI happy */
		return 0;
	}

	for (i = 0; i < or_leng; i++) {
		for (j = 0;
			 j < ARRAY_SIZE(supported_data_rate); j++) {
			/* Validate Rate Set */
			if (supported_data_rate[j].beacon_rate_index ==
				(operational_rates[i] & 0x7F)) {
				current_rate =
					supported_data_rate[j].
					supported_rate[rssidx];
				break;
			}
		}
		/* Update MAX rate */
		max_rate = (current_rate > max_rate) ? current_rate : max_rate;
	}

	/* Get Extended Rate Set */
	if (0 != ucfg_mlme_get_ext_opr_rate_set(hdd_ctx->psoc,
						extended_rates,
						&er_leng)) {
		hdd_err("cfg get returned failure");
		/*To keep GUI happy */
		return 0;
	}

	for (i = 0; i < er_leng; i++) {
		for (j = 0; j < ARRAY_SIZE(supported_data_rate); j++) {
			if (supported_data_rate[j].beacon_rate_index ==
			    (extended_rates[i] & 0x7F)) {
				current_rate = supported_data_rate[j].
					       supported_rate[rssidx];
				break;
			}
		}
		/* Update MAX rate */
		max_rate = (current_rate > max_rate) ? current_rate : max_rate;
	}
	/* Get MCS Rate Set --
	 * Only if we are connected in non legacy mode and not reporting
	 * actual speed
	 */
	if ((3 != rssidx) && !(tx_rate_flags & TX_RATE_LEGACY)) {
		if (0 != ucfg_mlme_get_current_mcs_set(hdd_ctx->psoc,
						       mcs_rates,
						       &mcs_leng)) {
			hdd_err("cfg get returned failure");
			/*To keep GUI happy */
			return 0;
		}
		rate_flag = 0;
		supported_vht_mcs_rate = (struct index_vht_data_rate_type *)
					  ((tx_nss == 1) ?
					  &supported_vht_mcs_rate_nss1 :
					  &supported_vht_mcs_rate_nss2);

		if (tx_rate_flags & TX_RATE_VHT80)
			mode = 2;
		else if ((tx_rate_flags & TX_RATE_VHT40) ||
			 (tx_rate_flags & TX_RATE_HT40))
			mode = 1;
		else
			mode = 0;

		/* VHT80 rate has separate rate table */
		if (tx_rate_flags & (TX_RATE_VHT20 | TX_RATE_VHT40 |
		    TX_RATE_VHT80)) {
			stat = ucfg_mlme_cfg_get_vht_tx_mcs_map(hdd_ctx->psoc,
								&vht_mcs_map);
			if (QDF_IS_STATUS_ERROR(stat))
				hdd_err("failed to get tx_mcs_map");
			vht_max_mcs = (enum data_rate_11ac_max_mcs)
				(vht_mcs_map & DATA_RATE_11AC_MCS_MASK);
			if (tx_rate_flags & TX_RATE_SGI)
				rate_flag |= 1;

			if (DATA_RATE_11AC_MAX_MCS_7 == vht_max_mcs) {
				max_mcs_idx = 7;
			} else if (DATA_RATE_11AC_MAX_MCS_8 == vht_max_mcs) {
				max_mcs_idx = 8;
			} else if (DATA_RATE_11AC_MAX_MCS_9 == vht_max_mcs) {
				max_mcs_idx = 9;
			}

			if (rssidx != 0) {
				for (i = 0; i <= max_mcs_idx; i++) {
					if (sinfo->signal <=
						rssi_mcs_tbl[mode][i]) {
						max_mcs_idx = i;
						break;
					}
				}
			}

			if (tx_rate_flags & TX_RATE_VHT80) {
				current_rate =
				  supported_vht_mcs_rate[tx_mcs_index].
				  supported_VHT80_rate[rate_flag];
				max_rate =
				  supported_vht_mcs_rate[max_mcs_idx].
					supported_VHT80_rate[rate_flag];
			} else if (tx_rate_flags & TX_RATE_VHT40) {
				current_rate =
				  supported_vht_mcs_rate[tx_mcs_index].
				  supported_VHT40_rate[rate_flag];
				max_rate =
				  supported_vht_mcs_rate[max_mcs_idx].
					supported_VHT40_rate[rate_flag];
			} else if (tx_rate_flags & TX_RATE_VHT20) {
				current_rate =
				  supported_vht_mcs_rate[tx_mcs_index].
				  supported_VHT20_rate[rate_flag];
				max_rate =
				  supported_vht_mcs_rate[max_mcs_idx].
				  supported_VHT20_rate[rate_flag];
			}

			max_speed_mcs = 1;
			if (current_rate > max_rate)
				max_rate = current_rate;

		} else {
			if (tx_rate_flags & TX_RATE_HT40)
				rate_flag |= 1;
			if (tx_rate_flags & TX_RATE_SGI)
				rate_flag |= 2;

			supported_mcs_rate =
				(struct index_data_rate_type *)
				((tx_nss ==
				  1) ? &supported_mcs_rate_nss1 :
				 &supported_mcs_rate_nss2);

			max_ht_idx = MAX_HT_MCS_IDX;
			if (rssidx != 0) {
				for (i = 0; i < MAX_HT_MCS_IDX; i++) {
					if (sinfo->signal <=
						rssi_mcs_tbl[mode][i]) {
						max_ht_idx = i + 1;
						break;
					}
				}
			}

			for (i = 0; i < mcs_leng; i++) {
				for (j = 0; j < max_ht_idx; j++) {
					if (supported_mcs_rate[j].
						beacon_rate_index ==
						mcs_rates[i]) {
						current_rate =
						  supported_mcs_rate[j].
						  supported_rate
						  [rate_flag];
						max_mcs_idx =
						  supported_mcs_rate[j].
						  beacon_rate_index;
						break;
					}
				}

				if ((j < MAX_HT_MCS_IDX) &&
				    (current_rate > max_rate)) {
					max_rate = current_rate;
				}
				max_speed_mcs = 1;
			}
			if (tx_nss == 2)
				max_mcs_idx += MAX_HT_MCS_IDX;
		}
	}

	else if (!(tx_rate_flags & TX_RATE_LEGACY)) {
		max_rate = my_tx_rate;
		max_speed_mcs = 1;
		max_mcs_idx = tx_mcs_index;
	}
	/* report a value at least as big as current rate */
	if ((max_rate < my_tx_rate) || (0 == max_rate)) {
		max_rate = my_tx_rate;
		if (tx_rate_flags & TX_RATE_LEGACY) {
			max_speed_mcs = 0;
		} else {
			max_speed_mcs = 1;
			max_mcs_idx = tx_mcs_index;
			}
	}

	if (tx_rate_flags & TX_RATE_LEGACY) {
		sinfo->txrate.legacy = max_rate;

		hdd_info("Reporting legacy rate %d",
			 sinfo->txrate.legacy);
	} else {
		sinfo->txrate.mcs = max_mcs_idx;
		sinfo->txrate.nss = tx_nss;
		if (tx_rate_flags & TX_RATE_VHT80)
			hdd_set_rate_bw(&sinfo->txrate, HDD_RATE_BW_80);
		else if (tx_rate_flags & TX_RATE_VHT40)
			hdd_set_rate_bw(&sinfo->txrate, HDD_RATE_BW_40);
		else if (tx_rate_flags & TX_RATE_VHT20)
			hdd_set_rate_bw(&sinfo->txrate, HDD_RATE_BW_20);

		if (tx_rate_flags &
		    (TX_RATE_HT20 | TX_RATE_HT40)) {
			sinfo->txrate.flags |= RATE_INFO_FLAGS_MCS;
			if (tx_rate_flags & TX_RATE_HT40)
				hdd_set_rate_bw(&sinfo->txrate,
						HDD_RATE_BW_40);
			else if (tx_rate_flags & TX_RATE_HT20)
				hdd_set_rate_bw(&sinfo->txrate,
						HDD_RATE_BW_20);
		} else {
			sinfo->txrate.flags |= RATE_INFO_FLAGS_VHT_MCS;
		}

		if (tx_rate_flags & TX_RATE_SGI) {
			if (!
			    (sinfo->txrate.
			     flags & RATE_INFO_FLAGS_VHT_MCS))
				sinfo->txrate.flags |=
					RATE_INFO_FLAGS_MCS;
			sinfo->txrate.flags |= RATE_INFO_FLAGS_SHORT_GI;
		}
		linkspeed_dbg("Reporting MCS rate %d flags %x\n",
			      sinfo->txrate.mcs, sinfo->txrate.flags);
	}
	return 1;
}

/**
 * hdd_report_actual_rate() - Fill the actual rate stats.
 *
 * @rate_flags: The rate flags computed from rate
 * @my_rate: The rate from fw stats
 * @rate: The station_info struct member strust rate_info to be filled
 * @mcs_index; The mcs index computed from rate
 * @nss: The NSS from fw stats
 *
 * Return: None
 */
static void hdd_report_actual_rate(uint8_t rate_flags, uint16_t my_rate,
				   struct rate_info *rate, uint8_t mcs_index,
				   uint8_t nss)
{
	/* report current rate instead of max rate */

	if (rate_flags & TX_RATE_LEGACY) {
		/* provide to the UI in units of 100kbps */
		rate->legacy = my_rate;
		linkspeed_dbg("Reporting actual legacy rate %d",
			      rate->legacy);
	} else {
		/* must be MCS */
		rate->mcs = mcs_index;
		rate->nss = nss;

		if (rate_flags & TX_RATE_VHT80)
			hdd_set_rate_bw(rate, HDD_RATE_BW_80);
		else if (rate_flags & TX_RATE_VHT40)
			hdd_set_rate_bw(rate, HDD_RATE_BW_40);

		if (rate_flags &
			(TX_RATE_HT20 | TX_RATE_HT40)) {
			rate->flags |= RATE_INFO_FLAGS_MCS;
			if (rate_flags & TX_RATE_HT40)
				hdd_set_rate_bw(rate, HDD_RATE_BW_40);
		} else {
			rate->flags |= RATE_INFO_FLAGS_VHT_MCS;
		}

		if (rate_flags & TX_RATE_SGI) {
			rate->flags |= RATE_INFO_FLAGS_SHORT_GI;
		}

		linkspeed_dbg("Reporting actual MCS rate %d flags %x\n",
			      rate->mcs, rate->flags);
	}
}

/**
 * hdd_wlan_fill_per_chain_rssi_stats() - Fill per chain rssi stats
 *
 * @sinfo: The station_info structure to be filled.
 * @adapter: The HDD adapter structure
 *
 * Return: None
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
static inline
void hdd_wlan_fill_per_chain_rssi_stats(struct station_info *sinfo,
					struct hdd_adapter *adapter)
{
	bool rssi_stats_valid = false;
	uint8_t i;

	sinfo->signal_avg = WLAN_HDD_TGT_NOISE_FLOOR_DBM;
	for (i = 0; i < NUM_CHAINS_MAX; i++) {
		sinfo->chain_signal_avg[i] =
			   adapter->hdd_stats.per_chain_rssi_stats.rssi[i];
		sinfo->chains |= 1 << i;
		if (sinfo->chain_signal_avg[i] > sinfo->signal_avg &&
		    sinfo->chain_signal_avg[i] != 0)
			sinfo->signal_avg = sinfo->chain_signal_avg[i];

		hdd_debug("RSSI for chain %d, vdev_id %d is %d",
			  i, adapter->vdev_id, sinfo->chain_signal_avg[i]);

		if (!rssi_stats_valid && sinfo->chain_signal_avg[i])
			rssi_stats_valid = true;
	}

	if (rssi_stats_valid) {
		sinfo->filled |= HDD_INFO_CHAIN_SIGNAL_AVG;
		sinfo->filled |= HDD_INFO_SIGNAL_AVG;
	}
}

#else

static inline
void hdd_wlan_fill_per_chain_rssi_stats(struct station_info *sinfo,
					struct hdd_adapter *adapter)
{
}

#endif

/**
 * wlan_hdd_get_sta_stats() - get aggregate STA stats
 * @wiphy: wireless phy
 * @adapter: STA adapter to get stats for
 * @mac: mac address of sta
 * @sinfo: kernel station_info struct to populate
 *
 * Fetch the vdev-level aggregate stats for the given STA adapter. This is to
 * support "station dump" and "station get" for STA vdevs
 *
 * Return: errno
 */
static int wlan_hdd_get_sta_stats(struct wiphy *wiphy,
				  struct hdd_adapter *adapter,
				  const uint8_t *mac,
				  struct station_info *sinfo)
{
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	uint8_t rate_flags, tx_rate_flags, rx_rate_flags;
	uint8_t tx_mcs_index, rx_mcs_index;
	struct hdd_context *hdd_ctx = (struct hdd_context *) wiphy_priv(wiphy);
	mac_handle_t mac_handle;
	int8_t snr = 0;
	uint16_t my_tx_rate, my_rx_rate;
	uint8_t tx_nss = 1, rx_nss = 1;
	int32_t rcpi_value;
	int link_speed_rssi_high = 0;
	int link_speed_rssi_mid = 0;
	int link_speed_rssi_low = 0;
	uint32_t link_speed_rssi_report = 0;

	qdf_mtrace(QDF_MODULE_ID_HDD, QDF_MODULE_ID_HDD,
		   TRACE_CODE_HDD_CFG80211_GET_STA,
		   adapter->vdev_id, 0);

	if (eConnectionState_Associated != sta_ctx->conn_info.conn_state) {
		hdd_debug("Not associated");
		/*To keep GUI happy */
		return 0;
	}

	if (sta_ctx->hdd_reassoc_scenario) {
		hdd_debug("Roaming is in progress, cannot continue with this request");
		/*
		 * supplicant reports very low rssi to upper layer
		 * and handover happens to cellular.
		 * send the cached rssi when get_station
		 */
		sinfo->signal = adapter->rssi;
		sinfo->filled |= HDD_INFO_SIGNAL;
		return 0;
	}

	ucfg_mlme_stats_get_cfg_values(hdd_ctx->psoc,
				       &link_speed_rssi_high,
				       &link_speed_rssi_mid,
				       &link_speed_rssi_low,
				       &link_speed_rssi_report);

	if (hdd_ctx->rcpi_enabled)
		wlan_hdd_get_rcpi(adapter, (uint8_t *)mac, &rcpi_value,
				  RCPI_MEASUREMENT_TYPE_AVG_MGMT);

	wlan_hdd_get_station_stats(adapter);

	adapter->rssi = adapter->hdd_stats.summary_stat.rssi;
	snr = adapter->hdd_stats.summary_stat.snr;

	/* for new connection there might be no valid previous RSSI */
	if (!adapter->rssi) {
		hdd_get_rssi_snr_by_bssid(adapter,
				sta_ctx->conn_info.bssid.bytes,
				&adapter->rssi, &snr);
	}

	sinfo->signal = adapter->rssi;
	hdd_debug("snr: %d, rssi: %d",
		adapter->hdd_stats.summary_stat.snr,
		adapter->hdd_stats.summary_stat.rssi);
	sta_ctx->conn_info.signal = sinfo->signal;
	sta_ctx->conn_info.noise =
		sta_ctx->conn_info.signal - snr;
	sta_ctx->cache_conn_info.signal = sinfo->signal;
	sta_ctx->cache_conn_info.noise = sta_ctx->conn_info.noise;
	sinfo->filled |= HDD_INFO_SIGNAL;

	/*
	 * we notify connect to lpass here instead of during actual
	 * connect processing because rssi info is not accurate during
	 * actual connection.  lpass will ensure the notification is
	 * only processed once per association.
	 */
	hdd_lpass_notify_connect(adapter);

	rate_flags = adapter->hdd_stats.class_a_stat.tx_rx_rate_flags;
	tx_rate_flags = rx_rate_flags = rate_flags;

	tx_mcs_index = adapter->hdd_stats.class_a_stat.tx_mcs_index;
	rx_mcs_index = adapter->hdd_stats.class_a_stat.rx_mcs_index;
	mac_handle = hdd_ctx->mac_handle;

	/* convert to the UI units of 100kbps */
	my_tx_rate = adapter->hdd_stats.class_a_stat.tx_rate;
	my_rx_rate = adapter->hdd_stats.class_a_stat.rx_rate;

	if (!(rate_flags & TX_RATE_LEGACY)) {
		tx_nss = adapter->hdd_stats.class_a_stat.tx_nss;
		rx_nss = adapter->hdd_stats.class_a_stat.rx_nss;

		if ((tx_nss > 1) &&
		    policy_mgr_is_current_hwmode_dbs(hdd_ctx->psoc) &&
		    !policy_mgr_is_hw_dbs_2x2_capable(hdd_ctx->psoc)) {
			hdd_debug("Hw mode is DBS, Reduce nss(%d) to 1",
				  tx_nss);
			tx_nss--;
		}

		if ((rx_nss > 1) &&
		    policy_mgr_is_current_hwmode_dbs(hdd_ctx->psoc) &&
		    !policy_mgr_is_hw_dbs_2x2_capable(hdd_ctx->psoc)) {
			hdd_debug("Hw mode is DBS, Reduce nss(%d) to 1",
				  rx_nss);
			rx_nss--;
		}

		if (ucfg_mlme_stats_is_link_speed_report_actual(
					hdd_ctx->psoc)) {
			/* Get current rate flags if report actual */
			/* WMA fails to find mcs_index for legacy tx rates */
			if (tx_mcs_index == INVALID_MCS_IDX && my_tx_rate)
				tx_rate_flags = TX_RATE_LEGACY;
			else
				tx_rate_flags =
			      adapter->hdd_stats.class_a_stat.tx_mcs_rate_flags;

			if (rx_mcs_index == INVALID_MCS_IDX && my_rx_rate)
				rx_rate_flags = TX_RATE_LEGACY;
			else
				rx_rate_flags =
			      adapter->hdd_stats.class_a_stat.rx_mcs_rate_flags;
		}

		if (tx_mcs_index == INVALID_MCS_IDX)
			tx_mcs_index = 0;
		if (rx_mcs_index == INVALID_MCS_IDX)
			rx_mcs_index = 0;
	}

	hdd_debug("RSSI %d, RLMS %u, rssi high %d, rssi mid %d, rssi low %d",
		  sinfo->signal, link_speed_rssi_report,
		  link_speed_rssi_high, link_speed_rssi_mid,
		  link_speed_rssi_low);
	hdd_debug("Rate info: TX: %d, RX: %d", my_tx_rate, my_rx_rate);
	hdd_debug("Rate flags: TX: 0x%x, RX: 0x%x", (int)tx_rate_flags,
		  (int)rx_rate_flags);
	hdd_debug("MCS Index: TX: %d, RX: %d", (int)tx_mcs_index,
		  (int)rx_mcs_index);
	hdd_debug("NSS: TX: %d, RX: %d", (int)tx_nss, (int)rx_nss);

	/* assume basic BW. anything else will override this later */
	hdd_set_rate_bw(&sinfo->txrate, HDD_RATE_BW_20);

	if (!ucfg_mlme_stats_is_link_speed_report_actual(hdd_ctx->psoc)) {
		if (!hdd_report_max_rate(mac_handle, sinfo,
					 tx_rate_flags, tx_mcs_index,
					 my_tx_rate, tx_nss)) {
			/* Keep GUI happy */
			return 0;
		}
	} else {

		/* Fill TX stats */
		hdd_report_actual_rate(tx_rate_flags, my_tx_rate,
				       &sinfo->txrate, tx_mcs_index, tx_nss);

		/* Fill RX stats */
		hdd_report_actual_rate(rx_rate_flags, my_rx_rate,
				       &sinfo->rxrate, rx_mcs_index, rx_nss);
	}

	wlan_hdd_fill_summary_stats(&adapter->hdd_stats.summary_stat, sinfo);
	sinfo->tx_bytes = adapter->stats.tx_bytes;
	sinfo->rx_bytes = adapter->hdd_stats.peer_stats.rx_bytes;
	sinfo->rx_packets = adapter->hdd_stats.peer_stats.rx_count;

	qdf_mem_copy(&sta_ctx->conn_info.txrate,
		     &sinfo->txrate, sizeof(sinfo->txrate));
	qdf_mem_copy(&sta_ctx->cache_conn_info.txrate,
		     &sinfo->txrate, sizeof(sinfo->txrate));

	sinfo->filled |= HDD_INFO_TX_BITRATE |
			 HDD_INFO_RX_BITRATE |
			 HDD_INFO_TX_BYTES   |
			 HDD_INFO_RX_BYTES   |
			 HDD_INFO_RX_PACKETS;

	if (tx_rate_flags & TX_RATE_LEGACY)
		hdd_debug("Reporting legacy rate %d pkt cnt tx %d rx %d",
			sinfo->txrate.legacy, sinfo->tx_packets,
			sinfo->rx_packets);
	else
		hdd_debug("Reporting MCS rate %d flags 0x%x pkt cnt tx %d rx %d",
			sinfo->txrate.mcs, sinfo->txrate.flags,
			sinfo->tx_packets, sinfo->rx_packets);

	hdd_wlan_fill_per_chain_rssi_stats(sinfo, adapter);

	hdd_exit();

	return 0;
}

/**
 * __wlan_hdd_cfg80211_get_station() - get station statistics
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @mac: Pointer to mac
 * @sinfo: Pointer to station info
 *
 * Return: 0 for success, non-zero for failure
 */
static int __wlan_hdd_cfg80211_get_station(struct wiphy *wiphy,
					   struct net_device *dev,
					   const uint8_t *mac,
					   struct station_info *sinfo)
{
	int status;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = (struct hdd_context *) wiphy_priv(wiphy);

	hdd_enter_dev(dev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	status = wlan_hdd_validate_context(hdd_ctx);
	if (status)
		return status;

	if (wlan_hdd_validate_vdev_id(adapter->vdev_id))
		return -EINVAL;

	if (adapter->device_mode == QDF_SAP_MODE)
		return wlan_hdd_get_sap_stats(adapter, sinfo);
	else
		return wlan_hdd_get_sta_stats(wiphy, adapter, mac, sinfo);
}

/**
 * wlan_hdd_cfg80211_get_station() - get station statistics
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @mac: Pointer to mac
 * @sinfo: Pointer to station info
 *
 * Return: 0 for success, non-zero for failure
 */
int wlan_hdd_cfg80211_get_station(struct wiphy *wiphy,
				  struct net_device *dev, const uint8_t *mac,
				  struct station_info *sinfo)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(dev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_get_station(wiphy, dev, mac, sinfo);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}

/**
 * __wlan_hdd_cfg80211_dump_station() - dump station statistics
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @idx: variable to determine whether to get stats or not
 * @mac: Pointer to mac
 * @sinfo: Pointer to station info
 *
 * Return: 0 for success, non-zero for failure
 */
static int __wlan_hdd_cfg80211_dump_station(struct wiphy *wiphy,
				struct net_device *dev,
				int idx, u8 *mac,
				struct station_info *sinfo)
{
	struct hdd_context *hdd_ctx = (struct hdd_context *) wiphy_priv(wiphy);

	hdd_debug("%s: idx %d", __func__, idx);
	if (idx != 0)
		return -ENOENT;
	if (hdd_ctx->num_provisioned_addr)
		qdf_mem_copy(mac, hdd_ctx->provisioned_mac_addr[0].bytes,
			     QDF_MAC_ADDR_SIZE);
	else
		qdf_mem_copy(mac, hdd_ctx->derived_mac_addr[0].bytes,
			     QDF_MAC_ADDR_SIZE);
	return __wlan_hdd_cfg80211_get_station(wiphy, dev, mac, sinfo);
}

/**
 * wlan_hdd_cfg80211_dump_station() - dump station statistics
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @idx: variable to determine whether to get stats or not
 * @mac: Pointer to mac
 * @sinfo: Pointer to station info
 *
 * Return: 0 for success, non-zero for failure
 */
int wlan_hdd_cfg80211_dump_station(struct wiphy *wiphy,
				struct net_device *dev,
				int idx, u8 *mac,
				struct station_info *sinfo)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(dev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_dump_station(wiphy, dev, idx, mac, sinfo);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}

/**
 * hdd_get_stats() - Function to retrieve interface statistics
 * @dev: pointer to network device
 *
 * This function is the ndo_get_stats method for all netdevs
 * registered with the kernel
 *
 * Return: pointer to net_device_stats structure
 */
struct net_device_stats *hdd_get_stats(struct net_device *dev)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);

	hdd_enter_dev(dev);
	return &adapter->stats;
}


/*
 * time = cycle_count * cycle
 * cycle = 1 / clock_freq
 * Since the unit of clock_freq reported from
 * FW is MHZ, and we want to calculate time in
 * ms level, the result is
 * time = cycle / (clock_freq * 1000)
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
static bool wlan_fill_survey_result(struct survey_info *survey, int opfreq,
				    struct scan_chan_info *chan_info,
				    struct ieee80211_channel *channels)
{
	uint64_t clock_freq = chan_info->clock_freq * 1000;

	if (channels->center_freq != (uint16_t)chan_info->freq)
		return false;

	survey->channel = channels;
	survey->noise = chan_info->noise_floor;
	survey->filled = SURVEY_INFO_NOISE_DBM;

	if (opfreq == chan_info->freq)
		survey->filled |= SURVEY_INFO_IN_USE;

	if (clock_freq == 0)
		return true;

	survey->time = qdf_do_div(chan_info->cycle_count, clock_freq);

	survey->time_busy = qdf_do_div(chan_info->rx_clear_count, clock_freq);

	survey->time_tx = qdf_do_div(chan_info->tx_frame_count, clock_freq);

	survey->filled |= SURVEY_INFO_TIME |
			  SURVEY_INFO_TIME_BUSY |
			  SURVEY_INFO_TIME_TX;
	return true;
}
#else
static bool wlan_fill_survey_result(struct survey_info *survey, int opfreq,
				    struct scan_chan_info *chan_info,
				    struct ieee80211_channel *channels)
{
	uint64_t clock_freq = chan_info->clock_freq * 1000;

	if (channels->center_freq != (uint16_t)chan_info->freq)
		return false;

	survey->channel = channels;
	survey->noise = chan_info->noise_floor;
	survey->filled = SURVEY_INFO_NOISE_DBM;

	if (opfreq == chan_info->freq)
		survey->filled |= SURVEY_INFO_IN_USE;

	if (clock_freq == 0)
		return true;

	survey->channel_time = qdf_do_div(chan_info->cycle_count, clock_freq);

	survey->channel_time_busy = qdf_do_div(chan_info->rx_clear_count,
							 clock_freq);

	survey->channel_time_tx = qdf_do_div(chan_info->tx_frame_count,
							 clock_freq);

	survey->filled |= SURVEY_INFO_CHANNEL_TIME |
			  SURVEY_INFO_CHANNEL_TIME_BUSY |
			  SURVEY_INFO_CHANNEL_TIME_TX;
	return true;
}
#endif

static bool wlan_hdd_update_survey_info(struct wiphy *wiphy,
					struct hdd_adapter *adapter,
					struct survey_info *survey, int idx)
{
	bool filled = false;
	int i, j = 0;
	uint32_t channel = 0, opfreq; /* Initialization Required */
	struct hdd_context *hdd_ctx;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	sme_get_operation_channel(hdd_ctx->mac_handle, &channel,
				  adapter->vdev_id);
	opfreq = wlan_reg_chan_to_freq(hdd_ctx->pdev, channel);

	mutex_lock(&hdd_ctx->chan_info_lock);

	for (i = 0; i < HDD_NUM_NL80211_BANDS && !filled; i++) {
		if (!wiphy->bands[i])
			continue;

		for (j = 0; j < wiphy->bands[i]->n_channels && !filled; j++) {
			struct ieee80211_supported_band *band = wiphy->bands[i];

			filled = wlan_fill_survey_result(survey, opfreq,
				&hdd_ctx->chan_info[idx],
				&band->channels[j]);
		}
	}
	mutex_unlock(&hdd_ctx->chan_info_lock);

	return filled;
}

/**
 * __wlan_hdd_cfg80211_dump_survey() - get survey related info
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @idx: Index
 * @survey: Pointer to survey info
 *
 * Return: 0 for success, non-zero for failure
 */
static int __wlan_hdd_cfg80211_dump_survey(struct wiphy *wiphy,
					   struct net_device *dev,
					   int idx, struct survey_info *survey)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx;
	struct hdd_station_ctx *sta_ctx;
	int status;
	bool filled = false;

	hdd_enter_dev(dev);

	hdd_debug("dump survey index: %d", idx);
	if (idx > QDF_MAX_NUM_CHAN - 1)
		return -EINVAL;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	status = wlan_hdd_validate_context(hdd_ctx);
	if (0 != status)
		return status;

	if (!hdd_ctx->chan_info) {
		hdd_debug("chan_info is NULL");
		return -EINVAL;
	}

	if (hdd_get_conparam() == QDF_GLOBAL_FTM_MODE) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	if (!hdd_ctx->config->enable_snr_monitoring)
		return -ENONET;

	if (sta_ctx->hdd_reassoc_scenario) {
		hdd_info("Roaming in progress, hence return");
		return -ENONET;
	}

	filled = wlan_hdd_update_survey_info(wiphy, adapter, survey, idx);

	if (!filled)
		return -ENONET;
	hdd_exit();
	return 0;
}

/**
 * wlan_hdd_cfg80211_dump_survey() - get survey related info
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @idx: Index
 * @survey: Pointer to survey info
 *
 * Return: 0 for success, non-zero for failure
 */
int wlan_hdd_cfg80211_dump_survey(struct wiphy *wiphy,
				  struct net_device *dev,
				  int idx, struct survey_info *survey)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(dev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_dump_survey(wiphy, dev, idx, survey);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}

/**
 * hdd_display_hif_stats() - display hif stats
 *
 * Return: none
 *
 */
void hdd_display_hif_stats(void)
{
	void *hif_ctx = cds_get_context(QDF_MODULE_ID_HIF);

	if (!hif_ctx)
		return;

	hif_display_stats(hif_ctx);
}

/**
 * hdd_clear_hif_stats() - clear hif stats
 *
 * Return: none
 */
void hdd_clear_hif_stats(void)
{
	void *hif_ctx = cds_get_context(QDF_MODULE_ID_HIF);

	if (!hif_ctx)
		return;
	hif_clear_stats(hif_ctx);
}

/**
 * hdd_is_rcpi_applicable() - validates RCPI request
 * @adapter: adapter upon which the measurement is requested
 * @mac_addr: peer addr for which measurement is requested
 * @rcpi_value: pointer to where the RCPI should be returned
 * @reassoc: used to return cached RCPI during reassoc
 *
 * Return: true for success, false for failure
 */

static bool hdd_is_rcpi_applicable(struct hdd_adapter *adapter,
				   struct qdf_mac_addr *mac_addr,
				   int32_t *rcpi_value,
				   bool *reassoc)
{
	struct hdd_station_ctx *hdd_sta_ctx;

	if (adapter->device_mode == QDF_STA_MODE ||
	    adapter->device_mode == QDF_P2P_CLIENT_MODE) {
		hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
		if (hdd_sta_ctx->conn_info.conn_state !=
		    eConnectionState_Associated)
			return false;

		if (hdd_sta_ctx->hdd_reassoc_scenario) {
			/* return the cached rcpi, if mac addr matches */
			hdd_debug("Roaming in progress, return cached RCPI");
			if (!qdf_mem_cmp(&adapter->rcpi.mac_addr,
					 mac_addr, sizeof(*mac_addr))) {
				*rcpi_value = adapter->rcpi.rcpi;
				*reassoc = true;
				return true;
			}
			return false;
		}

		if (qdf_mem_cmp(mac_addr, &hdd_sta_ctx->conn_info.bssid,
				sizeof(*mac_addr))) {
			hdd_err("mac addr is different from bssid connected");
			return false;
		}
	} else if (adapter->device_mode == QDF_SAP_MODE ||
		   adapter->device_mode == QDF_P2P_GO_MODE) {
		if (!test_bit(SOFTAP_BSS_STARTED, &adapter->event_flags)) {
			hdd_err("Invalid rcpi request, softap not started");
			return false;
		}

		/* check if peer mac addr is associated to softap */
		if (!hdd_is_peer_associated(adapter, mac_addr)) {
			hdd_err("invalid peer mac-addr: not associated");
			return false;
		}
	} else {
		hdd_err("Invalid rcpi request");
		return false;
	}

	*reassoc = false;
	return true;
}

/**
 * wlan_hdd_get_rcpi_cb() - callback function for rcpi response
 * @context: Pointer to rcpi context
 * @rcpi_req: Pointer to rcpi response
 *
 * Return: None
 */
static void wlan_hdd_get_rcpi_cb(void *context, struct qdf_mac_addr mac_addr,
				 int32_t rcpi, QDF_STATUS status)
{
	struct osif_request *request;
	struct rcpi_info *priv;

	if (!context) {
		hdd_err("No rcpi context");
		return;
	}

	request = osif_request_get(context);
	if (!request) {
		hdd_err("Obsolete RCPI request");
		return;
	}

	priv = osif_request_priv(request);
	priv->mac_addr = mac_addr;

	if (!QDF_IS_STATUS_SUCCESS(status)) {
		priv->rcpi = 0;
		hdd_err("Error in computing RCPI");
	} else {
		priv->rcpi = rcpi;
	}

	osif_request_complete(request);
	osif_request_put(request);
}

/**
 * wlan_hdd_get_rcpi() - local function to get RCPI
 * @adapter: adapter upon which the measurement is requested
 * @mac: peer addr for which measurement is requested
 * @rcpi_value: pointer to where the RCPI should be returned
 * @measurement_type: type of rcpi measurement
 *
 * Return: 0 for success, non-zero for failure
 */
int wlan_hdd_get_rcpi(struct hdd_adapter *adapter,
		      uint8_t *mac,
		      int32_t *rcpi_value,
		      enum rcpi_measurement_type measurement_type)
{
	struct hdd_context *hdd_ctx;
	int status = 0, ret = 0;
	struct qdf_mac_addr mac_addr;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	struct sme_rcpi_req *rcpi_req;
	void *cookie;
	struct rcpi_info *priv;
	struct osif_request *request;
	static const struct osif_request_params params = {
		.priv_size = sizeof(*priv),
		.timeout_ms = WLAN_WAIT_TIME_RCPI,
	};
	bool reassoc;

	hdd_enter();

	/* initialize the rcpi value to zero, useful in error cases */
	*rcpi_value = 0;

	if (hdd_get_conparam() == QDF_GLOBAL_FTM_MODE) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (!adapter) {
		hdd_warn("adapter context is NULL");
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	status = wlan_hdd_validate_context(hdd_ctx);
	if (status)
		return -EINVAL;

	if (!hdd_ctx->rcpi_enabled) {
		hdd_debug("RCPI not supported");
		return -EINVAL;
	}

	if (!mac) {
		hdd_warn("RCPI peer mac-addr is NULL");
		return -EINVAL;
	}

	qdf_mem_copy(&mac_addr, mac, QDF_MAC_ADDR_SIZE);

	if (!hdd_is_rcpi_applicable(adapter, &mac_addr, rcpi_value, &reassoc))
		return -EINVAL;
	if (reassoc)
		return 0;

	rcpi_req = qdf_mem_malloc(sizeof(*rcpi_req));
	if (!rcpi_req)
		return -EINVAL;

	request = osif_request_alloc(&params);
	if (!request) {
		hdd_err("Request allocation failure");
		qdf_mem_free(rcpi_req);
		return -ENOMEM;
	}
	cookie = osif_request_cookie(request);

	rcpi_req->mac_addr = mac_addr;
	rcpi_req->session_id = adapter->vdev_id;
	rcpi_req->measurement_type = measurement_type;
	rcpi_req->rcpi_callback = wlan_hdd_get_rcpi_cb;
	rcpi_req->rcpi_context = cookie;

	qdf_status = sme_get_rcpi(hdd_ctx->mac_handle, rcpi_req);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		hdd_err("Unable to retrieve RCPI");
		status = qdf_status_to_os_return(qdf_status);
		goto out;
	}

	/* request was sent -- wait for the response */
	ret = osif_request_wait_for_response(request);
	if (ret) {
		hdd_err("SME timed out while retrieving RCPI");
		status = -EINVAL;
		goto out;
	}

	/* update the adapter with the fresh results */
	priv = osif_request_priv(request);
	adapter->rcpi.mac_addr = priv->mac_addr;
	adapter->rcpi.rcpi = priv->rcpi;
	if (qdf_mem_cmp(&mac_addr, &priv->mac_addr, sizeof(mac_addr))) {
		hdd_err("mis match of mac addr from call-back");
		status = -EINVAL;
		goto out;
	}

	*rcpi_value = adapter->rcpi.rcpi;
	hdd_debug("RCPI = %d", *rcpi_value);
out:
	qdf_mem_free(rcpi_req);
	osif_request_put(request);

	hdd_exit();
	return status;
}

#ifdef QCA_SUPPORT_CP_STATS
QDF_STATUS wlan_hdd_get_rssi(struct hdd_adapter *adapter, int8_t *rssi_value)
{
	int ret = 0, i;
	struct hdd_station_ctx *sta_ctx;
	struct stats_event *rssi_info;

	if (!adapter) {
		hdd_err("Invalid context, adapter");
		return QDF_STATUS_E_FAULT;
	}
	if (cds_is_driver_recovering() || cds_is_driver_in_bad_state()) {
		hdd_err("Recovery in Progress. State: 0x%x Ignore!!!",
			cds_get_driver_state());
		/* return a cached value */
		*rssi_value = adapter->rssi;
		return QDF_STATUS_SUCCESS;
	}

	sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	if (eConnectionState_Associated != sta_ctx->conn_info.conn_state) {
		hdd_debug("Not associated!, rssi on disconnect %d",
			  adapter->rssi_on_disconnect);
		*rssi_value = adapter->rssi_on_disconnect;
		return QDF_STATUS_SUCCESS;
	}

	if (sta_ctx->hdd_reassoc_scenario) {
		hdd_debug("Roaming in progress, return cached RSSI");
		*rssi_value = adapter->rssi;
		return QDF_STATUS_SUCCESS;
	}

	rssi_info = wlan_cfg80211_mc_cp_stats_get_peer_rssi(
			adapter->vdev,
			sta_ctx->conn_info.bssid.bytes,
			&ret);
	if (ret || !rssi_info) {
		wlan_cfg80211_mc_cp_stats_free_stats_event(rssi_info);
		return ret;
	}

	for (i = 0; i < rssi_info->num_peer_stats; i++) {
		if (!qdf_mem_cmp(rssi_info->peer_stats[i].peer_macaddr,
				 sta_ctx->conn_info.bssid.bytes,
				 QDF_MAC_ADDR_SIZE)) {
			*rssi_value = rssi_info->peer_stats[i].peer_rssi;
			hdd_debug("RSSI = %d", *rssi_value);
			wlan_cfg80211_mc_cp_stats_free_stats_event(rssi_info);
			return QDF_STATUS_SUCCESS;
		}
	}

	wlan_cfg80211_mc_cp_stats_free_stats_event(rssi_info);
	hdd_err("bss peer not present in returned result");
	return QDF_STATUS_E_FAULT;
}
#else /* QCA_SUPPORT_CP_STATS */
struct rssi_priv {
	int8_t rssi;
};

/**
 * hdd_get_rssi_cb() - "Get RSSI" callback function
 * @rssi: Current RSSI of the station
 * @sta_id: ID of the station
 * @context: opaque context originally passed to SME.  HDD always passes
 *	a cookie for the request context
 *
 * Return: None
 */
static void hdd_get_rssi_cb(int8_t rssi, uint32_t sta_id, void *context)
{
	struct osif_request *request;
	struct rssi_priv *priv;

	request = osif_request_get(context);
	if (!request) {
		hdd_err("Obsolete request");
		return;
	}

	priv = osif_request_priv(request);
	priv->rssi = rssi;
	osif_request_complete(request);
	osif_request_put(request);
}

QDF_STATUS wlan_hdd_get_rssi(struct hdd_adapter *adapter, int8_t *rssi_value)
{
	struct hdd_context *hdd_ctx;
	struct hdd_station_ctx *sta_ctx;
	QDF_STATUS status;
	int ret;
	void *cookie;
	struct osif_request *request;
	struct rssi_priv *priv;
	static const struct osif_request_params params = {
		.priv_size = sizeof(*priv),
		.timeout_ms = WLAN_WAIT_TIME_STATS,
	};

	if (!adapter) {
		hdd_err("Invalid context, adapter");
		return QDF_STATUS_E_FAULT;
	}
	if (cds_is_driver_recovering() || cds_is_driver_in_bad_state()) {
		hdd_err("Recovery in Progress. State: 0x%x Ignore!!!",
			cds_get_driver_state());
		/* return a cached value */
		*rssi_value = adapter->rssi;
		return QDF_STATUS_SUCCESS;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	if (eConnectionState_Associated != sta_ctx->conn_info.conn_state) {
		hdd_debug("Not associated!, rssi on disconnect %d",
			adapter->rssi_on_disconnect);
		*rssi_value = adapter->rssi_on_disconnect;
		return QDF_STATUS_SUCCESS;
	}

	if (sta_ctx->hdd_reassoc_scenario) {
		hdd_debug("Roaming in progress, return cached RSSI");
		*rssi_value = adapter->rssi;
		return QDF_STATUS_SUCCESS;
	}

	request = osif_request_alloc(&params);
	if (!request) {
		hdd_err("Request allocation failure, return cached RSSI");
		*rssi_value = adapter->rssi;
		return QDF_STATUS_SUCCESS;
	}
	cookie = osif_request_cookie(request);

	status = sme_get_rssi(hdd_ctx->mac_handle, hdd_get_rssi_cb,
			      sta_ctx->conn_info.sta_id[0],
			      sta_ctx->conn_info.bssid, adapter->rssi,
			      cookie);
	if (QDF_STATUS_SUCCESS != status) {
		hdd_err("Unable to retrieve RSSI");
		/* we'll returned a cached value below */
	} else {
		/* request was sent -- wait for the response */
		ret = osif_request_wait_for_response(request);
		if (ret) {
			hdd_warn("SME timed out while retrieving RSSI");
			/* we'll returned a cached value below */
		} else {
			/* update the adapter with the fresh results */
			priv = osif_request_priv(request);

			adapter->rssi = priv->rssi;

			/*
			 * for new connection there might be no valid previous
			 * RSSI.
			 */
			if (!adapter->rssi) {
				hdd_get_rssi_snr_by_bssid(adapter,
					sta_ctx->conn_info.bssid.bytes,
					&adapter->rssi, NULL);
			}
		}
	}

	/*
	 * either we never sent a request, we sent a request and
	 * received a response or we sent a request and timed out.
	 * regardless we are done with the request.
	 */
	osif_request_put(request);

	*rssi_value = adapter->rssi;
	hdd_debug("RSSI = %d", *rssi_value);

	return QDF_STATUS_SUCCESS;
}
#endif /* QCA_SUPPORT_CP_STATS */

struct snr_priv {
	int8_t snr;
};

/**
 * hdd_get_snr_cb() - "Get SNR" callback function
 * @snr: Current SNR of the station
 * @sta_id: ID of the station
 * @context: opaque context originally passed to SME.  HDD always passes
 *	a cookie for the request context
 *
 * Return: None
 */
static void hdd_get_snr_cb(int8_t snr, uint32_t sta_id, void *context)
{
	struct osif_request *request;
	struct snr_priv *priv;

	request = osif_request_get(context);
	if (!request) {
		hdd_err("Obsolete request");
		return;
	}

	/* propagate response back to requesting thread */
	priv = osif_request_priv(request);
	priv->snr = snr;
	osif_request_complete(request);
	osif_request_put(request);
}

QDF_STATUS wlan_hdd_get_snr(struct hdd_adapter *adapter, int8_t *snr)
{
	struct hdd_context *hdd_ctx;
	struct hdd_station_ctx *sta_ctx;
	QDF_STATUS status;
	int ret;
	void *cookie;
	struct osif_request *request;
	struct snr_priv *priv;
	static const struct osif_request_params params = {
		.priv_size = sizeof(*priv),
		.timeout_ms = WLAN_WAIT_TIME_STATS,
	};

	hdd_enter();

	if (!adapter) {
		hdd_err("Invalid context, adapter");
		return QDF_STATUS_E_FAULT;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return QDF_STATUS_E_FAULT;

	sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	request = osif_request_alloc(&params);
	if (!request) {
		hdd_err("Request allocation failure");
		return QDF_STATUS_E_FAULT;
	}
	cookie = osif_request_cookie(request);

	status = sme_get_snr(hdd_ctx->mac_handle, hdd_get_snr_cb,
			     sta_ctx->conn_info.sta_id[0],
			     sta_ctx->conn_info.bssid, cookie);
	if (QDF_STATUS_SUCCESS != status) {
		hdd_err("Unable to retrieve RSSI");
		/* we'll returned a cached value below */
	} else {
		/* request was sent -- wait for the response */
		ret = osif_request_wait_for_response(request);
		if (ret) {
			hdd_err("SME timed out while retrieving SNR");
			/* we'll now returned a cached value below */
		} else {
			/* update the adapter with the fresh results */
			priv = osif_request_priv(request);
			adapter->snr = priv->snr;
		}
	}

	/*
	 * either we never sent a request, we sent a request and
	 * received a response or we sent a request and timed out.
	 * regardless we are done with the request.
	 */
	osif_request_put(request);

	*snr = adapter->snr;
	hdd_exit();
	return QDF_STATUS_SUCCESS;
}

struct linkspeed_priv {
	struct link_speed_info linkspeed_info;
};

static void
hdd_get_link_speed_cb(struct link_speed_info *linkspeed_info, void *context)
{
	struct osif_request *request;
	struct linkspeed_priv *priv;

	if (!linkspeed_info) {
		hdd_err("NULL linkspeed");
		return;
	}

	request = osif_request_get(context);
	if (!request) {
		hdd_err("Obsolete request");
		return;
	}

	priv = osif_request_priv(request);
	priv->linkspeed_info = *linkspeed_info;
	osif_request_complete(request);
	osif_request_put(request);
}

int wlan_hdd_get_linkspeed_for_peermac(struct hdd_adapter *adapter,
				       struct qdf_mac_addr *mac_address,
				       uint32_t *linkspeed)
{
	int ret;
	QDF_STATUS status;
	void *cookie;
	struct link_speed_info *linkspeed_info;
	struct osif_request *request;
	struct linkspeed_priv *priv;
	static const struct osif_request_params params = {
		.priv_size = sizeof(*priv),
		.timeout_ms = WLAN_WAIT_TIME_STATS,
	};

	if ((!adapter) || (!linkspeed)) {
		hdd_err("NULL argument");
		return -EINVAL;
	}

	request = osif_request_alloc(&params);
	if (!request) {
		hdd_err("Request allocation failure");
		ret = -ENOMEM;
		goto return_cached_value;
	}

	cookie = osif_request_cookie(request);
	priv = osif_request_priv(request);

	linkspeed_info = &priv->linkspeed_info;
	qdf_copy_macaddr(&linkspeed_info->peer_macaddr, mac_address);
	status = sme_get_link_speed(adapter->hdd_ctx->mac_handle,
				    linkspeed_info,
				    cookie, hdd_get_link_speed_cb);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("Unable to retrieve statistics for link speed");
		ret = qdf_status_to_os_return(status);
		goto cleanup;
	}
	ret = osif_request_wait_for_response(request);
	if (ret) {
		hdd_err("SME timed out while retrieving link speed");
		goto cleanup;
	}
	adapter->estimated_linkspeed = linkspeed_info->estLinkSpeed;

cleanup:
	/*
	 * either we never sent a request, we sent a request and
	 * received a response or we sent a request and timed out.
	 * regardless we are done with the request.
	 */
	osif_request_put(request);

return_cached_value:
	*linkspeed = adapter->estimated_linkspeed;

	return ret;
}

int wlan_hdd_get_link_speed(struct hdd_adapter *adapter, uint32_t *link_speed)
{
	struct hdd_context *hddctx = WLAN_HDD_GET_CTX(adapter);
	struct hdd_station_ctx *hdd_stactx =
				WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	int ret;

	ret = wlan_hdd_validate_context(hddctx);
	if (ret)
		return ret;

	/* Linkspeed is allowed only for P2P mode */
	if (adapter->device_mode != QDF_P2P_CLIENT_MODE) {
		hdd_err("Link Speed is not allowed in Device mode %s(%d)",
			qdf_opmode_str(adapter->device_mode),
			adapter->device_mode);
		return -ENOTSUPP;
	}

	if (eConnectionState_Associated != hdd_stactx->conn_info.conn_state) {
		/* we are not connected so we don't have a classAstats */
		*link_speed = 0;
	} else {
		struct qdf_mac_addr bssid;

		qdf_copy_macaddr(&bssid, &hdd_stactx->conn_info.bssid);

		ret = wlan_hdd_get_linkspeed_for_peermac(adapter, &bssid,
							 link_speed);
		if (ret) {
			hdd_err("Unable to retrieve SME linkspeed");
			return ret;
		}
		/* linkspeed in units of 500 kbps */
		*link_speed = (*link_speed) / 500;
	}
	return 0;
}

struct peer_rssi_priv {
	struct sir_peer_sta_info peer_sta_info;
};

/**
 * hdd_get_peer_rssi_cb() - get peer station's rssi callback
 * @sta_rssi: pointer of peer information
 * @context: get rssi callback context
 *
 * This function will fill rssi information to rssi priv
 * adapter
 *
 */
static void hdd_get_peer_rssi_cb(struct sir_peer_info_resp *sta_rssi,
				 void *context)
{
	struct osif_request *request;
	struct peer_rssi_priv *priv;
	struct sir_peer_info *rssi_info;
	uint8_t peer_num;

	if ((!sta_rssi)) {
		hdd_err("Bad param, sta_rssi [%pK]", sta_rssi);
		return;
	}

	request = osif_request_get(context);
	if (!request) {
		hdd_err("Obsolete request");
		return;
	}

	priv = osif_request_priv(request);

	peer_num = sta_rssi->count;
	rssi_info = sta_rssi->info;

	hdd_debug("%d peers", peer_num);

	if (peer_num > MAX_PEER_STA) {
		hdd_warn("Exceed max peer sta to handle one time %d", peer_num);
		peer_num = MAX_PEER_STA;
	}

	qdf_mem_copy(priv->peer_sta_info.info, rssi_info,
		     peer_num * sizeof(*rssi_info));
	priv->peer_sta_info.sta_num = peer_num;

	osif_request_complete(request);
	osif_request_put(request);

}

int wlan_hdd_get_peer_rssi(struct hdd_adapter *adapter,
			   struct qdf_mac_addr *macaddress,
			   struct sir_peer_sta_info *peer_sta_info)
{
	QDF_STATUS status;
	void *cookie;
	int ret;
	struct sir_peer_info_req rssi_req;
	struct osif_request *request;
	struct peer_rssi_priv *priv;
	static const struct osif_request_params params = {
		.priv_size = sizeof(*priv),
		.timeout_ms = WLAN_WAIT_TIME_STATS,
	};

	if (!adapter || !macaddress || !peer_sta_info) {
		hdd_err("adapter [%pK], macaddress [%pK], peer_sta_info[%pK]",
			adapter, macaddress, peer_sta_info);
		return -EFAULT;
	}

	request = osif_request_alloc(&params);
	if (!request) {
		hdd_err("Request allocation failure");
		return -ENOMEM;
	}

	cookie = osif_request_cookie(request);
	priv = osif_request_priv(request);

	qdf_mem_copy(&rssi_req.peer_macaddr, macaddress,
		     QDF_MAC_ADDR_SIZE);
	rssi_req.sessionid = adapter->vdev_id;
	status = sme_get_peer_info(adapter->hdd_ctx->mac_handle,
				   rssi_req,
				   cookie,
				   hdd_get_peer_rssi_cb);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("Unable to retrieve statistics for rssi");
		ret = -EFAULT;
	} else {
		ret = osif_request_wait_for_response(request);
		if (ret) {
			hdd_err("SME timed out while retrieving rssi");
			ret = -EFAULT;
		} else {
			*peer_sta_info = priv->peer_sta_info;
			ret = 0;
		}
	}

	osif_request_put(request);

	return ret;
}

struct peer_info_priv {
	struct sir_peer_sta_ext_info peer_sta_ext_info;
};

/**
 * wlan_hdd_get_peer_info_cb() - get peer info callback
 * @sta_info: pointer of peer information
 * @context: get peer info callback context
 *
 * This function will fill stats info to peer info priv
 *
 */
static void wlan_hdd_get_peer_info_cb(struct sir_peer_info_ext_resp *sta_info,
				      void *context)
{
	struct osif_request *request;
	struct peer_info_priv *priv;
	uint8_t sta_num;

	if ((!sta_info) || (!context)) {
		hdd_err("Bad param, sta_info [%pK] context [%pK]",
			sta_info, context);
		return;
	}

	if (!sta_info->count) {
		hdd_err("Fail to get remote peer info");
		return;
	}

	if (sta_info->count > MAX_PEER_STA) {
		hdd_warn("Exceed max peer number %d", sta_info->count);
		sta_num = MAX_PEER_STA;
	} else {
		sta_num = sta_info->count;
	}

	request = osif_request_get(context);
	if (!request) {
		hdd_err("Obsolete request");
		return;
	}

	priv = osif_request_priv(request);

	priv->peer_sta_ext_info.sta_num = sta_num;
	qdf_mem_copy(&priv->peer_sta_ext_info.info,
		     sta_info->info,
		     sta_num * sizeof(sta_info->info[0]));

	osif_request_complete(request);
	osif_request_put(request);
}

int wlan_hdd_get_peer_info(struct hdd_adapter *adapter,
			   struct qdf_mac_addr macaddress,
			   struct sir_peer_info_ext *peer_info_ext)
{
	QDF_STATUS status;
	void *cookie;
	int ret;
	struct sir_peer_info_ext_req peer_info_req;
	struct osif_request *request;
	struct peer_info_priv *priv;
	static const struct osif_request_params params = {
		.priv_size = sizeof(*priv),
		.timeout_ms = WLAN_WAIT_TIME_STATS,
	};

	if (!adapter) {
		hdd_err("adapter is NULL");
		return -EFAULT;
	}

	request = osif_request_alloc(&params);
	if (!request) {
		hdd_err("Request allocation failure");
		return -ENOMEM;
	}

	cookie = osif_request_cookie(request);
	priv = osif_request_priv(request);

	qdf_mem_copy(&peer_info_req.peer_macaddr, &macaddress,
		     QDF_MAC_ADDR_SIZE);
	peer_info_req.sessionid = adapter->vdev_id;
	peer_info_req.reset_after_request = 0;
	status = sme_get_peer_info_ext(adapter->hdd_ctx->mac_handle,
				       &peer_info_req,
				       cookie,
				       wlan_hdd_get_peer_info_cb);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("Unable to retrieve statistics for peer info");
		ret = -EFAULT;
	} else {
		ret = osif_request_wait_for_response(request);
		if (ret) {
			hdd_err("SME timed out while retrieving peer info");
			ret = -EFAULT;
		} else {
			/* only support one peer by now */
			*peer_info_ext = priv->peer_sta_ext_info.info[0];
			ret = 0;
		}
	}

	osif_request_put(request);

	return ret;
}

#ifndef QCA_SUPPORT_CP_STATS
struct class_a_stats {
	tCsrGlobalClassAStatsInfo class_a_stats;
};

/**
 * hdd_get_class_a_statistics_cb() - Get Class A stats callback function
 * @stats: pointer to Class A stats
 * @context: user context originally registered with SME (always the
 *	cookie from the request context)
 *
 * Return: None
 */
static void hdd_get_class_a_statistics_cb(void *stats, void *context)
{
	struct osif_request *request;
	struct class_a_stats *priv;
	tCsrGlobalClassAStatsInfo *returned_stats;

	hdd_enter();
	if (!stats) {
		hdd_err("Bad param, stats");
		return;
	}

	request = osif_request_get(context);
	if (!request) {
		hdd_err("Obsolete request");
		return;
	}

	returned_stats = stats;
	priv = osif_request_priv(request);
	priv->class_a_stats = *returned_stats;
	osif_request_complete(request);
	osif_request_put(request);
	hdd_exit();
}

QDF_STATUS wlan_hdd_get_class_astats(struct hdd_adapter *adapter)
{
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	QDF_STATUS status;
	int ret;
	void *cookie;
	struct osif_request *request;
	struct class_a_stats *priv;
	static const struct osif_request_params params = {
		.priv_size = sizeof(*priv),
		.timeout_ms = WLAN_WAIT_TIME_STATS,
	};

	if (!adapter) {
		hdd_err("adapter is NULL");
		return QDF_STATUS_E_FAULT;
	}
	if (cds_is_driver_recovering() || cds_is_driver_in_bad_state()) {
		hdd_debug("Recovery in Progress. State: 0x%x Ignore!!!",
			 cds_get_driver_state());
		return QDF_STATUS_SUCCESS;
	}

	request = osif_request_alloc(&params);
	if (!request) {
		hdd_err("Request allocation failure");
		return QDF_STATUS_E_NOMEM;
	}
	cookie = osif_request_cookie(request);

	/* query only for Class A statistics (which include link speed) */
	status = sme_get_statistics(adapter->hdd_ctx->mac_handle,
				    eCSR_HDD, SME_GLOBAL_CLASSA_STATS,
				    hdd_get_class_a_statistics_cb,
				    sta_ctx->conn_info.sta_id[0],
				    cookie, adapter->vdev_id);
	if (QDF_STATUS_SUCCESS != status) {
		hdd_warn("Unable to retrieve Class A statistics");
		goto return_cached_results;
	}

	/* request was sent -- wait for the response */
	ret = osif_request_wait_for_response(request);
	if (ret) {
		hdd_warn("SME timed out while retrieving Class A statistics");
		goto return_cached_results;
	}

	/* update the adapter with the fresh results */
	priv = osif_request_priv(request);
	adapter->hdd_stats.class_a_stat = priv->class_a_stats;

return_cached_results:
	/*
	 * either we never sent a request, we sent a request and
	 * received a response or we sent a request and timed out.
	 * regardless we are done with the request.
	 */
	osif_request_put(request);

	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef QCA_SUPPORT_CP_STATS
int wlan_hdd_get_station_stats(struct hdd_adapter *adapter)
{
	int ret = 0;
	uint8_t mcs_rate_flags;
	struct stats_event *stats;

	stats = wlan_cfg80211_mc_cp_stats_get_station_stats(adapter->vdev,
							    &ret);
	if (ret || !stats) {
		wlan_cfg80211_mc_cp_stats_free_stats_event(stats);
		return ret;
	}

	/* save summary stats to legacy location */
	qdf_mem_copy(adapter->hdd_stats.summary_stat.retry_cnt,
		stats->vdev_summary_stats[0].stats.retry_cnt,
		sizeof(adapter->hdd_stats.summary_stat.retry_cnt));
	qdf_mem_copy(adapter->hdd_stats.summary_stat.multiple_retry_cnt,
		stats->vdev_summary_stats[0].stats.multiple_retry_cnt,
		sizeof(adapter->hdd_stats.summary_stat.multiple_retry_cnt));
	qdf_mem_copy(adapter->hdd_stats.summary_stat.tx_frm_cnt,
		stats->vdev_summary_stats[0].stats.tx_frm_cnt,
		sizeof(adapter->hdd_stats.summary_stat.tx_frm_cnt));
	qdf_mem_copy(adapter->hdd_stats.summary_stat.fail_cnt,
		stats->vdev_summary_stats[0].stats.fail_cnt,
		sizeof(adapter->hdd_stats.summary_stat.fail_cnt));
	adapter->hdd_stats.summary_stat.snr =
		stats->vdev_summary_stats[0].stats.snr;
	adapter->hdd_stats.summary_stat.rssi =
		stats->vdev_summary_stats[0].stats.rssi;
	adapter->hdd_stats.summary_stat.rx_frm_cnt =
		stats->vdev_summary_stats[0].stats.rx_frm_cnt;
	adapter->hdd_stats.summary_stat.frm_dup_cnt =
		stats->vdev_summary_stats[0].stats.frm_dup_cnt;
	adapter->hdd_stats.summary_stat.rts_fail_cnt =
		stats->vdev_summary_stats[0].stats.rts_fail_cnt;
	adapter->hdd_stats.summary_stat.ack_fail_cnt =
		stats->vdev_summary_stats[0].stats.ack_fail_cnt;
	adapter->hdd_stats.summary_stat.rts_succ_cnt =
		stats->vdev_summary_stats[0].stats.rts_succ_cnt;
	adapter->hdd_stats.summary_stat.rx_discard_cnt =
		stats->vdev_summary_stats[0].stats.rx_discard_cnt;
	adapter->hdd_stats.summary_stat.rx_error_cnt =
		stats->vdev_summary_stats[0].stats.rx_error_cnt;
	adapter->hdd_stats.peer_stats.rx_count =
		stats->peer_adv_stats->rx_count;
	adapter->hdd_stats.peer_stats.rx_bytes =
		stats->peer_adv_stats->rx_bytes;
	adapter->hdd_stats.peer_stats.fcs_count =
		stats->peer_adv_stats->fcs_count;

	/* save class a stats to legacy location */
	adapter->hdd_stats.class_a_stat.tx_nss =
		wlan_vdev_mlme_get_nss(adapter->vdev);
	adapter->hdd_stats.class_a_stat.rx_nss =
		wlan_vdev_mlme_get_nss(adapter->vdev);
	adapter->hdd_stats.class_a_stat.tx_rate = stats->tx_rate;
	adapter->hdd_stats.class_a_stat.rx_rate = stats->rx_rate;
	adapter->hdd_stats.class_a_stat.tx_rx_rate_flags = stats->tx_rate_flags;
	adapter->hdd_stats.class_a_stat.tx_mcs_index =
		sme_get_mcs_idx(stats->tx_rate, stats->tx_rate_flags,
				&adapter->hdd_stats.class_a_stat.tx_nss,
				&mcs_rate_flags);
	adapter->hdd_stats.class_a_stat.tx_mcs_rate_flags = mcs_rate_flags;
	adapter->hdd_stats.class_a_stat.rx_mcs_index =
		sme_get_mcs_idx(stats->rx_rate, stats->tx_rate_flags,
				&adapter->hdd_stats.class_a_stat.rx_nss,
				&mcs_rate_flags);
	adapter->hdd_stats.class_a_stat.rx_mcs_rate_flags = mcs_rate_flags;

	/* save per chain rssi to legacy location */
	qdf_mem_copy(adapter->hdd_stats.per_chain_rssi_stats.rssi,
		     stats->vdev_chain_rssi[0].chain_rssi,
		     sizeof(stats->vdev_chain_rssi[0].chain_rssi));
	wlan_cfg80211_mc_cp_stats_free_stats_event(stats);

	return 0;
}
#else /* QCA_SUPPORT_CP_STATS */
struct station_stats {
	tCsrSummaryStatsInfo summary_stats;
	tCsrGlobalClassAStatsInfo class_a_stats;
	struct csr_per_chain_rssi_stats_info per_chain_rssi_stats;
};

/**
 * hdd_get_station_statistics_cb() - Get stats callback function
 * @stats: pointer to combined station stats
 * @context: user context originally registered with SME (always the
 *	cookie from the request context)
 *
 * Return: None
 */
static void hdd_get_station_statistics_cb(void *stats, void *context)
{
	struct osif_request *request;
	struct station_stats *priv;
	tCsrSummaryStatsInfo *summary_stats;
	tCsrGlobalClassAStatsInfo *class_a_stats;
	struct csr_per_chain_rssi_stats_info *per_chain_rssi_stats;

	if ((!stats) || (!context)) {
		hdd_err("Bad param, stats [%pK] context [%pK]",
			stats, context);
		return;
	}

	request = osif_request_get(context);
	if (!request) {
		hdd_err("Obsolete request");
		return;
	}

	summary_stats = (tCsrSummaryStatsInfo *) stats;
	class_a_stats = (tCsrGlobalClassAStatsInfo *) (summary_stats + 1);
	per_chain_rssi_stats = (struct csr_per_chain_rssi_stats_info *)
				(class_a_stats + 1);
	priv = osif_request_priv(request);

	/* copy over the stats. do so as a struct copy */
	priv->summary_stats = *summary_stats;
	priv->class_a_stats = *class_a_stats;
	priv->per_chain_rssi_stats = *per_chain_rssi_stats;

	osif_request_complete(request);
	osif_request_put(request);
}

int wlan_hdd_get_station_stats(struct hdd_adapter *adapter)
{
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	QDF_STATUS status;
	int errno;
	void *cookie;
	struct osif_request *request;
	struct station_stats *priv;
	static const struct osif_request_params params = {
		.priv_size = sizeof(*priv),
		.timeout_ms = WLAN_WAIT_TIME_STATS,
	};

	if (!adapter) {
		hdd_err("adapter is NULL");
		return 0;
	}

	request = osif_request_alloc(&params);
	if (!request) {
		hdd_err("Request allocation failure");
		return -ENOMEM;
	}
	cookie = osif_request_cookie(request);

	/* query only for Summary & Class A statistics */
	status = sme_get_statistics(adapter->hdd_ctx->mac_handle,
				    eCSR_HDD,
				    SME_SUMMARY_STATS |
					    SME_GLOBAL_CLASSA_STATS |
					    SME_PER_CHAIN_RSSI_STATS,
				    hdd_get_station_statistics_cb,
				    sta_ctx->conn_info.sta_id[0],
				    cookie,
				    adapter->vdev_id);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("Failed to retrieve statistics, status %d", status);
		goto put_request;
	}

	/* request was sent -- wait for the response */
	errno = osif_request_wait_for_response(request);
	if (errno) {
		hdd_err("Failed to wait for statistics, errno %d", errno);
		goto put_request;
	}

	/* update the adapter with the fresh results */
	priv = osif_request_priv(request);
	adapter->hdd_stats.summary_stat = priv->summary_stats;
	adapter->hdd_stats.class_a_stat = priv->class_a_stats;
	adapter->hdd_stats.per_chain_rssi_stats = priv->per_chain_rssi_stats;

put_request:
	/*
	 * either we never sent a request, we sent a request and
	 * received a response or we sent a request and timed out.
	 * regardless we are done with the request.
	 */
	osif_request_put(request);

	/* either callback updated adapter stats or it has cached data */
	return 0;
}
#endif /* QCA_SUPPORT_CP_STATS */

struct temperature_priv {
	int temperature;
};

/**
 * hdd_get_temperature_cb() - "Get Temperature" callback function
 * @temperature: measured temperature
 * @context: callback context
 *
 * This function is passed to sme_get_temperature() as the callback
 * function to be invoked when the temperature measurement is
 * available.
 *
 * Return: None
 */
static void hdd_get_temperature_cb(int temperature, void *context)
{
	struct osif_request *request;
	struct temperature_priv *priv;

	hdd_enter();

	request = osif_request_get(context);
	if (!request) {
		hdd_err("Obsolete request");
		return;
	}

	priv = osif_request_priv(request);
	priv->temperature = temperature;
	osif_request_complete(request);
	osif_request_put(request);
	hdd_exit();
}

int wlan_hdd_get_temperature(struct hdd_adapter *adapter, int *temperature)
{
	QDF_STATUS status;
	int ret;
	void *cookie;
	struct osif_request *request;
	struct temperature_priv *priv;
	static const struct osif_request_params params = {
		.priv_size = sizeof(*priv),
		.timeout_ms = WLAN_WAIT_TIME_STATS,
	};

	hdd_enter();
	if (!adapter) {
		hdd_err("adapter is NULL");
		return -EPERM;
	}

	request = osif_request_alloc(&params);
	if (!request) {
		hdd_err("Request allocation failure");
		return -ENOMEM;
	}
	cookie = osif_request_cookie(request);
	status = sme_get_temperature(adapter->hdd_ctx->mac_handle, cookie,
				     hdd_get_temperature_cb);
	if (QDF_STATUS_SUCCESS != status) {
		hdd_err("Unable to retrieve temperature");
	} else {
		ret = osif_request_wait_for_response(request);
		if (ret) {
			hdd_err("SME timed out while retrieving temperature");
		} else {
			/* update the adapter with the fresh results */
			priv = osif_request_priv(request);
			if (priv->temperature)
				adapter->temperature = priv->temperature;
		}
	}

	/*
	 * either we never sent a request, we sent a request and
	 * received a response or we sent a request and timed out.
	 * regardless we are done with the request.
	 */
	osif_request_put(request);

	*temperature = adapter->temperature;
	hdd_exit();
	return 0;
}

void wlan_hdd_display_txrx_stats(struct hdd_context *ctx)
{
	struct hdd_adapter *adapter = NULL;
	struct hdd_tx_rx_stats *stats;
	int i = 0;
	uint32_t total_rx_pkt, total_rx_dropped,
		 total_rx_delv, total_rx_refused;

	hdd_for_each_adapter(ctx, adapter) {
		total_rx_pkt = 0;
		total_rx_dropped = 0;
		total_rx_delv = 0;
		total_rx_refused = 0;
		stats = &adapter->hdd_stats.tx_rx_stats;
		hdd_debug("adapter: %u", adapter->vdev_id);
		for (; i < NUM_CPUS; i++) {
			total_rx_pkt += stats->rx_packets[i];
			total_rx_dropped += stats->rx_dropped[i];
			total_rx_delv += stats->rx_delivered[i];
			total_rx_refused += stats->rx_refused[i];
		}

		hdd_debug("TX - called %u, dropped %u orphan %u",
			  stats->tx_called, stats->tx_dropped,
			  stats->tx_orphaned);

		for (i = 0; i < NUM_CPUS; i++) {
			if (stats->rx_packets[i] == 0)
				continue;
			hdd_debug("Rx CPU[%d]: packets %u, dropped %u, delivered %u, refused %u",
				  i, stats->rx_packets[i], stats->rx_dropped[i],
				  stats->rx_delivered[i], stats->rx_refused[i]);
		}
		hdd_debug("RX - packets %u, dropped %u, unsolict_arp_n_mcast_drp %u, delivered %u, refused %u GRO - agg %u non-agg %u flushes(%u %u) disabled(conc %u low-tput %u)",
			  total_rx_pkt, total_rx_dropped,
			  qdf_atomic_read(&stats->rx_usolict_arp_n_mcast_drp),
			  total_rx_delv,
			  total_rx_refused, stats->rx_aggregated,
			  stats->rx_non_aggregated, stats->rx_gro_flushes,
			  stats->rx_gro_force_flushes,
			  qdf_atomic_read(&ctx->disable_rx_ol_in_concurrency),
			  qdf_atomic_read(&ctx->disable_rx_ol_in_low_tput));
	}
}
