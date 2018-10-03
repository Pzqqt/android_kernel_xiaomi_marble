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
#include "wlan_hdd_green_ap_cfg.h"

struct hdd_context;

#define FW_MODULE_LOG_LEVEL_STRING_LENGTH  (512)
#define TX_SCHED_WRR_PARAM_STRING_LENGTH   (50)
#define TX_SCHED_WRR_PARAMS_NUM            (5)
#define CFG_ENABLE_RX_THREAD		BIT(0)
#define CFG_ENABLE_RPS			BIT(1)
#define CFG_ENABLE_NAPI			BIT(2)
#define CFG_ENABLE_DYNAMIC_RPS		BIT(3)
#define CFG_ENABLE_DP_RX_THREADS	BIT(4)

#ifdef DHCP_SERVER_OFFLOAD
#define IPADDR_NUM_ENTRIES     (4)
#define IPADDR_STRING_LENGTH   (16)
#endif

#define CFG_DBS_SCAN_PARAM_LENGTH          (42)

/* Number of items that can be configured */
#define MAX_CFG_INI_ITEMS   1024

#define MAX_PRB_REQ_VENDOR_OUI_INI_LEN 160
#define VENDOR_SPECIFIC_IE_BITMAP 0x20000000

#define CFG_CONCURRENT_IFACE_MAX_LEN 16

#define CFG_TX_AGGREGATION_SIZE_MIN  0
#define CFG_TX_AGGREGATION_SIZE_MAX  64
#define CFG_RX_AGGREGATION_SIZE_MIN  1
#define CFG_RX_AGGREGATION_SIZE_MAX  64

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

#ifdef WLAN_NUD_TRACKING
/*
 * <ini>
 * gEnableNUDTracking - Will enable or disable NUD tracking within driver
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable or disable NUD tracking within driver
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: External
 *
 * <ini>
 */

#define CFG_ENABLE_NUD_TRACKING_NAME            "gEnableNUDTracking"
#define CFG_ENABLE_NUD_TRACKING_MIN             (0)
#define CFG_ENABLE_NUD_TRACKING_MAX             (1)
#define CFG_ENABLE_NUD_TRACKING_DEFAULT         (1)
#endif

#define CFG_OPERATING_CHANNEL_NAME             "gOperatingChannel"
#define CFG_OPERATING_CHANNEL_MIN              (0)
#define CFG_OPERATING_CHANNEL_MAX              (14)
#define CFG_OPERATING_CHANNEL_DEFAULT          (1)

/*
 * <ini>
 * gShortSlotTimeEnabled - It will set slot timing slot.
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to set default timing slot.
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_SHORT_SLOT_TIME_ENABLED_NAME       "gShortSlotTimeEnabled"
#define CFG_SHORT_SLOT_TIME_ENABLED_MIN        WNI_CFG_SHORT_SLOT_TIME_STAMIN
#define CFG_SHORT_SLOT_TIME_ENABLED_MAX        WNI_CFG_SHORT_SLOT_TIME_STAMAX
#define CFG_SHORT_SLOT_TIME_ENABLED_DEFAULT    WNI_CFG_SHORT_SLOT_TIME_STADEF

#define CFG_11D_SUPPORT_ENABLED_NAME           "g11dSupportEnabled"
#define CFG_11D_SUPPORT_ENABLED_MIN            WNI_CFG_11D_ENABLED_STAMIN
#define CFG_11D_SUPPORT_ENABLED_MAX            WNI_CFG_11D_ENABLED_STAMAX
#define CFG_11D_SUPPORT_ENABLED_DEFAULT        WNI_CFG_11D_ENABLED_STADEF       /* Default is ON */

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

#define CFG_11H_SUPPORT_ENABLED_NAME           "g11hSupportEnabled"
#define CFG_11H_SUPPORT_ENABLED_MIN            (0)
#define CFG_11H_SUPPORT_ENABLED_MAX            (1)
#define CFG_11H_SUPPORT_ENABLED_DEFAULT        (1)    /* Default is ON */

/* COUNTRY Code Priority */
#define CFG_COUNTRY_CODE_PRIORITY_NAME         "gCountryCodePriority"
#define CFG_COUNTRY_CODE_PRIORITY_MIN          (0)
#define CFG_COUNTRY_CODE_PRIORITY_MAX          (1)
#define CFG_COUNTRY_CODE_PRIORITY_DEFAULT      (0)

/*
 * <ini>
 * gMaxRxAmpduFactor - Provide the maximum ampdu factor.
 * @Min: 0
 * @Max: 3
 * @Default: 3
 *
 * This ini is used to set default maxampdu factor
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_MAX_RX_AMPDU_FACTOR_NAME         "gMaxRxAmpduFactor"
#define CFG_MAX_RX_AMPDU_FACTOR_MIN          WNI_CFG_MAX_RX_AMPDU_FACTOR_STAMIN
#define CFG_MAX_RX_AMPDU_FACTOR_MAX          WNI_CFG_MAX_RX_AMPDU_FACTOR_STAMAX
#define CFG_MAX_RX_AMPDU_FACTOR_DEFAULT      WNI_CFG_MAX_RX_AMPDU_FACTOR_STADEF

/* Configuration option for HT MPDU density (Table 8-125 802.11-2012)
 * 0 for no restriction
 * 1 for 1/4 micro sec
 * 2 for 1/2 micro sec
 * 3 for 1 micro sec
 * 4 for 2 micro sec
 * 5 for 4 micro sec
 * 6 for 8 micro sec
 * 7 for 16 micro sec
 */
#define CFG_HT_MPDU_DENSITY_NAME               "ght_mpdu_density"
#define CFG_HT_MPDU_DENSITY_MIN                WNI_CFG_MPDU_DENSITY_STAMIN
#define CFG_HT_MPDU_DENSITY_MAX                WNI_CFG_MPDU_DENSITY_STAMAX
#define CFG_HT_MPDU_DENSITY_DEFAULT            WNI_CFG_MPDU_DENSITY_STADEF

#define CFG_REG_CHANGE_DEF_COUNTRY_NAME          "gRegulatoryChangeCountry"
#define CFG_REG_CHANGE_DEF_COUNTRY_DEFAULT       (0)
#define CFG_REG_CHANGE_DEF_COUNTRY_MIN           (0)
#define CFG_REG_CHANGE_DEF_COUNTRY_MAX           (1)

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
 * override_ht20_40_24g - use channel Bonding in 24 GHz from supplicant
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to use channel Bonding in 24 GHz from supplicant if
 * gChannelBondingMode24GHz is set
 *
 * Related: gChannelBondingMode24GHz
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_OVERRIDE_HT40_20_24GHZ_NAME    "override_ht20_40_24g"
#define CFG_OVERRIDE_HT40_20_24GHZ_MIN           0
#define CFG_OVERRIDE_HT40_20_24GHZ_MAX           1
#define CFG_OVERRIDE_HT40_20_24GHZ_DEFAULT       0

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
 * gNeighborScanTimerPeriod - Set neighbor scan timer period
 * @Min: 3
 * @Max: 300
 * @Default: 200
 *
 * This ini is used to set the timer period in secs after
 * which neighbor scan is trigerred.
 *
 * Related: None
 *
 * Supported Feature: LFR Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_NEIGHBOR_SCAN_TIMER_PERIOD_NAME             "gNeighborScanTimerPeriod"
#define CFG_NEIGHBOR_SCAN_TIMER_PERIOD_MIN              (3)
#define CFG_NEIGHBOR_SCAN_TIMER_PERIOD_MAX              (300)
#define CFG_NEIGHBOR_SCAN_TIMER_PERIOD_DEFAULT          (100)

/*
 * <ini>
 * gRoamRestTimeMin - Set min neighbor scan timer period
 * @Min: 3
 * @Max: 300
 * @Default: 200
 *
 * This is the min rest time after which firmware will check for traffic
 * and if there no traffic it will move to a new channel to scan
 * else it will stay on the home channel till gNeighborScanTimerPeriod time
 * and then will move to a new channel to scan.
 *
 * Related: None
 *
 * Supported Feature: LFR Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_NEIGHBOR_SCAN_MIN_TIMER_PERIOD_NAME         "gRoamRestTimeMin"
#define CFG_NEIGHBOR_SCAN_MIN_TIMER_PERIOD_MIN          (3)
#define CFG_NEIGHBOR_SCAN_MIN_TIMER_PERIOD_MAX          (300)
#define CFG_NEIGHBOR_SCAN_MIN_TIMER_PERIOD_DEFAULT      (50)

/*
 * <ini>
 * gOpportunisticThresholdDiff - Set oppurtunistic threshold diff
 * @Min: 0
 * @Max: 127
 * @Default: 0
 *
 * This ini is used to set opportunistic threshold diff.
 * This parameter is the RSSI diff above neighbor lookup
 * threshold, when opportunistic scan should be triggered.
 * MAX value is chosen so that this type of scan can be
 * always enabled by user.
 * MIN value will cause opportunistic scan to be triggered
 * in neighbor lookup RSSI range.
 *
 * Related: None
 *
 * Supported Feature: LFR Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_OPPORTUNISTIC_SCAN_THRESHOLD_DIFF_NAME            "gOpportunisticThresholdDiff"
#define CFG_OPPORTUNISTIC_SCAN_THRESHOLD_DIFF_MIN             (0)
#define CFG_OPPORTUNISTIC_SCAN_THRESHOLD_DIFF_MAX             (127)
#define CFG_OPPORTUNISTIC_SCAN_THRESHOLD_DIFF_DEFAULT         (0)

/*
 * <ini>
 * gNeighborScanChannelList - Set channels to be scanned
 * by firmware for LFR scan
 * @Default: ""
 *
 * This ini is used to set the channels to be scanned
 * by firmware for LFR scan.
 *
 * Related: None
 *
 * Supported Feature: LFR Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_NEIGHBOR_SCAN_CHAN_LIST_NAME                      "gNeighborScanChannelList"
#define CFG_NEIGHBOR_SCAN_CHAN_LIST_DEFAULT                   ""

/*
 * <ini>
 * gNeighborScanChannelMinTime - Set neighbor scan channel min time
 * @Min: 10
 * @Max: 40
 * @Default: 20
 *
 * This ini is used to set the minimum time in secs spent on each
 * channel in LFR scan inside firmware.
 *
 * Related: None
 *
 * Supported Feature: LFR Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_NEIGHBOR_SCAN_MIN_CHAN_TIME_NAME                  "gNeighborScanChannelMinTime"
#define CFG_NEIGHBOR_SCAN_MIN_CHAN_TIME_MIN                   (10)
#define CFG_NEIGHBOR_SCAN_MIN_CHAN_TIME_MAX                   (40)
#define CFG_NEIGHBOR_SCAN_MIN_CHAN_TIME_DEFAULT               (20)

/*
 * <ini>
 * gNeighborScanChannelMaxTime - Set neighbor scan channel max time
 * @Min: 3
 * @Max: 300
 * @Default: 30
 *
 * This ini is used to set the maximum time in secs spent on each
 * channel in LFR scan inside firmware.
 *
 * Related: None
 *
 * Supported Feature: LFR Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_NEIGHBOR_SCAN_MAX_CHAN_TIME_NAME                  "gNeighborScanChannelMaxTime"
#define CFG_NEIGHBOR_SCAN_MAX_CHAN_TIME_MIN                   (3)
#define CFG_NEIGHBOR_SCAN_MAX_CHAN_TIME_MAX                   (300)
#define CFG_NEIGHBOR_SCAN_MAX_CHAN_TIME_DEFAULT               (30)

/*
 * <ini>
 * gNeighborScanRefreshPeriod - Set neighbor scan refresh period
 * @Min: 1000
 * @Max: 60000
 * @Default: 20000
 *
 * This ini is used by firmware to set scan refresh period
 * in msecs for lfr scan.
 *
 * Related: None
 *
 * Supported Feature: LFR Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_NAME         "gNeighborScanRefreshPeriod"
#define CFG_NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_MIN          (1000)
#define CFG_NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_MAX          (60000)
#define CFG_NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_DEFAULT      (20000)

/*
 * <ini>
 * gEmptyScanRefreshPeriod - Set empty scan refresh period
 * @Min: 0
 * @Max: 60000
 * @Default: 0
 *
 * This ini is used by firmware to set scan period in msecs
 * following empty scan results.
 *
 * Related: None
 *
 * Supported Feature: LFR Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_EMPTY_SCAN_REFRESH_PERIOD_NAME         "gEmptyScanRefreshPeriod"
#define CFG_EMPTY_SCAN_REFRESH_PERIOD_MIN          (0)
#define CFG_EMPTY_SCAN_REFRESH_PERIOD_MAX          (60000)
#define CFG_EMPTY_SCAN_REFRESH_PERIOD_DEFAULT      (0)

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

/*
 * <ini>
 * gRoamPrefer5GHz - Prefer roaming to 5GHz Bss
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to inform FW to prefer roaming to 5GHz BSS
 *
 * Related: None
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ROAM_PREFER_5GHZ                  "gRoamPrefer5GHz"
#define CFG_ROAM_PREFER_5GHZ_MIN              (0)
#define CFG_ROAM_PREFER_5GHZ_MAX              (1)
#define CFG_ROAM_PREFER_5GHZ_DEFAULT          (1)

/*
 * <ini>
 * gRoamIntraBand - Prefer roaming within Band
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to inform FW to prefer roaming within band
 *
 * Related: None
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ROAM_INTRA_BAND                   "gRoamIntraBand"
#define CFG_ROAM_INTRA_BAND_MIN               (0)
#define CFG_ROAM_INTRA_BAND_MAX               (1)
#define CFG_ROAM_INTRA_BAND_DEFAULT           (0)

/*
 * <ini>
 * FastRoamEnabled - Enable fast roaming
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to inform FW to enable fast roaming
 *
 * Related: None
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_LFR_FEATURE_ENABLED_NAME                        "FastRoamEnabled"
#define CFG_LFR_FEATURE_ENABLED_MIN                         (0)
#define CFG_LFR_FEATURE_ENABLED_MAX                         (1)
#define CFG_LFR_FEATURE_ENABLED_DEFAULT                     (0)

/*
 * <ini>
 * FastTransitionEnabled - Enable fast transition in case of 11r and ese.
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to turn ON/OFF the whole neighbor roam, pre-auth, reassoc.
 * With this turned OFF 11r will completely not work. For 11r this flag has to
 * be ON. For ESE fastroam will not work.
 *
 * Related: None
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_FAST_TRANSITION_ENABLED_NAME                    "FastTransitionEnabled"
#define CFG_FAST_TRANSITION_ENABLED_NAME_MIN                (0)
#define CFG_FAST_TRANSITION_ENABLED_NAME_MAX                (1)
#define CFG_FAST_TRANSITION_ENABLED_NAME_DEFAULT            (1)

/*
 * <ini>
 * RoamRssiDiff - Enable roam based on rssi
 * @Min: 0
 * @Max: 30
 * @Default: 5
 *
 * This INI is used to decide whether to Roam or not based on RSSI. AP1 is the
 * currently associated AP and AP2 is chosen for roaming. The Roaming will
 * happen only if AP2 has better Signal Quality and it has a RSSI better than
 * AP2. RoamRssiDiff is the number of units (typically measured in dB) AP2
 * is better than AP1.
 *
 * Related: None
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ROAM_RSSI_DIFF_NAME                             "RoamRssiDiff"
#define CFG_ROAM_RSSI_DIFF_MIN                              (0)
#define CFG_ROAM_RSSI_DIFF_MAX                              (30)
#define CFG_ROAM_RSSI_DIFF_DEFAULT                          (5)

/*
 * <ini>
 * gRoamScanNProbes - Sets the number of probes to be sent for firmware roaming
 * @Min: 1
 * @Max: 10
 * @Default: 2
 *
 * This INI is used to set the maximum number of probes the firmware can send
 * for firmware internal roaming cases.
 *
 * Related: None
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ROAM_SCAN_N_PROBES                              "gRoamScanNProbes"
#define CFG_ROAM_SCAN_N_PROBES_MIN                          (1)
#define CFG_ROAM_SCAN_N_PROBES_MAX                          (10)
#define CFG_ROAM_SCAN_N_PROBES_DEFAULT                      (2)

/*
 * <ini>
 * gRoamScanHomeAwayTime - Sets the Home Away Time to firmware
 * @Min: 0
 * @Max: 300
 * @Default: 0
 *
 * Home Away Time should be at least equal to (gNeighborScanChannelMaxTime
 * + (2*RFS)), where RFS is the RF Switching time(3). It is twice RFS
 * to consider the time to go off channel and return to the home channel.
 *
 * Related: gNeighborScanChannelMaxTime
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ROAM_SCAN_HOME_AWAY_TIME                        "gRoamScanHomeAwayTime"
#define CFG_ROAM_SCAN_HOME_AWAY_TIME_MIN                    (0)
#define CFG_ROAM_SCAN_HOME_AWAY_TIME_MAX                    (300)
#define CFG_ROAM_SCAN_HOME_AWAY_TIME_DEFAULT                (0)

/*
 * <ini>
 * OkcEnabled - Enable OKC(Oppurtunistic Key Caching)
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This INI is used to enable OKC feature
 *
 * Related: None
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_OKC_FEATURE_ENABLED_NAME                       "OkcEnabled"
#define CFG_OKC_FEATURE_ENABLED_MIN                        (0)
#define CFG_OKC_FEATURE_ENABLED_MAX                        (1)
#define CFG_OKC_FEATURE_ENABLED_DEFAULT                    (1)

/*
 * <ini>
 * gRoamScanOffloadEnabled - Enable Roam Scan Offload
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This INI is used to enable Roam Scan Offload in firmware
 *
 * Related: None
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ROAM_SCAN_OFFLOAD_ENABLED                       "gRoamScanOffloadEnabled"
#define CFG_ROAM_SCAN_OFFLOAD_ENABLED_MIN                   (0)
#define CFG_ROAM_SCAN_OFFLOAD_ENABLED_MAX                   (1)
#define CFG_ROAM_SCAN_OFFLOAD_ENABLED_DEFAULT               (1)

/*
 * <ini>
 * gRoamRescanRssiDiff - Sets RSSI for Scan trigger in firmware
 * @Min: 0
 * @Max: 100
 * @Default: 5
 *
 * This INI is the drop in RSSI value that will trigger a precautionary
 * scan by firmware. Max value is chosen in such a way that this type
 * of scan can be disabled by user.
 *
 * Related: None
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ROAM_RESCAN_RSSI_DIFF_NAME                  "gRoamRescanRssiDiff"
#define CFG_ROAM_RESCAN_RSSI_DIFF_MIN                   (0)
#define CFG_ROAM_RESCAN_RSSI_DIFF_MAX                   (100)
#define CFG_ROAM_RESCAN_RSSI_DIFF_DEFAULT               (5)

/*
 * <ini>
 * gDroppedPktDisconnectTh - Sets dropped packet threshold in firmware
 * @Min: 0
 * @Max: 512
 * @Default: 512
 *
 * This INI is the packet drop threshold will trigger disconnect from remote
 * peer.
 *
 * Related: None
 *
 * Supported Feature: connection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_DROPPED_PKT_DISCONNECT_TH_NAME      "gDroppedPktDisconnectTh"
#define CFG_DROPPED_PKT_DISCONNECT_TH_MIN       (0)
#define CFG_DROPPED_PKT_DISCONNECT_TH_MAX       (65535)
#define CFG_DROPPED_PKT_DISCONNECT_TH_DEFAULT   (512)

/*
 * <ini>
 * gForce1x1Exception - force 1x1 when connecting to certain peer
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This INI when enabled will force 1x1 connection with certain peer.
 *
 *
 * Related: None
 *
 * Supported Feature: connection
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_FORCE_1X1_NAME      "gForce1x1Exception"
#define CFG_FORCE_1X1_MIN       (0)
#define CFG_FORCE_1X1_MAX       (1)
#define CFG_FORCE_1X1_DEFAULT   (1)

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
#define CFG_ENABLE_FAST_ROAM_IN_CONCURRENCY          "gEnableFastRoamInConcurrency"
#define CFG_ENABLE_FAST_ROAM_IN_CONCURRENCY_MIN      (0)
#define CFG_ENABLE_FAST_ROAM_IN_CONCURRENCY_MAX      (1)
#define CFG_ENABLE_FAST_ROAM_IN_CONCURRENCY_DEFAULT  (1)

/*
 * <ini>
 * gSelect5GHzMargin - Sets RSSI preference for 5GHz over 2.4GHz AP.
 * @Min: 0
 * @Max: 60
 * @Default: 0
 *
 * Prefer connecting to 5G AP even if its RSSI is lower by gSelect5GHzMargin
 * dBm than 2.4G AP. This feature requires the dependent cfg.ini
 * "gRoamPrefer5GHz" set to 1
 *
 * Related: gRoamPrefer5GHz
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_STRICT_5GHZ_PREF_BY_MARGIN                 "gSelect5GHzMargin"
#define CFG_STRICT_5GHZ_PREF_BY_MARGIN_MIN             (0)
#define CFG_STRICT_5GHZ_PREF_BY_MARGIN_MAX             (60)
#define CFG_STRICT_5GHZ_PREF_BY_MARGIN_DEFAULT         (0)

/*
 * <ini>
 * gRoamScanHiRssiMaxCount - Sets 5GHz maximum scan count
 * @Min: 0
 * @Max: 10
 * @Default: 3
 *
 * This INI is used to set maximum scan count in 5GHz
 *
 * Related: None
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ROAM_SCAN_HI_RSSI_MAXCOUNT_NAME         "gRoamScanHiRssiMaxCount"
#define CFG_ROAM_SCAN_HI_RSSI_MAXCOUNT_MIN          (0)
#define CFG_ROAM_SCAN_HI_RSSI_MAXCOUNT_MAX          (10)
#define CFG_ROAM_SCAN_HI_RSSI_MAXCOUNT_DEFAULT      (3)

/*
 * <ini>
 * gRoamScanHiRssiDelta - Sets RSSI Delta for scan trigger
 * @Min: 0
 * @Max: 16
 * @Default: 10
 *
 * This INI is used to set change in RSSI at which scan is triggered
 * in 5GHz.
 *
 * Related: None
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ROAM_SCAN_HI_RSSI_DELTA_NAME           "gRoamScanHiRssiDelta"
#define CFG_ROAM_SCAN_HI_RSSI_DELTA_MIN            (0)
#define CFG_ROAM_SCAN_HI_RSSI_DELTA_MAX            (16)
#define CFG_ROAM_SCAN_HI_RSSI_DELTA_DEFAULT        (10)

/*
 * <ini>
 * gRoamScanHiRssiDelay - Sets minimum delay between 5GHz scans
 * @Min: 5000
 * @Max: 0x7fffffff
 * @Default: 15000
 *
 * This INI is used to set the minimum delay between 5GHz scans.
 *
 * Related: None
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ROAM_SCAN_HI_RSSI_DELAY_NAME            "gRoamScanHiRssiDelay"
#define CFG_ROAM_SCAN_HI_RSSI_DELAY_MIN             (5000)
#define CFG_ROAM_SCAN_HI_RSSI_DELAY_MAX             (0x7fffffff)
#define CFG_ROAM_SCAN_HI_RSSI_DELAY_DEFAULT         (15000)

/*
 * <ini>
 * gRoamScanHiRssiUpperBound - Sets upper bound after which 5GHz scan
 * @Min: -66
 * @Max: 0
 * @Default: -30
 *
 * This INI is used to set the RSSI upper bound above which the 5GHz scan
 * will not be performed.
 *
 * Related: None
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ROAM_SCAN_HI_RSSI_UB_NAME              "gRoamScanHiRssiUpperBound"
#define CFG_ROAM_SCAN_HI_RSSI_UB_MIN               (-66)
#define CFG_ROAM_SCAN_HI_RSSI_UB_MAX               (0)
#define CFG_ROAM_SCAN_HI_RSSI_UB_DEFAULT           (-30)

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
#define CFG_ROAM_BG_SCAN_BAD_RSSI_OFFSET_2G_NAME "roam_bad_rssi_thresh_offset_2g"
#define CFG_ROAM_BG_SCAN_BAD_RSSI_OFFSET_2G_MIN     (0)
#define CFG_ROAM_BG_SCAN_BAD_RSSI_OFFSET_2G_MAX     (86)
#define CFG_ROAM_BG_SCAN_BAD_RSSI_OFFSET_2G_DEFAULT (40)

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
 * Timer waiting for interface up from the upper layer. If
 * this timer expires all the cds modules shall be closed.
 * Time Units: ms
 */
#define CFG_INTERFACE_CHANGE_WAIT_NAME    "gInterfaceChangeWait"
#define CFG_INTERFACE_CHANGE_WAIT_MIN     (10)
#define CFG_INTERFACE_CHANGE_WAIT_MAX     (500000)
#define CFG_INTERFACE_CHANGE_WAIT_DEFAULT (10000)

/*
 * <ini>
 * gShortPreamble - Set Short Preamble
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to set default short Preamble
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_SHORT_PREAMBLE_NAME                "gShortPreamble"
#define CFG_SHORT_PREAMBLE_MIN                 WNI_CFG_SHORT_PREAMBLE_STAMIN
#define CFG_SHORT_PREAMBLE_MAX                 WNI_CFG_SHORT_PREAMBLE_STAMAX
#define CFG_SHORT_PREAMBLE_DEFAULT             WNI_CFG_SHORT_PREAMBLE_STADEF

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
 * wlm_latency_enable - WLM latency Enable
 *
 * @min: 0
 * @max: 1
 * @default: 0
 *
 * 0 - disable
 * 1 - enable
 *
 * </ini>
 */
#define CFG_LATENCY_ENABLE_NAME    "wlm_latency_enable"
#define CFG_LATENCY_ENABLE_MIN     (0)
#define CFG_LATENCY_ENABLE_MAX     (1)
#define CFG_LATENCY_ENABLE_DEFAULT (0)

/*
 * <ini>
 * wlm_latency_level - WLM latency level
 * Define 4 latency level to gain latency
 *
 * @min: 0
 * @max: 3
 * @defalut: 0
 *
 * 0 - normal
 * 1 - moderate
 * 2 - low
 * 3 - ultralow
 *
 * </ini>
 */
#define CFG_LATENCY_LEVEL_NAME    "wlm_latency_level"
#define CFG_LATENCY_LEVEL_MIN     (0)
#define CFG_LATENCY_LEVEL_MAX     (3)
#define CFG_LATENCY_LEVEL_DEFAULT (0)

/*
 * <ini>
 * wlm_latency_flags_normal - WLM flags setting for normal level
 *
 * @min: 0x0
 * @max: 0xffffffff
 * @defalut: 0x0
 *
 * |31  12|  11  |  10  |9    8|7    6|5    4|3    2|  1  |  0  |
 * +------+------+------+------+------+------+------+-----+-----+
 * | RSVD | SSLP | CSLP | RSVD | Roam | RSVD | DWLT | DFS | SUP |
 * +------+-------------+-------------+-------------------------+
 * |  WAL |      PS     |     Roam    |         Scan            |
 *
 * bit 0: Avoid scan request from HLOS if setting
 * bit 1: Skip DFS channel SCAN if setting
 * bit 2-3: Define policy of dwell time/duration for each foreign channel
 *     (b2 b3)
 *     (0  0 ): Default scan dwell time
 *     (0  1 ): Reserve
 *     (1  0 ): Shrink off channel dwell time
 *     (1  1 ): Reserve
 * bit 4-5: Reserve for scan
 * bit 6-7: Define roaming policy
 *     (b6 b7)
 *     (0  0 ): Default roaming behavior, allow roaming in all scenarios
 *     (0  1 ): Disallow all roaming
 *     (1  0 ): Allow roaming when final bmissed
 *     (1  1 ): Reserve
 * bit 8-9: Reserve for roaming
 * bit 10: Disable css power collapse if setting
 * bit 11: Disable sys sleep if setting
 * bit 12-31: Reserve for future useage
 *
 * </ini>
 */
#define CFG_LATENCY_FLAGS_NORMAL_NAME    "wlm_latency_flags_normal"
#define CFG_LATENCY_FLAGS_NORMAL_MIN     (0x0)
#define CFG_LATENCY_FLAGS_NORMAL_MAX     (0xffffffff)
#define CFG_LATENCY_FLAGS_NORMAL_DEFAULT (0x0)

/*
 * <ini>
 * wlm_latency_flags_moderate - WLM flags setting for moderate level
 *
 * @min: 0x0
 * @max: 0xffffffff
 * @defalut: 0x8
 *
 * |31  12|  11  |  10  |9    8|7    6|5    4|3    2|  1  |  0  |
 * +------+------+------+------+------+------+------+-----+-----+
 * | RSVD | SSLP | CSLP | RSVD | Roam | RSVD | DWLT | DFS | SUP |
 * +------+-------------+-------------+-------------------------+
 * |  WAL |      PS     |     Roam    |         Scan            |
 *
 * bit 0: Avoid scan request from HLOS if setting
 * bit 1: Skip DFS channel SCAN if setting
 * bit 2-3: Define policy of dwell time/duration for each foreign channel
 *     (b2 b3)
 *     (0  0 ): Default scan dwell time
 *     (0  1 ): Reserve
 *     (1  0 ): Shrink off channel dwell time
 *     (1  1 ): Reserve
 * bit 4-5: Reserve for scan
 * bit 6-7: Define roaming policy
 *     (b6 b7)
 *     (0  0 ): Default roaming behavior, allow roaming in all scenarios
 *     (0  1 ): Disallow all roaming
 *     (1  0 ): Allow roaming when final bmissed
 *     (1  1 ): Reserve
 * bit 8-9: Reserve for roaming
 * bit 10: Disable css power collapse if setting
 * bit 11: Disable sys sleep if setting
 * bit 12-31: Reserve for future useage
 *
 * </ini>
 */
#define CFG_LATENCY_FLAGS_MODERATE_NAME    "wlm_latency_flags_moderate"
#define CFG_LATENCY_FLAGS_MODERATE_MIN     (0x0)
#define CFG_LATENCY_FLAGS_MODERATE_MAX     (0xffffffff)
#define CFG_LATENCY_FLAGS_MODERATE_DEFAULT (0x8)

/*
 * <ini>
 * wlm_latency_flags_low - WLM flags setting for low level
 *
 * @min: 0x0
 * @max: 0xffffffff
 * @defalut: 0xa
 *
 * |31  12|  11  |  10  |9    8|7    6|5    4|3    2|  1  |  0  |
 * +------+------+------+------+------+------+------+-----+-----+
 * | RSVD | SSLP | CSLP | RSVD | Roam | RSVD | DWLT | DFS | SUP |
 * +------+-------------+-------------+-------------------------+
 * |  WAL |      PS     |     Roam    |         Scan            |
 *
 * bit 0: Avoid scan request from HLOS if setting
 * bit 1: Skip DFS channel SCAN if setting
 * bit 2-3: Define policy of dwell time/duration for each foreign channel
 *     (b2 b3)
 *     (0  0 ): Default scan dwell time
 *     (0  1 ): Reserve
 *     (1  0 ): Shrink off channel dwell time
 *     (1  1 ): Reserve
 * bit 4-5: Reserve for scan
 * bit 6-7: Define roaming policy
 *     (b6 b7)
 *     (0  0 ): Default roaming behavior, allow roaming in all scenarios
 *     (0  1 ): Disallow all roaming
 *     (1  0 ): Allow roaming when final bmissed
 *     (1  1 ): Reserve
 * bit 8-9: Reserve for roaming
 * bit 10: Disable css power collapse if setting
 * bit 11: Disable sys sleep if setting
 * bit 12-31: Reserve for future useage
 *
 * </ini>
 */
#define CFG_LATENCY_FLAGS_LOW_NAME    "wlm_latency_flags_low"
#define CFG_LATENCY_FLAGS_LOW_MIN     (0x0)
#define CFG_LATENCY_FLAGS_LOW_MAX     (0xffffffff)
#define CFG_LATENCY_FLAGS_LOW_DEFAULT (0xa)

/*
 * <ini>
 * wlm_latency_flags_ultralow - WLM flags setting for ultralow level
 *
 * @min: 0x0
 * @max: 0xffffffff
 * @defalut: 0xc8a
 *
 * |31  12|  11  |  10  |9    8|7    6|5    4|3    2|  1  |  0  |
 * +------+------+------+------+------+------+------+-----+-----+
 * | RSVD | SSLP | CSLP | RSVD | Roam | RSVD | DWLT | DFS | SUP |
 * +------+-------------+-------------+-------------------------+
 * |  WAL |      PS     |     Roam    |         Scan            |
 *
 * bit 0: Avoid scan request from HLOS if setting
 * bit 1: Skip DFS channel SCAN if setting
 * bit 2-3: Define policy of dwell time/duration for each foreign channel
 *     (b2 b3)
 *     (0  0 ): Default scan dwell time
 *     (0  1 ): Reserve
 *     (1  0 ): Shrink off channel dwell time
 *     (1  1 ): Reserve
 * bit 4-5: Reserve for scan
 * bit 6-7: Define roaming policy
 *     (b6 b7)
 *     (0  0 ): Default roaming behavior, allow roaming in all scenarios
 *     (0  1 ): Disallow all roaming
 *     (1  0 ): Allow roaming when final bmissed
 *     (1  1 ): Reserve
 * bit 8-9: Reserve for roaming
 * bit 10: Disable css power collapse if setting
 * bit 11: Disable sys sleep if setting
 * bit 12-31: Reserve for future useage
 *
 * </ini>
 */
