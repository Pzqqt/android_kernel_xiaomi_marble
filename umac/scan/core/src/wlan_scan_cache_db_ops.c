/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * DOC: contains scan cache operations
 */

#include <qdf_status.h>
#include <wlan_scan_ucfg_api.h>
#include <wlan_scan_utils_api.h>
#include "wlan_scan_cache_db.h"
#include "wlan_scan_main.h"
#include "wlan_scan_cache_db_i.h"
#ifdef WLAN_POLICY_MGR_ENABLE
#include "wlan_policy_mgr_api.h"
#endif

#define SCM_20MHZ_BW_INDEX                  0
#define SCM_40MHZ_BW_INDEX                  1
#define SCM_80MHZ_BW_INDEX                  2
#define SCM_160MHZ_BW_INDEX                 3
#define SCM_MAX_BW_INDEX                    4

#define SCM_NSS_1x1_INDEX                   0
#define SCM_NSS_2x2_INDEX                   1
#define SCM_NSS_3x3_INDEX                   2
#define SCM_NSS_4x4_INDEX                   3
#define SCM_MAX_NSS_INDEX                   4

#define SCM_BAND_2G_INDEX                   0
#define SCM_BAND_5G_INDEX                   1
/* 2 and 3 are reserved */
#define SCM_MAX_BAND_INDEX                  4

#define SCM_SCORE_INDEX_0                   0
#define SCM_SCORE_INDEX_3                   3
#define SCM_SCORE_INDEX_7                   7
#define SCM_SCORE_OFFSET_INDEX_7_4          4
#define SCM_SCORE_INDEX_11                  11
#define SCM_SCORE_OFFSET_INDEX_11_8         8
#define SCM_SCORE_MAX_INDEX                 15
#define SCM_SCORE_OFFSET_INDEX_15_12        12

#define SCM_MAX_OCE_WAN_DL_CAP 16

#define SCM_MAX_CHANNEL_WEIGHT 100
#define SCM_MAX_CHANNEL_UTILIZATION 100
#define SCM_MAX_ESTIMATED_AIR_TIME_FRACTION 255
#define MAX_AP_LOAD 255

#define SCM_MAX_OCE_WAN_DL_CAP 16

#define SCM_MAX_WEIGHT_OF_PCL_CHANNELS 255
#define SCM_PCL_GROUPS_WEIGHT_DIFFERENCE 20

bool scm_is_better_bss(struct scan_default_params *params,
	struct scan_cache_entry *bss1,
	struct scan_cache_entry *bss2)
{
	if (bss1->bss_score > bss2->bss_score)
		return true;
	else if (bss1->bss_score == bss2->bss_score)
		if (bss1->rssi_raw > bss2->rssi_raw)
			return true;

	return false;
}

/**
 * scm_limit_max_per_index_score() -check if per index score does not exceed
 * 100% (0x64). If it exceed make it 100%
 *
 * @per_index_score: per_index_score as input
 *
 * Return: per_index_score within the max limit
 */
static uint32_t scm_limit_max_per_index_score(uint32_t per_index_score)
{
	uint8_t i, score;

	for (i = 0; i < MAX_INDEX_PER_INI; i++) {
		score = WLAN_GET_SCORE_PERCENTAGE(per_index_score, i);
		if (score > MAX_INDEX_SCORE)
			WLAN_SET_SCORE_PERCENTAGE(per_index_score,
				MAX_INDEX_SCORE, i);
	}

	return per_index_score;
}

void scm_validate_scoring_config(struct scoring_config *score_cfg)
{
	int total_weight;

	total_weight = score_cfg->weight_cfg.rssi_weightage +
		       score_cfg->weight_cfg.ht_caps_weightage +
		       score_cfg->weight_cfg.vht_caps_weightage +
		       score_cfg->weight_cfg.chan_width_weightage +
		       score_cfg->weight_cfg.chan_band_weightage +
		       score_cfg->weight_cfg.nss_weightage +
		       score_cfg->weight_cfg.beamforming_cap_weightage +
		       score_cfg->weight_cfg.pcl_weightage +
		       score_cfg->weight_cfg.channel_congestion_weightage +
		       score_cfg->weight_cfg.oce_wan_weightage;

	if (total_weight > BEST_CANDIDATE_MAX_WEIGHT) {

		scm_err("total weight is greater than %d fallback to default values",
			BEST_CANDIDATE_MAX_WEIGHT);

		score_cfg->weight_cfg.rssi_weightage = RSSI_WEIGHTAGE;
		score_cfg->weight_cfg.ht_caps_weightage =
			HT_CAPABILITY_WEIGHTAGE;
		score_cfg->weight_cfg.vht_caps_weightage = VHT_CAP_WEIGHTAGE;
		score_cfg->weight_cfg.chan_width_weightage =
			CHAN_WIDTH_WEIGHTAGE;
		score_cfg->weight_cfg.chan_band_weightage =
			CHAN_BAND_WEIGHTAGE;
		score_cfg->weight_cfg.nss_weightage = NSS_WEIGHTAGE;
		score_cfg->weight_cfg.beamforming_cap_weightage =
			BEAMFORMING_CAP_WEIGHTAGE;
		score_cfg->weight_cfg.pcl_weightage = PCL_WEIGHT;
			score_cfg->weight_cfg.channel_congestion_weightage =
			CHANNEL_CONGESTION_WEIGHTAGE;
		score_cfg->weight_cfg.oce_wan_weightage = OCE_WAN_WEIGHTAGE;
	}

	score_cfg->bandwidth_weight_per_index =
		scm_limit_max_per_index_score(
			score_cfg->bandwidth_weight_per_index);
	score_cfg->nss_weight_per_index =
		scm_limit_max_per_index_score(score_cfg->nss_weight_per_index);
	score_cfg->band_weight_per_index =
		scm_limit_max_per_index_score(score_cfg->band_weight_per_index);


	score_cfg->esp_qbss_scoring.score_pcnt3_to_0 =
		scm_limit_max_per_index_score(
			score_cfg->esp_qbss_scoring.score_pcnt3_to_0);
	score_cfg->esp_qbss_scoring.score_pcnt7_to_4 =
		scm_limit_max_per_index_score(
			score_cfg->esp_qbss_scoring.score_pcnt7_to_4);
	score_cfg->esp_qbss_scoring.score_pcnt11_to_8 =
		scm_limit_max_per_index_score(
			score_cfg->esp_qbss_scoring.score_pcnt11_to_8);
	score_cfg->esp_qbss_scoring.score_pcnt15_to_12 =
		scm_limit_max_per_index_score(
			score_cfg->esp_qbss_scoring.score_pcnt15_to_12);

	score_cfg->oce_wan_scoring.score_pcnt3_to_0 =
		scm_limit_max_per_index_score(
			score_cfg->oce_wan_scoring.score_pcnt3_to_0);
	score_cfg->oce_wan_scoring.score_pcnt7_to_4 =
		scm_limit_max_per_index_score(
			score_cfg->oce_wan_scoring.score_pcnt7_to_4);
	score_cfg->oce_wan_scoring.score_pcnt11_to_8 =
		scm_limit_max_per_index_score(
			score_cfg->oce_wan_scoring.score_pcnt11_to_8);
	score_cfg->oce_wan_scoring.score_pcnt15_to_12 =
		scm_limit_max_per_index_score(
			score_cfg->oce_wan_scoring.score_pcnt15_to_12);

}

/**
 * scm_get_rssi_pcnt_for_slot () - calculate rssi % score based on the slot
 * index between the high rssi and low rssi threshold
 * @high_rssi_threshold: High rssi of the window
 * @low_rssi_threshold: low rssi of the window
 * @high_rssi_pcnt: % score for the high rssi
 * @low_rssi_pcnt: %score for the low rssi
 * @bucket_size: bucket size of the window
 * @bss_rssi: Input rssi for which value need to be calculated
 *
 * Return : rssi pct to use for the given rssi
 */
