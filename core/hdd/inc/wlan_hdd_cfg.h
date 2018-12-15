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

#if !defined(HDD_CONFIG_H__)
#define HDD_CONFIG_H__

/**
 *
 * DOC: wlan_hdd_config.h
 *
 * WLAN Adapter Configuration functions
 */

/* $HEADER$ */

/* Include files */
#include <wlan_hdd_includes.h>
#include <wlan_hdd_wmm.h>
#include <qdf_types.h>
#include <csr_api.h>
#include <sap_api.h>
#include "osapi_linux.h"
#include <wmi_unified.h>
#include "wlan_pmo_hw_filter_public_struct.h"
#include "wlan_action_oui_public_struct.h"
#include "hdd_config.h"

struct hdd_context;

#define CFG_DP_RPS_RX_QUEUE_CPU_MAP_LIST_LEN 30

#define FW_MODULE_LOG_LEVEL_STRING_LENGTH  (512)
#define TX_SCHED_WRR_PARAMS_NUM            (5)

#ifdef DHCP_SERVER_OFFLOAD
#define IPADDR_NUM_ENTRIES     (4)
#define IPADDR_STRING_LENGTH   (16)
#endif

/* Number of items that can be configured */
#define MAX_CFG_INI_ITEMS   1024

/* Defines for all of the things we read from the configuration (registry). */
/*
 * <ini>
 * gEnableConnectedScan - Will enable or disable scan in connected state
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable or disable the scanning in
 * Connected state
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: External
 *
 * <ini>
 */

#define CFG_ENABLE_CONNECTED_SCAN_NAME        "gEnableConnectedScan"
#define CFG_ENABLE_CONNECTED_SCAN_MIN         (0)
#define CFG_ENABLE_CONNECTED_SCAN_MAX         (1)
#define CFG_ENABLE_CONNECTED_SCAN_DEFAULT     (1)

/*
 * <ini>
 * enable_11d_in_world_mode - enable 11d in world mode
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini enables 11d in world mode, irrespective of value of
 * g11dSupportEnabled
 *
 * Usage: External
 *
 * </ini>
 */
 #define CFG_ENABLE_11D_IN_WORLD_MODE_NAME "enable_11d_in_world_mode"
 #define CFG_ENABLE_11D_IN_WORLD_MODE_MIN     (0)
 #define CFG_ENABLE_11D_IN_WORLD_MODE_MAX     (1)
 #define CFG_ENABLE_11D_IN_WORLD_MODE_DEFAULT (0)

#define CFG_REG_CHANGE_DEF_COUNTRY_NAME          "gRegulatoryChangeCountry"
#define CFG_REG_CHANGE_DEF_COUNTRY_DEFAULT       (0)
#define CFG_REG_CHANGE_DEF_COUNTRY_MIN           (0)
#define CFG_REG_CHANGE_DEF_COUNTRY_MAX           (1)

/*
 * <ini>
 * gAdvertiseConcurrentOperation - Iface combination advertising
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to control whether driver should indicate to kernel
 * wiphy layer the combination of all its interfaces' supportability.
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_ADVERTISE_CONCURRENT_OPERATION_NAME    "gAdvertiseConcurrentOperation"
#define CFG_ADVERTISE_CONCURRENT_OPERATION_DEFAULT (1)
#define CFG_ADVERTISE_CONCURRENT_OPERATION_MIN     (0)
#define CFG_ADVERTISE_CONCURRENT_OPERATION_MAX     (1)

enum hdd_dot11_mode {
	eHDD_DOT11_MODE_AUTO = 0,       /* covers all things we support */
	eHDD_DOT11_MODE_abg,    /* 11a/b/g only, no HT, no proprietary */
	eHDD_DOT11_MODE_11b,
	eHDD_DOT11_MODE_11g,
	eHDD_DOT11_MODE_11n,
	eHDD_DOT11_MODE_11g_ONLY,
	eHDD_DOT11_MODE_11n_ONLY,
	eHDD_DOT11_MODE_11b_ONLY,
	eHDD_DOT11_MODE_11ac_ONLY,
	eHDD_DOT11_MODE_11ac,
	eHDD_DOT11_MODE_11a,
	eHDD_DOT11_MODE_11ax_ONLY,
	eHDD_DOT11_MODE_11ax,
};

/*
 * <ini>
 * gChannelBondingMode24GHz - Configures Channel Bonding in 24 GHz
 * @Min: 0
 * @Max: 10
 * @Default: 0
 *
 * This ini is used to set default channel bonding mode 24GHZ
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_CHANNEL_BONDING_MODE_24GHZ_NAME    "gChannelBondingMode24GHz"
#define CFG_CHANNEL_BONDING_MODE_MIN           WNI_CFG_CHANNEL_BONDING_MODE_STAMIN
#define CFG_CHANNEL_BONDING_MODE_MAX           WNI_CFG_CHANNEL_BONDING_MODE_STAMAX
#define CFG_CHANNEL_BONDING_MODE_DEFAULT       WNI_CFG_CHANNEL_BONDING_MODE_STADEF

/*
 * <ini>
 * gChannelBondingMode5GHz - Configures Channel Bonding in 5 GHz
 * @Min: 0
 * @Max: 10
 * @Default: 0
 *
 * This ini is used to set default channel bonding mode 5GHZ
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_CHANNEL_BONDING_MODE_5GHZ_NAME     "gChannelBondingMode5GHz"
#define CFG_CHANNEL_BONDING_MODE_MIN           WNI_CFG_CHANNEL_BONDING_MODE_STAMIN
#define CFG_CHANNEL_BONDING_MODE_MAX           WNI_CFG_CHANNEL_BONDING_MODE_STAMAX
#define CFG_CHANNEL_BONDING_MODE_DEFAULT       WNI_CFG_CHANNEL_BONDING_MODE_STADEF

/*
 * <ini>
 * gScanResultAgeCount - Set scan result age count
 * @Min: 1
 * @Max: 100
 * @Default: 1
 *
 * This ini parameter is the number of times a scan
 * doesn't find it before it is removed from results.
 *
 * Related: None
 *
 * Supported Feature: Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCAN_RESULT_AGE_COUNT_NAME         "gScanResultAgeCount"
#define CFG_SCAN_RESULT_AGE_COUNT_MIN          (1)
#define CFG_SCAN_RESULT_AGE_COUNT_MAX          (100)
#define CFG_SCAN_RESULT_AGE_COUNT_DEFAULT      (1)

/*
 * <ini>
 * gEnableDFSChnlScan - Enable DFS channel scan
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable or disable DFS channel
 * scan
 */
#define CFG_ENABLE_DFS_CHNL_SCAN_NAME              "gEnableDFSChnlScan"
#define CFG_ENABLE_DFS_CHNL_SCAN_MIN               (0)
#define CFG_ENABLE_DFS_CHNL_SCAN_MAX               (1)
#define CFG_ENABLE_DFS_CHNL_SCAN_DEFAULT           (1)