#define CFG_LATENCY_FLAGS_ULTRALOW_NAME    "wlm_latency_flags_ultralow"
#define CFG_LATENCY_FLAGS_ULTRALOW_MIN     (0x0)
#define CFG_LATENCY_FLAGS_ULTRALOW_MAX     (0xffffffff)
#define CFG_LATENCY_FLAGS_ULTRALOW_DEFAULT (0xc8a)

#define CFG_INTF0_MAC_ADDR_NAME                  "Intf0MacAddress"
#define CFG_INTF0_MAC_ADDR_MIN                   "000000000000"
#define CFG_INTF0_MAC_ADDR_MAX                   "ffffffffffff"
#define CFG_INTF0_MAC_ADDR_DEFAULT               "000AF58989FF"

#define CFG_INTF1_MAC_ADDR_NAME                  "Intf1MacAddress"
#define CFG_INTF1_MAC_ADDR_MIN                   "000000000000"
#define CFG_INTF1_MAC_ADDR_MAX                   "ffffffffffff"
#define CFG_INTF1_MAC_ADDR_DEFAULT               "000AF58989FE"

#define CFG_INTF2_MAC_ADDR_NAME                  "Intf2MacAddress"
#define CFG_INTF2_MAC_ADDR_MIN                   "000000000000"
#define CFG_INTF2_MAC_ADDR_MAX                   "ffffffffffff"
#define CFG_INTF2_MAC_ADDR_DEFAULT               "000AF58989FD"

#define CFG_INTF3_MAC_ADDR_NAME                  "Intf3MacAddress"
#define CFG_INTF3_MAC_ADDR_MIN                   "000000000000"
#define CFG_INTF3_MAC_ADDR_MAX                   "ffffffffffff"
#define CFG_INTF3_MAC_ADDR_DEFAULT               "000AF58989FC"

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
 * gEnableApUapsd - Enable/disable UAPSD for SoftAP
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to setup setup U-APSD for Acs at association
 *
 * Related: None.
 *
 * Supported Feature: SAP
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_AP_QOS_UAPSD_MODE_NAME             "gEnableApUapsd"
#define CFG_AP_QOS_UAPSD_MODE_MIN              (0)
#define CFG_AP_QOS_UAPSD_MODE_MAX              (1)
#define CFG_AP_QOS_UAPSD_MODE_DEFAULT          (1)

/*
 * <ini>
 * gEnableApRandomBssid - Create ramdom BSSID
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to create a random BSSID in SoftAP mode to meet
 * the Android requirement.
 *
 * Related: None.
 *
 * Supported Feature: SAP
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_AP_ENABLE_RANDOM_BSSID_NAME            "gEnableApRandomBssid"
#define CFG_AP_ENABLE_RANDOM_BSSID_MIN             (0)
#define CFG_AP_ENABLE_RANDOM_BSSID_MAX             (1)
#define CFG_AP_ENABLE_RANDOM_BSSID_DEFAULT         (0)

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

/*
 * <ini>
 * gApProtection - Set AP protection parameter
 * @Min: 0x0
 * @Max: 0xFFFF
 * @Default: 0xBFFF
 *
 * This ini is used to set AP protection parameter
 * Bit map for CFG_AP_PROTECTION_MODE_DEFAULT
 * LOWER byte for associated stations
 * UPPER byte for overlapping stations
 * each byte will have the following info
 * bit15 bit14 bit13     bit12  bit11 bit10    bit9     bit8
 * OBSS  RIFS  LSIG_TXOP NON_GF HT20  FROM_11G FROM_11B FROM_11A
 * bit7  bit6  bit5      bit4   bit3  bit2     bit1     bit0
 * OBSS  RIFS  LSIG_TXOP NON_GF HT_20 FROM_11G FROM_11B FROM_11A
 *
 * Related: None.
 *
 * Supported Feature: SAP
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_AP_PROTECTION_MODE_NAME            "gApProtection"
#define CFG_AP_PROTECTION_MODE_MIN             (0x0)
#define CFG_AP_PROTECTION_MODE_MAX             (0xFFFF)
#define CFG_AP_PROTECTION_MODE_DEFAULT         (0xBFFF)

/*
 * <ini>
 * gEnableApOBSSProt - Enable/Disable AP OBSS protection
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable/disable AP OBSS protection
 *
 * Related: None.
 *
 * Supported Feature: SAP
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_AP_OBSS_PROTECTION_MODE_NAME       "gEnableApOBSSProt"
#define CFG_AP_OBSS_PROTECTION_MODE_MIN        (0)
#define CFG_AP_OBSS_PROTECTION_MODE_MAX        (1)
#define CFG_AP_OBSS_PROTECTION_MODE_DEFAULT    (0)

/*
 * <ini>
 * gDisableIntraBssFwd - Disable intrs BSS Rx packets
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to disbale to forward Intra-BSS Rx packets when
 * ap_isolate=1 in hostapd.conf
 *
 * Related: None.
 *
 * Supported Feature: SAP
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_AP_STA_SECURITY_SEPERATION_NAME    "gDisableIntraBssFwd"
#define CFG_AP_STA_SECURITY_SEPERATION_MIN     (0)
#define CFG_AP_STA_SECURITY_SEPERATION_MAX     (1)
#define CFG_AP_STA_SECURITY_SEPERATION_DEFAULT (0)

#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
#define CFG_WLAN_AUTO_SHUTDOWN              "gWlanAutoShutdown"
#define CFG_WLAN_AUTO_SHUTDOWN_MIN          (0)
#define CFG_WLAN_AUTO_SHUTDOWN_MAX          (86400)   /* Max 1 day timeout */
#define CFG_WLAN_AUTO_SHUTDOWN_DEFAULT      (0)
#endif

#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
/*
 * <ini>
 * gWlanMccToSccSwitchMode - Control SAP channel.
 * @Min: 0
 * @Max: 5
 * @Default: 0
 *
 * This ini is used to override SAP channel.
 * If gWlanMccToSccSwitchMode = 0: disabled.
 * If gWlanMccToSccSwitchMode = 1: Enable switch.
 * If gWlainMccToSccSwitchMode = 2: Force switch with SAP restart.
 * If gWlainMccToSccSwitchMode = 3: Force switch without SAP restart.
 * If gWlainMccToSccSwitchMode = 4: Switch using
 *		fav channel(s)without SAP restart.
 * If gWlainMccToSccSwitchMode = 5: Force switch without SAP restart.MCC allowed
 *					in exceptional cases.
 * If gWlainMccToSccSwitchMode = 6: Force Switch without SAP restart only in
					user preffered band.
 * Related: None.
 *
 * Supported Feature: Concurrency
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_WLAN_MCC_TO_SCC_SWITCH_MODE          "gWlanMccToSccSwitchMode"
#define CFG_WLAN_MCC_TO_SCC_SWITCH_MODE_MIN      (QDF_MCC_TO_SCC_SWITCH_DISABLE)
#define CFG_WLAN_MCC_TO_SCC_SWITCH_MODE_MAX \
		   (QDF_MCC_TO_SCC_SWITCH_MAX - 1)
#define CFG_WLAN_MCC_TO_SCC_SWITCH_MODE_DEFAULT  (QDF_MCC_TO_SCC_SWITCH_DISABLE)
#endif

#define CFG_DISABLE_PACKET_FILTER		"gDisablePacketFilter"
#define CFG_DISABLE_PACKET_FILTER_MIN		(0)
#define CFG_DISABLE_PACKET_FILTER_MAX		(1)
#define CFG_DISABLE_PACKET_FILTER_DEFAULT	(1)

#define CFG_VCC_RSSI_TRIGGER_NAME             "gVccRssiTrigger"
#define CFG_VCC_RSSI_TRIGGER_MIN              (0)
#define CFG_VCC_RSSI_TRIGGER_MAX              (80)
#define CFG_VCC_RSSI_TRIGGER_DEFAULT          (80)

#define CFG_VCC_UL_MAC_LOSS_THRESH_NAME       "gVccUlMacLossThresh"
#define CFG_VCC_UL_MAC_LOSS_THRESH_MIN        (0)
#define CFG_VCC_UL_MAC_LOSS_THRESH_MAX        (9)
#define CFG_VCC_UL_MAC_LOSS_THRESH_DEFAULT    (9)

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
 * gPassiveMinChannelTimeConc - Minimum passive scan time in milliseconds.
 * @Min: 0
 * @Max: 10000
 * @Default: 60
 *
 * This ini is used to set minimum passive scan time in STA+SAP concurrent
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
#define CFG_PASSIVE_MIN_CHANNEL_TIME_CONC_NAME      "gPassiveMinChannelTimeConc"
#define CFG_PASSIVE_MIN_CHANNEL_TIME_CONC_MIN       (0)
#define CFG_PASSIVE_MIN_CHANNEL_TIME_CONC_MAX       (10000)
#define CFG_PASSIVE_MIN_CHANNEL_TIME_CONC_DEFAULT   (60)

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
 * gActiveMinChannelTimeConc - Minimum active scan time in milliseconds..
 * @Min: 0
 * @Max: 10000
 * @Default: 20
 *
 * This ini is used to set minimum active scan time in STA+SAP concurrent
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
#define CFG_ACTIVE_MIN_CHANNEL_TIME_CONC_NAME       "gActiveMinChannelTimeConc"
#define CFG_ACTIVE_MIN_CHANNEL_TIME_CONC_MIN        (0)
#define CFG_ACTIVE_MIN_CHANNEL_TIME_CONC_MAX        (10000)
#define CFG_ACTIVE_MIN_CHANNEL_TIME_CONC_DEFAULT    (20)

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

#define CFG_MAX_PS_POLL_NAME                   "gMaxPsPoll"
#define CFG_MAX_PS_POLL_MIN                    WNI_CFG_MAX_PS_POLL_STAMIN
#define CFG_MAX_PS_POLL_MAX                    WNI_CFG_MAX_PS_POLL_STAMAX
#define CFG_MAX_PS_POLL_DEFAULT                WNI_CFG_MAX_PS_POLL_STADEF

#define CFG_MAX_TX_POWER_NAME                   "gTxPowerCap"
#define CFG_MAX_TX_POWER_MIN                    WNI_CFG_CURRENT_TX_POWER_LEVEL_STAMIN
#define CFG_MAX_TX_POWER_MAX                    WNI_CFG_CURRENT_TX_POWER_LEVEL_STAMAX
/* Not to use CFG default because if no registry setting, this is ignored by SME. */
#define CFG_MAX_TX_POWER_DEFAULT                WNI_CFG_CURRENT_TX_POWER_LEVEL_STAMAX

/* This ini controls driver to honor/dishonor power constraint from AP */
#define CFG_TX_POWER_CTRL_NAME                 "gAllowTPCfromAP"
#define CFG_TX_POWER_CTRL_DEFAULT              (1)
#define CFG_TX_POWER_CTRL_MIN                  (0)
#define CFG_TX_POWER_CTRL_MAX                  (1)

/*
 * <ini>
 * gMaxLIModulatedDTIM - Set MaxLIModulate Dtim
 * @Min: 1
 * @Max: 10
 * @Default: 10
 *
 * This ini is used to set default MaxLIModulatedDTIM
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_MAX_LI_MODULATED_DTIM_NAME         "gMaxLIModulatedDTIM"
#define CFG_MAX_LI_MODULATED_DTIM_MIN          (1)
#define CFG_MAX_LI_MODULATED_DTIM_MAX          (10)
#define CFG_MAX_LI_MODULATED_DTIM_DEFAULT      (10)

/*
 * <ini>
 * gFWMccRtsCtsProtection - RTS-CTS protection in MCC.
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable/disable use of long duration RTS-CTS protection
 * when SAP goes off channel in MCC mode.
 *
 * Related: None.
 *
 * Supported Feature: Concurrency
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_FW_MCC_RTS_CTS_PROT_NAME           "gFWMccRtsCtsProtection"
#define CFG_FW_MCC_RTS_CTS_PROT_MIN            (0)
#define CFG_FW_MCC_RTS_CTS_PROT_MAX            (1)
#define CFG_FW_MCC_RTS_CTS_PROT_DEFAULT        (0)

/*
 * <ini>
 * gFWMccBCastProbeResponse - Broadcast Probe Response in MCC.
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable/disable use of broadcast probe response to
 * increase the detectability of SAP in MCC mode.
 *
 * Related: None.
 *
 * Supported Feature: Concurrency
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_FW_MCC_BCAST_PROB_RESP_NAME        "gFWMccBCastProbeResponse"
#define CFG_FW_MCC_BCAST_PROB_RESP_MIN         (0)
#define CFG_FW_MCC_BCAST_PROB_RESP_MAX         (1)
#define CFG_FW_MCC_BCAST_PROB_RESP_DEFAULT     (0)

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

/**
 * enum station_keepalive_method - available keepalive methods for stations
 * @HDD_STA_KEEPALIVE_NULL_DATA: null data packet
 * @HDD_STA_KEEPALIVE_GRAT_ARP: gratuitous ARP packet
 * @HDD_STA_KEEPALIVE_COUNT: number of method options available
 */
enum station_keepalive_method {
	HDD_STA_KEEPALIVE_NULL_DATA,
	HDD_STA_KEEPALIVE_GRAT_ARP,
	/* keep at the end */
	HDD_STA_KEEPALIVE_COUNT
};

/*
 * <ini>
 * gStaKeepAliveMethod - Which keepalive method to use
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini determines which keepalive method to use for station interfaces
 *       1) Use null data packets
 *       2) Use gratuitous ARP packets
 *
 * Related: gStaKeepAlivePeriod, gApKeepAlivePeriod, gGoKeepAlivePeriod
 *
 * Supported Feature: STA, Keepalive
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_STA_KEEPALIVE_METHOD_NAME              "gStaKeepAliveMethod"
#define CFG_STA_KEEPALIVE_METHOD_MIN               (HDD_STA_KEEPALIVE_NULL_DATA)
#define CFG_STA_KEEPALIVE_METHOD_MAX               (HDD_STA_KEEPALIVE_COUNT - 1)
#define CFG_STA_KEEPALIVE_METHOD_DEFAULT           (HDD_STA_KEEPALIVE_GRAT_ARP)

/* WMM configuration */
/*
 * <ini>
 * WmmIsEnabled - Enable WMM feature
 * @Min: 0
 * @Max: 2
 * @Default: 0
 *
 * This ini is used to enable/disable WMM.
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_MODE_NAME                          "WmmIsEnabled"
#define CFG_QOS_WMM_MODE_MIN                           (0)
#define CFG_QOS_WMM_MODE_MAX                           (2) /* HDD_WMM_NO_QOS */
#define CFG_QOS_WMM_MODE_DEFAULT                       (0) /* HDD_WMM_AUTO */

/*
 * <ini>
 * 80211eIsEnabled - Enable 802.11e feature
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable/disable 802.11e.
 *
 * Related: None.
 *
 * Supported Feature: 802.11e
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_80211E_ENABLED_NAME                   "80211eIsEnabled"
#define CFG_QOS_WMM_80211E_ENABLED_MIN                     (0)
#define CFG_QOS_WMM_80211E_ENABLED_MAX                     (1)
#define CFG_QOS_WMM_80211E_ENABLED_DEFAULT                 (0)

/*
 * <ini>
 * UapsdMask - To setup U-APSD mask for ACs
 * @Min: 0x00
 * @Max: 0xFF
 * @Default: 0x00
 *
 * This ini is used to setup U-APSD mask for ACs.
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_UAPSD_MASK_NAME                        "UapsdMask"
#define CFG_QOS_WMM_UAPSD_MASK_MIN                         (0x00)
#define CFG_QOS_WMM_UAPSD_MASK_MAX                         (0xFF)
#define CFG_QOS_WMM_UAPSD_MASK_DEFAULT                     (0x00)

/*
 * <ini>
 * ImplicitQosIsEnabled - Enableimplicit QOS
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable/disable implicit QOS.
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_IMPLICIT_SETUP_ENABLED_NAME             "ImplicitQosIsEnabled"
#define CFG_QOS_IMPLICIT_SETUP_ENABLED_MIN              (0)
#define CFG_QOS_IMPLICIT_SETUP_ENABLED_MAX              (1)
#define CFG_QOS_IMPLICIT_SETUP_ENABLED_DEFAULT          (0)

/*
 * <ini>
 * InfraUapsdViSrvIntv - Set Uapsd service interval for video
 * @Min: 0
 * @Max: 4294967295UL
 * @Default: 300
 *
 * This ini is used to set Uapsd service interval for video.
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_INFRA_UAPSD_VI_SRV_INTV_NAME         "InfraUapsdViSrvIntv"
#define CFG_QOS_WMM_INFRA_UAPSD_VI_SRV_INTV_MIN           (0)
#define CFG_QOS_WMM_INFRA_UAPSD_VI_SRV_INTV_MAX           (4294967295UL)
#define CFG_QOS_WMM_INFRA_UAPSD_VI_SRV_INTV_DEFAULT       (300)

/*
 * <ini>
 * InfraUapsdViSuspIntv - Set Uapsd suspension interval for video
 * @Min: 0
 * @Max: 4294967295UL
 * @Default: 2000
 *
 * This ini is used to set Uapsd suspension interval for video
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_INFRA_UAPSD_VI_SUS_INTV_NAME         "InfraUapsdViSuspIntv"
#define CFG_QOS_WMM_INFRA_UAPSD_VI_SUS_INTV_MIN           (0)
#define CFG_QOS_WMM_INFRA_UAPSD_VI_SUS_INTV_MAX           (4294967295UL)
#define CFG_QOS_WMM_INFRA_UAPSD_VI_SUS_INTV_DEFAULT       (2000)

/*
 * <ini>
 * InfraUapsdBeSrvIntv - Set Uapsd service interval for BE
 * @Min: 0
 * @Max: 4294967295UL
 * @Default: 300
 *
 * This ini is used to set Uapsd service interval for BE
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_INFRA_UAPSD_BE_SRV_INTV_NAME         "InfraUapsdBeSrvIntv"
#define CFG_QOS_WMM_INFRA_UAPSD_BE_SRV_INTV_MIN           (0)
#define CFG_QOS_WMM_INFRA_UAPSD_BE_SRV_INTV_MAX           (4294967295UL)
#define CFG_QOS_WMM_INFRA_UAPSD_BE_SRV_INTV_DEFAULT       (300)

/*
 * <ini>
 * InfraUapsdBeSuspIntv - Set Uapsd suspension interval for BE
 * @Min: 0
 * @Max: 4294967295UL
 * @Default: 2000
 *
 * This ini is used to set Uapsd suspension interval for BE
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_INFRA_UAPSD_BE_SUS_INTV_NAME         "InfraUapsdBeSuspIntv"
#define CFG_QOS_WMM_INFRA_UAPSD_BE_SUS_INTV_MIN           (0)
#define CFG_QOS_WMM_INFRA_UAPSD_BE_SUS_INTV_MAX           (4294967295UL)
#define CFG_QOS_WMM_INFRA_UAPSD_BE_SUS_INTV_DEFAULT       (2000)

/*
 * <ini>
 * InfraUapsdBkSrvIntv - Set Uapsd service interval for BK
 * @Min: 0
 * @Max: 4294967295UL
 * @Default: 300
 *
 * This ini is used to set Uapsd service interval for BK
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_INFRA_UAPSD_BK_SRV_INTV_NAME         "InfraUapsdBkSrvIntv"
#define CFG_QOS_WMM_INFRA_UAPSD_BK_SRV_INTV_MIN           (0)
#define CFG_QOS_WMM_INFRA_UAPSD_BK_SRV_INTV_MAX           (4294967295UL)
#define CFG_QOS_WMM_INFRA_UAPSD_BK_SRV_INTV_DEFAULT       (300)

/*
 * <ini>
 * InfraUapsdBkSuspIntv - Set Uapsd suspension interval for BK
 * @Min: 0
 * @Max: 4294967295UL
 * @Default: 2000
 *
 * This ini is used to set Uapsd suspension interval for BK
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_INFRA_UAPSD_BK_SUS_INTV_NAME         "InfraUapsdBkSuspIntv"
#define CFG_QOS_WMM_INFRA_UAPSD_BK_SUS_INTV_MIN           (0)
#define CFG_QOS_WMM_INFRA_UAPSD_BK_SUS_INTV_MAX           (4294967295UL)
#define CFG_QOS_WMM_INFRA_UAPSD_BK_SUS_INTV_DEFAULT       (2000)

 /* default TSPEC parameters for AC_VI */
/*
 * <ini>
 * InfraDirAcVi - Set TSPEC direction for VI
 * @Min: 0
 * @Max: 3
 * @Default: 3
 *
 * This ini is used to set TSPEC direction for VI
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_INFRA_DIR_AC_VI_NAME   "InfraDirAcVi"
#define CFG_QOS_WMM_INFRA_DIR_AC_VI_MIN     (0)
#define CFG_QOS_WMM_INFRA_DIR_AC_VI_MAX     (3)
#define CFG_QOS_WMM_INFRA_DIR_AC_VI_DEFAULT (3) /*WLAN_QCT_CUST_WMM_TSDIR_BOTH*/

/*
 * <ini>
 * InfraNomMsduSizeAcVi - Set normal MSDU size for VI
 * @Min: 0x0
 * @Max: 0xFFFF
 * @Default: 0x85DC
 *
 * This ini is used to set normal MSDU size for VI
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VI_NAME      "InfraNomMsduSizeAcVi"
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VI_MIN        (0x0)
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VI_MAX        (0xFFFF)
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VI_DEFAULT    (0x85DC)

/*
 * <ini>
 * InfraMeanDataRateAcVi - Set mean data rate for VI
 * @Min: 0x0
 * @Max: 0xFFFFFFFF
 * @Default: 0x57E40
 *
 * This ini is used to set mean data rate for VI
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VI_NAME    "InfraMeanDataRateAcVi"
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VI_MIN      (0x0)
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VI_MAX      (0xFFFFFFFF)
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VI_DEFAULT  (0x57E40)

/*
 * <ini>
 * iInfraMinPhyRateAcVi - Set min PHY rate for VI
 * @Min: 0x0
 * @Max: 0xFFFFFFFF
 * @Default: 0x5B8D80
 *
 * This ini is used to set min PHY rate for VI
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VI_NAME        "InfraMinPhyRateAcVi"
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VI_MIN          (0x0)
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VI_MAX          (0xFFFFFFFF)
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VI_DEFAULT      (0x5B8D80)

/*
 * <ini>
 * InfraSbaAcVi - Set surplus bandwidth allowance for VI
 * @Min: 0x2001
 * @Max: 0xFFFF
 * @Default: 0x2001
 *
 * This ini is used to set surplus bandwidth allowance for VI
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
 #define CFG_QOS_WMM_INFRA_SBA_AC_VI_NAME                   "InfraSbaAcVi"
 #define CFG_QOS_WMM_INFRA_SBA_AC_VI_MIN                     (0x2001)
 #define CFG_QOS_WMM_INFRA_SBA_AC_VI_MAX                     (0xFFFF)
 #define CFG_QOS_WMM_INFRA_SBA_AC_VI_DEFAULT                 (0x2001)

 /* default TSPEC parameters for AC_BE*/
/*
 * <ini>
 * InfraDirAcBe - Set TSPEC direction for BE
 * @Min: 0
 * @Max: 3
 * @Default: 3
 *
 * This ini is used to set TSPEC direction for BE
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_INFRA_DIR_AC_BE_NAME   "InfraDirAcBe"
#define CFG_QOS_WMM_INFRA_DIR_AC_BE_MIN     (0)
#define CFG_QOS_WMM_INFRA_DIR_AC_BE_MAX     (3)
#define CFG_QOS_WMM_INFRA_DIR_AC_BE_DEFAULT (3) /*WLAN_QCT_CUST_WMM_TSDIR_BOTH*/

/*
 * <ini>
 * InfraNomMsduSizeAcBe - Set normal MSDU size for BE
 * @Min: 0x0
 * @Max: 0xFFFF
 * @Default: 0x85DC
 *
 * This ini is used to set normal MSDU size for BE
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BE_NAME      "InfraNomMsduSizeAcBe"
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BE_MIN        (0x0)
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BE_MAX        (0xFFFF)
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BE_DEFAULT    (0x85DC)

/*
 * <ini>
 * InfraMeanDataRateAcBe - Set mean data rate for BE
 * @Min: 0x0
 * @Max: 0xFFFFFFFF
 * @Default: 0x493E0
 *
 * This ini is used to set mean data rate for BE
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BE_NAME    "InfraMeanDataRateAcBe"
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BE_MIN      (0x0)
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BE_MAX      (0xFFFFFFFF)
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BE_DEFAULT  (0x493E0)

/*
 * <ini>
 * InfraMinPhyRateAcBe - Set min PHY rate for BE
 * @Min: 0x0
 * @Max: 0xFFFFFFFF
 * @Default: 0x5B8D80
 *
 * This ini is used to set min PHY rate for BE
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BE_NAME        "InfraMinPhyRateAcBe"
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BE_MIN          (0x0)
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BE_MAX          (0xFFFFFFFF)
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BE_DEFAULT      (0x5B8D80)

/*
 * <ini>
 * InfraSbaAcBe - Set surplus bandwidth allowance for BE
 * @Min: 0x2001
 * @Max: 0xFFFF
 * @Default: 0x2001
 *
 * This ini is used to set surplus bandwidth allowance for BE
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
 #define CFG_QOS_WMM_INFRA_SBA_AC_BE_NAME                   "InfraSbaAcBe"
 #define CFG_QOS_WMM_INFRA_SBA_AC_BE_MIN                     (0x2001)
 #define CFG_QOS_WMM_INFRA_SBA_AC_BE_MAX                     (0xFFFF)
 #define CFG_QOS_WMM_INFRA_SBA_AC_BE_DEFAULT                 (0x2001)

 /* default TSPEC parameters for AC_Bk*/
/*
 * <ini>
 * InfraDirAcBk - Set TSPEC direction for BK
 * @Min: 0
 * @Max: 3
 * @Default: 3
 *
 * This ini is used to set TSPEC direction for BK
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_INFRA_DIR_AC_BK_NAME   "InfraDirAcBk"
#define CFG_QOS_WMM_INFRA_DIR_AC_BK_MIN     (0)
#define CFG_QOS_WMM_INFRA_DIR_AC_BK_MAX     (3)
#define CFG_QOS_WMM_INFRA_DIR_AC_BK_DEFAULT (3) /*WLAN_QCT_CUST_WMM_TSDIR_BOTH*/

/*
 * <ini>
 * InfraNomMsduSizeAcBk - Set normal MSDU size for BK
 * @Min: 0x0
 * @Max: 0xFFFF
 * @Default: 0x85DC
 *
 * This ini is used to set normal MSDU size for BK
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BK_NAME      "InfraNomMsduSizeAcBk"
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BK_MIN        (0x0)
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BK_MAX        (0xFFFF)
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BK_DEFAULT    (0x85DC)

/*
 * <ini>
 * InfraMeanDataRateAcBk - Set mean data rate for BK
 * @Min: 0x0
 * @Max: 0xFFFFFFFF
 * @Default: 0x493E0
 *
 * This ini is used to set mean data rate for BK
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BK_NAME    "InfraMeanDataRateAcBk"
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BK_MIN      (0x0)
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BK_MAX      (0xFFFFFFFF)
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BK_DEFAULT  (0x493E0)

/*
 * <ini>
 * InfraMinPhyRateAcBke - Set min PHY rate for BK
 * @Min: 0x0
 * @Max: 0xFFFFFFFF
 * @Default: 0x5B8D80
 *
 * This ini is used to set min PHY rate for BK
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BK_NAME        "InfraMinPhyRateAcBk"
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BK_MIN          (0x0)
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BK_MAX          (0xFFFFFFFF)
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BK_DEFAULT      (0x5B8D80)

/*
 * <ini>
 * InfraSbaAcBk - Set surplus bandwidth allowance for BK
 * @Min: 0x2001
 * @Max: 0xFFFF
 * @Default: 0x2001
 *
 * This ini is used to set surplus bandwidth allowance for BK
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
 #define CFG_QOS_WMM_INFRA_SBA_AC_BK_NAME                   "InfraSbaAcBk"
 #define CFG_QOS_WMM_INFRA_SBA_AC_BK_MIN                     (0x2001)
 #define CFG_QOS_WMM_INFRA_SBA_AC_BK_MAX                     (0xFFFF)
 #define CFG_QOS_WMM_INFRA_SBA_AC_BK_DEFAULT                 (0x2001)

/*
 * <ini>
 * burstSizeDefinition - Set TS burst size
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to set TS burst size
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_BURST_SIZE_DEFN_NAME                "burstSizeDefinition"
#define CFG_QOS_WMM_BURST_SIZE_DEFN_MIN                  (0)
#define CFG_QOS_WMM_BURST_SIZE_DEFN_MAX                  (1)
#define CFG_QOS_WMM_BURST_SIZE_DEFN_DEFAULT              (0)

/*
 * <ini>
 * tsInfoAckPolicy - Set TS ack policy
 * @Min: 0x00
 * @Max: 0x01
 * @Default: 0x00
 *
 * This ini is used to set TS ack policy
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_TS_INFO_ACK_POLICY_NAME              "tsInfoAckPolicy"
#define CFG_QOS_WMM_TS_INFO_ACK_POLICY_MIN                (0x00)
#define CFG_QOS_WMM_TS_INFO_ACK_POLICY_MAX                (0x01)
#define CFG_QOS_WMM_TS_INFO_ACK_POLICY_DEFAULT            (0x00)

/*
 * <ini>
 * gAddTSWhenACMIsOff - Set ACM value for AC
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to set ACM value for AC
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_ADDTS_WHEN_ACM_IS_OFF_NAME               "gAddTSWhenACMIsOff"
#define CFG_QOS_ADDTS_WHEN_ACM_IS_OFF_MIN                (0)
/* Send AddTs even when ACM is not set for the AC */
#define CFG_QOS_ADDTS_WHEN_ACM_IS_OFF_MAX                (1)
#define CFG_QOS_ADDTS_WHEN_ACM_IS_OFF_DEFAULT            (0)

#ifdef FEATURE_WLAN_ESE
#define CFG_QOS_WMM_INFRA_INACTIVITY_INTERVAL_NAME    "InfraInactivityInterval"
#define CFG_QOS_WMM_INFRA_INACTIVITY_INTERVAL_MIN      (0)
#define CFG_QOS_WMM_INFRA_INACTIVITY_INTERVAL_MAX      (4294967295UL)
#define CFG_QOS_WMM_INFRA_INACTIVITY_INTERVAL_DEFAULT  (0) /* disabled */

#define CFG_ESE_FEATURE_ENABLED_NAME                       "EseEnabled"
#define CFG_ESE_FEATURE_ENABLED_MIN                         (0)
#define CFG_ESE_FEATURE_ENABLED_MAX                         (1)
#define CFG_ESE_FEATURE_ENABLED_DEFAULT                     (0) /* disabled */
#endif /* FEATURE_WLAN_ESE */

/*
 * <ini>
 * MAWCEnabled - Enable/Disable Motion Aided Wireless Connectivity Global
 * @Min: 0 - Disabled
 * @Max: 1 - Enabled
 * @Default: 0
 *
 * This ini is used to controls the MAWC feature globally.
 * MAWC is Motion Aided Wireless Connectivity.
 *
 * Related: mawc_roam_enabled.
 *
 * Supported Feature: Roaming and PNO/NLO
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_LFR_MAWC_FEATURE_ENABLED_NAME                   "MAWCEnabled"
#define CFG_LFR_MAWC_FEATURE_ENABLED_MIN                    (0)
#define CFG_LFR_MAWC_FEATURE_ENABLED_MAX                    (1)
#define CFG_LFR_MAWC_FEATURE_ENABLED_DEFAULT                (0)

/*This parameter is used to set Wireless Extended Security Mode.*/
#define CFG_ENABLE_WES_MODE_NAME                            "gWESModeEnabled"
#define CFG_ENABLE_WES_MODE_NAME_MIN                        (0)
#define CFG_ENABLE_WES_MODE_NAME_MAX                        (1)
#define CFG_ENABLE_WES_MODE_NAME_DEFAULT                    (0)

#define CFG_TL_DELAYED_TRGR_FRM_INT_NAME                   "DelayedTriggerFrmInt"
#define CFG_TL_DELAYED_TRGR_FRM_INT_MIN                     1
#define CFG_TL_DELAYED_TRGR_FRM_INT_MAX                     (4294967295UL)
#define CFG_TL_DELAYED_TRGR_FRM_INT_DEFAULT                 3000

