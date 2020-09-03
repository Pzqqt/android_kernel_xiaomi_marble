/*
 * Copyright (c) 2017-2020, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: Define bss scoring structures and APIs
 */

#ifndef _WLAN_CM_BSS_SCORE_H
#define _WLAN_CM_BSS_SCORE_H

#include <wlan_scan_utils_api.h>
#include "wlan_reg_services_api.h"

/**
 * struct weight_cfg - weight params to calculate best candidate
 * @rssi_weightage: RSSI weightage
 * @ht_caps_weightage: HT caps weightage
 * @vht_caps_weightage: VHT caps weightage
 * @he_caps_weightage: HE caps weightage
 * @chan_width_weightage: Channel width weightage
 * @chan_band_weightage: Channel band weightage
 * @nss_weightage: NSS weightage
 * @beamforming_cap_weightage: Beamforming caps weightage
 * @pcl_weightage: PCL weightage
 * @channel_congestion_weightage: channel congestion weightage
 * @oce_wan_weightage: OCE WAN metrics weightage
 * @oce_ap_tx_pwr_weightage: OCE AP tx power weigtage
 * @oce_subnet_id_weightage: OCE subnet id weigtage
 * @sae_pk_ap_weightage: SAE-PK AP weigtage
 */
struct weight_cfg {
	uint8_t rssi_weightage;
	uint8_t ht_caps_weightage;
	uint8_t vht_caps_weightage;
	uint8_t he_caps_weightage;
	uint8_t chan_width_weightage;
	uint8_t chan_band_weightage;
	uint8_t nss_weightage;
	uint8_t beamforming_cap_weightage;
	uint8_t pcl_weightage;
	uint8_t channel_congestion_weightage;
	uint8_t oce_wan_weightage;
	uint8_t oce_ap_tx_pwr_weightage;
	uint8_t oce_subnet_id_weightage;
	uint8_t sae_pk_ap_weightage;
};

/**
 * struct rssi_config_score - rssi related params for scoring logic
 * @best_rssi_threshold: RSSI weightage
 * @good_rssi_threshold: HT caps weightage
 * @bad_rssi_threshold: VHT caps weightage
 * @good_rssi_pcnt: HE caps weightage
 * @bad_rssi_pcnt: Channel width weightage
 * @good_rssi_bucket_size: Channel band weightage
 * @bad_rssi_bucket_size: NSS weightage
 * @rssi_pref_5g_rssi_thresh: Beamforming caps weightage
 */
struct rssi_config_score  {
	uint8_t best_rssi_threshold;
	uint8_t good_rssi_threshold;
	uint8_t bad_rssi_threshold;
	uint8_t good_rssi_pcnt;
	uint8_t bad_rssi_pcnt;
	uint8_t good_rssi_bucket_size;
	uint8_t bad_rssi_bucket_size;
	uint8_t rssi_pref_5g_rssi_thresh;
};

/**
 * struct per_slot_score - define % score for differents slots for a
 *                               scoring param.
 * num_slot: number of slots in which the param will be divided.
 *           Max 15. index 0 is used for 'not_present. Num_slot will
 *           equally divide 100. e.g, if num_slot = 4 slot 0 = 0-25%, slot
 *           1 = 26-50% slot 2 = 51-75%, slot 3 = 76-100%
 * score_pcnt3_to_0: Conatins score percentage for slot 0-3
 *             BITS 0-7   :- the scoring pcnt when not present
 *             BITS 8-15  :- SLOT_1
 *             BITS 16-23 :- SLOT_2
 *             BITS 24-31 :- SLOT_3
 * score_pcnt7_to_4: Conatins score percentage for slot 4-7
 *             BITS 0-7   :- SLOT_4
 *             BITS 8-15  :- SLOT_5
 *             BITS 16-23 :- SLOT_6
 *             BITS 24-31 :- SLOT_7
 * score_pcnt11_to_8: Conatins score percentage for slot 8-11
 *             BITS 0-7   :- SLOT_8
 *             BITS 8-15  :- SLOT_9
 *             BITS 16-23 :- SLOT_10
 *             BITS 24-31 :- SLOT_11
 * score_pcnt15_to_12: Conatins score percentage for slot 12-15
 *             BITS 0-7   :- SLOT_12
 *             BITS 8-15  :- SLOT_13
 *             BITS 16-23 :- SLOT_14
 *             BITS 24-31 :- SLOT_15
 */
