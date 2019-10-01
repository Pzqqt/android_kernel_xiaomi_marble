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
 * DOC: This file contains MLME SCORING related CFG/INI Items.
 */

#ifndef __CFG_MLME_SCORING_H
#define __CFG_MLME_SCORING_H

/*
 * <ini>
 * rssi_weightage - RSSI Weightage to calculate best candidate
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
	"rssi_weightage", \
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
	2, \
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
	10, \
	CFG_VALUE_OR_DEFAULT, \
	"PCL Weightage")

/*
 * <ini>
 * channel_congestion_weightage - channel Congestion Weightage to
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
	"channel_congestion_weightage", \
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
	2, \
	CFG_VALUE_OR_DEFAULT, \
	"OCE WAN Weightage")

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
 *     2 Index (BITS 16-23): Reserved
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
	0x0000644B, \
	CFG_VALUE_OR_DEFAULT, \
	"Band weight per index")

/*
 * <ini>
 * num_esp_qbss_slots - number of slots in which the esp/qbss load will
 * be divided
 *
 * @Min: 1
 * @Max: 15
 * @Default: 4
 *
 * Number of slots in which the esp/qbss load will be divided. Max 15. index 0
 * is used for 'not_present. Num_slot will equally divide 100. e.g, if
 * num_slot = 4 slot 1 = 0-25% load, slot 2 = 26-50% load, slot 3 = 51-75% load,
 * slot 4 = 76-100% load. Remaining unused index can be 0.
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCORING_NUM_ESP_QBSS_SLOTS CFG_INI_UINT( \
	"num_esp_qbss_slots", \
	1, \
	15, \
	4, \
	CFG_VALUE_OR_DEFAULT, \
	"Num ESP QPSS Slots")

/*
 * <ini>
 * esp_qbss_score_idx3_to_0 - percentage for  esp/qbss load for slots 0-3
 * @Min: 0x00000000
 * @Max: 0x64646464
 * @Default: 0x19326432
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
	0x19326432, \
	CFG_VALUE_OR_DEFAULT, \
	"ESP QPSS Score Index 3 to 0")

/*
 * <ini>
 * esp_qbss_score_idx7_to_4 - percentage for  esp/qbss load for slots 4-7
 * @Min: 0x00000000
 * @Max: 0x64646464
 * @Default: 0x0000000A
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
	0x0000000A, \
	CFG_VALUE_OR_DEFAULT, \
	"ESP QPSS Score Index 7 to 4")

/*
 * <ini>
 * esp_qbss_score_idx11_to_8 - percentage for  esp/qbss load for slots 8-11
 * @Min: 0x00000000
 * @Max: 0x64646464
 * @Default: 0x00000000
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
	0x00000000, \
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
 * roam_score_delta_bitmap - bitmap to enable roam triggers on
 * which roam score delta is to be applied during roam candidate
 * selection
 * @Min: 0
 * @Max: 0xffffffff
 * @Default: 0xffffffff
 *
 * Bitmap value of the following roam triggers:
 * ROAM_TRIGGER_REASON_NONE       - B0,
 * ROAM_TRIGGER_REASON_PER        - B1,
 * ROAM_TRIGGER_REASON_BMISS      - B2,
 * ROAM_TRIGGER_REASON_LOW_RSSI   - B3,
 * ROAM_TRIGGER_REASON_HIGH_RSSI  - B4,
 * ROAM_TRIGGER_REASON_PERIODIC   - B5,
 * ROAM_TRIGGER_REASON_MAWC       - B6,
 * ROAM_TRIGGER_REASON_DENSE      - B7,
 * ROAM_TRIGGER_REASON_BACKGROUND - B8,
 * ROAM_TRIGGER_REASON_FORCED     - B9,
 * ROAM_TRIGGER_REASON_BTM        - B10,
 * ROAM_TRIGGER_REASON_UNIT_TEST  - B11,
 * ROAM_TRIGGER_REASON_BSS_LOAD   - B12
 * ROAM_TRIGGER_REASON_DISASSOC   - B13
 * ROAM_TRIGGER_REASON_IDLE_ROAM  - B14
 *
 * When the bit corresponding to a particular roam trigger reason
 * is set, the value of "roam_score_delta" is expected over the
 * roam score of the current connected AP, for that triggered roam
 *
 * Related: None
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ROAM_TRIGGER_BITMAP CFG_INI_UINT( \
			"roam_score_delta_bitmap", \
			0, \
			0xFFFFFFFF, \
			0xFFFFFFFF, \
			CFG_VALUE_OR_DEFAULT, \
			"Bitmap for various roam triggers")

/*
 * <ini>
 * roam_score_delta - Percentage increment in roam score value
 * that is expected from a roaming candidate AP.
 * @Min: 0
 * @Max: 100
 * @Default: 0
 *
 * This ini is used to provide the percentage increment value over roam
 * score for the candidate APs so that they can be preferred over current
 * AP for roaming.
 *
 * Related: None
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ROAM_SCORE_DELTA CFG_INI_UINT( \
			"roam_score_delta", \
			0, \
			100, \
			0, \
			CFG_VALUE_OR_DEFAULT, \
			"candidate AP's percentage roam score delta")

/*
 * <ini>
 * enable_scoring_for_roam - enable/disable scoring logic in FW for candidate
 * selection during roaming
 *
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable/disable scoring logic in FW for candidate
 * selection during roaming.
 *
 * Supported Feature: STA Candidate selection by FW during roaming based on
 * scoring logic.
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ENABLE_SCORING_FOR_ROAM CFG_INI_BOOL( \
		"enable_scoring_for_roam", \
		1, \
		"Enable Scoring for Roam")

/*
 * <cfg>
 * apsd_enabled - Enable automatic power save delivery
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * Supported Feature: Power save
 *
 * Usage: Internal
 *
 * </cfg>
 */