/*
 * <ini>
 * gRrmEnable - Enable/Disable RRM
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to controls the capabilities (11 k) included
 * in the capabilities field.
 *
 * Related: None.
 *
 * Supported Feature: 11k
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_RRM_ENABLE_NAME                              "gRrmEnable"
#define CFG_RRM_ENABLE_MIN                               (0)
#define CFG_RRM_ENABLE_MAX                               (1)
#define CFG_RRM_ENABLE_DEFAULT                           (0)

/*
 * <ini>
 * gRrmRandnIntvl - Randomization interval
 * @Min: 10
 * @Max: 100
 * @Default: 100
 *
 * This ini is used to set randomization interval which is used to start a timer
 * of a random value within randomization interval. Next RRM Scan request
 * will be issued after the expiry of this random interval.
 *
 * Related: None.
 *
 * Supported Feature: 11k
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_RRM_MEAS_RANDOMIZATION_INTVL_NAME            "gRrmRandnIntvl"
#define CFG_RRM_MEAS_RANDOMIZATION_INTVL_MIN             (10)
#define CFG_RRM_MEAS_RANDOMIZATION_INTVL_MAX             (100)
#define CFG_RRM_MEAS_RANDOMIZATION_INTVL_DEFAULT         (100)

/*
 * <ini>
 * rm_capability - Configure RM enabled capabilities IE
 * @Default: 73,10,91,00,04
 *
 * This ini is used to configure RM enabled capabilities IE.
 * Using this INI, we can set/unset any of the bits in 5 bytes
 * (last 4bytes are reserved). Bit details are updated as per
 * Draft version of 11mc spec. (Draft P802.11REVmc_D4.2)
 *
 * Bitwise details are defined as bit mask in rrm_global.h
 * Comma is used as a separator for each byte.
 *
 * Related: None.
 *
 * Supported Feature: 11k
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_RM_CAPABILITY_NAME            "rm_capability"
#define CFG_RM_CAPABILITY_DEFAULT         "73,10,91,00,04"

#define CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_NAME      "gNeighborLookupThreshold"
#define CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_MIN       (10)
#define CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_MAX       (120)
#define CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_DEFAULT   (78)

#define CFG_DELAY_BEFORE_VDEV_STOP_NAME              "gDelayBeforeVdevStop"
#define CFG_DELAY_BEFORE_VDEV_STOP_MIN               (2)
#define CFG_DELAY_BEFORE_VDEV_STOP_MAX               (200)
#define CFG_DELAY_BEFORE_VDEV_STOP_DEFAULT           (20)

#define CFG_11R_NEIGHBOR_REQ_MAX_TRIES_NAME           "gMaxNeighborReqTries"
#define CFG_11R_NEIGHBOR_REQ_MAX_TRIES_MIN            (1)
#define CFG_11R_NEIGHBOR_REQ_MAX_TRIES_MAX            (4)
#define CFG_11R_NEIGHBOR_REQ_MAX_TRIES_DEFAULT        (3)

#define CFG_ROAM_BMISS_FIRST_BCNT_NAME                  "gRoamBmissFirstBcnt"
#define CFG_ROAM_BMISS_FIRST_BCNT_MIN                   (5)
#define CFG_ROAM_BMISS_FIRST_BCNT_MAX                   (100)
#define CFG_ROAM_BMISS_FIRST_BCNT_DEFAULT               (10)

#define CFG_ROAM_BMISS_FINAL_BCNT_NAME                  "gRoamBmissFinalBcnt"
#define CFG_ROAM_BMISS_FINAL_BCNT_MIN                   (5)
#define CFG_ROAM_BMISS_FINAL_BCNT_MAX                   (100)
#define CFG_ROAM_BMISS_FINAL_BCNT_DEFAULT               (20)

#define CFG_ROAM_BEACON_RSSI_WEIGHT_NAME                "gRoamBeaconRssiWeight"
#define CFG_ROAM_BEACON_RSSI_WEIGHT_MIN                 (0)
#define CFG_ROAM_BEACON_RSSI_WEIGHT_MAX                 (16)
#define CFG_ROAM_BEACON_RSSI_WEIGHT_DEFAULT             (14)

#define CFG_AP_DATA_AVAIL_POLL_PERIOD_NAME      "gApDataAvailPollInterval"
#define CFG_AP_DATA_AVAIL_POLL_PERIOD_MIN       (WNI_CFG_AP_DATA_AVAIL_POLL_PERIOD_STAMIN)
#define CFG_AP_DATA_AVAIL_POLL_PERIOD_MAX       (WNI_CFG_AP_DATA_AVAIL_POLL_PERIOD_STAMAX)
#define CFG_AP_DATA_AVAIL_POLL_PERIOD_DEFAULT   (WNI_CFG_AP_DATA_AVAIL_POLL_PERIOD_STADEF)

/*
 * <ini>
 * gHwFilterMode - configure hardware filter for DTIM mode
 * @Min: 0
 * @Max: 3
 * @Default: 1
 *
 * The hardware filter is only effective in DTIM mode. Use this configuration
 * to blanket drop broadcast/multicast packets at the hardware level, without
 * waking up the firmware
 *
 * Takes a bitmap of frame types to drop
 * @E.g.
 *	# disable feature
 *	gHwFilterMode=0
 *	# drop all broadcast frames, except ARP (default)
 *	gHwFilterMode=1
 *	# drop all multicast frames, except ICMPv6
 *	gHwFilterMode=2
 *	# drop all broadcast and multicast frames, except ARP and ICMPv6
 *	gHwFilterMode=3
 *
 * Related: N/A
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_HW_FILTER_MODE_BITMAP_NAME	"gHwFilterMode"
#define CFG_HW_FILTER_MODE_BITMAP_MIN		(0)
#define CFG_HW_FILTER_MODE_BITMAP_MAX		(3)
#define CFG_HW_FILTER_MODE_BITMAP_DEFAULT	(1)

#define CFG_ENABLE_BYPASS_11D_NAME                 "gEnableBypass11d"
#define CFG_ENABLE_BYPASS_11D_MIN                  (0)
#define CFG_ENABLE_BYPASS_11D_MAX                  (1)
#define CFG_ENABLE_BYPASS_11D_DEFAULT              (1)

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

/*
 * <ini>
 * gEnableDumpCollect - It will use for collect the dumps
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to set collect default dump
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_ENABLE_RAMDUMP_COLLECTION              "gEnableDumpCollect"
#define CFG_ENABLE_RAMDUMP_COLLECTION_MIN          (0)
#define CFG_ENABLE_RAMDUMP_COLLECTION_MAX          (1)
#define CFG_ENABLE_RAMDUMP_COLLECTION_DEFAULT      (1)

enum hdd_link_speed_rpt_type {
	eHDD_LINK_SPEED_REPORT_ACTUAL = 0,
	eHDD_LINK_SPEED_REPORT_MAX = 1,
	eHDD_LINK_SPEED_REPORT_MAX_SCALED = 2,
};

/*
 * <ini>
 * enable_bt_chain_separation - Enables/disables bt /wlan chainmask assignment
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini disables/enables chainmask setting on 2x2, mainly used for ROME
 * BT/WLAN chainmask assignment.
 *
 * 0, Disable
 * 1, Enable
 *
 * Related: NA
 *
 * Supported Feature: 11n/11ac
 *
 * Usage: External
 *
 * </ini>
 */

#define CFG_ENABLE_BT_CHAIN_SEPARATION         "enableBTChainSeparation"
#define CFG_ENABLE_BT_CHAIN_SEPARATION_MIN     (0)
#define CFG_ENABLE_BT_CHAIN_SEPARATION_MAX     (1)
#define CFG_ENABLE_BT_CHAIN_SEPARATION_DEFAULT (0)

/*
 * <ini>
 * gVdevTypeNss_2g - set Number of streams per VDEV for 2G band.
 * @Min: 0x5555
 * @Max: 0xAAAA
 * @Default: 0xAAAA
 *
 * This ini is  used to set set Number of streams per VDEV for 2G band
 *
 * These Nss parameters will have 32-bit configuration value, 2 bits are
 * allocated for each vdev.
 * Valid values are:
 * Min value – 0x5555
 * Max value – 0xAAAA
 * Default value will be 0xAAAA for both the parameters.
 * Value 0x5555 will configure all vdevs in 1x1 mode in 2.4G band.
 * Value 0xAAAA will configure all vdevs in 2x2 mode in 2.4G band.
 *
 * The max value is defined based on the valid max Nss of the vdev, the valid
 * values for each vdev 2-bits are 0x1 and 0x2. 0x3 and 0x0 are not valid vdev
 * Nss values.
 *
 * NSS cfg bit definition.
 * STA          BIT[0:1]
 * SAP          BIT[2:3]
 * P2P_GO       BIT[4:5]
 * P2P_CLIENT   BIT[6:7]
 * IBSS         BIT[8:9]
 * TDLS         BIT[10:11]
 * P2P_DEVICE   BIT[12:13]
 * OCB          BIT[14:15]
 *
 * Related: NA
 *
 * Supported Feature: Antenna Sharing
 *
 * Usage: External
 *
 * </ini>
 */

#define CFG_VDEV_TYPE_NSS_2G         "gVdevTypeNss_2g"
#define CFG_VDEV_TYPE_NSS_2G_MIN     (0x5555)
#define CFG_VDEV_TYPE_NSS_2G_MAX     (0xAAAA)
#define CFG_VDEV_TYPE_NSS_2G_DEFAULT (0xAAAA)

/*
 * <ini>
 * gVdevTypeNss_5g - set Number of streams per VDEV for 5G band.
 * @Min: 0x5555
 * @Max: 0xAAAA
 * @Default: 0xAAAA
 *
 * This ini is  used to set set Number of streams per VDEV for 2G band
 *
 * These Nss parameters will have 32-bit configuration value, 2 bits are
 * allocated for each vdev.
 * Valid values are:
 * Min value – 0x5555
 * Max value – 0xAAAA
 * Default value will be 0xAAAA for both the parameters.
 * Value 0x5555 will configure all vdevs in 1x1 mode in 5 band.
 * Value 0xAAAA will configure all vdevs in 2x2 mode in 5 band.
 *
 * The max value is defined based on the valid max Nss of the vdev, the valid
 * values for each vdev 2-bits are 0x1 and 0x2. 0x3 and 0x0 are not valid vdev
 * Nss values.
 *
 * NSS cfg bit definition.
 * STA          BIT[0:1]
 * SAP          BIT[2:3]
 * P2P_GO       BIT[4:5]
 * P2P_CLIENT   BIT[6:7]
 * IBSS         BIT[8:9]
 * TDLS         BIT[10:11]
 * P2P_DEVICE   BIT[12:13]
 * OCB          BIT[14:15]
 *
 * Related: NA
 *
 * Supported Feature: Antenna Sharing
 *
 * Usage: External
 *
 * </ini>
 */

#define CFG_VDEV_TYPE_NSS_5G         "gVdevTypeNss_5g"
#define CFG_VDEV_TYPE_NSS_5G_MIN     (0x5555)
#define CFG_VDEV_TYPE_NSS_5G_MAX     (0xAAAA)
#define CFG_VDEV_TYPE_NSS_5G_DEFAULT (0xAAAA)

#define CFG_HT_ENABLE_SMPS_CAP_FEATURE          "gEnableHtSMPS"
#define CFG_HT_ENABLE_SMPS_CAP_FEATURE_MIN      (0)
#define CFG_HT_ENABLE_SMPS_CAP_FEATURE_MAX      (1)
#define CFG_HT_ENABLE_SMPS_CAP_FEATURE_DEFAULT  (0)

#define CFG_HT_SMPS_CAP_FEATURE                 "gHtSMPS"
#define CFG_HT_SMPS_CAP_FEATURE_MIN             (0)
#define CFG_HT_SMPS_CAP_FEATURE_MAX             (3)
#define CFG_HT_SMPS_CAP_FEATURE_DEFAULT         (3)

/*
 * <ini>
 * gDisableDFSChSwitch - Disable channel switch if radar is found
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to disable channel switch if radar is found
 * on that channel.
 * Related: NA.
 *
 * Supported Feature: DFS
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_DISABLE_DFS_CH_SWITCH                 "gDisableDFSChSwitch"
#define CFG_DISABLE_DFS_CH_SWITCH_MIN             (0)
#define CFG_DISABLE_DFS_CH_SWITCH_MAX             (1)
#define CFG_DISABLE_DFS_CH_SWITCH_DEFAULT         (0)

/*
 * <ini>
 * gEnableDFSMasterCap - Enable DFS master capability
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable/disable the DFS master capability.
 * Disabling it will cause driver to not advertise the spectrum
 * management capability
 * Related: NA.
 *
 * Supported Feature: DFS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_ENABLE_DFS_MASTER_CAPABILITY               "gEnableDFSMasterCap"
#define CFG_ENABLE_DFS_MASTER_CAPABILITY_MIN           (0)
#define CFG_ENABLE_DFS_MASTER_CAPABILITY_MAX           (1)
#define CFG_ENABLE_DFS_MASTER_CAPABILITY_DEFAULT       (0)

/*
 * <ini>
 * gSapPreferredChanLocation - Restrict channel switches between ondoor and
 * outdoor.
 * @Min: 0
 * @Max: 2
 * @Default: 0
 *
 * This ini is used for restricting channel switches between Indoor and outdoor
 * channels after radar detection.
 * 0- No preferred channel location
 * 1- Use indoor channels only
 * 2- Use outdoor channels only
 * Related: NA.
 *
 * Supported Feature: DFS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_SAP_PREFERRED_CHANNEL_LOCATION          "gSapPreferredChanLocation"
#define CFG_SAP_PREFERRED_CHANNEL_LOCATION_MIN      (0)
#define CFG_SAP_PREFERRED_CHANNEL_LOCATION_MAX      (2)
#define CFG_SAP_PREFERRED_CHANNEL_LOCATION_DEFAULT  (0)

/*
 * <ini>
 * gDisableDfsJapanW53 - Block W53 channels in random channel selection
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to block W53 Japan channel in random channel selection
 * Related: NA.
 *
 * Supported Feature: DFS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_DISABLE_DFS_JAPAN_W53                      "gDisableDfsJapanW53"
#define CFG_DISABLE_DFS_JAPAN_W53_MIN                  (0)
#define CFG_DISABLE_DFS_JAPAN_W53_MAX                  (1)
#define CFG_DISABLE_DFS_JAPAN_W53_DEFAULT              (0)

/*
 * <ini>
 * gDisableDfsJapanW53 - Enable dfs phyerror filtering offload in FW
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to to enable dfs phyerror filtering offload to firmware
 * Enabling it will cause basic phy error to be discarding in firmware.
 * Related: NA.
 *
 * Supported Feature: DFS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_ENABLE_DFS_PHYERR_FILTEROFFLOAD_NAME       "dfsPhyerrFilterOffload"
#define CFG_ENABLE_DFS_PHYERR_FILTEROFFLOAD_MIN        (0)
#define CFG_ENABLE_DFS_PHYERR_FILTEROFFLOAD_MAX        (1)
#define CFG_ENABLE_DFS_PHYERR_FILTEROFFLOAD_DEFAULT    (0)

#define CFG_REPORT_MAX_LINK_SPEED                  "gReportMaxLinkSpeed"
#define CFG_REPORT_MAX_LINK_SPEED_MIN              (eHDD_LINK_SPEED_REPORT_ACTUAL)
#define CFG_REPORT_MAX_LINK_SPEED_MAX              (eHDD_LINK_SPEED_REPORT_MAX_SCALED)
#define CFG_REPORT_MAX_LINK_SPEED_DEFAULT          (eHDD_LINK_SPEED_REPORT_ACTUAL)

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

/*
 * <ini>
 * gEnableSSR - Enable/Disable SSR
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable/disable System Self Recovery at the times of
 * System crash or fatal errors
 * gEnableSSR = 0 Disabled
 * gEnableSSR = 1 wlan shutdown and re-init happens
 *
 * Related: None
 *
 * Supported Feature: SSR
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ENABLE_SSR                      "gEnableSSR"
#define CFG_ENABLE_SSR_MIN                  (0)
#define CFG_ENABLE_SSR_MAX                  (1)
#define CFG_ENABLE_SSR_DEFAULT              (1)

/**
 * <ini>
 * gEnableDataStallDetection - Enable/Disable Data stall detection
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable/disable data stall detection
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_ENABLE_DATA_STALL_DETECTION           "gEnableDataStallDetection"
#define CFG_ENABLE_DATA_STALL_DETECTION_MIN       (0)
#define CFG_ENABLE_DATA_STALL_DETECTION_MAX       (1)
#define CFG_ENABLE_DATA_STALL_DETECTION_DEFAULT   (1)

/*
 * <ini>
 * gEnableOverLapCh - Enables Overlap Channel. If set, allow overlapping
 *                    channels to be selected for the SoftAP
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to set Overlap Channel
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_ENABLE_OVERLAP_CH               "gEnableOverLapCh"
#define CFG_ENABLE_OVERLAP_CH_MIN           (0)
#define CFG_ENABLE_OVERLAP_CH_MAX           (1)
#define CFG_ENABLE_OVERLAP_CH_DEFAULT       (0)

#define CFG_ENABLE_MEMORY_DEEP_SLEEP          "gEnableMemDeepSleep"
#define CFG_ENABLE_MEMORY_DEEP_SLEEP_MIN      (0)
#define CFG_ENABLE_MEMORY_DEEP_SLEEP_MAX      (1)
#define CFG_ENABLE_MEMORY_DEEP_SLEEP_DEFAULT  (1)

/*
 * <ini>
 *
 * gEnableCckTxFirOverride - Enable/disable CCK TxFIR Override
 * @Min: 0 (disabled)
 * @Max: 1 (enabled)
 * @Default: 0 (disabled)
 *
 * When operating in an 802.11b mode, this configuration item forces a 2x2 radio
 * configuration into 1x for Tx and 2x for Rx (ie 1x2) for regulatory compliance
 * reasons.
 *
 * Related: enable2x2
 *
 * Supported Feature: 802.11b, 2x2
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_ENABLE_CCK_TX_FIR_OVERRIDE_NAME     "gEnableCckTxFirOverride"
#define CFG_ENABLE_CCK_TX_FIR_OVERRIDE_MIN      (0)
#define CFG_ENABLE_CCK_TX_FIR_OVERRIDE_MAX      (1)
#define CFG_ENABLE_CCK_TX_FIR_OVERRIDE_DEFAULT  (0)

#ifndef REMOVE_PKT_LOG
#define CFG_ENABLE_PACKET_LOG            "gEnablePacketLog"
#define CFG_ENABLE_PACKET_LOG_MIN        (0)
#define CFG_ENABLE_PACKET_LOG_MAX        (1)
#ifdef FEATURE_PKTLOG
#define CFG_ENABLE_PACKET_LOG_DEFAULT    (1)
#else
#define CFG_ENABLE_PACKET_LOG_DEFAULT    (0)
#endif
#endif

/* For valid values of log levels check enum DBGLOG_LOG_LVL and
 * for valid values of module ids check enum WLAN_MODULE_ID.
 */
#define CFG_ENABLE_FW_MODULE_LOG_LEVEL    "gFwDebugModuleLoglevel"
#define CFG_ENABLE_FW_MODULE_LOG_DEFAULT  "2,1,3,1,5,1,9,1,13,1,14,1,18,1,19,1,26,1,28,1,29,1,31,1,36,1,38,1,46,1,47,1,50,1,52,1,53,1,56,1,60,1,61,1,4,1"

/*
 * <ini>
 * gEnableConcurrentSTA - This will control the creation of concurrent STA
 * interface
 * @Default: NULL
 *
 * This ini is used for providing control to create a concurrent STA session
 * along with the creation of wlan0 and p2p0. The name of the interface is
 * specified as the parameter
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_ENABLE_CONCURRENT_STA           "gEnableConcurrentSTA"
#define CFG_ENABLE_CONCURRENT_STA_DEFAULT   ""

/*
 * QDF Trace Enable Control
 * Notes:
 *  the MIN/MAX/DEFAULT values apply for all modules
 *  the DEFAULT value is outside the valid range.  if the DEFAULT
 *    value is not overridden, then no change will be made to the
 *    "built in" default values compiled into the code
 *  values are a bitmap indicating which log levels are to enabled
 *    (must match order of qdf_trace_level enumerations)
 *    00000001  FATAL
 *    00000010  ERROR
 *    00000100  WARN
 *    00001000  INFO
 *    00010000  INFO HIGH
 *    00100000  INFO MED
 *    01000000  INFO LOW
 *    10000000  DEBUG
 *
 *  hence a value of 0xFF would set all bits (enable all logs)
 */

#define CFG_QDF_TRACE_ENABLE_WDI_NAME     "qdf_trace_enable_wdi"
#define CFG_QDF_TRACE_ENABLE_HDD_NAME     "qdf_trace_enable_hdd"
#define CFG_QDF_TRACE_ENABLE_SME_NAME     "qdf_trace_enable_sme"
#define CFG_QDF_TRACE_ENABLE_PE_NAME      "qdf_trace_enable_pe"
#define CFG_QDF_TRACE_ENABLE_PMC_NAME     "qdf_trace_enable_pmc"
#define CFG_QDF_TRACE_ENABLE_WMA_NAME     "qdf_trace_enable_wma"
#define CFG_QDF_TRACE_ENABLE_SYS_NAME     "qdf_trace_enable_sys"
#define CFG_QDF_TRACE_ENABLE_QDF_NAME     "qdf_trace_enable_qdf"
#define CFG_QDF_TRACE_ENABLE_SAP_NAME     "qdf_trace_enable_sap"
#define CFG_QDF_TRACE_ENABLE_HDD_SAP_NAME "qdf_trace_enable_hdd_sap"
#define CFG_QDF_TRACE_ENABLE_BMI_NAME     "qdf_trace_enable_bmi"
#define CFG_QDF_TRACE_ENABLE_CFG_NAME     "qdf_trace_enable_cfg"
#define CFG_QDF_TRACE_ENABLE_EPPING       "qdf_trace_enable_epping"
#define CFG_QDF_TRACE_ENABLE_QDF_DEVICES  "qdf_trace_enable_qdf_devices"
#define CFG_QDF_TRACE_ENABLE_TXRX_NAME    "qdf_trace_enable_txrx"
#define CFG_QDF_TRACE_ENABLE_DP_NAME      "qdf_trace_enable_dp"
#define CFG_QDF_TRACE_ENABLE_HTC_NAME     "qdf_trace_enable_htc"
#define CFG_QDF_TRACE_ENABLE_HIF_NAME     "qdf_trace_enable_hif"
#define CFG_CDR_TRACE_ENABLE_HDD_SAP_DATA_NAME   "qdf_trace_enable_hdd_sap_data"
#define CFG_QDF_TRACE_ENABLE_HDD_DATA_NAME       "qdf_trace_enable_hdd_data"
#define CFG_QDF_TRACE_ENABLE_WIFI_POS     "qdf_trace_enable_wifi_pos"
#define CFG_QDF_TRACE_ENABLE_NAN          "qdf_trace_enable_nan"
#define CFG_QDF_TRACE_ENABLE_REGULATORY   "qdf_trace_enable_regulatory"
#define CFG_QDF_TRACE_ENABLE_CP_STATS     "qdf_trace_enable_cp_stats"

#define CFG_QDF_TRACE_ENABLE_MIN          (0)
#define CFG_QDF_TRACE_ENABLE_MAX          (0xff)
#define CFG_QDF_TRACE_ENABLE_DEFAULT      (0xffff)
/* disable debug logs for DP by default */
#define CFG_QDF_TRACE_ENABLE_DP_DEFAULT   (0x7f)

#ifdef ENABLE_MTRACE_LOG
/*
 * Enable MTRACE for all modules
 */
#define CFG_ENABLE_MTRACE            "enable_mtrace"
#define CFG_ENABLE_MTRACE_MIN        (0)
#define CFG_ENABLE_MTRACE_MAX        (1)
#define CFG_ENABLE_MTRACE_DEFAULT    (0)
#endif

#define HDD_MCASTBCASTFILTER_FILTER_NONE                       0x00
#define HDD_MCASTBCASTFILTER_FILTER_ALL_MULTICAST              0x01
#define HDD_MCASTBCASTFILTER_FILTER_ALL_BROADCAST              0x02
#define HDD_MCASTBCASTFILTER_FILTER_ALL_MULTICAST_BROADCAST    0x03
#define HDD_MULTICAST_FILTER_LIST                              0x04
#define HDD_MULTICAST_FILTER_LIST_CLEAR                        0x05

/*
 * <ini>
 * gConfigVCmodeBitmap - Bitmap for operating voltage corner mode
 * @Min: 0x00000000
 * @Max: 0x0fffffff
 * @Default: 0x0000000a
 * This ini is used to set operating voltage corner mode for differenet
 * phymode and bw configurations. Every 2 bits till BIT27 are dedicated
 * for a specific configuration. Bit values decide the type of voltage
 * corner mode. All the details below -
 *
 * Configure operating voltage corner mode based on phymode and bw.
 * bit 0-1 -   operating voltage corner mode for 11a/b.
 * bit 2-3 -   operating voltage corner mode for 11g.
 * bit 4-5 -   operating voltage corner mode for 11n, 20MHz, 1x1.
 * bit 6-7 -   operating voltage corner mode for 11n, 20MHz, 2x2.
 * bit 8-9 -   operating voltage corner mode for 11n, 40MHz, 1x1.
 * bit 10-11 - operating voltage corner mode for 11n, 40MHz, 2x2.
 * bit 12-13 - operating voltage corner mode for 11ac, 20MHz, 1x1.
 * bit 14-15 - operating voltage corner mode for 11ac, 20MHz, 2x2.
 * bit 16-17 - operating voltage corner mode for 11ac, 40MHz, 1x1.
 * bit 18-19 - operating voltage corner mode for 11ac, 40MHz, 2x2.
 * bit 20-21 - operating voltage corner mode for 11ac, 80MHz, 1x1.
 * bit 22-23 - operating voltage corner mode for 11ac, 80MHz, 2x2.
 * bit 24-25 - operating voltage corner mode for 11ac, 160MHz, 1x1.
 * bit 26-27 - operating voltage corner mode for 11ac, 160MHz, 2x2.
 * ---------------------------------------------
 * 00 - Static voltage corner SVS
 * 01 - static voltage corner LOW SVS
 * 10 - Dynamic voltage corner selection based on TPUT
 * 11 - Dynamic voltage corner selection based on TPUT and Tx Flush counters

 * Related: None
 *
 * Supported Feature: None
 *
 * Usage: External
 *
 * </ini>
 */

#define CFG_VC_MODE_BITMAP                  "gConfigVCmode"
#define CFG_VC_MODE_BITMAP_MIN              (0x00000000)
#define CFG_VC_MODE_BITMAP_MAX              (0x0fffffff)
#define CFG_VC_MODE_BITMAP_DEFAULT          (0x00000005)

/*
 * <ini>
 * gEnableSAPManadatoryChanList - Enable SAP Mandatory channel list
 * Options.
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable/disable the SAP manadatory chan list
 * 0 - Disable SAP mandatory chan list
 * 1 - Enable SAP mandatory chan list
 *
 * Supported Feature: SAP
 *
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_ENABLE_SAP_MANDATORY_CHAN_LIST       "gEnableSAPManadatoryChanList"
#define CFG_ENABLE_SAP_MANDATORY_CHAN_LIST_MIN   (0)
#define CFG_ENABLE_SAP_MANDATORY_CHAN_LIST_MAX   (1)
#define CFG_ENABLE_SAP_MANDATORY_CHAN_LIST_DEFAULT (0)

/*
 * <ini>
 * gShortGI40Mhz - It will check gShortGI20Mhz and
 * gShortGI40Mhz from session entry
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to set default gShortGI40Mhz
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_SHORT_GI_40MHZ_NAME                "gShortGI40Mhz"
#define CFG_SHORT_GI_40MHZ_MIN                 0
#define CFG_SHORT_GI_40MHZ_MAX                 1
#define CFG_SHORT_GI_40MHZ_DEFAULT             1

/*
 * <ini>
 * gEnableMCCMode - Enable/Disable MCC feature.
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable/disable MCC feature.
 *
 * Related: None.
 *
 * Supported Feature: Concurrency
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_ENABLE_MCC_ENABLED_NAME             "gEnableMCCMode"
#define CFG_ENABLE_MCC_ENABLED_MIN              (0)
#define CFG_ENABLE_MCC_ENABLED_MAX              (1)
#define CFG_ENABLE_MCC_ENABLED_DEFAULT          (1)

/*
 * <ini>
 * gAllowMCCGODiffBI - Allow GO in MCC mode to accept different beacon interval
 * than STA's.
 * @Min: 0
 * @Max: 4
 * @Default: 4
 *
 * This ini is used to allow GO in MCC mode to accept different beacon interval
 * than STA's.
 * Added for Wi-Fi Cert. 5.1.12
 * If gAllowMCCGODiffBI = 1
 *	Set to 1 for WFA certification. GO Beacon interval is not changed.
 *	MCC GO doesn't work well in optimized way. In worst scenario, it may
 *	invite STA disconnection.
 * If gAllowMCCGODiffBI = 2
 *	If set to 2 workaround 1 disassoc all the clients and update beacon
 *	Interval.
 * If gAllowMCCGODiffBI = 3
 *	If set to 3 tear down the P2P link in auto/Non-autonomous -GO case.
 * If gAllowMCCGODiffBI = 4
 *	If set to 4 don't disconnect the P2P client in autonomous/Non-auto-
 *	nomous -GO case update the BI dynamically
 *
 * Related: None.
 *
 * Supported Feature: Concurrency
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_ALLOW_MCC_GO_DIFF_BI_NAME           "gAllowMCCGODiffBI"
#define CFG_ALLOW_MCC_GO_DIFF_BI_MIN            (0)
#define CFG_ALLOW_MCC_GO_DIFF_BI_MAX            (4)
#define CFG_ALLOW_MCC_GO_DIFF_BI_DEFAULT        (4)

#if defined(CONFIG_HL_SUPPORT) && defined(QCA_BAD_PEER_TX_FLOW_CL)
/*
 * Enable/Disable Bad Peer TX CTL feature
 * Default: Enable
 */
#define CFG_BAD_PEER_TX_CTL_ENABLE_NAME		"gBadPeerTxCtlEnable"
#define CFG_BAD_PEER_TX_CTL_ENABLE_MIN         (0)
#define CFG_BAD_PEER_TX_CTL_ENABLE_MAX         (1)
#define CFG_BAD_PEER_TX_CTL_ENABLE_DEFAULT     (1)

#define CFG_BAD_PEER_TX_CTL_PERIOD_NAME		"gBadPeerTxCtlPeriod"
#define CFG_BAD_PEER_TX_CTL_PERIOD_MIN         (10)
#define CFG_BAD_PEER_TX_CTL_PERIOD_MAX         (10000)
#define CFG_BAD_PEER_TX_CTL_PERIOD_DEFAULT     (50)

#define CFG_BAD_PEER_TX_CTL_TXQ_LIMIT_NAME	"gBadPeerTxCtlTxqLimit"
#define CFG_BAD_PEER_TX_CTL_TXQ_LIMIT_MIN      (1)
#define CFG_BAD_PEER_TX_CTL_TXQ_LIMIT_MAX      (5000)
#define CFG_BAD_PEER_TX_CTL_TXQ_LIMIT_DEFAULT  (100)

#define CFG_BAD_PEER_TX_CTL_TGT_BACKOFF_T_NAME	"gBadPeerTxCtlTgtBackoffTime"
#define CFG_BAD_PEER_TX_CTL_TGT_BACKOFF_T_MIN     (1)
#define CFG_BAD_PEER_TX_CTL_TGT_BACKOFF_T_MAX     (5000)
#define CFG_BAD_PEER_TX_CTL_TGT_BACKOFF_T_DEFAULT (20)

#define CFG_BAD_PEER_TX_CTL_TGT_REPORT_PRD_NAME	"gBadPeerTxCtlTgtReportPeriod"
#define CFG_BAD_PEER_TX_CTL_TGT_REPORT_PRD_MIN     (1)
#define CFG_BAD_PEER_TX_CTL_TGT_REPORT_PRD_MAX     (5000)
#define CFG_BAD_PEER_TX_CTL_TGT_REPORT_PRD_DEFAULT (500)

#define CFG_BAD_PEER_TX_CTL_COND_LEVEL_IEEEB_NAME	"gBadPeerTxCtlCondLevelIeeeB"
#define CFG_BAD_PEER_TX_CTL_COND_LEVEL_IEEEB_MIN     (1)
#define CFG_BAD_PEER_TX_CTL_COND_LEVEL_IEEEB_MAX     (2)
#define CFG_BAD_PEER_TX_CTL_COND_LEVEL_IEEEB_DEFAULT (2)