/*
 * <ini>
 * gEnableDFSPnoChnlScan - enable dfs channels in PNO scan
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable/disable dfs channels in PNO scan request,
 * enabling this ini enables driver to include dfs channels in its
 * PNO scan request
 * Related: NA
 *
 * Supported Feature: DFS, PNO
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_ENABLE_DFS_PNO_CHNL_SCAN_NAME              "gEnableDFSPnoChnlScan"
#define CFG_ENABLE_DFS_PNO_CHNL_SCAN_MIN               (0)
#define CFG_ENABLE_DFS_PNO_CHNL_SCAN_MAX               (1)
#define CFG_ENABLE_DFS_PNO_CHNL_SCAN_DEFAULT           (1)

/*
 * <ini>
 * gEnableFirstScan2GOnly - Enable first scan 2G only
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to scan 2G channels only in first scan.
 *
 * Related: None
 *
 * Supported Feature: Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ENABLE_FIRST_SCAN_2G_ONLY_NAME            "gEnableFirstScan2GOnly"
#define CFG_ENABLE_FIRST_SCAN_2G_ONLY_MIN        (0)
#define CFG_ENABLE_FIRST_SCAN_2G_ONLY_MAX        (1)
#define CFG_ENABLE_FIRST_SCAN_2G_ONLY_DEFAULT    (0)

/*
 * <ini>
 * gScanAgingTime - Set scan aging time
 * @Min: 0
 * @Max: 200
 * @Default: 30
 *
 * This ini is used to set scan aging timeout value
 * in secs. For example after 30 secs the bss results
 * greater than 30secs age will be flushed.
 *
 * Related: None
 *
 * Supported Feature: Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCAN_AGING_PARAM_NAME          "gScanAgingTime"
#define CFG_SCAN_AGING_PARAM_MIN           (0)
#define CFG_SCAN_AGING_PARAM_MAX           (200)
#ifdef QCA_WIFI_NAPIER_EMULATION
#define CFG_SCAN_AGING_PARAM_DEFAULT       (90)
#else
#define CFG_SCAN_AGING_PARAM_DEFAULT       (30)
#endif

#ifdef FEATURE_WLAN_SCAN_PNO
/*
 * <ini>
 * gPNOScanSupport - Enable or Disable PNO scan
 * @Min: 1
 * @Max: 0
 * @Default: 1
 *
 * This ini is used to Enable or Disable PNO scan
 *
 * Related: None
 *
 * Supported Feature: Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_PNO_SCAN_SUPPORT                         "gPNOScanSupport"
#define CFG_PNO_SCAN_SUPPORT_ENABLE                  (1)
#define CFG_PNO_SCAN_SUPPORT_DISABLE                 (0)
#define CFG_PNO_SCAN_SUPPORT_DEFAULT                 (1)

/*
 * <ini>
 * gPNOScanTimerRepeatValue - Set PNO scan timer repeat value
 * @Min: 30
 * @Max: 0
 * @Default: 0xffffffff
 *
 * This ini is used by firmware to set fast scan max cycles
 * equal to gPNOScanTimerRepeatValue. Taking power consumption
 * into account firmware after gPNOScanTimerRepeatValue times
 * fast_scan_period switches to slow_scan_period.
 *
 * Related: None
 *
 * Supported Feature: Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_PNO_SCAN_TIMER_REPEAT_VALUE              "gPNOScanTimerRepeatValue"
#define CFG_PNO_SCAN_TIMER_REPEAT_VALUE_DEFAULT      (30)
#define CFG_PNO_SCAN_TIMER_REPEAT_VALUE_MIN          (0)
#define CFG_PNO_SCAN_TIMER_REPEAT_VALUE_MAX          (0xffffffff)

/*
 * <ini>
 * gPNOSlowScanMultiplier - Set PNO slow scan multiplier
 * @Min: 6
 * @Max: 0
 * @Default: 30
 *
 * This ini is used by firmware to set slow scan period
 * as gPNOSlowScanMultiplier times fast_scan_period.
 *
 * Related: None
 *
 * Supported Feature: Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_PNO_SLOW_SCAN_MULTIPLIER                 "gPNOSlowScanMultiplier"
#define CFG_PNO_SLOW_SCAN_MULTIPLIER_DEFAULT         (6)
#define CFG_PNO_SLOW_SCAN_MULTIPLIER_MIN             (0)
#define CFG_PNO_SLOW_SCAN_MULTIPLIER_MAX             (30)
#endif

/*
 * <ini>
 * max_scan_count - Set maximum number of scans
 * @Min: 1
 * @Max: 8
 * @Default: 4
 *
 * This ini is used to set the maximum number of
 * scans that host can queue at firmware.
 * Rome firmware support 8 scan queue size and 4
 * are reserved for internal scan requests like
 * roaming. So host can send 4 scan requests.
 * In iHelium, there is no constraint in number of
 * scan queue size at firmware but the current use
 * cases needs support of maximum of 4 scan request
 * from host.
 *
 * Related: None
 *
 * Supported Feature: Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_MAX_SCAN_COUNT_NAME           "max_scan_count"
#define CFG_MAX_SCAN_COUNT_MIN            (1)
#define CFG_MAX_SCAN_COUNT_MAX            (8)
#define CFG_MAX_SCAN_COUNT_DEFAULT        (4)

/*
 * <ini>
 * gPassiveMaxChannelTime - Set max channel time for passive scan
 * @Min: 0
 * @Max: 10000
 * @Default: 110
 *
 * This ini is used to set maximum channel time in secs spent in
 * passive scan
 *
 * Related: None
 *
 * Supported Feature: Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_PASSIVE_MAX_CHANNEL_TIME_NAME      "gPassiveMaxChannelTime"
#define CFG_PASSIVE_MAX_CHANNEL_TIME_MIN       (0)
#define CFG_PASSIVE_MAX_CHANNEL_TIME_MAX       (10000)
#define CFG_PASSIVE_MAX_CHANNEL_TIME_DEFAULT   (110)

/*
 * <ini>
 * gActiveMaxChannelTime - Set max channel time for active scan
 * @Min: 0
 * @Max: 10000
 * @Default: 40
 *
 * This ini is used to set maximum channel time in secs spent in
 * active scan
 *
 * Related: None
 *
 * Supported Feature: Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ACTIVE_MAX_CHANNEL_TIME_NAME       "gActiveMaxChannelTime"
#define CFG_ACTIVE_MAX_CHANNEL_TIME_MIN        (0)
#define CFG_ACTIVE_MAX_CHANNEL_TIME_MAX        (10000)
#define CFG_ACTIVE_MAX_CHANNEL_TIME_DEFAULT    (40)

/*
 * <ini>
 * gScanNumProbes - Set the number of probes on each channel for active scan
 * @Min: 0
 * @Max: 20
 * @Default: 0
 *
 * This ini is used to set number of probes on each channel for
 * active scan
 *
 * Related: None
 *
 * Supported Feature: Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCAN_NUM_PROBES_NAME       "gScanNumProbes"
#define CFG_SCAN_NUM_PROBES_MIN        (0)
#define CFG_SCAN_NUM_PROBES_MAX        (20)
#define CFG_SCAN_NUM_PROBES_DEFAULT    (0)

/*
 * <ini>
 * gScanProbeRepeatTime - Set the probe repeat time on each channel for active scan
 * @Min: 0
 * @Max: 30
 * @Default: 0
 *
 * This ini is used to set probe repeat time on each channel for
 * active scan
 *
 * Related: None
 *
 * Supported Feature: Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCAN_PROBE_REPEAT_TIME_NAME       "gScanProbeRepeatTime"
#define CFG_SCAN_PROBE_REPEAT_TIME_MIN        (0)
#define CFG_SCAN_PROBE_REPEAT_TIME_MAX        (30)
#define CFG_SCAN_PROBE_REPEAT_TIME_DEFAULT    (0)

/*
 * <ini>
 * gChPredictionFullScanMs - Set periodic timer for channel
 * prediction
 * @Min: 3000
 * @Max: 0x7fffffff
 * @Default: 60000
 *
 * This ini is used to set the periodic timer upon which
 * a full scan needs to be triggered when PNO channel
 * prediction feature is enabled. This parameter is intended
 * to tweak the internal algortihm for experiments.
 *
 * Related: None
 *
 * Supported Feature: Scan
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_CHANNEL_PREDICTION_FULL_SCAN_MS_NAME      "gChPredictionFullScanMs"
#define CFG_CHANNEL_PREDICTION_FULL_SCAN_MS_MIN       (30000)
#define CFG_CHANNEL_PREDICTION_FULL_SCAN_MS_MAX       (0x7fffffff)
#define CFG_CHANNEL_PREDICTION_FULL_SCAN_MS_DEFAULT   (60000)

/*
 * <ini>
 * gbug_report_for_scan_results - Enable bug report
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to create bug report in
 * case of nil scan results.
 *
 * Related: None
 *
 * Supported Feature: Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_CREATE_BUG_REPORT_FOR_SCAN       "gbug_report_for_scan_results"
#define CFG_CREATE_BUG_REPORT_FOR_SCAN_DISABLE    (0)
#define CFG_CREATE_BUG_REPORT_FOR_SCAN_ENABLE     (1)
#define CFG_CREATE_BUG_REPORT_FOR_SCAN_DEFAULT    (0)

/*
 * <ini>
 * hostscan_adaptive_dwell_mode - Enable adaptive dwell mode
 * during host scan with conneciton
 * @Min: 0
 * @Max: 4
 * @Default: 2
 *
 * This ini will set the algo used in dwell time optimization
 * during host scan with connection.
 * See enum scan_dwelltime_adaptive_mode.
 * Acceptable values for this:
 * 0: Default (Use firmware default mode)
 * 1: Conservative optimization
 * 2: Moderate optimization
 * 3: Aggressive optimization
 * 4: Static
 *
 * Related: None
 *
 * Supported Feature: Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ADAPTIVE_SCAN_DWELL_MODE_NAME        "hostscan_adaptive_dwell_mode"
#define CFG_ADAPTIVE_SCAN_DWELL_MODE_MIN         (0)
#define CFG_ADAPTIVE_SCAN_DWELL_MODE_MAX         (4)
#define CFG_ADAPTIVE_SCAN_DWELL_MODE_DEFAULT     (2)

/*
 * <ini>
 * hostscan_adaptive_dwell_mode_no_conn - Enable adaptive dwell mode
 * during host scan without connection
 * @Min: 0
 * @Max: 4
 * @Default: 1
 *
 * This ini will set the algo used in dwell time optimization
 * during host scan without connection.
 * See enum scan_dwelltime_adaptive_mode.
 * Acceptable values for this:
 * 0: Default (Use firmware default mode)
 * 1: Conservative optimization
 * 2: Moderate optimization
 * 3: Aggressive optimization
 * 4: Static
 *
 * Related: None
 *
 * Supported Feature: Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ADAPTIVE_SCAN_DWELL_MODE_NC_NAME    "hostscan_adaptive_dwell_mode_no_conn"
#define CFG_ADAPTIVE_SCAN_DWELL_MODE_NC_MIN     (0)
#define CFG_ADAPTIVE_SCAN_DWELL_MODE_NC_MAX     (4)
#define CFG_ADAPTIVE_SCAN_DWELL_MODE_NC_DEFAULT (1)

/*
 * <ini>
 * extscan_adaptive_dwell_mode - Enable adaptive dwell mode
 * during ext scan
 * @Min: 0
 * @Max: 4
 * @Default: 1
 *
 * This ini will set the algo used in dwell time optimization
 * during ext scan. see enum scan_dwelltime_adaptive_mode.
 * Acceptable values for this:
 * 0: Default (Use firmware default mode)
 * 1: Conservative optimization
 * 2: Moderate optimization
 * 3: Aggressive optimization
 * 4: Static
 *
 * Related: None
 *
 * Supported Feature: Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ADAPTIVE_EXTSCAN_DWELL_MODE_NAME     "extscan_adaptive_dwell_mode"
#define CFG_ADAPTIVE_EXTSCAN_DWELL_MODE_MIN      (0)
#define CFG_ADAPTIVE_EXTSCAN_DWELL_MODE_MAX      (4)
#define CFG_ADAPTIVE_EXTSCAN_DWELL_MODE_DEFAULT  (1)

/*
 * <ini>
 * pnoscan_adaptive_dwell_mode - Enable adaptive dwell mode
 * during pno scan
 * @Min: 0
 * @Max: 4
 * @Default: 1
 *
 * This ini will set the algo used in dwell time optimization
 * during pno scan. see enum scan_dwelltime_adaptive_mode.
 * Acceptable values for this:
 * 0: Default (Use firmware default mode)
 * 1: Conservative optimization
 * 2: Moderate optimization
 * 3: Aggressive optimization
 * 4: Static
 *
 * Related: None
 *
 * Supported Feature: Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ADAPTIVE_PNOSCAN_DWELL_MODE_NAME     "pnoscan_adaptive_dwell_mode"
#define CFG_ADAPTIVE_PNOSCAN_DWELL_MODE_MIN      (0)
#define CFG_ADAPTIVE_PNOSCAN_DWELL_MODE_MAX      (4)
#define CFG_ADAPTIVE_PNOSCAN_DWELL_MODE_DEFAULT  (1)

/*
 * <ini>
 * adaptive_dwell_mode_enabled - Enable adaptive dwell mode
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This parameter will globally disable/enable the adaptive dwell config.
 * Following parameters will set different values of attributes for dwell
 * time optimization thus reducing total scan time.
 * Acceptable values for this:
 * 0: Config is disabled
 * 1: Config is enabled
 *
 * Related: None
 *
 * Supported Feature: Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ADAPTIVE_DWELL_MODE_ENABLED_NAME      "adaptive_dwell_mode_enabled"
#define CFG_ADAPTIVE_DWELL_MODE_ENABLED_MIN       (0)
#define CFG_ADAPTIVE_DWELL_MODE_ENABLED_MAX       (1)
#define CFG_ADAPTIVE_DWELL_MODE_ENABLED_DEFAULT   (1)

/*
 * <ini>
 * global_adapt_dwelltime_mode - Set default adaptive mode
 * @Min: 0
 * @Max: 4
 * @Default: 0
 *
 * This parameter will set default adaptive mode, will be used if any of the
 * scan dwell mode is set to default.
 * For uses : see enum scan_dwelltime_adaptive_mode
 *
 * Related: None
 *
 * Supported Feature: Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_GLOBAL_ADAPTIVE_DWELL_MODE_NAME       "global_adapt_dwelltime_mode"
#define CFG_GLOBAL_ADAPTIVE_DWELL_MODE_MIN        (0)
#define CFG_GLOBAL_ADAPTIVE_DWELL_MODE_MAX        (4)
#define CFG_GLOBAL_ADAPTIVE_DWELL_MODE_DEFAULT    (0)

/*
 * <ini>
 * gRssiCatGap - Set Rssi CatGap
 * @Min: 5
 * @Max: 100
 * @Default: 5
 *
 * This ini is used to set default RssiCatGap
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_RSSI_CATEGORY_GAP_NAME             "gRssiCatGap"
#define CFG_RSSI_CATEGORY_GAP_MIN              (5)
#define CFG_RSSI_CATEGORY_GAP_MAX              (100)
#define CFG_RSSI_CATEGORY_GAP_DEFAULT          (5)

#ifdef FEATURE_LFR_SUBNET_DETECTION
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
#define CFG_ENABLE_LFR_SUBNET_DETECTION    "gLFRSubnetDetectionEnable"
#define CFG_ENABLE_LFR_SUBNET_MIN          (0)
#define CFG_ENABLE_LFR_SUBNET_MAX          (1)
#define CFG_ENABLE_LFR_SUBNET_DEFAULT      (1)
#endif /* FEATURE_LFR_SUBNET_DETECTION */

/*
 * <ini>
 * enable_ftopen - enable/disable FT open feature
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This INI is used to enable/disable FT open feature
*
* Related: None
*
* Supported Feature: Roaming
*
* Usage: External
*
* </ini>
*/
#define CFG_ROAM_FT_OPEN_ENABLE_NAME                "enable_ftopen"
#define CFG_ROAM_FT_OPEN_ENABLE_MIN                 (0)
#define CFG_ROAM_FT_OPEN_ENABLE_MAX                 (1)
#define CFG_ROAM_FT_OPEN_ENABLE_DEFAULT             (1)

