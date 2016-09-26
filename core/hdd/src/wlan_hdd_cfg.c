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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
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
#include <cds_concurrency.h>

static void
cb_notify_set_roam_prefer5_g_hz(hdd_context_t *pHddCtx, unsigned long notifyId)
{
	sme_update_roam_prefer5_g_hz(pHddCtx->hHal,
				     pHddCtx->config->nRoamPrefer5GHz);
}

static void
cb_notify_set_roam_rssi_diff(hdd_context_t *pHddCtx, unsigned long notifyId)
{
	sme_update_roam_rssi_diff(pHddCtx->hHal,
				  0, pHddCtx->config->RoamRssiDiff);
}

static void
cb_notify_set_fast_transition_enabled(hdd_context_t *pHddCtx,
				      unsigned long notifyId)
{
	sme_update_fast_transition_enabled(pHddCtx->hHal,
					   pHddCtx->config->
					   isFastTransitionEnabled);
}

static void
cb_notify_set_roam_intra_band(hdd_context_t *pHddCtx, unsigned long notifyId)
{
	sme_set_roam_intra_band(pHddCtx->hHal, pHddCtx->config->nRoamIntraBand);
}

static void cb_notify_set_wes_mode(hdd_context_t *pHddCtx, unsigned long notifyId)
{
	/* At the point this routine is called, the value in the hdd config
	   table has already been updated */
	sme_update_wes_mode(pHddCtx->hHal, pHddCtx->config->isWESModeEnabled, 0);
}

static void
cb_notify_set_roam_scan_n_probes(hdd_context_t *pHddCtx, unsigned long notifyId)
{
	sme_update_roam_scan_n_probes(pHddCtx->hHal, 0, pHddCtx->config->nProbes);
}

static void
cb_notify_set_roam_scan_home_away_time(hdd_context_t *pHddCtx, unsigned long notifyId)
{
	sme_update_roam_scan_home_away_time(pHddCtx->hHal, 0,
					    pHddCtx->config->nRoamScanHomeAwayTime,
					    true);
}

static void
notify_is_fast_roam_ini_feature_enabled(hdd_context_t *pHddCtx,
					unsigned long notifyId)
{
	/* At the point this routine is called, the value in the hdd config
	   table has already been updated */
	sme_update_is_fast_roam_ini_feature_enabled(pHddCtx->hHal, 0,
						    pHddCtx->config->
						    isFastRoamIniFeatureEnabled);
}

static void
notify_is_mawc_ini_feature_enabled(hdd_context_t *pHddCtx, unsigned long notifyId)
{
	/* At the point this routine is called, the value in the hdd config
	   table has already been updated */
	sme_update_is_mawc_ini_feature_enabled(pHddCtx->hHal,
					       pHddCtx->config->MAWCEnabled);
}

#ifdef FEATURE_WLAN_ESE
static void
cb_notify_set_ese_feature_enabled(hdd_context_t *pHddCtx, unsigned long notifyId)
{
	/* At the point this routine is called, the value in the hdd config
	   table has already been updated */
	sme_update_is_ese_feature_enabled(pHddCtx->hHal, 0,
					  pHddCtx->config->isEseIniFeatureEnabled);
}
#endif

static void
cb_notify_set_fw_rssi_monitoring(hdd_context_t *pHddCtx, unsigned long notifyId)
{
	/* At the point this routine is called, the value in the hdd config
	   table has already been updated */
	sme_update_config_fw_rssi_monitoring(pHddCtx->hHal,
					     pHddCtx->config->
					     fEnableFwRssiMonitoring);
}

static void cb_notify_set_opportunistic_scan_threshold_diff(hdd_context_t *pHddCtx,
							    unsigned long notifyId)
{
	/* At the point this routine is called, the value in the hdd config
	   table has already been updated */
	sme_set_roam_opportunistic_scan_threshold_diff(pHddCtx->hHal, 0,
						       pHddCtx->config->
						       nOpportunisticThresholdDiff);
}

static void cb_notify_set_roam_rescan_rssi_diff(hdd_context_t *pHddCtx,
						unsigned long notifyId)
{
	/* At the point this routine is called, the value in the hdd config
	   table has already been updated */
	sme_set_roam_rescan_rssi_diff(pHddCtx->hHal,
				      0, pHddCtx->config->nRoamRescanRssiDiff);
}

static void
cb_notify_set_neighbor_lookup_rssi_threshold(hdd_context_t *pHddCtx,
					     unsigned long notifyId)
{
	/* At the point this routine is called, the value in the hdd config
	   table has already been updated */
	sme_set_neighbor_lookup_rssi_threshold(pHddCtx->hHal, 0,
					       pHddCtx->config->
					       nNeighborLookupRssiThreshold);
}

static void
cb_notify_set_delay_before_vdev_stop(hdd_context_t *hdd_ctx,
				     unsigned long notify_id)
{
	/*
	 * At the point this routine is called, the value in the cfg_ini
	 * table has already been updated
	 */
	sme_set_delay_before_vdev_stop(hdd_ctx->hHal, 0,
				hdd_ctx->config->delay_before_vdev_stop);
}

static void
cb_notify_set_neighbor_scan_period(hdd_context_t *pHddCtx, unsigned long notifyId)
{
	/* At the point this routine is called, the value in the hdd config
	   table has already been updated */
	sme_set_neighbor_scan_period(pHddCtx->hHal, 0,
				     pHddCtx->config->nNeighborScanPeriod);
}

static void
cb_notify_set_neighbor_results_refresh_period(hdd_context_t *pHddCtx,
					      unsigned long notifyId)
{
	/* At the point this routine is called, the value in the hdd config
	   table has already been updated */
	sme_set_neighbor_scan_refresh_period(pHddCtx->hHal, 0,
					     pHddCtx->config->
					     nNeighborResultsRefreshPeriod);
}

static void
cb_notify_set_empty_scan_refresh_period(hdd_context_t *pHddCtx,
					unsigned long notifyId)
{
	/* At the point this routine is called, the value in the hdd config
	   table has already been updated */
	sme_update_empty_scan_refresh_period(pHddCtx->hHal, 0,
					     pHddCtx->config->
					     nEmptyScanRefreshPeriod);
}

static void
cb_notify_set_neighbor_scan_min_chan_time(hdd_context_t *pHddCtx,
					  unsigned long notifyId)
{
	/* At the point this routine is called, the value in the hdd config
	   table has already been updated */
	sme_set_neighbor_scan_min_chan_time(pHddCtx->hHal,
					    pHddCtx->config->
					    nNeighborScanMinChanTime, 0);
}

static void
cb_notify_set_neighbor_scan_max_chan_time(hdd_context_t *pHddCtx,
					  unsigned long notifyId)
{
	sme_set_neighbor_scan_max_chan_time(pHddCtx->hHal, 0,
					    pHddCtx->config->
					    nNeighborScanMaxChanTime);
}

static void cb_notify_set_roam_bmiss_first_bcnt(hdd_context_t *pHddCtx,
						unsigned long notifyId)
{
	/* At the point this routine is called, the value in the hdd config
	   table has already been updated */
	sme_set_roam_bmiss_first_bcnt(pHddCtx->hHal,
				      0, pHddCtx->config->nRoamBmissFirstBcnt);
}

static void cb_notify_set_roam_bmiss_final_bcnt(hdd_context_t *pHddCtx,
						unsigned long notifyId)
{
	sme_set_roam_bmiss_final_bcnt(pHddCtx->hHal, 0,
				      pHddCtx->config->nRoamBmissFinalBcnt);
}

static void cb_notify_set_roam_beacon_rssi_weight(hdd_context_t *pHddCtx,
						  unsigned long notifyId)
{
	sme_set_roam_beacon_rssi_weight(pHddCtx->hHal, 0,
					pHddCtx->config->nRoamBeaconRssiWeight);
}

static void
cb_notify_set_dfs_scan_mode(hdd_context_t *pHddCtx, unsigned long notifyId)
{
	/* At the point this routine is called, the value in the hdd config
	   table has already been updated */
	sme_update_dfs_scan_mode(pHddCtx->hHal, 0,
				 pHddCtx->config->allowDFSChannelRoam);
}

static void cb_notify_set_enable_ssr(hdd_context_t *pHddCtx,
				     unsigned long notifyId)
{
	sme_update_enable_ssr(pHddCtx->hHal, pHddCtx->config->enableSSR);
}

static void cb_notify_set_g_sap_preferred_chan_location(hdd_context_t *pHddCtx,
							unsigned long notifyId)
{
	wlansap_set_dfs_preferred_channel_location(pHddCtx->hHal,
						   pHddCtx->config->
						   gSapPreferredChanLocation);
}
static void ch_notify_set_g_disable_dfs_japan_w53(hdd_context_t *pHddCtx,
						  unsigned long notifyId)
{
	wlansap_set_dfs_restrict_japan_w53(pHddCtx->hHal,
					   pHddCtx->config->
					   gDisableDfsJapanW53);
}
static void
cb_notify_update_roam_scan_offload_enabled(hdd_context_t *pHddCtx,
					   unsigned long notifyId)
{
	sme_update_roam_scan_offload_enabled(pHddCtx->hHal,
					     pHddCtx->config->
					     isRoamOffloadScanEnabled);
	if (0 == pHddCtx->config->isRoamOffloadScanEnabled) {
		pHddCtx->config->bFastRoamInConIniFeatureEnabled = 0;
		sme_update_enable_fast_roam_in_concurrency(pHddCtx->hHal,
							   pHddCtx->config->
							   bFastRoamInConIniFeatureEnabled);
	}
}