#define CFG_BAD_PEER_TX_CTL_DELTA_LEVEL_IEEEB_NAME	"gBadPeerTxCtlDeltaLevelIeeeB"
#define CFG_BAD_PEER_TX_CTL_DELTA_LEVEL_IEEEB_MIN     (1)
#define CFG_BAD_PEER_TX_CTL_DELTA_LEVEL_IEEEB_MAX     (11)
#define CFG_BAD_PEER_TX_CTL_DELTA_LEVEL_IEEEB_DEFAULT (2)

#define CFG_BAD_PEER_TX_CTL_PCT_LEVEL_IEEEB_NAME	"gBadPeerTxCtlPctLevelIeeeB"
#define CFG_BAD_PEER_TX_CTL_PCT_LEVEL_IEEEB_MIN        (1)
#define CFG_BAD_PEER_TX_CTL_PCT_LEVEL_IEEEB_MAX        (8)
#define CFG_BAD_PEER_TX_CTL_PCT_LEVEL_IEEEB_DEFAULT    (1)

#define CFG_BAD_PEER_TX_CTL_TPUT_LEVEL_IEEEB_NAME	"gBadPeerTxCtlTputLevelIeeeB"
#define CFG_BAD_PEER_TX_CTL_TPUT_LEVEL_IEEEB_MIN       (1)
#define CFG_BAD_PEER_TX_CTL_TPUT_LEVEL_IEEEB_MAX       (11)
#define CFG_BAD_PEER_TX_CTL_TPUT_LEVEL_IEEEB_DEFAULT   (2)

#define CFG_BAD_PEER_TX_CTL_TX_LIMIT_LEVEL_IEEEB_NAME	"gBadPeerTxCtlTxLimitLevelIeeeB"
#define CFG_BAD_PEER_TX_CTL_TX_LIMIT_LEVEL_IEEEB_MIN      (0)
#define CFG_BAD_PEER_TX_CTL_TX_LIMIT_LEVEL_IEEEB_MAX      (50)
#define CFG_BAD_PEER_TX_CTL_TX_LIMIT_LEVEL_IEEEB_DEFAULT  (3)

#define CFG_BAD_PEER_TX_CTL_COND_LEVEL_IEEEAG_NAME	"gBadPeerTxCtlCondLevelIeeeAG"
#define CFG_BAD_PEER_TX_CTL_COND_LEVEL_IEEEAG_MIN         (1)
#define CFG_BAD_PEER_TX_CTL_COND_LEVEL_IEEEAG_MAX         (2)
#define CFG_BAD_PEER_TX_CTL_COND_LEVEL_IEEEAG_DEFAULT     (2)

#define CFG_BAD_PEER_TX_CTL_DELTA_LEVEL_IEEEAG_NAME	"gBadPeerTxCtlDeltaLevelIeeeAG"
#define CFG_BAD_PEER_TX_CTL_DELTA_LEVEL_IEEEAG_MIN        (6)
#define CFG_BAD_PEER_TX_CTL_DELTA_LEVEL_IEEEAG_MAX        (54)
#define CFG_BAD_PEER_TX_CTL_DELTA_LEVEL_IEEEAG_DEFAULT    (6)

#define CFG_BAD_PEER_TX_CTL_PCT_LEVEL_IEEEAG_NAME	"gBadPeerTxCtlPctLevelIeeeAG"
#define CFG_BAD_PEER_TX_CTL_PCT_LEVEL_IEEEAG_MIN          (1)
#define CFG_BAD_PEER_TX_CTL_PCT_LEVEL_IEEEAG_MAX          (8)
#define CFG_BAD_PEER_TX_CTL_PCT_LEVEL_IEEEAG_DEFAULT      (1)

#define CFG_BAD_PEER_TX_CTL_TPUT_LEVEL_IEEEAG_NAME	"gBadPeerTxCtlTputLevelIeeeAG"
#define CFG_BAD_PEER_TX_CTL_TPUT_LEVEL_IEEEAG_MIN         (6)
#define CFG_BAD_PEER_TX_CTL_TPUT_LEVEL_IEEEAG_MAX         (54)
#define CFG_BAD_PEER_TX_CTL_TPUT_LEVEL_IEEEAG_DEFAULT     (6)

#define CFG_BAD_PEER_TX_CTL_TX_LIMIT_LEVEL_IEEEAG_NAME	"gBadPeerTxCtlTxLimitLevelIeeeAG"
#define CFG_BAD_PEER_TX_CTL_TX_LIMIT_LEVEL_IEEEAG_MIN     (0)
#define CFG_BAD_PEER_TX_CTL_TX_LIMIT_LEVEL_IEEEAG_MAX     (50)
#define CFG_BAD_PEER_TX_CTL_TX_LIMIT_LEVEL_IEEEAG_DEFAULT (3)

#define CFG_BAD_PEER_TX_CTL_COND_LEVEL_IEEEN_NAME	"gBadPeerTxCtlCondLevelIeeeN"
#define CFG_BAD_PEER_TX_CTL_COND_LEVEL_IEEEN_MIN          (1)
#define CFG_BAD_PEER_TX_CTL_COND_LEVEL_IEEEN_MAX          (2)
#define CFG_BAD_PEER_TX_CTL_COND_LEVEL_IEEEN_DEFAULT      (2)

#define CFG_BAD_PEER_TX_CTL_DELTA_LEVEL_IEEEN_NAME	"gBadPeerTxCtlDeltaLevelIeeeN"
#define CFG_BAD_PEER_TX_CTL_DELTA_LEVEL_IEEEN_MIN         (6)
#define CFG_BAD_PEER_TX_CTL_DELTA_LEVEL_IEEEN_MAX         (72)
#define CFG_BAD_PEER_TX_CTL_DELTA_LEVEL_IEEEN_DEFAULT     (6)

#define CFG_BAD_PEER_TX_CTL_PCT_LEVEL_IEEEN_NAME	"gBadPeerTxCtlPctLevelIeeeN"
#define CFG_BAD_PEER_TX_CTL_PCT_LEVEL_IEEEN_MIN           (1)
#define CFG_BAD_PEER_TX_CTL_PCT_LEVEL_IEEEN_MAX           (8)
#define CFG_BAD_PEER_TX_CTL_PCT_LEVEL_IEEEN_DEFAULT       (1)

#define CFG_BAD_PEER_TX_CTL_TPUT_LEVEL_IEEEN_NAME	"gBadPeerTxCtlTputLevelIeeeN"
#define CFG_BAD_PEER_TX_CTL_TPUT_LEVEL_IEEEN_MIN          (6)
#define CFG_BAD_PEER_TX_CTL_TPUT_LEVEL_IEEEN_MAX          (72)
#define CFG_BAD_PEER_TX_CTL_TPUT_LEVEL_IEEEN_DEFAULT      (15)

#define CFG_BAD_PEER_TX_CTL_TX_LIMIT_LEVEL_IEEEN_NAME	"gBadPeerTxCtlTxLimitLevelIeeeN"
#define CFG_BAD_PEER_TX_CTL_TX_LIMIT_LEVEL_IEEEN_MIN      (0)
#define CFG_BAD_PEER_TX_CTL_TX_LIMIT_LEVEL_IEEEN_MAX      (50)
#define CFG_BAD_PEER_TX_CTL_TX_LIMIT_LEVEL_IEEEN_DEFAULT  (3)

#define CFG_BAD_PEER_TX_CTL_COND_LEVEL_IEEEAC_NAME	"gBadPeerTxCtlCondLevelIeeeAC"
#define CFG_BAD_PEER_TX_CTL_COND_LEVEL_IEEEAC_MIN         (1)
#define CFG_BAD_PEER_TX_CTL_COND_LEVEL_IEEEAC_MAX         (2)
#define CFG_BAD_PEER_TX_CTL_COND_LEVEL_IEEEAC_DEFAULT     (2)

#define CFG_BAD_PEER_TX_CTL_DELTA_LEVEL_IEEEAC_NAME	"gBadPeerTxCtlDeltaLevelIeeeAC"
#define CFG_BAD_PEER_TX_CTL_DELTA_LEVEL_IEEEAC_MIN        (6)
#define CFG_BAD_PEER_TX_CTL_DELTA_LEVEL_IEEEAC_MAX        (433)
#define CFG_BAD_PEER_TX_CTL_DELTA_LEVEL_IEEEAC_DEFAULT    (6)

#define CFG_BAD_PEER_TX_CTL_PCT_LEVEL_IEEEAC_NAME	"gBadPeerTxCtlPctLevelIeeeAC"
#define CFG_BAD_PEER_TX_CTL_PCT_LEVEL_IEEEAC_MIN          (1)
#define CFG_BAD_PEER_TX_CTL_PCT_LEVEL_IEEEAC_MAX          (8)
#define CFG_BAD_PEER_TX_CTL_PCT_LEVEL_IEEEAC_DEFAULT      (1)

#define CFG_BAD_PEER_TX_CTL_TPUT_LEVEL_IEEEAC_NAME	"gBadPeerTxCtlTputLevelIeeeAC"
#define CFG_BAD_PEER_TX_CTL_TPUT_LEVEL_IEEEAC_MIN         (6)
#define CFG_BAD_PEER_TX_CTL_TPUT_LEVEL_IEEEAC_MAX         (433)
#define CFG_BAD_PEER_TX_CTL_TPUT_LEVEL_IEEEAC_DEFAULT     (15)

#define CFG_BAD_PEER_TX_CTL_TX_LIMIT_LEVEL_IEEEAC_NAME    "gBadPeerTxCtlTxLimitLevelIeeeAC"
#define CFG_BAD_PEER_TX_CTL_TX_LIMIT_LEVEL_IEEEAC_MIN     (0)
#define CFG_BAD_PEER_TX_CTL_TX_LIMIT_LEVEL_IEEEAC_MAX     (50)
#define CFG_BAD_PEER_TX_CTL_TX_LIMIT_LEVEL_IEEEAC_DEFAULT (3)
#endif


/*
 * Enable/Disable Thermal Mitigation feature
 * Default: Enable
 */
#define CFG_THERMAL_MIGRATION_ENABLE_NAME      "gThermalMitigationEnable"
#define CFG_THERMAL_MIGRATION_ENABLE_MIN       (0)
#define CFG_THERMAL_MIGRATION_ENABLE_MAX       (1)
#define CFG_THERMAL_MIGRATION_ENABLE_DEFAULT   (0)

#define CFG_THROTTLE_PERIOD_NAME               "gThrottlePeriod"
#define CFG_THROTTLE_PERIOD_MIN                (10)
#define CFG_THROTTLE_PERIOD_MAX                (10000)
#define CFG_THROTTLE_PERIOD_DEFAULT            (4000)

/*
 * Configure Throttle Period Different Level Duty Cycle in percentage
 * When temperature measured is greater than threshold at particular level,
 * then throtling level will get increased by one level and
 * will reduce TX duty by the given percentage
 */
#define CFG_THROTTLE_DUTY_CYCLE_LEVEL0_NAME    "gThrottleDutyCycleLevel0"
#define CFG_THROTTLE_DUTY_CYCLE_LEVEL0_MIN     (0)
#define CFG_THROTTLE_DUTY_CYCLE_LEVEL0_MAX     (0)
#define CFG_THROTTLE_DUTY_CYCLE_LEVEL0_DEFAULT (0)

#define CFG_THROTTLE_DUTY_CYCLE_LEVEL1_NAME    "gThrottleDutyCycleLevel1"
#define CFG_THROTTLE_DUTY_CYCLE_LEVEL1_MIN     (0)
#define CFG_THROTTLE_DUTY_CYCLE_LEVEL1_MAX     (100)
#define CFG_THROTTLE_DUTY_CYCLE_LEVEL1_DEFAULT (50)

#define CFG_THROTTLE_DUTY_CYCLE_LEVEL2_NAME    "gThrottleDutyCycleLevel2"
#define CFG_THROTTLE_DUTY_CYCLE_LEVEL2_MIN     (0)
#define CFG_THROTTLE_DUTY_CYCLE_LEVEL2_MAX     (100)
#define CFG_THROTTLE_DUTY_CYCLE_LEVEL2_DEFAULT (75)

#define CFG_THROTTLE_DUTY_CYCLE_LEVEL3_NAME    "gThrottleDutyCycleLevel3"
#define CFG_THROTTLE_DUTY_CYCLE_LEVEL3_MIN     (0)
#define CFG_THROTTLE_DUTY_CYCLE_LEVEL3_MAX     (100)
#define CFG_THROTTLE_DUTY_CYCLE_LEVEL3_DEFAULT (94)

/*
 * <ini>
 * gMCAddrListEnable - Enable/Disable Multicast MAC Address List feature
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to set default MAC Address
 * Default: Enable
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_MC_ADDR_LIST_ENABLE_NAME          "gMCAddrListEnable"
#define CFG_MC_ADDR_LIST_ENABLE_MIN           (0)
#define CFG_MC_ADDR_LIST_ENABLE_MAX           (1)
#define CFG_MC_ADDR_LIST_ENABLE_DEFAULT       (1)

/*
 * <ini>
 * gEnableRXSTBC - Enables/disables Rx STBC capability in STA mode
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to set default Rx STBC capability
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_ENABLE_RX_STBC                       "gEnableRXSTBC"
#define CFG_ENABLE_RX_STBC_MIN                   (0)
#define CFG_ENABLE_RX_STBC_MAX                   (1)
#define CFG_ENABLE_RX_STBC_DEFAULT               (1)

/*
 * <ini>
 * gEnableTXSTBC - Enables/disables Tx STBC capability in STA mode
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to set default Tx STBC capability
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_ENABLE_TX_STBC                       "gEnableTXSTBC"
#define CFG_ENABLE_TX_STBC_MIN                   (0)
#define CFG_ENABLE_TX_STBC_MAX                   (1)
#define CFG_ENABLE_TX_STBC_DEFAULT               (0)

/*
 * <ini>
 * gPreventLinkDown - Enable to prevent bus link from going down
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * Enable to prevent bus link from going down. Useful for platforms that do not
 * (yet) support link down suspend cases.
 *
 * Related: N/A
 *
 * Supported Feature: Suspend/Resume
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_PREVENT_LINK_DOWN_NAME		"gPreventLinkDown"
#define CFG_PREVENT_LINK_DOWN_MIN		(0)
#define CFG_PREVENT_LINK_DOWN_MAX		(1)
#if defined(QCA_WIFI_NAPIER_EMULATION) || defined(QCA_WIFI_QCA6290)
#define CFG_PREVENT_LINK_DOWN_DEFAULT		(1)
#else
#define CFG_PREVENT_LINK_DOWN_DEFAULT		(0)
#endif /* QCA_WIFI_NAPIER_EMULATION */

/*
 * This INI item is used to control subsystem restart(SSR) test framework
 * Set it's value to 1 to enable APPS trigerred SSR testing
 */
#define CFG_ENABLE_CRASH_INJECT_NAME    "gEnableForceTargetAssert"
#define CFG_ENABLE_CRASH_INJECT_MIN     (0)
#define CFG_ENABLE_CRASH_INJECT_MAX     (1)
#define CFG_ENABLE_CRASH_INJECT_DEFAULT (0)

/*
 * <ini>
 * gTxLdpcEnable - Config Param to enable Tx LDPC capability
 * @Min: 0
 * @Max: 3
 * @Default: 3
 *
 * This ini is used to enable/disable Tx LDPC capability
 * 0 - disable
 * 1 - HT LDPC enable
 * 2 - VHT LDPC enable
 * 3 - HT & VHT LDPC enable
 *
 * Related: STA/SAP/P2P/IBSS/NAN.
 *
 * Supported Feature: Concurrency/Standalone
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_TX_LDPC_ENABLE_FEATURE         "gTxLdpcEnable"
#define CFG_TX_LDPC_ENABLE_FEATURE_MIN     (0)
#define CFG_TX_LDPC_ENABLE_FEATURE_MAX     (3)
#define CFG_TX_LDPC_ENABLE_FEATURE_DEFAULT (3)

/*
 * <ini>
 * gEnableRXLDPC - Config Param to enable Rx LDPC capability
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable/disable Rx LDPC capability
 *
 * Related: STA/SAP/P2P/IBSS/NAN.
 *
 * Supported Feature: Concurrency/Standalone
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_ENABLE_RX_LDPC                       "gEnableRXLDPC"
#define CFG_ENABLE_RX_LDPC_MIN                   (0)
#define CFG_ENABLE_RX_LDPC_MAX                   (1)
#define CFG_ENABLE_RX_LDPC_DEFAULT               (0)

/*
 * <ini>
 * gEnableMCCAdaptiveScheduler - MCC Adaptive Scheduler feature.
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable/disable MCC Adaptive Scheduler feature.
 *
 * Related: None.
 *
 * Supported Feature: Concurrency
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_ENABLE_MCC_ADATIVE_SCHEDULER_ENABLED_NAME             "gEnableMCCAdaptiveScheduler"
#define CFG_ENABLE_MCC_ADATIVE_SCHEDULER_ENABLED_MIN              (0)
#define CFG_ENABLE_MCC_ADATIVE_SCHEDULER_ENABLED_MAX              (1)
#define CFG_ENABLE_MCC_ADATIVE_SCHEDULER_ENABLED_DEFAULT          (1)

#define CFG_VHT_SU_BEAMFORMEE_CAP_FEATURE         "gTxBFEnable"
#define CFG_VHT_SU_BEAMFORMEE_CAP_FEATURE_MIN     (WNI_CFG_VHT_SU_BEAMFORMEE_CAP_STAMIN)
#define CFG_VHT_SU_BEAMFORMEE_CAP_FEATURE_MAX     (WNI_CFG_VHT_SU_BEAMFORMEE_CAP_STAMAX)
#define CFG_VHT_SU_BEAMFORMEE_CAP_FEATURE_DEFAULT (WNI_CFG_VHT_SU_BEAMFORMEE_CAP_STADEF)

/*
 * <ini>
 * enable_subfee_vendor_vhtie - ini to enable/disable SU Bformee in vendor VHTIE
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable/disable SU Bformee in vendor vht ie if gTxBFEnable
 * is enabled. if gTxBFEnable is 0 this will not have any effect.
 *
 * Related: gTxBFEnable.
 *
 * Supported Feature: STA
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ENABLE_SUBFEE_IN_VENDOR_VHTIE_NAME    "enable_subfee_vendor_vhtie"
#define CFG_ENABLE_SUBFEE_IN_VENDOR_VHTIE_MIN     (0)
#define CFG_ENABLE_SUBFEE_IN_VENDOR_VHTIE_MAX     (1)
#define CFG_ENABLE_SUBFEE_IN_VENDOR_VHTIE_DEFAULT (1)

/*
 * Enable / Disable Tx beamformee in SAP mode
 * Default: Disable
 */
#define CFG_VHT_ENABLE_TXBF_SAP_MODE         "gEnableTxBFeeSAP"
#define CFG_VHT_ENABLE_TXBF_SAP_MODE_MIN     (0)
#define CFG_VHT_ENABLE_TXBF_SAP_MODE_MAX     (1)
#define CFG_VHT_ENABLE_TXBF_SAP_MODE_DEFAULT (0)

/*
 * <ini>
 * gTxBFCsnValue - ini to set VHT/HE STS Caps field
 * @Min: 0
 * @Max: 7
 * @Default: 7
 *
 * This ini is used to configure the STS capability shown in AC/AX mode
 * MGMT frame IE, the final STS field shown in VHT/HE IE will be calculated
 * by MIN of (INI set, target report value). Only if gTxBFEnable is enabled
 * and SU/MU BEAMFORMEE Caps is shown, then STS Caps make sense.
 *
 * Related: gTxBFEnable.
 *
 * Supported Feature: STA/SAP
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_VHT_CSN_BEAMFORMEE_ANT_SUPPORTED         "gTxBFCsnValue"
#define CFG_VHT_CSN_BEAMFORMEE_ANT_SUPPORTED_MIN     (WNI_CFG_VHT_CSN_BEAMFORMEE_ANT_SUPPORTED_STAMIN)
#define CFG_VHT_CSN_BEAMFORMEE_ANT_SUPPORTED_MAX     (WNI_CFG_VHT_CSN_BEAMFORMEE_ANT_SUPPORTED_STAMAX - 1)
#define CFG_VHT_CSN_BEAMFORMEE_ANT_SUPPORTED_DEFAULT (WNI_CFG_VHT_CSN_BEAMFORMEE_ANT_SUPPORTED_STADEF - 1)

#define CFG_VHT_ENABLE_TXBF_IN_20MHZ               "gEnableTxBFin20MHz"
#define CFG_VHT_ENABLE_TXBF_IN_20MHZ_MIN           (0)
#define CFG_VHT_ENABLE_TXBF_IN_20MHZ_MAX           (1)
#define CFG_VHT_ENABLE_TXBF_IN_20MHZ_DEFAULT       (0)

#define CFG_VHT_ENABLE_TX_SU_BEAM_FORMER         "gEnableTxSUBeamformer"
#define CFG_VHT_ENABLE_TX_SU_BEAM_FORMER_MIN     (0)
#define CFG_VHT_ENABLE_TX_SU_BEAM_FORMER_MAX     (1)
#define CFG_VHT_ENABLE_TX_SU_BEAM_FORMER_DEFAULT (0)

#define CFG_DISABLE_LDPC_WITH_TXBF_AP             "gDisableLDPCWithTxbfAP"
#define CFG_DISABLE_LDPC_WITH_TXBF_AP_MIN         (0)
#define CFG_DISABLE_LDPC_WITH_TXBF_AP_MAX         (1)
#define CFG_DISABLE_LDPC_WITH_TXBF_AP_DEFAULT     (0)

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

/*
 * <ini>
 * gEnableIpTcpUdpChecksumOffload - It enables IP, TCP and UDP checksum
 * offload in hardware
 * @Min: 0
 * @Max: 1
 * @Default: DEF
 *
 * This ini is used to enable IP, TCP and UDP checksum offload in hardware
 * and also advertise same to network stack
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_ENABLE_IP_TCP_UDP_CHKSUM_OFFLOAD            "gEnableIpTcpUdpChecksumOffload"
#define CFG_ENABLE_IP_TCP_UDP_CHKSUM_OFFLOAD_DISABLE    (0)
#define CFG_ENABLE_IP_TCP_UDP_CHKSUM_OFFLOAD_ENABLE     (1)
#define CFG_ENABLE_IP_TCP_UDP_CHKSUM_OFFLOAD_DEFAULT    (CFG_ENABLE_IP_TCP_UDP_CHKSUM_OFFLOAD_ENABLE)

#ifdef WLAN_FEATURE_FASTPATH

/*
 * <ini>
 * gEnableFastPath - Control to enable fastpath feature
 *
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable fastpath feature
 *
 * Supported Feature: Wlan Fastpath Feature
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_ENABLE_FASTPATH                      "gEnableFastPath"
#define CFG_ENABLE_FASTPATH_MIN                  (0)
#define CFG_ENABLE_FASTPATH_MAX                  (1)
#define CFG_ENABLE_FASTPATH_DEFAULT              (CFG_ENABLE_FASTPATH_MIN)
#endif /* WLAN_FEATURE_FASTPATH */

/*
 * IPA Offload configuration - Each bit enables a feature
 * bit0 - IPA Enable
 * bit1 - IPA Pre filter enable
 * bit2 - IPv6 enable
 * bit3 - IPA Resource Manager (RM) enable
 * bit4 - IPA Clock scaling enable
 */
#define CFG_IPA_OFFLOAD_CONFIG_NAME              "gIPAConfig"
#define CFG_IPA_OFFLOAD_CONFIG_MIN               (0)
#define CFG_IPA_OFFLOAD_CONFIG_MAX               (0xFFFFFFFF)
#define CFG_IPA_OFFLOAD_CONFIG_DEFAULT           (CFG_IPA_OFFLOAD_CONFIG_MIN)

/*
 * IPA DESC SIZE
 */
#define CFG_IPA_DESC_SIZE_NAME                   "gIPADescSize"
#define CFG_IPA_DESC_SIZE_MIN                    (800)
#define CFG_IPA_DESC_SIZE_MAX                    (8000)
#define CFG_IPA_DESC_SIZE_DEFAULT                (800)

#define CFG_IPA_HIGH_BANDWIDTH_MBPS              "gIPAHighBandwidthMbps"
#define CFG_IPA_HIGH_BANDWIDTH_MBPS_MIN          (200)
#define CFG_IPA_HIGH_BANDWIDTH_MBPS_MAX          (1000)
#define CFG_IPA_HIGH_BANDWIDTH_MBPS_DEFAULT      (400)

#define CFG_IPA_MEDIUM_BANDWIDTH_MBPS            "gIPAMediumBandwidthMbps"
#define CFG_IPA_MEDIUM_BANDWIDTH_MBPS_MIN        (100)
#define CFG_IPA_MEDIUM_BANDWIDTH_MBPS_MAX        (400)
#define CFG_IPA_MEDIUM_BANDWIDTH_MBPS_DEFAULT    (200)

#define CFG_IPA_LOW_BANDWIDTH_MBPS               "gIPALowBandwidthMbps"
#define CFG_IPA_LOW_BANDWIDTH_MBPS_MIN           (0)
#define CFG_IPA_LOW_BANDWIDTH_MBPS_MAX           (100)
#define CFG_IPA_LOW_BANDWIDTH_MBPS_DEFAULT       (100)

/*
 * Firmware uart print
 */
#define CFG_ENABLE_FW_UART_PRINT_NAME             "gEnablefwprint"
#define CFG_ENABLE_FW_UART_PRINT_DISABLE          (0)
#define CFG_ENABLE_FW_UART_PRINT_ENABLE           (1)
#define CFG_ENABLE_FW_UART_PRINT_DEFAULT          (CFG_ENABLE_FW_UART_PRINT_DISABLE)

/*
 * Firmware log
 */
#define CFG_ENABLE_FW_LOG_NAME                   "gEnablefwlog"
#define CFG_ENABLE_FW_LOG_DISABLE                (0)
#define CFG_ENABLE_FW_LOG_WMI                    (1)
#define CFG_ENABLE_FW_LOG_DIAG                   (2)
#define CFG_ENABLE_FW_LOG_MIN                    (CFG_ENABLE_FW_LOG_DISABLE)
#define CFG_ENABLE_FW_LOG_MAX                    (CFG_ENABLE_FW_LOG_DIAG)
#define CFG_ENABLE_FW_LOG_DEFAULT                (CFG_ENABLE_FW_LOG_WMI)

/*
 * <ini>
 * gEnableDebugLog - Enable/Disable the Connection related logs
 * @Min: 0
 * @Max: 0xFF
 * @Default: 0x0F
 *
 * This ini is used to enable/disable the connection related logs
 * 0x1 - Enable mgmt pkt logs (excpet probe req/rsp, beacons).
 * 0x2 - Enable EAPOL pkt logs.
 * 0x4 - Enable DHCP pkt logs.
 * 0x8 - Enable mgmt action frames logs.
 * 0x0 - Disable all the above connection related logs.
 * The default value of 0x0F will enable all the above logs
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_ENABLE_DEBUG_CONNECT_ISSUE             "gEnableDebugLog"
#define CFG_ENABLE_DEBUG_CONNECT_ISSUE_MIN         (0)
#define CFG_ENABLE_DEBUG_CONNECT_ISSUE_MAX         (0xFF)
#define CFG_ENABLE_DEBUG_CONNECT_ISSUE_DEFAULT     (0x0F)

/* SAR Thermal limit values for 2g and 5g */

#define CFG_SET_TXPOWER_LIMIT2G_NAME               "TxPower2g"
#define CFG_SET_TXPOWER_LIMIT2G_MIN                (0)
#define CFG_SET_TXPOWER_LIMIT2G_MAX                (30)
#define CFG_SET_TXPOWER_LIMIT2G_DEFAULT            (30)

#define CFG_SET_TXPOWER_LIMIT5G_NAME               "TxPower5g"
#define CFG_SET_TXPOWER_LIMIT5G_MIN                (0)
#define CFG_SET_TXPOWER_LIMIT5G_MAX                (30)
#define CFG_SET_TXPOWER_LIMIT5G_DEFAULT            (30)

#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL

/*
 * <ini>
 * TxFlowLowWaterMark - Low watermark for pausing network queues
 *
 * @Min: 0
 * @Max: 1000
 * @Default: 300
 *
 * This ini specifies the low watermark of data packets transmitted
 * before pausing netif queues in tx flow path. It is only applicable
 * where legacy flow control is used i.e.for Rome.
 *
 * Related: TxFlowHighWaterMarkOffset, TxFlowMaxQueueDepth,
 *          TxLbwFlowLowWaterMark, TxLbwFlowHighWaterMarkOffset,
 *          TxLbwFlowMaxQueueDepth, TxHbwFlowLowWaterMark,
 *          TxHbwFlowHighWaterMarkOffset, TxHbwFlowMaxQueueDepth
 *
 * Supported Feature: Dynamic Flow Control
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_LL_TX_FLOW_LWM                         "TxFlowLowWaterMark"
#define CFG_LL_TX_FLOW_LWM_MIN                     (0)
#define CFG_LL_TX_FLOW_LWM_MAX                     (1000)
#define CFG_LL_TX_FLOW_LWM_DEFAULT                 (300)

/*
 * <ini>
 * TxFlowHighWaterMarkOffset - High Watermark offset to unpause Netif queues
 * @Min: 0
 * @Max: 300
 * @Default: 94
 *
 * This ini specifies the offset to upause the netif queues
 * when they are paused due to insufficient descriptors as guided by
 * ini TxFlowLowWaterMark. It is only applicable where legacy flow control
 * is used i.e.for Rome.
 *
 * Related: TxFlowLowWaterMark, TxFlowMaxQueueDepth,
 *          TxLbwFlowLowWaterMark, TxLbwFlowHighWaterMarkOffset,
 *          TxLbwFlowMaxQueueDepth, TxHbwFlowLowWaterMark,
 *          TxHbwFlowHighWaterMarkOffset, TxHbwFlowMaxQueueDepth
 *
 * Supported Feature: Dynamic Flow Control
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_LL_TX_FLOW_HWM_OFFSET                  "TxFlowHighWaterMarkOffset"
#define CFG_LL_TX_FLOW_HWM_OFFSET_MIN              (0)
#define CFG_LL_TX_FLOW_HWM_OFFSET_MAX              (300)
#define CFG_LL_TX_FLOW_HWM_OFFSET_DEFAULT          (94)

/*
 * <ini>
 * TxFlowMaxQueueDepth - Max pause queue depth.
 *
 * @Min: 400
 * @Max: 3500
 * @Default: 1500
 *
 * This ini specifies the max queue pause depth.It is only applicable
 * where legacy flow control is used i.e.for Rome.
 *
 * Related: TxFlowLowWaterMark, TxFlowHighWaterMarkOffset,
 *          TxLbwFlowLowWaterMark, TxLbwFlowHighWaterMarkOffset,
 *          TxLbwFlowMaxQueueDepth, TxHbwFlowLowWaterMark,
 *          TxHbwFlowHighWaterMarkOffset, TxHbwFlowMaxQueueDepth
 *
 * Supported Feature: Dynamic Flow Control
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_LL_TX_FLOW_MAX_Q_DEPTH                 "TxFlowMaxQueueDepth"
#define CFG_LL_TX_FLOW_MAX_Q_DEPTH_MIN             (400)
#define CFG_LL_TX_FLOW_MAX_Q_DEPTH_MAX             (3500)
#define CFG_LL_TX_FLOW_MAX_Q_DEPTH_DEFAULT         (1500)

/*
 * <ini>
 * TxLbwFlowLowWaterMark - Low watermark for pausing network queues
 *                         in low bandwidth band
 * @Min: 0
 * @Max: 1000
 * @Default: 450
 *
 * This ini specifies the low watermark of data packets transmitted
 * before pausing netif queues in tx flow path in low bandwidth band.
 * It is only applicable where legacy flow control is used i.e.for Rome.
 *
 * Related: TxFlowLowWaterMark, TxFlowHighWaterMarkOffset,
 *          TxFlowMaxQueueDepth, TxLbwFlowHighWaterMarkOffset,
 *          TxLbwFlowMaxQueueDepth, TxHbwFlowLowWaterMark,
 *          TxHbwFlowHighWaterMarkOffset, TxHbwFlowMaxQueueDepth
 *
 * Supported Feature: Dynamic Flow Control
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_LL_TX_LBW_FLOW_LWM                     "TxLbwFlowLowWaterMark"
#define CFG_LL_TX_LBW_FLOW_LWM_MIN                 (0)
#define CFG_LL_TX_LBW_FLOW_LWM_MAX                 (1000)
#define CFG_LL_TX_LBW_FLOW_LWM_DEFAULT             (450)

/*
 * <ini>
 * TxLbwFlowHighWaterMarkOffset - High Watermark offset to unpause Netif queues
 *                                in low bandwidth band.
 * @Min: 0
 * @Max: 300
 * @Default: 50
 *
 * This ini specifies the offset to upause the netif queues
 * when they are paused due to insufficient descriptors as guided by
 * ini TxLbwFlowLowWaterMark in low bandwidth band. It is only applicable
 * where legacy flow control is used i.e.for Rome.
 *
 * Related: TxFlowLowWaterMark, TxFlowHighWaterMarkOffset,
 *          TxFlowMaxQueueDepth, TxLbwFlowLowWaterMark,
 *          TxLbwFlowMaxQueueDepth, TxHbwFlowLowWaterMark,
 *          TxHbwFlowHighWaterMarkOffset, TxHbwFlowMaxQueueDepth
 *
 * Supported Feature: Dynamic Flow Control
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_LL_TX_LBW_FLOW_HWM_OFFSET              "TxLbwFlowHighWaterMarkOffset"
#define CFG_LL_TX_LBW_FLOW_HWM_OFFSET_MIN          (0)
#define CFG_LL_TX_LBW_FLOW_HWM_OFFSET_MAX          (300)
#define CFG_LL_TX_LBW_FLOW_HWM_OFFSET_DEFAULT      (50)

/*
 * <ini>
 * TxLbwFlowMaxQueueDepth - Max pause queue depth in low bandwidth band
 *
 * @Min: 400
 * @Max: 3500
 * @Default: 750
 *
 * This ini specifies the max queue pause depth in low bandwidth band.
 * It is only applicable where legacy flow control is used i.e.for Rome.
 *
 * Related: TxFlowLowWaterMark, TxFlowHighWaterMarkOffset,
 *          TxFlowMaxQueueDepth, TxLbwFlowLowWaterMark,
 *          TxLbwFlowHighWaterMarkOffset, TxHbwFlowLowWaterMark,
 *          TxHbwFlowHighWaterMarkOffset, TxHbwFlowMaxQueueDepth
 *
 * Supported Feature: Dynamic Flow Control
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_LL_TX_LBW_FLOW_MAX_Q_DEPTH             "TxLbwFlowMaxQueueDepth"
#define CFG_LL_TX_LBW_FLOW_MAX_Q_DEPTH_MIN         (400)
#define CFG_LL_TX_LBW_FLOW_MAX_Q_DEPTH_MAX         (3500)
#define CFG_LL_TX_LBW_FLOW_MAX_Q_DEPTH_DEFAULT     (750)

/*
 * <ini>
 * TxHbwFlowLowWaterMark - Low watermark for pausing network queues
 *                         in high bandwidth band
 * @Min: 0
 * @Max: 1000
 * @Default: 406
 *
 * This ini specifies the threshold of data packets transmitted
 * before pausing netif queues.It is only applicable where
 * legacy flow control is used i.e.for Rome.
 *
 * Related: TxFlowLowWaterMark, TxFlowHighWaterMarkOffset,
 *          TxFlowMaxQueueDepth, TxLbwFlowLowWaterMark,
 *          TxLbwFlowHighWaterMarkOffset, TxLbwFlowMaxQueueDepth,
 *          TxHbwFlowHighWaterMarkOffset, TxHbwFlowMaxQueueDepth
 *
 * Supported Feature: Dynamic Flow Control
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_LL_TX_HBW_FLOW_LWM                     "TxHbwFlowLowWaterMark"
#define CFG_LL_TX_HBW_FLOW_LWM_MIN                 (0)
#define CFG_LL_TX_HBW_FLOW_LWM_MAX                 (1000)
#define CFG_LL_TX_HBW_FLOW_LWM_DEFAULT             (406)

/*
 * <ini>
 * TxHbwFlowHighWaterMarkOffset - High Watermark offset to unpause Netif queues
 *                                in high bandwidth band.
 * @Min: 0
 * @Max: 300
 * @Default: 94
 *
 * This ini specifies the offset to upause the netif queues
 * when they are paused due to insufficient descriptors as guided by
 * ini TxHbwFlowLowWaterMark in high bandwidth band. It is only applicable
 * where legacy flow control is used i.e.for Rome.
 *
 * Related: TxFlowLowWaterMark, TxFlowHighWaterMarkOffset,
 *          TxFlowMaxQueueDepth, TxLbwFlowLowWaterMark,
 *          TxLbwFlowHighWaterMarkOffset, TxLbwFlowMaxQueueDepth,
 *          TxHbwFlowLowWaterMark, TxHbwFlowMaxQueueDepth
 *
 * Supported Feature: Dynamic Flow Control
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_LL_TX_HBW_FLOW_HWM_OFFSET              "TxHbwFlowHighWaterMarkOffset"
#define CFG_LL_TX_HBW_FLOW_HWM_OFFSET_MIN          (0)
#define CFG_LL_TX_HBW_FLOW_HWM_OFFSET_MAX          (300)
#define CFG_LL_TX_HBW_FLOW_HWM_OFFSET_DEFAULT      (94)

/*
 * <ini>
 * TxHbwFlowMaxQueueDepth - Max pause queue depth in high bandwidth band
 * @Min: 4000
 * @Max: 3500
 * @Default: 1500
 *
 * This ini specifies the max queue pause depth in high bandwidth band.
 * It is only applicable where legacy flow control is used i.e.for Rome.
 *
 * Related: TxFlowLowWaterMark, TxFlowHighWaterMarkOffset,
 *          TxFlowMaxQueueDepth, TxLbwFlowLowWaterMark,
 *          TxLbwFlowHighWaterMarkOffset, TxLbwFlowMaxQueueDepth,
 *          TxHbwFlowLowWaterMark, TxHbwFlowHighWaterMarkOffset
 *
 * Supported Feature: Dynamic Flow Control
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_LL_TX_HBW_FLOW_MAX_Q_DEPTH             "TxHbwFlowMaxQueueDepth"
#define CFG_LL_TX_HBW_FLOW_MAX_Q_DEPTH_MIN         (400)
#define CFG_LL_TX_HBW_FLOW_MAX_Q_DEPTH_MAX         (3500)
#define CFG_LL_TX_HBW_FLOW_MAX_Q_DEPTH_DEFAULT     (1500)
#endif /* QCA_LL_LEGACY_TX_FLOW_CONTROL */

#ifdef QCA_LL_TX_FLOW_CONTROL_V2

/*
 * <ini>
 * TxFlowStopQueueThreshold - Stop queue Threshold to pause
 *                            Netif queues when it reaches
 * @Min: 0
 * @Max: 50
 * @Default: 15
 *
 * This ini specifies the threshold of data packets transmitted
 * before pausing netif queues.
 *
 * Related: TxFlowStartQueueOffset
 *
 * Supported Feature: Dynamic Flow Control
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_LL_TX_FLOW_STOP_QUEUE_TH               "TxFlowStopQueueThreshold"
#define CFG_LL_TX_FLOW_STOP_QUEUE_TH_DEFAULT       (15)
#define CFG_LL_TX_FLOW_STOP_QUEUE_TH_MIN           (0)
#define CFG_LL_TX_FLOW_STOP_QUEUE_TH_MAX           (50)

/*
 * <ini>
 * TxFlowStartQueueOffset - Start queue offset to unpause
 *                          Netif queues
 * @Min: 0
 * @Max: 30
 * @Default: 11
 *
 * This ini specifies the offset to upause the netif queues
 * when they are paused due to insufficient descriptors as guided by
 * ini TxFlowStopQueueThreshold.
 *
 * Related: TxFlowStopQueueThreshold
 *
 * Supported Feature: Dynamic Flow Control
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_LL_TX_FLOW_START_QUEUE_OFFSET          "TxFlowStartQueueOffset"
#define CFG_LL_TX_FLOW_START_QUEUE_OFFSET_DEFAULT  (10)
#define CFG_LL_TX_FLOW_START_QUEUE_OFFSET_MIN      (0)
#define CFG_LL_TX_FLOW_START_QUEUE_OFFSET_MAX      (30)

#endif /* QCA_LL_TX_FLOW_CONTROL_V2 */

#ifdef FEATURE_WLAN_RA_FILTERING
#define CFG_RA_RATE_LIMIT_INTERVAL_NAME            "gRArateLimitInterval"
#define CFG_RA_RATE_LIMIT_INTERVAL_MIN             (60)
#define CFG_RA_RATE_LIMIT_INTERVAL_MAX             (3600)
#define CFG_RA_RATE_LIMIT_INTERVAL_DEFAULT         (60) /*60 SEC */
#endif

/*
 * <ini>
 * gEnableMemoryDebug - Enables the memory debug
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable default memory debug
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#ifdef MEMORY_DEBUG
#define CFG_ENABLE_MEMORY_DEBUG_NAME             "gEnableMemoryDebug"
#define CFG_ENABLE_MEMORY_DEBUG_MIN              (0)
#define CFG_ENABLE_MEMORY_DEBUG_MAX              (1)
#define CFG_ENABLE_MEMORY_DEBUG_DEFAULT          (1)
#endif

/*
 * <ini>
 * gMaxAmsduNum - Max number of MSDU's in aggregate
 * @Min: 0
 * @Max: 3
 * @Default: 1
 * gMaxAmsduNum is the number of MSDU's transmitted in the 11n aggregate
 * frame. Setting it to a value larger than 1 enables transmit aggregation.
 * It is a PHY parameter that applies to all vdev's in firmware.
 *
 * Supported Feature: 11n aggregation
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_MAX_AMSDU_NUM_NAME                "gMaxAmsduNum"
#define CFG_MAX_AMSDU_NUM_MIN                 (0)
#define CFG_MAX_AMSDU_NUM_MAX                 (3)
#define CFG_MAX_AMSDU_NUM_DEFAULT             (1)

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

/*
 * <ini>
 * gAllowDFSChannelRoam - Allow dfs channel in roam
 * @Min: 0
 * @Max: 1
 * @Default: 2
 *
 * This ini is used to set default dfs channel
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_ROAMING_DFS_CHANNEL_NAME               "gAllowDFSChannelRoam"
#define CFG_ROAMING_DFS_CHANNEL_DISABLED           (0)
#define CFG_ROAMING_DFS_CHANNEL_ENABLED_NORMAL     (1)
#define CFG_ROAMING_DFS_CHANNEL_ENABLED_ACTIVE     (2)
#define CFG_ROAMING_DFS_CHANNEL_MIN                (CFG_ROAMING_DFS_CHANNEL_DISABLED)
#define CFG_ROAMING_DFS_CHANNEL_MAX                (CFG_ROAMING_DFS_CHANNEL_ENABLED_ACTIVE)
#define CFG_ROAMING_DFS_CHANNEL_DEFAULT            (CFG_ROAMING_DFS_CHANNEL_DISABLED)

#ifdef MSM_PLATFORM
#define CFG_BUS_BANDWIDTH_HIGH_THRESHOLD           "gBusBandwidthHighThreshold"
#define CFG_BUS_BANDWIDTH_HIGH_THRESHOLD_DEFAULT   (2000)
#define CFG_BUS_BANDWIDTH_HIGH_THRESHOLD_MIN       (0)
#define CFG_BUS_BANDWIDTH_HIGH_THRESHOLD_MAX       (4294967295UL)

#define CFG_BUS_BANDWIDTH_MEDIUM_THRESHOLD         "gBusBandwidthMediumThreshold"
#define CFG_BUS_BANDWIDTH_MEDIUM_THRESHOLD_DEFAULT (500)
#define CFG_BUS_BANDWIDTH_MEDIUM_THRESHOLD_MIN     (0)
#define CFG_BUS_BANDWIDTH_MEDIUM_THRESHOLD_MAX     (4294967295UL)

#define CFG_BUS_BANDWIDTH_LOW_THRESHOLD            "gBusBandwidthLowThreshold"
#define CFG_BUS_BANDWIDTH_LOW_THRESHOLD_DEFAULT    (150)
#define CFG_BUS_BANDWIDTH_LOW_THRESHOLD_MIN        (0)
#define CFG_BUS_BANDWIDTH_LOW_THRESHOLD_MAX        (4294967295UL)

#define CFG_BUS_BANDWIDTH_COMPUTE_INTERVAL         "gBusBandwidthComputeInterval"
#define CFG_BUS_BANDWIDTH_COMPUTE_INTERVAL_DEFAULT (100)
#define CFG_BUS_BANDWIDTH_COMPUTE_INTERVAL_MIN     (0)
#define CFG_BUS_BANDWIDTH_COMPUTE_INTERVAL_MAX     (10000)

/*
 * <ini>
 * gEnableTcpLimitOutput - Control to enable TCP limit output byte
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable dynamic configuration of TCP limit output bytes
 * tcp_limit_output_bytes param. Enabling this will let driver post message to
 * cnss-daemon, accordingly cnss-daemon will modify the tcp_limit_output_bytes.
 *
 * Supported Feature: Tcp limit output bytes
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_ENABLE_TCP_LIMIT_OUTPUT                      "gTcpLimitOutputEnable"
#define CFG_ENABLE_TCP_LIMIT_OUTPUT_DEFAULT              (1)
#define CFG_ENABLE_TCP_LIMIT_OUTPUT_MIN                  (0)
#define CFG_ENABLE_TCP_LIMIT_OUTPUT_MAX                  (1)

/*
 * <ini>
 * gTcpAdvWinScaleEnable - Control to enable  TCP adv window scaling
 * @Min: -0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable dynamic configuration of TCP adv window scaling system parameter.
 *
 * Supported Feature: Tcp Advance Window Scaling
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_ENABLE_TCP_ADV_WIN_SCALE                      "gTcpAdvWinScaleEnable"
#define CFG_ENABLE_TCP_ADV_WIN_SCALE_DEFAULT              (1)
#define CFG_ENABLE_TCP_ADV_WIN_SCALE_MIN                  (0)
#define CFG_ENABLE_TCP_ADV_WIN_SCALE_MAX                  (1)

/*
 * <ini>
 * gTcpDelAckEnable - Control to enable Dynamic Configuration of Tcp Delayed Ack
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable Dynamic Configuration of Tcp Delayed Ack
 *
 * Related: gTcpDelAckThresholdHigh, gTcpDelAckThresholdLow,
 *          gTcpDelAckTimerCount
 *
 * Supported Feature: Tcp Delayed Ack
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_ENABLE_TCP_DELACK                      "gTcpDelAckEnable"
#define CFG_ENABLE_TCP_DELACK_DEFAULT              (1)
#define CFG_ENABLE_TCP_DELACK_MIN                  (0)
#define CFG_ENABLE_TCP_DELACK_MAX                  (1)


/*
 * <ini>
 * gTcpDelAckThresholdHigh - High Threshold inorder to trigger TCP Del Ack
 *                                          indication
 * @Min: 0
 * @Max: 16000
 * @Default: 500
 *
 * This ini is used to mention the High Threshold inorder to trigger TCP Del Ack
 * indication i.e the threshold of packets received over a period of 100 ms.
 * i.e to have a low RX throughput requirement
 * Related: gTcpDelAckEnable, gTcpDelAckThresholdLow, gTcpDelAckTimerCount
 *
 * Supported Feature: Tcp Delayed Ack
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_TCP_DELACK_THRESHOLD_HIGH              "gTcpDelAckThresholdHigh"
#define CFG_TCP_DELACK_THRESHOLD_HIGH_DEFAULT      (500)
#define CFG_TCP_DELACK_THRESHOLD_HIGH_MIN          (0)
#define CFG_TCP_DELACK_THRESHOLD_HIGH_MAX          (16000)

/*
 * <ini>
 * gTcpDelAckThresholdLow - Low Threshold inorder to trigger TCP Del Ack
 *                                          indication
 * @Min: 0
 * @Max: 10000
 * @Default: 1000
 *
 * This ini is used to mention the Low Threshold inorder to trigger TCP Del Ack
 * indication i.e the threshold of packets received over a period of 100 ms.
 * i.e to have a low RX throughput requirement
 *
 * Related: gTcpDelAckEnable, gTcpDelAckThresholdHigh, gTcpDelAckTimerCount
 *
 * Supported Feature: Tcp Delayed Ack
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_TCP_DELACK_THRESHOLD_LOW               "gTcpDelAckThresholdLow"
#define CFG_TCP_DELACK_THRESHOLD_LOW_DEFAULT       (1000)
#define CFG_TCP_DELACK_THRESHOLD_LOW_MIN           (0)
#define CFG_TCP_DELACK_THRESHOLD_LOW_MAX           (10000)

/*
 * <ini>
 * gTcpDelAckTimerCount - Del Ack Timer Count  inorder to trigger TCP Del Ack
 *                                      indication
 * @Min: 1
 * @Max: 1000
 * @Default: 30
 *
 * This ini is used to mention the Del Ack Timer Count inorder to
 * trigger TCP Del Ack indication i.e number of 100 ms periods
 *
 * Related: gTcpDelAckEnable, gTcpDelAckThresholdHigh, gTcpDelAckThresholdLow
 *
 * Supported Feature: Tcp Delayed Ack
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_TCP_DELACK_TIMER_COUNT                 "gTcpDelAckTimerCount"
#define CFG_TCP_DELACK_TIMER_COUNT_DEFAULT         (30)
#define CFG_TCP_DELACK_TIMER_COUNT_MIN             (1)
#define CFG_TCP_DELACK_TIMER_COUNT_MAX             (1000)


/*
 * <ini>
 * gTcpTxHighTputThreshold - High Threshold inorder to trigger High
 *                                          Tx Throughput requirement.
 * @Min: 0
 * @Max: 16000
 * @Default: 500
 *
 * This ini specifies the threshold of packets transmitted
 * over a period of 100 ms beyond which TCP can be considered to have a high
 * TX throughput requirement. The driver uses this condition to tweak TCP TX
 * specific parameters (via cnss-daemon)
 *
 * Supported Feature: To tweak TCP TX n/w parameters
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_TCP_TX_HIGH_TPUT_THRESHOLD_NAME         "gTcpTxHighTputThreshold"
#define CFG_TCP_TX_HIGH_TPUT_THRESHOLD_DEFAULT      (500)
#define CFG_TCP_TX_HIGH_TPUT_THRESHOLD_MIN          (0)
#define CFG_TCP_TX_HIGH_TPUT_THRESHOLD_MAX          (16000)

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

/*
 * <ini>
 * gMaxConcurrentActiveSessions - Maximum number of concurrent connections.
 * @Min: 1
 * @Max: 4
 * @Default: 3
 *
 * This ini is used to configure the maximum number of concurrent connections.
 *
 * Related: None.
 *
 * Supported Feature: Concurrency
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_MAX_CONCURRENT_CONNECTIONS_NAME        "gMaxConcurrentActiveSessions"
#define CFG_MAX_CONCURRENT_CONNECTIONS_DEFAULT     (3)
#define CFG_MAX_CONCURRENT_CONNECTIONS_MIN         (1)
#define CFG_MAX_CONCURRENT_CONNECTIONS_MAX         (4)

/*
 * <ini>
 * gIgnoreCAC - Used to ignore CAC
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to set default CAC
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_IGNORE_CAC_NAME                        "gIgnoreCAC"
#define CFG_IGNORE_CAC_MIN                         (0)
#define CFG_IGNORE_CAC_MAX                         (1)
#define CFG_IGNORE_CAC_DEFAULT                     (0)

#define CFG_DFS_RADAR_PRI_MULTIPLIER_NAME          "gDFSradarMappingPriMultiplier"
#define CFG_DFS_RADAR_PRI_MULTIPLIER_DEFAULT       (4)
#define CFG_DFS_RADAR_PRI_MULTIPLIER_MIN           (0)
#define CFG_DFS_RADAR_PRI_MULTIPLIER_MAX           (10)

/*
 * <ini>
 * gReorderOffloadSupported - Packet reordering offload to firmware
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to set default Packet reordering
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_REORDER_OFFLOAD_SUPPORT_NAME    "gReorderOffloadSupported"
#define CFG_REORDER_OFFLOAD_SUPPORT_MIN     (0)
#define CFG_REORDER_OFFLOAD_SUPPORT_MAX     (1)
#define CFG_REORDER_OFFLOAD_SUPPORT_DEFAULT (1)

/* IpaUcTxBufCount should be power of 2 */
#define CFG_IPA_UC_TX_BUF_COUNT_NAME               "IpaUcTxBufCount"
#define CFG_IPA_UC_TX_BUF_COUNT_MIN                (0)
#define CFG_IPA_UC_TX_BUF_COUNT_MAX                (2048)
#define CFG_IPA_UC_TX_BUF_COUNT_DEFAULT            (512)

#define CFG_IPA_UC_TX_BUF_SIZE_NAME                "IpaUcTxBufSize"
#define CFG_IPA_UC_TX_BUF_SIZE_MIN                (0)
#define CFG_IPA_UC_TX_BUF_SIZE_MAX                (4096)
#define CFG_IPA_UC_TX_BUF_SIZE_DEFAULT            (2048)

/* IpaUcRxIndRingCount should be power of 2 */
#define CFG_IPA_UC_RX_IND_RING_COUNT_NAME          "IpaUcRxIndRingCount"
#define CFG_IPA_UC_RX_IND_RING_COUNT_MIN           (0)
#define CFG_IPA_UC_RX_IND_RING_COUNT_MAX           (2048)
#define CFG_IPA_UC_RX_IND_RING_COUNT_DEFAULT       (1024)

#define CFG_IPA_UC_TX_PARTITION_BASE_NAME          "IpaUcTxPartitionBase"
#define CFG_IPA_UC_TX_PARTITION_BASE_MIN           (0)
#define CFG_IPA_UC_TX_PARTITION_BASE_MAX           (9000)
#define CFG_IPA_UC_TX_PARTITION_BASE_DEFAULT       (3000)

#ifdef WLAN_LOGGING_SOCK_SVC_ENABLE
/* Enable WLAN Logging to app space */
#define CFG_WLAN_LOGGING_SUPPORT_NAME               "wlanLoggingEnable"
#define CFG_WLAN_LOGGING_SUPPORT_ENABLE             (1)
#define CFG_WLAN_LOGGING_SUPPORT_DISABLE            (0)
#define CFG_WLAN_LOGGING_SUPPORT_DEFAULT            (1)

/* Enable forwarding the driver logs to kmsg console */
#define CFG_WLAN_LOGGING_CONSOLE_SUPPORT_NAME    "wlanLoggingToConsole"
#define CFG_WLAN_LOGGING_CONSOLE_SUPPORT_ENABLE  (1)
#define CFG_WLAN_LOGGING_CONSOLE_SUPPORT_DISABLE (0)
#define CFG_WLAN_LOGGING_CONSOLE_SUPPORT_DEFAULT (1)
#endif /* WLAN_LOGGING_SOCK_SVC_ENABLE */

#ifdef WLAN_FEATURE_LPSS
#define CFG_ENABLE_LPASS_SUPPORT_NAME                     "gEnableLpassSupport"
#define CFG_ENABLE_LPASS_SUPPORT_DEFAULT                  (0)
#define CFG_ENABLE_LPASS_SUPPORT_MIN                      (0)
#define CFG_ENABLE_LPASS_SUPPORT_MAX                      (1)
#endif

#define CFG_ENABLE_SELF_RECOVERY_NAME              "gEnableSelfRecovery"
#define CFG_ENABLE_SELF_RECOVERY_MIN               (0)
#define CFG_ENABLE_SELF_RECOVERY_MAX               (1)
#define CFG_ENABLE_SELF_RECOVERY_DEFAULT           (0)

#define CFG_ENABLE_SAP_SUSPEND                     "gEnableSapSuspend"
#define CFG_ENABLE_SAP_SUSPEND_MIN                 (0)
#define CFG_ENABLE_SAP_SUSPEND_MAX                 (1)
#define CFG_ENABLE_SAP_SUSPEND_DEFAULT             (1)

/*
 * <ini>
 * gEnableDeauthToDisassocMap - Enables deauth to disassoc map
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to set default  disassoc map
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_ENABLE_DEAUTH_TO_DISASSOC_MAP_NAME    "gEnableDeauthToDisassocMap"
#define CFG_ENABLE_DEAUTH_TO_DISASSOC_MAP_MIN     (0)
#define CFG_ENABLE_DEAUTH_TO_DISASSOC_MAP_MAX     (1)
#define CFG_ENABLE_DEAUTH_TO_DISASSOC_MAP_DEFAULT (0)

#ifdef DHCP_SERVER_OFFLOAD
#define CFG_DHCP_SERVER_IP_NAME     "gDHCPServerIP"
#define CFG_DHCP_SERVER_IP_DEFAULT  ""
#endif /* DHCP_SERVER_OFFLOAD */

/*
 * <ini>
 * gEnableCustomConcRule1 - Enable custom concurrency rule1.
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable/disable custom concurrency rule1.
 * If SAP comes up first and STA comes up later then SAP needs to follow STA's
 * channel.
 *
 * Related: None.
 *
 * Supported Feature: Concurrency
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_ENABLE_CUSTOM_CONC_RULE1_NAME         "gEnableCustomConcRule1"
#define CFG_ENABLE_CUSTOM_CONC_RULE1_NAME_MIN     (0)
#define CFG_ENABLE_CUSTOM_CONC_RULE1_NAME_MAX     (1)
#define CFG_ENABLE_CUSTOM_CONC_RULE1_NAME_DEFAULT (0)

/*
 * <ini>
 * gEnableCustomConcRule2 - Enable custom concurrency rule2.
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable/disable custom concurrency rule2.
 * If P2PGO comes up first and STA comes up later then P2PGO need to follow
 * STA's channel in 5Ghz. In following if condition we are just adding sanity
 * check to make sure that by this time P2PGO's channel is same as STA's
 * channel.
 *
 * Related: None.
 *
 * Supported Feature: Concurrency
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_ENABLE_CUSTOM_CONC_RULE2_NAME         "gEnableCustomConcRule2"
#define CFG_ENABLE_CUSTOM_CONC_RULE2_NAME_MIN     (0)
#define CFG_ENABLE_CUSTOM_CONC_RULE2_NAME_MAX     (1)
#define CFG_ENABLE_CUSTOM_CONC_RULE2_NAME_DEFAULT (0)

#define CFG_ENABLE_STA_CONNECTION_IN_5GHZ         "gEnableStaConnectionIn5Ghz"
#define CFG_ENABLE_STA_CONNECTION_IN_5GHZ_MIN     (0)
#define CFG_ENABLE_STA_CONNECTION_IN_5GHZ_MAX     (1)
#define CFG_ENABLE_STA_CONNECTION_IN_5GHZ_DEFAULT (1)

#define CFG_ENABLE_MAC_ADDR_SPOOFING                "gEnableMacAddrSpoof"
#define CFG_ENABLE_MAC_ADDR_SPOOFING_MIN            (0)
#define CFG_ENABLE_MAC_ADDR_SPOOFING_MAX            (1)
#define CFG_ENABLE_MAC_ADDR_SPOOFING_DEFAULT        (1)

/*
 * <ini>
 * gStaMiracastMccRestTimeVal - Rest time when Miracast is running.
 * @Min: 100
 * @Max: 500
 * @Default: 400
 *
 * This ini is used to set rest time for home channel for Miracast before
 * going for scan.
 *
 * Related: None.
 *
 * Supported Feature: Concurrency
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_STA_MIRACAST_MCC_REST_TIME_VAL          "gStaMiracastMccRestTimeVal"
#define CFG_STA_MIRACAST_MCC_REST_TIME_VAL_MIN     (100)
#define CFG_STA_MIRACAST_MCC_REST_TIME_VAL_MAX     (500)
#define CFG_STA_MIRACAST_MCC_REST_TIME_VAL_DEFAULT (400)

#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
/*
 * <ini>
 * gSapChannelAvoidance - SAP MCC channel avoidance.
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to sets sap mcc channel avoidance.
 *
 * Related: None.
 *
 * Supported Feature: Concurrency
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_SAP_MCC_CHANNEL_AVOIDANCE_NAME         "gSapChannelAvoidance"
#define CFG_SAP_MCC_CHANNEL_AVOIDANCE_MIN          (0)
#define CFG_SAP_MCC_CHANNEL_AVOIDANCE_MAX          (1)
#define CFG_SAP_MCC_CHANNEL_AVOIDANCE_DEFAULT      (0)
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */

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

#define CFG_SAP_DOT11MC_NAME          "gSapDot11mc"
#define CFG_SAP_DOT11MC_MIN           (0)
#define CFG_SAP_DOT11MC_MAX           (1)
#define CFG_SAP_DOT11MC_DEFAULT       (0)

#define CFG_ENABLE_NON_DFS_CHAN_ON_RADAR           "gPreferNonDfsChanOnRadar"
#define CFG_ENABLE_NON_DFS_CHAN_ON_RADAR_MIN       (0)
#define CFG_ENABLE_NON_DFS_CHAN_ON_RADAR_MAX       (1)
#define CFG_ENABLE_NON_DFS_CHAN_ON_RADAR_DEFAULT   (0)

#define CFG_MULTICAST_HOST_FW_MSGS          "gMulticastHostFwMsgs"
#define CFG_MULTICAST_HOST_FW_MSGS_MIN      (0)
#define CFG_MULTICAST_HOST_FW_MSGS_MAX      (1)
#if defined(MDM_PLATFORM) && !defined(FEATURE_MULTICAST_HOST_FW_MSGS)
#define CFG_MULTICAST_HOST_FW_MSGS_DEFAULT  (0)
#else
#define CFG_MULTICAST_HOST_FW_MSGS_DEFAULT  (1)
#endif


/*
 * <ini>
 * gSystemPref - Configure wlan system preference for PCL.
 * @Min: 0
 * @Max: 2
 * @Default: 0
 *
 * This ini is used to configure wlan system preference option to help
 * policy manager decide on Preferred Channel List for a new connection.
 * For possible values refer to enum hdd_conc_priority_mode
 *
 * Related: None.
 *
 * Supported Feature: DBS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_CONC_SYSTEM_PREF               "gSystemPref"
#define CFG_CONC_SYSTEM_PREF_MIN           (0)
#define CFG_CONC_SYSTEM_PREF_MAX           (2)
#define CFG_CONC_SYSTEM_PREF_DEFAULT       (0)

/*
 * <ini>
 * dbs_selection_policy - Configure dbs selection policy.
 * @Min: 0
 * @Max: 3
 * @Default: 0
 *
 *  set band preference or Vdev preference.
 *      bit[0] = 0: 5G 2x2 preferred to select 2x2 5G + 1x1 2G DBS mode.
 *      bit[0] = 1: 2G 2x2 preferred to select 2x2 2G + 1x1 5G DBS mode.
 *      bit[1] = 1: vdev priority enabled. The INI "vdev_priority_list" will
 * specify the vdev priority.
 *      bit[1] = 0: vdev priority disabled.
 * This INI only take effect for Genoa dual DBS hw.
 *
 * Supported Feature: DBS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_DBS_SELECTION_POLICY               "dbs_selection_policy"
#define CFG_DBS_SELECTION_POLICY_MIN           (0)
#define CFG_DBS_SELECTION_POLICY_MAX           (0x3)
#define CFG_DBS_SELECTION_POLICY_DEFAULT       (0)

/*
 * <ini>
 * vdev_priority_list - Configure vdev priority list.
 * @Min: 0
 * @Max: 0x4444
 * @Default: 0x4321
 *
 * @vdev_priority_list: vdev priority list
 *      bit[0-3]: pri_id (policy_mgr_pri_id) of highest priority
 *      bit[4-7]: pri_id (policy_mgr_pri_id) of second priority
 *      bit[8-11]: pri_id (policy_mgr_pri_id) of third priority
 *      bit[12-15]: pri_id (policy_mgr_pri_id) of fourth priority
 *      example: 0x4321 - CLI < GO < SAP < STA
 *      vdev priority id mapping:
 *        PM_STA_PRI_ID = 1,
 *        PM_SAP_PRI_ID = 2,
 *        PM_P2P_GO_PRI_ID = 3,
 *        PM_P2P_CLI_PRI_ID = 4,
 * When the previous INI "dbs_selection_policy" bit[1]=1, which means
 * the vdev 2x2 prioritization enabled. Then this INI will be used to
 * specify the vdev type priority list. For example :
 * dbs_selection_policy=0x2
 * vdev_priority_list=0x4312
 * means: default preference 2x2 band is 5G, vdev 2x2 prioritization enabled.
 * And the priority list is CLI < GO < STA < SAP
 *
 * This INI only take effect for Genoa dual DBS hw.
 *
 * Supported Feature: DBS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_VDEV_PRIORITY_LIST               "vdev_priority_list"
#define CFG_VDEV_PRIORITY_LIST_MIN           (0)
#define CFG_VDEV_PRIORITY_LIST_MAX           (0x4444)
#define CFG_VDEV_PRIORITY_LIST_DEFAULT       (0x4321)

/*
 * <ini>
 * TSOEnable - Control to enable tso feature
 *
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable TSO feature
 *
 * Supported Feature: TSO Feature
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_TSO_ENABLED_NAME           "TSOEnable"
#define CFG_TSO_ENABLED_MIN            (0)
#define CFG_TSO_ENABLED_MAX            (1)
#define CFG_TSO_ENABLED_DEFAULT        (0)

/*
 * <ini>
 * LROEnable - Control to enable lro feature
 *
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable LRO feature
 *
 * Supported Feature: LRO
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_LRO_ENABLED_NAME           "LROEnable"
#define CFG_LRO_ENABLED_MIN            (0)
#define CFG_LRO_ENABLED_MAX            (1)
#define CFG_LRO_ENABLED_DEFAULT        (0)

/*
 * <ini>
 * GROEnable - Control to enable gro feature
 *
 * @Disable: 0
 * @Enable: 1
 * @Default: 0
 *
 * This ini is used to enable GRO feature
 *
 * Supported Feature: GRO
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_GRO_ENABLED_NAME           "GROEnable"
#define CFG_GRO_ENABLED_MIN            (0)
#define CFG_GRO_ENABLED_MAX            (1)
#define CFG_GRO_ENABLED_DEFAULT        (0)

/*
 * Enable Rx traffic flow steering to enable Rx interrupts on multiple CEs based
 * on the flows. Different CEs<==>different IRQs<==>probably different CPUs.
 * Parallel Rx paths.
 * 1 - enable  0 - disable
 */