/*
 * <ini>
 * min_delay_btw_roam_scans - Min duration (in sec) allowed btw two
 * consecutive roam scans
 * @Min: 0
 * @Max: 60
 * @Default: 10
 *
 * Roam scan is not allowed if duration between two consecutive
 * roam scans is less than this time.
 *
 * Related: None
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_MIN_DELAY_BTW_ROAM_SCAN_NAME    "min_delay_btw_roam_scans"
#define CFG_MIN_DELAY_BTW_ROAM_SCAN_MIN     (0)
#define CFG_MIN_DELAY_BTW_ROAM_SCAN_MAX     (60)
#define CFG_MIN_DELAY_BTW_ROAM_SCAN_DEFAULT (10)

/*
 * <ini>
 * roam_trigger_reason_bitmask - Contains roam_trigger_reasons
 * @Min: 0
 * @Max: 0xFFFFFFFF
 * @Default: 0xDA
 *
 * Bitmask containing roam_trigger_reasons for which
 * min_delay_btw_roam_scans constraint should be applied.
 * Currently supported bit positions are as follows:
 * Bit 0 is reserved in the firmware.
 * WMI_ROAM_TRIGGER_REASON_PER - 1
 * WMI_ROAM_TRIGGER_REASON_BMISS - 2
 * WMI_ROAM_TRIGGER_REASON_LOW_RSSI - 3
 * WMI_ROAM_TRIGGER_REASON_HIGH_RSSI - 4
 * WMI_ROAM_TRIGGER_REASON_PERIODIC - 5
 * WMI_ROAM_TRIGGER_REASON_MAWC - 6
 * WMI_ROAM_TRIGGER_REASON_DENSE - 7
 * WMI_ROAM_TRIGGER_REASON_BACKGROUND - 8
 * WMI_ROAM_TRIGGER_REASON_FORCED - 9
 * WMI_ROAM_TRIGGER_REASON_BTM - 10
 * WMI_ROAM_TRIGGER_REASON_UNIT_TEST - 11
 * WMI_ROAM_TRIGGER_REASON_MAX - 12
 *
 * For Ex: 0xDA (PER, LOW_RSSI, HIGH_RSSI, MAWC, DENSE)
 *
 * Related: None
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ROAM_SCAN_TRIGGER_REASON_BITMASK_NAME "roam_trigger_reason_bitmask"
#define CFG_ROAM_SCAN_TRIGGER_REASON_BITMASK_MIN     (0)
#define CFG_ROAM_SCAN_TRIGGER_REASON_BITMASK_MAX     (0xFFFFFFFF)
#define CFG_ROAM_SCAN_TRIGGER_REASON_BITMASK_DEFAULT (0xDA)

/*
 * <ini>
 * ho_delay_for_rx - Delay Hand-off (In msec) by this duration to receive
 * pending rx frames from current BSS
 * @Min: 0
 * @Max: 200
 * @Default: 0
 *
 * For LFR 3.0 roaming scenario, once roam candidate is found, firmware
 * waits for minimum this much duration to receive pending rx frames from
 * current BSS before switching to new channel for handoff to new AP.
 *
 * Related: None
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ROAM_HO_DELAY_FOR_RX_NAME    "ho_delay_for_rx"
#define CFG_ROAM_HO_DELAY_FOR_RX_MIN     (0)
#define CFG_ROAM_HO_DELAY_FOR_RX_MAX     (200)
#define CFG_ROAM_HO_DELAY_FOR_RX_DEFAULT (0)

/*
 * <ini>
 * roam_force_rssi_trigger - To force RSSI trigger
 * irrespective of channel list type
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to set roam scan mode
 * WMI_ROAM_SCAN_MODE_RSSI_CHANGE, irrespective of whether
 * channel list type is CHANNEL_LIST_STATIC or not
 *
 * Related: None
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ROAM_FORCE_RSSI_TRIGGER_NAME  "roam_force_rssi_trigger"
#define CFG_ROAM_FORCE_RSSI_TRIGGER_MIN     (0)
#define CFG_ROAM_FORCE_RSSI_TRIGGER_MAX     (1)
#define CFG_ROAM_FORCE_RSSI_TRIGGER_DEFAULT (1)

/*
 * <ini>
 * gIbssBssid - Default IBSS BSSID if BSSID is not provided by supplicant
 * @Min: "000000000000"
 * @Max: "ffffffffffff"
 * @Default: "000AF5040506"
 *
 * This ini is used to set Default IBSS BSSID if BSSID
 * is not provided by supplicant and Coalesing is disabled
 *
 * Related: Only applicable if gCoalesingInIBSS is 0
 *
 * Supported Feature: IBSS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_IBSS_BSSID_NAME                    "gIbssBssid"
#define CFG_IBSS_BSSID_MIN                     "000000000000"
#define CFG_IBSS_BSSID_MAX                     "ffffffffffff"
#define CFG_IBSS_BSSID_DEFAULT                 "000AF5040506"

/*
 * <ini>
 * gAdHocChannel5G - Default 5Ghz IBSS channel if channel is not
 * provided by supplicant.
 * @Min: 36
 * @Max: 165
 * @Default: 44
 *
 * This ini is used to set default 5Ghz IBSS channel
 * if channel is not provided by supplicant and band is 5Ghz
 *
 * Related: None
 *
 * Supported Feature: IBSS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_IBSS_ADHOC_CHANNEL_5GHZ_NAME          "gAdHocChannel5G"
#define CFG_IBSS_ADHOC_CHANNEL_5GHZ_MIN           (36)
#define CFG_IBSS_ADHOC_CHANNEL_5GHZ_MAX           (165)
#define CFG_IBSS_ADHOC_CHANNEL_5GHZ_DEFAULT       (44)

/*
 * <ini>
 * gAdHocChannel24G - Default 2.4Ghz IBSS channel if channel is not
 * provided by supplicant.
 * @Min: 1
 * @Max: 14
 * @Default: 6
 *
 * This ini is used to set default 2.4Ghz IBSS channel
 * if channel is not provided by supplicant and band is 2.4Ghz
 *
 * Related: None
 *
 * Supported Feature: IBSS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_IBSS_ADHOC_CHANNEL_24GHZ_NAME         "gAdHocChannel24G"
#define CFG_IBSS_ADHOC_CHANNEL_24GHZ_MIN          (1)
#define CFG_IBSS_ADHOC_CHANNEL_24GHZ_MAX          (14)
#define CFG_IBSS_ADHOC_CHANNEL_24GHZ_DEFAULT      (6)

/*
 * <ini>
 * gCoalesingInIBSS - If IBSS coalesing is enabled.
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to set IBSS coalesing
 *
 * Related: None
 *
 * Supported Feature: IBSS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_COALESING_IN_IBSS_NAME                "gCoalesingInIBSS"
#define CFG_COALESING_IN_IBSS_MIN                 (0)
#define CFG_COALESING_IN_IBSS_MAX                 (1)
#define CFG_COALESING_IN_IBSS_DEFAULT             (0)   /* disabled */

/*
 * <ini>
 * gIbssATIMWinSize - Set IBSS ATIM window size
 * @Min: 0
 * @Max: 50
 * @Default: 0
 *
 * This ini is used to set IBSS ATIM window size
 *
 * Related: None
 *
 * Supported Feature: IBSS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_IBSS_ATIM_WIN_SIZE_NAME                "gIbssATIMWinSize"
#define CFG_IBSS_ATIM_WIN_SIZE_MIN                 (0)
#define CFG_IBSS_ATIM_WIN_SIZE_MAX                 (50)
#define CFG_IBSS_ATIM_WIN_SIZE_DEFAULT             (0)


/*
 * <ini>
 * gIbssIsPowerSaveAllowed - Indicates if IBSS Power Save is
 * supported or not
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to Indicates if IBSS Power Save is
 * supported or not. When not allowed,IBSS station has
 * to stay awake all the time and should never set PM=1
 * in its transmitted frames.
 *
 * Related: valid only when gIbssATIMWinSize is non-zero
 *
 * Supported Feature: IBSS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_IBSS_IS_POWER_SAVE_ALLOWED_NAME        "gIbssIsPowerSaveAllowed"
#define CFG_IBSS_IS_POWER_SAVE_ALLOWED_MIN         (0)
#define CFG_IBSS_IS_POWER_SAVE_ALLOWED_MAX         (1)
#define CFG_IBSS_IS_POWER_SAVE_ALLOWED_DEFAULT     (1)

/*
 * <ini>
 * gIbssIsPowerCollapseAllowed - Indicates if IBSS Power Collapse
 * is allowed
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to indicates if IBSS Power Collapse
 * is allowed
 *
 * Related: None
 *
 * Supported Feature: IBSS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_IBSS_IS_POWER_COLLAPSE_ALLOWED_NAME    "gIbssIsPowerCollapseAllowed"
#define CFG_IBSS_IS_POWER_COLLAPSE_ALLOWED_MIN     (0)
#define CFG_IBSS_IS_POWER_COLLAPSE_ALLOWED_MAX     (1)
#define CFG_IBSS_IS_POWER_COLLAPSE_ALLOWED_DEFAULT (1)

/*
 * <ini>
 * gIbssAwakeOnTxRx - Indicates whether IBSS station
 * can exit power save mode and enter power active
 * state whenever there is a TX/RX activity.
 *
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to ndicates whether IBSS station
 * can exit power save mode and enter power active
 * state whenever there is a TX/RX activity.
 *
 * Related: None
 *
 * Supported Feature: IBSS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_IBSS_AWAKE_ON_TX_RX_NAME               "gIbssAwakeOnTxRx"
#define CFG_IBSS_AWAKE_ON_TX_RX_MIN                (0)
#define CFG_IBSS_AWAKE_ON_TX_RX_MAX                (1)
#define CFG_IBSS_AWAKE_ON_TX_RX_DEFAULT            (0)

/*
 * <ini>
 * gIbssInactivityTime - Indicates the data
 * inactivity time in number of beacon intervals
 * after which IBSS station re-inters power save
 *
 * @Min: 1
 * @Max: 10
 * @Default: 1
 *
 * In IBSS mode if Awake on TX/RX activity is enabled
 * Ibss Inactivity parameter indicates the data
 * inactivity time in number of beacon intervals
 * after which IBSS station re-inters power save
 * by sending Null frame with PM=1
 *
 * Related: Aplicable if gIbssAwakeOnTxRx is enabled
 *
 * Supported Feature: IBSS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_IBSS_INACTIVITY_TIME_NAME              "gIbssInactivityTime"
#define CFG_IBSS_INACTIVITY_TIME_MIN               (1)
#define CFG_IBSS_INACTIVITY_TIME_MAX               (10)
#define CFG_IBSS_INACTIVITY_TIME_DEFAULT           (1)

/*
 * <ini>
 * gIbssTxSpEndInactivityTime - Indicates the time after
 * which TX Service Period is terminated by
 * sending a Qos Null frame with EOSP.
 *
 * @Min: 0
 * @Max: 100
 * @Default: 0
 *
 * In IBSS mode Tx Service Period Inactivity
 * time in msecs indicates the time after
 * which TX Service Period is terminated by
 * sending a Qos Null frame with EOSP.
 * If value is 0, TX SP is terminated with the
 * last buffered packet itself instead of waiting
 * for the inactivity.
 *
 * Related: None
 *
 * Supported Feature: IBSS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_IBSS_TXSP_END_INACTIVITY_NAME          "gIbssTxSpEndInactivityTime"
#define CFG_IBSS_TXSP_END_INACTIVITY_MIN           (0)
#define CFG_IBSS_TXSP_END_INACTIVITY_MAX           (100)
#define CFG_IBSS_TXSP_END_INACTIVITY_DEFAULT       (0)

/*
 * <ini>
 * gIbssPsWarmupTime - PS-supporting device
 * does not enter protocol sleep state during first
 * gIbssPsWarmupTime seconds.
 *
 * @Min: 0
 * @Max: 65535
 * @Default: 0
 *
 * When IBSS network is initialized, PS-supporting device
 * does not enter protocol sleep state during first
 * gIbssPsWarmupTime seconds.
 *
 * Related: valid if gIbssIsPowerSaveAllowed is set
 *
 * Supported Feature: IBSS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_IBSS_PS_WARMUP_TIME_NAME               "gIbssPsWarmupTime"
#define CFG_IBSS_PS_WARMUP_TIME_MIN                (0)
/* Allow unsigned Int Max for now */
#define CFG_IBSS_PS_WARMUP_TIME_MAX                (65535)
#define CFG_IBSS_PS_WARMUP_TIME_DEFAULT            (0)