static void
cb_notify_set_enable_fast_roam_in_concurrency(hdd_context_t *pHddCtx,
					      unsigned long notifyId)
{
	sme_update_enable_fast_roam_in_concurrency(pHddCtx->hHal,
						   pHddCtx->config->
						   bFastRoamInConIniFeatureEnabled);
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
cb_notify_set_roam_scan_hi_rssi_scan_params(hdd_context_t *hdd_ctx,
				    unsigned long notify_id)
{
	int32_t val;

	if (wlan_hdd_validate_context(hdd_ctx)) {
		return;
	}

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

	sme_update_roam_scan_hi_rssi_scan_params(hdd_ctx->hHal, 0,
		notify_id, val);
}


REG_TABLE_ENTRY g_registry_table[] = {
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

	REG_VARIABLE(CFG_BMPS_MODERATE_LI_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nBmpsModListenInterval,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_BMPS_MODERATE_LI_DEFAULT,
		     CFG_BMPS_MODERATE_LI_MIN,
		     CFG_BMPS_MODERATE_LI_MAX),

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
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK ,
		     CFG_HT_MPDU_DENSITY_DEFAULT,
		     CFG_HT_MPDU_DENSITY_MIN,
		     CFG_HT_MPDU_DENSITY_MAX),

	REG_VARIABLE(CFG_FIXED_RATE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, TxRate,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_FIXED_RATE_DEFAULT,
		     CFG_FIXED_RATE_MIN,
		     CFG_FIXED_RATE_MAX),

	REG_VARIABLE(CFG_SHORT_GI_20MHZ_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, ShortGI20MhzEnable,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_SHORT_GI_20MHZ_DEFAULT,
		     CFG_SHORT_GI_20MHZ_MIN,
		     CFG_SHORT_GI_20MHZ_MAX),

	REG_VARIABLE(CFG_SCAN_RESULT_AGE_COUNT_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, ScanResultAgeCount,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_SCAN_RESULT_AGE_COUNT_DEFAULT,
		     CFG_SCAN_RESULT_AGE_COUNT_MIN,
		     CFG_SCAN_RESULT_AGE_COUNT_MAX),

	REG_VARIABLE(CFG_SCAN_RESULT_AGE_TIME_NCNPS_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nScanAgeTimeNCNPS,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_SCAN_RESULT_AGE_TIME_NCNPS_DEFAULT,
		     CFG_SCAN_RESULT_AGE_TIME_NCNPS_MIN,
		     CFG_SCAN_RESULT_AGE_TIME_NCNPS_MAX),

	REG_VARIABLE(CFG_SCAN_RESULT_AGE_TIME_NCPS_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nScanAgeTimeNCPS,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_SCAN_RESULT_AGE_TIME_NCPS_DEFAULT,
		     CFG_SCAN_RESULT_AGE_TIME_NCPS_MIN,
		     CFG_SCAN_RESULT_AGE_TIME_NCPS_MAX),

	REG_VARIABLE(CFG_SCAN_RESULT_AGE_TIME_CNPS_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nScanAgeTimeCNPS,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_SCAN_RESULT_AGE_TIME_CNPS_DEFAULT,
		     CFG_SCAN_RESULT_AGE_TIME_CNPS_MIN,
		     CFG_SCAN_RESULT_AGE_TIME_CNPS_MAX),

	REG_VARIABLE(CFG_SCAN_RESULT_AGE_TIME_CPS_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nScanAgeTimeCPS,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_SCAN_RESULT_AGE_TIME_CPS_DEFAULT,
		     CFG_SCAN_RESULT_AGE_TIME_CPS_MIN,
		     CFG_SCAN_RESULT_AGE_TIME_CPS_MAX),

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
	REG_VARIABLE(CFG_FORCE_SAP_ACS, WLAN_PARAM_Integer,
		struct hdd_config, force_sap_acs,
		VAR_FLAGS_DYNAMIC_CFG |
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_FORCE_SAP_ACS_DEFAULT,
		CFG_FORCE_SAP_ACS_MIN,
		CFG_FORCE_SAP_ACS_MAX),

	REG_VARIABLE(CFG_FORCE_SAP_ACS_START_CH, WLAN_PARAM_Integer,
		struct hdd_config, force_sap_acs_st_ch,
		VAR_FLAGS_DYNAMIC_CFG |
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_FORCE_SAP_ACS_START_CH_DEFAULT,
		CFG_FORCE_SAP_ACS_START_CH_MIN,
		CFG_FORCE_SAP_ACS_START_CH_MAX),

	REG_VARIABLE(CFG_FORCE_SAP_ACS_END_CH, WLAN_PARAM_Integer,
		struct hdd_config, force_sap_acs_end_ch,
		VAR_FLAGS_DYNAMIC_CFG |
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_FORCE_SAP_ACS_END_CH_DEFAULT,
		CFG_FORCE_SAP_ACS_END_CH_MIN,
		CFG_FORCE_SAP_ACS_END_CH_MAX),

	REG_VARIABLE(CFG_AP_KEEP_ALIVE_PERIOD_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, apKeepAlivePeriod,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_AP_KEEP_ALIVE_PERIOD_DEFAULT,
		     CFG_AP_KEEP_ALIVE_PERIOD_MIN,
		     CFG_AP_KEEP_ALIVE_PERIOD_MAX),

	REG_VARIABLE(CFG_GO_KEEP_ALIVE_PERIOD_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, goKeepAlivePeriod,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_GO_KEEP_ALIVE_PERIOD_DEFAULT,
		     CFG_GO_KEEP_ALIVE_PERIOD_MIN,
		     CFG_GO_KEEP_ALIVE_PERIOD_MAX),

	REG_VARIABLE(CFG_AP_LINK_MONITOR_PERIOD_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, apLinkMonitorPeriod,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_AP_LINK_MONITOR_PERIOD_DEFAULT,
		     CFG_AP_LINK_MONITOR_PERIOD_MIN,
		     CFG_AP_LINK_MONITOR_PERIOD_MAX),

	REG_VARIABLE(CFG_GO_LINK_MONITOR_PERIOD_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, goLinkMonitorPeriod,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_GO_LINK_MONITOR_PERIOD_DEFAULT,
		     CFG_GO_LINK_MONITOR_PERIOD_MIN,
		     CFG_GO_LINK_MONITOR_PERIOD_MAX),

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

	REG_VARIABLE(CFG_ROAMING_TIME_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nRoamingTime,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ROAMING_TIME_DEFAULT,
		     CFG_ROAMING_TIME_MIN,
		     CFG_ROAMING_TIME_MAX),

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

	REG_VARIABLE(CFG_PASSIVE_MIN_CHANNEL_TIME_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nPassiveMinChnTime,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_PASSIVE_MIN_CHANNEL_TIME_DEFAULT,
		     CFG_PASSIVE_MIN_CHANNEL_TIME_MIN,
		     CFG_PASSIVE_MIN_CHANNEL_TIME_MAX),

	REG_VARIABLE(CFG_ACTIVE_MAX_CHANNEL_TIME_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nActiveMaxChnTime,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ACTIVE_MAX_CHANNEL_TIME_DEFAULT,
		     CFG_ACTIVE_MAX_CHANNEL_TIME_MIN,
		     CFG_ACTIVE_MAX_CHANNEL_TIME_MAX),

	REG_VARIABLE(CFG_ACTIVE_MIN_CHANNEL_TIME_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nActiveMinChnTime,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ACTIVE_MIN_CHANNEL_TIME_DEFAULT,
		     CFG_ACTIVE_MIN_CHANNEL_TIME_MIN,
		     CFG_ACTIVE_MIN_CHANNEL_TIME_MAX),

	REG_VARIABLE(CFG_RETRY_LIMIT_ZERO_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, retryLimitZero,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_RETRY_LIMIT_ZERO_DEFAULT,
		     CFG_RETRY_LIMIT_ZERO_MIN,
		     CFG_RETRY_LIMIT_ZERO_MAX),

	REG_VARIABLE(CFG_RETRY_LIMIT_ONE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, retryLimitOne,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_RETRY_LIMIT_ONE_DEFAULT,
		     CFG_RETRY_LIMIT_ONE_MIN,
		     CFG_RETRY_LIMIT_ONE_MAX),

	REG_VARIABLE(CFG_RETRY_LIMIT_TWO_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, retryLimitTwo,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_RETRY_LIMIT_TWO_DEFAULT,
		     CFG_RETRY_LIMIT_TWO_MIN,
		     CFG_RETRY_LIMIT_TWO_MAX),

#ifdef WLAN_AP_STA_CONCURRENCY
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

	REG_VARIABLE(CFG_IDLE_TIME_NAME , WLAN_PARAM_Integer,
		      struct hdd_config, idle_time_conc,
		      VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		      CFG_IDLE_TIME_DEFAULT,
		      CFG_IDLE_TIME_MIN,
		      CFG_IDLE_TIME_MAX) ,

	REG_VARIABLE(CFG_NUM_STA_CHAN_COMBINED_CONC_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nNumStaChanCombinedConc,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_NUM_STA_CHAN_COMBINED_CONC_DEFAULT,
		     CFG_NUM_STA_CHAN_COMBINED_CONC_MIN,
		     CFG_NUM_STA_CHAN_COMBINED_CONC_MAX),

	REG_VARIABLE(CFG_NUM_P2P_CHAN_COMBINED_CONC_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nNumP2PChanCombinedConc,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_NUM_P2P_CHAN_COMBINED_CONC_DEFAULT,
		     CFG_NUM_P2P_CHAN_COMBINED_CONC_MIN,
		     CFG_NUM_P2P_CHAN_COMBINED_CONC_MAX),
#endif

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

	REG_VARIABLE(CFG_LOW_GAIN_OVERRIDE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, fIsLowGainOverride,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_LOW_GAIN_OVERRIDE_DEFAULT,
		     CFG_LOW_GAIN_OVERRIDE_MIN,
		     CFG_LOW_GAIN_OVERRIDE_MAX),

	REG_VARIABLE(CFG_RSSI_FILTER_PERIOD_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nRssiFilterPeriod,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_RSSI_FILTER_PERIOD_DEFAULT,
		     CFG_RSSI_FILTER_PERIOD_MIN,
		     CFG_RSSI_FILTER_PERIOD_MAX),

	REG_VARIABLE(CFG_IGNORE_DTIM_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, fIgnoreDtim,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_IGNORE_DTIM_DEFAULT,
		     CFG_IGNORE_DTIM_MIN,
		     CFG_IGNORE_DTIM_MAX),

	REG_VARIABLE(CFG_MAX_LI_MODULATED_DTIM_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, fMaxLIModulatedDTIM,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_MAX_LI_MODULATED_DTIM_DEFAULT,
		     CFG_MAX_LI_MODULATED_DTIM_MIN,
		     CFG_MAX_LI_MODULATED_DTIM_MAX),

	REG_VARIABLE(CFG_FW_HEART_BEAT_MONITORING_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, fEnableFwHeartBeatMonitoring,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_FW_HEART_BEAT_MONITORING_DEFAULT,
		     CFG_FW_HEART_BEAT_MONITORING_MIN,
		     CFG_FW_HEART_BEAT_MONITORING_MAX),

	REG_VARIABLE(CFG_FW_BEACON_FILTERING_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, fEnableFwBeaconFiltering,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_FW_BEACON_FILTERING_DEFAULT,
		     CFG_FW_BEACON_FILTERING_MIN,
		     CFG_FW_BEACON_FILTERING_MAX),

	REG_DYNAMIC_VARIABLE(CFG_FW_RSSI_MONITORING_NAME, WLAN_PARAM_Integer,
			     struct hdd_config, fEnableFwRssiMonitoring,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_FW_RSSI_MONITORING_DEFAULT,
			     CFG_FW_RSSI_MONITORING_MIN,
			     CFG_FW_RSSI_MONITORING_MAX,
			     cb_notify_set_fw_rssi_monitoring, 0),

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

	/* Variable to specify the delta/difference between the RSSI of current AP
	 * and roamable AP while roaming */
	REG_DYNAMIC_VARIABLE(CFG_ROAM_RSSI_DIFF_NAME, WLAN_PARAM_Integer,
			     struct hdd_config, RoamRssiDiff,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_ROAM_RSSI_DIFF_DEFAULT,
			     CFG_ROAM_RSSI_DIFF_MIN,
			     CFG_ROAM_RSSI_DIFF_MAX,
			     cb_notify_set_roam_rssi_diff, 0),

	REG_DYNAMIC_VARIABLE(CFG_ENABLE_WES_MODE_NAME, WLAN_PARAM_Integer,
			     struct hdd_config, isWESModeEnabled,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_ENABLE_WES_MODE_NAME_DEFAULT,
			     CFG_ENABLE_WES_MODE_NAME_MIN,
			     CFG_ENABLE_WES_MODE_NAME_MAX,
			     cb_notify_set_wes_mode, 0),
	REG_VARIABLE(CFG_OKC_FEATURE_ENABLED_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, isOkcIniFeatureEnabled,
		     VAR_FLAGS_OPTIONAL |
		     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_OKC_FEATURE_ENABLED_DEFAULT,
		     CFG_OKC_FEATURE_ENABLED_MIN,
		     CFG_OKC_FEATURE_ENABLED_MAX),
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

	REG_VARIABLE_STRING(CFG_WOWL_PATTERN_NAME, WLAN_PARAM_String,
			    struct hdd_config, wowlPattern,
			    VAR_FLAGS_OPTIONAL,
			    (void *)CFG_WOWL_PATTERN_DEFAULT),

	REG_VARIABLE(CFG_QOS_IMPLICIT_SETUP_ENABLED_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, bImplicitQosEnabled,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_IMPLICIT_SETUP_ENABLED_DEFAULT,
		     CFG_QOS_IMPLICIT_SETUP_ENABLED_MIN,
		     CFG_QOS_IMPLICIT_SETUP_ENABLED_MAX),

	REG_VARIABLE(CFG_AP_AUTO_SHUT_OFF, WLAN_PARAM_Integer,
		     struct hdd_config, nAPAutoShutOff,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_AP_AUTO_SHUT_OFF_DEFAULT,
		     CFG_AP_AUTO_SHUT_OFF_MIN,
		     CFG_AP_AUTO_SHUT_OFF_MAX),

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

	REG_VARIABLE(CFG_FT_RESOURCE_REQ_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, fFTResourceReqSupported,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_FT_RESOURCE_REQ_DEFAULT,
		     CFG_FT_RESOURCE_REQ_MIN,
		     CFG_FT_RESOURCE_REQ_MAX),

	REG_DYNAMIC_VARIABLE(CFG_NEIGHBOR_SCAN_TIMER_PERIOD_NAME,
			     WLAN_PARAM_Integer,
			     struct hdd_config, nNeighborScanPeriod,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_NEIGHBOR_SCAN_TIMER_PERIOD_DEFAULT,
			     CFG_NEIGHBOR_SCAN_TIMER_PERIOD_MIN,
			     CFG_NEIGHBOR_SCAN_TIMER_PERIOD_MAX,
			     cb_notify_set_neighbor_scan_period, 0),

	REG_DYNAMIC_VARIABLE(CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_NAME,
			     WLAN_PARAM_Integer,
			     struct hdd_config, nNeighborLookupRssiThreshold,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_DEFAULT,
			     CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_MIN,
			     CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_MAX,
			     cb_notify_set_neighbor_lookup_rssi_threshold, 0),

	REG_DYNAMIC_VARIABLE(CFG_OPPORTUNISTIC_SCAN_THRESHOLD_DIFF_NAME,
			     WLAN_PARAM_Integer,
			     struct hdd_config, nOpportunisticThresholdDiff,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_OPPORTUNISTIC_SCAN_THRESHOLD_DIFF_DEFAULT,
			     CFG_OPPORTUNISTIC_SCAN_THRESHOLD_DIFF_MIN,
			     CFG_OPPORTUNISTIC_SCAN_THRESHOLD_DIFF_MAX,
			     cb_notify_set_opportunistic_scan_threshold_diff, 0),

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

	REG_VARIABLE(CFG_MCAST_BCAST_FILTER_SETTING_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, mcastBcastFilterSetting,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_MCAST_BCAST_FILTER_SETTING_DEFAULT,
		     CFG_MCAST_BCAST_FILTER_SETTING_MIN,
		     CFG_MCAST_BCAST_FILTER_SETTING_MAX),

	REG_VARIABLE(CFG_ENABLE_HOST_ARPOFFLOAD_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, fhostArpOffload,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_HOST_ARPOFFLOAD_DEFAULT,
		     CFG_ENABLE_HOST_ARPOFFLOAD_MIN,
		     CFG_ENABLE_HOST_ARPOFFLOAD_MAX),

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

	REG_VARIABLE(CFG_DYNAMIC_PSPOLL_VALUE_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, dynamicPsPollValue,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_DYNAMIC_PSPOLL_VALUE_DEFAULT,
		     CFG_DYNAMIC_PSPOLL_VALUE_MIN,
		     CFG_DYNAMIC_PSPOLL_VALUE_MAX),

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

	REG_VARIABLE(CFG_QOS_ADDTS_WHEN_ACM_IS_OFF_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, AddTSWhenACMIsOff,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_QOS_ADDTS_WHEN_ACM_IS_OFF_DEFAULT,
		     CFG_QOS_ADDTS_WHEN_ACM_IS_OFF_MIN,
		     CFG_QOS_ADDTS_WHEN_ACM_IS_OFF_MAX),

	REG_VARIABLE(CFG_VALIDATE_SCAN_LIST_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, fValidateScanList,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_VALIDATE_SCAN_LIST_DEFAULT,
		     CFG_VALIDATE_SCAN_LIST_MIN,
		     CFG_VALIDATE_SCAN_LIST_MAX),

	REG_VARIABLE(CFG_NULLDATA_AP_RESP_TIMEOUT_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nNullDataApRespTimeout,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_NULLDATA_AP_RESP_TIMEOUT_DEFAULT,
		     CFG_NULLDATA_AP_RESP_TIMEOUT_MIN,
		     CFG_NULLDATA_AP_RESP_TIMEOUT_MAX),

	REG_VARIABLE(CFG_AP_DATA_AVAIL_POLL_PERIOD_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, apDataAvailPollPeriodInMs,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_AP_DATA_AVAIL_POLL_PERIOD_DEFAULT,
		     CFG_AP_DATA_AVAIL_POLL_PERIOD_MIN,
		     CFG_AP_DATA_AVAIL_POLL_PERIOD_MAX),

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
		     struct hdd_config, cfd_trace_enable_txrx,
		     VAR_FLAGS_OPTIONAL,
		     CFG_QDF_TRACE_ENABLE_DEFAULT,
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

	REG_VARIABLE(CFG_TELE_BCN_TRANS_LI_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nTeleBcnTransListenInterval,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TELE_BCN_TRANS_LI_DEFAULT,
		     CFG_TELE_BCN_TRANS_LI_MIN,
		     CFG_TELE_BCN_TRANS_LI_MAX),

	REG_VARIABLE(CFG_TELE_BCN_TRANS_LI_NUM_IDLE_BCNS_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, nTeleBcnTransLiNumIdleBeacons,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TELE_BCN_TRANS_LI_NUM_IDLE_BCNS_DEFAULT,
		     CFG_TELE_BCN_TRANS_LI_NUM_IDLE_BCNS_MIN,
		     CFG_TELE_BCN_TRANS_LI_NUM_IDLE_BCNS_MAX),

	REG_VARIABLE(CFG_TELE_BCN_MAX_LI_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nTeleBcnMaxListenInterval,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TELE_BCN_MAX_LI_DEFAULT,
		     CFG_TELE_BCN_MAX_LI_MIN,
		     CFG_TELE_BCN_MAX_LI_MAX),

	REG_VARIABLE(CFG_TELE_BCN_MAX_LI_NUM_IDLE_BCNS_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, nTeleBcnMaxLiNumIdleBeacons,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TELE_BCN_MAX_LI_NUM_IDLE_BCNS_DEFAULT,
		     CFG_TELE_BCN_MAX_LI_NUM_IDLE_BCNS_MIN,
		     CFG_TELE_BCN_MAX_LI_NUM_IDLE_BCNS_MAX),

	REG_VARIABLE(CFG_AP_DATA_AVAIL_POLL_PERIOD_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, apDataAvailPollPeriodInMs,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_AP_DATA_AVAIL_POLL_PERIOD_DEFAULT,
		     CFG_AP_DATA_AVAIL_POLL_PERIOD_MIN,
		     CFG_AP_DATA_AVAIL_POLL_PERIOD_MAX),

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

	REG_VARIABLE(CFG_P2P_DEVICE_ADDRESS_ADMINISTRATED_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, isP2pDeviceAddrAdministrated,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_P2P_DEVICE_ADDRESS_ADMINISTRATED_DEFAULT,
		     CFG_P2P_DEVICE_ADDRESS_ADMINISTRATED_MIN,
		     CFG_P2P_DEVICE_ADDRESS_ADMINISTRATED_MAX),

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

	REG_VARIABLE(CFG_VHT_ENABLE_1x1_TX_CHAINMASK, WLAN_PARAM_Integer,
		     struct hdd_config, txchainmask1x1,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_VHT_ENABLE_1x1_TX_CHAINMASK_DEFAULT,
		     CFG_VHT_ENABLE_1x1_TX_CHAINMASK_MIN,
		     CFG_VHT_ENABLE_1x1_TX_CHAINMASK_MAX),

	REG_VARIABLE(CFG_VHT_ENABLE_1x1_RX_CHAINMASK, WLAN_PARAM_Integer,
		     struct hdd_config, rxchainmask1x1,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_VHT_ENABLE_1x1_RX_CHAINMASK_DEFAULT,
		     CFG_VHT_ENABLE_1x1_RX_CHAINMASK_MIN,
		     CFG_VHT_ENABLE_1x1_RX_CHAINMASK_MAX),

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
	REG_VARIABLE(CFG_ENABLE_FIRST_SCAN_2G_ONLY_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, enableFirstScan2GOnly,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_FIRST_SCAN_2G_ONLY_DEFAULT,
		     CFG_ENABLE_FIRST_SCAN_2G_ONLY_MIN,
		     CFG_ENABLE_FIRST_SCAN_2G_ONLY_MAX),

	REG_VARIABLE(CFG_ENABLE_SKIP_DFS_IN_P2P_SEARCH_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, skipDfsChnlInP2pSearch,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_SKIP_DFS_IN_P2P_SEARCH_DEFAULT,
		     CFG_ENABLE_SKIP_DFS_IN_P2P_SEARCH_MIN,
		     CFG_ENABLE_SKIP_DFS_IN_P2P_SEARCH_MAX),

	REG_VARIABLE(CFG_IGNORE_DYNAMIC_DTIM_IN_P2P_MODE_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, ignoreDynamicDtimInP2pMode,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_IGNORE_DYNAMIC_DTIM_IN_P2P_MODE_DEFAULT,
		     CFG_IGNORE_DYNAMIC_DTIM_IN_P2P_MODE_MIN,
		     CFG_IGNORE_DYNAMIC_DTIM_IN_P2P_MODE_MAX),

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

	REG_VARIABLE(CFG_ENABLE_RX_LDPC, WLAN_PARAM_Integer,
		     struct hdd_config, enableRxLDPC,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_RX_LDPC_DEFAULT,
		     CFG_ENABLE_RX_LDPC_MIN,
		     CFG_ENABLE_RX_LDPC_MAX),

	REG_VARIABLE(CFG_PPS_ENABLE_5G_EBT, WLAN_PARAM_Integer,
		     struct hdd_config, enable5gEBT,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_PPS_ENABLE_5G_EBT_FEATURE_DEFAULT,
		     CFG_PPS_ENABLE_5G_EBT_FEATURE_MIN,
		     CFG_PPS_ENABLE_5G_EBT_FEATURE_MAX),

#ifdef FEATURE_WLAN_TDLS
	REG_VARIABLE(CFG_TDLS_SUPPORT_ENABLE, WLAN_PARAM_Integer,
		     struct hdd_config, fEnableTDLSSupport,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TDLS_SUPPORT_ENABLE_DEFAULT,
		     CFG_TDLS_SUPPORT_ENABLE_MIN,
		     CFG_TDLS_SUPPORT_ENABLE_MAX),

	REG_VARIABLE(CFG_TDLS_IMPLICIT_TRIGGER, WLAN_PARAM_Integer,
		     struct hdd_config, fEnableTDLSImplicitTrigger,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TDLS_IMPLICIT_TRIGGER_DEFAULT,
		     CFG_TDLS_IMPLICIT_TRIGGER_MIN,
		     CFG_TDLS_IMPLICIT_TRIGGER_MAX),

	REG_VARIABLE(CFG_TDLS_TX_STATS_PERIOD, WLAN_PARAM_Integer,
		     struct hdd_config, fTDLSTxStatsPeriod,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TDLS_TX_STATS_PERIOD_DEFAULT,
		     CFG_TDLS_TX_STATS_PERIOD_MIN,
		     CFG_TDLS_TX_STATS_PERIOD_MAX),

	REG_VARIABLE(CFG_TDLS_TX_PACKET_THRESHOLD, WLAN_PARAM_Integer,
		     struct hdd_config, fTDLSTxPacketThreshold,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TDLS_TX_PACKET_THRESHOLD_DEFAULT,
		     CFG_TDLS_TX_PACKET_THRESHOLD_MIN,
		     CFG_TDLS_TX_PACKET_THRESHOLD_MAX),

	REG_VARIABLE(CFG_TDLS_MAX_DISCOVERY_ATTEMPT, WLAN_PARAM_Integer,
		     struct hdd_config, fTDLSMaxDiscoveryAttempt,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TDLS_MAX_DISCOVERY_ATTEMPT_DEFAULT,
		     CFG_TDLS_MAX_DISCOVERY_ATTEMPT_MIN,
		     CFG_TDLS_MAX_DISCOVERY_ATTEMPT_MAX),

	REG_VARIABLE(CFG_TDLS_IDLE_TIMEOUT, WLAN_PARAM_Integer,
		     struct hdd_config, tdls_idle_timeout,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TDLS_IDLE_TIMEOUT_DEFAULT,
		     CFG_TDLS_IDLE_TIMEOUT_MIN,
		     CFG_TDLS_IDLE_TIMEOUT_MAX),

	REG_VARIABLE(CFG_TDLS_IDLE_PACKET_THRESHOLD, WLAN_PARAM_Integer,
		     struct hdd_config, fTDLSIdlePacketThreshold,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TDLS_IDLE_PACKET_THRESHOLD_DEFAULT,
		     CFG_TDLS_IDLE_PACKET_THRESHOLD_MIN,
		     CFG_TDLS_IDLE_PACKET_THRESHOLD_MAX),

	REG_VARIABLE(CFG_TDLS_RSSI_TRIGGER_THRESHOLD, WLAN_PARAM_SignedInteger,
		     struct hdd_config, fTDLSRSSITriggerThreshold,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TDLS_RSSI_TRIGGER_THRESHOLD_DEFAULT,
		     CFG_TDLS_RSSI_TRIGGER_THRESHOLD_MIN,
		     CFG_TDLS_RSSI_TRIGGER_THRESHOLD_MAX),

	REG_VARIABLE(CFG_TDLS_RSSI_TEARDOWN_THRESHOLD, WLAN_PARAM_SignedInteger,
		     struct hdd_config, fTDLSRSSITeardownThreshold,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TDLS_RSSI_TEARDOWN_THRESHOLD_DEFAULT,
		     CFG_TDLS_RSSI_TEARDOWN_THRESHOLD_MIN,
		     CFG_TDLS_RSSI_TEARDOWN_THRESHOLD_MAX),

	REG_VARIABLE(CFG_TDLS_RSSI_DELTA, WLAN_PARAM_SignedInteger,
		     struct hdd_config, fTDLSRSSIDelta,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TDLS_RSSI_DELTA_DEFAULT,
		     CFG_TDLS_RSSI_DELTA_MIN,
		     CFG_TDLS_RSSI_DELTA_MAX),

	REG_VARIABLE(CFG_TDLS_QOS_WMM_UAPSD_MASK_NAME, WLAN_PARAM_HexInteger,
		     struct hdd_config, fTDLSUapsdMask,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TDLS_QOS_WMM_UAPSD_MASK_DEFAULT,
		     CFG_TDLS_QOS_WMM_UAPSD_MASK_MIN,
		     CFG_TDLS_QOS_WMM_UAPSD_MASK_MAX),

	REG_VARIABLE(CFG_TDLS_BUFFER_STA_SUPPORT_ENABLE, WLAN_PARAM_Integer,
		     struct hdd_config, fEnableTDLSBufferSta,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TDLS_BUFFER_STA_SUPPORT_ENABLE_DEFAULT,
		     CFG_TDLS_BUFFER_STA_SUPPORT_ENABLE_MIN,
		     CFG_TDLS_BUFFER_STA_SUPPORT_ENABLE_MAX),

	REG_VARIABLE(CFG_TDLS_OFF_CHANNEL_SUPPORT_ENABLE, WLAN_PARAM_Integer,
		     struct hdd_config, fEnableTDLSOffChannel,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TDLS_OFF_CHANNEL_SUPPORT_ENABLE_DEFAULT,
		     CFG_TDLS_OFF_CHANNEL_SUPPORT_ENABLE_MIN,
		     CFG_TDLS_OFF_CHANNEL_SUPPORT_ENABLE_MAX),

	REG_VARIABLE(CFG_TDLS_PREFERRED_OFF_CHANNEL_NUM, WLAN_PARAM_Integer,
		     struct hdd_config, fTDLSPrefOffChanNum,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TDLS_PREFERRED_OFF_CHANNEL_NUM_DEFAULT,
		     CFG_TDLS_PREFERRED_OFF_CHANNEL_NUM_MIN,
		     CFG_TDLS_PREFERRED_OFF_CHANNEL_NUM_MAX),

	REG_VARIABLE(CFG_TDLS_PREFERRED_OFF_CHANNEL_BW, WLAN_PARAM_Integer,
		     struct hdd_config, fTDLSPrefOffChanBandwidth,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TDLS_PREFERRED_OFF_CHANNEL_BW_DEFAULT,
		     CFG_TDLS_PREFERRED_OFF_CHANNEL_BW_MIN,
		     CFG_TDLS_PREFERRED_OFF_CHANNEL_BW_MAX),

	REG_VARIABLE(CFG_TDLS_PUAPSD_INACTIVITY_TIME, WLAN_PARAM_Integer,
		     struct hdd_config, fTDLSPuapsdInactivityTimer,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TDLS_PUAPSD_INACTIVITY_TIME_DEFAULT,
		     CFG_TDLS_PUAPSD_INACTIVITY_TIME_MIN,
		     CFG_TDLS_PUAPSD_INACTIVITY_TIME_MAX),

	REG_VARIABLE(CFG_TDLS_PUAPSD_RX_FRAME_THRESHOLD, WLAN_PARAM_Integer,
		     struct hdd_config, fTDLSRxFrameThreshold,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TDLS_PUAPSD_RX_FRAME_THRESHOLD_DEFAULT,
		     CFG_TDLS_PUAPSD_RX_FRAME_THRESHOLD_MIN,
		     CFG_TDLS_PUAPSD_RX_FRAME_THRESHOLD_MAX),

	REG_VARIABLE(CFG_TDLS_PUAPSD_PEER_TRAFFIC_IND_WINDOW,
		     WLAN_PARAM_Integer,
		     struct hdd_config, fTDLSPuapsdPTIWindow,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TDLS_PUAPSD_PEER_TRAFFIC_IND_WINDOW_DEFAULT,
		     CFG_TDLS_PUAPSD_PEER_TRAFFIC_IND_WINDOW_MIN,
		     CFG_TDLS_PUAPSD_PEER_TRAFFIC_IND_WINDOW_MAX),

	REG_VARIABLE(CFG_TDLS_PUAPSD_PEER_TRAFFIC_RSP_TIMEOUT,
		     WLAN_PARAM_Integer,
		     struct hdd_config, fTDLSPuapsdPTRTimeout,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TDLS_PUAPSD_PEER_TRAFFIC_RSP_TIMEOUT_DEFAULT,
		     CFG_TDLS_PUAPSD_PEER_TRAFFIC_RSP_TIMEOUT_MIN,
		     CFG_TDLS_PUAPSD_PEER_TRAFFIC_RSP_TIMEOUT_MAX),

	REG_VARIABLE(CFG_TDLS_EXTERNAL_CONTROL, WLAN_PARAM_Integer,
		     struct hdd_config, fTDLSExternalControl,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TDLS_EXTERNAL_CONTROL_DEFAULT,
		     CFG_TDLS_EXTERNAL_CONTROL_MIN,
		     CFG_TDLS_EXTERNAL_CONTROL_MAX),
	REG_VARIABLE(CFG_TDLS_WMM_MODE_ENABLE, WLAN_PARAM_Integer,
		     struct hdd_config, fEnableTDLSWmmMode,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TDLS_WMM_MODE_ENABLE_DEFAULT,
		     CFG_TDLS_WMM_MODE_ENABLE_MIN,
		     CFG_TDLS_WMM_MODE_ENABLE_MAX),

	REG_VARIABLE(CFG_TDLS_SCAN_ENABLE, WLAN_PARAM_Integer,
		     struct hdd_config, enable_tdls_scan,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TDLS_SCAN_ENABLE_DEFAULT,
		     CFG_TDLS_SCAN_ENABLE_MIN,
		     CFG_TDLS_SCAN_ENABLE_MAX),

	REG_VARIABLE(CFG_TDLS_PEER_KICKOUT_THRESHOLD, WLAN_PARAM_Integer,
		     struct hdd_config, tdls_peer_kickout_threshold,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TDLS_PEER_KICKOUT_THRESHOLD_DEFAULT,
		     CFG_TDLS_PEER_KICKOUT_THRESHOLD_MIN,
		     CFG_TDLS_PEER_KICKOUT_THRESHOLD_MAX),

#endif

#ifdef WLAN_SOFTAP_VSTA_FEATURE
	REG_VARIABLE(CFG_VSTA_SUPPORT_ENABLE, WLAN_PARAM_Integer,
		     struct hdd_config, fEnableVSTASupport,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_VSTA_SUPPORT_ENABLE_DEFAULT,
		     CFG_VSTA_SUPPORT_ENABLE_MIN,
		     CFG_VSTA_SUPPORT_ENABLE_MAX),
#endif
	REG_VARIABLE(CFG_ENABLE_LPWR_IMG_TRANSITION_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, enableLpwrImgTransition,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_LPWR_IMG_TRANSITION_DEFAULT,
		     CFG_ENABLE_LPWR_IMG_TRANSITION_MIN,
		     CFG_ENABLE_LPWR_IMG_TRANSITION_MAX),

	REG_VARIABLE(CFG_ENABLE_LPWR_IMG_TRANSITION_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, enableLpwrImgTransition,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_LPWR_IMG_TRANSITION_DEFAULT,
		     CFG_ENABLE_LPWR_IMG_TRANSITION_MIN,
		     CFG_ENABLE_LPWR_IMG_TRANSITION_MAX),

	REG_VARIABLE(CFG_SCAN_AGING_PARAM_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, scanAgingTimeout,
		     VAR_FLAGS_OPTIONAL,
		     CFG_SCAN_AGING_PARAM_DEFAULT,
		     CFG_SCAN_AGING_PARAM_MIN,
		     CFG_SCAN_AGING_PARAM_MAX),

	REG_VARIABLE(CFG_TX_LDPC_ENABLE_FEATURE, WLAN_PARAM_Integer,
		     struct hdd_config, enableTxLdpc,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TX_LDPC_ENABLE_FEATURE_DEFAULT,
		     CFG_TX_LDPC_ENABLE_FEATURE_MIN,
		     CFG_TX_LDPC_ENABLE_FEATURE_MAX),

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

	REG_VARIABLE(CFG_MAX_MEDIUM_TIME, WLAN_PARAM_Integer,
		     struct hdd_config, cfgMaxMediumTime,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_MAX_MEDIUM_TIME_STADEFAULT,
		     CFG_MAX_MEDIUM_TIME_STAMIN,
		     CFG_MAX_MEDIUM_TIME_STAMAX),

	REG_VARIABLE(CFG_ENABLE_VHT_FOR_24GHZ_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, enableVhtFor24GHzBand,
		     VAR_FLAGS_OPTIONAL,
		     CFG_ENABLE_VHT_FOR_24GHZ_DEFAULT,
		     CFG_ENABLE_VHT_FOR_24GHZ_MIN,
		     CFG_ENABLE_VHT_FOR_24GHZ_MAX),

	REG_DYNAMIC_VARIABLE(CFG_ENABLE_FAST_ROAM_IN_CONCURRENCY,
			     WLAN_PARAM_Integer,
			     struct hdd_config, bFastRoamInConIniFeatureEnabled,
			     VAR_FLAGS_OPTIONAL |
			     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			     CFG_ENABLE_FAST_ROAM_IN_CONCURRENCY_DEFAULT,
			     CFG_ENABLE_FAST_ROAM_IN_CONCURRENCY_MIN,
			     CFG_ENABLE_FAST_ROAM_IN_CONCURRENCY_MAX,
			     cb_notify_set_enable_fast_roam_in_concurrency, 0),

	REG_VARIABLE(CFG_ENABLE_ADAPT_RX_DRAIN_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, fEnableAdaptRxDrain,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_ENABLE_ADAPT_RX_DRAIN_DEFAULT,
		     CFG_ENABLE_ADAPT_RX_DRAIN_MIN,
		     CFG_ENABLE_ADAPT_RX_DRAIN_MAX),

	REG_VARIABLE(CFG_ENABLE_HEART_BEAT_OFFLOAD, WLAN_PARAM_Integer,
		     struct hdd_config, enableIbssHeartBeatOffload,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_HEART_BEAT_OFFLOAD_DEFAULT,
		     CFG_ENABLE_HEART_BEAT_OFFLOAD_MIN,
		     CFG_ENABLE_HEART_BEAT_OFFLOAD_MAX),

	REG_VARIABLE(CFG_ANTENNA_DIVERSITY_PARAM_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, antennaDiversity,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ANTENNA_DIVERSITY_PARAM_DEFAULT,
		     CFG_ANTENNA_DIVERSITY_PARAM_MIN,
		     CFG_ANTENNA_DIVERSITY_PARAM_MAX),

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
	REG_VARIABLE(CFG_AMSDU_SUPPORT_IN_AMPDU_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, isAmsduSupportInAMPDU,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_AMSDU_SUPPORT_IN_AMPDU_DEFAULT,
		     CFG_AMSDU_SUPPORT_IN_AMPDU_MIN,
		     CFG_AMSDU_SUPPORT_IN_AMPDU_MAX),

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
		     CFG_ENABLE_FW_LOG_ENABLE),

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

	REG_VARIABLE(CFG_MAX_WOW_FILTERS_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, maxWoWFilters,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK,
		     CFG_MAX_WOW_FILTERS_DEFAULT,
		     CFG_MAX_WOW_FILTERS_MIN,
		     CFG_MAX_WOW_FILTERS_MAX),

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

	REG_VARIABLE(CFG_DEFAULT_RATE_INDEX_24GH, WLAN_PARAM_Integer,
		     struct hdd_config, defaultRateIndex24Ghz,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_DEFAULT_RATE_INDEX_24GH_DEFAULT,
		     CFG_DEFAULT_RATE_INDEX_24GH_MIN,
		     CFG_DEFAULT_RATE_INDEX_24GH_MAX),

