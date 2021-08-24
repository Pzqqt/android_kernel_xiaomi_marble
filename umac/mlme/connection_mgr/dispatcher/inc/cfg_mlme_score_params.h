/*
 * Copyright (c) 2012-2021, The Linux Foundation. All rights reserved.
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
 * DOC: This file contains MLME SCORING related CFG/INI Items.
 */

#ifndef __CFG_MLME_SCORE_PARAMS_H
#define __CFG_MLME_SCORE_PARAMS_H

/*
 * <ini>
 * rssi_weightage/RoamAPScore_RSSIWeight - RSSI Weightage to calculate best
 * candidate
 * @Min: 0
 * @Max: 100
 * @Default: 20
 *
 * This ini is used to increase/decrease RSSI weightage in best candidate
 * selection. AP with better RSSI will get more weightage.
 *
 * Related: None
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_RSSI_WEIGHTAGE CFG_INI_UINT( \
	"rssi_weightage RoamAPScore_RSSIWeight", \
	0, \
	100, \
	20, \
	CFG_VALUE_OR_DEFAULT, \
	"RSSI Weightage")

/*
 * <ini>
 * ht_caps_weightage - HT caps weightage to calculate best candidate
 * @Min: 0
 * @Max: 100
 * @Default: 2
 *
 * This ini is used to increase/decrease HT caps weightage in best candidate
 * selection. If AP supports HT caps, AP will get additional Weightage with
 * this param. Weightage will be given only if dot11mode is HT capable.
 *
 * Related: None
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_HT_CAPS_WEIGHTAGE CFG_INI_UINT( \
	"ht_caps_weightage", \
	0, \
	100, \
	2, \
	CFG_VALUE_OR_DEFAULT, \
	"HT Caps Weightage")

/*
 * <ini>
 * vht_caps_weightage - VHT caps Weightage to calculate best candidate
 * @Min: 0
 * @Max: 100
 * @Default: 1
 *
 * This ini is used to increase/decrease VHT caps weightage in best candidate
 * selection. If AP supports VHT caps, AP will get additional weightage with
 * this param. Weightage will be given only if dot11mode is VHT capable.
 *
 * Related: None
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_VHT_CAPS_WEIGHTAGE CFG_INI_UINT( \
	"vht_caps_weightage", \
	0, \
	100, \
	1, \
	CFG_VALUE_OR_DEFAULT, \
	"HT Caps Weightage")

/*
 * <ini>
 * he_caps_weightage - HE caps Weightage to calculate best candidate
 * @Min: 0
 * @Max: 100
 * @Default: 2
 *
 * This ini is used to increase/decrease HE caps weightage in best candidate
 * selection. If AP supports HE caps, AP will get additional weightage with
 * this param. Weightage will be given only if dot11mode is HE capable.
 *
 * Related: None
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_HE_CAPS_WEIGHTAGE CFG_INI_UINT( \
	"he_caps_weightage", \
	0, \
	100, \
	2, \
	CFG_VALUE_OR_DEFAULT, \
	"HE Caps Weightage")

/*
 * <ini>
 * chan_width_weightage - Channel Width Weightage to calculate best candidate
 * @Min: 0
 * @Max: 100
 * @Default: 12
 *
 * This ini is used to increase/decrease Channel Width weightage in best
 * candidate selection. AP with Higher channel width will get higher weightage
 *
 * Related: bandwidth_weight_per_index
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_CHAN_WIDTH_WEIGHTAGE CFG_INI_UINT( \
	"chan_width_weightage", \
	0, \
	100, \
	12, \
	CFG_VALUE_OR_DEFAULT, \
	"Channel width weightage")

/*
 * <ini>
 * chan_band_weightage - Channel Band perferance to 5GHZ to
 * calculate best candidate
 * @Min: 0
 * @Max: 100
 * @Default: 2
 *
 * This ini is used to increase/decrease Channel Band Preference weightage
 * in best candidate selection. 5GHZ AP get this additional boost compare to
 * 2GHZ AP before   rssi_pref_5g_rssi_thresh and 2.4Ghz get weightage after
 * rssi_pref_5g_rssi_thresh.
 *
 * Related: rssi_pref_5g_rssi_thresh, band_weight_per_index
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_CHAN_BAND_WEIGHTAGE CFG_INI_UINT( \
	"chan_band_weightage", \
	0, \
	100, \
	2, \
	CFG_VALUE_OR_DEFAULT, \
	"Channel Band Weightage")

/*
 * <ini>
 * nss_weightage - NSS Weightage to calculate best candidate
 * @Min: 0
 * @Max: 100
 * @Default: 16
 *
 * This ini is used to increase/decrease NSS weightage in best candidate
 * selection. If there are two AP, one AP supports 2x2 and another one supports
 * 1x1 and station supports 2X2, first A will get this additional weightage
 * depending on self-capability.
 *
 * Related: nss_weight_per_index
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_NSS_WEIGHTAGE CFG_INI_UINT( \
	"nss_weightage", \
	0, \
	100, \
	16, \
	CFG_VALUE_OR_DEFAULT, \
	"NSS Weightage")
/*
 * <ini>
 * beamforming_cap_weightage - Beam Forming Weightage to
 *			       calculate best candidate
 * @Min: 0
 * @Max: 100
 * @Default: 2
 *
 * This ini is used to increase/decrease Beam forming Weightage if some AP
 * support Beam forming or not. If AP supports Beam forming, that AP will get
 * additional boost of this weightage.
 *
 * Related: None
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_BEAMFORM_CAP_WEIGHTAGE CFG_INI_UINT( \
	"beamforming_cap_weightage", \
	0, \
	100, \
	PLATFORM_VALUE(2, 0), \
	CFG_VALUE_OR_DEFAULT, \
	"Beamforming Cap Weightage")

/*
 * <ini>
 * pcl_weightage - PCL Weightage to calculate best candidate
 * @Min: 0
 * @Max: 100
 * @Default: 10
 *
 * This ini is used to increase/decrease PCL weightage in best candidate
 * selection. If some APs are in PCL list, those AP will get addition
 * weightage.
 *
 * Related: None
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_PCL_WEIGHTAGE CFG_INI_UINT( \
	"pcl_weightage", \
	0, \
	100, \
	PLATFORM_VALUE(10, 0), \
	CFG_VALUE_OR_DEFAULT, \
	"PCL Weightage")

/*
 * <ini>
 * channel_congestion_weightage/RoamAPScore_CUWeight - channel Congestion
 * Weightage to
 * calculate best candidate
 * @Min: 0
 * @Max: 100
 * @Default: 25
 *
 * This ini is used to increase/decrease channel congestion weightage in
 * candidate selection. Congestion is measured with the help of ESP/QBSS load.
 *
 * Related: num_esp_qbss_slots
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_CHAN_CONGESTION_WEIGHTAGE CFG_INI_UINT( \
	"channel_congestion_weightage RoamAPScore_CUWeight", \
	0, \
	100, \
	25, \
	CFG_VALUE_OR_DEFAULT, \
	"Channel Congestion Weightage")

/*
 * <ini>
 * oce_wan_weightage - OCE WAN DL capacity Weightage to calculate best candidate
 * @Min: 0
 * @Max: 100
 * @Default: 2
 *
 * This ini is used to increase/decrease OCE WAN caps weightage in best
 * candidate selection. If AP have OCE WAN information, give weightage depending
 * on the downaload available capacity.
 *
 * Related: num_oce_wan_slots
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_OCE_WAN_WEIGHTAGE CFG_INI_UINT( \
	"oce_wan_weightage", \
	0, \
	100, \
	PLATFORM_VALUE(2, 0), \
	CFG_VALUE_OR_DEFAULT, \
	"OCE WAN Weightage")

/*
 * <ini>
 * oce_ap_tx_pwr_weightage - update scoring param based on ap tx power
 * @Min: 0
 * @Max: 10
 * @Default: 5
 *
 * This ini is used to store calculate weightage based on ap tx power.
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_OCE_AP_TX_PWR_WEIGHTAGE CFG_INI_UINT( \
		"oce_ap_tx_pwr_weightage", \
		0, \
		10, \
		PLATFORM_VALUE(5, 0), \
		CFG_VALUE_OR_DEFAULT,\
		"AP weigtage for OCE ap tx power")

/*
 * <ini>
 * oce_subnet_id_weightage - update scoring param based on subnet id
 * @Min: 0
 * @Max: 10
 * @Default: 3
 *
 * This ini is used to calculate subnet id weightage in roam score.
 * If the MBO-OCE ie has "IP subnet indication attribute", then host
 * considers 50% of the "oce_subnet_id_weightage" value to calculate
 * roam score for the initial connection and 100% of the
 * "oce_subnet_id_weightage" value to calculate roam score for roaming
 * case.
 * "oce_ap_tx_pwr_weightage" adds a small amount of percentage advantage
 * in roam score while selection of an AP candidate within the same subnet
 * ID. If "oce_ap_tx_pwr_weightage" value is 0(min), roam score doesn't
 * include percentage weightage for subnet id and if "oce_ap_tx_pwr_weightage"
 * value is 10(max), then the weightage given because of same subnet ID is
 * more in roam score. This ini is also used for WFA certification.
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_OCE_SUBNET_ID_WEIGHTAGE CFG_INI_UINT( \
		"oce_subnet_id_weightage", \
		0, \
		10, \
		PLATFORM_VALUE(3, 0), \
		CFG_VALUE_OR_DEFAULT,\
		"AP weigtage for OCE subnet id")

/*
 * <ini>
 * sae_pk_ap_weightage - update scoring param based on SAE PK ap weightage
 * @Min: 0
 * @Max: 10
 * @Default: 3
 *
 * This ini is used to calculate SAE PK ap weightage in roam score. SAE Public
 * Key (SAE-PK) authentication is an extension of SAE that is intended for use
 * cases where authentication is based on a password that might be
 * distributed to or obtained by a potential adversary. With SAE-PK, the AP in
 * an infrastructure network is additionally authenticated based on a static
 * public/private key pair. This ini is also used for WFA certification.
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SAE_PK_AP_WEIGHTAGE CFG_INI_UINT( \
		"sae_pk_ap_weightage", \
		0, \
		10, \
		PLATFORM_VALUE(3, 0), \
		CFG_VALUE_OR_DEFAULT,\
		"SAE-PK AP weightage")

/*
 * <ini>
 * best_rssi_threshold - Best Rssi for score calculation
 * @Min: 0
 * @Max: 96
 * @Default: 55
 *
 * This ini tells limit for best RSSI. RSSI better than this limit are
 * considered as best RSSI. The best RSSI is given full rssi_weightage.
 *
 * Related: rssi_weightage
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_BEST_RSSI_THRESHOLD CFG_INI_UINT( \
	"best_rssi_threshold", \
	0, \
	96, \
	55, \
	CFG_VALUE_OR_DEFAULT, \
	"Best RSSI threshold")

/*
 * <ini>
 * good_rssi_threshold - Good Rssi for score calculation
 * @Min: 0
 * @Max: 96
 * @Default: 70
 *
 * This ini tells limit for good RSSI. RSSI better than this limit and less
 * than best_rssi_threshold is considered as good RSSI.
 *
 * Related: rssi_weightage, best_rssi_threshold
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_GOOD_RSSI_THRESHOLD CFG_INI_UINT( \
	"good_rssi_threshold", \
	0, \
	96, \
	70, \
	CFG_VALUE_OR_DEFAULT, \
	"Good RSSI threshold")

/*
 * <ini>
 * bad_rssi_threshold - Bad Rssi for score calculation
 * @Min: 0
 * @Max: 96
 * @Default: 80
 *
 * This ini tells limit for Bad RSSI. RSSI greater then bad_rssi_threshold
 * is considered as bad RSSI.
 *
 * Related: rssi_weightage, good_rssi_threshold
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_BAD_RSSI_THRESHOLD CFG_INI_UINT( \
	"bad_rssi_threshold", \
	0, \
	96, \
	80, \
	CFG_VALUE_OR_DEFAULT, \
	"Bad RSSI threshold")

/*
 * <ini>
 * good_rssi_pcnt - Percent Score to Good RSSI out of total RSSI score.
 * @Min: 0
 * @Max: 100
 * @Default: 80
 *
 * This ini tells about how much percent should be given to good RSSI(RSSI
 * between best_rssi_threshold and good_rssi_threshold) out of RSSI weightage.
 *
 * Related: rssi_weightage, best_rssi_threshold, good_rssi_threshold
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_GOOD_RSSI_PERCENT CFG_INI_UINT( \
	"good_rssi_pcnt", \
	0, \
	100, \
	80, \
	CFG_VALUE_OR_DEFAULT, \
	"Good RSSI Percent")

/*
 * <ini>
 * bad_rssi_pcnt - Percent Score to BAD RSSI out of total RSSI score.
 * @Min: 0
 * @Max: 100
 * @Default: 25
 *
 * This ini tells about how much percent should be given to bad RSSI (RSSI
 * between good_rssi_threshold and bad_rssi_threshold) out of RSSI weightage.
 *
 * Related: rssi_weightage, good_rssi_threshold, bad_rssi_threshold
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_BAD_RSSI_PERCENT CFG_INI_UINT( \
	"bad_rssi_pcnt", \
	0, \
	100, \
	25, \
	CFG_VALUE_OR_DEFAULT, \
	"Bad RSSI Percent")

/*
 * <ini>
 * good_rssi_bucket_size - Bucket size between best and good RSSI to score.
 * @Min: 1
 * @Max: 10
 * @Default: 5
 *
 * This ini tells about bucket size for scoring between best and good RSSI.
 * Below Best RSSI, 100% score will be given. Between best and good RSSI, RSSI
 * is divided in buckets and score will be assigned bucket wise starting from
 * good_rssi_pcnt.
 *
 * Related: rssi_weightage, good_rssi_pcnt
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_GOOD_RSSI_BUCKET_SIZE CFG_INI_UINT( \
	"good_rssi_bucket_size", \
	1, \
	10, \
	5, \
	CFG_VALUE_OR_DEFAULT, \
	"Good RSSI Bucket Size")

/*
 * <ini>
 * bad_rssi_bucket_size - Bucket size between good and bad RSSI to score.
 * @Min: 1
 * @Max: 10
 * @Default: 5
 *
 * This ini tells about bucket size for scoring between good and bad RSSI.
 * Between good and bad RSSI, RSSI is divided in buckets and score will be
 * assigned bucket wise starting from bad_rssi_pcnt.
 *
 * Related: rssi_weightage, bad_rssi_pcnt
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_BAD_RSSI_BUCKET_SIZE CFG_INI_UINT( \
	"bad_rssi_bucket_size", \
	1, \
	10, \
	5, \
	CFG_VALUE_OR_DEFAULT, \
	"Bad RSSI Bucket Size")

/*
 * <ini>
 * rssi_pref_5g_rssi_thresh - A RSSI threshold above which 5 GHz is not favored
 * @Min: 0
 * @Max: 96
 * @Default: 76
 *
 * 5G AP are given chan_band_weightage. This ini tells about RSSI threshold
 * above which 5GHZ is not favored.
 *
 * Related: chan_band_weightage
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_RSSI_PREF_5G_THRESHOLD CFG_INI_UINT( \
	"rssi_pref_5g_rssi_thresh", \
	0, \
	96, \
	76, \
	CFG_VALUE_OR_DEFAULT, \
	"RSSI Pref 5G Threshold")

/*
 * <ini>
 * bandwidth_weight_per_index - percentage as per bandwidth
 * @Min: 0x00000000
 * @Max: 0x64646464
 * @Default: 0x6432190C
 *
 * This INI give percentage value of chan_width_weightage to be used as per
 * peer bandwidth. Self BW is also considered while calculating score. Eg if
 * self BW is 20 MHZ 10% will be given for all AP irrespective of the AP
 * capability.
 *
 * Indexes are defined in this way.
 *     0 Index (BITS 0-7): 20 MHz - Def 12%
 *     1 Index (BITS 8-15): 40 MHz - Def 25%
 *     2 Index (BITS 16-23): 80 MHz - Def 50%
 *     3 Index (BITS 24-31): 160 MHz - Def 100%
 * These percentage values are stored in HEX. For any index max value, can be 64
 *
 * Related: chan_width_weightage
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_BW_WEIGHT_PER_IDX CFG_INI_UINT( \
	"bandwidth_weight_per_index", \
	0x00000000, \
	0x64646464, \
	0x6432190C, \
	CFG_VALUE_OR_DEFAULT, \
	"Bandwidth weight per index")

/*
 * <ini>
 * nss_weight_per_index - percentage as per NSS
 * @Min: 0x00000000
 * @Max: 0x64646464
 * @Default: 0x6432190C
 *
 * This INI give percentage value of nss_weightage to be used as per peer NSS.
 * Self NSS capability is also considered. Eg if self NSS is 1x1 10% will be
 * given for all AP irrespective of the AP capability.
 *
 * Indexes are defined in this way.
 *     0 Index (BITS 0-7): 1X1- Def 12%
 *     1 Index (BITS 8-15): 2X2- Def 25%
 *     2 Index (BITS 16-23): 3X3- Def 50%
 *     3 Index (BITS 24-31): 4X4- Def 100%
 * These percentage values are stored in HEX. For any index max value, can be 64
 *
 * Related: nss_weightage
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_NSS_WEIGHT_PER_IDX CFG_INI_UINT( \
	"nss_weight_per_index", \
	0x00000000, \
	0x64646464, \
	0x6432190C, \
	CFG_VALUE_OR_DEFAULT, \
	"NSS weight per index")

/*
 * <ini>
 * band_weight_per_index - percentage as per band
 * @Min: 0x00000000
 * @Max: 0x64646464
 * @Default: 0x0000644B
 *
 * This INI give percentage value of chan_band_weightage to be used as per band.
 * If RSSI is greater than rssi_pref_5g_rssi_thresh preference is given for 5Ghz
 * else, it's given for 2.4Ghz.
 *
 * Indexes are defined in this way.
 *     0 Index (BITS 0-7): 2.4GHz - Def 10%
 *     1 Index (BITS 8-15): 5GHz - Def 20%
 *     2 Index (BITS 16-23): 6Ghz - Def - 100%
 *     3 Index (BITS 24-31): Reserved
 * These percentage values are stored in HEX. For any index max value, can be 64
 *
 * Related: chan_band_weightage, rssi_pref_5g_rssi_thresh
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_BAND_WEIGHT_PER_IDX CFG_INI_UINT( \
	"band_weight_per_index", \
	0x00000000, \
	0x64646464, \
	0x00644B32, \
	CFG_VALUE_OR_DEFAULT, \
	"Band weight per index")

/*
 * <ini>
 * num_esp_qbss_slots - number of slots in which the esp/qbss load will
 * be divided
 *
 * @Min: 1
 * @Max: 15
 * @Default: 8
 *
 * Number of slots in which the esp/qbss load will be divided. Max 15. index 0
 * is used for 'not_present. Num_slot will equally divide 100. e.g, if
 * num_slot = 4 slot 1 = 0-25% load, slot 2 = 26-50% load, slot 3 = 51-75% load,
 * slot 4 = 76-100% load. Remaining unused index can be 0.
 * Following is load percentage, score percentage and score of num_slot = 8,
 * weight=25.
 * 0-12% 13-25% 26-38% 39-50% 51-63 %64-75% 76-88% 87-100% Not Present
 * 100%  90%    80%    70%    50%   25%     10%    5%      50%
 * 2500  2250   2000   1750   1250  625     250    125     1250
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_NUM_ESP_QBSS_SLOTS CFG_INI_UINT( \
	"num_esp_qbss_slots", \
	1, \
	15, \
	8, \
	CFG_VALUE_OR_DEFAULT, \
	"Num ESP QPSS Slots")

/*
 * <ini>
 * esp_qbss_score_idx3_to_0 - percentage for  esp/qbss load for slots 0-3
 * @Min: 0x00000000
 * @Max: 0x64646464
 * @Default: 0x505A6432
 *
 * This INI give percentage value of channel_congestion_weightage to be used as
 * index in which the load value falls. Index 0 is for percentage when ESP/QBSS
 * is not present.
 *
 * Indexes are defined in this way.
 *     0 Index (BITS 0-7): when ESP/QBSS is not present
 *     1 Index (BITS 8-15): SLOT_1
 *     2 Index (BITS 16-23): SLOT_2
 *     3 Index (BITS 24-31): SLOT_3
 * These percentage values are stored in HEX. For any index max value, can be 64
 *
 * Related: channel_congestion_weightage, num_esp_qbss_slots
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_ESP_QBSS_SCORE_IDX_3_TO_0 CFG_INI_UINT( \
	"esp_qbss_score_idx3_to_0", \
	0x00000000, \
	0x64646464, \
	0x505A6432, \
	CFG_VALUE_OR_DEFAULT, \
	"ESP QPSS Score Index 3 to 0")

/*
 * <ini>
 * esp_qbss_score_idx7_to_4 - percentage for  esp/qbss load for slots 4-7
 * @Min: 0x00000000
 * @Max: 0x64646464
 * @Default: 0x0A193246
 *
 * This INI give percentage value of channel_congestion_weightage to be used as
 * index in which the load value falls. Used only if num_esp_qbss_slots is
 * greater than 3.
 *
 * Indexes are defined in this way.
 *     0 Index (BITS 0-7): SLOT_4
 *     1 Index (BITS 8-15): SLOT_5
 *     2 Index (BITS 16-23): SLOT_6
 *     3 Index (BITS 24-31): SLOT_7
 * These percentage values are stored in HEX. For any index max value, can be 64
 *
 * Related: channel_congestion_weightage, num_esp_qbss_slots
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_ESP_QBSS_SCORE_IDX_7_TO_4 CFG_INI_UINT( \
	"esp_qbss_score_idx7_to_4", \
	0x00000000, \
	0x64646464, \
	0x0A193246, \
	CFG_VALUE_OR_DEFAULT, \
	"ESP QPSS Score Index 7 to 4")

/*
 * <ini>
 * esp_qbss_score_idx11_to_8 - percentage for  esp/qbss load for slots 8-11
 * @Min: 0x00000000
 * @Max: 0x64646464
 * @Default: 0x00000005
 *
 * This INI give percentage value of channel_congestion_weightage to be used as
 * index in which the load value falls. Used only if num_esp_qbss_slots is
 * greater than 7.
 *
 * Indexes are defined in this way.
 *     0 Index (BITS 0-7): SLOT_8
 *     1 Index (BITS 8-15): SLOT_9
 *     2 Index (BITS 16-23): SLOT_10
 *     3 Index (BITS 24-31): SLOT_11
 * These percentage values are stored in HEX. For any index max value, can be 64
 *
 * Related: channel_congestion_weightage, num_esp_qbss_slots
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_ESP_QBSS_SCORE_IDX_11_TO_8 CFG_INI_UINT( \
	"esp_qbss_score_idx11_to_8", \
	0x00000000, \
	0x64646464, \
	0x00000005, \
	CFG_VALUE_OR_DEFAULT, \
	"ESP QPSS Score Index 11 to 8")

/*
 * <ini>
 * esp_qbss_score_idx15_to_12 - percentage for	esp/qbss load for slots 12-15
 * @Min: 0x00000000
 * @Max: 0x64646464
 * @Default: 0x00000000
 *
 * This INI give percentage value of channel_congestion_weightage to be used as
 * index in which the load value falls. Used only if num_esp_qbss_slots is
 * greater than 11.
 *
 * Indexes are defined in this way.
 *     0 Index (BITS 0-7): SLOT_12
 *     1 Index (BITS 8-15): SLOT_13
 *     2 Index (BITS 16-23): SLOT_14
 *     3 Index (BITS 24-31): SLOT_15
 * These percentage values are stored in HEX. For any index max value, can be 64
 *
 * Related: channel_congestion_weightage, num_esp_qbss_slots
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_ESP_QBSS_SCORE_IDX_15_TO_12 CFG_INI_UINT( \
	"esp_qbss_score_idx15_to_12", \
	0x00000000, \
	0x64646464, \
	0x00000000, \
	CFG_VALUE_OR_DEFAULT, \
	"ESP QPSS Score Index 15 to 12")

/*
 * <ini>
 * num_oce_wan_slots - number of slots in which the oce wan metrics will
 * be divided
 *
 * @Min: 1
 * @Max: 15
 * @Default: 8
 *
 * Number of slots in which the oce wan metrics will be divided. Max 15. index 0
 * is used for not_present. Num_slot will equally divide 100. e.g, if
 * num_slot = 4 slot 1 = 0-3 DL CAP, slot 2 = 4-7 DL CAP, slot 3 = 8-11 DL CAP,
 * slot 4 = 12-15 DL CAP. Remaining unused index can be 0.
 *
 * Related: oce_wan_weightage
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_NUM_OCE_WAN_SLOTS CFG_INI_UINT( \
	"num_oce_wan_slots", \
	1, \
	15, \
	15, \
	CFG_VALUE_OR_DEFAULT, \
	"Num OCE WAN Slots")

/*
 * <ini>
 * oce_wan_score_idx3_to_0 - percentage for OCE WAN metrics score for slots 0-3
 * @Min: 0x00000000
 * @Max: 0x64646464
 * @Default: 0x00000032
 *
 * This INI give percentage value of OCE WAN metrics DL CAP, to be used as
 * index in which the DL CAP value falls. Index 0 is for percentage when
 * OCE WAN metrics DL CAP is not present.
 *
 * Indexes are defined in this way.
 *     0 Index (BITS 0-7): when OCE WAN metrics DL CAP is not present
 *     1 Index (BITS 8-15): SLOT_1
 *     2 Index (BITS 16-23): SLOT_2
 *     3 Index (BITS 24-31): SLOT_3
 * These percentage values are stored in HEX. For any index max value, can be 64
 *
 * Related: num_oce_wan_slots, oce_wan_weightage
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_OCE_WAN_SCORE_IDX_3_TO_0 CFG_INI_UINT( \
	"oce_wan_score_idx3_to_0", \
	0x00000000, \
	0x64646464, \
	0x00000032, \
	CFG_VALUE_OR_DEFAULT, \
	"OCE WAN Score Index 3 to 0")

/*
 * <ini>
 * oce_wan_score_idx7_to_4 - percentage for OCE WAN metrics score for slots 4-7
 * @Min: 0x00000000
 * @Max: 0x64646464
 * @Default: 0x00000000
 *
 * This INI give percentage value of OCE WAN metrics DL CAP, to be used as
 * index in which the DL CAP value falls. Used only if num_oce_wan_slots is
 * greater than 3.
 *
 * Indexes are defined in this way.
 *     0 Index (BITS 0-7): SLOT_4
 *     1 Index (BITS 8-15): SLOT_5
 *     2 Index (BITS 16-23): SLOT_6
 *     3 Index (BITS 24-31): SLOT_7
 * These percentage values are stored in HEX. For any index max value, can be 64
 *
 * Related: num_oce_wan_slots, oce_wan_weightage
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_OCE_WAN_SCORE_IDX_7_TO_4 CFG_INI_UINT( \
	"oce_wan_score_idx7_to_4", \
	0x00000000, \
	0x64646464, \
	0x00000000, \
	CFG_VALUE_OR_DEFAULT, \
	"OCE WAN Score Index 7 to 4")

/*
 * <ini>
 * oce_wan_score_idx11_to_8 - percentage for OCE WAN metrics score for slot 8-11
 * @Min: 0x00000000
 * @Max: 0x64646464
 * @Default: 0x06030000
 *
 * This INI give percentage value of OCE WAN metrics DL CAP, to be used as
 * index in which the DL CAP value falls. Used only if num_oce_wan_slots is
 * greater than 7.
 *
 * Indexes are defined in this way.
 *     0 Index (BITS 0-7): SLOT_8
 *     1 Index (BITS 8-15): SLOT_9
 *     2 Index (BITS 16-23): SLOT_10
 *     3 Index (BITS 24-31): SLOT_11
 * These percentage values are stored in HEX. For any index max value, can be 64
 *
 * Related: num_oce_wan_slots, oce_wan_weightage
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_OCE_WAN_SCORE_IDX_11_TO_8 CFG_INI_UINT( \
	"oce_wan_score_idx11_to_8", \
	0x00000000, \
	0x64646464, \
	0x06030000, \
	CFG_VALUE_OR_DEFAULT, \
	"OCE WAN Score Index 11 to 8")

/*
 * <ini>
 * oce_wan_score_idx15_to_12 - % for OCE WAN metrics score for slot 12-15
 * @Min: 0x00000000
 * @Max: 0x64646464
 * @Default: 0x6432190C
 *
 * This INI give percentage value of OCE WAN metrics DL CAP, to be used as
 * index in which the DL CAP value falls. Used only if num_oce_wan_slots is
 * greater than 11.
 *
 * Indexes are defined in this way.
 *     0 Index (BITS 0-7): SLOT_12
 *     1 Index (BITS 8-15): SLOT_13
 *     2 Index (BITS 16-23): SLOT_14
 *     3 Index (BITS 24-31): SLOT_15
 * These percentage values are stored in HEX. For any index max value, can be 64
 *
 * Related: num_oce_wan_slots, oce_wan_weightage
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_OCE_WAN_SCORE_IDX_15_TO_12 CFG_INI_UINT( \
	"oce_wan_score_idx15_to_12", \
	0x00000000, \
	0x64646464, \
	0x6432190C, \
	CFG_VALUE_OR_DEFAULT, \
	"OCE WAN Score Index 15 to 12")

/*
 * <ini>
 * is_bssid_hint_priority - Set priority for connection with bssid_hint
 * BSSID.
 * @Min: 0
 * @Max: 1
 * @Default: 1 - AP and 0 - non AP
 *
 * This ini is used to give priority to BSS for connection which comes
 * as part of bssid_hint
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_IS_BSSID_HINT_PRIORITY CFG_INI_UINT(\
			"is_bssid_hint_priority",\
			0, 1, \
			PLATFORM_VALUE(0, 1), \
			CFG_VALUE_OR_DEFAULT, \
			"Set priority for connection with bssid_hint")

/*
 * <ini>
 * vendor_roam_score_algorithm - Algorithm to calculate AP score
 * @Min: false
 * @Max: true
 * @Default: false
 *
 * By default the value is false and default roam algorithm will be used.
 * When the value is true, the V2 roaming algorithm will be used:
 * For this V2 algo, AP score calculation is based on ETP and below equation:
 * AP Score = (RSSIfactor * rssiweight(0.65)) + (CUfactor *cuweight(0.35))
 *
 * Related: None
 *
 * Supported Feature: roam score algorithm
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_VENDOR_ROAM_SCORE_ALGORITHM \
	CFG_INI_BOOL("vendor_roam_score_algorithm", false, \
	"Roam candidate selection score algorithm")

#ifdef CONFIG_BAND_6GHZ
/*
 * <ini>
 * check_6ghz_security - Enable check for 6Ghz allowed security
 * BSSID.
 * @Min: 0
 * @Max: 1
 * @Default: non AP 1, AP 0
 *
 * This ini is used to Enable check for 6Ghz allowed security. If enabled
 * only WPA3 and other allowed security will be allowed for 6Ghz connection
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_CHECK_6GHZ_SECURITY CFG_INI_BOOL(\
				"check_6ghz_security", PLATFORM_VALUE(1, 0), \
				"Enable check for 6Ghz allowed security")
/*
 * <ini>
 * key_mgmt_mask_6ghz - AKM bit mask (@wlan_crypto_key_mgmt) allowed in 6Ghz
 * channel
 * @Min: 0
 * @Max: 0xffffffff
 * @Default: 0xffffffff
 *
 * This ini is used to set allowed AKM check for 6Ghz. If enabled
 * only only AKM bits allowed will be used to connect to candidate.
 * valid only if check_6ghz_security is 0. By default all AKM are allowed
 *
 * Related: check_6Ghz_security
 *
 * Supported Feature: STA
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_6GHZ_ALLOWED_AKM_MASK CFG_INI_UINT(\
			"key_mgmt_mask_6ghz",\
			0, DEFAULT_KEYMGMT_6G_MASK, DEFAULT_KEYMGMT_6G_MASK,\
			CFG_VALUE_OR_DEFAULT, \
			"Set priority for connection with bssid_hint")

#define CFG_6GHZ_CONFIG \
	CFG(CFG_CHECK_6GHZ_SECURITY) \
	CFG(CFG_6GHZ_ALLOWED_AKM_MASK)
#else
#define CFG_6GHZ_CONFIG
#endif

#ifdef WLAN_FEATURE_11BE_MLO
/**
 * <ini>
 * eht_caps_weightage - EHT caps Weightage to calculate best candidate
 * @Min: 0
 * @Max: 100
 * @Default: 2
 *
 * This ini is used to increase/decrease EHT caps weightage in best candidate
 * selection. If AP supports EHT caps, AP will get additional weightage with
 * this param. Weightage will be given only if dot11mode is EHT capable.
 *
 * Related: None
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_EHT_CAPS_WEIGHTAGE CFG_INI_UINT( \
	"eht_caps_weightage", \
	0, \
	100, \
	2, \
	CFG_VALUE_OR_DEFAULT, \
	"EHT Caps Weightage")

/**
 * <ini>
 * mlo_weightage - MLO Weightage to calculate best candidate
 * @Min: 0
 * @Max: 100
 * @Default: 3
 *
 * This ini is used to increase/decrease MLO weightage in best candidate
 * selection. If AP supports MLO, AP will get additional weightage with
 * this param.
 *
 * Related: None
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_MLO_WEIGHTAGE CFG_INI_UINT( \
	"mlo_weightage", \
	0, \
	100, \
	3, \
	CFG_VALUE_OR_DEFAULT, \
	"MLO Weightage")

/**
 * <ini>
 * emlsr_weightage - eMLSR Weightage to calculate best candidate
 * @Min: 0
 * @Max: 100
 * @Default: 3
 *
 * This ini is used to increase/decrease eMLSR weightage in best candidate
 * selection. If AP supports eMLSR, AP will get additional weightage with
 * this param.
 *
 * Related: None
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_EMLSR_WEIGHTAGE CFG_INI_UINT( \
	"emlsr_weightage", \
	0, \
	100, \
	3, \
	CFG_VALUE_OR_DEFAULT, \
	"eMLSR Weightage")

/**
 * <ini>
 * wlm_indication_weightage - WLM indication Weightage to calculate best
 *                            candidate
 * @Min: 0
 * @Max: 100
 * @Default: 2
 *
 * This ini is used to increase/decrease WLM indication weightage in best
 * candidate selection. AP will get additional weightage with this param based
 * on WLM indicates ultra low latency or low latency.
 *
 * Related: None
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_WLM_INDICATION_WEIGHTAGE CFG_INI_UINT( \
	"wlm_indication_weightage", \
	0, \
	100, \
	2, \
	CFG_VALUE_OR_DEFAULT, \
	"WLM indication Weightage")

/**
 * <ini>
 * mlsr_link_selection - MLSR link selection criteria
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to select the link to operate in MLSR mode.
 *
 * The values are defined as follows:
 *    0 : ML Link with highest link score
 *    1 : Link corresponds to best average ML score
 *
 * Related: None
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_MLSR_LINK_SELECTION CFG_INI_UINT( \
	"mlsr_link_selection", \
	0, \
	1, \
	0, \
	CFG_VALUE_OR_DEFAULT, \
	"MLSR link selection")

/**
 * <ini>
 * joint_rssi_alpha - Joint RSSI alpha to select best ML candidate
 * @Min: 0
 * @Max: 100
 * @Default: 50
 *
 * This ini is used to increase/decrease joint RSSI alpha percentage for
 * an MLO candidate in best candidate selection. Using this the joint RSSI
 * will be calculated as below.
 *
 *     joint_rssi = (alpha % Link1 RSSI) + ((100 - alpha)% Link2 RSSI)
 *
 * Related: low_band_rssi_boost
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_JOINT_RSSI_ALPHA CFG_INI_UINT( \
	"joint_rssi_alpha", \
	0, \
	100, \
	50, \
	CFG_VALUE_OR_DEFAULT, \
	"Joint RSSI alpha")

/**
 * <ini>
 * low_band_rssi_boost - Low band RSSI boost in joint RSSI calculation
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * If joint RSSI alpha is not 50 then joint RSSI calculation will depend on
 * this INI param as explained below.
 *
 * If this INI set to 1, then lower band will be given more weightage.
 * Otherwise higher band will get more weightage.
 *
 *    Example: Joint RSSI alpha is 70
 *	if (low_band_rssi_boost)
 *		joint_rssi_5g_6g = (70 * 5g_rssi) + (30 * 6g_rssi)
 *	else
 *		joint_rssi_5g_6g = (30 * 5g_rssi) + (70 * 6g_rssi)
 *
 * Related: joint_rssi_alpha
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_LOW_BAND_RSSI_BOOST CFG_INI_BOOL( \
				"low_band_rssi_boost", \
				1, \
				"Low band RSSI boost ")

/**
 * <ini>
 * joint_esp_alpha - Joint ESP alpha to select best ML candidate
 * @Min: 0
 * @Max: 100
 * @Default: 50
 *
 * This ini is used to increase/decrease joint ESP alpha percentage for
 * an MLO candidate in best candidate selection. Using this the joint ESP
 * will be calculated as below.
 *
 *     joint_esp = (alpha % Link1 ESP) + ((100 - alpha)% Link2 ESP)
 *
 * Related: low_band_esp_boost
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_JOINT_ESP_ALPHA CFG_INI_UINT( \
	"joint_esp_alpha", \
	0, \
	100, \
	50, \
	CFG_VALUE_OR_DEFAULT, \
	"Joint ESP alpha")

/**
 * <ini>
 * low_band_esp_boost - Low band ESP boost in joint ESP calculation
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * If joint ESP alpha is not 50 then joint ESP calculation will depend on
 * this INI param as explained below.
 *
 * If this INI set to 1, then lower band will be given more weightage.
 * Otherwise higher band will get more weightage.
 *
 *    Example: Joint ESP alpha is 70
 *	if (low_band_esp_boost)
 *		joint_esp_5g_6g = (70 * 5g_esp) + (30 * 6g_esp)
 *	else
 *		joint_esp_5g_6g = (30 * 5g_esp) + (70 * 6g_esp)
 *
 * Related: joint_esp_alpha
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_LOW_BAND_ESP_BOOST CFG_INI_BOOL( \
				"low_band_esp_boost", \
				1, \
				"Low band ESP boost ")

/**
 * <ini>
 * joint_oce_alpha - Joint OCE alpha to select best ML candidate
 * @Min: 0
 * @Max: 100
 * @Default: 50
 *
 * This ini is used to increase/decrease joint OCE alpha percentage for
 * an MLO candidate in best candidate selection. Using this the joint OCE
 * will be calculated as below.
 *
 *     joint_oce = (alpha % Link1 OCE) + ((100 - alpha)% Link2 OCE)
 *
 * Related: low_band_oce_boost
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_JOINT_OCE_ALPHA CFG_INI_UINT( \
	"joint_oce_alpha", \
	0, \
	100, \
	50, \
	CFG_VALUE_OR_DEFAULT, \
	"Joint OCE alpha")

/**
 * <ini>
 * low_band_oce_boost - Low band OCE boost in joint OCE calculation
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * If joint OCE alpha is not 50 then joint OCE calculation will depend on
 * this INI param as explained below.
 *
 * If this INI set to 1, then lower band will be given more weightage.
 * Otherwise higher band will get more weightage.
 *
 *    Example: Joint OCE alpha is 70
 *	if (low_band_oce_boost)
 *		joint_oce_5g_6g = (70 * 5g_oce) + (30 * 6g_oce)
 *	else
 *		joint_oce_5g_6g = (30 * 5g_oce) + (70 * 6g_oce)
 *
 * Related: joint_oce_alpha
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_LOW_BAND_OCE_BOOST CFG_INI_BOOL( \
				"low_band_oce_boost", \
				1, \
				"Low band OCE boost ")

/*
 * <ini>
 * ml_bandwidth_weight_per_index_4_to_7 - percentage as per bandwidth
 * @Min: 0x00000000
 * @Max: 0x64646464
 * @Default: 0x342A1F14
 *
 * This INI give percentage value of chan_width_weightage to be used as per
 * peer bandwidth for two links. Self BW is also considered while calculating
 * score. Eg if self BW is 20+20 MHZ 20% will be given for all AP irrespective
 * of the AP  capability.
 *
 * Indexes are defined in this way.
 *     4 Index (BITS 0-7): 20+20 MHz - Def 20%
 *     5 Index (BITS 8-15): 20+40 MHz - Def 31%
 *     6 Index (BITS 16-23): 40+40 MHz - Def 42%
 *     7 Index (BITS 24-31): 20+80 MHz - Def 52%
 * These percentage values are stored in HEX. For any index max value, can be 64
 *
 * Indexes 0-3 are considered as part of the INI bandwidth_weight_per_index
 *
 * Related: chan_width_weightage
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_ML_BW_WEIGHT_PER_IDX_4_TO_7 CFG_INI_UINT( \
	"ml_bandwidth_weight_per_index_4_to_7", \
	0x00000000, \
	0x64646464, \
	0x342A1F14, \
	CFG_VALUE_OR_DEFAULT, \
	"ML Bandwidth weight per index 4 to 7")

/*
 * <ini>
 * ml_bandwidth_weight_per_index_8_to_11 - percentage as per bandwidth
 * @Min: 0x00000000
 * @Max: 0x64646464
 * @Default: 0x5A57553F
 *
 * This INI give percentage value of chan_width_weightage to be used as per
 * peer bandwidth for two links. Self BW is also considered while calculating
 * score. Eg if self BW is 20+20 MHZ 20% will be given for all AP irrespective
 * of the AP  capability.
 *
 * Indexes are defined in this way.
 *     8 Index (BITS 0-7): 40+80 MHz - Def 63%
 *     9 Index (BITS 8-15): 80+80 MHz - Def 85%
 *     10 Index (BITS 16-23): 20+160 MHz - Def 87%
 *     11 Index (BITS 24-31): 40+160 MHz - Def 90%
 * These percentage values are stored in HEX. For any index max value, can be 64
 *
 * Related: chan_width_weightage
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_ML_BW_WEIGHT_PER_IDX_8_TO_11 CFG_INI_UINT( \
	"ml_bandwidth_weight_per_index_8_to_11", \
	0x00000000, \
	0x64646464, \
	0x5A57553F, \
	CFG_VALUE_OR_DEFAULT, \
	"ML Bandwidth weight per index 8 to 11")

/*
 * <ini>
 * ml_bandwidth_weight_per_index_12_to_15 - percentage as per bandwidth
 * @Min: 0x00000000
 * @Max: 0x00646464
 * @Default: 0x0064645F
 *
 * This INI give percentage value of chan_width_weightage to be used as per
 * peer bandwidth for two links. Self BW is also considered while calculating
 * score. Eg if self BW is 20+20 MHZ 20% will be given for all AP irrespective
 * of the AP  capability.
 *
 * Indexes are defined in this way.
 *     12 Index (BITS 0-7): 80+160 MHz - Def 95%
 *     13 Index (BITS 8-15): 160+160 MHz - Def 100%
 *     14 Index (BITS 16-23): 320 MHz - Def 100%
 *     15 Index (BITS 24-31): Reserved - Def 0
 * These percentage values are stored in HEX. For any index max value, can be 64
 *
 * Related: chan_width_weightage
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_ML_BW_WEIGHT_PER_IDX_12_TO_15 CFG_INI_UINT( \
	"ml_bandwidth_weight_per_index_12_to_15", \
	0x00000000, \
	0x00646464, \
	0x0064645F, \
	CFG_VALUE_OR_DEFAULT, \
	"ML Bandwidth weight per index 12 to 15")

/*
 * <ini>
 * ml_nss_weight_per_index_4_to_7 - percentage as per NSS
 * @Min: 0x00000000
 * @Max: 0x00646464
 * @Default: 0x00645019
 *
 * This INI give percentage value of nss_weightage to be used as per peer NSS.
 * Self NSS capability is also considered. Eg if self NSS is 1x1 10% will be
 * given for all AP irrespective of the AP capability.
 *
 * Indexes are defined in this way.
 *     4 Index (BITS 0-7): 2x2 + 2x2 - Def 25%
 *     5 Index (BITS 8-15): 4x4 + 4x4 - Def 80%
 *     6 Index (BITS 16-23): 8X8- Def 100%
 *     7 Index (BITS 24-31): Reserved - Def 0
 * These percentage values are stored in HEX. For any index max value, can be 64
 *
 * Indexes 0-3 are considered as part of the INI nss_weight_per_index
 *
 * Related: nss_weightage
 *
 * Supported Feature: STA Candidate selection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_ML_NSS_WEIGHT_PER_IDX_4_TO_7 CFG_INI_UINT( \
	"ml_nss_weight_per_index_4_to_7", \
	0x00000000, \
	0x00646464, \
	0x00645019, \
	CFG_VALUE_OR_DEFAULT, \
	"ML NSS weight per index 4 to 7")

#define CFG_MLO_CONFIG \
	CFG(CFG_SCORING_EHT_CAPS_WEIGHTAGE) \
	CFG(CFG_SCORING_EMLSR_WEIGHTAGE) \
	CFG(CFG_SCORING_JOINT_ESP_ALPHA) \
	CFG(CFG_SCORING_JOINT_OCE_ALPHA) \
	CFG(CFG_SCORING_JOINT_RSSI_ALPHA) \
	CFG(CFG_SCORING_LOW_BAND_ESP_BOOST) \
	CFG(CFG_SCORING_LOW_BAND_OCE_BOOST) \
	CFG(CFG_SCORING_LOW_BAND_RSSI_BOOST) \
	CFG(CFG_SCORING_ML_BW_WEIGHT_PER_IDX_4_TO_7) \
	CFG(CFG_SCORING_ML_BW_WEIGHT_PER_IDX_8_TO_11) \
	CFG(CFG_SCORING_ML_BW_WEIGHT_PER_IDX_12_TO_15) \
	CFG(CFG_SCORING_ML_NSS_WEIGHT_PER_IDX_4_TO_7) \
	CFG(CFG_SCORING_MLO_WEIGHTAGE) \
	CFG(CFG_SCORING_MLSR_LINK_SELECTION) \
	CFG(CFG_SCORING_WLM_INDICATION_WEIGHTAGE)
#else
#define CFG_MLO_CONFIG
#endif

#define CFG_MLME_SCORE_ALL \
	CFG(CFG_SCORING_RSSI_WEIGHTAGE) \
	CFG(CFG_SCORING_HT_CAPS_WEIGHTAGE) \
	CFG(CFG_SCORING_VHT_CAPS_WEIGHTAGE) \
	CFG(CFG_SCORING_HE_CAPS_WEIGHTAGE) \
	CFG(CFG_SCORING_CHAN_WIDTH_WEIGHTAGE) \
	CFG(CFG_SCORING_CHAN_BAND_WEIGHTAGE) \
	CFG(CFG_SCORING_NSS_WEIGHTAGE) \
	CFG(CFG_SCORING_BEAMFORM_CAP_WEIGHTAGE) \
	CFG(CFG_SCORING_PCL_WEIGHTAGE) \
	CFG(CFG_SCORING_CHAN_CONGESTION_WEIGHTAGE) \
	CFG(CFG_SCORING_OCE_WAN_WEIGHTAGE) \
	CFG(CFG_OCE_AP_TX_PWR_WEIGHTAGE) \
	CFG(CFG_OCE_SUBNET_ID_WEIGHTAGE) \
	CFG(CFG_SAE_PK_AP_WEIGHTAGE) \
	CFG(CFG_SCORING_BEST_RSSI_THRESHOLD) \
	CFG(CFG_SCORING_GOOD_RSSI_THRESHOLD) \
	CFG(CFG_SCORING_BAD_RSSI_THRESHOLD) \
	CFG(CFG_SCORING_GOOD_RSSI_PERCENT) \
	CFG(CFG_SCORING_BAD_RSSI_PERCENT) \
	CFG(CFG_SCORING_GOOD_RSSI_BUCKET_SIZE) \
	CFG(CFG_SCORING_BAD_RSSI_BUCKET_SIZE) \
	CFG(CFG_SCORING_RSSI_PREF_5G_THRESHOLD) \
	CFG(CFG_SCORING_BW_WEIGHT_PER_IDX) \
	CFG(CFG_SCORING_NSS_WEIGHT_PER_IDX) \
	CFG(CFG_SCORING_BAND_WEIGHT_PER_IDX) \
	CFG(CFG_SCORING_NUM_ESP_QBSS_SLOTS) \
	CFG(CFG_SCORING_ESP_QBSS_SCORE_IDX_3_TO_0) \
	CFG(CFG_SCORING_ESP_QBSS_SCORE_IDX_7_TO_4) \
	CFG(CFG_SCORING_ESP_QBSS_SCORE_IDX_11_TO_8) \
	CFG(CFG_SCORING_ESP_QBSS_SCORE_IDX_15_TO_12) \
	CFG(CFG_SCORING_NUM_OCE_WAN_SLOTS) \
	CFG(CFG_SCORING_OCE_WAN_SCORE_IDX_3_TO_0) \
	CFG(CFG_SCORING_OCE_WAN_SCORE_IDX_7_TO_4) \
	CFG(CFG_SCORING_OCE_WAN_SCORE_IDX_11_TO_8) \
	CFG(CFG_SCORING_OCE_WAN_SCORE_IDX_15_TO_12) \
	CFG(CFG_IS_BSSID_HINT_PRIORITY) \
	CFG(CFG_VENDOR_ROAM_SCORE_ALGORITHM) \
	CFG_6GHZ_CONFIG \
	CFG_MLO_CONFIG

#endif /* __CFG_MLME_SCORE_PARAMS_H */