#define CFG_APSD_ENABLED CFG_BOOL( \
		"apsd_enabled", \
		0, \
		"Enable APSD")

/*
 * <ini>
 * candidate_min_rssi_for_disconnect - Candidate AP minimum RSSI in
 * idle roam trigger(in dBm).
 * @Min: -120
 * @Max: 0
 * @Default: -70
 *
 * Minimum RSSI value of the candidate AP to consider it as candidate for
 * roaming when roam trigger is Deauthentication/Disconnection from current
 * AP. This value will be sent to firmware over the WMI_ROAM_AP_PROFILE
 * wmi command in the roam_min_rssi_param_list tlv.
 *
 * Related: enable_idle_roam.
 *
 * Supported Feature: Roaming
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_DISCONNECT_ROAM_TRIGGER_MIN_RSSI CFG_INI_INT( \
		"candidate_min_rssi_for_disconnect", \
		-120, \
		0, \
		-70, \
		CFG_VALUE_OR_DEFAULT, \
		"Minimum RSSI of candidate AP for Disconnect roam trigger")

/*
 * <ini>
 * candidate_min_rssi_for_beacon_miss - Candidate AP minimum RSSI for beacon
 * miss roam trigger (in dBm)
 * @Min: -120
 * @Max: 0
 * @Default: -70
 *
 * Minimum RSSI value of the candidate AP to consider it as candidate for
 * roaming when roam trigger is disconnection from current AP due to beacon
 * miss. This value will be sent to firmware over the WMI_ROAM_AP_PROFILE
 * wmi command in the roam_min_rssi_param_list tlv.
 *
 * Related: None
 *
 * Supported Feature: Roaming
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_BMISS_ROAM_MIN_RSSI CFG_INI_INT( \
	"candidate_min_rssi_for_beacon_miss", \
	-120, \
	0, \
	-70, \
	CFG_VALUE_OR_DEFAULT, \
	"Minimum RSSI of candidate AP for Bmiss roam trigger")

/*
 * <ini>
 * idle_roam_score_delta - Roam score delta value in percentage for idle roam.
 * @Min: 0
 * @Max: 100
 * @Default: 0
 *
 * This ini is used to configure the minimum change in roam score
 * value of the AP to consider it as candidate for
 * roaming when roam trigger is due to idle state of sta.
 * This value will be sent to firmware over the WMI_ROAM_AP_PROFILE wmi
 * command in the roam_score_delta_param_list tlv.
 * Related: None
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_IDLE_ROAM_SCORE_DELTA CFG_INI_UINT( \
		"idle_roam_score_delta", \
		0, \
		100, \
		0, \
		CFG_VALUE_OR_DEFAULT, \
		"Roam score delta for Idle roam trigger")

/*
 * <ini>
 * btm_roam_score_delta - Roam score delta value in percentage for BTM triggered
 * roaming.
 * @Min: 0
 * @Max: 100
 * @Default: 0
 *
 * This ini is used to configure the minimum change in roam score
 * value of the AP to consider it as candidate when the sta is disconnected
 * from the current AP due to BTM kickout.
 * This value will be sent to firmware over the WMI_ROAM_AP_PROFILE wmi
 * command in the roam_score_delta_param_list tlv.
 *
 * Related: None
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_BTM_ROAM_SCORE_DELTA CFG_INI_UINT( \
	"btm_roam_score_delta", \
	0, \
	100, \
	0, \
	CFG_VALUE_OR_DEFAULT, \
	"Roam score delta for BTM roam trigger")

/*
 * <ini>
 * vendor_roam_score_algorithm - Algorithm to calculate AP score
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * By default the value is 0 and default roam algorithm will be used.
 * When the value is 1, the V2 roaming algorithm will be used:
 * For this V2 algo, AP score calculation is based on below equation:
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
#define CFG_VENDOR_ROAM_SCORE_ALGORITHM CFG_INI_UINT( \
	"vendor_roam_score_algorithm", \
	0, \
	1, \
	0, \
	CFG_VALUE_OR_DEFAULT, \
	"Roam candidate selection score algorithm")

#define CFG_SCORING_ALL \
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
	CFG(CFG_ROAM_TRIGGER_BITMAP) \
	CFG(CFG_ROAM_SCORE_DELTA) \
	CFG(CFG_ENABLE_SCORING_FOR_ROAM) \
	CFG(CFG_APSD_ENABLED) \
	CFG(CFG_DISCONNECT_ROAM_TRIGGER_MIN_RSSI) \
	CFG(CFG_BMISS_ROAM_MIN_RSSI) \
	CFG(CFG_IDLE_ROAM_SCORE_DELTA) \
	CFG(CFG_BTM_ROAM_SCORE_DELTA) \
	CFG(CFG_VENDOR_ROAM_SCORE_ALGORITHM) \

#endif /* __CFG_MLME_SCORING_H */