#ifdef MEMORY_DEBUG
	REG_VARIABLE(CFG_ENABLE_MEMORY_DEBUG_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, IsMemoryDebugSupportEnabled,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_MEMORY_DEBUG_DEFAULT,
		     CFG_ENABLE_MEMORY_DEBUG_MIN,
		     CFG_ENABLE_MEMORY_DEBUG_MAX),
#endif

	REG_VARIABLE(CFG_DEBUG_P2P_REMAIN_ON_CHANNEL_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, debugP2pRemainOnChannel,
		     VAR_FLAGS_OPTIONAL,
		     CFG_DEBUG_P2P_REMAIN_ON_CHANNEL_DEFAULT,
		     CFG_DEBUG_P2P_REMAIN_ON_CHANNEL_MIN,
		     CFG_DEBUG_P2P_REMAIN_ON_CHANNEL_MAX),

	REG_VARIABLE(CFG_ENABLE_PACKET_LOG, WLAN_PARAM_Integer,
		     struct hdd_config, enablePacketLog,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_PACKET_LOG_DEFAULT,
		     CFG_ENABLE_PACKET_LOG_MIN,
		     CFG_ENABLE_PACKET_LOG_MAX),

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

	REG_VARIABLE_STRING(CFG_ENABLE_FW_MODULE_LOG_LEVEL, WLAN_PARAM_String,
			    struct hdd_config, enableFwModuleLogLevel,
			    VAR_FLAGS_OPTIONAL,
			    (void *)CFG_ENABLE_FW_MODULE_LOG_DEFAULT),

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

#ifdef FEATURE_GREEN_AP
	REG_VARIABLE(CFG_ENABLE_GREEN_AP_FEATURE, WLAN_PARAM_Integer,
		     struct hdd_config, enableGreenAP,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_GREEN_AP_FEATURE_DEFAULT,
		     CFG_ENABLE_GREEN_AP_FEATURE_MIN,
		     CFG_ENABLE_GREEN_AP_FEATURE_MAX),
	REG_VARIABLE(CFG_ENABLE_EGAP_ENABLE_FEATURE, WLAN_PARAM_Integer,
		     struct hdd_config, enable_egap,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_EGAP_ENABLE_FEATURE_DEFAULT,
		     CFG_ENABLE_EGAP_ENABLE_FEATURE_MIN,
		     CFG_ENABLE_EGAP_ENABLE_FEATURE_MAX),
	REG_VARIABLE(CFG_ENABLE_EGAP_INACT_TIME_FEATURE, WLAN_PARAM_Integer,
		     struct hdd_config, egap_inact_time,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_EGAP_INACT_TIME_FEATURE_DEFAULT,
		     CFG_ENABLE_EGAP_INACT_TIME_FEATURE_MIN,
		     CFG_ENABLE_EGAP_INACT_TIME_FEATURE_MAX),
	REG_VARIABLE(CFG_ENABLE_EGAP_WAIT_TIME_FEATURE, WLAN_PARAM_Integer,
		     struct hdd_config, egap_wait_time,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_EGAP_WAIT_TIME_FEATURE_DEFAULT,
		     CFG_ENABLE_EGAP_WAIT_TIME_FEATURE_MIN,
		     CFG_ENABLE_EGAP_WAIT_TIME_FEATURE_MAX),
	REG_VARIABLE(CFG_ENABLE_EGAP_FLAGS_FEATURE, WLAN_PARAM_Integer,
		     struct hdd_config, egap_feature_flag,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_EGAP_FLAGS_FEATURE_DEFAULT,
		     CFG_ENABLE_EGAP_FLAGS_FEATURE_MIN,
		     CFG_ENABLE_EGAP_FLAGS_FEATURE_MAX),
#endif

	REG_VARIABLE(CFG_IGNORE_CAC_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, ignoreCAC,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_IGNORE_CAC_DEFAULT,
		     CFG_IGNORE_CAC_MIN,
		     CFG_IGNORE_CAC_MAX),

	REG_VARIABLE(CFG_ENABLE_SAP_DFS_CH_SIFS_BURST_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, IsSapDfsChSifsBurstEnabled,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_SAP_DFS_CH_SIFS_BURST_DEFAULT,
		     CFG_ENABLE_SAP_DFS_CH_SIFS_BURST_MIN,
		     CFG_ENABLE_SAP_DFS_CH_SIFS_BURST_MAX),

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
		     struct hdd_config, wlanLoggingEnable,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_WLAN_LOGGING_SUPPORT_DEFAULT,
		     CFG_WLAN_LOGGING_SUPPORT_DISABLE,
		     CFG_WLAN_LOGGING_SUPPORT_ENABLE),

	REG_VARIABLE(CFG_WLAN_LOGGING_FE_CONSOLE_SUPPORT_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, wlanLoggingFEToConsole,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_WLAN_LOGGING_FE_CONSOLE_SUPPORT_DEFAULT,
		     CFG_WLAN_LOGGING_FE_CONSOLE_SUPPORT_DISABLE,
		     CFG_WLAN_LOGGING_FE_CONSOLE_SUPPORT_ENABLE),

	REG_VARIABLE(CFG_WLAN_LOGGING_NUM_BUF_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, wlanLoggingNumBuf,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_WLAN_LOGGING_NUM_BUF_DEFAULT,
		     CFG_WLAN_LOGGING_NUM_BUF_MIN,
		     CFG_WLAN_LOGGING_NUM_BUF_MAX),
#endif /* WLAN_LOGGING_SOCK_SVC_ENABLE */

	REG_VARIABLE(CFG_ENABLE_SIFS_BURST, WLAN_PARAM_Integer,
		     struct hdd_config, enableSifsBurst,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_SIFS_BURST_DEFAULT,
		     CFG_ENABLE_SIFS_BURST_MIN,
		     CFG_ENABLE_SIFS_BURST_MAX),

#ifdef WLAN_FEATURE_LPSS
	REG_VARIABLE(CFG_ENABLE_LPASS_SUPPORT, WLAN_PARAM_Integer,
		     struct hdd_config, enable_lpass_support,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_LPASS_SUPPORT_DEFAULT,
		     CFG_ENABLE_LPASS_SUPPORT_MIN,
		     CFG_ENABLE_LPASS_SUPPORT_MAX),
#endif

#ifdef WLAN_FEATURE_NAN
	REG_VARIABLE(CFG_ENABLE_NAN_SUPPORT, WLAN_PARAM_Integer,
		     struct hdd_config, enable_nan_support,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_NAN_SUPPORT_DEFAULT,
		     CFG_ENABLE_NAN_SUPPORT_MIN,
		     CFG_ENABLE_NAN_SUPPORT_MAX),
#endif

	REG_VARIABLE(CFG_ENABLE_SELF_RECOVERY, WLAN_PARAM_Integer,
		     struct hdd_config, enableSelfRecovery,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_ENABLE_SELF_RECOVERY_DEFAULT,
		     CFG_ENABLE_SELF_RECOVERY_MIN,
		     CFG_ENABLE_SELF_RECOVERY_MAX),

#ifdef FEATURE_WLAN_FORCE_SAP_SCC
	REG_VARIABLE(CFG_SAP_SCC_CHAN_AVOIDANCE, WLAN_PARAM_Integer,
		     struct hdd_config, SapSccChanAvoidance,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_SAP_SCC_CHAN_AVOIDANCE_DEFAULT,
		     CFG_SAP_SCC_CHAN_AVOIDANCE_MIN,
		     CFG_SAP_SCC_CHAN_AVOIDANCE_MAX),
#endif /* FEATURE_WLAN_FORCE_SAP_SCC */

	REG_VARIABLE(CFG_ENABLE_SAP_SUSPEND, WLAN_PARAM_Integer,
		     struct hdd_config, enableSapSuspend,
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
		struct hdd_config, sendDeauthBeforeCon,
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

	REG_VARIABLE(CFG_SAP_P2P_11AC_OVERRIDE_NAME, WLAN_PARAM_Integer,
			struct hdd_config, sap_p2p_11ac_override,
			VAR_FLAGS_OPTIONAL |
					VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			CFG_SAP_P2P_11AC_OVERRIDE_DEFAULT,
			CFG_SAP_P2P_11AC_OVERRIDE_MIN,
			CFG_SAP_P2P_11AC_OVERRIDE_MAX),

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

	REG_VARIABLE(CFG_BPF_PACKET_FILTER_OFFLOAD, WLAN_PARAM_Integer,
		     struct hdd_config, bpf_packet_filter_enable,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_BPF_PACKET_FILTER_OFFLOAD_DEFAULT,
		     CFG_BPF_PACKET_FILTER_OFFLOAD_MIN,
		     CFG_BPF_PACKET_FILTER_OFFLOAD_MAX),

	REG_VARIABLE(CFG_FLOW_STEERING_ENABLED_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, flow_steering_enable,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_FLOW_STEERING_ENABLED_DEFAULT,
		     CFG_FLOW_STEERING_ENABLED_MIN,
		     CFG_FLOW_STEERING_ENABLED_MAX),

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

#ifdef FEATURE_WLAN_EXTSCAN
	REG_VARIABLE(CFG_EXTSCAN_ALLOWED_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, extscan_enabled,
		     VAR_FLAGS_OPTIONAL |
		     VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_EXTSCAN_ALLOWED_DEF,
		     CFG_EXTSCAN_ALLOWED_MIN,
		     CFG_EXTSCAN_ALLOWED_MAX),

	REG_VARIABLE(CFG_EXTSCAN_PASSIVE_MAX_CHANNEL_TIME_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, extscan_passive_max_chn_time,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_EXTSCAN_PASSIVE_MAX_CHANNEL_TIME_DEFAULT,
		     CFG_EXTSCAN_PASSIVE_MAX_CHANNEL_TIME_MIN,
		     CFG_EXTSCAN_PASSIVE_MAX_CHANNEL_TIME_MAX),

	REG_VARIABLE(CFG_EXTSCAN_PASSIVE_MIN_CHANNEL_TIME_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, extscan_passive_min_chn_time,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_EXTSCAN_PASSIVE_MIN_CHANNEL_TIME_DEFAULT,
		     CFG_EXTSCAN_PASSIVE_MIN_CHANNEL_TIME_MIN,
		     CFG_EXTSCAN_PASSIVE_MIN_CHANNEL_TIME_MAX),

	REG_VARIABLE(CFG_EXTSCAN_ACTIVE_MAX_CHANNEL_TIME_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, extscan_active_max_chn_time,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_EXTSCAN_ACTIVE_MAX_CHANNEL_TIME_DEFAULT,
		     CFG_EXTSCAN_ACTIVE_MAX_CHANNEL_TIME_MIN,
		     CFG_EXTSCAN_ACTIVE_MAX_CHANNEL_TIME_MAX),

	REG_VARIABLE(CFG_EXTSCAN_ACTIVE_MIN_CHANNEL_TIME_NAME,
		     WLAN_PARAM_Integer,
		     struct hdd_config, extscan_active_min_chn_time,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_EXTSCAN_ACTIVE_MIN_CHANNEL_TIME_DEFAULT,
		     CFG_EXTSCAN_ACTIVE_MIN_CHANNEL_TIME_MIN,
		     CFG_EXTSCAN_ACTIVE_MIN_CHANNEL_TIME_MAX),
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

	REG_VARIABLE(CFG_TX_CHAIN_MASK_CCK, WLAN_PARAM_Integer,
		     struct hdd_config, tx_chain_mask_cck,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TX_CHAIN_MASK_CCK_DEFAULT,
		     CFG_TX_CHAIN_MASK_CCK_MIN,
		     CFG_TX_CHAIN_MASK_CCK_MAX),

	REG_VARIABLE(CFG_TX_CHAIN_MASK_1SS, WLAN_PARAM_Integer,
		     struct hdd_config, tx_chain_mask_1ss,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_TX_CHAIN_MASK_1SS_DEFAULT,
		     CFG_TX_CHAIN_MASK_1SS_MIN,
		     CFG_TX_CHAIN_MASK_1SS_MAX),

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

	REG_VARIABLE(CFG_IGNORE_PEER_HT_MODE_NAME, WLAN_PARAM_Integer,
			struct hdd_config, ignore_peer_ht_opmode,
			VAR_FLAGS_OPTIONAL |
			VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
			CFG_IGNORE_PEER_HT_MODE_DEFAULT,
			CFG_IGNORE_PEER_HT_MODE_MIN,
			CFG_IGNORE_PEER_HT_MODE_MAX),

	REG_VARIABLE(CFG_ROAM_DENSE_MIN_APS, WLAN_PARAM_Integer,
		struct hdd_config, roam_dense_min_aps,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ROAM_DENSE_MIN_APS_DEFAULT,
		CFG_ROAM_DENSE_MIN_APS_MIN,
		CFG_ROAM_DENSE_MIN_APS_MAX),

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
#ifdef WLAN_FEATURE_NAN_DATAPATH
	REG_VARIABLE(CFG_ENABLE_NAN_DATAPATH_NAME, WLAN_PARAM_Integer,
		struct hdd_config, enable_nan_datapath,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ENABLE_NAN_DATAPATH_DEFAULT,
		CFG_ENABLE_NAN_DATAPATH_MIN,
		CFG_ENABLE_NAN_DATAPATH_MAX),

	REG_VARIABLE(CFG_ENABLE_NAN_NDI_CHANNEL_NAME, WLAN_PARAM_Integer,
		struct hdd_config, nan_datapath_ndi_channel,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ENABLE_NAN_NDI_CHANNEL_DEFAULT,
		CFG_ENABLE_NAN_NDI_CHANNEL_MIN,
		CFG_ENABLE_NAN_NDI_CHANNEL_MAX),
#endif
	REG_VARIABLE(CFG_CREATE_BUG_REPORT_FOR_SCAN, WLAN_PARAM_Integer,
		struct hdd_config, bug_report_for_no_scan_results,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_CREATE_BUG_REPORT_FOR_SCAN_DEFAULT,
		CFG_CREATE_BUG_REPORT_FOR_SCAN_DISABLE,
		CFG_CREATE_BUG_REPORT_FOR_SCAN_ENABLE),

	REG_VARIABLE(CFG_ENABLE_DP_TRACE, WLAN_PARAM_Integer,
		struct hdd_config, enable_dp_trace,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ENABLE_DP_TRACE_DEFAULT,
		CFG_ENABLE_DP_TRACE_MIN,
		CFG_ENABLE_DP_TRACE_MAX),

	REG_VARIABLE(CFG_ADAPTIVE_SCAN_DWELL_MODE_NAME, WLAN_PARAM_Integer,
		struct hdd_config, scan_adaptive_dwell_mode,
		VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		CFG_ADAPTIVE_SCAN_DWELL_MODE_DEFAULT,
		CFG_ADAPTIVE_SCAN_DWELL_MODE_MIN,
		CFG_ADAPTIVE_SCAN_DWELL_MODE_MAX),

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

	REG_VARIABLE(CFG_SIFS_BURST_DURATION_NAME, WLAN_PARAM_Integer,
		     struct hdd_config, sifs_burst_duration,
		     VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT,
		     CFG_SIFS_BURST_DURATION_DEFAULT,
		     CFG_SIFS_BURST_DURATION_MIN,
		     CFG_SIFS_BURST_DURATION_MAX),
};

