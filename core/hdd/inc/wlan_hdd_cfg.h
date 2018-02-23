/*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
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

struct hdd_context;

#define FW_MODULE_LOG_LEVEL_STRING_LENGTH  (512)
#define TX_SCHED_WRR_PARAM_STRING_LENGTH   (50)
#define TX_SCHED_WRR_PARAMS_NUM            (5)
#define CFG_ENABLE_RX_THREAD		(1 << 0)
#define CFG_ENABLE_RPS			(1 << 1)
#define CFG_ENABLE_NAPI			(1 << 2)

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
 * RTSThreshold - Will provide RTSThreshold
 * @Min: 0
 * @Max: 1048576
 * @Default: 2347
 *
 * This ini is used to set default RTSThreshold
 * If minimum value 0 is selectd then it will use always RTS
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_RTS_THRESHOLD_NAME                 "RTSThreshold"
#define CFG_RTS_THRESHOLD_MIN                  WNI_CFG_RTS_THRESHOLD_STAMIN     /* min is 0, meaning always use RTS. */
#define CFG_RTS_THRESHOLD_MAX                  WNI_CFG_RTS_THRESHOLD_STAMAX     /* max is the max frame size */
#define CFG_RTS_THRESHOLD_DEFAULT              WNI_CFG_RTS_THRESHOLD_STADEF

/*
 * <ini>
 * gFragmentationThreshold - It will set fragmentation threshold
 * @Min: 256
 * @Max: 8000
 * @Default: 8000
 *
 * This ini is used to indicate default fragmentation threshold
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_FRAG_THRESHOLD_NAME                "gFragmentationThreshold"
#define CFG_FRAG_THRESHOLD_MIN                 WNI_CFG_FRAGMENTATION_THRESHOLD_STAMIN
#define CFG_FRAG_THRESHOLD_MAX                 WNI_CFG_FRAGMENTATION_THRESHOLD_STAMAX
#define CFG_FRAG_THRESHOLD_DEFAULT             WNI_CFG_FRAGMENTATION_THRESHOLD_STADEF

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

#define CFG_11H_SUPPORT_ENABLED_NAME           "g11hSupportEnabled"
#define CFG_11H_SUPPORT_ENABLED_MIN            (0)
#define CFG_11H_SUPPORT_ENABLED_MAX            (1)
#define CFG_11H_SUPPORT_ENABLED_DEFAULT        (1)    /* Default is ON */

/* COUNTRY Code Priority */
#define CFG_COUNTRY_CODE_PRIORITY_NAME         "gCountryCodePriority"
#define CFG_COUNTRY_CODE_PRIORITY_MIN          (0)
#define CFG_COUNTRY_CODE_PRIORITY_MAX          (1)
#define CFG_COUNTRY_CODE_PRIORITY_DEFAULT      (0)

#define CFG_HEARTBEAT_THRESH_24_NAME           "gHeartbeat24"
#define CFG_HEARTBEAT_THRESH_24_MIN            WNI_CFG_HEART_BEAT_THRESHOLD_STAMIN
#define CFG_HEARTBEAT_THRESH_24_MAX            WNI_CFG_HEART_BEAT_THRESHOLD_STAMAX
#define CFG_HEARTBEAT_THRESH_24_DEFAULT        WNI_CFG_HEART_BEAT_THRESHOLD_STADEF

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

/*
 * <ini>
 * gEnableAdaptRxDrain - It will enable adapt received drain.
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to Configuration added to enable/disable CTS2SELF in
 * Adaptive RX drain feature.
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_ENABLE_ADAPT_RX_DRAIN_NAME     "gEnableAdaptRxDrain"
#define CFG_ENABLE_ADAPT_RX_DRAIN_MIN       WNI_CFG_ENABLE_ADAPT_RX_DRAIN_STAMIN
#define CFG_ENABLE_ADAPT_RX_DRAIN_MAX       WNI_CFG_ENABLE_ADAPT_RX_DRAIN_STAMAX
#define CFG_ENABLE_ADAPT_RX_DRAIN_DEFAULT   WNI_CFG_ENABLE_ADAPT_RX_DRAIN_STADEF

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
 * gFixedRate - It will provide fixed rate
 * @Min: 0
 * @Max: 44
 * @Default: 0
 *
 * This ini is used to set default fixed rate
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_FIXED_RATE_NAME                    "gFixedRate"
#define CFG_FIXED_RATE_MIN                     WNI_CFG_FIXED_RATE_STAMIN
#define CFG_FIXED_RATE_MAX                     WNI_CFG_FIXED_RATE_STAMAX
#define CFG_FIXED_RATE_DEFAULT                 WNI_CFG_FIXED_RATE_STADEF

/*
 * <ini>
 * gShortGI20Mhz - Short Guard Interval for HT20
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to set default short interval for HT20
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_SHORT_GI_20MHZ_NAME                "gShortGI20Mhz"
#define CFG_SHORT_GI_20MHZ_MIN                 WNI_CFG_SHORT_GI_20MHZ_STAMIN
#define CFG_SHORT_GI_20MHZ_MAX                 WNI_CFG_SHORT_GI_20MHZ_STAMAX
#define CFG_SHORT_GI_20MHZ_DEFAULT             WNI_CFG_SHORT_GI_20MHZ_STADEF

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
 * MAX value is choosen so that this type of scan can be
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
 * pmkidModes - Enable PMKID modes
 * This INI is used to enable PMKID feature options
 * @Min: 0
 * @Max: 3
 * @Default: 3
 *
 * Related: None
 *
 * Supported Feature: Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_PMKID_MODES_NAME                       "pmkidModes"
#define CFG_PMKID_MODES_MIN                        (0x0)
#define CFG_PMKID_MODES_MAX                        (0x3)
#define CFG_PMKID_MODES_DEFAULT                    (0x3)
#define CFG_PMKID_MODES_OKC                        (0x1)
#define CFG_PMKID_MODES_PMKSA_CACHING              (0x2)

/*
 * <ini>
 * gEnableDFSPnoChnlScan - Enable DFS PNO channel scan
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable or disable DFS channel
 * for PNO scan
 *
 * Related: None
 *
 * Supported Feature: Scan
 *
 * Usage: External
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
 * gPassiveMinChannelTime - Set min channel time for passive scan
 * @Min: 0
 * @Max: 10000
 * @Default: 60
 *
 * This ini is used to set minimum channel time in secs spent in
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
#define CFG_PASSIVE_MIN_CHANNEL_TIME_NAME      "gPassiveMinChannelTime"
#define CFG_PASSIVE_MIN_CHANNEL_TIME_MIN       (0)
#define CFG_PASSIVE_MIN_CHANNEL_TIME_MAX       (10000)
#define CFG_PASSIVE_MIN_CHANNEL_TIME_DEFAULT   (60)

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
 * gActiveMinChannelTime - Set min channel time for active scan
 * @Min: 0
 * @Max: 10000
 * @Default: 20
 *
 * This ini is used to set minimum channel time in secs spent in
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
#define CFG_ACTIVE_MIN_CHANNEL_TIME_NAME       "gActiveMinChannelTime"
#define CFG_ACTIVE_MIN_CHANNEL_TIME_MIN        (0)
#define CFG_ACTIVE_MIN_CHANNEL_TIME_MAX        (10000)
#define CFG_ACTIVE_MIN_CHANNEL_TIME_DEFAULT    (20)

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

#ifdef FEATURE_WLAN_EXTSCAN
/*
 * <ini>
 * gExtScanEnable - Enable external scan
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to control enabling of external scan
 * feature.
 *
 * Related: None
 *
 * Supported Feature: Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_EXTSCAN_ALLOWED_NAME                   "gExtScanEnable"
#define CFG_EXTSCAN_ALLOWED_MIN                    (0)
#define CFG_EXTSCAN_ALLOWED_MAX                    (1)
#define CFG_EXTSCAN_ALLOWED_DEF                    (1)

/*
 * <ini>
 * gExtScanPassiveMaxChannelTime - Set max channel time for external
 * passive scan
 * @Min: 0
 * @Max: 500
 * @Default: 110
 *
 * This ini is used to set maximum channel time  in secs spent in
 * external passive scan
 *
 * Related: None
 *
 * Supported Feature: Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_EXTSCAN_PASSIVE_MAX_CHANNEL_TIME_NAME      "gExtScanPassiveMaxChannelTime"
#define CFG_EXTSCAN_PASSIVE_MAX_CHANNEL_TIME_MIN       (0)
#define CFG_EXTSCAN_PASSIVE_MAX_CHANNEL_TIME_MAX       (500)
#define CFG_EXTSCAN_PASSIVE_MAX_CHANNEL_TIME_DEFAULT   (110)

/*
 * <ini>
 * gExtScanPassiveMinChannelTime - Set min channel time for external
 * passive scan
 * @Min: 0
 * @Max: 500
 * @Default: 60
 *
 * This ini is used to set minimum channel time in secs spent in
 * external passive scan
 *
 * Related: None
 *
 * Supported Feature: Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_EXTSCAN_PASSIVE_MIN_CHANNEL_TIME_NAME      "gExtScanPassiveMinChannelTime"
#define CFG_EXTSCAN_PASSIVE_MIN_CHANNEL_TIME_MIN       (0)
#define CFG_EXTSCAN_PASSIVE_MIN_CHANNEL_TIME_MAX       (500)
#define CFG_EXTSCAN_PASSIVE_MIN_CHANNEL_TIME_DEFAULT   (60)

/*
 * <ini>
 * gExtScanActiveMaxChannelTime - Set min channel time for external
 * active scan
 * @Min: 0
 * @Max: 110
 * @Default: 40
 *
 * This ini is used to set maximum channel time in secs spent in
 * external active scan
 *
 * Related: None
 *
 * Supported Feature: Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_EXTSCAN_ACTIVE_MAX_CHANNEL_TIME_NAME       "gExtScanActiveMaxChannelTime"
#define CFG_EXTSCAN_ACTIVE_MAX_CHANNEL_TIME_MIN        (0)
#define CFG_EXTSCAN_ACTIVE_MAX_CHANNEL_TIME_MAX        (110)
#define CFG_EXTSCAN_ACTIVE_MAX_CHANNEL_TIME_DEFAULT    (40)

/*
 * <ini>
 * gExtScanActiveMinChannelTime - Set min channel time for external
 * active scan
 * @Min: 0
 * @Max: 110
 * @Default: 20
 *
 * This ini is used to set minimum channel time in secs spent in
 * external active scan
 *
 * Related: None
 *
 * Supported Feature: Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_EXTSCAN_ACTIVE_MIN_CHANNEL_TIME_NAME       "gExtScanActiveMinChannelTime"
#define CFG_EXTSCAN_ACTIVE_MIN_CHANNEL_TIME_MIN        (0)
#define CFG_EXTSCAN_ACTIVE_MIN_CHANNEL_TIME_MAX        (110)
#define CFG_EXTSCAN_ACTIVE_MIN_CHANNEL_TIME_DEFAULT    (20)
#endif

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
#define CFG_EARLY_STOP_SCAN_ENABLE           "gEnableEarlyStopScan"
#define CFG_EARLY_STOP_SCAN_ENABLE_MIN       (0)
#define CFG_EARLY_STOP_SCAN_ENABLE_MAX       (1)
#define CFG_EARLY_STOP_SCAN_ENABLE_DEFAULT   (1)

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
#define CFG_EARLY_STOP_SCAN_MIN_THRESHOLD           "gEarlyStopScanMinThreshold"
#define CFG_EARLY_STOP_SCAN_MIN_THRESHOLD_MIN       (-80)
#define CFG_EARLY_STOP_SCAN_MIN_THRESHOLD_MAX       (-70)
#define CFG_EARLY_STOP_SCAN_MIN_THRESHOLD_DEFAULT   (-73)

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
#define CFG_EARLY_STOP_SCAN_MAX_THRESHOLD           "gEarlyStopScanMaxThreshold"
#define CFG_EARLY_STOP_SCAN_MAX_THRESHOLD_MIN       (-60)
#define CFG_EARLY_STOP_SCAN_MAX_THRESHOLD_MAX       (-40)
#define CFG_EARLY_STOP_SCAN_MAX_THRESHOLD_DEFAULT   (-43)

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
#define CFG_FIRST_SCAN_BUCKET_THRESHOLD_NAME      "gFirstScanBucketThreshold"
#define CFG_FIRST_SCAN_BUCKET_THRESHOLD_MIN       (-50)
#define CFG_FIRST_SCAN_BUCKET_THRESHOLD_MAX       (-30)
#define CFG_FIRST_SCAN_BUCKET_THRESHOLD_DEFAULT   (-30)

/*
 * <ini>
 * obss_active_dwelltime - Set obss active dwelltime
 * @Min: 5
 * @Max: 1000
 * @Default: 10
 *
 * This ini is used to set dwell time in secs for active
 * obss scan
 *
 * Related: None
 *
 * Supported Feature: Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_OBSS_HT40_SCAN_ACTIVE_DWELL_TIME_NAME    "obss_active_dwelltime"
#define CFG_OBSS_HT40_SCAN_ACTIVE_DWELL_TIME_MIN     (5)
#define CFG_OBSS_HT40_SCAN_ACTIVE_DWELL_TIME_MAX     (1000)
#define CFG_OBSS_HT40_SCAN_ACTIVE_DWELL_TIME_DEFAULT (10)

/*
 * <ini>
 * obss_passive_dwelltime - Set obss passive dwelltime
 * @Min: 10
 * @Max: 1000
 * @Default: 20
 *
 * This ini is used to set dwell time in secs for passive
 * obss scan
 *
 * Related: None
 *
 * Supported Feature: Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_OBSS_HT40_SCAN_PASSIVE_DWELL_TIME_NAME   "obss_passive_dwelltime"
#define CFG_OBSS_HT40_SCAN_PASSIVE_DWELL_TIME_MIN    (10)
#define CFG_OBSS_HT40_SCAN_PASSIVE_DWELL_TIME_MAX    (1000)
#define CFG_OBSS_HT40_SCAN_PASSIVE_DWELL_TIME_DEFAULT (20)

/*
 * <ini>
 * obss_width_trigger_interval - Set obss trigger interval
 * @Min: 10
 * @Max: 900
 * @Default: 200
 *
 * This ini is used during an OBSS scan operation,
 * where each channel in the set is scanned at least
 * once per configured trigger interval time.
 *
 * Related: None
 *
 * Supported Feature: Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_OBSS_HT40_SCAN_WIDTH_TRIGGER_INTERVAL_NAME "obss_width_trigger_interval"
#define CFG_OBSS_HT40_SCAN_WIDTH_TRIGGER_INTERVAL_MIN  (10)
#define CFG_OBSS_HT40_SCAN_WIDTH_TRIGGER_INTERVAL_MAX  (900)
#define CFG_OBSS_HT40_SCAN_WIDTH_TRIGGER_INTERVAL_DEFAULT (200)

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
 * See enum wmi_dwelltime_adaptive_mode.
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
 * See enum wmi_dwelltime_adaptive_mode.
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
 * during ext scan. see enum wmi_dwelltime_adaptive_mode.
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
 * during pno scan. see enum wmi_dwelltime_adaptive_mode.
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
 * For uses : see enum wmi_dwelltime_adaptive_mode
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
#define CFG_ROAM_RSSI_ABS_THRESHOLD_NAME        "rssi_abs_thresh"
#define CFG_ROAM_RSSI_ABS_THRESHOLD_MIN         (-96)
#define CFG_ROAM_RSSI_ABS_THRESHOLD_MAX         (0)
#define CFG_ROAM_RSSI_ABS_THRESHOLD_DEFAULT     (0)

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
 * g11bNumTxChains - Number of Tx Chanins in 11b mode
 * @Min: 0
 * @Max: 2
 * @Default: 0
 *
 * Number of Tx Chanins in 11b mode
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
#define CFG_11B_NUM_TX_CHAIN_NAME      "g11bNumTxChains"
#define CFG_11B_NUM_TX_CHAIN_MIN       (0)
#define CFG_11B_NUM_TX_CHAIN_MAX       (2)
#define CFG_11B_NUM_TX_CHAIN_DEFAULT   (0)

/*
 * <ini>
 * g11agNumTxChains - Number of Tx Chanins in 11ag mode
 * @Min: 0
 * @Max: 2
 * @Default: 0
 *
 * Number of Tx Chanins in 11ag mode
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
#define CFG_11AG_NUM_TX_CHAIN_NAME      "g11agNumTxChains"
#define CFG_11AG_NUM_TX_CHAIN_MIN       (0)
#define CFG_11AG_NUM_TX_CHAIN_MAX       (2)
#define CFG_11AG_NUM_TX_CHAIN_DEFAULT   (0)

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

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
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
#define CFG_ROAMING_OFFLOAD_NAME                "gRoamOffloadEnabled"
#define CFG_ROAMING_OFFLOAD_MIN                 (0)
#define CFG_ROAMING_OFFLOAD_MAX                 (1)
#define CFG_ROAMING_OFFLOAD_DEFAULT             (1)
#endif

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
 * groam_dense_rssi_thresh_offset - Sets dense roam RSSI threshold diff
 * @Min: 0
 * @Max: 20
 * @Default: 10
 *
 * This INI is used to set offset value from normal RSSI threshold to dense
 * RSSI threshold Fw will optimize roaming based on new RSSI threshold once
 * it detects dense enviournment.
 *
 * Related: None
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ROAM_DENSE_RSSI_THRE_OFFSET         "groam_dense_rssi_thresh_offset"
#define CFG_ROAM_DENSE_RSSI_THRE_OFFSET_MIN     (0)
#define CFG_ROAM_DENSE_RSSI_THRE_OFFSET_MAX     (20)
#define CFG_ROAM_DENSE_RSSI_THRE_OFFSET_DEFAULT (10)

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
#define CFG_ROAM_DENSE_MIN_APS         "groam_dense_min_aps"
#define CFG_ROAM_DENSE_MIN_APS_MIN     (1)
#define CFG_ROAM_DENSE_MIN_APS_MAX     (5)
#define CFG_ROAM_DENSE_MIN_APS_DEFAULT (3)

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
#define CFG_ROAM_BG_SCAN_BAD_RSSI_THRESHOLD_NAME  "roam_bg_scan_bad_rssi_thresh"
#define CFG_ROAM_BG_SCAN_BAD_RSSI_THRESHOLD_MIN     (-96)
#define CFG_ROAM_BG_SCAN_BAD_RSSI_THRESHOLD_MAX     (0)
#define CFG_ROAM_BG_SCAN_BAD_RSSI_THRESHOLD_DEFAULT (-76)

/*
 * <ini>
 * roam_bg_scan_client_bitmap - Bitmap used to identify the scan clients
 * @Min: 0
 * @Max: 0x3FF
 * @Default: 0x
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
#define CFG_ROAM_BG_SCAN_CLIENT_BITMAP_NAME     "roam_bg_scan_client_bitmap"
#define CFG_ROAM_BG_SCAN_CLIENT_BITMAP_MIN      (0)
#define CFG_ROAM_BG_SCAN_CLIENT_BITMAP_MAX      (0x7FF)
#define CFG_ROAM_BG_SCAN_CLIENT_BITMAP_DEFAULT  (0x424)

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
 * roamscan_adaptive_dwell_mode - Sets dwell time adaptive mode
 * @Min: 0
 * @Max: 4
 * @Default: 1
 *
 * This parameter will set the algo used in dwell time optimization during
 * roam scan. see enum wmi_dwelltime_adaptive_mode.
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
#define CFG_ADAPTIVE_ROAMSCAN_DWELL_MODE_NAME    "roamscan_adaptive_dwell_mode"
#define CFG_ADAPTIVE_ROAMSCAN_DWELL_MODE_MIN     (0)
#define CFG_ADAPTIVE_ROAMSCAN_DWELL_MODE_MAX     (4)
#define CFG_ADAPTIVE_ROAMSCAN_DWELL_MODE_DEFAULT (1)

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

/*
 * <ini>
 * gAPAutoShutOff - Auto shutdown when timer expires
 * @Min: 0
 * @Max: 4294967295UL
 * @Default: 0
 *
 * This ini is used to configure timer value to shutdown AP once timer expired
 *
 * Related: None.
 *
 * Supported Feature: SAP
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_AP_AUTO_SHUT_OFF                "gAPAutoShutOff"
#define CFG_AP_AUTO_SHUT_OFF_MIN            (0)
#define CFG_AP_AUTO_SHUT_OFF_MAX            (4294967295UL)
#define CFG_AP_AUTO_SHUT_OFF_DEFAULT        (0)

/*
 * <ini>
 * gApKeepAlivePeriod - AP keep alive period
 * @Min: 1
 * @Max: 65535
 * @Default: 20
 *
 * This ini is used to set keep alive period of AP
 *
 * Related: None.
 *
 * Supported Feature: SAP
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_AP_KEEP_ALIVE_PERIOD_NAME          "gApKeepAlivePeriod"
#define CFG_AP_KEEP_ALIVE_PERIOD_MIN           WNI_CFG_AP_KEEP_ALIVE_TIMEOUT_STAMIN
#define CFG_AP_KEEP_ALIVE_PERIOD_MAX           WNI_CFG_AP_KEEP_ALIVE_TIMEOUT_STAMAX
#define CFG_AP_KEEP_ALIVE_PERIOD_DEFAULT       WNI_CFG_AP_KEEP_ALIVE_TIMEOUT_STADEF

/*
 * <ini>
 * gApLinkMonitorPeriod - AP keep alive period
 * @Min: 3
 * @Max: 50
 * @Default: 10
 *
 * This ini is used to configure AP link monitor timeout value
 *
 * Related: None.
 *
 * Supported Feature: SAP
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_AP_LINK_MONITOR_PERIOD_NAME          "gApLinkMonitorPeriod"
#define CFG_AP_LINK_MONITOR_PERIOD_MIN           (3)
#define CFG_AP_LINK_MONITOR_PERIOD_MAX           (50)
#define CFG_AP_LINK_MONITOR_PERIOD_DEFAULT       (10)

/*
 * <ini>
 * gBeaconInterval - Beacon interval for SoftAP
 * @Min: 0
 * @Max: 65535
 * @Default: 100
 *
 * This ini is used to set beacon interval for SoftAP
 *
 * Related: None.
 *
 * Supported Feature: SAP
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_BEACON_INTERVAL_NAME               "gBeaconInterval"
#define CFG_BEACON_INTERVAL_MIN                WNI_CFG_BEACON_INTERVAL_STAMIN
#define CFG_BEACON_INTERVAL_MAX                WNI_CFG_BEACON_INTERVAL_STAMAX
#define CFG_BEACON_INTERVAL_DEFAULT            WNI_CFG_BEACON_INTERVAL_STADEF

/*
 * <ini>
 * gEnableVSTASupport - Enable/disable VSTA support
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable/disable VSTA based on max assoc limit.
 * Enable to have maximum 32 STA (P2P GC) on DUT as P2P GO or SAP
 *
 * Related: None.
 *
 * Supported Feature: SAP
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#ifdef WLAN_SOFTAP_VSTA_FEATURE
#define CFG_VSTA_SUPPORT_ENABLE               "gEnableVSTASupport"
#define CFG_VSTA_SUPPORT_ENABLE_MIN           (0)
#define CFG_VSTA_SUPPORT_ENABLE_MAX           (1)
#define CFG_VSTA_SUPPORT_ENABLE_DEFAULT       (0)
#endif

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
		   (QDF_MCC_TO_SCC_SWITCH_FORCE_PREFERRED_WITHOUT_DISCONNECTION)
#define CFG_WLAN_MCC_TO_SCC_SWITCH_MODE_DEFAULT  (QDF_MCC_TO_SCC_SWITCH_DISABLE)
#endif

#define CFG_DISABLE_PACKET_FILTER		"gDisablePacketFilter"
#define CFG_DISABLE_PACKET_FILTER_MIN		(0)
#define CFG_DISABLE_PACKET_FILTER_MAX		(1)
#define CFG_DISABLE_PACKET_FILTER_DEFAULT	(1)

#define CFG_ENABLE_LTE_COEX              "gEnableLTECoex"
#define CFG_ENABLE_LTE_COEX_MIN               (0)
#define CFG_ENABLE_LTE_COEX_MAX               (1)
#define CFG_ENABLE_LTE_COEX_DEFAULT           (0)

#define CFG_GO_KEEP_ALIVE_PERIOD_NAME          "gGoKeepAlivePeriod"
#define CFG_GO_KEEP_ALIVE_PERIOD_MIN           WNI_CFG_GO_KEEP_ALIVE_TIMEOUT_STAMIN
#define CFG_GO_KEEP_ALIVE_PERIOD_MAX           WNI_CFG_GO_KEEP_ALIVE_TIMEOUT_STAMAX
#define CFG_GO_KEEP_ALIVE_PERIOD_DEFAULT       WNI_CFG_GO_KEEP_ALIVE_TIMEOUT_STADEF

/* gGoLinkMonitorPeriod is period where link is idle and where
 * we send NULL frame
 */
#define CFG_GO_LINK_MONITOR_PERIOD_NAME          "gGoLinkMonitorPeriod"
#define CFG_GO_LINK_MONITOR_PERIOD_MIN           (3)
#define CFG_GO_LINK_MONITOR_PERIOD_MAX           (50)
#define CFG_GO_LINK_MONITOR_PERIOD_DEFAULT       (10)

#define CFG_VCC_RSSI_TRIGGER_NAME             "gVccRssiTrigger"
#define CFG_VCC_RSSI_TRIGGER_MIN              (0)
#define CFG_VCC_RSSI_TRIGGER_MAX              (80)
#define CFG_VCC_RSSI_TRIGGER_DEFAULT          (80)

#define CFG_VCC_UL_MAC_LOSS_THRESH_NAME       "gVccUlMacLossThresh"
#define CFG_VCC_UL_MAC_LOSS_THRESH_MIN        (0)
#define CFG_VCC_UL_MAC_LOSS_THRESH_MAX        (9)
#define CFG_VCC_UL_MAC_LOSS_THRESH_DEFAULT    (9)

#define CFG_RETRY_LIMIT_ZERO_NAME       "gRetryLimitZero"
#define CFG_RETRY_LIMIT_ZERO_MIN        (0)
#define CFG_RETRY_LIMIT_ZERO_MAX        (15)
#define CFG_RETRY_LIMIT_ZERO_DEFAULT    (5)

#define CFG_RETRY_LIMIT_ONE_NAME       "gRetryLimitOne"
#define CFG_RETRY_LIMIT_ONE_MIN        (0)
#define CFG_RETRY_LIMIT_ONE_MAX        (15)
#define CFG_RETRY_LIMIT_ONE_DEFAULT    (10)

#define CFG_RETRY_LIMIT_TWO_NAME       "gRetryLimitTwo"
#define CFG_RETRY_LIMIT_TWO_MIN        (0)
#define CFG_RETRY_LIMIT_TWO_MAX        (15)
#define CFG_RETRY_LIMIT_TWO_DEFAULT    (15)

#ifdef WLAN_AP_STA_CONCURRENCY
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
#endif

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
 * gLowGainOverride - Indicates Low Gain Override
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to set default Low Gain Override
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_LOW_GAIN_OVERRIDE_NAME             "gLowGainOverride"
#define CFG_LOW_GAIN_OVERRIDE_MIN              WNI_CFG_LOW_GAIN_OVERRIDE_STAMIN
#define CFG_LOW_GAIN_OVERRIDE_MAX              WNI_CFG_LOW_GAIN_OVERRIDE_STAMAX
#define CFG_LOW_GAIN_OVERRIDE_DEFAULT          WNI_CFG_LOW_GAIN_OVERRIDE_STADEF

/*
 * <ini>
 * gRssiFilterPeriod - Enable gRssi Filter for RSSI Monitoring
 * @Min: STAMIN
 * @Max: STAMAX
 * @Default: STADEF
 *
 * This ini is used to Increased this value for Non-ESE AP this is cause FW
 * RSSI Monitoring the consumer of this value is ON by default. So to impact
 * power numbers we are setting this to a high value.
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_RSSI_FILTER_PERIOD_NAME            "gRssiFilterPeriod"
#define CFG_RSSI_FILTER_PERIOD_MIN             WNI_CFG_RSSI_FILTER_PERIOD_STAMIN
#define CFG_RSSI_FILTER_PERIOD_MAX             WNI_CFG_RSSI_FILTER_PERIOD_STAMAX
#define CFG_RSSI_FILTER_PERIOD_DEFAULT         WNI_CFG_RSSI_FILTER_PERIOD_STADEF

#define CFG_IGNORE_DTIM_NAME                   "gIgnoreDtim"
#define CFG_IGNORE_DTIM_MIN                    WNI_CFG_IGNORE_DTIM_STAMIN
#define CFG_IGNORE_DTIM_MAX                    WNI_CFG_IGNORE_DTIM_STAMAX
#define CFG_IGNORE_DTIM_DEFAULT                WNI_CFG_IGNORE_DTIM_STADEF

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
 * gEnableFWHeartBeatMonitoring - Enable FWHeartBeat Monitor
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to set default FWHeartBeat Monitor
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_FW_HEART_BEAT_MONITORING_NAME      "gEnableFWHeartBeatMonitoring"
#define CFG_FW_HEART_BEAT_MONITORING_MIN       (0)
#define CFG_FW_HEART_BEAT_MONITORING_MAX       (1)
#define CFG_FW_HEART_BEAT_MONITORING_DEFAULT   (1)

/*
 * <ini>
 * gEnableFWBeaconFiltering - Enable FWBeacon Filter
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to set default FWBeacon Filter
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_FW_BEACON_FILTERING_NAME           "gEnableFWBeaconFiltering"
#define CFG_FW_BEACON_FILTERING_MIN            (0)
#define CFG_FW_BEACON_FILTERING_MAX            (1)
#define CFG_FW_BEACON_FILTERING_DEFAULT        (1)

/*
 * <ini>
 * gEnableFWRssiMonitoring - Enable FWRssi Monitor
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to set default FWRssi Monitor
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_FW_RSSI_MONITORING_NAME            "gEnableFWRssiMonitoring"
#define CFG_FW_RSSI_MONITORING_MIN             (0)
#define CFG_FW_RSSI_MONITORING_MAX             (1)
#define CFG_FW_RSSI_MONITORING_DEFAULT         (1)

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
 * gDataInactivityTimeout - Data activity timeout for non wow mode.
 * @Min: 1
 * @Max: 255
 * @Default: 200
 *
 * This ini is used to set data inactivity timeout in non wow mode.
 *
 * Supported Feature: inactivity timeout in non wow mode
 *
 * Usage: External
 *
 * </ini>
 */

