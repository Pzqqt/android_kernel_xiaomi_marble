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
 * DOC: This file contains configuration definitions for MLME LFR.
 */

#ifndef CFG_MLME_LFR_H__
#define CFG_MLME_LFR_H__

/*
 * <ini>
 * mawc_roam_enabled - Enable/Disable MAWC during roaming
 * @Min: 0 - Disabled
 * @Max: 1 - Enabled
 * @Default: 0
 *
 * This ini is used to control MAWC during roaming.
 *
 * Related: MAWCEnabled.
 *
 * Supported Feature: MAWC Roaming
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_LFR_MAWC_ROAM_ENABLED CFG_INI_BOOL( \
	"mawc_roam_enabled", \
	0, \
	"Enable/Disable MAWC during roaming")

/*
 * <ini>
 * mawc_roam_traffic_threshold - Configure traffic threshold
 * @Min: 0
 * @Max: 0xFFFFFFFF
 * @Default: 300
 *
 * This ini is used to configure the data traffic load in kbps to
 * register CMC.
 *
 * Related: mawc_roam_enabled.
 *
 * Supported Feature: MAWC Roaming
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_LFR_MAWC_ROAM_TRAFFIC_THRESHOLD CFG_INI_UINT( \
	"mawc_roam_traffic_threshold", \
	0, \
	0xFFFFFFFF, \
	300, \
	CFG_VALUE_OR_DEFAULT, \
	"Configure traffic threshold")

/*
 * <ini>
 * mawc_roam_ap_rssi_threshold - Best AP RSSI threshold
 * @Min: -120
 * @Max: 0
 * @Default: -66
 *
 * This ini is used to specify the RSSI threshold to scan for the AP.
 *
 * Related: mawc_roam_enabled.
 *
 * Supported Feature: MAWC Roaming
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_LFR_MAWC_ROAM_AP_RSSI_THRESHOLD CFG_INI_INT( \
	"mawc_roam_ap_rssi_threshold", \
	-120, \
	0, \
	-66, \
	CFG_VALUE_OR_DEFAULT, \
	"Best AP RSSI threshold")

/*
 * <ini>
 * mawc_roam_rssi_high_adjust - Adjust MAWC roam high RSSI
 * @Min: 3
 * @Max: 5
 * @Default: 5
 *
 * This ini is used for high RSSI threshold adjustment in stationary state
 * to suppress the scan.
 *
 * Related: mawc_roam_enabled.
 *
 * Supported Feature: MAWC Roaming
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_LFR_MAWC_ROAM_RSSI_HIGH_ADJUST CFG_INI_UINT( \
	"mawc_roam_ap_rssi_threshold", \
	3, \
	5, \
	5, \
	CFG_VALUE_OR_DEFAULT, \
	"Adjust MAWC roam high RSSI")

/*
 * <ini>
 * mawc_roam_rssi_low_adjust - Adjust MAWC roam low RSSI
 * @Min: 3
 * @Max: 5
 * @Default: 5
 *
 * This ini is used for low RSSI threshold adjustment in stationary state
 * to suppress the scan.
 *
 * Related: mawc_roam_enabled.
 *
 * Supported Feature: MAWC Roaming
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_LFR_MAWC_ROAM_RSSI_LOW_ADJUST CFG_INI_UINT( \
	"mawc_roam_rssi_low_adjust", \
	3, \
	5, \
	5, \
	CFG_VALUE_OR_DEFAULT, \
	"Adjust MAWC roam low RSSI")

/*
 * <ini>
 * rssi_abs_thresh - The min RSSI of the candidate AP to consider roam
 * @Min: -96
 * @Max: 0
 * @Default: 0
 *
 * The RSSI value of the candidate AP should be higher than rssi_abs_thresh
 * to roam to the AP. 0 means no absolute minimum RSSI is required.
 *
 * Related: None
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_LFR_ROAM_RSSI_ABS_THRESHOLD CFG_INI_INT( \
	"rssi_abs_thresh", \
	-96, \
	0, \
	0, \
	CFG_VALUE_OR_DEFAULT, \
	"The min RSSI of the candidate AP to consider roam")

/*
 * <ini>
 * lookup_threshold_5g_offset - Lookup threshold offset for 5G band
 * @Min: -120
 * @Max: 120
 * @Default: 0
 *
 * This ini is used to set the 5G band lookup threshold for roaming.
 * It depends on another INI which is gNeighborLookupThreshold.
 * gNeighborLookupThreshold is a legacy INI item which will be used to
 * set the RSSI lookup threshold for both 2G and 5G bands. If the
 * user wants to setup a different threshold for a 5G band, then user
 * can use this offset value which will be summed up to the value of
 * gNeighborLookupThreshold and used for 5G
 * e.g: gNeighborLookupThreshold = -76dBm
 *      lookup_threshold_5g_offset = 6dBm
 *      Then the 5G band will be configured to -76+6 = -70dBm
 * A default value of Zero to lookup_threshold_5g_offset will keep the
 * thresholds same for both 2G and 5G bands
 *
 * Related: gNeighborLookupThreshold
 *
 * Supported Feature: Roaming
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_LFR_5G_RSSI_THRESHOLD_OFFSET CFG_INI_INT( \
	"lookup_threshold_5g_offset", \
	-120, \
	120, \
	0, \
	CFG_VALUE_OR_DEFAULT, \
	"Lookup threshold offset for 5G band")

/*
 * <ini>
 * gEnableFastRoamInConcurrency - Enable LFR roaming on STA during concurrency
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This INI is used to enable Legacy fast roaming(LFR) on STA link during
 * concurrent sessions.
 *
 * Related: None
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_LFR_ENABLE_FAST_ROAM_IN_CONCURRENCY CFG_INI_BOOL( \
	"gEnableFastRoamInConcurrency", \
	1, \
	"Enable LFR roaming on STA during concurrency")

/*
 * <ini>
 * gRoamOffloadEnabled - enable/disable roam offload feature
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This INI is used to enable/disable roam offload feature
 *
 * Related: None
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_LFR3_ROAMING_OFFLOAD CFG_INI_BOOL( \
	"gRoamOffloadEnabled", \
	1, \
	"enable roam offload")

/*
 * <ini>
 * gEnableEarlyStopScan - Set early stop scan
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to set early stop scan. Early stop
 * scan is a feature for roaming to stop the scans at
 * an early stage as soon as we find a better AP to roam.
 * This would make the roaming happen quickly.
 *
 * Related: None
 *
 * Supported Feature: LFR Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_LFR_EARLY_STOP_SCAN_ENABLE CFG_INI_BOOL( \
	"gEnableEarlyStopScan", \
	1, \
	"Set early stop scan")

/*
 * <ini>
 * gEarlyStopScanMinThreshold - Set early stop scan min
 * threshold
 * @Min: -80
 * @Max: -70
 * @Default: -73
 *
 * This ini is used to set the early stop scan minimum
 * threshold. Early stop scan minimum threshold is the
 * minimum threshold to be considered for stopping the
 * scan. The algorithm starts with a scan on the greedy
 * channel list with the maximum threshold and steps down
 * the threshold by 20% for each further channel. It can
 * step down on each channel but cannot go lower than the
 * minimum threshold.
 *
 * Related: None
 *
 * Supported Feature: Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_LFR_EARLY_STOP_SCAN_MIN_THRESHOLD CFG_INI_INT( \
	"gEarlyStopScanMinThreshold", \
	-80, \
	-70, \
	-73, \
	CFG_VALUE_OR_DEFAULT, \
	"Set early stop scan min")

/*
 * <ini>
 * gEarlyStopScanMaxThreshold - Set early stop scan max
 * threshold
 * @Min: -60
 * @Max: -40
 * @Default: -43
 *
 * This ini is used to set the the early stop scan maximum
 * threshold at which the candidate AP should be to be
 * qualified as a potential roam candidate and good enough
 * to stop the roaming scan.
 *
 * Related: None
 *
 * Supported Feature: Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_LFR_EARLY_STOP_SCAN_MAX_THRESHOLD CFG_INI_INT( \
	"gEarlyStopScanMaxThreshold", \
	-60, \
	-40, \
	-43, \
	CFG_VALUE_OR_DEFAULT, \
	"Set early stop scan max")

/*
 * <ini>
 * gFirstScanBucketThreshold - Set first scan bucket
 * threshold
 * @Min: -50
 * @Max: -30
 * @Default: -30
 *
 * This ini will configure the first scan bucket
 * threshold to the mentioned value and all the AP's which
 * have RSSI under this threshold will fall under this
 * bucket. This configuration item used to tweak and
 * test the input for internal algorithm.
 *
 * Related: None
 *
 * Supported Feature: Scan
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_LFR_FIRST_SCAN_BUCKET_THRESHOLD CFG_INI_INT( \
	"gFirstScanBucketThreshold", \
	-50, \
	-30, \
	-30, \
	CFG_VALUE_OR_DEFAULT, \
	"Set first scan bucket")

/*
 * <ini>
 * gLFRSubnetDetectionEnable - Enable LFR3 subnet detection
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * Enable IP subnet detection during legacy fast roming version 3. Legacy fast
 * roaming could roam across IP subnets without host processors' knowledge.
 * This feature enables firmware to wake up the host processor if it
 * successfully determines change in the IP subnet. Change in IP subnet could
 * potentially cause disruption in IP connnectivity if IP address is not
 * refreshed.
 *
 * Related: None
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_LFR3_ENABLE_SUBNET_DETECTION CFG_INI_BOOL( \
	"gLFRSubnetDetectionEnable", \
	1, \
	"Set early stop scan")

/*
 * <ini>
 * gtraffic_threshold - Dense traffic threshold
 * @Min: 0
 * @Max: 0xffffffff
 * @Default: 400
 *
 * Dense traffic threshold
 * traffic threshold required for dense roam scan
 * Measured in kbps
 *
 * Related: None
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_LFR_ROAM_DENSE_TRAFFIC_THRESHOLD CFG_INI_UINT( \
	"gtraffic_threshold", \
	0, \
	0xffffffff, \
	400, \
	CFG_VALUE_OR_DEFAULT, \
	"Dense traffic threshold")

/*
 * <ini>
 * groam_dense_rssi_thresh_offset - Sets dense roam RSSI threshold diff
 * @Min: 0
 * @Max: 20
 * @Default: 10
 *
 * This INI is used to set offset value from normal RSSI threshold to dense
 * RSSI threshold FW will optimize roaming based on new RSSI threshold once
 * it detects dense environment.
 *
 * Related: None
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_LFR_ROAM_DENSE_RSSI_THRE_OFFSET CFG_INI_UINT( \
	"groam_dense_rssi_thresh_offset", \
	0, \
	20, \
	10, \
	CFG_VALUE_OR_DEFAULT, \
	"Dense traffic threshold")

/*
 * <ini>
 * groam_dense_min_aps - Sets minimum number of AP for dense roam
 * @Min: 1
 * @Max: 5
 * @Default: 3
 *
 * Minimum number of APs required for dense roam. FW will consider
 * environment as dense once it detects #APs operating is more than
 * groam_dense_min_aps.
 *
 * Related: None
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_LFR_ROAM_DENSE_MIN_APS CFG_INI_UINT( \
	"groam_dense_min_aps", \
	1, \
	5, \
	3, \
	CFG_VALUE_OR_DEFAULT, \
	"Sets minimum number of AP for dense roam")

/*
 * <ini>
 * roam_bg_scan_bad_rssi_thresh - RSSI threshold for background roam
 * @Min: -96
 * @Max: 0
 * @Default: -76
 *
 * If the DUT is connected to an AP with weak signal, then the bad RSSI
 * threshold will be used as an opportunity to use the scan results
 * from other scan clients and try to roam if there is a better AP
 * available in the environment.
 *
 * Related: None
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_LFR_ROAM_BG_SCAN_BAD_RSSI_THRESHOLD CFG_INI_INT( \
	"roam_bg_scan_bad_rssi_thresh", \
	-96, \
	0, \
	-76, \
	CFG_VALUE_OR_DEFAULT, \
	"RSSI threshold for background roam")

/*
 * <ini>
 * roam_bg_scan_client_bitmap - Bitmap used to identify the scan clients
 * @Min: 0
 * @Max: 0x7FF
 * @Default: 0x424
 *
 * This bitmap is used to define the client scans that need to be used
 * by the roaming module to perform a background roaming.
 * Currently supported bit positions are as follows:
 * Bit 0 is reserved in the firmware.
 * WMI_SCAN_CLIENT_NLO - 1
 * WMI_SCAN_CLIENT_EXTSCAN - 2
 * WMI_SCAN_CLIENT_ROAM - 3
 * WMI_SCAN_CLIENT_P2P - 4
 * WMI_SCAN_CLIENT_LPI - 5
 * WMI_SCAN_CLIENT_NAN - 6
 * WMI_SCAN_CLIENT_ANQP - 7
 * WMI_SCAN_CLIENT_OBSS - 8
 * WMI_SCAN_CLIENT_PLM - 9
 * WMI_SCAN_CLIENT_HOST - 10
 *
 * Related: None
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_LFR_ROAM_BG_SCAN_CLIENT_BITMAP CFG_INI_UINT( \
	"roam_bg_scan_client_bitmap", \
	0, \
	0x7FF, \
	0x424, \
	CFG_VALUE_OR_DEFAULT, \
	"Bitmap used to identify the scan clients")

/*
 * <ini>
 * roam_bad_rssi_thresh_offset_2g - RSSI threshold offset for 2G to 5G roam
 * @Min: 0
 * @Max: 86
 * @Default: 40
 *
 * If the DUT is connected to an AP with weak signal in 2G band, then the
 * bad RSSI offset for 2g would be used as offset from the bad RSSI
 * threshold configured and then use the resulting rssi for an opportunity
 * to use the scan results from other scan clients and try to roam to
 * 5G Band ONLY if there is a better AP available in the environment.
 *
 * For example if the roam_bg_scan_bad_rssi_thresh is -76 and
 * roam_bad_rssi_thresh_offset_2g is 40 then the difference of -36 would be
 * used as a trigger to roam to a 5G AP if DUT initially connected to a 2G AP
 *
 * Related: roam_bg_scan_bad_rssi_thresh
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_LFR_ROAM_BG_SCAN_BAD_RSSI_OFFSET_2G CFG_INI_UINT( \
	"roam_bad_rssi_thresh_offset_2g", \
	0, \
	86, \
	40, \
	CFG_VALUE_OR_DEFAULT, \
	"RSSI threshold offset for 2G to 5G roam")

/*
 * <ini>
 * roamscan_adaptive_dwell_mode - Sets dwell time adaptive mode
 * @Min: 0
 * @Max: 4
 * @Default: 1
 *
 * This parameter will set the algo used in dwell time optimization during
 * roam scan. see enum scan_dwelltime_adaptive_mode.
 * Acceptable values for this:
 * 0: Default (Use firmware default mode)
 * 1: Conservative optimization
 * 2: Moderate optimization
 * 3: Aggressive optimization
 * 4: Static
 *
 * Related: None
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_LFR_ADAPTIVE_ROAMSCAN_DWELL_MODE CFG_INI_UINT( \
	"roamscan_adaptive_dwell_mode", \
	0, \
	4, \
	1, \
	CFG_VALUE_OR_DEFAULT, \
	"Sets dwell time adaptive mode")

/*
 * <ini>
 * gper_roam_enabled - To enabled/disable PER based roaming in FW
 * @Min: 0
 * @Max: 3
 * @Default: 3
 *
 * This ini is used to enable/disable Packet error based roaming, enabling this
 * will cause DUT to monitor Tx and Rx traffic and roam to a better candidate
 * if current is not good enough.
 *
 * Values supported:
 * 0: disabled
 * 1: enabled for Rx traffic
 * 2: enabled for Tx traffic
 * 3: enabled for Tx and Rx traffic
 *
 * Related: gper_roam_high_rate_th, gper_roam_low_rate_th,
 *          gper_roam_th_percent, gper_roam_rest_time
 *
 * Supported Feature: LFR-3.0
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_LFR_PER_ROAM_ENABLE CFG_INI_UINT( \
	"gper_roam_enabled", \
	0, \
	3, \
	3, \
	CFG_VALUE_OR_DEFAULT, \
	"To enabled/disable PER based roaming in FW")

/*
 * <ini>
 * gper_roam_high_rate_th - Rate at which PER based roam will stop
 * @Min: 1 Mbps
 * @Max: 0xffffffff
 * @Default: 40 Mbps
 *
 * This ini is used to define the data rate in mbps*10 at which FW will stop
 * monitoring the traffic for PER based roam.
 *
 * Related: gper_roam_enabled, gper_roam_low_rate_th,
 *          gper_roam_th_percent, gper_roam_rest_time
 *
 * Supported Feature: LFR-3.0
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_LFR_PER_ROAM_CONFIG_HIGH_RATE_TH CFG_INI_UINT( \
	"gper_roam_high_rate_th", \
	10, \
	0xffffffff, \
	400, \
	CFG_VALUE_OR_DEFAULT, \
	"Rate at which PER based roam will stop")

/*
 * <ini>
 * gper_roam_low_rate_th - Rate at which FW starts considering traffic for PER
 * based roam.
 *
 * @Min: 1 Mbps
 * @Max: 0xffffffff
 * @Default: 20 Mbps
 *
 * This ini is used to define the rate in mbps*10 at which FW starts considering
 * traffic for PER based roam, if gper_roam_th_percent of data is below this
 * rate, FW will issue a roam scan.
 *
 * Related: gper_roam_enabled, gper_roam_high_rate_th,
 *          gper_roam_th_percent, gper_roam_rest_time
 *
 * Supported Feature: LFR-3.0
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_LFR_PER_ROAM_CONFIG_LOW_RATE_TH CFG_INI_UINT( \
	"gper_roam_low_rate_th", \
	10, \
	0xffffffff, \
	200, \
	CFG_VALUE_OR_DEFAULT, \
	"Rate at which FW starts considering traffic for PER")

/*
 * <ini>
 * gper_roam_th_percent - Percentage at which FW will issue a roam scan if
 * traffic is below gper_roam_low_rate_th rate.
 *
 * @Min: 10%
 * @Max: 100%
 * @Default: 60%
 *
 * This ini is used to define the percentage at which FW will issue a roam scan
 * if traffic is below gper_roam_low_rate_th rate.
 *
 * Related: gper_roam_enabled, gper_roam_high_rate_th,
 *          gper_roam_high_rate_th, gper_roam_rest_time
 *
 * Supported Feature: LFR-3.0
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_LFR_PER_ROAM_CONFIG_RATE_TH_PERCENT CFG_INI_UINT( \
	"gper_roam_th_percent", \
	10, \
	100, \
	60, \
	CFG_VALUE_OR_DEFAULT, \
	"Percentage at which FW will issue a roam scan")

/*
 * <ini>
 * gper_roam_rest_time - Time for which FW will wait once it issues a
 * roam scan.
 *
 * @Min: 10 seconds
 * @Max: 3600 seconds
 * @Default: 300 seconds
 *
 * This ini is used to define the time for which FW will wait once it issues a
 * PER based roam scan.
 *
 * Related: gper_roam_enabled, gper_roam_high_rate_th,
 *          gper_roam_high_rate_th, gper_roam_th_percent
 *
 * Supported Feature: LFR-3.0
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_LFR_PER_ROAM_REST_TIME CFG_INI_UINT( \
	"gper_roam_rest_time", \
	10, \
	3600, \
	300, \
	CFG_VALUE_OR_DEFAULT, \
	"Time for which FW will wait once it issues a roam scan")

/*
 * <ini>
 * gper_roam_mon_time - Minimum time required in seconds to
 * be considered as valid scenario for PER based roam
 * @Min: 5
 * @Max: 25
 * @Default: 25
 *
 * This ini is used to define minimum time in seconds for which DUT has
 * collected the PER stats before it can consider the stats hysteresis to be
 * valid for PER based scan.
 * DUT collects following information during this period:
 *     1. % of packets below gper_roam_low_rate_th
 *     2. # packets above gper_roam_high_rate_th
 * if DUT gets (1) greater than gper_roam_th_percent and (2) is zero during
 * this period, it triggers PER based roam scan.
 *
 * Related: gper_roam_enabled, gper_roam_high_rate_th, gper_roam_low_rate_th,
 *          gper_roam_th_percent, gper_roam_rest_time
 *
 * Supported Feature: LFR-3.0
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_LFR_PER_ROAM_MONITOR_TIME CFG_INI_UINT( \
	"gper_roam_mon_time", \
	5, \
	25, \
	25, \
	CFG_VALUE_OR_DEFAULT, \
	"Minimum time to be considered as valid scenario for PER based roam")

/*
 * <ini>
 * gper_min_rssi_threshold_for_roam -  Minimum roamable AP RSSI for
 * candidate selection for PER based roam
 * @Min: 0
 * @Max: 96
 * @Default: 83
 *
 * Minimum roamable AP RSSI for candidate selection for PER based roam
 *
 * Related: gper_roam_enabled, gper_roam_high_rate_th, gper_roam_low_rate_th,
 *          gper_roam_th_percent, gper_roam_rest_time
 *
 * Supported Feature: LFR-3.0
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_LFR_PER_ROAM_MIN_CANDIDATE_RSSI CFG_INI_UINT( \
	"gper_min_rssi_threshold_for_roam", \
	10, \
	96, \
	83, \
	CFG_VALUE_OR_DEFAULT, \
	"Minimum roamable AP RSSI for candidate selection for PER based roam")

/*
 * <ini>
 * groam_disallow_duration - disallow duration before roaming
 * @Min: 0
 * @Max: 3600
 * @Default: 30
 *
 * This ini is used to configure how long LCA[Last Connected AP] AP will
 * be disallowed before it can be a roaming candidate again, in units of
 * seconds.
 *
 * Related: LFR
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_LFR3_ROAM_DISALLOW_DURATION CFG_INI_UINT( \
	"groam_disallow_duration", \
	0, \
	3600, \
	30, \
	CFG_VALUE_OR_DEFAULT, \
	"disallow duration before roaming")

/*
 * <ini>
 * grssi_channel_penalization - RSSI penalization
 * @Min: 0
 * @Max: 15
 * @Default: 5
 *
 * This ini is used to configure RSSI that will be penalized if candidate(s)
 * are found to be in the same channel as disallowed AP's, in units of db.
 *
 * Related: LFR
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_LFR3_ROAM_RSSI_CHANNEL_PENALIZATION CFG_INI_UINT( \
	"grssi_channel_penalization", \
	0, \
	15, \
	5, \
	CFG_VALUE_OR_DEFAULT, \
	"RSSI penalization")

/*
 * <ini>
 * groam_num_disallowed_aps - Max number of AP's to maintain in LCA list
 * @Min: 0
 * @Max: 8
 * @Default: 3
 *
 * This ini is used to set the maximum number of AP's to be maintained
 * in LCA [Last Connected AP] list.
 *
 * Related: LFR
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_LFR3_ROAM_NUM_DISALLOWED_APS CFG_INI_UINT( \
	"groam_num_disallowed_aps", \
	0, \
	8, \
	3, \
	CFG_VALUE_OR_DEFAULT, \
	"Max number of AP's to maintain in LCA list")

/*
 * <ini>
 * enable_5g_band_pref - Enable preference for 5G from INI.
 * @Min: 0
 * @Max: 1
 * @Default: 0
 * This ini is used to enable 5G preference parameters.
 *
 * Related: 5g_rssi_boost_threshold, 5g_rssi_boost_factor, 5g_max_rssi_boost
 * 5g_rssi_penalize_threshold, 5g_rssi_penalize_factor, 5g_max_rssi_penalize
 *
 * Supported Feature: 5G band preference
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_LFR_ENABLE_5G_BAND_PREF CFG_INI_BOOL( \
	"enable_5g_band_pref", \
	0, \
	"Enable preference for 5G from INI")

/*
 * <ini>
 * 5g_rssi_boost_threshold - A_band_boost_threshold above which 5G is favored.
 * @Min: -55
 * @Max: -70
 * @Default: -60
 * This ini is used to set threshold for 5GHz band preference.
 *
 * Related: 5g_rssi_boost_factor, 5g_max_rssi_boost
 * 5g_rssi_penalize_threshold, 5g_rssi_penalize_factor, 5g_max_rssi_penalize
 *
 * Supported Feature: 5G band preference
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_LFR_5G_RSSI_BOOST_THRESHOLD CFG_INI_INT( \
	"5g_rssi_boost_threshold", \
	-55, \
	-70, \
	-60, \
	CFG_VALUE_OR_DEFAULT, \
	"A_band_boost_threshold above which 5 GHz is favored")

/*
 * <ini>
 * 5g_rssi_boost_factor - Factor by which 5GHz RSSI is boosted.
 * @Min: 0
 * @Max: 2
 * @Default: 1
 * This ini is used to set the 5Ghz boost factor.
 *
 * Related: 5g_rssi_boost_threshold, 5g_max_rssi_boost
 * 5g_rssi_penalize_threshold, 5g_rssi_penalize_factor, 5g_max_rssi_penalize
 *
 * Supported Feature: 5G band preference
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_LFR_5G_RSSI_BOOST_FACTOR CFG_INI_UINT( \
	"5g_rssi_boost_factor", \
	0, \
	2, \
	1, \
	CFG_VALUE_OR_DEFAULT, \
	"Factor by which 5GHz RSSI is boosted")

/*
 * <ini>
 * 5g_max_rssi_boost - Maximum boost that can be applied to 5GHz RSSI.
 * @Min: 0
 * @Max: 20
 * @Default: 10
 * This ini is used to set maximum boost which can be given to a 5Ghz network.
 *
 * Related: 5g_rssi_boost_threshold, 5g_rssi_boost_factor
 * 5g_rssi_penalize_threshold, 5g_rssi_penalize_factor, 5g_max_rssi_penalize
 *
 * Supported Feature: 5G band preference
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_LFR_5G_MAX_RSSI_BOOST CFG_INI_UINT( \
	"5g_max_rssi_boost", \
	0, \
	20, \
	10, \
	CFG_VALUE_OR_DEFAULT, \
	"Maximum boost that can be applied to 5GHz RSSI")

/*
 * <ini>
 * 5g_rssi_penalize_threshold - A_band_penalize_threshold above which
 * 5 GHz is not favored.
 * @Min: -65
 * @Max: -80
 * @Default: -70
 * This ini is used to set threshold for 5GHz band preference.
 *
 * Related: 5g_rssi_penalize_factor, 5g_max_rssi_penalize
 * 5g_rssi_boost_threshold, 5g_rssi_boost_factor, 5g_max_rssi_boost
 *
 * Supported Feature: 5G band preference
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_LFR_5G_RSSI_PENALIZE_THRESHOLD CFG_INI_INT( \
	"5g_rssi_penalize_threshold", \
	-65, \
	-80, \
	-70, \
	CFG_VALUE_OR_DEFAULT, \
	"A_band_penalize_threshold above which 5 GHz is not favored")

/*
 * <ini>
 * 5g_rssi_penalize_factor - Factor by which 5GHz RSSI is penalizeed.
 * @Min: 0
 * @Max: 2
 * @Default: 1
 * This ini is used to set the 5Ghz penalize factor.
 *
 * Related: 5g_rssi_penalize_threshold, 5g_max_rssi_penalize
 * 5g_rssi_boost_threshold, 5g_rssi_boost_factor, 5g_max_rssi_boost
 *
 * Supported Feature: 5G band preference
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_LFR_5G_RSSI_PENALIZE_FACTOR CFG_INI_UINT( \
	"5g_rssi_penalize_factor", \
	0, \
	2, \
	1, \
	CFG_VALUE_OR_DEFAULT, \
	"Factor by which 5GHz RSSI is penalizeed")

/*
 * <ini>
 * 5g_max_rssi_penalize - Maximum penalty that can be applied to 5GHz RSSI.
 * @Min: 0
 * @Max: 20
 * @Default: 10
 * This ini is used to set maximum penalty which can be given to a 5Ghz network.
 *
 * Related: 5g_rssi_penalize_threshold, 5g_rssi_penalize_factor
 * 5g_rssi_boost_threshold, 5g_rssi_boost_factor, 5g_max_rssi_boost
 *
 * Supported Feature: 5G band preference
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_LFR_5G_MAX_RSSI_PENALIZE CFG_INI_UINT( \
	"5g_max_rssi_penalize", \
	0, \
	20, \
	10, \
	CFG_VALUE_OR_DEFAULT, \
	"Maximum penalty that can be applied to 5GHz RSSI")

/*
 * max_num_pre_auth - Configure max number of pre-auth
 * @Min: 0
 * @Max: 256
 * @Default: 64
 *
 * This ini is used to configure the data max number of pre-auth
 *
 * Usage: Internal
 *
 */