/**
 * get_next_line() - find and locate the new line pointer
 * @str: pointer to string
 *
 * This function returns a pointer to the character after the occurence
 * of a new line character. It also modifies the original string by replacing
 * the '\n' character with the null character.
 *
 * Return: the pointer to the character at new line,
 *            or NULL if no new line character was found
 */
static char *get_next_line(char *str)
{
	char c;

	if (str == NULL || *str == '\0') {
		return NULL;
	}

	c = *str;
	while (c != '\n' && c != '\0' && c != 0xd) {
		str = str + 1;
		c = *str;
	}

	if (c == '\0') {
		return NULL;
	} else {
		*str = '\0';
		return str + 1;
	}

	return NULL;
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
 * @pHddCtx: pointer to hdd context
 * @pBuf: buffer to store the configuration
 * @buflen: size of the buffer
 *
 * Return: QDF_STATUS_SUCCESS if the configuration and buffer size can carry
 *		the content, otherwise QDF_STATUS_E_RESOURCES
 */
static QDF_STATUS hdd_cfg_get_config(REG_TABLE_ENTRY *reg_table,
				     unsigned long cRegTableEntries,
				     uint8_t *ini_struct,
				     hdd_context_t *pHddCtx, char *pBuf,
				     int buflen)
{
	unsigned int idx;
	REG_TABLE_ENTRY *pRegEntry = reg_table;
	uint32_t value;
	char valueStr[CFG_VALUE_MAX_LEN];
	char configStr[CFG_ENTRY_MAX_LEN];
	char *fmt;
	void *pField;
	struct qdf_mac_addr *pMacAddr;
	char *pCur = pBuf;
	int curlen;

	/* start with an empty string */
	*pCur = '\0';

	for (idx = 0; idx < cRegTableEntries; idx++, pRegEntry++) {
		pField = ini_struct + pRegEntry->VarOffset;

		if ((WLAN_PARAM_Integer == pRegEntry->RegType) ||
		    (WLAN_PARAM_SignedInteger == pRegEntry->RegType) ||
		    (WLAN_PARAM_HexInteger == pRegEntry->RegType)) {
			value = 0;
			memcpy(&value, pField, pRegEntry->VarSize);
			if (WLAN_PARAM_HexInteger == pRegEntry->RegType) {
				fmt = "%x";
			} else if (WLAN_PARAM_SignedInteger ==
				   pRegEntry->RegType) {
				fmt = "%d";
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
		curlen = scnprintf(configStr, CFG_ENTRY_MAX_LEN,
				   "%s=[%s]%s\n",
				   pRegEntry->RegName,
				   valueStr,
				   test_bit(idx,
					    (void *)&pHddCtx->config->
					    bExplicitCfg) ? "*" : "");

		/* Ideally we want to return the config to the application,
		 * however the config is too big so we just printk() for now
		 */
#ifdef RETURN_IN_BUFFER
		if (curlen < buflen) {
			/* copy string + '\0' */
			memcpy(pCur, configStr, curlen + 1);

			/* account for addition; */
			pCur += curlen;
			buflen -= curlen;
		} else {
			/* buffer space exhausted, return what we have */
			return QDF_STATUS_E_RESOURCES;
		}
#else
		printk(KERN_INFO "%s", configStr);
#endif /* RETURN_IN_BUFFER */

	}

#ifndef RETURN_IN_BUFFER
	/* notify application that output is in system log */
	snprintf(pCur, buflen, "WLAN configuration written to system log");
#endif /* RETURN_IN_BUFFER */

	return QDF_STATUS_SUCCESS;
}

/** struct tCfgIniEntry - ini configuration entry
 *
 * @name: name of the entry
 * @value: value of the entry
 */
typedef struct {
	char *name;
	char *value;
} tCfgIniEntry;

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
static QDF_STATUS find_cfg_item(tCfgIniEntry *iniTable, unsigned long entries,
				char *name, char **value)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	unsigned long i;

	for (i = 0; i < entries; i++) {
		if (strcmp(iniTable[i].name, name) == 0) {
			*value = iniTable[i].value;
			QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
				  "Found %s entry for Name=[%s] Value=[%s] ",
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
 * @pHddCtx: the pointer to hdd context
 * @macTable: the macTable to carry the conversion
 * @num: number of the interface
 *
 * 00AA00BB00CC -> 0x00 0xAA 0x00 0xBB 0x00 0xCC
 *
 * Return: None
 */
static void update_mac_from_string(hdd_context_t *pHddCtx,
				   tCfgIniEntry *macTable, int num)
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
			qdf_mem_copy((uint8_t *) &pHddCtx->config->
				     intfMacAddr[i].bytes[0],
				     (uint8_t *) &macaddr[i].bytes[0],
				     QDF_MAC_ADDR_SIZE);
		}
	}
}

/**
 * hdd_apply_cfg_ini() - apply the ini configuration file
 * @pHddCtx: the pointer to hdd context
 * @iniTable: pointer to configuration table
 * @entries: number fo the configuration entries
 * It overwrites the MAC address if config file exist.
 *
 * Return: QDF_STATUS_SUCCESS if the ini configuration file is correctly parsed,
 *		otherwise QDF_STATUS_E_INVAL
 */
static QDF_STATUS hdd_apply_cfg_ini(hdd_context_t *pHddCtx,
				    tCfgIniEntry *iniTable,
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
	void *pStructBase = pHddCtx->config;
	REG_TABLE_ENTRY *pRegEntry = g_registry_table;
	unsigned long cRegTableEntries = QDF_ARRAY_SIZE(g_registry_table);
	uint32_t cbOutString;
	int i;
	int rv;

	if (MAX_CFG_INI_ITEMS < cRegTableEntries) {
		hddLog(LOGE, FL("MAX_CFG_INI_ITEMS too small, must be at least %ld"),
		       cRegTableEntries);
		WARN_ON(1);
	}

	for (idx = 0; idx < cRegTableEntries; idx++, pRegEntry++) {
		/* Calculate the address of the destination field in the structure. */
		pField = ((uint8_t *) pStructBase) + pRegEntry->VarOffset;

		match_status =
			find_cfg_item(iniTable, entries, pRegEntry->RegName,
				      &value_str);

		if ((match_status != QDF_STATUS_SUCCESS)
		    && (pRegEntry->Flags & VAR_FLAGS_REQUIRED)) {
			/* If we could not read the cfg item and it is required, this is an error. */
			hddLog(LOGE,
			       "%s: Failed to read required config parameter %s",
			       __func__, pRegEntry->RegName);
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
					hddLog(LOGE,
					       "%s: Reg Parameter %s invalid. Enforcing default",
					       __func__, pRegEntry->RegName);
					value = pRegEntry->VarDefault;
				}
			} else if (match_status == QDF_STATUS_SUCCESS
				   && (WLAN_PARAM_HexInteger ==
				       pRegEntry->RegType)) {
				rv = kstrtou32(value_str, 16, &value);
				if (rv < 0) {
					hddLog(LOGE,
					       "%s: Reg paramter %s invalid. Enforcing default",
					       __func__, pRegEntry->RegName);
					value = pRegEntry->VarDefault;
				}
			} else {
				value = pRegEntry->VarDefault;
			}

			/* If this parameter needs range checking, do it here. */
			if (pRegEntry->Flags & VAR_FLAGS_RANGE_CHECK) {
				if (value > pRegEntry->VarMax) {
					hddLog(LOGE,
					       "%s: Reg Parameter %s > allowed Maximum [%u > %lu]. Enforcing Maximum",
					       __func__, pRegEntry->RegName,
					       value, pRegEntry->VarMax);
					value = pRegEntry->VarMax;
				}

				if (value < pRegEntry->VarMin) {
					hddLog(LOGE,
					       "%s: Reg Parameter %s < allowed Minimum [%u < %lu]. Enforcing Minimum",
					       __func__, pRegEntry->RegName,
					       value, pRegEntry->VarMin);
					value = pRegEntry->VarMin;
				}
			}
			/* If this parameter needs range checking, do it here. */
			else if (pRegEntry->
				 Flags & VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT) {
				if (value > pRegEntry->VarMax) {
					hddLog(LOGE,
					       "%s: Reg Parameter %s > allowed Maximum [%u > %lu]. Enforcing Default= %lu",
					       __func__, pRegEntry->RegName,
					       value, pRegEntry->VarMax,
					       pRegEntry->VarDefault);
					value = pRegEntry->VarDefault;
				}

				if (value < pRegEntry->VarMin) {
					hddLog(LOGE,
					       "%s: Reg Parameter %s < allowed Minimum [%u < %lu]. Enforcing Default= %lu",
					       __func__, pRegEntry->RegName,
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
					hddLog(QDF_TRACE_LEVEL_WARN,
					       "%s: Reg Parameter %s invalid. Enforcing Default",
					       __func__, pRegEntry->RegName);
					svalue =
						(int32_t) pRegEntry->VarDefault;
				}
			} else {
				svalue = (int32_t) pRegEntry->VarDefault;
			}

			/* If this parameter needs range checking, do it here. */
			if (pRegEntry->Flags & VAR_FLAGS_RANGE_CHECK) {
				if (svalue > (int32_t) pRegEntry->VarMax) {
					hddLog(LOGE,
					       "%s: Reg Parameter %s > allowed Maximum "
					       "[%d > %d]. Enforcing Maximum",
					       __func__, pRegEntry->RegName,
					       svalue, (int)pRegEntry->VarMax);
					svalue = (int32_t) pRegEntry->VarMax;
				}

				if (svalue < (int32_t) pRegEntry->VarMin) {
					hddLog(LOGE,
					       "%s: Reg Parameter %s < allowed Minimum "
					       "[%d < %d]. Enforcing Minimum",
					       __func__, pRegEntry->RegName,
					       svalue, (int)pRegEntry->VarMin);
					svalue = (int32_t) pRegEntry->VarMin;
				}
			}
			/* If this parameter needs range checking, do it here. */
			else if (pRegEntry->
				 Flags & VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT) {
				if (svalue > (int32_t) pRegEntry->VarMax) {
					hddLog(LOGE,
					       "%s: Reg Parameter %s > allowed Maximum "
					       "[%d > %d]. Enforcing Default= %d",
					       __func__, pRegEntry->RegName,
					       svalue, (int)pRegEntry->VarMax,
					       (int)pRegEntry->VarDefault);
					svalue =
						(int32_t) pRegEntry->VarDefault;
				}

				if (svalue < (int32_t) pRegEntry->VarMin) {
					hddLog(LOGE,
					       "%s: Reg Parameter %s < allowed Minimum "
					       "[%d < %d]. Enforcing Default= %d",
					       __func__, pRegEntry->RegName,
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
			QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
				  "RegName = %s, VarOffset %u VarSize %u VarDefault %s",
				  pRegEntry->RegName, pRegEntry->VarOffset,
				  pRegEntry->VarSize,
				  (char *)pRegEntry->VarDefault);
#endif

			if (match_status == QDF_STATUS_SUCCESS) {
				len_value_str = strlen(value_str);

				if (len_value_str > (pRegEntry->VarSize - 1)) {
					hddLog(LOGE,
					       "%s: Invalid Value=[%s] specified for Name=[%s] in %s",
					       __func__, value_str,
					       pRegEntry->RegName,
					       WLAN_INI_FILE);
					cbOutString =
						util_min(strlen
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
					util_min(strlen((char *)pRegEntry->VarDefault),
						 pRegEntry->VarSize - 1);
				memcpy(pField, (void *)(pRegEntry->VarDefault),
				       cbOutString);
				((uint8_t *) pField)[cbOutString] = '\0';
			}
		} else if (WLAN_PARAM_MacAddr == pRegEntry->RegType) {
			if (pRegEntry->VarSize != QDF_MAC_ADDR_SIZE) {
				hddLog(LOGE,
				       "%s: Invalid VarSize %u for Name=[%s]",
				       __func__, pRegEntry->VarSize,
				       pRegEntry->RegName);
				continue;
			}
			candidate = (char *)pRegEntry->VarDefault;
			if (match_status == QDF_STATUS_SUCCESS) {
				len_value_str = strlen(value_str);
				if (len_value_str != (QDF_MAC_ADDR_SIZE * 2)) {
					hddLog(LOGE,
					       "%s: Invalid MAC addr [%s] specified for Name=[%s] in %s",
					       __func__, value_str,
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
			hddLog(LOGE,
			       "%s: Unknown param type for name[%s] in registry table",
			       __func__, pRegEntry->RegName);
		}

		/* did we successfully parse a cfg item for this parameter? */
		if ((match_status == QDF_STATUS_SUCCESS) &&
		    (idx < MAX_CFG_INI_ITEMS)) {
			set_bit(idx, (void *)&pHddCtx->config->bExplicitCfg);
		}
	}

	return ret_status;
}

/**
 * hdd_execute_config_command() - executes an arbitrary configuration command
 * @reg_table: the pointer to configuration table
 * @tableSize: the size of the configuration table
 * @ini_struct: pointer to the hdd config knob
 * @pHddCtx: the pointer to hdd context
 * @command: the command to run
 *
 * Return: QDF_STATUS_SUCCESS if the command is found and able to execute,
 *		otherwise the appropriate QDF_STATUS will be returned
 */
static QDF_STATUS hdd_execute_config_command(REG_TABLE_ENTRY *reg_table,
					     unsigned long tableSize,
					     uint8_t *ini_struct,
					     hdd_context_t *pHddCtx,
					     char *command)
{
	REG_TABLE_ENTRY *pRegEntry;
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
	if (NULL == clone) {
		hddLog(LOGE,
		       "%s: memory allocation failure, unable to process [%s]",
		       __func__, command);
		return vstatus;
	}
	/* 'clone' will point to the beginning of the string so it can be freed
	 * 'pCmd' will be used to walk/parse the command
	 */
	pCmd = clone;

	/* get rid of leading/trailing whitespace */
	pCmd = i_trim(pCmd);
	if ('\0' == *pCmd) {
		/* only whitespace */
		hddLog(LOGE, "%s: invalid command, only whitespace:[%s]",
		       __func__, command);
		goto done;
	}
	/* parse the <name> = <value> */
	name = pCmd;
	while (('=' != *pCmd) && ('\0' != *pCmd)) {
		pCmd++;
	}
	if ('\0' == *pCmd) {
		/* did not find '=' */
		hddLog(LOGE, "%s: invalid command, no '=':[%s]",
		       __func__, command);
		goto done;
	}
	/* replace '=' with NUL to terminate the <name> */
	*pCmd++ = '\0';
	name = i_trim(name);
	if ('\0' == *name) {
		/* did not find a name */
		hddLog(LOGE, "%s: invalid command, no <name>:[%s]",
		       __func__, command);
		goto done;
	}

	value_str = i_trim(pCmd);
	if ('\0' == *value_str) {
		/* did not find a value */
		hddLog(LOGE, "%s: invalid command, no <value>:[%s]",
		       __func__, command);
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
		hddLog(LOGE,
		       "%s: invalid command, unknown configuration item:[%s]",
		       __func__, command);
		goto done;
	}

	pRegEntry = &reg_table[idx];
	if (!(pRegEntry->Flags & VAR_FLAGS_DYNAMIC_CFG)) {
		/* does not support dynamic configuration */
		hddLog(LOGE, "%s: Global_Registry_Table.%s does not support "
		       "dynamic configuration", __func__, name);
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
			hddLog(LOGE,
			       "%s: invalid command, value %u < min value %lu",
			       __func__, value, pRegEntry->VarMin);
			goto done;
		}
		if (value > pRegEntry->VarMax) {
			/* out of range */
			hddLog(LOGE,
			       "%s: invalid command, value %u > max value %lu",
			       __func__, value, pRegEntry->VarMax);
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
			hddLog(LOGE,
			       "%s: invalid command, value %x < min value %lx",
			       __func__, value, pRegEntry->VarMin);
			goto done;
		}
		if (value > pRegEntry->VarMax) {
			/* out of range */
			hddLog(LOGE,
			       "%s: invalid command, value %x > max value %lx",
			       __func__, value, pRegEntry->VarMax);
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
			hddLog(LOGE,
			       "%s: invalid command, value %d < min value %d",
			       __func__, svalue, (int)pRegEntry->VarMin);
			goto done;
		}
		if (svalue > (int32_t) pRegEntry->VarMax) {
			/* out of range */
			hddLog(LOGE,
			       "%s: invalid command, value %d > max value %d",
			       __func__, svalue, (int)pRegEntry->VarMax);
			goto done;
		}
		memcpy(pField, &svalue, pRegEntry->VarSize);
		break;

	case WLAN_PARAM_String:
		len_value_str = strlen(value_str);
		if (len_value_str > (pRegEntry->VarSize - 1)) {
			/* too big */
			hddLog(LOGE,
			       "%s: invalid command, string [%s] length "
			       "%zu exceeds maximum length %u",
			       __func__, value_str,
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
			hddLog(LOGE,
			       "%s: invalid command, MAC address [%s] length "
			       "%zu is not expected length %u",
			       __func__, value_str,
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
	if (NULL != pRegEntry->pfnDynamicnotify) {
		(pRegEntry->pfnDynamicnotify)(pHddCtx, pRegEntry->notifyId);
	}
	/* note that this item was explicitly configured */
	if (idx < MAX_CFG_INI_ITEMS) {
		set_bit(idx, (void *)&pHddCtx->config->bExplicitCfg);
	}
done:
	kfree(clone);
	return vstatus;
}

/**
 * hdd_set_power_save_offload_config() - set power save offload configuration
 * @pHddCtx: the pointer to hdd context
 *
 * Return: none
 */
static void hdd_set_power_save_offload_config(hdd_context_t *pHddCtx)
{
	struct hdd_config *pConfig = pHddCtx->config;
	uint32_t listenInterval = 0;

	if (strcmp(pConfig->PowerUsageControl, "Min") == 0) {
		listenInterval = pConfig->nBmpsMinListenInterval;
	} else if (strcmp(pConfig->PowerUsageControl, "Max") == 0) {
		listenInterval = pConfig->nBmpsMaxListenInterval;
	} else if (strcmp(pConfig->PowerUsageControl, "Mod") == 0) {
		listenInterval = pConfig->nBmpsModListenInterval;
	}

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
		sme_cfg_set_int(pHddCtx->hHal, WNI_CFG_LISTEN_INTERVAL,
				listenInterval);
	}

}

#ifdef FEATURE_RUNTIME_PM
static void hdd_cfg_print_runtime_pm(hdd_context_t *hdd_ctx)
{
	hdd_info("Name = [gRuntimePM] Value = [%u] ",
		 hdd_ctx->config->runtime_pm);

	hdd_info("Name = [gRuntimePMDelay] Value = [%u] ",
		 hdd_ctx->config->runtime_pm_delay);
}
#else
static void hdd_cfg_print_runtime_pm(hdd_context_t *hdd_ctx)
{
}
#endif

/**
 * hdd_cfg_print() - print the hdd configuration
 * @iniTable: pointer to hdd context
 *
 * Return: None
 */
void hdd_cfg_print(hdd_context_t *pHddCtx)
{
	int i;

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "*********Config values in HDD Adapter*******");
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [RTSThreshold] Value = %u",
		  pHddCtx->config->RTSThreshold);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [OperatingChannel] Value = [%u]",
		  pHddCtx->config->OperatingChannel);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [PowerUsageControl] Value = [%s]",
		  pHddCtx->config->PowerUsageControl);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [fIsImpsEnabled] Value = [%u]",
		  pHddCtx->config->fIsImpsEnabled);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [nVccRssiTrigger] Value = [%u]",
		  pHddCtx->config->nVccRssiTrigger);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gIbssBssid] Value =[" MAC_ADDRESS_STR "]",
		  MAC_ADDR_ARRAY(pHddCtx->config->IbssBssid.bytes));

	for (i = 0; i < QDF_MAX_CONCURRENCY_PERSONA; i++) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
			  "Name = [Intf%dMacAddress] Value =[" MAC_ADDRESS_STR "]",
			  i, MAC_ADDR_ARRAY(pHddCtx->config->intfMacAddr[i].bytes));
	}

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gApEnableUapsd] value = [%u]",
		  pHddCtx->config->apUapsdEnabled);

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gEnableApProt] value = [%u]",
		  pHddCtx->config->apProtEnabled);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gAPAutoShutOff] Value = [%u]",
		  pHddCtx->config->nAPAutoShutOff);
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gWlanMccToSccSwitchMode] Value = [%u]",
		  pHddCtx->config->WlanMccToSccSwitchMode);
#endif
#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gWlanAutoShutdown] Value = [%u]",
		  pHddCtx->config->WlanAutoShutdown);
#endif
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gApProtection] value = [%u]",
		  pHddCtx->config->apProtection);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gEnableApOBSSProt] value = [%u]",
		  pHddCtx->config->apOBSSProtEnabled);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		"Name = [%s] value = [%u]", CFG_FORCE_SAP_ACS,
		pHddCtx->config->force_sap_acs);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		"Name = [%s] value = [%u]", CFG_FORCE_SAP_ACS_START_CH,
		pHddCtx->config->force_sap_acs_st_ch);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		"Name = [%s] value = [%u]", CFG_FORCE_SAP_ACS_END_CH,
		pHddCtx->config->force_sap_acs_end_ch);