static inline
int8_t scm_get_rssi_pcnt_for_slot(int32_t high_rssi_threshold,
	int32_t low_rssi_threshold, uint32_t high_rssi_pcnt,
	uint32_t low_rssi_pcnt, uint32_t bucket_size, int8_t bss_rssi)
{
	int8_t slot_index, slot_size, rssi_diff, num_slot, rssi_pcnt;

	num_slot = ((high_rssi_threshold -
		     low_rssi_threshold) / bucket_size) + 1;
	slot_size = ((high_rssi_pcnt - low_rssi_pcnt) +
		     (num_slot / 2)) / (num_slot);
	rssi_diff = high_rssi_threshold - bss_rssi;
	slot_index = (rssi_diff / bucket_size) + 1;
	rssi_pcnt = high_rssi_pcnt - (slot_size * slot_index);
	if (rssi_pcnt < low_rssi_pcnt)
		rssi_pcnt = low_rssi_pcnt;

	scm_debug("Window %d -> %d pcnt range %d -> %d bucket_size %d bss_rssi %d num_slot %d slot_size %d rssi_diff %d slot_index %d rssi_pcnt %d",
		  high_rssi_threshold, low_rssi_threshold, high_rssi_pcnt,
		  low_rssi_pcnt, bucket_size, bss_rssi, num_slot, slot_size,
		  rssi_diff, slot_index, rssi_pcnt);

	return rssi_pcnt;
}

/**
 * scm_calculate_rssi_score () - Calculate RSSI score based on AP RSSI
 * @score_param: rssi score params
 * @rssi: rssi of the AP
 * @rssi_weightage: rssi_weightage out of total weightage
 *
 * Return : rssi score
 */
static int32_t scm_calculate_rssi_score(
	struct rssi_cfg_score *score_param,
	int32_t rssi, uint8_t rssi_weightage)
{
	int8_t rssi_pcnt;
	int32_t total_rssi_score;
	int32_t best_rssi_threshold;
	int32_t good_rssi_threshold;
	int32_t bad_rssi_threshold;
	uint32_t good_rssi_pcnt;
	uint32_t bad_rssi_pcnt;
	uint32_t good_bucket_size;
	uint32_t bad_bucket_size;

	best_rssi_threshold = score_param->best_rssi_threshold*(-1);
	good_rssi_threshold = score_param->good_rssi_threshold*(-1);
	bad_rssi_threshold = score_param->bad_rssi_threshold*(-1);
	good_rssi_pcnt = score_param->good_rssi_pcnt;
	bad_rssi_pcnt = score_param->bad_rssi_pcnt;
	good_bucket_size = score_param->good_rssi_bucket_size;
	bad_bucket_size = score_param->bad_rssi_bucket_size;

	total_rssi_score = (BEST_CANDIDATE_MAX_WEIGHT * rssi_weightage);

	/*
	 * If RSSI is better than the best rssi threshold then it return full
	 * score.
	 */
	if (rssi > best_rssi_threshold)
		return total_rssi_score;
	/*
	 * If RSSI is less or equal to bad rssi threshold then it return
	 * least score.
	 */
	if (rssi <= bad_rssi_threshold)
		return (total_rssi_score * bad_rssi_pcnt) / 100;

	/* RSSI lies between best to good rssi threshold */
	if (rssi > good_rssi_threshold)
		rssi_pcnt = scm_get_rssi_pcnt_for_slot(best_rssi_threshold,
				good_rssi_threshold, 100, good_rssi_pcnt,
				good_bucket_size, rssi);
	else
		rssi_pcnt = scm_get_rssi_pcnt_for_slot(good_rssi_threshold,
				bad_rssi_threshold, good_rssi_pcnt,
				bad_rssi_pcnt, bad_bucket_size,
				rssi);

	return (total_rssi_score * rssi_pcnt) / 100;

}

/**
 * scm_calculate_pcl_score () - Calculate PCL score based on PCL weightage
 * @pcl_chan_weight: pcl weight of BSS channel
 * @pcl_weightage: PCL _weightage out of total weightage
 *
 * Return : pcl score
 */
static int32_t scm_calculate_pcl_score(int pcl_chan_weight,
		uint8_t pcl_weightage)
{
	int32_t pcl_score = 0;
	int32_t temp_pcl_chan_weight = 0;

	if (pcl_chan_weight) {
		temp_pcl_chan_weight =
			(SCM_MAX_WEIGHT_OF_PCL_CHANNELS - pcl_chan_weight);
		temp_pcl_chan_weight = qdf_do_div(temp_pcl_chan_weight,
					  SCM_PCL_GROUPS_WEIGHT_DIFFERENCE);
		pcl_score = pcl_weightage - temp_pcl_chan_weight;
		if (pcl_score < 0)
			pcl_score = 0;
	}
	return pcl_score * BEST_CANDIDATE_MAX_WEIGHT;

}

/**
 * scm_rssi_is_same_bucket () - check if both rssi fall in same bucket
 * @rssi_top_thresh: high rssi threshold of the the window
 * @low_rssi_threshold: low rssi of the window
 * @rssi_ref1: rssi ref one
 * @rssi_ref2: rssi ref two
 * @bucket_size: bucket size of the window
 *
 * Return : true if both fall in same window
 */
static inline bool scm_rssi_is_same_bucket(int8_t rssi_top_thresh,
	int8_t rssi_ref1, int8_t rssi_ref2, int8_t bucket_size)
{
	int8_t rssi_diff1 = 0;
	int8_t rssi_diff2 = 0;

	rssi_diff1 = rssi_top_thresh - rssi_ref1;
	rssi_diff2 = rssi_top_thresh - rssi_ref2;

	return (rssi_diff1 / bucket_size) == (rssi_diff2 / bucket_size);
}

/**
 * scm_roam_calculate_prorated_pcnt_by_rssi () - Calculate prorated RSSI score
 * based on AP RSSI. This will be used to determine HT VHT score
 * @score_param: rssi score params
 * @rssi: bss rssi
 * @rssi_weightage: rssi_weightage out of total weightage
 *
 * If rssi is greater than good threshold return 100, if less than bad return 0,
 * if between good and bad, return prorated rssi score for the index.
 *
 * Return : rssi prorated score
 */
static int8_t scm_roam_calculate_prorated_pcnt_by_rssi(
	struct rssi_cfg_score *score_param,
	int32_t rssi, uint8_t rssi_weightage)
{
	int32_t good_rssi_threshold;
	int32_t bad_rssi_threshold;
	int8_t rssi_pref_5g_rssi_thresh;
	bool same_bucket;

	good_rssi_threshold = score_param->good_rssi_threshold * (-1);
	bad_rssi_threshold = score_param->bad_rssi_threshold * (-1);
	rssi_pref_5g_rssi_thresh = score_param->rssi_pref_5g_rssi_thresh * (-1);

	/* If RSSI is greater than good rssi return full weight */
	if (rssi > good_rssi_threshold)
		return BEST_CANDIDATE_MAX_WEIGHT;

	same_bucket = scm_rssi_is_same_bucket(good_rssi_threshold,
			rssi, rssi_pref_5g_rssi_thresh,
			score_param->bad_rssi_bucket_size);
	if (same_bucket || (rssi < rssi_pref_5g_rssi_thresh))
		return 0;
	/* If RSSI is less or equal to bad rssi threshold then it return 0 */
	if (rssi <= bad_rssi_threshold)
		return 0;

	/* If RSSI is between good and bad threshold */
	return scm_get_rssi_pcnt_for_slot(good_rssi_threshold,
					  bad_rssi_threshold,
					  score_param->good_rssi_pcnt,
					  score_param->bad_rssi_pcnt,
					  score_param->bad_rssi_bucket_size,
					  rssi);
}