struct per_slot_score {
	uint8_t num_slot;
	uint32_t score_pcnt3_to_0;
	uint32_t score_pcnt7_to_4;
	uint32_t score_pcnt11_to_8;
	uint32_t score_pcnt15_to_12;
};

/**
 * struct scoring_cfg - Scoring related configuration
 * @weight_cfg: weigtage config for config
 * @rssi_score: Rssi related config for scoring config
 * @esp_qbss_scoring: esp and qbss related scoring config
 * @oce_wan_scoring: oce related scoring config
 * @bandwidth_weight_per_index: BW wight per index
 * @nss_weight_per_index: nss weight per index
 * @band_weight_per_index: band weight per index
 * @is_bssid_hint_priority: True if bssid_hint is given priority
 * @check_assoc_disallowed: Should assoc be disallowed if MBO OCE IE indicate so
 */
struct scoring_cfg {
	struct weight_cfg weight_config;
	struct rssi_config_score rssi_score;
	struct per_slot_score esp_qbss_scoring;
	struct per_slot_score oce_wan_scoring;
	uint32_t bandwidth_weight_per_index;
	uint32_t nss_weight_per_index;
	uint32_t band_weight_per_index;
	bool is_bssid_hint_priority;
	bool check_assoc_disallowed;
};

/**
 * struct pcl_freq_weight_list - pcl freq weight info
 * @num_of_pcl_channels: number of pcl channel
 * @pcl_freq_list: pcl freq list
 * @pcl_weight_list: pcl freq weight list
 */
struct pcl_freq_weight_list {
	uint32_t num_of_pcl_channels;
	uint32_t pcl_freq_list[NUM_CHANNELS];
	uint8_t pcl_weight_list[NUM_CHANNELS];
};

/**
 * enum cm_blacklist_action - action taken by blacklist manager for the bssid
 * @CM_BLM_NO_ACTION: No operation to be taken for the BSSID in the scan list.
 * @CM_BLM_REMOVE: Remove the BSSID from the scan list (AP is blacklisted)
 * @CM_BLM_AVOID: Add the Ap at last of the scan list (AP to Avoid)
 */
enum cm_blacklist_action {
	CM_BLM_NO_ACTION,
	CM_BLM_REMOVE,
	CM_BLM_AVOID,
};

#ifdef FEATURE_BLACKLIST_MGR
enum cm_blacklist_action
wlan_blacklist_action_on_bssid(struct wlan_objmgr_pdev *pdev,
			       struct scan_cache_entry *entry);
#else
static inline enum cm_blacklist_action
wlan_blacklist_action_on_bssid(struct wlan_objmgr_pdev *pdev,
			       struct scan_cache_entry *entry)
{
	return CM_BLM_NO_ACTION;
}
#endif

/**
 * wlan_cm_calculate_bss_score() - calculate bss score for the scan list
 * @pdev: pointer to pdev object
 * @pcl_list: pcl list for scoring
 * @scan_list: scan list, contains the input list and after the
 *             func it will have sorted list
 * @bssid_hint: bssid hint
 *
 * Return: void
 */
void wlan_cm_calculate_bss_score(struct wlan_objmgr_pdev *pdev,
				 struct pcl_freq_weight_list *pcl_lst,
				 qdf_list_t *scan_list,
				 struct qdf_mac_addr *bssid_hint);

/**
 * wlan_cm_init_score_config() - Init score INI and config
 * @psoc: pointer to psoc object
 * @score_cfg: score config
 *
 * Return: void
 */
void wlan_cm_init_score_config(struct wlan_objmgr_psoc *psoc,
			       struct scoring_cfg *score_cfg);

#ifdef CONN_MGR_ADV_FEATURE
/**
 * wlan_cm_set_check_assoc_disallowed() - Set check assoc disallowed param
 * @psoc: pointer to psoc object
 * @value: value to be set
 *
 * Return: void
 */
void wlan_cm_set_check_assoc_disallowed(struct wlan_objmgr_psoc *psoc,
					bool value);

/**
 * wlan_cm_get_check_assoc_disallowed() - get check assoc disallowed param
 * @psoc: pointer to psoc object
 * @value: value to be filled
 *
 * Return: void
 */
void wlan_cm_get_check_assoc_disallowed(struct wlan_objmgr_psoc *psoc,
					bool *value);
#endif
#endif