#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
	QDF_TRACE(QDF_MODULE_ID_HDD,
		  QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [sap_channel_avoidance] value = [%u]",
		  pHddCtx->config->sap_channel_avoidance);
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		"Name = [%s] value = [%u]", CFG_SAP_P2P_11AC_OVERRIDE_NAME,
				pHddCtx->config->sap_p2p_11ac_override);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [ChannelBondingMode] Value = [%u]",
		  pHddCtx->config->nChannelBondingMode24GHz);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [ChannelBondingMode] Value = [%u]",
		  pHddCtx->config->nChannelBondingMode5GHz);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [dot11Mode] Value = [%u]",
		  pHddCtx->config->dot11Mode);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [WmmMode] Value = [%u] ", pHddCtx->config->WmmMode);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [UapsdMask] Value = [0x%x] ",
		  pHddCtx->config->UapsdMask);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [ImplicitQosIsEnabled] Value = [%u]",
		  (int)pHddCtx->config->bImplicitQosEnabled);

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [InfraUapsdVoSrvIntv] Value = [%u] ",
		  pHddCtx->config->InfraUapsdVoSrvIntv);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [InfraUapsdVoSuspIntv] Value = [%u] ",
		  pHddCtx->config->InfraUapsdVoSuspIntv);

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [InfraUapsdViSrvIntv] Value = [%u] ",
		  pHddCtx->config->InfraUapsdViSrvIntv);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [InfraUapsdViSuspIntv] Value = [%u] ",
		  pHddCtx->config->InfraUapsdViSuspIntv);

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [InfraUapsdBeSrvIntv] Value = [%u] ",
		  pHddCtx->config->InfraUapsdBeSrvIntv);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [InfraUapsdBeSuspIntv] Value = [%u] ",
		  pHddCtx->config->InfraUapsdBeSuspIntv);

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [InfraUapsdBkSrvIntv] Value = [%u] ",
		  pHddCtx->config->InfraUapsdBkSrvIntv);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [InfraUapsdBkSuspIntv] Value = [%u] ",
		  pHddCtx->config->InfraUapsdBkSuspIntv);
#ifdef FEATURE_WLAN_ESE
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [InfraInactivityInterval] Value = [%u] ",
		  pHddCtx->config->InfraInactivityInterval);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [EseEnabled] Value = [%u] ",
		  pHddCtx->config->isEseIniFeatureEnabled);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [FastTransitionEnabled] Value = [%u] ",
		  pHddCtx->config->isFastTransitionEnabled);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gTxPowerCap] Value = [%u] dBm ",
		  pHddCtx->config->nTxPowerCap);
#endif
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [FastRoamEnabled] Value = [%u] ",
		  pHddCtx->config->isFastRoamIniFeatureEnabled);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [MAWCEnabled] Value = [%u] ",
		  pHddCtx->config->MAWCEnabled);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [RoamRssiDiff] Value = [%u] ",
		  pHddCtx->config->RoamRssiDiff);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [isWESModeEnabled] Value = [%u] ",
		  pHddCtx->config->isWESModeEnabled);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [OkcEnabled] Value = [%u] ",
		  pHddCtx->config->isOkcIniFeatureEnabled);
#ifdef FEATURE_WLAN_SCAN_PNO
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [configPNOScanSupport] Value = [%u] ",
		  pHddCtx->config->configPNOScanSupport);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [configPNOScanTimerRepeatValue] Value = [%u] ",
		  pHddCtx->config->configPNOScanTimerRepeatValue);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gPNOSlowScanMultiplier] Value = [%u] ",
		  pHddCtx->config->pno_slow_scan_multiplier);
#endif
#ifdef FEATURE_WLAN_TDLS
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [fEnableTDLSSupport] Value = [%u] ",
		  pHddCtx->config->fEnableTDLSSupport);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [fEnableTDLSImplicitTrigger] Value = [%u] ",
		  pHddCtx->config->fEnableTDLSImplicitTrigger);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [fTDLSExternalControl] Value = [%u] ",
		  pHddCtx->config->fTDLSExternalControl);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [fTDLSUapsdMask] Value = [%u] ",
		  pHddCtx->config->fTDLSUapsdMask);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [fEnableTDLSBufferSta] Value = [%u] ",
		  pHddCtx->config->fEnableTDLSBufferSta);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [fEnableTDLSWmmMode] Value = [%u] ",
		  pHddCtx->config->fEnableTDLSWmmMode);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [enable_tdls_scan] Value = [%u]",
		  pHddCtx->config->enable_tdls_scan);
#endif
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [InfraDirAcVo] Value = [%u] ",
		  pHddCtx->config->InfraDirAcVo);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [InfraNomMsduSizeAcVo] Value = [0x%x] ",
		  pHddCtx->config->InfraNomMsduSizeAcVo);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [InfraMeanDataRateAcVo] Value = [0x%x] ",
		  pHddCtx->config->InfraMeanDataRateAcVo);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [InfraMinPhyRateAcVo] Value = [0x%x] ",
		  pHddCtx->config->InfraMinPhyRateAcVo);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [InfraSbaAcVo] Value = [0x%x] ",
		  pHddCtx->config->InfraSbaAcVo);

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [InfraDirAcVi] Value = [%u] ",
		  pHddCtx->config->InfraDirAcVi);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [InfraNomMsduSizeAcVi] Value = [0x%x] ",
		  pHddCtx->config->InfraNomMsduSizeAcVi);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [InfraMeanDataRateAcVi] Value = [0x%x] ",
		  pHddCtx->config->InfraMeanDataRateAcVi);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [InfraMinPhyRateAcVi] Value = [0x%x] ",
		  pHddCtx->config->InfraMinPhyRateAcVi);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [InfraSbaAcVi] Value = [0x%x] ",
		  pHddCtx->config->InfraSbaAcVi);

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [InfraDirAcBe] Value = [%u] ",
		  pHddCtx->config->InfraDirAcBe);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [InfraNomMsduSizeAcBe] Value = [0x%x] ",
		  pHddCtx->config->InfraNomMsduSizeAcBe);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [InfraMeanDataRateAcBe] Value = [0x%x] ",
		  pHddCtx->config->InfraMeanDataRateAcBe);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [InfraMinPhyRateAcBe] Value = [0x%x] ",
		  pHddCtx->config->InfraMinPhyRateAcBe);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [InfraSbaAcBe] Value = [0x%x] ",
		  pHddCtx->config->InfraSbaAcBe);

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [InfraDirAcBk] Value = [%u] ",
		  pHddCtx->config->InfraDirAcBk);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [InfraNomMsduSizeAcBk] Value = [0x%x] ",
		  pHddCtx->config->InfraNomMsduSizeAcBk);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [InfraMeanDataRateAcBk] Value = [0x%x] ",
		  pHddCtx->config->InfraMeanDataRateAcBk);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [InfraMinPhyRateAcBk] Value = [0x%x] ",
		  pHddCtx->config->InfraMinPhyRateAcBk);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [InfraSbaAcBk] Value = [0x%x] ",
		  pHddCtx->config->InfraSbaAcBk);

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [DelayedTriggerFrmInt] Value = [%u] ",
		  pHddCtx->config->DelayedTriggerFrmInt);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [mcastBcastFilterSetting] Value = [%u] ",
		  pHddCtx->config->mcastBcastFilterSetting);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [fhostArpOffload] Value = [%u] ",
		  pHddCtx->config->fhostArpOffload);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [ssdp] Value = [%u] ", pHddCtx->config->ssdp);
	hdd_cfg_print_runtime_pm(pHddCtx);
#ifdef FEATURE_WLAN_RA_FILTERING
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [RArateLimitInterval] Value = [%u] ",
		  pHddCtx->config->RArateLimitInterval);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [IsRArateLimitEnabled] Value = [%u] ",
		  pHddCtx->config->IsRArateLimitEnabled);
#endif
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [fFTResourceReqSupported] Value = [%u] ",
		  pHddCtx->config->fFTResourceReqSupported);

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [nNeighborLookupRssiThreshold] Value = [%u] ",
		  pHddCtx->config->nNeighborLookupRssiThreshold);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [delay_before_vdev_stop] Value = [%u] ",
		  pHddCtx->config->delay_before_vdev_stop);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [nOpportunisticThresholdDiff] Value = [%u] ",
		  pHddCtx->config->nOpportunisticThresholdDiff);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [nRoamRescanRssiDiff] Value = [%u] ",
		  pHddCtx->config->nRoamRescanRssiDiff);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [nNeighborScanMinChanTime] Value = [%u] ",
		  pHddCtx->config->nNeighborScanMinChanTime);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [nNeighborScanMaxChanTime] Value = [%u] ",
		  pHddCtx->config->nNeighborScanMaxChanTime);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [nMaxNeighborRetries] Value = [%u] ",
		  pHddCtx->config->nMaxNeighborReqTries);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [nNeighborScanPeriod] Value = [%u] ",
		  pHddCtx->config->nNeighborScanPeriod);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [nNeighborScanResultsRefreshPeriod] Value = [%u] ",
		  pHddCtx->config->nNeighborResultsRefreshPeriod);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [nEmptyScanRefreshPeriod] Value = [%u] ",
		  pHddCtx->config->nEmptyScanRefreshPeriod);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [nRoamBmissFirstBcnt] Value = [%u] ",
		  pHddCtx->config->nRoamBmissFirstBcnt);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [nRoamBmissFinalBcnt] Value = [%u] ",
		  pHddCtx->config->nRoamBmissFinalBcnt);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [nRoamBeaconRssiWeight] Value = [%u] ",
		  pHddCtx->config->nRoamBeaconRssiWeight);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [allowDFSChannelRoam] Value = [%u] ",
		  pHddCtx->config->allowDFSChannelRoam);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [nhi_rssi_scan_max_count] Value = [%u] ",
		  pHddCtx->config->nhi_rssi_scan_max_count);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [nhi_rssi_scan_rssi_delta] Value = [%u] ",
		  pHddCtx->config->nhi_rssi_scan_rssi_delta);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [nhi_rssi_scan_delay] Value = [%u] ",
		  pHddCtx->config->nhi_rssi_scan_delay);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [nhi_rssi_scan_rssi_ub] Value = [%u] ",
		  pHddCtx->config->nhi_rssi_scan_rssi_ub);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [burstSizeDefinition] Value = [0x%x] ",
		  pHddCtx->config->burstSizeDefinition);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [tsInfoAckPolicy] Value = [0x%x] ",
		  pHddCtx->config->tsInfoAckPolicy);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [rfSettlingTimeUs] Value = [%u] ",
		  pHddCtx->config->rfSettlingTimeUs);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [bSingleTidRc] Value = [%u] ",
		  pHddCtx->config->bSingleTidRc);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gDynamicPSPollvalue] Value = [%u] ",
		  pHddCtx->config->dynamicPsPollValue);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gAddTSWhenACMIsOff] Value = [%u] ",
		  pHddCtx->config->AddTSWhenACMIsOff);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gValidateScanList] Value = [%u] ",
		  pHddCtx->config->fValidateScanList);

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gStaKeepAlivePeriod] Value = [%u] ",
		  pHddCtx->config->infraStaKeepAlivePeriod);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gApDataAvailPollInterVal] Value = [%u] ",
		  pHddCtx->config->apDataAvailPollPeriodInMs);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [BandCapability] Value = [%u] ",
		  pHddCtx->config->nBandCapability);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [teleBcnWakeupEnable] Value = [%u] ",
		  pHddCtx->config->teleBcnWakeupEn);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [transListenInterval] Value = [%u] ",
		  pHddCtx->config->nTeleBcnTransListenInterval);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [transLiNumIdleBeacons] Value = [%u] ",
		  pHddCtx->config->nTeleBcnTransLiNumIdleBeacons);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [maxListenInterval] Value = [%u] ",
		  pHddCtx->config->nTeleBcnMaxListenInterval);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [maxLiNumIdleBeacons] Value = [%u] ",
		  pHddCtx->config->nTeleBcnMaxLiNumIdleBeacons);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gApDataAvailPollInterVal] Value = [%u] ",
		  pHddCtx->config->apDataAvailPollPeriodInMs);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gEnableBypass11d] Value = [%u] ",
		  pHddCtx->config->enableBypass11d);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gEnableDFSChnlScan] Value = [%u] ",
		  pHddCtx->config->enableDFSChnlScan);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gEnableDFSPnoChnlScan] Value = [%u] ",
		  pHddCtx->config->enable_dfs_pno_chnl_scan);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gReportMaxLinkSpeed] Value = [%u] ",
		  pHddCtx->config->reportMaxLinkSpeed);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [thermalMitigationEnable] Value = [%u] ",
		  pHddCtx->config->thermalMitigationEnable);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gVhtChannelWidth] value = [%u]",
		  pHddCtx->config->vhtChannelWidth);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [enableFirstScan2GOnly] Value = [%u] ",
		  pHddCtx->config->enableFirstScan2GOnly);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [skipDfsChnlInP2pSearch] Value = [%u] ",
		  pHddCtx->config->skipDfsChnlInP2pSearch);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [ignoreDynamicDtimInP2pMode] Value = [%u] ",
		  pHddCtx->config->ignoreDynamicDtimInP2pMode);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [enableRxSTBC] Value = [%u] ",
		  pHddCtx->config->enableRxSTBC);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gEnableLpwrImgTransition] Value = [%u] ",
		  pHddCtx->config->enableLpwrImgTransition);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gEnableSSR] Value = [%u] ",
		  pHddCtx->config->enableSSR);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gEnableVhtFor24GHzBand] Value = [%u] ",
		  pHddCtx->config->enableVhtFor24GHzBand);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gEnableIbssHeartBeatOffload] Value = [%u] ",
		  pHddCtx->config->enableIbssHeartBeatOffload);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gAntennaDiversity] Value = [%u] ",
		  pHddCtx->config->antennaDiversity);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gGoLinkMonitorPeriod] Value = [%u]",
		  pHddCtx->config->goLinkMonitorPeriod);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gApLinkMonitorPeriod] Value = [%u]",
		  pHddCtx->config->apLinkMonitorPeriod);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gGoKeepAlivePeriod] Value = [%u]",
		  pHddCtx->config->goKeepAlivePeriod);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gApKeepAlivePeriod]Value = [%u]",
		  pHddCtx->config->apKeepAlivePeriod);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gAmsduSupportInAMPDU] Value = [%u] ",
		  pHddCtx->config->isAmsduSupportInAMPDU);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [nSelect5GHzMargin] Value = [%u] ",
		  pHddCtx->config->nSelect5GHzMargin);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gCoalesingInIBSS] Value = [%u] ",
		  pHddCtx->config->isCoalesingInIBSSAllowed);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gIbssATIMWinSize] Value = [%u] ",
		  pHddCtx->config->ibssATIMWinSize);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gIbssIsPowerSaveAllowed] Value = [%u] ",
		  pHddCtx->config->isIbssPowerSaveAllowed);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gIbssIsPowerCollapseAllowed] Value = [%u] ",
		  pHddCtx->config->isIbssPowerCollapseAllowed);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gIbssAwakeOnTxRx] Value = [%u] ",
		  pHddCtx->config->isIbssAwakeOnTxRx);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gIbssInactivityTime] Value = [%u] ",
		  pHddCtx->config->ibssInactivityCount);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gIbssTxSpEndInactivityTime] Value = [%u] ",
		  pHddCtx->config->ibssTxSpEndInactivityTime);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gIbssPsWarmupTime] Value = [%u] ",
		  pHddCtx->config->ibssPsWarmupTime);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gIbssPs1RxChainInAtim] Value = [%u] ",
		  pHddCtx->config->ibssPs1RxChainInAtimEnable);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [fDfsPhyerrFilterOffload] Value = [%u] ",
		  pHddCtx->config->fDfsPhyerrFilterOffload);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gIgnorePeerErpInfo] Value = [%u] ",
		  pHddCtx->config->ignore_peer_erp_info);
#ifdef IPA_OFFLOAD
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gIPAConfig] Value = [0x%x] ",
		  pHddCtx->config->IpaConfig);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gIPADescSize] Value = [%u] ",
		  pHddCtx->config->IpaDescSize);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [IpaHighBandwidthMbpsg] Value = [%u] ",
		  pHddCtx->config->IpaHighBandwidthMbps);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [IpaMediumBandwidthMbps] Value = [%u] ",
		  pHddCtx->config->IpaMediumBandwidthMbps);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [IpaLowBandwidthMbps] Value = [%u] ",
		  pHddCtx->config->IpaLowBandwidthMbps);
#endif
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gEnableOverLapCh] Value = [%u] ",
		  pHddCtx->config->gEnableOverLapCh);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gMaxOffloadPeers] Value = [%u] ",
		  pHddCtx->config->apMaxOffloadPeers);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gMaxOffloadReorderBuffs] value = [%u] ",
		  pHddCtx->config->apMaxOffloadReorderBuffs);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gAllowDFSChannelRoam] Value = [%u] ",
		  pHddCtx->config->allowDFSChannelRoam);
	hddLog(QDF_TRACE_LEVEL_INFO_HIGH,
	       "Name = [gMaxConcurrentActiveSessions] Value = [%u] ",
	       pHddCtx->config->gMaxConcurrentActiveSessions);

#ifdef MSM_PLATFORM
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gBusBandwidthHighThreshold] Value = [%u] ",
		  pHddCtx->config->busBandwidthHighThreshold);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gBusBandwidthMediumThreshold] Value = [%u] ",
		  pHddCtx->config->busBandwidthMediumThreshold);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gBusBandwidthLowThreshold] Value = [%u] ",
		  pHddCtx->config->busBandwidthLowThreshold);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gbusBandwidthComputeInterval] Value = [%u] ",
		  pHddCtx->config->busBandwidthComputeInterval);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [%s] Value = [%u] ",
		  CFG_ENABLE_TCP_DELACK,
		  pHddCtx->config->enable_tcp_delack);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gTcpDelAckThresholdHigh] Value = [%u] ",
		  pHddCtx->config->tcpDelackThresholdHigh);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gTcpDelAckThresholdLow] Value = [%u] ",
		  pHddCtx->config->tcpDelackThresholdLow);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [%s] Value = [%u] ",
		  CFG_TCP_DELACK_TIMER_COUNT,
		  pHddCtx->config->tcp_delack_timer_count);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [%s] Value = [%u] ",
		  CFG_TCP_TX_HIGH_TPUT_THRESHOLD_NAME,
		  pHddCtx->config->tcp_tx_high_tput_thres);

#endif

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gIgnoreCAC] Value = [%u] ",
		  pHddCtx->config->ignoreCAC);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gSapPreferredChanLocation] Value = [%u] ",
		  pHddCtx->config->gSapPreferredChanLocation);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gDisableDfsJapanW53] Value = [%u] ",
		  pHddCtx->config->gDisableDfsJapanW53);
#ifdef FEATURE_GREEN_AP
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gEnableGreenAp] Value = [%u] ",
		  pHddCtx->config->enableGreenAP);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gEenableEGAP] Value = [%u] ",
		  pHddCtx->config->enable_egap);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gEGAPInactTime] Value = [%u] ",
		  pHddCtx->config->egap_inact_time);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gEGAPWaitTime] Value = [%u] ",
		  pHddCtx->config->egap_wait_time);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gEGAPFeatures] Value = [%u] ",
		  pHddCtx->config->egap_feature_flag);
#endif
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [isRoamOffloadEnabled] Value = [%u]",
		  pHddCtx->config->isRoamOffloadEnabled);
#endif
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gEnableSifsBurst] Value = [%u]",
		  pHddCtx->config->enableSifsBurst);

#ifdef WLAN_FEATURE_LPSS
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gEnableLpassSupport] Value = [%u] ",
		  pHddCtx->config->enable_lpass_support);
#endif

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gEnableSelfRecovery] Value = [%u]",
		  pHddCtx->config->enableSelfRecovery);

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gEnableSapSuspend] Value = [%u]",
		  pHddCtx->config->enableSapSuspend);

#ifdef WLAN_FEATURE_EXTWOW_SUPPORT
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gExtWoWgotoSuspend] Value = [%u]",
		  pHddCtx->config->extWowGotoSuspend);

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gExtWowApp1WakeupPinNumber] Value = [%u]",
		  pHddCtx->config->extWowApp1WakeupPinNumber);

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gExtWowApp2WakeupPinNumber] Value = [%u]",
		  pHddCtx->config->extWowApp2WakeupPinNumber);

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gExtWoWApp2KAInitPingInterval] Value = [%u]",
		  pHddCtx->config->extWowApp2KAInitPingInterval);

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gExtWoWApp2KAMinPingInterval] Value = [%u]",
		  pHddCtx->config->extWowApp2KAMinPingInterval);

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gExtWoWApp2KAMaxPingInterval] Value = [%u]",
		  pHddCtx->config->extWowApp2KAMaxPingInterval);

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gExtWoWApp2KAIncPingInterval] Value = [%u]",
		  pHddCtx->config->extWowApp2KAIncPingInterval);

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gExtWoWApp2TcpSrcPort] Value = [%u]",
		  pHddCtx->config->extWowApp2TcpSrcPort);

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gExtWoWApp2TcpDstPort] Value = [%u]",
		  pHddCtx->config->extWowApp2TcpDstPort);

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gExtWoWApp2TcpTxTimeout] Value = [%u]",
		  pHddCtx->config->extWowApp2TcpTxTimeout);

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gExtWoWApp2TcpRxTimeout] Value = [%u]",
		  pHddCtx->config->extWowApp2TcpRxTimeout);