/*
 * <ini>
 * gIbssPs1RxChainInAtim - IBSS Power Save Enable/Disable 1 RX
 * chain usage during the ATIM window
 *
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * IBSS Power Save Enable/Disable 1 RX
 * chain usage during the ATIM window
 *
 * Related: Depend on gIbssIsPowerSaveAllowed
 *
 * Supported Feature: IBSS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_IBSS_PS_1RX_CHAIN_IN_ATIM_WINDOW_NAME    "gIbssPs1RxChainInAtim"
#define CFG_IBSS_PS_1RX_CHAIN_IN_ATIM_WINDOW_MIN     (0)
#define CFG_IBSS_PS_1RX_CHAIN_IN_ATIM_WINDOW_MAX     (1)
#define CFG_IBSS_PS_1RX_CHAIN_IN_ATIM_WINDOW_DEFAULT (0)

/*
 * <ini>
 * gDot11Mode - SAP phy mode
 * @Min: 0
 * @Max: 12 (11ax)
 * @Default: 12 (11ax)
 *
 * This ini is used to set Phy Mode (auto, b, g, n, etc/) Valid values are
 * 0-12, with 0 = Auto, 12 = 11ax.
 *
 * Related: None.
 *
 * Supported Feature: SAP
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_DOT11_MODE_NAME                    "gDot11Mode"
#define CFG_DOT11_MODE_MIN                     eHDD_DOT11_MODE_AUTO
#define CFG_DOT11_MODE_DEFAULT                 eHDD_DOT11_MODE_11ax
#define CFG_DOT11_MODE_MAX                     eHDD_DOT11_MODE_11ax

/*
 * <ini>
 * gEnableApProt - Enable/Disable AP protection
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable/disable AP protection
 *
 * Related: None.
 *
 * Supported Feature: SAP
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_AP_ENABLE_PROTECTION_MODE_NAME            "gEnableApProt"
#define CFG_AP_ENABLE_PROTECTION_MODE_MIN             (0)
#define CFG_AP_ENABLE_PROTECTION_MODE_MAX             (1)
#define CFG_AP_ENABLE_PROTECTION_MODE_DEFAULT         (1)

#define CFG_DISABLE_PACKET_FILTER		"gDisablePacketFilter"
#define CFG_DISABLE_PACKET_FILTER_MIN		(0)
#define CFG_DISABLE_PACKET_FILTER_MAX		(1)
#define CFG_DISABLE_PACKET_FILTER_DEFAULT	(1)

/*
 * <ini>
 * gPassiveMaxChannelTimeConc - Maximum passive scan time in milliseconds.
 * @Min: 0
 * @Max: 10000
 * @Default: 110
 *
 * This ini is used to set maximum passive scan time in STA+SAP concurrent
 * mode.
 *
 * Related: None.
 *
 * Supported Feature: Concurrency
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_PASSIVE_MAX_CHANNEL_TIME_CONC_NAME      "gPassiveMaxChannelTimeConc"
#define CFG_PASSIVE_MAX_CHANNEL_TIME_CONC_MIN       (0)
#define CFG_PASSIVE_MAX_CHANNEL_TIME_CONC_MAX       (10000)
#define CFG_PASSIVE_MAX_CHANNEL_TIME_CONC_DEFAULT   (110)

/*
 * <ini>
 * gActiveMaxChannelTimeConc - Maximum active scan time in milliseconds.
 * @Min: 0
 * @Max: 10000
 * @Default: 40
 *
 * This ini is used to set maximum active scan time in STA+SAP concurrent
 * mode.
 *
 * Related: None.
 *
 * Supported Feature: Concurrency
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_ACTIVE_MAX_CHANNEL_TIME_CONC_NAME       "gActiveMaxChannelTimeConc"
#define CFG_ACTIVE_MAX_CHANNEL_TIME_CONC_MIN        (0)
#define CFG_ACTIVE_MAX_CHANNEL_TIME_CONC_MAX        (10000)
#define CFG_ACTIVE_MAX_CHANNEL_TIME_CONC_DEFAULT    (40)

/*
 * <ini>
 * gRestTimeConc - Rest time before moving to a new channel to scan.
 * @Min: 0
 * @Max: 10000
 * @Default: 100
 *
 * This ini is used to configure rest time.
 *
 * Related: None.
 *
 * Supported Feature: Concurrency
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_REST_TIME_CONC_NAME                     "gRestTimeConc"
#define CFG_REST_TIME_CONC_MIN                      (0)
#define CFG_REST_TIME_CONC_MAX                      (10000)
#define CFG_REST_TIME_CONC_DEFAULT                  (100)

/*
 * <ini>
 * gMinRestTimeConc - Mininum time spent on home channel before moving to a
 * new channel to scan.
 * @Min: 0
 * @Max: 50
 * @Default: 50
 *
 * This ini is used to configure minimum time spent on home channel before
 * moving to a new channel to scan.
 *
 * Related: None.
 *
 * Supported Feature: Concurrency
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_MIN_REST_TIME_NAME                      "gMinRestTimeConc"
#define CFG_MIN_REST_TIME_MIN                       (0)
#define CFG_MIN_REST_TIME_MAX                       (50)
#define CFG_MIN_REST_TIME_DEFAULT                   (50)

/*
 * <ini>
 * gIdleTimeConc - Data inactivity time in msec.
 * @Min: 0
 * @Max: 25
 * @Default: 25
 *
 * This ini is used to configure data inactivity time in msec on bss channel
 * that will be used by scan engine in firmware.
 * For example if this value is 25ms then firmware will check for data
 * inactivity every 25ms till gRestTimeConc is reached.
 * If inactive then scan engine will move from home channel to scan the next
 * frequency.
 *
 * Related: None.
 *
 * Supported Feature: Concurrency
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_IDLE_TIME_NAME                          "gIdleTimeConc"
#define CFG_IDLE_TIME_MIN                           (0)
#define CFG_IDLE_TIME_MAX                           (25)
#define CFG_IDLE_TIME_DEFAULT                       (25)

/*
 * <ini>
 * gTxPowerCap - WLAN max tx power
 * @Min: 0
 * @Max: 128
 * @Default: 128
 *
 * This ini is used to configure the device max tx power.
 *
 * Related: None.
 *
 * Supported Feature: Concurrency
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_MAX_TX_POWER_NAME                   "gTxPowerCap"
#define CFG_MAX_TX_POWER_MIN                    WNI_CFG_CURRENT_TX_POWER_LEVEL_STAMIN
#define CFG_MAX_TX_POWER_MAX                    WNI_CFG_CURRENT_TX_POWER_LEVEL_STAMAX
/* Not to use CFG default because if no registry setting, this is ignored by SME. */
#define CFG_MAX_TX_POWER_DEFAULT                WNI_CFG_CURRENT_TX_POWER_LEVEL_STAMAX

/*
 * <ini>
 * g_wow_data_inactivity_timeout - Data activity timeout in wow mode.
 * @Min: 1
 * @Max: 255
 * @Default: 50
 *
 * This ini is used to set data inactivity timeout in wow mode.
 *
 * Supported Feature: inactivity timeout in wow mode
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_WOW_DATA_INACTIVITY_TIMEOUT_NAME     "g_wow_data_inactivity_timeout"
#define CFG_WOW_DATA_INACTIVITY_TIMEOUT_MIN      (1)
#define CFG_WOW_DATA_INACTIVITY_TIMEOUT_MAX      (255)
#define CFG_WOW_DATA_INACTIVITY_TIMEOUT_DEFAULT  (50)

/* WMM configuration */
/*
 * <ini>
 * DelayedTriggerFrmInt - UAPSD delay interval
 * @Min: 1
 * @Max: 4294967295
 * @Default: 3000
 *
 * This parameter controls the delay interval(in ms) of UAPSD auto trigger.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_TL_DELAYED_TRGR_FRM_INT_NAME                 "DelayedTriggerFrmInt"
#define CFG_TL_DELAYED_TRGR_FRM_INT_MIN                  1
#define CFG_TL_DELAYED_TRGR_FRM_INT_MAX                  (4294967295UL)
#define CFG_TL_DELAYED_TRGR_FRM_INT_DEFAULT              3000

/*
 * gEnableDFSChnlScan - enable dfs channel scan.
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable/disable dfs channels in scan, enabling this
 * will enable driver to include dfs channels in its scan list.
 * Related: NA
 *
 * Supported Feature: DFS, Scan
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_ENABLE_DFS_CHNL_SCAN_NAME              "gEnableDFSChnlScan"
#define CFG_ENABLE_DFS_CHNL_SCAN_MIN               (0)
#define CFG_ENABLE_DFS_CHNL_SCAN_MAX               (1)
#define CFG_ENABLE_DFS_CHNL_SCAN_DEFAULT           (1)

enum hdd_link_speed_rpt_type {
	eHDD_LINK_SPEED_REPORT_ACTUAL = 0,
	eHDD_LINK_SPEED_REPORT_MAX = 1,
	eHDD_LINK_SPEED_REPORT_MAX_SCALED = 2,
};

/*
 * <ini>
 * gReportMaxLinkSpeed - Reporting of max link speed
 * @Min: 0
 * @Max: 2
 * @Default: 0
 *
 * This ini is used to control how max link speed is reported to OS when
 * driver is handling NL80211_CMD_GET_STATION request.
 * 0: report actual link speed;
 * 1: report max possible link speed;
 * 2: report max possible link speed with RSSI scaling.
 *
 * Related: NA.
 *
 * Supported Feature SAP
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_REPORT_MAX_LINK_SPEED           "gReportMaxLinkSpeed"
#define CFG_REPORT_MAX_LINK_SPEED_MIN       (eHDD_LINK_SPEED_REPORT_ACTUAL)
#define CFG_REPORT_MAX_LINK_SPEED_MAX       (eHDD_LINK_SPEED_REPORT_MAX_SCALED)
#define CFG_REPORT_MAX_LINK_SPEED_DEFAULT   (eHDD_LINK_SPEED_REPORT_ACTUAL)

/*
 * <ini>
 * gLinkSpeedRssiHigh - Report the max possible speed with RSSI scaling
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to set default eHDD_LINK_SPEED_REPORT
 * Used when eHDD_LINK_SPEED_REPORT_SCALED is selected
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_LINK_SPEED_RSSI_HIGH                   "gLinkSpeedRssiHigh"
#define CFG_LINK_SPEED_RSSI_HIGH_MIN               (-127)
#define CFG_LINK_SPEED_RSSI_HIGH_MAX               (0)
#define CFG_LINK_SPEED_RSSI_HIGH_DEFAULT           (-55)

/*
 * <ini>
 * gLinkSpeedRssiMed - Used when eHDD_LINK_SPEED_REPORT_SCALED is selected
 * @Min: -127
 * @Max: 0
 * @Default: -65
 *
 * This ini is used to set medium rssi link speed
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_LINK_SPEED_RSSI_MID                    "gLinkSpeedRssiMed"
#define CFG_LINK_SPEED_RSSI_MID_MIN                (-127)
#define CFG_LINK_SPEED_RSSI_MID_MAX                (0)
#define CFG_LINK_SPEED_RSSI_MID_DEFAULT            (-65)

/*
 * <ini>
 * gLinkSpeedRssiLow - Used when eHDD_LINK_SPEED_REPORT_SCALED is selected
 * @Min: -127
 * @Max: 0
 * @Default: -80
 *
 * This ini is used to set low rssi link speed
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_LINK_SPEED_RSSI_LOW                    "gLinkSpeedRssiLow"
#define CFG_LINK_SPEED_RSSI_LOW_MIN                (-127)
#define CFG_LINK_SPEED_RSSI_LOW_MAX                (0)
#define CFG_LINK_SPEED_RSSI_LOW_DEFAULT            (-80)

#ifdef ENABLE_MTRACE_LOG
/*
 * Enable MTRACE for all modules
 */
#define CFG_ENABLE_MTRACE            "enable_mtrace"
#define CFG_ENABLE_MTRACE_MIN        (0)
#define CFG_ENABLE_MTRACE_MAX        (1)
#define CFG_ENABLE_MTRACE_DEFAULT    (0)
#endif

/*
 * <ini>
 * gEnableSNRMonitoring - Enables SNR Monitoring
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to set default snr monitor
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_ENABLE_SNR_MONITORING_NAME              "gEnableSNRMonitoring"
#define CFG_ENABLE_SNR_MONITORING_MIN               (0)
#define CFG_ENABLE_SNR_MONITORING_MAX               (1)
#define CFG_ENABLE_SNR_MONITORING_DEFAULT           (0)

#ifdef FEATURE_WLAN_RA_FILTERING
#define CFG_RA_RATE_LIMIT_INTERVAL_NAME            "gRArateLimitInterval"
#define CFG_RA_RATE_LIMIT_INTERVAL_MIN             (60)
#define CFG_RA_RATE_LIMIT_INTERVAL_MAX             (3600)
#define CFG_RA_RATE_LIMIT_INTERVAL_DEFAULT         (60) /*60 SEC */
#endif

/*
 * <ini>
 * gInitialDwellTime - Used to set initial dwell time
 * @Min: 0
 * @Max: 0
 * @Default: 100
 *
 * This ini is used to set default initial dwell time
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_INITIAL_DWELL_TIME_NAME            "gInitialDwellTime"
#define CFG_INITIAL_DWELL_TIME_DEFAULT         (0)
#define CFG_INITIAL_DWELL_TIME_MIN             (0)
#define CFG_INITIAL_DWELL_TIME_MAX             (100)

/*
 * <ini>
 * gInitialScanNoDFSChnl - WLAN skips scanning the DFS channels
 * @Min: 0
 * @Max: 0
 * @Default: 1
 *
 * This ini is used to set for the first scan after driver
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_INITIAL_SCAN_NO_DFS_CHNL_NAME         "gInitialScanNoDFSChnl"
#define CFG_INITIAL_SCAN_NO_DFS_CHNL_DEFAULT      (0)
#define CFG_INITIAL_SCAN_NO_DFS_CHNL_MIN          (0)
#define CFG_INITIAL_SCAN_NO_DFS_CHNL_MAX          (1)

#ifdef MSM_PLATFORM
/*
 * <ini>
 * periodic_stats_display_time - time(seconds) after which stats will be printed
 * @Min: 0
 * @Max: 256
 * @Default: 10
 *
 * This values specifies the recurring time period after which stats will be
 * printed in wlan driver logs.
 *
 * Usage: Internal / External
 *
 * </ini>
 */
#define CFG_PERIODIC_STATS_DISPLAY_TIME_NAME       "periodic_stats_display_time"
#define CFG_PERIODIC_STATS_DISPLAY_TIME_DEFAULT    (10)
#define CFG_PERIODIC_STATS_DISPLAY_TIME_MIN        (0)
#define CFG_PERIODIC_STATS_DISPLAY_TIME_MAX        (256)

#endif /* MSM_PLATFORM */

#ifdef DHCP_SERVER_OFFLOAD
#define CFG_DHCP_SERVER_IP_NAME     "gDHCPServerIP"
#define CFG_DHCP_SERVER_IP_DEFAULT  ""
#endif /* DHCP_SERVER_OFFLOAD */

#define CFG_ENABLE_MAC_ADDR_SPOOFING                "gEnableMacAddrSpoof"
#define CFG_ENABLE_MAC_ADDR_SPOOFING_MIN            (0)
#define CFG_ENABLE_MAC_ADDR_SPOOFING_MAX            (1)
#define CFG_ENABLE_MAC_ADDR_SPOOFING_DEFAULT        (1)