/**
 * scm_calculate_bandwidth_score () - Calculate BW score
 * @entry: scan entry
 * @score_config: scoring config
 * @prorated_pct: prorated % to return dependent on RSSI
 *
 * Return : bw score
 */
static int32_t scm_calculate_bandwidth_score(
	struct scan_cache_entry *entry,
	struct scoring_config *score_config, uint8_t prorated_pct)
{
	uint32_t score;
	int32_t bw_weight_per_idx;
	uint8_t cbmode = 0;
	uint8_t ch_width_index;
	bool is_vht;

	bw_weight_per_idx = score_config->bandwidth_weight_per_index;

	if (WLAN_CHAN_IS_2GHZ(entry->channel.chan_idx)) {
		cbmode = score_config->cb_mode_24G;
		if (score_config->vht_24G_cap)
			is_vht = true;
	} else if (score_config->vht_cap) {
		is_vht = true;
		cbmode = score_config->cb_mode_5G;
	}

	if (entry->phy_mode == WLAN_PHYMODE_11AC_VHT80_80 ||
	    entry->phy_mode == WLAN_PHYMODE_11AC_VHT160)
		ch_width_index = SCM_160MHZ_BW_INDEX;
	else if (entry->phy_mode == WLAN_PHYMODE_11AC_VHT80)
		 ch_width_index = SCM_80MHZ_BW_INDEX;
	else if (entry->phy_mode == WLAN_PHYMODE_11NA_HT40PLUS ||
		 entry->phy_mode == WLAN_PHYMODE_11NA_HT40MINUS ||
		 entry->phy_mode == WLAN_PHYMODE_11NG_HT40PLUS ||
		 entry->phy_mode == WLAN_PHYMODE_11NG_HT40MINUS ||
		 entry->phy_mode == WLAN_PHYMODE_11NG_HT40 ||
		 entry->phy_mode == WLAN_PHYMODE_11NA_HT40 ||
		 entry->phy_mode == WLAN_PHYMODE_11AC_VHT40PLUS ||
		 entry->phy_mode == WLAN_PHYMODE_11AC_VHT40MINUS ||
		 entry->phy_mode == WLAN_PHYMODE_11AC_VHT40)
		ch_width_index = SCM_40MHZ_BW_INDEX;
	else
		ch_width_index = SCM_20MHZ_BW_INDEX;


	if (!score_config->ht_cap && ch_width_index > SCM_20MHZ_BW_INDEX)
		ch_width_index = SCM_20MHZ_BW_INDEX;

	if (!is_vht && ch_width_index > SCM_40MHZ_BW_INDEX)
		ch_width_index = SCM_40MHZ_BW_INDEX;

	if (cbmode && ch_width_index > SCM_20MHZ_BW_INDEX)
		score = WLAN_GET_SCORE_PERCENTAGE(bw_weight_per_idx,
					ch_width_index);
	else
		score = WLAN_GET_SCORE_PERCENTAGE(bw_weight_per_idx,
					SCM_20MHZ_BW_INDEX);

	return (prorated_pct * score *
		score_config->weight_cfg.chan_width_weightage) /
		BEST_CANDIDATE_MAX_WEIGHT;
}

/**
 * scm_get_score_for_index () - get score for the given index
 * @index: index for which we need the score
 * @weightage: weigtage for the param
 * @score: per slot score
 *
 * Return : score for the index
 */
static int32_t scm_get_score_for_index(uint8_t index,
	uint8_t weightage, struct per_slot_scoring *score)
{
	if (index <= SCM_SCORE_INDEX_3)
		return weightage * WLAN_GET_SCORE_PERCENTAGE(
				   score->score_pcnt3_to_0,
				   index);
	else if (index <= SCM_SCORE_INDEX_7)
		return weightage * WLAN_GET_SCORE_PERCENTAGE(
				   score->score_pcnt7_to_4,
				   index - SCM_SCORE_OFFSET_INDEX_7_4);
	else if (index <= SCM_SCORE_INDEX_11)
		return weightage * WLAN_GET_SCORE_PERCENTAGE(
				   score->score_pcnt11_to_8,
				   index - SCM_SCORE_OFFSET_INDEX_11_8);
	else
		return weightage * WLAN_GET_SCORE_PERCENTAGE(
				   score->score_pcnt15_to_12,
				   index - SCM_SCORE_OFFSET_INDEX_15_12);
}

/**
 * scm_calculate_congestion_score () - Calculate congestion score
 * @entry: bss information
 * @score_params: bss score params
 *
 * Return : congestion score
 */
static int32_t scm_calculate_congestion_score(
	struct scan_cache_entry *entry,
	struct scoring_config *score_params)
{
	uint32_t ap_load = 0;
	uint32_t est_air_time_percentage = 0;
	uint32_t congestion = 0;
	uint32_t window_size;
	uint8_t index;
	int32_t good_rssi_threshold;

	if (!score_params->esp_qbss_scoring.num_slot)
		return 0;

	if (score_params->esp_qbss_scoring.num_slot >
	    SCM_SCORE_MAX_INDEX)
		score_params->esp_qbss_scoring.num_slot =
			SCM_SCORE_MAX_INDEX;

	good_rssi_threshold =
		score_params->rssi_score.good_rssi_threshold * (-1);

	/* For bad zone rssi get score from last index */
	if (entry->rssi_raw < good_rssi_threshold)
		return scm_get_score_for_index(
				score_params->esp_qbss_scoring.num_slot,
				score_params->weight_cfg.
				channel_congestion_weightage,
				&score_params->esp_qbss_scoring);

	if (entry->air_time_fraction) {
			/* Convert 0-255 range to percentage */
			est_air_time_percentage =
				entry->air_time_fraction *
					SCM_MAX_CHANNEL_WEIGHT;
			est_air_time_percentage =
				qdf_do_div(est_air_time_percentage,
					   SCM_MAX_ESTIMATED_AIR_TIME_FRACTION);
			/*
			 * Calculate channel congestion from estimated air time
			 * fraction.
			 */
			congestion = SCM_MAX_CHANNEL_UTILIZATION -
					est_air_time_percentage;
	} else if (entry->qbss_chan_load) {
			ap_load = (entry->qbss_chan_load *
					BEST_CANDIDATE_MAX_WEIGHT);
			/*
			 * Calculate ap_load in % from qbss channel load from
			 * 0-255 range
			 */
			congestion = qdf_do_div(ap_load, MAX_AP_LOAD);
	} else {
		return score_params->weight_cfg.channel_congestion_weightage *
			   WLAN_GET_SCORE_PERCENTAGE(
			   score_params->esp_qbss_scoring.score_pcnt3_to_0,
			   SCM_SCORE_INDEX_0);
	}

	window_size = BEST_CANDIDATE_MAX_WEIGHT /
			score_params->esp_qbss_scoring.num_slot;

	/* Desired values are from 1 to 15, as 0 is for not present. so do +1 */
	index = qdf_do_div(congestion, window_size) + 1;

	if (index > score_params->esp_qbss_scoring.num_slot)
		index = score_params->esp_qbss_scoring.num_slot;

	return scm_get_score_for_index(index, score_params->weight_cfg.
				       channel_congestion_weightage,
				       &score_params->esp_qbss_scoring);
}

/**
 * scm_calculate_nss_score () - Calculate congestion score
 * @psoc: psoc ptr
 * @score_config: scoring config
 * @ap_nss: ap nss
 * @prorated_pct: prorated % to return dependent on RSSI
 *
 * Return : nss score
 */