#endif

#ifdef DHCP_SERVER_OFFLOAD
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gDHCPServerOffloadEnable] Value = [%u]",
		  pHddCtx->config->enableDHCPServerOffload);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gDHCPMaxNumClients] Value = [%u]",
		  pHddCtx->config->dhcpMaxNumClients);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gDHCPServerIP] Value = [%s]",
		  pHddCtx->config->dhcpServerIP);
#endif

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gEnableDumpCollect] Value = [%u]",
			pHddCtx->config->is_ramdump_enabled);

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [gP2PListenDeferInterval] Value = [%u]",
		  pHddCtx->config->p2p_listen_defer_interval);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
		  "Name = [is_ps_enabled] value = [%d]",
		  pHddCtx->config->is_ps_enabled);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
		  "Name = [tso_enable] value = [%d]",
		  pHddCtx->config->tso_enable);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
		  "Name = [LROEnable] value = [%d]",
		  pHddCtx->config->lro_enable);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
		  "Name = [active_mode_offload] value = [%d]",
		  pHddCtx->config->active_mode_offload);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
		  "Name = [gfine_time_meas_cap] value = [%u]",
		  pHddCtx->config->fine_time_meas_cap);
#ifdef WLAN_FEATURE_FASTPATH
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [fastpath_enable] Value = [%u]",
		  pHddCtx->config->fastpath_enable);
#endif
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
		  "Name = [max_scan_count] value = [%d]",
		  pHddCtx->config->max_scan_count);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
		  "Name = [%s] value = [%d]",
		  CFG_RX_MODE_NAME, pHddCtx->config->rx_mode);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Name = [%s] Value = [%u]",
		  CFG_CE_CLASSIFY_ENABLE_NAME,
		  pHddCtx->config->ce_classify_enabled);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
		  "Name = [%s] value = [%u]",
		  CFG_DUAL_MAC_FEATURE_DISABLE,
		  pHddCtx->config->dual_mac_feature_disable);
#ifdef FEATURE_WLAN_SCAN_PNO
	hddLog(LOGE, "Name = [%s] Value = [%u]",
			CFG_PNO_CHANNEL_PREDICTION_NAME,
			pHddCtx->config->pno_channel_prediction);
	hddLog(LOGE, "Name = [%s] Value = [%u]",
			CFG_TOP_K_NUM_OF_CHANNELS_NAME,
			pHddCtx->config->top_k_num_of_channels);
	hddLog(LOGE, "Name = [%s] Value = [%u]",
			CFG_STATIONARY_THRESHOLD_NAME,
			pHddCtx->config->stationary_thresh);
	hddLog(LOGE, "Name = [%s] Value = [%u]",
			CFG_CHANNEL_PREDICTION_FULL_SCAN_MS_NAME,
			pHddCtx->config->channel_prediction_full_scan);
	hddLog(LOGE, "Name = [%s] Value = [%u]",
			CFG_ADAPTIVE_PNOSCAN_DWELL_MODE_NAME,
			pHddCtx->config->pnoscan_adaptive_dwell_mode);
#endif
	hddLog(LOGE, "Name = [%s] Value = [%d]",
		  CFG_EARLY_STOP_SCAN_ENABLE,
		  pHddCtx->config->early_stop_scan_enable);
	hddLog(LOGE, "Name = [%s] Value = [%d]",
		CFG_EARLY_STOP_SCAN_MIN_THRESHOLD,
		pHddCtx->config->early_stop_scan_min_threshold);
	hddLog(LOGE, "Name = [%s] Value = [%d]",
		CFG_EARLY_STOP_SCAN_MAX_THRESHOLD,
		pHddCtx->config->early_stop_scan_max_threshold);
	hddLog(LOGE, "Name = [%s] Value = [%d]",
		CFG_FIRST_SCAN_BUCKET_THRESHOLD_NAME,
		pHddCtx->config->first_scan_bucket_threshold);
	hddLog(LOGE, "Name = [%s] Value = [%u]",
		CFG_HT_MPDU_DENSITY_NAME,
		pHddCtx->config->ht_mpdu_density);


#ifdef FEATURE_LFR_SUBNET_DETECTION
	hddLog(LOGE, "Name = [%s] Value = [%d]",
		CFG_ENABLE_LFR_SUBNET_DETECTION,
		pHddCtx->config->enable_lfr_subnet_detection);
#endif
	hdd_info("Name = [%s] Value = [%u]",
		CFG_ROAM_DENSE_TRAFFIC_THRESHOLD,
		pHddCtx->config->roam_dense_traffic_thresh);
	hdd_info("Name = [%s] Value = [%u]",
		CFG_ROAM_DENSE_RSSI_THRE_OFFSET,
		pHddCtx->config->roam_dense_rssi_thresh_offset);
	hdd_info("Name = [%s] Value = [%u]",
		CFG_IGNORE_PEER_HT_MODE_NAME,
		pHddCtx->config->ignore_peer_ht_opmode);
	hdd_info("Name = [%s] Value = [%u]",
		CFG_ENABLE_FATAL_EVENT_TRIGGER,
		pHddCtx->config->enable_fatal_event);
	hdd_info("Name = [%s] Value = [%u]",
		CFG_ROAM_DENSE_MIN_APS,
		pHddCtx->config->roam_dense_min_aps);
	hdd_info("Name = [%s] Value = [%u]",
		CFG_MIN_REST_TIME_NAME,
		pHddCtx->config->min_rest_time_conc);
	hdd_info("Name = [%s] Value = [%u]",
		CFG_IDLE_TIME_NAME,
		pHddCtx->config->idle_time_conc);
	hdd_info("Name = [%s] Value = [%u]",
		CFG_INTERFACE_CHANGE_WAIT_NAME,
		pHddCtx->config->iface_change_wait_time);

	hdd_info("Name = [%s] Value = [%u]",
		CFG_ENABLE_EDCA_INI_NAME,
		pHddCtx->config->enable_edca_params);

	hdd_info("Name = [%s] Value = [%u]",
		CFG_EDCA_VO_CWMIN_VALUE_NAME,
		pHddCtx->config->edca_vo_cwmin);
	hdd_info("Name = [%s] Value = [%u]",
		CFG_EDCA_VI_CWMIN_VALUE_NAME,
		pHddCtx->config->edca_vi_cwmin);
	hdd_info("Name = [%s] Value = [%u]",
		CFG_EDCA_BK_CWMIN_VALUE_NAME,
		pHddCtx->config->edca_bk_cwmin);
	hdd_info("Name = [%s] Value = [%u]",
		CFG_EDCA_BE_CWMIN_VALUE_NAME,
		pHddCtx->config->edca_be_cwmin);

	hdd_info("Name = [%s] Value = [%u]",
		CFG_EDCA_VO_CWMAX_VALUE_NAME,
		pHddCtx->config->edca_vo_cwmax);
	hdd_info("Name = [%s] Value = [%u]",
		CFG_EDCA_VI_CWMAX_VALUE_NAME,
		pHddCtx->config->edca_vi_cwmax);
	hdd_info("Name = [%s] Value = [%u]",
		CFG_EDCA_BK_CWMAX_VALUE_NAME,
		pHddCtx->config->edca_bk_cwmax);
	hdd_info("Name = [%s] Value = [%u]",
		CFG_EDCA_BE_CWMAX_VALUE_NAME,
		pHddCtx->config->edca_be_cwmax);

	hdd_info("Name = [%s] Value = [%u]",
		CFG_EDCA_VO_AIFS_VALUE_NAME,
		pHddCtx->config->edca_vo_aifs);
	hdd_info("Name = [%s] Value = [%u]",
		CFG_EDCA_VI_AIFS_VALUE_NAME,
		pHddCtx->config->edca_vi_aifs);
	hdd_info("Name = [%s] Value = [%u]",
		CFG_EDCA_BK_AIFS_VALUE_NAME,
		pHddCtx->config->edca_bk_aifs);
	hdd_info("Name = [%s] Value = [%u]",
		CFG_EDCA_BE_AIFS_VALUE_NAME,
		pHddCtx->config->edca_be_aifs);
	hdd_info("Name = [%s] Value = [%u]",
		CFG_ENABLE_DP_TRACE,
		pHddCtx->config->enable_dp_trace);
	hdd_info("Name = [%s] Value = [%u]",
		CFG_ADAPTIVE_SCAN_DWELL_MODE_NAME,
		pHddCtx->config->scan_adaptive_dwell_mode);
	hdd_info("Name = [%s] Value = [%u]",
		CFG_ADAPTIVE_ROAMSCAN_DWELL_MODE_NAME,
		pHddCtx->config->roamscan_adaptive_dwell_mode);
	hdd_info("Name = [%s] Value = [%u]",
		CFG_ADAPTIVE_EXTSCAN_DWELL_MODE_NAME,
		pHddCtx->config->extscan_adaptive_dwell_mode);
	hdd_info("Name = [%s] Value = [%u]",
		CFG_ADAPTIVE_DWELL_MODE_ENABLED_NAME,
		pHddCtx->config->adaptive_dwell_mode_enabled);
	hdd_info("Name = [%s] Value = [%u]",
		CFG_GLOBAL_ADAPTIVE_DWELL_MODE_NAME,
		pHddCtx->config->global_adapt_dwelltime_mode);
	hdd_info("Name = [%s] Value = [%u]",
		CFG_ADAPT_DWELL_LPF_WEIGHT_NAME,
		pHddCtx->config->adapt_dwell_lpf_weight);
	hdd_info("Name = [%s] Value = [%u]",
		CFG_ADAPT_DWELL_PASMON_INTVAL_NAME,
		pHddCtx->config->adapt_dwell_passive_mon_intval);
	hdd_info("Name = [%s] Value = [%u]",
		CFG_ADAPT_DWELL_WIFI_THRESH_NAME,
		pHddCtx->config->adapt_dwell_wifi_act_threshold);
	hdd_info("Name = [%s] value = [%u]",
		 CFG_SUB_20_CHANNEL_WIDTH_NAME,
		 pHddCtx->config->enable_sub_20_channel_width);
	hdd_info("Name = [%s] Value = [%u]",
		 CFG_TGT_GTX_USR_CFG_NAME,
		 pHddCtx->config->tgt_gtx_usr_cfg);
	hdd_ndp_print_ini_config(pHddCtx);
	hdd_info("Name = [%s] Value = [%s]",
		CFG_RM_CAPABILITY_NAME,
		pHddCtx->config->rm_capability);
	hdd_info("Name = [%s] Value = [%d]",
		CFG_BPF_PACKET_FILTER_OFFLOAD,
		pHddCtx->config->bpf_packet_filter_enable);
	hdd_info("Name = [%s] Value = [%d]",
		CFG_FILTER_MULTICAST_REPLAY_NAME,
		pHddCtx->config->multicast_replay_filter);
}


/**
 * hdd_update_mac_config() - update MAC address from cfg file
 * @pHddCtx: the pointer to hdd context
 *
 * It overwrites the MAC address if config file exist.
 *
 * Return: QDF_STATUS_SUCCESS if the MAC address is found from cfg file
 *      and overwritten, otherwise QDF_STATUS_E_INVAL
 */
QDF_STATUS hdd_update_mac_config(hdd_context_t *pHddCtx)
{
	int status, i = 0;
	const struct firmware *fw = NULL;
	char *line, *buffer = NULL;
	char *temp = NULL;
	char *name, *value;
	tCfgIniEntry macTable[QDF_MAX_CONCURRENCY_PERSONA];
	tSirMacAddr customMacAddr;

	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	memset(macTable, 0, sizeof(macTable));
	status = request_firmware(&fw, WLAN_MAC_FILE, pHddCtx->parent_dev);

	if (status) {
		hddLog(QDF_TRACE_LEVEL_WARN, "%s: request_firmware failed %d",
		       __func__, status);
		qdf_status = QDF_STATUS_E_FAILURE;
		return qdf_status;
	}
	if (!fw || !fw->data || !fw->size) {
		hddLog(QDF_TRACE_LEVEL_FATAL, "%s: invalid firmware", __func__);
		qdf_status = QDF_STATUS_E_INVAL;
		goto config_exit;
	}

	hdd_debug("wlan_mac.bin size %zu", fw->size);

	temp = qdf_mem_malloc(fw->size);

	if (temp == NULL) {
		hdd_err("fail to alloc memory");
		qdf_status = QDF_STATUS_E_NOMEM;
		goto config_exit;
	}
	buffer = temp;
	qdf_mem_copy(buffer, fw->data, fw->size);

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
		hddLog(QDF_TRACE_LEVEL_INFO, "%s: %d Mac addresses provided",
		       __func__, i);
	} else {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       "%s: invalid number of Mac address provided, nMac = %d",
		       __func__, i);
		qdf_status = QDF_STATUS_E_INVAL;
		goto config_exit;
	}

	update_mac_from_string(pHddCtx, &macTable[0], i);

	qdf_mem_copy(&customMacAddr,
		     &pHddCtx->config->intfMacAddr[0].bytes[0],
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
static void hdd_override_all_ps(hdd_context_t *hdd_ctx)
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
void hdd_set_rx_mode_value(hdd_context_t *hdd_ctx)
{
	if (hdd_ctx->config->rx_mode & CFG_ENABLE_RX_THREAD &&
		 hdd_ctx->config->rx_mode & CFG_ENABLE_RPS) {
		hdd_err("rx_mode wrong configuration. Make it default");
		hdd_ctx->config->rx_mode = CFG_RX_MODE_DEFAULT;
	}

	if (hdd_ctx->config->rx_mode & CFG_ENABLE_RX_THREAD)
		hdd_ctx->enableRxThread = true;

	if (hdd_ctx->config->rx_mode & CFG_ENABLE_RPS)
		hdd_ctx->rps = true;

	if (hdd_ctx->config->rx_mode & CFG_ENABLE_NAPI)
		hdd_ctx->napi_enable = true;
}

/**
 * hdd_parse_config_ini() - parse the ini configuration file
 * @pHddCtx: the pointer to hdd context
 *
 * This function reads the qcom_cfg.ini file and
 * parses each 'Name=Value' pair in the ini file
 *
 * Return: QDF_STATUS_SUCCESS if the qcom_cfg.ini is correctly read,
 *		otherwise QDF_STATUS_E_INVAL
 */
QDF_STATUS hdd_parse_config_ini(hdd_context_t *pHddCtx)
{
	int status, i = 0;
	/** Pointer for firmware image data */
	const struct firmware *fw = NULL;
	char *buffer, *line, *pTemp = NULL;
	size_t size;
	char *name, *value;
	/* cfgIniTable is static to avoid excess stack usage */
	static tCfgIniEntry cfgIniTable[MAX_CFG_INI_ITEMS];
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	memset(cfgIniTable, 0, sizeof(cfgIniTable));

	status = request_firmware(&fw, WLAN_INI_FILE, pHddCtx->parent_dev);

	if (status) {
		hddLog(QDF_TRACE_LEVEL_FATAL, "%s: request_firmware failed %d",
		       __func__, status);
		qdf_status = QDF_STATUS_E_FAILURE;
		goto config_exit;
	}
	if (!fw || !fw->data || !fw->size) {
		hddLog(QDF_TRACE_LEVEL_FATAL, "%s: %s download failed",
		       __func__, WLAN_INI_FILE);
		qdf_status = QDF_STATUS_E_FAILURE;
		goto config_exit;
	}

	hddLog(LOG1, "%s: qcom_cfg.ini Size %zu", __func__, fw->size);

	buffer = (char *)qdf_mem_malloc(fw->size);

	if (NULL == buffer) {
		hddLog(QDF_TRACE_LEVEL_FATAL, FL("qdf_mem_malloc failure"));
		release_firmware(fw);
		return QDF_STATUS_E_NOMEM;
	}
	pTemp = buffer;

	qdf_mem_copy((void *)buffer, (void *)fw->data, fw->size);
	size = fw->size;

	while (buffer != NULL) {
		line = get_next_line(buffer);
		buffer = i_trim(buffer);

		hddLog(LOG1, "%s: item", buffer);

		if (strlen((char *)buffer) == 0 || *buffer == '#') {
			buffer = line;
			continue;
		} else if (strncmp(buffer, "END", 3) == 0) {
			break;
		} else {
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
						while (!i_isspace(*buffer)
						       && *buffer != '\0')
							buffer++;
						*buffer = '\0';
						cfgIniTable[i].name = name;
						cfgIniTable[i++].value = value;
						if (i >= MAX_CFG_INI_ITEMS) {
							hddLog(LOGE,
							       "%s: Number of items in %s > %d",
							       __func__,
							       WLAN_INI_FILE,
							       MAX_CFG_INI_ITEMS);
							break;
						}
					}
				}
			}
		}
		buffer = line;
	}

	/* Loop through the registry table and apply all these configs */
	qdf_status = hdd_apply_cfg_ini(pHddCtx, cfgIniTable, i);
	hdd_set_rx_mode_value(pHddCtx);
	if (QDF_GLOBAL_MONITOR_MODE == cds_get_conparam())
		hdd_override_all_ps(pHddCtx);

config_exit:
	release_firmware(fw);
	qdf_mem_free(pTemp);
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
eCsrPhyMode hdd_cfg_xlate_to_csr_phy_mode(eHddDot11Mode dot11Mode)
{
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
	}

}

/**
 * hdd_set_idle_ps_config() - set idle power save configuration
 * @pHddCtx: the pointer to hdd context
 * @val: the value to configure
 *
 * Return: QDF_STATUS_SUCCESS if command set correctly,
 *		otherwise the QDF_STATUS return from SME layer
 */
QDF_STATUS hdd_set_idle_ps_config(hdd_context_t *pHddCtx, uint32_t val)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	hddLog(LOG1, "hdd_set_idle_ps_config: Enter Val %d", val);

	status = sme_set_idle_powersave_config(pHddCtx->pcds_context,
			pHddCtx->hHal, val);
	if (QDF_STATUS_SUCCESS != status)
		hddLog(LOGE, "Fail to Set Idle PS Config val %d", val);
	return status;
}

/**
 * hdd_set_fine_time_meas_cap() - set fine timing measurement capability
 * @hdd_ctx: HDD context
 * @sme_config: pointer to SME config
 *
 * This function is used to pass fine timing measurement capability coming
 * from INI to SME. This function make sure that configure INI is supported
 * by the device. Use bit mask to mask out the unsupported capabilities.
 *
 * Return: None
 */
static void hdd_set_fine_time_meas_cap(hdd_context_t *hdd_ctx,
				       tSmeConfigParams *sme_config)
{
	struct hdd_config *config = hdd_ctx->config;
	uint32_t capability = config->fine_time_meas_cap;

	/* Make sure only supported capabilities are enabled in INI */
	capability &= CFG_FINE_TIME_MEAS_CAPABILITY_MAX;
	sme_config->csrConfig.fine_time_meas_cap = capability;

	hddLog(LOG1, FL("fine time meas capability - INI: %04x Enabled: %04x"),
		config->fine_time_meas_cap,
		sme_config->csrConfig.fine_time_meas_cap);

	return;
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
			       uint8_t *len, uint8_t array_max_len, bool to_hex)
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
		 * Ignore it. */
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
QDF_STATUS hdd_hex_string_to_u8_array(char *str, uint8_t *hex_array,
				      uint8_t *len, uint8_t array_max_len)
{
	return hdd_convert_string_to_array(str, hex_array, len,
					   array_max_len, true);
}

/**
 * hdd_string_to_u8_array() - used to convert decimal string into u8 array
 * @str: Decimal string
 * @hex_array: Array where converted value is stored
 * @len: Length of the populated array
 * @array_max_len: Maximum length of the array
 *
 * This API is called to convert decimal string (each byte separated by
 * a comma) into an u8 array
 *
 * Return: QDF_STATUS
 */

