/*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/**
 * DOC: wlan_hdd_cfg80211.c
 *
 * WLAN Host Device Driver cfg80211 APIs implementation
 *
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/etherdevice.h>
#include <linux/wireless.h>
#include <wlan_hdd_includes.h>
#include <net/arp.h>
#include <net/cfg80211.h>
#include <wlan_hdd_wowl.h>
#include <ani_global.h>
#include "sir_params.h"
#include "dot11f.h"
#include "wlan_hdd_assoc.h"
#include "wlan_hdd_wext.h"
#include "sme_api.h"
#include "sme_power_save_api.h"
#include "wlan_hdd_p2p.h"
#include "wlan_hdd_cfg80211.h"
#include "wlan_hdd_hostapd.h"
#include "wlan_hdd_softap_tx_rx.h"
#include "wlan_hdd_main.h"
#include "wlan_hdd_power.h"
#include "wlan_hdd_trace.h"
#include "qdf_str.h"
#include "qdf_trace.h"
#include "qdf_types.h"
#include "cds_utils.h"
#include "cds_sched.h"
#include "wlan_hdd_scan.h"
#include <qc_sap_ioctl.h>
#include "wlan_hdd_tdls.h"
#include "wlan_hdd_wmm.h"
#include "wma_types.h"
#include "wma.h"
#include "wlan_hdd_misc.h"
#include "wlan_hdd_nan.h"
#include <wlan_hdd_ipa.h>
#include "wlan_logging_sock_svc.h"
#include "sap_api.h"
#include "csr_api.h"
#include "pld_common.h"
#include "wmi_unified_param.h"

#ifdef WLAN_UMAC_CONVERGENCE
#include "wlan_cfg80211.h"
#endif
#include <cdp_txrx_handle.h>
#include <wlan_cfg80211_scan.h>
#include <wlan_cfg80211_ftm.h>

#ifdef FEATURE_WLAN_EXTSCAN
#include "wlan_hdd_ext_scan.h"
#endif

#ifdef WLAN_FEATURE_LINK_LAYER_STATS
#include "wlan_hdd_stats.h"
#endif
#include "cds_api.h"
#include "wlan_policy_mgr_api.h"
#include "qwlan_version.h"

#include "wlan_hdd_ocb.h"
#include "wlan_hdd_tsf.h"

#include "wlan_hdd_subnet_detect.h"
#include <wlan_hdd_regulatory.h>
#include "wlan_hdd_lpass.h"
#include "wlan_hdd_nan_datapath.h"
#include "wlan_hdd_disa.h"
#include "wlan_hdd_request_manager.h"
#include "wlan_hdd_he.h"

#include <cdp_txrx_cmn.h>
#include <cdp_txrx_misc.h>
#include <qca_vendor.h>
#include "wlan_pmo_ucfg_api.h"
#include "os_if_wifi_pos.h"
#include "wlan_utility.h"
#include "wlan_reg_ucfg_api.h"
#include "wifi_pos_api.h"
#include "wlan_hdd_spectralscan.h"
#define g_mode_rates_size (12)
#define a_mode_rates_size (8)

/*
 * Android CTS verifier needs atleast this much wait time (in msec)
 */
#define MAX_REMAIN_ON_CHANNEL_DURATION (5000)

/*
 * Refer @tCfgProtection structure for definition of the bit map.
 * below value is obtained by setting the following bit-fields.
 * enable obss, fromllb, overlapOBSS and overlapFromllb protection.
 */
#define IBSS_CFG_PROTECTION_ENABLE_MASK 0x8282

#define HDD2GHZCHAN(freq, chan, flag)   {     \
		.band = HDD_NL80211_BAND_2GHZ, \
		.center_freq = (freq), \
		.hw_value = (chan), \
		.flags = (flag), \
		.max_antenna_gain = 0, \
		.max_power = 0, \
}

#define HDD5GHZCHAN(freq, chan, flag)   {     \
		.band =  HDD_NL80211_BAND_5GHZ, \
		.center_freq = (freq), \
		.hw_value = (chan), \
		.flags = (flag), \
		.max_antenna_gain = 0, \
		.max_power = 0, \
}

#define HDD_G_MODE_RATETAB(rate, rate_id, flag)	\
	{ \
		.bitrate = rate, \
		.hw_value = rate_id, \
		.flags = flag, \
	}

#ifndef WLAN_AKM_SUITE_FT_8021X
#define WLAN_AKM_SUITE_FT_8021X         0x000FAC03
#endif

#ifndef WLAN_AKM_SUITE_FT_PSK
#define WLAN_AKM_SUITE_FT_PSK           0x000FAC04
#endif

#define HDD_CHANNEL_14 14

#define IS_DFS_MODE_VALID(mode) ((mode >= DFS_MODE_NONE && \
			mode <= DFS_MODE_DEPRIORITIZE))

#define MAX_TXPOWER_SCALE 4
#define CDS_MAX_FEATURE_SET   8

/*
 * Number of DPTRACE records to dump when a cfg80211 disconnect with reason
 * WLAN_REASON_DEAUTH_LEAVING DEAUTH is received from user-space.
 */
#define WLAN_DEAUTH_DPTRACE_DUMP_COUNT 100
#ifndef WLAN_CIPHER_SUITE_GCMP
#define WLAN_CIPHER_SUITE_GCMP 0x000FAC08
#endif
#ifndef WLAN_CIPHER_SUITE_GCMP_256
#define WLAN_CIPHER_SUITE_GCMP_256 0x000FAC09
#endif

static const u32 hdd_gcmp_cipher_suits[] = {
	WLAN_CIPHER_SUITE_GCMP,
	WLAN_CIPHER_SUITE_GCMP_256,
};

static const u32 hdd_cipher_suites[] = {
	WLAN_CIPHER_SUITE_WEP40,
	WLAN_CIPHER_SUITE_WEP104,
	WLAN_CIPHER_SUITE_TKIP,
#ifdef FEATURE_WLAN_ESE
#define WLAN_CIPHER_SUITE_BTK 0x004096fe        /* use for BTK */
#define WLAN_CIPHER_SUITE_KRK 0x004096ff        /* use for KRK */
	WLAN_CIPHER_SUITE_BTK,
	WLAN_CIPHER_SUITE_KRK,
	WLAN_CIPHER_SUITE_CCMP,
#else
	WLAN_CIPHER_SUITE_CCMP,
#endif
#ifdef FEATURE_WLAN_WAPI
	WLAN_CIPHER_SUITE_SMS4,
#endif
#ifdef WLAN_FEATURE_11W
	WLAN_CIPHER_SUITE_AES_CMAC,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
	WLAN_CIPHER_SUITE_BIP_GMAC_128,
	WLAN_CIPHER_SUITE_BIP_GMAC_256,
#endif
#endif
};

static const struct ieee80211_channel hdd_channels_2_4_ghz[] = {
	HDD2GHZCHAN(2412, 1, 0),
	HDD2GHZCHAN(2417, 2, 0),
	HDD2GHZCHAN(2422, 3, 0),
	HDD2GHZCHAN(2427, 4, 0),
	HDD2GHZCHAN(2432, 5, 0),
	HDD2GHZCHAN(2437, 6, 0),
	HDD2GHZCHAN(2442, 7, 0),
	HDD2GHZCHAN(2447, 8, 0),
	HDD2GHZCHAN(2452, 9, 0),
	HDD2GHZCHAN(2457, 10, 0),
	HDD2GHZCHAN(2462, 11, 0),
	HDD2GHZCHAN(2467, 12, 0),
	HDD2GHZCHAN(2472, 13, 0),
	HDD2GHZCHAN(2484, 14, 0),
};

static const struct ieee80211_channel hdd_channels_5_ghz[] = {
	HDD5GHZCHAN(5180, 36, 0),
	HDD5GHZCHAN(5200, 40, 0),
	HDD5GHZCHAN(5220, 44, 0),
	HDD5GHZCHAN(5240, 48, 0),
	HDD5GHZCHAN(5260, 52, 0),
	HDD5GHZCHAN(5280, 56, 0),
	HDD5GHZCHAN(5300, 60, 0),
	HDD5GHZCHAN(5320, 64, 0),
	HDD5GHZCHAN(5500, 100, 0),
	HDD5GHZCHAN(5520, 104, 0),
	HDD5GHZCHAN(5540, 108, 0),
	HDD5GHZCHAN(5560, 112, 0),
	HDD5GHZCHAN(5580, 116, 0),
	HDD5GHZCHAN(5600, 120, 0),
	HDD5GHZCHAN(5620, 124, 0),
	HDD5GHZCHAN(5640, 128, 0),
	HDD5GHZCHAN(5660, 132, 0),
	HDD5GHZCHAN(5680, 136, 0),
	HDD5GHZCHAN(5700, 140, 0),
	HDD5GHZCHAN(5720, 144, 0),
	HDD5GHZCHAN(5745, 149, 0),
	HDD5GHZCHAN(5765, 153, 0),
	HDD5GHZCHAN(5785, 157, 0),
	HDD5GHZCHAN(5805, 161, 0),
	HDD5GHZCHAN(5825, 165, 0),
};

static const struct ieee80211_channel hdd_channels_dot11p[] = {
	HDD5GHZCHAN(5852, 170, 0),
	HDD5GHZCHAN(5855, 171, 0),
	HDD5GHZCHAN(5860, 172, 0),
	HDD5GHZCHAN(5865, 173, 0),
	HDD5GHZCHAN(5870, 174, 0),
	HDD5GHZCHAN(5875, 175, 0),
	HDD5GHZCHAN(5880, 176, 0),
	HDD5GHZCHAN(5885, 177, 0),
	HDD5GHZCHAN(5890, 178, 0),
	HDD5GHZCHAN(5895, 179, 0),
	HDD5GHZCHAN(5900, 180, 0),
	HDD5GHZCHAN(5905, 181, 0),
	HDD5GHZCHAN(5910, 182, 0),
	HDD5GHZCHAN(5915, 183, 0),
	HDD5GHZCHAN(5920, 184, 0),
};

static struct ieee80211_rate g_mode_rates[] = {
	HDD_G_MODE_RATETAB(10, 0x1, 0),
	HDD_G_MODE_RATETAB(20, 0x2, 0),
	HDD_G_MODE_RATETAB(55, 0x4, 0),
	HDD_G_MODE_RATETAB(110, 0x8, 0),
	HDD_G_MODE_RATETAB(60, 0x10, 0),
	HDD_G_MODE_RATETAB(90, 0x20, 0),
	HDD_G_MODE_RATETAB(120, 0x40, 0),
	HDD_G_MODE_RATETAB(180, 0x80, 0),
	HDD_G_MODE_RATETAB(240, 0x100, 0),
	HDD_G_MODE_RATETAB(360, 0x200, 0),
	HDD_G_MODE_RATETAB(480, 0x400, 0),
	HDD_G_MODE_RATETAB(540, 0x800, 0),
};

static struct ieee80211_rate a_mode_rates[] = {
	HDD_G_MODE_RATETAB(60, 0x10, 0),
	HDD_G_MODE_RATETAB(90, 0x20, 0),
	HDD_G_MODE_RATETAB(120, 0x40, 0),
	HDD_G_MODE_RATETAB(180, 0x80, 0),
	HDD_G_MODE_RATETAB(240, 0x100, 0),
	HDD_G_MODE_RATETAB(360, 0x200, 0),
	HDD_G_MODE_RATETAB(480, 0x400, 0),
	HDD_G_MODE_RATETAB(540, 0x800, 0),
};

static struct ieee80211_supported_band wlan_hdd_band_2_4_ghz = {
	.channels = NULL,
	.n_channels = ARRAY_SIZE(hdd_channels_2_4_ghz),
	.band = HDD_NL80211_BAND_2GHZ,
	.bitrates = g_mode_rates,
	.n_bitrates = g_mode_rates_size,
	.ht_cap.ht_supported = 1,
	.ht_cap.cap = IEEE80211_HT_CAP_SGI_20
		      | IEEE80211_HT_CAP_GRN_FLD
		      | IEEE80211_HT_CAP_DSSSCCK40
		      | IEEE80211_HT_CAP_LSIG_TXOP_PROT
		      | IEEE80211_HT_CAP_SGI_40 | IEEE80211_HT_CAP_SUP_WIDTH_20_40,
	.ht_cap.ampdu_factor = IEEE80211_HT_MAX_AMPDU_64K,
	.ht_cap.ampdu_density = IEEE80211_HT_MPDU_DENSITY_16,
	.ht_cap.mcs.rx_mask = {0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
	.ht_cap.mcs.rx_highest = cpu_to_le16(72),
	.ht_cap.mcs.tx_params = IEEE80211_HT_MCS_TX_DEFINED,
};

static struct ieee80211_supported_band wlan_hdd_band_5_ghz = {
	.channels = NULL,
	.n_channels = ARRAY_SIZE(hdd_channels_5_ghz),
	.band = HDD_NL80211_BAND_5GHZ,
	.bitrates = a_mode_rates,
	.n_bitrates = a_mode_rates_size,
	.ht_cap.ht_supported = 1,
	.ht_cap.cap = IEEE80211_HT_CAP_SGI_20
		      | IEEE80211_HT_CAP_GRN_FLD
		      | IEEE80211_HT_CAP_DSSSCCK40
		      | IEEE80211_HT_CAP_LSIG_TXOP_PROT
		      | IEEE80211_HT_CAP_SGI_40 | IEEE80211_HT_CAP_SUP_WIDTH_20_40,
	.ht_cap.ampdu_factor = IEEE80211_HT_MAX_AMPDU_64K,
	.ht_cap.ampdu_density = IEEE80211_HT_MPDU_DENSITY_16,
	.ht_cap.mcs.rx_mask = {0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
	.ht_cap.mcs.rx_highest = cpu_to_le16(72),
	.ht_cap.mcs.tx_params = IEEE80211_HT_MCS_TX_DEFINED,
	.vht_cap.vht_supported = 1,
};

/* This structure contain information what kind of frame are expected in
 * TX/RX direction for each kind of interface
 */
static const struct ieee80211_txrx_stypes
	wlan_hdd_txrx_stypes[NUM_NL80211_IFTYPES] = {
	[NL80211_IFTYPE_STATION] = {
		.tx = 0xffff,
		.rx = BIT(SIR_MAC_MGMT_ACTION) |
		      BIT(SIR_MAC_MGMT_PROBE_REQ) |
		      BIT(SIR_MAC_MGMT_AUTH),
	},
	[NL80211_IFTYPE_AP] = {
		.tx = 0xffff,
		.rx = BIT(SIR_MAC_MGMT_ASSOC_REQ) |
		      BIT(SIR_MAC_MGMT_REASSOC_REQ) |
		      BIT(SIR_MAC_MGMT_PROBE_REQ) |
		      BIT(SIR_MAC_MGMT_DISASSOC) |
		      BIT(SIR_MAC_MGMT_AUTH) |
		      BIT(SIR_MAC_MGMT_DEAUTH) |
		      BIT(SIR_MAC_MGMT_ACTION),
	},
	[NL80211_IFTYPE_ADHOC] = {
		.tx = 0xffff,
		.rx = BIT(SIR_MAC_MGMT_ASSOC_REQ) |
		      BIT(SIR_MAC_MGMT_REASSOC_REQ) |
		      BIT(SIR_MAC_MGMT_PROBE_REQ) |
		      BIT(SIR_MAC_MGMT_DISASSOC) |
		      BIT(SIR_MAC_MGMT_AUTH) |
		      BIT(SIR_MAC_MGMT_DEAUTH) |
		      BIT(SIR_MAC_MGMT_ACTION),
	},
	[NL80211_IFTYPE_P2P_CLIENT] = {
		.tx = 0xffff,
		.rx = BIT(SIR_MAC_MGMT_ACTION) |
		      BIT(SIR_MAC_MGMT_PROBE_REQ),
	},
	[NL80211_IFTYPE_P2P_GO] = {
		/* This is also same as for SoftAP */
		.tx = 0xffff,
		.rx = BIT(SIR_MAC_MGMT_ASSOC_REQ) |
		      BIT(SIR_MAC_MGMT_REASSOC_REQ) |
		      BIT(SIR_MAC_MGMT_PROBE_REQ) |
		      BIT(SIR_MAC_MGMT_DISASSOC) |
		      BIT(SIR_MAC_MGMT_AUTH) |
		      BIT(SIR_MAC_MGMT_DEAUTH) |
		      BIT(SIR_MAC_MGMT_ACTION),
	},
};

/* Interface limits and combinations registered by the driver */

/* STA ( + STA ) combination */
static const struct ieee80211_iface_limit
	wlan_hdd_sta_iface_limit[] = {
	{
		.max = 3,       /* p2p0 is a STA as well */
		.types = BIT(NL80211_IFTYPE_STATION),
	},
};

/* ADHOC (IBSS) limit */
static const struct ieee80211_iface_limit
	wlan_hdd_adhoc_iface_limit[] = {
	{
		.max = 1,
		.types = BIT(NL80211_IFTYPE_STATION),
	},
	{
		.max = 1,
		.types = BIT(NL80211_IFTYPE_ADHOC),
	},
};

/* AP ( + AP ) combination */
static const struct ieee80211_iface_limit
	wlan_hdd_ap_iface_limit[] = {
	{
		.max = (QDF_MAX_NO_OF_SAP_MODE + SAP_MAX_OBSS_STA_CNT),
		.types = BIT(NL80211_IFTYPE_AP),
	},
};

/* P2P limit */
static const struct ieee80211_iface_limit
	wlan_hdd_p2p_iface_limit[] = {
	{
		.max = 1,
		.types = BIT(NL80211_IFTYPE_P2P_CLIENT),
	},
	{
		.max = 1,
		.types = BIT(NL80211_IFTYPE_P2P_GO),
	},
};

static const struct ieee80211_iface_limit
	wlan_hdd_sta_ap_iface_limit[] = {
	{
		/* We need 1 extra STA interface for OBSS scan when SAP starts
		 * with HT40 in STA+SAP concurrency mode
		 */
		.max = (1 + SAP_MAX_OBSS_STA_CNT),
		.types = BIT(NL80211_IFTYPE_STATION),
	},
	{
		.max = QDF_MAX_NO_OF_SAP_MODE,
		.types = BIT(NL80211_IFTYPE_AP),
	},
};

/* STA + P2P combination */
static const struct ieee80211_iface_limit
	wlan_hdd_sta_p2p_iface_limit[] = {
	{
		/* One reserved for dedicated P2PDEV usage */
		.max = 2,
		.types = BIT(NL80211_IFTYPE_STATION)
	},
	{
		/* Support for two identical (GO + GO or CLI + CLI)
		 * or dissimilar (GO + CLI) P2P interfaces
		 */
		.max = 2,
		.types = BIT(NL80211_IFTYPE_P2P_GO) | BIT(NL80211_IFTYPE_P2P_CLIENT),
	},
};

/* STA + AP + P2PGO combination */
static const struct ieee80211_iface_limit
wlan_hdd_sta_ap_p2pgo_iface_limit[] = {
	/* Support for AP+P2PGO interfaces */
	{
	   .max = 2,
	   .types = BIT(NL80211_IFTYPE_STATION)
	},
	{
	   .max = 1,
	   .types = BIT(NL80211_IFTYPE_P2P_GO)
	},
	{
	   .max = 1,
	   .types = BIT(NL80211_IFTYPE_AP)
	}
};

/* SAP + P2P combination */
static const struct ieee80211_iface_limit
wlan_hdd_sap_p2p_iface_limit[] = {
	{
	   /* 1 dedicated for p2p0 which is a STA type */
	   .max = 1,
	   .types = BIT(NL80211_IFTYPE_STATION)
	},
	{
	   /* The p2p interface in SAP+P2P can be GO/CLI.
	    * The p2p connection can be formed on p2p0 or p2p-p2p0-x.
	    */
	   .max = 1,
	   .types = BIT(NL80211_IFTYPE_P2P_GO) | BIT(NL80211_IFTYPE_P2P_CLIENT)
	},
	{
	   /* SAP+GO to support only one SAP interface */
	   .max = 1,
	   .types = BIT(NL80211_IFTYPE_AP)
	}
};

/* P2P + P2P combination */
static const struct ieee80211_iface_limit
wlan_hdd_p2p_p2p_iface_limit[] = {
	{
	   /* 1 dedicated for p2p0 which is a STA type */
	   .max = 1,
	   .types = BIT(NL80211_IFTYPE_STATION)
	},
	{
	   /* The p2p interface in P2P+P2P can be GO/CLI.
	    * For P2P+P2P, the new interfaces are formed on p2p-p2p0-x.
	    */
	   .max = 2,
	   .types = BIT(NL80211_IFTYPE_P2P_GO) | BIT(NL80211_IFTYPE_P2P_CLIENT)
	},
};

static const struct ieee80211_iface_limit
	wlan_hdd_mon_iface_limit[] = {
	{
		.max = 3,       /* Monitor interface */
		.types = BIT(NL80211_IFTYPE_MONITOR),
	},
};

static struct ieee80211_iface_combination
	wlan_hdd_iface_combination[] = {
	/* STA */
	{
		.limits = wlan_hdd_sta_iface_limit,
		.num_different_channels = 2,
		.max_interfaces = 3,
		.n_limits = ARRAY_SIZE(wlan_hdd_sta_iface_limit),
	},
	/* ADHOC */
	{
		.limits = wlan_hdd_adhoc_iface_limit,
		.num_different_channels = 2,
		.max_interfaces = 2,
		.n_limits = ARRAY_SIZE(wlan_hdd_adhoc_iface_limit),
	},
	/* AP */
	{
		.limits = wlan_hdd_ap_iface_limit,
		.num_different_channels = 2,
		.max_interfaces = (SAP_MAX_OBSS_STA_CNT + QDF_MAX_NO_OF_SAP_MODE),
		.n_limits = ARRAY_SIZE(wlan_hdd_ap_iface_limit),
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 10, 0)) || \
	defined(CFG80211_BEACON_INTERVAL_BACKPORT)
		.beacon_int_min_gcd = 1,
#endif
	},
	/* P2P */
	{
		.limits = wlan_hdd_p2p_iface_limit,
		.num_different_channels = 2,
		.max_interfaces = 2,
		.n_limits = ARRAY_SIZE(wlan_hdd_p2p_iface_limit),
	},
	/* STA + AP */
	{
		.limits = wlan_hdd_sta_ap_iface_limit,
		.num_different_channels = 2,
		.max_interfaces = (1 + SAP_MAX_OBSS_STA_CNT + QDF_MAX_NO_OF_SAP_MODE),
		.n_limits = ARRAY_SIZE(wlan_hdd_sta_ap_iface_limit),
		.beacon_int_infra_match = true,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 10, 0)) || \
	defined(CFG80211_BEACON_INTERVAL_BACKPORT)
		.beacon_int_min_gcd = 1,
#endif
	},
	/* STA + P2P */
	{
		.limits = wlan_hdd_sta_p2p_iface_limit,
		.num_different_channels = 2,
		/* one interface reserved for P2PDEV dedicated usage */
		.max_interfaces = 4,
		.n_limits = ARRAY_SIZE(wlan_hdd_sta_p2p_iface_limit),
		.beacon_int_infra_match = true,
	},
	/* STA + P2P GO + SAP */
	{
		.limits = wlan_hdd_sta_ap_p2pgo_iface_limit,
		/* we can allow 3 channels for three different persona
		 * but due to firmware limitation, allow max 2 concrnt channels.
		 */
		.num_different_channels = 2,
		/* one interface reserved for P2PDEV dedicated usage */
		.max_interfaces = 4,
		.n_limits = ARRAY_SIZE(wlan_hdd_sta_ap_p2pgo_iface_limit),
		.beacon_int_infra_match = true,
	},
	/* SAP + P2P */
	{
		.limits = wlan_hdd_sap_p2p_iface_limit,
		.num_different_channels = 2,
		/* 1-p2p0 + 1-SAP + 1-P2P (on p2p0 or p2p-p2p0-x) */
		.max_interfaces = 3,
		.n_limits = ARRAY_SIZE(wlan_hdd_sap_p2p_iface_limit),
		.beacon_int_infra_match = true,
	},
	/* P2P + P2P */
	{
		.limits = wlan_hdd_p2p_p2p_iface_limit,
		.num_different_channels = 2,
		/* 1-p2p0 + 2-P2P (on p2p-p2p0-x) */
		.max_interfaces = 3,
		.n_limits = ARRAY_SIZE(wlan_hdd_p2p_p2p_iface_limit),
		.beacon_int_infra_match = true,
	},
	/* Monitor */
	{
		.limits = wlan_hdd_mon_iface_limit,
		.max_interfaces = 3,
		.num_different_channels = 2,
		.n_limits = ARRAY_SIZE(wlan_hdd_mon_iface_limit),
	},
};

static struct cfg80211_ops wlan_hdd_cfg80211_ops;

#ifdef WLAN_NL80211_TESTMODE
enum wlan_hdd_tm_attr {
	WLAN_HDD_TM_ATTR_INVALID = 0,
	WLAN_HDD_TM_ATTR_CMD = 1,
	WLAN_HDD_TM_ATTR_DATA = 2,
	WLAN_HDD_TM_ATTR_STREAM_ID = 3,
	WLAN_HDD_TM_ATTR_TYPE = 4,
	/* keep last */
	WLAN_HDD_TM_ATTR_AFTER_LAST,
	WLAN_HDD_TM_ATTR_MAX = WLAN_HDD_TM_ATTR_AFTER_LAST - 1,
};

enum wlan_hdd_tm_cmd {
	WLAN_HDD_TM_CMD_WLAN_FTM = 0,
	WLAN_HDD_TM_CMD_WLAN_HB = 1,
};

#define WLAN_HDD_TM_DATA_MAX_LEN    5000

static const struct nla_policy wlan_hdd_tm_policy[WLAN_HDD_TM_ATTR_MAX + 1] = {
	[WLAN_HDD_TM_ATTR_CMD] = {.type = NLA_U32},
	[WLAN_HDD_TM_ATTR_DATA] = {.type = NLA_BINARY,
				   .len = WLAN_HDD_TM_DATA_MAX_LEN},
};
#endif /* WLAN_NL80211_TESTMODE */

enum wlan_hdd_vendor_ie_access_policy {
	WLAN_HDD_VENDOR_IE_ACCESS_NONE = 0,
	WLAN_HDD_VENDOR_IE_ACCESS_ALLOW_IF_LISTED,
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0))
static const struct wiphy_wowlan_support wowlan_support_cfg80211_init = {
	.flags = WIPHY_WOWLAN_MAGIC_PKT,
	.n_patterns = WOWL_MAX_PTRNS_ALLOWED,
	.pattern_min_len = 1,
	.pattern_max_len = WOWL_PTRN_MAX_SIZE,
};
#endif

/**
 * hdd_add_channel_switch_support()- Adds Channel Switch flag if supported
 * @flags: Pointer to the flags to Add channel switch flag.
 *
 * This Function adds Channel Switch support flag, if channel switch is
 * supported by kernel.
 * Return: void.
 */
#ifdef CHANNEL_SWITCH_SUPPORTED
static inline void hdd_add_channel_switch_support(uint32_t *flags)
{
	*flags |= WIPHY_FLAG_HAS_CHANNEL_SWITCH;
}
#else
static inline void hdd_add_channel_switch_support(uint32_t *flags)
{
}
#endif

#ifdef FEATURE_WLAN_TDLS

/* TDLS capabilities params */
#define PARAM_MAX_TDLS_SESSION \
		QCA_WLAN_VENDOR_ATTR_TDLS_GET_CAPS_MAX_CONC_SESSIONS
#define PARAM_TDLS_FEATURE_SUPPORT \
		QCA_WLAN_VENDOR_ATTR_TDLS_GET_CAPS_FEATURES_SUPPORTED

/**
 * __wlan_hdd_cfg80211_get_tdls_capabilities() - Provide TDLS Capabilites.
 * @wiphy:    WIPHY structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of the data received
 *
 * This function provides TDLS capabilities
 *
 * Return: 0 on success and errno on failure
 */
static int __wlan_hdd_cfg80211_get_tdls_capabilities(struct wiphy *wiphy,
						     struct wireless_dev *wdev,
						     const void *data,
						     int data_len)
{
	int status;
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct sk_buff *skb;
	uint32_t set = 0;
	uint32_t max_num_tdls_sta = 0;

	ENTER_DEV(wdev->netdev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	status = wlan_hdd_validate_context(hdd_ctx);
	if (status)
		return status;

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, (2 * sizeof(u32)) +
						   NLMSG_HDRLEN);
	if (!skb) {
		hdd_err("cfg80211_vendor_cmd_alloc_reply_skb failed");
		goto fail;
	}

	if (false == hdd_ctx->config->fEnableTDLSSupport) {
		hdd_debug("TDLS feature not Enabled or Not supported in FW");
		if (nla_put_u32(skb, PARAM_MAX_TDLS_SESSION, 0) ||
			nla_put_u32(skb, PARAM_TDLS_FEATURE_SUPPORT, 0)) {
			hdd_err("nla put fail");
			goto fail;
		}
	} else {
		set = set | WIFI_TDLS_SUPPORT;
		set = set | (hdd_ctx->config->fTDLSExternalControl ?
					WIFI_TDLS_EXTERNAL_CONTROL_SUPPORT : 0);
		set = set | (hdd_ctx->config->fEnableTDLSOffChannel ?
					WIIF_TDLS_OFFCHANNEL_SUPPORT : 0);
		if (hdd_ctx->config->fEnableTDLSSleepSta ||
		    hdd_ctx->config->fEnableTDLSBufferSta ||
		    hdd_ctx->config->fEnableTDLSOffChannel)
			max_num_tdls_sta = HDD_MAX_NUM_TDLS_STA_P_UAPSD_OFFCHAN;
		else
			max_num_tdls_sta = HDD_MAX_NUM_TDLS_STA;

		hdd_debug("TDLS Feature supported value %x", set);
		if (nla_put_u32(skb, PARAM_MAX_TDLS_SESSION,
				max_num_tdls_sta) ||
		    nla_put_u32(skb, PARAM_TDLS_FEATURE_SUPPORT, set)) {
			hdd_err("nla put fail");
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
 * wlan_hdd_cfg80211_get_tdls_capabilities() - Provide TDLS Capabilites.
 * @wiphy:    WIPHY structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of the data received
 *
 * This function provides TDLS capabilities
 *
 * Return: 0 on success and errno on failure
 */
static int
wlan_hdd_cfg80211_get_tdls_capabilities(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					const void *data,
					int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_get_tdls_capabilities(wiphy, wdev,
							data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}
#endif

#ifdef QCA_HT_2040_COEX
static void wlan_hdd_cfg80211_start_pending_acs(struct work_struct *work);
#endif

int wlan_hdd_merge_avoid_freqs(struct ch_avoid_ind_type *destFreqList,
		struct ch_avoid_ind_type *srcFreqList)
{
	int i;
	struct ch_avoid_freq_type *avoid_range =
	&destFreqList->avoid_freq_range[destFreqList->ch_avoid_range_cnt];

	destFreqList->ch_avoid_range_cnt += srcFreqList->ch_avoid_range_cnt;
	if (destFreqList->ch_avoid_range_cnt > CH_AVOID_MAX_RANGE) {
		hdd_err("avoid freq overflow");
		return -EINVAL;
	}

	for (i = 0; i < srcFreqList->ch_avoid_range_cnt; i++) {
		avoid_range->start_freq =
			srcFreqList->avoid_freq_range[i].start_freq;
		avoid_range->end_freq =
			srcFreqList->avoid_freq_range[i].end_freq;
		avoid_range++;
	}
	return 0;
}
/*
 * FUNCTION: wlan_hdd_send_avoid_freq_event
 * This is called when wlan driver needs to send vendor specific
 * avoid frequency range event to userspace
 */
int wlan_hdd_send_avoid_freq_event(struct hdd_context *hdd_ctx,
				   struct ch_avoid_ind_type *avoid_freq_list)
{
	struct sk_buff *vendor_event;

	ENTER();

	if (!hdd_ctx) {
		hdd_err("HDD context is null");
		return -EINVAL;
	}

	if (!avoid_freq_list) {
		hdd_err("avoid_freq_list is null");
		return -EINVAL;
	}

	vendor_event = cfg80211_vendor_event_alloc(hdd_ctx->wiphy,
			NULL, sizeof(struct ch_avoid_ind_type),
			QCA_NL80211_VENDOR_SUBCMD_AVOID_FREQUENCY_INDEX,
			GFP_KERNEL);

	if (!vendor_event) {
		hdd_err("cfg80211_vendor_event_alloc failed");
		return -EINVAL;
	}

	memcpy(skb_put(vendor_event, sizeof(struct ch_avoid_ind_type)),
	       (void *)avoid_freq_list, sizeof(struct ch_avoid_ind_type));

	cfg80211_vendor_event(vendor_event, GFP_KERNEL);

	EXIT();
	return 0;
}

/*
 * define short names for the global vendor params
 * used by QCA_NL80211_VENDOR_SUBCMD_HANG
 */
#define HANG_REASON_INDEX QCA_NL80211_VENDOR_SUBCMD_HANG_REASON_INDEX

/**
 * hdd_convert_hang_reason() - Convert cds recovery reason to vendor specific
 * hang reason
 * @reason: cds recovery reason
 *
 * Return: Vendor specific reason code
 */
static enum qca_wlan_vendor_hang_reason
hdd_convert_hang_reason(enum qdf_hang_reason reason)
{
	u32 ret_val;

	switch (reason) {
	case QDF_RX_HASH_NO_ENTRY_FOUND:
		ret_val = QCA_WLAN_HANG_RX_HASH_NO_ENTRY_FOUND;
		break;
	case QDF_PEER_DELETION_TIMEDOUT:
		ret_val = QCA_WLAN_HANG_PEER_DELETION_TIMEDOUT;
		break;
	case QDF_PEER_UNMAP_TIMEDOUT:
		ret_val = QCA_WLAN_HANG_PEER_UNMAP_TIMEDOUT;
		break;
	case QDF_SCAN_REQ_EXPIRED:
		ret_val = QCA_WLAN_HANG_SCAN_REQ_EXPIRED;
		break;
	case QDF_SCAN_ATTEMPT_FAILURES:
		ret_val = QCA_WLAN_HANG_SCAN_ATTEMPT_FAILURES;
		break;
	case QDF_GET_MSG_BUFF_FAILURE:
		ret_val = QCA_WLAN_HANG_GET_MSG_BUFF_FAILURE;
		break;
	case QDF_ACTIVE_LIST_TIMEOUT:
		ret_val = QCA_WLAN_HANG_ACTIVE_LIST_TIMEOUT;
		break;
	case QDF_SUSPEND_TIMEOUT:
		ret_val = QCA_WLAN_HANG_SUSPEND_TIMEOUT;
		break;
	case QDF_RESUME_TIMEOUT:
		ret_val = QCA_WLAN_HANG_RESUME_TIMEOUT;
		break;
	case QDF_REASON_UNSPECIFIED:
	default:
		ret_val = QCA_WLAN_HANG_REASON_UNSPECIFIED;
	}
	return ret_val;
}

/**
 * wlan_hdd_send_hang_reason_event() - Send hang reason to the userspace
 * @hdd_ctx: Pointer to hdd context
 * @reason: cds recovery reason
 *
 * Return: 0 on success or failure reason
 */
int wlan_hdd_send_hang_reason_event(struct hdd_context *hdd_ctx,
				    enum qdf_hang_reason reason)
{
	struct sk_buff *vendor_event;
	enum qca_wlan_vendor_hang_reason hang_reason;

	ENTER();

	if (!hdd_ctx) {
		hdd_err("HDD context is null");
		return -EINVAL;
	}

	vendor_event = cfg80211_vendor_event_alloc(hdd_ctx->wiphy,
						   NULL,
						   sizeof(uint32_t),
						   HANG_REASON_INDEX,
						   GFP_KERNEL);
	if (!vendor_event) {
		hdd_err("cfg80211_vendor_event_alloc failed");
		return -ENOMEM;
	}

	hang_reason = hdd_convert_hang_reason(reason);

	if (nla_put_u32(vendor_event, QCA_WLAN_VENDOR_ATTR_HANG_REASON,
			(uint32_t)hang_reason)) {
		hdd_err("QCA_WLAN_VENDOR_ATTR_HANG_REASON put fail");
		kfree_skb(vendor_event);
		return -EINVAL;
	}

	cfg80211_vendor_event(vendor_event, GFP_KERNEL);

	EXIT();
	return 0;
}

#undef HANG_REASON_INDEX

/**
 * wlan_hdd_get_adjacent_chan(): Gets next/previous channel
 * with respect to the channel passed.
 * @chan: Channel
 * @upper: If "true" then next channel is returned or else
 * previous channel is returned.
 *
 * This function returns the next/previous adjacent-channel to
 * the channel passed. If "upper = true" then next channel is
 * returned else previous is returned.
 */
int wlan_hdd_get_adjacent_chan(uint8_t chan, bool upper)
{
	enum channel_enum ch_idx = reg_get_chan_enum(chan);

	if (ch_idx == INVALID_CHANNEL)
		return -EINVAL;

	if (upper && (ch_idx < (NUM_CHANNELS - 1)))
		ch_idx++;
	else if (!upper && (ch_idx > CHAN_ENUM_1))
		ch_idx--;
	else
		return -EINVAL;

	return WLAN_REG_CH_NUM(ch_idx);
}

/**
 * wlan_hdd_send_avoid_freq_for_dnbs(): Sends list of frequencies to be
 * avoided when Do_Not_Break_Stream is active.
 * @hdd_ctx:  HDD Context
 * @op_chan:  AP/P2P-GO operating channel
 *
 * This function sends list of frequencies to be avoided when
 * Do_Not_Break_Stream is active.
 * To clear the avoid_frequency_list in the application,
 * op_chan = 0 can be passed.
 *
 * Return: 0 on success and errno on failure
 */
int wlan_hdd_send_avoid_freq_for_dnbs(struct hdd_context *hdd_ctx, uint8_t op_chan)
{
	struct ch_avoid_ind_type p2p_avoid_freq_list;
	uint8_t min_chan, max_chan;
	int ret;
	int chan;

	ENTER();

	if (!hdd_ctx) {
		hdd_err("invalid param");
		return -EINVAL;
	}

	qdf_mem_zero(&p2p_avoid_freq_list, sizeof(struct ch_avoid_ind_type));
	/*
	 * If channel passed is zero, clear the avoid_freq list in application.
	 */
	if (!op_chan) {
#ifdef FEATURE_WLAN_CH_AVOID
		mutex_lock(&hdd_ctx->avoid_freq_lock);
		qdf_mem_zero(&hdd_ctx->dnbs_avoid_freq_list,
				sizeof(struct ch_avoid_ind_type));
		if (hdd_ctx->coex_avoid_freq_list.ch_avoid_range_cnt)
			memcpy(&p2p_avoid_freq_list,
			       &hdd_ctx->coex_avoid_freq_list,
			       sizeof(struct ch_avoid_ind_type));
		mutex_unlock(&hdd_ctx->avoid_freq_lock);
#endif
		ret = wlan_hdd_send_avoid_freq_event(hdd_ctx,
						     &p2p_avoid_freq_list);
		if (ret)
			hdd_err("wlan_hdd_send_avoid_freq_event error:%d",
				ret);

		return ret;
	}

	if (WLAN_REG_IS_24GHZ_CH(op_chan)) {
		min_chan = REG_MIN_24GHZ_CH_NUM;
		max_chan = REG_MAX_24GHZ_CH_NUM;
	} else if WLAN_REG_IS_5GHZ_CH(op_chan) {
		min_chan = REG_MIN_5GHZ_CH_NUM;
		max_chan = REG_MAX_5GHZ_CH_NUM;
	} else {
		hdd_err("invalid channel:%d", op_chan);
		return -EINVAL;
	}

	if ((op_chan > min_chan) && (op_chan < max_chan)) {
		p2p_avoid_freq_list.ch_avoid_range_cnt = 2;
		p2p_avoid_freq_list.avoid_freq_range[0].start_freq =
			wlan_chan_to_freq(min_chan);

		/* Get channel before the op_chan */
		chan = wlan_hdd_get_adjacent_chan(op_chan, false);
		if (chan < 0)
			return -EINVAL;
		p2p_avoid_freq_list.avoid_freq_range[0].end_freq =
			wlan_chan_to_freq(chan);

		/* Get channel next to the op_chan */
		chan = wlan_hdd_get_adjacent_chan(op_chan, true);
		if (chan < 0)
			return -EINVAL;
		p2p_avoid_freq_list.avoid_freq_range[1].start_freq =
			wlan_chan_to_freq(chan);

		p2p_avoid_freq_list.avoid_freq_range[1].end_freq =
			wlan_chan_to_freq(max_chan);
	} else if (op_chan == min_chan) {
		p2p_avoid_freq_list.ch_avoid_range_cnt = 1;

		chan = wlan_hdd_get_adjacent_chan(op_chan, true);
		if (chan < 0)
			return -EINVAL;
		p2p_avoid_freq_list.avoid_freq_range[0].start_freq =
			wlan_chan_to_freq(chan);

		p2p_avoid_freq_list.avoid_freq_range[0].end_freq =
			wlan_chan_to_freq(max_chan);
	} else {
		p2p_avoid_freq_list.ch_avoid_range_cnt = 1;
		p2p_avoid_freq_list.avoid_freq_range[0].start_freq =
			wlan_chan_to_freq(min_chan);

		chan = wlan_hdd_get_adjacent_chan(op_chan, false);
		if (chan < 0)
			return -EINVAL;
		p2p_avoid_freq_list.avoid_freq_range[0].end_freq =
			wlan_chan_to_freq(chan);
	}
#ifdef FEATURE_WLAN_CH_AVOID
	mutex_lock(&hdd_ctx->avoid_freq_lock);
	hdd_ctx->dnbs_avoid_freq_list = p2p_avoid_freq_list;
	if (hdd_ctx->coex_avoid_freq_list.ch_avoid_range_cnt) {
		ret = wlan_hdd_merge_avoid_freqs(&p2p_avoid_freq_list,
				&hdd_ctx->coex_avoid_freq_list);
		if (ret) {
			mutex_unlock(&hdd_ctx->avoid_freq_lock);
			hdd_err("avoid freq merge failed");
			return ret;
		}
	}
	mutex_unlock(&hdd_ctx->avoid_freq_lock);
#endif
	ret = wlan_hdd_send_avoid_freq_event(hdd_ctx, &p2p_avoid_freq_list);
	if (ret)
		hdd_err("wlan_hdd_send_avoid_freq_event error:%d", ret);

	return ret;
}

/* vendor specific events */
static const struct nl80211_vendor_cmd_info wlan_hdd_cfg80211_vendor_events[] = {
	[QCA_NL80211_VENDOR_SUBCMD_AVOID_FREQUENCY_INDEX] = {
		.vendor_id =
			QCA_NL80211_VENDOR_ID,
		.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_AVOID_FREQUENCY
	},

#ifdef WLAN_FEATURE_NAN
	[QCA_NL80211_VENDOR_SUBCMD_NAN_INDEX] = {
		.vendor_id =
			QCA_NL80211_VENDOR_ID,
		.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_NAN
	},
#endif

#ifdef WLAN_FEATURE_STATS_EXT
	[QCA_NL80211_VENDOR_SUBCMD_STATS_EXT_INDEX] = {
		.vendor_id =
			QCA_NL80211_VENDOR_ID,
		.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_STATS_EXT
	},
#endif /* WLAN_FEATURE_STATS_EXT */
#ifdef FEATURE_WLAN_EXTSCAN
	[QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_START_INDEX] = {
		.vendor_id =
			QCA_NL80211_VENDOR_ID,
		.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_START
	},
	[QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_STOP_INDEX] = {
		.vendor_id =
			QCA_NL80211_VENDOR_ID,
		.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_STOP
	},
	[QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_CAPABILITIES_INDEX] = {
		.
		vendor_id
			=
				QCA_NL80211_VENDOR_ID,
		.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_CAPABILITIES
	},
	[QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_CACHED_RESULTS_INDEX] = {
		.
		vendor_id
			=
				QCA_NL80211_VENDOR_ID,
		.
		subcmd =
			QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_CACHED_RESULTS
	},
	[QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SCAN_RESULTS_AVAILABLE_INDEX] = {
		.
		vendor_id
			=
				QCA_NL80211_VENDOR_ID,
		.
		subcmd
			=
				QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SCAN_RESULTS_AVAILABLE
	},
	[QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_FULL_SCAN_RESULT_INDEX] = {
		.
		vendor_id
			=
				QCA_NL80211_VENDOR_ID,
		.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_FULL_SCAN_RESULT
	},
	[QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SCAN_EVENT_INDEX] = {
		.vendor_id =
			QCA_NL80211_VENDOR_ID,
		.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SCAN_EVENT
	},
	[QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_AP_FOUND_INDEX] = {
		.
		vendor_id
			=
				QCA_NL80211_VENDOR_ID,
		.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_AP_FOUND
	},
	[QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_BSSID_HOTLIST_INDEX] = {
		.
		vendor_id
			=
				QCA_NL80211_VENDOR_ID,
		.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_BSSID_HOTLIST
	},
	[QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_BSSID_HOTLIST_INDEX] = {
		.
		vendor_id
			=
				QCA_NL80211_VENDOR_ID,
		.
		subcmd
			=
				QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_BSSID_HOTLIST
	},
	[QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SIGNIFICANT_CHANGE_INDEX] = {
		.
		vendor_id
			=
				QCA_NL80211_VENDOR_ID,
		.
		subcmd =
			QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SIGNIFICANT_CHANGE
	},
	[QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_SIGNIFICANT_CHANGE_INDEX] = {
		.
		vendor_id
			=
				QCA_NL80211_VENDOR_ID,
		.
		subcmd
			=
				QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_SIGNIFICANT_CHANGE
	},
	[QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_SIGNIFICANT_CHANGE_INDEX] = {
		.
		vendor_id
			=
				QCA_NL80211_VENDOR_ID,
		.
		subcmd
			=
				QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_SIGNIFICANT_CHANGE
	},
#endif /* FEATURE_WLAN_EXTSCAN */

#ifdef WLAN_FEATURE_LINK_LAYER_STATS
	[QCA_NL80211_VENDOR_SUBCMD_LL_STATS_SET_INDEX] = {
		.vendor_id =
			QCA_NL80211_VENDOR_ID,
		.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_LL_STATS_SET
	},
	[QCA_NL80211_VENDOR_SUBCMD_LL_STATS_GET_INDEX] = {
		.vendor_id =
			QCA_NL80211_VENDOR_ID,
		.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_LL_STATS_GET
	},
	[QCA_NL80211_VENDOR_SUBCMD_LL_STATS_CLR_INDEX] = {
		.vendor_id =
			QCA_NL80211_VENDOR_ID,
		.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_LL_STATS_CLR
	},
	[QCA_NL80211_VENDOR_SUBCMD_LL_RADIO_STATS_INDEX] = {
		.vendor_id =
			QCA_NL80211_VENDOR_ID,
		.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_LL_STATS_RADIO_RESULTS
	},
	[QCA_NL80211_VENDOR_SUBCMD_LL_IFACE_STATS_INDEX] = {
		.vendor_id =
			QCA_NL80211_VENDOR_ID,
		.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_LL_STATS_IFACE_RESULTS
	},
	[QCA_NL80211_VENDOR_SUBCMD_LL_PEER_INFO_STATS_INDEX] = {
		.vendor_id =
			QCA_NL80211_VENDOR_ID,
		.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_LL_STATS_PEERS_RESULTS
	},
	[QCA_NL80211_VENDOR_SUBCMD_LL_STATS_EXT_INDEX] = {
		.vendor_id =
			QCA_NL80211_VENDOR_ID,
		.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_LL_STATS_EXT
	},
#endif /* WLAN_FEATURE_LINK_LAYER_STATS */
	[QCA_NL80211_VENDOR_SUBCMD_TDLS_STATE_CHANGE_INDEX] = {
		.vendor_id =
			QCA_NL80211_VENDOR_ID,
		.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_TDLS_STATE
	},
	[QCA_NL80211_VENDOR_SUBCMD_DO_ACS_INDEX] = {
		.vendor_id = QCA_NL80211_VENDOR_ID,
		.subcmd = QCA_NL80211_VENDOR_SUBCMD_DO_ACS
	},
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	[QCA_NL80211_VENDOR_SUBCMD_KEY_MGMT_ROAM_AUTH_INDEX] = {
		.vendor_id =
			QCA_NL80211_VENDOR_ID,
		.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_KEY_MGMT_ROAM_AUTH
	},
#endif
	[QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_STARTED_INDEX] =  {
		.vendor_id =
			QCA_NL80211_VENDOR_ID,
		.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_STARTED
	},
	[QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_FINISHED_INDEX] =  {
		.vendor_id =
			QCA_NL80211_VENDOR_ID,
		.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_FINISHED
	},
	[QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_ABORTED_INDEX] =  {
		.vendor_id =
			QCA_NL80211_VENDOR_ID,
		.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_ABORTED
	},
	[QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_NOP_FINISHED_INDEX] =  {
		.vendor_id =
			QCA_NL80211_VENDOR_ID,
		.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_NOP_FINISHED
	},
	[QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_RADAR_DETECTED_INDEX] =  {
		.vendor_id =
			QCA_NL80211_VENDOR_ID,
		.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_RADAR_DETECTED
	},
#ifdef FEATURE_WLAN_EXTSCAN
	[QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_NETWORK_FOUND_INDEX] = {
		.vendor_id = QCA_NL80211_VENDOR_ID,
		.subcmd = QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_NETWORK_FOUND
	},
	[QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_PASSPOINT_NETWORK_FOUND_INDEX] = {
		.vendor_id = QCA_NL80211_VENDOR_ID,
		.subcmd = QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_PASSPOINT_NETWORK_FOUND
	},
	[QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_AP_LOST_INDEX] = {
		.vendor_id = QCA_NL80211_VENDOR_ID,
		.subcmd = QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_AP_LOST
	},
#endif /* FEATURE_WLAN_EXTSCAN */
	[QCA_NL80211_VENDOR_SUBCMD_MONITOR_RSSI_INDEX] = {
		.vendor_id = QCA_NL80211_VENDOR_ID,
		.subcmd = QCA_NL80211_VENDOR_SUBCMD_MONITOR_RSSI
	},
#ifdef WLAN_FEATURE_TSF
	[QCA_NL80211_VENDOR_SUBCMD_TSF_INDEX] = {
		.vendor_id = QCA_NL80211_VENDOR_ID,
		.subcmd = QCA_NL80211_VENDOR_SUBCMD_TSF
	},
#endif
	[QCA_NL80211_VENDOR_SUBCMD_SCAN_DONE_INDEX] = {
		.vendor_id = QCA_NL80211_VENDOR_ID,
		.subcmd = QCA_NL80211_VENDOR_SUBCMD_SCAN_DONE
	},
	[QCA_NL80211_VENDOR_SUBCMD_SCAN_INDEX] = {
		.vendor_id = QCA_NL80211_VENDOR_ID,
		.subcmd = QCA_NL80211_VENDOR_SUBCMD_TRIGGER_SCAN
	},
	/* OCB events */
	[QCA_NL80211_VENDOR_SUBCMD_DCC_STATS_EVENT_INDEX] = {
		.vendor_id = QCA_NL80211_VENDOR_ID,
		.subcmd = QCA_NL80211_VENDOR_SUBCMD_DCC_STATS_EVENT
	},
#ifdef FEATURE_LFR_SUBNET_DETECTION
	[QCA_NL80211_VENDOR_SUBCMD_GW_PARAM_CONFIG_INDEX] = {
		.vendor_id = QCA_NL80211_VENDOR_ID,
		.subcmd = QCA_NL80211_VENDOR_SUBCMD_GW_PARAM_CONFIG
	},
#endif /*FEATURE_LFR_SUBNET_DETECTION */

#ifdef WLAN_FEATURE_NAN_DATAPATH
	[QCA_NL80211_VENDOR_SUBCMD_NDP_INDEX] = {
		.vendor_id = QCA_NL80211_VENDOR_ID,
		.subcmd = QCA_NL80211_VENDOR_SUBCMD_NDP
	},
#endif /* WLAN_FEATURE_NAN_DATAPATH */

	[QCA_NL80211_VENDOR_SUBCMD_P2P_LO_EVENT_INDEX] = {
		.vendor_id = QCA_NL80211_VENDOR_ID,
		.subcmd = QCA_NL80211_VENDOR_SUBCMD_P2P_LISTEN_OFFLOAD_STOP
	},
	[QCA_NL80211_VENDOR_SUBCMD_SAP_CONDITIONAL_CHAN_SWITCH_INDEX] = {
		.vendor_id = QCA_NL80211_VENDOR_ID,
		.subcmd = QCA_NL80211_VENDOR_SUBCMD_SAP_CONDITIONAL_CHAN_SWITCH
	},
	[QCA_NL80211_VENDOR_SUBCMD_UPDATE_EXTERNAL_ACS_CONFIG] = {
		.vendor_id = QCA_NL80211_VENDOR_ID,
		.subcmd = QCA_NL80211_VENDOR_SUBCMD_EXTERNAL_ACS
	},
	[QCA_NL80211_VENDOR_SUBCMD_PWR_SAVE_FAIL_DETECTED_INDEX] = {
		.vendor_id = QCA_NL80211_VENDOR_ID,
		.subcmd = QCA_NL80211_VENDOR_SUBCMD_CHIP_PWRSAVE_FAILURE
	},
	[QCA_NL80211_VENDOR_SUBCMD_NUD_STATS_GET_INDEX] = {
		.vendor_id = QCA_NL80211_VENDOR_ID,
		.subcmd = QCA_NL80211_VENDOR_SUBCMD_NUD_STATS_GET,
	},
	[QCA_NL80211_VENDOR_SUBCMD_HANG_REASON_INDEX] = {
		.vendor_id = QCA_NL80211_VENDOR_ID,
		.subcmd = QCA_NL80211_VENDOR_SUBCMD_HANG,
	},
	[QCA_NL80211_VENDOR_SUBCMD_WLAN_MAC_INFO_INDEX] = {
		.vendor_id = QCA_NL80211_VENDOR_ID,
		.subcmd = QCA_NL80211_VENDOR_SUBCMD_WLAN_MAC_INFO,
	},
#ifdef WLAN_UMAC_CONVERGENCE
	COMMON_VENDOR_EVENTS
#endif
};

/**
 * __is_driver_dfs_capable() - get driver DFS capability
 * @wiphy:   pointer to wireless wiphy structure.
 * @wdev:    pointer to wireless_dev structure.
 * @data:    Pointer to the data to be passed via vendor interface
 * @data_len:Length of the data to be passed
 *
 * This function is called by userspace to indicate whether or not
 * the driver supports DFS offload.
 *
 * Return: 0 on success, negative errno on failure
 */
static int __is_driver_dfs_capable(struct wiphy *wiphy,
				   struct wireless_dev *wdev,
				   const void *data,
				   int data_len)
{
	u32 dfs_capability = 0;
	struct sk_buff *temp_skbuff;
	int ret_val;
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);

	ENTER_DEV(wdev->netdev);

	ret_val = wlan_hdd_validate_context(hdd_ctx);
	if (ret_val)
		return ret_val;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	dfs_capability = !!(wiphy->flags & WIPHY_FLAG_DFS_OFFLOAD);

	temp_skbuff = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, sizeof(u32) +
							  NLMSG_HDRLEN);

	if (temp_skbuff != NULL) {
		ret_val = nla_put_u32(temp_skbuff, QCA_WLAN_VENDOR_ATTR_DFS,
				      dfs_capability);
		if (ret_val) {
			hdd_err("QCA_WLAN_VENDOR_ATTR_DFS put fail");
			kfree_skb(temp_skbuff);

			return ret_val;
		}

		return cfg80211_vendor_cmd_reply(temp_skbuff);
	}

	hdd_err("dfs capability: buffer alloc fail");
	return -ENOMEM;
}

/**
 * is_driver_dfs_capable() - get driver DFS capability
 * @wiphy:   pointer to wireless wiphy structure.
 * @wdev:    pointer to wireless_dev structure.
 * @data:    Pointer to the data to be passed via vendor interface
 * @data_len:Length of the data to be passed
 *
 * This function is called by userspace to indicate whether or not
 * the driver supports DFS offload.  This is an SSR-protected
 * wrapper function.
 *
 * Return: 0 on success, negative errno on failure
 */
static int is_driver_dfs_capable(struct wiphy *wiphy,
				 struct wireless_dev *wdev,
				 const void *data,
				 int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __is_driver_dfs_capable(wiphy, wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * wlan_hdd_sap_cfg_dfs_override() - DFS MCC restriction check
 *
 * @adapter: SAP adapter pointer
 *
 * DFS in MCC is not supported for Multi bssid SAP mode due to single physical
 * radio. So in case of DFS MCC scenario override current SAP given config
 * to follow concurrent SAP DFS config
 *
 * Return: 0 - No DFS issue, 1 - Override done and negative error codes
 */
int wlan_hdd_sap_cfg_dfs_override(struct hdd_adapter *adapter)
{
	struct hdd_adapter *con_sap_adapter;
	tsap_Config_t *sap_config, *con_sap_config;
	int con_ch;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	if (!hdd_ctx) {
		hdd_err("hdd context is NULL");
		return 0;
	}

	/*
	 * Check if AP+AP case, once primary AP chooses a DFS
	 * channel secondary AP should always follow primary APs channel
	 */
	if (!policy_mgr_concurrent_beaconing_sessions_running(
		hdd_ctx->hdd_psoc))
		return 0;

	con_sap_adapter = hdd_get_con_sap_adapter(adapter, true);
	if (!con_sap_adapter)
		return 0;

	sap_config = &adapter->session.ap.sap_config;
	con_sap_config = &con_sap_adapter->session.ap.sap_config;
	con_ch = con_sap_adapter->session.ap.operating_channel;

	if (!wlan_reg_is_dfs_ch(hdd_ctx->hdd_pdev, con_ch))
		return 0;

	hdd_debug("Only SCC AP-AP DFS Permitted (ch=%d, con_ch=%d)",
						sap_config->channel, con_ch);
	hdd_debug("Overriding guest AP's channel");
	sap_config->channel = con_ch;

	if (con_sap_config->acs_cfg.acs_mode == true) {
		if (con_ch != con_sap_config->acs_cfg.pri_ch &&
				con_ch != con_sap_config->acs_cfg.ht_sec_ch) {
			hdd_err("Primary AP channel config error");
			hdd_err("Operating ch: %d ACS ch: %d %d",
				con_ch, con_sap_config->acs_cfg.pri_ch,
				con_sap_config->acs_cfg.ht_sec_ch);
			return -EINVAL;
		}
		/* Sec AP ACS info is overwritten with Pri AP due to DFS
		 * MCC restriction. So free ch list allocated in do_acs
		 * func for Sec AP and realloc for Pri AP ch list size
		 */
		if (sap_config->acs_cfg.ch_list)
			qdf_mem_free(sap_config->acs_cfg.ch_list);

		qdf_mem_copy(&sap_config->acs_cfg,
					&con_sap_config->acs_cfg,
					sizeof(struct sap_acs_cfg));
		sap_config->acs_cfg.ch_list = qdf_mem_malloc(
					sizeof(uint8_t) *
					con_sap_config->acs_cfg.ch_list_count);
		if (!sap_config->acs_cfg.ch_list) {
			hdd_err("ACS config alloc fail");
			return -ENOMEM;
		}

		qdf_mem_copy(sap_config->acs_cfg.ch_list,
					con_sap_config->acs_cfg.ch_list,
					con_sap_config->acs_cfg.ch_list_count);

	} else {
		sap_config->acs_cfg.pri_ch = con_ch;
		if (sap_config->acs_cfg.ch_width > eHT_CHANNEL_WIDTH_20MHZ)
			sap_config->acs_cfg.ht_sec_ch = con_sap_config->sec_ch;
	}

	return con_ch;
}

/**
 * wlan_hdd_reset_force_acs_chan_range: Set acs channel ranges as per force ACS
 * configuration.
 * @hdd_ctx: pointer to hdd context
 * @sap_config: pointer to SAP config struct
 *
 * Return: 0 if success else error code
 */
static int wlan_hdd_reset_force_acs_chan_range(struct hdd_context *hdd_ctx,
						tsap_Config_t *sap_config)
{
	bool is_dfs_mode_enabled = false;
	uint32_t i, num_channels = 0;
	uint8_t channels[WNI_CFG_VALID_CHANNEL_LIST_LEN] = {0};
	eCsrPhyMode hw_mode;
	tSirMacHTChannelWidth ch_width;

	if (hdd_ctx->config->force_sap_acs_st_ch >
			hdd_ctx->config->force_sap_acs_end_ch) {
		hdd_err("invalid configuration for start and end channel");
		return -EINVAL;
	}
	if (hdd_ctx->config->enableDFSMasterCap)
		is_dfs_mode_enabled = true;

	sap_config->acs_cfg.start_ch =
			hdd_ctx->config->force_sap_acs_st_ch;
	sap_config->acs_cfg.end_ch =
			hdd_ctx->config->force_sap_acs_end_ch;

	for (i = sap_config->acs_cfg.start_ch;
			i <= sap_config->acs_cfg.end_ch; i++) {
		if ((CHANNEL_STATE_ENABLE ==
		     wlan_reg_get_channel_state(hdd_ctx->hdd_pdev, i)) ||
		    (is_dfs_mode_enabled &&
		     CHANNEL_STATE_DFS ==
		     wlan_reg_get_channel_state(hdd_ctx->hdd_pdev, i))) {
			channels[num_channels] = i;
			num_channels++;
		}
	}
	if (sap_config->acs_cfg.ch_list)
		qdf_mem_free(sap_config->acs_cfg.ch_list);

	sap_config->acs_cfg.ch_list = qdf_mem_malloc(num_channels);
	if (!sap_config->acs_cfg.ch_list) {
		hdd_err("ACS config alloc fail");
		return -ENOMEM;
	}
	qdf_mem_copy(sap_config->acs_cfg.ch_list, channels, num_channels);
	sap_config->acs_cfg.ch_list_count = num_channels;

	/* Derive ACS HW mode */
	hw_mode = hdd_cfg_xlate_to_csr_phy_mode(hdd_ctx->config->dot11Mode);
	if (hw_mode == eCSR_DOT11_MODE_AUTO) {
		if (sme_is_feature_supported_by_fw(DOT11AX))
			hw_mode = eCSR_DOT11_MODE_11ax;
		else
			hw_mode = eCSR_DOT11_MODE_11ac;
	}

	if (hdd_ctx->config->sap_force_11n_for_11ac) {
		if (hw_mode == eCSR_DOT11_MODE_11ac ||
		    hw_mode == eCSR_DOT11_MODE_11ac_ONLY)
			hw_mode = eCSR_DOT11_MODE_11n;
	}

	if ((hw_mode == eCSR_DOT11_MODE_11b ||
	     hw_mode == eCSR_DOT11_MODE_11g ||
	     hw_mode == eCSR_DOT11_MODE_11g_ONLY) &&
			sap_config->acs_cfg.start_ch > 14) {
		hdd_err("Invalid ACS HW Mode %d + CH range <%d - %d>",
			hw_mode, sap_config->acs_cfg.start_ch,
			sap_config->acs_cfg.end_ch);
		return -EINVAL;
	}
	sap_config->acs_cfg.hw_mode = hw_mode;

	/* Derive ACS BW */
	ch_width = eHT_CHANNEL_WIDTH_20MHZ;
	if (hw_mode == eCSR_DOT11_MODE_11ac ||
	    hw_mode == eCSR_DOT11_MODE_11ac_ONLY ||
	    hw_mode == eCSR_DOT11_MODE_11ax ||
	    hw_mode == eCSR_DOT11_MODE_11ax_ONLY) {
		ch_width = hdd_ctx->config->vhtChannelWidth;
		/* VHT in 2.4G depends on gChannelBondingMode24GHz INI param */
		if (sap_config->acs_cfg.end_ch <= 14)
			ch_width =
				hdd_ctx->config->nChannelBondingMode24GHz ?
				eHT_CHANNEL_WIDTH_40MHZ :
				eHT_CHANNEL_WIDTH_20MHZ;
	}

	if (hw_mode == eCSR_DOT11_MODE_11n ||
	    hw_mode == eCSR_DOT11_MODE_11n_ONLY) {
		if (sap_config->acs_cfg.end_ch <= 14)
			ch_width =
				hdd_ctx->config->nChannelBondingMode24GHz ?
				eHT_CHANNEL_WIDTH_40MHZ :
				eHT_CHANNEL_WIDTH_20MHZ;
		else
			ch_width =
				hdd_ctx->config->nChannelBondingMode5GHz ?
				eHT_CHANNEL_WIDTH_40MHZ :
				eHT_CHANNEL_WIDTH_20MHZ;
	}
	sap_config->acs_cfg.ch_width = ch_width;

	return 0;
}

/**
 * wlan_hdd_set_acs_ch_range : Populate ACS hw mode and channel range values
 * @sap_cfg: pointer to SAP config struct
 * @hw_mode: hw mode retrieved from vendor command buffer
 * @ht_enabled: whether HT phy mode is enabled
 * @vht_enabled: whether VHT phy mode is enabled
 *
 * This function populates the ACS hw mode based on the configuration retrieved
 * from the vendor command buffer; and sets ACS start and end channel for the
 * given band.
 *
 * Return: 0 if success; -EINVAL if ACS channel list is NULL
 */
static int wlan_hdd_set_acs_ch_range(
	tsap_Config_t *sap_cfg, enum qca_wlan_vendor_acs_hw_mode hw_mode,
	bool ht_enabled, bool vht_enabled)
{
	int i;

	if (hw_mode == QCA_ACS_MODE_IEEE80211B) {
		sap_cfg->acs_cfg.hw_mode = eCSR_DOT11_MODE_11b;
		sap_cfg->acs_cfg.start_ch = WLAN_REG_CH_NUM(CHAN_ENUM_1);
		sap_cfg->acs_cfg.end_ch = WLAN_REG_CH_NUM(CHAN_ENUM_14);
	} else if (hw_mode == QCA_ACS_MODE_IEEE80211G) {
		sap_cfg->acs_cfg.hw_mode = eCSR_DOT11_MODE_11g;
		sap_cfg->acs_cfg.start_ch = WLAN_REG_CH_NUM(CHAN_ENUM_1);
		sap_cfg->acs_cfg.end_ch = WLAN_REG_CH_NUM(CHAN_ENUM_13);
	} else if (hw_mode == QCA_ACS_MODE_IEEE80211A) {
		sap_cfg->acs_cfg.hw_mode = eCSR_DOT11_MODE_11a;
		sap_cfg->acs_cfg.start_ch = WLAN_REG_CH_NUM(CHAN_ENUM_36);
		sap_cfg->acs_cfg.end_ch = WLAN_REG_CH_NUM(CHAN_ENUM_165);
	} else if (hw_mode == QCA_ACS_MODE_IEEE80211ANY) {
		sap_cfg->acs_cfg.hw_mode = eCSR_DOT11_MODE_abg;
		sap_cfg->acs_cfg.start_ch = WLAN_REG_CH_NUM(CHAN_ENUM_1);
		sap_cfg->acs_cfg.end_ch = WLAN_REG_CH_NUM(CHAN_ENUM_165);
	}

	if (ht_enabled)
		sap_cfg->acs_cfg.hw_mode = eCSR_DOT11_MODE_11n;

	if (vht_enabled)
		sap_cfg->acs_cfg.hw_mode = eCSR_DOT11_MODE_11ac;

	/* Parse ACS Chan list from hostapd */
	if (!sap_cfg->acs_cfg.ch_list)
		return -EINVAL;

	sap_cfg->acs_cfg.start_ch = sap_cfg->acs_cfg.ch_list[0];
	sap_cfg->acs_cfg.end_ch =
		sap_cfg->acs_cfg.ch_list[sap_cfg->acs_cfg.ch_list_count - 1];
	for (i = 0; i < sap_cfg->acs_cfg.ch_list_count; i++) {
		/* avoid channel as start channel */
		if (sap_cfg->acs_cfg.start_ch > sap_cfg->acs_cfg.ch_list[i] &&
		    sap_cfg->acs_cfg.ch_list[i] != 0)
			sap_cfg->acs_cfg.start_ch = sap_cfg->acs_cfg.ch_list[i];
		if (sap_cfg->acs_cfg.end_ch < sap_cfg->acs_cfg.ch_list[i])
			sap_cfg->acs_cfg.end_ch = sap_cfg->acs_cfg.ch_list[i];
	}

	return 0;
}


static void wlan_hdd_cfg80211_start_pending_acs(struct work_struct *work);


static void hdd_update_acs_channel_list(tsap_Config_t *sap_config,
					enum band_info band)
{
	int i, temp_count = 0;
	int acs_list_count = sap_config->acs_cfg.ch_list_count;

	for (i = 0; i < acs_list_count; i++) {
		if (BAND_2G == band) {
			if (WLAN_REG_IS_24GHZ_CH(
				sap_config->acs_cfg.ch_list[i])) {
				sap_config->acs_cfg.ch_list[temp_count] =
					sap_config->acs_cfg.ch_list[i];
				temp_count++;
			}
		} else if (BAND_5G == band) {
			if (WLAN_REG_IS_5GHZ_CH(
				sap_config->acs_cfg.ch_list[i])) {
				sap_config->acs_cfg.ch_list[temp_count] =
					sap_config->acs_cfg.ch_list[i];
				temp_count++;
			}
		}
	}
	sap_config->acs_cfg.ch_list_count = temp_count;
}


/**
 * wlan_hdd_cfg80211_start_acs : Start ACS Procedure for SAP
 * @adapter: pointer to SAP adapter struct
 *
 * This function starts the ACS procedure if there are no
 * constraints like MBSSID DFS restrictions.
 *
 * Return: Status of ACS Start procedure
 */
int wlan_hdd_cfg80211_start_acs(struct hdd_adapter *adapter)
{

	struct hdd_context *hdd_ctx;
	tsap_Config_t *sap_config;
	tpWLAN_SAPEventCB acs_event_callback;
	int status;

	if (!adapter) {
		hdd_err("adapater is NULL");
		return -EINVAL;
	}
	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	sap_config = &adapter->session.ap.sap_config;
	if (hdd_ctx->acs_policy.acs_channel)
		sap_config->channel = hdd_ctx->acs_policy.acs_channel;
	else
		sap_config->channel = AUTO_CHANNEL_SELECT;
	/*
	 * No DFS SCC is allowed in Auto use case. Hence not
	 * calling DFS override
	 */
	if (QDF_MCC_TO_SCC_SWITCH_FORCE_PREFERRED_WITHOUT_DISCONNECTION !=
	    hdd_ctx->config->WlanMccToSccSwitchMode) {
		status = wlan_hdd_sap_cfg_dfs_override(adapter);
		if (status < 0)
			return status;

		if (status > 0) {
			/*notify hostapd about channel override */
			wlan_hdd_cfg80211_acs_ch_select_evt(adapter);
			clear_bit(ACS_IN_PROGRESS, &hdd_ctx->g_event_flags);
			return 0;
		}
	}
	/* When first 2 connections are on the same frequency band,
	 * then PCL would include only channels from the other
	 * frequency band on which no connections are active
	 */
	if ((policy_mgr_get_connection_count(hdd_ctx->hdd_psoc) == 2) &&
		(sap_config->acs_cfg.band == QCA_ACS_MODE_IEEE80211ANY)) {
		struct policy_mgr_conc_connection_info *conc_connection_info;
		uint32_t i;

		conc_connection_info = policy_mgr_get_conn_info(&i);
		if (conc_connection_info[0].mac ==
			conc_connection_info[1].mac) {
			if (WLAN_REG_IS_5GHZ_CH(sap_config->acs_cfg.
				pcl_channels[0])) {
				sap_config->acs_cfg.band =
					QCA_ACS_MODE_IEEE80211A;
				hdd_update_acs_channel_list(sap_config,
					BAND_5G);
			} else {
				sap_config->acs_cfg.band =
					QCA_ACS_MODE_IEEE80211G;
				hdd_update_acs_channel_list(sap_config,
					BAND_2G);
			}
		}
	}
	status = wlan_hdd_config_acs(hdd_ctx, adapter);
	if (status) {
		hdd_err("ACS config failed");
		return -EINVAL;
	}

	acs_event_callback = hdd_hostapd_sap_event_cb;

	qdf_mem_copy(sap_config->self_macaddr.bytes,
		adapter->mac_addr.bytes, sizeof(struct qdf_mac_addr));
	hdd_notice("ACS Started for %s", adapter->dev->name);
	status = wlansap_acs_chselect(
		WLAN_HDD_GET_SAP_CTX_PTR(adapter),
		acs_event_callback, sap_config, adapter->dev);


	if (status) {
		hdd_err("ACS channel select failed");
		return -EINVAL;
	}
	if (sap_is_auto_channel_select(WLAN_HDD_GET_SAP_CTX_PTR(adapter)))
		sap_config->acs_cfg.acs_mode = true;
	set_bit(ACS_IN_PROGRESS, &hdd_ctx->g_event_flags);

	return 0;
}

/**
 * hdd_update_vendor_pcl_list() - This API will return unsorted pcl list
 * @hdd_ctx: hdd context
 * @acs_chan_params: external acs channel params
 * @sap_config: SAP config
 *
 * This API provides unsorted pcl list.
 * this list is a subset of the valid channel list given by hostapd.
 * if channel is not present in pcl, weightage will be given as zero
 *
 * Return: Zero on success, non-zero on failure
 */
static void hdd_update_vendor_pcl_list(struct hdd_context *hdd_ctx,
		struct hdd_vendor_acs_chan_params *acs_chan_params,
		tsap_Config_t *sap_config)
{
	int i, j;
	/*
	 * PCL shall contain only the preferred channels from the
	 * application. If those channels are not present in the
	 * driver PCL, then set the weight to zero
	 */
	for (i = 0; i < sap_config->acs_cfg.ch_list_count; i++) {
		acs_chan_params->vendor_pcl_list[i] =
			sap_config->acs_cfg.ch_list[i];
		acs_chan_params->vendor_weight_list[i] = 0;
		for (j = 0; j < sap_config->acs_cfg.pcl_ch_count; j++) {
			if (sap_config->acs_cfg.ch_list[i] ==
			sap_config->acs_cfg.pcl_channels[j]) {
				acs_chan_params->vendor_weight_list[i] =
				sap_config->
				acs_cfg.pcl_channels_weight_list[j];
				break;
			}
		}
	}
	acs_chan_params->pcl_count = sap_config->acs_cfg.ch_list_count;
}

/**
 * hdd_update_reg_chan_info : This API contructs channel info
 * for all the given channel
 * @adapter: pointer to SAP adapter struct
 * @channel_count: channel count
 * @channel_list: channel list
 *
 * Return: Status of of channel information updation
 */
static int hdd_update_reg_chan_info(struct hdd_adapter *adapter,
			uint32_t channel_count,
			uint8_t *channel_list)
{
	int i;
	struct hdd_channel_info *icv;
	struct ch_params ch_params = {0};
	uint8_t bw_offset = 0, chan = 0;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	tsap_Config_t *sap_config = &adapter->session.ap.sap_config;

	/* memory allocation */
	sap_config->channel_info = qdf_mem_malloc(
					sizeof(struct hdd_channel_info) *
					channel_count);
	if (!sap_config->channel_info) {
		hdd_err("memory allocation failed");
		return -ENOMEM;

	}
	sap_config->channel_info_count = channel_count;
	for (i = 0; i < channel_count; i++) {
		icv = &sap_config->channel_info[i];
		chan = channel_list[i];

		if (chan == 0)
			continue;

		if (sap_config->acs_cfg.ch_width == CH_WIDTH_40MHZ)
			bw_offset = 1 << BW_40_OFFSET_BIT;
		else if (sap_config->acs_cfg.ch_width == CH_WIDTH_20MHZ)
			bw_offset = 1 << BW_20_OFFSET_BIT;
		icv->freq = wlan_reg_get_channel_freq(hdd_ctx->hdd_pdev, chan);
		icv->ieee_chan_number = chan;
		icv->max_reg_power = wlan_reg_get_channel_reg_power(
				hdd_ctx->hdd_pdev, chan);

		/* filling demo values */
		icv->max_radio_power = HDD_MAX_TX_POWER;
		icv->min_radio_power = HDD_MIN_TX_POWER;
		/* not supported in current driver */
		icv->max_antenna_gain = 0;

		icv->reg_class_id = wlan_hdd_find_opclass(
					WLAN_HDD_GET_HAL_CTX(adapter),
					chan, bw_offset);

		if (WLAN_REG_IS_5GHZ_CH(chan)) {
			ch_params.ch_width = sap_config->acs_cfg.ch_width;
			wlan_reg_set_channel_params(hdd_ctx->hdd_pdev, chan,
					0, &ch_params);
			icv->vht_center_freq_seg0 = ch_params.center_freq_seg0;
			icv->vht_center_freq_seg1 = ch_params.center_freq_seg1;
		}

		icv->flags = 0;
		icv->flags = cds_get_vendor_reg_flags(hdd_ctx->hdd_pdev, chan,
				sap_config->acs_cfg.ch_width,
				sap_config->acs_cfg.is_ht_enabled,
				sap_config->acs_cfg.is_vht_enabled,
				hdd_ctx->config->enable_sub_20_channel_width);
		if (icv->flags & IEEE80211_CHAN_PASSIVE)
			icv->flagext |= IEEE80211_CHAN_DFS;

		hdd_debug("freq %d flags %d flagext %d ieee %d maxreg %d maxpw %d minpw %d regClass %d antenna %d seg0 %d seg1 %d",
			icv->freq, icv->flags,
			icv->flagext, icv->ieee_chan_number,
			icv->max_reg_power, icv->max_radio_power,
			icv->min_radio_power, icv->reg_class_id,
			icv->max_antenna_gain, icv->vht_center_freq_seg0,
			icv->vht_center_freq_seg1);
	}
	return 0;
}

/* Short name for QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_CHAN_INFO event */
#define CHAN_INFO_ATTR_FLAGS \
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FLAGS
#define CHAN_INFO_ATTR_FLAG_EXT \
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FLAG_EXT
#define CHAN_INFO_ATTR_FREQ \
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FREQ
#define CHAN_INFO_ATTR_MAX_REG_POWER \
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_MAX_REG_POWER
#define CHAN_INFO_ATTR_MAX_POWER \
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_MAX_POWER
#define CHAN_INFO_ATTR_MIN_POWER \
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_MIN_POWER
#define CHAN_INFO_ATTR_REG_CLASS_ID \
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_REG_CLASS_ID
#define CHAN_INFO_ATTR_ANTENNA_GAIN \
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_ANTENNA_GAIN
#define CHAN_INFO_ATTR_VHT_SEG_0 \
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_VHT_SEG_0
#define CHAN_INFO_ATTR_VHT_SEG_1 \
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_VHT_SEG_1

/**
 * hdd_cfg80211_update_channel_info() - add channel info attributes
 * @skb: pointer to sk buff
 * @hdd_ctx: pointer to hdd station context
 * @idx: attribute index
 *
 * Return: Success(0) or reason code for failure
 */
static int32_t
hdd_cfg80211_update_channel_info(struct sk_buff *skb,
			   tsap_Config_t *sap_config, int idx)
{
	struct nlattr *nla_attr, *channel;
	struct hdd_channel_info *icv;
	int i;

	nla_attr = nla_nest_start(skb, idx);
	if (!nla_attr)
		goto fail;

	for (i = 0; i < sap_config->channel_info_count; i++) {
		channel = nla_nest_start(skb, i);
		if (!channel)
			goto fail;

		icv = &sap_config->channel_info[i];
		if (!icv) {
			hdd_err("channel info not found");
			goto fail;
		}
		if (nla_put_u16(skb, CHAN_INFO_ATTR_FREQ,
				icv->freq) ||
		    nla_put_u32(skb, CHAN_INFO_ATTR_FLAGS,
				icv->flags) ||
		    nla_put_u32(skb, CHAN_INFO_ATTR_FLAG_EXT,
				icv->flagext) ||
		    nla_put_u8(skb, CHAN_INFO_ATTR_MAX_REG_POWER,
				icv->max_reg_power) ||
		    nla_put_u8(skb, CHAN_INFO_ATTR_MAX_POWER,
				icv->max_radio_power) ||
		    nla_put_u8(skb, CHAN_INFO_ATTR_MIN_POWER,
				icv->min_radio_power) ||
		    nla_put_u8(skb, CHAN_INFO_ATTR_REG_CLASS_ID,
				icv->reg_class_id) ||
		    nla_put_u8(skb, CHAN_INFO_ATTR_ANTENNA_GAIN,
				icv->max_antenna_gain) ||
		    nla_put_u8(skb, CHAN_INFO_ATTR_VHT_SEG_0,
				icv->vht_center_freq_seg0) ||
		    nla_put_u8(skb, CHAN_INFO_ATTR_VHT_SEG_1,
				icv->vht_center_freq_seg1)) {
			hdd_err("put fail");
			goto fail;
		}
		nla_nest_end(skb, channel);
	}
	nla_nest_end(skb, nla_attr);
	return 0;
fail:
	hdd_err("nl channel update failed");
	return -EINVAL;
}
#undef CHAN_INFO_ATTR_FLAGS
#undef CHAN_INFO_ATTR_FLAG_EXT
#undef CHAN_INFO_ATTR_FREQ
#undef CHAN_INFO_ATTR_MAX_REG_POWER
#undef CHAN_INFO_ATTR_MAX_POWER
#undef CHAN_INFO_ATTR_MIN_POWER
#undef CHAN_INFO_ATTR_REG_CLASS_ID
#undef CHAN_INFO_ATTR_ANTENNA_GAIN
#undef CHAN_INFO_ATTR_VHT_SEG_0
#undef CHAN_INFO_ATTR_VHT_SEG_1

/**
 * hdd_cfg80211_update_pcl() - add pcl info attributes
 * @skb: pointer to sk buff
 * @hdd_ctx: pointer to hdd station context
 * @idx: attribute index
 * @vendor_pcl_list: PCL list
 * @vendor_weight_list: PCL weights
 *
 * Return: Success(0) or reason code for failure
 */
static int32_t
hdd_cfg80211_update_pcl(struct sk_buff *skb,
			uint8_t ch_list_count, int idx,
			uint8_t *vendor_pcl_list, uint8_t *vendor_weight_list)
{
	struct nlattr *nla_attr, *channel;
	int i;

	nla_attr = nla_nest_start(skb, idx);

	if (!nla_attr)
		goto fail;

	for (i = 0; i < ch_list_count; i++) {
		channel = nla_nest_start(skb, i);
		if (!channel)
			goto fail;
		if (nla_put_u8(skb, QCA_WLAN_VENDOR_ATTR_PCL_CHANNEL,
			       vendor_pcl_list[i]) ||
		    nla_put_u8(skb, QCA_WLAN_VENDOR_ATTR_PCL_WEIGHT,
			       vendor_weight_list[i])) {
			hdd_err("put fail");
			goto fail;
		}
		nla_nest_end(skb, channel);
	}
	nla_nest_end(skb, nla_attr);

	return 0;
fail:
	hdd_err("updating pcl list failed");
	return -EINVAL;
}

static void hdd_get_scan_band(struct hdd_context *hdd_ctx,
			      tsap_Config_t *sap_config,
			      enum band_info *band)
{
	/* Get scan band */
	if ((sap_config->acs_cfg.band == QCA_ACS_MODE_IEEE80211B) ||
	   (sap_config->acs_cfg.band == QCA_ACS_MODE_IEEE80211G)) {
		*band = BAND_2G;
	} else if (sap_config->acs_cfg.band == QCA_ACS_MODE_IEEE80211A) {
		*band = BAND_5G;
	} else if (sap_config->acs_cfg.band == QCA_ACS_MODE_IEEE80211ANY) {
		*band = BAND_ALL;
	}
	/* Auto is not supported currently */
	if (!((*band == BAND_2G) || (BAND_5G == *band))) {
		hdd_err("invalid band");
		if (HDD_EXTERNAL_ACS_FREQ_BAND_24GHZ ==
			hdd_ctx->config->external_acs_freq_band)
			*band = BAND_2G;
		else
			*band = BAND_5G;

		hdd_update_acs_channel_list(sap_config, *band);
	}

	if (*band == BAND_2G &&
	    (sap_config->acs_cfg.ch_width == CH_WIDTH_80MHZ ||
	    sap_config->acs_cfg.ch_width == CH_WIDTH_160MHZ ||
	    sap_config->acs_cfg.ch_width == CH_WIDTH_80P80MHZ))
		sap_config->acs_cfg.ch_width = CH_WIDTH_40MHZ;
}


/**
 * hdd_get_freq_list: API to get Frequency list based on channel list
 * @channel_list: channel list
 * @freq_list: frequency list
 * @channel_count: channel count
 *
 * Return: None
 */
static void hdd_get_freq_list(uint8_t *channel_list, uint32_t *freq_list,
				uint32_t channel_count)
{
	int count;

	for (count = 0; count < channel_count ; count++)
		freq_list[count] = cds_chan_to_freq(channel_list[count]);
}

int hdd_cfg80211_update_acs_config(struct hdd_adapter *adapter,
				   uint8_t reason)
{
	struct sk_buff *skb;
	tsap_Config_t *sap_config;
	uint32_t channel_count = 0, status = -EINVAL;
	uint8_t channel_list[QDF_MAX_NUM_CHAN] = {0};
	uint32_t freq_list[QDF_MAX_NUM_CHAN] = {0};
	uint8_t vendor_pcl_list[QDF_MAX_NUM_CHAN] = {0};
	uint8_t vendor_weight_list[QDF_MAX_NUM_CHAN] = {0};
	struct hdd_vendor_acs_chan_params acs_chan_params;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	enum band_info band = BAND_2G;
	eCsrPhyMode phy_mode;
	enum qca_wlan_vendor_attr_external_acs_policy acs_policy;
	uint32_t i;


	if (!hdd_ctx) {
		hdd_err("HDD context is NULL");
		return -EINVAL;
	}

	ENTER();
	sap_config = &adapter->session.ap.sap_config;
	/* When first 2 connections are on the same frequency band,
	 * then PCL would include only channels from the other
	 * frequency band on which no connections are active
	 */
	if ((policy_mgr_get_connection_count(hdd_ctx->hdd_psoc) == 2) &&
	    (sap_config->acs_cfg.band == QCA_ACS_MODE_IEEE80211ANY)) {
		struct policy_mgr_conc_connection_info	*conc_connection_info;

		conc_connection_info = policy_mgr_get_conn_info(&i);
		if (conc_connection_info[0].mac ==
			conc_connection_info[1].mac) {

			if (WLAN_REG_IS_5GHZ_CH(sap_config->acs_cfg.
				pcl_channels[0])) {
				sap_config->acs_cfg.band =
					QCA_ACS_MODE_IEEE80211A;
				hdd_update_acs_channel_list(sap_config,
					BAND_5G);
			} else {
				sap_config->acs_cfg.band =
					QCA_ACS_MODE_IEEE80211G;
				hdd_update_acs_channel_list(sap_config,
					BAND_2G);
			}
		}
	}

	hdd_get_scan_band(hdd_ctx, &adapter->session.ap.sap_config, &band);

	if (sap_config->acs_cfg.ch_list) {
		/* Copy INI or hostapd provided ACS channel range*/
		qdf_mem_copy(channel_list, sap_config->acs_cfg.ch_list,
				sap_config->acs_cfg.ch_list_count);
		channel_count = sap_config->acs_cfg.ch_list_count;
	} else {
		/* No channel list provided, copy all valid channels */
		wlan_hdd_sap_get_valid_channellist(adapter,
			&channel_count,
			channel_list,
			band);
	}

	hdd_update_reg_chan_info(adapter, channel_count, channel_list);
	hdd_get_freq_list(channel_list, freq_list, channel_count);
	/* Get phymode */
	phy_mode = adapter->session.ap.sap_config.acs_cfg.hw_mode;

	skb = cfg80211_vendor_event_alloc(hdd_ctx->wiphy,
			&(adapter->wdev),
			EXTSCAN_EVENT_BUF_SIZE + NLMSG_HDRLEN,
			QCA_NL80211_VENDOR_SUBCMD_UPDATE_EXTERNAL_ACS_CONFIG,
			GFP_KERNEL);

	if (!skb) {
		hdd_err("cfg80211_vendor_event_alloc failed");
		return -ENOMEM;
	}
	/*
	 * Application expects pcl to be a subset of channel list
	 * Remove all channels which are not in channel list from pcl
	 * and add weight as zero
	 */
	acs_chan_params.channel_count = channel_count;
	acs_chan_params.channel_list = channel_list;
	acs_chan_params.vendor_pcl_list = vendor_pcl_list;
	acs_chan_params.vendor_weight_list = vendor_weight_list;

	hdd_update_vendor_pcl_list(hdd_ctx, &acs_chan_params,
				   sap_config);

	if (acs_chan_params.channel_count) {
		hdd_debug("ACS channel list: len: %d",
			  acs_chan_params.channel_count);
		for (i = 0; i < acs_chan_params.channel_count; i++)
			hdd_debug("%d ", acs_chan_params.channel_list[i]);
	}

	if (acs_chan_params.pcl_count) {
		hdd_debug("ACS PCL list: len: %d",
			  acs_chan_params.pcl_count);
		for (i = 0; i < acs_chan_params.pcl_count; i++)
			hdd_debug("channel:%d, weight:%d ",
				  acs_chan_params.
				  vendor_pcl_list[i],
				  acs_chan_params.
				  vendor_weight_list[i]);
	}

	if (HDD_EXTERNAL_ACS_PCL_MANDATORY ==
		hdd_ctx->config->external_acs_policy) {
		acs_policy =
			QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_POLICY_PCL_MANDATORY;
	} else {
		acs_policy =
			QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_POLICY_PCL_PREFERRED;
	}
	/* Update values in NL buffer */
	if (nla_put_u8(skb, QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_REASON,
		       reason) ||
	    nla_put_flag(skb,
		QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_IS_OFFLOAD_ENABLED) ||
	    nla_put_flag(skb,
		QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_ADD_CHAN_STATS_SUPPORT)
		||
	    nla_put_u8(skb, QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_CHAN_WIDTH,
		       sap_config->acs_cfg.ch_width) ||
	    nla_put_u8(skb, QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_BAND,
		       band) ||
	    nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_PHY_MODE,
		       phy_mode) ||
	    nla_put(skb, QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_FREQ_LIST,
		    channel_count * sizeof(uint32_t), freq_list)) {
		hdd_err("nla put fail");
		goto fail;
	}
	status =
	hdd_cfg80211_update_pcl(skb,
				acs_chan_params.
				pcl_count,
				QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_PCL,
				vendor_pcl_list,
				vendor_weight_list);

	if (status != 0)
		goto fail;

	status = hdd_cfg80211_update_channel_info(skb, sap_config,
			QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_CHAN_INFO);

	qdf_mem_free(sap_config->channel_info);
	if (status != 0)
		goto fail;

	status = nla_put_u32(skb,
			     QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_POLICY,
			     acs_policy);

	if (status != 0)
		goto fail;

	cfg80211_vendor_event(skb, GFP_KERNEL);
	return 0;
fail:
	if (skb)
		kfree_skb(skb);
	return status;
}

/**
 * hdd_create_acs_timer(): Initialize vendor ACS timer
 * @adapter: pointer to SAP adapter struct
 *
 * This function initializes the vendor ACS timer.
 *
 * Return: Status of create vendor ACS timer
 */
static int hdd_create_acs_timer(struct hdd_adapter *adapter)
{
	struct hdd_external_acs_timer_context *timer_context;
	QDF_STATUS status;

	if (adapter->session.ap.vendor_acs_timer_initialized)
		return 0;

	hdd_debug("Starting vendor app based ACS");
	timer_context = qdf_mem_malloc(sizeof(*timer_context));
	if (!timer_context) {
		hdd_err("Could not allocate for timer_context");
		return -ENOMEM;
	}
	timer_context->adapter = adapter;

	set_bit(VENDOR_ACS_RESPONSE_PENDING, &adapter->event_flags);
	status = qdf_mc_timer_init(&adapter->session.ap.vendor_acs_timer,
		  QDF_TIMER_TYPE_SW,
		  hdd_acs_response_timeout_handler, timer_context);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("Failed to initialize acs response timeout timer");
		return -EFAULT;
	}
	adapter->session.ap.vendor_acs_timer_initialized = true;
	return 0;
}

static const struct nla_policy
wlan_hdd_cfg80211_do_acs_policy[QCA_WLAN_VENDOR_ATTR_ACS_MAX+1] = {
	[QCA_WLAN_VENDOR_ATTR_ACS_HW_MODE] = { .type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_ACS_HT_ENABLED] = { .type = NLA_FLAG },
	[QCA_WLAN_VENDOR_ATTR_ACS_HT40_ENABLED] = { .type = NLA_FLAG },
	[QCA_WLAN_VENDOR_ATTR_ACS_VHT_ENABLED] = { .type = NLA_FLAG },
	[QCA_WLAN_VENDOR_ATTR_ACS_CHWIDTH] = { .type = NLA_U16 },
	[QCA_WLAN_VENDOR_ATTR_ACS_CH_LIST] = { .type = NLA_UNSPEC },
	[QCA_WLAN_VENDOR_ATTR_ACS_FREQ_LIST] = { .type = NLA_UNSPEC },
};

int hdd_start_vendor_acs(struct hdd_adapter *adapter)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	int status;

	status = hdd_create_acs_timer(adapter);
	if (status != 0) {
		hdd_err("failed to create acs timer");
		return status;
	}
	status = hdd_update_acs_timer_reason(adapter,
		QCA_WLAN_VENDOR_ACS_SELECT_REASON_INIT);
	if (status != 0) {
		hdd_err("failed to update acs timer reason");
		return status;
	}

	if (hdd_ctx->config->acs_support_for_dfs_ltecoex)
		status = qdf_status_to_os_return(wlan_sap_set_vendor_acs(
				WLAN_HDD_GET_SAP_CTX_PTR(adapter),
				true));
	else
		status = qdf_status_to_os_return(wlan_sap_set_vendor_acs(
				WLAN_HDD_GET_SAP_CTX_PTR(adapter),
				false));

	return status;
}

/**
 * __wlan_hdd_cfg80211_do_acs(): CFG80211 handler function for DO_ACS Vendor CMD
 * @wiphy:  Linux wiphy struct pointer
 * @wdev:   Linux wireless device struct pointer
 * @data:   ACS information from hostapd
 * @data_len: ACS information length
 *
 * This function handle DO_ACS Vendor command from hostapd, parses ACS config
 * and starts ACS procedure.
 *
 * Return: ACS procedure start status
 */
static int __wlan_hdd_cfg80211_do_acs(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					const void *data, int data_len)
{
	struct net_device *ndev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(ndev);
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	tsap_Config_t *sap_config;
	struct sk_buff *temp_skbuff;
	int ret, i;
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_ACS_MAX + 1];
	bool ht_enabled, ht40_enabled, vht_enabled;
	uint8_t ch_width;
	enum qca_wlan_vendor_acs_hw_mode hw_mode;
	QDF_STATUS qdf_status;

	/* ***Note*** Donot set SME config related to ACS operation here because
	 * ACS operation is not synchronouse and ACS for Second AP may come when
	 * ACS operation for first AP is going on. So only do_acs is split to
	 * seperate start_acs routine. Also SME-PMAC struct that is used to
	 * pass paremeters from HDD to SAP is global. Thus All ACS related SME
	 * config shall be set only from start_acs.
	 */

	ENTER_DEV(ndev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	if (hdd_ctx->config->force_sap_acs &&
	    !hdd_ctx->config->vendor_acs_support) {
		hdd_err("Hostapd ACS rejected as Driver ACS enabled");
		return -EPERM;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		goto out;

	if (cds_is_sub_20_mhz_enabled()) {
		hdd_err("ACS not supported in sub 20 MHz ch wd.");
		ret = -EINVAL;
		goto out;
	}

	if (qdf_atomic_inc_return(&hdd_ctx->is_acs_allowed) > 1) {
		hdd_err("ACS rejected as previous req already in progress");
		ret = -EINVAL;
		goto out;
	}

	sap_config = &adapter->session.ap.sap_config;
	if (sap_config->acs_cfg.ch_list)
		qdf_mem_free(sap_config->acs_cfg.ch_list);
	qdf_mem_zero(&sap_config->acs_cfg, sizeof(struct sap_acs_cfg));

	ret = wlan_cfg80211_nla_parse(tb, QCA_WLAN_VENDOR_ATTR_ACS_MAX, data,
					 data_len,
					 wlan_hdd_cfg80211_do_acs_policy);
	if (ret) {
		hdd_err("Invalid ATTR");
		qdf_atomic_set(&hdd_ctx->is_acs_allowed, 0);
		goto out;
	}

	if (!tb[QCA_WLAN_VENDOR_ATTR_ACS_HW_MODE]) {
		hdd_err("Attr hw_mode failed");
		ret = -EINVAL;
		qdf_atomic_set(&hdd_ctx->is_acs_allowed, 0);
		goto out;
	}
	hw_mode = nla_get_u8(tb[QCA_WLAN_VENDOR_ATTR_ACS_HW_MODE]);

	if (tb[QCA_WLAN_VENDOR_ATTR_ACS_HT_ENABLED])
		ht_enabled =
			nla_get_flag(tb[QCA_WLAN_VENDOR_ATTR_ACS_HT_ENABLED]);
	else
		ht_enabled = 0;

	if (tb[QCA_WLAN_VENDOR_ATTR_ACS_HT40_ENABLED])
		ht40_enabled =
			nla_get_flag(tb[QCA_WLAN_VENDOR_ATTR_ACS_HT40_ENABLED]);
	else
		ht40_enabled = 0;

	hdd_debug("ht40_enabled %d", ht40_enabled);
	if (tb[QCA_WLAN_VENDOR_ATTR_ACS_VHT_ENABLED])
		vht_enabled =
			nla_get_flag(tb[QCA_WLAN_VENDOR_ATTR_ACS_VHT_ENABLED]);
	else
		vht_enabled = 0;

	if (((adapter->device_mode == QDF_SAP_MODE) &&
	     (hdd_ctx->config->sap_force_11n_for_11ac)) ||
	     ((adapter->device_mode == QDF_P2P_GO_MODE) &&
	     (hdd_ctx->config->go_force_11n_for_11ac))) {
		vht_enabled = 0;
		hdd_log(LOG1, FL("VHT is Disabled in ACS"));
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_ACS_CHWIDTH]) {
		ch_width = nla_get_u16(tb[QCA_WLAN_VENDOR_ATTR_ACS_CHWIDTH]);
	} else {
		if (ht_enabled && ht40_enabled)
			ch_width = 40;
		else
			ch_width = 20;
	}

	/* this may be possible, when sap_force_11n_for_11ac or
	 * go_force_11n_for_11ac is set
	 */
	if ((ch_width == 80 || ch_width == 160) && !vht_enabled) {
		if (ht_enabled && ht40_enabled)
			ch_width = 40;
		else
			ch_width = 20;
	}

	hdd_debug("channel width =%d", ch_width);
	if (ch_width == 160)
		sap_config->acs_cfg.ch_width = CH_WIDTH_160MHZ;
	else if (ch_width == 80)
		sap_config->acs_cfg.ch_width = CH_WIDTH_80MHZ;
	else if (ch_width == 40)
		sap_config->acs_cfg.ch_width = CH_WIDTH_40MHZ;
	else
		sap_config->acs_cfg.ch_width = CH_WIDTH_20MHZ;

	/* hw_mode = a/b/g: QCA_WLAN_VENDOR_ATTR_ACS_CH_LIST and
	 * QCA_WLAN_VENDOR_ATTR_ACS_FREQ_LIST attrs are present, and
	 * QCA_WLAN_VENDOR_ATTR_ACS_CH_LIST is used for obtaining the
	 * channel list, QCA_WLAN_VENDOR_ATTR_ACS_FREQ_LIST is ignored
	 * since it contains the frequency values of the channels in
	 * the channel list.
	 * hw_mode = any: only QCA_WLAN_VENDOR_ATTR_ACS_FREQ_LIST attr
	 * is present
	 */

	wlan_hdd_undo_acs(adapter);
	if (tb[QCA_WLAN_VENDOR_ATTR_ACS_CH_LIST]) {
		char *tmp = nla_data(tb[QCA_WLAN_VENDOR_ATTR_ACS_CH_LIST]);

		sap_config->acs_cfg.ch_list_count = nla_len(
					tb[QCA_WLAN_VENDOR_ATTR_ACS_CH_LIST]);
		if (sap_config->acs_cfg.ch_list_count) {
			sap_config->acs_cfg.ch_list = qdf_mem_malloc(
					sizeof(uint8_t) *
					sap_config->acs_cfg.ch_list_count);
			if (!sap_config->acs_cfg.ch_list) {
				hdd_err("ACS config alloc fail");
				ret = -ENOMEM;
				qdf_atomic_set(&hdd_ctx->is_acs_allowed, 0);
				goto out;
			}

			qdf_mem_copy(sap_config->acs_cfg.ch_list, tmp,
					sap_config->acs_cfg.ch_list_count);
		}
	} else if (tb[QCA_WLAN_VENDOR_ATTR_ACS_FREQ_LIST]) {
		uint32_t *freq =
			nla_data(tb[QCA_WLAN_VENDOR_ATTR_ACS_FREQ_LIST]);
		sap_config->acs_cfg.ch_list_count = nla_len(
			tb[QCA_WLAN_VENDOR_ATTR_ACS_FREQ_LIST]) /
				sizeof(uint32_t);
		if (sap_config->acs_cfg.ch_list_count) {
			sap_config->acs_cfg.ch_list = qdf_mem_malloc(
				sap_config->acs_cfg.ch_list_count);
			if (!sap_config->acs_cfg.ch_list) {
				hdd_err("ACS config alloc fail");
				ret = -ENOMEM;
				qdf_atomic_set(&hdd_ctx->is_acs_allowed, 0);
				goto out;
			}

			/* convert frequency to channel */
			for (i = 0; i < sap_config->acs_cfg.ch_list_count; i++)
				sap_config->acs_cfg.ch_list[i] =
					ieee80211_frequency_to_channel(freq[i]);
		}
	}

	if (!sap_config->acs_cfg.ch_list_count) {
		ret = -EINVAL;
		goto out;
	}

	hdd_debug("get pcl for DO_ACS vendor command");

	/* consult policy manager to get PCL */
	qdf_status = policy_mgr_get_pcl(hdd_ctx->hdd_psoc, PM_SAP_MODE,
				sap_config->acs_cfg.pcl_channels,
				&sap_config->acs_cfg.pcl_ch_count,
				sap_config->acs_cfg.pcl_channels_weight_list,
				QDF_MAX_NUM_CHAN);
	if (qdf_status != QDF_STATUS_SUCCESS)
		hdd_err("Get PCL failed");

	if (sap_config->acs_cfg.pcl_ch_count) {
		hdd_debug("ACS config PCL: len: %d",
			  sap_config->acs_cfg.pcl_ch_count);
		for (i = 0; i < sap_config->acs_cfg.pcl_ch_count; i++)
			hdd_debug("channel:%d, weight:%d ",
				  sap_config->acs_cfg.
				  pcl_channels[i],
				  sap_config->acs_cfg.
				  pcl_channels_weight_list[i]);
	}

	if (hw_mode == QCA_ACS_MODE_IEEE80211ANY)
		policy_mgr_trim_acs_channel_list(hdd_ctx->hdd_psoc,
			sap_config->acs_cfg.ch_list,
			&sap_config->acs_cfg.ch_list_count);

	if (hdd_ctx->config->force_sap_acs) {
		hdd_debug("forcing SAP acs start and end channel");
		ret = wlan_hdd_reset_force_acs_chan_range(hdd_ctx,
						sap_config);
		if (ret) {
			hdd_err("reset force acs channel range failed");
			goto out;
		}
	}

	sap_config->acs_cfg.band = hw_mode;
	ret = wlan_hdd_set_acs_ch_range(sap_config, hw_mode,
					   ht_enabled, vht_enabled);
	if (ret) {
		hdd_err("set acs channel range failed");
		goto out;
	}

	/* ACS override for android */
	if (hdd_ctx->config->sap_p2p_11ac_override && ht_enabled &&
	    sap_config->acs_cfg.end_ch >= WLAN_REG_CH_NUM(CHAN_ENUM_36) &&
	    !(((adapter->device_mode == QDF_SAP_MODE) &&
	      (hdd_ctx->config->sap_force_11n_for_11ac)) ||
	      ((adapter->device_mode == QDF_P2P_GO_MODE) &&
	      (hdd_ctx->config->go_force_11n_for_11ac)))) {
		hdd_debug("ACS Config override for 11AC vhtChannelWidth %d",
			hdd_ctx->config->vhtChannelWidth);
		vht_enabled = 1;
		sap_config->acs_cfg.hw_mode = eCSR_DOT11_MODE_11ac;
		sap_config->acs_cfg.ch_width =
					hdd_ctx->config->vhtChannelWidth;
		/* No VHT80 in 2.4G so perform ACS accordingly */
		if (sap_config->acs_cfg.end_ch <= 14 &&
		    sap_config->acs_cfg.ch_width == eHT_CHANNEL_WIDTH_80MHZ) {
			sap_config->acs_cfg.ch_width = eHT_CHANNEL_WIDTH_40MHZ;
			hdd_debug("resetting to 40Mhz in 2.4Ghz");
		}
	}

	hdd_debug("ACS Config for %s: HW_MODE: %d ACS_BW: %d HT: %d VHT: %d START_CH: %d END_CH: %d band %d",
		adapter->dev->name, sap_config->acs_cfg.hw_mode,
		sap_config->acs_cfg.ch_width, ht_enabled, vht_enabled,
		sap_config->acs_cfg.start_ch, sap_config->acs_cfg.end_ch,
		sap_config->acs_cfg.band);

	if (hdd_ctx->config->auto_channel_select_weight)
		sap_config->auto_channel_select_weight =
		    hdd_ctx->config->auto_channel_select_weight;

	sap_config->acs_cfg.is_ht_enabled = ht_enabled;
	sap_config->acs_cfg.is_vht_enabled = vht_enabled;

	if (sap_config->acs_cfg.ch_list_count) {
		hdd_debug("ACS channel list: len: %d",
					sap_config->acs_cfg.ch_list_count);
		for (i = 0; i < sap_config->acs_cfg.ch_list_count; i++)
			hdd_debug("%d ", sap_config->acs_cfg.ch_list[i]);
	}
	sap_config->acs_cfg.acs_mode = true;
	if (test_bit(ACS_IN_PROGRESS, &hdd_ctx->g_event_flags)) {
		/* ***Note*** Completion variable usage is not allowed
		 * here since ACS scan operation may take max 2.2 sec
		 * for 5G band:
		 *   9 Active channel X 40 ms active scan time +
		 *   16 Passive channel X 110ms passive scan time
		 * Since this CFG80211 call lock rtnl mutex, we cannot hold on
		 * for this long. So we split up the scanning part.
		 */
		set_bit(ACS_PENDING, &adapter->event_flags);
		hdd_debug("ACS Pending for %s", adapter->dev->name);
		ret = 0;
	} else {
		/* Check if vendor specific acs is enabled */
		if (hdd_ctx->config->vendor_acs_support)
			ret = hdd_start_vendor_acs(adapter);
		else
			ret = wlan_hdd_cfg80211_start_acs(adapter);
	}

out:
	if (ret == 0) {
		temp_skbuff = cfg80211_vendor_cmd_alloc_reply_skb(wiphy,
							      NLMSG_HDRLEN);
		if (temp_skbuff != NULL)
			return cfg80211_vendor_cmd_reply(temp_skbuff);
	}
	wlan_hdd_undo_acs(adapter);
	clear_bit(ACS_IN_PROGRESS, &hdd_ctx->g_event_flags);

	return ret;
}

/**
 * wlan_hdd_cfg80211_do_acs : CFG80211 handler function for DO_ACS Vendor CMD
 * @wiphy:  Linux wiphy struct pointer
 * @wdev:   Linux wireless device struct pointer
 * @data:   ACS information from hostapd
 * @data_len: ACS information len
 *
 * This function handle DO_ACS Vendor command from hostapd, parses ACS config
 * and starts ACS procedure.
 *
 * Return: ACS procedure start status
 */

static int wlan_hdd_cfg80211_do_acs(struct wiphy *wiphy,
				    struct wireless_dev *wdev,
				    const void *data, int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_do_acs(wiphy, wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * wlan_hdd_undo_acs : Do cleanup of DO_ACS
 * @adapter:  Pointer to adapter struct
 *
 * This function handle cleanup of what was done in DO_ACS, including free
 * memory.
 *
 * Return: void
 */

void wlan_hdd_undo_acs(struct hdd_adapter *adapter)
{
	if (adapter == NULL)
		return;
	if (adapter->session.ap.sap_config.acs_cfg.ch_list) {
		qdf_mem_free(adapter->session.ap.sap_config.acs_cfg.ch_list);
		adapter->session.ap.sap_config.acs_cfg.ch_list = NULL;
	}
}

/**
 * wlan_hdd_cfg80211_start_pending_acs : Start pending ACS procedure for SAP
 * @work:  Linux workqueue struct pointer for ACS work
 *
 * This function starts the ACS procedure which was marked pending when an ACS
 * procedure was in progress for a concurrent SAP interface.
 *
 * Return: None
 */

static void wlan_hdd_cfg80211_start_pending_acs(struct work_struct *work)
{
	struct hdd_adapter *adapter = container_of(work, struct hdd_adapter,
							acs_pending_work.work);
	if (!adapter)
		return;
	clear_bit(ACS_PENDING, &adapter->event_flags);
	wlan_hdd_cfg80211_start_acs(adapter);
}

/**
 * wlan_hdd_cfg80211_acs_ch_select_evt: Callback function for ACS evt
 * @adapter: Pointer to SAP adapter struct
 * @pri_channel: SAP ACS procedure selected Primary channel
 * @sec_channel: SAP ACS procedure selected secondary channel
 *
 * This is a callback function from SAP module on ACS procedure is completed.
 * This function send the ACS selected channel information to hostapd
 *
 * Return: None
 */

void wlan_hdd_cfg80211_acs_ch_select_evt(struct hdd_adapter *adapter)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	tsap_Config_t *sap_cfg = &(WLAN_HDD_GET_AP_CTX_PTR(adapter))->sap_config;
	struct sk_buff *vendor_event;
	int ret_val;
	struct hdd_adapter *con_sap_adapter;
	uint16_t ch_width;

	vendor_event = cfg80211_vendor_event_alloc(hdd_ctx->wiphy,
			&(adapter->wdev),
			4 * sizeof(u8) + 1 * sizeof(u16) + 4 + NLMSG_HDRLEN,
			QCA_NL80211_VENDOR_SUBCMD_DO_ACS_INDEX,
			GFP_KERNEL);

	if (!vendor_event) {
		hdd_err("cfg80211_vendor_event_alloc failed");
		return;
	}

	ret_val = nla_put_u8(vendor_event,
				QCA_WLAN_VENDOR_ATTR_ACS_PRIMARY_CHANNEL,
				sap_cfg->acs_cfg.pri_ch);
	if (ret_val) {
		hdd_err("QCA_WLAN_VENDOR_ATTR_ACS_PRIMARY_CHANNEL put fail");
		kfree_skb(vendor_event);
		return;
	}

	ret_val = nla_put_u8(vendor_event,
				QCA_WLAN_VENDOR_ATTR_ACS_SECONDARY_CHANNEL,
				sap_cfg->acs_cfg.ht_sec_ch);
	if (ret_val) {
		hdd_err("QCA_WLAN_VENDOR_ATTR_ACS_SECONDARY_CHANNEL put fail");
		kfree_skb(vendor_event);
		return;
	}

	ret_val = nla_put_u8(vendor_event,
			QCA_WLAN_VENDOR_ATTR_ACS_VHT_SEG0_CENTER_CHANNEL,
			sap_cfg->acs_cfg.vht_seg0_center_ch);
	if (ret_val) {
		hdd_err("QCA_WLAN_VENDOR_ATTR_ACS_VHT_SEG0_CENTER_CHANNEL put fail");
		kfree_skb(vendor_event);
		return;
	}

	ret_val = nla_put_u8(vendor_event,
			QCA_WLAN_VENDOR_ATTR_ACS_VHT_SEG1_CENTER_CHANNEL,
			sap_cfg->acs_cfg.vht_seg1_center_ch);
	if (ret_val) {
		hdd_err("QCA_WLAN_VENDOR_ATTR_ACS_VHT_SEG1_CENTER_CHANNEL put fail");
		kfree_skb(vendor_event);
		return;
	}

	if (sap_cfg->acs_cfg.ch_width == CH_WIDTH_160MHZ)
		ch_width = 160;
	else if (sap_cfg->acs_cfg.ch_width == CH_WIDTH_80MHZ)
		ch_width = 80;
	else if (sap_cfg->acs_cfg.ch_width == CH_WIDTH_40MHZ)
		ch_width = 40;
	else
		ch_width = 20;

	ret_val = nla_put_u16(vendor_event,
				QCA_WLAN_VENDOR_ATTR_ACS_CHWIDTH,
				ch_width);
	if (ret_val) {
		hdd_err("QCA_WLAN_VENDOR_ATTR_ACS_CHWIDTH put fail");
		kfree_skb(vendor_event);
		return;
	}
	if (sap_cfg->acs_cfg.pri_ch > 14)
		ret_val = nla_put_u8(vendor_event,
					QCA_WLAN_VENDOR_ATTR_ACS_HW_MODE,
					QCA_ACS_MODE_IEEE80211A);
	else
		ret_val = nla_put_u8(vendor_event,
					QCA_WLAN_VENDOR_ATTR_ACS_HW_MODE,
					QCA_ACS_MODE_IEEE80211G);

	if (ret_val) {
		hdd_err("QCA_WLAN_VENDOR_ATTR_ACS_HW_MODE put fail");
		kfree_skb(vendor_event);
		return;
	}

	hdd_debug("ACS result for %s: PRI_CH: %d SEC_CH: %d VHT_SEG0: %d VHT_SEG1: %d ACS_BW: %d",
		adapter->dev->name, sap_cfg->acs_cfg.pri_ch,
		sap_cfg->acs_cfg.ht_sec_ch, sap_cfg->acs_cfg.vht_seg0_center_ch,
		sap_cfg->acs_cfg.vht_seg1_center_ch, ch_width);

	cfg80211_vendor_event(vendor_event, GFP_KERNEL);
	/* ***Note*** As already mentioned Completion variable usage is not
	 * allowed here since ACS scan operation may take max 2.2 sec.
	 * Further in AP-AP mode pending ACS is resumed here to serailize ACS
	 * operation.
	 * TODO: Delayed operation is used since SME-PMAC strut is global. Thus
	 * when Primary AP ACS is complete and secondary AP ACS is started here
	 * immediately, Primary AP start_bss may come inbetween ACS operation
	 * and overwrite Sec AP ACS paramters. Thus Sec AP ACS is executed with
	 * delay. This path and below constraint will be removed on sessionizing
	 * SAP acs parameters and decoupling SAP from PMAC (WIP).
	 * As per design constraint user space control application must take
	 * care of serailizing hostapd start for each VIF in AP-AP mode to avoid
	 * this code path. Sec AP hostapd should be started after Primary AP
	 * start beaconing which can be confirmed by getchannel iwpriv command
	 */

	con_sap_adapter = hdd_get_con_sap_adapter(adapter, false);
	if (con_sap_adapter &&
		test_bit(ACS_PENDING, &con_sap_adapter->event_flags)) {
		INIT_DELAYED_WORK(&con_sap_adapter->acs_pending_work,
				      wlan_hdd_cfg80211_start_pending_acs);
		/* Lets give 1500ms for OBSS + START_BSS to complete */
		schedule_delayed_work(&con_sap_adapter->acs_pending_work,
					msecs_to_jiffies(1500));
	}
}

static int
__wlan_hdd_cfg80211_get_supported_features(struct wiphy *wiphy,
					 struct wireless_dev *wdev,
					 const void *data,
					 int data_len)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct sk_buff *skb = NULL;
	uint32_t fset = 0;
	int ret;

	/* ENTER_DEV() intentionally not used in a frequently invoked API */

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	if (wiphy->interface_modes & BIT(NL80211_IFTYPE_STATION)) {
		hdd_debug("Infra Station mode is supported by driver");
		fset |= WIFI_FEATURE_INFRA;
	}
	if (true == hdd_is_5g_supported(hdd_ctx)) {
		hdd_debug("INFRA_5G is supported by firmware");
		fset |= WIFI_FEATURE_INFRA_5G;
	}
#ifdef WLAN_FEATURE_P2P
	if ((wiphy->interface_modes & BIT(NL80211_IFTYPE_P2P_CLIENT)) &&
	    (wiphy->interface_modes & BIT(NL80211_IFTYPE_P2P_GO))) {
		hdd_debug("WiFi-Direct is supported by driver");
		fset |= WIFI_FEATURE_P2P;
	}
#endif
	fset |= WIFI_FEATURE_SOFT_AP;

	/* HOTSPOT is a supplicant feature, enable it by default */
	fset |= WIFI_FEATURE_HOTSPOT;

#ifdef FEATURE_WLAN_EXTSCAN
	if (hdd_ctx->config->extscan_enabled &&
	    sme_is_feature_supported_by_fw(EXTENDED_SCAN)) {
		hdd_debug("EXTScan is supported by firmware");
		fset |= WIFI_FEATURE_EXTSCAN | WIFI_FEATURE_HAL_EPNO;
	}
#endif
	if (wlan_hdd_nan_is_supported()) {
		hdd_debug("NAN is supported by firmware");
		fset |= WIFI_FEATURE_NAN;
	}
	if (sme_is_feature_supported_by_fw(RTT)) {
		hdd_debug("RTT is supported by firmware");
		fset |= WIFI_FEATURE_D2D_RTT;
		fset |= WIFI_FEATURE_D2AP_RTT;
	}
#ifdef FEATURE_WLAN_SCAN_PNO
	if (hdd_ctx->config->configPNOScanSupport &&
	    sme_is_feature_supported_by_fw(PNO)) {
		hdd_debug("PNO is supported by firmware");
		fset |= WIFI_FEATURE_PNO;
	}
#endif
	fset |= WIFI_FEATURE_ADDITIONAL_STA;
#ifdef FEATURE_WLAN_TDLS
	if ((true == hdd_ctx->config->fEnableTDLSSupport) &&
	    sme_is_feature_supported_by_fw(TDLS)) {
		hdd_debug("TDLS is supported by firmware");
		fset |= WIFI_FEATURE_TDLS;
	}
	if (sme_is_feature_supported_by_fw(TDLS) &&
	    (true == hdd_ctx->config->fEnableTDLSOffChannel) &&
	    sme_is_feature_supported_by_fw(TDLS_OFF_CHANNEL)) {
		hdd_debug("TDLS off-channel is supported by firmware");
		fset |= WIFI_FEATURE_TDLS_OFFCHANNEL;
	}
#endif
#ifdef WLAN_AP_STA_CONCURRENCY
	fset |= WIFI_FEATURE_AP_STA;
#endif
	fset |= WIFI_FEATURE_RSSI_MONITOR;
	fset |= WIFI_FEATURE_TX_TRANSMIT_POWER;
	fset |= WIFI_FEATURE_SET_TX_POWER_LIMIT;
	fset |= WIFI_FEATURE_CONFIG_NDO;

	if (hdd_link_layer_stats_supported())
		fset |= WIFI_FEATURE_LINK_LAYER_STATS;

	if (hdd_roaming_supported(hdd_ctx))
		fset |= WIFI_FEATURE_CONTROL_ROAMING;

	if (hdd_scan_random_mac_addr_supported())
		fset |= WIFI_FEATURE_SCAN_RAND;

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, sizeof(fset) +
						  NLMSG_HDRLEN);
	if (!skb) {
		hdd_err("cfg80211_vendor_cmd_alloc_reply_skb failed");
		return -EINVAL;
	}
	hdd_debug("Supported Features : 0x%x", fset);
	if (nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_FEATURE_SET, fset)) {
		hdd_err("nla put fail");
		goto nla_put_failure;
	}
	ret = cfg80211_vendor_cmd_reply(skb);
	return ret;
nla_put_failure:
	kfree_skb(skb);
	return -EINVAL;
}

/**
 * wlan_hdd_cfg80211_get_supported_features() - get supported features
 * @wiphy:   pointer to wireless wiphy structure.
 * @wdev:    pointer to wireless_dev structure.
 * @data:    Pointer to the data to be passed via vendor interface
 * @data_len:Length of the data to be passed
 *
 * Return:   Return the Success or Failure code.
 */
static int
wlan_hdd_cfg80211_get_supported_features(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					const void *data, int data_len)
{
	int ret = 0;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_get_supported_features(wiphy, wdev,
						data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __wlan_hdd_cfg80211_set_scanning_mac_oui() - set scan MAC
 * @wiphy:   pointer to wireless wiphy structure.
 * @wdev:    pointer to wireless_dev structure.
 * @data:    Pointer to the data to be passed via vendor interface
 * @data_len:Length of the data to be passed
 *
 * Set the MAC address that is to be used for scanning.
 *
 * Return:   Return the Success or Failure code.
 */
static int
__wlan_hdd_cfg80211_set_scanning_mac_oui(struct wiphy *wiphy,
					 struct wireless_dev *wdev,
					 const void *data,
					 int data_len)
{
	tpSirScanMacOui pReqMsg = NULL;
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_SET_SCANNING_MAC_OUI_MAX + 1];
	QDF_STATUS status;
	int ret;
	int len;
	struct net_device *ndev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(ndev);

	ENTER_DEV(wdev->netdev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	if (false == hdd_ctx->config->enable_mac_spoofing) {
		hdd_warn("MAC address spoofing is not enabled");
		return -ENOTSUPP;
	}

	/*
	 * audit note: it is ok to pass a NULL policy here since only
	 * one attribute is parsed and it is explicitly validated
	 */
	if (wlan_cfg80211_nla_parse(tb,
				  QCA_WLAN_VENDOR_ATTR_SET_SCANNING_MAC_OUI_MAX,
				  data, data_len, NULL)) {
		hdd_err("Invalid ATTR");
		return -EINVAL;
	}
	pReqMsg = qdf_mem_malloc(sizeof(*pReqMsg));
	if (!pReqMsg) {
		hdd_err("qdf_mem_malloc failed");
		return -ENOMEM;
	}

	if (!tb[QCA_WLAN_VENDOR_ATTR_SET_SCANNING_MAC_OUI]) {
		hdd_err("attr mac oui failed");
		goto fail;
	}

	len = nla_len(tb[QCA_WLAN_VENDOR_ATTR_SET_SCANNING_MAC_OUI]);
	if (len != sizeof(pReqMsg->oui)) {
		hdd_err("attr mac oui invalid size %d expected %zu",
			len, sizeof(pReqMsg->oui));
		goto fail;
	}

	nla_memcpy(&pReqMsg->oui[0],
		   tb[QCA_WLAN_VENDOR_ATTR_SET_SCANNING_MAC_OUI],
		   sizeof(pReqMsg->oui));

	/* populate pReqMsg for mac addr randomization */
	pReqMsg->vdev_id = adapter->session_id;
	pReqMsg->enb_probe_req_sno_randomization = true;

	hdd_debug("Oui (%02x:%02x:%02x), vdev_id = %d", pReqMsg->oui[0],
		  pReqMsg->oui[1], pReqMsg->oui[2], pReqMsg->vdev_id);

	hdd_update_ie_whitelist_attr(&pReqMsg->ie_whitelist, hdd_ctx->config);

	status = sme_set_scanning_mac_oui(hdd_ctx->hHal, pReqMsg);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("sme_set_scanning_mac_oui failed(err=%d)", status);
		goto fail;
	}
	return 0;
fail:
	qdf_mem_free(pReqMsg);
	return -EINVAL;
}

/**
 * wlan_hdd_cfg80211_set_scanning_mac_oui() - set scan MAC
 * @wiphy:   pointer to wireless wiphy structure.
 * @wdev:    pointer to wireless_dev structure.
 * @data:    Pointer to the data to be passed via vendor interface
 * @data_len:Length of the data to be passed
 *
 * Set the MAC address that is to be used for scanning.  This is an
 * SSR-protecting wrapper function.
 *
 * Return:   Return the Success or Failure code.
 */
static int
wlan_hdd_cfg80211_set_scanning_mac_oui(struct wiphy *wiphy,
				       struct wireless_dev *wdev,
				       const void *data,
				       int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_set_scanning_mac_oui(wiphy, wdev,
						       data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

#define MAX_CONCURRENT_MATRIX \
	QCA_WLAN_VENDOR_ATTR_GET_CONCURRENCY_MATRIX_MAX
#define MATRIX_CONFIG_PARAM_SET_SIZE_MAX \
	QCA_WLAN_VENDOR_ATTR_GET_CONCURRENCY_MATRIX_CONFIG_PARAM_SET_SIZE_MAX
static const struct nla_policy
wlan_hdd_get_concurrency_matrix_policy[MAX_CONCURRENT_MATRIX + 1] = {
	[MATRIX_CONFIG_PARAM_SET_SIZE_MAX] = {.type = NLA_U32},
};

/**
 * __wlan_hdd_cfg80211_get_concurrency_matrix() - to retrieve concurrency matrix
 * @wiphy: pointer phy adapter
 * @wdev: pointer to wireless device structure
 * @data: pointer to data buffer
 * @data_len: length of data
 *
 * This routine will give concurrency matrix
 *
 * Return: int status code
 */
static int __wlan_hdd_cfg80211_get_concurrency_matrix(struct wiphy *wiphy,
					 struct wireless_dev *wdev,
					 const void *data,
					 int data_len)
{
	uint32_t feature_set_matrix[CDS_MAX_FEATURE_SET] = {0};
	uint8_t i, feature_sets, max_feature_sets;
	struct nlattr *tb[MAX_CONCURRENT_MATRIX + 1];
	struct sk_buff *reply_skb;
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	int ret;

	ENTER_DEV(wdev->netdev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	if (wlan_cfg80211_nla_parse(tb, MAX_CONCURRENT_MATRIX, data, data_len,
				    wlan_hdd_get_concurrency_matrix_policy)) {
		hdd_err("Invalid ATTR");
		return -EINVAL;
	}

	/* Parse and fetch max feature set */
	if (!tb[MATRIX_CONFIG_PARAM_SET_SIZE_MAX]) {
		hdd_err("Attr max feature set size failed");
		return -EINVAL;
	}
	max_feature_sets = nla_get_u32(tb[MATRIX_CONFIG_PARAM_SET_SIZE_MAX]);
	hdd_debug("Max feature set size: %d", max_feature_sets);

	/* Fill feature combination matrix */
	feature_sets = 0;
	feature_set_matrix[feature_sets++] = WIFI_FEATURE_INFRA |
						WIFI_FEATURE_P2P;
	feature_set_matrix[feature_sets++] = WIFI_FEATURE_INFRA |
						WIFI_FEATURE_NAN;
	/* Add more feature combinations here */

	feature_sets = QDF_MIN(feature_sets, max_feature_sets);
	hdd_debug("Number of feature sets: %d", feature_sets);
	hdd_debug("Feature set matrix");
	for (i = 0; i < feature_sets; i++)
		hdd_debug("[%d] 0x%02X", i, feature_set_matrix[i]);

	reply_skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, sizeof(u32) +
			sizeof(u32) * feature_sets + NLMSG_HDRLEN);
	if (!reply_skb) {
		hdd_err("Feature set matrix: buffer alloc fail");
		return -ENOMEM;
	}

	if (nla_put_u32(reply_skb,
			QCA_WLAN_VENDOR_ATTR_GET_CONCURRENCY_MATRIX_RESULTS_SET_SIZE,
			feature_sets) ||
	    nla_put(reply_skb,
		    QCA_WLAN_VENDOR_ATTR_GET_CONCURRENCY_MATRIX_RESULTS_SET,
		    sizeof(u32) * feature_sets,
		    feature_set_matrix)) {
		hdd_err("nla put fail");
		kfree_skb(reply_skb);
		return -EINVAL;
	}
	return cfg80211_vendor_cmd_reply(reply_skb);
}

#undef MAX_CONCURRENT_MATRIX
#undef MATRIX_CONFIG_PARAM_SET_SIZE_MAX

/**
 * wlan_hdd_cfg80211_get_concurrency_matrix() - get concurrency matrix
 * @wiphy:   pointer to wireless wiphy structure.
 * @wdev:    pointer to wireless_dev structure.
 * @data:    Pointer to the data to be passed via vendor interface
 * @data_len:Length of the data to be passed
 *
 * Retrieves the concurrency feature set matrix
 *
 * Return: 0 on success, negative errno on failure
 */
static int
wlan_hdd_cfg80211_get_concurrency_matrix(struct wiphy *wiphy,
				       struct wireless_dev *wdev,
				       const void *data,
				       int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_get_concurrency_matrix(wiphy, wdev,
							data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * wlan_hdd_cfg80211_set_feature() - Set the bitmask for supported features
 * @feature_flags: pointer to the byte array of features.
 * @feature: Feature to be turned ON in the byte array.
 *
 * Return: None
 *
 * This is called to turn ON or SET the feature flag for the requested feature.
 **/
#define NUM_BITS_IN_BYTE       8
static void wlan_hdd_cfg80211_set_feature(uint8_t *feature_flags,
					  uint8_t feature)
{
	uint32_t index;
	uint8_t bit_mask;

	index = feature / NUM_BITS_IN_BYTE;
	bit_mask = 1 << (feature % NUM_BITS_IN_BYTE);
	feature_flags[index] |= bit_mask;
}

/**
 * __wlan_hdd_cfg80211_get_features() - Get the Driver Supported features
 * @wiphy: pointer to wireless wiphy structure.
 * @wdev: pointer to wireless_dev structure.
 * @data: Pointer to the data to be passed via vendor interface
 * @data_len:Length of the data to be passed
 *
 * This is called when wlan driver needs to send supported feature set to
 * supplicant upon a request/query from the supplicant.
 *
 * Return: Return the Success or Failure code.
 **/
#define MAX_CONCURRENT_CHAN_ON_24G    2
#define MAX_CONCURRENT_CHAN_ON_5G     2
static int
__wlan_hdd_cfg80211_get_features(struct wiphy *wiphy,
				 struct wireless_dev *wdev,
				 const void *data, int data_len)
{
	struct sk_buff *skb = NULL;
	uint32_t dbs_capability = 0;
	bool one_by_one_dbs, two_by_two_dbs;
	QDF_STATUS ret = QDF_STATUS_E_FAILURE;
	int ret_val;

	uint8_t feature_flags[(NUM_QCA_WLAN_VENDOR_FEATURES + 7) / 8] = {0};
	struct hdd_context *hdd_ctx_ptr = wiphy_priv(wiphy);

	ENTER_DEV(wdev->netdev);

	ret_val = wlan_hdd_validate_context(hdd_ctx_ptr);
	if (ret_val)
		return ret_val;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	if (roaming_offload_enabled(hdd_ctx_ptr)) {
		hdd_debug("Key Mgmt Offload is supported");
		wlan_hdd_cfg80211_set_feature(feature_flags,
				QCA_WLAN_VENDOR_FEATURE_KEY_MGMT_OFFLOAD);
	}

	wlan_hdd_cfg80211_set_feature(feature_flags,
				QCA_WLAN_VENDOR_FEATURE_SUPPORT_HW_MODE_ANY);
	if (policy_mgr_is_scan_simultaneous_capable(hdd_ctx_ptr->hdd_psoc))
		wlan_hdd_cfg80211_set_feature(feature_flags,
			QCA_WLAN_VENDOR_FEATURE_OFFCHANNEL_SIMULTANEOUS);

	if (wma_is_p2p_lo_capable())
		wlan_hdd_cfg80211_set_feature(feature_flags,
			QCA_WLAN_VENDOR_FEATURE_P2P_LISTEN_OFFLOAD);

	if (hdd_ctx_ptr->config->oce_sta_enabled)
		wlan_hdd_cfg80211_set_feature(feature_flags,
					      QCA_WLAN_VENDOR_FEATURE_OCE_STA);

	if (hdd_ctx_ptr->config->oce_sap_enabled)
		wlan_hdd_cfg80211_set_feature(feature_flags,
					  QCA_WLAN_VENDOR_FEATURE_OCE_STA_CFON);

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, sizeof(feature_flags) +
			NLMSG_HDRLEN);

	if (!skb) {
		hdd_err("cfg80211_vendor_cmd_alloc_reply_skb failed");
		return -ENOMEM;
	}

	if (nla_put(skb, QCA_WLAN_VENDOR_ATTR_FEATURE_FLAGS,
			sizeof(feature_flags), feature_flags))
		goto nla_put_failure;

	ret = policy_mgr_get_dbs_hw_modes(hdd_ctx_ptr->hdd_psoc,
		&one_by_one_dbs, &two_by_two_dbs);
	if (QDF_STATUS_SUCCESS == ret) {
		if (one_by_one_dbs)
			dbs_capability = DRV_DBS_CAPABILITY_1X1;

		if (two_by_two_dbs)
			dbs_capability = DRV_DBS_CAPABILITY_2X2;

		if (!one_by_one_dbs && !two_by_two_dbs)
			dbs_capability = DRV_DBS_CAPABILITY_DISABLED;
	} else {
		hdd_err("wma_get_dbs_hw_mode failed");
		dbs_capability = DRV_DBS_CAPABILITY_DISABLED;
	}

	hdd_debug("dbs_capability is %d", dbs_capability);

	if (nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_CONCURRENCY_CAPA,
			dbs_capability))
		goto nla_put_failure;


	if (nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_MAX_CONCURRENT_CHANNELS_2_4_BAND,
			MAX_CONCURRENT_CHAN_ON_24G))
		goto nla_put_failure;

	if (nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_MAX_CONCURRENT_CHANNELS_5_0_BAND,
			MAX_CONCURRENT_CHAN_ON_5G))
		goto nla_put_failure;

	return cfg80211_vendor_cmd_reply(skb);

nla_put_failure:
	kfree_skb(skb);
	return -EINVAL;
}

/**
 * wlan_hdd_cfg80211_get_features() - Get the Driver Supported features
 * @wiphy:   pointer to wireless wiphy structure.
 * @wdev:    pointer to wireless_dev structure.
 * @data:    Pointer to the data to be passed via vendor interface
 * @data_len:Length of the data to be passed
 *
 * This is called when wlan driver needs to send supported feature set to
 * supplicant upon a request/query from the supplicant.
 *
 * Return:   Return the Success or Failure code.
 */
static int
wlan_hdd_cfg80211_get_features(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_get_features(wiphy, wdev,
					       data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

#define PARAM_NUM_NW \
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_WHITE_LIST_SSID_NUM_NETWORKS
#define PARAM_SET_BSSID \
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_BSSID_PARAMS_BSSID
#define PARAM_SSID_LIST QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_WHITE_LIST_SSID_LIST
#define PARAM_LIST_SSID  QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_WHITE_LIST_SSID
#define MAX_ROAMING_PARAM \
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_MAX
#define PARAM_NUM_BSSID \
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_LAZY_ROAM_NUM_BSSID
#define PARAM_BSSID_PREFS \
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_BSSID_PREFS
#define PARAM_ROAM_BSSID \
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_LAZY_ROAM_BSSID
#define PARAM_RSSI_MODIFIER \
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_LAZY_ROAM_RSSI_MODIFIER
#define PARAMS_NUM_BSSID \
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_BSSID_PARAMS_NUM_BSSID
#define PARAM_BSSID_PARAMS \
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_BSSID_PARAMS
#define PARAM_A_BAND_BOOST_THLD \
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_A_BAND_BOOST_THRESHOLD
#define PARAM_A_BAND_PELT_THLD \
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_A_BAND_PENALTY_THRESHOLD
#define PARAM_A_BAND_BOOST_FACTOR \
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_A_BAND_BOOST_FACTOR
#define PARAM_A_BAND_PELT_FACTOR \
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_A_BAND_PENALTY_FACTOR
#define PARAM_A_BAND_MAX_BOOST \
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_A_BAND_MAX_BOOST
#define PARAM_ROAM_HISTERESYS \
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_LAZY_ROAM_HISTERESYS
#define PARAM_RSSI_TRIGGER \
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_ALERT_ROAM_RSSI_TRIGGER
#define PARAM_ROAM_ENABLE \
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_LAZY_ROAM_ENABLE


static const struct nla_policy
wlan_hdd_set_roam_param_policy[MAX_ROAMING_PARAM + 1] = {
	[QCA_WLAN_VENDOR_ATTR_ROAMING_SUBCMD] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_ROAMING_REQ_ID] = {.type = NLA_U32},
	[PARAM_NUM_NW] = {.type = NLA_U32},
	[PARAM_A_BAND_BOOST_FACTOR] = {.type = NLA_U32},
	[PARAM_A_BAND_PELT_FACTOR] = {.type = NLA_U32},
	[PARAM_A_BAND_MAX_BOOST] = {.type = NLA_U32},
	[PARAM_ROAM_HISTERESYS] = {.type = NLA_S32},
	[PARAM_A_BAND_BOOST_THLD] = {.type = NLA_S32},
	[PARAM_A_BAND_PELT_THLD] = {.type = NLA_S32},
	[PARAM_RSSI_TRIGGER] = {.type = NLA_U32},
	[PARAM_ROAM_ENABLE] = {	.type = NLA_S32},
	[PARAM_NUM_BSSID] = {.type = NLA_U32},
	[PARAM_RSSI_MODIFIER] = {.type = NLA_U32},
	[PARAMS_NUM_BSSID] = {.type = NLA_U32},
	[PARAM_ROAM_BSSID] = {.type = NLA_UNSPEC, .len = QDF_MAC_ADDR_SIZE},
	[PARAM_SET_BSSID] = {.type = NLA_UNSPEC, .len = QDF_MAC_ADDR_SIZE},
};

/**
 * hdd_set_white_list() - parse white list
 * @hddctx:        HDD context
 * @roam_params:   roam params
 * @tb:            list of attributes
 * @session_id:    session id
 *
 * Return: 0 on success; error number on failure
 */
static int hdd_set_white_list(struct hdd_context *hddctx,
				struct roam_ext_params *roam_params,
				struct nlattr **tb, uint8_t session_id)
{
	int rem, i;
	uint32_t buf_len = 0, count;
	struct nlattr *tb2[MAX_ROAMING_PARAM + 1];
	struct nlattr *curr_attr = NULL;

	i = 0;
	if (tb[PARAM_NUM_NW]) {
		count = nla_get_u32(tb[PARAM_NUM_NW]);
	} else {
		hdd_err("Number of networks is not provided");
		goto fail;
	}

	if (count && tb[PARAM_SSID_LIST]) {
		nla_for_each_nested(curr_attr,
			tb[PARAM_SSID_LIST], rem) {
			if (wlan_cfg80211_nla_parse(tb2,
					   QCA_WLAN_VENDOR_ATTR_ROAM_SUBCMD_MAX,
					   nla_data(curr_attr),
					   nla_len(curr_attr),
					   wlan_hdd_set_roam_param_policy)) {
				hdd_err("nla_parse failed");
				goto fail;
			}
			/* Parse and Fetch allowed SSID list*/
			if (!tb2[PARAM_LIST_SSID]) {
				hdd_err("attr allowed ssid failed");
				goto fail;
			}
			buf_len = nla_len(tb2[PARAM_LIST_SSID]);
			/*
			 * Upper Layers include a null termination
			 * character. Check for the actual permissible
			 * length of SSID and also ensure not to copy
			 * the NULL termination character to the driver
			 * buffer.
			 */
			if (buf_len && (i < MAX_SSID_ALLOWED_LIST) &&
			    ((buf_len - 1) <= SIR_MAC_MAX_SSID_LENGTH)) {
				nla_memcpy(roam_params->ssid_allowed_list[i].ssId,
					tb2[PARAM_LIST_SSID], buf_len - 1);
				roam_params->ssid_allowed_list[i].length = buf_len - 1;
				hdd_debug("SSID[%d]: %.*s,length = %d",
					i,
					roam_params->ssid_allowed_list[i].length,
					roam_params->ssid_allowed_list[i].ssId,
					roam_params->ssid_allowed_list[i].length);
					i++;
			} else {
				hdd_err("Invalid buffer length");
			}
		}
	}

	if (i != count) {
		hdd_err("Invalid number of SSIDs i = %d, count = %d", i, count);
		goto fail;
	}

	roam_params->num_ssid_allowed_list = i;
	hdd_debug("Num of Allowed SSID %d", roam_params->num_ssid_allowed_list);
	sme_update_roam_params(hddctx->hHal, session_id,
			       roam_params, REASON_ROAM_SET_SSID_ALLOWED);
	return 0;

fail:
	return -EINVAL;
}

/**
 * hdd_set_bssid_prefs() - parse set bssid prefs
 * @hddctx:        HDD context
 * @roam_params:   roam params
 * @tb:            list of attributes
 * @session_id:    session id
 *
 * Return: 0 on success; error number on failure
 */
static int hdd_set_bssid_prefs(struct hdd_context *hddctx,
				struct roam_ext_params *roam_params,
				struct nlattr **tb, uint8_t session_id)
{
	int rem, i;
	uint32_t count;
	struct nlattr *tb2[MAX_ROAMING_PARAM + 1];
	struct nlattr *curr_attr = NULL;

	/* Parse and fetch number of preferred BSSID */
	if (!tb[PARAM_NUM_BSSID]) {
		hdd_err("attr num of preferred bssid failed");
		goto fail;
	}
	count = nla_get_u32(tb[PARAM_NUM_BSSID]);
	if (count > MAX_BSSID_FAVORED) {
		hdd_err("Preferred BSSID count %u exceeds max %u",
			count, MAX_BSSID_FAVORED);
		goto fail;
	}
	hdd_debug("Num of Preferred BSSID (%d)", count);
	if (!tb[QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_BSSID_PREFS]) {
		hdd_err("attr Preferred BSSID failed");
		goto fail;
	}

	i = 0;
	nla_for_each_nested(curr_attr,
		tb[QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_BSSID_PREFS],
		rem) {
		if (i == count) {
			hdd_warn("Ignoring excess Preferred BSSID");
			break;
		}

		if (wlan_cfg80211_nla_parse(tb2,
					 QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_MAX,
					 nla_data(curr_attr),
					 nla_len(curr_attr),
					 wlan_hdd_set_roam_param_policy)) {
			hdd_err("nla_parse failed");
			goto fail;
		}
		/* Parse and fetch MAC address */
		if (!tb2[PARAM_ROAM_BSSID]) {
			hdd_err("attr mac address failed");
			goto fail;
		}
		nla_memcpy(roam_params->bssid_favored[i].bytes,
			  tb2[PARAM_ROAM_BSSID],
			  QDF_MAC_ADDR_SIZE);
		hdd_debug(MAC_ADDRESS_STR,
			  MAC_ADDR_ARRAY(roam_params->bssid_favored[i].bytes));
		/* Parse and fetch preference factor*/
		if (!tb2[PARAM_RSSI_MODIFIER]) {
			hdd_err("BSSID Preference score failed");
			goto fail;
		}
		roam_params->bssid_favored_factor[i] = nla_get_u32(
			tb2[PARAM_RSSI_MODIFIER]);
		hdd_debug("BSSID Preference score (%d)",
			  roam_params->bssid_favored_factor[i]);
		i++;
	}
	if (i < count)
		hdd_warn("Num Preferred BSSID %u less than expected %u",
				 i, count);

	roam_params->num_bssid_favored = i;
	sme_update_roam_params(hddctx->hHal, session_id,
			       roam_params, REASON_ROAM_SET_FAVORED_BSSID);

	return 0;

fail:
	return -EINVAL;
}

/**
 * hdd_set_blacklist_bssid() - parse set blacklist bssid
 * @hddctx:        HDD context
 * @roam_params:   roam params
 * @tb:            list of attributes
 * @session_id:    session id
 *
 * Return: 0 on success; error number on failure
 */
static int hdd_set_blacklist_bssid(struct hdd_context *hddctx,
				struct roam_ext_params *roam_params,
				struct nlattr **tb,
				uint8_t session_id)
{
	int rem, i;
	uint32_t count;
	struct nlattr *tb2[MAX_ROAMING_PARAM + 1];
	struct nlattr *curr_attr = NULL;

	/* Parse and fetch number of blacklist BSSID */
	if (!tb[PARAMS_NUM_BSSID]) {
		hdd_err("attr num of blacklist bssid failed");
		goto fail;
	}
	count = nla_get_u32(tb[PARAMS_NUM_BSSID]);
	if (count > MAX_BSSID_AVOID_LIST) {
		hdd_err("Blacklist BSSID count %u exceeds max %u",
			count, MAX_BSSID_AVOID_LIST);
		goto fail;
	}
	hdd_debug("Num of blacklist BSSID (%d)", count);

	i = 0;
	if (count && tb[PARAM_BSSID_PARAMS]) {
		nla_for_each_nested(curr_attr,
			tb[PARAM_BSSID_PARAMS],
			rem) {
			if (i == count) {
				hdd_warn("Ignoring excess Blacklist BSSID");
				break;
			}

			if (wlan_cfg80211_nla_parse(tb2,
					 QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_MAX,
					 nla_data(curr_attr),
					 nla_len(curr_attr),
					 wlan_hdd_set_roam_param_policy)) {
				hdd_err("nla_parse failed");
				goto fail;
			}
			/* Parse and fetch MAC address */
			if (!tb2[PARAM_SET_BSSID]) {
				hdd_err("attr blacklist addr failed");
				goto fail;
			}
			nla_memcpy(roam_params->bssid_avoid_list[i].bytes,
				   tb2[PARAM_SET_BSSID], QDF_MAC_ADDR_SIZE);
			hdd_debug(MAC_ADDRESS_STR,
				  MAC_ADDR_ARRAY(roam_params->bssid_avoid_list[i].bytes));
			i++;
		}
	}

	if (i < count)
		hdd_warn("Num Blacklist BSSID %u less than expected %u",
			 i, count);

	roam_params->num_bssid_avoid_list = i;
	sme_update_roam_params(hddctx->hHal, session_id,
			       roam_params, REASON_ROAM_SET_BLACKLIST_BSSID);

	return 0;
fail:
	return -EINVAL;
}

/**
 * hdd_set_ext_roam_params() - parse ext roam params
 * @hddctx:        HDD context
 * @roam_params:   roam params
 * @tb:            list of attributes
 * @session_id:    session id
 *
 * Return: 0 on success; error number on failure
 */
static int hdd_set_ext_roam_params(struct hdd_context *hddctx,
				const void *data, int data_len,
				uint8_t session_id,
				struct roam_ext_params *roam_params)
{
	uint32_t cmd_type, req_id;
	struct nlattr *tb[MAX_ROAMING_PARAM + 1];
	int ret;

	if (wlan_cfg80211_nla_parse(tb, MAX_ROAMING_PARAM, data, data_len,
				    wlan_hdd_set_roam_param_policy)) {
		hdd_err("Invalid ATTR");
		return -EINVAL;
	}
	/* Parse and fetch Command Type */
	if (!tb[QCA_WLAN_VENDOR_ATTR_ROAMING_SUBCMD]) {
		hdd_err("roam cmd type failed");
		goto fail;
	}

	cmd_type = nla_get_u32(tb[QCA_WLAN_VENDOR_ATTR_ROAMING_SUBCMD]);
	if (!tb[QCA_WLAN_VENDOR_ATTR_ROAMING_REQ_ID]) {
		hdd_err("attr request id failed");
		goto fail;
	}
	req_id = nla_get_u32(
		tb[QCA_WLAN_VENDOR_ATTR_ROAMING_REQ_ID]);
	hdd_debug("Req Id: %u Cmd Type: %u", req_id, cmd_type);
	switch (cmd_type) {
	case QCA_WLAN_VENDOR_ATTR_ROAM_SUBCMD_SSID_WHITE_LIST:
		ret = hdd_set_white_list(hddctx, roam_params, tb, session_id);
		if (ret)
			goto fail;
		break;

	case QCA_WLAN_VENDOR_ATTR_ROAM_SUBCMD_SET_EXTSCAN_ROAM_PARAMS:
		/* Parse and fetch 5G Boost Threshold */
		if (!tb[PARAM_A_BAND_BOOST_THLD]) {
			hdd_err("5G boost threshold failed");
			goto fail;
		}
		roam_params->raise_rssi_thresh_5g = nla_get_s32(
			tb[PARAM_A_BAND_BOOST_THLD]);
		hdd_debug("5G Boost Threshold (%d)",
			roam_params->raise_rssi_thresh_5g);
		/* Parse and fetch 5G Penalty Threshold */
		if (!tb[PARAM_A_BAND_PELT_THLD]) {
			hdd_err("5G penalty threshold failed");
			goto fail;
		}
		roam_params->drop_rssi_thresh_5g = nla_get_s32(
			tb[PARAM_A_BAND_PELT_THLD]);
		hdd_debug("5G Penalty Threshold (%d)",
			roam_params->drop_rssi_thresh_5g);
		/* Parse and fetch 5G Boost Factor */
		if (!tb[PARAM_A_BAND_BOOST_FACTOR]) {
			hdd_err("5G boost Factor failed");
			goto fail;
		}
		roam_params->raise_factor_5g = nla_get_u32(
			tb[PARAM_A_BAND_BOOST_FACTOR]);
		hdd_debug("5G Boost Factor (%d)",
			roam_params->raise_factor_5g);
		/* Parse and fetch 5G Penalty factor */
		if (!tb[PARAM_A_BAND_PELT_FACTOR]) {
			hdd_err("5G Penalty Factor failed");
			goto fail;
		}
		roam_params->drop_factor_5g = nla_get_u32(
			tb[PARAM_A_BAND_PELT_FACTOR]);
		hdd_debug("5G Penalty factor (%d)",
			roam_params->drop_factor_5g);
		/* Parse and fetch 5G Max Boost */
		if (!tb[PARAM_A_BAND_MAX_BOOST]) {
			hdd_err("5G Max Boost failed");
			goto fail;
		}
		roam_params->max_raise_rssi_5g = nla_get_u32(
			tb[PARAM_A_BAND_MAX_BOOST]);
		hdd_debug("5G Max Boost (%d)",
			roam_params->max_raise_rssi_5g);
		/* Parse and fetch Rssi Diff */
		if (!tb[PARAM_ROAM_HISTERESYS]) {
			hdd_err("Rssi Diff failed");
			goto fail;
		}
		roam_params->rssi_diff = nla_get_s32(
			tb[PARAM_ROAM_HISTERESYS]);
		hdd_debug("RSSI Diff (%d)",
			roam_params->rssi_diff);
		/* Parse and fetch Alert Rssi Threshold */
		if (!tb[PARAM_RSSI_TRIGGER]) {
			hdd_err("Alert Rssi Threshold failed");
			goto fail;
		}
		roam_params->alert_rssi_threshold = nla_get_u32(
			tb[PARAM_RSSI_TRIGGER]);
		hdd_debug("Alert RSSI Threshold (%d)",
			roam_params->alert_rssi_threshold);
		sme_update_roam_params(hddctx->hHal, session_id,
			roam_params,
			REASON_ROAM_EXT_SCAN_PARAMS_CHANGED);
		break;
	case QCA_WLAN_VENDOR_ATTR_ROAM_SUBCMD_SET_LAZY_ROAM:
		/* Parse and fetch Activate Good Rssi Roam */
		if (!tb[PARAM_ROAM_ENABLE]) {
			hdd_err("Activate Good Rssi Roam failed");
			goto fail;
		}
		roam_params->good_rssi_roam = nla_get_s32(
			tb[PARAM_ROAM_ENABLE]);
		hdd_debug("Activate Good Rssi Roam (%d)",
			roam_params->good_rssi_roam);
		sme_update_roam_params(hddctx->hHal, session_id,
			roam_params, REASON_ROAM_GOOD_RSSI_CHANGED);
		break;
	case QCA_WLAN_VENDOR_ATTR_ROAM_SUBCMD_SET_BSSID_PREFS:
		ret = hdd_set_bssid_prefs(hddctx, roam_params, tb, session_id);
		if (ret)
			goto fail;
		break;
	case QCA_WLAN_VENDOR_ATTR_ROAM_SUBCMD_SET_BLACKLIST_BSSID:
		ret = hdd_set_blacklist_bssid(hddctx, roam_params, tb, session_id);
		if (ret)
			goto fail;
		break;
	}

	return 0;

fail:
	return -EINVAL;
}

/**
 * __wlan_hdd_cfg80211_set_ext_roam_params() - Settings for roaming parameters
 * @wiphy:                 The wiphy structure
 * @wdev:                  The wireless device
 * @data:                  Data passed by framework
 * @data_len:              Parameters to be configured passed as data
 *
 * The roaming related parameters are configured by the framework
 * using this interface.
 *
 * Return: Return either success or failure code.
 */
static int
__wlan_hdd_cfg80211_set_ext_roam_params(struct wiphy *wiphy,
	struct wireless_dev *wdev, const void *data, int data_len)
{
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct roam_ext_params *roam_params = NULL;
	int ret;

	ENTER_DEV(dev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	if (hdd_ctx->driver_status == DRIVER_MODULES_CLOSED) {
		hdd_err("Driver Modules are closed");
		return -EINVAL;
	}

	roam_params = qdf_mem_malloc(sizeof(*roam_params));
	if (!roam_params) {
		hdd_err("failed to allocate memory");
		return -ENOMEM;
	}

	ret = hdd_set_ext_roam_params(hdd_ctx, data, data_len,
				      adapter->session_id, roam_params);
	if (ret)
		goto fail;

	if (roam_params)
		qdf_mem_free(roam_params);
	return 0;
fail:
	if (roam_params)
		qdf_mem_free(roam_params);

	return ret;
}
#undef PARAM_NUM_NW
#undef PARAM_SET_BSSID
#undef PARAM_SSID_LIST
#undef PARAM_LIST_SSID
#undef MAX_ROAMING_PARAM
#undef PARAM_NUM_BSSID
#undef PARAM_BSSID_PREFS
#undef PARAM_ROAM_BSSID
#undef PARAM_RSSI_MODIFIER
#undef PARAMS_NUM_BSSID
#undef PARAM_BSSID_PARAMS
#undef PARAM_A_BAND_BOOST_THLD
#undef PARAM_A_BAND_PELT_THLD
#undef PARAM_A_BAND_BOOST_FACTOR
#undef PARAM_A_BAND_PELT_FACTOR
#undef PARAM_A_BAND_MAX_BOOST
#undef PARAM_ROAM_HISTERESYS
#undef PARAM_RSSI_TRIGGER
#undef PARAM_ROAM_ENABLE


/**
 * wlan_hdd_cfg80211_set_ext_roam_params() - set ext scan roam params
 * @wiphy:   pointer to wireless wiphy structure.
 * @wdev:    pointer to wireless_dev structure.
 * @data:    Pointer to the data to be passed via vendor interface
 * @data_len:Length of the data to be passed
 *
 * Return:   Return the Success or Failure code.
 */
static int
wlan_hdd_cfg80211_set_ext_roam_params(struct wiphy *wiphy,
				struct wireless_dev *wdev,
				const void *data,
				int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_set_ext_roam_params(wiphy, wdev,
							data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

#define PWR_SAVE_FAIL_CMD_INDEX \
	QCA_NL80211_VENDOR_SUBCMD_PWR_SAVE_FAIL_DETECTED_INDEX
/**
 * hdd_chip_pwr_save_fail_detected_cb() - chip power save failure detected
 * callback
 * @hddctx: HDD context
 * @data: chip power save failure detected data
 *
 * This function reads the chip power save failure detected data and fill in
 * the skb with NL attributes and send up the NL event.
 * This callback execute in atomic context and must not invoke any
 * blocking calls.
 *
 * Return: none
 */
void hdd_chip_pwr_save_fail_detected_cb(void *hddctx,
			struct chip_pwr_save_fail_detected_params
			*data)
{
	struct hdd_context *hdd_ctx	= hddctx;
	struct sk_buff *skb;
	int flags = cds_get_gfp_flags();

	ENTER();

	if (wlan_hdd_validate_context(hdd_ctx))
		return;

	if (!data) {
		hdd_debug("data is null");
		return;
	}

	skb = cfg80211_vendor_event_alloc(hdd_ctx->wiphy,
			  NULL, NLMSG_HDRLEN +
			  sizeof(data->failure_reason_code) +
			  NLMSG_HDRLEN, PWR_SAVE_FAIL_CMD_INDEX,
			  flags);

	if (!skb) {
		hdd_notice("cfg80211_vendor_event_alloc failed");
		return;
	}

	hdd_debug(FL("failure reason code: %u"),
		data->failure_reason_code);

	if (nla_put_u32(skb,
		QCA_ATTR_CHIP_POWER_SAVE_FAILURE_REASON,
		data->failure_reason_code))
		goto fail;

	cfg80211_vendor_event(skb, flags);
	EXIT();
	return;

fail:
	kfree_skb(skb);
}
#undef PWR_SAVE_FAIL_CMD_INDEX

static const struct nla_policy
wlan_hdd_set_no_dfs_flag_config_policy[QCA_WLAN_VENDOR_ATTR_SET_NO_DFS_FLAG_MAX
				       +1] = {
	[QCA_WLAN_VENDOR_ATTR_SET_NO_DFS_FLAG] = {.type = NLA_U32 },
};

/**
 *  wlan_hdd_check_dfs_channel_for_adapter() - check dfs channel in adapter
 *  @hdd_ctx:      HDD context
 *  @device_mode:    device mode
 *  Return:         bool
 */
static bool wlan_hdd_check_dfs_channel_for_adapter(struct hdd_context *hdd_ctx,
				enum QDF_OPMODE device_mode)
{
	struct hdd_adapter *adapter;
	struct hdd_ap_ctx *ap_ctx;
	struct hdd_station_ctx *sta_ctx;

	hdd_for_each_adapter(hdd_ctx, adapter) {
		if ((device_mode == adapter->device_mode) &&
		    (device_mode == QDF_SAP_MODE)) {
			ap_ctx =
				WLAN_HDD_GET_AP_CTX_PTR(adapter);

			/*
			 *  if there is SAP already running on DFS channel,
			 *  do not disable scan on dfs channels. Note that
			 *  with SAP on DFS, there cannot be conurrency on
			 *  single radio. But then we can have multiple
			 *  radios !!
			 */
			if (CHANNEL_STATE_DFS == wlan_reg_get_channel_state(
						hdd_ctx->hdd_pdev,
						ap_ctx->operating_channel)) {
				hdd_err("SAP running on DFS channel");
				return true;
			}
		}

		if ((device_mode == adapter->device_mode) &&
		    (device_mode == QDF_STA_MODE)) {
			sta_ctx =
				WLAN_HDD_GET_STATION_CTX_PTR(adapter);

			/*
			 *  if STA is already connected on DFS channel,
			 *  do not disable scan on dfs channels
			 */
			if (hdd_conn_is_connected(sta_ctx) &&
				(CHANNEL_STATE_DFS ==
				wlan_reg_get_channel_state(hdd_ctx->hdd_pdev,
					sta_ctx->conn_info.operationChannel))) {
				hdd_err("client connected on DFS channel");
				return true;
			}
		}
	}

	return false;
}

/**
 * wlan_hdd_enable_dfs_chan_scan() - disable/enable DFS channels
 * @hdd_ctx: HDD context within host driver
 * @enable_dfs_channels: If true, DFS channels can be used for scanning
 *
 * Loops through devices to see who is operating on DFS channels
 * and then disables/enables DFS channels.
 * Fails the disable request if any device is active on a DFS channel.
 *
 * Return: 0 or other error codes.
 */

int wlan_hdd_enable_dfs_chan_scan(struct hdd_context *hdd_ctx,
				  bool enable_dfs_channels)
{
	QDF_STATUS status;
	bool err;

	if (enable_dfs_channels == hdd_ctx->config->enableDFSChnlScan) {
		hdd_info("DFS channels are already %s",
			 enable_dfs_channels ? "enabled" : "disabled");
		return 0;
	}

	if (!enable_dfs_channels) {
		err = wlan_hdd_check_dfs_channel_for_adapter(hdd_ctx,
							     QDF_STA_MODE);
		if (err)
			return -EOPNOTSUPP;

		err = wlan_hdd_check_dfs_channel_for_adapter(hdd_ctx,
							     QDF_SAP_MODE);
		if (err)
			return -EOPNOTSUPP;
	}

	hdd_ctx->config->enableDFSChnlScan = enable_dfs_channels;

	hdd_abort_mac_scan_all_adapters(hdd_ctx);

	/* pass dfs channel status to regulatory component */
	status = ucfg_reg_enable_dfs_channels(hdd_ctx->hdd_pdev,
			enable_dfs_channels);

	if (QDF_IS_STATUS_ERROR(status))
		hdd_err("Failed to %s DFS channels",
			enable_dfs_channels ? "enable" : "disable");

	return qdf_status_to_os_return(status);
}

/**
 *  __wlan_hdd_cfg80211_disable_dfs_chan_scan() - DFS channel configuration
 *  @wiphy:          corestack handler
 *  @wdev:           wireless device
 *  @data:           data
 *  @data_len:       data length
 *  Return:         success(0) or reason code for failure
 */
static int __wlan_hdd_cfg80211_disable_dfs_chan_scan(struct wiphy *wiphy,
						     struct wireless_dev *wdev,
						     const void *data,
						     int data_len)
{
	struct net_device *dev = wdev->netdev;
	struct hdd_context *hdd_ctx  = wiphy_priv(wiphy);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_SET_NO_DFS_FLAG_MAX + 1];
	int ret_val;
	uint32_t no_dfs_flag = 0;

	ENTER_DEV(dev);

	ret_val = wlan_hdd_validate_context(hdd_ctx);
	if (ret_val)
		return ret_val;

	if (wlan_cfg80211_nla_parse(tb,
				    QCA_WLAN_VENDOR_ATTR_SET_NO_DFS_FLAG_MAX,
				    data, data_len,
				    wlan_hdd_set_no_dfs_flag_config_policy)) {
		hdd_err("invalid attr");
		return -EINVAL;
	}

	if (!tb[QCA_WLAN_VENDOR_ATTR_SET_NO_DFS_FLAG]) {
		hdd_err("attr dfs flag failed");
		return -EINVAL;
	}

	no_dfs_flag = nla_get_u32(
		tb[QCA_WLAN_VENDOR_ATTR_SET_NO_DFS_FLAG]);

	hdd_debug("DFS flag: %d", no_dfs_flag);

	if (no_dfs_flag > 1) {
		hdd_err("invalid value of dfs flag");
		return -EINVAL;
	}

	ret_val = wlan_hdd_enable_dfs_chan_scan(hdd_ctx, !no_dfs_flag);
	return ret_val;
}

/**
 * wlan_hdd_cfg80211_disable_dfs_chan_scan () - DFS scan vendor command
 *
 * @wiphy: wiphy device pointer
 * @wdev: wireless device pointer
 * @data: Vendor command data buffer
 * @data_len: Buffer length
 *
 * Handles QCA_WLAN_VENDOR_ATTR_SET_NO_DFS_FLAG_MAX. Validate it and
 * call wlan_hdd_disable_dfs_chan_scan to send it to firmware.
 *
 * Return: EOK or other error codes.
 */

static int wlan_hdd_cfg80211_disable_dfs_chan_scan(struct wiphy *wiphy,
						   struct wireless_dev *wdev,
						   const void *data,
						   int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_disable_dfs_chan_scan(wiphy, wdev,
							data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

static const struct nla_policy
wlan_hdd_wisa_cmd_policy[QCA_WLAN_VENDOR_ATTR_WISA_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_WISA_MODE] = {.type = NLA_U32 },
};

/**
 * __wlan_hdd_cfg80211_handle_wisa_cmd() - Handle WISA vendor cmd
 * @wiphy: wiphy device pointer
 * @wdev: wireless device pointer
 * @data: Vendor command data buffer
 * @data_len: Buffer length
 *
 * Handles QCA_WLAN_VENDOR_SUBCMD_WISA. Validate cmd attributes and
 * setup WISA Mode features.
 *
 * Return: Success(0) or reason code for failure
 */
static int __wlan_hdd_cfg80211_handle_wisa_cmd(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int data_len)
{
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx  = wiphy_priv(wiphy);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_WISA_MAX + 1];
	struct sir_wisa_params wisa;
	int ret_val;
	QDF_STATUS status;
	bool wisa_mode;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	void *pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	ENTER_DEV(dev);
	ret_val = wlan_hdd_validate_context(hdd_ctx);
	if (ret_val)
		goto err;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	if (wlan_cfg80211_nla_parse(tb, QCA_WLAN_VENDOR_ATTR_WISA_MAX, data,
				    data_len, wlan_hdd_wisa_cmd_policy)) {
		hdd_err("Invalid WISA cmd attributes");
		ret_val = -EINVAL;
		goto err;
	}
	if (!tb[QCA_WLAN_VENDOR_ATTR_WISA_MODE]) {
		hdd_err("Invalid WISA mode");
		ret_val = -EINVAL;
		goto err;
	}

	wisa_mode = !!nla_get_u32(tb[QCA_WLAN_VENDOR_ATTR_WISA_MODE]);
	hdd_debug("WISA Mode: %d", wisa_mode);
	wisa.mode = wisa_mode;
	wisa.vdev_id = adapter->session_id;
	status = sme_set_wisa_params(hdd_ctx->hHal, &wisa);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("Unable to set WISA mode: %d to FW", wisa_mode);
		ret_val = -EINVAL;
	}
	if (QDF_IS_STATUS_SUCCESS(status) || wisa_mode == false)
		cdp_set_wisa_mode(soc,
			(struct cdp_vdev *)cdp_get_vdev_from_vdev_id(soc,
				(struct cdp_pdev *)pdev,
				adapter->session_id),
			wisa_mode);
err:
	EXIT();
	return ret_val;
}

/**
 * wlan_hdd_cfg80211_handle_wisa_cmd() - Handle WISA vendor cmd
 * @wiphy:          corestack handler
 * @wdev:           wireless device
 * @data:           data
 * @data_len:       data length
 *
 * Handles QCA_WLAN_VENDOR_SUBCMD_WISA. Validate cmd attributes and
 * setup WISA mode features.
 *
 * Return: Success(0) or reason code for failure
 */
static int wlan_hdd_cfg80211_handle_wisa_cmd(struct wiphy *wiphy,
						   struct wireless_dev *wdev,
						   const void *data,
						   int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_handle_wisa_cmd(wiphy, wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

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

	hdd_info("congestion:%d", hdd_sta_ctx->conn_info.cca);
	if (nla_put_u32(skb, NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY,
			hdd_sta_ctx->conn_info.cca)) {
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
	if (!nla_attr)
		goto fail;
	/* cfg80211_calculate_bitrate will return 0 for mcs >= 32 */
	bitrate = cfg80211_calculate_bitrate(&hdd_sta_ctx->conn_info.txrate);

	/* report 16-bit bitrate only if we can */
	bitrate_compat = bitrate < (1UL << 16) ? bitrate : 0;
	if (bitrate > 0 &&
	    nla_put_u32(skb, NL80211_RATE_INFO_BITRATE32, bitrate)) {
		hdd_err("put fail");
		goto fail;
	}
	if (bitrate_compat > 0 &&
	    nla_put_u16(skb, NL80211_RATE_INFO_BITRATE, bitrate_compat)) {
		hdd_err("put fail");
		goto fail;
	}
	if (nla_put_u8(skb, NL80211_RATE_INFO_VHT_NSS,
		       hdd_sta_ctx->conn_info.txrate.nss)) {
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
				       struct hdd_station_ctx *hdd_sta_ctx, int idx)
{
	struct nlattr *nla_attr;

	nla_attr = nla_nest_start(skb, idx);
	if (!nla_attr)
		goto fail;
	if (nla_put_u8(skb, NL80211_STA_INFO_SIGNAL,
		       (hdd_sta_ctx->conn_info.signal + 100))) {
		hdd_err("put fail");
		goto fail;
	}
	if (hdd_add_tx_bitrate(skb, hdd_sta_ctx, NL80211_STA_INFO_TX_BITRATE))
		goto fail;
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
			hdd_sta_ctx->conn_info.freq) ||
	    nla_put_u8(skb, NL80211_SURVEY_INFO_NOISE,
		       (hdd_sta_ctx->conn_info.noise + 100))) {
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
	if (!nla_attr)
		goto fail;
	if (nla_put(skb,
		    NL80211_ATTR_SSID,
		    hdd_sta_ctx->conn_info.last_ssid.SSID.length,
		    hdd_sta_ctx->conn_info.last_ssid.SSID.ssId)) {
		hdd_err("put fail");
		goto fail;
	}
	if (nla_put(skb, NL80211_ATTR_MAC, QDF_MAC_ADDR_SIZE,
		    hdd_sta_ctx->conn_info.bssId.bytes)) {
		goto fail;
	}
	if (hdd_add_survey_info(skb, hdd_sta_ctx, NL80211_ATTR_SURVEY_INFO))
		goto fail;
	if (hdd_add_sta_info(skb, hdd_sta_ctx, NL80211_ATTR_STA_INFO))
		goto fail;
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
	if (hdd_sta_ctx->conn_info.conn_flag.vht_present)
		if (nla_put(skb, NL80211_ATTR_VHT_CAPABILITY,
			    sizeof(hdd_sta_ctx->conn_info.vht_caps),
			    &hdd_sta_ctx->conn_info.vht_caps)) {
			hdd_err("put fail");
			goto fail;
		}
	if (hdd_sta_ctx->conn_info.conn_flag.ht_present)
		if (nla_put(skb, NL80211_ATTR_HT_CAPABILITY,
			    sizeof(hdd_sta_ctx->conn_info.ht_caps),
			    &hdd_sta_ctx->conn_info.ht_caps)) {
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
	nl_buf_len += sizeof(hdd_sta_ctx->conn_info.last_ssid.SSID.length) +
		      QDF_MAC_ADDR_SIZE +
		      sizeof(hdd_sta_ctx->conn_info.freq) +
		      sizeof(hdd_sta_ctx->conn_info.noise) +
		      sizeof(hdd_sta_ctx->conn_info.signal) +
		      (sizeof(uint32_t) * 2) +
		      sizeof(hdd_sta_ctx->conn_info.txrate.nss) +
		      sizeof(hdd_sta_ctx->conn_info.roam_count) +
		      sizeof(hdd_sta_ctx->conn_info.last_auth_type) +
		      sizeof(hdd_sta_ctx->conn_info.dot11Mode);
	if (hdd_sta_ctx->conn_info.conn_flag.vht_present)
		nl_buf_len += sizeof(hdd_sta_ctx->conn_info.vht_caps);
	if (hdd_sta_ctx->conn_info.conn_flag.ht_present)
		nl_buf_len += sizeof(hdd_sta_ctx->conn_info.ht_caps);
	if (hdd_sta_ctx->conn_info.conn_flag.hs20_present) {
		tmp_hs20 = (uint8_t *)&(hdd_sta_ctx->conn_info.hs20vendor_ie);
		nl_buf_len += (sizeof(hdd_sta_ctx->conn_info.hs20vendor_ie) -
			       1);
	}
	if (hdd_sta_ctx->conn_info.conn_flag.ht_op_present)
		nl_buf_len += sizeof(hdd_sta_ctx->conn_info.ht_operation);
	if (hdd_sta_ctx->conn_info.conn_flag.vht_op_present)
		nl_buf_len += sizeof(hdd_sta_ctx->conn_info.vht_operation);


	skb = cfg80211_vendor_cmd_alloc_reply_skb(hdd_ctx->wiphy, nl_buf_len);
	if (!skb) {
		hdd_err(FL("cfg80211_vendor_cmd_alloc_reply_skb failed"));
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
			hdd_sta_ctx->conn_info.roam_count) ||
	    nla_put_u32(skb, INFO_AKM,
			hdd_convert_auth_type(
			hdd_sta_ctx->conn_info.last_auth_type)) ||
	    nla_put_u32(skb, WLAN802_11_MODE,
			hdd_convert_dot11mode(
			hdd_sta_ctx->conn_info.dot11Mode))) {
		hdd_err("put fail");
		goto fail;
	}
	if (hdd_sta_ctx->conn_info.conn_flag.ht_op_present)
		if (nla_put(skb, HT_OPERATION,
			    (sizeof(hdd_sta_ctx->conn_info.ht_operation)),
			    &hdd_sta_ctx->conn_info.ht_operation)) {
			hdd_err("put fail");
			goto fail;
		}
	if (hdd_sta_ctx->conn_info.conn_flag.vht_op_present)
		if (nla_put(skb, VHT_OPERATION,
			    (sizeof(hdd_sta_ctx->conn_info.vht_operation)),
			    &hdd_sta_ctx->conn_info.vht_operation)) {
			hdd_err("put fail");
			goto fail;
		}
	if (hdd_sta_ctx->conn_info.conn_flag.hs20_present)
		if (nla_put(skb, AP_INFO_HS20_INDICATION,
			    (sizeof(hdd_sta_ctx->conn_info.hs20vendor_ie) - 1),
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

struct hdd_station_info *hdd_get_stainfo(struct hdd_station_info *astainfo,
					 struct qdf_mac_addr mac_addr)
{
	struct hdd_station_info *stainfo = NULL;
	int i;

	for (i = 0; i < WLAN_MAX_STA_COUNT; i++) {
		if (!qdf_mem_cmp(&astainfo[i].sta_mac,
				 &mac_addr,
				 QDF_MAC_ADDR_SIZE)) {
			stainfo = &astainfo[i];
			break;
		}
	}

	return stainfo;
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
		hdd_add_tx_bitrate_sap_get_len());
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
			(sizeof(stainfo->rx_rate) + NLA_HDRLEN);

	skb = cfg80211_vendor_cmd_alloc_reply_skb(hdd_ctx->wiphy, nl_buf_len);
	if (!skb) {
		hdd_err(FL("cfg80211_vendor_cmd_alloc_reply_skb failed"));
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
			hdd_convert_dot11mode(
			stainfo->mode)) ||
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

	nl_buf_len = NLMSG_HDRLEN;
	nl_buf_len += (sizeof(stainfo->max_phy_rate) + NLA_HDRLEN) +
		(sizeof(stainfo->tx_packets) + NLA_HDRLEN) +
		(sizeof(stainfo->tx_bytes) + NLA_HDRLEN) +
		(sizeof(stainfo->rx_packets) + NLA_HDRLEN) +
		(sizeof(stainfo->rx_bytes) + NLA_HDRLEN) +
		(sizeof(stainfo->is_qos_enabled) + NLA_HDRLEN) +
		(sizeof(stainfo->mode) + NLA_HDRLEN);

	if (!hdd_ctx->config->sap_get_peer_info ||
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

	ENTER_DEV(dev);
	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		status = -EPERM;
		goto out;
	}

	status = wlan_hdd_validate_context(hdd_ctx);
	if (0 != status)
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
	EXIT();
out:
	return status;
}

/**
 * wlan_hdd_cfg80211_get_station_cmd() - Handle get station vendor cmd
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
static int32_t
hdd_cfg80211_get_station_cmd(struct wiphy *wiphy,
			     struct wireless_dev *wdev,
			     const void *data,
			     int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __hdd_cfg80211_get_station_cmd(wiphy, wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/*
 * undef short names defined for get station command
 * used by __wlan_hdd_cfg80211_get_station_cmd()
 */
#undef STATION_INVALID
#undef STATION_INFO
#undef STATION_ASSOC_FAIL_REASON
#undef STATION_REMOTE
#undef STATION_MAX
#undef LINK_INFO_STANDARD_NL80211_ATTR
#undef AP_INFO_STANDARD_NL80211_ATTR
#undef INFO_ROAM_COUNT
#undef INFO_AKM
#undef WLAN802_11_MODE
#undef AP_INFO_HS20_INDICATION
#undef HT_OPERATION
#undef VHT_OPERATION
#undef INFO_ASSOC_FAIL_REASON
#undef REMOTE_MAX_PHY_RATE
#undef REMOTE_TX_PACKETS
#undef REMOTE_TX_BYTES
#undef REMOTE_RX_PACKETS
#undef REMOTE_RX_BYTES
#undef REMOTE_LAST_TX_RATE
#undef REMOTE_LAST_RX_RATE
#undef REMOTE_WMM
#undef REMOTE_SUPPORTED_MODE
#undef REMOTE_AMPDU
#undef REMOTE_TX_STBC
#undef REMOTE_RX_STBC
#undef REMOTE_CH_WIDTH
#undef REMOTE_SGI_ENABLE
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
#undef REMOTE_PAD
#endif

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * __wlan_hdd_cfg80211_keymgmt_set_key() - Store the Keys in the driver session
 * @wiphy: pointer to wireless wiphy structure.
 * @wdev: pointer to wireless_dev structure.
 * @data: Pointer to the Key data
 * @data_len:Length of the data passed
 *
 * This is called when wlan driver needs to save the keys received via
 * vendor specific command.
 *
 * Return: Return the Success or Failure code.
 */
static int __wlan_hdd_cfg80211_keymgmt_set_key(struct wiphy *wiphy,
					       struct wireless_dev *wdev,
					       const void *data, int data_len)
{
	uint8_t local_pmk[SIR_ROAM_SCAN_PSK_SIZE];
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *hdd_adapter_ptr = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx_ptr;
	int status;
	struct pmkid_mode_bits pmkid_modes;

	ENTER_DEV(dev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	if ((data == NULL) || (data_len == 0) ||
			(data_len > SIR_ROAM_SCAN_PSK_SIZE)) {
		hdd_err("Invalid data");
		return -EINVAL;
	}

	hdd_ctx_ptr = WLAN_HDD_GET_CTX(hdd_adapter_ptr);
	if (!hdd_ctx_ptr) {
		hdd_err("HDD context is null");
		return -EINVAL;
	}

	status = wlan_hdd_validate_context(hdd_ctx_ptr);
	if (status)
		return status;

	hdd_get_pmkid_modes(hdd_ctx_ptr, &pmkid_modes);

	sme_update_roam_key_mgmt_offload_enabled(hdd_ctx_ptr->hHal,
			hdd_adapter_ptr->session_id,
			true,
			&pmkid_modes);
	qdf_mem_zero(&local_pmk, SIR_ROAM_SCAN_PSK_SIZE);
	qdf_mem_copy(local_pmk, data, data_len);
	sme_roam_set_psk_pmk(WLAN_HDD_GET_HAL_CTX(hdd_adapter_ptr),
			hdd_adapter_ptr->session_id, local_pmk, data_len);
	return 0;
}

/**
 * wlan_hdd_cfg80211_keymgmt_set_key() - Store the Keys in the driver session
 * @wiphy:   pointer to wireless wiphy structure.
 * @wdev:    pointer to wireless_dev structure.
 * @data:    Pointer to the Key data
 * @data_len:Length of the data passed
 *
 * This is called when wlan driver needs to save the keys received via
 * vendor specific command.
 *
 * Return:   Return the Success or Failure code.
 */
static int wlan_hdd_cfg80211_keymgmt_set_key(struct wiphy *wiphy,
					     struct wireless_dev *wdev,
					     const void *data, int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_keymgmt_set_key(wiphy, wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}
#endif

static const struct nla_policy qca_wlan_vendor_get_wifi_info_policy[
			QCA_WLAN_VENDOR_ATTR_WIFI_INFO_GET_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_WIFI_INFO_DRIVER_VERSION] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_WIFI_INFO_FIRMWARE_VERSION] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_WIFI_INFO_RADIO_INDEX] = {.type = NLA_U32 },
};

/**
 * __wlan_hdd_cfg80211_get_wifi_info() - Get the wifi driver related info
 * @wiphy:   pointer to wireless wiphy structure.
 * @wdev:    pointer to wireless_dev structure.
 * @data:    Pointer to the data to be passed via vendor interface
 * @data_len:Length of the data to be passed
 *
 * This is called when wlan driver needs to send wifi driver related info
 * (driver/fw version) to the user space application upon request.
 *
 * Return:   Return the Success or Failure code.
 */
static int
__wlan_hdd_cfg80211_get_wifi_info(struct wiphy *wiphy,
				  struct wireless_dev *wdev,
				  const void *data, int data_len)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct nlattr *tb_vendor[QCA_WLAN_VENDOR_ATTR_WIFI_INFO_GET_MAX + 1];
	tSirVersionString driver_version;
	tSirVersionString firmware_version;
	const char *hw_version;
	uint32_t major_spid = 0, minor_spid = 0, siid = 0, crmid = 0;
	uint32_t sub_id = 0;
	int status;
	struct sk_buff *reply_skb;
	uint32_t skb_len = 0, count = 0;

	ENTER_DEV(wdev->netdev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	status = wlan_hdd_validate_context(hdd_ctx);
	if (status)
		return status;

	if (wlan_cfg80211_nla_parse(tb_vendor,
				    QCA_WLAN_VENDOR_ATTR_WIFI_INFO_GET_MAX,
				    data, data_len,
				    qca_wlan_vendor_get_wifi_info_policy)) {
		hdd_err("WIFI_INFO_GET NL CMD parsing failed");
		return -EINVAL;
	}

	if (tb_vendor[QCA_WLAN_VENDOR_ATTR_WIFI_INFO_DRIVER_VERSION]) {
		hdd_debug("Rcvd req for Driver version");
		strlcpy(driver_version, QWLAN_VERSIONSTR,
			sizeof(driver_version));
		skb_len += strlen(driver_version) + 1;
		count++;
	}

	if (tb_vendor[QCA_WLAN_VENDOR_ATTR_WIFI_INFO_FIRMWARE_VERSION]) {
		hdd_debug("Rcvd req for FW version");
		hdd_get_fw_version(hdd_ctx, &major_spid, &minor_spid, &siid,
				   &crmid);
		sub_id = (hdd_ctx->target_fw_vers_ext & 0xf0000000) >> 28;
		hw_version = hdd_ctx->target_hw_name;
		snprintf(firmware_version, sizeof(firmware_version),
			"FW:%d.%d.%d.%d.%d HW:%s", major_spid, minor_spid,
			siid, crmid, sub_id, hw_version);
		skb_len += strlen(firmware_version) + 1;
		count++;
	}

	if (count == 0) {
		hdd_err("unknown attribute in get_wifi_info request");
		return -EINVAL;
	}

	skb_len += (NLA_HDRLEN * count) + NLMSG_HDRLEN;
	reply_skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, skb_len);

	if (!reply_skb) {
		hdd_err("cfg80211_vendor_cmd_alloc_reply_skb failed");
		return -ENOMEM;
	}

	if (tb_vendor[QCA_WLAN_VENDOR_ATTR_WIFI_INFO_DRIVER_VERSION]) {
		if (nla_put_string(reply_skb,
			    QCA_WLAN_VENDOR_ATTR_WIFI_INFO_DRIVER_VERSION,
			    driver_version))
			goto error_nla_fail;
	}

	if (tb_vendor[QCA_WLAN_VENDOR_ATTR_WIFI_INFO_FIRMWARE_VERSION]) {
		if (nla_put_string(reply_skb,
			    QCA_WLAN_VENDOR_ATTR_WIFI_INFO_FIRMWARE_VERSION,
			    firmware_version))
			goto error_nla_fail;
	}

	if (tb_vendor[QCA_WLAN_VENDOR_ATTR_WIFI_INFO_RADIO_INDEX]) {
		if (nla_put_u32(reply_skb,
				QCA_WLAN_VENDOR_ATTR_WIFI_INFO_RADIO_INDEX,
				hdd_ctx->radio_index))
			goto error_nla_fail;
	}

	return cfg80211_vendor_cmd_reply(reply_skb);

error_nla_fail:
	hdd_err("nla put fail");
	kfree_skb(reply_skb);
	return -EINVAL;
}

/**
 * wlan_hdd_cfg80211_get_wifi_info() - Get the wifi driver related info
 * @wiphy:   pointer to wireless wiphy structure.
 * @wdev:    pointer to wireless_dev structure.
 * @data:    Pointer to the data to be passed via vendor interface
 * @data_len:Length of the data to be passed
 *
 * This is called when wlan driver needs to send wifi driver related info
 * (driver/fw version) to the user space application upon request.
 *
 * Return:   Return the Success or Failure code.
 */
static int
wlan_hdd_cfg80211_get_wifi_info(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_get_wifi_info(wiphy, wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __wlan_hdd_cfg80211_get_logger_supp_feature() - Get the wifi logger features
 * @wiphy:   pointer to wireless wiphy structure.
 * @wdev:    pointer to wireless_dev structure.
 * @data:    Pointer to the data to be passed via vendor interface
 * @data_len:Length of the data to be passed
 *
 * This is called by userspace to know the supported logger features
 *
 * Return:   Return the Success or Failure code.
 */
static int
__wlan_hdd_cfg80211_get_logger_supp_feature(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int data_len)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	int status;
	uint32_t features;
	struct sk_buff *reply_skb = NULL;

	ENTER_DEV(wdev->netdev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	status = wlan_hdd_validate_context(hdd_ctx);
	if (status)
		return status;

	features = 0;

	features |= WIFI_LOGGER_PER_PACKET_TX_RX_STATUS_SUPPORTED;
	features |= WIFI_LOGGER_CONNECT_EVENT_SUPPORTED;
	features |= WIFI_LOGGER_WAKE_LOCK_SUPPORTED;
	features |= WIFI_LOGGER_DRIVER_DUMP_SUPPORTED;

	reply_skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy,
			sizeof(uint32_t) + NLA_HDRLEN + NLMSG_HDRLEN);
	if (!reply_skb) {
		hdd_err("cfg80211_vendor_cmd_alloc_reply_skb failed");
		return -ENOMEM;
	}

	hdd_debug("Supported logger features: 0x%0x", features);
	if (nla_put_u32(reply_skb, QCA_WLAN_VENDOR_ATTR_LOGGER_SUPPORTED,
				   features)) {
		hdd_err("nla put fail");
		kfree_skb(reply_skb);
		return -EINVAL;
	}

	return cfg80211_vendor_cmd_reply(reply_skb);
}

/**
 * wlan_hdd_cfg80211_get_logger_supp_feature() - Get the wifi logger features
 * @wiphy:   pointer to wireless wiphy structure.
 * @wdev:    pointer to wireless_dev structure.
 * @data:    Pointer to the data to be passed via vendor interface
 * @data_len:Length of the data to be passed
 *
 * This is called by userspace to know the supported logger features
 *
 * Return:   Return the Success or Failure code.
 */
static int
wlan_hdd_cfg80211_get_logger_supp_feature(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_get_logger_supp_feature(wiphy, wdev,
							  data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

#ifdef WLAN_FEATURE_GTK_OFFLOAD
void wlan_hdd_save_gtk_offload_params(struct hdd_adapter *adapter,
				      uint8_t *kck_ptr, uint8_t *kek_ptr,
				      uint32_t kek_len, uint8_t *replay_ctr,
				      bool big_endian)
{
	struct hdd_station_ctx *hdd_sta_ctx;
	uint8_t *buf;
	int i;
	struct pmo_gtk_req *gtk_req = NULL;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	gtk_req = qdf_mem_malloc(sizeof(*gtk_req));
	if (!gtk_req) {
		hdd_err("cannot allocate gtk_req");
		return;
	}

	hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	if (kck_ptr)
		qdf_mem_copy(gtk_req->kck, kck_ptr, NL80211_KCK_LEN);

	if (kek_ptr) {
		/* paranoia */
		if (kek_len > sizeof(gtk_req->kek)) {
			kek_len = sizeof(gtk_req->kek);
			QDF_ASSERT(0);
		}
		qdf_mem_copy(gtk_req->kek, kek_ptr, kek_len);
	}

	qdf_copy_macaddr(&gtk_req->bssid, &hdd_sta_ctx->conn_info.bssId);

	gtk_req->kek_len = kek_len;

	/* convert big to little endian since driver work on little endian */
	buf = (uint8_t *)&gtk_req->replay_counter;
	for (i = 0; i < 8; i++)
		buf[7 - i] = replay_ctr[i];

	status = pmo_ucfg_cache_gtk_offload_req(adapter->hdd_vdev, gtk_req);
	if (status != QDF_STATUS_SUCCESS)
		hdd_err("Failed to cache GTK Offload");

	qdf_mem_free(gtk_req);
}
#else
void wlan_hdd_save_gtk_offload_params(struct hdd_adapter *adapter,
					     uint8_t *kck_ptr,
					     uint8_t *kek_ptr,
					     uint32_t kek_len,
					     uint8_t *replay_ctr,
					     bool big_endian)
{
}
#endif

#if defined(WLAN_FEATURE_FILS_SK) && defined(WLAN_FEATURE_ROAM_OFFLOAD)
/**
 * wlan_hdd_add_fils_params_roam_auth_event() - Adds FILS params in roam auth
 * @skb: SK buffer
 * @roam_info: Roam info
 *
 * API adds fils params[pmk, pmkid, next sequence number] to roam auth event
 *
 * Return: zero on success, error code on failure
 */
static int
wlan_hdd_add_fils_params_roam_auth_event(struct sk_buff *skb,
					 struct csr_roam_info *roam_info)
{
	if (roam_info->pmk_len &&
	    nla_put(skb, QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_PMK,
		    roam_info->pmk_len, roam_info->pmk)) {
		hdd_err("pmk send fail");
		return -EINVAL;
	}

	if (nla_put(skb, QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_PMKID,
		    SIR_PMKID_LEN, roam_info->pmkid)) {
		hdd_err("pmkid send fail");
		return -EINVAL;
	}

	hdd_debug("Update ERP Seq Num %d, Next ERP Seq Num %d",
			roam_info->update_erp_next_seq_num,
			roam_info->next_erp_seq_num);
	if (roam_info->update_erp_next_seq_num &&
	    nla_put_u16(skb,
			QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_FILS_ERP_NEXT_SEQ_NUM,
			roam_info->next_erp_seq_num)) {
		hdd_err("ERP seq num send fail");
		return -EINVAL;
	}

	return 0;
}
#else
static inline int
wlan_hdd_add_fils_params_roam_auth_event(struct sk_buff *skb,
					 struct csr_roam_info *roam_info)
{
	return 0;
}
#endif

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * wlan_hdd_send_roam_auth_event() - Send the roamed and authorized event
 * @adapter: Pointer to adapter struct
 * @bssid: pointer to bssid of roamed AP.
 * @req_rsn_ie: Pointer to request RSN IE
 * @req_rsn_len: Length of the request RSN IE
 * @rsp_rsn_ie: Pointer to response RSN IE
 * @rsp_rsn_len: Length of the response RSN IE
 * @roam_info_ptr: Pointer to the roaming related information
 *
 * This is called when wlan driver needs to send the roaming and
 * authorization information after roaming.
 *
 * The information that would be sent is the request RSN IE, response
 * RSN IE and BSSID of the newly roamed AP.
 *
 * If the Authorized status is authenticated, then additional parameters
 * like PTK's KCK and KEK and Replay Counter would also be passed to the
 * supplicant.
 *
 * The supplicant upon receiving this event would ignore the legacy
 * cfg80211_roamed call and use the entire information from this event.
 * The cfg80211_roamed should still co-exist since the kernel will
 * make use of the parameters even if the supplicant ignores it.
 *
 * Return: Return the Success or Failure code.
 */
int wlan_hdd_send_roam_auth_event(struct hdd_adapter *adapter, uint8_t *bssid,
		uint8_t *req_rsn_ie, uint32_t req_rsn_len, uint8_t *rsp_rsn_ie,
		uint32_t rsp_rsn_len, struct csr_roam_info *roam_info_ptr)
{
	struct hdd_context *hdd_ctx_ptr = WLAN_HDD_GET_CTX(adapter);
	struct sk_buff *skb = NULL;
	eCsrAuthType auth_type;
	uint32_t fils_params_len;
	int status;

	ENTER();

	if (wlan_hdd_validate_context(hdd_ctx_ptr))
		return -EINVAL;

	if (!roaming_offload_enabled(hdd_ctx_ptr) ||
			!roam_info_ptr->roamSynchInProgress)
		return 0;

	/*
	 * PMK is sent from FW in Roam Synch Event for FILS Roaming.
	 * In that case, add three more NL attributes.ie. PMK, PMKID
	 * and ERP next sequence number. Add corresponding lengths
	 * with 3 extra NL message headers for each of the
	 * aforementioned params.
	 */
	fils_params_len = roam_info_ptr->pmk_len + SIR_PMKID_LEN +
			  sizeof(uint16_t) + (3 * NLMSG_HDRLEN);

	skb = cfg80211_vendor_event_alloc(hdd_ctx_ptr->wiphy,
			&(adapter->wdev),
			ETH_ALEN + req_rsn_len + rsp_rsn_len +
			sizeof(uint8_t) + SIR_REPLAY_CTR_LEN +
			SIR_KCK_KEY_LEN + roam_info_ptr->kek_len +
			sizeof(uint8_t) + (8 * NLMSG_HDRLEN) +
			fils_params_len,
			QCA_NL80211_VENDOR_SUBCMD_KEY_MGMT_ROAM_AUTH_INDEX,
			GFP_KERNEL);

	if (!skb) {
		hdd_err("cfg80211_vendor_event_alloc failed");
		return -EINVAL;
	}

	if (nla_put(skb, QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_BSSID,
				ETH_ALEN, bssid) ||
			nla_put(skb, QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_REQ_IE,
				req_rsn_len, req_rsn_ie) ||
			nla_put(skb, QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_RESP_IE,
				rsp_rsn_len, rsp_rsn_ie)) {
		hdd_err("nla put fail");
		goto nla_put_failure;
	}
	if (roam_info_ptr->synchAuthStatus ==
			CSR_ROAM_AUTH_STATUS_AUTHENTICATED) {
		hdd_debug("Include Auth Params TLV's");
		if (nla_put_u8(skb,
			QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_AUTHORIZED, true)) {
			hdd_err("nla put fail");
			goto nla_put_failure;
		}
		auth_type = roam_info_ptr->u.pConnectedProfile->AuthType;
		/* if FT or CCKM connection: dont send replay counter */
		if (auth_type != eCSR_AUTH_TYPE_FT_RSN &&
		    auth_type != eCSR_AUTH_TYPE_FT_RSN_PSK &&
		    auth_type != eCSR_AUTH_TYPE_CCKM_WPA &&
		    auth_type != eCSR_AUTH_TYPE_CCKM_RSN &&
		    nla_put(skb,
			    QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_KEY_REPLAY_CTR,
			    SIR_REPLAY_CTR_LEN,
			    roam_info_ptr->replay_ctr)) {
			hdd_err("non FT/non CCKM connection");
			hdd_err("failed to send replay counter");
			goto nla_put_failure;
		}
		if (roam_info_ptr->kek_len > SIR_KEK_KEY_LEN_FILS ||
		    nla_put(skb,
			QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_PTK_KCK,
			SIR_KCK_KEY_LEN, roam_info_ptr->kck) ||
		    nla_put(skb,
			QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_PTK_KEK,
			roam_info_ptr->kek_len, roam_info_ptr->kek)) {
			hdd_err("nla put fail, kek_len %d",
				roam_info_ptr->kek_len);
			goto nla_put_failure;
		}

		status = wlan_hdd_add_fils_params_roam_auth_event(skb,
							roam_info_ptr);
		if (status)
			goto nla_put_failure;

		/*
		 * Save the gtk rekey parameters in HDD STA context. They will
		 * be used next time when host enables GTK offload and goes
		 * into power save state.
		 */
		wlan_hdd_save_gtk_offload_params(adapter, roam_info_ptr->kck,
						 roam_info_ptr->kek,
						 roam_info_ptr->kek_len,
						 roam_info_ptr->replay_ctr,
						 true);
		hdd_debug("roam_info_ptr->replay_ctr 0x%llx",
			*((uint64_t *)roam_info_ptr->replay_ctr));

	} else {
		hdd_debug("No Auth Params TLV's");
		if (nla_put_u8(skb, QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_AUTHORIZED,
					false)) {
			hdd_err("nla put fail");
			goto nla_put_failure;
		}
	}

	hdd_debug("Auth Status = %d Subnet Change Status = %d",
		  roam_info_ptr->synchAuthStatus,
		  roam_info_ptr->subnet_change_status);

	/*
	 * Add subnet change status if subnet has changed
	 * 0 = unchanged
	 * 1 = changed
	 * 2 = unknown
	 */
	if (roam_info_ptr->subnet_change_status) {
		if (nla_put_u8(skb,
				QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_SUBNET_STATUS,
				roam_info_ptr->subnet_change_status)) {
			hdd_err("nla put fail");
			goto nla_put_failure;
		}
	}

	cfg80211_vendor_event(skb, GFP_KERNEL);
	return 0;

nla_put_failure:
	kfree_skb(skb);
	return -EINVAL;
}
#endif

#define ANT_DIV_PROBE_PERIOD \
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_PROBE_PERIOD
#define ANT_DIV_STAY_PERIOD \
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_STAY_PERIOD
#define ANT_DIV_SNR_DIFF \
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_SNR_DIFF
#define ANT_DIV_PROBE_DWELL_TIME \
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_PROBE_DWELL_TIME
#define ANT_DIV_MGMT_SNR_WEIGHT \
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_MGMT_SNR_WEIGHT
#define ANT_DIV_DATA_SNR_WEIGHT \
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_DATA_SNR_WEIGHT
#define ANT_DIV_ACK_SNR_WEIGHT \
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_ACK_SNR_WEIGHT
#define RX_REORDER_TIMEOUT_VOICE \
	QCA_WLAN_VENDOR_ATTR_CONFIG_RX_REORDER_TIMEOUT_VOICE
#define RX_REORDER_TIMEOUT_VIDEO \
	QCA_WLAN_VENDOR_ATTR_CONFIG_RX_REORDER_TIMEOUT_VIDEO
#define RX_REORDER_TIMEOUT_BESTEFFORT \
	QCA_WLAN_VENDOR_ATTR_CONFIG_RX_REORDER_TIMEOUT_BESTEFFORT
#define RX_REORDER_TIMEOUT_BACKGROUND \
	QCA_WLAN_VENDOR_ATTR_CONFIG_RX_REORDER_TIMEOUT_BACKGROUND
#define RX_BLOCKSIZE_PEER_MAC \
	QCA_WLAN_VENDOR_ATTR_CONFIG_RX_BLOCKSIZE_PEER_MAC
#define RX_BLOCKSIZE_WINLIMIT \
	QCA_WLAN_VENDOR_ATTR_CONFIG_RX_BLOCKSIZE_WINLIMIT
static const struct nla_policy
wlan_hdd_wifi_config_policy[QCA_WLAN_VENDOR_ATTR_CONFIG_MAX + 1] = {

	[QCA_WLAN_VENDOR_ATTR_CONFIG_MODULATED_DTIM] = {.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_STATS_AVG_FACTOR] = {.type = NLA_U16 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_GUARD_TIME] = {.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_FINE_TIME_MEASUREMENT] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_CHANNEL_AVOIDANCE_IND] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_TX_MPDU_AGGREGATION] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_RX_MPDU_AGGREGATION] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_NON_AGG_RETRY] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_AGG_RETRY] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_MGMT_RETRY] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_CTRL_RETRY] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_PROPAGATION_DELAY] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_TX_FAIL_COUNT] = {.type = NLA_U32 },
	[ANT_DIV_PROBE_PERIOD] = {.type = NLA_U32},
	[ANT_DIV_STAY_PERIOD] = {.type = NLA_U32},
	[ANT_DIV_SNR_DIFF] = {.type = NLA_U32},
	[ANT_DIV_PROBE_DWELL_TIME] = {.type = NLA_U32},
	[ANT_DIV_MGMT_SNR_WEIGHT] = {.type = NLA_U32},
	[ANT_DIV_DATA_SNR_WEIGHT] = {.type = NLA_U32},
	[ANT_DIV_ACK_SNR_WEIGHT] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_RESTRICT_OFFCHANNEL] = {.type = NLA_U8},
	[RX_REORDER_TIMEOUT_VOICE] = {.type = NLA_U32},
	[RX_REORDER_TIMEOUT_VIDEO] = {.type = NLA_U32},
	[RX_REORDER_TIMEOUT_BESTEFFORT] = {.type = NLA_U32},
	[RX_REORDER_TIMEOUT_BACKGROUND] = {.type = NLA_U32},
	[RX_BLOCKSIZE_PEER_MAC] = {
		.type = NLA_UNSPEC,
		.len = QDF_MAC_ADDR_SIZE},
	[RX_BLOCKSIZE_WINLIMIT] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_LISTEN_INTERVAL] = {.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_LRO] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_ENA] = {.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_CHAIN] = {.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_SELFTEST] = {.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_SELFTEST_INTVL] = {.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL] = {.type = NLA_U16 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_TOTAL_BEACON_MISS_COUNT] = {.type = NLA_U8},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_SCAN_ENABLE] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_RSN_IE] = {.type = NLA_U8},
};

static const struct nla_policy
wlan_hdd_wifi_test_config_policy[
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_MAX + 1] = {
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_WMM_ENABLE] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_ACCEPT_ADDBA_REQ] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_MCS] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_SEND_ADDBA_REQ] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_FRAGMENTATION] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_WEP_TKIP_IN_HE] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_ADD_DEL_BA_SESSION] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_BA_TID] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_ADDBA_BUFF_SIZE] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_ENABLE_NO_ACK] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_NO_ACK_AC] = {
			.type = NLA_U8},
};

/**
 * wlan_hdd_add_qcn_ie() - Add QCN IE to a given IE buffer
 * @ie_data: IE buffer
 * @ie_len: length of the @ie_data
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS wlan_hdd_add_qcn_ie(uint8_t *ie_data, uint16_t *ie_len)
{
	tDot11fIEQCN_IE qcn_ie;
	uint8_t qcn_ie_hdr[QCN_IE_HDR_LEN]
		= {IE_EID_VENDOR, DOT11F_IE_QCN_IE_MAX_LEN,
			0x8C, 0xFD, 0xF0, 0x1};

	if (((*ie_len) + QCN_IE_HDR_LEN +
		QCN_IE_VERSION_SUBATTR_DATA_LEN) > MAX_DEFAULT_SCAN_IE_LEN) {
		hdd_err("IE buffer not enough for QCN IE");
		return QDF_STATUS_E_FAILURE;
	}

	/* Add QCN IE header */
	qdf_mem_copy(ie_data + (*ie_len), qcn_ie_hdr, QCN_IE_HDR_LEN);
	(*ie_len) += QCN_IE_HDR_LEN;

	/* Retrieve Version sub-attribute data */
	populate_dot11f_qcn_ie(&qcn_ie);

	/* Add QCN IE data[version sub attribute] */
	qdf_mem_copy(ie_data + (*ie_len), qcn_ie.version,
				 (QCN_IE_VERSION_SUBATTR_LEN));
	(*ie_len) += (QCN_IE_VERSION_SUBATTR_LEN);

	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_hdd_save_default_scan_ies() - API to store the default scan IEs
 * @hdd_ctx: HDD context
 * @adapter: Pointer to HDD adapter
 * @ie_data: Pointer to Scan IEs buffer
 * @ie_len: Length of Scan IEs
 *
 * This API is used to store the default scan ies received from
 * supplicant. Also saves QCN IE if g_qcn_ie_support INI is enabled
 *
 * Return: 0 on success; error number otherwise
 */
static int wlan_hdd_save_default_scan_ies(struct hdd_context *hdd_ctx,
					  struct hdd_adapter *adapter,
					  uint8_t *ie_data, uint16_t ie_len)
{
	struct hdd_scan_info *scan_info = &adapter->scan_info;
	bool add_qcn_ie = hdd_ctx->config->qcn_ie_support;

	if (!scan_info)
		return -EINVAL;

	if (scan_info->default_scan_ies) {
		qdf_mem_free(scan_info->default_scan_ies);
		scan_info->default_scan_ies = NULL;
	}

	scan_info->default_scan_ies_len = ie_len;

	if (add_qcn_ie)
		ie_len += (QCN_IE_HDR_LEN + QCN_IE_VERSION_SUBATTR_LEN);

	scan_info->default_scan_ies = qdf_mem_malloc(ie_len);
	if (!scan_info->default_scan_ies) {
		scan_info->default_scan_ies_len = 0;
		return -ENOMEM;
	}

	qdf_mem_copy(scan_info->default_scan_ies, ie_data,
			  scan_info->default_scan_ies_len);

	/* Add QCN IE if g_qcn_ie_support INI is enabled */
	if (add_qcn_ie)
		wlan_hdd_add_qcn_ie(scan_info->default_scan_ies,
					&(scan_info->default_scan_ies_len));

	hdd_debug("Saved default scan IE:");
	qdf_trace_hex_dump(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_DEBUG,
				(uint8_t *) scan_info->default_scan_ies,
				scan_info->default_scan_ies_len);

	return 0;
}

/**
 * wlan_hdd_handle_restrict_offchan_config() -
 * Handle wifi configuration attribute :
 * QCA_WLAN_VENDOR_ATTR_CONFIG_RESTRICT_OFFCHANNEL
 * @adapter: Pointer to HDD adapter
 * @restrict_offchan: Restrict offchannel setting done by
 * application
 *
 * Return: 0 on success; error number otherwise
 */
static int wlan_hdd_handle_restrict_offchan_config(struct hdd_adapter *adapter,
						   u8 restrict_offchan)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	enum QDF_OPMODE dev_mode = adapter->device_mode;
	int ret_val = 0;
	QDF_STATUS status;

	if (!(dev_mode == QDF_SAP_MODE || dev_mode == QDF_P2P_GO_MODE)) {
		hdd_err("Invalid interface type:%d", dev_mode);
		return -EINVAL;
	}
	status = wlan_objmgr_vdev_try_get_ref(adapter->hdd_vdev, WLAN_OSIF_ID);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("Access hdd_vdev failed: %d", status);
		return -EINVAL;
	}
	if (restrict_offchan == 1) {
		enum policy_mgr_con_mode pmode =
		policy_mgr_convert_device_mode_to_qdf_type(dev_mode);
		int chan;

		u32 vdev_id = wlan_vdev_get_id(adapter->hdd_vdev);

		wlan_vdev_obj_lock(adapter->hdd_vdev);
		wlan_vdev_mlme_cap_set(adapter->hdd_vdev,
				       WLAN_VDEV_C_RESTRICT_OFFCHAN);
		wlan_vdev_obj_unlock(adapter->hdd_vdev);
		chan = policy_mgr_get_channel(hdd_ctx->hdd_psoc, pmode,
					      &vdev_id);
		if (!chan ||
		    wlan_hdd_send_avoid_freq_for_dnbs(hdd_ctx, chan)) {
			hdd_err("unable to send avoid_freq");
			ret_val = -EINVAL;
		}
		hdd_info("vdev %d mode %d dnbs enabled", vdev_id, dev_mode);
	} else if (restrict_offchan == 0) {
		wlan_vdev_obj_lock(adapter->hdd_vdev);
		wlan_vdev_mlme_cap_clear(adapter->hdd_vdev,
					 WLAN_VDEV_C_RESTRICT_OFFCHAN);
		wlan_vdev_obj_unlock(adapter->hdd_vdev);
		if (wlan_hdd_send_avoid_freq_for_dnbs(hdd_ctx, 0)) {
			hdd_err("unable to clear avoid_freq");
			ret_val = -EINVAL;
		}
		hdd_info("vdev mode %d dnbs disabled", dev_mode);
	} else {
		ret_val = -EINVAL;
		hdd_err("Invalid RESTRICT_OFFCHAN setting");
	}
	wlan_objmgr_vdev_release_ref(adapter->hdd_vdev, WLAN_OSIF_ID);
	return ret_val;
}

/**
 * wlan_hdd_cfg80211_wifi_set_reorder_timeout - set reorder timeout
 *
 * @hdd_ctx: hdd context
 * @tb: array of pointer to struct nlattr
 *
 * Return: 0 on success; error number otherwise
 */
static int wlan_hdd_cfg80211_wifi_set_reorder_timeout(struct hdd_context *hdd_ctx,
						      struct nlattr *tb[])
{
	int ret_val = 0;
	QDF_STATUS qdf_status;
	struct sir_set_rx_reorder_timeout_val reorder_timeout;

#define RX_TIMEOUT_VAL_MIN 10
#define RX_TIMEOUT_VAL_MAX 1000

	if (tb[RX_REORDER_TIMEOUT_VOICE] ||
	    tb[RX_REORDER_TIMEOUT_VIDEO] ||
	    tb[RX_REORDER_TIMEOUT_BESTEFFORT] ||
	    tb[RX_REORDER_TIMEOUT_BACKGROUND]) {

		/* if one is specified, all must be specified */
		if (!tb[RX_REORDER_TIMEOUT_VOICE] ||
		    !tb[RX_REORDER_TIMEOUT_VIDEO] ||
		    !tb[RX_REORDER_TIMEOUT_BESTEFFORT] ||
		    !tb[RX_REORDER_TIMEOUT_BACKGROUND]) {
			hdd_err("four AC timeout val are required MAC");
			return -EINVAL;
		}

		reorder_timeout.rx_timeout_pri[0] = nla_get_u32(
			tb[RX_REORDER_TIMEOUT_VOICE]);
		reorder_timeout.rx_timeout_pri[1] = nla_get_u32(
			tb[RX_REORDER_TIMEOUT_VIDEO]);
		reorder_timeout.rx_timeout_pri[2] = nla_get_u32(
			tb[RX_REORDER_TIMEOUT_BESTEFFORT]);
		reorder_timeout.rx_timeout_pri[3] = nla_get_u32(
			tb[RX_REORDER_TIMEOUT_BACKGROUND]);
		/* timeout value is required to be in the rang 10 to 1000ms */
		if (reorder_timeout.rx_timeout_pri[0] >= RX_TIMEOUT_VAL_MIN &&
		    reorder_timeout.rx_timeout_pri[0] <= RX_TIMEOUT_VAL_MAX &&
		    reorder_timeout.rx_timeout_pri[1] >= RX_TIMEOUT_VAL_MIN &&
		    reorder_timeout.rx_timeout_pri[1] <= RX_TIMEOUT_VAL_MAX &&
		    reorder_timeout.rx_timeout_pri[2] >= RX_TIMEOUT_VAL_MIN &&
		    reorder_timeout.rx_timeout_pri[2] <= RX_TIMEOUT_VAL_MAX &&
		    reorder_timeout.rx_timeout_pri[3] >= RX_TIMEOUT_VAL_MIN &&
		    reorder_timeout.rx_timeout_pri[3] <= RX_TIMEOUT_VAL_MAX) {
			qdf_status = sme_set_reorder_timeout(hdd_ctx->hHal,
							     &reorder_timeout);
			if (qdf_status != QDF_STATUS_SUCCESS) {
				hdd_err("failed to set reorder timeout err %d",
					qdf_status);
				ret_val = -EPERM;
			}
		} else {
			hdd_err("one of the timeout value is not in range");
			ret_val = -EINVAL;
		}
	}

	return ret_val;
}

/**
 * wlan_hdd_cfg80211_wifi_set_rx_blocksize - set rx blocksize
 *
 * @hdd_ctx: hdd context
 * @adapter: hdd adapter
 * @tb: array of pointer to struct nlattr
 *
 * Return: 0 on success; error number otherwise
 */
static int wlan_hdd_cfg80211_wifi_set_rx_blocksize(struct hdd_context *hdd_ctx,
						   struct hdd_adapter *adapter,
						   struct nlattr *tb[])
{
	int ret_val = 0;
	uint32_t set_value;
	QDF_STATUS qdf_status;
	struct sir_peer_set_rx_blocksize rx_blocksize;

#define WINDOW_SIZE_VAL_MIN 1
#define WINDOW_SIZE_VAL_MAX 64

	if (tb[RX_BLOCKSIZE_PEER_MAC] ||
	    tb[RX_BLOCKSIZE_WINLIMIT]) {

		/* if one is specified, both must be specified */
		if (!tb[RX_BLOCKSIZE_PEER_MAC] ||
		    !tb[RX_BLOCKSIZE_WINLIMIT]) {
			hdd_err(FL("Both Peer MAC and windows limit required"));
			return -EINVAL;
		}

		memcpy(&rx_blocksize.peer_macaddr,
		       nla_data(tb[RX_BLOCKSIZE_PEER_MAC]),
		       sizeof(rx_blocksize.peer_macaddr)),

		rx_blocksize.vdev_id = adapter->session_id;
		set_value = nla_get_u32(tb[RX_BLOCKSIZE_WINLIMIT]);
		/* maximum window size is 64 */
		if (set_value >= WINDOW_SIZE_VAL_MIN &&
		    set_value <= WINDOW_SIZE_VAL_MAX) {
			rx_blocksize.rx_block_ack_win_limit = set_value;
			qdf_status = sme_set_rx_set_blocksize(hdd_ctx->hHal,
							      &rx_blocksize);
			if (qdf_status != QDF_STATUS_SUCCESS) {
				hdd_err("failed to set aggr sizes err %d",
					qdf_status);
				ret_val = -EPERM;
			}
		} else {
			hdd_err("window size val is not in range");
			ret_val = -EINVAL;
		}
	}

	return ret_val;
}

static int hdd_config_scan_default_ies(struct hdd_adapter *adapter,
				       const struct nlattr *attr)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	uint8_t *scan_ie;
	uint16_t scan_ie_len;
	QDF_STATUS status;

	if (!attr)
		return 0;

	scan_ie_len = nla_len(attr);
	hdd_debug("IE len %d session %d device mode %d",
		  scan_ie_len, adapter->session_id, adapter->device_mode);

	if (!scan_ie_len) {
		hdd_err("zero-length IE prohibited");
		return -EINVAL;
	}

	if (scan_ie_len > MAX_DEFAULT_SCAN_IE_LEN) {
		hdd_err("IE length %d exceeds max of %d",
			scan_ie_len, MAX_DEFAULT_SCAN_IE_LEN);
		return -EINVAL;
	}

	scan_ie = nla_data(attr);
	if (!wlan_is_ie_valid(scan_ie, scan_ie_len)) {
		hdd_err("Invalid default scan IEs");
		return -EINVAL;
	}

	if (wlan_hdd_save_default_scan_ies(hdd_ctx, adapter,
					   scan_ie, scan_ie_len))
		hdd_err("Failed to save default scan IEs");

	if (adapter->device_mode == QDF_STA_MODE) {
		status = sme_set_default_scan_ie(hdd_ctx->hHal,
						 adapter->session_id, scan_ie,
						 scan_ie_len);
		if (QDF_STATUS_SUCCESS != status) {
			hdd_err("failed to set default scan IEs in sme: %d",
				status);
			return -EPERM;
		}
	}

	return 0;
}

/**
 * __wlan_hdd_cfg80211_wifi_configuration_set() - Wifi configuration
 * vendor command
 *
 * @wiphy: wiphy device pointer
 * @wdev: wireless device pointer
 * @data: Vendor command data buffer
 * @data_len: Buffer length
 *
 * Handles QCA_WLAN_VENDOR_ATTR_CONFIG_MAX.
 *
 * Return: Error code.
 */
static int
__wlan_hdd_cfg80211_wifi_configuration_set(struct wiphy *wiphy,
					   struct wireless_dev *wdev,
					   const void *data,
					   int data_len)
{
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx  = wiphy_priv(wiphy);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_CONFIG_MAX + 1];
	const struct nlattr *attr;
	int ret;
	int ret_val = 0;
	u32 modulated_dtim, override_li;
	u16 stats_avg_factor;
	u32 guard_time;
	uint8_t set_value;
	u32 ftm_capab;
	u8 qpower;
	QDF_STATUS status;
	int attr_len;
	int access_policy = 0;
	char vendor_ie[SIR_MAC_MAX_IE_LENGTH + 2];
	bool vendor_ie_present = false, access_policy_present = false;
	struct sir_set_tx_rx_aggregation_size request;
	QDF_STATUS qdf_status;
	uint8_t retry, delay, enable_flag;
	uint32_t abs_delay;
	int param_id;
	uint32_t tx_fail_count;
	uint32_t ant_div_usrcfg;
	uint32_t antdiv_enable, antdiv_chain;
	uint32_t antdiv_selftest, antdiv_selftest_intvl;
	uint8_t bmiss_bcnt;
	uint16_t latency_level;

	ENTER_DEV(dev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	ret_val = wlan_hdd_validate_context(hdd_ctx);
	if (ret_val)
		return ret_val;

	if (wlan_cfg80211_nla_parse(tb, QCA_WLAN_VENDOR_ATTR_CONFIG_MAX, data,
				    data_len, wlan_hdd_wifi_config_policy)) {
		hdd_err("invalid attr");
		return -EINVAL;
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_CONFIG_FINE_TIME_MEASUREMENT]) {
		ftm_capab = nla_get_u32(tb[
			QCA_WLAN_VENDOR_ATTR_CONFIG_FINE_TIME_MEASUREMENT]);
		hdd_ctx->config->fine_time_meas_cap =
			hdd_ctx->fine_time_meas_cap_target & ftm_capab;
		sme_update_fine_time_measurement_capab(hdd_ctx->hHal,
			adapter->session_id,
			hdd_ctx->config->fine_time_meas_cap);
		ucfg_wifi_pos_set_ftm_cap(hdd_ctx->hdd_psoc,
			hdd_ctx->config->fine_time_meas_cap);
		hdd_debug("FTM capability: user value: 0x%x, target value: 0x%x, final value: 0x%x",
			 ftm_capab, hdd_ctx->fine_time_meas_cap_target,
			 hdd_ctx->config->fine_time_meas_cap);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_CONFIG_MODULATED_DTIM]) {
		modulated_dtim = nla_get_u32(
			tb[QCA_WLAN_VENDOR_ATTR_CONFIG_MODULATED_DTIM]);

		status = sme_configure_modulated_dtim(hdd_ctx->hHal,
						      adapter->session_id,
						      modulated_dtim);

		if (QDF_STATUS_SUCCESS != status)
			ret_val = -EPERM;
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_CONFIG_LISTEN_INTERVAL]) {
		override_li = nla_get_u32(
			tb[QCA_WLAN_VENDOR_ATTR_CONFIG_LISTEN_INTERVAL]);

		status = sme_override_listen_interval(hdd_ctx->hHal,
						      adapter->session_id,
						      override_li);

		if (status != QDF_STATUS_SUCCESS)
			ret_val = -EPERM;
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_CONFIG_LRO]) {
		enable_flag = nla_get_u8(tb[QCA_WLAN_VENDOR_ATTR_CONFIG_LRO]);
		ret_val = hdd_lro_set_reset(hdd_ctx, adapter,
							 enable_flag);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_CONFIG_SCAN_ENABLE]) {
		enable_flag =
			nla_get_u8(tb[QCA_WLAN_VENDOR_ATTR_CONFIG_SCAN_ENABLE]);
		sme_set_scan_disable(hdd_ctx->hHal, !enable_flag);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_CONFIG_QPOWER]) {
		qpower = nla_get_u8(
			tb[QCA_WLAN_VENDOR_ATTR_CONFIG_QPOWER]);
		if (hdd_set_qpower_config(hdd_ctx, adapter, qpower) != 0)
			ret_val = -EINVAL;
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_CONFIG_STATS_AVG_FACTOR]) {
		stats_avg_factor = nla_get_u16(
			tb[QCA_WLAN_VENDOR_ATTR_CONFIG_STATS_AVG_FACTOR]);
		status = sme_configure_stats_avg_factor(hdd_ctx->hHal,
							adapter->session_id,
							stats_avg_factor);

		if (QDF_STATUS_SUCCESS != status)
			ret_val = -EPERM;
	}


	if (tb[QCA_WLAN_VENDOR_ATTR_CONFIG_GUARD_TIME]) {
		guard_time = nla_get_u32(
			tb[QCA_WLAN_VENDOR_ATTR_CONFIG_GUARD_TIME]);
		status = sme_configure_guard_time(hdd_ctx->hHal,
						  adapter->session_id,
						  guard_time);

		if (QDF_STATUS_SUCCESS != status)
			ret_val = -EPERM;
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_CONFIG_ACCESS_POLICY_IE_LIST]) {
		qdf_mem_zero(&vendor_ie[0], SIR_MAC_MAX_IE_LENGTH + 2);
		attr_len = nla_len(
			tb[QCA_WLAN_VENDOR_ATTR_CONFIG_ACCESS_POLICY_IE_LIST]);
		if (attr_len < 0 || attr_len > SIR_MAC_MAX_IE_LENGTH + 2) {
			hdd_err("Invalid value. attr_len %d",
				attr_len);
			return -EINVAL;
		}

		nla_memcpy(&vendor_ie,
			tb[QCA_WLAN_VENDOR_ATTR_CONFIG_ACCESS_POLICY_IE_LIST],
			attr_len);
		vendor_ie_present = true;
		hdd_debug("Access policy vendor ie present.attr_len %d",
			attr_len);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_CONFIG_ACCESS_POLICY]) {
		access_policy = (int) nla_get_u32(
		tb[QCA_WLAN_VENDOR_ATTR_CONFIG_ACCESS_POLICY]);
		if ((access_policy < QCA_ACCESS_POLICY_ACCEPT_UNLESS_LISTED) ||
			(access_policy >
				QCA_ACCESS_POLICY_DENY_UNLESS_LISTED)) {
			hdd_err("Invalid value. access_policy %d",
				access_policy);
			return -EINVAL;
		}
		access_policy_present = true;
		hdd_debug("Access policy present. access_policy %d",
			access_policy);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_CONFIG_NON_AGG_RETRY]) {
		retry = nla_get_u8(tb[
				QCA_WLAN_VENDOR_ATTR_CONFIG_NON_AGG_RETRY]);
		retry = retry > CFG_NON_AGG_RETRY_MAX ?
				CFG_NON_AGG_RETRY_MAX : retry;
		param_id = WMI_PDEV_PARAM_NON_AGG_SW_RETRY_TH;
		ret_val = wma_cli_set_command(adapter->session_id, param_id,
					      retry, PDEV_CMD);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_CONFIG_AGG_RETRY]) {
		retry = nla_get_u8(tb[QCA_WLAN_VENDOR_ATTR_CONFIG_AGG_RETRY]);
		retry = retry > CFG_AGG_RETRY_MAX ?
			CFG_AGG_RETRY_MAX : retry;

		/* Value less than CFG_AGG_RETRY_MIN has side effect to t-put */
		retry = ((retry > 0) && (retry < CFG_AGG_RETRY_MIN)) ?
				CFG_AGG_RETRY_MIN : retry;
		param_id = WMI_PDEV_PARAM_AGG_SW_RETRY_TH;
		ret_val = wma_cli_set_command(adapter->session_id, param_id,
					      retry, PDEV_CMD);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_CONFIG_MGMT_RETRY]) {
		retry = nla_get_u8(tb[QCA_WLAN_VENDOR_ATTR_CONFIG_MGMT_RETRY]);
		retry = retry > CFG_MGMT_RETRY_MAX ?
				CFG_MGMT_RETRY_MAX : retry;
		param_id = WMI_PDEV_PARAM_MGMT_RETRY_LIMIT;
		ret_val = wma_cli_set_command(adapter->session_id, param_id,
					      retry, PDEV_CMD);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_CONFIG_CTRL_RETRY]) {
		retry = nla_get_u8(tb[QCA_WLAN_VENDOR_ATTR_CONFIG_CTRL_RETRY]);
		retry = retry > CFG_CTRL_RETRY_MAX ?
				CFG_CTRL_RETRY_MAX : retry;
		param_id = WMI_PDEV_PARAM_CTRL_RETRY_LIMIT;
		ret_val = wma_cli_set_command(adapter->session_id, param_id,
					      retry, PDEV_CMD);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_CONFIG_PROPAGATION_DELAY]) {
		delay = nla_get_u8(tb[
				QCA_WLAN_VENDOR_ATTR_CONFIG_PROPAGATION_DELAY]);
		delay = delay > CFG_PROPAGATION_DELAY_MAX ?
				CFG_PROPAGATION_DELAY_MAX : delay;
		abs_delay = delay + CFG_PROPAGATION_DELAY_BASE;
		param_id = WMI_PDEV_PARAM_PROPAGATION_DELAY;
		ret_val = wma_cli_set_command(adapter->session_id, param_id,
					      abs_delay, PDEV_CMD);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_CONFIG_PROPAGATION_ABS_DELAY]) {
		abs_delay = nla_get_u8(tb[
			QCA_WLAN_VENDOR_ATTR_CONFIG_PROPAGATION_ABS_DELAY]);
		param_id = WMI_PDEV_PARAM_PROPAGATION_DELAY;
		ret_val = wma_cli_set_command(adapter->session_id, param_id,
					      abs_delay, PDEV_CMD);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_CONFIG_TX_FAIL_COUNT]) {
		tx_fail_count = nla_get_u32(
			tb[QCA_WLAN_VENDOR_ATTR_CONFIG_TX_FAIL_COUNT]);
		if (tx_fail_count) {
			status = sme_update_tx_fail_cnt_threshold(hdd_ctx->hHal,
					adapter->session_id, tx_fail_count);
			if (QDF_STATUS_SUCCESS != status) {
				hdd_err("sme_update_tx_fail_cnt_threshold (err=%d)",
					status);
				return -EINVAL;
			}
		}
	}

	if (vendor_ie_present && access_policy_present) {
		if (access_policy == QCA_ACCESS_POLICY_DENY_UNLESS_LISTED) {
			access_policy =
				WLAN_HDD_VENDOR_IE_ACCESS_ALLOW_IF_LISTED;
		} else {
			access_policy = WLAN_HDD_VENDOR_IE_ACCESS_NONE;
		}

		hdd_debug("calling sme_update_access_policy_vendor_ie");
		status = sme_update_access_policy_vendor_ie(hdd_ctx->hHal,
				adapter->session_id, &vendor_ie[0],
				access_policy);
		if (QDF_STATUS_SUCCESS != status) {
			hdd_err("Failed to set vendor ie and access policy.");
			return -EINVAL;
		}
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_CONFIG_CHANNEL_AVOIDANCE_IND]) {
		set_value = nla_get_u8(
			tb[QCA_WLAN_VENDOR_ATTR_CONFIG_CHANNEL_AVOIDANCE_IND]);
		hdd_debug("set_value: %d", set_value);
		ret_val = hdd_enable_disable_ca_event(hdd_ctx, set_value);
	}

	attr = tb[QCA_WLAN_VENDOR_ATTR_CONFIG_SCAN_DEFAULT_IES];
	ret = hdd_config_scan_default_ies(adapter, attr);
	if (ret)
		ret_val = ret;

	if (tb[QCA_WLAN_VENDOR_ATTR_CONFIG_TX_MPDU_AGGREGATION] ||
	    tb[QCA_WLAN_VENDOR_ATTR_CONFIG_RX_MPDU_AGGREGATION]) {
		/* if one is specified, both must be specified */
		if (!tb[QCA_WLAN_VENDOR_ATTR_CONFIG_TX_MPDU_AGGREGATION] ||
		    !tb[QCA_WLAN_VENDOR_ATTR_CONFIG_RX_MPDU_AGGREGATION]) {
			hdd_err("Both TX and RX MPDU Aggregation required");
			return -EINVAL;
		}

		request.tx_aggregation_size = nla_get_u8(
			tb[QCA_WLAN_VENDOR_ATTR_CONFIG_TX_MPDU_AGGREGATION]);
		request.rx_aggregation_size = nla_get_u8(
			tb[QCA_WLAN_VENDOR_ATTR_CONFIG_RX_MPDU_AGGREGATION]);
		request.vdev_id = adapter->session_id;

		if (request.tx_aggregation_size >=
					CFG_TX_AGGREGATION_SIZE_MIN &&
			request.tx_aggregation_size <=
					CFG_TX_AGGREGATION_SIZE_MAX &&
			request.rx_aggregation_size >=
					CFG_RX_AGGREGATION_SIZE_MIN &&
			request.rx_aggregation_size <=
					CFG_RX_AGGREGATION_SIZE_MAX) {
			qdf_status = wma_set_tx_rx_aggregation_size(&request);
			if (qdf_status != QDF_STATUS_SUCCESS) {
				hdd_err("failed to set aggr sizes err %d",
					qdf_status);
				ret_val = -EPERM;
			}
		} else {
			hdd_err("TX %d RX %d MPDU aggr size not in range",
				request.tx_aggregation_size,
				request.rx_aggregation_size);
			ret_val = -EINVAL;
		}
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_CONFIG_IGNORE_ASSOC_DISALLOWED]) {
		uint8_t ignore_assoc_disallowed;

		ignore_assoc_disallowed
			= nla_get_u8(tb[
			QCA_WLAN_VENDOR_ATTR_CONFIG_IGNORE_ASSOC_DISALLOWED]);
		hdd_debug("Set ignore_assoc_disallowed value - %d",
					ignore_assoc_disallowed);
		if ((ignore_assoc_disallowed <
			QCA_IGNORE_ASSOC_DISALLOWED_DISABLE) ||
			(ignore_assoc_disallowed >
				QCA_IGNORE_ASSOC_DISALLOWED_ENABLE))
			return -EPERM;

		sme_update_session_param(hdd_ctx->hHal,
					adapter->session_id,
					SIR_PARAM_IGNORE_ASSOC_DISALLOWED,
					ignore_assoc_disallowed);
	}

#define ANT_DIV_SET_PERIOD(probe_period, stay_period) \
	((1<<26)|((probe_period&0x1fff)<<13)|(stay_period&0x1fff))

#define ANT_DIV_SET_SNR_DIFF(snr_diff) \
	((1<<27)|(snr_diff&0x1fff))

#define ANT_DIV_SET_PROBE_DWELL_TIME(probe_dwell_time) \
	((1<<28)|(probe_dwell_time&0x1fff))

#define ANT_DIV_SET_WEIGHT(mgmt_snr_weight, data_snr_weight, ack_snr_weight) \
	((1<<29)|((mgmt_snr_weight&0xff)<<16)|((data_snr_weight&0xff)<<8)| \
	(ack_snr_weight&0xff))

	if (tb[ANT_DIV_PROBE_PERIOD] ||
	    tb[ANT_DIV_STAY_PERIOD]) {

		if (!tb[ANT_DIV_PROBE_PERIOD] ||
		    !tb[ANT_DIV_STAY_PERIOD]) {
			hdd_err("Both probe and stay period required");
			return -EINVAL;
		}

		ant_div_usrcfg = ANT_DIV_SET_PERIOD(
			nla_get_u32(tb[ANT_DIV_PROBE_PERIOD]),
			nla_get_u32(tb[ANT_DIV_STAY_PERIOD]));
		hdd_debug("ant div set period: %x", ant_div_usrcfg);
		ret_val = wma_cli_set_command((int)adapter->session_id,
					(int)WMI_PDEV_PARAM_ANT_DIV_USRCFG,
					ant_div_usrcfg, PDEV_CMD);
		if (ret_val) {
			hdd_err("Failed to set ant div period");
			return ret_val;
		}
	}

	if (tb[ANT_DIV_SNR_DIFF]) {
		ant_div_usrcfg = ANT_DIV_SET_SNR_DIFF(
			nla_get_u32(tb[ANT_DIV_SNR_DIFF]));
		hdd_debug("ant div set snr diff: %x", ant_div_usrcfg);
		ret_val = wma_cli_set_command((int)adapter->session_id,
					(int)WMI_PDEV_PARAM_ANT_DIV_USRCFG,
					ant_div_usrcfg, PDEV_CMD);
		if (ret_val) {
			hdd_err("Failed to set ant snr diff");
			return ret_val;
		}
	}

	if (tb[ANT_DIV_PROBE_DWELL_TIME]) {
		ant_div_usrcfg = ANT_DIV_SET_PROBE_DWELL_TIME(
			nla_get_u32(tb[ANT_DIV_PROBE_DWELL_TIME]));
		hdd_debug("ant div set probe dewll time: %x",
					ant_div_usrcfg);
		ret_val = wma_cli_set_command((int)adapter->session_id,
					(int)WMI_PDEV_PARAM_ANT_DIV_USRCFG,
					ant_div_usrcfg, PDEV_CMD);
		if (ret_val) {
			hdd_err("Failed to set ant div probe dewll time");
			return ret_val;
		}
	}

	if (tb[ANT_DIV_MGMT_SNR_WEIGHT] ||
	    tb[ANT_DIV_DATA_SNR_WEIGHT] ||
	    tb[ANT_DIV_ACK_SNR_WEIGHT]) {

		if (!tb[ANT_DIV_MGMT_SNR_WEIGHT] ||
		    !tb[ANT_DIV_DATA_SNR_WEIGHT] ||
		    !tb[ANT_DIV_ACK_SNR_WEIGHT]) {
			hdd_err("Mgmt snr, data snr and ack snr weight are required");
			return -EINVAL;
		}

		ant_div_usrcfg = ANT_DIV_SET_WEIGHT(
			nla_get_u32(tb[ANT_DIV_MGMT_SNR_WEIGHT]),
			nla_get_u32(tb[ANT_DIV_DATA_SNR_WEIGHT]),
			nla_get_u32(tb[ANT_DIV_ACK_SNR_WEIGHT]));
		hdd_debug("ant div set weight: %x", ant_div_usrcfg);
		ret_val = wma_cli_set_command((int)adapter->session_id,
					(int)WMI_PDEV_PARAM_ANT_DIV_USRCFG,
					ant_div_usrcfg, PDEV_CMD);
		if (ret_val) {
			hdd_err("Failed to set ant div weight");
			return ret_val;
		}
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_CONFIG_RESTRICT_OFFCHANNEL]) {
		u8 restrict_offchan = nla_get_u8(
			tb[QCA_WLAN_VENDOR_ATTR_CONFIG_RESTRICT_OFFCHANNEL]);

		hdd_debug("Restrict offchannel:%d", restrict_offchan);
		if (restrict_offchan <= 1)
			ret_val =
			wlan_hdd_handle_restrict_offchan_config(adapter,
							restrict_offchan);
		else {
			ret_val = -EINVAL;
			hdd_err("Invalid RESTRICT_OFFCHAN setting");
		}
	}

	ret_val =
		wlan_hdd_cfg80211_wifi_set_reorder_timeout(hdd_ctx, tb);
	if (ret_val != 0)
		return ret_val;

	ret_val =
		wlan_hdd_cfg80211_wifi_set_rx_blocksize(hdd_ctx, adapter, tb);
	if (ret_val != 0)
		return ret_val;

	if (tb[QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_ENA]) {
		antdiv_enable = nla_get_u32(
			tb[QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_ENA]);
		hdd_debug("antdiv_enable: %d", antdiv_enable);
		ret_val = wma_cli_set_command((int)adapter->session_id,
					(int)WMI_PDEV_PARAM_ENA_ANT_DIV,
					antdiv_enable, PDEV_CMD);
		if (ret_val) {
			hdd_err("Failed to set antdiv_enable");
			return ret_val;
		}
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_CHAIN]) {
		antdiv_chain = nla_get_u32(
			tb[QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_CHAIN]);
		hdd_debug("antdiv_chain: %d", antdiv_chain);
		ret_val = wma_cli_set_command((int)adapter->session_id,
					(int)WMI_PDEV_PARAM_FORCE_CHAIN_ANT,
					antdiv_chain, PDEV_CMD);
		if (ret_val) {
			hdd_err("Failed to set antdiv_chain");
			return ret_val;
		}
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_SELFTEST]) {
		antdiv_selftest = nla_get_u32(
			tb[QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_SELFTEST]);
		hdd_debug("antdiv_selftest: %d", antdiv_selftest);
		ret_val = wma_cli_set_command((int)adapter->session_id,
					(int)WMI_PDEV_PARAM_ANT_DIV_SELFTEST,
					antdiv_selftest, PDEV_CMD);
		if (ret_val) {
			hdd_err("Failed to set antdiv_selftest");
			return ret_val;
		}
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_SELFTEST_INTVL]) {
		antdiv_selftest_intvl = nla_get_u32(
			tb[QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_SELFTEST_INTVL]);
		hdd_debug("antdiv_selftest_intvl: %d",
			antdiv_selftest_intvl);
		ret_val = wma_cli_set_command((int)adapter->session_id,
				(int)WMI_PDEV_PARAM_ANT_DIV_SELFTEST_INTVL,
				antdiv_selftest_intvl, PDEV_CMD);
		if (ret_val) {
			hdd_err("Failed to set antdiv_selftest_intvl");
			return ret_val;
		}
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_CONFIG_TOTAL_BEACON_MISS_COUNT]) {
		bmiss_bcnt = nla_get_u8(
		tb[QCA_WLAN_VENDOR_ATTR_CONFIG_TOTAL_BEACON_MISS_COUNT]);
		if (hdd_ctx->config->nRoamBmissFirstBcnt < bmiss_bcnt) {
			hdd_ctx->config->nRoamBmissFinalBcnt = bmiss_bcnt
				- hdd_ctx->config->nRoamBmissFirstBcnt;
			hdd_debug("Bmiss first cnt(%d), Bmiss final cnt(%d)",
				hdd_ctx->config->nRoamBmissFirstBcnt,
				hdd_ctx->config->nRoamBmissFinalBcnt);
			ret_val = sme_set_roam_bmiss_final_bcnt(hdd_ctx->hHal,
				0, hdd_ctx->config->nRoamBmissFinalBcnt);

			if (ret_val) {
				hdd_err("Failed to set bmiss final Bcnt");
				return ret_val;
			}

			ret_val = sme_set_bmiss_bcnt(adapter->session_id,
				hdd_ctx->config->nRoamBmissFirstBcnt,
				hdd_ctx->config->nRoamBmissFinalBcnt);
			if (ret_val) {
				hdd_err("Failed to set bmiss Bcnt");
				return ret_val;
			}
		} else {
			hdd_err("Bcnt(%d) needs to exceed BmissFirstBcnt(%d)",
				bmiss_bcnt,
				hdd_ctx->config->nRoamBmissFirstBcnt);
			return -EINVAL;
		}
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL]) {
		latency_level = nla_get_u16(
			tb[QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL]);

		if ((latency_level >
		    QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL_MAX) ||
		    (latency_level ==
		    QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL_INVALID)) {
			hdd_err("Invalid Wlan latency level value");
			return -EINVAL;
		}

		/* Mapping the latency value to the level which fw expected
		 * 0 - normal, 1 - moderate, 2 - low, 3 - ultralow
		 */
		latency_level = latency_level - 1;
		qdf_status = sme_set_wlm_latency_level(hdd_ctx->hHal,
						       adapter->session_id,
						       latency_level);
		if (qdf_status != QDF_STATUS_SUCCESS) {
			hdd_err("set Wlan latency level failed");
			ret_val = -EINVAL;
		}
	}

	if (adapter->device_mode == QDF_STA_MODE &&
	    tb[QCA_WLAN_VENDOR_ATTR_CONFIG_DISABLE_FILS]) {
		uint8_t disable_fils;

		disable_fils = nla_get_u8(tb[
			QCA_WLAN_VENDOR_ATTR_CONFIG_DISABLE_FILS]);
		hdd_debug("Set disable_fils - %d", disable_fils);

		qdf_status = sme_update_fils_setting(hdd_ctx->hHal,
						     adapter->session_id,
						     disable_fils);
		if (qdf_status != QDF_STATUS_SUCCESS) {
			hdd_err("set disable_fils failed");
			ret_val = -EINVAL;
		}
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_CONFIG_RSN_IE] &&
	    hdd_ctx->config->force_rsne_override) {
		uint8_t force_rsne_override;

		force_rsne_override =
			nla_get_u8(tb[QCA_WLAN_VENDOR_ATTR_CONFIG_RSN_IE]);
		if (force_rsne_override > 1) {
			hdd_err("Invalid test_mode %d", force_rsne_override);
			ret_val = -EINVAL;
		}

		hdd_ctx->force_rsne_override = force_rsne_override;
		hdd_debug("force_rsne_override - %d",
			   hdd_ctx->force_rsne_override);
	}

	return ret_val;
}

/**
 * wlan_hdd_cfg80211_wifi_configuration_set() - Wifi configuration
 * vendor command
 *
 * @wiphy: wiphy device pointer
 * @wdev: wireless device pointer
 * @data: Vendor command data buffer
 * @data_len: Buffer length
 *
 * Handles QCA_WLAN_VENDOR_ATTR_CONFIG_MAX.
 *
 * Return: EOK or other error codes.
 */
static int wlan_hdd_cfg80211_wifi_configuration_set(struct wiphy *wiphy,
						    struct wireless_dev *wdev,
						    const void *data,
						    int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_wifi_configuration_set(wiphy, wdev,
							 data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __wlan_hdd_cfg80211_set_wifi_test_config() - Wifi test configuration
 * vendor command
 *
 * @wiphy: wiphy device pointer
 * @wdev: wireless device pointer
 * @data: Vendor command data buffer
 * @data_len: Buffer length
 *
 * Handles QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_MAX
 *
 * Return: Error code.
 */
static int
__wlan_hdd_cfg80211_set_wifi_test_config(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int data_len) {

	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx  = wiphy_priv(wiphy);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_MAX + 1];
	int ret_val = 0;
	uint8_t cfg_val = 0;
	uint8_t set_val = 0;
	tSmeConfigParams *sme_config;
	bool update_sme_cfg = false;
	uint8_t tid = 0, ac;
	uint16_t buff_size = 0;

	ENTER_DEV(dev);

	sme_config = qdf_mem_malloc(sizeof(*sme_config));
	if (!sme_config) {
		hdd_err("mem alloc failed for sme_config");
		return -ENOMEM;
	}
	sme_get_config_param(hdd_ctx->hHal, sme_config);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		ret_val = -EPERM;
		goto send_err;
	}

	ret_val = wlan_hdd_validate_context(hdd_ctx);
	if (ret_val)
		goto send_err;

	if (hdd_ctx->driver_status == DRIVER_MODULES_CLOSED) {
		hdd_err("Driver Modules are closed, can not start logger");
		ret_val = -EINVAL;
		goto send_err;
	}

	if (wlan_cfg80211_nla_parse(tb,
			QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_MAX,
			data, data_len, wlan_hdd_wifi_test_config_policy)) {
		hdd_err("invalid attr");
		ret_val = -EINVAL;
		goto send_err;
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_ACCEPT_ADDBA_REQ]) {
		cfg_val = nla_get_u8(tb[
			QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_ACCEPT_ADDBA_REQ]
			);
		hdd_debug("set addba accept req from peer value %d", cfg_val);
		ret_val = sme_set_addba_accept(hdd_ctx->hHal,
				adapter->session_id, cfg_val);
		if (ret_val)
			goto send_err;
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_MCS]) {
		cfg_val = nla_get_u8(tb[
			QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_MCS]);
		hdd_debug("set HE MCS value 0x%0X", cfg_val);
		ret_val = sme_update_he_mcs(hdd_ctx->hHal, adapter->session_id,
					    cfg_val);
		if (ret_val)
			goto send_err;
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_WMM_ENABLE]) {
		cfg_val = nla_get_u8(tb[
			QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_WMM_ENABLE]);
		if (!cfg_val) {
			sme_config->csrConfig.WMMSupportMode =
				hdd_to_csr_wmm_mode(HDD_WMM_USER_MODE_NO_QOS);
			hdd_debug("wmm is disabled");
		} else {
			sme_config->csrConfig.WMMSupportMode =
				hdd_to_csr_wmm_mode(hdd_ctx->config->WmmMode);
			hdd_debug("using wmm default value");
		}
		update_sme_cfg = true;
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_SEND_ADDBA_REQ]) {
		cfg_val = nla_get_u8(tb[
			QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_SEND_ADDBA_REQ]);
		if (cfg_val) {
			/*Auto BA mode*/
			set_val = 0;
			hdd_debug("BA operating mode is set to auto");
		} else {
			/*Manual BA mode*/
			set_val = 1;
			hdd_debug("BA operating mode is set to Manual");
		}

		ret_val = wma_cli_set_command(adapter->session_id,
				WMI_VDEV_PARAM_BA_MODE, set_val, VDEV_CMD);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_FRAGMENTATION]) {
		cfg_val = nla_get_u8(tb[
			QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_FRAGMENTATION]
			);
		if (cfg_val > HE_FRAG_LEVEL1)
			set_val = HE_FRAG_LEVEL1;
		else
			set_val = cfg_val;

		hdd_debug("set HE fragmention to %d", set_val);
		ret_val = sme_update_he_frag_supp(hdd_ctx->hHal,
				adapter->session_id, set_val);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_WEP_TKIP_IN_HE]) {
		cfg_val = nla_get_u8(tb[
			QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_WEP_TKIP_IN_HE]);
		sme_config->csrConfig.wep_tkip_in_he = cfg_val;
		hdd_debug("Set WEP/TKIP allow in HE %d", cfg_val);

		update_sme_cfg = true;
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_ADD_DEL_BA_SESSION]) {
		if (tb[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_BA_TID]) {
			tid = nla_get_u8(tb[
				QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_BA_TID]);
		} else {
			hdd_err("TID is not set for ADD/DEL BA cfg");
			ret_val = -EINVAL;
			goto send_err;
		}
		cfg_val = nla_get_u8(tb[
		QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_ADD_DEL_BA_SESSION]);
		if (cfg_val == QCA_WLAN_ADD_BA) {
			if (tb[
			QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_ADDBA_BUFF_SIZE])
				buff_size = nla_get_u8(tb[
				QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_ADDBA_BUFF_SIZE]);
			ret_val = sme_send_addba_req(hdd_ctx->hHal,
					adapter->session_id, tid, buff_size);
		} else if (cfg_val == QCA_WLAN_DELETE_BA) {
		} else {
			hdd_err("Invalid BA session cfg");
			ret_val = -EINVAL;
			goto send_err;
		}
	} else if (tb[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_ADDBA_BUFF_SIZE]) {
		buff_size = nla_get_u8(tb[
		QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_ADDBA_BUFF_SIZE]);
		hdd_debug("set buff size to %d for all tids", buff_size);
		ret_val = sme_set_ba_buff_size(hdd_ctx->hHal,
				adapter->session_id, buff_size);
		if (ret_val)
			goto send_err;
		if (buff_size > 64)
			/* Configure ADDBA req buffer size to 256 */
			set_val = 3;
		else
			/* Configure ADDBA req buffer size to 64 */
			set_val = 2;
		ret_val = wma_cli_set_command(adapter->session_id,
				WMI_VDEV_PARAM_BA_MODE, set_val, VDEV_CMD);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_ENABLE_NO_ACK]) {
		if (tb[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_NO_ACK_AC]) {
			ac = nla_get_u8(tb[
			     QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_NO_ACK_AC]);
		} else {
			hdd_err("AC is not set for NO ACK policy config");
			ret_val = -EINVAL;
			goto send_err;
		}
		cfg_val = nla_get_u8(tb[
			QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_ENABLE_NO_ACK]);
		hdd_debug("Set NO_ACK to %d for ac %d", cfg_val, ac);
		ret_val = sme_set_no_ack_policy(hdd_ctx->hHal,
				adapter->session_id, cfg_val, ac);
	}

	if (update_sme_cfg)
		sme_update_config(hdd_ctx->hHal, sme_config);

send_err:
	qdf_mem_free(sme_config);

	return ret_val;
}

/**
 * wlan_hdd_cfg80211_set_wifi_test_config() - Wifi test configuration
 * vendor command
 *
 * @wiphy: wiphy device pointer
 * @wdev: wireless device pointer
 * @data: Vendor command data buffer
 * @data_len: Buffer length
 *
 * Handles QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_MAX
 *
 * Return: EOK or other error codes.
 */
static int wlan_hdd_cfg80211_set_wifi_test_config(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_set_wifi_test_config(wiphy, wdev,
			data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

static const struct
nla_policy
qca_wlan_vendor_wifi_logger_start_policy
[QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_START_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_RING_ID]
		= {.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_VERBOSE_LEVEL]
		= {.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_FLAGS]
		= {.type = NLA_U32 },
};

/**
 * __wlan_hdd_cfg80211_wifi_logger_start() - This function is used to enable
 * or disable the collection of packet statistics from the firmware
 * @wiphy:    WIPHY structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of the data received
 *
 * This function enables or disables the collection of packet statistics from
 * the firmware
 *
 * Return: 0 on success and errno on failure
 */
static int __wlan_hdd_cfg80211_wifi_logger_start(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data,
		int data_len)
{
	QDF_STATUS status;
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_START_MAX + 1];
	struct sir_wifi_start_log start_log;

	ENTER_DEV(wdev->netdev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	status = wlan_hdd_validate_context(hdd_ctx);
	if (status)
		return status;

	if (hdd_ctx->driver_status == DRIVER_MODULES_CLOSED) {
		hdd_err("Driver Modules are closed, can not start logger");
		return -EINVAL;
	}

	if (wlan_cfg80211_nla_parse(tb,
				    QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_START_MAX,
				    data, data_len,
				    qca_wlan_vendor_wifi_logger_start_policy)) {
		hdd_err("Invalid attribute");
		return -EINVAL;
	}

	/* Parse and fetch ring id */
	if (!tb[QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_RING_ID]) {
		hdd_err("attr ATTR failed");
		return -EINVAL;
	}
	start_log.ring_id = nla_get_u32(
			tb[QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_RING_ID]);
	hdd_debug("Ring ID=%d", start_log.ring_id);

	/* Parse and fetch verbose level */
	if (!tb[QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_VERBOSE_LEVEL]) {
		hdd_err("attr verbose_level failed");
		return -EINVAL;
	}
	start_log.verbose_level = nla_get_u32(
			tb[QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_VERBOSE_LEVEL]);
	hdd_debug("verbose_level=%d", start_log.verbose_level);

	/* Parse and fetch flag */
	if (!tb[QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_FLAGS]) {
		hdd_err("attr flag failed");
		return -EINVAL;
	}
	start_log.is_iwpriv_command = nla_get_u32(
			tb[QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_FLAGS]);

	/* size is buff size which can be set using iwpriv command*/
	start_log.size = 0;
	start_log.is_pktlog_buff_clear = false;

	cds_set_ring_log_level(start_log.ring_id, start_log.verbose_level);

	if (start_log.ring_id == RING_ID_WAKELOCK) {
		/* Start/stop wakelock events */
		if (start_log.verbose_level > WLAN_LOG_LEVEL_OFF)
			cds_set_wakelock_logging(true);
		else
			cds_set_wakelock_logging(false);
		return 0;
	}

	status = sme_wifi_start_logger(hdd_ctx->hHal, start_log);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("sme_wifi_start_logger failed(err=%d)",
				status);
		return -EINVAL;
	}
	return 0;
}

/**
 * wlan_hdd_cfg80211_wifi_logger_start() - Wrapper function used to enable
 * or disable the collection of packet statistics from the firmware
 * @wiphy:    WIPHY structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of the data received
 *
 * This function is used to enable or disable the collection of packet
 * statistics from the firmware
 *
 * Return: 0 on success and errno on failure
 */
static int wlan_hdd_cfg80211_wifi_logger_start(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data,
		int data_len)
{
	int ret = 0;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_wifi_logger_start(wiphy,
			wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

static const struct
nla_policy
qca_wlan_vendor_wifi_logger_get_ring_data_policy
[QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_GET_RING_DATA_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_GET_RING_DATA_ID]
		= {.type = NLA_U32 },
};

/**
 * __wlan_hdd_cfg80211_wifi_logger_get_ring_data() - Flush per packet stats
 * @wiphy:    WIPHY structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of the data received
 *
 * This function is used to flush or retrieve the per packet statistics from
 * the driver
 *
 * Return: 0 on success and errno on failure
 */
static int __wlan_hdd_cfg80211_wifi_logger_get_ring_data(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data,
		int data_len)
{
	QDF_STATUS status;
	uint32_t ring_id;
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct nlattr *tb
		[QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_GET_RING_DATA_MAX + 1];

	ENTER_DEV(wdev->netdev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	status = wlan_hdd_validate_context(hdd_ctx);
	if (status)
		return status;

	if (wlan_cfg80211_nla_parse(tb,
			    QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_GET_RING_DATA_MAX,
			    data, data_len,
			    qca_wlan_vendor_wifi_logger_get_ring_data_policy)) {
		hdd_err("Invalid attribute");
		return -EINVAL;
	}

	/* Parse and fetch ring id */
	if (!tb[QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_GET_RING_DATA_ID]) {
		hdd_err("attr ATTR failed");
		return -EINVAL;
	}

	ring_id = nla_get_u32(
			tb[QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_GET_RING_DATA_ID]);

	if (ring_id == RING_ID_PER_PACKET_STATS) {
		wlan_logging_set_per_pkt_stats();
		hdd_debug("Flushing/Retrieving packet stats");
	} else if (ring_id == RING_ID_DRIVER_DEBUG) {
		/*
		 * As part of DRIVER ring ID, flush both driver and fw logs.
		 * For other Ring ID's driver doesn't have any rings to flush
		 */
		hdd_notice("Bug report triggered by framework");

		status = cds_flush_logs(WLAN_LOG_TYPE_NON_FATAL,
				WLAN_LOG_INDICATOR_FRAMEWORK,
				WLAN_LOG_REASON_CODE_UNUSED,
				true, false);
		if (QDF_STATUS_SUCCESS != status) {
			hdd_err("Failed to trigger bug report");
			return -EINVAL;
		}
	} else {
		wlan_report_log_completion(WLAN_LOG_TYPE_NON_FATAL,
					   WLAN_LOG_INDICATOR_FRAMEWORK,
					   WLAN_LOG_REASON_CODE_UNUSED,
					   ring_id);
	}
	return 0;
}

/**
 * wlan_hdd_cfg80211_wifi_logger_get_ring_data() - Wrapper to flush packet stats
 * @wiphy:    WIPHY structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of the data received
 *
 * This function is used to flush or retrieve the per packet statistics from
 * the driver
 *
 * Return: 0 on success and errno on failure
 */
static int wlan_hdd_cfg80211_wifi_logger_get_ring_data(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data,
		int data_len)
{
	int ret = 0;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_wifi_logger_get_ring_data(wiphy,
			wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

#ifdef WLAN_FEATURE_OFFLOAD_PACKETS
/**
 * hdd_map_req_id_to_pattern_id() - map request id to pattern id
 * @hdd_ctx: HDD context
 * @request_id: [input] request id
 * @pattern_id: [output] pattern id
 *
 * This function loops through request id to pattern id array
 * if the slot is available, store the request id and return pattern id
 * if entry exists, return the pattern id
 *
 * Return: 0 on success and errno on failure
 */
static int hdd_map_req_id_to_pattern_id(struct hdd_context *hdd_ctx,
					  uint32_t request_id,
					  uint8_t *pattern_id)
{
	uint32_t i;

	mutex_lock(&hdd_ctx->op_ctx.op_lock);
	for (i = 0; i < MAXNUM_PERIODIC_TX_PTRNS; i++) {
		if (hdd_ctx->op_ctx.op_table[i].request_id == MAX_REQUEST_ID) {
			hdd_ctx->op_ctx.op_table[i].request_id = request_id;
			*pattern_id = hdd_ctx->op_ctx.op_table[i].pattern_id;
			mutex_unlock(&hdd_ctx->op_ctx.op_lock);
			return 0;
		} else if (hdd_ctx->op_ctx.op_table[i].request_id ==
					request_id) {
			*pattern_id = hdd_ctx->op_ctx.op_table[i].pattern_id;
			mutex_unlock(&hdd_ctx->op_ctx.op_lock);
			return 0;
		}
	}
	mutex_unlock(&hdd_ctx->op_ctx.op_lock);
	return -EINVAL;
}

/**
 * hdd_unmap_req_id_to_pattern_id() - unmap request id to pattern id
 * @hdd_ctx: HDD context
 * @request_id: [input] request id
 * @pattern_id: [output] pattern id
 *
 * This function loops through request id to pattern id array
 * reset request id to 0 (slot available again) and
 * return pattern id
 *
 * Return: 0 on success and errno on failure
 */
static int hdd_unmap_req_id_to_pattern_id(struct hdd_context *hdd_ctx,
					  uint32_t request_id,
					  uint8_t *pattern_id)
{
	uint32_t i;

	mutex_lock(&hdd_ctx->op_ctx.op_lock);
	for (i = 0; i < MAXNUM_PERIODIC_TX_PTRNS; i++) {
		if (hdd_ctx->op_ctx.op_table[i].request_id == request_id) {
			hdd_ctx->op_ctx.op_table[i].request_id = MAX_REQUEST_ID;
			*pattern_id = hdd_ctx->op_ctx.op_table[i].pattern_id;
			mutex_unlock(&hdd_ctx->op_ctx.op_lock);
			return 0;
		}
	}
	mutex_unlock(&hdd_ctx->op_ctx.op_lock);
	return -EINVAL;
}


/*
 * define short names for the global vendor params
 * used by __wlan_hdd_cfg80211_offloaded_packets()
 */
#define PARAM_MAX QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_MAX
#define PARAM_REQUEST_ID \
		QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_REQUEST_ID
#define PARAM_CONTROL \
		QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_SENDING_CONTROL
#define PARAM_IP_PACKET \
		QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_IP_PACKET_DATA
#define PARAM_SRC_MAC_ADDR \
		QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_SRC_MAC_ADDR
#define PARAM_DST_MAC_ADDR \
		QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_DST_MAC_ADDR
#define PARAM_PERIOD QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_PERIOD

/**
 * wlan_hdd_add_tx_ptrn() - add tx pattern
 * @adapter: adapter pointer
 * @hdd_ctx: hdd context
 * @tb: nl attributes
 *
 * This function reads the NL attributes and forms a AddTxPtrn message
 * posts it to SME.
 *
 */
static int
wlan_hdd_add_tx_ptrn(struct hdd_adapter *adapter, struct hdd_context *hdd_ctx,
			struct nlattr **tb)
{
	struct sSirAddPeriodicTxPtrn *add_req;
	QDF_STATUS status;
	uint32_t request_id, ret, len;
	uint8_t pattern_id = 0;
	struct qdf_mac_addr dst_addr;
	uint16_t eth_type = htons(ETH_P_IP);

	if (!hdd_conn_is_connected(WLAN_HDD_GET_STATION_CTX_PTR(adapter))) {
		hdd_err("Not in Connected state!");
		return -ENOTSUPP;
	}

	add_req = qdf_mem_malloc(sizeof(*add_req));
	if (!add_req) {
		hdd_err("memory allocation failed");
		return -ENOMEM;
	}

	/* Parse and fetch request Id */
	if (!tb[PARAM_REQUEST_ID]) {
		hdd_err("attr request id failed");
		goto fail;
	}

	request_id = nla_get_u32(tb[PARAM_REQUEST_ID]);
	if (request_id == MAX_REQUEST_ID) {
		hdd_err("request_id cannot be MAX");
		goto fail;
	}
	hdd_debug("Request Id: %u", request_id);

	if (!tb[PARAM_PERIOD]) {
		hdd_err("attr period failed");
		goto fail;
	}
	add_req->usPtrnIntervalMs = nla_get_u32(tb[PARAM_PERIOD]);
	hdd_debug("Period: %u ms", add_req->usPtrnIntervalMs);
	if (add_req->usPtrnIntervalMs == 0) {
		hdd_err("Invalid interval zero, return failure");
		goto fail;
	}

	if (!tb[PARAM_SRC_MAC_ADDR]) {
		hdd_err("attr source mac address failed");
		goto fail;
	}
	nla_memcpy(add_req->mac_address.bytes, tb[PARAM_SRC_MAC_ADDR],
			QDF_MAC_ADDR_SIZE);
	hdd_debug("input src mac address: "MAC_ADDRESS_STR,
			MAC_ADDR_ARRAY(add_req->mac_address.bytes));

	if (!qdf_is_macaddr_equal(&add_req->mac_address,
				  &adapter->mac_addr)) {
		hdd_err("input src mac address and connected ap bssid are different");
		goto fail;
	}

	if (!tb[PARAM_DST_MAC_ADDR]) {
		hdd_err("attr dst mac address failed");
		goto fail;
	}
	nla_memcpy(dst_addr.bytes, tb[PARAM_DST_MAC_ADDR], QDF_MAC_ADDR_SIZE);
	hdd_debug("input dst mac address: "MAC_ADDRESS_STR,
			MAC_ADDR_ARRAY(dst_addr.bytes));

	if (!tb[PARAM_IP_PACKET]) {
		hdd_err("attr ip packet failed");
		goto fail;
	}
	add_req->ucPtrnSize = nla_len(tb[PARAM_IP_PACKET]);
	hdd_debug("IP packet len: %u", add_req->ucPtrnSize);

	if (add_req->ucPtrnSize < 0 ||
		add_req->ucPtrnSize > (PERIODIC_TX_PTRN_MAX_SIZE -
					ETH_HLEN)) {
		hdd_err("Invalid IP packet len: %d",
				add_req->ucPtrnSize);
		goto fail;
	}

	len = 0;
	qdf_mem_copy(&add_req->ucPattern[0], dst_addr.bytes, QDF_MAC_ADDR_SIZE);
	len += QDF_MAC_ADDR_SIZE;
	qdf_mem_copy(&add_req->ucPattern[len], add_req->mac_address.bytes,
			QDF_MAC_ADDR_SIZE);
	len += QDF_MAC_ADDR_SIZE;
	qdf_mem_copy(&add_req->ucPattern[len], &eth_type, 2);
	len += 2;

	/*
	 * This is the IP packet, add 14 bytes Ethernet (802.3) header
	 * ------------------------------------------------------------
	 * | 14 bytes Ethernet (802.3) header | IP header and payload |
	 * ------------------------------------------------------------
	 */
	qdf_mem_copy(&add_req->ucPattern[len],
			nla_data(tb[PARAM_IP_PACKET]),
			add_req->ucPtrnSize);
	add_req->ucPtrnSize += len;

	ret = hdd_map_req_id_to_pattern_id(hdd_ctx, request_id, &pattern_id);
	if (ret) {
		hdd_err("req id to pattern id failed (ret=%d)", ret);
		goto fail;
	}
	add_req->ucPtrnId = pattern_id;
	hdd_debug("pattern id: %d", add_req->ucPtrnId);

	status = sme_add_periodic_tx_ptrn(hdd_ctx->hHal, add_req);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("sme_add_periodic_tx_ptrn failed (err=%d)", status);
		goto fail;
	}

	EXIT();
	qdf_mem_free(add_req);
	return 0;

fail:
	qdf_mem_free(add_req);
	return -EINVAL;
}

/**
 * wlan_hdd_del_tx_ptrn() - delete tx pattern
 * @adapter: adapter pointer
 * @hdd_ctx: hdd context
 * @tb: nl attributes
 *
 * This function reads the NL attributes and forms a DelTxPtrn message
 * posts it to SME.
 *
 */
static int
wlan_hdd_del_tx_ptrn(struct hdd_adapter *adapter, struct hdd_context *hdd_ctx,
			struct nlattr **tb)
{
	struct sSirDelPeriodicTxPtrn *del_req;
	QDF_STATUS status;
	uint32_t request_id, ret;
	uint8_t pattern_id = 0;

	/* Parse and fetch request Id */
	if (!tb[PARAM_REQUEST_ID]) {
		hdd_err("attr request id failed");
		return -EINVAL;
	}
	request_id = nla_get_u32(tb[PARAM_REQUEST_ID]);
	if (request_id == MAX_REQUEST_ID) {
		hdd_err("request_id cannot be MAX");
		return -EINVAL;
	}

	ret = hdd_unmap_req_id_to_pattern_id(hdd_ctx, request_id, &pattern_id);
	if (ret) {
		hdd_err("req id to pattern id failed (ret=%d)", ret);
		return -EINVAL;
	}

	del_req = qdf_mem_malloc(sizeof(*del_req));
	if (!del_req) {
		hdd_err("memory allocation failed");
		return -ENOMEM;
	}

	qdf_copy_macaddr(&del_req->mac_address, &adapter->mac_addr);
	hdd_debug(MAC_ADDRESS_STR, MAC_ADDR_ARRAY(del_req->mac_address.bytes));
	del_req->ucPtrnId = pattern_id;
	hdd_debug("Request Id: %u Pattern id: %d",
			 request_id, del_req->ucPtrnId);

	status = sme_del_periodic_tx_ptrn(hdd_ctx->hHal, del_req);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("sme_del_periodic_tx_ptrn failed (err=%d)", status);
		goto fail;
	}

	EXIT();
	qdf_mem_free(del_req);
	return 0;

fail:
	qdf_mem_free(del_req);
	return -EINVAL;
}


/**
 * __wlan_hdd_cfg80211_offloaded_packets() - send offloaded packets
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: 0 on success, negative errno on failure
 */
static int
__wlan_hdd_cfg80211_offloaded_packets(struct wiphy *wiphy,
				     struct wireless_dev *wdev,
				     const void *data,
				     int data_len)
{
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct nlattr *tb[PARAM_MAX + 1];
	uint8_t control;
	int ret;
	static const struct nla_policy policy[PARAM_MAX + 1] = {
			[PARAM_REQUEST_ID] = { .type = NLA_U32 },
			[PARAM_CONTROL] = { .type = NLA_U32 },
			[PARAM_SRC_MAC_ADDR] = { .type = NLA_BINARY,
						.len = QDF_MAC_ADDR_SIZE },
			[PARAM_DST_MAC_ADDR] = { .type = NLA_BINARY,
						.len = QDF_MAC_ADDR_SIZE },
			[PARAM_PERIOD] = { .type = NLA_U32 },
	};

	ENTER_DEV(dev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	if (!sme_is_feature_supported_by_fw(WLAN_PERIODIC_TX_PTRN)) {
		hdd_err("Periodic Tx Pattern Offload feature is not supported in FW!");
		return -ENOTSUPP;
	}

	if (wlan_cfg80211_nla_parse(tb, PARAM_MAX, data, data_len, policy)) {
		hdd_err("Invalid ATTR");
		return -EINVAL;
	}

	if (!tb[PARAM_CONTROL]) {
		hdd_err("attr control failed");
		return -EINVAL;
	}
	control = nla_get_u32(tb[PARAM_CONTROL]);
	hdd_debug("Control: %d", control);

	if (control == WLAN_START_OFFLOADED_PACKETS)
		return wlan_hdd_add_tx_ptrn(adapter, hdd_ctx, tb);
	if (control == WLAN_STOP_OFFLOADED_PACKETS)
		return wlan_hdd_del_tx_ptrn(adapter, hdd_ctx, tb);

	hdd_err("Invalid control: %d", control);
	return -EINVAL;
}

/*
 * done with short names for the global vendor params
 * used by __wlan_hdd_cfg80211_offloaded_packets()
 */
#undef PARAM_MAX
#undef PARAM_REQUEST_ID
#undef PARAM_CONTROL
#undef PARAM_IP_PACKET
#undef PARAM_SRC_MAC_ADDR
#undef PARAM_DST_MAC_ADDR
#undef PARAM_PERIOD

/**
 * wlan_hdd_cfg80211_offloaded_packets() - Wrapper to offload packets
 * @wiphy:    wiphy structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of @data
 *
 * Return: 0 on success; errno on failure
 */
static int wlan_hdd_cfg80211_offloaded_packets(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len)
{
	int ret = 0;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_offloaded_packets(wiphy,
					wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}
#endif

/*
 * define short names for the global vendor params
 * used by __wlan_hdd_cfg80211_monitor_rssi()
 */
#define PARAM_MAX QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_MAX
#define PARAM_REQUEST_ID QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_REQUEST_ID
#define PARAM_CONTROL QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_CONTROL
#define PARAM_MIN_RSSI QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_MIN_RSSI
#define PARAM_MAX_RSSI QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_MAX_RSSI

/**
 * __wlan_hdd_cfg80211_monitor_rssi() - monitor rssi
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: 0 on success, negative errno on failure
 */
static int
__wlan_hdd_cfg80211_monitor_rssi(struct wiphy *wiphy,
				 struct wireless_dev *wdev,
				 const void *data,
				 int data_len)
{
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct nlattr *tb[PARAM_MAX + 1];
	struct rssi_monitor_req req;
	QDF_STATUS status;
	int ret;
	uint32_t control;
	static const struct nla_policy policy[PARAM_MAX + 1] = {
			[PARAM_REQUEST_ID] = { .type = NLA_U32 },
			[PARAM_CONTROL] = { .type = NLA_U32 },
			[PARAM_MIN_RSSI] = { .type = NLA_S8 },
			[PARAM_MAX_RSSI] = { .type = NLA_S8 },
	};

	ENTER_DEV(dev);

	if (wlan_hdd_validate_session_id(adapter->session_id)) {
		hdd_err("invalid session id: %d", adapter->session_id);
		return -EINVAL;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	if (!hdd_conn_is_connected(WLAN_HDD_GET_STATION_CTX_PTR(adapter))) {
		hdd_err("Not in Connected state!");
		return -ENOTSUPP;
	}

	if (wlan_cfg80211_nla_parse(tb, PARAM_MAX, data, data_len, policy)) {
		hdd_err("Invalid ATTR");
		return -EINVAL;
	}

	if (!tb[PARAM_REQUEST_ID]) {
		hdd_err("attr request id failed");
		return -EINVAL;
	}

	if (!tb[PARAM_CONTROL]) {
		hdd_err("attr control failed");
		return -EINVAL;
	}

	req.request_id = nla_get_u32(tb[PARAM_REQUEST_ID]);
	req.session_id = adapter->session_id;
	control = nla_get_u32(tb[PARAM_CONTROL]);

	if (control == QCA_WLAN_RSSI_MONITORING_START) {
		req.control = true;
		if (!tb[PARAM_MIN_RSSI]) {
			hdd_err("attr min rssi failed");
			return -EINVAL;
		}

		if (!tb[PARAM_MAX_RSSI]) {
			hdd_err("attr max rssi failed");
			return -EINVAL;
		}

		req.min_rssi = nla_get_s8(tb[PARAM_MIN_RSSI]);
		req.max_rssi = nla_get_s8(tb[PARAM_MAX_RSSI]);

		if (!(req.min_rssi < req.max_rssi)) {
			hdd_warn("min_rssi: %d must be less than max_rssi: %d",
					req.min_rssi, req.max_rssi);
			return -EINVAL;
		}
		hdd_debug("Min_rssi: %d Max_rssi: %d",
			req.min_rssi, req.max_rssi);

	} else if (control == QCA_WLAN_RSSI_MONITORING_STOP)
		req.control = false;
	else {
		hdd_err("Invalid control cmd: %d", control);
		return -EINVAL;
	}
	hdd_debug("Request Id: %u Session_id: %d Control: %d",
			req.request_id, req.session_id, req.control);

	status = sme_set_rssi_monitoring(hdd_ctx->hHal, &req);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("sme_set_rssi_monitoring failed(err=%d)", status);
		return -EINVAL;
	}

	return 0;
}

/*
 * done with short names for the global vendor params
 * used by __wlan_hdd_cfg80211_monitor_rssi()
 */
#undef PARAM_MAX
#undef PARAM_CONTROL
#undef PARAM_REQUEST_ID
#undef PARAM_MAX_RSSI
#undef PARAM_MIN_RSSI

/**
 * wlan_hdd_cfg80211_monitor_rssi() - SSR wrapper to rssi monitoring
 * @wiphy:    wiphy structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of @data
 *
 * Return: 0 on success; errno on failure
 */
static int
wlan_hdd_cfg80211_monitor_rssi(struct wiphy *wiphy, struct wireless_dev *wdev,
			       const void *data, int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_monitor_rssi(wiphy, wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * hdd_rssi_threshold_breached() - rssi breached NL event
 * @hddctx: HDD context
 * @data: rssi breached event data
 *
 * This function reads the rssi breached event %data and fill in the skb with
 * NL attributes and send up the NL event.
 *
 * Return: none
 */
void hdd_rssi_threshold_breached(void *hddctx,
				 struct rssi_breach_event *data)
{
	struct hdd_context *hdd_ctx  = hddctx;
	struct sk_buff *skb;

	ENTER();

	if (wlan_hdd_validate_context(hdd_ctx))
		return;
	if (!data) {
		hdd_err("data is null");
		return;
	}

	skb = cfg80211_vendor_event_alloc(hdd_ctx->wiphy,
				  NULL,
				  EXTSCAN_EVENT_BUF_SIZE + NLMSG_HDRLEN,
				  QCA_NL80211_VENDOR_SUBCMD_MONITOR_RSSI_INDEX,
				  GFP_KERNEL);

	if (!skb) {
		hdd_err("cfg80211_vendor_event_alloc failed");
		return;
	}

	hdd_debug("Req Id: %u Current rssi: %d",
			data->request_id, data->curr_rssi);
	hdd_debug("Current BSSID: "MAC_ADDRESS_STR,
			MAC_ADDR_ARRAY(data->curr_bssid.bytes));

	if (nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_REQUEST_ID,
		data->request_id) ||
	    nla_put(skb, QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_CUR_BSSID,
		sizeof(data->curr_bssid), data->curr_bssid.bytes) ||
	    nla_put_s8(skb, QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_CUR_RSSI,
		data->curr_rssi)) {
		hdd_err("nla put fail");
		goto fail;
	}

	cfg80211_vendor_event(skb, GFP_KERNEL);
	return;

fail:
	kfree_skb(skb);
}

static const struct nla_policy
ns_offload_set_policy[QCA_WLAN_VENDOR_ATTR_ND_OFFLOAD_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_ND_OFFLOAD_FLAG] = {.type = NLA_U8},
};

/**
 * __wlan_hdd_cfg80211_set_ns_offload() - enable/disable NS offload
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Length of @data
 *
 * Return: 0 on success, negative errno on failure
 */
static int
__wlan_hdd_cfg80211_set_ns_offload(struct wiphy *wiphy,
			struct wireless_dev *wdev,
			const void *data, int data_len)
{
	int status;
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_ND_OFFLOAD_MAX + 1];
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter =  WLAN_HDD_GET_PRIV_PTR(dev);

	ENTER_DEV(wdev->netdev);

	status = wlan_hdd_validate_context(hdd_ctx);
	if (0 != status)
		return status;
	if (!hdd_ctx->config->fhostNSOffload) {
		hdd_err("ND Offload not supported");
		return -EINVAL;
	}

	if (!hdd_ctx->config->active_mode_offload) {
		hdd_warn("Active mode offload is disabled");
		return -EINVAL;
	}

	if (wlan_cfg80211_nla_parse(tb, QCA_WLAN_VENDOR_ATTR_ND_OFFLOAD_MAX,
				    (struct nlattr *)data, data_len,
				    ns_offload_set_policy)) {
		hdd_err("nla_parse failed");
		return -EINVAL;
	}

	if (!tb[QCA_WLAN_VENDOR_ATTR_ND_OFFLOAD_FLAG]) {
		hdd_err("ND Offload flag attribute not present");
		return -EINVAL;
	}

	hdd_ctx->ns_offload_enable =
		nla_get_u8(tb[QCA_WLAN_VENDOR_ATTR_ND_OFFLOAD_FLAG]);

	if (QDF_IBSS_MODE == adapter->device_mode) {
		hdd_debug("NS Offload is not supported in IBSS mode");
		return -EINVAL;
	}

	/* update ns offload in case it is already enabled/disabled */
	if (hdd_ctx->ns_offload_enable)
		hdd_enable_ns_offload(adapter, pmo_ns_offload_dynamic_update);
	else
		hdd_disable_ns_offload(adapter, pmo_ns_offload_dynamic_update);

	return 0;
}

/**
 * wlan_hdd_cfg80211_set_ns_offload() - enable/disable NS offload
 * @wiphy:   pointer to wireless wiphy structure.
 * @wdev:    pointer to wireless_dev structure.
 * @data:    Pointer to the data to be passed via vendor interface
 * @data_len:Length of the data to be passed
 *
 * Return:   Return the Success or Failure code.
 */
static int wlan_hdd_cfg80211_set_ns_offload(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					const void *data, int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_set_ns_offload(wiphy, wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

static const struct nla_policy get_preferred_freq_list_policy
		[QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST_IFACE_TYPE] = {
		.type = NLA_U32},
};

/** __wlan_hdd_cfg80211_get_preferred_freq_list() - get preferred frequency list
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * This function return the preferred frequency list generated by the policy
 * manager.
 *
 * Return: success or failure code
 */
static int __wlan_hdd_cfg80211_get_preferred_freq_list(struct wiphy *wiphy,
						 struct wireless_dev
						 *wdev, const void *data,
						 int data_len)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	int i, ret = 0;
	QDF_STATUS status;
	uint8_t pcl[QDF_MAX_NUM_CHAN], weight_list[QDF_MAX_NUM_CHAN];
	uint32_t pcl_len = 0;
	uint32_t freq_list[QDF_MAX_NUM_CHAN];
	enum policy_mgr_con_mode intf_mode;
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST_MAX + 1];
	struct sk_buff *reply_skb;

	ENTER_DEV(wdev->netdev);

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return -EINVAL;

	if (wlan_cfg80211_nla_parse(tb,
			       QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST_MAX,
			       data, data_len,
			       get_preferred_freq_list_policy)) {
		hdd_err("Invalid ATTR");
		return -EINVAL;
	}

	if (!tb[QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST_IFACE_TYPE]) {
		hdd_err("attr interface type failed");
		return -EINVAL;
	}

	intf_mode = nla_get_u32(tb
		    [QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST_IFACE_TYPE]);

	if (intf_mode < PM_STA_MODE || intf_mode >= PM_MAX_NUM_OF_MODE) {
		hdd_err("Invalid interface type");
		return -EINVAL;
	}

	hdd_debug("Userspace requested pref freq list");

	status = policy_mgr_get_pcl(hdd_ctx->hdd_psoc,
				intf_mode, pcl, &pcl_len,
				weight_list, QDF_ARRAY_SIZE(weight_list));
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("Get pcl failed");
		return -EINVAL;
	}

	/* convert channel number to frequency */
	for (i = 0; i < pcl_len; i++) {
		if (pcl[i] <= ARRAY_SIZE(hdd_channels_2_4_ghz))
			freq_list[i] =
				ieee80211_channel_to_frequency(pcl[i],
							HDD_NL80211_BAND_2GHZ);
		else
			freq_list[i] =
				ieee80211_channel_to_frequency(pcl[i],
							HDD_NL80211_BAND_5GHZ);
	}

	/* send the freq_list back to supplicant */
	reply_skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, sizeof(u32) +
							sizeof(u32) *
							pcl_len +
							NLMSG_HDRLEN);

	if (!reply_skb) {
		hdd_err("Allocate reply_skb failed");
		return -EINVAL;
	}

	if (nla_put_u32(reply_skb,
		QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST_IFACE_TYPE,
			intf_mode) ||
		nla_put(reply_skb,
			QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST,
			sizeof(uint32_t) * pcl_len,
			freq_list)) {
		hdd_err("nla put fail");
		kfree_skb(reply_skb);
		return -EINVAL;
	}

	return cfg80211_vendor_cmd_reply(reply_skb);
}

/** wlan_hdd_cfg80211_get_preferred_freq_list () - get preferred frequency list
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * This function return the preferred frequency list generated by the policy
 * manager.
 *
 * Return: success or failure code
 */
static int wlan_hdd_cfg80211_get_preferred_freq_list(struct wiphy *wiphy,
						 struct wireless_dev
						 *wdev, const void *data,
						 int data_len)
{
	int ret = 0;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_get_preferred_freq_list(wiphy, wdev,
						data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

static const struct nla_policy set_probable_oper_channel_policy
		[QCA_WLAN_VENDOR_ATTR_PROBABLE_OPER_CHANNEL_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_PROBABLE_OPER_CHANNEL_IFACE_TYPE] = {
		.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_PROBABLE_OPER_CHANNEL_FREQ] = {
		.type = NLA_U32},
};

/**
 * __wlan_hdd_cfg80211_set_probable_oper_channel () - set probable channel
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: 0 on success, negative errno on failure
 */
static int __wlan_hdd_cfg80211_set_probable_oper_channel(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len)
{
	struct net_device *ndev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(ndev);
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	int ret = 0;
	enum policy_mgr_con_mode intf_mode;
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_PROBABLE_OPER_CHANNEL_MAX + 1];
	uint32_t channel_hint;

	ENTER_DEV(ndev);

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	if (wlan_cfg80211_nla_parse(tb,
				 QCA_WLAN_VENDOR_ATTR_PROBABLE_OPER_CHANNEL_MAX,
				 data, data_len,
				 set_probable_oper_channel_policy)) {
		hdd_err("Invalid ATTR");
		return -EINVAL;
	}

	if (!tb[QCA_WLAN_VENDOR_ATTR_PROBABLE_OPER_CHANNEL_IFACE_TYPE]) {
		hdd_err("attr interface type failed");
		return -EINVAL;
	}

	intf_mode = nla_get_u32(tb
		    [QCA_WLAN_VENDOR_ATTR_PROBABLE_OPER_CHANNEL_IFACE_TYPE]);

	if (intf_mode < PM_STA_MODE || intf_mode >= PM_MAX_NUM_OF_MODE) {
		hdd_err("Invalid interface type");
		return -EINVAL;
	}

	if (!tb[QCA_WLAN_VENDOR_ATTR_PROBABLE_OPER_CHANNEL_FREQ]) {
		hdd_err("attr probable freq failed");
		return -EINVAL;
	}

	channel_hint = cds_freq_to_chan(nla_get_u32(tb
			[QCA_WLAN_VENDOR_ATTR_PROBABLE_OPER_CHANNEL_FREQ]));

	/* check pcl table */
	if (!policy_mgr_allow_concurrency(hdd_ctx->hdd_psoc, intf_mode,
					channel_hint, HW_MODE_20_MHZ)) {
		hdd_err("Set channel hint failed due to concurrency check");
		return -EINVAL;
	}

	if (0 != wlan_hdd_check_remain_on_channel(adapter))
		hdd_warn("Remain On Channel Pending");

	if (wlan_hdd_change_hw_mode_for_given_chnl(adapter, channel_hint,
				POLICY_MGR_UPDATE_REASON_SET_OPER_CHAN)) {
		hdd_err("Failed to change hw mode");
		return -EINVAL;
	}

	return 0;
}

/**
 * wlan_hdd_cfg80211_set_probable_oper_channel () - set probable channel
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: 0 on success, negative errno on failure
 */
static int wlan_hdd_cfg80211_set_probable_oper_channel(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len)
{
	int ret = 0;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_set_probable_oper_channel(wiphy, wdev,
						data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

static const struct
nla_policy
qca_wlan_vendor_attr_policy[QCA_WLAN_VENDOR_ATTR_MAX+1] = {
	[QCA_WLAN_VENDOR_ATTR_MAC_ADDR] = {
		.type = NLA_BINARY, .len = QDF_MAC_ADDR_SIZE },
};

/**
 * __wlan_hdd_cfg80211_get_link_properties() - Get link properties
 * @wiphy: WIPHY structure pointer
 * @wdev: Wireless device structure pointer
 * @data: Pointer to the data received
 * @data_len: Length of the data received
 *
 * This function is used to get link properties like nss, rate flags and
 * operating frequency for the active connection with the given peer.
 *
 * Return: 0 on success and errno on failure
 */
static int __wlan_hdd_cfg80211_get_link_properties(struct wiphy *wiphy,
						   struct wireless_dev *wdev,
						   const void *data,
						   int data_len)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_station_ctx *hdd_sta_ctx;
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_MAX+1];
	uint8_t peer_mac[QDF_MAC_ADDR_SIZE];
	uint32_t sta_id;
	struct sk_buff *reply_skb;
	uint32_t rate_flags = 0;
	uint8_t nss;
	uint8_t final_rate_flags = 0;
	uint32_t freq;

	ENTER_DEV(dev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	if (0 != wlan_hdd_validate_context(hdd_ctx))
		return -EINVAL;

	if (wlan_cfg80211_nla_parse(tb, QCA_WLAN_VENDOR_ATTR_MAX, data,
				    data_len, qca_wlan_vendor_attr_policy)) {
		hdd_err("Invalid attribute");
		return -EINVAL;
	}

	if (!tb[QCA_WLAN_VENDOR_ATTR_MAC_ADDR]) {
		hdd_err("Attribute peerMac not provided for mode=%d",
		       adapter->device_mode);
		return -EINVAL;
	}

	if (nla_len(tb[QCA_WLAN_VENDOR_ATTR_MAC_ADDR]) < QDF_MAC_ADDR_SIZE) {
		hdd_err("Attribute peerMac is invalid for mode=%d",
			adapter->device_mode);
		return -EINVAL;
	}

	qdf_mem_copy(peer_mac, nla_data(tb[QCA_WLAN_VENDOR_ATTR_MAC_ADDR]),
		     QDF_MAC_ADDR_SIZE);
	hdd_debug("peerMac="MAC_ADDRESS_STR" for device_mode:%d",
	       MAC_ADDR_ARRAY(peer_mac), adapter->device_mode);

	if (adapter->device_mode == QDF_STA_MODE ||
	    adapter->device_mode == QDF_P2P_CLIENT_MODE) {
		hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
		if ((hdd_sta_ctx->conn_info.connState !=
			eConnectionState_Associated) ||
		    qdf_mem_cmp(hdd_sta_ctx->conn_info.bssId.bytes,
			peer_mac, QDF_MAC_ADDR_SIZE)) {
			hdd_err("Not Associated to mac "MAC_ADDRESS_STR,
			       MAC_ADDR_ARRAY(peer_mac));
			return -EINVAL;
		}

		nss  = hdd_sta_ctx->conn_info.nss;
		freq = cds_chan_to_freq(
				hdd_sta_ctx->conn_info.operationChannel);
		rate_flags = hdd_sta_ctx->conn_info.rate_flags;
	} else if (adapter->device_mode == QDF_P2P_GO_MODE ||
		   adapter->device_mode == QDF_SAP_MODE) {

		for (sta_id = 0; sta_id < WLAN_MAX_STA_COUNT; sta_id++) {
			if (adapter->sta_info[sta_id].in_use &&
			    !qdf_is_macaddr_broadcast(
				&adapter->sta_info[sta_id].sta_mac) &&
			    !qdf_mem_cmp(
				&adapter->sta_info[sta_id].sta_mac.bytes,
				peer_mac, QDF_MAC_ADDR_SIZE))
				break;
		}

		if (WLAN_MAX_STA_COUNT == sta_id) {
			hdd_err("No active peer with mac="MAC_ADDRESS_STR,
			       MAC_ADDR_ARRAY(peer_mac));
			return -EINVAL;
		}

		nss = adapter->sta_info[sta_id].nss;
		freq = cds_chan_to_freq(
			(WLAN_HDD_GET_AP_CTX_PTR(adapter))->operating_channel);
		rate_flags = adapter->sta_info[sta_id].rate_flags;
	} else {
		hdd_err("Not Associated! with mac "MAC_ADDRESS_STR,
		       MAC_ADDR_ARRAY(peer_mac));
		return -EINVAL;
	}

	if (!(rate_flags & eHAL_TX_RATE_LEGACY)) {
		if (rate_flags & eHAL_TX_RATE_VHT80) {
			final_rate_flags |= RATE_INFO_FLAGS_VHT_MCS;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)) && !defined(WITH_BACKPORTS)
			final_rate_flags |= RATE_INFO_FLAGS_80_MHZ_WIDTH;
#endif
		} else if (rate_flags & eHAL_TX_RATE_VHT40) {
			final_rate_flags |= RATE_INFO_FLAGS_VHT_MCS;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)) && !defined(WITH_BACKPORTS)
			final_rate_flags |= RATE_INFO_FLAGS_40_MHZ_WIDTH;
#endif
		} else if (rate_flags & eHAL_TX_RATE_VHT20) {
			final_rate_flags |= RATE_INFO_FLAGS_VHT_MCS;
		} else if (rate_flags &
				(eHAL_TX_RATE_HT20 | eHAL_TX_RATE_HT40)) {
			final_rate_flags |= RATE_INFO_FLAGS_MCS;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)) && !defined(WITH_BACKPORTS)
			if (rate_flags & eHAL_TX_RATE_HT40)
				final_rate_flags |=
					RATE_INFO_FLAGS_40_MHZ_WIDTH;
#endif
		}

		if (rate_flags & eHAL_TX_RATE_SGI) {
			if (!(final_rate_flags & RATE_INFO_FLAGS_VHT_MCS))
				final_rate_flags |= RATE_INFO_FLAGS_MCS;
			final_rate_flags |= RATE_INFO_FLAGS_SHORT_GI;
		}
	}

	reply_skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy,
			sizeof(u8) + sizeof(u8) + sizeof(u32) + NLMSG_HDRLEN);

	if (NULL == reply_skb) {
		hdd_err("getLinkProperties: skb alloc failed");
		return -EINVAL;
	}

	if (nla_put_u8(reply_skb,
		QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_NSS,
		nss) ||
	    nla_put_u8(reply_skb,
		QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_RATE_FLAGS,
		final_rate_flags) ||
	    nla_put_u32(reply_skb,
		QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_FREQ,
		freq)) {
		hdd_err("nla_put failed");
		kfree_skb(reply_skb);
		return -EINVAL;
	}

	return cfg80211_vendor_cmd_reply(reply_skb);
}

/**
 * wlan_hdd_cfg80211_get_link_properties() - Wrapper function to get link
 * properties.
 * @wiphy: WIPHY structure pointer
 * @wdev: Wireless device structure pointer
 * @data: Pointer to the data received
 * @data_len: Length of the data received
 *
 * This function is used to get link properties like nss, rate flags and
 * operating frequency for the active connection with the given peer.
 *
 * Return: 0 on success and errno on failure
 */
static int wlan_hdd_cfg80211_get_link_properties(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data,
						 int data_len)
{
	int ret = 0;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_get_link_properties(wiphy,
			wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

static const struct
nla_policy
qca_wlan_vendor_ota_test_policy
[QCA_WLAN_VENDOR_ATTR_OTA_TEST_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_OTA_TEST_ENABLE] = {.type = NLA_U8 },
};

/**
 * __wlan_hdd_cfg80211_set_ota_test () - enable/disable OTA test
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: 0 on success, negative errno on failure
 */
static int __wlan_hdd_cfg80211_set_ota_test(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len)
{
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	tHalHandle hal = WLAN_HDD_GET_HAL_CTX(adapter);
	struct hdd_context *hdd_ctx  = wiphy_priv(wiphy);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_OTA_TEST_MAX + 1];
	uint8_t ota_enable = 0;
	QDF_STATUS status;
	uint32_t current_roam_state;

	ENTER_DEV(dev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	if (0 != wlan_hdd_validate_context(hdd_ctx))
		return -EINVAL;

	if (wlan_cfg80211_nla_parse(tb, QCA_WLAN_VENDOR_ATTR_OTA_TEST_MAX,
				    data, data_len,
				    qca_wlan_vendor_ota_test_policy)) {
		hdd_err("invalid attr");
		return -EINVAL;
	}

	if (!tb[QCA_WLAN_VENDOR_ATTR_OTA_TEST_ENABLE]) {
		hdd_err("attr ota test failed");
		return -EINVAL;
	}

	ota_enable = nla_get_u8(
		tb[QCA_WLAN_VENDOR_ATTR_OTA_TEST_ENABLE]);

	hdd_debug(" OTA test enable = %d", ota_enable);
	if (ota_enable != 1) {
		hdd_err("Invalid value, only enable test mode is supported!");
		return -EINVAL;
	}

	current_roam_state =
			sme_get_current_roam_state(hal, adapter->session_id);
	status = sme_stop_roaming(hal, adapter->session_id,
					eCsrHddIssued);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("Enable/Disable roaming failed");
		return -EINVAL;
	}

	status = sme_ps_enable_disable(hal, adapter->session_id,
					SME_PS_DISABLE);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("Enable/Disable power save failed");
		/* restore previous roaming setting */
		if (current_roam_state == eCSR_ROAMING_STATE_JOINING ||
			 current_roam_state == eCSR_ROAMING_STATE_JOINED)
			status = sme_start_roaming(hal, adapter->session_id,
						eCsrHddIssued);
		else if (current_roam_state == eCSR_ROAMING_STATE_STOP ||
			 current_roam_state == eCSR_ROAMING_STATE_IDLE)
			status = sme_stop_roaming(hal, adapter->session_id,
						eCsrHddIssued);

		if (status != QDF_STATUS_SUCCESS)
			hdd_err("Restoring roaming state failed");

		return -EINVAL;
	}


	return 0;
}

/**
 * wlan_hdd_cfg80211_set_ota_test () - Enable or disable OTA test
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: 0 on success, negative errno on failure
 */
static int wlan_hdd_cfg80211_set_ota_test(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					const void *data,
					int data_len)
{
	int ret = 0;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_set_ota_test(wiphy, wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

static const struct nla_policy
txpower_scale_policy[QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE] = { .type = NLA_U8 },
};

/**
 * __wlan_hdd_cfg80211_txpower_scale () - txpower scaling
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: 0 on success, negative errno on failure
 */
static int __wlan_hdd_cfg80211_txpower_scale(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter;
	int ret;
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_MAX + 1];
	uint8_t scale_value;
	QDF_STATUS status;

	ENTER_DEV(dev);

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	adapter = WLAN_HDD_GET_PRIV_PTR(dev);

	if (wlan_cfg80211_nla_parse(tb, QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_MAX,
				    data, data_len, txpower_scale_policy)) {
		hdd_err("Invalid ATTR");
		return -EINVAL;
	}

	if (!tb[QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE]) {
		hdd_err("attr tx power scale failed");
		return -EINVAL;
	}

	scale_value = nla_get_u8(tb
		    [QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE]);

	if (scale_value > MAX_TXPOWER_SCALE) {
		hdd_err("Invalid tx power scale level");
		return -EINVAL;
	}

	status = wma_set_tx_power_scale(adapter->session_id, scale_value);

	if (QDF_STATUS_SUCCESS != status) {
		hdd_err("Set tx power scale failed");
		return -EINVAL;
	}

	return 0;
}

/**
 * wlan_hdd_cfg80211_txpower_scale () - txpower scaling
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: 0 on success, negative errno on failure
 */
static int wlan_hdd_cfg80211_txpower_scale(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_txpower_scale(wiphy, wdev,
						data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

static const struct nla_policy txpower_scale_decr_db_policy
[QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_DECR_DB_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_DECR_DB] = { .type = NLA_U8 },
};

/**
 * __wlan_hdd_cfg80211_txpower_scale_decr_db () - txpower scaling
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: 0 on success, negative errno on failure
 */
static int __wlan_hdd_cfg80211_txpower_scale_decr_db(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter;
	int ret;
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_DECR_DB_MAX + 1];
	uint8_t scale_value;
	QDF_STATUS status;

	ENTER_DEV(dev);

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	adapter = WLAN_HDD_GET_PRIV_PTR(dev);

	if (wlan_cfg80211_nla_parse(tb,
				 QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_DECR_DB_MAX,
				 data, data_len,
				 txpower_scale_decr_db_policy)) {
		hdd_err("Invalid ATTR");
		return -EINVAL;
	}

	if (!tb[QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_DECR_DB]) {
		hdd_err("attr tx power decrease db value failed");
		return -EINVAL;
	}

	scale_value = nla_get_u8(tb
		    [QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_DECR_DB]);

	status = wma_set_tx_power_scale_decr_db(adapter->session_id,
							scale_value);

	if (QDF_STATUS_SUCCESS != status) {
		hdd_err("Set tx power decrease db failed");
		return -EINVAL;
	}

	return 0;
}

/**
 * wlan_hdd_cfg80211_txpower_scale_decr_db () - txpower scaling
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: 0 on success, negative errno on failure
 */
static int wlan_hdd_cfg80211_txpower_scale_decr_db(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_txpower_scale_decr_db(wiphy, wdev,
						data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __wlan_hdd_cfg80211_conditional_chan_switch() - Conditional channel switch
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Processes the conditional channel switch request and invokes the helper
 * APIs to process the channel switch request.
 *
 * Return: 0 on success, negative errno on failure
 */
static int __wlan_hdd_cfg80211_conditional_chan_switch(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len)
{
	int ret;
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter;
	struct nlattr
		*tb[QCA_WLAN_VENDOR_ATTR_SAP_CONDITIONAL_CHAN_SWITCH_MAX + 1];
	uint32_t freq_len, i;
	uint32_t *freq;
	uint8_t chans[QDF_MAX_NUM_CHAN] = {0};

	ENTER_DEV(dev);

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	if (!hdd_ctx->config->enableDFSMasterCap) {
		hdd_err("DFS master capability is not present in the driver");
		return -EINVAL;
	}

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	if (adapter->device_mode != QDF_SAP_MODE) {
		hdd_err("Invalid device mode %d", adapter->device_mode);
		return -EINVAL;
	}

	/*
	 * audit note: it is ok to pass a NULL policy here since only
	 * one attribute is parsed which is array of frequencies and
	 * it is explicitly validated for both under read and over read
	 */
	if (wlan_cfg80211_nla_parse(tb,
			   QCA_WLAN_VENDOR_ATTR_SAP_CONDITIONAL_CHAN_SWITCH_MAX,
			   data, data_len, NULL)) {
		hdd_err("Invalid ATTR");
		return -EINVAL;
	}

	if (!tb[QCA_WLAN_VENDOR_ATTR_SAP_CONDITIONAL_CHAN_SWITCH_FREQ_LIST]) {
		hdd_err("Frequency list is missing");
		return -EINVAL;
	}

	freq_len = nla_len(
		tb[QCA_WLAN_VENDOR_ATTR_SAP_CONDITIONAL_CHAN_SWITCH_FREQ_LIST])/
		sizeof(uint32_t);

	if (freq_len > QDF_MAX_NUM_CHAN) {
		hdd_err("insufficient space to hold channels");
		return -ENOMEM;
	}

	hdd_debug("freq_len=%d", freq_len);

	freq = nla_data(
		tb[QCA_WLAN_VENDOR_ATTR_SAP_CONDITIONAL_CHAN_SWITCH_FREQ_LIST]);


	for (i = 0; i < freq_len; i++) {
		if (freq[i] == 0)
			chans[i] = 0;
		else
			chans[i] = ieee80211_frequency_to_channel(freq[i]);

		hdd_debug("freq[%d]=%d", i, freq[i]);
	}

	/*
	 * The input frequency list from user space is designed to be a
	 * priority based frequency list. This is only to accommodate any
	 * future request. But, current requirement is only to perform CAC
	 * on a single channel. So, the first entry from the list is picked.
	 *
	 * If channel is zero, any channel in the available outdoor regulatory
	 * domain will be selected.
	 */
	ret = wlan_hdd_request_pre_cac(chans[0]);
	if (ret) {
		hdd_err("pre cac request failed with reason:%d", ret);
		return ret;
	}

	return 0;
}

/* P2P listen offload device types parameters length in bytes */
#define P2P_LO_MAX_REQ_DEV_TYPE_COUNT (10)
#define P2P_LO_WPS_DEV_TYPE_LEN (8)
#define P2P_LO_DEV_TYPE_MAX_LEN \
	(P2P_LO_MAX_REQ_DEV_TYPE_COUNT * P2P_LO_WPS_DEV_TYPE_LEN)

static const struct nla_policy
p2p_listen_offload_policy[QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_CHANNEL] = { .type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_PERIOD] = { .type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_INTERVAL] = {
							.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_COUNT] = { .type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_DEVICE_TYPES] = {
					.type = NLA_BINARY,
					.len = P2P_LO_DEV_TYPE_MAX_LEN },
	[QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_VENDOR_IE] = {
					.type = NLA_BINARY,
					.len = MAX_GENIE_LEN },
	[QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_CTRL_FLAG] = {
					.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_CHANNEL] = { .type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_STOP_REASON] = {
						.type = NLA_U8 },
};

/**
 * __wlan_hdd_cfg80211_p2p_lo_start () - start P2P Listen Offload
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * This function is to process the p2p listen offload start vendor
 * command. It parses the input parameters and invoke WMA API to
 * send the command to firmware.
 *
 * Return: 0 on success, negative errno on failure
 */
static int __wlan_hdd_cfg80211_p2p_lo_start(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len)
{
	int ret;
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter;
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_MAX + 1];
	struct sir_p2p_lo_start params;

	ENTER_DEV(dev);

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	if ((adapter->device_mode != QDF_P2P_DEVICE_MODE) &&
	    (adapter->device_mode != QDF_P2P_CLIENT_MODE) &&
	    (adapter->device_mode != QDF_P2P_GO_MODE)) {
		hdd_err("Invalid device mode %d", adapter->device_mode);
		return -EINVAL;
	}

	if (wlan_cfg80211_nla_parse(tb,
				    QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_MAX,
				    data, data_len,
				    p2p_listen_offload_policy)) {
		hdd_err("Invalid ATTR");
		return -EINVAL;
	}

	memset(&params, 0, sizeof(params));

	if (!tb[QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_CTRL_FLAG])
		params.ctl_flags = 1;  /* set to default value */
	else
		params.ctl_flags = nla_get_u32(tb
			[QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_CTRL_FLAG]);

	if (!tb[QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_CHANNEL] ||
	    !tb[QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_PERIOD] ||
	    !tb[QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_INTERVAL] ||
	    !tb[QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_COUNT] ||
	    !tb[QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_DEVICE_TYPES] ||
	    !tb[QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_VENDOR_IE]) {
		hdd_err("Attribute parsing failed");
		return -EINVAL;
	}

	params.vdev_id = adapter->session_id;
	params.freq = nla_get_u32(tb
		[QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_CHANNEL]);
	if ((params.freq != 2412) && (params.freq != 2437) &&
		(params.freq != 2462)) {
		hdd_err("Invalid listening channel: %d", params.freq);
		return -EINVAL;
	}

	params.period = nla_get_u32(tb
		[QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_PERIOD]);
	if (!((params.period > 0) && (params.period < UINT_MAX))) {
		hdd_err("Invalid period: %d", params.period);
		return -EINVAL;
	}

	params.interval = nla_get_u32(tb
		[QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_INTERVAL]);
	if (!((params.interval > 0) && (params.interval < UINT_MAX))) {
		hdd_err("Invalid interval: %d", params.interval);
		return -EINVAL;
	}

	params.count = nla_get_u32(tb
		[QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_COUNT]);
	if (!((params.count >= 0) && (params.count < UINT_MAX))) {
		hdd_err("Invalid count: %d", params.count);
		return -EINVAL;
	}

	params.device_types = nla_data(tb
		[QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_DEVICE_TYPES]);
	if (params.device_types == NULL) {
		hdd_err("Invalid device types");
		return -EINVAL;
	}

	params.dev_types_len = nla_len(tb
		[QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_DEVICE_TYPES]);
	/* device type length has to be multiple of P2P_LO_WPS_DEV_TYPE_LEN */
	if (0 != (params.dev_types_len % P2P_LO_WPS_DEV_TYPE_LEN)) {
		hdd_err("Invalid device type length: %d", params.dev_types_len);
		return -EINVAL;
	}

	params.probe_resp_tmplt = nla_data(tb
		[QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_VENDOR_IE]);
	if (params.probe_resp_tmplt == NULL) {
		hdd_err("Invalid probe response template");
		return -EINVAL;
	}

	/*
	 * IEs minimum length should be 2 bytes: 1 byte for element id
	 * and 1 byte for element id length.
	 */
	params.probe_resp_len = nla_len(tb
		[QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_VENDOR_IE]);
	if (params.probe_resp_len < MIN_GENIE_LEN) {
		hdd_err("Invalid probe resp template length: %d",
				params.probe_resp_len);
		return -EINVAL;
	}

	hdd_debug("P2P LO params: freq=%d, period=%d, interval=%d, count=%d",
		  params.freq, params.period, params.interval, params.count);

	return wlan_hdd_listen_offload_start(adapter, &params);
}


/**
 * wlan_hdd_cfg80211_p2p_lo_start () - start P2P Listen Offload
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * This function inovkes internal __wlan_hdd_cfg80211_p2p_lo_start()
 * to process p2p listen offload start vendor command.
 *
 * Return: 0 on success, negative errno on failure
 */
static int wlan_hdd_cfg80211_p2p_lo_start(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len)
{
	int ret = 0;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_p2p_lo_start(wiphy, wdev,
					data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __wlan_hdd_cfg80211_p2p_lo_stop () - stop P2P Listen Offload
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * This function is to process the p2p listen offload stop vendor
 * command. It invokes WMA API to send command to firmware.
 *
 * Return: 0 on success, negative errno on failure
 */
static int __wlan_hdd_cfg80211_p2p_lo_stop(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len)
{
	struct hdd_adapter *adapter;
	struct net_device *dev = wdev->netdev;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	if ((adapter->device_mode != QDF_P2P_DEVICE_MODE) &&
	    (adapter->device_mode != QDF_P2P_CLIENT_MODE) &&
	    (adapter->device_mode != QDF_P2P_GO_MODE)) {
		hdd_err("Invalid device mode");
		return -EINVAL;
	}

	return wlan_hdd_listen_offload_stop(adapter);
}

/**
 * wlan_hdd_cfg80211_p2p_lo_stop () - stop P2P Listen Offload
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * This function inovkes internal __wlan_hdd_cfg80211_p2p_lo_stop()
 * to process p2p listen offload stop vendor command.
 *
 * Return: 0 on success, negative errno on failure
 */
static int wlan_hdd_cfg80211_p2p_lo_stop(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len)
{
	int ret = 0;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_p2p_lo_stop(wiphy, wdev,
						data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * wlan_hdd_cfg80211_conditional_chan_switch() - SAP conditional channel switch
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Inovkes internal API __wlan_hdd_cfg80211_conditional_chan_switch()
 * to process the conditional channel switch request.
 *
 * Return: 0 on success, negative errno on failure
 */
static int wlan_hdd_cfg80211_conditional_chan_switch(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_conditional_chan_switch(wiphy, wdev,
						data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/*
 * define short names for the global vendor params
 * used by __wlan_hdd_cfg80211_bpf_offload()
 */
#define BPF_INVALID \
	QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_INVALID
#define BPF_SET_RESET \
	QCA_WLAN_VENDOR_ATTR_SET_RESET_PACKET_FILTER
#define BPF_VERSION \
	QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_VERSION
#define BPF_FILTER_ID \
	QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_ID
#define BPF_PACKET_SIZE \
	QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_SIZE
#define BPF_CURRENT_OFFSET \
	QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_CURRENT_OFFSET
#define BPF_PROGRAM \
	QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_PROGRAM
#define BPF_MAX \
	QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_MAX

static const struct nla_policy
wlan_hdd_bpf_offload_policy[BPF_MAX + 1] = {
	[BPF_SET_RESET] = {.type = NLA_U32},
	[BPF_VERSION] = {.type = NLA_U32},
	[BPF_FILTER_ID] = {.type = NLA_U32},
	[BPF_PACKET_SIZE] = {.type = NLA_U32},
	[BPF_CURRENT_OFFSET] = {.type = NLA_U32},
	[BPF_PROGRAM] = {.type = NLA_U8},
};

struct bpf_offload_priv {
	struct sir_bpf_get_offload bpf_get_offload;
};

/**
 * hdd_get_bpf_offload_cb() - Callback function to BPF Offload
 * @context: opaque context originally passed to SME.  HDD always passes
 *	a cookie for the request context
 * @bpf_get_offload: struct for get offload
 *
 * This function receives the response/data from the lower layer and
 * checks to see if the thread is still waiting then post the results to
 * upper layer, if the request has timed out then ignore.
 *
 * Return: None
 */
static void hdd_get_bpf_offload_cb(void *context,
				   struct sir_bpf_get_offload *data)
{
	struct hdd_request *request;
	struct bpf_offload_priv *priv;

	ENTER();

	request = hdd_request_get(context);
	if (!request) {
		hdd_err("Obsolete request");
		return;
	}

	priv = hdd_request_priv(request);
	priv->bpf_get_offload = *data;
	hdd_request_complete(request);
	hdd_request_put(request);
}

/**
 * hdd_post_get_bpf_capabilities_rsp() - Callback function to BPF Offload
 * @hdd_context: hdd_context
 * @bpf_get_offload: struct for get offload
 *
 * Return: 0 on success, error number otherwise.
 */
static int hdd_post_get_bpf_capabilities_rsp(struct hdd_context *hdd_ctx,
			    struct sir_bpf_get_offload *bpf_get_offload)
{
	struct sk_buff *skb;
	uint32_t nl_buf_len;

	ENTER();

	nl_buf_len = NLMSG_HDRLEN;
	nl_buf_len +=
		(sizeof(bpf_get_offload->max_bytes_for_bpf_inst) + NLA_HDRLEN) +
		(sizeof(bpf_get_offload->bpf_version) + NLA_HDRLEN);

	skb = cfg80211_vendor_cmd_alloc_reply_skb(hdd_ctx->wiphy, nl_buf_len);
	if (!skb) {
		hdd_err("cfg80211_vendor_cmd_alloc_reply_skb failed");
		return -ENOMEM;
	}

	hdd_debug("BPF Version: %u BPF max bytes: %u",
			bpf_get_offload->bpf_version,
			bpf_get_offload->max_bytes_for_bpf_inst);

	if (nla_put_u32(skb, BPF_PACKET_SIZE,
			bpf_get_offload->max_bytes_for_bpf_inst) ||
	    nla_put_u32(skb, BPF_VERSION, bpf_get_offload->bpf_version)) {
		hdd_err("nla put failure");
		goto nla_put_failure;
	}

	cfg80211_vendor_cmd_reply(skb);
	EXIT();
	return 0;

nla_put_failure:
	kfree_skb(skb);
	return -EINVAL;
}

/**
 * hdd_get_bpf_offload - Get BPF offload Capabilities
 * @hdd_ctx: Hdd context
 *
 * Return: 0 on success, errno on failure
 */
static int hdd_get_bpf_offload(struct hdd_context *hdd_ctx)
{
	QDF_STATUS status;
	int ret;
	void *cookie;
	struct hdd_request *request;
	struct bpf_offload_priv *priv;
	static const struct hdd_request_params params = {
		.priv_size = sizeof(*priv),
		.timeout_ms = WLAN_WAIT_TIME_BPF,
	};

	ENTER();

	request = hdd_request_alloc(&params);
	if (!request) {
		hdd_err("Unable to allocate request");
		return -EINVAL;
	}
	cookie = hdd_request_cookie(request);

	status = sme_get_bpf_offload_capabilities(hdd_ctx->hHal,
						  hdd_get_bpf_offload_cb,
						  cookie);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("Unable to retrieve BPF caps");
		ret = qdf_status_to_os_return(status);
		goto cleanup;
	}
	ret = hdd_request_wait_for_response(request);
	if (ret) {
		hdd_err("Target response timed out");
		goto cleanup;
	}
	priv = hdd_request_priv(request);
	ret = hdd_post_get_bpf_capabilities_rsp(hdd_ctx,
						&priv->bpf_get_offload);
	if (ret)
		hdd_err("Failed to post get bpf capabilities");

cleanup:
	/*
	 * either we never sent a request to SME, we sent a request to
	 * SME and timed out, or we sent a request to SME, received a
	 * response from SME, and posted the response to userspace.
	 * regardless we are done with the request.
	 */
	hdd_request_put(request);
	EXIT();

	return ret;
}

/**
 * hdd_set_reset_bpf_offload - Post set/reset bpf to SME
 * @hdd_ctx: Hdd context
 * @tb: Length of @data
 * @adapter: pointer to adapter struct
 *
 * Return: 0 on success; errno on failure
 */
static int hdd_set_reset_bpf_offload(struct hdd_context *hdd_ctx,
				     struct nlattr **tb,
				     struct hdd_adapter *adapter)
{
	struct sir_bpf_set_offload *bpf_set_offload;
	QDF_STATUS status;
	int prog_len;
	int ret = 0;

	ENTER();

	if (adapter->device_mode == QDF_STA_MODE ||
	    adapter->device_mode == QDF_P2P_CLIENT_MODE) {
		if (!hdd_conn_is_connected(
		    WLAN_HDD_GET_STATION_CTX_PTR(adapter))) {
			hdd_err("Not in Connected state!");
			return -ENOTSUPP;
		}
	}

	bpf_set_offload = qdf_mem_malloc(sizeof(*bpf_set_offload));
	if (bpf_set_offload == NULL) {
		hdd_err("qdf_mem_malloc failed for bpf_set_offload");
		return -ENOMEM;
	}

	/* Parse and fetch bpf packet size */
	if (!tb[BPF_PACKET_SIZE]) {
		hdd_err("attr bpf packet size failed");
		ret = -EINVAL;
		goto fail;
	}
	bpf_set_offload->total_length = nla_get_u32(tb[BPF_PACKET_SIZE]);

	if (!bpf_set_offload->total_length) {
		hdd_debug("BPF reset packet filter received");
		goto post_sme;
	}

	/* Parse and fetch bpf program */
	if (!tb[BPF_PROGRAM]) {
		hdd_err("attr bpf program failed");
		ret = -EINVAL;
		goto fail;
	}

	prog_len = nla_len(tb[BPF_PROGRAM]);
	bpf_set_offload->program = qdf_mem_malloc(sizeof(uint8_t) * prog_len);

	if (bpf_set_offload->program == NULL) {
		hdd_err("qdf_mem_malloc failed for bpf offload program");
		ret = -ENOMEM;
		goto fail;
	}

	bpf_set_offload->current_length = prog_len;
	nla_memcpy(bpf_set_offload->program, tb[BPF_PROGRAM], prog_len);
	bpf_set_offload->session_id = adapter->session_id;

	hdd_debug("BPF set instructions");
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_DEBUG,
			   bpf_set_offload->program, prog_len);

	/* Parse and fetch filter Id */
	if (!tb[BPF_FILTER_ID]) {
		hdd_err("attr filter id failed");
		ret = -EINVAL;
		goto fail;
	}
	bpf_set_offload->filter_id = nla_get_u32(tb[BPF_FILTER_ID]);

	/* Parse and fetch current offset */
	if (!tb[BPF_CURRENT_OFFSET]) {
		hdd_err("attr current offset failed");
		ret = -EINVAL;
		goto fail;
	}
	bpf_set_offload->current_offset = nla_get_u32(tb[BPF_CURRENT_OFFSET]);

post_sme:
	hdd_debug("Posting BPF SET/RESET to SME, session_id: %d Bpf Version: %d filter ID: %d total_length: %d current_length: %d current offset: %d",
			bpf_set_offload->session_id,
			bpf_set_offload->version,
			bpf_set_offload->filter_id,
			bpf_set_offload->total_length,
			bpf_set_offload->current_length,
			bpf_set_offload->current_offset);

	status = sme_set_bpf_instructions(hdd_ctx->hHal, bpf_set_offload);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("sme_set_bpf_instructions failed(err=%d)", status);
		ret = -EINVAL;
		goto fail;
	}
	EXIT();

fail:
	if (bpf_set_offload->current_length)
		qdf_mem_free(bpf_set_offload->program);
	qdf_mem_free(bpf_set_offload);
	return ret;
}

/**
 * wlan_hdd_cfg80211_bpf_offload() - Set/Reset to BPF Offload
 * @wiphy:    wiphy structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of @data
 *
 * Return: 0 on success; errno on failure
 */
static int
__wlan_hdd_cfg80211_bpf_offload(struct wiphy *wiphy,
				struct wireless_dev *wdev,
				const void *data, int data_len)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter =  WLAN_HDD_GET_PRIV_PTR(dev);
	struct nlattr *tb[BPF_MAX + 1];
	int ret_val, packet_filter_subcmd;

	ENTER();

	ret_val = wlan_hdd_validate_context(hdd_ctx);
	if (ret_val)
		return ret_val;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (!hdd_ctx->bpf_enabled) {
		hdd_err("BPF offload is not supported/enabled");
		return -ENOTSUPP;
	}

	if (wlan_cfg80211_nla_parse(tb, BPF_MAX, data, data_len,
				    wlan_hdd_bpf_offload_policy)) {
		hdd_err("Invalid ATTR");
		return -EINVAL;
	}

	if (!tb[BPF_SET_RESET]) {
		hdd_err("attr bpf set reset failed");
		return -EINVAL;
	}

	packet_filter_subcmd = nla_get_u32(tb[BPF_SET_RESET]);

	if (packet_filter_subcmd == QCA_WLAN_GET_PACKET_FILTER)
		return hdd_get_bpf_offload(hdd_ctx);
	else
		return hdd_set_reset_bpf_offload(hdd_ctx, tb,
						 adapter);
}

/**
 * wlan_hdd_cfg80211_bpf_offload() - SSR Wrapper to BPF Offload
 * @wiphy:    wiphy structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of @data
 *
 * Return: 0 on success; errno on failure
 */

static int wlan_hdd_cfg80211_bpf_offload(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_bpf_offload(wiphy, wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * wlan_hdd_set_pre_cac_status() - Set the pre cac status
 * @pre_cac_adapter: AP adapter used for pre cac
 * @status: Status (true or false)
 * @handle: Global handle
 *
 * Sets the status of pre cac i.e., whether the pre cac is active or not
 *
 * Return: Zero on success, non-zero on failure
 */
static int wlan_hdd_set_pre_cac_status(struct hdd_adapter *pre_cac_adapter,
				bool status, tHalHandle handle)
{
	QDF_STATUS ret;

	ret = wlan_sap_set_pre_cac_status(
		WLAN_HDD_GET_SAP_CTX_PTR(pre_cac_adapter), status, handle);
	if (QDF_IS_STATUS_ERROR(ret))
		return -EINVAL;

	return 0;
}

/**
 * wlan_hdd_set_chan_before_pre_cac() - Save the channel before pre cac
 * @ap_adapter: AP adapter
 * @chan_before_pre_cac: Channel
 *
 * Saves the channel which the AP was beaconing on before moving to the pre
 * cac channel. If radar is detected on the pre cac channel, this saved
 * channel will be used for AP operations.
 *
 * Return: Zero on success, non-zero on failure
 */
static int wlan_hdd_set_chan_before_pre_cac(struct hdd_adapter *ap_adapter,
			uint8_t chan_before_pre_cac)
{
	QDF_STATUS ret;

	ret = wlan_sap_set_chan_before_pre_cac(
		WLAN_HDD_GET_SAP_CTX_PTR(ap_adapter), chan_before_pre_cac);
	if (QDF_IS_STATUS_ERROR(ret))
		return -EINVAL;

	return 0;
}

int wlan_hdd_sap_get_valid_channellist(struct hdd_adapter *adapter,
				       uint32_t *channel_count,
				       uint8_t *channel_list,
				       enum band_info band)
{
	tsap_Config_t *sap_config;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	uint8_t tmp_chan_list[QDF_MAX_NUM_CHAN] = {0};
	uint32_t chan_count;
	uint8_t i;
	QDF_STATUS status;
	struct wlan_objmgr_pdev *pdev = hdd_ctx->hdd_pdev;
	uint8_t tmp_chan;

	sap_config = &adapter->session.ap.sap_config;

	status =
		policy_mgr_get_valid_chans(hdd_ctx->hdd_psoc,
					   tmp_chan_list,
					   &chan_count);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("Failed to get channel list");
		return -EINVAL;
	}

	for (i = 0; i < chan_count; i++) {
		tmp_chan = tmp_chan_list[i];
		if (*channel_count < QDF_MAX_NUM_CHAN) {
			if ((BAND_2G == band) &&
			    (WLAN_REG_IS_24GHZ_CH(tmp_chan)) &&
			    (!wlan_reg_is_disable_ch(pdev, tmp_chan))) {
				channel_list[*channel_count] = tmp_chan;
				*channel_count += 1;
			} else if ((BAND_5G == band) &&
				(WLAN_REG_IS_5GHZ_CH(tmp_chan)) &&
				(!wlan_reg_is_disable_ch(pdev, tmp_chan))) {
				channel_list[*channel_count] = tmp_chan;
				*channel_count += 1;
			}
		} else {
			break;
		}
	}

	if (*channel_count == 0) {
		hdd_err("no valid channel found");
		return -EINVAL;
	}

	return 0;
}

/**
 * wlan_hdd_validate_and_get_pre_cac_ch() - Validate and get pre cac channel
 * @hdd_ctx: HDD context
 * @ap_adapter: AP adapter
 * @channel: Channel requested by userspace
 * @pre_cac_chan: Pointer to the pre CAC channel
 *
 * Validates the channel provided by userspace. If user provided channel 0,
 * a valid outdoor channel must be selected from the regulatory channel.
 *
 * Return: Zero on success and non zero value on error
 */
static int wlan_hdd_validate_and_get_pre_cac_ch(struct hdd_context *hdd_ctx,
						struct hdd_adapter *ap_adapter,
						uint8_t channel,
						uint8_t *pre_cac_chan)
{
	uint32_t i;
	QDF_STATUS status;
	uint32_t weight_len = 0;
	uint32_t len = WNI_CFG_VALID_CHANNEL_LIST_LEN;
	uint8_t channel_list[QDF_MAX_NUM_CHAN] = {0};
	uint8_t pcl_weights[QDF_MAX_NUM_CHAN] = {0};

	if (0 == channel) {
		/* Channel is not obtained from PCL because PCL may not have
		 * the entire channel list. For example: if SAP is up on
		 * channel 6 and PCL is queried for the next SAP interface,
		 * if SCC is preferred, the PCL will contain only the channel
		 * 6. But, we are in need of a DFS channel. So, going with the
		 * first channel from the valid channel list.
		 */
		status = policy_mgr_get_valid_chans(hdd_ctx->hdd_psoc,
				channel_list, &len);
		if (QDF_IS_STATUS_ERROR(status)) {
			hdd_err("Failed to get channel list");
			return -EINVAL;
		}
		policy_mgr_update_with_safe_channel_list(hdd_ctx->hdd_psoc,
				channel_list, &len, pcl_weights, weight_len);
		for (i = 0; i < len; i++) {
			if (wlan_reg_is_dfs_ch(hdd_ctx->hdd_pdev,
						channel_list[i])) {
				*pre_cac_chan = channel_list[i];
				break;
			}
		}
		if (*pre_cac_chan == 0) {
			hdd_err("unable to find outdoor channel");
			return -EINVAL;
		}
	} else {
		/* Only when driver selects a channel, check is done for
		 * unnsafe and NOL channels. When user provides a fixed channel
		 * the user is expected to take care of this.
		 */
		if (!sme_is_channel_valid(hdd_ctx->hHal, channel) ||
			!wlan_reg_is_dfs_ch(hdd_ctx->hdd_pdev, channel)) {
			hdd_err("Invalid channel for pre cac:%d", channel);
			return -EINVAL;
		}
		*pre_cac_chan = channel;
	}
	hdd_debug("selected pre cac channel:%d", *pre_cac_chan);
	return 0;
}

/**
 * wlan_hdd_request_pre_cac() - Start pre CAC in the driver
 * @channel: Channel option provided by userspace
 *
 * Sets the driver to the required hardware mode and start an adapater for
 * pre CAC which will mimic an AP.
 *
 * Return: Zero on success, non-zero value on error
 */
int wlan_hdd_request_pre_cac(uint8_t channel)
{
	uint8_t pre_cac_chan = 0, *mac_addr;
	struct hdd_context *hdd_ctx;
	int ret;
	struct hdd_adapter *ap_adapter, *pre_cac_adapter;
	struct hdd_ap_ctx *hdd_ap_ctx;
	QDF_STATUS status;
	struct wiphy *wiphy;
	struct net_device *dev;
	struct cfg80211_chan_def chandef;
	enum nl80211_channel_type channel_type;
	uint32_t freq;
	struct ieee80211_channel *chan;
	tHalHandle handle;
	bool val;

	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	if (0 != wlan_hdd_validate_context(hdd_ctx))
		return -EINVAL;

	if (policy_mgr_get_connection_count(hdd_ctx->hdd_psoc) > 1) {
		hdd_err("pre cac not allowed in concurrency");
		return -EINVAL;
	}

	ap_adapter = hdd_get_adapter(hdd_ctx, QDF_SAP_MODE);
	if (!ap_adapter) {
		hdd_err("unable to get SAP adapter");
		return -EINVAL;
	}

	handle = WLAN_HDD_GET_HAL_CTX(ap_adapter);
	if (!handle) {
		hdd_err("Invalid handle");
		return -EINVAL;
	}

	val = wlan_sap_is_pre_cac_active(handle);
	if (val) {
		hdd_err("pre cac is already in progress");
		return -EINVAL;
	}

	hdd_ap_ctx = WLAN_HDD_GET_AP_CTX_PTR(ap_adapter);
	if (!hdd_ap_ctx) {
		hdd_err("SAP context is NULL");
		return -EINVAL;
	}

	if (wlan_reg_is_dfs_ch(hdd_ctx->hdd_pdev,
				hdd_ap_ctx->operating_channel)) {
		hdd_err("SAP is already on DFS channel:%d",
			hdd_ap_ctx->operating_channel);
		return -EINVAL;
	}

	if (!WLAN_REG_IS_24GHZ_CH(hdd_ap_ctx->operating_channel)) {
		hdd_err("pre CAC alllowed only when SAP is in 2.4GHz:%d",
			hdd_ap_ctx->operating_channel);
		return -EINVAL;
	}

	mac_addr = wlan_hdd_get_intf_addr(hdd_ctx);
	if (!mac_addr) {
		hdd_err("can't add virtual intf: Not getting valid mac addr");
		return -EINVAL;
	}

	hdd_debug("channel:%d", channel);

	ret = wlan_hdd_validate_and_get_pre_cac_ch(hdd_ctx, ap_adapter, channel,
							&pre_cac_chan);
	if (ret != 0) {
		hdd_err("can't validate pre-cac channel");
		goto release_intf_addr_and_return_failure;
	}

	hdd_debug("starting pre cac SAP  adapter");

	/* Starting a SAP adapter:
	 * Instead of opening an adapter, we could just do a SME open session
	 * for AP type. But, start BSS would still need an adapter.
	 * So, this option is not taken.
	 *
	 * hdd open adapter is going to register this precac interface with
	 * user space. This interface though exposed to user space will be in
	 * DOWN state. Consideration was done to avoid this registration to the
	 * user space. But, as part of SAP operations multiple events are sent
	 * to user space. Some of these events received from unregistered
	 * interface was causing crashes. So, retaining the registration.
	 *
	 * So, this interface would remain registered and will remain in DOWN
	 * state for the CAC duration. We will add notes in the feature
	 * announcement to not use this temporary interface for any activity
	 * from user space.
	 */
	pre_cac_adapter = hdd_open_adapter(hdd_ctx, QDF_SAP_MODE, "precac%d",
					   mac_addr, NET_NAME_UNKNOWN, true);
	if (!pre_cac_adapter) {
		hdd_err("error opening the pre cac adapter");
		goto release_intf_addr_and_return_failure;
	}

	/*
	 * This interface is internally created by the driver. So, no interface
	 * up comes for this interface from user space and hence starting
	 * the adapter internally.
	 */
	if (hdd_start_adapter(pre_cac_adapter)) {
		hdd_err("error starting the pre cac adapter");
		goto close_pre_cac_adapter;
	}

	hdd_debug("preparing for start ap/bss on the pre cac adapter");

	wiphy = hdd_ctx->wiphy;
	dev = pre_cac_adapter->dev;

	/* Since this is only a dummy interface lets us use the IEs from the
	 * other active SAP interface. In regular scenarios, these IEs would
	 * come from the user space entity
	 */
	pre_cac_adapter->session.ap.beacon = qdf_mem_malloc(
			sizeof(*ap_adapter->session.ap.beacon));
	if (!pre_cac_adapter->session.ap.beacon) {
		hdd_err("failed to alloc mem for beacon");
		goto stop_close_pre_cac_adapter;
	}
	qdf_mem_copy(pre_cac_adapter->session.ap.beacon,
			ap_adapter->session.ap.beacon,
			sizeof(*pre_cac_adapter->session.ap.beacon));
	pre_cac_adapter->session.ap.sap_config.ch_width_orig =
			ap_adapter->session.ap.sap_config.ch_width_orig;
	pre_cac_adapter->session.ap.sap_config.authType =
			ap_adapter->session.ap.sap_config.authType;

	/* Premise is that on moving from 2.4GHz to 5GHz, the SAP will continue
	 * to operate on the same bandwidth as that of the 2.4GHz operations.
	 * Only bandwidths 20MHz/40MHz are possible on 2.4GHz band.
	 */
	switch (ap_adapter->session.ap.sap_config.ch_width_orig) {
	case CH_WIDTH_20MHZ:
		channel_type = NL80211_CHAN_HT20;
		break;
	case CH_WIDTH_40MHZ:
		if (ap_adapter->session.ap.sap_config.sec_ch >
				ap_adapter->session.ap.sap_config.channel)
			channel_type = NL80211_CHAN_HT40PLUS;
		else
			channel_type = NL80211_CHAN_HT40MINUS;
		break;
	default:
		channel_type = NL80211_CHAN_NO_HT;
		break;
	}

	freq = cds_chan_to_freq(pre_cac_chan);
	chan = ieee80211_get_channel(wiphy, freq);
	if (!chan) {
		hdd_err("channel converion failed");
		goto stop_close_pre_cac_adapter;
	}

	cfg80211_chandef_create(&chandef, chan, channel_type);

	hdd_debug("orig width:%d channel_type:%d freq:%d",
		ap_adapter->session.ap.sap_config.ch_width_orig,
		channel_type, freq);
	/*
	 * Doing update after opening and starting pre-cac adapter will make
	 * sure that driver won't do hardware mode change if there are any
	 * initial hick-ups or issues in pre-cac adapter's configuration.
	 * Since current SAP is in 2.4GHz and pre CAC channel is in 5GHz, this
	 * connection update should result in DBS mode
	 */
	status = policy_mgr_update_and_wait_for_connection_update(
						hdd_ctx->hdd_psoc,
						ap_adapter->session_id,
						pre_cac_chan,
						SIR_UPDATE_REASON_PRE_CAC);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("error in moving to DBS mode");
		goto stop_close_pre_cac_adapter;
	}


	ret = wlan_hdd_set_channel(wiphy, dev, &chandef, channel_type);
	if (0 != ret) {
		hdd_err("failed to set channel");
		goto stop_close_pre_cac_adapter;
	}

	status = wlan_hdd_cfg80211_start_bss(pre_cac_adapter, NULL,
			PRE_CAC_SSID, qdf_str_len(PRE_CAC_SSID),
			NL80211_HIDDEN_SSID_NOT_IN_USE, false);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("start bss failed");
		goto stop_close_pre_cac_adapter;
	}

	/*
	 * The pre cac status is set here. But, it would not be reset explicitly
	 * anywhere, since after the pre cac success/failure, the pre cac
	 * adapter itself would be removed.
	 */
	ret = wlan_hdd_set_pre_cac_status(pre_cac_adapter, true, handle);
	if (0 != ret) {
		hdd_err("failed to set pre cac status");
		goto stop_close_pre_cac_adapter;
	}

	ret = wlan_hdd_set_chan_before_pre_cac(ap_adapter,
				hdd_ap_ctx->operating_channel);
	if (0 != ret) {
		hdd_err("failed to set channel before pre cac");
		goto stop_close_pre_cac_adapter;
	}

	ap_adapter->pre_cac_chan = pre_cac_chan;

	return 0;

stop_close_pre_cac_adapter:
	hdd_stop_adapter(hdd_ctx, pre_cac_adapter);
	qdf_mem_free(pre_cac_adapter->session.ap.beacon);
	pre_cac_adapter->session.ap.beacon = NULL;
close_pre_cac_adapter:
	hdd_close_adapter(hdd_ctx, pre_cac_adapter, false);
release_intf_addr_and_return_failure:
	/*
	 * Release the interface address as the adapter
	 * failed to start, if you don't release then next
	 * adapter which is trying to come wouldn't get valid
	 * mac address. Remember we have limited pool of mac addresses
	 */
	wlan_hdd_release_intf_addr(hdd_ctx, mac_addr);
	return -EINVAL;
}

static const struct nla_policy
wlan_hdd_sap_config_policy[QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_CHANNEL] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_SAP_MANDATORY_FREQUENCY_LIST] = {
							.type = NLA_NESTED },
};

static const struct nla_policy
wlan_hdd_set_acs_dfs_config_policy[QCA_WLAN_VENDOR_ATTR_ACS_DFS_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_ACS_DFS_MODE] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_ACS_CHANNEL_HINT] = {.type = NLA_U8 },
};

static const struct nla_policy
wlan_hdd_set_limit_off_channel_param_policy
[QCA_WLAN_VENDOR_ATTR_ACTIVE_TOS_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_ACTIVE_TOS] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_ACTIVE_TOS_START] = {.type = NLA_U8 },
};

/**
 * __wlan_hdd_cfg80211_acs_dfs_mode() - set ACS DFS mode and channel
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Length of @data
 *
 * This function parses the incoming NL vendor command data attributes and
 * updates the SAP context about channel_hint and DFS mode.
 * If channel_hint is set, SAP will choose that channel
 * as operating channel.
 *
 * If DFS mode is enabled, driver will include DFS channels
 * in ACS else driver will skip DFS channels.
 *
 * Return: 0 on success, negative errno on failure
 */
static int
__wlan_hdd_cfg80211_acs_dfs_mode(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int data_len)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_ACS_DFS_MAX + 1];
	int ret;
	struct acs_dfs_policy *acs_policy;
	int mode = DFS_MODE_NONE;
	int channel_hint = 0;

	ENTER_DEV(wdev->netdev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	if (wlan_cfg80211_nla_parse(tb, QCA_WLAN_VENDOR_ATTR_ACS_DFS_MAX,
				    data, data_len,
				    wlan_hdd_set_acs_dfs_config_policy)) {
		hdd_err("invalid attr");
		return -EINVAL;
	}

	acs_policy = &hdd_ctx->acs_policy;
	/*
	 * SCM sends this attribute to restrict SAP from choosing
	 * DFS channels from ACS.
	 */
	if (tb[QCA_WLAN_VENDOR_ATTR_ACS_DFS_MODE])
		mode = nla_get_u8(tb[QCA_WLAN_VENDOR_ATTR_ACS_DFS_MODE]);

	if (!IS_DFS_MODE_VALID(mode)) {
		hdd_err("attr acs dfs mode is not valid");
		return -EINVAL;
	}
	acs_policy->acs_dfs_mode = mode;

	/*
	 * SCM sends this attribute to provide an active channel,
	 * to skip redundant ACS between drivers, and save driver start up time
	 */
	if (tb[QCA_WLAN_VENDOR_ATTR_ACS_CHANNEL_HINT])
		channel_hint = nla_get_u8(
				tb[QCA_WLAN_VENDOR_ATTR_ACS_CHANNEL_HINT]);

	if (!IS_CHANNEL_VALID(channel_hint)) {
		hdd_err("acs channel is not valid");
		return -EINVAL;
	}
	acs_policy->acs_channel = channel_hint;

	return 0;
}

/**
 * wlan_hdd_cfg80211_acs_dfs_mode() - Wrapper to set ACS DFS mode
 * @wiphy:    wiphy structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of @data
 *
 * This function parses the incoming NL vendor command data attributes and
 * updates the SAP context about channel_hint and DFS mode.
 *
 * Return: 0 on success; errno on failure
 */
static int wlan_hdd_cfg80211_acs_dfs_mode(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_acs_dfs_mode(wiphy, wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * wlan_hdd_get_sta_roam_dfs_mode() - get sta roam dfs mode policy
 * @mode : cfg80211 dfs mode
 *
 * Return: return csr sta roam dfs mode else return NONE
 */
static enum sta_roam_policy_dfs_mode wlan_hdd_get_sta_roam_dfs_mode(
		enum dfs_mode mode)
{
	switch (mode) {
	case DFS_MODE_ENABLE:
		return CSR_STA_ROAM_POLICY_DFS_ENABLED;
	case DFS_MODE_DISABLE:
		return CSR_STA_ROAM_POLICY_DFS_DISABLED;
	case DFS_MODE_DEPRIORITIZE:
		return CSR_STA_ROAM_POLICY_DFS_DEPRIORITIZE;
	default:
		hdd_err("STA Roam policy dfs mode is NONE");
		return  CSR_STA_ROAM_POLICY_NONE;
	}
}

/*
 * hdd_get_sap_operating_band:  Get current operating channel
 * for sap.
 * @hdd_ctx: hdd context
 *
 * Return : Corresponding band for SAP operating channel
 */
uint8_t hdd_get_sap_operating_band(struct hdd_context *hdd_ctx)
{
	struct hdd_adapter *adapter;
	uint8_t  operating_channel = 0;
	uint8_t sap_operating_band = 0;

	hdd_for_each_adapter(hdd_ctx, adapter) {
		if (adapter->device_mode != QDF_SAP_MODE)
			continue;

		operating_channel = adapter->session.ap.operating_channel;
		if (IS_24G_CH(operating_channel))
			sap_operating_band = BAND_2G;
		else if (IS_5G_CH(operating_channel))
			sap_operating_band = BAND_5G;
		else
			sap_operating_band = BAND_ALL;
	}

	return sap_operating_band;
}

static const struct nla_policy
wlan_hdd_set_sta_roam_config_policy[
QCA_WLAN_VENDOR_ATTR_STA_CONNECT_ROAM_POLICY_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_STA_DFS_MODE] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_STA_SKIP_UNSAFE_CHANNEL] = {.type = NLA_U8 },
};

/**
 * __wlan_hdd_cfg80211_sta_roam_policy() - Set params to restrict scan channels
 * for station connection or roaming.
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Length of @data
 *
 * __wlan_hdd_cfg80211_sta_roam_policy will decide if DFS channels or unsafe
 * channels needs to be skipped in scanning or not.
 * If dfs_mode is disabled, driver will not scan DFS channels.
 * If skip_unsafe_channels is set, driver will skip unsafe channels
 * in Scanning.
 *
 * Return: 0 on success, negative errno on failure
 */
static int
__wlan_hdd_cfg80211_sta_roam_policy(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int data_len)
{
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct nlattr *tb[
		QCA_WLAN_VENDOR_ATTR_STA_CONNECT_ROAM_POLICY_MAX + 1];
	int ret;
	enum sta_roam_policy_dfs_mode sta_roam_dfs_mode;
	enum dfs_mode mode = DFS_MODE_NONE;
	bool skip_unsafe_channels = false;
	QDF_STATUS status;
	uint8_t sap_operating_band;

	ENTER_DEV(dev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;
	if (wlan_cfg80211_nla_parse(tb,
			       QCA_WLAN_VENDOR_ATTR_STA_CONNECT_ROAM_POLICY_MAX,
			       data, data_len,
			       wlan_hdd_set_sta_roam_config_policy)) {
		hdd_err("invalid attr");
		return -EINVAL;
	}
	if (tb[QCA_WLAN_VENDOR_ATTR_STA_DFS_MODE])
		mode = nla_get_u8(tb[QCA_WLAN_VENDOR_ATTR_STA_DFS_MODE]);
	if (!IS_DFS_MODE_VALID(mode)) {
		hdd_err("attr sta roam dfs mode policy is not valid");
		return -EINVAL;
	}

	sta_roam_dfs_mode = wlan_hdd_get_sta_roam_dfs_mode(mode);

	if (tb[QCA_WLAN_VENDOR_ATTR_STA_SKIP_UNSAFE_CHANNEL])
		skip_unsafe_channels = nla_get_u8(
			tb[QCA_WLAN_VENDOR_ATTR_STA_SKIP_UNSAFE_CHANNEL]);
	sap_operating_band = hdd_get_sap_operating_band(hdd_ctx);
	status = sme_update_sta_roam_policy(hdd_ctx->hHal, sta_roam_dfs_mode,
			skip_unsafe_channels, adapter->session_id,
			sap_operating_band);

	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("sme_update_sta_roam_policy (err=%d)", status);
		return -EINVAL;
	}
	return 0;
}

/**
 * wlan_hdd_cfg80211_sta_roam_policy() - Wrapper to restrict scan channels,
 * connection and roaming for station.
 * @wiphy:    wiphy structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of @data
 *
 * __wlan_hdd_cfg80211_sta_roam_policy will decide if DFS channels or unsafe
 * channels needs to be skipped in scanning or not.
 * If dfs_mode is disabled, driver will not scan DFS channels.
 * If skip_unsafe_channels is set, driver will skip unsafe channels
 * in Scanning.
 * Return: 0 on success; errno on failure
 */
static int wlan_hdd_cfg80211_sta_roam_policy(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_sta_roam_policy(wiphy, wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

#ifdef FEATURE_WLAN_CH_AVOID

static int hdd_validate_avoid_freq_chanlist(
					struct hdd_context *hdd_ctx,
					struct ch_avoid_ind_type *channel_list)
{
	unsigned int range_idx, ch_idx;
	unsigned int unsafe_channel_index, unsafe_channel_count = 0;
	bool ch_found = false;

	unsafe_channel_count = QDF_MIN((uint16_t)hdd_ctx->unsafe_channel_count,
				       (uint16_t)NUM_CHANNELS);

	for (range_idx = 0; range_idx < channel_list->ch_avoid_range_cnt;
					range_idx++) {
		if ((channel_list->avoid_freq_range[range_idx].start_freq <
		     CDS_24_GHZ_CHANNEL_1) ||
		    (channel_list->avoid_freq_range[range_idx].end_freq >
		     CDS_5_GHZ_CHANNEL_165) ||
		    (channel_list->avoid_freq_range[range_idx].start_freq >
		     channel_list->avoid_freq_range[range_idx].end_freq))
			continue;

		for (ch_idx = channel_list->
				avoid_freq_range[range_idx].start_freq;
		     ch_idx <= channel_list->
					avoid_freq_range[range_idx].end_freq;
		     ch_idx++) {
			for (unsafe_channel_index = 0;
			     unsafe_channel_index < unsafe_channel_count;
			     unsafe_channel_index++) {
				if (ch_idx ==
					hdd_ctx->unsafe_channel_list[
					unsafe_channel_index]) {
					hdd_log(QDF_TRACE_LEVEL_INFO,
						"Duplicate channel %d",
						ch_idx);
					ch_found = true;
					break;
				}
			}
			if (!ch_found) {
				hdd_ctx->unsafe_channel_list[
				unsafe_channel_count++] = ch_idx;
			}
			ch_found = false;
		}
	}
	return unsafe_channel_count;
}

/**
 * __wlan_hdd_cfg80211_avoid_freq() - ask driver to restart SAP if SAP
 * is on unsafe channel.
 * @wiphy:    wiphy structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of @data
 *
 * wlan_hdd_cfg80211_avoid_freq do restart the sap if sap is already
 * on any of unsafe channels.
 * If sap is on any of unsafe channel, hdd_unsafe_channel_restart_sap
 * will send WLAN_SVC_LTE_COEX_IND indication to userspace to restart.
 *
 * Return: 0 on success; errno on failure
 */
static int
__wlan_hdd_cfg80211_avoid_freq(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int data_len)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	int ret;
	qdf_device_t qdf_ctx = cds_get_context(QDF_MODULE_ID_QDF_DEVICE);
	uint16_t *local_unsafe_list;
	uint16_t unsafe_channel_index, local_unsafe_list_count;
	struct ch_avoid_ind_type *channel_list;
	enum tQDF_GLOBAL_CON_MODE curr_mode;

	ENTER_DEV(wdev->netdev);

	if (!qdf_ctx) {
		hdd_err("qdf_ctx is NULL");
		return -EINVAL;
	}
	curr_mode = hdd_get_conparam();
	if (QDF_GLOBAL_FTM_MODE == curr_mode ||
	    QDF_GLOBAL_MONITOR_MODE == curr_mode) {
		hdd_err("Command not allowed in FTM/MONITOR mode");
		return -EINVAL;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	channel_list = (struct ch_avoid_ind_type *)data;
	if (!channel_list) {
		hdd_log(QDF_TRACE_LEVEL_ERROR,
			"Avoid frequency channel list empty");
		return -EINVAL;
	}

	ret = hdd_clone_local_unsafe_chan(hdd_ctx,
					  &local_unsafe_list,
					  &local_unsafe_list_count);
	if (0 != ret) {
		hdd_err("failed to clone the cur unsafe chan list");
		return ret;
	}

	pld_get_wlan_unsafe_channel(qdf_ctx->dev, hdd_ctx->unsafe_channel_list,
			&(hdd_ctx->unsafe_channel_count),
			sizeof(hdd_ctx->unsafe_channel_list));

	hdd_ctx->unsafe_channel_count = hdd_validate_avoid_freq_chanlist(
								hdd_ctx,
								channel_list);

	pld_set_wlan_unsafe_channel(qdf_ctx->dev, hdd_ctx->unsafe_channel_list,
				    hdd_ctx->unsafe_channel_count);

	for (unsafe_channel_index = 0;
	     unsafe_channel_index < hdd_ctx->unsafe_channel_count;
	     unsafe_channel_index++) {
		hdd_debug("Channel %d is not safe",
			  hdd_ctx->unsafe_channel_list[unsafe_channel_index]);
	}
	if (hdd_local_unsafe_channel_updated(hdd_ctx, local_unsafe_list,
					     local_unsafe_list_count))
		hdd_unsafe_channel_restart_sap(hdd_ctx);
	qdf_mem_free(local_unsafe_list);

	return 0;
}

/**
 * wlan_hdd_cfg80211_avoid_freq() - ask driver to restart SAP if SAP
 * is on unsafe channel.
 * @wiphy:    wiphy structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of @data
 *
 * wlan_hdd_cfg80211_avoid_freq do restart the sap if sap is already
 * on any of unsafe channels.
 * If sap is on any of unsafe channel, hdd_unsafe_channel_restart_sap
 * will send WLAN_SVC_LTE_COEX_IND indication to userspace to restart.
 *
 * Return: 0 on success; errno on failure
 */
static int wlan_hdd_cfg80211_avoid_freq(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_avoid_freq(wiphy, wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

#endif
/**
 * __wlan_hdd_cfg80211_sap_configuration_set() - ask driver to restart SAP if
 * SAP is on unsafe channel.
 * @wiphy:    wiphy structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of @data
 *
 * __wlan_hdd_cfg80211_sap_configuration_set function set SAP params to
 * driver.
 * QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_CHAN will set sap config channel and
 * will initiate restart of sap.
 *
 * Return: 0 on success; errno on failure
 */
static int
__wlan_hdd_cfg80211_sap_configuration_set(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int data_len)
{
	struct net_device *ndev = wdev->netdev;
	struct hdd_adapter *hostapd_adapter = WLAN_HDD_GET_PRIV_PTR(ndev);
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_MAX + 1];
	uint8_t config_channel = 0;
	struct hdd_ap_ctx *ap_ctx;
	int ret;
	QDF_STATUS status;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return -EINVAL;

	if (wlan_cfg80211_nla_parse(tb, QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_MAX,
				    data, data_len,
				    wlan_hdd_sap_config_policy)) {
		hdd_err("invalid attr");
		return -EINVAL;
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_CHANNEL]) {
		if (!test_bit(SOFTAP_BSS_STARTED,
					&hostapd_adapter->event_flags)) {
			hdd_err("SAP is not started yet. Restart sap will be invalid");
			return -EINVAL;
		}

		config_channel =
			nla_get_u8(tb[QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_CHANNEL]);

		if (!((IS_24G_CH(config_channel)) ||
			(IS_5G_CH(config_channel)))) {
			hdd_err("Channel  %d is not valid to restart SAP",
					config_channel);
			return -ENOTSUPP;
		}

		ap_ctx = WLAN_HDD_GET_AP_CTX_PTR(hostapd_adapter);
		ap_ctx->sap_config.channel = config_channel;
		ap_ctx->sap_config.ch_params.ch_width =
					ap_ctx->sap_config.ch_width_orig;
		ap_ctx->bss_stop_reason = BSS_STOP_DUE_TO_VENDOR_CONFIG_CHAN;

		wlan_reg_set_channel_params(hdd_ctx->hdd_pdev,
				ap_ctx->sap_config.channel,
				ap_ctx->sap_config.sec_ch,
				&ap_ctx->sap_config.ch_params);

		hdd_restart_sap(hostapd_adapter);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_SAP_MANDATORY_FREQUENCY_LIST]) {
		uint32_t freq_len, i;
		uint32_t *freq;
		uint8_t chans[QDF_MAX_NUM_CHAN];

		hdd_debug("setting mandatory freq/chan list");

		freq_len = nla_len(
		    tb[QCA_WLAN_VENDOR_ATTR_SAP_MANDATORY_FREQUENCY_LIST])/
		    sizeof(uint32_t);

		if (freq_len > QDF_MAX_NUM_CHAN) {
			hdd_err("insufficient space to hold channels");
			return -ENOMEM;
		}

		freq = nla_data(
		    tb[QCA_WLAN_VENDOR_ATTR_SAP_MANDATORY_FREQUENCY_LIST]);

		hdd_debug("freq_len=%d", freq_len);

		for (i = 0; i < freq_len; i++) {
			chans[i] = ieee80211_frequency_to_channel(freq[i]);
			hdd_debug("freq[%d]=%d", i, freq[i]);
		}

		status = policy_mgr_set_sap_mandatory_channels(
			hdd_ctx->hdd_psoc, chans, freq_len);
		if (QDF_IS_STATUS_ERROR(status))
			return -EINVAL;
	}

	return 0;
}

/**
 * wlan_hdd_cfg80211_sap_configuration_set() - sap configuration vendor command
 * @wiphy:    wiphy structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of @data
 *
 * __wlan_hdd_cfg80211_sap_configuration_set function set SAP params to
 * driver.
 * QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_CHAN will set sap config channel and
 * will initiate restart of sap.
 *
 * Return: 0 on success; errno on failure
 */
static int wlan_hdd_cfg80211_sap_configuration_set(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_sap_configuration_set(wiphy,
			wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

#undef BPF_INVALID
#undef BPF_SET_RESET
#undef BPF_VERSION
#undef BPF_ID
#undef BPF_PACKET_SIZE
#undef BPF_CURRENT_OFFSET
#undef BPF_PROGRAM
#undef BPF_MAX

/**
 * define short names for the global vendor params
 * used by wlan_hdd_cfg80211_wakelock_stats_rsp_callback()
 */
#define PARAM_TOTAL_CMD_EVENT_WAKE \
		QCA_WLAN_VENDOR_ATTR_TOTAL_CMD_EVENT_WAKE
#define PARAM_CMD_EVENT_WAKE_CNT_PTR \
		QCA_WLAN_VENDOR_ATTR_CMD_EVENT_WAKE_CNT_PTR
#define PARAM_CMD_EVENT_WAKE_CNT_SZ \
		QCA_WLAN_VENDOR_ATTR_CMD_EVENT_WAKE_CNT_SZ
#define PARAM_TOTAL_DRIVER_FW_LOCAL_WAKE \
		QCA_WLAN_VENDOR_ATTR_TOTAL_DRIVER_FW_LOCAL_WAKE
#define PARAM_DRIVER_FW_LOCAL_WAKE_CNT_PTR \
		QCA_WLAN_VENDOR_ATTR_DRIVER_FW_LOCAL_WAKE_CNT_PTR
#define PARAM_DRIVER_FW_LOCAL_WAKE_CNT_SZ \
		QCA_WLAN_VENDOR_ATTR_DRIVER_FW_LOCAL_WAKE_CNT_SZ
#define PARAM_TOTAL_RX_DATA_WAKE \
		QCA_WLAN_VENDOR_ATTR_TOTAL_RX_DATA_WAKE
#define PARAM_RX_UNICAST_CNT \
		QCA_WLAN_VENDOR_ATTR_RX_UNICAST_CNT
#define PARAM_RX_MULTICAST_CNT \
		QCA_WLAN_VENDOR_ATTR_RX_MULTICAST_CNT
#define PARAM_RX_BROADCAST_CNT \
		QCA_WLAN_VENDOR_ATTR_RX_BROADCAST_CNT
#define PARAM_ICMP_PKT \
		QCA_WLAN_VENDOR_ATTR_ICMP_PKT
#define PARAM_ICMP6_PKT \
		QCA_WLAN_VENDOR_ATTR_ICMP6_PKT
#define PARAM_ICMP6_RA \
		QCA_WLAN_VENDOR_ATTR_ICMP6_RA
#define PARAM_ICMP6_NA \
		QCA_WLAN_VENDOR_ATTR_ICMP6_NA
#define PARAM_ICMP6_NS \
		QCA_WLAN_VENDOR_ATTR_ICMP6_NS
#define PARAM_ICMP4_RX_MULTICAST_CNT \
		QCA_WLAN_VENDOR_ATTR_ICMP4_RX_MULTICAST_CNT
#define PARAM_ICMP6_RX_MULTICAST_CNT \
		QCA_WLAN_VENDOR_ATTR_ICMP6_RX_MULTICAST_CNT
#define PARAM_OTHER_RX_MULTICAST_CNT \
		QCA_WLAN_VENDOR_ATTR_OTHER_RX_MULTICAST_CNT
#define PARAM_RSSI_BREACH_CNT \
		QCA_WLAN_VENDOR_ATTR_RSSI_BREACH_CNT
#define PARAM_LOW_RSSI_CNT \
		QCA_WLAN_VENDOR_ATTR_LOW_RSSI_CNT
#define PARAM_GSCAN_CNT \
		QCA_WLAN_VENDOR_ATTR_GSCAN_CNT
#define PARAM_PNO_COMPLETE_CNT \
		QCA_WLAN_VENDOR_ATTR_PNO_COMPLETE_CNT
#define PARAM_PNO_MATCH_CNT \
		QCA_WLAN_VENDOR_ATTR_PNO_MATCH_CNT



/**
 * hdd_send_wakelock_stats() - API to send wakelock stats
 * @ctx: context to be passed to callback
 * @data: data passed to callback
 *
 * This function is used to send wake lock stats to HAL layer
 *
 * Return: 0 on success, error number otherwise.
 */
static uint32_t hdd_send_wakelock_stats(struct hdd_context *hdd_ctx,
					const struct sir_wake_lock_stats *data)
{
	struct sk_buff *skb;
	uint32_t nl_buf_len;
	uint32_t total_rx_data_wake, rx_multicast_cnt;
	uint32_t ipv6_rx_multicast_addr_cnt;
	uint32_t icmpv6_cnt;

	ENTER();

	nl_buf_len = NLMSG_HDRLEN;
	nl_buf_len +=
		QCA_WLAN_VENDOR_GET_WAKE_STATS_MAX *
				(NLMSG_HDRLEN + sizeof(uint32_t));

	skb = cfg80211_vendor_cmd_alloc_reply_skb(hdd_ctx->wiphy, nl_buf_len);

	if (!skb) {
		hdd_err("cfg80211_vendor_cmd_alloc_reply_skb failed");
		return -ENOMEM;
	}

	hdd_debug("wow_ucast_wake_up_count %d",
			data->wow_ucast_wake_up_count);
	hdd_debug("wow_bcast_wake_up_count %d",
			data->wow_bcast_wake_up_count);
	hdd_debug("wow_ipv4_mcast_wake_up_count %d",
			data->wow_ipv4_mcast_wake_up_count);
	hdd_debug("wow_ipv6_mcast_wake_up_count %d",
			data->wow_ipv6_mcast_wake_up_count);
	hdd_debug("wow_ipv6_mcast_ra_stats %d",
			data->wow_ipv6_mcast_ra_stats);
	hdd_debug("wow_ipv6_mcast_ns_stats %d",
			data->wow_ipv6_mcast_ns_stats);
	hdd_debug("wow_ipv6_mcast_na_stats %d",
			data->wow_ipv6_mcast_na_stats);
	hdd_debug("wow_icmpv4_count %d", data->wow_icmpv4_count);
	hdd_debug("wow_icmpv6_count %d",
			data->wow_icmpv6_count);
	hdd_debug("wow_rssi_breach_wake_up_count %d",
			data->wow_rssi_breach_wake_up_count);
	hdd_debug("wow_low_rssi_wake_up_count %d",
			data->wow_low_rssi_wake_up_count);
	hdd_debug("wow_gscan_wake_up_count %d",
			data->wow_gscan_wake_up_count);
	hdd_debug("wow_pno_complete_wake_up_count %d",
			data->wow_pno_complete_wake_up_count);
	hdd_debug("wow_pno_match_wake_up_count %d",
			data->wow_pno_match_wake_up_count);

	ipv6_rx_multicast_addr_cnt =
		data->wow_ipv6_mcast_wake_up_count;

	icmpv6_cnt =
		data->wow_icmpv6_count;

	rx_multicast_cnt =
		data->wow_ipv4_mcast_wake_up_count +
		ipv6_rx_multicast_addr_cnt;

	total_rx_data_wake =
		data->wow_ucast_wake_up_count +
		data->wow_bcast_wake_up_count +
		rx_multicast_cnt;

	if (nla_put_u32(skb, PARAM_TOTAL_CMD_EVENT_WAKE, 0) ||
	    nla_put_u32(skb, PARAM_CMD_EVENT_WAKE_CNT_PTR, 0) ||
	    nla_put_u32(skb, PARAM_CMD_EVENT_WAKE_CNT_SZ, 0) ||
	    nla_put_u32(skb, PARAM_TOTAL_DRIVER_FW_LOCAL_WAKE, 0) ||
	    nla_put_u32(skb, PARAM_DRIVER_FW_LOCAL_WAKE_CNT_PTR, 0) ||
	    nla_put_u32(skb, PARAM_DRIVER_FW_LOCAL_WAKE_CNT_SZ, 0) ||
	    nla_put_u32(skb, PARAM_TOTAL_RX_DATA_WAKE,
				total_rx_data_wake) ||
	    nla_put_u32(skb, PARAM_RX_UNICAST_CNT,
				data->wow_ucast_wake_up_count) ||
	    nla_put_u32(skb, PARAM_RX_MULTICAST_CNT,
				rx_multicast_cnt) ||
	    nla_put_u32(skb, PARAM_RX_BROADCAST_CNT,
				data->wow_bcast_wake_up_count) ||
	    nla_put_u32(skb, PARAM_ICMP_PKT,
				data->wow_icmpv4_count) ||
	    nla_put_u32(skb, PARAM_ICMP6_PKT,
				icmpv6_cnt) ||
	    nla_put_u32(skb, PARAM_ICMP6_RA,
				data->wow_ipv6_mcast_ra_stats) ||
	    nla_put_u32(skb, PARAM_ICMP6_NA,
				data->wow_ipv6_mcast_na_stats) ||
	    nla_put_u32(skb, PARAM_ICMP6_NS,
				data->wow_ipv6_mcast_ns_stats) ||
	    nla_put_u32(skb, PARAM_ICMP4_RX_MULTICAST_CNT,
				data->wow_ipv4_mcast_wake_up_count) ||
	    nla_put_u32(skb, PARAM_ICMP6_RX_MULTICAST_CNT,
				ipv6_rx_multicast_addr_cnt) ||
	    nla_put_u32(skb, PARAM_OTHER_RX_MULTICAST_CNT, 0) ||
	    nla_put_u32(skb, PARAM_RSSI_BREACH_CNT,
				data->wow_rssi_breach_wake_up_count) ||
	    nla_put_u32(skb, PARAM_LOW_RSSI_CNT,
				data->wow_low_rssi_wake_up_count) ||
	    nla_put_u32(skb, PARAM_GSCAN_CNT,
				data->wow_gscan_wake_up_count) ||
	    nla_put_u32(skb, PARAM_PNO_COMPLETE_CNT,
				data->wow_pno_complete_wake_up_count) ||
	    nla_put_u32(skb, PARAM_PNO_MATCH_CNT,
				data->wow_pno_match_wake_up_count)) {
		hdd_err("nla put fail");
		goto nla_put_failure;
	}

	cfg80211_vendor_cmd_reply(skb);

	EXIT();
	return 0;

nla_put_failure:
	kfree_skb(skb);
	return -EINVAL;
}

/**
 * __wlan_hdd_cfg80211_get_wakelock_stats() - gets wake lock stats
 * @wiphy: wiphy pointer
 * @wdev: pointer to struct wireless_dev
 * @data: pointer to incoming NL vendor data
 * @data_len: length of @data
 *
 * This function parses the incoming NL vendor command data attributes and
 * invokes the SME Api and blocks on a completion variable.
 * WMA copies required data and invokes callback
 * wlan_hdd_cfg80211_wakelock_stats_rsp_callback to send wake lock stats.
 *
 * Return: 0 on success; error number otherwise.
 */
static int __wlan_hdd_cfg80211_get_wakelock_stats(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					const void *data,
					int data_len)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	int status, ret;
	struct sir_wake_lock_stats wake_lock_stats;
	QDF_STATUS qdf_status;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	status = wlan_hdd_validate_context(hdd_ctx);
	if (0 != status)
		return -EINVAL;

	qdf_status = wma_get_wakelock_stats(&wake_lock_stats);
	if (qdf_status != QDF_STATUS_SUCCESS) {
		hdd_err("failed to get wakelock stats(err=%d)", qdf_status);
		return -EINVAL;
	}

	ret = hdd_send_wakelock_stats(hdd_ctx,
					&wake_lock_stats);
	if (ret)
		hdd_err("Failed to post wake lock stats");

	EXIT();
	return ret;
}

/**
 * wlan_hdd_cfg80211_get_wakelock_stats() - gets wake lock stats
 * @wiphy: wiphy pointer
 * @wdev: pointer to struct wireless_dev
 * @data: pointer to incoming NL vendor data
 * @data_len: length of @data
 *
 * This function parses the incoming NL vendor command data attributes and
 * invokes the SME Api and blocks on a completion variable.
 * WMA copies required data and invokes callback
 * wlan_hdd_cfg80211_wakelock_stats_rsp_callback to send wake lock stats.
 *
 * Return: 0 on success; error number otherwise.
 */
static int wlan_hdd_cfg80211_get_wakelock_stats(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data, int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_get_wakelock_stats(wiphy, wdev, data,
								data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __wlan_hdd_cfg80211_get_bus_size() - Get WMI Bus size
 * @wiphy:    wiphy structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of @data
 *
 * This function reads wmi max bus size and fill in the skb with
 * NL attributes and send up the NL event.
 * Return: 0 on success; errno on failure
 */
static int
__wlan_hdd_cfg80211_get_bus_size(struct wiphy *wiphy,
				 struct wireless_dev *wdev,
				 const void *data, int data_len)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	int ret_val;
	struct sk_buff *skb;
	uint32_t nl_buf_len;

	ENTER();

	ret_val = wlan_hdd_validate_context(hdd_ctx);
	if (ret_val)
		return ret_val;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	hdd_debug("WMI Max Bus size: %d", hdd_ctx->wmi_max_len);

	nl_buf_len = NLMSG_HDRLEN;
	nl_buf_len +=  (sizeof(hdd_ctx->wmi_max_len) + NLA_HDRLEN);

	skb = cfg80211_vendor_cmd_alloc_reply_skb(hdd_ctx->wiphy, nl_buf_len);
	if (!skb) {
		hdd_err("cfg80211_vendor_cmd_alloc_reply_skb failed");
		return -ENOMEM;
	}

	if (nla_put_u16(skb, QCA_WLAN_VENDOR_ATTR_DRV_INFO_BUS_SIZE,
			hdd_ctx->wmi_max_len)) {
		hdd_err("nla put failure");
		goto nla_put_failure;
	}

	cfg80211_vendor_cmd_reply(skb);

	EXIT();

	return 0;

nla_put_failure:
	kfree_skb(skb);
	return -EINVAL;
}

/**
 * wlan_hdd_cfg80211_get_bus_size() - SSR Wrapper to Get Bus size
 * @wiphy:    wiphy structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of @data
 *
 * Return: 0 on success; errno on failure
 */
static int wlan_hdd_cfg80211_get_bus_size(struct wiphy *wiphy,
					  struct wireless_dev *wdev,
					  const void *data, int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_get_bus_size(wiphy, wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 *__wlan_hdd_cfg80211_setband() - set band
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Length of @data
 *
 * Return: 0 on success, negative errno on failure
 */
static int __wlan_hdd_cfg80211_setband(struct wiphy *wiphy,
				       struct wireless_dev *wdev,
				       const void *data, int data_len)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct net_device *dev = wdev->netdev;
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_MAX + 1];
	int ret;
	static const struct nla_policy policy[QCA_WLAN_VENDOR_ATTR_MAX + 1]
		= {[QCA_WLAN_VENDOR_ATTR_SETBAND_VALUE] = { .type = NLA_U32 } };

	ENTER();

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	if (wlan_cfg80211_nla_parse(tb, QCA_WLAN_VENDOR_ATTR_MAX,
				    data, data_len, policy)) {
		hdd_err(FL("Invalid ATTR"));
		return -EINVAL;
	}

	if (!tb[QCA_WLAN_VENDOR_ATTR_SETBAND_VALUE]) {
		hdd_err(FL("attr SETBAND_VALUE failed"));
		return -EINVAL;
	}

	ret = hdd_reg_set_band(dev,
		nla_get_u32(tb[QCA_WLAN_VENDOR_ATTR_SETBAND_VALUE]));

	EXIT();
	return ret;
}

/**
 *wlan_hdd_validate_acs_channel() - validate channel provided by ACS
 * @adapter: hdd adapter
 * @channel: channel number
 *
 * return: QDF status based on success or failure
 */
static QDF_STATUS wlan_hdd_validate_acs_channel(struct hdd_adapter *adapter,
						int channel, int chan_bw)
{
	if (QDF_STATUS_SUCCESS !=
	    wlan_hdd_validate_operation_channel(adapter, channel))
		return QDF_STATUS_E_FAILURE;
	if ((wlansap_is_channel_in_nol_list(WLAN_HDD_GET_SAP_CTX_PTR(adapter),
				channel,
				PHY_SINGLE_CHANNEL_CENTERED))) {
		hdd_notice("channel %d is in nol", channel);
		return -EINVAL;
	}

	if ((wlansap_is_channel_leaking_in_nol(
				WLAN_HDD_GET_SAP_CTX_PTR(adapter),
				channel, chan_bw))) {
		hdd_notice("channel %d is leaking in nol", channel);
		return -EINVAL;
	}

	return 0;

}

static void hdd_update_acs_sap_config(struct hdd_context *hdd_ctx,
				     tsap_Config_t *sap_config,
				     struct hdd_vendor_chan_info *channel_list)
{
	sap_config->channel = channel_list->pri_ch;

	sap_config->ch_params.center_freq_seg0 =
				channel_list->vht_seg0_center_ch;
	sap_config->ch_params.center_freq_seg1 =
				channel_list->vht_seg1_center_ch;

	sap_config->ch_params.sec_ch_offset = channel_list->ht_sec_ch;
	sap_config->ch_params.ch_width = channel_list->chan_width;
	if (sap_config->channel >= 36)
		sap_config->ch_width_orig =
				hdd_ctx->config->vhtChannelWidth;
	else
		sap_config->ch_width_orig =
			hdd_ctx->config->nChannelBondingMode24GHz ?
			eHT_CHANNEL_WIDTH_40MHZ :
			eHT_CHANNEL_WIDTH_20MHZ;

	sap_config->acs_cfg.pri_ch = channel_list->pri_ch;
	sap_config->acs_cfg.ch_width = channel_list->chan_width;
	sap_config->acs_cfg.vht_seg0_center_ch =
				channel_list->vht_seg0_center_ch;
	sap_config->acs_cfg.vht_seg1_center_ch =
				channel_list->vht_seg1_center_ch;
	sap_config->acs_cfg.ht_sec_ch = channel_list->ht_sec_ch;
}

static int hdd_update_acs_channel(struct hdd_adapter *adapter, uint8_t reason,
				  uint8_t channel_cnt,
				  struct hdd_vendor_chan_info *channel_list)
{
	tsap_Config_t *sap_config;
	struct hdd_ap_ctx *hdd_ap_ctx;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!channel_list) {
		hdd_err("channel_list is NULL");
		return -EINVAL;
	}

	hdd_ap_ctx = WLAN_HDD_GET_AP_CTX_PTR(adapter);
	sap_config = &adapter->session.ap.sap_config;

	if (QDF_TIMER_STATE_RUNNING ==
	    qdf_mc_timer_get_current_state(&adapter->session.
					ap.vendor_acs_timer)) {
		qdf_mc_timer_stop(&adapter->session.ap.vendor_acs_timer);
	}

	if (channel_list->pri_ch == 0) {
		/* Check mode, set default channel */
		channel_list->pri_ch = 6;
		/*
		 * sap_select_default_oper_chan(hdd_ctx->hHal,
		 *      sap_config->acs_cfg.hw_mode);
		 */
	}

	switch (reason) {
	/* SAP init case */
	case QCA_WLAN_VENDOR_ACS_SELECT_REASON_INIT:
		hdd_update_acs_sap_config(hdd_ctx, sap_config, channel_list);
		/* Update Hostapd */
		wlan_hdd_cfg80211_acs_ch_select_evt(adapter);
		break;

	/* DFS detected on current channel */
	case QCA_WLAN_VENDOR_ACS_SELECT_REASON_DFS:
		wlan_sap_update_next_channel(
				WLAN_HDD_GET_SAP_CTX_PTR(adapter),
				channel_list->pri_ch,
				channel_list->chan_width);
		status = sme_update_new_channel_event(
					WLAN_HDD_GET_HAL_CTX(adapter),
					adapter->session_id);
		break;

	/* LTE coex event on current channel */
	case QCA_WLAN_VENDOR_ACS_SELECT_REASON_LTE_COEX:
		sap_config->acs_cfg.pri_ch = channel_list->pri_ch;
		sap_config->acs_cfg.ch_width = channel_list->chan_width;
		hdd_ap_ctx->sap_config.ch_width_orig =
				channel_list->chan_width;
		hdd_switch_sap_channel(adapter, sap_config->acs_cfg.pri_ch,
				       true);
		break;

	default:
		hdd_info("invalid reason for timer invoke");
	}
	EXIT();
	return qdf_status_to_os_return(status);
}

/**
 * Define short name for vendor channel set config
 */
#define SET_CHAN_REASON QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_REASON
#define SET_CHAN_CHAN_LIST QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_LIST
#define SET_CHAN_PRIMARY_CHANNEL \
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_PRIMARY
#define SET_CHAN_SECONDARY_CHANNEL \
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_SECONDARY
#define SET_CHAN_SEG0_CENTER_CHANNEL \
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_CENTER_SEG0
#define	SET_CHAN_SEG1_CENTER_CHANNEL \
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_CENTER_SEG1
#define	SET_CHAN_CHANNEL_WIDTH \
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_WIDTH
#define	SET_CHAN_MAX QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_MAX
#define SET_EXT_ACS_BAND QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_BAND

/**
 * hdd_parse_vendor_acs_chan_config() - API to parse vendor acs channel config
 * @channel_list: pointer to hdd_vendor_chan_info
 * @reason: channel change reason
 * @channel_cnt: channel count
 * @data: data
 * @data_len: data len
 *
 * Return: 0 on success, negative errno on failure
 */
static int hdd_parse_vendor_acs_chan_config(struct hdd_vendor_chan_info
		**chan_list_ptr, uint8_t *reason, uint8_t *channel_cnt,
		const void *data, int data_len)
{
	int rem, i = 0;
	struct nlattr *tb[SET_CHAN_MAX + 1];
	struct nlattr *tb2[SET_CHAN_MAX + 1];
	struct nlattr *curr_attr;
	struct hdd_vendor_chan_info *channel_list;

	if (wlan_cfg80211_nla_parse(tb, SET_CHAN_MAX, data, data_len, NULL)) {
		hdd_err("Invalid ATTR");
		return -EINVAL;
	}

	if (tb[SET_CHAN_REASON])
		*reason = nla_get_u8(tb[SET_CHAN_REASON]);

	nla_for_each_nested(curr_attr, tb[SET_CHAN_CHAN_LIST], rem)
		i++;

	*channel_cnt = i;

	if (i == 0)
		hdd_err("incorrect channel count");

	channel_list = qdf_mem_malloc(sizeof(struct hdd_vendor_chan_info) *
					(*channel_cnt));
	if (NULL == channel_list) {
		hdd_err("Could not allocate for channel_list");
		return -ENOMEM;
	}

	i = 0;
	nla_for_each_nested(curr_attr, tb[SET_CHAN_CHAN_LIST], rem) {
		if (wlan_cfg80211_nla_parse(tb2, SET_CHAN_MAX,
					    nla_data(curr_attr),
					    nla_len(curr_attr), NULL)) {
			hdd_err("nla_parse failed");
			return -EINVAL;
		}
		if (tb2[SET_EXT_ACS_BAND]) {
			channel_list[i].band =
				nla_get_u8(tb2[SET_EXT_ACS_BAND]);
		}
		/* Parse and Fetch allowed SSID list*/
		if (tb2[SET_CHAN_PRIMARY_CHANNEL]) {
			channel_list[i].pri_ch =
				nla_get_u8(
					tb2[SET_CHAN_PRIMARY_CHANNEL]);
		}
		if (tb2[SET_CHAN_SECONDARY_CHANNEL]) {
			channel_list[i].ht_sec_ch =
				nla_get_u8(tb2[SET_CHAN_SECONDARY_CHANNEL]);
		}
		if (tb2[SET_CHAN_SEG0_CENTER_CHANNEL]) {
			channel_list[i].vht_seg0_center_ch =
				nla_get_u8(tb2[SET_CHAN_SEG0_CENTER_CHANNEL]);
		}
		if (tb2[SET_CHAN_SEG1_CENTER_CHANNEL]) {
			channel_list[i].vht_seg1_center_ch =
				nla_get_u8(tb2[SET_CHAN_SEG1_CENTER_CHANNEL]);
		}
		if (tb2[SET_CHAN_CHANNEL_WIDTH]) {
			channel_list[i].chan_width =
				nla_get_u8(tb2[SET_CHAN_CHANNEL_WIDTH]);
		}
		hdd_debug("index %d pri %d sec %d seg0 %d seg1 %d width %d",
			i, channel_list[i].pri_ch,
			channel_list[i].ht_sec_ch,
			channel_list[i].vht_seg0_center_ch,
			channel_list[i].vht_seg1_center_ch,
			channel_list[i].chan_width);
		i++;
	}
	*chan_list_ptr = channel_list;

	return 0;
}

/**
 * Undef short names for vendor set channel configuration
 */
#undef SET_CHAN_REASON
#undef SET_CHAN_CHANNEL_COUNT
#undef SET_CHAN_CHAN_LIST
#undef SET_CHAN_PRIMARY_CHANNEL
#undef SET_CHAN_SECONDARY_CHANNEL
#undef SET_CHAN_SEG0_CENTER_CHANNEL
#undef SET_CHAN_SEG1_CENTER_CHANNEL
#undef SET_CHAN_CHANNEL_WIDTH
#undef SET_CHAN_MAX

/**
 * __wlan_hdd_cfg80211_update_vendor_channel() - update vendor channel
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Length of @data
 *
 * Return: 0 on success, negative errno on failure
 */
static int __wlan_hdd_cfg80211_update_vendor_channel(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int data_len)
{
	int ret_val;
	QDF_STATUS qdf_status;
	uint8_t channel_cnt = 0, reason = -1;
	struct hdd_vendor_chan_info *channel_list = NULL;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(wdev->netdev);
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct hdd_vendor_chan_info *channel_list_ptr;

	ENTER();

	ret_val = wlan_hdd_validate_context(hdd_ctx);
	if (ret_val)
		return ret_val;

	if (hdd_get_conparam() == QDF_GLOBAL_FTM_MODE) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (test_bit(VENDOR_ACS_RESPONSE_PENDING, &adapter->event_flags))
		clear_bit(VENDOR_ACS_RESPONSE_PENDING, &adapter->event_flags);
	else {
		hdd_err("already timeout happened for acs");
		return -EINVAL;
	}

	ret_val = hdd_parse_vendor_acs_chan_config(&channel_list, &reason,
					&channel_cnt, data, data_len);
	channel_list_ptr = channel_list;
	if (ret_val)
		return ret_val;

	/* Validate channel to be set */
	while (channel_cnt && channel_list) {
		qdf_status = wlan_hdd_validate_acs_channel(adapter,
					channel_list->pri_ch,
					channel_list->chan_width);
		if (qdf_status == QDF_STATUS_SUCCESS)
			break;
		else if (channel_cnt == 1) {
			hdd_err("invalid channel %d received from app",
				channel_list->pri_ch);
			channel_list->pri_ch = 0;
			break;
		}

		channel_cnt--;
		channel_list++;
	}

	if ((channel_cnt <= 0) || !channel_list) {
		hdd_err("no available channel/chanlist %d/%pK", channel_cnt,
			channel_list);
		qdf_mem_free(channel_list_ptr);
		return -EINVAL;
	}

	hdd_debug("received primary channel as %d", channel_list->pri_ch);

	ret_val = hdd_update_acs_channel(adapter, reason,
				      channel_cnt, channel_list);
	qdf_mem_free(channel_list_ptr);
	return ret_val;
}

/**
 * wlan_hdd_cfg80211_update_vendor_channel() - update vendor channel
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Length of @data
 *
 * Return: 0 on success, negative errno on failure
 */
static int wlan_hdd_cfg80211_update_vendor_channel(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data, int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_update_vendor_channel(wiphy, wdev, data,
								data_len);
	cds_ssr_protect(__func__);

	return ret;
}

/**
 * wlan_hdd_cfg80211_setband() - Wrapper to setband
 * @wiphy:    wiphy structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of @data
 *
 * Return: 0 on success; errno on failure
 */
static int wlan_hdd_cfg80211_setband(struct wiphy *wiphy,
				    struct wireless_dev *wdev,
				    const void *data, int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_setband(wiphy, wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * wlan_hdd_cfg80211_sar_convert_limit_set() - Convert limit set value
 * @nl80211_value:    Vendor command attribute value
 * @wmi_value:        Pointer to return converted WMI return value
 *
 * Convert NL80211 vendor command value for SAR limit set to WMI value
 * Return: 0 on success, -1 on invalid value
 */
static int wlan_hdd_cfg80211_sar_convert_limit_set(u32 nl80211_value,
						   u32 *wmi_value)
{
	int ret = 0;

	switch (nl80211_value) {
	case QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_NONE:
		*wmi_value = WMI_SAR_FEATURE_OFF;
		break;
	case QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_BDF0:
		*wmi_value = WMI_SAR_FEATURE_ON_SET_0;
		break;
	case QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_BDF1:
		*wmi_value = WMI_SAR_FEATURE_ON_SET_1;
		break;
	case QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_BDF2:
		*wmi_value = WMI_SAR_FEATURE_ON_SET_2;
		break;
	case QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_BDF3:
		*wmi_value = WMI_SAR_FEATURE_ON_SET_3;
		break;
	case QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_BDF4:
		*wmi_value = WMI_SAR_FEATURE_ON_SET_4;
		break;
	case QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_USER:
		*wmi_value = WMI_SAR_FEATURE_ON_USER_DEFINED;
		break;
	default:
		ret = -1;
	}
	return ret;
}

/**
 * wlan_hdd_cfg80211_sar_convert_band() - Convert WLAN band value
 * @nl80211_value:    Vendor command attribute value
 * @wmi_value:        Pointer to return converted WMI return value
 *
 * Convert NL80211 vendor command value for SAR BAND to WMI value
 * Return: 0 on success, -1 on invalid value
 */
static int wlan_hdd_cfg80211_sar_convert_band(u32 nl80211_value, u32 *wmi_value)
{
	int ret = 0;

	switch (nl80211_value) {
	case HDD_NL80211_BAND_2GHZ:
		*wmi_value = WMI_SAR_2G_ID;
		break;
	case HDD_NL80211_BAND_5GHZ:
		*wmi_value = WMI_SAR_5G_ID;
		break;
	default:
		ret = -1;
	}
	return ret;
}

/**
 * wlan_hdd_cfg80211_sar_convert_modulation() - Convert WLAN modulation value
 * @nl80211_value:    Vendor command attribute value
 * @wmi_value:        Pointer to return converted WMI return value
 *
 * Convert NL80211 vendor command value for SAR Modulation to WMI value
 * Return: 0 on success, -1 on invalid value
 */
static int wlan_hdd_cfg80211_sar_convert_modulation(u32 nl80211_value,
						    u32 *wmi_value)
{
	int ret = 0;

	switch (nl80211_value) {
	case QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_MODULATION_CCK:
		*wmi_value = WMI_SAR_MOD_CCK;
		break;
	case QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_MODULATION_OFDM:
		*wmi_value = WMI_SAR_MOD_OFDM;
		break;
	default:
		ret = -1;
	}
	return ret;
}

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

static const struct nla_policy
sar_limits_policy[QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SAR_ENABLE] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_NUM_SPECS] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_BAND] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_CHAIN] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_MODULATION] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_POWER_LIMIT] = {.type = NLA_U32},
};

#define WLAN_WAIT_TIME_SAR 5000

/**
 * hdd_sar_context - hdd sar context
 * @event: sar limit event
 */
struct hdd_sar_context {
	struct sar_limit_event event;
};

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
	struct hdd_request *request;
	struct hdd_sar_context *context;

	ENTER();

	if (!event) {
		hdd_err("response is NULL");
		return;
	}

	request = hdd_request_get(cookie);
	if (!request) {
		hdd_debug("Obsolete request");
		return;
	}

	context = hdd_request_priv(request);
	context->event = *event;
	hdd_request_complete(request);
	hdd_request_put(request);

	EXIT();
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
	struct hdd_request *request;
	struct hdd_sar_context *context;
	void *cookie;
	QDF_STATUS status;
	int ret;
	static const struct hdd_request_params params = {
		.priv_size = sizeof(*context),
		.timeout_ms = WLAN_WAIT_TIME_SAR,
	};

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	if (wlan_hdd_validate_context(hdd_ctx))
		return -EINVAL;

	request = hdd_request_alloc(&params);
	if (!request) {
		hdd_err("Request allocation failure");
		return -ENOMEM;
	}

	cookie = hdd_request_cookie(request);

	status = sme_get_sar_power_limits(hdd_ctx->hHal, hdd_sar_cb, cookie);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("Unable to post sar message");
		ret = -EINVAL;
		goto cleanup;
	}

	ret = hdd_request_wait_for_response(request);
	if (ret) {
		hdd_err("Target response timed out");
		goto cleanup;
	}

	context = hdd_request_priv(request);
	ret = hdd_sar_send_response(wiphy, &context->event);

cleanup:
	hdd_request_put(request);

	return ret;
}

/**
 * wlan_hdd_cfg80211_get_sar_power_limits() - Get SAR power limits
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Length of @data
 *
 * Wrapper function of __wlan_hdd_cfg80211_get_sar_power_limits()
 *
 * Return: 0 on success, negative errno on failure
 */
static int wlan_hdd_cfg80211_get_sar_power_limits(struct wiphy *wiphy,
						  struct wireless_dev *wdev,
						  const void *data,
						  int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_get_sar_power_limits(wiphy, wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __wlan_hdd_set_sar_power_limits() - Set SAR power limits
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Length of @data
 *
 * This function is used to setup Specific Absorption Rate limit specs.
 *
 * Return: 0 on success, negative errno on failure
 */
static int __wlan_hdd_set_sar_power_limits(struct wiphy *wiphy,
					   struct wireless_dev *wdev,
					   const void *data, int data_len)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct nlattr *sar_spec[QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_MAX + 1],
		      *tb[QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_MAX + 1],
		      *sar_spec_list;
	struct sar_limit_cmd_params sar_limit_cmd = {0};
	int ret = -EINVAL, i = 0, rem = 0;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	if (wlan_hdd_validate_context(hdd_ctx))
		return -EINVAL;

	if (wlan_cfg80211_nla_parse(tb, QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_MAX,
				    data, data_len, sar_limits_policy)) {
		hdd_err("Invalid SAR attributes");
		return -EINVAL;
	}

	/* Vendor command manadates all SAR Specs in single call */
	sar_limit_cmd.commit_limits = 1;
	sar_limit_cmd.sar_enable = WMI_SAR_FEATURE_NO_CHANGE;
	if (tb[QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SAR_ENABLE]) {
		if (wlan_hdd_cfg80211_sar_convert_limit_set(nla_get_u32(
				tb[QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SAR_ENABLE]),
				&sar_limit_cmd.sar_enable) < 0) {
			hdd_err("Invalid SAR Enable attr");
			goto fail;
		}
	}
	hdd_debug("attr sar sar_enable %d", sar_limit_cmd.sar_enable);

	if (tb[QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_NUM_SPECS]) {
		sar_limit_cmd.num_limit_rows = nla_get_u32(
			tb[QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_NUM_SPECS]);
		hdd_debug("attr sar num_limit_rows %d",
			sar_limit_cmd.num_limit_rows);
	}
	if (sar_limit_cmd.num_limit_rows > MAX_SAR_LIMIT_ROWS_SUPPORTED) {
		hdd_err("SAR Spec list exceed supported size");
		goto fail;
	}
	if (sar_limit_cmd.num_limit_rows == 0)
		goto send_sar_limits;
	sar_limit_cmd.sar_limit_row_list = qdf_mem_malloc(sizeof(
						struct sar_limit_cmd_row) *
						sar_limit_cmd.num_limit_rows);
	if (!sar_limit_cmd.sar_limit_row_list) {
		ret = -ENOMEM;
		goto fail;
	}
	if (!tb[QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC]) {
		hdd_err("Invalid SAR SPECs list");
		goto fail;
	}

	nla_for_each_nested(sar_spec_list,
			    tb[QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC], rem) {
		if (i == sar_limit_cmd.num_limit_rows) {
			hdd_warn("SAR Cmd has excess SPECs in list");
			break;
		}

		if (wlan_cfg80211_nla_parse(sar_spec,
					    QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_MAX,
					    nla_data(sar_spec_list),
					    nla_len(sar_spec_list),
					    sar_limits_policy)) {
			hdd_err("nla_parse failed for SAR Spec list");
			goto fail;
		}
		sar_limit_cmd.sar_limit_row_list[i].validity_bitmap = 0;
		if (sar_spec[
			    QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_POWER_LIMIT]) {
			sar_limit_cmd.sar_limit_row_list[i].limit_value =
				nla_get_u32(sar_spec[
				QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_POWER_LIMIT]);
		} else {
			hdd_err("SAR Spec does not have power limit value");
			goto fail;
		}

		if (sar_spec[QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_BAND]) {
			if (wlan_hdd_cfg80211_sar_convert_band(nla_get_u32(
					sar_spec[QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_BAND]),
					&sar_limit_cmd.sar_limit_row_list[i].band_id)
					< 0) {
				hdd_err("Invalid SAR Band attr");
				goto fail;
			}
			sar_limit_cmd.sar_limit_row_list[i].validity_bitmap |=
						WMI_SAR_BAND_ID_VALID_MASK;
		}
		if (sar_spec[QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_CHAIN]) {
			sar_limit_cmd.sar_limit_row_list[i].chain_id =
				nla_get_u32(sar_spec[
				QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_CHAIN]);
			sar_limit_cmd.sar_limit_row_list[i].validity_bitmap |=
						WMI_SAR_CHAIN_ID_VALID_MASK;
		}
		if (sar_spec[QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_MODULATION]) {
			if (wlan_hdd_cfg80211_sar_convert_modulation(nla_get_u32(
					sar_spec[QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_MODULATION]),
					&sar_limit_cmd.sar_limit_row_list[i].mod_id)
					< 0) {
				hdd_err("Invalid SAR Modulation attr");
				goto fail;
			}
			sar_limit_cmd.sar_limit_row_list[i].validity_bitmap |=
						WMI_SAR_MOD_ID_VALID_MASK;
		}
		hdd_debug("Spec_ID: %d, Band: %d Chain: %d Mod: %d POW_Limit: %d Validity_Bitmap: %d",
			 i, sar_limit_cmd.sar_limit_row_list[i].band_id,
			 sar_limit_cmd.sar_limit_row_list[i].chain_id,
			 sar_limit_cmd.sar_limit_row_list[i].mod_id,
			 sar_limit_cmd.sar_limit_row_list[i].limit_value,
			 sar_limit_cmd.sar_limit_row_list[i].validity_bitmap);
		i++;
	}

	if (i < sar_limit_cmd.num_limit_rows) {
		hdd_warn("SAR Cmd has less SPECs in list");
		sar_limit_cmd.num_limit_rows = i;
	}

send_sar_limits:
	if (sme_set_sar_power_limits(hdd_ctx->hHal, &sar_limit_cmd) ==
							QDF_STATUS_SUCCESS)
		ret = 0;
fail:
	qdf_mem_free(sar_limit_cmd.sar_limit_row_list);
	return ret;
}

/**
 * wlan_hdd_cfg80211_set_sar_power_limits() - Set SAR power limits
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Length of @data
 *
 * Wrapper function of __wlan_hdd_cfg80211_set_sar_power_limits()
 *
 * Return: 0 on success, negative errno on failure
 */
static int wlan_hdd_cfg80211_set_sar_power_limits(struct wiphy *wiphy,
						  struct wireless_dev *wdev,
						  const void *data,
						  int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_set_sar_power_limits(wiphy, wdev, data,
					      data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

static const struct
nla_policy qca_wlan_vendor_attr[QCA_WLAN_VENDOR_ATTR_MAX+1] = {
	[QCA_WLAN_VENDOR_ATTR_ROAMING_POLICY] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_MAC_ADDR]       = {.type = NLA_BINARY,
						 .len = QDF_MAC_ADDR_SIZE},
};

void wlan_hdd_rso_cmd_status_cb(void *ctx, struct rso_cmd_status *rso_status)
{
	struct hdd_context *hdd_ctx = (struct hdd_context *)ctx;
	struct hdd_adapter *adapter;

	adapter = hdd_get_adapter_by_vdev(hdd_ctx, rso_status->vdev_id);
	if (!adapter) {
		hdd_err("adapter NULL");
		return;
	}

	adapter->lfr_fw_status.is_disabled = rso_status->status;
	complete(&adapter->lfr_fw_status.disable_lfr_event);
}

/**
 * __wlan_hdd_cfg80211_set_fast_roaming() - enable/disable roaming
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Length of @data
 *
 * This function is used to enable/disable roaming using vendor commands
 *
 * Return: 0 on success, negative errno on failure
 */
static int __wlan_hdd_cfg80211_set_fast_roaming(struct wiphy *wiphy,
					    struct wireless_dev *wdev,
					    const void *data, int data_len)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_MAX + 1];
	uint32_t is_fast_roam_enabled;
	int ret;
	QDF_STATUS qdf_status;
	unsigned long rc;
	struct hdd_station_ctx *hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	ENTER_DEV(dev);

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	ret = wlan_cfg80211_nla_parse(tb,
				      QCA_WLAN_VENDOR_ATTR_MAX, data, data_len,
				      qca_wlan_vendor_attr);
	if (ret) {
		hdd_err("Invalid ATTR");
		return -EINVAL;
	}

	/* Parse and fetch Enable flag */
	if (!tb[QCA_WLAN_VENDOR_ATTR_ROAMING_POLICY]) {
		hdd_err("attr enable failed");
		return -EINVAL;
	}

	is_fast_roam_enabled = nla_get_u32(
				tb[QCA_WLAN_VENDOR_ATTR_ROAMING_POLICY]);
	hdd_debug("isFastRoamEnabled %d", is_fast_roam_enabled);

	/* Update roaming */
	qdf_status = sme_config_fast_roaming(hdd_ctx->hHal, adapter->session_id,
					     is_fast_roam_enabled);
	if (qdf_status != QDF_STATUS_SUCCESS)
		hdd_err("sme_config_fast_roaming failed with status=%d",
				qdf_status);
	ret = qdf_status_to_os_return(qdf_status);

	if (eConnectionState_Associated == hdd_sta_ctx->conn_info.connState &&
		QDF_IS_STATUS_SUCCESS(qdf_status) && !is_fast_roam_enabled) {

		INIT_COMPLETION(adapter->lfr_fw_status.disable_lfr_event);
		/*
		 * wait only for LFR disable in fw as LFR enable
		 * is always success
		 */
		rc = wait_for_completion_timeout(
				&adapter->lfr_fw_status.disable_lfr_event,
				msecs_to_jiffies(WAIT_TIME_RSO_CMD_STATUS));
		if (!rc) {
			hdd_err("Timed out waiting for RSO CMD status");
			return -ETIMEDOUT;
		}

		if (!adapter->lfr_fw_status.is_disabled) {
			hdd_err("Roam disable attempt in FW fails");
			return -EBUSY;
		}
	}

	EXIT();
	return ret;
}

/**
 * wlan_hdd_cfg80211_set_fast_roaming() - enable/disable roaming
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Length of @data
 *
 * Wrapper function of __wlan_hdd_cfg80211_set_fast_roaming()
 *
 * Return: 0 on success, negative errno on failure
 */
static int wlan_hdd_cfg80211_set_fast_roaming(struct wiphy *wiphy,
					  struct wireless_dev *wdev,
					  const void *data, int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_set_fast_roaming(wiphy, wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}


void hdd_update_cca_info_cb(void *context, uint32_t congestion,
			uint32_t vdev_id)
{
	struct hdd_context *hdd_ctx = (struct hdd_context *)context;
	int status;
	struct hdd_adapter *adapter = NULL;
	struct hdd_station_ctx *hdd_sta_ctx;

	status = wlan_hdd_validate_context(hdd_ctx);
	if (status != 0)
		return;

	adapter = hdd_get_adapter_by_vdev(hdd_ctx, vdev_id);
	if (adapter == NULL) {
		hdd_err("vdev_id %d does not exist with host", vdev_id);
		return;
	}

	hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	hdd_sta_ctx->conn_info.cca = congestion;
	hdd_info("congestion:%d", congestion);
}

static const struct nla_policy qca_wlan_vendor_set_trace_level_policy[
		QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_PARAM] = {.type = NLA_NESTED },
	[QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_MODULE_ID] = {.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_TRACE_MASK] = {.type = NLA_U32 },
};

/**
 * __wlan_hdd_cfg80211_set_trace_level() - Set the trace level
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Length of @data
 *
 * Return: 0 on success, negative errno on failure
 */
static int
__wlan_hdd_cfg80211_set_trace_level(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					const void *data,
					int data_len)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct nlattr *tb1[QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_MAX + 1];
	struct nlattr *tb2[QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_MAX + 1];
	struct nlattr *apth;
	int rem;
	int ret = 1;
	int print_idx = -1;
	int module_id = -1;
	int bit_mask = -1;
	int status;

	ENTER();

	if (hdd_get_conparam() == QDF_GLOBAL_FTM_MODE) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret != 0)
		return -EINVAL;

	print_idx = qdf_get_pidx();
	if (print_idx < 0 || print_idx >= MAX_PRINT_CONFIG_SUPPORTED) {
		hdd_err("Invalid print controle object index");
		return -EINVAL;
	}

	if (wlan_cfg80211_nla_parse(tb1,
				    QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_MAX,
				    data, data_len,
				    qca_wlan_vendor_set_trace_level_policy)) {
		hdd_err("Invalid attr");
		return -EINVAL;
	}

	if (!tb1[QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_PARAM]) {
		hdd_err("attr trace level param failed");
		return -EINVAL;
	}

	nla_for_each_nested(apth,
			tb1[QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_PARAM], rem) {
		if (wlan_cfg80211_nla_parse(tb2,
				       QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_MAX,
				       nla_data(apth), nla_len(apth), NULL)) {
			hdd_err("Invalid attr");
			return -EINVAL;
		}

		if (!tb2[QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_MODULE_ID]) {
			hdd_err("attr Module ID failed");
			return -EINVAL;
		}
		module_id = nla_get_u32
			(tb2[QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_MODULE_ID]);

		if (!tb2[QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_TRACE_MASK]) {
			hdd_err("attr Verbose mask failed");
			return -EINVAL;
		}
		bit_mask = nla_get_u32
		      (tb2[QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_TRACE_MASK]);

		status = hdd_qdf_trace_enable(module_id, bit_mask);

		if (status != 0)
			hdd_err("can not set verbose mask %d for the category %d",
				bit_mask, module_id);
	}

	EXIT();
	return ret;
}

/**
 * wlan_hdd_cfg80211_set_trace_level() - Set the trace level
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Length of @data
 *
 * Wrapper function of __wlan_hdd_cfg80211_set_trace_level()
 *
 * Return: 0 on success, negative errno on failure
 */

static int wlan_hdd_cfg80211_set_trace_level(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_set_trace_level(wiphy, wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/*
 * define short names for the global vendor params
 * used by wlan_hdd_cfg80211_setarp_stats_cmd()
 */
#define STATS_SET_INVALID \
	QCA_ATTR_NUD_STATS_SET_INVALID
#define STATS_SET_START \
	QCA_ATTR_NUD_STATS_SET_START
#define STATS_GW_IPV4 \
	QCA_ATTR_NUD_STATS_GW_IPV4
#define STATS_SET_MAX \
	QCA_ATTR_NUD_STATS_SET_MAX

const struct nla_policy
qca_wlan_vendor_set_nud_stats[STATS_SET_MAX + 1] = {
	[STATS_SET_START] = {.type = NLA_FLAG },
	[STATS_GW_IPV4] = {.type = NLA_U32 },
};

/**
 * __wlan_hdd_cfg80211_set_nud_stats() - set arp stats command to firmware
 * @wiphy: pointer to wireless wiphy structure.
 * @wdev: pointer to wireless_dev structure.
 * @data: pointer to apfind configuration data.
 * @data_len: the length in byte of apfind data.
 *
 * This is called when wlan driver needs to send arp stats to
 * firmware.
 *
 * Return: An error code or 0 on success.
 */
static int __wlan_hdd_cfg80211_set_nud_stats(struct wiphy *wiphy,
					     struct wireless_dev *wdev,
					     const void *data, int data_len)
{
	struct nlattr *tb[STATS_SET_MAX + 1];
	struct net_device   *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct set_arp_stats_params arp_stats_params;
	int err = 0;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	err = wlan_hdd_validate_context(hdd_ctx);
	if (0 != err)
		return err;

	err = wlan_cfg80211_nla_parse(tb, STATS_SET_MAX, data, data_len,
				      qca_wlan_vendor_set_nud_stats);
	if (err) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s STATS_SET_START ATTR", __func__);
		return err;
	}

	if (adapter->session_id == HDD_SESSION_ID_INVALID) {
		hdd_err("Invalid session id");
		return -EINVAL;
	}

	if (tb[STATS_SET_START]) {
		if (!tb[STATS_GW_IPV4]) {
			QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
				  "%s STATS_SET_START CMD", __func__);
			return -EINVAL;
		}
		arp_stats_params.flag = true;
		arp_stats_params.ip_addr = nla_get_u32(tb[STATS_GW_IPV4]);
		hdd_ctx->track_arp_ip = arp_stats_params.ip_addr;
	} else {
		arp_stats_params.flag = false;
	}
	if (arp_stats_params.flag)
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
			  "%s STATS_SET_START Cleared!!", __func__);

	qdf_mem_zero(&adapter->hdd_stats.hdd_arp_stats,
		     sizeof(adapter->hdd_stats.hdd_arp_stats));

	arp_stats_params.pkt_type = WLAN_NUD_STATS_ARP_PKT_TYPE;
	arp_stats_params.vdev_id = adapter->session_id;

	if (QDF_STATUS_SUCCESS !=
	    sme_set_nud_debug_stats(hdd_ctx->hHal, &arp_stats_params)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s STATS_SET_START CMD Failed!!", __func__);
		return -EINVAL;
	}

	EXIT();

	return err;
}

/**
 * wlan_hdd_cfg80211_set_nud_stats() - set arp stats command to firmware
 * @wiphy: pointer to wireless wiphy structure.
 * @wdev: pointer to wireless_dev structure.
 * @data: pointer to apfind configuration data.
 * @data_len: the length in byte of apfind data.
 *
 * This is called when wlan driver needs to send arp stats to
 * firmware.
 *
 * Return: An error code or 0 on success.
 */
static int wlan_hdd_cfg80211_set_nud_stats(struct wiphy *wiphy,
					   struct wireless_dev *wdev,
					   const void *data, int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_set_nud_stats(wiphy, wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

#undef STATS_SET_INVALID
#undef STATS_SET_START
#undef STATS_GW_IPV4
#undef STATS_SET_MAX

/*
 * define short names for the global vendor params
 * used by wlan_hdd_cfg80211_setarp_stats_cmd()
 */
#define STATS_GET_INVALID \
	QCA_ATTR_NUD_STATS_SET_INVALID
#define COUNT_FROM_NETDEV \
	QCA_ATTR_NUD_STATS_ARP_REQ_COUNT_FROM_NETDEV
#define COUNT_TO_LOWER_MAC \
	QCA_ATTR_NUD_STATS_ARP_REQ_COUNT_TO_LOWER_MAC
#define RX_COUNT_BY_LOWER_MAC \
	QCA_ATTR_NUD_STATS_ARP_REQ_RX_COUNT_BY_LOWER_MAC
#define COUNT_TX_SUCCESS \
	QCA_ATTR_NUD_STATS_ARP_REQ_COUNT_TX_SUCCESS
#define RSP_RX_COUNT_BY_LOWER_MAC \
	QCA_ATTR_NUD_STATS_ARP_RSP_RX_COUNT_BY_LOWER_MAC
#define RSP_RX_COUNT_BY_UPPER_MAC \
	QCA_ATTR_NUD_STATS_ARP_RSP_RX_COUNT_BY_UPPER_MAC
#define RSP_COUNT_TO_NETDEV \
	QCA_ATTR_NUD_STATS_ARP_RSP_COUNT_TO_NETDEV
#define RSP_COUNT_OUT_OF_ORDER_DROP \
	QCA_ATTR_NUD_STATS_ARP_RSP_COUNT_OUT_OF_ORDER_DROP
#define AP_LINK_ACTIVE \
	QCA_ATTR_NUD_STATS_AP_LINK_ACTIVE
#define AP_LINK_DAD \
	QCA_ATTR_NUD_STATS_IS_DAD
#define STATS_GET_MAX \
	QCA_ATTR_NUD_STATS_GET_MAX

const struct nla_policy
qca_wlan_vendor_get_nud_stats[STATS_GET_MAX + 1] = {
	[COUNT_FROM_NETDEV] = {.type = NLA_U16 },
	[COUNT_TO_LOWER_MAC] = {.type = NLA_U16 },
	[RX_COUNT_BY_LOWER_MAC] = {.type = NLA_U16 },
	[COUNT_TX_SUCCESS] = {.type = NLA_U16 },
	[RSP_RX_COUNT_BY_LOWER_MAC] = {.type = NLA_U16 },
	[RSP_RX_COUNT_BY_UPPER_MAC] = {.type = NLA_U16 },
	[RSP_COUNT_TO_NETDEV] = {.type = NLA_U16 },
	[RSP_COUNT_OUT_OF_ORDER_DROP] = {.type = NLA_U16 },
	[AP_LINK_ACTIVE] = {.type = NLA_FLAG },
	[AP_LINK_DAD] = {.type = NLA_FLAG },
};

/**
 * __wlan_hdd_cfg80211_get_nud_stats() - get arp stats command to firmware
 * @wiphy: pointer to wireless wiphy structure.
 * @wdev: pointer to wireless_dev structure.
 * @data: pointer to apfind configuration data.
 * @data_len: the length in byte of apfind data.
 *
 * This is called when wlan driver needs to get arp stats to
 * firmware.
 *
 * Return: An error code or 0 on success.
 */
static int __wlan_hdd_cfg80211_get_nud_stats(struct wiphy *wiphy,
					     struct wireless_dev *wdev,
					     const void *data, int data_len)
{
	int err = 0;
	unsigned long rc;
	struct hdd_nud_stats_context *context;
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct get_arp_stats_params arp_stats_params;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	struct sk_buff *skb;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	err = wlan_hdd_validate_context(hdd_ctx);
	if (0 != err)
		return err;

	err = hdd_validate_adapter(adapter);
	if (err)
		return err;

	arp_stats_params.pkt_type = WLAN_NUD_STATS_ARP_PKT_TYPE;
	arp_stats_params.vdev_id = adapter->session_id;

	spin_lock(&hdd_context_lock);
	context = &hdd_ctx->nud_stats_context;
	INIT_COMPLETION(context->response_event);
	spin_unlock(&hdd_context_lock);

	if (hdd_ctx->config->enable_data_stall_det)
		cdp_post_data_stall_event(soc,
				      DATA_STALL_LOG_INDICATOR_FRAMEWORK,
				      DATA_STALL_LOG_NUD_FAILURE,
				      0xFF, 0XFF,
				      DATA_STALL_LOG_RECOVERY_TRIGGER_PDR);

	if (QDF_STATUS_SUCCESS !=
	    sme_get_nud_debug_stats(hdd_ctx->hHal, &arp_stats_params)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s STATS_SET_START CMD Failed!!", __func__);
		return -EINVAL;
	}

	rc = wait_for_completion_timeout(&context->response_event,
					 msecs_to_jiffies(
						WLAN_WAIT_TIME_NUD_STATS));
	if (!rc) {
		hdd_err("Target response timed out request ");
		return -ETIMEDOUT;
	}

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy,
						  WLAN_NUD_STATS_LEN);
	if (!skb) {
		hdd_err("%s: cfg80211_vendor_cmd_alloc_reply_skb failed",
			__func__);
		return -ENOMEM;
	}

	if (nla_put_u16(skb, COUNT_FROM_NETDEV,
			adapter->hdd_stats.hdd_arp_stats.tx_arp_req_count) ||
	    nla_put_u16(skb, COUNT_TO_LOWER_MAC,
			adapter->hdd_stats.hdd_arp_stats.tx_host_fw_sent) ||
	    nla_put_u16(skb, RX_COUNT_BY_LOWER_MAC,
			adapter->hdd_stats.hdd_arp_stats.tx_host_fw_sent) ||
	    nla_put_u16(skb, COUNT_TX_SUCCESS,
			adapter->hdd_stats.hdd_arp_stats.tx_ack_cnt) ||
	    nla_put_u16(skb, RSP_RX_COUNT_BY_LOWER_MAC,
			adapter->hdd_stats.hdd_arp_stats.rx_fw_cnt) ||
	    nla_put_u16(skb, RSP_RX_COUNT_BY_UPPER_MAC,
			adapter->hdd_stats.hdd_arp_stats.rx_arp_rsp_count) ||
	    nla_put_u16(skb, RSP_COUNT_TO_NETDEV,
			adapter->hdd_stats.hdd_arp_stats.rx_delivered) ||
	    nla_put_u16(skb, RSP_COUNT_OUT_OF_ORDER_DROP,
			adapter->hdd_stats.hdd_arp_stats.
			rx_host_drop_reorder)) {
		hdd_err("nla put fail");
		kfree_skb(skb);
		return -EINVAL;
	}
	if (adapter->con_status)
		nla_put_flag(skb, AP_LINK_ACTIVE);
	if (adapter->dad)
		nla_put_flag(skb, AP_LINK_DAD);

	hdd_ctx->track_arp_ip = 0;
	cfg80211_vendor_cmd_reply(skb);
	return err;
}

/**
 * wlan_hdd_cfg80211_get_nud_stats() - get arp stats command to firmware
 * @wiphy: pointer to wireless wiphy structure.
 * @wdev: pointer to wireless_dev structure.
 * @data: pointer to apfind configuration data.
 * @data_len: the length in byte of apfind data.
 *
 * This is called when wlan driver needs to get arp stats to
 * firmware.
 *
 * Return: An error code or 0 on success.
 */
static int wlan_hdd_cfg80211_get_nud_stats(struct wiphy *wiphy,
					   struct wireless_dev *wdev,
					   const void *data, int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_get_nud_stats(wiphy, wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

#undef QCA_ATTR_NUD_STATS_SET_INVALID
#undef QCA_ATTR_NUD_STATS_ARP_REQ_COUNT_FROM_NETDEV
#undef QCA_ATTR_NUD_STATS_ARP_REQ_COUNT_TO_LOWER_MAC
#undef QCA_ATTR_NUD_STATS_ARP_REQ_RX_COUNT_BY_LOWER_MAC
#undef QCA_ATTR_NUD_STATS_ARP_REQ_COUNT_TX_SUCCESS
#undef QCA_ATTR_NUD_STATS_ARP_RSP_RX_COUNT_BY_LOWER_MAC
#undef QCA_ATTR_NUD_STATS_ARP_RSP_RX_COUNT_BY_UPPER_MAC
#undef QCA_ATTR_NUD_STATS_ARP_RSP_COUNT_TO_NETDEV
#undef QCA_ATTR_NUD_STATS_ARP_RSP_COUNT_OUT_OF_ORDER_DROP
#undef QCA_ATTR_NUD_STATS_AP_LINK_ACTIVE
#undef QCA_ATTR_NUD_STATS_GET_MAX

void hdd_bt_activity_cb(void *context, uint32_t bt_activity)
{
	struct hdd_context *hdd_ctx = (struct hdd_context *)context;
	int status;

	status = wlan_hdd_validate_context(hdd_ctx);
	if (0 != status)
		return;

	if (bt_activity == WLAN_COEX_EVENT_BT_A2DP_PROFILE_ADD)
		hdd_ctx->bt_a2dp_active = 1;
	else if (bt_activity == WLAN_COEX_EVENT_BT_A2DP_PROFILE_REMOVE)
		hdd_ctx->bt_a2dp_active = 0;
	else if (bt_activity == WLAN_COEX_EVENT_BT_VOICE_PROFILE_ADD)
		hdd_ctx->bt_vo_active = 1;
	else if (bt_activity == WLAN_COEX_EVENT_BT_VOICE_PROFILE_REMOVE)
		hdd_ctx->bt_vo_active = 0;
	else
		return;

	ucfg_scan_set_bt_activity(hdd_ctx->hdd_psoc, hdd_ctx->bt_a2dp_active);
	hdd_debug("a2dp_active: %d vo_active: %d", hdd_ctx->bt_a2dp_active,
		 hdd_ctx->bt_vo_active);
}


/**
 * wlan_hdd_is_bt_in_progress() - check if bt activity is in progress
 * @hdd_ctx : HDD context
 *
 * Return: true if BT activity is in progress else false
 */
static inline bool wlan_hdd_is_bt_in_progress(struct hdd_context *hdd_ctx)
{
	if (hdd_ctx->bt_a2dp_active || hdd_ctx->bt_vo_active)
		return true;

	return false;
}

struct chain_rssi_priv {
	struct chain_rssi_result chain_rssi;
};

/**
 * hdd_get_chain_rssi_cb() - Callback function to get chain rssi
 * @context: opaque context originally passed to SME. HDD always passes
 * a cookie for the request context
 * @data: struct for get chain rssi
 *
 * This function receives the response/data from the lower layer and
 * checks to see if the thread is still waiting then post the results to
 * upper layer, if the request has timed out then ignore.
 *
 * Return: None
 */
static void hdd_get_chain_rssi_cb(void *context,
				  struct chain_rssi_result *data)
{
	struct hdd_request *request;
	struct chain_rssi_priv *priv;

	ENTER();

	request = hdd_request_get(context);
	if (!request) {
		hdd_err("Obsolete request");
		return;
	}

	priv = hdd_request_priv(request);
	priv->chain_rssi = *data;
	hdd_request_complete(request);
	hdd_request_put(request);
}

/**
 * hdd_post_get_chain_rssi_rsp - send rsp to user space
 * @hdd_ctx: pointer to hdd context
 * @result: chain rssi result
 *
 * Return: 0 for success, non-zero for failure
 */
static int hdd_post_get_chain_rssi_rsp(struct hdd_context *hdd_ctx,
				       struct chain_rssi_result *result)
{
	struct sk_buff *skb;
	int data_len = sizeof(result->chain_rssi);

	skb = cfg80211_vendor_cmd_alloc_reply_skb(hdd_ctx->wiphy,
		data_len + NLA_HDRLEN + NLMSG_HDRLEN);

	if (!skb) {
		hdd_err("cfg80211_vendor_event_alloc failed");
		return -ENOMEM;
	}

	if (nla_put(skb, QCA_WLAN_VENDOR_ATTR_CHAIN_RSSI, data_len,
			result->chain_rssi)) {
		hdd_err("put fail");
		goto nla_put_failure;
	}

	cfg80211_vendor_cmd_reply(skb);
	return 0;

nla_put_failure:
	kfree_skb(skb);
	return -EINVAL;
}

/**
 * __wlan_hdd_cfg80211_get_chain_rssi() - get chain rssi
 * @wiphy: wiphy pointer
 * @wdev: pointer to struct wireless_dev
 * @data: pointer to incoming NL vendor data
 * @data_len: length of @data
 *
 * Return: 0 on success; error number otherwise.
 */
static int __wlan_hdd_cfg80211_get_chain_rssi(struct wiphy *wiphy,
					      struct wireless_dev *wdev,
					      const void *data,
					      int data_len)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(wdev->netdev);
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct get_chain_rssi_req_params req_msg;
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_MAX + 1];
	QDF_STATUS status;
	int retval;
	void *cookie;
	struct hdd_request *request;
	struct chain_rssi_priv *priv;
	static const struct hdd_request_params params = {
		.priv_size = sizeof(*priv),
		.timeout_ms = WLAN_WAIT_TIME_STATS,
	};

	ENTER();

	retval = wlan_hdd_validate_context(hdd_ctx);
	if (0 != retval)
		return retval;

	if (wlan_cfg80211_nla_parse(tb, QCA_WLAN_VENDOR_ATTR_MAX,
				    data, data_len, NULL)) {
		hdd_err("Invalid ATTR");
		return -EINVAL;
	}

	if (!tb[QCA_WLAN_VENDOR_ATTR_MAC_ADDR]) {
		hdd_err("attr mac addr failed");
		return -EINVAL;
	}
	if (nla_len(tb[QCA_WLAN_VENDOR_ATTR_MAC_ADDR]) !=
		QDF_MAC_ADDR_SIZE) {
		hdd_err("incorrect mac size");
		return -EINVAL;
	}
	memcpy(&req_msg.peer_macaddr,
		nla_data(tb[QCA_WLAN_VENDOR_ATTR_MAC_ADDR]),
		QDF_MAC_ADDR_SIZE);
	req_msg.session_id = adapter->session_id;

	request = hdd_request_alloc(&params);
	if (!request) {
		hdd_err("Request allocation failure");
		return -ENOMEM;
	}
	cookie = hdd_request_cookie(request);

	status = sme_get_chain_rssi(hdd_ctx->hHal,
			&req_msg,
			hdd_get_chain_rssi_cb,
			cookie);
	if (QDF_STATUS_SUCCESS != status) {
		hdd_err("Unable to get chain rssi");
		retval = qdf_status_to_os_return(status);
	} else {
		retval = hdd_request_wait_for_response(request);
		if (retval) {
			hdd_err("Target response timed out");
		} else {
			priv = hdd_request_priv(request);
			retval = hdd_post_get_chain_rssi_rsp(hdd_ctx,
					&priv->chain_rssi);
			if (retval)
				hdd_err("Failed to post chain rssi");
		}
	}
	hdd_request_put(request);

	EXIT();
	return retval;
}

/**
 * wlan_hdd_cfg80211_get_chain_rssi() - get chain rssi
 * @wiphy: wiphy pointer
 * @wdev: pointer to struct wireless_dev
 * @data: pointer to incoming NL vendor data
 * @data_len: length of @data
 *
 * Return: 0 on success; error number otherwise.
 */
static int wlan_hdd_cfg80211_get_chain_rssi(struct wiphy *wiphy,
					    struct wireless_dev *wdev,
					    const void *data,
					    int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_get_chain_rssi(wiphy, wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __wlan_hdd_cfg80211_set_limit_offchan_param() - set limit off-channel cmd
 * parameters
 * @wiphy: pointer to wireless wiphy structure.
 * @wdev: pointer to wireless_dev structure.
 * @data: pointer to limit off-channel command parameters.
 * @data_len: the length in byte of  limit off-channel command parameters.
 *
 * This is called when application wants to limit the off channel time due to
 * active voip traffic.
 *
 * Return: An error code or 0 on success.
 */
static int __wlan_hdd_cfg80211_set_limit_offchan_param(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int data_len)
{
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_ACTIVE_TOS_MAX + 1];
	struct net_device   *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	int ret = 0;
	uint8_t tos;
	uint8_t tos_status;

	ENTER();

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret < 0)
		return ret;

	if (wlan_cfg80211_nla_parse(tb, QCA_WLAN_VENDOR_ATTR_ACTIVE_TOS_MAX,
				 data, data_len,
				 wlan_hdd_set_limit_off_channel_param_policy)) {
		hdd_err("Invalid ATTR");
		return -EINVAL;
	}

	if (!tb[QCA_WLAN_VENDOR_ATTR_ACTIVE_TOS]) {
		hdd_err("attr tos failed");
		goto fail;
	}

	tos = nla_get_u8(tb[QCA_WLAN_VENDOR_ATTR_ACTIVE_TOS]);
	hdd_debug("tos %d", tos);

	if (!tb[QCA_WLAN_VENDOR_ATTR_ACTIVE_TOS_START]) {
		hdd_err("attr tos active failed");
		goto fail;
	}
	tos_status = nla_get_u8(tb[QCA_WLAN_VENDOR_ATTR_ACTIVE_TOS_START]);

	hdd_debug("tos status %d", tos_status);
	ret = hdd_set_limit_off_chan_for_tos(adapter, tos, tos_status);

fail:
	return ret;
}

/**
 * wlan_hdd_cfg80211_set_limit_offchan_param() - set limit off-channel cmd
 * parameters
 * @wiphy: pointer to wireless wiphy structure.
 * @wdev: pointer to wireless_dev structure.
 * @data: pointer to limit off-channel command parameters.
 * @data_len: the length in byte of  limit off-channel command parameters.
 *
 * This is called when application wants to limit the off channel time due to
 * active voip traffic.
 *
 * Return: An error code or 0 on success.
 */
static int wlan_hdd_cfg80211_set_limit_offchan_param(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int data_len)

{
	int ret = 0;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_set_limit_offchan_param(wiphy, wdev, data,
			data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * wlan_hdd_fill_btm_resp() - Fill bss candidate response buffer
 * @reply_skb : pointer to reply_skb
 * @info : bss candidate information
 * @index : attribute type index for nla_next_start()
 *
 * Return : 0 on success and errno on failure
 */
static int wlan_hdd_fill_btm_resp(struct sk_buff *reply_skb,
				  struct bss_candidate_info *info,
				  int index)
{
	struct nlattr *attr;

	attr = nla_nest_start(reply_skb, index);
	if (!attr) {
		hdd_err("nla_nest_start failed");
		kfree_skb(reply_skb);
		return -EINVAL;
	}

	if (nla_put(reply_skb,
		  QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO_BSSID,
		  ETH_ALEN, info->bssid.bytes) ||
	    nla_put_u32(reply_skb,
		 QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO_STATUS,
		 info->status)) {
		hdd_err("nla_put failed");
		kfree_skb(reply_skb);
		return -EINVAL;
	}

	nla_nest_end(reply_skb, attr);

	return 0;
}

/**
 * __wlan_hdd_cfg80211_fetch_bss_transition_status() - fetch bss transition
 * status
 * @wiphy : WIPHY structure pointer
 * @wdev : Wireless device structure pointer
 * @data : Pointer to the data received
 * @data_len : Length of the data received
 *
 * This fuction is used to fetch transition status for candidate bss. The
 * transition status is either accept or reason for reject.
 *
 * Return : 0 on success and errno on failure
 */
static int __wlan_hdd_cfg80211_fetch_bss_transition_status(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data, int data_len)
{
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_MAX + 1];
	struct nlattr *tb_msg[QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO_MAX + 1];
	uint8_t transition_reason;
	struct nlattr *attr;
	struct sk_buff *reply_skb;
	int rem, j;
	int ret;
	bool is_bt_in_progress;
	struct bss_candidate_info candidate_info[MAX_CANDIDATE_INFO];
	uint16_t nof_candidates, i = 0;
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_station_ctx *hdd_sta_ctx =
					WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	tHalHandle hal = WLAN_HDD_GET_HAL_CTX(adapter);

	const struct nla_policy
	btm_params_policy[QCA_WLAN_VENDOR_ATTR_MAX + 1] = {
		[QCA_WLAN_VENDOR_ATTR_BTM_MBO_TRANSITION_REASON] = {
							.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO] = {
							.type = NLA_NESTED},
	};
	const struct nla_policy
	btm_cand_list_policy[QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO_MAX + 1]
		= {[QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO_BSSID] = {
						.len = QDF_MAC_ADDR_SIZE},
		   [QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO_STATUS] = {
							.type = NLA_U32},
		};

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	if (adapter->device_mode != QDF_STA_MODE ||
	    hdd_sta_ctx->conn_info.connState != eConnectionState_Associated) {
		hdd_err("Command is either not invoked for STA mode (device mode: %d) or STA is not associated (Connection state: %d)",
			adapter->device_mode, hdd_sta_ctx->conn_info.connState);
		return -EINVAL;
	}

	ret = wlan_cfg80211_nla_parse(tb, QCA_WLAN_VENDOR_ATTR_MAX, data,
				      data_len, btm_params_policy);
	if (ret) {
		hdd_err("Attribute parse failed");
		return -EINVAL;
	}

	if (!tb[QCA_WLAN_VENDOR_ATTR_BTM_MBO_TRANSITION_REASON] ||
	    !tb[QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO]) {
		hdd_err("Missing attributes");
		return -EINVAL;
	}

	transition_reason = nla_get_u8(
			    tb[QCA_WLAN_VENDOR_ATTR_BTM_MBO_TRANSITION_REASON]);

	nla_for_each_nested(attr,
			    tb[QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO],
			    rem) {
		ret = wlan_cfg80211_nla_parse_nested(tb_msg,
				    QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO_MAX,
				    attr, btm_cand_list_policy);
		if (ret) {
			hdd_err("Attribute parse failed");
			return -EINVAL;
		}

		if (!tb_msg[QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO_BSSID]) {
			hdd_err("Missing BSSID attribute");
			return -EINVAL;
		}

		qdf_mem_copy((void *)candidate_info[i].bssid.bytes,
			     nla_data(tb_msg[
			     QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO_BSSID]),
			     QDF_MAC_ADDR_SIZE);
		i++;
		if (i == MAX_CANDIDATE_INFO)
			break;
	}

	/*
	 * Determine status for each candidate and fill in the status field.
	 * Also arrange the candidates in the order of preference.
	 */
	nof_candidates = i;

	is_bt_in_progress = wlan_hdd_is_bt_in_progress(hdd_ctx);

	ret = sme_get_bss_transition_status(hal, transition_reason,
					    &hdd_sta_ctx->conn_info.bssId,
					    candidate_info,
					    nof_candidates,
					    is_bt_in_progress);
	if (ret)
		return -EINVAL;

	/* Prepare the reply and send it to userspace */
	reply_skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy,
			((QDF_MAC_ADDR_SIZE + sizeof(uint32_t)) *
			 nof_candidates) + NLMSG_HDRLEN);
	if (!reply_skb) {
		hdd_err("reply buffer alloc failed");
		return -ENOMEM;
	}

	attr = nla_nest_start(reply_skb,
			      QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO);
	if (!attr) {
		hdd_err("nla_nest_start failed");
		kfree_skb(reply_skb);
		return -EINVAL;
	}

	/*
	 * Order candidates as - accepted candidate list followed by rejected
	 * candidate list
	 */
	for (i = 0, j = 0; i < nof_candidates; i++) {
		/* copy accepted candidate list */
		if (candidate_info[i].status == QCA_STATUS_ACCEPT) {
			if (wlan_hdd_fill_btm_resp(reply_skb,
						   &candidate_info[i], j))
				return -EINVAL;
			j++;
		}
	}
	for (i = 0; i < nof_candidates; i++) {
		/* copy rejected candidate list */
		if (candidate_info[i].status != QCA_STATUS_ACCEPT) {
			if (wlan_hdd_fill_btm_resp(reply_skb,
						   &candidate_info[i], j))
				return -EINVAL;
			j++;
		}
	}
	nla_nest_end(reply_skb, attr);

	EXIT();

	return cfg80211_vendor_cmd_reply(reply_skb);
}

/**
 * wlan_hdd_cfg80211_fetch_bss_transition_status() - fetch bss transition status
 * @wiphy : WIPHY structure pointer
 * @wdev : Wireless device structure pointer
 * @data : Pointer to the data received
 * @data_len : Length of the data received
 *
 * This fuction is used to fetch transition status for candidate bss. The
 * transition status is either accept or reason for reject.
 *
 * Return : 0 on success and errno on failure
 */
static int wlan_hdd_cfg80211_fetch_bss_transition_status(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data, int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_fetch_bss_transition_status(wiphy, wdev,
							      data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * wlan_hdd_fill_intf_info() - Fill skb buffer with interface info
 * @skb: Pointer to skb
 * @info: mac mode info
 * @index: attribute type index for nla_nest_start()
 *
 * Return : 0 on success and errno on failure
 */
static int wlan_hdd_fill_intf_info(struct sk_buff *skb,
				   struct connection_info *info, int index)
{
	struct nlattr *attr;
	uint32_t freq;
	struct hdd_context *hdd_ctx;
	struct hdd_adapter *hdd_adapter;

	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	if (!hdd_ctx)
		goto error;

	hdd_adapter = hdd_get_adapter_by_vdev(hdd_ctx, info->vdev_id);
	if (!hdd_adapter)
		goto error;

	attr = nla_nest_start(skb, index);
	if (!attr)
		goto error;

	freq = sme_chn_to_freq(info->channel);

	if (nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_MAC_IFACE_INFO_IFINDEX,
	    hdd_adapter->dev->ifindex) ||
	    nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_MAC_IFACE_INFO_FREQ, freq))
		goto error;

	nla_nest_end(skb, attr);

	return 0;
error:
	hdd_err("Fill buffer with interface info failed");
	kfree_skb(skb);
	return -EINVAL;
}

/**
 * wlan_hdd_fill_mac_info() - Fill skb buffer with mac info
 * @skb: Pointer to skb
 * @info: mac mode info
 * @mac_id: MAC id
 * @conn_count: number of current connections
 *
 * Return : 0 on success and errno on failure
 */
static int wlan_hdd_fill_mac_info(struct sk_buff *skb,
				  struct connection_info *info, uint32_t mac_id,
				  uint32_t conn_count)
{
	struct nlattr *attr, *intf_attr;
	uint32_t band = 0, i = 0, j = 0;
	bool present = false;

	while (i < conn_count) {
		if (info[i].mac_id == mac_id) {
			present = true;
			if (info[i].channel <= SIR_11B_CHANNEL_END)
				band |= 1 << NL80211_BAND_2GHZ;
			else if (info[i].channel <= SIR_11A_CHANNEL_END)
				band |= 1 << NL80211_BAND_5GHZ;
		}
		i++;
	}

	if (!present)
		return 0;

	i = 0;
	attr = nla_nest_start(skb, mac_id);
	if (!attr)
		goto error;

	if (nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_MAC_INFO_MAC_ID, mac_id) ||
	    nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_MAC_INFO_BAND, band))
		goto error;

	intf_attr = nla_nest_start(skb, QCA_WLAN_VENDOR_ATTR_MAC_IFACE_INFO);
	if (!intf_attr)
		goto error;

	while (i < conn_count) {
		if (info[i].mac_id == mac_id) {
			if (wlan_hdd_fill_intf_info(skb, &info[i], j))
				return -EINVAL;
			j++;
		}
		i++;
	}

	nla_nest_end(skb, intf_attr);

	nla_nest_end(skb, attr);

	return 0;
error:
	hdd_err("Fill buffer with mac info failed");
	kfree_skb(skb);
	return -EINVAL;
}


int wlan_hdd_send_mode_change_event(void)
{
	int err;
	struct hdd_context *hdd_ctx;
	struct sk_buff *skb;
	struct nlattr *attr;
	struct connection_info info[MAX_NUMBER_OF_CONC_CONNECTIONS];
	uint32_t conn_count, mac_id;

	ENTER();
	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	if (!hdd_ctx) {
		hdd_err("HDD context is NULL");
		return -EINVAL;
	}

	err = wlan_hdd_validate_context(hdd_ctx);
	if (0 != err)
		return err;

	conn_count = policy_mgr_get_connection_info(hdd_ctx->hdd_psoc, info);
	if (!conn_count)
		return -EINVAL;

	skb = cfg80211_vendor_event_alloc(hdd_ctx->wiphy, NULL,
				  (sizeof(uint32_t) * 4) *
				  MAX_NUMBER_OF_CONC_CONNECTIONS + NLMSG_HDRLEN,
				  QCA_NL80211_VENDOR_SUBCMD_WLAN_MAC_INFO_INDEX,
				  GFP_KERNEL);
	if (!skb) {
		hdd_err("cfg80211_vendor_cmd_alloc_reply_skb failed");
		return -ENOMEM;
	}

	attr = nla_nest_start(skb, QCA_WLAN_VENDOR_ATTR_MAC_INFO);
	if (!attr) {
		hdd_err("nla_nest_start failed");
		kfree_skb(skb);
		return -EINVAL;
	}

	for (mac_id = 0; mac_id < MAX_MAC; mac_id++) {
		if (wlan_hdd_fill_mac_info(skb, info, mac_id, conn_count))
			return -EINVAL;
	}

	nla_nest_end(skb, attr);

	cfg80211_vendor_event(skb, GFP_KERNEL);
	EXIT();

	return err;
}

const struct wiphy_vendor_command hdd_wiphy_vendor_commands[] = {
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_DFS_CAPABILITY,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = is_driver_dfs_capable
	},

#ifdef WLAN_FEATURE_NAN
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_NAN,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV | WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_nan_request
	},
#endif

#ifdef WLAN_FEATURE_STATS_EXT
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_STATS_EXT,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV | WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_stats_ext_request
	},
#endif
#ifdef FEATURE_WLAN_EXTSCAN
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_START,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV | WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_extscan_start
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_STOP,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV | WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_extscan_stop
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_VALID_CHANNELS,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV | WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_extscan_get_valid_channels
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_CAPABILITIES,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV | WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_extscan_get_capabilities
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_CACHED_RESULTS,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV | WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_extscan_get_cached_results
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_BSSID_HOTLIST,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV | WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_extscan_set_bssid_hotlist
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_BSSID_HOTLIST,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV | WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_extscan_reset_bssid_hotlist
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_SIGNIFICANT_CHANGE,
		.flags =
			WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_extscan_set_significant_change
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_SIGNIFICANT_CHANGE,
		.flags =
			WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_extscan_reset_significant_change
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_SET_LIST,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_set_epno_list
	},
#endif /* FEATURE_WLAN_EXTSCAN */

#ifdef WLAN_FEATURE_LINK_LAYER_STATS
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_LL_STATS_CLR,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV | WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_ll_stats_clear
	},

	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_LL_STATS_SET,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV | WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_ll_stats_set
	},

	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_LL_STATS_GET,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV | WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_ll_stats_get
	},
#endif /* WLAN_FEATURE_LINK_LAYER_STATS */
#ifdef FEATURE_WLAN_TDLS
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_TDLS_ENABLE,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV | WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_exttdls_enable
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_TDLS_DISABLE,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV | WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_exttdls_disable
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_TDLS_GET_STATUS,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = wlan_hdd_cfg80211_exttdls_get_status
	},
#endif
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_GET_SUPPORTED_FEATURES,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = wlan_hdd_cfg80211_get_supported_features
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_SCANNING_MAC_OUI,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_set_scanning_mac_oui
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_GET_CONCURRENCY_MATRIX,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = wlan_hdd_cfg80211_get_concurrency_matrix
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_NO_DFS_FLAG,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_disable_dfs_chan_scan
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_WISA,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_handle_wisa_cmd
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_GET_STATION,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = hdd_cfg80211_get_station_cmd
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_DO_ACS,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
				WIPHY_VENDOR_CMD_NEED_NETDEV |
				WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_do_acs
	},

	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_GET_FEATURES,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = wlan_hdd_cfg80211_get_features
	},
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_KEY_MGMT_SET_KEY,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_keymgmt_set_key
	},
#endif
#ifdef FEATURE_WLAN_EXTSCAN
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_SET_PASSPOINT_LIST,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_set_passpoint_list
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_RESET_PASSPOINT_LIST,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_reset_passpoint_list
	},
#endif /* FEATURE_WLAN_EXTSCAN */
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_GET_WIFI_INFO,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = wlan_hdd_cfg80211_get_wifi_info
	},
#ifndef WLAN_UMAC_CONVERGENCE
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_SET_WIFI_CONFIGURATION,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_wifi_configuration_set
	},
#endif
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_WIFI_TEST_CONFIGURATION,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_set_wifi_test_config
	},

	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_ROAM,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_set_ext_roam_params
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_WIFI_LOGGER_START,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_wifi_logger_start
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_GET_RING_DATA,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_wifi_logger_get_ring_data
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_GET_PREFERRED_FREQ_LIST,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_get_preferred_freq_list
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_SET_PROBABLE_OPER_CHANNEL,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_set_probable_oper_channel
	},
#ifdef WLAN_FEATURE_TSF
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_TSF,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_handle_tsf_cmd
	},
#endif
#ifdef FEATURE_WLAN_TDLS
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_TDLS_GET_CAPABILITIES,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_get_tdls_capabilities
	},
#endif
#ifdef WLAN_FEATURE_OFFLOAD_PACKETS
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_OFFLOADED_PACKETS,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_offloaded_packets
	},
#endif
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_MONITOR_RSSI,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_monitor_rssi
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_ND_OFFLOAD,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV |
			 WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_set_ns_offload
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_GET_LOGGER_FEATURE_SET,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = wlan_hdd_cfg80211_get_logger_supp_feature
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_TRIGGER_SCAN,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_vendor_scan
	},

	/* Vendor abort scan */
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_ABORT_SCAN,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_vendor_abort_scan
	},

	/* OCB commands */
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_OCB_SET_CONFIG,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
				 WIPHY_VENDOR_CMD_NEED_NETDEV |
				 WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_ocb_set_config
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_OCB_SET_UTC_TIME,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
				 WIPHY_VENDOR_CMD_NEED_NETDEV |
				 WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_ocb_set_utc_time
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_OCB_START_TIMING_ADVERT,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
				 WIPHY_VENDOR_CMD_NEED_NETDEV |
				 WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_ocb_start_timing_advert
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_OCB_STOP_TIMING_ADVERT,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
				 WIPHY_VENDOR_CMD_NEED_NETDEV |
				 WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_ocb_stop_timing_advert
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_OCB_GET_TSF_TIMER,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
				 WIPHY_VENDOR_CMD_NEED_NETDEV |
				 WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_ocb_get_tsf_timer
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_DCC_GET_STATS,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
				 WIPHY_VENDOR_CMD_NEED_NETDEV |
				 WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_dcc_get_stats
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_DCC_CLEAR_STATS,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
				 WIPHY_VENDOR_CMD_NEED_NETDEV |
				 WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_dcc_clear_stats
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_DCC_UPDATE_NDL,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
				 WIPHY_VENDOR_CMD_NEED_NETDEV |
				 WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_dcc_update_ndl
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_LINK_PROPERTIES,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV |
			 WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_get_link_properties
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_OTA_TEST,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
				 WIPHY_VENDOR_CMD_NEED_NETDEV |
				 WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_set_ota_test
	},
#ifdef FEATURE_LFR_SUBNET_DETECTION
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_GW_PARAM_CONFIG,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
				WIPHY_VENDOR_CMD_NEED_NETDEV |
				WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_set_gateway_params
	},
#endif /* FEATURE_LFR_SUBNET_DETECTION */
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_SET_TXPOWER_SCALE,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
				 WIPHY_VENDOR_CMD_NEED_NETDEV |
				 WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_txpower_scale
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_SET_TXPOWER_SCALE_DECR_DB,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
				 WIPHY_VENDOR_CMD_NEED_NETDEV |
				 WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_txpower_scale_decr_db
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_PACKET_FILTER,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_bpf_offload
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_ACS_POLICY,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_acs_dfs_mode
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_STA_CONNECT_ROAM_POLICY,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_sta_roam_policy
	},
#ifdef FEATURE_WLAN_CH_AVOID
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_AVOID_FREQUENCY,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_avoid_freq
	},
#endif
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_SET_SAP_CONFIG,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_sap_configuration_set
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_P2P_LISTEN_OFFLOAD_START,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
				WIPHY_VENDOR_CMD_NEED_NETDEV |
				WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_p2p_lo_start
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_P2P_LISTEN_OFFLOAD_STOP,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
				WIPHY_VENDOR_CMD_NEED_NETDEV |
				WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_p2p_lo_stop
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_SAP_CONDITIONAL_CHAN_SWITCH,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
				WIPHY_VENDOR_CMD_NEED_NETDEV |
				WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_conditional_chan_switch
	},
#ifdef WLAN_FEATURE_NAN_DATAPATH
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_NDP,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_process_ndp_cmd
	},
#endif
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_GET_WAKE_REASON_STATS,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV |
			 WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_get_wakelock_stats
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_GET_BUS_SIZE,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV |
			 WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_get_bus_size
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_EXTERNAL_ACS,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV |
			 WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_update_vendor_channel
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_SETBAND,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
					WIPHY_VENDOR_CMD_NEED_NETDEV |
					WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_setband
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_ROAMING,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_set_fast_roaming
	},
#ifdef WLAN_FEATURE_DISA
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_ENCRYPTION_TEST,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
				 WIPHY_VENDOR_CMD_NEED_NETDEV |
				 WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_encrypt_decrypt_msg
	},
#endif
#ifdef FEATURE_WLAN_TDLS
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_CONFIGURE_TDLS,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV |
			 WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_configure_tdls_mode
	},
#endif
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_GET_SAR_LIMITS,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_get_sar_power_limits
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_SET_SAR_LIMITS,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_set_sar_power_limits
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_SET_TRACE_LEVEL,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV |
			 WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_set_trace_level
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_LL_STATS_EXT,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
				 WIPHY_VENDOR_CMD_NEED_NETDEV |
				 WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_ll_stats_ext_set_param
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_NUD_STATS_SET,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_set_nud_stats
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_NUD_STATS_GET,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_get_nud_stats
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_FETCH_BSS_TRANSITION_STATUS,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV |
			 WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_fetch_bss_transition_status
	},
	FEATURE_SPECTRAL_SCAN_VENDOR_COMMANDS
#ifdef WLAN_UMAC_CONVERGENCE
	COMMON_VENDOR_COMMANDS
#endif
	FEATURE_11AX_VENDOR_COMMANDS

	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_GET_CHAIN_RSSI,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_get_chain_rssi
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_ACTIVE_TOS,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_set_limit_offchan_param
	},

};

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0)
static inline void
hdd_wiphy_set_max_sched_scans(struct wiphy *wiphy, uint8_t max_scans)
{
	if (max_scans == 0)
		wiphy->flags &= ~WIPHY_FLAG_SUPPORTS_SCHED_SCAN;
	else
		wiphy->flags |= WIPHY_FLAG_SUPPORTS_SCHED_SCAN;
}
#else
static inline void
hdd_wiphy_set_max_sched_scans(struct wiphy *wiphy, uint8_t max_scans)
{
	wiphy->max_sched_scan_reqs = max_scans;
}
#endif /* KERNEL_VERSION(4, 12, 0) */

/**
 * wlan_hdd_cfg80211_add_connected_pno_support() - Set connected PNO support
 * @wiphy: Pointer to wireless phy
 *
 * This function is used to set connected PNO support to kernel
 *
 * Return: None
 */
#if defined(CFG80211_REPORT_BETTER_BSS_IN_SCHED_SCAN)
static void wlan_hdd_cfg80211_add_connected_pno_support(struct wiphy *wiphy)
{
	wiphy_ext_feature_set(wiphy,
		NL80211_EXT_FEATURE_SCHED_SCAN_RELATIVE_RSSI);
}
#else
static void wlan_hdd_cfg80211_add_connected_pno_support(struct wiphy *wiphy)
{
}
#endif

#if ((LINUX_VERSION_CODE > KERNEL_VERSION(4, 4, 0)) || \
	defined(CFG80211_MULTI_SCAN_PLAN_BACKPORT)) && \
	defined(FEATURE_WLAN_SCAN_PNO)
/**
 * hdd_config_sched_scan_plans_to_wiphy() - configure sched scan plans to wiphy
 * @wiphy: pointer to wiphy
 * @config: pointer to config
 *
 * Return: None
 */
static void hdd_config_sched_scan_plans_to_wiphy(struct wiphy *wiphy,
						 struct hdd_config *config)
{
	if (config->configPNOScanSupport) {
		hdd_wiphy_set_max_sched_scans(wiphy, 1);
		wiphy->max_sched_scan_ssids = SCAN_PNO_MAX_SUPP_NETWORKS;
		wiphy->max_match_sets = SCAN_PNO_MAX_SUPP_NETWORKS;
		wiphy->max_sched_scan_ie_len = SIR_MAC_MAX_IE_LENGTH;
		wiphy->max_sched_scan_plans = SCAN_PNO_MAX_PLAN_REQUEST;
		if (config->max_sched_scan_plan_interval)
			wiphy->max_sched_scan_plan_interval =
				config->max_sched_scan_plan_interval;
		if (config->max_sched_scan_plan_iterations)
			wiphy->max_sched_scan_plan_iterations =
				config->max_sched_scan_plan_iterations;
	}
}
#else
static void hdd_config_sched_scan_plans_to_wiphy(struct wiphy *wiphy,
						 struct hdd_config *config)
{
}
#endif


/**
 * hdd_cfg80211_wiphy_alloc() - Allocate wiphy context
 * @priv_size:         Size of the hdd context.
 *
 * Allocate wiphy context and hdd context.
 *
 * Return: hdd context on success and NULL on failure.
 */
struct hdd_context *hdd_cfg80211_wiphy_alloc(int priv_size)
{
	struct wiphy *wiphy;
	struct hdd_context *hdd_ctx;

	ENTER();

	wiphy = wiphy_new(&wlan_hdd_cfg80211_ops, priv_size);

	if (!wiphy) {
		hdd_err("wiphy init failed!");
		return NULL;
	}

	hdd_ctx = wiphy_priv(wiphy);

	hdd_ctx->wiphy = wiphy;

	return hdd_ctx;
}

/*
 * FUNCTION: wlan_hdd_cfg80211_update_band
 * This function is called from the supplicant through a
 * private ioctl to change the band value
 */
int wlan_hdd_cfg80211_update_band(struct hdd_context *hdd_ctx, struct wiphy *wiphy,
		enum band_info eBand)
{
	int i, j;
	enum channel_state channelEnabledState;

	ENTER();

	for (i = 0; i < HDD_NUM_NL80211_BANDS; i++) {

		if (NULL == wiphy->bands[i])
			continue;

		for (j = 0; j < wiphy->bands[i]->n_channels; j++) {
			struct ieee80211_supported_band *band = wiphy->bands[i];

			channelEnabledState = wlan_reg_get_channel_state(
					hdd_ctx->hdd_pdev,
					band->channels[j].hw_value);

			if (HDD_NL80211_BAND_2GHZ == i &&
				BAND_5G == eBand) {
				/* 5G only */
#ifdef WLAN_ENABLE_SOCIAL_CHANNELS_5G_ONLY
				/* Enable Social channels for P2P */
				if (WLAN_HDD_IS_SOCIAL_CHANNEL
					    (band->channels[j].center_freq)
				    && CHANNEL_STATE_ENABLE ==
				    channelEnabledState)
					band->channels[j].flags &=
						~IEEE80211_CHAN_DISABLED;
				else
#endif
				band->channels[j].flags |=
					IEEE80211_CHAN_DISABLED;
				continue;
			} else if (HDD_NL80211_BAND_5GHZ == i &&
					BAND_2G == eBand) {
				/* 2G only */
				band->channels[j].flags |=
					IEEE80211_CHAN_DISABLED;
				continue;
			}

			if (CHANNEL_STATE_DISABLE != channelEnabledState)
				band->channels[j].flags &=
					~IEEE80211_CHAN_DISABLED;
		}
	}
	return 0;
}

#if defined(CFG80211_SCAN_RANDOM_MAC_ADDR) || \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
static void wlan_hdd_cfg80211_scan_randomization_init(struct wiphy *wiphy)
{
	struct hdd_context *hdd_ctx;

	hdd_ctx = wiphy_priv(wiphy);

	if (false == hdd_ctx->config->enable_mac_spoofing) {
		hdd_warn("MAC address spoofing is not enabled");
	} else {
		wiphy->features |= NL80211_FEATURE_SCAN_RANDOM_MAC_ADDR;
		wiphy->features |= NL80211_FEATURE_SCHED_SCAN_RANDOM_MAC_ADDR;
	}
}
#else
static void wlan_hdd_cfg80211_scan_randomization_init(struct wiphy *wiphy)
{
}
#endif

#define WLAN_HDD_MAX_NUM_CSA_COUNTERS 2

#if defined(WLAN_FEATURE_FILS_SK) && \
	(defined(CFG80211_FILS_SK_OFFLOAD_SUPPORT) || \
		 (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)))
static void wlan_hdd_cfg80211_set_wiphy_fils_feature(struct wiphy *wiphy)
{
	wiphy_ext_feature_set(wiphy, NL80211_EXT_FEATURE_FILS_SK_OFFLOAD);
}
#else
static void wlan_hdd_cfg80211_set_wiphy_fils_feature(struct wiphy *wiphy)
{
}
#endif

#ifdef WLAN_FEATURE_SAE
/**
 * wlan_hdd_cfg80211_set_wiphy_sae_feature() - Indicates support of SAE feature
 * @wiphy: Pointer to wiphy
 * @config: pointer to config
 *
 * This function is used to indicate the support of SAE
 *
 * Return: None
 */
static void wlan_hdd_cfg80211_set_wiphy_sae_feature(struct wiphy *wiphy,
			struct hdd_config *config)
{
	if (config->is_sae_enabled)
		wiphy->features |= NL80211_FEATURE_SAE;
}
#else
static void wlan_hdd_cfg80211_set_wiphy_sae_feature(struct wiphy *wiphy,
			struct hdd_config *config)
{
}
#endif

/*
 * FUNCTION: wlan_hdd_cfg80211_init
 * This function is called by hdd_wlan_startup()
 * during initialization.
 * This function is used to initialize and register wiphy structure.
 */
int wlan_hdd_cfg80211_init(struct device *dev,
			   struct wiphy *wiphy, struct hdd_config *pCfg)
{
	int i, j;
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	int len = 0;
	uint32_t *cipher_suites;

	ENTER();

	/* Now bind the underlying wlan device with wiphy */
	set_wiphy_dev(wiphy, dev);

	wiphy->mgmt_stypes = wlan_hdd_txrx_stypes;

	wiphy->flags |= WIPHY_FLAG_HAVE_AP_SME
			| WIPHY_FLAG_AP_PROBE_RESP_OFFLOAD
			| WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL
#ifdef FEATURE_WLAN_STA_4ADDR_SCHEME
			| WIPHY_FLAG_4ADDR_STATION
#endif
			| WIPHY_FLAG_OFFCHAN_TX;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0))
	wiphy->wowlan = &wowlan_support_cfg80211_init;
#else
	wiphy->wowlan.flags = WIPHY_WOWLAN_MAGIC_PKT;
	wiphy->wowlan.n_patterns = WOWL_MAX_PTRNS_ALLOWED;
	wiphy->wowlan.pattern_min_len = 1;
	wiphy->wowlan.pattern_max_len = WOWL_PTRN_MAX_SIZE;
#endif

	if (pCfg->isFastTransitionEnabled || pCfg->isFastRoamIniFeatureEnabled
#ifdef FEATURE_WLAN_ESE
	    || pCfg->isEseIniFeatureEnabled
#endif
	    ) {
		wiphy->flags |= WIPHY_FLAG_SUPPORTS_FW_ROAM;
	}
#ifdef FEATURE_WLAN_TDLS
	wiphy->flags |= WIPHY_FLAG_SUPPORTS_TDLS
			| WIPHY_FLAG_TDLS_EXTERNAL_SETUP;
#endif

	wiphy->features |= NL80211_FEATURE_HT_IBSS;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
	wiphy_ext_feature_set(wiphy, NL80211_EXT_FEATURE_VHT_IBSS);
#endif
	if (pCfg->is_fils_enabled)
		wlan_hdd_cfg80211_set_wiphy_fils_feature(wiphy);

	wlan_hdd_cfg80211_set_wiphy_sae_feature(wiphy, pCfg);

	hdd_config_sched_scan_plans_to_wiphy(wiphy, pCfg);
	wlan_hdd_cfg80211_add_connected_pno_support(wiphy);

	wiphy->max_scan_ssids = MAX_SCAN_SSID;

	wiphy->max_scan_ie_len = SIR_MAC_MAX_ADD_IE_LENGTH;

	wiphy->max_acl_mac_addrs = MAX_ACL_MAC_ADDRESS;

	wiphy->interface_modes = BIT(NL80211_IFTYPE_STATION)
				 | BIT(NL80211_IFTYPE_ADHOC)
				 | BIT(NL80211_IFTYPE_P2P_CLIENT)
				 | BIT(NL80211_IFTYPE_P2P_GO)
				 | BIT(NL80211_IFTYPE_AP)
				 | BIT(NL80211_IFTYPE_MONITOR);

	if (pCfg->advertiseConcurrentOperation) {
		if (pCfg->enableMCC) {
			int i;

			for (i = 0;
			     i < ARRAY_SIZE(wlan_hdd_iface_combination);
			     i++) {
				if (!pCfg->allowMCCGODiffBI)
					wlan_hdd_iface_combination[i].
					beacon_int_infra_match = true;
			}
		}
		wiphy->n_iface_combinations =
			ARRAY_SIZE(wlan_hdd_iface_combination);
		wiphy->iface_combinations = wlan_hdd_iface_combination;
	}

	/* Before registering we need to update the ht capabilitied based
	 * on ini values
	 */
	if (!pCfg->ShortGI20MhzEnable) {
		wlan_hdd_band_2_4_ghz.ht_cap.cap &= ~IEEE80211_HT_CAP_SGI_20;
		wlan_hdd_band_5_ghz.ht_cap.cap &= ~IEEE80211_HT_CAP_SGI_20;
	}

	if (!pCfg->ShortGI40MhzEnable)
		wlan_hdd_band_5_ghz.ht_cap.cap &=
			~IEEE80211_HT_CAP_SGI_40;

	if (!pCfg->nChannelBondingMode5GHz)
		wlan_hdd_band_5_ghz.ht_cap.cap &=
			~IEEE80211_HT_CAP_SUP_WIDTH_20_40;

	/*
	 * In case of static linked driver at the time of driver unload,
	 * module exit doesn't happens. Module cleanup helps in cleaning
	 * of static memory.
	 * If driver load happens statically, at the time of driver unload,
	 * wiphy flags don't get reset because of static memory.
	 * It's better not to store channel in static memory.
	 */
	wiphy->bands[HDD_NL80211_BAND_2GHZ] = &wlan_hdd_band_2_4_ghz;
	wiphy->bands[HDD_NL80211_BAND_2GHZ]->channels =
		qdf_mem_malloc(sizeof(hdd_channels_2_4_ghz));
	if (wiphy->bands[HDD_NL80211_BAND_2GHZ]->channels == NULL) {
		hdd_err("Not enough memory to allocate channels");
		return -ENOMEM;
	}
	qdf_mem_copy(wiphy->bands[HDD_NL80211_BAND_2GHZ]->channels,
			&hdd_channels_2_4_ghz[0],
			sizeof(hdd_channels_2_4_ghz));
	if ((hdd_is_5g_supported(hdd_ctx)) &&
		((eHDD_DOT11_MODE_11b != pCfg->dot11Mode) &&
		 (eHDD_DOT11_MODE_11g != pCfg->dot11Mode) &&
		 (eHDD_DOT11_MODE_11b_ONLY != pCfg->dot11Mode) &&
		 (eHDD_DOT11_MODE_11g_ONLY != pCfg->dot11Mode))) {
		wiphy->bands[HDD_NL80211_BAND_5GHZ] = &wlan_hdd_band_5_ghz;

		if (pCfg->dot11p_mode) {
			wiphy->bands[HDD_NL80211_BAND_5GHZ]->channels =
				qdf_mem_malloc(sizeof(hdd_channels_5_ghz) +
						sizeof(hdd_channels_dot11p));
			if (wiphy->bands[HDD_NL80211_BAND_5GHZ]->channels ==
								NULL) {
				goto mem_fail;
			}
			wiphy->bands[HDD_NL80211_BAND_5GHZ]->n_channels =
					QDF_ARRAY_SIZE(hdd_channels_5_ghz) +
					QDF_ARRAY_SIZE(hdd_channels_dot11p);

			qdf_mem_copy(wiphy->bands[HDD_NL80211_BAND_5GHZ]->
					channels, &hdd_channels_5_ghz[0],
					sizeof(hdd_channels_5_ghz));
			len = sizeof(hdd_channels_5_ghz);
			qdf_mem_copy((char *)wiphy->
					bands[HDD_NL80211_BAND_5GHZ]->channels
						+ len,
						&hdd_channels_dot11p[0],
					sizeof(hdd_channels_dot11p));

		} else {
			wiphy->bands[HDD_NL80211_BAND_5GHZ]->channels =
			qdf_mem_malloc(sizeof(hdd_channels_5_ghz));
			if (wiphy->bands[HDD_NL80211_BAND_5GHZ]->channels ==
								NULL) {
				goto mem_fail;
			}
			qdf_mem_copy(wiphy->
				bands[HDD_NL80211_BAND_5GHZ]->channels,
				&hdd_channels_5_ghz[0],
				sizeof(hdd_channels_5_ghz));
		}
	}

	for (i = 0; i < HDD_NUM_NL80211_BANDS; i++) {

		if (NULL == wiphy->bands[i])
			continue;

		for (j = 0; j < wiphy->bands[i]->n_channels; j++) {
			struct ieee80211_supported_band *band = wiphy->bands[i];

			if (HDD_NL80211_BAND_2GHZ == i &&
				BAND_5G == pCfg->nBandCapability) {
				/* 5G only */
#ifdef WLAN_ENABLE_SOCIAL_CHANNELS_5G_ONLY
				/* Enable social channels for P2P */
				if (WLAN_HDD_IS_SOCIAL_CHANNEL
					    (band->channels[j].center_freq))
					band->channels[j].flags &=
						~IEEE80211_CHAN_DISABLED;
				else
#endif
				band->channels[j].flags |=
					IEEE80211_CHAN_DISABLED;
				continue;
			} else if (HDD_NL80211_BAND_5GHZ == i &&
					BAND_2G == pCfg->nBandCapability) {
				/* 2G only */
				band->channels[j].flags |=
					IEEE80211_CHAN_DISABLED;
				continue;
			}
		}
	}
	/*Initialise the supported cipher suite details */
	if (pCfg->gcmp_enabled) {
		cipher_suites = qdf_mem_malloc(sizeof(hdd_cipher_suites) +
					       sizeof(hdd_gcmp_cipher_suits));
		if (cipher_suites == NULL) {
			hdd_err("Not enough memory for cipher suites");
			return -ENOMEM;
		}
		wiphy->n_cipher_suites = QDF_ARRAY_SIZE(hdd_cipher_suites) +
			 QDF_ARRAY_SIZE(hdd_gcmp_cipher_suits);
		qdf_mem_copy(cipher_suites, &hdd_cipher_suites,
			     sizeof(hdd_cipher_suites));
		qdf_mem_copy(cipher_suites + QDF_ARRAY_SIZE(hdd_cipher_suites),
			     &hdd_gcmp_cipher_suits,
			     sizeof(hdd_gcmp_cipher_suits));
	} else {
		cipher_suites = qdf_mem_malloc(sizeof(hdd_cipher_suites));
		if (cipher_suites == NULL) {
			hdd_err("Not enough memory for cipher suites");
			return -ENOMEM;
		}
		wiphy->n_cipher_suites = QDF_ARRAY_SIZE(hdd_cipher_suites);
		qdf_mem_copy(cipher_suites, &hdd_cipher_suites,
			     sizeof(hdd_cipher_suites));
	}
	wiphy->cipher_suites = cipher_suites;
	cipher_suites = NULL;
	/*signal strength in mBm (100*dBm) */
	wiphy->signal_type = CFG80211_SIGNAL_TYPE_MBM;
	wiphy->max_remain_on_channel_duration = MAX_REMAIN_ON_CHANNEL_DURATION;

	if (cds_get_conparam() != QDF_GLOBAL_FTM_MODE) {
		wiphy->n_vendor_commands =
				ARRAY_SIZE(hdd_wiphy_vendor_commands);
		wiphy->vendor_commands = hdd_wiphy_vendor_commands;

		wiphy->vendor_events = wlan_hdd_cfg80211_vendor_events;
		wiphy->n_vendor_events =
				ARRAY_SIZE(wlan_hdd_cfg80211_vendor_events);
	}

	if (pCfg->enableDFSMasterCap)
		wiphy->flags |= WIPHY_FLAG_DFS_OFFLOAD;

	wiphy->max_ap_assoc_sta = pCfg->maxNumberOfPeers;

#ifdef QCA_HT_2040_COEX
	wiphy->features |= NL80211_FEATURE_AP_MODE_CHAN_WIDTH_CHANGE;
#endif
	wiphy->features |= NL80211_FEATURE_INACTIVITY_TIMER;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)) || \
	defined(CFG80211_BEACON_TX_RATE_CUSTOM_BACKPORT)
	wiphy_ext_feature_set(wiphy, NL80211_EXT_FEATURE_BEACON_RATE_LEGACY);
	wiphy_ext_feature_set(wiphy, NL80211_EXT_FEATURE_BEACON_RATE_HT);
	wiphy_ext_feature_set(wiphy, NL80211_EXT_FEATURE_BEACON_RATE_VHT);
#endif

	hdd_add_channel_switch_support(&wiphy->flags);
	wiphy->max_num_csa_counters = WLAN_HDD_MAX_NUM_CSA_COUNTERS;
	if (pCfg->enable_mac_spoofing)
		wlan_hdd_cfg80211_scan_randomization_init(wiphy);

	EXIT();
	return 0;

mem_fail:
	hdd_err("Not enough memory to allocate channels");
	if (wiphy->bands[HDD_NL80211_BAND_2GHZ]->channels != NULL) {
		qdf_mem_free(wiphy->bands[HDD_NL80211_BAND_2GHZ]->channels);
		wiphy->bands[HDD_NL80211_BAND_2GHZ]->channels = NULL;
	}
	return -ENOMEM;
}

/**
 * wlan_hdd_cfg80211_deinit() - Deinit cfg80211
 * @wiphy: the wiphy to validate against
 *
 * this function deinit cfg80211 and cleanup the
 * memory allocated in wlan_hdd_cfg80211_init also
 * reset the global reg params.
 *
 * Return: void
 */
void wlan_hdd_cfg80211_deinit(struct wiphy *wiphy)
{
	int i;
	const uint32_t *cipher_suites;

	for (i = 0; i < HDD_NUM_NL80211_BANDS; i++) {
		if (NULL != wiphy->bands[i] &&
		   (NULL != wiphy->bands[i]->channels)) {
			qdf_mem_free(wiphy->bands[i]->channels);
			wiphy->bands[i]->channels = NULL;
		}
	}

	cipher_suites = wiphy->cipher_suites;
	wiphy->cipher_suites = NULL;
	wiphy->n_cipher_suites = 0;
	qdf_mem_free((uint32_t *)cipher_suites);
	cipher_suites = NULL;
	hdd_reset_global_reg_params();
}

/**
 * wlan_hdd_update_band_cap() - update capabilities for supported bands
 * @hdd_ctx: HDD context
 *
 * this function will update capabilities for supported bands
 *
 * Return: void
 */
static void wlan_hdd_update_band_cap(struct hdd_context *hdd_ctx)
{
	uint32_t val32;
	uint16_t val16;
	tSirMacHTCapabilityInfo *ht_cap_info;
	QDF_STATUS status;

	status = sme_cfg_get_int(hdd_ctx->hHal, WNI_CFG_HT_CAP_INFO, &val32);
	if (QDF_STATUS_SUCCESS != status) {
		hdd_err("could not get HT capability info");
		val32 = 0;
	}
	val16 = (uint16_t)val32;
	ht_cap_info = (tSirMacHTCapabilityInfo *)&val16;

	if (ht_cap_info->txSTBC == true) {
		if (NULL != hdd_ctx->wiphy->bands[HDD_NL80211_BAND_2GHZ])
			hdd_ctx->wiphy->bands[HDD_NL80211_BAND_2GHZ]->ht_cap.cap |=
						IEEE80211_HT_CAP_TX_STBC;
		if (NULL != hdd_ctx->wiphy->bands[HDD_NL80211_BAND_5GHZ])
			hdd_ctx->wiphy->bands[HDD_NL80211_BAND_5GHZ]->ht_cap.cap |=
						IEEE80211_HT_CAP_TX_STBC;
	}

	if (!sme_is_feature_supported_by_fw(DOT11AC)) {
		hdd_ctx->wiphy->bands[HDD_NL80211_BAND_2GHZ]->
						vht_cap.vht_supported = 0;
		hdd_ctx->wiphy->bands[HDD_NL80211_BAND_2GHZ]->vht_cap.cap = 0;
		hdd_ctx->wiphy->bands[HDD_NL80211_BAND_5GHZ]->
						vht_cap.vht_supported = 0;
		hdd_ctx->wiphy->bands[HDD_NL80211_BAND_5GHZ]->vht_cap.cap = 0;
	}
}

/*
 * In this function, wiphy structure is updated after QDF
 * initialization. In wlan_hdd_cfg80211_init, only the
 * default values will be initialized. The final initialization
 * of all required members can be done here.
 */
void wlan_hdd_update_wiphy(struct hdd_context *hdd_ctx)
{
	hdd_ctx->wiphy->max_ap_assoc_sta = hdd_ctx->config->maxNumberOfPeers;

	wlan_hdd_update_band_cap(hdd_ctx);
}

/**
 * wlan_hdd_update_11n_mode - update 11n mode in hdd cfg
 * @cfg: hdd cfg
 *
 * this function update 11n mode in hdd cfg
 *
 * Return: void
 */
void wlan_hdd_update_11n_mode(struct hdd_config *cfg)
{
	if (sme_is_feature_supported_by_fw(DOT11AC)) {
		hdd_debug("support 11ac");
	} else {
		hdd_debug("not support 11ac");
		if ((cfg->dot11Mode == eHDD_DOT11_MODE_11ac_ONLY) ||
		    (cfg->dot11Mode == eHDD_DOT11_MODE_11ac)) {
			cfg->dot11Mode = eHDD_DOT11_MODE_11n;
			cfg->sap_p2p_11ac_override = 0;
		}
	}
}

/* In this function we are registering wiphy. */
int wlan_hdd_cfg80211_register(struct wiphy *wiphy)
{
	ENTER();
	/* Register our wiphy dev with cfg80211 */
	if (0 > wiphy_register(wiphy)) {
		hdd_err("wiphy register failed");
		return -EIO;
	}

	EXIT();
	return 0;
}

/*
 * HDD function to update wiphy capability based on target offload status.
 *
 * wlan_hdd_cfg80211_init() does initialization of all wiphy related
 * capability even before downloading firmware to the target. In discrete
 * case, host will get know certain offload capability (say sched_scan
 * caps) only after downloading firmware to the target and target boots up.
 * This function is used to override setting done in wlan_hdd_cfg80211_init()
 * based on target capability.
 */
void wlan_hdd_cfg80211_update_wiphy_caps(struct wiphy *wiphy)
{
#ifdef FEATURE_WLAN_SCAN_PNO
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct hdd_config *pCfg = hdd_ctx->config;

	/* wlan_hdd_cfg80211_init() sets sched_scan caps already in wiphy before
	 * control comes here. Here just we need to clear it if firmware doesn't
	 * have PNO support.
	 */
	if (!pCfg->PnoOffload) {
		hdd_wiphy_set_max_sched_scans(wiphy, 0);
		wiphy->max_sched_scan_ssids = 0;
		wiphy->max_match_sets = 0;
		wiphy->max_sched_scan_ie_len = 0;
	}
#endif
}

/* This function registers for all frame which supplicant is interested in */
int wlan_hdd_cfg80211_register_frames(struct hdd_adapter *adapter)
{
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(adapter);
	/* Register for all P2P action, public action etc frames */
	uint16_t type = (SIR_MAC_MGMT_FRAME << 2) | (SIR_MAC_MGMT_ACTION << 4);
	QDF_STATUS status;

	ENTER();

	/* Register frame indication call back */
	status = sme_register_mgmt_frame_ind_callback(hHal,
			hdd_indicate_mgmt_frame);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("Failed to register hdd_indicate_mgmt_frame");
		goto ret_status;
	}

	/* Right now we are registering these frame when driver is getting
	 * initialized. Once we will move to 2.6.37 kernel, in which we have
	 * frame register ops, we will move this code as a part of that
	 */

	/* GAS Initial Request */
	status = sme_register_mgmt_frame(hHal, SME_SESSION_ID_ANY, type,
				(uint8_t *) GAS_INITIAL_REQ,
				GAS_INITIAL_REQ_SIZE);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("Failed to register GAS_INITIAL_REQ");
		goto ret_status;
	}

	/* GAS Initial Response */
	status = sme_register_mgmt_frame(hHal, SME_SESSION_ID_ANY, type,
				(uint8_t *) GAS_INITIAL_RSP,
				GAS_INITIAL_RSP_SIZE);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("Failed to register GAS_INITIAL_RSP");
		goto dereg_gas_initial_req;
	}

	/* GAS Comeback Request */
	status = sme_register_mgmt_frame(hHal, SME_SESSION_ID_ANY, type,
				(uint8_t *) GAS_COMEBACK_REQ,
				GAS_COMEBACK_REQ_SIZE);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("Failed to register GAS_COMEBACK_REQ");
		goto dereg_gas_initial_rsp;
	}

	/* GAS Comeback Response */
	status = sme_register_mgmt_frame(hHal, SME_SESSION_ID_ANY, type,
				(uint8_t *) GAS_COMEBACK_RSP,
				GAS_COMEBACK_RSP_SIZE);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("Failed to register GAS_COMEBACK_RSP");
		goto dereg_gas_comeback_req;
	}

	/* WNM BSS Transition Request frame */
	status = sme_register_mgmt_frame(hHal, SME_SESSION_ID_ANY, type,
				(uint8_t *) WNM_BSS_ACTION_FRAME,
				WNM_BSS_ACTION_FRAME_SIZE);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("Failed to register WNM_BSS_ACTION_FRAME");
		goto dereg_gas_comeback_rsp;
	}

	/* WNM-Notification */
	status = sme_register_mgmt_frame(hHal, adapter->session_id, type,
				(uint8_t *) WNM_NOTIFICATION_FRAME,
				WNM_NOTIFICATION_FRAME_SIZE);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("Failed to register WNM_NOTIFICATION_FRAME");
		goto dereg_wnm_bss_action_frm;
	}

dereg_wnm_bss_action_frm:
	sme_deregister_mgmt_frame(hHal, SME_SESSION_ID_ANY, type,
			(uint8_t *) WNM_BSS_ACTION_FRAME,
			WNM_BSS_ACTION_FRAME_SIZE);
dereg_gas_comeback_rsp:
	sme_deregister_mgmt_frame(hHal, SME_SESSION_ID_ANY, type,
			(uint8_t *) GAS_COMEBACK_RSP,
			GAS_COMEBACK_RSP_SIZE);
dereg_gas_comeback_req:
	sme_deregister_mgmt_frame(hHal, SME_SESSION_ID_ANY, type,
			(uint8_t *) GAS_COMEBACK_REQ,
			GAS_COMEBACK_REQ_SIZE);
dereg_gas_initial_rsp:
	sme_deregister_mgmt_frame(hHal, SME_SESSION_ID_ANY, type,
			(uint8_t *) GAS_INITIAL_RSP,
			GAS_INITIAL_RSP_SIZE);
dereg_gas_initial_req:
	sme_deregister_mgmt_frame(hHal, SME_SESSION_ID_ANY, type,
			(uint8_t *) GAS_INITIAL_REQ,
			GAS_INITIAL_REQ_SIZE);
ret_status:
	return qdf_status_to_os_return(status);
}

void wlan_hdd_cfg80211_deregister_frames(struct hdd_adapter *adapter)
{
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(adapter);
	/* Register for all P2P action, public action etc frames */
	uint16_t type = (SIR_MAC_MGMT_FRAME << 2) | (SIR_MAC_MGMT_ACTION << 4);

	ENTER();

	/* Right now we are registering these frame when driver is getting
	 * initialized. Once we will move to 2.6.37 kernel, in which we have
	 * frame register ops, we will move this code as a part of that
	 */

	/* GAS Initial Request */

	sme_deregister_mgmt_frame(hHal, SME_SESSION_ID_ANY, type,
				  (uint8_t *) GAS_INITIAL_REQ,
				  GAS_INITIAL_REQ_SIZE);

	/* GAS Initial Response */
	sme_deregister_mgmt_frame(hHal, SME_SESSION_ID_ANY, type,
				  (uint8_t *) GAS_INITIAL_RSP,
				  GAS_INITIAL_RSP_SIZE);

	/* GAS Comeback Request */
	sme_deregister_mgmt_frame(hHal, SME_SESSION_ID_ANY, type,
				  (uint8_t *) GAS_COMEBACK_REQ,
				  GAS_COMEBACK_REQ_SIZE);

	/* GAS Comeback Response */
	sme_deregister_mgmt_frame(hHal, SME_SESSION_ID_ANY, type,
				  (uint8_t *) GAS_COMEBACK_RSP,
				  GAS_COMEBACK_RSP_SIZE);

	/* P2P Public Action */
	sme_deregister_mgmt_frame(hHal, SME_SESSION_ID_ANY, type,
				  (uint8_t *) P2P_PUBLIC_ACTION_FRAME,
				  P2P_PUBLIC_ACTION_FRAME_SIZE);

	/* P2P Action */
	sme_deregister_mgmt_frame(hHal, SME_SESSION_ID_ANY, type,
				  (uint8_t *) P2P_ACTION_FRAME,
				  P2P_ACTION_FRAME_SIZE);

	/* WNM-Notification */
	sme_deregister_mgmt_frame(hHal, adapter->session_id, type,
				  (uint8_t *) WNM_NOTIFICATION_FRAME,
				  WNM_NOTIFICATION_FRAME_SIZE);
}

#ifdef FEATURE_WLAN_WAPI
void wlan_hdd_cfg80211_set_key_wapi(struct hdd_adapter *adapter,
				    uint8_t key_index,
				    const uint8_t *mac_addr, const uint8_t *key,
				    int key_Len)
{
	tCsrRoamSetKey setKey;
	bool isConnected = true;
	int status = 0;
	uint32_t roamId = INVALID_ROAM_ID;
	uint8_t *pKeyPtr = NULL;

	hdd_debug("Device_mode %s(%d)",
		hdd_device_mode_to_string(adapter->device_mode),
		adapter->device_mode);

	qdf_mem_zero(&setKey, sizeof(tCsrRoamSetKey));
	setKey.keyId = key_index;       /* Store Key ID */
	setKey.encType = eCSR_ENCRYPT_TYPE_WPI; /* SET WAPI Encryption */
	setKey.keyDirection = eSIR_TX_RX;       /* Key Directionn both TX and RX */
	setKey.paeRole = 0;     /* the PAE role */
	if (!mac_addr || is_broadcast_ether_addr(mac_addr))
		qdf_set_macaddr_broadcast(&setKey.peerMac);
	else
		qdf_mem_copy(setKey.peerMac.bytes, mac_addr, QDF_MAC_ADDR_SIZE);

	setKey.keyLength = key_Len;
	pKeyPtr = setKey.Key;
	memcpy(pKeyPtr, key, key_Len);

	hdd_debug("WAPI KEY LENGTH:0x%04x", key_Len);

	if (isConnected) {
		status = sme_roam_set_key(WLAN_HDD_GET_HAL_CTX(adapter),
					  adapter->session_id, &setKey, &roamId);
		if (status != 0)
			hdd_err("sme_roam_set_key failed status: %d", status);
	}
}
#endif /* FEATURE_WLAN_WAPI */

bool wlan_hdd_is_ap_supports_immediate_power_save(uint8_t *ies, int length)
{
	const uint8_t *vendor_ie;

	if (length < 2) {
		hdd_debug("bss size is less than expected");
		return true;
	}
	if (!ies) {
		hdd_debug("invalid IE pointer");
		return true;
	}
	vendor_ie = wlan_get_vendor_ie_ptr_from_oui(VENDOR1_AP_OUI_TYPE,
				VENDOR1_AP_OUI_TYPE_SIZE, ies, length);
	if (vendor_ie) {
		hdd_debug("AP can't support immediate powersave. defer it");
		return false;
	}
	return true;
}

/*
 * FUNCTION: wlan_hdd_validate_operation_channel
 * called by wlan_hdd_cfg80211_start_bss() and
 * wlan_hdd_set_channel()
 * This function validates whether given channel is part of valid
 * channel list.
 */
QDF_STATUS wlan_hdd_validate_operation_channel(struct hdd_adapter *adapter,
					       int channel)
{

	uint32_t num_ch = 0;
	u8 valid_ch[WNI_CFG_VALID_CHANNEL_LIST_LEN];
	u32 indx = 0;
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(adapter);
	uint8_t fValidChannel = false, count = 0;
	struct hdd_config *hdd_pConfig_ini = (WLAN_HDD_GET_CTX(adapter))->config;

	num_ch = WNI_CFG_VALID_CHANNEL_LIST_LEN;

	if (hdd_pConfig_ini->sapAllowAllChannel) {
		/* Validate the channel */
		for (count = CHAN_ENUM_1; count <= CHAN_ENUM_165; count++) {
			if (channel == WLAN_REG_CH_NUM(count)) {
				fValidChannel = true;
				break;
			}
		}
		if (fValidChannel != true) {
			hdd_err("Invalid Channel: %d", channel);
			return QDF_STATUS_E_FAILURE;
		}
	} else {
		if (0 != sme_cfg_get_str(hHal, WNI_CFG_VALID_CHANNEL_LIST,
					 valid_ch, &num_ch)) {
			hdd_err("failed to get valid channel list");
			return QDF_STATUS_E_FAILURE;
		}
		for (indx = 0; indx < num_ch; indx++) {
			if (channel == valid_ch[indx])
				break;
		}

		if (indx >= num_ch) {
			hdd_err("Invalid Channel: %d", channel);
			return QDF_STATUS_E_FAILURE;
		}
	}
	return QDF_STATUS_SUCCESS;

}

#ifdef DHCP_SERVER_OFFLOAD
static void wlan_hdd_set_dhcp_server_offload(struct hdd_adapter *adapter)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	tpSirDhcpSrvOffloadInfo pDhcpSrvInfo;
	uint8_t numEntries = 0;
	uint8_t srv_ip[IPADDR_NUM_ENTRIES];
	uint8_t num;
	uint32_t temp;

	pDhcpSrvInfo = qdf_mem_malloc(sizeof(*pDhcpSrvInfo));
	if (NULL == pDhcpSrvInfo) {
		hdd_err("could not allocate tDhcpSrvOffloadInfo!");
		return;
	}
	pDhcpSrvInfo->vdev_id = adapter->session_id;
	pDhcpSrvInfo->dhcpSrvOffloadEnabled = true;
	pDhcpSrvInfo->dhcpClientNum = hdd_ctx->config->dhcpMaxNumClients;
	hdd_string_to_u8_array(hdd_ctx->config->dhcpServerIP,
			       srv_ip, &numEntries, IPADDR_NUM_ENTRIES);
	if (numEntries != IPADDR_NUM_ENTRIES) {
		hdd_err("Incorrect IP address (%s) assigned for DHCP server!", hdd_ctx->config->dhcpServerIP);
		goto end;
	}
	if ((srv_ip[0] >= 224) && (srv_ip[0] <= 239)) {
		hdd_err("Invalid IP address (%s)! It could NOT be multicast IP address!", hdd_ctx->config->dhcpServerIP);
		goto end;
	}
	if (srv_ip[IPADDR_NUM_ENTRIES - 1] >= 100) {
		hdd_err("Invalid IP address (%s)! The last field must be less than 100!", hdd_ctx->config->dhcpServerIP);
		goto end;
	}
	for (num = 0; num < numEntries; num++) {
		temp = srv_ip[num];
		pDhcpSrvInfo->dhcpSrvIP |= (temp << (8 * num));
	}
	if (QDF_STATUS_SUCCESS !=
	    sme_set_dhcp_srv_offload(hdd_ctx->hHal, pDhcpSrvInfo)) {
		hdd_err("sme_setDHCPSrvOffload fail!");
		goto end;
	}
	hdd_debug("enable DHCP Server offload successfully!");
end:
	qdf_mem_free(pDhcpSrvInfo);
}
#endif /* DHCP_SERVER_OFFLOAD */

static int __wlan_hdd_cfg80211_change_bss(struct wiphy *wiphy,
					  struct net_device *dev,
					  struct bss_parameters *params)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	int ret = 0;
	QDF_STATUS qdf_ret_status;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(adapter->session_id)) {
		hdd_err("invalid session id: %d", adapter->session_id);
		return -EINVAL;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_CHANGE_BSS,
			 adapter->session_id, params->ap_isolate));
	hdd_debug("Device_mode %s(%d), ap_isolate = %d",
		hdd_device_mode_to_string(adapter->device_mode),
		adapter->device_mode, params->ap_isolate);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	if (!(adapter->device_mode == QDF_SAP_MODE ||
	      adapter->device_mode == QDF_P2P_GO_MODE)) {
		return -EOPNOTSUPP;
	}

	/* ap_isolate == -1 means that in change bss, upper layer doesn't
	 * want to update this parameter
	 */
	if (-1 != params->ap_isolate) {
		adapter->session.ap.disable_intrabss_fwd =
			!!params->ap_isolate;

		qdf_ret_status = sme_ap_disable_intra_bss_fwd(hdd_ctx->hHal,
							      adapter->session_id,
							      adapter->session.
							      ap.
							      disable_intrabss_fwd);
		if (!QDF_IS_STATUS_SUCCESS(qdf_ret_status))
			ret = -EINVAL;
	}

	EXIT();
	return ret;
}

/**
 * wlan_hdd_change_client_iface_to_new_mode() - to change iface to provided mode
 * @ndev: pointer to net device provided by supplicant
 * @type: type of the interface, upper layer wanted to change
 *
 * Upper layer provides the new interface mode that needs to be changed
 * for given net device
 *
 * Return: success or failure in terms of integer value
 */
static int wlan_hdd_change_client_iface_to_new_mode(struct net_device *ndev,
					     enum nl80211_iftype type)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(ndev);
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct hdd_config *config = hdd_ctx->config;
	struct hdd_wext_state *wext;
	struct wireless_dev *wdev;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	ENTER();

	if (test_bit(ACS_IN_PROGRESS, &hdd_ctx->g_event_flags)) {
		hdd_warn("ACS is in progress, don't change iface!");
		return -EBUSY;
	}

	wdev = ndev->ieee80211_ptr;
	hdd_stop_adapter(hdd_ctx, adapter);
	hdd_deinit_adapter(hdd_ctx, adapter, true);
	wdev->iftype = type;
	/*Check for sub-string p2p to confirm its a p2p interface */
	if (NULL != strnstr(ndev->name, "p2p", 3)) {
		adapter->device_mode =
			(type == NL80211_IFTYPE_STATION) ?
			QDF_P2P_DEVICE_MODE : QDF_P2P_CLIENT_MODE;
	} else if (type == NL80211_IFTYPE_ADHOC) {
		adapter->device_mode = QDF_IBSS_MODE;
	} else {
		adapter->device_mode =
			(type == NL80211_IFTYPE_STATION) ?
			QDF_STA_MODE : QDF_P2P_CLIENT_MODE;
	}
	memset(&adapter->session, 0, sizeof(adapter->session));
	hdd_set_station_ops(adapter->dev);
	wext = WLAN_HDD_GET_WEXT_STATE_PTR(adapter);
	wext->roamProfile.pAddIEScan = adapter->scan_info.scan_add_ie.addIEdata;
	wext->roamProfile.nAddIEScanLength =
		adapter->scan_info.scan_add_ie.length;
	if (type == NL80211_IFTYPE_ADHOC) {
		status = hdd_start_station_adapter(adapter);
		wext->roamProfile.BSSType = eCSR_BSS_TYPE_START_IBSS;
		wext->roamProfile.phyMode =
			hdd_cfg_xlate_to_csr_phy_mode(config->dot11Mode);
	}
	EXIT();

	return qdf_status_to_os_return(status);
}

static int wlan_hdd_cfg80211_change_bss(struct wiphy *wiphy,
					struct net_device *dev,
					struct bss_parameters *params)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_change_bss(wiphy, dev, params);
	cds_ssr_unprotect(__func__);

	return ret;
}

/* FUNCTION: wlan_hdd_change_country_code_cd
 *  to wait for contry code completion
 */
void *wlan_hdd_change_country_code_cb(void *adapter)
{
	struct hdd_adapter *call_back_adapter = adapter;

	complete(&call_back_adapter->change_country_code);
	return NULL;
}

/**
 * __wlan_hdd_cfg80211_change_iface() - change interface cfg80211 op
 * @wiphy: Pointer to the wiphy structure
 * @ndev: Pointer to the net device
 * @type: Interface type
 * @flags: Flags for change interface
 * @params: Pointer to change interface parameters
 *
 * Return: 0 for success, error number on failure.
 */
static int __wlan_hdd_cfg80211_change_iface(struct wiphy *wiphy,
					    struct net_device *ndev,
					    enum nl80211_iftype type,
					    u32 *flags,
					    struct vif_params *params)
{
	struct wireless_dev *wdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(ndev);
	struct hdd_context *hdd_ctx;
	tCsrRoamProfile *roam_profile = NULL;
	eCsrRoamBssType LastBSSType;
	struct hdd_config *pConfig = NULL;
	int status;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	status = wlan_hdd_validate_context(hdd_ctx);
	if (0 != status)
		return status;

	if (cds_is_fw_down()) {
		hdd_err("Ignore if FW is already down");
		return -EINVAL;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_CHANGE_IFACE,
			 adapter->session_id, type));

	hdd_debug("Device_mode = %d, IFTYPE = 0x%x",
	       adapter->device_mode, type);

	status = hdd_wlan_start_modules(hdd_ctx, adapter, false);
	if (status) {
		hdd_err("Failed to start modules");
		return -EINVAL;
	}

	if (!policy_mgr_allow_concurrency(hdd_ctx->hdd_psoc,
				wlan_hdd_convert_nl_iftype_to_hdd_type(type),
				0, HW_MODE_20_MHZ)) {
		hdd_debug("This concurrency combination is not allowed");
		return -EINVAL;
	}

	pConfig = hdd_ctx->config;
	wdev = ndev->ieee80211_ptr;

	/* Reset the current device mode bit mask */
	policy_mgr_clear_concurrency_mode(hdd_ctx->hdd_psoc,
		adapter->device_mode);

	hdd_notify_teardown_tdls_links(adapter->hdd_vdev);
	if ((adapter->device_mode == QDF_STA_MODE) ||
	    (adapter->device_mode == QDF_P2P_CLIENT_MODE) ||
	    (adapter->device_mode == QDF_P2P_DEVICE_MODE) ||
	    (adapter->device_mode == QDF_IBSS_MODE)) {
		struct hdd_wext_state *pWextState =
			WLAN_HDD_GET_WEXT_STATE_PTR(adapter);

		roam_profile = &pWextState->roamProfile;
		LastBSSType = roam_profile->BSSType;

		switch (type) {
		case NL80211_IFTYPE_STATION:
		case NL80211_IFTYPE_P2P_CLIENT:
		case NL80211_IFTYPE_ADHOC:
			if (type == NL80211_IFTYPE_ADHOC) {
				hdd_deregister_tx_flow_control(adapter);
				hdd_debug("Setting interface Type to ADHOC");
			}
			status = wlan_hdd_change_client_iface_to_new_mode(ndev,
					type);
			if (status) {
				hdd_err("Failed to change iface to new mode:%d status %d",
						type, status);
				return status;
			}
			if (hdd_start_adapter(adapter)) {
				hdd_err("Failed to start adapter :%d",
						adapter->device_mode);
				return -EINVAL;
			}
			goto done;
		case NL80211_IFTYPE_AP:
		case NL80211_IFTYPE_P2P_GO:
		{
			hdd_debug("Setting interface Type to %s",
			       (type ==
				NL80211_IFTYPE_AP) ? "SoftAP" :
			       "P2pGo");

			/* Cancel any remain on channel for GO mode */
			if (NL80211_IFTYPE_P2P_GO == type) {
				wlan_hdd_cancel_existing_remain_on_channel
					(adapter);
			}

			hdd_stop_adapter(hdd_ctx, adapter);
			/* De-init the adapter */
			hdd_deinit_adapter(hdd_ctx, adapter, true);
			memset(&adapter->session, 0,
			       sizeof(adapter->session));
			adapter->device_mode =
				(type ==
				 NL80211_IFTYPE_AP) ? QDF_SAP_MODE :
				QDF_P2P_GO_MODE;

			/*
			 * Fw will take care incase of concurrency
			 */

			if ((QDF_SAP_MODE == adapter->device_mode)
			    && (pConfig->apRandomBssidEnabled)) {
				/* To meet Android requirements create
				 * a randomized MAC address of the
				 * form 02:1A:11:Fx:xx:xx
				 */
				get_random_bytes(&ndev->dev_addr[3], 3);
				ndev->dev_addr[0] = 0x02;
				ndev->dev_addr[1] = 0x1A;
				ndev->dev_addr[2] = 0x11;
				ndev->dev_addr[3] |= 0xF0;
				memcpy(adapter->mac_addr.
				       bytes, ndev->dev_addr,
				       QDF_MAC_ADDR_SIZE);
				pr_info("wlan: Generated HotSpot BSSID "
					MAC_ADDRESS_STR "\n",
					MAC_ADDR_ARRAY(ndev->dev_addr));
			}

			hdd_set_ap_ops(adapter->dev);

			if (hdd_start_adapter(adapter)) {
				hdd_err("Error initializing the ap mode");
				return -EINVAL;
			}
			/* Interface type changed update in wiphy structure */
			if (wdev) {
				wdev->iftype = type;
			} else {
				hdd_err("Wireless dev is NULL");
				return -EINVAL;
			}
			goto done;
		}

		default:
			hdd_err("Unsupported interface type: %d", type);
			return -EOPNOTSUPP;
		}
	} else if ((adapter->device_mode == QDF_SAP_MODE) ||
		   (adapter->device_mode == QDF_P2P_GO_MODE)) {
		switch (type) {
		case NL80211_IFTYPE_STATION:
		case NL80211_IFTYPE_P2P_CLIENT:
		case NL80211_IFTYPE_ADHOC:
			status = wlan_hdd_change_client_iface_to_new_mode(ndev,
					type);
			if (status != QDF_STATUS_SUCCESS)
				return status;
			if (hdd_start_adapter(adapter)) {
				hdd_err("Failed to start adapter: %d",
						adapter->device_mode);
				return -EINVAL;
			}
			goto done;

		case NL80211_IFTYPE_AP:
		case NL80211_IFTYPE_P2P_GO:
			wdev->iftype = type;
			adapter->device_mode = (type == NL80211_IFTYPE_AP) ?
						QDF_SAP_MODE : QDF_P2P_GO_MODE;
			goto done;

		default:
			hdd_err("Unsupported interface type: %d", type);
			return -EOPNOTSUPP;
		}
	} else {
		hdd_err("Unsupported device mode: %d",
		       adapter->device_mode);
		return -EOPNOTSUPP;
	}
done:
	/* Set bitmask based on updated value */
	policy_mgr_set_concurrency_mode(hdd_ctx->hdd_psoc,
		adapter->device_mode);

	hdd_lpass_notify_mode_change(adapter);

	EXIT();
	return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0)
/**
 * wlan_hdd_cfg80211_change_iface() - change interface cfg80211 op
 * @wiphy: Pointer to the wiphy structure
 * @ndev: Pointer to the net device
 * @type: Interface type
 * @flags: Flags for change interface
 * @params: Pointer to change interface parameters
 *
 * Return: 0 for success, error number on failure.
 */
static int wlan_hdd_cfg80211_change_iface(struct wiphy *wiphy,
					  struct net_device *ndev,
					  enum nl80211_iftype type,
					  u32 *flags,
					  struct vif_params *params)
{
	int ret;

	cds_ssr_protect(__func__);
	ret =
		__wlan_hdd_cfg80211_change_iface(wiphy, ndev, type, flags, params);
	cds_ssr_unprotect(__func__);

	return ret;
}
#else
static int wlan_hdd_cfg80211_change_iface(struct wiphy *wiphy,
					  struct net_device *ndev,
					  enum nl80211_iftype type,
					  struct vif_params *params)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_change_iface(wiphy, ndev, type,
					       &params->flags, params);
	cds_ssr_unprotect(__func__);

	return ret;
}
#endif /* KERNEL_VERSION(4, 12, 0) */

/**
 * __wlan_hdd_change_station() - change station
 * @wiphy: Pointer to the wiphy structure
 * @dev: Pointer to the net device.
 * @mac: bssid
 * @params: Pointer to station parameters
 *
 * Return: 0 for success, error number on failure.
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
static int __wlan_hdd_change_station(struct wiphy *wiphy,
				   struct net_device *dev,
				   const uint8_t *mac,
				   struct station_parameters *params)
#else
static int __wlan_hdd_change_station(struct wiphy *wiphy,
				   struct net_device *dev,
				   uint8_t *mac,
				   struct station_parameters *params)
#endif
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx;
	struct hdd_station_ctx *sta_ctx;
	struct qdf_mac_addr STAMacAddress;
	int ret;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CHANGE_STATION,
			 adapter->session_id, params->listen_interval));

	if (wlan_hdd_validate_session_id(adapter->session_id)) {
		hdd_err("invalid session id: %d", adapter->session_id);
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	qdf_mem_copy(STAMacAddress.bytes, mac, QDF_MAC_ADDR_SIZE);

	if ((adapter->device_mode == QDF_SAP_MODE) ||
	    (adapter->device_mode == QDF_P2P_GO_MODE)) {
		if (params->sta_flags_set & BIT(NL80211_STA_FLAG_AUTHORIZED)) {
			status =
				hdd_softap_change_sta_state(adapter,
							    &STAMacAddress,
							    OL_TXRX_PEER_STATE_AUTH);

			if (status != QDF_STATUS_SUCCESS) {
				hdd_debug("Not able to change TL state to AUTHENTICATED");
				return -EINVAL;
			}
		}
	} else if ((adapter->device_mode == QDF_STA_MODE) ||
		   (adapter->device_mode == QDF_P2P_CLIENT_MODE)) {
		if (params->sta_flags_set & BIT(NL80211_STA_FLAG_TDLS_PEER)) {
#if defined(FEATURE_WLAN_TDLS)
			ret = wlan_cfg80211_tdls_update_peer(hdd_ctx->hdd_pdev,
							     dev, mac, params);
#endif
		}
	}
	EXIT();
	return ret;
}

/**
 * wlan_hdd_change_station() - cfg80211 change station handler function
 * @wiphy: Pointer to the wiphy structure
 * @dev: Pointer to the net device.
 * @mac: bssid
 * @params: Pointer to station parameters
 *
 * This is the cfg80211 change station handler function which invokes
 * the internal function @__wlan_hdd_change_station with
 * SSR protection.
 *
 * Return: 0 for success, error number on failure.
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0)) || defined(WITH_BACKPORTS)
static int wlan_hdd_change_station(struct wiphy *wiphy,
				   struct net_device *dev,
				   const u8 *mac,
				   struct station_parameters *params)
#else
static int wlan_hdd_change_station(struct wiphy *wiphy,
				   struct net_device *dev,
				   u8 *mac,
				   struct station_parameters *params)
#endif
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_change_station(wiphy, dev, mac, params);
	cds_ssr_unprotect(__func__);

	return ret;
}

/*
 * FUNCTION: __wlan_hdd_cfg80211_add_key
 * This function is used to initialize the key information
 */
static int __wlan_hdd_cfg80211_add_key(struct wiphy *wiphy,
				       struct net_device *ndev,
				       u8 key_index, bool pairwise,
				       const u8 *mac_addr,
				       struct key_params *params)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(ndev);
	tCsrRoamSetKey setKey;
	int status;
	uint32_t roamId = INVALID_ROAM_ID;
	QDF_STATUS qdf_ret_status;
	struct hdd_context *hdd_ctx;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(adapter->session_id)) {
		hdd_err("Invalid session id: %d", adapter->session_id);
		return -EINVAL;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_ADD_KEY,
			 adapter->session_id, params->key_len));
	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	status = wlan_hdd_validate_context(hdd_ctx);

	if (0 != status)
		return status;

	hdd_debug("Device_mode %s(%d)",
		hdd_device_mode_to_string(adapter->device_mode),
		adapter->device_mode);

	if (CSR_MAX_NUM_KEY <= key_index) {
		hdd_err("Invalid key index %d", key_index);

		return -EINVAL;
	}

	if (CSR_MAX_KEY_LEN < params->key_len) {
		hdd_err("Invalid key length %d", params->key_len);

		return -EINVAL;
	}

	if (CSR_MAX_RSC_LEN < params->seq_len) {
		hdd_err("Invalid seq length %d", params->seq_len);

		return -EINVAL;
	}

	hdd_debug("key index %d, key length %d, seq length %d",
		  key_index, params->key_len, params->seq_len);

	/*extract key idx, key len and key */
	qdf_mem_zero(&setKey, sizeof(tCsrRoamSetKey));
	setKey.keyId = key_index;
	setKey.keyLength = params->key_len;
	qdf_mem_copy(&setKey.Key[0], params->key, params->key_len);
	qdf_mem_copy(&setKey.keyRsc[0], params->seq, params->seq_len);

	switch (params->cipher) {
	case WLAN_CIPHER_SUITE_WEP40:
		setKey.encType = eCSR_ENCRYPT_TYPE_WEP40_STATICKEY;
		break;

	case WLAN_CIPHER_SUITE_WEP104:
		setKey.encType = eCSR_ENCRYPT_TYPE_WEP104_STATICKEY;
		break;

	case WLAN_CIPHER_SUITE_TKIP:
	{
		u8 *pKey = &setKey.Key[0];

		setKey.encType = eCSR_ENCRYPT_TYPE_TKIP;
		qdf_mem_zero(pKey, CSR_MAX_KEY_LEN);

		/* Supplicant sends the 32bytes key in this order
		 *
		 * |--------------|----------|----------|
		 * |   Tk1        |TX-MIC    |  RX Mic  |
		 * |--------------|----------|----------|
		 * <---16bytes---><--8bytes--><--8bytes-->
		 *
		 * Sme expects the 32 bytes key to be in the below order
		 *
		 * |--------------|----------|----------|
		 * |   Tk1        |RX-MIC    |  TX Mic  |
		 * |--------------|----------|----------|
		 * <---16bytes---><--8bytes--><--8bytes-->
		 */
		/* Copy the Temporal Key 1 (TK1) */
		qdf_mem_copy(pKey, params->key, 16);

		/*Copy the rx mic first */
		qdf_mem_copy(&pKey[16], &params->key[24], 8);

		/*Copy the tx mic */
		qdf_mem_copy(&pKey[24], &params->key[16], 8);

		break;
	}

	case WLAN_CIPHER_SUITE_CCMP:
		setKey.encType = eCSR_ENCRYPT_TYPE_AES;
		break;

#ifdef FEATURE_WLAN_WAPI
	case WLAN_CIPHER_SUITE_SMS4:
	{
		qdf_mem_zero(&setKey, sizeof(tCsrRoamSetKey));
		wlan_hdd_cfg80211_set_key_wapi(adapter, key_index,
					       mac_addr, params->key,
					       params->key_len);
		return 0;
	}
#endif

#ifdef FEATURE_WLAN_ESE
	case WLAN_CIPHER_SUITE_KRK:
		setKey.encType = eCSR_ENCRYPT_TYPE_KRK;
		break;
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	case WLAN_CIPHER_SUITE_BTK:
		setKey.encType = eCSR_ENCRYPT_TYPE_BTK;
		break;
#endif
#endif

#ifdef WLAN_FEATURE_11W
	case WLAN_CIPHER_SUITE_AES_CMAC:
		setKey.encType = eCSR_ENCRYPT_TYPE_AES_CMAC;
		break;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
	case WLAN_CIPHER_SUITE_BIP_GMAC_128:
		setKey.encType = eCSR_ENCRYPT_TYPE_AES_GMAC_128;
		break;
	case WLAN_CIPHER_SUITE_BIP_GMAC_256:
		setKey.encType = eCSR_ENCRYPT_TYPE_AES_GMAC_256;
		break;
#endif
#endif
	case WLAN_CIPHER_SUITE_GCMP:
		setKey.encType = eCSR_ENCRYPT_TYPE_AES_GCMP;
		break;
	case WLAN_CIPHER_SUITE_GCMP_256:
		setKey.encType = eCSR_ENCRYPT_TYPE_AES_GCMP_256;
		break;

	default:
		hdd_err("Unsupported cipher type: %u", params->cipher);
		return -EOPNOTSUPP;
	}

	hdd_debug("encryption type %d", setKey.encType);

	if (!pairwise) {
		/* set group key */
		hdd_debug("setting Broadcast key");
		setKey.keyDirection = eSIR_RX_ONLY;
		qdf_set_macaddr_broadcast(&setKey.peerMac);
	} else {
		/* set pairwise key */
		hdd_debug("setting pairwise key");
		setKey.keyDirection = eSIR_TX_RX;
		qdf_mem_copy(setKey.peerMac.bytes, mac_addr, QDF_MAC_ADDR_SIZE);
	}
	if ((QDF_IBSS_MODE == adapter->device_mode) && !pairwise) {
		/* if a key is already installed, block all subsequent ones */
		if (adapter->session.station.ibss_enc_key_installed) {
			hdd_debug("IBSS key installed already");
			return 0;
		}

		setKey.keyDirection = eSIR_TX_RX;
		/*Set the group key */
		status = sme_roam_set_key(WLAN_HDD_GET_HAL_CTX(adapter),
					  adapter->session_id, &setKey, &roamId);

		if (0 != status) {
			hdd_err("sme_roam_set_key failed, status: %d", status);
			return -EINVAL;
		}
		/* Save the keys here and call sme_roam_set_key for setting
		 * the PTK after peer joins the IBSS network
		 */
		qdf_mem_copy(&adapter->session.station.ibss_enc_key,
			     &setKey, sizeof(tCsrRoamSetKey));

		adapter->session.station.ibss_enc_key_installed = 1;
		return status;
	}
	if ((adapter->device_mode == QDF_SAP_MODE) ||
	    (adapter->device_mode == QDF_P2P_GO_MODE)) {
		struct hdd_hostapd_state *hostapd_state =
			WLAN_HDD_GET_HOSTAP_STATE_PTR(adapter);
		struct hdd_ap_ctx *ap_ctx =
			WLAN_HDD_GET_AP_CTX_PTR(adapter);

		if (hostapd_state->bss_state == BSS_START) {
			status = wlansap_set_key_sta(
				WLAN_HDD_GET_SAP_CTX_PTR(adapter), &setKey);
			if (status != QDF_STATUS_SUCCESS) {
				hdd_err("wlansap_set_key_sta failed status: %d",
					status);
			}
		}

		/* Save the key in ap ctx for use on START_BSS and restart */
		if (pairwise ||
			eCSR_ENCRYPT_TYPE_WEP40_STATICKEY == setKey.encType ||
			eCSR_ENCRYPT_TYPE_WEP104_STATICKEY == setKey.encType)
			qdf_mem_copy(&ap_ctx->wep_key[key_index], &setKey,
				     sizeof(tCsrRoamSetKey));
		else
			qdf_mem_copy(&ap_ctx->group_key, &setKey,
				     sizeof(tCsrRoamSetKey));

	} else if ((adapter->device_mode == QDF_STA_MODE) ||
		   (adapter->device_mode == QDF_P2P_CLIENT_MODE)) {
		struct hdd_wext_state *pWextState =
			WLAN_HDD_GET_WEXT_STATE_PTR(adapter);
		struct hdd_station_ctx *sta_ctx =
			WLAN_HDD_GET_STATION_CTX_PTR(adapter);

		if (!pairwise) {
			/* set group key */
			if (sta_ctx->roam_info.defer_key_complete) {
				hdd_debug("Perform Set key Complete");
				hdd_perform_roam_set_key_complete(adapter);
			}
		}

		pWextState->roamProfile.Keys.KeyLength[key_index] =
			(u8) params->key_len;

		pWextState->roamProfile.Keys.defaultIndex = key_index;

		qdf_mem_copy(&pWextState->roamProfile.Keys.
			     KeyMaterial[key_index][0], params->key,
			     params->key_len);

		hdd_debug("Set key for peerMac "MAC_ADDRESS_STR" direction %d",
		       MAC_ADDR_ARRAY(setKey.peerMac.bytes),
		       setKey.keyDirection);

		/* The supplicant may attempt to set the PTK once
		 * pre-authentication is done. Save the key in the
		 * UMAC and include it in the ADD BSS request
		 */
		qdf_ret_status = sme_ft_update_key(WLAN_HDD_GET_HAL_CTX(adapter),
						   adapter->session_id, &setKey);
		if (qdf_ret_status == QDF_STATUS_FT_PREAUTH_KEY_SUCCESS) {
			hdd_debug("Update PreAuth Key success");
			return 0;
		} else if (qdf_ret_status == QDF_STATUS_FT_PREAUTH_KEY_FAILED) {
			hdd_err("Update PreAuth Key failed");
			return -EINVAL;
		}

		/* issue set key request to SME */
		status = sme_roam_set_key(WLAN_HDD_GET_HAL_CTX(adapter),
					  adapter->session_id, &setKey, &roamId);

		if (0 != status) {
			hdd_err("sme_roam_set_key failed, status: %d", status);
			return -EINVAL;
		}

		/* in case of IBSS as there was no information
		 * available about WEP keys during IBSS join, group
		 * key intialized with NULL key, so re-initialize
		 * group key with correct value
		 */
		if ((eCSR_BSS_TYPE_START_IBSS ==
		     pWextState->roamProfile.BSSType)
		    &&
		    !((IW_AUTH_KEY_MGMT_802_1X ==
		       (pWextState->authKeyMgmt & IW_AUTH_KEY_MGMT_802_1X))
		      && (eCSR_AUTH_TYPE_OPEN_SYSTEM ==
			  sta_ctx->conn_info.authType)
		      )
		    && ((WLAN_CIPHER_SUITE_WEP40 == params->cipher)
			|| (WLAN_CIPHER_SUITE_WEP104 == params->cipher)
			)
		    ) {
			setKey.keyDirection = eSIR_RX_ONLY;
			qdf_set_macaddr_broadcast(&setKey.peerMac);

			hdd_debug("Set key peerMac "MAC_ADDRESS_STR" direction %d",
			       MAC_ADDR_ARRAY(setKey.peerMac.bytes),
			       setKey.keyDirection);

			status = sme_roam_set_key(WLAN_HDD_GET_HAL_CTX(adapter),
						  adapter->session_id, &setKey,
						  &roamId);

			if (0 != status) {
				hdd_err("sme_roam_set_key failed for group key (IBSS), returned %d", status);
				return -EINVAL;
			}
		}
	}
	EXIT();
	return 0;
}

static int wlan_hdd_cfg80211_add_key(struct wiphy *wiphy,
				     struct net_device *ndev,
				     u8 key_index, bool pairwise,
				     const u8 *mac_addr,
				     struct key_params *params)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_add_key(wiphy, ndev, key_index, pairwise,
					  mac_addr, params);
	cds_ssr_unprotect(__func__);

	return ret;
}

/*
 * FUNCTION: __wlan_hdd_cfg80211_get_key
 * This function is used to get the key information
 */
static int __wlan_hdd_cfg80211_get_key(struct wiphy *wiphy,
				       struct net_device *ndev,
				       u8 key_index, bool pairwise,
				       const u8 *mac_addr, void *cookie,
				       void (*callback)(void *cookie,
							struct key_params *)
				       )
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(ndev);
	tCsrRoamProfile *roam_profile;
	struct key_params params;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(adapter->session_id)) {
		hdd_err("Invalid session id: %d", adapter->session_id);
		return -EINVAL;
	}

	hdd_debug("Device_mode %s(%d)",
		hdd_device_mode_to_string(adapter->device_mode),
		adapter->device_mode);

	memset(&params, 0, sizeof(params));

	if (CSR_MAX_NUM_KEY <= key_index) {
		hdd_err("Invalid key index: %d", key_index);
		return -EINVAL;
	}

	if ((adapter->device_mode == QDF_SAP_MODE) ||
	    (adapter->device_mode == QDF_P2P_GO_MODE)) {
		struct hdd_ap_ctx *ap_ctx =
			WLAN_HDD_GET_AP_CTX_PTR(adapter);
		roam_profile =
			wlan_sap_get_roam_profile(ap_ctx->sap_context);
	} else {
		struct hdd_wext_state *pWextState =
			WLAN_HDD_GET_WEXT_STATE_PTR(adapter);
		roam_profile = &(pWextState->roamProfile);
	}

	if (roam_profile == NULL) {
		hdd_err("Get roam profile failed!");
		return -EINVAL;
	}

	switch (roam_profile->EncryptionType.encryptionType[0]) {
	case eCSR_ENCRYPT_TYPE_NONE:
		params.cipher = IW_AUTH_CIPHER_NONE;
		break;

	case eCSR_ENCRYPT_TYPE_WEP40_STATICKEY:
	case eCSR_ENCRYPT_TYPE_WEP40:
		params.cipher = WLAN_CIPHER_SUITE_WEP40;
		break;

	case eCSR_ENCRYPT_TYPE_WEP104_STATICKEY:
	case eCSR_ENCRYPT_TYPE_WEP104:
		params.cipher = WLAN_CIPHER_SUITE_WEP104;
		break;

	case eCSR_ENCRYPT_TYPE_TKIP:
		params.cipher = WLAN_CIPHER_SUITE_TKIP;
		break;

	case eCSR_ENCRYPT_TYPE_AES:
		params.cipher = WLAN_CIPHER_SUITE_AES_CMAC;
		break;
	case eCSR_ENCRYPT_TYPE_AES_GCMP:
		params.cipher = WLAN_CIPHER_SUITE_GCMP;
		break;
	case eCSR_ENCRYPT_TYPE_AES_GCMP_256:
		params.cipher = WLAN_CIPHER_SUITE_GCMP_256;
		break;
	default:
		params.cipher = IW_AUTH_CIPHER_NONE;
		break;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_GET_KEY,
			 adapter->session_id, params.cipher));

	params.key_len = roam_profile->Keys.KeyLength[key_index];
	params.seq_len = 0;
	params.seq = NULL;
	params.key = &roam_profile->Keys.KeyMaterial[key_index][0];
	callback(cookie, &params);

	EXIT();
	return 0;
}

static int wlan_hdd_cfg80211_get_key(struct wiphy *wiphy,
				     struct net_device *ndev,
				     u8 key_index, bool pairwise,
				     const u8 *mac_addr, void *cookie,
				     void (*callback)(void *cookie,
						      struct key_params *)
				     )
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_get_key(wiphy, ndev, key_index, pairwise,
					  mac_addr, cookie, callback);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __wlan_hdd_cfg80211_del_key() - Delete the encryption key for station
 * @wiphy: wiphy interface context
 * @ndev: pointer to net device
 * @key_index: Key index used in 802.11 frames
 * @unicast: true if it is unicast key
 * @multicast: true if it is multicast key
 *
 * This function is required for cfg80211_ops API.
 * It is used to delete the key information
 * Underlying hardware implementation does not have API to delete the
 * encryption key. It is automatically deleted when the peer is
 * removed. Hence this function currently does nothing.
 * Future implementation may interprete delete key operation to
 * replacing the key with a random junk value, effectively making it
 * useless.
 *
 * Return: status code, always 0.
 */

static int __wlan_hdd_cfg80211_del_key(struct wiphy *wiphy,
				     struct net_device *ndev,
				     u8 key_index,
				     bool pairwise, const u8 *mac_addr)
{
	EXIT();
	return 0;
}

/**
 * wlan_hdd_cfg80211_del_key() - cfg80211 delete key handler function
 * @wiphy: Pointer to wiphy structure.
 * @dev: Pointer to net_device structure.
 * @key_index: key index
 * @pairwise: pairwise
 * @mac_addr: mac address
 *
 * This is the cfg80211 delete key handler function which invokes
 * the internal function @__wlan_hdd_cfg80211_del_key with
 * SSR protection.
 *
 * Return: 0 for success, error number on failure.
 */
static int wlan_hdd_cfg80211_del_key(struct wiphy *wiphy,
					struct net_device *dev,
					u8 key_index,
					bool pairwise, const u8 *mac_addr)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_del_key(wiphy, dev, key_index,
					  pairwise, mac_addr);
	cds_ssr_unprotect(__func__);

	return ret;
}

#ifdef FEATURE_WLAN_WAPI
static bool hdd_is_wapi_enc_type(eCsrEncryptionType ucEncryptionType)
{
	if (ucEncryptionType == eCSR_ENCRYPT_TYPE_WPI)
		return true;

	return false;
}
#else
static bool hdd_is_wapi_enc_type(eCsrEncryptionType ucEncryptionType)
{
	return false;
}
#endif

/*
 * FUNCTION: __wlan_hdd_cfg80211_set_default_key
 * This function is used to set the default tx key index
 */
static int __wlan_hdd_cfg80211_set_default_key(struct wiphy *wiphy,
					       struct net_device *ndev,
					       u8 key_index,
					       bool unicast, bool multicast)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(ndev);
	struct hdd_context *hdd_ctx;
	int status;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(adapter->session_id)) {
		hdd_err("Invalid session id: %d", adapter->session_id);
		return -EINVAL;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_SET_DEFAULT_KEY,
			 adapter->session_id, key_index));

	hdd_debug("Device_mode %s(%d) key_index = %d",
		hdd_device_mode_to_string(adapter->device_mode),
		adapter->device_mode, key_index);

	if (CSR_MAX_NUM_KEY <= key_index) {
		hdd_err("Invalid key index: %d", key_index);
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	status = wlan_hdd_validate_context(hdd_ctx);

	if (0 != status)
		return status;

	if ((adapter->device_mode == QDF_STA_MODE) ||
	    (adapter->device_mode == QDF_P2P_CLIENT_MODE)) {
		struct hdd_wext_state *pWextState =
			WLAN_HDD_GET_WEXT_STATE_PTR(adapter);
		struct hdd_station_ctx *sta_ctx =
			WLAN_HDD_GET_STATION_CTX_PTR(adapter);

		if ((eCSR_ENCRYPT_TYPE_TKIP !=
		     sta_ctx->conn_info.ucEncryptionType) &&
		    !hdd_is_wapi_enc_type(
		     sta_ctx->conn_info.ucEncryptionType) &&
		    (eCSR_ENCRYPT_TYPE_AES !=
		     sta_ctx->conn_info.ucEncryptionType) &&
		    (eCSR_ENCRYPT_TYPE_AES_GCMP !=
			sta_ctx->conn_info.ucEncryptionType) &&
		    (eCSR_ENCRYPT_TYPE_AES_GCMP_256 !=
		     sta_ctx->conn_info.ucEncryptionType)) {
			/* If default key index is not same as previous one,
			 * then update the default key index
			 */

			tCsrRoamSetKey setKey;
			uint32_t roamId = INVALID_ROAM_ID;
			tCsrKeys *Keys = &pWextState->roamProfile.Keys;

			hdd_debug("Default tx key index %d", key_index);

			Keys->defaultIndex = (u8) key_index;
			qdf_mem_zero(&setKey, sizeof(tCsrRoamSetKey));
			setKey.keyId = key_index;
			setKey.keyLength = Keys->KeyLength[key_index];

			qdf_mem_copy(&setKey.Key[0],
				     &Keys->KeyMaterial[key_index][0],
				     Keys->KeyLength[key_index]);

			setKey.keyDirection = eSIR_TX_RX;

			qdf_copy_macaddr(&setKey.peerMac,
					 &sta_ctx->conn_info.bssId);

			if (Keys->KeyLength[key_index] == CSR_WEP40_KEY_LEN &&
			    pWextState->roamProfile.EncryptionType.
			    encryptionType[0] == eCSR_ENCRYPT_TYPE_WEP104) {
				/* In the case of dynamic wep
				 * supplicant hardcodes DWEP type to
				 * eCSR_ENCRYPT_TYPE_WEP104 even
				 * though ap is configured for WEP-40
				 * encryption. In this canse the key
				 * length is 5 but the encryption type
				 * is 104 hence checking the key
				 * lenght(5) and encryption type(104)
				 * and switching encryption type to 40
				 */
				pWextState->roamProfile.EncryptionType.
				encryptionType[0] = eCSR_ENCRYPT_TYPE_WEP40;
				pWextState->roamProfile.mcEncryptionType.
				encryptionType[0] = eCSR_ENCRYPT_TYPE_WEP40;
			}

			setKey.encType =
				pWextState->roamProfile.EncryptionType.
				encryptionType[0];

			/* Issue set key request */
			status = sme_roam_set_key(WLAN_HDD_GET_HAL_CTX(adapter),
						  adapter->session_id, &setKey,
						  &roamId);

			if (0 != status) {
				hdd_err("sme_roam_set_key failed, status: %d",
				       status);
				return -EINVAL;
			}
		}
	} else if (QDF_SAP_MODE == adapter->device_mode) {
		struct hdd_ap_ctx *ap_ctx =
			WLAN_HDD_GET_AP_CTX_PTR(adapter);
		tCsrRoamProfile *profile =
			wlan_sap_get_roam_profile(ap_ctx->sap_context);

		if (!profile) {
			hdd_err("Failed to get SAP Roam Profile");
			return -EINVAL;
		}
		/* In SoftAp mode setting key direction for default mode */
		if ((eCSR_ENCRYPT_TYPE_TKIP !=
		    profile->EncryptionType.encryptionType[0]) &&
		    (eCSR_ENCRYPT_TYPE_AES !=
		    profile->EncryptionType.encryptionType[0]) &&
		    (eCSR_ENCRYPT_TYPE_AES_GCMP !=
		    profile->EncryptionType.encryptionType[0]) &&
		    (eCSR_ENCRYPT_TYPE_AES_GCMP_256 !=
		    profile->EncryptionType.encryptionType[0])) {
			/* Saving key direction for default key index to TX default */
			ap_ctx->wep_key[key_index].keyDirection =
				eSIR_TX_DEFAULT;
			hdd_debug("WEP default key index set to SAP context %d",
				key_index);
			ap_ctx->wep_def_key_idx = key_index;
		}
	}

	EXIT();
	return status;
}

static int wlan_hdd_cfg80211_set_default_key(struct wiphy *wiphy,
					     struct net_device *ndev,
					     u8 key_index,
					     bool unicast, bool multicast)
{
	int ret;

	cds_ssr_protect(__func__);
	ret =
		__wlan_hdd_cfg80211_set_default_key(wiphy, ndev, key_index, unicast,
						    multicast);
	cds_ssr_unprotect(__func__);

	return ret;
}

/*
 * wlan_hdd_cfg80211_update_bss_list :to inform nl80211
 * interface that BSS might have been lost.
 * @adapter: adaptor
 * @bssid: bssid which might have been lost
 *
 * Return: bss which is unlinked from kernel cache
 */
struct cfg80211_bss *wlan_hdd_cfg80211_update_bss_list(
	struct hdd_adapter *adapter, tSirMacAddr bssid)
{
	struct net_device *dev = adapter->dev;
	struct wireless_dev *wdev = dev->ieee80211_ptr;
	struct wiphy *wiphy = wdev->wiphy;
	struct cfg80211_bss *bss = NULL;

	bss = hdd_cfg80211_get_bss(wiphy, NULL, bssid,
			NULL, 0);
	if (bss == NULL) {
		hdd_err("BSS not present");
	} else {
		hdd_debug("cfg80211_unlink_bss called for BSSID "
			MAC_ADDRESS_STR, MAC_ADDR_ARRAY(bssid));
		cfg80211_unlink_bss(wiphy, bss);
	}
	return bss;
}

#ifdef WLAN_ENABLE_AGEIE_ON_SCAN_RESULTS
static inline int
wlan_hdd_get_frame_len(struct bss_description *bss_desc)
{
	return GET_IE_LEN_IN_BSS(bss_desc->length) + sizeof(qcom_ie_age);
}

static inline void wlan_hdd_add_age_ie(struct ieee80211_mgmt *mgmt,
	uint32_t *ie_length, struct bss_description *bss_desc)
{
	qcom_ie_age *qie_age = NULL;

	/*
	 * GPS Requirement: need age ie per entry. Using vendor specific.
	 * Assuming this is the last IE, copy at the end
	 */
	*ie_length -= sizeof(qcom_ie_age);
	qie_age = (qcom_ie_age *)(mgmt->u.probe_resp.variable + *ie_length);
	qie_age->element_id = QCOM_VENDOR_IE_ID;
	qie_age->len = QCOM_VENDOR_IE_AGE_LEN;
	qie_age->oui_1 = QCOM_OUI1;
	qie_age->oui_2 = QCOM_OUI2;
	qie_age->oui_3 = QCOM_OUI3;
	qie_age->type = QCOM_VENDOR_IE_AGE_TYPE;
	/*
	 * Lowi expects the timestamp of bss in units of 1/10 ms. In driver
	 * all bss related timestamp is in units of ms. Due to this when scan
	 * results are sent to lowi the scan age is high.To address this,
	 * send age in units of 1/10 ms.
	 */
	qie_age->age = (uint32_t)(qdf_mc_timer_get_system_time() -
				  bss_desc->received_time)/10;
	qie_age->tsf_delta = bss_desc->tsf_delta;
	memcpy(&qie_age->beacon_tsf, bss_desc->timeStamp,
	       sizeof(qie_age->beacon_tsf));
	memcpy(&qie_age->seq_ctrl, &bss_desc->seq_ctrl,
	       sizeof(qie_age->seq_ctrl));
}
#else
static inline int
wlan_hdd_get_frame_len(struct bss_description *bss_desc)
{
	return GET_IE_LEN_IN_BSS(bss_desc->length);
}
static inline void wlan_hdd_add_age_ie(struct ieee80211_mgmt *mgmt,
	uint32_t *ie_length, struct bss_description *bss_desc)
{
}
#endif /* WLAN_ENABLE_AGEIE_ON_SCAN_RESULTS */


struct cfg80211_bss *
wlan_hdd_inform_bss_frame(struct hdd_adapter *adapter,
				     struct bss_description *bss_desc)
{
	struct wireless_dev *wdev = adapter->dev->ieee80211_ptr;
	struct wiphy *wiphy = wdev->wiphy;
	uint32_t ie_length = wlan_hdd_get_frame_len(bss_desc);
	const char *ie =
		((ie_length != 0) ? (const char *)&bss_desc->ieFields : NULL);
	uint32_t freq, i;
	struct cfg80211_bss *bss_status = NULL;
	struct hdd_context *hdd_ctx;
	struct timespec ts;
	struct hdd_config *cfg_param;
	struct wlan_cfg80211_inform_bss bss_data = {0};

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	/*
	 * wlan_hdd_validate_context should not be used here, In validate ctx
	 * start_modules_in_progress or stop_modules_in_progress is validated,
	 * If the start_modules_in_progress is set to true means the interface
	 * is not UP yet if the stop_modules_in_progress means that interface
	 * is already down. So in both the two scenario's driver should not be
	 * informing bss to kernel. Hence removing the validate context.
	 */

	if (!hdd_ctx || !hdd_ctx->config) {
		hdd_debug("HDD context is Null");
		return NULL;
	}

	if (cds_is_driver_recovering() ||
	    cds_is_load_or_unload_in_progress()) {
		hdd_debug("Recovery or load/unload in progress. State: 0x%x",
			  cds_get_driver_state());
		return NULL;
	}

	cfg_param = hdd_ctx->config;
	bss_data.frame_len = ie_length + offsetof(struct ieee80211_mgmt,
						  u.probe_resp.variable);
	bss_data.mgmt = qdf_mem_malloc(bss_data.frame_len);
	if (!bss_data.mgmt) {
		hdd_err("memory allocation failed");
		return NULL;
	}

	memcpy(bss_data.mgmt->bssid, bss_desc->bssId, ETH_ALEN);

	/* Android does not want the timestamp from the frame.
	 * Instead it wants a monotonic increasing value
	 */
	get_monotonic_boottime(&ts);
	bss_data.mgmt->u.probe_resp.timestamp =
		((u64) ts.tv_sec * 1000000) + (ts.tv_nsec / 1000);

	bss_data.mgmt->u.probe_resp.beacon_int = bss_desc->beaconInterval;
	bss_data.mgmt->u.probe_resp.capab_info = bss_desc->capabilityInfo;

	wlan_hdd_add_age_ie(bss_data.mgmt, &ie_length, bss_desc);

	memcpy(bss_data.mgmt->u.probe_resp.variable, ie, ie_length);
	if (bss_desc->fProbeRsp) {
		bss_data.mgmt->frame_control |=
			(u16) (IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_PROBE_RESP);
	} else {
		bss_data.mgmt->frame_control |=
			(u16) (IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_BEACON);
	}

	if (bss_desc->channelId <= ARRAY_SIZE(hdd_channels_2_4_ghz) &&
	    (wiphy->bands[HDD_NL80211_BAND_2GHZ] != NULL)) {
		freq =
			ieee80211_channel_to_frequency(bss_desc->channelId,
						       HDD_NL80211_BAND_2GHZ);
	} else if ((bss_desc->channelId > ARRAY_SIZE(hdd_channels_2_4_ghz))
		   && (wiphy->bands[HDD_NL80211_BAND_5GHZ] != NULL)) {
		freq =
			ieee80211_channel_to_frequency(bss_desc->channelId,
						       HDD_NL80211_BAND_5GHZ);
	} else {
		hdd_err("Invalid channel: %d", bss_desc->channelId);
		qdf_mem_free(bss_data.mgmt);
		return NULL;
	}

	bss_data.chan = ieee80211_get_channel(wiphy, freq);
	if (!bss_data.chan) {
		hdd_err("chan pointer is NULL, chan_no: %d freq: %d",
			bss_desc->channelId, freq);
		qdf_mem_free(bss_data.mgmt);
		return NULL;
	}

	/*
	 * Based on .ini configuration, raw rssi can be reported for bss.
	 * Raw rssi is typically used for estimating power.
	 */
	bss_data.rssi = (cfg_param->inform_bss_rssi_raw) ? bss_desc->rssi_raw :
			bss_desc->rssi;

	/* Supplicant takes the signal strength in terms of mBm(100*dBm) */
	bss_data.rssi = QDF_MIN(bss_data.rssi, 0) * 100;

	bss_data.boottime_ns = bss_desc->scansystimensec;

	/* Set all per chain rssi as invalid */
	for (i = 0; i < WLAN_MGMT_TXRX_HOST_MAX_ANTENNA; i++)
		bss_data.per_chain_snr[i] = WLAN_INVALID_PER_CHAIN_RSSI;

	hdd_debug("BSSID: " MAC_ADDRESS_STR " Channel:%d RSSI:%d TSF %u",
	       MAC_ADDR_ARRAY(bss_data.mgmt->bssid), bss_data.chan->center_freq,
	       (int)(bss_data.rssi / 100),
	       bss_desc->timeStamp[0]);

	bss_status = wlan_cfg80211_inform_bss_frame_data(wiphy, &bss_data);
	hdd_ctx->beacon_probe_rsp_cnt_per_scan++;
	qdf_mem_free(bss_data.mgmt);
	return bss_status;
}

/**
 * wlan_hdd_cfg80211_update_bss_db() - update bss database of CF80211
 * @adapter: Pointer to adapter
 * @roam_info: Pointer to roam info
 *
 * This function is used to update the BSS data base of CFG8011
 *
 * Return: struct cfg80211_bss pointer
 */
struct cfg80211_bss *
wlan_hdd_cfg80211_update_bss_db(struct hdd_adapter *adapter,
				struct csr_roam_info *roam_info)
{
	tCsrRoamConnectedProfile roamProfile;
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(adapter);
	struct cfg80211_bss *bss = NULL;

	memset(&roamProfile, 0, sizeof(tCsrRoamConnectedProfile));
	sme_roam_get_connect_profile(hHal, adapter->session_id, &roamProfile);

	if (NULL != roamProfile.pBssDesc) {
		bss = wlan_hdd_inform_bss_frame(adapter, roamProfile.pBssDesc);

		if (NULL == bss)
			hdd_debug("wlan_hdd_inform_bss_frame returned NULL");

		sme_roam_free_connect_profile(&roamProfile);
	} else {
		hdd_err("roamProfile.pBssDesc is NULL");
	}
	return bss;
}

/**
 * wlan_hdd_cfg80211_pmksa_candidate_notify() - notify a new PMSKA candidate
 * @adapter: Pointer to adapter
 * @roam_info: Pointer to roam info
 * @index: Index
 * @preauth: Preauth flag
 *
 * This function is used to notify the supplicant of a new PMKSA candidate.
 * PMK value is notified to supplicant whether PMK caching or OKC is enabled
 * in firmware or not. Supplicant needs this value becaue it uses PMK caching
 * by default.
 *
 * Return: 0 for success, non-zero for failure
 */
int wlan_hdd_cfg80211_pmksa_candidate_notify(struct hdd_adapter *adapter,
					     struct csr_roam_info *roam_info,
					     int index, bool preauth)
{
	struct net_device *dev = adapter->dev;

	ENTER();
	hdd_debug("is going to notify supplicant of:");

	if (NULL == roam_info) {
		hdd_err("roam_info is NULL");
		return -EINVAL;
	}

	hdd_notice(MAC_ADDRESS_STR,
	       MAC_ADDR_ARRAY(roam_info->bssid.bytes));
	cfg80211_pmksa_candidate_notify(dev, index,
					roam_info->bssid.bytes,
					preauth, GFP_KERNEL);
	return 0;
}

#ifdef FEATURE_WLAN_LFR_METRICS
/**
 * wlan_hdd_cfg80211_roam_metrics_preauth() - roam metrics preauth
 * @adapter: Pointer to adapter
 * @roam_info: Pointer to roam info
 *
 * 802.11r/LFR metrics reporting function to report preauth initiation
 *
 * Return: QDF status
 */
#define MAX_LFR_METRICS_EVENT_LENGTH 100
QDF_STATUS
wlan_hdd_cfg80211_roam_metrics_preauth(struct hdd_adapter *adapter,
				       struct csr_roam_info *roam_info)
{
	unsigned char metrics_notification[MAX_LFR_METRICS_EVENT_LENGTH + 1];
	union iwreq_data wrqu;

	ENTER();

	if (NULL == adapter) {
		hdd_err("adapter is NULL!");
		return QDF_STATUS_E_FAILURE;
	}

	/* create the event */
	memset(&wrqu, 0, sizeof(wrqu));
	memset(metrics_notification, 0, sizeof(metrics_notification));

	wrqu.data.pointer = metrics_notification;
	wrqu.data.length = scnprintf(metrics_notification,
				     sizeof(metrics_notification),
				     "QCOM: LFR_PREAUTH_INIT " MAC_ADDRESS_STR,
				     MAC_ADDR_ARRAY(roam_info->bssid.bytes));

	wireless_send_event(adapter->dev, IWEVCUSTOM, &wrqu,
			    metrics_notification);

	EXIT();

	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_hdd_cfg80211_roam_metrics_handover() - roam metrics hand over
 * @adapter: Pointer to adapter
 * @roam_info: Pointer to roam info
 * @preauth_status: Preauth status
 *
 * 802.11r/LFR metrics reporting function to report handover initiation
 *
 * Return: QDF status
 */
QDF_STATUS
wlan_hdd_cfg80211_roam_metrics_preauth_status(struct hdd_adapter *adapter,
					      struct csr_roam_info *roam_info,
					      bool preauth_status)
{
	unsigned char metrics_notification[MAX_LFR_METRICS_EVENT_LENGTH + 1];
	union iwreq_data wrqu;

	ENTER();

	if (NULL == adapter) {
		hdd_err("adapter is NULL!");
		return QDF_STATUS_E_FAILURE;
	}

	/* create the event */
	memset(&wrqu, 0, sizeof(wrqu));
	memset(metrics_notification, 0, sizeof(metrics_notification));

	scnprintf(metrics_notification, sizeof(metrics_notification),
		  "QCOM: LFR_PREAUTH_STATUS " MAC_ADDRESS_STR,
		  MAC_ADDR_ARRAY(roam_info->bssid.bytes));

	if (1 == preauth_status)
		strlcat(metrics_notification, " true",
				sizeof(metrics_notification));
	else
		strlcat(metrics_notification, " false",
				sizeof(metrics_notification));

	wrqu.data.pointer = metrics_notification;
	wrqu.data.length = strlen(metrics_notification);

	wireless_send_event(adapter->dev, IWEVCUSTOM, &wrqu,
			    metrics_notification);

	EXIT();

	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_hdd_cfg80211_roam_metrics_handover() - roam metrics hand over
 * @adapter: Pointer to adapter
 * @roam_info: Pointer to roam info
 *
 * 802.11r/LFR metrics reporting function to report handover initiation
 *
 * Return: QDF status
 */
QDF_STATUS
wlan_hdd_cfg80211_roam_metrics_handover(struct hdd_adapter *adapter,
					struct csr_roam_info *roam_info)
{
	unsigned char metrics_notification[MAX_LFR_METRICS_EVENT_LENGTH + 1];
	union iwreq_data wrqu;

	ENTER();

	if (NULL == adapter) {
		hdd_err("adapter is NULL!");
		return QDF_STATUS_E_FAILURE;
	}

	/* create the event */
	memset(&wrqu, 0, sizeof(wrqu));
	memset(metrics_notification, 0, sizeof(metrics_notification));

	wrqu.data.pointer = metrics_notification;
	wrqu.data.length = scnprintf(metrics_notification,
				     sizeof(metrics_notification),
				     "QCOM: LFR_PREAUTH_HANDOVER "
				     MAC_ADDRESS_STR,
				     MAC_ADDR_ARRAY(roam_info->bssid.bytes));

	wireless_send_event(adapter->dev, IWEVCUSTOM, &wrqu,
			    metrics_notification);

	EXIT();

	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * hdd_select_cbmode() - select channel bonding mode
 * @adapter: Pointer to adapter
 * @operatingChannel: Operating channel
 * @ch_params: channel info struct to populate
 *
 * Return: none
 */
void hdd_select_cbmode(struct hdd_adapter *adapter, uint8_t operationChannel,
			struct ch_params *ch_params)
{
	struct hdd_station_ctx *station_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	struct hdd_mon_set_ch_info *ch_info = &station_ctx->ch_info;
	uint8_t sec_ch = 0;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	/*
	 * CDS api expects secondary channel for calculating
	 * the channel params
	 */
	if ((ch_params->ch_width == CH_WIDTH_40MHZ) &&
	    (WLAN_REG_IS_24GHZ_CH(operationChannel))) {
		if (operationChannel >= 1 && operationChannel <= 5)
			sec_ch = operationChannel + 4;
		else if (operationChannel >= 6 && operationChannel <= 13)
			sec_ch = operationChannel - 4;
	}

	/* This call decides required channel bonding mode */
	wlan_reg_set_channel_params(hdd_ctx->hdd_pdev, operationChannel,
			sec_ch, ch_params);

	if (QDF_GLOBAL_MONITOR_MODE == cds_get_conparam()) {
		enum hdd_dot11_mode hdd_dot11_mode;
		uint8_t iniDot11Mode =
			(WLAN_HDD_GET_CTX(adapter))->config->dot11Mode;

		hdd_debug("Dot11Mode is %u", iniDot11Mode);
		switch (iniDot11Mode) {
		case eHDD_DOT11_MODE_AUTO:
		case eHDD_DOT11_MODE_11ax:
		case eHDD_DOT11_MODE_11ax_ONLY:
			if (sme_is_feature_supported_by_fw(DOT11AX))
				hdd_dot11_mode = eHDD_DOT11_MODE_11ax;
			else if (sme_is_feature_supported_by_fw(DOT11AC))
				hdd_dot11_mode = eHDD_DOT11_MODE_11ac;
			else
				hdd_dot11_mode = eHDD_DOT11_MODE_11n;
			break;
		case eHDD_DOT11_MODE_11ac:
		case eHDD_DOT11_MODE_11ac_ONLY:
			if (sme_is_feature_supported_by_fw(DOT11AC))
				hdd_dot11_mode = eHDD_DOT11_MODE_11ac;
			else
				hdd_dot11_mode = eHDD_DOT11_MODE_11n;
			break;
		case eHDD_DOT11_MODE_11n:
		case eHDD_DOT11_MODE_11n_ONLY:
			hdd_dot11_mode = eHDD_DOT11_MODE_11n;
			break;
		default:
			hdd_dot11_mode = iniDot11Mode;
			break;
		}
		ch_info->channel_width = ch_params->ch_width;
		ch_info->phy_mode =
			hdd_cfg_xlate_to_csr_phy_mode(hdd_dot11_mode);
		ch_info->channel = operationChannel;
		ch_info->cb_mode = ch_params->ch_width;
		hdd_debug("ch_info width %d, phymode %d channel %d",
			 ch_info->channel_width, ch_info->phy_mode,
			 ch_info->channel);
	}
}

/**
 * wlan_hdd_handle_sap_sta_dfs_conc() - to handle SAP STA DFS conc
 * @adapter: STA adapter
 * @roam_profile: STA roam profile
 *
 * This routine will move SAP from dfs to non-dfs, if sta is coming up.
 *
 * Return: false if sta-sap conc is not allowed, else return true
 */
static bool wlan_hdd_handle_sap_sta_dfs_conc(struct hdd_adapter *adapter,
						tCsrRoamProfile *roam_profile)
{
	struct hdd_context *hdd_ctx;
	struct hdd_adapter *ap_adapter;
	struct hdd_ap_ctx *hdd_ap_ctx;
	struct hdd_hostapd_state *hostapd_state;
	uint8_t channel = 0;
	QDF_STATUS status;

	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	if (!hdd_ctx) {
		hdd_err("HDD context is NULL");
		return true;
	}

	ap_adapter = hdd_get_adapter(hdd_ctx, QDF_SAP_MODE);
	/* probably no sap running, no handling required */
	if (ap_adapter == NULL)
		return true;

	/*
	 * sap is not in started state, so it is fine to go ahead with sta.
	 * if sap is currently doing CAC then don't allow sta to go further.
	 */
	if (!test_bit(SOFTAP_BSS_STARTED, &(ap_adapter)->event_flags) &&
	    (hdd_ctx->dev_dfs_cac_status != DFS_CAC_IN_PROGRESS))
		return true;

	if (hdd_ctx->dev_dfs_cac_status == DFS_CAC_IN_PROGRESS) {
		hdd_err("Concurrent SAP is in CAC state, STA is not allowed");
		return false;
	}

	/*
	 * log and return error, if we allow STA to go through, we don't
	 * know what is going to happen better stop sta connection
	 */
	hdd_ap_ctx = WLAN_HDD_GET_AP_CTX_PTR(ap_adapter);
	if (NULL == hdd_ap_ctx) {
		hdd_err("AP context not found");
		return false;
	}

	/* sap is on non-dfs channel, nothing to handle */
	if (!wlan_reg_is_dfs_ch(hdd_ctx->hdd_pdev,
				hdd_ap_ctx->operating_channel)) {
		hdd_info("sap is on non-dfs channel, sta is allowed");
		return true;
	}
	/*
	 * find out by looking in to scan cache where sta is going to
	 * connect by passing its roam_profile.
	 */
	status = policy_mgr_get_channel_from_scan_result(hdd_ctx->hdd_psoc,
			roam_profile, &channel);

	/*
	 * If the STA's channel is 2.4 GHz, then set pcl with only 2.4 GHz
	 * channels for roaming case.
	 */
	if (WLAN_REG_IS_24GHZ_CH(channel)) {
		hdd_info("sap is on dfs, new sta conn on 2.4 is allowed");
		return true;
	}

	/*
	 * If channel is 0 or DFS or LTE unsafe then better to call pcl and
	 * find out the best channel. If channel is non-dfs 5 GHz then
	 * better move SAP to STA's channel to make scc, so we have room
	 * for 3port MCC scenario.
	 */
	if ((0 == channel) || wlan_reg_is_dfs_ch(hdd_ctx->hdd_pdev, channel) ||
		!policy_mgr_is_safe_channel(hdd_ctx->hdd_psoc, channel))
		channel = policy_mgr_get_nondfs_preferred_channel(
			hdd_ctx->hdd_psoc, PM_SAP_MODE, true);

	hostapd_state = WLAN_HDD_GET_HOSTAP_STATE_PTR(ap_adapter);
	qdf_event_reset(&hostapd_state->qdf_event);
	status = wlansap_set_channel_change_with_csa(
			WLAN_HDD_GET_SAP_CTX_PTR(ap_adapter), channel,
			hdd_ap_ctx->sap_config.ch_width_orig, false);

	if (QDF_STATUS_SUCCESS != status) {
		hdd_err("Set channel with CSA IE failed, can't allow STA");
		return false;
	}

	/*
	 * wait here for SAP to finish the channel switch. When channel
	 * switch happens, SAP sends few beacons with CSA_IE. After
	 * successfully Transmission of those beacons, it will move its
	 * state from started to disconnected and move to new channel.
	 * once it moves to new channel, sap again moves its state
	 * machine from disconnected to started and set this event.
	 * wait for 10 secs to finish this.
	 */
	status = qdf_wait_for_event_completion(&hostapd_state->qdf_event, 10000);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("wait for qdf_event failed, STA not allowed!!");
		return false;
	}

	return true;
}

#ifdef WLAN_FEATURE_11W
/**
 * wlan_hdd_cfg80211_check_pmf_valid() - check if pmf status is ok
 * @roam_profile: pointer to roam profile
 *
 * if MFPEnabled is set but the peer AP is non-PMF i.e 80211w=2
 * or pmf=2 is an explicit configuration in the supplicant
 * configuration, drop the connection request.
 *
 * Return: 0 if check result is valid, otherwise return error code
 */
static int wlan_hdd_cfg80211_check_pmf_valid(tCsrRoamProfile *roam_profile)
{
	if (roam_profile->MFPEnabled &&
	    !(roam_profile->MFPRequired ||
	    roam_profile->MFPCapable)) {
		hdd_err("Drop connect req as supplicant has indicated PMF required for the non-PMF peer. MFPEnabled %d MFPRequired %d MFPCapable %d",
				roam_profile->MFPEnabled,
				roam_profile->MFPRequired,
				roam_profile->MFPCapable);
		return -EINVAL;
	}
	return 0;
}
#else
static inline
int wlan_hdd_cfg80211_check_pmf_valid(tCsrRoamProfile *roam_profile)
{
	return 0;
}
#endif

/**
 * wlan_hdd_cfg80211_connect_start() - to start the association process
 * @adapter: Pointer to adapter
 * @ssid: Pointer to ssid
 * @ssid_len: Length of ssid
 * @bssid: Pointer to bssid
 * @bssid_hint: Pointer to bssid hint
 * @operatingChannel: Operating channel
 * @ch_width: channel width. this is needed only for IBSS
 *
 * This function is used to start the association process
 *
 * Return: 0 for success, non-zero for failure
 */
static int wlan_hdd_cfg80211_connect_start(struct hdd_adapter *adapter,
				    const u8 *ssid, size_t ssid_len,
				    const u8 *bssid, const u8 *bssid_hint,
				    u8 operatingChannel,
				    enum nl80211_chan_width ch_width)
{
	int status = 0;
	QDF_STATUS qdf_status;
	struct hdd_wext_state *pWextState;
	struct hdd_context *hdd_ctx;
	struct hdd_station_ctx *hdd_sta_ctx;
	uint32_t roamId = INVALID_ROAM_ID;
	tCsrRoamProfile *roam_profile;
	eCsrAuthType RSNAuthType;
	tSmeConfigParams *sme_config;
	uint8_t channel = 0;

	ENTER();

	pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(adapter);
	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	status = wlan_hdd_validate_context(hdd_ctx);
	if (status)
		goto ret_status;

	if (SIR_MAC_MAX_SSID_LENGTH < ssid_len) {
		hdd_err("wrong SSID len");
		status = -EINVAL;
		goto ret_status;
	}

	if (true == hdd_is_connection_in_progress(NULL, NULL)) {
		hdd_err("Connection refused: conn in progress");
		status = -EINVAL;
		goto ret_status;
	}

	hdd_notify_teardown_tdls_links(adapter->hdd_vdev);

	roam_profile = &pWextState->roamProfile;
	qdf_mem_zero(&hdd_sta_ctx->conn_info.conn_flag,
		     sizeof(hdd_sta_ctx->conn_info.conn_flag));

	if (roam_profile) {
		struct hdd_station_ctx *sta_ctx;

		sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

		/* Restart the opportunistic timer
		 *
		 * If hw_mode_change_in_progress is true, then wait
		 * till firmware sends the callback for hw_mode change.
		 *
		 * Else set connect_in_progress as true and proceed.
		 */
		policy_mgr_restart_opportunistic_timer(
			hdd_ctx->hdd_psoc, false);
		if (policy_mgr_is_hw_mode_change_in_progress(
			hdd_ctx->hdd_psoc)) {
			qdf_status = policy_mgr_wait_for_connection_update(
				hdd_ctx->hdd_psoc);
			if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
				hdd_err("qdf wait for event failed!!");
				status = -EINVAL;
				goto ret_status;
			}
		}
		hdd_set_connection_in_progress(true);

		if (HDD_WMM_USER_MODE_NO_QOS ==
		    (WLAN_HDD_GET_CTX(adapter))->config->WmmMode) {
			/*QoS not enabled in cfg file */
			roam_profile->uapsd_mask = 0;
		} else {
			/*QoS enabled, update uapsd mask from cfg file */
			roam_profile->uapsd_mask =
				(WLAN_HDD_GET_CTX(adapter))->config->UapsdMask;
		}

		roam_profile->SSIDs.numOfSSIDs = 1;
		roam_profile->SSIDs.SSIDList->SSID.length = ssid_len;
		qdf_mem_zero(roam_profile->SSIDs.SSIDList->SSID.ssId,
			     sizeof(roam_profile->SSIDs.SSIDList->SSID.ssId));
		qdf_mem_copy((void *)(roam_profile->SSIDs.SSIDList->SSID.ssId),
			     ssid, ssid_len);

		roam_profile->supplicant_disabled_roaming = false;

		/* cleanup bssid hint */
		qdf_mem_zero(roam_profile->bssid_hint.bytes,
			QDF_MAC_ADDR_SIZE);
		qdf_mem_zero((void *)(roam_profile->BSSIDs.bssid),
			QDF_MAC_ADDR_SIZE);

		if (bssid) {
			roam_profile->BSSIDs.numOfBSSIDs = 1;
			roam_profile->supplicant_disabled_roaming = true;
			qdf_mem_copy((void *)(roam_profile->BSSIDs.bssid),
				     bssid, QDF_MAC_ADDR_SIZE);
			/*
			 * Save BSSID in seperate variable as
			 * roam_profile's BSSID is getting zeroed out in the
			 * association process. In case of join failure
			 * we should send valid BSSID to supplicant
			 */
			qdf_mem_copy(sta_ctx->requested_bssid.bytes,
				     bssid, QDF_MAC_ADDR_SIZE);
			hdd_debug("bssid is given by upper layer %pM", bssid);
		} else if (bssid_hint) {
			qdf_mem_copy(roam_profile->bssid_hint.bytes,
				bssid_hint, QDF_MAC_ADDR_SIZE);
			/*
			 * Save BSSID in a separate variable as
			 * roam_profile's BSSID is getting zeroed out in the
			 * association process. In case of join failure
			 * we should send valid BSSID to supplicant
			 */
			qdf_mem_copy(sta_ctx->requested_bssid.bytes,
					bssid_hint, QDF_MAC_ADDR_SIZE);
			hdd_debug("bssid_hint is given by upper layer %pM",
					bssid_hint);
		}

		hdd_debug("Connect to SSID: %.*s operating Channel: %u",
		       roam_profile->SSIDs.SSIDList->SSID.length,
		       roam_profile->SSIDs.SSIDList->SSID.ssId,
		       operatingChannel);

		if (hdd_sta_ctx->wpa_versions) {
			hdd_set_genie_to_csr(adapter, &RSNAuthType);
			hdd_set_csr_auth_type(adapter, RSNAuthType);
		}
#ifdef FEATURE_WLAN_WAPI
		if (adapter->wapi_info.wapi_mode) {
			hdd_debug("Setting WAPI AUTH Type and Encryption Mode values");
			switch (adapter->wapi_info.wapi_auth_mode) {
			case WAPI_AUTH_MODE_PSK:
			{
				hdd_debug("WAPI AUTH TYPE: PSK: %d",
				       adapter->wapi_info.wapi_auth_mode);
				roam_profile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_WAPI_WAI_PSK;
				break;
			}
			case WAPI_AUTH_MODE_CERT:
			{
				hdd_debug("WAPI AUTH TYPE: CERT: %d",
				       adapter->wapi_info.wapi_auth_mode);
				roam_profile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_WAPI_WAI_CERTIFICATE;
				break;
			}
			default:
				break;
			} /* End of switch */
			if (adapter->wapi_info.wapi_auth_mode ==
			    WAPI_AUTH_MODE_PSK
			    || adapter->wapi_info.wapi_auth_mode ==
			    WAPI_AUTH_MODE_CERT) {
				hdd_debug("WAPI PAIRWISE/GROUP ENCRYPTION: WPI");
				roam_profile->AuthType.numEntries = 1;
				roam_profile->EncryptionType.numEntries = 1;
				roam_profile->EncryptionType.encryptionType[0] =
					eCSR_ENCRYPT_TYPE_WPI;
				roam_profile->mcEncryptionType.numEntries = 1;
				roam_profile->mcEncryptionType.
				encryptionType[0] = eCSR_ENCRYPT_TYPE_WPI;
			}
		}
#endif
		pmo_ucfg_flush_gtk_offload_req(adapter->hdd_vdev);
		roam_profile->csrPersona = adapter->device_mode;

		if (operatingChannel) {
			roam_profile->ChannelInfo.ChannelList =
				&operatingChannel;
			roam_profile->ChannelInfo.numOfChannels = 1;
		} else {
			roam_profile->ChannelInfo.ChannelList = NULL;
			roam_profile->ChannelInfo.numOfChannels = 0;
		}
		if ((QDF_IBSS_MODE == adapter->device_mode)
		    && operatingChannel) {
			/*
			 * Need to post the IBSS power save parameters
			 * to WMA. WMA will configure this parameters
			 * to firmware if power save is enabled by the
			 * firmware.
			 */
			qdf_status = hdd_set_ibss_power_save_params(adapter);

			if (QDF_STATUS_SUCCESS != qdf_status) {
				hdd_err("Set IBSS Power Save Params Failed");
				status = -EINVAL;
				goto conn_failure;
			}
			roam_profile->ch_params.ch_width =
				hdd_map_nl_chan_width(ch_width);
			/*
			 * In IBSS mode while operating in 2.4 GHz,
			 * the device supports only 20 MHz.
			 */
			if (WLAN_REG_IS_24GHZ_CH(operatingChannel))
				roam_profile->ch_params.ch_width =
					CH_WIDTH_20MHZ;
			hdd_select_cbmode(adapter, operatingChannel,
					  &roam_profile->ch_params);
		}

		if (wlan_hdd_cfg80211_check_pmf_valid(
		   &pWextState->roamProfile)) {
			status = -EINVAL;
			goto conn_failure;
		}

		/*
		 * After 8-way handshake supplicant should give the scan command
		 * in that it update the additional IEs, But because of scan
		 * enhancements, the supplicant is not issuing the scan command
		 * now. So the unicast frames which are sent from the host are
		 * not having the additional IEs. If it is P2P CLIENT and there
		 * is no additional IE present in roamProfile, then use the
		 * addtional IE form scan_info
		 */

		if ((adapter->device_mode == QDF_P2P_CLIENT_MODE) &&
		    (!roam_profile->pAddIEScan)) {
			roam_profile->pAddIEScan =
				&adapter->scan_info.scan_add_ie.addIEdata[0];
			roam_profile->nAddIEScanLength =
				adapter->scan_info.scan_add_ie.length;
		}

		if ((policy_mgr_is_hw_dbs_capable(hdd_ctx->hdd_psoc) == true)
			&& (false == wlan_hdd_handle_sap_sta_dfs_conc(adapter,
				roam_profile))) {
			hdd_err("sap-sta conc will fail, can't allow sta");
			hdd_conn_set_connection_state(adapter,
					eConnectionState_NotConnected);
			status = -ENOMEM;
			goto conn_failure;
		}

		sme_config = qdf_mem_malloc(sizeof(*sme_config));
		if (!sme_config) {
			hdd_err("unable to allocate sme_config");
			hdd_conn_set_connection_state(adapter,
					eConnectionState_NotConnected);
			status = -ENOMEM;
			goto conn_failure;
		}
		sme_get_config_param(hdd_ctx->hHal, sme_config);
		/* These values are not sessionized. So, any change in these SME
		 * configs on an older or parallel interface will affect the
		 * cb mode. So, restoring the default INI params before starting
		 * interfaces such as sta, cli etc.,
		 */
		sme_config->csrConfig.channelBondingMode5GHz =
			hdd_ctx->config->nChannelBondingMode5GHz;
		sme_config->csrConfig.channelBondingMode24GHz =
			hdd_ctx->config->nChannelBondingMode24GHz;
		sme_update_config(hdd_ctx->hHal, sme_config);
		qdf_mem_free(sme_config);
		/*
		 * Change conn_state to connecting before sme_roam_connect(),
		 * because sme_roam_connect() has a direct path to call
		 * hdd_sme_roam_callback(), which will change the conn_state
		 * If direct path, conn_state will be accordingly changed to
		 * NotConnected or Associated by either
		 * hdd_association_completion_handler() or
		 * hdd_dis_connect_handler() in sme_RoamCallback()if
		 * sme_RomConnect is to be queued,
		 * Connecting state will remain until it is completed.
		 *
		 * If connection state is not changed, connection state will
		 * remain in eConnectionState_NotConnected state.
		 * In hdd_association_completion_handler, "hddDisconInProgress"
		 * is set to true if conn state is
		 * eConnectionState_NotConnected.
		 * If "hddDisconInProgress" is set to true then cfg80211 layer
		 * is not informed of connect result indication which
		 * is an issue.
		 */
		if (QDF_STA_MODE == adapter->device_mode ||
			QDF_P2P_CLIENT_MODE == adapter->device_mode)
			hdd_conn_set_connection_state(adapter,
			eConnectionState_Connecting);

		qdf_runtime_pm_prevent_suspend(
				&hdd_ctx->runtime_context.connect);
		hdd_prevent_suspend_timeout(HDD_WAKELOCK_TIMEOUT_CONNECT,
					    WIFI_POWER_EVENT_WAKELOCK_CONNECT);
		qdf_status = sme_roam_connect(WLAN_HDD_GET_HAL_CTX(adapter),
					  adapter->session_id, roam_profile,
					  &roamId);
		if (QDF_IS_STATUS_ERROR(qdf_status))
			status = qdf_status_to_os_return(qdf_status);

		if ((QDF_STATUS_SUCCESS != qdf_status) &&
		    (QDF_STA_MODE == adapter->device_mode ||
		     QDF_P2P_CLIENT_MODE == adapter->device_mode)) {
			hdd_err("sme_roam_connect (session %d) failed with "
			       "qdf_status %d. -> NotConnected",
			       adapter->session_id, qdf_status);
			/* change back to NotAssociated */
			hdd_conn_set_connection_state(adapter,
						      eConnectionState_NotConnected);
			qdf_runtime_pm_allow_suspend(
					&hdd_ctx->runtime_context.connect);
			hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_CONNECT);
		}

		/* Reset connect_in_progress */
		hdd_set_connection_in_progress(false);

		roam_profile->ChannelInfo.ChannelList = NULL;
		roam_profile->ChannelInfo.numOfChannels = 0;

		if ((QDF_STA_MODE == adapter->device_mode)
			&& policy_mgr_is_current_hwmode_dbs(hdd_ctx->hdd_psoc)
			&& !policy_mgr_is_hw_dbs_2x2_capable(
			hdd_ctx->hdd_psoc)) {
			policy_mgr_get_channel_from_scan_result(
				hdd_ctx->hdd_psoc,
				roam_profile, &channel);
			hdd_info("Move to single MAC mode(optimization) if applicable");
			if (channel)
				policy_mgr_checkn_update_hw_mode_single_mac_mode(
					hdd_ctx->hdd_psoc, channel);
		}

	} else {
		hdd_err("No valid Roam profile");
		status = -EINVAL;
	}
	goto ret_status;

conn_failure:
	/* Reset connect_in_progress */
	hdd_set_connection_in_progress(false);

ret_status:
	EXIT();
	return status;
}

/**
 * wlan_hdd_cfg80211_set_auth_type() - set auth type
 * @adapter: Pointer to adapter
 * @auth_type: Auth type
 *
 * This function is used to set the authentication type (OPEN/SHARED).
 *
 * Return: 0 for success, non-zero for failure
 */
static int wlan_hdd_cfg80211_set_auth_type(struct hdd_adapter *adapter,
					   enum nl80211_auth_type auth_type)
{
	struct hdd_wext_state *pWextState =
		WLAN_HDD_GET_WEXT_STATE_PTR(adapter);
	struct hdd_station_ctx *sta_ctx =
		WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	/*set authentication type */
	switch (auth_type) {
	case NL80211_AUTHTYPE_AUTOMATIC:
		hdd_debug("set authentication type to AUTOSWITCH");
		sta_ctx->conn_info.authType = eCSR_AUTH_TYPE_AUTOSWITCH;
		break;

	case NL80211_AUTHTYPE_OPEN_SYSTEM:
	case NL80211_AUTHTYPE_FT:
		hdd_debug("set authentication type to OPEN");
		sta_ctx->conn_info.authType = eCSR_AUTH_TYPE_OPEN_SYSTEM;
		break;

	case NL80211_AUTHTYPE_SHARED_KEY:
		hdd_debug("set authentication type to SHARED");
		sta_ctx->conn_info.authType = eCSR_AUTH_TYPE_SHARED_KEY;
		break;
#ifdef FEATURE_WLAN_ESE
	case NL80211_AUTHTYPE_NETWORK_EAP:
		hdd_debug("set authentication type to CCKM WPA");
		sta_ctx->conn_info.authType = eCSR_AUTH_TYPE_CCKM_WPA;
		break;
#endif
#if defined(WLAN_FEATURE_FILS_SK) && \
	(defined(CFG80211_FILS_SK_OFFLOAD_SUPPORT) || \
		 (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)))
	case NL80211_AUTHTYPE_FILS_SK:
		hdd_notice("set authentication type to FILS SHARED");
		sta_ctx->conn_info.authType = eCSR_AUTH_TYPE_OPEN_SYSTEM;
		break;
#endif
	case NL80211_AUTHTYPE_SAE:
		hdd_debug("set authentication type to SAE");
		sta_ctx->conn_info.authType = eCSR_AUTH_TYPE_SAE;
		break;
	default:
		hdd_err("Unsupported authentication type: %d", auth_type);
		sta_ctx->conn_info.authType = eCSR_AUTH_TYPE_UNKNOWN;
		return -EINVAL;
	}

	pWextState->roamProfile.AuthType.authType[0] =
		sta_ctx->conn_info.authType;
	return 0;
}

#if defined(WLAN_FEATURE_FILS_SK) && \
	(defined(CFG80211_FILS_SK_OFFLOAD_SUPPORT) || \
		 (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)))
static bool hdd_validate_fils_info_ptr(struct hdd_wext_state *wext_state)
{
	struct cds_fils_connection_info *fils_con_info;

	fils_con_info = wext_state->roamProfile.fils_con_info;
	if (!fils_con_info) {
		hdd_err("No valid Roam profile");
		return false;
	}

	return true;
}

static enum eAniAuthType wlan_hdd_get_fils_auth_type(
		enum nl80211_auth_type auth)
{
	switch (auth) {
	case NL80211_AUTHTYPE_FILS_SK:
		return SIR_FILS_SK_WITHOUT_PFS;
	case NL80211_AUTHTYPE_FILS_SK_PFS:
		return SIR_FILS_SK_WITH_PFS;
	case NL80211_AUTHTYPE_FILS_PK:
		return SIR_FILS_PK_AUTH;
	default:
		return eSIR_DONOT_USE_AUTH_TYPE;
	}
}

static bool wlan_hdd_fils_data_in_limits(struct cfg80211_connect_params *req)
{
	hdd_debug("seq=%d auth=%d lengths: user=%zu rrk=%zu realm=%zu",
		  req->fils_erp_next_seq_num, req->auth_type,
		  req->fils_erp_username_len, req->fils_erp_rrk_len,
		  req->fils_erp_realm_len);
	if (!req->fils_erp_rrk_len || !req->fils_erp_realm_len ||
	    !req->fils_erp_username_len ||
	    req->fils_erp_rrk_len > FILS_MAX_RRK_LENGTH ||
	    req->fils_erp_realm_len > FILS_MAX_REALM_LEN ||
	    req->fils_erp_username_len > FILS_MAX_KEYNAME_NAI_LENGTH) {
		hdd_err("length incorrect, user=%zu rrk=%zu realm=%zu",
			req->fils_erp_username_len, req->fils_erp_rrk_len,
			req->fils_erp_realm_len);
		return false;
	}

	return true;
}

/**
 * wlan_hdd_cfg80211_set_fils_config() - set fils config params during connect
 * @adapter: Pointer to adapter
 * @req: Pointer to fils parameters
 *
 * Return: 0 for success, non-zero for failure
 */
static int wlan_hdd_cfg80211_set_fils_config(struct hdd_adapter *adapter,
					 struct cfg80211_connect_params *req)
{
	struct hdd_wext_state *wext_state;
	tCsrRoamProfile *roam_profile;
	enum eAniAuthType auth_type;
	uint8_t *buf;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	wext_state = WLAN_HDD_GET_WEXT_STATE_PTR(adapter);
	roam_profile = &wext_state->roamProfile;

	if (!hdd_ctx->config->is_fils_enabled) {
		hdd_err("FILS disabled");
		return -EINVAL;
	}
	hdd_clear_fils_connection_info(adapter);
	roam_profile->fils_con_info =
		qdf_mem_malloc(sizeof(*roam_profile->fils_con_info));

	if (!roam_profile->fils_con_info) {
		hdd_err("failed to allocate memory");
		return -EINVAL;
	}
	/*
	 * The initial connection for FILS may happen with an OPEN
	 * auth type. Hence we need to allow the connection to go
	 * through in that case as well. Below is_fils_connection
	 * flag is propagated down to CSR and PE sessions through
	 * the JOIN request. As the flag is used, do not free the
	 * memory allocated to fils_con_info and return success.
	 */
	if (req->auth_type != NL80211_AUTHTYPE_FILS_SK) {
		roam_profile->fils_con_info->is_fils_connection = false;
		return 0;
	}

	/*
	 * Once above check is done, then we can check for valid FILS
	 * auth types. Currently only NL80211_AUTHTYPE_FILS_SK is
	 * supported. Once all auth types are supported, then we can
	 * merge these 2 conditions into one.
	 */
	auth_type = wlan_hdd_get_fils_auth_type(req->auth_type);
	if (auth_type == eSIR_DONOT_USE_AUTH_TYPE) {
		hdd_err("invalid auth type for fils %d", req->auth_type);
		goto fils_conn_fail;
	}
	if (!wlan_hdd_fils_data_in_limits(req))
		goto fils_conn_fail;

	roam_profile->fils_con_info->is_fils_connection = true;
	roam_profile->fils_con_info->sequence_number =
		req->fils_erp_next_seq_num;
	roam_profile->fils_con_info->auth_type = auth_type;

	roam_profile->fils_con_info->r_rk_length =
			req->fils_erp_rrk_len;
	if (req->fils_erp_rrk_len)
		qdf_mem_copy(roam_profile->fils_con_info->r_rk,
			req->fils_erp_rrk,
			roam_profile->fils_con_info->r_rk_length);

	roam_profile->fils_con_info->realm_len = req->fils_erp_realm_len;
	if (req->fils_erp_realm_len)
		qdf_mem_copy(roam_profile->fils_con_info->realm,
			req->fils_erp_realm,
			roam_profile->fils_con_info->realm_len);

	roam_profile->fils_con_info->key_nai_length =
		req->fils_erp_username_len + sizeof(char) +
				req->fils_erp_realm_len;
	hdd_debug("key_nai_length = %d",
		  roam_profile->fils_con_info->key_nai_length);
	if (roam_profile->fils_con_info->key_nai_length >
		FILS_MAX_KEYNAME_NAI_LENGTH) {
		hdd_err("Do not allow FILS conn due to excess NAI Length %d",
			roam_profile->fils_con_info->key_nai_length);
		goto fils_conn_fail;
	}
	buf = roam_profile->fils_con_info->keyname_nai;
	qdf_mem_copy(buf, req->fils_erp_username, req->fils_erp_username_len);
	buf += req->fils_erp_username_len;
	*buf++ = '@';
	qdf_mem_copy(buf, req->fils_erp_realm, req->fils_erp_realm_len);

	return 0;

fils_conn_fail:
	if (roam_profile->fils_con_info) {
		qdf_mem_free(roam_profile->fils_con_info);
		roam_profile->fils_con_info = NULL;
	}
	return -EINVAL;
}

static bool wlan_hdd_is_akm_suite_fils(uint32_t key_mgmt)
{
	switch (key_mgmt) {
	case WLAN_AKM_SUITE_FILS_SHA256:
	case WLAN_AKM_SUITE_FILS_SHA384:
	case WLAN_AKM_SUITE_FT_FILS_SHA256:
	case WLAN_AKM_SUITE_FT_FILS_SHA384:
		return true;
	default:
		return false;
	}
}

static bool wlan_hdd_is_conn_type_fils(struct cfg80211_connect_params *req)
{
	enum nl80211_auth_type auth_type = req->auth_type;
	/*
	 * Below n_akm_suites is defined as int in the kernel, even though it
	 * is supposed to be unsigned.
	 */
	int num_akm_suites = req->crypto.n_akm_suites;
	uint32_t key_mgmt = req->crypto.akm_suites[0];
	enum eAniAuthType fils_auth_type =
		wlan_hdd_get_fils_auth_type(req->auth_type);

	hdd_debug("Num of AKM suites = %d", num_akm_suites);
	if (num_akm_suites <= 0)
		return false;

	/*
	 * Auth type will be either be OPEN or FILS type for a FILS connection
	 */
	if ((auth_type != NL80211_AUTHTYPE_OPEN_SYSTEM) &&
	    (fils_auth_type == eSIR_DONOT_USE_AUTH_TYPE)) {
		hdd_debug("Not a FILS auth type, auth = %d, fils auth = %d",
			  auth_type, fils_auth_type);
		return false;
	}

	if (!wlan_hdd_is_akm_suite_fils(key_mgmt)) {
		hdd_debug("Not a FILS AKM SUITE %d", key_mgmt);
		return false;
	}

	return true;
}

#else
static bool hdd_validate_fils_info_ptr(struct hdd_wext_state *wext_state)
{
	return true;
}

static int wlan_hdd_cfg80211_set_fils_config(struct hdd_adapter *adapter,
					 struct cfg80211_connect_params *req)
{
	return 0;
}

static bool wlan_hdd_is_akm_suite_fils(uint32_t key_mgmt)
{
	return false;
}

static bool wlan_hdd_is_conn_type_fils(struct cfg80211_connect_params *req)
{
	return false;
}
#endif

/**
 * wlan_hdd_set_akm_suite() - set key management type
 * @adapter: Pointer to adapter
 * @key_mgmt: Key management type
 *
 * This function is used to set the key mgmt type(PSK/8021x).
 *
 * Return: 0 for success, non-zero for failure
 */
static int wlan_hdd_set_akm_suite(struct hdd_adapter *adapter, u32 key_mgmt)
{
	struct hdd_wext_state *pWextState =
		WLAN_HDD_GET_WEXT_STATE_PTR(adapter);
	tCsrRoamProfile *roam_profile;

	roam_profile = &pWextState->roamProfile;

	if (wlan_hdd_is_akm_suite_fils(key_mgmt) &&
	    !hdd_validate_fils_info_ptr(pWextState))
		return -EINVAL;
#ifndef WLAN_AKM_SUITE_8021X_SHA256
#define WLAN_AKM_SUITE_8021X_SHA256 0x000FAC05
#endif
#ifndef WLAN_AKM_SUITE_PSK_SHA256
#define WLAN_AKM_SUITE_PSK_SHA256   0x000FAC06
#endif
	/*set key mgmt type */
	switch (key_mgmt) {
	case WLAN_AKM_SUITE_PSK:
	case WLAN_AKM_SUITE_PSK_SHA256:
	case WLAN_AKM_SUITE_FT_PSK:
	case WLAN_AKM_SUITE_DPP_RSN:
		hdd_debug("setting key mgmt type to PSK");
		pWextState->authKeyMgmt |= IW_AUTH_KEY_MGMT_PSK;
		break;

	case WLAN_AKM_SUITE_8021X_SHA256:
	case WLAN_AKM_SUITE_8021X:
	case WLAN_AKM_SUITE_FT_8021X:
		hdd_debug("setting key mgmt type to 8021x");
		pWextState->authKeyMgmt |= IW_AUTH_KEY_MGMT_802_1X;
		break;
#ifdef FEATURE_WLAN_ESE
#define WLAN_AKM_SUITE_CCKM         0x00409600  /* Should be in ieee802_11_defs.h */
#define IW_AUTH_KEY_MGMT_CCKM       8   /* Should be in linux/wireless.h */
	case WLAN_AKM_SUITE_CCKM:
		hdd_debug("setting key mgmt type to CCKM");
		pWextState->authKeyMgmt |= IW_AUTH_KEY_MGMT_CCKM;
		break;
#endif
#ifndef WLAN_AKM_SUITE_OSEN
#define WLAN_AKM_SUITE_OSEN         0x506f9a01  /* Should be in ieee802_11_defs.h */
#endif
	case WLAN_AKM_SUITE_OSEN:
		hdd_debug("setting key mgmt type to OSEN");
		pWextState->authKeyMgmt |= IW_AUTH_KEY_MGMT_802_1X;
		break;
#if defined(WLAN_FEATURE_FILS_SK) && \
	(defined(CFG80211_FILS_SK_OFFLOAD_SUPPORT) || \
		 (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)))
	case WLAN_AKM_SUITE_FILS_SHA256:
		hdd_debug("setting key mgmt type to FILS SHA256");
		pWextState->authKeyMgmt |= IW_AUTH_KEY_MGMT_802_1X;
		roam_profile->fils_con_info->akm_type =
			eCSR_AUTH_TYPE_FILS_SHA256;
		break;

	case WLAN_AKM_SUITE_FILS_SHA384:
		hdd_debug("setting key mgmt type to FILS SHA384");
		pWextState->authKeyMgmt |= IW_AUTH_KEY_MGMT_802_1X;
		roam_profile->fils_con_info->akm_type =
			eCSR_AUTH_TYPE_FILS_SHA384;
		break;

	case WLAN_AKM_SUITE_FT_FILS_SHA256:
		hdd_debug("setting key mgmt type to FILS FT SHA256");
		pWextState->authKeyMgmt |= IW_AUTH_KEY_MGMT_802_1X;
		roam_profile->fils_con_info->akm_type =
			eCSR_AUTH_TYPE_FT_FILS_SHA256;
		break;

	case WLAN_AKM_SUITE_FT_FILS_SHA384:
		hdd_debug("setting key mgmt type to FILS FT SHA384");
		pWextState->authKeyMgmt |= IW_AUTH_KEY_MGMT_802_1X;
		roam_profile->fils_con_info->akm_type =
			eCSR_AUTH_TYPE_FT_FILS_SHA384;
		break;
#endif

	case WLAN_AKM_SUITE_OWE:
		hdd_debug("setting key mgmt type to OWE");
		pWextState->authKeyMgmt |= IW_AUTH_KEY_MGMT_802_1X;
		break;

	case WLAN_AKM_SUITE_EAP_SHA256:
		hdd_debug("setting key mgmt type to EAP_SHA256");
		pWextState->authKeyMgmt |= IW_AUTH_KEY_MGMT_802_1X;
		break;
	case WLAN_AKM_SUITE_EAP_SHA384:
		hdd_debug("setting key mgmt type to EAP_SHA384");
		pWextState->authKeyMgmt |= IW_AUTH_KEY_MGMT_802_1X;
		break;

	case WLAN_AKM_SUITE_SAE:
		hdd_debug("setting key mgmt type to SAE");
		pWextState->authKeyMgmt |= IW_AUTH_KEY_MGMT_802_1X;
		break;

	default:
		hdd_err("Unsupported key mgmt type: %d", key_mgmt);
		return -EINVAL;

	}
	return 0;
}

/**
 * wlan_hdd_cfg80211_set_cipher() - set encryption type
 * @adapter: Pointer to adapter
 * @cipher: Cipher type
 * @ucast: Unicast flag
 *
 * This function is used to set the encryption type
 * (NONE/WEP40/WEP104/TKIP/CCMP).
 *
 * Return: 0 for success, non-zero for failure
 */
static int wlan_hdd_cfg80211_set_cipher(struct hdd_adapter *adapter,
					u32 cipher, bool ucast)
{
	eCsrEncryptionType encryptionType = eCSR_ENCRYPT_TYPE_NONE;
	struct hdd_wext_state *pWextState =
		WLAN_HDD_GET_WEXT_STATE_PTR(adapter);
	struct hdd_station_ctx *sta_ctx =
		WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	if (!cipher) {
		hdd_debug("received cipher %d - considering none", cipher);
		encryptionType = eCSR_ENCRYPT_TYPE_NONE;
	} else {

		/*set encryption method */
		switch (cipher) {
		case IW_AUTH_CIPHER_NONE:
			encryptionType = eCSR_ENCRYPT_TYPE_NONE;
			break;

		case WLAN_CIPHER_SUITE_WEP40:
			encryptionType = eCSR_ENCRYPT_TYPE_WEP40;
			break;

		case WLAN_CIPHER_SUITE_WEP104:
			encryptionType = eCSR_ENCRYPT_TYPE_WEP104;
			break;

		case WLAN_CIPHER_SUITE_TKIP:
			encryptionType = eCSR_ENCRYPT_TYPE_TKIP;
			break;

		case WLAN_CIPHER_SUITE_CCMP:
			encryptionType = eCSR_ENCRYPT_TYPE_AES;
			break;
#ifdef FEATURE_WLAN_WAPI
		case WLAN_CIPHER_SUITE_SMS4:
			encryptionType = eCSR_ENCRYPT_TYPE_WPI;
			break;
#endif

#ifdef FEATURE_WLAN_ESE
		case WLAN_CIPHER_SUITE_KRK:
			encryptionType = eCSR_ENCRYPT_TYPE_KRK;
			break;
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
		case WLAN_CIPHER_SUITE_BTK:
			encryptionType = eCSR_ENCRYPT_TYPE_BTK;
			break;
#endif
#endif
		case WLAN_CIPHER_SUITE_GCMP:
			encryptionType = eCSR_ENCRYPT_TYPE_AES_GCMP;
			break;
		case WLAN_CIPHER_SUITE_GCMP_256:
			encryptionType = eCSR_ENCRYPT_TYPE_AES_GCMP_256;
			break;
		default:
			hdd_err("Unsupported cipher type: %d", cipher);
			return -EOPNOTSUPP;
		}
	}

	if (ucast) {
		hdd_debug("setting unicast cipher type to %d", encryptionType);
		sta_ctx->conn_info.ucEncryptionType = encryptionType;
		pWextState->roamProfile.EncryptionType.numEntries = 1;
		pWextState->roamProfile.EncryptionType.encryptionType[0] =
			encryptionType;
	} else {
		hdd_debug("setting mcast cipher type to %d", encryptionType);
		sta_ctx->conn_info.mcEncryptionType = encryptionType;
		pWextState->roamProfile.mcEncryptionType.numEntries = 1;
		pWextState->roamProfile.mcEncryptionType.encryptionType[0] =
			encryptionType;
	}

	return 0;
}

/**
 * wlan_hdd_add_assoc_ie() - Add Assoc IE to roamProfile
 * @wext_state: Pointer to wext state
 * @gen_ie: Pointer to IE data
 * @len: length of IE data
 *
 * Return: 0 for success, non-zero for failure
 */
static int wlan_hdd_add_assoc_ie(struct hdd_wext_state *wext_state,
				const uint8_t *gen_ie, uint16_t len)
{
	uint16_t cur_add_ie_len =
		wext_state->assocAddIE.length;

	if (SIR_MAC_MAX_ADD_IE_LENGTH <
			(wext_state->assocAddIE.length + len)) {
		hdd_err("Cannot accommodate assocAddIE Need bigger buffer space");
		QDF_ASSERT(0);
		return -ENOMEM;
	}
	memcpy(wext_state->assocAddIE.addIEdata +
			cur_add_ie_len, gen_ie, len);
	wext_state->assocAddIE.length += len;

	wext_state->roamProfile.pAddIEAssoc =
		wext_state->assocAddIE.addIEdata;
	wext_state->roamProfile.nAddIEAssocLength =
		wext_state->assocAddIE.length;
	return 0;
}

#ifdef WLAN_FEATURE_FILS_SK
/**
 * wlan_hdd_save_hlp_ie - API to save HLP IE
 * @roam_profile: Pointer to roam profile
 * @gen_ie: IE buffer to store
 * @len: length of the IE buffer @gen_ie
 * @flush: Flush the older saved HLP if any
 *
 * Return: None
 */
static void wlan_hdd_save_hlp_ie(tCsrRoamProfile *roam_profile,
				const uint8_t *gen_ie, uint16_t len,
				bool flush)
{
	uint8_t *hlp_ie = roam_profile->hlp_ie;

	if (flush) {
		roam_profile->hlp_ie_len = 0;
		if (hlp_ie) {
			qdf_mem_free(hlp_ie);
			roam_profile->hlp_ie = NULL;
		}
	}

	if ((roam_profile->hlp_ie_len +
			len) > FILS_MAX_HLP_DATA_LEN) {
		hdd_err("HLP len exceeds: hlp_ie_len %d len %d",
			roam_profile->hlp_ie_len, len);
		return;
	}

	if (!roam_profile->hlp_ie) {
		roam_profile->hlp_ie =
				qdf_mem_malloc(FILS_MAX_HLP_DATA_LEN);
		hlp_ie = roam_profile->hlp_ie;
		if (!hlp_ie) {
			hdd_err("HLP IE mem alloc fails");
			return;
		}
	}

	qdf_mem_copy(hlp_ie + roam_profile->hlp_ie_len, gen_ie, len);
	roam_profile->hlp_ie_len += len;
}
#else
static inline void wlan_hdd_save_hlp_ie(tCsrRoamProfile *roam_profile,
				const uint8_t *gen_ie, uint16_t len,
				bool flush)
{}
#endif
/**
 * wlan_hdd_cfg80211_set_ie() - set IEs
 * @adapter: Pointer to adapter
 * @ie: Pointer ot ie
 * @ie: IE length
 *
 * Return: 0 for success, non-zero for failure
 */
static int wlan_hdd_cfg80211_set_ie(struct hdd_adapter *adapter,
				    const uint8_t *ie,
				    size_t ie_len)
{
	struct hdd_wext_state *pWextState =
		WLAN_HDD_GET_WEXT_STATE_PTR(adapter);
	const uint8_t *genie = ie;
	uint16_t remLen = ie_len;
#ifdef FEATURE_WLAN_WAPI
	uint32_t akmsuite[MAX_NUM_AKM_SUITES];
	u16 *tmp;
	uint16_t akmsuiteCount;
	int *akmlist;
#endif
	int status;

	/* clear previous assocAddIE */
	pWextState->assocAddIE.length = 0;
	pWextState->roamProfile.bWPSAssociation = false;
	pWextState->roamProfile.bOSENAssociation = false;

	while (remLen >= 2) {
		uint16_t eLen = 0;
		uint8_t elementId;

		elementId = *genie++;
		eLen = *genie++;
		remLen -= 2;

		/* Sanity check on eLen */
		if (eLen > remLen) {
			hdd_err("%s: Invalid IE length[%d] for IE[0x%X]",
				__func__, eLen, elementId);
			QDF_ASSERT(0);
			return -EINVAL;
		}

		hdd_debug("IE[0x%X], LEN[%d]", elementId, eLen);

		switch (elementId) {
		case DOT11F_EID_WPA:
			if (4 > eLen) { /* should have at least OUI which is 4 bytes so extra 2 bytes not needed */
				hdd_err("Invalid WPA IE");
				return -EINVAL;
			} else if (0 ==
				   memcmp(&genie[0], "\x00\x50\xf2\x04", 4)) {
				uint16_t curAddIELen =
					pWextState->assocAddIE.length;
				hdd_debug("Set WPS IE(len %d)", eLen + 2);

				if (SIR_MAC_MAX_ADD_IE_LENGTH <
				    (pWextState->assocAddIE.length + eLen)) {
					hdd_err("Cannot accommodate assocAddIE. Need bigger buffer space");
					QDF_ASSERT(0);
					return -ENOMEM;
				}
				/* WSC IE is saved to Additional IE ; it should be accumulated to handle WPS IE + P2P IE */
				memcpy(pWextState->assocAddIE.addIEdata +
				       curAddIELen, genie - 2, eLen + 2);
				pWextState->assocAddIE.length += eLen + 2;

				pWextState->roamProfile.bWPSAssociation = true;
				pWextState->roamProfile.pAddIEAssoc =
					pWextState->assocAddIE.addIEdata;
				pWextState->roamProfile.nAddIEAssocLength =
					pWextState->assocAddIE.length;
			} else if (0 == memcmp(&genie[0], "\x00\x50\xf2", 3)) {
				if (eLen > (MAX_WPA_RSN_IE_LEN - 2)) {
					hdd_err("%s: Invalid WPA IE length[%d]",
						__func__, eLen);
					QDF_ASSERT(0);
					return -EINVAL;
				}
				hdd_debug("Set WPA IE (len %d)", eLen + 2);
				memset(pWextState->WPARSNIE, 0,
				       MAX_WPA_RSN_IE_LEN);
				memcpy(pWextState->WPARSNIE, genie - 2,
				       (eLen + 2));
				pWextState->roamProfile.pWPAReqIE =
					pWextState->WPARSNIE;
				pWextState->roamProfile.nWPAReqIELength = eLen + 2;     /* ie_len; */
			} else if ((0 == memcmp(&genie[0], P2P_OUI_TYPE,
						P2P_OUI_TYPE_SIZE))) {
				uint16_t curAddIELen =
					pWextState->assocAddIE.length;
				hdd_debug("Set P2P IE(len %d)", eLen + 2);

				if (SIR_MAC_MAX_ADD_IE_LENGTH <
				    (pWextState->assocAddIE.length + eLen)) {
					hdd_err("Cannot accommodate assocAddIE Need bigger buffer space");
					QDF_ASSERT(0);
					return -ENOMEM;
				}
				/* P2P IE is saved to Additional IE ; it should be accumulated to handle WPS IE + P2P IE */
				memcpy(pWextState->assocAddIE.addIEdata +
				       curAddIELen, genie - 2, eLen + 2);
				pWextState->assocAddIE.length += eLen + 2;

				pWextState->roamProfile.pAddIEAssoc =
					pWextState->assocAddIE.addIEdata;
				pWextState->roamProfile.nAddIEAssocLength =
					pWextState->assocAddIE.length;
			}
#ifdef WLAN_FEATURE_WFD
			else if ((0 == memcmp(&genie[0], WFD_OUI_TYPE,
					      WFD_OUI_TYPE_SIZE)) &&
				/* Consider WFD IE, only for P2P Client */
				 (QDF_P2P_CLIENT_MODE ==
				     adapter->device_mode)) {
				uint16_t curAddIELen =
					pWextState->assocAddIE.length;
				hdd_debug("Set WFD IE(len %d)", eLen + 2);

				if (SIR_MAC_MAX_ADD_IE_LENGTH <
				    (pWextState->assocAddIE.length + eLen)) {
					hdd_err("Cannot accommodate assocAddIE Need bigger buffer space");
					QDF_ASSERT(0);
					return -ENOMEM;
				}
				/* WFD IE is saved to Additional IE ; it should
				 * be accumulated to handle WPS IE + P2P IE +
				 * WFD IE
				 */
				memcpy(pWextState->assocAddIE.addIEdata +
				       curAddIELen, genie - 2, eLen + 2);
				pWextState->assocAddIE.length += eLen + 2;

				pWextState->roamProfile.pAddIEAssoc =
					pWextState->assocAddIE.addIEdata;
				pWextState->roamProfile.nAddIEAssocLength =
					pWextState->assocAddIE.length;
			}
#endif
			/* Appending HS 2.0 Indication Element in Assiciation Request */
			else if ((0 == memcmp(&genie[0], HS20_OUI_TYPE,
					      HS20_OUI_TYPE_SIZE))) {
				uint16_t curAddIELen =
					pWextState->assocAddIE.length;
				hdd_debug("Set HS20 IE(len %d)", eLen + 2);

				if (SIR_MAC_MAX_ADD_IE_LENGTH <
				    (pWextState->assocAddIE.length + eLen)) {
					hdd_err("Cannot accommodate assocAddIE Need bigger buffer space");
					QDF_ASSERT(0);
					return -ENOMEM;
				}
				memcpy(pWextState->assocAddIE.addIEdata +
				       curAddIELen, genie - 2, eLen + 2);
				pWextState->assocAddIE.length += eLen + 2;

				pWextState->roamProfile.pAddIEAssoc =
					pWextState->assocAddIE.addIEdata;
				pWextState->roamProfile.nAddIEAssocLength =
					pWextState->assocAddIE.length;
			}
			/* Appending OSEN Information  Element in Assiciation Request */
			else if ((0 == memcmp(&genie[0], OSEN_OUI_TYPE,
					      OSEN_OUI_TYPE_SIZE))) {
				uint16_t curAddIELen =
					pWextState->assocAddIE.length;
				hdd_debug("Set OSEN IE(len %d)", eLen + 2);

				if (SIR_MAC_MAX_ADD_IE_LENGTH <
				    (pWextState->assocAddIE.length + eLen)) {
					hdd_err("Cannot accommodate assocAddIE Need bigger buffer space");
					QDF_ASSERT(0);
					return -ENOMEM;
				}
				memcpy(pWextState->assocAddIE.addIEdata +
				       curAddIELen, genie - 2, eLen + 2);
				pWextState->assocAddIE.length += eLen + 2;

				pWextState->roamProfile.bOSENAssociation = true;
				pWextState->roamProfile.pAddIEAssoc =
					pWextState->assocAddIE.addIEdata;
				pWextState->roamProfile.nAddIEAssocLength =
					pWextState->assocAddIE.length;
			} else if ((0 == memcmp(&genie[0], MBO_OUI_TYPE,
							MBO_OUI_TYPE_SIZE))){
				hdd_debug("Set MBO IE(len %d)", eLen + 2);
				status = wlan_hdd_add_assoc_ie(pWextState,
							genie - 2, eLen + 2);
				if (status)
					return status;
			} else {
				uint16_t add_ie_len =
					pWextState->assocAddIE.length;

				hdd_debug("Set OSEN IE(len %d)", eLen + 2);

				if (SIR_MAC_MAX_ADD_IE_LENGTH <
				    (pWextState->assocAddIE.length + eLen)) {
					hdd_err("Cannot accommodate assocAddIE Need bigger buffer space");
					QDF_ASSERT(0);
					return -ENOMEM;
				}

				memcpy(pWextState->assocAddIE.addIEdata +
				       add_ie_len, genie - 2, eLen + 2);
				pWextState->assocAddIE.length += eLen + 2;

				pWextState->roamProfile.pAddIEAssoc =
					pWextState->assocAddIE.addIEdata;
				pWextState->roamProfile.nAddIEAssocLength =
					pWextState->assocAddIE.length;
			}
			break;
		case DOT11F_EID_RSN:
			hdd_debug("Set RSN IE(len %d)", eLen + 2);
			if (eLen > (MAX_WPA_RSN_IE_LEN - 2)) {
				hdd_err("%s: Invalid WPA RSN IE length[%d]",
					__func__, eLen);
				return -EINVAL;
			}
			memset(pWextState->WPARSNIE, 0, MAX_WPA_RSN_IE_LEN);
			memcpy(pWextState->WPARSNIE, genie - 2,
			       (eLen + 2));
			pWextState->roamProfile.pRSNReqIE =
				pWextState->WPARSNIE;
			pWextState->roamProfile.nRSNReqIELength = eLen + 2;     /* ie_len; */
			break;
		/*
		 * Appending Extended Capabilities with Interworking bit set
		 * in Assoc Req.
		 *
		 * In assoc req this EXT Cap will only be taken into account if
		 * interworkingService bit is set to 1. Currently
		 * driver is only interested in interworkingService capability
		 * from supplicant. If in future any other EXT Cap info is
		 * required from supplicat, it needs to be handled while
		 * sending Assoc Req in LIM.
		 */
		case DOT11F_EID_EXTCAP:
		{
			uint16_t curAddIELen =
				pWextState->assocAddIE.length;
			hdd_debug("Set Extended CAPS IE(len %d)", eLen + 2);

			if (SIR_MAC_MAX_ADD_IE_LENGTH <
			    (pWextState->assocAddIE.length + eLen)) {
				hdd_err("Cannot accommodate assocAddIE Need bigger buffer space");
				QDF_ASSERT(0);
				return -ENOMEM;
			}
			memcpy(pWextState->assocAddIE.addIEdata +
			       curAddIELen, genie - 2, eLen + 2);
			pWextState->assocAddIE.length += eLen + 2;

			pWextState->roamProfile.pAddIEAssoc =
				pWextState->assocAddIE.addIEdata;
			pWextState->roamProfile.nAddIEAssocLength =
				pWextState->assocAddIE.length;
			break;
		}
#ifdef FEATURE_WLAN_WAPI
		case WLAN_EID_WAPI:
			/* Setting WAPI Mode to ON=1 */
			adapter->wapi_info.wapi_mode = true;
			hdd_debug("WAPI MODE IS %u", adapter->wapi_info.wapi_mode);
			tmp = (u16 *) ie;
			tmp = tmp + 2;  /* Skip element Id and Len, Version */
			akmsuiteCount = WPA_GET_LE16(tmp);
			tmp = tmp + 1;
			akmlist = (int *)(tmp);
			if (akmsuiteCount <= MAX_NUM_AKM_SUITES) {
				memcpy(akmsuite, akmlist, (4 * akmsuiteCount));
			} else {
				hdd_err("Invalid akmSuite count: %u",
					akmsuiteCount);
				QDF_ASSERT(0);
				return -EINVAL;
			}

			if (WAPI_PSK_AKM_SUITE == akmsuite[0]) {
				hdd_debug("WAPI AUTH MODE SET TO PSK");
				adapter->wapi_info.wapi_auth_mode =
					WAPI_AUTH_MODE_PSK;
			}
			if (WAPI_CERT_AKM_SUITE == akmsuite[0]) {
				hdd_debug("WAPI AUTH MODE SET TO CERTIFICATE");
				adapter->wapi_info.wapi_auth_mode =
					WAPI_AUTH_MODE_CERT;
			}
			break;
#endif
		case DOT11F_EID_SUPPOPERATINGCLASSES:
			{
				hdd_debug("Set Supported Operating Classes IE(len %d)", eLen + 2);
				status = wlan_hdd_add_assoc_ie(pWextState,
							genie - 2, eLen + 2);
				if (status)
					return status;
				break;
			}
		case SIR_MAC_REQUEST_EID_MAX:
			{
				if (genie[0] == SIR_FILS_HLP_EXT_EID) {
					hdd_debug("Set HLP EXT IE(len %d)",
							eLen + 2);
					wlan_hdd_save_hlp_ie(&pWextState->
							roamProfile,
							genie - 2, eLen + 2,
							true);
					status = wlan_hdd_add_assoc_ie(
							pWextState, genie - 2,
							eLen + 2);
					if (status)
						return status;
				} else if (genie[0] ==
					   SIR_DH_PARAMETER_ELEMENT_EXT_EID) {
					hdd_debug("Set DH EXT IE(len %d)",
							eLen + 2);
					status = wlan_hdd_add_assoc_ie(
							pWextState, genie - 2,
							eLen + 2);
					if (status)
						return status;
				} else {
					hdd_err("UNKNOWN EID: %X", genie[0]);
				}
				break;
			}
		case DOT11F_EID_FRAGMENT_IE:
			{
				hdd_debug("Set Fragment IE(len %d)", eLen + 2);
				wlan_hdd_save_hlp_ie(&pWextState->roamProfile,
							genie - 2, eLen + 2,
							false);
				status = wlan_hdd_add_assoc_ie(pWextState,
							genie - 2, eLen + 2);
				if (status)
					return status;
				break;
			}
		default:
			hdd_err("Set UNKNOWN IE: %X", elementId);
			/* when Unknown IE is received we break
			 * and continue to the next IE in the buffer
			 */
			break;
		}
		genie += eLen;
		remLen -= eLen;
	}
	return 0;
}

/**
 * hdd_is_wpaie_present() - check for WPA ie
 * @ie: Pointer to ie
 * @ie_len: Ie length
 *
 * Parse the received IE to find the WPA IE
 *
 * Return: true if wpa ie is found else false
 */
static bool hdd_is_wpaie_present(const uint8_t *ie, uint8_t ie_len)
{
	uint8_t eLen = 0;
	uint16_t remLen = ie_len;
	uint8_t elementId = 0;

	while (remLen >= 2) {
		elementId = *ie++;
		eLen = *ie++;
		remLen -= 2;
		if (eLen > remLen) {
			hdd_err("Invalid IE length: %d", eLen);
			return false;
		}
		if ((elementId == DOT11F_EID_WPA) && (remLen > 5)) {
			/* OUI - 0x00 0X50 0XF2
			 * WPA Information Element - 0x01
			 * WPA version - 0x01
			 */
			if (0 == memcmp(&ie[0], "\x00\x50\xf2\x01\x01", 5))
				return true;
		}
		ie += eLen;
		remLen -= eLen;
	}
	return false;
}

/**
 * wlan_hdd_cfg80211_set_privacy() - set security parameters during connection
 * @adapter: Pointer to adapter
 * @req: Pointer to security parameters
 *
 * Return: 0 for success, non-zero for failure
 */
static int wlan_hdd_cfg80211_set_privacy(struct hdd_adapter *adapter,
					 struct cfg80211_connect_params *req)
{
	int status = 0;
	struct hdd_wext_state *pWextState =
		WLAN_HDD_GET_WEXT_STATE_PTR(adapter);
	struct hdd_station_ctx *sta_ctx;

	ENTER();

	sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	sta_ctx->wpa_versions = req->crypto.wpa_versions;
	hdd_debug("set wpa version to %d", sta_ctx->wpa_versions);

	/*set authentication type */
	status = wlan_hdd_cfg80211_set_auth_type(adapter, req->auth_type);

	if (wlan_hdd_is_conn_type_fils(req)) {
		status = wlan_hdd_cfg80211_set_fils_config(adapter, req);

		if (0 > status) {
			hdd_err("Failed to set fils config");
			return status;
		}
	}

	/*set key mgmt type */
	if (req->crypto.n_akm_suites) {
		status =
			wlan_hdd_set_akm_suite(adapter, req->crypto.akm_suites[0]);
		if (0 > status) {
			hdd_err("Failed to set akm suite");
			return status;
		}
	}

	/*set pairwise cipher type */
	if (req->crypto.n_ciphers_pairwise) {
		status = wlan_hdd_cfg80211_set_cipher(adapter,
						      req->crypto.
						      ciphers_pairwise[0],
						      true);
		if (0 > status) {
			hdd_err("Failed to set unicast cipher type");
			return status;
		}
	} else {
		/*Reset previous cipher suite to none */
		status = wlan_hdd_cfg80211_set_cipher(adapter, 0, true);
		if (0 > status) {
			hdd_err("Failed to set unicast cipher type");
			return status;
		}
	}

	/*set group cipher type */
	status =
		wlan_hdd_cfg80211_set_cipher(adapter, req->crypto.cipher_group,
					     false);

	if (0 > status) {
		hdd_err("Failed to set mcast cipher type");
		return status;
	}
#ifdef WLAN_FEATURE_11W
	pWextState->roamProfile.MFPEnabled = (req->mfp == NL80211_MFP_REQUIRED);
#endif

	/*parse WPA/RSN IE, and set the correspoing fileds in Roam profile */
	if (req->ie_len) {
		status =
			wlan_hdd_cfg80211_set_ie(adapter, req->ie, req->ie_len);
		if (0 > status) {
			hdd_err("Failed to parse the WPA/RSN IE");
			return status;
		}
	}

	/*incase of WEP set default key information */
	if (req->key && req->key_len) {
		u8 key_len = req->key_len;
		u8 key_idx = req->key_idx;

		if ((WLAN_CIPHER_SUITE_WEP40 == req->crypto.ciphers_pairwise[0])
		    || (WLAN_CIPHER_SUITE_WEP104 ==
			req->crypto.ciphers_pairwise[0])
		    ) {
			if (IW_AUTH_KEY_MGMT_802_1X
			    ==
			    (pWextState->
			     authKeyMgmt & IW_AUTH_KEY_MGMT_802_1X)) {
				hdd_err("Dynamic WEP not supported");
				return -EOPNOTSUPP;
			}

			if ((eCSR_SECURITY_WEP_KEYSIZE_MAX_BYTES >= key_len)
			    && (CSR_MAX_NUM_KEY > key_idx)) {
				hdd_debug("setting default wep key, key_idx = %hu key_len %hu",
					   key_idx, key_len);
				qdf_mem_copy(&pWextState->roamProfile.
					     Keys.
					     KeyMaterial[key_idx][0],
					     req->key, key_len);
				pWextState->roamProfile.Keys.
					KeyLength[key_idx] = (u8) key_len;
				pWextState->roamProfile.Keys.
					defaultIndex = (u8) key_idx;
			}
		}
	}

	return status;
}

int wlan_hdd_try_disconnect(struct hdd_adapter *adapter)
{
	unsigned long rc;
	struct hdd_station_ctx *sta_ctx;
	int status, result = 0;
	tHalHandle hal;

	sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	hal = WLAN_HDD_GET_HAL_CTX(adapter);
	if (adapter->device_mode ==  QDF_STA_MODE) {
		sme_indicate_disconnect_inprogress(hal, adapter->session_id);
		hdd_debug("Stop firmware roaming");
		sme_stop_roaming(hal, adapter->session_id, eCsrForcedDisassoc);

		/*
		 * If firmware has already started roaming process, driver
		 * needs to wait for processing of this disconnect request.
		 *
		 */
		INIT_COMPLETION(adapter->roaming_comp_var);
		if (hdd_is_roaming_in_progress(adapter)) {
			rc = wait_for_completion_timeout(
				&adapter->roaming_comp_var,
				msecs_to_jiffies(WLAN_WAIT_TIME_STOP_ROAM));
			if (!rc) {
				hdd_err("roaming comp var timed out session Id: %d",
					adapter->session_id);
			}
			if (adapter->roam_ho_fail) {
				INIT_COMPLETION(adapter->disconnect_comp_var);
				hdd_conn_set_connection_state(adapter,
						eConnectionState_Disconnecting);
			}
		}
	}

	if ((QDF_IBSS_MODE == adapter->device_mode) ||
	  (eConnectionState_Associated == sta_ctx->conn_info.connState) ||
	  (eConnectionState_Connecting == sta_ctx->conn_info.connState) ||
	  (eConnectionState_IbssConnected == sta_ctx->conn_info.connState)) {
		hdd_conn_set_connection_state(adapter,
						eConnectionState_Disconnecting);
		/* Issue disconnect to CSR */
		INIT_COMPLETION(adapter->disconnect_comp_var);

		status = sme_roam_disconnect(WLAN_HDD_GET_HAL_CTX(adapter),
				adapter->session_id,
				eCSR_DISCONNECT_REASON_UNSPECIFIED);
		/*
		 * Wait here instead of returning directly, this will block the
		 * next connect command and allow processing of the scan for
		 * ssid and the previous connect command in CSR. Else we might
		 * hit some race conditions leading to SME and HDD out of sync.
		 */
		if (QDF_STATUS_CMD_NOT_QUEUED == status) {
			hdd_debug("Already disconnected or connect was in sme/roam pending list and removed by disconnect");
		} else if (0 != status) {
			hdd_err("sme_roam_disconnect failure, status: %d",
				(int)status);
			sta_ctx->sta_debug_state = status;
			result = -EINVAL;
			goto disconnected;
		}

		rc = wait_for_completion_timeout(
			&adapter->disconnect_comp_var,
			msecs_to_jiffies(WLAN_WAIT_TIME_DISCONNECT));
		if (!rc && (QDF_STATUS_CMD_NOT_QUEUED != status)) {
			hdd_err("Sme disconnect event timed out session Id: %d sta_debug_state: %d",
				adapter->session_id, sta_ctx->sta_debug_state);
			result = -ETIMEDOUT;
		}
	} else if (eConnectionState_Disconnecting ==
				sta_ctx->conn_info.connState) {
		rc = wait_for_completion_timeout(&adapter->disconnect_comp_var,
				msecs_to_jiffies(WLAN_WAIT_TIME_DISCONNECT));
		if (!rc) {
			hdd_err("Disconnect event timed out session Id: %d sta_debug_state: %d",
				adapter->session_id, sta_ctx->sta_debug_state);
			result = -ETIMEDOUT;
		}
	}
disconnected:
	hdd_conn_set_connection_state(adapter, eConnectionState_NotConnected);
	return result;
}

/**
 * wlan_hdd_reassoc_bssid_hint() - Start reassociation if bssid is present
 * @adapter: Pointer to the HDD adapter
 * @req: Pointer to the structure cfg_connect_params receieved from user space
 * @status: out variable for status of reassoc request
 *
 * This function will start reassociation if prev_bssid is set and bssid/
 * bssid_hint, channel/channel_hint parameters are present in connect request.
 *
 * Return: true if connect was for ReAssociation, false otherwise
 */
#if defined(CFG80211_CONNECT_PREV_BSSID) || \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
static bool wlan_hdd_reassoc_bssid_hint(struct hdd_adapter *adapter,
					struct cfg80211_connect_params *req,
					int *status)
{
	bool reassoc = false;
	const uint8_t *bssid = NULL;
	uint16_t channel = 0;
	struct hdd_station_ctx *sta_ctx;

	if (req->bssid)
		bssid = req->bssid;
	else if (req->bssid_hint)
		bssid = req->bssid_hint;

	if (req->channel)
		channel = req->channel->hw_value;
	else if (req->channel_hint)
		channel = req->channel_hint->hw_value;

	if (bssid && channel && req->prev_bssid) {
		reassoc = true;
		hdd_debug(FL("REASSOC Attempt on channel %d to "MAC_ADDRESS_STR),
				channel, MAC_ADDR_ARRAY(bssid));
		/*
		 * Save BSSID in a separate variable as
		 * roam_profile's BSSID is getting zeroed out in the
		 * association process. In case of join failure
		 * we should send valid BSSID to supplicant
		 */
		sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
		qdf_mem_copy(sta_ctx->requested_bssid.bytes, bssid,
			     QDF_MAC_ADDR_SIZE);

		*status = hdd_reassoc(adapter, bssid, channel,
				      CONNECT_CMD_USERSPACE);
		hdd_debug("hdd_reassoc: status: %d", *status);
	}
	return reassoc;
}
#else
static bool wlan_hdd_reassoc_bssid_hint(struct hdd_adapter *adapter,
					struct cfg80211_connect_params *req,
					int *status)
{
	return false;
}
#endif


/**
 * wlan_hdd_check_ht20_ht40_ind() - check if Supplicant has indicated to
 * connect in HT20 mode
 * @hdd_ctx: hdd context
 * @adapter: Pointer to the HDD adapter
 * @req: Pointer to the structure cfg_connect_params receieved from user space
 *
 * This function will check if supplicant has indicated to to connect in HT20
 * mode. this is currently applicable only for 2.4Ghz mode only.
 * if feature is enabled and supplicant indicate HT20 set
 * force_24ghz_in_ht20 to true to force 2.4Ghz in HT20 else set it to false.
 *
 * Return: void
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 3, 0)
static void wlan_hdd_check_ht20_ht40_ind(struct hdd_context *hdd_ctx,
	struct hdd_adapter *adapter,
	struct cfg80211_connect_params *req)
{
	struct hdd_wext_state *wext_state =
		WLAN_HDD_GET_WEXT_STATE_PTR(adapter);
	tCsrRoamProfile *roam_profile;

	roam_profile = &wext_state->roamProfile;
	roam_profile->force_24ghz_in_ht20 = false;

	if (hdd_ctx->config->override_ht20_40_24g &&
	    !(req->ht_capa.cap_info &
	      IEEE80211_HT_CAP_SUP_WIDTH_20_40))
		roam_profile->force_24ghz_in_ht20 = true;

	hdd_debug("req->ht_capa.cap_info %x override_ht20_40_24g %d",
		  req->ht_capa.cap_info,
		  hdd_ctx->config->override_ht20_40_24g);
}
#else
static inline void wlan_hdd_check_ht20_ht40_ind(struct hdd_context *hdd_ctx,
	struct hdd_adapter *adapter,
	struct cfg80211_connect_params *req)
{
	struct hdd_wext_state *wext_state =
		WLAN_HDD_GET_WEXT_STATE_PTR(adapter);
	tCsrRoamProfile *roam_profile;

	roam_profile = &wext_state->roamProfile;

	roam_profile->force_24ghz_in_ht20 = false;
}
#endif

/**
 * __wlan_hdd_cfg80211_connect() - cfg80211 connect api
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @req: Pointer to cfg80211 connect request
 *
 * This function is used to start the association process
 *
 * Return: 0 for success, non-zero for failure
 */
static int __wlan_hdd_cfg80211_connect(struct wiphy *wiphy,
				       struct net_device *ndev,
				       struct cfg80211_connect_params *req)
{
	int status;
	u16 channel;
	const u8 *bssid = NULL;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 0))
	const u8 *bssid_hint = req->bssid_hint;
#else
	const u8 *bssid_hint = NULL;
#endif
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(ndev);
	struct hdd_context *hdd_ctx;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(adapter->session_id)) {
		hdd_err("invalid session id: %d", adapter->session_id);
		return -EINVAL;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_CONNECT,
			 adapter->session_id, adapter->device_mode));
	hdd_debug("Device_mode %s(%d)",
		hdd_device_mode_to_string(adapter->device_mode),
		adapter->device_mode);

	if (adapter->device_mode != QDF_STA_MODE &&
		adapter->device_mode != QDF_P2P_CLIENT_MODE) {
		hdd_err("Device_mode %s(%d) is not supported",
			hdd_device_mode_to_string(adapter->device_mode),
			adapter->device_mode);
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (!hdd_ctx) {
		hdd_err("HDD context is null");
		return -EINVAL;
	}

	status = wlan_hdd_validate_context(hdd_ctx);
	if (0 != status)
		return status;

	if (req->bssid)
		bssid = req->bssid;
	else if (bssid_hint)
		bssid = bssid_hint;

	if (bssid && hdd_get_adapter_by_macaddr(hdd_ctx, (uint8_t *)bssid)) {
		hdd_err("adapter exist with same mac address " MAC_ADDRESS_STR,
			MAC_ADDR_ARRAY(bssid));
		return -EINVAL;
	}

	if (true == wlan_hdd_reassoc_bssid_hint(adapter, req, &status))
		return status;

	/* Try disconnecting if already in connected state */
	status = wlan_hdd_try_disconnect(adapter);
	if (0 > status) {
		hdd_err("Failed to disconnect the existing connection");
		return -EALREADY;
	}

	/* Check for max concurrent connections after doing disconnect if any */
	if (req->channel) {
		bool ok = false;

		if (req->channel->hw_value && policy_mgr_is_chan_ok_for_dnbs(
						hdd_ctx->hdd_psoc,
						req->channel->hw_value,
						&ok)) {
			hdd_warn("Unable to get channel:%d eligibility for DNBS",
					req->channel->hw_value);
			return -EINVAL;
		}
		/**
		 * Send connection timedout, so that Android framework does not
		 * blacklist us.
		 */
		if (!ok) {
			struct ieee80211_channel *chan =
				ieee80211_get_channel(wiphy,
				wlan_chan_to_freq(req->channel->hw_value));
			struct cfg80211_bss *bss;

			hdd_warn("Channel:%d not OK for DNBS",
				req->channel->hw_value);
			if (chan) {
				bss = hdd_cfg80211_get_bss(wiphy,
							chan,
							req->bssid, req->ssid,
							req->ssid_len);
				if (bss) {
					cfg80211_assoc_timeout(ndev, bss);
					return -ETIMEDOUT;
				}
			}
			return -EINVAL;
		}

		if (!policy_mgr_allow_concurrency(hdd_ctx->hdd_psoc,
				policy_mgr_convert_device_mode_to_qdf_type(
				adapter->device_mode),
				req->channel->hw_value, HW_MODE_20_MHZ)) {
			hdd_warn("This concurrency combination is not allowed");
			return -ECONNREFUSED;
		}
	} else {
		if (!policy_mgr_allow_concurrency(hdd_ctx->hdd_psoc,
				policy_mgr_convert_device_mode_to_qdf_type(
				adapter->device_mode), 0, HW_MODE_20_MHZ)) {
			hdd_warn("This concurrency combination is not allowed");
			return -ECONNREFUSED;
		}
	}

	/*initialise security parameters */
	status = wlan_hdd_cfg80211_set_privacy(adapter, req);

	if (0 > status) {
		hdd_err("Failed to set security params");
		return status;
	}

	if (req->channel)
		channel = req->channel->hw_value;
	else
		channel = 0;

	wlan_hdd_check_ht20_ht40_ind(hdd_ctx, adapter, req);

	status = wlan_hdd_cfg80211_connect_start(adapter, req->ssid,
						 req->ssid_len, req->bssid,
						 bssid_hint, channel, 0);
	if (0 > status) {
		hdd_err("connect failed");
		return status;
	}
	EXIT();
	return status;
}

/**
 * wlan_hdd_cfg80211_connect() - cfg80211 connect api
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @req: Pointer to cfg80211 connect request
 *
 * Return: 0 for success, non-zero for failure
 */
static int wlan_hdd_cfg80211_connect(struct wiphy *wiphy,
				     struct net_device *ndev,
				     struct cfg80211_connect_params *req)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_connect(wiphy, ndev, req);
	cds_ssr_unprotect(__func__);

	return ret;
}

int wlan_hdd_disconnect(struct hdd_adapter *adapter, u16 reason)
{
	int status, result = 0;
	unsigned long rc;
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	eConnectionState prev_conn_state;
	tHalHandle hal = WLAN_HDD_GET_HAL_CTX(adapter);
	uint32_t wait_time = WLAN_WAIT_TIME_DISCONNECT;

	ENTER();

	status = wlan_hdd_validate_context(hdd_ctx);

	if (0 != status)
		return status;
	if (adapter->device_mode ==  QDF_STA_MODE) {
		sme_indicate_disconnect_inprogress(hal, adapter->session_id);
		hdd_debug("Stop firmware roaming");
		status = sme_stop_roaming(hal, adapter->session_id,
				eCsrForcedDisassoc);
		/*
		 * If firmware has already started roaming process, driver
		 * needs to wait for processing of this disconnect request.
		 *
		 */
		INIT_COMPLETION(adapter->roaming_comp_var);
		if (hdd_is_roaming_in_progress(adapter)) {
			rc = wait_for_completion_timeout(
				&adapter->roaming_comp_var,
				msecs_to_jiffies(WLAN_WAIT_TIME_STOP_ROAM));
			if (!rc) {
				hdd_err("roaming comp var timed out session Id: %d",
					adapter->session_id);
			}
			if (adapter->roam_ho_fail) {
				INIT_COMPLETION(adapter->disconnect_comp_var);
				hdd_debug("Disabling queues");
				wlan_hdd_netif_queue_control(adapter,
					WLAN_STOP_ALL_NETIF_QUEUE_N_CARRIER,
					WLAN_CONTROL_PATH);
				hdd_conn_set_connection_state(adapter,
						eConnectionState_Disconnecting);
				goto wait_for_disconnect;
			}
		}
	}

	prev_conn_state = sta_ctx->conn_info.connState;
	/*stop tx queues */
	hdd_info("Disabling queues");
	wlan_hdd_netif_queue_control(adapter,
		WLAN_STOP_ALL_NETIF_QUEUE_N_CARRIER, WLAN_CONTROL_PATH);
	hdd_debug("Set HDD connState to eConnectionState_Disconnecting");
	hdd_conn_set_connection_state(adapter, eConnectionState_Disconnecting);

	INIT_COMPLETION(adapter->disconnect_comp_var);

	/* issue disconnect */

	status = sme_roam_disconnect(WLAN_HDD_GET_HAL_CTX(adapter),
				     adapter->session_id, reason);
	if ((QDF_STATUS_CMD_NOT_QUEUED == status) &&
			prev_conn_state != eConnectionState_Connecting) {
		hdd_debug("status = %d, already disconnected", status);
		result = 0;
		/*
		 * Wait here instead of returning directly. This will block the
		 * next connect command and allow processing of the disconnect
		 * in SME else we might hit some race conditions leading to SME
		 * and HDD out of sync. As disconnect is already in progress,
		 * wait here for 1 sec instead of 5 sec.
		 */
		wait_time = WLAN_WAIT_DISCONNECT_ALREADY_IN_PROGRESS;
	} else if (QDF_STATUS_CMD_NOT_QUEUED == status) {
		/*
		 * Wait here instead of returning directly, this will block the
		 * next connect command and allow processing of the scan for
		 * ssid and the previous connect command in CSR. Else we might
		 * hit some race conditions leading to SME and HDD out of sync.
		 */
		hdd_debug("Already disconnected or connect was in sme/roam pending list and removed by disconnect");
	} else if (0 != status) {
		hdd_err("csr_roam_disconnect failure, status: %d", (int)status);
		sta_ctx->sta_debug_state = status;
		result = -EINVAL;
		goto disconnected;
	}
wait_for_disconnect:
	rc = wait_for_completion_timeout(&adapter->disconnect_comp_var,
					 msecs_to_jiffies(wait_time));

	if (!rc && (QDF_STATUS_CMD_NOT_QUEUED != status)) {
		hdd_err("Failed to disconnect, timed out");
		result = -ETIMEDOUT;
	}
disconnected:
	hdd_conn_set_connection_state(adapter, eConnectionState_NotConnected);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0)
	/* Sending disconnect event to userspace for kernel version < 3.11
	 * is handled by __cfg80211_disconnect call to __cfg80211_disconnected
	 */
	hdd_debug("Send disconnected event to userspace");
	wlan_hdd_cfg80211_indicate_disconnect(adapter->dev, true,
						WLAN_REASON_UNSPECIFIED);
#endif

	return result;
}

/**
 * hdd_ieee80211_reason_code_to_str() - return string conversion of reason code
 * @reason: ieee80211 reason code.
 *
 * This utility function helps log string conversion of reason code.
 *
 * Return: string conversion of reason code, if match found;
 *         "Unknown" otherwise.
 */
static const char *hdd_ieee80211_reason_code_to_str(uint16_t reason)
{
	switch (reason) {
	CASE_RETURN_STRING(WLAN_REASON_UNSPECIFIED);
	CASE_RETURN_STRING(WLAN_REASON_PREV_AUTH_NOT_VALID);
	CASE_RETURN_STRING(WLAN_REASON_DEAUTH_LEAVING);
	CASE_RETURN_STRING(WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY);
	CASE_RETURN_STRING(WLAN_REASON_DISASSOC_AP_BUSY);
	CASE_RETURN_STRING(WLAN_REASON_CLASS2_FRAME_FROM_NONAUTH_STA);
	CASE_RETURN_STRING(WLAN_REASON_CLASS3_FRAME_FROM_NONASSOC_STA);
	CASE_RETURN_STRING(WLAN_REASON_DISASSOC_STA_HAS_LEFT);
	CASE_RETURN_STRING(WLAN_REASON_STA_REQ_ASSOC_WITHOUT_AUTH);
	CASE_RETURN_STRING(WLAN_REASON_DISASSOC_BAD_POWER);
	CASE_RETURN_STRING(WLAN_REASON_DISASSOC_BAD_SUPP_CHAN);
	CASE_RETURN_STRING(WLAN_REASON_INVALID_IE);
	CASE_RETURN_STRING(WLAN_REASON_MIC_FAILURE);
	CASE_RETURN_STRING(WLAN_REASON_4WAY_HANDSHAKE_TIMEOUT);
	CASE_RETURN_STRING(WLAN_REASON_GROUP_KEY_HANDSHAKE_TIMEOUT);
	CASE_RETURN_STRING(WLAN_REASON_IE_DIFFERENT);
	CASE_RETURN_STRING(WLAN_REASON_INVALID_GROUP_CIPHER);
	CASE_RETURN_STRING(WLAN_REASON_INVALID_PAIRWISE_CIPHER);
	CASE_RETURN_STRING(WLAN_REASON_INVALID_AKMP);
	CASE_RETURN_STRING(WLAN_REASON_UNSUPP_RSN_VERSION);
	CASE_RETURN_STRING(WLAN_REASON_INVALID_RSN_IE_CAP);
	CASE_RETURN_STRING(WLAN_REASON_IEEE8021X_FAILED);
	CASE_RETURN_STRING(WLAN_REASON_CIPHER_SUITE_REJECTED);
	CASE_RETURN_STRING(WLAN_REASON_DISASSOC_UNSPECIFIED_QOS);
	CASE_RETURN_STRING(WLAN_REASON_DISASSOC_QAP_NO_BANDWIDTH);
	CASE_RETURN_STRING(WLAN_REASON_DISASSOC_LOW_ACK);
	CASE_RETURN_STRING(WLAN_REASON_DISASSOC_QAP_EXCEED_TXOP);
	CASE_RETURN_STRING(WLAN_REASON_QSTA_LEAVE_QBSS);
	CASE_RETURN_STRING(WLAN_REASON_QSTA_NOT_USE);
	CASE_RETURN_STRING(WLAN_REASON_QSTA_REQUIRE_SETUP);
	CASE_RETURN_STRING(WLAN_REASON_QSTA_TIMEOUT);
	CASE_RETURN_STRING(WLAN_REASON_QSTA_CIPHER_NOT_SUPP);
	CASE_RETURN_STRING(WLAN_REASON_MESH_PEER_CANCELED);
	CASE_RETURN_STRING(WLAN_REASON_MESH_MAX_PEERS);
	CASE_RETURN_STRING(WLAN_REASON_MESH_CONFIG);
	CASE_RETURN_STRING(WLAN_REASON_MESH_CLOSE);
	CASE_RETURN_STRING(WLAN_REASON_MESH_MAX_RETRIES);
	CASE_RETURN_STRING(WLAN_REASON_MESH_CONFIRM_TIMEOUT);
	CASE_RETURN_STRING(WLAN_REASON_MESH_INVALID_GTK);
	CASE_RETURN_STRING(WLAN_REASON_MESH_INCONSISTENT_PARAM);
	CASE_RETURN_STRING(WLAN_REASON_MESH_INVALID_SECURITY);
	CASE_RETURN_STRING(WLAN_REASON_MESH_PATH_ERROR);
	CASE_RETURN_STRING(WLAN_REASON_MESH_PATH_NOFORWARD);
	CASE_RETURN_STRING(WLAN_REASON_MESH_PATH_DEST_UNREACHABLE);
	CASE_RETURN_STRING(WLAN_REASON_MAC_EXISTS_IN_MBSS);
	CASE_RETURN_STRING(WLAN_REASON_MESH_CHAN_REGULATORY);
	CASE_RETURN_STRING(WLAN_REASON_MESH_CHAN);
	default:
		return "Unknown";
	}
}

/**
 * __wlan_hdd_cfg80211_disconnect() - cfg80211 disconnect api
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @reason: Disconnect reason code
 *
 * This function is used to issue a disconnect request to SME
 *
 * Return: 0 for success, non-zero for failure
 */
static int __wlan_hdd_cfg80211_disconnect(struct wiphy *wiphy,
					  struct net_device *dev, u16 reason)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	int status;
	struct hdd_station_ctx *sta_ctx =
		WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(adapter->session_id)) {
		hdd_err("Invalid session id: %d", adapter->session_id);
		return -EINVAL;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_DISCONNECT,
			 adapter->session_id, reason));
	hdd_debug("Device_mode %s(%d) reason code(%d)",
		hdd_device_mode_to_string(adapter->device_mode),
		adapter->device_mode, reason);

	status = wlan_hdd_validate_context(hdd_ctx);

	if (0 != status)
		return status;

	/* Issue disconnect request to SME, if station is in connected state */
	if ((sta_ctx->conn_info.connState == eConnectionState_Associated) ||
	    (sta_ctx->conn_info.connState == eConnectionState_Connecting)) {
		eCsrRoamDisconnectReason reasonCode =
			eCSR_DISCONNECT_REASON_UNSPECIFIED;

		switch (reason) {
		case WLAN_REASON_MIC_FAILURE:
			reasonCode = eCSR_DISCONNECT_REASON_MIC_ERROR;
			break;

		case WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY:
		case WLAN_REASON_DISASSOC_AP_BUSY:
		case WLAN_REASON_CLASS3_FRAME_FROM_NONASSOC_STA:
			reasonCode = eCSR_DISCONNECT_REASON_DISASSOC;
			break;

		case WLAN_REASON_PREV_AUTH_NOT_VALID:
		case WLAN_REASON_CLASS2_FRAME_FROM_NONAUTH_STA:
			reasonCode = eCSR_DISCONNECT_REASON_DEAUTH;
			break;

		case WLAN_REASON_DEAUTH_LEAVING:
			reasonCode =
				hdd_ctx->config->
				gEnableDeauthToDisassocMap ?
				eCSR_DISCONNECT_REASON_STA_HAS_LEFT :
				eCSR_DISCONNECT_REASON_DEAUTH;
			qdf_dp_trace_dump_all(
				WLAN_DEAUTH_DPTRACE_DUMP_COUNT,
				QDF_TRACE_DEFAULT_PDEV_ID);
			break;
		case WLAN_REASON_DISASSOC_STA_HAS_LEFT:
			reasonCode = eCSR_DISCONNECT_REASON_STA_HAS_LEFT;
			break;
		default:
			reasonCode = eCSR_DISCONNECT_REASON_UNSPECIFIED;
			break;
		}
		if (ucfg_scan_get_vdev_status(adapter->hdd_vdev) !=
				SCAN_NOT_IN_PROGRESS) {
			hdd_debug("Disconnect is in progress, Aborting Scan");
			wlan_abort_scan(hdd_ctx->hdd_pdev, INVAL_PDEV_ID,
				adapter->session_id, INVALID_SCAN_ID, false);
		}
		wlan_hdd_cleanup_remain_on_channel_ctx(adapter);
		/* First clean up the tdls peers if any */
		hdd_notify_sta_disconnect(adapter->session_id,
			  false, true, adapter->hdd_vdev);

		hdd_info("Disconnect request from user space with reason: %d (%s) internal reason code: %d",
			reason, hdd_ieee80211_reason_code_to_str(reason), reasonCode);
		status = wlan_hdd_disconnect(adapter, reasonCode);
		if (0 != status) {
			hdd_err("wlan_hdd_disconnect failed, status: %d", status);
			return -EINVAL;
		}
	} else {
		hdd_err("Unexpected cfg disconnect called while in state: %d",
		       sta_ctx->conn_info.connState);
	}

	return status;
}

/**
 * wlan_hdd_cfg80211_disconnect() - cfg80211 disconnect api
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @reason: Disconnect reason code
 *
 * Return: 0 for success, non-zero for failure
 */
static int wlan_hdd_cfg80211_disconnect(struct wiphy *wiphy,
					struct net_device *dev, u16 reason)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_disconnect(wiphy, dev, reason);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * wlan_hdd_cfg80211_set_privacy_ibss() - set ibss privacy
 * @adapter: Pointer to adapter
 * @param: Pointer to IBSS parameters
 *
 * This function is used to initialize the security settings in IBSS mode
 *
 * Return: 0 for success, non-zero for failure
 */
static int wlan_hdd_cfg80211_set_privacy_ibss(struct hdd_adapter *adapter,
					      struct cfg80211_ibss_params
					      *params)
{
	uint32_t ret;
	int status = 0;
	struct hdd_wext_state *pWextState =
		WLAN_HDD_GET_WEXT_STATE_PTR(adapter);
	eCsrEncryptionType encryptionType = eCSR_ENCRYPT_TYPE_NONE;
	struct hdd_station_ctx *sta_ctx =
		WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	ENTER();

	sta_ctx->wpa_versions = 0;
	qdf_mem_zero(&sta_ctx->ibss_enc_key, sizeof(tCsrRoamSetKey));
	sta_ctx->ibss_enc_key_installed = 0;

	if (params->ie_len && (NULL != params->ie)) {
		if (wlan_get_ie_ptr_from_eid(WLAN_EID_RSN, params->ie,
					     params->ie_len)) {
			sta_ctx->wpa_versions = NL80211_WPA_VERSION_2;
			encryptionType = eCSR_ENCRYPT_TYPE_AES;
		} else if (hdd_is_wpaie_present(params->ie, params->ie_len)) {
			tDot11fIEWPA dot11WPAIE;
			tHalHandle halHandle = WLAN_HDD_GET_HAL_CTX(adapter);
			const u8 *ie;

			memset(&dot11WPAIE, 0, sizeof(dot11WPAIE));
			ie = wlan_get_ie_ptr_from_eid(DOT11F_EID_WPA,
						params->ie, params->ie_len);
			if (NULL != ie) {
				sta_ctx->wpa_versions = NL80211_WPA_VERSION_1;
				/* Unpack the WPA IE
				 * Skip past the EID byte and length byte
				 * and four byte WiFi OUI
				 */
				if (ie[1] < DOT11F_IE_WPA_MIN_LEN ||
				    ie[1] > DOT11F_IE_WPA_MAX_LEN) {
					hdd_err("invalid ie len:%d", ie[1]);
					return -EINVAL;
				}
				ret = dot11f_unpack_ie_wpa(
						(tpAniSirGlobal) halHandle,
						(uint8_t *)&ie[2 + 4],
						ie[1] - 4, &dot11WPAIE, false);
				if (DOT11F_FAILED(ret)) {
					hdd_err("unpack failed ret: 0x%x", ret);
					return -EINVAL;
				}
				/*
				 * Extract the multicast cipher, the
				 * encType for unicast cipher for
				 * wpa-none is none
				 */
				encryptionType =
					hdd_translate_wpa_to_csr_encryption_type
						(dot11WPAIE.multicast_cipher);
			}
		}

		status =
			wlan_hdd_cfg80211_set_ie(adapter, params->ie,
						 params->ie_len);

		if (0 > status) {
			hdd_err("Failed to parse WPA/RSN IE");
			return status;
		}
	}

	pWextState->roamProfile.AuthType.authType[0] =
		sta_ctx->conn_info.authType = eCSR_AUTH_TYPE_OPEN_SYSTEM;

	if (params->privacy) {
		/* Security enabled IBSS, At this time there is no information
		 * available about the security paramters, so initialise the
		 * encryption type to eCSR_ENCRYPT_TYPE_WEP40_STATICKEY.
		 * The correct security parameters will be updated later in
		 * wlan_hdd_cfg80211_add_key Hal expects encryption type to be
		 * set inorder enable privacy bit in beacons
		 */

		encryptionType = eCSR_ENCRYPT_TYPE_WEP40_STATICKEY;
	}
	hdd_debug("encryptionType=%d", encryptionType);
	sta_ctx->conn_info.ucEncryptionType = encryptionType;
	pWextState->roamProfile.EncryptionType.numEntries = 1;
	pWextState->roamProfile.EncryptionType.encryptionType[0] =
		encryptionType;
	return status;
}

/**
 * __wlan_hdd_cfg80211_join_ibss() - join ibss
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @param: Pointer to IBSS join parameters
 *
 * This function is used to create/join an IBSS network
 *
 * Return: 0 for success, non-zero for failure
 */
static int __wlan_hdd_cfg80211_join_ibss(struct wiphy *wiphy,
					 struct net_device *dev,
					 struct cfg80211_ibss_params *params)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_wext_state *pWextState =
		WLAN_HDD_GET_WEXT_STATE_PTR(adapter);
	tCsrRoamProfile *roam_profile;
	int status;
	struct hdd_station_ctx *sta_ctx =
		WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct qdf_mac_addr bssid;
	u8 channelNum = 0;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(adapter->session_id)) {
		hdd_err("Invalid session id: %d", adapter->session_id);
		return -EINVAL;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_JOIN_IBSS,
			 adapter->session_id, adapter->device_mode));
	hdd_debug("Device_mode %s(%d)",
		hdd_device_mode_to_string(adapter->device_mode),
		adapter->device_mode);

	status = wlan_hdd_validate_context(hdd_ctx);

	if (0 != status)
		return status;

	if (NULL !=
		params->chandef.chan) {
		uint32_t numChans = WNI_CFG_VALID_CHANNEL_LIST_LEN;
		uint8_t validChan[WNI_CFG_VALID_CHANNEL_LIST_LEN];
		tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(adapter);
		int indx;

		/* Get channel number */
		channelNum = ieee80211_frequency_to_channel(
			params->
			chandef.
			chan->
			center_freq);

		if (0 != sme_cfg_get_str(hHal, WNI_CFG_VALID_CHANNEL_LIST,
					 validChan, &numChans)) {
			hdd_err("No valid channel list");
			return -EOPNOTSUPP;
		}

		for (indx = 0; indx < numChans; indx++) {
			if (channelNum == validChan[indx])
				break;
		}
		if (indx >= numChans) {
			hdd_err("Not valid Channel: %d", channelNum);
			return -EINVAL;
		}
	}

	if (!policy_mgr_allow_concurrency(hdd_ctx->hdd_psoc,
		PM_IBSS_MODE, channelNum, HW_MODE_20_MHZ)) {
		hdd_err("This concurrency combination is not allowed");
		return -ECONNREFUSED;
	}

	status = policy_mgr_reset_connection_update(hdd_ctx->hdd_psoc);
	if (!QDF_IS_STATUS_SUCCESS(status))
		hdd_err("qdf_reset_connection_update failed status: %d", status);

	status = policy_mgr_current_connections_update(hdd_ctx->hdd_psoc,
					adapter->session_id, channelNum,
					SIR_UPDATE_REASON_JOIN_IBSS);
	if (QDF_STATUS_E_FAILURE == status) {
		hdd_err("connections update failed!!");
		return -EINVAL;
	}

	if (QDF_STATUS_SUCCESS == status) {
		status = policy_mgr_wait_for_connection_update(
			hdd_ctx->hdd_psoc);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			hdd_err("qdf wait for event failed!!");
			return -EINVAL;
		}
	}

	/*Try disconnecting if already in connected state */
	status = wlan_hdd_try_disconnect(adapter);
	if (0 > status) {
		hdd_err("Failed to disconnect the existing IBSS connection");
		return -EALREADY;
	}

	roam_profile = &pWextState->roamProfile;

	if (eCSR_BSS_TYPE_START_IBSS != roam_profile->BSSType) {
		hdd_err("Interface type is not set to IBSS");
		return -EINVAL;
	}

	/* enable selected protection checks in IBSS mode */
	roam_profile->cfg_protection = IBSS_CFG_PROTECTION_ENABLE_MASK;

	if (QDF_STATUS_E_FAILURE == sme_cfg_set_int(hdd_ctx->hHal,
						    WNI_CFG_IBSS_ATIM_WIN_SIZE,
						    hdd_ctx->config->
						    ibssATIMWinSize)) {
		hdd_err("Could not pass on WNI_CFG_IBSS_ATIM_WIN_SIZE to CCM");
	}

	/* BSSID is provided by upper layers hence no need to AUTO generate */
	if (NULL != params->bssid) {
		if (sme_cfg_set_int(hdd_ctx->hHal, WNI_CFG_IBSS_AUTO_BSSID, 0)
				== QDF_STATUS_E_FAILURE) {
			hdd_err("ccmCfgStInt failed for WNI_CFG_IBSS_AUTO_BSSID");
			return -EIO;
		}
		qdf_mem_copy(bssid.bytes, params->bssid, QDF_MAC_ADDR_SIZE);
	} else if (hdd_ctx->config->isCoalesingInIBSSAllowed == 0) {
		if (sme_cfg_set_int(hdd_ctx->hHal, WNI_CFG_IBSS_AUTO_BSSID, 0)
				== QDF_STATUS_E_FAILURE) {
			hdd_err("ccmCfgStInt failed for WNI_CFG_IBSS_AUTO_BSSID");
			return -EIO;
		}
		qdf_copy_macaddr(&bssid, &hdd_ctx->config->IbssBssid);
	}
	if ((params->beacon_interval > CFG_BEACON_INTERVAL_MIN)
	    && (params->beacon_interval <= CFG_BEACON_INTERVAL_MAX))
		roam_profile->beaconInterval = params->beacon_interval;
	else {
		roam_profile->beaconInterval = CFG_BEACON_INTERVAL_DEFAULT;
		hdd_debug("input beacon interval %d TU is invalid, use default %d TU",
			params->beacon_interval, roam_profile->beaconInterval);
	}

	/* Set Channel */
	if (channelNum)	{
		/* Set the Operational Channel */
		hdd_debug("set channel %d", channelNum);
		roam_profile->ChannelInfo.numOfChannels = 1;
		sta_ctx->conn_info.operationChannel = channelNum;
		roam_profile->ChannelInfo.ChannelList =
			&sta_ctx->conn_info.operationChannel;
	}

	/* Initialize security parameters */
	status = wlan_hdd_cfg80211_set_privacy_ibss(adapter, params);
	if (status < 0) {
		hdd_err("failed to set security parameters");
		return status;
	}

	/* Issue connect start */
	status = wlan_hdd_cfg80211_connect_start(adapter, params->ssid,
						 params->ssid_len,
						 bssid.bytes, NULL,
						 sta_ctx->conn_info.
						 operationChannel,
						 params->chandef.width);

	if (0 > status) {
		hdd_err("connect failed");
		return status;
	}
	EXIT();
	return 0;
}

/**
 * wlan_hdd_cfg80211_join_ibss() - join ibss
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @param: Pointer to IBSS join parameters
 *
 * This function is used to create/join an IBSS network
 *
 * Return: 0 for success, non-zero for failure
 */
static int wlan_hdd_cfg80211_join_ibss(struct wiphy *wiphy,
				       struct net_device *dev,
				       struct cfg80211_ibss_params *params)
{
	int ret = 0;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_join_ibss(wiphy, dev, params);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __wlan_hdd_cfg80211_leave_ibss() - leave ibss
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 *
 * This function is used to leave an IBSS network
 *
 * Return: 0 for success, non-zero for failure
 */
static int __wlan_hdd_cfg80211_leave_ibss(struct wiphy *wiphy,
					  struct net_device *dev)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_wext_state *pWextState =
		WLAN_HDD_GET_WEXT_STATE_PTR(adapter);
	tCsrRoamProfile *roam_profile;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	int status;
	QDF_STATUS hal_status;
	unsigned long rc;
	tSirUpdateIE updateIE;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(adapter->session_id)) {
		hdd_err("Invalid session id: %d", adapter->session_id);
		return -EINVAL;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_LEAVE_IBSS,
			 adapter->session_id,
			 eCSR_DISCONNECT_REASON_IBSS_LEAVE));
	status = wlan_hdd_validate_context(hdd_ctx);
	if (0 != status)
		return status;

	hdd_debug("Device_mode %s(%d)",
		hdd_device_mode_to_string(adapter->device_mode),
		adapter->device_mode);
	if (NULL == pWextState) {
		hdd_err("Data Storage Corruption");
		return -EIO;
	}

	roam_profile = &pWextState->roamProfile;

	/* Issue disconnect only if interface type is set to IBSS */
	if (eCSR_BSS_TYPE_START_IBSS != roam_profile->BSSType) {
		hdd_err("BSS Type is not set to IBSS");
		return -EINVAL;
	}
	/* Clearing add IE of beacon */
	qdf_mem_copy(updateIE.bssid.bytes, adapter->mac_addr.bytes,
		     sizeof(tSirMacAddr));
	updateIE.smeSessionId = adapter->session_id;
	updateIE.ieBufferlength = 0;
	updateIE.pAdditionIEBuffer = NULL;
	updateIE.append = true;
	updateIE.notify = true;
	if (sme_update_add_ie(WLAN_HDD_GET_HAL_CTX(adapter),
			      &updateIE,
			      eUPDATE_IE_PROBE_BCN) == QDF_STATUS_E_FAILURE) {
		hdd_err("Could not pass on PROBE_RSP_BCN data to PE");
	}

	/* Reset WNI_CFG_PROBE_RSP Flags */
	wlan_hdd_reset_prob_rspies(adapter);

	/* Issue Disconnect request */
	INIT_COMPLETION(adapter->disconnect_comp_var);
	hal_status = sme_roam_disconnect(WLAN_HDD_GET_HAL_CTX(adapter),
					 adapter->session_id,
					 eCSR_DISCONNECT_REASON_IBSS_LEAVE);
	if (!QDF_IS_STATUS_SUCCESS(hal_status)) {
		hdd_err("sme_roam_disconnect failed status: %d",
		       hal_status);
		return -EAGAIN;
	}

	/* wait for mc thread to cleanup and then return to upper stack
	 * so by the time upper layer calls the change interface, we are
	 * all set to proceed further
	 */
	rc = wait_for_completion_timeout(&adapter->disconnect_comp_var,
			msecs_to_jiffies(WLAN_WAIT_TIME_DISCONNECT));
	if (!rc) {
		hdd_err("Failed to disconnect, timed out");
		return -ETIMEDOUT;
	}

	EXIT();
	return 0;
}

/**
 * wlan_hdd_cfg80211_leave_ibss() - leave ibss
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 *
 * This function is used to leave an IBSS network
 *
 * Return: 0 for success, non-zero for failure
 */
static int wlan_hdd_cfg80211_leave_ibss(struct wiphy *wiphy,
					struct net_device *dev)
{
	int ret = 0;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_leave_ibss(wiphy, dev);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __wlan_hdd_cfg80211_set_wiphy_params() - set wiphy parameters
 * @wiphy: Pointer to wiphy
 * @changed: Parameters changed
 *
 * This function is used to set the phy parameters. RTS Threshold/FRAG
 * Threshold/Retry Count etc.
 *
 * Return: 0 for success, non-zero for failure
 */
static int __wlan_hdd_cfg80211_set_wiphy_params(struct wiphy *wiphy,
						u32 changed)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	tHalHandle hHal = hdd_ctx->hHal;
	int status;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_SET_WIPHY_PARAMS,
			 NO_SESSION, wiphy->rts_threshold));
	status = wlan_hdd_validate_context(hdd_ctx);

	if (0 != status)
		return status;

	if (changed & WIPHY_PARAM_RTS_THRESHOLD) {
		u32 rts_threshold = (wiphy->rts_threshold == -1) ?
				    WNI_CFG_RTS_THRESHOLD_STAMAX : wiphy->rts_threshold;

		if ((WNI_CFG_RTS_THRESHOLD_STAMIN > rts_threshold) ||
		    (WNI_CFG_RTS_THRESHOLD_STAMAX < rts_threshold)) {
			hdd_err("Invalid RTS Threshold value: %u",
				rts_threshold);
			return -EINVAL;
		}

		if (0 != sme_cfg_set_int(hHal, WNI_CFG_RTS_THRESHOLD,
					rts_threshold)) {
			hdd_err("sme_cfg_set_int failed for rts_threshold value %u",
				rts_threshold);
			return -EIO;
		}

		hdd_debug("set rts threshold %u", rts_threshold);
	}

	if (changed & WIPHY_PARAM_FRAG_THRESHOLD) {
		u16 frag_threshold = (wiphy->frag_threshold == -1) ?
				     WNI_CFG_FRAGMENTATION_THRESHOLD_STAMAX :
				     wiphy->frag_threshold;

		if ((WNI_CFG_FRAGMENTATION_THRESHOLD_STAMIN > frag_threshold) ||
		    (WNI_CFG_FRAGMENTATION_THRESHOLD_STAMAX < frag_threshold)) {
			hdd_err("Invalid frag_threshold value %hu",
				frag_threshold);
			return -EINVAL;
		}

		if (0 != sme_cfg_set_int(hHal, WNI_CFG_FRAGMENTATION_THRESHOLD,
					 frag_threshold)) {
			hdd_err("sme_cfg_set_int failed for frag_threshold value %hu",
				frag_threshold);
			return -EIO;
		}

		hdd_debug("set frag threshold %hu", frag_threshold);
	}

	if ((changed & WIPHY_PARAM_RETRY_SHORT)
	    || (changed & WIPHY_PARAM_RETRY_LONG)) {
		u8 retry_value = (changed & WIPHY_PARAM_RETRY_SHORT) ?
				 wiphy->retry_short : wiphy->retry_long;

		if ((WNI_CFG_LONG_RETRY_LIMIT_STAMIN > retry_value) ||
		    (WNI_CFG_LONG_RETRY_LIMIT_STAMAX < retry_value)) {
			hdd_err("Invalid Retry count: %hu", retry_value);
			return -EINVAL;
		}

		if (changed & WIPHY_PARAM_RETRY_LONG) {
			if (0 != sme_cfg_set_int(hHal,
						WNI_CFG_LONG_RETRY_LIMIT,
						retry_value)) {
				hdd_err("sme_cfg_set_int failed for long retry count: %hu",
					retry_value);
				return -EIO;
			}
			hdd_debug("set long retry count %hu", retry_value);
		} else if (changed & WIPHY_PARAM_RETRY_SHORT) {
			if (0 != sme_cfg_set_int(hHal,
						WNI_CFG_SHORT_RETRY_LIMIT,
						retry_value)) {
				hdd_err("sme_cfg_set_int failed for short retry count: %hu",
					retry_value);
				return -EIO;
			}
			hdd_debug("set short retry count %hu", retry_value);
		}
	}
	EXIT();
	return 0;
}

/**
 * wlan_hdd_cfg80211_set_wiphy_params() - set wiphy parameters
 * @wiphy: Pointer to wiphy
 * @changed: Parameters changed
 *
 * Return: 0 for success, non-zero for failure
 */
static int wlan_hdd_cfg80211_set_wiphy_params(struct wiphy *wiphy, u32 changed)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_set_wiphy_params(wiphy, changed);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __wlan_hdd_set_default_mgmt_key() - dummy implementation of set default mgmt
 *				     key
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @key_index: Key index
 *
 * Return: 0
 */
static int __wlan_hdd_set_default_mgmt_key(struct wiphy *wiphy,
					 struct net_device *netdev,
					 u8 key_index)
{
	ENTER();
	return 0;
}

/**
 * wlan_hdd_set_default_mgmt_key() - SSR wrapper for
 *				wlan_hdd_set_default_mgmt_key
 * @wiphy: pointer to wiphy
 * @netdev: pointer to net_device structure
 * @key_index: key index
 *
 * Return: 0 on success, error number on failure
 */
static int wlan_hdd_set_default_mgmt_key(struct wiphy *wiphy,
					   struct net_device *netdev,
					   u8 key_index)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_set_default_mgmt_key(wiphy, netdev, key_index);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __wlan_hdd_set_txq_params() - dummy implementation of set tx queue params
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @params: Pointer to tx queue parameters
 *
 * Return: 0
 */
static int __wlan_hdd_set_txq_params(struct wiphy *wiphy,
				   struct net_device *dev,
				   struct ieee80211_txq_params *params)
{
	ENTER();
	return 0;
}

/**
 * wlan_hdd_set_txq_params() - SSR wrapper for wlan_hdd_set_txq_params
 * @wiphy: pointer to wiphy
 * @netdev: pointer to net_device structure
 * @params: pointer to ieee80211_txq_params
 *
 * Return: 0 on success, error number on failure
 */
static int wlan_hdd_set_txq_params(struct wiphy *wiphy,
				   struct net_device *dev,
				   struct ieee80211_txq_params *params)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_set_txq_params(wiphy, dev, params);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __wlan_hdd_cfg80211_del_station() - delete station v2
 * @wiphy: Pointer to wiphy
 * @param: Pointer to delete station parameter
 *
 * Return: 0 for success, non-zero for failure
 */
static
int __wlan_hdd_cfg80211_del_station(struct wiphy *wiphy,
				    struct net_device *dev,
				    struct csr_del_sta_params *pDelStaParams)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx;
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	struct hdd_hostapd_state *hapd_state;
	int status;
	uint8_t staId;
	uint8_t *mac;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(adapter->session_id)) {
		hdd_err("Invalid session id: %d", adapter->session_id);
		return -EINVAL;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_DEL_STA,
			 adapter->session_id, adapter->device_mode));

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	status = wlan_hdd_validate_context(hdd_ctx);

	if (0 != status)
		return status;

	mac = (uint8_t *) pDelStaParams->peerMacAddr.bytes;

	if ((QDF_SAP_MODE == adapter->device_mode) ||
	    (QDF_P2P_GO_MODE == adapter->device_mode)) {

		hapd_state = WLAN_HDD_GET_HOSTAP_STATE_PTR(adapter);
		if (!hapd_state) {
			hdd_err("Hostapd State is Null");
			return 0;
		}

		if (qdf_is_macaddr_broadcast((struct qdf_mac_addr *) mac)) {
			uint16_t i;

			for (i = 0; i < WLAN_MAX_STA_COUNT; i++) {
				if ((adapter->sta_info[i].in_use) &&
				    (!adapter->sta_info[i].
				     is_deauth_in_progress)) {
					qdf_mem_copy(
						mac,
						adapter->sta_info[i].
							sta_mac.bytes,
						QDF_MAC_ADDR_SIZE);

					hdd_debug("Delete STA with MAC::"
						  MAC_ADDRESS_STR,
					       MAC_ADDR_ARRAY(mac));

					if (hdd_ctx->dev_dfs_cac_status ==
							DFS_CAC_IN_PROGRESS)
						goto fn_end;

					qdf_event_reset(&hapd_state->qdf_sta_disassoc_event);
					hdd_softap_sta_disassoc(adapter,
								pDelStaParams);
					qdf_status =
						hdd_softap_sta_deauth(adapter,
							pDelStaParams);
					if (QDF_IS_STATUS_SUCCESS(qdf_status)) {
						adapter->sta_info[i].
						is_deauth_in_progress = true;
						qdf_status =
							qdf_wait_for_event_completion(
							 &hapd_state->
							 qdf_sta_disassoc_event,
							 SME_CMD_TIMEOUT_VALUE);
						if (!QDF_IS_STATUS_SUCCESS(
								qdf_status))
							hdd_warn("Deauth wait time expired");
					}
				}
			}
		} else {
			qdf_status =
				hdd_softap_get_sta_id(adapter,
					      (struct qdf_mac_addr *) mac,
					      &staId);
			if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
				hdd_debug("Skip DEL STA as this is not used::"
					  MAC_ADDRESS_STR,
				       MAC_ADDR_ARRAY(mac));
				return -ENOENT;
			}

			if (adapter->sta_info[staId].is_deauth_in_progress ==
			    true) {
				hdd_debug("Skip DEL STA as deauth is in progress::"
					  MAC_ADDRESS_STR,
					  MAC_ADDR_ARRAY(mac));
				return -ENOENT;
			}

			adapter->sta_info[staId].is_deauth_in_progress = true;

			hdd_debug("Delete STA with MAC::" MAC_ADDRESS_STR,
			       MAC_ADDR_ARRAY(mac));

			/* Case: SAP in ACS selected DFS ch and client connected
			 * Now Radar detected. Then if random channel is another
			 * DFS ch then new CAC is initiated and no TX allowed.
			 * So do not send any mgmt frames as it will timeout
			 * during CAC.
			 */

			if (hdd_ctx->dev_dfs_cac_status == DFS_CAC_IN_PROGRESS)
				goto fn_end;

			qdf_event_reset(&hapd_state->qdf_sta_disassoc_event);
			sme_send_disassoc_req_frame(WLAN_HDD_GET_HAL_CTX
					(adapter), adapter->session_id,
					(uint8_t *)&pDelStaParams->peerMacAddr,
					pDelStaParams->reason_code, 0);
			qdf_status = hdd_softap_sta_deauth(adapter,
							   pDelStaParams);
			if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
				adapter->sta_info[staId].is_deauth_in_progress =
					false;
				hdd_debug("STA removal failed for ::"
					  MAC_ADDRESS_STR,
				       MAC_ADDR_ARRAY(mac));
				return -ENOENT;
			}
			qdf_status = qdf_wait_for_event_completion(
					&hapd_state->
					qdf_sta_disassoc_event,
					SME_CMD_TIMEOUT_VALUE);
			if (!QDF_IS_STATUS_SUCCESS(qdf_status))
				hdd_warn("Deauth wait time expired");

			adapter->cache_sta_info[staId].reason_code =
				pDelStaParams->reason_code;
		}
	}

fn_end:
	EXIT();
	return 0;
}

#if defined(USE_CFG80211_DEL_STA_V2)
/**
 * wlan_hdd_del_station() - delete station wrapper
 * @adapter: pointer to the hdd adapter
 *
 * Return: None
 */
void wlan_hdd_del_station(struct hdd_adapter *adapter)
{
	struct station_del_parameters del_sta;

	del_sta.mac = NULL;
	del_sta.subtype = SIR_MAC_MGMT_DEAUTH >> 4;
	del_sta.reason_code = eCsrForcedDeauthSta;

	wlan_hdd_cfg80211_del_station(adapter->wdev.wiphy, adapter->dev,
				      &del_sta);
}
#else
void wlan_hdd_del_station(struct hdd_adapter *adapter)
{
	wlan_hdd_cfg80211_del_station(adapter->wdev.wiphy, adapter->dev, NULL);
}
#endif

#if defined(USE_CFG80211_DEL_STA_V2)
/**
 * wlan_hdd_cfg80211_del_station() - delete station v2
 * @wiphy: Pointer to wiphy
 * @param: Pointer to delete station parameter
 *
 * Return: 0 for success, non-zero for failure
 */
int wlan_hdd_cfg80211_del_station(struct wiphy *wiphy,
				  struct net_device *dev,
				  struct station_del_parameters *param)
#else
/**
 * wlan_hdd_cfg80211_del_station() - delete station
 * @wiphy: Pointer to wiphy
 * @mac: Pointer to station mac address
 *
 * Return: 0 for success, non-zero for failure
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
int wlan_hdd_cfg80211_del_station(struct wiphy *wiphy,
				  struct net_device *dev,
				  const uint8_t *mac)
#else
int wlan_hdd_cfg80211_del_station(struct wiphy *wiphy,
				  struct net_device *dev,
				  uint8_t *mac)
#endif
#endif
{
	int ret;
	struct csr_del_sta_params delStaParams;

	cds_ssr_protect(__func__);
#if defined(USE_CFG80211_DEL_STA_V2)
	if (NULL == param) {
		hdd_err("Invalid argument passed");
		return -EINVAL;
	}
	wlansap_populate_del_sta_params(param->mac, param->reason_code,
					param->subtype, &delStaParams);
#else
	wlansap_populate_del_sta_params(mac, eSIR_MAC_DEAUTH_LEAVING_BSS_REASON,
					(SIR_MAC_MGMT_DEAUTH >> 4),
					&delStaParams);
#endif
	ret = __wlan_hdd_cfg80211_del_station(wiphy, dev, &delStaParams);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __wlan_hdd_cfg80211_add_station() - add station
 * @wiphy: Pointer to wiphy
 * @mac: Pointer to station mac address
 * @pmksa: Pointer to add station parameter
 *
 * Return: 0 for success, non-zero for failure
 */
static int __wlan_hdd_cfg80211_add_station(struct wiphy *wiphy,
					   struct net_device *dev,
					   const uint8_t *mac,
					   struct station_parameters *params)
{
	int status = -EPERM;
#ifdef FEATURE_WLAN_TDLS
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	u32 mask, set;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(adapter->session_id)) {
		hdd_err("Invalid session id: %d", adapter->session_id);
		return -EINVAL;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_ADD_STA,
			 adapter->session_id, params->listen_interval));

	if (0 != wlan_hdd_validate_context(hdd_ctx))
		return -EINVAL;

	mask = params->sta_flags_mask;

	set = params->sta_flags_set;

	hdd_debug("mask 0x%x set 0x%x " MAC_ADDRESS_STR, mask, set,
		MAC_ADDR_ARRAY(mac));

	if (mask & BIT(NL80211_STA_FLAG_TDLS_PEER)) {
		if (set & BIT(NL80211_STA_FLAG_TDLS_PEER))
			status = wlan_cfg80211_tdls_add_peer(hdd_ctx->hdd_pdev,
							     dev, mac);
	}
#endif
	EXIT();
	return status;
}

/**
 * wlan_hdd_cfg80211_add_station() - add station
 * @wiphy: Pointer to wiphy
 * @mac: Pointer to station mac address
 * @pmksa: Pointer to add station parameter
 *
 * Return: 0 for success, non-zero for failure
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
static int wlan_hdd_cfg80211_add_station(struct wiphy *wiphy,
					 struct net_device *dev,
					 const uint8_t *mac,
					 struct station_parameters *params)
#else
static int wlan_hdd_cfg80211_add_station(struct wiphy *wiphy,
					 struct net_device *dev, uint8_t *mac,
					 struct station_parameters *params)
#endif
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_add_station(wiphy, dev, mac, params);
	cds_ssr_unprotect(__func__);

	return ret;
}

#if defined(CFG80211_FILS_SK_OFFLOAD_SUPPORT) || \
	 (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0))
/*
 * wlan_hdd_is_pmksa_valid: API to validate pmksa
 * @pmksa: pointer to cfg80211_pmksa structure
 *
 * Return: True if valid else false
 */
static inline bool wlan_hdd_is_pmksa_valid(struct cfg80211_pmksa *pmksa)
{
	if (!pmksa->bssid) {
		hdd_warn("bssid (%pK) is NULL",
					pmksa->bssid);
		if (!pmksa->ssid || !pmksa->cache_id) {
			hdd_err("either ssid (%pK) or cache_id (%pK) are NULL",
					pmksa->ssid, pmksa->cache_id);
			return false;
		}
	}
	return true;
}

/*
 * hdd_fill_pmksa_info: API to update tPmkidCacheInfo from cfg80211_pmksa
 * @pmk_cache: pmksa from supplicant
 * @pmk_cache: pmk needs to be updated
 *
 * Return: None
 */
static void hdd_fill_pmksa_info(tPmkidCacheInfo *pmk_cache,
				  struct cfg80211_pmksa *pmksa, bool is_delete)
{
	if (pmksa->bssid) {
		hdd_debug("%s PMKSA for " MAC_ADDRESS_STR,
			  is_delete ? "Delete" : "Set",
			  MAC_ADDR_ARRAY(pmksa->bssid));
		qdf_mem_copy(pmk_cache->BSSID.bytes,
			     pmksa->bssid, QDF_MAC_ADDR_SIZE);
	} else {
		qdf_mem_copy(pmk_cache->ssid, pmksa->ssid,
			     SIR_MAC_MAX_SSID_LENGTH);
		qdf_mem_copy(pmk_cache->cache_id, pmksa->cache_id,
			     CACHE_ID_LEN);
		pmk_cache->ssid_len = pmksa->ssid_len;
		hdd_debug("%s PMKSA for ssid %*.*s cache_id %x %x",
			  is_delete ? "Delete" : "Set",
			  pmk_cache->ssid_len, pmk_cache->ssid_len,
			  pmk_cache->ssid, pmk_cache->cache_id[0],
			  pmk_cache->cache_id[1]);
	}

	if (is_delete)
		return;

	qdf_mem_copy(pmk_cache->PMKID, pmksa->pmkid, CSR_RSN_PMKID_SIZE);
	if (pmksa->pmk_len && (pmksa->pmk_len <= CSR_RSN_MAX_PMK_LEN)) {
		qdf_mem_copy(pmk_cache->pmk, pmksa->pmk, pmksa->pmk_len);
		pmk_cache->pmk_len = pmksa->pmk_len;
	} else
		hdd_debug("pmk len is %zu", pmksa->pmk_len);
}
#else
/*
 * wlan_hdd_is_pmksa_valid: API to validate pmksa
 * @pmksa: pointer to cfg80211_pmksa structure
 *
 * Return: True if valid else false
 */
static inline bool wlan_hdd_is_pmksa_valid(struct cfg80211_pmksa *pmksa)
{
	if (!pmksa->bssid) {
		hdd_err("both bssid is NULL %pK", pmksa->bssid);
		return false;
	}
	return true;
}

/*
 * hdd_fill_pmksa_info: API to update tPmkidCacheInfo from cfg80211_pmksa
 * @pmk_cache: pmksa from supplicant
 * @pmk_cache: pmk needs to be updated
 *
 * Return: None
 */
static void hdd_fill_pmksa_info(tPmkidCacheInfo *pmk_cache,
				  struct cfg80211_pmksa *pmksa, bool is_delete)
{
	hdd_debug("%s PMKSA for " MAC_ADDRESS_STR, is_delete ? "Delete" : "Set",
	MAC_ADDR_ARRAY(pmksa->bssid));
	qdf_mem_copy(pmk_cache->BSSID.bytes,
				pmksa->bssid, QDF_MAC_ADDR_SIZE);

	if (is_delete)
		return;

	qdf_mem_copy(pmk_cache->PMKID, pmksa->pmkid, CSR_RSN_PMKID_SIZE);
}
#endif

/**
 * __wlan_hdd_cfg80211_set_pmksa() - set pmksa
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @pmksa: Pointer to set pmksa parameter
 *
 * Return: 0 for success, non-zero for failure
 */
static int __wlan_hdd_cfg80211_set_pmksa(struct wiphy *wiphy,
					 struct net_device *dev,
					 struct cfg80211_pmksa *pmksa)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	tHalHandle halHandle;
	QDF_STATUS result = QDF_STATUS_SUCCESS;
	int status;
	tPmkidCacheInfo pmk_cache;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(adapter->session_id)) {
		hdd_err("Invalid session id: %d", adapter->session_id);
		return -EINVAL;
	}

	if (!pmksa) {
		hdd_err("pmksa is NULL");
		return -EINVAL;
	}

	if (!pmksa->pmkid) {
		hdd_err("pmksa->pmkid(%pK) is NULL",
		       pmksa->pmkid);
		return -EINVAL;
	}

	if (!wlan_hdd_is_pmksa_valid(pmksa))
		return -EINVAL;

	status = wlan_hdd_validate_context(hdd_ctx);

	if (0 != status)
		return status;

	halHandle = WLAN_HDD_GET_HAL_CTX(adapter);

	qdf_mem_zero(&pmk_cache, sizeof(pmk_cache));

	hdd_fill_pmksa_info(&pmk_cache, pmksa, false);

	/*
	 * Add to the PMKSA Cache in CSR
	 * PMKSA cache will be having following
	 * 1. pmkid id
	 * 2. pmk
	 * 3. bssid or cache identifier
	 */
	result = sme_roam_set_pmkid_cache(halHandle, adapter->session_id,
					  &pmk_cache, 1, false);

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_SET_PMKSA,
			 adapter->session_id, result));

	sme_set_del_pmkid_cache(halHandle, adapter->session_id,
					&pmk_cache, true);

	EXIT();
	return QDF_IS_STATUS_SUCCESS(result) ? 0 : -EINVAL;
}

/**
 * wlan_hdd_cfg80211_set_pmksa() - set pmksa
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @pmksa: Pointer to set pmksa parameter
 *
 * Return: 0 for success, non-zero for failure
 */
static int wlan_hdd_cfg80211_set_pmksa(struct wiphy *wiphy,
				       struct net_device *dev,
				       struct cfg80211_pmksa *pmksa)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_set_pmksa(wiphy, dev, pmksa);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __wlan_hdd_cfg80211_del_pmksa() - delete pmksa
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @pmksa: Pointer to pmksa parameter
 *
 * Return: 0 for success, non-zero for failure
 */
static int __wlan_hdd_cfg80211_del_pmksa(struct wiphy *wiphy,
					 struct net_device *dev,
					 struct cfg80211_pmksa *pmksa)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	tHalHandle halHandle;
	int status = 0;
	tPmkidCacheInfo pmk_cache;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(adapter->session_id)) {
		hdd_err("invalid session id: %d", adapter->session_id);
		return -EINVAL;
	}

	if (!pmksa) {
		hdd_err("pmksa is NULL");
		return -EINVAL;
	}

	if (!wlan_hdd_is_pmksa_valid(pmksa))
		return -EINVAL;

	status = wlan_hdd_validate_context(hdd_ctx);

	if (0 != status)
		return status;

	halHandle = WLAN_HDD_GET_HAL_CTX(adapter);

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_DEL_PMKSA,
			 adapter->session_id, 0));

	qdf_mem_zero(&pmk_cache, sizeof(pmk_cache));

	hdd_fill_pmksa_info(&pmk_cache, pmksa, true);

	/* Delete the PMKID CSR cache */
	if (QDF_STATUS_SUCCESS !=
	    sme_roam_del_pmkid_from_cache(halHandle,
					  adapter->session_id, &pmk_cache,
					  false)) {
		hdd_err("Failed to delete PMKSA for " MAC_ADDRESS_STR,
		       MAC_ADDR_ARRAY(pmksa->bssid));
		status = -EINVAL;
	}

	sme_set_del_pmkid_cache(halHandle, adapter->session_id, &pmk_cache,
						false);
	EXIT();
	return status;
}

/**
 * wlan_hdd_cfg80211_del_pmksa() - delete pmksa
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @pmksa: Pointer to pmksa parameter
 *
 * Return: 0 for success, non-zero for failure
 */
static int wlan_hdd_cfg80211_del_pmksa(struct wiphy *wiphy,
				       struct net_device *dev,
				       struct cfg80211_pmksa *pmksa)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_del_pmksa(wiphy, dev, pmksa);
	cds_ssr_unprotect(__func__);

	return ret;

}

/**
 * __wlan_hdd_cfg80211_flush_pmksa() - flush pmksa
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 *
 * Return: 0 for success, non-zero for failure
 */
static int __wlan_hdd_cfg80211_flush_pmksa(struct wiphy *wiphy,
					   struct net_device *dev)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	tHalHandle halHandle;
	int status = 0;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(adapter->session_id)) {
		hdd_err("invalid session id: %d", adapter->session_id);
		return -EINVAL;
	}

	hdd_debug("Flushing PMKSA");

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	status = wlan_hdd_validate_context(hdd_ctx);

	if (0 != status)
		return status;

	/* Retrieve halHandle */
	halHandle = WLAN_HDD_GET_HAL_CTX(adapter);

	/* Flush the PMKID cache in CSR */
	if (QDF_STATUS_SUCCESS !=
	    sme_roam_del_pmkid_from_cache(halHandle, adapter->session_id, NULL,
					  true)) {
		hdd_err("Cannot flush PMKIDCache");
		status = -EINVAL;
	}

	sme_set_del_pmkid_cache(halHandle, adapter->session_id, NULL, false);
	EXIT();
	return status;
}

/**
 * wlan_hdd_cfg80211_flush_pmksa() - flush pmksa
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 *
 * Return: 0 for success, non-zero for failure
 */
static int wlan_hdd_cfg80211_flush_pmksa(struct wiphy *wiphy,
					 struct net_device *dev)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_flush_pmksa(wiphy, dev);
	cds_ssr_unprotect(__func__);

	return ret;
}

#if defined(KERNEL_SUPPORT_11R_CFG80211)
/**
 * __wlan_hdd_cfg80211_update_ft_ies() - update fast transition ies
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @ftie: Pointer to fast transition ie parameter
 *
 * Return: 0 for success, non-zero for failure
 */
static int
__wlan_hdd_cfg80211_update_ft_ies(struct wiphy *wiphy,
				  struct net_device *dev,
				  struct cfg80211_update_ft_ies_params *ftie)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	int status;

	ENTER();

	status = wlan_hdd_validate_context(hdd_ctx);
	if (status)
		return status;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(adapter->session_id)) {
		hdd_err("invalid session id: %d", adapter->session_id);
		return -EINVAL;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_UPDATE_FT_IES,
			 adapter->session_id, sta_ctx->conn_info.connState));
	/* Added for debug on reception of Re-assoc Req. */
	if (eConnectionState_Associated != sta_ctx->conn_info.connState) {
		hdd_err("Called with Ie of length = %zu when not associated",
		       ftie->ie_len);
		hdd_err("Should be Re-assoc Req IEs");
	}
	hdd_debug("%s called with Ie of length = %zu", __func__,
	       ftie->ie_len);

	/* Pass the received FT IEs to SME */
	sme_set_ft_ies(WLAN_HDD_GET_HAL_CTX(adapter), adapter->session_id,
		       (const u8 *)ftie->ie, ftie->ie_len);
	EXIT();
	return 0;
}

/**
 * wlan_hdd_cfg80211_update_ft_ies() - update fast transition ies
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @ftie: Pointer to fast transition ie parameter
 *
 * Return: 0 for success, non-zero for failure
 */
static int
wlan_hdd_cfg80211_update_ft_ies(struct wiphy *wiphy,
				struct net_device *dev,
				struct cfg80211_update_ft_ies_params *ftie)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_update_ft_ies(wiphy, dev, ftie);
	cds_ssr_unprotect(__func__);

	return ret;
}
#endif

void wlan_hdd_cfg80211_update_replay_counter_cb(
		void *cb_ctx, struct pmo_gtk_rsp_params *gtk_rsp_param)

{
	struct hdd_adapter *adapter = (struct hdd_adapter *)cb_ctx;
	uint8_t temp_replay_counter[8];
	int i;
	uint8_t *p;

	ENTER();

	if (!adapter) {
		hdd_err("HDD adapter is Null");
		goto out;
	}

	if (!gtk_rsp_param) {
		hdd_err("gtk_rsp_param is Null");
		goto out;
	}

	if (gtk_rsp_param->status_flag != QDF_STATUS_SUCCESS) {
		hdd_err("wlan Failed to get replay counter value");
		goto out;
	}

	hdd_debug("updated replay counter: %llu from fwr",
		gtk_rsp_param->replay_counter);
	/* convert little to big endian since supplicant works on big endian */
	p = (uint8_t *)&gtk_rsp_param->replay_counter;
	for (i = 0; i < 8; i++)
		temp_replay_counter[7 - i] = (uint8_t) p[i];

	hdd_debug("gtk_rsp_param bssid %pM", gtk_rsp_param->bssid.bytes);
	/* Update replay counter to NL */
	cfg80211_gtk_rekey_notify(adapter->dev,
					gtk_rsp_param->bssid.bytes,
					temp_replay_counter, GFP_KERNEL);
out:
	EXIT();

}

/**
 * wlan_hdd_copy_gtk_kek - Copy the KEK from GTK rekey data to GTK request
 * @gtk_req: Pointer to GTK request
 * @data: Pointer to rekey data
 *
 * Return: none
 */
#ifdef CFG80211_REKEY_DATA_KEK_LEN
static
void wlan_hdd_copy_gtk_kek(struct pmo_gtk_req *gtk_req,
			   struct cfg80211_gtk_rekey_data *data)
{
	qdf_mem_copy(gtk_req->kek, data->kek, data->kek_len);
	gtk_req->kek_len = data->kek_len;
}
#else
static
void wlan_hdd_copy_gtk_kek(struct pmo_gtk_req *gtk_req,
			   struct cfg80211_gtk_rekey_data *data)
{
	qdf_mem_copy(gtk_req->kek, data->kek, NL80211_KEK_LEN);
	gtk_req->kek_len = NL80211_KEK_LEN;
}
#endif

/**
 * __wlan_hdd_cfg80211_set_rekey_data() - set rekey data
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @data: Pointer to rekey data
 *
 * This function is used to offload GTK rekeying job to the firmware.
 *
 * Return: 0 for success, non-zero for failure
 */
static
int __wlan_hdd_cfg80211_set_rekey_data(struct wiphy *wiphy,
		struct net_device *dev,
		struct cfg80211_gtk_rekey_data *data)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	int result, i;
	struct pmo_gtk_req *gtk_req = NULL;
	struct hdd_context *hdd_ctx =  WLAN_HDD_GET_CTX(adapter);
	uint8_t *buf;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		result = -EINVAL;
		goto out;
	}

	if (wlan_hdd_validate_session_id(adapter->session_id)) {
		hdd_err("invalid session id: %d", adapter->session_id);
		result = -EINVAL;
		goto out;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_SET_REKEY_DATA,
			 adapter->session_id, adapter->device_mode));

	result = wlan_hdd_validate_context(hdd_ctx);
	if (0 != result)
		goto out;

	gtk_req = qdf_mem_malloc(sizeof(*gtk_req));
	if (!gtk_req) {
		hdd_err("cannot allocate gtk_req");
		result = -ENOMEM;
		goto out;
	}

	/* convert big to little endian since driver work on little endian */
	buf = (uint8_t *)&gtk_req->replay_counter;
	for (i = 0; i < 8; i++)
		buf[7 - i] = data->replay_ctr[i];

	hdd_debug("current replay counter: %llu in user space",
		gtk_req->replay_counter);

	wlan_hdd_copy_gtk_kek(gtk_req, data);
	qdf_mem_copy(gtk_req->kck, data->kck, NL80211_KCK_LEN);
	status = pmo_ucfg_cache_gtk_offload_req(adapter->hdd_vdev, gtk_req);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("Failed to cache GTK Offload");
		result = qdf_status_to_os_return(status);
	}
out:
	if (gtk_req)
		qdf_mem_free(gtk_req);
	EXIT();

	return result;
}

/**
 * wlan_hdd_cfg80211_set_rekey_data() - set rekey data
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @data: Pointer to rekey data
 *
 * This function is used to offload GTK rekeying job to the firmware.
 *
 * Return: 0 for success, non-zero for failure
 */
static
int wlan_hdd_cfg80211_set_rekey_data(struct wiphy *wiphy,
				     struct net_device *dev,
				     struct cfg80211_gtk_rekey_data *data)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_set_rekey_data(wiphy, dev, data);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __wlan_hdd_cfg80211_set_mac_acl() - set access control policy
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @param: Pointer to access control parameter
 *
 * Return: 0 for success, non-zero for failure
 */
static int __wlan_hdd_cfg80211_set_mac_acl(struct wiphy *wiphy,
					 struct net_device *dev,
					 const struct cfg80211_acl_data *params)
{
	int i;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_hostapd_state *hostapd_state;
	tsap_Config_t *pConfig;
	struct hdd_context *hdd_ctx;
	int status;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (NULL == params) {
		hdd_err("params is Null");
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	status = wlan_hdd_validate_context(hdd_ctx);

	if (0 != status)
		return status;

	hostapd_state = WLAN_HDD_GET_HOSTAP_STATE_PTR(adapter);

	if (NULL == hostapd_state) {
		hdd_err("hostapd_state is Null");
		return -EINVAL;
	}

	hdd_debug("acl policy: %d num acl entries: %d", params->acl_policy,
		params->n_acl_entries);

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_SET_MAC_ACL,
			 adapter->session_id, adapter->device_mode));
	if (QDF_SAP_MODE == adapter->device_mode) {
		pConfig = &adapter->session.ap.sap_config;

		/* default value */
		pConfig->num_accept_mac = 0;
		pConfig->num_deny_mac = 0;

		/**
		 * access control policy
		 * @NL80211_ACL_POLICY_ACCEPT_UNLESS_LISTED: Deny stations which are
		 *   listed in hostapd.deny file.
		 * @NL80211_ACL_POLICY_DENY_UNLESS_LISTED: Allow stations which are
		 *   listed in hostapd.accept file.
		 */
		if (NL80211_ACL_POLICY_DENY_UNLESS_LISTED == params->acl_policy) {
			pConfig->SapMacaddr_acl = eSAP_DENY_UNLESS_ACCEPTED;
		} else if (NL80211_ACL_POLICY_ACCEPT_UNLESS_LISTED ==
			   params->acl_policy) {
			pConfig->SapMacaddr_acl = eSAP_ACCEPT_UNLESS_DENIED;
		} else {
			hdd_warn("Acl Policy : %d is not supported",
				params->acl_policy);
			return -ENOTSUPP;
		}

		if (eSAP_DENY_UNLESS_ACCEPTED == pConfig->SapMacaddr_acl) {
			pConfig->num_accept_mac = params->n_acl_entries;
			for (i = 0; i < params->n_acl_entries; i++) {
				hdd_debug("** Add ACL MAC entry %i in WhiletList :"
					MAC_ADDRESS_STR, i,
					MAC_ADDR_ARRAY(
						params->mac_addrs[i].addr));

				qdf_mem_copy(&pConfig->accept_mac[i],
					     params->mac_addrs[i].addr,
					     sizeof(qcmacaddr));
			}
		} else if (eSAP_ACCEPT_UNLESS_DENIED == pConfig->SapMacaddr_acl) {
			pConfig->num_deny_mac = params->n_acl_entries;
			for (i = 0; i < params->n_acl_entries; i++) {
				hdd_debug("** Add ACL MAC entry %i in BlackList :"
					MAC_ADDRESS_STR, i,
					MAC_ADDR_ARRAY(
						params->mac_addrs[i].addr));

				qdf_mem_copy(&pConfig->deny_mac[i],
					     params->mac_addrs[i].addr,
					     sizeof(qcmacaddr));
			}
		}
		qdf_status = wlansap_set_mac_acl(
			WLAN_HDD_GET_SAP_CTX_PTR(adapter), pConfig);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			hdd_err("SAP Set Mac Acl fail");
			return -EINVAL;
		}
	} else {
		hdd_debug("Invalid device_mode %s(%d)",
			hdd_device_mode_to_string(adapter->device_mode),
			adapter->device_mode);
		return -EINVAL;
	}
	EXIT();
	return 0;
}

/**
 * wlan_hdd_cfg80211_set_mac_acl() - SSR wrapper for
 *				__wlan_hdd_cfg80211_set_mac_acl
 * @wiphy: pointer to wiphy structure
 * @dev: pointer to net_device
 * @params: pointer to cfg80211_acl_data
 *
 * Return; 0 on success, error number otherwise
 */
static int
wlan_hdd_cfg80211_set_mac_acl(struct wiphy *wiphy,
			      struct net_device *dev,
			      const struct cfg80211_acl_data *params)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_set_mac_acl(wiphy, dev, params);
	cds_ssr_unprotect(__func__);

	return ret;
}

#ifdef WLAN_NL80211_TESTMODE
#ifdef FEATURE_WLAN_LPHB
/**
 * wlan_hdd_cfg80211_lphb_ind_handler() - handle low power heart beat indication
 * @hdd_ctx: Pointer to hdd context
 * @lphbInd: Pointer to low power heart beat indication parameter
 *
 * Return: none
 */
static void wlan_hdd_cfg80211_lphb_ind_handler(void *hdd_ctx,
		struct pmo_lphb_rsp *lphb_ind)
{
	struct sk_buff *skb;

	hdd_debug("LPHB indication arrived");

	if (0 != wlan_hdd_validate_context((struct hdd_context *) hdd_ctx))
		return;

	if (!lphb_ind) {
		hdd_err("invalid argument lphbInd");
		return;
	}

	skb = cfg80211_testmode_alloc_event_skb(((struct hdd_context *) hdd_ctx)->
			wiphy, sizeof(*lphb_ind), GFP_ATOMIC);
	if (!skb) {
		hdd_err("LPHB timeout, NL buffer alloc fail");
		return;
	}

	if (nla_put_u32(skb, WLAN_HDD_TM_ATTR_CMD, WLAN_HDD_TM_CMD_WLAN_HB)) {
		hdd_err("WLAN_HDD_TM_ATTR_CMD put fail");
		goto nla_put_failure;
	}
	if (nla_put_u32(skb, WLAN_HDD_TM_ATTR_TYPE, lphb_ind->protocol_type)) {
		hdd_err("WLAN_HDD_TM_ATTR_TYPE put fail");
		goto nla_put_failure;
	}
	if (nla_put(skb, WLAN_HDD_TM_ATTR_DATA, sizeof(*lphb_ind),
			lphb_ind)) {
		hdd_err("WLAN_HDD_TM_ATTR_DATA put fail");
		goto nla_put_failure;
	}
	cfg80211_testmode_event(skb, GFP_ATOMIC);
	return;

nla_put_failure:
	hdd_err("NLA Put fail");
	kfree_skb(skb);
}
#endif /* FEATURE_WLAN_LPHB */

/**
 * __wlan_hdd_cfg80211_testmode() - test mode
 * @wiphy: Pointer to wiphy
 * @data: Data pointer
 * @len: Data length
 *
 * Return: 0 for success, non-zero for failure
 */
static int __wlan_hdd_cfg80211_testmode(struct wiphy *wiphy,
					void *data, int len)
{
	struct nlattr *tb[WLAN_HDD_TM_ATTR_MAX + 1];
	int err;
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);

	ENTER();

	err = wlan_hdd_validate_context(hdd_ctx);
	if (err)
		return err;

	err = wlan_cfg80211_nla_parse(tb, WLAN_HDD_TM_ATTR_MAX, data,
				      len, wlan_hdd_tm_policy);
	if (err) {
		hdd_err("Testmode INV ATTR");
		return err;
	}

	if (!tb[WLAN_HDD_TM_ATTR_CMD]) {
		hdd_err("Testmode INV CMD");
		return -EINVAL;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_TESTMODE,
			 NO_SESSION, nla_get_u32(tb[WLAN_HDD_TM_ATTR_CMD])));
	switch (nla_get_u32(tb[WLAN_HDD_TM_ATTR_CMD])) {
#ifdef FEATURE_WLAN_LPHB
	/* Low Power Heartbeat configuration request */
	case WLAN_HDD_TM_CMD_WLAN_HB:
	{
		int buf_len;
		void *buf;
		struct pmo_lphb_req *hb_params = NULL;
		struct pmo_lphb_req *hb_params_temp = NULL;
		QDF_STATUS status;

		if (!tb[WLAN_HDD_TM_ATTR_DATA]) {
			hdd_err("Testmode INV DATA");
			return -EINVAL;
		}

		buf = nla_data(tb[WLAN_HDD_TM_ATTR_DATA]);
		buf_len = nla_len(tb[WLAN_HDD_TM_ATTR_DATA]);

		hb_params_temp = (struct pmo_lphb_req *) buf;
		if ((hb_params_temp->cmd == pmo_lphb_set_tcp_pararm_indid)
		    && (hb_params_temp->params.lphb_tcp_params.
			time_period_sec == 0))
			return -EINVAL;

		if (buf_len > sizeof(*hb_params)) {
			hdd_err("buf_len=%d exceeded hb_params size limit",
				buf_len);
			return -ERANGE;
		}

		hb_params = (struct pmo_lphb_req *)qdf_mem_malloc(
				sizeof(*hb_params));
		if (NULL == hb_params) {
			hdd_err("Request Buffer Alloc Fail");
			return -ENOMEM;
		}

		qdf_mem_zero(hb_params, sizeof(*hb_params));
		qdf_mem_copy(hb_params, buf, buf_len);
		status = pmo_ucfg_lphb_config_req(hdd_ctx->hdd_psoc,
					hb_params, (void *)hdd_ctx,
					    wlan_hdd_cfg80211_lphb_ind_handler);
		if (status != QDF_STATUS_SUCCESS)
			hdd_err("LPHB Config Fail, disable");

		qdf_mem_free(hb_params);
		return 0;
	}
#endif /* FEATURE_WLAN_LPHB */

#if  defined(QCA_WIFI_FTM)
	case WLAN_HDD_TM_CMD_WLAN_FTM:
	{
		err = wlan_cfg80211_ftm_testmode_cmd(hdd_ctx->hdd_pdev,
						     data, len);
		break;
	}
#endif
	default:
		hdd_err("command: %d not supported",
			nla_get_u32(tb[WLAN_HDD_TM_ATTR_CMD]));
		return -EOPNOTSUPP;
	}

	EXIT();
	return err;
}

/**
 * wlan_hdd_cfg80211_testmode() - test mode
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @data: Data pointer
 * @len: Data length
 *
 * Return: 0 for success, non-zero for failure
 */
static int wlan_hdd_cfg80211_testmode(struct wiphy *wiphy,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 12, 0))
				      struct wireless_dev *wdev,
#endif
				      void *data, int len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_testmode(wiphy, data, len);
	cds_ssr_unprotect(__func__);

	return ret;
}

#endif /* CONFIG_NL80211_TESTMODE */

#ifdef QCA_HT_2040_COEX
/**
 * __wlan_hdd_cfg80211_set_ap_channel_width() - set ap channel bandwidth
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @chandef: Pointer to channel definition parameter
 *
 * Return: 0 for success, non-zero for failure
 */
static int
__wlan_hdd_cfg80211_set_ap_channel_width(struct wiphy *wiphy,
					 struct net_device *dev,
					 struct cfg80211_chan_def *chandef)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx;
	QDF_STATUS status;
	int retval = 0;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(adapter->session_id)) {
		hdd_err("invalid session id: %d", adapter->session_id);
		return -EINVAL;
	}
	if (!(adapter->device_mode == QDF_SAP_MODE ||
	      adapter->device_mode == QDF_P2P_GO_MODE))
		return -EOPNOTSUPP;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	status = wlan_hdd_validate_context(hdd_ctx);
	if (status)
		return status;

	hdd_debug("Channel width changed to %d ",
		  cfg80211_get_chandef_type(chandef));

	/* Change SAP ht2040 mode */
	status = hdd_set_sap_ht2040_mode(adapter,
					 cfg80211_get_chandef_type(chandef));
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("Cannot set SAP HT20/40 mode!");
		retval = -EINVAL;
	}

	return retval;
}

/**
 * wlan_hdd_cfg80211_set_ap_channel_width() - set ap channel bandwidth
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @chandef: Pointer to channel definition parameter
 *
 * Return: 0 for success, non-zero for failure
 */
static int
wlan_hdd_cfg80211_set_ap_channel_width(struct wiphy *wiphy,
				       struct net_device *dev,
				       struct cfg80211_chan_def *chandef)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_set_ap_channel_width(wiphy, dev, chandef);
	cds_ssr_unprotect(__func__);

	return ret;
}
#endif

#ifdef CHANNEL_SWITCH_SUPPORTED
/**
 * __wlan_hdd_cfg80211_channel_switch()- function to switch
 * channel in SAP/GO
 * @wiphy:  wiphy pointer
 * @dev: dev pointer.
 * @csa_params: Change channel params
 *
 * This function is called to switch channel in SAP/GO
 *
 * Return: 0 if success else return non zero
 */
static int __wlan_hdd_cfg80211_channel_switch(struct wiphy *wiphy,
				struct net_device *dev,
				struct cfg80211_csa_settings *csa_params)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx;
	uint8_t channel;
	uint16_t freq;
	int ret;
	enum phy_ch_width ch_width;

	hdd_debug("Set Freq %d",
		  csa_params->chandef.chan->center_freq);

	if (wlan_hdd_validate_session_id(adapter->session_id)) {
		hdd_err("invalid session id: %d", adapter->session_id);
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);

	if (0 != ret)
		return ret;

	if ((QDF_P2P_GO_MODE != adapter->device_mode) &&
		(QDF_SAP_MODE != adapter->device_mode))
		return -ENOTSUPP;

	freq = csa_params->chandef.chan->center_freq;
	channel = cds_freq_to_chan(freq);

	ch_width = hdd_map_nl_chan_width(csa_params->chandef.width);

	ret = hdd_softap_set_channel_change(dev, channel, ch_width, false);
	return ret;
}

/**
 * wlan_hdd_cfg80211_channel_switch()- function to switch
 * channel in SAP/GO
 * @wiphy:  wiphy pointer
 * @dev: dev pointer.
 * @csa_params: Change channel params
 *
 * This function is called to switch channel in SAP/GO
 *
 * Return: 0 if success else return non zero
 */
static int wlan_hdd_cfg80211_channel_switch(struct wiphy *wiphy,
				struct net_device *dev,
				struct cfg80211_csa_settings *csa_params)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_channel_switch(wiphy, dev, csa_params);
	cds_ssr_unprotect(__func__);
	return ret;
}
#endif

/**
 * wlan_hdd_convert_nl_iftype_to_hdd_type() - provides the type
 * translation from NL to policy manager type
 * @type: Generic connection mode type defined in NL
 *
 *
 * This function provides the type translation
 *
 * Return: cds_con_mode enum
 */
enum policy_mgr_con_mode wlan_hdd_convert_nl_iftype_to_hdd_type(
						enum nl80211_iftype type)
{
	enum policy_mgr_con_mode mode = PM_MAX_NUM_OF_MODE;

	switch (type) {
	case NL80211_IFTYPE_STATION:
		mode = PM_STA_MODE;
		break;
	case NL80211_IFTYPE_P2P_CLIENT:
		mode = PM_P2P_CLIENT_MODE;
		break;
	case NL80211_IFTYPE_P2P_GO:
		mode = PM_P2P_GO_MODE;
		break;
	case NL80211_IFTYPE_AP:
		mode = PM_SAP_MODE;
		break;
	case NL80211_IFTYPE_ADHOC:
		mode = PM_IBSS_MODE;
		break;
	default:
		hdd_err("Unsupported interface type: %d", type);
	}
	return mode;
}

int wlan_hdd_change_hw_mode_for_given_chnl(struct hdd_adapter *adapter,
			uint8_t channel,
			enum policy_mgr_conn_update_reason reason)
{
	QDF_STATUS status;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	ENTER();
	if (0 != wlan_hdd_validate_context(hdd_ctx))
		return -EINVAL;

	status = policy_mgr_reset_connection_update(hdd_ctx->hdd_psoc);
	if (!QDF_IS_STATUS_SUCCESS(status))
		hdd_err("clearing event failed");

	status = policy_mgr_current_connections_update(hdd_ctx->hdd_psoc,
			adapter->session_id, channel, reason);
	switch (status) {
	case QDF_STATUS_E_FAILURE:
		/*
		 * QDF_STATUS_E_FAILURE indicates that some error has occured
		 * while changing the hw mode
		 */
		hdd_err("ERROR: connections update failed!!");
		return -EINVAL;

	case QDF_STATUS_SUCCESS:
		/*
		 * QDF_STATUS_SUCCESS indicates that HW mode change has been
		 * triggered and wait for it to finish.
		 */
		status = policy_mgr_wait_for_connection_update(
						hdd_ctx->hdd_psoc);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			hdd_err("ERROR: qdf wait for event failed!!");
			return -EINVAL;
		}
		if (QDF_MONITOR_MODE == adapter->device_mode)
			hdd_info("Monitor mode:channel:%d (SMM->DBS)", channel);
		break;

	default:
		/*
		 * QDF_STATUS_E_NOSUPPORT indicates that no HW mode change is
		 * required, so caller can proceed further.
		 */
		break;

	}
	EXIT();

	return 0;
}

/**
 * wlan_hdd_cfg80211_set_mon_ch() - Set monitor mode capture channel
 * @wiphy: Handle to struct wiphy to get handle to module context.
 * @chandef: Contains information about the capture channel to be set.
 *
 * This interface is called if and only if monitor mode interface alone is
 * active.
 *
 * Return: 0 success or error code on failure.
 */
static int __wlan_hdd_cfg80211_set_mon_ch(struct wiphy *wiphy,
				       struct cfg80211_chan_def *chandef)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct hdd_adapter *adapter;
	struct hdd_station_ctx *sta_ctx;
	struct hdd_mon_set_ch_info *ch_info;
	QDF_STATUS status;
	tHalHandle hal_hdl;
	struct qdf_mac_addr bssid;
	tCsrRoamProfile roam_profile;
	struct ch_params ch_params;
	uint8_t sec_ch = 0;
	int ret;
	uint16_t chan_num = cds_freq_to_chan(chandef->chan->center_freq);

	ENTER();

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	hal_hdl = hdd_ctx->hHal;

	adapter = hdd_get_adapter(hdd_ctx, QDF_MONITOR_MODE);
	if (!adapter)
		return -EIO;

	hdd_debug("%s: set monitor mode Channel %d and freq %d",
		 adapter->dev->name, chan_num, chandef->chan->center_freq);

	sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	ch_info = &sta_ctx->ch_info;
	roam_profile.ChannelInfo.ChannelList = &ch_info->channel;
	roam_profile.ChannelInfo.numOfChannels = 1;
	roam_profile.phyMode = ch_info->phy_mode;
	roam_profile.ch_params.ch_width = hdd_map_nl_chan_width(chandef->width);
	hdd_select_cbmode(adapter, chan_num, &roam_profile.ch_params);

	qdf_mem_copy(bssid.bytes, adapter->mac_addr.bytes,
		     QDF_MAC_ADDR_SIZE);

	ch_params.ch_width = hdd_map_nl_chan_width(chandef->width);
	/*
	 * CDS api expects secondary channel for calculating
	 * the channel params
	 */
	if ((ch_params.ch_width == CH_WIDTH_40MHZ) &&
	    (WLAN_REG_IS_24GHZ_CH(chan_num))) {
		if (chan_num >= 1 && chan_num <= 5)
			sec_ch = chan_num + 4;
		else if (chan_num >= 6 && chan_num <= 13)
			sec_ch = chan_num - 4;
	}
	wlan_reg_set_channel_params(hdd_ctx->hdd_pdev, chan_num,
			sec_ch, &ch_params);
	if (wlan_hdd_change_hw_mode_for_given_chnl(adapter, chan_num,
				POLICY_MGR_UPDATE_REASON_SET_OPER_CHAN)) {
		hdd_err("Failed to change hw mode");
		return -EINVAL;
	}
	status = sme_roam_channel_change_req(hal_hdl, bssid, &ch_params,
						 &roam_profile);
	if (status) {
		hdd_err("Failed to set sme_RoamChannel for monitor mode status: %d",
			status);
		ret = qdf_status_to_os_return(status);
		return ret;
	}
	EXIT();

	return 0;
}

/**
 * wlan_hdd_cfg80211_set_mon_ch() - Set monitor mode capture channel
 * @wiphy: Handle to struct wiphy to get handle to module context.
 * @chandef: Contains information about the capture channel to be set.
 *
 * This interface is called if and only if monitor mode interface alone is
 * active.
 *
 * Return: 0 success or error code on failure.
 */
static int wlan_hdd_cfg80211_set_mon_ch(struct wiphy *wiphy,
				       struct cfg80211_chan_def *chandef)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_set_mon_ch(wiphy, chandef);
	cds_ssr_unprotect(__func__);
	return ret;
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
	tHalHandle hal = WLAN_HDD_GET_HAL_CTX(adapter);

	link_layer_stats_clear_req.statsClearReqMask = WIFI_STATS_IFACE_AC |
		WIFI_STATS_IFACE_ALL_PEER;
	link_layer_stats_clear_req.stopReq = 0;
	link_layer_stats_clear_req.reqId = 1;
	link_layer_stats_clear_req.staId = adapter->session_id;
	sme_ll_stats_clear_req(hal, &link_layer_stats_clear_req);
}

#define CNT_DIFF(cur, prev) \
	((cur >= prev) ? (cur - prev) : (cur + (MAX_COUNT - (prev) + 1)))
#define MAX_COUNT 0xffffffff
static void hdd_update_chan_info(struct hdd_context *hdd_ctx,
			struct scan_chan_info *chan,
			struct scan_chan_info *info, uint32_t cmd_flag)
{
	if ((info->cmd_flag != WMI_CHAN_InFO_START_RESP) &&
	   (info->cmd_flag != WMI_CHAN_InFO_END_RESP))
		hdd_err("cmd flag is invalid: %d", info->cmd_flag);

	mutex_lock(&hdd_ctx->chan_info_lock);

	if (info->cmd_flag == WMI_CHAN_InFO_START_RESP)
		qdf_mem_zero(chan, sizeof(*chan));

	chan->freq = info->freq;
	chan->noise_floor = info->noise_floor;
	chan->clock_freq = info->clock_freq;
	chan->cmd_flag = info->cmd_flag;
	chan->cycle_count = CNT_DIFF(info->cycle_count, chan->cycle_count);

	chan->rx_clear_count =
			CNT_DIFF(info->rx_clear_count, chan->rx_clear_count);

	chan->tx_frame_count =
			CNT_DIFF(info->tx_frame_count, chan->tx_frame_count);

	mutex_unlock(&hdd_ctx->chan_info_lock);

}
#undef CNT_DIFF
#undef MAX_COUNT

#if defined(WLAN_FEATURE_FILS_SK) &&\
	defined(CFG80211_FILS_SK_OFFLOAD_SUPPORT) &&\
	(defined(CFG80211_UPDATE_CONNECT_PARAMS) ||\
		(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 10, 0)))

#ifndef UPDATE_FILS_ERP_INFO
#define UPDATE_FILS_ERP_INFO BIT(1)
#endif

#ifndef UPDATE_FILS_AUTH_TYPE
#define UPDATE_FILS_AUTH_TYPE BIT(2)
#endif

/**
 * __wlan_hdd_cfg80211_update_connect_params - update connect params
 * @wiphy: Handle to struct wiphy to get handle to module context.
 * @dev: Pointer to network device
 * @req: Pointer to connect params
 * @changed: Bitmap used to indicate the changed params
 *
 * Update the connect parameters while connected to a BSS. The updated
 * parameters can be used by driver/firmware for subsequent BSS selection
 * (roaming) decisions and to form the Authentication/(Re)Association
 * Request frames. This call does not request an immediate disassociation
 * or reassociation with the current BSS, i.e., this impacts only
 * subsequent (re)associations. The bits in changed are defined in enum
 * cfg80211_connect_params_changed
 *
 * Return: zero for success, non-zero for failure
 */
static int __wlan_hdd_cfg80211_update_connect_params(
			struct wiphy *wiphy, struct net_device *dev,
			struct cfg80211_connect_params *req, uint32_t changed)
{
	struct hdd_wext_state *wext_state;
	tCsrRoamProfile *roam_profile;
	uint8_t *buf;
	int ret;
	enum eAniAuthType auth_type;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	QDF_STATUS status;
	struct cds_fils_connection_info *fils_info;

	ENTER_DEV(dev);

	if (wlan_hdd_validate_session_id(adapter->session_id)) {
		hdd_err("invalid session id: %d", adapter->session_id);
		return -EINVAL;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return -EINVAL;

	wext_state = WLAN_HDD_GET_WEXT_STATE_PTR(adapter);
	roam_profile = &wext_state->roamProfile;
	fils_info = roam_profile->fils_con_info;

	if (!fils_info) {
		hdd_err("No valid FILS conn info");
		return -EINVAL;
	}

	if (req->ie_len)
		wlan_hdd_cfg80211_set_ie(adapter, req->ie, req->ie_len);

	if (changed)
		fils_info->is_fils_connection = true;

	if (changed & UPDATE_FILS_ERP_INFO) {
		if (!wlan_hdd_fils_data_in_limits(req))
			return -EINVAL;
		fils_info->key_nai_length = req->fils_erp_username_len +
					    sizeof(char) +
					    req->fils_erp_realm_len;
		if (req->fils_erp_username_len && req->fils_erp_username) {
			buf = fils_info->keyname_nai;
			qdf_mem_copy(buf, req->fils_erp_username,
					req->fils_erp_username_len);
			buf += req->fils_erp_username_len;
			*buf++ = '@';
			qdf_mem_copy(buf, req->fils_erp_realm,
					req->fils_erp_realm_len);
		}

		fils_info->sequence_number = req->fils_erp_next_seq_num;
		fils_info->r_rk_length = req->fils_erp_rrk_len;

		if (req->fils_erp_rrk_len && req->fils_erp_rrk)
			qdf_mem_copy(fils_info->r_rk, req->fils_erp_rrk,
						fils_info->r_rk_length);

		fils_info->realm_len = req->fils_erp_realm_len;
		if (req->fils_erp_realm_len && req->fils_erp_realm)
			qdf_mem_copy(fils_info->realm, req->fils_erp_realm,
						fils_info->realm_len);
	}

	if (changed & UPDATE_FILS_AUTH_TYPE) {
		auth_type = wlan_hdd_get_fils_auth_type(req->auth_type);
		if (auth_type == eSIR_DONOT_USE_AUTH_TYPE) {
			hdd_err("invalid auth type for fils %d",
				req->auth_type);
			return -EINVAL;
		}

		roam_profile->fils_con_info->auth_type = auth_type;
	}

	hdd_debug("fils conn update: changed %x is_fils %d keyname nai len %d",
		  changed, roam_profile->fils_con_info->is_fils_connection,
		  roam_profile->fils_con_info->key_nai_length);

	if (!hdd_ctx->is_fils_roaming_supported) {
		hdd_debug("FILS roaming support %d",
			  hdd_ctx->is_fils_roaming_supported);
		return 0;
	}

	status = sme_update_fils_config(hdd_ctx->hHal, adapter->session_id,
			roam_profile);
	if (QDF_IS_STATUS_ERROR(status))
		hdd_err("Update FILS connect params to Fw failed %d", status);

	return 0;
}

/**
 * wlan_hdd_cfg80211_update_connect_params - SSR wrapper for
 *                __wlan_hdd_cfg80211_update_connect_params
 * @wiphy: Pointer to wiphy structure
 * @dev: Pointer to net_device
 * @req: Pointer to connect params
 * @changed: flags used to indicate the changed params
 *
 * Return: zero for success, non-zero for failure
 */
static int wlan_hdd_cfg80211_update_connect_params(struct wiphy *wiphy,
					    struct net_device *dev,
					    struct cfg80211_connect_params *req,
					    uint32_t changed)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_update_connect_params(wiphy, dev,
							req, changed);
	cds_ssr_unprotect(__func__);

	return ret;
}
#endif

/**
 * wlan_hdd_chan_info_cb() - channel info callback
 * @chan_info: struct scan_chan_info
 *
 * Store channel info into HDD context
 *
 * Return: None.
 */
static void wlan_hdd_chan_info_cb(struct scan_chan_info *info)
{
	struct hdd_context *hdd_ctx;
	struct scan_chan_info *chan;
	uint8_t idx;

	ENTER();

	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	if (wlan_hdd_validate_context(hdd_ctx) != 0) {
		hdd_err("hdd_ctx is invalid");
		return;
	}

	if (!hdd_ctx->chan_info) {
		hdd_err("chan_info is NULL");
		return;
	}

	chan = hdd_ctx->chan_info;
	for (idx = 0; idx < SIR_MAX_NUM_CHANNELS; idx++) {
		if (chan[idx].freq == info->freq) {
			hdd_update_chan_info(hdd_ctx, &chan[idx], info,
				info->cmd_flag);
			hdd_info("cmd:%d freq:%u nf:%d cc:%u rcc:%u clk:%u cmd:%d tfc:%d index:%d",
				chan[idx].cmd_flag, chan[idx].freq,
				chan[idx].noise_floor,
				chan[idx].cycle_count, chan[idx].rx_clear_count,
				chan[idx].clock_freq, chan[idx].cmd_flag,
				chan[idx].tx_frame_count, idx);
			if (chan[idx].freq == 0)
				break;

		}
	}

	EXIT();
}

/**
 * wlan_hdd_init_chan_info() - init chan info in hdd context
 * @hdd_ctx: HDD context pointer
 *
 * Return: none
 */
void wlan_hdd_init_chan_info(struct hdd_context *hdd_ctx)
{
	uint8_t num_2g, num_5g, index = 0;

	hdd_ctx->chan_info = NULL;
	if (!hdd_ctx->config->fEnableSNRMonitoring) {
		hdd_info("SNR monitoring is disabled");
		return;
	}

	hdd_ctx->chan_info =
		qdf_mem_malloc(sizeof(struct scan_chan_info)
					* QDF_MAX_NUM_CHAN);
	if (hdd_ctx->chan_info == NULL) {
		hdd_err("Failed to malloc for chan info");
		return;
	}
	mutex_init(&hdd_ctx->chan_info_lock);

	num_2g = QDF_ARRAY_SIZE(hdd_channels_2_4_ghz);
	for (; index < num_2g; index++) {
		hdd_ctx->chan_info[index].freq =
			hdd_channels_2_4_ghz[index].center_freq;
	}

	num_5g = QDF_ARRAY_SIZE(hdd_channels_5_ghz);
	for (; (index - num_2g) < num_5g; index++) {
		if (WLAN_REG_IS_11P_CH(
			    hdd_channels_5_ghz[index - num_2g].hw_value))
			continue;
		hdd_ctx->chan_info[index].freq =
			hdd_channels_5_ghz[index - num_2g].center_freq;
	}
	sme_set_chan_info_callback(hdd_ctx->hHal,
				   &wlan_hdd_chan_info_cb);
}

/**
 * wlan_hdd_deinit_chan_info() - deinit chan info in hdd context
 * @hdd_ctx: hdd context pointer
 *
 * Return: none
 */
void wlan_hdd_deinit_chan_info(struct hdd_context *hdd_ctx)
{
	struct scan_chan_info *chan;

	chan = hdd_ctx->chan_info;
	hdd_ctx->chan_info = NULL;
	if (chan)
		qdf_mem_free(chan);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)) || defined(WITH_BACKPORTS)
static enum rate_info_bw hdd_map_hdd_bw_to_os(enum hdd_rate_info_bw hdd_bw)
{
	switch (hdd_bw) {
	case HDD_RATE_BW_5:
		return RATE_INFO_BW_5;
	case HDD_RATE_BW_10:
		return RATE_INFO_BW_10;
	case HDD_RATE_BW_20:
		return RATE_INFO_BW_20;
	case HDD_RATE_BW_40:
		return RATE_INFO_BW_40;
	case HDD_RATE_BW_80:
		return RATE_INFO_BW_80;
	case HDD_RATE_BW_160:
		return RATE_INFO_BW_160;
	}

	hdd_err("Unhandled HDD_RATE_BW: %d", hdd_bw);

	return RATE_INFO_BW_20;
}

void hdd_set_rate_bw(struct rate_info *info, enum hdd_rate_info_bw hdd_bw)
{
	info->bw = hdd_map_hdd_bw_to_os(hdd_bw);
}
#else
static enum rate_info_flags hdd_map_hdd_bw_to_os(enum hdd_rate_info_bw hdd_bw)
{
	switch (hdd_bw) {
	case HDD_RATE_BW_5:
	case HDD_RATE_BW_10:
	case HDD_RATE_BW_20:
		return (enum rate_info_flags)0;
	case HDD_RATE_BW_40:
		return RATE_INFO_FLAGS_40_MHZ_WIDTH;
	case HDD_RATE_BW_80:
		return RATE_INFO_FLAGS_80_MHZ_WIDTH;
	case HDD_RATE_BW_160:
		return RATE_INFO_FLAGS_160_MHZ_WIDTH;
	}

	hdd_err("Unhandled HDD_RATE_BW: %d", hdd_bw);

	return (enum rate_info_flags)0;
}

void hdd_set_rate_bw(struct rate_info *info, enum hdd_rate_info_bw hdd_bw)
{
	const enum rate_info_flags all_bws =
		RATE_INFO_FLAGS_40_MHZ_WIDTH |
		RATE_INFO_FLAGS_80_MHZ_WIDTH |
		RATE_INFO_FLAGS_80P80_MHZ_WIDTH |
		RATE_INFO_FLAGS_160_MHZ_WIDTH;

	info->flags &= ~all_bws;
	info->flags |= hdd_map_hdd_bw_to_os(hdd_bw);
}
#endif

/**
 * struct cfg80211_ops - cfg80211_ops
 *
 * @add_virtual_intf: Add virtual interface
 * @del_virtual_intf: Delete virtual interface
 * @change_virtual_intf: Change virtual interface
 * @change_station: Change station
 * @add_beacon: Add beacon in sap mode
 * @del_beacon: Delete beacon in sap mode
 * @set_beacon: Set beacon in sap mode
 * @start_ap: Start ap
 * @change_beacon: Change beacon
 * @stop_ap: Stop ap
 * @change_bss: Change bss
 * @add_key: Add key
 * @get_key: Get key
 * @del_key: Delete key
 * @set_default_key: Set default key
 * @set_channel: Set channel
 * @scan: Scan
 * @connect: Connect
 * @disconnect: Disconnect
 * @join_ibss = Join ibss
 * @leave_ibss = Leave ibss
 * @set_wiphy_params = Set wiphy params
 * @set_tx_power = Set tx power
 * @get_tx_power = get tx power
 * @remain_on_channel = Remain on channel
 * @cancel_remain_on_channel = Cancel remain on channel
 * @mgmt_tx = Tx management frame
 * @mgmt_tx_cancel_wait = Cancel management tx wait
 * @set_default_mgmt_key = Set default management key
 * @set_txq_params = Set tx queue parameters
 * @get_station = Get station
 * @set_power_mgmt = Set power management
 * @del_station = Delete station
 * @add_station = Add station
 * @set_pmksa = Set pmksa
 * @del_pmksa = Delete pmksa
 * @flush_pmksa = Flush pmksa
 * @update_ft_ies = Update FT IEs
 * @tdls_mgmt = Tdls management
 * @tdls_oper = Tdls operation
 * @set_rekey_data = Set rekey data
 * @sched_scan_start = Scheduled scan start
 * @sched_scan_stop = Scheduled scan stop
 * @resume = Resume wlan
 * @suspend = Suspend wlan
 * @set_mac_acl = Set mac acl
 * @testmode_cmd = Test mode command
 * @set_ap_chanwidth = Set AP channel bandwidth
 * @dump_survey = Dump survey
 * @key_mgmt_set_pmk = Set pmk key management
 * @update_connect_params = Update connect params
 */
static struct cfg80211_ops wlan_hdd_cfg80211_ops = {
	.add_virtual_intf = wlan_hdd_add_virtual_intf,
	.del_virtual_intf = wlan_hdd_del_virtual_intf,
	.change_virtual_intf = wlan_hdd_cfg80211_change_iface,
	.change_station = wlan_hdd_change_station,
	.start_ap = wlan_hdd_cfg80211_start_ap,
	.change_beacon = wlan_hdd_cfg80211_change_beacon,
	.stop_ap = wlan_hdd_cfg80211_stop_ap,
	.change_bss = wlan_hdd_cfg80211_change_bss,
	.add_key = wlan_hdd_cfg80211_add_key,
	.get_key = wlan_hdd_cfg80211_get_key,
	.del_key = wlan_hdd_cfg80211_del_key,
	.set_default_key = wlan_hdd_cfg80211_set_default_key,
	.scan = wlan_hdd_cfg80211_scan,
	.connect = wlan_hdd_cfg80211_connect,
	.disconnect = wlan_hdd_cfg80211_disconnect,
	.join_ibss = wlan_hdd_cfg80211_join_ibss,
	.leave_ibss = wlan_hdd_cfg80211_leave_ibss,
	.set_wiphy_params = wlan_hdd_cfg80211_set_wiphy_params,
	.set_tx_power = wlan_hdd_cfg80211_set_txpower,
	.get_tx_power = wlan_hdd_cfg80211_get_txpower,
	.remain_on_channel = wlan_hdd_cfg80211_remain_on_channel,
	.cancel_remain_on_channel = wlan_hdd_cfg80211_cancel_remain_on_channel,
	.mgmt_tx = wlan_hdd_mgmt_tx,
	.mgmt_tx_cancel_wait = wlan_hdd_cfg80211_mgmt_tx_cancel_wait,
	.set_default_mgmt_key = wlan_hdd_set_default_mgmt_key,
	.set_txq_params = wlan_hdd_set_txq_params,
	.dump_station = wlan_hdd_cfg80211_dump_station,
	.get_station = wlan_hdd_cfg80211_get_station,
	.set_power_mgmt = wlan_hdd_cfg80211_set_power_mgmt,
	.del_station = wlan_hdd_cfg80211_del_station,
	.add_station = wlan_hdd_cfg80211_add_station,
	.set_pmksa = wlan_hdd_cfg80211_set_pmksa,
	.del_pmksa = wlan_hdd_cfg80211_del_pmksa,
	.flush_pmksa = wlan_hdd_cfg80211_flush_pmksa,
#if defined(KERNEL_SUPPORT_11R_CFG80211)
	.update_ft_ies = wlan_hdd_cfg80211_update_ft_ies,
#endif
#ifdef FEATURE_WLAN_TDLS
	.tdls_mgmt = wlan_hdd_cfg80211_tdls_mgmt,
	.tdls_oper = wlan_hdd_cfg80211_tdls_oper,
#endif
#ifdef WLAN_FEATURE_GTK_OFFLOAD
	.set_rekey_data = wlan_hdd_cfg80211_set_rekey_data,
#endif /* WLAN_FEATURE_GTK_OFFLOAD */
#ifdef FEATURE_WLAN_SCAN_PNO
	.sched_scan_start = wlan_hdd_cfg80211_sched_scan_start,
	.sched_scan_stop = wlan_hdd_cfg80211_sched_scan_stop,
#endif /*FEATURE_WLAN_SCAN_PNO */
	.resume = wlan_hdd_cfg80211_resume_wlan,
	.suspend = wlan_hdd_cfg80211_suspend_wlan,
	.set_mac_acl = wlan_hdd_cfg80211_set_mac_acl,
#ifdef WLAN_NL80211_TESTMODE
	.testmode_cmd = wlan_hdd_cfg80211_testmode,
#endif
#ifdef QCA_HT_2040_COEX
	.set_ap_chanwidth = wlan_hdd_cfg80211_set_ap_channel_width,
#endif
	.dump_survey = wlan_hdd_cfg80211_dump_survey,
#ifdef CHANNEL_SWITCH_SUPPORTED
	.channel_switch = wlan_hdd_cfg80211_channel_switch,
#endif
	.set_monitor_channel = wlan_hdd_cfg80211_set_mon_ch,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0)) || \
	defined(CFG80211_ABORT_SCAN)
	.abort_scan = wlan_hdd_cfg80211_abort_scan,
#endif
#if defined(WLAN_FEATURE_FILS_SK) &&\
	defined(CFG80211_FILS_SK_OFFLOAD_SUPPORT) &&\
	(defined(CFG80211_UPDATE_CONNECT_PARAMS) ||\
		(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 10, 0)))
	.update_connect_params = wlan_hdd_cfg80211_update_connect_params,
#endif
};