static int32_t scm_calculate_nss_score(struct wlan_objmgr_psoc *psoc,
	struct scoring_config *score_config, uint8_t ap_nss,
	uint8_t prorated_pct)
{
	uint8_t nss;
	uint8_t score_pct;
	uint8_t sta_nss;

	sta_nss = score_config->nss;

#ifdef WLAN_POLICY_MGR_ENABLE
	if (policy_mgr_is_current_hwmode_dbs(psoc))
		sta_nss--;
#endif
	nss = ap_nss;
	if (sta_nss < nss)
		nss = sta_nss;

	if (nss == 4)
		score_pct = WLAN_GET_SCORE_PERCENTAGE(
				score_config->nss_weight_per_index,
				SCM_NSS_4x4_INDEX);
	else if (nss == 3)
		score_pct = WLAN_GET_SCORE_PERCENTAGE(
				score_config->nss_weight_per_index,
				SCM_NSS_3x3_INDEX);
	else if (nss == 2)
		score_pct = WLAN_GET_SCORE_PERCENTAGE(
				score_config->nss_weight_per_index,
				SCM_NSS_2x2_INDEX);
	else
		score_pct = WLAN_GET_SCORE_PERCENTAGE(
				score_config->nss_weight_per_index,
				SCM_NSS_1x1_INDEX);

	return (score_config->weight_cfg.nss_weightage * score_pct *
		prorated_pct) / BEST_CANDIDATE_MAX_WEIGHT;
}

/**
 * scm_calculate_oce_wan_score () - Calculate oce wan score
 * @entry: bss information
 * @score_params: bss score params
 *
 * Return : oce wan score
 */
static int32_t scm_calculate_oce_wan_score(
	struct scan_cache_entry *entry,
	struct scoring_config *score_params)
{
	uint32_t window_size;
	uint8_t index;
	struct oce_reduced_wan_metrics wan_metrics;
	uint8_t *mbo_oce_ie;

	if (!score_params->oce_wan_scoring.num_slot)
		return 0;

	if (score_params->oce_wan_scoring.num_slot >
	    SCM_SCORE_MAX_INDEX)
		score_params->oce_wan_scoring.num_slot =
			SCM_SCORE_MAX_INDEX;

	window_size = SCM_MAX_OCE_WAN_DL_CAP/
			score_params->oce_wan_scoring.num_slot;
	mbo_oce_ie = util_scan_entry_mbo_oce(entry);
	if (wlan_parse_oce_reduced_wan_metrics_ie(mbo_oce_ie,
	    &wan_metrics)) {
		scm_err("downlink_av_cap %d", wan_metrics.downlink_av_cap);
		/* Desired values are from 1 to 15, as 0 is for not present.*/
		index = qdf_do_div(wan_metrics.downlink_av_cap,
				   window_size) + 1;
	} else {
		index = SCM_SCORE_INDEX_0;
	}

	if (index > score_params->oce_wan_scoring.num_slot)
		index = score_params->oce_wan_scoring.num_slot;

	return scm_get_score_for_index(index,
			score_params->weight_cfg.oce_wan_weightage,
			&score_params->oce_wan_scoring);
}

int scm_calculate_bss_score(struct wlan_objmgr_psoc *psoc,
		struct scan_default_params *params,
		struct scan_cache_entry *entry,
		int pcl_chan_weight)
{
	int32_t score = 0;
	int32_t rssi_score = 0;
	int32_t pcl_score = 0;
	int32_t ht_score = 0;
	int32_t vht_score = 0;
	int32_t he_score = 0;
	int32_t bandwidth_score = 0;
	int32_t beamformee_score = 0;
	int32_t band_score = 0;
	int32_t nss_score = 0;
	int32_t congestion_score = 0;
	int32_t oce_wan_score = 0;
	uint8_t prorated_pcnt;
	bool is_vht = false;
	int8_t good_rssi_threshold;
	int8_t rssi_pref_5g_rssi_thresh;
	bool same_bucket = false;
	bool ap_su_beam_former = false;
	struct wlan_ie_vhtcaps *vht_cap;
	struct scoring_config *score_config;
	struct weight_config *weight_config;
	struct wlan_scan_obj *scan_obj;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj) {
		scm_err("scan_obj is NULL");
		return 0;
	}

	score_config = &scan_obj->scan_def.score_config;
	weight_config = &score_config->weight_cfg;

	rssi_score = scm_calculate_rssi_score(&score_config->rssi_score,
			entry->rssi_raw, weight_config->rssi_weightage);
	score += rssi_score;

	pcl_score = scm_calculate_pcl_score(pcl_chan_weight,
					    weight_config->pcl_weightage);
	score += pcl_score;

	prorated_pcnt = scm_roam_calculate_prorated_pcnt_by_rssi(
				&score_config->rssi_score, entry->rssi_raw,
				weight_config->rssi_weightage);
	/* If device and AP supports HT caps, extra 10% score will be added */
	if (score_config->ht_cap && entry->ie_list.htcap)
		ht_score = prorated_pcnt *
				weight_config->ht_caps_weightage;
	score += ht_score;

	if (WLAN_CHAN_IS_2GHZ(entry->channel.chan_idx)) {
		if (score_config->vht_24G_cap)
			is_vht = true;
	} else if (score_config->vht_cap) {
		is_vht = true;
	}
	/*
	 * If device and AP supports VHT caps, Extra 6% score will
	 * be added to score
	 */
	if (is_vht && entry->ie_list.vhtcap)
		vht_score = prorated_pcnt *
				 weight_config->vht_caps_weightage;
	score += vht_score;

	if (score_config->he_cap && entry->ie_list.hecap)
		he_score = prorated_pcnt *
				 weight_config->he_caps_weightage;
	score += he_score;

	bandwidth_score = scm_calculate_bandwidth_score(entry, score_config,
							prorated_pcnt);
	score += bandwidth_score;

	good_rssi_threshold =
		score_config->rssi_score.good_rssi_threshold * (-1);
	rssi_pref_5g_rssi_thresh =
		score_config->rssi_score.rssi_pref_5g_rssi_thresh * (-1);
	if (entry->rssi_raw < good_rssi_threshold)
		same_bucket = scm_rssi_is_same_bucket(good_rssi_threshold,
				entry->rssi_raw, rssi_pref_5g_rssi_thresh,
				score_config->rssi_score.bad_rssi_bucket_size);

	vht_cap = (struct wlan_ie_vhtcaps *) util_scan_entry_vhtcap(entry);
	if (vht_cap && vht_cap->su_beam_former)
		ap_su_beam_former = true;
	if (is_vht && ap_su_beam_former &&
	    (entry->rssi_raw > rssi_pref_5g_rssi_thresh) && !same_bucket)
		beamformee_score = BEST_CANDIDATE_MAX_WEIGHT *
				weight_config->beamforming_cap_weightage;
	score += beamformee_score;

	/*
	 * If AP is on 5Ghz channel , extra weigtage is added to BSS score.
	 * if RSSI is greater tha 5g rssi threshold or fall in same bucket.
	 * else give weigtage to 2.4 GH.
	 */
	if ((entry->rssi_raw > rssi_pref_5g_rssi_thresh) && !same_bucket) {
		if (WLAN_CHAN_IS_2GHZ(entry->channel.chan_idx))
			band_score = weight_config->chan_band_weightage *
					WLAN_GET_SCORE_PERCENTAGE(
					score_config->band_weight_per_index,
					SCM_BAND_5G_INDEX);
	} else if (WLAN_CHAN_IS_2GHZ(entry->channel.chan_idx)) {
		band_score = weight_config->chan_band_weightage *
					WLAN_GET_SCORE_PERCENTAGE(
					score_config->band_weight_per_index,
					SCM_BAND_2G_INDEX);
	}
	score += band_score;

	congestion_score = scm_calculate_congestion_score(entry, score_config);
	score += congestion_score;

	/*
	 * If station support nss as 2*2 but AP support NSS as 1*1,
	 * this AP will be given half weight compare to AP which are having
	 * NSS as 2*2.
	 */
	nss_score = scm_calculate_nss_score(psoc, score_config, entry->nss,
					    prorated_pcnt);
	score += nss_score;

	oce_wan_score = scm_calculate_oce_wan_score(entry, score_config);
	score += oce_wan_score;

	scm_debug("Self Cap: HT %d VHT %d HE %d VHT_24Ghz %d BF cap %d cb_mode_24g %d cb_mode_5G %d NSS %d",
		  score_config->ht_cap, score_config->vht_cap,
		  score_config->he_cap,  score_config->vht_24G_cap,
		  score_config->beamformee_cap, score_config->cb_mode_24G,
		  score_config->cb_mode_5G, score_config->nss);

	scm_debug("Candidate (BSSID: %pM Chan %d) Cap:: rssi=%d HT=%d VHT=%d HE %d su beamformer %d phymode=%d  air time fraction %d qbss load %d NSS %d",
		  entry->bssid.bytes, entry->channel.chan_idx,
		  entry->rssi_raw, util_scan_entry_htcap(entry) ? 1 : 0,
		  util_scan_entry_vhtcap(entry) ? 1 : 0,
		  util_scan_entry_hecap(entry) ? 1 : 0, ap_su_beam_former,
		  entry->phy_mode, entry->air_time_fraction,
		  entry->qbss_chan_load, entry->nss);

	scm_debug("Candidate Scores : prorated_pcnt %d rssi %d pcl %d ht %d vht %d he %d beamformee %d bw %d band %d congestion %d nss %d oce wan %d TOTAL score %d",
		  prorated_pcnt, rssi_score, pcl_score, ht_score, vht_score,
		  he_score, beamformee_score, bandwidth_score, band_score,
		  congestion_score, nss_score, oce_wan_score, score);

	entry->bss_score = score;
	return score;
}