#define CFG_LFR_MAX_NUM_PRE_AUTH CFG_UINT( \
	"max_num_pre_auth", \
	0, \
	256, \
	64, \
	CFG_VALUE_OR_DEFAULT, \
	"")

#define CFG_LFR_ALL \
	CFG(CFG_LFR_MAWC_ROAM_ENABLED) \
	CFG(CFG_LFR_MAWC_ROAM_TRAFFIC_THRESHOLD) \
	CFG(CFG_LFR_MAWC_ROAM_AP_RSSI_THRESHOLD) \
	CFG(CFG_LFR_MAWC_ROAM_RSSI_HIGH_ADJUST) \
	CFG(CFG_LFR_MAWC_ROAM_RSSI_LOW_ADJUST) \
	CFG(CFG_LFR_ROAM_RSSI_ABS_THRESHOLD) \
	CFG(CFG_LFR_5G_RSSI_THRESHOLD_OFFSET) \
	CFG(CFG_LFR_ENABLE_FAST_ROAM_IN_CONCURRENCY) \
	CFG(CFG_LFR3_ROAMING_OFFLOAD) \
	CFG(CFG_LFR_EARLY_STOP_SCAN_ENABLE) \
	CFG(CFG_LFR_EARLY_STOP_SCAN_MIN_THRESHOLD) \
	CFG(CFG_LFR_EARLY_STOP_SCAN_MAX_THRESHOLD) \
	CFG(CFG_LFR_FIRST_SCAN_BUCKET_THRESHOLD) \
	CFG(CFG_LFR3_ENABLE_SUBNET_DETECTION) \
	CFG(CFG_LFR_ROAM_DENSE_TRAFFIC_THRESHOLD) \
	CFG(CFG_LFR_ROAM_DENSE_RSSI_THRE_OFFSET) \
	CFG(CFG_LFR_ROAM_DENSE_MIN_APS) \
	CFG(CFG_LFR_ROAM_BG_SCAN_BAD_RSSI_THRESHOLD) \
	CFG(CFG_LFR_ROAM_BG_SCAN_CLIENT_BITMAP) \
	CFG(CFG_LFR_ROAM_BG_SCAN_BAD_RSSI_OFFSET_2G) \
	CFG(CFG_LFR_ADAPTIVE_ROAMSCAN_DWELL_MODE) \
	CFG(CFG_LFR_PER_ROAM_ENABLE) \
	CFG(CFG_LFR_PER_ROAM_CONFIG_HIGH_RATE_TH) \
	CFG(CFG_LFR_PER_ROAM_CONFIG_LOW_RATE_TH) \
	CFG(CFG_LFR_PER_ROAM_CONFIG_RATE_TH_PERCENT) \
	CFG(CFG_LFR_PER_ROAM_REST_TIME) \
	CFG(CFG_LFR_PER_ROAM_MONITOR_TIME) \
	CFG(CFG_LFR_PER_ROAM_MIN_CANDIDATE_RSSI) \
	CFG(CFG_LFR3_ROAM_DISALLOW_DURATION) \
	CFG(CFG_LFR3_ROAM_RSSI_CHANNEL_PENALIZATION) \
	CFG(CFG_LFR3_ROAM_NUM_DISALLOWED_APS) \
	CFG(CFG_LFR_ENABLE_5G_BAND_PREF) \
	CFG(CFG_LFR_5G_RSSI_BOOST_THRESHOLD) \
	CFG(CFG_LFR_5G_RSSI_BOOST_FACTOR) \
	CFG(CFG_LFR_5G_MAX_RSSI_BOOST) \
	CFG(CFG_LFR_5G_RSSI_PENALIZE_THRESHOLD) \
	CFG(CFG_LFR_5G_RSSI_PENALIZE_FACTOR) \
	CFG(CFG_LFR_5G_MAX_RSSI_PENALIZE) \
	CFG(CFG_LFR_MAX_NUM_PRE_AUTH)

#endif /* CFG_MLME_LFR_H__ */