#define CFG_FLOW_STEERING_ENABLED_NAME        "gEnableFlowSteering"
#define CFG_FLOW_STEERING_ENABLED_MIN         (0)
#define CFG_FLOW_STEERING_ENABLED_MAX         (1)
#define CFG_FLOW_STEERING_ENABLED_DEFAULT     (0)

/*
 * Max number of MSDUs per HTT RX IN ORDER INDICATION msg.
 * Note that this has a direct impact on the size of source CE rings.
 * It is possible to go below 8, but would require testing; so we are
 * restricting the lower limit to 8 artificially
 *
 * It is recommended that this value is a POWER OF 2.
 *
 * Values lower than 8 are for experimental purposes only.
 */
#define CFG_MAX_MSDUS_PER_RXIND_NAME          "maxMSDUsPerRxInd"
#define CFG_MAX_MSDUS_PER_RXIND_MIN           (4)
#define CFG_MAX_MSDUS_PER_RXIND_MAX           (32)
#define CFG_MAX_MSDUS_PER_RXIND_DEFAULT       (32)

/*
 * set the self gen power value from
 * 0 to 0xffff
 */
#define CFG_SELF_GEN_FRM_PWR        "gSelfGenFrmPwr"
#define CFG_SELF_GEN_FRM_PWR_MIN      (0)
#define CFG_SELF_GEN_FRM_PWR_MAX      (0xffff)
#define CFG_SELF_GEN_FRM_PWR_DEFAULT  (0)

/*
 * fine timing measurement capability information
 *
 * <----- fine_time_meas_cap (in bits) ----->
 *+----------+-----+-----+------+------+-------+-------+-----+-----+
 *|   8-31   |  7  |  6  |   5  |   4  |   3   |   2   |  1  |  0  |
 *+----------+-----+-----+------+------+-------+-------+-----+-----+
 *| reserved | SAP | SAP |P2P-GO|P2P-GO|P2P-CLI|P2P-CLI| STA | STA |
 *|          |resp |init |resp  |init  |resp   |init   |resp |init |
 *+----------+-----+-----+------+------+-------+-------+-----+-----+
 *
 * resp - responder role; init- initiator role
 *
 * CFG_FINE_TIME_MEAS_CAPABILITY_MAX computed based on the table
 * +-----------------+-----------------+-----------+
 * |  Device Role    |   Initiator     | Responder |
 * +-----------------+-----------------+-----------+
 * |   Station       |       Y         |     N     |
 * |   P2P-CLI       |       Y         |     Y     |
 * |   P2P-GO        |       Y         |     Y     |
 * |   SAP           |       N         |     Y     |
 * +-----------------+-----------------+-----------+
 */
#define CFG_FINE_TIME_MEAS_CAPABILITY              "gfine_time_meas_cap"
#define CFG_FINE_TIME_MEAS_CAPABILITY_MIN          (0x0000)
#define CFG_FINE_TIME_MEAS_CAPABILITY_MAX          (0x00BD)
#define CFG_FINE_TIME_MEAS_CAPABILITY_DEFAULT      (0x000D)

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

#define CFG_CE_CLASSIFY_ENABLE_NAME	"gCEClassifyEnable"
#define CFG_CE_CLASSIFY_ENABLE_MIN	(0)
#define CFG_CE_CLASSIFY_ENABLE_MAX	(1)
#define CFG_CE_CLASSIFY_ENABLE_DEFAULT	(1)

/*
 *
 * <ini>
 * gDualMacFeatureDisable - Disable Dual MAC feature.
 * @Min: 0
 * @Max: 4
 * @Default: 0
 *
 * This ini is used to enable/disable dual MAC feature.
 * 0 - enable DBS
 * 1 - disable DBS
 * 2 - disable DBS for connection but keep DBS for scan
 * 3 - disable DBS for connection but keep DBS scan with async
 * scan policy disabled
 * 4 - enable DBS for connection as well as for scan with async
 * scan policy disabled
 * 5 - enable DBS for connection but disable DBS for scan.
 * 6 - enable DBS for connection but disable simultaneous scan
 * from upper layer (DBS scan remains enabled in FW).
 *
 * Note: INI item value should match 'enum dbs_support'
 *
 * Related: None.
 *
 * Supported Feature: DBS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_DUAL_MAC_FEATURE_DISABLE              "gDualMacFeatureDisable"
#define CFG_DUAL_MAC_FEATURE_DISABLE_MIN          (0)
#define CFG_DUAL_MAC_FEATURE_DISABLE_MAX          (6)
#define CFG_DUAL_MAC_FEATURE_DISABLE_DEFAULT      (0)

/*
 * <ini>
 * gdbs_scan_selection - DBS Scan Selection.
 * @Default: ""
 *
 * This ini is used to enable DBS scan selection.
 * Example
 * @Value: "5,2,2,16,2,2"
 * 1st argument is module_id, 2nd argument is number of DBS scan,
 * 3rd argument is number of non-DBS scan,
 * and other arguments follows.
 * 5,2,2,16,2,2 means:
 * 5 is module id, 2 is num of DBS scan, 2 is num of non-DBS scan.
 * 16 is module id, 2 is num of DBS scan, 2 is num of non-DBS scan.
 *
 * Related: None.
 *
 * Supported Feature: DBS Scan
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_DBS_SCAN_SELECTION_NAME          "gdbs_scan_selection"
#define CFG_DBS_SCAN_SELECTION_DEFAULT       ""

/*
 * <ini>
 * g_sta_sap_scc_on_dfs_chan - Allow STA+SAP SCC on DFS channel with master
 * mode support disabled.
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to allow STA+SAP SCC on DFS channel with master mode
 * support disabled.
 * 0 - Disallow STA+SAP SCC on DFS channel
 * 1 - Allow STA+SAP SCC on DFS channel with master mode disabled
 *
 * Related: None.
 *
 * Supported Feature: Non-DBS, DBS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_STA_SAP_SCC_ON_DFS_CHAN              "g_sta_sap_scc_on_dfs_chan"
#define CFG_STA_SAP_SCC_ON_DFS_CHAN_MIN          (0)
#define CFG_STA_SAP_SCC_ON_DFS_CHAN_MAX          (1)
#define CFG_STA_SAP_SCC_ON_DFS_CHAN_DEFAULT      (0)

/*
 * <ini>
 * g_sta_sap_scc_on_lte_coex_chan - Allow STA+SAP SCC on LTE coex channel
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to allow STA+SAP SCC on LTE coex channel
 * 0 - Disallow STA+SAP SCC on LTE coex channel
 * 1 - Allow STA+SAP SCC on LTE coex channel
 *
 * Related: None.
 *
 * Supported Feature: Non-DBS, DBS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_STA_SAP_SCC_ON_LTE_COEX_CHAN              "g_sta_sap_scc_on_lte_coex_chan"
#define CFG_STA_SAP_SCC_ON_LTE_COEX_CHAN_MIN          (0)
#define CFG_STA_SAP_SCC_ON_LTE_COEX_CHAN_MAX          (1)
#define CFG_STA_SAP_SCC_ON_LTE_COEX_CHAN_DEFAULT      (0)

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

/* Option to report rssi in cfg80211_inform_bss_frame()
 * 0 = use rssi value based on noise floor = -96 dBm
 * 1 = use rssi value based on actual noise floor in hardware
 */
#define CFG_INFORM_BSS_RSSI_RAW_NAME               "gInformBssRssiRaw"
#define CFG_INFORM_BSS_RSSI_RAW_MIN                (0)
#define CFG_INFORM_BSS_RSSI_RAW_MAX                (1)
#define CFG_INFORM_BSS_RSSI_RAW_DEFAULT            (1)

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

/*
 * Enable/Disable to initiate BUG report in case of fatal event
 * Default: Enable
 */
#define CFG_ENABLE_FATAL_EVENT_TRIGGER_NAME            "gEnableFatalEvent"
#define CFG_ENABLE_FATAL_EVENT_TRIGGER_MIN             (0)
#define CFG_ENABLE_FATAL_EVENT_TRIGGER_MAX             (1)
#define CFG_ENABLE_FATAL_EVENT_TRIGGER_DEFAULT         (1)

/*
 * <ini>
 * gEnableEdcaParams - Enable edca parameter
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used if gEnableEdcaParams is set to 1, params gEdcaVoCwmin,
 * gEdcaViCwmin, gEdcaBkCwmin, gEdcaBeCwmin, gEdcaVoCwmax,
 * gEdcaViCwmax, gEdcaBkCwmax, gEdcaBeCwmax, gEdcaVoAifs,
 * gEdcaViAifs, gEdcaBkAifs and gEdcaBeAifs values are used
 * to overwrite the values received from AP
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_ENABLE_EDCA_INI_NAME       "gEnableEdcaParams"
#define CFG_ENABLE_EDCA_INI_MIN        (0)
#define CFG_ENABLE_EDCA_INI_MAX        (1)
#define CFG_ENABLE_EDCA_INI_DEFAULT    (0)

/*
 * <ini>
 * gEdcaVoCwmin - Set Cwmin value for EDCA_AC_VO
 * @Min: 0
 * @Max: 0x15
 * @Default: 2
 *
 * This ini is used to set default Cwmin value for EDCA_AC_VO
 * Cwmin value for EDCA_AC_VO. CWVomin = 2^gEdcaVoCwmin -1
 *
 * Related: If gEnableEdcaParams is set to 1, params gEdcaVoCwmin etc
 * are aplicable
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_EDCA_VO_CWMIN_VALUE_NAME      "gEdcaVoCwmin"
#define CFG_EDCA_VO_CWMIN_VALUE_MIN       (0x0)
#define CFG_EDCA_VO_CWMIN_VALUE_MAX       (15)
#define CFG_EDCA_VO_CWMIN_VALUE_DEFAULT   (2)

/*
 * <ini>
 * gEdcaViCwmin - Set Cwmin value for EDCA_AC_VI
 * @Min: 0x0
 * @Max: 15
 * @Default: 3
 *
 * This ini is used to set default value for EDCA_AC_VI
 * Cwmin value for EDCA_AC_VI. CWVimin = 2^gEdcaViCwmin -1
 *
 * Related: If gEnableEdcaParams is set to 1, params gEdcaVoCwmin
 * etc are aplicable
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_EDCA_VI_CWMIN_VALUE_NAME      "gEdcaViCwmin"
#define CFG_EDCA_VI_CWMIN_VALUE_MIN       (0x0)
#define CFG_EDCA_VI_CWMIN_VALUE_MAX       (15)
#define CFG_EDCA_VI_CWMIN_VALUE_DEFAULT   (3)

/*
 * <ini>
 * gEdcaBkCwmin - Set Cwmin value for EDCA_AC_BK
 * @Min: 0x0
 * @Max: 15
 * @Default: 4
 *
 * This ini is used to set default Cwmin value for EDCA_AC_BK
 * Cwmin value for EDCA_AC_BK. CWBkmin = 2^gEdcaBkCwmin -1
 *
 * Related: If gEnableEdcaParams is set to 1, params gEdcaVoCwmin
 * etc are aplicable
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 * </ini>
 */

#define CFG_EDCA_BK_CWMIN_VALUE_NAME      "gEdcaBkCwmin"
#define CFG_EDCA_BK_CWMIN_VALUE_MIN       (0x0)
#define CFG_EDCA_BK_CWMIN_VALUE_MAX       (15)
#define CFG_EDCA_BK_CWMIN_VALUE_DEFAULT   (4)

/*
 * <ini>
 * gEdcaBeCwmin - Set Cwmin value for EDCA_AC_BE
 * @Min: 0x0
 * @Max: 15
 * @Default: 4
 *
 * This ini is used to set default Cwmin value for EDCA_AC_BE
 * Cwmin value for EDCA_AC_BE. CWBemin = 2^gEdcaBeCwmin
 *
 * Related: If gEnableEdcaParams is set to 1, params gEdcaVoCwmin
 * etc are aplicable
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_EDCA_BE_CWMIN_VALUE_NAME      "gEdcaBeCwmin"
#define CFG_EDCA_BE_CWMIN_VALUE_MIN       (0x0)
#define CFG_EDCA_BE_CWMIN_VALUE_MAX       (15)
#define CFG_EDCA_BE_CWMIN_VALUE_DEFAULT   (4)

/*
 * <ini>
 * gEdcaVoCwmax - Set Cwmax value for EDCA_AC_VO
 * @Min: 0
 * @Max: 15
 * @Default: 3
 *
 * This ini is used to set default Cwmax value for EDCA_AC_VO
 * Cwmax value for EDCA_AC_VO. CWVomax = 2^gEdcaVoCwmax -1
 *
 * Related: If gEnableEdcaParams is set to 1, params gEdcaVoCwmin
 * etc are aplicable
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_EDCA_VO_CWMAX_VALUE_NAME      "gEdcaVoCwmax"
#define CFG_EDCA_VO_CWMAX_VALUE_MIN       (0)
#define CFG_EDCA_VO_CWMAX_VALUE_MAX       (15)
#define CFG_EDCA_VO_CWMAX_VALUE_DEFAULT   (3)

/*
 * <ini>
 * gEdcaViCwmax - Set Cwmax value for EDCA_AC_VI
 * @Min: 0
 * @Max: 15
 * @Default: 4
 *
 * This ini is used to set default Cwmax value for EDCA_AC_VI
 * Cwmax value for EDCA_AC_VI. CWVimax = 2^gEdcaViCwmax -1
 *
 * Related: If gEnableEdcaParams is set to 1, params gEdcaVoCwmin
 * etc are aplicable
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_EDCA_VI_CWMAX_VALUE_NAME      "gEdcaViCwmax"
#define CFG_EDCA_VI_CWMAX_VALUE_MIN       (0)
#define CFG_EDCA_VI_CWMAX_VALUE_MAX       (15)
#define CFG_EDCA_VI_CWMAX_VALUE_DEFAULT   (4)

/*
 * <ini>
 * gEdcaBkCwmax - Set Cwmax value for EDCA_AC_BK
 * @Min: 0
 * @Max: 15
 * @Default: 10
 *
 * This ini is used to set default Cwmax value for EDCA_AC_BK
 * Cwmax value for EDCA_AC_BK. CWBkmax = 2^gEdcaBkCwmax -1
 *
 * Related: If gEnableEdcaParams is set to 1, params gEdcaVoCwmin
 * etc are aplicable
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_EDCA_BK_CWMAX_VALUE_NAME      "gEdcaBkCwmax"
#define CFG_EDCA_BK_CWMAX_VALUE_MIN       (0)
#define CFG_EDCA_BK_CWMAX_VALUE_MAX       (15)
#define CFG_EDCA_BK_CWMAX_VALUE_DEFAULT   (10)

/*
 * <ini>
 * gEdcaBeCwmax - Set Cwmax value for EDCA_AC_BE
 * @Min: 0
 * @Max: 15
 * @Default: 10
 *
 * This ini is used to set default Cwmax value for EDCA_AC_BE
 * Cwmax value for EDCA_AC_BE. CWBemax = 2^gEdcaBeCwmax -1
 *
 * Related: If gEnableEdcaParams is set to 1, params gEdcaVoCwmin
 * etc are aplicable
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_EDCA_BE_CWMAX_VALUE_NAME      "gEdcaBeCwmax"
#define CFG_EDCA_BE_CWMAX_VALUE_MIN       (0)
#define CFG_EDCA_BE_CWMAX_VALUE_MAX       (15)
#define CFG_EDCA_BE_CWMAX_VALUE_DEFAULT   (10)

/*
 * <ini>
 * gEdcaVoAifs - Set Aifs value for EDCA_AC_VO
 * @Min: 0
 * @Max: 15
 * @Default: 2
 *
 * This ini is used to set default Aifs value for EDCA_AC_VO
 *
 * Related: If gEnableEdcaParams is set to 1, params gEdcaVoCwmin
 * etc are aplicable
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_EDCA_VO_AIFS_VALUE_NAME       "gEdcaVoAifs"
#define CFG_EDCA_VO_AIFS_VALUE_MIN        (0)
#define CFG_EDCA_VO_AIFS_VALUE_MAX        (15)
#define CFG_EDCA_VO_AIFS_VALUE_DEFAULT    (2)

/*
 * <ini>
 * gEdcaViAifs - Set Aifs value for EDCA_AC_VI
 * @Min: 0
 * @Max: 15
 * @Default: 2
 *
 * This ini is used to set default Aifs value for EDCA_AC_VI
 *
 * Related: If gEnableEdcaParams is set to 1, params gEdcaVoCwmin
 * etc are aplicable
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_EDCA_VI_AIFS_VALUE_NAME       "gEdcaViAifs"
#define CFG_EDCA_VI_AIFS_VALUE_MIN        (0)
#define CFG_EDCA_VI_AIFS_VALUE_MAX        (15)
#define CFG_EDCA_VI_AIFS_VALUE_DEFAULT    (2)

/*
 * <ini>
 * gEdcaBkAifs - Set Aifs value for EDCA_AC_BK
 * @Min: 0
 * @Max: 15
 * @Default: 7
 *
 * This ini is used to set default Aifs value for EDCA_AC_BK
 *
 * Related: If gEnableEdcaParams is set to 1, params gEdcaVoCwmin
 * etc are aplicable
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_EDCA_BK_AIFS_VALUE_NAME       "gEdcaBkAifs"
#define CFG_EDCA_BK_AIFS_VALUE_MIN        (0)
#define CFG_EDCA_BK_AIFS_VALUE_MAX        (15)
#define CFG_EDCA_BK_AIFS_VALUE_DEFAULT    (7)

/*
 * <ini>
 * gEdcaBeAifs - Set Aifs value for EDCA_AC_BE
 * @Min: 0
 * @Max: 15
 * @Default: 3
 *
 * This ini is used to set default Aifs value for EDCA_AC_BE
 *
 * Related: If gEnableEdcaParams is set to 1, params gEdcaVoCwmin
 * etc are aplicable
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_EDCA_BE_AIFS_VALUE_NAME       "gEdcaBeAifs"
#define CFG_EDCA_BE_AIFS_VALUE_MIN        (0)
#define CFG_EDCA_BE_AIFS_VALUE_MAX        (15)
#define CFG_EDCA_BE_AIFS_VALUE_DEFAULT    (3)

/*
 * <ini>
 * gEnableTxSchedWrrVO - Set TX sched parameters for VO
 * @Default:
 *
 * This key is mapping to VO defined in data path module through
 * OL_TX_SCHED_WRR_ADV_CAT_CFG_SPEC. The user can tune the
 * WRR TX sched parameters such as skip, credit, limit, credit, disc for VO.
 * e.g., gEnableTxSchedWrrVO = 10, 9, 8, 1, 8
 */
#define CFG_ENABLE_TX_SCHED_WRR_VO_NAME     "gEnableTxSchedWrrVO"
#define CFG_ENABLE_TX_SCHED_WRR_VO_DEFAULT  ""

/*
 * <ini>
 * gEnableTxSchedWrrVI - Set TX sched parameters for VI
 * @Default:
 *
 * This key is mapping to VI defined in data path module through
 * OL_TX_SCHED_WRR_ADV_CAT_CFG_SPEC. The user can tune the
 * WRR TX sched parameters such as skip, credit, limit, credit, disc for VI.
 * e.g., gEnableTxSchedWrrVI = 10, 9, 8, 1, 8
 */
#define CFG_ENABLE_TX_SCHED_WRR_VI_NAME     "gEnableTxSchedWrrVI"
#define CFG_ENABLE_TX_SCHED_WRR_VI_DEFAULT  ""

/*
 * <ini>
 * gEnableTxSchedWrrBE - Set TX sched parameters for BE
 * @Default:
 *
 * This key is mapping to BE defined in data path module through
 * OL_TX_SCHED_WRR_ADV_CAT_CFG_SPEC. The user can tune the
 * WRR TX sched parameters such as skip, credit, limit, credit, disc for BE.
 * e.g., gEnableTxSchedWrrBE = 10, 9, 8, 1, 8
 */
#define CFG_ENABLE_TX_SCHED_WRR_BE_NAME     "gEnableTxSchedWrrBE"
#define CFG_ENABLE_TX_SCHED_WRR_BE_DEFAULT  ""

/*
 * <ini>
 * gEnableTxSchedWrrBK - Set TX sched parameters for BK
 * @Default:
 *
 * This key is mapping to BK defined in data path module through
 * OL_TX_SCHED_WRR_ADV_CAT_CFG_SPEC. The user can tune the
 * WRR TX sched parameters such as skip, credit, limit, credit, disc for BK.
 * e.g., gEnableTxSchedWrrBK = 10, 9, 8, 1, 8
 */
#define CFG_ENABLE_TX_SCHED_WRR_BK_NAME     "gEnableTxSchedWrrBK"
#define CFG_ENABLE_TX_SCHED_WRR_BK_DEFAULT  ""

#ifdef CONFIG_DP_TRACE
/*
 * Enable/disable DPTRACE
 * Enabling this might have performace impact.
 */
#define CFG_ENABLE_DP_TRACE		"enable_dp_trace"
#define CFG_ENABLE_DP_TRACE_MIN		(0)
#define CFG_ENABLE_DP_TRACE_MAX	(1)
#define CFG_ENABLE_DP_TRACE_DEFAULT	(1)

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
/*
 * Config DPTRACE
 * The sequence of params is important. If some param is missing, defaults are
 * considered.
 * Param 1: Enable/Disable DP Trace live mode (uint8_t)
 * Param 2: DP Trace live mode high bandwidth thresh.(uint8_t)
 *         (packets/second) beyond which DP Trace is disabled. Decimal Val.
 *          MGMT, DHCP, EAPOL, ARP pkts are not counted. ICMP and Data are.
 * Param 3: Default Verbosity (0-4)
 * Param 4: Proto Bitmap (uint8_t). Decimal Value.
 *          (decimal 62 = 0x3e)
 * e.g., to disable live mode, use the following param in the ini file.
 * gDptraceConfig = 0
 * e.g., to enable dptrace live mode and set the thresh as 6,
 * use the following param in the ini file.
 * gDptraceConfig = 1, 6
 */
#define CFG_ENABLE_DP_TRACE_CONFIG		"gDptraceConfig"
#define CFG_ENABLE_DP_TRACE_CONFIG_DEFAULT	"1, 6, 2, 126"
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
 * g_bug_on_reinit_failure  - Enable/Disable bug on reinit
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to debug ssr reinit failure issues by raising vos bug so
 * dumps can be collected.
 * g_bug_on_reinit_failure = 0 wlan driver will only recover after driver
 * unload and load
 * g_bug_on_reinit_failure = 1 raise vos bug to collect dumps
 *
 * Related: gEnableSSR
 *
 * Supported Feature: SSR
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_BUG_ON_REINIT_FAILURE_NAME     "g_bug_on_reinit_failure"
#define CFG_BUG_ON_REINIT_FAILURE_MIN      (0)
#define CFG_BUG_ON_REINIT_FAILURE_MAX      (1)
#define CFG_BUG_ON_REINIT_FAILURE_DEFAULT  (1)

/*
 * <ini>
 * gSub20ChannelWidth - Control sub 20 channel width (5/10 Mhz)
 * @Min: 0
 * @Max: 2
 * @Default: 0
 *
 * This ini is used to set the sub 20 channel width.
 * gSub20ChannelWidth=0: indicates do not use Sub 20 MHz bandwidth
 * gSub20ChannelWidth=1: Bring up SAP/STA in 5 MHz bandwidth
 * gSub20ChannelWidth=2: Bring up SAP/STA in 10 MHz bandwidth
 *
 * Related: None
 *
 * Supported Feature: 5/10 Mhz channel width support
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_SUB_20_CHANNEL_WIDTH_NAME              "gSub20ChannelWidth"
#define CFG_SUB_20_CHANNEL_WIDTH_MIN               (WLAN_SUB_20_CH_WIDTH_NONE)
#define CFG_SUB_20_CHANNEL_WIDTH_MAX               (WLAN_SUB_20_CH_WIDTH_10)
#define CFG_SUB_20_CHANNEL_WIDTH_DEFAULT           (WLAN_SUB_20_CH_WIDTH_NONE)

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
 * <ini>
 * rx_mode - Control to decide rx mode for packet procesing
 *
 * @Min: 0
 * @Max: (CFG_ENABLE_RX_THREAD | CFG_ENABLE_RPS | CFG_ENABLE_NAPI | \
 *	 CFG_ENABLE_DYNAMIC_RPS)
 * Some possible configurations:
 * rx_mode=0 - Uses tasklets for bottom half
 * CFG_ENABLE_NAPI (rx_mode=4) - Uses NAPI for bottom half
 * CFG_ENABLE_RX_THREAD | CFG_ENABLE_NAPI (rx_mode=5) - NAPI for bottom half,
 * rx_thread for stack. Single threaded.
 * CFG_ENABLE_DP_RX_THREAD | CFG_ENABLE_NAPI (rx_mode=10) - NAPI for bottom
 * half, dp_rx_thread for stack processing. Supports multiple rx threads.
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_RX_MODE_NAME     "rx_mode"
#define CFG_RX_MODE_MIN      (0)
#define CFG_RX_MODE_MAX      (CFG_ENABLE_RX_THREAD | CFG_ENABLE_RPS | \
				 CFG_ENABLE_NAPI | CFG_ENABLE_DYNAMIC_RPS | \
				 CFG_ENABLE_DP_RX_THREADS)
#ifdef MDM_PLATFORM
#define CFG_RX_MODE_DEFAULT  (0)
#elif defined(HELIUMPLUS)
#define CFG_RX_MODE_DEFAULT  CFG_ENABLE_NAPI
#elif defined(QCA_WIFI_QCA6290_11AX)
#define CFG_RX_MODE_DEFAULT  (CFG_ENABLE_DP_RX_THREADS | CFG_ENABLE_NAPI)
#else
#define CFG_RX_MODE_DEFAULT  (CFG_ENABLE_RX_THREAD | CFG_ENABLE_NAPI)
#endif

/*
 * <ini>
 * num_dp_rx_threads - Control to set the number of dp rx threads
 *
 * @Min: 1
 * @Max: 4
 * @Default: 1
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_NUM_DP_RX_THREADS_NAME     "num_dp_rx_threads"
#define CFG_NUM_DP_RX_THREADS_MIN      (1)
#define CFG_NUM_DP_RX_THREADS_MAX      (4)
#define CFG_NUM_DP_RX_THREADS_DEFAULT  (1)

/*
 * <ini>
 * ce_service_max_yield_time - Control to set ce service max yield time (in us)
 *
 * @Min: 500
 * @Max: 10000
 * @Default: 10000
 *
 * This ini is used to set ce service max yield time (in us)
 *
 * Supported Feature: NAPI
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_CE_SERVICE_MAX_YIELD_TIME_NAME     "ce_service_max_yield_time"
#define CFG_CE_SERVICE_MAX_YIELD_TIME_MIN      (500)
#define CFG_CE_SERVICE_MAX_YIELD_TIME_MAX      (10000)
#define CFG_CE_SERVICE_MAX_YIELD_TIME_DEFAULT  (10000)

/*
 * <ini>
 * ce_service_max_rx_ind_flush - Control to set ce service max rx ind flush
 *
 * @Min: 1
 * @Max: 32
 * @Default: 1
 *
 * This ini is used to set ce service max rx ind flush
 *
 * Supported Feature: NAPI
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_CE_SERVICE_MAX_RX_IND_FLUSH_NAME     "ce_service_max_rx_ind_flush"
#define CFG_CE_SERVICE_MAX_RX_IND_FLUSH_MIN      (1)
#define CFG_CE_SERVICE_MAX_RX_IND_FLUSH_MAX      (32)
#define CFG_CE_SERVICE_MAX_RX_IND_FLUSH_DEFAULT  (32)

/*
 * <ini>
 * NAPI_CPU_AFFINITY_MASK - CPU mask to affine NAPIs
 *
 * @Min: 0
 * @Max: 0xFF
 * @Default: 0
 *
 * This ini is used to set NAPI IRQ CPU affinity
 *
 * Supported Feature: NAPI
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_NAPI_CE_CPU_MASK_NAME	"NAPI_CPU_AFFINITY_MASK"
#define CFG_NAPI_CE_CPU_MASK_MIN	(0)
#define CFG_NAPI_CE_CPU_MASK_MAX	(0xFF)
#define CFG_NAPI_CE_CPU_MASK_DEFAULT	(0)

/*
 * <ini>
 * RX_THREAD_CPU_AFFINITY_MASK - CPU mask to affine Rx_thread
 *
 * @Min: 0
 * @Max: 0xFF
 * @Default: 0
 *
 * This ini is used to set Rx_thread CPU affinity
 *
 * Supported Feature: Rx_thread
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_RX_THREAD_CPU_MASK_NAME	"RX_THREAD_CPU_AFFINITY_MASK"
#define CFG_RX_THREAD_CPU_MASK_MIN	(0)
#define CFG_RX_THREAD_CPU_MASK_MAX	(0xFF)
#define CFG_RX_THREAD_CPU_MASK_DEFAULT	(0)

/* List of RPS CPU maps for different rx queues registered by WLAN driver
 * Ref - Kernel/Documentation/networking/scaling.txt
 * RPS CPU map for a particular RX queue, selects CPU(s) for bottom half
 * processing of RX packets. For example, for a system with 4 CPUs,
 * 0xe: Use CPU1 - CPU3 and donot use CPU0.
 * 0x0: RPS is disabled, packets are processed on the interrupting CPU.
.*
 * WLAN driver registers NUM_TX_QUEUES queues for tx and rx each during
 * alloc_netdev_mq. Hence, we need to have a cpu mask for each of the rx queues.
 *
 * For example, if the NUM_TX_QUEUES is 4, a sample WLAN ini entry may look like
 * rpsRxQueueCpuMapList=a b c d
 * For a 4 CPU system (CPU0 - CPU3), this implies:
 * 0xa - (1010) use CPU1, CPU3 for rx queue 0
 * 0xb - (1011) use CPU0, CPU1 and CPU3 for rx queue 1
 * 0xc - (1100) use CPU2, CPU3 for rx queue 2
 * 0xd - (1101) use CPU0, CPU2 and CPU3 for rx queue 3

 * In practice, we may want to avoid the cores which are heavily loaded.
 */

/* Name of the ini file entry to specify RPS map for different RX queus */
#define CFG_RPS_RX_QUEUE_CPU_MAP_LIST_NAME         "rpsRxQueueCpuMapList"

/* Default value of rpsRxQueueCpuMapList. Different platforms may have
 * different configurations for NUM_TX_QUEUES and # of cpus, and will need to
 * configure an appropriate value via ini file. Setting default value to 'e' to
 * avoid use of CPU0 (since its heavily used by other system processes) by rx
 * queue 0, which is currently being used for rx packet processing.
 */
#define CFG_RPS_RX_QUEUE_CPU_MAP_LIST_DEFAULT      "e"

/* Maximum length of string used to hold a list of cpu maps for various rx
 * queues. Considering a 16 core system with 5 rx queues, a RPS CPU map
 * list may look like -
 * rpsRxQueueCpuMapList = ffff ffff ffff ffff ffff
 * (all 5 rx queues can be processed on all 16 cores)
 * max string len = 24 + 1(for '\0'). Considering 30 to be on safe side.
 */
#define CFG_RPS_RX_QUEUE_CPU_MAP_LIST_LEN 30

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
 * g_mark_sap_indoor_as_disable - Enable/Disable Indoor channel
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to mark the Indoor channel as
 * disable when SAP start and revert it on SAP stop,
 * so SAP will not turn on indoor channel and
 * sta will not scan/associate and roam on indoor
 * channels.
 *
 * Related: If g_mark_sap_indoor_as_disable set, turn the
 * indoor channels to disable and update Wiphy & fw.
 *
 * Supported Feature: SAP/STA
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_MARK_INDOOR_AS_DISABLE_NAME     "g_mark_sap_indoor_as_disable"
#define CFG_MARK_INDOOR_AS_DISABLE_MIN      (0)
#define CFG_MARK_INDOOR_AS_DISABLE_MAX      (1)
#define CFG_MARK_INDOOR_AS_DISABLE_DEFAULT  (0)