#define CFG_DATA_INACTIVITY_TIMEOUT_NAME       "gDataInactivityTimeout"
#define CFG_DATA_INACTIVITY_TIMEOUT_MIN        (1)
#define CFG_DATA_INACTIVITY_TIMEOUT_MAX        (255)
#define CFG_DATA_INACTIVITY_TIMEOUT_DEFAULT    (200)

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

/*
 * <ini>
 * rfSettlingTimeUs - Settle the TimeUs
 * @Min: 0
 * @Max: 60000
 * @Default: 1500
 *
 * This ini is used to set default TimeUs
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_RF_SETTLING_TIME_CLK_NAME          "rfSettlingTimeUs"
#define CFG_RF_SETTLING_TIME_CLK_MIN           (0)
#define CFG_RF_SETTLING_TIME_CLK_MAX           (60000)
#define CFG_RF_SETTLING_TIME_CLK_DEFAULT       (1500)

/*
 * <ini>
 * gStaKeepAlivePeriod - Sends NULL frame to AP periodically in
 * seconds to notify STA's existence
 * @Min: 0
 * @Max: 65535
 * @Default: 30
 *
 * This ini is used to send default NULL frame to AP
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_INFRA_STA_KEEP_ALIVE_PERIOD_NAME          "gStaKeepAlivePeriod"
#define CFG_INFRA_STA_KEEP_ALIVE_PERIOD_MIN           (0)
#define CFG_INFRA_STA_KEEP_ALIVE_PERIOD_MAX           (65535)
#define CFG_INFRA_STA_KEEP_ALIVE_PERIOD_DEFAULT       (90)

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
 * InfraUapsdVoSrvIntv - Set Uapsd service interval for voice
 * @Min: 0
 * @Max: 4294967295UL
 * @Default: 20
 *
 * This ini is used to set Uapsd service interval for voice.
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_INFRA_UAPSD_VO_SRV_INTV_NAME         "InfraUapsdVoSrvIntv"
#define CFG_QOS_WMM_INFRA_UAPSD_VO_SRV_INTV_MIN           (0)
#define CFG_QOS_WMM_INFRA_UAPSD_VO_SRV_INTV_MAX           (4294967295UL)
#define CFG_QOS_WMM_INFRA_UAPSD_VO_SRV_INTV_DEFAULT       (20)

/*
 * <ini>
 * InfraUapsdVoSuspIntv - Set Uapsd suspension interval for voice
 * @Min: 0
 * @Max: 4294967295UL
 * @Default: 2000
 *
 * This ini is used to set Uapsd suspension interval for voice.
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_INFRA_UAPSD_VO_SUS_INTV_NAME         "InfraUapsdVoSuspIntv"
#define CFG_QOS_WMM_INFRA_UAPSD_VO_SUS_INTV_MIN           (0)
#define CFG_QOS_WMM_INFRA_UAPSD_VO_SUS_INTV_MAX           (4294967295UL)
#define CFG_QOS_WMM_INFRA_UAPSD_VO_SUS_INTV_DEFAULT       (2000)

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

/* default TSPEC parameters for AC_VO */
/*
 * <ini>
 * InfraDirAcVo - Set TSPEC direction for VO
 * @Min: 0
 * @Max: 3
 * @Default: 3
 *
 * This ini is used to set TSPEC direction for VO
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_INFRA_DIR_AC_VO_NAME   "InfraDirAcVo"
#define CFG_QOS_WMM_INFRA_DIR_AC_VO_MIN     (0)
#define CFG_QOS_WMM_INFRA_DIR_AC_VO_MAX     (3)
#define CFG_QOS_WMM_INFRA_DIR_AC_VO_DEFAULT (3) /*WLAN_QCT_CUST_WMM_TSDIR_BOTH*/

/*
 * <ini>
 * InfraNomMsduSizeAcVo - Set normal MSDU size for VO
 * @Min: 0x0
 * @Max: 0xFFFF
 * @Default: 0x80D0
 *
 * This ini is used to set normal MSDU size for VO
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VO_NAME      "InfraNomMsduSizeAcVo"
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VO_MIN        (0x0)
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VO_MAX        (0xFFFF)
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VO_DEFAULT    (0x80D0)

/*
 * <ini>
 * InfraMeanDataRateAcVo - Set mean data rate for VO
 * @Min: 0x0
 * @Max: 0xFFFFFFFF
 * @Default: 0x14500
 *
 * This ini is used to set mean data rate for VO
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VO_NAME    "InfraMeanDataRateAcVo"
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VO_MIN      (0x0)
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VO_MAX      (0xFFFFFFFF)
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VO_DEFAULT  (0x14500)

/*
 * <ini>
 * InfraMinPhyRateAcVo - Set min PHY rate for VO
 * @Min: 0x0
 * @Max: 0xFFFFFFFF
 * @Default: 0x5B8D80
 *
 * This ini is used to set min PHY rate for VO
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VO_NAME       "InfraMinPhyRateAcVo"
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VO_MIN         (0x0)
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VO_MAX         (0xFFFFFFFF)
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VO_DEFAULT     (0x5B8D80)
/*
 * <ini>
 * InfraSbaAcVo - Set surplus bandwidth allowance for VO
 * @Min: 0x2001
 * @Max: 0xFFFF
 * @Default: 0x2001
 *
 * This ini is used to set surplus bandwidth allowance for VO
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
 #define CFG_QOS_WMM_INFRA_SBA_AC_VO_NAME                   "InfraSbaAcVo"
 #define CFG_QOS_WMM_INFRA_SBA_AC_VO_MIN                     (0x2001)
 #define CFG_QOS_WMM_INFRA_SBA_AC_VO_MAX                     (0xFFFF)
 #define CFG_QOS_WMM_INFRA_SBA_AC_VO_DEFAULT                 (0x2001)

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
 * SingleTIDRC - Set replay counter for all TID's
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to set replay counter for all TID's
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_SINGLE_TID_RC_NAME    "SingleTIDRC"
#define CFG_SINGLE_TID_RC_MIN      (0) /* Separate replay counter for all TID */
#define CFG_SINGLE_TID_RC_MAX      (1) /* Single replay counter for all TID */
#define CFG_SINGLE_TID_RC_DEFAULT  (1)

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
 * @Default: 1
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
#define CFG_LFR_MAWC_FEATURE_ENABLED_DEFAULT                (1)

/*
 * <ini>
 * mawc_roam_enabled - Enable/Disable MAWC during roaming
 * @Min: 0 - Disabled
 * @Max: 1 - Enabled
 * @Default: 1
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
#define CFG_MAWC_ROAM_ENABLED_NAME            "mawc_roam_enabled"
#define CFG_MAWC_ROAM_ENABLED_MIN             (0)
#define CFG_MAWC_ROAM_ENABLED_MAX             (1)
#define CFG_MAWC_ROAM_ENABLED_DEFAULT         (1)

/*
 * <ini>
 * mawc_roam_traffic_threshold - Configure traffic threshold
 * @Min: 0
 * @Max: 0xFFFFFFFF
 * @Default: 300
 *
 * This ini is used to configure the data traffic load in kBps to
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
#define CFG_MAWC_ROAM_TRAFFIC_THRESHOLD_NAME       "mawc_roam_traffic_threshold"
#define CFG_MAWC_ROAM_TRAFFIC_THRESHOLD_MIN        (0)
#define CFG_MAWC_ROAM_TRAFFIC_THRESHOLD_MAX        (0xFFFFFFFF)
#define CFG_MAWC_ROAM_TRAFFIC_THRESHOLD_DEFAULT    (300)

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
#define CFG_MAWC_ROAM_AP_RSSI_THRESHOLD_NAME       "mawc_roam_ap_rssi_threshold"
#define CFG_MAWC_ROAM_AP_RSSI_THRESHOLD_MIN        (-120)
#define CFG_MAWC_ROAM_AP_RSSI_THRESHOLD_MAX        (0)
#define CFG_MAWC_ROAM_AP_RSSI_THRESHOLD_DEFAULT    (-66)

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
#define CFG_MAWC_ROAM_RSSI_HIGH_ADJUST_NAME        "mawc_roam_rssi_high_adjust"
#define CFG_MAWC_ROAM_RSSI_HIGH_ADJUST_MIN         (3)
#define CFG_MAWC_ROAM_RSSI_HIGH_ADJUST_MAX         (5)
#define CFG_MAWC_ROAM_RSSI_HIGH_ADJUST_DEFAULT     (5)

/*
 * <ini>
 * mawc_roam_rssi_high_adjust - Adjust MAWC roam low RSSI
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
#define CFG_MAWC_ROAM_RSSI_LOW_ADJUST_NAME        "mawc_roam_rssi_low_adjust"
#define CFG_MAWC_ROAM_RSSI_LOW_ADJUST_MIN         (3)
#define CFG_MAWC_ROAM_RSSI_LOW_ADJUST_MAX         (5)
#define CFG_MAWC_ROAM_RSSI_LOW_ADJUST_DEFAULT     (5)

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

#define CFG_FT_RESOURCE_REQ_NAME                        "gFTResourceReqSupported"
#define CFG_FT_RESOURCE_REQ_MIN                         (0)
#define CFG_FT_RESOURCE_REQ_MAX                         (1)
#define CFG_FT_RESOURCE_REQ_DEFAULT                     (0)

#define CFG_TELE_BCN_TRANS_LI_NAME                   "telescopicBeaconTransListenInterval"
#define CFG_TELE_BCN_TRANS_LI_MIN                    (0)
#define CFG_TELE_BCN_TRANS_LI_MAX                    (7)
#define CFG_TELE_BCN_TRANS_LI_DEFAULT                (3)

#define CFG_TELE_BCN_TRANS_LI_NUM_IDLE_BCNS_NAME     "telescopicBeaconTransListenIntervalNumIdleBcns"
#define CFG_TELE_BCN_TRANS_LI_NUM_IDLE_BCNS_MIN      (5)
#define CFG_TELE_BCN_TRANS_LI_NUM_IDLE_BCNS_MAX      (255)
#define CFG_TELE_BCN_TRANS_LI_NUM_IDLE_BCNS_DEFAULT  (10)

#define CFG_TELE_BCN_MAX_LI_NAME                     "telescopicBeaconMaxListenInterval"
#define CFG_TELE_BCN_MAX_LI_MIN                      (0)
#define CFG_TELE_BCN_MAX_LI_MAX                      (7)
#define CFG_TELE_BCN_MAX_LI_DEFAULT                  (5)

#define CFG_TELE_BCN_MAX_LI_NUM_IDLE_BCNS_NAME       "telescopicBeaconMaxListenIntervalNumIdleBcns"
#define CFG_TELE_BCN_MAX_LI_NUM_IDLE_BCNS_MIN        (5)
#define CFG_TELE_BCN_MAX_LI_NUM_IDLE_BCNS_MAX        (255)
#define CFG_TELE_BCN_MAX_LI_NUM_IDLE_BCNS_DEFAULT    (15)

#define CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_NAME      "gNeighborLookupThreshold"
#define CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_MIN       (10)
#define CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_MAX       (120)
#define CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_DEFAULT   (78)

/*
 * <ini>
 * lookup_threshold_5g_offset - Lookup Threshold offset for 5G band
 * @Min: -120
 * @Max: +120
 * @Default: 0
 *
 * This ini is  used to set the 5G band lookup threshold for roaming.
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
#define CFG_5G_RSSI_THRESHOLD_OFFSET_NAME      "lookup_threshold_5g_offset"
#define CFG_5G_RSSI_THRESHOLD_OFFSET_MIN       (-120)
#define CFG_5G_RSSI_THRESHOLD_OFFSET_MAX       (120)
#define CFG_5G_RSSI_THRESHOLD_OFFSET_DEFAULT   (0)

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

/*
 * <ini>
 * gDynamicPSPollvalue - Set dynamic PSpoll value
 * @Min: 0
 * @Max: 255
 * @Default: 0
 *
 * This ini is used to send default PSpoll value
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_DYNAMIC_PSPOLL_VALUE_NAME          "gDynamicPSPollvalue"
#define CFG_DYNAMIC_PSPOLL_VALUE_MIN           (0)
#define CFG_DYNAMIC_PSPOLL_VALUE_MAX           (255)
#define CFG_DYNAMIC_PSPOLL_VALUE_DEFAULT       (0)

/*
 * <ini>
 * gTelescopicBeaconWakeupEn - Set teles copic beacon wakeup
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to set default teles copic beacon wakeup
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_TELE_BCN_WAKEUP_EN_NAME            "gTelescopicBeaconWakeupEn"
#define CFG_TELE_BCN_WAKEUP_EN_MIN             (0)
#define CFG_TELE_BCN_WAKEUP_EN_MAX             (1)
#define CFG_TELE_BCN_WAKEUP_EN_DEFAULT         (0)

/*
 * <ini>
 * gValidateScanList - Set valid date scan list
 * @Min: 0
 * @Max: 65535
 * @Default: 30
 *
 * This ini is used to set default valid date scan list
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_VALIDATE_SCAN_LIST_NAME                 "gValidateScanList"
#define CFG_VALIDATE_SCAN_LIST_MIN                  (0)
#define CFG_VALIDATE_SCAN_LIST_MAX                  (1)
#define CFG_VALIDATE_SCAN_LIST_DEFAULT              (0)

#define CFG_NULLDATA_AP_RESP_TIMEOUT_NAME       "gNullDataApRespTimeout"
#define CFG_NULLDATA_AP_RESP_TIMEOUT_MIN        (WNI_CFG_PS_NULLDATA_AP_RESP_TIMEOUT_STAMIN)
#define CFG_NULLDATA_AP_RESP_TIMEOUT_MAX        (WNI_CFG_PS_NULLDATA_AP_RESP_TIMEOUT_STAMAX)
#define CFG_NULLDATA_AP_RESP_TIMEOUT_DEFAULT    (WNI_CFG_PS_NULLDATA_AP_RESP_TIMEOUT_STADEF)

#define CFG_AP_DATA_AVAIL_POLL_PERIOD_NAME      "gApDataAvailPollInterval"
#define CFG_AP_DATA_AVAIL_POLL_PERIOD_MIN       (WNI_CFG_AP_DATA_AVAIL_POLL_PERIOD_STAMIN)
#define CFG_AP_DATA_AVAIL_POLL_PERIOD_MAX       (WNI_CFG_AP_DATA_AVAIL_POLL_PERIOD_STAMAX)
#define CFG_AP_DATA_AVAIL_POLL_PERIOD_DEFAULT   (WNI_CFG_AP_DATA_AVAIL_POLL_PERIOD_STADEF)

#define CFG_ENABLE_HOST_ARPOFFLOAD_NAME         "hostArpOffload"
#define CFG_ENABLE_HOST_ARPOFFLOAD_MIN          (0)
#define CFG_ENABLE_HOST_ARPOFFLOAD_MAX          (1)
#define CFG_ENABLE_HOST_ARPOFFLOAD_DEFAULT      (1)

#define CFG_ENABLE_HOST_SSDP_NAME              "ssdp"
#define CFG_ENABLE_HOST_SSDP_MIN               (0)
#define CFG_ENABLE_HOST_SSDP_MAX               (1)
#define CFG_ENABLE_HOST_SSDP_DEFAULT           (1)

#define CFG_ENABLE_HOST_NSOFFLOAD_NAME         "hostNSOffload"
#define CFG_ENABLE_HOST_NSOFFLOAD_MIN          (0)
#define CFG_ENABLE_HOST_NSOFFLOAD_MAX          (1)
#define CFG_ENABLE_HOST_NSOFFLOAD_DEFAULT      (1)

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
#define CFG_HW_FILTER_MODE_NAME		"gHwFilterMode"
#define CFG_HW_FILTER_MODE_MIN		(0)
#define CFG_HW_FILTER_MODE_MAX		(3)
#define CFG_HW_FILTER_MODE_DEFAULT	(1)

/*
 * <ini>
 * BandCapability - Preferred band (0: Both,  1: 2.4G only,  2: 5G only)
 * @Min: 0
 * @Max: 2
 * @Default: 0
 *
 * This ini is used to set default band capability
 * (0: Both, 1: 2.4G only, 2: 5G only)
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_BAND_CAPABILITY_NAME          "BandCapability"
#define CFG_BAND_CAPABILITY_MIN           (0)
#define CFG_BAND_CAPABILITY_MAX           (2)
#define CFG_BAND_CAPABILITY_DEFAULT       (0)

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
 * gVhtChannelWidth - Channel width capability for 11ac
 * @Min: 0
 * @Max: 4
 * @Default: 3
 *
 * This ini is  used to set channel width capability for 11AC.
 * eHT_CHANNEL_WIDTH_20MHZ = 0,
 * eHT_CHANNEL_WIDTH_40MHZ = 1,
 * eHT_CHANNEL_WIDTH_80MHZ = 2,
 * eHT_CHANNEL_WIDTH_160MHZ = 3,
 * eHT_CHANNEL_WIDTH_80P80MHZ = 4,
 *
 * Related: NA
 *
 * Supported Feature: 11AC
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_VHT_CHANNEL_WIDTH                "gVhtChannelWidth"
#define CFG_VHT_CHANNEL_WIDTH_MIN            (0)
#define CFG_VHT_CHANNEL_WIDTH_MAX            (4)
#define CFG_VHT_CHANNEL_WIDTH_DEFAULT        (2)

/*
 * <ini>
 * gVhtRxMCS - VHT Rx MCS capability for 1x1 mode
 * @Min: 0
 * @Max: 2
 * @Default: 0
 *
 * This ini is  used to set VHT Rx MCS capability for 1x1 mode.
 * 0, MCS0-7
 * 1, MCS0-8
 * 2, MCS0-9
 *
 * Related: NA
 *
 * Supported Feature: 11AC
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_VHT_ENABLE_RX_MCS_8_9               "gVhtRxMCS"
#define CFG_VHT_ENABLE_RX_MCS_8_9_MIN           (0)
#define CFG_VHT_ENABLE_RX_MCS_8_9_MAX           (2)
#define CFG_VHT_ENABLE_RX_MCS_8_9_DEFAULT       (0)

/*
 * <ini>
 * gVhtTxMCS - VHT Tx MCS capability for 1x1 mode
 * @Min: 0
 * @Max: 2
 * @Default: 0
 *
 * This ini is  used to set VHT Tx MCS capability for 1x1 mode.
 * 0, MCS0-7
 * 1, MCS0-8
 * 2, MCS0-9
 *
 * Related: NA
 *
 * Supported Feature: 11AC
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_VHT_ENABLE_TX_MCS_8_9               "gVhtTxMCS"
#define CFG_VHT_ENABLE_TX_MCS_8_9_MIN           (0)
#define CFG_VHT_ENABLE_TX_MCS_8_9_MAX           (2)
#define CFG_VHT_ENABLE_TX_MCS_8_9_DEFAULT       (0)

/*
 * <ini>
 * gVhtRxMCS2x2 - VHT Rx MCS capability for 2x2 mode
 * @Min: 0
 * @Max: 2
 * @Default: 0
 *
 * This ini is  used to set VHT Rx MCS capability for 2x2 mode.
 * 0, MCS0-7
 * 1, MCS0-8
 * 2, MCS0-9
 *
 * Related: NA
 *
 * Supported Feature: 11AC
 *
 * Usage: External
 *
 * </ini>
 */

#define CFG_VHT_ENABLE_RX_MCS2x2_8_9               "gVhtRxMCS2x2"
#define CFG_VHT_ENABLE_RX_MCS2x2_8_9_MIN           (0)
#define CFG_VHT_ENABLE_RX_MCS2x2_8_9_MAX           (2)
#define CFG_VHT_ENABLE_RX_MCS2x2_8_9_DEFAULT       (0)

/*
 * <ini>
 * gVhtTxMCS2x2 - VHT Tx MCS capability for 2x2 mode
 * @Min: 0
 * @Max: 2
 * @Default: 0
 *
 * This ini is  used to set VHT Tx MCS capability for 2x2 mode.
 * 0, MCS0-7
 * 1, MCS0-8
 * 2, MCS0-9
 *
 * Related: NA
 *
 * Supported Feature: 11AC
 *
 * Usage: External
 *
 * </ini>
 */

#define CFG_VHT_ENABLE_TX_MCS2x2_8_9               "gVhtTxMCS2x2"
#define CFG_VHT_ENABLE_TX_MCS2x2_8_9_MIN           (0)
#define CFG_VHT_ENABLE_TX_MCS2x2_8_9_MAX           (2)
#define CFG_VHT_ENABLE_TX_MCS2x2_8_9_DEFAULT       (0)

/*
 * <ini>
 * gEnable2x2 - Enables/disables VHT Tx/Rx MCS values for 2x2
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini disables/enables 2x2 mode. If this is zero then DUT operates as 1x1
 *
 * 0, Disable
 * 1, Enable
 *
 * Related: NA
 *
 * Supported Feature: 11AC
 *
 * Usage: External
 *
 * </ini>
 */

#define CFG_VHT_ENABLE_2x2_CAP_FEATURE         "gEnable2x2"
#define CFG_VHT_ENABLE_2x2_CAP_FEATURE_MIN     (0)
#define CFG_VHT_ENABLE_2x2_CAP_FEATURE_MAX     (1)
#define CFG_VHT_ENABLE_2x2_CAP_FEATURE_DEFAULT (0)

/*
 * <ini>
 * disable_high_ht_mcs_2x2 - disable high mcs index for 2nd stream in 2.4G
 * @Min: 0
 * @Max: 8
 * @Default: 0
 *
 * This ini is used to disable high HT MCS index for 2.4G STA connection.
 * It has been introduced to resolve IOT issue with one of the vendor.
 *
 * Note: This INI is not useful with 1x1 setting. If some platform supports
 * only 1x1 then this INI is not useful.
 *
 * 0 - It won't disable any HT MCS index (just like normal HT MCS)
 * 1 - It will disable 15th bit from HT RX MCS set (from 8-15 bits slot)
 * 2 - It will disable 14th & 15th bits from HT RX MCS set
 * 3 - It will disable 13th, 14th, & 15th bits from HT RX MCS set
 * and so on.
 *
 * Related: STA
 *
 * Supported Feature: 11n
 *
 * Usage: External
 */
#define CFG_DISABLE_HIGH_HT_RX_MCS_2x2         "disable_high_ht_mcs_2x2"
#define CFG_DISABLE_HIGH_HT_RX_MCS_2x2_MIN     (0)
#define CFG_DISABLE_HIGH_HT_RX_MCS_2x2_MAX     (8)
#define CFG_DISABLE_HIGH_HT_RX_MCS_2x2_DEFAULT (0)

/*
 * <ini>
 * gStaPrefer80MHzOver160MHz - set Sta perferance to connect in 80HZ/160HZ
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is  used to set Sta perferance to connect in 80HZ/160HZ
 *
 * 0 - Connects in 160MHz 1x1 when AP is 160MHz 2x2
 * 1 - Connects in 80MHz 2x2 when AP is 160MHz 2x2
 *
 * Related: NA
 *
 * Supported Feature: 11AC
 *
 * Usage: External
 *
 * </ini>
 */

#define CFG_STA_PREFER_80MHZ_OVER_160MHZ         "gStaPrefer80MHzOver160MHz"
#define CFG_STA_PREFER_80MHZ_OVER_160MHZ_MIN     (0)
#define CFG_STA_PREFER_80MHZ_OVER_160MHZ_MAX     (1)
#define CFG_STA_PREFER_80MHZ_OVER_160MHZ_DEFAULT (1)

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

/*
 * <ini>
 * gEnableMuBformee - Enables/disables multi-user (MU) beam formee capability
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini enables/disables multi-user (MU) beam formee
 * capability
 *
 * Change MU Bformee only when  gTxBFEnable is enabled.
 * When gTxBFEnable and gEnableMuBformee are set, MU beam formee capability is
 * enabled.
 * Related:  gTxBFEnable
 *
 * Supported Feature: 11AC
 *
 * Usage: External
 *
 * </ini>
 */

#define CFG_VHT_ENABLE_MU_BFORMEE_CAP_FEATURE         "gEnableMuBformee"
#define CFG_VHT_ENABLE_MU_BFORMEE_CAP_FEATURE_MIN     (0)
#define CFG_VHT_ENABLE_MU_BFORMEE_CAP_FEATURE_MAX     (1)
#define CFG_VHT_ENABLE_MU_BFORMEE_CAP_FEATURE_DEFAULT (0)

#define CFG_VHT_ENABLE_PAID_FEATURE             "gEnablePAID"
#define CFG_VHT_ENABLE_PAID_FEATURE_MIN         (0)
#define CFG_VHT_ENABLE_PAID_FEATURE_MAX         (1)
#define CFG_VHT_ENABLE_PAID_FEATURE_DEFAULT     (0)

#define CFG_VHT_ENABLE_GID_FEATURE              "gEnableGID"
#define CFG_VHT_ENABLE_GID_FEATURE_MIN          (0)
#define CFG_VHT_ENABLE_GID_FEATURE_MAX          (1)
#define CFG_VHT_ENABLE_GID_FEATURE_DEFAULT      (0)

/*
 * <ini>
 * gSetTxChainmask1x1 - Sets Transmit chain mask.
 * @Min: 1
 * @Max: 2
 * @Default: 1
 *
 * This ini Sets Transmit chain mask.
 *
 * If gEnable2x2 is disabled, gSetTxChainmask1x1 and gSetRxChainmask1x1 values
 * are taken into account. If chainmask value exceeds the maximum number of
 * chains supported by target, the max number of chains is used. By default,
 * chain0 is selected for both Tx and Rx.
 * gSetTxChainmask1x1=1 or gSetRxChainmask1x1=1 to select chain0.
 * gSetTxChainmask1x1=2 or gSetRxChainmask1x1=2 to select chain1.
 *
 * Supported Feature: 11AC
 *
 * Usage: External
 *
 * </ini>
 */

#define CFG_VHT_ENABLE_1x1_TX_CHAINMASK         "gSetTxChainmask1x1"
#define CFG_VHT_ENABLE_1x1_TX_CHAINMASK_MIN     (0)
#define CFG_VHT_ENABLE_1x1_TX_CHAINMASK_MAX     (2)
#define CFG_VHT_ENABLE_1x1_TX_CHAINMASK_DEFAULT (0)

/*
 * <ini>
 * gSetRxChainmask1x1 - Sets Receive chain mask.
 * @Min: 1
 * @Max: 2
 * @Default: 1
 *
 * This ini is  used to set Receive chain mask.
 *
 * If gEnable2x2 is disabled, gSetTxChainmask1x1 and gSetRxChainmask1x1 values
 * are taken into account. If chainmask value exceeds the maximum number of
 * chains supported by target, the max number of chains is used. By default,
 * chain0 is selected for both Tx and Rx.
 * gSetTxChainmask1x1=1 or gSetRxChainmask1x1=1 to select chain0.
 * gSetTxChainmask1x1=2 or gSetRxChainmask1x1=2 to select chain1.
 *
 * Supported Feature: 11AC
 *
 * Usage: External
 *
 * </ini>
 */

#define CFG_VHT_ENABLE_1x1_RX_CHAINMASK         "gSetRxChainmask1x1"
#define CFG_VHT_ENABLE_1x1_RX_CHAINMASK_MIN     (0)
#define CFG_VHT_ENABLE_1x1_RX_CHAINMASK_MAX     (2)
#define CFG_VHT_ENABLE_1x1_RX_CHAINMASK_DEFAULT (0)

/*
 * <ini>
 * gEnableAMPDUPS - Enable the AMPDUPS
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to set default AMPDUPS
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_ENABLE_AMPDUPS_FEATURE              "gEnableAMPDUPS"
#define CFG_ENABLE_AMPDUPS_FEATURE_MIN          (0)
#define CFG_ENABLE_AMPDUPS_FEATURE_MAX          (1)
#define CFG_ENABLE_AMPDUPS_FEATURE_DEFAULT      (0)

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
#define CFG_REPORT_MAX_LINK_SPEED_DEFAULT          (eHDD_LINK_SPEED_REPORT_MAX_SCALED)

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
 * isP2pDeviceAddrAdministrated - Enables to derive the P2P MAC address from
 * the primary MAC address
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable/disable to derive the P2P MAC address from the
 * primary MAC address.
 *
 * Supported Feature: P2P
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_P2P_DEVICE_ADDRESS_ADMINISTRATED_NAME                "isP2pDeviceAddrAdministrated"
#define CFG_P2P_DEVICE_ADDRESS_ADMINISTRATED_MIN                 (0)
#define CFG_P2P_DEVICE_ADDRESS_ADMINISTRATED_MAX                 (1)
#define CFG_P2P_DEVICE_ADDRESS_ADMINISTRATED_DEFAULT             (1)

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

/*
 * <ini>
 * gEnable5gEBT - Enables/disables 5G early beacon termination. When enabled
 *                 terminate the reception of beacon if the TIM element is
 *                 clear for the power saving
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to set default 5G early beacon termination
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_PPS_ENABLE_5G_EBT                 "gEnable5gEBT"
#define CFG_PPS_ENABLE_5G_EBT_FEATURE_MIN     (0)
#define CFG_PPS_ENABLE_5G_EBT_FEATURE_MAX     (1)
#define CFG_PPS_ENABLE_5G_EBT_FEATURE_DEFAULT (1)

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

/*
 * <ini>
 * gDefaultRateIndex24Ghz -Set the rate index for 24Ghz
 * @Min: 1
 * @Max: 9
 * @Default: 1
 *
 * This ini is used to set default rate index
 * In cfg.dat 1=1MBPS, 2=2MBPS, 3=5_5MBPS, 4=11MBPS, 5=6MBPS, 6=9MBPS,
 * 7=12MBPS, 8=18MBPS, 9=24MBPS. But 6=9MBPS and 8=18MBPS are not basic
 * 11g rates and should not be set by gDefaultRateIndex24Ghz.
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_DEFAULT_RATE_INDEX_24GH               "gDefaultRateIndex24Ghz"
#define CFG_DEFAULT_RATE_INDEX_24GH_MIN           (1)
#define CFG_DEFAULT_RATE_INDEX_24GH_MAX           (9)
#define CFG_DEFAULT_RATE_INDEX_24GH_DEFAULT       (1)

#define CFG_ENABLE_PACKET_LOG            "gEnablePacketLog"
#define CFG_ENABLE_PACKET_LOG_MIN        (0)
#define CFG_ENABLE_PACKET_LOG_MAX        (1)
#ifdef FEATURE_PKTLOG
#define CFG_ENABLE_PACKET_LOG_DEFAULT    (1)
#else
#define CFG_ENABLE_PACKET_LOG_DEFAULT    (0)
#endif



/* gFwDebugLogType takes values from enum dbglog_process_t,
 * make default value as DBGLOG_PROCESS_NET_RAW to give the
 * logs to net link since cnss_diag service is started at boot
 * time by default.
 */