QDF_STATUS hdd_string_to_u8_array(char *str, uint8_t *array,
				  uint8_t *len, uint8_t array_max_len)
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

	hdd_err("str %p intArray %p intArrayMaxLen %d",
		s, int_array, int_array_max_len);

	*len = 0;

	while ((s != NULL) && (*len < int_array_max_len)) {
		/*
		 * Increment length only if sscanf successfully extracted one
		 * element. Any other return value means error. Ignore it.
		 */
		if (sscanf(s, "%x", &val) == 1) {
			int_array[*len] = (uint16_t) val;
			hdd_debug("s %p val %x intArray[%d]=0x%x",
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
 * hdd_update_ht_cap_in_cfg() - to update HT cap in global CFG
 * @pHddCtx: pointer to hdd context
 *
 * This API will update the HT config in CFG after taking intersection
 * of INI and firmware capabilities provided reading CFG
 *
 * Return: true or false
 */
static bool hdd_update_ht_cap_in_cfg(hdd_context_t *pHddCtx)
{
	bool fStatus = true;
	uint32_t val;
	uint16_t val16;
	struct hdd_config *pConfig = pHddCtx->config;
	tSirMacHTCapabilityInfo *phtCapInfo;

	if (sme_cfg_get_int(pHddCtx->hHal, WNI_CFG_HT_CAP_INFO,
			    &val) ==
			QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hdd_err("Could not pass on WNI_CFG_HT_CAP_INFO to CFG");
	}
	if (pConfig->ShortGI20MhzEnable)
		val |= HT_CAPS_SHORT_GI20;
	else
		val &= ~(HT_CAPS_SHORT_GI20);

	if (pConfig->ShortGI40MhzEnable)
		val |= HT_CAPS_SHORT_GI40;
	else
		val &= ~(HT_CAPS_SHORT_GI40);

	if (sme_cfg_set_int(pHddCtx->hHal, WNI_CFG_HT_CAP_INFO,
			  val) ==
			QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hdd_err("Could not pass on WNI_CFG_HT_CAP_INFO to CFG");
	}
	sme_cfg_get_int(pHddCtx->hHal, WNI_CFG_HT_CAP_INFO, &val);
	val16 = (uint16_t) val;
	phtCapInfo = (tSirMacHTCapabilityInfo *) &val16;
	phtCapInfo->rxSTBC = pConfig->enableRxSTBC;
	phtCapInfo->advCodingCap = pConfig->enableRxLDPC;
	val = val16;
	if (sme_cfg_set_int(pHddCtx->hHal, WNI_CFG_HT_CAP_INFO, val)
			== QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE, "Could not pass on WNI_CFG_HT_CAP_INFO to CFG");
	}

	return fStatus;
}

/**
 * hdd_update_vht_cap_in_cfg() - to update VHT cap in global CFG
 * @pHddctx: pointer to hdd context
 *
 * This API will update the VHT config in CFG after taking intersection
 * of INI and firmware capabilities provided reading CFG
 *
 * Return: true or false
 */
static bool hdd_update_vht_cap_in_cfg(hdd_context_t *pHddCtx)
{
	bool fStatus = true;
	uint32_t val;
	struct hdd_config *pConfig = pHddCtx->config;

	if (sme_cfg_set_int(pHddCtx->hHal, WNI_CFG_VHT_ENABLE_TXBF_20MHZ,
			    pConfig->enableTxBFin20MHz) ==
			QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not set value for WNI_CFG_VHT_ENABLE_TXBF_20MHZ");
	}
	/* Based on cfg.ini, update the Basic MCS set, RX/TX MCS map
	 * in the cfg.dat. Valid values are 0(MCS0-7), 1(MCS0-8), 2(MCS0-9)
	 * we update only the least significant 2 bits in the
	 * corresponding fields.
	 */
	if ((pConfig->dot11Mode == eHDD_DOT11_MODE_AUTO) ||
	    (pConfig->dot11Mode == eHDD_DOT11_MODE_11ac_ONLY) ||
	    (pConfig->dot11Mode == eHDD_DOT11_MODE_11ac)) {
		/* Currently shortGI40Mhz is used for shortGI80Mhz */
		if (sme_cfg_set_int
			    (pHddCtx->hHal, WNI_CFG_VHT_SHORT_GI_80MHZ,
			    pConfig->ShortGI40MhzEnable) ==
			    QDF_STATUS_E_FAILURE) {
			fStatus = false;
			hddLog(LOGE,
			       "Could not pass WNI_VHT_SHORT_GI_80MHZ to CFG");
		}
		/* Hardware is capable of doing
		 * 128K AMPDU in 11AC mode */
		if (sme_cfg_set_int(pHddCtx->hHal,
			     WNI_CFG_VHT_AMPDU_LEN_EXPONENT,
			     pConfig->fVhtAmpduLenExponent) ==
			    QDF_STATUS_E_FAILURE) {
			fStatus = false;
			hddLog(LOGE,
			       "Could not pass on WNI_CFG_VHT_AMPDU_LEN_EXPONENT to CFG");
		}
		/* Change MU Bformee only when TxBF is enabled */
		if (pConfig->enableTxBF) {
			sme_cfg_get_int(pHddCtx->hHal,
				WNI_CFG_VHT_MU_BEAMFORMEE_CAP, &val);

			if (val != pConfig->enableMuBformee) {
				if (sme_cfg_set_int(pHddCtx->hHal,
					    WNI_CFG_VHT_MU_BEAMFORMEE_CAP,
					    pConfig->enableMuBformee
					    ) == QDF_STATUS_E_FAILURE) {
					fStatus = false;
					hddLog(LOGE,
						"Could not pass on WNI_CFG_VHT_MU_BEAMFORMEE_CAP to CFG");
				}
			}
		}
		if (sme_cfg_set_int(pHddCtx->hHal, WNI_CFG_VHT_MAX_MPDU_LENGTH,
			    pConfig->vhtMpduLen) == QDF_STATUS_E_FAILURE) {
			fStatus = false;
			hddLog(LOGE,
			       "Could not pass on WNI_CFG_VHT_MAX_MPDU_LENGTH to CFG");
		}

		if (pConfig->enable2x2 && pConfig->enable_su_tx_bformer) {
			if (sme_cfg_set_int(pHddCtx->hHal,
					WNI_CFG_VHT_SU_BEAMFORMER_CAP,
					pConfig->enable_su_tx_bformer) ==
				QDF_STATUS_E_FAILURE) {
				fStatus = false;
				hdd_err("set SU_BEAMFORMER_CAP to CFG failed");
			}
			if (sme_cfg_set_int(pHddCtx->hHal,
					WNI_CFG_VHT_NUM_SOUNDING_DIMENSIONS,
					NUM_OF_SOUNDING_DIMENSIONS) ==
				QDF_STATUS_E_FAILURE) {
				fStatus = false;
				hdd_err("failed to set NUM_OF_SOUNDING_DIM");
			}
		}
	}

	if (sme_cfg_set_int(pHddCtx->hHal, WNI_CFG_VHT_RXSTBC,
			    pConfig->enableRxSTBC) == QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE, "Could not pass on WNI_CFG_VHT_RXSTBC to CFG");
	}

	if (sme_cfg_set_int(pHddCtx->hHal, WNI_CFG_VHT_TXSTBC,
			    pConfig->enableTxSTBC) == QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE, "Could not pass on WNI_CFG_VHT_TXSTBC to CFG");
	}

	if (sme_cfg_set_int(pHddCtx->hHal, WNI_CFG_VHT_LDPC_CODING_CAP,
			    pConfig->enableRxLDPC) == QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_VHT_LDPC_CODING_CAP to CFG");
	}

	if (sme_cfg_set_int(pHddCtx->hHal,
		WNI_CFG_VHT_CSN_BEAMFORMEE_ANT_SUPPORTED,
		pConfig->txBFCsnValue) ==
			QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_VHT_CSN_BEAMFORMEE_ANT_SUPPORTED to CFG");
	}
	return fStatus;

}

/**
 * hdd_update_config_dat() - scan the string and convery to u8 array
 * @str: the pointer to the string
 * @intArray: the pointer of buffer to store the u8 value
 * @len: size of the buffer
 *
 * Return: QDF_STATUS_SUCCESS if the configuration could be updated corectly,
 *		otherwise QDF_STATUS_E_INVAL
 */
bool hdd_update_config_dat(hdd_context_t *pHddCtx)
{
	bool fStatus = true;
	uint32_t val;
	struct hdd_config *pConfig = pHddCtx->config;

	/*
	 * During the initialization both 2G and 5G capabilities should be same.
	 * So read 5G HT capablity and update 2G and 5G capablities.
	 */
	if (!hdd_update_ht_cap_in_cfg(pHddCtx)) {
		fStatus = false;
		hddLog(LOGE, "Could not set HT CAP in cfg");
	}

	if (!hdd_update_vht_cap_in_cfg(pHddCtx)) {
		fStatus = false;
		hddLog(LOGE, "Could not set VHT CAP in cfg");
	}

	if (sme_cfg_set_int(pHddCtx->hHal, WNI_CFG_FIXED_RATE, pConfig->TxRate)
			    == QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE, "Could not pass on WNI_CFG_FIXED_RATE to CFG");
	}

	if (sme_cfg_set_int(pHddCtx->hHal, WNI_CFG_MAX_RX_AMPDU_FACTOR,
			    pConfig->MaxRxAmpduFactor) ==
			QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_HT_AMPDU_PARAMS_MAX_RX_AMPDU_FACTOR to CFG");
	}

	if (sme_cfg_set_int(pHddCtx->hHal, WNI_CFG_MPDU_DENSITY,
			    pConfig->ht_mpdu_density) ==
			QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_MPDU_DENSITY to CFG");
	}

	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_SHORT_PREAMBLE,
		     pConfig->fIsShortPreamble) == QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
			"Could not pass on WNI_CFG_SHORT_PREAMBLE to CFG");
	}

	if (sme_cfg_set_int(pHddCtx->hHal,
				WNI_CFG_PASSIVE_MINIMUM_CHANNEL_TIME,
				pConfig->nPassiveMinChnTime)
				== QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_PASSIVE_MINIMUM_CHANNEL_TIME"
		       " to CFG");
	}

	if (sme_cfg_set_int(pHddCtx->hHal,
				WNI_CFG_PASSIVE_MAXIMUM_CHANNEL_TIME,
				pConfig->nPassiveMaxChnTime)
				== QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_PASSIVE_MAXIMUM_CHANNEL_TIME"
		       " to CFG");
	}

	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_BEACON_INTERVAL,
		     pConfig->nBeaconInterval) == QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_BEACON_INTERVAL to CFG");
	}

	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_MAX_PS_POLL,
		     pConfig->nMaxPsPoll) == QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE, "Could not pass on WNI_CFG_MAX_PS_POLL to CFG");
	}

	if (sme_cfg_set_int (pHddCtx->hHal, WNI_CFG_LOW_GAIN_OVERRIDE,
		    pConfig->fIsLowGainOverride) == QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_LOW_GAIN_OVERRIDE to HAL");
	}

	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_RSSI_FILTER_PERIOD,
		    pConfig->nRssiFilterPeriod) == QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_RSSI_FILTER_PERIOD to CFG");
	}

	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_IGNORE_DTIM,
		     pConfig->fIgnoreDtim) == QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_IGNORE_DTIM to CFG");
	}

	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_PS_ENABLE_HEART_BEAT,
		    pConfig->fEnableFwHeartBeatMonitoring)
		    == QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_PS_HEART_BEAT to CFG");
	}

	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_PS_ENABLE_BCN_FILTER,
		    pConfig->fEnableFwBeaconFiltering) ==
		    QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_PS_BCN_FILTER to CFG");
	}

	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_PS_ENABLE_RSSI_MONITOR,
		    pConfig->fEnableFwRssiMonitoring) ==
		    QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_PS_RSSI_MONITOR to CFG");
	}

	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_PS_DATA_INACTIVITY_TIMEOUT,
		    pConfig->nDataInactivityTimeout) == QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_PS_DATA_INACTIVITY_TIMEOUT to CFG");
	}

	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_ENABLE_LTE_COEX,
		     pConfig->enableLTECoex) == QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_ENABLE_LTE_COEX to CFG");
	}

	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_AP_KEEP_ALIVE_TIMEOUT,
		    pConfig->apKeepAlivePeriod) == QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_AP_KEEP_ALIVE_TIMEOUT to CFG");
	}

	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_GO_KEEP_ALIVE_TIMEOUT,
		    pConfig->goKeepAlivePeriod) == QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_GO_KEEP_ALIVE_TIMEOUT to CFG");
	}

	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_AP_LINK_MONITOR_TIMEOUT,
		    pConfig->apLinkMonitorPeriod) == QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_AP_LINK_MONITOR_TIMEOUT to CFG");
	}

	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_GO_LINK_MONITOR_TIMEOUT,
		    pConfig->goLinkMonitorPeriod) == QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_GO_LINK_MONITOR_TIMEOUT to CFG");
	}

	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_MCAST_BCAST_FILTER_SETTING,
		    pConfig->mcastBcastFilterSetting) == QDF_STATUS_E_FAILURE)
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_MCAST_BCAST_FILTER_SETTING to CFG");

	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_SINGLE_TID_RC,
		    pConfig->bSingleTidRc) == QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_SINGLE_TID_RC to CFG");
	}

	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_TELE_BCN_WAKEUP_EN,
		    pConfig->teleBcnWakeupEn) == QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_TELE_BCN_WAKEUP_EN to CFG");
	}

	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_TELE_BCN_TRANS_LI,
		    pConfig->nTeleBcnTransListenInterval) ==
		    QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_TELE_BCN_TRANS_LI to CFG");
	}

	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_TELE_BCN_MAX_LI,
		    pConfig->nTeleBcnMaxListenInterval) ==
		    QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_TELE_BCN_MAX_LI to CFG");
	}

	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_TELE_BCN_TRANS_LI_IDLE_BCNS,
		    pConfig->nTeleBcnTransLiNumIdleBeacons) ==
		    QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_TELE_BCN_TRANS_LI_IDLE_BCNS to CFG");
	}

	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_TELE_BCN_MAX_LI_IDLE_BCNS,
		    pConfig->nTeleBcnMaxLiNumIdleBeacons) ==
		    QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_TELE_BCN_MAX_LI_IDLE_BCNS to CFG");
	}

	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_RF_SETTLING_TIME_CLK,
		    pConfig->rfSettlingTimeUs) == QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_RF_SETTLING_TIME_CLK to CFG");
	}

	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_INFRA_STA_KEEP_ALIVE_PERIOD,
		    pConfig->infraStaKeepAlivePeriod) ==
		    QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_INFRA_STA_KEEP_ALIVE_PERIOD to CFG");
	}
	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_DYNAMIC_PS_POLL_VALUE,
		    pConfig->dynamicPsPollValue) == QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_DYNAMIC_PS_POLL_VALUE to CFG");
	}

	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_PS_NULLDATA_AP_RESP_TIMEOUT,
		    pConfig->nNullDataApRespTimeout) == QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_PS_NULLDATA_DELAY_TIMEOUT to CFG");
	}

	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_AP_DATA_AVAIL_POLL_PERIOD,
		    pConfig->apDataAvailPollPeriodInMs) ==
		    QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_AP_DATA_AVAIL_POLL_PERIOD to CFG");
	}
	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_FRAGMENTATION_THRESHOLD,
		    pConfig->FragmentationThreshold) == QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_FRAGMENTATION_THRESHOLD to CFG");
	}
	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_RTS_THRESHOLD,
		     pConfig->RTSThreshold) == QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_RTS_THRESHOLD to CFG");
	}

	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_11D_ENABLED,
		     pConfig->Is11dSupportEnabled) == QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_11D_ENABLED to CFG");
	}

	if (sme_cfg_set_int(pHddCtx->hHal, WNI_CFG_DFS_MASTER_ENABLED,
			    pConfig->enableDFSMasterCap) ==
			QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Failure: Could not set value for WNI_CFG_DFS_MASTER_ENABLED");
	}

	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_HEART_BEAT_THRESHOLD,
		    pConfig->HeartbeatThresh24) == QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_HEART_BEAT_THRESHOLD to CFG");
	}

	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_AP_DATA_AVAIL_POLL_PERIOD,
		    pConfig->apDataAvailPollPeriodInMs) ==
		    QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_AP_DATA_AVAIL_POLL_PERIOD to CFG");
	}

	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_ENABLE_MC_ADDR_LIST,
		    pConfig->fEnableMCAddrList) == QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_ENABLE_MC_ADDR_LIST to CFG");
	}

#ifdef WLAN_SOFTAP_VSTA_FEATURE
	if (pConfig->fEnableVSTASupport) {
		sme_cfg_get_int(pHddCtx->hHal, WNI_CFG_ASSOC_STA_LIMIT, &val);
		if (val <= WNI_CFG_ASSOC_STA_LIMIT_STADEF)
			val = WNI_CFG_ASSOC_STA_LIMIT_STAMAX;
	} else {
		val = pConfig->maxNumberOfPeers;

	}
	if (sme_cfg_set_int(pHddCtx->hHal, WNI_CFG_ASSOC_STA_LIMIT, val) ==
			QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_ASSOC_STA_LIMIT to CFG");
	}
#endif
	if (sme_cfg_set_int(pHddCtx->hHal, WNI_CFG_ENABLE_LPWR_IMG_TRANSITION,
			    pConfig->enableLpwrImgTransition)
			== QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_ENABLE_LPWR_IMG_TRANSITION to CFG");
	}

	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_ENABLE_MCC_ADAPTIVE_SCHED,
		    pConfig->enableMCCAdaptiveScheduler) ==
		    QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_ENABLE_MCC_ADAPTIVE_SCHED to CFG");
	}
	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_DISABLE_LDPC_WITH_TXBF_AP,
		    pConfig->disableLDPCWithTxbfAP) == QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_DISABLE_LDPC_WITH_TXBF_AP to CFG");
	}

	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_DYNAMIC_THRESHOLD_ZERO,
		    pConfig->retryLimitZero) == QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_DYNAMIC_THRESHOLD_ZERO to CFG");
	}

	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_DYNAMIC_THRESHOLD_ONE,
		    pConfig->retryLimitOne) == QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_DYNAMIC_THRESHOLD_ONE to CFG");
	}

	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_DYNAMIC_THRESHOLD_TWO,
		    pConfig->retryLimitTwo) == QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_DYNAMIC_THRESHOLD_TWO to CFG");
	}

	if (sme_cfg_set_int
		    (pHddCtx->hHal, WNI_CFG_MAX_MEDIUM_TIME,
		     pConfig->cfgMaxMediumTime) == QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_MAX_MEDIUM_TIME to CFG");
	}
#ifdef FEATURE_WLAN_TDLS

	if (sme_cfg_set_int(pHddCtx->hHal, WNI_CFG_TDLS_QOS_WMM_UAPSD_MASK,
			    pConfig->fTDLSUapsdMask) == QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_TDLS_QOS_WMM_UAPSD_MASK to CFG");
	}
	if (sme_cfg_set_int(pHddCtx->hHal, WNI_CFG_TDLS_BUF_STA_ENABLED,
			    pConfig->fEnableTDLSBufferSta) ==
			QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_TDLS_BUF_STA_ENABLED to CFG");
	}
	if (sme_cfg_set_int(pHddCtx->hHal, WNI_CFG_TDLS_PUAPSD_INACT_TIME,
			    pConfig->fTDLSPuapsdInactivityTimer) ==
			QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_TDLS_PUAPSD_INACT_TIME to CFG");
	}
	if (sme_cfg_set_int(pHddCtx->hHal, WNI_CFG_TDLS_RX_FRAME_THRESHOLD,
			    pConfig->fTDLSRxFrameThreshold) ==
			QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_TDLS_RX_FRAME_THRESHOLD to CFG");
	}

	if (sme_cfg_set_int(pHddCtx->hHal, WNI_CFG_TDLS_OFF_CHANNEL_ENABLED,
			    pConfig->fEnableTDLSOffChannel) ==
			QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_TDLS_BUF_STA_ENABLED to CFG");
	}
	if (sme_cfg_set_int(pHddCtx->hHal, WNI_CFG_TDLS_WMM_MODE_ENABLED,
			    pConfig->fEnableTDLSWmmMode) ==
			QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_TDLS_WMM_MODE_ENABLED to CFG");
	}
#endif

	if (sme_cfg_set_int(pHddCtx->hHal, WNI_CFG_ENABLE_ADAPT_RX_DRAIN,
			    pConfig->fEnableAdaptRxDrain) ==
			QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_ENABLE_ADAPT_RX_DRAIN to CFG");
	}

	if (sme_cfg_set_int(pHddCtx->hHal, WNI_CFG_ANTENNA_DIVESITY,
			    pConfig->antennaDiversity) ==
			QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_ANTENNA_DIVESITY to CFG");
	}

	if (sme_cfg_set_int(pHddCtx->hHal,
			    WNI_CFG_DEFAULT_RATE_INDEX_24GHZ,
			    pConfig->defaultRateIndex24Ghz) ==
			QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_DEFAULT_RATE_INDEX_24GHZ to CFG");
	}

	if (sme_cfg_set_int(pHddCtx->hHal,
			    WNI_CFG_DEBUG_P2P_REMAIN_ON_CHANNEL,
			    pConfig->debugP2pRemainOnChannel) ==
			QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_DEBUG_P2P_REMAIN_ON_CHANNEL to CFG");
	}
#ifdef WLAN_FEATURE_11W
	if (sme_cfg_set_int(pHddCtx->hHal, WNI_CFG_PMF_SA_QUERY_MAX_RETRIES,
			    pConfig->pmfSaQueryMaxRetries) ==
			QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_SA_QUERY_MAX_RETRIES to CFG");
	}

	if (sme_cfg_set_int(pHddCtx->hHal, WNI_CFG_PMF_SA_QUERY_RETRY_INTERVAL,
			    pConfig->pmfSaQueryRetryInterval) ==
			QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_SA_QUERY_RETRY_INTERVAL to CFG");
	}