/*
 * <ini>
 * gSAP11ACOverride - Override bw to 11ac for SAP in driver even if supplicant
 *                    or hostapd configures HT.
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable/disable 11AC override for SAP.
 * Android UI does not provide advanced configuration options
 * for SoftAP for Android O and below.
 * Default override disabled for android. Can be enabled from
 * ini for Android O and below.
 *
 *
 * Supported Feature: SAP
 *
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_SAP_11AC_OVERRIDE_NAME             "gSAP11ACOverride"
#define CFG_SAP_11AC_OVERRIDE_MIN              (0)
#define CFG_SAP_11AC_OVERRIDE_MAX              (1)
#define CFG_SAP_11AC_OVERRIDE_DEFAULT          (0)

/*
 * <ini>
 * gGO11ACOverride - Override bw to 11ac for P2P GO
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable/disable 11AC override for GO.
 * P2P GO also follows start_bss and since P2P GO could not be
 * configured to setup VHT channel width in wpa_supplicant, driver
 * can override 11AC.
 *
 *
 * Supported Feature: P2P
 *
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_GO_11AC_OVERRIDE_NAME             "gGO11ACOverride"
#define CFG_GO_11AC_OVERRIDE_MIN              (0)
#define CFG_GO_11AC_OVERRIDE_MAX              (1)
#define CFG_GO_11AC_OVERRIDE_DEFAULT          (1)

/*
 * set the self gen power value from
 * 0 to 0xffff
 */
#define CFG_SELF_GEN_FRM_PWR        "gSelfGenFrmPwr"
#define CFG_SELF_GEN_FRM_PWR_MIN      (0)
#define CFG_SELF_GEN_FRM_PWR_MAX      (0xffff)
#define CFG_SELF_GEN_FRM_PWR_DEFAULT  (0)

/*
 * <ini>
 * etsi13_srd_chan_in_master_mode - Enable/disable ETSI SRD channels in
 * master mode PCL and ACS functionality
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * etsi13_srd_chan_in_master_mode is to enable/disable ETSI SRD channels in
 * master mode PCL and ACS functionality
 *
 * Related: None
 *
 * Supported Feature: SAP/P2P-GO
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_ETSI13_SRD_CHAN_IN_MASTER_MODE    "etsi13_srd_chan_in_master_mode"
#define CFG_ETSI13_SRD_CHAN_IN_MASTER_MODE_DEF (0)
#define CFG_ETSI13_SRD_CHAN_IN_MASTER_MODE_MIN (0)
#define CFG_ETSI13_SRD_CHAN_IN_MASTER_MODE_MAX (1)

/*
 * gPNOChannelPrediction will allow user to enable/disable the
 * PNO channel prediction feature.
 * In current PNO implementation, scan is always done until all configured
 * channels are scanned. If we can determine DUT is stationary based on
 * scanning a subset of channels, we may cancel the remaining channels.
 * Hence, we can save additional power consumption.
 */
#define CFG_PNO_CHANNEL_PREDICTION_NAME      "gPNOChannelPrediction"
#define CFG_PNO_CHANNEL_PREDICTION_MIN       (0)
#define CFG_PNO_CHANNEL_PREDICTION_MAX       (1)
#define CFG_PNO_CHANNEL_PREDICTION_DEFAULT   (0)
/*
 * The top K number of channels are used for tanimoto distance
 * calculation. These are the top channels on which the probability
 * of finding the AP's is extremely high. This number is intended
 * for tweaking the internal algorithm for experiments. This should
 * not be changed externally.
 */
#define CFG_TOP_K_NUM_OF_CHANNELS_NAME      "gTopKNumOfChannels"
#define CFG_TOP_K_NUM_OF_CHANNELS_MIN       (1)
#define CFG_TOP_K_NUM_OF_CHANNELS_MAX       (5)
#define CFG_TOP_K_NUM_OF_CHANNELS_DEFAULT   (3)
/*
 * This is the threshold value to determine that the STA is
 * stationary. If the tanimoto distance is less than this
 * value, then the device is considered to be stationary.
 * This parameter is intended to tweak the internal algorithm
 * for experiments. This should not be changed externally.
 */
#define CFG_STATIONARY_THRESHOLD_NAME      "gStationaryThreshold"
#define CFG_STATIONARY_THRESHOLD_MIN       (0)
#define CFG_STATIONARY_THRESHOLD_MAX       (100)
#define CFG_STATIONARY_THRESHOLD_DEFAULT   (10)

#ifdef WLAN_FEATURE_TSF_PLUS
/* PTP options */
#define CFG_SET_TSF_PTP_OPT_NAME                  "gtsf_ptp_options"
#define CFG_SET_TSF_PTP_OPT_MIN                   (0)
#define CFG_SET_TSF_PTP_OPT_MAX                   (0xff)
#define CFG_SET_TSF_PTP_OPT_RX                    (0x1)
#define CFG_SET_TSF_PTP_OPT_TX                    (0x2)
#define CFG_SET_TSF_PTP_OPT_RAW                   (0x4)
#define CFG_SET_TSF_DBG_FS                        (0x8)
#define CFG_SET_TSF_PTP_OPT_DEFAULT               (0xf)
#endif

#ifdef CONFIG_DP_TRACE
/* Max length of gDptraceConfig string. e.g.- "1, 6, 1, 62" */
#define DP_TRACE_CONFIG_STRING_LENGTH		(20)

/* At max 4 DP Trace config parameters are allowed. Refer - gDptraceConfig */
#define DP_TRACE_CONFIG_NUM_PARAMS		(4)

/*
 * Default value of live mode in case it cannot be determined from cfg string
 * gDptraceConfig
 */
#define DP_TRACE_CONFIG_DEFAULT_LIVE_MODE	(1)

/*
 * Default value of thresh (packets/second) beyond which DP Trace is disabled.
 * Use this default in case the value cannot be determined from cfg string
 * gDptraceConfig
 */
#define DP_TRACE_CONFIG_DEFAULT_THRESH		(6)

/*
 * Number of intervals of BW timer to wait before enabling/disabling DP Trace.
 * Since throughput threshold to disable live logging for DP Trace is very low,
 * we calculate throughput based on # packets received in a second.
 * For example assuming bandwidth timer interval is 100ms, and if more than 6
 * prints are received in 10 * 100 ms interval, we want to disable DP Trace
 * live logging. DP_TRACE_CONFIG_DEFAULT_THRESH_TIME_LIMIT is the default
 * value, to be used in case the real value cannot be derived from
 * bw timer interval
 */
#define DP_TRACE_CONFIG_DEFAULT_THRESH_TIME_LIMIT (10)

/* Default proto bitmap in case its missing in gDptraceConfig string */
#define DP_TRACE_CONFIG_DEFAULT_BITMAP \
			(QDF_NBUF_PKT_TRAC_TYPE_EAPOL |\
			QDF_NBUF_PKT_TRAC_TYPE_DHCP |\
			QDF_NBUF_PKT_TRAC_TYPE_MGMT_ACTION |\
			QDF_NBUF_PKT_TRAC_TYPE_ARP |\
			QDF_NBUF_PKT_TRAC_TYPE_ICMP |\
			QDF_NBUF_PKT_TRAC_TYPE_ICMPv6)\

/* Default verbosity, in case its missing in gDptraceConfig string*/
#define DP_TRACE_CONFIG_DEFAULT_VERBOSTY QDF_DP_TRACE_VERBOSITY_LOW

#endif

/*
 * This parameter will set the weight to calculate the average low pass
 * filter for channel congestion.
 * Acceptable values for this: 0-100 (In %)
 */
#define CFG_ADAPT_DWELL_LPF_WEIGHT_NAME       "adapt_dwell_lpf_weight"
#define CFG_ADAPT_DWELL_LPF_WEIGHT_MIN        (0)
#define CFG_ADAPT_DWELL_LPF_WEIGHT_MAX        (100)
#define CFG_ADAPT_DWELL_LPF_WEIGHT_DEFAULT    (80)

/*
 * This parameter will set interval to monitor wifi activity
 * in passive scan in msec.
 * Acceptable values for this: 0-25
 */
#define CFG_ADAPT_DWELL_PASMON_INTVAL_NAME     "adapt_dwell_passive_mon_intval"
#define CFG_ADAPT_DWELL_PASMON_INTVAL_MIN      (0)
#define CFG_ADAPT_DWELL_PASMON_INTVAL_MAX      (25)
#define CFG_ADAPT_DWELL_PASMON_INTVAL_DEFAULT  (10)

/*
 * This parameter will set % of wifi activity used in passive scan 0-100.
 * Acceptable values for this: 0-100 (in %)
 */
#define CFG_ADAPT_DWELL_WIFI_THRESH_NAME       "adapt_dwell_wifi_act_threshold"
#define CFG_ADAPT_DWELL_WIFI_THRESH_MIN        (0)
#define CFG_ADAPT_DWELL_WIFI_THRESH_MAX        (100)
#define CFG_ADAPT_DWELL_WIFI_THRESH_DEFAULT    (10)

/*
 * <ini>
 * restart_beaconing_on_chan_avoid_event - control the beaconing entity to move
 * away from active LTE channels
 * @Min: 0
 * @Max: 2
 * @Default: 1
 *
 * This ini is used to control the beaconing entity (SAP/GO) to move away from
 * active LTE channels when channel avoidance event is received
 * restart_beaconing_on_chan_avoid_event=0: Don't allow beaconing entity move
 * from active LTE channels
 * restart_beaconing_on_chan_avoid_event=1: Allow beaconing entity move from
 * active LTE channels
 * restart_beaconing_on_chan_avoid_event=2: Allow beaconing entity move from
 * 2.4G active LTE channels only
 *
 * Related: None
 *
 * Supported Feature: channel avoidance
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_RESTART_BEACONING_ON_CH_AVOID_NAME    "restart_beaconing_on_chan_avoid_event"
#define CFG_RESTART_BEACONING_ON_CH_AVOID_MIN     (CH_AVOID_RULE_DO_NOT_RESTART)
#define CFG_RESTART_BEACONING_ON_CH_AVOID_MAX     (CH_AVOID_RULE_RESTART_24G_ONLY)
#define CFG_RESTART_BEACONING_ON_CH_AVOID_DEFAULT (CH_AVOID_RULE_RESTART)

/*
 * Support to start sap in indoor channel
 * Customer can config this item to enable/disable sap in indoor channel
 * Default: Disable
 */
#define CFG_INDOOR_CHANNEL_SUPPORT_NAME     "gindoor_channel_support"
#define CFG_INDOOR_CHANNEL_SUPPORT_MIN      (0)
#define CFG_INDOOR_CHANNEL_SUPPORT_MAX      (1)
#define CFG_INDOOR_CHANNEL_SUPPORT_DEFAULT  (0)

/*
 * <ini>
 * gActiveUcBpfMode - Control UC active APF mode
 * @Min: 0 (disabled)
 * @Max: 2 (adaptive)
 * @Default: 0 (disabled)
 *
 * This config item controls UC APF in active mode. There are 3 modes:
 *	0) disabled - APF is disabled in active mode
 *	1) enabled - APF is enabled for all packets in active mode
 *	2) adaptive - APF is enabled for packets up to some throughput threshold
 *
 * Related: gActiveMcBcBpfMode
 *
 * Supported Feature: Active Mode APF
 *
 * Usage: Internal/External
 * </ini>
 */
#define CFG_ACTIVE_UC_APF_MODE_NAME    "gActiveUcBpfMode"
#define CFG_ACTIVE_UC_APF_MODE_MIN     (ACTIVE_APF_DISABLED)
#define CFG_ACTIVE_UC_APF_MODE_MAX     (ACTIVE_APF_MODE_COUNT - 1)
#define CFG_ACTIVE_UC_APF_MODE_DEFAULT (ACTIVE_APF_DISABLED)

/*
 * <ini>
 * gActiveMcBcBpfMode - Control MC/BC active APF mode
 * @Min: 0 (disabled)
 * @Max: 1 (enabled)
 * @Default: 0 (disabled)
 *
 * This config item controls MC/BC APF in active mode. There are 3 modes:
 *	0) disabled - APF is disabled in active mode
 *	1) enabled - APF is enabled for all packets in active mode
 *	2) adaptive - APF is enabled for packets up to some throughput threshold
 *
 * Related: gActiveUcBpfMode
 *
 * Supported Feature: Active Mode APF
 *
 * Usage: Internal/External
 * </ini>
 */
#define CFG_ACTIVE_MC_BC_APF_MODE_NAME    "gActiveMcBcBpfMode"
#define CFG_ACTIVE_MC_BC_APF_MODE_MIN     (ACTIVE_APF_DISABLED)
#define CFG_ACTIVE_MC_BC_APF_MODE_MAX     (ACTIVE_APF_ENABLED)
#define CFG_ACTIVE_MC_BC_APF_MODE_DEFAULT (ACTIVE_APF_DISABLED)