#define CFG_ENABLE_FW_LOG_TYPE            "gFwDebugLogType"
#define CFG_ENABLE_FW_LOG_TYPE_MIN        (0)
#define CFG_ENABLE_FW_LOG_TYPE_MAX        (255)
#define CFG_ENABLE_FW_LOG_TYPE_DEFAULT    (3)

/* gFwDebugLogLevel takes values from enum DBGLOG_LOG_LVL,
 * make default value as DBGLOG_WARN to enable error and
 * warning logs by default.
 */
#define CFG_ENABLE_FW_DEBUG_LOG_LEVEL          "gFwDebugLogLevel"
#define CFG_ENABLE_FW_DEBUG_LOG_LEVEL_MIN      (0)
#define CFG_ENABLE_FW_DEBUG_LOG_LEVEL_MAX      (255)
#define CFG_ENABLE_FW_DEBUG_LOG_LEVEL_DEFAULT  (3)

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
 * <ini>
 * gEnableRTSProfiles - It will use configuring different RTS profiles
 * @Min: 0
 * @Max: 66
 * @Default: 33
 *
 * This ini used for configuring different RTS profiles
 * to firmware.
 * Following are the valid values for the rts profile:
 * RTSCTS_DISABLED				0
 * NOT_ALLOWED					1
 * NOT_ALLOWED					2
 * RTSCTS_DISABLED				16
 * RTSCTS_ENABLED_4_SECOND_RATESERIES		17
 * CTS2SELF_ENABLED_4_SECOND_RATESERIES		18
 * RTSCTS_DISABLED				32
 * RTSCTS_ENABLED_4_SWRETRIES			33
 * CTS2SELF_ENABLED_4_SWRETRIES			34
 * NOT_ALLOWED					48
 * NOT_ALLOWED					49
 * NOT_ALLOWED					50
 * RTSCTS_DISABLED				64
 * RTSCTS_ENABLED_4_ALL_RATESERIES		65
 * CTS2SELF_ENABLED_4_ALL_RATESERIES		66
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_ENABLE_FW_RTS_PROFILE              "gEnableRTSProfiles"
#define CFG_ENABLE_FW_RTS_PROFILE_MIN          (0)
#define CFG_ENABLE_FW_RTS_PROFILE_MAX          (66)
#define CFG_ENABLE_FW_RTS_PROFILE_DEFAULT      (33)

#ifdef WLAN_SUPPORT_GREEN_AP
#define CFG_ENABLE_GREEN_AP_FEATURE         "gEnableGreenAp"
#define CFG_ENABLE_GREEN_AP_FEATURE_MIN     (0)
#define CFG_ENABLE_GREEN_AP_FEATURE_MAX     (1)
#define CFG_ENABLE_GREEN_AP_FEATURE_DEFAULT (1)

/* Enhanced Green AP (EGAP) flags/params */
#define CFG_ENABLE_EGAP_ENABLE_FEATURE             "gEnableEGAP"
#define CFG_ENABLE_EGAP_ENABLE_FEATURE_MIN         (0)
#define CFG_ENABLE_EGAP_ENABLE_FEATURE_MAX         (1)
#define CFG_ENABLE_EGAP_ENABLE_FEATURE_DEFAULT     (1)

#define CFG_ENABLE_EGAP_INACT_TIME_FEATURE         "gEGAPInactTime"
#define CFG_ENABLE_EGAP_INACT_TIME_FEATURE_MIN     (0)
#define CFG_ENABLE_EGAP_INACT_TIME_FEATURE_MAX     (300000)
#define CFG_ENABLE_EGAP_INACT_TIME_FEATURE_DEFAULT (2000)

#define CFG_ENABLE_EGAP_WAIT_TIME_FEATURE          "gEGAPWaitTime"
#define CFG_ENABLE_EGAP_WAIT_TIME_FEATURE_MIN      (0)
#define CFG_ENABLE_EGAP_WAIT_TIME_FEATURE_MAX      (300000)
#define CFG_ENABLE_EGAP_WAIT_TIME_FEATURE_DEFAULT  (150)

#define CFG_ENABLE_EGAP_FLAGS_FEATURE              "gEGAPFeatures"
#define CFG_ENABLE_EGAP_FLAGS_FEATURE_MIN          (0)
#define CFG_ENABLE_EGAP_FLAGS_FEATURE_MAX          (15)
#define CFG_ENABLE_EGAP_FLAGS_FEATURE_DEFAULT      (3)
/* end Enhanced Green AP flags/params */

#endif

#ifdef FEATURE_WLAN_FORCE_SAP_SCC
/*
 * <ini>
 * gSapSccChanAvoidance - Channel avoidance for SAP in SCC.
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable/disable channel avoidance for SAP in SCC
 * scenario.
 *
 * Related: None.
 *
 * Supported Feature: Concurrency
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_SAP_SCC_CHAN_AVOIDANCE         "gSapSccChanAvoidance"
#define CFG_SAP_SCC_CHAN_AVOIDANCE_MIN     (0)
#define CFG_SAP_SCC_CHAN_AVOIDANCE_MAX     (1)
#define CFG_SAP_SCC_CHAN_AVOIDANCE_DEFAULT (0)
#endif /* FEATURE_WLAN_FORCE_SAP_SCC */

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

#define CFG_QDF_TRACE_ENABLE_MIN          (0)
#define CFG_QDF_TRACE_ENABLE_MAX          (0xff)
#define CFG_QDF_TRACE_ENABLE_DEFAULT      (0xffff)
/* disable debug logs for DP by default */
#define CFG_QDF_TRACE_ENABLE_DP_DEFAULT   (0x7f)

#define HDD_MCASTBCASTFILTER_FILTER_NONE                       0x00
#define HDD_MCASTBCASTFILTER_FILTER_ALL_MULTICAST              0x01
#define HDD_MCASTBCASTFILTER_FILTER_ALL_BROADCAST              0x02
#define HDD_MCASTBCASTFILTER_FILTER_ALL_MULTICAST_BROADCAST    0x03
#define HDD_MULTICAST_FILTER_LIST                              0x04
#define HDD_MULTICAST_FILTER_LIST_CLEAR                        0x05

/*
 * Enable Dynamic DTIM
 * Options
 * 0 -Disable DynamicDTIM
 * 1 to 5 - SLM will switch to DTIM specified here when host suspends and
 *          switch DTIM1 when host resumes
 */
#define CFG_ENABLE_DYNAMIC_DTIM_NAME            "gEnableDynamicDTIM"
#define CFG_ENABLE_DYNAMIC_DTIM_MIN        (0)
#define CFG_ENABLE_DYNAMIC_DTIM_MAX        (9)
#define CFG_ENABLE_DYNAMIC_DTIM_DEFAULT    (0)

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
 * gApAutoChannelSelection - Force ACS from ini
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to set to enable force acs from driver.
 * If enabled, channel/ hw config from hostapd is ignored.
 * Driver uses INI params dot11Mode, channel bonding mode and vht chan width
 * to derive ACS HW mode and operating BW.
 *
 * Non android platforms shall not use force ACS method and rely on hostapd
 * driven ACS method for concurrent SAP ACS configuration, OBSS etc.
 *
 * Related: Only applicable if gCoalesingInIBSS is 0
 *
 * Supported Feature: SAP
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_FORCE_SAP_ACS                  "gApAutoChannelSelection"
#define CFG_FORCE_SAP_ACS_MIN              (0)
#define CFG_FORCE_SAP_ACS_MAX              (1)
#define CFG_FORCE_SAP_ACS_DEFAULT          (0)

/*
 * <ini>
 * gAPChannelSelectStartChannel - start channel for ACS
 * @Min: 0
 * @Max: 0xFF
 * @Default: 1
 *
 * This ini is used to set start channel for ACS.
 * ACS scan will choose channel between force_sap_acs_st_ch
 * and force_sap_acs_end_ch
 *
 * Related: Only applicable gAPChannelSelectEndChannel is set
 *
 * Supported Feature: SAP
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_FORCE_SAP_ACS_START_CH         "gAPChannelSelectStartChannel"
#define CFG_FORCE_SAP_ACS_START_CH_MIN     (0)
#define CFG_FORCE_SAP_ACS_START_CH_MAX     (0xFF)
#define CFG_FORCE_SAP_ACS_START_CH_DEFAULT (1)

/*
 * <ini>
 * gAPChannelSelectEndChannel - end channel for ACS
 * @Min: 0
 * @Max: 0xFF
 * @Default: 11
 *
 * This ini is used to set end channel for ACS.
 * ACS scan will choose channel between force_sap_acs_st_ch
 * and force_sap_acs_end_ch
 *
 * Related: Only applicable if gAPChannelSelectStartChannel is set
 *
 * Supported Feature: SAP
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_FORCE_SAP_ACS_END_CH           "gAPChannelSelectEndChannel"
#define CFG_FORCE_SAP_ACS_END_CH_MIN       (0)
#define CFG_FORCE_SAP_ACS_END_CH_MAX       (0xFF)
#define CFG_FORCE_SAP_ACS_END_CH_DEFAULT   (11)

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
 * Skip DFS Channel in case of P2P Search
 * Options
 * 0 - Don't Skip DFS Channel in case of P2P Search
 * 1 - Skip DFS Channel in case of P2P Search
 */
/*
 * <ini>
 * gSkipDfsChannelInP2pSearch - Skip DFS Channel in case of P2P Search
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to to disable(skip) dfs channel in p2p search.
 * Related: NA.
 *
 * Supported Feature: DFS P2P
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_ENABLE_SKIP_DFS_IN_P2P_SEARCH_NAME       "gSkipDfsChannelInP2pSearch"
#define CFG_ENABLE_SKIP_DFS_IN_P2P_SEARCH_MIN        (0)
#define CFG_ENABLE_SKIP_DFS_IN_P2P_SEARCH_MAX        (1)
#define CFG_ENABLE_SKIP_DFS_IN_P2P_SEARCH_DEFAULT    (1)
/*
 * <ini>
 * gIgnoreDynamicDtimInP2pMode - Ignore Dynamic Dtim in case of P2P
 * Options.
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to decide if Dynamic Dtim needs to be consider or
 * not in case of P2P.
 * 0 - Consider Dynamic Dtim incase of P2P
 * 1 - Ignore Dynamic Dtim incase of P2P
 *
 * Supported Feature: P2P
 *
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_IGNORE_DYNAMIC_DTIM_IN_P2P_MODE_NAME       "gIgnoreDynamicDtimInP2pMode"
#define CFG_IGNORE_DYNAMIC_DTIM_IN_P2P_MODE_MIN        (0)
#define CFG_IGNORE_DYNAMIC_DTIM_IN_P2P_MODE_MAX        (1)
#define CFG_IGNORE_DYNAMIC_DTIM_IN_P2P_MODE_DEFAULT    (0)

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

#define CFG_THERMAL_TEMP_MIN_LEVEL0_NAME      "gThermalTempMinLevel0"
#define CFG_THERMAL_TEMP_MIN_LEVEL0_MIN       (0)
#define CFG_THERMAL_TEMP_MIN_LEVEL0_MAX       (1000)
#define CFG_THERMAL_TEMP_MIN_LEVEL0_DEFAULT   (0)

#define CFG_THERMAL_TEMP_MAX_LEVEL0_NAME      "gThermalTempMaxLevel0"
#define CFG_THERMAL_TEMP_MAX_LEVEL0_MIN       (0)
#define CFG_THERMAL_TEMP_MAX_LEVEL0_MAX       (1000)
#define CFG_THERMAL_TEMP_MAX_LEVEL0_DEFAULT   (90)

#define CFG_THERMAL_TEMP_MIN_LEVEL1_NAME      "gThermalTempMinLevel1"
#define CFG_THERMAL_TEMP_MIN_LEVEL1_MIN       (0)
#define CFG_THERMAL_TEMP_MIN_LEVEL1_MAX       (1000)
#define CFG_THERMAL_TEMP_MIN_LEVEL1_DEFAULT   (70)

#define CFG_THERMAL_TEMP_MAX_LEVEL1_NAME      "gThermalTempMaxLevel1"
#define CFG_THERMAL_TEMP_MAX_LEVEL1_MIN       (0)
#define CFG_THERMAL_TEMP_MAX_LEVEL1_MAX       (1000)
#define CFG_THERMAL_TEMP_MAX_LEVEL1_DEFAULT   (110)

#define CFG_THERMAL_TEMP_MIN_LEVEL2_NAME      "gThermalTempMinLevel2"
#define CFG_THERMAL_TEMP_MIN_LEVEL2_MIN       (0)
#define CFG_THERMAL_TEMP_MIN_LEVEL2_MAX       (1000)
#define CFG_THERMAL_TEMP_MIN_LEVEL2_DEFAULT   (90)

#define CFG_THERMAL_TEMP_MAX_LEVEL2_NAME      "gThermalTempMaxLevel2"
#define CFG_THERMAL_TEMP_MAX_LEVEL2_MIN       (0)
#define CFG_THERMAL_TEMP_MAX_LEVEL2_MAX       (1000)
#define CFG_THERMAL_TEMP_MAX_LEVEL2_DEFAULT   (125)

#define CFG_THERMAL_TEMP_MIN_LEVEL3_NAME      "gThermalTempMinLevel3"
#define CFG_THERMAL_TEMP_MIN_LEVEL3_MIN       (0)
#define CFG_THERMAL_TEMP_MIN_LEVEL3_MAX       (1000)
#define CFG_THERMAL_TEMP_MIN_LEVEL3_DEFAULT   (110)

#define CFG_THERMAL_TEMP_MAX_LEVEL3_NAME      "gThermalTempMaxLevel3"
#define CFG_THERMAL_TEMP_MAX_LEVEL3_MIN       (0)
#define CFG_THERMAL_TEMP_MAX_LEVEL3_MAX       (1000)
#define CFG_THERMAL_TEMP_MAX_LEVEL3_DEFAULT   (0)

/*
 * Enable/Disable Modulated DTIM feature
 * Default: Disable
 */
#define CFG_ENABLE_MODULATED_DTIM_NAME       "gEnableModulatedDTIM"
#define CFG_ENABLE_MODULATED_DTIM_MIN        (0)
#define CFG_ENABLE_MODULATED_DTIM_MAX        (5)
#define CFG_ENABLE_MODULATED_DTIM_DEFAULT    (0)

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
 * gMaxHTMCSForTxData - max HT mcs for TX
 * @Min: 0
 * @Max: 383
 * @Default: 0
 *
 * This ini is used to configure the max HT mcs
 * for tx data.
 *
 * Usage: External
 *
 * bits 0-15:  max HT mcs
 * bits 16-31: zero to disable, otherwise enable.
 *
 * </ini>
 */
#define CFG_MAX_HT_MCS_FOR_TX_DATA          "gMaxHTMCSForTxData"
#define CFG_MAX_HT_MCS_FOR_TX_DATA_MIN      (WNI_CFG_MAX_HT_MCS_TX_DATA_STAMIN)
#define CFG_MAX_HT_MCS_FOR_TX_DATA_MAX      (WNI_CFG_MAX_HT_MCS_TX_DATA_STAMAX)
#define CFG_MAX_HT_MCS_FOR_TX_DATA_DEFAULT  (WNI_CFG_MAX_HT_MCS_TX_DATA_STADEF)

/*
 * <ini>
 * gSapGetPeerInfo - Enable/Disable remote peer info query support
 * @Min: 0 - Disable remote peer info query support
 * @Max: 1 - Enable remote peer info query support
 * @Default: 0
 *
 * This ini is used to enable/disable remote peer info query support
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SAP_GET_PEER_INFO                      "gSapGetPeerInfo"
#define CFG_SAP_GET_PEER_INFO_MIN                   (0)
#define CFG_SAP_GET_PEER_INFO_MAX                   (1)
#define CFG_SAP_GET_PEER_INFO_DEFAULT               (0)

/*
 * <ini>
 * gDisableABGRateForTxData - disable abg rate for tx data
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to disable abg rate for tx data.
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_DISABLE_ABG_RATE_FOR_TX_DATA        "gDisableABGRateForTxData"
#define CFG_DISABLE_ABG_RATE_FOR_TX_DATA_MIN \
	(WNI_CFG_DISABLE_ABG_RATE_FOR_TX_DATA_STAMIN)
#define CFG_DISABLE_ABG_RATE_FOR_TX_DATA_MAX \
	(WNI_CFG_DISABLE_ABG_RATE_FOR_TX_DATA_STAMAX)
#define CFG_DISABLE_ABG_RATE_FOR_TX_DATA_DEFAULT \
	(WNI_CFG_DISABLE_ABG_RATE_FOR_TX_DATA_STADEF)

/*
 * <ini>
 * gRateForTxMgmt - rate for tx mgmt frame
 * @Min: 0x0
 * @Max: 0xFF
 * @Default: 0xFF
 *
 * This ini is used to configure the rate for tx
 * mgmt frame. Default 0xFF means disable.
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_RATE_FOR_TX_MGMT                  "gRateForTxMgmt"
#define CFG_RATE_FOR_TX_MGMT_MIN              (WNI_CFG_RATE_FOR_TX_MGMT_STAMIN)
#define CFG_RATE_FOR_TX_MGMT_MAX              (WNI_CFG_RATE_FOR_TX_MGMT_STAMAX)
#define CFG_RATE_FOR_TX_MGMT_DEFAULT          (WNI_CFG_RATE_FOR_TX_MGMT_STADEF)

/*
 * <ini>
 * gRateForTxMgmt2G - rate for tx mgmt frame on 2G
 * @Min: 0x0
 * @Max: 0xFF
 * @Default: 0xFF
 *
 * This ini is used to configure the rate for tx
 * mgmt frame on 2G Band. Default 0xFF means disable.
 * It has higher priority and will overwrite gRateForTxMgmt
 * setting.
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_RATE_FOR_TX_MGMT_2G            "gRateForTxMgmt2G"
#define CFG_RATE_FOR_TX_MGMT_2G_MIN        (WNI_CFG_RATE_FOR_TX_MGMT_2G_STAMIN)
#define CFG_RATE_FOR_TX_MGMT_2G_MAX        (WNI_CFG_RATE_FOR_TX_MGMT_2G_STAMAX)
#define CFG_RATE_FOR_TX_MGMT_2G_DEFAULT    (WNI_CFG_RATE_FOR_TX_MGMT_2G_STADEF)

/*
 * <ini>
 * gRateForTxMgmt5G - rate for tx mgmt frame on 5G
 * @Min: 0x0
 * @Max: 0xFF
 * @Default: 0xFF
 *
 * This ini is used to configure the rate for tx
 * mgmt frame on 5G Band. Default 0xFF means disable.
 * It has higher priority and will overwrite gRateForTxMgmt
 * setting.
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_RATE_FOR_TX_MGMT_5G            "gRateForTxMgmt5G"
#define CFG_RATE_FOR_TX_MGMT_5G_MIN        (WNI_CFG_RATE_FOR_TX_MGMT_5G_STAMIN)
#define CFG_RATE_FOR_TX_MGMT_5G_MAX        (WNI_CFG_RATE_FOR_TX_MGMT_5G_STAMAX)
#define CFG_RATE_FOR_TX_MGMT_5G_DEFAULT    (WNI_CFG_RATE_FOR_TX_MGMT_5G_STADEF)

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

#ifdef FEATURE_WLAN_TDLS
/*
 * <ini>
 * gEnableTDLSSupport - Enable support for TDLS.
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable/disable TDLS support.
 *
 * Related: None.
 *
 * Supported Feature: TDLS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_TDLS_SUPPORT_ENABLE                     "gEnableTDLSSupport"
#define CFG_TDLS_SUPPORT_ENABLE_MIN                 (0)
#define CFG_TDLS_SUPPORT_ENABLE_MAX                 (1)
#define CFG_TDLS_SUPPORT_ENABLE_DEFAULT             (0)

/*
 * <ini>
 * gEnableTDLSImplicitTrigger - Enable Implicit TDLS.
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable/disable implicit TDLS.
 * CLD driver initiates TDLS Discovery towards a peer whenever TDLS Setup
 * criteria (throughput and RSSI thresholds) is met and then it tears down
 * TDLS when teardown criteria (idle packet count and RSSI) is met.
 *
 * Related: gEnableTDLSSupport.
 *
 * Supported Feature: TDLS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_TDLS_IMPLICIT_TRIGGER                   "gEnableTDLSImplicitTrigger"
#define CFG_TDLS_IMPLICIT_TRIGGER_MIN               (0)
#define CFG_TDLS_IMPLICIT_TRIGGER_MAX               (1)
#define CFG_TDLS_IMPLICIT_TRIGGER_DEFAULT           (0)

/*
 * <ini>
 * gTDLSTxStatsPeriod - TDLS TX statistics time period.
 * @Min: 1000
 * @Max: 4294967295
 * @Default: 2000
 *
 * This ini is used to configure the time period (in ms) to evaluate whether
 * the number of Tx/Rx packets exceeds TDLSTxPacketThreshold and triggers a
 * TDLS Discovery request.
 *
 * Related: gEnableTDLSSupport.
 *
 * Supported Feature: TDLS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_TDLS_TX_STATS_PERIOD                    "gTDLSTxStatsPeriod"
#define CFG_TDLS_TX_STATS_PERIOD_MIN                (1000)
#define CFG_TDLS_TX_STATS_PERIOD_MAX                (4294967295UL)
#define CFG_TDLS_TX_STATS_PERIOD_DEFAULT            (2000)

/*
 * <ini>
 * gTDLSTxPacketThreshold - Tx/Rx Packet threshold for initiating TDLS.
 * @Min: 0
 * @Max: 4294967295
 * @Default: 40
 *
 * This ini is used to configure the number of Tx/Rx packets during the
 * period of gTDLSTxStatsPeriod when exceeded, a TDLS Discovery request
 * is triggered.
 * Related: gEnableTDLSSupport.
 *
 * Supported Feature: TDLS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_TDLS_TX_PACKET_THRESHOLD                "gTDLSTxPacketThreshold"
#define CFG_TDLS_TX_PACKET_THRESHOLD_MIN            (0)
#define CFG_TDLS_TX_PACKET_THRESHOLD_MAX            (4294967295UL)
#define CFG_TDLS_TX_PACKET_THRESHOLD_DEFAULT        (40)

/*
 * <ini>
 * gTDLSMaxDiscoveryAttempt - Attempts for sending TDLS discovery requests.
 * @Min: 1
 * @Max: 100
 * @Default: 5
 *
 * This ini is used to configure the number of failures of discover request,
 * when exceeded, the peer is assumed to be not TDLS capable and no further
 * TDLS Discovery request is made.
 *
 * Related: gEnableTDLSSupport.
 *
 * Supported Feature: TDLS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_TDLS_MAX_DISCOVERY_ATTEMPT              "gTDLSMaxDiscoveryAttempt"
#define CFG_TDLS_MAX_DISCOVERY_ATTEMPT_MIN          (1)
#define CFG_TDLS_MAX_DISCOVERY_ATTEMPT_MAX          (100)
#define CFG_TDLS_MAX_DISCOVERY_ATTEMPT_DEFAULT      (5)

/*
 * <ini>
 * gTDLSIdleTimeout - Duration within which number of TX / RX frames meet the
 * criteria for TDLS teardown.
 * @Min: 500
 * @Max: 40000
 * @Default: 5000
 *
 * This ini is used to configure the time period (in ms) to evaluate whether
 * the number of Tx/Rx packets exceeds gTDLSIdlePacketThreshold and thus meets
 * criteria for TDLS teardown.
 * Teardown notification interval (gTDLSIdleTimeout) should be multiple of
 * setup notification (gTDLSTxStatsPeriod) interval.
 * e.g.
 *      if setup notification (gTDLSTxStatsPeriod) interval = 500, then
 *      teardown notification (gTDLSIdleTimeout) interval should be 1000,
 *      1500, 2000, 2500...
 *
 * Related: gEnableTDLSSupport.
 *
 * Supported Feature: TDLS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_TDLS_IDLE_TIMEOUT                       "gTDLSIdleTimeout"
#define CFG_TDLS_IDLE_TIMEOUT_MIN                   (500)
#define CFG_TDLS_IDLE_TIMEOUT_MAX                   (40000)
#define CFG_TDLS_IDLE_TIMEOUT_DEFAULT               (5000)


/*
 * <ini>
 * gTDLSIdlePacketThreshold - Number of idle packet.
 * @Min: 0
 * @Max: 40000
 * @Default: 3
 *
 * This ini is used to configure the number of Tx/Rx packet, below which
 * within last gTDLSTxStatsPeriod period is considered as idle condition.
 *
 * Related: gEnableTDLSSupport.
 *
 * Supported Feature: TDLS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_TDLS_IDLE_PACKET_THRESHOLD              "gTDLSIdlePacketThreshold"
#define CFG_TDLS_IDLE_PACKET_THRESHOLD_MIN          (0)
#define CFG_TDLS_IDLE_PACKET_THRESHOLD_MAX          (40000)
#define CFG_TDLS_IDLE_PACKET_THRESHOLD_DEFAULT      (3)

/*
 * <ini>
 * gTDLSRSSITriggerThreshold - RSSI threshold for TDLS connection.
 * @Min: -120
 * @Max: 0
 * @Default: -75
 *
 * This ini is used to configure the absolute value (in dB) of the peer RSSI,
 * below which a TDLS setup request is triggered.
 *
 * Related: gEnableTDLSSupport.
 *
 * Supported Feature: TDLS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_TDLS_RSSI_TRIGGER_THRESHOLD             "gTDLSRSSITriggerThreshold"
#define CFG_TDLS_RSSI_TRIGGER_THRESHOLD_MIN         (-120)
#define CFG_TDLS_RSSI_TRIGGER_THRESHOLD_MAX         (0)
#define CFG_TDLS_RSSI_TRIGGER_THRESHOLD_DEFAULT     (-75)

/*
 * <ini>
 * gTDLSRSSITeardownThreshold - RSSI threshold for TDLS teardown.
 * @Min: -120
 * @Max: 0
 * @Default: -75
 *
 * This ini is used to configure the absolute value (in dB) of the peer RSSI,
 * when exceed, a TDLS teardown is triggered.
 *
 * Related: gEnableTDLSSupport.
 *
 * Supported Feature: TDLS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_TDLS_RSSI_TEARDOWN_THRESHOLD            "gTDLSRSSITeardownThreshold"
#define CFG_TDLS_RSSI_TEARDOWN_THRESHOLD_MIN        (-120)
#define CFG_TDLS_RSSI_TEARDOWN_THRESHOLD_MAX        (0)
#define CFG_TDLS_RSSI_TEARDOWN_THRESHOLD_DEFAULT    (-75)

/*
 * <ini>
 * gTDLSRSSIDelta - Delta value for the peer RSSI that can trigger teardown.
 * @Min: -30
 * @Max: 0
 * @Default: -20
 *
 * This ini is used to .
 * This ini is used to configure delta for peer RSSI such that if Peer RSSI
 * is less than AP RSSI plus delta will trigger a teardown.
 *
 * Related: gEnableTDLSSupport.
 *
 * Supported Feature: TDLS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_TDLS_RSSI_DELTA                         "gTDLSRSSIDelta"
#define CFG_TDLS_RSSI_DELTA_MIN                     (-30)
#define CFG_TDLS_RSSI_DELTA_MAX                     (0)
#define CFG_TDLS_RSSI_DELTA_DEFAULT                 (-20)

/*
 * <ini>
 * gTDLSUapsdMask - ACs to setup U-APSD for TDLS Sta.
 * @Min: 0
 * @Max: 0x0F
 * @Default: 0x0F
 *
 * This ini is used to configure the ACs for which mask needs to be enabled.
 * 0x1: Background	0x2: Best effort
 * 0x4: Video		0x8:Voice
 *
 * Related: gEnableTDLSSupport.
 *
 * Supported Feature: TDLS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_TDLS_QOS_WMM_UAPSD_MASK_NAME            "gTDLSUapsdMask"
#define CFG_TDLS_QOS_WMM_UAPSD_MASK_MIN             (0)
#define CFG_TDLS_QOS_WMM_UAPSD_MASK_MAX             (0x0F)
#define CFG_TDLS_QOS_WMM_UAPSD_MASK_DEFAULT         (0x0F)

/*
 * <ini>
 * gEnableTDLSBufferSta - Controls the TDLS buffer.
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to control the TDLS buffer.
 * Buffer STA is not enabled in CLD 2.0 yet.
 *
 * Related: gEnableTDLSSupport.
 *
 * Supported Feature: TDLS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_TDLS_BUFFER_STA_SUPPORT_ENABLE          "gEnableTDLSBufferSta"
#define CFG_TDLS_BUFFER_STA_SUPPORT_ENABLE_MIN      (0)
#define CFG_TDLS_BUFFER_STA_SUPPORT_ENABLE_MAX      (1)
#define CFG_TDLS_BUFFER_STA_SUPPORT_ENABLE_DEFAULT  (1)

/*
 * <ini>
 * gTDLSPuapsdInactivityTime - Peer UAPSD Inactivity time.
 * @Min: 0
 * @Max: 10
 * @Default: 0
 *
 * This ini is used to configure peer uapsd inactivity time.
 *
 * Related: gEnableTDLSSupport.
 *
 * Supported Feature: TDLS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_TDLS_PUAPSD_INACTIVITY_TIME             "gTDLSPuapsdInactivityTime"
#define CFG_TDLS_PUAPSD_INACTIVITY_TIME_MIN         (0)
#define CFG_TDLS_PUAPSD_INACTIVITY_TIME_MAX         (10)
#define CFG_TDLS_PUAPSD_INACTIVITY_TIME_DEFAULT     (0)

/*
 * <ini>
 * gTDLSPuapsdRxFrameThreshold - Peer UAPSD Rx frame threshold.
 * @Min: 10
 * @Max: 20
 * @Default: 10
 *
 * This ini is used to configure maximum Rx frame during SP.
 *
 * Related: gEnableTDLSSupport.
 *
 * Supported Feature: TDLS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_TDLS_PUAPSD_RX_FRAME_THRESHOLD          "gTDLSPuapsdRxFrameThreshold"
#define CFG_TDLS_PUAPSD_RX_FRAME_THRESHOLD_MIN      (10)
#define CFG_TDLS_PUAPSD_RX_FRAME_THRESHOLD_MAX      (20)
#define CFG_TDLS_PUAPSD_RX_FRAME_THRESHOLD_DEFAULT  (10)

/*
 * <ini>
 * gTDLSPuapsdPTIWindow - This ini is used to configure peer traffic indication
 * window.
 * @Min: 1
 * @Max: 5
 * @Default: 2
 *
 * This ini is used to configure buffering time in number of beacon intervals.
 *
 * Related: gEnableTDLSSupport.
 *
 * Supported Feature: TDLS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_TDLS_PUAPSD_PEER_TRAFFIC_IND_WINDOW          "gTDLSPuapsdPTIWindow"
#define CFG_TDLS_PUAPSD_PEER_TRAFFIC_IND_WINDOW_MIN      (1)
#define CFG_TDLS_PUAPSD_PEER_TRAFFIC_IND_WINDOW_MAX      (5)
#define CFG_TDLS_PUAPSD_PEER_TRAFFIC_IND_WINDOW_DEFAULT  (2)

/*
 * <ini>
 * gTDLSPuapsdPTRTimeout - Peer Traffic Response timer duration in ms.
 * @Min: 0
 * @Max: 10000
 * @Default: 5000
 *
 * This ini is used to configure the peer traffic response timer duration
 * in ms.
 *
 * Related: gEnableTDLSSupport.
 *
 * Supported Feature: TDLS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_TDLS_PUAPSD_PEER_TRAFFIC_RSP_TIMEOUT         "gTDLSPuapsdPTRTimeout"
#define CFG_TDLS_PUAPSD_PEER_TRAFFIC_RSP_TIMEOUT_MIN     (0)
#define CFG_TDLS_PUAPSD_PEER_TRAFFIC_RSP_TIMEOUT_MAX     (10000)
#define CFG_TDLS_PUAPSD_PEER_TRAFFIC_RSP_TIMEOUT_DEFAULT (5000)

/*
 * <ini>
 * gTDLSExternalControl - Enable external TDLS control.
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable/disable external TDLS control.
 * TDLS external control works with TDLS implicit trigger. TDLS external
 * control allows a user to add a MAC address of potential TDLS peers so
 * that the CLD driver can initiate implicit TDLS setup to only those peers
 * when criteria for TDLS setup (throughput and RSSI threshold) is met.
 *
 * Related: gEnableTDLSSupport, gEnableTDLSImplicitTrigger.
 *
 * Supported Feature: TDLS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_TDLS_EXTERNAL_CONTROL                   "gTDLSExternalControl"
#define CFG_TDLS_EXTERNAL_CONTROL_MIN               (0)
#define CFG_TDLS_EXTERNAL_CONTROL_MAX               (1)
#define CFG_TDLS_EXTERNAL_CONTROL_DEFAULT           (1)

/*
 * This INI item is used to control subsystem restart(SSR) test framework
 * Set it's value to 1 to enable APPS trigerred SSR testing
 */
