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
 * DOC:  wlan_hdd_cfg.c
 *
 * WLAN Host Device Driver configuration interface implementation
 */

/* Include Files */

#include <linux/firmware.h>
#include <linux/string.h>
#include <wlan_hdd_includes.h>
#include <wlan_hdd_main.h>
#include <wlan_hdd_assoc.h>
#include <wlan_hdd_cfg.h>
#include <linux/string.h>
#include <qdf_types.h>
#include <csr_api.h>
#include <wlan_hdd_misc.h>
#include <wlan_hdd_napi.h>
#include <cds_api.h>
#include "wlan_hdd_he.h"
#include <wlan_policy_mgr_api.h>
#include "wifi_pos_api.h"
#include "wlan_hdd_green_ap.h"
#include "wlan_hdd_green_ap_cfg.h"
#include "wlan_hdd_twt.h"
#include "wlan_mlme_ucfg_api.h"
#include "wlan_mlme_public_struct.h"

static void
cb_notify_set_roam_prefer5_g_hz(struct hdd_context *hdd_ctx,
				unsigned long notify_id)
{
	sme_update_roam_prefer5_g_hz(hdd_ctx->mac_handle,
				     hdd_ctx->config->nRoamPrefer5GHz);
}

static void
cb_notify_set_roam_rssi_diff(struct hdd_context *hdd_ctx,
			     unsigned long notify_id)
{
	sme_update_roam_rssi_diff(hdd_ctx->mac_handle,
				  0, hdd_ctx->config->RoamRssiDiff);
}

static void
cb_notify_set_fast_transition_enabled(struct hdd_context *hdd_ctx,
				      unsigned long notify_id)
{
	bool enabled = hdd_ctx->config->isFastTransitionEnabled;

	sme_update_fast_transition_enabled(hdd_ctx->mac_handle, enabled);
}

static void
cb_notify_set_roam_intra_band(struct hdd_context *hdd_ctx,
			      unsigned long notify_id)
{
	sme_set_roam_intra_band(hdd_ctx->mac_handle,
				hdd_ctx->config->nRoamIntraBand);
}

static void cb_notify_set_wes_mode(struct hdd_context *hdd_ctx,
				   unsigned long notify_id)
{
	sme_update_wes_mode(hdd_ctx->mac_handle,
			    hdd_ctx->config->isWESModeEnabled, 0);
}

static void
cb_notify_set_roam_scan_n_probes(struct hdd_context *hdd_ctx,
				 unsigned long notify_id)
{
	sme_update_roam_scan_n_probes(hdd_ctx->mac_handle, 0,
				      hdd_ctx->config->nProbes);
}

static void
cb_notify_set_roam_scan_home_away_time(struct hdd_context *hdd_ctx,
				       unsigned long notify_id)
{
	uint16_t away_time = hdd_ctx->config->nRoamScanHomeAwayTime;

	sme_update_roam_scan_home_away_time(hdd_ctx->mac_handle, 0,
					    away_time, true);
}

static void
notify_is_fast_roam_ini_feature_enabled(struct hdd_context *hdd_ctx,
					unsigned long notify_id)
{
	bool enabled = hdd_ctx->config->isFastRoamIniFeatureEnabled;

	sme_update_is_fast_roam_ini_feature_enabled(hdd_ctx->mac_handle, 0,
						    enabled);
}

static void
notify_is_mawc_ini_feature_enabled(struct hdd_context *hdd_ctx,
				   unsigned long notify_id)
{
	sme_update_is_mawc_ini_feature_enabled(hdd_ctx->mac_handle,
					       hdd_ctx->config->MAWCEnabled);
}

#ifdef FEATURE_WLAN_ESE
static void
cb_notify_set_ese_feature_enabled(struct hdd_context *hdd_ctx,
				  unsigned long notify_id)
{
	bool enabled = hdd_ctx->config->isEseIniFeatureEnabled;

	sme_update_is_ese_feature_enabled(hdd_ctx->mac_handle, 0, enabled);
}
#endif

static void
cb_notify_set_opportunistic_scan_threshold_diff(struct hdd_context *hdd_ctx,
						unsigned long notify_id)
{
	uint8_t diff = hdd_ctx->config->nOpportunisticThresholdDiff;

	sme_set_roam_opportunistic_scan_threshold_diff(hdd_ctx->mac_handle,
						       0, diff);
}

static void cb_notify_set_roam_rescan_rssi_diff(struct hdd_context *hdd_ctx,
						unsigned long notify_id)
{
	sme_set_roam_rescan_rssi_diff(hdd_ctx->mac_handle,
				      0, hdd_ctx->config->nRoamRescanRssiDiff);
}

static void
cb_notify_set_neighbor_lookup_rssi_threshold(struct hdd_context *hdd_ctx,
					     unsigned long notify_id)
{
	uint8_t threshold = hdd_ctx->config->nNeighborLookupRssiThreshold;

	sme_set_neighbor_lookup_rssi_threshold(hdd_ctx->mac_handle, 0,
					       threshold);
}

static void
cb_notify_set_delay_before_vdev_stop(struct hdd_context *hdd_ctx,
				     unsigned long notify_id)
{
	sme_set_delay_before_vdev_stop(hdd_ctx->mac_handle, 0,
				       hdd_ctx->config->delay_before_vdev_stop);
}

static void
cb_notify_set_neighbor_scan_period(struct hdd_context *hdd_ctx,
				   unsigned long notify_id)
{
	sme_set_neighbor_scan_period(hdd_ctx->mac_handle, 0,
				     hdd_ctx->config->nNeighborScanPeriod);
}

/*
 * cb_notify_set_neighbor_scan_min_period() - configure min rest
 * time during roaming scan
 *
 * @hdd_ctx: HDD context data structure
 * @notify_id: Identifies 1 of the 4 parameters to be modified
 *
 * Picks up the value from hdd configuration and passes it to SME.
 * Return: void
 */
static void
cb_notify_set_neighbor_scan_min_period(struct hdd_context *hdd_ctx,
				       unsigned long notify_id)
{
	uint16_t period = hdd_ctx->config->neighbor_scan_min_period;

	sme_set_neighbor_scan_min_period(hdd_ctx->mac_handle, 0,
					 period);
}

static void
cb_notify_set_neighbor_results_refresh_period(struct hdd_context *hdd_ctx,
					      unsigned long notify_id)
{
	uint16_t period = hdd_ctx->config->nNeighborResultsRefreshPeriod;

	sme_set_neighbor_scan_refresh_period(hdd_ctx->mac_handle, 0,
					     period);
}

static void
cb_notify_set_empty_scan_refresh_period(struct hdd_context *hdd_ctx,
					unsigned long notify_id)
{
	uint16_t period = hdd_ctx->config->nEmptyScanRefreshPeriod;

	sme_update_empty_scan_refresh_period(hdd_ctx->mac_handle, 0,
					     period);
}

static void
cb_notify_set_neighbor_scan_min_chan_time(struct hdd_context *hdd_ctx,
					  unsigned long notify_id)
{
	uint16_t min_chan_time = hdd_ctx->config->nNeighborScanMinChanTime;

	sme_set_neighbor_scan_min_chan_time(hdd_ctx->mac_handle,
					    min_chan_time, 0);
}

static void
cb_notify_set_neighbor_scan_max_chan_time(struct hdd_context *hdd_ctx,
					  unsigned long notify_id)
{
	uint16_t max_chan_time = hdd_ctx->config->nNeighborScanMaxChanTime;

	sme_set_neighbor_scan_max_chan_time(hdd_ctx->mac_handle, 0,
					    max_chan_time);
}

static void cb_notify_set_roam_bmiss_first_bcnt(struct hdd_context *hdd_ctx,
						unsigned long notify_id)
{
	sme_set_roam_bmiss_first_bcnt(hdd_ctx->mac_handle,
				      0, hdd_ctx->config->nRoamBmissFirstBcnt);
}

static void cb_notify_set_roam_bmiss_final_bcnt(struct hdd_context *hdd_ctx,
						unsigned long notify_id)
{
	sme_set_roam_bmiss_final_bcnt(hdd_ctx->mac_handle, 0,
				      hdd_ctx->config->nRoamBmissFinalBcnt);
}

static void cb_notify_set_roam_beacon_rssi_weight(struct hdd_context *hdd_ctx,
						  unsigned long notify_id)
{
	sme_set_roam_beacon_rssi_weight(hdd_ctx->mac_handle, 0,
					hdd_ctx->config->nRoamBeaconRssiWeight);
}

static void
cb_notify_set_dfs_scan_mode(struct hdd_context *hdd_ctx,
			    unsigned long notify_id)
{
	sme_update_dfs_scan_mode(hdd_ctx->mac_handle, 0,
				 hdd_ctx->config->allowDFSChannelRoam);
}

static void cb_notify_set_enable_ssr(struct hdd_context *hdd_ctx,
				     unsigned long notify_id)
{
	sme_update_enable_ssr(hdd_ctx->mac_handle, hdd_ctx->config->enableSSR);
}

static void
cb_notify_set_g_sap_preferred_chan_location(struct hdd_context *hdd_ctx,
					    unsigned long notify_id)
{
	uint8_t location = hdd_ctx->config->gSapPreferredChanLocation;

	wlansap_set_dfs_preferred_channel_location(hdd_ctx->mac_handle,
						   location);
}

static void ch_notify_set_g_disable_dfs_japan_w53(struct hdd_context *hdd_ctx,
						  unsigned long notify_id)
{
	bool disabled = hdd_ctx->config->gDisableDfsJapanW53;

	wlansap_set_dfs_restrict_japan_w53(hdd_ctx->mac_handle, disabled);
}

static void
cb_notify_update_roam_scan_offload_enabled(struct hdd_context *hdd_ctx,
					   unsigned long notify_id)
{
	bool enabled = hdd_ctx->config->isRoamOffloadScanEnabled;

	sme_update_roam_scan_offload_enabled(hdd_ctx->mac_handle, enabled);
	if (enabled)
		return;

	/* fate sharing */
	hdd_ctx->config->bFastRoamInConIniFeatureEnabled = false;
	sme_update_enable_fast_roam_in_concurrency(hdd_ctx->mac_handle, false);
}

static void
cb_notify_set_enable_fast_roam_in_concurrency(struct hdd_context *hdd_ctx,
					      unsigned long notify_id)
{
	bool enabled = hdd_ctx->config->bFastRoamInConIniFeatureEnabled;

	sme_update_enable_fast_roam_in_concurrency(hdd_ctx->mac_handle,
						   enabled);
}

/**
 * cb_notify_set_roam_scan_hi_rssi_scan_params() - configure hi rssi
 * scan params from cfg to sme.
 * @hdd_ctx: HDD context data structure
 * @notify_id: Identifies 1 of the 4 parameters to be modified
 *
 * Picks up the value from hdd configuration and passes it to SME.
 * Return: void
 */

static void
cb_notify_set_roam_scan_hi_rssi_scan_params(struct hdd_context *hdd_ctx,
					    unsigned long notify_id)
{
	int32_t val;

	if (wlan_hdd_validate_context(hdd_ctx))
		return;

	switch (notify_id) {
	case eCSR_HI_RSSI_SCAN_MAXCOUNT_ID:
		val = hdd_ctx->config->nhi_rssi_scan_max_count;
		break;

	case eCSR_HI_RSSI_SCAN_RSSI_DELTA_ID:
		val = hdd_ctx->config->nhi_rssi_scan_rssi_delta;
		break;

	case eCSR_HI_RSSI_SCAN_DELAY_ID:
		val = hdd_ctx->config->nhi_rssi_scan_delay;
		break;

	case eCSR_HI_RSSI_SCAN_RSSI_UB_ID:
		val = hdd_ctx->config->nhi_rssi_scan_rssi_ub;
		break;

	default:
		return;
	}

	sme_update_roam_scan_hi_rssi_scan_params(hdd_ctx->mac_handle, 0,
						 notify_id, val);
}


struct reg_table_entry g_registry_table[] = {
#ifdef WLAN_NUD_TRACKING
	REG_VARIABLE(CFG_ENABLE_NUD_TRACKING_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, enable_nud_tracking,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_NUD_TRACKING_DEFAULT,
		     CFG_ENABLE_NUD_TRACKING_MIN,
		     CFG_ENABLE_NUD_TRACKING_MAX),
#endif

	REG_VARIABLE(CFG_ENABLE_CONNECTED_SCAN_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, enable_connected_scan,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_CONNECTED_SCAN_DEFAULT,
		     CFG_ENABLE_CONNECTED_SCAN_MIN,
		     CFG_ENABLE_CONNECTED_SCAN_MAX),

	REG_VARIABLE(CFG_RTS_THRESHOLD_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, RTSThreshold,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_RTS_THRESHOLD_DEFAULT,
		     CFG_RTS_THRESHOLD_MIN,
		     CFG_RTS_THRESHOLD_MAX),

	REG_VARIABLE(CFG_FRAG_THRESHOLD_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, FragmentationThreshold,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_FRAG_THRESHOLD_DEFAULT,
		     CFG_FRAG_THRESHOLD_MIN,
		     CFG_FRAG_THRESHOLD_MAX),

	REG_VARIABLE(CFG_OPERATING_CHANNEL_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, OperatingChannel,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_OPERATING_CHANNEL_DEFAULT,
		     CFG_OPERATING_CHANNEL_MIN,
		     CFG_OPERATING_CHANNEL_MAX),

	REG_VARIABLE(CFG_SHORT_SLOT_TIME_ENABLED_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, ShortSlotTimeEnabled,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_SHORT_SLOT_TIME_ENABLED_DEFAULT,
		     CFG_SHORT_SLOT_TIME_ENABLED_MIN,
		     CFG_SHORT_SLOT_TIME_ENABLED_MAX),

	REG_VARIABLE(CFG_11D_SUPPORT_ENABLED_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, Is11dSupportEnabled,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_11D_SUPPORT_ENABLED_DEFAULT,
		     CFG_11D_SUPPORT_ENABLED_MIN,
		     CFG_11D_SUPPORT_ENABLED_MAX),

	REG_VARIABLE(CFG_11H_SUPPORT_ENABLED_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, Is11hSupportEnabled,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_11H_SUPPORT_ENABLED_DEFAULT,
		     CFG_11H_SUPPORT_ENABLED_MIN,
		     CFG_11H_SUPPORT_ENABLED_MAX),

	REG_VARIABLE(CFG_COUNTRY_CODE_PRIORITY_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, fSupplicantCountryCodeHasPriority,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_COUNTRY_CODE_PRIORITY_DEFAULT,
		     CFG_COUNTRY_CODE_PRIORITY_MIN,
		     CFG_COUNTRY_CODE_PRIORITY_MAX),

	REG_VARIABLE(CFG_HEARTBEAT_THRESH_24_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, HeartbeatThresh24,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_HEARTBEAT_THRESH_24_DEFAULT,
		     CFG_HEARTBEAT_THRESH_24_MIN,
		     CFG_HEARTBEAT_THRESH_24_MAX),

	REG_VARIABLE_STRING(CFG_POWER_USAGE_NAME, WLAN_PARAM_String,
			    struct hdd_config, PowerUsageControl,
			    VAR_FLAGS_OPTIONAL,
			    (void *)CFG_POWER_USAGE_DEFAULT),

	REG_VARIABLE(CFG_ENABLE_IMPS_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, fIsImpsEnabled,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_IMPS_DEFAULT,
		     CFG_ENABLE_IMPS_MIN,
		     CFG_ENABLE_IMPS_MAX),

	REG_VARIABLE(CFG_ENABLE_PS_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, is_ps_enabled,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_PS_DEFAULT,
		     CFG_ENABLE_PS_MIN,
		     CFG_ENABLE_PS_MAX),

	REG_VARIABLE(CFG_AUTO_PS_ENABLE_TIMER_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, auto_bmps_timer_val,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_AUTO_PS_ENABLE_TIMER_DEFAULT,
		     CFG_AUTO_PS_ENABLE_TIMER_MIN,
		     CFG_AUTO_PS_ENABLE_TIMER_MAX),

#ifdef WLAN_ICMP_DISABLE_PS
	REG_VARIABLE(CFG_ICMP_DISABLE_PS_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, icmp_disable_ps_val,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ICMP_DISABLE_PS_DEFAULT,
		     CFG_ICMP_DISABLE_PS_MIN,
		     CFG_ICMP_DISABLE_PS_MAX),
#endif

	REG_VARIABLE(CFG_BMPS_MINIMUM_LI_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nBmpsMinListenInterval,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_BMPS_MINIMUM_LI_DEFAULT,
		     CFG_BMPS_MINIMUM_LI_MIN,
		     CFG_BMPS_MINIMUM_LI_MAX),

	REG_VARIABLE(CFG_BMPS_MAXIMUM_LI_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nBmpsMaxListenInterval,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_BMPS_MAXIMUM_LI_DEFAULT,
		     CFG_BMPS_MAXIMUM_LI_MIN,
		     CFG_BMPS_MAXIMUM_LI_MAX),

	REG_VARIABLE(CFG_DOT11_MODE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, dot11Mode,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_DOT11_MODE_DEFAULT,
		     CFG_DOT11_MODE_MIN,
		     CFG_DOT11_MODE_MAX),

	REG_VARIABLE(CFG_CHANNEL_BONDING_MODE_24GHZ_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nChannelBondingMode24GHz,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_CHANNEL_BONDING_MODE_DEFAULT,
		     CFG_CHANNEL_BONDING_MODE_MIN,
		     CFG_CHANNEL_BONDING_MODE_MAX),

	REG_VARIABLE(CFG_OVERRIDE_HT40_20_24GHZ_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, override_ht20_40_24g,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_OVERRIDE_HT40_20_24GHZ_DEFAULT,
		     CFG_OVERRIDE_HT40_20_24GHZ_MIN,
		     CFG_OVERRIDE_HT40_20_24GHZ_MAX),

	REG_VARIABLE(CFG_CHANNEL_BONDING_MODE_5GHZ_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nChannelBondingMode5GHz,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_CHANNEL_BONDING_MODE_DEFAULT,
		     CFG_CHANNEL_BONDING_MODE_MIN,
		     CFG_CHANNEL_BONDING_MODE_MAX),

	REG_VARIABLE(CFG_MAX_RX_AMPDU_FACTOR_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, MaxRxAmpduFactor,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_MAX_RX_AMPDU_FACTOR_DEFAULT,
		     CFG_MAX_RX_AMPDU_FACTOR_MIN,
		     CFG_MAX_RX_AMPDU_FACTOR_MAX),

	REG_VARIABLE(CFG_HT_MPDU_DENSITY_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, ht_mpdu_density,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_HT_MPDU_DENSITY_DEFAULT,
		     CFG_HT_MPDU_DENSITY_MIN,
		     CFG_HT_MPDU_DENSITY_MAX),

	REG_VARIABLE(CFG_SCAN_RESULT_AGE_COUNT_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, ScanResultAgeCount,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_SCAN_RESULT_AGE_COUNT_DEFAULT,
		     CFG_SCAN_RESULT_AGE_COUNT_MIN,
		     CFG_SCAN_RESULT_AGE_COUNT_MAX),

	REG_VARIABLE(CFG_RSSI_CATEGORY_GAP_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nRssiCatGap,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_RSSI_CATEGORY_GAP_DEFAULT,
		     CFG_RSSI_CATEGORY_GAP_MIN,
		     CFG_RSSI_CATEGORY_GAP_MAX),

	REG_VARIABLE(CFG_SHORT_PREAMBLE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, fIsShortPreamble,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_SHORT_PREAMBLE_DEFAULT,
		     CFG_SHORT_PREAMBLE_MIN,
		     CFG_SHORT_PREAMBLE_MAX),

	REG_VARIABLE_STRING(CFG_IBSS_BSSID_NAME, WLAN_PARAM_MacAddr,
			    struct hdd_config, IbssBssid,
			    VAR_FLAGS_OPTIONAL,
			    (void *)CFG_IBSS_BSSID_DEFAULT),

	REG_VARIABLE_STRING(CFG_INTF0_MAC_ADDR_NAME, WLAN_PARAM_MacAddr,
			    struct hdd_config, intfMacAddr[0],
			    VAR_FLAGS_OPTIONAL,
			    (void *)CFG_INTF0_MAC_ADDR_DEFAULT),

	REG_VARIABLE_STRING(CFG_INTF1_MAC_ADDR_NAME, WLAN_PARAM_MacAddr,
			    struct hdd_config, intfMacAddr[1],
			    VAR_FLAGS_OPTIONAL,
			    (void *)CFG_INTF1_MAC_ADDR_DEFAULT),

	REG_VARIABLE_STRING(CFG_INTF2_MAC_ADDR_NAME, WLAN_PARAM_MacAddr,
			    struct hdd_config, intfMacAddr[2],
			    VAR_FLAGS_OPTIONAL,
			    (void *)CFG_INTF2_MAC_ADDR_DEFAULT),

	REG_VARIABLE_STRING(CFG_INTF3_MAC_ADDR_NAME, WLAN_PARAM_MacAddr,
			    struct hdd_config, intfMacAddr[3],
			    VAR_FLAGS_OPTIONAL,
			    (void *)CFG_INTF3_MAC_ADDR_DEFAULT),

	REG_VARIABLE(CFG_AP_QOS_UAPSD_MODE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, apUapsdEnabled,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_AP_QOS_UAPSD_MODE_DEFAULT,
		     CFG_AP_QOS_UAPSD_MODE_MIN,
		     CFG_AP_QOS_UAPSD_MODE_MAX),


	REG_VARIABLE(CFG_AP_ENABLE_RANDOM_BSSID_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, apRandomBssidEnabled,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_AP_ENABLE_RANDOM_BSSID_DEFAULT,
		     CFG_AP_ENABLE_RANDOM_BSSID_MIN,
		     CFG_AP_ENABLE_RANDOM_BSSID_MAX),

	REG_VARIABLE(CFG_AP_ENABLE_PROTECTION_MODE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, apProtEnabled,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_AP_ENABLE_PROTECTION_MODE_DEFAULT,
		     CFG_AP_ENABLE_PROTECTION_MODE_MIN,
		     CFG_AP_ENABLE_PROTECTION_MODE_MAX),

	REG_VARIABLE(CFG_AP_PROTECTION_MODE_NAME, WLAN_PARAM_HexInteger,
		     struct hdd_config, apProtection,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_AP_PROTECTION_MODE_DEFAULT,
		     CFG_AP_PROTECTION_MODE_MIN,
		     CFG_AP_PROTECTION_MODE_MAX),

	REG_VARIABLE(CFG_AP_OBSS_PROTECTION_MODE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, apOBSSProtEnabled,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_AP_OBSS_PROTECTION_MODE_DEFAULT,
		     CFG_AP_OBSS_PROTECTION_MODE_MIN,
		     CFG_AP_OBSS_PROTECTION_MODE_MAX),

	REG_VARIABLE(CFG_AP_STA_SECURITY_SEPERATION_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, apDisableIntraBssFwd,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_AP_STA_SECURITY_SEPERATION_DEFAULT,
		     CFG_AP_STA_SECURITY_SEPERATION_MIN,
		     CFG_AP_STA_SECURITY_SEPERATION_MAX),

	REG_VARIABLE(CFG_ENABLE_LTE_COEX, WLAN_PARAM_Integer,
		     struct hdd_config, enableLTECoex,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_LTE_COEX_DEFAULT,
		     CFG_ENABLE_LTE_COEX_MIN,
		     CFG_ENABLE_LTE_COEX_MAX),

#ifdef FEATURE_WLAN_DYNAMIC_CVM
	REG_VARIABLE(CFG_VC_MODE_BITMAP, WLAN_PARAM_HexInteger,
		struct hdd_config, vc_mode_cfg_bitmap,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_VC_MODE_BITMAP_DEFAULT,
		CFG_VC_MODE_BITMAP_MIN,
		CFG_VC_MODE_BITMAP_MAX),
#endif

	REG_VARIABLE(CFG_ENABLE_SAP_MANDATORY_CHAN_LIST, WLAN_PARAM_Integer,
		     struct hdd_config, enable_sap_mandatory_chan_list,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_SAP_MANDATORY_CHAN_LIST_DEFAULT,
		     CFG_ENABLE_SAP_MANDATORY_CHAN_LIST_MIN,
		     CFG_ENABLE_SAP_MANDATORY_CHAN_LIST_MAX),

	REG_VARIABLE(CFG_AP_KEEP_ALIVE_PERIOD_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, apKeepAlivePeriod,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_AP_KEEP_ALIVE_PERIOD_DEFAULT,
		     CFG_AP_KEEP_ALIVE_PERIOD_MIN,
		     CFG_AP_KEEP_ALIVE_PERIOD_MAX),

	REG_VARIABLE(CFG_AP_LINK_MONITOR_PERIOD_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, apLinkMonitorPeriod,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_AP_LINK_MONITOR_PERIOD_DEFAULT,
		     CFG_AP_LINK_MONITOR_PERIOD_MIN,
		     CFG_AP_LINK_MONITOR_PERIOD_MAX),

	REG_VARIABLE(CFG_DISABLE_PACKET_FILTER, WLAN_PARAM_Integer,
		     struct hdd_config, disablePacketFilter,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_DISABLE_PACKET_FILTER_DEFAULT,
		     CFG_DISABLE_PACKET_FILTER_MIN,
		     CFG_DISABLE_PACKET_FILTER_MAX),

	REG_VARIABLE(CFG_BEACON_INTERVAL_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nBeaconInterval,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_BEACON_INTERVAL_DEFAULT,
		     CFG_BEACON_INTERVAL_MIN,
		     CFG_BEACON_INTERVAL_MAX),

	REG_VARIABLE(CFG_VCC_RSSI_TRIGGER_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nVccRssiTrigger,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_VCC_RSSI_TRIGGER_DEFAULT,
		     CFG_VCC_RSSI_TRIGGER_MIN,
		     CFG_VCC_RSSI_TRIGGER_MAX),

	REG_VARIABLE(CFG_VCC_UL_MAC_LOSS_THRESH_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nVccUlMacLossThreshold,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_VCC_UL_MAC_LOSS_THRESH_DEFAULT,
		     CFG_VCC_UL_MAC_LOSS_THRESH_MIN,
		     CFG_VCC_UL_MAC_LOSS_THRESH_MAX),

	REG_VARIABLE(CFG_PASSIVE_MAX_CHANNEL_TIME_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nPassiveMaxChnTime,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_PASSIVE_MAX_CHANNEL_TIME_DEFAULT,
		     CFG_PASSIVE_MAX_CHANNEL_TIME_MIN,
		     CFG_PASSIVE_MAX_CHANNEL_TIME_MAX),

	REG_VARIABLE(CFG_ACTIVE_MAX_CHANNEL_TIME_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nActiveMaxChnTime,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ACTIVE_MAX_CHANNEL_TIME_DEFAULT,
		     CFG_ACTIVE_MAX_CHANNEL_TIME_MIN,
		     CFG_ACTIVE_MAX_CHANNEL_TIME_MAX),

	REG_VARIABLE(CFG_SCAN_NUM_PROBES_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, scan_num_probes,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_SCAN_NUM_PROBES_DEFAULT,
		     CFG_SCAN_NUM_PROBES_MIN,
		     CFG_SCAN_NUM_PROBES_MAX),

	REG_VARIABLE(CFG_SCAN_PROBE_REPEAT_TIME_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, scan_probe_repeat_time,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_SCAN_PROBE_REPEAT_TIME_DEFAULT,
		     CFG_SCAN_PROBE_REPEAT_TIME_MIN,
		     CFG_SCAN_PROBE_REPEAT_TIME_MAX),

	REG_VARIABLE(CFG_PASSIVE_MAX_CHANNEL_TIME_CONC_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nPassiveMaxChnTimeConc,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_PASSIVE_MAX_CHANNEL_TIME_CONC_DEFAULT,
		     CFG_PASSIVE_MAX_CHANNEL_TIME_CONC_MIN,
		     CFG_PASSIVE_MAX_CHANNEL_TIME_CONC_MAX),

	REG_VARIABLE(CFG_PASSIVE_MIN_CHANNEL_TIME_CONC_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nPassiveMinChnTimeConc,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_PASSIVE_MIN_CHANNEL_TIME_CONC_DEFAULT,
		     CFG_PASSIVE_MIN_CHANNEL_TIME_CONC_MIN,
		     CFG_PASSIVE_MIN_CHANNEL_TIME_CONC_MAX),

	REG_VARIABLE(CFG_ACTIVE_MAX_CHANNEL_TIME_CONC_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nActiveMaxChnTimeConc,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ACTIVE_MAX_CHANNEL_TIME_CONC_DEFAULT,
		     CFG_ACTIVE_MAX_CHANNEL_TIME_CONC_MIN,
		     CFG_ACTIVE_MAX_CHANNEL_TIME_CONC_MAX),

	REG_VARIABLE(CFG_ACTIVE_MIN_CHANNEL_TIME_CONC_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nActiveMinChnTimeConc,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ACTIVE_MIN_CHANNEL_TIME_CONC_DEFAULT,
		     CFG_ACTIVE_MIN_CHANNEL_TIME_CONC_MIN,
		     CFG_ACTIVE_MIN_CHANNEL_TIME_CONC_MAX),

	REG_VARIABLE(CFG_REST_TIME_CONC_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nRestTimeConc,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_REST_TIME_CONC_DEFAULT,
		     CFG_REST_TIME_CONC_MIN,
		     CFG_REST_TIME_CONC_MAX),

	REG_VARIABLE(CFG_MIN_REST_TIME_NAME, WLAN_PARAM_Integer,
		      struct hdd_config, min_rest_time_conc,
		      VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		      CFG_MIN_REST_TIME_DEFAULT,
		      CFG_MIN_REST_TIME_MIN,
		      CFG_MIN_REST_TIME_MAX),

	REG_VARIABLE(CFG_IDLE_TIME_NAME, WLAN_PARAM_Integer,
		      struct hdd_config, idle_time_conc,
		      VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		      CFG_IDLE_TIME_DEFAULT,
		      CFG_IDLE_TIME_MIN,
		      CFG_IDLE_TIME_MAX),

	REG_VARIABLE(CFG_MAX_PS_POLL_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nMaxPsPoll,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_MAX_PS_POLL_DEFAULT,
		     CFG_MAX_PS_POLL_MIN,
		     CFG_MAX_PS_POLL_MAX),

	REG_VARIABLE(CFG_MAX_TX_POWER_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nTxPowerCap,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_MAX_TX_POWER_DEFAULT,
		     CFG_MAX_TX_POWER_MIN,
		     CFG_MAX_TX_POWER_MAX),

	REG_VARIABLE(CFG_TX_POWER_CTRL_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, allow_tpc_from_ap,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TX_POWER_CTRL_DEFAULT,
		     CFG_TX_POWER_CTRL_MIN,
		     CFG_TX_POWER_CTRL_MAX),

	REG_VARIABLE(CFG_MAX_LI_MODULATED_DTIM_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, fMaxLIModulatedDTIM,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_MAX_LI_MODULATED_DTIM_DEFAULT,
		     CFG_MAX_LI_MODULATED_DTIM_MIN,
		     CFG_MAX_LI_MODULATED_DTIM_MAX),

	REG_VARIABLE(CFG_FW_MCC_RTS_CTS_PROT_NAME, WLAN_PARAM_Integer,
		struct hdd_config, mcc_rts_cts_prot_enable,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_FW_MCC_RTS_CTS_PROT_DEFAULT,
		CFG_FW_MCC_RTS_CTS_PROT_MIN,
		CFG_FW_MCC_RTS_CTS_PROT_MAX),

	REG_VARIABLE(CFG_FW_MCC_BCAST_PROB_RESP_NAME, WLAN_PARAM_Integer,
		struct hdd_config, mcc_bcast_prob_resp_enable,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_FW_MCC_BCAST_PROB_RESP_DEFAULT,
		CFG_FW_MCC_BCAST_PROB_RESP_MIN,
		CFG_FW_MCC_BCAST_PROB_RESP_MAX),

	REG_VARIABLE(CFG_DATA_INACTIVITY_TIMEOUT_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nDataInactivityTimeout,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_DATA_INACTIVITY_TIMEOUT_DEFAULT,
		     CFG_DATA_INACTIVITY_TIMEOUT_MIN,
		     CFG_DATA_INACTIVITY_TIMEOUT_MAX),

	REG_VARIABLE(CFG_WOW_DATA_INACTIVITY_TIMEOUT_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, wow_data_inactivity_timeout,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_WOW_DATA_INACTIVITY_TIMEOUT_DEFAULT,
		     CFG_WOW_DATA_INACTIVITY_TIMEOUT_MIN,
		     CFG_WOW_DATA_INACTIVITY_TIMEOUT_MAX),

	REG_VARIABLE(CFG_QOS_WMM_MODE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, WmmMode,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_WMM_MODE_DEFAULT,
		     CFG_QOS_WMM_MODE_MIN,
		     CFG_QOS_WMM_MODE_MAX),

	REG_VARIABLE(CFG_QOS_WMM_80211E_ENABLED_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, b80211eIsEnabled,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_WMM_80211E_ENABLED_DEFAULT,
		     CFG_QOS_WMM_80211E_ENABLED_MIN,
		     CFG_QOS_WMM_80211E_ENABLED_MAX),

	REG_VARIABLE(CFG_QOS_WMM_UAPSD_MASK_NAME, WLAN_PARAM_HexInteger,
		     struct hdd_config, UapsdMask,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_WMM_UAPSD_MASK_DEFAULT,
		     CFG_QOS_WMM_UAPSD_MASK_MIN,
		     CFG_QOS_WMM_UAPSD_MASK_MAX),

	REG_VARIABLE(CFG_QOS_WMM_INFRA_UAPSD_VO_SRV_INTV_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, InfraUapsdVoSrvIntv,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_WMM_INFRA_UAPSD_VO_SRV_INTV_DEFAULT,
		     CFG_QOS_WMM_INFRA_UAPSD_VO_SRV_INTV_MIN,
		     CFG_QOS_WMM_INFRA_UAPSD_VO_SRV_INTV_MAX),

	REG_VARIABLE(CFG_QOS_WMM_INFRA_UAPSD_VO_SUS_INTV_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, InfraUapsdVoSuspIntv,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_WMM_INFRA_UAPSD_VO_SUS_INTV_DEFAULT,
		     CFG_QOS_WMM_INFRA_UAPSD_VO_SUS_INTV_MIN,
		     CFG_QOS_WMM_INFRA_UAPSD_VO_SUS_INTV_MAX),

	REG_VARIABLE(CFG_QOS_WMM_INFRA_UAPSD_VI_SRV_INTV_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, InfraUapsdViSrvIntv,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_WMM_INFRA_UAPSD_VI_SRV_INTV_DEFAULT,
		     CFG_QOS_WMM_INFRA_UAPSD_VI_SRV_INTV_MIN,
		     CFG_QOS_WMM_INFRA_UAPSD_VI_SRV_INTV_MAX),

	REG_VARIABLE(CFG_QOS_WMM_INFRA_UAPSD_VI_SUS_INTV_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, InfraUapsdViSuspIntv,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_WMM_INFRA_UAPSD_VI_SUS_INTV_DEFAULT,
		     CFG_QOS_WMM_INFRA_UAPSD_VI_SUS_INTV_MIN,
		     CFG_QOS_WMM_INFRA_UAPSD_VI_SUS_INTV_MAX),

	REG_VARIABLE(CFG_QOS_WMM_INFRA_UAPSD_BE_SRV_INTV_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, InfraUapsdBeSrvIntv,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_WMM_INFRA_UAPSD_BE_SRV_INTV_DEFAULT,
		     CFG_QOS_WMM_INFRA_UAPSD_BE_SRV_INTV_MIN,
		     CFG_QOS_WMM_INFRA_UAPSD_BE_SRV_INTV_MAX),

	REG_VARIABLE(CFG_QOS_WMM_INFRA_UAPSD_BE_SUS_INTV_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, InfraUapsdBeSuspIntv,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_WMM_INFRA_UAPSD_BE_SUS_INTV_DEFAULT,
		     CFG_QOS_WMM_INFRA_UAPSD_BE_SUS_INTV_MIN,
		     CFG_QOS_WMM_INFRA_UAPSD_BE_SUS_INTV_MAX),

	REG_VARIABLE(CFG_QOS_WMM_INFRA_UAPSD_BK_SRV_INTV_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, InfraUapsdBkSrvIntv,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_WMM_INFRA_UAPSD_BK_SRV_INTV_DEFAULT,
		     CFG_QOS_WMM_INFRA_UAPSD_BK_SRV_INTV_MIN,
		     CFG_QOS_WMM_INFRA_UAPSD_BK_SRV_INTV_MAX),

	REG_VARIABLE(CFG_QOS_WMM_INFRA_UAPSD_BK_SUS_INTV_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, InfraUapsdBkSuspIntv,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_WMM_INFRA_UAPSD_BK_SUS_INTV_DEFAULT,
		     CFG_QOS_WMM_INFRA_UAPSD_BK_SUS_INTV_MIN,
		     CFG_QOS_WMM_INFRA_UAPSD_BK_SUS_INTV_MAX),

#ifdef FEATURE_WLAN_ESE
	REG_VARIABLE(CFG_QOS_WMM_INFRA_INACTIVITY_INTERVAL_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, InfraInactivityInterval,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_WMM_INFRA_INACTIVITY_INTERVAL_DEFAULT,
		     CFG_QOS_WMM_INFRA_INACTIVITY_INTERVAL_MIN,
		     CFG_QOS_WMM_INFRA_INACTIVITY_INTERVAL_MAX),

	REG_DYNAMIC_VARIABLE(CFG_ESE_FEATURE_ENABLED_NAME, WLAN_PARAM_Integer,
			     struct hdd_config, isEseIniFeatureEnabled,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_ESE_FEATURE_ENABLED_DEFAULT,
			     CFG_ESE_FEATURE_ENABLED_MIN,
			     CFG_ESE_FEATURE_ENABLED_MAX,
			     cb_notify_set_ese_feature_enabled, 0),
#endif /* FEATURE_WLAN_ESE */

	/* flag to turn ON/OFF Legacy Fast Roaming */
	REG_DYNAMIC_VARIABLE(CFG_LFR_FEATURE_ENABLED_NAME, WLAN_PARAM_Integer,
			     struct hdd_config, isFastRoamIniFeatureEnabled,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_LFR_FEATURE_ENABLED_DEFAULT,
			     CFG_LFR_FEATURE_ENABLED_MIN,
			     CFG_LFR_FEATURE_ENABLED_MAX,
			     notify_is_fast_roam_ini_feature_enabled, 0),

	/* flag to turn ON/OFF Motion assistance for Legacy Fast Roaming */
	REG_DYNAMIC_VARIABLE(CFG_LFR_MAWC_FEATURE_ENABLED_NAME,
			     WLAN_PARAM_Integer,
			     struct hdd_config, MAWCEnabled,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_LFR_MAWC_FEATURE_ENABLED_DEFAULT,
			     CFG_LFR_MAWC_FEATURE_ENABLED_MIN,
			     CFG_LFR_MAWC_FEATURE_ENABLED_MAX,
			     notify_is_mawc_ini_feature_enabled, 0),

	REG_VARIABLE(CFG_MAWC_ROAM_ENABLED_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, mawc_roam_enabled,
		     VAR_FLAGS_OPTIONAL |
		     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_MAWC_ROAM_ENABLED_DEFAULT,
		     CFG_MAWC_ROAM_ENABLED_MIN,
		     CFG_MAWC_ROAM_ENABLED_MAX),

	REG_VARIABLE(CFG_MAWC_ROAM_TRAFFIC_THRESHOLD_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, mawc_roam_traffic_threshold,
		     VAR_FLAGS_OPTIONAL |
		     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_MAWC_ROAM_TRAFFIC_THRESHOLD_DEFAULT,
		     CFG_MAWC_ROAM_TRAFFIC_THRESHOLD_MIN,
		     CFG_MAWC_ROAM_TRAFFIC_THRESHOLD_MAX),

	REG_VARIABLE(CFG_MAWC_ROAM_AP_RSSI_THRESHOLD_NAME,
		     WLAN_PARAM_SignedInteger, struct hdd_config,
		     mawc_roam_ap_rssi_threshold, VAR_FLAGS_OPTIONAL |
		     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_MAWC_ROAM_AP_RSSI_THRESHOLD_DEFAULT,
		     CFG_MAWC_ROAM_AP_RSSI_THRESHOLD_MIN,
		     CFG_MAWC_ROAM_AP_RSSI_THRESHOLD_MAX),

	REG_VARIABLE(CFG_MAWC_ROAM_RSSI_HIGH_ADJUST_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, mawc_roam_rssi_high_adjust,
		     VAR_FLAGS_OPTIONAL |
		     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_MAWC_ROAM_RSSI_HIGH_ADJUST_DEFAULT,
		     CFG_MAWC_ROAM_RSSI_HIGH_ADJUST_MIN,
		     CFG_MAWC_ROAM_RSSI_HIGH_ADJUST_MAX),

	REG_VARIABLE(CFG_MAWC_ROAM_RSSI_LOW_ADJUST_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, mawc_roam_rssi_low_adjust,
		     VAR_FLAGS_OPTIONAL |
		     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_MAWC_ROAM_RSSI_LOW_ADJUST_DEFAULT,
		     CFG_MAWC_ROAM_RSSI_LOW_ADJUST_MIN,
		     CFG_MAWC_ROAM_RSSI_LOW_ADJUST_MAX),

	/* flag to turn ON/OFF 11r and ESE FastTransition */
	REG_DYNAMIC_VARIABLE(CFG_FAST_TRANSITION_ENABLED_NAME,
			     WLAN_PARAM_Integer,
			     struct hdd_config, isFastTransitionEnabled,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_FAST_TRANSITION_ENABLED_NAME_DEFAULT,
			     CFG_FAST_TRANSITION_ENABLED_NAME_MIN,
			     CFG_FAST_TRANSITION_ENABLED_NAME_MAX,
			     cb_notify_set_fast_transition_enabled, 0),

	/* Variable to specify the delta/difference between the
	 * RSSI of current AP and roamable AP while roaming
	 */
	REG_DYNAMIC_VARIABLE(CFG_ROAM_RSSI_DIFF_NAME, WLAN_PARAM_Integer,
			     struct hdd_config, RoamRssiDiff,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_ROAM_RSSI_DIFF_DEFAULT,
			     CFG_ROAM_RSSI_DIFF_MIN,
			     CFG_ROAM_RSSI_DIFF_MAX,
			     cb_notify_set_roam_rssi_diff, 0),

	REG_VARIABLE(CFG_ROAM_RSSI_ABS_THRESHOLD_NAME, WLAN_PARAM_SignedInteger,
		     struct hdd_config, rssi_abs_thresh,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ROAM_RSSI_ABS_THRESHOLD_DEFAULT,
		     CFG_ROAM_RSSI_ABS_THRESHOLD_MIN,
		     CFG_ROAM_RSSI_ABS_THRESHOLD_MAX),

	REG_DYNAMIC_VARIABLE(CFG_ENABLE_WES_MODE_NAME, WLAN_PARAM_Integer,
			     struct hdd_config, isWESModeEnabled,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_ENABLE_WES_MODE_NAME_DEFAULT,
			     CFG_ENABLE_WES_MODE_NAME_MIN,
			     CFG_ENABLE_WES_MODE_NAME_MAX,
			     cb_notify_set_wes_mode, 0),
	REG_VARIABLE(CFG_PMKID_MODES_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, pmkid_modes,
		     VAR_FLAGS_OPTIONAL |
		     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_PMKID_MODES_DEFAULT,
		     CFG_PMKID_MODES_MIN,
		     CFG_PMKID_MODES_MAX),

	REG_DYNAMIC_VARIABLE(CFG_ROAM_SCAN_OFFLOAD_ENABLED, WLAN_PARAM_Integer,
			     struct hdd_config, isRoamOffloadScanEnabled,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_ROAM_SCAN_OFFLOAD_ENABLED_DEFAULT,
			     CFG_ROAM_SCAN_OFFLOAD_ENABLED_MIN,
			     CFG_ROAM_SCAN_OFFLOAD_ENABLED_MAX,
			     cb_notify_update_roam_scan_offload_enabled, 0),

	REG_VARIABLE(CFG_QOS_WMM_INFRA_DIR_AC_VO_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, InfraDirAcVo,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_WMM_INFRA_DIR_AC_VO_DEFAULT,
		     CFG_QOS_WMM_INFRA_DIR_AC_VO_MIN,
		     CFG_QOS_WMM_INFRA_DIR_AC_VO_MAX),

	REG_VARIABLE(CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VO_NAME,
		     WLAN_PARAM_HexInteger,
		     struct hdd_config, InfraNomMsduSizeAcVo,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VO_DEFAULT,
		     CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VO_MIN,
		     CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VO_MAX),

	REG_VARIABLE(CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VO_NAME,
		     WLAN_PARAM_HexInteger,
		     struct hdd_config, InfraMeanDataRateAcVo,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VO_DEFAULT,
		     CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VO_MIN,
		     CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VO_MAX),

	REG_VARIABLE(CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VO_NAME,
		     WLAN_PARAM_HexInteger,
		     struct hdd_config, InfraMinPhyRateAcVo,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VO_DEFAULT,
		     CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VO_MIN,
		     CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VO_MAX),

	REG_VARIABLE(CFG_QOS_WMM_INFRA_SBA_AC_VO_NAME, WLAN_PARAM_HexInteger,
		     struct hdd_config, InfraSbaAcVo,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_WMM_INFRA_SBA_AC_VO_DEFAULT,
		     CFG_QOS_WMM_INFRA_SBA_AC_VO_MIN,
		     CFG_QOS_WMM_INFRA_SBA_AC_VO_MAX),

	REG_VARIABLE(CFG_QOS_WMM_INFRA_DIR_AC_VI_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, InfraDirAcVi,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_WMM_INFRA_DIR_AC_VI_DEFAULT,
		     CFG_QOS_WMM_INFRA_DIR_AC_VI_MIN,
		     CFG_QOS_WMM_INFRA_DIR_AC_VI_MAX),

	REG_VARIABLE(CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VI_NAME,
		     WLAN_PARAM_HexInteger,
		     struct hdd_config, InfraNomMsduSizeAcVi,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VI_DEFAULT,
		     CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VI_MIN,
		     CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VI_MAX),

	REG_VARIABLE(CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VI_NAME,
		     WLAN_PARAM_HexInteger,
		     struct hdd_config, InfraMeanDataRateAcVi,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VI_DEFAULT,
		     CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VI_MIN,
		     CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VI_MAX),

	REG_VARIABLE(CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VI_NAME,
		     WLAN_PARAM_HexInteger,
		     struct hdd_config, InfraMinPhyRateAcVi,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VI_DEFAULT,
		     CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VI_MIN,
		     CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VI_MAX),

	REG_VARIABLE(CFG_QOS_WMM_INFRA_SBA_AC_VI_NAME, WLAN_PARAM_HexInteger,
		     struct hdd_config, InfraSbaAcVi,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_WMM_INFRA_SBA_AC_VI_DEFAULT,
		     CFG_QOS_WMM_INFRA_SBA_AC_VI_MIN,
		     CFG_QOS_WMM_INFRA_SBA_AC_VI_MAX),

	REG_VARIABLE(CFG_QOS_WMM_INFRA_DIR_AC_BE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, InfraDirAcBe,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_WMM_INFRA_DIR_AC_BE_DEFAULT,
		     CFG_QOS_WMM_INFRA_DIR_AC_BE_MIN,
		     CFG_QOS_WMM_INFRA_DIR_AC_BE_MAX),

	REG_VARIABLE(CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BE_NAME,
		     WLAN_PARAM_HexInteger,
		     struct hdd_config, InfraNomMsduSizeAcBe,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BE_DEFAULT,
		     CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BE_MIN,
		     CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BE_MAX),

	REG_VARIABLE(CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BE_NAME,
		     WLAN_PARAM_HexInteger,
		     struct hdd_config, InfraMeanDataRateAcBe,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BE_DEFAULT,
		     CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BE_MIN,
		     CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BE_MAX),

	REG_VARIABLE(CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BE_NAME,
		     WLAN_PARAM_HexInteger,
		     struct hdd_config, InfraMinPhyRateAcBe,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BE_DEFAULT,
		     CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BE_MIN,
		     CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BE_MAX),

	REG_VARIABLE(CFG_QOS_WMM_INFRA_SBA_AC_BE_NAME, WLAN_PARAM_HexInteger,
		     struct hdd_config, InfraSbaAcBe,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_WMM_INFRA_SBA_AC_BE_DEFAULT,
		     CFG_QOS_WMM_INFRA_SBA_AC_BE_MIN,
		     CFG_QOS_WMM_INFRA_SBA_AC_BE_MAX),

	REG_VARIABLE(CFG_QOS_WMM_INFRA_DIR_AC_BK_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, InfraDirAcBk,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_WMM_INFRA_DIR_AC_BK_DEFAULT,
		     CFG_QOS_WMM_INFRA_DIR_AC_BK_MIN,
		     CFG_QOS_WMM_INFRA_DIR_AC_BK_MAX),

	REG_VARIABLE(CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BK_NAME,
		     WLAN_PARAM_HexInteger,
		     struct hdd_config, InfraNomMsduSizeAcBk,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BK_DEFAULT,
		     CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BK_MIN,
		     CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BK_MAX),

	REG_VARIABLE(CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BK_NAME,
		     WLAN_PARAM_HexInteger,
		     struct hdd_config, InfraMeanDataRateAcBk,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BK_DEFAULT,
		     CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BK_MIN,
		     CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BK_MAX),

	REG_VARIABLE(CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BK_NAME,
		     WLAN_PARAM_HexInteger,
		     struct hdd_config, InfraMinPhyRateAcBk,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BK_DEFAULT,
		     CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BK_MIN,
		     CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BK_MAX),

	REG_VARIABLE(CFG_QOS_WMM_INFRA_SBA_AC_BK_NAME, WLAN_PARAM_HexInteger,
		     struct hdd_config, InfraSbaAcBk,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_WMM_INFRA_SBA_AC_BK_DEFAULT,
		     CFG_QOS_WMM_INFRA_SBA_AC_BK_MIN,
		     CFG_QOS_WMM_INFRA_SBA_AC_BK_MAX),

	REG_VARIABLE(CFG_TL_DELAYED_TRGR_FRM_INT_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, DelayedTriggerFrmInt,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TL_DELAYED_TRGR_FRM_INT_DEFAULT,
		     CFG_TL_DELAYED_TRGR_FRM_INT_MIN,
		     CFG_TL_DELAYED_TRGR_FRM_INT_MAX),

	REG_VARIABLE(CFG_QOS_IMPLICIT_SETUP_ENABLED_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, bImplicitQosEnabled,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_IMPLICIT_SETUP_ENABLED_DEFAULT,
		     CFG_QOS_IMPLICIT_SETUP_ENABLED_MIN,
		     CFG_QOS_IMPLICIT_SETUP_ENABLED_MAX),

#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
	REG_VARIABLE(CFG_WLAN_MCC_TO_SCC_SWITCH_MODE, WLAN_PARAM_Integer,
		     struct hdd_config, WlanMccToSccSwitchMode,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_WLAN_MCC_TO_SCC_SWITCH_MODE_DEFAULT,
		     CFG_WLAN_MCC_TO_SCC_SWITCH_MODE_MIN,
		     CFG_WLAN_MCC_TO_SCC_SWITCH_MODE_MAX),
#endif
#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
	REG_VARIABLE(CFG_WLAN_AUTO_SHUTDOWN, WLAN_PARAM_Integer,
		     struct hdd_config, WlanAutoShutdown,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_WLAN_AUTO_SHUTDOWN_DEFAULT,
		     CFG_WLAN_AUTO_SHUTDOWN_MIN,
		     CFG_WLAN_AUTO_SHUTDOWN_MAX),
#endif
	REG_VARIABLE(CFG_RRM_ENABLE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, fRrmEnable,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_RRM_ENABLE_DEFAULT,
		     CFG_RRM_ENABLE_MIN,
		     CFG_RRM_ENABLE_MAX),

	REG_VARIABLE(CFG_RRM_MEAS_RANDOMIZATION_INTVL_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nRrmRandnIntvl,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_RRM_MEAS_RANDOMIZATION_INTVL_DEFAULT,
		     CFG_RRM_MEAS_RANDOMIZATION_INTVL_MIN,
		     CFG_RRM_MEAS_RANDOMIZATION_INTVL_MAX),

	REG_VARIABLE_STRING(CFG_RM_CAPABILITY_NAME, WLAN_PARAM_String,
			    struct hdd_config, rm_capability,
			    VAR_FLAGS_OPTIONAL,
			    (void *) CFG_RM_CAPABILITY_DEFAULT),

	REG_DYNAMIC_VARIABLE(CFG_NEIGHBOR_SCAN_TIMER_PERIOD_NAME,
			     WLAN_PARAM_Integer,
			     struct hdd_config, nNeighborScanPeriod,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_NEIGHBOR_SCAN_TIMER_PERIOD_DEFAULT,
			     CFG_NEIGHBOR_SCAN_TIMER_PERIOD_MIN,
			     CFG_NEIGHBOR_SCAN_TIMER_PERIOD_MAX,
			     cb_notify_set_neighbor_scan_period, 0),

	REG_DYNAMIC_VARIABLE(CFG_NEIGHBOR_SCAN_MIN_TIMER_PERIOD_NAME,
			     WLAN_PARAM_Integer,
			     struct hdd_config, neighbor_scan_min_period,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_NEIGHBOR_SCAN_MIN_TIMER_PERIOD_DEFAULT,
			     CFG_NEIGHBOR_SCAN_MIN_TIMER_PERIOD_MIN,
			     CFG_NEIGHBOR_SCAN_MIN_TIMER_PERIOD_MAX,
			     cb_notify_set_neighbor_scan_min_period, 0),

	REG_DYNAMIC_VARIABLE(CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_NAME,
			     WLAN_PARAM_Integer,
			     struct hdd_config, nNeighborLookupRssiThreshold,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_DEFAULT,
			     CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_MIN,
			     CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_MAX,
			     cb_notify_set_neighbor_lookup_rssi_threshold, 0),

	REG_VARIABLE(CFG_5G_RSSI_THRESHOLD_OFFSET_NAME,
		     WLAN_PARAM_SignedInteger, struct hdd_config,
		     rssi_thresh_offset_5g,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_5G_RSSI_THRESHOLD_OFFSET_DEFAULT,
		     CFG_5G_RSSI_THRESHOLD_OFFSET_MIN,
		     CFG_5G_RSSI_THRESHOLD_OFFSET_MAX),

	REG_DYNAMIC_VARIABLE(CFG_OPPORTUNISTIC_SCAN_THRESHOLD_DIFF_NAME,
			     WLAN_PARAM_Integer,
			     struct hdd_config, nOpportunisticThresholdDiff,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_OPPORTUNISTIC_SCAN_THRESHOLD_DIFF_DEFAULT,
			     CFG_OPPORTUNISTIC_SCAN_THRESHOLD_DIFF_MIN,
			     CFG_OPPORTUNISTIC_SCAN_THRESHOLD_DIFF_MAX,
			     cb_notify_set_opportunistic_scan_threshold_diff,
			     0),

	REG_DYNAMIC_VARIABLE(CFG_ROAM_RESCAN_RSSI_DIFF_NAME, WLAN_PARAM_Integer,
			     struct hdd_config, nRoamRescanRssiDiff,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_ROAM_RESCAN_RSSI_DIFF_DEFAULT,
			     CFG_ROAM_RESCAN_RSSI_DIFF_MIN,
			     CFG_ROAM_RESCAN_RSSI_DIFF_MAX,
			     cb_notify_set_roam_rescan_rssi_diff, 0),

	REG_VARIABLE_STRING(CFG_NEIGHBOR_SCAN_CHAN_LIST_NAME, WLAN_PARAM_String,
			    struct hdd_config, neighborScanChanList,
			    VAR_FLAGS_OPTIONAL,
			    (void *)CFG_NEIGHBOR_SCAN_CHAN_LIST_DEFAULT),

	REG_DYNAMIC_VARIABLE(CFG_NEIGHBOR_SCAN_MIN_CHAN_TIME_NAME,
			     WLAN_PARAM_Integer,
			     struct hdd_config, nNeighborScanMinChanTime,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_NEIGHBOR_SCAN_MIN_CHAN_TIME_DEFAULT,
			     CFG_NEIGHBOR_SCAN_MIN_CHAN_TIME_MIN,
			     CFG_NEIGHBOR_SCAN_MIN_CHAN_TIME_MAX,
			     cb_notify_set_neighbor_scan_min_chan_time, 0),

	REG_DYNAMIC_VARIABLE(CFG_NEIGHBOR_SCAN_MAX_CHAN_TIME_NAME,
			     WLAN_PARAM_Integer,
			     struct hdd_config, nNeighborScanMaxChanTime,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_NEIGHBOR_SCAN_MAX_CHAN_TIME_DEFAULT,
			     CFG_NEIGHBOR_SCAN_MAX_CHAN_TIME_MIN,
			     CFG_NEIGHBOR_SCAN_MAX_CHAN_TIME_MAX,
			     cb_notify_set_neighbor_scan_max_chan_time, 0),

	REG_VARIABLE(CFG_11R_NEIGHBOR_REQ_MAX_TRIES_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nMaxNeighborReqTries,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_11R_NEIGHBOR_REQ_MAX_TRIES_DEFAULT,
		     CFG_11R_NEIGHBOR_REQ_MAX_TRIES_MIN,
		     CFG_11R_NEIGHBOR_REQ_MAX_TRIES_MAX),

	REG_DYNAMIC_VARIABLE(CFG_NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_NAME,
			     WLAN_PARAM_Integer,
			     struct hdd_config, nNeighborResultsRefreshPeriod,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_DEFAULT,
			     CFG_NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_MIN,
			     CFG_NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_MAX,
			     cb_notify_set_neighbor_results_refresh_period, 0),

	REG_DYNAMIC_VARIABLE(CFG_EMPTY_SCAN_REFRESH_PERIOD_NAME,
			     WLAN_PARAM_Integer,
			     struct hdd_config, nEmptyScanRefreshPeriod,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_EMPTY_SCAN_REFRESH_PERIOD_DEFAULT,
			     CFG_EMPTY_SCAN_REFRESH_PERIOD_MIN,
			     CFG_EMPTY_SCAN_REFRESH_PERIOD_MAX,
			     cb_notify_set_empty_scan_refresh_period, 0),

	REG_DYNAMIC_VARIABLE(CFG_ROAM_BMISS_FIRST_BCNT_NAME, WLAN_PARAM_Integer,
			     struct hdd_config, nRoamBmissFirstBcnt,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_ROAM_BMISS_FIRST_BCNT_DEFAULT,
			     CFG_ROAM_BMISS_FIRST_BCNT_MIN,
			     CFG_ROAM_BMISS_FIRST_BCNT_MAX,
			     cb_notify_set_roam_bmiss_first_bcnt, 0),

	REG_DYNAMIC_VARIABLE(CFG_ROAM_BMISS_FINAL_BCNT_NAME, WLAN_PARAM_Integer,
			     struct hdd_config, nRoamBmissFinalBcnt,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_ROAM_BMISS_FINAL_BCNT_DEFAULT,
			     CFG_ROAM_BMISS_FINAL_BCNT_MIN,
			     CFG_ROAM_BMISS_FINAL_BCNT_MAX,
			     cb_notify_set_roam_bmiss_final_bcnt, 0),

	REG_DYNAMIC_VARIABLE(CFG_ROAM_BEACON_RSSI_WEIGHT_NAME,
			     WLAN_PARAM_Integer,
			     struct hdd_config, nRoamBeaconRssiWeight,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_ROAM_BEACON_RSSI_WEIGHT_DEFAULT,
			     CFG_ROAM_BEACON_RSSI_WEIGHT_MIN,
			     CFG_ROAM_BEACON_RSSI_WEIGHT_MAX,
			     cb_notify_set_roam_beacon_rssi_weight, 0),

	REG_DYNAMIC_VARIABLE(CFG_ROAMING_DFS_CHANNEL_NAME, WLAN_PARAM_Integer,
			     struct hdd_config, allowDFSChannelRoam,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_ROAMING_DFS_CHANNEL_DEFAULT,
			     CFG_ROAMING_DFS_CHANNEL_MIN,
			     CFG_ROAMING_DFS_CHANNEL_MAX,
			     cb_notify_set_dfs_scan_mode, 0),

	REG_DYNAMIC_VARIABLE(CFG_DELAY_BEFORE_VDEV_STOP_NAME,
			     WLAN_PARAM_Integer,
			     struct hdd_config,
			     delay_before_vdev_stop,
			     VAR_FLAGS_OPTIONAL |
				VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_DELAY_BEFORE_VDEV_STOP_DEFAULT,
			     CFG_DELAY_BEFORE_VDEV_STOP_MIN,
			     CFG_DELAY_BEFORE_VDEV_STOP_MAX,
			     cb_notify_set_delay_before_vdev_stop,
			     0),

	REG_DYNAMIC_VARIABLE(CFG_ROAM_SCAN_HI_RSSI_MAXCOUNT_NAME,
			     WLAN_PARAM_Integer,
			     struct hdd_config,
			     nhi_rssi_scan_max_count,
			     VAR_FLAGS_OPTIONAL |
				VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_ROAM_SCAN_HI_RSSI_MAXCOUNT_DEFAULT,
			     CFG_ROAM_SCAN_HI_RSSI_MAXCOUNT_MIN,
			     CFG_ROAM_SCAN_HI_RSSI_MAXCOUNT_MAX,
			     cb_notify_set_roam_scan_hi_rssi_scan_params,
			     eCSR_HI_RSSI_SCAN_MAXCOUNT_ID),

	REG_DYNAMIC_VARIABLE(CFG_ROAM_SCAN_HI_RSSI_DELTA_NAME,
			     WLAN_PARAM_Integer,
			     struct hdd_config,
			     nhi_rssi_scan_rssi_delta,
			     VAR_FLAGS_OPTIONAL |
				VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_ROAM_SCAN_HI_RSSI_DELTA_DEFAULT,
			     CFG_ROAM_SCAN_HI_RSSI_DELTA_MIN,
			     CFG_ROAM_SCAN_HI_RSSI_DELTA_MAX,
			     cb_notify_set_roam_scan_hi_rssi_scan_params,
			     eCSR_HI_RSSI_SCAN_RSSI_DELTA_ID),

	REG_DYNAMIC_VARIABLE(CFG_ROAM_SCAN_HI_RSSI_DELAY_NAME,
			     WLAN_PARAM_Integer,
			     struct hdd_config,
			     nhi_rssi_scan_delay,
			     VAR_FLAGS_OPTIONAL |
				VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_ROAM_SCAN_HI_RSSI_DELAY_DEFAULT,
			     CFG_ROAM_SCAN_HI_RSSI_DELAY_MIN,
			     CFG_ROAM_SCAN_HI_RSSI_DELAY_MAX,
			     cb_notify_set_roam_scan_hi_rssi_scan_params,
			     eCSR_HI_RSSI_SCAN_DELAY_ID),

	REG_DYNAMIC_VARIABLE(CFG_ROAM_SCAN_HI_RSSI_UB_NAME,
			     WLAN_PARAM_SignedInteger,
			     struct hdd_config,
			     nhi_rssi_scan_rssi_ub,
			     VAR_FLAGS_OPTIONAL |
				VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_ROAM_SCAN_HI_RSSI_UB_DEFAULT,
			     CFG_ROAM_SCAN_HI_RSSI_UB_MIN,
			     CFG_ROAM_SCAN_HI_RSSI_UB_MAX,
			     cb_notify_set_roam_scan_hi_rssi_scan_params,
			     eCSR_HI_RSSI_SCAN_RSSI_UB_ID),

	REG_VARIABLE(CFG_QOS_WMM_BURST_SIZE_DEFN_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, burstSizeDefinition,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_WMM_BURST_SIZE_DEFN_DEFAULT,
		     CFG_QOS_WMM_BURST_SIZE_DEFN_MIN,
		     CFG_QOS_WMM_BURST_SIZE_DEFN_MAX),

	REG_VARIABLE(CFG_ENABLE_HOST_ARPOFFLOAD_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, fhostArpOffload,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_HOST_ARPOFFLOAD_DEFAULT,
		     CFG_ENABLE_HOST_ARPOFFLOAD_MIN,
		     CFG_ENABLE_HOST_ARPOFFLOAD_MAX),

	REG_VARIABLE(CFG_HW_FILTER_MODE_BITMAP_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, hw_filter_mode_bitmap,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_HW_FILTER_MODE_BITMAP_DEFAULT,
		     CFG_HW_FILTER_MODE_BITMAP_MIN,
		     CFG_HW_FILTER_MODE_BITMAP_MAX),

#ifdef FEATURE_WLAN_RA_FILTERING
	REG_VARIABLE(CFG_RA_FILTER_ENABLE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, IsRArateLimitEnabled,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_RA_FILTER_ENABLE_DEFAULT,
		     CFG_RA_FILTER_ENABLE_MIN,
		     CFG_RA_FILTER_ENABLE_MAX),

	REG_VARIABLE(CFG_RA_RATE_LIMIT_INTERVAL_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, RArateLimitInterval,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_RA_RATE_LIMIT_INTERVAL_DEFAULT,
		     CFG_RA_RATE_LIMIT_INTERVAL_MIN,
		     CFG_RA_RATE_LIMIT_INTERVAL_MAX),
#endif

	REG_VARIABLE(CFG_IGNORE_PEER_ERP_INFO_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, ignore_peer_erp_info,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_IGNORE_PEER_ERP_INFO_DEFAULT,
		     CFG_IGNORE_PEER_ERP_INFO_MIN,
		     CFG_IGNORE_PEER_ERP_INFO_MAX),

	REG_VARIABLE(CFG_ENABLE_HOST_SSDP_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, ssdp,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_HOST_SSDP_DEFAULT,
		     CFG_ENABLE_HOST_SSDP_MIN,
		     CFG_ENABLE_HOST_SSDP_MAX),

#ifdef FEATURE_RUNTIME_PM
	REG_VARIABLE(CFG_ENABLE_RUNTIME_PM, WLAN_PARAM_Integer,
		     struct hdd_config, runtime_pm,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_RUNTIME_PM_DEFAULT,
		     CFG_ENABLE_RUNTIME_PM_MIN,
		     CFG_ENABLE_RUNTIME_PM_MAX),

	REG_VARIABLE(CFG_RUNTIME_PM_DELAY_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, runtime_pm_delay,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_RUNTIME_PM_DELAY_DEFAULT,
		     CFG_RUNTIME_PM_DELAY_MIN,
		     CFG_RUNTIME_PM_DELAY_MAX),
#endif


	REG_VARIABLE(CFG_ENABLE_HOST_NSOFFLOAD_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, fhostNSOffload,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_HOST_NSOFFLOAD_DEFAULT,
		     CFG_ENABLE_HOST_NSOFFLOAD_MIN,
		     CFG_ENABLE_HOST_NSOFFLOAD_MAX),

	REG_VARIABLE(CFG_QOS_WMM_TS_INFO_ACK_POLICY_NAME, WLAN_PARAM_HexInteger,
		     struct hdd_config, tsInfoAckPolicy,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_WMM_TS_INFO_ACK_POLICY_DEFAULT,
		     CFG_QOS_WMM_TS_INFO_ACK_POLICY_MIN,
		     CFG_QOS_WMM_TS_INFO_ACK_POLICY_MAX),

	REG_VARIABLE(CFG_SINGLE_TID_RC_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, bSingleTidRc,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_SINGLE_TID_RC_DEFAULT,
		     CFG_SINGLE_TID_RC_MIN,
		     CFG_SINGLE_TID_RC_MAX),

	REG_VARIABLE(CFG_TELE_BCN_WAKEUP_EN_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, teleBcnWakeupEn,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TELE_BCN_WAKEUP_EN_DEFAULT,
		     CFG_TELE_BCN_WAKEUP_EN_MIN,
		     CFG_TELE_BCN_WAKEUP_EN_MAX),

	REG_VARIABLE(CFG_INFRA_STA_KEEP_ALIVE_PERIOD_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, infraStaKeepAlivePeriod,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_INFRA_STA_KEEP_ALIVE_PERIOD_DEFAULT,
		     CFG_INFRA_STA_KEEP_ALIVE_PERIOD_MIN,
		     CFG_INFRA_STA_KEEP_ALIVE_PERIOD_MAX),

	REG_VARIABLE(CFG_STA_KEEPALIVE_METHOD_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, sta_keepalive_method,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_STA_KEEPALIVE_METHOD_DEFAULT,
		     CFG_STA_KEEPALIVE_METHOD_MIN,
		     CFG_STA_KEEPALIVE_METHOD_MAX),

	REG_VARIABLE(CFG_QOS_ADDTS_WHEN_ACM_IS_OFF_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, AddTSWhenACMIsOff,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_ADDTS_WHEN_ACM_IS_OFF_DEFAULT,
		     CFG_QOS_ADDTS_WHEN_ACM_IS_OFF_MIN,
		     CFG_QOS_ADDTS_WHEN_ACM_IS_OFF_MAX),

	REG_VARIABLE(CFG_BAND_CAPABILITY_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nBandCapability,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_BAND_CAPABILITY_DEFAULT,
		     CFG_BAND_CAPABILITY_MIN,
		     CFG_BAND_CAPABILITY_MAX),

/* CFG_QDF_TRACE_ENABLE Parameters */
	REG_VARIABLE(CFG_QDF_TRACE_ENABLE_WDI_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, qdf_trace_enable_wdi,
		     VAR_FLAGS_OPTIONAL,
		     CFG_QDF_TRACE_ENABLE_DEFAULT,
		     CFG_QDF_TRACE_ENABLE_MIN,
		     CFG_QDF_TRACE_ENABLE_MAX),

	REG_VARIABLE(CFG_QDF_TRACE_ENABLE_HDD_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, qdf_trace_enable_hdd,
		     VAR_FLAGS_OPTIONAL,
		     CFG_QDF_TRACE_ENABLE_DEFAULT,
		     CFG_QDF_TRACE_ENABLE_MIN,
		     CFG_QDF_TRACE_ENABLE_MAX),

	REG_VARIABLE(CFG_QDF_TRACE_ENABLE_BMI_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, qdf_trace_enable_bmi,
		     VAR_FLAGS_OPTIONAL,
		     CFG_QDF_TRACE_ENABLE_DEFAULT,
		     CFG_QDF_TRACE_ENABLE_MIN,
		     CFG_QDF_TRACE_ENABLE_MAX),

	REG_VARIABLE(CFG_QDF_TRACE_ENABLE_SME_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, qdf_trace_enable_sme,
		     VAR_FLAGS_OPTIONAL,
		     CFG_QDF_TRACE_ENABLE_DEFAULT,
		     CFG_QDF_TRACE_ENABLE_MIN,
		     CFG_QDF_TRACE_ENABLE_MAX),

	REG_VARIABLE(CFG_QDF_TRACE_ENABLE_PE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, qdf_trace_enable_pe,
		     VAR_FLAGS_OPTIONAL,
		     CFG_QDF_TRACE_ENABLE_DEFAULT,
		     CFG_QDF_TRACE_ENABLE_MIN,
		     CFG_QDF_TRACE_ENABLE_MAX),

	REG_VARIABLE(CFG_QDF_TRACE_ENABLE_WMA_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, qdf_trace_enable_wma,
		     VAR_FLAGS_OPTIONAL,
		     CFG_QDF_TRACE_ENABLE_DEFAULT,
		     CFG_QDF_TRACE_ENABLE_MIN,
		     CFG_QDF_TRACE_ENABLE_MAX),

	REG_VARIABLE(CFG_QDF_TRACE_ENABLE_SYS_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, qdf_trace_enable_sys,
		     VAR_FLAGS_OPTIONAL,
		     CFG_QDF_TRACE_ENABLE_DEFAULT,
		     CFG_QDF_TRACE_ENABLE_MIN,
		     CFG_QDF_TRACE_ENABLE_MAX),

	REG_VARIABLE(CFG_QDF_TRACE_ENABLE_QDF_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, qdf_trace_enable_qdf,
		     VAR_FLAGS_OPTIONAL,
		     CFG_QDF_TRACE_ENABLE_DEFAULT,
		     CFG_QDF_TRACE_ENABLE_MIN,
		     CFG_QDF_TRACE_ENABLE_MAX),

	REG_VARIABLE(CFG_QDF_TRACE_ENABLE_SAP_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, qdf_trace_enable_sap,
		     VAR_FLAGS_OPTIONAL,
		     CFG_QDF_TRACE_ENABLE_DEFAULT,
		     CFG_QDF_TRACE_ENABLE_MIN,
		     CFG_QDF_TRACE_ENABLE_MAX),

	REG_VARIABLE(CFG_QDF_TRACE_ENABLE_HDD_SAP_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, qdf_trace_enable_hdd_sap,
		     VAR_FLAGS_OPTIONAL,
		     CFG_QDF_TRACE_ENABLE_DEFAULT,
		     CFG_QDF_TRACE_ENABLE_MIN,
		     CFG_QDF_TRACE_ENABLE_MAX),

	REG_VARIABLE(CFG_QDF_TRACE_ENABLE_CFG_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, qdf_trace_enable_cfg,
		     VAR_FLAGS_OPTIONAL,
		     CFG_QDF_TRACE_ENABLE_DEFAULT,
		     CFG_QDF_TRACE_ENABLE_MIN,
		     CFG_QDF_TRACE_ENABLE_MAX),

	REG_VARIABLE(CFG_QDF_TRACE_ENABLE_TXRX_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, qdf_trace_enable_txrx,
		     VAR_FLAGS_OPTIONAL,
		     CFG_QDF_TRACE_ENABLE_DEFAULT,
		     CFG_QDF_TRACE_ENABLE_MIN,
		     CFG_QDF_TRACE_ENABLE_MAX),

	REG_VARIABLE(CFG_QDF_TRACE_ENABLE_DP_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, qdf_trace_enable_dp,
		     VAR_FLAGS_OPTIONAL,
		     CFG_QDF_TRACE_ENABLE_DP_DEFAULT,
		     CFG_QDF_TRACE_ENABLE_MIN,
		     CFG_QDF_TRACE_ENABLE_MAX),

	REG_VARIABLE(CFG_QDF_TRACE_ENABLE_HTC_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, qdf_trace_enable_htc,
		     VAR_FLAGS_OPTIONAL,
		     CFG_QDF_TRACE_ENABLE_DEFAULT,
		     CFG_QDF_TRACE_ENABLE_MIN,
		     CFG_QDF_TRACE_ENABLE_MAX),

	REG_VARIABLE(CFG_QDF_TRACE_ENABLE_HIF_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, qdf_trace_enable_hif,
		     VAR_FLAGS_OPTIONAL,
		     CFG_QDF_TRACE_ENABLE_DEFAULT,
		     CFG_QDF_TRACE_ENABLE_MIN,
		     CFG_QDF_TRACE_ENABLE_MAX),

	REG_VARIABLE(CFG_CDR_TRACE_ENABLE_HDD_SAP_DATA_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, qdf_trace_enable_hdd_sap_data,
		     VAR_FLAGS_OPTIONAL,
		     CFG_QDF_TRACE_ENABLE_DEFAULT,
		     CFG_QDF_TRACE_ENABLE_MIN,
		     CFG_QDF_TRACE_ENABLE_MAX),

	REG_VARIABLE(CFG_QDF_TRACE_ENABLE_HDD_DATA_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, qdf_trace_enable_hdd_data,
		     VAR_FLAGS_OPTIONAL,
		     CFG_QDF_TRACE_ENABLE_DEFAULT,
		     CFG_QDF_TRACE_ENABLE_MIN,
		     CFG_QDF_TRACE_ENABLE_MAX),

	REG_VARIABLE(CFG_QDF_TRACE_ENABLE_EPPING, WLAN_PARAM_Integer,
		     struct hdd_config, qdf_trace_enable_epping,
		     VAR_FLAGS_OPTIONAL,
		     CFG_QDF_TRACE_ENABLE_DEFAULT,
		     CFG_QDF_TRACE_ENABLE_MIN,
		     CFG_QDF_TRACE_ENABLE_MAX),

	REG_VARIABLE(CFG_QDF_TRACE_ENABLE_QDF_DEVICES, WLAN_PARAM_Integer,
		     struct hdd_config, qdf_trace_enable_qdf_devices,
		     VAR_FLAGS_OPTIONAL,
		     CFG_QDF_TRACE_ENABLE_DEFAULT,
		     CFG_QDF_TRACE_ENABLE_MIN,
		     CFG_QDF_TRACE_ENABLE_MAX),

	REG_VARIABLE(CFG_QDF_TRACE_ENABLE_WIFI_POS, WLAN_PARAM_Integer,
		     struct hdd_config, qdf_trace_enable_wifi_pos,
		     VAR_FLAGS_OPTIONAL,
		     CFG_QDF_TRACE_ENABLE_DEFAULT,
		     CFG_QDF_TRACE_ENABLE_MIN,
		     CFG_QDF_TRACE_ENABLE_MAX),

	REG_VARIABLE(CFG_QDF_TRACE_ENABLE_NAN, WLAN_PARAM_Integer,
		     struct hdd_config, qdf_trace_enable_nan,
		     VAR_FLAGS_OPTIONAL,
		     CFG_QDF_TRACE_ENABLE_DEFAULT,
		     CFG_QDF_TRACE_ENABLE_MIN,
		     CFG_QDF_TRACE_ENABLE_MAX),

	REG_VARIABLE(CFG_QDF_TRACE_ENABLE_REGULATORY, WLAN_PARAM_Integer,
		     struct hdd_config, qdf_trace_enable_regulatory,
		     VAR_FLAGS_OPTIONAL,
		     CFG_QDF_TRACE_ENABLE_DEFAULT,
		     CFG_QDF_TRACE_ENABLE_MIN,
		     CFG_QDF_TRACE_ENABLE_MAX),

	REG_VARIABLE(CFG_QDF_TRACE_ENABLE_CP_STATS, WLAN_PARAM_Integer,
		     struct hdd_config, qdf_trace_enable_cp_stats,
		     VAR_FLAGS_OPTIONAL,
		     CFG_QDF_TRACE_ENABLE_DEFAULT,
		     CFG_QDF_TRACE_ENABLE_MIN,
		     CFG_QDF_TRACE_ENABLE_MAX),

#ifdef ENABLE_MTRACE_LOG
	REG_VARIABLE(CFG_ENABLE_MTRACE, WLAN_PARAM_Integer,
		     struct hdd_config, enable_mtrace,
		     VAR_FLAGS_OPTIONAL,
		     CFG_ENABLE_MTRACE_DEFAULT,
		     CFG_ENABLE_MTRACE_MIN,
		     CFG_ENABLE_MTRACE_MAX),
#endif

	REG_VARIABLE(CFG_TELE_BCN_MAX_LI_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nTeleBcnMaxListenInterval,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TELE_BCN_MAX_LI_DEFAULT,
		     CFG_TELE_BCN_MAX_LI_MIN,
		     CFG_TELE_BCN_MAX_LI_MAX),

	REG_VARIABLE(CFG_ENABLE_BYPASS_11D_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, enableBypass11d,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_BYPASS_11D_DEFAULT,
		     CFG_ENABLE_BYPASS_11D_MIN,
		     CFG_ENABLE_BYPASS_11D_MAX),

	REG_VARIABLE(CFG_ENABLE_DFS_CHNL_SCAN_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, enableDFSChnlScan,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_DFS_CHNL_SCAN_DEFAULT,
		     CFG_ENABLE_DFS_CHNL_SCAN_MIN,
		     CFG_ENABLE_DFS_CHNL_SCAN_MAX),

	REG_VARIABLE(CFG_ENABLE_DFS_PNO_CHNL_SCAN_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, enable_dfs_pno_chnl_scan,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_DFS_PNO_CHNL_SCAN_DEFAULT,
		     CFG_ENABLE_DFS_PNO_CHNL_SCAN_MIN,
		     CFG_ENABLE_DFS_PNO_CHNL_SCAN_MAX),

	REG_VARIABLE(CFG_ENABLE_DYNAMIC_DTIM_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, enableDynamicDTIM,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_DYNAMIC_DTIM_DEFAULT,
		     CFG_ENABLE_DYNAMIC_DTIM_MIN,
		     CFG_ENABLE_DYNAMIC_DTIM_MAX),

	REG_VARIABLE(CFG_SHORT_GI_40MHZ_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, ShortGI40MhzEnable,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_SHORT_GI_40MHZ_DEFAULT,
		     CFG_SHORT_GI_40MHZ_MIN,
		     CFG_SHORT_GI_40MHZ_MAX),

	REG_DYNAMIC_VARIABLE(CFG_REPORT_MAX_LINK_SPEED, WLAN_PARAM_Integer,
			     struct hdd_config, reportMaxLinkSpeed,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_REPORT_MAX_LINK_SPEED_DEFAULT,
			     CFG_REPORT_MAX_LINK_SPEED_MIN,
			     CFG_REPORT_MAX_LINK_SPEED_MAX,
			     NULL, 0),

	REG_DYNAMIC_VARIABLE(CFG_LINK_SPEED_RSSI_HIGH, WLAN_PARAM_SignedInteger,
			     struct hdd_config, linkSpeedRssiHigh,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_LINK_SPEED_RSSI_HIGH_DEFAULT,
			     CFG_LINK_SPEED_RSSI_HIGH_MIN,
			     CFG_LINK_SPEED_RSSI_HIGH_MAX,
			     NULL, 0),

	REG_DYNAMIC_VARIABLE(CFG_LINK_SPEED_RSSI_MID, WLAN_PARAM_SignedInteger,
			     struct hdd_config, linkSpeedRssiMid,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_LINK_SPEED_RSSI_MID_DEFAULT,
			     CFG_LINK_SPEED_RSSI_MID_MIN,
			     CFG_LINK_SPEED_RSSI_MID_MAX,
			     NULL, 0),

	REG_DYNAMIC_VARIABLE(CFG_LINK_SPEED_RSSI_LOW, WLAN_PARAM_SignedInteger,
			     struct hdd_config, linkSpeedRssiLow,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_LINK_SPEED_RSSI_LOW_DEFAULT,
			     CFG_LINK_SPEED_RSSI_LOW_MIN,
			     CFG_LINK_SPEED_RSSI_LOW_MAX,
			     NULL, 0),

	REG_DYNAMIC_VARIABLE(CFG_ROAM_PREFER_5GHZ, WLAN_PARAM_Integer,
			     struct hdd_config, nRoamPrefer5GHz,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_ROAM_PREFER_5GHZ_DEFAULT,
			     CFG_ROAM_PREFER_5GHZ_MIN,
			     CFG_ROAM_PREFER_5GHZ_MAX,
			     cb_notify_set_roam_prefer5_g_hz, 0),

	REG_DYNAMIC_VARIABLE(CFG_ROAM_INTRA_BAND, WLAN_PARAM_Integer,
			     struct hdd_config, nRoamIntraBand,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_ROAM_INTRA_BAND_DEFAULT,
			     CFG_ROAM_INTRA_BAND_MIN,
			     CFG_ROAM_INTRA_BAND_MAX,
			     cb_notify_set_roam_intra_band, 0),

	REG_DYNAMIC_VARIABLE(CFG_ROAM_SCAN_N_PROBES, WLAN_PARAM_Integer,
			     struct hdd_config, nProbes,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_ROAM_SCAN_N_PROBES_DEFAULT,
			     CFG_ROAM_SCAN_N_PROBES_MIN,
			     CFG_ROAM_SCAN_N_PROBES_MAX,
			     cb_notify_set_roam_scan_n_probes, 0),

	REG_DYNAMIC_VARIABLE(CFG_ROAM_SCAN_HOME_AWAY_TIME, WLAN_PARAM_Integer,
			     struct hdd_config, nRoamScanHomeAwayTime,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_ROAM_SCAN_HOME_AWAY_TIME_DEFAULT,
			     CFG_ROAM_SCAN_HOME_AWAY_TIME_MIN,
			     CFG_ROAM_SCAN_HOME_AWAY_TIME_MAX,
			     cb_notify_set_roam_scan_home_away_time, 0),

	REG_VARIABLE(CFG_ENABLE_MCC_ENABLED_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, enableMCC,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_MCC_ENABLED_DEFAULT,
		     CFG_ENABLE_MCC_ENABLED_MIN,
		     CFG_ENABLE_MCC_ENABLED_MAX),

	REG_VARIABLE(CFG_ALLOW_MCC_GO_DIFF_BI_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, allowMCCGODiffBI,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ALLOW_MCC_GO_DIFF_BI_DEFAULT,
		     CFG_ALLOW_MCC_GO_DIFF_BI_MIN,
		     CFG_ALLOW_MCC_GO_DIFF_BI_MAX),

	REG_VARIABLE(CFG_THERMAL_MIGRATION_ENABLE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, thermalMitigationEnable,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_THERMAL_MIGRATION_ENABLE_DEFAULT,
		     CFG_THERMAL_MIGRATION_ENABLE_MIN,
		     CFG_THERMAL_MIGRATION_ENABLE_MAX),

	REG_VARIABLE(CFG_THROTTLE_PERIOD_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, throttlePeriod,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_THROTTLE_PERIOD_DEFAULT,
		     CFG_THROTTLE_PERIOD_MIN,
		     CFG_THROTTLE_PERIOD_MAX),

	REG_VARIABLE(CFG_THROTTLE_DUTY_CYCLE_LEVEL0_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, throttle_dutycycle_level0,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_THROTTLE_DUTY_CYCLE_LEVEL0_DEFAULT,
		     CFG_THROTTLE_DUTY_CYCLE_LEVEL0_MIN,
		     CFG_THROTTLE_DUTY_CYCLE_LEVEL0_MAX),

	REG_VARIABLE(CFG_THROTTLE_DUTY_CYCLE_LEVEL1_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, throttle_dutycycle_level1,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_THROTTLE_DUTY_CYCLE_LEVEL1_DEFAULT,
		     CFG_THROTTLE_DUTY_CYCLE_LEVEL1_MIN,
		     CFG_THROTTLE_DUTY_CYCLE_LEVEL1_MAX),

	REG_VARIABLE(CFG_THROTTLE_DUTY_CYCLE_LEVEL2_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, throttle_dutycycle_level2,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_THROTTLE_DUTY_CYCLE_LEVEL2_DEFAULT,
		     CFG_THROTTLE_DUTY_CYCLE_LEVEL2_MIN,
		     CFG_THROTTLE_DUTY_CYCLE_LEVEL2_MAX),

	REG_VARIABLE(CFG_THROTTLE_DUTY_CYCLE_LEVEL3_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, throttle_dutycycle_level3,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_THROTTLE_DUTY_CYCLE_LEVEL3_DEFAULT,
		     CFG_THROTTLE_DUTY_CYCLE_LEVEL3_MIN,
		     CFG_THROTTLE_DUTY_CYCLE_LEVEL3_MAX),

	REG_VARIABLE(CFG_ENABLE_MODULATED_DTIM_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, enableModulatedDTIM,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_MODULATED_DTIM_DEFAULT,
		     CFG_ENABLE_MODULATED_DTIM_MIN,
		     CFG_ENABLE_MODULATED_DTIM_MAX),

	REG_VARIABLE(CFG_MC_ADDR_LIST_ENABLE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, fEnableMCAddrList,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_MC_ADDR_LIST_ENABLE_DEFAULT,
		     CFG_MC_ADDR_LIST_ENABLE_MIN,
		     CFG_MC_ADDR_LIST_ENABLE_MAX),

	REG_VARIABLE(CFG_VHT_CHANNEL_WIDTH, WLAN_PARAM_Integer,
		     struct hdd_config, vhtChannelWidth,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_VHT_CHANNEL_WIDTH_DEFAULT,
		     CFG_VHT_CHANNEL_WIDTH_MIN,
		     CFG_VHT_CHANNEL_WIDTH_MAX),

	REG_VARIABLE(CFG_VHT_ENABLE_RX_MCS_8_9, WLAN_PARAM_Integer,
		     struct hdd_config, vhtRxMCS,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_VHT_ENABLE_RX_MCS_8_9_DEFAULT,
		     CFG_VHT_ENABLE_RX_MCS_8_9_MIN,
		     CFG_VHT_ENABLE_RX_MCS_8_9_MAX),

	REG_VARIABLE(CFG_VHT_ENABLE_TX_MCS_8_9, WLAN_PARAM_Integer,
		     struct hdd_config, vhtTxMCS,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_VHT_ENABLE_TX_MCS_8_9_DEFAULT,
		     CFG_VHT_ENABLE_TX_MCS_8_9_MIN,
		     CFG_VHT_ENABLE_TX_MCS_8_9_MAX),

	REG_VARIABLE(CFG_VHT_ENABLE_RX_MCS2x2_8_9, WLAN_PARAM_Integer,
		     struct hdd_config, vhtRxMCS2x2,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_VHT_ENABLE_RX_MCS2x2_8_9_DEFAULT,
		     CFG_VHT_ENABLE_RX_MCS2x2_8_9_MIN,
		     CFG_VHT_ENABLE_RX_MCS2x2_8_9_MAX),

	REG_VARIABLE(CFG_VHT_ENABLE_TX_MCS2x2_8_9, WLAN_PARAM_Integer,
		     struct hdd_config, vhtTxMCS2x2,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_VHT_ENABLE_TX_MCS2x2_8_9_DEFAULT,
		     CFG_VHT_ENABLE_TX_MCS2x2_8_9_MIN,
		     CFG_VHT_ENABLE_TX_MCS2x2_8_9_MAX),

	REG_VARIABLE(CFG_ENABLE_VHT20_MCS9, WLAN_PARAM_Integer,
		     struct hdd_config, enable_vht20_mcs9,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_ENABLE_VHT20_MCS9_DEFAULT,
		     CFG_ENABLE_VHT20_MCS9_MIN,
		     CFG_ENABLE_VHT20_MCS9_MAX),

	REG_VARIABLE(CFG_VHT_ENABLE_2x2_CAP_FEATURE, WLAN_PARAM_Integer,
		     struct hdd_config, enable2x2,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_VHT_ENABLE_2x2_CAP_FEATURE_DEFAULT,
		     CFG_VHT_ENABLE_2x2_CAP_FEATURE_MIN,
		     CFG_VHT_ENABLE_2x2_CAP_FEATURE_MAX),

	REG_VARIABLE(CFG_VDEV_TYPE_NSS_2G, WLAN_PARAM_Integer,
		     struct hdd_config, vdev_type_nss_2g,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_VDEV_TYPE_NSS_2G_DEFAULT,
		     CFG_VDEV_TYPE_NSS_2G_MIN,
		     CFG_VDEV_TYPE_NSS_2G_MAX),

	REG_VARIABLE(CFG_ENABLE_BT_CHAIN_SEPARATION, WLAN_PARAM_Integer,
		     struct hdd_config, enable_bt_chain_separation,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_BT_CHAIN_SEPARATION_DEFAULT,
		     CFG_ENABLE_BT_CHAIN_SEPARATION_MIN,
		     CFG_ENABLE_BT_CHAIN_SEPARATION_MAX),

	REG_VARIABLE(CFG_STA_PREFER_80MHZ_OVER_160MHZ, WLAN_PARAM_Integer,
		     struct hdd_config, sta_prefer_80MHz_over_160MHz,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_STA_PREFER_80MHZ_OVER_160MHZ_DEFAULT,
		     CFG_STA_PREFER_80MHZ_OVER_160MHZ_MIN,
		     CFG_STA_PREFER_80MHZ_OVER_160MHZ_MAX),

	REG_VARIABLE(CFG_VDEV_TYPE_NSS_5G, WLAN_PARAM_Integer,
		     struct hdd_config, vdev_type_nss_5g,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_VDEV_TYPE_NSS_5G_DEFAULT,
		     CFG_VDEV_TYPE_NSS_5G_MIN,
		     CFG_VDEV_TYPE_NSS_5G_MAX),

	REG_VARIABLE(CFG_VHT_ENABLE_MU_BFORMEE_CAP_FEATURE, WLAN_PARAM_Integer,
		     struct hdd_config, enableMuBformee,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_VHT_ENABLE_MU_BFORMEE_CAP_FEATURE_DEFAULT,
		     CFG_VHT_ENABLE_MU_BFORMEE_CAP_FEATURE_MIN,
		     CFG_VHT_ENABLE_MU_BFORMEE_CAP_FEATURE_MAX),

	REG_VARIABLE(CFG_VHT_ENABLE_PAID_FEATURE, WLAN_PARAM_Integer,
		     struct hdd_config, enableVhtpAid,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_VHT_ENABLE_PAID_FEATURE_DEFAULT,
		     CFG_VHT_ENABLE_PAID_FEATURE_MIN,
		     CFG_VHT_ENABLE_PAID_FEATURE_MAX),

	REG_VARIABLE(CFG_VHT_ENABLE_GID_FEATURE, WLAN_PARAM_Integer,
		     struct hdd_config, enableVhtGid,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_VHT_ENABLE_GID_FEATURE_DEFAULT,
		     CFG_VHT_ENABLE_GID_FEATURE_MIN,
		     CFG_VHT_ENABLE_GID_FEATURE_MAX),

	REG_VARIABLE(CFG_ENABLE_AMPDUPS_FEATURE, WLAN_PARAM_Integer,
		     struct hdd_config, enableAmpduPs,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_AMPDUPS_FEATURE_DEFAULT,
		     CFG_ENABLE_AMPDUPS_FEATURE_MIN,
		     CFG_ENABLE_AMPDUPS_FEATURE_MAX),

	REG_VARIABLE(CFG_HT_ENABLE_SMPS_CAP_FEATURE, WLAN_PARAM_Integer,
		     struct hdd_config, enableHtSmps,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_HT_ENABLE_SMPS_CAP_FEATURE_DEFAULT,
		     CFG_HT_ENABLE_SMPS_CAP_FEATURE_MIN,
		     CFG_HT_ENABLE_SMPS_CAP_FEATURE_MAX),

	REG_VARIABLE(CFG_HT_SMPS_CAP_FEATURE, WLAN_PARAM_Integer,
		     struct hdd_config, htSmps,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_HT_SMPS_CAP_FEATURE_DEFAULT,
		     CFG_HT_SMPS_CAP_FEATURE_MIN,
		     CFG_HT_SMPS_CAP_FEATURE_MAX),

	REG_VARIABLE(CFG_DISABLE_DFS_CH_SWITCH, WLAN_PARAM_Integer,
		     struct hdd_config, disableDFSChSwitch,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_DISABLE_DFS_CH_SWITCH_DEFAULT,
		     CFG_DISABLE_DFS_CH_SWITCH_MIN,
		     CFG_DISABLE_DFS_CH_SWITCH_MAX),

	REG_VARIABLE(CFG_ENABLE_DFS_MASTER_CAPABILITY, WLAN_PARAM_Integer,
		     struct hdd_config, enableDFSMasterCap,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_DFS_MASTER_CAPABILITY_DEFAULT,
		     CFG_ENABLE_DFS_MASTER_CAPABILITY_MIN,
		     CFG_ENABLE_DFS_MASTER_CAPABILITY_MAX),

	REG_DYNAMIC_VARIABLE(CFG_SAP_PREFERRED_CHANNEL_LOCATION,
			     WLAN_PARAM_Integer,
			     struct hdd_config, gSapPreferredChanLocation,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_SAP_PREFERRED_CHANNEL_LOCATION_DEFAULT,
			     CFG_SAP_PREFERRED_CHANNEL_LOCATION_MIN,
			     CFG_SAP_PREFERRED_CHANNEL_LOCATION_MAX,
			     cb_notify_set_g_sap_preferred_chan_location, 0),
	REG_DYNAMIC_VARIABLE(CFG_DISABLE_DFS_JAPAN_W53, WLAN_PARAM_Integer,
			     struct hdd_config, gDisableDfsJapanW53,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_DISABLE_DFS_JAPAN_W53_DEFAULT,
			     CFG_DISABLE_DFS_JAPAN_W53_MIN,
			     CFG_DISABLE_DFS_JAPAN_W53_MAX,
			     ch_notify_set_g_disable_dfs_japan_w53, 0),

	REG_VARIABLE(CFG_SAP_GET_PEER_INFO, WLAN_PARAM_Integer,
		     struct hdd_config, sap_get_peer_info,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_SAP_GET_PEER_INFO_DEFAULT,
		     CFG_SAP_GET_PEER_INFO_MIN,
		     CFG_SAP_GET_PEER_INFO_MAX),

	REG_VARIABLE(CFG_RATE_FOR_TX_MGMT, WLAN_PARAM_HexInteger,
		     struct hdd_config, rate_for_tx_mgmt,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_RATE_FOR_TX_MGMT_DEFAULT,
		     CFG_RATE_FOR_TX_MGMT_MIN,
		     CFG_RATE_FOR_TX_MGMT_MAX),

	REG_VARIABLE(CFG_RATE_FOR_TX_MGMT_2G, WLAN_PARAM_HexInteger,
		     struct hdd_config, rate_for_tx_mgmt_2g,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_RATE_FOR_TX_MGMT_2G_DEFAULT,
		     CFG_RATE_FOR_TX_MGMT_2G_MIN,
		     CFG_RATE_FOR_TX_MGMT_2G_MAX),

	REG_VARIABLE(CFG_RATE_FOR_TX_MGMT_5G, WLAN_PARAM_HexInteger,
		     struct hdd_config, rate_for_tx_mgmt_5g,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_RATE_FOR_TX_MGMT_5G_DEFAULT,
		     CFG_RATE_FOR_TX_MGMT_5G_MIN,
		     CFG_RATE_FOR_TX_MGMT_5G_MAX),

	REG_VARIABLE(CFG_ENABLE_FIRST_SCAN_2G_ONLY_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, enableFirstScan2GOnly,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_FIRST_SCAN_2G_ONLY_DEFAULT,
		     CFG_ENABLE_FIRST_SCAN_2G_ONLY_MIN,
		     CFG_ENABLE_FIRST_SCAN_2G_ONLY_MAX),

	REG_VARIABLE(CFG_ENABLE_RX_STBC, WLAN_PARAM_Integer,
		     struct hdd_config, enableRxSTBC,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_RX_STBC_DEFAULT,
		     CFG_ENABLE_RX_STBC_MIN,
		     CFG_ENABLE_RX_STBC_MAX),

	REG_VARIABLE(CFG_ENABLE_TX_STBC, WLAN_PARAM_Integer,
		     struct hdd_config, enableTxSTBC,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_TX_STBC_DEFAULT,
		     CFG_ENABLE_TX_STBC_MIN,
		     CFG_ENABLE_TX_STBC_MAX),

	REG_VARIABLE(CFG_PPS_ENABLE_5G_EBT, WLAN_PARAM_Integer,
		     struct hdd_config, enable5gEBT,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_PPS_ENABLE_5G_EBT_FEATURE_DEFAULT,
		     CFG_PPS_ENABLE_5G_EBT_FEATURE_MIN,
		     CFG_PPS_ENABLE_5G_EBT_FEATURE_MAX),

	REG_VARIABLE(CFG_PREVENT_LINK_DOWN_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, prevent_link_down,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_PREVENT_LINK_DOWN_DEFAULT,
		     CFG_PREVENT_LINK_DOWN_MIN,
		     CFG_PREVENT_LINK_DOWN_MAX),

	REG_VARIABLE(CFG_SCAN_AGING_PARAM_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, scanAgingTimeout,
		     VAR_FLAGS_OPTIONAL,
		     CFG_SCAN_AGING_PARAM_DEFAULT,
		     CFG_SCAN_AGING_PARAM_MIN,
		     CFG_SCAN_AGING_PARAM_MAX),

	REG_VARIABLE(CFG_TX_LDPC_ENABLE_FEATURE, WLAN_PARAM_Integer,
		     struct hdd_config, enable_tx_ldpc,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TX_LDPC_ENABLE_FEATURE_DEFAULT,
		     CFG_TX_LDPC_ENABLE_FEATURE_MIN,
		     CFG_TX_LDPC_ENABLE_FEATURE_MAX),

	REG_VARIABLE(CFG_ENABLE_RX_LDPC, WLAN_PARAM_Integer,
		     struct hdd_config, enable_rx_ldpc,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_RX_LDPC_DEFAULT,
		     CFG_ENABLE_RX_LDPC_MIN,
		     CFG_ENABLE_RX_LDPC_MAX),

	REG_VARIABLE(CFG_ENABLE_MCC_ADATIVE_SCHEDULER_ENABLED_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, enableMCCAdaptiveScheduler,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_MCC_ADATIVE_SCHEDULER_ENABLED_DEFAULT,
		     CFG_ENABLE_MCC_ADATIVE_SCHEDULER_ENABLED_MIN,
		     CFG_ENABLE_MCC_ADATIVE_SCHEDULER_ENABLED_MAX),

	REG_VARIABLE(CFG_IBSS_ADHOC_CHANNEL_5GHZ_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, AdHocChannel5G,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_IBSS_ADHOC_CHANNEL_5GHZ_DEFAULT,
		     CFG_IBSS_ADHOC_CHANNEL_5GHZ_MIN,
		     CFG_IBSS_ADHOC_CHANNEL_5GHZ_MAX),

	REG_VARIABLE(CFG_IBSS_ADHOC_CHANNEL_24GHZ_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, AdHocChannel24G,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_IBSS_ADHOC_CHANNEL_24GHZ_DEFAULT,
		     CFG_IBSS_ADHOC_CHANNEL_24GHZ_MIN,
		     CFG_IBSS_ADHOC_CHANNEL_24GHZ_MAX),

	REG_VARIABLE(CFG_VHT_SU_BEAMFORMEE_CAP_FEATURE, WLAN_PARAM_Integer,
		     struct hdd_config, enableTxBF,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_VHT_SU_BEAMFORMEE_CAP_FEATURE_DEFAULT,
		     CFG_VHT_SU_BEAMFORMEE_CAP_FEATURE_MIN,
		     CFG_VHT_SU_BEAMFORMEE_CAP_FEATURE_MAX),

	REG_VARIABLE(CFG_ENABLE_SUBFEE_IN_VENDOR_VHTIE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, enable_subfee_vendor_vhtie,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_SUBFEE_IN_VENDOR_VHTIE_DEFAULT,
		     CFG_ENABLE_SUBFEE_IN_VENDOR_VHTIE_MIN,
		     CFG_ENABLE_SUBFEE_IN_VENDOR_VHTIE_MAX),

	REG_VARIABLE(CFG_VHT_ENABLE_TXBF_SAP_MODE, WLAN_PARAM_Integer,
		     struct hdd_config, enable_txbf_sap_mode,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_VHT_ENABLE_TXBF_SAP_MODE_DEFAULT,
		     CFG_VHT_ENABLE_TXBF_SAP_MODE_MIN,
		     CFG_VHT_ENABLE_TXBF_SAP_MODE_MAX),

	REG_VARIABLE(CFG_VHT_ENABLE_TXBF_IN_20MHZ, WLAN_PARAM_Integer,
		     struct hdd_config, enableTxBFin20MHz,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_VHT_ENABLE_TXBF_IN_20MHZ_DEFAULT,
		     CFG_VHT_ENABLE_TXBF_IN_20MHZ_MIN,
		     CFG_VHT_ENABLE_TXBF_IN_20MHZ_MAX),

	REG_VARIABLE(CFG_VHT_CSN_BEAMFORMEE_ANT_SUPPORTED, WLAN_PARAM_Integer,
		     struct hdd_config, txBFCsnValue,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_VHT_CSN_BEAMFORMEE_ANT_SUPPORTED_DEFAULT,
		     CFG_VHT_CSN_BEAMFORMEE_ANT_SUPPORTED_MIN,
		     CFG_VHT_CSN_BEAMFORMEE_ANT_SUPPORTED_MAX),

	REG_VARIABLE(CFG_VHT_ENABLE_TX_SU_BEAM_FORMER, WLAN_PARAM_Integer,
		     struct hdd_config, enable_su_tx_bformer,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_VHT_ENABLE_TX_SU_BEAM_FORMER_DEFAULT,
		     CFG_VHT_ENABLE_TX_SU_BEAM_FORMER_MIN,
		     CFG_VHT_ENABLE_TX_SU_BEAM_FORMER_MAX),

	REG_VARIABLE(CFG_SAP_ALLOW_ALL_CHANNEL_PARAM_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, sapAllowAllChannel,
		     VAR_FLAGS_OPTIONAL,
		     CFG_SAP_ALLOW_ALL_CHANNEL_PARAM_DEFAULT,
		     CFG_SAP_ALLOW_ALL_CHANNEL_PARAM_MIN,
		     CFG_SAP_ALLOW_ALL_CHANNEL_PARAM_MAX),

	REG_VARIABLE(CFG_DISABLE_LDPC_WITH_TXBF_AP, WLAN_PARAM_Integer,
		     struct hdd_config, disableLDPCWithTxbfAP,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_DISABLE_LDPC_WITH_TXBF_AP_DEFAULT,
		     CFG_DISABLE_LDPC_WITH_TXBF_AP_MIN,
		     CFG_DISABLE_LDPC_WITH_TXBF_AP_MAX),

	REG_DYNAMIC_VARIABLE(CFG_ENABLE_SSR, WLAN_PARAM_Integer,
			     struct hdd_config, enableSSR,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_ENABLE_SSR_DEFAULT,
			     CFG_ENABLE_SSR_MIN,
			     CFG_ENABLE_SSR_MAX,
			     cb_notify_set_enable_ssr, 0),

	REG_VARIABLE(CFG_ENABLE_DATA_STALL_DETECTION, WLAN_PARAM_Integer,
		     struct hdd_config, enable_data_stall_det,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_DATA_STALL_DETECTION_DEFAULT,
		     CFG_ENABLE_DATA_STALL_DETECTION_MIN,
		     CFG_ENABLE_DATA_STALL_DETECTION_MAX),

	REG_VARIABLE(CFG_ENABLE_VHT_FOR_24GHZ_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, enableVhtFor24GHzBand,
		     VAR_FLAGS_OPTIONAL,
		     CFG_ENABLE_VHT_FOR_24GHZ_DEFAULT,
		     CFG_ENABLE_VHT_FOR_24GHZ_MIN,
		     CFG_ENABLE_VHT_FOR_24GHZ_MAX),


	REG_VARIABLE(CFG_ENABLE_VENDOR_VHT_FOR_24GHZ_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, enable_sap_vendor_vht,
		     VAR_FLAGS_OPTIONAL,
		     CFG_ENABLE_VENDOR_VHT_FOR_24GHZ_DEFAULT,
		     CFG_ENABLE_VENDOR_VHT_FOR_24GHZ_MIN,
		     CFG_ENABLE_VENDOR_VHT_FOR_24GHZ_MAX),

	REG_DYNAMIC_VARIABLE(CFG_ENABLE_FAST_ROAM_IN_CONCURRENCY,
			     WLAN_PARAM_Integer,
			     struct hdd_config, bFastRoamInConIniFeatureEnabled,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_ENABLE_FAST_ROAM_IN_CONCURRENCY_DEFAULT,
			     CFG_ENABLE_FAST_ROAM_IN_CONCURRENCY_MIN,
			     CFG_ENABLE_FAST_ROAM_IN_CONCURRENCY_MAX,
			     cb_notify_set_enable_fast_roam_in_concurrency, 0),

	REG_VARIABLE(CFG_ENABLE_SNR_MONITORING_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, fEnableSNRMonitoring,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_ENABLE_SNR_MONITORING_DEFAULT,
		     CFG_ENABLE_SNR_MONITORING_MIN,
		     CFG_ENABLE_SNR_MONITORING_MAX),

#ifdef FEATURE_WLAN_SCAN_PNO
	REG_VARIABLE(CFG_PNO_SCAN_SUPPORT, WLAN_PARAM_Integer,
		     struct hdd_config, configPNOScanSupport,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_PNO_SCAN_SUPPORT_DEFAULT,
		     CFG_PNO_SCAN_SUPPORT_DISABLE,
		     CFG_PNO_SCAN_SUPPORT_ENABLE),

	REG_VARIABLE(CFG_PNO_SCAN_TIMER_REPEAT_VALUE, WLAN_PARAM_Integer,
		     struct hdd_config, configPNOScanTimerRepeatValue,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_PNO_SCAN_TIMER_REPEAT_VALUE_DEFAULT,
		     CFG_PNO_SCAN_TIMER_REPEAT_VALUE_MIN,
		     CFG_PNO_SCAN_TIMER_REPEAT_VALUE_MAX),

	REG_VARIABLE(CFG_PNO_SLOW_SCAN_MULTIPLIER, WLAN_PARAM_Integer,
		     struct hdd_config, pno_slow_scan_multiplier,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_PNO_SLOW_SCAN_MULTIPLIER_DEFAULT,
		     CFG_PNO_SLOW_SCAN_MULTIPLIER_MIN,
		     CFG_PNO_SLOW_SCAN_MULTIPLIER_MAX),
#endif
	REG_VARIABLE(CFG_MAX_AMSDU_NUM_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, max_amsdu_num,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_MAX_AMSDU_NUM_DEFAULT,
		     CFG_MAX_AMSDU_NUM_MIN,
		     CFG_MAX_AMSDU_NUM_MAX),

	REG_VARIABLE(CFG_STRICT_5GHZ_PREF_BY_MARGIN, WLAN_PARAM_Integer,
		     struct hdd_config, nSelect5GHzMargin,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_STRICT_5GHZ_PREF_BY_MARGIN_DEFAULT,
		     CFG_STRICT_5GHZ_PREF_BY_MARGIN_MIN,
		     CFG_STRICT_5GHZ_PREF_BY_MARGIN_MAX),

	REG_VARIABLE(CFG_ENABLE_IP_TCP_UDP_CHKSUM_OFFLOAD, WLAN_PARAM_Integer,
		     struct hdd_config, enable_ip_tcp_udp_checksum_offload,
		     VAR_FLAGS_OPTIONAL,
		     CFG_ENABLE_IP_TCP_UDP_CHKSUM_OFFLOAD_DEFAULT,
		     CFG_ENABLE_IP_TCP_UDP_CHKSUM_OFFLOAD_DISABLE,
		     CFG_ENABLE_IP_TCP_UDP_CHKSUM_OFFLOAD_ENABLE),

	REG_VARIABLE(CFG_POWERSAVE_OFFLOAD_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, enablePowersaveOffload,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_POWERSAVE_OFFLOAD_DEFAULT,
		     CFG_POWERSAVE_OFFLOAD_MIN,
		     CFG_POWERSAVE_OFFLOAD_MAX),

	REG_VARIABLE(CFG_ENABLE_FW_UART_PRINT_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, enablefwprint,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_FW_UART_PRINT_DEFAULT,
		     CFG_ENABLE_FW_UART_PRINT_DISABLE,
		     CFG_ENABLE_FW_UART_PRINT_ENABLE),

	REG_VARIABLE(CFG_ENABLE_FW_LOG_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, enable_fw_log,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_FW_LOG_DEFAULT,
		     CFG_ENABLE_FW_LOG_DISABLE,
		     CFG_ENABLE_FW_LOG_MAX),

#ifdef IPA_OFFLOAD
	REG_VARIABLE(CFG_IPA_OFFLOAD_CONFIG_NAME, WLAN_PARAM_HexInteger,
		     struct hdd_config, IpaConfig,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_IPA_OFFLOAD_CONFIG_DEFAULT,
		     CFG_IPA_OFFLOAD_CONFIG_MIN,
		     CFG_IPA_OFFLOAD_CONFIG_MAX),

	REG_VARIABLE(CFG_IPA_DESC_SIZE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, IpaDescSize,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_IPA_DESC_SIZE_DEFAULT,
		     CFG_IPA_DESC_SIZE_MIN,
		     CFG_IPA_DESC_SIZE_MAX),

	REG_VARIABLE(CFG_IPA_HIGH_BANDWIDTH_MBPS, WLAN_PARAM_Integer,
		     struct hdd_config, IpaHighBandwidthMbps,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_IPA_HIGH_BANDWIDTH_MBPS_DEFAULT,
		     CFG_IPA_HIGH_BANDWIDTH_MBPS_MIN,
		     CFG_IPA_HIGH_BANDWIDTH_MBPS_MAX),

	REG_VARIABLE(CFG_IPA_MEDIUM_BANDWIDTH_MBPS, WLAN_PARAM_Integer,
		     struct hdd_config, IpaMediumBandwidthMbps,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_IPA_MEDIUM_BANDWIDTH_MBPS_DEFAULT,
		     CFG_IPA_MEDIUM_BANDWIDTH_MBPS_MIN,
		     CFG_IPA_MEDIUM_BANDWIDTH_MBPS_MAX),

	REG_VARIABLE(CFG_IPA_LOW_BANDWIDTH_MBPS, WLAN_PARAM_Integer,
		     struct hdd_config, IpaLowBandwidthMbps,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_IPA_LOW_BANDWIDTH_MBPS_DEFAULT,
		     CFG_IPA_LOW_BANDWIDTH_MBPS_MIN,
		     CFG_IPA_LOW_BANDWIDTH_MBPS_MAX),
#endif

	REG_VARIABLE(CFG_VHT_AMPDU_LEN_EXPONENT_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, fVhtAmpduLenExponent,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_VHT_AMPDU_LEN_EXPONENT_DEFAULT,
		     CFG_VHT_AMPDU_LEN_EXPONENT_MIN,
		     CFG_VHT_AMPDU_LEN_EXPONENT_MAX),

	REG_VARIABLE(CFG_VHT_MPDU_LEN_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, vhtMpduLen,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_VHT_MPDU_LEN_DEFAULT,
		     CFG_VHT_MPDU_LEN_MIN,
		     CFG_VHT_MPDU_LEN_MAX),

	REG_VARIABLE(CFG_WOW_STATUS_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, wowEnable,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_WOW_STATUS_DEFAULT,
		     CFG_WOW_ENABLE_MIN,
		     CFG_WOW_ENABLE_MAX),

	REG_VARIABLE(CFG_COALESING_IN_IBSS_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, isCoalesingInIBSSAllowed,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_COALESING_IN_IBSS_DEFAULT,
		     CFG_COALESING_IN_IBSS_MIN,
		     CFG_COALESING_IN_IBSS_MAX),

	REG_VARIABLE(CFG_IBSS_ATIM_WIN_SIZE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, ibssATIMWinSize,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_IBSS_ATIM_WIN_SIZE_DEFAULT,
		     CFG_IBSS_ATIM_WIN_SIZE_MIN,
		     CFG_IBSS_ATIM_WIN_SIZE_MAX),

	REG_VARIABLE(CFG_SAP_MAX_NO_PEERS, WLAN_PARAM_Integer,
		     struct hdd_config, maxNumberOfPeers,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_SAP_MAX_NO_PEERS_DEFAULT,
		     CFG_SAP_MAX_NO_PEERS_MIN,
		     CFG_SAP_MAX_NO_PEERS_MAX),

	REG_VARIABLE(CFG_IBSS_IS_POWER_SAVE_ALLOWED_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, isIbssPowerSaveAllowed,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_IBSS_IS_POWER_SAVE_ALLOWED_DEFAULT,
		     CFG_IBSS_IS_POWER_SAVE_ALLOWED_MIN,
		     CFG_IBSS_IS_POWER_SAVE_ALLOWED_MAX),

	REG_VARIABLE(CFG_IBSS_IS_POWER_COLLAPSE_ALLOWED_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, isIbssPowerCollapseAllowed,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_IBSS_IS_POWER_COLLAPSE_ALLOWED_DEFAULT,
		     CFG_IBSS_IS_POWER_COLLAPSE_ALLOWED_MIN,
		     CFG_IBSS_IS_POWER_COLLAPSE_ALLOWED_MAX),

	REG_VARIABLE(CFG_IBSS_AWAKE_ON_TX_RX_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, isIbssAwakeOnTxRx,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_IBSS_AWAKE_ON_TX_RX_DEFAULT,
		     CFG_IBSS_AWAKE_ON_TX_RX_MIN,
		     CFG_IBSS_AWAKE_ON_TX_RX_MAX),

	REG_VARIABLE(CFG_IBSS_INACTIVITY_TIME_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, ibssInactivityCount,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_IBSS_INACTIVITY_TIME_DEFAULT,
		     CFG_IBSS_INACTIVITY_TIME_MIN,
		     CFG_IBSS_INACTIVITY_TIME_MAX),

	REG_VARIABLE(CFG_IBSS_TXSP_END_INACTIVITY_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, ibssTxSpEndInactivityTime,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_IBSS_TXSP_END_INACTIVITY_DEFAULT,
		     CFG_IBSS_TXSP_END_INACTIVITY_MIN,
		     CFG_IBSS_TXSP_END_INACTIVITY_MAX),

	REG_VARIABLE(CFG_IBSS_PS_WARMUP_TIME_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, ibssPsWarmupTime,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_IBSS_PS_WARMUP_TIME_DEFAULT,
		     CFG_IBSS_PS_WARMUP_TIME_MIN,
		     CFG_IBSS_PS_WARMUP_TIME_MAX),

	REG_VARIABLE(CFG_IBSS_PS_1RX_CHAIN_IN_ATIM_WINDOW_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, ibssPs1RxChainInAtimEnable,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_IBSS_PS_1RX_CHAIN_IN_ATIM_WINDOW_DEFAULT,
		     CFG_IBSS_PS_1RX_CHAIN_IN_ATIM_WINDOW_MIN,
		     CFG_IBSS_PS_1RX_CHAIN_IN_ATIM_WINDOW_MAX),

	REG_VARIABLE(CFG_THERMAL_TEMP_MIN_LEVEL0_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, thermalTempMinLevel0,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_THERMAL_TEMP_MIN_LEVEL0_DEFAULT,
		     CFG_THERMAL_TEMP_MIN_LEVEL0_MIN,
		     CFG_THERMAL_TEMP_MIN_LEVEL0_MAX),

	REG_VARIABLE(CFG_THERMAL_TEMP_MAX_LEVEL0_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, thermalTempMaxLevel0,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_THERMAL_TEMP_MAX_LEVEL0_DEFAULT,
		     CFG_THERMAL_TEMP_MAX_LEVEL0_MIN,
		     CFG_THERMAL_TEMP_MAX_LEVEL0_MAX),

	REG_VARIABLE(CFG_THERMAL_TEMP_MIN_LEVEL1_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, thermalTempMinLevel1,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_THERMAL_TEMP_MIN_LEVEL1_DEFAULT,
		     CFG_THERMAL_TEMP_MIN_LEVEL1_MIN,
		     CFG_THERMAL_TEMP_MIN_LEVEL1_MAX),

	REG_VARIABLE(CFG_THERMAL_TEMP_MAX_LEVEL1_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, thermalTempMaxLevel1,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_THERMAL_TEMP_MAX_LEVEL1_DEFAULT,
		     CFG_THERMAL_TEMP_MAX_LEVEL1_MIN,
		     CFG_THERMAL_TEMP_MAX_LEVEL1_MAX),

	REG_VARIABLE(CFG_THERMAL_TEMP_MIN_LEVEL2_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, thermalTempMinLevel2,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_THERMAL_TEMP_MIN_LEVEL2_DEFAULT,
		     CFG_THERMAL_TEMP_MIN_LEVEL2_MIN,
		     CFG_THERMAL_TEMP_MIN_LEVEL2_MAX),

	REG_VARIABLE(CFG_THERMAL_TEMP_MAX_LEVEL2_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, thermalTempMaxLevel2,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_THERMAL_TEMP_MAX_LEVEL2_DEFAULT,
		     CFG_THERMAL_TEMP_MAX_LEVEL2_MIN,
		     CFG_THERMAL_TEMP_MAX_LEVEL2_MAX),

	REG_VARIABLE(CFG_THERMAL_TEMP_MIN_LEVEL3_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, thermalTempMinLevel3,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_THERMAL_TEMP_MIN_LEVEL3_DEFAULT,
		     CFG_THERMAL_TEMP_MIN_LEVEL3_MIN,
		     CFG_THERMAL_TEMP_MIN_LEVEL3_MAX),

	REG_VARIABLE(CFG_THERMAL_TEMP_MAX_LEVEL3_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, thermalTempMaxLevel3,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_THERMAL_TEMP_MAX_LEVEL3_DEFAULT,
		     CFG_THERMAL_TEMP_MAX_LEVEL3_MIN,
		     CFG_THERMAL_TEMP_MAX_LEVEL3_MAX),

	REG_VARIABLE(CFG_SET_TXPOWER_LIMIT2G_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, TxPower2g,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_SET_TXPOWER_LIMIT2G_DEFAULT,
		     CFG_SET_TXPOWER_LIMIT2G_MIN,
		     CFG_SET_TXPOWER_LIMIT2G_MAX),

	REG_VARIABLE(CFG_SET_TXPOWER_LIMIT5G_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, TxPower5g,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_SET_TXPOWER_LIMIT5G_DEFAULT,
		     CFG_SET_TXPOWER_LIMIT5G_MIN,
		     CFG_SET_TXPOWER_LIMIT5G_MAX),

	REG_VARIABLE(CFG_ENABLE_DEBUG_CONNECT_ISSUE, WLAN_PARAM_Integer,
		     struct hdd_config, gEnableDebugLog,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_DEBUG_CONNECT_ISSUE_DEFAULT,
		     CFG_ENABLE_DEBUG_CONNECT_ISSUE_MIN,
		     CFG_ENABLE_DEBUG_CONNECT_ISSUE_MAX),

	REG_VARIABLE(CFG_ENABLE_DFS_PHYERR_FILTEROFFLOAD_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, fDfsPhyerrFilterOffload,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_DFS_PHYERR_FILTEROFFLOAD_DEFAULT,
		     CFG_ENABLE_DFS_PHYERR_FILTEROFFLOAD_MIN,
		     CFG_ENABLE_DFS_PHYERR_FILTEROFFLOAD_MAX),

	REG_VARIABLE(CFG_ENABLE_OVERLAP_CH, WLAN_PARAM_Integer,
		     struct hdd_config, gEnableOverLapCh,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_ENABLE_OVERLAP_CH_DEFAULT,
		     CFG_ENABLE_OVERLAP_CH_MIN,
		     CFG_ENABLE_OVERLAP_CH_MAX),

	REG_VARIABLE(CFG_REG_CHANGE_DEF_COUNTRY_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, fRegChangeDefCountry,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_REG_CHANGE_DEF_COUNTRY_DEFAULT,
		     CFG_REG_CHANGE_DEF_COUNTRY_MIN,
		     CFG_REG_CHANGE_DEF_COUNTRY_MAX),

	REG_VARIABLE(CFG_ACS_WITH_MORE_PARAM_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, acs_with_more_param,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ACS_WITH_MORE_PARAM_DEFAULT,
		     CFG_ACS_WITH_MORE_PARAM_MIN,
		     CFG_ACS_WITH_MORE_PARAM_MAX),

	REG_VARIABLE(CFG_AUTO_CHANNEL_SELECT_WEIGHT, WLAN_PARAM_HexInteger,
		     struct hdd_config, auto_channel_select_weight,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_AUTO_CHANNEL_SELECT_WEIGHT_DEFAULT,
		     CFG_AUTO_CHANNEL_SELECT_WEIGHT_MIN,
		     CFG_AUTO_CHANNEL_SELECT_WEIGHT_MAX),

#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL
	REG_VARIABLE(CFG_LL_TX_FLOW_LWM, WLAN_PARAM_Integer,
		     struct hdd_config, TxFlowLowWaterMark,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_LL_TX_FLOW_LWM_DEFAULT,
		     CFG_LL_TX_FLOW_LWM_MIN,
		     CFG_LL_TX_FLOW_LWM_MAX),
	REG_VARIABLE(CFG_LL_TX_FLOW_HWM_OFFSET, WLAN_PARAM_Integer,
		     struct hdd_config, TxFlowHighWaterMarkOffset,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_LL_TX_FLOW_HWM_OFFSET_DEFAULT,
		     CFG_LL_TX_FLOW_HWM_OFFSET_MIN,
		     CFG_LL_TX_FLOW_HWM_OFFSET_MAX),
	REG_VARIABLE(CFG_LL_TX_FLOW_MAX_Q_DEPTH, WLAN_PARAM_Integer,
		     struct hdd_config, TxFlowMaxQueueDepth,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_LL_TX_FLOW_MAX_Q_DEPTH_DEFAULT,
		     CFG_LL_TX_FLOW_MAX_Q_DEPTH_MIN,
		     CFG_LL_TX_FLOW_MAX_Q_DEPTH_MAX),
	REG_VARIABLE(CFG_LL_TX_LBW_FLOW_LWM, WLAN_PARAM_Integer,
		     struct hdd_config, TxLbwFlowLowWaterMark,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_LL_TX_LBW_FLOW_LWM_DEFAULT,
		     CFG_LL_TX_LBW_FLOW_LWM_MIN,
		     CFG_LL_TX_LBW_FLOW_LWM_MAX),

	REG_VARIABLE(CFG_LL_TX_LBW_FLOW_HWM_OFFSET, WLAN_PARAM_Integer,
		     struct hdd_config, TxLbwFlowHighWaterMarkOffset,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_LL_TX_LBW_FLOW_HWM_OFFSET_DEFAULT,
		     CFG_LL_TX_LBW_FLOW_HWM_OFFSET_MIN,
		     CFG_LL_TX_LBW_FLOW_HWM_OFFSET_MAX),

	REG_VARIABLE(CFG_LL_TX_LBW_FLOW_MAX_Q_DEPTH, WLAN_PARAM_Integer,
		     struct hdd_config, TxLbwFlowMaxQueueDepth,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_LL_TX_LBW_FLOW_MAX_Q_DEPTH_DEFAULT,
		     CFG_LL_TX_LBW_FLOW_MAX_Q_DEPTH_MIN,
		     CFG_LL_TX_LBW_FLOW_MAX_Q_DEPTH_MAX),

	REG_VARIABLE(CFG_LL_TX_HBW_FLOW_LWM, WLAN_PARAM_Integer,
		     struct hdd_config, TxHbwFlowLowWaterMark,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_LL_TX_HBW_FLOW_LWM_DEFAULT,
		     CFG_LL_TX_HBW_FLOW_LWM_MIN,
		     CFG_LL_TX_HBW_FLOW_LWM_MAX),

	REG_VARIABLE(CFG_LL_TX_HBW_FLOW_HWM_OFFSET, WLAN_PARAM_Integer,
		     struct hdd_config, TxHbwFlowHighWaterMarkOffset,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_LL_TX_HBW_FLOW_HWM_OFFSET_DEFAULT,
		     CFG_LL_TX_HBW_FLOW_HWM_OFFSET_MIN,
		     CFG_LL_TX_HBW_FLOW_HWM_OFFSET_MAX),

	REG_VARIABLE(CFG_LL_TX_HBW_FLOW_MAX_Q_DEPTH, WLAN_PARAM_Integer,
		     struct hdd_config, TxHbwFlowMaxQueueDepth,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_LL_TX_HBW_FLOW_MAX_Q_DEPTH_DEFAULT,
		     CFG_LL_TX_HBW_FLOW_MAX_Q_DEPTH_MIN,
		     CFG_LL_TX_HBW_FLOW_MAX_Q_DEPTH_MAX),
#endif /* QCA_LL_LEGACY_TX_FLOW_CONTROL */
#ifdef QCA_LL_TX_FLOW_CONTROL_V2

	REG_VARIABLE(CFG_LL_TX_FLOW_STOP_QUEUE_TH, WLAN_PARAM_Integer,
		     struct hdd_config, TxFlowStopQueueThreshold,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_LL_TX_FLOW_STOP_QUEUE_TH_DEFAULT,
		     CFG_LL_TX_FLOW_STOP_QUEUE_TH_MIN,
		     CFG_LL_TX_FLOW_STOP_QUEUE_TH_MAX),

	REG_VARIABLE(CFG_LL_TX_FLOW_START_QUEUE_OFFSET, WLAN_PARAM_Integer,
		     struct hdd_config, TxFlowStartQueueOffset,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_LL_TX_FLOW_START_QUEUE_OFFSET_DEFAULT,
		     CFG_LL_TX_FLOW_START_QUEUE_OFFSET_MIN,
		     CFG_LL_TX_FLOW_START_QUEUE_OFFSET_MAX),

#endif
	REG_VARIABLE(CFG_INITIAL_DWELL_TIME_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nInitialDwellTime,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_INITIAL_DWELL_TIME_DEFAULT,
		     CFG_INITIAL_DWELL_TIME_MIN,
		     CFG_INITIAL_DWELL_TIME_MAX),

	REG_VARIABLE(CFG_INITIAL_SCAN_NO_DFS_CHNL_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, initial_scan_no_dfs_chnl,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_INITIAL_SCAN_NO_DFS_CHNL_DEFAULT,
		     CFG_INITIAL_SCAN_NO_DFS_CHNL_MIN,
		     CFG_INITIAL_SCAN_NO_DFS_CHNL_MAX),

	REG_VARIABLE(CFG_SAP_MAX_OFFLOAD_PEERS, WLAN_PARAM_Integer,
		     struct hdd_config, apMaxOffloadPeers,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_SAP_MAX_OFFLOAD_PEERS_DEFAULT,
		     CFG_SAP_MAX_OFFLOAD_PEERS_MIN,
		     CFG_SAP_MAX_OFFLOAD_PEERS_MAX),

	REG_VARIABLE(CFG_SAP_MAX_OFFLOAD_REORDER_BUFFS, WLAN_PARAM_Integer,
		     struct hdd_config, apMaxOffloadReorderBuffs,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_SAP_MAX_OFFLOAD_REORDER_BUFFS_DEFAULT,
		     CFG_SAP_MAX_OFFLOAD_REORDER_BUFFS_MIN,
		     CFG_SAP_MAX_OFFLOAD_REORDER_BUFFS_MAX),

	REG_VARIABLE(CFG_ADVERTISE_CONCURRENT_OPERATION_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, advertiseConcurrentOperation,
		     VAR_FLAGS_OPTIONAL,
		     CFG_ADVERTISE_CONCURRENT_OPERATION_DEFAULT,
		     CFG_ADVERTISE_CONCURRENT_OPERATION_MIN,
		     CFG_ADVERTISE_CONCURRENT_OPERATION_MAX),

	REG_VARIABLE(CFG_ENABLE_MEMORY_DEEP_SLEEP, WLAN_PARAM_Integer,
		     struct hdd_config, enableMemDeepSleep,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_MEMORY_DEEP_SLEEP_DEFAULT,
		     CFG_ENABLE_MEMORY_DEEP_SLEEP_MIN,
		     CFG_ENABLE_MEMORY_DEEP_SLEEP_MAX),

	REG_VARIABLE(CFG_ENABLE_CCK_TX_FIR_OVERRIDE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, enable_cck_tx_fir_override,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_CCK_TX_FIR_OVERRIDE_DEFAULT,
		     CFG_ENABLE_CCK_TX_FIR_OVERRIDE_MIN,
		     CFG_ENABLE_CCK_TX_FIR_OVERRIDE_MAX),

#ifndef REMOVE_PKT_LOG
	REG_VARIABLE(CFG_ENABLE_PACKET_LOG, WLAN_PARAM_Integer,
		     struct hdd_config, enablePacketLog,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_PACKET_LOG_DEFAULT,
		     CFG_ENABLE_PACKET_LOG_MIN,
		     CFG_ENABLE_PACKET_LOG_MAX),
#endif

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	REG_VARIABLE(CFG_ROAMING_OFFLOAD_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, isRoamOffloadEnabled,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_ROAMING_OFFLOAD_DEFAULT,
		     CFG_ROAMING_OFFLOAD_MIN,
		     CFG_ROAMING_OFFLOAD_MAX),
#endif
#ifdef MSM_PLATFORM
	REG_VARIABLE(CFG_BUS_BANDWIDTH_HIGH_THRESHOLD, WLAN_PARAM_Integer,
		     struct hdd_config, busBandwidthHighThreshold,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_BUS_BANDWIDTH_HIGH_THRESHOLD_DEFAULT,
		     CFG_BUS_BANDWIDTH_HIGH_THRESHOLD_MIN,
		     CFG_BUS_BANDWIDTH_HIGH_THRESHOLD_MAX),

	REG_VARIABLE(CFG_BUS_BANDWIDTH_MEDIUM_THRESHOLD, WLAN_PARAM_Integer,
		     struct hdd_config, busBandwidthMediumThreshold,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_BUS_BANDWIDTH_MEDIUM_THRESHOLD_DEFAULT,
		     CFG_BUS_BANDWIDTH_MEDIUM_THRESHOLD_MIN,
		     CFG_BUS_BANDWIDTH_MEDIUM_THRESHOLD_MAX),

	REG_VARIABLE(CFG_BUS_BANDWIDTH_LOW_THRESHOLD, WLAN_PARAM_Integer,
		     struct hdd_config, busBandwidthLowThreshold,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_BUS_BANDWIDTH_LOW_THRESHOLD_DEFAULT,
		     CFG_BUS_BANDWIDTH_LOW_THRESHOLD_MIN,
		     CFG_BUS_BANDWIDTH_LOW_THRESHOLD_MAX),

	REG_VARIABLE(CFG_BUS_BANDWIDTH_COMPUTE_INTERVAL, WLAN_PARAM_Integer,
		     struct hdd_config, busBandwidthComputeInterval,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_BUS_BANDWIDTH_COMPUTE_INTERVAL_DEFAULT,
		     CFG_BUS_BANDWIDTH_COMPUTE_INTERVAL_MIN,
		     CFG_BUS_BANDWIDTH_COMPUTE_INTERVAL_MAX),

	REG_VARIABLE(CFG_ENABLE_TCP_LIMIT_OUTPUT, WLAN_PARAM_Integer,
		     struct hdd_config, enable_tcp_limit_output,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_TCP_LIMIT_OUTPUT_DEFAULT,
		     CFG_ENABLE_TCP_LIMIT_OUTPUT_MIN,
		     CFG_ENABLE_TCP_LIMIT_OUTPUT_MAX),

	REG_VARIABLE(CFG_ENABLE_TCP_ADV_WIN_SCALE, WLAN_PARAM_Integer,
		     struct hdd_config, enable_tcp_adv_win_scale,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_TCP_ADV_WIN_SCALE_DEFAULT,
		     CFG_ENABLE_TCP_ADV_WIN_SCALE_MIN,
		     CFG_ENABLE_TCP_ADV_WIN_SCALE_MAX),

	REG_VARIABLE(CFG_ENABLE_TCP_DELACK, WLAN_PARAM_Integer,
		     struct hdd_config, enable_tcp_delack,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_TCP_DELACK_DEFAULT,
		     CFG_ENABLE_TCP_DELACK_MIN,
		     CFG_ENABLE_TCP_DELACK_MAX),

	REG_VARIABLE(CFG_TCP_DELACK_THRESHOLD_HIGH, WLAN_PARAM_Integer,
		     struct hdd_config, tcpDelackThresholdHigh,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TCP_DELACK_THRESHOLD_HIGH_DEFAULT,
		     CFG_TCP_DELACK_THRESHOLD_HIGH_MIN,
		     CFG_TCP_DELACK_THRESHOLD_HIGH_MAX),

	REG_VARIABLE(CFG_TCP_DELACK_THRESHOLD_LOW, WLAN_PARAM_Integer,
		     struct hdd_config, tcpDelackThresholdLow,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TCP_DELACK_THRESHOLD_LOW_DEFAULT,
		     CFG_TCP_DELACK_THRESHOLD_LOW_MIN,
		     CFG_TCP_DELACK_THRESHOLD_LOW_MAX),

	REG_VARIABLE(CFG_TCP_DELACK_TIMER_COUNT, WLAN_PARAM_Integer,
		     struct hdd_config, tcp_delack_timer_count,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TCP_DELACK_TIMER_COUNT_DEFAULT,
		     CFG_TCP_DELACK_TIMER_COUNT_MIN,
		     CFG_TCP_DELACK_TIMER_COUNT_MAX),

	REG_VARIABLE(CFG_TCP_TX_HIGH_TPUT_THRESHOLD_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, tcp_tx_high_tput_thres,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TCP_TX_HIGH_TPUT_THRESHOLD_DEFAULT,
		     CFG_TCP_TX_HIGH_TPUT_THRESHOLD_MIN,
		     CFG_TCP_TX_HIGH_TPUT_THRESHOLD_MAX),
	REG_VARIABLE(CFG_PERIODIC_STATS_DISPLAY_TIME_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, periodic_stats_disp_time,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_PERIODIC_STATS_DISPLAY_TIME_DEFAULT,
		     CFG_PERIODIC_STATS_DISPLAY_TIME_MIN,
		     CFG_PERIODIC_STATS_DISPLAY_TIME_MAX),
#endif

	REG_VARIABLE(CFG_ENABLE_FW_LOG_TYPE, WLAN_PARAM_Integer,
		     struct hdd_config, enableFwLogType,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_FW_LOG_TYPE_DEFAULT,
		     CFG_ENABLE_FW_LOG_TYPE_MIN,
		     CFG_ENABLE_FW_LOG_TYPE_MAX),

	REG_VARIABLE(CFG_ENABLE_FW_DEBUG_LOG_LEVEL, WLAN_PARAM_Integer,
		     struct hdd_config, enableFwLogLevel,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_FW_DEBUG_LOG_LEVEL_DEFAULT,
		     CFG_ENABLE_FW_DEBUG_LOG_LEVEL_MIN,
		     CFG_ENABLE_FW_DEBUG_LOG_LEVEL_MAX),

	REG_VARIABLE(CFG_ENABLE_FW_RTS_PROFILE, WLAN_PARAM_Integer,
		     struct hdd_config, rts_profile,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_FW_RTS_PROFILE_DEFAULT,
		     CFG_ENABLE_FW_RTS_PROFILE_MIN,
		     CFG_ENABLE_FW_RTS_PROFILE_MAX),

	REG_VARIABLE_STRING(CFG_ENABLE_FW_MODULE_LOG_LEVEL, WLAN_PARAM_String,
			    struct hdd_config, enableFwModuleLogLevel,
			    VAR_FLAGS_OPTIONAL,
			    (void *)CFG_ENABLE_FW_MODULE_LOG_DEFAULT),

	REG_VARIABLE_STRING(CFG_ENABLE_CONCURRENT_STA, WLAN_PARAM_String,
			    struct hdd_config, enableConcurrentSTA,
			    VAR_FLAGS_NONE,
			    (void *)CFG_ENABLE_CONCURRENT_STA_DEFAULT),

#ifdef WLAN_FEATURE_11W
	REG_VARIABLE(CFG_PMF_SA_QUERY_MAX_RETRIES_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, pmfSaQueryMaxRetries,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_PMF_SA_QUERY_MAX_RETRIES_DEFAULT,
		     CFG_PMF_SA_QUERY_MAX_RETRIES_MIN,
		     CFG_PMF_SA_QUERY_MAX_RETRIES_MAX),

	REG_VARIABLE(CFG_PMF_SA_QUERY_RETRY_INTERVAL_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, pmfSaQueryRetryInterval,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_PMF_SA_QUERY_RETRY_INTERVAL_DEFAULT,
		     CFG_PMF_SA_QUERY_RETRY_INTERVAL_MIN,
		     CFG_PMF_SA_QUERY_RETRY_INTERVAL_MAX),
#endif
	REG_VARIABLE(CFG_MAX_CONCURRENT_CONNECTIONS_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, gMaxConcurrentActiveSessions,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_MAX_CONCURRENT_CONNECTIONS_DEFAULT,
		     CFG_MAX_CONCURRENT_CONNECTIONS_MIN,
		     CFG_MAX_CONCURRENT_CONNECTIONS_MAX),

	REG_VARIABLE(CFG_ENABLE_CRASH_INJECT, WLAN_PARAM_Integer,
		     struct hdd_config, crash_inject_enabled,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_CRASH_INJECT_DEFAULT,
		     CFG_ENABLE_CRASH_INJECT_MIN,
		     CFG_ENABLE_CRASH_INJECT_MAX),

	REG_VARIABLE(CFG_IGNORE_CAC_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, ignoreCAC,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_IGNORE_CAC_DEFAULT,
		     CFG_IGNORE_CAC_MIN,
		     CFG_IGNORE_CAC_MAX),

	REG_VARIABLE(CFG_DFS_RADAR_PRI_MULTIPLIER_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, dfsRadarPriMultiplier,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_DFS_RADAR_PRI_MULTIPLIER_DEFAULT,
		     CFG_DFS_RADAR_PRI_MULTIPLIER_MIN,
		     CFG_DFS_RADAR_PRI_MULTIPLIER_MAX),

	REG_VARIABLE(CFG_REORDER_OFFLOAD_SUPPORT_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, reorderOffloadSupport,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_REORDER_OFFLOAD_SUPPORT_DEFAULT,
		     CFG_REORDER_OFFLOAD_SUPPORT_MIN,
		     CFG_REORDER_OFFLOAD_SUPPORT_MAX),

	REG_VARIABLE(CFG_IPA_UC_TX_BUF_COUNT_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, IpaUcTxBufCount,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_IPA_UC_TX_BUF_COUNT_DEFAULT,
		     CFG_IPA_UC_TX_BUF_COUNT_MIN,
		     CFG_IPA_UC_TX_BUF_COUNT_MAX),

	REG_VARIABLE(CFG_IPA_UC_TX_BUF_SIZE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, IpaUcTxBufSize,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_IPA_UC_TX_BUF_SIZE_DEFAULT,
		     CFG_IPA_UC_TX_BUF_SIZE_MIN,
		     CFG_IPA_UC_TX_BUF_SIZE_MAX),

	REG_VARIABLE(CFG_IPA_UC_RX_IND_RING_COUNT_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, IpaUcRxIndRingCount,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_IPA_UC_RX_IND_RING_COUNT_DEFAULT,
		     CFG_IPA_UC_RX_IND_RING_COUNT_MIN,
		     CFG_IPA_UC_RX_IND_RING_COUNT_MAX),

	REG_VARIABLE(CFG_IPA_UC_TX_PARTITION_BASE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, IpaUcTxPartitionBase,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_IPA_UC_TX_PARTITION_BASE_DEFAULT,
		     CFG_IPA_UC_TX_PARTITION_BASE_MIN,
		     CFG_IPA_UC_TX_PARTITION_BASE_MAX),
#ifdef WLAN_LOGGING_SOCK_SVC_ENABLE
	REG_VARIABLE(CFG_WLAN_LOGGING_SUPPORT_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, wlan_logging_enable,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_WLAN_LOGGING_SUPPORT_DEFAULT,
		     CFG_WLAN_LOGGING_SUPPORT_DISABLE,
		     CFG_WLAN_LOGGING_SUPPORT_ENABLE),

	REG_VARIABLE(CFG_WLAN_LOGGING_CONSOLE_SUPPORT_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, wlan_logging_to_console,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_WLAN_LOGGING_CONSOLE_SUPPORT_DEFAULT,
		     CFG_WLAN_LOGGING_CONSOLE_SUPPORT_DISABLE,
		     CFG_WLAN_LOGGING_CONSOLE_SUPPORT_ENABLE),
#endif /* WLAN_LOGGING_SOCK_SVC_ENABLE */

#ifdef WLAN_FEATURE_LPSS
	REG_VARIABLE(CFG_ENABLE_LPASS_SUPPORT, WLAN_PARAM_Integer,
		     struct hdd_config, enable_lpass_support,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_LPASS_SUPPORT_DEFAULT,
		     CFG_ENABLE_LPASS_SUPPORT_MIN,
		     CFG_ENABLE_LPASS_SUPPORT_MAX),
#endif

	REG_VARIABLE(CFG_ENABLE_SELF_RECOVERY, WLAN_PARAM_Integer,
		     struct hdd_config, enableSelfRecovery,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_SELF_RECOVERY_DEFAULT,
		     CFG_ENABLE_SELF_RECOVERY_MIN,
		     CFG_ENABLE_SELF_RECOVERY_MAX),

	REG_VARIABLE(CFG_ENABLE_SAP_SUSPEND, WLAN_PARAM_Integer,
		     struct hdd_config, enable_sap_suspend,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_SAP_SUSPEND_DEFAULT,
		     CFG_ENABLE_SAP_SUSPEND_MIN,
		     CFG_ENABLE_SAP_SUSPEND_MAX),

#ifdef WLAN_FEATURE_EXTWOW_SUPPORT
	REG_VARIABLE(CFG_EXTWOW_GO_TO_SUSPEND, WLAN_PARAM_Integer,
		     struct hdd_config, extWowGotoSuspend,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_EXTWOW_GO_TO_SUSPEND_DEFAULT,
		     CFG_EXTWOW_GO_TO_SUSPEND_MIN,
		     CFG_EXTWOW_GO_TO_SUSPEND_MAX),

	REG_VARIABLE(CFG_EXTWOW_APP1_WAKE_PIN_NUMBER, WLAN_PARAM_Integer,
		     struct hdd_config, extWowApp1WakeupPinNumber,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_EXTWOW_APP1_WAKE_PIN_NUMBER_DEFAULT,
		     CFG_EXTWOW_APP1_WAKE_PIN_NUMBER_MIN,
		     CFG_EXTWOW_APP1_WAKE_PIN_NUMBER_MAX),

	REG_VARIABLE(CFG_EXTWOW_APP2_WAKE_PIN_NUMBER, WLAN_PARAM_Integer,
		     struct hdd_config, extWowApp2WakeupPinNumber,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_EXTWOW_APP2_WAKE_PIN_NUMBER_DEFAULT,
		     CFG_EXTWOW_APP2_WAKE_PIN_NUMBER_MIN,
		     CFG_EXTWOW_APP2_WAKE_PIN_NUMBER_MAX),

	REG_VARIABLE(CFG_EXTWOW_KA_INIT_PING_INTERVAL, WLAN_PARAM_Integer,
		     struct hdd_config, extWowApp2KAInitPingInterval,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_EXTWOW_KA_INIT_PING_INTERVAL_DEFAULT,
		     CFG_EXTWOW_KA_INIT_PING_INTERVAL_MIN,
		     CFG_EXTWOW_KA_INIT_PING_INTERVAL_MAX),

	REG_VARIABLE(CFG_EXTWOW_KA_MIN_PING_INTERVAL, WLAN_PARAM_Integer,
		     struct hdd_config, extWowApp2KAMinPingInterval,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_EXTWOW_KA_MIN_PING_INTERVAL_DEFAULT,
		     CFG_EXTWOW_KA_MIN_PING_INTERVAL_MIN,
		     CFG_EXTWOW_KA_MIN_PING_INTERVAL_MAX),

	REG_VARIABLE(CFG_EXTWOW_KA_MAX_PING_INTERVAL, WLAN_PARAM_Integer,
		     struct hdd_config, extWowApp2KAMaxPingInterval,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_EXTWOW_KA_MAX_PING_INTERVAL_DEFAULT,
		     CFG_EXTWOW_KA_MAX_PING_INTERVAL_MIN,
		     CFG_EXTWOW_KA_MAX_PING_INTERVAL_MAX),

	REG_VARIABLE(CFG_EXTWOW_KA_INC_PING_INTERVAL, WLAN_PARAM_Integer,
		     struct hdd_config, extWowApp2KAIncPingInterval,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_EXTWOW_KA_INC_PING_INTERVAL_DEFAULT,
		     CFG_EXTWOW_KA_INC_PING_INTERVAL_MIN,
		     CFG_EXTWOW_KA_INC_PING_INTERVAL_MAX),

	REG_VARIABLE(CFG_EXTWOW_TCP_SRC_PORT, WLAN_PARAM_Integer,
		     struct hdd_config, extWowApp2TcpSrcPort,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_EXTWOW_TCP_SRC_PORT_DEFAULT,
		     CFG_EXTWOW_TCP_SRC_PORT_MIN,
		     CFG_EXTWOW_TCP_SRC_PORT_MAX),

	REG_VARIABLE(CFG_EXTWOW_TCP_DST_PORT, WLAN_PARAM_Integer,
		     struct hdd_config, extWowApp2TcpDstPort,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_EXTWOW_TCP_DST_PORT_DEFAULT,
		     CFG_EXTWOW_TCP_DST_PORT_MIN,
		     CFG_EXTWOW_TCP_DST_PORT_MAX),

	REG_VARIABLE(CFG_EXTWOW_TCP_TX_TIMEOUT, WLAN_PARAM_Integer,
		     struct hdd_config, extWowApp2TcpTxTimeout,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_EXTWOW_TCP_TX_TIMEOUT_DEFAULT,
		     CFG_EXTWOW_TCP_TX_TIMEOUT_MIN,
		     CFG_EXTWOW_TCP_TX_TIMEOUT_MAX),

	REG_VARIABLE(CFG_EXTWOW_TCP_RX_TIMEOUT, WLAN_PARAM_Integer,
		     struct hdd_config, extWowApp2TcpRxTimeout,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_EXTWOW_TCP_RX_TIMEOUT_DEFAULT,
		     CFG_EXTWOW_TCP_RX_TIMEOUT_MIN,
		     CFG_EXTWOW_TCP_RX_TIMEOUT_MAX),
#endif
	REG_VARIABLE(CFG_ENABLE_DEAUTH_TO_DISASSOC_MAP_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, gEnableDeauthToDisassocMap,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_DEAUTH_TO_DISASSOC_MAP_DEFAULT,
		     CFG_ENABLE_DEAUTH_TO_DISASSOC_MAP_MIN,
		     CFG_ENABLE_DEAUTH_TO_DISASSOC_MAP_MAX),
#ifdef DHCP_SERVER_OFFLOAD
	REG_VARIABLE(CFG_DHCP_SERVER_OFFLOAD_SUPPORT_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, enableDHCPServerOffload,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_DHCP_SERVER_OFFLOAD_SUPPORT_DEFAULT,
		     CFG_DHCP_SERVER_OFFLOAD_SUPPORT_MIN,
		     CFG_DHCP_SERVER_OFFLOAD_SUPPORT_MAX),
	REG_VARIABLE(CFG_DHCP_SERVER_OFFLOAD_NUM_CLIENT_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, dhcpMaxNumClients,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_DHCP_SERVER_OFFLOAD_NUM_CLIENT_DEFAULT,
		     CFG_DHCP_SERVER_OFFLOAD_NUM_CLIENT_MIN,
		     CFG_DHCP_SERVER_OFFLOAD_NUM_CLIENT_MAX),
	REG_VARIABLE_STRING(CFG_DHCP_SERVER_IP_NAME, WLAN_PARAM_String,
			    struct hdd_config, dhcpServerIP,
			    VAR_FLAGS_OPTIONAL,
			    (void *)CFG_DHCP_SERVER_IP_DEFAULT),
#endif /* DHCP_SERVER_OFFLOAD */

	REG_VARIABLE(CFG_ENABLE_DEAUTH_BEFORE_CONNECTION, WLAN_PARAM_Integer,
		struct hdd_config, send_deauth_before_con,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ENABLE_DEAUTH_BEFORE_CONNECTION_DEFAULT,
		CFG_ENABLE_DEAUTH_BEFORE_CONNECTION_MIN,
		CFG_ENABLE_DEAUTH_BEFORE_CONNECTION_MAX),

	REG_VARIABLE(CFG_ENABLE_MAC_ADDR_SPOOFING, WLAN_PARAM_Integer,
		     struct hdd_config, enable_mac_spoofing,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_MAC_ADDR_SPOOFING_DEFAULT,
		     CFG_ENABLE_MAC_ADDR_SPOOFING_MIN,
		     CFG_ENABLE_MAC_ADDR_SPOOFING_MAX),

	REG_VARIABLE(CFG_ENABLE_CUSTOM_CONC_RULE1_NAME,  WLAN_PARAM_Integer,
		     struct hdd_config, conc_custom_rule1,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_ENABLE_CUSTOM_CONC_RULE1_NAME_DEFAULT,
		     CFG_ENABLE_CUSTOM_CONC_RULE1_NAME_MIN,
		     CFG_ENABLE_CUSTOM_CONC_RULE1_NAME_MAX),

	REG_VARIABLE(CFG_ENABLE_CUSTOM_CONC_RULE2_NAME,  WLAN_PARAM_Integer,
		     struct hdd_config, conc_custom_rule2,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_ENABLE_CUSTOM_CONC_RULE2_NAME_DEFAULT,
		     CFG_ENABLE_CUSTOM_CONC_RULE2_NAME_MIN,
		     CFG_ENABLE_CUSTOM_CONC_RULE2_NAME_MAX),

	REG_VARIABLE(CFG_ENABLE_STA_CONNECTION_IN_5GHZ,  WLAN_PARAM_Integer,
		     struct hdd_config, is_sta_connection_in_5gz_enabled,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_ENABLE_STA_CONNECTION_IN_5GHZ_DEFAULT,
		     CFG_ENABLE_STA_CONNECTION_IN_5GHZ_MIN,
		     CFG_ENABLE_STA_CONNECTION_IN_5GHZ_MAX),

	REG_VARIABLE(CFG_STA_MIRACAST_MCC_REST_TIME_VAL, WLAN_PARAM_Integer,
		     struct hdd_config, sta_miracast_mcc_rest_time_val,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_STA_MIRACAST_MCC_REST_TIME_VAL_DEFAULT,
		     CFG_STA_MIRACAST_MCC_REST_TIME_VAL_MIN,
		     CFG_STA_MIRACAST_MCC_REST_TIME_VAL_MAX),

#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
	REG_VARIABLE(CFG_SAP_MCC_CHANNEL_AVOIDANCE_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config,
		     sap_channel_avoidance,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_SAP_MCC_CHANNEL_AVOIDANCE_DEFAULT,
		     CFG_SAP_MCC_CHANNEL_AVOIDANCE_MIN,
		     CFG_SAP_MCC_CHANNEL_AVOIDANCE_MAX),
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */

	REG_VARIABLE(CFG_SAP_11AC_OVERRIDE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, sap_11ac_override,
		     VAR_FLAGS_OPTIONAL |
				VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_SAP_11AC_OVERRIDE_DEFAULT,
		     CFG_SAP_11AC_OVERRIDE_MIN,
		     CFG_SAP_11AC_OVERRIDE_MAX),

	REG_VARIABLE(CFG_GO_11AC_OVERRIDE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, go_11ac_override,
		     VAR_FLAGS_OPTIONAL |
				VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_GO_11AC_OVERRIDE_DEFAULT,
		     CFG_GO_11AC_OVERRIDE_MIN,
		     CFG_GO_11AC_OVERRIDE_MAX),

	REG_VARIABLE(CFG_ENABLE_RAMDUMP_COLLECTION, WLAN_PARAM_Integer,
		     struct hdd_config, is_ramdump_enabled,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_RAMDUMP_COLLECTION_DEFAULT,
		     CFG_ENABLE_RAMDUMP_COLLECTION_MIN,
		     CFG_ENABLE_RAMDUMP_COLLECTION_MAX),

	REG_VARIABLE(CFG_SAP_DOT11MC, WLAN_PARAM_Integer,
		     struct hdd_config, sap_dot11mc,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_SAP_DOT11MC_DEFAULT,
		     CFG_SAP_DOT11MC_MIN,
		     CFG_SAP_DOT11MC_MAX),

	REG_VARIABLE(CFG_ENABLE_NON_DFS_CHAN_ON_RADAR, WLAN_PARAM_Integer,
		     struct hdd_config, prefer_non_dfs_on_radar,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_NON_DFS_CHAN_ON_RADAR_DEFAULT,
		     CFG_ENABLE_NON_DFS_CHAN_ON_RADAR_MIN,
		     CFG_ENABLE_NON_DFS_CHAN_ON_RADAR_MAX),

	REG_VARIABLE(CFG_MULTICAST_HOST_FW_MSGS, WLAN_PARAM_Integer,
		     struct hdd_config, multicast_host_fw_msgs,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_MULTICAST_HOST_FW_MSGS_DEFAULT,
		     CFG_MULTICAST_HOST_FW_MSGS_MIN,
		     CFG_MULTICAST_HOST_FW_MSGS_MAX),

	REG_VARIABLE(CFG_CONC_SYSTEM_PREF, WLAN_PARAM_Integer,
		   struct hdd_config, conc_system_pref,
		   VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		   CFG_CONC_SYSTEM_PREF_DEFAULT,
		   CFG_CONC_SYSTEM_PREF_MIN,
		   CFG_CONC_SYSTEM_PREF_MAX),

	REG_VARIABLE(CFG_DBS_SELECTION_POLICY, WLAN_PARAM_Integer,
		     struct hdd_config, dbs_selection_policy,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_DBS_SELECTION_POLICY_DEFAULT,
		     CFG_DBS_SELECTION_POLICY_MIN,
		     CFG_DBS_SELECTION_POLICY_MAX),

	REG_VARIABLE(CFG_VDEV_PRIORITY_LIST, WLAN_PARAM_Integer,
		     struct hdd_config, vdev_priority_list,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_VDEV_PRIORITY_LIST_DEFAULT,
		     CFG_VDEV_PRIORITY_LIST_MIN,
		     CFG_VDEV_PRIORITY_LIST_MAX),

	REG_VARIABLE(CFG_TSO_ENABLED_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, tso_enable,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TSO_ENABLED_DEFAULT,
		     CFG_TSO_ENABLED_MIN,
		     CFG_TSO_ENABLED_MAX),

	REG_VARIABLE(CFG_LRO_ENABLED_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, lro_enable,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_LRO_ENABLED_DEFAULT,
		     CFG_LRO_ENABLED_MIN,
		     CFG_LRO_ENABLED_MAX),

	REG_VARIABLE(CFG_GRO_ENABLED_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, gro_enable,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_GRO_ENABLED_DEFAULT,
		     CFG_GRO_ENABLED_MIN,
		     CFG_GRO_ENABLED_MAX),

	REG_VARIABLE(CFG_APF_PACKET_FILTER_OFFLOAD, WLAN_PARAM_Integer,
		     struct hdd_config, apf_packet_filter_enable,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_APF_PACKET_FILTER_OFFLOAD_DEFAULT,
		     CFG_APF_PACKET_FILTER_OFFLOAD_MIN,
		     CFG_APF_PACKET_FILTER_OFFLOAD_MAX),

	REG_VARIABLE(CFG_FLOW_STEERING_ENABLED_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, flow_steering_enable,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_FLOW_STEERING_ENABLED_DEFAULT,
		     CFG_FLOW_STEERING_ENABLED_MIN,
		     CFG_FLOW_STEERING_ENABLED_MAX),

	REG_VARIABLE(CFG_MAX_MSDUS_PER_RXIND_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, max_msdus_per_rxinorderind,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_MAX_MSDUS_PER_RXIND_DEFAULT,
		     CFG_MAX_MSDUS_PER_RXIND_MIN,
		     CFG_MAX_MSDUS_PER_RXIND_MAX),

	REG_VARIABLE(CFG_ACTIVE_MODE_OFFLOAD, WLAN_PARAM_Integer,
		     struct hdd_config, active_mode_offload,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ACTIVE_MODE_OFFLOAD_DEFAULT,
		     CFG_ACTIVE_MODE_OFFLOAD_MIN,
		     CFG_ACTIVE_MODE_OFFLOAD_MAX),

	REG_VARIABLE(CFG_FINE_TIME_MEAS_CAPABILITY, WLAN_PARAM_HexInteger,
		struct hdd_config, fine_time_meas_cap,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_FINE_TIME_MEAS_CAPABILITY_DEFAULT,
		CFG_FINE_TIME_MEAS_CAPABILITY_MIN,
		CFG_FINE_TIME_MEAS_CAPABILITY_MAX),

#ifdef WLAN_FEATURE_FASTPATH
	REG_VARIABLE(CFG_ENABLE_FASTPATH, WLAN_PARAM_Integer,
		     struct hdd_config, fastpath_enable,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_FASTPATH_DEFAULT,
		     CFG_ENABLE_FASTPATH_MIN,
		     CFG_ENABLE_FASTPATH_MAX),
#endif
	REG_VARIABLE(CFG_MAX_SCAN_COUNT_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, max_scan_count,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_MAX_SCAN_COUNT_DEFAULT,
		     CFG_MAX_SCAN_COUNT_MIN,
		     CFG_MAX_SCAN_COUNT_MAX),

	REG_VARIABLE(CFG_DOT11P_MODE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, dot11p_mode,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_DOT11P_MODE_DEFAULT,
		     CFG_DOT11P_MODE_MIN,
		     CFG_DOT11P_MODE_MAX),

	REG_VARIABLE(CFG_ETSI13_SRD_CHAN_IN_MASTER_MODE, WLAN_PARAM_Integer,
		     struct hdd_config, etsi13_srd_chan_in_master_mode,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ETSI13_SRD_CHAN_IN_MASTER_MODE_DEF,
		     CFG_ETSI13_SRD_CHAN_IN_MASTER_MODE_MIN,
		     CFG_ETSI13_SRD_CHAN_IN_MASTER_MODE_MAX),

#ifdef WLAN_FEATURE_WOW_PULSE
	REG_VARIABLE(CFG_WOW_PULSE_SUPPORT_NAME, WLAN_PARAM_Integer,
		struct hdd_config, wow_pulse_support,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_WOW_PULSE_SUPPORT_DEFAULT,
		CFG_WOW_PULSE_SUPPORT_MIN,
		CFG_WOW_PULSE_SUPPORT_MAX),

	REG_VARIABLE(CFG_WOW_PULSE_PIN_NAME, WLAN_PARAM_Integer,
		struct hdd_config, wow_pulse_pin,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_WOW_PULSE_PIN_DEFAULT,
		CFG_WOW_PULSE_PIN_MIN,
		CFG_WOW_PULSE_PIN_MAX),

	REG_VARIABLE(CFG_WOW_PULSE_INTERVAL_LOW_NAME, WLAN_PARAM_Integer,
		struct hdd_config, wow_pulse_interval_low,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_WOW_PULSE_INTERVAL_LOW_DEFAULT,
		CFG_WOW_PULSE_INTERVAL_LOW_MIN,
		CFG_WOW_PULSE_INTERVAL_LOW_MAX),

	REG_VARIABLE(CFG_WOW_PULSE_INTERVAL_HIGH_NAME, WLAN_PARAM_Integer,
		struct hdd_config, wow_pulse_interval_high,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_WOW_PULSE_INTERVAL_HIGH_DEFAULT,
		CFG_WOW_PULSE_INTERVAL_HIGH_MIN,
		CFG_WOW_PULSE_INTERVAL_HIGH_MAX),
#endif


	REG_VARIABLE(CFG_CE_CLASSIFY_ENABLE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, ce_classify_enabled,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_CE_CLASSIFY_ENABLE_DEFAULT,
		     CFG_CE_CLASSIFY_ENABLE_MIN,
		     CFG_CE_CLASSIFY_ENABLE_MAX),

	REG_VARIABLE(CFG_DUAL_MAC_FEATURE_DISABLE, WLAN_PARAM_HexInteger,
		     struct hdd_config, dual_mac_feature_disable,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_DUAL_MAC_FEATURE_DISABLE_DEFAULT,
		     CFG_DUAL_MAC_FEATURE_DISABLE_MIN,
		     CFG_DUAL_MAC_FEATURE_DISABLE_MAX),

	REG_VARIABLE_STRING(CFG_DBS_SCAN_SELECTION_NAME, WLAN_PARAM_String,
		     struct hdd_config, dbs_scan_selection,
		     VAR_FLAGS_OPTIONAL,
		     (void *)CFG_DBS_SCAN_SELECTION_DEFAULT),

	REG_VARIABLE(CFG_STA_SAP_SCC_ON_DFS_CHAN, WLAN_PARAM_HexInteger,
		     struct hdd_config, sta_sap_scc_on_dfs_chan,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_STA_SAP_SCC_ON_DFS_CHAN_DEFAULT,
		     CFG_STA_SAP_SCC_ON_DFS_CHAN_MIN,
		     CFG_STA_SAP_SCC_ON_DFS_CHAN_MAX),

	REG_VARIABLE(CFG_STA_SAP_SCC_ON_LTE_COEX_CHAN, WLAN_PARAM_HexInteger,
		     struct hdd_config, sta_sap_scc_on_lte_coex_chan,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_STA_SAP_SCC_ON_LTE_COEX_CHAN_DEFAULT,
		     CFG_STA_SAP_SCC_ON_LTE_COEX_CHAN_MIN,
		     CFG_STA_SAP_SCC_ON_LTE_COEX_CHAN_MAX),

#ifdef FEATURE_WLAN_SCAN_PNO
	REG_VARIABLE(CFG_PNO_CHANNEL_PREDICTION_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, pno_channel_prediction,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_PNO_CHANNEL_PREDICTION_DEFAULT,
		     CFG_PNO_CHANNEL_PREDICTION_MIN,
		     CFG_PNO_CHANNEL_PREDICTION_MAX),

	REG_VARIABLE(CFG_TOP_K_NUM_OF_CHANNELS_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, top_k_num_of_channels,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TOP_K_NUM_OF_CHANNELS_DEFAULT,
		     CFG_TOP_K_NUM_OF_CHANNELS_MIN,
		     CFG_TOP_K_NUM_OF_CHANNELS_MAX),

	REG_VARIABLE(CFG_STATIONARY_THRESHOLD_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, stationary_thresh,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_STATIONARY_THRESHOLD_DEFAULT,
		     CFG_STATIONARY_THRESHOLD_MIN,
		     CFG_STATIONARY_THRESHOLD_MAX),

	REG_VARIABLE(CFG_CHANNEL_PREDICTION_FULL_SCAN_MS_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, channel_prediction_full_scan,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_CHANNEL_PREDICTION_FULL_SCAN_MS_DEFAULT,
		     CFG_CHANNEL_PREDICTION_FULL_SCAN_MS_MIN,
		     CFG_CHANNEL_PREDICTION_FULL_SCAN_MS_MAX),

	REG_VARIABLE(CFG_ADAPTIVE_PNOSCAN_DWELL_MODE_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, pnoscan_adaptive_dwell_mode,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ADAPTIVE_PNOSCAN_DWELL_MODE_DEFAULT,
		     CFG_ADAPTIVE_PNOSCAN_DWELL_MODE_MIN,
		     CFG_ADAPTIVE_PNOSCAN_DWELL_MODE_MAX),
#endif
	REG_VARIABLE(CFG_ENABLE_SMART_CHAINMASK_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, smart_chainmask_enabled,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_SMART_CHAINMASK_DEFAULT,
		     CFG_ENABLE_SMART_CHAINMASK_MIN,
		     CFG_ENABLE_SMART_CHAINMASK_MAX),

	REG_VARIABLE(CFG_ENABLE_COEX_ALT_CHAINMASK_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, alternative_chainmask_enabled,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_COEX_ALT_CHAINMASK_DEFAULT,
		     CFG_ENABLE_COEX_ALT_CHAINMASK_MIN,
		     CFG_ENABLE_COEX_ALT_CHAINMASK_MAX),

	REG_VARIABLE(CFG_SELF_GEN_FRM_PWR, WLAN_PARAM_Integer,
		     struct hdd_config, self_gen_frm_pwr,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_SELF_GEN_FRM_PWR_DEFAULT,
		     CFG_SELF_GEN_FRM_PWR_MIN,
		     CFG_SELF_GEN_FRM_PWR_MAX),

	REG_VARIABLE(CFG_EARLY_STOP_SCAN_ENABLE, WLAN_PARAM_Integer,
		     struct hdd_config, early_stop_scan_enable,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_EARLY_STOP_SCAN_ENABLE_DEFAULT,
		     CFG_EARLY_STOP_SCAN_ENABLE_MIN,
		     CFG_EARLY_STOP_SCAN_ENABLE_MAX),

	REG_VARIABLE(CFG_EARLY_STOP_SCAN_MIN_THRESHOLD,
		     WLAN_PARAM_SignedInteger, struct hdd_config,
		     early_stop_scan_min_threshold,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_EARLY_STOP_SCAN_MIN_THRESHOLD_DEFAULT,
		     CFG_EARLY_STOP_SCAN_MIN_THRESHOLD_MIN,
		     CFG_EARLY_STOP_SCAN_MIN_THRESHOLD_MAX),

	REG_VARIABLE(CFG_EARLY_STOP_SCAN_MAX_THRESHOLD,
		     WLAN_PARAM_SignedInteger, struct hdd_config,
		     early_stop_scan_max_threshold,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_EARLY_STOP_SCAN_MAX_THRESHOLD_DEFAULT,
		     CFG_EARLY_STOP_SCAN_MAX_THRESHOLD_MIN,
		     CFG_EARLY_STOP_SCAN_MAX_THRESHOLD_MAX),

	REG_VARIABLE(CFG_FIRST_SCAN_BUCKET_THRESHOLD_NAME,
		     WLAN_PARAM_SignedInteger,
		     struct hdd_config, first_scan_bucket_threshold,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_FIRST_SCAN_BUCKET_THRESHOLD_DEFAULT,
		     CFG_FIRST_SCAN_BUCKET_THRESHOLD_MIN,
		     CFG_FIRST_SCAN_BUCKET_THRESHOLD_MAX),

#ifdef FEATURE_LFR_SUBNET_DETECTION
	REG_VARIABLE(CFG_ENABLE_LFR_SUBNET_DETECTION, WLAN_PARAM_Integer,
		     struct hdd_config, enable_lfr_subnet_detection,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_LFR_SUBNET_DEFAULT,
		     CFG_ENABLE_LFR_SUBNET_MIN,
		     CFG_ENABLE_LFR_SUBNET_MAX),
#endif
	REG_VARIABLE(CFG_OBSS_HT40_SCAN_ACTIVE_DWELL_TIME_NAME,
		WLAN_PARAM_Integer,
		struct hdd_config, obss_active_dwelltime,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_OBSS_HT40_SCAN_ACTIVE_DWELL_TIME_DEFAULT,
		CFG_OBSS_HT40_SCAN_ACTIVE_DWELL_TIME_MIN,
		CFG_OBSS_HT40_SCAN_ACTIVE_DWELL_TIME_MAX),

	REG_VARIABLE(CFG_OBSS_HT40_SCAN_PASSIVE_DWELL_TIME_NAME,
		WLAN_PARAM_Integer,
		struct hdd_config, obss_passive_dwelltime,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_OBSS_HT40_SCAN_PASSIVE_DWELL_TIME_DEFAULT,
		CFG_OBSS_HT40_SCAN_PASSIVE_DWELL_TIME_MIN,
		CFG_OBSS_HT40_SCAN_PASSIVE_DWELL_TIME_MAX),

	REG_VARIABLE(CFG_OBSS_HT40_SCAN_WIDTH_TRIGGER_INTERVAL_NAME,
		WLAN_PARAM_Integer,
		struct hdd_config, obss_width_trigger_interval,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_OBSS_HT40_SCAN_WIDTH_TRIGGER_INTERVAL_DEFAULT,
		CFG_OBSS_HT40_SCAN_WIDTH_TRIGGER_INTERVAL_MIN,
		CFG_OBSS_HT40_SCAN_WIDTH_TRIGGER_INTERVAL_MAX),

	REG_VARIABLE(CFG_INFORM_BSS_RSSI_RAW_NAME, WLAN_PARAM_Integer,
		struct hdd_config, inform_bss_rssi_raw,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_INFORM_BSS_RSSI_RAW_DEFAULT,
		CFG_INFORM_BSS_RSSI_RAW_MIN,
		CFG_INFORM_BSS_RSSI_RAW_MAX),

#ifdef WLAN_FEATURE_TSF
	REG_VARIABLE(CFG_SET_TSF_GPIO_PIN_NAME, WLAN_PARAM_Integer,
		struct hdd_config, tsf_gpio_pin,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_SET_TSF_GPIO_PIN_DEFAULT,
		CFG_SET_TSF_GPIO_PIN_MIN,
		CFG_SET_TSF_GPIO_PIN_MAX),

#ifdef WLAN_FEATURE_TSF_PLUS
	REG_VARIABLE(CFG_SET_TSF_PTP_OPT_NAME, WLAN_PARAM_HexInteger,
		     struct hdd_config, tsf_ptp_options,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_SET_TSF_PTP_OPT_DEFAULT,
		     CFG_SET_TSF_PTP_OPT_MIN,
		     CFG_SET_TSF_PTP_OPT_MAX),
#endif /* WLAN_FEATURE_TSF_PLUS */
#endif

	REG_VARIABLE(CFG_ROAM_DENSE_TRAFFIC_THRESHOLD, WLAN_PARAM_Integer,
		struct hdd_config, roam_dense_traffic_thresh,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ROAM_DENSE_TRAFFIC_THRESHOLD_DEFAULT,
		CFG_ROAM_DENSE_TRAFFIC_THRESHOLD_MIN,
		CFG_ROAM_DENSE_TRAFFIC_THRESHOLD_MAX),

	REG_VARIABLE(CFG_ROAM_DENSE_RSSI_THRE_OFFSET, WLAN_PARAM_Integer,
		struct hdd_config, roam_dense_rssi_thresh_offset,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ROAM_DENSE_RSSI_THRE_OFFSET_DEFAULT,
		CFG_ROAM_DENSE_RSSI_THRE_OFFSET_MIN,
		CFG_ROAM_DENSE_RSSI_THRE_OFFSET_MAX),

	REG_VARIABLE(CFG_ROAM_DENSE_MIN_APS, WLAN_PARAM_Integer,
		struct hdd_config, roam_dense_min_aps,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ROAM_DENSE_MIN_APS_DEFAULT,
		CFG_ROAM_DENSE_MIN_APS_MIN,
		CFG_ROAM_DENSE_MIN_APS_MAX),

	REG_VARIABLE(CFG_ROAM_BG_SCAN_BAD_RSSI_THRESHOLD_NAME,
		WLAN_PARAM_SignedInteger, struct hdd_config,
		roam_bg_scan_bad_rssi_thresh,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ROAM_BG_SCAN_BAD_RSSI_THRESHOLD_DEFAULT,
		CFG_ROAM_BG_SCAN_BAD_RSSI_THRESHOLD_MIN,
		CFG_ROAM_BG_SCAN_BAD_RSSI_THRESHOLD_MAX),

	REG_VARIABLE(CFG_ROAM_BG_SCAN_CLIENT_BITMAP_NAME, WLAN_PARAM_Integer,
		struct hdd_config, roam_bg_scan_client_bitmap,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ROAM_BG_SCAN_CLIENT_BITMAP_DEFAULT,
		CFG_ROAM_BG_SCAN_CLIENT_BITMAP_MIN,
		CFG_ROAM_BG_SCAN_CLIENT_BITMAP_MAX),

	REG_VARIABLE(CFG_ROAM_BG_SCAN_BAD_RSSI_OFFSET_2G_NAME,
		WLAN_PARAM_Integer, struct hdd_config,
		roam_bad_rssi_thresh_offset_2g,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ROAM_BG_SCAN_BAD_RSSI_OFFSET_2G_DEFAULT,
		CFG_ROAM_BG_SCAN_BAD_RSSI_OFFSET_2G_MIN,
		CFG_ROAM_BG_SCAN_BAD_RSSI_OFFSET_2G_MAX),

	REG_VARIABLE(CFG_ROAM_HO_DELAY_FOR_RX_NAME,
		WLAN_PARAM_Integer, struct hdd_config,
		ho_delay_for_rx,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ROAM_HO_DELAY_FOR_RX_DEFAULT,
		CFG_ROAM_HO_DELAY_FOR_RX_MIN,
		CFG_ROAM_HO_DELAY_FOR_RX_MAX),

	REG_VARIABLE(CFG_MIN_DELAY_BTW_ROAM_SCAN_NAME,
		WLAN_PARAM_Integer, struct hdd_config,
		min_delay_btw_roam_scans,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_MIN_DELAY_BTW_ROAM_SCAN_DEFAULT,
		CFG_MIN_DELAY_BTW_ROAM_SCAN_MIN,
		CFG_MIN_DELAY_BTW_ROAM_SCAN_MAX),

	REG_VARIABLE(CFG_ROAM_SCAN_TRIGGER_REASON_BITMASK_NAME,
		WLAN_PARAM_HexInteger, struct hdd_config,
		roam_trigger_reason_bitmask,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ROAM_SCAN_TRIGGER_REASON_BITMASK_DEFAULT,
		CFG_ROAM_SCAN_TRIGGER_REASON_BITMASK_MIN,
		CFG_ROAM_SCAN_TRIGGER_REASON_BITMASK_MAX),

	REG_VARIABLE(CFG_ENABLE_FATAL_EVENT_TRIGGER, WLAN_PARAM_Integer,
			struct hdd_config, enable_fatal_event,
			VAR_FLAGS_OPTIONAL |
			VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			CFG_ENABLE_FATAL_EVENT_TRIGGER_DEFAULT,
			CFG_ENABLE_FATAL_EVENT_TRIGGER_MIN,
			CFG_ENABLE_FATAL_EVENT_TRIGGER_MAX),

	REG_VARIABLE(CFG_ENABLE_EDCA_INI_NAME, WLAN_PARAM_Integer,
			struct hdd_config, enable_edca_params,
			VAR_FLAGS_OPTIONAL |
			VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			CFG_ENABLE_EDCA_INI_DEFAULT,
			CFG_ENABLE_EDCA_INI_MIN,
			CFG_ENABLE_EDCA_INI_MAX),

	REG_VARIABLE(CFG_ENABLE_GO_CTS2SELF_FOR_STA, WLAN_PARAM_Integer,
			struct hdd_config, enable_go_cts2self_for_sta,
			VAR_FLAGS_OPTIONAL |
			VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			CFG_ENABLE_GO_CTS2SELF_FOR_STA_DEFAULT,
			CFG_ENABLE_GO_CTS2SELF_FOR_STA_MIN,
			CFG_ENABLE_GO_CTS2SELF_FOR_STA_MAX),

	REG_VARIABLE(CFG_EDCA_VO_CWMIN_VALUE_NAME, WLAN_PARAM_Integer,
			struct hdd_config, edca_vo_cwmin,
			VAR_FLAGS_OPTIONAL |
			VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			CFG_EDCA_VO_CWMIN_VALUE_DEFAULT,
			CFG_EDCA_VO_CWMIN_VALUE_MIN,
			CFG_EDCA_VO_CWMIN_VALUE_MAX),

	REG_VARIABLE(CFG_EDCA_VI_CWMIN_VALUE_NAME, WLAN_PARAM_Integer,
			struct hdd_config, edca_vi_cwmin,
			VAR_FLAGS_OPTIONAL |
			VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			CFG_EDCA_VI_CWMIN_VALUE_DEFAULT,
			CFG_EDCA_VI_CWMIN_VALUE_MIN,
			CFG_EDCA_VI_CWMIN_VALUE_MAX),

	REG_VARIABLE(CFG_EDCA_BK_CWMIN_VALUE_NAME, WLAN_PARAM_Integer,
			struct hdd_config, edca_bk_cwmin,
			VAR_FLAGS_OPTIONAL |
			VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			CFG_EDCA_BK_CWMIN_VALUE_DEFAULT,
			CFG_EDCA_BK_CWMIN_VALUE_MIN,
			CFG_EDCA_BK_CWMIN_VALUE_MAX),

	REG_VARIABLE(CFG_EDCA_BE_CWMIN_VALUE_NAME, WLAN_PARAM_Integer,
			struct hdd_config, edca_be_cwmin,
			VAR_FLAGS_OPTIONAL |
			VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			CFG_EDCA_BE_CWMIN_VALUE_DEFAULT,
			CFG_EDCA_BE_CWMIN_VALUE_MIN,
			CFG_EDCA_BE_CWMIN_VALUE_MAX),

	REG_VARIABLE(CFG_EDCA_VO_CWMAX_VALUE_NAME, WLAN_PARAM_Integer,
			struct hdd_config, edca_vo_cwmax,
			VAR_FLAGS_OPTIONAL |
			VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			CFG_EDCA_VO_CWMAX_VALUE_DEFAULT,
			CFG_EDCA_VO_CWMAX_VALUE_MIN,
			CFG_EDCA_VO_CWMAX_VALUE_MAX),

	REG_VARIABLE(CFG_EDCA_VI_CWMAX_VALUE_NAME, WLAN_PARAM_Integer,
			struct hdd_config, edca_vi_cwmax,
			VAR_FLAGS_OPTIONAL |
			VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			CFG_EDCA_VI_CWMAX_VALUE_DEFAULT,
			CFG_EDCA_VI_CWMAX_VALUE_MIN,
			CFG_EDCA_VI_CWMAX_VALUE_MAX),

	REG_VARIABLE(CFG_EDCA_BK_CWMAX_VALUE_NAME, WLAN_PARAM_Integer,
			struct hdd_config, edca_bk_cwmax,
			VAR_FLAGS_OPTIONAL |
			VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			CFG_EDCA_BK_CWMAX_VALUE_DEFAULT,
			CFG_EDCA_BK_CWMAX_VALUE_MIN,
			CFG_EDCA_BK_CWMAX_VALUE_MAX),

	REG_VARIABLE(CFG_EDCA_BE_CWMAX_VALUE_NAME, WLAN_PARAM_Integer,
			struct hdd_config, edca_be_cwmax,
			VAR_FLAGS_OPTIONAL |
			VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			CFG_EDCA_BE_CWMAX_VALUE_DEFAULT,
			CFG_EDCA_BE_CWMAX_VALUE_MIN,
			CFG_EDCA_BE_CWMAX_VALUE_MAX),

	REG_VARIABLE(CFG_EDCA_VO_AIFS_VALUE_NAME, WLAN_PARAM_Integer,
			struct hdd_config, edca_vo_aifs,
			VAR_FLAGS_OPTIONAL |
			VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			CFG_EDCA_VO_AIFS_VALUE_DEFAULT,
			CFG_EDCA_VO_AIFS_VALUE_MIN,
			CFG_EDCA_VO_AIFS_VALUE_MAX),

	REG_VARIABLE(CFG_EDCA_VI_AIFS_VALUE_NAME, WLAN_PARAM_Integer,
			struct hdd_config, edca_vi_aifs,
			VAR_FLAGS_OPTIONAL |
			VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			CFG_EDCA_VI_AIFS_VALUE_DEFAULT,
			CFG_EDCA_VI_AIFS_VALUE_MIN,
			CFG_EDCA_VI_AIFS_VALUE_MAX),

	REG_VARIABLE(CFG_EDCA_BK_AIFS_VALUE_NAME, WLAN_PARAM_Integer,
			struct hdd_config, edca_bk_aifs,
			VAR_FLAGS_OPTIONAL |
			VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			CFG_EDCA_BK_AIFS_VALUE_DEFAULT,
			CFG_EDCA_BK_AIFS_VALUE_MIN,
			CFG_EDCA_BK_AIFS_VALUE_MAX),

	REG_VARIABLE(CFG_EDCA_BE_AIFS_VALUE_NAME, WLAN_PARAM_Integer,
			struct hdd_config, edca_be_aifs,
			VAR_FLAGS_OPTIONAL |
			VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			CFG_EDCA_BE_AIFS_VALUE_DEFAULT,
			CFG_EDCA_BE_AIFS_VALUE_MIN,
			CFG_EDCA_BE_AIFS_VALUE_MAX),

	REG_VARIABLE_STRING(CFG_ENABLE_TX_SCHED_WRR_VO_NAME,
			WLAN_PARAM_String,
			struct hdd_config, tx_sched_wrr_vo,
			VAR_FLAGS_OPTIONAL,
			(void *) CFG_ENABLE_TX_SCHED_WRR_VO_DEFAULT),

	REG_VARIABLE_STRING(CFG_ENABLE_TX_SCHED_WRR_VI_NAME,
			WLAN_PARAM_String,
			struct hdd_config, tx_sched_wrr_vi,
			VAR_FLAGS_OPTIONAL,
			(void *) CFG_ENABLE_TX_SCHED_WRR_VI_DEFAULT),

	REG_VARIABLE_STRING(CFG_ENABLE_TX_SCHED_WRR_BE_NAME,
			WLAN_PARAM_String,
			struct hdd_config, tx_sched_wrr_be,
			VAR_FLAGS_OPTIONAL,
			(void *) CFG_ENABLE_TX_SCHED_WRR_BE_DEFAULT),

	REG_VARIABLE_STRING(CFG_ENABLE_TX_SCHED_WRR_BK_NAME,
			WLAN_PARAM_String,
			struct hdd_config, tx_sched_wrr_bk,
			VAR_FLAGS_OPTIONAL,
			(void *) CFG_ENABLE_TX_SCHED_WRR_BK_DEFAULT),

	REG_VARIABLE(CFG_CREATE_BUG_REPORT_FOR_SCAN, WLAN_PARAM_Integer,
		struct hdd_config, bug_report_for_no_scan_results,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_CREATE_BUG_REPORT_FOR_SCAN_DEFAULT,
		CFG_CREATE_BUG_REPORT_FOR_SCAN_DISABLE,
		CFG_CREATE_BUG_REPORT_FOR_SCAN_ENABLE),

	REG_VARIABLE(CFG_USER_AUTO_CHANNEL_SELECTION, WLAN_PARAM_Integer,
		struct hdd_config, vendor_acs_support,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_USER_AUTO_CHANNEL_SELECTION_DEFAULT,
		CFG_USER_AUTO_CHANNEL_SELECTION_DISABLE,
		CFG_USER_AUTO_CHANNEL_SELECTION_ENABLE),

	REG_VARIABLE(CFG_USER_ACS_DFS_LTE, WLAN_PARAM_Integer,
		struct hdd_config, acs_support_for_dfs_ltecoex,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_USER_ACS_DFS_LTE_DEFAULT,
		CFG_USER_ACS_DFS_LTE_DISABLE,
		CFG_USER_ACS_DFS_LTE_ENABLE),

#ifdef CONFIG_DP_TRACE
	REG_VARIABLE(CFG_ENABLE_DP_TRACE, WLAN_PARAM_Integer,
		struct hdd_config, enable_dp_trace,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ENABLE_DP_TRACE_DEFAULT,
		CFG_ENABLE_DP_TRACE_MIN,
		CFG_ENABLE_DP_TRACE_MAX),


	REG_VARIABLE_STRING(CFG_ENABLE_DP_TRACE_CONFIG, WLAN_PARAM_String,
			struct hdd_config, dp_trace_config,
			VAR_FLAGS_OPTIONAL,
			(void *) CFG_ENABLE_DP_TRACE_CONFIG_DEFAULT),
#endif

	REG_VARIABLE(CFG_ADAPTIVE_SCAN_DWELL_MODE_NAME, WLAN_PARAM_Integer,
		struct hdd_config, scan_adaptive_dwell_mode,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ADAPTIVE_SCAN_DWELL_MODE_DEFAULT,
		CFG_ADAPTIVE_SCAN_DWELL_MODE_MIN,
		CFG_ADAPTIVE_SCAN_DWELL_MODE_MAX),

	REG_VARIABLE(CFG_ADAPTIVE_SCAN_DWELL_MODE_NC_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, scan_adaptive_dwell_mode_nc,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ADAPTIVE_SCAN_DWELL_MODE_NC_DEFAULT,
		     CFG_ADAPTIVE_SCAN_DWELL_MODE_NC_MIN,
		     CFG_ADAPTIVE_SCAN_DWELL_MODE_NC_MAX),

	REG_VARIABLE(CFG_ADAPTIVE_ROAMSCAN_DWELL_MODE_NAME, WLAN_PARAM_Integer,
		struct hdd_config, roamscan_adaptive_dwell_mode,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ADAPTIVE_ROAMSCAN_DWELL_MODE_DEFAULT,
		CFG_ADAPTIVE_ROAMSCAN_DWELL_MODE_MIN,
		CFG_ADAPTIVE_ROAMSCAN_DWELL_MODE_MAX),

	REG_VARIABLE(CFG_ADAPTIVE_EXTSCAN_DWELL_MODE_NAME, WLAN_PARAM_Integer,
		struct hdd_config, extscan_adaptive_dwell_mode,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ADAPTIVE_EXTSCAN_DWELL_MODE_DEFAULT,
		CFG_ADAPTIVE_EXTSCAN_DWELL_MODE_MIN,
		CFG_ADAPTIVE_EXTSCAN_DWELL_MODE_MAX),

	REG_VARIABLE(CFG_ADAPTIVE_DWELL_MODE_ENABLED_NAME, WLAN_PARAM_Integer,
		struct hdd_config, adaptive_dwell_mode_enabled,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ADAPTIVE_DWELL_MODE_ENABLED_DEFAULT,
		CFG_ADAPTIVE_DWELL_MODE_ENABLED_MIN,
		CFG_ADAPTIVE_DWELL_MODE_ENABLED_MAX),

	REG_VARIABLE(CFG_GLOBAL_ADAPTIVE_DWELL_MODE_NAME, WLAN_PARAM_Integer,
		struct hdd_config, global_adapt_dwelltime_mode,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_GLOBAL_ADAPTIVE_DWELL_MODE_DEFAULT,
		CFG_GLOBAL_ADAPTIVE_DWELL_MODE_MIN,
		CFG_GLOBAL_ADAPTIVE_DWELL_MODE_MAX),

	REG_VARIABLE(CFG_ADAPT_DWELL_LPF_WEIGHT_NAME, WLAN_PARAM_Integer,
		struct hdd_config, adapt_dwell_lpf_weight,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ADAPT_DWELL_LPF_WEIGHT_DEFAULT,
		CFG_ADAPT_DWELL_LPF_WEIGHT_MIN,
		CFG_ADAPT_DWELL_LPF_WEIGHT_MAX),

	REG_VARIABLE(CFG_SUB_20_CHANNEL_WIDTH_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, enable_sub_20_channel_width,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_SUB_20_CHANNEL_WIDTH_DEFAULT,
		     CFG_SUB_20_CHANNEL_WIDTH_MIN,
		     CFG_SUB_20_CHANNEL_WIDTH_MAX),

	REG_VARIABLE(CFG_TGT_GTX_USR_CFG_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, tgt_gtx_usr_cfg,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TGT_GTX_USR_CFG_DEFAULT,
		     CFG_TGT_GTX_USR_CFG_MIN,
		     CFG_TGT_GTX_USR_CFG_MAX),

	REG_VARIABLE(CFG_ADAPT_DWELL_PASMON_INTVAL_NAME, WLAN_PARAM_Integer,
		struct hdd_config, adapt_dwell_passive_mon_intval,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ADAPT_DWELL_PASMON_INTVAL_DEFAULT,
		CFG_ADAPT_DWELL_PASMON_INTVAL_MIN,
		CFG_ADAPT_DWELL_PASMON_INTVAL_MAX),

	REG_VARIABLE(CFG_ADAPT_DWELL_WIFI_THRESH_NAME, WLAN_PARAM_Integer,
		struct hdd_config, adapt_dwell_wifi_act_threshold,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ADAPT_DWELL_WIFI_THRESH_DEFAULT,
		CFG_ADAPT_DWELL_WIFI_THRESH_MIN,
		CFG_ADAPT_DWELL_WIFI_THRESH_MAX),

	REG_VARIABLE(CFG_RX_MODE_NAME, WLAN_PARAM_Integer,
		struct hdd_config, rx_mode,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_RX_MODE_DEFAULT,
		CFG_RX_MODE_MIN,
		CFG_RX_MODE_MAX),

	REG_VARIABLE(CFG_NUM_DP_RX_THREADS_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, num_dp_rx_threads,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_NUM_DP_RX_THREADS_DEFAULT,
		     CFG_NUM_DP_RX_THREADS_MIN,
		     CFG_NUM_DP_RX_THREADS_MAX),

	REG_VARIABLE(CFG_CE_SERVICE_MAX_YIELD_TIME_NAME, WLAN_PARAM_Integer,
		struct hdd_config, ce_service_max_yield_time,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_CE_SERVICE_MAX_YIELD_TIME_DEFAULT,
		CFG_CE_SERVICE_MAX_YIELD_TIME_MIN,
		CFG_CE_SERVICE_MAX_YIELD_TIME_MAX),

	REG_VARIABLE(CFG_CE_SERVICE_MAX_RX_IND_FLUSH_NAME, WLAN_PARAM_Integer,
		struct hdd_config, ce_service_max_rx_ind_flush,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_CE_SERVICE_MAX_RX_IND_FLUSH_DEFAULT,
		CFG_CE_SERVICE_MAX_RX_IND_FLUSH_MIN,
		CFG_CE_SERVICE_MAX_RX_IND_FLUSH_MAX),

	REG_VARIABLE(CFG_NAPI_CE_CPU_MASK_NAME, WLAN_PARAM_HexInteger,
		struct hdd_config, napi_cpu_affinity_mask,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_NAPI_CE_CPU_MASK_DEFAULT,
		CFG_NAPI_CE_CPU_MASK_MIN,
		CFG_NAPI_CE_CPU_MASK_MAX),

	REG_VARIABLE(CFG_RX_THREAD_CPU_MASK_NAME, WLAN_PARAM_HexInteger,
		struct hdd_config, rx_thread_affinity_mask,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_RX_THREAD_CPU_MASK_DEFAULT,
		CFG_RX_THREAD_CPU_MASK_MIN,
		CFG_RX_THREAD_CPU_MASK_MAX),

	REG_VARIABLE_STRING(CFG_RPS_RX_QUEUE_CPU_MAP_LIST_NAME,
				 WLAN_PARAM_String,
				 struct hdd_config, cpu_map_list,
				 VAR_FLAGS_OPTIONAL,
				 (void *)CFG_RPS_RX_QUEUE_CPU_MAP_LIST_DEFAULT),

	REG_VARIABLE(CFG_INDOOR_CHANNEL_SUPPORT_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, indoor_channel_support,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_INDOOR_CHANNEL_SUPPORT_DEFAULT,
		     CFG_INDOOR_CHANNEL_SUPPORT_MIN,
		     CFG_INDOOR_CHANNEL_SUPPORT_MAX),

	REG_VARIABLE(CFG_MARK_INDOOR_AS_DISABLE_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, force_ssc_disable_indoor_channel,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_MARK_INDOOR_AS_DISABLE_DEFAULT,
		     CFG_MARK_INDOOR_AS_DISABLE_MIN,
		     CFG_MARK_INDOOR_AS_DISABLE_MAX),

	REG_VARIABLE(CFG_SAP_TX_LEAKAGE_THRESHOLD_NAME,
		WLAN_PARAM_Integer,
		struct hdd_config, sap_tx_leakage_threshold,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_SAP_TX_LEAKAGE_THRESHOLD_DEFAULT,
		CFG_SAP_TX_LEAKAGE_THRESHOLD_MIN,
		CFG_SAP_TX_LEAKAGE_THRESHOLD_MAX),

	REG_VARIABLE(CFG_BUG_ON_REINIT_FAILURE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, bug_on_reinit_failure,
		     VAR_FLAGS_OPTIONAL |
		     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_BUG_ON_REINIT_FAILURE_DEFAULT,
		     CFG_BUG_ON_REINIT_FAILURE_MIN,
		     CFG_BUG_ON_REINIT_FAILURE_MAX),

	REG_VARIABLE(CFG_SAP_FORCE_11N_FOR_11AC_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, sap_force_11n_for_11ac,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_SAP_FORCE_11N_FOR_11AC_DEFAULT,
		     CFG_SAP_FORCE_11N_FOR_11AC_MIN,
		     CFG_SAP_FORCE_11N_FOR_11AC_MAX),

	REG_VARIABLE(CFG_GO_FORCE_11N_FOR_11AC_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, go_force_11n_for_11ac,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_GO_FORCE_11N_FOR_11AC_DEFAULT,
		     CFG_GO_FORCE_11N_FOR_11AC_MIN,
		     CFG_GO_FORCE_11N_FOR_11AC_MAX),

	REG_VARIABLE(CFG_INTERFACE_CHANGE_WAIT_NAME, WLAN_PARAM_Integer,
			struct hdd_config, iface_change_wait_time,
			VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
			CFG_INTERFACE_CHANGE_WAIT_DEFAULT,
			CFG_INTERFACE_CHANGE_WAIT_MIN,
			CFG_INTERFACE_CHANGE_WAIT_MAX),

	REG_VARIABLE(CFG_FILTER_MULTICAST_REPLAY_NAME,
		WLAN_PARAM_Integer,
		struct hdd_config, multicast_replay_filter,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_FILTER_MULTICAST_REPLAY_DEFAULT,
		CFG_FILTER_MULTICAST_REPLAY_MIN,
		CFG_FILTER_MULTICAST_REPLAY_MAX),

	REG_VARIABLE(CFG_ENABLE_PHY_REG, WLAN_PARAM_HexInteger,
		     struct hdd_config, enable_phy_reg_retention,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_PHY_REG_DEFAULT,
		     CFG_PHY_REG_MIN,
		     CFG_PHY_REG_MAX),

	REG_VARIABLE(CFG_OPTIMIZE_CA_EVENT_NAME, WLAN_PARAM_Integer,
			struct hdd_config, goptimize_chan_avoid_event,
			VAR_FLAGS_OPTIONAL |
			VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			CFG_OPTIMIZE_CA_EVENT_DEFAULT,
			CFG_OPTIMIZE_CA_EVENT_DISABLE,
			CFG_OPTIMIZE_CA_EVENT_ENABLE),

	REG_VARIABLE(CFG_TX_AGGREGATION_SIZE, WLAN_PARAM_Integer,
		     struct hdd_config, tx_aggregation_size,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TX_AGGREGATION_SIZE_DEFAULT,
		     CFG_TX_AGGREGATION_SIZE_MIN,
		     CFG_TX_AGGREGATION_SIZE_MAX),

	REG_VARIABLE(CFG_TX_AGGREGATION_SIZEBE, WLAN_PARAM_Integer,
		     struct hdd_config, tx_aggregation_size_be,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TX_AGGREGATION_SIZEBE_DEFAULT,
		     CFG_TX_AGGREGATION_SIZEBE_MIN,
		     CFG_TX_AGGREGATION_SIZEBE_MAX),

	REG_VARIABLE(CFG_TX_AGGREGATION_SIZEBK, WLAN_PARAM_Integer,
		     struct hdd_config, tx_aggregation_size_bk,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TX_AGGREGATION_SIZEBK_DEFAULT,
		     CFG_TX_AGGREGATION_SIZEBK_MIN,
		     CFG_TX_AGGREGATION_SIZEBK_MAX),

	REG_VARIABLE(CFG_TX_AGGREGATION_SIZEVI, WLAN_PARAM_Integer,
		     struct hdd_config, tx_aggregation_size_vi,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TX_AGGREGATION_SIZEVI_DEFAULT,
		     CFG_TX_AGGREGATION_SIZEVI_MIN,
		     CFG_TX_AGGREGATION_SIZEVI_MAX),

	REG_VARIABLE(CFG_TX_AGGREGATION_SIZEVO, WLAN_PARAM_Integer,
		     struct hdd_config, tx_aggregation_size_vo,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TX_AGGREGATION_SIZEVO_DEFAULT,
		     CFG_TX_AGGREGATION_SIZEVO_MIN,
		     CFG_TX_AGGREGATION_SIZEVO_MAX),

	REG_VARIABLE(CFG_RX_AGGREGATION_SIZE, WLAN_PARAM_Integer,
		struct hdd_config, rx_aggregation_size,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_RX_AGGREGATION_SIZE_DEFAULT,
		CFG_RX_AGGREGATION_SIZE_MIN,
		CFG_RX_AGGREGATION_SIZE_MAX),

	REG_VARIABLE(CFG_TX_AGGR_SW_RETRY_BE, WLAN_PARAM_Integer,
		     struct hdd_config, tx_aggr_sw_retry_threshold_be,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TX_AGGR_SW_RETRY_BE_DEFAULT,
		     CFG_TX_AGGR_SW_RETRY_BE_MIN,
		     CFG_TX_AGGR_SW_RETRY_BE_MAX),

	REG_VARIABLE(CFG_TX_AGGR_SW_RETRY_BK, WLAN_PARAM_Integer,
		     struct hdd_config, tx_aggr_sw_retry_threshold_bk,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TX_AGGR_SW_RETRY_BK_DEFAULT,
		     CFG_TX_AGGR_SW_RETRY_BK_MIN,
		     CFG_TX_AGGR_SW_RETRY_BK_MAX),

	REG_VARIABLE(CFG_TX_AGGR_SW_RETRY_VI, WLAN_PARAM_Integer,
		     struct hdd_config, tx_aggr_sw_retry_threshold_vi,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TX_AGGR_SW_RETRY_VI_DEFAULT,
		     CFG_TX_AGGR_SW_RETRY_VI_MIN,
		     CFG_TX_AGGR_SW_RETRY_VI_MAX),

	REG_VARIABLE(CFG_TX_AGGR_SW_RETRY_VO, WLAN_PARAM_Integer,
		     struct hdd_config, tx_aggr_sw_retry_threshold_vo,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TX_AGGR_SW_RETRY_VO_DEFAULT,
		     CFG_TX_AGGR_SW_RETRY_VO_MIN,
		     CFG_TX_AGGR_SW_RETRY_VO_MAX),

	REG_VARIABLE(CFG_SAP_MAX_INACTIVITY_OVERRIDE_NAME, WLAN_PARAM_Integer,
		struct hdd_config, sap_max_inactivity_override,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_SAP_MAX_INACTIVITY_OVERRIDE_DEFAULT,
		CFG_SAP_MAX_INACTIVITY_OVERRIDE_MIN,
		CFG_SAP_MAX_INACTIVITY_OVERRIDE_MAX),
	REG_VARIABLE(CFG_CRASH_FW_TIMEOUT_NAME, WLAN_PARAM_Integer,
		struct hdd_config, fw_timeout_crash,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_CRASH_FW_TIMEOUT_DEFAULT,
		CFG_CRASH_FW_TIMEOUT_DISABLE,
		CFG_CRASH_FW_TIMEOUT_ENABLE),
	REG_VARIABLE(CFG_RX_WAKELOCK_TIMEOUT_NAME, WLAN_PARAM_Integer,
		struct hdd_config, rx_wakelock_timeout,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_RX_WAKELOCK_TIMEOUT_DEFAULT,
		CFG_RX_WAKELOCK_TIMEOUT_MIN,
		CFG_RX_WAKELOCK_TIMEOUT_MAX),
	REG_VARIABLE(CFG_SAP_CH_SWITCH_BEACON_CNT, WLAN_PARAM_Integer,
		     struct hdd_config, sap_chanswitch_beacon_cnt,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_SAP_CH_SWITCH_BEACON_CNT_DEFAULT,
		     CFG_SAP_CH_SWITCH_BEACON_CNT_MIN,
		     CFG_SAP_CH_SWITCH_BEACON_CNT_MAX),
	REG_VARIABLE(CFG_SAP_CH_SWITCH_MODE, WLAN_PARAM_Integer,
		     struct hdd_config, sap_chanswitch_mode,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_SAP_CH_SWITCH_MODE_DEFAULT,
		     CFG_SAP_CH_SWITCH_MODE_MIN,
		     CFG_SAP_CH_SWITCH_MODE_MAX),
	REG_VARIABLE(CFG_MAX_SCHED_SCAN_PLAN_INT_NAME, WLAN_PARAM_Integer,
		struct hdd_config, max_sched_scan_plan_interval,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_MAX_SCHED_SCAN_PLAN_INT_DEFAULT,
		CFG_MAX_SCHED_SCAN_PLAN_INT_MIN,
		CFG_MAX_SCHED_SCAN_PLAN_INT_MAX),
	REG_VARIABLE(CFG_MAX_SCHED_SCAN_PLAN_ITRNS_NAME, WLAN_PARAM_Integer,
		struct hdd_config, max_sched_scan_plan_iterations,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_MAX_SCHED_SCAN_PLAN_ITRNS_DEFAULT,
		CFG_MAX_SCHED_SCAN_PLAN_ITRNS_MIN,
		CFG_MAX_SCHED_SCAN_PLAN_ITRNS_MAX),

	REG_VARIABLE(CFG_PRIVATE_WEXT_CONTROL_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, private_wext_control,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_PRIVATE_WEXT_CONTROL_DEFAULT,
		     CFG_PRIVATE_WEXT_CONTROL_MIN,
		     CFG_PRIVATE_WEXT_CONTROL_MAX),

	REG_VARIABLE(CFG_SAP_INTERNAL_RESTART_NAME, WLAN_PARAM_Integer,
		struct hdd_config, sap_internal_restart,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_SAP_INTERNAL_RESTART_DEFAULT,
		CFG_SAP_INTERNAL_RESTART_MIN,
		CFG_SAP_INTERNAL_RESTART_MAX),

	REG_VARIABLE(CFG_RESTART_BEACONING_ON_CH_AVOID_NAME, WLAN_PARAM_Integer,
		struct hdd_config, restart_beaconing_on_chan_avoid_event,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_RESTART_BEACONING_ON_CH_AVOID_DEFAULT,
		CFG_RESTART_BEACONING_ON_CH_AVOID_MIN,
		CFG_RESTART_BEACONING_ON_CH_AVOID_MAX),

	REG_VARIABLE(CFG_PER_ROAM_ENABLE_NAME, WLAN_PARAM_Integer,
		struct hdd_config, is_per_roam_enabled,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_PER_ROAM_ENABLE_DEFAULT,
		CFG_PER_ROAM_ENABLE_MIN,
		CFG_PER_ROAM_ENABLE_MAX),

	REG_VARIABLE(CFG_PER_ROAM_CONFIG_HIGH_RATE_TH_NAME, WLAN_PARAM_Integer,
		struct hdd_config, per_roam_high_rate_threshold,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_PER_ROAM_CONFIG_HIGH_RATE_TH_DEFAULT,
		CFG_PER_ROAM_CONFIG_HIGH_RATE_TH_MIN,
		CFG_PER_ROAM_CONFIG_HIGH_RATE_TH_MAX),

	REG_VARIABLE(CFG_PER_ROAM_CONFIG_LOW_RATE_TH_NAME, WLAN_PARAM_Integer,
		struct hdd_config, per_roam_low_rate_threshold,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_PER_ROAM_CONFIG_LOW_RATE_TH_DEFAULT,
		CFG_PER_ROAM_CONFIG_LOW_RATE_TH_MIN,
		CFG_PER_ROAM_CONFIG_LOW_RATE_TH_MAX),

	REG_VARIABLE(CFG_PER_ROAM_CONFIG_RATE_TH_PERCENT_NAME,
		WLAN_PARAM_Integer, struct hdd_config, per_roam_th_percent,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_PER_ROAM_CONFIG_RATE_TH_PERCENT_DEFAULT,
		CFG_PER_ROAM_CONFIG_RATE_TH_PERCENT_MIN,
		CFG_PER_ROAM_CONFIG_RATE_TH_PERCENT_MAX),

	REG_VARIABLE(CFG_PER_ROAM_REST_TIME_NAME, WLAN_PARAM_Integer,
		struct hdd_config, per_roam_rest_time,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_PER_ROAM_REST_TIME_DEFAULT,
		CFG_PER_ROAM_REST_TIME_MIN,
		CFG_PER_ROAM_REST_TIME_MAX),

	REG_VARIABLE(CFG_PER_ROAM_MONITOR_TIME, WLAN_PARAM_Integer,
		struct hdd_config, per_roam_mon_time,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_PER_ROAM_MONTIOR_TIME_DEFAULT,
		CFG_PER_ROAM_MONITOR_TIME_MIN,
		CFG_PER_ROAM_MONITOR_TIME_MAX),

	REG_VARIABLE(CFG_PER_ROAM_MIN_CANDIDATE_RSSI, WLAN_PARAM_Integer,
		struct hdd_config, min_candidate_rssi,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_PER_ROAM_MIN_CANDIDATE_RSSI_DEFAULT,
		CFG_PER_ROAM_MIN_CANDIDATE_RSSI_MIN,
		CFG_PER_ROAM_MIN_CANDIDATE_RSSI_MAX),

	REG_VARIABLE(CFG_ACTIVE_UC_APF_MODE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, active_uc_apf_mode,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ACTIVE_UC_APF_MODE_DEFAULT,
		     CFG_ACTIVE_UC_APF_MODE_MIN,
		     CFG_ACTIVE_UC_APF_MODE_MAX),

	REG_VARIABLE(CFG_ACTIVE_MC_BC_APF_MODE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, active_mc_bc_apf_mode,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ACTIVE_MC_BC_APF_MODE_DEFAULT,
		     CFG_ACTIVE_MC_BC_APF_MODE_MIN,
		     CFG_ACTIVE_MC_BC_APF_MODE_MAX),

	REG_VARIABLE(CFG_ENABLE_BCAST_PROBE_RESP_NAME, WLAN_PARAM_Integer,
		struct hdd_config, enable_bcast_probe_rsp,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ENABLE_BCAST_PROBE_RESP_DEFAULT,
		CFG_ENABLE_BCAST_PROBE_RESP_MIN,
		CFG_ENABLE_BCAST_PROBE_RESP_MAX),

#ifdef WLAN_FEATURE_11AX
	REG_VARIABLE(CFG_ENABLE_UL_MIMO_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, enable_ul_mimo,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_UL_MIMO_DEFAULT,
		     CFG_ENABLE_UL_MIMO_MIN,
		     CFG_ENABLE_UL_MIMO_MAX),

	REG_VARIABLE(CFG_HE_DYNAMIC_FRAGMENTATION_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, he_dynamic_frag_support,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_HE_DYNAMIC_FRAGMENTATION_DEFAULT,
		     CFG_HE_DYNAMIC_FRAGMENTATION_MIN,
		     CFG_HE_DYNAMIC_FRAGMENTATION_MAX),

	REG_VARIABLE(CFG_ENABLE_UL_OFDMA_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, enable_ul_ofdma,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_UL_OFDMA_DEFAULT,
		     CFG_ENABLE_UL_OFDMA_MIN,
		     CFG_ENABLE_UL_OFDMA_MAX),

	REG_VARIABLE(CFG_HE_STA_OBSSPD_NAME, WLAN_PARAM_HexInteger,
		     struct hdd_config, he_sta_obsspd,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_HE_STA_OBSSPD_DEFAULT,
		     CFG_HE_STA_OBSSPD_MIN,
		     CFG_HE_STA_OBSSPD_MAX),
#endif
#ifdef WLAN_SUPPORT_TWT
	REG_VARIABLE(CFG_ENABLE_TWT_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, enable_twt,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_TWT_DEFAULT,
		     CFG_ENABLE_TWT_MIN,
		     CFG_ENABLE_TWT_MAX),

	REG_VARIABLE(CFG_TWT_CONGESTION_TIMEOUT_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, twt_congestion_timeout,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TWT_CONGESTION_TIMEOUT_DEFAULT,
		     CFG_TWT_CONGESTION_TIMEOUT_MIN,
		     CFG_TWT_CONGESTION_TIMEOUT_MAX),
#endif

	REG_VARIABLE(CFG_ARP_AC_CATEGORY, WLAN_PARAM_Integer,
		struct hdd_config, arp_ac_category,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ARP_AC_CATEGORY_DEFAULT,
		CFG_ARP_AC_CATEGORY_MIN,
		CFG_ARP_AC_CATEGORY_MAX),

	REG_VARIABLE(CFG_ENABLE_ANI_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, ani_enabled,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_ANI_DEFAULT,
		     CFG_ENABLE_ANI_MIN,
		     CFG_ENABLE_ANI_MAX),

	REG_VARIABLE(CFG_QCN_IE_SUPPORT_NAME, WLAN_PARAM_Integer,
		struct hdd_config, qcn_ie_support,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_QCN_IE_SUPPORT_DEFAULT,
		CFG_QCN_IE_SUPPORT_MIN,
		CFG_QCN_IE_SUPPORT_MAX),

	REG_VARIABLE(CFG_TIMER_MULTIPLIER_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, timer_multiplier,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TIMER_MULTIPLIER_DEFAULT,
		     CFG_TIMER_MULTIPLIER_MIN,
		     CFG_TIMER_MULTIPLIER_MAX),

	REG_VARIABLE(CFG_PRB_REQ_IE_WHITELIST_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, probe_req_ie_whitelist,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_PRB_REQ_IE_WHITELIST_DEFAULT,
		     CFG_PRB_REQ_IE_WHITELIST_MIN,
		     CFG_PRB_REQ_IE_WHITELIST_MAX),

	REG_VARIABLE(CFG_PRB_REQ_IE_BIT_MAP0_NAME, WLAN_PARAM_HexInteger,
		     struct hdd_config, probe_req_ie_bitmap_0,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_PRB_REQ_IE_BIT_MAP0_DEFAULT,
		     CFG_PRB_REQ_IE_BIT_MAP0_MIN,
		     CFG_PRB_REQ_IE_BIT_MAP0_MAX),

	REG_VARIABLE(CFG_PRB_REQ_IE_BIT_MAP1_NAME, WLAN_PARAM_HexInteger,
		     struct hdd_config, probe_req_ie_bitmap_1,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_PRB_REQ_IE_BIT_MAP1_DEFAULT,
		     CFG_PRB_REQ_IE_BIT_MAP1_MIN,
		     CFG_PRB_REQ_IE_BIT_MAP1_MAX),

	REG_VARIABLE(CFG_PRB_REQ_IE_BIT_MAP2_NAME, WLAN_PARAM_HexInteger,
		     struct hdd_config, probe_req_ie_bitmap_2,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_PRB_REQ_IE_BIT_MAP2_DEFAULT,
		     CFG_PRB_REQ_IE_BIT_MAP2_MIN,
		     CFG_PRB_REQ_IE_BIT_MAP2_MAX),

	REG_VARIABLE(CFG_PRB_REQ_IE_BIT_MAP3_NAME, WLAN_PARAM_HexInteger,
		     struct hdd_config, probe_req_ie_bitmap_3,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_PRB_REQ_IE_BIT_MAP3_DEFAULT,
		     CFG_PRB_REQ_IE_BIT_MAP3_MIN,
		     CFG_PRB_REQ_IE_BIT_MAP3_MAX),

	REG_VARIABLE(CFG_PRB_REQ_IE_BIT_MAP4_NAME, WLAN_PARAM_HexInteger,
		     struct hdd_config, probe_req_ie_bitmap_4,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_PRB_REQ_IE_BIT_MAP4_DEFAULT,
		     CFG_PRB_REQ_IE_BIT_MAP4_MIN,
		     CFG_PRB_REQ_IE_BIT_MAP4_MAX),

	REG_VARIABLE(CFG_PRB_REQ_IE_BIT_MAP5_NAME, WLAN_PARAM_HexInteger,
		     struct hdd_config, probe_req_ie_bitmap_5,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_PRB_REQ_IE_BIT_MAP5_DEFAULT,
		     CFG_PRB_REQ_IE_BIT_MAP5_MIN,
		     CFG_PRB_REQ_IE_BIT_MAP5_MAX),

	REG_VARIABLE(CFG_PRB_REQ_IE_BIT_MAP6_NAME, WLAN_PARAM_HexInteger,
		     struct hdd_config, probe_req_ie_bitmap_6,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_PRB_REQ_IE_BIT_MAP6_DEFAULT,
		     CFG_PRB_REQ_IE_BIT_MAP6_MIN,
		     CFG_PRB_REQ_IE_BIT_MAP6_MAX),

	REG_VARIABLE(CFG_PRB_REQ_IE_BIT_MAP7_NAME, WLAN_PARAM_HexInteger,
		     struct hdd_config, probe_req_ie_bitmap_7,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_PRB_REQ_IE_BIT_MAP7_DEFAULT,
		     CFG_PRB_REQ_IE_BIT_MAP7_MIN,
		     CFG_PRB_REQ_IE_BIT_MAP7_MAX),

	REG_VARIABLE_STRING(CFG_PROBE_REQ_OUI_NAME, WLAN_PARAM_String,
			    struct hdd_config, probe_req_ouis,
			    VAR_FLAGS_OPTIONAL,
			    (void *)CFG_PROBE_REQ_OUI_DEFAULT),

	REG_VARIABLE(CFG_FILS_MAX_CHAN_GUARD_TIME_NAME, WLAN_PARAM_Integer,
		struct hdd_config, fils_max_chan_guard_time,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_FILS_MAX_CHAN_GUARD_TIME_DEFAULT,
		CFG_FILS_MAX_CHAN_GUARD_TIME_MIN,
		CFG_FILS_MAX_CHAN_GUARD_TIME_MAX),

	REG_VARIABLE(CFG_SCAN_BACKOFF_MULTIPLIER_NAME, WLAN_PARAM_Integer,
		struct hdd_config, scan_backoff_multiplier,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_SCAN_BACKOFF_MULTIPLIER_DEFAULT,
		CFG_SCAN_BACKOFF_MULTIPLIER_MIN,
		CFG_SCAN_BACKOFF_MULTIPLIER_MAX),

	REG_VARIABLE(CFG_MAWC_NLO_ENABLED_NAME, WLAN_PARAM_Integer,
		struct hdd_config, mawc_nlo_enabled,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_MAWC_NLO_ENABLED_DEFAULT,
		CFG_MAWC_NLO_ENABLED_MIN,
		CFG_MAWC_NLO_ENABLED_MAX),

	REG_VARIABLE(CFG_MAWC_NLO_EXP_BACKOFF_RATIO_NAME, WLAN_PARAM_Integer,
		struct hdd_config, mawc_nlo_exp_backoff_ratio,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_MAWC_NLO_EXP_BACKOFF_RATIO_DEFAULT,
		CFG_MAWC_NLO_EXP_BACKOFF_RATIO_MIN,
		CFG_MAWC_NLO_EXP_BACKOFF_RATIO_MAX),

	REG_VARIABLE(CFG_MAWC_NLO_INIT_SCAN_INTERVAL_NAME, WLAN_PARAM_Integer,
		struct hdd_config, mawc_nlo_init_scan_interval,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_MAWC_NLO_INIT_SCAN_INTERVAL_DEFAULT,
		CFG_MAWC_NLO_INIT_SCAN_INTERVAL_MIN,
		CFG_MAWC_NLO_INIT_SCAN_INTERVAL_MAX),

	REG_VARIABLE(CFG_MAWC_NLO_MAX_SCAN_INTERVAL_NAME, WLAN_PARAM_Integer,
		struct hdd_config, mawc_nlo_max_scan_interval,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_MAWC_NLO_MAX_SCAN_INTERVAL_DEFAULT,
		CFG_MAWC_NLO_MAX_SCAN_INTERVAL_MIN,
		CFG_MAWC_NLO_MAX_SCAN_INTERVAL_MAX),


	REG_VARIABLE(CFG_EXTERNAL_ACS_POLICY, WLAN_PARAM_Integer,
		     struct hdd_config, external_acs_policy,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_EXTERNAL_ACS_POLICY_DEFAULT,
		     CFG_EXTERNAL_ACS_POLICY_MIN,
		     CFG_EXTERNAL_ACS_POLICY_MAX),

	REG_VARIABLE(CFG_DROPPED_PKT_DISCONNECT_TH_NAME, WLAN_PARAM_Integer,
		struct hdd_config, pkt_err_disconn_th,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_DROPPED_PKT_DISCONNECT_TH_DEFAULT,
		CFG_DROPPED_PKT_DISCONNECT_TH_MIN,
		CFG_DROPPED_PKT_DISCONNECT_TH_MAX),

	REG_VARIABLE(CFG_FORCE_1X1_NAME, WLAN_PARAM_Integer,
		struct hdd_config, is_force_1x1,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_FORCE_1X1_DEFAULT,
		CFG_FORCE_1X1_MIN,
		CFG_FORCE_1X1_MAX),

	REG_VARIABLE(CFG_SET_RTS_FOR_SIFS_BURSTING, WLAN_PARAM_Integer,
		struct hdd_config, enable_rts_sifsbursting,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_SET_RTS_FOR_SIFS_BURSTING_DEFAULT,
		CFG_SET_RTS_FOR_SIFS_BURSTING_MIN,
		CFG_SET_RTS_FOR_SIFS_BURSTING_MAX),

	REG_VARIABLE(CFG_MAX_MPDUS_IN_AMPDU, WLAN_PARAM_Integer,
		struct hdd_config, max_mpdus_inampdu,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_MAX_MPDUS_IN_AMPDU_DEFAULT,
		CFG_MAX_MPDUS_IN_AMPDU_MIN,
		CFG_MAX_MPDUS_IN_AMPDU_MAX),

	REG_VARIABLE(CFG_TX_ORPHAN_ENABLE_NAME, WLAN_PARAM_Integer,
		struct hdd_config, tx_orphan_enable,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_TX_ORPHAN_ENABLE_DEFAULT,
		CFG_TX_ORPHAN_ENABLE_MIN,
		CFG_TX_ORPHAN_ENABLE_MAX),

	REG_VARIABLE(CFG_AUTO_DETECT_POWER_FAIL_MODE_NAME, WLAN_PARAM_Integer,
		struct hdd_config, auto_pwr_save_fail_mode,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_AUTO_DETECT_POWER_FAIL_MODE_DEFAULT,
		CFG_AUTO_DETECT_POWER_FAIL_MODE_MIN,
		CFG_AUTO_DETECT_POWER_FAIL_MODE_MAX),

	REG_VARIABLE(CFG_ITO_REPEAT_COUNT_NAME, WLAN_PARAM_Integer,
		struct hdd_config, ito_repeat_count,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ITO_REPEAT_COUNT_DEFAULT,
		CFG_ITO_REPEAT_COUNT_MIN,
		CFG_ITO_REPEAT_COUNT_MAX),

	REG_VARIABLE(CFG_ROAM_DISALLOW_DURATION_NAME, WLAN_PARAM_Integer,
		struct hdd_config, disallow_duration,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ROAM_DISALLOW_DURATION_DEFAULT,
		CFG_ROAM_DISALLOW_DURATION_MIN,
		CFG_ROAM_DISALLOW_DURATION_MAX),

	REG_VARIABLE(CFG_ROAM_RSSI_CHANNEL_PENALIZATION_NAME,
		WLAN_PARAM_Integer, struct hdd_config,
		rssi_channel_penalization,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ROAM_RSSI_CHANNEL_PENALIZATION_DEFAULT,
		CFG_ROAM_RSSI_CHANNEL_PENALIZATION_MIN,
		CFG_ROAM_RSSI_CHANNEL_PENALIZATION_MAX),

	REG_VARIABLE(CFG_ROAM_NUM_DISALLOWED_APS_NAME, WLAN_PARAM_Integer,
		struct hdd_config, num_disallowed_aps,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ROAM_NUM_DISALLOWED_APS_DEFAULT,
		CFG_ROAM_NUM_DISALLOWED_APS_MIN,
		CFG_ROAM_NUM_DISALLOWED_APS_MAX),

	REG_VARIABLE(CFG_OCE_ENABLE_STA_NAME, WLAN_PARAM_Integer,
		struct hdd_config, oce_sta_enabled,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_OCE_ENABLE_STA_DEFAULT,
		CFG_OCE_ENABLE_STA_MIN,
		CFG_OCE_ENABLE_STA_MAX),

	REG_VARIABLE(CFG_OCE_ENABLE_SAP_NAME, WLAN_PARAM_Integer,
		struct hdd_config, oce_sap_enabled,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_OCE_ENABLE_SAP_DEFAULT,
		CFG_OCE_ENABLE_SAP_MIN,
		CFG_OCE_ENABLE_SAP_MAX),

	REG_VARIABLE(CFG_ENABLE_11D_IN_WORLD_MODE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, enable_11d_in_world_mode,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_11D_IN_WORLD_MODE_DEFAULT,
		     CFG_ENABLE_11D_IN_WORLD_MODE_MIN,
		     CFG_ENABLE_11D_IN_WORLD_MODE_MAX),

	REG_VARIABLE(CFG_ENABLE_5G_BAND_PREF_NAME, WLAN_PARAM_Integer,
		struct hdd_config, enable_5g_band_pref,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ENABLE_5G_BAND_PREF_DEFAULT,
		CFG_ENABLE_5G_BAND_PREF_MIN,
		CFG_ENABLE_5G_BAND_PREF_MAX),

	REG_VARIABLE(CFG_5G_RSSI_BOOST_THRESHOLD_NAME, WLAN_PARAM_Integer,
		struct hdd_config, rssi_boost_threshold_5g,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_5G_RSSI_BOOST_THRESHOLD_DEFAULT,
		CFG_5G_RSSI_BOOST_THRESHOLD_MIN,
		CFG_5G_RSSI_BOOST_THRESHOLD_MAX),

	REG_VARIABLE(CFG_5G_RSSI_BOOST_FACTOR_NAME, WLAN_PARAM_Integer,
		struct hdd_config, rssi_boost_factor_5g,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_5G_RSSI_BOOST_FACTOR_DEFAULT,
		CFG_5G_RSSI_BOOST_FACTOR_MIN,
		CFG_5G_RSSI_BOOST_FACTOR_MAX),

	REG_VARIABLE(CFG_5G_MAX_RSSI_BOOST_NAME, WLAN_PARAM_Integer,
		struct hdd_config, max_rssi_boost_5g,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_5G_MAX_RSSI_BOOST_DEFAULT,
		CFG_5G_MAX_RSSI_BOOST_MIN,
		CFG_5G_MAX_RSSI_BOOST_MAX),

	REG_VARIABLE(CFG_5G_RSSI_PENALIZE_THRESHOLD_NAME, WLAN_PARAM_Integer,
		struct hdd_config, rssi_penalize_threshold_5g,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_5G_RSSI_PENALIZE_THRESHOLD_DEFAULT,
		CFG_5G_RSSI_PENALIZE_THRESHOLD_MIN,
		CFG_5G_RSSI_PENALIZE_THRESHOLD_MAX),

	REG_VARIABLE(CFG_5G_RSSI_PENALIZE_FACTOR_NAME, WLAN_PARAM_Integer,
		struct hdd_config, rssi_penalize_factor_5g,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_5G_RSSI_PENALIZE_FACTOR_DEFAULT,
		CFG_5G_RSSI_PENALIZE_FACTOR_MIN,
		CFG_5G_RSSI_PENALIZE_FACTOR_MAX),

	REG_VARIABLE(CFG_5G_MAX_RSSI_PENALIZE_NAME, WLAN_PARAM_Integer,
		struct hdd_config, max_rssi_penalize_5g,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_5G_MAX_RSSI_PENALIZE_DEFAULT,
		CFG_5G_MAX_RSSI_PENALIZE_MIN,
		CFG_5G_MAX_RSSI_PENALIZE_MAX),

	REG_VARIABLE(CFG_LPRx_NAME, WLAN_PARAM_Integer,
		struct hdd_config, enable_lprx,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_LPRx_DEFAULT,
		CFG_LPRx_MIN,
		CFG_LPRx_MAX),

	REG_VARIABLE(CFG_UPPER_BRSSI_THRESH_NAME, WLAN_PARAM_Integer,
		struct hdd_config, upper_brssi_thresh,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_UPPER_BRSSI_THRESH_DEFAULT,
		CFG_UPPER_BRSSI_THRESH_MIN,
		CFG_UPPER_BRSSI_THRESH_MAX),

	REG_VARIABLE(CFG_LOWER_BRSSI_THRESH_NAME, WLAN_PARAM_Integer,
		struct hdd_config, lower_brssi_thresh,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_LOWER_BRSSI_THRESH_DEFAULT,
		CFG_LOWER_BRSSI_THRESH_MIN,
		CFG_LOWER_BRSSI_THRESH_MAX),

	REG_VARIABLE(CFG_ENABLE_ACTION_OUI, WLAN_PARAM_Integer,
		     struct hdd_config, action_oui_enable,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_ACTION_OUI_DEFAULT,
		     CFG_ENABLE_ACTION_OUI_MIN,
		     CFG_ENABLE_ACTION_OUI_MAX),

	REG_VARIABLE_STRING(CFG_ACTION_OUI_CONNECT_1X1_NAME, WLAN_PARAM_String,
			    struct hdd_config, action_oui_str[0],
			    VAR_FLAGS_OPTIONAL,
			    (void *)CFG_ACTION_OUI_CONNECT_1X1_DEFAULT),

	REG_VARIABLE_STRING(CFG_ACTION_OUI_ITO_EXTENSION_NAME,
			    WLAN_PARAM_String,
			    struct hdd_config, action_oui_str[1],
			    VAR_FLAGS_OPTIONAL,
			    (void *)CFG_ACTION_OUI_ITO_EXTENSION_DEFAULT),

	REG_VARIABLE_STRING(CFG_ACTION_OUI_CCKM_1X1_NAME, WLAN_PARAM_String,
			    struct hdd_config, action_oui_str[2],
			    VAR_FLAGS_OPTIONAL,
			    (void *)CFG_ACTION_OUI_CCKM_1X1_DEFAULT),

	REG_VARIABLE_STRING(CFG_ACTION_OUI_ITO_ALTERNATE_NAME,
			    WLAN_PARAM_String,
			    struct hdd_config, action_oui_str[3],
			    VAR_FLAGS_OPTIONAL,
			    (void *)CFG_ACTION_OUI_ITO_ALTERNATE_DEFAULT),
	REG_VARIABLE_STRING(CFG_ACTION_OUI_SWITCH_TO_11N_MODE_NAME,
			    WLAN_PARAM_String,
			    struct hdd_config, action_oui_str[4],
			    VAR_FLAGS_OPTIONAL,
			    (void *)CFG_ACTION_OUI_SWITCH_TO_11N_MODE_DEFAULT),

	REG_VARIABLE_STRING(CFG_ACTION_OUI_CONNECT_1X1_WITH_1_CHAIN_NAME,
			    WLAN_PARAM_String,
			    struct hdd_config, action_oui_str[5],
			    VAR_FLAGS_OPTIONAL,
			    (void *)
			    CFG_ACTION_OUI_CONNECT_1X1_WITH_1_CHAIN_DEFAULT),

	REG_VARIABLE(CFG_DTIM_1CHRX_ENABLE_NAME, WLAN_PARAM_Integer,
		struct hdd_config, enable_dtim_1chrx,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_DTIM_1CHRX_ENABLE_DEFAULT,
		CFG_DTIM_1CHRX_ENABLE_MIN,
		CFG_DTIM_1CHRX_ENABLE_MAX),

	REG_VARIABLE(CFG_SCAN_11D_INTERVAL_NAME, WLAN_PARAM_Integer,
		struct hdd_config, scan_11d_interval,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_SCAN_11D_INTERVAL_DEFAULT,
		CFG_SCAN_11D_INTERVAL_MIN,
		CFG_SCAN_11D_INTERVAL_MAX),

	REG_VARIABLE(CFG_CHAN_SWITCH_HOSTAPD_RATE_ENABLED_NAME,
		WLAN_PARAM_Integer,
		struct hdd_config, chan_switch_hostapd_rate_enabled,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_CHAN_SWITCH_HOSTAPD_RATE_ENABLED_DEFAULT,
		CFG_CHAN_SWITCH_HOSTAPD_RATE_ENABLED_MIN,
		CFG_CHAN_SWITCH_HOSTAPD_RATE_ENABLED_MAX),

	REG_VARIABLE(CFG_IS_BSSID_HINT_PRIORITY_NAME, WLAN_PARAM_Integer,
		struct hdd_config, is_bssid_hint_priority,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_IS_BSSID_HINT_PRIORITY_DEFAULT,
		CFG_IS_BSSID_HINT_PRIORITY_MIN,
		CFG_IS_BSSID_HINT_PRIORITY_MAX),

	REG_VARIABLE(CFG_LATENCY_ENABLE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, wlm_latency_enable,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_LATENCY_ENABLE_DEFAULT,
		     CFG_LATENCY_ENABLE_MIN,
		     CFG_LATENCY_ENABLE_MAX),

	REG_VARIABLE(CFG_LATENCY_LEVEL_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, wlm_latency_level,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_LATENCY_LEVEL_DEFAULT,
		     CFG_LATENCY_LEVEL_MIN,
		     CFG_LATENCY_LEVEL_MAX),

	REG_VARIABLE(CFG_LATENCY_FLAGS_NORMAL_NAME, WLAN_PARAM_HexInteger,
		     struct hdd_config, wlm_latency_flags_normal,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_LATENCY_FLAGS_NORMAL_DEFAULT,
		     CFG_LATENCY_FLAGS_NORMAL_MIN,
		     CFG_LATENCY_FLAGS_NORMAL_MAX),

	REG_VARIABLE(CFG_LATENCY_FLAGS_MODERATE_NAME, WLAN_PARAM_HexInteger,
		     struct hdd_config, wlm_latency_flags_moderate,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_LATENCY_FLAGS_MODERATE_DEFAULT,
		     CFG_LATENCY_FLAGS_MODERATE_MIN,
		     CFG_LATENCY_FLAGS_MODERATE_MAX),

	REG_VARIABLE(CFG_LATENCY_FLAGS_LOW_NAME, WLAN_PARAM_HexInteger,
		     struct hdd_config, wlm_latency_flags_low,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_LATENCY_FLAGS_LOW_DEFAULT,
		     CFG_LATENCY_FLAGS_LOW_MIN,
		     CFG_LATENCY_FLAGS_LOW_MAX),

	REG_VARIABLE(CFG_LATENCY_FLAGS_ULTRALOW_NAME, WLAN_PARAM_HexInteger,
		     struct hdd_config, wlm_latency_flags_ultralow,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_LATENCY_FLAGS_ULTRALOW_DEFAULT,
		     CFG_LATENCY_FLAGS_ULTRALOW_MIN,
		     CFG_LATENCY_FLAGS_ULTRALOW_MAX),

	REG_VARIABLE(CFG_RSSI_WEIGHTAGE_NAME, WLAN_PARAM_Integer,
		struct hdd_config, rssi_weightage,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_RSSI_WEIGHTAGE_DEFAULT,
		CFG_RSSI_WEIGHTAGE_MIN,
		CFG_RSSI_WEIGHTAGE_MAX),

	REG_VARIABLE(CFG_HT_CAPABILITY_WEIGHTAGE_NAME, WLAN_PARAM_Integer,
		struct hdd_config, ht_caps_weightage,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_HT_CAPABILITY_WEIGHTAGE_DEFAULT,
		CFG_HT_CAPABILITY_WEIGHTAGE_MIN,
		CFG_HT_CAPABILITY_WEIGHTAGE_MAX),

	REG_VARIABLE(CFG_VHT_CAPABILITY_WEIGHTAGE_NAME, WLAN_PARAM_Integer,
		struct hdd_config, vht_caps_weightage,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_VHT_CAPABILITY_WEIGHTAGE_DEFAULT,
		CFG_VHT_CAPABILITY_WEIGHTAGE_MIN,
		CFG_VHT_CAPABILITY_WEIGHTAGE_MAX),

	REG_VARIABLE(CFG_HE_CAPABILITY_WEIGHTAGE_NAME, WLAN_PARAM_Integer,
		struct hdd_config, he_caps_weightage,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_HE_CAPABILITY_WEIGHTAGE_DEFAULT,
		CFG_HE_CAPABILITY_WEIGHTAGE_MIN,
		CFG_HE_CAPABILITY_WEIGHTAGE_MAX),

	REG_VARIABLE(CFG_CHAN_WIDTH_WEIGHTAGE_NAME, WLAN_PARAM_Integer,
		struct hdd_config, chan_width_weightage,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_CHAN_WIDTH_WEIGHTAGE_DEFAULT,
		CFG_CHAN_WIDTH_WEIGHTAGE_MIN,
		CFG_CHAN_WIDTH_WEIGHTAGE_MAX),

	REG_VARIABLE(CFG_CHAN_BAND_WEIGHTAGE_NAME, WLAN_PARAM_Integer,
		struct hdd_config, chan_band_weightage,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_CHAN_BAND_WEIGHTAGE_DEFAULT,
		CFG_CHAN_BAND_WEIGHTAGE_MIN,
		CFG_CHAN_BAND_WEIGHTAGE_MAX),

	REG_VARIABLE(CFG_NSS_WEIGHTAGE_NAME, WLAN_PARAM_Integer,
		struct hdd_config, nss_weightage,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_NSS_WEIGHTAGE_DEFAULT,
		CFG_NSS_WEIGHTAGE_MIN,
		CFG_NSS_WEIGHTAGE_MAX),

	REG_VARIABLE(CFG_BEAMFORMING_CAP_WEIGHTAGE_NAME, WLAN_PARAM_Integer,
		struct hdd_config, beamforming_cap_weightage,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_BEAMFORMING_CAP_WEIGHTAGE_DEFAULT,
		CFG_BEAMFORMING_CAP_WEIGHTAGE_MIN,
		CFG_BEAMFORMING_CAP_WEIGHTAGE_MAX),

	REG_VARIABLE(CFG_PCL_WEIGHT_WEIGHTAGE_NAME, WLAN_PARAM_Integer,
		struct hdd_config, pcl_weightage,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_PCL_WEIGHT_DEFAULT,
		CFG_PCL_WEIGHT_MIN,
		CFG_PCL_WEIGHT_MAX),

	REG_VARIABLE(CFG_CHANNEL_CONGESTION_WEIGHTAGE_NAME, WLAN_PARAM_Integer,
		struct hdd_config, channel_congestion_weightage,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_CHANNEL_CONGESTION_WEIGHTAGE_DEFAULT,
		CFG_CHANNEL_CONGESTION_WEIGHTAGE_MIN,
		CFG_CHANNEL_CONGESTION_WEIGHTAGE_MAX),

	REG_VARIABLE(CFG_OCE_WAN_WEIGHTAGE_NAME, WLAN_PARAM_Integer,
		struct hdd_config, oce_wan_weightage,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_OCE_WAN_WEIGHTAGE_DEFAULT,
		CFG_OCE_WAN_WEIGHTAGE_MIN,
		CFG_OCE_WAN_WEIGHTAGE_MAX),

	REG_VARIABLE(CFG_BEST_RSSI_THRESHOLD_NAME, WLAN_PARAM_Integer,
		struct hdd_config, best_rssi_threshold,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_BEST_RSSI_THRESHOLD_DEFAULT,
		CFG_BEST_RSSI_THRESHOLD_MIN,
		CFG_BEST_RSSI_THRESHOLD_MAX),

	REG_VARIABLE(CFG_GOOD_RSSI_THRESHOLD_NAME, WLAN_PARAM_Integer,
		struct hdd_config, good_rssi_threshold,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_GOOD_RSSI_THRESHOLD_DEFAULT,
		CFG_GOOD_RSSI_THRESHOLD_MIN,
		CFG_GOOD_RSSI_THRESHOLD_MAX),

	REG_VARIABLE(CFG_BAD_RSSI_THRESHOLD_NAME, WLAN_PARAM_Integer,
		struct hdd_config, bad_rssi_threshold,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_BAD_RSSI_THRESHOLD_DEFAULT,
		CFG_BAD_RSSI_THRESHOLD_MIN,
		CFG_BAD_RSSI_THRESHOLD_MAX),

	REG_VARIABLE(CFG_GOOD_RSSI_PCNT_NAME, WLAN_PARAM_Integer,
		struct hdd_config, good_rssi_pcnt,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_GOOD_RSSI_PCNT_DEFAULT,
		CFG_GOOD_RSSI_PCNT_MIN,
		CFG_GOOD_RSSI_PCNT_MAX),

	REG_VARIABLE(CFG_BAD_RSSI_PCNT_NAME, WLAN_PARAM_Integer,
		struct hdd_config, bad_rssi_pcnt,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_BAD_RSSI_PCNT_DEFAULT,
		CFG_BAD_RSSI_PCNT_MIN,
		CFG_BAD_RSSI_PCNT_MAX),

	REG_VARIABLE(CFG_GOOD_RSSI_BUCKET_SIZE_NAME, WLAN_PARAM_Integer,
		struct hdd_config, good_rssi_bucket_size,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_GOOD_RSSI_BUCKET_SIZE_DEFAULT,
		CFG_GOOD_RSSI_BUCKET_SIZE_MIN,
		CFG_GOOD_RSSI_BUCKET_SIZE_MAX),

	REG_VARIABLE(CFG_BAD_RSSI_BUCKET_SIZE_NAME, WLAN_PARAM_Integer,
		struct hdd_config, bad_rssi_bucket_size,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_BAD_RSSI_BUCKET_SIZE_DEFAULT,
		CFG_BAD_RSSI_BUCKET_SIZE_MIN,
		CFG_BAD_RSSI_BUCKET_SIZE_MAX),

	REG_VARIABLE(CFG_RSSI_PERF_5G_THRESHOLD_NAME, WLAN_PARAM_Integer,
		struct hdd_config, rssi_pref_5g_rssi_thresh,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_RSSI_PERF_5G_THRESHOLD_DEFAULT,
		CFG_RSSI_PERF_5G_THRESHOLD_MIN,
		CFG_RSSI_PERF_5G_THRESHOLD_MAX),

	REG_VARIABLE(CFG_BAND_WIDTH_WEIGHT_PER_INDEX_NAME,
		WLAN_PARAM_HexInteger,
		struct hdd_config, bandwidth_weight_per_index,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_BAND_WIDTH_WEIGHT_PER_INDEX_DEFAULT,
		CFG_BAND_WIDTH_WEIGHT_PER_INDEX_MIN,
		CFG_BAND_WIDTH_WEIGHT_PER_INDEX_MAX),

	REG_VARIABLE(CFG_NSS_WEIGHT_PER_INDEX_NAME, WLAN_PARAM_HexInteger,
		struct hdd_config, nss_weight_per_index,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_NSS_WEIGHT_PER_INDEX_DEFAULT,
		CFG_NSS_WEIGHT_PER_INDEX_MIN,
		CFG_NSS_WEIGHT_PER_INDEX_MAX),

	REG_VARIABLE(CFG_BAND_WEIGHT_PER_INDEX_NAME, WLAN_PARAM_HexInteger,
		struct hdd_config, band_weight_per_index,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_BAND_WEIGHT_PER_INDEX_DEFAULT,
		CFG_BAND_WEIGHT_PER_INDEX_MIN,
		CFG_BAND_WEIGHT_PER_INDEX_MAX),

	REG_VARIABLE(CFG_ESP_QBSS_SLOTS_NAME, WLAN_PARAM_Integer,
		struct hdd_config, num_esp_qbss_slots,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ESP_QBSS_SLOTS_DEFAULT,
		CFG_ESP_QBSS_SLOTS_MIN,
		CFG_ESP_QBSS_SLOTS_MAX),

	REG_VARIABLE(CFG_ESP_QBSS_SCORE_IDX3_TO_0_NAME, WLAN_PARAM_HexInteger,
		struct hdd_config, esp_qbss_score_slots3_to_0,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ESP_QBSS_SCORE_IDX3_TO_0_DEFAULT,
		CFG_ESP_QBSS_SCORE_IDX3_TO_0_MIN,
		CFG_ESP_QBSS_SCORE_IDX3_TO_0_MAX),

	REG_VARIABLE(CFG_ESP_QBSS_SCORE_IDX7_TO_4_NAME, WLAN_PARAM_HexInteger,
		struct hdd_config, esp_qbss_score_slots7_to_4,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ESP_QBSS_SCORE_IDX7_TO_4_DEFAULT,
		CFG_ESP_QBSS_SCORE_IDX7_TO_4_MIN,
		CFG_ESP_QBSS_SCORE_IDX7_TO_4_MAX),

	REG_VARIABLE(CFG_ESP_QBSS_SCORE_IDX11_TO_8_NAME, WLAN_PARAM_HexInteger,
		struct hdd_config, esp_qbss_score_slots11_to_8,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ESP_QBSS_SCORE_IDX11_TO_8_DEFAULT,
		CFG_ESP_QBSS_SCORE_IDX11_TO_8_MIN,
		CFG_ESP_QBSS_SCORE_IDX11_TO_8_MAX),

	REG_VARIABLE(CFG_ESP_QBSS_SCORE_IDX15_TO_12_NAME, WLAN_PARAM_HexInteger,
		struct hdd_config, esp_qbss_score_slots15_to_12,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ESP_QBSS_SCORE_IDX15_TO_12_DEFAULT,
		CFG_ESP_QBSS_SCORE_IDX15_TO_12_MIN,
		CFG_ESP_QBSS_SCORE_IDX15_TO_12_MAX),

	REG_VARIABLE(CFG_OCE_WAN_SLOTS_NAME, WLAN_PARAM_Integer,
		struct hdd_config, num_oce_wan_slots,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_OCE_WAN_SLOTS_DEFAULT,
		CFG_OCE_WAN_SLOTS_MIN,
		CFG_OCE_WAN_SLOTS_MAX),

	REG_VARIABLE(CFG_OCE_WAN_SCORE_IDX3_TO_0_NAME, WLAN_PARAM_HexInteger,
		struct hdd_config, oce_wan_score_slots3_to_0,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_OCE_WAN_SCORE_IDX3_TO_0_DEFAULT,
		CFG_OCE_WAN_SCORE_IDX3_TO_0_MIN,
		CFG_OCE_WAN_SCORE_IDX3_TO_0_MAX),

	REG_VARIABLE(CFG_OCE_WAN_SCORE_IDX7_TO_4_NAME, WLAN_PARAM_HexInteger,
		struct hdd_config, oce_wan_score_slots7_to_4,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_OCE_WAN_SCORE_IDX7_TO_4_DEFAULT,
		CFG_OCE_WAN_SCORE_IDX7_TO_4_MIN,
		CFG_OCE_WAN_SCORE_IDX7_TO_4_MAX),

	REG_VARIABLE(CFG_OCE_WAN_SCORE_IDX11_TO_8_NAME, WLAN_PARAM_HexInteger,
		struct hdd_config, oce_wan_score_slots11_to_8,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_OCE_WAN_SCORE_IDX11_TO_8_DEFAULT,
		CFG_OCE_WAN_SCORE_IDX11_TO_8_MIN,
		CFG_OCE_WAN_SCORE_IDX11_TO_8_MAX),

	REG_VARIABLE(CFG_OCE_WAN_SCORE_IDX15_TO_12_NAME, WLAN_PARAM_HexInteger,
		struct hdd_config, oce_wan_score_slots15_to_12,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_OCE_WAN_SCORE_IDX15_TO_12_DEFAULT,
		CFG_OCE_WAN_SCORE_IDX15_TO_12_MIN,
		CFG_OCE_WAN_SCORE_IDX15_TO_12_MAX),

	REG_VARIABLE(CFG_ENABLE_SCORING_FOR_ROAM_NAME, WLAN_PARAM_Integer,
		struct hdd_config, enable_scoring_for_roam,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ENABLE_SCORING_FOR_ROAM_DEFAULT,
		CFG_ENABLE_SCORING_FOR_ROAM_MIN,
		CFG_ENABLE_SCORING_FOR_ROAM_MAX),

	REG_VARIABLE(CFG_FORCE_RSNE_OVERRIDE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, force_rsne_override,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_FORCE_RSNE_OVERRIDE_DEFAULT,
		     CFG_FORCE_RSNE_OVERRIDE_MIN,
		     CFG_FORCE_RSNE_OVERRIDE_MAX),

	REG_VARIABLE(CFG_MBO_CANDIDATE_RSSI_THRESHOLD_NAME,
		WLAN_PARAM_SignedInteger, struct hdd_config,
		mbo_candidate_rssi_thres,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_CANDIDATE_RSSI_THRESHOLD_DEFAULT,
		CFG_CANDIDATE_RSSI_THRESHOLD_MIN,
		CFG_CANDIDATE_RSSI_THRESHOLD_MAX),

	REG_VARIABLE(CFG_MBO_CURRENT_RSSI_THRESHOLD_NAME,
		WLAN_PARAM_SignedInteger, struct hdd_config,
		mbo_current_rssi_thres,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_CURRENT_RSSI_THRESHOLD_DEFAULT,
		CFG_CURRENT_RSSI_THRESHOLD_MIN,
		CFG_CURRENT_RSSI_THRESHOLD_MAX),

	REG_VARIABLE(CFG_MBO_CUR_RSSI_MCC_THRESHOLD_NAME,
		WLAN_PARAM_SignedInteger, struct hdd_config,
		mbo_current_rssi_mcc_thres,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_MBO_CUR_RSSI_MCC_THRESHOLD_DEFAULT,
		CFG_MBO_CUR_RSSI_MCC_THRESHOLD_MIN,
		CFG_MBO_CUR_RSSI_MCC_THRESHOLD_MAX),

	REG_VARIABLE(CFG_MBO_CAND_RSSI_BTC_THRESHOLD_NAME,
		WLAN_PARAM_SignedInteger, struct hdd_config,
		mbo_candidate_rssi_btc_thres,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_MBO_CAND_RSSI_BTC_THRESHOLD_DEFAULT,
		CFG_MBO_CAND_RSSI_BTC_THRESHOLD_MIN,
		CFG_MBO_CAND_RSSI_BTC_THRESHOLD_MAX),
	REG_VARIABLE(CFG_IS_FILS_ENABLED_NAME, WLAN_PARAM_Integer,
		struct hdd_config, is_fils_enabled,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_IS_FILS_ENABLED_DEFAULT,
		CFG_IS_FILS_ENABLED_MIN,
		CFG_IS_FILS_ENABLED_MAX),

#ifdef WLAN_FEATURE_PACKET_FILTERING
	REG_VARIABLE(CFG_ENABLE_PACKET_FILTERS_NAME, WLAN_PARAM_Integer,
		struct hdd_config, packet_filters_bitmap,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ENABLE_PACKET_FILTERS_DEFAULT,
		CFG_ENABLE_PACKET_FILTERS_MIN,
		CFG_ENABLE_PACKET_FILTERS_MAX),
#endif /* WLAN_FEATURE_PACKET_FILTERING */

	REG_VARIABLE(CFG_DFS_BEACON_TX_ENHANCED, WLAN_PARAM_Integer,
		struct hdd_config, dfs_beacon_tx_enhanced,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_DFS_BEACON_TX_ENHANCED_DEFAULT,
		CFG_DFS_BEACON_TX_ENHANCED_MIN,
		CFG_DFS_BEACON_TX_ENHANCED_MAX),

	REG_VARIABLE(CFG_REDUCED_BEACON_INTERVAL, WLAN_PARAM_Integer,
		struct hdd_config, reduced_beacon_interval,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_REDUCED_BEACON_INTERVAL_DEFAULT,
		CFG_REDUCED_BEACON_INTERVAL_MIN,
		CFG_REDUCED_BEACON_INTERVAL_MAX),

	REG_VARIABLE(CFG_OCE_ENABLE_RSSI_BASED_ASSOC_REJECT_NAME,
		     WLAN_PARAM_Integer, struct hdd_config,
		     rssi_assoc_reject_enabled,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_OCE_ENABLE_RSSI_BASED_ASSOC_REJECT_DEFAULT,
		     CFG_OCE_ENABLE_RSSI_BASED_ASSOC_REJECT_MIN,
		     CFG_OCE_ENABLE_RSSI_BASED_ASSOC_REJECT_MAX),

	REG_VARIABLE(CFG_OCE_PROBE_REQ_RATE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, oce_probe_req_rate_enabled,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_OCE_PROBE_REQ_RATE_DEFAULT,
		     CFG_OCE_PROBE_REQ_RATE_MIN,
		     CFG_OCE_PROBE_REQ_RATE_MAX),

	REG_VARIABLE(CFG_OCE_PROBE_RSP_RATE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, oce_probe_resp_rate_enabled,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_OCE_PROBE_RSP_RATE_DEFAULT,
		     CFG_OCE_PROBE_RSP_RATE_MIN,
		     CFG_OCE_PROBE_RSP_RATE_MAX),

	REG_VARIABLE(CFG_OCE_BEACON_RATE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, oce_beacon_rate_enabled,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_OCE_BEACON_RATE_DEFAULT,
		     CFG_OCE_BEACON_RATE_MIN,
		     CFG_OCE_BEACON_RATE_MAX),

	REG_VARIABLE(CFG_ENABLE_PROBE_REQ_DEFERRAL_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, probe_req_deferral_enabled,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_PROBE_REQ_DEFERRAL_DEFAULT,
		     CFG_ENABLE_PROBE_REQ_DEFERRAL_MIN,
		     CFG_ENABLE_PROBE_REQ_DEFERRAL_MAX),

	REG_VARIABLE(CFG_ENABLE_FILS_DISCOVERY_SAP_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, fils_discovery_sap_enabled,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_FILS_DISCOVERY_SAP_DEFAULT,
		     CFG_ENABLE_FILS_DISCOVERY_SAP_MIN,
		     CFG_ENABLE_FILS_DISCOVERY_SAP_MAX),

	REG_VARIABLE(CFG_ENABLE_ESP_FEATURE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, esp_for_roam_enabled,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_ESP_FEATURE_DEFAULT,
		     CFG_ENABLE_ESP_FEATURE_MIN,
		     CFG_ENABLE_ESP_FEATURE_MAX),

	REG_VARIABLE(CFG_BTM_ENABLE_NAME, WLAN_PARAM_HexInteger,
		     struct hdd_config, btm_offload_config,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_BTM_ENABLE_DEFAULT,
		     CFG_BTM_ENABLE_MIN,
		     CFG_BTM_ENABLE_MAX),

#ifdef WLAN_FEATURE_SAE
	REG_VARIABLE(CFG_IS_SAE_ENABLED_NAME, WLAN_PARAM_Integer,
		struct hdd_config, is_sae_enabled,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_IS_SAE_ENABLED_DEFAULT,
		CFG_IS_SAE_ENABLED_MIN,
		CFG_IS_SAE_ENABLED_MAX),
#endif

	REG_VARIABLE(CFG_BTM_SOLICITED_TIMEOUT, WLAN_PARAM_Integer,
		     struct hdd_config, btm_solicited_timeout,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_BTM_SOLICITED_TIMEOUT_DEFAULT,
		     CFG_BTM_SOLICITED_TIMEOUT_MIN,
		     CFG_BTM_SOLICITED_TIMEOUT_MAX),

	REG_VARIABLE(CFG_BTM_MAX_ATTEMPT_CNT, WLAN_PARAM_Integer,
		     struct hdd_config, btm_max_attempt_cnt,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_BTM_MAX_ATTEMPT_CNT_DEFAULT,
		     CFG_BTM_MAX_ATTEMPT_CNT_MIN,
		     CFG_BTM_MAX_ATTEMPT_CNT_MAX),

	REG_VARIABLE(CFG_BTM_STICKY_TIME, WLAN_PARAM_Integer,
		     struct hdd_config, btm_sticky_time,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_BTM_STICKY_TIME_DEFAULT,
		     CFG_BTM_STICKY_TIME_MIN,
		     CFG_BTM_STICKY_TIME_MAX),

	REG_VARIABLE(CFG_ENABLE_GCMP_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, gcmp_enabled,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_GCMP_DEFAULT,
		     CFG_ENABLE_GCMP_MIN,
		     CFG_ENABLE_GCMP_MAX),

	REG_VARIABLE(CFG_OFFLOAD_11K_ENABLE_BITMASK_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, offload_11k_enable_bitmask,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_OFFLOAD_11K_ENABLE_BITMASK_DEFAULT,
		     CFG_OFFLOAD_11K_ENABLE_BITMASK_MIN,
		     CFG_OFFLOAD_11K_ENABLE_BITMASK_MAX),

	REG_VARIABLE(CFG_OFFLOAD_NEIGHBOR_REPORT_PARAMS_BITMASK_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, neighbor_report_offload_params_bitmask,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_OFFLOAD_NEIGHBOR_REPORT_PARAMS_BITMASK_DEFAULT,
		     CFG_OFFLOAD_NEIGHBOR_REPORT_PARAMS_BITMASK_MIN,
		     CFG_OFFLOAD_NEIGHBOR_REPORT_PARAMS_BITMASK_MAX),

	REG_VARIABLE(CFG_OFFLOAD_NEIGHBOR_REPORT_TIME_OFFSET_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, neighbor_report_offload_time_offset,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_OFFLOAD_NEIGHBOR_REPORT_TIME_OFFSET_DEFAULT,
		     CFG_OFFLOAD_NEIGHBOR_REPORT_TIME_OFFSET_MIN,
		     CFG_OFFLOAD_NEIGHBOR_REPORT_TIME_OFFSET_MAX),

	REG_VARIABLE(CFG_OFFLOAD_NEIGHBOR_REPORT_LOW_RSSI_OFFSET_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, neighbor_report_offload_low_rssi_offset,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_OFFLOAD_NEIGHBOR_REPORT_LOW_RSSI_OFFSET_DEFAULT,
		     CFG_OFFLOAD_NEIGHBOR_REPORT_LOW_RSSI_OFFSET_MIN,
		     CFG_OFFLOAD_NEIGHBOR_REPORT_LOW_RSSI_OFFSET_MAX),

	REG_VARIABLE(CFG_OFFLOAD_NEIGHBOR_REPORT_BMISS_COUNT_TRIGGER_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config,
		     neighbor_report_offload_bmiss_count_trigger,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_OFFLOAD_NEIGHBOR_REPORT_BMISS_COUNT_TRIGGER_DEFAULT,
		     CFG_OFFLOAD_NEIGHBOR_REPORT_BMISS_COUNT_TRIGGER_MIN,
		     CFG_OFFLOAD_NEIGHBOR_REPORT_BMISS_COUNT_TRIGGER_MAX),

	REG_VARIABLE(CFG_OFFLOAD_NEIGHBOR_REPORT_PER_THRESHOLD_OFFSET_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config,
		     neighbor_report_offload_per_threshold_offset,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_OFFLOAD_NEIGHBOR_REPORT_PER_THRESHOLD_OFFSET_DEFAULT,
		     CFG_OFFLOAD_NEIGHBOR_REPORT_PER_THRESHOLD_OFFSET_MIN,
		     CFG_OFFLOAD_NEIGHBOR_REPORT_PER_THRESHOLD_OFFSET_MAX),

	REG_VARIABLE(CFG_OFFLOAD_NEIGHBOR_REPORT_CACHE_TIMEOUT_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, neighbor_report_offload_cache_timeout,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_OFFLOAD_NEIGHBOR_REPORT_CACHE_TIMEOUT_DEFAULT,
		     CFG_OFFLOAD_NEIGHBOR_REPORT_CACHE_TIMEOUT_MIN,
		     CFG_OFFLOAD_NEIGHBOR_REPORT_CACHE_TIMEOUT_MAX),

	REG_VARIABLE(CFG_OFFLOAD_NEIGHBOR_REPORT_MAX_REQ_CAP_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, neighbor_report_offload_max_req_cap,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_OFFLOAD_NEIGHBOR_REPORT_MAX_REQ_CAP_DEFAULT,
		     CFG_OFFLOAD_NEIGHBOR_REPORT_MAX_REQ_CAP_MIN,
		     CFG_OFFLOAD_NEIGHBOR_REPORT_MAX_REQ_CAP_MAX),

	REG_VARIABLE(CFG_WMI_WQ_WATCHDOG, WLAN_PARAM_Integer,
		     struct hdd_config, wmi_wq_watchdog_timeout,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_WMI_WQ_WATCHDOG_DEFAULT,
		     CFG_WMI_WQ_WATCHDOG_MIN,
		     CFG_WMI_WQ_WATCHDOG_MAX),

	REG_VARIABLE(CFG_DTIM_SELECTION_DIVERSITY_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, enable_dtim_selection_diversity,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_DTIM_SELECTION_DIVERSITY_DEFAULT,
		     CFG_DTIM_SELECTION_DIVERSITY_MIN,
		     CFG_DTIM_SELECTION_DIVERSITY_MAX),

	REG_VARIABLE(CFG_CHANNEL_SELECT_LOGIC_CONC_NAME, WLAN_PARAM_HexInteger,
		     struct hdd_config, channel_select_logic_conc,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_CHANNEL_SELECT_LOGIC_CONC_DEFAULT,
		     CFG_CHANNEL_SELECT_LOGIC_CONC_MIN,
		     CFG_CHANNEL_SELECT_LOGIC_CONC_MAX),

	REG_VARIABLE(CFG_TX_SCH_DELAY_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, enable_tx_sch_delay,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TX_SCH_DELAY_DEFAULT,
		     CFG_TX_SCH_DELAY_MIN,
		     CFG_TX_SCH_DELAY_MAX),

	HDD_GREEN_AP_REG_VARIABLES

	REG_VARIABLE(CFG_ENABLE_UNIT_TEST_FRAMEWORK_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, is_unit_test_framework_enabled,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_UINT_TEST_FRAMEWORK_DEFAULT,
		     CFG_ENABLE_UNIT_TEST_FRAMEWORK_MIN,
		     CFG_ENABLE_UNIT_TEST_FRAMEWORK_MAX),

	REG_VARIABLE(CFG_ROAM_FT_OPEN_ENABLE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, enable_ftopen,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ROAM_FT_OPEN_ENABLE_DEFAULT,
		     CFG_ROAM_FT_OPEN_ENABLE_MIN,
		     CFG_ROAM_FT_OPEN_ENABLE_MAX),

	REG_VARIABLE(CFG_ENABLE_RTT_MAC_RANDOMIZATION_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, enable_rtt_mac_randomization,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_RTT_MAC_RANDOMIZATION_DEFAULT,
		     CFG_ENABLE_RTT_MAC_RANDOMIZATION_MIN,
		     CFG_ENABLE_RTT_MAC_RANDOMIZATION_MAX),

	REG_VARIABLE(CFG_ENABLE_SECONDARY_RATE_NAME,
		     WLAN_PARAM_HexInteger,
		     struct hdd_config, enable_secondary_rate,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_SECONDARY_RATE_DEFAULT,
		     CFG_ENABLE_SECONDARY_RATE_MIN,
		     CFG_ENABLE_SECONDARY_RATE_MAX),

	REG_VARIABLE(CFG_ROAM_FORCE_RSSI_TRIGGER_NAME,
		     WLAN_PARAM_Integer, struct hdd_config,
		     roam_force_rssi_trigger,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ROAM_FORCE_RSSI_TRIGGER_DEFAULT,
		     CFG_ROAM_FORCE_RSSI_TRIGGER_MIN,
		     CFG_ROAM_FORCE_RSSI_TRIGGER_MAX),

	REG_VARIABLE(CFG_NUM_VDEV_ENABLE_NAME,
		     WLAN_PARAM_HexInteger,
		     struct hdd_config, num_vdevs,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_NUM_VDEV_ENABLE_DEFAULT,
		     CFG_NUM_VDEV_ENABLE_MIN,
		     CFG_NUM_VDEV_ENABLE_MAX),

	REG_VARIABLE(CFG_CHANGE_CHANNEL_BANDWIDTH_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, enable_change_channel_bandwidth,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_CHANGE_CHANNEL_BANDWIDTH_DEFAULT,
		     CFG_CHANGE_CHANNEL_BANDWIDTH_MIN,
		     CFG_CHANGE_CHANNEL_BANDWIDTH_MAX),
#ifdef MWS_COEX
	REG_VARIABLE(CFG_MWS_COEX_4G_QUICK_FTDM_NAME, WLAN_PARAM_HexInteger,
		     struct hdd_config, mws_coex_4g_quick_tdm,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_MWS_COEX_4G_QUICK_FTDM_DEFAULT,
		     CFG_MWS_COEX_4G_QUICK_FTDM_MIN,
		     CFG_MWS_COEX_4G_QUICK_FTDM_MAX),
	REG_VARIABLE(CFG_MWS_COEX_5G_NR_PWR_LIMIT_NAME, WLAN_PARAM_HexInteger,
		     struct hdd_config, mws_coex_5g_nr_pwr_limit,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_MWS_COEX_5G_NR_PWR_LIMIT_DEFAULT,
		     CFG_MWS_COEX_5G_NR_PWR_LIMIT_MIN,
		     CFG_MWS_COEX_5G_NR_PWR_LIMIT_MAX),
#endif
};


/**
 * get_next_line() - find and locate the new line pointer
 * @str: pointer to string
 *
 * This function returns a pointer to the character after the occurrence
 * of a new line character. It also modifies the original string by replacing
 * the '\n' character with the null character.
 *
 * Return: the pointer to the character at new line,
 *            or NULL if no new line character was found
 */
static char *get_next_line(char *str)
{
	char c;

	if (str == NULL || *str == '\0')
		return NULL;

	c = *str;
	while (c != '\n' && c != '\0' && c != 0xd) {
		str = str + 1;
		c = *str;
	}

	if (c == '\0')
		return NULL;

	*str = '\0';
	return str + 1;
}

/** look for space. Ascii values to look are
 * 0x09 == horizontal tab
 * 0x0a == Newline ("\n")
 * 0x0b == vertical tab
 * 0x0c == Newpage or feed form.
 * 0x0d == carriage return (CR or "\r")
 * Null ('\0') should not considered as space.
 */
#define i_isspace(ch)  (((ch) >= 0x09 && (ch) <= 0x0d) || (ch) == ' ')

/**
 * i_trim() - trims any leading and trailing white spaces
 * @str: pointer to string
 *
 * Return: the pointer of the string
 */
static char *i_trim(char *str)
{
	char *ptr;

	if (*str == '\0')
		return str;

	/* Find the first non white-space */
	ptr = str;
	while (i_isspace(*ptr))
		ptr++;

	if (*ptr == '\0')
		return str;

	/* This is the new start of the string */
	str = ptr;

	/* Find the last non white-space */
	ptr += strlen(ptr) - 1;

	while (ptr != str && i_isspace(*ptr))
		ptr--;

	/* Null terminate the following character */
	ptr[1] = '\0';

	return str;
}

/* Maximum length of the confgiuration name and value */
#define CFG_VALUE_MAX_LEN 256
#define CFG_ENTRY_MAX_LEN (32+CFG_VALUE_MAX_LEN)

/**
 * hdd_cfg_get_config() - get the configuration content
 * @reg_table: pointer to configuration table
 * @cRegTableEntries: number of the configuration entries
 * @ini_struct: pointer to the hdd config knob
 * @hdd_ctx: pointer to hdd context
 * @print_fn: print function pointer
 *
 * Return: none
 */
static void hdd_cfg_get_config(struct reg_table_entry *reg_table,
			       unsigned long cRegTableEntries,
			       uint8_t *ini_struct, struct hdd_context *hdd_ctx,
			       void (*print_fn)(const char *))
{
	unsigned int idx;
	struct reg_table_entry *pRegEntry = reg_table;
	uint32_t value;
	char valueStr[CFG_VALUE_MAX_LEN];
	char config_str[CFG_ENTRY_MAX_LEN];
	char *fmt;
	void *pField;
	struct qdf_mac_addr *pMacAddr;
	int curlen;

	for (idx = 0; idx < cRegTableEntries; idx++, pRegEntry++) {
		pField = ini_struct + pRegEntry->VarOffset;

		if ((WLAN_PARAM_Integer == pRegEntry->RegType) ||
		    (WLAN_PARAM_SignedInteger == pRegEntry->RegType) ||
		    (WLAN_PARAM_HexInteger == pRegEntry->RegType)) {
			value = 0;

			if ((pRegEntry->VarSize > sizeof(value)) ||
			    (pRegEntry->VarSize == 0)) {
				pr_warn("Invalid length of %s: %d",
					pRegEntry->RegName, pRegEntry->VarSize);
				continue;
			}

			memcpy(&value, pField, pRegEntry->VarSize);
			if (WLAN_PARAM_HexInteger == pRegEntry->RegType) {
				fmt = "%x";
			} else if (WLAN_PARAM_SignedInteger ==
				   pRegEntry->RegType) {
				fmt = "%d";
				value = sign_extend32(
						value,
						pRegEntry->VarSize * 8 - 1);
			} else {
				fmt = "%u";
			}
			snprintf(valueStr, CFG_VALUE_MAX_LEN, fmt, value);
		} else if (WLAN_PARAM_String == pRegEntry->RegType) {
			snprintf(valueStr, CFG_VALUE_MAX_LEN, "%s",
				 (char *)pField);
		} else if (WLAN_PARAM_MacAddr == pRegEntry->RegType) {
			pMacAddr = (struct qdf_mac_addr *) pField;
			snprintf(valueStr, CFG_VALUE_MAX_LEN,
				 "%02x:%02x:%02x:%02x:%02x:%02x",
				 pMacAddr->bytes[0],
				 pMacAddr->bytes[1],
				 pMacAddr->bytes[2],
				 pMacAddr->bytes[3],
				 pMacAddr->bytes[4], pMacAddr->bytes[5]);
		} else {
			snprintf(valueStr, CFG_VALUE_MAX_LEN, "(unhandled)");
		}
		curlen = scnprintf(config_str, CFG_ENTRY_MAX_LEN,
				   "%s=%s%s\n",
				   pRegEntry->RegName,
				   valueStr,
				   test_bit(idx,
					    (void *)&hdd_ctx->config->
					    bExplicitCfg) ? "*" : "");
		(*print_fn)(config_str);
	}
}

/** struct hdd_cfg_entry - ini configuration entry
 * @name: name of the entry
 * @value: value of the entry
 */
struct hdd_cfg_entry {
	char *name;
	char *value;
};

/**
 * find_cfg_item() - find the configuration item
 * @iniTable: pointer to configuration table
 * @entries: number fo the configuration entries
 * @name: the interested configuration to find
 * @value: the value to read back
 *
 * Return: QDF_STATUS_SUCCESS if the interested configuration is found,
 *		otherwise QDF_STATUS_E_FAILURE
 */
static QDF_STATUS find_cfg_item(struct hdd_cfg_entry *iniTable,
				unsigned long entries,
				char *name, char **value)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	unsigned long i;

	for (i = 0; i < entries; i++) {
		if (strcmp(iniTable[i].name, name) == 0) {
			*value = iniTable[i].value;
			hdd_debug("Found %s entry for Name=[%s] Value=[%s] ",
				  WLAN_INI_FILE, name, *value);
			return QDF_STATUS_SUCCESS;
		}
	}

	return status;
}

/**
 * parse_hex_digit() - conversion to hex value
 * @c: the character to convert
 *
 * Return: the hex value, otherwise 0
 */
static int parse_hex_digit(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;

	return 0;
}

/**
 * update_mac_from_string() - convert string to 6 bytes mac address
 * @hdd_ctx: the pointer to hdd context
 * @macTable: the macTable to carry the conversion
 * @num: number of the interface
 *
 * 00AA00BB00CC -> 0x00 0xAA 0x00 0xBB 0x00 0xCC
 *
 * Return: None
 */
static void update_mac_from_string(struct hdd_context *hdd_ctx,
				   struct hdd_cfg_entry *macTable, int num)
{
	int i = 0, j = 0, res = 0;
	char *candidate = NULL;
	struct qdf_mac_addr macaddr[QDF_MAX_CONCURRENCY_PERSONA];

	memset(macaddr, 0, sizeof(macaddr));

	for (i = 0; i < num; i++) {
		candidate = macTable[i].value;
		for (j = 0; j < QDF_MAC_ADDR_SIZE; j++) {
			res =
				hex2bin(&macaddr[i].bytes[j], &candidate[(j << 1)],
					1);
			if (res < 0)
				break;
		}
		if (res == 0 && !qdf_is_macaddr_zero(&macaddr[i])) {
			qdf_mem_copy((uint8_t *) &hdd_ctx->config->
				     intfMacAddr[i].bytes[0],
				     (uint8_t *) &macaddr[i].bytes[0],
				     QDF_MAC_ADDR_SIZE);
		}
	}
}

/**
 * hdd_apply_cfg_ini() - apply the ini configuration file
 * @hdd_ctx: the pointer to hdd context
 * @iniTable: pointer to configuration table
 * @entries: number fo the configuration entries
 * It overwrites the MAC address if config file exist.
 *
 * Return: QDF_STATUS_SUCCESS if the ini configuration file is correctly parsed,
 *		otherwise QDF_STATUS_E_INVAL
 */
static QDF_STATUS hdd_apply_cfg_ini(struct hdd_context *hdd_ctx,
				    struct hdd_cfg_entry *iniTable,
				    unsigned long entries)
{
	QDF_STATUS match_status = QDF_STATUS_E_FAILURE;
	QDF_STATUS ret_status = QDF_STATUS_SUCCESS;
	unsigned int idx;
	void *pField;
	char *value_str = NULL;
	unsigned long len_value_str;
	char *candidate;
	uint32_t value;
	int32_t svalue;
	void *pStructBase = hdd_ctx->config;
	struct reg_table_entry *pRegEntry = g_registry_table;
	unsigned long cRegTableEntries = QDF_ARRAY_SIZE(g_registry_table);
	uint32_t cbOutString;
	int i;
	int rv;

	BUILD_BUG_ON(MAX_CFG_INI_ITEMS < cRegTableEntries);

	for (idx = 0; idx < cRegTableEntries; idx++, pRegEntry++) {
		/* Calculate the address of the destination field in the structure. */
		pField = ((uint8_t *) pStructBase) + pRegEntry->VarOffset;

		match_status =
			find_cfg_item(iniTable, entries, pRegEntry->RegName,
				      &value_str);

		if ((match_status != QDF_STATUS_SUCCESS)
		    && (pRegEntry->Flags & VAR_FLAGS_REQUIRED)) {
			/* If we could not read the cfg item and it is required, this is an error. */
			hdd_err("Failed to read required config parameter %s", pRegEntry->RegName);
			ret_status = QDF_STATUS_E_FAILURE;
			break;
		}

		if ((WLAN_PARAM_Integer == pRegEntry->RegType) ||
		    (WLAN_PARAM_HexInteger == pRegEntry->RegType)) {
			/* If successfully read from the registry, use the value read.
			 * If not, use the default value.
			 */
			if (match_status == QDF_STATUS_SUCCESS
			    && (WLAN_PARAM_Integer == pRegEntry->RegType)) {
				rv = kstrtou32(value_str, 10, &value);
				if (rv < 0) {
					hdd_warn("Reg Parameter %s invalid. Enforcing default", pRegEntry->RegName);
					value = pRegEntry->VarDefault;
				}
			} else if (match_status == QDF_STATUS_SUCCESS
				   && (WLAN_PARAM_HexInteger ==
				       pRegEntry->RegType)) {
				rv = kstrtou32(value_str, 16, &value);
				if (rv < 0) {
					hdd_warn("Reg parameter %s invalid. Enforcing default", pRegEntry->RegName);
					value = pRegEntry->VarDefault;
				}
			} else {
				value = pRegEntry->VarDefault;
			}

			/* Only if the parameter is set in the ini file, do the range check here */
			if (match_status == QDF_STATUS_SUCCESS &&
			    pRegEntry->Flags & VAR_FLAGS_RANGE_CHECK) {
				if (value > pRegEntry->VarMax) {
					hdd_warn("Reg Parameter %s > allowed Maximum [%u > %lu]. Enforcing Maximum", pRegEntry->RegName,
					       value, pRegEntry->VarMax);
					value = pRegEntry->VarMax;
				}

				if (value < pRegEntry->VarMin) {
					hdd_warn("Reg Parameter %s < allowed Minimum [%u < %lu]. Enforcing Minimum", pRegEntry->RegName,
					       value, pRegEntry->VarMin);
					value = pRegEntry->VarMin;
				}
			}
			/* Only if the parameter is set in the ini file, do the range check here */
			else if (match_status == QDF_STATUS_SUCCESS &&
				 pRegEntry->Flags &
					VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT) {
				if (value > pRegEntry->VarMax) {
					hdd_warn("Reg Parameter %s > allowed Maximum [%u > %lu]. Enforcing Default: %lu", pRegEntry->RegName,
					       value, pRegEntry->VarMax,
					       pRegEntry->VarDefault);
					value = pRegEntry->VarDefault;
				}

				if (value < pRegEntry->VarMin) {
					hdd_warn("Reg Parameter %s < allowed Minimum [%u < %lu]. Enforcing Default: %lu", pRegEntry->RegName,
					       value, pRegEntry->VarMin,
					       pRegEntry->VarDefault);
					value = pRegEntry->VarDefault;
				}
			}
			/* Move the variable into the output field. */
			memcpy(pField, &value, pRegEntry->VarSize);
		} else if (WLAN_PARAM_SignedInteger == pRegEntry->RegType) {
			/* If successfully read from the registry, use the value read.
			 * If not, use the default value.
			 */
			if (QDF_STATUS_SUCCESS == match_status) {
				rv = kstrtos32(value_str, 10, &svalue);
				if (rv < 0) {
					hdd_warn("Reg Parameter %s invalid. Enforcing Default", pRegEntry->RegName);
					svalue =
						(int32_t) pRegEntry->VarDefault;
				}
			} else {
				svalue = (int32_t) pRegEntry->VarDefault;
			}

			/* Only if the parameter is set in the ini file, do the range check here */
			if (match_status == QDF_STATUS_SUCCESS &&
			    pRegEntry->Flags & VAR_FLAGS_RANGE_CHECK) {
				if (svalue > (int32_t) pRegEntry->VarMax) {
					hdd_warn("Reg Parameter %s > allowed Maximum "
					       "[%d > %d]. Enforcing Maximum", pRegEntry->RegName,
					       svalue, (int)pRegEntry->VarMax);
					svalue = (int32_t) pRegEntry->VarMax;
				}

				if (svalue < (int32_t) pRegEntry->VarMin) {
					hdd_warn("Reg Parameter %s < allowed Minimum "
					       "[%d < %d]. Enforcing Minimum", pRegEntry->RegName,
					       svalue, (int)pRegEntry->VarMin);
					svalue = (int32_t) pRegEntry->VarMin;
				}
			}
			/* Only if the parameter is set in the ini file, do the range check here */
			else if (match_status == QDF_STATUS_SUCCESS &&
				 pRegEntry->Flags &
					VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT) {
				if (svalue > (int32_t) pRegEntry->VarMax) {
					hdd_warn("Reg Parameter %s > allowed Maximum "
					       "[%d > %d]. Enforcing Default: %d", pRegEntry->RegName,
					       svalue, (int)pRegEntry->VarMax,
					       (int)pRegEntry->VarDefault);
					svalue =
						(int32_t) pRegEntry->VarDefault;
				}

				if (svalue < (int32_t) pRegEntry->VarMin) {
					hdd_warn("Reg Parameter %s < allowed Minimum "
					       "[%d < %d]. Enforcing Default: %d", pRegEntry->RegName,
					       svalue, (int)pRegEntry->VarMin,
					       (int)pRegEntry->VarDefault);
					svalue = pRegEntry->VarDefault;
				}
			}
			/* Move the variable into the output field. */
			memcpy(pField, &svalue, pRegEntry->VarSize);
		}
		/* Handle string parameters */
		else if (WLAN_PARAM_String == pRegEntry->RegType) {
#ifdef WLAN_CFG_DEBUG
			hdd_debug("RegName = %s, VarOffset %u VarSize %u VarDefault %s",
				  pRegEntry->RegName, pRegEntry->VarOffset,
				  pRegEntry->VarSize,
				  (char *)pRegEntry->VarDefault);
#endif

			if (match_status == QDF_STATUS_SUCCESS) {
				len_value_str = strlen(value_str);

				if (len_value_str > (pRegEntry->VarSize - 1)) {
					hdd_err("Invalid Value=[%s] specified for Name=[%s] in %s", value_str,
					       pRegEntry->RegName,
					       WLAN_INI_FILE);
					cbOutString =
						QDF_MIN(strlen
							 ((char *)pRegEntry->
								 VarDefault),
							 pRegEntry->VarSize - 1);
					memcpy(pField,
					       (void *)(pRegEntry->VarDefault),
					       cbOutString);
					((uint8_t *) pField)[cbOutString] =
						'\0';
				} else {
					memcpy(pField, (void *)(value_str),
					       len_value_str);
					((uint8_t *) pField)[len_value_str] =
						'\0';
				}
			} else {
				/* Failed to read the string parameter from the registry.  Use the default. */
				cbOutString =
					QDF_MIN(strlen((char *)pRegEntry->VarDefault),
						 pRegEntry->VarSize - 1);
				memcpy(pField, (void *)(pRegEntry->VarDefault),
				       cbOutString);
				((uint8_t *) pField)[cbOutString] = '\0';
			}
		} else if (WLAN_PARAM_MacAddr == pRegEntry->RegType) {
			if (pRegEntry->VarSize != QDF_MAC_ADDR_SIZE) {
				hdd_warn("Invalid VarSize %u for Name=[%s]", pRegEntry->VarSize,
				       pRegEntry->RegName);
				continue;
			}
			candidate = (char *)pRegEntry->VarDefault;
			if (match_status == QDF_STATUS_SUCCESS) {
				len_value_str = strlen(value_str);
				if (len_value_str != (QDF_MAC_ADDR_SIZE * 2)) {
					hdd_err("Invalid MAC addr [%s] specified for Name=[%s] in %s", value_str,
					       pRegEntry->RegName,
					       WLAN_INI_FILE);
				} else
					candidate = value_str;
			}
			/* parse the string and store it in the byte array */
			for (i = 0; i < QDF_MAC_ADDR_SIZE; i++) {
				((char *)pField)[i] =
					(char)(parse_hex_digit(candidate[i * 2]) *
					       16 +
					       parse_hex_digit(candidate[i * 2 + 1]));
			}
		} else {
			hdd_warn("Unknown param type for name[%s] in registry table", pRegEntry->RegName);
		}

		/* did we successfully parse a cfg item for this parameter? */
		if ((match_status == QDF_STATUS_SUCCESS) &&
		    (idx < MAX_CFG_INI_ITEMS)) {
			set_bit(idx, (void *)&hdd_ctx->config->bExplicitCfg);
		}
	}

	return ret_status;
}

/**
 * hdd_execute_config_command() - executes an arbitrary configuration command
 * @reg_table: the pointer to configuration table
 * @tableSize: the size of the configuration table
 * @ini_struct: pointer to the hdd config knob
 * @hdd_ctx: the pointer to hdd context
 * @command: the command to run
 *
 * Return: QDF_STATUS_SUCCESS if the command is found and able to execute,
 *		otherwise the appropriate QDF_STATUS will be returned
 */
static QDF_STATUS hdd_execute_config_command(struct reg_table_entry *reg_table,
					     unsigned long tableSize,
					     uint8_t *ini_struct,
					     struct hdd_context *hdd_ctx,
					     char *command)
{
	struct reg_table_entry *pRegEntry;
	char *clone;
	char *pCmd;
	void *pField;
	char *name;
	char *value_str;
	uint32_t value;
	int32_t svalue;
	size_t len_value_str;
	unsigned int idx;
	unsigned int i;
	QDF_STATUS vstatus;
	int rv;

	/* assume failure until proven otherwise */
	vstatus = QDF_STATUS_E_FAILURE;

	/* clone the command so that we can manipulate it */
	clone = kstrdup(command, GFP_ATOMIC);
	if (NULL == clone)
		return vstatus;

	/* 'clone' will point to the beginning of the string so it can be freed
	 * 'pCmd' will be used to walk/parse the command
	 */
	pCmd = clone;

	/* get rid of leading/trailing whitespace */
	pCmd = i_trim(pCmd);
	if ('\0' == *pCmd) {
		/* only whitespace */
		hdd_err("invalid command, only whitespace:[%s]", command);
		goto done;
	}
	/* parse the <name> = <value> */
	name = pCmd;
	while (('=' != *pCmd) && ('\0' != *pCmd))
		pCmd++;

	if ('\0' == *pCmd) {
		/* did not find '=' */
		hdd_err("invalid command, no '=':[%s]", command);
		goto done;
	}
	/* replace '=' with NUL to terminate the <name> */
	*pCmd++ = '\0';
	name = i_trim(name);
	if ('\0' == *name) {
		/* did not find a name */
		hdd_err("invalid command, no <name>:[%s]", command);
		goto done;
	}

	value_str = i_trim(pCmd);
	if ('\0' == *value_str) {
		/* did not find a value */
		hdd_err("invalid command, no <value>:[%s]", command);
		goto done;
	}
	/* lookup the configuration item */
	for (idx = 0; idx < tableSize; idx++) {
		if (0 == strcmp(name, reg_table[idx].RegName)) {
			/* found a match */
			break;
		}
	}
	if (tableSize == idx) {
		/* did not match the name */
		hdd_err("invalid command, unknown configuration item:[%s]", command);
		goto done;
	}

	pRegEntry = &reg_table[idx];
	if (!(pRegEntry->Flags & VAR_FLAGS_DYNAMIC_CFG)) {
		/* does not support dynamic configuration */
		hdd_err("Global_Registry_Table. %s does not support "
		       "dynamic configuration", name);
		vstatus = QDF_STATUS_E_PERM;
		goto done;
	}

	pField = ini_struct + pRegEntry->VarOffset;

	switch (pRegEntry->RegType) {
	case WLAN_PARAM_Integer:
		rv = kstrtou32(value_str, 10, &value);
		if (rv < 0)
			goto done;
		if (value < pRegEntry->VarMin) {
			/* out of range */
			hdd_err("Invalid command, value %u < min value %lu", value, pRegEntry->VarMin);
			goto done;
		}
		if (value > pRegEntry->VarMax) {
			/* out of range */
			hdd_err("Invalid command, value %u > max value %lu", value, pRegEntry->VarMax);
			goto done;
		}
		memcpy(pField, &value, pRegEntry->VarSize);
		break;

	case WLAN_PARAM_HexInteger:
		rv = kstrtou32(value_str, 16, &value);
		if (rv < 0)
			goto done;
		if (value < pRegEntry->VarMin) {
			/* out of range */
			hdd_err("Invalid command, value %x < min value %lx", value, pRegEntry->VarMin);
			goto done;
		}
		if (value > pRegEntry->VarMax) {
			/* out of range */
			hdd_err("Invalid command, value %x > max value %lx", value, pRegEntry->VarMax);
			goto done;
		}
		memcpy(pField, &value, pRegEntry->VarSize);
		break;

	case WLAN_PARAM_SignedInteger:
		rv = kstrtos32(value_str, 10, &svalue);
		if (rv < 0)
			goto done;
		if (svalue < (int32_t) pRegEntry->VarMin) {
			/* out of range */
			hdd_err("Invalid command, value %d < min value %d", svalue, (int)pRegEntry->VarMin);
			goto done;
		}
		if (svalue > (int32_t) pRegEntry->VarMax) {
			/* out of range */
			hdd_err("Invalid command, value %d > max value %d", svalue, (int)pRegEntry->VarMax);
			goto done;
		}
		memcpy(pField, &svalue, pRegEntry->VarSize);
		break;

	case WLAN_PARAM_String:
		len_value_str = strlen(value_str);
		if (len_value_str > (pRegEntry->VarSize - 1)) {
			/* too big */
			hdd_err("Invalid command, string [%s] length "
			       "%zu exceeds maximum length %u", value_str,
			       len_value_str, (pRegEntry->VarSize - 1));
			goto done;
		}
		/* copy string plus NUL */
		memcpy(pField, value_str, (len_value_str + 1));
		break;

	case WLAN_PARAM_MacAddr:
		len_value_str = strlen(value_str);
		if (len_value_str != (QDF_MAC_ADDR_SIZE * 2)) {
			/* out of range */
			hdd_err("Invalid command, MAC address [%s] length "
			       "%zu is not expected length %u", value_str,
			       len_value_str, (QDF_MAC_ADDR_SIZE * 2));
			goto done;
		}
		/* parse the string and store it in the byte array */
		for (i = 0; i < QDF_MAC_ADDR_SIZE; i++) {
			((char *)pField)[i] = (char)
					      ((parse_hex_digit(value_str[(i * 2)]) * 16) +
					       parse_hex_digit(value_str[(i * 2) + 1]));
		}
		break;

	default:
		goto done;
	}

	/* if we get here, we had a successful modification */
	vstatus = QDF_STATUS_SUCCESS;

	/* config table has been modified, is there a notifier? */
	if (NULL != pRegEntry->pfnDynamicnotify)
		(pRegEntry->pfnDynamicnotify)(hdd_ctx, pRegEntry->notifyId);

	/* note that this item was explicitly configured */
	if (idx < MAX_CFG_INI_ITEMS)
		set_bit(idx, (void *)&hdd_ctx->config->bExplicitCfg);

done:
	kfree(clone);
	return vstatus;
}

/**
 * hdd_set_power_save_offload_config() - set power save offload configuration
 * @hdd_ctx: the pointer to hdd context
 *
 * Return: none
 */
static void hdd_set_power_save_offload_config(struct hdd_context *hdd_ctx)
{
	struct hdd_config *pConfig = hdd_ctx->config;
	uint32_t listenInterval = 0;

	if (strcmp(pConfig->PowerUsageControl, "Min") == 0)
		listenInterval = pConfig->nBmpsMinListenInterval;
	else if (strcmp(pConfig->PowerUsageControl, "Max") == 0)
		listenInterval = pConfig->nBmpsMaxListenInterval;

	/*
	 * Based on Mode Set the LI
	 * Otherwise default LI value of 1 will
	 * be taken
	 */
	if (listenInterval) {
		/*
		 * setcfg for listenInterval.
		 * Make sure CFG is updated because PE reads this
		 * from CFG at the time of assoc or reassoc
		 */
		sme_cfg_set_int(hdd_ctx->mac_handle, WNI_CFG_LISTEN_INTERVAL,
				listenInterval);
	}

}

/**
 * hdd_update_mac_config() - update MAC address from cfg file
 * @hdd_ctx: the pointer to hdd context
 *
 * It overwrites the MAC address if config file exist.
 *
 * Return: QDF_STATUS_SUCCESS if the MAC address is found from cfg file
 *      and overwritten, otherwise QDF_STATUS_E_INVAL
 */
QDF_STATUS hdd_update_mac_config(struct hdd_context *hdd_ctx)
{
	int status, i = 0;
	const struct firmware *fw = NULL;
	char *line, *buffer = NULL;
	char *temp = NULL;
	char *name, *value;
	int max_mac_addr = QDF_MAX_CONCURRENCY_PERSONA;
	struct hdd_cfg_entry macTable[QDF_MAX_CONCURRENCY_PERSONA];
	tSirMacAddr customMacAddr;

	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	memset(macTable, 0, sizeof(macTable));
	status = request_firmware(&fw, WLAN_MAC_FILE, hdd_ctx->parent_dev);
	if (status) {
		/*
		 * request_firmware "fails" if the file is not found, which is a
		 * valid setup for us, so log using debug instead of error
		 */
		hdd_debug("request_firmware failed; status:%d", status);
		return QDF_STATUS_E_FAILURE;
	}

	if (!fw || !fw->data || !fw->size) {
		hdd_alert("invalid firmware");
		qdf_status = QDF_STATUS_E_INVAL;
		goto config_exit;
	}

	hdd_debug("wlan_mac.bin size %zu", fw->size);

	temp = qdf_mem_malloc(fw->size + 1);

	if (temp == NULL) {
		hdd_err("fail to alloc memory");
		qdf_status = QDF_STATUS_E_NOMEM;
		goto config_exit;
	}
	buffer = temp;
	qdf_mem_copy(buffer, fw->data, fw->size);
	buffer[fw->size] = 0x0;

	/* data format:
	 * Intf0MacAddress=00AA00BB00CC
	 * Intf1MacAddress=00AA00BB00CD
	 * END
	 */
	while (buffer != NULL) {
		line = get_next_line(buffer);
		buffer = i_trim(buffer);

		if (strlen((char *)buffer) == 0 || *buffer == '#') {
			buffer = line;
			continue;
		}
		if (strncmp(buffer, "END", 3) == 0)
			break;

		name = buffer;
		buffer = strnchr(buffer, strlen(buffer), '=');
		if (buffer) {
			*buffer++ = '\0';
			i_trim(name);
			if (strlen(name) != 0) {
				buffer = i_trim(buffer);
				if (strlen(buffer) == 12) {
					value = buffer;
					macTable[i].name = name;
					macTable[i++].value = value;
					if (i >= QDF_MAX_CONCURRENCY_PERSONA)
						break;
				}
			}
		}
		buffer = line;
	}

	if (i <= QDF_MAX_CONCURRENCY_PERSONA) {
		hdd_debug("%d Mac addresses provided", i);
	} else {
		hdd_err("invalid number of Mac address provided, nMac = %d", i);
		qdf_status = QDF_STATUS_E_INVAL;
		goto config_exit;
	}

	update_mac_from_string(hdd_ctx, &macTable[0], i);
	hdd_debug("Populating remaining %d Mac addresses",
		   max_mac_addr - i);
	hdd_populate_random_mac_addr(hdd_ctx, max_mac_addr - i);

	qdf_mem_copy(&customMacAddr,
		     &hdd_ctx->config->intfMacAddr[0].bytes[0],
		     sizeof(tSirMacAddr));
	sme_set_custom_mac_addr(customMacAddr);

config_exit:
	qdf_mem_free(temp);
	release_firmware(fw);
	return qdf_status;
}

/**
 * hdd_disable_runtime_pm() - Override to disable runtime_pm.
 * @cfg_ini: Handle to struct hdd_config
 *
 * Return: None
 */
#ifdef FEATURE_RUNTIME_PM
static void hdd_disable_runtime_pm(struct hdd_config *cfg_ini)
{
	cfg_ini->runtime_pm = 0;
}
#else
static void hdd_disable_runtime_pm(struct hdd_config *cfg_ini)
{
}
#endif

/**
 * hdd_disable_auto_shutdown() - Override to disable auto_shutdown.
 * @cfg_ini: Handle to struct hdd_config
 *
 * Return: None
 */
#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
static void hdd_disable_auto_shutdown(struct hdd_config *cfg_ini)
{
	cfg_ini->WlanAutoShutdown = 0;
}
#else
static void hdd_disable_auto_shutdown(struct hdd_config *cfg_ini)
{
}
#endif

/**
 * hdd_override_all_ps() - overrides to disables all the powersave features.
 * @hdd_ctx: Pointer to HDD context.
 * Overrides below powersave ini configurations.
 * gEnableImps=0
 * gEnableBmps=0
 * gRuntimePM=0
 * gWlanAutoShutdown = 0
 * gEnableSuspend=0
 * gEnablePowerSaveOffload=0
 * gEnableWoW=0
 *
 * Return: None
 */
static void hdd_override_all_ps(struct hdd_context *hdd_ctx)
{
	struct hdd_config *cfg_ini = hdd_ctx->config;

	cfg_ini->fIsImpsEnabled = 0;
	cfg_ini->is_ps_enabled = 0;
	hdd_disable_runtime_pm(cfg_ini);
	hdd_disable_auto_shutdown(cfg_ini);
	cfg_ini->enablePowersaveOffload = 0;
	cfg_ini->wowEnable = 0;
}

/**
 * hdd_set_rx_mode_value() - set rx_mode values
 * @hdd_ctx: hdd context
 *
 * Return: none
 */
static void hdd_set_rx_mode_value(struct hdd_context *hdd_ctx)
{
	/* RPS has higher priority than dynamic RPS when both bits are set */
	if (hdd_ctx->config->rx_mode & CFG_ENABLE_RPS &&
	    hdd_ctx->config->rx_mode & CFG_ENABLE_DYNAMIC_RPS)
		hdd_ctx->config->rx_mode &= ~CFG_ENABLE_DYNAMIC_RPS;

	if (hdd_ctx->config->rx_mode & CFG_ENABLE_RX_THREAD &&
		 hdd_ctx->config->rx_mode & CFG_ENABLE_RPS) {
		hdd_warn("rx_mode wrong configuration. Make it default");
		hdd_ctx->config->rx_mode = CFG_RX_MODE_DEFAULT;
	}

	if (hdd_ctx->config->rx_mode & CFG_ENABLE_RX_THREAD)
		hdd_ctx->enable_rxthread = true;
	else if (hdd_ctx->config->rx_mode & CFG_ENABLE_DP_RX_THREADS)
		hdd_ctx->enable_dp_rx_threads = true;

	if (hdd_ctx->config->rx_mode & CFG_ENABLE_RPS)
		hdd_ctx->rps = true;

	if (hdd_ctx->config->rx_mode & CFG_ENABLE_NAPI)
		hdd_ctx->napi_enable = true;

	if (hdd_ctx->config->rx_mode & CFG_ENABLE_DYNAMIC_RPS)
		hdd_ctx->dynamic_rps = true;

	hdd_info("rx_mode:%u dp_rx_threads:%u rx_thread:%u napi:%u rps:%u dynamic rps %u",
		 hdd_ctx->config->rx_mode, hdd_ctx->enable_dp_rx_threads,
		 hdd_ctx->enable_rxthread, hdd_ctx->napi_enable,
		 hdd_ctx->rps, hdd_ctx->dynamic_rps);
}

/**
 * hdd_parse_config_ini() - parse the ini configuration file
 * @hdd_ctx: the pointer to hdd context
 *
 * This function reads the qcom_cfg.ini file and
 * parses each 'Name=Value' pair in the ini file
 *
 * Return: QDF_STATUS_SUCCESS if the qcom_cfg.ini is correctly read,
 *		otherwise QDF_STATUS_E_INVAL
 */
QDF_STATUS hdd_parse_config_ini(struct hdd_context *hdd_ctx)
{
	int status = 0;
	int i = 0;
	int retry = 0;
	/** Pointer for firmware image data */
	const struct firmware *fw = NULL;
	char *buffer, *line, *pTemp = NULL;
	size_t size;
	char *name, *value;
	struct hdd_cfg_entry *cfg_ini_table;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	size = MAX_CFG_INI_ITEMS * sizeof(*cfg_ini_table);
	cfg_ini_table = qdf_mem_malloc(size);

	if (!cfg_ini_table) {
		hdd_err("Failed to alloc %zu bytes for cfg_ini_table", size);
		return QDF_STATUS_E_NOMEM;
	}

	do {
		if (status == -EAGAIN)
			msleep(HDD_CFG_REQUEST_FIRMWARE_DELAY);

		status = request_firmware(&fw, WLAN_INI_FILE,
					  hdd_ctx->parent_dev);

		retry++;
	} while ((retry < HDD_CFG_REQUEST_FIRMWARE_RETRIES) &&
		 (status == -EAGAIN));

	if (status) {
		hdd_alert("request_firmware failed %d", status);
		qdf_status = QDF_STATUS_E_FAILURE;
		goto config_exit;
	}
	if (!fw || !fw->data || !fw->size) {
		hdd_alert("%s download failed", WLAN_INI_FILE);
		qdf_status = QDF_STATUS_E_FAILURE;
		goto config_exit;
	}

	hdd_debug("qcom_cfg.ini Size %zu", fw->size);

	buffer = (char *)qdf_mem_malloc(fw->size);

	if (NULL == buffer) {
		hdd_err("qdf_mem_malloc failure");
		qdf_status = QDF_STATUS_E_NOMEM;
		goto config_exit;
	}
	pTemp = buffer;

	qdf_mem_copy((void *)buffer, (void *)fw->data, fw->size);
	size = fw->size;

	while (buffer != NULL) {
		line = get_next_line(buffer);
		buffer = i_trim(buffer);

		hdd_debug("%s: item", buffer);

		if (strlen((char *)buffer) == 0 || *buffer == '#') {
			buffer = line;
			continue;
		}

		if (strncmp(buffer, "END", 3) == 0)
			break;

		name = buffer;
		while (*buffer != '=' && *buffer != '\0')
			buffer++;
		if (*buffer != '\0') {
			*buffer++ = '\0';
			i_trim(name);
			if (strlen(name) != 0) {
				buffer = i_trim(buffer);
				if (strlen(buffer) > 0) {
					value = buffer;
					while (*buffer != '\0')
						buffer++;
					*buffer = '\0';
					cfg_ini_table[i].name = name;
					cfg_ini_table[i++].value = value;
					if (i >= MAX_CFG_INI_ITEMS) {
						hdd_err("Number of items in %s > %d",
							WLAN_INI_FILE,
							MAX_CFG_INI_ITEMS);
						break;
					}
				}
			}
		}
		buffer = line;
	}

	/* Loop through the registry table and apply all these configs */
	qdf_status = hdd_apply_cfg_ini(hdd_ctx, cfg_ini_table, i);
	hdd_set_rx_mode_value(hdd_ctx);
	if (QDF_GLOBAL_MONITOR_MODE == cds_get_conparam())
		hdd_override_all_ps(hdd_ctx);

config_exit:
	release_firmware(fw);
	qdf_mem_free(pTemp);
	qdf_mem_free(cfg_ini_table);
	return qdf_status;
}

/**
 * hdd_cfg_xlate_to_csr_phy_mode() - convert PHY mode
 * @dot11Mode: the mode to convert
 *
 * Convert the configuration PHY mode to CSR PHY mode
 *
 * Return: the CSR phy mode value
 */
eCsrPhyMode hdd_cfg_xlate_to_csr_phy_mode(enum hdd_dot11_mode dot11Mode)
{
	if (cds_is_sub_20_mhz_enabled())
		return eCSR_DOT11_MODE_abg;

	switch (dot11Mode) {
	case (eHDD_DOT11_MODE_abg):
		return eCSR_DOT11_MODE_abg;
	case (eHDD_DOT11_MODE_11b):
		return eCSR_DOT11_MODE_11b;
	case (eHDD_DOT11_MODE_11g):
		return eCSR_DOT11_MODE_11g;
	default:
	case (eHDD_DOT11_MODE_11n):
		return eCSR_DOT11_MODE_11n;
	case (eHDD_DOT11_MODE_11g_ONLY):
		return eCSR_DOT11_MODE_11g_ONLY;
	case (eHDD_DOT11_MODE_11n_ONLY):
		return eCSR_DOT11_MODE_11n_ONLY;
	case (eHDD_DOT11_MODE_11b_ONLY):
		return eCSR_DOT11_MODE_11b_ONLY;
	case (eHDD_DOT11_MODE_11ac_ONLY):
		return eCSR_DOT11_MODE_11ac_ONLY;
	case (eHDD_DOT11_MODE_11ac):
		return eCSR_DOT11_MODE_11ac;
	case (eHDD_DOT11_MODE_AUTO):
		return eCSR_DOT11_MODE_AUTO;
	case (eHDD_DOT11_MODE_11a):
		return eCSR_DOT11_MODE_11a;
	case (eHDD_DOT11_MODE_11ax_ONLY):
		return eCSR_DOT11_MODE_11ax_ONLY;
	case (eHDD_DOT11_MODE_11ax):
		return eCSR_DOT11_MODE_11ax;
	}

}

/**
 * hdd_set_idle_ps_config() - set idle power save configuration
 * @hdd_ctx: the pointer to hdd context
 * @val: the value to configure
 *
 * Return: QDF_STATUS_SUCCESS if command set correctly,
 *		otherwise the QDF_STATUS return from SME layer
 */
QDF_STATUS hdd_set_idle_ps_config(struct hdd_context *hdd_ctx, bool val)
{
	QDF_STATUS status;

	hdd_debug("Enter Val %d", val);

	if (hdd_ctx->imps_enabled == val) {
		hdd_info("Already in the requested power state:%d", val);
		return QDF_STATUS_SUCCESS;
	}

	status = sme_set_idle_powersave_config(val);
	if (QDF_STATUS_SUCCESS != status) {
		hdd_err("Fail to Set Idle PS Config val %d", val);
		return status;
	}

	hdd_ctx->imps_enabled = val;

	return status;
}

/**
 * hdd_set_fine_time_meas_cap() - set fine timing measurement capability
 * @hdd_ctx: HDD context
 *
 * This function is used to pass fine timing measurement capability coming
 * from INI to SME. This function make sure that configure INI is supported
 * by the device. Use bit mask to mask out the unsupported capabilities.
 *
 * Return: None
 */
static void hdd_set_fine_time_meas_cap(struct hdd_context *hdd_ctx)
{
	struct hdd_config *config = hdd_ctx->config;
	uint32_t capability = config->fine_time_meas_cap;

	/* Make sure only supported capabilities are enabled in INI */
	capability &= CFG_FINE_TIME_MEAS_CAPABILITY_MAX;
	ucfg_wifi_pos_set_ftm_cap(hdd_ctx->hdd_psoc, capability);

	hdd_debug("fine time meas capability - INI: %04x Enabled: %04x",
		config->fine_time_meas_cap,
		capability);
}

/**
 * hdd_convert_string_to_u8_array() - used to convert string into u8 array
 * @str: String to be converted
 * @hex_array: Array where converted value is stored
 * @len: Length of the populated array
 * @array_max_len: Maximum length of the array
 * @to_hex: true, if conversion required for hex string
 *
 * This API is called to convert string (each byte separated by
 * a comma) into an u8 array
 *
 * Return: QDF_STATUS
 */

static QDF_STATUS hdd_convert_string_to_array(char *str, uint8_t *array,
			     uint8_t *len, uint16_t array_max_len, bool to_hex)
{
	char *format, *s = str;

	if (str == NULL || array == NULL || len == NULL)
		return QDF_STATUS_E_INVAL;

	format = (to_hex) ? "%02x" : "%d";

	*len = 0;
	while ((s != NULL) && (*len < array_max_len)) {
		int val;
		/* Increment length only if sscanf successfully extracted
		 * one element. Any other return value means error.
		 * Ignore it.
		 */
		if (sscanf(s, format, &val) == 1) {
			array[*len] = (uint8_t) val;
			*len += 1;
		}

		s = strpbrk(s, ",");
		if (s)
			s++;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * hdd_hex_string_to_u8_array() - used to convert hex string into u8 array
 * @str: Hexadecimal string
 * @hex_array: Array where converted value is stored
 * @len: Length of the populated array
 * @array_max_len: Maximum length of the array
 *
 * This API is called to convert hexadecimal string (each byte separated by
 * a comma) into an u8 array
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS hdd_hex_string_to_u8_array(char *str, uint8_t *hex_array,
					     uint8_t *len,
					     uint8_t array_max_len)
{
	return hdd_convert_string_to_array(str, hex_array, len,
					   array_max_len, true);
}

QDF_STATUS hdd_string_to_u8_array(char *str, uint8_t *array,
				  uint8_t *len, uint16_t array_max_len)
{
	return hdd_convert_string_to_array(str, array, len,
					   array_max_len, false);
}

/**
 * hdd_hex_string_to_u16_array() - convert a hex string to a uint16 array
 * @str: input string
 * @int_array: pointer to input array of type uint16
 * @len: pointer to number of elements which the function adds to the array
 * @int_array_max_len: maximum number of elements in input uint16 array
 *
 * This function is used to convert a space separated hex string to an array of
 * uint16_t. For example, an input string str = "a b c d" would be converted to
 * a unint16 array, int_array = {0xa, 0xb, 0xc, 0xd}, *len = 4.
 * This assumes that input value int_array_max_len >= 4.
 *
 * Return: QDF_STATUS_SUCCESS - if the conversion is successful
 *         non zero value     - if the conversion is a failure
 */
QDF_STATUS hdd_hex_string_to_u16_array(char *str,
		uint16_t *int_array, uint8_t *len, uint8_t int_array_max_len)
{
	char *s = str;
	uint32_t val = 0;

	if (str == NULL || int_array == NULL || len == NULL)
		return QDF_STATUS_E_INVAL;

	hdd_debug("str %pK intArray %pK intArrayMaxLen %d",
		s, int_array, int_array_max_len);

	*len = 0;

	while ((s != NULL) && (*len < int_array_max_len)) {
		/*
		 * Increment length only if sscanf successfully extracted one
		 * element. Any other return value means error. Ignore it.
		 */
		if (sscanf(s, "%x", &val) == 1) {
			int_array[*len] = (uint16_t) val;
			hdd_debug("s %pK val %x intArray[%d]=0x%x",
				s, val, *len, int_array[*len]);
			*len += 1;
		}
		s = strpbrk(s, " ");
		if (s)
			s++;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * hdd_update_vht_cap_in_cfg() - to update VHT cap in global CFG
 * @hdd_ctx: pointer to hdd context
 *
 * This API will update the VHT config in CFG after taking intersection
 * of INI and firmware capabilities provided reading CFG
 *
 * Return: true or false
 */
static bool hdd_update_vht_cap_in_cfg(struct hdd_context *hdd_ctx)
{
	bool status = true;
	uint32_t val;
	struct hdd_config *config = hdd_ctx->config;
	mac_handle_t mac_handle = hdd_ctx->mac_handle;

	if (sme_cfg_set_int(mac_handle, WNI_CFG_VHT_ENABLE_TXBF_20MHZ,
			    config->enableTxBFin20MHz) ==
			QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Couldn't set value for WNI_CFG_VHT_ENABLE_TXBF_20MHZ");
	}
	/* Based on cfg.ini, update the Basic MCS set, RX/TX MCS map
	 * in the cfg.dat. Valid values are 0(MCS0-7), 1(MCS0-8), 2(MCS0-9)
	 * we update only the least significant 2 bits in the
	 * corresponding fields.
	 */
	if ((config->dot11Mode == eHDD_DOT11_MODE_AUTO) ||
	    (config->dot11Mode == eHDD_DOT11_MODE_11ac_ONLY) ||
	    (config->dot11Mode == eHDD_DOT11_MODE_11ac)) {
		/* Currently shortGI40Mhz is used for shortGI80Mhz and 160MHz*/
		if (sme_cfg_set_int(mac_handle, WNI_CFG_VHT_SHORT_GI_80MHZ,
			config->ShortGI40MhzEnable) == QDF_STATUS_E_FAILURE) {
			status = false;
			hdd_err("Couldn't pass WNI_VHT_SHORT_GI_80MHZ to CFG");
		}

		if (sme_cfg_set_int(mac_handle,
			WNI_CFG_VHT_SHORT_GI_160_AND_80_PLUS_80MHZ,
			config->ShortGI40MhzEnable) == QDF_STATUS_E_FAILURE) {
			status = false;
			hdd_err("Couldn't pass SHORT_GI_160MHZ to CFG");
		}

		/* Hardware is capable of doing
		 * 128K AMPDU in 11AC mode
		 */
		if (sme_cfg_set_int(mac_handle,
			     WNI_CFG_VHT_AMPDU_LEN_EXPONENT,
			     config->fVhtAmpduLenExponent) ==
			    QDF_STATUS_E_FAILURE) {
			status = false;
			hdd_err("Couldn't pass on WNI_CFG_VHT_AMPDU_LEN_EXPONENT to CFG");
		}
		/* Change MU Bformee only when TxBF is enabled */
		if (config->enableTxBF) {
			sme_cfg_get_int(mac_handle,
				WNI_CFG_VHT_MU_BEAMFORMEE_CAP, &val);

			if (val != config->enableMuBformee) {
				if (sme_cfg_set_int(mac_handle,
					    WNI_CFG_VHT_MU_BEAMFORMEE_CAP,
					    config->enableMuBformee
					    ) == QDF_STATUS_E_FAILURE) {
					status = false;
					hdd_err("Couldn't pass on WNI_CFG_VHT_MU_BEAMFORMEE_CAP to CFG");
				}
			}
		}
		if (sme_cfg_set_int(mac_handle, WNI_CFG_VHT_MAX_MPDU_LENGTH,
			    config->vhtMpduLen) == QDF_STATUS_E_FAILURE) {
			status = false;
			hdd_err("Couldn't pass on WNI_CFG_VHT_MAX_MPDU_LENGTH to CFG");
		}

		if (config->enable2x2 && config->enable_su_tx_bformer) {
			if (sme_cfg_set_int(mac_handle,
					WNI_CFG_VHT_SU_BEAMFORMER_CAP,
					config->enable_su_tx_bformer) ==
				QDF_STATUS_E_FAILURE) {
				status = false;
				hdd_err("set SU_BEAMFORMER_CAP to CFG failed");
			}
			if (sme_cfg_set_int(mac_handle,
					WNI_CFG_VHT_NUM_SOUNDING_DIMENSIONS,
					NUM_OF_SOUNDING_DIMENSIONS) ==
				QDF_STATUS_E_FAILURE) {
				status = false;
				hdd_err("failed to set NUM_OF_SOUNDING_DIM");
			}
		}
	}

	if (sme_cfg_set_int(mac_handle, WNI_CFG_VHT_RXSTBC,
			    config->enableRxSTBC) == QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Couldn't pass on WNI_CFG_VHT_RXSTBC to CFG");
	}

	if (sme_cfg_set_int(mac_handle, WNI_CFG_VHT_TXSTBC,
			    config->enableTxSTBC) == QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Couldn't pass on WNI_CFG_VHT_TXSTBC to CFG");
	}

	/* first get HW RX LDPC capability */
	if (sme_cfg_get_int(mac_handle, WNI_CFG_VHT_LDPC_CODING_CAP, &val) ==
							QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Could not get WNI_CFG_VHT_LDPC_CODING_CAP");
	}

	/* enable RX LDPC only when both INI and HW are enabled */
	if (sme_cfg_set_int(mac_handle, WNI_CFG_VHT_LDPC_CODING_CAP,
				config->enable_rx_ldpc && val) ==
			QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Couldn't pass on WNI_CFG_VHT_LDPC_CODING_CAP to CFG");
	}

	if (sme_cfg_set_int(mac_handle,
		WNI_CFG_VHT_CSN_BEAMFORMEE_ANT_SUPPORTED,
		config->txBFCsnValue) ==
			QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Couldn't pass on WNI_CFG_VHT_CSN_BEAMFORMEE_ANT_SUPPORTED to CFG");
	}
	return status;

}

/**
 * hdd_update_config_cfg() - API to update INI setting based on hw/fw caps
 * @hdd_ctx: pointer to hdd_ctx
 *
 * This API reads the cfg file which is updated with hardware/firmware
 * capabilities and intersect it with INI setting provided by user. After
 * taking intersection it adjust cfg it self. For example, if user has enabled
 * RX LDPC through INI but hardware/firmware doesn't support it then disable
 * it in CFG file here.
 *
 * Return: true or false based on outcome.
 */
bool hdd_update_config_cfg(struct hdd_context *hdd_ctx)
{
	bool status = true;
	struct hdd_config *config = hdd_ctx->config;
	mac_handle_t mac_handle;

	/*
	 * During the initialization both 2G and 5G capabilities should be same.
	 * So read 5G HT capablity and update 2G and 5G capablities.
	 */

	if (!hdd_update_vht_cap_in_cfg(hdd_ctx)) {
		status = false;
		hdd_err("Couldn't set VHT CAP in cfg");
	}

	if (0 != hdd_update_he_cap_in_cfg(hdd_ctx)) {
		status = false;
		hdd_err("Couldn't set HE CAP in cfg");
	}

	mac_handle = hdd_ctx->mac_handle;

	if (sme_cfg_set_int(mac_handle, WNI_CFG_MAX_RX_AMPDU_FACTOR,
			    config->MaxRxAmpduFactor) ==
			QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Couldn't pass on WNI_CFG_HT_AMPDU_PARAMS_MAX_RX_AMPDU_FACTOR to CFG");
	}

	if (sme_cfg_set_int(mac_handle, WNI_CFG_MPDU_DENSITY,
			    config->ht_mpdu_density) ==
			QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Couldn't pass on WNI_CFG_MPDU_DENSITY to CFG");
	}

	if (sme_cfg_set_int(mac_handle, WNI_CFG_SHORT_PREAMBLE,
		     config->fIsShortPreamble) == QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Couldn't pass on WNI_CFG_SHORT_PREAMBLE to CFG");
	}

	if (sme_cfg_set_int(mac_handle,
				WNI_CFG_PASSIVE_MAXIMUM_CHANNEL_TIME,
				config->nPassiveMaxChnTime)
				== QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Couldn't pass on WNI_CFG_PASSIVE_MAXIMUM_CHANNEL_TIME to CFG");
	}

	if (sme_cfg_set_int(mac_handle, WNI_CFG_BEACON_INTERVAL,
		     config->nBeaconInterval) == QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Couldn't pass on WNI_CFG_BEACON_INTERVAL to CFG");
	}

	if (sme_cfg_set_int(mac_handle, WNI_CFG_MAX_PS_POLL,
		     config->nMaxPsPoll) == QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Couldn't pass on WNI_CFG_MAX_PS_POLL to CFG");
	}

	if (sme_cfg_set_int(mac_handle, WNI_CFG_PS_DATA_INACTIVITY_TIMEOUT,
		    config->nDataInactivityTimeout) == QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Couldn't pass on WNI_CFG_PS_DATA_INACTIVITY_TIMEOUT to CFG");
	}

	if (sme_cfg_set_int(mac_handle,
		WNI_CFG_PS_WOW_DATA_INACTIVITY_TIMEOUT,
		config->wow_data_inactivity_timeout) == QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Fail to pass WNI_CFG_PS_WOW_DATA_INACTIVITY_TO CFG");
	}

	if (sme_cfg_set_int(mac_handle, WNI_CFG_ENABLE_LTE_COEX,
		     config->enableLTECoex) == QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Couldn't pass on WNI_CFG_ENABLE_LTE_COEX to CFG");
	}

	if (sme_cfg_set_int(mac_handle, WNI_CFG_AP_KEEP_ALIVE_TIMEOUT,
		    config->apKeepAlivePeriod) == QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Couldn't pass on WNI_CFG_AP_KEEP_ALIVE_TIMEOUT to CFG");
	}

	if (sme_cfg_set_int(mac_handle, WNI_CFG_AP_LINK_MONITOR_TIMEOUT,
		    config->apLinkMonitorPeriod) == QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Couldn't pass on WNI_CFG_AP_LINK_MONITOR_TIMEOUT to CFG");
	}

	if (sme_cfg_set_int(mac_handle, WNI_CFG_SINGLE_TID_RC,
		    config->bSingleTidRc) == QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Couldn't pass on WNI_CFG_SINGLE_TID_RC to CFG");
	}

	if (sme_cfg_set_int(mac_handle, WNI_CFG_TELE_BCN_WAKEUP_EN,
		    config->teleBcnWakeupEn) == QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Couldn't pass on WNI_CFG_TELE_BCN_WAKEUP_EN to CFG");
	}

	if (sme_cfg_set_int(mac_handle, WNI_CFG_TELE_BCN_MAX_LI,
		    config->nTeleBcnMaxListenInterval) ==
		    QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Couldn't pass on WNI_CFG_TELE_BCN_MAX_LI to CFG");
	}

	if (sme_cfg_set_int(mac_handle, WNI_CFG_INFRA_STA_KEEP_ALIVE_PERIOD,
		    config->infraStaKeepAlivePeriod) ==
		    QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Couldn't pass on WNI_CFG_INFRA_STA_KEEP_ALIVE_PERIOD to CFG");
	}

	if (sme_cfg_set_int(mac_handle, WNI_CFG_FRAGMENTATION_THRESHOLD,
		    config->FragmentationThreshold) == QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Couldn't pass on WNI_CFG_FRAGMENTATION_THRESHOLD to CFG");
	}
	if (sme_cfg_set_int(mac_handle, WNI_CFG_RTS_THRESHOLD,
		     config->RTSThreshold) == QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Couldn't pass on WNI_CFG_RTS_THRESHOLD to CFG");
	}

	if (sme_cfg_set_int(mac_handle, WNI_CFG_11D_ENABLED,
		     config->Is11dSupportEnabled) == QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Couldn't pass on WNI_CFG_11D_ENABLED to CFG");
	}

	if (sme_cfg_set_int(mac_handle, WNI_CFG_DFS_MASTER_ENABLED,
			    config->enableDFSMasterCap) ==
			QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Failure: Couldn't set value for WNI_CFG_DFS_MASTER_ENABLED");
	}

	if (sme_cfg_set_int(mac_handle, WNI_CFG_HEART_BEAT_THRESHOLD,
		    config->HeartbeatThresh24) == QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Couldn't pass on WNI_CFG_HEART_BEAT_THRESHOLD to CFG");
	}

	if (sme_cfg_set_int(mac_handle, WNI_CFG_ENABLE_MC_ADDR_LIST,
		    config->fEnableMCAddrList) == QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Couldn't pass on WNI_CFG_ENABLE_MC_ADDR_LIST to CFG");
	}

	if (sme_cfg_set_int(mac_handle, WNI_CFG_ENABLE_MCC_ADAPTIVE_SCHED,
		    config->enableMCCAdaptiveScheduler) ==
		    QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Couldn't pass on WNI_CFG_ENABLE_MCC_ADAPTIVE_SCHED to CFG");
	}
	if (sme_cfg_set_int(mac_handle, WNI_CFG_DISABLE_LDPC_WITH_TXBF_AP,
		    config->disableLDPCWithTxbfAP) == QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Couldn't pass on WNI_CFG_DISABLE_LDPC_WITH_TXBF_AP to CFG");
	}

#ifdef WLAN_FEATURE_11W
	if (sme_cfg_set_int(mac_handle, WNI_CFG_PMF_SA_QUERY_MAX_RETRIES,
			    config->pmfSaQueryMaxRetries) ==
			QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Couldn't pass on WNI_CFG_SA_QUERY_MAX_RETRIES to CFG");
	}

	if (sme_cfg_set_int(mac_handle, WNI_CFG_PMF_SA_QUERY_RETRY_INTERVAL,
			    config->pmfSaQueryRetryInterval) ==
			QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Couldn't pass on WNI_CFG_SA_QUERY_RETRY_INTERVAL to CFG");
	}
#endif

	if (sme_cfg_set_int(mac_handle, WNI_CFG_IBSS_ATIM_WIN_SIZE,
			    config->ibssATIMWinSize) ==
			QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Couldn't pass on WNI_CFG_IBSS_ATIM_WIN_SIZE to CFG");
	}

	if (sme_cfg_set_int(mac_handle, WNI_CFG_TGT_GTX_USR_CFG,
	    config->tgt_gtx_usr_cfg) == QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Couldn't pass on WNI_CFG_TGT_GTX_USR_CFG to CCM");
	}

	if (sme_cfg_set_int(mac_handle, WNI_CFG_RATE_FOR_TX_MGMT,
			    config->rate_for_tx_mgmt) ==
			    QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Couldn't pass on WNI_CFG_RATE_FOR_TX_MGMT to CCM");
	}

	if (sme_cfg_set_int(mac_handle, WNI_CFG_RATE_FOR_TX_MGMT_2G,
			    config->rate_for_tx_mgmt_2g) ==
			    QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Couldn't pass on WNI_CFG_RATE_FOR_TX_MGMT_2G to CCM");
	}

	if (sme_cfg_set_int(mac_handle, WNI_CFG_RATE_FOR_TX_MGMT_5G,
			    config->rate_for_tx_mgmt_5g) ==
			    QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Couldn't pass on WNI_CFG_RATE_FOR_TX_MGMT_5G to CCM");
	}

	if (sme_cfg_set_int(mac_handle, WNI_CFG_ASSOC_STA_LIMIT,
			    config->maxNumberOfPeers) ==
				QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Couldn't pass on WNI_CFG_ASSOC_STA_LIMIT to CFG");
	}

	return status;
}

/**
 * hdd_update_per_config_to_sme() -initializes the sme config for PER roam
 *
 * @hdd_ctx: the pointer to hdd context
 * @sme_config: sme configuation pointer
 *
 * Return: None
 */
static void hdd_update_per_config_to_sme(struct hdd_context *hdd_ctx,
					 tSmeConfigParams *sme_config)
{
	sme_config->csrConfig.per_roam_config.enable =
			hdd_ctx->config->is_per_roam_enabled;

	/* Assigning Tx and Rx for same value */
	sme_config->csrConfig.per_roam_config.tx_high_rate_thresh =
			hdd_ctx->config->per_roam_high_rate_threshold;
	sme_config->csrConfig.per_roam_config.rx_high_rate_thresh =
			hdd_ctx->config->per_roam_high_rate_threshold;

	/* Assigning Tx and Rx for same value */
	sme_config->csrConfig.per_roam_config.tx_low_rate_thresh =
			hdd_ctx->config->per_roam_low_rate_threshold;
	sme_config->csrConfig.per_roam_config.rx_low_rate_thresh =
			hdd_ctx->config->per_roam_low_rate_threshold;

	/* Assigning Tx and Rx for same value */
	sme_config->csrConfig.per_roam_config.tx_rate_thresh_percnt =
			hdd_ctx->config->per_roam_th_percent;
	sme_config->csrConfig.per_roam_config.rx_rate_thresh_percnt =
			hdd_ctx->config->per_roam_th_percent;

	sme_config->csrConfig.per_roam_config.per_rest_time =
			hdd_ctx->config->per_roam_rest_time;
	sme_config->csrConfig.per_roam_config.tx_per_mon_time =
			hdd_ctx->config->per_roam_mon_time;
	sme_config->csrConfig.per_roam_config.rx_per_mon_time =
			hdd_ctx->config->per_roam_mon_time;

	/* Assigning minimum roamable AP RSSI for candidate selection */
	sme_config->csrConfig.per_roam_config.min_candidate_rssi =
			hdd_ctx->config->min_candidate_rssi;
}

/**
 * hdd_set_policy_mgr_user_cfg() -initializes the policy manager
 * configuration parameters
 *
 * @hdd_ctx: the pointer to hdd context
 *
 * Return: QDF_STATUS_SUCCESS if configuration is correctly applied,
 *		otherwise the appropriate QDF_STATUS would be returned
 */
QDF_STATUS hdd_set_policy_mgr_user_cfg(struct hdd_context *hdd_ctx)
{
	QDF_STATUS status;
	struct policy_mgr_user_cfg *user_cfg;

	user_cfg = qdf_mem_malloc(sizeof(*user_cfg));
	if (NULL == user_cfg) {
		hdd_err("unable to allocate user_cfg");
		return QDF_STATUS_E_NOMEM;
	}

	user_cfg->conc_system_pref = hdd_ctx->config->conc_system_pref;
	user_cfg->enable_mcc_adaptive_scheduler =
		hdd_ctx->config->enableMCCAdaptiveScheduler;
	user_cfg->max_concurrent_active_sessions =
		hdd_ctx->config->gMaxConcurrentActiveSessions;
	user_cfg->enable2x2 = hdd_ctx->config->enable2x2;
	user_cfg->mcc_to_scc_switch_mode =
		hdd_ctx->config->WlanMccToSccSwitchMode;
	user_cfg->sub_20_mhz_enabled = cds_is_sub_20_mhz_enabled();
	user_cfg->is_sta_sap_scc_allowed_on_dfs_chan =
		hdd_ctx->config->sta_sap_scc_on_dfs_chan;
	user_cfg->channel_select_logic_conc =
		hdd_ctx->config->channel_select_logic_conc;
	user_cfg->sta_sap_scc_on_lte_coex_chan =
		hdd_ctx->config->sta_sap_scc_on_lte_coex_chan;
	user_cfg->dbs_selection_policy = hdd_ctx->config->dbs_selection_policy;
	user_cfg->vdev_priority_list = hdd_ctx->config->vdev_priority_list;
	status = policy_mgr_set_user_cfg(hdd_ctx->hdd_psoc, user_cfg);
	qdf_mem_free(user_cfg);

	return status;
}

eCsrRoamWmmUserModeType hdd_to_csr_wmm_mode(enum hdd_wmm_user_mode mode)
{
	switch (mode) {
	case HDD_WMM_USER_MODE_QBSS_ONLY:
		return eCsrRoamWmmQbssOnly;
	case HDD_WMM_USER_MODE_NO_QOS:
		return eCsrRoamWmmNoQos;
	case HDD_WMM_USER_MODE_AUTO:
	default:
		return eCsrRoamWmmAuto;
	}
}

/**
 * hdd_update_score_params() -initializes the sme config for bss score params
 *
 * @config: pointer to config
 * @score_params: bss score params
 *
 * Return: None
 */
static void hdd_update_bss_score_params(struct hdd_config *config,
					struct sir_score_config *score_params)
{
	int total_weight;

	score_params->enable_scoring_for_roam =
			config->enable_scoring_for_roam;
	score_params->weight_cfg.rssi_weightage = config->rssi_weightage;
	score_params->weight_cfg.ht_caps_weightage = config->ht_caps_weightage;
	score_params->weight_cfg.vht_caps_weightage =
					config->vht_caps_weightage;
	score_params->weight_cfg.he_caps_weightage =
					config->he_caps_weightage;
	score_params->weight_cfg.chan_width_weightage =
		config->chan_width_weightage;
	score_params->weight_cfg.chan_band_weightage =
		config->chan_band_weightage;
	score_params->weight_cfg.nss_weightage = config->nss_weightage;
	score_params->weight_cfg.beamforming_cap_weightage =
		config->beamforming_cap_weightage;
	score_params->weight_cfg.pcl_weightage = config->pcl_weightage;
	score_params->weight_cfg.channel_congestion_weightage =
			config->channel_congestion_weightage;
	score_params->weight_cfg.oce_wan_weightage = config->oce_wan_weightage;

	total_weight = score_params->weight_cfg.rssi_weightage +
		       score_params->weight_cfg.ht_caps_weightage +
		       score_params->weight_cfg.vht_caps_weightage +
		       score_params->weight_cfg.he_caps_weightage +
		       score_params->weight_cfg.chan_width_weightage +
		       score_params->weight_cfg.chan_band_weightage +
		       score_params->weight_cfg.nss_weightage +
		       score_params->weight_cfg.beamforming_cap_weightage +
		       score_params->weight_cfg.pcl_weightage +
		       score_params->weight_cfg.channel_congestion_weightage +
		       score_params->weight_cfg.oce_wan_weightage;

	if (total_weight > BEST_CANDIDATE_MAX_WEIGHT) {
		hdd_err("total weight is greater than %d fallback to default values",
			BEST_CANDIDATE_MAX_WEIGHT);

		score_params->weight_cfg.rssi_weightage = RSSI_WEIGHTAGE;
		score_params->weight_cfg.ht_caps_weightage =
			HT_CAPABILITY_WEIGHTAGE;
		score_params->weight_cfg.vht_caps_weightage = VHT_CAP_WEIGHTAGE;
		score_params->weight_cfg.he_caps_weightage = HE_CAP_WEIGHTAGE;
		score_params->weight_cfg.chan_width_weightage =
			CHAN_WIDTH_WEIGHTAGE;
		score_params->weight_cfg.chan_band_weightage =
			CHAN_BAND_WEIGHTAGE;
		score_params->weight_cfg.nss_weightage = NSS_WEIGHTAGE;
		score_params->weight_cfg.beamforming_cap_weightage =
			BEAMFORMING_CAP_WEIGHTAGE;
		score_params->weight_cfg.pcl_weightage = PCL_WEIGHT;
		score_params->weight_cfg.channel_congestion_weightage =
			CHANNEL_CONGESTION_WEIGHTAGE;
		score_params->weight_cfg.oce_wan_weightage = OCE_WAN_WEIGHTAGE;
	}

	score_params->bandwidth_weight_per_index =
		hdd_limit_max_per_index_score(
			config->bandwidth_weight_per_index);
	score_params->nss_weight_per_index =
		hdd_limit_max_per_index_score(config->nss_weight_per_index);
	score_params->band_weight_per_index =
		hdd_limit_max_per_index_score(config->band_weight_per_index);

	score_params->rssi_score.best_rssi_threshold =
				config->best_rssi_threshold;
	score_params->rssi_score.good_rssi_threshold =
				config->good_rssi_threshold;
	score_params->rssi_score.bad_rssi_threshold =
				config->bad_rssi_threshold;
	score_params->rssi_score.good_rssi_pcnt = config->good_rssi_pcnt;
	score_params->rssi_score.bad_rssi_pcnt = config->bad_rssi_pcnt;
	score_params->rssi_score.good_rssi_bucket_size =
		config->good_rssi_bucket_size;
	score_params->rssi_score.bad_rssi_bucket_size =
		config->bad_rssi_bucket_size;
	score_params->rssi_score.rssi_pref_5g_rssi_thresh =
		config->rssi_pref_5g_rssi_thresh;

	score_params->esp_qbss_scoring.num_slot = config->num_esp_qbss_slots;
	score_params->esp_qbss_scoring.score_pcnt3_to_0 =
		hdd_limit_max_per_index_score(
			config->esp_qbss_score_slots3_to_0);
	score_params->esp_qbss_scoring.score_pcnt7_to_4 =
		hdd_limit_max_per_index_score(
			config->esp_qbss_score_slots7_to_4);
	score_params->esp_qbss_scoring.score_pcnt11_to_8 =
		hdd_limit_max_per_index_score(
			config->esp_qbss_score_slots11_to_8);
	score_params->esp_qbss_scoring.score_pcnt15_to_12 =
		hdd_limit_max_per_index_score(
			config->esp_qbss_score_slots15_to_12);

	score_params->oce_wan_scoring.num_slot = config->num_oce_wan_slots;
	score_params->oce_wan_scoring.score_pcnt3_to_0 =
		hdd_limit_max_per_index_score(
			config->oce_wan_score_slots3_to_0);
	score_params->oce_wan_scoring.score_pcnt7_to_4 =
		hdd_limit_max_per_index_score(
			config->oce_wan_score_slots7_to_4);
	score_params->oce_wan_scoring.score_pcnt11_to_8 =
		hdd_limit_max_per_index_score(
			config->oce_wan_score_slots11_to_8);
	score_params->oce_wan_scoring.score_pcnt15_to_12 =
		hdd_limit_max_per_index_score(
			config->oce_wan_score_slots15_to_12);
}

/**
 * hdd_update_11k_offload_params() - initializes the 11k offload related params
 *
 * @config: pointer to hdd_config structure
 * @csr_config: pointer to the csr config structure
 *
 * Return: None
 */
static
void hdd_update_11k_offload_params(struct hdd_config *config,
				tCsrConfigParam *csr_config)
{
	csr_config->offload_11k_enable_bitmask =
		config->offload_11k_enable_bitmask;
	csr_config->neighbor_report_offload.params_bitmask =
		config->neighbor_report_offload_params_bitmask;
	csr_config->neighbor_report_offload.time_offset =
		config->neighbor_report_offload_time_offset;
	csr_config->neighbor_report_offload.low_rssi_offset =
		config->neighbor_report_offload_low_rssi_offset;
	csr_config->neighbor_report_offload.bmiss_count_trigger =
		config->neighbor_report_offload_bmiss_count_trigger;
	csr_config->neighbor_report_offload.per_threshold_offset =
		config->neighbor_report_offload_per_threshold_offset;
	csr_config->neighbor_report_offload.neighbor_report_cache_timeout =
		config->neighbor_report_offload_cache_timeout;
	csr_config->neighbor_report_offload.max_neighbor_report_req_cap =
		config->neighbor_report_offload_max_req_cap;
}

/**
 * hdd_set_sme_config() -initializes the sme configuration parameters
 *
 * @hdd_ctx: the pointer to hdd context
 *
 * Return: QDF_STATUS_SUCCESS if configuration is correctly applied,
 *		otherwise the appropriate QDF_STATUS would be returned
 */
QDF_STATUS hdd_set_sme_config(struct hdd_context *hdd_ctx)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSmeConfigParams *smeConfig;
	uint8_t rrm_capab_len, val;
	bool ignore_peer_ht_mode;
	mac_handle_t mac_handle = hdd_ctx->mac_handle;

	struct hdd_config *pConfig = hdd_ctx->config;

	smeConfig = qdf_mem_malloc(sizeof(*smeConfig));
	if (NULL == smeConfig) {
		hdd_err("unable to allocate smeConfig");
		return QDF_STATUS_E_NOMEM;
	}

	hdd_debug("%s bWmmIsEnabled=%d 802_11e_enabled=%d dot11Mode=%d",
		  __func__, pConfig->WmmMode, pConfig->b80211eIsEnabled,
		  pConfig->dot11Mode);

	/* Config params obtained from the registry
	 * To Do: set regulatory information here
	 */

	smeConfig->csrConfig.RTSThreshold = pConfig->RTSThreshold;
	smeConfig->csrConfig.FragmentationThreshold =
		pConfig->FragmentationThreshold;
	smeConfig->csrConfig.shortSlotTime = pConfig->ShortSlotTimeEnabled;
	smeConfig->csrConfig.Is11dSupportEnabled = pConfig->Is11dSupportEnabled;
	smeConfig->csrConfig.HeartbeatThresh24 = pConfig->HeartbeatThresh24;

	smeConfig->csrConfig.phyMode =
		hdd_cfg_xlate_to_csr_phy_mode(pConfig->dot11Mode);

	if (pConfig->dot11Mode == eHDD_DOT11_MODE_abg ||
	    pConfig->dot11Mode == eHDD_DOT11_MODE_11b ||
	    pConfig->dot11Mode == eHDD_DOT11_MODE_11g ||
	    pConfig->dot11Mode == eHDD_DOT11_MODE_11b_ONLY ||
	    pConfig->dot11Mode == eHDD_DOT11_MODE_11g_ONLY) {
		smeConfig->csrConfig.channelBondingMode24GHz = 0;
		smeConfig->csrConfig.channelBondingMode5GHz = 0;
	} else {
		smeConfig->csrConfig.channelBondingMode24GHz =
			pConfig->nChannelBondingMode24GHz;
		smeConfig->csrConfig.channelBondingMode5GHz =
			pConfig->nChannelBondingMode5GHz;
	}
	smeConfig->csrConfig.nScanResultAgeCount = pConfig->ScanResultAgeCount;
	smeConfig->csrConfig.AdHocChannel24 = pConfig->OperatingChannel;
	smeConfig->csrConfig.fSupplicantCountryCodeHasPriority =
		pConfig->fSupplicantCountryCodeHasPriority;
	smeConfig->csrConfig.bCatRssiOffset = pConfig->nRssiCatGap;
	smeConfig->csrConfig.vccRssiThreshold = pConfig->nVccRssiTrigger;
	smeConfig->csrConfig.vccUlMacLossThreshold =
		pConfig->nVccUlMacLossThreshold;
	smeConfig->csrConfig.nInitialDwellTime = pConfig->nInitialDwellTime;
	smeConfig->csrConfig.initial_scan_no_dfs_chnl =
					pConfig->initial_scan_no_dfs_chnl;
	smeConfig->csrConfig.nActiveMaxChnTime = pConfig->nActiveMaxChnTime;
	smeConfig->csrConfig.nPassiveMaxChnTime = pConfig->nPassiveMaxChnTime;
	smeConfig->csrConfig.nActiveMaxChnTimeConc =
		pConfig->nActiveMaxChnTimeConc;
	smeConfig->csrConfig.nActiveMinChnTimeConc =
		pConfig->nActiveMinChnTimeConc;
	smeConfig->csrConfig.nPassiveMaxChnTimeConc =
		pConfig->nPassiveMaxChnTimeConc;
	smeConfig->csrConfig.nPassiveMinChnTimeConc =
		pConfig->nPassiveMinChnTimeConc;
	smeConfig->csrConfig.nRestTimeConc = pConfig->nRestTimeConc;
	smeConfig->csrConfig.min_rest_time_conc = pConfig->min_rest_time_conc;
	smeConfig->csrConfig.idle_time_conc     = pConfig->idle_time_conc;

	smeConfig->csrConfig.Is11eSupportEnabled = pConfig->b80211eIsEnabled;
	smeConfig->csrConfig.WMMSupportMode =
		hdd_to_csr_wmm_mode(pConfig->WmmMode);

	smeConfig->rrmConfig.rrm_enabled = pConfig->fRrmEnable;
	smeConfig->rrmConfig.max_randn_interval = pConfig->nRrmRandnIntvl;
	hdd_hex_string_to_u8_array(pConfig->rm_capability,
			smeConfig->rrmConfig.rm_capability, &rrm_capab_len,
			DOT11F_IE_RRMENABLEDCAP_MAX_LEN);
	/* Remaining config params not obtained from registry
	 * On RF EVB beacon using channel 1.
	 */
	smeConfig->csrConfig.nVhtChannelWidth = pConfig->vhtChannelWidth;
	smeConfig->csrConfig.enableTxBF = pConfig->enableTxBF;
	smeConfig->csrConfig.enable_subfee_vendor_vhtie =
				pConfig->enable_subfee_vendor_vhtie;

	smeConfig->csrConfig.enable_txbf_sap_mode =
		pConfig->enable_txbf_sap_mode;
	smeConfig->csrConfig.enable2x2 = pConfig->enable2x2;
	smeConfig->csrConfig.enableVhtFor24GHz = pConfig->enableVhtFor24GHzBand;
	smeConfig->csrConfig.vendor_vht_sap =
		pConfig->enable_sap_vendor_vht;
	smeConfig->csrConfig.enableMuBformee = pConfig->enableMuBformee;
	smeConfig->csrConfig.enableVhtpAid = pConfig->enableVhtpAid;
	smeConfig->csrConfig.enableVhtGid = pConfig->enableVhtGid;
	smeConfig->csrConfig.enableAmpduPs = pConfig->enableAmpduPs;
	smeConfig->csrConfig.enableHtSmps = pConfig->enableHtSmps;
	smeConfig->csrConfig.htSmps = pConfig->htSmps;
	/* This param cannot be configured from INI */
	smeConfig->csrConfig.send_smps_action = true;
	smeConfig->csrConfig.AdHocChannel5G = pConfig->AdHocChannel5G;
	smeConfig->csrConfig.AdHocChannel24 = pConfig->AdHocChannel24G;
	smeConfig->csrConfig.ProprietaryRatesEnabled = 0;
	smeConfig->csrConfig.HeartbeatThresh50 = 40;
	smeConfig->csrConfig.bandCapability = pConfig->nBandCapability;
	if (pConfig->nBandCapability == BAND_2G) {
		smeConfig->csrConfig.Is11hSupportEnabled = 0;
	} else {
		smeConfig->csrConfig.Is11hSupportEnabled =
			pConfig->Is11hSupportEnabled;
	}
	smeConfig->csrConfig.eBand = pConfig->nBandCapability;
	smeConfig->csrConfig.nTxPowerCap = pConfig->nTxPowerCap;
	smeConfig->csrConfig.allow_tpc_from_ap = pConfig->allow_tpc_from_ap;
	smeConfig->csrConfig.fEnableBypass11d = pConfig->enableBypass11d;
	smeConfig->csrConfig.fEnableDFSChnlScan = pConfig->enableDFSChnlScan;
	smeConfig->csrConfig.nRoamPrefer5GHz = pConfig->nRoamPrefer5GHz;
	smeConfig->csrConfig.nRoamIntraBand = pConfig->nRoamIntraBand;
	smeConfig->csrConfig.nProbes = pConfig->nProbes;

	smeConfig->csrConfig.nRoamScanHomeAwayTime =
		pConfig->nRoamScanHomeAwayTime;
	smeConfig->csrConfig.fFirstScanOnly2GChnl =
		pConfig->enableFirstScan2GOnly;

	smeConfig->csrConfig.Csr11dinfo.Channels.numChannels = 0;

	hdd_set_power_save_offload_config(hdd_ctx);

	smeConfig->csrConfig.isFastRoamIniFeatureEnabled =
		pConfig->isFastRoamIniFeatureEnabled;
	smeConfig->csrConfig.csr_mawc_config.mawc_enabled =
		pConfig->MAWCEnabled;
	smeConfig->csrConfig.csr_mawc_config.mawc_roam_enabled =
		pConfig->mawc_roam_enabled;
	smeConfig->csrConfig.csr_mawc_config.mawc_roam_traffic_threshold =
		pConfig->mawc_roam_traffic_threshold;
	smeConfig->csrConfig.csr_mawc_config.mawc_roam_ap_rssi_threshold =
		pConfig->mawc_roam_ap_rssi_threshold;
	smeConfig->csrConfig.csr_mawc_config.mawc_roam_rssi_high_adjust =
		pConfig->mawc_roam_rssi_high_adjust;
	smeConfig->csrConfig.csr_mawc_config.mawc_roam_rssi_low_adjust =
		pConfig->mawc_roam_rssi_low_adjust;
#ifdef FEATURE_WLAN_ESE
	smeConfig->csrConfig.isEseIniFeatureEnabled =
		pConfig->isEseIniFeatureEnabled;
	if (pConfig->isEseIniFeatureEnabled)
		pConfig->isFastTransitionEnabled = true;
#endif
	smeConfig->csrConfig.isFastTransitionEnabled =
		pConfig->isFastTransitionEnabled;
	smeConfig->csrConfig.RoamRssiDiff = pConfig->RoamRssiDiff;
	smeConfig->csrConfig.rssi_abs_thresh = pConfig->rssi_abs_thresh;
	smeConfig->csrConfig.isWESModeEnabled = pConfig->isWESModeEnabled;
	smeConfig->csrConfig.isRoamOffloadScanEnabled =
		pConfig->isRoamOffloadScanEnabled;
	smeConfig->csrConfig.bFastRoamInConIniFeatureEnabled =
		pConfig->bFastRoamInConIniFeatureEnabled;

	if (0 == smeConfig->csrConfig.isRoamOffloadScanEnabled) {
		/* Disable roaming in concurrency if roam scan
		 * offload is disabled
		 */
		smeConfig->csrConfig.bFastRoamInConIniFeatureEnabled = 0;
	}
	smeConfig->csrConfig.neighborRoamConfig.nNeighborLookupRssiThreshold =
		pConfig->nNeighborLookupRssiThreshold;
	smeConfig->csrConfig.neighborRoamConfig.rssi_thresh_offset_5g =
		pConfig->rssi_thresh_offset_5g;
	smeConfig->csrConfig.neighborRoamConfig.delay_before_vdev_stop =
		pConfig->delay_before_vdev_stop;
	smeConfig->csrConfig.neighborRoamConfig.nOpportunisticThresholdDiff =
		pConfig->nOpportunisticThresholdDiff;
	smeConfig->csrConfig.neighborRoamConfig.nRoamRescanRssiDiff =
		pConfig->nRoamRescanRssiDiff;
	smeConfig->csrConfig.neighborRoamConfig.nNeighborScanMaxChanTime =
		pConfig->nNeighborScanMaxChanTime;
	smeConfig->csrConfig.neighborRoamConfig.nNeighborScanMinChanTime =
		pConfig->nNeighborScanMinChanTime;
	smeConfig->csrConfig.neighborRoamConfig.nNeighborScanTimerPeriod =
		pConfig->nNeighborScanPeriod;
	smeConfig->csrConfig.neighborRoamConfig.
		neighbor_scan_min_timer_period =
		pConfig->neighbor_scan_min_period;
	smeConfig->csrConfig.neighborRoamConfig.nMaxNeighborRetries =
		pConfig->nMaxNeighborReqTries;
	smeConfig->csrConfig.neighborRoamConfig.nNeighborResultsRefreshPeriod =
		pConfig->nNeighborResultsRefreshPeriod;
	smeConfig->csrConfig.neighborRoamConfig.nEmptyScanRefreshPeriod =
		pConfig->nEmptyScanRefreshPeriod;
	hdd_string_to_u8_array(pConfig->neighborScanChanList,
			       smeConfig->csrConfig.neighborRoamConfig.
			       neighborScanChanList.channelList,
			       &smeConfig->csrConfig.neighborRoamConfig.
			       neighborScanChanList.numChannels,
			       WNI_CFG_VALID_CHANNEL_LIST_LEN);
	smeConfig->csrConfig.neighborRoamConfig.nRoamBmissFirstBcnt =
		pConfig->nRoamBmissFirstBcnt;
	smeConfig->csrConfig.neighborRoamConfig.nRoamBmissFinalBcnt =
		pConfig->nRoamBmissFinalBcnt;
	smeConfig->csrConfig.neighborRoamConfig.nRoamBeaconRssiWeight =
		pConfig->nRoamBeaconRssiWeight;
	smeConfig->csrConfig.neighborRoamConfig.nhi_rssi_scan_max_count =
		pConfig->nhi_rssi_scan_max_count;
	smeConfig->csrConfig.neighborRoamConfig.nhi_rssi_scan_rssi_delta =
		pConfig->nhi_rssi_scan_rssi_delta;
	smeConfig->csrConfig.neighborRoamConfig.nhi_rssi_scan_delay =
		pConfig->nhi_rssi_scan_delay;
	smeConfig->csrConfig.neighborRoamConfig.nhi_rssi_scan_rssi_ub =
		pConfig->nhi_rssi_scan_rssi_ub;
	smeConfig->csrConfig.addTSWhenACMIsOff = pConfig->AddTSWhenACMIsOff;
	smeConfig->csrConfig.allowDFSChannelRoam = pConfig->allowDFSChannelRoam;

	/* Enable/Disable MCC */
	smeConfig->csrConfig.fEnableMCCMode = pConfig->enableMCC;
	smeConfig->csrConfig.mcc_rts_cts_prot_enable =
					pConfig->mcc_rts_cts_prot_enable;
	smeConfig->csrConfig.mcc_bcast_prob_resp_enable =
					pConfig->mcc_bcast_prob_resp_enable;
	smeConfig->csrConfig.fAllowMCCGODiffBI = pConfig->allowMCCGODiffBI;

	/* Scan Results Aging Time out value */
	smeConfig->csrConfig.scanCfgAgingTime = pConfig->scanAgingTimeout;

	smeConfig->csrConfig.enable_tx_ldpc = pConfig->enable_tx_ldpc;
	smeConfig->csrConfig.enable_rx_ldpc = pConfig->enable_rx_ldpc;
	smeConfig->csrConfig.enable_vht20_mcs9 = pConfig->enable_vht20_mcs9;
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
	smeConfig->csrConfig.cc_switch_mode = pConfig->WlanMccToSccSwitchMode;
#endif

	smeConfig->csrConfig.max_amsdu_num = pConfig->max_amsdu_num;
	smeConfig->csrConfig.nSelect5GHzMargin = pConfig->nSelect5GHzMargin;

	smeConfig->csrConfig.isCoalesingInIBSSAllowed =
		hdd_ctx->config->isCoalesingInIBSSAllowed;
	smeConfig->csrConfig.ignore_peer_erp_info =
						pConfig->ignore_peer_erp_info;
	/* update SSR config */
	sme_update_enable_ssr(mac_handle, hdd_ctx->config->enableSSR);

	/* Update maximum interfaces information */
	smeConfig->csrConfig.max_intf_count = hdd_ctx->max_intf_count;

	smeConfig->csrConfig.fEnableDebugLog = hdd_ctx->config->gEnableDebugLog;

	smeConfig->csrConfig.enable5gEBT = hdd_ctx->config->enable5gEBT;

	smeConfig->csrConfig.enableSelfRecovery =
			hdd_ctx->config->enableSelfRecovery;
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	smeConfig->csrConfig.isRoamOffloadEnabled =
		hdd_ctx->config->isRoamOffloadEnabled;
#endif
	smeConfig->csrConfig.conc_custom_rule1 =
		hdd_ctx->config->conc_custom_rule1;
	smeConfig->csrConfig.conc_custom_rule2 =
		hdd_ctx->config->conc_custom_rule2;
	smeConfig->csrConfig.is_sta_connection_in_5gz_enabled =
		hdd_ctx->config->is_sta_connection_in_5gz_enabled;

	smeConfig->csrConfig.f_sta_miracast_mcc_rest_time_val =
		hdd_ctx->config->sta_miracast_mcc_rest_time_val;

#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
	smeConfig->csrConfig.sap_channel_avoidance =
		hdd_ctx->config->sap_channel_avoidance;
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */
	smeConfig->csrConfig.acs_with_more_param =
		hdd_ctx->config->acs_with_more_param;

	smeConfig->csrConfig.f_prefer_non_dfs_on_radar =
		hdd_ctx->config->prefer_non_dfs_on_radar;

	smeConfig->csrConfig.is_ps_enabled = hdd_ctx->config->is_ps_enabled;
	smeConfig->csrConfig.auto_bmps_timer_val =
		hdd_ctx->config->auto_bmps_timer_val;
	hdd_set_fine_time_meas_cap(hdd_ctx);

	cds_set_multicast_logging(hdd_ctx->config->multicast_host_fw_msgs);

	smeConfig->csrConfig.send_deauth_before_con =
		pConfig->send_deauth_before_con;

	smeConfig->csrConfig.max_scan_count =
			hdd_ctx->config->max_scan_count;

	/* Update 802.11p config */
	smeConfig->csrConfig.enable_dot11p =
		(hdd_ctx->config->dot11p_mode != WLAN_HDD_11P_DISABLED);

	smeConfig->csrConfig.early_stop_scan_enable =
		hdd_ctx->config->early_stop_scan_enable;
	smeConfig->csrConfig.early_stop_scan_min_threshold =
		hdd_ctx->config->early_stop_scan_min_threshold;
	smeConfig->csrConfig.early_stop_scan_max_threshold =
		hdd_ctx->config->early_stop_scan_max_threshold;
	smeConfig->csrConfig.first_scan_bucket_threshold =
		hdd_ctx->config->first_scan_bucket_threshold;

	smeConfig->csrConfig.roam_dense_rssi_thresh_offset =
			hdd_ctx->config->roam_dense_rssi_thresh_offset;
	smeConfig->csrConfig.roam_dense_min_aps =
			hdd_ctx->config->roam_dense_min_aps;
	smeConfig->csrConfig.roam_dense_traffic_thresh =
			hdd_ctx->config->roam_dense_traffic_thresh;
	smeConfig->csrConfig.roam_bg_scan_bad_rssi_thresh =
		hdd_ctx->config->roam_bg_scan_bad_rssi_thresh;
	smeConfig->csrConfig.roam_bg_scan_client_bitmap =
		hdd_ctx->config->roam_bg_scan_client_bitmap;
	smeConfig->csrConfig.roam_bad_rssi_thresh_offset_2g =
		hdd_ctx->config->roam_bad_rssi_thresh_offset_2g;
	smeConfig->csrConfig.ho_delay_for_rx =
		hdd_ctx->config->ho_delay_for_rx;
	smeConfig->csrConfig.min_delay_btw_roam_scans =
		hdd_ctx->config->min_delay_btw_roam_scans;
	smeConfig->csrConfig.roam_trigger_reason_bitmask =
		hdd_ctx->config->roam_trigger_reason_bitmask;
	smeConfig->csrConfig.obss_width_interval =
			hdd_ctx->config->obss_width_trigger_interval;
	smeConfig->csrConfig.obss_active_dwelltime =
			hdd_ctx->config->obss_active_dwelltime;
	smeConfig->csrConfig.obss_passive_dwelltime =
			hdd_ctx->config->obss_passive_dwelltime;

	status = ucfg_mlme_get_ignore_peer_ht_mode(hdd_ctx->hdd_psoc,
						   &ignore_peer_ht_mode);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("Get ignore_peer_ht_mode failed");
		goto error;
	}
	smeConfig->csrConfig.ignore_peer_ht_opmode = ignore_peer_ht_mode;

	smeConfig->csrConfig.enable_fatal_event =
			pConfig->enable_fatal_event;
	smeConfig->csrConfig.scan_adaptive_dwell_mode =
			hdd_ctx->config->scan_adaptive_dwell_mode;
	smeConfig->csrConfig.scan_adaptive_dwell_mode_nc =
			hdd_ctx->config->scan_adaptive_dwell_mode_nc;
	smeConfig->csrConfig.roamscan_adaptive_dwell_mode =
			hdd_ctx->config->roamscan_adaptive_dwell_mode;
	smeConfig->csrConfig.enable_ftopen =
			hdd_ctx->config->enable_ftopen;
	smeConfig->csrConfig.roam_force_rssi_trigger =
			hdd_ctx->config->roam_force_rssi_trigger;

	hdd_update_per_config_to_sme(hdd_ctx, smeConfig);

	smeConfig->csrConfig.enable_edca_params =
			pConfig->enable_edca_params;

	smeConfig->csrConfig.edca_vo_cwmin =
			pConfig->edca_vo_cwmin;
	smeConfig->csrConfig.edca_vi_cwmin =
			pConfig->edca_vi_cwmin;
	smeConfig->csrConfig.edca_bk_cwmin =
			pConfig->edca_bk_cwmin;
	smeConfig->csrConfig.edca_be_cwmin =
			pConfig->edca_be_cwmin;

	smeConfig->csrConfig.edca_vo_cwmax =
			pConfig->edca_vo_cwmax;
	smeConfig->csrConfig.edca_vi_cwmax =
			pConfig->edca_vi_cwmax;
	smeConfig->csrConfig.edca_bk_cwmax =
			pConfig->edca_bk_cwmax;
	smeConfig->csrConfig.edca_be_cwmax =
			pConfig->edca_be_cwmax;

	smeConfig->csrConfig.edca_vo_aifs =
			pConfig->edca_vo_aifs;
	smeConfig->csrConfig.edca_vi_aifs =
			pConfig->edca_vi_aifs;
	smeConfig->csrConfig.edca_bk_aifs =
			pConfig->edca_bk_aifs;
	smeConfig->csrConfig.edca_be_aifs =
			pConfig->edca_be_aifs;
	smeConfig->csrConfig.sta_roam_policy_params.dfs_mode =
		CSR_STA_ROAM_POLICY_DFS_ENABLED;
	smeConfig->csrConfig.sta_roam_policy_params.skip_unsafe_channels = 0;

	smeConfig->snr_monitor_enabled = hdd_ctx->config->fEnableSNRMonitoring;

	smeConfig->csrConfig.tx_aggregation_size =
			hdd_ctx->config->tx_aggregation_size;
	smeConfig->csrConfig.tx_aggregation_size_be =
			hdd_ctx->config->tx_aggregation_size_be;
	smeConfig->csrConfig.tx_aggregation_size_bk =
			hdd_ctx->config->tx_aggregation_size_bk;
	smeConfig->csrConfig.tx_aggregation_size_vi =
			hdd_ctx->config->tx_aggregation_size_vi;
	smeConfig->csrConfig.tx_aggregation_size_vo =
			hdd_ctx->config->tx_aggregation_size_vo;
	smeConfig->csrConfig.rx_aggregation_size =
			hdd_ctx->config->rx_aggregation_size;
	smeConfig->csrConfig.tx_aggr_sw_retry_threshold_be =
			hdd_ctx->config->tx_aggr_sw_retry_threshold_be;
	smeConfig->csrConfig.tx_aggr_sw_retry_threshold_bk =
			hdd_ctx->config->tx_aggr_sw_retry_threshold_bk;
	smeConfig->csrConfig.tx_aggr_sw_retry_threshold_vi =
			hdd_ctx->config->tx_aggr_sw_retry_threshold_vi;
	smeConfig->csrConfig.tx_aggr_sw_retry_threshold_vo =
			hdd_ctx->config->tx_aggr_sw_retry_threshold_vo;
	smeConfig->csrConfig.enable_bcast_probe_rsp =
			hdd_ctx->config->enable_bcast_probe_rsp;
	smeConfig->csrConfig.is_fils_enabled =
			hdd_ctx->config->is_fils_enabled;
	smeConfig->csrConfig.qcn_ie_support =
			hdd_ctx->config->qcn_ie_support;
	smeConfig->csrConfig.fils_max_chan_guard_time =
			hdd_ctx->config->fils_max_chan_guard_time;

	hdd_he_set_sme_config(smeConfig, pConfig);

	smeConfig->csrConfig.wlm_latency_enable =
			hdd_ctx->config->wlm_latency_enable;
	smeConfig->csrConfig.wlm_latency_level =
			hdd_ctx->config->wlm_latency_level;
	smeConfig->csrConfig.wlm_latency_flags[0] =
			hdd_ctx->config->wlm_latency_flags_normal;
	smeConfig->csrConfig.wlm_latency_flags[1] =
			hdd_ctx->config->wlm_latency_flags_moderate;
	smeConfig->csrConfig.wlm_latency_flags[2] =
			hdd_ctx->config->wlm_latency_flags_low;
	smeConfig->csrConfig.wlm_latency_flags[3] =
			hdd_ctx->config->wlm_latency_flags_ultralow;

	smeConfig->csrConfig.pkt_err_disconn_th =
			hdd_ctx->config->pkt_err_disconn_th;
	smeConfig->csrConfig.disallow_duration =
			hdd_ctx->config->disallow_duration;
	smeConfig->csrConfig.rssi_channel_penalization =
			hdd_ctx->config->rssi_channel_penalization;
	smeConfig->csrConfig.num_disallowed_aps =
			hdd_ctx->config->num_disallowed_aps;

	smeConfig->csrConfig.is_force_1x1 =
			hdd_ctx->config->is_force_1x1;

	val = (pConfig->oce_probe_req_rate_enabled *
		WMI_VDEV_OCE_PROBE_REQUEST_RATE_FEATURE_BITMAP) +
		(pConfig->oce_probe_resp_rate_enabled *
		WMI_VDEV_OCE_PROBE_RESPONSE_RATE_FEATURE_BITMAP) +
		(pConfig->oce_beacon_rate_enabled *
		WMI_VDEV_OCE_BEACON_RATE_FEATURE_BITMAP) +
		(pConfig->probe_req_deferral_enabled *
		WMI_VDEV_OCE_PROBE_REQUEST_DEFERRAL_FEATURE_BITMAP) +
		(pConfig->fils_discovery_sap_enabled *
		WMI_VDEV_OCE_FILS_DISCOVERY_FRAME_FEATURE_BITMAP) +
		(pConfig->esp_for_roam_enabled *
		WMI_VDEV_OCE_ESP_FEATURE_BITMAP) +
		(pConfig->rssi_assoc_reject_enabled *
		WMI_VDEV_OCE_REASSOC_REJECT_FEATURE_BITMAP);
	smeConfig->csrConfig.oce_feature_bitmap = val;
	smeConfig->csrConfig.mbo_thresholds.mbo_candidate_rssi_thres =
		hdd_ctx->config->mbo_candidate_rssi_thres;
	smeConfig->csrConfig.mbo_thresholds.mbo_current_rssi_thres =
		hdd_ctx->config->mbo_current_rssi_thres;
	smeConfig->csrConfig.mbo_thresholds.mbo_current_rssi_mcc_thres =
		hdd_ctx->config->mbo_current_rssi_mcc_thres;
	smeConfig->csrConfig.mbo_thresholds.mbo_candidate_rssi_btc_thres =
		hdd_ctx->config->mbo_candidate_rssi_btc_thres;
	smeConfig->csrConfig.btm_offload_config =
			hdd_ctx->config->btm_offload_config;
	smeConfig->csrConfig.btm_solicited_timeout =
			hdd_ctx->config->btm_solicited_timeout;
	smeConfig->csrConfig.btm_max_attempt_cnt =
			hdd_ctx->config->btm_max_attempt_cnt;
	smeConfig->csrConfig.btm_sticky_time =
			hdd_ctx->config->btm_sticky_time;
	hdd_update_bss_score_params(hdd_ctx->config,
			&smeConfig->csrConfig.bss_score_params);

	hdd_update_11k_offload_params(hdd_ctx->config,
					&smeConfig->csrConfig);

	status = sme_update_config(mac_handle, smeConfig);
	if (!QDF_IS_STATUS_SUCCESS(status))
		hdd_err("sme_update_config() failure: %d", status);

error:
	qdf_mem_free(smeConfig);
	return status;
}

/**
 * hdd_execute_global_config_command() - execute the global config command
 * @hdd_ctx: the pointer to hdd context
 * @command: the command to run
 *
 * Return: the QDF_STATUS return from hdd_execute_config_command
 */
QDF_STATUS hdd_execute_global_config_command(struct hdd_context *hdd_ctx,
					     char *command)
{
	return hdd_execute_config_command(g_registry_table,
					  ARRAY_SIZE(g_registry_table),
					  (uint8_t *) hdd_ctx->config,
					  hdd_ctx, command);
}

static void print_info_handler(const char *buf)
{
	hdd_nofl_info("%s", buf);
}

static void print_debug_handler(const char *buf)
{
	hdd_nofl_debug("%s", buf);
}

/**
 * hdd_cfg_get_global_config() - get the configuration table
 * @hdd_ctx: pointer to hdd context
 * @pBuf: buffer to store the configuration
 * @buflen: size of the buffer
 *
 * Return: none
 */
void hdd_cfg_get_global_config(struct hdd_context *hdd_ctx, char *buf,
			       int buflen)
{
	hdd_cfg_get_config(g_registry_table,
			   ARRAY_SIZE(g_registry_table),
			   (uint8_t *)hdd_ctx->config, hdd_ctx,
			   &print_info_handler);

	snprintf(buf, buflen,
		 "WLAN configuration written to system log");
}

/**
 * hdd_cfg_print_global_config() - print the configuration table
 * @hdd_ctx: pointer to hdd context
 *
 * Return: none
 */
void hdd_cfg_print_global_config(struct hdd_context *hdd_ctx)
{
	hdd_cfg_get_config(g_registry_table,
			   ARRAY_SIZE(g_registry_table),
			   (uint8_t *)hdd_ctx->config, hdd_ctx,
			   &print_debug_handler);
}

/**
 * hdd_get_pmkid_modes() - returns PMKID mode bits
 * @hdd_ctx: the pointer to hdd context
 *
 * Return: value of pmkid_modes
 */
void hdd_get_pmkid_modes(struct hdd_context *hdd_ctx,
			 struct pmkid_mode_bits *pmkid_modes)
{
	pmkid_modes->fw_okc = (hdd_ctx->config->pmkid_modes &
			       CFG_PMKID_MODES_OKC) ? 1 : 0;
	pmkid_modes->fw_pmksa_cache = (hdd_ctx->config->pmkid_modes &
				       CFG_PMKID_MODES_PMKSA_CACHING) ? 1 : 0;
}

bool hdd_validate_prb_req_ie_bitmap(struct hdd_context *hdd_ctx)
{
	if (!(hdd_ctx->config->probe_req_ie_bitmap_0 ||
	    hdd_ctx->config->probe_req_ie_bitmap_1 ||
	    hdd_ctx->config->probe_req_ie_bitmap_2 ||
	    hdd_ctx->config->probe_req_ie_bitmap_3 ||
	    hdd_ctx->config->probe_req_ie_bitmap_4 ||
	    hdd_ctx->config->probe_req_ie_bitmap_5 ||
	    hdd_ctx->config->probe_req_ie_bitmap_6 ||
	    hdd_ctx->config->probe_req_ie_bitmap_7))
		return false;

	/*
	 * check whether vendor oui IE is set and OUIs are present, each OUI
	 * is entered in the form of string of 8 characters from ini, therefore,
	 * for atleast one OUI, minimum length is 8 and hence this string length
	 * is checked for minimum of 8
	 */
	if ((hdd_ctx->config->probe_req_ie_bitmap_6 &
	    VENDOR_SPECIFIC_IE_BITMAP) &&
	    (strlen(hdd_ctx->config->probe_req_ouis) < 8))
		return false;

	/* check whether vendor oui IE is not set but OUIs are present */
	if (!(hdd_ctx->config->probe_req_ie_bitmap_6 &
	    VENDOR_SPECIFIC_IE_BITMAP) &&
	    (strlen(hdd_ctx->config->probe_req_ouis) > 0))
		return false;

	return true;
}

int hdd_parse_probe_req_ouis(struct hdd_context *hdd_ctx)
{
	uint32_t *voui = hdd_ctx->config->probe_req_voui;
	char *str;
	uint8_t *token;
	uint32_t oui_indx = 0;
	int ret;
	uint32_t hex_value;

	str = (char *)(hdd_ctx->config->probe_req_ouis);
	str[MAX_PRB_REQ_VENDOR_OUI_INI_LEN - 1] = '\0';
	hdd_ctx->config->no_of_probe_req_ouis = 0;

	if (!strlen(str)) {
		hdd_info("NO OUIS to parse");
		return 0;
	}

	token = strsep(&str, " ");
	while (token) {
		if (strlen(token) != 8)
			goto next_token;

		ret = kstrtouint(token, 16, &hex_value);
		if (ret)
			goto next_token;

		voui[oui_indx++] = cpu_to_be32(hex_value);
		if (oui_indx >= MAX_PROBE_REQ_OUIS)
			break;

next_token:
		token = strsep(&str, " ");
	}

	if (!oui_indx)
		return -EINVAL;

	hdd_ctx->config->no_of_probe_req_ouis = oui_indx;

	return 0;
}

/**
 * hdd_update_nss() - Update the number of spatial streams supported.
 * Ensure that nss is either 1 or 2 before calling this.
 *
 * @adapter: the pointer to adapter
 * @nss: the number of spatial streams to be updated
 *
 * This function is used to modify the number of spatial streams
 * supported when not in connected state.
 *
 * Return: QDF_STATUS_SUCCESS if nss is correctly updated,
 *              otherwise QDF_STATUS_E_FAILURE would be returned
 */
QDF_STATUS hdd_update_nss(struct hdd_adapter *adapter, uint8_t nss)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct hdd_config *hdd_config = hdd_ctx->config;
	uint32_t temp = 0;
	uint32_t rx_supp_data_rate, tx_supp_data_rate;
	bool status = true;
	QDF_STATUS qdf_status;
	struct mlme_ht_capabilities_info ht_cap_info;
	uint8_t mcs_set[SIZE_OF_SUPPORTED_MCS_SET] = {0};
	uint8_t mcs_set_temp[SIZE_OF_SUPPORTED_MCS_SET];
	uint32_t val, val32;
	uint8_t enable2x2;
	mac_handle_t mac_handle;

	if ((nss == 2) && (hdd_ctx->num_rf_chains != 2)) {
		hdd_err("No support for 2 spatial streams");
		return QDF_STATUS_E_INVAL;
	}

	enable2x2 = (nss == 1) ? 0 : 1;

	if (hdd_config->enable2x2 == enable2x2) {
		hdd_debug("NSS same as requested");
		return QDF_STATUS_SUCCESS;
	}

	mac_handle = hdd_ctx->mac_handle;
	if (sme_is_any_session_in_connected_state(mac_handle)) {
		hdd_err("Connected sessions present, Do not change NSS");
		return QDF_STATUS_E_INVAL;
	}

	hdd_config->enable2x2 = enable2x2;

	if (!hdd_config->enable2x2) {
		/* 1x1 */
		rx_supp_data_rate = VHT_RX_HIGHEST_SUPPORTED_DATA_RATE_1_1;
		tx_supp_data_rate = VHT_TX_HIGHEST_SUPPORTED_DATA_RATE_1_1;
	} else {
		/* 2x2 */
		rx_supp_data_rate = VHT_RX_HIGHEST_SUPPORTED_DATA_RATE_2_2;
		tx_supp_data_rate = VHT_TX_HIGHEST_SUPPORTED_DATA_RATE_2_2;
	}

	/* Update Rx Highest Long GI data Rate */
	if (sme_cfg_set_int(mac_handle,
			    WNI_CFG_VHT_RX_HIGHEST_SUPPORTED_DATA_RATE,
			    rx_supp_data_rate) == QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Could not pass on WNI_CFG_VHT_RX_HIGHEST_SUPPORTED_DATA_RATE to CFG");
	}

	/* Update Tx Highest Long GI data Rate */
	if (sme_cfg_set_int(mac_handle,
			    WNI_CFG_VHT_TX_HIGHEST_SUPPORTED_DATA_RATE,
			    tx_supp_data_rate) == QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Could not pass on WNI_CFG_VHT_TX_HIGHEST_SUPPORTED_DATA_RATE to CFG");
	}

	qdf_status = ucfg_mlme_get_ht_cap_info(hdd_ctx->hdd_psoc, &ht_cap_info);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		hdd_err("Failed to get HT Cap info");
		status = false;
	}

	if (!(hdd_ctx->ht_tx_stbc_supported && hdd_config->enable2x2)) {
		ht_cap_info.tx_stbc = 0;
	} else {
		sme_cfg_get_int(mac_handle, WNI_CFG_VHT_TXSTBC, &val32);
		hdd_debug("STBC %d", val32);
		ht_cap_info.tx_stbc = val32;
	}

	qdf_status = ucfg_mlme_set_ht_cap_info(hdd_ctx->hdd_psoc, ht_cap_info);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		status = false;
		hdd_err("Could not set the HT_CAP_INFO");
	}

	sme_cfg_get_int(mac_handle, WNI_CFG_VHT_BASIC_MCS_SET, &temp);
	temp = (temp & 0xFFFC) | hdd_config->vhtRxMCS;
	if (hdd_config->enable2x2)
		temp = (temp & 0xFFF3) | (hdd_config->vhtRxMCS2x2 << 2);
	else
		temp |= 0x000C;

	if (sme_cfg_set_int(mac_handle, WNI_CFG_VHT_BASIC_MCS_SET,
			    temp) == QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Could not pass on WNI_CFG_VHT_BASIC_MCS_SET to CFG");
	}

	sme_cfg_get_int(mac_handle, WNI_CFG_VHT_RX_MCS_MAP, &temp);
	temp = (temp & 0xFFFC) | hdd_config->vhtRxMCS;
	if (hdd_config->enable2x2)
		temp = (temp & 0xFFF3) | (hdd_config->vhtRxMCS2x2 << 2);
	else
		temp |= 0x000C;

	if (sme_cfg_set_int(mac_handle, WNI_CFG_VHT_RX_MCS_MAP,
			    temp) == QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Could not pass on WNI_CFG_VHT_RX_MCS_MAP to CFG");
	}

	sme_cfg_get_int(mac_handle, WNI_CFG_VHT_TX_MCS_MAP, &temp);
	temp = (temp & 0xFFFC) | hdd_config->vhtTxMCS;
	if (hdd_config->enable2x2)
		temp = (temp & 0xFFF3) | (hdd_config->vhtTxMCS2x2 << 2);
	else
		temp |= 0x000C;

	if (sme_cfg_set_int(mac_handle, WNI_CFG_VHT_TX_MCS_MAP,
			    temp) == QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Could not pass on WNI_CFG_VHT_TX_MCS_MAP to CFG");
	}

#define WLAN_HDD_RX_MCS_ALL_NSTREAM_RATES 0xff
	val = SIZE_OF_SUPPORTED_MCS_SET;
	sme_cfg_get_str(mac_handle, WNI_CFG_SUPPORTED_MCS_SET,
			mcs_set_temp, &val);

	mcs_set[0] = mcs_set_temp[0];
	if (hdd_config->enable2x2)
		for (val = 0; val < nss; val++)
			mcs_set[val] = WLAN_HDD_RX_MCS_ALL_NSTREAM_RATES;

	if (sme_cfg_set_str(mac_handle, WNI_CFG_SUPPORTED_MCS_SET,
			    mcs_set,
			    SIZE_OF_SUPPORTED_MCS_SET) ==
				QDF_STATUS_E_FAILURE) {
		status = false;
		hdd_err("Could not pass on MCS SET to CFG");
	}
	sme_update_he_cap_nss(mac_handle, adapter->session_id, nss);
#undef WLAN_HDD_RX_MCS_ALL_NSTREAM_RATES

	if (QDF_STATUS_SUCCESS != sme_update_nss(mac_handle, nss))
		status = false;

	hdd_set_policy_mgr_user_cfg(hdd_ctx);
	return (status == false) ? QDF_STATUS_E_FAILURE : QDF_STATUS_SUCCESS;
}