#ifdef WLAN_FEATURE_11AX
/* 11AX related INI configuration */
/*
 * <ini>
 * he_dynamic_frag_support - configure dynamic fragmentation
 * @Min: 0
 * @Max: 3
 * @Default: 1
 *
 * This ini is used to configure dynamic fragmentation.
 *
 * Related: NA
 *
 * Supported Feature: 11AX
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_HE_DYNAMIC_FRAGMENTATION_NAME    "he_dynamic_frag_support"
#define CFG_HE_DYNAMIC_FRAGMENTATION_MIN     (0)
#define CFG_HE_DYNAMIC_FRAGMENTATION_MAX     (3)
#define CFG_HE_DYNAMIC_FRAGMENTATION_DEFAULT (0)

/*
 * <ini>
 * enable_ul_mimo- Enable UL MIMO.
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable or disable UL MIMO.
 *
 * Related: NA
 *
 * Supported Feature: 11AX
 *
 * Usage: External
 *
 * </ini>
 */

#define CFG_ENABLE_UL_MIMO_NAME    "enable_ul_mimo"
#define CFG_ENABLE_UL_MIMO_MIN     (0)
#define CFG_ENABLE_UL_MIMO_MAX     (1)
#define CFG_ENABLE_UL_MIMO_DEFAULT (0)

/*
 * <ini>
 * enable_ul_ofdma- Enable UL OFDMA.
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable or disable UL OFDMA.
 *
 * Related: NA
 *
 * Supported Feature: 11AX
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ENABLE_UL_OFDMA_NAME    "enable_ul_ofdma"
#define CFG_ENABLE_UL_OFDMA_MIN     (0)
#define CFG_ENABLE_UL_OFDMA_MAX     (1)
#define CFG_ENABLE_UL_OFDMA_DEFAULT (0)

/*
 * <ini>
 * he_sta_obsspd- 11AX HE OBSS PD bit field
 * @Min: 0
 * @Max: uin32_t max
 * @Default: 0x15b8c2ae
 *
 * 4 Byte value with each byte representing a signed value for following params:
 * Param                   Bit position    Default
 * OBSS_PD min (primary)   7:0             -82 (0xae)
 * OBSS_PD max (primary)   15:8            -62 (0xc2)
 * Secondary channel Ed    23:16           -72 (0xb8)
 * TX_PWR(ref)             31:24           21  (0x15)
 * This bit field value is directly applied to FW
 *
 * Related: NA
 *
 * Supported Feature: 11AX
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_HE_STA_OBSSPD_NAME    "he_sta_obsspd"
#define CFG_HE_STA_OBSSPD_MIN     (0)
#define CFG_HE_STA_OBSSPD_MAX     (0xffffffff)
#define CFG_HE_STA_OBSSPD_DEFAULT (0x15b8c2ae)

#endif /* WLAN_FEATURE_11AX */
#ifdef WLAN_SUPPORT_TWT
/*
 * <ini>
 * enable_twt - Enable Target Wake Time support.
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable or disable TWT support.
 *
 * Related: NA
 *
 * Supported Feature: 11AX
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ENABLE_TWT_NAME    "enable_twt"
#define CFG_ENABLE_TWT_MIN     (0)
#define CFG_ENABLE_TWT_MAX     (1)
#define CFG_ENABLE_TWT_DEFAULT (1)

/*
 * <ini>
 * twt_congestion_timeout - Target wake time congestion timeout.
 * @Min: 0
 * @Max: 10000
 * @Default: 100
 *
 * STA uses this timer to continuously monitor channel congestion levels to
 * decide whether to start or stop TWT. This ini is used to configure the
 * target wake time congestion timeout value in the units of milliseconds.
 * A value of Zero indicates that this is a host triggered TWT and all the
 * necessary configuration for TWT will be directed from the host.
 *
 * Related: NA
 *
 * Supported Feature: 11AX
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_TWT_CONGESTION_TIMEOUT_NAME    "twt_congestion_timeout"
#define CFG_TWT_CONGESTION_TIMEOUT_MIN     (0)
#define CFG_TWT_CONGESTION_TIMEOUT_MAX     (10000)
#define CFG_TWT_CONGESTION_TIMEOUT_DEFAULT (100)

#endif /* WLAN_SUPPORT_TWT */

/*
 * <ini>
 * gScanBackoffMultiplier - For NLO/PNO, multiply fast scan period by this every
 *	max cycles
 * @Min: 0
 * @Max: 255
 * @Default: 0
 *
 * For Network Listen Offload and Perfered Network Offload, multiply the fast
 * scan period by this value after max cycles have occurred. Setting this to 0
 * disables the feature.
 *
 * @E.g.
 *	# Disable scan backoff multiplier
 *	gScanBackoffMultiplier=0
 *	# Effectively the same
 *	gScanBackoffMultiplier=1
 *	# Double the scan period after each max cycles have occurred
 *	gScanBackoffMultiplier=2
 *
 * Related: NLO, PNO
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_SCAN_BACKOFF_MULTIPLIER_NAME	"gScanBackoffMultiplier"
#define CFG_SCAN_BACKOFF_MULTIPLIER_MIN		(0)
#define CFG_SCAN_BACKOFF_MULTIPLIER_MAX		(255)
#define CFG_SCAN_BACKOFF_MULTIPLIER_DEFAULT	(0)

/*
 * <ini>
 * mawc_nlo_enabled - For NLO/PNO, enable MAWC based scan
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * Enable/Disable the Motion Aided Wireless Connectivity
 * based NLO using this parameter
 *
 * Related: NLO, PNO
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_MAWC_NLO_ENABLED_NAME	"mawc_nlo_enabled"
#define CFG_MAWC_NLO_ENABLED_MIN	(0)
#define CFG_MAWC_NLO_ENABLED_MAX	(1)
#define CFG_MAWC_NLO_ENABLED_DEFAULT	(1)

/*
 * <ini>
 * mawc_nlo_exp_backoff_ratio - Exponential back off ratio
 * @Min: 0
 * @Max: 300
 * @Default: 3
 *
 * Configure the exponential back off ratio using this
 * parameter for MAWC based NLO
 * ratio of exponential backoff, next = current + current*ratio/100
 *
 * Related: NLO, PNO
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_MAWC_NLO_EXP_BACKOFF_RATIO_NAME     "mawc_nlo_exp_backoff_ratio"
#define CFG_MAWC_NLO_EXP_BACKOFF_RATIO_MIN      (0)
#define CFG_MAWC_NLO_EXP_BACKOFF_RATIO_MAX      (300)
#define CFG_MAWC_NLO_EXP_BACKOFF_RATIO_DEFAULT  (3)

/*
 * <ini>
 * mawc_nlo_init_scan_interval - Initial Scan Interval
 * @Min: 1000
 * @Max: 0xFFFFFFFF
 * @Default: 10000
 *
 * Configure the initial scan interval  using this
 * parameter for MAWC based NLO (Units in Milliseconds)
 *
 * Related: NLO, PNO
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_MAWC_NLO_INIT_SCAN_INTERVAL_NAME     "mawc_nlo_init_scan_interval"
#define CFG_MAWC_NLO_INIT_SCAN_INTERVAL_MIN      (1000)
#define CFG_MAWC_NLO_INIT_SCAN_INTERVAL_MAX      (0xFFFFFFFF)
#define CFG_MAWC_NLO_INIT_SCAN_INTERVAL_DEFAULT  (10000)

/*
 * <ini>
 * mawc_nlo_max_scan_interval - Maximum Scan Interval
 * @Min: 1000
 * @Max: 0xFFFFFFFF
 * @Default: 60000
 *
 * Configure the maximum scan interval  using this
 * parameter for MAWC based NLO (Units in Milliseconds)
 *
 * Related: NLO, PNO
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_MAWC_NLO_MAX_SCAN_INTERVAL_NAME     "mawc_nlo_max_scan_interval"
#define CFG_MAWC_NLO_MAX_SCAN_INTERVAL_MIN      (1000)
#define CFG_MAWC_NLO_MAX_SCAN_INTERVAL_MAX      (0xFFFFFFFF)
#define CFG_MAWC_NLO_MAX_SCAN_INTERVAL_DEFAULT  (60000)

/*
 * <ini>
 * gEnableLPRx - Enable/Disable LPRx
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini Enables or disables the LPRx in FW
 *
 * Usage: External
 *
 * </ini>
 */

#define CFG_LPRx_NAME       "gEnableLPRx"
#define CFG_LPRx_MIN         (0)
#define CFG_LPRx_MAX         (1)
#define CFG_LPRx_DEFAULT     (1)

/*
 * <ini>
 * scan_11d_interval - 11d scan interval in ms
 * @Min: 1 sec
 * @Max: 10 hr
 * @Default: 1 hr
 *
 * This ini sets the 11d scan interval in FW
 *
 *
 *
 * Supported Feature: STA
 *
 * Usage: External
 *
 * </ini>
 */

#define CFG_SCAN_11D_INTERVAL_NAME      "scan_11d_interval"
#define CFG_SCAN_11D_INTERVAL_DEFAULT   (3600000)
#define CFG_SCAN_11D_INTERVAL_MIN       (1000)
#define CFG_SCAN_11D_INTERVAL_MAX       (36000000)
/*
 * <ini>
 * is_bssid_hint_priority - Set priority for connection with bssid_hint
 * BSSID.
 * @Min: 0
 * @Max: 1
 * @Default: 1
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

#define CFG_IS_BSSID_HINT_PRIORITY_NAME    "is_bssid_hint_priority"
#define CFG_IS_BSSID_HINT_PRIORITY_DEFAULT (0)
#define CFG_IS_BSSID_HINT_PRIORITY_MIN     (0)
#define CFG_IS_BSSID_HINT_PRIORITY_MAX     (1)

/*
 * <ini>
 * sae_enabled - Enable/Disable SAE support in driver
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable/disable SAE support in driver
 * Driver will update config to supplicant based on this config.
 *
 * Related: None
 *
 * Supported Feature: SAE
 * Usage: External
 *
 * </ini>
 */

#define CFG_IS_SAE_ENABLED_NAME    "sae_enabled"
#define CFG_IS_SAE_ENABLED_DEFAULT (1)
#define CFG_IS_SAE_ENABLED_MIN     (0)
#define CFG_IS_SAE_ENABLED_MAX     (1)

/*
 * <ini>
 * gcmp_enabled - ini to enable/disable GCMP
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * Currently Firmware update the sequence number for each TID with 2^3
 * because of security issues. But with this PN mechanism, throughput drop
 * is observed. With this ini FW takes the decision to trade off between
 * security and throughput
 *
 * Supported Feature: STA/SAP/P2P
 *
 * Usage: External
 *
 * </ini>
 */

#define CFG_ENABLE_GCMP_NAME    "gcmp_enabled"
#define CFG_ENABLE_GCMP_MIN     (0)
#define CFG_ENABLE_GCMP_MAX     (1)
#define CFG_ENABLE_GCMP_DEFAULT (1)

/*
 * <ini>
 * 11k_offload_enable_bitmask - Bitmask to enable 11k offload to FW
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to set which of the 11k features is offloaded to FW
 * Currently Neighbor Report Request is supported for offload and is enabled
 * by default
 * B0: Offload 11k neighbor report requests
 * B1-B31: Reserved
 *
 * Related : None
 *
 * Usage: External
 *
 * </ini>
 */

#define CFG_OFFLOAD_11K_ENABLE_BITMASK_NAME    "11k_offload_enable_bitmask"
#define CFG_OFFLOAD_11K_ENABLE_BITMASK_MIN     (0)
#define CFG_OFFLOAD_11K_ENABLE_BITMASK_MAX     (1)
#define CFG_OFFLOAD_11K_ENABLE_BITMASK_DEFAULT (1)

#define OFFLOAD_11K_BITMASK_NEIGHBOR_REPORT_REQUEST  0x1
/*
 * <ini>
 * nr_offload_params_bitmask - bitmask to specify which of the
 * neighbor report offload params are valid in the ini
 * frame
 * @Min: 0
 * @Max: 63
 * @Default: 63
 *
 * This ini specifies which of the neighbor report offload params are valid
 * and should be considered by the FW. The bitmask is as follows
 * B0: nr_offload_time_offset
 * B1: nr_offload_low_rssi_offset
 * B2: nr_offload_bmiss_count_trigger
 * B3: nr_offload_per_threshold_offset
 * B4: nr_offload_cache_timeout
 * B5: nr_offload_max_req_cap
 * B6-B7: Reserved
 *
 * Related : 11k_offload_enable_bitmask
 *
 * Usage: External
 *
 * </ini>
 */

#define CFG_OFFLOAD_NEIGHBOR_REPORT_PARAMS_BITMASK_NAME \
	"nr_offload_params_bitmask"
#define CFG_OFFLOAD_NEIGHBOR_REPORT_PARAMS_BITMASK_MIN      (0)
#define CFG_OFFLOAD_NEIGHBOR_REPORT_PARAMS_BITMASK_MAX      (63)
#define CFG_OFFLOAD_NEIGHBOR_REPORT_PARAMS_BITMASK_DEFAULT  (63)