#define CFG_ENABLE_CRASH_INJECT         "gEnableForceTargetAssert"
#define CFG_ENABLE_CRASH_INJECT_MIN     (0)
#define CFG_ENABLE_CRASH_INJECT_MAX     (1)
#define CFG_ENABLE_CRASH_INJECT_DEFAULT (0)

/*
 * <ini>
 * gEnableTDLSOffChannel - Enables off-channel support for TDLS link.
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable/disable off-channel support for TDLS link.
 *
 * Related: gEnableTDLSSupport.
 *
 * Supported Feature: TDLS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_TDLS_OFF_CHANNEL_SUPPORT_ENABLE          "gEnableTDLSOffChannel"
#define CFG_TDLS_OFF_CHANNEL_SUPPORT_ENABLE_MIN      (0)
#define CFG_TDLS_OFF_CHANNEL_SUPPORT_ENABLE_MAX      (1)
#define CFG_TDLS_OFF_CHANNEL_SUPPORT_ENABLE_DEFAULT  (0)

/*
 * <ini>
 * gEnableTDLSWmmMode - Enables WMM support over TDLS link.
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable/disable WMM support over TDLS link.
 * This is required to be set to 1 for any TDLS and uAPSD functionality.
 *
 * Related: gEnableTDLSSupport.
 *
 * Supported Feature: TDLS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_TDLS_WMM_MODE_ENABLE                     "gEnableTDLSWmmMode"
#define CFG_TDLS_WMM_MODE_ENABLE_MIN                 (0)
#define CFG_TDLS_WMM_MODE_ENABLE_MAX                 (1)
#define CFG_TDLS_WMM_MODE_ENABLE_DEFAULT             (1)

/*
 * <ini>
 * gTDLSPrefOffChanNum - Preferred TDLS channel number when off-channel support
 * is enabled.
 * @Min: 1
 * @Max: 165
 * @Default: 36
 *
 * This ini is used to configure preferred TDLS channel number when off-channel
 * support is enabled.
 *
 * Related: gEnableTDLSSupport, gEnableTDLSOffChannel.
 *
 * Supported Feature: TDLS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_TDLS_PREFERRED_OFF_CHANNEL_NUM          "gTDLSPrefOffChanNum"
#define CFG_TDLS_PREFERRED_OFF_CHANNEL_NUM_MIN      (1)
#define CFG_TDLS_PREFERRED_OFF_CHANNEL_NUM_MAX      (165)
#define CFG_TDLS_PREFERRED_OFF_CHANNEL_NUM_DEFAULT  (36)

/*
 * <ini>
 * gTDLSPrefOffChanBandwidth - Preferred TDLS channel bandwidth when
 * off-channel support is enabled.
 * @Min: 0x01
 * @Max: 0x0F
 * @Default: 0x07
 *
 * This ini is used to configure preferred TDLS channel bandwidth when
 * off-channel support is enabled.
 * 0x1: 20 MHz	0x2: 40 MHz	0x4: 80 MHz	0x8: 160 MHz
 * When more than one bits are set then firmware starts from the highest and
 * selects one based on capability of peer.
 *
 * Related: gEnableTDLSSupport, gEnableTDLSOffChannel.
 *
 * Supported Feature: TDLS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_TDLS_PREFERRED_OFF_CHANNEL_BW           "gTDLSPrefOffChanBandwidth"
#define CFG_TDLS_PREFERRED_OFF_CHANNEL_BW_MIN      (0x01)
#define CFG_TDLS_PREFERRED_OFF_CHANNEL_BW_MAX      (0x0F)
#define CFG_TDLS_PREFERRED_OFF_CHANNEL_BW_DEFAULT  (0x07)

/*
 * <ini>
 * gEnableTDLSScan - Allow scan and maintain TDLS link.
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable/disable TDLS scan.
 *  0: If peer is not buffer STA capable and device is not sleep STA
 *     capable, then teardown TDLS link when scan is initiated. If peer
 *     is buffer STA and we can be sleep STA then TDLS link is maintained
 *     during scan.
 *  1: Maintain TDLS link and allow scan even if peer is not buffer STA
 *     capable and device is not sleep STA capable. There will be loss of
 *     Rx pkts since peer would not know when device moves away from tdls
 *     channel. Tx on TDLS link would stop when device moves away from tdls
 *     channel.
 *
 * Related: gEnableTDLSSupport.
 *
 * Supported Feature: TDLS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_TDLS_SCAN_ENABLE                       "gEnableTDLSScan"
#define CFG_TDLS_SCAN_ENABLE_MIN                   (0)
#define CFG_TDLS_SCAN_ENABLE_MAX                   (1)
#define CFG_TDLS_SCAN_ENABLE_DEFAULT               (1)

/*
 * <ini>
 * gTDLSPeerKickoutThreshold - TDLS peer kickout threshold to firmware.
 * @Min: 10
 * @Max: 5000
 * @Default: 96
 *
 * This ini is used to configure TDLS peer kickout threshold to firmware.
 *     Firmware will use this value to determine, when to send TDLS
 *     peer kick out event to host.
 *     E.g.
 *        if peer kick out threshold is 10, then firmware will wait for 10
 *        consecutive packet failures and then send TDLS kickout
 *        notification to host driver
 *
 * Related: gEnableTDLSSupport.
 *
 * Supported Feature: TDLS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_TDLS_PEER_KICKOUT_THRESHOLD            "gTDLSPeerKickoutThreshold"
#define CFG_TDLS_PEER_KICKOUT_THRESHOLD_MIN        (10)
#define CFG_TDLS_PEER_KICKOUT_THRESHOLD_MAX        (5000)
#define CFG_TDLS_PEER_KICKOUT_THRESHOLD_DEFAULT    (96)

#endif

/*
 * <ini>
 * gTDLSEnableDeferTime - Timer to defer for enabling TDLS on P2P listen.
 * @Min: 500
 * @Max: 6000
 * @Default: 2000
 *
 * This ini is used to set the timer to defer for enabling TDLS on P2P
 * listen (value in milliseconds).
 *
 * Related: gEnableTDLSSupport.
 *
 * Supported Feature: TDLS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_TDLS_ENABLE_DEFER_TIMER                "gTDLSEnableDeferTime"
#define CFG_TDLS_ENABLE_DEFER_TIMER_MIN            (500)
#define CFG_TDLS_ENABLE_DEFER_TIMER_MAX            (6000)
#define CFG_TDLS_ENABLE_DEFER_TIMER_DEFAULT        (2000)

/* Enable/Disable LPWR Image(cMEM uBSP) Transition */
#define CFG_ENABLE_LPWR_IMG_TRANSITION_NAME        "gEnableLpwrImgTransition"
#define CFG_ENABLE_LPWR_IMG_TRANSITION_MIN         (0)
#define CFG_ENABLE_LPWR_IMG_TRANSITION_MAX         (1)
#define CFG_ENABLE_LPWR_IMG_TRANSITION_DEFAULT     (0)

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

/* Enable debug for remain on channel issues */
#define CFG_DEBUG_P2P_REMAIN_ON_CHANNEL_NAME    "gDebugP2pRemainOnChannel"
#define CFG_DEBUG_P2P_REMAIN_ON_CHANNEL_DEFAULT (0)
#define CFG_DEBUG_P2P_REMAIN_ON_CHANNEL_MIN     (0)
#define CFG_DEBUG_P2P_REMAIN_ON_CHANNEL_MAX     (1)

/*
 * SAP ALLOW All Channels
 */
#define CFG_SAP_ALLOW_ALL_CHANNEL_PARAM_NAME          "gSapAllowAllChannel"
#define CFG_SAP_ALLOW_ALL_CHANNEL_PARAM_MIN           (0)
#define CFG_SAP_ALLOW_ALL_CHANNEL_PARAM_MAX           (1)
#define CFG_SAP_ALLOW_ALL_CHANNEL_PARAM_DEFAULT       (0)

#define CFG_DISABLE_LDPC_WITH_TXBF_AP             "gDisableLDPCWithTxbfAP"
#define CFG_DISABLE_LDPC_WITH_TXBF_AP_MIN         (0)
#define CFG_DISABLE_LDPC_WITH_TXBF_AP_MAX         (1)
#define CFG_DISABLE_LDPC_WITH_TXBF_AP_DEFAULT     (0)

/* Parameter to control VHT support in 2.4 GHz band */
#define CFG_ENABLE_VHT_FOR_24GHZ_NAME             "gEnableVhtFor24GHzBand"
#define CFG_ENABLE_VHT_FOR_24GHZ_MIN              (0)
#define CFG_ENABLE_VHT_FOR_24GHZ_MAX              (1)
#define CFG_ENABLE_VHT_FOR_24GHZ_DEFAULT          (0)

/*
 * gEnableVendorVhtFor24GHzBand:Parameter to control VHT support
 * based on vendor ie in 2.4 GHz band
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This parameter will enable SAP to read VHT capability in vendor ie in Assoc
 * Req and send VHT caps in Resp to establish connection in VHT Mode.
 * Supported Feature: SAP
 *
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_ENABLE_VENDOR_VHT_FOR_24GHZ_NAME      "gEnableVendorVhtFor24GHzBand"
#define CFG_ENABLE_VENDOR_VHT_FOR_24GHZ_MIN       (0)
#define CFG_ENABLE_VENDOR_VHT_FOR_24GHZ_MAX       (1)
#define CFG_ENABLE_VENDOR_VHT_FOR_24GHZ_DEFAULT   (1)

/*
 * <ini>
 * gMaxMediumTime - Set Maximum channel time
 * @Min: STAMIN
 * @Max: STAMAX
 * @Default: STADEF
 *
 * This ini is used to set default max channel time
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_MAX_MEDIUM_TIME                      "gMaxMediumTime"
#define CFG_MAX_MEDIUM_TIME_STAMIN               WNI_CFG_MAX_MEDIUM_TIME_STAMIN
#define CFG_MAX_MEDIUM_TIME_STAMAX               WNI_CFG_MAX_MEDIUM_TIME_STAMAX
#define CFG_MAX_MEDIUM_TIME_STADEFAULT           WNI_CFG_MAX_MEDIUM_TIME_STADEF

/*
 * <ini>
 * gEnableIbssHeartBeatOffload - Enable heart beat monitoring offload to FW
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to set default ibbs heartbeat offload
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_ENABLE_HEART_BEAT_OFFLOAD          "gEnableIbssHeartBeatOffload"
#define CFG_ENABLE_HEART_BEAT_OFFLOAD_MIN      (0)
#define CFG_ENABLE_HEART_BEAT_OFFLOAD_MAX      (1)
#define CFG_ENABLE_HEART_BEAT_OFFLOAD_DEFAULT  (1)

/*
 * <ini>
 * gAntennaDiversity - It will use to set Antenna diversity
 * @Min: 0
 * @Max: 3
 * @Default: 0
 *
 * This ini is used to set default Antenna diversity
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_ANTENNA_DIVERSITY_PARAM_NAME          "gAntennaDiversity"
#define CFG_ANTENNA_DIVERSITY_PARAM_MIN           (0)
#define CFG_ANTENNA_DIVERSITY_PARAM_MAX           (3)
#define CFG_ANTENNA_DIVERSITY_PARAM_DEFAULT       (0)

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
 * gEnableFwSelfRecovery - Enable/disable FW self-recovery for USB
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable/disable FW self-recovery
 * gEnableFwSelfRecovery = 0: Disabled
 * gEnableFwSelfRecovery = 1: Driver triggers SSR instead of triggering
 * kernel  panic after firmware crash.
 *
 * Related: gEnableSSR
 *
 * Supported Feature: SSR
 *
 * Usage: External
 *
 * </ini>
 */

#define CFG_ENABLE_FW_SELF_RECOVERY_NAME         "gEnableFwSelfRecovery"
#define CFG_ENABLE_FW_SELF_RECOVERY_DISABLE      (0)
#define CFG_ENABLE_FW_SELF_RECOVERY_ENABLE       (1)
#define CFG_ENABLE_FW_SELF_RECOVERY_DEFAULT      (CFG_ENABLE_FW_SELF_RECOVERY_DISABLE)

/* Macro to handle maximum receive AMPDU size configuration */
#define CFG_VHT_AMPDU_LEN_EXPONENT_NAME                "gVhtAmpduLenExponent"
#define CFG_VHT_AMPDU_LEN_EXPONENT_MIN                 (0)
#define CFG_VHT_AMPDU_LEN_EXPONENT_MAX                 (7)
#define CFG_VHT_AMPDU_LEN_EXPONENT_DEFAULT             (3)

#define CFG_VHT_MPDU_LEN_NAME                          "gVhtMpduLen"
#define CFG_VHT_MPDU_LEN_MIN                           (0)
#define CFG_VHT_MPDU_LEN_MAX                           (2)
#define CFG_VHT_MPDU_LEN_DEFAULT                       (0)

#define CFG_SAP_MAX_NO_PEERS                       "gSoftApMaxPeers"
#define CFG_SAP_MAX_NO_PEERS_MIN                   (1)
#define CFG_SAP_MAX_NO_PEERS_MAX                   (SIR_SAP_MAX_NUM_PEERS)
#define CFG_SAP_MAX_NO_PEERS_DEFAULT               (SIR_SAP_MAX_NUM_PEERS)

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

#define CFG_SAP_MAX_OFFLOAD_PEERS                  "gMaxOffloadPeers"
#define CFG_SAP_MAX_OFFLOAD_PEERS_MIN              (2)
#define CFG_SAP_MAX_OFFLOAD_PEERS_MAX              (5)
#define CFG_SAP_MAX_OFFLOAD_PEERS_DEFAULT          (2)

#define CFG_SAP_MAX_OFFLOAD_REORDER_BUFFS          "gMaxOffloadReorderBuffs"
#define CFG_SAP_MAX_OFFLOAD_REORDER_BUFFS_MIN      (0)
#define CFG_SAP_MAX_OFFLOAD_REORDER_BUFFS_MAX      (3)
#define CFG_SAP_MAX_OFFLOAD_REORDER_BUFFS_DEFAULT  (2)

#ifdef FEATURE_WLAN_RA_FILTERING
#define CFG_RA_FILTER_ENABLE_NAME                  "gRAFilterEnable"
#define CFG_RA_FILTER_ENABLE_MIN                   (0)
#define CFG_RA_FILTER_ENABLE_MAX                   (1)
#define CFG_RA_FILTER_ENABLE_DEFAULT               (1)

#define CFG_RA_RATE_LIMIT_INTERVAL_NAME            "gRArateLimitInterval"
#define CFG_RA_RATE_LIMIT_INTERVAL_MIN             (60)
#define CFG_RA_RATE_LIMIT_INTERVAL_MAX             (3600)
#define CFG_RA_RATE_LIMIT_INTERVAL_DEFAULT         (60) /*60 SEC */
#endif

/*
 * <ini>
 * gIgnorePeerErpInfo - Used for ignore peer infrormation
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to ignore default peer info
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_IGNORE_PEER_ERP_INFO_NAME      "gIgnorePeerErpInfo"
#define CFG_IGNORE_PEER_ERP_INFO_MIN       (0)
#define CFG_IGNORE_PEER_ERP_INFO_MAX       (1)
#define CFG_IGNORE_PEER_ERP_INFO_DEFAULT   (0)

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
 * g_auto_detect_power_failure_mode - auto detect power save failure mode
 * @Min: PMO_FW_TO_CRASH_ON_PWR_FAILURE
 * @Max: PMO_AUTO_PWR_FAILURE_DETECT_DISABLE
 * @Default: PMO_FW_TO_CRASH_ON_PWR_FAILURE
 *
 * This ini specifies the behavior of FW in case of
 * CHIP_POWER_SAVE_FAIL_DETECTED event
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_AUTO_DETECT_POWER_FAIL_MODE_NAME    "g_auto_detect_power_failure_mode"
#define CFG_AUTO_DETECT_POWER_FAIL_MODE_DEFAULT (PMO_FW_TO_CRASH_ON_PWR_FAILURE)
#define CFG_AUTO_DETECT_POWER_FAIL_MODE_MIN     (PMO_FW_TO_CRASH_ON_PWR_FAILURE)
#define CFG_AUTO_DETECT_POWER_FAIL_MODE_MAX \
					(PMO_AUTO_PWR_FAILURE_DETECT_DISABLE)
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

#ifdef WLAN_FEATURE_11W
/*
 * <ini>
 * pmfSaQueryMaxRetries - Control PMF SA query retries for SAP
 * @Min: 0
 * @Max: 20
 * @Default: 5
 *
 * This ini to set the number of PMF SA query retries for SAP
 *
 * Related: None.
 *
 * Supported Feature: PMF(11W)
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_PMF_SA_QUERY_MAX_RETRIES_NAME          "pmfSaQueryMaxRetries"
#define CFG_PMF_SA_QUERY_MAX_RETRIES_DEFAULT       (5)
#define CFG_PMF_SA_QUERY_MAX_RETRIES_MIN           (0)
#define CFG_PMF_SA_QUERY_MAX_RETRIES_MAX           (20)

/*
 * <ini>
 * pmfSaQueryRetryInterval - Control PMF SA query retry interval
 * for SAP in ms
 * @Min: 0
 * @Max: 2000
 * @Default: 200
 *
 * This ini to set the PMF SA query retry interval for SAP in ms
 *
 * Related: None.
 *
 * Supported Feature: PMF(11W)
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_PMF_SA_QUERY_RETRY_INTERVAL_NAME       "pmfSaQueryRetryInterval"
#define CFG_PMF_SA_QUERY_RETRY_INTERVAL_DEFAULT    (200)
#define CFG_PMF_SA_QUERY_RETRY_INTERVAL_MIN        (0)
#define CFG_PMF_SA_QUERY_RETRY_INTERVAL_MAX        (2000)
#endif

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

#define CFG_ENABLE_SAP_DFS_CH_SIFS_BURST_NAME      "gEnableSAPDfsChSifsBurst"
#define CFG_ENABLE_SAP_DFS_CH_SIFS_BURST_MIN       (0)
#define CFG_ENABLE_SAP_DFS_CH_SIFS_BURST_MAX       (1)
#define CFG_ENABLE_SAP_DFS_CH_SIFS_BURST_DEFAULT   (1)

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

/*
 * <ini>
 * gEnableSifsBurst - Enables Sifs Burst
 * @Min: 0
 * @Max: 3
 * @Default: 0
 *
 * Sifs burst mode configuration
 *     0) disabled
 *     1) enabled, but disabled for legacy mode
 *     3) enabled
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_ENABLE_SIFS_BURST                      "gEnableSifsBurst"
#define CFG_ENABLE_SIFS_BURST_MIN                  (0)
#define CFG_ENABLE_SIFS_BURST_MAX                  (3)
#define CFG_ENABLE_SIFS_BURST_DEFAULT              (0)

#ifdef WLAN_FEATURE_LPSS
#define CFG_ENABLE_LPASS_SUPPORT                          "gEnableLpassSupport"
#define CFG_ENABLE_LPASS_SUPPORT_DEFAULT                  (0)
#define CFG_ENABLE_LPASS_SUPPORT_MIN                      (0)
#define CFG_ENABLE_LPASS_SUPPORT_MAX                      (1)
#endif

/*
 * NaN feature support configuration
 * gEnableNanSupport = 0 means NaN is not supported
 * gEnableNanSupport = 1 means NaN is supported
 */
#ifdef WLAN_FEATURE_NAN
#define CFG_ENABLE_NAN_SUPPORT                          "gEnableNanSupport"
#define CFG_ENABLE_NAN_SUPPORT_DEFAULT                  (0)
#define CFG_ENABLE_NAN_SUPPORT_MIN                      (0)
#define CFG_ENABLE_NAN_SUPPORT_MAX                      (1)
#endif

#define CFG_ENABLE_SELF_RECOVERY                   "gEnableSelfRecovery"
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
#define CFG_DHCP_SERVER_OFFLOAD_SUPPORT_NAME      "gDHCPServerOffloadEnable"
#define CFG_DHCP_SERVER_OFFLOAD_SUPPORT_MIN       (0)
#define CFG_DHCP_SERVER_OFFLOAD_SUPPORT_MAX       (1)
#define CFG_DHCP_SERVER_OFFLOAD_SUPPORT_DEFAULT   (CFG_DHCP_SERVER_OFFLOAD_SUPPORT_MIN)

#define CFG_DHCP_SERVER_OFFLOAD_NUM_CLIENT_NAME     "gDHCPMaxNumClients"
#define CFG_DHCP_SERVER_OFFLOAD_NUM_CLIENT_MIN      (1)
#define CFG_DHCP_SERVER_OFFLOAD_NUM_CLIENT_MAX      (8)
#define CFG_DHCP_SERVER_OFFLOAD_NUM_CLIENT_DEFAULT  (CFG_DHCP_SERVER_OFFLOAD_NUM_CLIENT_MAX)

#define CFG_DHCP_SERVER_IP_NAME     "gDHCPServerIP"
#define CFG_DHCP_SERVER_IP_DEFAULT  ""
#endif /* DHCP_SERVER_OFFLOAD */

/*
 * <ini>
 * gSendDeauthBeforeCon - It will send deauth before connection
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to set default DeauthBeforeCon
 * If last disconnection was due to HB failure and we reconnect
 * to same AP next time, send Deauth before starting connection
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_ENABLE_DEAUTH_BEFORE_CONNECTION                  "gSendDeauthBeforeCon"
#define CFG_ENABLE_DEAUTH_BEFORE_CONNECTION_MIN              (0)
#define CFG_ENABLE_DEAUTH_BEFORE_CONNECTION_MAX              (1)
#define CFG_ENABLE_DEAUTH_BEFORE_CONNECTION_DEFAULT          (0)

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
 * gP2PListenDeferInterval - Defer Remain on channel for some duration
 * @Min: 100
 * @Max: 200
 * @Default: 100
 *
 * This ini is used to defer back to back RoC request when sta is
 * connected.
 * If back to back listen received when sta is connected then fw is
 * not getting enough time to spend on home channel so it leading to
 * heartbeat failure.
 *
 * Supported Feature: P2P
 *
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_P2P_LISTEN_DEFER_INTERVAL_NAME        "gP2PListenDeferInterval"
#define CFG_P2P_LISTEN_DEFER_INTERVAL_MIN         (100)
#define CFG_P2P_LISTEN_DEFER_INTERVAL_MAX         (200)
#define CFG_P2P_LISTEN_DEFER_INTERVAL_DEFAULT     (100)

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
 * gAP11ACOverride - Override 11AC in driver even if supplicant or hostapd
 * configures HT.
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable/disable 11AC override.
 * 1. P2P GO or SAP also follows start_bss and since p2p GO or SAP
 *    could not be  configured to setup VHT channel width in
 *    wpa_supplicant, driver can override 11AC.
 * 2. Android UI does not provide advanced configuration options
 *    for SoftAP
 *    Default override enabled for android. MDM shall
 *    disable it in ini
 *
 *
 * Supported Feature: SAP
 *
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_SAP_P2P_11AC_OVERRIDE_NAME             "gAP11ACOverride"
#define CFG_SAP_P2P_11AC_OVERRIDE_MIN              (0)
#define CFG_SAP_P2P_11AC_OVERRIDE_MAX              (1)
#define CFG_SAP_P2P_11AC_OVERRIDE_DEFAULT          (1)

#define CFG_SAP_DOT11MC               "gSapDot11mc"
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
 * In static display use case when APPS is in stand alone power save mode enable
 * active offload mode which helps FW to filter out MC/BC data packets to avoid
 * APPS wake up and save more power.
 *
 * By default enable active mode offload as it helps to save more power in
 * static display usecase(APPS stand alone power collapse).
 *
 * If active mode offload(gActiveModeOffload=1) is enabled then all applicable
 * data offload/filtering is enabled immediately in FW once config is available
 * in WLAN driver and FW caches this configuration accross suspend/resume
 *
 * If active mode offload is disabled(gActiveModeOffload=0) then all applicable
 * data offload/filtering is enabled during cfg80211 suspend and disabled
 * during cfg80211 resume
 *
 * Active mode offload feature is bydefault enabled for all targets
 */

#define CFG_ACTIVE_MODE_OFFLOAD            "gActiveModeOffload"
#define CFG_ACTIVE_MODE_OFFLOAD_MIN        (0)
#define CFG_ACTIVE_MODE_OFFLOAD_MAX        (1)
#define CFG_ACTIVE_MODE_OFFLOAD_DEFAULT    (1)

/*
 * 0: Disable BPF packet filter
 * 1: Enable BPF packet filter
 */
#define CFG_BPF_PACKET_FILTER_OFFLOAD           "gBpfFilterEnable"
#define CFG_BPF_PACKET_FILTER_OFFLOAD_MIN       (0)
#define CFG_BPF_PACKET_FILTER_OFFLOAD_MAX       (1)
#define CFG_BPF_PACKET_FILTER_OFFLOAD_DEFAULT   (1)