/**
 * scm_is_open_security() - Check if scan entry support open security
 * @filter: scan filter
 * @db_entry: db entry
 * @security: matched security.
 *
 * Return: true if open security else false
 */
static bool scm_is_open_security(struct scan_filter *filter,
	struct scan_cache_entry *db_entry,
	struct security_info *security)
{
	bool match = false;
	int i;

	if (db_entry->cap_info.wlan_caps.privacy)
		return false;

	/* Check MC cipher and Auth type requested. */
	for (i = 0; i < filter->num_of_mc_enc_type; i++) {
		if (WLAN_ENCRYPT_TYPE_NONE ==
			filter->mc_enc_type[i]) {
			security->mc_enc =
				filter->mc_enc_type[i];
			match = true;
			break;
		}
	}
	if (!match && filter->num_of_mc_enc_type)
		return match;

	match = false;
	/* Check Auth list. It should contain AuthOpen. */
	for (i = 0; i < filter->num_of_auth; i++) {
		if ((WLAN_AUTH_TYPE_OPEN_SYSTEM ==
			filter->auth_type[i]) ||
			(WLAN_AUTH_TYPE_AUTOSWITCH ==
			filter->auth_type[i])) {
			security->auth_type =
				WLAN_AUTH_TYPE_OPEN_SYSTEM;
			match = true;
			break;
		}
	}

	return match;
}

/**
 * scm_is_cipher_match() - Check if cipher match the cipher list
 * @cipher_list: cipher list to match
 * @num_cipher: number of cipher in cipher list
 * @cipher_to_match: cipher to found in cipher list
 *
 * Return: true if open security else false
 */
static bool scm_is_cipher_match(
	uint32_t *cipher_list,
	uint16_t num_cipher, uint32_t cipher_to_match)
{
	int i;
	bool match = false;

	for (i = 0; i < num_cipher ; i++) {
		match = (cipher_list[i] == cipher_to_match);
		if (match)
			break;
	}

	return match;
}

/**
 * scm_get_cipher_suite_type() - get cypher suite type from enc type
 * @enc: enc type
 *
 * Return: cypher suite type
 */
static uint8_t scm_get_cipher_suite_type(enum wlan_enc_type enc)
{
	uint8_t cipher_type;

	switch (enc) {
	case WLAN_ENCRYPT_TYPE_WEP40:
	case WLAN_ENCRYPT_TYPE_WEP40_STATICKEY:
		cipher_type = WLAN_CSE_WEP40;
		break;
	case WLAN_ENCRYPT_TYPE_WEP104:
	case WLAN_ENCRYPT_TYPE_WEP104_STATICKEY:
		cipher_type = WLAN_CSE_WEP104;
		break;
	case WLAN_ENCRYPT_TYPE_TKIP:
		cipher_type = WLAN_CSE_TKIP;
		break;
	case WLAN_ENCRYPT_TYPE_AES:
		cipher_type = WLAN_CSE_CCMP;
		break;
	case WLAN_ENCRYPT_TYPE_AES_GCMP:
		cipher_type = WLAN_CSE_GCMP_128;
		break;
	case WLAN_ENCRYPT_TYPE_AES_GCMP_256:
		cipher_type = WLAN_CSE_GCMP_256;
		break;
	case WLAN_ENCRYPT_TYPE_NONE:
		cipher_type = WLAN_CSE_NONE;
		break;
	case WLAN_ENCRYPT_TYPE_WPI:
		cipher_type = WLAN_WAI_CERT_OR_SMS4;
		break;
	default:
		cipher_type = WLAN_CSE_RESERVED;
		break;
	}

	return cipher_type;
}

/**
 * scm_is_wep_security() - Check if scan entry support WEP security
 * @filter: scan filter
 * @db_entry: db entry
 * @security: matched security.
 *
 * Return: true if WEP security else false
 */
static bool scm_is_wep_security(struct scan_filter *filter,
	struct scan_cache_entry *db_entry,
	struct security_info *security)
{
	int i;
	bool match = false;
	enum wlan_auth_type neg_auth = WLAN_AUTH_TYPE_OPEN_SYSTEM;
	enum wlan_enc_type neg_mccipher = WLAN_ENCRYPT_TYPE_NONE;

	/* If privacy bit is not set, consider no match */
	if (!db_entry->cap_info.wlan_caps.privacy)
		return false;

	for (i = 0; i < filter->num_of_mc_enc_type; i++) {
		switch (filter->mc_enc_type[i]) {
		case WLAN_ENCRYPT_TYPE_WEP40_STATICKEY:
		case WLAN_ENCRYPT_TYPE_WEP104_STATICKEY:
		case WLAN_ENCRYPT_TYPE_WEP40:
		case WLAN_ENCRYPT_TYPE_WEP104:
			/*
			 * Multicast list may contain WEP40/WEP104.
			 * Check whether it matches UC.
			 */
			if (security->uc_enc ==
			   filter->mc_enc_type[i]) {
				match = true;
				neg_mccipher =
				   filter->mc_enc_type[i];
			}
			break;
		default:
			match = false;
			break;
		}
		if (match)
			break;
	}

	if (!match)
		return match;

	for (i = 0; i < filter->num_of_auth; i++) {
		switch (filter->auth_type[i]) {
		case WLAN_AUTH_TYPE_OPEN_SYSTEM:
		case WLAN_AUTH_TYPE_SHARED_KEY:
		case WLAN_AUTH_TYPE_AUTOSWITCH:
			match = true;
			neg_auth = filter->auth_type[i];
			break;
		default:
			match = false;
		}
		if (match)
			break;
	}

	if (!match)
		return match;