/*
 * Force softap to 11n, when gSapForce11NFor11AC is set to 1 from ini
 * despite of hostapd.conf request for 11ac
 */
#define CFG_SAP_FORCE_11N_FOR_11AC_NAME    "gSapForce11NFor11AC"
#define CFG_SAP_FORCE_11N_FOR_11AC_MIN     (0)
#define CFG_SAP_FORCE_11N_FOR_11AC_MAX     (1)
#define CFG_SAP_FORCE_11N_FOR_11AC_DEFAULT (0)

#define CFG_GO_FORCE_11N_FOR_11AC_NAME    "gGoForce11NFor11AC"
#define CFG_GO_FORCE_11N_FOR_11AC_MIN     (0)
#define CFG_GO_FORCE_11N_FOR_11AC_MAX     (1)
#define CFG_GO_FORCE_11N_FOR_11AC_DEFAULT (0)

/*
 * sap tx leakage threshold
 * customer can set this value from 100 to 1000 which means
 * sap tx leakage threshold is -10db to -100db
 */
#define CFG_SAP_TX_LEAKAGE_THRESHOLD_NAME    "gsap_tx_leakage_threshold"
#define CFG_SAP_TX_LEAKAGE_THRESHOLD_MIN     (100)
#define CFG_SAP_TX_LEAKAGE_THRESHOLD_MAX     (1000)
#define CFG_SAP_TX_LEAKAGE_THRESHOLD_DEFAULT (310)


/*
 * Enable filtering of replayed multicast packets
 * In a typical infrastructure setup, it is quite normal to receive
 * replayed multicast packets. These packets may cause more harm than
 * help if not handled properly. Providing a configuration option
 * to enable filtering of such packets
 */
#define CFG_FILTER_MULTICAST_REPLAY_NAME    "enable_multicast_replay_filter"
#define CFG_FILTER_MULTICAST_REPLAY_MIN      (0)
#define CFG_FILTER_MULTICAST_REPLAY_MAX      (1)
#define CFG_FILTER_MULTICAST_REPLAY_DEFAULT  (1)

/* Optimize channel avoidance indication coming from firmware */
#define CFG_OPTIMIZE_CA_EVENT_NAME       "goptimize_chan_avoid_event"
#define CFG_OPTIMIZE_CA_EVENT_DISABLE    (0)
#define CFG_OPTIMIZE_CA_EVENT_ENABLE     (1)
#define CFG_OPTIMIZE_CA_EVENT_DEFAULT    (0)

/*
 * <ini>
 * fw_timeout_crash - Enable/Disable BUG ON
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to Trigger host crash when firmware fails to send the
 * response to host
 * fw_timeout_crash = 0 Disabled
 * fw_timeout_crash = 1 Trigger host crash
 *
 * Related: None
 *
 * Supported Feature: SSR
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_CRASH_FW_TIMEOUT_NAME       "fw_timeout_crash"
#define CFG_CRASH_FW_TIMEOUT_DISABLE    (0)
#define CFG_CRASH_FW_TIMEOUT_ENABLE     (1)
#define CFG_CRASH_FW_TIMEOUT_DEFAULT    (1)

/*
 * <ini>
 * rx_wakelock_timeout - Amount of time to hold wakelock for RX unicast packets
 * @Min: 0
 * @Max: 100
 * @Default: 50
 *
 * This ini item configures the amount of time, in milliseconds, that the driver
 * should prevent system power collapse after receiving an RX unicast packet.
 * A conigured value of 0 disables the RX Wakelock feature completely.
 *
 * Related: None.
 *
 * Supported Feature: RX Wakelock
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_RX_WAKELOCK_TIMEOUT_NAME     "rx_wakelock_timeout"
#define CFG_RX_WAKELOCK_TIMEOUT_DEFAULT  (50)
#define CFG_RX_WAKELOCK_TIMEOUT_MIN      (0)
#define CFG_RX_WAKELOCK_TIMEOUT_MAX      (100)

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

#define CFG_ENABLE_5G_BAND_PREF_NAME             "enable_5g_band_pref"
#define CFG_ENABLE_5G_BAND_PREF_MIN              (0)
#define CFG_ENABLE_5G_BAND_PREF_MAX              (1)
#define CFG_ENABLE_5G_BAND_PREF_DEFAULT          (0)

/*
 * <ini>
 * 5g_rssi_boost_threshold - A_band_boost_threshold above which 5 GHz is favored.
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
#define CFG_5G_RSSI_BOOST_THRESHOLD_NAME         "5g_rssi_boost_threshold"
#define CFG_5G_RSSI_BOOST_THRESHOLD_MIN          (-55)
#define CFG_5G_RSSI_BOOST_THRESHOLD_MAX          (-70)
#define CFG_5G_RSSI_BOOST_THRESHOLD_DEFAULT      (-60)

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
#define CFG_5G_RSSI_BOOST_FACTOR_NAME            "5g_rssi_boost_factor"
#define CFG_5G_RSSI_BOOST_FACTOR_MIN             (0)
#define CFG_5G_RSSI_BOOST_FACTOR_MAX             (2)
#define CFG_5G_RSSI_BOOST_FACTOR_DEFAULT         (1)

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
#define CFG_5G_MAX_RSSI_BOOST_NAME               "5g_max_rssi_boost"
#define CFG_5G_MAX_RSSI_BOOST_MIN                (0)
#define CFG_5G_MAX_RSSI_BOOST_MAX                (20)
#define CFG_5G_MAX_RSSI_BOOST_DEFAULT            (10)

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
#define CFG_5G_RSSI_PENALIZE_THRESHOLD_NAME      "5g_rssi_penalize_threshold"
#define CFG_5G_RSSI_PENALIZE_THRESHOLD_MIN       (-65)
#define CFG_5G_RSSI_PENALIZE_THRESHOLD_MAX       (-80)
#define CFG_5G_RSSI_PENALIZE_THRESHOLD_DEFAULT   (-70)

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
#define CFG_5G_RSSI_PENALIZE_FACTOR_NAME         "5g_rssi_penalize_factor"
#define CFG_5G_RSSI_PENALIZE_FACTOR_MIN          (0)
#define CFG_5G_RSSI_PENALIZE_FACTOR_MAX          (2)
#define CFG_5G_RSSI_PENALIZE_FACTOR_DEFAULT      (1)

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
#define CFG_5G_MAX_RSSI_PENALIZE_NAME            "5g_max_rssi_penalize"
#define CFG_5G_MAX_RSSI_PENALIZE_MIN             (0)
#define CFG_5G_MAX_RSSI_PENALIZE_MAX             (20)
#define CFG_5G_MAX_RSSI_PENALIZE_DEFAULT         (10)

/*
 * <ini>
 * g_max_sched_scan_plan_int - pno sched max scan plan interval.
 * @Min: 1
 * @Max: 7200
 * @Default: 3600
 *
 * This ini is used to set max sched scan plan interval for pno scan
 * (value in seconds).
 *
 * Related: gPNOScanSupport
 *
 * Supported Feature: PNO scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_MAX_SCHED_SCAN_PLAN_INT_NAME       "g_max_sched_scan_plan_int"
#define CFG_MAX_SCHED_SCAN_PLAN_INT_MIN        (1)
#define CFG_MAX_SCHED_SCAN_PLAN_INT_MAX        (7200)
#define CFG_MAX_SCHED_SCAN_PLAN_INT_DEFAULT    (3600)

/*
 * <ini>
 * g_max_sched_scan_plan_iterations - pno sched max scan plan iterations.
 * @Min: 1
 * @Max: 100
 * @Default: 10
 *
 * This ini is used to set max sched scan plan iterations for pno scan
 * (value in seconds).
 *
 * Related: gPNOScanSupport
 *
 * Supported Feature: PNO scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_MAX_SCHED_SCAN_PLAN_ITRNS_NAME   "g_max_sched_scan_plan_iterations"
#define CFG_MAX_SCHED_SCAN_PLAN_ITRNS_MIN     (1)
#define CFG_MAX_SCHED_SCAN_PLAN_ITRNS_MAX     (100)
#define CFG_MAX_SCHED_SCAN_PLAN_ITRNS_DEFAULT (10)

/**
 * enum hdd_wext_control - knob for wireless extensions
 * @hdd_wext_disabled - interface is completely disabled. An access
 *      control error log will be generated for each attempted use.
 * @hdd_wext_deprecated - interface is available but should not be
 *      used. An access control warning log will be generated for each
 *      use.
 * @hdd_wext_enabled - interface is available without restriction. No
 *      access control logs will be generated.
 *
 * enum hdd_wext_control is used to enable coarse grained control on
 * wireless extensions ioctls. This control is used by configuration
 * item private_wext_control.
 *
 */
enum hdd_wext_control {
	hdd_wext_disabled = 0,
	hdd_wext_deprecated = 1,
	hdd_wext_enabled = 2,
};

/*
 * <ini>
 * private_wext_control - Private wireless extensions control
 * @Min: 0
 * @Max: 2
 * @Default: 1
 *
 * Values are per enum hdd_wext_control.
 *
 * This ini is used to control access to private wireless extensions
 * ioctls SIOCIWFIRSTPRIV (0x8BE0) thru SIOCIWLASTPRIV (0x8BFF). The
 * functionality provided by some of these ioctls has been superceeded
 * by cfg80211 (either standard commands or vendor commands), but many
 * of the private ioctls do not have a cfg80211-based equivalent, so
 * by default support for these ioctls is deprecated.
 *
 * Related: None
 *
 * Supported Feature: All
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_PRIVATE_WEXT_CONTROL_NAME           "private_wext_control"
#define CFG_PRIVATE_WEXT_CONTROL_MIN            (hdd_wext_disabled)
#define CFG_PRIVATE_WEXT_CONTROL_MAX            (hdd_wext_enabled)
#define CFG_PRIVATE_WEXT_CONTROL_DEFAULT        (hdd_wext_deprecated)

/*
 * <ini>
 * gPowerUsage - Preferred Power Usage
 * @Min: Min
 * @Max: Max
 * @Default: Mod
 *
 * This ini is used to set the preferred power usage
 *
 * Related: None
 *
 * Supported Feature: Power Save
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_POWER_USAGE_NAME                   "gPowerUsage"
#define CFG_POWER_USAGE_MIN                    "Min"
#define CFG_POWER_USAGE_MAX                    "Max"
#define CFG_POWER_USAGE_DEFAULT                "Mod"

/*
 * <ini>
 * gEnableImps - Enable/Disable IMPS
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable/Disable IMPS(IdleModePowerSave) Mode
 *
 * Related: None
 *
 * Supported Feature: Power Save
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ENABLE_IMPS_NAME                   "gEnableImps"
#define CFG_ENABLE_IMPS_MIN                    (0)
#define CFG_ENABLE_IMPS_MAX                    (1)
#define CFG_ENABLE_IMPS_DEFAULT                (1)

/*
 * <ini>
 * gEnableBmps - Enable/Disable BMPS
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable/Disable BMPS(BeaconModePowerSave) Mode
 *
 * Related: None
 *
 * Supported Feature: Power Save
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ENABLE_PS_NAME                     "gEnableBmps"
#define CFG_ENABLE_PS_MIN                      (0)
#define CFG_ENABLE_PS_MAX                      (1)
#define CFG_ENABLE_PS_DEFAULT                  (1)

/*
 * <ini>
 * gAutoBmpsTimerValue - Set Auto BMPS Timer value
 * @Min: 0
 * @Max: 120
 * @Default: 90
 *
 * This ini is used to set Auto BMPS Timer value in seconds
 *
 * Related: gEnableBmps
 *
 * Supported Feature: Power Save
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_AUTO_PS_ENABLE_TIMER_NAME          "gAutoBmpsTimerValue"
#define CFG_AUTO_PS_ENABLE_TIMER_MIN           (0)
#define CFG_AUTO_PS_ENABLE_TIMER_MAX           (120)
#define CFG_AUTO_PS_ENABLE_TIMER_DEFAULT       (90)

#ifdef WLAN_ICMP_DISABLE_PS
/*
 * <ini>
 * gIcmpDisablePsValue - Set ICMP packet disable power save value
 * @Min:     0
 * @Max:     10000
 * @Default: 5000
 *
 * This ini is used to set ICMP packet disable power save value in
 * millisecond.
 *
 * Related: gEnableBmps
 *
 * Supported Feature: Power Save
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ICMP_DISABLE_PS_NAME               "gIcmpDisablePsValue"
#define CFG_ICMP_DISABLE_PS_MIN                (0)
#define CFG_ICMP_DISABLE_PS_MAX                (10000)
#define CFG_ICMP_DISABLE_PS_DEFAULT            (5000)
#endif

/*
 * <ini>
 * gBmpsMinListenInterval - Set BMPS Minimum Listen Interval
 * @Min: 1
 * @Max: 65535
 * @Default: 1
 *
 * This ini is used to set BMPS Minimum Listen Interval. If gPowerUsage
 * is set "Min", this INI need to be set.
 *
 * Related: gEnableBmps, gPowerUsage
 *
 * Supported Feature: Power Save
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_BMPS_MINIMUM_LI_NAME               "gBmpsMinListenInterval"
#define CFG_BMPS_MINIMUM_LI_MIN                (1)
#define CFG_BMPS_MINIMUM_LI_MAX                (65535)
#define CFG_BMPS_MINIMUM_LI_DEFAULT            (1)

/*
 * <ini>
 * gBmpsMaxListenInterval - Set BMPS Maximum Listen Interval
 * @Min: 1
 * @Max: 65535
 * @Default: 1
 *
 * This ini is used to set BMPS Maximum Listen Interval. If gPowerUsage
 * is set "Max", this INI need to be set.
 *
 * Related: gEnableBmps, gPowerUsage
 *
 * Supported Feature: Power Save
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_BMPS_MAXIMUM_LI_NAME               "gBmpsMaxListenInterval"
#define CFG_BMPS_MAXIMUM_LI_MIN                (1)
#define CFG_BMPS_MAXIMUM_LI_MAX                (65535)
#define CFG_BMPS_MAXIMUM_LI_DEFAULT            (1)

#ifdef FEATURE_RUNTIME_PM
/*
 * <ini>
 * gRuntimePM - enable runtime suspend
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable runtime_suspend
 *
 * Related: None
 *
 * Supported Feature: Power Save
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ENABLE_RUNTIME_PM                  "gRuntimePM"
#define CFG_ENABLE_RUNTIME_PM_MIN              (0)
#define CFG_ENABLE_RUNTIME_PM_MAX              (1)
#define CFG_ENABLE_RUNTIME_PM_DEFAULT          (0)
#endif

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
 * gTimerMultiplier - Scale QDF timers by this value
 * @Min: 1
 * @Max: 0xFFFFFFFF
 * @Default: 1 (100 for emulation)
 *
 * To assist in debugging emulation setups, scale QDF timers by this factor.
 *
 * @E.g.
 *	# QDF timers expire in real time
 *	gTimerMultiplier=1
 *	# QDF timers expire after 100 times real time
 *	gTimerMultiplier=100
 *
 * Related: N/A
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_TIMER_MULTIPLIER_NAME	"gTimerMultiplier"
#define CFG_TIMER_MULTIPLIER_MIN	(1)
#define CFG_TIMER_MULTIPLIER_MAX	(0xFFFFFFFF)
#ifdef QCA_WIFI_NAPIER_EMULATION
#define CFG_TIMER_MULTIPLIER_DEFAULT	(100)
#else
#define CFG_TIMER_MULTIPLIER_DEFAULT	(1)
#endif

/*
 * For vendor specific IE, Probe Req OUI types and sub types which are
 * to be white listed are specified in gProbeReqOUIs in the following
 * example format - gProbeReqOUIs=AABBCCDD EEFF1122
 */

/*
 * <ini>
 * gProbeReqOUIs - Used to specify vendor specific OUIs
 * @Default: Empty string
 *
 * This ini is used to include the specified OUIs in vendor specific IE
 * of probe request.
 *
 * Related: Need to enable g_enable_probereq_whitelist_ies and
 * vendor specific IE should be set in g_probe_req_ie_bitmap_6.
 *
 * Supported Feature: Probe request ie whitelisting
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_PROBE_REQ_OUI_NAME    "gProbeReqOUIs"
#define CFG_PROBE_REQ_OUI_DEFAULT ""
/* End of probe request IE whitelisting feature ini params */

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
 * gEnableTxOrphan- Enable/Disable orphaning of Tx packets
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable/disable orphaning of Tx packets.
 *
 * Related: None
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_TX_ORPHAN_ENABLE_NAME    "gEnableTxOrphan"
#define CFG_TX_ORPHAN_ENABLE_DEFAULT (0)
#define CFG_TX_ORPHAN_ENABLE_MIN     (0)
#define CFG_TX_ORPHAN_ENABLE_MAX     (1)

/*
 * <ini>
 * gItoRepeatCount - sets ito repeated count
 * @Min: 0
 * @Max: 5
 * @Default: 0
 *
 * This ini sets the ito count in FW
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ITO_REPEAT_COUNT_NAME "gItoRepeatCount"
#define CFG_ITO_REPEAT_COUNT_MIN        (0)
#define CFG_ITO_REPEAT_COUNT_MAX        (5)
#define CFG_ITO_REPEAT_COUNT_DEFAULT    (0)

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

#ifdef WLAN_FEATURE_PACKET_FILTERING
/*
 * <ini>
 * g_enable_packet_filter_bitmap - Enable Packet filters before going into
 * suspend mode
 * @Min: 0
 * @Max: 63
 * @Default: 0
 * Below is the Detailed bit map of the Filters
 * bit-0 : IPv6 multicast
 * bit-1 : IPv4 multicast
 * bit-2 : IPv4 broadcast
 * bit-3 : XID - Exchange station Identification packet, solicits the
 * identification of the receiving station
 * bit-4 : STP - Spanning Tree Protocol, builds logical loop free topology
 * bit-5 : DTP/LLC/CDP
 * DTP - Dynamic Trunking Protocol is used by Ciscoswitches to negotiate
 * whether an interconnection between two switches should be put into access or
 * trunk mode
 * LLC - Logical link control, used for multiplexing, flow & error control
 * CDP - Cisco Discovery Protocol packet contains information about the cisco
 * devices in the network
 *
 * This ini support to enable above mentioned packet filters
 * when target goes to suspend mode, clear those when resume
 *
 * Related: None
 *
 * Supported Feature: PACKET FILTERING
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_ENABLE_PACKET_FILTERS_NAME     "g_enable_packet_filter_bitmap"
#define CFG_ENABLE_PACKET_FILTERS_DEFAULT  (0)
#define CFG_ENABLE_PACKET_FILTERS_MIN      (0)
#define CFG_ENABLE_PACKET_FILTERS_MAX      (63)
#endif /* WLAN_FEATURE_PACKET_FILTERING */

/*
 * <ini>
 * gDfsBeaconTxEnhanced - beacon tx enhanced
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enhance dfs beacon tx
 *
 * Related: none
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_DFS_BEACON_TX_ENHANCED         "gDfsBeaconTxEnhanced"
#define CFG_DFS_BEACON_TX_ENHANCED_MIN     (0)
#define CFG_DFS_BEACON_TX_ENHANCED_MAX     (1)
#define CFG_DFS_BEACON_TX_ENHANCED_DEFAULT (0)

/*
 * <ini>
 * btm_offload_config - Configure BTM
 * @Min: 0x00000000
 * @Max: 0xFFFFFFFF
 * @Default: 0x00000000
 *
 * This ini is used to configure BTM
 *
 * Bit 0: Enable/Disable the BTM offload. Set this to 1 will
 * enable and 0 will disable BTM offload.
 *
 * BIT 2, 1: Action on non matching candidate with cache. If a BTM request
 * is received from AP then the candidate AP's may/may-not be present in
 * the firmware scan cache . Based on below config firmware will decide
 * whether to forward BTM frame to host or consume with firmware and proceed
 * with Roaming to candidate AP.
 * 00 scan and consume
 * 01 no scan and forward to host
 * 10, 11 reserved
 *
 * BIT 5, 4, 3: Roaming handoff decisions on multiple candidates match
 * 000 match if exact BSSIDs are found
 * 001 match if at least one top priority BSSID only
 * 010, 011, 100, 101, 110, 111 reserved
 *
 * BIT 6: Set this to 1 will send BTM query frame and 0 not sent.
 *
 * BIT 7-31: Reserved
 *
 * Supported Feature: STA
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_BTM_ENABLE_NAME      "btm_offload_config"
#define CFG_BTM_ENABLE_MIN       (0x00000000)
#define CFG_BTM_ENABLE_MAX       (0xffffffff)
#define CFG_BTM_ENABLE_DEFAULT   (0x00000001)

/*
 * <ini>
 * btm_solicited_timeout - timeout value for waiting BTM request
 * @Min: 1
 * @Max: 10000
 * @Default: 100
 *
 * This ini is used to configure timeout value for waiting BTM request.
 * Unit: millionsecond
 *
 * Supported Feature: STA
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_BTM_SOLICITED_TIMEOUT           "btm_solicited_timeout"
#define CFG_BTM_SOLICITED_TIMEOUT_MIN       (1)
#define CFG_BTM_SOLICITED_TIMEOUT_MAX       (10000)
#define CFG_BTM_SOLICITED_TIMEOUT_DEFAULT   (100)

/*
 * <ini>
 * btm_max_attempt_cnt - Maximum attempt for sending BTM query to ESS
 * @Min: 1
 * @Max: 0xFFFFFFFF
 * @Default: 3
 *
 * This ini is used to configure maximum attempt for sending BTM query to ESS.
 *
 * Supported Feature: STA
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_BTM_MAX_ATTEMPT_CNT           "btm_max_attempt_cnt"
#define CFG_BTM_MAX_ATTEMPT_CNT_MIN       (0x00000001)
#define CFG_BTM_MAX_ATTEMPT_CNT_MAX       (0xFFFFFFFF)
#define CFG_BTM_MAX_ATTEMPT_CNT_DEFAULT   (0x00000003)

/*
 * <ini>
 * sticky_time - Stick time after roaming to new AP by BTM
 * @Min: 0
 * @Max: 0x0000FFFF
 * @Default: 300
 *
 * This ini is used to configure Stick time after roaming to new AP by BTM.
 * Unit: seconds
 *
 * Supported Feature: STA
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_BTM_STICKY_TIME           "btm_sticky_time"
#define CFG_BTM_STICKY_TIME_MIN       (0x00000000)
#define CFG_BTM_STICKY_TIME_MAX       (0x0000FFFF)
#define CFG_BTM_STICKY_TIME_DEFAULT   (300)

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
 * wmi_wq_watchdog - Sets timeout period for wmi watchdog bite
 * @Min: 0
 * @Max: 30
 * @Default: 20
 *
 * This ini is used to set timeout period for wmi watchdog bite. If it is
 * 0 then wmi watchdog bite is disabled.
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_WMI_WQ_WATCHDOG          "wmi_wq_watchdog"
#define CFG_WMI_WQ_WATCHDOG_MIN      (0)
#define CFG_WMI_WQ_WATCHDOG_MAX      (30) /* 30s */
#define CFG_WMI_WQ_WATCHDOG_DEFAULT  (20) /* 20s */

/*
 * <ini>
 * gEnableDTIMSelectionDiversity - Enable/Disable chain
 * selection optimization for one chain dtim
 * @Min: 0
 * @Max: 30
 * @Default: 5
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_DTIM_SELECTION_DIVERSITY_NAME    "gEnableDTIMSelectionDiversity"
#define CFG_DTIM_SELECTION_DIVERSITY_MIN     (0)
#define CFG_DTIM_SELECTION_DIVERSITY_MAX     (30)
#define CFG_DTIM_SELECTION_DIVERSITY_DEFAULT (5)

/*
 * <ini>
 * channel_select_logic_conc - Set channel selection logic
 * for different concurrency combinations to DBS or inter band
 * MCC. Default is DBS for STA+STA and STA+P2P.
 * @Min: 0x00000000
 * @Max: 0xFFFFFFFF
 * @Default: 0x00000000
 *
 * 0 - inter-band MCC
 * 1 - DBS
 *
 * BIT 0: STA+STA
 * BIT 1: STA+P2P
 * BIT 2-31: Reserved
 *
 * Supported Feature: STA+STA, STA+P2P
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_CHANNEL_SELECT_LOGIC_CONC_NAME    "channel_select_logic_conc"
#define CFG_CHANNEL_SELECT_LOGIC_CONC_MIN     (0x00000000)
#define CFG_CHANNEL_SELECT_LOGIC_CONC_MAX     (0xFFFFFFFF)
#define CFG_CHANNEL_SELECT_LOGIC_CONC_DEFAULT (0x00000003)

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
 * Start of action oui inis
 *
 * To enable action oui feature, set gEnableActionOUI
 *
 * Each action oui is expected in the following format:
 * <Extension 1> <Extension 2> ..... <Extension 10> (maximum 10)
 *
 * whereas, each Extension is separated by space and have the following format:
 * <Token1> <Token2> <Token3> <Token4> <Token5> <Token6> <Token7> <Token8>
 * where each Token is a string of hexa-decimal digits and
 * following are the details about each token
 *
 * Token1 = OUI
 * Token2 = Data_Length
 * Token3 = Data
 * Token4 = Data_Mask
 * Token5 = Info_Presence_Bit
 * Token6 = MAC_Address
 * Token7 = Mac_Address Mask
 * Token8 = Capability
 *
 * <OUI> is mandatory and it can be either 3 or 5 bytes means 6 or 10
 * hexa-decimal characters
 * If the OUI and Data checks needs to be ignored, the oui FFFFFF
 * needs to be provided as OUI and bit 0 of Info_Presence_Bit should
 * be set to 0.
 *
 * <Data_Length> is mandatory field and should give length of
 * the <Data> if present else zero
 *
 * Presence of <Data> is controlled by <Data_Length>, if <Data_Length> is 0,
 * then <Data> is not expected else Data of the size Data Length bytes are
 * expected which means the length of Data string is 2 * Data Length,
 * since every byte constitutes two hexa-decimal characters.
 *
 * <Data_Mask> is mandatory if <Data> is present and length of the
 * Data mask string depends on the <Data Length>
 * If <Data Length> is 06, then length of Data Mask string is
 * 2 characters (represents 1 byte)
 * data_mask_length = ((Data_Length - (Data_Length % 8)) / 8) +
 *                    ((Data_Length % 8) ? 1 : 0)
 * and <Data_Mask> has to be constructed from left to right.
 *
 * Presence of <Mac_Address> and <Capability> is
 * controlled by <Info_Presence_Bit> which is mandatory
 * <Info_Presence_Bit> will give the information for
 *   OUI – bit 0 Should be set to 1
 *               Setting to 0 will ignore OUI and data check
 *   Mac Address present – bit 1
 *   NSS – bit 2
 *   HT check – bit 3
 *   VHT check – bit 4
 *   Band info – bit 5
 *   reserved – bit 6 (should always be zero)
 *   reserved – bit 7 (should always be zero)
 * and should be constructed from right to left (b7b6b5b4b3b2b1b0)
 *
 * <Mac_Address_Mask> for <Mac_Address> should be constructed from left to right
 *
 * <Capability> is 1 byte long and it contains the below info
 *   NSS – 4 bits starting from LSB (b0 – b3)
 *   HT enabled – bit 4
 *   VHT enabled – bit 5
 *   2G band – bit 6
 *   5G band – bit 7
 * and should be constructed from right to left (b7b6b5b4b3b2b1b0)
 * <Capability> is present if atleast one of the bit is set
 * from b2 - b6 in <Info_Presence_Bit>
 *
 * Example 1:
 *
 * OUI is 00-10-18, data length is 05 (hex form), data is 02-11-04-5C-DE and
 * need to consider first 3 bytes and last byte of data for comparision
 * mac-addr EE-1A-59-FE-FD-AF is present and first 3 bytes and last byte of
 * mac address should be considered for comparision
 * capability is not present
 * then action OUI for gActionOUIITOExtension is as follows:
 *
 * gActionOUIITOExtension=001018 05 0211045CDE E8 03 EE1A59FEFDAF E4
 *
 * data mask calculation in above example:
 * Data[0] = 02 ---- d0 = 1
 * Data[1] = 11 ---- d1 = 1
 * Data[2] = 04 ---- d2 = 1
 * Data[3] = 5C ---- d3 = 0
 * Data[4] = DE ---- d4 = 1
 * data_mask = d0d1d2d3d4 + append with zeros to complete 8-bit = 11101000 = E8
 *
 * mac mask calculation in above example:
 * mac_addr[0] = EE ---- m0 = 1
 * mac_addr[1] = 1A ---- m1 = 1
 * mac_addr[2] = 59 ---- m2 = 1
 * mac_addr[3] = FE ---- m3 = 0
 * mac_addr[4] = FD ---- m4 = 0
 * mac_addr[5] = AF ---- m5 = 1
 * mac_mask = m0m1m2m3m4m5 + append with zeros to complete 8-bit = 11100100 = E4
 *
 * Example 2:
 *
 * OUI is 00-10-18, data length is 00 and no Mac Address and capability
 *
 * gActionOUIITOExtension=001018 00 01
 *
 */

/*
 * <ini>
 * gEnableActionOUI - Enable/Disable action oui feature
 * @Min: 0 (disable)
 * @Max: 1 (enable)
 * @Default: 1 (enable)
 *
 * This ini is used to enable the action oui feature to control
 * mode of connection, connected AP's in-activity time, Tx rate etc.,
 *
 * Related: If gEnableActionOUI is set, then at least one of the following inis
 * must be set with the proper action oui extensions:
 * gActionOUIConnect1x1, gActionOUIITOExtension, gActionOUICCKM1X1
 *
 * Supported Feature: action ouis
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ENABLE_ACTION_OUI         "gEnableActionOUI"
#define CFG_ENABLE_ACTION_OUI_MIN     (0)
#define CFG_ENABLE_ACTION_OUI_MAX     (1)
#define CFG_ENABLE_ACTION_OUI_DEFAULT (1)

/*
 * <ini>
 * gActionOUIConnect1x1 - Used to specify action OUIs for 1x1 connection
 * @Default: 000C43 00 25 42 001018 06 02FFF02C0000 BC 25 42 001018 06 02FF040C0000 BC 25 42 00037F 00 35 6C
 * Note: User should strictly add new action OUIs at the end of this
 * default value.
 *
 * Default OUIs: (All values in Hex)
 * OUI 1 : 000C43
 *   OUI data Len : 00
 *   Info Mask : 25 - Check for NSS and Band
 *   Capabilities: 42 - NSS == 2 && Band == 2G
 * OUI 2 : 001018
 *   OUI data Len : 06
 *   OUI Data : 02FFF02C0000
 *   OUI data Mask: BC - 10111100
 *   Info Mask : 25 - Check for NSS and Band
 *   Capabilities: 42 - NSS == 2 && Band == 2G
 * OUI 3 : 001018
 *   OUI data Len : 06
 *   OUI Data : 02FF040C0000
 *   OUI data Mask: BC - 10111100
 *   Info Mask : 25 - Check for NSS and Band
 *   Capabilities: 42 - NSS == 2 && Band == 2G
 * OUI 4 : 00037F
 *   OUI data Len : 00
 *   Info Mask : 35 - Check for NSS, VHT Caps and Band
 *   Capabilities: 6C - (NSS == 3 or 4) && VHT Caps Preset && Band == 2G
 *
 * This ini is used to specify the AP OUIs with which only 1x1 connection
 * is allowed.
 *
 * Related: None
 *
 * Supported Feature: Action OUIs
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ACTION_OUI_CONNECT_1X1_NAME    "gActionOUIConnect1x1"
#define CFG_ACTION_OUI_CONNECT_1X1_DEFAULT "000C43 00 25 42 001018 06 02FFF02C0000 BC 25 42 001018 06 02FF040C0000 BC 25 42 00037F 00 35 6C"

/*
 * <ini>
 * gActionOUIITOExtension - Used to extend in-activity time for specified APs
 * @Default: 00037F 06 01010000FF7F FC 01 000AEB 02 0100 C0 01 000B86 03 010408 E0 01
 * Note: User should strictly add new action OUIs at the end of this
 * default value.
 *
 * Default OUIs: (All values in Hex)
 * OUI 1: 00037F
 *   OUI data Len: 06
 *   OUI Data: 01010000FF7F
 *   OUI data Mask: FC - 11111100
 *   Info Mask : 01 - only OUI present in Info mask
 *
 * OUI 2: 000AEB
 *   OUI data Len: 02
 *   OUI Data: 0100
 *   OUI data Mask: C0 - 11000000
 *   Info Mask : 01 - only OUI present in Info mask
 *
 * OUI 3: 000B86
 *   OUI data Len: 03
 *   OUI Data: 010408
 *   OUI data Mask: E0 - 11100000
 *   Info Mask : 01 - only OUI present in Info mask
 *
 * This ini is used to specify AP OUIs using which station's in-activity time
 * can be extended with the respective APs
 *
 * Related: None
 *
 * Supported Feature: Action OUIs
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ACTION_OUI_ITO_EXTENSION_NAME    "gActionOUIITOExtension"
#define CFG_ACTION_OUI_ITO_EXTENSION_DEFAULT "00037F 06 01010000FF7F FC 01 000AEB 02 0100 C0 01 000B86 03 010408 E0 01"

/*
 * <ini>
 * gActionOUICCKM1X1 - Used to specify action OUIs to control station's TX rates
 *
 * This ini is used to specify AP OUIs for which station's CCKM TX rates
 * should be 1x1 only.
 *
 * Related: None
 *
 * Supported Feature: Action OUIs
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ACTION_OUI_CCKM_1X1_NAME    "gActionOUICCKM1X1"
#define CFG_ACTION_OUI_CCKM_1X1_DEFAULT ""

/*
 * <ini>
 * gActionOUIITOAlternate - Used to specify action OUIs to have alternate ITO in
 * weak RSSI state
 *
 * This ini is used to specify AP OUIs for which the stations will have
 * alternate ITOs for the case when the RSSI is weak.
 *
 * Related: None
 *
 * Supported Feature: Action OUIs
 *
 * Usage: External
 *
 * </ini>
 */
 #define CFG_ACTION_OUI_ITO_ALTERNATE_NAME    "gActionOUIITOAlternate"
 #define CFG_ACTION_OUI_ITO_ALTERNATE_DEFAULT "001018 06 0202001c0000 FC 01"