/*
 * <ini>
 * gCckChainMaskEnable - Used to enable/disable Cck ChainMask
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to set default Cck ChainMask
 * 0: disable the cck tx chain mask (default)
 * 1: enable the cck tx chain mask
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_TX_CHAIN_MASK_CCK          "gCckChainMaskEnable"
#define CFG_TX_CHAIN_MASK_CCK_MIN      (0)
#define CFG_TX_CHAIN_MASK_CCK_MAX      (1)
#define CFG_TX_CHAIN_MASK_CCK_DEFAULT  (0)
/*
 * <ini>
 * gTxChainMask1ss - Enables/disables tx chain Mask1ss
 * @Min: 0
 * @Max: 3
 * @Default: 1
 *
 * This ini is used to set default tx chain Mask1ss
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_TX_CHAIN_MASK_1SS       "gTxChainMask1ss"
#define CFG_TX_CHAIN_MASK_1SS_MIN      (0)
#define CFG_TX_CHAIN_MASK_1SS_MAX      (3)
#define CFG_TX_CHAIN_MASK_1SS_DEFAULT  (1)

/*
 * <ini>
 * gEnableSmartChainmask - Enable Smart Chainmask
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable/disable the Smart Chainmask feature via
 * the WMI_PDEV_PARAM_SMART_CHAINMASK_SCHEME firmware parameter.
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_ENABLE_SMART_CHAINMASK_NAME    "gEnableSmartChainmask"
#define CFG_ENABLE_SMART_CHAINMASK_MIN     (0)
#define CFG_ENABLE_SMART_CHAINMASK_MAX     (1)
#define CFG_ENABLE_SMART_CHAINMASK_DEFAULT (0)

/*
 * <ini>
 * gEnableAlternativeChainmask - Enable Co-Ex Alternative Chainmask
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable/disable the Co-ex Alternative Chainmask
 * feature via the WMI_PDEV_PARAM_ALTERNATIVE_CHAINMASK_SCHEME
 * firmware parameter.
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_ENABLE_COEX_ALT_CHAINMASK_NAME    "gEnableAlternativeChainmask"
#define CFG_ENABLE_COEX_ALT_CHAINMASK_MIN     (0)
#define CFG_ENABLE_COEX_ALT_CHAINMASK_MAX     (1)
#define CFG_ENABLE_COEX_ALT_CHAINMASK_DEFAULT (0)

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
 * gTxAggregationSize - Gives an option to configure Tx aggregation size
 * in no of MPDUs
 * @Min: 0
 * @Max: 64
 * @Default: 64
 *
 * gTxAggregationSize gives an option to configure Tx aggregation size
 * in no of MPDUs.This can be useful in debugging throughput issues
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_TX_AGGREGATION_SIZE      "gTxAggregationSize"
#define CFG_TX_AGGREGATION_SIZE_MIN      (0)
#define CFG_TX_AGGREGATION_SIZE_MAX      (64)
#define CFG_TX_AGGREGATION_SIZE_DEFAULT  (64)

/*
 * <ini>
 * gRxAggregationSize - Gives an option to configure Rx aggregation size
 * in no of MPDUs
 * @Min: 1
 * @Max: 64
 * @Default: 64
 *
 * gRxAggregationSize gives an option to configure Rx aggregation size
 * in no of MPDUs. This can be useful in debugging throughput issues
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_RX_AGGREGATION_SIZE      "gRxAggregationSize"
#define CFG_RX_AGGREGATION_SIZE_MIN      (1)
#define CFG_RX_AGGREGATION_SIZE_MAX      (64)
#define CFG_RX_AGGREGATION_SIZE_DEFAULT  (64)

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

/**
 * enum dot11p_mode - The 802.11p mode of operation
 * @WLAN_HDD_11P_DISABLED:   802.11p mode is disabled
 * @WLAN_HDD_11P_STANDALONE: 802.11p-only operation
 * @WLAN_HDD_11P_CONCURRENT: 802.11p and WLAN operate concurrently
 */
enum dot11p_mode {
	WLAN_HDD_11P_DISABLED = 0,
	WLAN_HDD_11P_STANDALONE,
	WLAN_HDD_11P_CONCURRENT,
};

#define CFG_DOT11P_MODE_NAME             "gDot11PMode"
#define CFG_DOT11P_MODE_DEFAULT          (WLAN_HDD_11P_DISABLED)
#define CFG_DOT11P_MODE_MIN              (WLAN_HDD_11P_DISABLED)
#define CFG_DOT11P_MODE_MAX              (WLAN_HDD_11P_CONCURRENT)

/*
 * <ini>
 * gEnable_go_cts2self_for_sta - Indicate firmware to stop NOA and
 * start using cts2self
 * @Min: 1
 * @Max: 1
 * @Default: 0
 *
 * When gEnable_go_cts2self_for_sta is
 * enabled  then if a legacy client connects to P2P GO,
 * Host will send a WMI VDEV command to FW to stop using NOA for P2P GO
 * and start using CTS2SELF.
 *
 *
 * Supported Feature: P2P
 *
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_ENABLE_GO_CTS2SELF_FOR_STA   "gEnable_go_cts2self_for_sta"
#define CFG_ENABLE_GO_CTS2SELF_FOR_STA_DEFAULT  (0)
#define CFG_ENABLE_GO_CTS2SELF_FOR_STA_MIN      (0)
#define CFG_ENABLE_GO_CTS2SELF_FOR_STA_MAX      (1)

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
#define CFG_DUAL_MAC_FEATURE_DISABLE_MAX          (4)
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

/* GPIO pin to toggle when capture tsf */
#define CFG_SET_TSF_GPIO_PIN_NAME                  "gtsf_gpio_pin"
#define CFG_SET_TSF_GPIO_PIN_MIN                   (0)
#define CFG_SET_TSF_GPIO_PIN_MAX                   (254)
#define TSF_GPIO_PIN_INVALID                       (255)
#define CFG_SET_TSF_GPIO_PIN_DEFAULT               (TSF_GPIO_PIN_INVALID)

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
 * Dense traffic threshold
 * traffic threshold required for dense roam scan
 * Measured in kbps
 */
#define CFG_ROAM_DENSE_TRAFFIC_THRESHOLD         "gtraffic_threshold"
#define CFG_ROAM_DENSE_TRAFFIC_THRESHOLD_MIN     (0)
#define CFG_ROAM_DENSE_TRAFFIC_THRESHOLD_MAX     (0xffffffff)
#define CFG_ROAM_DENSE_TRAFFIC_THRESHOLD_DEFAULT (400)

/*
 * <ini>
 * gvendor_acs_support - vendor based channel selection manager
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * Enabling this parameter will force driver to use user application based
 * channel selection algo instead of driver based auto channel selection
 * logic.
 *
 * Supported Feature: ACS
 *
 * Usage: External/Internal
 *
 * </ini>
 */
#define CFG_USER_AUTO_CHANNEL_SELECTION       "gvendor_acs_support"
#define CFG_USER_AUTO_CHANNEL_SELECTION_DISABLE   (0)
#define CFG_USER_AUTO_CHANNEL_SELECTION_ENABLE    (1)
#define CFG_USER_AUTO_CHANNEL_SELECTION_DEFAULT   (0)

/*
 * <ini>
 * gacs_support_for_dfs_lte_coex - acs support for lte coex and dfs event
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * Enabling this parameter will force driver to use user application based
 * channel selection algo for channel selection in case of dfs and lte
 * coex event.
 *
 * Supported Feature: ACS
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_USER_ACS_DFS_LTE           "gacs_support_for_dfs_lte_coex"
#define CFG_USER_ACS_DFS_LTE_DISABLE   (0)
#define CFG_USER_ACS_DFS_LTE_ENABLE    (1)
#define CFG_USER_ACS_DFS_LTE_DEFAULT   (0)

/*
 * Enabling gignore_peer_ht_opmode will enable 11g
 * protection only when there is a 11g AP in vicinity.
 */
#define CFG_IGNORE_PEER_HT_MODE_NAME       "gignore_peer_ht_opmode"
#define CFG_IGNORE_PEER_HT_MODE_MIN        (0)
#define CFG_IGNORE_PEER_HT_MODE_MAX        (1)
#define CFG_IGNORE_PEER_HT_MODE_DEFAULT    (0)

#ifdef WLAN_FEATURE_NAN_DATAPATH
/*
 * Enable NaN data path feature. NaN data path enables
 * NaN supported devices to exchange data over traditional
 * TCP/UDP network stack.
 */
#define CFG_ENABLE_NAN_DATAPATH_NAME    "genable_nan_datapath"
#define CFG_ENABLE_NAN_DATAPATH_MIN     (0)
#define CFG_ENABLE_NAN_DATAPATH_MAX     (1)
#define CFG_ENABLE_NAN_DATAPATH_DEFAULT (0)

/*
 * NAN channel on which NAN data interface to start
 */
#define CFG_ENABLE_NAN_NDI_CHANNEL_NAME    "gnan_datapath_ndi_channel"
#define CFG_ENABLE_NAN_NDI_CHANNEL_MIN     (6)
#define CFG_ENABLE_NAN_NDI_CHANNEL_MAX     (149)
#define CFG_ENABLE_NAN_NDI_CHANNEL_DEFAULT (6)
#endif

/*
 * Enable/Disable to initiate BUG report in case of fatal event
 * Default: Enable
 */
#define CFG_ENABLE_FATAL_EVENT_TRIGGER                 "gEnableFatalEvent"
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
#define DP_TRACE_CONFIG_DEFAULT_THRESH		(4)

/*
 * Number of intervals of BW timer to wait before enabling/disabling DP Trace.
 * Since throughput threshold to disable live logging for DP Trace is very low,
 * we calculate throughput based on # packets received in a second.
 * For example assuming bandwidth timer interval is 100ms, and if more than 4
 * packets are received in 10 * 100 ms interval, we want to disable DP Trace
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
 * Param 3: Default Verbosity (0-3)
 * Param 4: Proto Bitmap (uint8_t). Decimal Value.
 *          (decimal 62 = 0x3e)
 * e.g., to disable live mode, use the following param in the ini file.
 * gDptraceConfig = 0
 * e.g., to enable dptrace live mode and set the thresh as 4,
 * use the following param in the ini file.
 * gDptraceConfig = 1, 4
 */
#define CFG_ENABLE_DP_TRACE_CONFIG		"gDptraceConfig"
#define CFG_ENABLE_DP_TRACE_CONFIG_DEFAULT	"1, 8, 1, 126"

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

#define CFG_TGT_GTX_USR_CFG_NAME    "tgt_gtx_usr_cfg"
#define CFG_TGT_GTX_USR_CFG_MIN     (0)
#define CFG_TGT_GTX_USR_CFG_MAX     (32)
#define CFG_TGT_GTX_USR_CFG_DEFAULT (32)

#define CFG_SAP_INTERNAL_RESTART_NAME    "gEnableSapInternalRestart"
#define CFG_SAP_INTERNAL_RESTART_MIN     (0)
#define CFG_SAP_INTERNAL_RESTART_MAX     (1)
#define CFG_SAP_INTERNAL_RESTART_DEFAULT (1)

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
 * This parameter will avoid updating ap_sta_inactivity from hostapd.conf
 * file. If a station does not send anything in ap_max_inactivity seconds, an
 * empty data frame is sent to it in order to verify whether it is
 * still in range. If this frame is not ACKed, the station will be
 * disassociated and then deauthenticated. This feature is used to
 * clear station table of old entries when the STAs move out of the
 * range.
 * Default : Disable
 */
#define CFG_SAP_MAX_INACTIVITY_OVERRIDE_NAME     "gSapMaxInactivityOverride"
#define CFG_SAP_MAX_INACTIVITY_OVERRIDE_MIN      (0)
#define CFG_SAP_MAX_INACTIVITY_OVERRIDE_MAX      (1)
#define CFG_SAP_MAX_INACTIVITY_OVERRIDE_DEFAULT  (0)

/*
 * <ini>
 * rx_mode - Control to decide rx mode
 *
 * @Min: 0
 * @Max: (CFG_ENABLE_RX_THREAD | CFG_ENABLE_RPS | CFG_ENABLE_NAPI)
 * @Default: MDM_PLATFORM   -  0
 *           HELIUMPLUS     -  CFG_ENABLE_NAPI
 *           Other cases    -  (CFG_ENABLE_RX_THREAD | CFG_ENABLE_NAPI)
 *
 * This ini is used to decide mode for the rx path
 *
 * Supported Feature: NAPI
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_RX_MODE_NAME     "rx_mode"
#define CFG_RX_MODE_MIN      (0)
#define CFG_RX_MODE_MAX      (CFG_ENABLE_RX_THREAD | CFG_ENABLE_RPS | \
				 CFG_ENABLE_NAPI)
#ifdef MDM_PLATFORM
#define CFG_RX_MODE_DEFAULT  (0)
#elif defined(HELIUMPLUS)
#define CFG_RX_MODE_DEFAULT  CFG_ENABLE_NAPI
#else
#define CFG_RX_MODE_DEFAULT  (CFG_ENABLE_RX_THREAD | CFG_ENABLE_NAPI)
#endif

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

#ifdef WLAN_FEATURE_WOW_PULSE
/*
 * Enable/Disable  WOW PULSE feature
 * Set the wakeup pulse which FW use to wake up HOST
 * Default : Disable
 */
#define CFG_WOW_PULSE_SUPPORT_NAME     "gwow_pulse_support"
#define CFG_WOW_PULSE_SUPPORT_MIN      (0)
#define CFG_WOW_PULSE_SUPPORT_MAX      (1)
#define CFG_WOW_PULSE_SUPPORT_DEFAULT  (CFG_WOW_PULSE_SUPPORT_MIN)

/*
 * GPIO PIN for Pulse
 * Which PIN to send the Pulse
 */
#define CFG_WOW_PULSE_PIN_NAME         "gwow_pulse_pin"
#define CFG_WOW_PULSE_PIN_MIN          (CFG_SET_TSF_GPIO_PIN_MIN)
#define CFG_WOW_PULSE_PIN_MAX          (CFG_SET_TSF_GPIO_PIN_MAX)
#define CFG_WOW_PULSE_PIN_DEFAULT      (35)

/*
 * Pulse interval low
 * The interval of low level in the pulse
 * The value which defined by customer should between 160 and 480
 */
#define CFG_WOW_PULSE_INTERVAL_LOW_NAME     "gwow_pulse_interval_low"
#define CFG_WOW_PULSE_INTERVAL_LOW_MIN      (160)
#define CFG_WOW_PULSE_INTERVAL_LOW_MAX      (480)
#define CFG_WOW_PULSE_INTERVAL_LOW_DEFAULT  (180)

/*
 * Pulse interval high
 * The interval of high level in the pulse
 * The value which defined by customer should between 20 and 40
 */
#define CFG_WOW_PULSE_INTERVAL_HIGH_NAME    "gwow_pulse_interval_high"
#define CFG_WOW_PULSE_INTERVAL_HIGH_MIN     (20)
#define CFG_WOW_PULSE_INTERVAL_HIGH_MAX     (40)
#define CFG_WOW_PULSE_INTERVAL_HIGH_DEFAULT (20)
#endif

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

/*
 * This parameter will control SIFS burst duration in FW from 0 to 12 ms.
 * Default value is set to 8ms.
 */
#define CFG_SIFS_BURST_DURATION_NAME     "g_sifs_burst_duration"
#define CFG_SIFS_BURST_DURATION_MIN      (0)
#define CFG_SIFS_BURST_DURATION_MAX      (12)
#define CFG_SIFS_BURST_DURATION_DEFAULT  (8)

/* Optimize channel avoidance indication comming from firmware */
#define CFG_OPTIMIZE_CA_EVENT_NAME       "goptimize_chan_avoid_event"
#define CFG_OPTIMIZE_CA_EVENT_DISABLE    (0)
#define CFG_OPTIMIZE_CA_EVENT_ENABLE     (1)
#define CFG_OPTIMIZE_CA_EVENT_DEFAULT    (0)

/*
 * <ini>
 * fw_timeout_crash - Enable/Disable BUG ON
 * @Min: 0
 * @Max: 1
 * @Default: 0
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
#define CFG_CRASH_FW_TIMEOUT_DEFAULT    (0)

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
 * items standard_wext_control and private_wext_control.
 *
 */
enum hdd_wext_control {
	hdd_wext_disabled = 0,
	hdd_wext_deprecated = 1,
	hdd_wext_enabled = 2,
};

/*
 * <ini>
 * standard_wext_control - Standard wireless extensions control
 * @Min: 0
 * @Max: 2
 * @Default: 0
 *
 * Values are per enum hdd_wext_control.
 *
 * This ini is used to control access to standard wireless extensions
 * ioctls SIOCSIWCOMMIT (0x8B00) thru SIOCSIWPMKSA (0x8B36). The
 * functionality originally provided by these ioctls has been
 * completely superceeded by the functionality of cfg80211, and hence
 * by default support for these ioctls is disabled.
 *
 * Related: None
 *
 * Supported Feature: All
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_STANDARD_WEXT_CONTROL_NAME          "standard_wext_control"
#define CFG_STANDARD_WEXT_CONTROL_MIN           (hdd_wext_disabled)
#define CFG_STANDARD_WEXT_CONTROL_MAX           (hdd_wext_enabled)
#define CFG_STANDARD_WEXT_CONTROL_DEFAULT       (hdd_wext_disabled)

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
#define CFG_PER_ROAM_MONITOR_TIME                "gper_roam_mon_time"
#define CFG_PER_ROAM_MONITOR_TIME_MIN            (5)
#define CFG_PER_ROAM_MONITOR_TIME_MAX            (25)
#define CFG_PER_ROAM_MONTIOR_TIME_DEFAULT        (25)

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
 * gWowlPattern - WOW Pattern to used when PBM filtering is enabled
 * @Default:
 *
 * This ini is used to set the WOW Pattern to be used for PBM Filtering
 *
 * Related: gMaxWoWFilters
 *
 * Supported Feature: Power Save
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_WOWL_PATTERN_NAME                  "gWowlPattern"
#define CFG_WOWL_PATTERN_DEFAULT               ""

/*
 * <ini>
 * gMaxWoWFilters - Maximum WoW patterns that can be configured
 * @Min: 0
 * @Max: WOW_MAX_BITMAP_FILTERS(32)
 * @Default: WOW_MAX_BITMAP_FILTERS(32)
 *
 * This ini is used to set the maximum WoW patterns that can be configured
 *
 * Related: gWowlPattern
 *
 * Supported Feature: Power Save
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_MAX_WOW_FILTERS_NAME               "gMaxWoWFilters"
#define CFG_MAX_WOW_FILTERS_MIN                (0)
#define CFG_MAX_WOW_FILTERS_MAX                (WOW_MAX_BITMAP_FILTERS)
#define CFG_MAX_WOW_FILTERS_DEFAULT            (WOW_MAX_BITMAP_FILTERS)

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
#define CFG_PER_ROAM_MIN_CANDIDATE_RSSI           "gper_min_rssi_threshold_for_roam"
#define CFG_PER_ROAM_MIN_CANDIDATE_RSSI_MIN       (0)
#define CFG_PER_ROAM_MIN_CANDIDATE_RSSI_MAX       (96)
#define CFG_PER_ROAM_MIN_CANDIDATE_RSSI_DEFAULT   (83)


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
 * @Default: 0
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
#define CFG_AUTO_PS_ENABLE_TIMER_DEFAULT       (0)

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
 * gBmpsModListenInterval - Set BMPS Moderate Listen Interval
 * @Min: 1
 * @Max: 65535
 * @Default: 1
 *
 * This ini is used to set BMPS Moderate Listen Interval. If gPowerUsage
 * is set "Mod", this INI need to be set.
 *
 * Related: gEnableBmps, gPowerUsage
 *
 * Supported Feature: Power Save
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_BMPS_MODERATE_LI_NAME              "gBmpsModListenInterval"
#define CFG_BMPS_MODERATE_LI_MIN               (1)
#define CFG_BMPS_MODERATE_LI_MAX               (65535)
#define CFG_BMPS_MODERATE_LI_DEFAULT           (1)

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

/*
 * <ini>
 * gRuntimePMDelay - Set runtime pm's inactivity timer
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to set runtime pm's inactivity timer value.
 * the wlan driver will wait for this number of milliseconds of
 * inactivity before performing a runtime suspend.
 *
 * Related: gRuntimePM
 *
 * Supported Feature: Power Save
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_RUNTIME_PM_DELAY_NAME               "gRuntimePMDelay"
#define CFG_RUNTIME_PM_DELAY_MIN                (100)
#define CFG_RUNTIME_PM_DELAY_MAX                (10000)
#define CFG_RUNTIME_PM_DELAY_DEFAULT            (500)
#endif

/*
 * <ini>
 * gEnablePowerSaveOffload - Enable Power Save Offload
 * @Min: 0
 * @Max: 5
 * @Default: 0
 *
 * This ini is used to set Power Save Offload configuration:
 * Current values of gEnablePowerSaveOffload:
 * 0 -> Power save offload is disabled
 * 1 -> Legacy Power save enabled + Deep sleep Disabled
 * 2 -> QPower enabled + Deep sleep Disabled
 * 3 -> Legacy Power save enabled + Deep sleep Enabled
 * 4 -> QPower enabled + Deep sleep Enabled
 * 5 -> Duty cycling QPower enabled
 *
 * Related: None
 *
 * Supported Feature: Power Save
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_POWERSAVE_OFFLOAD_NAME                "gEnablePowerSaveOffload"
#define CFG_POWERSAVE_OFFLOAD_MIN                 (0)
#define CFG_POWERSAVE_OFFLOAD_MAX                 (PS_DUTY_CYCLING_QPOWER)
#define CFG_POWERSAVE_OFFLOAD_DEFAULT             (CFG_POWERSAVE_OFFLOAD_MIN)

/*
 * <ini>
 * gEnableWoW - Enable/Disable WoW
 * @Min: 0
 * @Max: 3
 * @Default: 3
 *
 * This ini is used to enable/disable WoW. Configurations are as follows:
 * 0 - Disable both magic pattern match and pattern byte match.
 * 1 - Enable magic pattern match on all interfaces.
 * 2 - Enable pattern byte match on all interfaces.
 * 3 - Enable both magic patter and pattern byte match on all interfaces.
 *
 * Related: None
 *
 * Supported Feature: Power Save
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_WOW_STATUS_NAME                    "gEnableWoW"
#define CFG_WOW_ENABLE_MIN                     (0)
#define CFG_WOW_ENABLE_MAX                     (3)
#define CFG_WOW_STATUS_DEFAULT                 (3)

#ifdef WLAN_FEATURE_EXTWOW_SUPPORT
/*
 * <ini>
 * gExtWoWgotoSuspend - Enable/Disable Extended WoW
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable/disable Extended WoW.
 *
 * Related: None
 *
 * Supported Feature: Power Save
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_EXTWOW_GO_TO_SUSPEND               "gExtWoWgotoSuspend"
#define CFG_EXTWOW_GO_TO_SUSPEND_MIN           (0)
#define CFG_EXTWOW_GO_TO_SUSPEND_MAX           (1)
#define CFG_EXTWOW_GO_TO_SUSPEND_DEFAULT       (1)

/*
 * <ini>
 * gExtWowApp1WakeupPinNumber - Set wakeup1 PIN number
 * @Min: 0
 * @Max: 255
 * @Default: 12
 *
 * This ini is used to set EXT WOW APP1 wakeup PIN number
 *
 * Related: None
 *
 * Supported Feature: Power Save
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_EXTWOW_APP1_WAKE_PIN_NUMBER            "gExtWowApp1WakeupPinNumber"
#define CFG_EXTWOW_APP1_WAKE_PIN_NUMBER_MIN        (0)
#define CFG_EXTWOW_APP1_WAKE_PIN_NUMBER_MAX        (255)
#define CFG_EXTWOW_APP1_WAKE_PIN_NUMBER_DEFAULT    (12)

/*
 * <ini>
 * gExtWowApp2WakeupPinNumber - Set wakeup2 PIN number
 * @Min: 0
 * @Max: 255
 * @Default: 16
 *
 * This ini is used to set EXT WOW APP2 wakeup PIN number
 *
 * Related: None
 *
 * Supported Feature: Power Save
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_EXTWOW_APP2_WAKE_PIN_NUMBER            "gExtWowApp2WakeupPinNumber"
#define CFG_EXTWOW_APP2_WAKE_PIN_NUMBER_MIN        (0)
#define CFG_EXTWOW_APP2_WAKE_PIN_NUMBER_MAX        (255)
#define CFG_EXTWOW_APP2_WAKE_PIN_NUMBER_DEFAULT    (16)

/*
 * <ini>
 * gExtWoWApp2KAInitPingInterval - Set Keep Alive Init Ping Interval
 * @Min: 0
 * @Max: 0xffffffff
 * @Default: 240
 *
 * This ini is used to set Keep Alive Init Ping Interval for EXT WOW
 *
 * Related: None
 *
 * Supported Feature: Power Save
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_EXTWOW_KA_INIT_PING_INTERVAL           "gExtWoWApp2KAInitPingInterval"
#define CFG_EXTWOW_KA_INIT_PING_INTERVAL_MIN       (0)
#define CFG_EXTWOW_KA_INIT_PING_INTERVAL_MAX       (0xffffffff)
#define CFG_EXTWOW_KA_INIT_PING_INTERVAL_DEFAULT   (240)

/*
 * <ini>
 * gExtWoWApp2KAMinPingInterval - Set Keep Alive Minimum Ping Interval
 * @Min: 0
 * @Max: 0xffffffff
 * @Default: 240
 *
 * This ini is used to set Keep Alive Minimum Ping Interval for EXT WOW
 *
 * Related: None
 *
 * Supported Feature: Power Save
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_EXTWOW_KA_MIN_PING_INTERVAL            "gExtWoWApp2KAMinPingInterval"
#define CFG_EXTWOW_KA_MIN_PING_INTERVAL_MIN        (0)
#define CFG_EXTWOW_KA_MIN_PING_INTERVAL_MAX        (0xffffffff)
#define CFG_EXTWOW_KA_MIN_PING_INTERVAL_DEFAULT    (240)

/*
 * <ini>
 * gExtWoWApp2KAMaxPingInterval - Set Keep Alive Maximum Ping Interval
 * @Min: 0
 * @Max: 0xffffffff
 * @Default: 1280
 *
 * This ini is used to set Keep Alive Maximum Ping Interval for EXT WOW
 *
 * Related: None
 *
 * Supported Feature: Power Save
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_EXTWOW_KA_MAX_PING_INTERVAL            "gExtWoWApp2KAMaxPingInterval"
#define CFG_EXTWOW_KA_MAX_PING_INTERVAL_MIN        (0)
#define CFG_EXTWOW_KA_MAX_PING_INTERVAL_MAX        (0xffffffff)
#define CFG_EXTWOW_KA_MAX_PING_INTERVAL_DEFAULT    (1280)

/*
 * <ini>
 * gExtWoWApp2KAIncPingInterval - Set Keep Alive increment of Ping Interval
 * @Min: 0
 * @Max: 0xffffffff
 * @Default: 4
 *
 * This ini is used to set Keep Alive increment of Ping Interval for EXT WOW
 *
 * Related: None
 *
 * Supported Feature: Power Save
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_EXTWOW_KA_INC_PING_INTERVAL            "gExtWoWApp2KAIncPingInterval"
#define CFG_EXTWOW_KA_INC_PING_INTERVAL_MIN        (0)
#define CFG_EXTWOW_KA_INC_PING_INTERVAL_MAX        (0xffffffff)
#define CFG_EXTWOW_KA_INC_PING_INTERVAL_DEFAULT    (4)

/*
 * <ini>
 * gExtWoWApp2KAIncPingInterval - Set TCP source port
 * @Min: 0
 * @Max: 65535
 * @Default: 5000
 *
 * This ini is used to set TCP source port when EXT WOW is enabled
 *
 * Related: None
 *
 * Supported Feature: Power Save
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_EXTWOW_TCP_SRC_PORT                    "gExtWoWApp2TcpSrcPort"
#define CFG_EXTWOW_TCP_SRC_PORT_MIN                (0)
#define CFG_EXTWOW_TCP_SRC_PORT_MAX                (65535)
#define CFG_EXTWOW_TCP_SRC_PORT_DEFAULT            (5000)

/*
 * <ini>
 * gExtWoWApp2TcpDstPort - Set TCP Destination port
 * @Min: 0
 * @Max: 65535
 * @Default: 5001
 *
 * This ini is used to set TCP Destination port when EXT WOW is enabled
 *
 * Related: None
 *
 * Supported Feature: Power Save
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_EXTWOW_TCP_DST_PORT                    "gExtWoWApp2TcpDstPort"
#define CFG_EXTWOW_TCP_DST_PORT_MIN                (0)
#define CFG_EXTWOW_TCP_DST_PORT_MAX                (65535)
#define CFG_EXTWOW_TCP_DST_PORT_DEFAULT            (5001)

/*
 * <ini>
 * gExtWoWApp2TcpTxTimeout - Set TCP tx timeout
 * @Min: 0
 * @Max: 0xffffffff
 * @Default: 200
 *
 * This ini is used to set TCP Tx timeout when EXT WOW is enabled
 *
 * Related: None
 *
 * Supported Feature: Power Save
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_EXTWOW_TCP_TX_TIMEOUT                  "gExtWoWApp2TcpTxTimeout"
#define CFG_EXTWOW_TCP_TX_TIMEOUT_MIN              (0)
#define CFG_EXTWOW_TCP_TX_TIMEOUT_MAX              (0xffffffff)
#define CFG_EXTWOW_TCP_TX_TIMEOUT_DEFAULT          (200)

/*
 * <ini>
 * gExtWoWApp2TcpRxTimeout - Set TCP rx timeout
 * @Min: 0
 * @Max: 0xffffffff
 * @Default: 200
 *
 * This ini is used to set TCP Rx timeout when EXT WOW is enabled
 *
 * Related: None
 *
 * Supported Feature: Power Save
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_EXTWOW_TCP_RX_TIMEOUT                  "gExtWoWApp2TcpRxTimeout"
#define CFG_EXTWOW_TCP_RX_TIMEOUT_MIN              (0)
#define CFG_EXTWOW_TCP_RX_TIMEOUT_MAX              (0xffffffff)
#define CFG_EXTWOW_TCP_RX_TIMEOUT_DEFAULT          (200)
#endif

/*
 * <ini>
 * gper_roam_enabled - To enabled/disable PER based roaming in FW
 * @Min: 0
 * @Max: 3
 * @Default: 0
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
#define CFG_PER_ROAM_ENABLE_NAME           "gper_roam_enabled"
#define CFG_PER_ROAM_ENABLE_MIN            (0)
#define CFG_PER_ROAM_ENABLE_MAX            (3)
#define CFG_PER_ROAM_ENABLE_DEFAULT        (3)

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
#define CFG_PER_ROAM_CONFIG_HIGH_RATE_TH_NAME    "gper_roam_high_rate_th"
#define CFG_PER_ROAM_CONFIG_HIGH_RATE_TH_MIN     (10)
#define CFG_PER_ROAM_CONFIG_HIGH_RATE_TH_MAX     (0xffffffff)
#define CFG_PER_ROAM_CONFIG_HIGH_RATE_TH_DEFAULT (400)

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
#define CFG_PER_ROAM_CONFIG_LOW_RATE_TH_NAME    "gper_roam_low_rate_th"
#define CFG_PER_ROAM_CONFIG_LOW_RATE_TH_MIN     (10)
#define CFG_PER_ROAM_CONFIG_LOW_RATE_TH_MAX     (0xffffffff)
#define CFG_PER_ROAM_CONFIG_LOW_RATE_TH_DEFAULT (200)

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
#define CFG_PER_ROAM_CONFIG_RATE_TH_PERCENT_NAME      "gper_roam_th_percent"
#define CFG_PER_ROAM_CONFIG_RATE_TH_PERCENT_MIN       (10)
#define CFG_PER_ROAM_CONFIG_RATE_TH_PERCENT_MAX       (100)
#define CFG_PER_ROAM_CONFIG_RATE_TH_PERCENT_DEFAULT   (60)

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
#define CFG_PER_ROAM_REST_TIME_NAME     "gper_roam_rest_time"
#define CFG_PER_ROAM_REST_TIME_MIN      (10)
#define CFG_PER_ROAM_REST_TIME_MAX      (3600)
#define CFG_PER_ROAM_REST_TIME_DEFAULT  (300)

/*
 * <ini>
 * g_is_fils_enabled - Enable/Disable FILS support in driver
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable/disable FILS support in driver
 * Driver will update config to supplicant based on this config.
 *
 * Related: None
 *
 * Supported Feature: FILS
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_IS_FILS_ENABLED_NAME    "g_is_fils_enabled"
#define CFG_IS_FILS_ENABLED_DEFAULT (1)
#define CFG_IS_FILS_ENABLED_MIN     (0)
#define CFG_IS_FILS_ENABLED_MAX     (1)

/*
 * <ini>
 * g_enable_bcast_probe_rsp - Enable Broadcast probe response.
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable/disable broadcast probe response.
 * If this is disabled then OCE ini oce_sta_enable will also be
 * disabled and OCE IE will not be sent in frames.
 *
 * Related: None
 *
 * Supported Feature: FILS
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ENABLE_BCAST_PROBE_RESP_NAME    "g_enable_bcast_probe_rsp"
#define CFG_ENABLE_BCAST_PROBE_RESP_MIN     (0)
#define CFG_ENABLE_BCAST_PROBE_RESP_MAX     (1)
#define CFG_ENABLE_BCAST_PROBE_RESP_DEFAULT (1)

/**
 * arp_ac_category - ARP access category
 * @Min: 0
 * @Max: 3
 * @Default: 3
 *
 * Firmware by default categorizes ARP packets with VOICE TID.
 * This ini shall be used to override the default configuration.
 * Access category enums are referenced in ieee80211_common.h
 * WME_AC_BE = 0 (Best effort)
 * WME_AC_BK = 1 (Background)
 * WME_AC_VI = 2 (Video)
 * WME_AC_VO = 3 (Voice)
 *
 * Related: none
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_ARP_AC_CATEGORY                "arp_ac_category"
#define CFG_ARP_AC_CATEGORY_MIN            (0)
#define CFG_ARP_AC_CATEGORY_MAX            (3)
#define CFG_ARP_AC_CATEGORY_DEFAULT        (3)

/*
 * <ini>
 * gActiveUcBpfMode - Control UC active BPF mode
 * @Min: 0 (disabled)
 * @Max: 2 (adaptive)
 * @Default: 0 (disabled)
 *
 * This config item controls UC BPF in active mode. There are 3 modes:
 *	0) disabled - BPF is disabled in active mode
 *	1) enabled - BPF is enabled for all packets in active mode
 *	2) adaptive - BPF is enabled for packets up to some throughput threshold
 *
 * Related: gActiveMcBcBpfMode
 *
 * Supported Feature: Active Mode BPF
 *
 * Usage: Internal/External
 * </ini>
 */