	/*
	 * In case of WPA / WPA2, check whether it supports WEP as well.
	 * Prepare the encryption type for WPA/WPA2 functions
	 */
	if (security->uc_enc == WLAN_ENCRYPT_TYPE_WEP40_STATICKEY)
		security->uc_enc = WLAN_ENCRYPT_TYPE_WEP40;
	else if (security->uc_enc == WLAN_ENCRYPT_TYPE_WEP104)
		security->uc_enc = WLAN_ENCRYPT_TYPE_WEP104;

	/* else we can use the encryption type directly */
	if (util_scan_entry_wpa(db_entry)) {
		struct wlan_wpa_ie wpa = {0};
		uint8_t cipher_type;

		cipher_type =
			scm_get_cipher_suite_type(security->uc_enc);
		wlan_parse_wpa_ie(
			util_scan_entry_wpa(db_entry), &wpa);

		match = scm_is_cipher_match(&wpa.mc_cipher,
				  1, WLAN_WPA_SEL(cipher_type));
	}
	if (!match && util_scan_entry_rsn(db_entry)) {
		struct wlan_rsn_ie rsn = {0};
		uint8_t cipher_type;

		cipher_type =
			scm_get_cipher_suite_type(security->uc_enc);

		wlan_parse_rsn_ie(
			   util_scan_entry_rsn(db_entry), &rsn);
		match = scm_is_cipher_match(&rsn.gp_cipher_suite,
				  1, WLAN_RSN_SEL(cipher_type));
	}


	if (match && security) {
		security->auth_type = neg_auth;
		security->mc_enc = neg_mccipher;
	}

	return match;
}

/**
 * scm_check_pmf_match() - Check PMF security of entry match filter
 * @filter: scan filter
 * @rsn: rsn IE of the scan entry
 *
 * Return: true if PMF security match else false
 */
static bool
scm_check_pmf_match(struct scan_filter *filter,
	struct wlan_rsn_ie *rsn)
{
	enum wlan_pmf_cap ap_pmf_cap = WLAN_PMF_DISABLED;

	if (rsn->cap & RSN_CAP_MFP_CAPABLE)
		ap_pmf_cap = WLAN_PMF_CAPABLE;
	if (rsn->cap & RSN_CAP_MFP_REQUIRED)
		ap_pmf_cap = WLAN_PMF_REQUIRED;

	if ((filter->pmf_cap == WLAN_PMF_REQUIRED) &&
		(ap_pmf_cap == WLAN_PMF_DISABLED))
		return false;
	else if ((filter->pmf_cap == WLAN_PMF_DISABLED) &&
		(ap_pmf_cap == WLAN_PMF_REQUIRED))
		return false;

	return true;
}

/**
 * scm_is_rsn_security() - Check if scan entry support RSN security
 * @filter: scan filter
 * @db_entry: db entry
 * @security: matched security.
 *
 * Return: true if RSN security else false
 */
static bool scm_is_rsn_security(struct scan_filter *filter,
	struct scan_cache_entry *db_entry,
	struct security_info *security)
{
	int i;
	uint8_t cipher_type;
	bool match = false;
	enum wlan_auth_type neg_auth = WLAN_NUM_OF_SUPPORT_AUTH_TYPE;
	enum wlan_enc_type neg_mccipher = WLAN_ENCRYPT_TYPE_NONE;
	struct wlan_rsn_ie rsn = {0};

	if (!util_scan_entry_rsn(db_entry))
		return false;

	wlan_parse_rsn_ie(
		   util_scan_entry_rsn(db_entry), &rsn);

	cipher_type =
		scm_get_cipher_suite_type(security->uc_enc);
	match = scm_is_cipher_match(rsn.pwise_cipher_suites,
		rsn.pwise_cipher_count, WLAN_RSN_SEL(cipher_type));
	if (!match)
		return false;

	for (i = 0; i < filter->num_of_mc_enc_type; i++) {
		cipher_type =
			scm_get_cipher_suite_type(
				filter->mc_enc_type[i]);
		match = scm_is_cipher_match(&rsn.gp_cipher_suite,
			   1, WLAN_RSN_SEL(cipher_type));
		if (match)
			break;
	}
	if (!match)
		return false;

	neg_mccipher = filter->mc_enc_type[i];

	/* Initializing with false as it has true value already */
	match = false;
	for (i = 0; i < filter->num_of_auth; i++) {
		/*
		 * Ciphers are supported, Match authentication algorithm and
		 * pick first matching authtype.
		 */
		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count,
		   WLAN_RSN_SEL(WLAN_AKM_FILS_FT_SHA384))) {
			if (WLAN_AUTH_TYPE_FT_FILS_SHA384 ==
			   filter->auth_type[i]) {
				neg_auth = WLAN_AUTH_TYPE_FT_FILS_SHA384;
				match = true;
				break;
			}
		}
		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count,
		   WLAN_RSN_SEL(WLAN_AKM_FILS_FT_SHA256))) {
			if (WLAN_AUTH_TYPE_FT_FILS_SHA256 ==
			   filter->auth_type[i]) {
				neg_auth = WLAN_AUTH_TYPE_FT_FILS_SHA256;
				match = true;
				break;
			}
		}
		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count,
		   WLAN_RSN_SEL(WLAN_AKM_FILS_SHA384))) {
			if (WLAN_AUTH_TYPE_FILS_SHA384 ==
			   filter->auth_type[i]) {
				neg_auth = WLAN_AUTH_TYPE_FILS_SHA384;
				match = true;
				break;
			}
		}
		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count,
		   WLAN_RSN_SEL(WLAN_AKM_FILS_SHA256))) {
			if (WLAN_AUTH_TYPE_FILS_SHA256 ==
			   filter->auth_type[i]) {
				neg_auth = WLAN_AUTH_TYPE_FILS_SHA256;
				match = true;
				break;
			}
		}
		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count, WLAN_RSN_DPP_AKM)) {
			if (WLAN_AUTH_TYPE_DPP_RSN ==
			   filter->auth_type[i]) {
				neg_auth = WLAN_AUTH_TYPE_DPP_RSN;
				match = true;
				break;
			}
		}
		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count,
		   WLAN_RSN_SEL(WLAN_AKM_OWE))) {
			if (WLAN_AUTH_TYPE_OWE ==
			   filter->auth_type[i]) {
				neg_auth = WLAN_AUTH_TYPE_OWE;
				match = true;
				break;
			}
		}
		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count,
		   WLAN_RSN_SEL(WLAN_AKM_FT_IEEE8021X))) {
			if (WLAN_AUTH_TYPE_FT_RSN ==
			   filter->auth_type[i]) {
				neg_auth = WLAN_AUTH_TYPE_FT_RSN;
				match = true;
				break;
			}
		}

		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count,
		   WLAN_RSN_SEL(WLAN_AKM_FT_PSK))) {
			if (WLAN_AUTH_TYPE_FT_RSN_PSK ==
			   filter->auth_type[i]) {
				neg_auth = WLAN_AUTH_TYPE_FT_RSN_PSK;
				match = true;
				break;
			}
		}
		/* ESE only supports 802.1X.  No PSK. */
		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count,
		   WLAN_RSN_CCKM_AKM)) {
			if (WLAN_AUTH_TYPE_CCKM_RSN ==
			   filter->auth_type[i]) {
				neg_auth = WLAN_AUTH_TYPE_CCKM_RSN;
				match = true;
				break;
			}
		}
		/* RSN */
		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count,
		   WLAN_RSN_SEL(WLAN_AKM_IEEE8021X))) {
			if (WLAN_AUTH_TYPE_RSN ==
			   filter->auth_type[i]) {
				neg_auth = WLAN_AUTH_TYPE_RSN;
				match = true;
				break;
			}
		}
		/* TKIP */
		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count,
		   WLAN_RSN_SEL(WLAN_AKM_PSK))) {
			if (WLAN_AUTH_TYPE_RSN_PSK ==
			   filter->auth_type[i]) {
				neg_auth = WLAN_AUTH_TYPE_RSN_PSK;
				match = true;
				break;
			}
		}
		/* SHA256 */
		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count,
		   WLAN_RSN_SEL(WLAN_AKM_SHA256_PSK))) {
			if (WLAN_AUTH_TYPE_RSN_PSK_SHA256 ==
			   filter->auth_type[i]) {
				neg_auth =
					WLAN_AUTH_TYPE_RSN_PSK_SHA256;
				match = true;
				break;
			}
		}
		/* 8021X SHA256 */
		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count,
		   WLAN_RSN_SEL(WLAN_AKM_SHA256_IEEE8021X))) {
			if (WLAN_AUTH_TYPE_RSN_8021X_SHA256 ==
			   filter->auth_type[i]) {
				neg_auth =
					WLAN_AUTH_TYPE_RSN_8021X_SHA256;
				match = true;
				break;
			}
		}
	}

	if (!match)
		return false;
	match = scm_check_pmf_match(filter, &rsn);

	if (match && security) {
		security->auth_type = neg_auth;
		security->mc_enc = neg_mccipher;
	}

	return match;
}