/*
 * <ini>
 * gActionOUISwitchTo11nMode - Used to specify action OUIs for switching to 11n
 *
 * This ini is used to specify which AP for which the connection has to be
 * made in 2x2 mode with HT capabilities only and not VHT.
 *
 * Default OUIs: (All values in Hex)
 * OUI 1 : 00904C
 *   OUI data Len : 03
 *   OUI Data : 0418BF
 *   OUI data Mask: E0 - 11100000
 *   Info Mask : 21 - Check for Band
 *   Capabilities: 40 - Band == 2G
 *
 * Related: None
 *
 * Supported Feature: Action OUIs
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ACTION_OUI_SWITCH_TO_11N_MODE_NAME    "gActionOUISwitchTo11nMode"
#define CFG_ACTION_OUI_SWITCH_TO_11N_MODE_DEFAULT "00904C 03 0418BF E0 21 40"

/*
 * <ini>
 * gActionOUIConnect1x1with1TxRxChain - Used to specify action OUIs for
 *					 1x1 connection with one Tx/Rx Chain
 * @Default:
 * Note: User should strictly add new action OUIs at the end of this
 * default value.
 *
 * Default OUIs: (All values in Hex)
 * OUI 1 : 001018
 *   OUI data Len : 06
 *   OUI Data : 02FFF0040000
 *   OUI data Mask: BC - 10111100
 *   Info Mask : 21 - Check for Band
 *   Capabilities: 40 - Band == 2G
 *
 * OUI 2 : 001018
 *   OUI data Len : 06
 *   OUI Data : 02FFF0050000
 *   OUI data Mask: BC - 10111100
 *   Info Mask : 21 - Check for Band
 *   Capabilities: 40 - Band == 2G
 *
 * OUI 3 : 001018
 *   OUI data Len : 06
 *   OUI Data : 02FFF4050000
 *   OUI data Mask: BC - 10111100
 *   Info Mask : 21 - Check for Band
 *   Capabilities: 40 - Band == 2G
 *
 * This ini is used to specify the AP OUIs with which only 1x1 connection
 * with one Tx/Rx Chain is allowed.
 *
 * Related: gEnableActionOUI
 *
 * Supported Feature: Action OUIs
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ACTION_OUI_CONNECT_1X1_WITH_1_CHAIN_NAME    "gActionOUIConnect1x1with1TxRxChain"
#define CFG_ACTION_OUI_CONNECT_1X1_WITH_1_CHAIN_DEFAULT "001018 06 02FFF0040000 BC 21 40 001018 06 02FFF0050000 BC 21 40 001018 06 02FFF4050000 BC 21 40"

 /* End of action oui inis */


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
 * enable_rtt_mac_randomization - Enable/Disable rtt mac randomization
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ENABLE_RTT_MAC_RANDOMIZATION_NAME    "enable_rtt_mac_randomization"
#define CFG_ENABLE_RTT_MAC_RANDOMIZATION_MIN     (0)
#define CFG_ENABLE_RTT_MAC_RANDOMIZATION_MAX     (1)
#define CFG_ENABLE_RTT_MAC_RANDOMIZATION_DEFAULT (0)

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
 * gNumVdevs - max number of VDEVs supported
 *
 * @Min: 0x1
 * @Max: 0x4
 * @Default: CFG_TGT_NUM_VDEV
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_NUM_VDEV_ENABLE_NAME      "gNumVdevs"
#define CFG_NUM_VDEV_ENABLE_MIN       (0x1)
#define CFG_NUM_VDEV_ENABLE_MAX       (0x4)
#define CFG_NUM_VDEV_ENABLE_DEFAULT   (CFG_TGT_NUM_VDEV)

#ifdef MWS_COEX
/*
 * <ini>
 * gMwsCoex4gQuickTdm - Bitmap to control MWS-COEX 4G quick FTDM policy
 * @Min: 0x00000000
 * @Max: 0xFFFFFFFF
 * @Default: 0x00000000
 *
 * It is a 32 bit value such that the various bits represent as below:
 * Bit-0 : 0 - Don't allow quick FTDM policy (Default)
 *        1 - Allow quick FTDM policy
 * Bit 1-31 : reserved for future use
 *
 * It is used to enable or disable MWS-COEX 4G (LTE) Quick FTDM
 *
 * Usage: Internal
 *
 * </ini>
 */

#define CFG_MWS_COEX_4G_QUICK_FTDM_NAME      "gMwsCoex4gQuickTdm"
#define CFG_MWS_COEX_4G_QUICK_FTDM_MIN       (0x00000000)
#define CFG_MWS_COEX_4G_QUICK_FTDM_MAX       (0xFFFFFFFF)
#define CFG_MWS_COEX_4G_QUICK_FTDM_DEFAULT   (0x00000000)

/*
 * <ini>
 * gMwsCoex5gnrPwrLimit - Bitmap to set MWS-COEX 5G-NR power limit
 * @Min: 0x00000000
 * @Max: 0xFFFFFFFF
 * @Default: 0x00000000
 *
 * It is a 32 bit value such that the various bits represent as below:
 * Bit-0 : Don't apply user specific power limit,
 *        use internal power limit (Default)
 * Bit 1-2 : Invalid value (Ignored)
 * Bit 3-21 : Apply the specified value as the external power limit, in dBm
 * Bit 22-31 : Invalid value (Ignored)
 *
 * It is used to set MWS-COEX 5G-NR power limit
 *
 * Usage: Internal
 *
 * </ini>
 */

#define CFG_MWS_COEX_5G_NR_PWR_LIMIT_NAME      "gMwsCoex5gnrPwrLimit"
#define CFG_MWS_COEX_5G_NR_PWR_LIMIT_MIN       (0x00000000)
#define CFG_MWS_COEX_5G_NR_PWR_LIMIT_MAX       (0xFFFFFFFF)
#define CFG_MWS_COEX_5G_NR_PWR_LIMIT_DEFAULT   (0x00000000)
#endif

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
 * Type declarations
 */

struct hdd_config {
	/* Bitmap to track what is explicitly configured */
	DECLARE_BITMAP(bExplicitCfg, MAX_CFG_INI_ITEMS);

	/* Config parameters */
#ifdef WLAN_NUD_TRACKING
	bool enable_nud_tracking;
#endif
	bool enable_connected_scan;
	uint8_t OperatingChannel;
	bool ShortSlotTimeEnabled;
	bool Is11dSupportEnabled;
	bool Is11hSupportEnabled;
	bool fSupplicantCountryCodeHasPriority;
	char PowerUsageControl[4];
	bool fIsImpsEnabled;
	bool is_ps_enabled;
	uint32_t auto_bmps_timer_val;
	uint32_t icmp_disable_ps_val;
	uint32_t nBmpsMaxListenInterval;
	uint32_t nBmpsMinListenInterval;
	enum hdd_dot11_mode dot11Mode;
	uint32_t nChannelBondingMode24GHz;
	bool override_ht20_40_24g;
	uint32_t nChannelBondingMode5GHz;
	uint32_t MaxRxAmpduFactor;
	uint32_t ScanResultAgeCount;
	uint8_t nRssiCatGap;
	bool fIsShortPreamble;
	struct qdf_mac_addr IbssBssid;
	uint32_t AdHocChannel5G;
	uint32_t AdHocChannel24G;
	uint8_t intfAddrMask;
	struct qdf_mac_addr intfMacAddr[QDF_MAX_CONCURRENCY_PERSONA];

	bool apUapsdEnabled;
	bool apRandomBssidEnabled;
	bool apProtEnabled;
	uint16_t apProtection;
	bool apOBSSProtEnabled;
	bool apDisableIntraBssFwd;
	enum station_keepalive_method sta_keepalive_method;
	uint8_t nTxPowerCap;    /* In dBm */
	bool allow_tpc_from_ap;
	uint8_t disablePacketFilter;
	bool fRrmEnable;
	uint16_t nRrmRandnIntvl;
	/* length includes separator */
	char rm_capability[3 * DOT11F_IE_RRMENABLEDCAP_MAX_LEN];

#ifdef FEATURE_WLAN_DYNAMIC_CVM
	/* Bitmap for operating voltage corner mode */
	uint32_t vc_mode_cfg_bitmap;
#endif

#ifdef MWS_COEX
	/* Bitmap for MWS-COEX 4G Quick FTDM */
	uint32_t mws_coex_4g_quick_tdm;

	/* Bitmap for MWS-COEX 5G-NR power limit */
	uint32_t mws_coex_5g_nr_pwr_limit;
#endif

	uint16_t nNeighborScanPeriod;
	uint16_t neighbor_scan_min_period;
	uint8_t nNeighborLookupRssiThreshold;
	uint8_t delay_before_vdev_stop;
	uint8_t nOpportunisticThresholdDiff;
	uint8_t nRoamRescanRssiDiff;
	uint8_t neighborScanChanList[WNI_CFG_VALID_CHANNEL_LIST_LEN];
	uint16_t nNeighborScanMinChanTime;
	uint16_t nNeighborScanMaxChanTime;
	uint16_t nNeighborResultsRefreshPeriod;
	uint16_t nEmptyScanRefreshPeriod;
	uint8_t nRoamBmissFirstBcnt;
	uint8_t nRoamBmissFinalBcnt;
	uint8_t nRoamBeaconRssiWeight;
	uint32_t nhi_rssi_scan_max_count;
	uint32_t nhi_rssi_scan_rssi_delta;
	uint32_t nhi_rssi_scan_delay;
	int32_t nhi_rssi_scan_rssi_ub;

	/* Additional Handoff params */
	uint16_t nVccRssiTrigger;
	uint32_t nVccUlMacLossThreshold;
	uint32_t nPassiveMaxChnTime;    /* in units of milliseconds */
	uint32_t nActiveMaxChnTime;     /* in units of milliseconds */
	uint32_t scan_probe_repeat_time;
	uint32_t scan_num_probes;

	uint32_t nInitialDwellTime;     /* in units of milliseconds */
	bool initial_scan_no_dfs_chnl;

	uint32_t nPassiveMinChnTimeConc;        /* in units of milliseconds */
	uint32_t nPassiveMaxChnTimeConc;        /* in units of milliseconds */
	uint32_t nActiveMinChnTimeConc; /* in units of milliseconds */
	uint32_t nActiveMaxChnTimeConc; /* in units of milliseconds */
	uint32_t nRestTimeConc; /* in units of milliseconds */
	/* In units of milliseconds */
	uint32_t       min_rest_time_conc;
	/* In units of milliseconds */
	uint32_t       idle_time_conc;

	uint8_t nRssiFilterPeriod;
	uint8_t fMaxLIModulatedDTIM;

	bool mcc_rts_cts_prot_enable;
	bool mcc_bcast_prob_resp_enable;
	uint8_t wow_data_inactivity_timeout;

	/* WMM QoS Configuration */
	enum hdd_wmm_user_mode WmmMode;
	bool b80211eIsEnabled;
	uint8_t UapsdMask;      /* what ACs to setup U-APSD for at assoc */
	uint32_t InfraUapsdViSrvIntv;
	uint32_t InfraUapsdViSuspIntv;
	uint32_t InfraUapsdBeSrvIntv;
	uint32_t InfraUapsdBeSuspIntv;
	uint32_t InfraUapsdBkSrvIntv;
	uint32_t InfraUapsdBkSuspIntv;
	bool isFastRoamIniFeatureEnabled;
	bool MAWCEnabled;
#ifdef FEATURE_WLAN_ESE
	uint32_t InfraInactivityInterval;
	bool isEseIniFeatureEnabled;
#endif
	bool isFastTransitionEnabled;
	uint8_t RoamRssiDiff;
	bool isWESModeEnabled;
	bool isRoamOffloadScanEnabled;
	bool bImplicitQosEnabled;

	/* default TSPEC parameters for AC_VI */
	enum sme_qos_wmm_dir_type InfraDirAcVi;
	uint16_t InfraNomMsduSizeAcVi;
	uint32_t InfraMeanDataRateAcVi;
	uint32_t InfraMinPhyRateAcVi;
	uint16_t InfraSbaAcVi;

	/* default TSPEC parameters for AC_BE */
	enum sme_qos_wmm_dir_type InfraDirAcBe;
	uint16_t InfraNomMsduSizeAcBe;
	uint32_t InfraMeanDataRateAcBe;
	uint32_t InfraMinPhyRateAcBe;
	uint16_t InfraSbaAcBe;

	/* default TSPEC parameters for AC_BK */
	enum sme_qos_wmm_dir_type InfraDirAcBk;
	uint16_t InfraNomMsduSizeAcBk;
	uint32_t InfraMeanDataRateAcBk;
	uint32_t InfraMinPhyRateAcBk;
	uint16_t InfraSbaAcBk;

	uint32_t DelayedTriggerFrmInt;

	char enableConcurrentSTA[CFG_CONCURRENT_IFACE_MAX_LEN];

#ifdef FEATURE_RUNTIME_PM
	bool runtime_pm;
#endif

#ifdef FEATURE_WLAN_RA_FILTERING
	uint16_t RArateLimitInterval;
#endif
#ifdef FEATURE_WLAN_SCAN_PNO
	bool PnoOffload;
#endif
	bool burstSizeDefinition;
	uint8_t tsInfoAckPolicy;

	bool AddTSWhenACMIsOff;

/* QDF Trace Control*/
	uint16_t qdf_trace_enable_wdi;
	uint16_t qdf_trace_enable_hdd;
	uint16_t qdf_trace_enable_sme;
	uint16_t qdf_trace_enable_pe;
	uint16_t qdf_trace_enable_pmc;
	uint16_t qdf_trace_enable_wma;
	uint16_t qdf_trace_enable_sys;
	uint16_t qdf_trace_enable_qdf;
	uint16_t qdf_trace_enable_sap;
	uint16_t qdf_trace_enable_hdd_sap;
	uint16_t qdf_trace_enable_bmi;
	uint16_t qdf_trace_enable_cfg;
	uint16_t qdf_trace_enable_txrx;
	uint16_t qdf_trace_enable_dp;
	uint16_t qdf_trace_enable_htc;
	uint16_t qdf_trace_enable_hif;
	uint16_t qdf_trace_enable_hdd_sap_data;
	uint16_t qdf_trace_enable_hdd_data;
	uint16_t qdf_trace_enable_epping;
	uint16_t qdf_trace_enable_qdf_devices;
	uint16_t qdf_trace_enable_wifi_pos;
	uint16_t qdf_trace_enable_nan;
	uint16_t qdf_trace_enable_regulatory;
	uint16_t qdf_trace_enable_cp_stats;
#ifdef ENABLE_MTRACE_LOG
	bool enable_mtrace;
#endif
	uint8_t enableBypass11d;
	uint8_t enableDFSChnlScan;
	uint8_t enable_dfs_pno_chnl_scan;
	uint8_t ShortGI40MhzEnable;
	enum hdd_link_speed_rpt_type reportMaxLinkSpeed;
	int32_t linkSpeedRssiHigh;
	int32_t linkSpeedRssiMid;
	int32_t linkSpeedRssiLow;
	bool nRoamPrefer5GHz;
	bool nRoamIntraBand;
	uint8_t nProbes;
	uint16_t nRoamScanHomeAwayTime;
	uint8_t enableMCC;
	uint8_t allowMCCGODiffBI;
	uint8_t thermalMitigationEnable;
	uint32_t throttlePeriod;
	uint32_t throttle_dutycycle_level0;
	uint32_t throttle_dutycycle_level1;
	uint32_t throttle_dutycycle_level2;
	uint32_t throttle_dutycycle_level3;
#if defined(CONFIG_HL_SUPPORT) && defined(QCA_BAD_PEER_TX_FLOW_CL)
	bool bad_peer_txctl_enable;
	uint32_t bad_peer_txctl_prd;
	uint32_t bad_peer_txctl_txq_lmt;
	uint32_t bad_peer_tgt_backoff;
	uint32_t bad_peer_tgt_report_prd;
	uint32_t bad_peer_cond_ieee80211b;
	uint32_t bad_peer_delta_ieee80211b;
	uint32_t bad_peer_pct_ieee80211b;
	uint32_t bad_peer_tput_ieee80211b;
	uint32_t bad_peer_limit_ieee80211b;
	uint32_t bad_peer_cond_ieee80211ag;
	uint32_t bad_peer_delta_ieee80211ag;
	uint32_t bad_peer_pct_ieee80211ag;
	uint32_t bad_peer_tput_ieee80211ag;
	uint32_t bad_peer_limit_ieee80211ag;
	uint32_t bad_peer_cond_ieee80211n;
	uint32_t bad_peer_delta_ieee80211n;
	uint32_t bad_peer_pct_ieee80211n;
	uint32_t bad_peer_tput_ieee80211n;
	uint32_t bad_peer_limit_ieee80211n;
	uint32_t bad_peer_cond_ieee80211ac;
	uint32_t bad_peer_delta_ieee80211ac;
	uint32_t bad_peer_pct_ieee80211ac;
	uint32_t bad_peer_tput_ieee80211ac;
	uint32_t bad_peer_limit_ieee80211ac;
#endif
	uint8_t vhtChannelWidth;
	uint8_t vhtRxMCS;
	uint8_t vhtTxMCS;
	bool enableTxBF;
	bool enable_subfee_vendor_vhtie;
	bool enable_txbf_sap_mode;
	bool enable_vht20_mcs9;
	uint8_t txBFCsnValue;
	bool enable_su_tx_bformer;
	uint8_t vhtRxMCS2x2;
	uint8_t vhtTxMCS2x2;
	bool enable2x2;
	uint32_t vdev_type_nss_2g;
	uint32_t vdev_type_nss_5g;
	bool enableMuBformee;
	bool enableVhtpAid;
	bool enableVhtGid;
	bool enableTxBFin20MHz;
	uint8_t enableAmpduPs;
	uint8_t enableHtSmps;
	uint8_t htSmps;
	bool enableFirstScan2GOnly;
	bool enableRxSTBC;
	bool enableTxSTBC;
	uint8_t enable_tx_ldpc;
	uint8_t enable_rx_ldpc;
	bool prevent_link_down;
	uint8_t scanAgingTimeout;
	uint8_t disableLDPCWithTxbfAP;
	uint8_t enableMCCAdaptiveScheduler;
	bool enableSSR;
	bool enable_data_stall_det;
	bool enableVhtFor24GHzBand;
	bool enable_sap_vendor_vht;
	bool bFastRoamInConIniFeatureEnabled;
	bool fEnableSNRMonitoring;
	/*PNO related parameters */
#ifdef FEATURE_WLAN_SCAN_PNO
	bool configPNOScanSupport;
	uint32_t configPNOScanTimerRepeatValue;
	uint32_t pno_slow_scan_multiplier;
#endif
	uint8_t max_amsdu_num;
	uint8_t nSelect5GHzMargin;
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

	bool enable_ip_tcp_udp_checksum_offload;
	bool enablefwprint;
	uint8_t enable_fw_log;
	uint8_t fVhtAmpduLenExponent;
	uint32_t vhtMpduLen;
	uint32_t IpaConfig;
	bool IpaClkScalingEnable;
	uint32_t IpaDescSize;
	uint32_t IpaHighBandwidthMbps;
	uint32_t IpaMediumBandwidthMbps;
	uint32_t IpaLowBandwidthMbps;
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
	uint32_t WlanMccToSccSwitchMode;
#endif
#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
	uint32_t WlanAutoShutdown;
#endif
	uint8_t disableDFSChSwitch;
	uint8_t enableDFSMasterCap;
	uint32_t TxPower2g;
	uint32_t TxPower5g;
	uint32_t gEnableDebugLog;
	bool fDfsPhyerrFilterOffload;
	uint8_t gSapPreferredChanLocation;
	uint8_t gDisableDfsJapanW53;
	bool gEnableOverLapCh;
	bool fRegChangeDefCountry;
#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL
	uint32_t TxFlowLowWaterMark;
	uint32_t TxFlowHighWaterMarkOffset;
	uint32_t TxFlowMaxQueueDepth;
	uint32_t TxLbwFlowLowWaterMark;
	uint32_t TxLbwFlowHighWaterMarkOffset;
	uint32_t TxLbwFlowMaxQueueDepth;
	uint32_t TxHbwFlowLowWaterMark;
	uint32_t TxHbwFlowHighWaterMarkOffset;
	uint32_t TxHbwFlowMaxQueueDepth;
#endif /* QCA_LL_LEGACY_TX_FLOW_CONTROL */
#ifdef QCA_LL_TX_FLOW_CONTROL_V2
	uint32_t TxFlowStopQueueThreshold;
	uint32_t TxFlowStartQueueOffset;
#endif
	bool advertiseConcurrentOperation;
	bool enableMemDeepSleep;
	bool enable_cck_tx_fir_override;

	uint8_t allowDFSChannelRoam;

#ifndef REMOVE_PKT_LOG
	bool enablePacketLog;
#endif

#ifdef MSM_PLATFORM
	uint32_t busBandwidthHighThreshold;
	uint32_t busBandwidthMediumThreshold;
	uint32_t busBandwidthLowThreshold;
	uint32_t busBandwidthComputeInterval;
	uint32_t enable_tcp_delack;
	bool     enable_tcp_limit_output;
	uint32_t enable_tcp_adv_win_scale;
	uint32_t tcpDelackThresholdHigh;
	uint32_t tcpDelackThresholdLow;
	uint32_t tcp_tx_high_tput_thres;
	uint32_t tcp_delack_timer_count;
	u8  periodic_stats_disp_time;
#endif /* MSM_PLATFORM */

	uint8_t enableFwModuleLogLevel[FW_MODULE_LOG_LEVEL_STRING_LENGTH];

	uint8_t gMaxConcurrentActiveSessions;

	uint8_t ignoreCAC;

	/* Flag to indicate crash inject enabled or not */
	bool crash_inject_enabled;

	bool enable_sap_mandatory_chan_list;

	int32_t dfsRadarPriMultiplier;
	uint8_t reorderOffloadSupport;

	uint32_t IpaUcTxBufCount;
	uint32_t IpaUcTxBufSize;
	uint32_t IpaUcRxIndRingCount;
	uint32_t IpaUcTxPartitionBase;
#ifdef WLAN_LOGGING_SOCK_SVC_ENABLE
	/* WLAN Logging */
	bool wlan_logging_enable;
	bool wlan_logging_to_console;
#endif /* WLAN_LOGGING_SOCK_SVC_ENABLE */

#ifdef WLAN_FEATURE_LPSS
	bool enable_lpass_support;
#endif
	bool enableSelfRecovery;
#ifdef FEATURE_WLAN_FORCE_SAP_SCC
	uint8_t SapSccChanAvoidance;
#endif /* FEATURE_WLAN_FORCE_SAP_SCC */

	bool enable_sap_suspend;

	bool gEnableDeauthToDisassocMap;
#ifdef DHCP_SERVER_OFFLOAD
	uint8_t dhcpServerIP[IPADDR_STRING_LENGTH];
#endif /* DHCP_SERVER_OFFLOAD */
	bool enable_mac_spoofing;
	uint8_t conc_custom_rule1;
	uint8_t conc_custom_rule2;
	uint8_t is_sta_connection_in_5gz_enabled;
	uint32_t sta_miracast_mcc_rest_time_val;
	bool is_ramdump_enabled;
#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
	bool sap_channel_avoidance;
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */
	uint8_t sap_11ac_override;
	uint8_t go_11ac_override;
	uint8_t sap_dot11mc;
	uint8_t prefer_non_dfs_on_radar;
	uint8_t multicast_host_fw_msgs;
	uint8_t conc_system_pref;
	uint32_t dbs_selection_policy;
	uint32_t vdev_priority_list;
	bool tso_enable;
	bool lro_enable;
	bool gro_enable;
	bool flow_steering_enable;
	uint8_t max_msdus_per_rxinorderind;
	/* parameter for defer timer for enabling TDLS on p2p listen */
	uint32_t fine_time_meas_cap;
	uint8_t max_scan_count;
#ifdef WLAN_FEATURE_FASTPATH
	bool fastpath_enable;
#endif
	bool etsi13_srd_chan_in_master_mode;
	uint8_t rx_mode;
	uint8_t num_dp_rx_threads;
	uint32_t ce_service_max_yield_time;
	uint8_t ce_service_max_rx_ind_flush;
	uint32_t napi_cpu_affinity_mask;
	/* CPU affinity mask for rx_thread */
	uint32_t rx_thread_affinity_mask;
	uint8_t cpu_map_list[CFG_RPS_RX_QUEUE_CPU_MAP_LIST_LEN];
	bool ce_classify_enabled;
	uint32_t dual_mac_feature_disable;
	uint8_t dbs_scan_selection[CFG_DBS_SCAN_PARAM_LENGTH];
	uint32_t sta_sap_scc_on_dfs_chan;
	uint32_t sta_sap_scc_on_lte_coex_chan;
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
	uint8_t ht_mpdu_density;
#ifdef FEATURE_LFR_SUBNET_DETECTION
	bool enable_lfr_subnet_detection;
#endif
	uint8_t inform_bss_rssi_raw;
#ifdef WLAN_FEATURE_TSF
#ifdef WLAN_FEATURE_TSF_PLUS
	uint8_t tsf_ptp_options;
#endif /* WLAN_FEATURE_TSF_PLUS */
#endif
	uint32_t ho_delay_for_rx;
	uint32_t min_delay_btw_roam_scans;
	uint32_t roam_trigger_reason_bitmask;
	uint32_t roam_bg_scan_client_bitmap;
	bool enable_edca_params;
	uint32_t edca_vo_cwmin;
	uint32_t edca_vi_cwmin;
	uint32_t edca_bk_cwmin;
	uint32_t edca_be_cwmin;
	uint32_t edca_vo_cwmax;
	uint32_t edca_vi_cwmax;
	uint32_t edca_bk_cwmax;
	uint32_t edca_be_cwmax;
	uint32_t edca_vo_aifs;
	uint32_t edca_vi_aifs;
	uint32_t edca_bk_aifs;
	uint32_t edca_be_aifs;

	/* Tuning TX sched parameters for VO (skip credit limit credit disc) */
	uint8_t  tx_sched_wrr_vo[TX_SCHED_WRR_PARAM_STRING_LENGTH];
	/* Tuning TX sched parameters for VI (skip credit limit credit disc) */
	uint8_t  tx_sched_wrr_vi[TX_SCHED_WRR_PARAM_STRING_LENGTH];
	/* Tuning TX sched parameters for BE (skip credit limit credit disc) */
	uint8_t  tx_sched_wrr_be[TX_SCHED_WRR_PARAM_STRING_LENGTH];
	/* Tuning TX sched parameters for BK (skip credit limit credit disc) */
	uint8_t  tx_sched_wrr_bk[TX_SCHED_WRR_PARAM_STRING_LENGTH];

	bool enable_fatal_event;
	bool apf_enabled;
#ifdef CONFIG_DP_TRACE
	bool enable_dp_trace;
	uint8_t dp_trace_config[DP_TRACE_CONFIG_STRING_LENGTH];
#endif
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
	bool bug_on_reinit_failure;
	uint32_t iface_change_wait_time;
	/* parameter to control GTX */
	enum cfg_sub_20_channel_width enable_sub_20_channel_width;
	bool indoor_channel_support;
	/* control marking indoor channel passive to disable */
	bool force_ssc_disable_indoor_channel;
	/* parameter to force sap into 11n */
	bool sap_force_11n_for_11ac;
	bool go_force_11n_for_11ac;
	uint16_t sap_tx_leakage_threshold;
	bool multicast_replay_filter;
	bool goptimize_chan_avoid_event;
	bool fw_timeout_crash;
	uint32_t rx_wakelock_timeout;
	uint32_t max_sched_scan_plan_interval;
	uint32_t max_sched_scan_plan_iterations;
	enum hdd_wext_control private_wext_control;
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

	/* Probe Request multiple vendor OUIs */
	uint8_t probe_req_ouis[MAX_PRB_REQ_VENDOR_OUI_INI_LEN];
	uint32_t no_of_probe_req_ouis;
	uint32_t probe_req_voui[MAX_PROBE_REQ_OUIS];

	uint32_t timer_multiplier;
	uint8_t scan_backoff_multiplier;
	bool mawc_nlo_enabled;
	uint32_t mawc_nlo_exp_backoff_ratio;
	uint32_t mawc_nlo_init_scan_interval;
	uint32_t mawc_nlo_max_scan_interval;
	/* threshold of packet drops at which FW initiates disconnect */
	uint16_t pkt_err_disconn_th;
	bool is_force_1x1;
	enum pmo_auto_pwr_detect_failure_mode auto_pwr_save_fail_mode;
	uint8_t ito_repeat_count;
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
	int8_t rssi_thresh_offset_5g;
	uint32_t scan_11d_interval;
	bool is_bssid_hint_priority;
	uint16_t wlm_latency_enable;
	uint16_t wlm_latency_level;
	uint32_t wlm_latency_flags_normal;
	uint32_t wlm_latency_flags_moderate;
	uint32_t wlm_latency_flags_low;
	uint32_t wlm_latency_flags_ultralow;
#ifdef WLAN_FEATURE_PACKET_FILTERING
	uint8_t packet_filters_bitmap;
#endif
	uint8_t dfs_beacon_tx_enhanced;
	uint32_t btm_offload_config;
#ifdef WLAN_FEATURE_SAE
	bool is_sae_enabled;
#endif
	bool enable_dtim_selection_diversity;
	bool gcmp_enabled;
	bool is_11k_offload_supported;
	uint32_t btm_solicited_timeout;
	uint32_t btm_max_attempt_cnt;
	uint32_t btm_sticky_time;
	uint32_t num_vdevs;
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
	uint32_t channel_select_logic_conc;
	uint16_t wmi_wq_watchdog_timeout;
	bool enable_bt_chain_separation;
	uint8_t enable_tx_sch_delay;
	uint32_t enable_secondary_rate;
	HDD_GREEN_AP_CFG_FIELDS
	bool is_unit_test_framework_enabled;
	bool enable_ftopen;
	bool enable_rtt_mac_randomization;
	bool roam_force_rssi_trigger;
	bool enable_change_channel_bandwidth;
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
 * @enum hdd_wmm_user_mode - hdd WMM user mode
 *
 * Return: CSR WMM mode
 */
eCsrRoamWmmUserModeType hdd_to_csr_wmm_mode(enum hdd_wmm_user_mode mode);

/* Function declarations and documenation */
QDF_STATUS hdd_parse_config_ini(struct hdd_context *hdd_ctx);

/**
 * hdd_validate_prb_req_ie_bitmap - validates user input for ie bit map
 * @hdd_ctx: the pointer to hdd context
 *
 * This function checks whether user has entered valid probe request
 * ie bitmap and also verifies vendor ouis if vendor specific ie is set
 *
 * Return: status of verification
 *         true - valid input
 *         false - invalid input
 */
bool hdd_validate_prb_req_ie_bitmap(struct hdd_context *hdd_ctx);

/**
 * hdd_parse_probe_req_ouis - form ouis from ini gProbeReqOUIs
 * @hdd_ctx: the pointer to hdd context
 *
 * This function parses the ini string gProbeReqOUIs which needs be to in the
 * following format:
 * "<8 characters of [0-9] or [A-F]>space<8 characters from [0-9] etc.,"
 * example: "AABBCCDD 1122EEFF"
 * and the logic counts the number of OUIS and allocates the memory
 * for every valid OUI and is stored in struct hdd_context
 *
 * Return: status of parsing
 *         0 - success
 *         negative value - failure
 */
int hdd_parse_probe_req_ouis(struct hdd_context *hdd_ctx);

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

bool hdd_is_okc_mode_enabled(struct hdd_context *hdd_ctx);
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