#define CFG_ACTIVE_UC_BPF_MODE_NAME    "gActiveUcBpfMode"
#define CFG_ACTIVE_UC_BPF_MODE_MIN     (ACTIVE_BPF_DISABLED)
#define CFG_ACTIVE_UC_BPF_MODE_MAX     (ACTIVE_BPF_MODE_COUNT - 1)
#define CFG_ACTIVE_UC_BPF_MODE_DEFAULT (ACTIVE_BPF_DISABLED)

/*
 * <ini>
 * gActiveMcBcBpfMode - Control MC/BC active BPF mode
 * @Min: 0 (disabled)
 * @Max: 1 (enabled)
 * @Default: 0 (disabled)
 *
 * This config item controls MC/BC BPF in active mode. There are 3 modes:
 *	0) disabled - BPF is disabled in active mode
 *	1) enabled - BPF is enabled for all packets in active mode
 *	2) adaptive - BPF is enabled for packets up to some throughput threshold
 *
 * Related: gActiveUcBpfMode
 *
 * Supported Feature: Active Mode BPF
 *
 * Usage: Internal/External
 * </ini>
 */
#define CFG_ACTIVE_MC_BC_BPF_MODE_NAME    "gActiveMcBcBpfMode"
#define CFG_ACTIVE_MC_BC_BPF_MODE_MIN     (ACTIVE_BPF_DISABLED)
#define CFG_ACTIVE_MC_BC_BPF_MODE_MAX     (ACTIVE_BPF_ENABLED)
#define CFG_ACTIVE_MC_BC_BPF_MODE_DEFAULT (ACTIVE_BPF_DISABLED)

/*
 * <ini>
 * acs_with_more_param- Enable acs calculation with more param.
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable acs calculation with more param.
 *
 * Related: NA
 *
 * Supported Feature: ACS
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_ACS_WITH_MORE_PARAM_NAME    "acs_with_more_param"
#define CFG_ACS_WITH_MORE_PARAM_MIN     (0)
#define CFG_ACS_WITH_MORE_PARAM_MAX     (1)
#define CFG_ACS_WITH_MORE_PARAM_DEFAULT (0)

/*
 * <ini>
 * AutoChannelSelectWeight - ACS channel weight
 * @Min: 0
 * @Max: 0xFFFFFFFF
 * @Default: 0x000000FF
 *
 * This ini is used to adjust weight of factors in
 * acs algorithm.
 *
 * Supported Feature: ACS
 *
 * Usage: Internal/External
 *
 * bits 0-3:   rssi weight
 * bits 4-7:   bss count weight
 * bits 8-11:  noise floor weight
 * bits 12-15: channel free weight
 * bits 16-19: tx power range weight
 * bits 20-23: tx power throughput weight
 * bits 24-31: reserved
 *
 * </ini>
 */
#define CFG_AUTO_CHANNEL_SELECT_WEIGHT            "AutoChannelSelectWeight"
#define CFG_AUTO_CHANNEL_SELECT_WEIGHT_MIN        (0)
#define CFG_AUTO_CHANNEL_SELECT_WEIGHT_MAX        (0xFFFFFFFF)
#define CFG_AUTO_CHANNEL_SELECT_WEIGHT_DEFAULT    (0x000000FF)

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

/**
 * enum l1ss_sleep_allowed - when L1ss Sleep is allowed on capable platforms
 * @L1SS_SLEEP_ALLOWED_NEVER: never allow L1ss Sleep
 * @L1SS_SLEEP_ALLOWED_STA_CONNECTED: allow in station mode (connected)
 * @L1SS_SLEEP_ALLOWED_STA_DISCONNECTED: allow in station mode (disconnected)
 * @L1SS_SLEEP_ALLOWED_ALWAYS: always allow L1ss Sleep
 */
enum l1ss_sleep_allowed {
	L1SS_SLEEP_ALLOWED_NEVER = 0,
	L1SS_SLEEP_ALLOWED_STA_CONNECTED = BIT(0),
	L1SS_SLEEP_ALLOWED_STA_DISCONNECTED = BIT(1),
	L1SS_SLEEP_ALLOWED_ALWAYS = 0xff
};

/*
 * <ini>
 * gL1ssSleepAllowed - Control when L1ss Sleep is allowed on capable platforms
 * @Min: 0 (disabled)
 * @Max: 255 (always allow)
 * @Default: 1 (station mode - connected)
 *
 * This config item controls when L1ss Sleep is allowed on capable platforms.
 * The default is connected station mode to support DTIM power savings.
 *	0) never allow
 *	1) station mode (connected)
 *	2) station mode (disconnected)
 *	255) always allow
 *
 * Related: N/A
 *
 * Supported Feature: L1ss Sleep
 *
 * Usage: Internal/External
 * </ini>
 */
#define CFG_L1SS_SLEEP_ALLOWED_NAME    "gL1ssSleepAllowed"
#define CFG_L1SS_SLEEP_ALLOWED_MIN     (L1SS_SLEEP_ALLOWED_NEVER)
#define CFG_L1SS_SLEEP_ALLOWED_MAX     (L1SS_SLEEP_ALLOWED_ALWAYS)
#define CFG_L1SS_SLEEP_ALLOWED_DEFAULT (L1SS_SLEEP_ALLOWED_STA_CONNECTED)

/*
 * <ini>
 * gEnableANI - Enable Adaptive Noise Immunity
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable or disable Adaptive Noise Immunity.
 *
 * Related: None
 *
 * Supported Feature: ANI
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ENABLE_ANI_NAME              "gEnableANI"
#define CFG_ENABLE_ANI_MIN               (0)
#define CFG_ENABLE_ANI_MAX               (1)
#define CFG_ENABLE_ANI_DEFAULT           (1)

/*
 * <ini>
 * g_qcn_ie_support - QCN IE Support
 * @Min: 0 (disabled)
 * @Max: 1 (enabled)
 * @Default: 1 (enabled)
 *
 * This config item is used to support QCN IE in probe/assoc/reassoc request
 * for STA mode. QCN IE support is not added for SAP mode.
 *
 * Related: N/A
 *
 * Supported Feature: N/A
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QCN_IE_SUPPORT_NAME    "g_qcn_ie_support"
#define CFG_QCN_IE_SUPPORT_MIN      0
#define CFG_QCN_IE_SUPPORT_MAX      1
#define CFG_QCN_IE_SUPPORT_DEFAULT  1

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

/* Begin of probe request IE whitelisting feature ini params */
/*
 * <ini>
 * g_enable_probereq_whitelist_ies - Enable IE white listing
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable/disable probe request IE white listing feature.
 * Values 0 and 1 are used to disable and enable respectively, by default this
 * feature is disabled.
 *
 * Related: None
 *
 * Supported Feature: Probe request IE whitelisting
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_PRB_REQ_IE_WHITELIST_NAME    "g_enable_probereq_whitelist_ies"
#define CFG_PRB_REQ_IE_WHITELIST_MIN     (0)
#define CFG_PRB_REQ_IE_WHITELIST_MAX     (1)
#define CFG_PRB_REQ_IE_WHITELIST_DEFAULT (0)

/*
 * For IE white listing in Probe Req, following ini parameters from
 * g_probe_req_ie_bitmap_0 to g_probe_req_ie_bitmap_7 are used. User needs to
 * input this values in hexa decimal format, when bit is set in bitmap,
 * corresponding IE needs to be included in probe request.
 *
 * Example:
 * ========
 * If IE 221 needs to be in the probe request, set the corresponding bit
 * as follows:
 * a= IE/32 = 221/32 = 6 = g_probe_req_ie_bitmap_6
 * b = IE modulo 32 = 29,
 * means set the bth bit in g_probe_req_ie_bitmap_a,
 * therefore set 29th bit in g_probe_req_ie_bitmap_6,
 * as a result, g_probe_req_ie_bitmap_6=20000000
 *
 * Note: For IE 221, its mandatory to set the gProbeReqOUIs.
 */

/*
 * <ini>
 * g_probe_req_ie_bitmap_0 - Used to set the bitmap of IEs from 0 to 31
 * @Min: 0x00000000
 * @Max: 0xFFFFFFFF
 * @Default: 0x00000000
 *
 * This ini is used to include the IEs from 0 to 31 in probe request,
 * when corresponding bit is set.
 *
 * Related: Need to enable g_enable_probereq_whitelist_ies.
 *
 * Supported Feature: Probe request ie whitelisting
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_PRB_REQ_IE_BIT_MAP0_NAME    "g_probe_req_ie_bitmap_0"
#define CFG_PRB_REQ_IE_BIT_MAP0_MIN     (0x00000000)
#define CFG_PRB_REQ_IE_BIT_MAP0_MAX     (0xFFFFFFFF)
#define CFG_PRB_REQ_IE_BIT_MAP0_DEFAULT (0x00000000)

/*
 * <ini>
 * g_probe_req_ie_bitmap_1 - Used to set the bitmap of IEs from 32 to 63
 * @Min: 0x00000000
 * @Max: 0xFFFFFFFF
 * @Default: 0x00000000
 *
 * This ini is used to include the IEs from 32 to 63 in probe request,
 * when corresponding bit is set.
 *
 * Related: Need to enable g_enable_probereq_whitelist_ies.
 *
 * Supported Feature: Probe request ie whitelisting
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_PRB_REQ_IE_BIT_MAP1_NAME    "g_probe_req_ie_bitmap_1"
#define CFG_PRB_REQ_IE_BIT_MAP1_MIN     (0x00000000)
#define CFG_PRB_REQ_IE_BIT_MAP1_MAX     (0xFFFFFFFF)
#define CFG_PRB_REQ_IE_BIT_MAP1_DEFAULT (0x00000000)

/*
 * <ini>
 * g_probe_req_ie_bitmap_2 - Used to set the bitmap of IEs from 64 to 95
 * @Min: 0x00000000
 * @Max: 0xFFFFFFFF
 * @Default: 0x00000000
 *
 * This ini is used to include the IEs from 64 to 95 in probe request,
 * when corresponding bit is set.
 *
 * Related: Need to enable g_enable_probereq_whitelist_ies.
 *
 * Supported Feature: Probe request ie whitelisting
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_PRB_REQ_IE_BIT_MAP2_NAME    "g_probe_req_ie_bitmap_2"
#define CFG_PRB_REQ_IE_BIT_MAP2_MIN     (0x00000000)
#define CFG_PRB_REQ_IE_BIT_MAP2_MAX     (0xFFFFFFFF)
#define CFG_PRB_REQ_IE_BIT_MAP2_DEFAULT (0x00000000)

/*
 * <ini>
 * g_probe_req_ie_bitmap_3 - Used to set the bitmap of IEs from 96 to 127
 * @Min: 0x00000000
 * @Max: 0xFFFFFFFF
 * @Default: 0x00000000
 *
 * This ini is used to include the IEs from 96 to 127 in probe request,
 * when corresponding bit is set.
 *
 * Related: Need to enable g_enable_probereq_whitelist_ies.
 *
 * Supported Feature: Probe request ie whitelisting
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_PRB_REQ_IE_BIT_MAP3_NAME    "g_probe_req_ie_bitmap_3"
#define CFG_PRB_REQ_IE_BIT_MAP3_MIN     (0x00000000)
#define CFG_PRB_REQ_IE_BIT_MAP3_MAX     (0xFFFFFFFF)
#define CFG_PRB_REQ_IE_BIT_MAP3_DEFAULT (0x00000000)

/*
 * <ini>
 * g_probe_req_ie_bitmap_4 - Used to set the bitmap of IEs from 128 to 159
 * @Min: 0x00000000
 * @Max: 0xFFFFFFFF
 * @Default: 0x00000000
 *
 * This ini is used to include the IEs from 128 to 159 in probe request,
 * when corresponding bit is set.
 *
 * Related: Need to enable g_enable_probereq_whitelist_ies.
 *
 * Supported Feature: Probe request ie whitelisting
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_PRB_REQ_IE_BIT_MAP4_NAME    "g_probe_req_ie_bitmap_4"
#define CFG_PRB_REQ_IE_BIT_MAP4_MIN     (0x00000000)
#define CFG_PRB_REQ_IE_BIT_MAP4_MAX     (0xFFFFFFFF)
#define CFG_PRB_REQ_IE_BIT_MAP4_DEFAULT (0x00000000)

/*
 * <ini>
 * g_probe_req_ie_bitmap_5 - Used to set the bitmap of IEs from 160 to 191
 * @Min: 0x00000000
 * @Max: 0xFFFFFFFF
 * @Default: 0x00000000
 *
 * This ini is used to include the IEs from 160 to 191 in probe request,
 * when corresponding bit is set.
 *
 * Related: Need to enable g_enable_probereq_whitelist_ies.
 *
 * Supported Feature: Probe request ie whitelisting
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_PRB_REQ_IE_BIT_MAP5_NAME    "g_probe_req_ie_bitmap_5"
#define CFG_PRB_REQ_IE_BIT_MAP5_MIN     (0x00000000)
#define CFG_PRB_REQ_IE_BIT_MAP5_MAX     (0xFFFFFFFF)
#define CFG_PRB_REQ_IE_BIT_MAP5_DEFAULT (0x00000000)

/*
 * <ini>
 * g_probe_req_ie_bitmap_6 - Used to set the bitmap of IEs from 192 to 223
 * @Min: 0x00000000
 * @Max: 0xFFFFFFFF
 * @Default: 0x00000000
 *
 * This ini is used to include the IEs from 192 to 223 in probe request,
 * when corresponding bit is set.
 *
 * Related: Need to enable g_enable_probereq_whitelist_ies.
 *
 * Supported Feature: Probe request ie whitelisting
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_PRB_REQ_IE_BIT_MAP6_NAME    "g_probe_req_ie_bitmap_6"
#define CFG_PRB_REQ_IE_BIT_MAP6_MIN     (0x00000000)
#define CFG_PRB_REQ_IE_BIT_MAP6_MAX     (0xFFFFFFFF)
#define CFG_PRB_REQ_IE_BIT_MAP6_DEFAULT (0x00000000)

/*
 * <ini>
 * g_probe_req_ie_bitmap_7 - Used to set the bitmap of IEs from 224 to 255
 * @Min: 0x00000000
 * @Max: 0xFFFFFFFF
 * @Default: 0x00000000
 *
 * This ini is used to include the IEs from 224 to 255 in probe request,
 * when corresponding bit is set.
 *
 * Related: Need to enable g_enable_probereq_whitelist_ies.
 *
 * Supported Feature: Probe request ie whitelisting
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_PRB_REQ_IE_BIT_MAP7_NAME    "g_probe_req_ie_bitmap_7"
#define CFG_PRB_REQ_IE_BIT_MAP7_MIN     (0x00000000)
#define CFG_PRB_REQ_IE_BIT_MAP7_MAX     (0xFFFFFFFF)
#define CFG_PRB_REQ_IE_BIT_MAP7_DEFAULT (0x00000000)

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
 * g_sap_chanswitch_beacon_cnt - channel switch beacon count
 * @Min: 1
 * @Max: 5
 * @Default: 5
 *
 * This ini is used to configure channel switch beacon count
 *
 * Related: none
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SAP_CH_SWITCH_BEACON_CNT         "g_sap_chanswitch_beacon_cnt"
#define CFG_SAP_CH_SWITCH_BEACON_CNT_MIN     (1)
#define CFG_SAP_CH_SWITCH_BEACON_CNT_MAX     (10)
#define CFG_SAP_CH_SWITCH_BEACON_CNT_DEFAULT (5)

/*
 * <ini>
 * g_sap_chanswitch_mode - channel switch mode
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to configure channel switch mode
 *
 * Related: none
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SAP_CH_SWITCH_MODE         "g_sap_chanswitch_mode"
#define CFG_SAP_CH_SWITCH_MODE_MIN     (0)
#define CFG_SAP_CH_SWITCH_MODE_MAX     (1)
#define CFG_SAP_CH_SWITCH_MODE_DEFAULT (1)

/*
 * <ini>
 * g_fils_max_chan_guard_time - Set maximum channel guard time(ms)
 * @Min: 0
 * @Max: 10
 * @Default: 0
 *
 * This ini is used to set maximum channel guard time in milli seconds
 *
 * Related: None
 *
 * Supported Feature: FILS
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_FILS_MAX_CHAN_GUARD_TIME_NAME    "g_fils_max_chan_guard_time"
#define CFG_FILS_MAX_CHAN_GUARD_TIME_MIN     (0)
#define CFG_FILS_MAX_CHAN_GUARD_TIME_MAX     (10)
#define CFG_FILS_MAX_CHAN_GUARD_TIME_DEFAULT (0)

/**
 * gSetRTSForSIFSBursting - set rts for sifs bursting
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini set rts for sifs bursting
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SET_RTS_FOR_SIFS_BURSTING           "gSetRTSForSIFSBursting"
#define CFG_SET_RTS_FOR_SIFS_BURSTING_MIN       (0)
#define CFG_SET_RTS_FOR_SIFS_BURSTING_MAX       (1)
#define CFG_SET_RTS_FOR_SIFS_BURSTING_DEFAULT   (0)

/**
 * <ini>
 * gMaxMPDUsInAMPDU - max mpdus in ampdu
 * @Min: 0
 * @Max: 64
 * @Default: 0
 *
 * This ini configure max mpdus in ampdu
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_MAX_MPDUS_IN_AMPDU                  "gMaxMPDUsInAMPDU"
#define CFG_MAX_MPDUS_IN_AMPDU_MIN              (0)
#define CFG_MAX_MPDUS_IN_AMPDU_MAX              (64)
#define CFG_MAX_MPDUS_IN_AMPDU_DEFAULT          (0)

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
 * enum hdd_external_acs_policy - External ACS policy
 * @HDD_EXTERNAL_ACS_PCL_PREFERRED -Preferable for ACS to select a
 *	channel with non-zero pcl weight.
 * @HDD_EXTERNAL_ACS_PCL_MANDATORY -Mandatory for ACS to select a
 *	channel with non-zero pcl weight.
 *
 * enum hdd_external_acs_policy is used to select the ACS policy.
 *
 */
enum hdd_external_acs_policy {
	HDD_EXTERNAL_ACS_PCL_PREFERRED = 0,
	HDD_EXTERNAL_ACS_PCL_MANDATORY = 1,
};

/*
 * <ini>
 * external_acs_policy - External ACS policy control
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * Values are per enum hdd_external_acs_policy.
 *
 * This ini is used to control the external ACS policy.
 *
 * Related: None
 *
 * Supported Feature: ACS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_EXTERNAL_ACS_POLICY         "acs_policy"
#define CFG_EXTERNAL_ACS_POLICY_MIN     (HDD_EXTERNAL_ACS_PCL_PREFERRED)
#define CFG_EXTERNAL_ACS_POLICY_MAX     (HDD_EXTERNAL_ACS_PCL_MANDATORY)
#define CFG_EXTERNAL_ACS_POLICY_DEFAULT (HDD_EXTERNAL_ACS_PCL_PREFERRED)

/*
 * enum hdd_external_acs_policyl - Preferred freq band for external ACS
 * @HDD_EXTERNAL_ACS_FREQ_BAND_24GHZ -2.4GHz band
 * @HDD_EXTERNAL_ACS_FREQ_BAND_5GHZ -5GHz band
 *
 * enum hdd_external_acs_freq_band is used to select the freq band for ACS.
 *
 */
enum hdd_external_acs_freq_band {
	HDD_EXTERNAL_ACS_FREQ_BAND_24GHZ = 0,
	HDD_EXTERNAL_ACS_FREQ_BAND_5GHZ = 1,
};

/*
 * <ini>
 * external_acs_freq_band - External ACS freq band
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * Values are per enum hdd_external_acs_freq_band.
 *
 * This ini is used to select the ACS freq band. Currently
 * the external ACS module doesn't support channels from
 * both the bands. Once multiple band support is added in
 * ICM, this ini can be removed
 *
 * Related: None
 *
 * Supported Feature: ACS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_EXTERNAL_ACS_FREQ_BAND          "acs_freq_band"
#define CFG_EXTERNAL_ACS_FREQ_BAND_MIN      (HDD_EXTERNAL_ACS_FREQ_BAND_24GHZ)
#define CFG_EXTERNAL_ACS_FREQ_BAND_MAX      (HDD_EXTERNAL_ACS_FREQ_BAND_5GHZ)
#define CFG_EXTERNAL_ACS_FREQ_BAND_DEFAULT  (HDD_EXTERNAL_ACS_FREQ_BAND_24GHZ)

/*
 * gSapMaxMCSForTxData - sap 11n max mcs
 * @Min: 0
 * @Max: 383
 * @Default: 0
 *
 * This ini configure SAP 11n max mcs
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SAP_MAX_MCS_FOR_TX_DATA         "gSapMaxMCSForTxData"
#define CFG_SAP_MAX_MCS_FOR_TX_DATA_MIN     (0)
#define CFG_SAP_MAX_MCS_FOR_TX_DATA_MAX     (383)
#define CFG_SAP_MAX_MCS_FOR_TX_DATA_DEFAULT (0)

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
 * oce_sta_enable - Enable/disable oce feature for STA
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable/disable oce feature for STA
 *
 * Related: None
 *
 * Supported Feature: OCE
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_OCE_ENABLE_STA_NAME    "oce_sta_enable"
#define CFG_OCE_ENABLE_STA_MIN     (0)
#define CFG_OCE_ENABLE_STA_MAX     (1)
#define CFG_OCE_ENABLE_STA_DEFAULT (1)

/*
 * <ini>
 * oce_sap_enable - Enable/disable oce feature for SAP
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable/disable oce feature for SAP
 *
 * Related: None
 *
 * Supported Feature: OCE
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_OCE_ENABLE_SAP_NAME    "oce_sap_enable"
#define CFG_OCE_ENABLE_SAP_MIN     (0)
#define CFG_OCE_ENABLE_SAP_MAX     (1)
#define CFG_OCE_ENABLE_SAP_DEFAULT (1)

/*
 * <ini>
 * groam_disallow_duration -disallow duration before roaming
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
#define CFG_ROAM_DISALLOW_DURATION_NAME    "groam_disallow_duration"
#define CFG_ROAM_DISALLOW_DURATION_MIN     (0)
#define CFG_ROAM_DISALLOW_DURATION_MAX     (3600)
#define CFG_ROAM_DISALLOW_DURATION_DEFAULT (30)

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
#define CFG_ROAM_RSSI_CHANNEL_PENALIZATION_NAME    "grssi_channel_penalization"
#define CFG_ROAM_RSSI_CHANNEL_PENALIZATION_MIN     (0)
#define CFG_ROAM_RSSI_CHANNEL_PENALIZATION_MAX     (15)
#define CFG_ROAM_RSSI_CHANNEL_PENALIZATION_DEFAULT (5)

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
#define CFG_ROAM_NUM_DISALLOWED_APS_NAME    "groam_num_disallowed_aps"
#define CFG_ROAM_NUM_DISALLOWED_APS_MIN     (0)
#define CFG_ROAM_NUM_DISALLOWED_APS_MAX     (8)
#define CFG_ROAM_NUM_DISALLOWED_APS_DEFAULT (3)


/*
 * <ini>
 * gEnableNDIMacRandomization - When enabled this will randomize NDI Mac
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * When enabled this will randomize NDI Mac
 *
 *
 * Related: None
 *
 * Supported Feature: NAN
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_RANDOMIZE_NDI_MAC_NAME      "gEnableNDIMacRandomization"
#define CFG_RANDOMIZE_NDI_MAC_MIN       (0)
#define CFG_RANDOMIZE_NDI_MAC_MAX       (1)
#define CFG_RANDOMIZE_NDI_MAC_DEFAULT   (1)

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
 * gUpperBrssiThresh - Sets Upper threshold for beacon RSSI
 * @Min: 36
 * @Max: 66
 * @Default: 46
 *
 * This ini sets Upper beacon threshold for beacon RSSI in FW
 * Used to reduced RX chainmask in FW, once this threshold is
 * reached FW will switch to 1X1 (Single chain).
 *
 * Supported Feature: STA
 *
 * Usage: External
 *
 * </ini>
 */

#define CFG_UPPER_BRSSI_THRESH_NAME             "gUpperBrssiThresh"
#define CFG_UPPER_BRSSI_THRESH_MIN              (36)
#define CFG_UPPER_BRSSI_THRESH_MAX              (66)
#define CFG_UPPER_BRSSI_THRESH_DEFAULT          (46)

/*
 * <ini>
 * gLowerrBrssiThresh - Sets Lower threshold for beacon RSSI
 * @Min: 6
 * @Max: 36
 * @Default: 26
 *
 * This ini sets Lower beacon threshold for beacon RSSI in FW
 * Used to increase RX chainmask in FW, once this threshold is
 * reached FW will switch to 2X2 chain.
 *
 * Supported Feature: STA
 *
 * Usage: External
 *
 * </ini>
 */

#define CFG_LOWER_BRSSI_THRESH_NAME     "gLowerBrssiThresh"
#define CFG_LOWER_BRSSI_THRESH_MIN      (6)
#define CFG_LOWER_BRSSI_THRESH_MAX      (36)
#define CFG_LOWER_BRSSI_THRESH_DEFAULT  (26)

/*
 * <ini>
 * gDtim1ChRxEnable - Enable/Disable DTIM 1Chrx feature
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini Enables or Disables DTIM 1CHRX feature in FW
 * If this flag is set FW enables shutting off one chain
 * while going to power save.
 *
 * Supported Feature: STA
 *
 * Usage: External
 *
 * </ini>
 */