#endif

	if (sme_cfg_set_int(pHddCtx->hHal, WNI_CFG_IBSS_ATIM_WIN_SIZE,
			    pConfig->ibssATIMWinSize) ==
			QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_IBSS_ATIM_WIN_SIZE to CFG");
	}

	if (sme_cfg_set_int(pHddCtx->hHal, WNI_CFG_TGT_GTX_USR_CFG,
	    pConfig->tgt_gtx_usr_cfg) == QDF_STATUS_E_FAILURE) {
		fStatus = false;
		hddLog(LOGE,
		       "Could not pass on WNI_CFG_TGT_GTX_USR_CFG to CCM");
	}
	return fStatus;
}
#ifdef FEATURE_WLAN_SCAN_PNO
/**
 * hdd_set_pno_channel_prediction_config() - Set PNO configuration
 * @sme_config:         Config params from SME Context
 * @hdd_ctx:            Config params from HDD Context
 *
 * Copy the PNO Channel prediction feature configuration parameters
 * from HDD context to SME context.
 *
 * Return: None
 */
void hdd_set_pno_channel_prediction_config(
		tpSmeConfigParams sme_config, hdd_context_t *hdd_ctx)
{
	sme_config->csrConfig.dual_mac_feature_disable =
		hdd_ctx->config->dual_mac_feature_disable;
	sme_config->csrConfig.pno_channel_prediction =
		hdd_ctx->config->pno_channel_prediction;
	sme_config->csrConfig.top_k_num_of_channels =
		hdd_ctx->config->top_k_num_of_channels;
	sme_config->csrConfig.stationary_thresh =
		hdd_ctx->config->stationary_thresh;
	sme_config->csrConfig.channel_prediction_full_scan =
		hdd_ctx->config->channel_prediction_full_scan;
	sme_config->csrConfig.pnoscan_adaptive_dwell_mode =
		hdd_ctx->config->pnoscan_adaptive_dwell_mode;
}
#endif
/**
 * hdd_set_sme_config() -initializes the sme configuration parameters
 *
 * @pHddCtx: the pointer to hdd context
 *
 * Return: QDF_STATUS_SUCCESS if configuration is correctly applied,
 *		otherwise the appropriate QDF_STATUS would be returned
 */
QDF_STATUS hdd_set_sme_config(hdd_context_t *pHddCtx)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSmeConfigParams *smeConfig;
	uint8_t rrm_capab_len;

	struct hdd_config *pConfig = pHddCtx->config;

	smeConfig = qdf_mem_malloc(sizeof(*smeConfig));
	if (NULL == smeConfig) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: unable to allocate smeConfig", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	qdf_mem_zero(smeConfig, sizeof(*smeConfig));

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "%s bWmmIsEnabled=%d 802_11e_enabled=%d dot11Mode=%d",
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
	smeConfig->csrConfig.TxRate = pConfig->TxRate;
	smeConfig->csrConfig.nScanResultAgeCount = pConfig->ScanResultAgeCount;
	smeConfig->csrConfig.scanAgeTimeNCNPS = pConfig->nScanAgeTimeNCNPS;
	smeConfig->csrConfig.scanAgeTimeNCPS = pConfig->nScanAgeTimeNCPS;
	smeConfig->csrConfig.scanAgeTimeCNPS = pConfig->nScanAgeTimeCNPS;
	smeConfig->csrConfig.scanAgeTimeCPS = pConfig->nScanAgeTimeCPS;
	smeConfig->csrConfig.AdHocChannel24 = pConfig->OperatingChannel;
	smeConfig->csrConfig.fSupplicantCountryCodeHasPriority =
		pConfig->fSupplicantCountryCodeHasPriority;
	smeConfig->csrConfig.bCatRssiOffset = pConfig->nRssiCatGap;
	smeConfig->csrConfig.vccRssiThreshold = pConfig->nVccRssiTrigger;
	smeConfig->csrConfig.vccUlMacLossThreshold =
		pConfig->nVccUlMacLossThreshold;
	smeConfig->csrConfig.nRoamingTime = pConfig->nRoamingTime;
	smeConfig->csrConfig.nInitialDwellTime = pConfig->nInitialDwellTime;
	smeConfig->csrConfig.initial_scan_no_dfs_chnl =
					pConfig->initial_scan_no_dfs_chnl;
	smeConfig->csrConfig.nActiveMaxChnTime = pConfig->nActiveMaxChnTime;
	smeConfig->csrConfig.nActiveMinChnTime = pConfig->nActiveMinChnTime;
	smeConfig->csrConfig.nPassiveMaxChnTime = pConfig->nPassiveMaxChnTime;
	smeConfig->csrConfig.nPassiveMinChnTime = pConfig->nPassiveMinChnTime;
#ifdef WLAN_AP_STA_CONCURRENCY
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
	smeConfig->csrConfig.nNumStaChanCombinedConc =
		pConfig->nNumStaChanCombinedConc;
	smeConfig->csrConfig.nNumP2PChanCombinedConc =
		pConfig->nNumP2PChanCombinedConc;

#endif
	smeConfig->csrConfig.Is11eSupportEnabled = pConfig->b80211eIsEnabled;
	smeConfig->csrConfig.WMMSupportMode = pConfig->WmmMode;

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
	smeConfig->csrConfig.enable_txbf_sap_mode =
		pConfig->enable_txbf_sap_mode;
	smeConfig->csrConfig.enable2x2 = pConfig->enable2x2;
	smeConfig->csrConfig.enableVhtFor24GHz = pConfig->enableVhtFor24GHzBand;
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
	if (pConfig->nBandCapability == eCSR_BAND_24) {
		smeConfig->csrConfig.Is11hSupportEnabled = 0;
	} else {
		smeConfig->csrConfig.Is11hSupportEnabled =
			pConfig->Is11hSupportEnabled;
	}
	smeConfig->csrConfig.cbChoice = 0;
	smeConfig->csrConfig.eBand = pConfig->nBandCapability;
	smeConfig->csrConfig.nTxPowerCap = pConfig->nTxPowerCap;
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

	hdd_set_power_save_offload_config(pHddCtx);

	smeConfig->csrConfig.csr11rConfig.IsFTResourceReqSupported =
		pConfig->fFTResourceReqSupported;
	smeConfig->csrConfig.isFastRoamIniFeatureEnabled =
		pConfig->isFastRoamIniFeatureEnabled;
	smeConfig->csrConfig.MAWCEnabled = pConfig->MAWCEnabled;
#ifdef FEATURE_WLAN_ESE
	smeConfig->csrConfig.isEseIniFeatureEnabled =
		pConfig->isEseIniFeatureEnabled;
	if (pConfig->isEseIniFeatureEnabled) {
		pConfig->isFastTransitionEnabled = true;
	}
#endif
	smeConfig->csrConfig.isFastTransitionEnabled =
		pConfig->isFastTransitionEnabled;
	smeConfig->csrConfig.RoamRssiDiff = pConfig->RoamRssiDiff;
	smeConfig->csrConfig.isWESModeEnabled = pConfig->isWESModeEnabled;
	smeConfig->csrConfig.isRoamOffloadScanEnabled =
		pConfig->isRoamOffloadScanEnabled;
	smeConfig->csrConfig.bFastRoamInConIniFeatureEnabled =
		pConfig->bFastRoamInConIniFeatureEnabled;

	if (0 == smeConfig->csrConfig.isRoamOffloadScanEnabled) {
		/* Disable roaming in concurrency if roam scan offload is disabled */
		smeConfig->csrConfig.bFastRoamInConIniFeatureEnabled = 0;
	}
	smeConfig->csrConfig.neighborRoamConfig.nNeighborLookupRssiThreshold =
		pConfig->nNeighborLookupRssiThreshold;
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
	smeConfig->csrConfig.fValidateList = pConfig->fValidateScanList;
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

	smeConfig->csrConfig.enableTxLdpc = pConfig->enableTxLdpc;
	smeConfig->csrConfig.enableRxLDPC = pConfig->enableRxLDPC;
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
	smeConfig->csrConfig.cc_switch_mode = pConfig->WlanMccToSccSwitchMode;
#endif

	smeConfig->csrConfig.isAmsduSupportInAMPDU =
		pConfig->isAmsduSupportInAMPDU;
	smeConfig->csrConfig.nSelect5GHzMargin = pConfig->nSelect5GHzMargin;

	smeConfig->csrConfig.isCoalesingInIBSSAllowed =
		pHddCtx->config->isCoalesingInIBSSAllowed;
	smeConfig->csrConfig.ignore_peer_erp_info =
						pConfig->ignore_peer_erp_info;
	/* update SSR config */
	sme_update_enable_ssr((tHalHandle) (pHddCtx->hHal),
			      pHddCtx->config->enableSSR);

#ifdef FEATURE_WLAN_SCAN_PNO
	/* Update PNO offoad status */
	smeConfig->csrConfig.pnoOffload = pHddCtx->config->PnoOffload;
#endif

	/* Update maximum interfaces information */
	smeConfig->csrConfig.max_intf_count = pHddCtx->max_intf_count;

	smeConfig->csrConfig.fEnableDebugLog = pHddCtx->config->gEnableDebugLog;

	smeConfig->csrConfig.enable5gEBT = pHddCtx->config->enable5gEBT;

	smeConfig->csrConfig.enableSelfRecovery =
			pHddCtx->config->enableSelfRecovery;
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	smeConfig->csrConfig.isRoamOffloadEnabled =
		pHddCtx->config->isRoamOffloadEnabled;
#endif
	smeConfig->csrConfig.conc_custom_rule1 =
		pHddCtx->config->conc_custom_rule1;
	smeConfig->csrConfig.conc_custom_rule2 =
		pHddCtx->config->conc_custom_rule2;
	smeConfig->csrConfig.is_sta_connection_in_5gz_enabled =
		pHddCtx->config->is_sta_connection_in_5gz_enabled;

	smeConfig->csrConfig.f_sta_miracast_mcc_rest_time_val =
		pHddCtx->config->sta_miracast_mcc_rest_time_val;

#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
	smeConfig->csrConfig.sap_channel_avoidance =
		pHddCtx->config->sap_channel_avoidance;
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */

	smeConfig->csrConfig.f_prefer_non_dfs_on_radar =
		pHddCtx->config->prefer_non_dfs_on_radar;

	smeConfig->csrConfig.is_ps_enabled = pHddCtx->config->is_ps_enabled;
	hdd_set_fine_time_meas_cap(pHddCtx, smeConfig);

	cds_set_multicast_logging(pHddCtx->config->multicast_host_fw_msgs);

	smeConfig->csrConfig.sendDeauthBeforeCon = pConfig->sendDeauthBeforeCon;

	smeConfig->csrConfig.max_scan_count =
			pHddCtx->config->max_scan_count;

	/* Update 802.11p config */
	smeConfig->csrConfig.enable_dot11p =
		(pHddCtx->config->dot11p_mode != WLAN_HDD_11P_DISABLED);
	hdd_set_pno_channel_prediction_config(smeConfig, pHddCtx);

	smeConfig->csrConfig.early_stop_scan_enable =
		pHddCtx->config->early_stop_scan_enable;
	smeConfig->csrConfig.early_stop_scan_min_threshold =
		pHddCtx->config->early_stop_scan_min_threshold;
	smeConfig->csrConfig.early_stop_scan_max_threshold =
		pHddCtx->config->early_stop_scan_max_threshold;
	smeConfig->csrConfig.first_scan_bucket_threshold =
		pHddCtx->config->first_scan_bucket_threshold;

	smeConfig->csrConfig.roam_dense_rssi_thresh_offset =
			pHddCtx->config->roam_dense_rssi_thresh_offset;
	smeConfig->csrConfig.roam_dense_min_aps =
			pHddCtx->config->roam_dense_min_aps;
	smeConfig->csrConfig.roam_dense_traffic_thresh =
			pHddCtx->config->roam_dense_traffic_thresh;
	smeConfig->csrConfig.obss_width_interval =
			pHddCtx->config->obss_width_trigger_interval;
	smeConfig->csrConfig.obss_active_dwelltime =
			pHddCtx->config->obss_active_dwelltime;
	smeConfig->csrConfig.obss_passive_dwelltime =
			pHddCtx->config->obss_passive_dwelltime;
	smeConfig->csrConfig.ignore_peer_ht_opmode =
			pConfig->ignore_peer_ht_opmode;
	smeConfig->csrConfig.enable_fatal_event =
			pConfig->enable_fatal_event;
	smeConfig->csrConfig.scan_adaptive_dwell_mode =
			pHddCtx->config->scan_adaptive_dwell_mode;
	smeConfig->csrConfig.roamscan_adaptive_dwell_mode =
			pHddCtx->config->roamscan_adaptive_dwell_mode;

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

	status = sme_update_config(pHddCtx->hHal, smeConfig);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hddLog(LOGE, "sme_update_config() return failure %d",
		       status);
	}

	qdf_mem_free(smeConfig);
	return status;
}

/**
 * hdd_execute_global_config_command() - execute the global config command
 * @pHddCtx: the pointer to hdd context
 * @command: the command to run
 *
 * Return: the QDF_STATUS return from hdd_execute_config_command
 */
QDF_STATUS hdd_execute_global_config_command(hdd_context_t *pHddCtx,
					     char *command)
{
	return hdd_execute_config_command(g_registry_table,
					  ARRAY_SIZE(g_registry_table),
					  (uint8_t *) pHddCtx->config,
					  pHddCtx, command);
}

/**
 * hdd_cfg_get_global_config() - get the configuration table
 * @pHddCtx: pointer to hdd context
 * @pBuf: buffer to store the configuration
 * @buflen: size of the buffer
 *
 * Return: QDF_STATUS_SUCCESS if the configuration and buffer size can carry
 *		the content, otherwise QDF_STATUS_E_RESOURCES
 */
QDF_STATUS hdd_cfg_get_global_config(hdd_context_t *pHddCtx, char *pBuf,
				     int buflen)
{
	return hdd_cfg_get_config(g_registry_table,
				  ARRAY_SIZE(g_registry_table),
				  (uint8_t *) pHddCtx->config, pHddCtx, pBuf,
				  buflen);
}

/**
 * hdd_is_okc_mode_enabled() - returns whether OKC mode is enabled or not
 * @pHddCtx: the pointer to hdd context
 *
 * Return: true if OKC is enabled, otherwise false
 */
bool hdd_is_okc_mode_enabled(hdd_context_t *pHddCtx)
{
	if (NULL == pHddCtx) {
		hddLog(QDF_TRACE_LEVEL_FATAL, "%s: pHddCtx is NULL", __func__);
		return -EINVAL;
	}
	return pHddCtx->config->isOkcIniFeatureEnabled;
}

/**
 * hdd_update_nss() - Update the number of spatial streams supported.
 * Ensure that nss is either 1 or 2 before calling this.
 *
 * @hdd_ctx: the pointer to hdd context
 * @nss: the number of spatial streams to be updated
 *
 * This function is used to modify the number of spatial streams
 * supported when not in connected state.
 *
 * Return: QDF_STATUS_SUCCESS if nss is correctly updated,
 *              otherwise QDF_STATUS_E_FAILURE would be returned
 */
QDF_STATUS hdd_update_nss(hdd_context_t *hdd_ctx, uint8_t nss)
{
	struct hdd_config *hdd_config = hdd_ctx->config;
	uint32_t temp = 0;
	uint32_t rx_supp_data_rate, tx_supp_data_rate;
	bool status = true;
	tSirMacHTCapabilityInfo *ht_cap_info;
	uint8_t mcs_set[SIZE_OF_SUPPORTED_MCS_SET] = {0};
	uint8_t mcs_set_temp[SIZE_OF_SUPPORTED_MCS_SET];
	uint32_t val, val32;
	uint16_t val16;
	uint8_t enable2x2;

	if ((nss == 2) && (hdd_ctx->num_rf_chains != 2)) {
		hddLog(LOGE, "No support for 2 spatial streams");
		return QDF_STATUS_E_INVAL;
	}

	enable2x2 = (nss == 1) ? 0 : 1;

	if (hdd_config->enable2x2 == enable2x2) {
		hddLog(LOGE, "NSS same as requested");
		return QDF_STATUS_SUCCESS;
	}

	if (true == sme_is_any_session_in_connected_state(hdd_ctx->hHal)) {
		hddLog(LOGE, "Connected sessions present, Do not change NSS");
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
	if (sme_cfg_set_int(hdd_ctx->hHal,
			    WNI_CFG_VHT_RX_HIGHEST_SUPPORTED_DATA_RATE,
			    rx_supp_data_rate) == QDF_STATUS_E_FAILURE) {
		status = false;
		hddLog(LOGE,
			"Could not pass on WNI_CFG_VHT_RX_HIGHEST_SUPPORTED_DATA_RATE to CFG");
	}

	/* Update Tx Highest Long GI data Rate */
	if (sme_cfg_set_int(hdd_ctx->hHal,
			    WNI_CFG_VHT_TX_HIGHEST_SUPPORTED_DATA_RATE,
			    tx_supp_data_rate) == QDF_STATUS_E_FAILURE) {
		status = false;
		hddLog(LOGE,
			"Could not pass on WNI_CFG_VHT_TX_HIGHEST_SUPPORTED_DATA_RATE to CFG");
	}

	sme_cfg_get_int(hdd_ctx->hHal, WNI_CFG_HT_CAP_INFO, &temp);
	val16 = (uint16_t)temp;
	ht_cap_info = (tSirMacHTCapabilityInfo *)&val16;
	if (!(hdd_ctx->ht_tx_stbc_supported && hdd_config->enable2x2)) {
		ht_cap_info->txSTBC = 0;
	} else {
		sme_cfg_get_int(hdd_ctx->hHal, WNI_CFG_VHT_TXSTBC, &val32);
		hddLog(LOG1, FL("STBC %d"), val32);
		ht_cap_info->txSTBC = val32;
	}
	temp = val16;
	if (sme_cfg_set_int(hdd_ctx->hHal, WNI_CFG_HT_CAP_INFO,
			    temp) == QDF_STATUS_E_FAILURE) {
		status = false;
		hddLog(LOGE, "Could not pass on WNI_CFG_HT_CAP_INFO to CFG");
	}

	sme_cfg_get_int(hdd_ctx->hHal, WNI_CFG_VHT_BASIC_MCS_SET, &temp);
	temp = (temp & 0xFFFC) | hdd_config->vhtRxMCS;
	if (hdd_config->enable2x2)
		temp = (temp & 0xFFF3) | (hdd_config->vhtRxMCS2x2 << 2);
	else
		temp |= 0x000C;

	if (sme_cfg_set_int(hdd_ctx->hHal, WNI_CFG_VHT_BASIC_MCS_SET,
			    temp) == QDF_STATUS_E_FAILURE) {
		status = false;
		hddLog(LOGE,
			"Could not pass on WNI_CFG_VHT_BASIC_MCS_SET to CFG");
	}

	sme_cfg_get_int(hdd_ctx->hHal, WNI_CFG_VHT_RX_MCS_MAP, &temp);
	temp = (temp & 0xFFFC) | hdd_config->vhtRxMCS;
	if (hdd_config->enable2x2)
		temp = (temp & 0xFFF3) | (hdd_config->vhtRxMCS2x2 << 2);
	else
		temp |= 0x000C;

	if (sme_cfg_set_int(hdd_ctx->hHal, WNI_CFG_VHT_RX_MCS_MAP,
			    temp) == QDF_STATUS_E_FAILURE) {
		status = false;
		hddLog(LOGE, "Could not pass on WNI_CFG_VHT_RX_MCS_MAP to CFG");
	}

	sme_cfg_get_int(hdd_ctx->hHal, WNI_CFG_VHT_TX_MCS_MAP, &temp);
	temp = (temp & 0xFFFC) | hdd_config->vhtTxMCS;
	if (hdd_config->enable2x2)
		temp = (temp & 0xFFF3) | (hdd_config->vhtTxMCS2x2 << 2);
	else
		temp |= 0x000C;

	if (sme_cfg_set_int(hdd_ctx->hHal, WNI_CFG_VHT_TX_MCS_MAP,
			    temp) == QDF_STATUS_E_FAILURE) {
		status = false;
		hddLog(LOGE, "Could not pass on WNI_CFG_VHT_TX_MCS_MAP to CFG");
	}

#define WLAN_HDD_RX_MCS_ALL_NSTREAM_RATES 0xff
	val = SIZE_OF_SUPPORTED_MCS_SET;
	sme_cfg_get_str(hdd_ctx->hHal, WNI_CFG_SUPPORTED_MCS_SET,
			mcs_set_temp, &val);

	mcs_set[0] = mcs_set_temp[0];
	if (hdd_config->enable2x2)
		for (val = 0; val < nss; val++)
			mcs_set[val] = WLAN_HDD_RX_MCS_ALL_NSTREAM_RATES;

	if (sme_cfg_set_str(hdd_ctx->hHal, WNI_CFG_SUPPORTED_MCS_SET,
			    mcs_set,
			    SIZE_OF_SUPPORTED_MCS_SET) ==
				QDF_STATUS_E_FAILURE) {
		status = false;
		hddLog(LOGE, "Could not pass on MCS SET to CFG");
	}
#undef WLAN_HDD_RX_MCS_ALL_NSTREAM_RATES

	if (QDF_STATUS_SUCCESS != sme_update_nss(hdd_ctx->hHal, nss))
		status = false;

	return (status == false) ? QDF_STATUS_E_FAILURE : QDF_STATUS_SUCCESS;
}
