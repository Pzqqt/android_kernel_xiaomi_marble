/*
 * Copyright (c) 2012-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "osif_sync.h"
#include <wlan_hdd_includes.h>
#include <net/arp.h>
#include <net/cfg80211.h>
#include <net/mac80211.h>
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
#include "wlan_hdd_tx_rx.h"
#include "qdf_str.h"
#include "qdf_trace.h"
#include "qdf_types.h"
#include "qdf_net_if.h"
#include "cds_utils.h"
#include "cds_sched.h"
#include "wlan_hdd_scan.h"
#include <qc_sap_ioctl.h>
#include "wlan_hdd_tdls.h"
#include "wlan_hdd_wmm.h"
#include "wma_types.h"
#include "wma.h"
#include "wma_twt.h"
#include "wlan_hdd_misc.h"
#include "wlan_hdd_nan.h"
#include "wlan_logging_sock_svc.h"
#include "sap_api.h"
#include "csr_api.h"
#include "pld_common.h"
#include "wmi_unified_param.h"

#include <cdp_txrx_handle.h>
#include <wlan_cfg80211_scan.h>
#include <wlan_cfg80211_ftm.h>

#include "wlan_hdd_ext_scan.h"

#include "wlan_hdd_stats.h"
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
#include "wlan_osif_request_manager.h"
#include "wlan_hdd_he.h"
#ifdef FEATURE_WLAN_APF
#include "wlan_hdd_apf.h"
#endif
#include "wlan_hdd_fw_state.h"
#include "wlan_hdd_mpta_helper.h"

#include <cdp_txrx_cmn.h>
#include <cdp_txrx_misc.h>
#include <cdp_txrx_ctrl.h>
#include <qca_vendor.h>
#include "wlan_pmo_ucfg_api.h"
#include "os_if_wifi_pos.h"
#include "wlan_utility.h"
#include "wlan_reg_ucfg_api.h"
#include "wifi_pos_api.h"
#include "wlan_hdd_spectralscan.h"
#include "wlan_ipa_ucfg_api.h"
#include <wlan_cfg80211_mc_cp_stats.h>
#include <wlan_cp_stats_mc_ucfg_api.h>
#include "wlan_tdls_cfg_api.h"
#include "wlan_tdls_ucfg_api.h"
#include <wlan_hdd_bss_transition.h>
#include <wlan_hdd_concurrency_matrix.h>
#include <wlan_hdd_p2p_listen_offload.h>
#include <wlan_hdd_rssi_monitor.h>
#include <wlan_hdd_sap_cond_chan_switch.h>
#include <wlan_hdd_station_info.h>
#include <wlan_hdd_tx_power.h>
#include <wlan_hdd_active_tos.h>
#include <wlan_hdd_sar_limits.h>
#include <wlan_hdd_ota_test.h>
#include "wlan_policy_mgr_ucfg.h"
#include "wlan_mlme_ucfg_api.h"
#include "wlan_mlme_twt_ucfg_api.h"
#include "wlan_mlme_public_struct.h"
#include "wlan_extscan_ucfg_api.h"
#include "wlan_pmo_cfg.h"
#include "cfg_ucfg_api.h"

#include "wlan_crypto_def_i.h"
#include "wlan_crypto_global_api.h"
#include "wlan_nl_to_crypto_params.h"
#include "wlan_crypto_global_def.h"
#include "cdp_txrx_cfg.h"
#include "wlan_hdd_object_manager.h"
#include "nan_ucfg_api.h"
#include "wlan_fwol_ucfg_api.h"
#include "wlan_cfg80211_crypto.h"
#include "wlan_cfg80211_interop_issues_ap.h"
#include "wlan_scan_ucfg_api.h"
#include "wlan_hdd_coex_config.h"
#include "wlan_hdd_bcn_recv.h"
#include "wlan_hdd_connectivity_logging.h"
#include "wlan_blm_ucfg_api.h"
#include "wlan_hdd_hw_capability.h"
#include "wlan_hdd_oemdata.h"
#include "os_if_fwol.h"
#include "wlan_hdd_sta_info.h"
#include "sme_api.h"
#include "wlan_hdd_thermal.h"
#include <ol_defines.h>
#include "wlan_hdd_btc_chain_mode.h"
#include "os_if_nan.h"
#include "wlan_hdd_apf.h"
#include "wlan_hdd_cfr.h"
#include "wlan_hdd_ioctl.h"
#include "wlan_cm_roam_ucfg_api.h"
#include "hif.h"
#include "wlan_reg_ucfg_api.h"
#include "wlan_hdd_twt.h"
#include "wlan_hdd_gpio.h"
#include "wlan_hdd_medium_assess.h"
#include "wlan_if_mgr_ucfg_api.h"
#include "wlan_if_mgr_public_struct.h"
#include "wlan_wfa_ucfg_api.h"
#include <osif_cm_util.h>
#include <osif_cm_req.h>
#include "wlan_hdd_bootup_marker.h"
#include "wlan_hdd_cm_api.h"
#include "wlan_roam_debug.h"
#include "wlan_hdd_avoid_freq_ext.h"
#include "qdf_util.h"
#include "wlan_hdd_mdns_offload.h"
#include "wlan_pkt_capture_ucfg_api.h"
#include "os_if_pkt_capture.h"
#include "wlan_hdd_peer_txq_flush.h"
#include "wlan_osif_features.h"
#include "wlan_hdd_coap.h"

#define g_mode_rates_size (12)
#define a_mode_rates_size (8)

#define WLAN_WAIT_WLM_LATENCY_LEVEL 1000

/**
 * rtt_is_initiator - Macro to check if the bitmap has any RTT roles set
 * @bitmap: The bitmap to be checked
 */
#define rtt_is_enabled(bitmap) \
	((bitmap) & (WMI_FW_STA_RTT_INITR | \
		     WMI_FW_STA_RTT_RESPR | \
		     WMI_FW_AP_RTT_INITR | \
		     WMI_FW_AP_RTT_RESPR))

/*
 * Android CTS verifier needs atleast this much wait time (in msec)
 */
#define MAX_REMAIN_ON_CHANNEL_DURATION (2000)

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

#define IS_DFS_MODE_VALID(mode) ((mode >= DFS_MODE_NONE && \
			mode <= DFS_MODE_DEPRIORITIZE))

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
	WLAN_CIPHER_SUITE_AES_CMAC,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
	WLAN_CIPHER_SUITE_BIP_GMAC_128,
	WLAN_CIPHER_SUITE_BIP_GMAC_256,
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

#ifdef WLAN_FEATURE_DSRC
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
#else
static const struct ieee80211_channel hdd_5dot9_ghz_ch[] = {
	HDD5GHZCHAN(5845, 169, 0),
	HDD5GHZCHAN(5865, 173, 0),
	HDD5GHZCHAN(5885, 177, 0),
};
#endif

#define band_2_ghz_channels_size sizeof(hdd_channels_2_4_ghz)

#ifdef WLAN_FEATURE_DSRC
#define band_5_ghz_chanenls_size (sizeof(hdd_channels_5_ghz) + \
	sizeof(hdd_channels_dot11p))
#else
#define band_5_ghz_chanenls_size (sizeof(hdd_channels_5_ghz) + \
	sizeof(hdd_5dot9_ghz_ch))
#endif

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

#if defined(CONFIG_BAND_6GHZ) && (defined(CFG80211_6GHZ_BAND_SUPPORTED) || \
	(KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE))

static struct ieee80211_channel hdd_channels_6_ghz[NUM_6GHZ_CHANNELS];

static struct ieee80211_supported_band wlan_hdd_band_6_ghz = {
	.channels = NULL,
	.n_channels = 0,
	.band = HDD_NL80211_BAND_6GHZ,
	.bitrates = a_mode_rates,
	.n_bitrates = a_mode_rates_size,
};

#define HDD_SET_6GHZCHAN(ch, freq, chan, flag)   {     \
		(ch).band =  HDD_NL80211_BAND_6GHZ; \
		(ch).center_freq = (freq); \
		(ch).hw_value = (chan); \
		(ch).flags = (flag); \
		(ch).max_antenna_gain = 0; \
		(ch).max_power = 0; \
}

static void hdd_init_6ghz(struct hdd_context *hdd_ctx)
{
	uint32_t i;
	struct wiphy *wiphy = hdd_ctx->wiphy;
	struct ieee80211_channel *chlist = hdd_channels_6_ghz;
	uint32_t num = ARRAY_SIZE(hdd_channels_6_ghz);
	uint16_t base_freq;
	QDF_STATUS status;
	uint32_t band_capability;

	hdd_enter();

	status = ucfg_mlme_get_band_capability(hdd_ctx->psoc, &band_capability);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("Failed to get MLME Band Capability");
		return;
	}

	if (!(band_capability & (BIT(REG_BAND_6G)))) {
		hdd_debug("6ghz band not enabled");
		return;
	}

	qdf_mem_zero(chlist, sizeof(*chlist) * num);
	base_freq = wlan_reg_min_6ghz_chan_freq();

	for (i = 0; i < num; i++)
		HDD_SET_6GHZCHAN(chlist[i],
				 base_freq + i * 20,
				 wlan_reg_freq_to_chan(hdd_ctx->pdev,
						       base_freq + i * 20),
				 IEEE80211_CHAN_DISABLED);
	wiphy->bands[HDD_NL80211_BAND_6GHZ] = &wlan_hdd_band_6_ghz;
	wiphy->bands[HDD_NL80211_BAND_6GHZ]->channels = chlist;
	wiphy->bands[HDD_NL80211_BAND_6GHZ]->n_channels = num;

	hdd_exit();
}
#else
static void hdd_init_6ghz(struct hdd_context *hdd_ctx)
{
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0)) || \
	defined(CFG80211_IFTYPE_AKM_SUITES_SUPPORT)
/*akm suits supported by sta*/
static const u32 hdd_sta_akm_suites[] = {
	WLAN_AKM_SUITE_8021X,
	WLAN_AKM_SUITE_PSK,
	WLAN_AKM_SUITE_FT_8021X,
	WLAN_AKM_SUITE_FT_PSK,
	WLAN_AKM_SUITE_8021X_SHA256,
	WLAN_AKM_SUITE_PSK_SHA256,
	WLAN_AKM_SUITE_TDLS,
	WLAN_AKM_SUITE_SAE,
	WLAN_AKM_SUITE_FT_OVER_SAE,
	WLAN_AKM_SUITE_EAP_SHA256,
	WLAN_AKM_SUITE_EAP_SHA384,
	WLAN_AKM_SUITE_FILS_SHA256,
	WLAN_AKM_SUITE_FILS_SHA384,
	WLAN_AKM_SUITE_FT_FILS_SHA256,
	WLAN_AKM_SUITE_FT_FILS_SHA384,
	WLAN_AKM_SUITE_OWE,
	WLAN_AKM_SUITE_DPP_RSN,
	WLAN_AKM_SUITE_FT_EAP_SHA_384,
	RSN_AUTH_KEY_MGMT_CCKM,
	RSN_AUTH_KEY_MGMT_OSEN,
	WAPI_PSK_AKM_SUITE,
	WAPI_CERT_AKM_SUITE,
};

/*akm suits supported by AP*/
static const u32 hdd_ap_akm_suites[] = {
	WLAN_AKM_SUITE_PSK,
	WLAN_AKM_SUITE_SAE,
	WLAN_AKM_SUITE_OWE,
};

/* This structure contain information what akm suits are
 * supported for each mode
 */
static const struct wiphy_iftype_akm_suites
	wlan_hdd_akm_suites[] = {
	{
		.iftypes_mask = BIT(NL80211_IFTYPE_STATION) |
				BIT(NL80211_IFTYPE_P2P_CLIENT),
		.akm_suites = hdd_sta_akm_suites,
		.n_akm_suites = (sizeof(hdd_sta_akm_suites) / sizeof(u32)),
	},
	{
		.iftypes_mask = BIT(NL80211_IFTYPE_AP) |
				BIT(NL80211_IFTYPE_P2P_GO),
		.akm_suites = hdd_ap_akm_suites,
		.n_akm_suites = (sizeof(hdd_ap_akm_suites) / sizeof(u32)),
	},
};
#endif

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
		      BIT(SIR_MAC_MGMT_AUTH) |
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
		.max = 2,
		.types = BIT(NL80211_IFTYPE_STATION),
	},
};

/* AP ( + AP ) combination */
static const struct ieee80211_iface_limit
	wlan_hdd_ap_iface_limit[] = {
	{
		.max = (QDF_MAX_NO_OF_SAP_MODE),
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

/* STA + AP combination */
static const struct ieee80211_iface_limit
	wlan_hdd_sta_ap_iface_limit[] = {
	{
		.max = 1,
		.types = BIT(NL80211_IFTYPE_STATION),
	},
	{
		.max = QDF_MAX_NO_OF_SAP_MODE,
		.types = BIT(NL80211_IFTYPE_AP),
	},
};

/* STA + P2P + P2P combination */
static const struct ieee80211_iface_limit
	wlan_hdd_sta_p2p_iface_limit[] = {
	{
		.max = 1,
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

/* STA + AP + P2P combination */
static const struct ieee80211_iface_limit
wlan_hdd_sta_ap_p2p_iface_limit[] = {
	{
	   .max = 1,
	   .types = BIT(NL80211_IFTYPE_STATION)
	},
	{
	   .max = 1,
	   .types = BIT(NL80211_IFTYPE_P2P_GO) | BIT(NL80211_IFTYPE_P2P_CLIENT)
	},
	{
	   .max = 1,
	   .types = BIT(NL80211_IFTYPE_AP)
	},
};

/* SAP + P2P combination */
static const struct ieee80211_iface_limit
wlan_hdd_sap_p2p_iface_limit[] = {
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
		.max = 2,       /* Monitor interface */
		.types = BIT(NL80211_IFTYPE_MONITOR),
	},
};

#if defined(WLAN_FEATURE_NAN) && \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
/* STA + NAN disc combination */
static const struct ieee80211_iface_limit
	wlan_hdd_sta_nan_iface_limit[] = {
	{
		/* STA */
		.max = 1,
		.types = BIT(NL80211_IFTYPE_STATION)
	},
	{
		/* NAN */
		.max = 1,
		.types = BIT(NL80211_IFTYPE_NAN),
	},
};

/* SAP + NAN disc combination */
static const struct ieee80211_iface_limit
	wlan_hdd_sap_nan_iface_limit[] = {
	{
		/* SAP */
		.max = 1,
		.types = BIT(NL80211_IFTYPE_AP)
	},
	{
		/* NAN */
		.max = 1,
		.types = BIT(NL80211_IFTYPE_NAN),
	},
};
#endif /* WLAN_FEATURE_NAN */

static struct ieee80211_iface_combination
	wlan_hdd_iface_combination[] = {
	/* STA */
	{
		.limits = wlan_hdd_sta_iface_limit,
		.num_different_channels = 2,
		.max_interfaces = 2,
		.n_limits = ARRAY_SIZE(wlan_hdd_sta_iface_limit),
	},
	/* AP */
	{
		.limits = wlan_hdd_ap_iface_limit,
		.num_different_channels = 2,
		.max_interfaces = (QDF_MAX_NO_OF_SAP_MODE),
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
		.max_interfaces = (1 + QDF_MAX_NO_OF_SAP_MODE),
		.n_limits = ARRAY_SIZE(wlan_hdd_sta_ap_iface_limit),
		.beacon_int_infra_match = true,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 10, 0)) || \
	defined(CFG80211_BEACON_INTERVAL_BACKPORT)
		.beacon_int_min_gcd = 1,
#endif
	},
	/* STA + P2P + P2P */
	{
		.limits = wlan_hdd_sta_p2p_iface_limit,
		.num_different_channels = 2,
		.max_interfaces = 3,
		.n_limits = ARRAY_SIZE(wlan_hdd_sta_p2p_iface_limit),
		.beacon_int_infra_match = true,
	},
	/* STA + P2P + SAP */
	{
		.limits = wlan_hdd_sta_ap_p2p_iface_limit,
		/* we can allow 3 channels for three different persona
		 * but due to firmware limitation, allow max 2 concrnt channels.
		 */
		.num_different_channels = 2,
		.max_interfaces = 3,
		.n_limits = ARRAY_SIZE(wlan_hdd_sta_ap_p2p_iface_limit),
		.beacon_int_infra_match = true,
	},
	/* SAP + P2P */
	{
		.limits = wlan_hdd_sap_p2p_iface_limit,
		.num_different_channels = 2,
		/* 1-SAP + 1-P2P */
		.max_interfaces = 2,
		.n_limits = ARRAY_SIZE(wlan_hdd_sap_p2p_iface_limit),
		.beacon_int_infra_match = true,
	},
	/* P2P + P2P */
	{
		.limits = wlan_hdd_p2p_p2p_iface_limit,
		.num_different_channels = 2,
		/* 2-P2P */
		.max_interfaces = 2,
		.n_limits = ARRAY_SIZE(wlan_hdd_p2p_p2p_iface_limit),
		.beacon_int_infra_match = true,
	},
	/* Monitor */
	{
		.limits = wlan_hdd_mon_iface_limit,
		.max_interfaces = 2,
		.num_different_channels = 2,
		.n_limits = ARRAY_SIZE(wlan_hdd_mon_iface_limit),
	},
#if defined(WLAN_FEATURE_NAN) && \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
	/* NAN + STA */
	{
		.limits = wlan_hdd_sta_nan_iface_limit,
		.max_interfaces = 2,
		.num_different_channels = 2,
		.n_limits = ARRAY_SIZE(wlan_hdd_sta_nan_iface_limit),
	},
	/* NAN + SAP */
	{
		.limits = wlan_hdd_sap_nan_iface_limit,
		.num_different_channels = 2,
		.max_interfaces = 2,
		.n_limits = ARRAY_SIZE(wlan_hdd_sap_nan_iface_limit),
		.beacon_int_infra_match = true,
	},
#endif /* WLAN_FEATURE_NAN */
};

/* 1 and 2 port concurrencies */
static struct ieee80211_iface_combination
	wlan_hdd_derived_combination[] = {
	/* STA */
	{
		.limits = wlan_hdd_sta_iface_limit,
		.num_different_channels = 2,
		.max_interfaces = 2,
		.n_limits = ARRAY_SIZE(wlan_hdd_sta_iface_limit),
	},
	/* AP */
	{
		.limits = wlan_hdd_ap_iface_limit,
		.num_different_channels = 2,
		.max_interfaces = (QDF_MAX_NO_OF_SAP_MODE),
		.n_limits = ARRAY_SIZE(wlan_hdd_ap_iface_limit),
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 10, 0)) || \
	defined(CFG80211_BEACON_INTERVAL_BACKPORT)
		.beacon_int_min_gcd = 1,
#endif
	},
#ifndef WLAN_FEATURE_NO_P2P_CONCURRENCY
	/* P2P */
	{
		.limits = wlan_hdd_p2p_iface_limit,
		.num_different_channels = 2,
		.max_interfaces = 2,
		.n_limits = ARRAY_SIZE(wlan_hdd_p2p_iface_limit),
	},

	/* SAP + P2P */
	{
		.limits = wlan_hdd_sap_p2p_iface_limit,
		.num_different_channels = 2,
		/* 1-SAP + 1-P2P */
		.max_interfaces = 2,
		.n_limits = ARRAY_SIZE(wlan_hdd_sap_p2p_iface_limit),
		.beacon_int_infra_match = true,
	},
	/* P2P + P2P */
	{
		.limits = wlan_hdd_p2p_p2p_iface_limit,
		.num_different_channels = 2,
		/* 2-P2P */
		.max_interfaces = 2,
		.n_limits = ARRAY_SIZE(wlan_hdd_p2p_p2p_iface_limit),
		.beacon_int_infra_match = true,
	},
#endif
	/* STA + P2P */
	{
		.limits = wlan_hdd_sta_p2p_iface_limit,
		.num_different_channels = 2,
		.max_interfaces = 2,
		.n_limits = ARRAY_SIZE(wlan_hdd_sta_p2p_iface_limit),
		.beacon_int_infra_match = true,
	},
#ifndef WLAN_FEATURE_NO_STA_SAP_CONCURRENCY
	/* STA + SAP */
	{
		.limits = wlan_hdd_sta_ap_iface_limit,
		.num_different_channels = 2,
		.max_interfaces = 2,
		.n_limits = ARRAY_SIZE(wlan_hdd_sta_ap_iface_limit),
		.beacon_int_infra_match = true,
	},
#endif /* WLAN_FEATURE_NO_STA_SAP_CONCURRENCY */
	/* Monitor */
	{
		.limits = wlan_hdd_mon_iface_limit,
		.max_interfaces = 2,
		.num_different_channels = 2,
		.n_limits = ARRAY_SIZE(wlan_hdd_mon_iface_limit),
	},
#if defined(WLAN_FEATURE_NAN) && \
	   (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
#ifndef WLAN_FEATURE_NO_STA_NAN_CONCURRENCY
	/* NAN + STA */
	{
		.limits = wlan_hdd_sta_nan_iface_limit,
		.max_interfaces = 2,
		.num_different_channels = 2,
		.n_limits = ARRAY_SIZE(wlan_hdd_sta_nan_iface_limit),
	},
#endif /* WLAN_FEATURE_NO_STA_NAN_CONCURRENCY */
#ifndef WLAN_FEATURE_NO_SAP_NAN_CONCURRENCY
	/* NAN + SAP */
	{
		.limits = wlan_hdd_sap_nan_iface_limit,
		.num_different_channels = 2,
		.max_interfaces = 2,
		.n_limits = ARRAY_SIZE(wlan_hdd_sap_nan_iface_limit),
		.beacon_int_infra_match = true,
	},
#endif /* !WLAN_FEATURE_NO_SAP_NAN_CONCURRENCY */
#endif /* WLAN_FEATURE_NAN */
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
 * __wlan_hdd_cfg80211_get_tdls_capabilities() - Provide TDLS Capabilities.
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
	bool tdls_support;
	bool tdls_external_control;
	bool tdls_sleep_sta_enable;
	bool tdls_buffer_sta;
	bool tdls_off_channel;

	hdd_enter_dev(wdev->netdev);

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

	if ((cfg_tdls_get_support_enable(hdd_ctx->psoc, &tdls_support) ==
	     QDF_STATUS_SUCCESS) && !tdls_support) {
		hdd_debug("TDLS feature not Enabled or Not supported in FW");
		if (nla_put_u32(skb, PARAM_MAX_TDLS_SESSION, 0) ||
			nla_put_u32(skb, PARAM_TDLS_FEATURE_SUPPORT, 0)) {
			hdd_err("nla put fail");
			goto fail;
		}
	} else {
		cfg_tdls_get_external_control(hdd_ctx->psoc,
					      &tdls_external_control);
		cfg_tdls_get_sleep_sta_enable(hdd_ctx->psoc,
					      &tdls_sleep_sta_enable);
		cfg_tdls_get_buffer_sta_enable(hdd_ctx->psoc,
					       &tdls_buffer_sta);
		cfg_tdls_get_off_channel_enable(hdd_ctx->psoc,
						&tdls_off_channel);
		set = set | WIFI_TDLS_SUPPORT;
		set = set | (tdls_external_control ?
					WIFI_TDLS_EXTERNAL_CONTROL_SUPPORT : 0);
		set = set | (tdls_off_channel ?
					WIIF_TDLS_OFFCHANNEL_SUPPORT : 0);
		max_num_tdls_sta = cfg_tdls_get_max_peer_count(hdd_ctx->psoc);

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
 * wlan_hdd_cfg80211_get_tdls_capabilities() - Provide TDLS Capabilities.
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
	struct osif_psoc_sync *psoc_sync;
	int errno;

	errno = osif_psoc_sync_op_start(wiphy_dev(wiphy), &psoc_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_get_tdls_capabilities(wiphy, wdev,
							  data, data_len);

	osif_psoc_sync_op_stop(psoc_sync);

	return errno;
}

static uint8_t hdd_get_bw_offset(uint32_t ch_width)
{
	uint8_t bw_offset = 0;

	if (ch_width == CH_WIDTH_40MHZ)
		bw_offset = 1 << BW_40_OFFSET_BIT;
	else if (ch_width == CH_WIDTH_20MHZ)
		bw_offset = 1 << BW_20_OFFSET_BIT;

	return bw_offset;
}

#else /* !FEATURE_WLAN_TDLS */

static inline uint8_t hdd_get_bw_offset(uint32_t ch_width)
{
	return 0;
}

#endif /* FEATURE_WLAN_TDLS */

/**
 * wlan_vendor_bitmap_to_reg_wifi_band_bitmap() - Convert vendor bitmap to
 * reg_wifi_band bitmap
 * @psoc: PSOC pointer
 * @vendor_bitmap: vendor bitmap value coming via vendor command
 *
 * Return: reg_wifi_band bitmap
 */
static uint32_t
wlan_vendor_bitmap_to_reg_wifi_band_bitmap(struct wlan_objmgr_psoc *psoc,
					   uint32_t vendor_bitmap)
{
	uint32_t reg_bitmap = 0;

	if (vendor_bitmap == QCA_SETBAND_AUTO)
		reg_bitmap |= REG_BAND_MASK_ALL;
	if (vendor_bitmap & QCA_SETBAND_2G)
		reg_bitmap |= BIT(REG_BAND_2G);
	if (vendor_bitmap & QCA_SETBAND_5G)
		reg_bitmap |= BIT(REG_BAND_5G);
	if (vendor_bitmap & QCA_SETBAND_6G)
		reg_bitmap |= BIT(REG_BAND_6G);

	if (!wlan_reg_is_6ghz_supported(psoc)) {
		hdd_debug("Driver doesn't support 6ghz");
		reg_bitmap = (reg_bitmap & (~BIT(REG_BAND_6G)));
	}

	return reg_bitmap;
}

int wlan_hdd_merge_avoid_freqs(struct ch_avoid_ind_type *destFreqList,
		struct ch_avoid_ind_type *srcFreqList)
{
	int i;
	uint32_t room;
	struct ch_avoid_freq_type *avoid_range =
	&destFreqList->avoid_freq_range[destFreqList->ch_avoid_range_cnt];

	room = CH_AVOID_MAX_RANGE - destFreqList->ch_avoid_range_cnt;
	if (srcFreqList->ch_avoid_range_cnt > room) {
		hdd_err("avoid freq overflow");
		return -EINVAL;
	}
	destFreqList->ch_avoid_range_cnt += srcFreqList->ch_avoid_range_cnt;

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

	hdd_enter();

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

	hdd_exit();
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
	case QDF_WMI_EXCEED_MAX_PENDING_CMDS:
		ret_val = QCA_WLAN_HANG_WMI_EXCEED_MAX_PENDING_CMDS;
		break;
	case QDF_AP_STA_CONNECT_REQ_TIMEOUT:
		ret_val = QCA_WLAN_HANG_AP_STA_CONNECT_REQ_TIMEOUT;
		break;
	case QDF_STA_AP_CONNECT_REQ_TIMEOUT:
		ret_val = QCA_WLAN_HANG_STA_AP_CONNECT_REQ_TIMEOUT;
		break;
	case QDF_MAC_HW_MODE_CHANGE_TIMEOUT:
		ret_val = QCA_WLAN_HANG_MAC_HW_MODE_CHANGE_TIMEOUT;
		break;
	case QDF_MAC_HW_MODE_CONFIG_TIMEOUT:
		ret_val = QCA_WLAN_HANG_MAC_HW_MODE_CONFIG_TIMEOUT;
		break;
	case QDF_VDEV_START_RESPONSE_TIMED_OUT:
		ret_val = QCA_WLAN_HANG_VDEV_START_RESPONSE_TIMED_OUT;
		break;
	case QDF_VDEV_RESTART_RESPONSE_TIMED_OUT:
		ret_val = QCA_WLAN_HANG_VDEV_RESTART_RESPONSE_TIMED_OUT;
		break;
	case QDF_VDEV_STOP_RESPONSE_TIMED_OUT:
		ret_val = QCA_WLAN_HANG_VDEV_STOP_RESPONSE_TIMED_OUT;
		break;
	case QDF_VDEV_DELETE_RESPONSE_TIMED_OUT:
		ret_val = QCA_WLAN_HANG_VDEV_DELETE_RESPONSE_TIMED_OUT;
		break;
	case QDF_VDEV_PEER_DELETE_ALL_RESPONSE_TIMED_OUT:
		ret_val = QCA_WLAN_HANG_VDEV_PEER_DELETE_ALL_RESPONSE_TIMED_OUT;
		break;
	case QDF_WMI_BUF_SEQUENCE_MISMATCH:
		ret_val = QCA_WLAN_HANG_VDEV_PEER_DELETE_ALL_RESPONSE_TIMED_OUT;
		break;
	case QDF_HAL_REG_WRITE_FAILURE:
		ret_val = QCA_WLAN_HANG_REG_WRITE_FAILURE;
		break;
	case QDF_SUSPEND_NO_CREDIT:
		ret_val = QCA_WLAN_HANG_SUSPEND_NO_CREDIT;
		break;
	case QCA_HANG_BUS_FAILURE:
		ret_val = QCA_WLAN_HANG_BUS_FAILURE;
		break;
	case QDF_TASKLET_CREDIT_LATENCY_DETECT:
		ret_val = QCA_WLAN_HANG_TASKLET_CREDIT_LATENCY_DETECT;
		break;
	case QDF_RX_REG_PKT_ROUTE_ERR:
		ret_val = QCA_WLAN_HANG_RX_MSDU_BUF_RCVD_IN_ERR_RING;
		break;
	case QDF_VDEV_SM_OUT_OF_SYNC:
		ret_val = QCA_WLAN_HANG_VDEV_SM_OUT_OF_SYNC;
		break;
	case QDF_STATS_REQ_TIMEDOUT:
		ret_val = QCA_WLAN_HANG_STATS_REQ_TIMEOUT;
		break;
	case QDF_TX_DESC_LEAK:
		ret_val = QCA_WLAN_HANG_TX_DESC_LEAK;
		break;
	case QDF_SCHED_TIMEOUT:
		ret_val = QCA_WLAN_HANG_SCHED_TIMEOUT;
		break;
	case QDF_SELF_PEER_DEL_FAILED:
		ret_val = QCA_WLAN_HANG_SELF_PEER_DEL_FAIL;
		break;
	case QDF_DEL_SELF_STA_FAILED:
		ret_val = QCA_WLAN_HANG_DEL_SELF_STA_FAIL;
		break;
	case QDF_FLUSH_LOGS:
		ret_val = QCA_WLAN_HANG_FLUSH_LOGS;
		break;
	case QDF_HOST_WAKEUP_REASON_PAGEFAULT:
		ret_val = QCA_WLAN_HANG_HOST_WAKEUP_REASON_PAGE_FAULT;
		break;
	case QDF_REASON_UNSPECIFIED:
	default:
		ret_val = QCA_WLAN_HANG_REASON_UNSPECIFIED;
		break;
	}
	return ret_val;
}

/**
 * wlan_hdd_send_hang_reason_event() - Send hang reason to the userspace
 * @hdd_ctx: Pointer to hdd context
 * @reason: cds recovery reason
 * @data: Hang Data
 *
 * Return: 0 on success or failure reason
 */
int wlan_hdd_send_hang_reason_event(struct hdd_context *hdd_ctx,
				    enum qdf_hang_reason reason, uint8_t *data,
				    size_t data_len)
{
	struct sk_buff *vendor_event;
	enum qca_wlan_vendor_hang_reason hang_reason;
	struct hdd_adapter *sta_adapter;
	struct wireless_dev *wdev = NULL;

	hdd_enter();

	if (!hdd_ctx) {
		hdd_err("HDD context is null");
		return -EINVAL;
	}

	sta_adapter = hdd_get_adapter(hdd_ctx, QDF_STA_MODE);
	if (sta_adapter)
		wdev = &(sta_adapter->wdev);

	vendor_event = cfg80211_vendor_event_alloc(hdd_ctx->wiphy,
						   wdev,
						   sizeof(uint32_t) + data_len,
						   HANG_REASON_INDEX,
						   GFP_KERNEL);
	if (!vendor_event) {
		hdd_err("cfg80211_vendor_event_alloc failed");
		return -ENOMEM;
	}

	hang_reason = hdd_convert_hang_reason(reason);

	if (nla_put_u32(vendor_event, QCA_WLAN_VENDOR_ATTR_HANG_REASON,
			(uint32_t)hang_reason) ||
	    nla_put(vendor_event, QCA_WLAN_VENDOR_ATTR_HANG_REASON_DATA,
		    data_len, data)) {
		hdd_err("QCA_WLAN_VENDOR_ATTR_HANG_REASON put fail");
		kfree_skb(vendor_event);
		return -EINVAL;
	}

	cfg80211_vendor_event(vendor_event, GFP_KERNEL);

	hdd_exit();
	return 0;
}

#undef HANG_REASON_INDEX

/**
 * wlan_hdd_get_adjacent_chan_freq(): Gets next/previous channel
 * with respect to the channel passed.
 * @freq: Channel frequency
 * @upper: If "true" then next channel is returned or else
 * previous channel is returned.
 *
 * This function returns the next/previous adjacent-channel to
 * the channel passed. If "upper = true" then next channel is
 * returned else previous is returned.
 */
static qdf_freq_t wlan_hdd_get_adjacent_chan_freq(qdf_freq_t freq, bool upper)
{
	enum channel_enum ch_idx = wlan_reg_get_chan_enum_for_freq(freq);

	if (reg_is_chan_enum_invalid(ch_idx))
		return -EINVAL;

	if (upper && (ch_idx < (NUM_CHANNELS - 1)))
		ch_idx++;
	else if (!upper && (ch_idx > CHAN_ENUM_2412))
		ch_idx--;
	else
		return -EINVAL;

	return WLAN_REG_CH_TO_FREQ(ch_idx);
}

/**
 * wlan_hdd_send_avoid_freq_for_dnbs(): Sends list of frequencies to be
 * avoided when Do_Not_Break_Stream is active.
 * @hdd_ctx:  HDD Context
 * @op_freq:  AP/P2P-GO operating channel frequency
 *
 * This function sends list of frequencies to be avoided when
 * Do_Not_Break_Stream is active.
 * To clear the avoid_frequency_list in the application,
 * op_freq = 0 can be passed.
 *
 * Return: 0 on success and errno on failure
 */
int wlan_hdd_send_avoid_freq_for_dnbs(struct hdd_context *hdd_ctx,
				      qdf_freq_t op_freq)
{
	struct ch_avoid_ind_type p2p_avoid_freq_list;
	qdf_freq_t min_freq, max_freq;
	int ret;
	qdf_freq_t freq;

	hdd_enter();

	if (!hdd_ctx) {
		hdd_err("invalid param");
		return -EINVAL;
	}

	qdf_mem_zero(&p2p_avoid_freq_list, sizeof(struct ch_avoid_ind_type));
	/*
	 * If channel passed is zero, clear the avoid_freq list in application.
	 */
	if (!op_freq) {
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

	if (WLAN_REG_IS_24GHZ_CH_FREQ(op_freq)) {
		min_freq = WLAN_REG_MIN_24GHZ_CHAN_FREQ;
		max_freq = WLAN_REG_MAX_24GHZ_CHAN_FREQ;
	} else if (WLAN_REG_IS_5GHZ_CH_FREQ(op_freq)) {
		min_freq = WLAN_REG_MIN_5GHZ_CHAN_FREQ;
		max_freq = WLAN_REG_MAX_5GHZ_CHAN_FREQ;
	} else {
		hdd_err("invalid channel freq:%d", op_freq);
		return -EINVAL;
	}

	if (op_freq > min_freq && op_freq < max_freq) {
		p2p_avoid_freq_list.ch_avoid_range_cnt = 2;
		p2p_avoid_freq_list.avoid_freq_range[0].start_freq = min_freq;

		/* Get channel before the op_freq */
		freq = wlan_hdd_get_adjacent_chan_freq(op_freq, false);
		if (freq < 0)
			return -EINVAL;
		p2p_avoid_freq_list.avoid_freq_range[0].end_freq = freq;

		/* Get channel next to the op_freq */
		freq = wlan_hdd_get_adjacent_chan_freq(op_freq, true);
		if (freq < 0)
			return -EINVAL;
		p2p_avoid_freq_list.avoid_freq_range[1].start_freq = freq;

		p2p_avoid_freq_list.avoid_freq_range[1].end_freq = max_freq;
	} else if (op_freq == min_freq) {
		p2p_avoid_freq_list.ch_avoid_range_cnt = 1;

		freq = wlan_hdd_get_adjacent_chan_freq(op_freq, true);
		if (freq < 0)
			return -EINVAL;
		p2p_avoid_freq_list.avoid_freq_range[0].start_freq = freq;

		p2p_avoid_freq_list.avoid_freq_range[0].end_freq = max_freq;
	} else {
		p2p_avoid_freq_list.ch_avoid_range_cnt = 1;
		p2p_avoid_freq_list.avoid_freq_range[0].start_freq = min_freq;

		freq = wlan_hdd_get_adjacent_chan_freq(op_freq, false);
		if (freq < 0)
			return -EINVAL;
		p2p_avoid_freq_list.avoid_freq_range[0].end_freq = freq;
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

	[QCA_NL80211_VENDOR_SUBCMD_NAN_INDEX] = {
		.vendor_id =
			QCA_NL80211_VENDOR_ID,
		.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_NAN
	},

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

	FEATURE_RSSI_MONITOR_VENDOR_EVENTS

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

	FEATURE_INTEROP_ISSUES_AP_VENDOR_COMMANDS_INDEX

	[QCA_NL80211_VENDOR_SUBCMD_NDP_INDEX] = {
		.vendor_id = QCA_NL80211_VENDOR_ID,
		.subcmd = QCA_NL80211_VENDOR_SUBCMD_NDP
	},

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
	[QCA_NL80211_VENDOR_SUBCMD_THROUGHPUT_CHANGE_EVENT_INDEX] = {
		.vendor_id = QCA_NL80211_VENDOR_ID,
		.subcmd = QCA_NL80211_VENDOR_SUBCMD_THROUGHPUT_CHANGE_EVENT,
	 },
	[QCA_NL80211_VENDOR_SUBCMD_NAN_EXT_INDEX] = {
		.vendor_id = QCA_NL80211_VENDOR_ID,
		.subcmd = QCA_NL80211_VENDOR_SUBCMD_NAN_EXT
	},
	[QCA_NL80211_VENDOR_SUBCMD_LINK_PROPERTIES_INDEX] = {
		.vendor_id = QCA_NL80211_VENDOR_ID,
		.subcmd = QCA_NL80211_VENDOR_SUBCMD_LINK_PROPERTIES,
	},

	BCN_RECV_FEATURE_VENDOR_EVENTS
	FEATURE_MEDIUM_ASSESS_VENDOR_EVENTS
	[QCA_NL80211_VENDOR_SUBCMD_ROAM_INDEX] = {
		.vendor_id = QCA_NL80211_VENDOR_ID,
		.subcmd = QCA_NL80211_VENDOR_SUBCMD_ROAM,
	},
	[QCA_NL80211_VENDOR_SUBCMD_OEM_DATA_INDEX] = {
		.vendor_id = QCA_NL80211_VENDOR_ID,
		.subcmd = QCA_NL80211_VENDOR_SUBCMD_OEM_DATA,
	},
	[QCA_NL80211_VENDOR_SUBCMD_REQUEST_SAR_LIMITS_INDEX] = {
		.vendor_id = QCA_NL80211_VENDOR_ID,
		.subcmd = QCA_NL80211_VENDOR_SUBCMD_GET_SAR_LIMITS_EVENT,
	},
	[QCA_NL80211_VENDOR_SUBCMD_UPDATE_STA_INFO_INDEX] = {
		.vendor_id = QCA_NL80211_VENDOR_ID,
		.subcmd = QCA_NL80211_VENDOR_SUBCMD_UPDATE_STA_INFO,
	},
	FEATURE_THERMAL_VENDOR_EVENTS
	FEATURE_DRIVER_DISCONNECT_REASON
#ifdef WLAN_SUPPORT_TWT
	FEATURE_TWT_VENDOR_EVENTS
#endif
	FEATURE_CFR_DATA_VENDOR_EVENTS
#ifdef WLAN_FEATURE_CONNECTIVITY_LOGGING
	FEATURE_CONNECTIVITY_LOGGING_EVENT
#endif
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	[QCA_NL80211_VENDOR_SUBCMD_ROAM_EVENTS_INDEX] = {
		.vendor_id = QCA_NL80211_VENDOR_ID,
		.subcmd = QCA_NL80211_VENDOR_SUBCMD_ROAM_EVENTS,
	},
#endif
};

/**
 * __is_driver_dfs_capable() - get driver DFS offload capability
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

	hdd_enter_dev(wdev->netdev);

	ret_val = wlan_hdd_validate_context(hdd_ctx);
	if (ret_val)
		return ret_val;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)) || \
	defined(CFG80211_DFS_OFFLOAD_BACKPORT)
	dfs_capability =
		wiphy_ext_feature_isset(wiphy,
					NL80211_EXT_FEATURE_DFS_OFFLOAD);
#else
	dfs_capability = !!(wiphy->flags & WIPHY_FLAG_DFS_OFFLOAD);
#endif

	temp_skbuff = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, sizeof(u32) +
							  NLMSG_HDRLEN);

	if (temp_skbuff) {
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
	struct osif_psoc_sync *psoc_sync;
	int errno;

	errno = osif_psoc_sync_op_start(wiphy_dev(wiphy), &psoc_sync);
	if (errno)
		return errno;

	errno = __is_driver_dfs_capable(wiphy, wdev, data, data_len);

	osif_psoc_sync_op_stop(psoc_sync);

	return errno;
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
	struct sap_config *sap_config, *con_sap_config;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	uint32_t con_ch_freq;

	if (!hdd_ctx) {
		hdd_err("hdd context is NULL");
		return 0;
	}

	/*
	 * Check if AP+AP case, once primary AP chooses a DFS
	 * channel secondary AP should always follow primary APs channel
	 */
	if (!policy_mgr_concurrent_beaconing_sessions_running(
		hdd_ctx->psoc))
		return 0;

	con_sap_adapter = hdd_get_con_sap_adapter(adapter, true);
	if (!con_sap_adapter)
		return 0;

	sap_config = &adapter->session.ap.sap_config;
	con_sap_config = &con_sap_adapter->session.ap.sap_config;
	con_ch_freq = con_sap_adapter->session.ap.operating_chan_freq;

	if (!wlan_reg_is_dfs_for_freq(hdd_ctx->pdev, con_ch_freq))
		return 0;

	hdd_debug("Only SCC AP-AP DFS Permitted (ch_freq=%d, con_ch_freq=%d)",
		  sap_config->chan_freq, con_ch_freq);
	hdd_debug("Overriding guest AP's channel");
	sap_config->chan_freq = con_ch_freq;

	if (con_sap_config->acs_cfg.acs_mode == true) {
		if (con_ch_freq != con_sap_config->acs_cfg.pri_ch_freq &&
		    con_ch_freq != con_sap_config->acs_cfg.ht_sec_ch_freq) {
			hdd_err("Primary AP channel config error");
			hdd_err("Operating ch: %d ACS ch freq: %d Sec Freq %d",
				con_ch_freq,
				con_sap_config->acs_cfg.pri_ch_freq,
				con_sap_config->acs_cfg.ht_sec_ch_freq);
			return -EINVAL;
		}
		/* Sec AP ACS info is overwritten with Pri AP due to DFS
		 * MCC restriction. So free ch list allocated in do_acs
		 * func for Sec AP and realloc for Pri AP ch list size
		 */
		if (sap_config->acs_cfg.freq_list) {
			qdf_mem_free(sap_config->acs_cfg.freq_list);
			sap_config->acs_cfg.freq_list = NULL;
		}
		if (sap_config->acs_cfg.master_freq_list) {
			qdf_mem_free(sap_config->acs_cfg.master_freq_list);
			sap_config->acs_cfg.master_freq_list = NULL;
		}

		qdf_mem_copy(&sap_config->acs_cfg,
			     &con_sap_config->acs_cfg,
			     sizeof(struct sap_acs_cfg));

		sap_config->acs_cfg.freq_list =
			qdf_mem_malloc(sizeof(uint32_t) *
				con_sap_config->acs_cfg.ch_list_count);
		if (!sap_config->acs_cfg.freq_list) {
			sap_config->acs_cfg.ch_list_count = 0;
			return -ENOMEM;
		}
		qdf_mem_copy(sap_config->acs_cfg.freq_list,
			     con_sap_config->acs_cfg.freq_list,
			     con_sap_config->acs_cfg.ch_list_count *
				sizeof(uint32_t));

		sap_config->acs_cfg.master_freq_list =
			qdf_mem_malloc(sizeof(uint32_t) *
				con_sap_config->acs_cfg.master_ch_list_count);
		if (!sap_config->acs_cfg.master_freq_list) {
			sap_config->acs_cfg.master_ch_list_count = 0;
			qdf_mem_free(sap_config->acs_cfg.freq_list);
			sap_config->acs_cfg.freq_list = NULL;
			return -ENOMEM;
		}
		qdf_mem_copy(sap_config->acs_cfg.master_freq_list,
			     con_sap_config->acs_cfg.master_freq_list,
			     con_sap_config->acs_cfg.master_ch_list_count *
				sizeof(uint32_t));
	} else {
		sap_config->acs_cfg.pri_ch_freq = con_ch_freq;
		if (sap_config->acs_cfg.ch_width > eHT_CHANNEL_WIDTH_20MHZ)
			sap_config->acs_cfg.ht_sec_ch_freq =
						con_sap_config->sec_ch_freq;
	}

	return con_ch_freq;
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
	struct sap_config *sap_cfg, enum qca_wlan_vendor_acs_hw_mode hw_mode,
	bool ht_enabled, bool vht_enabled)
{
	int i;

	if (hw_mode == QCA_ACS_MODE_IEEE80211B) {
		sap_cfg->acs_cfg.hw_mode = eCSR_DOT11_MODE_11b;
		sap_cfg->acs_cfg.start_ch_freq =
				wlan_reg_ch_to_freq(CHAN_ENUM_2412);
		sap_cfg->acs_cfg.end_ch_freq =
				wlan_reg_ch_to_freq(CHAN_ENUM_2484);
	} else if (hw_mode == QCA_ACS_MODE_IEEE80211G) {
		sap_cfg->acs_cfg.hw_mode = eCSR_DOT11_MODE_11g;
		sap_cfg->acs_cfg.start_ch_freq =
				wlan_reg_ch_to_freq(CHAN_ENUM_2412);
		sap_cfg->acs_cfg.end_ch_freq =
				wlan_reg_ch_to_freq(CHAN_ENUM_2472);
	} else if (hw_mode == QCA_ACS_MODE_IEEE80211A) {
		sap_cfg->acs_cfg.hw_mode = eCSR_DOT11_MODE_11a;
		sap_cfg->acs_cfg.start_ch_freq =
				wlan_reg_ch_to_freq(CHAN_ENUM_5180);
		sap_cfg->acs_cfg.end_ch_freq =
				wlan_reg_ch_to_freq(CHAN_ENUM_5885);
	} else if (hw_mode == QCA_ACS_MODE_IEEE80211ANY) {
		sap_cfg->acs_cfg.hw_mode = eCSR_DOT11_MODE_abg;
		sap_cfg->acs_cfg.start_ch_freq =
				wlan_reg_ch_to_freq(CHAN_ENUM_2412);
		sap_cfg->acs_cfg.end_ch_freq =
				wlan_reg_ch_to_freq(CHAN_ENUM_5885);
	}

	if (ht_enabled)
		sap_cfg->acs_cfg.hw_mode = eCSR_DOT11_MODE_11n;

	if (vht_enabled)
		sap_cfg->acs_cfg.hw_mode = eCSR_DOT11_MODE_11ac;

	/* Parse ACS Chan list from hostapd */
	if (!sap_cfg->acs_cfg.freq_list)
		return -EINVAL;

	sap_cfg->acs_cfg.start_ch_freq = sap_cfg->acs_cfg.freq_list[0];
	sap_cfg->acs_cfg.end_ch_freq =
		sap_cfg->acs_cfg.freq_list[sap_cfg->acs_cfg.ch_list_count - 1];
	for (i = 0; i < sap_cfg->acs_cfg.ch_list_count; i++) {
		/* avoid channel as start channel */
		if (sap_cfg->acs_cfg.start_ch_freq >
		    sap_cfg->acs_cfg.freq_list[i] &&
		    sap_cfg->acs_cfg.freq_list[i] != 0)
			sap_cfg->acs_cfg.start_ch_freq =
			    sap_cfg->acs_cfg.freq_list[i];
		if (sap_cfg->acs_cfg.end_ch_freq <
				sap_cfg->acs_cfg.freq_list[i])
			sap_cfg->acs_cfg.end_ch_freq =
			    sap_cfg->acs_cfg.freq_list[i];
	}

	return 0;
}

static void hdd_update_acs_channel_list(struct sap_config *sap_config,
					enum band_info band)
{
	int i, temp_count = 0;
	int acs_list_count = sap_config->acs_cfg.ch_list_count;

	for (i = 0; i < acs_list_count; i++) {
		if (BAND_2G == band) {
			if (WLAN_REG_IS_24GHZ_CH_FREQ(
				sap_config->acs_cfg.freq_list[i])) {
				sap_config->acs_cfg.freq_list[temp_count] =
					sap_config->acs_cfg.freq_list[i];
				temp_count++;
			}
		} else if (BAND_5G == band) {
			if (WLAN_REG_IS_5GHZ_CH_FREQ(
				sap_config->acs_cfg.freq_list[i]) ||
			    WLAN_REG_IS_6GHZ_CHAN_FREQ(
				sap_config->acs_cfg.freq_list[i])) {
				sap_config->acs_cfg.freq_list[temp_count] =
					sap_config->acs_cfg.freq_list[i];
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
	struct sap_config *sap_config;
	sap_event_cb acs_event_callback;
	uint8_t mcc_to_scc_switch = 0;
	int status;
	QDF_STATUS qdf_status;

	if (!adapter) {
		hdd_err("adapter is NULL");
		return -EINVAL;
	}
	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	status = wlan_hdd_validate_context(hdd_ctx);
	if (0 != status)
		return status;

	sap_config = &adapter->session.ap.sap_config;
	if (!sap_config) {
		hdd_err("SAP config is NULL");
		return -EINVAL;
	}
	if (hdd_ctx->acs_policy.acs_chan_freq)
		sap_config->chan_freq = hdd_ctx->acs_policy.acs_chan_freq;
	else
		sap_config->chan_freq = AUTO_CHANNEL_SELECT;
	ucfg_policy_mgr_get_mcc_scc_switch(hdd_ctx->psoc,
					   &mcc_to_scc_switch);
	/*
	 * No DFS SCC is allowed in Auto use case. Hence not
	 * calling DFS override
	 */
	if (QDF_MCC_TO_SCC_SWITCH_FORCE_PREFERRED_WITHOUT_DISCONNECTION !=
	    mcc_to_scc_switch &&
	    !(policy_mgr_is_hw_dbs_capable(hdd_ctx->psoc) &&
	    WLAN_REG_IS_24GHZ_CH_FREQ(sap_config->acs_cfg.end_ch_freq)) &&
	    !wlansap_dcs_is_wlan_interference_mitigation_enabled(
					WLAN_HDD_GET_SAP_CTX_PTR(adapter))) {
		status = wlan_hdd_sap_cfg_dfs_override(adapter);
		if (status < 0)
			return status;

		if (status > 0) {
			/*notify hostapd about channel override */
			wlan_hdd_cfg80211_acs_ch_select_evt(adapter);
			wlansap_dcs_set_wlan_interference_mitigation_on_band(
						WLAN_HDD_GET_SAP_CTX_PTR(adapter),
						sap_config);
			return 0;
		}
	}
	/* When first 2 connections are on the same frequency band,
	 * then PCL would include only channels from the other
	 * frequency band on which no connections are active
	 */
	if ((policy_mgr_get_connection_count(hdd_ctx->psoc) == 2) &&
		(sap_config->acs_cfg.band == QCA_ACS_MODE_IEEE80211ANY)) {
		struct policy_mgr_conc_connection_info *conc_connection_info;
		uint32_t i;

		conc_connection_info = policy_mgr_get_conn_info(&i);
		if (policy_mgr_are_2_freq_on_same_mac(hdd_ctx->psoc,
			conc_connection_info[0].freq,
			conc_connection_info[1].freq)) {
			if (!WLAN_REG_IS_24GHZ_CH_FREQ(
				sap_config->acs_cfg.pcl_chan_freq[0])) {
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

	qdf_status = wlansap_acs_chselect(WLAN_HDD_GET_SAP_CTX_PTR(adapter),
				      acs_event_callback,
				      sap_config, adapter->dev);

	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		hdd_err("ACS channel select failed");
		return -EINVAL;
	}
	if (sap_is_auto_channel_select(WLAN_HDD_GET_SAP_CTX_PTR(adapter)))
		sap_config->acs_cfg.acs_mode = true;

	qdf_atomic_set(&adapter->session.ap.acs_in_progress, 1);

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
		struct sap_config *sap_config)
{
	int i, j;
	/*
	 * PCL shall contain only the preferred channels from the
	 * application. If those channels are not present in the
	 * driver PCL, then set the weight to zero
	 */
	for (i = 0; i < sap_config->acs_cfg.ch_list_count; i++) {
		acs_chan_params->vendor_pcl_list[i] =
				sap_config->acs_cfg.freq_list[i];
		acs_chan_params->vendor_weight_list[i] = 0;
		for (j = 0; j < sap_config->acs_cfg.pcl_ch_count; j++) {
			if (sap_config->acs_cfg.freq_list[i] ==
			    sap_config->acs_cfg.pcl_chan_freq[j]) {
				acs_chan_params->vendor_weight_list[i] =
				sap_config->acs_cfg.pcl_channels_weight_list[j];
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
 * @freq_list: channel frequency (MHz) list
 *
 * Return: Status of of channel information updation
 */
static int
hdd_update_reg_chan_info(struct hdd_adapter *adapter,
			 uint32_t channel_count, uint32_t *freq_list)
{
	int i;
	struct hdd_channel_info *icv;
	struct ch_params ch_params = {0};
	uint8_t bw_offset = 0, chan = 0;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct sap_config *sap_config = &adapter->session.ap.sap_config;
	mac_handle_t mac_handle;
	uint8_t sub_20_chan_width = 0;
	QDF_STATUS status;

	mac_handle = hdd_ctx->mac_handle;
	sap_config->channel_info_count = channel_count;

	status = ucfg_mlme_get_sub_20_chan_width(hdd_ctx->psoc,
						 &sub_20_chan_width);
	if (QDF_IS_STATUS_ERROR(status))
		hdd_err("Failed to get sub_20_chan_width config");

	for (i = 0; i < channel_count; i++) {
		icv = &sap_config->channel_info[i];
		chan = wlan_reg_freq_to_chan(hdd_ctx->pdev,
					     freq_list[i]);
		if (chan == 0)
			continue;

		icv->freq = freq_list[i];
		icv->ieee_chan_number = chan;
		icv->max_reg_power = wlan_reg_get_channel_reg_power_for_freq(
				hdd_ctx->pdev, freq_list[i]);

		/* filling demo values */
		icv->max_radio_power = HDD_MAX_TX_POWER;
		icv->min_radio_power = HDD_MIN_TX_POWER;
		/* not supported in current driver */
		icv->max_antenna_gain = 0;

		bw_offset = hdd_get_bw_offset(sap_config->acs_cfg.ch_width);
		icv->reg_class_id =
			wlan_hdd_find_opclass(mac_handle, chan, bw_offset);

		if (WLAN_REG_IS_5GHZ_CH_FREQ(freq_list[i])) {
			ch_params.ch_width = sap_config->acs_cfg.ch_width;
			wlan_reg_set_channel_params_for_freq(hdd_ctx->pdev,
							     icv->freq,
							     0, &ch_params);
			icv->vht_center_freq_seg0 = ch_params.center_freq_seg0;
			icv->vht_center_freq_seg1 = ch_params.center_freq_seg1;
		}

		icv->flags = 0;
		icv->flags = cds_get_vendor_reg_flags(hdd_ctx->pdev,
				icv->freq,
				sap_config->acs_cfg.ch_width,
				sap_config->acs_cfg.is_ht_enabled,
				sap_config->acs_cfg.is_vht_enabled,
				sub_20_chan_width);
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

#define CHAN_INFO_ATTR_FREQ_VHT_SEG_0 \
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FREQ_VHT_SEG_0
#define CHAN_INFO_ATTR_FREQ_VHT_SEG_1 \
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FREQ_VHT_SEG_1

/**
 * hdd_cfg80211_update_channel_info() - add channel info attributes
 * @hdd_ctx: pointer to hdd context
 * @skb: pointer to sk buff
 * @hdd_ctx: pointer to hdd station context
 * @idx: attribute index
 *
 * Return: Success(0) or reason code for failure
 */
static int32_t
hdd_cfg80211_update_channel_info(struct hdd_context *hdd_ctx,
				 struct sk_buff *skb,
				 struct sap_config *sap_config, int idx)
{
	struct nlattr *nla_attr, *channel;
	struct hdd_channel_info *icv;
	int i;
	uint32_t freq_seg_0 = 0, freq_seg_1 = 0;
	enum reg_wifi_band band;
	uint8_t band_mask;

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

		band = wlan_reg_freq_to_band(icv->freq);
		band_mask = 1 << band;

		if (icv->vht_center_freq_seg0)
			freq_seg_0 = wlan_reg_chan_band_to_freq(hdd_ctx->pdev,
						    icv->vht_center_freq_seg0,
						    band_mask);
		if (icv->vht_center_freq_seg1)
			freq_seg_1 = wlan_reg_chan_band_to_freq(hdd_ctx->pdev,
						    icv->vht_center_freq_seg1,
						    band_mask);

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
				icv->vht_center_freq_seg1) ||
		    nla_put_u32(skb, CHAN_INFO_ATTR_FREQ_VHT_SEG_0,
				freq_seg_0) ||
		    nla_put_u32(skb, CHAN_INFO_ATTR_FREQ_VHT_SEG_1,
				freq_seg_1)) {
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

#undef CHAN_INFO_ATTR_FREQ_VHT_SEG_0
#undef CHAN_INFO_ATTR_FREQ_VHT_SEG_1

/**
 * hdd_cfg80211_update_pcl() - add pcl info attributes
 * @hdd_ctx: pointer to hdd context
 * @skb: pointer to sk buff
 * @hdd_ctx: pointer to hdd station context
 * @idx: attribute index
 * @vendor_pcl_list: PCL list
 * @vendor_weight_list: PCL weights
 *
 * Return: Success(0) or reason code for failure
 */
static int32_t
hdd_cfg80211_update_pcl(struct hdd_context *hdd_ctx,
			struct sk_buff *skb,
			uint8_t ch_list_count, int idx,
			uint32_t *vendor_pcl_list, uint8_t *vendor_weight_list)
{
	struct nlattr *nla_attr, *channel;
	int i;
	uint8_t chan;

	nla_attr = nla_nest_start(skb, idx);
	if (!nla_attr)
		goto fail;

	for (i = 0; i < ch_list_count; i++) {
		channel = nla_nest_start(skb, i);
		if (!channel)
			goto fail;

		chan = (uint8_t)wlan_reg_freq_to_chan(hdd_ctx->pdev,
						      vendor_pcl_list[i]);

		if (nla_put_u8(skb, QCA_WLAN_VENDOR_ATTR_PCL_CHANNEL, chan) ||
		    nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_PCL_FREQ,
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
			      struct sap_config *sap_config,
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
}

/**
 * wlan_hdd_sap_get_valid_channellist() - Get SAPs valid channel list
 * @ap_adapter: adapter
 * @channel_count: valid channel count
 * @freq_list: valid channel frequency (MHz) list
 * @band: frequency band
 *
 * This API returns valid channel list for SAP after removing nol and
 * channel which lies outside of configuration.
 *
 * Return: Zero on success, non-zero on failure
 */
static int wlan_hdd_sap_get_valid_channellist(struct hdd_adapter *adapter,
					      uint32_t *channel_count,
					      uint32_t *freq_list,
					      enum band_info band)
{
	struct sap_config *sap_config;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	uint32_t pcl_freqs[NUM_CHANNELS] = {0};
	uint32_t chan_count;
	uint32_t i;
	QDF_STATUS status;
	struct wlan_objmgr_pdev *pdev = hdd_ctx->pdev;

	sap_config = &adapter->session.ap.sap_config;

	status = policy_mgr_get_valid_chans(hdd_ctx->psoc,
					    pcl_freqs,
					    &chan_count);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("Failed to get channel list");
		return -EINVAL;
	}

	*channel_count = 0;
	for (i = 0; i < chan_count; i++) {
		if (*channel_count >= NUM_CHANNELS)
			break;

		if (band == BAND_2G &&
		    WLAN_REG_IS_24GHZ_CH_FREQ(pcl_freqs[i]) &&
		    !wlan_reg_is_disable_for_freq(pdev, pcl_freqs[i])) {
			freq_list[*channel_count] = pcl_freqs[i];
			*channel_count += 1;
		} else if (band == BAND_5G &&
			   (WLAN_REG_IS_5GHZ_CH_FREQ(pcl_freqs[i]) ||
			    WLAN_REG_IS_6GHZ_CHAN_FREQ(pcl_freqs[i])) &&
			   !wlan_reg_is_disable_for_freq(pdev, pcl_freqs[i])) {
			freq_list[*channel_count] = pcl_freqs[i];
			*channel_count += 1;
		}
	}

	if (*channel_count == 0) {
		hdd_err("no valid channel found");
		return -EINVAL;
	}

	return 0;
}

/**
 * hdd_get_external_acs_event_len() - Get event buffer length for external ACS
 * @channel_count: number of channels for ACS operation
 *
 * Return: External ACS event (SUBCMD_UPDATE_EXTERNAL_ACS_CONFIG) buffer length.
 */
static int hdd_get_external_acs_event_len(uint32_t channel_count)
{
	uint32_t len = NLMSG_HDRLEN;
	uint32_t i;

	/* QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_REASON */
	len += nla_total_size(sizeof(u8));

	/* QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_IS_OFFLOAD_ENABLED */
	len += nla_total_size(sizeof(u32));

	/* QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_ADD_CHAN_STATS_SUPPORT */
	len += nla_total_size(sizeof(u32));

	/* QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_CHAN_WIDTH */
	len += nla_total_size(sizeof(u8));

	/* QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_BAND */
	len += nla_total_size(sizeof(u8));

	/* QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_PHY_MODE */
	len += nla_total_size(sizeof(u32));

	/* QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_FREQ_LIST */
	len += nla_total_size(channel_count * sizeof(u32));

	for (i = 0; i < channel_count; i++) {
		/* QCA_WLAN_VENDOR_ATTR_PCL_CHANNEL */
		len += nla_total_size(sizeof(u8));

		/* QCA_WLAN_VENDOR_ATTR_PCL_FREQ */
		len += nla_total_size(sizeof(u32));

		/* QCA_WLAN_VENDOR_ATTR_PCL_WEIGHT */
		len += nla_total_size(sizeof(u8));
	}

	for (i = 0; i < channel_count; i++) {
		/* QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FREQ */
		len += nla_total_size(sizeof(u16));

		/* QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FLAGS */
		len += nla_total_size(sizeof(u32));

		/* QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FLAG_EXT */
		len += nla_total_size(sizeof(u32));

		/* VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_MAX_REG_POWER */
		len += nla_total_size(sizeof(u8));

		/* VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_MAX_POWER */
		len += nla_total_size(sizeof(u8));

		/* VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_MIN_POWER */
		len += nla_total_size(sizeof(u8));

		/* VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_REG_CLASS_ID */
		len += nla_total_size(sizeof(u8));

		/* VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_ANTENNA_GAIN */
		len += nla_total_size(sizeof(u8));

		/* VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_VHT_SEG_0 */
		len += nla_total_size(sizeof(u8));

		/* VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_VHT_SEG_1 */
		len += nla_total_size(sizeof(u8));

		/* VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FREQ_VHT_SEG_0 */
		len += nla_total_size(sizeof(u32));

		/* VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FREQ_VHT_SEG_1 */
		len += nla_total_size(sizeof(u32));
	}

	/* QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_POLICY */
	len += nla_total_size(sizeof(u32));

	return len;
}

int hdd_cfg80211_update_acs_config(struct hdd_adapter *adapter,
				   uint8_t reason)
{
	struct sk_buff *skb = NULL;
	struct sap_config *sap_config;
	uint32_t channel_count = 0, status = -EINVAL;
	uint32_t *freq_list;
	uint32_t vendor_pcl_list[NUM_CHANNELS] = {0};
	uint8_t vendor_weight_list[NUM_CHANNELS] = {0};
	struct hdd_vendor_acs_chan_params acs_chan_params;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	enum band_info band = BAND_2G;
	eCsrPhyMode phy_mode;
	enum qca_wlan_vendor_attr_external_acs_policy acs_policy;
	uint32_t i, id;
	QDF_STATUS qdf_status;
	bool is_external_acs_policy = cfg_default(CFG_EXTERNAL_ACS_POLICY);
	uint32_t len;

	if (!hdd_ctx) {
		hdd_err("HDD context is NULL");
		return -EINVAL;
	}

	hdd_enter();
	sap_config = &adapter->session.ap.sap_config;
	/* When first 2 connections are on the same frequency band,
	 * then PCL would include only channels from the other
	 * frequency band on which no connections are active
	 */
	if ((policy_mgr_get_connection_count(hdd_ctx->psoc) == 2) &&
	    (sap_config->acs_cfg.band == QCA_ACS_MODE_IEEE80211ANY)) {
		struct policy_mgr_conc_connection_info	*conc_connection_info;

		conc_connection_info = policy_mgr_get_conn_info(&i);
		if (policy_mgr_are_2_freq_on_same_mac(hdd_ctx->psoc,
			conc_connection_info[0].freq,
			conc_connection_info[1].freq)) {
			if (!WLAN_REG_IS_24GHZ_CH_FREQ(
				sap_config->acs_cfg.pcl_chan_freq[0])) {
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

	freq_list = qdf_mem_malloc(sizeof(uint32_t) * NUM_CHANNELS);
	if (!freq_list)
		return -ENOMEM;

	if (sap_config->acs_cfg.freq_list) {
		/* Copy INI or hostapd provided ACS channel range*/
		for (i = 0; i < sap_config->acs_cfg.ch_list_count; i++)
			freq_list[i] = sap_config->acs_cfg.freq_list[i];
		channel_count = sap_config->acs_cfg.ch_list_count;
	} else {
		/* No channel list provided, copy all valid channels */
		wlan_hdd_sap_get_valid_channellist(adapter,
			&channel_count,
			freq_list,
			band);
	}

	sap_config->channel_info = qdf_mem_malloc(
					sizeof(struct hdd_channel_info) *
					channel_count);
	if (!sap_config->channel_info) {
		status = -ENOMEM;
		goto fail;
	}

	hdd_update_reg_chan_info(adapter, channel_count, freq_list);

	/* Get phymode */
	phy_mode = adapter->session.ap.sap_config.acs_cfg.hw_mode;

	len = hdd_get_external_acs_event_len(channel_count);
	id = QCA_NL80211_VENDOR_SUBCMD_UPDATE_EXTERNAL_ACS_CONFIG;
	skb = cfg80211_vendor_event_alloc(hdd_ctx->wiphy, &adapter->wdev,
					  len, id, GFP_KERNEL);
	if (!skb) {
		hdd_err("cfg80211_vendor_event_alloc failed");
		status = -ENOMEM;
		goto fail;
	}
	/*
	 * Application expects pcl to be a subset of channel list
	 * Remove all channels which are not in channel list from pcl
	 * and add weight as zero
	 */
	acs_chan_params.vendor_pcl_list = vendor_pcl_list;
	acs_chan_params.vendor_weight_list = vendor_weight_list;

	hdd_update_vendor_pcl_list(hdd_ctx, &acs_chan_params,
				   sap_config);

	if (acs_chan_params.pcl_count) {
		hdd_debug("ACS PCL list: len: %d",
			  acs_chan_params.pcl_count);
		for (i = 0; i < acs_chan_params.pcl_count; i++)
			hdd_debug("channel_frequency: %u, weight: %u",
				  acs_chan_params.
				  vendor_pcl_list[i],
				  acs_chan_params.
				  vendor_weight_list[i]);
	}

	qdf_status = ucfg_mlme_get_external_acs_policy(hdd_ctx->psoc,
						       &is_external_acs_policy);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status))
		hdd_err("get_external_acs_policy failed, set default");

	if (is_external_acs_policy) {
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
	hdd_cfg80211_update_pcl(hdd_ctx, skb,
				acs_chan_params.pcl_count,
				QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_PCL,
				vendor_pcl_list,
				vendor_weight_list);

	if (status != 0)
		goto fail;

	id = QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_CHAN_INFO;

	status = hdd_cfg80211_update_channel_info(hdd_ctx, skb, sap_config, id);
	if (status != 0)
		goto fail;

	status = nla_put_u32(skb,
			     QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_POLICY,
			     acs_policy);

	if (status != 0)
		goto fail;

	cfg80211_vendor_event(skb, GFP_KERNEL);
	qdf_mem_free(freq_list);
	qdf_mem_free(sap_config->channel_info);

	return 0;
fail:
	qdf_mem_free(freq_list);
	if (sap_config->channel_info)
		qdf_mem_free(sap_config->channel_info);
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
	if (!timer_context)
		return -ENOMEM;

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
wlan_hdd_cfg80211_do_acs_policy[QCA_WLAN_VENDOR_ATTR_ACS_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_ACS_HW_MODE] = { .type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_ACS_HT_ENABLED] = { .type = NLA_FLAG },
	[QCA_WLAN_VENDOR_ATTR_ACS_HT40_ENABLED] = { .type = NLA_FLAG },
	[QCA_WLAN_VENDOR_ATTR_ACS_VHT_ENABLED] = { .type = NLA_FLAG },
	[QCA_WLAN_VENDOR_ATTR_ACS_CHWIDTH] = { .type = NLA_U16 },
	[QCA_WLAN_VENDOR_ATTR_ACS_CH_LIST] = { .type = NLA_BINARY,
				.len = sizeof(NLA_U8) * NUM_CHANNELS },
	[QCA_WLAN_VENDOR_ATTR_ACS_FREQ_LIST] = { .type = NLA_BINARY,
				.len = sizeof(NLA_U32) * NUM_CHANNELS },
	[QCA_WLAN_VENDOR_ATTR_ACS_EHT_ENABLED] = { .type = NLA_FLAG },
	[QCA_WLAN_VENDOR_ATTR_ACS_PUNCTURE_BITMAP] = { .type = NLA_U16 },
	[QCA_WLAN_VENDOR_ATTR_ACS_EDMG_ENABLED] = { .type = NLA_FLAG },
	[QCA_WLAN_VENDOR_ATTR_ACS_EDMG_CHANNEL] = { .type = NLA_U8 },
};

int hdd_start_vendor_acs(struct hdd_adapter *adapter)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	int status;
	QDF_STATUS qdf_status;
	bool is_acs_support_for_dfs_ltecoex = cfg_default(CFG_USER_ACS_DFS_LTE);

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
	qdf_status = ucfg_mlme_get_acs_support_for_dfs_ltecoex(
				hdd_ctx->psoc,
				&is_acs_support_for_dfs_ltecoex);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status))
		hdd_err("get_acs_support_for_dfs_ltecoex failed, set def");

	if (is_acs_support_for_dfs_ltecoex)
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
 * hdd_avoid_acs_channels() - Avoid acs channels
 * @hdd_ctx: Pointer to the hdd context
 * @sap_config: Sap config structure pointer
 *
 * This function avoids channels from the acs corresponding to
 * the frequencies configured in the ini sap_avoid_acs_freq_list
 *
 * Return: None
 */

#ifdef SAP_AVOID_ACS_FREQ_LIST
static void hdd_avoid_acs_channels(struct hdd_context *hdd_ctx,
				   struct sap_config *sap_config)
{
	int i, j, ch_cnt = 0;
	uint16_t avoid_acs_freq_list[CFG_VALID_CHANNEL_LIST_LEN];
	uint8_t avoid_acs_freq_list_num;

	ucfg_mlme_get_acs_avoid_freq_list(hdd_ctx->psoc,
					  avoid_acs_freq_list,
					  &avoid_acs_freq_list_num);

	for (i = 0; i < sap_config->acs_cfg.ch_list_count; i++) {
		for (j = 0; j < avoid_acs_freq_list_num; j++) {
			if (sap_config->acs_cfg.freq_list[i] ==
						avoid_acs_freq_list[j]) {
				hdd_debug("skip freq %d",
					  sap_config->acs_cfg.freq_list[i]);
				break;
			}
		}
		if (j == avoid_acs_freq_list_num)
			sap_config->acs_cfg.freq_list[ch_cnt++] =
					sap_config->acs_cfg.freq_list[i];
	}
	sap_config->acs_cfg.ch_list_count = ch_cnt;
}
#else
static void hdd_avoid_acs_channels(struct hdd_context *hdd_ctx,
				   struct sap_config *sap_config)
{
}
#endif

/**
 * wlan_hdd_trim_acs_channel_list() - Trims ACS channel list with
 * intersection of PCL
 * @pcl: preferred channel list
 * @pcl_count: Preferred channel list count
 * @org_ch_list: ACS channel list from user space
 * @org_ch_list_count: ACS channel count from user space
 *
 * Return: None
 */
static void wlan_hdd_trim_acs_channel_list(uint32_t *pcl, uint8_t pcl_count,
					   uint32_t *org_freq_list,
					   uint8_t *org_ch_list_count)
{
	uint16_t i, j, ch_list_count = 0;

	if (*org_ch_list_count >= NUM_CHANNELS) {
		hdd_err("org_ch_list_count too big %d",
			*org_ch_list_count);
		return;
	}

	if (pcl_count >= NUM_CHANNELS) {
		hdd_err("pcl_count is too big %d", pcl_count);
		return;
	}

	hdd_debug("Update ACS chan freq with PCL");
	for (j = 0; j < *org_ch_list_count; j++)
		for (i = 0; i < pcl_count; i++)
			if (pcl[i] == org_freq_list[j]) {
				org_freq_list[ch_list_count++] = pcl[i];
				break;
			}

	*org_ch_list_count = ch_list_count;
}

/* wlan_hdd_dump_freq_list() - Dump the ACS master frequency list
 *
 * @freq_list: Frequency list
 * @num_freq: num of frequencies in list
 *
 * Dump the ACS master frequency list.
 */
static inline
void wlan_hdd_dump_freq_list(uint32_t *freq_list, uint8_t num_freq)
{
	uint32_t buf_len = 0;
	uint32_t i = 0, j = 0;
	uint8_t *master_chlist;

	if (num_freq >= NUM_CHANNELS)
		return;

	buf_len = NUM_CHANNELS * 4;
	master_chlist = qdf_mem_malloc(buf_len);

	if (!master_chlist)
		return;

	for (i = 0; i < num_freq && j < buf_len; i++) {
		j += qdf_scnprintf(master_chlist + j, buf_len - j,
				   "%d ", freq_list[i]);
	}

	hdd_debug("Master channel list: %s", master_chlist);
	qdf_mem_free(master_chlist);
}

/**
 * wlan_hdd_handle_zero_acs_list() - Handle worst case of acs channel
 * trimmed to zero
 * @hdd_ctx: struct hdd_context
 * @org_ch_list: ACS channel list from user space
 * @org_ch_list_count: ACS channel count from user space
 *
 * When all chan in ACS freq list is filtered out
 * by wlan_hdd_trim_acs_channel_list, the hostapd start will fail.
 * This happens when PCL is PM_24G_SCC_CH_SBS_CH, and SAP acs range includes
 * 5G channel list. One example is STA active on 6Ghz chan. Hostapd
 * start SAP on 5G ACS range. The intersection of PCL and ACS range is zero.
 * Instead of ACS failure, this API selects one channel from ACS range
 * and report to Hostapd. When hostapd do start_ap, the driver will
 * force SCC to 6G or move SAP to 2G based on SAP's configuration.
 *
 * Return: None
 */
static void wlan_hdd_handle_zero_acs_list(struct hdd_context *hdd_ctx,
					  uint32_t *acs_freq_list,
					  uint8_t *acs_ch_list_count,
					  uint32_t *org_freq_list,
					  uint8_t org_ch_list_count)
{
	uint16_t i, sta_count;
	uint32_t acs_chan_default = 0;
	bool force_sap_allowed = false;

	if (!acs_ch_list_count || *acs_ch_list_count > 0 ||
	    !acs_freq_list) {
		return;
	}
	if (!org_ch_list_count || !org_freq_list)
		return;

	if (!policy_mgr_is_force_scc(hdd_ctx->psoc))
		return;
	sta_count = policy_mgr_mode_specific_connection_count
			(hdd_ctx->psoc, PM_STA_MODE, NULL);
	sta_count += policy_mgr_mode_specific_connection_count
			(hdd_ctx->psoc, PM_P2P_CLIENT_MODE, NULL);

	ucfg_mlme_get_force_sap_enabled(hdd_ctx->psoc, &force_sap_allowed);
	if (!sta_count && !force_sap_allowed)
		return;

	wlan_hdd_dump_freq_list(org_freq_list, org_ch_list_count);

	for (i = 0; i < org_ch_list_count; i++) {
		if (wlan_reg_is_dfs_for_freq(hdd_ctx->pdev,
					     org_freq_list[i]))
			continue;

		if (wlan_reg_is_6ghz_chan_freq(org_freq_list[i]) &&
		    !wlan_reg_is_6ghz_psc_chan_freq(org_freq_list[i]))
			continue;

		if (!policy_mgr_is_safe_channel(hdd_ctx->psoc,
						org_freq_list[i]))
			continue;
		acs_chan_default = org_freq_list[i];
		break;
	}
	if (!acs_chan_default)
		acs_chan_default = org_freq_list[0];

	acs_freq_list[0] = acs_chan_default;
	*acs_ch_list_count = 1;
	hdd_debug("retore acs chan list to single freq %d", acs_chan_default);
}

/**
 * wlan_hdd_handle_single_ch_in_acs_list() - Handle acs list with single channel
 * @hdd_ctx: hdd context
 * @adapter: adapter
 * @sap_config: sap acs config context
 *
 * If only one acs channel is left after filter, driver will return the channel
 * to hostapd without ACS scan.
 *
 * Return: None
 */
static void
wlan_hdd_handle_single_ch_in_acs_list(struct hdd_context *hdd_ctx,
				      struct hdd_adapter *adapter,
				      struct sap_config *sap_config)
{
	uint32_t channel_bonding_mode_2g;

	ucfg_mlme_get_channel_bonding_24ghz(hdd_ctx->psoc,
					    &channel_bonding_mode_2g);
	sap_config->acs_cfg.start_ch_freq =
		sap_config->acs_cfg.freq_list[0];
	sap_config->acs_cfg.end_ch_freq =
		sap_config->acs_cfg.freq_list[0];
	sap_config->acs_cfg.pri_ch_freq =
			      sap_config->acs_cfg.freq_list[0];
	if (sap_config->acs_cfg.pri_ch_freq <=
	    WLAN_REG_CH_TO_FREQ(CHAN_ENUM_2484) &&
	    sap_config->acs_cfg.ch_width >=
				CH_WIDTH_40MHZ &&
	    !channel_bonding_mode_2g) {
		sap_config->acs_cfg.ch_width = CH_WIDTH_20MHZ;
		hdd_debug("2.4ghz channel resetting BW to %d 2.4 cbmode %d",
			  sap_config->acs_cfg.ch_width,
			  channel_bonding_mode_2g);
	}

	wlan_sap_set_sap_ctx_acs_cfg(
		WLAN_HDD_GET_SAP_CTX_PTR(adapter), sap_config);
	sap_config_acs_result(hdd_ctx->mac_handle,
			      WLAN_HDD_GET_SAP_CTX_PTR(adapter),
			    sap_config->acs_cfg.ht_sec_ch_freq);
	sap_config->ch_params.ch_width =
			sap_config->acs_cfg.ch_width;
	sap_config->ch_params.sec_ch_offset =
			wlan_reg_freq_to_chan(
			hdd_ctx->pdev,
			sap_config->acs_cfg.ht_sec_ch_freq);
	sap_config->ch_params.center_freq_seg0 =
	wlan_reg_freq_to_chan(
		hdd_ctx->pdev,
		sap_config->acs_cfg.vht_seg0_center_ch_freq);
	sap_config->ch_params.center_freq_seg1 =
	wlan_reg_freq_to_chan(
		hdd_ctx->pdev,
		sap_config->acs_cfg.vht_seg1_center_ch_freq);
	sap_config->ch_params.mhz_freq_seg0 =
		sap_config->acs_cfg.vht_seg0_center_ch_freq;
	sap_config->ch_params.mhz_freq_seg1 =
		sap_config->acs_cfg.vht_seg1_center_ch_freq;
	/*notify hostapd about channel override */
	wlan_hdd_cfg80211_acs_ch_select_evt(adapter);
	wlansap_dcs_set_wlan_interference_mitigation_on_band(
		WLAN_HDD_GET_SAP_CTX_PTR(adapter),
		sap_config);
}

#if defined(WLAN_FEATURE_11BE) && defined(CFG80211_11BE_BASIC)
static void wlan_hdd_set_sap_acs_ch_width_320(struct sap_config *sap_config)
{
	sap_config->acs_cfg.ch_width = CH_WIDTH_320MHZ;
}

static bool wlan_hdd_is_sap_acs_ch_width_320(struct sap_config *sap_config)
{
	return sap_config->acs_cfg.ch_width == CH_WIDTH_320MHZ;
}

static void wlan_hdd_set_chandef(struct wlan_objmgr_vdev *vdev,
				 struct cfg80211_chan_def *chandef)
{
	if (vdev->vdev_mlme.des_chan->ch_width != CH_WIDTH_320MHZ)
		return;

	chandef->width = NL80211_CHAN_WIDTH_320;
	/* Set center_freq1 to center frequency of complete 320MHz */
	chandef->center_freq1 = vdev->vdev_mlme.des_chan->ch_cfreq2;
}
#else /* !WLAN_FEATURE_11BE */
static inline
void wlan_hdd_set_sap_acs_ch_width_320(struct sap_config *sap_config)
{
}

static inline
bool wlan_hdd_is_sap_acs_ch_width_320(struct sap_config *sap_config)
{
	return false;
}

static inline void wlan_hdd_set_chandef(struct wlan_objmgr_vdev *vdev,
					struct cfg80211_chan_def *chandef)
{
}
#endif /* WLAN_FEATURE_11BE */

#ifdef WLAN_FEATURE_11BE
/**
 * wlan_hdd_acs_set_eht_enabled() - set is_eht_enabled of acs config
 * @sap_config: pointer to sap_config
 * @eht_enabled: eht is enabled
 *
 * Return: void
 */
static void wlan_hdd_acs_set_eht_enabled(struct sap_config *sap_config,
					 bool eht_enabled)
{
	if (!sap_config) {
		hdd_err("Invalid sap_config");
		return;
	}

	sap_config->acs_cfg.is_eht_enabled = eht_enabled;
}
#else
static void wlan_hdd_acs_set_eht_enabled(struct sap_config *sap_config,
					 bool eht_enabled)
{
}
#endif /* WLAN_FEATURE_11BE */

static uint16_t wlan_hdd_update_bw_from_mlme(struct hdd_context *hdd_ctx,
					     struct sap_config *sap_config)
{
	uint16_t ch_width, temp_ch_width = 0;
	QDF_STATUS status;
	uint8_t hw_mode = HW_MODE_DBS;
	struct wma_caps_per_phy caps_per_phy = {0};

	ch_width = sap_config->acs_cfg.ch_width;

	if (ch_width > CH_WIDTH_80P80MHZ)
		return ch_width;

	/* 2.4ghz is already handled for acs */
	if (sap_config->acs_cfg.end_ch_freq <=
	    WLAN_REG_CH_TO_FREQ(CHAN_ENUM_2484))
		return ch_width;

	if (!policy_mgr_is_dbs_enable(hdd_ctx->psoc))
		hw_mode = HW_MODE_DBS_NONE;

	status = wma_get_caps_for_phyidx_hwmode(&caps_per_phy, hw_mode,
						CDS_BAND_5GHZ);
	if (!QDF_IS_STATUS_SUCCESS(status))
		return ch_width;

	switch (ch_width) {
	case CH_WIDTH_80P80MHZ:
		if (!(caps_per_phy.vht_5g & WMI_VHT_CAP_CH_WIDTH_80P80_160MHZ))
		{
			if (caps_per_phy.vht_5g & WMI_VHT_CAP_CH_WIDTH_160MHZ)
				temp_ch_width = CH_WIDTH_160MHZ;
			else
				temp_ch_width = CH_WIDTH_80MHZ;
		}
		break;
	case CH_WIDTH_160MHZ:
		if (!((caps_per_phy.vht_5g & WMI_VHT_CAP_CH_WIDTH_80P80_160MHZ)
		      || (caps_per_phy.vht_5g & WMI_VHT_CAP_CH_WIDTH_160MHZ)))
				temp_ch_width = CH_WIDTH_80MHZ;
		break;
	default:
		break;
	}

	if (!temp_ch_width)
		return ch_width;

	hdd_debug("ch_width updated from %d to %d vht_5g: %x", ch_width,
		  temp_ch_width, caps_per_phy.vht_5g);
	return temp_ch_width;
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
	struct sap_config *sap_config;
	struct sk_buff *temp_skbuff;
	int ret, i;
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_ACS_MAX + 1];
	bool ht_enabled, ht40_enabled, vht_enabled, eht_enabled;
	uint16_t ch_width;
	enum qca_wlan_vendor_acs_hw_mode hw_mode;
	enum policy_mgr_con_mode pm_mode;
	QDF_STATUS qdf_status;
	bool is_vendor_acs_support = false;
	bool is_external_acs_policy = false;
	bool is_vendor_unsafe_ch_present = false;
	bool sap_force_11n_for_11ac = 0;
	bool go_force_11n_for_11ac = 0;
	bool go_11ac_override = 0;
	bool sap_11ac_override = 0;
	uint8_t vht_ch_width;
	uint32_t channel_bonding_mode_2g;

	/* ***Note*** Donot set SME config related to ACS operation here because
	 * ACS operation is not synchronouse and ACS for Second AP may come when
	 * ACS operation for first AP is going on. So only do_acs is split to
	 * separate start_acs routine. Also SME-PMAC struct that is used to
	 * pass paremeters from HDD to SAP is global. Thus All ACS related SME
	 * config shall be set only from start_acs.
	 */

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	ucfg_mlme_get_sap_force_11n_for_11ac(hdd_ctx->psoc,
					     &sap_force_11n_for_11ac);
	ucfg_mlme_get_go_force_11n_for_11ac(hdd_ctx->psoc,
					    &go_force_11n_for_11ac);
	ucfg_mlme_get_channel_bonding_24ghz(hdd_ctx->psoc,
					    &channel_bonding_mode_2g);

	if (!((adapter->device_mode == QDF_SAP_MODE) ||
	      (adapter->device_mode == QDF_P2P_GO_MODE))) {
		hdd_err("Invalid device mode %d", adapter->device_mode);
		return -EINVAL;
	}

	if (cds_is_sub_20_mhz_enabled()) {
		hdd_err("ACS not supported in sub 20 MHz ch wd.");
		return -EINVAL;
	}

	if (qdf_atomic_read(&adapter->session.ap.acs_in_progress) > 0) {
		hdd_err("ACS rejected as previous req already in progress");
		return -EINVAL;
	} else {
		qdf_atomic_set(&adapter->session.ap.acs_in_progress, 1);
		qdf_event_reset(&adapter->acs_complete_event);
	}

	hdd_reg_wait_for_country_change(hdd_ctx);

	ret = wlan_cfg80211_nla_parse(tb, QCA_WLAN_VENDOR_ATTR_ACS_MAX, data,
					 data_len,
					 wlan_hdd_cfg80211_do_acs_policy);
	if (ret) {
		hdd_err("Invalid ATTR");
		goto out;
	}

	if (!tb[QCA_WLAN_VENDOR_ATTR_ACS_HW_MODE]) {
		hdd_err("Attr hw_mode failed");
		ret = -EINVAL;
		goto out;
	}
	hw_mode = nla_get_u8(tb[QCA_WLAN_VENDOR_ATTR_ACS_HW_MODE]);

	hdd_nofl_info("ACS request vid %d hw mode %d", adapter->vdev_id,
		      hw_mode);
	ht_enabled = nla_get_flag(tb[QCA_WLAN_VENDOR_ATTR_ACS_HT_ENABLED]);
	ht40_enabled = nla_get_flag(tb[QCA_WLAN_VENDOR_ATTR_ACS_HT40_ENABLED]);
	vht_enabled = nla_get_flag(tb[QCA_WLAN_VENDOR_ATTR_ACS_VHT_ENABLED]);
	eht_enabled = nla_get_flag(tb[QCA_WLAN_VENDOR_ATTR_ACS_EHT_ENABLED]);

	if (((adapter->device_mode == QDF_SAP_MODE) &&
	      sap_force_11n_for_11ac) ||
	    ((adapter->device_mode == QDF_P2P_GO_MODE) &&
	      go_force_11n_for_11ac)) {
		vht_enabled = 0;
		hdd_info("VHT is Disabled in ACS");
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_ACS_CHWIDTH]) {
		ch_width = nla_get_u16(tb[QCA_WLAN_VENDOR_ATTR_ACS_CHWIDTH]);
	} else {
		if (ht_enabled && ht40_enabled)
			ch_width = 40;
		else
			ch_width = 20;
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_ACS_EHT_ENABLED])
		eht_enabled =
			nla_get_flag(tb[QCA_WLAN_VENDOR_ATTR_ACS_EHT_ENABLED]);
	else
		eht_enabled = 0;

	if (ch_width == 320 && !eht_enabled)
		ch_width = 160;

	/* this may be possible, when sap_force_11n_for_11ac or
	 * go_force_11n_for_11ac is set
	 */
	if ((ch_width == 80 || ch_width == 160) && !vht_enabled) {
		if (ht_enabled && ht40_enabled)
			ch_width = 40;
		else
			ch_width = 20;
	}

	sap_config = &adapter->session.ap.sap_config;

	/* Check and free if memory is already allocated for acs channel list */
	wlan_hdd_undo_acs(adapter);

	qdf_mem_zero(&sap_config->acs_cfg, sizeof(struct sap_acs_cfg));

	if (ch_width == 320)
		wlan_hdd_set_sap_acs_ch_width_320(sap_config);
	else if (ch_width == 160)
		sap_config->acs_cfg.ch_width = CH_WIDTH_160MHZ;
	else if (ch_width == 80)
		sap_config->acs_cfg.ch_width = CH_WIDTH_80MHZ;
	else if (ch_width == 40)
		sap_config->acs_cfg.ch_width = CH_WIDTH_40MHZ;
	else
		sap_config->acs_cfg.ch_width = CH_WIDTH_20MHZ;

	/* Firstly try to get channel frequencies */
	if (tb[QCA_WLAN_VENDOR_ATTR_ACS_FREQ_LIST]) {
		uint32_t *freq =
			nla_data(tb[QCA_WLAN_VENDOR_ATTR_ACS_FREQ_LIST]);
		sap_config->acs_cfg.ch_list_count = nla_len(
			tb[QCA_WLAN_VENDOR_ATTR_ACS_FREQ_LIST]) /
				sizeof(uint32_t);
		if (sap_config->acs_cfg.ch_list_count) {
			sap_config->acs_cfg.freq_list = qdf_mem_malloc(
				sap_config->acs_cfg.ch_list_count *
				sizeof(uint32_t));
			sap_config->acs_cfg.master_freq_list = qdf_mem_malloc(
				sap_config->acs_cfg.ch_list_count *
				sizeof(uint32_t));
			if (!sap_config->acs_cfg.freq_list ||
			    !sap_config->acs_cfg.master_freq_list) {
				ret = -ENOMEM;
				goto out;
			}

			for (i = 0; i < sap_config->acs_cfg.ch_list_count;
			     i++) {
				sap_config->acs_cfg.master_freq_list[i] =
									freq[i];
				sap_config->acs_cfg.freq_list[i] = freq[i];
			}
			sap_config->acs_cfg.master_ch_list_count =
					sap_config->acs_cfg.ch_list_count;
		}
	} else if (tb[QCA_WLAN_VENDOR_ATTR_ACS_CH_LIST]) {
		uint8_t *tmp = nla_data(tb[QCA_WLAN_VENDOR_ATTR_ACS_CH_LIST]);

		sap_config->acs_cfg.ch_list_count = nla_len(
					tb[QCA_WLAN_VENDOR_ATTR_ACS_CH_LIST]);
		if (sap_config->acs_cfg.ch_list_count) {
			sap_config->acs_cfg.freq_list = qdf_mem_malloc(
					sap_config->acs_cfg.ch_list_count *
					sizeof(uint32_t));
			sap_config->acs_cfg.master_freq_list = qdf_mem_malloc(
					sap_config->acs_cfg.ch_list_count *
					sizeof(uint32_t));
			if (!sap_config->acs_cfg.freq_list ||
			    !sap_config->acs_cfg.master_freq_list) {
				ret = -ENOMEM;
				goto out;
			}

			/* convert channel to frequency */
			for (i = 0; i < sap_config->acs_cfg.ch_list_count;
			     i++) {
				sap_config->acs_cfg.freq_list[i] =
					wlan_reg_legacy_chan_to_freq(
								hdd_ctx->pdev,
								tmp[i]);
				sap_config->acs_cfg.master_freq_list[i] =
					sap_config->acs_cfg.freq_list[i];
			}
			sap_config->acs_cfg.master_ch_list_count =
					sap_config->acs_cfg.ch_list_count;
		}
	}

	if (!sap_config->acs_cfg.ch_list_count) {
		hdd_err("acs config chan count 0");
		ret = -EINVAL;
		goto out;
	}

	hdd_avoid_acs_channels(hdd_ctx, sap_config);

	pm_mode =
	      policy_mgr_convert_device_mode_to_qdf_type(adapter->device_mode);
	/* consult policy manager to get PCL */
	qdf_status = policy_mgr_get_pcl(hdd_ctx->psoc, pm_mode,
					sap_config->acs_cfg.pcl_chan_freq,
					&sap_config->acs_cfg.pcl_ch_count,
					sap_config->acs_cfg.
					pcl_channels_weight_list,
					NUM_CHANNELS);

	sap_config->acs_cfg.band = hw_mode;

	qdf_status = ucfg_mlme_get_external_acs_policy(hdd_ctx->psoc,
						       &is_external_acs_policy);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status))
		hdd_err("get_external_acs_policy failed");

	sap_config->acs_cfg.acs_mode = true;

	if (is_external_acs_policy &&
	    policy_mgr_is_force_scc(hdd_ctx->psoc) &&
	    policy_mgr_get_connection_count(hdd_ctx->psoc)) {
		if (adapter->device_mode == QDF_SAP_MODE)
			is_vendor_unsafe_ch_present =
			wlansap_filter_vendor_unsafe_ch_freq(
					WLAN_HDD_GET_SAP_CTX_PTR(adapter),
					sap_config);
		wlan_hdd_trim_acs_channel_list(
					sap_config->acs_cfg.pcl_chan_freq,
					sap_config->acs_cfg.pcl_ch_count,
					sap_config->acs_cfg.freq_list,
					&sap_config->acs_cfg.ch_list_count);
		if (!sap_config->acs_cfg.ch_list_count &&
		    sap_config->acs_cfg.master_ch_list_count &&
		    !is_vendor_unsafe_ch_present)
			wlan_hdd_handle_zero_acs_list(
				hdd_ctx,
				sap_config->acs_cfg.freq_list,
				&sap_config->acs_cfg.ch_list_count,
				sap_config->acs_cfg.master_freq_list,
				sap_config->acs_cfg.master_ch_list_count);
		/* if it is only one channel, send ACS event to upper layer */
		if (sap_config->acs_cfg.ch_list_count == 1) {
			wlan_hdd_handle_single_ch_in_acs_list(
					hdd_ctx, adapter, sap_config);
			ret = 0;
			goto out;
		} else if (!sap_config->acs_cfg.ch_list_count) {
			hdd_err("channel list count 0");
			ret = -EINVAL;
			goto out;
		}
	} else if (adapter->device_mode == QDF_SAP_MODE) {
		wlansap_filter_vendor_unsafe_ch_freq(
					WLAN_HDD_GET_SAP_CTX_PTR(adapter),
					sap_config);
		if (sap_config->acs_cfg.ch_list_count == 1) {
			wlan_hdd_handle_single_ch_in_acs_list(
					hdd_ctx, adapter, sap_config);
			ret = 0;
			goto out;
		} else if (!sap_config->acs_cfg.ch_list_count) {
			hdd_err("channel count 0 after vendor unsafe filter");
			ret = -EINVAL;
			goto out;
		}
	}

	ret = wlan_hdd_set_acs_ch_range(sap_config, hw_mode,
					ht_enabled, vht_enabled);
	if (ret) {
		hdd_err("set acs channel range failed");
		goto out;
	}

	ucfg_mlme_is_go_11ac_override(hdd_ctx->psoc, &go_11ac_override);
	ucfg_mlme_is_sap_11ac_override(hdd_ctx->psoc, &sap_11ac_override);
	/* ACS override for android */
	if (ht_enabled &&
	    sap_config->acs_cfg.end_ch_freq >=
		WLAN_REG_CH_TO_FREQ(CHAN_ENUM_5180) &&
	    ((adapter->device_mode == QDF_SAP_MODE &&
	      !sap_force_11n_for_11ac &&
	      sap_11ac_override) ||
	      (adapter->device_mode == QDF_P2P_GO_MODE &&
	      !go_force_11n_for_11ac &&
	      go_11ac_override))) {
		vht_enabled = 1;
		sap_config->acs_cfg.hw_mode = eCSR_DOT11_MODE_11ac;
		qdf_status =
			ucfg_mlme_get_vht_channel_width(hdd_ctx->psoc,
							&vht_ch_width);
		ch_width = vht_ch_width;
		sap_config->acs_cfg.ch_width = ch_width;
	}

	/* Check 2.4ghz cbmode and update BW if only 2.4 channels are present */
	if (sap_config->acs_cfg.end_ch_freq <=
	    WLAN_REG_CH_TO_FREQ(CHAN_ENUM_2484) &&
	    sap_config->acs_cfg.ch_width >= eHT_CHANNEL_WIDTH_40MHZ) {

		sap_config->acs_cfg.ch_width = channel_bonding_mode_2g ?
			eHT_CHANNEL_WIDTH_40MHZ : eHT_CHANNEL_WIDTH_20MHZ;

		hdd_debug("Only 2.4ghz channels, resetting BW to %d 2.4 cbmode %d",
			  sap_config->acs_cfg.ch_width,
			  channel_bonding_mode_2g);
	}

	sap_config->acs_cfg.ch_width = wlan_hdd_update_bw_from_mlme(hdd_ctx,
								    sap_config);

	hdd_nofl_debug("ACS Config country %s ch_width %d hw_mode %d ACS_BW: %d HT: %d VHT: %d EHT: %d START_CH: %d END_CH: %d band %d",
		       hdd_ctx->reg.alpha2, ch_width,
		       sap_config->acs_cfg.hw_mode, sap_config->acs_cfg.ch_width,
		       ht_enabled, vht_enabled, eht_enabled,
		       sap_config->acs_cfg.start_ch_freq,
		       sap_config->acs_cfg.end_ch_freq,
		       sap_config->acs_cfg.band);
	host_log_acs_req_event(adapter->dev->name,
			  csr_phy_mode_str(sap_config->acs_cfg.hw_mode),
			  ch_width, ht_enabled, vht_enabled,
			  sap_config->acs_cfg.start_ch_freq,
			  sap_config->acs_cfg.end_ch_freq);

	sap_config->acs_cfg.is_ht_enabled = ht_enabled;
	sap_config->acs_cfg.is_vht_enabled = vht_enabled;
	wlan_hdd_acs_set_eht_enabled(sap_config, eht_enabled);
	sap_dump_acs_channel(&sap_config->acs_cfg);

	qdf_status = ucfg_mlme_get_vendor_acs_support(hdd_ctx->psoc,
						&is_vendor_acs_support);
	if (QDF_IS_STATUS_ERROR(qdf_status))
		hdd_err("get_vendor_acs_support failed, set default");

	/* Check if vendor specific acs is enabled */
	if (is_vendor_acs_support)
		ret = hdd_start_vendor_acs(adapter);
	else
		ret = wlan_hdd_cfg80211_start_acs(adapter);

out:
	if (ret == 0) {
		temp_skbuff = cfg80211_vendor_cmd_alloc_reply_skb(wiphy,
							      NLMSG_HDRLEN);
		if (temp_skbuff)
			return cfg80211_vendor_cmd_reply(temp_skbuff);
	}
	qdf_atomic_set(&adapter->session.ap.acs_in_progress, 0);

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
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_do_acs(wiphy, wdev, data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
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
	sap_undo_acs(WLAN_HDD_GET_SAP_CTX_PTR(adapter),
		     &adapter->session.ap.sap_config);
}

/**
 * hdd_fill_acs_chan_freq() - Populate channel frequencies (MHz) selected in ACS
 * @hdd_ctx: pointer to hdd context
 * @sap_cfg: sap acs configuration
 * @vendor_event: output pointer to populate channel frequencies (MHz)
 *
 * Return: If populated successfully return 0 else negative value.
 */
static int hdd_fill_acs_chan_freq(struct hdd_context *hdd_ctx,
				  struct sap_config *sap_cfg,
				  struct sk_buff *vendor_event)
{
	uint32_t id;
	int errno;

	id = QCA_WLAN_VENDOR_ATTR_ACS_PRIMARY_FREQUENCY;
	errno = nla_put_u32(vendor_event, id, sap_cfg->acs_cfg.pri_ch_freq);
	if (errno) {
		hdd_err("VENDOR_ATTR_ACS_PRIMARY_FREQUENCY put fail");
		return errno;
	}

	id = QCA_WLAN_VENDOR_ATTR_ACS_SECONDARY_FREQUENCY;
	errno = nla_put_u32(vendor_event, id, sap_cfg->acs_cfg.ht_sec_ch_freq);
	if (errno) {
		hdd_err("VENDOR_ATTR_ACS_SECONDARY_FREQUENCY put fail");
		return errno;
	}

	id = QCA_WLAN_VENDOR_ATTR_ACS_VHT_SEG0_CENTER_FREQUENCY;
	errno = nla_put_u32(vendor_event, id,
			    sap_cfg->acs_cfg.vht_seg0_center_ch_freq);
	if (errno) {
		hdd_err("VENDOR_ATTR_ACS_VHT_SEG0_CENTER_FREQUENCY put fail");
		return errno;
	}

	id = QCA_WLAN_VENDOR_ATTR_ACS_VHT_SEG1_CENTER_FREQUENCY;
	errno = nla_put_u32(vendor_event, id,
			    sap_cfg->acs_cfg.vht_seg1_center_ch_freq);
	if (errno) {
		hdd_err("VENDOR_ATTR_ACS_VHT_SEG1_CENTER_FREQUENCY put fail");
		return errno;
	}

	return 0;
}

static int hdd_get_acs_evt_data_len(struct sap_config *sap_cfg)
{
	uint32_t len = NLMSG_HDRLEN;

	/* QCA_WLAN_VENDOR_ATTR_ACS_PRIMARY_CHANNEL */
	len += nla_total_size(sizeof(u8));

	/* QCA_WLAN_VENDOR_ATTR_ACS_SECONDARY_CHANNEL */
	len += nla_total_size(sizeof(u8));

	/* QCA_WLAN_VENDOR_ATTR_ACS_VHT_SEG0_CENTER_CHANNEL */
	len += nla_total_size(sizeof(u8));

	/* QCA_WLAN_VENDOR_ATTR_ACS_VHT_SEG1_CENTER_CHANNEL */
	len += nla_total_size(sizeof(u8));

	/* QCA_WLAN_VENDOR_ATTR_ACS_PRIMARY_FREQUENCY */
	len += nla_total_size(sizeof(u32));

	/* QCA_WLAN_VENDOR_ATTR_ACS_SECONDARY_FREQUENCY */
	len += nla_total_size(sizeof(u32));

	/* QCA_WLAN_VENDOR_ATTR_ACS_VHT_SEG0_CENTER_FREQUENCY */
	len += nla_total_size(sizeof(u32));

	/* QCA_WLAN_VENDOR_ATTR_ACS_VHT_SEG1_CENTER_FREQUENCY */
	len += nla_total_size(sizeof(u32));

	/* QCA_WLAN_VENDOR_ATTR_ACS_CHWIDTH */
	len += nla_total_size(sizeof(u16));

	/* QCA_WLAN_VENDOR_ATTR_ACS_HW_MODE */
	len += nla_total_size(sizeof(u8));

	/* QCA_WLAN_VENDOR_ATTR_ACS_PUNCTURE_BITMAP */
	if (sap_acs_is_puncture_applicable(&sap_cfg->acs_cfg))
		len += nla_total_size(sizeof(u16));

	return len;
}

#ifdef WLAN_FEATURE_11BE
/**
 * wlan_hdd_acs_get_puncture_bitmap() - get puncture_bitmap for acs result
 * @acs_cfg: pointer to struct sap_acs_cfg
 *
 * Return: acs puncture bitmap
 */
static uint16_t wlan_hdd_acs_get_puncture_bitmap(struct sap_acs_cfg *acs_cfg)
{
	if (sap_acs_is_puncture_applicable(acs_cfg))
		return acs_cfg->acs_puncture_bitmap;

	return 0;
}
#else
static uint16_t wlan_hdd_acs_get_puncture_bitmap(struct sap_acs_cfg *acs_cfg)
{
	return 0;
}
#endif /* WLAN_FEATURE_11BE */

/**
 * wlan_hdd_cfg80211_acs_ch_select_evt: Callback function for ACS evt
 * @adapter: Pointer to SAP adapter struct
 * @pri_channel: SAP ACS procedure selected Primary channel
 * @sec_channel: SAP ACS procedure selected secondary channel
 *
 * This is a callback function on ACS procedure is completed.
 * This function send the ACS selected channel information to hostapd
 *
 * Return: None
 */
void wlan_hdd_cfg80211_acs_ch_select_evt(struct hdd_adapter *adapter)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct sap_config *sap_cfg =
				&(WLAN_HDD_GET_AP_CTX_PTR(adapter))->sap_config;
	struct sk_buff *vendor_event;
	int ret_val;
	uint16_t ch_width;
	uint8_t pri_channel;
	uint8_t ht_sec_channel;
	uint8_t vht_seg0_center_ch, vht_seg1_center_ch;
	uint32_t id = QCA_NL80211_VENDOR_SUBCMD_DO_ACS_INDEX;
	uint32_t len = hdd_get_acs_evt_data_len(sap_cfg);
	uint16_t puncture_bitmap;

	qdf_atomic_set(&adapter->session.ap.acs_in_progress, 0);
	qdf_event_set(&adapter->acs_complete_event);

	vendor_event = cfg80211_vendor_event_alloc(hdd_ctx->wiphy,
						   &adapter->wdev, len, id,
						   GFP_KERNEL);

	if (!vendor_event) {
		hdd_err("cfg80211_vendor_event_alloc failed");
		return;
	}

	ret_val = hdd_fill_acs_chan_freq(hdd_ctx, sap_cfg, vendor_event);
	if (ret_val) {
		hdd_err("failed to put frequencies");
		kfree_skb(vendor_event);
		return;
	}

	pri_channel = wlan_reg_freq_to_chan(hdd_ctx->pdev,
					    sap_cfg->acs_cfg.pri_ch_freq);

	ht_sec_channel = wlan_reg_freq_to_chan(hdd_ctx->pdev,
					       sap_cfg->acs_cfg.ht_sec_ch_freq);

	ret_val = nla_put_u8(vendor_event,
				QCA_WLAN_VENDOR_ATTR_ACS_PRIMARY_CHANNEL,
				pri_channel);
	if (ret_val) {
		hdd_err("QCA_WLAN_VENDOR_ATTR_ACS_PRIMARY_CHANNEL put fail");
		kfree_skb(vendor_event);
		return;
	}

	ret_val = nla_put_u8(vendor_event,
			     QCA_WLAN_VENDOR_ATTR_ACS_SECONDARY_CHANNEL,
			     ht_sec_channel);
	if (ret_val) {
		hdd_err("QCA_WLAN_VENDOR_ATTR_ACS_SECONDARY_CHANNEL put fail");
		kfree_skb(vendor_event);
		return;
	}
	vht_seg0_center_ch = wlan_reg_freq_to_chan(
				hdd_ctx->pdev,
				sap_cfg->acs_cfg.vht_seg0_center_ch_freq);
	ret_val = nla_put_u8(vendor_event,
			QCA_WLAN_VENDOR_ATTR_ACS_VHT_SEG0_CENTER_CHANNEL,
			vht_seg0_center_ch);
	if (ret_val) {
		hdd_err("QCA_WLAN_VENDOR_ATTR_ACS_VHT_SEG0_CENTER_CHANNEL put fail");
		kfree_skb(vendor_event);
		return;
	}
	vht_seg1_center_ch = wlan_reg_freq_to_chan(
				hdd_ctx->pdev,
				sap_cfg->acs_cfg.vht_seg1_center_ch_freq);
	ret_val = nla_put_u8(vendor_event,
			QCA_WLAN_VENDOR_ATTR_ACS_VHT_SEG1_CENTER_CHANNEL,
			vht_seg1_center_ch);
	if (ret_val) {
		hdd_err("QCA_WLAN_VENDOR_ATTR_ACS_VHT_SEG1_CENTER_CHANNEL put fail");
		kfree_skb(vendor_event);
		return;
	}

	if (wlan_hdd_is_sap_acs_ch_width_320(sap_cfg))
		ch_width = 320;
	else if (sap_cfg->acs_cfg.ch_width == CH_WIDTH_160MHZ)
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
	if (WLAN_REG_IS_24GHZ_CH_FREQ(sap_cfg->acs_cfg.pri_ch_freq))
		ret_val = nla_put_u8(vendor_event,
					QCA_WLAN_VENDOR_ATTR_ACS_HW_MODE,
					QCA_ACS_MODE_IEEE80211G);
	else
		ret_val = nla_put_u8(vendor_event,
					QCA_WLAN_VENDOR_ATTR_ACS_HW_MODE,
					QCA_ACS_MODE_IEEE80211A);

	if (ret_val) {
		hdd_err("QCA_WLAN_VENDOR_ATTR_ACS_HW_MODE put fail");
		kfree_skb(vendor_event);
		return;
	}

	puncture_bitmap = wlan_hdd_acs_get_puncture_bitmap(&sap_cfg->acs_cfg);
	if (sap_acs_is_puncture_applicable(&sap_cfg->acs_cfg)) {
		ret_val = nla_put_u16(vendor_event,
				      QCA_WLAN_VENDOR_ATTR_ACS_PUNCTURE_BITMAP,
				      puncture_bitmap);
		if (ret_val) {
			hdd_err("QCA_WLAN_VENDOR_ATTR_ACS_PUNCTURE_BITMAP put fail");
			kfree_skb(vendor_event);
			return;
		}
	}

	hdd_debug("ACS result for %s: PRI_CH_FREQ: %d SEC_CH_FREQ: %d VHT_SEG0: %d VHT_SEG1: %d ACS_BW: %d punc support: %d punc bitmap: %d",
		  adapter->dev->name, sap_cfg->acs_cfg.pri_ch_freq,
		  sap_cfg->acs_cfg.ht_sec_ch_freq,
		  sap_cfg->acs_cfg.vht_seg0_center_ch_freq,
		  sap_cfg->acs_cfg.vht_seg1_center_ch_freq, ch_width,
		  sap_acs_is_puncture_applicable(&sap_cfg->acs_cfg),
		  puncture_bitmap);

	cfg80211_vendor_event(vendor_event, GFP_KERNEL);
}

/**
 * hdd_is_wlm_latency_manager_supported - Checks if WLM Latency manager is
 *                                        supported
 * @hdd_ctx: The HDD context
 *
 * Return: True if supported, false otherwise
 */
static inline
bool hdd_is_wlm_latency_manager_supported(struct hdd_context *hdd_ctx)
{
	bool latency_enable;

	if (QDF_IS_STATUS_ERROR(ucfg_mlme_get_latency_enable
				(hdd_ctx->psoc, &latency_enable)))
		return false;

	if (latency_enable &&
	    sme_is_feature_supported_by_fw(VDEV_LATENCY_CONFIG))
		return true;
	else
		return false;
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
#ifdef FEATURE_WLAN_TDLS
	bool bvalue;
#endif
	uint32_t fine_time_meas_cap;

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

	if (ucfg_extscan_get_enable(hdd_ctx->psoc) &&
	    sme_is_feature_supported_by_fw(EXTENDED_SCAN)) {
		hdd_debug("EXTScan is supported by firmware");
		fset |= WIFI_FEATURE_EXTSCAN | WIFI_FEATURE_HAL_EPNO;
	}
	if (wlan_hdd_nan_is_supported(hdd_ctx)) {
		hdd_debug("NAN is supported by firmware");
		fset |= WIFI_FEATURE_NAN;
	}

	ucfg_mlme_get_fine_time_meas_cap(hdd_ctx->psoc, &fine_time_meas_cap);

	if (sme_is_feature_supported_by_fw(RTT) &&
	    rtt_is_enabled(fine_time_meas_cap)) {
		hdd_debug("RTT is supported by firmware and driver: %x",
			  fine_time_meas_cap);
		fset |= WIFI_FEATURE_D2D_RTT;
		fset |= WIFI_FEATURE_D2AP_RTT;
	}
#ifdef FEATURE_WLAN_SCAN_PNO
	if (ucfg_scan_get_pno_scan_support(hdd_ctx->psoc) &&
	    sme_is_feature_supported_by_fw(PNO)) {
		hdd_debug("PNO is supported by firmware");
		fset |= WIFI_FEATURE_PNO;
	}
#endif
	fset |= WIFI_FEATURE_ADDITIONAL_STA;
#ifdef FEATURE_WLAN_TDLS
	cfg_tdls_get_support_enable(hdd_ctx->psoc, &bvalue);
	if ((bvalue) && sme_is_feature_supported_by_fw(TDLS)) {
		hdd_debug("TDLS is supported by firmware");
		fset |= WIFI_FEATURE_TDLS;
	}

	cfg_tdls_get_off_channel_enable(hdd_ctx->psoc, &bvalue);
	if (sme_is_feature_supported_by_fw(TDLS) &&
	    bvalue && sme_is_feature_supported_by_fw(TDLS_OFF_CHANNEL)) {
		hdd_debug("TDLS off-channel is supported by firmware");
		fset |= WIFI_FEATURE_TDLS_OFFCHANNEL;
	}
#endif
	fset |= WIFI_FEATURE_AP_STA;
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

	if (hdd_is_wlm_latency_manager_supported(hdd_ctx))
		fset |= WIFI_FEATURE_SET_LATENCY_MODE;

	if (hdd_dynamic_mac_addr_supported(hdd_ctx))
		fset |= WIFI_FEATURE_DYNAMIC_SET_MAC;

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
	struct osif_psoc_sync *psoc_sync;
	int errno;

	errno = osif_psoc_sync_op_start(wiphy_dev(wiphy), &psoc_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_get_supported_features(wiphy, wdev,
							   data, data_len);

	osif_psoc_sync_op_stop(psoc_sync);

	return errno;
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
	struct scan_mac_oui scan_mac_oui = { {0} };
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_SET_SCANNING_MAC_OUI_MAX + 1];
	QDF_STATUS status;
	int ret, len;
	struct net_device *ndev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(ndev);
	mac_handle_t mac_handle;
	bool mac_spoofing_enabled;

	hdd_enter_dev(wdev->netdev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	mac_spoofing_enabled = ucfg_scan_is_mac_spoofing_enabled(hdd_ctx->psoc);
	if (!mac_spoofing_enabled) {
		hdd_debug("MAC address spoofing is not enabled");
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

	if (!tb[QCA_WLAN_VENDOR_ATTR_SET_SCANNING_MAC_OUI]) {
		hdd_err("attr mac oui failed");
		return -EINVAL;
	}

	len = nla_len(tb[QCA_WLAN_VENDOR_ATTR_SET_SCANNING_MAC_OUI]);
	if (len != sizeof(scan_mac_oui.oui)) {
		hdd_err("attr mac oui invalid size %d expected %zu",
			len, sizeof(scan_mac_oui.oui));
		return -EINVAL;
	}

	nla_memcpy(scan_mac_oui.oui,
		   tb[QCA_WLAN_VENDOR_ATTR_SET_SCANNING_MAC_OUI],
		   sizeof(scan_mac_oui.oui));

	/* populate rest of scan_mac_oui for mac addr randomization */
	scan_mac_oui.vdev_id = adapter->vdev_id;
	scan_mac_oui.enb_probe_req_sno_randomization = true;

	hdd_debug("Oui (%02x:%02x:%02x), vdev_id = %d",
		  scan_mac_oui.oui[0], scan_mac_oui.oui[1],
		  scan_mac_oui.oui[2], scan_mac_oui.vdev_id);

	hdd_update_ie_whitelist_attr(&scan_mac_oui.ie_whitelist, hdd_ctx);

	mac_handle = hdd_ctx->mac_handle;
	status = sme_set_scanning_mac_oui(mac_handle, &scan_mac_oui);
	if (!QDF_IS_STATUS_SUCCESS(status))
		hdd_err("sme_set_scanning_mac_oui failed(err=%d)", status);

	return qdf_status_to_os_return(status);
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
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_set_scanning_mac_oui(wiphy, wdev,
							 data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
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
 * wlan_hdd_set_ndi_feature() - Set NDI related features
 * @feature_flags: pointer to the byte array of features.
 *
 * Return: None
 **/
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0))
static void wlan_hdd_set_ndi_feature(uint8_t *feature_flags)
{
	wlan_hdd_cfg80211_set_feature(feature_flags,
		QCA_WLAN_VENDOR_FEATURE_USE_ADD_DEL_VIRTUAL_INTF_FOR_NDI);
}
#else
static inline void wlan_hdd_set_ndi_feature(uint8_t *feature_flags)
{
}
#endif

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
	bool value, twt_req, twt_res;
	QDF_STATUS ret = QDF_STATUS_E_FAILURE;
	QDF_STATUS status;
	int ret_val;

	uint8_t feature_flags[(NUM_QCA_WLAN_VENDOR_FEATURES + 7) / 8] = {0};
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);

	hdd_enter_dev(wdev->netdev);

	ret_val = wlan_hdd_validate_context(hdd_ctx);
	if (ret_val)
		return ret_val;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	if (roaming_offload_enabled(hdd_ctx)) {
		hdd_debug("Key Mgmt Offload is supported");
		wlan_hdd_cfg80211_set_feature(feature_flags,
				QCA_WLAN_VENDOR_FEATURE_KEY_MGMT_OFFLOAD);
	}

	wlan_hdd_cfg80211_set_feature(feature_flags,
				QCA_WLAN_VENDOR_FEATURE_SUPPORT_HW_MODE_ANY);
	if (policy_mgr_is_scan_simultaneous_capable(hdd_ctx->psoc))
		wlan_hdd_cfg80211_set_feature(feature_flags,
			QCA_WLAN_VENDOR_FEATURE_OFFCHANNEL_SIMULTANEOUS);

	if (policy_mgr_is_hw_dbs_capable(hdd_ctx->psoc))
		wlan_hdd_cfg80211_set_feature(feature_flags,
			QCA_WLAN_VENDOR_FEATURE_CONCURRENT_BAND_SESSIONS);

	if (wma_is_p2p_lo_capable())
		wlan_hdd_cfg80211_set_feature(feature_flags,
			QCA_WLAN_VENDOR_FEATURE_P2P_LISTEN_OFFLOAD);

	value = false;
	status = ucfg_mlme_get_oce_sta_enabled_info(hdd_ctx->psoc, &value);
	if (QDF_IS_STATUS_ERROR(status))
		hdd_err("could not get OCE STA enable info");
	if (value)
		wlan_hdd_cfg80211_set_feature(feature_flags,
					      QCA_WLAN_VENDOR_FEATURE_OCE_STA);

	value = false;
	status = ucfg_mlme_get_oce_sap_enabled_info(hdd_ctx->psoc, &value);
	if (QDF_IS_STATUS_ERROR(status))
		hdd_err("could not get OCE SAP enable info");
	if (value)
		wlan_hdd_cfg80211_set_feature(feature_flags,
					  QCA_WLAN_VENDOR_FEATURE_OCE_STA_CFON);

	value = false;
	status = ucfg_mlme_get_adaptive11r_enabled(hdd_ctx->psoc, &value);
	if (QDF_IS_STATUS_ERROR(status))
		hdd_err("could not get FT-Adaptive 11R info");
	if (value) {
		hdd_debug("FT-Adaptive 11R is Enabled");
		wlan_hdd_cfg80211_set_feature(feature_flags,
					  QCA_WLAN_VENDOR_FEATURE_ADAPTIVE_11R);
	}

	hdd_get_twt_requestor(hdd_ctx->psoc, &twt_req);
	hdd_get_twt_responder(hdd_ctx->psoc, &twt_res);
	hdd_debug("twt_req:%d twt_res:%d", twt_req, twt_res);

	if (twt_req || twt_res) {
		wlan_hdd_cfg80211_set_feature(feature_flags,
					      QCA_WLAN_VENDOR_FEATURE_TWT);

		wlan_hdd_cfg80211_set_feature(
				feature_flags,
				QCA_WLAN_VENDOR_FEATURE_TWT_ASYNC_SUPPORT);
	}

	/* Check the kernel version for upstream commit aced43ce780dc5 that
	 * has support for processing user cell_base hints when wiphy is
	 * self managed or check the backport flag for the same.
	 */
#if defined CFG80211_USER_HINT_CELL_BASE_SELF_MANAGED || \
	    (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 18, 0))
	wlan_hdd_cfg80211_set_feature(feature_flags,
			QCA_WLAN_VENDOR_FEATURE_SELF_MANAGED_REGULATORY);
#endif

	if (wlan_hdd_thermal_config_support())
		wlan_hdd_cfg80211_set_feature(feature_flags,
					QCA_WLAN_VENDOR_FEATURE_THERMAL_CONFIG);

	wlan_hdd_set_ndi_feature(feature_flags);

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, sizeof(feature_flags) +
			NLMSG_HDRLEN);

	if (!skb) {
		hdd_err("cfg80211_vendor_cmd_alloc_reply_skb failed");
		return -ENOMEM;
	}

	if (nla_put(skb, QCA_WLAN_VENDOR_ATTR_FEATURE_FLAGS,
			sizeof(feature_flags), feature_flags))
		goto nla_put_failure;

	ret = policy_mgr_get_dbs_hw_modes(hdd_ctx->psoc,
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
	struct osif_psoc_sync *psoc_sync;
	int errno;

	errno = osif_psoc_sync_op_start(wiphy_dev(wiphy), &psoc_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_get_features(wiphy, wdev, data, data_len);

	osif_psoc_sync_op_stop(psoc_sync);

	return errno;
}

#define PARAM_NUM_NW \
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_WHITE_LIST_SSID_NUM_NETWORKS
#define PARAM_SET_BSSID \
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_BSSID_PARAMS_BSSID
#define PARAM_SET_BSSID_HINT \
		QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_BSSID_PARAMS_HINT
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
#define PARAM_ROAM_CONTROL_CONFIG \
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_CONTROL
#define PARAM_FREQ_LIST_SCHEME \
	QCA_ATTR_ROAM_CONTROL_FREQ_LIST_SCHEME
#define PARAM_FREQ_LIST_SCHEME_MAX \
	QCA_ATTR_ROAM_CONTROL_SCAN_FREQ_LIST_SCHEME_MAX
#define PARAM_SCAN_FREQ_LIST \
	QCA_ATTR_ROAM_CONTROL_SCAN_FREQ_LIST
#define PARAM_SCAN_FREQ_LIST_TYPE \
	QCA_ATTR_ROAM_CONTROL_SCAN_FREQ_LIST_TYPE
#define PARAM_CAND_SEL_CRITERIA_MAX \
	QCA_ATTR_ROAM_CAND_SEL_CRITERIA_RATE_MAX
#define PARAM_CAND_SEL_SCORE_RSSI \
	QCA_ATTR_ROAM_CAND_SEL_CRITERIA_SCORE_RSSI

const struct nla_policy wlan_hdd_set_roam_param_policy[
			MAX_ROAMING_PARAM + 1] = {
	[QCA_WLAN_VENDOR_ATTR_ROAMING_SUBCMD] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_ROAMING_REQ_ID] = {.type = NLA_U32},
	[PARAM_NUM_NW] = {.type = NLA_U32},
	[PARAM_SSID_LIST] = { .type = NLA_NESTED },
	[PARAM_LIST_SSID] = { .type = NLA_BINARY },
	[PARAM_A_BAND_BOOST_FACTOR] = {.type = NLA_U32},
	[PARAM_A_BAND_PELT_FACTOR] = {.type = NLA_U32},
	[PARAM_A_BAND_MAX_BOOST] = {.type = NLA_U32},
	[PARAM_ROAM_HISTERESYS] = {.type = NLA_S32},
	[PARAM_A_BAND_BOOST_THLD] = {.type = NLA_S32},
	[PARAM_A_BAND_PELT_THLD] = {.type = NLA_S32},
	[PARAM_RSSI_TRIGGER] = {.type = NLA_U32},
	[PARAM_ROAM_ENABLE] = {	.type = NLA_S32},
	[PARAM_BSSID_PREFS] = { .type = NLA_NESTED },
	[PARAM_NUM_BSSID] = {.type = NLA_U32},
	[PARAM_RSSI_MODIFIER] = {.type = NLA_U32},
	[PARAM_BSSID_PARAMS] = {.type = NLA_NESTED},
	[PARAMS_NUM_BSSID] = {.type = NLA_U32},
	[PARAM_ROAM_BSSID] = VENDOR_NLA_POLICY_MAC_ADDR,
	[PARAM_SET_BSSID] = VENDOR_NLA_POLICY_MAC_ADDR,
	[PARAM_SET_BSSID_HINT] = {.type = NLA_FLAG},
	[PARAM_ROAM_CONTROL_CONFIG] = {.type = NLA_NESTED},
};

/**
 * hdd_set_white_list() - parse white list
 * @hdd_ctx:        HDD context
 * @rso_config: rso config
 * @tb:            list of attributes
 * @vdev_id:    vdev id
 *
 * Return: 0 on success; error number on failure
 */
static int hdd_set_white_list(struct hdd_context *hdd_ctx,
			      struct rso_config_params *rso_config,
			      struct nlattr **tb, uint8_t vdev_id)
{
	int rem, i;
	uint32_t buf_len = 0, count;
	struct nlattr *tb2[MAX_ROAMING_PARAM + 1];
	struct nlattr *curr_attr = NULL;
	mac_handle_t mac_handle;

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
			if (i == MAX_SSID_ALLOWED_LIST) {
				hdd_err("Excess MAX_SSID_ALLOWED_LIST");
				goto fail;
			}
			if (wlan_cfg80211_nla_parse(tb2,
					QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_MAX,
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
			if (buf_len > 1 &&
			    ((buf_len - 1) <= WLAN_SSID_MAX_LEN)) {
				nla_memcpy(rso_config->ssid_allowed_list[i].ssid,
					tb2[PARAM_LIST_SSID], buf_len - 1);
				rso_config->ssid_allowed_list[i].length = buf_len - 1;
				hdd_debug("SSID[%d]: %.*s,length = %d",
					i,
					rso_config->ssid_allowed_list[i].length,
					rso_config->ssid_allowed_list[i].ssid,
					rso_config->ssid_allowed_list[i].length);
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

	rso_config->num_ssid_allowed_list = i;
	hdd_debug("Num of Allowed SSID %d", rso_config->num_ssid_allowed_list);
	mac_handle = hdd_ctx->mac_handle;
	sme_update_roam_params(mac_handle, vdev_id, rso_config,
			       REASON_ROAM_SET_SSID_ALLOWED);
	return 0;

fail:
	return -EINVAL;
}

/**
 * hdd_set_bssid_prefs() - parse set bssid prefs
 * @hdd_ctx:        HDD context
 * @rso_config: rso config
 * @tb:            list of attributes
 * @vdev_id:    vdev id
 *
 * Return: 0 on success; error number on failure
 */
static int hdd_set_bssid_prefs(struct hdd_context *hdd_ctx,
			       struct rso_config_params *rso_config,
			       struct nlattr **tb, uint8_t vdev_id)
{
	int rem, i;
	uint32_t count;
	struct nlattr *tb2[MAX_ROAMING_PARAM + 1];
	struct nlattr *curr_attr = NULL;
	mac_handle_t mac_handle;

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
		nla_memcpy(rso_config->bssid_favored[i].bytes,
			  tb2[PARAM_ROAM_BSSID],
			  QDF_MAC_ADDR_SIZE);
		hdd_debug(QDF_MAC_ADDR_FMT,
			  QDF_MAC_ADDR_REF(rso_config->bssid_favored[i].bytes));
		/* Parse and fetch preference factor*/
		if (!tb2[PARAM_RSSI_MODIFIER]) {
			hdd_err("BSSID Preference score failed");
			goto fail;
		}
		rso_config->bssid_favored_factor[i] = nla_get_u32(
			tb2[PARAM_RSSI_MODIFIER]);
		hdd_debug("BSSID Preference score (%d)",
			  rso_config->bssid_favored_factor[i]);
		i++;
	}
	if (i < count)
		hdd_warn("Num Preferred BSSID %u less than expected %u",
				 i, count);

	rso_config->num_bssid_favored = i;
	mac_handle = hdd_ctx->mac_handle;
	sme_update_roam_params(mac_handle, vdev_id,
			       rso_config, REASON_ROAM_SET_FAVORED_BSSID);

	return 0;

fail:
	return -EINVAL;
}

/**
 * hdd_set_blacklist_bssid() - parse set blacklist bssid
 * @hdd_ctx:        HDD context
 * @rso_config:   roam params
 * @tb:            list of attributes
 * @vdev_id:    vdev id
 *
 * Return: 0 on success; error number on failure
 */
static int hdd_set_blacklist_bssid(struct hdd_context *hdd_ctx,
				   struct rso_config_params *rso_config,
				   struct nlattr **tb,
				   uint8_t vdev_id)
{
	int rem, i;
	uint32_t count;
	uint8_t j = 0;
	struct nlattr *tb2[MAX_ROAMING_PARAM + 1];
	struct nlattr *curr_attr = NULL;
	struct qdf_mac_addr *black_list_bssid;
	mac_handle_t mac_handle;

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
	black_list_bssid = qdf_mem_malloc(sizeof(*black_list_bssid) *
					  MAX_BSSID_AVOID_LIST);
	if (!black_list_bssid)
		goto fail;

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
				qdf_mem_free(black_list_bssid);
				goto fail;
			}
			/* Parse and fetch MAC address */
			if (!tb2[PARAM_SET_BSSID]) {
				hdd_err("attr blacklist addr failed");
				qdf_mem_free(black_list_bssid);
				goto fail;
			}
			if (tb2[PARAM_SET_BSSID_HINT]) {
				struct reject_ap_info ap_info;

				qdf_mem_zero(&ap_info,
					     sizeof(struct reject_ap_info));
				nla_memcpy(ap_info.bssid.bytes,
					   tb2[PARAM_SET_BSSID],
					   QDF_MAC_ADDR_SIZE);
				ap_info.reject_ap_type = USERSPACE_AVOID_TYPE;
				ap_info.reject_reason =
						REASON_USERSPACE_AVOID_LIST;
				ap_info.source = ADDED_BY_DRIVER;

				/* This BSSID is avoided and not blacklisted */
				ucfg_blm_add_bssid_to_reject_list(hdd_ctx->pdev,
								  &ap_info);
				i++;
				continue;
			}
			nla_memcpy(black_list_bssid[j].bytes,
				   tb2[PARAM_SET_BSSID], QDF_MAC_ADDR_SIZE);
			hdd_debug(QDF_MAC_ADDR_FMT,
				  QDF_MAC_ADDR_REF(black_list_bssid[j].bytes));
			i++;
			j++;
		}
	}

	if (i < count)
		hdd_warn("Num Blacklist BSSID %u less than expected %u",
			 i, count);

	/* Send the blacklist to the blacklist mgr component */
	ucfg_blm_add_userspace_black_list(hdd_ctx->pdev, black_list_bssid, j);
	qdf_mem_free(black_list_bssid);
	mac_handle = hdd_ctx->mac_handle;
	sme_update_roam_params(mac_handle, vdev_id,
			       rso_config, REASON_ROAM_SET_BLACKLIST_BSSID);

	return 0;
fail:
	return -EINVAL;
}

static const struct nla_policy
roam_scan_freq_list_scheme_policy[PARAM_FREQ_LIST_SCHEME_MAX + 1] = {
	[PARAM_SCAN_FREQ_LIST_TYPE] = {.type = NLA_U32},
	[PARAM_SCAN_FREQ_LIST] = {.type = NLA_NESTED},
};

/**
 * hdd_send_roam_scan_channel_freq_list_to_sme() - Send control roam scan freqs
 * @hdd_ctx: HDD context
 * @vdev_id: vdev id
 * @tb: Nested attribute carrying frequency list scheme
 *
 * Extracts the frequency list and frequency list type from the frequency
 * list scheme and send the frequencies to SME.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
hdd_send_roam_scan_channel_freq_list_to_sme(struct hdd_context *hdd_ctx,
					    uint8_t vdev_id, struct nlattr *tb)
{
	QDF_STATUS status;
	struct nlattr *tb2[PARAM_FREQ_LIST_SCHEME_MAX + 1], *curr_attr;
	uint8_t num_chan = 0;
	uint32_t freq_list[SIR_MAX_SUPPORTED_CHANNEL_LIST] = {0};
	uint32_t list_type;
	mac_handle_t mac_handle = hdd_ctx->mac_handle;
	int rem;

	if (wlan_cfg80211_nla_parse_nested(tb2, PARAM_FREQ_LIST_SCHEME_MAX,
					   tb,
					   roam_scan_freq_list_scheme_policy)) {
		hdd_err("nla_parse failed");
		return QDF_STATUS_E_INVAL;
	}

	if (!tb2[PARAM_SCAN_FREQ_LIST] || !tb2[PARAM_SCAN_FREQ_LIST_TYPE]) {
		hdd_err("ROAM_CONTROL_SCAN_FREQ_LIST or type are not present");
		return QDF_STATUS_E_INVAL;
	}

	list_type = nla_get_u32(tb2[PARAM_SCAN_FREQ_LIST_TYPE]);
	if (list_type != QCA_PREFERRED_SCAN_FREQ_LIST &&
	    list_type != QCA_SPECIFIC_SCAN_FREQ_LIST) {
		hdd_err("Invalid freq list type received: %u", list_type);
		return QDF_STATUS_E_INVAL;
	}

	nla_for_each_nested(curr_attr, tb2[PARAM_SCAN_FREQ_LIST], rem) {
		if (num_chan >= SIR_MAX_SUPPORTED_CHANNEL_LIST) {
			hdd_err("number of channels (%d) supported exceeded max (%d)",
				num_chan, SIR_MAX_SUPPORTED_CHANNEL_LIST);
			return QDF_STATUS_E_INVAL;
		}
		num_chan++;
	}
	num_chan = 0;

	nla_for_each_nested(curr_attr, tb2[PARAM_SCAN_FREQ_LIST], rem) {
		if (nla_len(curr_attr) != sizeof(uint32_t)) {
			hdd_err("len is not correct for frequency %d",
				num_chan);
			return QDF_STATUS_E_INVAL;
		}
		freq_list[num_chan++] = nla_get_u32(curr_attr);
	}

	status = sme_update_roam_scan_freq_list(mac_handle, vdev_id, freq_list,
						num_chan, list_type);
	if (QDF_IS_STATUS_ERROR(status))
		hdd_err("Failed to update channel list information");

	return status;
}

static const struct nla_policy
roam_control_policy[QCA_ATTR_ROAM_CONTROL_MAX + 1] = {
	[QCA_ATTR_ROAM_CONTROL_ENABLE] = {.type = NLA_U8},
	[QCA_ATTR_ROAM_CONTROL_STATUS] = {.type = NLA_U8},
	[PARAM_FREQ_LIST_SCHEME] = {.type = NLA_NESTED},
	[QCA_ATTR_ROAM_CONTROL_FULL_SCAN_PERIOD] = {.type = NLA_U32},
	[QCA_ATTR_ROAM_CONTROL_CLEAR_ALL] = {.type = NLA_FLAG},
	[QCA_ATTR_ROAM_CONTROL_TRIGGERS] = {.type = NLA_U32},
	[QCA_ATTR_ROAM_CONTROL_SELECTION_CRITERIA] = {.type = NLA_NESTED},
	[QCA_ATTR_ROAM_CONTROL_SCAN_PERIOD] = {.type = NLA_U32},
	[QCA_ATTR_ROAM_CONTROL_SCAN_SCHEME] = {.type = NLA_U32},
	[QCA_ATTR_ROAM_CONTROL_CONNECTED_RSSI_THRESHOLD] = {.type = NLA_U32},
	[QCA_ATTR_ROAM_CONTROL_CANDIDATE_RSSI_THRESHOLD] = {.type = NLA_U32},
	[QCA_ATTR_ROAM_CONTROL_CANDIDATE_RSSI_THRESHOLD_2P4GHZ] = {
			.type = NLA_U32},
	[QCA_ATTR_ROAM_CONTROL_CANDIDATE_RSSI_THRESHOLD_5GHZ] = {
			.type = NLA_U32},
	[QCA_ATTR_ROAM_CONTROL_CANDIDATE_RSSI_THRESHOLD_6GHZ] = {
			.type = NLA_U32},
	[QCA_ATTR_ROAM_CONTROL_USER_REASON] = {.type = NLA_U32},
	[QCA_ATTR_ROAM_CONTROL_SCAN_SCHEME_TRIGGERS] = {.type = NLA_U32},
	[QCA_ATTR_ROAM_CONTROL_BAND_MASK] = {.type = NLA_U32},
	[QCA_ATTR_ROAM_CONTROL_HO_DELAY_FOR_RX] = {.type = NLA_U16},
	[QCA_ATTR_ROAM_CONTROL_FULL_SCAN_NO_REUSE_PARTIAL_SCAN_FREQ] = {
			.type = NLA_U8},
	[QCA_ATTR_ROAM_CONTROL_FULL_SCAN_6GHZ_ONLY_ON_PRIOR_DISCOVERY] = {
			.type = NLA_U8},
	[QCA_ATTR_ROAM_CONTROL_CONNECTED_HIGH_RSSI_OFFSET] = {.type = NLA_U8},
};

/**
 * hdd_send_roam_full_scan_period_to_sme() - Send full roam scan period to SME
 * @hdd_ctx: HDD context
 * @vdev_id: vdev id
 * @full_roam_scan_period: Idle period in seconds between two successive
 *			   full channel roam scans
 * @check_and_update: If this is true/set, update the value only if the current
 *		      configured value is not same as global value read from
 *		      ini param. This is to give priority to the user configured
 *		      values and retain the value, if updated already.
 *		      If this is not set, update the value without any check.
 *
 * Validate the full roam scan period and send it to firmware
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
hdd_send_roam_full_scan_period_to_sme(struct hdd_context *hdd_ctx,
				      uint8_t vdev_id,
				      uint32_t full_roam_scan_period,
				      bool check_and_update)
{
	QDF_STATUS status;
	uint32_t full_roam_scan_period_current, full_roam_scan_period_global;

	if (!ucfg_mlme_validate_full_roam_scan_period(full_roam_scan_period))
		return QDF_STATUS_E_INVAL;

	hdd_debug("Received Command to Set full roam scan period = %u",
		  full_roam_scan_period);

	status = sme_get_full_roam_scan_period(hdd_ctx->mac_handle, vdev_id,
					       &full_roam_scan_period_current);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	full_roam_scan_period_global =
		sme_get_full_roam_scan_period_global(hdd_ctx->mac_handle);
	if (check_and_update &&
	    full_roam_scan_period_current != full_roam_scan_period_global) {
		hdd_debug("Full roam scan period is already updated, value: %u",
			  full_roam_scan_period_current);
		return QDF_STATUS_SUCCESS;
	}
	status = sme_update_full_roam_scan_period(hdd_ctx->mac_handle, vdev_id,
						  full_roam_scan_period);
	if (QDF_IS_STATUS_ERROR(status))
		hdd_err("Failed to set full scan period");

	return status;
}

/**
 * wlan_hdd_convert_control_roam_trigger_reason_bitmap  - Convert the
 * vendor specific reason code to internal reason code.
 * @trigger_reason_bitmap: Vendor specific roam trigger bitmap
 *
 * Return: Internal roam trigger bitmap
 */
static uint32_t
wlan_hdd_convert_control_roam_trigger_bitmap(uint32_t trigger_reason_bitmap)
{
	uint32_t drv_trigger_bitmap = 0, all_bitmap;

	/* Enable the complete trigger bitmap when all bits are set in
	 * the control config bitmap
	 */
	all_bitmap = (QCA_ROAM_TRIGGER_REASON_EXTERNAL_SCAN << 1) - 1;
	if (trigger_reason_bitmap == all_bitmap)
		return BIT(ROAM_TRIGGER_REASON_MAX) - 1;

	if (trigger_reason_bitmap & QCA_ROAM_TRIGGER_REASON_PER)
		drv_trigger_bitmap |= BIT(ROAM_TRIGGER_REASON_PER);

	if (trigger_reason_bitmap & QCA_ROAM_TRIGGER_REASON_BEACON_MISS)
		drv_trigger_bitmap |= BIT(ROAM_TRIGGER_REASON_BMISS);

	if (trigger_reason_bitmap & QCA_ROAM_TRIGGER_REASON_POOR_RSSI)
		drv_trigger_bitmap |= BIT(ROAM_TRIGGER_REASON_LOW_RSSI);

	if (trigger_reason_bitmap & QCA_ROAM_TRIGGER_REASON_BETTER_RSSI)
		drv_trigger_bitmap |= BIT(ROAM_TRIGGER_REASON_HIGH_RSSI);

	if (trigger_reason_bitmap & QCA_ROAM_TRIGGER_REASON_PERIODIC)
		drv_trigger_bitmap |= BIT(ROAM_TRIGGER_REASON_PERIODIC);

	if (trigger_reason_bitmap & QCA_ROAM_TRIGGER_REASON_DENSE)
		drv_trigger_bitmap |= BIT(ROAM_TRIGGER_REASON_DENSE);

	if (trigger_reason_bitmap & QCA_ROAM_TRIGGER_REASON_BTM)
		drv_trigger_bitmap |= BIT(ROAM_TRIGGER_REASON_BTM);

	if (trigger_reason_bitmap & QCA_ROAM_TRIGGER_REASON_BSS_LOAD)
		drv_trigger_bitmap |= BIT(ROAM_TRIGGER_REASON_BSS_LOAD);

	if (trigger_reason_bitmap & QCA_ROAM_TRIGGER_REASON_USER_TRIGGER)
		drv_trigger_bitmap |= BIT(ROAM_TRIGGER_REASON_FORCED);

	if (trigger_reason_bitmap & QCA_ROAM_TRIGGER_REASON_DEAUTH)
		drv_trigger_bitmap |= BIT(ROAM_TRIGGER_REASON_DEAUTH);

	if (trigger_reason_bitmap & QCA_ROAM_TRIGGER_REASON_IDLE)
		drv_trigger_bitmap |= BIT(ROAM_TRIGGER_REASON_IDLE);

	if (trigger_reason_bitmap & QCA_ROAM_TRIGGER_REASON_TX_FAILURES)
		drv_trigger_bitmap |= BIT(ROAM_TRIGGER_REASON_STA_KICKOUT);

	if (trigger_reason_bitmap & QCA_ROAM_TRIGGER_REASON_EXTERNAL_SCAN)
		drv_trigger_bitmap |= BIT(ROAM_TRIGGER_REASON_BACKGROUND);

	return drv_trigger_bitmap;
}

/**
 * wlan_hdd_convert_control_roam_scan_scheme_bitmap()  - Convert the
 * vendor specific roam scan scheme for roam triggers to internal roam trigger
 * bitmap for partial scan.
 * @trigger_reason_bitmap: Vendor specific roam trigger bitmap
 *
 * Return: Internal roam scan scheme bitmap
 */
static uint32_t
wlan_hdd_convert_control_roam_scan_scheme_bitmap(uint32_t trigger_reason_bitmap)
{
	uint32_t drv_scan_scheme_bitmap = 0;

	/*
	 * Partial scan scheme override over default scan scheme only for
	 * the PER, BMISS, Low RSSI, BTM, BSS_LOAD Triggers
	 */
	if (trigger_reason_bitmap & QCA_ROAM_TRIGGER_REASON_PER)
		drv_scan_scheme_bitmap |= BIT(ROAM_TRIGGER_REASON_PER);

	if (trigger_reason_bitmap & QCA_ROAM_TRIGGER_REASON_BEACON_MISS)
		drv_scan_scheme_bitmap |= BIT(ROAM_TRIGGER_REASON_BMISS);

	if (trigger_reason_bitmap & QCA_ROAM_TRIGGER_REASON_POOR_RSSI)
		drv_scan_scheme_bitmap |= BIT(ROAM_TRIGGER_REASON_LOW_RSSI);

	if (trigger_reason_bitmap & QCA_ROAM_TRIGGER_REASON_BTM)
		drv_scan_scheme_bitmap |= BIT(ROAM_TRIGGER_REASON_BTM);

	if (trigger_reason_bitmap & QCA_ROAM_TRIGGER_REASON_BSS_LOAD)
		drv_scan_scheme_bitmap |= BIT(ROAM_TRIGGER_REASON_BSS_LOAD);

	if (trigger_reason_bitmap & QCA_ROAM_TRIGGER_REASON_USER_TRIGGER)
		drv_scan_scheme_bitmap |= BIT(ROAM_TRIGGER_REASON_FORCED);

	if (trigger_reason_bitmap & QCA_ROAM_TRIGGER_REASON_DEAUTH)
		drv_scan_scheme_bitmap |= BIT(ROAM_TRIGGER_REASON_DEAUTH);

	if (trigger_reason_bitmap & QCA_ROAM_TRIGGER_REASON_IDLE)
		drv_scan_scheme_bitmap |= BIT(ROAM_TRIGGER_REASON_IDLE);

	if (trigger_reason_bitmap & QCA_ROAM_TRIGGER_REASON_TX_FAILURES)
		drv_scan_scheme_bitmap |= BIT(ROAM_TRIGGER_REASON_STA_KICKOUT);

	if (trigger_reason_bitmap & QCA_ROAM_TRIGGER_REASON_EXTERNAL_SCAN)
		drv_scan_scheme_bitmap |= BIT(ROAM_TRIGGER_REASON_BACKGROUND);

	return drv_scan_scheme_bitmap;
}

/**
 * hdd_send_roam_triggers_to_sme() - Send roam trigger bitmap to SME
 * @hdd_ctx: HDD context
 * @vdev_id: vdev id
 * @roam_trigger_bitmap: Vendor configured roam trigger bitmap to be configured
 *			 to firmware
 *
 * Send the roam trigger bitmap received to SME
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
hdd_send_roam_triggers_to_sme(struct hdd_context *hdd_ctx,
			      uint8_t vdev_id,
			      uint32_t roam_trigger_bitmap)
{
	QDF_STATUS status;
	struct wlan_roam_triggers triggers;
	struct hdd_adapter *adapter;

	adapter = hdd_get_adapter_by_vdev(hdd_ctx, vdev_id);
	if (!adapter) {
		hdd_err("adapter NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (adapter->device_mode != QDF_STA_MODE) {
		hdd_err("Roam trigger bitmap supported only in STA mode");
		return QDF_STATUS_E_FAILURE;
	}

	triggers.vdev_id = vdev_id;
	triggers.trigger_bitmap =
	    wlan_hdd_convert_control_roam_trigger_bitmap(roam_trigger_bitmap);
	hdd_debug("trigger bitmap: 0x%x converted trigger_bitmap: 0x%x",
		  roam_trigger_bitmap, triggers.trigger_bitmap);
	/*
	 * In standalone STA, if this vendor command is received between
	 * ROAM_START and roam synch indication, it is better to reject
	 * roam disable since driver would send vdev_params command to
	 * de-initialize roaming structures in fw.
	 * In STA+STA mode, if this vendor command to enable roaming is
	 * received for one STA vdev and ROAM_START was received for other
	 * STA vdev, then also driver would be send vdev_params command to
	 * de-initialize roaming structures in fw on the roaming enabled
	 * vdev.
	 */
	if (hdd_is_roaming_in_progress(hdd_ctx)) {
		mlme_set_roam_trigger_bitmap(hdd_ctx->psoc, adapter->vdev_id,
					     triggers.trigger_bitmap);
		hdd_err("Reject set roam trigger as roaming is in progress");

		return QDF_STATUS_E_BUSY;
	}

	/*
	 * roam trigger bitmap is > 0 - Roam triggers are set.
	 * roam trigger bitmap is 0 - Disable roaming
	 *
	 * For both the above modes, reset the roam scan scheme bitmap to
	 * 0.
	 */
	status = ucfg_cm_update_roam_scan_scheme_bitmap(hdd_ctx->psoc,
							vdev_id, 0);

	status = ucfg_cm_rso_set_roam_trigger(hdd_ctx->pdev, vdev_id,
					      &triggers);
	if (QDF_IS_STATUS_ERROR(status))
		hdd_err("Failed to set roam control trigger bitmap");

	return status;
}

/*
 * Disable default scoring algorithm. This is intended to set all bits of the
 * disable_bitmap in struct scoring_param.
 */
#define DISABLE_SCORING 0

/*
 * Enable scoring algorithm. This is intended to clear all bits of the
 * disable_bitmap in struct scoring_param.
 */
#define ENABLE_SCORING 1

/*
 * Controlled roam candidate selection is enabled from userspace.
 * Driver/firmware should honor the selection criteria
 */
#define CONTROL_ROAM_CAND_SEL_ENABLE 1

/*
 * Controlled roam candidate selection is disabled from userspace.
 * Driver/firmware can use its internal candidate selection criteria
 */
#define CONTROL_ROAM_CAND_SEL_DISABLE 0

static const struct nla_policy
roam_scan_cand_sel_policy[PARAM_CAND_SEL_CRITERIA_MAX + 1] = {
	[PARAM_CAND_SEL_SCORE_RSSI] = {.type = NLA_U8},
};

/**
 * hdd_send_roam_cand_sel_criteria_to_sme() - Send candidate sel criteria to SME
 * @hdd_ctx: HDD context
 * @vdev_id: vdev id
 * @attr: Nested attribute carrying candidate selection criteria
 *
 * Extract different candidate sel criteria mentioned and convert it to
 * driver/firmware understable format.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
hdd_send_roam_cand_sel_criteria_to_sme(struct hdd_context *hdd_ctx,
				       uint8_t vdev_id,
				       struct nlattr *attr)
{
	QDF_STATUS status;
	struct nlattr *tb2[PARAM_CAND_SEL_CRITERIA_MAX + 1];
	struct nlattr *curr_attr;
	uint8_t sel_criteria = 0, rssi_score = 0, scoring;
	int rem;

	hdd_debug("Received Command to Set candidate selection criteria ");
	nla_for_each_nested(curr_attr, attr, rem) {
		sel_criteria++;
		break;
	}

	if (sel_criteria &&
	    wlan_cfg80211_nla_parse_nested(tb2, PARAM_CAND_SEL_CRITERIA_MAX,
					   attr, roam_scan_cand_sel_policy)) {
		hdd_err("nla_parse failed");
		return QDF_STATUS_E_INVAL;
	}

	/*
	 * Firmware supports the below configurations currently,
	 * 1. Default selection criteria where all scoring params
	 *    are enabled and different weightages/scores are given to
	 *    different parameters.
	 *    When userspace doesn't specify any candidate selection criteria,
	 *    this will be enabled.
	 * 2. Legacy candidate selection criteria where scoring
	 *    algorithm is disabled and only RSSI is considered for
	 *    roam candidate selection.
	 *    When userspace specify 100% weightage for RSSI, this will
	 *    be enabled.
	 * Rest of the combinations are not supported for now.
	 */
	if (sel_criteria == CONTROL_ROAM_CAND_SEL_ENABLE) {
		/* Legacy selection criteria: 100% weightage to RSSI */
		if (tb2[PARAM_CAND_SEL_SCORE_RSSI])
			rssi_score = nla_get_u8(tb2[PARAM_CAND_SEL_SCORE_RSSI]);

		if (rssi_score != 100) {
			hdd_debug("Ignore the candidate selection criteria");
			return QDF_STATUS_E_INVAL;
		}
		scoring = DISABLE_SCORING;
	} else {
		/* Default selection criteria */
		scoring = ENABLE_SCORING;
	}

	status = sme_modify_roam_cand_sel_criteria(hdd_ctx->mac_handle, vdev_id,
						   !!scoring);
	if (QDF_IS_STATUS_ERROR(status))
		hdd_err("Failed to disable scoring");

	return status;
}

/**
 * hdd_send_roam_scan_period_to_sme() - Send roam scan period to SME
 * @hdd_ctx: HDD context
 * @vdev_id: vdev id
 * @roam_scan_period: Roam scan period in seconds
 * @check_and_update: If this is true/set, update the value only if the current
 *		      configured value is not same as global value read from
 *		      ini param. This is to give priority to the user configured
 *		      values and retain the value, if updated already.
 *		      If this is not set, update the value without any check.
 *
 * Validate the roam scan period and send it to firmware if valid.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
hdd_send_roam_scan_period_to_sme(struct hdd_context *hdd_ctx,
				 uint8_t vdev_id,
				 uint32_t roam_scan_period,
				 bool check_and_update)
{
	QDF_STATUS status;
	uint16_t roam_scan_period_current, roam_scan_period_global;

	if (!ucfg_mlme_validate_scan_period(roam_scan_period * 1000))
		return QDF_STATUS_E_INVAL;

	hdd_debug("Received Command to Set roam scan period (Empty Scan refresh period) = %d",
		  roam_scan_period);

	status = sme_get_empty_scan_refresh_period(hdd_ctx->mac_handle, vdev_id,
						   &roam_scan_period_current);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	roam_scan_period_global =
		sme_get_empty_scan_refresh_period_global(hdd_ctx->mac_handle);
	if (check_and_update &&
	    roam_scan_period_current != roam_scan_period_global) {
		hdd_debug("roam scan period is already updated, value: %u",
			  roam_scan_period_current / 1000);
		return QDF_STATUS_SUCCESS;
	}
	status = sme_update_empty_scan_refresh_period(hdd_ctx->mac_handle,
						      vdev_id,
						      roam_scan_period * 1000);
	if (QDF_IS_STATUS_ERROR(status))
		hdd_err("Failed to set scan period");

	return status;
}

/* Roam Hand-off delay range is 20 to 1000 msec */
#define MIN_ROAM_HO_DELAY 20
#define MAX_ROAM_HO_DELAY 1000

/* Include/Exclude roam partial scan channels in full scan */
#define INCLUDE_ROAM_PARTIAL_SCAN_FREQ 0
#define EXCLUDE_ROAM_PARTIAL_SCAN_FREQ 1

/* Include the supported 6 GHz PSC channels in full scan by default */
#define INCLUDE_6GHZ_IN_FULL_SCAN_BY_DEF	0
/* Include the 6 GHz channels in roam full scan only on prior discovery */
#define INCLUDE_6GHZ_IN_FULL_SCAN_IF_DISC	1

/**
 * hdd_set_roam_with_control_config() - Set roam control configuration
 * @hdd_ctx: HDD context
 * @tb: List of attributes carrying roam subcmd data
 * @vdev_id: vdev id
 *
 * Extracts the attribute PARAM_ROAM_CONTROL_CONFIG from the attributes list tb
 * and sends the corresponding roam control configuration to driver/firmware.
 *
 * Return: 0 on success; error number on failure
 */
static int
hdd_set_roam_with_control_config(struct hdd_context *hdd_ctx,
				 struct nlattr **tb,
				 uint8_t vdev_id)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct nlattr *tb2[QCA_ATTR_ROAM_CONTROL_MAX + 1], *attr;
	uint32_t value;
	struct wlan_cm_roam_vendor_btm_params param = {0};
	bool is_wtc_param_updated = false;
	uint32_t band_mask;

	hdd_enter();
	/* The command must carry PARAM_ROAM_CONTROL_CONFIG */
	if (!tb[PARAM_ROAM_CONTROL_CONFIG]) {
		hdd_err("Attribute CONTROL_CONFIG is not present");
		return -EINVAL;
	}

	if (wlan_cfg80211_nla_parse_nested(tb2, QCA_ATTR_ROAM_CONTROL_MAX,
					   tb[PARAM_ROAM_CONTROL_CONFIG],
					   roam_control_policy)) {
		hdd_err("nla_parse failed");
		return -EINVAL;
	}

	attr = tb2[PARAM_FREQ_LIST_SCHEME];
	if (attr) {
		status = hdd_send_roam_scan_channel_freq_list_to_sme(hdd_ctx,
								     vdev_id,
								     attr);
		if (QDF_IS_STATUS_ERROR(status))
			hdd_err("failed to config roam control");
	}

	if (tb2[QCA_ATTR_ROAM_CONTROL_TRIGGERS]) {
		value = nla_get_u32(tb2[QCA_ATTR_ROAM_CONTROL_TRIGGERS]);
		hdd_debug("Received roam trigger bitmap: 0x%x", value);
		status = hdd_send_roam_triggers_to_sme(hdd_ctx,
						       vdev_id,
						       value);
		if (status)
			hdd_err("failed to config roam triggers");
	}

	attr = tb2[QCA_ATTR_ROAM_CONTROL_ENABLE];
	if (attr) {
		status = sme_set_roam_config_enable(hdd_ctx->mac_handle,
						    vdev_id,
						    nla_get_u8(attr));
		if (QDF_IS_STATUS_ERROR(status))
			hdd_err("failed to enable/disable roam control config");

		hdd_debug("Parse and send roam control %s:",
			  nla_get_u8(attr) ? "Enable" : "Disable");

		attr = tb2[QCA_ATTR_ROAM_CONTROL_SCAN_PERIOD];
		if (attr) {
			/* Default value received as part of Roam control enable
			 * Set this only if user hasn't configured any value so
			 * far.
			 */
			value = nla_get_u32(attr);
			status = hdd_send_roam_scan_period_to_sme(hdd_ctx,
								  vdev_id,
								  value, true);
			if (QDF_IS_STATUS_ERROR(status))
				hdd_err("failed to send scan period to firmware");
		}

		attr = tb2[QCA_ATTR_ROAM_CONTROL_FULL_SCAN_PERIOD];
		if (attr) {
			value = nla_get_u32(attr);
			/* Default value received as part of Roam control enable
			 * Set this only if user hasn't configured any value so
			 * far.
			 */
			status = hdd_send_roam_full_scan_period_to_sme(hdd_ctx,
								       vdev_id,
								       value,
								       true);
			if (status)
				hdd_err("failed to config full scan period");
		}
	} else {
		attr = tb2[QCA_ATTR_ROAM_CONTROL_SCAN_PERIOD];
		if (attr) {
			/* User configured value, cache the value directly */
			value = nla_get_u32(attr);
			status = hdd_send_roam_scan_period_to_sme(hdd_ctx,
								  vdev_id,
								  value, false);
			if (QDF_IS_STATUS_ERROR(status))
				hdd_err("failed to send scan period to firmware");
		}

		attr = tb2[QCA_ATTR_ROAM_CONTROL_FULL_SCAN_PERIOD];
		if (attr) {
			value = nla_get_u32(attr);
			/* User configured value, cache the value directly */
			status = hdd_send_roam_full_scan_period_to_sme(hdd_ctx,
								       vdev_id,
								       value,
								       false);
			if (status)
				hdd_err("failed to config full scan period");
		}
	}

	attr = tb2[QCA_ATTR_ROAM_CONTROL_SCAN_SCHEME_TRIGGERS];
	if (attr) {
		value = wlan_hdd_convert_control_roam_scan_scheme_bitmap(
							nla_get_u32(attr));
		status = ucfg_cm_update_roam_scan_scheme_bitmap(hdd_ctx->psoc,
								vdev_id,
								value);
	}

	/* Scoring and roam candidate selection criteria */
	attr = tb2[QCA_ATTR_ROAM_CONTROL_SELECTION_CRITERIA];
	if (attr) {
		status = hdd_send_roam_cand_sel_criteria_to_sme(hdd_ctx,
								vdev_id, attr);
		if (QDF_IS_STATUS_ERROR(status))
			hdd_err("failed to set candidate selection criteria");
	}

	attr = tb2[QCA_ATTR_ROAM_CONTROL_SCAN_SCHEME];
	if (attr) {
		param.scan_freq_scheme = nla_get_u32(attr);
		is_wtc_param_updated = true;
	}

	attr = tb2[QCA_ATTR_ROAM_CONTROL_CONNECTED_RSSI_THRESHOLD];
	if (attr) {
		param.connected_rssi_threshold = nla_get_u32(attr);
		is_wtc_param_updated = true;
	}

	attr = tb2[QCA_ATTR_ROAM_CONTROL_CANDIDATE_RSSI_THRESHOLD];
	if (attr) {
		param.candidate_rssi_threshold_2g = nla_get_u32(attr);
		is_wtc_param_updated = true;
	}

	attr = tb2[QCA_ATTR_ROAM_CONTROL_CANDIDATE_RSSI_THRESHOLD_2P4GHZ];
	if (attr) {
		param.candidate_rssi_threshold_2g = nla_get_u32(attr);
		is_wtc_param_updated = true;
	}

	attr = tb2[QCA_ATTR_ROAM_CONTROL_CANDIDATE_RSSI_THRESHOLD_5GHZ];
	if (attr) {
		param.candidate_rssi_threshold_5g = nla_get_u32(attr);
		is_wtc_param_updated = true;
	} else {
		param.candidate_rssi_threshold_5g =
					param.candidate_rssi_threshold_2g;
	}

	attr = tb2[QCA_ATTR_ROAM_CONTROL_CANDIDATE_RSSI_THRESHOLD_6GHZ];
	if (attr) {
		param.candidate_rssi_threshold_6g = nla_get_u32(attr);
		is_wtc_param_updated = true;
	} else {
		param.candidate_rssi_threshold_6g =
					param.candidate_rssi_threshold_2g;
	}

	attr = tb2[QCA_ATTR_ROAM_CONTROL_USER_REASON];
	if (attr) {
		param.user_roam_reason = nla_get_u32(attr);
		is_wtc_param_updated = true;
	} else {
		param.user_roam_reason = DISABLE_VENDOR_BTM_CONFIG;
	}

	if (tb2[QCA_ATTR_ROAM_CONTROL_BAND_MASK]) {
		band_mask =
			nla_get_u32(tb2[QCA_ATTR_ROAM_CONTROL_BAND_MASK]);
		band_mask =
			wlan_vendor_bitmap_to_reg_wifi_band_bitmap(hdd_ctx->psoc,
								   band_mask);
		hdd_debug("[ROAM BAND] set roam band mask:%d", band_mask);
		if (band_mask) {
			ucfg_cm_set_roam_band_mask(hdd_ctx->psoc, vdev_id,
						   band_mask);
		} else {
			hdd_debug("Invalid roam BAND_MASK");
			return -EINVAL;
		}

		if (ucfg_cm_is_change_in_band_allowed(hdd_ctx->psoc, vdev_id,
						      band_mask)) {

			/* Disable roaming on Vdev before setting PCL */
			sme_stop_roaming(hdd_ctx->mac_handle, vdev_id,
					 REASON_DRIVER_DISABLED, RSO_SET_PCL);

			policy_mgr_set_pcl_for_existing_combo(hdd_ctx->psoc,
							      PM_STA_MODE,
							      vdev_id);

			/* Enable roaming once SET pcl is done */
			sme_start_roaming(hdd_ctx->mac_handle, vdev_id,
					  REASON_DRIVER_ENABLED, RSO_SET_PCL);
		}
	}

	if (is_wtc_param_updated) {
		wlan_cm_roam_set_vendor_btm_params(hdd_ctx->psoc, &param);
		/* Sends RSO update */
		sme_send_vendor_btm_params(hdd_ctx->mac_handle, vdev_id);
	}

	attr = tb2[QCA_ATTR_ROAM_CONTROL_HO_DELAY_FOR_RX];
	if (attr) {
		value = nla_get_u16(attr);
		if (value < MIN_ROAM_HO_DELAY || value > MAX_ROAM_HO_DELAY) {
			hdd_err("Invalid roam HO delay value: %d", value);
			return -EINVAL;
		}

		hdd_debug("Received roam HO delay value: %d", value);

		status = ucfg_cm_roam_send_ho_delay_config(hdd_ctx->pdev,
							   vdev_id, value);
		if (QDF_IS_STATUS_ERROR(status))
			hdd_err("failed to set hand-off delay");
	}

	attr = tb2[QCA_ATTR_ROAM_CONTROL_FULL_SCAN_NO_REUSE_PARTIAL_SCAN_FREQ];
	if (attr) {
		value = nla_get_u8(attr);
		if (value < INCLUDE_ROAM_PARTIAL_SCAN_FREQ ||
		    value > EXCLUDE_ROAM_PARTIAL_SCAN_FREQ) {
			hdd_err("Invalid value %d to exclude partial scan freq",
				value);
			return -EINVAL;
		}

		hdd_debug("%s partial scan channels in roam full scan",
			  value ? "Exclude" : "Include");

		status = ucfg_cm_exclude_rm_partial_scan_freq(hdd_ctx->pdev,
							      vdev_id, value);
		if (QDF_IS_STATUS_ERROR(status))
			hdd_err("Fail to exclude roam partial scan channels");
	}

	attr = tb2[QCA_ATTR_ROAM_CONTROL_FULL_SCAN_6GHZ_ONLY_ON_PRIOR_DISCOVERY];
	if (attr) {
		value = nla_get_u8(attr);
		if (value < INCLUDE_6GHZ_IN_FULL_SCAN_BY_DEF ||
		    value > INCLUDE_6GHZ_IN_FULL_SCAN_IF_DISC) {
			hdd_err("Invalid value %d to decide inclusion of 6 GHz channels",
				value);
			return -EINVAL;
		}

		hdd_debug("Include 6 GHz channels in roam full scan by %s",
			  value ? "prior discovery" : "default");

		status = ucfg_cm_roam_full_scan_6ghz_on_disc(hdd_ctx->pdev,
							     vdev_id, value);
		if (QDF_IS_STATUS_ERROR(status))
			hdd_err("Fail to decide inclusion of 6 GHz channels");
	}

	attr = tb2[QCA_ATTR_ROAM_CONTROL_CONNECTED_HIGH_RSSI_OFFSET];
	if (attr) {
		value = nla_get_u8(attr);
		if (!cfg_in_range(CFG_LFR_ROAM_SCAN_HI_RSSI_DELTA, value)) {
			hdd_err("High RSSI offset value %d is out of range",
				value);
			return -EINVAL;
		}

		hdd_debug("%s roam scan high RSSI with offset: %d for vdev %d",
			  value ? "Enable" : "Disable", value, vdev_id);

		if (!value &&
		    !wlan_cm_get_roam_scan_high_rssi_offset(hdd_ctx->psoc)) {
			hdd_debug("Roam scan high RSSI is already disabled");
			return -EINVAL;
		}

		status = ucfg_cm_set_roam_scan_high_rssi_offset(hdd_ctx->psoc,
								vdev_id, value);
		if (QDF_IS_STATUS_ERROR(status))
			hdd_err("Fail to set roam scan high RSSI offset for vdev %d",
				vdev_id);
	}

	return qdf_status_to_os_return(status);
}

#define ENABLE_ROAM_TRIGGERS_ALL (QCA_ROAM_TRIGGER_REASON_PER | \
				  QCA_ROAM_TRIGGER_REASON_BEACON_MISS | \
				  QCA_ROAM_TRIGGER_REASON_POOR_RSSI | \
				  QCA_ROAM_TRIGGER_REASON_BETTER_RSSI | \
				  QCA_ROAM_TRIGGER_REASON_PERIODIC | \
				  QCA_ROAM_TRIGGER_REASON_DENSE | \
				  QCA_ROAM_TRIGGER_REASON_BTM | \
				  QCA_ROAM_TRIGGER_REASON_BSS_LOAD | \
				  QCA_ROAM_TRIGGER_REASON_USER_TRIGGER | \
				  QCA_ROAM_TRIGGER_REASON_DEAUTH | \
				  QCA_ROAM_TRIGGER_REASON_IDLE | \
				  QCA_ROAM_TRIGGER_REASON_TX_FAILURES | \
				  QCA_ROAM_TRIGGER_REASON_EXTERNAL_SCAN)

static int
hdd_clear_roam_control_config(struct hdd_context *hdd_ctx,
			      struct nlattr **tb,
			      uint8_t vdev_id)
{
	QDF_STATUS status;
	struct nlattr *tb2[QCA_ATTR_ROAM_CONTROL_MAX + 1];
	mac_handle_t mac_handle = hdd_ctx->mac_handle;
	uint32_t value;

	/* The command must carry PARAM_ROAM_CONTROL_CONFIG */
	if (!tb[PARAM_ROAM_CONTROL_CONFIG]) {
		hdd_err("Attribute CONTROL_CONFIG is not present");
		return -EINVAL;
	}

	if (wlan_cfg80211_nla_parse_nested(tb2, QCA_ATTR_ROAM_CONTROL_MAX,
					   tb[PARAM_ROAM_CONTROL_CONFIG],
					   roam_control_policy)) {
		hdd_err("nla_parse failed");
		return -EINVAL;
	}

	hdd_debug("Clear the control config done through SET");
	if (tb2[QCA_ATTR_ROAM_CONTROL_CLEAR_ALL]) {
		hdd_debug("Disable roam control config done through SET");
		status = sme_set_roam_config_enable(hdd_ctx->mac_handle,
						    vdev_id, 0);
		if (QDF_IS_STATUS_ERROR(status)) {
			hdd_err("failed to enable/disable roam control config");
			return qdf_status_to_os_return(status);
		}

		value = ENABLE_ROAM_TRIGGERS_ALL;
		hdd_debug("Reset roam trigger bitmap to 0x%x", value);
		status = hdd_send_roam_triggers_to_sme(hdd_ctx, vdev_id, value);
		if (QDF_IS_STATUS_ERROR(status)) {
			hdd_err("failed to restore roam trigger bitmap");
			return qdf_status_to_os_return(status);
		}

		status = sme_roam_control_restore_default_config(mac_handle,
								 vdev_id);
		if (QDF_IS_STATUS_ERROR(status)) {
			hdd_err("failed to config roam control");
			return qdf_status_to_os_return(status);
		}
	}

	return 0;
}

/**
 * hdd_roam_control_config_buf_size() - Calculate the skb size to be allocated
 * @hdd_ctx: HDD context
 * @tb: List of attributes to be populated
 *
 * Calculate the buffer size to be allocated based on the attributes
 * mentioned in tb.
 *
 * Return: buffer size to be allocated
 */
static uint16_t
hdd_roam_control_config_buf_size(struct hdd_context *hdd_ctx,
				 struct nlattr **tb)
{
	uint16_t skb_len = 0;

	if (tb[QCA_ATTR_ROAM_CONTROL_STATUS])
		skb_len += NLA_HDRLEN + sizeof(uint8_t);

	if (tb[QCA_ATTR_ROAM_CONTROL_FULL_SCAN_PERIOD])
		skb_len += NLA_HDRLEN + sizeof(uint32_t);

	if (tb[QCA_ATTR_ROAM_CONTROL_FREQ_LIST_SCHEME])
		/*
		 * Response has 3 nests, 1 atrribure value and a
		 * attribute list of frequencies.
		 */
		skb_len += 3 * nla_total_size(0) +
			nla_total_size(sizeof(uint32_t)) +
			(nla_total_size(sizeof(uint32_t)) *
			NUM_CHANNELS);

	if (tb[QCA_ATTR_ROAM_CONTROL_BAND_MASK])
		skb_len += NLA_HDRLEN + sizeof(uint32_t);

	return skb_len;
}

/**
 * wlan_reg_wifi_band_bitmap_to_vendor_bitmap() - Convert enum reg_wifi_band
 * to enum qca_set_band
 * @reg_wifi_band_bitmap: enum reg_wifi_band
 *
 * Return: qca_set_band value
 */
static uint32_t
wlan_reg_wifi_band_bitmap_to_vendor_bitmap(uint32_t reg_wifi_band_bitmap)
{
	uint32_t vendor_mask = 0;

	if (reg_wifi_band_bitmap & BIT(REG_BAND_2G))
		vendor_mask |= QCA_SETBAND_2G;
	if (reg_wifi_band_bitmap & BIT(REG_BAND_5G))
		vendor_mask |= QCA_SETBAND_5G;
	if (reg_wifi_band_bitmap & BIT(REG_BAND_6G))
		vendor_mask |= QCA_SETBAND_6G;

	return vendor_mask;
}

/**
 * hdd_roam_control_config_fill_data() - Fill the data requested by userspace
 * @hdd_ctx: HDD context
 * @vdev_id: vdev id
 * @skb: SK buffer
 * @tb: List of attributes
 *
 * Get the data corresponding to the attribute list specified in tb and
 * update the same to skb by populating the same attributes.
 *
 * Return: 0 on success; error number on failure
 */
static int
hdd_roam_control_config_fill_data(struct hdd_context *hdd_ctx, uint8_t vdev_id,
				  struct sk_buff *skb, struct nlattr **tb)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t roam_control;
	struct nlattr *config, *get_freq_scheme, *get_freq;
	uint32_t full_roam_scan_period, roam_band, vendor_band_mask;
	uint8_t num_channels = 0;
	uint32_t i = 0, freq_list[NUM_CHANNELS] = { 0 };
	struct hdd_adapter *hdd_adapter = NULL;

	config = nla_nest_start(skb, PARAM_ROAM_CONTROL_CONFIG);
	if (!config) {
		hdd_err("nla nest start failure");
		return -EINVAL;
	}

	if (tb[QCA_ATTR_ROAM_CONTROL_STATUS]) {
		status = sme_get_roam_config_status(hdd_ctx->mac_handle,
						    vdev_id,
						    &roam_control);
		if (QDF_IS_STATUS_ERROR(status))
			goto out;
		hdd_debug("Roam control: %s",
			  roam_control ? "Enabled" : "Disabled");
		if (nla_put_u8(skb, QCA_ATTR_ROAM_CONTROL_STATUS,
			       roam_control)) {
			hdd_info("failed to put vendor_roam_control");
			return -ENOMEM;
		}
	}

	if (tb[QCA_ATTR_ROAM_CONTROL_FULL_SCAN_PERIOD]) {
		status = sme_get_full_roam_scan_period(hdd_ctx->mac_handle,
						       vdev_id,
						       &full_roam_scan_period);
		if (QDF_IS_STATUS_ERROR(status))
			goto out;
		hdd_debug("full_roam_scan_period: %u", full_roam_scan_period);

		if (nla_put_u32(skb, QCA_ATTR_ROAM_CONTROL_FULL_SCAN_PERIOD,
				full_roam_scan_period)) {
			hdd_info("failed to put full_roam_scan_period");
			return -EINVAL;
		}
	}

	if (tb[QCA_ATTR_ROAM_CONTROL_FREQ_LIST_SCHEME]) {
		hdd_adapter = hdd_get_adapter_by_vdev(hdd_ctx, vdev_id);
		if (!hdd_adapter) {
			hdd_info("HDD adapter is NULL");
			return -EINVAL;
		}

		hdd_debug("Get roam scan frequencies req received");
		status = hdd_get_roam_scan_freq(hdd_adapter,
						hdd_ctx->mac_handle,
						freq_list, &num_channels);
		if (QDF_IS_STATUS_ERROR(status)) {
			hdd_info("failed to get roam scan freq");
			goto out;
		}

		hdd_debug("num_channels %d", num_channels);
		get_freq_scheme = nla_nest_start(
				skb, QCA_ATTR_ROAM_CONTROL_FREQ_LIST_SCHEME);
		if (!get_freq_scheme) {
			hdd_info("failed to nest start for roam scan freq");
			return -EINVAL;
		}

		if (nla_put_u32(skb, PARAM_SCAN_FREQ_LIST_TYPE, 0)) {
			hdd_info("failed to put list type");
			return -EINVAL;
		}

		get_freq = nla_nest_start(
				skb, QCA_ATTR_ROAM_CONTROL_SCAN_FREQ_LIST);
		if (!get_freq) {
			hdd_info("failed to nest start for roam scan freq");
			return -EINVAL;
		}

		for (i = 0; i < num_channels; i++) {
			if (nla_put_u32(skb, PARAM_SCAN_FREQ_LIST,
					freq_list[i])) {
				hdd_info("failed to put freq at index %d", i);
				return -EINVAL;
			}
		}
		nla_nest_end(skb, get_freq);
		nla_nest_end(skb, get_freq_scheme);
	}

	if (tb[QCA_ATTR_ROAM_CONTROL_BAND_MASK]) {
		status = ucfg_cm_get_roam_band(hdd_ctx->psoc, vdev_id,
					       &roam_band);
		if (QDF_IS_STATUS_ERROR(status))
			goto out;
		vendor_band_mask =
			wlan_reg_wifi_band_bitmap_to_vendor_bitmap(roam_band);
		if (nla_put_u32(skb, QCA_ATTR_ROAM_CONTROL_BAND_MASK,
				vendor_band_mask)) {
			hdd_info("failed to put roam_band");
			return -EINVAL;
		}
		hdd_debug("sending vendor_band_mask: %d reg band:%d",
			  vendor_band_mask, roam_band);
	}

	nla_nest_end(skb, config);

out:
	return qdf_status_to_os_return(status);
}

/**
 * hdd_send_roam_control_config() - Send the roam config as vendor cmd reply
 * @mac_handle: Opaque handle to the MAC context
 * @vdev_id: vdev id
 * @tb: List of attributes
 *
 * Parse the attributes list tb and  get the data corresponding to the
 * attributes specified in tb. Send them as a vendor response.
 *
 * Return: 0 on success; error number on failure
 */
static int
hdd_send_roam_control_config(struct hdd_context *hdd_ctx,
			     uint8_t vdev_id,
			     struct nlattr **tb)
{
	struct sk_buff *skb;
	uint16_t skb_len;
	int status;

	skb_len = hdd_roam_control_config_buf_size(hdd_ctx, tb);
	if (!skb_len) {
		hdd_err("No data requested");
		return -EINVAL;
	}

	skb_len += NLMSG_HDRLEN;
	skb = cfg80211_vendor_cmd_alloc_reply_skb(hdd_ctx->wiphy, skb_len);
	if (!skb) {
		hdd_info("cfg80211_vendor_cmd_alloc_reply_skb failed");
		return -ENOMEM;
	}

	status = hdd_roam_control_config_fill_data(hdd_ctx, vdev_id, skb, tb);
	if (status)
		goto fail;

	return cfg80211_vendor_cmd_reply(skb);

fail:
	hdd_err("nla put fail");
	kfree_skb(skb);
	return status;
}

/**
 * hdd_get_roam_control_config() - Send requested roam config to userspace
 * @hdd_ctx: HDD context
 * @tb: list of attributes
 * @vdev_id: vdev id
 *
 * Return: 0 on success; error number on failure
 */
static int hdd_get_roam_control_config(struct hdd_context *hdd_ctx,
				       struct nlattr **tb,
				       uint8_t vdev_id)
{
	QDF_STATUS status;
	struct nlattr *tb2[QCA_ATTR_ROAM_CONTROL_MAX + 1];

	/* The command must carry PARAM_ROAM_CONTROL_CONFIG */
	if (!tb[PARAM_ROAM_CONTROL_CONFIG]) {
		hdd_err("Attribute CONTROL_CONFIG is not present");
		return -EINVAL;
	}

	if (wlan_cfg80211_nla_parse_nested(tb2, QCA_ATTR_ROAM_CONTROL_MAX,
					   tb[PARAM_ROAM_CONTROL_CONFIG],
					   roam_control_policy)) {
		hdd_err("nla_parse failed");
		return -EINVAL;
	}

	status = hdd_send_roam_control_config(hdd_ctx, vdev_id, tb2);
	if (status) {
		hdd_err("failed to enable/disable roam control");
		return status;
	}

	return qdf_status_to_os_return(status);
}

#undef PARAM_ROAM_CONTROL_CONFIG
#undef PARAM_FREQ_LIST_SCHEME_MAX
#undef PARAM_FREQ_LIST_SCHEME
#undef PARAM_SCAN_FREQ_LIST
#undef PARAM_SCAN_FREQ_LIST_TYPE
#undef PARAM_CAND_SEL_CRITERIA_MAX
#undef PARAM_CAND_SEL_SCORE_RSSI

/**
 * hdd_set_ext_roam_params() - parse ext roam params
 * @hdd_ctx:        HDD context
 * @tb:            list of attributes
 * @vdev_id:       vdev id
 * @rso_config:    roam params
 *
 * Return: 0 on success; error number on failure
 */
static int hdd_set_ext_roam_params(struct hdd_context *hdd_ctx,
				   const void *data, int data_len,
				   uint8_t vdev_id,
				   struct rso_config_params *rso_config)
{
	uint32_t cmd_type, req_id;
	struct nlattr *tb[MAX_ROAMING_PARAM + 1];
	int ret;
	mac_handle_t mac_handle;

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
	mac_handle = hdd_ctx->mac_handle;
	req_id = nla_get_u32(
		tb[QCA_WLAN_VENDOR_ATTR_ROAMING_REQ_ID]);
	hdd_debug("Req Id: %u Cmd Type: %u", req_id, cmd_type);
	switch (cmd_type) {
	case QCA_WLAN_VENDOR_ROAMING_SUBCMD_SSID_WHITE_LIST:
		ret = hdd_set_white_list(hdd_ctx, rso_config, tb, vdev_id);
		if (ret)
			goto fail;
		break;

	case QCA_WLAN_VENDOR_ROAMING_SUBCMD_SET_EXTSCAN_ROAM_PARAMS:
		/* Parse and fetch 5G Boost Threshold */
		if (!tb[PARAM_A_BAND_BOOST_THLD]) {
			hdd_err("5G boost threshold failed");
			goto fail;
		}
		rso_config->raise_rssi_thresh_5g = nla_get_s32(
			tb[PARAM_A_BAND_BOOST_THLD]);
		hdd_debug("5G Boost Threshold (%d)",
			rso_config->raise_rssi_thresh_5g);
		/* Parse and fetch 5G Penalty Threshold */
		if (!tb[PARAM_A_BAND_PELT_THLD]) {
			hdd_err("5G penalty threshold failed");
			goto fail;
		}
		rso_config->drop_rssi_thresh_5g = nla_get_s32(
			tb[PARAM_A_BAND_PELT_THLD]);
		hdd_debug("5G Penalty Threshold (%d)",
			rso_config->drop_rssi_thresh_5g);
		/* Parse and fetch 5G Boost Factor */
		if (!tb[PARAM_A_BAND_BOOST_FACTOR]) {
			hdd_err("5G boost Factor failed");
			goto fail;
		}
		rso_config->raise_factor_5g = nla_get_u32(
			tb[PARAM_A_BAND_BOOST_FACTOR]);
		hdd_debug("5G Boost Factor (%d)",
			rso_config->raise_factor_5g);
		/* Parse and fetch 5G Penalty factor */
		if (!tb[PARAM_A_BAND_PELT_FACTOR]) {
			hdd_err("5G Penalty Factor failed");
			goto fail;
		}
		rso_config->drop_factor_5g = nla_get_u32(
			tb[PARAM_A_BAND_PELT_FACTOR]);
		hdd_debug("5G Penalty factor (%d)",
			rso_config->drop_factor_5g);
		/* Parse and fetch 5G Max Boost */
		if (!tb[PARAM_A_BAND_MAX_BOOST]) {
			hdd_err("5G Max Boost failed");
			goto fail;
		}
		rso_config->max_raise_rssi_5g = nla_get_u32(
			tb[PARAM_A_BAND_MAX_BOOST]);
		hdd_debug("5G Max Boost (%d)",
			rso_config->max_raise_rssi_5g);
		/* Parse and fetch Rssi Diff */
		if (!tb[PARAM_ROAM_HISTERESYS]) {
			hdd_err("Rssi Diff failed");
			goto fail;
		}
		rso_config->rssi_diff = nla_get_s32(
			tb[PARAM_ROAM_HISTERESYS]);
		hdd_debug("RSSI Diff (%d)",
			rso_config->rssi_diff);
		/* Parse and fetch Alert Rssi Threshold */
		if (!tb[PARAM_RSSI_TRIGGER]) {
			hdd_err("Alert Rssi Threshold failed");
			goto fail;
		}
		rso_config->alert_rssi_threshold = nla_get_u32(
			tb[PARAM_RSSI_TRIGGER]);
		hdd_debug("Alert RSSI Threshold (%d)",
			rso_config->alert_rssi_threshold);
		sme_update_roam_params(mac_handle, vdev_id, rso_config,
				       REASON_ROAM_EXT_SCAN_PARAMS_CHANGED);
		break;
	case QCA_WLAN_VENDOR_ROAMING_SUBCMD_SET_LAZY_ROAM:
		/* Parse and fetch Activate Good Rssi Roam */
		if (!tb[PARAM_ROAM_ENABLE]) {
			hdd_err("Activate Good Rssi Roam failed");
			goto fail;
		}
		rso_config->good_rssi_roam = nla_get_s32(
			tb[PARAM_ROAM_ENABLE]);
		hdd_debug("Activate Good Rssi Roam (%d)",
			  rso_config->good_rssi_roam);
		sme_update_roam_params(mac_handle, vdev_id, rso_config,
				       REASON_ROAM_GOOD_RSSI_CHANGED);
		break;
	case QCA_WLAN_VENDOR_ROAMING_SUBCMD_SET_BSSID_PREFS:
		ret = hdd_set_bssid_prefs(hdd_ctx, rso_config, tb, vdev_id);
		if (ret)
			goto fail;
		break;
	case QCA_WLAN_VENDOR_ROAMING_SUBCMD_SET_BLACKLIST_BSSID:
		ret = hdd_set_blacklist_bssid(hdd_ctx, rso_config, tb, vdev_id);
		if (ret)
			goto fail;
		break;
	case QCA_WLAN_VENDOR_ROAMING_SUBCMD_CONTROL_SET:
		ret = hdd_set_roam_with_control_config(hdd_ctx, tb, vdev_id);
		if (ret)
			goto fail;
		break;
	case QCA_WLAN_VENDOR_ROAMING_SUBCMD_CONTROL_CLEAR:
		ret = hdd_clear_roam_control_config(hdd_ctx, tb, vdev_id);
		if (ret)
			goto fail;
		break;
	case QCA_WLAN_VENDOR_ROAMING_SUBCMD_CONTROL_GET:
		ret = hdd_get_roam_control_config(hdd_ctx, tb, vdev_id);
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
	struct rso_config_params *rso_config;
	int ret;

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

	rso_config = qdf_mem_malloc(sizeof(*rso_config));
	if (!rso_config)
		return -ENOMEM;

	ret = hdd_set_ext_roam_params(hdd_ctx, data, data_len,
				      adapter->vdev_id, rso_config);
	qdf_mem_free(rso_config);
	if (ret)
		goto fail;

	return 0;
fail:
	return ret;
}
#undef PARAM_NUM_NW
#undef PARAM_SET_BSSID
#undef PARAM_SET_BSSID_HINT
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
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_set_ext_roam_params(wiphy, wdev,
							data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}

#define RATEMASK_PARAMS_TYPE_MAX 4
#define RATEMASK_PARAMS_BITMAP_MAX 16
#define RATEMASK_PARAMS_MAX QCA_WLAN_VENDOR_ATTR_RATEMASK_PARAMS_MAX
const struct nla_policy wlan_hdd_set_ratemask_param_policy[
			RATEMASK_PARAMS_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_RATEMASK_PARAMS_LIST] =
		VENDOR_NLA_POLICY_NESTED(wlan_hdd_set_ratemask_param_policy),
	[QCA_WLAN_VENDOR_ATTR_RATEMASK_PARAMS_TYPE] = {.type = NLA_U8},
	[QCA_WLAN_VENDOR_ATTR_RATEMASK_PARAMS_BITMAP] = {.type = NLA_BINARY,
					.len = RATEMASK_PARAMS_BITMAP_MAX},
};

/**
 * hdd_set_ratemask_params() - parse ratemask params
 * @hdd_ctx:        HDD context
 * @tb:            list of attributes
 * @vdev_id:       vdev id
 *
 * Return: 0 on success; error number on failure
 */
static int hdd_set_ratemask_params(struct hdd_context *hdd_ctx,
				   const void *data, int data_len,
				   struct wlan_objmgr_vdev *vdev)
{
	struct nlattr *tb[RATEMASK_PARAMS_MAX + 1];
	struct nlattr *tb2[RATEMASK_PARAMS_MAX + 1];
	struct nlattr *curr_attr;
	int ret, rem;
	struct config_ratemask_params rate_params[RATEMASK_PARAMS_TYPE_MAX];
	uint8_t ratemask_type, num_ratemask = 0, len;
	uint32_t bitmap[RATEMASK_PARAMS_BITMAP_MAX / 4];

	ret = wlan_cfg80211_nla_parse(tb,
				      RATEMASK_PARAMS_MAX,
				      data, data_len,
				      wlan_hdd_set_ratemask_param_policy);
	if (ret) {
		hdd_err("Invalid ATTR");
		return -EINVAL;
	}

	if (!tb[QCA_WLAN_VENDOR_ATTR_RATEMASK_PARAMS_LIST]) {
		hdd_err("ratemask array attribute not present");
		return -EINVAL;
	}

	memset(rate_params, 0, (RATEMASK_PARAMS_TYPE_MAX *
				sizeof(struct config_ratemask_params)));

	nla_for_each_nested(curr_attr,
			    tb[QCA_WLAN_VENDOR_ATTR_RATEMASK_PARAMS_LIST],
			    rem) {
		if (num_ratemask >= RATEMASK_PARAMS_TYPE_MAX) {
			hdd_err("Exceeding ratemask_list_param_num value");
			return -EINVAL;
		}

		if (wlan_cfg80211_nla_parse(
				tb2, RATEMASK_PARAMS_MAX,
				nla_data(curr_attr), nla_len(curr_attr),
				wlan_hdd_set_ratemask_param_policy)) {
			hdd_err("nla_parse failed");
			return -EINVAL;
		}

		if (!tb2[QCA_WLAN_VENDOR_ATTR_RATEMASK_PARAMS_TYPE]) {
			hdd_err("type attribute not present");
			return -EINVAL;
		}

		if (!tb2[QCA_WLAN_VENDOR_ATTR_RATEMASK_PARAMS_BITMAP]) {
			hdd_err("bitmap attribute not present");
			return -EINVAL;
		}

		ratemask_type =
		 nla_get_u8(tb2[QCA_WLAN_VENDOR_ATTR_RATEMASK_PARAMS_TYPE]);
		if (ratemask_type >= RATEMASK_PARAMS_TYPE_MAX) {
			hdd_err("invalid ratemask type");
			return -EINVAL;
		}

		len = nla_len(tb2[QCA_WLAN_VENDOR_ATTR_RATEMASK_PARAMS_BITMAP]);
		qdf_mem_zero(bitmap, sizeof(bitmap));
		nla_memcpy(bitmap,
			   tb2[QCA_WLAN_VENDOR_ATTR_RATEMASK_PARAMS_BITMAP],
			   len);

		hdd_debug("rate_type:%d, lower32 0x%x, lower32_2 0x%x, higher32 0x%x, higher32_2 0x%x",
			  ratemask_type, bitmap[0], bitmap[1],
			  bitmap[2], bitmap[3]);

		rate_params[num_ratemask].type = ratemask_type;
		rate_params[num_ratemask].lower32 = bitmap[0];
		rate_params[num_ratemask].lower32_2 = bitmap[1];
		rate_params[num_ratemask].higher32 = bitmap[2];
		rate_params[num_ratemask].higher32_2 = bitmap[3];

		num_ratemask += 1;
	}

	ret = ucfg_set_ratemask_params(vdev, num_ratemask, rate_params);
	if (ret)
		hdd_err("ucfg_set_ratemask_params failed");
	return ret;
}

/**
 * __wlan_hdd_cfg80211_set_ratemask_config() - Ratemask parameters
 * @wiphy:                 The wiphy structure
 * @wdev:                  The wireless device
 * @data:                  Data passed by framework
 * @data_len:              Parameters to be configured passed as data
 *
 * The ratemask parameters are configured by the framework
 * using this interface.
 *
 * Return: Return either success or failure code.
 */
static int
__wlan_hdd_cfg80211_set_ratemask_config(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					const void *data, int data_len)
{
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct wlan_objmgr_vdev *vdev;
	int ret;

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

	vdev = hdd_objmgr_get_vdev_by_user(adapter, WLAN_OSIF_POWER_ID);
	if (!vdev) {
		hdd_err("vdev not present");
		return -EINVAL;
	}

	ret = hdd_set_ratemask_params(hdd_ctx, data, data_len, vdev);
	hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_POWER_ID);
	if (ret)
		goto fail;

	return 0;
fail:
	return ret;
}

/**
 * wlan_hdd_cfg80211_set_ratemask_config() - set ratemask config
 * @wiphy:   pointer to wireless wiphy structure.
 * @wdev:    pointer to wireless_dev structure.
 * @data:    Pointer to the data to be passed via vendor interface
 * @data_len:Length of the data to be passed
 *
 * Return:   Return the Success or Failure code.
 */
static int
wlan_hdd_cfg80211_set_ratemask_config(struct wiphy *wiphy,
				      struct wireless_dev *wdev,
				      const void *data,
				      int data_len)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_set_ratemask_config(wiphy, wdev,
							data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}

#define PWR_SAVE_FAIL_CMD_INDEX \
	QCA_NL80211_VENDOR_SUBCMD_PWR_SAVE_FAIL_DETECTED_INDEX

void hdd_chip_pwr_save_fail_detected_cb(hdd_handle_t hdd_handle,
			struct chip_pwr_save_fail_detected_params
			*data)
{
	struct hdd_context *hdd_ctx = hdd_handle_to_context(hdd_handle);
	struct sk_buff *skb;
	int flags = cds_get_gfp_flags();

	hdd_enter();

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
		hdd_info("cfg80211_vendor_event_alloc failed");
		return;
	}

	hdd_debug("failure reason code: %u", data->failure_reason_code);

	if (nla_put_u32(skb,
		QCA_ATTR_CHIP_POWER_SAVE_FAILURE_REASON,
		data->failure_reason_code))
		goto fail;

	cfg80211_vendor_event(skb, flags);
	hdd_exit();
	return;

fail:
	kfree_skb(skb);
}
#undef PWR_SAVE_FAIL_CMD_INDEX

const struct nla_policy
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
	struct hdd_adapter *adapter, *next_adapter = NULL;
	struct hdd_ap_ctx *ap_ctx;
	struct hdd_station_ctx *sta_ctx;
	wlan_net_dev_ref_dbgid dbgid =
				NET_DEV_HOLD_CHECK_DFS_CHANNEL_FOR_ADAPTER;

	hdd_for_each_adapter_dev_held_safe(hdd_ctx, adapter, next_adapter,
					   dbgid) {
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
			 *
			 *  Indoor channels are also marked DFS, therefore
			 *  check if the channel has REGULATORY_CHAN_RADAR
			 *  channel flag to identify if the channel is DFS
			 */
			if (wlan_reg_is_dfs_for_freq(
						hdd_ctx->pdev,
						ap_ctx->operating_chan_freq)) {
				hdd_err("SAP running on DFS channel");
				hdd_adapter_dev_put_debug(adapter, dbgid);
				if (next_adapter)
					hdd_adapter_dev_put_debug(next_adapter,
								  dbgid);
				return true;
			}
		}

		if ((device_mode == adapter->device_mode) &&
		    (device_mode == QDF_STA_MODE)) {
			sta_ctx =
				WLAN_HDD_GET_STATION_CTX_PTR(adapter);
			/*
			 *  if STA is already connected on DFS channel,
			 *  do not disable scan on dfs channels.
			 *
			 *  Indoor channels are also marked DFS, therefore
			 *  check if the channel has REGULATORY_CHAN_RADAR
			 *  channel flag to identify if the channel is DFS
			 */
			if (hdd_cm_is_vdev_associated(adapter) &&
			    wlan_reg_is_dfs_for_freq(
				    hdd_ctx->pdev,
				    sta_ctx->conn_info.chan_freq)) {
				hdd_err("client connected on DFS channel");
				hdd_adapter_dev_put_debug(adapter, dbgid);
				if (next_adapter)
					hdd_adapter_dev_put_debug(next_adapter,
								  dbgid);
				return true;
			}
		}
		hdd_adapter_dev_put_debug(adapter, dbgid);
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
	mac_handle_t mac_handle;
	bool enable_dfs_scan = true;

	ucfg_scan_cfg_get_dfs_chan_scan_allowed(hdd_ctx->psoc,
						&enable_dfs_scan);

	if (enable_dfs_channels == enable_dfs_scan) {
		hdd_debug("DFS channels are already %s",
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

	ucfg_scan_cfg_set_dfs_chan_scan_allowed(hdd_ctx->psoc,
						enable_dfs_channels);

	mac_handle = hdd_ctx->mac_handle;
	status = sme_enable_dfs_chan_scan(mac_handle, enable_dfs_channels);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("Failed to set DFS channel scan flag to %d",
			enable_dfs_channels);
		return qdf_status_to_os_return(status);
	}

	hdd_abort_mac_scan_all_adapters(hdd_ctx);

	/* pass dfs channel status to regulatory component */
	status = ucfg_reg_enable_dfs_channels(hdd_ctx->pdev,
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
	bool enable_dfs_scan = true;
	hdd_enter_dev(dev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

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
	ucfg_scan_cfg_get_dfs_chan_scan_allowed(hdd_ctx->psoc,
						&enable_dfs_scan);

	if (enable_dfs_scan) {
		ret_val = wlan_hdd_enable_dfs_chan_scan(hdd_ctx, !no_dfs_flag);
	} else {
		if ((!no_dfs_flag) != enable_dfs_scan) {
			hdd_err("DFS chan ini configured %d, no dfs flag: %d",
				enable_dfs_scan,
				no_dfs_flag);
			return -EINVAL;
		}
	}

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
	struct osif_psoc_sync *psoc_sync;
	int errno;

	errno = osif_psoc_sync_op_start(wiphy_dev(wiphy), &psoc_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_disable_dfs_chan_scan(wiphy, wdev,
							  data, data_len);

	osif_psoc_sync_op_stop(psoc_sync);

	return errno;
}

const struct nla_policy
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
	mac_handle_t mac_handle;

	hdd_enter_dev(dev);
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
	wisa.vdev_id = adapter->vdev_id;
	mac_handle = hdd_ctx->mac_handle;
	status = sme_set_wisa_params(mac_handle, &wisa);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("Unable to set WISA mode: %d to FW", wisa_mode);
		ret_val = -EINVAL;
	}
	if (QDF_IS_STATUS_SUCCESS(status) || !wisa_mode)
		cdp_set_wisa_mode(soc, adapter->vdev_id, wisa_mode);
err:
	hdd_exit();
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
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_handle_wisa_cmd(wiphy, wdev,
						    data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
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
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *hdd_adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx;
	struct hdd_station_ctx *sta_ctx =
			WLAN_HDD_GET_STATION_CTX_PTR(hdd_adapter);
	struct wlan_crypto_pmksa pmksa;
	int status;
	mac_handle_t mac_handle;

	hdd_enter_dev(dev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	if ((!data) || (data_len <= 0) ||
	    (data_len > ROAM_SCAN_PSK_SIZE)) {
		hdd_err("Invalid data");
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(hdd_adapter);
	if (!hdd_ctx) {
		hdd_err("HDD context is null");
		return -EINVAL;
	}

	status = wlan_hdd_validate_context(hdd_ctx);
	if (status)
		return status;

	mac_handle = hdd_ctx->mac_handle;
	qdf_mem_zero(&pmksa, sizeof(pmksa));
	pmksa.pmk_len = data_len;
	qdf_mem_copy(pmksa.pmk, data, data_len);

	qdf_mem_copy(&pmksa.bssid, &sta_ctx->conn_info.bssid,
		     QDF_MAC_ADDR_SIZE);

	sme_roam_set_psk_pmk(mac_handle, &pmksa, hdd_adapter->vdev_id, true);
	qdf_mem_zero(&pmksa, sizeof(pmksa));
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
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_keymgmt_set_key(wiphy, wdev,
						    data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}
#endif

const struct nla_policy qca_wlan_vendor_get_wifi_info_policy[
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
	uint8_t *firmware_version = NULL;
	int status;
	struct sk_buff *reply_skb;
	uint32_t skb_len = 0, count = 0;
	struct pld_soc_info info;
	bool stt_flag = false;

	hdd_enter_dev(wdev->netdev);

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
		skb_len += strlen(QWLAN_VERSIONSTR) + 1;
		count++;
	}

	if (tb_vendor[QCA_WLAN_VENDOR_ATTR_WIFI_INFO_FIRMWARE_VERSION]) {
		hdd_debug("Rcvd req for FW version");
		if (!pld_get_soc_info(hdd_ctx->parent_dev, &info))
			stt_flag = true;

		firmware_version = qdf_mem_malloc(SIR_VERSION_STRING_LEN);
		if (!firmware_version)
			return -ENOMEM;

		snprintf(firmware_version, SIR_VERSION_STRING_LEN,
			 "FW:%d.%d.%d.%d.%d.%d HW:%s STT:%s",
			 hdd_ctx->fw_version_info.major_spid,
			 hdd_ctx->fw_version_info.minor_spid,
			 hdd_ctx->fw_version_info.siid,
			 hdd_ctx->fw_version_info.rel_id,
			 hdd_ctx->fw_version_info.crmid,
			 hdd_ctx->fw_version_info.sub_id,
			 hdd_ctx->target_hw_name,
			 (stt_flag ? info.fw_build_id : " "));
		skb_len += strlen(firmware_version) + 1;
		count++;
	}

	if (tb_vendor[QCA_WLAN_VENDOR_ATTR_WIFI_INFO_RADIO_INDEX]) {
		hdd_debug("Rcvd req for Radio index");
		skb_len += sizeof(uint32_t);
		count++;
	}

	if (count == 0) {
		hdd_err("unknown attribute in get_wifi_info request");
		qdf_mem_free(firmware_version);
		return -EINVAL;
	}

	skb_len += (NLA_HDRLEN * count) + NLMSG_HDRLEN;
	reply_skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, skb_len);

	if (!reply_skb) {
		hdd_err("cfg80211_vendor_cmd_alloc_reply_skb failed");
		qdf_mem_free(firmware_version);
		return -ENOMEM;
	}

	if (tb_vendor[QCA_WLAN_VENDOR_ATTR_WIFI_INFO_DRIVER_VERSION]) {
		if (nla_put_string(reply_skb,
			    QCA_WLAN_VENDOR_ATTR_WIFI_INFO_DRIVER_VERSION,
			    QWLAN_VERSIONSTR))
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

	qdf_mem_free(firmware_version);
	return cfg80211_vendor_cmd_reply(reply_skb);

error_nla_fail:
	hdd_err("nla put fail");
	qdf_mem_free(firmware_version);
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
	struct osif_psoc_sync *psoc_sync;
	int errno;

	errno = osif_psoc_sync_op_start(wiphy_dev(wiphy), &psoc_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_get_wifi_info(wiphy, wdev, data, data_len);

	osif_psoc_sync_op_stop(psoc_sync);

	return errno;
}

const struct nla_policy get_logger_set_policy[
			QCA_WLAN_VENDOR_ATTR_LOGGER_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_LOGGER_SUPPORTED] = {.type = NLA_U32},
};

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
	bool enable_ring_buffer;

	hdd_enter_dev(wdev->netdev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	status = wlan_hdd_validate_context(hdd_ctx);
	if (status)
		return status;

	features = 0;
	wlan_mlme_get_status_ring_buffer(hdd_ctx->psoc, &enable_ring_buffer);
	if (enable_ring_buffer) {
		features |= WIFI_LOGGER_PER_PACKET_TX_RX_STATUS_SUPPORTED;
		features |= WIFI_LOGGER_CONNECT_EVENT_SUPPORTED;
		features |= WIFI_LOGGER_WAKE_LOCK_SUPPORTED;
		features |= WIFI_LOGGER_DRIVER_DUMP_SUPPORTED;
		features |= WIFI_LOGGER_PACKET_FATE_SUPPORTED;
		hdd_debug("Supported logger features: 0x%0x", features);
	} else {
		hdd_info("Ring buffer disable");
	}

	reply_skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy,
			sizeof(uint32_t) + NLA_HDRLEN + NLMSG_HDRLEN);
	if (!reply_skb) {
		hdd_err("cfg80211_vendor_cmd_alloc_reply_skb failed");
		return -ENOMEM;
	}

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
	struct osif_psoc_sync *psoc_sync;
	int errno;

	errno = osif_psoc_sync_op_start(wiphy_dev(wiphy), &psoc_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_get_logger_supp_feature(wiphy, wdev,
							    data, data_len);

	osif_psoc_sync_op_stop(psoc_sync);

	return errno;
}

#ifdef WLAN_FEATURE_GTK_OFFLOAD
void wlan_hdd_save_gtk_offload_params(struct hdd_adapter *adapter,
				      uint8_t *kck_ptr, uint8_t kck_len,
				      uint8_t *kek_ptr, uint32_t kek_len,
				      uint8_t *replay_ctr, bool big_endian)
{
	struct hdd_station_ctx *hdd_sta_ctx;
	uint8_t *buf;
	int i;
	struct pmo_gtk_req *gtk_req = NULL;
	struct wlan_objmgr_vdev *vdev;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct hdd_context *hdd_ctx =  WLAN_HDD_GET_CTX(adapter);

	gtk_req = qdf_mem_malloc(sizeof(*gtk_req));
	if (!gtk_req)
		return;

	hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	if (kck_ptr) {
		if (kck_len > sizeof(gtk_req->kck)) {
			kck_len = sizeof(gtk_req->kck);
			QDF_ASSERT(0);
		}
		qdf_mem_copy(gtk_req->kck, kck_ptr, kck_len);
		gtk_req->kck_len = kck_len;
	}

	if (kek_ptr) {
		/* paranoia */
		if (kek_len > sizeof(gtk_req->kek)) {
			kek_len = sizeof(gtk_req->kek);
			QDF_ASSERT(0);
		}
		qdf_mem_copy(gtk_req->kek, kek_ptr, kek_len);
	}

	qdf_copy_macaddr(&gtk_req->bssid, &hdd_sta_ctx->conn_info.bssid);

	gtk_req->kek_len = kek_len;
	gtk_req->is_fils_connection = hdd_is_fils_connection(hdd_ctx, adapter);

	/* convert big to little endian since driver work on little endian */
	buf = (uint8_t *)&gtk_req->replay_counter;
	for (i = 0; i < 8; i++)
		buf[7 - i] = replay_ctr[i];

	vdev = hdd_objmgr_get_vdev_by_user(adapter, WLAN_OSIF_POWER_ID);
	if (!vdev)
		goto end;
	status = ucfg_pmo_cache_gtk_offload_req(vdev, gtk_req);
	hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_POWER_ID);
	if (status != QDF_STATUS_SUCCESS)
		hdd_err("Failed to cache GTK Offload");

end:
	qdf_mem_free(gtk_req);
}
#endif

#ifdef WLAN_CFR_ENABLE
void hdd_cfr_data_send_nl_event(uint8_t vdev_id, uint32_t pid,
				const void *data, uint32_t data_len)
{
	uint32_t len, ret;
	struct sk_buff *vendor_event;
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	struct hdd_adapter *adapter;
	struct nlmsghdr *nlhdr = NULL;

	if (!hdd_ctx) {
		hdd_err("HDD context is NULL");
		return;
	}

	adapter = hdd_get_adapter_by_vdev(hdd_ctx, vdev_id);
	if (!adapter) {
		hdd_err("adapter NULL for vdev id %d", vdev_id);
		return;
	}

	hdd_debug("vdev id %d pid %d data len %d", vdev_id, pid, data_len);
	len = nla_total_size(data_len) + NLMSG_HDRLEN;
	vendor_event = cfg80211_vendor_event_alloc(
			hdd_ctx->wiphy, &adapter->wdev, len,
			QCA_NL80211_VENDOR_SUBCMD_PEER_CFR_CAPTURE_CFG_INDEX,
			GFP_KERNEL);

	if (!vendor_event) {
		hdd_err("cfg80211_vendor_event_alloc failed vdev id %d, data len %d",
			vdev_id, data_len);
		return;
	}

	ret = nla_put(vendor_event,
		      QCA_WLAN_VENDOR_ATTR_PEER_CFR_RESP_DATA,
		      data_len, data);
	if (ret) {
		hdd_err("CFR event put fails status %d", ret);
		kfree_skb(vendor_event);
		return;
	}

	if (pid) {
		nlhdr = nlmsg_hdr(vendor_event);
		if (nlhdr)
			nlhdr->nlmsg_pid = pid;
		else
			hdd_err_rl("nlhdr is null");
	}

	cfg80211_vendor_event(vendor_event, GFP_ATOMIC);
}
#endif

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
void hdd_send_roam_scan_ch_list_event(struct hdd_context *hdd_ctx,
				      uint8_t vdev_id, uint16_t buf_len,
				      uint8_t *buf)
{
	struct sk_buff *vendor_event;
	struct hdd_adapter *adapter;
	uint32_t len, ret;

	if (!hdd_ctx) {
		hdd_err_rl("hdd context is null");
		return;
	}

	adapter = hdd_get_adapter_by_vdev(hdd_ctx, vdev_id);
	if (!adapter)
		return;

	len = nla_total_size(buf_len) + NLMSG_HDRLEN;
	vendor_event =
		cfg80211_vendor_event_alloc(
			hdd_ctx->wiphy, &(adapter->wdev), len,
			QCA_NL80211_VENDOR_SUBCMD_UPDATE_STA_INFO_INDEX,
			GFP_KERNEL);

	if (!vendor_event) {
		hdd_err("cfg80211_vendor_event_alloc failed");
		return;
	}

	ret = nla_put(vendor_event,
		      QCA_WLAN_VENDOR_ATTR_UPDATE_STA_INFO_CONNECT_CHANNELS,
		      buf_len, buf);
	if (ret) {
		hdd_err("OEM event put fails status %d", ret);
		kfree_skb(vendor_event);
		return;
	}

	cfg80211_vendor_event(vendor_event, GFP_KERNEL);
}
#endif

#define ANT_DIV_SET_PERIOD(probe_period, stay_period) \
	((1 << 26) | \
	 (((probe_period) & 0x1fff) << 13) | \
	 ((stay_period) & 0x1fff))

#define ANT_DIV_SET_SNR_DIFF(snr_diff) \
	((1 << 27) | \
	 ((snr_diff) & 0x1fff))

#define ANT_DIV_SET_PROBE_DWELL_TIME(probe_dwell_time) \
	((1 << 28) | \
	 ((probe_dwell_time) & 0x1fff))

#define ANT_DIV_SET_WEIGHT(mgmt_snr_weight, data_snr_weight, ack_snr_weight) \
	((1 << 29) | \
	 (((mgmt_snr_weight) & 0xff) << 16) | \
	 (((data_snr_weight) & 0xff) << 8) | \
	 ((ack_snr_weight) & 0xff))

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

const struct nla_policy wlan_hdd_wifi_config_policy[
			QCA_WLAN_VENDOR_ATTR_CONFIG_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_CONFIG_PENALIZE_AFTER_NCONS_BEACON_MISS] = {
							.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_SCAN_DEFAULT_IES] = {.type = NLA_BINARY},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_COMMAND] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_VALUE] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_DATA] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_LENGTH] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_FLAGS] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_IFINDEX] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_RX_REORDER_TIMEOUT_VOICE] = {
		.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_RX_REORDER_TIMEOUT_VIDEO] = {
		.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_RX_REORDER_TIMEOUT_BESTEFFORT] = {
		.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_RX_REORDER_TIMEOUT_BACKGROUND] = {
		.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_RX_BLOCKSIZE_PEER_MAC] =
		VENDOR_NLA_POLICY_MAC_ADDR,
	[QCA_WLAN_VENDOR_ATTR_CONFIG_RX_BLOCKSIZE_WINLIMIT] = {
		.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_BEACON_MISS_THRESHOLD_24] = {
		.type = NLA_U8},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_BEACON_MISS_THRESHOLD_5] = {
		.type = NLA_U8},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_TOTAL_BEACON_MISS_COUNT] = {
		.type = NLA_U8},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_BEACON_REPORT_FAIL] = {.type = NLA_U8},
	[QCA_WLAN_VENDOR_ATTR_CONF_TX_RATE] = {.type = NLA_U16},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_MODULATED_DTIM] = {.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_IGNORE_ASSOC_DISALLOWED] = {
		.type = NLA_U8},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_DISABLE_FILS] = {.type = NLA_U8 },
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
	[QCA_WLAN_VENDOR_ATTR_CONFIG_PROPAGATION_ABS_DELAY] = {
		.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_TX_FAIL_COUNT] = {.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_PROBE_PERIOD] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_STAY_PERIOD] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_SNR_DIFF] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_PROBE_DWELL_TIME] = {
		.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_MGMT_SNR_WEIGHT] = {
		.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_DATA_SNR_WEIGHT] = {
		.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_ACK_SNR_WEIGHT] = {
		.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_RESTRICT_OFFCHANNEL] = {.type = NLA_U8},
	[RX_REORDER_TIMEOUT_VOICE] = {.type = NLA_U32},
	[RX_REORDER_TIMEOUT_VIDEO] = {.type = NLA_U32},
	[RX_REORDER_TIMEOUT_BESTEFFORT] = {.type = NLA_U32},
	[RX_REORDER_TIMEOUT_BACKGROUND] = {.type = NLA_U32},
	[RX_BLOCKSIZE_PEER_MAC] = VENDOR_NLA_POLICY_MAC_ADDR,
	[RX_BLOCKSIZE_WINLIMIT] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_LISTEN_INTERVAL] = {.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_LRO] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_QPOWER] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_ENA] = {.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_CHAIN] = {.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_SELFTEST] = {.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_SELFTEST_INTVL] = {
						.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL] = {.type = NLA_U16 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_TOTAL_BEACON_MISS_COUNT] = {
						.type = NLA_U8},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_SCAN_ENABLE] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_RSN_IE] = {.type = NLA_U8},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_GTX] = {.type = NLA_U8},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_ELNA_BYPASS] = {.type = NLA_U8},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_ACCESS_POLICY] = {.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_ACCESS_POLICY_IE_LIST] = {
		.type = NLA_BINARY,
		.len = WLAN_MAX_IE_LEN + 2},
	[QCA_WLAN_VENDOR_ATTR_DISCONNECT_IES] = {
		.type = NLA_BINARY,
		.len = SIR_MAC_MAX_ADD_IE_LENGTH + 2},
	[QCA_WLAN_VENDOR_ATTR_CONFIG_ROAM_REASON] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_TX_MSDU_AGGREGATION] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_RX_MSDU_AGGREGATION] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_LDPC] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_TX_STBC] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_RX_STBC] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_PHY_MODE] = {.type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_CHANNEL_WIDTH] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_DYNAMIC_BW] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_NSS] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_OPTIMIZED_POWER_MANAGEMENT] = {
		.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_UDP_QOS_UPGRADE] = {
		.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_NUM_TX_CHAINS] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_NUM_RX_CHAINS] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_ANI_SETTING] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_ANI_LEVEL] = {.type = NLA_S32 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_TX_NSS] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_RX_NSS] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_CONCURRENT_STA_PRIMARY] = {
							.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_FT_OVER_DS] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_ARP_NS_OFFLOAD] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_CONFIG_WFC_STATE] = {
		.type = NLA_U8 },
};

static const struct nla_policy
qca_wlan_vendor_attr_he_omi_tx_policy [QCA_WLAN_VENDOR_ATTR_HE_OMI_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_HE_OMI_RX_NSS] =       {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_HE_OMI_CH_BW] =        {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_HE_OMI_ULMU_DISABLE] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_HE_OMI_TX_NSTS] =      {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_HE_OMI_ULMU_DATA_DISABLE] = {.type = NLA_U8 },
};

static const struct nla_policy
wlan_oci_override_policy [QCA_WLAN_VENDOR_ATTR_OCI_OVERRIDE_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_OCI_OVERRIDE_FRAME_TYPE] = {.type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_OCI_OVERRIDE_FREQUENCY] = {.type = NLA_U32 },
};

const struct nla_policy
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
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_KEEP_ALIVE_FRAME_TYPE] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_BA_TID] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_ADDBA_BUFF_SIZE] = {
			.type = NLA_U16},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_ENABLE_NO_ACK] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_NO_ACK_AC] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_LTF] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_ENABLE_TX_BEAMFORMEE] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_RX_CTRL_FRAME_TO_MBSS] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_BCAST_TWT_SUPPORT] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_TX_BEAMFORMEE_NSTS] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_MAC_PADDING_DUR] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_MU_EDCA_AC] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_OVERRIDE_MU_EDCA] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_OM_CTRL_SUPP] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_OM_CTRL_BW] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_OM_CTRL_NSS] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_CLEAR_HE_OM_CTRL_CONFIG] = {
			.type = NLA_FLAG},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_TX_SUPPDU] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_USE_BSSID_IN_PROBE_REQ_RA] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_HTC_HE_SUPP] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_OMI_TX] = {
			.type = NLA_NESTED},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_ACTION_TX_TB_PPDU] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_SET_HE_TESTBED_DEFAULTS]
			= {.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_ENABLE_2G_VHT] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_TWT_SETUP] = {
			.type = NLA_NESTED},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_TWT_TERMINATE] = {
			.type = NLA_NESTED},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_TWT_REQ_SUPPORT] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_BSS_MAX_IDLE_PERIOD_ENABLE] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_PMF_PROTECTION] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_DISABLE_DATA_MGMT_RSP_TX]
			= {.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_BSS_MAX_IDLE_PERIOD] = {
			.type = NLA_U16},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_ER_SU_PPDU_TYPE] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_PUNCTURED_PREAMBLE_RX] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_DISASSOC_TX] = {
			.type = NLA_FLAG},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_FT_REASSOCREQ_RSNXE_USED] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_IGNORE_CSA] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_OCI_OVERRIDE] = {
			.type = NLA_NESTED},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_IGNORE_SA_QUERY_TIMEOUT] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_FILS_DISCOVERY_FRAMES_TX] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_FULL_BW_UL_MU_MIMO] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_RU_242_TONE_TX] = {
			.type = NLA_U8},
		[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_6GHZ_SECURITY_TEST_MODE]
			= {.type = NLA_U8},
};

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
	bool add_qcn_ie;

	if (!scan_info)
		return -EINVAL;

	if (scan_info->default_scan_ies) {
		qdf_mem_free(scan_info->default_scan_ies);
		scan_info->default_scan_ies = NULL;
	}

	scan_info->default_scan_ies_len = ie_len;
	ucfg_mlme_get_qcn_ie_support(hdd_ctx->psoc, &add_qcn_ie);
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
		sme_add_qcn_ie(hdd_ctx->mac_handle,
			       scan_info->default_scan_ies,
			       &scan_info->default_scan_ies_len);

	hdd_debug("Saved default scan IE:len %d",
		  scan_info->default_scan_ies_len);
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
	struct wlan_objmgr_vdev *vdev;
	int ret_val = 0;

	if (!(dev_mode == QDF_SAP_MODE || dev_mode == QDF_P2P_GO_MODE)) {
		hdd_err("Invalid interface type:%d", dev_mode);
		return -EINVAL;
	}
	vdev = hdd_objmgr_get_vdev_by_user(adapter, WLAN_OSIF_ID);
	if (!vdev)
		return -EINVAL;
	if (restrict_offchan == 1) {
		enum policy_mgr_con_mode pmode =
		policy_mgr_convert_device_mode_to_qdf_type(dev_mode);
		uint32_t freq;

		u32 vdev_id = wlan_vdev_get_id(vdev);

		wlan_vdev_obj_lock(vdev);
		wlan_vdev_mlme_cap_set(vdev, WLAN_VDEV_C_RESTRICT_OFFCHAN);
		wlan_vdev_obj_unlock(vdev);
		freq = policy_mgr_get_channel(hdd_ctx->psoc, pmode, &vdev_id);
		if (!freq ||
		    wlan_hdd_send_avoid_freq_for_dnbs(hdd_ctx, freq)) {
			hdd_err("unable to send avoid_freq");
			ret_val = -EINVAL;
		}
		hdd_info("vdev %d mode %d dnbs enabled", vdev_id, dev_mode);
	} else if (restrict_offchan == 0) {
		wlan_vdev_obj_lock(vdev);
		wlan_vdev_mlme_cap_clear(vdev, WLAN_VDEV_C_RESTRICT_OFFCHAN);
		wlan_vdev_obj_unlock(vdev);
		if (wlan_hdd_send_avoid_freq_for_dnbs(hdd_ctx, 0)) {
			hdd_err("unable to clear avoid_freq");
			ret_val = -EINVAL;
		}
		hdd_info("vdev mode %d dnbs disabled", dev_mode);
	} else {
		ret_val = -EINVAL;
		hdd_err("Invalid RESTRICT_OFFCHAN setting");
	}
	hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_ID);
	return ret_val;
}

/**
 * wlan_hdd_cfg80211_wifi_set_reorder_timeout() - set reorder timeout
 * @adapter: Pointer to HDD adapter
 * @tb: array of pointer to struct nlattr
 *
 * Return: 0 on success; error number otherwise
 */
static
int wlan_hdd_cfg80211_wifi_set_reorder_timeout(struct hdd_adapter *adapter,
					       struct nlattr *tb[])
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	int ret_val = 0;
	QDF_STATUS qdf_status;
	struct sir_set_rx_reorder_timeout_val reorder_timeout;
	mac_handle_t mac_handle;

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
			mac_handle = hdd_ctx->mac_handle;
			qdf_status = sme_set_reorder_timeout(mac_handle,
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
 * wlan_hdd_cfg80211_wifi_set_rx_blocksize() - set rx blocksize
 * @adapter: hdd adapter
 * @tb: array of pointer to struct nlattr
 *
 * Return: 0 on success; error number otherwise
 */
static int wlan_hdd_cfg80211_wifi_set_rx_blocksize(struct hdd_adapter *adapter,
						   struct nlattr *tb[])
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	int ret_val = 0;
	uint32_t set_value;
	QDF_STATUS qdf_status;
	struct sir_peer_set_rx_blocksize rx_blocksize;
	mac_handle_t mac_handle;

#define WINDOW_SIZE_VAL_MIN 1
#define WINDOW_SIZE_VAL_MAX 64

	if (tb[RX_BLOCKSIZE_PEER_MAC] ||
	    tb[RX_BLOCKSIZE_WINLIMIT]) {

		/* if one is specified, both must be specified */
		if (!tb[RX_BLOCKSIZE_PEER_MAC] ||
		    !tb[RX_BLOCKSIZE_WINLIMIT]) {
			hdd_err("Both Peer MAC and windows limit required");
			return -EINVAL;
		}

		memcpy(&rx_blocksize.peer_macaddr,
		       nla_data(tb[RX_BLOCKSIZE_PEER_MAC]),
		       sizeof(rx_blocksize.peer_macaddr)),

		rx_blocksize.vdev_id = adapter->vdev_id;
		set_value = nla_get_u32(tb[RX_BLOCKSIZE_WINLIMIT]);
		/* maximum window size is 64 */
		if (set_value >= WINDOW_SIZE_VAL_MIN &&
		    set_value <= WINDOW_SIZE_VAL_MAX) {
			rx_blocksize.rx_block_ack_win_limit = set_value;
			mac_handle = hdd_ctx->mac_handle;
			qdf_status = sme_set_rx_set_blocksize(mac_handle,
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

int hdd_set_phy_mode(struct hdd_adapter *adapter,
		     enum qca_wlan_vendor_phy_mode vendor_phy_mode)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct wlan_objmgr_psoc *psoc = hdd_ctx->psoc;
	eCsrPhyMode phymode;
	uint8_t supported_band;
	uint32_t bonding_mode;
	int ret = 0;

	if (!psoc) {
		hdd_err("psoc is NULL");
		return -EINVAL;
	}

	ret = hdd_vendor_mode_to_phymode(vendor_phy_mode, &phymode);
	if (ret < 0)
		return ret;

	ret = hdd_vendor_mode_to_band(vendor_phy_mode, &supported_band,
				      wlan_reg_is_6ghz_supported(psoc));
	if (ret < 0)
		return ret;

	ret = hdd_vendor_mode_to_bonding_mode(vendor_phy_mode, &bonding_mode);
	if (ret < 0)
		return ret;

	return hdd_update_phymode(adapter, phymode, supported_band,
				  bonding_mode);
}

/**
 * hdd_config_phy_mode() - set PHY mode
 * @adapter: hdd adapter
 * @attr: nla attr sent from userspace
 *
 * Return: 0 on success; error number otherwise
 */
static int hdd_config_phy_mode(struct hdd_adapter *adapter,
			       const struct nlattr *attr)
{
	enum qca_wlan_vendor_phy_mode vendor_phy_mode;

	vendor_phy_mode = nla_get_u32(attr);

	return hdd_set_phy_mode(adapter, vendor_phy_mode);
}

/**
 * hdd_set_roam_reason_vsie_status() - enable/disable inclusion of
 * roam reason vsie in Reassoc
 *
 * @adapter: hdd adapter
 * @attr: nla attr sent by supplicant
 *
 * Return: 0 on success, negative errno on failure
 */
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
static int hdd_set_roam_reason_vsie_status(struct hdd_adapter *adapter,
					   const struct nlattr *attr)
{
	uint8_t roam_reason_vsie_enabled;
	int errno;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct hdd_context *hdd_ctx = NULL;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (!hdd_ctx) {
		hdd_err("hdd_ctx failure");
		return -EINVAL;
	}

	roam_reason_vsie_enabled = nla_get_u8(attr);
	if (roam_reason_vsie_enabled > 1)
		roam_reason_vsie_enabled = 1;

	status =
		ucfg_mlme_set_roam_reason_vsie_status(hdd_ctx->psoc,
						      roam_reason_vsie_enabled);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("set roam reason vsie failed");
		return -EINVAL;
	}

	errno = sme_cli_set_command
			(adapter->vdev_id,
			 WMI_VDEV_PARAM_ENABLE_DISABLE_ROAM_REASON_VSIE,
			 roam_reason_vsie_enabled, VDEV_CMD);
	if (errno) {
		hdd_err("Failed to set beacon report error vsie");
		status = QDF_STATUS_E_FAILURE;
	}

	return qdf_status_to_os_return(status);
}
#else
static int hdd_set_roam_reason_vsie_status(struct hdd_adapter *adapter,
					   const struct nlattr *attr)
{
	return -ENOTSUPP;
}
#endif

static int hdd_set_ft_over_ds(struct hdd_adapter *adapter,
			      const struct nlattr *attr)
{
	uint8_t ft_over_ds_enable;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct hdd_context *hdd_ctx = NULL;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (!hdd_ctx) {
		hdd_err("hdd_ctx failure");
		return -EINVAL;
	}

	ft_over_ds_enable = nla_get_u8(attr);

	if (ft_over_ds_enable != 0 && ft_over_ds_enable != 1) {
		hdd_err_rl("Invalid ft_over_ds_enable: %d", ft_over_ds_enable);
		return -EINVAL;
	}

	status = ucfg_mlme_set_ft_over_ds(hdd_ctx->psoc, ft_over_ds_enable);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("set ft_over_ds failed");
		return -EINVAL;
	}

	return status;
}

static int hdd_config_ldpc(struct hdd_adapter *adapter,
			   const struct nlattr *attr)
{
	uint8_t ldpc;
	int ret;

	ldpc = nla_get_u8(attr);

	ret = hdd_set_ldpc(adapter, ldpc);

	return ret;
}

static int hdd_config_tx_stbc(struct hdd_adapter *adapter,
			      const struct nlattr *attr)
{
	uint8_t tx_stbc;
	int ret;

	tx_stbc = nla_get_u8(attr);

	ret = hdd_set_tx_stbc(adapter, tx_stbc);

	return ret;
}

static int hdd_config_rx_stbc(struct hdd_adapter *adapter,
			      const struct nlattr *attr)
{
	uint8_t rx_stbc;
	int ret;

	rx_stbc = nla_get_u8(attr);

	ret = hdd_set_rx_stbc(adapter, rx_stbc);

	return ret;
}

static int hdd_config_access_policy(struct hdd_adapter *adapter,
				    struct nlattr *tb[])
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct nlattr *policy_attr =
		tb[QCA_WLAN_VENDOR_ATTR_CONFIG_ACCESS_POLICY];
	struct nlattr *ielist_attr =
		tb[QCA_WLAN_VENDOR_ATTR_CONFIG_ACCESS_POLICY_IE_LIST];
	uint32_t access_policy;
	uint8_t ie[WLAN_MAX_IE_LEN + 2];
	QDF_STATUS status;

	/* nothing to do if neither attribute is present */
	if (!ielist_attr && !policy_attr)
		return 0;

	/* if one is present, both must be present */
	if (!ielist_attr || !policy_attr) {
		hdd_err("Missing attribute for %s",
			policy_attr ?
				"ACCESS_POLICY_IE_LIST" : "ACCESS_POLICY");
		return -EINVAL;
	}

	/* validate the access policy */
	access_policy = nla_get_u32(policy_attr);
	switch (access_policy) {
	case QCA_ACCESS_POLICY_ACCEPT_UNLESS_LISTED:
	case QCA_ACCESS_POLICY_DENY_UNLESS_LISTED:
		/* valid */
		break;
	default:
		hdd_err("Invalid value. access_policy %u", access_policy);
		return -EINVAL;
	}

	/*
	 * ie length is validated by the nla_policy.  need to make a
	 * copy since SME will always read WLAN_MAX_IE_LEN+2 bytes
	 */
	nla_memcpy(ie, ielist_attr, sizeof(ie));

	hdd_debug("calling sme_update_access_policy_vendor_ie");
	status = sme_update_access_policy_vendor_ie(hdd_ctx->mac_handle,
						    adapter->vdev_id,
						    ie, access_policy);
	if (QDF_IS_STATUS_ERROR(status))
		hdd_err("Failed to set vendor ie and access policy, %d",
			status);

	return qdf_status_to_os_return(status);
}

static int hdd_config_mpdu_aggregation(struct hdd_adapter *adapter,
				       struct nlattr *tb[])
{
	struct nlattr *tx_attr =
		tb[QCA_WLAN_VENDOR_ATTR_CONFIG_TX_MPDU_AGGREGATION];
	struct nlattr *rx_attr =
		tb[QCA_WLAN_VENDOR_ATTR_CONFIG_RX_MPDU_AGGREGATION];
	uint8_t tx_size, rx_size;
	QDF_STATUS status;

	/* nothing to do if neither attribute is present */
	if (!tx_attr && !rx_attr)
		return 0;

	/* if one is present, both must be present */
	if (!tx_attr || !rx_attr) {
		hdd_err("Missing attribute for %s",
			tx_attr ? "RX" : "TX");
		return -EINVAL;
	}

	tx_size = nla_get_u8(tx_attr);
	rx_size = nla_get_u8(rx_attr);
	if (!cfg_in_range(CFG_TX_AGGREGATION_SIZE, tx_size) ||
	    !cfg_in_range(CFG_RX_AGGREGATION_SIZE, rx_size)) {
		hdd_err("TX %d RX %d MPDU aggr size not in range",
			tx_size, rx_size);

		return -EINVAL;
	}

	status = wma_set_tx_rx_aggr_size(adapter->vdev_id,
					 tx_size,
					 rx_size,
					 WMI_VDEV_CUSTOM_AGGR_TYPE_AMPDU);

	return qdf_status_to_os_return(status);
}

static int hdd_config_msdu_aggregation(struct hdd_adapter *adapter,
				       struct nlattr *tb[])
{
	struct nlattr *tx_attr =
		tb[QCA_WLAN_VENDOR_ATTR_CONFIG_TX_MSDU_AGGREGATION];
	struct nlattr *rx_attr =
		tb[QCA_WLAN_VENDOR_ATTR_CONFIG_RX_MSDU_AGGREGATION];
	uint8_t tx_size, rx_size;
	QDF_STATUS status;

	/* nothing to do if neither attribute is present */
	if (!tx_attr && !rx_attr)
		return 0;

	/* if one is present, both must be present */
	if (!tx_attr || !rx_attr) {
		hdd_err("Missing attribute for %s",
			tx_attr ? "RX" : "TX");
		return -EINVAL;
	}

	tx_size = nla_get_u8(tx_attr);
	rx_size = nla_get_u8(rx_attr);
	if (!cfg_in_range(CFG_TX_AGGREGATION_SIZE, tx_size) ||
	    !cfg_in_range(CFG_RX_AGGREGATION_SIZE, rx_size)) {
		hdd_err("TX %d RX %d MSDU aggr size not in range",
			tx_size, rx_size);

		return -EINVAL;
	}

	status = wma_set_tx_rx_aggr_size(adapter->vdev_id,
					 tx_size,
					 rx_size,
					 WMI_VDEV_CUSTOM_AGGR_TYPE_AMSDU);

	return qdf_status_to_os_return(status);
}

static QDF_STATUS
hdd_populate_vdev_chains(struct wlan_mlme_nss_chains *nss_chains_cfg,
			 uint8_t tx_chains,
			 uint8_t rx_chains,
			 enum nss_chains_band_info band,
			 struct wlan_objmgr_vdev *vdev)
{
	struct wlan_mlme_nss_chains *dynamic_cfg;

	nss_chains_cfg->num_rx_chains[band] = rx_chains;
	nss_chains_cfg->num_tx_chains[band] = tx_chains;

	dynamic_cfg = ucfg_mlme_get_dynamic_vdev_config(vdev);
	if (!dynamic_cfg) {
		hdd_err("nss chain dynamic config NULL");
		return QDF_STATUS_E_FAILURE;
	}
	/*
	 * If user gives any nss value, then chains will be adjusted based on
	 * nss (in SME func sme_validate_user_nss_chain_params).
	 * If Chains are not suitable as per current NSS then, we need to
	 * return, and the below logic is added for the same.
	 */

	if ((dynamic_cfg->rx_nss[band] > rx_chains) ||
	    (dynamic_cfg->tx_nss[band] > tx_chains)) {
		hdd_err("Chains less than nss, configure correct nss first.");
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

int
hdd_set_dynamic_antenna_mode(struct hdd_adapter *adapter,
			     uint8_t num_rx_chains,
			     uint8_t num_tx_chains)
{
	enum nss_chains_band_info band;
	struct wlan_mlme_nss_chains user_cfg;
	QDF_STATUS status;
	mac_handle_t mac_handle;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct wlan_objmgr_vdev *vdev;
	int ret;

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	mac_handle = hdd_ctx->mac_handle;
	if (!mac_handle) {
		hdd_err("NULL MAC handle");
		return -EINVAL;
	}

	if (!hdd_is_vdev_in_conn_state(adapter)) {
		hdd_debug("Vdev (id %d) not in connected/started state, cannot accept command",
			  adapter->vdev_id);
		return -EINVAL;
	}

	vdev = hdd_objmgr_get_vdev_by_user(adapter, WLAN_OSIF_ID);
	if (!vdev) {
		hdd_err("vdev is NULL");
		return -EINVAL;
	}

	qdf_mem_zero(&user_cfg, sizeof(user_cfg));
	for (band = NSS_CHAINS_BAND_2GHZ; band < NSS_CHAINS_BAND_MAX; band++) {
		status = hdd_populate_vdev_chains(&user_cfg,
						  num_rx_chains,
						  num_tx_chains, band, vdev);
		if (QDF_IS_STATUS_ERROR(status)) {
			hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_ID);
			return -EINVAL;
		}
	}
	hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_ID);

	status = sme_nss_chains_update(mac_handle,
				       &user_cfg,
				       adapter->vdev_id);
	if (QDF_IS_STATUS_ERROR(status))
		return -EINVAL;

	return 0;
}

static int hdd_config_vdev_chains(struct hdd_adapter *adapter,
				  struct nlattr *tb[])
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	uint8_t tx_chains, rx_chains;
	struct nlattr *tx_attr =
		tb[QCA_WLAN_VENDOR_ATTR_CONFIG_NUM_TX_CHAINS];
	struct nlattr *rx_attr =
		tb[QCA_WLAN_VENDOR_ATTR_CONFIG_NUM_RX_CHAINS];

	if (!tx_attr && !rx_attr)
		return 0;

	/* if one is present, both must be present */
	if (!tx_attr || !rx_attr) {
		hdd_err("Missing attribute for %s",
			tx_attr ? "RX" : "TX");
		return -EINVAL;
	}

	tx_chains = nla_get_u8(tx_attr);
	rx_chains = nla_get_u8(rx_attr);

	if (hdd_ctx->dynamic_nss_chains_support)
		return hdd_set_dynamic_antenna_mode(adapter, rx_chains,
						    tx_chains);
	return 0;
}

static int hdd_config_tx_rx_nss(struct hdd_adapter *adapter,
				  struct nlattr *tb[])
{
	uint8_t tx_nss, rx_nss;
	QDF_STATUS status;

	struct nlattr *tx_attr =
		tb[QCA_WLAN_VENDOR_ATTR_CONFIG_TX_NSS];
	struct nlattr *rx_attr =
		tb[QCA_WLAN_VENDOR_ATTR_CONFIG_RX_NSS];

	if (!tx_attr && !rx_attr)
		return 0;

	/* if one is present, both must be present */
	if (!tx_attr || !rx_attr) {
		hdd_err("Missing attribute for %s",
			tx_attr ? "RX" : "TX");
		return -EINVAL;
	}

	tx_nss = nla_get_u8(tx_attr);
	rx_nss = nla_get_u8(rx_attr);
	hdd_debug("tx_nss %d rx_nss %d", tx_nss, rx_nss);
	/* Only allow NSS for tx_rx_nss for 1x1, 1x2, 2x2 */
	if (!((tx_nss == 1 && rx_nss == 2) || (tx_nss == 1 && rx_nss == 1) ||
	      (tx_nss == 2 && rx_nss == 2))) {
		hdd_err("Setting tx_nss %d rx_nss %d not allowed", tx_nss,
			rx_nss);
		return 0;
	}
	status = hdd_update_nss(adapter, tx_nss, rx_nss);
	if (status != QDF_STATUS_SUCCESS)
		hdd_debug("Can't set tx_nss %d rx_nss %d", tx_nss, rx_nss);

	return 0;
}

static int hdd_config_ani(struct hdd_adapter *adapter,
			  struct nlattr *tb[])
{
	int errno;
	uint8_t ani_setting_type;
	int32_t ani_level = 0, enable_ani;
	struct nlattr *ani_setting_attr =
		tb[QCA_WLAN_VENDOR_ATTR_CONFIG_ANI_SETTING];
	struct nlattr *ani_level_attr =
		tb[QCA_WLAN_VENDOR_ATTR_CONFIG_ANI_LEVEL];

	if (!ani_setting_attr)
		return 0;

	ani_setting_type = nla_get_u8(ani_setting_attr);
	if (ani_setting_type != QCA_WLAN_ANI_SETTING_AUTO &&
	    ani_setting_type != QCA_WLAN_ANI_SETTING_FIXED) {
		hdd_err("invalid ani_setting_type %d", ani_setting_type);
		return -EINVAL;
	}

	if (ani_setting_type == QCA_WLAN_ANI_SETTING_AUTO &&
	    ani_level_attr) {
		hdd_err("Not support to set ani level in QCA_WLAN_ANI_SETTING_AUTO");
		return -EINVAL;
	}

	if (ani_setting_type == QCA_WLAN_ANI_SETTING_FIXED) {
		if (!ani_level_attr) {
			hdd_err("invalid ani_level_attr");
			return -EINVAL;
		}
		ani_level = nla_get_s32(ani_level_attr);
	}
	hdd_debug("ani_setting_type %u, ani_level %d",
		  ani_setting_type, ani_level);

	/* ANI (Adaptive noise immunity) */
	if (ani_setting_type == QCA_WLAN_ANI_SETTING_AUTO)
		enable_ani = 1;
	else
		enable_ani = 0;

	errno = wma_cli_set_command(adapter->vdev_id,
				    WMI_PDEV_PARAM_ANI_ENABLE,
				    enable_ani, PDEV_CMD);
	if (errno) {
		hdd_err("Failed to set ani enable, errno %d", errno);
		return errno;
	}

	if (ani_setting_type == QCA_WLAN_ANI_SETTING_FIXED) {
		errno = wma_cli_set_command(adapter->vdev_id,
					    WMI_PDEV_PARAM_ANI_OFDM_LEVEL,
					    ani_level, PDEV_CMD);
		if (errno) {
			hdd_err("Failed to set ani level, errno %d", errno);
			return errno;
		}
	}

	return 0;
}

static int hdd_config_ant_div_period(struct hdd_adapter *adapter,
				     struct nlattr *tb[])
{
	struct nlattr *probe_attr =
		tb[QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_PROBE_PERIOD];
	struct nlattr *stay_attr =
		tb[QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_STAY_PERIOD];
	uint32_t probe_period, stay_period, ant_div_usrcfg;
	int errno;

	/* nothing to do if neither attribute is present */
	if (!probe_attr && !stay_attr)
		return 0;

	/* if one is present, both must be present */
	if (!probe_attr || !stay_attr) {
		hdd_err("Missing attribute for %s",
			probe_attr ? "STAY" : "PROBE");
		return -EINVAL;
	}

	probe_period = nla_get_u32(probe_attr);
	stay_period = nla_get_u32(stay_attr);
	ant_div_usrcfg = ANT_DIV_SET_PERIOD(probe_period, stay_period);
	hdd_debug("ant div set period: %x", ant_div_usrcfg);
	errno = wma_cli_set_command(adapter->vdev_id,
				    WMI_PDEV_PARAM_ANT_DIV_USRCFG,
				    ant_div_usrcfg, PDEV_CMD);
	if (errno)
		hdd_err("Failed to set ant div period, %d", errno);

	return errno;
}

static int hdd_config_ant_div_snr_weight(struct hdd_adapter *adapter,
					 struct nlattr *tb[])
{
	struct nlattr *mgmt_attr =
		tb[QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_MGMT_SNR_WEIGHT];
	struct nlattr *data_attr =
		tb[QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_DATA_SNR_WEIGHT];
	struct nlattr *ack_attr =
		tb[QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_ACK_SNR_WEIGHT];
	uint32_t mgmt_snr, data_snr, ack_snr, ant_div_usrcfg;
	int errno;

	/* nothing to do if none of the attributes are present */
	if (!mgmt_attr && !data_attr && !ack_attr)
		return 0;

	/* if one is present, all must be present */
	if (!mgmt_attr || !data_attr || !ack_attr) {
		hdd_err("Missing attribute");
		return -EINVAL;
	}

	mgmt_snr = nla_get_u32(mgmt_attr);
	data_snr = nla_get_u32(data_attr);
	ack_snr = nla_get_u32(ack_attr);
	ant_div_usrcfg = ANT_DIV_SET_WEIGHT(mgmt_snr, data_snr, ack_snr);
	hdd_debug("ant div set weight: %x", ant_div_usrcfg);
	errno = wma_cli_set_command(adapter->vdev_id,
				    WMI_PDEV_PARAM_ANT_DIV_USRCFG,
				    ant_div_usrcfg, PDEV_CMD);
	if (errno)
		hdd_err("Failed to set ant div weight, %d", errno);

	return errno;
}

static int hdd_config_fine_time_measurement(struct hdd_adapter *adapter,
					    const struct nlattr *attr)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	uint32_t user_capability;
	uint32_t target_capability;
	uint32_t final_capability;
	QDF_STATUS status;

	user_capability = nla_get_u32(attr);
	target_capability = hdd_ctx->fine_time_meas_cap_target;
	final_capability = user_capability & target_capability;

	status = ucfg_mlme_set_fine_time_meas_cap(hdd_ctx->psoc,
						  final_capability);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("Unable to set value, status %d", status);
		return -EINVAL;
	}

	sme_update_fine_time_measurement_capab(hdd_ctx->mac_handle,
					       adapter->vdev_id,
					       final_capability);
	ucfg_wifi_pos_set_ftm_cap(hdd_ctx->psoc, final_capability);

	hdd_debug("user: 0x%x, target: 0x%x, final: 0x%x",
		  user_capability, target_capability, final_capability);

	return 0;
}

static int hdd_config_modulated_dtim(struct hdd_adapter *adapter,
				     const struct nlattr *attr)
{
	struct wlan_objmgr_vdev *vdev;
	uint32_t modulated_dtim;
	QDF_STATUS status;

	modulated_dtim = nla_get_u32(attr);

	vdev = hdd_objmgr_get_vdev_by_user(adapter, WLAN_OSIF_ID);
	if (!vdev)
		return -EINVAL;

	status = ucfg_pmo_config_modulated_dtim(vdev, modulated_dtim);

	hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_ID);

	return qdf_status_to_os_return(status);
}

static int hdd_config_listen_interval(struct hdd_adapter *adapter,
				      const struct nlattr *attr)
{
	struct wlan_objmgr_vdev *vdev;
	uint32_t listen_interval;
	QDF_STATUS status;

	listen_interval = nla_get_u32(attr);
	if (listen_interval > cfg_max(CFG_PMO_ENABLE_DYNAMIC_DTIM)) {
		hdd_err_rl("Invalid value for listen interval - %d",
			   listen_interval);
		return -EINVAL;
	}

	vdev = hdd_objmgr_get_vdev_by_user(adapter, WLAN_PMO_ID);
	if (!vdev)
		return -EINVAL;

	status = ucfg_pmo_config_listen_interval(vdev, listen_interval);

	hdd_objmgr_put_vdev_by_user(vdev, WLAN_PMO_ID);

	return qdf_status_to_os_return(status);
}

static int hdd_config_lro(struct hdd_adapter *adapter,
			  const struct nlattr *attr)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	uint8_t enable_flag;

	enable_flag = nla_get_u8(attr);

	return hdd_lro_set_reset(hdd_ctx, adapter, enable_flag);
}

static int hdd_config_scan_enable(struct hdd_adapter *adapter,
				  const struct nlattr *attr)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	uint8_t enable_flag;

	enable_flag = nla_get_u8(attr);
	if (enable_flag)
		ucfg_scan_psoc_set_enable(hdd_ctx->psoc,
					  REASON_USER_SPACE);
	else
		ucfg_scan_psoc_set_disable(hdd_ctx->psoc,
					   REASON_USER_SPACE);

	return 0;
}

/**
 * hdd_config_udp_qos_upgrade_threshold() - NL attribute handler to parse
 *					    priority upgrade threshold value.
 * @adapter: adapter for which this configuration is to be applied
 * @attr: NL attribute
 *
 * Returns: 0 on success, -EINVAL on failure
 */
static int hdd_config_udp_qos_upgrade_threshold(struct hdd_adapter *adapter,
						const struct nlattr *attr)
{
	uint8_t priority = nla_get_u8(attr);

	return hdd_set_udp_qos_upgrade_config(adapter, priority);
}

static int hdd_config_power(struct hdd_adapter *adapter,
			    const struct nlattr *attr)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	uint8_t power;

	if (!ucfg_pmo_get_default_power_save_mode(hdd_ctx->psoc)) {
		hdd_err_rl("OPM power save is disabled in ini");
		return -EINVAL;
	}

	power = nla_get_u8(attr);

	return hdd_set_power_config(hdd_ctx, adapter, power);
}

static int hdd_config_stats_avg_factor(struct hdd_adapter *adapter,
				       const struct nlattr *attr)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	uint16_t stats_avg_factor;
	QDF_STATUS status;

	stats_avg_factor = nla_get_u16(attr);
	status = sme_configure_stats_avg_factor(hdd_ctx->mac_handle,
						adapter->vdev_id,
						stats_avg_factor);

	return qdf_status_to_os_return(status);
}

static int hdd_config_non_agg_retry(struct hdd_adapter *adapter,
				    const struct nlattr *attr)
{
	uint8_t retry;

	retry = nla_get_u8(attr);

	/* Value less than CFG_AGG_RETRY_MIN has side effect to t-put */
	retry = (retry > CFG_NON_AGG_RETRY_MAX) ? CFG_NON_AGG_RETRY_MAX :
		((retry < CFG_NON_AGG_RETRY_MIN) ? CFG_NON_AGG_RETRY_MIN :
		  retry);
	hdd_debug("sending Non-Agg Retry Th: %d", retry);

	return sme_set_vdev_sw_retry(adapter->vdev_id, retry,
				     WMI_VDEV_CUSTOM_SW_RETRY_TYPE_NONAGGR);
}

static int hdd_config_agg_retry(struct hdd_adapter *adapter,
				const struct nlattr *attr)
{
	uint8_t retry;

	retry = nla_get_u8(attr);

	/* Value less than CFG_AGG_RETRY_MIN has side effect to t-put */
	retry = (retry > CFG_AGG_RETRY_MAX) ? CFG_AGG_RETRY_MAX :
		((retry < CFG_AGG_RETRY_MIN) ? CFG_AGG_RETRY_MIN :
		  retry);
	hdd_debug("sending Agg Retry Th: %d", retry);

	return sme_set_vdev_sw_retry(adapter->vdev_id, retry,
				     WMI_VDEV_CUSTOM_SW_RETRY_TYPE_AGGR);
}

static int hdd_config_mgmt_retry(struct hdd_adapter *adapter,
				 const struct nlattr *attr)
{
	uint8_t retry;
	int param_id;
	uint8_t max_mgmt_retry;

	retry = nla_get_u8(attr);
	max_mgmt_retry = (cfg_max(CFG_MGMT_RETRY_MAX));
	retry = retry > max_mgmt_retry ?
		max_mgmt_retry : retry;
	param_id = WMI_PDEV_PARAM_MGMT_RETRY_LIMIT;

	return wma_cli_set_command(adapter->vdev_id, param_id,
				   retry, PDEV_CMD);
}

static int hdd_config_ctrl_retry(struct hdd_adapter *adapter,
				 const struct nlattr *attr)
{
	uint8_t retry;
	int param_id;

	retry = nla_get_u8(attr);
	retry = retry > CFG_CTRL_RETRY_MAX ?
		CFG_CTRL_RETRY_MAX : retry;
	param_id = WMI_PDEV_PARAM_CTRL_RETRY_LIMIT;

	return wma_cli_set_command(adapter->vdev_id, param_id,
				   retry, PDEV_CMD);
}

static int hdd_config_propagation_delay(struct hdd_adapter *adapter,
					const struct nlattr *attr)
{
	uint8_t delay;
	uint32_t abs_delay;
	int param_id;

	delay = nla_get_u8(attr);
	delay = delay > CFG_PROPAGATION_DELAY_MAX ?
				CFG_PROPAGATION_DELAY_MAX : delay;
	abs_delay = delay + CFG_PROPAGATION_DELAY_BASE;
	param_id = WMI_PDEV_PARAM_PROPAGATION_DELAY;

	return  wma_cli_set_command(adapter->vdev_id, param_id,
				    abs_delay, PDEV_CMD);
}

static int hdd_config_propagation_abs_delay(struct hdd_adapter *adapter,
					    const struct nlattr *attr)
{
	uint32_t abs_delay;
	int param_id;

	abs_delay = nla_get_u32(attr);
	param_id = WMI_PDEV_PARAM_PROPAGATION_DELAY;

	return wma_cli_set_command(adapter->vdev_id, param_id,
				   abs_delay, PDEV_CMD);
}

static int hdd_config_tx_fail_count(struct hdd_adapter *adapter,
				    const struct nlattr *attr)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	uint32_t tx_fail_count;
	QDF_STATUS status;

	tx_fail_count = nla_get_u32(attr);
	if (!tx_fail_count)
		return 0;

	status = sme_update_tx_fail_cnt_threshold(hdd_ctx->mac_handle,
						  adapter->vdev_id,
						  tx_fail_count);
	if (QDF_IS_STATUS_ERROR(status))
		hdd_err("sme_update_tx_fail_cnt_threshold (err=%d)",
			status);

	return qdf_status_to_os_return(status);
}

static int hdd_config_channel_avoidance_ind(struct hdd_adapter *adapter,
					    const struct nlattr *attr)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	uint8_t set_value;

	set_value = nla_get_u8(attr);
	hdd_debug("set_value: %d", set_value);

	return hdd_enable_disable_ca_event(hdd_ctx, set_value);
}

static int hdd_config_guard_time(struct hdd_adapter *adapter,
				 const struct nlattr *attr)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	uint32_t guard_time;
	QDF_STATUS status;

	guard_time = nla_get_u32(attr);
	status = sme_configure_guard_time(hdd_ctx->mac_handle,
					  adapter->vdev_id,
					  guard_time);

	return qdf_status_to_os_return(status);
}

static int hdd_config_scan_default_ies(struct hdd_adapter *adapter,
				       const struct nlattr *attr)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	uint8_t *scan_ie;
	uint16_t scan_ie_len;
	QDF_STATUS status;
	mac_handle_t mac_handle;

	scan_ie_len = nla_len(attr);
	hdd_debug("IE len %d session %d device mode %d",
		  scan_ie_len, adapter->vdev_id, adapter->device_mode);

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
		mac_handle = hdd_ctx->mac_handle;
		status = sme_set_default_scan_ie(mac_handle,
						 adapter->vdev_id, scan_ie,
						 scan_ie_len);
		if (QDF_STATUS_SUCCESS != status) {
			hdd_err("failed to set default scan IEs in sme: %d",
				status);
			return -EPERM;
		}
	}

	return 0;
}

static int hdd_config_ant_div_ena(struct hdd_adapter *adapter,
				  const struct nlattr *attr)
{
	uint32_t antdiv_enable;
	int errno;

	antdiv_enable = nla_get_u32(attr);
	hdd_debug("antdiv_enable: %d", antdiv_enable);
	errno = wma_cli_set_command(adapter->vdev_id,
				    WMI_PDEV_PARAM_ENA_ANT_DIV,
				    antdiv_enable, PDEV_CMD);
	if (errno)
		hdd_err("Failed to set antdiv_enable, %d", errno);

	return errno;
}

static int hdd_config_ant_div_snr_diff(struct hdd_adapter *adapter,
				       const struct nlattr *attr)
{
	uint32_t ant_div_snr_diff;
	uint32_t ant_div_usrcfg;
	int errno;

	ant_div_snr_diff = nla_get_u32(attr);
	hdd_debug("snr diff: %x", ant_div_snr_diff);

	ant_div_usrcfg = ANT_DIV_SET_SNR_DIFF(ant_div_snr_diff);
	hdd_debug("usrcfg: %x", ant_div_usrcfg);

	errno = wma_cli_set_command(adapter->vdev_id,
				    WMI_PDEV_PARAM_ANT_DIV_USRCFG,
				    ant_div_usrcfg, PDEV_CMD);
	if (errno)
		hdd_err("Failed to set snr diff, %d", errno);

	return errno;
}

static int hdd_config_ant_div_probe_dwell_time(struct hdd_adapter *adapter,
					       const struct nlattr *attr)
{
	uint32_t dwell_time;
	uint32_t ant_div_usrcfg;
	int errno;

	dwell_time = nla_get_u32(attr);
	hdd_debug("dwell time: %x", dwell_time);

	ant_div_usrcfg = ANT_DIV_SET_PROBE_DWELL_TIME(dwell_time);
	hdd_debug("usrcfg: %x", ant_div_usrcfg);

	errno = wma_cli_set_command(adapter->vdev_id,
				    WMI_PDEV_PARAM_ANT_DIV_USRCFG,
				    ant_div_usrcfg, PDEV_CMD);
	if (errno)
		hdd_err("Failed to set probe dwell time, %d", errno);

	return errno;
}

static int hdd_config_ant_div_chain(struct hdd_adapter *adapter,
				    const struct nlattr *attr)
{
	uint32_t antdiv_chain;
	int errno;

	antdiv_chain = nla_get_u32(attr);
	hdd_debug("antdiv_chain: %d", antdiv_chain);

	errno = wma_cli_set_command(adapter->vdev_id,
				    WMI_PDEV_PARAM_FORCE_CHAIN_ANT,
				    antdiv_chain, PDEV_CMD);
	if (errno)
		hdd_err("Failed to set chain, %d", errno);

	return errno;
}

static int hdd_config_ant_div_selftest(struct hdd_adapter *adapter,
				       const struct nlattr *attr)
{
	uint32_t antdiv_selftest;
	int errno;

	antdiv_selftest = nla_get_u32(attr);
	hdd_debug("antdiv_selftest: %d", antdiv_selftest);

	errno = wma_cli_set_command(adapter->vdev_id,
				    WMI_PDEV_PARAM_ANT_DIV_SELFTEST,
				    antdiv_selftest, PDEV_CMD);
	if (errno)
		hdd_err("Failed to set selftest, %d", errno);

	return errno;
}

static int hdd_config_ant_div_selftest_intvl(struct hdd_adapter *adapter,
					     const struct nlattr *attr)
{
	uint32_t antdiv_selftest_intvl;
	int errno;

	antdiv_selftest_intvl = nla_get_u32(attr);
	hdd_debug("antdiv_selftest_intvl: %d", antdiv_selftest_intvl);

	errno = wma_cli_set_command(adapter->vdev_id,
				    WMI_PDEV_PARAM_ANT_DIV_SELFTEST_INTVL,
				    antdiv_selftest_intvl, PDEV_CMD);
	if (errno)
		hdd_err("Failed to set selftest interval, %d", errno);

	return errno;
}

static int hdd_config_ignore_assoc_disallowed(struct hdd_adapter *adapter,
					      const struct nlattr *attr)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	uint8_t ignore_assoc_disallowed;

	ignore_assoc_disallowed = nla_get_u8(attr);
	hdd_debug("%u", ignore_assoc_disallowed);
	if ((ignore_assoc_disallowed < QCA_IGNORE_ASSOC_DISALLOWED_DISABLE) ||
	    (ignore_assoc_disallowed > QCA_IGNORE_ASSOC_DISALLOWED_ENABLE))
		return -EINVAL;

	sme_set_check_assoc_disallowed(hdd_ctx->mac_handle,
				       !ignore_assoc_disallowed);

	return 0;
}

static int hdd_config_restrict_offchannel(struct hdd_adapter *adapter,
					  const struct nlattr *attr)
{
	uint8_t restrict_offchan;

	restrict_offchan = nla_get_u8(attr);
	hdd_debug("%u", restrict_offchan);

	if (restrict_offchan > 1) {
		hdd_err("Invalid value %u", restrict_offchan);
		return -EINVAL;
	}

	return wlan_hdd_handle_restrict_offchan_config(adapter,
						       restrict_offchan);
}

static int hdd_config_total_beacon_miss_count(struct hdd_adapter *adapter,
					      const struct nlattr *attr)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	uint8_t first_miss_count;
	uint8_t final_miss_count;
	uint8_t total_miss_count;
	QDF_STATUS status;

	if (adapter->device_mode != QDF_STA_MODE) {
		hdd_err("Only supported in sta mode");
		return -EINVAL;
	}

	total_miss_count = nla_get_u8(attr);
	ucfg_mlme_get_roam_bmiss_first_bcnt(hdd_ctx->psoc,
					    &first_miss_count);
	if (total_miss_count <= first_miss_count) {
		hdd_err("Total %u needs to exceed first %u",
			total_miss_count, first_miss_count);
		return -EINVAL;
	}

	final_miss_count = total_miss_count - first_miss_count;

	if (!ucfg_mlme_validate_roam_bmiss_final_bcnt(final_miss_count))
		return -EINVAL;

	hdd_debug("First count %u, final count %u",
		  first_miss_count, final_miss_count);

	status = sme_set_roam_bmiss_final_bcnt(hdd_ctx->mac_handle,
					       adapter->vdev_id,
					       final_miss_count);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("Failed to set final count, status %u", status);
		return qdf_status_to_os_return(status);
	}

	status = sme_set_bmiss_bcnt(adapter->vdev_id,
				    first_miss_count,
				    final_miss_count);
	if (QDF_IS_STATUS_ERROR(status))
		hdd_err("Failed to set count, status %u", status);

	return qdf_status_to_os_return(status);
}

#ifdef WLAN_FEATURE_LL_MODE
static inline
void wlan_hdd_set_wlm_mode(struct hdd_context *hdd_ctx, uint16_t latency_level)
{
	if (latency_level ==
		QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL_ULTRALOW)
		wlan_hdd_set_pm_qos_request(hdd_ctx, true);
	else
		wlan_hdd_set_pm_qos_request(hdd_ctx, false);
}
#else
static inline
void wlan_hdd_set_wlm_mode(struct hdd_context *hdd_ctx, uint16_t latency_level)
{
}
#endif

/**
 * hdd_set_wlm_host_latency_level() - set latency flags based on latency flags
 * @hdd_ctx: hdd context
 * @adapter: adapter context
 * @latency_host_flags: host latency flags
 *
 * Return: none
 */
static void hdd_set_wlm_host_latency_level(struct hdd_context *hdd_ctx,
					   struct hdd_adapter *adapter,
					   uint32_t latency_host_flags)
{
	ol_txrx_soc_handle soc_hdl = cds_get_context(QDF_MODULE_ID_SOC);

	if (!soc_hdl)
		return;

	if (latency_host_flags & WLM_HOST_PM_QOS_FLAG)
		hdd_ctx->pm_qos_request_flags |= (1 << adapter->vdev_id);
	else
		hdd_ctx->pm_qos_request_flags &= ~(1 << adapter->vdev_id);

	if (hdd_ctx->pm_qos_request_flags)
		wlan_hdd_set_pm_qos_request(hdd_ctx, true);
	else
		wlan_hdd_set_pm_qos_request(hdd_ctx, false);

	if (latency_host_flags & WLM_HOST_HBB_FLAG)
		hdd_ctx->high_bus_bw_request |= (1 << adapter->vdev_id);
	else
		hdd_ctx->high_bus_bw_request &= ~(1 << adapter->vdev_id);

	if (latency_host_flags & WLM_HOST_RX_THREAD_FLAG)
		adapter->runtime_disable_rx_thread = true;
	else
		adapter->runtime_disable_rx_thread = false;
}

#ifdef MULTI_CLIENT_LL_SUPPORT
void
hdd_latency_level_event_handler_cb(const struct latency_level_data *event_data,
				   uint8_t vdev_id)
{
	struct osif_request *request;
	struct latency_level_data *data;
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	struct hdd_adapter *hdd_adapter;
	uint32_t latency_host_flags = 0;
	QDF_STATUS status;

	hdd_enter();

	hdd_adapter = hdd_get_adapter_by_vdev(hdd_ctx, vdev_id);
	if (!hdd_adapter) {
		hdd_err("adapter is NULL vdev_id = %d", vdev_id);
		return;
	}

	if (wlan_hdd_validate_context(hdd_ctx))
		return;

	if (!event_data) {
		hdd_err("Invalid latency level event data");
		return;
	}

	if (hdd_adapter->multi_ll_resp_expected) {
		request =
			osif_request_get(hdd_adapter->multi_ll_response_cookie);
		if (!request) {
			hdd_err("Invalid request");
			return;
		}
		data = osif_request_priv(request);
		data->latency_level = event_data->latency_level;
		data->vdev_id = event_data->vdev_id;
		osif_request_complete(request);
		osif_request_put(request);
	} else {
		hdd_adapter->latency_level = event_data->latency_level;
		wlan_hdd_set_wlm_mode(hdd_ctx, hdd_adapter->latency_level);
		hdd_debug("adapter->latency_level:%d",
			  hdd_adapter->latency_level);
		status = ucfg_mlme_get_latency_host_flags(hdd_ctx->psoc,
						hdd_adapter->latency_level,
						&latency_host_flags);
		if (QDF_IS_STATUS_ERROR(status))
			hdd_err("failed to get latency host flags");
		else
			hdd_set_wlm_host_latency_level(hdd_ctx, hdd_adapter,
						       latency_host_flags);
		}

	hdd_exit();
}

uint8_t wlan_hdd_get_client_id_bitmap(struct hdd_adapter *adapter)
{
	uint8_t i, client_id_bitmap = 0;

	for (i = 0; i < WLM_MAX_HOST_CLIENT; i++) {
		if (!adapter->client_info[i].in_use)
			continue;
		client_id_bitmap |=
			BIT(adapter->client_info[i].client_id);
	}

	return client_id_bitmap;
}

QDF_STATUS wlan_hdd_get_set_client_info_id(struct hdd_adapter *adapter,
					   uint32_t port_id,
					   uint32_t *client_id)
{
	uint8_t i;
	QDF_STATUS status = QDF_STATUS_E_INVAL;

	for (i = 0; i < WLM_MAX_HOST_CLIENT; i++) {
		if (adapter->client_info[i].in_use) {
			/* Receives set latency cmd for an existing port id */
			if (port_id == adapter->client_info[i].port_id) {
				*client_id = adapter->client_info[i].client_id;
				status = QDF_STATUS_SUCCESS;
				break;
			}
			continue;
		} else {
			/* Process set latency level from a new client */
			adapter->client_info[i].in_use = true;
			adapter->client_info[i].port_id = port_id;
			*client_id = adapter->client_info[i].client_id;
			status = QDF_STATUS_SUCCESS;
			break;
		}
	}

	if (i == WLM_MAX_HOST_CLIENT)
		hdd_debug("Max client ID reached");

	return status;
}

QDF_STATUS wlan_hdd_set_wlm_latency_level(struct hdd_adapter *adapter,
					  uint16_t latency_level,
					  uint32_t client_id_bitmap,
					  bool force_reset)
{
	QDF_STATUS status;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	int ret;
	struct osif_request *request = NULL;
	struct latency_level_data *priv;
	static const struct osif_request_params params = {
		.priv_size = sizeof(*priv),
		.timeout_ms = WLAN_WAIT_WLM_LATENCY_LEVEL,
		.dealloc = NULL,
	};

	/* ignore unless in STA mode */
	if (adapter->device_mode != QDF_STA_MODE) {
		hdd_debug_rl("WLM offload is supported in STA mode only");
		return QDF_STATUS_E_FAILURE;
	}

	adapter->multi_ll_resp_expected = true;

	request = osif_request_alloc(&params);
	if (!request) {
		hdd_err("Request allocation failure");
		return QDF_STATUS_E_FAILURE;
	}
	adapter->multi_ll_response_cookie = osif_request_cookie(request);
	adapter->multi_ll_req_in_progress = true;

	status = sme_set_wlm_latency_level(hdd_ctx->mac_handle,
					   adapter->vdev_id, latency_level,
					   client_id_bitmap, force_reset);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("Failure while sending command to fw");
		goto err;
	}

	ret = osif_request_wait_for_response(request);
	if (ret) {
		hdd_err("SME timed out while retrieving latency level");
		status = qdf_status_from_os_return(ret);
		goto err;
	}
	priv = osif_request_priv(request);
	if (!priv) {
		hdd_err("invalid get latency level");
		status = QDF_STATUS_E_FAILURE;
		goto err;
	}

	hdd_debug("latency level received from FW:%d", priv->latency_level);
	adapter->latency_level = priv->latency_level;
err:
	if (request)
		osif_request_put(request);
	adapter->multi_ll_req_in_progress = false;
	adapter->multi_ll_resp_expected = false;
	adapter->multi_ll_response_cookie = NULL;

	return status;
}

bool hdd_get_multi_client_ll_support(struct hdd_adapter *adapter)
{
	return adapter->multi_client_ll_support;
}

/**
 * wlan_hdd_reset_client_info() - reset multi client info table
 * @adapter: adapter context
 * @client_id: client id
 *
 * Return: none
 */
static void wlan_hdd_reset_client_info(struct hdd_adapter *adapter,
				       uint32_t client_id)
{
	adapter->client_info[client_id].in_use = false;
	adapter->client_info[client_id].port_id = 0;
	adapter->client_info[client_id].client_id = client_id;
}

QDF_STATUS wlan_hdd_set_wlm_client_latency_level(struct hdd_adapter *adapter,
						 uint32_t port_id,
						 uint16_t latency_level)
{
	uint32_t client_id, client_id_bitmap;
	QDF_STATUS status;

	status = wlan_hdd_get_set_client_info_id(adapter, port_id,
						 &client_id);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	client_id_bitmap = BIT(client_id);
	status = wlan_hdd_set_wlm_latency_level(adapter,
						latency_level,
						client_id_bitmap,
						false);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_debug("Fail to set latency level for client_id:%d",
			  client_id);
		wlan_hdd_reset_client_info(adapter, client_id);
		return status;
	}
	return status;
}

/**
 * wlan_hdd_get_multi_ll_req_in_progress() - get multi_ll_req_in_progress flag
 * @adapter: adapter context
 *
 * Return: true if multi ll req in progress
 */
static bool wlan_hdd_get_multi_ll_req_in_progress(struct hdd_adapter *adapter)
{
	return adapter->multi_ll_req_in_progress;
}
#else
static inline bool
wlan_hdd_get_multi_ll_req_in_progress(struct hdd_adapter *adapter)
{
	return false;
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 1, 0))
static QDF_STATUS hdd_get_netlink_sender_portid(struct hdd_context *hdd_ctx,
						uint32_t *port_id)
{
	struct wiphy *wiphy = hdd_ctx->wiphy;

	/* get netlink portid of sender */
	*port_id =  cfg80211_vendor_cmd_get_sender(wiphy);

	return QDF_STATUS_SUCCESS;
}
#else
static inline QDF_STATUS
hdd_get_netlink_sender_portid(struct hdd_context *hdd_ctx, uint32_t *port_id)
{
	return QDF_STATUS_E_NOSUPPORT;
}
#endif

static int hdd_config_latency_level(struct hdd_adapter *adapter,
				    const struct nlattr *attr)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	uint32_t port_id;
	uint16_t latency_level, host_latency_level;
	QDF_STATUS status;
	uint32_t latency_host_flags = 0;
	int ret;

	if (hdd_validate_adapter(adapter))
		return -EINVAL;

	if (!hdd_is_wlm_latency_manager_supported(hdd_ctx))
		return -ENOTSUPP;

	latency_level = nla_get_u16(attr);
	switch (latency_level) {
	case QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL_NORMAL:
	case QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL_XR:
	case QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL_LOW:
	case QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL_ULTRALOW:
		/* valid values */
		break;
	default:
		hdd_err("Invalid value %u", latency_level);
		return -EINVAL;
	}

	host_latency_level = latency_level - 1;

	if (hdd_get_multi_client_ll_support(adapter)) {
		if (wlan_hdd_get_multi_ll_req_in_progress(adapter)) {
			hdd_err_rl("multi ll request already in progress");
			return -EBUSY;
		}
		/* get netlink portid of sender */
		status = hdd_get_netlink_sender_portid(hdd_ctx, &port_id);
		if (QDF_IS_STATUS_ERROR(status))
			goto error;
		status = wlan_hdd_set_wlm_client_latency_level(adapter, port_id,
							host_latency_level);
		if (QDF_IS_STATUS_ERROR(status)) {
			hdd_debug("Fail to set latency level");
			goto error;
		}
	} else {
		status = sme_set_wlm_latency_level(hdd_ctx->mac_handle,
						   adapter->vdev_id,
						   host_latency_level, 0,
						   false);
		if (QDF_IS_STATUS_ERROR(status)) {
			hdd_err("set latency level failed, %u", status);
			goto error;
		}
		adapter->latency_level = host_latency_level;
	}

	wlan_hdd_set_wlm_mode(hdd_ctx, adapter->latency_level);
	hdd_debug("adapter->latency_level:%d", adapter->latency_level);

	status = ucfg_mlme_get_latency_host_flags(hdd_ctx->psoc,
						  adapter->latency_level,
						  &latency_host_flags);
	if (QDF_IS_STATUS_ERROR(status))
		hdd_err("failed to get latency host flags");
	else
		hdd_set_wlm_host_latency_level(hdd_ctx, adapter,
					       latency_host_flags);
error:
	ret = qdf_status_to_os_return(status);

	return ret;
}

static int hdd_config_disable_fils(struct hdd_adapter *adapter,
				   const struct nlattr *attr)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	uint8_t disable_fils;
	bool enabled;
	QDF_STATUS status;

	/* ignore unless in STA mode */
	if (adapter->device_mode != QDF_STA_MODE)
		return 0;

	disable_fils = nla_get_u8(attr);
	hdd_debug("%u", disable_fils);

	enabled = !disable_fils;
	status = ucfg_mlme_set_fils_enabled_info(hdd_ctx->psoc, enabled);
	if (QDF_IS_STATUS_ERROR(status))
		hdd_err("could not set fils enabled info, %d", status);

	status = ucfg_mlme_set_enable_bcast_probe_rsp(hdd_ctx->psoc, enabled);
	if (QDF_IS_STATUS_ERROR(status))
		hdd_err("could not set enable bcast probe resp info, %d",
			status);

	status = wma_cli_set_command(adapter->vdev_id,
				     WMI_VDEV_PARAM_ENABLE_BCAST_PROBE_RESPONSE,
				     !disable_fils, VDEV_CMD);
	if (QDF_IS_STATUS_ERROR(status))
		hdd_err("failed to set enable bcast probe resp, %d",
			status);

	return qdf_status_to_os_return(status);
}

static int hdd_set_primary_interface(struct hdd_adapter *adapter,
				     const struct nlattr *attr)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	bool is_set_primary_iface;
	QDF_STATUS status;
	uint8_t primary_vdev_id, dual_sta_policy;
	int set_value;
	uint32_t count;
	bool enable_mcc_adaptive_sch = false;

	/* ignore unless in STA mode */
	if (adapter->device_mode != QDF_STA_MODE)
		return 0;

	is_set_primary_iface = nla_get_u8(attr);

	primary_vdev_id =
		is_set_primary_iface ? adapter->vdev_id : WLAN_UMAC_VDEV_ID_MAX;

	status = ucfg_mlme_set_primary_interface(hdd_ctx->psoc,
						 primary_vdev_id);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("could not set primary interface, %d", status);
		return -EINVAL;
	}

	/* After SSR, the dual sta configuration is lost. As SSR is hidden from
	 * userland, this command will not come from userspace after a SSR. To
	 * restore this configuration, save this in hdd context and restore
	 * after re-init.
	 */
	hdd_ctx->dual_sta_policy.primary_vdev_id = primary_vdev_id;

	count = policy_mgr_mode_specific_connection_count(hdd_ctx->psoc,
							  PM_STA_MODE, NULL);

	if (count < 2) {
		hdd_debug("STA + STA concurrency not present, count:%d", count);
		return 0;
	}

	/* if dual sta roaming enabled and both sta in DBS then no need
	 * to enable roaming on primary as both STA's have roaming enabled.
	 * if dual sta roaming enabled and both sta in MCC or SCC then need
	 * to enable roaming on primary vdev.
	 * if dual sta roaming NOT enabled then need to enable roaming on
	 * primary vdev for dual STA concurrency in MCC or DBS.
	 */
	if (primary_vdev_id !=  WLAN_UMAC_VDEV_ID_MAX)
		if ((ucfg_mlme_get_dual_sta_roaming_enabled(hdd_ctx->psoc) &&
		     !policy_mgr_concurrent_sta_doing_dbs(hdd_ctx->psoc)) ||
		    !ucfg_mlme_get_dual_sta_roaming_enabled(hdd_ctx->psoc)) {
			hdd_err("Enable roaming on requested interface: %d",
				adapter->vdev_id);
			hdd_debug("Enable roaming on requested interface: %d",
				  adapter->vdev_id);
			wlan_cm_roam_state_change(hdd_ctx->pdev,
						  adapter->vdev_id,
						  WLAN_ROAM_RSO_ENABLED,
						  REASON_ROAM_SET_PRIMARY);
	}

	/*
	 * send duty cycle percentage to FW only if STA + STA
	 * concurrency is in MCC.
	 */
	if (!policy_mgr_current_concurrency_is_mcc(hdd_ctx->psoc)) {
		hdd_debug("STA + STA concurrency not in MCC");
		return 0;
	}

	status = ucfg_mlme_get_dual_sta_policy(hdd_ctx->psoc, &dual_sta_policy);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("could not get dual sta policy, %d", status);
		return -EINVAL;
	}

	hdd_debug("is_set_primary_iface: %d, primary vdev id: %d, dual_sta_policy:%d",
		  is_set_primary_iface, primary_vdev_id, dual_sta_policy);

	if (is_set_primary_iface && dual_sta_policy ==
	    QCA_WLAN_CONCURRENT_STA_POLICY_PREFER_PRIMARY) {
		hdd_debug("Disable mcc_adaptive_scheduler");
		ucfg_policy_mgr_get_mcc_adaptive_sch(hdd_ctx->psoc,
						     &enable_mcc_adaptive_sch);
		if (enable_mcc_adaptive_sch) {
			ucfg_policy_mgr_set_dynamic_mcc_adaptive_sch(
							hdd_ctx->psoc, false);
			if (QDF_IS_STATUS_ERROR(sme_set_mas(false))) {
				hdd_err("Fail to disable mcc adaptive sched.");
					return -EINVAL;
			}
		}
		/* Configure mcc duty cycle percentage */
		set_value =
		   ucfg_mlme_get_mcc_duty_cycle_percentage(hdd_ctx->pdev);
		if (set_value < 0) {
			hdd_err("Invalid mcc duty cycle");
			return -EINVAL;
		}
		wlan_hdd_send_mcc_vdev_quota(adapter, set_value);
	} else {
		hdd_debug("Enable mcc_adaptive_scheduler");
		ucfg_policy_mgr_get_mcc_adaptive_sch(hdd_ctx->psoc,
						     &enable_mcc_adaptive_sch);
		if (enable_mcc_adaptive_sch) {
			ucfg_policy_mgr_set_dynamic_mcc_adaptive_sch(
							hdd_ctx->psoc, true);
			if (QDF_STATUS_SUCCESS != sme_set_mas(true)) {
				hdd_err("Fail to enable mcc_adaptive_sched.");
				return -EAGAIN;
			}
		}
	}

	return 0;
}

static int hdd_config_rsn_ie(struct hdd_adapter *adapter,
			     const struct nlattr *attr)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	uint8_t force_rsne_override;

	force_rsne_override = nla_get_u8(attr);
	if (force_rsne_override > 1) {
		hdd_err("Invalid value %d", force_rsne_override);
		return -EINVAL;
	}

	hdd_ctx->force_rsne_override = force_rsne_override;
	hdd_debug("force_rsne_override - %d", force_rsne_override);

	return 0;
}

static int hdd_config_gtx(struct hdd_adapter *adapter,
			  const struct nlattr *attr)
{
	uint8_t config_gtx;
	int errno;

	config_gtx = nla_get_u8(attr);
	if (config_gtx > 1) {
		hdd_err_rl("Invalid config_gtx value %d", config_gtx);
		return -EINVAL;
	}

	errno = sme_cli_set_command(adapter->vdev_id,
				    WMI_VDEV_PARAM_GTX_ENABLE,
				    config_gtx, VDEV_CMD);
	if (errno)
		hdd_err("Failed to set GTX, %d", errno);

	return errno;
}

/**
 * hdd_config_disconnect_ies() - Configure disconnect IEs
 * @adapter: Pointer to HDD adapter
 * @attr: array of pointer to struct nlattr
 *
 * Return: 0 on success; error number otherwise
 */
static int hdd_config_disconnect_ies(struct hdd_adapter *adapter,
				     const struct nlattr *attr)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	QDF_STATUS status;

	hdd_debug("IE len %u session %u device mode %u",
		  nla_len(attr), adapter->vdev_id, adapter->device_mode);
	if (!nla_len(attr) ||
	    nla_len(attr) > SIR_MAC_MAX_ADD_IE_LENGTH + 2 ||
	    !wlan_is_ie_valid(nla_data(attr), nla_len(attr))) {
		hdd_err("Invalid disconnect IEs");
		return -EINVAL;
	}

	status = sme_set_disconnect_ies(hdd_ctx->mac_handle,
					adapter->vdev_id,
					nla_data(attr),
					nla_len(attr));
	if (QDF_IS_STATUS_ERROR(status))
		hdd_err("Failed to set disconnect_ies");

	return qdf_status_to_os_return(status);
}

#ifdef WLAN_FEATURE_ELNA
/**
 * hdd_set_elna_bypass() - Set eLNA bypass
 * @adapter: Pointer to HDD adapter
 * @attr: Pointer to struct nlattr
 *
 * Return: 0 on success; error number otherwise
 */
static int hdd_set_elna_bypass(struct hdd_adapter *adapter,
			       const struct nlattr *attr)
{
	int ret;
	struct wlan_objmgr_vdev *vdev;

	vdev = hdd_objmgr_get_vdev_by_user(adapter, WLAN_FWOL_NB_ID);
	if (!vdev)
		return -EINVAL;

	ret = os_if_fwol_set_elna_bypass(vdev, attr);

	hdd_objmgr_put_vdev_by_user(vdev, WLAN_FWOL_NB_ID);

	return ret;
}
#endif

/**
 * hdd_mac_chwidth_to_bonding_mode() - get bonding_mode from chan width
 * @chwidth: chan width
 *
 * Return: bonding mode
 */
static uint32_t hdd_mac_chwidth_to_bonding_mode(
			enum eSirMacHTChannelWidth chwidth)
{
	uint32_t bonding_mode;

	switch (chwidth) {
	case eHT_CHANNEL_WIDTH_20MHZ:
		bonding_mode = WNI_CFG_CHANNEL_BONDING_MODE_DISABLE;
		break;
	default:
		bonding_mode = WNI_CFG_CHANNEL_BONDING_MODE_ENABLE;
	}

	return bonding_mode;
}

int hdd_set_mac_chan_width(struct hdd_adapter *adapter,
			   enum eSirMacHTChannelWidth chwidth)
{
	uint32_t bonding_mode;

	bonding_mode = hdd_mac_chwidth_to_bonding_mode(chwidth);

	return hdd_update_channel_width(adapter, chwidth, bonding_mode);
}

/**
 * hdd_set_channel_width() - set channel width
 *
 * @adapter: hdd adapter
 * @attr: nla attr sent by supplicant
 *
 * Return: 0 on success, negative errno on failure
 */
static int hdd_set_channel_width(struct hdd_adapter *adapter,
				 const struct nlattr *attr)
{
	uint8_t nl80211_chwidth;
	enum eSirMacHTChannelWidth chwidth;

	nl80211_chwidth = nla_get_u8(attr);
	chwidth = hdd_nl80211_chwidth_to_chwidth(nl80211_chwidth);
	if (chwidth < 0) {
		hdd_err("Invalid channel width");
		return -EINVAL;
	}

	return hdd_set_mac_chan_width(adapter, chwidth);
}

/**
 * hdd_set_dynamic_bw() - enable / disable dynamic bandwidth
 *
 * @adapter: hdd adapter
 * @attr: nla attr sent by supplicant
 *
 * Return: 0 on success, negative errno on failure
 */
static int hdd_set_dynamic_bw(struct hdd_adapter *adapter,
			      const struct nlattr *attr)
{
	uint8_t enable;

	enable = nla_get_u8(attr);

	return wma_cli_set_command(adapter->vdev_id, WMI_PDEV_PARAM_DYNAMIC_BW,
				   enable, PDEV_CMD);
}

/**
 * hdd_set_nss() - set the number of spatial streams supported by the adapter
 *
 * @adapter: hdd adapter
 * @attr: pointer to nla attr
 *
 * Return: 0 on success, negative errno on failure
 */
static int hdd_set_nss(struct hdd_adapter *adapter,
		       const struct nlattr *attr)
{
	uint8_t nss;
	int ret;
	QDF_STATUS status;

	nss = nla_get_u8(attr);

	status = hdd_update_nss(adapter, nss, nss);
	ret = qdf_status_to_os_return(status);

	if (ret == 0 && adapter->device_mode == QDF_SAP_MODE)
		ret = wma_cli_set_command(adapter->vdev_id, WMI_VDEV_PARAM_NSS,
					  nss, VDEV_CMD);

	return ret;
}

#ifdef FEATURE_WLAN_DYNAMIC_ARP_NS_OFFLOAD
#define DYNAMIC_ARP_NS_ENABLE    1
#define DYNAMIC_ARP_NS_DISABLE   0

/**
 * hdd_set_arp_ns_offload() - enable/disable arp/ns offload feature
 * @adapter: hdd adapter
 * @attr: pointer to nla attr
 *
 * Return: 0 on success, negative errno on failure
 */
static int hdd_set_arp_ns_offload(struct hdd_adapter *adapter,
				  const struct nlattr *attr)
{
	uint8_t offload_state;
	int errno;
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct wlan_objmgr_vdev *vdev;

	errno = wlan_hdd_validate_context(hdd_ctx);
	if (errno)
		return errno;

	if (!ucfg_pmo_is_arp_offload_enabled(hdd_ctx->psoc) ||
	    !ucfg_pmo_is_ns_offloaded(hdd_ctx->psoc)) {
		hdd_err_rl("ARP/NS Offload is disabled by ini");
		return -EINVAL;
	}

	if (!ucfg_pmo_is_active_mode_offloaded(hdd_ctx->psoc)) {
		hdd_err_rl("active mode offload is disabled by ini");
		return -EINVAL;
	}

	if (adapter->device_mode != QDF_STA_MODE &&
	    adapter->device_mode != QDF_P2P_CLIENT_MODE) {
		hdd_err_rl("only support on sta/p2p-cli mode");
		return -EINVAL;
	}

	vdev = hdd_objmgr_get_vdev_by_user(adapter, WLAN_OSIF_ID);
	if (!vdev) {
		hdd_err("vdev is NULL");
		return -EINVAL;
	}

	offload_state = nla_get_u8(attr);

	if (offload_state == DYNAMIC_ARP_NS_ENABLE)
		qdf_status = ucfg_pmo_dynamic_arp_ns_offload_enable(vdev);
	else if (offload_state == DYNAMIC_ARP_NS_DISABLE)
		qdf_status = ucfg_pmo_dynamic_arp_ns_offload_disable(vdev);

	if (QDF_IS_STATUS_SUCCESS(qdf_status)) {
		if (offload_state == DYNAMIC_ARP_NS_ENABLE)
			ucfg_pmo_dynamic_arp_ns_offload_runtime_allow(vdev);
		else
			ucfg_pmo_dynamic_arp_ns_offload_runtime_prevent(vdev);
	}

	hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_ID);

	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		if (qdf_status == QDF_STATUS_E_ALREADY) {
			hdd_info_rl("already set arp/ns offload %d",
				    offload_state);
			return 0;
		}
		return qdf_status_to_os_return(qdf_status);
	}

	if (!hdd_is_vdev_in_conn_state(adapter)) {
		hdd_info("set not in connect state, updated state %d",
			 offload_state);
		return 0;
	}

	if (offload_state == DYNAMIC_ARP_NS_ENABLE) {
		hdd_enable_arp_offload(adapter,
				       pmo_arp_ns_offload_dynamic_update);
		hdd_enable_ns_offload(adapter,
				      pmo_arp_ns_offload_dynamic_update);
	} else if (offload_state == DYNAMIC_ARP_NS_DISABLE) {
		hdd_disable_arp_offload(adapter,
					pmo_arp_ns_offload_dynamic_update);
		hdd_disable_ns_offload(adapter,
				       pmo_arp_ns_offload_dynamic_update);
	}

	return 0;
}

#undef DYNAMIC_ARP_NS_ENABLE
#undef DYNAMIC_ARP_NS_DISABLE
#endif

/**
 * hdd_set_wfc_state() - Set wfc state
 * @adapter: hdd adapter
 * @attr: pointer to nla attr
 *
 * Return: 0 on success, negative on failure
 */
static int hdd_set_wfc_state(struct hdd_adapter *adapter,
			     const struct nlattr *attr)
{
	uint8_t cfg_val;
	enum pld_wfc_mode set_val;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	int errno;

	errno = wlan_hdd_validate_context(hdd_ctx);
	if (errno)
		return errno;

	cfg_val = nla_get_u8(attr);

	hdd_debug_rl("set wfc state %d", cfg_val);
	if (cfg_val == 0)
		set_val = PLD_WFC_MODE_OFF;
	else if (cfg_val == 1)
		set_val = PLD_WFC_MODE_ON;
	else
		return -EINVAL;

	return pld_set_wfc_mode(hdd_ctx->parent_dev, set_val);

}

/**
 * typedef independent_setter_fn - independent attribute handler
 * @adapter: The adapter being configured
 * @attr: The nl80211 attribute being applied
 *
 * Defines the signature of functions in the independent attribute vtable
 *
 * Return: 0 if the attribute was handled successfully, otherwise an errno
 */
typedef int (*independent_setter_fn)(struct hdd_adapter *adapter,
				     const struct nlattr *attr);

/**
 * struct independent_setters
 * @id: vendor attribute which this entry handles
 * @cb: callback function to invoke to process the attribute when present
 */
struct independent_setters {
	uint32_t id;
	independent_setter_fn cb;
};

/* vtable for independent setters */
static const struct independent_setters independent_setters[] = {
	{QCA_WLAN_VENDOR_ATTR_CONFIG_SCAN_DEFAULT_IES,
	 hdd_config_scan_default_ies},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_FINE_TIME_MEASUREMENT,
	 hdd_config_fine_time_measurement},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_MODULATED_DTIM,
	 hdd_config_modulated_dtim},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_LISTEN_INTERVAL,
	 hdd_config_listen_interval},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_LRO,
	 hdd_config_lro},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_SCAN_ENABLE,
	 hdd_config_scan_enable},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_QPOWER,
	 hdd_config_power},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_STATS_AVG_FACTOR,
	 hdd_config_stats_avg_factor},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_GUARD_TIME,
	 hdd_config_guard_time},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_NON_AGG_RETRY,
	 hdd_config_non_agg_retry},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_AGG_RETRY,
	 hdd_config_agg_retry},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_MGMT_RETRY,
	 hdd_config_mgmt_retry},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_CTRL_RETRY,
	 hdd_config_ctrl_retry},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_PROPAGATION_DELAY,
	 hdd_config_propagation_delay},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_PROPAGATION_ABS_DELAY,
	 hdd_config_propagation_abs_delay},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_TX_FAIL_COUNT,
	 hdd_config_tx_fail_count},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_CHANNEL_AVOIDANCE_IND,
	 hdd_config_channel_avoidance_ind},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_ENA,
	 hdd_config_ant_div_ena},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_SNR_DIFF,
	 hdd_config_ant_div_snr_diff},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_PROBE_DWELL_TIME,
	 hdd_config_ant_div_probe_dwell_time},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_CHAIN,
	 hdd_config_ant_div_chain},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_SELFTEST,
	 hdd_config_ant_div_selftest},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_SELFTEST_INTVL,
	 hdd_config_ant_div_selftest_intvl},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_IGNORE_ASSOC_DISALLOWED,
	 hdd_config_ignore_assoc_disallowed},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_RESTRICT_OFFCHANNEL,
	 hdd_config_restrict_offchannel},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_TOTAL_BEACON_MISS_COUNT,
	 hdd_config_total_beacon_miss_count},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL,
	 hdd_config_latency_level},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_DISABLE_FILS,
	 hdd_config_disable_fils},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_RSN_IE,
	 hdd_config_rsn_ie},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_GTX,
	 hdd_config_gtx},
	{QCA_WLAN_VENDOR_ATTR_DISCONNECT_IES,
	 hdd_config_disconnect_ies},
#ifdef WLAN_FEATURE_ELNA
	{QCA_WLAN_VENDOR_ATTR_CONFIG_ELNA_BYPASS,
	 hdd_set_elna_bypass},
#endif
	{QCA_WLAN_VENDOR_ATTR_CONFIG_ROAM_REASON,
	 hdd_set_roam_reason_vsie_status},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_LDPC,
	 hdd_config_ldpc},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_TX_STBC,
	 hdd_config_tx_stbc},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_RX_STBC,
	 hdd_config_rx_stbc},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_PHY_MODE,
	 hdd_config_phy_mode},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_CHANNEL_WIDTH,
	 hdd_set_channel_width},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_DYNAMIC_BW,
	 hdd_set_dynamic_bw},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_NSS,
	 hdd_set_nss},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_OPTIMIZED_POWER_MANAGEMENT,
	 hdd_config_power},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_UDP_QOS_UPGRADE,
	 hdd_config_udp_qos_upgrade_threshold},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_CONCURRENT_STA_PRIMARY,
	 hdd_set_primary_interface},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_FT_OVER_DS,
	 hdd_set_ft_over_ds},
#ifdef FEATURE_WLAN_DYNAMIC_ARP_NS_OFFLOAD
	{QCA_WLAN_VENDOR_ATTR_CONFIG_ARP_NS_OFFLOAD,
	 hdd_set_arp_ns_offload},
#endif
	{QCA_WLAN_VENDOR_ATTR_CONFIG_WFC_STATE,
	 hdd_set_wfc_state},
};

#ifdef WLAN_FEATURE_ELNA
/**
 * hdd_get_elna_bypass() - Get eLNA bypass
 * @adapter: Pointer to HDD adapter
 * @skb: sk buffer to hold nl80211 attributes
 * @attr: Pointer to struct nlattr
 *
 * Return: 0 on success; error number otherwise
 */
static int hdd_get_elna_bypass(struct hdd_adapter *adapter,
			       struct sk_buff *skb,
			       const struct nlattr *attr)
{
	int ret;
	struct wlan_objmgr_vdev *vdev;

	vdev = hdd_objmgr_get_vdev_by_user(adapter, WLAN_FWOL_NB_ID);
	if (!vdev)
		return -EINVAL;

	ret = os_if_fwol_get_elna_bypass(vdev, skb, attr);

	hdd_objmgr_put_vdev_by_user(vdev, WLAN_FWOL_NB_ID);

	return ret;
}
#endif

/**
 * hdd_get_roam_reason_vsie_status() - Get roam_reason_vsie
 * @adapter: Pointer to HDD adapter
 * @skb: sk buffer to hold nl80211 attributes
 * @attr: Pointer to struct nlattr
 *
 * Return: 0 on success; error number otherwise
 */
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
static int hdd_get_roam_reason_vsie_status(struct hdd_adapter *adapter,
					   struct sk_buff *skb,
					   const struct nlattr *attr)
{
	uint8_t roam_reason_vsie_enabled;
	struct hdd_context *hdd_ctx = NULL;
	QDF_STATUS status;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	status = ucfg_mlme_get_roam_reason_vsie_status
			       (hdd_ctx->psoc,
				&roam_reason_vsie_enabled);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("get roam reason vsie failed");
		return -EINVAL;
	}
	hdd_debug("is roam_reason_vsie_enabled %d", roam_reason_vsie_enabled);
	if (nla_put_u8(skb, QCA_WLAN_VENDOR_ATTR_CONFIG_ROAM_REASON,
		       roam_reason_vsie_enabled)) {
		hdd_err("nla_put failure");
		return -EINVAL;
	}

	return 0;
}
#else
static int hdd_get_roam_reason_vsie_status(struct hdd_adapter *adapter,
					   struct sk_buff *skb,
					   const struct nlattr *attr)
{
	return -EINVAL;
}
#endif

static int hdd_vendor_attr_ldpc_get(struct hdd_adapter *adapter,
				    struct sk_buff *skb,
				    const struct nlattr *attr)
{
	int ldpc;
	int ret;

	ret = hdd_get_ldpc(adapter, &ldpc);
	if (ret) {
		hdd_err("get ldpc failed");
		return -EINVAL;
	}

	hdd_debug("ldpc %u", ldpc);
	if (nla_put_u8(skb, QCA_WLAN_VENDOR_ATTR_CONFIG_LDPC,
		       (uint8_t)ldpc)) {
		hdd_err("nla_put failure");
		return -EINVAL;
	}

	return 0;
}

static int hdd_vendor_attr_tx_stbc_get(struct hdd_adapter *adapter,
				       struct sk_buff *skb,
				       const struct nlattr *attr)
{
	int tx_stbc;
	int ret;

	ret = hdd_get_tx_stbc(adapter, &tx_stbc);
	if (ret) {
		hdd_err("get tx_stbc failed");
		return -EINVAL;
	}

	hdd_debug("tx_stbc %u", tx_stbc);
	if (nla_put_u8(skb, QCA_WLAN_VENDOR_ATTR_CONFIG_TX_STBC,
		       (uint8_t)tx_stbc)) {
		hdd_err("nla_put failure");
		return -EINVAL;
	}

	return 0;
}

static int hdd_vendor_attr_rx_stbc_get(struct hdd_adapter *adapter,
				       struct sk_buff *skb,
				       const struct nlattr *attr)
{
	int rx_stbc;
	int ret;

	ret = hdd_get_rx_stbc(adapter, &rx_stbc);
	if (ret) {
		hdd_err("get rx_stbc failed");
		return -EINVAL;
	}

	hdd_debug("rx_stbc %u", rx_stbc);
	if (nla_put_u8(skb, QCA_WLAN_VENDOR_ATTR_CONFIG_RX_STBC,
		       (uint8_t)rx_stbc)) {
		hdd_err("nla_put failure");
		return -EINVAL;
	}

	return 0;
}

/**
 * hdd_get_tx_ampdu() - Get TX AMPDU
 * @adapter: Pointer to HDD adapter
 * @skb: sk buffer to hold nl80211 attributes
 * @attr: Pointer to struct nlattr
 *
 * Return: 0 on success; error number otherwise
 */
static int hdd_get_tx_ampdu(struct hdd_adapter *adapter,
			    struct sk_buff *skb,
			    const struct nlattr *attr)
{
	int value;

	value = wma_cli_get_command(adapter->vdev_id, GEN_VDEV_PARAM_TX_AMPDU,
				    GEN_CMD);
	if (value < 0) {
		hdd_err("Failed to get tx_ampdu");
		return -EINVAL;
	}

	if (nla_put_u8(skb, QCA_WLAN_VENDOR_ATTR_CONFIG_TX_MPDU_AGGREGATION,
		       (uint8_t)value)) {
		hdd_err("nla_put failure");
		return -EINVAL;
	}

	return 0;
}

/**
 * hdd_get_rx_ampdu() - Get RX AMPDU
 * @adapter: Pointer to HDD adapter
 * @skb: sk buffer to hold nl80211 attributes
 * @attr: Pointer to struct nlattr
 *
 * Return: 0 on success; error number otherwise
 */
static int hdd_get_rx_ampdu(struct hdd_adapter *adapter,
			    struct sk_buff *skb,
			    const struct nlattr *attr)
{
	int value;

	value = wma_cli_get_command(adapter->vdev_id, GEN_VDEV_PARAM_RX_AMPDU,
				    GEN_CMD);
	if (value < 0) {
		hdd_err("Failed to get rx_ampdu");
		return -EINVAL;
	}

	if (nla_put_u8(skb, QCA_WLAN_VENDOR_ATTR_CONFIG_RX_MPDU_AGGREGATION,
		       (uint8_t)value)) {
		hdd_err("nla_put failure");
		return -EINVAL;
	}

	return 0;
}

/**
 * hdd_get_tx_amsdu() - Get TX AMSDU
 * @adapter: Pointer to HDD adapter
 * @skb: sk buffer to hold nl80211 attributes
 * @attr: Pointer to struct nlattr
 *
 * Return: 0 on success; error number otherwise
 */
static int hdd_get_tx_amsdu(struct hdd_adapter *adapter,
			    struct sk_buff *skb,
			    const struct nlattr *attr)
{
	int value;

	value = wma_cli_get_command(adapter->vdev_id, GEN_VDEV_PARAM_TX_AMSDU,
				    GEN_CMD);
	if (value < 0) {
		hdd_err("Failed to get tx_amsdu");
		return -EINVAL;
	}

	if (nla_put_u8(skb, QCA_WLAN_VENDOR_ATTR_CONFIG_TX_MSDU_AGGREGATION,
		       (uint8_t)value)) {
		hdd_err("nla_put failure");
		return -EINVAL;
	}

	return 0;
}

/**
 * hdd_get_rx_amsdu() - Get RX AMSDU
 * @adapter: Pointer to HDD adapter
 * @skb: sk buffer to hold nl80211 attributes
 * @attr: Pointer to struct nlattr
 *
 * Return: 0 on success; error number otherwise
 */
static int hdd_get_rx_amsdu(struct hdd_adapter *adapter,
			    struct sk_buff *skb,
			    const struct nlattr *attr)
{
	int value;

	value = wma_cli_get_command(adapter->vdev_id, GEN_VDEV_PARAM_RX_AMSDU,
				    GEN_CMD);
	if (value < 0) {
		hdd_err("Failed to get rx_amsdu");
		return -EINVAL;
	}

	if (nla_put_u8(skb, QCA_WLAN_VENDOR_ATTR_CONFIG_RX_MSDU_AGGREGATION,
		       (uint8_t)value)) {
		hdd_err("nla_put failure");
		return -EINVAL;
	}

	return 0;
}

/**
 * hdd_get_channel_width() - Get channel width
 * @adapter: Pointer to HDD adapter
 * @skb: sk buffer to hold nl80211 attributes
 * @attr: Pointer to struct nlattr
 *
 * Return: 0 on success; error number otherwise
 */
static int hdd_get_channel_width(struct hdd_adapter *adapter,
				 struct sk_buff *skb,
				 const struct nlattr *attr)
{
	enum eSirMacHTChannelWidth chwidth;
	uint8_t nl80211_chwidth;

	chwidth = wma_cli_get_command(adapter->vdev_id, WMI_VDEV_PARAM_CHWIDTH,
				      VDEV_CMD);
	if (chwidth < 0) {
		hdd_err("Failed to get chwidth");
		return -EINVAL;
	}

	nl80211_chwidth = hdd_chwidth_to_nl80211_chwidth(chwidth);
	if (nla_put_u8(skb, QCA_WLAN_VENDOR_ATTR_CONFIG_CHANNEL_WIDTH,
		       nl80211_chwidth)) {
		hdd_err("nla_put failure");
		return -EINVAL;
	}

	return 0;
}

/**
 * hdd_get_dynamic_bw() - Get dynamic bandwidth disabled / enabled
 * @adapter: Pointer to HDD adapter
 * @skb: sk buffer to hold nl80211 attributes
 * @attr: Pointer to struct nlattr
 *
 * Return: 0 on success; error number otherwise
 */
static int hdd_get_dynamic_bw(struct hdd_adapter *adapter,
			      struct sk_buff *skb,
			      const struct nlattr *attr)
{
	int enable;

	enable = wma_cli_get_command(adapter->vdev_id,
				     WMI_PDEV_PARAM_DYNAMIC_BW, PDEV_CMD);
	if (enable < 0) {
		hdd_err("Failed to get dynamic_bw");
		return -EINVAL;
	}

	if (nla_put_u8(skb, QCA_WLAN_VENDOR_ATTR_CONFIG_DYNAMIC_BW,
		       (uint8_t)enable)) {
		hdd_err("nla_put failure");
		return -EINVAL;
	}

	return 0;
}

/**
 * hdd_get_nss_config() - Get the number of spatial streams supported by
 * the adapter
 * @adapter: Pointer to HDD adapter
 * @skb: sk buffer to hold nl80211 attributes
 * @attr: Pointer to struct nlattr
 *
 * Return: 0 on success; error number otherwise
 */
static int hdd_get_nss_config(struct hdd_adapter *adapter,
			      struct sk_buff *skb,
			      const struct nlattr *attr)
{
	uint8_t nss;

	if (adapter->device_mode == QDF_SAP_MODE) {
		int value;

		value = wma_cli_get_command(adapter->vdev_id,
					    WMI_VDEV_PARAM_NSS, VDEV_CMD);
		if (value < 0) {
			hdd_err("Failed to get nss");
			return -EINVAL;
		}
		nss = (uint8_t)value;
	} else {
		QDF_STATUS status;

		status = hdd_get_nss(adapter, &nss);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			hdd_err("Failed to get nss");
			return -EINVAL;
		}
	}

	hdd_debug("nss %d", nss);

	if (nla_put_u8(skb, QCA_WLAN_VENDOR_ATTR_CONFIG_NSS, nss)) {
		hdd_err("nla_put failure");
		return -EINVAL;
	}

	return 0;
}

/**
 * hdd_get_tx_nss_config() - Get the number of tx spatial streams supported by
 * the adapter
 * @adapter: Pointer to HDD adapter
 * @skb: sk buffer to hold nl80211 attributes
 * @attr: Pointer to struct nlattr
 *
 * Return: 0 on success; error number otherwise
 */
static int hdd_get_tx_nss_config(struct hdd_adapter *adapter,
				 struct sk_buff *skb,
				 const struct nlattr *attr)
{
	uint8_t tx_nss;
	QDF_STATUS status;

	if (!hdd_is_vdev_in_conn_state(adapter)) {
		hdd_err("Not in connected state");
		return -EINVAL;
	}

	status = hdd_get_tx_nss(adapter, &tx_nss);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("Failed to get nss");
		return -EINVAL;
	}

	hdd_debug("tx_nss %d", tx_nss);
	if (nla_put_u8(skb, QCA_WLAN_VENDOR_ATTR_CONFIG_TX_NSS, tx_nss)) {
		hdd_err("nla_put failure");
		return -EINVAL;
	}

	return 0;
}

/**
 * hdd_get_rx_nss_config() - Get the number of rx spatial streams supported by
 * the adapter
 * @adapter: Pointer to HDD adapter
 * @skb: sk buffer to hold nl80211 attributes
 * @attr: Pointer to struct nlattr
 *
 * Return: 0 on success; error number otherwise
 */
static int hdd_get_rx_nss_config(struct hdd_adapter *adapter,
				 struct sk_buff *skb,
				 const struct nlattr *attr)
{
	uint8_t rx_nss;
	QDF_STATUS status;

	if (!hdd_is_vdev_in_conn_state(adapter)) {
		hdd_err("Not in connected state");
		return -EINVAL;
	}

	status = hdd_get_rx_nss(adapter, &rx_nss);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("Failed to get nss");
		return -EINVAL;
	}

	hdd_debug("rx_nss %d", rx_nss);
	if (nla_put_u8(skb, QCA_WLAN_VENDOR_ATTR_CONFIG_RX_NSS, rx_nss)) {
		hdd_err("nla_put failure");
		return -EINVAL;
	}

	return 0;
}

/**
 * hdd_get_optimized_power_config() - Get the number of spatial streams
 * supported by the adapter
 * @adapter: Pointer to HDD adapter
 * @skb: sk buffer to hold nl80211 attributes
 * @attr: Pointer to struct nlattr
 *
 * Return: 0 on success; error number otherwise
 */
static int hdd_get_optimized_power_config(struct hdd_adapter *adapter,
					  struct sk_buff *skb,
					  const struct nlattr *attr)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	uint8_t optimized_power_cfg;
	int errno;

	errno = wlan_hdd_validate_context(hdd_ctx);
	if (errno)
		return errno;

	optimized_power_cfg  = ucfg_pmo_get_power_save_mode(hdd_ctx->psoc);

	if (nla_put_u8(skb,
		       QCA_WLAN_VENDOR_ATTR_CONFIG_OPTIMIZED_POWER_MANAGEMENT,
		       optimized_power_cfg)) {
		hdd_err_rl("nla_put failure");
		return -EINVAL;
	}

	return 0;
}

/**
 * typedef config_getter_fn - get configuration handler
 * @adapter: The adapter being configured
 * @skb: sk buffer to hold nl80211 attributes
 * @attr: The nl80211 attribute being applied
 *
 * Defines the signature of functions in the attribute vtable
 *
 * Return: 0 if the attribute was handled successfully, otherwise an errno
 */
typedef int (*config_getter_fn)(struct hdd_adapter *adapter,
				struct sk_buff *skb,
				const struct nlattr *attr);

/**
 * struct config_getters
 * @id: vendor attribute which this entry handles
 * @cb: callback function to invoke to process the attribute when present
 */
struct config_getters {
	uint32_t id;
	size_t max_attr_len;
	config_getter_fn cb;
};

/* vtable for config getters */
static const struct config_getters config_getters[] = {
#ifdef WLAN_FEATURE_ELNA
	{QCA_WLAN_VENDOR_ATTR_CONFIG_ELNA_BYPASS,
	 sizeof(uint8_t),
	 hdd_get_elna_bypass},
#endif
	{QCA_WLAN_VENDOR_ATTR_CONFIG_ROAM_REASON,
	 sizeof(uint8_t),
	 hdd_get_roam_reason_vsie_status},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_TX_MPDU_AGGREGATION,
	 sizeof(uint8_t),
	 hdd_get_tx_ampdu},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_RX_MPDU_AGGREGATION,
	 sizeof(uint8_t),
	 hdd_get_rx_ampdu},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_TX_MSDU_AGGREGATION,
	 sizeof(uint8_t),
	 hdd_get_tx_amsdu},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_RX_MSDU_AGGREGATION,
	 sizeof(uint8_t),
	 hdd_get_rx_amsdu},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_LDPC,
	 sizeof(uint8_t),
	 hdd_vendor_attr_ldpc_get},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_TX_STBC,
	 sizeof(uint8_t),
	 hdd_vendor_attr_tx_stbc_get},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_RX_STBC,
	 sizeof(uint8_t),
	 hdd_vendor_attr_rx_stbc_get},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_CHANNEL_WIDTH,
	 sizeof(uint8_t),
	 hdd_get_channel_width},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_DYNAMIC_BW,
	 sizeof(uint8_t),
	 hdd_get_dynamic_bw},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_NSS,
	 sizeof(uint8_t),
	 hdd_get_nss_config},
	{QCA_WLAN_VENDOR_ATTR_CONFIG_OPTIMIZED_POWER_MANAGEMENT,
	 sizeof(uint8_t),
	 hdd_get_optimized_power_config},
	 {QCA_WLAN_VENDOR_ATTR_CONFIG_TX_NSS,
	 sizeof(uint8_t),
	 hdd_get_tx_nss_config},
	 {QCA_WLAN_VENDOR_ATTR_CONFIG_RX_NSS,
	 sizeof(uint8_t),
	 hdd_get_rx_nss_config},
};

/**
 * hdd_get_configuration() - Handle get configuration
 * @adapter: adapter upon which the vendor command was received
 * @tb: parsed attribute array
 *
 * This is a table-driven function which dispatches attributes
 * in a QCA_NL80211_VENDOR_SUBCMD_GET_WIFI_CONFIGURATION
 * vendor command.
 *
 * Return: 0 if there were no issues, otherwise errno of the last issue
 */
static int hdd_get_configuration(struct hdd_adapter *adapter,
				 struct nlattr **tb)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	uint32_t i, id;
	unsigned long nl_buf_len = NLMSG_HDRLEN;
	struct sk_buff *skb;
	struct nlattr *attr;
	config_getter_fn cb;
	int errno = 0;

	for (i = 0; i < QDF_ARRAY_SIZE(config_getters); i++) {
		id = config_getters[i].id;
		attr = tb[id];
		if (!attr)
			continue;

		nl_buf_len += NLA_HDRLEN +
			      NLA_ALIGN(config_getters[i].max_attr_len);
	}

	skb = cfg80211_vendor_cmd_alloc_reply_skb(hdd_ctx->wiphy, nl_buf_len);
	if (!skb) {
		hdd_err("cfg80211_vendor_cmd_alloc_reply_skb failed");
		return -ENOMEM;
	}

	for (i = 0; i < QDF_ARRAY_SIZE(config_getters); i++) {
		id = config_getters[i].id;
		attr = tb[id];
		if (!attr)
			continue;

		hdd_debug("Get wifi configuration %d", id);

		cb = config_getters[i].cb;
		errno = cb(adapter, skb, attr);
		if (errno)
			break;
	}

	if (errno) {
		hdd_err("Failed to get wifi configuration, errno = %d", errno);
		kfree_skb(skb);
		return -EINVAL;
	}

	cfg80211_vendor_cmd_reply(skb);

	return errno;
}

/**
 * hdd_set_independent_configuration() - Handle independent attributes
 * @adapter: adapter upon which the vendor command was received
 * @tb: parsed attribute array
 *
 * This is a table-driven function which dispatches independent
 * attributes in a QCA_NL80211_VENDOR_SUBCMD_SET_WIFI_CONFIGURATION
 * vendor command. An attribute is considered independent if it
 * doesn't depend upon any other attributes
 *
 * Return: 0 if there were no issues, otherwise errno of the last issue
 */
static int hdd_set_independent_configuration(struct hdd_adapter *adapter,
					     struct nlattr **tb)
{
	uint32_t i;
	uint32_t id;
	struct nlattr *attr;
	independent_setter_fn cb;
	int errno = 0;
	int ret;

	for (i = 0; i < QDF_ARRAY_SIZE(independent_setters); i++) {
		id = independent_setters[i].id;
		attr = tb[id];
		if (!attr)
			continue;

		hdd_debug("Set wifi configuration %d", id);

		cb = independent_setters[i].cb;
		ret = cb(adapter, attr);
		if (ret)
			errno = ret;
	}

	return errno;
}

/**
 * typedef interdependent_setter_fn - interdependent attribute handler
 * @adapter: The adapter being configured
 * @tb: The parsed nl80211 attributes being applied
 *
 * Defines the signature of functions in the interdependent attribute vtable
 *
 * Return: 0 if attributes were handled successfully, otherwise an errno
 */
typedef int (*interdependent_setter_fn)(struct hdd_adapter *adapter,
					struct nlattr **tb);

/* vtable for interdependent setters */
static const interdependent_setter_fn interdependent_setters[] = {
	hdd_config_access_policy,
	hdd_config_mpdu_aggregation,
	hdd_config_ant_div_period,
	hdd_config_ant_div_snr_weight,
	wlan_hdd_cfg80211_wifi_set_reorder_timeout,
	wlan_hdd_cfg80211_wifi_set_rx_blocksize,
	hdd_config_msdu_aggregation,
	hdd_config_vdev_chains,
	hdd_config_ani,
	hdd_config_tx_rx_nss,
};

/**
 * hdd_set_interdependent_configuration() - Handle interdependent attributes
 * @adapter: adapter upon which the vendor command was received
 * @tb: parsed attribute array
 *
 * This is a table-driven function which handles interdependent
 * attributes in a QCA_NL80211_VENDOR_SUBCMD_SET_WIFI_CONFIGURATION
 * vendor command. A set of attributes is considered interdependent if
 * they depend upon each other. In the typical case if one of the
 * attributes is present in the the attribute array, then all of the
 * attributes must be present.
 *
 * Return: 0 if there were no issues, otherwise errno of the last issue
 */
static int hdd_set_interdependent_configuration(struct hdd_adapter *adapter,
						struct nlattr **tb)
{
	uint32_t i;
	interdependent_setter_fn cb;
	int errno = 0;
	int ret;

	for (i = 0; i < QDF_ARRAY_SIZE(interdependent_setters); i++) {
		cb = interdependent_setters[i];
		ret = cb(adapter, tb);
		if (ret)
			errno = ret;
	}

	return errno;
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
	int errno;
	int ret;

	hdd_enter_dev(dev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	errno = wlan_hdd_validate_context(hdd_ctx);
	if (errno)
		return errno;

	if (wlan_cfg80211_nla_parse(tb, QCA_WLAN_VENDOR_ATTR_CONFIG_MAX, data,
				    data_len, wlan_hdd_wifi_config_policy)) {
		hdd_err("invalid attr");
		return -EINVAL;
	}

	ret = hdd_set_independent_configuration(adapter, tb);
	if (ret)
		errno = ret;

	ret = hdd_set_interdependent_configuration(adapter, tb);
	if (ret)
		errno = ret;

	return errno;
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
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_wifi_configuration_set(wiphy, wdev,
							   data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}

/**
 * __wlan_hdd_cfg80211_wifi_configuration_get() - Wifi configuration
 * vendor command
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
__wlan_hdd_cfg80211_wifi_configuration_get(struct wiphy *wiphy,
					   struct wireless_dev *wdev,
					   const void *data,
					   int data_len)
{
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx  = wiphy_priv(wiphy);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_CONFIG_MAX + 1];
	int errno;
	int ret;

	hdd_enter_dev(dev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	errno = wlan_hdd_validate_context(hdd_ctx);
	if (errno)
		return errno;

	if (wlan_cfg80211_nla_parse(tb, QCA_WLAN_VENDOR_ATTR_CONFIG_MAX, data,
				    data_len, wlan_hdd_wifi_config_policy)) {
		hdd_err("invalid attr");
		return -EINVAL;
	}

	ret = hdd_get_configuration(adapter, tb);
	if (ret)
		errno = ret;

	return errno;
}

/**
 * wlan_hdd_cfg80211_wifi_configuration_get() - Wifi configuration
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
static int wlan_hdd_cfg80211_wifi_configuration_get(struct wiphy *wiphy,
						    struct wireless_dev *wdev,
						    const void *data,
						    int data_len)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_wifi_configuration_get(wiphy, wdev,
							   data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}

static void hdd_disable_runtime_pm_for_user(struct hdd_context *hdd_ctx)
{
	struct hdd_runtime_pm_context *ctx = &hdd_ctx->runtime_context;

	if (!ctx)
		return;

	if (ctx->is_user_wakelock_acquired)
		return;

	ctx->is_user_wakelock_acquired = true;
	qdf_runtime_pm_prevent_suspend(&ctx->user);
}

static int hdd_test_config_6ghz_security_test_mode(struct hdd_context *hdd_ctx,
						   struct nlattr *attr)

{
	uint8_t cfg_val;
	bool rf_test_mode = false;
	QDF_STATUS status;

	status = ucfg_mlme_is_rf_test_mode_enabled(hdd_ctx->psoc,
						   &rf_test_mode);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("Get rf test mode failed");
		return -EINVAL;
	}
	if (rf_test_mode) {
		hdd_err("rf test mode is enabled, ignore setting");
		return 0;
	}

	cfg_val = nla_get_u8(attr);
	hdd_debug("safe mode setting %d", cfg_val);
	wlan_mlme_set_safe_mode_enable(hdd_ctx->psoc, cfg_val);
	if (cfg_val) {
		wlan_cm_set_check_6ghz_security(hdd_ctx->psoc, false);
		wlan_cm_set_6ghz_key_mgmt_mask(hdd_ctx->psoc,
					       DEFAULT_KEYMGMT_6G_MASK);
	} else {
		wlan_cm_set_check_6ghz_security(hdd_ctx->psoc, true);
		wlan_cm_set_6ghz_key_mgmt_mask(hdd_ctx->psoc,
					       ALLOWED_KEYMGMT_6G_MASK);
	}

	return 0;
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
		struct wireless_dev *wdev, const void *data, int data_len)
{
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx  = wiphy_priv(wiphy);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_MAX + 1];
	int ret_val = 0;
	uint8_t cfg_val = 0;
	uint8_t ini_val = 0;
	uint8_t set_val = 0;
	struct sme_config_params *sme_config;
	bool update_sme_cfg = false;
	uint8_t tid = 0, ac;
	uint16_t buff_size = 0;
	mac_handle_t mac_handle;
	QDF_STATUS status;
	bool bval = false;
	uint8_t value = 0;
	uint8_t wmm_mode = 0;
	uint32_t bss_max_idle_period = 0;
	uint32_t cmd_id;
	struct keep_alive_req keep_alive_req = {0};
	struct set_wfatest_params wfa_param = {0};
	struct hdd_station_ctx *hdd_sta_ctx =
		WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	hdd_enter_dev(dev);

	sme_config = qdf_mem_malloc(sizeof(*sme_config));
	if (!sme_config)
		return -ENOMEM;

	mac_handle = hdd_ctx->mac_handle;
	sme_get_config_param(mac_handle, sme_config);

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
		ret_val = sme_set_addba_accept(mac_handle, adapter->vdev_id,
					       cfg_val);
		if (ret_val)
			goto send_err;
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_MCS]) {
		cfg_val = nla_get_u8(tb[
			QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_MCS]);
		hdd_debug("set HE MCS value 0x%0X", cfg_val);
		ret_val = sme_update_he_mcs(mac_handle, adapter->vdev_id,
					    cfg_val);
		if (ret_val)
			goto send_err;
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_WMM_ENABLE]) {
		cfg_val = nla_get_u8(tb[
			QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_WMM_ENABLE]);
		if (!cfg_val) {
			sme_config->csr_config.WMMSupportMode =
				hdd_to_csr_wmm_mode(HDD_WMM_USER_MODE_NO_QOS);
			hdd_debug("wmm is disabled");
		} else {
			status = ucfg_mlme_get_wmm_mode(hdd_ctx->psoc,
							&wmm_mode);
			if (!QDF_IS_STATUS_SUCCESS(status)) {
				hdd_err("Get wmm_mode failed");
				ret_val = -EINVAL;
				goto send_err;
			}
			sme_config->csr_config.WMMSupportMode =
				hdd_to_csr_wmm_mode(wmm_mode);
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

		ret_val = wma_cli_set_command(adapter->vdev_id,
				WMI_VDEV_PARAM_BA_MODE, set_val, VDEV_CMD);
		if (ret_val) {
			hdd_err("Set BA operating mode failed");
			goto send_err;
		}
		if (!cfg_val) {
			ret_val = wma_cli_set_command(adapter->vdev_id,
				WMI_VDEV_PARAM_AMSDU_AGGREGATION_SIZE_OPTIMIZATION,
				0, VDEV_CMD);
		}
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
		ret_val = sme_update_he_frag_supp(mac_handle,
						  adapter->vdev_id, set_val);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_WEP_TKIP_IN_HE]) {
		cfg_val = nla_get_u8(tb[
			QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_WEP_TKIP_IN_HE]);
		sme_config->csr_config.wep_tkip_in_he = cfg_val;
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
				buff_size = nla_get_u16(tb[
				QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_ADDBA_BUFF_SIZE]);
			ret_val = sme_send_addba_req(mac_handle,
						     adapter->vdev_id,
						     tid, buff_size);
		} else if (cfg_val == QCA_WLAN_DELETE_BA) {
		} else {
			hdd_err("Invalid BA session cfg");
			ret_val = -EINVAL;
			goto send_err;
		}
	} else if (tb[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_ADDBA_BUFF_SIZE]) {
		buff_size = nla_get_u16(tb[
		QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_ADDBA_BUFF_SIZE]);
		hdd_debug("set buff size to %d for all tids", buff_size);
		ret_val = sme_set_ba_buff_size(mac_handle,
					       adapter->vdev_id, buff_size);
		if (ret_val)
			goto send_err;
		if (buff_size > 64)
			/* Configure ADDBA req buffer size to 256 */
			set_val = 3;
		else
			/* Configure ADDBA req buffer size to 64 */
			set_val = 2;
		ret_val = wma_cli_set_command(adapter->vdev_id,
				WMI_VDEV_PARAM_BA_MODE, set_val, VDEV_CMD);
		if (ret_val)
			hdd_err("Failed to set BA operating mode %d", set_val);
		ret_val = wma_cli_set_command(adapter->vdev_id,
					      GEN_VDEV_PARAM_AMPDU,
					      buff_size, GEN_CMD);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_ENABLE_NO_ACK]) {
		int he_mcs_val;

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
		ret_val = sme_set_no_ack_policy(mac_handle,
						adapter->vdev_id,
						cfg_val, ac);
		if (cfg_val) {
			status = ucfg_mlme_get_vht_enable2x2(hdd_ctx->psoc,
							     &bval);
			if (!QDF_IS_STATUS_SUCCESS(status))
				hdd_err("unable to get vht_enable2x2");
			if (bval)
				/*2x2 MCS 5 value*/
				he_mcs_val = 0x45;
			else
				/*1x1 MCS 5 value*/
				he_mcs_val = 0x25;

			if (hdd_set_11ax_rate(adapter, he_mcs_val, NULL))
				hdd_err("HE MCS set failed, MCS val %0x",
						he_mcs_val);
		} else {
			if (hdd_set_11ax_rate(adapter, 0xFF, NULL))
				hdd_err("disable fixed rate failed");
		}
	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_KEEP_ALIVE_FRAME_TYPE;
	if (tb[cmd_id]) {
		cfg_val = nla_get_u8(tb[cmd_id]);
		hdd_debug("Keep alive data type %d", cfg_val);
		if (cfg_val == QCA_WLAN_KEEP_ALIVE_DATA) {
			ret_val = hdd_set_grat_arp_keepalive(adapter);
			if (ret_val) {
				hdd_err("Keep alive data type set failed");
				goto send_err;
			}
		} else {
			if (cfg_val == QCA_WLAN_KEEP_ALIVE_MGMT)
				keep_alive_req.packetType =
						SIR_KEEP_ALIVE_MGMT_FRAME;
			else
				keep_alive_req.packetType =
						SIR_KEEP_ALIVE_NULL_PKT;
			ucfg_mlme_get_sta_keep_alive_period(
						hdd_ctx->psoc,
						&keep_alive_req.timePeriod);
			keep_alive_req.sessionId = adapter->vdev_id;
			status = sme_set_keep_alive(hdd_ctx->mac_handle,
						    adapter->vdev_id,
						    &keep_alive_req);
			if (QDF_IS_STATUS_ERROR(status)) {
				hdd_err("Failed to set keepalive");
				ret_val = qdf_status_to_os_return(status);
				goto send_err;
			}
		}
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_LTF]) {
		cfg_val = nla_get_u8(tb[
				QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_LTF]);
		hdd_debug("Set HE LTF to %d", cfg_val);
		ret_val = sme_set_auto_rate_he_ltf(mac_handle,
						   adapter->vdev_id,
						   cfg_val);
		if (ret_val)
			sme_err("Failed to set auto rate HE LTF");

		ret_val = wma_cli_set_command(adapter->vdev_id,
					      WMI_VDEV_PARAM_HE_LTF,
					      cfg_val, VDEV_CMD);
		if (ret_val)
			goto send_err;
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_ENABLE_TX_BEAMFORMEE]) {
		cfg_val = nla_get_u8(tb[
			QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_ENABLE_TX_BEAMFORMEE]);
		hdd_debug("Set Tx beamformee to %d", cfg_val);
		ret_val = sme_update_tx_bfee_supp(mac_handle,
						  adapter->vdev_id,
						  cfg_val);
		if (ret_val)
			sme_err("Failed to set Tx beamformee cap");

	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_RX_CTRL_FRAME_TO_MBSS;
	if (tb[cmd_id]) {
		cfg_val = nla_get_u8(tb[cmd_id]);
		ret_val = sme_update_he_capabilities(mac_handle,
						     adapter->vdev_id,
						     cfg_val, cmd_id);
		if (ret_val)
			sme_err("Failed to update HE cap");
	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_BCAST_TWT_SUPPORT;
	if (tb[cmd_id]) {
		cfg_val = nla_get_u8(tb[cmd_id]);
		ret_val = sme_update_he_capabilities(mac_handle,
						     adapter->vdev_id,
						     cfg_val, cmd_id);
		if (ret_val)
			sme_err("Failed to update HE cap");
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_TX_BEAMFORMEE_NSTS]) {
		cfg_val = nla_get_u8(tb[
			QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_TX_BEAMFORMEE_NSTS]);
		status = ucfg_mlme_cfg_get_vht_tx_bfee_ant_supp(hdd_ctx->psoc,
								&value);
		if (!QDF_IS_STATUS_SUCCESS(status))
			hdd_err("unable to get tx_bfee_ant_supp");

		if (!cfg_in_range(CFG_VHT_BEAMFORMEE_ANT_SUPP, cfg_val)) {
			hdd_err("NSTS %d not supported, supp_val %d", cfg_val,
				value);
			ret_val = -ENOTSUPP;
			goto send_err;
		}
		hdd_debug("Set Tx beamformee NSTS to %d", cfg_val);
		ret_val = sme_update_tx_bfee_nsts(hdd_ctx->mac_handle,
						  adapter->vdev_id,
						  cfg_val,
						  value);
		if (ret_val)
			sme_err("Failed to set Tx beamformee cap");

	}

	if (tb[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_MAC_PADDING_DUR]) {
		cfg_val = nla_get_u8(tb[
				     QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_MAC_PADDING_DUR]);
		if (cfg_val) {
			hdd_debug("Set HE mac padding dur to %d", cfg_val);
			ret_val = sme_cli_set_command(adapter->vdev_id,
					WMI_VDEV_PARAM_MU_EDCA_FW_UPDATE_EN,
					0, VDEV_CMD);
			if (ret_val)
				hdd_err("MU_EDCA update disable failed");
			sme_set_usr_cfg_mu_edca(hdd_ctx->mac_handle, true);
			sme_set_he_mu_edca_def_cfg(hdd_ctx->mac_handle);
			if (sme_update_mu_edca_params(hdd_ctx->mac_handle,
						      adapter->vdev_id))
				hdd_err("Failed to send mu edca params");
		} else {
			ret_val = sme_cli_set_command(adapter->vdev_id,
					WMI_VDEV_PARAM_MU_EDCA_FW_UPDATE_EN,
					1, VDEV_CMD);
			sme_set_usr_cfg_mu_edca(hdd_ctx->mac_handle, false);
		}
		ret_val = sme_update_he_trigger_frm_mac_pad(hdd_ctx->mac_handle,
							    adapter->vdev_id,
							    cfg_val);
		if (ret_val)
			hdd_err("Failed to set Trig frame mac padding cap");
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_OVERRIDE_MU_EDCA]) {
		cfg_val = nla_get_u8(tb[
				     QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_OVERRIDE_MU_EDCA]);
		if (cfg_val) {
			ret_val = sme_cli_set_command(adapter->vdev_id,
					WMI_VDEV_PARAM_MU_EDCA_FW_UPDATE_EN,
					0, VDEV_CMD);
			if (ret_val)
				hdd_err("MU_EDCA update disable failed");
			sme_set_usr_cfg_mu_edca(hdd_ctx->mac_handle, true);
			sme_set_he_mu_edca_def_cfg(hdd_ctx->mac_handle);
			if (sme_update_mu_edca_params(hdd_ctx->mac_handle,
						      adapter->vdev_id))
				hdd_err("Failed to send mu edca params");
		} else {
			ret_val = sme_cli_set_command(adapter->vdev_id,
					WMI_VDEV_PARAM_MU_EDCA_FW_UPDATE_EN,
					1, VDEV_CMD);
			sme_set_usr_cfg_mu_edca(hdd_ctx->mac_handle, false);
		}
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_OM_CTRL_SUPP]) {
		cfg_val = nla_get_u8(tb[
				     QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_OM_CTRL_SUPP]);
		ret_val = sme_update_he_om_ctrl_supp(hdd_ctx->mac_handle,
						     adapter->vdev_id,
						     cfg_val);
	}

	cmd_id =
		QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_USE_BSSID_IN_PROBE_REQ_RA;
	if (tb[cmd_id]) {
		cfg_val = nla_get_u8(tb[cmd_id]);
		if (cfg_val)
			status = ucfg_mlme_set_scan_probe_unicast_ra(
							hdd_ctx->psoc, true);
		else
			status = ucfg_mlme_set_scan_probe_unicast_ra(
							hdd_ctx->psoc, false);
		if (!QDF_IS_STATUS_SUCCESS(status))
			hdd_err("unable to set unicat probe ra cfg");
	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_OMI_TX;
	if (tb[cmd_id]) {
		struct nlattr *tb2[QCA_WLAN_VENDOR_ATTR_HE_OMI_MAX + 1];
		struct nlattr *curr_attr;
		int tmp, rc;
		nla_for_each_nested(curr_attr, tb[cmd_id], tmp) {
			rc = wlan_cfg80211_nla_parse(
					tb2, QCA_WLAN_VENDOR_ATTR_HE_OMI_MAX,
					nla_data(curr_attr),
					nla_len(curr_attr),
					qca_wlan_vendor_attr_he_omi_tx_policy);
			if (rc) {
				hdd_err("Invalid ATTR");
				goto send_err;
			}
			cmd_id = QCA_WLAN_VENDOR_ATTR_HE_OMI_CH_BW;
			if (tb2[cmd_id]) {
				cfg_val = nla_get_u8(tb2[cmd_id]);
				ret_val = sme_set_he_om_ctrl_param(
							hdd_ctx->mac_handle,
							adapter->vdev_id,
							cmd_id, cfg_val);
			}
			cmd_id = QCA_WLAN_VENDOR_ATTR_HE_OMI_RX_NSS;
			if (tb2[cmd_id]) {
				cfg_val = nla_get_u8(tb2[cmd_id]);
				ret_val = sme_set_he_om_ctrl_param(
							hdd_ctx->mac_handle,
							adapter->vdev_id,
							cmd_id, cfg_val);
			}

			cmd_id = QCA_WLAN_VENDOR_ATTR_HE_OMI_ULMU_DISABLE;
			if (tb2[cmd_id]) {
				cfg_val = nla_get_u8(tb2[cmd_id]);
				ret_val = sme_set_he_om_ctrl_param(
							hdd_ctx->mac_handle,
							adapter->vdev_id,
							cmd_id, cfg_val);
			}

			cmd_id = QCA_WLAN_VENDOR_ATTR_HE_OMI_TX_NSTS;
			if (tb2[cmd_id]) {
				cfg_val = nla_get_u8(tb2[cmd_id]);
				ret_val = sme_set_he_om_ctrl_param(
							hdd_ctx->mac_handle,
							adapter->vdev_id,
							cmd_id, cfg_val);
			}

			cmd_id = QCA_WLAN_VENDOR_ATTR_HE_OMI_ULMU_DATA_DISABLE;
			if (tb2[cmd_id]) {
				cfg_val = nla_get_u8(tb2[cmd_id]);
				ret_val = sme_set_he_om_ctrl_param(
							hdd_ctx->mac_handle,
							adapter->vdev_id,
							cmd_id, cfg_val);
			}

		}
		if (ret_val) {
			sme_reset_he_om_ctrl(hdd_ctx->mac_handle);
			goto send_err;
		}
		ret_val = sme_send_he_om_ctrl_update(hdd_ctx->mac_handle,
						     adapter->vdev_id);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_CLEAR_HE_OM_CTRL_CONFIG])
		sme_reset_he_om_ctrl(hdd_ctx->mac_handle);

	cmd_id = QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_TX_SUPPDU;
	if (tb[cmd_id]) {
		cfg_val = nla_get_u8(tb[cmd_id]);
		hdd_debug("Configure Tx SU PPDU enable %d", cfg_val);
		if (cfg_val)
			sme_config_su_ppdu_queue(adapter->vdev_id, true);
		else
			sme_config_su_ppdu_queue(adapter->vdev_id, false);
	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_ENABLE_2G_VHT;
	if (tb[cmd_id]) {
		cfg_val = nla_get_u8(tb[cmd_id]);
		hdd_debug("Configure 2G VHT support %d", cfg_val);
		ucfg_mlme_set_vht_for_24ghz(hdd_ctx->psoc,
					    (cfg_val ? true : false));
	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_HTC_HE_SUPP;
	if (tb[cmd_id]) {
		cfg_val = nla_get_u8(tb[cmd_id]);
		hdd_debug("Configure +HTC_HE support %d", cfg_val);
		sme_update_he_htc_he_supp(hdd_ctx->mac_handle,
					  adapter->vdev_id,
					  (cfg_val ? true : false));
	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_PUNCTURED_PREAMBLE_RX;
	if (tb[cmd_id]) {
		cfg_val = nla_get_u8(tb[cmd_id]);
		hdd_debug("Configure Punctured preamble Rx %d", cfg_val);
		ret_val = sme_update_he_capabilities(mac_handle,
						     adapter->vdev_id,
						     cfg_val, cmd_id);
		if (ret_val)
			sme_err("Failed to update HE cap");
	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_SET_HE_TESTBED_DEFAULTS;
	if (tb[cmd_id]) {
		hdd_disable_runtime_pm_for_user(hdd_ctx);
		cfg_val = nla_get_u8(tb[cmd_id]);
		hdd_debug("Configure HE testbed defaults %d", cfg_val);
		if (!cfg_val)
			sme_reset_he_caps(hdd_ctx->mac_handle,
					  adapter->vdev_id);
		else
			sme_set_he_testbed_def(hdd_ctx->mac_handle,
					       adapter->vdev_id);
	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_ACTION_TX_TB_PPDU;
	if (tb[cmd_id]) {
		cfg_val = nla_get_u8(tb[cmd_id]);
		hdd_debug("Configure Action frame Tx in TB PPDU %d", cfg_val);
		sme_config_action_tx_in_tb_ppdu(hdd_ctx->mac_handle,
						adapter->vdev_id, cfg_val);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_TWT_SETUP]) {
		ret_val = hdd_test_config_twt_setup_session(adapter, tb);
		if (ret_val)
			goto send_err;
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_TWT_TERMINATE]) {
		ret_val = hdd_test_config_twt_terminate_session(adapter, tb);
		if (ret_val)
			goto send_err;
	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_TWT_REQ_SUPPORT;
	if (tb[cmd_id]) {
		cfg_val = nla_get_u8(tb[cmd_id]);
		hdd_debug("twt_request: val %d", cfg_val);
		ret_val = sme_update_he_twt_req_support(hdd_ctx->mac_handle,
							adapter->vdev_id,
							cfg_val);
	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_FULL_BW_UL_MU_MIMO;
	if (tb[cmd_id]) {
		cfg_val = nla_get_u8(tb[cmd_id]);
		ini_val = cfg_get(hdd_ctx->psoc, CFG_HE_UL_MUMIMO);
		hdd_debug("fullbw_ulmumimo: cfg %d, ini %d", cfg_val, ini_val);
		if (cfg_val) {
			switch (ini_val) {
			case CFG_NO_SUPPORT_UL_MUMIMO:
			case CFG_FULL_BW_SUPPORT_UL_MUMIMO:
				cfg_val = CFG_FULL_BW_SUPPORT_UL_MUMIMO;
				break;
			case CFG_PARTIAL_BW_SUPPORT_UL_MUMIMO:
			case CFG_FULL_PARTIAL_BW_SUPPORT_UL_MUMIMO:
				cfg_val = CFG_FULL_PARTIAL_BW_SUPPORT_UL_MUMIMO;
				break;
			}
		} else {
			switch (ini_val) {
			case CFG_NO_SUPPORT_UL_MUMIMO:
			case CFG_FULL_BW_SUPPORT_UL_MUMIMO:
				cfg_val = CFG_NO_SUPPORT_UL_MUMIMO;
				break;
			case CFG_PARTIAL_BW_SUPPORT_UL_MUMIMO:
			case CFG_FULL_PARTIAL_BW_SUPPORT_UL_MUMIMO:
				cfg_val = CFG_PARTIAL_BW_SUPPORT_UL_MUMIMO;
				break;
			}
		}
		ret_val = sme_update_he_full_ul_mumimo(hdd_ctx->mac_handle,
						       adapter->vdev_id,
						       cfg_val);
	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_DISABLE_DATA_MGMT_RSP_TX;
	if (tb[cmd_id]) {
		cfg_val = nla_get_u8(tb[cmd_id]);
		hdd_debug("disable Tx cfg: val %d", cfg_val);
		sme_set_cfg_disable_tx(hdd_ctx->mac_handle, adapter->vdev_id,
				       cfg_val);
	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_PMF_PROTECTION;
	if (tb[cmd_id]) {
		cfg_val = nla_get_u8(tb[cmd_id]);
		hdd_debug("pmf cfg: val %d", cfg_val);
		sme_set_pmf_wep_cfg(hdd_ctx->mac_handle, cfg_val);
	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_BSS_MAX_IDLE_PERIOD;
	if (tb[cmd_id]) {
		cfg_val = nla_get_u16(tb[cmd_id]);
		hdd_debug("bss max idle period %d", cfg_val);
		sme_set_bss_max_idle_period(hdd_ctx->mac_handle, cfg_val);
	}

	cmd_id =
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_BSS_MAX_IDLE_PERIOD_ENABLE;
	if (tb[cmd_id]) {
		cfg_val = nla_get_u8(tb[cmd_id]);
		if (cfg_val)
			ucfg_mlme_get_sta_keep_alive_period(
							hdd_ctx->psoc,
							&bss_max_idle_period);
		hdd_debug("bss max idle period %d", bss_max_idle_period);
		sme_set_bss_max_idle_period(hdd_ctx->mac_handle,
					    bss_max_idle_period);
	}
	cmd_id = QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_DISASSOC_TX;
	if (tb[cmd_id]) {
		hdd_info("Send disassoc mgmt frame");
		sme_send_disassoc_req_frame(hdd_ctx->mac_handle,
					    adapter->vdev_id,
					    hdd_sta_ctx->conn_info.bssid.bytes,
					    1, false);
	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_RU_242_TONE_TX;
	if (tb[cmd_id]) {
		cfg_val = nla_get_u8(tb[cmd_id]);
		hdd_info("RU 242 tone Tx enable: %d", cfg_val);
		sme_set_ru_242_tone_tx_cfg(hdd_ctx->mac_handle, cfg_val);
		if (cfg_val)
			hdd_update_channel_width(
					adapter, eHT_CHANNEL_WIDTH_20MHZ,
					WNI_CFG_CHANNEL_BONDING_MODE_DISABLE);
	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_ER_SU_PPDU_TYPE;
	if (tb[cmd_id]) {
		cfg_val = nla_get_u8(tb[cmd_id]);
		hdd_debug("EU SU PPDU type Tx enable: %d", cfg_val);
		if (cfg_val) {
			hdd_update_channel_width(
					adapter, eHT_CHANNEL_WIDTH_20MHZ,
					WNI_CFG_CHANNEL_BONDING_MODE_DISABLE);
			hdd_set_tx_stbc(adapter, 0);
			hdd_set_11ax_rate(adapter, 0x400, NULL);
			status = wma_cli_set_command(adapter->vdev_id,
					WMI_VDEV_PARAM_HE_RANGE_EXT,
					1, VDEV_CMD);
			if (QDF_IS_STATUS_ERROR(status))
				hdd_err("failed to set HE_RANGE_EXT, %d",
					status);
			status = wma_cli_set_command(adapter->vdev_id,
					WMI_VDEV_PARAM_NON_DATA_HE_RANGE_EXT,
					1, VDEV_CMD);
			if (QDF_IS_STATUS_ERROR(status))
				hdd_err("fail to set NON_DATA_HE_RANGE_EXT %d",
					status);
		} else {
			hdd_update_channel_width(
					adapter, eHT_CHANNEL_WIDTH_80MHZ,
					WNI_CFG_CHANNEL_BONDING_MODE_ENABLE);
			hdd_set_tx_stbc(adapter, 1);
			hdd_set_11ax_rate(adapter, 0xFFFF, NULL);
			status = wma_cli_set_command(adapter->vdev_id,
					WMI_VDEV_PARAM_HE_RANGE_EXT,
					0, VDEV_CMD);
			if (QDF_IS_STATUS_ERROR(status))
				hdd_err("failed to set HE_RANGE_EXT, %d",
					status);
			status = wma_cli_set_command(adapter->vdev_id,
					WMI_VDEV_PARAM_NON_DATA_HE_RANGE_EXT,
					0, VDEV_CMD);
			if (QDF_IS_STATUS_ERROR(status))
				hdd_err("fail to set NON_DATA_HE_RANGE_EXT %d",
					status);
		}

	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_FT_REASSOCREQ_RSNXE_USED;
	if (tb[cmd_id]) {
		wfa_param.vdev_id = adapter->vdev_id;
		wfa_param.value = nla_get_u8(tb[cmd_id]);

		if (wfa_param.value < RSNXE_DEFAULT ||
		    wfa_param.value > RSNXE_OVERRIDE_2) {
			hdd_debug("Invalid RSNXE override %d", wfa_param.value);
			goto send_err;
		}
		wfa_param.cmd = WFA_CONFIG_RXNE;
		hdd_info("send wfa test config RXNE used %d", wfa_param.value);

		ret_val = ucfg_send_wfatest_cmd(adapter->vdev, &wfa_param);
	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_IGNORE_CSA;
	if (tb[cmd_id]) {
		wfa_param.vdev_id = adapter->vdev_id;
		wfa_param.value = nla_get_u8(tb[cmd_id]);

		if (wfa_param.value != CSA_DEFAULT &&
		    wfa_param.value != CSA_IGNORE) {
			hdd_debug("Invalid CSA config %d", wfa_param.value);
			goto send_err;
		}
		wfa_param.cmd = WFA_CONFIG_CSA;
		hdd_info("send wfa test config CSA used %d", wfa_param.value);

		ret_val = ucfg_send_wfatest_cmd(adapter->vdev, &wfa_param);
	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_6GHZ_SECURITY_TEST_MODE;
	if (tb[cmd_id]) {
		ret_val = hdd_test_config_6ghz_security_test_mode(hdd_ctx,
								  tb[cmd_id]);
		if (ret_val)
			goto send_err;
	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_OCI_OVERRIDE;
	if (tb[cmd_id]) {
		struct nlattr *tb2[QCA_WLAN_VENDOR_ATTR_OCI_OVERRIDE_MAX + 1];

		wfa_param.vdev_id = adapter->vdev_id;
		wfa_param.cmd = WFA_CONFIG_OCV;
		if (wlan_cfg80211_nla_parse_nested(
				tb2, QCA_WLAN_VENDOR_ATTR_OCI_OVERRIDE_MAX,
				tb[cmd_id], wlan_oci_override_policy)) {
			hdd_debug("Failed to parse OCI override");
			goto send_err;
		}

		if (!(tb2[QCA_WLAN_VENDOR_ATTR_OCI_OVERRIDE_FRAME_TYPE] &&
		      tb2[QCA_WLAN_VENDOR_ATTR_OCI_OVERRIDE_FREQUENCY])) {
			hdd_debug("Invalid ATTR FRAME_TYPE/FREQUENCY");
			goto send_err;
		}

		wfa_param.ocv_param = qdf_mem_malloc(
				sizeof(struct ocv_wfatest_params));
		if (!wfa_param.ocv_param) {
			hdd_err("Failed to alloc memory for ocv param");
			goto send_err;
		}

		cmd_id = QCA_WLAN_VENDOR_ATTR_OCI_OVERRIDE_FRAME_TYPE;
		switch (nla_get_u8(tb2[cmd_id])) {
		case QCA_WLAN_VENDOR_OCI_OVERRIDE_FRAME_SA_QUERY_REQ:
			wfa_param.ocv_param->frame_type =
				WMI_HOST_WFA_CONFIG_OCV_FRMTYPE_SAQUERY_REQ;
			break;

		case QCA_WLAN_VENDOR_OCI_OVERRIDE_FRAME_SA_QUERY_RESP:
			wfa_param.ocv_param->frame_type =
				WMI_HOST_WFA_CONFIG_OCV_FRMTYPE_SAQUERY_RSP;
			break;

		case QCA_WLAN_VENDOR_OCI_OVERRIDE_FRAME_FT_REASSOC_REQ:
			wfa_param.ocv_param->frame_type =
				WMI_HOST_WFA_CONFIG_OCV_FRMTYPE_FT_REASSOC_REQ;
			break;

		case QCA_WLAN_VENDOR_OCI_OVERRIDE_FRAME_FILS_REASSOC_REQ:
			wfa_param.ocv_param->frame_type =
			WMI_HOST_WFA_CONFIG_OCV_FRMTYPE_FILS_REASSOC_REQ;
			break;

		default:
			hdd_debug("Invalid frame type for ocv test config %d",
				  nla_get_u8(tb2[cmd_id]));
			qdf_mem_free(wfa_param.ocv_param);
				goto send_err;
		}

		cmd_id = QCA_WLAN_VENDOR_ATTR_OCI_OVERRIDE_FREQUENCY;
		wfa_param.ocv_param->freq = nla_get_u32(tb2[cmd_id]);

		if (!WLAN_REG_IS_24GHZ_CH_FREQ(wfa_param.ocv_param->freq) &&
		    !WLAN_REG_IS_5GHZ_CH_FREQ(wfa_param.ocv_param->freq) &&
		    !WLAN_REG_IS_6GHZ_CHAN_FREQ(wfa_param.ocv_param->freq)) {
			hdd_debug("Invalid Freq %d", wfa_param.ocv_param->freq);
			qdf_mem_free(wfa_param.ocv_param);
			goto send_err;
		}

		hdd_info("send wfa test config OCV frame type %d freq %d",
			 wfa_param.ocv_param->frame_type,
			 wfa_param.ocv_param->freq);
		ret_val = ucfg_send_wfatest_cmd(adapter->vdev, &wfa_param);
		qdf_mem_free(wfa_param.ocv_param);
	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_IGNORE_SA_QUERY_TIMEOUT;
	if (tb[cmd_id]) {
		wfa_param.vdev_id = adapter->vdev_id;
		wfa_param.value = nla_get_u8(tb[cmd_id]);

		if (wfa_param.value != SA_QUERY_TIMEOUT_DEFAULT &&
		    wfa_param.value != SA_QUERY_TIMEOUT_IGNORE) {
			hdd_debug("Invalid SA query timeout config %d",
				  wfa_param.value);
			goto send_err;
		}
		wfa_param.cmd = WFA_CONFIG_SA_QUERY;
		hdd_info("send wfa test config SAquery %d", wfa_param.value);

		ret_val = ucfg_send_wfatest_cmd(adapter->vdev, &wfa_param);
	}

	cmd_id = QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_FILS_DISCOVERY_FRAMES_TX;
	if (tb[cmd_id] && adapter->device_mode == QDF_SAP_MODE) {
		wfa_param.vdev_id = adapter->vdev_id;
		wfa_param.value = nla_get_u8(tb[cmd_id]);

		if (!(wfa_param.value == FILS_DISCV_FRAMES_DISABLE ||
		      wfa_param.value == FILS_DISCV_FRAMES_ENABLE)) {
			hdd_debug("Invalid FILS_DISCV_FRAMES config %d",
				  wfa_param.value);
			goto send_err;
		}
		wfa_param.cmd = WFA_FILS_DISCV_FRAMES;
		hdd_info("send wfa FILS_DISCV_FRAMES TX config %d",
			 wfa_param.value);
		ret_val = ucfg_send_wfatest_cmd(adapter->vdev, &wfa_param);
	}

	if (update_sme_cfg)
		sme_update_config(mac_handle, sme_config);

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
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_set_wifi_test_config(wiphy, wdev,
							 data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}

const struct nla_policy qca_wlan_vendor_wifi_logger_start_policy[
		QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_START_MAX + 1] = {
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
	struct sir_wifi_start_log start_log = { 0 };
	mac_handle_t mac_handle;

	hdd_enter_dev(wdev->netdev);

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

	start_log.user_triggered = 1;

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

	if (start_log.ring_id == RING_ID_PER_PACKET_STATS) {
		if (hdd_ctx->is_pktlog_enabled &&
		    (start_log.verbose_level == WLAN_LOG_LEVEL_ACTIVE))
			return 0;

		if ((!hdd_ctx->is_pktlog_enabled) &&
		    (start_log.verbose_level != WLAN_LOG_LEVEL_ACTIVE))
			return 0;
	}

	mac_handle = hdd_ctx->mac_handle;
	status = sme_wifi_start_logger(mac_handle, start_log);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("sme_wifi_start_logger failed(err=%d)",
				status);
		return -EINVAL;
	}

	if (start_log.ring_id == RING_ID_PER_PACKET_STATS) {
		if (start_log.verbose_level == WLAN_LOG_LEVEL_ACTIVE)
			hdd_ctx->is_pktlog_enabled = true;
		else
			hdd_ctx->is_pktlog_enabled = false;
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
	struct osif_psoc_sync *psoc_sync;
	int errno;

	errno = osif_psoc_sync_op_start(wiphy_dev(wiphy), &psoc_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_wifi_logger_start(wiphy, wdev,
						      data, data_len);

	osif_psoc_sync_op_stop(psoc_sync);

	return errno;
}

const struct nla_policy qca_wlan_vendor_wifi_logger_get_ring_data_policy[
		QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_GET_RING_DATA_MAX + 1] = {
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

	hdd_enter();

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
		hdd_debug("Bug report triggered by framework");

		status = cds_flush_logs(WLAN_LOG_TYPE_NON_FATAL,
				WLAN_LOG_INDICATOR_FRAMEWORK,
				WLAN_LOG_REASON_CODE_UNUSED,
				false, false);
		if (QDF_STATUS_SUCCESS != status) {
			hdd_err("Failed to trigger bug report");
			return -EINVAL;
		}
		status = wlan_logging_wait_for_flush_log_completion();
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			hdd_err("wait for flush log timed out");
			return qdf_status_to_os_return(status);
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
	struct osif_psoc_sync *psoc_sync;
	int errno;

	errno = osif_psoc_sync_op_start(wiphy_dev(wiphy), &psoc_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_wifi_logger_get_ring_data(wiphy, wdev,
							      data, data_len);

	osif_psoc_sync_op_stop(psoc_sync);

	return errno;
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
	return -ENOBUFS;
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
#define PARAM_PROTO_TYPE \
		QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_ETHER_PROTO_TYPE

const struct nla_policy offloaded_packet_policy[
			QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_SENDING_CONTROL] = {
			.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_REQUEST_ID] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_IP_PACKET_DATA] = {
			.type = NLA_BINARY},
	[QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_SRC_MAC_ADDR] = {
			.type = NLA_BINARY,
			.len = QDF_MAC_ADDR_SIZE },
	[QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_DST_MAC_ADDR] = {
			.type = NLA_BINARY,
			.len = QDF_MAC_ADDR_SIZE },
	[QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_PERIOD] = {
			.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_ETHER_PROTO_TYPE] = {
			.type = NLA_U16},
};

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
	uint32_t request_id, len;
	int32_t ret;
	uint8_t pattern_id = 0;
	struct qdf_mac_addr dst_addr;
	uint16_t eth_type = htons(ETH_P_IP);
	mac_handle_t mac_handle;

	if (!hdd_cm_is_vdev_associated(adapter)) {
		hdd_err("Not in Connected state!");
		return -ENOTSUPP;
	}

	add_req = qdf_mem_malloc(sizeof(*add_req));
	if (!add_req)
		return -ENOMEM;

	/* Parse and fetch request Id */
	if (!tb[PARAM_REQUEST_ID]) {
		hdd_err("attr request id failed");
		ret = -EINVAL;
		goto fail;
	}

	request_id = nla_get_u32(tb[PARAM_REQUEST_ID]);
	if (request_id == MAX_REQUEST_ID) {
		hdd_err("request_id cannot be MAX");
		ret = -EINVAL;
		goto fail;
	}
	hdd_debug("Request Id: %u", request_id);

	if (!tb[PARAM_PERIOD]) {
		hdd_err("attr period failed");
		ret = -EINVAL;
		goto fail;
	}

	add_req->usPtrnIntervalMs = nla_get_u32(tb[PARAM_PERIOD]);
	hdd_debug("Period: %u ms", add_req->usPtrnIntervalMs);
	if (add_req->usPtrnIntervalMs == 0) {
		hdd_err("Invalid interval zero, return failure");
		ret = -EINVAL;
		goto fail;
	}

	if (!tb[PARAM_SRC_MAC_ADDR]) {
		hdd_err("attr source mac address failed");
		ret = -EINVAL;
		goto fail;
	}
	nla_memcpy(add_req->mac_address.bytes, tb[PARAM_SRC_MAC_ADDR],
			QDF_MAC_ADDR_SIZE);
	hdd_debug("input src mac address: "QDF_MAC_ADDR_FMT,
			QDF_MAC_ADDR_REF(add_req->mac_address.bytes));

	if (!qdf_is_macaddr_equal(&add_req->mac_address,
				  &adapter->mac_addr)) {
		hdd_err("input src mac address and connected ap bssid are different");
		ret = -EINVAL;
		goto fail;
	}

	if (!tb[PARAM_DST_MAC_ADDR]) {
		hdd_err("attr dst mac address failed");
		ret = -EINVAL;
		goto fail;
	}
	nla_memcpy(dst_addr.bytes, tb[PARAM_DST_MAC_ADDR], QDF_MAC_ADDR_SIZE);
	hdd_debug("input dst mac address: "QDF_MAC_ADDR_FMT,
			QDF_MAC_ADDR_REF(dst_addr.bytes));

	if (!tb[PARAM_IP_PACKET]) {
		hdd_err("attr ip packet failed");
		ret = -EINVAL;
		goto fail;
	}
	add_req->ucPtrnSize = nla_len(tb[PARAM_IP_PACKET]);
	hdd_debug("IP packet len: %u", add_req->ucPtrnSize);

	if (add_req->ucPtrnSize < 0 ||
		add_req->ucPtrnSize > (PERIODIC_TX_PTRN_MAX_SIZE -
					ETH_HLEN)) {
		hdd_err("Invalid IP packet len: %d",
				add_req->ucPtrnSize);
		ret = -EINVAL;
		goto fail;
	}

	if (!tb[PARAM_PROTO_TYPE])
		eth_type = htons(ETH_P_IP);
	else
		eth_type = htons(nla_get_u16(tb[PARAM_PROTO_TYPE]));

	hdd_debug("packet proto type: %u", eth_type);

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

	mac_handle = hdd_ctx->mac_handle;
	status = sme_add_periodic_tx_ptrn(mac_handle, add_req);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("sme_add_periodic_tx_ptrn failed (err=%d)", status);
		ret = qdf_status_to_os_return(status);
		goto fail;
	}

	hdd_exit();

fail:
	qdf_mem_free(add_req);
	return ret;
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
	mac_handle_t mac_handle;

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
	if (!del_req)
		return -ENOMEM;

	qdf_copy_macaddr(&del_req->mac_address, &adapter->mac_addr);
	hdd_debug(QDF_MAC_ADDR_FMT,
		  QDF_MAC_ADDR_REF(del_req->mac_address.bytes));
	del_req->ucPtrnId = pattern_id;
	hdd_debug("Request Id: %u Pattern id: %d",
			 request_id, del_req->ucPtrnId);

	mac_handle = hdd_ctx->mac_handle;
	status = sme_del_periodic_tx_ptrn(mac_handle, del_req);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("sme_del_periodic_tx_ptrn failed (err=%d)", status);
		goto fail;
	}

	hdd_exit();
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

	hdd_enter_dev(dev);

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

	if (wlan_cfg80211_nla_parse(tb, PARAM_MAX, data, data_len,
				    offloaded_packet_policy)) {
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
#undef PARAM_PROTO_TYPE

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
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_offloaded_packets(wiphy, wdev,
						      data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}
#endif

#ifdef WLAN_NS_OFFLOAD
const struct nla_policy ns_offload_set_policy[
			QCA_WLAN_VENDOR_ATTR_ND_OFFLOAD_MAX + 1] = {
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

	hdd_enter_dev(wdev->netdev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	status = wlan_hdd_validate_context(hdd_ctx);
	if (0 != status)
		return status;

	if (!ucfg_pmo_is_ns_offloaded(hdd_ctx->psoc)) {
		hdd_err("ND Offload not supported");
		return -EINVAL;
	}

	if (!ucfg_pmo_is_active_mode_offloaded(hdd_ctx->psoc)) {
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
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_set_ns_offload(wiphy, wdev, data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}
#endif /* WLAN_NS_OFFLOAD */

/**
 * struct weighed_pcl: Preferred channel info
 * @freq: Channel frequency
 * @weight: Weightage of the channel
 * @flag: Validity of the channel in p2p negotiation
 */
struct weighed_pcl {
		u32 freq;
		u32 weight;
		u32 flag;
};

const struct nla_policy get_preferred_freq_list_policy[
		QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST_IFACE_TYPE] = {
		.type = NLA_U32},
};

static uint32_t wlan_hdd_populate_weigh_pcl(
				struct wlan_objmgr_psoc *psoc,
				struct policy_mgr_pcl_chan_weights *
				chan_weights,
				struct weighed_pcl *w_pcl,
				enum policy_mgr_con_mode intf_mode)
{
	u32 i, j, valid_weight;
	u32 chan_idx = 0;
	u32 pcl_len = chan_weights->pcl_len;
	u32 conn_count = policy_mgr_get_connection_count(psoc);

	/* convert channel number to frequency */
	for (i = 0; i < chan_weights->pcl_len; i++) {
		w_pcl[i].freq = chan_weights->pcl_list[i];
		w_pcl[i].weight = chan_weights->weight_list[i];

		if (intf_mode == PM_SAP_MODE || intf_mode == PM_P2P_GO_MODE)
			w_pcl[i].flag = PCL_CHANNEL_SUPPORT_GO;
		else
			w_pcl[i].flag = PCL_CHANNEL_SUPPORT_CLI;
	}
	chan_idx = pcl_len;
	if (!conn_count || policy_mgr_is_hw_dbs_capable(psoc) ||
	    policy_mgr_is_interband_mcc_supported(psoc)) {
		if (pcl_len && chan_weights->weight_list[pcl_len - 1] >
		    PCL_GROUPS_WEIGHT_DIFFERENCE)
		/*
		 * Set non-pcl channels weight 20 point less than the
		 * last PCL entry
		 */
			valid_weight = chan_weights->weight_list[pcl_len - 1] -
					PCL_GROUPS_WEIGHT_DIFFERENCE;
		else
			valid_weight = 1;

		/* Include rest of the valid channels */
		for (i = 0; i < chan_weights->saved_num_chan; i++) {
			for (j = 0; j < chan_weights->pcl_len; j++) {
				if (chan_weights->saved_chan_list[i] ==
					chan_weights->pcl_list[j])
					break;
			}
			if (j == chan_weights->pcl_len) {
				w_pcl[chan_idx].freq =
					chan_weights->saved_chan_list[i];

				if (!chan_weights->weighed_valid_list[i]) {
					w_pcl[chan_idx].flag =
						PCL_CHANNEL_EXCLUDE_IN_GO_NEG;
					w_pcl[chan_idx].weight = 0;
				} else {
					if (intf_mode == PM_SAP_MODE ||
					    intf_mode == PM_P2P_GO_MODE)
						w_pcl[chan_idx].flag =
						      PCL_CHANNEL_SUPPORT_GO;
					else
						w_pcl[chan_idx].flag =
						      PCL_CHANNEL_SUPPORT_CLI;
					w_pcl[chan_idx].weight = valid_weight;
				}
				chan_idx++;
			}
		}
	}
	return chan_idx;
}

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
	uint32_t pcl_len = 0;
	uint32_t pcl_len_legacy = 0;
	uint32_t freq_list[NUM_CHANNELS];
	uint32_t freq_list_legacy[NUM_CHANNELS];
	enum policy_mgr_con_mode intf_mode;
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST_MAX + 1];
	struct sk_buff *reply_skb;
	struct weighed_pcl *w_pcl;
	struct nlattr *nla_attr, *channel;
	struct policy_mgr_pcl_chan_weights *chan_weights;
	struct net_device *ndev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(ndev);

	hdd_enter_dev(wdev->netdev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

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

	chan_weights =
		qdf_mem_malloc(sizeof(struct policy_mgr_pcl_chan_weights));
	if (!chan_weights)
		return -ENOMEM;

	status = policy_mgr_get_pcl(
			hdd_ctx->psoc, intf_mode, chan_weights->pcl_list,
			&chan_weights->pcl_len, chan_weights->weight_list,
			QDF_ARRAY_SIZE(chan_weights->weight_list));
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("Get pcl failed");
		qdf_mem_free(chan_weights);
		return -EINVAL;
	}
	/*
	 * save the pcl in freq_list_legacy to be sent up with
	 * QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST.
	 * freq_list will carry the extended pcl in
	 * QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST_WEIGHED_PCL.
	 */
	pcl_len_legacy = chan_weights->pcl_len;
	for (i = 0; i < pcl_len_legacy; i++)
		freq_list_legacy[i] = chan_weights->pcl_list[i];
	chan_weights->saved_num_chan = NUM_CHANNELS;
	sme_get_valid_channels(chan_weights->saved_chan_list,
			       &chan_weights->saved_num_chan);
	policy_mgr_get_valid_chan_weights(hdd_ctx->psoc, chan_weights,
					  intf_mode,
					  adapter->vdev);
	w_pcl = qdf_mem_malloc(sizeof(struct weighed_pcl) * NUM_CHANNELS);
	if (!w_pcl) {
		qdf_mem_free(chan_weights);
		return -ENOMEM;
	}
	pcl_len = wlan_hdd_populate_weigh_pcl(hdd_ctx->psoc, chan_weights,
					      w_pcl, intf_mode);
	qdf_mem_free(chan_weights);

	for (i = 0; i < pcl_len; i++)
		freq_list[i] = w_pcl[i].freq;

	/* send the freq_list back to supplicant */
	reply_skb = cfg80211_vendor_cmd_alloc_reply_skb(
			wiphy,
			(sizeof(u32) + NLA_HDRLEN) +
			(sizeof(u32) * pcl_len_legacy + NLA_HDRLEN) +
			NLA_HDRLEN +
			(NLA_HDRLEN * 4 + sizeof(u32) * 3) * pcl_len +
			NLMSG_HDRLEN);

	if (!reply_skb) {
		hdd_err("Allocate reply_skb failed");
		qdf_mem_free(w_pcl);
		return -EINVAL;
	}

	if (nla_put_u32(reply_skb,
		QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST_IFACE_TYPE,
			intf_mode) ||
	    nla_put(reply_skb,
		    QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST,
		    sizeof(uint32_t) * pcl_len_legacy,
		    freq_list_legacy)) {
		hdd_err("nla put fail");
		kfree_skb(reply_skb);
		qdf_mem_free(w_pcl);
		return -EINVAL;
	}

	i = QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST_WEIGHED_PCL;
	nla_attr = nla_nest_start(reply_skb, i);

	if (!nla_attr) {
		hdd_err("nla nest start fail");
		kfree_skb(reply_skb);
		qdf_mem_free(w_pcl);
		return -EINVAL;
	}

	for (i = 0; i < pcl_len; i++) {
		channel = nla_nest_start(reply_skb, i);
		if (!channel) {
			hdd_err("updating pcl list failed");
			kfree_skb(reply_skb);
			qdf_mem_free(w_pcl);
			return -EINVAL;
		}
		if (nla_put_u32(reply_skb, QCA_WLAN_VENDOR_ATTR_PCL_FREQ,
				w_pcl[i].freq) ||
		    nla_put_u32(reply_skb, QCA_WLAN_VENDOR_ATTR_PCL_WEIGHT,
				w_pcl[i].weight) ||
		    nla_put_u32(reply_skb, QCA_WLAN_VENDOR_ATTR_PCL_FLAG,
				w_pcl[i].flag)) {
			hdd_err("nla put fail");
			kfree_skb(reply_skb);
			qdf_mem_free(w_pcl);
			return -EINVAL;
		}
		nla_nest_end(reply_skb, channel);
	}
	nla_nest_end(reply_skb, nla_attr);
	qdf_mem_free(w_pcl);

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
	struct osif_psoc_sync *psoc_sync;
	int errno;

	errno = osif_psoc_sync_op_start(wiphy_dev(wiphy), &psoc_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_get_preferred_freq_list(wiphy, wdev,
							    data, data_len);

	osif_psoc_sync_op_stop(psoc_sync);

	return errno;
}

const struct nla_policy set_probable_oper_channel_policy[
		QCA_WLAN_VENDOR_ATTR_PROBABLE_OPER_CHANNEL_MAX + 1] = {
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
	uint32_t ch_freq, conc_ext_flags;

	hdd_enter_dev(ndev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

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

	ch_freq = nla_get_u32(tb[
			      QCA_WLAN_VENDOR_ATTR_PROBABLE_OPER_CHANNEL_FREQ]);
	conc_ext_flags = policy_mgr_get_conc_ext_flags(adapter->vdev, false);

	/* check pcl table */
	if (!policy_mgr_allow_concurrency(hdd_ctx->psoc, intf_mode,
					  ch_freq, HW_MODE_20_MHZ,
					  conc_ext_flags)) {
		hdd_err("Set channel hint failed due to concurrency check");
		return -EINVAL;
	}

	if (0 != wlan_hdd_check_remain_on_channel(adapter))
		hdd_warn("Remain On Channel Pending");

	if (wlan_hdd_change_hw_mode_for_given_chnl(adapter, ch_freq,
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
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_set_probable_oper_channel(wiphy, wdev,
							      data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}

static const struct
nla_policy
wlan_hdd_get_link_properties_policy[QCA_WLAN_VENDOR_ATTR_MAX + 1] = {
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
	struct hdd_station_info *sta_info;
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_MAX+1];
	uint8_t peer_mac[QDF_MAC_ADDR_SIZE];
	struct sk_buff *reply_skb;
	uint32_t rate_flags = 0;
	uint8_t nss;
	uint8_t final_rate_flags = 0;
	uint32_t freq;

	hdd_enter_dev(dev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	if (0 != wlan_hdd_validate_context(hdd_ctx))
		return -EINVAL;

	if (wlan_cfg80211_nla_parse(tb, QCA_WLAN_VENDOR_ATTR_MAX, data,
				    data_len,
				    wlan_hdd_get_link_properties_policy)) {
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
	hdd_debug("peerMac="QDF_MAC_ADDR_FMT" for device_mode:%d",
		  QDF_MAC_ADDR_REF(peer_mac), adapter->device_mode);

	if (adapter->device_mode == QDF_STA_MODE ||
	    adapter->device_mode == QDF_P2P_CLIENT_MODE) {
		hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
		if (!hdd_cm_is_vdev_associated(adapter) ||
		    qdf_mem_cmp(hdd_sta_ctx->conn_info.bssid.bytes,
			peer_mac, QDF_MAC_ADDR_SIZE)) {
			hdd_err("Not Associated to mac "QDF_MAC_ADDR_FMT,
			       QDF_MAC_ADDR_REF(peer_mac));
			return -EINVAL;
		}

		nss  = hdd_sta_ctx->conn_info.nss;
		freq = hdd_sta_ctx->conn_info.chan_freq;
		rate_flags = hdd_sta_ctx->conn_info.rate_flags;
	} else if (adapter->device_mode == QDF_P2P_GO_MODE ||
		   adapter->device_mode == QDF_SAP_MODE) {

		if (QDF_IS_ADDR_BROADCAST(peer_mac)) {
			hdd_err("Ignore bcast/self sta");
			return -EINVAL;
		}

		sta_info = hdd_get_sta_info_by_mac(
					&adapter->sta_info_list, peer_mac,
					STA_INFO_CFG80211_GET_LINK_PROPERTIES);

		if (!sta_info) {
			hdd_err("No active peer with mac = " QDF_MAC_ADDR_FMT,
				QDF_MAC_ADDR_REF(peer_mac));
			return -EINVAL;
		}

		nss = sta_info->nss;
		freq = (WLAN_HDD_GET_AP_CTX_PTR(adapter))->operating_chan_freq;
		rate_flags = sta_info->rate_flags;
		hdd_put_sta_info_ref(&adapter->sta_info_list, &sta_info, true,
				     STA_INFO_CFG80211_GET_LINK_PROPERTIES);
	} else {
		hdd_err("Not Associated! with mac "QDF_MAC_ADDR_FMT,
		       QDF_MAC_ADDR_REF(peer_mac));
		return -EINVAL;
	}

	if (!(rate_flags & TX_RATE_LEGACY)) {
		if (rate_flags & TX_RATE_VHT80) {
			final_rate_flags |= RATE_INFO_FLAGS_VHT_MCS;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)) && !defined(WITH_BACKPORTS)
			final_rate_flags |= RATE_INFO_FLAGS_80_MHZ_WIDTH;
#endif
		} else if (rate_flags & TX_RATE_VHT40) {
			final_rate_flags |= RATE_INFO_FLAGS_VHT_MCS;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)) && !defined(WITH_BACKPORTS)
			final_rate_flags |= RATE_INFO_FLAGS_40_MHZ_WIDTH;
#endif
		} else if (rate_flags & TX_RATE_VHT20) {
			final_rate_flags |= RATE_INFO_FLAGS_VHT_MCS;
		} else if (rate_flags &
				(TX_RATE_HT20 | TX_RATE_HT40)) {
			final_rate_flags |= RATE_INFO_FLAGS_MCS;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)) && !defined(WITH_BACKPORTS)
			if (rate_flags & TX_RATE_HT40)
				final_rate_flags |=
					RATE_INFO_FLAGS_40_MHZ_WIDTH;
#endif
		}

		if (rate_flags & TX_RATE_SGI) {
			if (!(final_rate_flags & RATE_INFO_FLAGS_VHT_MCS))
				final_rate_flags |= RATE_INFO_FLAGS_MCS;
			final_rate_flags |= RATE_INFO_FLAGS_SHORT_GI;
		}
	}

	reply_skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy,
			sizeof(u8) + sizeof(u8) + sizeof(u32) + NLMSG_HDRLEN);

	if (!reply_skb) {
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
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_get_link_properties(wiphy, wdev,
							data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}

const struct nla_policy
wlan_hdd_sap_config_policy[QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_CHANNEL] = {.type = NLA_U8},
	[QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_FREQUENCY] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_SAP_MANDATORY_FREQUENCY_LIST] = {
							.type = NLA_BINARY},
};

const struct nla_policy
wlan_hdd_set_acs_dfs_config_policy[QCA_WLAN_VENDOR_ATTR_ACS_DFS_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_ACS_DFS_MODE] = {.type = NLA_U8},
	[QCA_WLAN_VENDOR_ATTR_ACS_CHANNEL_HINT] = {.type = NLA_U8},
	[QCA_WLAN_VENDOR_ATTR_ACS_FREQUENCY_HINT] = {.type = NLA_U32},
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
	uint32_t freq_hint = 0;

	hdd_enter_dev(wdev->netdev);

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
	if (tb[QCA_WLAN_VENDOR_ATTR_ACS_FREQUENCY_HINT]) {
		freq_hint = nla_get_u32(
				tb[QCA_WLAN_VENDOR_ATTR_ACS_FREQUENCY_HINT]);
	} else if (tb[QCA_WLAN_VENDOR_ATTR_ACS_CHANNEL_HINT]) {
		uint32_t channel_hint = nla_get_u8(
				tb[QCA_WLAN_VENDOR_ATTR_ACS_CHANNEL_HINT]);

		freq_hint = wlan_reg_legacy_chan_to_freq(hdd_ctx->pdev,
							 channel_hint);
	}

	if (freq_hint && !WLAN_REG_IS_24GHZ_CH_FREQ(freq_hint) &&
	    !WLAN_REG_IS_5GHZ_CH_FREQ(freq_hint) &&
	    !WLAN_REG_IS_6GHZ_CHAN_FREQ(freq_hint)) {
		hdd_err("acs channel frequency is not valid");
		return -EINVAL;
	}

	acs_policy->acs_chan_freq = freq_hint;

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
	struct osif_psoc_sync *psoc_sync;
	int errno;

	errno = osif_psoc_sync_op_start(wiphy_dev(wiphy), &psoc_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_acs_dfs_mode(wiphy, wdev, data, data_len);

	osif_psoc_sync_op_stop(psoc_sync);

	return errno;
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
		return STA_ROAM_POLICY_DFS_ENABLED;
	case DFS_MODE_DISABLE:
		return STA_ROAM_POLICY_DFS_DISABLED;
	case DFS_MODE_DEPRIORITIZE:
		return STA_ROAM_POLICY_DFS_DEPRIORITIZE;
	default:
		hdd_err("STA Roam policy dfs mode is NONE");
		return  STA_ROAM_POLICY_NONE;
	}
}

/*
 * hdd_get_sap_operating_band_by_adapter:  Get current adapter operating band
 * for sap.
 * @adapter: Pointer to adapter
 *
 * Return : Corresponding band for SAP operating channel
 */
uint8_t hdd_get_sap_operating_band_by_adapter(struct hdd_adapter *adapter)
{
	uint32_t operating_chan_freq;
	uint8_t sap_operating_band = 0;

	if (adapter->device_mode != QDF_SAP_MODE &&
	    adapter->device_mode != QDF_P2P_GO_MODE)
		return BAND_UNKNOWN;

	operating_chan_freq = adapter->session.ap.operating_chan_freq;
	if (WLAN_REG_IS_24GHZ_CH_FREQ(operating_chan_freq))
		sap_operating_band = BAND_2G;
	else if (WLAN_REG_IS_5GHZ_CH_FREQ(operating_chan_freq) ||
		 WLAN_REG_IS_6GHZ_CHAN_FREQ(operating_chan_freq))
		sap_operating_band = BAND_5G;
	else
		sap_operating_band = BAND_UNKNOWN;

	return sap_operating_band;
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
	struct hdd_adapter *adapter, *next_adapter = NULL;
	uint8_t operating_band = 0;
	wlan_net_dev_ref_dbgid dbgid = NET_DEV_HOLD_GET_SAP_OPERATING_BAND;

	hdd_for_each_adapter_dev_held_safe(hdd_ctx, adapter, next_adapter,
					   dbgid) {
		if (adapter->device_mode != QDF_SAP_MODE) {
			hdd_adapter_dev_put_debug(adapter, dbgid);
			continue;
		}

		operating_band = hdd_get_sap_operating_band_by_adapter(adapter);

		hdd_adapter_dev_put_debug(adapter, dbgid);
	}

	return operating_band;
}

const struct nla_policy
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
	mac_handle_t mac_handle;

	hdd_enter_dev(dev);

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
	mac_handle = hdd_ctx->mac_handle;
	status = sme_update_sta_roam_policy(mac_handle, sta_roam_dfs_mode,
					    skip_unsafe_channels,
					    adapter->vdev_id,
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
static int
wlan_hdd_cfg80211_sta_roam_policy(struct wiphy *wiphy,
				  struct wireless_dev *wdev, const void *data,
				  int data_len)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_sta_roam_policy(wiphy, wdev,
						    data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}

const struct nla_policy
wlan_hdd_set_dual_sta_policy[
QCA_WLAN_VENDOR_ATTR_CONCURRENT_STA_POLICY_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_CONCURRENT_STA_POLICY_CONFIG] = {.type = NLA_U8 },
};

/**
 * __wlan_hdd_cfg80211_dual_sta_policy() - Wrapper to configure the concurrent
 * session policies
 * @wiphy:    wiphy structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of @data
 *
 * Configure the concurrent session policies when multiple STA ifaces are
 * (getting) active.
 * Return: 0 on success; errno on failure
 */
static int __wlan_hdd_cfg80211_dual_sta_policy(struct wiphy *wiphy,
					       struct wireless_dev *wdev,
					       const void *data, int data_len)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct nlattr *tb[
		QCA_WLAN_VENDOR_ATTR_CONCURRENT_STA_POLICY_MAX + 1];
	QDF_STATUS status;
	uint8_t dual_sta_config =
		QCA_WLAN_CONCURRENT_STA_POLICY_UNBIASED;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	if (wlan_hdd_validate_context(hdd_ctx)) {
		hdd_err("Invalid hdd context");
		return -EINVAL;
	}

	if (wlan_cfg80211_nla_parse(tb,
			       QCA_WLAN_VENDOR_ATTR_CONCURRENT_STA_POLICY_MAX,
			       data, data_len,
			       wlan_hdd_set_dual_sta_policy)) {
		hdd_err("nla_parse failed");
		return -EINVAL;
	}

	if (!tb[QCA_WLAN_VENDOR_ATTR_CONCURRENT_STA_POLICY_CONFIG]) {
		hdd_err("sta policy config attribute not present");
		return -EINVAL;
	}

	dual_sta_config = nla_get_u8(
			tb[QCA_WLAN_VENDOR_ATTR_CONCURRENT_STA_POLICY_CONFIG]);
	hdd_debug("Concurrent STA policy : %d", dual_sta_config);

	if (dual_sta_config > QCA_WLAN_CONCURRENT_STA_POLICY_UNBIASED)
		return -EINVAL;

	status = ucfg_mlme_set_dual_sta_policy(hdd_ctx->psoc, dual_sta_config);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("failed to set MLME dual sta config");
		return -EINVAL;
	}

	/* After SSR, the dual sta configuration is lost. As SSR is hidden from
	 * userland, this command will not come from userspace after a SSR. To
	 * restore this configuration, save this in hdd context and restore
	 * after re-init.
	 */
	hdd_ctx->dual_sta_policy.dual_sta_policy = dual_sta_config;

	return 0;
}

/**
 * wlan_hdd_cfg80211_dual_sta_policy() - Wrapper to configure the concurrent
 * session policies
 * @wiphy:    wiphy structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of @data
 *
 * Configure the concurrent session policies when multiple STA ifaces are
 * (getting) active.
 * Return: 0 on success; errno on failure
 */
static int wlan_hdd_cfg80211_dual_sta_policy(struct wiphy *wiphy,
					     struct wireless_dev *wdev,
					     const void *data, int data_len)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_dual_sta_policy(wiphy, wdev, data,
						    data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}

#ifdef FEATURE_WLAN_CH_AVOID
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
	struct ch_avoid_ind_type *channel_list;
	struct ch_avoid_ind_type avoid_freq_list;
	enum QDF_GLOBAL_MODE curr_mode;
	uint8_t num_args = 0;

	hdd_enter_dev(wdev->netdev);

	if (!qdf_ctx)
		return -EINVAL;

	curr_mode = hdd_get_conparam();
	if (QDF_GLOBAL_FTM_MODE == curr_mode ||
	    QDF_GLOBAL_MONITOR_MODE == curr_mode) {
		hdd_err("Command not allowed in FTM/MONITOR mode");
		return -EINVAL;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;
	qdf_mem_zero(&avoid_freq_list, sizeof(struct ch_avoid_ind_type));

	if (!data && data_len == 0) {
		hdd_debug("Clear avoid frequency list");
		goto process_unsafe_channel;
	}
	if (!data || data_len < (sizeof(channel_list->ch_avoid_range_cnt) +
				 sizeof(struct ch_avoid_freq_type))) {
		hdd_err("Avoid frequency channel list empty");
		return -EINVAL;
	}
	num_args = (data_len - sizeof(channel_list->ch_avoid_range_cnt)) /
		   sizeof(channel_list->avoid_freq_range[0].start_freq);


	if (num_args < 2 || num_args > CH_AVOID_MAX_RANGE * 2 ||
	    num_args % 2 != 0) {
		hdd_err("Invalid avoid frequency channel list");
		return -EINVAL;
	}

	channel_list = (struct ch_avoid_ind_type *)data;
	if (channel_list->ch_avoid_range_cnt == 0 ||
	    channel_list->ch_avoid_range_cnt > CH_AVOID_MAX_RANGE ||
	    2 * channel_list->ch_avoid_range_cnt != num_args) {
		hdd_err("Invalid frequency range count %d",
			channel_list->ch_avoid_range_cnt);
		return -EINVAL;
	}

	qdf_mem_copy(&avoid_freq_list, channel_list, data_len);

process_unsafe_channel:
	ucfg_reg_ch_avoid(hdd_ctx->psoc, &avoid_freq_list);

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
	struct osif_psoc_sync *psoc_sync;
	int errno;

	errno = osif_psoc_sync_op_start(wiphy_dev(wiphy), &psoc_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_avoid_freq(wiphy, wdev, data, data_len);

	osif_psoc_sync_op_stop(psoc_sync);

	return errno;
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
	struct hdd_ap_ctx *ap_ctx;
	int ret;
	uint32_t chan_freq = 0;
	bool chan_freq_present = false;
	QDF_STATUS status;

	hdd_enter();

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

	if (tb[QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_FREQUENCY]) {
		chan_freq = nla_get_u32(
				tb[QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_FREQUENCY]);
		chan_freq_present = true;
	} else if (tb[QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_CHANNEL]) {
		uint32_t config_channel =
			nla_get_u8(tb[QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_CHANNEL]);

		chan_freq = wlan_reg_legacy_chan_to_freq(hdd_ctx->pdev,
							 config_channel);
		chan_freq_present = true;
	}

	if (chan_freq_present) {
		if (!test_bit(SOFTAP_BSS_STARTED,
					&hostapd_adapter->event_flags)) {
			hdd_err("SAP is not started yet. Restart sap will be invalid");
			return -EINVAL;
		}

		if (!WLAN_REG_IS_24GHZ_CH_FREQ(chan_freq) &&
		    !WLAN_REG_IS_5GHZ_CH_FREQ(chan_freq) &&
		    !WLAN_REG_IS_6GHZ_CHAN_FREQ(chan_freq)) {
			hdd_err("Channel frequency %u is invalid to restart SAP",
				chan_freq);
			return -ENOTSUPP;
		}

		ap_ctx = WLAN_HDD_GET_AP_CTX_PTR(hostapd_adapter);
		ap_ctx->sap_config.chan_freq = chan_freq;
		ap_ctx->sap_config.ch_params.ch_width =
					ap_ctx->sap_config.ch_width_orig;
		ap_ctx->bss_stop_reason = BSS_STOP_DUE_TO_VENDOR_CONFIG_CHAN;

		wlan_reg_set_channel_params_for_freq(
				hdd_ctx->pdev, chan_freq,
				ap_ctx->sap_config.sec_ch_freq,
				&ap_ctx->sap_config.ch_params);

		hdd_restart_sap(hostapd_adapter);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_SAP_MANDATORY_FREQUENCY_LIST]) {
		uint32_t freq_len, i;
		uint32_t *freq;

		hdd_debug("setting mandatory freq/chan list");

		freq_len = nla_len(
		    tb[QCA_WLAN_VENDOR_ATTR_SAP_MANDATORY_FREQUENCY_LIST])/
		    sizeof(uint32_t);

		if (freq_len > NUM_CHANNELS) {
			hdd_err("insufficient space to hold channels");
			return -ENOMEM;
		}

		freq = nla_data(
		    tb[QCA_WLAN_VENDOR_ATTR_SAP_MANDATORY_FREQUENCY_LIST]);

		hdd_debug("freq_len=%d", freq_len);

		for (i = 0; i < freq_len; i++) {
			hdd_debug("freq[%d]=%d", i, freq[i]);
		}

		status = policy_mgr_set_sap_mandatory_channels(
			hdd_ctx->psoc, freq, freq_len);
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
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_sap_configuration_set(wiphy, wdev,
							  data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}

/**
 * wlan_hdd_process_wake_lock_stats() - wrapper function to absract cp_stats
 * or legacy get_wake_lock_stats API.
 * @hdd_ctx: pointer to hdd_ctx
 *
 * Return: 0 on success; error number otherwise.
 */
static int wlan_hdd_process_wake_lock_stats(struct hdd_context *hdd_ctx)
{
	return wlan_cfg80211_mc_cp_stats_get_wakelock_stats(hdd_ctx->psoc,
							    hdd_ctx->wiphy);
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
	int ret;
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);

	hdd_enter();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return -EINVAL;

	ret = wlan_hdd_process_wake_lock_stats(hdd_ctx);
	hdd_exit();
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
	struct osif_psoc_sync *psoc_sync;
	int errno;

	errno = osif_psoc_sync_op_start(wiphy_dev(wiphy), &psoc_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_get_wakelock_stats(wiphy, wdev,
						       data, data_len);

	osif_psoc_sync_op_stop(psoc_sync);

	return errno;
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

	hdd_enter();

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

	hdd_exit();

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
	struct osif_psoc_sync *psoc_sync;
	int errno;

	errno = osif_psoc_sync_op_start(wiphy_dev(wiphy), &psoc_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_get_bus_size(wiphy, wdev, data, data_len);

	osif_psoc_sync_op_stop(psoc_sync);

	return errno;
}

const struct nla_policy setband_policy[QCA_WLAN_VENDOR_ATTR_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_SETBAND_VALUE] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_SETBAND_MASK] = {.type = NLA_U32},
};

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
	uint32_t reg_wifi_band_bitmap = 0, band_val, band_mask;

	hdd_enter();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	if (wlan_cfg80211_nla_parse(tb, QCA_WLAN_VENDOR_ATTR_MAX,
				    data, data_len, setband_policy)) {
		hdd_err("Invalid ATTR");
		return -EINVAL;
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_SETBAND_MASK]) {
		band_mask = nla_get_u32(tb[QCA_WLAN_VENDOR_ATTR_SETBAND_MASK]);
		reg_wifi_band_bitmap =
			wlan_vendor_bitmap_to_reg_wifi_band_bitmap(hdd_ctx->psoc,
								   band_mask);
		hdd_debug("[SET BAND] set band mask:%d", reg_wifi_band_bitmap);
	} else if (tb[QCA_WLAN_VENDOR_ATTR_SETBAND_VALUE]) {
		band_val = nla_get_u32(tb[QCA_WLAN_VENDOR_ATTR_SETBAND_VALUE]);
		reg_wifi_band_bitmap =
			hdd_reg_legacy_setband_to_reg_wifi_band_bitmap(
								      band_val);
	}

	if (!reg_wifi_band_bitmap) {
		hdd_err("attr SETBAND_VALUE failed");
		return -EINVAL;
	}

	ret = hdd_reg_set_band(dev, reg_wifi_band_bitmap);

	hdd_exit();
	return ret;
}

/**
 *wlan_hdd_validate_acs_channel() - validate channel frequency provided by ACS
 * @adapter: hdd adapter
 * @chan_freq: channel frequency in MHz
 *
 * return: QDF status based on success or failure
 */
static QDF_STATUS wlan_hdd_validate_acs_channel(struct hdd_adapter *adapter,
						uint32_t chan_freq, int chan_bw)
{
	if (QDF_STATUS_SUCCESS !=
	    wlan_hdd_validate_operation_channel(adapter, chan_freq))
		return QDF_STATUS_E_FAILURE;

	if ((wlansap_is_channel_in_nol_list(WLAN_HDD_GET_SAP_CTX_PTR(adapter),
				chan_freq,
				PHY_SINGLE_CHANNEL_CENTERED))) {
		hdd_info("channel %d is in nol", chan_freq);
		return -EINVAL;
	}

	if ((wlansap_is_channel_leaking_in_nol(
				WLAN_HDD_GET_SAP_CTX_PTR(adapter),
				chan_freq, chan_bw))) {
		hdd_info("channel freq %d is leaking in nol", chan_freq);
		return -EINVAL;
	}

	return 0;

}

static void hdd_update_acs_sap_config(struct hdd_context *hdd_ctx,
				     struct sap_config *sap_config,
				     struct hdd_vendor_chan_info *channel_list)
{
	uint8_t ch_width;
	QDF_STATUS status;
	uint32_t channel_bonding_mode;

	sap_config->chan_freq = channel_list->pri_chan_freq;

	sap_config->ch_params.center_freq_seg0 =
		wlan_reg_freq_to_chan(hdd_ctx->pdev,
				      channel_list->vht_seg0_center_chan_freq);
	sap_config->ch_params.center_freq_seg1 =
		wlan_reg_freq_to_chan(hdd_ctx->pdev,
				      channel_list->vht_seg1_center_chan_freq);

	sap_config->ch_params.sec_ch_offset =
		wlan_reg_freq_to_chan(hdd_ctx->pdev,
				      channel_list->ht_sec_chan_freq);

	sap_config->ch_params.ch_width = channel_list->chan_width;
	if (!WLAN_REG_IS_24GHZ_CH_FREQ(sap_config->chan_freq)) {
		status =
			ucfg_mlme_get_vht_channel_width(hdd_ctx->psoc,
							&ch_width);
		if (!QDF_IS_STATUS_SUCCESS(status))
			hdd_err("Failed to set channel_width");
		sap_config->ch_width_orig = ch_width;
	} else {
		ucfg_mlme_get_channel_bonding_24ghz(hdd_ctx->psoc,
						    &channel_bonding_mode);
		sap_config->ch_width_orig = channel_bonding_mode ?
			eHT_CHANNEL_WIDTH_40MHZ : eHT_CHANNEL_WIDTH_20MHZ;
	}
	sap_config->acs_cfg.pri_ch_freq = channel_list->pri_chan_freq;
	sap_config->acs_cfg.ch_width = channel_list->chan_width;
	sap_config->acs_cfg.vht_seg0_center_ch_freq =
			channel_list->vht_seg0_center_chan_freq;
	sap_config->acs_cfg.vht_seg1_center_ch_freq =
			channel_list->vht_seg1_center_chan_freq;
	sap_config->acs_cfg.ht_sec_ch_freq =
			channel_list->ht_sec_chan_freq;
}

static int hdd_update_acs_channel(struct hdd_adapter *adapter, uint8_t reason,
				  uint8_t channel_cnt,
				  struct hdd_vendor_chan_info *channel_list)
{
	struct sap_config *sap_config;
	struct hdd_ap_ctx *hdd_ap_ctx;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	mac_handle_t mac_handle;
	uint32_t ch;

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

	if (channel_list->pri_chan_freq == 0) {
		/* Check mode, set default channel */
		channel_list->pri_chan_freq = 2437;
		/*
		 * sap_select_default_oper_chan(mac_handle,
		 *      sap_config->acs_cfg.hw_mode);
		 */
	}

	mac_handle = hdd_ctx->mac_handle;
	switch (reason) {
	/* SAP init case */
	case QCA_WLAN_VENDOR_ACS_SELECT_REASON_INIT:
		hdd_update_acs_sap_config(hdd_ctx, sap_config, channel_list);
		/* Update Hostapd */
		wlan_hdd_cfg80211_acs_ch_select_evt(adapter);
		break;

	/* DFS detected on current channel */
	case QCA_WLAN_VENDOR_ACS_SELECT_REASON_DFS:
		ch = wlan_reg_freq_to_chan(hdd_ctx->pdev,
					   channel_list->pri_chan_freq);

		wlan_sap_update_next_channel(
				WLAN_HDD_GET_SAP_CTX_PTR(adapter), (uint8_t)ch,
				channel_list->chan_width);
		status = sme_update_new_channel_event(
					mac_handle,
					adapter->vdev_id);
		break;

	/* LTE coex event on current channel */
	case QCA_WLAN_VENDOR_ACS_SELECT_REASON_LTE_COEX:
		ch = wlan_reg_freq_to_chan(hdd_ctx->pdev,
					   channel_list->pri_chan_freq);
		sap_config->acs_cfg.pri_ch_freq = channel_list->pri_chan_freq;
		sap_config->acs_cfg.ch_width = channel_list->chan_width;
		hdd_ap_ctx->sap_config.ch_width_orig =
				channel_list->chan_width;
		wlan_hdd_set_sap_csa_reason(hdd_ctx->psoc, adapter->vdev_id,
					    CSA_REASON_LTE_COEX);
		hdd_switch_sap_channel(adapter, (uint8_t)ch, true);
		break;

	default:
		hdd_info("invalid reason for timer invoke");
	}
	hdd_exit();
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

#define SET_CHAN_FREQ_LIST QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_FREQUENCY_LIST
#define SET_CHAN_FREQUENCY_PRIMARY \
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_FREQUENCY_PRIMARY
#define SET_CHAN_FREQUENCY_SECONDARY \
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_FREQUENCY_SECONDARY
#define SET_CHAN_SEG0_CENTER_FREQUENCY \
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_FREQUENCY_CENTER_SEG0
#define SET_CHAN_SEG1_CENTER_FREQUENCY \
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_FREQUENCY_CENTER_SEG1

#define SET_CHAN_MAX QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_MAX
#define SET_EXT_ACS_BAND QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_BAND

static const struct nla_policy acs_chan_config_policy[SET_CHAN_MAX + 1] = {
	[SET_CHAN_REASON] = {.type = NLA_U8},
	[SET_CHAN_CHAN_LIST] = {.type = NLA_NESTED},
	[SET_CHAN_FREQ_LIST] = {.type = NLA_NESTED},
};

static const struct nla_policy acs_chan_list_policy[SET_CHAN_MAX + 1] = {
	[SET_CHAN_PRIMARY_CHANNEL] = {.type = NLA_U8},
	[SET_CHAN_SECONDARY_CHANNEL] = {.type = NLA_U8},
	[SET_CHAN_SEG0_CENTER_CHANNEL] = {.type = NLA_U8},
	[SET_CHAN_SEG1_CENTER_CHANNEL] = {.type = NLA_U8},
	[SET_CHAN_CHANNEL_WIDTH] = {.type = NLA_U8},
	[SET_EXT_ACS_BAND] = {.type = NLA_U8},

	[SET_CHAN_FREQUENCY_PRIMARY] = {.type = NLA_U32},
	[SET_CHAN_FREQUENCY_SECONDARY] = {.type = NLA_U32},
	[SET_CHAN_SEG0_CENTER_FREQUENCY] = {.type = NLA_U32},
	[SET_CHAN_SEG1_CENTER_FREQUENCY] = {.type = NLA_U32},
};

/**
 * hdd_extract_external_acs_frequencies() - API to parse and extract vendor acs
 * channel frequency (in MHz) configuration.
 * @hdd_ctx: pointer to hdd context
 * @list_ptr: pointer to hdd_vendor_chan_info
 * @channel_cnt: channel count
 * @data: data
 * @data_len: data len
 *
 * Return: 0 on success, negative errno on failure
 */
static int
hdd_extract_external_acs_frequencies(struct hdd_context *hdd_ctx,
				     struct hdd_vendor_chan_info **list_ptr,
				     uint8_t *channel_cnt,
				     const void *data, int data_len)
{
	int rem;
	uint32_t i = 0;
	struct nlattr *tb[SET_CHAN_MAX + 1];
	struct nlattr *tb2[SET_CHAN_MAX + 1];
	struct nlattr *curr_attr;
	struct hdd_vendor_chan_info *channel_list;

	if (wlan_cfg80211_nla_parse(tb, SET_CHAN_MAX, data, data_len,
				    acs_chan_config_policy)) {
		hdd_err("Invalid ATTR");
		return -EINVAL;
	}

	nla_for_each_nested(curr_attr, tb[SET_CHAN_FREQ_LIST], rem)
		i++;

	if (!i) {
		hdd_err_rl("Error: channel count is zero");
		return -EINVAL;
	}

	if (i > NUM_CHANNELS) {
		hdd_err_rl("Error: Exceeded max channels: %u", NUM_CHANNELS);
		return -ENOMEM;
	}

	channel_list = qdf_mem_malloc(sizeof(struct hdd_vendor_chan_info) * i);
	if (!channel_list)
		return -ENOMEM;

	*channel_cnt = (uint8_t)i;
	i = 0;
	nla_for_each_nested(curr_attr, tb[SET_CHAN_FREQ_LIST], rem) {
		if (wlan_cfg80211_nla_parse_nested(tb2, SET_CHAN_MAX,
						   curr_attr,
						   acs_chan_list_policy)) {
			hdd_err_rl("nla_parse failed");
			qdf_mem_free(channel_list);
			*channel_cnt = 0;
			return -EINVAL;
		}

		if (tb2[SET_EXT_ACS_BAND])
			channel_list[i].band =
				nla_get_u8(tb2[SET_EXT_ACS_BAND]);

		if (tb2[SET_CHAN_FREQUENCY_PRIMARY])
			channel_list[i].pri_chan_freq =
				nla_get_u32(tb2[SET_CHAN_FREQUENCY_PRIMARY]);

		if (tb2[SET_CHAN_FREQUENCY_SECONDARY])
			channel_list[i].ht_sec_chan_freq =
				nla_get_u32(tb2[SET_CHAN_FREQUENCY_SECONDARY]);

		if (tb2[SET_CHAN_SEG0_CENTER_FREQUENCY])
			channel_list[i].vht_seg0_center_chan_freq =
			 nla_get_u32(tb2[SET_CHAN_SEG0_CENTER_FREQUENCY]);

		if (tb2[SET_CHAN_SEG1_CENTER_FREQUENCY])
			channel_list[i].vht_seg1_center_chan_freq =
			 nla_get_u32(tb2[SET_CHAN_SEG1_CENTER_FREQUENCY]);

		if (tb2[SET_CHAN_CHANNEL_WIDTH])
			channel_list[i].chan_width =
				nla_get_u8(tb2[SET_CHAN_CHANNEL_WIDTH]);

		hdd_debug("index %d, pri_chan_freq %u, ht_sec_chan_freq %u seg0_freq %u seg1_freq %u width %u",
			  i, channel_list[i].pri_chan_freq,
			  channel_list[i].ht_sec_chan_freq,
			  channel_list[i].vht_seg0_center_chan_freq,
			  channel_list[i].vht_seg1_center_chan_freq,
			  channel_list[i].chan_width);
		i++;
	}
	*list_ptr = channel_list;

	return 0;
}

/**
 * hdd_extract_external_acs_channels() - API to parse and extract vendor acs
 * channel configuration.
 * @hdd_ctx: pointer to hdd context
 * @list_ptr: pointer to hdd_vendor_chan_info
 * @channel_cnt: channel count
 * @data: data
 * @data_len: data len
 *
 * Return: 0 on success, negative errno on failure
 */
static int
hdd_extract_external_acs_channels(struct hdd_context *hdd_ctx,
				  struct hdd_vendor_chan_info **list_ptr,
				  uint8_t *channel_cnt,
				  const void *data, int data_len)
{
	int rem;
	uint32_t i = 0;
	struct nlattr *tb[SET_CHAN_MAX + 1];
	struct nlattr *tb2[SET_CHAN_MAX + 1];
	struct nlattr *curr_attr;
	struct hdd_vendor_chan_info *channel_list;

	if (wlan_cfg80211_nla_parse(tb, SET_CHAN_MAX, data, data_len,
				    acs_chan_config_policy)) {
		hdd_err("Invalid ATTR");
		return -EINVAL;
	}

	nla_for_each_nested(curr_attr, tb[SET_CHAN_CHAN_LIST], rem)
		i++;

	if (!i) {
		hdd_err_rl("Error: channel count is zero");
		return -EINVAL;
	}

	if (i > NUM_CHANNELS) {
		hdd_err_rl("Error: Exceeded max channels: %u", NUM_CHANNELS);
		return -ENOMEM;
	}

	channel_list = qdf_mem_malloc(sizeof(struct hdd_vendor_chan_info) * i);
	if (!channel_list)
		return -ENOMEM;

	*channel_cnt = (uint8_t)i;
	i = 0;
	nla_for_each_nested(curr_attr, tb[SET_CHAN_CHAN_LIST], rem) {
		if (wlan_cfg80211_nla_parse_nested(tb2, SET_CHAN_MAX,
						   curr_attr,
						   acs_chan_list_policy)) {
			hdd_err("nla_parse failed");
			qdf_mem_free(channel_list);
			*channel_cnt = 0;
			return -EINVAL;
		}

		if (tb2[SET_EXT_ACS_BAND]) {
			channel_list[i].band =
				nla_get_u8(tb2[SET_EXT_ACS_BAND]);
		}

		if (tb2[SET_CHAN_PRIMARY_CHANNEL]) {
			uint32_t ch =
				nla_get_u8(tb2[SET_CHAN_PRIMARY_CHANNEL]);

			channel_list[i].pri_chan_freq =
				wlan_reg_legacy_chan_to_freq(hdd_ctx->pdev, ch);
		}

		if (tb2[SET_CHAN_SECONDARY_CHANNEL]) {
			uint32_t ch =
				nla_get_u8(tb2[SET_CHAN_SECONDARY_CHANNEL]);

			channel_list[i].ht_sec_chan_freq =
				wlan_reg_legacy_chan_to_freq(hdd_ctx->pdev, ch);
		}

		if (tb2[SET_CHAN_SEG0_CENTER_CHANNEL]) {
			uint32_t ch =
				nla_get_u8(tb2[SET_CHAN_SEG0_CENTER_CHANNEL]);

			channel_list[i].vht_seg0_center_chan_freq =
				wlan_reg_legacy_chan_to_freq(hdd_ctx->pdev, ch);
		}

		if (tb2[SET_CHAN_SEG1_CENTER_CHANNEL]) {
			uint32_t ch =
				nla_get_u8(tb2[SET_CHAN_SEG1_CENTER_CHANNEL]);

			channel_list[i].vht_seg1_center_chan_freq =
				wlan_reg_legacy_chan_to_freq(hdd_ctx->pdev, ch);
		}

		if (tb2[SET_CHAN_CHANNEL_WIDTH]) {
			channel_list[i].chan_width =
				nla_get_u8(tb2[SET_CHAN_CHANNEL_WIDTH]);
		}
		hdd_debug("index %d, pri_chan_freq %u, ht_sec_chan_freq %u seg0_freq %u seg1_freq %u width %u",
			  i, channel_list[i].pri_chan_freq,
			  channel_list[i].ht_sec_chan_freq,
			  channel_list[i].vht_seg0_center_chan_freq,
			  channel_list[i].vht_seg1_center_chan_freq,
			  channel_list[i].chan_width);
		i++;
	}
	*list_ptr = channel_list;

	return 0;
}

/**
 * hdd_parse_vendor_acs_chan_config() - API to parse vendor acs channel config
 * @hdd_ctx: pointer to hdd context
 * @channel_list: pointer to hdd_vendor_chan_info
 * @reason: channel change reason
 * @channel_cnt: channel count
 * @data: data
 * @data_len: data len
 *
 * Return: 0 on success, negative errno on failure
 */
static int
hdd_parse_vendor_acs_chan_config(struct hdd_context *hdd_ctx,
				 struct hdd_vendor_chan_info **chan_list_ptr,
				 uint8_t *reason, uint8_t *channel_cnt,
				 const void *data, int data_len)
{
	struct nlattr *tb[SET_CHAN_MAX + 1];
	int ret;

	if (wlan_cfg80211_nla_parse(tb, SET_CHAN_MAX, data, data_len,
				    acs_chan_config_policy)) {
		hdd_err("Invalid ATTR");
		return -EINVAL;
	}

	if (tb[SET_CHAN_REASON])
		*reason = nla_get_u8(tb[SET_CHAN_REASON]);

	if (!tb[SET_CHAN_FREQ_LIST] && !tb[SET_CHAN_CHAN_LIST]) {
		hdd_err("Both channel list and frequency list are empty");
		return -EINVAL;
	}

	if (tb[SET_CHAN_FREQ_LIST]) {
		ret = hdd_extract_external_acs_frequencies(hdd_ctx,
							   chan_list_ptr,
							   channel_cnt,
							   data, data_len);
		if (ret) {
			hdd_err("Failed to extract frequencies");
			return ret;
		}

		return 0;
	}

	ret = hdd_extract_external_acs_channels(hdd_ctx, chan_list_ptr,
						channel_cnt, data, data_len);
	if (ret)
		hdd_err("Failed to extract channels");

	return ret;
}

/**
 * Undef short names for vendor set channel configuration
 */
#undef SET_CHAN_REASON
#undef SET_CHAN_CHAN_LIST
#undef SET_CHAN_PRIMARY_CHANNEL
#undef SET_CHAN_SECONDARY_CHANNEL
#undef SET_CHAN_SEG0_CENTER_CHANNEL
#undef SET_CHAN_SEG1_CENTER_CHANNEL

#undef SET_CHAN_FREQ_LIST
#undef SET_CHAN_FREQUENCY_PRIMARY
#undef SET_CHAN_FREQUENCY_SECONDARY
#undef SET_CHAN_SEG0_CENTER_FREQUENCY
#undef SET_CHAN_SEG1_CENTER_FREQUENCY

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

	hdd_enter();

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

	ret_val = hdd_parse_vendor_acs_chan_config(hdd_ctx, &channel_list,
						   &reason, &channel_cnt, data,
						   data_len);
	channel_list_ptr = channel_list;
	if (ret_val)
		return ret_val;

	/* Validate channel to be set */
	while (channel_cnt && channel_list) {
		qdf_status = wlan_hdd_validate_acs_channel(adapter,
					channel_list->pri_chan_freq,
					channel_list->chan_width);
		if (qdf_status == QDF_STATUS_SUCCESS)
			break;
		else if (channel_cnt == 1) {
			hdd_err("invalid channel frequ %u received from app",
				channel_list->pri_chan_freq);
			channel_list->pri_chan_freq = 0;
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

	hdd_debug("received primary channel freq as %d",
		  channel_list->pri_chan_freq);

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
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_update_vendor_channel(wiphy, wdev,
							  data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
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
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_setband(wiphy, wdev, data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}

/**
 *__wlan_hdd_cfg80211_getband() - get band
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Length of @data
 *
 * Return: 0 on success, negative errno on failure
 */
static int __wlan_hdd_cfg80211_getband(struct wiphy *wiphy,
				       struct wireless_dev *wdev,
				       const void *data, int data_len)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct sk_buff *skb;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	int ret;
	uint32_t reg_wifi_band_bitmap, vendor_band_mask;

	hdd_enter();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	skb = cfg80211_vendor_cmd_alloc_reply_skb(hdd_ctx->wiphy,
						  sizeof(uint32_t) +
						  NLA_HDRLEN);

	if (!skb) {
		hdd_err("cfg80211_vendor_event_alloc failed");
		return -ENOMEM;
	}

	status = ucfg_reg_get_band(hdd_ctx->pdev, &reg_wifi_band_bitmap);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("failed to get band");
		goto failure;
	}

	vendor_band_mask = wlan_reg_wifi_band_bitmap_to_vendor_bitmap(
							reg_wifi_band_bitmap);

	if (nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_SETBAND_MASK,
			vendor_band_mask)) {
		hdd_err("nla put failure");
		goto failure;
	}

	cfg80211_vendor_cmd_reply(skb);

	hdd_exit();

	return 0;

failure:
	kfree_skb(skb);
	return -EINVAL;
}

/**
 * wlan_hdd_cfg80211_getband() - Wrapper to getband
 * @wiphy:    wiphy structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of @data
 *
 * Return: 0 on success; errno on failure
 */
static int wlan_hdd_cfg80211_getband(struct wiphy *wiphy,
				     struct wireless_dev *wdev,
				     const void *data, int data_len)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_getband(wiphy, wdev, data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}

static const struct
nla_policy qca_wlan_vendor_attr[QCA_WLAN_VENDOR_ATTR_MAX+1] = {
	[QCA_WLAN_VENDOR_ATTR_ROAMING_POLICY] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_MAC_ADDR]       = {.type = NLA_BINARY,
						 .len = QDF_MAC_ADDR_SIZE},
};

void wlan_hdd_rso_cmd_status_cb(hdd_handle_t hdd_handle,
				struct rso_cmd_status *rso_status)
{
	struct hdd_context *hdd_ctx = hdd_handle_to_context(hdd_handle);
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
	bool roaming_enabled;

	hdd_enter_dev(dev);

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	if (adapter->device_mode != QDF_STA_MODE) {
		hdd_err_rl("command not allowed in %d mode, vdev_id: %d",
			   adapter->device_mode, adapter->vdev_id);
		return -EINVAL;
	}

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
	hdd_debug("ROAM_CONFIG: isFastRoamEnabled %d", is_fast_roam_enabled);

	if (sme_roaming_in_progress(hdd_ctx->mac_handle, adapter->vdev_id)) {
		hdd_err_rl("Roaming in progress for vdev %d", adapter->vdev_id);
		return -EAGAIN;
	}

	/*
	 * Get current roaming state and decide whether to wait for RSO_STOP
	 * response or not.
	 */
	roaming_enabled = ucfg_is_roaming_enabled(hdd_ctx->pdev,
						  adapter->vdev_id);

	/* Update roaming */
	qdf_status = ucfg_user_space_enable_disable_rso(hdd_ctx->pdev,
							adapter->vdev_id,
							is_fast_roam_enabled);
	if (QDF_IS_STATUS_ERROR(qdf_status))
		hdd_err("ROAM_CONFIG: sme_config_fast_roaming failed with status=%d",
			qdf_status);

	ret = qdf_status_to_os_return(qdf_status);

	if (hdd_cm_is_vdev_associated(adapter) &&
	    roaming_enabled &&
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

	hdd_exit();
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
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_set_fast_roaming(wiphy, wdev,
						     data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
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
#define STATS_SET_DATA_PKT_INFO \
		QCA_ATTR_NUD_STATS_SET_DATA_PKT_INFO
#define STATS_SET_MAX \
	QCA_ATTR_NUD_STATS_SET_MAX

const struct nla_policy
qca_wlan_vendor_set_nud_stats_policy[STATS_SET_MAX + 1] = {
	[STATS_SET_START] = {.type = NLA_FLAG },
	[STATS_GW_IPV4] = {.type = NLA_U32 },
	[STATS_SET_DATA_PKT_INFO] = {.type = NLA_NESTED },
};

/* define short names for the global vendor params */
#define CONNECTIVITY_STATS_SET_INVALID \
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_SET_INVALID
#define STATS_PKT_INFO_TYPE \
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_STATS_PKT_INFO_TYPE
#define STATS_DNS_DOMAIN_NAME \
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_DNS_DOMAIN_NAME
#define STATS_SRC_PORT \
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_SRC_PORT
#define STATS_DEST_PORT \
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_DEST_PORT
#define STATS_DEST_IPV4 \
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_DEST_IPV4
#define STATS_DEST_IPV6 \
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_DEST_IPV6
#define CONNECTIVITY_STATS_SET_MAX \
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_SET_MAX

const struct nla_policy
qca_wlan_vendor_set_connectivity_check_stats[CONNECTIVITY_STATS_SET_MAX + 1] = {
	[STATS_PKT_INFO_TYPE] = {.type = NLA_U32 },
	[STATS_DNS_DOMAIN_NAME] = {.type = NLA_NUL_STRING,
					.len = DNS_DOMAIN_NAME_MAX_LEN },
	[STATS_SRC_PORT] = {.type = NLA_U32 },
	[STATS_DEST_PORT] = {.type = NLA_U32 },
	[STATS_DEST_IPV4] = {.type = NLA_U32 },
	[STATS_DEST_IPV6] = {.type = NLA_BINARY,
					.len = ICMPv6_ADDR_LEN },
};

/**
 * hdd_dns_unmake_name_query() - Convert an uncompressed DNS name to a
 *			     NUL-terminated string
 * @name: DNS name
 *
 * Return: Produce a printable version of a DNS name.
 */
static inline uint8_t *hdd_dns_unmake_name_query(uint8_t *name)
{
	uint8_t *p;
	unsigned int len;

	p = name;
	while ((len = *p)) {
		*(p++) = '.';
		p += len;
	}

	return name + 1;
}

/**
 * hdd_dns_make_name_query() - Convert a standard NUL-terminated string
 *				to DNS name
 * @string: Name as a NUL-terminated string
 * @buf: Buffer in which to place DNS name
 *
 * DNS names consist of "<length>element" pairs.
 *
 * Return: Byte following constructed DNS name
 */
static uint8_t *hdd_dns_make_name_query(const uint8_t *string,
					uint8_t *buf, uint8_t len)
{
	uint8_t *length_byte = buf++;
	uint8_t c;

	if (string[len - 1]) {
		hdd_debug("DNS name is not null terminated");
		return NULL;
	}

	while ((c = *(string++))) {
		if (c == '.') {
			*length_byte = buf - length_byte - 1;
			length_byte = buf;
		}
		*(buf++) = c;
	}
	*length_byte = buf - length_byte - 1;
	*(buf++) = '\0';
	return buf;
}

/**
 * hdd_set_clear_connectivity_check_stats_info() - set/clear stats info
 * @adapter: Pointer to hdd adapter
 * @arp_stats_params: arp stats structure to be sent to FW
 * @tb: nl attribute
 * @is_set_stats: set/clear stats
 *
 *
 * Return: 0 on success, negative errno on failure
 */
static int hdd_set_clear_connectivity_check_stats_info(
		struct hdd_adapter *adapter,
		struct set_arp_stats_params *arp_stats_params,
		struct nlattr **tb, bool is_set_stats)
{
	struct nlattr *tb2[CONNECTIVITY_STATS_SET_MAX + 1];
	struct nlattr *curr_attr = NULL;
	int err = 0;
	uint32_t pkt_bitmap;
	int rem;

	/* Set NUD command for start tracking is received. */
	nla_for_each_nested(curr_attr,
			    tb[STATS_SET_DATA_PKT_INFO],
			    rem) {

		if (wlan_cfg80211_nla_parse(tb2,
				CONNECTIVITY_STATS_SET_MAX,
				nla_data(curr_attr), nla_len(curr_attr),
				qca_wlan_vendor_set_connectivity_check_stats)) {
			hdd_err("nla_parse failed");
			err = -EINVAL;
			goto end;
		}

		if (tb2[STATS_PKT_INFO_TYPE]) {
			pkt_bitmap = nla_get_u32(tb2[STATS_PKT_INFO_TYPE]);
			if (!pkt_bitmap) {
				hdd_err("pkt tracking bitmap is empty");
				err = -EINVAL;
				goto end;
			}

			if (is_set_stats) {
				arp_stats_params->pkt_type_bitmap = pkt_bitmap;
				arp_stats_params->flag = true;
				adapter->pkt_type_bitmap |=
					arp_stats_params->pkt_type_bitmap;

				if (pkt_bitmap & CONNECTIVITY_CHECK_SET_ARP) {
					if (!tb[STATS_GW_IPV4]) {
						hdd_err("GW ipv4 address is not present");
						err = -EINVAL;
						goto end;
					}
					arp_stats_params->ip_addr =
						nla_get_u32(tb[STATS_GW_IPV4]);
					arp_stats_params->pkt_type =
						WLAN_NUD_STATS_ARP_PKT_TYPE;
					adapter->track_arp_ip =
						arp_stats_params->ip_addr;
				}

				if (pkt_bitmap & CONNECTIVITY_CHECK_SET_DNS) {
					uint8_t *domain_name;

					if (!tb2[STATS_DNS_DOMAIN_NAME]) {
						hdd_err("DNS domain id is not present");
						err = -EINVAL;
						goto end;
					}
					domain_name = nla_data(
						tb2[STATS_DNS_DOMAIN_NAME]);
					adapter->track_dns_domain_len =
						nla_len(tb2[
							STATS_DNS_DOMAIN_NAME]);
					if (!hdd_dns_make_name_query(
						domain_name,
						adapter->dns_payload,
						adapter->track_dns_domain_len))
						adapter->track_dns_domain_len =
							0;
					/* DNStracking isn't supported in FW. */
					arp_stats_params->pkt_type_bitmap &=
						~CONNECTIVITY_CHECK_SET_DNS;
				}

				if (pkt_bitmap &
				    CONNECTIVITY_CHECK_SET_TCP_HANDSHAKE) {
					if (!tb2[STATS_SRC_PORT] ||
					    !tb2[STATS_DEST_PORT]) {
						hdd_err("Source/Dest port is not present");
						err = -EINVAL;
						goto end;
					}
					arp_stats_params->tcp_src_port =
						nla_get_u32(
							tb2[STATS_SRC_PORT]);
					arp_stats_params->tcp_dst_port =
						nla_get_u32(
							tb2[STATS_DEST_PORT]);
					adapter->track_src_port =
						arp_stats_params->tcp_src_port;
					adapter->track_dest_port =
						arp_stats_params->tcp_dst_port;
				}

				if (pkt_bitmap &
				    CONNECTIVITY_CHECK_SET_ICMPV4) {
					if (!tb2[STATS_DEST_IPV4]) {
						hdd_err("destination ipv4 address to track ping packets is not present");
						err = -EINVAL;
						goto end;
					}
					arp_stats_params->icmp_ipv4 =
						nla_get_u32(
							tb2[STATS_DEST_IPV4]);
					adapter->track_dest_ipv4 =
						arp_stats_params->icmp_ipv4;
				}
			} else {
				/* clear stats command received */
				arp_stats_params->pkt_type_bitmap = pkt_bitmap;
				arp_stats_params->flag = false;
				adapter->pkt_type_bitmap &=
					(~arp_stats_params->pkt_type_bitmap);

				if (pkt_bitmap & CONNECTIVITY_CHECK_SET_ARP) {
					arp_stats_params->pkt_type =
						WLAN_NUD_STATS_ARP_PKT_TYPE;
					qdf_mem_zero(&adapter->hdd_stats.
								hdd_arp_stats,
						     sizeof(adapter->hdd_stats.
								hdd_arp_stats));
					adapter->track_arp_ip = 0;
				}

				if (pkt_bitmap & CONNECTIVITY_CHECK_SET_DNS) {
					/* DNStracking isn't supported in FW. */
					arp_stats_params->pkt_type_bitmap &=
						~CONNECTIVITY_CHECK_SET_DNS;
					qdf_mem_zero(&adapter->hdd_stats.
								hdd_dns_stats,
						     sizeof(adapter->hdd_stats.
								hdd_dns_stats));
					qdf_mem_zero(adapter->dns_payload,
						adapter->track_dns_domain_len);
					adapter->track_dns_domain_len = 0;
				}

				if (pkt_bitmap &
				    CONNECTIVITY_CHECK_SET_TCP_HANDSHAKE) {
					qdf_mem_zero(&adapter->hdd_stats.
								hdd_tcp_stats,
						     sizeof(adapter->hdd_stats.
								hdd_tcp_stats));
					adapter->track_src_port = 0;
					adapter->track_dest_port = 0;
				}

				if (pkt_bitmap &
				    CONNECTIVITY_CHECK_SET_ICMPV4) {
					qdf_mem_zero(&adapter->hdd_stats.
							hdd_icmpv4_stats,
						     sizeof(adapter->hdd_stats.
							hdd_icmpv4_stats));
					adapter->track_dest_ipv4 = 0;
				}
			}
		} else {
			hdd_err("stats list empty");
			err = -EINVAL;
			goto end;
		}
	}

end:
	return err;
}

const struct nla_policy qca_wlan_vendor_set_trace_level_policy[
		QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_PARAM] =
	VENDOR_NLA_POLICY_NESTED(qca_wlan_vendor_set_trace_level_policy),
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

	hdd_enter();

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
				     nla_data(apth), nla_len(apth),
				     qca_wlan_vendor_set_trace_level_policy)) {
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

	hdd_exit();
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
	struct osif_psoc_sync *psoc_sync;
	int errno;

	errno = osif_psoc_sync_op_start(wiphy_dev(wiphy), &psoc_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_set_trace_level(wiphy, wdev,
						    data, data_len);

	osif_psoc_sync_op_stop(psoc_sync);

	return errno;
}

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
	struct set_arp_stats_params arp_stats_params = {0};
	int err = 0;
	mac_handle_t mac_handle;

	hdd_enter();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	err = wlan_hdd_validate_context(hdd_ctx);
	if (0 != err)
		return err;

	if (adapter->vdev_id == WLAN_UMAC_VDEV_ID_MAX) {
		hdd_err("Invalid vdev id");
		return -EINVAL;
	}

	if (adapter->device_mode != QDF_STA_MODE) {
		hdd_err("STATS supported in only STA mode!");
		return -EINVAL;
	}

	if (!hdd_cm_is_vdev_associated(adapter)) {
		hdd_debug("Not Associated");
		return 0;
	}

	if (hdd_is_roaming_in_progress(hdd_ctx))
		return -EINVAL;

	err = wlan_cfg80211_nla_parse(tb, STATS_SET_MAX, data, data_len,
				      qca_wlan_vendor_set_nud_stats_policy);
	if (err) {
		hdd_err("STATS_SET_START ATTR");
		return err;
	}

	if (tb[STATS_SET_START]) {
		/* tracking is enabled for stats other than arp. */
		if (tb[STATS_SET_DATA_PKT_INFO]) {
			err = hdd_set_clear_connectivity_check_stats_info(
						adapter,
						&arp_stats_params, tb, true);
			if (err)
				return -EINVAL;

			/*
			 * if only tracking dns, then don't send
			 * wmi command to FW.
			 */
			if (!arp_stats_params.pkt_type_bitmap)
				return err;
		} else {
			if (!tb[STATS_GW_IPV4]) {
				hdd_err("STATS_SET_START CMD");
				return -EINVAL;
			}

			arp_stats_params.pkt_type_bitmap =
						CONNECTIVITY_CHECK_SET_ARP;
			adapter->pkt_type_bitmap |=
					arp_stats_params.pkt_type_bitmap;
			arp_stats_params.flag = true;
			arp_stats_params.ip_addr =
					nla_get_u32(tb[STATS_GW_IPV4]);
			adapter->track_arp_ip = arp_stats_params.ip_addr;
			arp_stats_params.pkt_type = WLAN_NUD_STATS_ARP_PKT_TYPE;
		}
	} else {
		/* clear stats command received. */
		if (tb[STATS_SET_DATA_PKT_INFO]) {
			err = hdd_set_clear_connectivity_check_stats_info(
						adapter,
						&arp_stats_params, tb, false);
			if (err)
				return -EINVAL;

			/*
			 * if only tracking dns, then don't send
			 * wmi command to FW.
			 */
			if (!arp_stats_params.pkt_type_bitmap)
				return err;
		} else {
			arp_stats_params.pkt_type_bitmap =
						CONNECTIVITY_CHECK_SET_ARP;
			adapter->pkt_type_bitmap &=
					(~arp_stats_params.pkt_type_bitmap);
			arp_stats_params.flag = false;
			qdf_mem_zero(&adapter->hdd_stats.hdd_arp_stats,
				     sizeof(adapter->hdd_stats.hdd_arp_stats));
			arp_stats_params.pkt_type = WLAN_NUD_STATS_ARP_PKT_TYPE;
		}
	}

	hdd_debug("STATS_SET_START Received flag %d!", arp_stats_params.flag);

	arp_stats_params.vdev_id = adapter->vdev_id;

	mac_handle = hdd_ctx->mac_handle;
	if (QDF_STATUS_SUCCESS !=
	    sme_set_nud_debug_stats(mac_handle, &arp_stats_params)) {
		hdd_err("STATS_SET_START CMD Failed!");
		return -EINVAL;
	}

	hdd_exit();

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
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_set_nud_stats(wiphy, wdev, data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
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
#define DATA_PKT_STATS \
	QCA_ATTR_NUD_STATS_DATA_PKT_STATS
#define STATS_GET_MAX \
	QCA_ATTR_NUD_STATS_GET_MAX

#define CHECK_STATS_INVALID \
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_INVALID
#define CHECK_STATS_PKT_TYPE \
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_PKT_TYPE
#define CHECK_STATS_PKT_DNS_DOMAIN_NAME \
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_PKT_DNS_DOMAIN_NAME
#define CHECK_STATS_PKT_SRC_PORT \
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_PKT_SRC_PORT
#define CHECK_STATS_PKT_DEST_PORT \
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_PKT_DEST_PORT
#define CHECK_STATS_PKT_DEST_IPV4 \
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_PKT_DEST_IPV4
#define CHECK_STATS_PKT_DEST_IPV6 \
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_PKT_DEST_IPV6
#define CHECK_STATS_PKT_REQ_COUNT_FROM_NETDEV \
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_PKT_REQ_COUNT_FROM_NETDEV
#define CHECK_STATS_PKT_REQ_COUNT_TO_LOWER_MAC \
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_PKT_REQ_COUNT_TO_LOWER_MAC
#define CHECK_STATS_PKT_REQ_RX_COUNT_BY_LOWER_MAC \
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_PKT_REQ_RX_COUNT_BY_LOWER_MAC
#define CHECK_STATS_PKT_REQ_COUNT_TX_SUCCESS \
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_PKT_REQ_COUNT_TX_SUCCESS
#define CHECK_STATS_PKT_RSP_RX_COUNT_BY_LOWER_MAC \
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_PKT_RSP_RX_COUNT_BY_LOWER_MAC
#define CHECK_STATS_PKT_RSP_RX_COUNT_BY_UPPER_MAC \
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_PKT_RSP_RX_COUNT_BY_UPPER_MAC
#define CHECK_STATS_PKT_RSP_COUNT_TO_NETDEV \
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_PKT_RSP_COUNT_TO_NETDEV
#define CHECK_STATS_PKT_RSP_COUNT_OUT_OF_ORDER_DROP \
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_PKT_RSP_COUNT_OUT_OF_ORDER_DROP
#define CHECK_DATA_STATS_MAX \
	QCA_ATTR_CONNECTIVITY_CHECK_DATA_STATS_MAX


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
	[DATA_PKT_STATS] = {.type = NLA_U16 },
};

/**
 * hdd_populate_dns_stats_info() - send dns stats info to network stack
 * @adapter: pointer to adapter context
 * @skb: pointer to skb
 *
 *
 * Return: An error code or 0 on success.
 */
static int hdd_populate_dns_stats_info(struct hdd_adapter *adapter,
				       struct sk_buff *skb)
{
	uint8_t *dns_query;

	dns_query = qdf_mem_malloc(adapter->track_dns_domain_len + 1);
	if (!dns_query)
		return -EINVAL;

	qdf_mem_copy(dns_query, adapter->dns_payload,
		     adapter->track_dns_domain_len);

	if (nla_put_u16(skb, CHECK_STATS_PKT_TYPE,
		CONNECTIVITY_CHECK_SET_DNS) ||
	    nla_put(skb, CHECK_STATS_PKT_DNS_DOMAIN_NAME,
		adapter->track_dns_domain_len,
		hdd_dns_unmake_name_query(dns_query)) ||
	    nla_put_u16(skb, CHECK_STATS_PKT_REQ_COUNT_FROM_NETDEV,
		adapter->hdd_stats.hdd_dns_stats.tx_dns_req_count) ||
	    nla_put_u16(skb, CHECK_STATS_PKT_REQ_COUNT_TO_LOWER_MAC,
		adapter->hdd_stats.hdd_dns_stats.tx_host_fw_sent) ||
	    nla_put_u16(skb, CHECK_STATS_PKT_REQ_RX_COUNT_BY_LOWER_MAC,
		adapter->hdd_stats.hdd_dns_stats.tx_host_fw_sent) ||
	    nla_put_u16(skb, CHECK_STATS_PKT_REQ_COUNT_TX_SUCCESS,
		adapter->hdd_stats.hdd_dns_stats.tx_ack_cnt) ||
	    nla_put_u16(skb, CHECK_STATS_PKT_RSP_RX_COUNT_BY_UPPER_MAC,
		adapter->hdd_stats.hdd_dns_stats.rx_dns_rsp_count) ||
	    nla_put_u16(skb, CHECK_STATS_PKT_RSP_COUNT_TO_NETDEV,
		adapter->hdd_stats.hdd_dns_stats.rx_delivered) ||
	    nla_put_u16(skb, CHECK_STATS_PKT_RSP_COUNT_OUT_OF_ORDER_DROP,
		adapter->hdd_stats.hdd_dns_stats.rx_host_drop)) {
		hdd_err("nla put fail");
		qdf_mem_free(dns_query);
		kfree_skb(skb);
		return -EINVAL;
	}
	qdf_mem_free(dns_query);
	return 0;
}

/**
 * hdd_populate_tcp_stats_info() - send tcp stats info to network stack
 * @adapter: pointer to adapter context
 * @skb: pointer to skb
 * @pkt_type: tcp pkt type
 *
 * Return: An error code or 0 on success.
 */
static int hdd_populate_tcp_stats_info(struct hdd_adapter *adapter,
				       struct sk_buff *skb,
				       uint8_t pkt_type)
{
	switch (pkt_type) {
	case CONNECTIVITY_CHECK_SET_TCP_SYN:
		/* Fill info for tcp syn packets (tx packet) */
		if (nla_put_u16(skb, CHECK_STATS_PKT_TYPE,
			CONNECTIVITY_CHECK_SET_TCP_SYN) ||
		    nla_put_u16(skb, CHECK_STATS_PKT_SRC_PORT,
			adapter->track_src_port) ||
		    nla_put_u16(skb, CHECK_STATS_PKT_DEST_PORT,
			adapter->track_dest_port) ||
		    nla_put_u16(skb, CHECK_STATS_PKT_REQ_COUNT_FROM_NETDEV,
			adapter->hdd_stats.hdd_tcp_stats.tx_tcp_syn_count) ||
		    nla_put_u16(skb, CHECK_STATS_PKT_REQ_COUNT_TO_LOWER_MAC,
			adapter->hdd_stats.hdd_tcp_stats.
						tx_tcp_syn_host_fw_sent) ||
		    nla_put_u16(skb, CHECK_STATS_PKT_REQ_RX_COUNT_BY_LOWER_MAC,
			adapter->hdd_stats.hdd_tcp_stats.
						tx_tcp_syn_host_fw_sent) ||
		    nla_put_u16(skb, CHECK_STATS_PKT_REQ_COUNT_TX_SUCCESS,
			adapter->hdd_stats.hdd_tcp_stats.tx_tcp_syn_ack_cnt)) {
			hdd_err("nla put fail");
			kfree_skb(skb);
			return -EINVAL;
		}
		break;
	case CONNECTIVITY_CHECK_SET_TCP_SYN_ACK:
		/* Fill info for tcp syn-ack packets (rx packet) */
		if (nla_put_u16(skb, CHECK_STATS_PKT_TYPE,
			CONNECTIVITY_CHECK_SET_TCP_SYN_ACK) ||
		    nla_put_u16(skb, CHECK_STATS_PKT_SRC_PORT,
			adapter->track_src_port) ||
		    nla_put_u16(skb, CHECK_STATS_PKT_DEST_PORT,
			adapter->track_dest_port) ||
		    nla_put_u16(skb, CHECK_STATS_PKT_RSP_RX_COUNT_BY_LOWER_MAC,
			adapter->hdd_stats.hdd_tcp_stats.rx_fw_cnt) ||
		    nla_put_u16(skb, CHECK_STATS_PKT_RSP_RX_COUNT_BY_UPPER_MAC,
			adapter->hdd_stats.hdd_tcp_stats.
							rx_tcp_syn_ack_count) ||
		    nla_put_u16(skb, CHECK_STATS_PKT_RSP_COUNT_TO_NETDEV,
			adapter->hdd_stats.hdd_tcp_stats.rx_delivered) ||
		    nla_put_u16(skb,
			CHECK_STATS_PKT_RSP_COUNT_OUT_OF_ORDER_DROP,
			adapter->hdd_stats.hdd_tcp_stats.rx_host_drop)) {
			hdd_err("nla put fail");
			kfree_skb(skb);
			return -EINVAL;
		}
		break;
	case CONNECTIVITY_CHECK_SET_TCP_ACK:
		/* Fill info for tcp ack packets (tx packet) */
		if (nla_put_u16(skb, CHECK_STATS_PKT_TYPE,
			CONNECTIVITY_CHECK_SET_TCP_ACK) ||
		    nla_put_u16(skb, CHECK_STATS_PKT_SRC_PORT,
			adapter->track_src_port) ||
		    nla_put_u16(skb, CHECK_STATS_PKT_DEST_PORT,
			adapter->track_dest_port) ||
		    nla_put_u16(skb, CHECK_STATS_PKT_REQ_COUNT_FROM_NETDEV,
			adapter->hdd_stats.hdd_tcp_stats.tx_tcp_ack_count) ||
		    nla_put_u16(skb, CHECK_STATS_PKT_REQ_COUNT_TO_LOWER_MAC,
			adapter->hdd_stats.hdd_tcp_stats.
						tx_tcp_ack_host_fw_sent) ||
		    nla_put_u16(skb, CHECK_STATS_PKT_REQ_RX_COUNT_BY_LOWER_MAC,
			adapter->hdd_stats.hdd_tcp_stats.
						tx_tcp_ack_host_fw_sent) ||
		    nla_put_u16(skb, CHECK_STATS_PKT_REQ_COUNT_TX_SUCCESS,
			adapter->hdd_stats.hdd_tcp_stats.tx_tcp_ack_ack_cnt)) {
			hdd_err("nla put fail");
			kfree_skb(skb);
			return -EINVAL;
		}
		break;
	default:
		break;
	}
	return 0;
}

/**
 * hdd_populate_icmpv4_stats_info() - send icmpv4 stats info to network stack
 * @adapter: pointer to adapter context
 * @skb: pointer to skb
 *
 *
 * Return: An error code or 0 on success.
 */
static int hdd_populate_icmpv4_stats_info(struct hdd_adapter *adapter,
					  struct sk_buff *skb)
{
	if (nla_put_u16(skb, CHECK_STATS_PKT_TYPE,
		CONNECTIVITY_CHECK_SET_ICMPV4) ||
	    nla_put_u32(skb, CHECK_STATS_PKT_DEST_IPV4,
		adapter->track_dest_ipv4) ||
	    nla_put_u16(skb, CHECK_STATS_PKT_REQ_COUNT_FROM_NETDEV,
		adapter->hdd_stats.hdd_icmpv4_stats.tx_icmpv4_req_count) ||
	    nla_put_u16(skb, CHECK_STATS_PKT_REQ_COUNT_TO_LOWER_MAC,
		adapter->hdd_stats.hdd_icmpv4_stats.tx_host_fw_sent) ||
	    nla_put_u16(skb, CHECK_STATS_PKT_REQ_RX_COUNT_BY_LOWER_MAC,
		adapter->hdd_stats.hdd_icmpv4_stats.tx_host_fw_sent) ||
	    nla_put_u16(skb, CHECK_STATS_PKT_REQ_COUNT_TX_SUCCESS,
		adapter->hdd_stats.hdd_icmpv4_stats.tx_ack_cnt) ||
	    nla_put_u16(skb, CHECK_STATS_PKT_RSP_RX_COUNT_BY_LOWER_MAC,
		adapter->hdd_stats.hdd_icmpv4_stats.rx_fw_cnt) ||
	    nla_put_u16(skb, CHECK_STATS_PKT_RSP_RX_COUNT_BY_UPPER_MAC,
		adapter->hdd_stats.hdd_icmpv4_stats.rx_icmpv4_rsp_count) ||
	    nla_put_u16(skb, CHECK_STATS_PKT_RSP_COUNT_TO_NETDEV,
		adapter->hdd_stats.hdd_icmpv4_stats.rx_delivered) ||
	    nla_put_u16(skb, CHECK_STATS_PKT_RSP_COUNT_OUT_OF_ORDER_DROP,
		adapter->hdd_stats.hdd_icmpv4_stats.rx_host_drop)) {
		hdd_err("nla put fail");
		kfree_skb(skb);
		return -EINVAL;
	}
	return 0;
}

/**
 * hdd_populate_connectivity_check_stats_info() - send connectivity stats info
 *						  to network stack
 * @adapter: pointer to adapter context
 * @skb: pointer to skb
 *
 *
 * Return: An error code or 0 on success.
 */

static int hdd_populate_connectivity_check_stats_info(
	struct hdd_adapter *adapter, struct sk_buff *skb)
{
	struct nlattr *connect_stats, *connect_info;
	uint32_t count = 0;

	connect_stats = nla_nest_start(skb, DATA_PKT_STATS);
	if (!connect_stats) {
		hdd_err("nla_nest_start failed");
		return -EINVAL;
	}

	if (adapter->pkt_type_bitmap & CONNECTIVITY_CHECK_SET_DNS) {
		connect_info = nla_nest_start(skb, count);
		if (!connect_info) {
			hdd_err("nla_nest_start failed count %u", count);
			return -EINVAL;
		}

		if (hdd_populate_dns_stats_info(adapter, skb))
			goto put_attr_fail;
		nla_nest_end(skb, connect_info);
		count++;
	}

	if (adapter->pkt_type_bitmap & CONNECTIVITY_CHECK_SET_TCP_HANDSHAKE) {
		connect_info = nla_nest_start(skb, count);
		if (!connect_info) {
			hdd_err("nla_nest_start failed count %u", count);
			return -EINVAL;
		}
		if (hdd_populate_tcp_stats_info(adapter, skb,
					CONNECTIVITY_CHECK_SET_TCP_SYN))
			goto put_attr_fail;
		nla_nest_end(skb, connect_info);
		count++;

		connect_info = nla_nest_start(skb, count);
		if (!connect_info) {
			hdd_err("nla_nest_start failed count %u", count);
			return -EINVAL;
		}
		if (hdd_populate_tcp_stats_info(adapter, skb,
					CONNECTIVITY_CHECK_SET_TCP_SYN_ACK))
			goto put_attr_fail;
		nla_nest_end(skb, connect_info);
		count++;

		connect_info = nla_nest_start(skb, count);
		if (!connect_info) {
			hdd_err("nla_nest_start failed count %u", count);
			return -EINVAL;
		}
		if (hdd_populate_tcp_stats_info(adapter, skb,
					CONNECTIVITY_CHECK_SET_TCP_ACK))
			goto put_attr_fail;
		nla_nest_end(skb, connect_info);
		count++;
	}

	if (adapter->pkt_type_bitmap & CONNECTIVITY_CHECK_SET_ICMPV4) {
		connect_info = nla_nest_start(skb, count);
		if (!connect_info) {
			hdd_err("nla_nest_start failed count %u", count);
			return -EINVAL;
		}

		if (hdd_populate_icmpv4_stats_info(adapter, skb))
			goto put_attr_fail;
		nla_nest_end(skb, connect_info);
		count++;
	}

	nla_nest_end(skb, connect_stats);
	return 0;

put_attr_fail:
	hdd_err("QCA_WLAN_VENDOR_ATTR put fail. count %u", count);
	return -EINVAL;
}


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
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct get_arp_stats_params arp_stats_params;
	mac_handle_t mac_handle;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	uint32_t pkt_type_bitmap;
	struct sk_buff *skb;
	struct osif_request *request = NULL;
	static const struct osif_request_params params = {
		.priv_size = 0,
		.timeout_ms = WLAN_WAIT_TIME_NUD_STATS,
	};
	void *cookie = NULL;

	hdd_enter();

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

	if (adapter->device_mode != QDF_STA_MODE) {
		hdd_err("STATS supported in only STA mode!");
		return -EINVAL;
	}

	request = osif_request_alloc(&params);
	if (!request) {
		hdd_err("Request allocation failure");
		return -ENOMEM;
	}

	cookie = osif_request_cookie(request);

	arp_stats_params.pkt_type = WLAN_NUD_STATS_ARP_PKT_TYPE;
	arp_stats_params.vdev_id = adapter->vdev_id;

	pkt_type_bitmap = adapter->pkt_type_bitmap;

	/* send NUD failure event only when ARP tracking is enabled. */
	if (cdp_cfg_get(soc, cfg_dp_enable_data_stall) &&
	    !hdd_ctx->config->enable_nud_tracking &&
	    (pkt_type_bitmap & CONNECTIVITY_CHECK_SET_ARP)) {
		QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_ERROR,
			  "Data stall due to NUD failure");
		cdp_post_data_stall_event(soc,
				      DATA_STALL_LOG_INDICATOR_FRAMEWORK,
				      DATA_STALL_LOG_NUD_FAILURE,
				      OL_TXRX_PDEV_ID, 0XFF,
				      DATA_STALL_LOG_RECOVERY_TRIGGER_PDR);
	}

	mac_handle = hdd_ctx->mac_handle;
	if (sme_set_nud_debug_stats_cb(mac_handle, hdd_get_nud_stats_cb,
				       cookie) != QDF_STATUS_SUCCESS) {
		hdd_err("Setting NUD debug stats callback failure");
		err = -EINVAL;
		goto exit;
	}

	if (QDF_STATUS_SUCCESS !=
	    sme_get_nud_debug_stats(mac_handle, &arp_stats_params)) {
		hdd_err("STATS_SET_START CMD Failed!");
		err = -EINVAL;
		goto exit;
	}

	err = osif_request_wait_for_response(request);
	if (err) {
		hdd_err("SME timedout while retrieving NUD stats");
		err = -ETIMEDOUT;
		goto exit;
	}

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy,
						  WLAN_NUD_STATS_LEN);
	if (!skb) {
		hdd_err("cfg80211_vendor_cmd_alloc_reply_skb failed");
		err = -ENOMEM;
		goto exit;
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
		err = -EINVAL;
		goto exit;
	}
	if (adapter->con_status)
		nla_put_flag(skb, AP_LINK_ACTIVE);
	if (adapter->dad)
		nla_put_flag(skb, AP_LINK_DAD);

	/* ARP tracking is done above. */
	pkt_type_bitmap &= ~CONNECTIVITY_CHECK_SET_ARP;

	if (pkt_type_bitmap) {
		if (hdd_populate_connectivity_check_stats_info(adapter, skb)) {
			err = -EINVAL;
			goto exit;
		}
	}

	cfg80211_vendor_cmd_reply(skb);
exit:
	osif_request_put(request);
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
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_get_nud_stats(wiphy, wdev, data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
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

void hdd_bt_activity_cb(hdd_handle_t hdd_handle, uint32_t bt_activity)
{
	struct hdd_context *hdd_ctx = hdd_handle_to_context(hdd_handle);
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
	else if (bt_activity == WLAN_COEX_EVENT_BT_PROFILE_CONNECTED)
		hdd_ctx->bt_profile_con = 1;
	else if (bt_activity == WLAN_COEX_EVENT_BT_PROFILE_DISCONNECTED)
		hdd_ctx->bt_profile_con = 0;
	else
		return;

	ucfg_scan_set_bt_activity(hdd_ctx->psoc, hdd_ctx->bt_a2dp_active);
	hdd_debug("a2dp_active: %d vo_active: %d connected:%d",
		  hdd_ctx->bt_a2dp_active,
		  hdd_ctx->bt_vo_active, hdd_ctx->bt_profile_con);
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
	struct osif_request *request;
	struct chain_rssi_priv *priv;

	hdd_enter();

	request = osif_request_get(context);
	if (!request) {
		hdd_err("Obsolete request");
		return;
	}

	priv = osif_request_priv(request);
	priv->chain_rssi = *data;
	osif_request_complete(request);
	osif_request_put(request);
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

	skb = cfg80211_vendor_cmd_alloc_reply_skb(hdd_ctx->wiphy,
		(sizeof(result->chain_rssi) + NLA_HDRLEN) +
		(sizeof(result->chain_evm) + NLA_HDRLEN) +
		(sizeof(result->ant_id) + NLA_HDRLEN) +
		NLMSG_HDRLEN);

	if (!skb) {
		hdd_err("cfg80211_vendor_event_alloc failed");
		return -ENOMEM;
	}

	if (nla_put(skb, QCA_WLAN_VENDOR_ATTR_CHAIN_RSSI,
		    sizeof(result->chain_rssi),
		    result->chain_rssi)) {
		hdd_err("put fail");
		goto nla_put_failure;
	}

	if (nla_put(skb, QCA_WLAN_VENDOR_ATTR_CHAIN_EVM,
		    sizeof(result->chain_evm),
		    result->chain_evm)) {
		hdd_err("put fail");
		goto nla_put_failure;
	}

	if (nla_put(skb, QCA_WLAN_VENDOR_ATTR_ANTENNA_INFO,
		    sizeof(result->ant_id),
		    result->ant_id)) {
		hdd_err("put fail");
		goto nla_put_failure;
	}

	cfg80211_vendor_cmd_reply(skb);
	return 0;

nla_put_failure:
	kfree_skb(skb);
	return -EINVAL;
}

static const struct
nla_policy get_chain_rssi_policy[QCA_WLAN_VENDOR_ATTR_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_MAC_ADDR]       = {.type = NLA_BINARY,
						 .len = QDF_MAC_ADDR_SIZE},
};

static const struct nla_policy
get_chan_info[QCA_WLAN_VENDOR_ATTR_CHAN_INFO_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_CHAN_INFO_INVALID] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_CHAN_INFO_PRIMARY_FREQ] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_CHAN_INFO_SEG0_FREQ] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_CHAN_INFO_SEG1_FREQ] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_CHAN_INFO_BANDWIDTH] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_CHAN_INFO_IFACE_MODE_MASK] = {.type = NLA_U32},
};

static const struct nla_policy
get_usable_channel_policy[QCA_WLAN_VENDOR_ATTR_USABLE_CHANNELS_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_USABLE_CHANNELS_INVALID] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_USABLE_CHANNELS_BAND_MASK] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_USABLE_CHANNELS_IFACE_MODE_MASK] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_USABLE_CHANNELS_FILTER_MASK] = {
		.type = NLA_U32
	},
	[QCA_WLAN_VENDOR_ATTR_USABLE_CHANNELS_CHAN_INFO] = {
		.type = NLA_NESTED
	},
};

#ifdef WLAN_FEATURE_GET_USABLE_CHAN_LIST
static enum nl80211_chan_width
hdd_convert_phy_bw_to_nl_bw(enum phy_ch_width bw)
{
	switch (bw) {
	case CH_WIDTH_20MHZ:
		return NL80211_CHAN_WIDTH_20;
	case CH_WIDTH_40MHZ:
		return NL80211_CHAN_WIDTH_40;
	case CH_WIDTH_160MHZ:
		return NL80211_CHAN_WIDTH_160;
	case CH_WIDTH_80MHZ:
		return NL80211_CHAN_WIDTH_80;
	case CH_WIDTH_80P80MHZ:
		return NL80211_CHAN_WIDTH_80P80;
	case CH_WIDTH_5MHZ:
		return NL80211_CHAN_WIDTH_5;
	case CH_WIDTH_10MHZ:
		return NL80211_CHAN_WIDTH_10;
#if defined(WLAN_FEATURE_11BE)
#if defined(CFG80211_11BE_BASIC)
	case CH_WIDTH_320MHZ:
		return NL80211_CHAN_WIDTH_320;
#else
	case CH_WIDTH_320MHZ:
		return NL80211_CHAN_WIDTH_20;
#endif
#endif
	case CH_WIDTH_INVALID:
	case CH_WIDTH_MAX:
		return NL80211_CHAN_WIDTH_20;
	}

	return NL80211_CHAN_WIDTH_20;
}

/**
 * hdd_fill_usable_channels_data() - Fill the data requested by userspace
 * @skb: SK buffer
 * @tb: List of attributes
 * @res_msg: structure of usable channel info
 * @count: no of usable channels
 *
 * Get the data corresponding to the attribute list specified in tb and
 * update the same to skb by populating the same attributes.
 *
 * Return: 0 on success; error number on failure
 */
static int
hdd_fill_usable_channels_data(struct sk_buff *skb, struct nlattr **tb,
			      struct get_usable_chan_res_params *res_msg,
			      int count)
{
	struct nlattr *config, *chan_params;
	uint8_t i, bw, j = 0;

	config = nla_nest_start(skb,
				QCA_WLAN_VENDOR_ATTR_USABLE_CHANNELS_CHAN_INFO);
	if (!config) {
		hdd_err("nla nest start failure");
		return -EINVAL;
	}
	for (i = 0; i < count ; i++) {
		if (!res_msg[i].freq)
			continue;
		chan_params = nla_nest_start(skb, j);
		if (!chan_params)
			return -EINVAL;
		j++;
		bw = hdd_convert_phy_bw_to_nl_bw(res_msg[i].bw);
		hdd_debug("populating chan_params freq %d bw %d iface mode %d, seg0 %d",
			  res_msg[i].freq, bw, res_msg[i].iface_mode_mask,
			  res_msg[i].seg0_freq);
		if (nla_put_u32(skb,
				QCA_WLAN_VENDOR_ATTR_CHAN_INFO_PRIMARY_FREQ,
				res_msg[i].freq) ||
		    nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_CHAN_INFO_SEG0_FREQ,
				res_msg[i].seg0_freq) ||
		    nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_CHAN_INFO_SEG1_FREQ,
				res_msg[i].seg1_freq) ||
		    nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_CHAN_INFO_BANDWIDTH,
				bw) ||
		    nla_put_u32(skb,
				QCA_WLAN_VENDOR_ATTR_CHAN_INFO_IFACE_MODE_MASK,
				res_msg[i].iface_mode_mask)) {
			hdd_err("nla put failre");
			return -EINVAL;
		}

		nla_nest_end(skb, chan_params);
	}
	nla_nest_end(skb, config);
	return 0;
}

/**
 * hdd_get_usable_cahnnel_len() - calculate the length required by skb
 * @count: number of usable channels
 *
 * Find the required length to send usable channel data to upper layer
 *
 * Return: required len
 */
static uint32_t
hdd_get_usable_cahnnel_len(uint32_t count)
{
	uint32_t len = 0;
	struct get_usable_chan_res_params res_msg;

	len = nla_total_size(sizeof(res_msg.freq)) +
		nla_total_size(sizeof(res_msg.seg0_freq)) +
		nla_total_size(sizeof(res_msg.seg1_freq)) +
		nla_total_size(sizeof(res_msg.bw)) +
		nla_total_size(sizeof(res_msg.iface_mode_mask));

	return len * count;
}

/**
 * hdd_send_usable_channel() - Send usable channels as vendor cmd reply
 * @mac_handle: Opaque handle to the MAC context
 * @vdev_id: vdev id
 * @tb: List of attributes
 *
 * Parse the attributes list tb and  get the data corresponding to the
 * attributes specified in tb. Send them as a vendor response.
 *
 * Return: 0 on success; error number on failure
 */
static int
hdd_send_usable_channel(struct hdd_context *hdd_ctx,
			struct get_usable_chan_res_params *res_msg,
			uint32_t count,
			struct nlattr **tb)
{
	struct sk_buff *skb;
	uint32_t skb_len;
	int status;

	skb_len = hdd_get_usable_cahnnel_len(count);
	if (!skb_len) {
		hdd_err("No data requested");
		return -EINVAL;
	}

	skb_len += NLMSG_HDRLEN;
	skb = cfg80211_vendor_cmd_alloc_reply_skb(hdd_ctx->wiphy, skb_len);
	if (!skb) {
		hdd_info("cfg80211_vendor_cmd_alloc_reply_skb failed");
		return -ENOMEM;
	}

	status = hdd_fill_usable_channels_data(skb, tb, res_msg, count);
	if (status)
		goto fail;

	return cfg80211_vendor_cmd_reply(skb);

fail:
	hdd_err("nla put fail");
	kfree_skb(skb);
	return status;
}

/**
 * hdd_get_all_band_mask() - get supported nl80211 bands
 *
 * Return: supported band mask
 */
static uint32_t
hdd_get_all_band_mask(void)
{
	uint32_t band_mask = 0;

	band_mask =
		(1 << REG_BAND_2G) | (1 << REG_BAND_5G) | (1 << REG_BAND_6G);

	return band_mask;
}

/**
 * hdd_get_all_iface_mode_mask() - get supported nl80211 iface mode
 *
 * Return: supported iface mode mask
 */
static uint32_t
hdd_get_all_iface_mode_mask(void)
{
	uint32_t mode_mask = 0;

	mode_mask = (1 << NL80211_IFTYPE_STATION) |
			(1 << NL80211_IFTYPE_AP) |
			(1 << NL80211_IFTYPE_P2P_GO) |
			(1 << NL80211_IFTYPE_P2P_CLIENT) |
			(1 << NL80211_IFTYPE_P2P_DEVICE) |
			(1 << NL80211_IFTYPE_NAN);

	return mode_mask;
}

/**
 * hdd_convert_nl80211_to_reg_band_mask() - convert n80211 band to reg band
 * @band: nl80211 band
 *
 * Return: reg band value
 */

static uint32_t
hdd_convert_nl80211_to_reg_band_mask(enum nl80211_band band)
{
	uint32_t reg_band = 0;

	if (band & 1 << NL80211_BAND_2GHZ)
		reg_band |= 1 << REG_BAND_2G;
	if (band & 1 << NL80211_BAND_5GHZ)
		reg_band |= 1 << REG_BAND_5G;
	if (band & 1 << NL80211_BAND_6GHZ)
		reg_band |= 1 << REG_BAND_6G;
	if (band & 1 << NL80211_BAND_60GHZ)
		hdd_err("band: %d not supported", NL80211_BAND_60GHZ);

	return reg_band;
}

/**
 * __wlan_hdd_cfg80211_get_usable_channel() - get chain rssi
 * @wiphy: wiphy pointer
 * @wdev: pointer to struct wireless_dev
 * @data: pointer to incoming NL vendor data
 * @data_len: length of @data
 *
 * Return: 0 on success; error number otherwise.
 */
static int __wlan_hdd_cfg80211_get_usable_channel(struct wiphy *wiphy,
						  struct wireless_dev *wdev,
						  const void *data,
						  int data_len)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct get_usable_chan_req_params req_msg = {0};
	struct get_usable_chan_res_params *res_msg;
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_USABLE_CHANNELS_MAX + 1];
	int ret = 0;
	uint32_t count = 0;
	QDF_STATUS status;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	if (hdd_ctx->driver_status == DRIVER_MODULES_CLOSED) {
		hdd_err("Driver Modules are closed");
		return -EINVAL;
	}

	res_msg = qdf_mem_malloc(NUM_CHANNELS *
				 sizeof(*res_msg));

	if (!res_msg) {
		hdd_err("res_msg invalid");
		return -EINVAL;
	}

	if (wlan_cfg80211_nla_parse(
				tb, QCA_WLAN_VENDOR_ATTR_USABLE_CHANNELS_MAX,
				data, data_len, get_usable_channel_policy)) {
		hdd_err("Invalid ATTR");
		ret = -EINVAL;
		goto err;
	}

	if (!tb[QCA_WLAN_VENDOR_ATTR_USABLE_CHANNELS_BAND_MASK]) {
		hdd_err("band mask not present");
		req_msg.band_mask = hdd_get_all_band_mask();
	} else {
		req_msg.band_mask =
		nla_get_u32(tb[QCA_WLAN_VENDOR_ATTR_USABLE_CHANNELS_BAND_MASK]);
		if (!req_msg.band_mask)
			req_msg.band_mask = hdd_get_all_band_mask();
		else
			req_msg.band_mask =
			hdd_convert_nl80211_to_reg_band_mask(req_msg.band_mask);
	}
	if (!tb[QCA_WLAN_VENDOR_ATTR_USABLE_CHANNELS_IFACE_MODE_MASK]) {
		hdd_err("iface mode mask not present");
		req_msg.iface_mode_mask = hdd_get_all_iface_mode_mask();
	} else {
		req_msg.iface_mode_mask = nla_get_u32(
		tb[QCA_WLAN_VENDOR_ATTR_USABLE_CHANNELS_IFACE_MODE_MASK]);
		if (!req_msg.iface_mode_mask)
			req_msg.iface_mode_mask = hdd_get_all_iface_mode_mask();
	}

	if (!tb[QCA_WLAN_VENDOR_ATTR_USABLE_CHANNELS_FILTER_MASK]) {
		hdd_err("usable channels filter mask not present");
		req_msg.filter_mask = 0;
	} else {
		req_msg.filter_mask =
			nla_get_u32(
			tb[QCA_WLAN_VENDOR_ATTR_USABLE_CHANNELS_FILTER_MASK]);
	}

	hdd_debug("get usable channel list for band %d mode %d filter %d",
		  req_msg.band_mask, req_msg.iface_mode_mask,
		  req_msg.filter_mask);

	status = wlan_reg_get_usable_channel(hdd_ctx->pdev, req_msg,
					     res_msg, &count);
	if (QDF_STATUS_SUCCESS != status) {
		hdd_err("get usable channel failed %d", status);
		ret = -EINVAL;
		goto err;
	}
	hdd_debug("usable channel count : %d", count);

	ret = hdd_send_usable_channel(hdd_ctx, res_msg, count, tb);
	if (ret) {
		hdd_err("failed to send usable_channels");
		ret = -EINVAL;
		goto err;
	}

err:
	qdf_mem_free(res_msg);
	if (ret)
		return ret;
	return qdf_status_to_os_return(status);
}
#endif

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * enum roam_stats_set_params - Different types of params to set the roam stats
 * @ROAM_RT_STATS_DISABLED:                Roam stats feature disabled
 * @ROAM_RT_STATS_ENABLED:                 Roam stats feature enabled
 * @ROAM_RT_STATS_ENABLED_IN_SUSPEND_MODE: Roam stats enabled in suspend mode
 */
enum roam_stats_set_params {
	ROAM_RT_STATS_DISABLED = 0,
	ROAM_RT_STATS_ENABLED = 1,
	ROAM_RT_STATS_ENABLED_IN_SUSPEND_MODE = 2,
};

#define EVENTS_CONFIGURE QCA_WLAN_VENDOR_ATTR_ROAM_EVENTS_CONFIGURE
#define SUSPEND_STATE    QCA_WLAN_VENDOR_ATTR_ROAM_EVENTS_SUSPEND_STATE

static const struct nla_policy
set_roam_events_policy[QCA_WLAN_VENDOR_ATTR_ROAM_EVENTS_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_ROAM_EVENTS_CONFIGURE] = {.type = NLA_U8},
	[QCA_WLAN_VENDOR_ATTR_ROAM_EVENTS_SUSPEND_STATE] = {.type = NLA_FLAG},
};

/**
 * __wlan_hdd_cfg80211_set_roam_events() - set roam stats
 * @wiphy: wiphy pointer
 * @wdev: pointer to struct wireless_dev
 * @data: pointer to incoming NL vendor data
 * @data_len: length of @data
 *
 * Return: 0 on success; error number otherwise.
 */
static int __wlan_hdd_cfg80211_set_roam_events(struct wiphy *wiphy,
					       struct wireless_dev *wdev,
					       const void *data,
					       int data_len)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(wdev->netdev);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_ROAM_EVENTS_MAX + 1];
	QDF_STATUS status;
	int ret;
	uint8_t config, state, param = 0;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret != 0) {
		hdd_err("Invalid hdd_ctx");
		return ret;
	}

	ret = hdd_validate_adapter(adapter);
	if (ret != 0) {
		hdd_err("Invalid adapter");
		return ret;
	}

	if (adapter->device_mode != QDF_STA_MODE) {
		hdd_err("STATS supported in only STA mode!");
		return -EINVAL;
	}

	if (wlan_cfg80211_nla_parse(tb, QCA_WLAN_VENDOR_ATTR_ROAM_EVENTS_MAX,
				    data, data_len, set_roam_events_policy)) {
		hdd_err("Invalid ATTR");
		return -EINVAL;
	}

	if (!tb[EVENTS_CONFIGURE]) {
		hdd_err("roam events configure not present");
		return -EINVAL;
	}

	config = nla_get_u8(tb[EVENTS_CONFIGURE]);
	hdd_debug("roam stats configured: %d", config);

	if (!tb[SUSPEND_STATE]) {
		hdd_debug("suspend state not present");
		param = config ? ROAM_RT_STATS_ENABLED : ROAM_RT_STATS_DISABLED;
	} else if (config == ROAM_RT_STATS_ENABLED) {
		state = nla_get_flag(tb[SUSPEND_STATE]);
		hdd_debug("Suspend state configured: %d", state);
		param = ROAM_RT_STATS_ENABLED |
			ROAM_RT_STATS_ENABLED_IN_SUSPEND_MODE;
	}

	hdd_debug("roam events param: %d", param);
	ucfg_cm_update_roam_rt_stats(hdd_ctx->psoc,
				     param, ROAM_RT_STATS_ENABLE);

	if (param == (ROAM_RT_STATS_ENABLED |
		      ROAM_RT_STATS_ENABLED_IN_SUSPEND_MODE)) {
		ucfg_pmo_enable_wakeup_event(hdd_ctx->psoc, adapter->vdev_id,
					     WOW_ROAM_STATS_EVENT);
		ucfg_cm_update_roam_rt_stats(hdd_ctx->psoc,
					     ROAM_RT_STATS_ENABLED,
					     ROAM_RT_STATS_SUSPEND_MODE_ENABLE);
	} else if (ucfg_cm_get_roam_rt_stats(hdd_ctx->psoc,
					  ROAM_RT_STATS_SUSPEND_MODE_ENABLE)) {
		ucfg_pmo_disable_wakeup_event(hdd_ctx->psoc, adapter->vdev_id,
					      WOW_ROAM_STATS_EVENT);
		ucfg_cm_update_roam_rt_stats(hdd_ctx->psoc,
					     ROAM_RT_STATS_DISABLED,
					     ROAM_RT_STATS_SUSPEND_MODE_ENABLE);
	}

	status = ucfg_cm_roam_send_rt_stats_config(hdd_ctx->pdev,
						   adapter->vdev_id, param);

	return qdf_status_to_os_return(status);
}

#undef EVENTS_CONFIGURE
#undef SUSPEND_STATE

/**
 * wlan_hdd_cfg80211_set_roam_events() - set roam stats
 * @wiphy: wiphy pointer
 * @wdev: pointer to struct wireless_dev
 * @data: pointer to incoming NL vendor data
 * @data_len: length of @data
 *
 * Return: 0 on success; error number otherwise.
 */
static int wlan_hdd_cfg80211_set_roam_events(struct wiphy *wiphy,
					     struct wireless_dev *wdev,
					     const void *data,
					     int data_len)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_set_roam_events(wiphy, wdev,
						    data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}
#endif

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
	mac_handle_t mac_handle;
	struct get_chain_rssi_req_params req_msg;
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_MAX + 1];
	QDF_STATUS status;
	int retval;
	void *cookie;
	struct osif_request *request;
	struct chain_rssi_priv *priv;
	static const struct osif_request_params params = {
		.priv_size = sizeof(*priv),
		.timeout_ms = WLAN_WAIT_TIME_STATS,
	};

	hdd_enter();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	retval = wlan_hdd_validate_context(hdd_ctx);
	if (0 != retval)
		return retval;

	if (wlan_cfg80211_nla_parse(tb, QCA_WLAN_VENDOR_ATTR_MAX,
				    data, data_len, get_chain_rssi_policy)) {
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
	req_msg.session_id = adapter->vdev_id;

	request = osif_request_alloc(&params);
	if (!request) {
		hdd_err("Request allocation failure");
		return -ENOMEM;
	}
	cookie = osif_request_cookie(request);

	mac_handle = hdd_ctx->mac_handle;
	status = sme_get_chain_rssi(mac_handle,
				    &req_msg,
				    hdd_get_chain_rssi_cb,
				    cookie);
	if (QDF_STATUS_SUCCESS != status) {
		hdd_err("Unable to get chain rssi");
		retval = qdf_status_to_os_return(status);
	} else {
		retval = osif_request_wait_for_response(request);
		if (retval) {
			hdd_err("Target response timed out");
		} else {
			priv = osif_request_priv(request);
			retval = hdd_post_get_chain_rssi_rsp(hdd_ctx,
					&priv->chain_rssi);
			if (retval)
				hdd_err("Failed to post chain rssi");
		}
	}
	osif_request_put(request);

	hdd_exit();
	return retval;
}

#ifdef WLAN_FEATURE_GET_USABLE_CHAN_LIST
/**
 * wlan_hdd_cfg80211_get_usable_channel() - get chain rssi
 * @wiphy: wiphy pointer
 * @wdev: pointer to struct wireless_dev
 * @data: pointer to incoming NL vendor data
 * @data_len: length of @data
 *
 * Return: 0 on success; error number otherwise.
 */
static int wlan_hdd_cfg80211_get_usable_channel(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_get_usable_channel(wiphy, wdev,
						       data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}
#else
static int wlan_hdd_cfg80211_get_usable_channel(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len)
{
	hdd_debug("get usable channel feature not supported");
	return -EPERM;
}
#endif

#ifdef WLAN_FEATURE_PKT_CAPTURE

/**
 * __wlan_hdd_cfg80211_set_monitor_mode() - Wifi monitor mode configuration
 * vendor command
 * @wiphy: wiphy device pointer
 * @wdev: wireless device pointer
 * @data: Vendor command data buffer
 * @data_len: Buffer length
 *
 * Handles .
 *
 * Return: 0 for Success and negative value for failure
 */
static int
__wlan_hdd_cfg80211_set_monitor_mode(struct wiphy *wiphy,
				     struct wireless_dev *wdev,
				     const void *data, int data_len)
{
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx  = wiphy_priv(wiphy);
	int errno;
	QDF_STATUS status;

	if (hdd_get_conparam() == QDF_GLOBAL_FTM_MODE) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	if (!ucfg_pkt_capture_get_mode(hdd_ctx->psoc) ||
	    !hdd_is_pkt_capture_mon_enable(adapter))
		return -EPERM;

	errno = hdd_validate_adapter(adapter);
	if (errno)
		return errno;

	status = os_if_monitor_mode_configure(adapter, data, data_len);

	return qdf_status_to_os_return(status);
}

/**
 * wlan_hdd_cfg80211_set_monitor_mode() - set monitor mode
 * @wiphy: wiphy pointer
 * @wdev: pointer to struct wireless_dev
 * @data: pointer to incoming NL vendor data
 * @data_len: length of @data
 *
 * Return: 0 on success; error number otherwise.
 */
static int wlan_hdd_cfg80211_set_monitor_mode(struct wiphy *wiphy,
					      struct wireless_dev *wdev,
					      const void *data, int data_len)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	hdd_enter_dev(wdev->netdev);

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_set_monitor_mode(wiphy, wdev,
						     data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	hdd_exit();

	return errno;
}

#endif

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
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_get_chain_rssi(wiphy, wdev, data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
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

	hdd_enter();
	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	if (!hdd_ctx)
		return -EINVAL;

	err = wlan_hdd_validate_context(hdd_ctx);
	if (0 != err)
		return err;

	conn_count = policy_mgr_get_connection_info(hdd_ctx->psoc, info);
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
		if (wlan_hdd_fill_mac_info(skb, info, mac_id, conn_count)) {
			kfree_skb(skb);
			return -EINVAL;
		}
	}

	nla_nest_end(skb, attr);

	cfg80211_vendor_event(skb, GFP_KERNEL);
	hdd_exit();

	return err;
}


/* Short name for QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_VALID_CHANNELS command */

#define EXTSCAN_CONFIG_MAX \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_MAX
#define EXTSCAN_CONFIG_REQUEST_ID \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_REQUEST_ID
#define EXTSCAN_CONFIG_WIFI_BAND \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_GET_VALID_CHANNELS_CONFIG_PARAM_WIFI_BAND
#define EXTSCAN_CONFIG_MAX_CHANNELS \
QCA_WLAN_VENDOR_ATTR_EXTSCAN_GET_VALID_CHANNELS_CONFIG_PARAM_MAX_CHANNELS
#define EXTSCAN_RESULTS_NUM_CHANNELS \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_NUM_CHANNELS
#define EXTSCAN_RESULTS_CHANNELS \
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CHANNELS

static const struct nla_policy
wlan_hdd_extscan_get_valid_channels_policy[EXTSCAN_CONFIG_MAX + 1] = {
	[EXTSCAN_CONFIG_REQUEST_ID] = {.type = NLA_U32},
	[EXTSCAN_CONFIG_WIFI_BAND] = {.type = NLA_U32},
	[EXTSCAN_CONFIG_MAX_CHANNELS] = {.type = NLA_U32},
};

/**
 * hdd_remove_passive_channels () - remove passive channels
 * @wiphy: Pointer to wireless phy
 * @chan_list: channel list
 * @num_channels: number of channels
 *
 * Return: none
 */
static void hdd_remove_passive_channels(struct wiphy *wiphy,
					uint32_t *chan_list,
					uint8_t *num_channels)
{
	uint8_t num_chan_temp = 0;
	int i, j, k;

	for (i = 0; i < *num_channels; i++)
		for (j = 0; j < HDD_NUM_NL80211_BANDS; j++) {
			if (!wiphy->bands[j])
				continue;
			for (k = 0; k < wiphy->bands[j]->n_channels; k++) {
				if ((chan_list[i] ==
				     wiphy->bands[j]->channels[k].center_freq)
				    && (!(wiphy->bands[j]->channels[k].flags &
				       IEEE80211_CHAN_PASSIVE_SCAN))
				) {
					chan_list[num_chan_temp] = chan_list[i];
					num_chan_temp++;
				}
			}
		}

	*num_channels = num_chan_temp;
}

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
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	uint32_t chan_list[CFG_VALID_CHANNEL_LIST_LEN] = {0};
	uint8_t num_channels  = 0, i, buf[256] = {0};
	struct nlattr *tb[EXTSCAN_CONFIG_MAX + 1];
	uint32_t request_id, max_channels;
	tWifiBand wifi_band;
	QDF_STATUS status;
	struct sk_buff *reply_skb;
	int ret, len = 0;

	/* ENTER_DEV() intentionally not used in a frequently invoked API */

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return -EINVAL;

	if (wlan_cfg80211_nla_parse(tb, EXTSCAN_CONFIG_MAX, data, data_len,
				  wlan_hdd_extscan_get_valid_channels_policy)) {
		hdd_err("Invalid ATTR");
		return -EINVAL;
	}

	/* Parse and fetch request Id */
	if (!tb[EXTSCAN_CONFIG_REQUEST_ID]) {
		hdd_err("attr request id failed");
		return -EINVAL;
	}
	request_id = nla_get_u32(tb[EXTSCAN_CONFIG_REQUEST_ID]);

	/* Parse and fetch wifi band */
	if (!tb[EXTSCAN_CONFIG_WIFI_BAND]) {
		hdd_err("attr wifi band failed");
		return -EINVAL;
	}
	wifi_band = nla_get_u32(tb[EXTSCAN_CONFIG_WIFI_BAND]);
	if (!tb[EXTSCAN_CONFIG_MAX_CHANNELS]) {
		hdd_err("attr max channels failed");
		return -EINVAL;
	}
	max_channels = nla_get_u32(tb[EXTSCAN_CONFIG_MAX_CHANNELS]);

	if (max_channels > CFG_VALID_CHANNEL_LIST_LEN) {
		hdd_err("Max channels %d exceeded Valid channel list len %d",
			max_channels, CFG_VALID_CHANNEL_LIST_LEN);
		return -EINVAL;
	}

	hdd_err("Req Id: %u Wifi band: %d Max channels: %d", request_id,
		wifi_band, max_channels);
	status = sme_get_valid_channels_by_band(hdd_ctx->mac_handle,
						wifi_band, chan_list,
						&num_channels);
	if (QDF_STATUS_SUCCESS != status) {
		hdd_err("sme_get_valid_channels_by_band failed (err=%d)",
		        status);
		return -EINVAL;
	}

	num_channels = QDF_MIN(num_channels, max_channels);

	if ((QDF_SAP_MODE == adapter->device_mode) ||
	    !strncmp(hdd_get_fwpath(), "ap", 2))
		hdd_remove_passive_channels(wiphy, chan_list,
					    &num_channels);

	hdd_debug("Number of channels: %d", num_channels);
	for (i = 0; i < num_channels; i++)
		len += scnprintf(buf + len, sizeof(buf) - len,
				 "%u ", chan_list[i]);

	hdd_debug("Channels: %s", buf);

	reply_skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, sizeof(u32) +
							sizeof(u32) *
							num_channels +
							NLMSG_HDRLEN);

	if (reply_skb) {
		if (nla_put_u32(
			reply_skb,
			EXTSCAN_RESULTS_NUM_CHANNELS,
			num_channels) ||
		    nla_put(
			reply_skb,
			EXTSCAN_RESULTS_CHANNELS,
			sizeof(u32) * num_channels, chan_list)) {
			hdd_err("nla put fail");
			kfree_skb(reply_skb);
			return -EINVAL;
		}
		ret = cfg80211_vendor_cmd_reply(reply_skb);
		return ret;
	}

	hdd_err("valid channels: buffer alloc fail");
	return -EINVAL;
}

#undef EXTSCAN_CONFIG_MAX
#undef EXTSCAN_CONFIG_REQUEST_ID
#undef EXTSCAN_CONFIG_WIFI_BAND
#undef ETCAN_CONFIG_MAX_CHANNELS
#undef EXTSCAN_RESULTS_NUM_CHANNELS
#undef EXTSCAN_RESULTS_CHANNELS

/**
 * wlan_hdd_cfg80211_extscan_get_valid_channels() - get ext scan valid channels
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: 0 on success, negative errno on failure
 */
static int wlan_hdd_cfg80211_extscan_get_valid_channels(
					struct wiphy *wiphy,
					struct wireless_dev *wdev,
					const void *data, int data_len)
{
	struct osif_psoc_sync *psoc_sync;
	int errno;

	errno = osif_psoc_sync_op_start(wiphy_dev(wiphy), &psoc_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_extscan_get_valid_channels(wiphy, wdev,
							       data, data_len);

	osif_psoc_sync_op_stop(psoc_sync);

	return errno;
}

#ifdef FEATURE_RADAR_HISTORY
static uint32_t get_radar_history_evt_len(uint32_t count)
{
	uint32_t data_len = NLMSG_HDRLEN;

	data_len +=
	/* nested attribute hdr QCA_WLAN_VENDOR_ATTR_RADAR_HISTORY_ENTRIES */
		nla_total_size(count *
			       (nla_total_size(
				     /* channel frequency */
				     nla_total_size(sizeof(uint32_t)) +
				     /* timestamp */
				     nla_total_size(sizeof(uint64_t)) +
				     /* radar detected flag */
				     nla_total_size(0))));

	return data_len;
}

/**
 * __wlan_hdd_cfg80211_radar_history () - Get radar history
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: 0 on success, negative errno on failure
 */
static int
__wlan_hdd_cfg80211_get_radar_history(struct wiphy *wiphy,
				      struct wireless_dev
				      *wdev, const void *data,
				      int data_len)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	QDF_STATUS status;
	struct sk_buff *reply_skb = NULL;
	int ret, len;
	struct dfs_radar_history *radar_history = NULL;
	uint32_t hist_count = 0;
	int idx;
	struct nlattr *ch_array, *ch_element;

	if (hdd_get_conparam() == QDF_GLOBAL_FTM_MODE) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return -EINVAL;

	status = wlansap_query_radar_history(hdd_ctx->mac_handle,
					     &radar_history, &hist_count);
	if (!QDF_IS_STATUS_SUCCESS(status))
		return -EINVAL;

	len = get_radar_history_evt_len(hist_count);
	reply_skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, len);
	if (!reply_skb) {
		ret = -ENOMEM;
		goto err;
	}

	ch_array = nla_nest_start(
			reply_skb, QCA_WLAN_VENDOR_ATTR_RADAR_HISTORY_ENTRIES);
	if (!ch_array) {
		ret = -ENOMEM;
		goto err;
	}

	for (idx = 0; idx < hist_count; idx++) {
		ch_element = nla_nest_start(reply_skb, idx);
		if (!ch_element) {
			ret = -ENOMEM;
			goto err;
		}

		if (nla_put_u32(reply_skb,
				QCA_WLAN_VENDOR_ATTR_RADAR_HISTORY_FREQ,
				radar_history[idx].ch_freq)) {
			ret = -ENOMEM;
			goto err;
		}

		if (wlan_cfg80211_nla_put_u64(
			reply_skb,
			QCA_WLAN_VENDOR_ATTR_RADAR_HISTORY_TIMESTAMP,
			radar_history[idx].time)) {
			ret = -ENOMEM;
			goto err;
		}

		if (radar_history[idx].radar_found &&
		    nla_put_flag(
			reply_skb,
			QCA_WLAN_VENDOR_ATTR_RADAR_HISTORY_DETECTED)) {
			ret = -ENOMEM;
			goto err;
		}

		nla_nest_end(reply_skb, ch_element);
	}
	nla_nest_end(reply_skb, ch_array);
	qdf_mem_free(radar_history);

	ret = cfg80211_vendor_cmd_reply(reply_skb);
	hdd_debug("get radar history count %d, ret %d", hist_count, ret);

	return ret;
err:
	qdf_mem_free(radar_history);
	if (reply_skb)
		kfree_skb(reply_skb);
	hdd_debug("get radar history error %d", ret);

	return ret;
}

/**
 * wlan_hdd_cfg80211_get_radar_history() - get radar history
 * @wiphy: wiphy pointer
 * @wdev: pointer to struct wireless_dev
 * @data: pointer to incoming NL vendor data
 * @data_len: length of @data
 *
 * Return: 0 on success; error number otherwise.
 */
static int wlan_hdd_cfg80211_get_radar_history(struct wiphy *wiphy,
					       struct wireless_dev *wdev,
					       const void *data,
					       int data_len)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_get_radar_history(wiphy, wdev,
						      data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}

#define FEATURE_RADAR_HISTORY_VENDOR_COMMANDS				\
{									\
	.info.vendor_id = QCA_NL80211_VENDOR_ID,			\
	.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_GET_RADAR_HISTORY,	\
	.flags = WIPHY_VENDOR_CMD_NEED_WDEV |				\
		WIPHY_VENDOR_CMD_NEED_NETDEV |				\
		WIPHY_VENDOR_CMD_NEED_RUNNING,				\
	.doit = wlan_hdd_cfg80211_get_radar_history,			\
	vendor_command_policy(VENDOR_CMD_RAW_DATA, 0)			\
},
#else
#define FEATURE_RADAR_HISTORY_VENDOR_COMMANDS
#endif

const struct wiphy_vendor_command hdd_wiphy_vendor_commands[] = {
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_DFS_CAPABILITY,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = is_driver_dfs_capable,
		vendor_command_policy(VENDOR_CMD_RAW_DATA, 0)
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_VALID_CHANNELS,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV |
			 WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_extscan_get_valid_channels,
		vendor_command_policy(
				wlan_hdd_extscan_get_valid_channels_policy,
				EXTSCAN_PARAM_MAX)
	},
#ifdef WLAN_FEATURE_STATS_EXT
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_STATS_EXT,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV | WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_stats_ext_request,
		vendor_command_policy(VENDOR_CMD_RAW_DATA, 0)
	},
#endif
	FEATURE_EXTSCAN_VENDOR_COMMANDS

	FEATURE_LL_STATS_VENDOR_COMMANDS

	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_GET_SUPPORTED_FEATURES,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = wlan_hdd_cfg80211_get_supported_features,
		vendor_command_policy(VENDOR_CMD_RAW_DATA, 0)
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_SCANNING_MAC_OUI,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_set_scanning_mac_oui,
		vendor_command_policy(VENDOR_CMD_RAW_DATA, 0)
	},

	FEATURE_CONCURRENCY_MATRIX_VENDOR_COMMANDS

	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_NO_DFS_FLAG,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_disable_dfs_chan_scan,
		vendor_command_policy(wlan_hdd_set_no_dfs_flag_config_policy,
				      QCA_WLAN_VENDOR_ATTR_SET_NO_DFS_FLAG_MAX)
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_WISA,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_handle_wisa_cmd,
		vendor_command_policy(wlan_hdd_wisa_cmd_policy,
				      QCA_WLAN_VENDOR_ATTR_WISA_MAX)
	},

	FEATURE_STATION_INFO_VENDOR_COMMANDS

	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_DO_ACS,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
				WIPHY_VENDOR_CMD_NEED_NETDEV |
				WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_do_acs,
		vendor_command_policy(wlan_hdd_cfg80211_do_acs_policy,
				      QCA_WLAN_VENDOR_ATTR_ACS_MAX)
	},

	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_GET_FEATURES,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = wlan_hdd_cfg80211_get_features,
		vendor_command_policy(VENDOR_CMD_RAW_DATA, 0)
	},
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_KEY_MGMT_SET_KEY,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_keymgmt_set_key,
		vendor_command_policy(VENDOR_CMD_RAW_DATA, 0)
	},
#endif
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_GET_WIFI_INFO,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = wlan_hdd_cfg80211_get_wifi_info,
		vendor_command_policy(qca_wlan_vendor_get_wifi_info_policy,
				      QCA_WLAN_VENDOR_ATTR_WIFI_INFO_GET_MAX)
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_SET_WIFI_CONFIGURATION,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_wifi_configuration_set,
		vendor_command_policy(wlan_hdd_wifi_config_policy,
				      QCA_WLAN_VENDOR_ATTR_CONFIG_MAX)
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_GET_WIFI_CONFIGURATION,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_wifi_configuration_get,
		vendor_command_policy(wlan_hdd_wifi_config_policy,
				      QCA_WLAN_VENDOR_ATTR_CONFIG_MAX)
	},

	FEATURE_VENDOR_SUBCMD_WIFI_TEST_CONFIGURATION
#ifdef WLAN_SUPPORT_TWT
	FEATURE_VENDOR_SUBCMD_WIFI_CONFIG_TWT
#endif

	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_ROAM,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_set_ext_roam_params,
		vendor_command_policy(wlan_hdd_set_roam_param_policy,
				      QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_MAX)
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_RATEMASK_CONFIG,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_set_ratemask_config,
		vendor_command_policy(wlan_hdd_set_ratemask_param_policy,
				      QCA_WLAN_VENDOR_ATTR_RATEMASK_PARAMS_MAX)
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_WIFI_LOGGER_START,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_wifi_logger_start,
		vendor_command_policy(
				qca_wlan_vendor_wifi_logger_start_policy,
				QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_START_MAX)
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_GET_RING_DATA,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV,
		.doit = wlan_hdd_cfg80211_wifi_logger_get_ring_data,
		vendor_command_policy(
			qca_wlan_vendor_wifi_logger_get_ring_data_policy,
			QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_GET_RING_DATA_MAX)
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_GET_PREFERRED_FREQ_LIST,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_get_preferred_freq_list,
		vendor_command_policy(
			get_preferred_freq_list_policy,
			QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST_MAX)
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_SET_PROBABLE_OPER_CHANNEL,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_set_probable_oper_channel,
		vendor_command_policy(
				set_probable_oper_channel_policy,
				QCA_WLAN_VENDOR_ATTR_PROBABLE_OPER_CHANNEL_MAX)
	},

	FEATURE_HANDLE_TSF_VENDOR_COMMANDS

#ifdef FEATURE_WLAN_TDLS
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_TDLS_GET_CAPABILITIES,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_get_tdls_capabilities,
		vendor_command_policy(VENDOR_CMD_RAW_DATA, 0)
	},
#endif
#ifdef WLAN_FEATURE_OFFLOAD_PACKETS
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_OFFLOADED_PACKETS,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_offloaded_packets,
		vendor_command_policy(offloaded_packet_policy,
				      QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_MAX)
	},
#endif
	FEATURE_RSSI_MONITOR_VENDOR_COMMANDS
	FEATURE_OEM_DATA_VENDOR_COMMANDS
	FEATURE_INTEROP_ISSUES_AP_VENDOR_COMMANDS

#ifdef WLAN_NS_OFFLOAD
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_ND_OFFLOAD,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV |
			 WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_set_ns_offload,
		vendor_command_policy(ns_offload_set_policy,
				      QCA_WLAN_VENDOR_ATTR_ND_OFFLOAD_MAX)
	},
#endif /* WLAN_NS_OFFLOAD */
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_GET_LOGGER_FEATURE_SET,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = wlan_hdd_cfg80211_get_logger_supp_feature,
		vendor_command_policy(get_logger_set_policy,
				      QCA_WLAN_VENDOR_ATTR_LOGGER_MAX)
	},

	FEATURE_TRIGGER_SCAN_VENDOR_COMMANDS

	/* Vendor abort scan */
	FEATURE_ABORT_SCAN_VENDOR_COMMANDS

	/* OCB commands */
	FEATURE_OCB_VENDOR_COMMANDS

	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_LINK_PROPERTIES,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV |
			 WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_get_link_properties,
		vendor_command_policy(wlan_hdd_get_link_properties_policy,
				      QCA_WLAN_VENDOR_ATTR_MAX)
	},

	FEATURE_OTA_TEST_VENDOR_COMMANDS

	FEATURE_LFR_SUBNET_DETECT_VENDOR_COMMANDS

	FEATURE_TX_POWER_VENDOR_COMMANDS

	FEATURE_APF_OFFLOAD_VENDOR_COMMANDS

	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_ACS_POLICY,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_acs_dfs_mode,
		vendor_command_policy(wlan_hdd_set_acs_dfs_config_policy,
				      QCA_WLAN_VENDOR_ATTR_ACS_DFS_MAX)
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_STA_CONNECT_ROAM_POLICY,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_sta_roam_policy,
		vendor_command_policy(
			wlan_hdd_set_sta_roam_config_policy,
			QCA_WLAN_VENDOR_ATTR_STA_CONNECT_ROAM_POLICY_MAX)
	},

	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_CONCURRENT_MULTI_STA_POLICY,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = wlan_hdd_cfg80211_dual_sta_policy,
		vendor_command_policy(
			wlan_hdd_set_dual_sta_policy,
			QCA_WLAN_VENDOR_ATTR_CONCURRENT_STA_POLICY_MAX)
	},

#ifdef FEATURE_WLAN_CH_AVOID
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_AVOID_FREQUENCY,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_avoid_freq,
		vendor_command_policy(VENDOR_CMD_RAW_DATA, 0)
	},
#endif
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_SET_SAP_CONFIG,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_sap_configuration_set,
		vendor_command_policy(wlan_hdd_sap_config_policy,
				      QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_MAX)
	},

	FEATURE_P2P_LISTEN_OFFLOAD_VENDOR_COMMANDS

	FEATURE_SAP_COND_CHAN_SWITCH_VENDOR_COMMANDS
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_GET_WAKE_REASON_STATS,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV |
			 WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_get_wakelock_stats,
		vendor_command_policy(VENDOR_CMD_RAW_DATA, 0)
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_GET_BUS_SIZE,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV |
			 WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_get_bus_size,
		vendor_command_policy(VENDOR_CMD_RAW_DATA, 0)
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_EXTERNAL_ACS,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV |
			 WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_update_vendor_channel,
		vendor_command_policy(acs_chan_config_policy,
				      QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_MAX)
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_SETBAND,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
					WIPHY_VENDOR_CMD_NEED_NETDEV |
					WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_setband,
		vendor_command_policy(setband_policy, QCA_WLAN_VENDOR_ATTR_MAX)
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_GETBAND,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV |
			 WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_getband,
		vendor_command_policy(VENDOR_CMD_RAW_DATA, 0)
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_ROAMING,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_set_fast_roaming,
		vendor_command_policy(qca_wlan_vendor_attr,
				      QCA_WLAN_VENDOR_ATTR_MAX)
	},
	FEATURE_DISA_VENDOR_COMMANDS
	FEATURE_TDLS_VENDOR_COMMANDS
	FEATURE_SAR_LIMITS_VENDOR_COMMANDS
	BCN_RECV_FEATURE_VENDOR_COMMANDS
	FEATURE_VENDOR_SUBCMD_SET_TRACE_LEVEL
#ifdef WLAN_FEATURE_LINK_LAYER_STATS
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd =
			QCA_NL80211_VENDOR_SUBCMD_LL_STATS_EXT,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
				 WIPHY_VENDOR_CMD_NEED_NETDEV |
				 WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_ll_stats_ext_set_param,
		vendor_command_policy(qca_wlan_vendor_ll_ext_policy,
				      QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_MAX)
	},
#endif
	FEATURE_VENDOR_SUBCMD_NUD_STATS_SET
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_NUD_STATS_GET,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_get_nud_stats,
		vendor_command_policy(VENDOR_CMD_RAW_DATA, 0)
	},

	FEATURE_BSS_TRANSITION_VENDOR_COMMANDS
	FEATURE_SPECTRAL_SCAN_VENDOR_COMMANDS
	FEATURE_CFR_VENDOR_COMMANDS
	FEATURE_11AX_VENDOR_COMMANDS

	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_GET_CHAIN_RSSI,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV |
			WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = wlan_hdd_cfg80211_get_chain_rssi,
		vendor_command_policy(get_chain_rssi_policy,
				      QCA_WLAN_VENDOR_ATTR_MAX)
	},
	{
		.info.vendor_id = QCA_NL80211_VENDOR_ID,
		.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_USABLE_CHANNELS,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = wlan_hdd_cfg80211_get_usable_channel,
		vendor_command_policy(get_usable_channel_policy,
				      QCA_WLAN_VENDOR_ATTR_MAX)
	},

	FEATURE_ACTIVE_TOS_VENDOR_COMMANDS
	FEATURE_NAN_VENDOR_COMMANDS
	FEATURE_FW_STATE_COMMANDS
	FEATURE_COEX_CONFIG_COMMANDS
	FEATURE_MPTA_HELPER_COMMANDS
	FEATURE_HW_CAPABILITY_COMMANDS
	FEATURE_THERMAL_VENDOR_COMMANDS
	FEATURE_BTC_CHAIN_MODE_COMMANDS
	FEATURE_WMM_COMMANDS
	FEATURE_GPIO_CFG_VENDOR_COMMANDS
	FEATURE_MEDIUM_ASSESS_VENDOR_COMMANDS
	FEATURE_RADAR_HISTORY_VENDOR_COMMANDS
	FEATURE_AVOID_FREQ_EXT_VENDOR_COMMANDS
	FEATURE_MDNS_OFFLOAD_VENDOR_COMMANDS

#ifdef WLAN_FEATURE_PKT_CAPTURE
	FEATURE_MONITOR_MODE_VENDOR_COMMANDS
#endif

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	{
	.info.vendor_id = QCA_NL80211_VENDOR_ID,
	.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_ROAM_EVENTS,
	.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
		 WIPHY_VENDOR_CMD_NEED_NETDEV |
		 WIPHY_VENDOR_CMD_NEED_RUNNING,
	.doit = wlan_hdd_cfg80211_set_roam_events,
	vendor_command_policy(set_roam_events_policy,
			      QCA_WLAN_VENDOR_ATTR_ROAM_EVENTS_MAX)
	},
#endif
	FEATURE_COAP_OFFLOAD_COMMANDS
	FEATURE_PEER_FLUSH_VENDOR_COMMANDS
};

struct hdd_context *hdd_cfg80211_wiphy_alloc(void)
{
	struct wiphy *wiphy;
	struct hdd_context *hdd_ctx;

	hdd_enter();

	wiphy = wiphy_new(&wlan_hdd_cfg80211_ops, sizeof(*hdd_ctx));
	if (!wiphy) {
		hdd_err("failed to allocate wiphy!");
		return NULL;
	}

	hdd_ctx = wiphy_priv(wiphy);
	hdd_ctx->wiphy = wiphy;

	return hdd_ctx;
}

int wlan_hdd_cfg80211_update_band(struct hdd_context *hdd_ctx,
				  struct wiphy *wiphy,
				  enum band_info new_band)
{
	int i, j;
	enum channel_state channel_state;

	hdd_enter();

	for (i = 0; i < HDD_NUM_NL80211_BANDS; i++) {

		if (!wiphy->bands[i])
			continue;

		for (j = 0; j < wiphy->bands[i]->n_channels; j++) {
			struct ieee80211_supported_band *band = wiphy->bands[i];

			channel_state = wlan_reg_get_channel_state_for_freq(
					hdd_ctx->pdev,
					band->channels[j].center_freq);

			if (HDD_NL80211_BAND_2GHZ == i &&
			    BAND_5G == new_band) {
				/* 5G only */
#ifdef WLAN_ENABLE_SOCIAL_CHANNELS_5G_ONLY
				/* Enable Social channels for P2P */
				if (WLAN_HDD_IS_SOCIAL_CHANNEL
					    (band->channels[j].center_freq)
				    && CHANNEL_STATE_ENABLE ==
				    channel_state)
					band->channels[j].flags &=
						~IEEE80211_CHAN_DISABLED;
				else
#endif
				band->channels[j].flags |=
					IEEE80211_CHAN_DISABLED;
				continue;
			} else if (HDD_NL80211_BAND_5GHZ == i &&
				   BAND_2G == new_band) {
				/* 2G only */
				band->channels[j].flags |=
					IEEE80211_CHAN_DISABLED;
				continue;
			}

			if (CHANNEL_STATE_DISABLE != channel_state)
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
	wiphy->features |= NL80211_FEATURE_SCAN_RANDOM_MAC_ADDR;
	wiphy->features |= NL80211_FEATURE_SCHED_SCAN_RANDOM_MAC_ADDR;
}
#else
static void wlan_hdd_cfg80211_scan_randomization_init(struct wiphy *wiphy)
{
}
#endif

#define WLAN_HDD_MAX_NUM_CSA_COUNTERS 2

#if defined(CFG80211_RAND_TA_FOR_PUBLIC_ACTION_FRAME) || \
		(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0))
/**
 * wlan_hdd_cfg80211_action_frame_randomization_init() - Randomize SA of MA
 * frames
 * @wiphy: Pointer to wiphy
 *
 * This function is used to indicate the support of source mac address
 * randomization of management action frames
 *
 * Return: None
 */
static void
wlan_hdd_cfg80211_action_frame_randomization_init(struct wiphy *wiphy)
{
	wiphy_ext_feature_set(wiphy, NL80211_EXT_FEATURE_MGMT_TX_RANDOM_TA);
}
#else
static void
wlan_hdd_cfg80211_action_frame_randomization_init(struct wiphy *wiphy)
{
}
#endif

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

#if defined (CFG80211_SCAN_DBS_CONTROL_SUPPORT) || \
	    (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 16, 0))
static void wlan_hdd_cfg80211_set_wiphy_scan_flags(struct wiphy *wiphy)
{
	wiphy_ext_feature_set(wiphy, NL80211_EXT_FEATURE_LOW_SPAN_SCAN);
	wiphy_ext_feature_set(wiphy, NL80211_EXT_FEATURE_LOW_POWER_SCAN);
	wiphy_ext_feature_set(wiphy, NL80211_EXT_FEATURE_HIGH_ACCURACY_SCAN);
}
#else
static void wlan_hdd_cfg80211_set_wiphy_scan_flags(struct wiphy *wiphy)
{
}
#endif

#if defined(CFG80211_BIGTK_CONFIGURATION_SUPPORT) || \
	   (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0))
static void wlan_hdd_cfg80211_set_bigtk_flags(struct wiphy *wiphy)
{
	wiphy_ext_feature_set(wiphy, NL80211_EXT_FEATURE_BEACON_PROTECTION);
}
#else
static void wlan_hdd_cfg80211_set_bigtk_flags(struct wiphy *wiphy)
{
}
#endif

#if defined(CFG80211_OCV_CONFIGURATION_SUPPORT) || \
	   (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0))
static void wlan_hdd_cfg80211_set_ocv_flags(struct wiphy *wiphy)
{
	wiphy_ext_feature_set(wiphy,
			      NL80211_EXT_FEATURE_OPERATING_CHANNEL_VALIDATION);
}
#else
static void wlan_hdd_cfg80211_set_ocv_flags(struct wiphy *wiphy)
{
}
#endif

#if defined(CFG80211_SCAN_OCE_CAPABILITY_SUPPORT) || \
	   (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0))
static void wlan_hdd_cfg80211_set_wiphy_oce_scan_flags(struct wiphy *wiphy)
{
	wiphy_ext_feature_set(wiphy, NL80211_EXT_FEATURE_FILS_MAX_CHANNEL_TIME);
	wiphy_ext_feature_set(wiphy,
			      NL80211_EXT_FEATURE_ACCEPT_BCAST_PROBE_RESP);
	wiphy_ext_feature_set(wiphy,
			      NL80211_EXT_FEATURE_OCE_PROBE_REQ_HIGH_TX_RATE);
	wiphy_ext_feature_set(
		wiphy, NL80211_EXT_FEATURE_OCE_PROBE_REQ_DEFERRAL_SUPPRESSION);
}
#else
static void wlan_hdd_cfg80211_set_wiphy_oce_scan_flags(struct wiphy *wiphy)
{
}
#endif

#if defined(WLAN_FEATURE_SAE) && \
		(defined(CFG80211_EXTERNAL_AUTH_SUPPORT) || \
		LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0))
/**
 * wlan_hdd_cfg80211_set_wiphy_sae_feature() - Indicates support of SAE feature
 * @wiphy: Pointer to wiphy
 *
 * This function is used to indicate the support of SAE
 *
 * Return: None
 */
static void wlan_hdd_cfg80211_set_wiphy_sae_feature(struct wiphy *wiphy)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);

	if (ucfg_fwol_get_sae_enable(hdd_ctx->psoc))
		wiphy->features |= NL80211_FEATURE_SAE;
}
#else
static void wlan_hdd_cfg80211_set_wiphy_sae_feature(struct wiphy *wiphy)
{
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)) || \
	defined(CFG80211_DFS_OFFLOAD_BACKPORT)
static void wlan_hdd_cfg80211_set_dfs_offload_feature(struct wiphy *wiphy)
{
	wiphy_ext_feature_set(wiphy, NL80211_EXT_FEATURE_DFS_OFFLOAD);
}
#else
static void wlan_hdd_cfg80211_set_dfs_offload_feature(struct wiphy *wiphy)
{
	wiphy->flags |= WIPHY_FLAG_DFS_OFFLOAD;
}
#endif

#ifdef WLAN_FEATURE_DSRC
static void wlan_hdd_get_num_dsrc_ch_and_len(struct hdd_config *hdd_cfg,
					     int *num_ch, int *ch_len)
{
	*num_ch = QDF_ARRAY_SIZE(hdd_channels_dot11p);
	*ch_len = sizeof(hdd_channels_dot11p);
}

static void wlan_hdd_copy_dsrc_ch(char *ch_ptr, int ch_arr_len)
{
	if (!ch_arr_len)
		return;
	qdf_mem_copy(ch_ptr, &hdd_channels_dot11p[0], ch_arr_len);
}

static void wlan_hdd_get_num_srd_ch_and_len(struct hdd_config *hdd_cfg,
					    int *num_ch, int *ch_len)
{
	*num_ch = 0;
	*ch_len = 0;
}

static void wlan_hdd_copy_srd_ch(char *ch_ptr, int ch_arr_len)
{
}

/**
 * wlan_hdd_populate_5dot9_chan_info() - Populate 5.9 GHz chan info in hdd
 * context
 * @hdd_ctx: pointer to hdd context
 * @index: 5.9 GHz channel beginning index in chan_info of @hdd_ctx
 *
 * Return: Number of 5.9 GHz channels populated
 */
static uint32_t
wlan_hdd_populate_5dot9_chan_info(struct hdd_context *hdd_ctx, uint32_t index)
{
	return 0;
}

#else

static void wlan_hdd_get_num_dsrc_ch_and_len(struct hdd_config *hdd_cfg,
					     int *num_ch, int *ch_len)
{
	*num_ch = 0;
	*ch_len = 0;
}

static void wlan_hdd_copy_dsrc_ch(char *ch_ptr, int ch_arr_len)
{
}

static void wlan_hdd_get_num_srd_ch_and_len(struct hdd_config *hdd_cfg,
					    int *num_ch, int *ch_len)
{
	*num_ch = QDF_ARRAY_SIZE(hdd_5dot9_ghz_ch);
	*ch_len = sizeof(hdd_5dot9_ghz_ch);
}

static void wlan_hdd_copy_srd_ch(char *ch_ptr, int ch_arr_len)
{
	if (!ch_arr_len)
		return;
	qdf_mem_copy(ch_ptr, &hdd_5dot9_ghz_ch[0], ch_arr_len);
}

/**
 * wlan_hdd_populate_5dot9_chan_info() - Populate 5.9 GHz chan info in hdd
 * context
 * @hdd_ctx: pointer to hdd context
 * @index: 5.9 GHz channel beginning index in chan_info of @hdd_ctx
 *
 * Return: Number of 5.9 GHz channels populated
 */
static uint32_t
wlan_hdd_populate_5dot9_chan_info(struct hdd_context *hdd_ctx, uint32_t index)
{
	uint32_t num_5dot9_ch, i;
	struct scan_chan_info *chan_info;

	num_5dot9_ch = QDF_ARRAY_SIZE(hdd_5dot9_ghz_ch);
	chan_info = hdd_ctx->chan_info;

	for (i = 0; i < num_5dot9_ch; i++)
		chan_info[index + i].freq = hdd_5dot9_ghz_ch[i].center_freq;

	return num_5dot9_ch;
}

#endif

#if defined(WLAN_FEATURE_11AX) && \
	(defined(CFG80211_SBAND_IFTYPE_DATA_BACKPORT) || \
	 (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)))
#if defined(CONFIG_BAND_6GHZ) && (defined(CFG80211_6GHZ_BAND_SUPPORTED) || \
	   (KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE))
static QDF_STATUS
wlan_hdd_iftype_data_alloc_6ghz(struct hdd_context *hdd_ctx)
{
	hdd_ctx->iftype_data_6g =
			qdf_mem_malloc(sizeof(*hdd_ctx->iftype_data_6g));

	if (!hdd_ctx->iftype_data_6g)
		return QDF_STATUS_E_NOMEM;

	return QDF_STATUS_SUCCESS;
}

static void
wlan_hdd_iftype_data_mem_free_6ghz(struct hdd_context *hdd_ctx)
{
	qdf_mem_free(hdd_ctx->iftype_data_6g);
	hdd_ctx->iftype_data_6g = NULL;
}
#else
static inline QDF_STATUS
wlan_hdd_iftype_data_alloc_6ghz(struct hdd_context *hdd_ctx)
{
	return QDF_STATUS_SUCCESS;
}

static inline void
wlan_hdd_iftype_data_mem_free_6ghz(struct hdd_context *hdd_ctx)
{
}
#endif

static QDF_STATUS
wlan_hdd_iftype_data_alloc(struct hdd_context *hdd_ctx)
{
	hdd_ctx->iftype_data_2g =
			qdf_mem_malloc(sizeof(*hdd_ctx->iftype_data_2g));

	if (!hdd_ctx->iftype_data_2g)
		return QDF_STATUS_E_NOMEM;

	hdd_ctx->iftype_data_5g =
			qdf_mem_malloc(sizeof(*hdd_ctx->iftype_data_5g));
	if (!hdd_ctx->iftype_data_5g) {
		qdf_mem_free(hdd_ctx->iftype_data_2g);
		hdd_ctx->iftype_data_2g = NULL;
		return QDF_STATUS_E_NOMEM;
	}

	if (QDF_IS_STATUS_ERROR(wlan_hdd_iftype_data_alloc_6ghz(hdd_ctx))) {
		qdf_mem_free(hdd_ctx->iftype_data_5g);
		qdf_mem_free(hdd_ctx->iftype_data_2g);
		hdd_ctx->iftype_data_2g = NULL;
		hdd_ctx->iftype_data_5g = NULL;
		return QDF_STATUS_E_NOMEM;
	}

	return QDF_STATUS_SUCCESS;
}

static void
wlan_hdd_iftype_data_mem_free(struct hdd_context *hdd_ctx)
{
	wlan_hdd_iftype_data_mem_free_6ghz(hdd_ctx);
	qdf_mem_free(hdd_ctx->iftype_data_5g);
	qdf_mem_free(hdd_ctx->iftype_data_2g);
	hdd_ctx->iftype_data_5g = NULL;
	hdd_ctx->iftype_data_2g = NULL;
}
#else
static QDF_STATUS
wlan_hdd_iftype_data_alloc(struct hdd_context *hdd_ctx)

{
	return QDF_STATUS_SUCCESS;
}

static inline void
wlan_hdd_iftype_data_mem_free(struct hdd_context *hdd_ctx)
{
}
#endif

#if defined(WLAN_FEATURE_NAN) && \
	   (KERNEL_VERSION(4, 9, 0) <= LINUX_VERSION_CODE)
static void wlan_hdd_set_nan_if_mode(struct wiphy *wiphy)
{
	wiphy->interface_modes |= BIT(NL80211_IFTYPE_NAN);
}
#else
static void wlan_hdd_set_nan_if_mode(struct wiphy *wiphy)
{
}
#endif

#if defined(WLAN_FEATURE_NAN) && \
	   (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
static void wlan_hdd_set_nan_supported_bands(struct wiphy *wiphy)
{
	wiphy->nan_supported_bands =
		BIT(NL80211_BAND_2GHZ) | BIT(NL80211_BAND_5GHZ);
}
#else
static void wlan_hdd_set_nan_supported_bands(struct wiphy *wiphy)
{
}
#endif

/**
 * wlan_hdd_update_akm_suit_info() - Populate akm suits supported by driver
 * @wiphy: wiphy
 *
 * Return: void
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0)) || \
	defined(CFG80211_IFTYPE_AKM_SUITES_SUPPORT)
static void
wlan_hdd_update_akm_suit_info(struct wiphy *wiphy)
{
	wiphy->iftype_akm_suites = wlan_hdd_akm_suites;
	wiphy->num_iftype_akm_suites = QDF_ARRAY_SIZE(wlan_hdd_akm_suites);
}
#else
static void
wlan_hdd_update_akm_suit_info(struct wiphy *wiphy)
{
}
#endif

#ifdef CFG80211_CTRL_FRAME_SRC_ADDR_TA_ADDR
static void wlan_hdd_update_eapol_over_nl80211_flags(struct wiphy *wiphy)
{
	wiphy_ext_feature_set(wiphy,
			      NL80211_EXT_FEATURE_CONTROL_PORT_OVER_NL80211);
	wiphy_ext_feature_set(wiphy,
			      NL80211_EXT_FEATURE_CONTROL_PORT_NO_PREAUTH);
}
#else
static void wlan_hdd_update_eapol_over_nl80211_flags(struct wiphy *wiphy)
{
}
#endif

#ifdef CFG80211_MULTI_AKM_CONNECT_SUPPORT
static void
wlan_hdd_update_max_connect_akm(struct wiphy *wiphy)
{
	wiphy->max_num_akms_connect = WLAN_CM_MAX_CONNECT_AKMS;
}
#else
static void
wlan_hdd_update_max_connect_akm(struct wiphy *wiphy)
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
			   struct wiphy *wiphy, struct hdd_config *config)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	uint32_t *cipher_suites;
	hdd_enter();

	/* Now bind the underlying wlan device with wiphy */
	set_wiphy_dev(wiphy, dev);

	wiphy->mgmt_stypes = wlan_hdd_txrx_stypes;
	wlan_hdd_update_akm_suit_info(wiphy);
	wiphy->flags |= WIPHY_FLAG_HAVE_AP_SME
			| WIPHY_FLAG_AP_PROBE_RESP_OFFLOAD
			| WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL
#ifdef FEATURE_WLAN_STA_4ADDR_SCHEME
			| WIPHY_FLAG_4ADDR_STATION
#endif
			| WIPHY_FLAG_OFFCHAN_TX;

	if (ucfg_pmo_get_suspend_mode(hdd_ctx->psoc) == PMO_SUSPEND_WOW) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0))
		wiphy->wowlan = &wowlan_support_cfg80211_init;
#else
		wiphy->wowlan.flags = WIPHY_WOWLAN_MAGIC_PKT;
		wiphy->wowlan.n_patterns = WOWL_MAX_PTRNS_ALLOWED;
		wiphy->wowlan.pattern_min_len = 1;
		wiphy->wowlan.pattern_max_len = WOWL_PTRN_MAX_SIZE;
#endif
	}

#ifdef FEATURE_WLAN_TDLS
	wiphy->flags |= WIPHY_FLAG_SUPPORTS_TDLS
			| WIPHY_FLAG_TDLS_EXTERNAL_SETUP;
#endif

	wlan_hdd_cfg80211_set_wiphy_scan_flags(wiphy);

	wlan_scan_cfg80211_add_connected_pno_support(wiphy);

	wiphy->max_scan_ssids = MAX_SCAN_SSID;

	wiphy->max_scan_ie_len = SIR_MAC_MAX_ADD_IE_LENGTH;

	wiphy->max_acl_mac_addrs = MAX_ACL_MAC_ADDRESS;

	wiphy->interface_modes = BIT(NL80211_IFTYPE_STATION)
				 | BIT(NL80211_IFTYPE_P2P_CLIENT)
				 | BIT(NL80211_IFTYPE_P2P_GO)
				 | BIT(NL80211_IFTYPE_AP)
				 | BIT(NL80211_IFTYPE_MONITOR);

	wlan_hdd_set_nan_if_mode(wiphy);

	/*
	 * In case of static linked driver at the time of driver unload,
	 * module exit doesn't happens. Module cleanup helps in cleaning
	 * of static memory.
	 * If driver load happens statically, at the time of driver unload,
	 * wiphy flags don't get reset because of static memory.
	 * It's better not to store channel in static memory.
	 * The memory is for channels of struct wiphy and shouldn't be
	 * released during stop modules. So if it's allocated in active
	 * domain, the memory leak detector would catch the leak during
	 * stop modules. To avoid this,alloc in init domain in advance.
	 */
	hdd_ctx->channels_2ghz = qdf_mem_malloc(band_2_ghz_channels_size);
	if (!hdd_ctx->channels_2ghz)
		return -ENOMEM;

	hdd_ctx->channels_5ghz = qdf_mem_malloc(band_5_ghz_chanenls_size);
	if (!hdd_ctx->channels_5ghz)
		goto mem_fail_5g;

	if (QDF_IS_STATUS_ERROR(wlan_hdd_iftype_data_alloc(hdd_ctx)))
		goto mem_fail_iftype_data;

	/*Initialise the supported cipher suite details */
	if (ucfg_fwol_get_gcmp_enable(hdd_ctx->psoc)) {
		cipher_suites = qdf_mem_malloc(sizeof(hdd_cipher_suites) +
					       sizeof(hdd_gcmp_cipher_suits));
		if (!cipher_suites)
			goto mem_fail_cipher_suites;
		wiphy->n_cipher_suites = QDF_ARRAY_SIZE(hdd_cipher_suites) +
			 QDF_ARRAY_SIZE(hdd_gcmp_cipher_suits);
		qdf_mem_copy(cipher_suites, &hdd_cipher_suites,
			     sizeof(hdd_cipher_suites));
		qdf_mem_copy(cipher_suites + QDF_ARRAY_SIZE(hdd_cipher_suites),
			     &hdd_gcmp_cipher_suits,
			     sizeof(hdd_gcmp_cipher_suits));
	} else {
		cipher_suites = qdf_mem_malloc(sizeof(hdd_cipher_suites));
		if (!cipher_suites)
			goto mem_fail_cipher_suites;
		wiphy->n_cipher_suites = QDF_ARRAY_SIZE(hdd_cipher_suites);
		qdf_mem_copy(cipher_suites, &hdd_cipher_suites,
			     sizeof(hdd_cipher_suites));
	}
	wiphy->cipher_suites = cipher_suites;
	cipher_suites = NULL;
	/*signal strength in mBm (100*dBm) */
	wiphy->signal_type = CFG80211_SIGNAL_TYPE_MBM;
	wiphy->max_remain_on_channel_duration = MAX_REMAIN_ON_CHANNEL_DURATION;

	wiphy->n_vendor_commands = ARRAY_SIZE(hdd_wiphy_vendor_commands);
	wiphy->vendor_commands = hdd_wiphy_vendor_commands;

	wiphy->vendor_events = wlan_hdd_cfg80211_vendor_events;
	wiphy->n_vendor_events = ARRAY_SIZE(wlan_hdd_cfg80211_vendor_events);

#ifdef QCA_HT_2040_COEX
	wiphy->features |= NL80211_FEATURE_AP_MODE_CHAN_WIDTH_CHANGE;
#endif
	wiphy->features |= NL80211_FEATURE_INACTIVITY_TIMER;

	wiphy->features |= NL80211_FEATURE_VIF_TXPOWER;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)) || \
	defined(CFG80211_BEACON_TX_RATE_CUSTOM_BACKPORT)
	wiphy_ext_feature_set(wiphy, NL80211_EXT_FEATURE_BEACON_RATE_LEGACY);
	wiphy_ext_feature_set(wiphy, NL80211_EXT_FEATURE_BEACON_RATE_HT);
	wiphy_ext_feature_set(wiphy, NL80211_EXT_FEATURE_BEACON_RATE_VHT);
#endif

	hdd_add_channel_switch_support(&wiphy->flags);
	wiphy->max_num_csa_counters = WLAN_HDD_MAX_NUM_CSA_COUNTERS;

	wlan_hdd_update_max_connect_akm(wiphy);

	wlan_hdd_cfg80211_action_frame_randomization_init(wiphy);

	wlan_hdd_set_nan_supported_bands(wiphy);

	wlan_hdd_update_eapol_over_nl80211_flags(wiphy);

	hdd_exit();
	return 0;

mem_fail_cipher_suites:
	wlan_hdd_iftype_data_mem_free(hdd_ctx);
mem_fail_iftype_data:
	qdf_mem_free(hdd_ctx->channels_5ghz);
	hdd_ctx->channels_5ghz = NULL;
mem_fail_5g:
	hdd_err("Not enough memory to allocate channels");
	qdf_mem_free(hdd_ctx->channels_2ghz);
	hdd_ctx->channels_2ghz = NULL;

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
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);

	for (i = 0; i < HDD_NUM_NL80211_BANDS; i++) {
		if (wiphy->bands[i] &&
		   (wiphy->bands[i]->channels))
			wiphy->bands[i]->channels = NULL;
	}
	wlan_hdd_iftype_data_mem_free(hdd_ctx);
	qdf_mem_free(hdd_ctx->channels_5ghz);
	qdf_mem_free(hdd_ctx->channels_2ghz);
	hdd_ctx->channels_2ghz = NULL;
	hdd_ctx->channels_5ghz = NULL;

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
static void wlan_hdd_update_ht_cap(struct hdd_context *hdd_ctx)
{
	struct mlme_ht_capabilities_info ht_cap_info = {0};
	QDF_STATUS status;
	uint32_t channel_bonding_mode;
	struct ieee80211_supported_band *band_2g;
	struct ieee80211_supported_band *band_5g;
	uint8_t i;

	status = ucfg_mlme_get_ht_cap_info(hdd_ctx->psoc, &ht_cap_info);
	if (QDF_STATUS_SUCCESS != status)
		hdd_err("could not get HT capability info");

	band_2g = hdd_ctx->wiphy->bands[HDD_NL80211_BAND_2GHZ];
	band_5g = hdd_ctx->wiphy->bands[HDD_NL80211_BAND_5GHZ];

	if (band_2g) {
		if (ht_cap_info.tx_stbc)
			band_2g->ht_cap.cap |= IEEE80211_HT_CAP_TX_STBC;

		if (!sme_is_feature_supported_by_fw(DOT11AC)) {
			band_2g->vht_cap.vht_supported = 0;
			band_2g->vht_cap.cap = 0;
		}

		if (!ht_cap_info.short_gi_20_mhz)
			band_2g->ht_cap.cap &= ~IEEE80211_HT_CAP_SGI_20;

		for (i = 0; i < hdd_ctx->num_rf_chains; i++)
			band_2g->ht_cap.mcs.rx_mask[i] = 0xff;

		/*
		 * According to mcs_nss HT MCS parameters highest data rate for
		 * Nss = 1 is 150 Mbps
		 */
		band_2g->ht_cap.mcs.rx_highest =
				cpu_to_le16(150 * hdd_ctx->num_rf_chains);
	}

	if (band_5g) {
		if (ht_cap_info.tx_stbc)
			band_5g->ht_cap.cap |= IEEE80211_HT_CAP_TX_STBC;

		if (!sme_is_feature_supported_by_fw(DOT11AC)) {
			band_5g->vht_cap.vht_supported = 0;
			band_5g->vht_cap.cap = 0;
		}

		if (!ht_cap_info.short_gi_20_mhz)
			band_5g->ht_cap.cap &= ~IEEE80211_HT_CAP_SGI_20;

		if (!ht_cap_info.short_gi_40_mhz)
			band_5g->ht_cap.cap &= ~IEEE80211_HT_CAP_SGI_40;

		ucfg_mlme_get_channel_bonding_5ghz(hdd_ctx->psoc,
						   &channel_bonding_mode);
		if (!channel_bonding_mode)
			band_5g->ht_cap.cap &=
					~IEEE80211_HT_CAP_SUP_WIDTH_20_40;

		for (i = 0; i < hdd_ctx->num_rf_chains; i++)
			band_5g->ht_cap.mcs.rx_mask[i] = 0xff;
		/*
		 * According to mcs_nss HT MCS parameters highest data rate for
		 * Nss = 1 is 150 Mbps
		 */
		band_5g->ht_cap.mcs.rx_highest =
				cpu_to_le16(150 * hdd_ctx->num_rf_chains);
	}
}

/**
 * wlan_hdd_update_band_cap_in_wiphy() - update channel flags based on band cap
 * @hdd_ctx: HDD context
 *
 * This function updates the channel flags based on the band capability set
 * in the MLME CFG
 *
 * Return: void
 */
static void wlan_hdd_update_band_cap_in_wiphy(struct hdd_context *hdd_ctx)
{
	int i, j;
	uint32_t band_capability;
	QDF_STATUS status;
	struct ieee80211_supported_band *band;

	status = ucfg_mlme_get_band_capability(hdd_ctx->psoc, &band_capability);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("Failed to get MLME Band Capability");
		return;
	}

	for (i = 0; i < HDD_NUM_NL80211_BANDS; i++) {
		if (!hdd_ctx->wiphy->bands[i])
			continue;

		for (j = 0; j < hdd_ctx->wiphy->bands[i]->n_channels; j++) {
			band = hdd_ctx->wiphy->bands[i];

			if (HDD_NL80211_BAND_2GHZ == i &&
			    BIT(REG_BAND_5G) == band_capability) {
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
				   BIT(REG_BAND_2G) == band_capability) {
				/* 2G only */
				band->channels[j].flags |=
					IEEE80211_CHAN_DISABLED;
				continue;
			}
		}
	}
}

#ifdef FEATURE_WLAN_ESE
/**
 * wlan_hdd_update_lfr_wiphy() - update LFR flag based on configures
 * @hdd_ctx: HDD context
 *
 * This function updates the LFR flag based on LFR configures
 *
 * Return: void
 */
static void wlan_hdd_update_lfr_wiphy(struct hdd_context *hdd_ctx)
{
	bool fast_transition_enabled;
	bool lfr_enabled;
	bool ese_enabled;
	bool roam_offload;

	ucfg_mlme_is_fast_transition_enabled(hdd_ctx->psoc,
					     &fast_transition_enabled);
	ucfg_mlme_is_lfr_enabled(hdd_ctx->psoc, &lfr_enabled);
	ucfg_mlme_is_ese_enabled(hdd_ctx->psoc, &ese_enabled);
	ucfg_mlme_get_roaming_offload(hdd_ctx->psoc, &roam_offload);
	if (fast_transition_enabled || lfr_enabled || ese_enabled ||
	    roam_offload)
		hdd_ctx->wiphy->flags |= WIPHY_FLAG_SUPPORTS_FW_ROAM;
}
#else
static void wlan_hdd_update_lfr_wiphy(struct hdd_context *hdd_ctx)
{
	bool fast_transition_enabled;
	bool lfr_enabled;
	bool roam_offload;

	ucfg_mlme_is_fast_transition_enabled(hdd_ctx->psoc,
					     &fast_transition_enabled);
	ucfg_mlme_is_lfr_enabled(hdd_ctx->psoc, &lfr_enabled);
	ucfg_mlme_get_roaming_offload(hdd_ctx->psoc, &roam_offload);
	if (fast_transition_enabled || lfr_enabled || roam_offload)
		hdd_ctx->wiphy->flags |= WIPHY_FLAG_SUPPORTS_FW_ROAM;
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
static void wlan_hdd_set_mfp_optional(struct wiphy *wiphy)
{
	wiphy_ext_feature_set(wiphy, NL80211_EXT_FEATURE_MFP_OPTIONAL);
}
#else
static void wlan_hdd_set_mfp_optional(struct wiphy *wiphy)
{
}
#endif

/*
 * In this function, wiphy structure is updated after QDF
 * initialization. In wlan_hdd_cfg80211_init, only the
 * default values will be initialized. The final initialization
 * of all required members can be done here.
 */
void wlan_hdd_update_wiphy(struct hdd_context *hdd_ctx)
{
	int value;
	bool fils_enabled, mac_spoofing_enabled;
	bool is_oce_sta_enabled = false;
	QDF_STATUS status;
	struct wiphy *wiphy = hdd_ctx->wiphy;
	uint8_t allow_mcc_go_diff_bi = 0, enable_mcc = 0;
	bool is_bigtk_supported;
	bool is_ocv_supported;
	uint8_t iface_num;
	bool dbs_one_by_one, dbs_two_by_two;

	if (!wiphy) {
		hdd_err("Invalid wiphy");
		return;
	}
	ucfg_mlme_get_sap_max_peers(hdd_ctx->psoc, &value);
	hdd_ctx->wiphy->max_ap_assoc_sta = value;
	wlan_hdd_update_ht_cap(hdd_ctx);
	wlan_hdd_update_band_cap_in_wiphy(hdd_ctx);
	wlan_hdd_update_lfr_wiphy(hdd_ctx);

	fils_enabled = 0;
	status = ucfg_mlme_get_fils_enabled_info(hdd_ctx->psoc,
						 &fils_enabled);
	if (QDF_IS_STATUS_ERROR(status))
		hdd_err("could not get fils enabled info");
	if (fils_enabled)
		wlan_hdd_cfg80211_set_wiphy_fils_feature(wiphy);

	wlan_hdd_cfg80211_set_dfs_offload_feature(wiphy);

	status = ucfg_mlme_get_bigtk_support(hdd_ctx->psoc,
					     &is_bigtk_supported);

	if (QDF_IS_STATUS_SUCCESS(status) && is_bigtk_supported)
		wlan_hdd_cfg80211_set_bigtk_flags(wiphy);

	status = ucfg_mlme_get_ocv_support(hdd_ctx->psoc,
					   &is_ocv_supported);
	if (QDF_IS_STATUS_SUCCESS(status) && is_ocv_supported)
		wlan_hdd_cfg80211_set_ocv_flags(wiphy);

	status = ucfg_mlme_get_oce_sta_enabled_info(hdd_ctx->psoc,
						    &is_oce_sta_enabled);
	if (QDF_IS_STATUS_ERROR(status))
		hdd_err("could not get OCE STA enable info");
	if (is_oce_sta_enabled)
		wlan_hdd_cfg80211_set_wiphy_oce_scan_flags(wiphy);

	wlan_hdd_cfg80211_set_wiphy_sae_feature(wiphy);

	if (QDF_STATUS_SUCCESS !=
	    ucfg_policy_mgr_get_allow_mcc_go_diff_bi(hdd_ctx->psoc,
						     &allow_mcc_go_diff_bi))
		hdd_err("can't get mcc_go_diff_bi value, use default");

	if (QDF_STATUS_SUCCESS !=
	    ucfg_mlme_get_mcc_feature(hdd_ctx->psoc, &enable_mcc))
		hdd_err("can't get enable_mcc value, use default");

	if (hdd_ctx->config->advertise_concurrent_operation) {
		if (enable_mcc) {
			int i;

			for (i = 0;
			     i < ARRAY_SIZE(wlan_hdd_iface_combination);
			     i++) {
				if (!allow_mcc_go_diff_bi)
					wlan_hdd_iface_combination[i].
					beacon_int_infra_match = true;
			}
		}

		status = ucfg_policy_mgr_get_dbs_hw_modes(hdd_ctx->psoc,
							  &dbs_one_by_one,
							  &dbs_two_by_two);

		if (QDF_IS_STATUS_ERROR(status)) {
			hdd_err("HW mode failure");
			return;
		}

		if (!ucfg_policy_mgr_is_fw_supports_dbs(hdd_ctx->psoc) ||
		    (dbs_one_by_one && !dbs_two_by_two)) {
			wiphy->iface_combinations =
						wlan_hdd_derived_combination;
			iface_num = ARRAY_SIZE(wlan_hdd_derived_combination);
		} else {
			wiphy->iface_combinations = wlan_hdd_iface_combination;
			iface_num = ARRAY_SIZE(wlan_hdd_iface_combination);
		}

		wiphy->n_iface_combinations = iface_num;
	}

	mac_spoofing_enabled = ucfg_scan_is_mac_spoofing_enabled(hdd_ctx->psoc);
	if (mac_spoofing_enabled)
		wlan_hdd_cfg80211_scan_randomization_init(wiphy);

	wlan_hdd_set_mfp_optional(wiphy);
}

/**
 * wlan_hdd_update_11n_mode - update 11n mode in hdd cfg
 * @cfg: hdd cfg
 *
 * this function update 11n mode in hdd cfg
 *
 * Return: void
 */
void wlan_hdd_update_11n_mode(struct hdd_context *hdd_ctx)
{
	struct hdd_config *cfg = hdd_ctx->config;

	if (sme_is_feature_supported_by_fw(DOT11AC)) {
		hdd_debug("support 11ac");
	} else {
		hdd_debug("not support 11ac");
		if ((cfg->dot11Mode == eHDD_DOT11_MODE_11ac_ONLY) ||
		    (cfg->dot11Mode == eHDD_DOT11_MODE_11ac)) {
			cfg->dot11Mode = eHDD_DOT11_MODE_11n;
			ucfg_mlme_set_sap_11ac_override(hdd_ctx->psoc, 0);
			ucfg_mlme_set_go_11ac_override(hdd_ctx->psoc, 0);
		}
	}
}

QDF_STATUS wlan_hdd_update_wiphy_supported_band(struct hdd_context *hdd_ctx)
{
	int len_5g_ch, num_ch;
	int num_dsrc_ch, len_dsrc_ch, num_srd_ch, len_srd_ch;
	bool is_vht_for_24ghz = false;
	QDF_STATUS status;
	struct hdd_config *cfg = hdd_ctx->config;
	struct wiphy *wiphy = hdd_ctx->wiphy;

	if (wiphy->registered)
		return QDF_STATUS_SUCCESS;

	if (hdd_is_2g_supported(hdd_ctx)) {
		if (!hdd_ctx->channels_2ghz)
			return QDF_STATUS_E_NOMEM;
		wiphy->bands[HDD_NL80211_BAND_2GHZ] = &wlan_hdd_band_2_4_ghz;
		wiphy->bands[HDD_NL80211_BAND_2GHZ]->channels =
							hdd_ctx->channels_2ghz;
		qdf_mem_copy(wiphy->bands[HDD_NL80211_BAND_2GHZ]->channels,
			     &hdd_channels_2_4_ghz[0],
			     sizeof(hdd_channels_2_4_ghz));

		status = ucfg_mlme_get_vht_for_24ghz(hdd_ctx->psoc,
						     &is_vht_for_24ghz);
		if (QDF_IS_STATUS_ERROR(status))
			hdd_err("could not get VHT capability");

		if (is_vht_for_24ghz &&
		    sme_is_feature_supported_by_fw(DOT11AC) &&
		    (cfg->dot11Mode == eHDD_DOT11_MODE_AUTO ||
		     cfg->dot11Mode == eHDD_DOT11_MODE_11ac_ONLY ||
		     cfg->dot11Mode == eHDD_DOT11_MODE_11ac ||
		     cfg->dot11Mode == eHDD_DOT11_MODE_11ax ||
		     cfg->dot11Mode == eHDD_DOT11_MODE_11ax_ONLY))
			wlan_hdd_band_2_4_ghz.vht_cap.vht_supported = 1;
	}
	if (!hdd_is_5g_supported(hdd_ctx) ||
	    (eHDD_DOT11_MODE_11b == cfg->dot11Mode) ||
	    (eHDD_DOT11_MODE_11g == cfg->dot11Mode) ||
	    (eHDD_DOT11_MODE_11b_ONLY == cfg->dot11Mode) ||
	    (eHDD_DOT11_MODE_11g_ONLY == cfg->dot11Mode))
		return QDF_STATUS_SUCCESS;

	if (!hdd_ctx->channels_5ghz)
		return QDF_STATUS_E_NOMEM;
	wiphy->bands[HDD_NL80211_BAND_5GHZ] = &wlan_hdd_band_5_ghz;
	wiphy->bands[HDD_NL80211_BAND_5GHZ]->channels = hdd_ctx->channels_5ghz;
	wlan_hdd_get_num_dsrc_ch_and_len(cfg, &num_dsrc_ch, &len_dsrc_ch);
	wlan_hdd_get_num_srd_ch_and_len(cfg, &num_srd_ch, &len_srd_ch);
	num_ch = QDF_ARRAY_SIZE(hdd_channels_5_ghz) + num_dsrc_ch + num_srd_ch;
	len_5g_ch = sizeof(hdd_channels_5_ghz);

	wiphy->bands[HDD_NL80211_BAND_5GHZ]->n_channels = num_ch;
	qdf_mem_copy(wiphy->bands[HDD_NL80211_BAND_5GHZ]->channels,
		     &hdd_channels_5_ghz[0], len_5g_ch);
	if (num_dsrc_ch)
		wlan_hdd_copy_dsrc_ch((char *)wiphy->bands[
				      HDD_NL80211_BAND_5GHZ]->channels +
				      len_5g_ch, len_dsrc_ch);
	if (num_srd_ch)
		wlan_hdd_copy_srd_ch((char *)wiphy->bands[
				     HDD_NL80211_BAND_5GHZ]->channels +
				     len_5g_ch, len_srd_ch);

	if (cfg->dot11Mode != eHDD_DOT11_MODE_AUTO &&
	    cfg->dot11Mode != eHDD_DOT11_MODE_11ac &&
	    cfg->dot11Mode != eHDD_DOT11_MODE_11ac_ONLY &&
	    cfg->dot11Mode != eHDD_DOT11_MODE_11ax &&
	    cfg->dot11Mode != eHDD_DOT11_MODE_11ax_ONLY)
		 wlan_hdd_band_5_ghz.vht_cap.vht_supported = 0;

	if (cfg->dot11Mode == eHDD_DOT11_MODE_AUTO ||
	    cfg->dot11Mode == eHDD_DOT11_MODE_11ax ||
	    cfg->dot11Mode == eHDD_DOT11_MODE_11ax_ONLY)
		hdd_init_6ghz(hdd_ctx);

	return QDF_STATUS_SUCCESS;
}

/* In this function we are registering wiphy. */
int wlan_hdd_cfg80211_register(struct wiphy *wiphy)
{
	int ret;

	hdd_enter();
	ret = wiphy_register(wiphy);
	/* Register our wiphy dev with cfg80211 */
	if (ret < 0) {
		hdd_err("wiphy register failed %d", ret);
		return -EIO;
	}

	hdd_exit();
	return 0;
}

/* This function registers for all frame which supplicant is interested in */
int wlan_hdd_cfg80211_register_frames(struct hdd_adapter *adapter)
{
	mac_handle_t mac_handle = hdd_adapter_get_mac_handle(adapter);
	/* Register for all P2P action, public action etc frames */
	uint16_t type = (SIR_MAC_MGMT_FRAME << 2) | (SIR_MAC_MGMT_ACTION << 4);
	QDF_STATUS status;

	hdd_enter();
	if (adapter->device_mode == QDF_FTM_MODE) {
		hdd_info("No need to register frames in FTM mode");
		return 0;
	}

	/* Register frame indication call back */
	status = sme_register_mgmt_frame_ind_callback(mac_handle,
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
	status = sme_register_mgmt_frame(mac_handle, SME_SESSION_ID_ANY, type,
					 (uint8_t *) GAS_INITIAL_REQ,
					 GAS_INITIAL_REQ_SIZE);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("Failed to register GAS_INITIAL_REQ");
		goto ret_status;
	}

	/* GAS Initial Response */
	status = sme_register_mgmt_frame(mac_handle, SME_SESSION_ID_ANY, type,
					 (uint8_t *) GAS_INITIAL_RSP,
					 GAS_INITIAL_RSP_SIZE);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("Failed to register GAS_INITIAL_RSP");
		goto dereg_gas_initial_req;
	}

	/* GAS Comeback Request */
	status = sme_register_mgmt_frame(mac_handle, SME_SESSION_ID_ANY, type,
					 (uint8_t *) GAS_COMEBACK_REQ,
					 GAS_COMEBACK_REQ_SIZE);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("Failed to register GAS_COMEBACK_REQ");
		goto dereg_gas_initial_rsp;
	}

	/* GAS Comeback Response */
	status = sme_register_mgmt_frame(mac_handle, SME_SESSION_ID_ANY, type,
					 (uint8_t *) GAS_COMEBACK_RSP,
					 GAS_COMEBACK_RSP_SIZE);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("Failed to register GAS_COMEBACK_RSP");
		goto dereg_gas_comeback_req;
	}

	/* WNM BSS Transition Request frame */
	status = sme_register_mgmt_frame(mac_handle, SME_SESSION_ID_ANY, type,
					 (uint8_t *) WNM_BSS_ACTION_FRAME,
					 WNM_BSS_ACTION_FRAME_SIZE);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("Failed to register WNM_BSS_ACTION_FRAME");
		goto dereg_gas_comeback_rsp;
	}

	/* WNM-Notification */
	status = sme_register_mgmt_frame(mac_handle, adapter->vdev_id, type,
					 (uint8_t *) WNM_NOTIFICATION_FRAME,
					 WNM_NOTIFICATION_FRAME_SIZE);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("Failed to register WNM_NOTIFICATION_FRAME");
		goto dereg_wnm_bss_action_frm;
	}

	return 0;

dereg_wnm_bss_action_frm:
	sme_deregister_mgmt_frame(mac_handle, SME_SESSION_ID_ANY, type,
				  (uint8_t *) WNM_BSS_ACTION_FRAME,
				  WNM_BSS_ACTION_FRAME_SIZE);
dereg_gas_comeback_rsp:
	sme_deregister_mgmt_frame(mac_handle, SME_SESSION_ID_ANY, type,
				  (uint8_t *) GAS_COMEBACK_RSP,
				  GAS_COMEBACK_RSP_SIZE);
dereg_gas_comeback_req:
	sme_deregister_mgmt_frame(mac_handle, SME_SESSION_ID_ANY, type,
				  (uint8_t *) GAS_COMEBACK_REQ,
				  GAS_COMEBACK_REQ_SIZE);
dereg_gas_initial_rsp:
	sme_deregister_mgmt_frame(mac_handle, SME_SESSION_ID_ANY, type,
				  (uint8_t *) GAS_INITIAL_RSP,
				  GAS_INITIAL_RSP_SIZE);
dereg_gas_initial_req:
	sme_deregister_mgmt_frame(mac_handle, SME_SESSION_ID_ANY, type,
				  (uint8_t *) GAS_INITIAL_REQ,
				  GAS_INITIAL_REQ_SIZE);
ret_status:
	return qdf_status_to_os_return(status);
}

void wlan_hdd_cfg80211_deregister_frames(struct hdd_adapter *adapter)
{
	mac_handle_t mac_handle = hdd_adapter_get_mac_handle(adapter);
	/* Deregister for all P2P action, public action etc frames */
	uint16_t type = (SIR_MAC_MGMT_FRAME << 2) | (SIR_MAC_MGMT_ACTION << 4);

	hdd_enter();

	/* Right now we are registering these frame when driver is getting
	 * initialized. Once we will move to 2.6.37 kernel, in which we have
	 * frame register ops, we will move this code as a part of that
	 */

	/* GAS Initial Request */

	sme_deregister_mgmt_frame(mac_handle, SME_SESSION_ID_ANY, type,
				  (uint8_t *) GAS_INITIAL_REQ,
				  GAS_INITIAL_REQ_SIZE);

	/* GAS Initial Response */
	sme_deregister_mgmt_frame(mac_handle, SME_SESSION_ID_ANY, type,
				  (uint8_t *) GAS_INITIAL_RSP,
				  GAS_INITIAL_RSP_SIZE);

	/* GAS Comeback Request */
	sme_deregister_mgmt_frame(mac_handle, SME_SESSION_ID_ANY, type,
				  (uint8_t *) GAS_COMEBACK_REQ,
				  GAS_COMEBACK_REQ_SIZE);

	/* GAS Comeback Response */
	sme_deregister_mgmt_frame(mac_handle, SME_SESSION_ID_ANY, type,
				  (uint8_t *) GAS_COMEBACK_RSP,
				  GAS_COMEBACK_RSP_SIZE);

	/* P2P Public Action */
	sme_deregister_mgmt_frame(mac_handle, SME_SESSION_ID_ANY, type,
				  (uint8_t *) P2P_PUBLIC_ACTION_FRAME,
				  P2P_PUBLIC_ACTION_FRAME_SIZE);

	/* P2P Action */
	sme_deregister_mgmt_frame(mac_handle, SME_SESSION_ID_ANY, type,
				  (uint8_t *) P2P_ACTION_FRAME,
				  P2P_ACTION_FRAME_SIZE);

	/* WNM-Notification */
	sme_deregister_mgmt_frame(mac_handle, adapter->vdev_id, type,
				  (uint8_t *) WNM_NOTIFICATION_FRAME,
				  WNM_NOTIFICATION_FRAME_SIZE);
}

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
					       uint32_t ch_freq)
{
	bool value = 0;
	uint32_t i;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct regulatory_channel *cur_chan_list;
	QDF_STATUS status;

	status = ucfg_mlme_get_sap_allow_all_channels(hdd_ctx->psoc, &value);
	if (status != QDF_STATUS_SUCCESS)
		hdd_err("Unable to fetch sap allow all channels");
	status = QDF_STATUS_E_INVAL;
	if (value) {
		/* Validate the channel */
		for (i = CHAN_ENUM_2412; i < NUM_CHANNELS; i++) {
			if (ch_freq == WLAN_REG_CH_TO_FREQ(i)) {
				status = QDF_STATUS_SUCCESS;
				break;
			}
		}
	} else {
		cur_chan_list = qdf_mem_malloc(NUM_CHANNELS *
				sizeof(struct regulatory_channel));
		if (!cur_chan_list)
			return QDF_STATUS_E_NOMEM;

		if (wlan_reg_get_secondary_current_chan_list(
		    hdd_ctx->pdev, cur_chan_list) != QDF_STATUS_SUCCESS) {
			qdf_mem_free(cur_chan_list);
			return QDF_STATUS_E_INVAL;
		}

		for (i = 0; i < NUM_CHANNELS; i++) {
			if (ch_freq != cur_chan_list[i].center_freq)
				continue;
			if (cur_chan_list[i].state != CHANNEL_STATE_DISABLE &&
			    cur_chan_list[i].state != CHANNEL_STATE_INVALID)
				status = QDF_STATUS_SUCCESS;
			break;
		}
		qdf_mem_free(cur_chan_list);
	}

	return status;

}

static int __wlan_hdd_cfg80211_change_bss(struct wiphy *wiphy,
					  struct net_device *dev,
					  struct bss_parameters *params)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	int ret = 0;
	QDF_STATUS qdf_ret_status;
	mac_handle_t mac_handle;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	cdp_config_param_type vdev_param;

	hdd_enter();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_vdev_id(adapter->vdev_id))
		return -EINVAL;

	qdf_mtrace(QDF_MODULE_ID_HDD, QDF_MODULE_ID_HDD,
		   TRACE_CODE_HDD_CFG80211_CHANGE_BSS,
		   adapter->vdev_id, params->ap_isolate);

	hdd_debug("Device_mode %s(%d), ap_isolate = %d",
		  qdf_opmode_str(adapter->device_mode),
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

		mac_handle = hdd_ctx->mac_handle;
		qdf_ret_status = sme_ap_disable_intra_bss_fwd(mac_handle,
							      adapter->vdev_id,
							      adapter->session.
							      ap.
							      disable_intrabss_fwd);
		if (!QDF_IS_STATUS_SUCCESS(qdf_ret_status))
			ret = -EINVAL;

		ucfg_ipa_set_ap_ibss_fwd(hdd_ctx->pdev,
					 adapter->vdev_id,
					 adapter->session.ap.
					 disable_intrabss_fwd);

		vdev_param.cdp_vdev_param_ap_brdg_en =
			!adapter->session.ap.disable_intrabss_fwd;
		cdp_txrx_set_vdev_param(soc, adapter->vdev_id,
					CDP_ENABLE_AP_BRIDGE,
					vdev_param);
	}

	hdd_exit();
	return ret;
}

/**
 * hdd_change_adapter_mode() - change @adapter's operating mode to @new_mode
 * @adapter: the adapter to change modes on
 * @new_mode: the new operating mode to change to
 *
 * Return: Errno
 */
static int hdd_change_adapter_mode(struct hdd_adapter *adapter,
				   enum QDF_OPMODE new_mode)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct net_device *netdev = adapter->dev;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	hdd_enter();

	hdd_stop_adapter(hdd_ctx, adapter);
	hdd_deinit_adapter(hdd_ctx, adapter, true);
	adapter->device_mode = new_mode;
	memset(&adapter->session, 0, sizeof(adapter->session));
	hdd_set_station_ops(netdev);

	hdd_exit();

	return qdf_status_to_os_return(status);
}

static int wlan_hdd_cfg80211_change_bss(struct wiphy *wiphy,
					struct net_device *dev,
					struct bss_parameters *params)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(dev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_change_bss(wiphy, dev, params);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}

static bool hdd_is_client_mode(enum QDF_OPMODE mode)
{
	switch (mode) {
	case QDF_STA_MODE:
	case QDF_P2P_CLIENT_MODE:
	case QDF_P2P_DEVICE_MODE:
		return true;
	default:
		return false;
	}
}

static bool hdd_is_ap_mode(enum QDF_OPMODE mode)
{
	switch (mode) {
	case QDF_SAP_MODE:
	case QDF_P2P_GO_MODE:
		return true;
	default:
		return false;
	}
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
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(ndev);
	struct hdd_context *hdd_ctx;
	bool iff_up = ndev->flags & IFF_UP;
	enum QDF_OPMODE new_mode;
	bool ap_random_bssid_enabled;
	QDF_STATUS status;
	int errno;
	uint8_t mac_addr[QDF_MAC_ADDR_SIZE];

	hdd_enter();

	if (hdd_get_conparam() == QDF_GLOBAL_FTM_MODE) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	errno = wlan_hdd_validate_context(hdd_ctx);
	if (errno)
		return errno;

	if (wlan_hdd_is_mon_concurrency())
		return -EINVAL;

	if (policy_mgr_is_sta_mon_concurrency(hdd_ctx->psoc))
		return -EINVAL;

	qdf_mtrace(QDF_MODULE_ID_HDD, QDF_MODULE_ID_HDD,
		   TRACE_CODE_HDD_CFG80211_CHANGE_IFACE,
		   adapter->vdev_id, type);

	status = hdd_nl_to_qdf_iface_type(type, &new_mode);
	if (QDF_IS_STATUS_ERROR(status))
		return qdf_status_to_os_return(status);

	/* A userspace issue leads to it sending a 'change to station mode'
	 * request on a "p2p" device, expecting the driver do execute a 'change
	 * to p2p-device mode' request instead. The (unfortunate) work around
	 * here is implemented by overriding the new mode if the net_device name
	 * starts with "p2p" and the requested mode was station.
	 */
	if (strnstr(ndev->name, "p2p", 3) && new_mode == QDF_STA_MODE)
		new_mode = QDF_P2P_DEVICE_MODE;

	hdd_debug("Changing mode for '%s' from %s to %s",
		  ndev->name,
		  qdf_opmode_str(adapter->device_mode),
		  qdf_opmode_str(new_mode));

	errno = hdd_trigger_psoc_idle_restart(hdd_ctx);
	if (errno) {
		hdd_err("Failed to restart psoc; errno:%d", errno);
		return -EINVAL;
	}

	/* Reset the current device mode bit mask */
	policy_mgr_clear_concurrency_mode(hdd_ctx->psoc, adapter->device_mode);

	if (hdd_is_client_mode(adapter->device_mode)) {
		if (adapter->device_mode == QDF_STA_MODE)
			hdd_cleanup_conn_info(adapter);

		if (hdd_is_client_mode(new_mode)) {
			errno = hdd_change_adapter_mode(adapter, new_mode);
			if (errno) {
				hdd_err("change intf mode fail %d", errno);
				goto err;
			}
		} else if (hdd_is_ap_mode(new_mode)) {
			if (new_mode == QDF_P2P_GO_MODE)
				wlan_hdd_cancel_existing_remain_on_channel
					(adapter);

			hdd_stop_adapter(hdd_ctx, adapter);
			hdd_deinit_adapter(hdd_ctx, adapter, true);
			memset(&adapter->session, 0, sizeof(adapter->session));
			adapter->device_mode = new_mode;

			status = ucfg_mlme_get_ap_random_bssid_enable(
						hdd_ctx->psoc,
						&ap_random_bssid_enabled);
			if (QDF_IS_STATUS_ERROR(status))
				return qdf_status_to_os_return(status);

			if (adapter->device_mode == QDF_SAP_MODE &&
			    ap_random_bssid_enabled) {
				/* To meet Android requirements create
				 * a randomized MAC address of the
				 * form 02:1A:11:Fx:xx:xx
				 */
				memcpy(mac_addr, ndev->dev_addr,
				       QDF_MAC_ADDR_SIZE);

				get_random_bytes(&mac_addr[3], 3);
				mac_addr[0] = 0x02;
				mac_addr[1] = 0x1A;
				mac_addr[2] = 0x11;
				mac_addr[3] |= 0xF0;
				memcpy(adapter->mac_addr.bytes, mac_addr,
				       QDF_MAC_ADDR_SIZE);
				qdf_net_update_net_device_dev_addr(ndev,
								   mac_addr,
								   QDF_MAC_ADDR_SIZE);

				pr_info("wlan: Generated HotSpot BSSID "
					QDF_MAC_ADDR_FMT "\n",
					QDF_MAC_ADDR_REF(ndev->dev_addr));
			}

			hdd_set_ap_ops(adapter->dev);
		} else {
			hdd_err("Changing to device mode '%s' is not supported",
				qdf_opmode_str(new_mode));
			errno = -EOPNOTSUPP;
			goto err;
		}
	} else if (hdd_is_ap_mode(adapter->device_mode)) {
		if (hdd_is_client_mode(new_mode)) {
			errno = hdd_change_adapter_mode(adapter, new_mode);
			if (errno) {
				hdd_err("change mode fail %d", errno);
				goto err;
			}
		} else if (hdd_is_ap_mode(new_mode)) {
			adapter->device_mode = new_mode;

			/* avoid starting the adapter, since it never stopped */
			iff_up = false;
		} else {
			hdd_err("Changing to device mode '%s' is not supported",
				qdf_opmode_str(new_mode));
			errno = -EOPNOTSUPP;
			goto err;
		}
	} else {
		hdd_err("Changing from device mode '%s' is not supported",
			qdf_opmode_str(adapter->device_mode));
		errno = -EOPNOTSUPP;
		goto err;
	}

	/* restart the adapter if it was up before the change iface request */
	if (iff_up) {
		errno = hdd_start_adapter(adapter);
		if (errno) {
			hdd_err("Failed to start adapter");
			errno = -EINVAL;
			goto err;
		}
	}

	ndev->ieee80211_ptr->iftype = type;
	hdd_lpass_notify_mode_change(adapter);
err:
	/* Set bitmask based on updated value */
	policy_mgr_set_concurrency_mode(hdd_ctx->psoc, adapter->device_mode);

	hdd_exit();

	return errno;
}

static int _wlan_hdd_cfg80211_change_iface(struct wiphy *wiphy,
					   struct net_device *net_dev,
					   enum nl80211_iftype type,
					   u32 *flags,
					   struct vif_params *params)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_trans_start(net_dev, &vdev_sync);
	if (errno)
		goto err;

	errno = __wlan_hdd_cfg80211_change_iface(wiphy, net_dev, type,
						 flags, params);

	osif_vdev_sync_trans_stop(vdev_sync);

	return errno;
err:
	/* In the SSR case, errno will be -EINVAL from
	 * __dsc_vdev_can_trans with qdf_is_recovering()
	 * is true, only change -EINVAL to -EBUSY to make
	 * wpa_supplicant has chance to retry mode switch.
	 * Meanwhile do not touch the errno from
	 * __wlan_hdd_cfg80211_change_iface with this
	 * change.
	 */
	if (errno && errno != -EAGAIN && errno != -EBUSY)
		errno = -EBUSY;
	return errno;
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
	return _wlan_hdd_cfg80211_change_iface(wiphy, ndev, type,
					       flags, params);
}
#else
static int wlan_hdd_cfg80211_change_iface(struct wiphy *wiphy,
					  struct net_device *ndev,
					  enum nl80211_iftype type,
					  struct vif_params *params)
{
	return _wlan_hdd_cfg80211_change_iface(wiphy, ndev, type,
					       &params->flags, params);
}
#endif /* KERNEL_VERSION(4, 12, 0) */

QDF_STATUS wlan_hdd_send_sta_authorized_event(
					struct hdd_adapter *adapter,
					struct hdd_context *hdd_ctx,
					const struct qdf_mac_addr *mac_addr)
{
	struct sk_buff *vendor_event;
	QDF_STATUS status;
	struct nl80211_sta_flag_update sta_flags;

	hdd_enter();
	if (!hdd_ctx) {
		hdd_err("HDD context is null");
		return QDF_STATUS_E_INVAL;
	}

	vendor_event =
		cfg80211_vendor_event_alloc(
			hdd_ctx->wiphy, &adapter->wdev, sizeof(sta_flags) +
			QDF_MAC_ADDR_SIZE + NLMSG_HDRLEN,
			QCA_NL80211_VENDOR_SUBCMD_LINK_PROPERTIES_INDEX,
			GFP_KERNEL);
	if (!vendor_event) {
		hdd_err("cfg80211_vendor_event_alloc failed");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mem_zero(&sta_flags, sizeof(sta_flags));

	sta_flags.mask |= BIT(NL80211_STA_FLAG_AUTHORIZED);
	sta_flags.set = true;

	status = nla_put(vendor_event,
			 QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_STA_FLAGS,
			 sizeof(struct  nl80211_sta_flag_update),
			 &sta_flags);
	if (status) {
		hdd_err("STA flag put fails");
		kfree_skb(vendor_event);
		return QDF_STATUS_E_FAILURE;
	}
	status = nla_put(vendor_event,
			 QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_MAC_ADDR,
			 QDF_MAC_ADDR_SIZE, mac_addr->bytes);
	if (status) {
		hdd_err("STA MAC put fails");
		kfree_skb(vendor_event);
		return QDF_STATUS_E_FAILURE;
	}

	cfg80211_vendor_event(vendor_event, GFP_KERNEL);

	hdd_exit();
	return QDF_STATUS_SUCCESS;
}

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
	struct hdd_ap_ctx *ap_ctx;
	struct qdf_mac_addr sta_macaddr;
	int ret;

	hdd_enter();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	qdf_mtrace(QDF_MODULE_ID_HDD, QDF_MODULE_ID_HDD,
		   TRACE_CODE_HDD_CHANGE_STATION,
		   adapter->vdev_id, params->listen_interval);

	if (wlan_hdd_validate_vdev_id(adapter->vdev_id))
		return -EINVAL;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	qdf_mem_copy(sta_macaddr.bytes, mac, QDF_MAC_ADDR_SIZE);

	if ((adapter->device_mode == QDF_SAP_MODE) ||
	    (adapter->device_mode == QDF_P2P_GO_MODE)) {
		if (params->sta_flags_set & BIT(NL80211_STA_FLAG_AUTHORIZED)) {
			ap_ctx = WLAN_HDD_GET_AP_CTX_PTR(adapter);
			/*
			 * For Encrypted SAP session, this will be done as
			 * part of eSAP_STA_SET_KEY_EVENT
			 */
			if (ap_ctx->encryption_type !=
			    eCSR_ENCRYPT_TYPE_NONE) {
				hdd_debug("Encrypt type %d, not setting peer authorized now",
					  ap_ctx->encryption_type);
				return 0;
			}

			status =
				hdd_softap_change_sta_state(adapter,
							    &sta_macaddr,
							    OL_TXRX_PEER_STATE_AUTH);

			if (status != QDF_STATUS_SUCCESS) {
				hdd_debug("Not able to change TL state to AUTHENTICATED");
				return -EINVAL;
			}
			status = wlan_hdd_send_sta_authorized_event(
								adapter,
								hdd_ctx,
								&sta_macaddr);
			if (status != QDF_STATUS_SUCCESS) {
				return -EINVAL;
			}
		}
	} else if ((adapter->device_mode == QDF_STA_MODE) ||
		   (adapter->device_mode == QDF_P2P_CLIENT_MODE)) {
		if (params->sta_flags_set & BIT(NL80211_STA_FLAG_TDLS_PEER)) {
#if defined(FEATURE_WLAN_TDLS)
			struct wlan_objmgr_vdev *vdev;

			vdev = hdd_objmgr_get_vdev_by_user(adapter,
							   WLAN_OSIF_TDLS_ID);
			if (!vdev)
				return -EINVAL;
			ret = wlan_cfg80211_tdls_update_peer(vdev, mac, params);
			hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_TDLS_ID);
#endif
		}
	}
	hdd_exit();
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
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(dev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_change_station(wiphy, dev, mac, params);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}

#ifdef FEATURE_WLAN_ESE
static bool hdd_is_krk_enc_type(uint32_t cipher_type)
{
	if (cipher_type == WLAN_CIPHER_SUITE_KRK)
		return true;

	return false;
}
#else
static bool hdd_is_krk_enc_type(uint32_t cipher_type)
{
	return false;
}
#endif

#if defined(FEATURE_WLAN_ESE) && defined(WLAN_FEATURE_ROAM_OFFLOAD)
static bool hdd_is_btk_enc_type(uint32_t cipher_type)
{
	if (cipher_type == WLAN_CIPHER_SUITE_BTK)
		return true;

	return false;
}
#else
static bool hdd_is_btk_enc_type(uint32_t cipher_type)
{
	return false;
}
#endif

static int wlan_hdd_add_key_sap(struct hdd_adapter *adapter,
				bool pairwise, u8 key_index,
				enum wlan_crypto_cipher_type cipher)
{
	struct wlan_objmgr_vdev *vdev;
	int errno = 0;
	struct hdd_hostapd_state *hostapd_state =
		WLAN_HDD_GET_HOSTAP_STATE_PTR(adapter);

	vdev = hdd_objmgr_get_vdev_by_user(adapter, WLAN_OSIF_ID);
	if (!vdev)
		return -EINVAL;

	/* Do not send install key when sap restart is in progress. If there is
	 * critical channel request handling going on, fw will stop that request
	 * and will not send restart resposne
	 */
	if (wlan_vdev_is_restart_progress(vdev) == QDF_STATUS_SUCCESS) {
		hdd_err("vdev: %d restart in progress", wlan_vdev_get_id(vdev));
		hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_ID);
		return -EINVAL;
	}

	if (hostapd_state->bss_state == BSS_START) {
		errno =
		wlan_cfg80211_crypto_add_key(vdev,
					     (pairwise ?
					      WLAN_CRYPTO_KEY_TYPE_UNICAST :
					      WLAN_CRYPTO_KEY_TYPE_GROUP),
					     key_index, true);
		if (!errno)
			wma_update_set_key(adapter->vdev_id, pairwise,
					   key_index, cipher);
	}
	hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_ID);

	return errno;
}

static int wlan_hdd_add_key_sta(struct wlan_objmgr_pdev *pdev,
				struct hdd_adapter *adapter,
				bool pairwise, u8 key_index, bool *ft_mode)
{
	struct wlan_objmgr_vdev *vdev;
	int errno;
	QDF_STATUS status;

	/* The supplicant may attempt to set the PTK once
	 * pre-authentication is done. Save the key in the
	 * UMAC and install it after association
	 */
	status = ucfg_cm_check_ft_status(pdev, adapter->vdev_id);
	if (status == QDF_STATUS_SUCCESS) {
		*ft_mode = true;
		return 0;
	}
	vdev = hdd_objmgr_get_vdev_by_user(adapter, WLAN_OSIF_ID);
	if (!vdev)
		return -EINVAL;
	errno = wlan_cfg80211_crypto_add_key(vdev, (pairwise ?
					     WLAN_CRYPTO_KEY_TYPE_UNICAST :
					     WLAN_CRYPTO_KEY_TYPE_GROUP),
					     key_index, true);
	hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_ID);
	if (!errno && adapter->send_mode_change) {
		wlan_hdd_send_mode_change_event();
		adapter->send_mode_change = false;
	}

	return errno;
}

static int __wlan_hdd_cfg80211_add_key(struct wiphy *wiphy,
				       struct net_device *ndev,
				       u8 key_index, bool pairwise,
				       const u8 *mac_addr,
				       struct key_params *params)
{
	struct hdd_context *hdd_ctx;
	mac_handle_t mac_handle;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(ndev);
	struct wlan_objmgr_vdev *vdev;
	bool ft_mode = false;
	enum wlan_crypto_cipher_type cipher;
	int errno;
	int32_t cipher_cap, ucast_cipher = 0;
	struct qdf_mac_addr mac_address;

	hdd_enter();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_vdev_id(adapter->vdev_id))
		return -EINVAL;

	qdf_mtrace(QDF_MODULE_ID_HDD, QDF_MODULE_ID_HDD,
		   TRACE_CODE_HDD_CFG80211_ADD_KEY,
		   adapter->vdev_id, params->key_len);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	errno = wlan_hdd_validate_context(hdd_ctx);
	if (errno)
		return errno;

	hdd_debug("converged Device_mode %s(%d) index %d, pairwise %d",
		  qdf_opmode_str(adapter->device_mode),
		  adapter->device_mode, key_index, pairwise);
	 mac_handle = hdd_ctx->mac_handle;

	if (hdd_is_btk_enc_type(params->cipher))
		return sme_add_key_btk(mac_handle, adapter->vdev_id,
				       params->key, params->key_len);
	if (hdd_is_krk_enc_type(params->cipher))
		return sme_add_key_krk(mac_handle, adapter->vdev_id,
				       params->key, params->key_len);

	vdev = hdd_objmgr_get_vdev_by_user(adapter, WLAN_OSIF_ID);
	if (!vdev)
		return -EINVAL;
	if (!pairwise && ((adapter->device_mode == QDF_STA_MODE) ||
	    (adapter->device_mode == QDF_P2P_CLIENT_MODE))) {
		qdf_mem_copy(mac_address.bytes,
			     adapter->session.station.conn_info.bssid.bytes,
			     QDF_MAC_ADDR_SIZE);
	} else {
		if (mac_addr)
			qdf_mem_copy(mac_address.bytes, mac_addr,
				     QDF_MAC_ADDR_SIZE);
	}
	errno = wlan_cfg80211_store_key(vdev, key_index,
					(pairwise ?
					WLAN_CRYPTO_KEY_TYPE_UNICAST :
					WLAN_CRYPTO_KEY_TYPE_GROUP),
					mac_address.bytes, params);
	cipher_cap = wlan_crypto_get_param(vdev, WLAN_CRYPTO_PARAM_CIPHER_CAP);
	hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_ID);
	if (errno)
		return errno;
	cipher = osif_nl_to_crypto_cipher_type(params->cipher);
	QDF_SET_PARAM(ucast_cipher, cipher);
	if (pairwise)
		wma_set_peer_ucast_cipher(mac_address.bytes,
					  ucast_cipher, cipher_cap);

	cdp_peer_flush_frags(cds_get_context(QDF_MODULE_ID_SOC),
			     wlan_vdev_get_id(vdev), mac_address.bytes);

	switch (adapter->device_mode) {
	case QDF_SAP_MODE:
	case QDF_P2P_GO_MODE:
		errno = wlan_hdd_add_key_sap(adapter, pairwise,
					     key_index, cipher);
		break;
	case QDF_STA_MODE:
	case QDF_P2P_CLIENT_MODE:
		errno = wlan_hdd_add_key_sta(hdd_ctx->pdev, adapter, pairwise,
					     key_index, &ft_mode);
		if (ft_mode)
			return 0;
		break;
	default:
		break;
	}
	if (!errno && (adapter->device_mode != QDF_SAP_MODE))
		wma_update_set_key(adapter->vdev_id, pairwise, key_index,
				   cipher);
	hdd_exit();

	return errno;
}

#ifdef CFG80211_KEY_INSTALL_SUPPORT_ON_WDEV
#ifdef CFG80211_SET_KEY_WITH_SRC_MAC
static int wlan_hdd_cfg80211_add_key(struct wiphy *wiphy,
				     struct wireless_dev *wdev,
				     u8 key_index, bool pairwise,
				     const u8 *src_addr,
				     const u8 *mac_addr,
				     struct key_params *params)
#else
static int wlan_hdd_cfg80211_add_key(struct wiphy *wiphy,
				     struct wireless_dev *wdev,
				     u8 key_index, bool pairwise,
				     const u8 *mac_addr,
				     struct key_params *params)
#endif
{
	int errno = -EINVAL;
	struct osif_vdev_sync *vdev_sync;
	struct hdd_adapter *adapter = qdf_container_of(wdev,
						   struct hdd_adapter,
						   wdev);

	if (!adapter || wlan_hdd_validate_vdev_id(adapter->vdev_id))
		return errno;

	errno = osif_vdev_sync_op_start(adapter->dev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_add_key(wiphy, adapter->dev, key_index,
					    pairwise, mac_addr, params);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}
#elif defined(CFG80211_SET_KEY_WITH_SRC_MAC)
static int wlan_hdd_cfg80211_add_key(struct wiphy *wiphy,
				     struct net_device *ndev,
				     u8 key_index, bool pairwise,
				     const u8 *src_addr,
				     const u8 *mac_addr,
				     struct key_params *params)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(ndev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_add_key(wiphy, ndev, key_index, pairwise,
					    mac_addr, params);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}
#elif defined(CFG80211_SINGLE_NETDEV_MULTI_LINK_SUPPORT)
static int wlan_hdd_cfg80211_add_key(struct wiphy *wiphy,
				     struct net_device *ndev,
				     int link_id, u8 key_index, bool pairwise,
				     const u8 *mac_addr,
				     struct key_params *params)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(ndev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_add_key(wiphy, ndev, key_index, pairwise,
					    mac_addr, params);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}
#else
static int wlan_hdd_cfg80211_add_key(struct wiphy *wiphy,
				     struct net_device *ndev,
				     u8 key_index, bool pairwise,
				     const u8 *mac_addr,
				     struct key_params *params)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(ndev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_add_key(wiphy, ndev, key_index, pairwise,
					    mac_addr, params);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}
#endif

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
	struct key_params params;
	eCsrEncryptionType enc_type;
	int32_t ucast_cipher = 0;

	hdd_enter();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_vdev_id(adapter->vdev_id))
		return -EINVAL;

	hdd_debug("Device_mode %s(%d)",
		  qdf_opmode_str(adapter->device_mode), adapter->device_mode);

	memset(&params, 0, sizeof(params));

	if (key_index >= (WLAN_CRYPTO_MAXKEYIDX + WLAN_CRYPTO_MAXIGTKKEYIDX +
			  WLAN_CRYPTO_MAXBIGTKKEYIDX)) {
		hdd_err("Invalid key index: %d", key_index);
		return -EINVAL;
	}
	if (adapter->vdev)
		ucast_cipher = wlan_crypto_get_param(adapter->vdev,
						WLAN_CRYPTO_PARAM_UCAST_CIPHER);

	sme_fill_enc_type(&enc_type, ucast_cipher);

	switch (enc_type) {
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

	qdf_mtrace(QDF_MODULE_ID_HDD, QDF_MODULE_ID_HDD,
		   TRACE_CODE_HDD_CFG80211_GET_KEY,
		   adapter->vdev_id, params.cipher);

	params.key_len = 0;
	params.seq_len = 0;
	params.seq = NULL;
	params.key = NULL;
	callback(cookie, &params);

	hdd_exit();
	return 0;
}

#ifdef CFG80211_KEY_INSTALL_SUPPORT_ON_WDEV
static int wlan_hdd_cfg80211_get_key(struct wiphy *wiphy,
				     struct wireless_dev *wdev,
				     u8 key_index, bool pairwise,
				     const u8 *mac_addr, void *cookie,
				     void (*callback)(void *cookie,
						      struct key_params *)
				     )
{
	int errno = -EINVAL;
	struct osif_vdev_sync *vdev_sync;
	struct hdd_adapter *adapter = qdf_container_of(wdev,
						   struct hdd_adapter,
						   wdev);

	if (!adapter || wlan_hdd_validate_vdev_id(adapter->vdev_id))
		return errno;

	errno = osif_vdev_sync_op_start(adapter->dev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_get_key(wiphy, adapter->dev, key_index,
					    pairwise, mac_addr, cookie,
					    callback);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}
#elif defined(CFG80211_SINGLE_NETDEV_MULTI_LINK_SUPPORT)
static int wlan_hdd_cfg80211_get_key(struct wiphy *wiphy,
				     struct net_device *ndev,
				     int link_id, u8 key_index, bool pairwise,
				     const u8 *mac_addr, void *cookie,
				     void (*callback)(void *cookie,
						      struct key_params *)
				     )
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(ndev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_get_key(wiphy, ndev, key_index, pairwise,
					    mac_addr, cookie, callback);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}
#else
static int wlan_hdd_cfg80211_get_key(struct wiphy *wiphy,
				     struct net_device *ndev,
				     u8 key_index, bool pairwise,
				     const u8 *mac_addr, void *cookie,
				     void (*callback)(void *cookie,
						      struct key_params *)
				     )
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(ndev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_get_key(wiphy, ndev, key_index, pairwise,
					    mac_addr, cookie, callback);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}
#endif

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
	hdd_exit();
	return 0;
}

/**
 * wlan_hdd_cfg80211_del_key() - cfg80211 delete key handler function
 * @wiphy: Pointer to wiphy structure.
 * @wdev: Pointer to wireless_dev structure.
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
#ifdef CFG80211_KEY_INSTALL_SUPPORT_ON_WDEV
static int wlan_hdd_cfg80211_del_key(struct wiphy *wiphy,
				     struct wireless_dev *wdev,
				     u8 key_index,
				     bool pairwise, const u8 *mac_addr)
{
	int errno = -EINVAL;
	struct osif_vdev_sync *vdev_sync;
	struct hdd_adapter *adapter = qdf_container_of(wdev,
						   struct hdd_adapter,
						   wdev);

	if (!adapter || wlan_hdd_validate_vdev_id(adapter->vdev_id))
		return errno;

	errno = osif_vdev_sync_op_start(adapter->dev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_del_key(wiphy, adapter->dev, key_index,
					    pairwise, mac_addr);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}
#elif defined(CFG80211_SINGLE_NETDEV_MULTI_LINK_SUPPORT)
static int wlan_hdd_cfg80211_del_key(struct wiphy *wiphy,
				     struct net_device *dev,
				     int link_id, u8 key_index,
				     bool pairwise, const u8 *mac_addr)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(dev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_del_key(wiphy, dev, key_index,
					    pairwise, mac_addr);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}
#else
static int wlan_hdd_cfg80211_del_key(struct wiphy *wiphy,
				     struct net_device *dev,
				     u8 key_index,
				     bool pairwise, const u8 *mac_addr)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(dev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_del_key(wiphy, dev, key_index,
					    pairwise, mac_addr);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}
#endif
static int __wlan_hdd_cfg80211_set_default_key(struct wiphy *wiphy,
					       struct net_device *ndev,
					       u8 key_index,
					       bool unicast, bool multicast)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(ndev);
	struct hdd_context *hdd_ctx;
	struct qdf_mac_addr bssid = QDF_MAC_ADDR_BCAST_INIT;
	struct hdd_ap_ctx *ap_ctx;
	struct wlan_crypto_key *crypto_key;
	struct wlan_objmgr_vdev *vdev;
	int ret;
	QDF_STATUS status;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_vdev_id(adapter->vdev_id))
		return -EINVAL;

	qdf_mtrace(QDF_MODULE_ID_HDD, QDF_MODULE_ID_HDD,
		   TRACE_CODE_HDD_CFG80211_SET_DEFAULT_KEY,
		   adapter->vdev_id, key_index);

	hdd_debug("Device_mode %s(%d) key_index = %d",
		  qdf_opmode_str(adapter->device_mode),
		  adapter->device_mode, key_index);

	if (key_index >= (WLAN_CRYPTO_MAXKEYIDX + WLAN_CRYPTO_MAXIGTKKEYIDX +
			  WLAN_CRYPTO_MAXBIGTKKEYIDX)) {
		hdd_err("Invalid key index: %d", key_index);
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);

	if (0 != ret)
		return ret;

	vdev = hdd_objmgr_get_vdev_by_user(adapter, WLAN_OSIF_ID);
	if (!vdev)
		return -EINVAL;

	crypto_key = wlan_crypto_get_key(vdev, key_index);
	if (!crypto_key) {
		hdd_err("Invalid NULL key info");
		ret = -EINVAL;
		goto out;
	}
	hdd_debug("unicast %d, multicast %d cipher %d",
		  unicast, multicast, crypto_key->cipher_type);
	if (!IS_WEP_CIPHER(crypto_key->cipher_type)) {
		ret = 0;
		goto out;
	}

	if ((adapter->device_mode == QDF_STA_MODE) ||
	    (adapter->device_mode == QDF_P2P_CLIENT_MODE)) {
		ret =
		wlan_cfg80211_crypto_add_key(vdev,
					     (unicast ?
					      WLAN_CRYPTO_KEY_TYPE_UNICAST :
					      WLAN_CRYPTO_KEY_TYPE_GROUP),
					     key_index, true);
		wma_update_set_key(adapter->vdev_id, unicast, key_index,
				   crypto_key->cipher_type);
	}

	if (adapter->device_mode == QDF_SAP_MODE ||
	    adapter->device_mode == QDF_P2P_GO_MODE) {
		status = wlan_cfg80211_set_default_key(vdev, key_index,
						       &bssid);
		if (QDF_IS_STATUS_ERROR(status)) {
			hdd_err("ret fail status %d", ret);
			ret = -EINVAL;
			goto out;
		}
		ap_ctx = WLAN_HDD_GET_AP_CTX_PTR(adapter);
		ap_ctx->wep_def_key_idx = key_index;
	}

out:
	hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_ID);
	return ret;
}

#ifdef CFG80211_KEY_INSTALL_SUPPORT_ON_WDEV
static int wlan_hdd_cfg80211_set_default_key(struct wiphy *wiphy,
					     struct wireless_dev *wdev,
					     u8 key_index,
					     bool unicast, bool multicast)
{
	int errno = -EINVAL;
	struct osif_vdev_sync *vdev_sync;
	struct hdd_adapter *adapter = qdf_container_of(wdev,
						   struct hdd_adapter,
						   wdev);

	if (!adapter || wlan_hdd_validate_vdev_id(adapter->vdev_id))
		return errno;

	errno = osif_vdev_sync_op_start(adapter->dev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_set_default_key(wiphy, adapter->dev,
						    key_index, unicast,
						    multicast);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}
#elif defined(CFG80211_SINGLE_NETDEV_MULTI_LINK_SUPPORT)
static int wlan_hdd_cfg80211_set_default_key(struct wiphy *wiphy,
					     struct net_device *ndev,
					     int link_id, u8 key_index,
					     bool unicast, bool multicast)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(ndev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_set_default_key(wiphy, ndev, key_index,
						    unicast, multicast);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}
#else
static int wlan_hdd_cfg80211_set_default_key(struct wiphy *wiphy,
					     struct net_device *ndev,
					     u8 key_index,
					     bool unicast, bool multicast)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(ndev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_set_default_key(wiphy, ndev, key_index,
						    unicast, multicast);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}
#endif

#if defined (CFG80211_BIGTK_CONFIGURATION_SUPPORT) || \
	    (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0))
static int _wlan_hdd_cfg80211_set_default_beacon_key(struct wiphy *wiphy,
						     struct net_device *ndev,
						     u8 key_index)
{
	hdd_enter();
	return 0;
}

#ifdef CFG80211_KEY_INSTALL_SUPPORT_ON_WDEV
static int wlan_hdd_cfg80211_set_default_beacon_key(struct wiphy *wiphy,
						    struct wireless_dev *wdev,
						    u8 key_index)
{
	int errno = -EINVAL;
	struct osif_vdev_sync *vdev_sync;
	struct hdd_adapter *adapter = qdf_container_of(wdev,
						   struct hdd_adapter,
						   wdev);

	if (!adapter || wlan_hdd_validate_vdev_id(adapter->vdev_id))
		return errno;

	errno = osif_vdev_sync_op_start(adapter->dev, &vdev_sync);
	if (errno)
		return errno;

	errno = _wlan_hdd_cfg80211_set_default_beacon_key(wiphy, adapter->dev,
							  key_index);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}
#elif defined(CFG80211_SINGLE_NETDEV_MULTI_LINK_SUPPORT)
static int wlan_hdd_cfg80211_set_default_beacon_key(struct wiphy *wiphy,
						    struct net_device *ndev,
						    int link_id, u8 key_index)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(ndev, &vdev_sync);
	if (errno)
		return errno;

	errno = _wlan_hdd_cfg80211_set_default_beacon_key(wiphy, ndev,
							  key_index);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}
#else
static int wlan_hdd_cfg80211_set_default_beacon_key(struct wiphy *wiphy,
						    struct net_device *ndev,
						    u8 key_index)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(ndev, &vdev_sync);
	if (errno)
		return errno;

	errno = _wlan_hdd_cfg80211_set_default_beacon_key(wiphy, ndev,
							  key_index);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}
#endif
#endif

#ifdef FEATURE_MONITOR_MODE_SUPPORT
static
void hdd_mon_select_cbmode(struct hdd_adapter *adapter,
			   uint32_t op_freq,
			   struct ch_params *ch_params)
{
	struct hdd_station_ctx *station_ctx =
			 WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	struct hdd_mon_set_ch_info *ch_info = &station_ctx->ch_info;
	enum hdd_dot11_mode hdd_dot11_mode;
	uint8_t ini_dot11_mode =
			(WLAN_HDD_GET_CTX(adapter))->config->dot11Mode;

	hdd_debug("Dot11Mode is %u", ini_dot11_mode);
	switch (ini_dot11_mode) {
	case eHDD_DOT11_MODE_AUTO:
#ifdef WLAN_FEATURE_11BE
	case eHDD_DOT11_MODE_11be:
	case eHDD_DOT11_MODE_11be_ONLY:
		if (sme_is_feature_supported_by_fw(DOT11BE))
			hdd_dot11_mode = eHDD_DOT11_MODE_11be;
		else
#endif
		if (sme_is_feature_supported_by_fw(DOT11AX))
			hdd_dot11_mode = eHDD_DOT11_MODE_11ax;
		else if (sme_is_feature_supported_by_fw(DOT11AC))
			hdd_dot11_mode = eHDD_DOT11_MODE_11ac;
		else
			hdd_dot11_mode = eHDD_DOT11_MODE_11n;
		break;
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
		hdd_dot11_mode = ini_dot11_mode;
		break;
	}
	ch_info->channel_width = ch_params->ch_width;
	ch_info->phy_mode =
		hdd_cfg_xlate_to_csr_phy_mode(hdd_dot11_mode);
	ch_info->freq = op_freq;
	ch_info->cb_mode = ch_params->ch_width;
	hdd_debug("ch_info width %d, phymode %d channel freq %d",
		  ch_info->channel_width, ch_info->phy_mode,
		  ch_info->freq);
}
#else
static
void hdd_mon_select_cbmode(struct hdd_adapter *adapter,
			   uint32_t op_freq,
			   struct ch_params *ch_params)
{
}
#endif

void hdd_select_cbmode(struct hdd_adapter *adapter, qdf_freq_t oper_freq,
		       qdf_freq_t sec_ch_2g_freq, struct ch_params *ch_params)
{
	uint32_t sec_ch_freq = 0;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	/*
	 * CDS api expects secondary channel for calculating
	 * the channel params
	 */
	if (ch_params->ch_width == CH_WIDTH_40MHZ &&
	    WLAN_REG_IS_24GHZ_CH_FREQ(oper_freq)) {
		if (sec_ch_2g_freq) {
			sec_ch_freq = sec_ch_2g_freq;
		} else {
			if (oper_freq >= 2412 && oper_freq <= 2432)
				sec_ch_freq = oper_freq + 20;
			else if (oper_freq >= 2437 && oper_freq <= 2472)
				sec_ch_freq = oper_freq - 20;
		}
	}

	/* This call decides required channel bonding mode */
	wlan_reg_set_channel_params_for_freq(hdd_ctx->pdev, oper_freq,
					     sec_ch_freq, ch_params);

	if (cds_get_conparam() == QDF_GLOBAL_MONITOR_MODE ||
	    policy_mgr_is_sta_mon_concurrency(hdd_ctx->psoc))
		hdd_mon_select_cbmode(adapter, oper_freq, ch_params);
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
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(ndev, &vdev_sync);
	if (errno)
		return errno;

	errno = wlan_hdd_cm_connect(wiphy, ndev, req);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
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
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(dev, &vdev_sync);
	if (errno)
		return errno;

	errno = wlan_hdd_cm_disconnect(wiphy, dev, reason);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
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
	int status;

	hdd_enter();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	qdf_mtrace(QDF_MODULE_ID_HDD, QDF_MODULE_ID_HDD,
		   TRACE_CODE_HDD_CFG80211_SET_WIPHY_PARAMS,
		   NO_SESSION, wiphy->rts_threshold);

	status = wlan_hdd_validate_context(hdd_ctx);

	if (0 != status)
		return status;

	if (changed & WIPHY_PARAM_RTS_THRESHOLD) {
		u32 rts_threshold = (wiphy->rts_threshold == -1) ?
				     cfg_max(CFG_RTS_THRESHOLD) :
				     wiphy->rts_threshold;

		if ((cfg_min(CFG_RTS_THRESHOLD) > rts_threshold) ||
		    (cfg_max(CFG_RTS_THRESHOLD) < rts_threshold)) {
			hdd_err("Invalid RTS Threshold value: %u",
				rts_threshold);
			return -EINVAL;
		}

		if (0 != ucfg_mlme_set_rts_threshold(hdd_ctx->psoc,
		    rts_threshold)) {
			hdd_err("mlme_set_rts_threshold failed for val %u",
				rts_threshold);
			return -EIO;
		}

		hdd_debug("set rts threshold %u", rts_threshold);
	}

	if (changed & WIPHY_PARAM_FRAG_THRESHOLD) {
		u16 frag_threshold = (wiphy->frag_threshold == -1) ?
				     cfg_max(CFG_FRAG_THRESHOLD) :
				     wiphy->frag_threshold;

		if ((cfg_min(CFG_FRAG_THRESHOLD) > frag_threshold) ||
		    (cfg_max(CFG_FRAG_THRESHOLD) < frag_threshold)) {
			hdd_err("Invalid frag_threshold value %hu",
				frag_threshold);
			return -EINVAL;
		}

		if (0 != ucfg_mlme_set_frag_threshold(hdd_ctx->psoc,
						      frag_threshold)) {
			hdd_err("mlme_set_frag_threshold failed for val %hu",
				frag_threshold);
			return -EIO;
		}

		hdd_debug("set frag threshold %hu", frag_threshold);
	}

	hdd_exit();
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
	struct osif_psoc_sync *psoc_sync;
	int errno;

	errno = osif_psoc_sync_op_start(wiphy_dev(wiphy), &psoc_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_set_wiphy_params(wiphy, changed);

	osif_psoc_sync_op_stop(psoc_sync);

	return errno;
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
	hdd_enter();
	return 0;
}

/**
 * wlan_hdd_set_default_mgmt_key() - SSR wrapper for
 *				wlan_hdd_set_default_mgmt_key
 * @wiphy: pointer to wiphy
 * @wdev: pointer to wireless_device structure
 * @key_index: key index
 *
 * Return: 0 on success, error number on failure
 */
#ifdef CFG80211_KEY_INSTALL_SUPPORT_ON_WDEV
static int wlan_hdd_set_default_mgmt_key(struct wiphy *wiphy,
					 struct wireless_dev *wdev,
					 u8 key_index)
{
	int errno = -EINVAL;
	struct osif_vdev_sync *vdev_sync;
	struct hdd_adapter *adapter = qdf_container_of(wdev,
						   struct hdd_adapter,
						   wdev);

	if (!adapter || wlan_hdd_validate_vdev_id(adapter->vdev_id))
		return errno;

	errno = osif_vdev_sync_op_start(adapter->dev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_set_default_mgmt_key(wiphy, adapter->dev, key_index);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}
#elif defined(CFG80211_SINGLE_NETDEV_MULTI_LINK_SUPPORT)
static int wlan_hdd_set_default_mgmt_key(struct wiphy *wiphy,
					 struct net_device *netdev,
					 int link_id, u8 key_index)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_set_default_mgmt_key(wiphy, netdev, key_index);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}
#else
static int wlan_hdd_set_default_mgmt_key(struct wiphy *wiphy,
					 struct net_device *netdev,
					 u8 key_index)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_set_default_mgmt_key(wiphy, netdev, key_index);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}
#endif

/**
 * Default val of cwmin, this value is used to overide the
 * incorrect user set value
 */
#define DEFAULT_CWMIN 15

/**
 * Default val of cwmax, this value is used to overide the
 * incorrect user set value
 */
#define DEFAULT_CWMAX 1023

/**
 * __wlan_hdd_set_txq_params() - implementation of set tx queue params
 *				to configure internal EDCA parameters
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
	QDF_STATUS status;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	mac_handle_t mac_handle;
	tSirMacEdcaParamRecord txq_edca_params;
	static const uint8_t ieee_ac_to_qca_ac[] = {
		[IEEE80211_AC_VO] = QCA_WLAN_AC_VO,
		[IEEE80211_AC_VI] = QCA_WLAN_AC_VI,
		[IEEE80211_AC_BE] = QCA_WLAN_AC_BE,
		[IEEE80211_AC_BK] = QCA_WLAN_AC_BK,
	};

	hdd_enter();

	if (wlan_hdd_validate_context(hdd_ctx))
		return -EINVAL;

	mac_handle = hdd_ctx->mac_handle;
	if (params->cwmin == 0 || params->cwmin > DEFAULT_CWMAX)
		params->cwmin = DEFAULT_CWMIN;

	if (params->cwmax < params->cwmin || params->cwmax > DEFAULT_CWMAX)
		params->cwmax = DEFAULT_CWMAX;

	txq_edca_params.cw.min = convert_cw(params->cwmin);
	txq_edca_params.cw.max = convert_cw(params->cwmax);
	txq_edca_params.aci.aifsn = params->aifs;
	/* The txop is multiple of 32us units */
	txq_edca_params.txoplimit = params->txop;
	txq_edca_params.aci.aci =
			ieee_ac_to_qca_ac[params->ac];

	status = sme_update_session_txq_edca_params(mac_handle,
						    adapter->vdev_id,
						    &txq_edca_params);

	hdd_exit();
	return qdf_status_to_os_return(status);
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
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(dev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_set_txq_params(wiphy, dev, params);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}

/**
 * hdd_softap_deauth_current_sta() - Deauth current sta
 * @sta_info: pointer to the current station info structure
 * @adapter: pointer to adapter structure
 * @hdd_ctx: pointer to hdd context
 * @hapd_state: pointer to hostapd state structure
 * @param: pointer to del sta params
 *
 * Return: QDF_STATUS on success, corresponding QDF failure status on failure
 */
static
QDF_STATUS hdd_softap_deauth_current_sta(struct hdd_adapter *adapter,
					 struct hdd_station_info *sta_info,
					 struct hdd_hostapd_state *hapd_state,
					 struct csr_del_sta_params *param)
{
	qdf_event_t *disassoc_event = &hapd_state->qdf_sta_disassoc_event;
	struct hdd_context *hdd_ctx;
	QDF_STATUS qdf_status;
	struct hdd_station_info *tmp = NULL;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (!hdd_ctx) {
		hdd_err("hdd_ctx is NULL");
		return QDF_STATUS_E_INVAL;
	}

	qdf_event_reset(&hapd_state->qdf_sta_disassoc_event);

	if (!qdf_is_macaddr_broadcast(&param->peerMacAddr))
		sme_send_disassoc_req_frame(hdd_ctx->mac_handle,
					    adapter->vdev_id,
					    (uint8_t *)&param->peerMacAddr,
					    param->reason_code, 0);

	qdf_status = hdd_softap_sta_deauth(adapter, param);

	if (QDF_IS_STATUS_SUCCESS(qdf_status)) {
		if(qdf_is_macaddr_broadcast(&sta_info->sta_mac)) {
			hdd_for_each_sta_ref_safe(
					adapter->sta_info_list,
					sta_info, tmp,
					STA_INFO_SOFTAP_DEAUTH_CURRENT_STA) {
				sta_info->is_deauth_in_progress = true;
				hdd_put_sta_info_ref(
					&adapter->sta_info_list,
					&sta_info, true,
					STA_INFO_SOFTAP_DEAUTH_CURRENT_STA);
			}
		} else {
			sta_info->is_deauth_in_progress = true;
		}
		qdf_status = qdf_wait_for_event_completion(
						disassoc_event,
						SME_PEER_DISCONNECT_TIMEOUT);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			hdd_warn("Deauth time expired");
	} else {
		sta_info->is_deauth_in_progress = false;
		hdd_debug("STA removal failed for ::" QDF_MAC_ADDR_FMT,
			  QDF_MAC_ADDR_REF(sta_info->sta_mac.bytes));
		return QDF_STATUS_E_NOENT;
	}
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS hdd_softap_deauth_all_sta(struct hdd_adapter *adapter,
				     struct hdd_hostapd_state *hapd_state,
				     struct csr_del_sta_params *param)
{
	QDF_STATUS status;
	bool is_sap_bcast_deauth_enabled = false;
	struct hdd_context *hdd_ctx;
	struct hdd_station_info *sta_info, *tmp = NULL;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (!hdd_ctx) {
		hdd_err("hdd_ctx is NULL");
		return QDF_STATUS_E_INVAL;
	}

	ucfg_mlme_get_sap_bcast_deauth_enabled(hdd_ctx->psoc,
					       &is_sap_bcast_deauth_enabled);

	hdd_debug("sap_bcast_deauth_enabled %d", is_sap_bcast_deauth_enabled);

	if (is_sap_bcast_deauth_enabled) {
		struct hdd_station_info bcast_sta_info;

		qdf_set_macaddr_broadcast(&bcast_sta_info.sta_mac);
		return hdd_softap_deauth_current_sta(adapter, &bcast_sta_info,
						     hapd_state, param);
	}

	hdd_for_each_sta_ref_safe(adapter->sta_info_list, sta_info, tmp,
				  STA_INFO_SOFTAP_DEAUTH_ALL_STA) {
		if (!sta_info->is_deauth_in_progress) {
			hdd_debug("Delete STA with MAC:" QDF_MAC_ADDR_FMT,
				  QDF_MAC_ADDR_REF(sta_info->sta_mac.bytes));

			if (QDF_IS_ADDR_BROADCAST(sta_info->sta_mac.bytes)) {
				hdd_put_sta_info_ref(&adapter->sta_info_list,
						&sta_info, true,
						STA_INFO_SOFTAP_DEAUTH_ALL_STA);
				continue;
			}

			qdf_mem_copy(param->peerMacAddr.bytes,
				     sta_info->sta_mac.bytes,
				     QDF_MAC_ADDR_SIZE);
			status =
			    hdd_softap_deauth_current_sta(adapter, sta_info,
							  hapd_state, param);
			if (QDF_IS_STATUS_ERROR(status)) {
				hdd_put_sta_info_ref(
						&adapter->sta_info_list,
						&sta_info, true,
						STA_INFO_SOFTAP_DEAUTH_ALL_STA);
				if (tmp)
					hdd_put_sta_info_ref(
						&adapter->sta_info_list,
						&tmp, true,
						STA_INFO_SOFTAP_DEAUTH_ALL_STA);
				return status;
			}
		}
		hdd_put_sta_info_ref(&adapter->sta_info_list, &sta_info, true,
				     STA_INFO_SOFTAP_DEAUTH_ALL_STA);
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * __wlan_hdd_cfg80211_del_station() - delete station v2
 * @wiphy: Pointer to wiphy
 * @dev: Underlying net device
 * @param: Pointer to delete station parameter
 *
 * Return: 0 for success, non-zero for failure
 */
static
int __wlan_hdd_cfg80211_del_station(struct wiphy *wiphy,
				    struct net_device *dev,
				    struct csr_del_sta_params *param)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx;
	struct hdd_hostapd_state *hapd_state;
	uint8_t *mac;
	struct hdd_station_info *sta_info;

	hdd_enter();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_vdev_id(adapter->vdev_id))
		return -EINVAL;

	qdf_mtrace(QDF_MODULE_ID_HDD, QDF_MODULE_ID_HDD,
		   TRACE_CODE_HDD_CFG80211_DEL_STA,
		   adapter->vdev_id, adapter->device_mode);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (!hdd_ctx) {
		hdd_err("hdd_ctx is NULL");
		return -EINVAL;
	}

	mac = (uint8_t *) param->peerMacAddr.bytes;

	if (QDF_SAP_MODE != adapter->device_mode &&
	    QDF_P2P_GO_MODE != adapter->device_mode)
		goto fn_end;

	hapd_state = WLAN_HDD_GET_HOSTAP_STATE_PTR(adapter);
	if (!hapd_state) {
		hdd_err("Hostapd State is Null");
		return 0;
	}

	if (qdf_is_macaddr_broadcast((struct qdf_mac_addr *)mac)) {
		if (!QDF_IS_STATUS_SUCCESS(hdd_softap_deauth_all_sta(adapter,
								     hapd_state,
								     param)))
			goto fn_end;
	} else {
		if (param->reason_code == REASON_1X_AUTH_FAILURE)
			hdd_softap_check_wait_for_tx_eap_pkt(
					adapter, (struct qdf_mac_addr *)mac);

		sta_info = hdd_get_sta_info_by_mac(
						&adapter->sta_info_list,
						mac,
						STA_INFO_CFG80211_DEL_STATION);

		if (!sta_info) {
			hdd_debug("Skip DEL STA as this is not used::"
				  QDF_MAC_ADDR_FMT,
				  QDF_MAC_ADDR_REF(mac));
			return -ENOENT;
		}

		if (sta_info->is_deauth_in_progress) {
			hdd_debug("Skip DEL STA as deauth is in progress::"
				  QDF_MAC_ADDR_FMT,
				  QDF_MAC_ADDR_REF(mac));
			hdd_put_sta_info_ref(&adapter->sta_info_list, &sta_info,
					     true,
					     STA_INFO_CFG80211_DEL_STATION);
			return -ENOENT;
		}

		hdd_debug("ucast, Delete STA with MAC:" QDF_MAC_ADDR_FMT,
			  QDF_MAC_ADDR_REF(mac));
		hdd_softap_deauth_current_sta(adapter, sta_info, hapd_state,
					      param);
		hdd_put_sta_info_ref(&adapter->sta_info_list, &sta_info, true,
				     STA_INFO_CFG80211_DEL_STATION);
	}

fn_end:
	hdd_exit();
	return 0;
}

#if defined(USE_CFG80211_DEL_STA_V2)
int wlan_hdd_del_station(struct hdd_adapter *adapter, const uint8_t *mac)
{
	struct station_del_parameters del_sta;

	del_sta.mac = mac;
	del_sta.subtype = IEEE80211_STYPE_DEAUTH >> 4;
	del_sta.reason_code = WLAN_REASON_DEAUTH_LEAVING;

	return wlan_hdd_cfg80211_del_station(adapter->wdev.wiphy,
					     adapter->dev, &del_sta);
}
#else
int wlan_hdd_del_station(struct hdd_adapter *adapter, const uint8_t *mac)
{
	return wlan_hdd_cfg80211_del_station(adapter->wdev.wiphy,
					     adapter->dev, mac);
}
#endif

/**
 * wlan_hdd_cfg80211_del_station() - delete station entry handler
 * @wiphy: Pointer to wiphy
 * @dev: net_device to operate against
 * @mac: binary mac address
 * @reason_code: reason for the deauthorization/disassociation
 * @subtype: management frame subtype to indicate removal
 *
 * Return: Errno
 */
static int _wlan_hdd_cfg80211_del_station(struct wiphy *wiphy,
					  struct net_device *dev,
					  const uint8_t *mac,
					  uint16_t reason_code,
					  uint8_t subtype)
{
	int errno;
	struct csr_del_sta_params delStaParams;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(dev, &vdev_sync);
	if (errno)
		return errno;

	wlansap_populate_del_sta_params(mac, reason_code, subtype,
					&delStaParams);
	errno = __wlan_hdd_cfg80211_del_station(wiphy, dev, &delStaParams);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}

#ifdef USE_CFG80211_DEL_STA_V2
int wlan_hdd_cfg80211_del_station(struct wiphy *wiphy,
				  struct net_device *dev,
				  struct station_del_parameters *param)
{
	if (!param)
		return -EINVAL;

	return _wlan_hdd_cfg80211_del_station(wiphy, dev, param->mac,
					      param->reason_code,
					      param->subtype);
}
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
int wlan_hdd_cfg80211_del_station(struct wiphy *wiphy, struct net_device *dev,
				  const uint8_t *mac)
{
	uint16_t reason = REASON_DEAUTH_NETWORK_LEAVING;
	uint8_t subtype = SIR_MAC_MGMT_DEAUTH >> 4;

	return _wlan_hdd_cfg80211_del_station(wiphy, dev, mac, reason, subtype);
}
#else
int wlan_hdd_cfg80211_del_station(struct wiphy *wiphy, struct net_device *dev,
				  uint8_t *mac)
{
	uint16_t reason = REASON_DEAUTH_NETWORK_LEAVING;
	uint8_t subtype = SIR_MAC_MGMT_DEAUTH >> 4;

	return _wlan_hdd_cfg80211_del_station(wiphy, dev, mac, reason, subtype);
}
#endif

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

	hdd_enter();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_vdev_id(adapter->vdev_id))
		return -EINVAL;

	qdf_mtrace(QDF_MODULE_ID_HDD, QDF_MODULE_ID_HDD,
		   TRACE_CODE_HDD_CFG80211_ADD_STA,
		   adapter->vdev_id, params->listen_interval);

	if (0 != wlan_hdd_validate_context(hdd_ctx))
		return -EINVAL;

	mask = params->sta_flags_mask;

	set = params->sta_flags_set;

	hdd_debug("mask 0x%x set 0x%x " QDF_MAC_ADDR_FMT, mask, set,
		  QDF_MAC_ADDR_REF(mac));

	if (mask & BIT(NL80211_STA_FLAG_TDLS_PEER)) {
		if (set & BIT(NL80211_STA_FLAG_TDLS_PEER)) {
			struct wlan_objmgr_vdev *vdev;

			vdev = hdd_objmgr_get_vdev_by_user(adapter,
							   WLAN_OSIF_TDLS_ID);
			if (vdev) {
				status = wlan_cfg80211_tdls_add_peer(vdev,
								     mac);
				hdd_objmgr_put_vdev_by_user(vdev,
							    WLAN_OSIF_TDLS_ID);
			}
		}
	}
#endif
	hdd_exit();
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
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(dev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_add_station(wiphy, dev, mac, params);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}

#if (defined(CFG80211_CONFIG_PMKSA_TIMER_PARAMS_SUPPORT) || \
	     (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0)))
static inline void
hdd_fill_pmksa_lifetime(struct cfg80211_pmksa *pmksa,
			struct wlan_crypto_pmksa *pmk_cache)
{
	pmk_cache->pmk_lifetime = pmksa->pmk_lifetime;
	if (pmk_cache->pmk_lifetime > WLAN_CRYPTO_MAX_PMKID_LIFETIME)
		pmk_cache->pmk_lifetime = WLAN_CRYPTO_MAX_PMKID_LIFETIME;

	pmk_cache->pmk_lifetime_threshold = pmksa->pmk_reauth_threshold;
	if (pmk_cache->pmk_lifetime_threshold >=
	    WLAN_CRYPTO_MAX_PMKID_LIFETIME_THRESHOLD)
		pmk_cache->pmk_lifetime_threshold =
			WLAN_CRYPTO_MAX_PMKID_LIFETIME_THRESHOLD - 1;

	hdd_debug("PMKSA: lifetime:%d threshold:%d",  pmk_cache->pmk_lifetime,
		  pmk_cache->pmk_lifetime_threshold);
}
#else
static inline void
hdd_fill_pmksa_lifetime(struct cfg80211_pmksa *pmksa,
			struct wlan_crypto_pmksa *src_pmk_cache)
{}
#endif

static QDF_STATUS wlan_hdd_set_pmksa_cache(struct hdd_adapter *adapter,
					   struct wlan_crypto_pmksa *pmk_cache)
{
	QDF_STATUS result;
	struct wlan_crypto_pmksa *pmksa;
	struct wlan_objmgr_vdev *vdev;
	mac_handle_t mac_handle;
	struct hdd_context *hdd_ctx;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (!hdd_ctx) {
		hdd_err("HDD context is null");
		return QDF_STATUS_E_INVAL;
	}

	if (wlan_hdd_validate_context(hdd_ctx))
		return QDF_STATUS_E_INVAL;
	mac_handle = hdd_ctx->mac_handle;

	vdev = hdd_objmgr_get_vdev_by_user(adapter, WLAN_OSIF_ID);
	if (!vdev)
		return QDF_STATUS_E_FAILURE;

	pmksa = qdf_mem_malloc(sizeof(*pmksa));
	if (!pmksa) {
		hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_ID);
		return QDF_STATUS_E_NOMEM;
	}

	if (!pmk_cache->ssid_len) {
		qdf_copy_macaddr(&pmksa->bssid, &pmk_cache->bssid);
	} else {
		qdf_mem_copy(pmksa->ssid, pmk_cache->ssid, pmk_cache->ssid_len);
		qdf_mem_copy(pmksa->cache_id, pmk_cache->cache_id,
			     WLAN_CACHE_ID_LEN);
		pmksa->ssid_len = pmk_cache->ssid_len;
	}
	qdf_mem_copy(pmksa->pmkid, pmk_cache->pmkid, PMKID_LEN);
	qdf_mem_copy(pmksa->pmk, pmk_cache->pmk, pmk_cache->pmk_len);
	pmksa->pmk_len = pmk_cache->pmk_len;
	pmksa->pmk_entry_ts = qdf_get_system_timestamp();
	pmksa->pmk_lifetime = pmk_cache->pmk_lifetime;
	pmksa->pmk_lifetime_threshold = pmk_cache->pmk_lifetime_threshold;

	result = wlan_crypto_set_del_pmksa(vdev, pmksa, true);
	if (result != QDF_STATUS_SUCCESS) {
		qdf_mem_zero(pmksa, sizeof(*pmksa));
		qdf_mem_free(pmksa);
	}

	if (result == QDF_STATUS_SUCCESS && pmk_cache->pmk_len) {
		sme_roam_set_psk_pmk(mac_handle, pmksa, adapter->vdev_id,
				     false);
		sme_set_pmk_cache_ft(mac_handle, adapter->vdev_id, pmk_cache);
	}
	hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_ID);

	return result;
}

static QDF_STATUS wlan_hdd_del_pmksa_cache(struct hdd_adapter *adapter,
					   struct wlan_crypto_pmksa *pmk_cache)
{
	QDF_STATUS result;
	struct wlan_crypto_pmksa pmksa;
	struct wlan_objmgr_vdev *vdev;

	vdev = hdd_objmgr_get_vdev_by_user(adapter, WLAN_OSIF_ID);
	if (!vdev)
		return QDF_STATUS_E_FAILURE;

	qdf_mem_zero(&pmksa, sizeof(pmksa));
	if (!pmk_cache->ssid_len) {
		qdf_copy_macaddr(&pmksa.bssid, &pmk_cache->bssid);
	} else {
		qdf_mem_copy(pmksa.ssid, pmk_cache->ssid, pmk_cache->ssid_len);
		qdf_mem_copy(pmksa.cache_id, pmk_cache->cache_id,
			     WLAN_CACHE_ID_LEN);
		pmksa.ssid_len = pmk_cache->ssid_len;
	}

	result = wlan_crypto_set_del_pmksa(vdev, &pmksa, false);
	hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_ID);

	return result;
}

QDF_STATUS wlan_hdd_flush_pmksa_cache(struct hdd_adapter *adapter)
{
	QDF_STATUS result;
	struct wlan_objmgr_vdev *vdev;

	vdev = hdd_objmgr_get_vdev_by_user(adapter, WLAN_OSIF_ID);
	if (!vdev)
		return QDF_STATUS_E_FAILURE;

	result = wlan_crypto_set_del_pmksa(vdev, NULL, false);
	hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_ID);

	return result;
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
		hdd_warn("bssid is NULL");
		if (!pmksa->ssid || !pmksa->cache_id) {
			hdd_err("either ssid or cache_id are NULL");
			return false;
		}
	}
	return true;
}

/*
 * hdd_fill_pmksa_info: API to update tPmkidCacheInfo from cfg80211_pmksa
 * @adapter: Pointer to hdd adapter
 * @pmk_cache: pmk that needs to be udated
 * @pmksa: pmk from supplicant
 * @is_delete: Bool to decide set or delete PMK
 * Return: None
 */
static void hdd_fill_pmksa_info(struct hdd_adapter *adapter,
				struct wlan_crypto_pmksa *pmk_cache,
				struct cfg80211_pmksa *pmksa, bool is_delete)
{
	if (pmksa->bssid) {
		hdd_debug("%s PMKSA for " QDF_MAC_ADDR_FMT,
			  is_delete ? "Delete" : "Set",
			  QDF_MAC_ADDR_REF(pmksa->bssid));
		qdf_mem_copy(pmk_cache->bssid.bytes,
			     pmksa->bssid, QDF_MAC_ADDR_SIZE);
	} else {
		qdf_mem_copy(pmk_cache->ssid, pmksa->ssid, pmksa->ssid_len);
		qdf_mem_copy(pmk_cache->cache_id, pmksa->cache_id,
			     CACHE_ID_LEN);
		pmk_cache->ssid_len = pmksa->ssid_len;
		hdd_debug("%s PMKSA for ssid %*.*s cache_id %x %x",
			  is_delete ? "Delete" : "Set",
			  pmk_cache->ssid_len, pmk_cache->ssid_len,
			  pmk_cache->ssid, pmk_cache->cache_id[0],
			  pmk_cache->cache_id[1]);
	}

	hdd_fill_pmksa_lifetime(pmksa, pmk_cache);

	if (is_delete)
		return;

	qdf_mem_copy(pmk_cache->pmkid, pmksa->pmkid, PMKID_LEN);
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
 * hdd_fill_pmksa_info: API to update struct wlan_crypto_pmksa from
 * cfg80211_pmksa
 * @adapter: Pointer to hdd adapter
 * @pmk_cache: pmk which needs to be updated
 * @pmksa: pmk from supplicant
 * @is_delete: Bool to decide whether to set or delete PMK
 *
 * Return: None
 */
static void hdd_fill_pmksa_info(struct hdd_adapter *adapter,
				struct wlan_crypto_pmksa *pmk_cache,
				struct cfg80211_pmksa *pmksa, bool is_delete)
{
	mac_handle_t mac_handle;

	hdd_debug("%s PMKSA for " QDF_MAC_ADDR_FMT, is_delete ? "Delete" : "Set",
		  QDF_MAC_ADDR_REF(pmksa->bssid));
	qdf_mem_copy(pmk_cache->bssid.bytes, pmksa->bssid, QDF_MAC_ADDR_SIZE);

	if (is_delete)
		return;
	mac_handle = hdd_adapter_get_mac_handle(adapter);
	sme_get_pmk_info(mac_handle, adapter->vdev_id, pmk_cache);
	qdf_mem_copy(pmk_cache->pmkid, pmksa->pmkid, PMKID_LEN);
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
	QDF_STATUS result = QDF_STATUS_SUCCESS;
	int status;
	struct wlan_crypto_pmksa *pmk_cache;

	hdd_enter();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_vdev_id(adapter->vdev_id))
		return -EINVAL;

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

	pmk_cache = qdf_mem_malloc(sizeof(*pmk_cache));
	if (!pmk_cache)
		return -ENOMEM;

	hdd_fill_pmksa_info(adapter, pmk_cache, pmksa, false);

	/*
	 * Add to the PMKSA Cache in CSR
	 * PMKSA cache will be having following
	 * 1. pmkid id
	 * 2. pmk
	 * 3. bssid or cache identifier
	 */
	result = wlan_hdd_set_pmksa_cache(adapter, pmk_cache);

	qdf_mtrace(QDF_MODULE_ID_HDD, QDF_MODULE_ID_HDD,
		   TRACE_CODE_HDD_CFG80211_SET_PMKSA,
		   adapter->vdev_id, result);

	if (QDF_IS_STATUS_SUCCESS(result))
		sme_set_del_pmkid_cache(hdd_ctx->psoc, adapter->vdev_id,
					pmk_cache, true);

	qdf_mem_zero(pmk_cache, sizeof(*pmk_cache));

	qdf_mem_free(pmk_cache);
	hdd_exit();

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
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(dev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_set_pmksa(wiphy, dev, pmksa);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
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
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	int status = 0;
	struct wlan_crypto_pmksa *pmk_cache;

	hdd_enter();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_vdev_id(adapter->vdev_id))
		return -EINVAL;

	if (!pmksa) {
		hdd_err("pmksa is NULL");
		return -EINVAL;
	}

	if (!wlan_hdd_is_pmksa_valid(pmksa))
		return -EINVAL;

	status = wlan_hdd_validate_context(hdd_ctx);

	if (0 != status)
		return status;

	pmk_cache = qdf_mem_malloc(sizeof(*pmk_cache));
	if (!pmk_cache)
		return -ENOMEM;

	qdf_mtrace(QDF_MODULE_ID_HDD, QDF_MODULE_ID_HDD,
		   TRACE_CODE_HDD_CFG80211_DEL_PMKSA,
		   adapter->vdev_id, 0);

	hdd_fill_pmksa_info(adapter, pmk_cache, pmksa, true);

	qdf_status = wlan_hdd_del_pmksa_cache(adapter, pmk_cache);
	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		if (!pmksa->bssid)
			hdd_err("Failed to delete PMKSA for null bssid");
		else
			hdd_err("Failed to delete PMKSA for " QDF_MAC_ADDR_FMT,
				QDF_MAC_ADDR_REF(pmksa->bssid));
		status = -EINVAL;
	} else {
		/* clear single_pmk_info information */
		sme_clear_sae_single_pmk_info(hdd_ctx->psoc, adapter->vdev_id,
					      pmk_cache);

		/* Send the delete pmkid command to firmware */
		sme_set_del_pmkid_cache(hdd_ctx->psoc, adapter->vdev_id,
					pmk_cache, false);
	}

	qdf_mem_zero(pmk_cache, sizeof(*pmk_cache));
	qdf_mem_free(pmk_cache);

	hdd_exit();

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
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(dev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_del_pmksa(wiphy, dev, pmksa);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;

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
	int errno;
	QDF_STATUS status;

	hdd_enter();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_vdev_id(adapter->vdev_id))
		return -EINVAL;

	hdd_debug("Flushing PMKSA");

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	errno  = wlan_hdd_validate_context(hdd_ctx);
	if (errno)
		return errno;

	status = wlan_hdd_flush_pmksa_cache(adapter);
	if (status == QDF_STATUS_E_NOSUPPORT) {
		errno = -EOPNOTSUPP;
	} else if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("Cannot flush PMKIDCache");
		errno = -EINVAL;
	}

	sme_set_del_pmkid_cache(hdd_ctx->psoc, adapter->vdev_id, NULL, false);
	hdd_exit();
	return errno;
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
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(dev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_flush_pmksa(wiphy, dev);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
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
	int status;

	hdd_enter();

	status = wlan_hdd_validate_context(hdd_ctx);
	if (status)
		return status;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_vdev_id(adapter->vdev_id))
		return -EINVAL;

	qdf_mtrace(QDF_MODULE_ID_HDD, QDF_MODULE_ID_HDD,
		   TRACE_CODE_HDD_CFG80211_UPDATE_FT_IES, adapter->vdev_id, 0);

	/* Added for debug on reception of Re-assoc Req. */
	if (!hdd_cm_is_vdev_associated(adapter)) {
		hdd_err("Called with Ie of length = %zu when not associated",
		       ftie->ie_len);
		hdd_err("Should be Re-assoc Req IEs");
	}
	hdd_debug("called with Ie of length = %zu", ftie->ie_len);

	ucfg_cm_set_ft_ies(hdd_ctx->pdev, adapter->vdev_id,
			   (const u8 *)ftie->ie, ftie->ie_len);
	hdd_exit();
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
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(dev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_update_ft_ies(wiphy, dev, ftie);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}
#endif

#if defined(CFG80211_EXTERNAL_DH_UPDATE_SUPPORT) || \
(LINUX_VERSION_CODE > KERNEL_VERSION(5, 2, 0))
/**
 * __wlan_hdd_cfg80211_update_owe_info() - update OWE info
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @owe_info: Pointer to OWE info
 *
 * Return: 0 for success, non-zero for failure
 */
static int
__wlan_hdd_cfg80211_update_owe_info(struct wiphy *wiphy,
				    struct net_device *dev,
				    struct cfg80211_update_owe_info *owe_info)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	QDF_STATUS status;
	int errno;

	hdd_enter_dev(dev);

	errno = wlan_hdd_validate_context(hdd_ctx);
	if (errno)
		return errno;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_vdev_id(adapter->vdev_id))
		return -EINVAL;

	hdd_debug("owe_status %d", owe_info->status);

	status = wlansap_update_owe_info(WLAN_HDD_GET_SAP_CTX_PTR(adapter),
					 owe_info->peer, owe_info->ie,
					 owe_info->ie_len, owe_info->status);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("Failed to update OWE info");
		errno = qdf_status_to_os_return(status);
	}

	hdd_exit();
	return errno;
}

/**
 * wlan_hdd_cfg80211_update_owe_info() - update OWE info
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @owe_info: Pointer to OWE info
 *
 * Return: 0 for success, non-zero for failure
 */
static int
wlan_hdd_cfg80211_update_owe_info(struct wiphy *wiphy,
				  struct net_device *net_dev,
				  struct cfg80211_update_owe_info *owe_info)
{
	struct osif_vdev_sync *vdev_sync;
	int errno;

	errno = osif_vdev_sync_op_start(net_dev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_update_owe_info(wiphy, net_dev, owe_info);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}
#endif

void wlan_hdd_cfg80211_update_replay_counter_cb(
		void *cb_ctx, struct pmo_gtk_rsp_params *gtk_rsp_param)

{
	struct hdd_adapter *adapter = (struct hdd_adapter *)cb_ctx;
	uint8_t temp_replay_counter[8];
	int i;
	uint8_t *p;

	hdd_enter();

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

	hdd_debug("gtk_rsp_param bssid "QDF_MAC_ADDR_FMT,
		  QDF_MAC_ADDR_REF(gtk_rsp_param->bssid.bytes));
	/* Update replay counter to NL */
	cfg80211_gtk_rekey_notify(adapter->dev,
					gtk_rsp_param->bssid.bytes,
					temp_replay_counter, GFP_KERNEL);
out:
	hdd_exit();

}

#ifdef WLAN_FEATURE_GTK_OFFLOAD
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
	struct wlan_objmgr_vdev *vdev;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	hdd_enter();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		result = -EINVAL;
		goto out;
	}

	if (wlan_hdd_validate_vdev_id(adapter->vdev_id)) {
		result = -EINVAL;
		goto out;
	}

	qdf_mtrace(QDF_MODULE_ID_HDD, QDF_MODULE_ID_HDD,
		   TRACE_CODE_HDD_CFG80211_SET_REKEY_DATA,
		   adapter->vdev_id, adapter->device_mode);

	result = wlan_hdd_validate_context(hdd_ctx);
	if (0 != result)
		goto out;

	gtk_req = qdf_mem_malloc(sizeof(*gtk_req));
	if (!gtk_req) {
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
	if (data->kck) {
		qdf_mem_copy(gtk_req->kck, data->kck, NL80211_KCK_LEN);
		gtk_req->kck_len = NL80211_KCK_LEN;
	}
	gtk_req->is_fils_connection = hdd_is_fils_connection(hdd_ctx, adapter);
	vdev = hdd_objmgr_get_vdev_by_user(adapter, WLAN_OSIF_POWER_ID);
	if (!vdev) {
		result = -EINVAL;
		goto out;
	}
	status = ucfg_pmo_cache_gtk_offload_req(vdev, gtk_req);
	hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_POWER_ID);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("Failed to cache GTK Offload");
		result = qdf_status_to_os_return(status);
	}
out:
	if (gtk_req)
		qdf_mem_free(gtk_req);
	hdd_exit();

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
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(dev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_set_rekey_data(wiphy, dev, data);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}
#endif /* WLAN_FEATURE_GTK_OFFLOAD */

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
	struct sap_config *config;
	struct hdd_context *hdd_ctx;
	int status;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	hdd_enter();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (!params) {
		hdd_err("params is Null");
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	status = wlan_hdd_validate_context(hdd_ctx);

	if (0 != status)
		return status;

	hostapd_state = WLAN_HDD_GET_HOSTAP_STATE_PTR(adapter);

	if (!hostapd_state) {
		hdd_err("hostapd_state is Null");
		return -EINVAL;
	}

	hdd_debug("acl policy: %d num acl entries: %d", params->acl_policy,
		params->n_acl_entries);

	qdf_mtrace(QDF_MODULE_ID_HDD, QDF_MODULE_ID_HDD,
		   TRACE_CODE_HDD_CFG80211_SET_MAC_ACL,
		   adapter->vdev_id, adapter->device_mode);

	if (QDF_SAP_MODE == adapter->device_mode) {
		config = &adapter->session.ap.sap_config;

		/* default value */
		config->num_accept_mac = 0;
		config->num_deny_mac = 0;

		/**
		 * access control policy
		 * @NL80211_ACL_POLICY_ACCEPT_UNLESS_LISTED: Deny stations which are
		 *   listed in hostapd.deny file.
		 * @NL80211_ACL_POLICY_DENY_UNLESS_LISTED: Allow stations which are
		 *   listed in hostapd.accept file.
		 */
		if (NL80211_ACL_POLICY_DENY_UNLESS_LISTED == params->acl_policy) {
			config->SapMacaddr_acl = eSAP_DENY_UNLESS_ACCEPTED;
		} else if (NL80211_ACL_POLICY_ACCEPT_UNLESS_LISTED ==
			   params->acl_policy) {
			config->SapMacaddr_acl = eSAP_ACCEPT_UNLESS_DENIED;
		} else {
			hdd_warn("Acl Policy : %d is not supported",
				params->acl_policy);
			return -ENOTSUPP;
		}

		if (eSAP_DENY_UNLESS_ACCEPTED == config->SapMacaddr_acl) {
			config->num_accept_mac = params->n_acl_entries;
			for (i = 0; i < params->n_acl_entries; i++) {
				hdd_debug("** Add ACL MAC entry %i in WhiletList :"
					QDF_MAC_ADDR_FMT, i,
					QDF_MAC_ADDR_REF(
						params->mac_addrs[i].addr));

				qdf_mem_copy(&config->accept_mac[i],
					     params->mac_addrs[i].addr,
					     QDF_MAC_ADDR_SIZE);
			}
		} else if (eSAP_ACCEPT_UNLESS_DENIED == config->SapMacaddr_acl) {
			config->num_deny_mac = params->n_acl_entries;
			for (i = 0; i < params->n_acl_entries; i++) {
				hdd_debug("** Add ACL MAC entry %i in BlackList :"
					QDF_MAC_ADDR_FMT, i,
					QDF_MAC_ADDR_REF(
						params->mac_addrs[i].addr));

				qdf_mem_copy(&config->deny_mac[i],
					     params->mac_addrs[i].addr,
					     QDF_MAC_ADDR_SIZE);
			}
		}
		qdf_status = wlansap_set_mac_acl(
			WLAN_HDD_GET_SAP_CTX_PTR(adapter), config);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			hdd_err("SAP Set Mac Acl fail");
			return -EINVAL;
		}
	} else {
		hdd_debug("Invalid device_mode %s(%d)",
			  qdf_opmode_str(adapter->device_mode),
			  adapter->device_mode);
		return -EINVAL;
	}
	hdd_exit();
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
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(dev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_set_mac_acl(wiphy, dev, params);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
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

	hdd_enter();

	err = wlan_hdd_validate_context(hdd_ctx);
	if (err)
		return err;

	if (hdd_ctx->driver_status == DRIVER_MODULES_CLOSED) {
		hdd_err("Driver Modules are closed");
		return -EINVAL;
	}

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

	qdf_mtrace(QDF_MODULE_ID_HDD, QDF_MODULE_ID_HDD,
		   TRACE_CODE_HDD_CFG80211_TESTMODE,
		   NO_SESSION, nla_get_u32(tb[WLAN_HDD_TM_ATTR_CMD]));

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
		if (buf_len < sizeof(*hb_params_temp)) {
			hdd_err("Invalid buffer length for TM_ATTR_DATA");
			return -EINVAL;
		}

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
		if (!hb_params)
			return -ENOMEM;

		qdf_mem_zero(hb_params, sizeof(*hb_params));
		qdf_mem_copy(hb_params, buf, buf_len);
		status = ucfg_pmo_lphb_config_req(
					hdd_ctx->psoc,
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
		if (QDF_GLOBAL_FTM_MODE != hdd_get_conparam()) {
			hdd_err("FTM Command not allowed in mission mode, mode %d",
				hdd_get_conparam());
			return -EINVAL;
		}

		err = wlan_cfg80211_ftm_testmode_cmd(hdd_ctx->pdev,
						     data, len);
		break;
	}
#endif
	default:
		hdd_err("command: %d not supported",
			nla_get_u32(tb[WLAN_HDD_TM_ATTR_CMD]));
		return -EOPNOTSUPP;
	}

	hdd_exit();
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
	struct osif_psoc_sync *psoc_sync;
	int errno;

	errno = osif_psoc_sync_op_start(wiphy_dev(wiphy), &psoc_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_testmode(wiphy, data, len);

	osif_psoc_sync_op_stop(psoc_sync);

	return errno;
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
	enum nl80211_channel_type channel_type;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_vdev_id(adapter->vdev_id))
		return -EINVAL;

	if (!(adapter->device_mode == QDF_SAP_MODE ||
	      adapter->device_mode == QDF_P2P_GO_MODE))
		return -EOPNOTSUPP;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	status = wlan_hdd_validate_context(hdd_ctx);
	if (status)
		return status;

	if (chandef->width < NL80211_CHAN_WIDTH_80)
		channel_type = cfg80211_get_chandef_type(chandef);
	else
		channel_type = NL80211_CHAN_HT40PLUS;
	hdd_debug("Channel width changed to %d ", channel_type);

	/* Change SAP ht2040 mode */
	status = hdd_set_sap_ht2040_mode(adapter, channel_type);
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
 * @link_id: Link id for which channel width has to be applied
 * @chandef: Pointer to channel definition parameter
 *
 * Return: 0 for success, non-zero for failure
 */
#ifdef CFG80211_SINGLE_NETDEV_MULTI_LINK_SUPPORT
static int
wlan_hdd_cfg80211_set_ap_channel_width(struct wiphy *wiphy,
				       struct net_device *dev,
				       unsigned int link_id,
				       struct cfg80211_chan_def *chandef)
#else
static int
wlan_hdd_cfg80211_set_ap_channel_width(struct wiphy *wiphy,
				       struct net_device *dev,
				       struct cfg80211_chan_def *chandef)
#endif
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(dev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_set_ap_channel_width(wiphy, dev, chandef);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
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
	int ret;
	enum phy_ch_width ch_width;
	bool status;

	if (wlan_hdd_validate_vdev_id(adapter->vdev_id))
		return -EINVAL;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);

	if (0 != ret)
		return ret;

	if ((QDF_P2P_GO_MODE != adapter->device_mode) &&
		(QDF_SAP_MODE != adapter->device_mode))
		return -ENOTSUPP;

	status = policy_mgr_is_sap_allowed_on_dfs_freq(
					hdd_ctx->pdev,
					adapter->vdev_id,
					csa_params->chandef.chan->center_freq);
	if (!status)
		return -EINVAL;

	wlan_hdd_set_sap_csa_reason(hdd_ctx->psoc, adapter->vdev_id,
				    CSA_REASON_USER_INITIATED);

	ch_width = hdd_map_nl_chan_width(csa_params->chandef.width);
	hdd_debug("Freq %d width %d ch_width %d",
		  csa_params->chandef.chan->center_freq,
		  csa_params->chandef.width, ch_width);

	ret =
	    hdd_softap_set_channel_change(dev,
					  csa_params->chandef.chan->center_freq,
					  ch_width, false);
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
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(dev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_channel_switch(wiphy, dev, csa_params);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}
#endif

int wlan_hdd_change_hw_mode_for_given_chnl(struct hdd_adapter *adapter,
					   uint32_t chan_freq,
					   enum policy_mgr_conn_update_reason reason)
{
	QDF_STATUS status;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	hdd_enter();

	status = policy_mgr_reset_connection_update(hdd_ctx->psoc);
	if (!QDF_IS_STATUS_SUCCESS(status))
		hdd_err("clearing event failed");

	status = policy_mgr_current_connections_update(
			hdd_ctx->psoc, adapter->vdev_id,
			chan_freq, reason, POLICY_MGR_DEF_REQ_ID);
	switch (status) {
	case QDF_STATUS_E_FAILURE:
		/*
		 * QDF_STATUS_E_FAILURE indicates that some error has occurred
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
						hdd_ctx->psoc);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			hdd_err("ERROR: qdf wait for event failed!!");
			return -EINVAL;
		}
		if (QDF_MONITOR_MODE == adapter->device_mode)
			hdd_info("Monitor mode:channel freq:%d (SMM->DBS)", chan_freq);
		break;

	default:
		/*
		 * QDF_STATUS_E_NOSUPPORT indicates that no HW mode change is
		 * required, so caller can proceed further.
		 */
		break;

	}
	hdd_exit();

	return 0;
}

#ifdef FEATURE_MONITOR_MODE_SUPPORT
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
	mac_handle_t mac_handle;
	struct qdf_mac_addr bssid;
	struct csr_roam_profile roam_profile;
	struct ch_params ch_params = {0};
	int ret;
	enum channel_state chan_freq_state;
	uint8_t max_fw_bw;
	enum phy_ch_width ch_width;
	qdf_freq_t sec_ch_2g_freq = 0;

	hdd_enter();

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	mac_handle = hdd_ctx->mac_handle;

	adapter = hdd_get_adapter(hdd_ctx, QDF_MONITOR_MODE);
	if (!adapter)
		return -EIO;

	hdd_debug("%s: set monitor mode freq %d",
		  adapter->dev->name, chandef->chan->center_freq);

	/* Verify channel state before accepting this request */
	chan_freq_state =
		wlan_reg_get_channel_state_for_freq(hdd_ctx->pdev,
						    chandef->chan->center_freq);
	if (chan_freq_state == CHANNEL_STATE_DISABLE ||
	    chan_freq_state == CHANNEL_STATE_INVALID) {
		hdd_err("Invalid chan freq received for monitor mode aborting");
		return -EINVAL;
	}

	/* Verify the BW before accepting this request */
	ch_width = hdd_map_nl_chan_width(chandef->width);

	if (ch_width > CH_WIDTH_10MHZ ||
	   (!cds_is_sub_20_mhz_enabled() && ch_width > CH_WIDTH_160MHZ)) {
		hdd_err("invalid BW received %d", ch_width);
		return -EINVAL;
	}

	max_fw_bw = sme_get_vht_ch_width();

	if ((ch_width == CH_WIDTH_160MHZ &&
	    max_fw_bw <= WNI_CFG_VHT_CHANNEL_WIDTH_80MHZ) ||
	    (ch_width == CH_WIDTH_80P80MHZ &&
	    max_fw_bw <= WNI_CFG_VHT_CHANNEL_WIDTH_160MHZ)) {
		hdd_err("FW does not support this BW %d max BW supported %d",
			ch_width, max_fw_bw);
		return -EINVAL;
	}
	sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	ch_info = &sta_ctx->ch_info;
	roam_profile.ChannelInfo.freq_list = &ch_info->freq;
	roam_profile.ChannelInfo.numOfChannels = 1;
	roam_profile.phyMode = ch_info->phy_mode;
	roam_profile.ch_params.ch_width = ch_width;
	if (WLAN_REG_IS_24GHZ_CH_FREQ(chandef->chan->center_freq) &&
	    chandef->width == NL80211_CHAN_WIDTH_40 &&
	    chandef->center_freq1) {
		if (chandef->center_freq1 > chandef->chan->center_freq)
			sec_ch_2g_freq = chandef->chan->center_freq + 20;
		else if (chandef->center_freq1 < chandef->chan->center_freq)
			sec_ch_2g_freq = chandef->chan->center_freq - 20;
	}
	hdd_debug("set mon ch:width=%d, freq %d sec_ch_2g_freq=%d",
		  chandef->width, chandef->chan->center_freq, sec_ch_2g_freq);
	hdd_select_cbmode(adapter, chandef->chan->center_freq, sec_ch_2g_freq,
			  &roam_profile.ch_params);
	qdf_mem_copy(bssid.bytes, adapter->mac_addr.bytes,
		     QDF_MAC_ADDR_SIZE);

	ch_params.ch_width = ch_width;
	wlan_reg_set_channel_params_for_freq(hdd_ctx->pdev,
					     chandef->chan->center_freq,
					     sec_ch_2g_freq, &ch_params);
	if (wlan_hdd_change_hw_mode_for_given_chnl(adapter,
						   chandef->chan->center_freq,
						   POLICY_MGR_UPDATE_REASON_SET_OPER_CHAN)) {
		hdd_err("Failed to change hw mode");
		return -EINVAL;
	}

	if (adapter->monitor_mode_vdev_up_in_progress) {
		hdd_err_rl("monitor mode vdev up in progress");
		return -EBUSY;
	}

	status = qdf_event_reset(&adapter->qdf_monitor_mode_vdev_up_event);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err_rl("failed to reinit monitor mode vdev up event");
		return qdf_status_to_os_return(status);
	}
	adapter->monitor_mode_vdev_up_in_progress = true;

	status = sme_roam_channel_change_req(mac_handle, bssid,
					     adapter->vdev_id,
					     &roam_profile.ch_params,
					     &roam_profile);
	if (status) {
		hdd_err_rl("Failed to set sme_RoamChannel for monitor mode status: %d",
			   status);
		adapter->monitor_mode_vdev_up_in_progress = false;
		ret = qdf_status_to_os_return(status);
		return ret;
	}

	/* block on a completion variable until vdev up success*/
	status = qdf_wait_for_event_completion(
				       &adapter->qdf_monitor_mode_vdev_up_event,
					WLAN_MONITOR_MODE_VDEV_UP_EVT);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err_rl("monitor vdev up event time out vdev id: %d",
			  adapter->vdev_id);
		if (adapter->qdf_monitor_mode_vdev_up_event.force_set)
			/*
			 * SSR/PDR has caused shutdown, which has
			 * forcefully set the event.
			 */
			hdd_err_rl("monitor mode vdev up event forcefully set");
		else if (status == QDF_STATUS_E_TIMEOUT)
			hdd_err_rl("monitor mode vdev up timed out");
		else
			hdd_err_rl("Failed monitor mode vdev up(status-%d)",
				  status);

		adapter->monitor_mode_vdev_up_in_progress = false;
		return qdf_status_to_os_return(status);
	}

	hdd_exit();

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
	struct osif_psoc_sync *psoc_sync;
	int errno;

	errno = osif_psoc_sync_op_start(wiphy_dev(wiphy), &psoc_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_set_mon_ch(wiphy, chandef);

	osif_psoc_sync_op_stop(psoc_sync);

	return errno;
}
#endif

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

#ifndef UPDATE_ASSOC_IE
#define UPDATE_ASSOC_IE BIT(0)
#endif

#ifndef UPDATE_FILS_ERP_INFO
#define UPDATE_FILS_ERP_INFO BIT(1)
#endif

#ifndef UPDATE_FILS_AUTH_TYPE
#define UPDATE_FILS_AUTH_TYPE BIT(2)
#endif

#if defined(WLAN_FEATURE_FILS_SK) &&\
	(defined(CFG80211_FILS_SK_OFFLOAD_SUPPORT) ||\
		(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0))) &&\
	(defined(CFG80211_UPDATE_CONNECT_PARAMS) ||\
		(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 10, 0)))
static inline int
hdd_update_connect_params_fils_info(struct hdd_adapter *adapter,
				    struct hdd_context *hdd_ctx,
				    struct cfg80211_connect_params *req,
				    uint32_t changed)
{
	uint8_t *buf;
	QDF_STATUS status;
	enum wlan_fils_auth_type auth_type;
	struct wlan_fils_con_info *fils_info;
	int ret = 0;

	fils_info = qdf_mem_malloc(sizeof(*fils_info));
	if (!fils_info)
		return -EINVAL;

	fils_info->is_fils_connection = true;
	if (changed & UPDATE_FILS_ERP_INFO) {
		fils_info->username_len = req->fils_erp_username_len +
					    sizeof(char) +
					    req->fils_erp_realm_len;
		if (fils_info->username_len >
		    WLAN_CM_FILS_MAX_KEYNAME_NAI_LENGTH) {
			hdd_err("Key NAI Length %d",
				fils_info->username_len);
			ret = -EINVAL;
			goto free_mem;
		}
		if (req->fils_erp_username_len && req->fils_erp_username) {
			buf = fils_info->username;
			qdf_mem_copy(buf, req->fils_erp_username,
				     req->fils_erp_username_len);
			buf += req->fils_erp_username_len;
			*buf++ = '@';
			qdf_mem_copy(buf, req->fils_erp_realm,
				     req->fils_erp_realm_len);
		}

		fils_info->next_seq_num = req->fils_erp_next_seq_num + 1;
		fils_info->rrk_len = req->fils_erp_rrk_len;

		if (fils_info->rrk_len > WLAN_CM_FILS_MAX_RRK_LENGTH) {
			hdd_err("r_rk_length is invalid %d",
				fils_info->rrk_len);
			ret = -EINVAL;
			goto free_mem;
		}

		if (req->fils_erp_rrk_len && req->fils_erp_rrk)
			qdf_mem_copy(fils_info->rrk, req->fils_erp_rrk,
				     fils_info->rrk_len);

		fils_info->realm_len = req->fils_erp_realm_len;
		if (fils_info->realm_len > WLAN_CM_FILS_MAX_REALM_LEN) {
			hdd_err("Invalid fils realm len %d",
				fils_info->realm_len);
			ret = -EINVAL;
			goto free_mem;
		}
		if (req->fils_erp_realm_len && req->fils_erp_realm)
			qdf_mem_copy(fils_info->realm, req->fils_erp_realm,
				     fils_info->realm_len);
	}

	if (changed & UPDATE_FILS_AUTH_TYPE) {
		auth_type = osif_cm_get_fils_auth_type(req->auth_type);
		if (auth_type == FILS_PK_MAX) {
			hdd_err("invalid auth type for fils %d",
				req->auth_type);
			ret = -EINVAL;
			goto free_mem;
		}

		fils_info->auth_type = auth_type;
	}

	hdd_debug("fils conn update: changed %x is_fils %d keyname nai len %d",
		  changed, fils_info->is_fils_connection,
		  fils_info->username_len);
	/*
	 * Update the FILS config from adapter->roam_profile to
	 * csr_session
	 */
	status = ucfg_cm_update_fils_config(hdd_ctx->psoc, adapter->vdev_id,
					    fils_info);
	if (QDF_IS_STATUS_ERROR(status))
		hdd_err("Update FILS connect params failed %d", status);
free_mem:
	qdf_mem_free(fils_info);

	return ret;
}
#else
static inline int
hdd_update_connect_params_fils_info(struct hdd_adapter *adapter,
				    struct hdd_context *hdd_ctx,
				    struct cfg80211_connect_params *req,
				    uint32_t changed)
{
	return -EINVAL;
}
#endif

#if defined(CFG80211_UPDATE_CONNECT_PARAMS) ||\
	(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 10, 0))

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
static int
__wlan_hdd_cfg80211_update_connect_params(struct wiphy *wiphy,
					  struct net_device *dev,
					  struct cfg80211_connect_params *req,
					  uint32_t changed)
{
	int ret;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	QDF_STATUS status;
	mac_handle_t mac_handle;
	struct element_info assoc_ie;

	hdd_enter_dev(dev);

	if (wlan_hdd_validate_vdev_id(adapter->vdev_id))
		return -EINVAL;

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return -EINVAL;

	mac_handle = hdd_ctx->mac_handle;

	if (changed & UPDATE_ASSOC_IE) {
		assoc_ie.len = req->ie_len;
		assoc_ie.ptr = (uint8_t *)req->ie;
		/*
		 * Update this assoc IE received from user space to
		 * umac. RSO command will pick up the assoc
		 * IEs to be sent to firmware from the umac.
		 */
		ucfg_cm_update_session_assoc_ie(hdd_ctx->psoc, adapter->vdev_id,
						&assoc_ie);
	}

	if ((changed & UPDATE_FILS_ERP_INFO) ||
	    (changed & UPDATE_FILS_AUTH_TYPE)) {
		ret = hdd_update_connect_params_fils_info(adapter, hdd_ctx,
							  req, changed);
		if (ret)
			return -EINVAL;

		if (!hdd_ctx->is_fils_roaming_supported) {
			hdd_debug("FILS roaming support %d",
				  hdd_ctx->is_fils_roaming_supported);
			return 0;
		}
	}

	if (changed) {
		status = sme_send_rso_connect_params(mac_handle,
						     adapter->vdev_id);
		if (QDF_IS_STATUS_ERROR(status))
			hdd_err("Update connect params to fw failed %d",
				status);
	}

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
static int
wlan_hdd_cfg80211_update_connect_params(struct wiphy *wiphy,
					struct net_device *dev,
					struct cfg80211_connect_params *req,
					uint32_t changed)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(dev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_update_connect_params(wiphy, dev,
							  req, changed);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}
#endif

#if defined(WLAN_FEATURE_SAE) && \
		(defined(CFG80211_EXTERNAL_AUTH_SUPPORT) || \
		LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0))
#if (defined(CFG80211_EXTERNAL_AUTH_AP_SUPPORT) || \
		LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
/**
 * wlan_hdd_extauth_cache_pmkid() - Extract and cache pmkid
 * @adapter: hdd vdev/net_device context
 * @mac_handle: Handle to the MAC
 * @params: Pointer to external auth params.
 *
 * Extract the PMKID and BSS from external auth params and add to the
 * PMKSA Cache in CSR.
 */
static void
wlan_hdd_extauth_cache_pmkid(struct hdd_adapter *adapter,
			     mac_handle_t mac_handle,
			     struct cfg80211_external_auth_params *params)
{
	struct wlan_crypto_pmksa *pmk_cache;
	QDF_STATUS result;

	if (params->pmkid) {
		pmk_cache = qdf_mem_malloc(sizeof(*pmk_cache));
		if (!pmk_cache)
			return;

		qdf_mem_copy(pmk_cache->bssid.bytes, params->bssid,
			     QDF_MAC_ADDR_SIZE);
		qdf_mem_copy(pmk_cache->pmkid, params->pmkid,
			     PMKID_LEN);
		result = wlan_hdd_set_pmksa_cache(adapter, pmk_cache);
		if (!QDF_IS_STATUS_SUCCESS(result))
			hdd_debug("external_auth: Failed to cache PMKID");

		qdf_mem_free(pmk_cache);
	}

}

/**
 * wlan_hdd_extauth_copy_pmkid() - Copy the pmkid received from the
 * external authentication command received from the userspace.
 * @params: pointer to auth params
 * @pmkid: Pointer to destination pmkid buffer to be filled
 *
 * The caller should ensure that destination pmkid buffer is not NULL.
 *
 * Return: None
 */
static void
wlan_hdd_extauth_copy_pmkid(struct cfg80211_external_auth_params *params,
			    uint8_t *pmkid)
{
	if (params->pmkid)
		qdf_mem_copy(pmkid, params->pmkid, PMKID_LEN);
}

#else
static void
wlan_hdd_extauth_cache_pmkid(struct hdd_adapter *adapter,
			     mac_handle_t mac_handle,
			     struct cfg80211_external_auth_params *params)
{}

static void
wlan_hdd_extauth_copy_pmkid(struct cfg80211_external_auth_params *params,
			    uint8_t *pmkid)
{}
#endif
/**
 * __wlan_hdd_cfg80211_external_auth() - Handle external auth
 *
 * @wiphy: Pointer to wireless phy
 * @dev: net device
 * @params: Pointer to external auth params.
 * Return: 0 on success, negative errno on failure
 *
 * Userspace sends status of the external authentication(e.g., SAE) with a peer.
 * The message carries BSSID of the peer and auth status (WLAN_STATUS_SUCCESS/
 * WLAN_STATUS_UNSPECIFIED_FAILURE) in params.
 * Userspace may send PMKID in params, which can be used for
 * further connections.
 */
static int
__wlan_hdd_cfg80211_external_auth(struct wiphy *wiphy,
				  struct net_device *dev,
				  struct cfg80211_external_auth_params *params)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	int ret;
	mac_handle_t mac_handle;
	struct qdf_mac_addr peer_mac_addr;
	uint8_t pmkid[PMKID_LEN] = {0};

	if (hdd_get_conparam() == QDF_GLOBAL_FTM_MODE) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	if (wlan_hdd_validate_vdev_id(adapter->vdev_id))
		return -EINVAL;

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	hdd_debug("external_auth status: %d peer mac: " QDF_MAC_ADDR_FMT,
		  params->status, QDF_MAC_ADDR_REF(params->bssid));
	mac_handle = hdd_ctx->mac_handle;
	qdf_mem_copy(peer_mac_addr.bytes, params->bssid, QDF_MAC_ADDR_SIZE);

	wlan_hdd_extauth_cache_pmkid(adapter, mac_handle, params);

	wlan_hdd_extauth_copy_pmkid(params, pmkid);
	sme_handle_sae_msg(mac_handle, adapter->vdev_id, params->status,
			   peer_mac_addr, pmkid);

	return ret;
}

/**
 * wlan_hdd_cfg80211_external_auth() - Handle external auth
 * @wiphy: Pointer to wireless phy
 * @dev: net device
 * @params: Pointer to external auth params
 *
 * Return: 0 on success, negative errno on failure
 */
static int
wlan_hdd_cfg80211_external_auth(struct wiphy *wiphy,
				struct net_device *dev,
				struct cfg80211_external_auth_params *params)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(dev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_external_auth(wiphy, dev, params);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}
#endif

#if defined(WLAN_FEATURE_NAN) && \
	   (KERNEL_VERSION(4, 9, 0) <= LINUX_VERSION_CODE)
static int
wlan_hdd_cfg80211_start_nan(struct wiphy *wiphy, struct wireless_dev *wdev,
			    struct cfg80211_nan_conf *conf)
{
	return -EOPNOTSUPP;
}

static void
wlan_hdd_cfg80211_stop_nan(struct wiphy *wiphy, struct wireless_dev *wdev)
{
}

static int wlan_hdd_cfg80211_add_nan_func(struct wiphy *wiphy,
					  struct wireless_dev *wdev,
					  struct cfg80211_nan_func *nan_func)
{
	return -EOPNOTSUPP;
}

static void wlan_hdd_cfg80211_del_nan_func(struct wiphy *wiphy,
					   struct wireless_dev *wdev,
					   u64 cookie)
{
}

static int wlan_hdd_cfg80211_nan_change_conf(struct wiphy *wiphy,
					     struct wireless_dev *wdev,
					     struct cfg80211_nan_conf *conf,
					     u32 changes)
{
	return -EOPNOTSUPP;
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

	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	if (wlan_hdd_validate_context(hdd_ctx) != 0)
		return;

	if (!hdd_ctx->chan_info) {
		hdd_err("chan_info is NULL");
		return;
	}

	chan = hdd_ctx->chan_info;
	for (idx = 0; idx < SIR_MAX_NUM_CHANNELS; idx++) {
		if (chan[idx].freq == info->freq) {
			hdd_update_chan_info(hdd_ctx, &chan[idx], info,
				info->cmd_flag);
			hdd_debug("cmd:%d freq:%u nf:%d cc:%u rcc:%u clk:%u cmd:%d tfc:%d index:%d",
				  chan[idx].cmd_flag, chan[idx].freq,
				  chan[idx].noise_floor,
				  chan[idx].cycle_count,
				  chan[idx].rx_clear_count,
				  chan[idx].clock_freq, chan[idx].cmd_flag,
				  chan[idx].tx_frame_count, idx);
			if (chan[idx].freq == 0)
				break;

		}
	}
}

/**
 * wlan_hdd_init_chan_info() - init chan info in hdd context
 * @hdd_ctx: HDD context pointer
 *
 * Return: none
 */
void wlan_hdd_init_chan_info(struct hdd_context *hdd_ctx)
{
	uint32_t num_2g, num_5g, index = 0;
	mac_handle_t mac_handle;

	hdd_ctx->chan_info = NULL;
	if (!ucfg_scan_is_snr_monitor_enabled(hdd_ctx->psoc)) {
		hdd_debug("SNR monitoring is disabled");
		return;
	}

	hdd_ctx->chan_info =
		qdf_mem_malloc(sizeof(struct scan_chan_info)
					* NUM_CHANNELS);
	if (!hdd_ctx->chan_info)
		return;
	mutex_init(&hdd_ctx->chan_info_lock);

	num_2g = QDF_ARRAY_SIZE(hdd_channels_2_4_ghz);
	for (; index < num_2g; index++) {
		hdd_ctx->chan_info[index].freq =
			hdd_channels_2_4_ghz[index].center_freq;
	}

	num_5g = QDF_ARRAY_SIZE(hdd_channels_5_ghz);
	for (; (index - num_2g) < num_5g; index++) {
		if (wlan_reg_is_dsrc_freq(
		    hdd_channels_5_ghz[index - num_2g].center_freq))
			continue;
		hdd_ctx->chan_info[index].freq =
			hdd_channels_5_ghz[index - num_2g].center_freq;
	}

	index = num_2g + num_5g;
	index = wlan_hdd_populate_5dot9_chan_info(hdd_ctx, index);

	mac_handle = hdd_ctx->mac_handle;
	sme_set_chan_info_callback(mac_handle,
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

#if defined(CFG80211_EXTERNAL_DH_UPDATE_SUPPORT) || \
(LINUX_VERSION_CODE > KERNEL_VERSION(5, 2, 0))

#ifdef WLAN_MLD_AP_OWE_INFO_SUPPORT
static void
hdd_ml_sap_owe_fill_ml_info(struct hdd_adapter *adapter,
			    struct cfg80211_update_owe_info *owe_info,
			    uint8_t *peer_mac)
{
	bool is_mlo_vdev;
	struct wlan_objmgr_peer *peer;
	struct wlan_objmgr_vdev *vdev;
	uint8_t *peer_mld_addr;

	vdev = hdd_objmgr_get_vdev_by_user(adapter, WLAN_OSIF_ID);
	if (!vdev)
		return;

	is_mlo_vdev = wlan_vdev_mlme_is_mlo_vdev(vdev);
	if (is_mlo_vdev)
		owe_info->link_id = wlan_vdev_get_link_id(vdev);
	else
		owe_info->link_id = -1;
	hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_ID);

	if (!is_mlo_vdev)
		return;

	peer = wlan_objmgr_get_peer_by_mac(adapter->hdd_ctx->psoc,
					   peer_mac, WLAN_OSIF_ID);
	if (!peer) {
		hdd_err("Peer not found with MAC " QDF_MAC_ADDR_FMT,
			QDF_MAC_ADDR_REF(peer_mac));
		return;
	}

	peer_mld_addr = wlan_peer_mlme_get_mldaddr(peer);
	qdf_mem_copy(&owe_info->peer_mld_addr[0], peer_mld_addr, ETH_ALEN);
	wlan_objmgr_peer_release_ref(peer, WLAN_OSIF_ID);
}
#elif defined(CFG80211_MLD_AP_STA_CONNECT_UPSTREAM_SUPPORT)
static void
hdd_ml_sap_owe_fill_ml_info(struct hdd_adapter *adapter,
			    struct cfg80211_update_owe_info *owe_info,
			    uint8_t *peer_mac)
{
	bool is_mlo_vdev;
	struct wlan_objmgr_peer *peer;
	struct wlan_objmgr_vdev *vdev;
	uint8_t *peer_mld_addr;

	vdev = hdd_objmgr_get_vdev_by_user(adapter,
					   WLAN_HDD_ID_OBJ_MGR);
	if (!vdev)
		return;

	is_mlo_vdev = wlan_vdev_mlme_is_mlo_vdev(vdev);
	if (!is_mlo_vdev) {
		owe_info->assoc_link_id = -1;
		hdd_objmgr_put_vdev_by_user(vdev, WLAN_HDD_ID_OBJ_MGR);
		return;
	}

	owe_info->assoc_link_id = wlan_vdev_get_link_id(vdev);
	hdd_objmgr_put_vdev_by_user(vdev, WLAN_HDD_ID_OBJ_MGR);

	peer = wlan_objmgr_get_peer_by_mac(adapter->hdd_ctx->psoc,
					   peer_mac, WLAN_HDD_ID_OBJ_MGR);
	if (!peer) {
		hdd_err("Peer not found with MAC " QDF_MAC_ADDR_FMT,
			QDF_MAC_ADDR_REF(peer_mac));
		return;
	}

	peer_mld_addr = wlan_peer_mlme_get_mldaddr(peer);
	qdf_mem_copy(&owe_info->peer_mld_addr[0], peer_mld_addr, ETH_ALEN);
	wlan_objmgr_peer_release_ref(peer, WLAN_HDD_ID_OBJ_MGR);
}
#else
static void
hdd_ml_sap_owe_fill_ml_info(struct hdd_adapter *adapter,
			    struct cfg80211_update_owe_info *owe_info,
			    uint8_t *peer_mac)
{
}
#endif

void hdd_send_update_owe_info_event(struct hdd_adapter *adapter,
				    uint8_t sta_addr[],
				    uint8_t *owe_ie,
				    uint32_t owe_ie_len)
{
	struct cfg80211_update_owe_info owe_info;
	struct net_device *dev = adapter->dev;

	hdd_enter_dev(dev);

	qdf_mem_zero(&owe_info, sizeof(owe_info));
	qdf_mem_copy(owe_info.peer, sta_addr, ETH_ALEN);
	hdd_ml_sap_owe_fill_ml_info(adapter, &owe_info, sta_addr);
	owe_info.ie = owe_ie;
	owe_info.ie_len = owe_ie_len;

	cfg80211_update_owe_info_event(dev, &owe_info, GFP_KERNEL);

	hdd_exit();
}
#endif

static int __wlan_hdd_cfg80211_set_chainmask(struct wiphy *wiphy,
					     uint32_t tx_mask,
					     uint32_t rx_mask)
{
	int ret;
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	enum hdd_chain_mode chains;

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return -EINVAL;

	if (hdd_ctx->num_rf_chains != HDD_ANTENNA_MODE_2X2 ||
	    !ucfg_mlme_is_chain_mask_supported(hdd_ctx->psoc)) {
		hdd_info_rl("Chainmask can't be configured, num of rf chain %d",
			    hdd_ctx->num_rf_chains);
		return -ENOTSUPP;
	}
	chains = HDD_CHAIN_MODE_2X2;
	if (!tx_mask || tx_mask > chains || !rx_mask || rx_mask > chains) {
		hdd_err_rl("Invalid masks. txMask: %d rxMask: %d num_rf_chains: %d",
			   tx_mask, rx_mask, hdd_ctx->num_rf_chains);

		return -EINVAL;
	}

	ret = wma_cli_set_command(0, WMI_PDEV_PARAM_TX_CHAIN_MASK,
				  tx_mask, PDEV_CMD);
	if (ret)
		hdd_err_rl("Failed to set TX the mask");

	ret = wma_cli_set_command(0, WMI_PDEV_PARAM_RX_CHAIN_MASK,
				  rx_mask, PDEV_CMD);
	if (ret)
		hdd_err_rl("Failed to set RX the mask");

	return ret;
}

static int wlan_hdd_cfg80211_set_chainmask(struct wiphy *wiphy,
					   uint32_t tx_mask,
					   uint32_t rx_mask)
{
	struct osif_psoc_sync *psoc_sync;
	int errno;

	errno = osif_psoc_sync_op_start(wiphy_dev(wiphy), &psoc_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_set_chainmask(wiphy, tx_mask, rx_mask);
	osif_psoc_sync_op_stop(psoc_sync);

	return errno;
}

static int __wlan_hdd_cfg80211_get_chainmask(struct wiphy *wiphy,
					     uint32_t *tx_mask,
					     uint32_t *rx_mask)

{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	int ret;

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return -EINVAL;

	*tx_mask = wma_cli_get_command(0, WMI_PDEV_PARAM_TX_CHAIN_MASK,
				       PDEV_CMD);
	*rx_mask = wma_cli_get_command(0, WMI_PDEV_PARAM_RX_CHAIN_MASK,
				       PDEV_CMD);

	/* if 0 return max value as 0 mean no set cmnd received yet */
	if (!*tx_mask)
		*tx_mask = hdd_ctx->num_rf_chains == HDD_ANTENNA_MODE_2X2 ?
				HDD_CHAIN_MODE_2X2 : HDD_CHAIN_MODE_1X1;
	if (!*rx_mask)
		*rx_mask = hdd_ctx->num_rf_chains == HDD_ANTENNA_MODE_2X2 ?
				HDD_CHAIN_MODE_2X2 : HDD_CHAIN_MODE_1X1;
	hdd_debug("tx_mask: %d rx_mask: %d", *tx_mask, *rx_mask);

	return 0;
}

static int wlan_hdd_cfg80211_get_chainmask(struct wiphy *wiphy,
					   uint32_t *tx_mask,
					   uint32_t *rx_mask)
{
	struct osif_psoc_sync *psoc_sync;
	int errno;

	errno = osif_psoc_sync_op_start(wiphy_dev(wiphy), &psoc_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_get_chainmask(wiphy, tx_mask, rx_mask);
	osif_psoc_sync_op_stop(psoc_sync);

	return errno;
}

enum qca_wlan_802_11_mode
hdd_convert_cfgdot11mode_to_80211mode(enum csr_cfgdot11mode mode)
{
	switch (mode) {
	case eCSR_CFG_DOT11_MODE_11A:
		return QCA_WLAN_802_11_MODE_11A;
	case eCSR_CFG_DOT11_MODE_11B:
		return QCA_WLAN_802_11_MODE_11B;
	case eCSR_CFG_DOT11_MODE_11G:
		return QCA_WLAN_802_11_MODE_11G;
	case eCSR_CFG_DOT11_MODE_11N:
		return QCA_WLAN_802_11_MODE_11N;
	case eCSR_CFG_DOT11_MODE_11AC:
		return QCA_WLAN_802_11_MODE_11AC;
	case eCSR_CFG_DOT11_MODE_11G_ONLY:
		return QCA_WLAN_802_11_MODE_11G;
	case eCSR_CFG_DOT11_MODE_11N_ONLY:
		return QCA_WLAN_802_11_MODE_11N;
	case eCSR_CFG_DOT11_MODE_11AC_ONLY:
		return QCA_WLAN_802_11_MODE_11AC;
	case eCSR_CFG_DOT11_MODE_11AX:
		return QCA_WLAN_802_11_MODE_11AX;
	case eCSR_CFG_DOT11_MODE_11AX_ONLY:
		return QCA_WLAN_802_11_MODE_11AX;
	case eCSR_CFG_DOT11_MODE_11BE:
	case eCSR_CFG_DOT11_MODE_11BE_ONLY:
		return QCA_WLAN_802_11_MODE_11BE;
	case eCSR_CFG_DOT11_MODE_ABG:
	case eCSR_CFG_DOT11_MODE_AUTO:
	default:
		return QCA_WLAN_802_11_MODE_INVALID;
	}
}

bool hdd_is_legacy_connection(struct hdd_adapter *adapter)
{
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	int connection_mode;

	connection_mode = hdd_convert_cfgdot11mode_to_80211mode(
						sta_ctx->conn_info.dot11mode);
	if (connection_mode == QCA_WLAN_802_11_MODE_11A ||
	    connection_mode == QCA_WLAN_802_11_MODE_11B ||
	    connection_mode == QCA_WLAN_802_11_MODE_11G)
		return true;
	else
		return false;
}

static int __wlan_hdd_cfg80211_get_channel(struct wiphy *wiphy,
					   struct wireless_dev *wdev,
					   struct cfg80211_chan_def *chandef)
{
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx;
	bool is_legacy_phymode = false;
	struct wlan_objmgr_vdev *vdev;
	uint32_t chan_freq;

	hdd_enter_dev(wdev->netdev);

	if (hdd_validate_adapter(adapter))
		return -EINVAL;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (wlan_hdd_validate_context(hdd_ctx))
		return -EINVAL;

	if ((adapter->device_mode == QDF_STA_MODE) ||
	    (adapter->device_mode == QDF_P2P_CLIENT_MODE)) {
		struct hdd_station_ctx *sta_ctx;

		if (!hdd_cm_is_vdev_associated(adapter))
			return -EINVAL;

		sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
		if (sta_ctx->conn_info.dot11mode < eCSR_CFG_DOT11_MODE_11N)
			is_legacy_phymode = true;

	} else if ((adapter->device_mode == QDF_SAP_MODE) ||
			(adapter->device_mode == QDF_P2P_GO_MODE)) {
		struct hdd_ap_ctx *ap_ctx;

		ap_ctx = WLAN_HDD_GET_AP_CTX_PTR(adapter);

		if (!test_bit(SOFTAP_BSS_STARTED, &adapter->event_flags))
			return -EINVAL;

		switch (ap_ctx->sap_config.SapHw_mode) {
		case eCSR_DOT11_MODE_11n:
		case eCSR_DOT11_MODE_11n_ONLY:
		case eCSR_DOT11_MODE_11ac:
		case eCSR_DOT11_MODE_11ac_ONLY:
		case eCSR_DOT11_MODE_11ax:
		case eCSR_DOT11_MODE_11ax_ONLY:
			is_legacy_phymode = false;
			break;
		default:
			is_legacy_phymode = true;
			break;
		}
	} else {
		return -EINVAL;
	}

	vdev = hdd_objmgr_get_vdev_by_user(adapter, WLAN_OSIF_ID);
	if (!vdev)
		return -EINVAL;

	chan_freq = vdev->vdev_mlme.des_chan->ch_freq;
	chandef->center_freq1 = vdev->vdev_mlme.des_chan->ch_cfreq1;
	chandef->center_freq2 = 0;
	chandef->chan = ieee80211_get_channel(wiphy, chan_freq);

	switch (vdev->vdev_mlme.des_chan->ch_width) {
	case CH_WIDTH_20MHZ:
		if (is_legacy_phymode)
			chandef->width = NL80211_CHAN_WIDTH_20_NOHT;
		else
			chandef->width = NL80211_CHAN_WIDTH_20;
		break;
	case CH_WIDTH_40MHZ:
		chandef->width = NL80211_CHAN_WIDTH_40;
		break;
	case CH_WIDTH_80MHZ:
		chandef->width = NL80211_CHAN_WIDTH_80;
		break;
	case CH_WIDTH_160MHZ:
		chandef->width = NL80211_CHAN_WIDTH_160;
		/* Set center_freq1 to center frequency of complete 160MHz */
		chandef->center_freq1 = vdev->vdev_mlme.des_chan->ch_cfreq2;
		break;
	case CH_WIDTH_80P80MHZ:
		chandef->width = NL80211_CHAN_WIDTH_80P80;
		chandef->center_freq2 = vdev->vdev_mlme.des_chan->ch_cfreq2;
		break;
	case CH_WIDTH_5MHZ:
		chandef->width = NL80211_CHAN_WIDTH_5;
		break;
	case CH_WIDTH_10MHZ:
		chandef->width = NL80211_CHAN_WIDTH_10;
		break;
	default:
		chandef->width = NL80211_CHAN_WIDTH_20;
		break;
	}

	wlan_hdd_set_chandef(vdev, chandef);

	hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_ID);
	hdd_debug("primary_freq:%d, ch_width:%d, center_freq1:%d, center_freq2:%d",
		  chan_freq, chandef->width, chandef->center_freq1,
		  chandef->center_freq2);
	return 0;
}

/**
 * wlan_hdd_cfg80211_get_channel() - API to process cfg80211 get_channel request
 * @wiphy: Pointer to wiphy
 * @wdev: Pointer to wireless device
 * @chandef: Pointer to channel definition
 *
 * Return: 0 for success, non zero for failure
 */
#ifdef CFG80211_SINGLE_NETDEV_MULTI_LINK_SUPPORT
static int wlan_hdd_cfg80211_get_channel(struct wiphy *wiphy,
					 struct wireless_dev *wdev,
					 unsigned int link_id,
					 struct cfg80211_chan_def *chandef)
#else
static int wlan_hdd_cfg80211_get_channel(struct wiphy *wiphy,
					 struct wireless_dev *wdev,
					 struct cfg80211_chan_def *chandef)
#endif
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_get_channel(wiphy, wdev, chandef);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}

static bool hdd_check_bitmask_for_single_rate(enum nl80211_band band,
				const struct cfg80211_bitrate_mask *mask)
{
	int num_rates = 0, i;

	num_rates += qdf_get_hweight32(mask->control[band].legacy);

	for (i = 0; i < QDF_ARRAY_SIZE(mask->control[band].ht_mcs); i++)
		num_rates += qdf_get_hweight8(mask->control[band].ht_mcs[i]);

	for (i = 0; i < QDF_ARRAY_SIZE(mask->control[band].vht_mcs); i++)
		num_rates += qdf_get_hweight16(mask->control[band].vht_mcs[i]);

	return num_rates ? true : false;
}

static int __wlan_hdd_cfg80211_set_bitrate_mask(struct wiphy *wiphy,
						struct net_device *dev,
						const u8 *peer,
				       const struct cfg80211_bitrate_mask *mask)
{
	enum nl80211_band band;
	int errno;
	struct hdd_adapter *adapter = netdev_priv(dev);
	uint8_t nss, i;
	int bit_rate = -1;
	uint8_t rate_index;
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	uint8_t vdev_id;
	uint8_t gi_val;
#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 4, 0))
	uint8_t auto_rate_he_gi = 0;
#endif

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam() ||
	    QDF_GLOBAL_MONITOR_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in mode");
		return -EINVAL;
	}

	errno = hdd_validate_adapter(adapter);
	if (errno)
		return errno;

	errno = wlan_hdd_validate_context(hdd_ctx);
	if (errno)
		return errno;

	vdev_id = adapter->vdev_id;

	for (band = NL80211_BAND_2GHZ; band <= NL80211_BAND_5GHZ; band++) {
		/* Support configuring only one bitrate */
		if (!hdd_check_bitmask_for_single_rate(band, mask)) {
			hdd_err("Multiple bitrate set not supported for band %u",
				band);
			errno = -EINVAL;
			continue;
		}

		if (!hweight32(mask->control[band].legacy)) {
			hdd_err("Legacy bit rate setting not supported for band %u",
				band);
			errno = -EINVAL;
			continue;
		}

		for (i = 0;
			i < QDF_ARRAY_SIZE(mask->control[band].ht_mcs); i++) {
			if (qdf_get_hweight8(mask->control[band].ht_mcs[i])
									== 1) {
				nss = i;
				rate_index =
				      (ffs(mask->control[band].ht_mcs[i]) - 1);
				bit_rate = hdd_assemble_rate_code(
						WMI_RATE_PREAMBLE_HT,
						nss, rate_index);
				goto configure_fw;
			}
		}

		for (i = 0;
			i < QDF_ARRAY_SIZE(mask->control[band].vht_mcs); i++) {
			if (qdf_get_hweight16(mask->control[band].vht_mcs[i])
									== 1) {
				nss = i;
				rate_index =
				     (ffs(mask->control[band].vht_mcs[i]) - 1);
				bit_rate = hdd_assemble_rate_code(
						WMI_RATE_PREAMBLE_VHT,
						nss, rate_index);
				break;
			}
		}

configure_fw:
		if (bit_rate != -1) {
			hdd_debug("WMI_VDEV_PARAM_FIXED_RATE val %d", bit_rate);

			errno = wma_cli_set_command(adapter->vdev_id,
						    WMI_VDEV_PARAM_FIXED_RATE,
						    bit_rate, VDEV_CMD);

			if (errno)
				hdd_err("Failed to set firmware, errno %d",
					errno);
		}


#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 4, 0))
		if (NL80211_RATE_INFO_HE_GI_0_8 == mask->control[band].he_gi) {
			auto_rate_he_gi = AUTO_RATE_GI_800NS;
			gi_val = 1;
		} else if (NL80211_RATE_INFO_HE_GI_1_6 ==
			   mask->control[band].he_gi) {
			auto_rate_he_gi = AUTO_RATE_GI_1600NS;
			gi_val = 2;
		} else if (NL80211_RATE_INFO_HE_GI_3_2 ==
			   mask->control[band].he_gi) {
			auto_rate_he_gi = AUTO_RATE_GI_3200NS;
			gi_val = 3;
		}
		if (auto_rate_he_gi) {
			errno = sme_set_auto_rate_he_sgi(hdd_ctx->mac_handle,
							 adapter->vdev_id,
							 auto_rate_he_gi);
			if (errno)
				hdd_err("auto rate GI %d set fail, status %d",
					auto_rate_he_gi, errno);

			errno = sme_update_ht_config(
					hdd_ctx->mac_handle,
					adapter->vdev_id,
					WNI_CFG_HT_CAP_INFO_SHORT_GI_20MHZ,
					gi_val);

			if (errno) {
				hdd_err("cfg set failed, value %d status %d",
					gi_val, errno);
			}
		} else
#endif
		if (mask->control[band].gi) {
			if (NL80211_TXRATE_FORCE_SGI == mask->control[band].gi)
				gi_val = 0;
			else
				gi_val = 1;

			errno = sme_update_ht_config(
					hdd_ctx->mac_handle,
					adapter->vdev_id,
					WNI_CFG_HT_CAP_INFO_SHORT_GI_20MHZ,
					gi_val);

			if (errno)
				hdd_err("cfg set failed, value %d status %d",
					mask->control[band].gi, errno);
		}
	}

	return errno;
}

#ifdef CFG80211_SINGLE_NETDEV_MULTI_LINK_SUPPORT
static int wlan_hdd_cfg80211_set_bitrate_mask(struct wiphy *wiphy,
					      struct net_device *netdev,
					      unsigned int link_id,
					      const u8 *peer,
				       const struct cfg80211_bitrate_mask *mask)
#else
static int wlan_hdd_cfg80211_set_bitrate_mask(struct wiphy *wiphy,
					      struct net_device *netdev,
					      const u8 *peer,
				       const struct cfg80211_bitrate_mask *mask)
#endif
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(netdev, &vdev_sync);
	if (errno) {
		hdd_err("vdev_sync_op_start failure");
		return errno;
	}

	errno = __wlan_hdd_cfg80211_set_bitrate_mask(wiphy, netdev, peer,
						     mask);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
static void wlan_hdd_select_queue(struct net_device *dev, struct sk_buff *skb)
{
	hdd_select_queue(dev, skb, NULL);
}
#else
static void wlan_hdd_select_queue(struct net_device *dev, struct sk_buff *skb)
{
	hdd_select_queue(dev, skb, NULL, NULL);
}
#endif

static int __wlan_hdd_cfg80211_tx_control_port(struct wiphy *wiphy,
					       struct net_device *dev,
					       const u8 *buf, size_t len,
					       const u8 *src, const u8 *dest,
						__be16 proto, bool unencrypted)
{
	qdf_nbuf_t nbuf;
	struct ethhdr *ehdr;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);

	hdd_enter();

	nbuf = dev_alloc_skb(len + sizeof(struct ethhdr));
	if (!nbuf)
		return -ENOMEM;

	skb_reserve(nbuf, sizeof(struct ethhdr));
	skb_put_data(nbuf, buf, len);
	ehdr = skb_push(nbuf, sizeof(struct ethhdr));
	qdf_mem_copy(ehdr->h_dest, dest, ETH_ALEN);

	if (!src || qdf_is_macaddr_zero((struct qdf_mac_addr *)src))
		qdf_mem_copy(ehdr->h_source, adapter->mac_addr.bytes, ETH_ALEN);
	else
		qdf_mem_copy(ehdr->h_source, src, ETH_ALEN);

	ehdr->h_proto = proto;

	nbuf->dev = dev;
	nbuf->protocol = htons(ETH_P_PAE);
	skb_reset_network_header(nbuf);
	skb_reset_mac_header(nbuf);

	netif_tx_lock(dev);
	wlan_hdd_select_queue(dev, nbuf);
	dev->netdev_ops->ndo_start_xmit(nbuf, dev);
	netif_tx_unlock(dev);

	hdd_exit();
	return 0;
}

static int _wlan_hdd_cfg80211_tx_control_port(struct wiphy *wiphy,
					      struct net_device *dev,
					      const u8 *buf, size_t len,
					      const u8 *src, const u8 *dest,
					      __be16 proto, bool unencrypted)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(dev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_tx_control_port(wiphy, dev, buf, len, src,
						    dest, proto, unencrypted);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0) || \
	defined(CFG80211_TX_CONTROL_PORT_LINK_SUPPORT))
static int wlan_hdd_cfg80211_tx_control_port(struct wiphy *wiphy,
					     struct net_device *dev,
					     const u8 *buf,
					     size_t len,
					     const u8 *dest, const __be16 proto,
					     bool unencrypted, int link_id,
					     u64 *cookie)
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0))
static int wlan_hdd_cfg80211_tx_control_port(struct wiphy *wiphy,
					     struct net_device *dev,
					     const u8 *buf, size_t len,
					     const u8 *dest, __be16 proto,
					     bool unencrypted, u64 *cookie)
#else
static int wlan_hdd_cfg80211_tx_control_port(struct wiphy *wiphy,
					     struct net_device *dev,
					     const u8 *buf, size_t len,
					     const u8 *dest, __be16 proto,
					     bool unencrypted)
#endif
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);

	return _wlan_hdd_cfg80211_tx_control_port(wiphy, dev, buf, len,
						  adapter->mac_addr.bytes,
						  dest, proto, unencrypted);
}

#if defined(CFG80211_CTRL_FRAME_SRC_ADDR_TA_ADDR)
bool wlan_hdd_cfg80211_rx_control_port(struct net_device *dev,
				       u8 *ta_addr,
				       struct sk_buff *skb,
				       bool unencrypted)
{
	return cfg80211_rx_control_port(dev, ta_addr, skb, unencrypted);
}
#else
bool wlan_hdd_cfg80211_rx_control_port(struct net_device *dev,
				       u8 *ta_addr,
				       struct sk_buff *skb,
				       bool unencrypted)
{
	return false;
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
	.set_wiphy_params = wlan_hdd_cfg80211_set_wiphy_params,
	.set_tx_power = wlan_hdd_cfg80211_set_txpower,
	.get_tx_power = wlan_hdd_cfg80211_get_txpower,
	.remain_on_channel = wlan_hdd_cfg80211_remain_on_channel,
	.cancel_remain_on_channel = wlan_hdd_cfg80211_cancel_remain_on_channel,
	.mgmt_tx = wlan_hdd_mgmt_tx,
	.mgmt_tx_cancel_wait = wlan_hdd_cfg80211_mgmt_tx_cancel_wait,
	.set_default_mgmt_key = wlan_hdd_set_default_mgmt_key,
#if defined (CFG80211_BIGTK_CONFIGURATION_SUPPORT) || \
	    (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0))
	.set_default_beacon_key = wlan_hdd_cfg80211_set_default_beacon_key,
#endif
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
#if defined(CFG80211_EXTERNAL_DH_UPDATE_SUPPORT) || \
(LINUX_VERSION_CODE > KERNEL_VERSION(5, 2, 0))
	.update_owe_info = wlan_hdd_cfg80211_update_owe_info,
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
#ifdef FEATURE_MONITOR_MODE_SUPPORT
	.set_monitor_channel = wlan_hdd_cfg80211_set_mon_ch,
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0)) || \
	defined(CFG80211_ABORT_SCAN)
	.abort_scan = wlan_hdd_cfg80211_abort_scan,
#endif
#if defined(CFG80211_UPDATE_CONNECT_PARAMS) || \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 10, 0))
	.update_connect_params = wlan_hdd_cfg80211_update_connect_params,
#endif
#if defined(WLAN_FEATURE_SAE) && \
		(defined(CFG80211_EXTERNAL_AUTH_SUPPORT) || \
		LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0))
	.external_auth = wlan_hdd_cfg80211_external_auth,
#endif
#if defined(WLAN_FEATURE_NAN) && \
	   (KERNEL_VERSION(4, 9, 0) <= LINUX_VERSION_CODE)
	.start_nan = wlan_hdd_cfg80211_start_nan,
	.stop_nan = wlan_hdd_cfg80211_stop_nan,
	.add_nan_func = wlan_hdd_cfg80211_add_nan_func,
	.del_nan_func = wlan_hdd_cfg80211_del_nan_func,
	.nan_change_conf = wlan_hdd_cfg80211_nan_change_conf,
#endif
	.set_antenna = wlan_hdd_cfg80211_set_chainmask,
	.get_antenna = wlan_hdd_cfg80211_get_chainmask,
	.get_channel = wlan_hdd_cfg80211_get_channel,
	.set_bitrate_mask = wlan_hdd_cfg80211_set_bitrate_mask,
	.tx_control_port = wlan_hdd_cfg80211_tx_control_port,
};