#define CFG_DTIM_1CHRX_ENABLE_NAME      "gDtim1ChRxEnable"
#define CFG_DTIM_1CHRX_ENABLE_MIN       (0)
#define CFG_DTIM_1CHRX_ENABLE_MAX       (1)
#define CFG_DTIM_1CHRX_ENABLE_DEFAULT   (1)

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
#define CFG_RSSI_WEIGHTAGE_NAME    "rssi_weightage"
#define CFG_RSSI_WEIGHTAGE_DEFAULT (20)
#define CFG_RSSI_WEIGHTAGE_MIN     (0)
#define CFG_RSSI_WEIGHTAGE_MAX     (100)

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
#define CFG_HT_CAPABILITY_WEIGHTAGE_NAME   "ht_caps_weightage"
#define CFG_HT_CAPABILITY_WEIGHTAGE_DEFAULT (2)
#define CFG_HT_CAPABILITY_WEIGHTAGE_MIN     (0)
#define CFG_HT_CAPABILITY_WEIGHTAGE_MAX     (100)

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
#define CFG_VHT_CAPABILITY_WEIGHTAGE_NAME    "vht_caps_weightage"
#define CFG_VHT_CAPABILITY_WEIGHTAGE_DEFAULT (1)
#define CFG_VHT_CAPABILITY_WEIGHTAGE_MIN     (0)
#define CFG_VHT_CAPABILITY_WEIGHTAGE_MAX     (100)

/*
 * <ini>
 * he_caps_weightage - HE caps Weightage to calculate best candidate
 * @Min: 0
 * @Max: 100
 * @Default: 1
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
#define CFG_HE_CAPABILITY_WEIGHTAGE_NAME    "he_caps_weightage"
#define CFG_HE_CAPABILITY_WEIGHTAGE_DEFAULT (2)
#define CFG_HE_CAPABILITY_WEIGHTAGE_MIN     (0)
#define CFG_HE_CAPABILITY_WEIGHTAGE_MAX     (100)

/*
 * <ini>
 * chan_width_weightage - Channel Width Weightage to calculate best candidate
 * @Min: 0
 * @Max: 100
 * @Default: 17
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
#define CFG_CHAN_WIDTH_WEIGHTAGE_NAME    "chan_width_weightage"
#define CFG_CHAN_WIDTH_WEIGHTAGE_DEFAULT (17)
#define CFG_CHAN_WIDTH_WEIGHTAGE_MIN     (0)
#define CFG_CHAN_WIDTH_WEIGHTAGE_MAX     (100)

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
 * gEnableFastPwrTransition - Configuration for fast power transition
 * @Min: 0
 * @Max: 2
 * @Default: 0
 *
 * This ini supported values:
 * 0x0: Phy register retention disabled (Higher timeline, Good for power)
 * 0x1: Phy register retention statically enabled
 * 0x2: Phy register retention enabled/disabled dynamically
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_ENABLE_PHY_REG  "gEnableFastPwrTransition"
#define CFG_PHY_REG_DEFAULT (0x0)
#define CFG_PHY_REG_MIN     (0x0)
#define CFG_PHY_REG_MAX     (0x2)

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
#define CFG_IS_SAE_ENABLED_DEFAULT (0)
#define CFG_IS_SAE_ENABLED_MIN     (0)
#define CFG_IS_SAE_ENABLED_MAX     (1)

/*
 * Type declarations
 */
#define CFG_CHAN_BAND_WEIGHTAGE_NAME    "chan_band_weightage"
#define CFG_CHAN_BAND_WEIGHTAGE_DEFAULT (2)
#define CFG_CHAN_BAND_WEIGHTAGE_MIN     (0)
#define CFG_CHAN_BAND_WEIGHTAGE_MAX     (100)

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
#define CFG_NSS_WEIGHTAGE_NAME    "nss_weightage"
#define CFG_NSS_WEIGHTAGE_DEFAULT (16)
#define CFG_NSS_WEIGHTAGE_MIN     (0)
#define CFG_NSS_WEIGHTAGE_MAX     (100)

/*
 * <ini>
 * beamforming_cap_weightage - Beam Forming Weightage to
 *                             calculate best candidate
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
#define CFG_BEAMFORMING_CAP_WEIGHTAGE_NAME "beamforming_cap_weightage"
#define CFG_BEAMFORMING_CAP_WEIGHTAGE_DEFAULT (2)
#define CFG_BEAMFORMING_CAP_WEIGHTAGE_MIN     (0)
#define CFG_BEAMFORMING_CAP_WEIGHTAGE_MAX     (100)

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
#define CFG_PCL_WEIGHT_WEIGHTAGE_NAME "pcl_weightage"
#define CFG_PCL_WEIGHT_DEFAULT        (0)
#define CFG_PCL_WEIGHT_MIN            (0)
#define CFG_PCL_WEIGHT_MAX            (100)

/*
 * <ini>
 * channel_congestion_weightage - channel Congestion Weightage to
 * calculate best candidate
 * @Min: 0
 * @Max: 100
 * @Default: 5
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
#define CFG_CHANNEL_CONGESTION_WEIGHTAGE_NAME "channel_congestion_weightage"
#define CFG_CHANNEL_CONGESTION_WEIGHTAGE_DEFAULT (5)
#define CFG_CHANNEL_CONGESTION_WEIGHTAGE_MIN     (0)
#define CFG_CHANNEL_CONGESTION_WEIGHTAGE_MAX     (100)

/*
 * <ini>
 * oce_wan_weightage - OCE WAN DL capacity Weightage to calculate best candidate
 * @Min: 0
 * @Max: 100
 * @Default: 0
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
#define CFG_OCE_WAN_WEIGHTAGE_NAME    "oce_wan_weightage"
#define CFG_OCE_WAN_WEIGHTAGE_DEFAULT (0)
#define CFG_OCE_WAN_WEIGHTAGE_MIN     (0)
#define CFG_OCE_WAN_WEIGHTAGE_MAX     (100)

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
#define CFG_BEST_RSSI_THRESHOLD_NAME         "best_rssi_threshold"
#define CFG_BEST_RSSI_THRESHOLD_DEFAULT      (55)
#define CFG_BEST_RSSI_THRESHOLD_MIN          (0)
#define CFG_BEST_RSSI_THRESHOLD_MAX          (96)

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
#define CFG_GOOD_RSSI_THRESHOLD_NAME          "good_rssi_threshold"
#define CFG_GOOD_RSSI_THRESHOLD_DEFAULT       (70)
#define CFG_GOOD_RSSI_THRESHOLD_MIN           (0)
#define CFG_GOOD_RSSI_THRESHOLD_MAX           (96)

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
#define CFG_BAD_RSSI_THRESHOLD_NAME            "bad_rssi_threshold"
#define CFG_BAD_RSSI_THRESHOLD_DEFAULT         (80)
#define CFG_BAD_RSSI_THRESHOLD_MIN             (0)
#define CFG_BAD_RSSI_THRESHOLD_MAX             (96)

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
#define CFG_GOOD_RSSI_PCNT_NAME            "good_rssi_pcnt"
#define CFG_GOOD_RSSI_PCNT_DEFAULT         (80)
#define CFG_GOOD_RSSI_PCNT_MIN             (0)
#define CFG_GOOD_RSSI_PCNT_MAX             (100)

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
#define CFG_BAD_RSSI_PCNT_NAME            "bad_rssi_pcnt"
#define CFG_BAD_RSSI_PCNT_DEFAULT         (25)
#define CFG_BAD_RSSI_PCNT_MIN             (0)
#define CFG_BAD_RSSI_PCNT_MAX             (100)

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
#define CFG_GOOD_RSSI_BUCKET_SIZE_NAME            "good_rssi_bucket_size"
#define CFG_GOOD_RSSI_BUCKET_SIZE_DEFAULT         (5)
#define CFG_GOOD_RSSI_BUCKET_SIZE_MIN             (1)
#define CFG_GOOD_RSSI_BUCKET_SIZE_MAX             (10)

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
#define CFG_BAD_RSSI_BUCKET_SIZE_NAME            "bad_rssi_bucket_size"
#define CFG_BAD_RSSI_BUCKET_SIZE_DEFAULT         (5)
#define CFG_BAD_RSSI_BUCKET_SIZE_MIN             (1)
#define CFG_BAD_RSSI_BUCKET_SIZE_MAX             (10)

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
#define CFG_RSSI_PERF_5G_THRESHOLD_NAME    "rssi_pref_5g_rssi_thresh"
#define CFG_RSSI_PERF_5G_THRESHOLD_DEFAULT (76)
#define CFG_RSSI_PERF_5G_THRESHOLD_MIN     (0)
#define CFG_RSSI_PERF_5G_THRESHOLD_MAX     (96)

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
#define CFG_BAND_WIDTH_WEIGHT_PER_INDEX_NAME       "bandwidth_weight_per_index"
#define CFG_BAND_WIDTH_WEIGHT_PER_INDEX_DEFAULT    (0x6432190C)
#define CFG_BAND_WIDTH_WEIGHT_PER_INDEX_MIN        (0x00000000)
#define CFG_BAND_WIDTH_WEIGHT_PER_INDEX_MAX        (0x64646464)

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
#define CFG_NSS_WEIGHT_PER_INDEX_NAME       "nss_weight_per_index"
#define CFG_NSS_WEIGHT_PER_INDEX_DEFAULT    (0x6432190C)
#define CFG_NSS_WEIGHT_PER_INDEX_MIN        (0x00000000)
#define CFG_NSS_WEIGHT_PER_INDEX_MAX        (0x64646464)

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
#define CFG_BAND_WEIGHT_PER_INDEX_NAME      "band_weight_per_index"
#define CFG_BAND_WEIGHT_PER_INDEX_DEFAULT   (0x0000644B)
#define CFG_BAND_WEIGHT_PER_INDEX_MIN       (0x00000000)
#define CFG_BAND_WEIGHT_PER_INDEX_MAX       (0x64646464)

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
#define CFG_ESP_QBSS_SLOTS_NAME      "num_esp_qbss_slots"
#define CFG_ESP_QBSS_SLOTS_DEFAULT   (4)
#define CFG_ESP_QBSS_SLOTS_MIN       (1)
#define CFG_ESP_QBSS_SLOTS_MAX       (15)

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
#define CFG_ESP_QBSS_SCORE_IDX3_TO_0_NAME      "esp_qbss_score_idx3_to_0"
#define CFG_ESP_QBSS_SCORE_IDX3_TO_0_DEFAULT   (0x19326432)
#define CFG_ESP_QBSS_SCORE_IDX3_TO_0_MIN       (0x00000000)
#define CFG_ESP_QBSS_SCORE_IDX3_TO_0_MAX       (0x64646464)

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
#define CFG_ESP_QBSS_SCORE_IDX7_TO_4_NAME      "esp_qbss_score_idx7_to_4"
#define CFG_ESP_QBSS_SCORE_IDX7_TO_4_DEFAULT   (0x0000000A)
#define CFG_ESP_QBSS_SCORE_IDX7_TO_4_MIN       (0x00000000)
#define CFG_ESP_QBSS_SCORE_IDX7_TO_4_MAX       (0x64646464)


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
#define CFG_ESP_QBSS_SCORE_IDX11_TO_8_NAME      "esp_qbss_score_idx11_to_8"
#define CFG_ESP_QBSS_SCORE_IDX11_TO_8_DEFAULT   (0x00000000)
#define CFG_ESP_QBSS_SCORE_IDX11_TO_8_MIN       (0x00000000)
#define CFG_ESP_QBSS_SCORE_IDX11_TO_8_MAX       (0x64646464)


/*
 * <ini>
 * esp_qbss_score_idx15_to_12 - percentage for  esp/qbss load for slots 12-15
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
#define CFG_ESP_QBSS_SCORE_IDX15_TO_12_NAME      "esp_qbss_score_idx15_to_12"
#define CFG_ESP_QBSS_SCORE_IDX15_TO_12_DEFAULT   (0x00000000)
#define CFG_ESP_QBSS_SCORE_IDX15_TO_12_MIN       (0x00000000)
#define CFG_ESP_QBSS_SCORE_IDX15_TO_12_MAX       (0x64646464)

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
#define CFG_OCE_WAN_SLOTS_NAME      "num_oce_wan_slots"
#define CFG_OCE_WAN_SLOTS_DEFAULT   (8)
#define CFG_OCE_WAN_SLOTS_MIN       (1)
#define CFG_OCE_WAN_SLOTS_MAX       (15)

/*
 * <ini>
 * oce_wan_score_idx3_to_0 - percentage for OCE WAN metrics score for slots 0-3
 * @Min: 0x00000000
 * @Max: 0x64646464
 * @Default: 0x00000000
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
#define CFG_OCE_WAN_SCORE_IDX3_TO_0_NAME      "oce_wan_score_idx3_to_0"
#define CFG_OCE_WAN_SCORE_IDX3_TO_0_DEFAULT   (0x00000000)
#define CFG_OCE_WAN_SCORE_IDX3_TO_0_MIN       (0x00000000)
#define CFG_OCE_WAN_SCORE_IDX3_TO_0_MAX       (0x64646464)

/*
 * <ini>
 * oce_wan_score_idx7_to_4 - percentage for OCE WAN metrics score for slots 4-7
 * @Min: 0x00000000
 * @Max: 0x64646464
 * @Default: 0x64640000
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
#define CFG_OCE_WAN_SCORE_IDX7_TO_4_NAME      "oce_wan_score_idx7_to_4"
#define CFG_OCE_WAN_SCORE_IDX7_TO_4_DEFAULT   (0x64640000)
#define CFG_OCE_WAN_SCORE_IDX7_TO_4_MIN       (0x00000000)
#define CFG_OCE_WAN_SCORE_IDX7_TO_4_MAX       (0x64646464)

/*
 * <ini>
 * oce_wan_score_idx11_to_8 - percentage for OCE WAN metrics score for slot 8-11
 * @Min: 0x00000000
 * @Max: 0x64646464
 * @Default: 0x00000064
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
#define CFG_OCE_WAN_SCORE_IDX11_TO_8_NAME      "oce_wan_score_idx11_to_8"
#define CFG_OCE_WAN_SCORE_IDX11_TO_8_DEFAULT   (0x00000064)
#define CFG_OCE_WAN_SCORE_IDX11_TO_8_MIN       (0x00000000)
#define CFG_OCE_WAN_SCORE_IDX11_TO_8_MAX       (0x64646464)

/*
 * <ini>
 * oce_wan_score_idx15_to_12 - % for OCE WAN metrics score for slot 12-15
 * @Min: 0x00000000
 * @Max: 0x64646464
 * @Default: 0x00000000
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
#define CFG_OCE_WAN_SCORE_IDX15_TO_12_NAME      "oce_wan_score_idx15_to_12"
#define CFG_OCE_WAN_SCORE_IDX15_TO_12_DEFAULT   (0x00000000)
#define CFG_OCE_WAN_SCORE_IDX15_TO_12_MIN       (0x00000000)
#define CFG_OCE_WAN_SCORE_IDX15_TO_12_MAX       (0x64646464)

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
#define CFG_ENABLE_SCORING_FOR_ROAM_NAME     "enable_scoring_for_roam"
#define CFG_ENABLE_SCORING_FOR_ROAM_DEFAULT  (1)
#define CFG_ENABLE_SCORING_FOR_ROAM_MIN      (0)
#define CFG_ENABLE_SCORING_FOR_ROAM_MAX      (1)

/*
 * <ini>
 * force_rsne_override - force rsnie override from user
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable/disable test mode to force rsne override used in
 * security enhancement test cases to pass the RSNIE sent by user in
 * assoc request.
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: internal
 *
 * </ini>
 */
#define CFG_FORCE_RSNE_OVERRIDE_NAME    "force_rsne_override"
#define CFG_FORCE_RSNE_OVERRIDE_MIN     (0)
#define CFG_FORCE_RSNE_OVERRIDE_MAX     (1)
#define CFG_FORCE_RSNE_OVERRIDE_DEFAULT (0)

/*
 * <ini>
 * gChanSwitchHostapdRateEnabled - Enable/disable hostapd rate when doing SAP
 * channel switch
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to set supported rates calculated from hostapd.conf file
 * or not when doing SAP channel switch. It must set it to 0 when cross-band
 * channel switch happens such as from 2G to 5G or 5G to 2G.
 *
 * Related: When doing SAP channel switch, if gChanSwitchHostapdRateEnabled is
 * set to 1, supported rates will be calculated from hostapd.conf file,
 * if gChanSwitchHostapdRateEnabled is set to 0, supported rates will be
 * calculated from driver default rates.
 *
 * Supported Feature: SAP
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_CHAN_SWITCH_HOSTAPD_RATE_ENABLED_NAME \
	"gChanSwitchHostapdRateEnabled"
#define CFG_CHAN_SWITCH_HOSTAPD_RATE_ENABLED_MIN     (0)
#define CFG_CHAN_SWITCH_HOSTAPD_RATE_ENABLED_MAX     (1)
#define CFG_CHAN_SWITCH_HOSTAPD_RATE_ENABLED_DEFAULT (0)

/*
 * <ini>
 * g_mbo_candidate_rssi_thres - Candidate AP's minimum RSSI to accept
 * @Min: -120
 * @Max: 0
 * @Default: -72
 *
 * This ini specifies the minimum RSSI value a candidate should have to accept
 * it as a target for transition.
 *
 * Related: N/A
 *
 * Supported Feature: MBO
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_MBO_CANDIDATE_RSSI_THRESHOLD_NAME   "g_mbo_candidate_rssi_thres"
#define CFG_CANDIDATE_RSSI_THRESHOLD_DEFAULT    (-72)
#define CFG_CANDIDATE_RSSI_THRESHOLD_MIN        (-120)
#define CFG_CANDIDATE_RSSI_THRESHOLD_MAX        (0)

/*
 * <ini>
 * g_mbo_current_rssi_thres - Connected AP's RSSI threshold to consider a
 * transition
 * @Min: -120
 * @Max: 0
 * @Default: -65
 *
 * This ini is used to configure connected AP's RSSI threshold value to consider
 * a transition.
 *
 * Related: N/A
 *
 * Supported Feature: MBO
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_MBO_CURRENT_RSSI_THRESHOLD_NAME     "g_mbo_current_rssi_thres"
#define CFG_CURRENT_RSSI_THRESHOLD_DEFAULT      (-65)
#define CFG_CURRENT_RSSI_THRESHOLD_MIN          (-120)
#define CFG_CURRENT_RSSI_THRESHOLD_MAX          (0)

/*
 * <ini>
 * g_mbo_current_rssi_mcc_thres - connected AP's RSSI threshold value to prefer
 * against a MCC
 * @Min: -120
 * @Max: 0
 * @Default: -75
 *
 * This ini is used to configure connected AP's minimum RSSI threshold that is
 * preferred against a MCC case, if the candidate can cause MCC.
 *
 * Related: N/A
 *
 * Supported Feature: MBO
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_MBO_CUR_RSSI_MCC_THRESHOLD_NAME       "g_mbo_current_rssi_mcc_thres"
#define CFG_MBO_CUR_RSSI_MCC_THRESHOLD_DEFAULT    (-75)
#define CFG_MBO_CUR_RSSI_MCC_THRESHOLD_MIN        (-120)
#define CFG_MBO_CUR_RSSI_MCC_THRESHOLD_MAX        (0)

/*
 * <ini>
 * g_mbo_candidate_rssi_btc_thres -  Candidate AP's minimum RSSI threshold to
 * prefer it even in case of BT coex
 * @Min: -120
 * @Max: 0
 * @Default: -70
 *
 * This ini is used to configure candidate AP's minimum RSSI threshold to prefer
 * it for transition even in case of BT coex.
 *
 * Related: N/A
 *
 * Supported Feature: MBO
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_MBO_CAND_RSSI_BTC_THRESHOLD_NAME    "g_mbo_candidate_rssi_btc_thres"
#define CFG_MBO_CAND_RSSI_BTC_THRESHOLD_DEFAULT (-70)
#define CFG_MBO_CAND_RSSI_BTC_THRESHOLD_MIN     (-120)
#define CFG_MBO_CAND_RSSI_BTC_THRESHOLD_MAX     (0)

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
 * gReducedBeaconInterval - beacon interval reduced
 * @Min: 0
 * @Max: 100
 * @Default: 0
 *
 * This ini is used to reduce beacon interval before channel
 * switch (when val great than 0, or the feature is disabled).
 * It would reduce the downtime on the STA side which is
 * waiting for beacons from the AP to resume back transmission.
 * Switch back the beacon_interval to its original value after
 * channel switch based on the timeout.
 *
 * Related: none
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_REDUCED_BEACON_INTERVAL         "gReducedBeaconInterval"
#define CFG_REDUCED_BEACON_INTERVAL_MIN     (0)
#define CFG_REDUCED_BEACON_INTERVAL_MAX     (100)
#define CFG_REDUCED_BEACON_INTERVAL_DEFAULT (0)

/*
 * oce_enable_rssi_assoc_reject - Enable/disable rssi based assoc rejection
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable/disable rssi based assoc rejection. If this is
 * disabled then OCE ini oce_sta_enable will also be disabled and OCE IE will
 * not be sent in frames.
 *
 * Related: None
 *
 * Supported Feature: OCE
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_OCE_ENABLE_RSSI_BASED_ASSOC_REJECT_NAME \
	"oce_enable_rssi_assoc_reject"
#define CFG_OCE_ENABLE_RSSI_BASED_ASSOC_REJECT_MIN     (0)
#define CFG_OCE_ENABLE_RSSI_BASED_ASSOC_REJECT_MAX     (1)
#define CFG_OCE_ENABLE_RSSI_BASED_ASSOC_REJECT_DEFAULT (1)

/*
 * <ini>
 * oce_enable_probe_req_rate - Set probe request rate
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to set probe request rate to 5.5Mbps as per OCE requirement
 * in 2.4G band
 *
 * Related: None
 *
 * Supported Feature: OCE
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_OCE_PROBE_REQ_RATE_NAME    "oce_enable_probe_req_rate"
#define CFG_OCE_PROBE_REQ_RATE_MIN     (0)
#define CFG_OCE_PROBE_REQ_RATE_MAX     (1)
#define CFG_OCE_PROBE_REQ_RATE_DEFAULT (0)

/*
 * <ini>
 * oce_enable_probe_resp_rate - Set probe response rate
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to set probe response rate to 5.5Mbps as per OCE requirement
 * in 2.4G band
 *
 * Related: None
 *
 * Supported Feature: OCE
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_OCE_PROBE_RSP_RATE_NAME    "oce_enable_probe_resp_rate"
#define CFG_OCE_PROBE_RSP_RATE_MIN     (0)
#define CFG_OCE_PROBE_RSP_RATE_MAX     (1)
#define CFG_OCE_PROBE_RSP_RATE_DEFAULT (0)

/*
 * <ini>
 * oce_enable_beacon_rate - Set beacon rate
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to set beacon rate to 5.5Mbps as per OCE requirement in
 * 2.4G band
 *
 * Related: None
 *
 * Supported Feature: OCE
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_OCE_BEACON_RATE_NAME    "oce_enable_beacon_rate"
#define CFG_OCE_BEACON_RATE_MIN     (0)
#define CFG_OCE_BEACON_RATE_MAX     (1)
#define CFG_OCE_BEACON_RATE_DEFAULT (0)

/*
 * <ini>
 * oce_enable_probe_req_deferral - Enable/disable probe request deferral
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable/disable probe request deferral as per OCE spec
 *
 * Related: None
 *
 * Supported Feature: OCE
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ENABLE_PROBE_REQ_DEFERRAL_NAME    "oce_enable_probe_req_deferral"
#define CFG_ENABLE_PROBE_REQ_DEFERRAL_MIN     (0)
#define CFG_ENABLE_PROBE_REQ_DEFERRAL_MAX     (1)
#define CFG_ENABLE_PROBE_REQ_DEFERRAL_DEFAULT (0)

/*
 * <ini>
 * oce_enable_fils_discovery_sap - Enable/disable fils discovery in sap mode
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable/disable fils discovery in sap mode
 *
 * Related: None
 *
 * Supported Feature: FILS
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ENABLE_FILS_DISCOVERY_SAP_NAME    "oce_enable_fils_discovery_sap"
#define CFG_ENABLE_FILS_DISCOVERY_SAP_MIN     (0)
#define CFG_ENABLE_FILS_DISCOVERY_SAP_MAX     (1)
#define CFG_ENABLE_FILS_DISCOVERY_SAP_DEFAULT (0)

/*
 * <ini>
 * enable_esp_for_roam - Enable/disable esp feature
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable/disable ESP(Estimated service parameters) IE
 * parsing and decides whether firmware will include this in its scoring algo.
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ENABLE_ESP_FEATURE_NAME    "enable_esp_for_roam"
#define CFG_ENABLE_ESP_FEATURE_MIN     (0)
#define CFG_ENABLE_ESP_FEATURE_MAX     (1)
#define CFG_ENABLE_ESP_FEATURE_DEFAULT (0)

/*
 * <ini>
 * tx_chain_mask_2g - tx chain mask for 2g
 * @Min: 0
 * @Max: 3
 * @Default: 0
 *
 * This ini will set tx chain mask for 2g. To use the ini, make sure:
 * gSetTxChainmask1x1/gSetRxChainmask1x1 = 0,
 * gDualMacFeatureDisable = 1
 * gEnable2x2 = 0
 *
 * tx_chain_mask_2g=0 : don't care
 * tx_chain_mask_2g=1 : for 2g tx use chain 0
 * tx_chain_mask_2g=2 : for 2g tx use chain 1
 * tx_chain_mask_2g=3 : for 2g tx can use either chain
 *
 * Related: None
 *
 * Supported Feature: All profiles
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_TX_CHAIN_MASK_2G_NAME    "tx_chain_mask_2g"
#define CFG_TX_CHAIN_MASK_2G_MIN     (0)
#define CFG_TX_CHAIN_MASK_2G_MAX     (3)
#define CFG_TX_CHAIN_MASK_2G_DEFAULT (0)


/*
 * <ini>
 * tx_chain_mask_5g - tx chain mask for 5g
 * @Min: 0
 * @Max: 3
 * @Default: 0
 *
 * This ini will set tx chain mask for 5g. To use the ini, make sure:
 * gSetTxChainmask1x1/gSetRxChainmask1x1 = 0,
 * gDualMacFeatureDisable = 1
 * gEnable2x2 = 0
 *
 * tx_chain_mask_5g=0 : don't care
 * tx_chain_mask_5g=1 : for 5g tx use chain 0
 * tx_chain_mask_5g=2 : for 5g tx use chain 1
 * tx_chain_mask_5g=3 : for 5g tx can use either chain
 *
 * Related: None
 *
 * Supported Feature: All profiles
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_TX_CHAIN_MASK_5G_NAME    "tx_chain_mask_5g"
#define CFG_TX_CHAIN_MASK_5G_MIN     (0)
#define CFG_TX_CHAIN_MASK_5G_MAX     (3)
#define CFG_TX_CHAIN_MASK_5G_DEFAULT (0)


/*
 * <ini>
 * rx_chain_mask_2g - rx chain mask for 2g
 * @Min: 0
 * @Max: 3
 * @Default: 0
 *
 * This ini will set rx chain mask for 2g. To use the ini, make sure:
 * gSetTxChainmask1x1/gSetRxChainmask1x1 = 0,
 * gDualMacFeatureDisable = 1
 * gEnable2x2 = 0
 *
 * rx_chain_mask_2g=0 : don't care
 * rx_chain_mask_2g=1 : for 2g rx use chain 0
 * rx_chain_mask_2g=2 : for 2g rx use chain 1
 * rx_chain_mask_2g=3 : for 2g rx can use either chain
 *
 * Related: None
 *
 * Supported Feature: All profiles
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_RX_CHAIN_MASK_2G_NAME    "rx_chain_mask_2g"
#define CFG_RX_CHAIN_MASK_2G_MIN     (0)
#define CFG_RX_CHAIN_MASK_2G_MAX     (3)
#define CFG_RX_CHAIN_MASK_2G_DEFAULT (0)


/*
 * <ini>
 * rx_chain_mask_5g - rx chain mask for 5g
 * @Min: 0
 * @Max: 3
 * @Default: 0
 *
 * This ini will set rx chain mask for 5g. To use the ini, make sure:
 * gSetTxChainmask1x1/gSetRxChainmask1x1 = 0,
 * gDualMacFeatureDisable = 1
 * gEnable2x2 = 0
 *
 * rx_chain_mask_5g=0 : don't care
 * rx_chain_mask_5g=1 : for 5g rx use chain 0
 * rx_chain_mask_5g=2 : for 5g rx use chain 1
 * rx_chain_mask_5g=3 : for 5g rx can use either chain
 *
 * Related: None
 *
 * Supported Feature: All profiles
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_RX_CHAIN_MASK_5G_NAME    "rx_chain_mask_5g"
#define CFG_RX_CHAIN_MASK_5G_MIN     (0)
#define CFG_RX_CHAIN_MASK_5G_MAX     (3)
#define CFG_RX_CHAIN_MASK_5G_DEFAULT (0)

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
#define CFG_BTM_ENABLE_DEFAULT   (0x00000041)

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
#define CFG_ENABLE_GCMP_DEFAULT (0)

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
 * Type declarations
 */
struct hdd_config {
	/* Bitmap to track what is explicitly configured */
	DECLARE_BITMAP(bExplicitCfg, MAX_CFG_INI_ITEMS);

	/* Config parameters */
	bool enable_connected_scan;
	uint32_t RTSThreshold;
	uint32_t FragmentationThreshold;
	uint8_t OperatingChannel;
	bool ShortSlotTimeEnabled;
	bool Is11dSupportEnabled;
	bool Is11hSupportEnabled;
	bool fSupplicantCountryCodeHasPriority;
	uint32_t HeartbeatThresh24;
	char PowerUsageControl[4];
	bool fIsImpsEnabled;
	bool is_ps_enabled;
	uint32_t auto_bmps_timer_val;
	uint32_t icmp_disable_ps_val;
	uint32_t nBmpsModListenInterval;
	uint32_t nBmpsMaxListenInterval;
	uint32_t nBmpsMinListenInterval;
	enum hdd_dot11_mode dot11Mode;
	uint32_t nChannelBondingMode24GHz;
	bool override_ht20_40_24g;
	uint32_t nChannelBondingMode5GHz;
	uint32_t MaxRxAmpduFactor;
	uint16_t TxRate;
	uint32_t ShortGI20MhzEnable;
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
	uint32_t nAPAutoShutOff;
	uint8_t enableLTECoex;
	uint32_t apKeepAlivePeriod;
	uint32_t goKeepAlivePeriod;
	enum station_keepalive_method sta_keepalive_method;
	uint32_t apLinkMonitorPeriod;
	uint32_t goLinkMonitorPeriod;
	uint32_t nBeaconInterval;
	uint8_t nTxPowerCap;    /* In dBm */
	bool allow_tpc_from_ap;
	bool fIsLowGainOverride;
	uint8_t disablePacketFilter;
	bool fRrmEnable;
	uint16_t nRrmRandnIntvl;
	/* length includes separator */
	char rm_capability[3 * DOT11F_IE_RRMENABLEDCAP_MAX_LEN];