/**
 * scm_is_wpa_security() - Check if scan entry support WPA security
 * @filter: scan filter
 * @db_entry: db entry
 * @security: matched security.
 *
 * Return: true if WPA security else false
 */
static bool scm_is_wpa_security(struct scan_filter *filter,
	struct scan_cache_entry *db_entry,
	struct security_info *security)
{
	int i;
	uint8_t cipher_type;
	bool match = false;
	enum wlan_auth_type neg_auth = WLAN_NUM_OF_SUPPORT_AUTH_TYPE;
	enum wlan_enc_type neg_mccipher = WLAN_ENCRYPT_TYPE_NONE;
	struct wlan_wpa_ie wpa = {0};

	if (!util_scan_entry_wpa(db_entry))
		return false;

	wlan_parse_wpa_ie(util_scan_entry_wpa(db_entry), &wpa);

	cipher_type =
		scm_get_cipher_suite_type(security->uc_enc);
	match = scm_is_cipher_match(wpa.uc_ciphers,
		wpa.uc_cipher_count, WLAN_WPA_SEL(cipher_type));
	if (!match)
		return false;

	for (i = 0; i < filter->num_of_mc_enc_type; i++) {
		cipher_type =
		  scm_get_cipher_suite_type(
		  filter->mc_enc_type[i]);
		match = scm_is_cipher_match(&wpa.mc_cipher,
			   1, WLAN_WPA_SEL(cipher_type));
		if (match)
			break;
	}
	if (!match)
		return false;
	neg_mccipher = filter->mc_enc_type[i];

	/* Initializing with false as it has true value already */
	match = false;
	for (i = 0; i < filter->num_of_auth; i++) {
		/*
		 * Ciphers are supported, Match authentication algorithm and
		 * pick first matching authtype.
		 */
		/**/
		if (scm_is_cipher_match(wpa.auth_suites,
		   wpa.auth_suite_count,
		   WLAN_WPA_SEL(WLAN_AKM_IEEE8021X))) {
			if (WLAN_AUTH_TYPE_WPA ==
			   filter->auth_type[i]) {
				neg_auth = WLAN_AUTH_TYPE_WPA;
				match = true;
				break;
			}
		}
		if (scm_is_cipher_match(wpa.auth_suites,
		   wpa.auth_suite_count,
		   WLAN_WPA_SEL(WLAN_AKM_PSK))) {
			if (WLAN_AUTH_TYPE_WPA_PSK ==
			   filter->auth_type[i]) {
				neg_auth = WLAN_AUTH_TYPE_WPA_PSK;
				match = true;
				break;
			}
		}
		if (scm_is_cipher_match(wpa.auth_suites,
		   wpa.auth_suite_count,
		   WLAN_WPA_CCKM_AKM)) {
			if (WLAN_AUTH_TYPE_CCKM_WPA ==
			   filter->auth_type[i]) {
				neg_auth = WLAN_AUTH_TYPE_CCKM_WPA;
				match = true;
				break;
			}
		}
	}

	if (match && security) {
		security->auth_type = neg_auth;
		security->mc_enc = neg_mccipher;
	}

	return match;
}

/**
 * scm_is_wapi_security() - Check if scan entry support WAPI security
 * @filter: scan filter
 * @db_entry: db entry
 * @security: matched security.
 *
 * Return: true if WAPI security else false
 */
static bool scm_is_wapi_security(struct scan_filter *filter,
	struct scan_cache_entry *db_entry,
	struct security_info *security)
{
	int i;
	uint8_t cipher_type;
	bool match = false;
	enum wlan_auth_type neg_auth = WLAN_NUM_OF_SUPPORT_AUTH_TYPE;
	enum wlan_enc_type neg_mccipher = WLAN_ENCRYPT_TYPE_NONE;
	struct wlan_wapi_ie wapi = {0};

	if (!util_scan_entry_wapi(db_entry))
		return false;

	wlan_parse_wapi_ie(
		   util_scan_entry_wapi(db_entry), &wapi);

	cipher_type =
		scm_get_cipher_suite_type(security->uc_enc);
	match = scm_is_cipher_match(wapi.uc_cipher_suites,
		wapi.uc_cipher_count, WLAN_WAPI_SEL(cipher_type));
	if (!match)
		return false;

	for (i = 0; i < filter->num_of_mc_enc_type; i++) {
		cipher_type =
		  scm_get_cipher_suite_type(
		  filter->mc_enc_type[i]);
		match = scm_is_cipher_match(&wapi.mc_cipher_suite,
				  1, WLAN_WAPI_SEL(cipher_type));
		if (match)
			break;
	}
	if (!match)
		return false;
	neg_mccipher = filter->mc_enc_type[i];

	if (scm_is_cipher_match(wapi.akm_suites,
	   wapi.akm_suite_count,
	   WLAN_WAPI_SEL(WLAN_WAI_CERT_OR_SMS4)))
		neg_auth =
			WLAN_AUTH_TYPE_WAPI_WAI_CERTIFICATE;
	else if (scm_is_cipher_match(wapi.akm_suites,
	   wapi.akm_suite_count, WLAN_WAPI_SEL(WLAN_WAI_PSK)))
		neg_auth = WLAN_AUTH_TYPE_WAPI_WAI_PSK;
	else
		return false;

	match = false;
	for (i = 0; i < filter->num_of_auth; i++) {
		if (filter->auth_type[i] == neg_auth) {
			match = true;
			break;
		}
	}

	if (match && security) {
		security->auth_type = neg_auth;
		security->mc_enc = neg_mccipher;
	}

	return match;
}

/**
 * scm_is_def_security() - Check if any security in filter match
 * @filter: scan filter
 * @db_entry: db entry
 * @security: matched security.
 *
 * Return: true if any security else false
 */
static bool scm_is_def_security(struct scan_filter *filter,
	struct scan_cache_entry *db_entry,
	struct security_info *security)
{
	bool match_any = false;
	bool match = true;

	/* It is allowed to match anything. Try the more secured ones first. */
	/* Check AES first */
	security->uc_enc = WLAN_ENCRYPT_TYPE_AES;
	match_any = scm_is_rsn_security(filter,
			    db_entry, security);
	if (!match_any) {
		/* Check TKIP */
		security->uc_enc = WLAN_ENCRYPT_TYPE_TKIP;
		match_any = scm_is_rsn_security(filter,
			    db_entry, security);
	}