/*
 * <ini>
 * nr_offload_time_offset - time interval in seconds after the
 * neighbor report offload command to send the first neighbor report request
 * frame
 * @Min: 0
 * @Max: 3600
 * @Default: 30
 *
 * Related : nr_offload_params_bitmask
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_OFFLOAD_NEIGHBOR_REPORT_TIME_OFFSET_NAME \
	"nr_offload_time_offset"
#define CFG_OFFLOAD_NEIGHBOR_REPORT_TIME_OFFSET_MIN      (0)
#define CFG_OFFLOAD_NEIGHBOR_REPORT_TIME_OFFSET_MAX      (3600)
#define CFG_OFFLOAD_NEIGHBOR_REPORT_TIME_OFFSET_DEFAULT  (30)

/*
 * <ini>
 * nr_offload_low_rssi_offset - offset from the roam RSSI threshold
 * to trigger the neighbor report request frame (in dBm)
 * @Min: 4
 * @Max: 10
 * @Default: 4
 *
 * Related : nr_offload_params_bitmask
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_OFFLOAD_NEIGHBOR_REPORT_LOW_RSSI_OFFSET_NAME \
	"nr_offload_low_rssi_offset"
#define CFG_OFFLOAD_NEIGHBOR_REPORT_LOW_RSSI_OFFSET_MIN     (4)
#define CFG_OFFLOAD_NEIGHBOR_REPORT_LOW_RSSI_OFFSET_MAX     (10)
#define CFG_OFFLOAD_NEIGHBOR_REPORT_LOW_RSSI_OFFSET_DEFAULT (4)

/*
 * <ini>
 * nr_offload_bmiss_count_trigger - Number of beacon miss events to
 * trigger a neighbor report request frame
 * @Min: 1
 * @Max: 5
 * @Default: 1
 *
 * Related : nr_offload_params_bitmask
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_OFFLOAD_NEIGHBOR_REPORT_BMISS_COUNT_TRIGGER_NAME \
	"nr_offload_bmiss_count_trigger"
#define CFG_OFFLOAD_NEIGHBOR_REPORT_BMISS_COUNT_TRIGGER_MIN     (1)
#define CFG_OFFLOAD_NEIGHBOR_REPORT_BMISS_COUNT_TRIGGER_MAX     (5)
#define CFG_OFFLOAD_NEIGHBOR_REPORT_BMISS_COUNT_TRIGGER_DEFAULT (1)

/*
 * <ini>
 * nr_offload_per_threshold_offset - offset from PER threshold to
 * trigger a neighbor report request frame (in %)
 * @Min: 5
 * @Max: 20
 * @Default: 5
 *
 * This ini is used to set the neighbor report offload parameter:
 *
 * Related : nr_offload_params_bitmask
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_OFFLOAD_NEIGHBOR_REPORT_PER_THRESHOLD_OFFSET_NAME \
	"nr_offload_per_threshold_offset"
#define CFG_OFFLOAD_NEIGHBOR_REPORT_PER_THRESHOLD_OFFSET_MIN     (5)
#define CFG_OFFLOAD_NEIGHBOR_REPORT_PER_THRESHOLD_OFFSET_MAX     (20)
#define CFG_OFFLOAD_NEIGHBOR_REPORT_PER_THRESHOLD_OFFSET_DEFAULT (5)

/*
 * <ini>
 * nr_offload_cache_timeout - time in seconds after which the
 * neighbor report cache is marked as timed out and any of the triggers would
 * cause a neighbor report request frame to be sent.
 * @Min: 5
 * @Max: 86400
 * @Default: 1200
 *
 * Related : nr_offload_params_bitmask
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_OFFLOAD_NEIGHBOR_REPORT_CACHE_TIMEOUT_NAME \
	"nr_offload_cache_timeout"
#define CFG_OFFLOAD_NEIGHBOR_REPORT_CACHE_TIMEOUT_MIN     (5)
#define CFG_OFFLOAD_NEIGHBOR_REPORT_CACHE_TIMEOUT_MAX     (86400)
#define CFG_OFFLOAD_NEIGHBOR_REPORT_CACHE_TIMEOUT_DEFAULT (1200)

/*
 * <ini>
 * nr_offload_max_req_cap - Max number of neighbor
 * report requests that can be sent to a connected peer in the current session.
 * This counter is reset once a successful roam happens or at cache timeout
 * @Min: 3
 * @Max: 300
 * @Default: 3
 *
 * Related : nr_offload_params_bitmask
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_OFFLOAD_NEIGHBOR_REPORT_MAX_REQ_CAP_NAME \
	"nr_offload_max_req_cap"
#define CFG_OFFLOAD_NEIGHBOR_REPORT_MAX_REQ_CAP_MIN     (3)
#define CFG_OFFLOAD_NEIGHBOR_REPORT_MAX_REQ_CAP_MAX     (300)
#define CFG_OFFLOAD_NEIGHBOR_REPORT_MAX_REQ_CAP_DEFAULT (3)

/*
 * <ini>
 * gTxSchDelay - Enable/Disable Tx sch delay
 * @Min: 0
 * @Max: 5
 * @Default: 0
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_TX_SCH_DELAY_NAME          "gTxSchDelay"
#define CFG_TX_SCH_DELAY_MIN           (0)
#define CFG_TX_SCH_DELAY_MAX           (5)
#define CFG_TX_SCH_DELAY_DEFAULT       (0)

/*
 * <ini>
 * gEnableUnitTestFramework - Enable/Disable unit test framework
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * Usage: Internal (only for dev and test team)
 *
 * </ini>
 */
#define CFG_ENABLE_UNIT_TEST_FRAMEWORK_NAME    "gEnableUnitTestFramework"
#define CFG_ENABLE_UNIT_TEST_FRAMEWORK_MIN     (0)
#define CFG_ENABLE_UNIT_TEST_FRAMEWORK_MAX     (1)
#define CFG_ENABLE_UINT_TEST_FRAMEWORK_DEFAULT (0)

/*
 * <ini>
 * gEnableSecondaryRate - Enable/Disable Secondary Retry Rate feature subset
 *
 * @Min: 0x0
 * @Max: 0x3F
 * @Default: 0x17
 *
 * It is a 32 bit value such that the various bits represent as below -
 * Bit-0 : is Enable/Disable Control for "PPDU Secondary Retry Support"
 * Bit-1 : is Enable/Disable Control for "RTS Black/White-listing Support"
 * Bit-2 : is Enable/Disable Control for "Higher MCS retry restriction
 *         on XRETRY failures"
 * Bit 3-5 : is "Xretry threshold" to use
 * Bit 3~31 : reserved for future use.
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ENABLE_SECONDARY_RATE_NAME          "gEnableSecondaryRate"
#define CFG_ENABLE_SECONDARY_RATE_MIN           (0)
#define CFG_ENABLE_SECONDARY_RATE_MAX           (0x3F)
#define CFG_ENABLE_SECONDARY_RATE_DEFAULT       (0x17)

/*
 * <ini>
 * gEnableChangeChannelBandWidth - Enable/Disable change
 * channel&bandwidth in the mission mode
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * 0 - not allow change channel&bandwidth by setMonChan
 * 1 - allow change channel&bandwidth by setMonChan
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_CHANGE_CHANNEL_BANDWIDTH_NAME    "gEnableChangeChannelBandWidth"
#define CFG_CHANGE_CHANNEL_BANDWIDTH_MIN     (0)
#define CFG_CHANGE_CHANNEL_BANDWIDTH_MAX     (1)
#define CFG_CHANGE_CHANNEL_BANDWIDTH_DEFAULT (0)

/*
 * <ini>
 * gRemoveTimeStampSyncCmd - Enable/Disable to remove time stamp sync cmd
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable/disable the removal of time stamp sync cmd
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_REMOVE_TIME_STAMP_SYNC_CMD_NAME    "gRemoveTimeStampSyncCmd"
#define CFG_REMOVE_TIME_STAMP_SYNC_CMD_MIN     (0)
#define CFG_REMOVE_TIME_STAMP_SYNC_CMD_MAX     (1)
#define CFG_REMOVE_TIME_STAMP_SYNC_CMD_DEFAULT (0)

/*
 * Type declarations
 */

struct hdd_config {
	/* Bitmap to track what is explicitly configured */
	DECLARE_BITMAP(bExplicitCfg, MAX_CFG_INI_ITEMS);

	/* Config parameters */
	bool enable_connected_scan;
	enum hdd_dot11_mode dot11Mode;
	uint32_t nChannelBondingMode24GHz;
	uint32_t nChannelBondingMode5GHz;
	uint32_t ScanResultAgeCount;
	uint8_t nRssiCatGap;
	struct qdf_mac_addr IbssBssid;
	uint32_t AdHocChannel5G;
	uint32_t AdHocChannel24G;

	bool apProtEnabled;
	uint8_t nTxPowerCap;    /* In dBm */
	uint8_t disablePacketFilter;

#ifdef FEATURE_WLAN_DYNAMIC_CVM
	/* Bitmap for operating voltage corner mode */
	uint32_t vc_mode_cfg_bitmap;
#endif

	/* Additional Handoff params */
	uint32_t nPassiveMaxChnTime;    /* in units of milliseconds */
	uint32_t nActiveMaxChnTime;     /* in units of milliseconds */
	uint32_t scan_probe_repeat_time;
	uint32_t scan_num_probes;

	uint32_t nInitialDwellTime;     /* in units of milliseconds */
	bool initial_scan_no_dfs_chnl;

	uint8_t wow_data_inactivity_timeout;

	uint32_t DelayedTriggerFrmInt;

#ifdef FEATURE_WLAN_RA_FILTERING
	uint16_t RArateLimitInterval;
#endif
#ifdef FEATURE_WLAN_SCAN_PNO
	bool PnoOffload;
#endif
#ifdef ENABLE_MTRACE_LOG
	bool enable_mtrace;
#endif
	uint8_t enableDFSChnlScan;
	uint8_t enable_dfs_pno_chnl_scan;
	enum hdd_link_speed_rpt_type reportMaxLinkSpeed;
	int32_t linkSpeedRssiHigh;
	int32_t linkSpeedRssiMid;
	int32_t linkSpeedRssiLow;
	bool enableFirstScan2GOnly;
	bool prevent_link_down;
	uint8_t scanAgingTimeout;
	bool fEnableSNRMonitoring;
	/*PNO related parameters */
#ifdef FEATURE_WLAN_SCAN_PNO
	bool configPNOScanSupport;
	uint32_t configPNOScanTimerRepeatValue;
	uint32_t pno_slow_scan_multiplier;
#endif
	uint8_t isCoalesingInIBSSAllowed;

	/* IBSS Power Save related parameters */
	uint32_t ibssATIMWinSize;
	uint8_t isIbssPowerSaveAllowed;
	uint8_t isIbssPowerCollapseAllowed;
	uint8_t isIbssAwakeOnTxRx;
	uint32_t ibssInactivityCount;
	uint32_t ibssTxSpEndInactivityTime;
	uint32_t ibssPsWarmupTime;
	uint32_t ibssPs1RxChainInAtimEnable;
	bool fRegChangeDefCountry;
	bool advertiseConcurrentOperation;
#ifdef DHCP_SERVER_OFFLOAD
	uint8_t dhcpServerIP[IPADDR_STRING_LENGTH];
#endif /* DHCP_SERVER_OFFLOAD */
	bool enable_mac_spoofing;
	uint8_t sap_11ac_override;
	uint8_t go_11ac_override;
	uint8_t max_scan_count;
	bool etsi13_srd_chan_in_master_mode;
	uint16_t  self_gen_frm_pwr;
#ifdef FEATURE_WLAN_SCAN_PNO
	bool pno_channel_prediction;
	uint8_t top_k_num_of_channels;
	uint8_t stationary_thresh;
	uint32_t channel_prediction_full_scan;
#endif
	bool early_stop_scan_enable;
	int8_t early_stop_scan_min_threshold;
	int8_t early_stop_scan_max_threshold;
	int8_t first_scan_bucket_threshold;
#ifdef FEATURE_LFR_SUBNET_DETECTION
	bool enable_lfr_subnet_detection;
#endif
#ifdef WLAN_FEATURE_TSF
#ifdef WLAN_FEATURE_TSF_PLUS
	uint8_t tsf_ptp_options;
#endif /* WLAN_FEATURE_TSF_PLUS */
#endif
	uint32_t ho_delay_for_rx;
	uint32_t min_delay_btw_roam_scans;
	uint32_t roam_trigger_reason_bitmask;
	uint32_t roam_bg_scan_client_bitmap;