	/* Vowifi 11r params */
	bool fFTResourceReqSupported;

	/* Bitmap for operating voltage corner mode */
	uint32_t vc_mode_cfg_bitmap;

	uint16_t nNeighborScanPeriod;
	uint16_t neighbor_scan_min_period;
	uint8_t nNeighborLookupRssiThreshold;
	uint8_t delay_before_vdev_stop;
	uint8_t nOpportunisticThresholdDiff;
	uint8_t nRoamRescanRssiDiff;
	uint8_t neighborScanChanList[WNI_CFG_VALID_CHANNEL_LIST_LEN];
	uint16_t nNeighborScanMinChanTime;
	uint16_t nNeighborScanMaxChanTime;
	uint16_t nMaxNeighborReqTries;
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

	uint32_t nPassiveMinChnTime;    /* in units of milliseconds */
	uint32_t nPassiveMaxChnTime;    /* in units of milliseconds */
	uint32_t nActiveMinChnTime;     /* in units of milliseconds */
	uint32_t nActiveMaxChnTime;     /* in units of milliseconds */
	uint32_t scan_probe_repeat_time;
	uint32_t scan_num_probes;

	uint32_t nInitialDwellTime;     /* in units of milliseconds */
	bool initial_scan_no_dfs_chnl;

#ifdef WLAN_AP_STA_CONCURRENCY
	uint32_t nPassiveMinChnTimeConc;        /* in units of milliseconds */
	uint32_t nPassiveMaxChnTimeConc;        /* in units of milliseconds */
	uint32_t nActiveMinChnTimeConc; /* in units of milliseconds */
	uint32_t nActiveMaxChnTimeConc; /* in units of milliseconds */
	uint32_t nRestTimeConc; /* in units of milliseconds */
	/* In units of milliseconds */
	uint32_t       min_rest_time_conc;
	/* In units of milliseconds */
	uint32_t       idle_time_conc;
#endif

	uint8_t nMaxPsPoll;

	uint8_t nRssiFilterPeriod;
	bool fIgnoreDtim;
	uint8_t fMaxLIModulatedDTIM;

	uint8_t fEnableFwHeartBeatMonitoring;
	uint8_t fEnableFwBeaconFiltering;
	bool fEnableFwRssiMonitoring;
	bool mcc_rts_cts_prot_enable;
	bool mcc_bcast_prob_resp_enable;
	uint8_t nDataInactivityTimeout;
	uint8_t wow_data_inactivity_timeout;

	/* WMM QoS Configuration */
	enum hdd_wmm_user_mode WmmMode;
	bool b80211eIsEnabled;
	uint8_t UapsdMask;      /* what ACs to setup U-APSD for at assoc */
	uint32_t InfraUapsdVoSrvIntv;
	uint32_t InfraUapsdVoSuspIntv;
	uint32_t InfraUapsdViSrvIntv;
	uint32_t InfraUapsdViSuspIntv;
	uint32_t InfraUapsdBeSrvIntv;
	uint32_t InfraUapsdBeSuspIntv;
	uint32_t InfraUapsdBkSrvIntv;
	uint32_t InfraUapsdBkSuspIntv;
	bool isFastRoamIniFeatureEnabled;
	bool MAWCEnabled;
	bool mawc_roam_enabled;
	uint32_t mawc_roam_traffic_threshold;
	int8_t mawc_roam_ap_rssi_threshold;
	uint8_t mawc_roam_rssi_high_adjust;
	uint8_t mawc_roam_rssi_low_adjust;
#ifdef FEATURE_WLAN_ESE
	uint32_t InfraInactivityInterval;
	bool isEseIniFeatureEnabled;
#endif
	bool isFastTransitionEnabled;
	uint8_t RoamRssiDiff;
	int32_t rssi_abs_thresh;
	bool isWESModeEnabled;
	uint32_t pmkid_modes;
	bool isRoamOffloadScanEnabled;
	bool bImplicitQosEnabled;

	/* default TSPEC parameters for AC_VO */
	enum sme_qos_wmm_dir_type InfraDirAcVo;
	uint16_t InfraNomMsduSizeAcVo;
	uint32_t InfraMeanDataRateAcVo;
	uint32_t InfraMinPhyRateAcVo;
	uint16_t InfraSbaAcVo;

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

	/* Wowl pattern */
	char wowlPattern[1024];

	/* Control for Replay counetr. value 1 means
	 * single replay counter for all TID
	 */
	bool bSingleTidRc;
	bool fhostArpOffload;
	enum pmo_hw_filter_mode hw_filter_mode;
	bool ssdp;

#ifdef FEATURE_RUNTIME_PM
	bool runtime_pm;
	uint32_t runtime_pm_delay;
#endif

#ifdef FEATURE_WLAN_RA_FILTERING
	bool IsRArateLimitEnabled;
	uint16_t RArateLimitInterval;
#endif
#ifdef FEATURE_WLAN_SCAN_PNO
	bool PnoOffload;
#endif
	bool fhostNSOffload;
	bool burstSizeDefinition;
	uint8_t tsInfoAckPolicy;

	/* RF Settling Time Clock */
	uint32_t rfSettlingTimeUs;

	uint8_t dynamicPsPollValue;
	bool AddTSWhenACMIsOff;
	bool fValidateScanList;

	uint32_t infraStaKeepAlivePeriod;
	uint8_t nNullDataApRespTimeout;
	uint8_t nBandCapability;

	uint32_t apDataAvailPollPeriodInMs;
	bool teleBcnWakeupEn;

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

	uint16_t nTeleBcnTransListenInterval;
	uint16_t nTeleBcnMaxListenInterval;
	uint16_t nTeleBcnTransLiNumIdleBeacons;
	uint16_t nTeleBcnMaxLiNumIdleBeacons;
	uint8_t enableBypass11d;
	uint8_t enableDFSChnlScan;
	uint8_t enable_dfs_pno_chnl_scan;
	uint8_t enableDynamicDTIM;
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
	bool isP2pDeviceAddrAdministrated;
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
	uint8_t txBFCsnValue;
	bool enable_su_tx_bformer;
	uint8_t vhtRxMCS2x2;
	uint8_t vhtTxMCS2x2;
	uint8_t disable_high_ht_mcs_2x2;
	bool enable2x2;
	uint32_t vdev_type_nss_2g;
	uint32_t vdev_type_nss_5g;
	uint8_t txchainmask1x1;
	uint8_t rxchainmask1x1;
	bool enableMuBformee;
	bool enableVhtpAid;
	bool enableVhtGid;
	bool enableTxBFin20MHz;
	uint8_t enableAmpduPs;
	uint8_t enableHtSmps;
	uint8_t htSmps;
	uint8_t enableModulatedDTIM;
	uint32_t fEnableMCAddrList;
	bool enableFirstScan2GOnly;
	bool skipDfsChnlInP2pSearch;
	bool ignoreDynamicDtimInP2pMode;
	bool enableRxSTBC;
	bool enableTxSTBC;
	uint8_t enable_tx_ldpc;
	uint8_t enable_rx_ldpc;
	bool enable5gEBT;
	bool prevent_link_down;
#ifdef FEATURE_WLAN_TDLS
	bool fEnableTDLSSupport;
	bool fEnableTDLSImplicitTrigger;
	uint32_t fTDLSTxStatsPeriod;
	uint32_t fTDLSTxPacketThreshold;
	uint32_t fTDLSMaxDiscoveryAttempt;
	uint32_t tdls_idle_timeout;
	uint32_t fTDLSIdlePacketThreshold;
	int32_t fTDLSRSSITriggerThreshold;
	int32_t fTDLSRSSITeardownThreshold;
	int32_t fTDLSRSSIDelta;
	uint32_t fTDLSUapsdMask;        /* what ACs to setup U-APSD for TDLS */
	uint32_t fEnableTDLSBufferSta;
	uint32_t fEnableTDLSSleepSta;
	uint32_t fTDLSPuapsdInactivityTimer;
	uint32_t fTDLSRxFrameThreshold;
	uint32_t fTDLSPuapsdPTIWindow;
	uint32_t fTDLSPuapsdPTRTimeout;
	bool fTDLSExternalControl;
	uint32_t fEnableTDLSOffChannel;
	uint32_t fEnableTDLSWmmMode;
	uint8_t fTDLSPrefOffChanNum;
	uint8_t fTDLSPrefOffChanBandwidth;
	uint8_t enable_tdls_scan;
	uint32_t tdls_peer_kickout_threshold;
#endif
#ifdef WLAN_SOFTAP_VSTA_FEATURE
	bool fEnableVSTASupport;
#endif
	uint32_t enableLpwrImgTransition;
	uint8_t scanAgingTimeout;
	uint8_t disableLDPCWithTxbfAP;
	uint8_t enableMCCAdaptiveScheduler;
	bool sapAllowAllChannel;
	uint8_t retryLimitZero;
	uint8_t retryLimitOne;
	uint8_t retryLimitTwo;
	bool enableSSR;
	bool enable_data_stall_det;
	uint32_t cfgMaxMediumTime;
	bool enableVhtFor24GHzBand;
	bool enable_sap_vendor_vht;
	bool bFastRoamInConIniFeatureEnabled;
	bool fEnableAdaptRxDrain;
	bool enableIbssHeartBeatOffload;
	uint32_t antennaDiversity;
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
	uint8_t enablePowersaveOffload;
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
	uint8_t maxWoWFilters;
	uint8_t wowEnable;
	uint8_t maxNumberOfPeers;
	uint8_t disableDFSChSwitch;
	uint8_t enableDFSMasterCap;
	uint16_t thermalTempMinLevel0;
	uint16_t thermalTempMaxLevel0;
	uint16_t thermalTempMinLevel1;
	uint16_t thermalTempMaxLevel1;
	uint16_t thermalTempMinLevel2;
	uint16_t thermalTempMaxLevel2;
	uint16_t thermalTempMinLevel3;
	uint16_t thermalTempMaxLevel3;
	uint32_t TxPower2g;
	uint32_t TxPower5g;
	uint32_t gEnableDebugLog;
	bool fDfsPhyerrFilterOffload;
	uint8_t gSapPreferredChanLocation;
	uint8_t gDisableDfsJapanW53;
	bool gEnableOverLapCh;
	bool fRegChangeDefCountry;
	bool acs_with_more_param;
	uint32_t auto_channel_select_weight;
	uint16_t max_ht_mcs_txdata;
	bool sap_get_peer_info;
	bool disable_abg_rate_txdata;
	uint8_t rate_for_tx_mgmt;
	uint8_t rate_for_tx_mgmt_2g;
	uint8_t rate_for_tx_mgmt_5g;
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
	uint8_t apMaxOffloadPeers;
	uint8_t apMaxOffloadReorderBuffs;
	bool advertiseConcurrentOperation;
	bool enableMemDeepSleep;
	bool enable_cck_tx_fir_override;

	uint32_t defaultRateIndex24Ghz;
#ifdef MEMORY_DEBUG
	bool IsMemoryDebugSupportEnabled;
#endif

	uint8_t allowDFSChannelRoam;

	bool debugP2pRemainOnChannel;

	bool enablePacketLog;
#ifdef MSM_PLATFORM
	uint32_t busBandwidthHighThreshold;
	uint32_t busBandwidthMediumThreshold;
	uint32_t busBandwidthLowThreshold;
	uint32_t busBandwidthComputeInterval;
	uint32_t enable_tcp_delack;
	uint32_t enable_tcp_adv_win_scale;
	uint32_t tcpDelackThresholdHigh;
	uint32_t tcpDelackThresholdLow;
	uint32_t tcp_tx_high_tput_thres;
	uint32_t tcp_delack_timer_count;
	u8  periodic_stats_disp_time;
#endif /* MSM_PLATFORM */

	/* FW debug log parameters */
	uint32_t enableFwLogType;
	uint32_t enableFwLogLevel;
	uint8_t enableFwModuleLogLevel[FW_MODULE_LOG_LEVEL_STRING_LENGTH];

	/* RTS profile parameter */
	uint32_t rts_profile;

#ifdef WLAN_FEATURE_11W
	uint32_t pmfSaQueryMaxRetries;
	uint32_t pmfSaQueryRetryInterval;
#endif

	uint8_t gMaxConcurrentActiveSessions;

	uint8_t ignoreCAC;
	bool IsSapDfsChSifsBurstEnabled;

#ifdef WLAN_SUPPORT_GREEN_AP
	bool enableGreenAP;
	bool enable_egap;
	uint32_t egap_feature_flag;
	uint32_t egap_inact_time;
	uint32_t egap_wait_time;
#endif
	/* Flag to indicate crash inject enabled or not */
	bool crash_inject_enabled;
	uint8_t force_sap_acs;
	uint8_t force_sap_acs_st_ch;
	uint8_t force_sap_acs_end_ch;

	bool enable_sap_mandatory_chan_list;

	int32_t dfsRadarPriMultiplier;
	uint8_t reorderOffloadSupport;

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	bool isRoamOffloadEnabled;
#endif

	uint32_t IpaUcTxBufCount;
	uint32_t IpaUcTxBufSize;
	uint32_t IpaUcRxIndRingCount;
	uint32_t IpaUcTxPartitionBase;
#ifdef WLAN_LOGGING_SOCK_SVC_ENABLE
	/* WLAN Logging */
	bool wlanLoggingEnable;
	bool wlanLoggingToConsole;
#endif /* WLAN_LOGGING_SOCK_SVC_ENABLE */

	uint8_t enableSifsBurst;

#ifdef WLAN_FEATURE_LPSS
	bool enable_lpass_support;
#endif
#ifdef WLAN_FEATURE_NAN
	bool enable_nan_support;
#endif
	bool enableSelfRecovery;
#ifdef FEATURE_WLAN_FORCE_SAP_SCC
	uint8_t SapSccChanAvoidance;
#endif /* FEATURE_WLAN_FORCE_SAP_SCC */

	bool enable_sap_suspend;

#ifdef WLAN_FEATURE_EXTWOW_SUPPORT
	uint8_t extWowGotoSuspend;
	uint8_t extWowApp1WakeupPinNumber;
	uint8_t extWowApp2WakeupPinNumber;
	uint32_t extWowApp2KAInitPingInterval;
	uint32_t extWowApp2KAMinPingInterval;
	uint32_t extWowApp2KAMaxPingInterval;
	uint32_t extWowApp2KAIncPingInterval;
	uint16_t extWowApp2TcpSrcPort;
	uint16_t extWowApp2TcpDstPort;
	uint32_t extWowApp2TcpTxTimeout;
	uint32_t extWowApp2TcpRxTimeout;
#endif
	bool gEnableDeauthToDisassocMap;
#ifdef DHCP_SERVER_OFFLOAD
	bool enableDHCPServerOffload;
	uint32_t dhcpMaxNumClients;
	uint8_t dhcpServerIP[IPADDR_STRING_LENGTH];
#endif /* DHCP_SERVER_OFFLOAD */
	bool enable_mac_spoofing;
	uint8_t conc_custom_rule1;
	uint8_t conc_custom_rule2;
	uint8_t is_sta_connection_in_5gz_enabled;
	uint16_t p2p_listen_defer_interval;
	uint32_t sta_miracast_mcc_rest_time_val;
	bool is_ramdump_enabled;
#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
	bool sap_channel_avoidance;
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */
	uint8_t sap_p2p_11ac_override;
	uint8_t sap_dot11mc;
	uint8_t prefer_non_dfs_on_radar;
	bool ignore_peer_erp_info;
	uint8_t multicast_host_fw_msgs;
	uint8_t conc_system_pref;
	bool send_deauth_before_con;
	bool tso_enable;
	bool lro_enable;
	bool flow_steering_enable;
	uint8_t max_msdus_per_rxinorderind;
	bool active_mode_offload;
	bool bpf_packet_filter_enable;
	/* parameter for defer timer for enabling TDLS on p2p listen */
	uint16_t tdls_enable_defer_time;
	uint32_t fine_time_meas_cap;
	uint8_t max_scan_count;
#ifdef WLAN_FEATURE_FASTPATH
	bool fastpath_enable;
#endif
	uint8_t dot11p_mode;
	uint8_t rx_mode;
	uint8_t cpu_map_list[CFG_RPS_RX_QUEUE_CPU_MAP_LIST_LEN];
#ifdef FEATURE_WLAN_EXTSCAN
	bool     extscan_enabled;
	uint32_t extscan_passive_max_chn_time;
	uint32_t extscan_passive_min_chn_time;
	uint32_t extscan_active_max_chn_time;
	uint32_t extscan_active_min_chn_time;
#endif
	bool ce_classify_enabled;
	uint32_t dual_mac_feature_disable;
	uint8_t dbs_scan_selection[CFG_DBS_SCAN_PARAM_LENGTH];
	uint32_t sta_sap_scc_on_dfs_chan;
	bool     tx_chain_mask_cck;
	uint8_t  tx_chain_mask_1ss;
	bool smart_chainmask_enabled;
	bool alternative_chainmask_enabled;
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
	uint16_t obss_active_dwelltime;
	uint16_t obss_passive_dwelltime;
	uint16_t obss_width_trigger_interval;
	uint8_t inform_bss_rssi_raw;
#ifdef WLAN_FEATURE_TSF
	uint32_t tsf_gpio_pin;

#ifdef WLAN_FEATURE_TSF_PLUS
	uint8_t tsf_ptp_options;
#endif /* WLAN_FEATURE_TSF_PLUS */
#endif
	uint32_t roam_dense_traffic_thresh;
	uint32_t roam_dense_rssi_thresh_offset;
	bool ignore_peer_ht_opmode;
	uint32_t roam_dense_min_aps;
	int8_t roam_bg_scan_bad_rssi_thresh;
	uint8_t roam_bad_rssi_thresh_offset_2g;
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
	bool bpf_enabled;
	bool enable_dp_trace;
	uint8_t dp_trace_config[DP_TRACE_CONFIG_STRING_LENGTH];
	bool adaptive_dwell_mode_enabled;
	enum wmi_dwelltime_adaptive_mode scan_adaptive_dwell_mode;
	enum wmi_dwelltime_adaptive_mode scan_adaptive_dwell_mode_nc;
	enum wmi_dwelltime_adaptive_mode roamscan_adaptive_dwell_mode;
	enum wmi_dwelltime_adaptive_mode extscan_adaptive_dwell_mode;
	enum wmi_dwelltime_adaptive_mode pnoscan_adaptive_dwell_mode;
	enum wmi_dwelltime_adaptive_mode global_adapt_dwelltime_mode;
	uint8_t adapt_dwell_lpf_weight;
	uint8_t adapt_dwell_passive_mon_intval;
	uint8_t adapt_dwell_wifi_act_threshold;
	bool vendor_acs_support;
	bool acs_support_for_dfs_ltecoex;
	bool bug_report_for_no_scan_results;
	bool bug_on_reinit_failure;
#ifdef WLAN_FEATURE_NAN_DATAPATH
	bool enable_nan_datapath;
	uint8_t nan_datapath_ndi_channel;
#endif
	uint32_t iface_change_wait_time;
	/* parameter to control GTX */
	uint32_t tgt_gtx_usr_cfg;
	enum cfg_sub_20_channel_width enable_sub_20_channel_width;
	bool indoor_channel_support;
	/* parameter to force sap into 11n */
	bool sap_force_11n_for_11ac;
	bool go_force_11n_for_11ac;
	uint16_t sap_tx_leakage_threshold;
	bool multicast_replay_filter;
	/* parameter for indicating sifs burst duration to fw */
	uint8_t sifs_burst_duration;
	bool goptimize_chan_avoid_event;
	bool enable_go_cts2self_for_sta;
	uint32_t tx_aggregation_size;
	uint32_t rx_aggregation_size;
	bool sta_prefer_80MHz_over_160MHz;
	uint8_t sap_max_inactivity_override;
	bool fw_timeout_crash;
	/* beacon count before channel switch */
	uint8_t sap_chanswitch_beacon_cnt;
	uint8_t sap_chanswitch_mode;
	uint32_t rx_wakelock_timeout;
	uint32_t max_sched_scan_plan_interval;
	uint32_t max_sched_scan_plan_iterations;
#ifdef WLAN_FEATURE_WOW_PULSE
	bool wow_pulse_support;
	uint8_t wow_pulse_pin;
	uint16_t wow_pulse_interval_high;
	uint16_t wow_pulse_interval_low;
#endif
	enum hdd_wext_control standard_wext_control;
	enum hdd_wext_control private_wext_control;
	bool sap_internal_restart;
	enum restart_beaconing_on_ch_avoid_rule
		restart_beaconing_on_chan_avoid_event;
	uint8_t is_per_roam_enabled;
	uint32_t per_roam_high_rate_threshold;
	uint32_t per_roam_low_rate_threshold;
	uint32_t per_roam_th_percent;
	uint32_t per_roam_rest_time;
	uint32_t per_roam_mon_time;
	uint32_t min_candidate_rssi;
	enum active_bpf_mode active_uc_bpf_mode;
	enum active_bpf_mode active_mc_bc_bpf_mode;
	bool enable_bcast_probe_rsp;
#ifdef WLAN_FEATURE_11AX
	bool enable_ul_mimo;
	bool enable_ul_ofdma;
	uint32_t he_sta_obsspd;
	uint8_t he_dynamic_frag_support;
#endif
	enum l1ss_sleep_allowed l1ss_sleep_allowed;
	uint32_t arp_ac_category;
	bool ani_enabled;
	bool qcn_ie_support;
	bool tx_orphan_enable;

	bool probe_req_ie_whitelist;
	/* probe request bit map ies */
	uint32_t probe_req_ie_bitmap_0;
	uint32_t probe_req_ie_bitmap_1;
	uint32_t probe_req_ie_bitmap_2;
	uint32_t probe_req_ie_bitmap_3;
	uint32_t probe_req_ie_bitmap_4;
	uint32_t probe_req_ie_bitmap_5;
	uint32_t probe_req_ie_bitmap_6;
	uint32_t probe_req_ie_bitmap_7;
	/* Probe Request multiple vendor OUIs */
	uint8_t probe_req_ouis[MAX_PRB_REQ_VENDOR_OUI_INI_LEN];
	uint32_t no_of_probe_req_ouis;
	uint32_t probe_req_voui[MAX_PROBE_REQ_OUIS];

	uint32_t timer_multiplier;
	uint8_t fils_max_chan_guard_time;
	uint8_t scan_backoff_multiplier;
	bool mawc_nlo_enabled;
	uint32_t mawc_nlo_exp_backoff_ratio;
	uint32_t mawc_nlo_init_scan_interval;
	uint32_t mawc_nlo_max_scan_interval;
	enum hdd_external_acs_policy external_acs_policy;
	enum hdd_external_acs_freq_band external_acs_freq_band;
	/* threshold of packet drops at which FW initiates disconnect */
	uint16_t pkt_err_disconn_th;
	bool is_force_1x1;
	uint8_t enable_rts_sifsbursting;
	uint8_t max_mpdus_inampdu;
	uint16_t sap_max_mcs_txdata;
	enum pmo_auto_pwr_detect_failure_mode auto_pwr_save_fail_mode;
	uint16_t num_11b_tx_chains;
	uint16_t num_11ag_tx_chains;
	uint8_t ito_repeat_count;
	/* LCA(Last connected AP) disallow configs */
	uint32_t disallow_duration;
	uint32_t rssi_channel_penalization;
	uint32_t num_disallowed_aps;
	bool oce_sta_enabled;
	bool oce_sap_enabled;
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
	uint8_t upper_brssi_thresh;
	uint8_t lower_brssi_thresh;
	bool enable_dtim_1chrx;
	int8_t rssi_thresh_offset_5g;
	bool is_ndi_mac_randomized;
	uint32_t scan_11d_interval;
	bool chan_switch_hostapd_rate_enabled;
	bool is_bssid_hint_priority;
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
	uint32_t bandwidth_weight_per_index;
	uint32_t nss_weight_per_index;
	uint32_t band_weight_per_index;
	uint32_t best_rssi_threshold;
	uint32_t good_rssi_threshold;
	uint32_t bad_rssi_threshold;
	uint32_t good_rssi_pcnt;
	uint32_t bad_rssi_pcnt;
	uint32_t good_rssi_bucket_size;
	uint32_t bad_rssi_bucket_size;
	uint32_t rssi_pref_5g_rssi_thresh;
	uint8_t num_esp_qbss_slots;
	uint32_t esp_qbss_score_slots3_to_0;
	uint32_t esp_qbss_score_slots7_to_4;
	uint32_t esp_qbss_score_slots11_to_8;
	uint32_t esp_qbss_score_slots15_to_12;
	uint8_t num_oce_wan_slots;
	uint32_t oce_wan_score_slots3_to_0;
	uint32_t oce_wan_score_slots7_to_4;
	uint32_t oce_wan_score_slots11_to_8;
	uint32_t oce_wan_score_slots15_to_12;
	bool enable_scoring_for_roam;
	bool force_rsne_override;
	bool is_fils_enabled;
	uint16_t wlm_latency_enable;
	uint16_t wlm_latency_level;
	uint32_t wlm_latency_flags_normal;
	uint32_t wlm_latency_flags_moderate;
	uint32_t wlm_latency_flags_low;
	uint32_t wlm_latency_flags_ultralow;
	/* mbo related thresholds */
	int8_t mbo_candidate_rssi_thres;
	int8_t mbo_current_rssi_thres;
	int8_t mbo_current_rssi_mcc_thres;
	int8_t mbo_candidate_rssi_btc_thres;
	uint8_t packet_filters_bitmap;
	uint8_t enable_phy_reg_retention;
	uint8_t dfs_beacon_tx_enhanced;
	uint16_t reduced_beacon_interval;
	bool rssi_assoc_reject_enabled;
	bool oce_probe_req_rate_enabled;
	bool oce_probe_resp_rate_enabled;
	bool oce_beacon_rate_enabled;
	bool probe_req_deferral_enabled;
	bool fils_discovery_sap_enabled;
	bool esp_for_roam_enabled;
	uint8_t tx_chain_mask_2g;
	uint8_t rx_chain_mask_2g;
	uint8_t tx_chain_mask_5g;
	uint8_t rx_chain_mask_5g;
	uint32_t btm_offload_config;
#ifdef WLAN_FEATURE_SAE
	bool is_sae_enabled;
#endif
	uint32_t btm_solicited_timeout;
	uint32_t btm_max_attempt_cnt;
	uint32_t btm_sticky_time;
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
	uint16_t wmi_wq_watchdog_timeout;
	bool enable_dtim_selection_diversity;
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
		VAR_OFFSET(_Struct, _VarName),			\
		VAR_SIZE(_Struct, _VarName),			\
		(_Default),					\
		(_Min),						\
		(_Max),						\
		NULL,						\
		0						\
	}

#define REG_DYNAMIC_VARIABLE(_Name, _Type,  _Struct, _VarName,	\
			      _Flags, _Default, _Min, _Max,	\
			      _CBFunc, _CBParam)		\
	{							\
		(_Name),					\
		(_Type),					\
		(VAR_FLAGS_DYNAMIC_CFG | (_Flags)),		\
		VAR_OFFSET(_Struct, _VarName),			\
		VAR_SIZE(_Struct, _VarName),			\
		(_Default),					\
		(_Min),						\
		(_Max),						\
		(_CBFunc),					\
		(_CBParam)					\
	}

#define REG_VARIABLE_STRING(_Name, _Type,  _Struct, _VarName,	\
			     _Flags, _Default)			\
	{							\
		(_Name),					\
		(_Type),					\
		(_Flags),					\
		VAR_OFFSET(_Struct, _VarName),			\
		VAR_SIZE(_Struct, _VarName),			\
		(unsigned long)(_Default),			\
		0,						\
		0,						\
		NULL,						\
		0						\
	}

struct reg_table_entry {
	char *RegName;          /* variable name in the qcom_cfg.ini file */
	enum wlan_parameter_type RegType;    /* variable type in hdd_config struct */
	unsigned long Flags;    /* Specify optional parms and if RangeCheck is performed */
	unsigned short VarOffset;       /* offset to field from the base address of the structure */
	unsigned short VarSize; /* size (in bytes) of the field */
	unsigned long VarDefault;       /* default value to use */
	unsigned long VarMin;   /* minimum value, for range checking */
	unsigned long VarMax;   /* maximum value, for range checking */
	/* Dynamic modification notifier */
	void (*pfnDynamicnotify)(struct hdd_context *hdd_ctx,
				 unsigned long notifyId);
	unsigned long notifyId; /* Dynamic modification identifier */
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
QDF_STATUS hdd_cfg_get_global_config(struct hdd_context *hdd_ctx, char *pBuf,
				     int buflen);

eCsrPhyMode hdd_cfg_xlate_to_csr_phy_mode(enum hdd_dot11_mode dot11Mode);
QDF_STATUS hdd_execute_global_config_command(struct hdd_context *hdd_ctx,
					     char *command);

bool hdd_is_okc_mode_enabled(struct hdd_context *hdd_ctx);
QDF_STATUS hdd_set_idle_ps_config(struct hdd_context *hdd_ctx, bool val);
void hdd_get_pmkid_modes(struct hdd_context *hdd_ctx,
			 struct pmkid_mode_bits *pmkid_modes);

void hdd_update_tgt_cfg(void *context, void *param);

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

void hdd_cfg_print(struct hdd_context *hdd_ctx);

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