	if (!match_any) {
		/* Check WAPI */
		security->uc_enc = WLAN_ENCRYPT_TYPE_WPI;
		match_any = scm_is_wapi_security(filter,
			    db_entry, security);
	}

	if (match_any)
		return match;

	security->uc_enc = WLAN_ENCRYPT_TYPE_WEP104;
	if (scm_is_wep_security(filter,
	   db_entry, security))
		return true;
	security->uc_enc = WLAN_ENCRYPT_TYPE_WEP40;
	if (scm_is_wep_security(filter,
	   db_entry, security))
		return true;
	security->uc_enc = WLAN_ENCRYPT_TYPE_WEP104_STATICKEY;
	if (scm_is_wep_security(filter,
	   db_entry, security))
		return true;
	security->uc_enc = WLAN_ENCRYPT_TYPE_WEP40_STATICKEY;
	if (scm_is_wep_security(filter,
	   db_entry, security))
		return true;

	/* It must be open and no enc */
	if (db_entry->cap_info.wlan_caps.privacy)
		return false;

	security->auth_type = WLAN_AUTH_TYPE_OPEN_SYSTEM;
	security->mc_enc = WLAN_ENCRYPT_TYPE_NONE;
	security->uc_enc = WLAN_ENCRYPT_TYPE_NONE;

	return match;
}

/**
 * scm_is_fils_config_match() - Check if FILS config matches
 * @filter: scan filter
 * @db_entry: db entry
 *
 * Return: true if FILS config matches else false
 */
static bool scm_is_fils_config_match(struct scan_filter *filter,
	struct scan_cache_entry *db_entry)
{
	int i;
	struct fils_indication_ie *indication_ie;
	uint8_t *data;

	if (!filter->fils_scan_filter.realm_check)
		return true;

	if (!db_entry->ie_list.fils_indication)
		return false;


	indication_ie =
		(struct fils_indication_ie *) db_entry->ie_list.fils_indication;

	data = indication_ie->variable_data;
	if (indication_ie->is_cache_id_present)
		data += CACHE_IDENTIFIER_LEN;

	if (indication_ie->is_hessid_present)
		data += HESSID_LEN;

	for (i = 1; i <= indication_ie->realm_identifiers_cnt; i++) {
		if (!qdf_mem_cmp(filter->fils_scan_filter.fils_realm,
				 data, REAM_HASH_LEN))
			return true;
		/* Max realm count reached */
		if (indication_ie->realm_identifiers_cnt == i)
			break;
		else
			data = data + REAM_HASH_LEN;
	}

	return false;
}

/**
 * scm_is_security_match() - Check if security in filter match
 * @filter: scan filter
 * @db_entry: db entry
 * @security: matched security.
 *
 * Return: true if security match else false
 */
static bool scm_is_security_match(struct scan_filter *filter,
	struct scan_cache_entry *db_entry,
	struct security_info *security)
{
	int i;
	bool match = false;
	struct security_info local_security = {0};

	if (!filter->num_of_enc_type)
		return true;

	for (i = 0; (i < filter->num_of_enc_type) &&
	    !match; i++) {

		local_security.uc_enc =
			filter->enc_type[i];

		switch (filter->enc_type[i]) {
		case WLAN_ENCRYPT_TYPE_NONE:
			match = scm_is_open_security(filter,
				    db_entry, &local_security);
			break;
		case WLAN_ENCRYPT_TYPE_WEP40_STATICKEY:
		case WLAN_ENCRYPT_TYPE_WEP104_STATICKEY:
		case WLAN_ENCRYPT_TYPE_WEP40:
		case WLAN_ENCRYPT_TYPE_WEP104:
			match = scm_is_wep_security(filter,
				    db_entry, &local_security);
			break;
		case WLAN_ENCRYPT_TYPE_TKIP:
		case WLAN_ENCRYPT_TYPE_AES:
		case WLAN_ENCRYPT_TYPE_AES_GCMP:
		case WLAN_ENCRYPT_TYPE_AES_GCMP_256:
			/* First check if there is a RSN match */
			match = scm_is_rsn_security(filter,
				    db_entry, &local_security);
			/* If not RSN, then check WPA match */
			if (!match)
				match = scm_is_wpa_security(filter,
				    db_entry, &local_security);
			break;
		case WLAN_ENCRYPT_TYPE_WPI:/* WAPI */
			match = scm_is_wapi_security(filter,
				    db_entry, &local_security);
			break;
		case WLAN_ENCRYPT_TYPE_ANY:
		default:
			match  = scm_is_def_security(filter,
				    db_entry, &local_security);
			break;
		}
	}

	if (match && security)
		qdf_mem_copy(security,
			&local_security, sizeof(*security));

	return match;
}

bool scm_filter_match(struct wlan_objmgr_psoc *psoc,
	struct scan_cache_entry *db_entry,
	struct scan_filter *filter,
	struct security_info *security)
{
	int i;
	bool match = false;
	struct roam_filter_params *roam_params;
	struct scan_default_params *def_param;

	def_param = wlan_scan_psoc_get_def_params(psoc);
	roam_params = &def_param->roam_params;

	if (filter->p2p_results && !db_entry->is_p2p)
		return false;

	for (i = 0; i < roam_params->num_bssid_avoid_list; i++)
		if (qdf_is_macaddr_equal(&roam_params->bssid_avoid_list[i],
		   &db_entry->bssid))
			return false;

	match = false;
	if (db_entry->ssid.length) {
		for (i = 0; i < filter->num_of_ssid; i++) {
			if (util_is_ssid_match(&filter->ssid_list[i],
			   &db_entry->ssid)) {
				match = true;
				break;
			}
		}
	}
	if (!match && filter->num_of_ssid)
		return false;

	match = false;
	/* TO do Fill p2p MAC*/
	for (i = 0; i < filter->num_of_bssid; i++) {
		if (util_is_bssid_match(&filter->bssid_list[i],
		   &db_entry->bssid)) {
			match = true;
			break;
		}
		/* TODO match p2p mac */
	}
	if (!match && filter->num_of_bssid)
		return false;

	match = false;
	for (i = 0; i < filter->num_of_channels; i++) {
		if (!filter->channel_list[i] || (
		   (filter->channel_list[i] ==
		   db_entry->channel.chan_idx))) {
			match = true;
			break;
		}
	}

	if (!match && filter->num_of_channels)
		return false;

	if (filter->rrm_measurement_filter)
		return true;

	/* TODO match phyMode */

	if (!filter->ignore_auth_enc_type &&
	   !scm_is_security_match(filter,
	   db_entry, security))
		return false;

	if (!util_is_bss_type_match(filter->bss_type,
	   db_entry->cap_info))
		return false;

	/* TODO match rate set */

	if (filter->only_wmm_ap &&
	   !db_entry->ie_list.wmeinfo &&
	   !db_entry->ie_list.wmeparam)
		return false;

	/* Match realm */
	if (!scm_is_fils_config_match(filter, db_entry))
		return false;

	if (!util_country_code_match(filter->country,
	   db_entry->ie_list.country))
		return false;

	if (!util_mdie_match(filter->mobility_domain,
	   (struct rsn_mdie *)db_entry->ie_list.mdie))
		return false;

	return true;
}
bool scm_get_pcl_weight_of_channel(int channel_id,
		struct scan_filter *filter,
		int *pcl_chan_weight,
		uint8_t *weight_list)
{
	int i;
	bool found = false;

	if (NULL == filter)
		return found;

	for (i = 0; i < filter->num_of_pcl_channels; i++) {
		if (filter->pcl_channel_list[i] == channel_id) {
			*pcl_chan_weight = filter->pcl_weight_list[i];
			found = true;
			break;
		}
	}
	return found;
}