	bool apf_enabled;
	bool adaptive_dwell_mode_enabled;
	enum scan_dwelltime_adaptive_mode scan_adaptive_dwell_mode;
	enum scan_dwelltime_adaptive_mode scan_adaptive_dwell_mode_nc;
	enum scan_dwelltime_adaptive_mode extscan_adaptive_dwell_mode;
	enum scan_dwelltime_adaptive_mode pnoscan_adaptive_dwell_mode;
	enum scan_dwelltime_adaptive_mode global_adapt_dwelltime_mode;
	uint8_t adapt_dwell_lpf_weight;
	uint8_t adapt_dwell_passive_mon_intval;
	uint8_t adapt_dwell_wifi_act_threshold;
	bool bug_report_for_no_scan_results;
	bool indoor_channel_support;
	uint16_t sap_tx_leakage_threshold;
	bool sap_internal_restart;
	enum restart_beaconing_on_ch_avoid_rule
		restart_beaconing_on_chan_avoid_event;
	enum active_apf_mode active_uc_apf_mode;
	enum active_apf_mode active_mc_bc_apf_mode;
	uint8_t he_dynamic_frag_support;
#ifdef WLAN_FEATURE_11AX
	bool enable_ul_mimo;
	bool enable_ul_ofdma;
	uint32_t he_sta_obsspd;
#endif
#ifdef WLAN_SUPPORT_TWT
	bool enable_twt;
	uint32_t twt_congestion_timeout;
#endif
	bool tx_orphan_enable;

	uint8_t scan_backoff_multiplier;
	bool mawc_nlo_enabled;
	uint32_t mawc_nlo_exp_backoff_ratio;
	uint32_t mawc_nlo_init_scan_interval;
	uint32_t mawc_nlo_max_scan_interval;
	bool enable_11d_in_world_mode;
	/* 5G preference parameters for boosting RSSI */
	bool                        enable_5g_band_pref;
	int8_t                      rssi_boost_threshold_5g;
	uint8_t                     rssi_boost_factor_5g;
	uint8_t                     max_rssi_boost_5g;
	/* 5G preference parameters for dropping RSSI*/
	int8_t                      rssi_penalize_threshold_5g;
	uint8_t                     rssi_penalize_factor_5g;
	uint8_t                     max_rssi_penalize_5g;
	bool enable_lprx;
	uint32_t scan_11d_interval;
	bool is_bssid_hint_priority;
#ifdef WLAN_FEATURE_SAE
	bool is_sae_enabled;
#endif
	bool gcmp_enabled;
	bool is_11k_offload_supported;
	uint32_t offload_11k_enable_bitmask;
	uint32_t neighbor_report_offload_params_bitmask;
	uint32_t neighbor_report_offload_time_offset;
	uint32_t neighbor_report_offload_low_rssi_offset;
	uint32_t neighbor_report_offload_bmiss_count_trigger;
	uint32_t neighbor_report_offload_per_threshold_offset;
	uint32_t neighbor_report_offload_cache_timeout;
	uint32_t neighbor_report_offload_max_req_cap;
	bool action_oui_enable;
	uint8_t action_oui_str[ACTION_OUI_MAXIMUM_ID][ACTION_OUI_MAX_STR_LEN];
	uint8_t enable_tx_sch_delay;
	uint32_t enable_secondary_rate;
	bool is_unit_test_framework_enabled;
	bool enable_ftopen;
	bool roam_force_rssi_trigger;
	bool enable_change_channel_bandwidth;

	/* HDD converged ini items are listed below this*/
	bool bug_on_reinit_failure;
	bool is_ramdump_enabled;
	uint32_t iface_change_wait_time;
	uint8_t multicast_host_fw_msgs;
	enum hdd_wext_control private_wext_control;
	uint32_t timer_multiplier;
	bool enablefwprint;
	uint8_t enable_fw_log;

#ifdef WLAN_LOGGING_SOCK_SVC_ENABLE
	/* WLAN Logging */
	bool wlan_logging_enable;
	bool wlan_logging_to_console;
#endif /* WLAN_LOGGING_SOCK_SVC_ENABLE */

#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
	uint32_t wlan_auto_shutdown;
#endif

#ifndef REMOVE_PKT_LOG
	bool enable_packet_log;
#endif
	uint32_t rx_mode;
#ifdef MSM_PLATFORM
	uint32_t bus_bw_high_threshold;
	uint32_t bus_bw_medium_threshold;
	uint32_t bus_bw_low_threshold;
	uint32_t bus_bw_compute_interval;
	uint32_t enable_tcp_delack;
	bool     enable_tcp_limit_output;
	uint32_t enable_tcp_adv_win_scale;
	uint32_t tcp_delack_thres_high;
	uint32_t tcp_delack_thres_low;
	uint32_t tcp_tx_high_tput_thres;
	uint32_t tcp_delack_timer_count;
	bool     enable_tcp_param_update;
	u8  periodic_stats_disp_time;
#endif /* MSM_PLATFORM */
#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL
	uint32_t tx_flow_low_watermark;
	uint32_t tx_flow_hi_watermark_offset;
	uint32_t tx_flow_max_queue_depth;
	uint32_t tx_lbw_flow_low_watermark;
	uint32_t tx_lbw_flow_hi_watermark_offset;
	uint32_t tx_lbw_flow_max_queue_depth;
	uint32_t tx_hbw_flow_low_watermark;
	uint32_t tx_hbw_flow_hi_watermark_offset;
	uint32_t tx_hbw_flow_max_queue_depth;
#endif /* QCA_LL_LEGACY_TX_FLOW_CONTROL */
	uint32_t napi_cpu_affinity_mask;
	/* CPU affinity mask for rx_thread */
	uint32_t rx_thread_affinity_mask;
	uint8_t cpu_map_list[CFG_DP_RPS_RX_QUEUE_CPU_MAP_LIST_LEN];
	bool multicast_replay_filter;
	uint32_t rx_wakelock_timeout;
	uint8_t num_dp_rx_threads;
#ifdef CONFIG_DP_TRACE
	bool enable_dp_trace;
	uint8_t dp_trace_config[DP_TRACE_CONFIG_STRING_LENGTH];
#endif
#ifdef WLAN_NUD_TRACKING
	bool enable_nud_tracking;
#endif
	uint8_t operating_channel;
	uint8_t num_vdevs;
	uint8_t enable_concurrent_sta[CFG_CONCURRENT_IFACE_MAX_LEN];
	uint8_t dbs_scan_selection[CFG_DBS_SCAN_PARAM_LENGTH];
	uint8_t remove_time_stamp_sync_cmd;
#ifdef FEATURE_RUNTIME_PM
	bool runtime_pm;
#endif
	uint8_t inform_bss_rssi_raw;

	bool mac_provision;
	uint32_t provisioned_intf_pool;
	uint32_t derived_intf_pool;
};

#define VAR_OFFSET(_Struct, _Var) (offsetof(_Struct, _Var))
#define VAR_SIZE(_Struct, _Var) (sizeof(((_Struct *)0)->_Var))

#define VAR_FLAGS_NONE         (0)

/* bit 0 is Required or Optional */
#define VAR_FLAGS_REQUIRED     (1 << 0)
#define VAR_FLAGS_OPTIONAL     (0 << 0)

/*
 * bit 1 tells if range checking is required.
 * If less than MIN, assume MIN.
 * If greater than MAX, assume MAX.
 */
#define VAR_FLAGS_RANGE_CHECK  (1 << 1)
#define VAR_FLAGS_RANGE_CHECK_ASSUME_MINMAX (VAR_FLAGS_RANGE_CHECK)

/*
 * bit 2 is range checking that assumes the DEFAULT value
 * If less than MIN, assume DEFAULT,
 * If greater than MAX, assume DEFAULT.
 */
#define VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT (1 << 2)

/*
 * Bit 3 indicates that the config item can be modified dynamicially
 * on a running system
 */
#define VAR_FLAGS_DYNAMIC_CFG (1 << 3)

enum wlan_parameter_type {
	WLAN_PARAM_Integer,
	WLAN_PARAM_SignedInteger,
	WLAN_PARAM_HexInteger,
	WLAN_PARAM_String,
	WLAN_PARAM_MacAddr,
};

#define REG_VARIABLE(_Name, _Type,  _Struct, _VarName,		\
		      _Flags, _Default, _Min, _Max)		\
	{							\
		(_Name),					\
		(_Type),					\
		(_Flags),					\
		0,						\
		VAR_OFFSET(_Struct, _VarName),			\
		VAR_SIZE(_Struct, _VarName),			\
		(_Default),					\
		(_Min),						\
		(_Max),						\
		NULL						\
	}

#define REG_DYNAMIC_VARIABLE(_Name, _Type,  _Struct, _VarName,	\
			      _Flags, _Default, _Min, _Max,	\
			      _CBFunc, _CBParam)		\
	{							\
		(_Name),					\
		(_Type),					\
		(VAR_FLAGS_DYNAMIC_CFG | (_Flags)),		\
		(_CBParam),					\
		VAR_OFFSET(_Struct, _VarName),			\
		VAR_SIZE(_Struct, _VarName),			\
		(_Default),					\
		(_Min),						\
		(_Max),						\
		(_CBFunc)					\
	}

#define REG_VARIABLE_STRING(_Name, _Type,  _Struct, _VarName,	\
			     _Flags, _Default)			\
	{							\
		(_Name),					\
		(_Type),					\
		(_Flags),					\
		0,						\
		VAR_OFFSET(_Struct, _VarName),			\
		VAR_SIZE(_Struct, _VarName),			\
		(unsigned long)(_Default),			\
		0,						\
		0,						\
		NULL						\
	}

struct reg_table_entry {
	char *RegName;          /* variable name in the qcom_cfg.ini file */
	unsigned char RegType;    /* variable type in hdd_config struct */
	unsigned char Flags;    /* Specify optional parms and if RangeCheck is performed */
	unsigned char notifyId; /* Dynamic modification identifier */
	unsigned short VarOffset;       /* offset to field from the base address of the structure */
	unsigned short VarSize; /* size (in bytes) of the field */
	unsigned long VarDefault;       /* default value to use */
	unsigned long VarMin;   /* minimum value, for range checking */
	unsigned long VarMax;   /* maximum value, for range checking */
	/* Dynamic modification notifier */
	void (*pfnDynamicnotify)(struct hdd_context *hdd_ctx,
				 unsigned long notifyId);
};

/**
 * hdd_to_csr_wmm_mode() - Utility function to convert HDD to CSR WMM mode
 *
 * @uint8_t mode - hdd WMM user mode
 *
 * Return: CSR WMM mode
 */
eCsrRoamWmmUserModeType hdd_to_csr_wmm_mode(uint8_t mode);

/* Function declarations and documenation */
QDF_STATUS hdd_parse_config_ini(struct hdd_context *hdd_ctx);

QDF_STATUS hdd_update_mac_config(struct hdd_context *hdd_ctx);
QDF_STATUS hdd_set_sme_config(struct hdd_context *hdd_ctx);
QDF_STATUS hdd_set_policy_mgr_user_cfg(struct hdd_context *hdd_ctx);
QDF_STATUS hdd_set_sme_chan_list(struct hdd_context *hdd_ctx);
bool hdd_update_config_cfg(struct hdd_context *hdd_ctx);
void hdd_cfg_get_global_config(struct hdd_context *hdd_ctx, char *buf,
			       int buflen);

eCsrPhyMode hdd_cfg_xlate_to_csr_phy_mode(enum hdd_dot11_mode dot11Mode);
QDF_STATUS hdd_execute_global_config_command(struct hdd_context *hdd_ctx,
					     char *command);

QDF_STATUS hdd_set_idle_ps_config(struct hdd_context *hdd_ctx, bool val);
void hdd_get_pmkid_modes(struct hdd_context *hdd_ctx,
			 struct pmkid_mode_bits *pmkid_modes);

void hdd_update_tgt_cfg(hdd_handle_t hdd_handle, struct wma_tgt_cfg *cfg);

/**
 * hdd_string_to_u8_array() - used to convert decimal string into u8 array
 * @str: Decimal string
 * @array: Array where converted value is stored
 * @len: Length of the populated array
 * @array_max_len: Maximum length of the array
 *
 * This API is called to convert decimal string (each byte separated by
 * a comma) into an u8 array
 *
 * Return: QDF_STATUS
 */
QDF_STATUS hdd_string_to_u8_array(char *str, uint8_t *array,
				  uint8_t *len, uint16_t array_max_len);

QDF_STATUS hdd_hex_string_to_u16_array(char *str, uint16_t *int_array,
				uint8_t *len, uint8_t int_array_max_len);

void hdd_cfg_print_global_config(struct hdd_context *hdd_ctx);

QDF_STATUS hdd_update_nss(struct hdd_adapter *adapter, uint8_t nss);

/**
 * hdd_dfs_indicate_radar() - Block tx as radar found on the channel
 * @hdd_ctxt: HDD context pointer
 *
 * This function is invoked in atomic context when a radar
 * is found on the SAP current operating channel and Data Tx
 * from netif has to be stopped to honor the DFS regulations.
 * Actions: Stop the netif Tx queues,Indicate Radar present
 * in HDD context for future usage.
 *
 * Return: true on success, else false
 */
bool hdd_dfs_indicate_radar(struct hdd_context *hdd_ctx);
#endif
