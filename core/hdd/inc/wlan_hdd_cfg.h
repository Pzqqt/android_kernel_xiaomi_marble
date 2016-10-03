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

#define FW_MODULE_LOG_LEVEL_STRING_LENGTH  (255)

#define CFG_ENABLE_RX_THREAD		(1 << 0)
#define CFG_ENABLE_RPS			(1 << 1)
#define CFG_ENABLE_NAPI			(1 << 2)

#ifdef DHCP_SERVER_OFFLOAD
#define IPADDR_NUM_ENTRIES     (4)
#define IPADDR_STRING_LENGTH   (16)
#endif

/* Number of items that can be configured */
#define MAX_CFG_INI_ITEMS   1024

/* Defines for all of the things we read from the configuration (registry). */

#define CFG_RTS_THRESHOLD_NAME                 "RTSThreshold"
#define CFG_RTS_THRESHOLD_MIN                  WNI_CFG_RTS_THRESHOLD_STAMIN     /* min is 0, meaning always use RTS. */
#define CFG_RTS_THRESHOLD_MAX                  WNI_CFG_RTS_THRESHOLD_STAMAX     /* max is the max frame size */
#define CFG_RTS_THRESHOLD_DEFAULT              WNI_CFG_RTS_THRESHOLD_STADEF

#define CFG_FRAG_THRESHOLD_NAME                "gFragmentationThreshold"
#define CFG_FRAG_THRESHOLD_MIN                 WNI_CFG_FRAGMENTATION_THRESHOLD_STAMIN
#define CFG_FRAG_THRESHOLD_MAX                 WNI_CFG_FRAGMENTATION_THRESHOLD_STAMAX
#define CFG_FRAG_THRESHOLD_DEFAULT             WNI_CFG_FRAGMENTATION_THRESHOLD_STADEF

#define CFG_OPERATING_CHANNEL_NAME             "gOperatingChannel"
#define CFG_OPERATING_CHANNEL_MIN              (0)
#define CFG_OPERATING_CHANNEL_MAX              (14)
#define CFG_OPERATING_CHANNEL_DEFAULT          (1)

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

#define CFG_POWER_USAGE_NAME                   "gPowerUsage"
#define CFG_POWER_USAGE_MIN                    "Min"    /* Minimum Power Save */
#define CFG_POWER_USAGE_MAX                    "Max"    /* Maximum Power Save */
#define CFG_POWER_USAGE_DEFAULT                "Mod"    /* Moderate Power Save */

#define CFG_WOWL_PATTERN_NAME                  "gWowlPattern"
#define CFG_WOWL_PATTERN_DEFAULT               ""

/* IMPS = IdleModePowerSave */
#define CFG_ENABLE_IMPS_NAME                   "gEnableImps"
#define CFG_ENABLE_IMPS_MIN                    (0)
#define CFG_ENABLE_IMPS_MAX                    (1)
#define CFG_ENABLE_IMPS_DEFAULT                (1)

/* PS = PowerSave */
#define CFG_ENABLE_PS_NAME                     "gEnableBmps"
#define CFG_ENABLE_PS_MIN                      (0)
#define CFG_ENABLE_PS_MAX                      (1)
#define CFG_ENABLE_PS_DEFAULT                  (1)

#define CFG_BMPS_MINIMUM_LI_NAME               "gBmpsMinListenInterval"
#define CFG_BMPS_MINIMUM_LI_MIN                (1)
#define CFG_BMPS_MINIMUM_LI_MAX                (65535)
#define CFG_BMPS_MINIMUM_LI_DEFAULT            (1)

#define CFG_BMPS_MODERATE_LI_NAME              "gBmpsModListenInterval"
#define CFG_BMPS_MODERATE_LI_MIN               (1)
#define CFG_BMPS_MODERATE_LI_MAX               (65535)
#define CFG_BMPS_MODERATE_LI_DEFAULT           (1)

#define CFG_BMPS_MAXIMUM_LI_NAME               "gBmpsMaxListenInterval"
#define CFG_BMPS_MAXIMUM_LI_MIN                (1)
#define CFG_BMPS_MAXIMUM_LI_MAX                (65535)
#define CFG_BMPS_MAXIMUM_LI_DEFAULT            (1)

#define CFG_MAX_RX_AMPDU_FACTOR_NAME           "gMaxRxAmpduFactor"
#define CFG_MAX_RX_AMPDU_FACTOR_MIN            WNI_CFG_MAX_RX_AMPDU_FACTOR_STAMIN
#define CFG_MAX_RX_AMPDU_FACTOR_MAX            WNI_CFG_MAX_RX_AMPDU_FACTOR_STAMAX
#define CFG_MAX_RX_AMPDU_FACTOR_DEFAULT        WNI_CFG_MAX_RX_AMPDU_FACTOR_STADEF

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

/* Configuration added to enable/disable CTS2SELF in */
/* Adaptive RX drain feature */
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

typedef enum {
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
} eHddDot11Mode;

#define CFG_DOT11_MODE_NAME                    "gDot11Mode"
#define CFG_DOT11_MODE_MIN                     eHDD_DOT11_MODE_AUTO
#define CFG_DOT11_MODE_DEFAULT                 eHDD_DOT11_MODE_11ac
#define CFG_DOT11_MODE_MAX                     eHDD_DOT11_MODE_11a

#define CFG_CHANNEL_BONDING_MODE_24GHZ_NAME    "gChannelBondingMode24GHz"
#define CFG_CHANNEL_BONDING_MODE_MIN           WNI_CFG_CHANNEL_BONDING_MODE_STAMIN
#define CFG_CHANNEL_BONDING_MODE_MAX           WNI_CFG_CHANNEL_BONDING_MODE_STAMAX
#define CFG_CHANNEL_BONDING_MODE_DEFAULT       WNI_CFG_CHANNEL_BONDING_MODE_STADEF

#define CFG_CHANNEL_BONDING_MODE_5GHZ_NAME     "gChannelBondingMode5GHz"
#define CFG_CHANNEL_BONDING_MODE_MIN           WNI_CFG_CHANNEL_BONDING_MODE_STAMIN
#define CFG_CHANNEL_BONDING_MODE_MAX           WNI_CFG_CHANNEL_BONDING_MODE_STAMAX
#define CFG_CHANNEL_BONDING_MODE_DEFAULT       WNI_CFG_CHANNEL_BONDING_MODE_STADEF

#define CFG_FIXED_RATE_NAME                    "gFixedRate"
#define CFG_FIXED_RATE_MIN                     WNI_CFG_FIXED_RATE_STAMIN
#define CFG_FIXED_RATE_MAX                     WNI_CFG_FIXED_RATE_STAMAX
#define CFG_FIXED_RATE_DEFAULT                 WNI_CFG_FIXED_RATE_STADEF

#define CFG_SHORT_GI_20MHZ_NAME                "gShortGI20Mhz"
#define CFG_SHORT_GI_20MHZ_MIN                 WNI_CFG_SHORT_GI_20MHZ_STAMIN
#define CFG_SHORT_GI_20MHZ_MAX                 WNI_CFG_SHORT_GI_20MHZ_STAMAX
#define CFG_SHORT_GI_20MHZ_DEFAULT             WNI_CFG_SHORT_GI_20MHZ_STADEF

#define CFG_SCAN_RESULT_AGE_COUNT_NAME         "gScanResultAgeCount"
#define CFG_SCAN_RESULT_AGE_COUNT_MIN          (1)
#define CFG_SCAN_RESULT_AGE_COUNT_MAX          (100)
#define CFG_SCAN_RESULT_AGE_COUNT_DEFAULT      (1)

/* All in seconds */
/* Not Connect, No Power Save */
#define CFG_SCAN_RESULT_AGE_TIME_NCNPS_NAME    "gScanResultAgeNCNPS"
#define CFG_SCAN_RESULT_AGE_TIME_NCNPS_MIN     (10)
#define CFG_SCAN_RESULT_AGE_TIME_NCNPS_MAX     (10000)
#define CFG_SCAN_RESULT_AGE_TIME_NCNPS_DEFAULT (50)
/* Not Connect, Power Save */
#define CFG_SCAN_RESULT_AGE_TIME_NCPS_NAME     "gScanResultAgeNCPS"
#define CFG_SCAN_RESULT_AGE_TIME_NCPS_MIN      (10)
#define CFG_SCAN_RESULT_AGE_TIME_NCPS_MAX      (10000)
#define CFG_SCAN_RESULT_AGE_TIME_NCPS_DEFAULT  (300)
/* Connect, No Power Save */
#define CFG_SCAN_RESULT_AGE_TIME_CNPS_NAME     "gScanResultAgeCNPS"
#define CFG_SCAN_RESULT_AGE_TIME_CNPS_MIN      (10)
#define CFG_SCAN_RESULT_AGE_TIME_CNPS_MAX      (10000)
#define CFG_SCAN_RESULT_AGE_TIME_CNPS_DEFAULT  (150)
/* Connect, Power Save */
#define CFG_SCAN_RESULT_AGE_TIME_CPS_NAME      "gScanResultAgeCPS"
#define CFG_SCAN_RESULT_AGE_TIME_CPS_MIN       (10)
#define CFG_SCAN_RESULT_AGE_TIME_CPS_MAX       (10000)
#define CFG_SCAN_RESULT_AGE_TIME_CPS_DEFAULT   (600)

#define CFG_RSSI_CATEGORY_GAP_NAME             "gRssiCatGap"
#define CFG_RSSI_CATEGORY_GAP_MIN              (5)
#define CFG_RSSI_CATEGORY_GAP_MAX              (100)
#define CFG_RSSI_CATEGORY_GAP_DEFAULT          (5)

#define CFG_ROAM_PREFER_5GHZ                   "gRoamPrefer5GHz"
#define CFG_ROAM_PREFER_5GHZ_MIN              (0)
#define CFG_ROAM_PREFER_5GHZ_MAX              (1)
#define CFG_ROAM_PREFER_5GHZ_DEFAULT          (1)

/*
 * Timer waiting for interface up from the upper layer. If
 * this timer expires all the cds modules shall be closed.
 * Time Units: ms
 */
#define CFG_INTERFACE_CHANGE_WAIT_NAME    "gInterfaceChangeWait"
#define CFG_INTERFACE_CHANGE_WAIT_MIN     (10)
#define CFG_INTERFACE_CHANGE_WAIT_MAX     (10000)
#define CFG_INTERFACE_CHANGE_WAIT_DEFAULT (50)

/*
    To enable, set gRoamIntraBand=1 (Roaming within band)
    To disable, set gRoamIntraBand=0 (Roaming across band)
 */
#define CFG_ROAM_INTRA_BAND                   "gRoamIntraBand"
#define CFG_ROAM_INTRA_BAND_MIN               (0)
#define CFG_ROAM_INTRA_BAND_MAX               (1)
#define CFG_ROAM_INTRA_BAND_DEFAULT           (0)

#define CFG_SHORT_PREAMBLE_NAME                "gShortPreamble"
#define CFG_SHORT_PREAMBLE_MIN                 WNI_CFG_SHORT_PREAMBLE_STAMIN
#define CFG_SHORT_PREAMBLE_MAX                 WNI_CFG_SHORT_PREAMBLE_STAMAX
#define CFG_SHORT_PREAMBLE_DEFAULT             WNI_CFG_SHORT_PREAMBLE_STADEF

#define CFG_IBSS_BSSID_NAME                    "gIbssBssid"
#define CFG_IBSS_BSSID_MIN                     "000000000000"
#define CFG_IBSS_BSSID_MAX                     "ffffffffffff"
#define CFG_IBSS_BSSID_DEFAULT                 "000AF5040506"

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

#define CFG_AP_QOS_UAPSD_MODE_NAME             "gEnableApUapsd" /* ACs to setup U-APSD for at assoc */
#define CFG_AP_QOS_UAPSD_MODE_MIN              (0)
#define CFG_AP_QOS_UAPSD_MODE_MAX              (1)
#define CFG_AP_QOS_UAPSD_MODE_DEFAULT          (1)

#define CFG_AP_ENABLE_RANDOM_BSSID_NAME            "gEnableApRandomBssid"
#define CFG_AP_ENABLE_RANDOM_BSSID_MIN             (0)
#define CFG_AP_ENABLE_RANDOM_BSSID_MAX             (1)
#define CFG_AP_ENABLE_RANDOM_BSSID_DEFAULT         (0)

#define CFG_AP_ENABLE_PROTECTION_MODE_NAME            "gEnableApProt"
#define CFG_AP_ENABLE_PROTECTION_MODE_MIN             (0)
#define CFG_AP_ENABLE_PROTECTION_MODE_MAX             (1)
#define CFG_AP_ENABLE_PROTECTION_MODE_DEFAULT         (1)

/* Bit map for CFG_AP_PROTECTION_MODE_DEFAULT */
/* LOWER byte for associated stations */
/* UPPER byte for overlapping stations */
/* each byte will have the following info */
/* bit15 bit14 bit13     bit12  bit11 bit10    bit9     bit8 */
/* OBSS  RIFS  LSIG_TXOP NON_GF HT20  FROM_11G FROM_11B FROM_11A */
/* bit7  bit6  bit5      bit4   bit3  bit2     bit1     bit0 */
/* OBSS  RIFS  LSIG_TXOP NON_GF HT_20 FROM_11G FROM_11B FROM_11A */
#define CFG_AP_PROTECTION_MODE_NAME            "gApProtection"
#define CFG_AP_PROTECTION_MODE_MIN             (0x0)
#define CFG_AP_PROTECTION_MODE_MAX             (0xFFFF)
#define CFG_AP_PROTECTION_MODE_DEFAULT         (0xBFFF)

#define CFG_AP_OBSS_PROTECTION_MODE_NAME       "gEnableApOBSSProt"
#define CFG_AP_OBSS_PROTECTION_MODE_MIN        (0)
#define CFG_AP_OBSS_PROTECTION_MODE_MAX        (1)
#define CFG_AP_OBSS_PROTECTION_MODE_DEFAULT    (0)

#define CFG_AP_STA_SECURITY_SEPERATION_NAME    "gDisableIntraBssFwd"
#define CFG_AP_STA_SECURITY_SEPERATION_MIN     (0)
#define CFG_AP_STA_SECURITY_SEPERATION_MAX     (1)
#define CFG_AP_STA_SECURITY_SEPERATION_DEFAULT (0)

#define CFG_AP_AUTO_SHUT_OFF                "gAPAutoShutOff"
#define CFG_AP_AUTO_SHUT_OFF_MIN            (0)
#define CFG_AP_AUTO_SHUT_OFF_MAX            (4294967295UL)
#define CFG_AP_AUTO_SHUT_OFF_DEFAULT        (0)

#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
#define CFG_WLAN_AUTO_SHUTDOWN              "gWlanAutoShutdown"
#define CFG_WLAN_AUTO_SHUTDOWN_MIN          (0)
#define CFG_WLAN_AUTO_SHUTDOWN_MAX          (86400)   /* Max 1 day timeout */
#define CFG_WLAN_AUTO_SHUTDOWN_DEFAULT      (0)
#endif

#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
#define CFG_WLAN_MCC_TO_SCC_SWITCH_MODE          "gWlanMccToSccSwitchMode"
#define CFG_WLAN_MCC_TO_SCC_SWITCH_MODE_MIN      (QDF_MCC_TO_SCC_SWITCH_DISABLE)
#define CFG_WLAN_MCC_TO_SCC_SWITCH_MODE_MAX \
				   (QDF_MCC_TO_SCC_SWITCH_WITH_FAVORITE_CHANNEL)
#define CFG_WLAN_MCC_TO_SCC_SWITCH_MODE_DEFAULT  (QDF_MCC_TO_SCC_SWITCH_DISABLE)
#endif

#define CFG_DISABLE_PACKET_FILTER "gDisablePacketFilter"
#define CFG_DISABLE_PACKET_FILTER_MIN       (0)
#define CFG_DISABLE_PACKET_FILTER_MAX               (0x1)
#define CFG_DISABLE_PACKET_FILTER_DEFAULT           (0)

#define CFG_ENABLE_LTE_COEX              "gEnableLTECoex"
#define CFG_ENABLE_LTE_COEX_MIN               (0)
#define CFG_ENABLE_LTE_COEX_MAX               (1)
#define CFG_ENABLE_LTE_COEX_DEFAULT           (0)

#define CFG_AP_KEEP_ALIVE_PERIOD_NAME          "gApKeepAlivePeriod"
#define CFG_AP_KEEP_ALIVE_PERIOD_MIN           WNI_CFG_AP_KEEP_ALIVE_TIMEOUT_STAMIN
#define CFG_AP_KEEP_ALIVE_PERIOD_MAX           WNI_CFG_AP_KEEP_ALIVE_TIMEOUT_STAMAX
#define CFG_AP_KEEP_ALIVE_PERIOD_DEFAULT       WNI_CFG_AP_KEEP_ALIVE_TIMEOUT_STADEF

#define CFG_GO_KEEP_ALIVE_PERIOD_NAME          "gGoKeepAlivePeriod"
#define CFG_GO_KEEP_ALIVE_PERIOD_MIN           WNI_CFG_GO_KEEP_ALIVE_TIMEOUT_STAMIN
#define CFG_GO_KEEP_ALIVE_PERIOD_MAX           WNI_CFG_GO_KEEP_ALIVE_TIMEOUT_STAMAX
#define CFG_GO_KEEP_ALIVE_PERIOD_DEFAULT       WNI_CFG_GO_KEEP_ALIVE_TIMEOUT_STADEF

#define CFG_AP_LINK_MONITOR_PERIOD_NAME          "gApLinkMonitorPeriod"
#define CFG_AP_LINK_MONITOR_PERIOD_MIN           (3)
#define CFG_AP_LINK_MONITOR_PERIOD_MAX           (50)
#define CFG_AP_LINK_MONITOR_PERIOD_DEFAULT       (10)

/* gGoLinkMonitorPeriod is period where link is idle and where
 * we send NULL frame
 */
#define CFG_GO_LINK_MONITOR_PERIOD_NAME          "gGoLinkMonitorPeriod"
#define CFG_GO_LINK_MONITOR_PERIOD_MIN           (3)
#define CFG_GO_LINK_MONITOR_PERIOD_MAX           (50)
#define CFG_GO_LINK_MONITOR_PERIOD_DEFAULT       (10)

#define CFG_BEACON_INTERVAL_NAME               "gBeaconInterval"
#define CFG_BEACON_INTERVAL_MIN                WNI_CFG_BEACON_INTERVAL_STAMIN
#define CFG_BEACON_INTERVAL_MAX                WNI_CFG_BEACON_INTERVAL_STAMAX
#define CFG_BEACON_INTERVAL_DEFAULT            WNI_CFG_BEACON_INTERVAL_STADEF

/* Additional Handoff related Parameters */
#define CFG_ROAMING_TIME_NAME                 "gRoamingTime"
#define CFG_ROAMING_TIME_MIN                  (0)
#define CFG_ROAMING_TIME_MAX                  (4294967UL)
#define CFG_ROAMING_TIME_DEFAULT              (10)

#define CFG_VCC_RSSI_TRIGGER_NAME             "gVccRssiTrigger"
#define CFG_VCC_RSSI_TRIGGER_MIN              (0)
#define CFG_VCC_RSSI_TRIGGER_MAX              (80)
#define CFG_VCC_RSSI_TRIGGER_DEFAULT          (80)

#define CFG_VCC_UL_MAC_LOSS_THRESH_NAME       "gVccUlMacLossThresh"
#define CFG_VCC_UL_MAC_LOSS_THRESH_MIN        (0)
#define CFG_VCC_UL_MAC_LOSS_THRESH_MAX        (9)
#define CFG_VCC_UL_MAC_LOSS_THRESH_DEFAULT    (9)

#define CFG_PASSIVE_MAX_CHANNEL_TIME_NAME      "gPassiveMaxChannelTime"
#define CFG_PASSIVE_MAX_CHANNEL_TIME_MIN       (0)
#define CFG_PASSIVE_MAX_CHANNEL_TIME_MAX       (10000)
#define CFG_PASSIVE_MAX_CHANNEL_TIME_DEFAULT   (110)

#define CFG_PASSIVE_MIN_CHANNEL_TIME_NAME      "gPassiveMinChannelTime"
#define CFG_PASSIVE_MIN_CHANNEL_TIME_MIN       (0)
#define CFG_PASSIVE_MIN_CHANNEL_TIME_MAX       (10000)
#define CFG_PASSIVE_MIN_CHANNEL_TIME_DEFAULT   (60)

#define CFG_ACTIVE_MAX_CHANNEL_TIME_NAME       "gActiveMaxChannelTime"
#define CFG_ACTIVE_MAX_CHANNEL_TIME_MIN        (0)
#define CFG_ACTIVE_MAX_CHANNEL_TIME_MAX        (10000)
#define CFG_ACTIVE_MAX_CHANNEL_TIME_DEFAULT    (40)

#define CFG_ACTIVE_MIN_CHANNEL_TIME_NAME       "gActiveMinChannelTime"
#define CFG_ACTIVE_MIN_CHANNEL_TIME_MIN        (0)
#define CFG_ACTIVE_MIN_CHANNEL_TIME_MAX        (10000)
#define CFG_ACTIVE_MIN_CHANNEL_TIME_DEFAULT    (20)

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

#define CFG_PASSIVE_MAX_CHANNEL_TIME_CONC_NAME      "gPassiveMaxChannelTimeConc"
#define CFG_PASSIVE_MAX_CHANNEL_TIME_CONC_MIN       (0)
#define CFG_PASSIVE_MAX_CHANNEL_TIME_CONC_MAX       (10000)
#define CFG_PASSIVE_MAX_CHANNEL_TIME_CONC_DEFAULT   (110)

#define CFG_PASSIVE_MIN_CHANNEL_TIME_CONC_NAME      "gPassiveMinChannelTimeConc"
#define CFG_PASSIVE_MIN_CHANNEL_TIME_CONC_MIN       (0)
#define CFG_PASSIVE_MIN_CHANNEL_TIME_CONC_MAX       (10000)
#define CFG_PASSIVE_MIN_CHANNEL_TIME_CONC_DEFAULT   (60)

#define CFG_ACTIVE_MAX_CHANNEL_TIME_CONC_NAME       "gActiveMaxChannelTimeConc"
#define CFG_ACTIVE_MAX_CHANNEL_TIME_CONC_MIN        (0)
#define CFG_ACTIVE_MAX_CHANNEL_TIME_CONC_MAX        (10000)
#define CFG_ACTIVE_MAX_CHANNEL_TIME_CONC_DEFAULT    (40)

#define CFG_ACTIVE_MIN_CHANNEL_TIME_CONC_NAME       "gActiveMinChannelTimeConc"
#define CFG_ACTIVE_MIN_CHANNEL_TIME_CONC_MIN        (0)
#define CFG_ACTIVE_MIN_CHANNEL_TIME_CONC_MAX        (10000)
#define CFG_ACTIVE_MIN_CHANNEL_TIME_CONC_DEFAULT    (20)

#define CFG_REST_TIME_CONC_NAME                     "gRestTimeConc"
#define CFG_REST_TIME_CONC_MIN                      (0)
#define CFG_REST_TIME_CONC_MAX                      (10000)
#define CFG_REST_TIME_CONC_DEFAULT                  (100)

/* Mininum time spent on home channel before moving to a new channel to scan */
#define CFG_MIN_REST_TIME_NAME                      "gMinRestTimeConc"
#define CFG_MIN_REST_TIME_MIN                       (0)
#define CFG_MIN_REST_TIME_MAX                       (50)
#define CFG_MIN_REST_TIME_DEFAULT                   (50)

/* Data inactivity time in msec on bss channel that will be used
 * by scan engine in firmware.
 * for example if this value is 25ms then firmware will check for
 * data inactivity every 25ms till gRestTimeConc is reached.
 * If inactive then scan engine will move from home channel to
 * scan the next frequency.
 */
#define CFG_IDLE_TIME_NAME                          "gIdleTimeConc"
#define CFG_IDLE_TIME_MIN                           (0)
#define CFG_IDLE_TIME_MAX                           (25)
#define CFG_IDLE_TIME_DEFAULT                       (25)


#define CFG_NUM_STA_CHAN_COMBINED_CONC_NAME             "gNumStaChanCombinedConc"
#define CFG_NUM_STA_CHAN_COMBINED_CONC_MIN              (1)
#define CFG_NUM_STA_CHAN_COMBINED_CONC_MAX              (255)
#define CFG_NUM_STA_CHAN_COMBINED_CONC_DEFAULT          (3)

#define CFG_NUM_P2P_CHAN_COMBINED_CONC_NAME             "gNumP2PChanCombinedConc"
#define CFG_NUM_P2P_CHAN_COMBINED_CONC_MIN              (1)
#define CFG_NUM_P2P_CHAN_COMBINED_CONC_MAX              (255)
#define CFG_NUM_P2P_CHAN_COMBINED_CONC_DEFAULT          (1)
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

#define CFG_LOW_GAIN_OVERRIDE_NAME             "gLowGainOverride"
#define CFG_LOW_GAIN_OVERRIDE_MIN              WNI_CFG_LOW_GAIN_OVERRIDE_STAMIN
#define CFG_LOW_GAIN_OVERRIDE_MAX              WNI_CFG_LOW_GAIN_OVERRIDE_STAMAX
#define CFG_LOW_GAIN_OVERRIDE_DEFAULT          WNI_CFG_LOW_GAIN_OVERRIDE_STADEF

#define CFG_RSSI_FILTER_PERIOD_NAME            "gRssiFilterPeriod"
#define CFG_RSSI_FILTER_PERIOD_MIN             WNI_CFG_RSSI_FILTER_PERIOD_STAMIN
#define CFG_RSSI_FILTER_PERIOD_MAX             WNI_CFG_RSSI_FILTER_PERIOD_STAMAX
/* Increased this value for Non-ESE AP. This is cause FW RSSI Monitoring */
/* the consumer of this value is ON by default. So to impact power numbers */
/* we are setting this to a high value. */
#define CFG_RSSI_FILTER_PERIOD_DEFAULT         WNI_CFG_RSSI_FILTER_PERIOD_STADEF

#define CFG_IGNORE_DTIM_NAME                   "gIgnoreDtim"
#define CFG_IGNORE_DTIM_MIN                    WNI_CFG_IGNORE_DTIM_STAMIN
#define CFG_IGNORE_DTIM_MAX                    WNI_CFG_IGNORE_DTIM_STAMAX
#define CFG_IGNORE_DTIM_DEFAULT                WNI_CFG_IGNORE_DTIM_STADEF

#define CFG_MAX_LI_MODULATED_DTIM_NAME         "gMaxLIModulatedDTIM"
#define CFG_MAX_LI_MODULATED_DTIM_MIN          (1)
#define CFG_MAX_LI_MODULATED_DTIM_MAX          (10)
#define CFG_MAX_LI_MODULATED_DTIM_DEFAULT      (10)

#define CFG_FW_HEART_BEAT_MONITORING_NAME      "gEnableFWHeartBeatMonitoring"
#define CFG_FW_HEART_BEAT_MONITORING_MIN       (0)
#define CFG_FW_HEART_BEAT_MONITORING_MAX       (1)
#define CFG_FW_HEART_BEAT_MONITORING_DEFAULT   (1)

#define CFG_FW_BEACON_FILTERING_NAME           "gEnableFWBeaconFiltering"
#define CFG_FW_BEACON_FILTERING_MIN            (0)
#define CFG_FW_BEACON_FILTERING_MAX            (1)
#define CFG_FW_BEACON_FILTERING_DEFAULT        (1)

#define CFG_FW_RSSI_MONITORING_NAME            "gEnableFWRssiMonitoring"
#define CFG_FW_RSSI_MONITORING_MIN             (0)
#define CFG_FW_RSSI_MONITORING_MAX             (1)
#define CFG_FW_RSSI_MONITORING_DEFAULT         (1)

/* enable use of long duration RTS-CTS protection when SAP goes off channel
 * in MCC mode
 */
#define CFG_FW_MCC_RTS_CTS_PROT_NAME           "gFWMccRtsCtsProtection"
#define CFG_FW_MCC_RTS_CTS_PROT_MIN            (0)
#define CFG_FW_MCC_RTS_CTS_PROT_MAX            (1)
#define CFG_FW_MCC_RTS_CTS_PROT_DEFAULT        (0)

/* Enable use of broadcast probe response to increase the detectability of
 * SAP in MCC mode
 */
#define CFG_FW_MCC_BCAST_PROB_RESP_NAME        "gFWMccBCastProbeResponse"
#define CFG_FW_MCC_BCAST_PROB_RESP_MIN         (0)
#define CFG_FW_MCC_BCAST_PROB_RESP_MAX         (1)
#define CFG_FW_MCC_BCAST_PROB_RESP_DEFAULT     (0)

#define CFG_DATA_INACTIVITY_TIMEOUT_NAME       "gDataInactivityTimeout"
#define CFG_DATA_INACTIVITY_TIMEOUT_MIN        (1)
#define CFG_DATA_INACTIVITY_TIMEOUT_MAX        (255)
#define CFG_DATA_INACTIVITY_TIMEOUT_DEFAULT    (200)

#define CFG_RF_SETTLING_TIME_CLK_NAME          "rfSettlingTimeUs"
#define CFG_RF_SETTLING_TIME_CLK_MIN           (0)
#define CFG_RF_SETTLING_TIME_CLK_MAX           (60000)
#define CFG_RF_SETTLING_TIME_CLK_DEFAULT       (1500)

#define CFG_INFRA_STA_KEEP_ALIVE_PERIOD_NAME          "gStaKeepAlivePeriod"
#define CFG_INFRA_STA_KEEP_ALIVE_PERIOD_MIN           (0)
#define CFG_INFRA_STA_KEEP_ALIVE_PERIOD_MAX           (65535)
#define CFG_INFRA_STA_KEEP_ALIVE_PERIOD_DEFAULT       (0)

/* WMM configuration */
#define CFG_QOS_WMM_MODE_NAME                             "WmmIsEnabled"
#define CFG_QOS_WMM_MODE_MIN                               (0)
#define CFG_QOS_WMM_MODE_MAX                               (2)  /* HDD_WMM_NO_QOS */
#define CFG_QOS_WMM_MODE_DEFAULT                           (0)  /* HDD_WMM_AUTO */

#define CFG_QOS_WMM_80211E_ENABLED_NAME                   "80211eIsEnabled"
#define CFG_QOS_WMM_80211E_ENABLED_MIN                     (0)
#define CFG_QOS_WMM_80211E_ENABLED_MAX                     (1)
#define CFG_QOS_WMM_80211E_ENABLED_DEFAULT                 (0)

#define CFG_QOS_WMM_UAPSD_MASK_NAME                        "UapsdMask"  /* ACs to setup U-APSD for at assoc */
#define CFG_QOS_WMM_UAPSD_MASK_MIN                         (0x00)
#define CFG_QOS_WMM_UAPSD_MASK_MAX                         (0xFF)
#define CFG_QOS_WMM_UAPSD_MASK_DEFAULT                     (0x00)

#define CFG_QOS_WMM_INFRA_UAPSD_VO_SRV_INTV_NAME           "InfraUapsdVoSrvIntv"
#define CFG_QOS_WMM_INFRA_UAPSD_VO_SRV_INTV_MIN             (0)
#define CFG_QOS_WMM_INFRA_UAPSD_VO_SRV_INTV_MAX             (4294967295UL)
#define CFG_QOS_WMM_INFRA_UAPSD_VO_SRV_INTV_DEFAULT         (20)

#define CFG_QOS_WMM_INFRA_UAPSD_VO_SUS_INTV_NAME           "InfraUapsdVoSuspIntv"
#define CFG_QOS_WMM_INFRA_UAPSD_VO_SUS_INTV_MIN             (0)
#define CFG_QOS_WMM_INFRA_UAPSD_VO_SUS_INTV_MAX             (4294967295UL)
#define CFG_QOS_WMM_INFRA_UAPSD_VO_SUS_INTV_DEFAULT         (2000)

#define CFG_QOS_WMM_INFRA_UAPSD_VI_SRV_INTV_NAME           "InfraUapsdViSrvIntv"
#define CFG_QOS_WMM_INFRA_UAPSD_VI_SRV_INTV_MIN             (0)
#define CFG_QOS_WMM_INFRA_UAPSD_VI_SRV_INTV_MAX             (4294967295UL)
#define CFG_QOS_WMM_INFRA_UAPSD_VI_SRV_INTV_DEFAULT         (300)

#define CFG_QOS_WMM_INFRA_UAPSD_VI_SUS_INTV_NAME           "InfraUapsdViSuspIntv"
#define CFG_QOS_WMM_INFRA_UAPSD_VI_SUS_INTV_MIN             (0)
#define CFG_QOS_WMM_INFRA_UAPSD_VI_SUS_INTV_MAX             (4294967295UL)
#define CFG_QOS_WMM_INFRA_UAPSD_VI_SUS_INTV_DEFAULT         (2000)

#define CFG_QOS_WMM_INFRA_UAPSD_BE_SRV_INTV_NAME           "InfraUapsdBeSrvIntv"
#define CFG_QOS_WMM_INFRA_UAPSD_BE_SRV_INTV_MIN             (0)
#define CFG_QOS_WMM_INFRA_UAPSD_BE_SRV_INTV_MAX             (4294967295UL)
#define CFG_QOS_WMM_INFRA_UAPSD_BE_SRV_INTV_DEFAULT         (300)

#define CFG_QOS_WMM_INFRA_UAPSD_BE_SUS_INTV_NAME           "InfraUapsdBeSuspIntv"
#define CFG_QOS_WMM_INFRA_UAPSD_BE_SUS_INTV_MIN             (0)
#define CFG_QOS_WMM_INFRA_UAPSD_BE_SUS_INTV_MAX             (4294967295UL)
#define CFG_QOS_WMM_INFRA_UAPSD_BE_SUS_INTV_DEFAULT         (2000)

#define CFG_QOS_WMM_INFRA_UAPSD_BK_SRV_INTV_NAME           "InfraUapsdBkSrvIntv"
#define CFG_QOS_WMM_INFRA_UAPSD_BK_SRV_INTV_MIN             (0)
#define CFG_QOS_WMM_INFRA_UAPSD_BK_SRV_INTV_MAX             (4294967295UL)
#define CFG_QOS_WMM_INFRA_UAPSD_BK_SRV_INTV_DEFAULT         (300)

#define CFG_QOS_WMM_INFRA_UAPSD_BK_SUS_INTV_NAME           "InfraUapsdBkSuspIntv"
#define CFG_QOS_WMM_INFRA_UAPSD_BK_SUS_INTV_MIN             (0)
#define CFG_QOS_WMM_INFRA_UAPSD_BK_SUS_INTV_MAX             (4294967295UL)
#define CFG_QOS_WMM_INFRA_UAPSD_BK_SUS_INTV_DEFAULT         (2000)

#ifdef FEATURE_WLAN_ESE
#define CFG_QOS_WMM_INFRA_INACTIVITY_INTERVAL_NAME         "InfraInactivityInterval"
#define CFG_QOS_WMM_INFRA_INACTIVITY_INTERVAL_MIN           (0)
#define CFG_QOS_WMM_INFRA_INACTIVITY_INTERVAL_MAX           (4294967295UL)
#define CFG_QOS_WMM_INFRA_INACTIVITY_INTERVAL_DEFAULT       (0) /* disabled */

#define CFG_ESE_FEATURE_ENABLED_NAME                       "EseEnabled"
#define CFG_ESE_FEATURE_ENABLED_MIN                         (0)
#define CFG_ESE_FEATURE_ENABLED_MAX                         (1)
#define CFG_ESE_FEATURE_ENABLED_DEFAULT                     (0) /* disabled */
#endif /* FEATURE_WLAN_ESE */

#define CFG_LFR_FEATURE_ENABLED_NAME                       "FastRoamEnabled"
#define CFG_LFR_FEATURE_ENABLED_MIN                         (0)
#define CFG_LFR_FEATURE_ENABLED_MAX                         (1)
#define CFG_LFR_FEATURE_ENABLED_DEFAULT                     (0) /* disabled */

#define CFG_LFR_MAWC_FEATURE_ENABLED_NAME                   "MAWCEnabled"
#define CFG_LFR_MAWC_FEATURE_ENABLED_MIN                    (0)
#define CFG_LFR_MAWC_FEATURE_ENABLED_MAX                    (1)
#define CFG_LFR_MAWC_FEATURE_ENABLED_DEFAULT                (0) /* disabled */

/* This flag will control fasttransition in case of 11r and ese. */
/* Basically with this the whole neighbor roam, pre-auth, reassoc */
/* can be turned ON/OFF. */
/* With this turned OFF 11r will completely not work. */
/* For 11r this flag has to be ON. */
/* For ESE fastroam will not work. */
#define CFG_FAST_TRANSITION_ENABLED_NAME                    "FastTransitionEnabled"
#define CFG_FAST_TRANSITION_ENABLED_NAME_MIN                (0)
#define CFG_FAST_TRANSITION_ENABLED_NAME_MAX                (1)
#define CFG_FAST_TRANSITION_ENABLED_NAME_DEFAULT            (1) /* Enabled */

/* This parameter is used to decide whether to Roam or not.
 * AP1 is the currently associated AP and AP2 is chosen for roaming.
 * The Roaming will happen only if AP2 has better Signal Quality and it has a RSSI better than AP1
 * in terms of RoamRssiDiff,and RoamRssiDiff is the number of units (typically measured in dB) AP2
 * is better than AP1.
 * This check is not done if the value is Zero */
#define CFG_ROAM_RSSI_DIFF_NAME                             "RoamRssiDiff"
#define CFG_ROAM_RSSI_DIFF_MIN                              (0)
#define CFG_ROAM_RSSI_DIFF_MAX                              (30)
#define CFG_ROAM_RSSI_DIFF_DEFAULT                          (5)

/*This parameter is used to set Wireless Extended Security Mode.*/
#define CFG_ENABLE_WES_MODE_NAME                            "gWESModeEnabled"
#define CFG_ENABLE_WES_MODE_NAME_MIN                        (0)
#define CFG_ENABLE_WES_MODE_NAME_MAX                        (1)
#define CFG_ENABLE_WES_MODE_NAME_DEFAULT                    (0)

#define CFG_ROAM_SCAN_N_PROBES                             "gRoamScanNProbes"
#define CFG_ROAM_SCAN_N_PROBES_MIN                          (1)
#define CFG_ROAM_SCAN_N_PROBES_MAX                          (10)
#define CFG_ROAM_SCAN_N_PROBES_DEFAULT                      (2)

#define CFG_ROAM_SCAN_HOME_AWAY_TIME                        "gRoamScanHomeAwayTime"
#define CFG_ROAM_SCAN_HOME_AWAY_TIME_MIN                    (0) /* 0 for disable */
#define CFG_ROAM_SCAN_HOME_AWAY_TIME_MAX                    (300)
#define CFG_ROAM_SCAN_HOME_AWAY_TIME_DEFAULT                (CFG_ROAM_SCAN_HOME_AWAY_TIME_MIN)
/* disabled by default */

#define CFG_OKC_FEATURE_ENABLED_NAME                       "OkcEnabled"
#define CFG_OKC_FEATURE_ENABLED_MIN                        (0)
#define CFG_OKC_FEATURE_ENABLED_MAX                        (1)
#define CFG_OKC_FEATURE_ENABLED_DEFAULT                    (1)

#define CFG_ROAM_SCAN_OFFLOAD_ENABLED                       "gRoamScanOffloadEnabled"
#define CFG_ROAM_SCAN_OFFLOAD_ENABLED_MIN                   (0)
#define CFG_ROAM_SCAN_OFFLOAD_ENABLED_MAX                   (1)
#define CFG_ROAM_SCAN_OFFLOAD_ENABLED_DEFAULT               (1)

/* default TSPEC parameters for AC_VO */
#define CFG_QOS_WMM_INFRA_DIR_AC_VO_NAME                   "InfraDirAcVo"
#define CFG_QOS_WMM_INFRA_DIR_AC_VO_MIN                     (0)
#define CFG_QOS_WMM_INFRA_DIR_AC_VO_MAX                     (3)
#define CFG_QOS_WMM_INFRA_DIR_AC_VO_DEFAULT                 (3) /* WLAN_QCT_CUST_WMM_TSDIR_BOTH */

#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VO_NAME         "InfraNomMsduSizeAcVo"
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VO_MIN           (0x0)
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VO_MAX           (0xFFFF)
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VO_DEFAULT       (0x80D0)

#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VO_NAME        "InfraMeanDataRateAcVo"
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VO_MIN          (0x0)
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VO_MAX          (0xFFFFFFFF)
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VO_DEFAULT      (0x14500)

#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VO_NAME          "InfraMinPhyRateAcVo"
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VO_MIN            (0x0)
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VO_MAX            (0xFFFFFFFF)
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VO_DEFAULT        (0x5B8D80)

#define CFG_QOS_WMM_INFRA_SBA_AC_VO_NAME                   "InfraSbaAcVo"
#define CFG_QOS_WMM_INFRA_SBA_AC_VO_MIN                     (0x2001)
#define CFG_QOS_WMM_INFRA_SBA_AC_VO_MAX                     (0xFFFF)
#define CFG_QOS_WMM_INFRA_SBA_AC_VO_DEFAULT                 (0x2001)

/* default TSPEC parameters for AC_VI */
#define CFG_QOS_WMM_INFRA_DIR_AC_VI_NAME                   "InfraDirAcVi"
#define CFG_QOS_WMM_INFRA_DIR_AC_VI_MIN                     (0)
#define CFG_QOS_WMM_INFRA_DIR_AC_VI_MAX                     (3)
#define CFG_QOS_WMM_INFRA_DIR_AC_VI_DEFAULT                 (3) /* WLAN_QCT_CUST_WMM_TSDIR_BOTH */

#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VI_NAME         "InfraNomMsduSizeAcVi"
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VI_MIN           (0x0)
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VI_MAX           (0xFFFF)
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VI_DEFAULT       (0x85DC)

#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VI_NAME        "InfraMeanDataRateAcVi"
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VI_MIN          (0x0)
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VI_MAX          (0xFFFFFFFF)
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VI_DEFAULT      (0x57E40)

#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VI_NAME          "InfraMinPhyRateAcVi"
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VI_MIN            (0x0)
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VI_MAX            (0xFFFFFFFF)
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VI_DEFAULT        (0x5B8D80)

#define CFG_QOS_WMM_INFRA_SBA_AC_VI_NAME                   "InfraSbaAcVi"
#define CFG_QOS_WMM_INFRA_SBA_AC_VI_MIN                     (0x2001)
#define CFG_QOS_WMM_INFRA_SBA_AC_VI_MAX                     (0xFFFF)
#define CFG_QOS_WMM_INFRA_SBA_AC_VI_DEFAULT                 (0x2001)

/* default TSPEC parameters for AC_BE*/
#define CFG_QOS_WMM_INFRA_DIR_AC_BE_NAME                   "InfraDirAcBe"
#define CFG_QOS_WMM_INFRA_DIR_AC_BE_MIN                     (0)
#define CFG_QOS_WMM_INFRA_DIR_AC_BE_MAX                     (3)
#define CFG_QOS_WMM_INFRA_DIR_AC_BE_DEFAULT                 (3) /* WLAN_QCT_CUST_WMM_TSDIR_BOTH */

#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BE_NAME         "InfraNomMsduSizeAcBe"
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BE_MIN           (0x0)
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BE_MAX           (0xFFFF)
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BE_DEFAULT       (0x85DC)

#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BE_NAME        "InfraMeanDataRateAcBe"
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BE_MIN          (0x0)
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BE_MAX          (0xFFFFFFFF)
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BE_DEFAULT      (0x493E0)

#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BE_NAME          "InfraMinPhyRateAcBe"
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BE_MIN            (0x0)
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BE_MAX            (0xFFFFFFFF)
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BE_DEFAULT        (0x5B8D80)

#define CFG_QOS_WMM_INFRA_SBA_AC_BE_NAME                   "InfraSbaAcBe"
#define CFG_QOS_WMM_INFRA_SBA_AC_BE_MIN                     (0x2001)
#define CFG_QOS_WMM_INFRA_SBA_AC_BE_MAX                     (0xFFFF)
#define CFG_QOS_WMM_INFRA_SBA_AC_BE_DEFAULT                 (0x2001)

/* default TSPEC parameters for AC_Bk*/
#define CFG_QOS_WMM_INFRA_DIR_AC_BK_NAME                   "InfraDirAcBk"
#define CFG_QOS_WMM_INFRA_DIR_AC_BK_MIN                     (0)
#define CFG_QOS_WMM_INFRA_DIR_AC_BK_MAX                     (3)
#define CFG_QOS_WMM_INFRA_DIR_AC_BK_DEFAULT                 (3) /* WLAN_QCT_CUST_WMM_TSDIR_BOTH */

#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BK_NAME         "InfraNomMsduSizeAcBk"
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BK_MIN           (0x0)
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BK_MAX           (0xFFFF)
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BK_DEFAULT       (0x85DC)

#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BK_NAME        "InfraMeanDataRateAcBk"
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BK_MIN          (0x0)
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BK_MAX          (0xFFFFFFFF)
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BK_DEFAULT      (0x493E0)

#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BK_NAME          "InfraMinPhyRateAcBk"
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BK_MIN            (0x0)
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BK_MAX            (0xFFFFFFFF)
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BK_DEFAULT        (0x5B8D80)

#define CFG_QOS_WMM_INFRA_SBA_AC_BK_NAME                   "InfraSbaAcBk"
#define CFG_QOS_WMM_INFRA_SBA_AC_BK_MIN                     (0x2001)
#define CFG_QOS_WMM_INFRA_SBA_AC_BK_MAX                     (0xFFFF)
#define CFG_QOS_WMM_INFRA_SBA_AC_BK_DEFAULT                 (0x2001)

#define CFG_TL_DELAYED_TRGR_FRM_INT_NAME                   "DelayedTriggerFrmInt"
#define CFG_TL_DELAYED_TRGR_FRM_INT_MIN                     1
#define CFG_TL_DELAYED_TRGR_FRM_INT_MAX                     (4294967295UL)
#define CFG_TL_DELAYED_TRGR_FRM_INT_DEFAULT                 3000

#define CFG_RRM_ENABLE_NAME                              "gRrmEnable"
#define CFG_RRM_ENABLE_MIN                               (0)
#define CFG_RRM_ENABLE_MAX                               (1)
#define CFG_RRM_ENABLE_DEFAULT                           (0)

#define CFG_RRM_MEAS_RANDOMIZATION_INTVL_NAME            "gRrmRandnIntvl"
#define CFG_RRM_MEAS_RANDOMIZATION_INTVL_MIN             (10)
#define CFG_RRM_MEAS_RANDOMIZATION_INTVL_MAX             (100)
#define CFG_RRM_MEAS_RANDOMIZATION_INTVL_DEFAULT         (100)

/**
 * This INI is used to configure RM enabled capabilities IE.
 * Using this INI, we can set/unset any of the bits in 5 bytes
 * (last 4bytes are reserved). Bit details are updated as per
 * Draft version of 11mc spec. (Draft P802.11REVmc_D4.2)
 *
 * Bitwise details are defined as bit mask in rrm_global.h
 * Comma is used as a separator for each byte.
 */
#define CFG_RM_CAPABILITY_NAME            "rm_capability"
#define CFG_RM_CAPABILITY_DEFAULT         "73,10,91,00,04"

#define CFG_QOS_IMPLICIT_SETUP_ENABLED_NAME                 "ImplicitQosIsEnabled"
#define CFG_QOS_IMPLICIT_SETUP_ENABLED_MIN                  (0)
#define CFG_QOS_IMPLICIT_SETUP_ENABLED_MAX                  (1)
#define CFG_QOS_IMPLICIT_SETUP_ENABLED_DEFAULT              (0)

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

#define CFG_NEIGHBOR_SCAN_TIMER_PERIOD_NAME             "gNeighborScanTimerPeriod"
#define CFG_NEIGHBOR_SCAN_TIMER_PERIOD_MIN              (3)
#define CFG_NEIGHBOR_SCAN_TIMER_PERIOD_MAX              (300)
#define CFG_NEIGHBOR_SCAN_TIMER_PERIOD_DEFAULT          (200)

#define CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_NAME      "gNeighborLookupThreshold"
#define CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_MIN       (10)
#define CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_MAX       (120)
#define CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_DEFAULT   (78)

#define CFG_DELAY_BEFORE_VDEV_STOP_NAME              "gDelayBeforeVdevStop"
#define CFG_DELAY_BEFORE_VDEV_STOP_MIN               (2)
#define CFG_DELAY_BEFORE_VDEV_STOP_MAX               (200)
#define CFG_DELAY_BEFORE_VDEV_STOP_DEFAULT           (20)

/*
 * This parameter is the drop in RSSI value that will trigger a precautionary
 * scan by firmware.
 * MAX value is choose so that this type of scan can be disabled by user.
 */
#define CFG_ROAM_RESCAN_RSSI_DIFF_NAME                  "gRoamRescanRssiDiff"
#define CFG_ROAM_RESCAN_RSSI_DIFF_MIN                   (0)
#define CFG_ROAM_RESCAN_RSSI_DIFF_MAX                   (100)
#define CFG_ROAM_RESCAN_RSSI_DIFF_DEFAULT               (5)

/*
 * This parameter is the RSSI diff above neighbor lookup threshold, when
 * opportunistic scan should be triggered.
 * MAX value is choose so that this type of scan can be always enabled by user.
 * MIN value will cause opportunistic scan to be triggered in neighbor lookup
 * RSSI range.
 */
#define CFG_OPPORTUNISTIC_SCAN_THRESHOLD_DIFF_NAME            "gOpportunisticThresholdDiff"
#define CFG_OPPORTUNISTIC_SCAN_THRESHOLD_DIFF_MIN             (0)
#define CFG_OPPORTUNISTIC_SCAN_THRESHOLD_DIFF_MAX             (127)
#define CFG_OPPORTUNISTIC_SCAN_THRESHOLD_DIFF_DEFAULT         (0)

#define CFG_NEIGHBOR_SCAN_CHAN_LIST_NAME                      "gNeighborScanChannelList"
#define CFG_NEIGHBOR_SCAN_CHAN_LIST_DEFAULT                   ""

#define CFG_NEIGHBOR_SCAN_MIN_CHAN_TIME_NAME                  "gNeighborScanChannelMinTime"
#define CFG_NEIGHBOR_SCAN_MIN_CHAN_TIME_MIN                   (10)
#define CFG_NEIGHBOR_SCAN_MIN_CHAN_TIME_MAX                   (40)
#define CFG_NEIGHBOR_SCAN_MIN_CHAN_TIME_DEFAULT               (20)

#define CFG_NEIGHBOR_SCAN_MAX_CHAN_TIME_NAME                  "gNeighborScanChannelMaxTime"
#define CFG_NEIGHBOR_SCAN_MAX_CHAN_TIME_MIN                   (3)
#define CFG_NEIGHBOR_SCAN_MAX_CHAN_TIME_MAX                   (300)
#define CFG_NEIGHBOR_SCAN_MAX_CHAN_TIME_DEFAULT               (30)

#define CFG_11R_NEIGHBOR_REQ_MAX_TRIES_NAME           "gMaxNeighborReqTries"
#define CFG_11R_NEIGHBOR_REQ_MAX_TRIES_MIN            (1)
#define CFG_11R_NEIGHBOR_REQ_MAX_TRIES_MAX            (4)
#define CFG_11R_NEIGHBOR_REQ_MAX_TRIES_DEFAULT        (3)

#define CFG_NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_NAME         "gNeighborScanRefreshPeriod"
#define CFG_NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_MIN          (1000)
#define CFG_NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_MAX          (60000)
#define CFG_NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_DEFAULT      (20000)

#define CFG_EMPTY_SCAN_REFRESH_PERIOD_NAME         "gEmptyScanRefreshPeriod"
#define CFG_EMPTY_SCAN_REFRESH_PERIOD_MIN          (0)
#define CFG_EMPTY_SCAN_REFRESH_PERIOD_MAX          (60000)
#define CFG_EMPTY_SCAN_REFRESH_PERIOD_DEFAULT      (0)

#define CFG_ROAM_BMISS_FIRST_BCNT_NAME                  "gRoamBmissFirstBcnt"
#define CFG_ROAM_BMISS_FIRST_BCNT_MIN                   (5)
#define CFG_ROAM_BMISS_FIRST_BCNT_MAX                   (100)
#define CFG_ROAM_BMISS_FIRST_BCNT_DEFAULT               (10)

#define CFG_ROAM_BMISS_FINAL_BCNT_NAME                  "gRoamBmissFinalBcnt"
#define CFG_ROAM_BMISS_FINAL_BCNT_MIN                   (5)
#define CFG_ROAM_BMISS_FINAL_BCNT_MAX                   (100)
#define CFG_ROAM_BMISS_FINAL_BCNT_DEFAULT               (10)

#define CFG_ROAM_BEACON_RSSI_WEIGHT_NAME                "gRoamBeaconRssiWeight"
#define CFG_ROAM_BEACON_RSSI_WEIGHT_MIN                 (0)
#define CFG_ROAM_BEACON_RSSI_WEIGHT_MAX                 (16)
#define CFG_ROAM_BEACON_RSSI_WEIGHT_DEFAULT             (14)

#define CFG_QOS_WMM_BURST_SIZE_DEFN_NAME                        "burstSizeDefinition"
#define CFG_QOS_WMM_BURST_SIZE_DEFN_MIN                         (0)
#define CFG_QOS_WMM_BURST_SIZE_DEFN_MAX                         (1)
#define CFG_QOS_WMM_BURST_SIZE_DEFN_DEFAULT                     (0)

#define CFG_QOS_WMM_TS_INFO_ACK_POLICY_NAME                        "tsInfoAckPolicy"
#define CFG_QOS_WMM_TS_INFO_ACK_POLICY_MIN                         (0x00)
#define CFG_QOS_WMM_TS_INFO_ACK_POLICY_MAX                         (0x01)
#define CFG_QOS_WMM_TS_INFO_ACK_POLICY_DEFAULT                     (0x00)

#define CFG_SINGLE_TID_RC_NAME                             "SingleTIDRC"
#define CFG_SINGLE_TID_RC_MIN                               (0) /* Seperate replay counter for all TID */
#define CFG_SINGLE_TID_RC_MAX                               (1) /* Single replay counter for all TID */
#define CFG_SINGLE_TID_RC_DEFAULT                           (1)
#define CFG_MCAST_BCAST_FILTER_SETTING_NAME          "McastBcastFilter"
#define CFG_MCAST_BCAST_FILTER_SETTING_MIN           (0)
#define CFG_MCAST_BCAST_FILTER_SETTING_MAX           (3)
#define CFG_MCAST_BCAST_FILTER_SETTING_DEFAULT       (0)

#define CFG_DYNAMIC_PSPOLL_VALUE_NAME          "gDynamicPSPollvalue"
#define CFG_DYNAMIC_PSPOLL_VALUE_MIN           (0)
#define CFG_DYNAMIC_PSPOLL_VALUE_MAX           (255)
#define CFG_DYNAMIC_PSPOLL_VALUE_DEFAULT       (0)

#define CFG_TELE_BCN_WAKEUP_EN_NAME            "gTelescopicBeaconWakeupEn"
#define CFG_TELE_BCN_WAKEUP_EN_MIN             (0)
#define CFG_TELE_BCN_WAKEUP_EN_MAX             (1)
#define CFG_TELE_BCN_WAKEUP_EN_DEFAULT         (0)

#define CFG_QOS_ADDTS_WHEN_ACM_IS_OFF_NAME                 "gAddTSWhenACMIsOff"
#define CFG_QOS_ADDTS_WHEN_ACM_IS_OFF_MIN                  (0)
#define CFG_QOS_ADDTS_WHEN_ACM_IS_OFF_MAX                  (1)  /* Send AddTs even when ACM is not set for the AC */
#define CFG_QOS_ADDTS_WHEN_ACM_IS_OFF_DEFAULT              (0)

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

#ifdef FEATURE_RUNTIME_PM
/*
 * config item to enable runtime suspend
 * 1 means runtime suspend is enabled
 * by default runtime suspend is disabled
 */
#define CFG_ENABLE_RUNTIME_PM                  "gRuntimePM"
#define CFG_ENABLE_RUNTIME_PM_MIN              (0)
#define CFG_ENABLE_RUNTIME_PM_MAX              (1)
#define CFG_ENABLE_RUNTIME_PM_DEFAULT          (0)

/*
 * config item for runtime pm's inactivity timer.
 * the wlan driver will wait for this number of miliseconds
 * of inactivity before performing a runtime suspend.
 */
#define CFG_RUNTIME_PM_DELAY_NAME               "gRuntimePMDelay"
#define CFG_RUNTIME_PM_DELAY_MIN                (100)
#define CFG_RUNTIME_PM_DELAY_MAX                (10000)
#define CFG_RUNTIME_PM_DELAY_DEFAULT            (500)
#endif

#define CFG_ENABLE_HOST_NSOFFLOAD_NAME         "hostNSOffload"
#define CFG_ENABLE_HOST_NSOFFLOAD_MIN          (0)
#define CFG_ENABLE_HOST_NSOFFLOAD_MAX          (1)
#define CFG_ENABLE_HOST_NSOFFLOAD_DEFAULT      (1)

#define CFG_BAND_CAPABILITY_NAME          "BandCapability"
#define CFG_BAND_CAPABILITY_MIN           (0)
#define CFG_BAND_CAPABILITY_MAX           (2)
#define CFG_BAND_CAPABILITY_DEFAULT       (0)

#define CFG_ENABLE_BYPASS_11D_NAME                 "gEnableBypass11d"
#define CFG_ENABLE_BYPASS_11D_MIN                  (0)
#define CFG_ENABLE_BYPASS_11D_MAX                  (1)
#define CFG_ENABLE_BYPASS_11D_DEFAULT              (1)

#define CFG_ENABLE_DFS_CHNL_SCAN_NAME              "gEnableDFSChnlScan"
#define CFG_ENABLE_DFS_CHNL_SCAN_MIN               (0)
#define CFG_ENABLE_DFS_CHNL_SCAN_MAX               (1)
#define CFG_ENABLE_DFS_CHNL_SCAN_DEFAULT           (1)

#define CFG_ENABLE_DFS_PNO_CHNL_SCAN_NAME              "gEnableDFSPnoChnlScan"
#define CFG_ENABLE_DFS_PNO_CHNL_SCAN_MIN               (0)
#define CFG_ENABLE_DFS_PNO_CHNL_SCAN_MAX               (1)
#define CFG_ENABLE_DFS_PNO_CHNL_SCAN_DEFAULT           (1)

#define CFG_ENABLE_RAMDUMP_COLLECTION              "gEnableDumpCollect"
#define CFG_ENABLE_RAMDUMP_COLLECTION_MIN          (0)
#define CFG_ENABLE_RAMDUMP_COLLECTION_MAX          (1)
#define CFG_ENABLE_RAMDUMP_COLLECTION_DEFAULT      (1)

typedef enum {
	eHDD_LINK_SPEED_REPORT_ACTUAL = 0,
	eHDD_LINK_SPEED_REPORT_MAX = 1,
	eHDD_LINK_SPEED_REPORT_MAX_SCALED = 2,
} eHddLinkSpeedReportType;
#define CFG_VHT_CHANNEL_WIDTH                "gVhtChannelWidth"
#define CFG_VHT_CHANNEL_WIDTH_MIN            (0)
#define CFG_VHT_CHANNEL_WIDTH_MAX            (4)
#define CFG_VHT_CHANNEL_WIDTH_DEFAULT        (3)

#define CFG_VHT_ENABLE_RX_MCS_8_9               "gVhtRxMCS"
#define CFG_VHT_ENABLE_RX_MCS_8_9_MIN           (0)
#define CFG_VHT_ENABLE_RX_MCS_8_9_MAX           (2)
#define CFG_VHT_ENABLE_RX_MCS_8_9_DEFAULT       (0)

#define CFG_VHT_ENABLE_TX_MCS_8_9               "gVhtTxMCS"
#define CFG_VHT_ENABLE_TX_MCS_8_9_MIN           (0)
#define CFG_VHT_ENABLE_TX_MCS_8_9_MAX           (2)
#define CFG_VHT_ENABLE_TX_MCS_8_9_DEFAULT       (0)

#define CFG_VHT_ENABLE_RX_MCS2x2_8_9               "gVhtRxMCS2x2"
#define CFG_VHT_ENABLE_RX_MCS2x2_8_9_MIN           (0)
#define CFG_VHT_ENABLE_RX_MCS2x2_8_9_MAX           (2)
#define CFG_VHT_ENABLE_RX_MCS2x2_8_9_DEFAULT       (0)

#define CFG_VHT_ENABLE_TX_MCS2x2_8_9               "gVhtTxMCS2x2"
#define CFG_VHT_ENABLE_TX_MCS2x2_8_9_MIN           (0)
#define CFG_VHT_ENABLE_TX_MCS2x2_8_9_MAX           (2)
#define CFG_VHT_ENABLE_TX_MCS2x2_8_9_DEFAULT       (0)

#define CFG_VHT_ENABLE_2x2_CAP_FEATURE         "gEnable2x2"
#define CFG_VHT_ENABLE_2x2_CAP_FEATURE_MIN     (0)
#define CFG_VHT_ENABLE_2x2_CAP_FEATURE_MAX     (1)
#define CFG_VHT_ENABLE_2x2_CAP_FEATURE_DEFAULT (0)

/*
 * NSS cfg bit definition.
 * STA          BIT[0:1]
 * SAP          BIT[2:3]
 * P2P_GO       BIT[4:5]
 * P2P_CLIENT   BIT[6:7]
 * IBSS         BIT[8:9]
 * TDLS         BIT[10:11]
 * P2P_DEVICE   BIT[12:13]
 * OCB          BIT[14:15]
 */
#define CFG_VDEV_TYPE_NSS_2G         "gVdevTypeNss_2g"
#define CFG_VDEV_TYPE_NSS_2G_MIN     (0x5555)
#define CFG_VDEV_TYPE_NSS_2G_MAX     (0xAAAA)
#define CFG_VDEV_TYPE_NSS_2G_DEFAULT (0xAAAA)

#define CFG_VDEV_TYPE_NSS_5G         "gVdevTypeNss_5g"
#define CFG_VDEV_TYPE_NSS_5G_MIN     (0x5555)
#define CFG_VDEV_TYPE_NSS_5G_MAX     (0xAAAA)
#define CFG_VDEV_TYPE_NSS_5G_DEFAULT (0xAAAA)

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

#define CFG_VHT_ENABLE_1x1_TX_CHAINMASK         "gSetTxChainmask1x1"
#define CFG_VHT_ENABLE_1x1_TX_CHAINMASK_MIN     (1)
#define CFG_VHT_ENABLE_1x1_TX_CHAINMASK_MAX     (2)
#define CFG_VHT_ENABLE_1x1_TX_CHAINMASK_DEFAULT (1)

#define CFG_VHT_ENABLE_1x1_RX_CHAINMASK         "gSetRxChainmask1x1"
#define CFG_VHT_ENABLE_1x1_RX_CHAINMASK_MIN     (1)
#define CFG_VHT_ENABLE_1x1_RX_CHAINMASK_MAX     (2)
#define CFG_VHT_ENABLE_1x1_RX_CHAINMASK_DEFAULT (1)

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

#define CFG_DISABLE_DFS_CH_SWITCH                 "gDisableDFSChSwitch"
#define CFG_DISABLE_DFS_CH_SWITCH_MIN             (0)
#define CFG_DISABLE_DFS_CH_SWITCH_MAX             (1)
#define CFG_DISABLE_DFS_CH_SWITCH_DEFAULT         (0)

#define CFG_ENABLE_DFS_MASTER_CAPABILITY               "gEnableDFSMasterCap"
#define CFG_ENABLE_DFS_MASTER_CAPABILITY_MIN           (0)
#define CFG_ENABLE_DFS_MASTER_CAPABILITY_MAX           (1)
#define CFG_ENABLE_DFS_MASTER_CAPABILITY_DEFAULT       (0)

#define CFG_SAP_PREFERRED_CHANNEL_LOCATION          "gSapPreferredChanLocation"
#define CFG_SAP_PREFERRED_CHANNEL_LOCATION_MIN      (0)
#define CFG_SAP_PREFERRED_CHANNEL_LOCATION_MAX      (2)
#define CFG_SAP_PREFERRED_CHANNEL_LOCATION_DEFAULT  (0)

#define CFG_DISABLE_DFS_JAPAN_W53                      "gDisableDfsJapanW53"
#define CFG_DISABLE_DFS_JAPAN_W53_MIN                  (0)
#define CFG_DISABLE_DFS_JAPAN_W53_MAX                  (1)
#define CFG_DISABLE_DFS_JAPAN_W53_DEFAULT              (0)

#define CFG_ENABLE_DFS_PHYERR_FILTEROFFLOAD_NAME       "dfsPhyerrFilterOffload"
#define CFG_ENABLE_DFS_PHYERR_FILTEROFFLOAD_MIN        (0)
#define CFG_ENABLE_DFS_PHYERR_FILTEROFFLOAD_MAX        (1)
#define CFG_ENABLE_DFS_PHYERR_FILTEROFFLOAD_DEFAULT    (0)

#define CFG_REPORT_MAX_LINK_SPEED                  "gReportMaxLinkSpeed"
#define CFG_REPORT_MAX_LINK_SPEED_MIN              (eHDD_LINK_SPEED_REPORT_ACTUAL)
#define CFG_REPORT_MAX_LINK_SPEED_MAX              (eHDD_LINK_SPEED_REPORT_MAX_SCALED)
#define CFG_REPORT_MAX_LINK_SPEED_DEFAULT          (eHDD_LINK_SPEED_REPORT_MAX_SCALED)

/*
 * RSSI Thresholds
 * Used when eHDD_LINK_SPEED_REPORT_SCALED is selected
 */
#define CFG_LINK_SPEED_RSSI_HIGH                   "gLinkSpeedRssiHigh"
#define CFG_LINK_SPEED_RSSI_HIGH_MIN               (-127)
#define CFG_LINK_SPEED_RSSI_HIGH_MAX               (0)
#define CFG_LINK_SPEED_RSSI_HIGH_DEFAULT           (-55)

#define CFG_LINK_SPEED_RSSI_MID                    "gLinkSpeedRssiMed"
#define CFG_LINK_SPEED_RSSI_MID_MIN                (-127)
#define CFG_LINK_SPEED_RSSI_MID_MAX                (0)
#define CFG_LINK_SPEED_RSSI_MID_DEFAULT            (-65)

#define CFG_LINK_SPEED_RSSI_LOW                    "gLinkSpeedRssiLow"
#define CFG_LINK_SPEED_RSSI_LOW_MIN                (-127)
#define CFG_LINK_SPEED_RSSI_LOW_MAX                (0)
#define CFG_LINK_SPEED_RSSI_LOW_DEFAULT            (-80)

#define CFG_P2P_DEVICE_ADDRESS_ADMINISTRATED_NAME                "isP2pDeviceAddrAdministrated"
#define CFG_P2P_DEVICE_ADDRESS_ADMINISTRATED_MIN                 (0)
#define CFG_P2P_DEVICE_ADDRESS_ADMINISTRATED_MAX                 (1)
#define CFG_P2P_DEVICE_ADDRESS_ADMINISTRATED_DEFAULT             (1)

#define CFG_ENABLE_SSR                      "gEnableSSR"
#define CFG_ENABLE_SSR_MIN                  (0)
#define CFG_ENABLE_SSR_MAX                  (1)
#define CFG_ENABLE_SSR_DEFAULT              (1)

#define CFG_ENABLE_OVERLAP_CH               "gEnableOverLapCh"
#define CFG_ENABLE_OVERLAP_CH_MIN           (0)
#define CFG_ENABLE_OVERLAP_CH_MAX           (1)
#define CFG_ENABLE_OVERLAP_CH_DEFAULT       (0)

#define CFG_PPS_ENABLE_5G_EBT                 "gEnable5gEBT"
#define CFG_PPS_ENABLE_5G_EBT_FEATURE_MIN     (0)
#define CFG_PPS_ENABLE_5G_EBT_FEATURE_MAX     (1)
#define CFG_PPS_ENABLE_5G_EBT_FEATURE_DEFAULT (1)

#define CFG_ENABLE_MEMORY_DEEP_SLEEP          "gEnableMemDeepSleep"
#define CFG_ENABLE_MEMORY_DEEP_SLEEP_MIN      (0)
#define CFG_ENABLE_MEMORY_DEEP_SLEEP_MAX      (1)
#define CFG_ENABLE_MEMORY_DEEP_SLEEP_DEFAULT  (1)

/* In cfg.dat 1=1MBPS, 2=2MBPS, 3=5_5MBPS, 4=11MBPS, 5=6MBPS, 6=9MBPS,
 * 7=12MBPS, 8=18MBPS, 9=24MBPS. But 6=9MBPS and 8=18MBPS are not basic
 * 11g rates and should not be set by gDefaultRateIndex24Ghz.
 */

#define CFG_DEFAULT_RATE_INDEX_24GH               "gDefaultRateIndex24Ghz"
#define CFG_DEFAULT_RATE_INDEX_24GH_MIN           (1)
#define CFG_DEFAULT_RATE_INDEX_24GH_MAX           (9)
#define CFG_DEFAULT_RATE_INDEX_24GH_DEFAULT       (1)

#define CFG_ENABLE_PACKET_LOG            "gEnablePacketLog"
#define CFG_ENABLE_PACKET_LOG_MIN        (0)
#define CFG_ENABLE_PACKET_LOG_MAX        (1)
#define CFG_ENABLE_PACKET_LOG_DEFAULT    (1)

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
#define CFG_ENABLE_FW_DEBUG_LOG_LEVEL_DEFAULT  (4)

/* For valid values of log levels check enum DBGLOG_LOG_LVL and
 * for valid values of module ids check enum WLAN_MODULE_ID.
 */
#define CFG_ENABLE_FW_MODULE_LOG_LEVEL    "gFwDebugModuleLoglevel"
#define CFG_ENABLE_FW_MODULE_LOG_DEFAULT  ""

#ifdef FEATURE_GREEN_AP
#define CFG_ENABLE_GREEN_AP_FEATURE         "gEnableGreenAp"
#define CFG_ENABLE_GREEN_AP_FEATURE_MIN     (0)
#define CFG_ENABLE_GREEN_AP_FEATURE_MAX     (1)
#define CFG_ENABLE_GREEN_AP_FEATURE_DEFAULT (1)

/* Enhanced Green AP (EGAP) flags/params */
#define CFG_ENABLE_EGAP_ENABLE_FEATURE             "gEnableEGAP"
#define CFG_ENABLE_EGAP_ENABLE_FEATURE_MIN         (0)
#define CFG_ENABLE_EGAP_ENABLE_FEATURE_MAX         (1)
#define CFG_ENABLE_EGAP_ENABLE_FEATURE_DEFAULT     (0)

#define CFG_ENABLE_EGAP_INACT_TIME_FEATURE         "gEGAPInactTime"
#define CFG_ENABLE_EGAP_INACT_TIME_FEATURE_MIN     (0)
#define CFG_ENABLE_EGAP_INACT_TIME_FEATURE_MAX     (300000)
#define CFG_ENABLE_EGAP_INACT_TIME_FEATURE_DEFAULT (1000)

#define CFG_ENABLE_EGAP_WAIT_TIME_FEATURE          "gEGAPWaitTime"
#define CFG_ENABLE_EGAP_WAIT_TIME_FEATURE_MIN      (0)
#define CFG_ENABLE_EGAP_WAIT_TIME_FEATURE_MAX      (300000)
#define CFG_ENABLE_EGAP_WAIT_TIME_FEATURE_DEFAULT  (100)

#define CFG_ENABLE_EGAP_FLAGS_FEATURE              "gEGAPFeatures"
#define CFG_ENABLE_EGAP_FLAGS_FEATURE_MIN          (0)
#define CFG_ENABLE_EGAP_FLAGS_FEATURE_MAX          (15)
#define CFG_ENABLE_EGAP_FLAGS_FEATURE_DEFAULT      (7)
/* end Enhanced Green AP flags/params */

#endif

#ifdef FEATURE_WLAN_FORCE_SAP_SCC
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
#define CFG_QDF_TRACE_ENABLE_TXRX_NAME    "cfd_trace_enable_txrx"
#define CFG_QDF_TRACE_ENABLE_HTC_NAME     "qdf_trace_enable_htc"
#define CFG_QDF_TRACE_ENABLE_HIF_NAME     "qdf_trace_enable_hif"
#define CFG_CDR_TRACE_ENABLE_HDD_SAP_DATA_NAME   "qdf_trace_enable_hdd_sap_data"
#define CFG_QDF_TRACE_ENABLE_HDD_DATA_NAME       "qdf_trace_enable_hdd_data"

#define CFG_QDF_TRACE_ENABLE_MIN          (0)
#define CFG_QDF_TRACE_ENABLE_MAX          (0xff)
#define CFG_QDF_TRACE_ENABLE_DEFAULT      (0xffff)

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
 *          switch DTIM1 when host resumes */
#define CFG_ENABLE_DYNAMIC_DTIM_NAME            "gEnableDynamicDTIM"
#define CFG_ENABLE_DYNAMIC_DTIM_MIN        (0)
#define CFG_ENABLE_DYNAMIC_DTIM_MAX        (5)
#define CFG_ENABLE_DYNAMIC_DTIM_DEFAULT    (0)

/*
 * Enable First Scan 2G Only
 * Options
 * 0 - Disable First Scan 2G Option
 * 1 - Enable First Scan 2G Option
 */
#define CFG_ENABLE_FIRST_SCAN_2G_ONLY_NAME            "gEnableFirstScan2GOnly"
#define CFG_ENABLE_FIRST_SCAN_2G_ONLY_MIN        (0)
#define CFG_ENABLE_FIRST_SCAN_2G_ONLY_MAX        (1)
#define CFG_ENABLE_FIRST_SCAN_2G_ONLY_DEFAULT    (0)

/*
 * Driver Force ACS is reintroduced for android SAP legacy configuration method.
 * If Driver force acs is enabled, channel/ hw config from hostapd is ignored.
 * Driver uses INI params dot11Mode, channel bonding mode and vht chan width
 * to derive ACS HW mode and operating BW.
 *
 * Non android platforms shall not use force ACS method and rely on hostapd
 * driven ACS method for concurrent SAP ACS configuration, OBSS etc.
 */
#define CFG_FORCE_SAP_ACS                  "gApAutoChannelSelection"
#define CFG_FORCE_SAP_ACS_MIN              (0)
#define CFG_FORCE_SAP_ACS_MAX              (1)
#define CFG_FORCE_SAP_ACS_DEFAULT          (0)

#define CFG_FORCE_SAP_ACS_START_CH         "gAPChannelSelectStartChannel"
#define CFG_FORCE_SAP_ACS_START_CH_MIN     (0)
#define CFG_FORCE_SAP_ACS_START_CH_MAX     (0xFF)
#define CFG_FORCE_SAP_ACS_START_CH_DEFAULT (1)

#define CFG_FORCE_SAP_ACS_END_CH           "gAPChannelSelectEndChannel"
#define CFG_FORCE_SAP_ACS_END_CH_MIN       (0)
#define CFG_FORCE_SAP_ACS_END_CH_MAX       (0xFF)
#define CFG_FORCE_SAP_ACS_END_CH_DEFAULT   (11)

/*
 * Skip DFS Channel in case of P2P Search
 * Options
 * 0 - Don't Skip DFS Channel in case of P2P Search
 * 1 - Skip DFS Channel in case of P2P Search
 */
#define CFG_ENABLE_SKIP_DFS_IN_P2P_SEARCH_NAME       "gSkipDfsChannelInP2pSearch"
#define CFG_ENABLE_SKIP_DFS_IN_P2P_SEARCH_MIN        (0)
#define CFG_ENABLE_SKIP_DFS_IN_P2P_SEARCH_MAX        (1)
#define CFG_ENABLE_SKIP_DFS_IN_P2P_SEARCH_DEFAULT    (1)

/*
 * Ignore Dynamic Dtim in case of P2P
 * Options
 * 0 - Consider Dynamic Dtim incase of P2P
 * 1 - Ignore Dynamic Dtim incase of P2P
 */
#define CFG_IGNORE_DYNAMIC_DTIM_IN_P2P_MODE_NAME       "gIgnoreDynamicDtimInP2pMode"
#define CFG_IGNORE_DYNAMIC_DTIM_IN_P2P_MODE_MIN        (0)
#define CFG_IGNORE_DYNAMIC_DTIM_IN_P2P_MODE_MAX        (1)
#define CFG_IGNORE_DYNAMIC_DTIM_IN_P2P_MODE_DEFAULT    (0)

#define CFG_SHORT_GI_40MHZ_NAME                "gShortGI40Mhz"
#define CFG_SHORT_GI_40MHZ_MIN                 0
#define CFG_SHORT_GI_40MHZ_MAX                 1
#define CFG_SHORT_GI_40MHZ_DEFAULT             1

/*
 * Enable / Disable MCC feature
 * Default: Enable
 */
#define CFG_ENABLE_MCC_ENABLED_NAME             "gEnableMCCMode"
#define CFG_ENABLE_MCC_ENABLED_MIN              (0)
#define CFG_ENABLE_MCC_ENABLED_MAX              (1)
#define CFG_ENABLE_MCC_ENABLED_DEFAULT          (1)

/*
 * Allow GO in MCC mode to accept different beacon interval than STA's.
 * Added for Wi-Fi Cert. 5.1.12
 * Default: gAllowMCCGODiffBI = 2
 * If gAllowMCCGODiffBI = 1  // Set to 1 for WFA certification. GO Beacon
 *                              interval is not changed. MCC GO
 *                              doesn't work well in optimized way.
 *                              In worst scenario, it may invite STA
 *                              disconnection.
 * gAllowMCCGODiffBI = 2     //If set to 2 workaround 1 disassoc all the clients
 *                             and update beacon Interval
 * gAllowMCCGODiffBI = 3     //If set to 3 tear down the P2P link in auto/
 *                             Non-autonomous -GO case
 * gAllowMCCGODiffBI = 4     //If set to 4 don't disconnect the P2P client
 *                             in autonomous/Non-autonomous -GO case update
 *                             the BI dynamically
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
#define CFG_THERMAL_MIGRATION_ENABLE_DEFAULT   (1)

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
 * Enable/Disable Multicast MAC Address List feature
 * Default: Enable
 */
#define CFG_MC_ADDR_LIST_ENABLE_NAME          "gMCAddrListEnable"
#define CFG_MC_ADDR_LIST_ENABLE_MIN           (0)
#define CFG_MC_ADDR_LIST_ENABLE_MAX           (1)
#define CFG_MC_ADDR_LIST_ENABLE_DEFAULT       (1)

#define CFG_ENABLE_RX_STBC                       "gEnableRXSTBC"
#define CFG_ENABLE_RX_STBC_MIN                   (0)
#define CFG_ENABLE_RX_STBC_MAX                   (1)
#define CFG_ENABLE_RX_STBC_DEFAULT               (1)

#define CFG_ENABLE_TX_STBC                       "gEnableTXSTBC"
#define CFG_ENABLE_TX_STBC_MIN                   (0)
#define CFG_ENABLE_TX_STBC_MAX                   (1)
#define CFG_ENABLE_TX_STBC_DEFAULT               (0)

#define CFG_ENABLE_RX_LDPC                       "gEnableRXLDPC"
#define CFG_ENABLE_RX_LDPC_MIN                   (0)
#define CFG_ENABLE_RX_LDPC_MAX                   (1)
#define CFG_ENABLE_RX_LDPC_DEFAULT               (0)

/*
 * Enable/Disable vsta based on MAX Assoc limit
 * defined in WCNSS_qcom_cfg.ini.
 */
#ifdef WLAN_SOFTAP_VSTA_FEATURE
#define CFG_VSTA_SUPPORT_ENABLE               "gEnableVSTASupport"
#define CFG_VSTA_SUPPORT_ENABLE_MIN           (0)
#define CFG_VSTA_SUPPORT_ENABLE_MAX           (1)
#define CFG_VSTA_SUPPORT_ENABLE_DEFAULT       (0)
#endif

#ifdef FEATURE_WLAN_TDLS
#define CFG_TDLS_SUPPORT_ENABLE                     "gEnableTDLSSupport"
#define CFG_TDLS_SUPPORT_ENABLE_MIN                 (0)
#define CFG_TDLS_SUPPORT_ENABLE_MAX                 (1)
#define CFG_TDLS_SUPPORT_ENABLE_DEFAULT             (0)

#define CFG_TDLS_IMPLICIT_TRIGGER                   "gEnableTDLSImplicitTrigger"
#define CFG_TDLS_IMPLICIT_TRIGGER_MIN               (0)
#define CFG_TDLS_IMPLICIT_TRIGGER_MAX               (1)
#define CFG_TDLS_IMPLICIT_TRIGGER_DEFAULT           (0)

#define CFG_TDLS_TX_STATS_PERIOD                    "gTDLSTxStatsPeriod"
#define CFG_TDLS_TX_STATS_PERIOD_MIN                (1000)
#define CFG_TDLS_TX_STATS_PERIOD_MAX                (4294967295UL)
#define CFG_TDLS_TX_STATS_PERIOD_DEFAULT            (2000)

#define CFG_TDLS_TX_PACKET_THRESHOLD                "gTDLSTxPacketThreshold"
#define CFG_TDLS_TX_PACKET_THRESHOLD_MIN            (0)
#define CFG_TDLS_TX_PACKET_THRESHOLD_MAX            (4294967295UL)
#define CFG_TDLS_TX_PACKET_THRESHOLD_DEFAULT        (40)

#define CFG_TDLS_MAX_DISCOVERY_ATTEMPT              "gTDLSMaxDiscoveryAttempt"
#define CFG_TDLS_MAX_DISCOVERY_ATTEMPT_MIN          (1)
#define CFG_TDLS_MAX_DISCOVERY_ATTEMPT_MAX          (100)
#define CFG_TDLS_MAX_DISCOVERY_ATTEMPT_DEFAULT      (5)

/*  teardown notification interval (gTDLSIdleTimeout) should be multiple of
 *  setup notification (gTDLSTxStatsPeriod) interval.
 *  e.g.
 *       if setup notification (gTDLSTxStatsPeriod) interval = 500, then
 *       teardown notification (gTDLSIdleTimeout) interval should be 1000,
 *       1500, 2000, 2500...
 */
#define CFG_TDLS_IDLE_TIMEOUT                       "gTDLSIdleTimeout"
#define CFG_TDLS_IDLE_TIMEOUT_MIN                   (500)
#define CFG_TDLS_IDLE_TIMEOUT_MAX                   (40000)
#define CFG_TDLS_IDLE_TIMEOUT_DEFAULT               (5000)


#define CFG_TDLS_IDLE_PACKET_THRESHOLD              "gTDLSIdlePacketThreshold"
#define CFG_TDLS_IDLE_PACKET_THRESHOLD_MIN          (0)
#define CFG_TDLS_IDLE_PACKET_THRESHOLD_MAX          (40000)
#define CFG_TDLS_IDLE_PACKET_THRESHOLD_DEFAULT      (3)

#define CFG_TDLS_RSSI_TRIGGER_THRESHOLD             "gTDLSRSSITriggerThreshold"
#define CFG_TDLS_RSSI_TRIGGER_THRESHOLD_MIN         (-120)
#define CFG_TDLS_RSSI_TRIGGER_THRESHOLD_MAX         (0)
#define CFG_TDLS_RSSI_TRIGGER_THRESHOLD_DEFAULT     (-75)

#define CFG_TDLS_RSSI_TEARDOWN_THRESHOLD            "gTDLSRSSITeardownThreshold"
#define CFG_TDLS_RSSI_TEARDOWN_THRESHOLD_MIN        (-120)
#define CFG_TDLS_RSSI_TEARDOWN_THRESHOLD_MAX        (0)
#define CFG_TDLS_RSSI_TEARDOWN_THRESHOLD_DEFAULT    (-75)

#define CFG_TDLS_RSSI_DELTA                         "gTDLSRSSIDelta"
#define CFG_TDLS_RSSI_DELTA_MIN                     (-30)
#define CFG_TDLS_RSSI_DELTA_MAX                     (0)
#define CFG_TDLS_RSSI_DELTA_DEFAULT                 (-20)

#define CFG_TDLS_QOS_WMM_UAPSD_MASK_NAME            "gTDLSUapsdMask"    /* ACs to setup U-APSD for TDLS Sta */
#define CFG_TDLS_QOS_WMM_UAPSD_MASK_MIN             (0)
#define CFG_TDLS_QOS_WMM_UAPSD_MASK_MAX             (0x0F)
#define CFG_TDLS_QOS_WMM_UAPSD_MASK_DEFAULT         (0x0F)

#define CFG_TDLS_BUFFER_STA_SUPPORT_ENABLE          "gEnableTDLSBufferSta"
#define CFG_TDLS_BUFFER_STA_SUPPORT_ENABLE_MIN      (0)
#define CFG_TDLS_BUFFER_STA_SUPPORT_ENABLE_MAX      (1)
/* Buffer STA is not enabled in CLD 2.0 yet */
#define CFG_TDLS_BUFFER_STA_SUPPORT_ENABLE_DEFAULT  (1)

#define CFG_TDLS_PUAPSD_INACTIVITY_TIME             "gTDLSPuapsdInactivityTime"
#define CFG_TDLS_PUAPSD_INACTIVITY_TIME_MIN         (0)
#define CFG_TDLS_PUAPSD_INACTIVITY_TIME_MAX         (10)
#define CFG_TDLS_PUAPSD_INACTIVITY_TIME_DEFAULT     (0)

#define CFG_TDLS_PUAPSD_RX_FRAME_THRESHOLD          "gTDLSPuapsdRxFrameThreshold"
#define CFG_TDLS_PUAPSD_RX_FRAME_THRESHOLD_MIN      (10)
#define CFG_TDLS_PUAPSD_RX_FRAME_THRESHOLD_MAX      (20)
#define CFG_TDLS_PUAPSD_RX_FRAME_THRESHOLD_DEFAULT  (10)

#define CFG_TDLS_PUAPSD_PEER_TRAFFIC_IND_WINDOW          "gTDLSPuapsdPTIWindow"
#define CFG_TDLS_PUAPSD_PEER_TRAFFIC_IND_WINDOW_MIN      (1)
#define CFG_TDLS_PUAPSD_PEER_TRAFFIC_IND_WINDOW_MAX      (5)
#define CFG_TDLS_PUAPSD_PEER_TRAFFIC_IND_WINDOW_DEFAULT  (2)

#define CFG_TDLS_PUAPSD_PEER_TRAFFIC_RSP_TIMEOUT         "gTDLSPuapsdPTRTimeout"
#define CFG_TDLS_PUAPSD_PEER_TRAFFIC_RSP_TIMEOUT_MIN     (0)
#define CFG_TDLS_PUAPSD_PEER_TRAFFIC_RSP_TIMEOUT_MAX     (10000)
#define CFG_TDLS_PUAPSD_PEER_TRAFFIC_RSP_TIMEOUT_DEFAULT (5000)

#define CFG_TDLS_EXTERNAL_CONTROL                   "gTDLSExternalControl"
#define CFG_TDLS_EXTERNAL_CONTROL_MIN               (0)
#define CFG_TDLS_EXTERNAL_CONTROL_MAX               (1)
#define CFG_TDLS_EXTERNAL_CONTROL_DEFAULT           (0)

#define CFG_TDLS_OFF_CHANNEL_SUPPORT_ENABLE          "gEnableTDLSOffChannel"
#define CFG_TDLS_OFF_CHANNEL_SUPPORT_ENABLE_MIN      (0)
#define CFG_TDLS_OFF_CHANNEL_SUPPORT_ENABLE_MAX      (1)
#define CFG_TDLS_OFF_CHANNEL_SUPPORT_ENABLE_DEFAULT  (0)

#define CFG_TDLS_WMM_MODE_ENABLE                     "gEnableTDLSWmmMode"
#define CFG_TDLS_WMM_MODE_ENABLE_MIN                 (0)
#define CFG_TDLS_WMM_MODE_ENABLE_MAX                 (1)
#define CFG_TDLS_WMM_MODE_ENABLE_DEFAULT             (1)

#define CFG_TDLS_PREFERRED_OFF_CHANNEL_NUM          "gTDLSPrefOffChanNum"
#define CFG_TDLS_PREFERRED_OFF_CHANNEL_NUM_MIN      (1)
#define CFG_TDLS_PREFERRED_OFF_CHANNEL_NUM_MAX      (165)
#define CFG_TDLS_PREFERRED_OFF_CHANNEL_NUM_DEFAULT  (36)

#define CFG_TDLS_PREFERRED_OFF_CHANNEL_BW           "gTDLSPrefOffChanBandwidth"
#define CFG_TDLS_PREFERRED_OFF_CHANNEL_BW_MIN      (0)
#define CFG_TDLS_PREFERRED_OFF_CHANNEL_BW_MAX      (0x0F)
#define CFG_TDLS_PREFERRED_OFF_CHANNEL_BW_DEFAULT  (0x07)

/* Enable TDLS Scan: Allow scan and maintain TDLS link.
 *  0: If peer is not buffer STA capable and device is not sleep STA
 *     capable, then teardown TDLS link when scan is initiated. If peer
 *     is buffer STA and we can be sleep STA then TDLS link is maintained
 *     during scan.
 *  1: Maintain TDLS link and allow scan even if peer is not buffer STA
 *     capable and device is not sleep STA capable. There will be loss of
 *     Rx pkts since peer would not know when device moves away from tdls
 *     channel. Tx on TDLS link would stop when device moves away from tdls
 *     channel.
 */
#define CFG_TDLS_SCAN_ENABLE                       "gEnableTDLSScan"
#define CFG_TDLS_SCAN_ENABLE_MIN                   (0)
#define CFG_TDLS_SCAN_ENABLE_MAX                   (1)
#define CFG_TDLS_SCAN_ENABLE_DEFAULT               (0)

/* TDLS peer kickout threshold to fw
 *     Firmware will use this value to determine, when to send TDLS
 *     peer kick out event to host.
 *     E.g.
 *        if peer kick out threshold is 10, then firmware will wait for 10
 *        consecutive packet failures and then send TDLS kickout
 *        notification to host driver
 */
#define CFG_TDLS_PEER_KICKOUT_THRESHOLD            "gTDLSPeerKickoutThreshold"
#define CFG_TDLS_PEER_KICKOUT_THRESHOLD_MIN        (10)
#define CFG_TDLS_PEER_KICKOUT_THRESHOLD_MAX        (5000)
#define CFG_TDLS_PEER_KICKOUT_THRESHOLD_DEFAULT    (96)

#endif

/* Enable/Disable LPWR Image(cMEM uBSP) Transition */
#define CFG_ENABLE_LPWR_IMG_TRANSITION_NAME        "gEnableLpwrImgTransition"
#define CFG_ENABLE_LPWR_IMG_TRANSITION_MIN         (0)
#define CFG_ENABLE_LPWR_IMG_TRANSITION_MAX         (1)
#define CFG_ENABLE_LPWR_IMG_TRANSITION_DEFAULT     (0)

/*
 * Scan Aging timeout value in seconds
 */
#define CFG_SCAN_AGING_PARAM_NAME          "gScanAgingTime"
#define CFG_SCAN_AGING_PARAM_MIN           (0)
#define CFG_SCAN_AGING_PARAM_MAX           (200)
#define CFG_SCAN_AGING_PARAM_DEFAULT       (30)

/* Config Param to enable the txLdpc capability
 * 0 - disable
 * 1 - HT LDPC enable
 * 2 - VHT LDPC enable
 * 3 - HT & VHT LDPC enable */
#define CFG_TX_LDPC_ENABLE_FEATURE         "gTxLdpcEnable"
#define CFG_TX_LDPC_ENABLE_FEATURE_MIN     (0)
#define CFG_TX_LDPC_ENABLE_FEATURE_MAX     (3)
#define CFG_TX_LDPC_ENABLE_FEATURE_DEFAULT (3)

/*
 * Enable / Disable MCC Adaptive Scheduler feature
 * Default: Enable
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
#define CFG_VHT_CSN_BEAMFORMEE_ANT_SUPPORTED_DEFAULT (WNI_CFG_VHT_CSN_BEAMFORMEE_ANT_SUPPORTED_STAMAX - 1)

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

/*
 * IBSS Operating Channels for 2.4G and 5GHz channels
 */
#define CFG_IBSS_ADHOC_CHANNEL_5GHZ_NAME          "gAdHocChannel5G"
#define CFG_IBSS_ADHOC_CHANNEL_5GHZ_MIN           (36)
#define CFG_IBSS_ADHOC_CHANNEL_5GHZ_MAX           (165)
#define CFG_IBSS_ADHOC_CHANNEL_5GHZ_DEFAULT       (44)

#define CFG_IBSS_ADHOC_CHANNEL_24GHZ_NAME         "gAdHocChannel24G"
#define CFG_IBSS_ADHOC_CHANNEL_24GHZ_MIN          (1)
#define CFG_IBSS_ADHOC_CHANNEL_24GHZ_MAX          (14)
#define CFG_IBSS_ADHOC_CHANNEL_24GHZ_DEFAULT      (6)

/* Parameter to control VHT support in 2.4 GHz band */
#define CFG_ENABLE_VHT_FOR_24GHZ_NAME             "gEnableVhtFor24GHzBand"
#define CFG_ENABLE_VHT_FOR_24GHZ_MIN              (0)
#define CFG_ENABLE_VHT_FOR_24GHZ_MAX              (1)
#define CFG_ENABLE_VHT_FOR_24GHZ_DEFAULT          (0)

#define CFG_MAX_MEDIUM_TIME                      "gMaxMediumTime"
#define CFG_MAX_MEDIUM_TIME_STAMIN               WNI_CFG_MAX_MEDIUM_TIME_STAMIN
#define CFG_MAX_MEDIUM_TIME_STAMAX               WNI_CFG_MAX_MEDIUM_TIME_STAMAX
#define CFG_MAX_MEDIUM_TIME_STADEFAULT           WNI_CFG_MAX_MEDIUM_TIME_STADEF

/*
 * Enable legacy fast roaming (LFR) on STA link during concurrent sessions
 */
#define CFG_ENABLE_FAST_ROAM_IN_CONCURRENCY          "gEnableFastRoamInConcurrency"
#define CFG_ENABLE_FAST_ROAM_IN_CONCURRENCY_MIN      (0)
#define CFG_ENABLE_FAST_ROAM_IN_CONCURRENCY_MAX      (1)
#define CFG_ENABLE_FAST_ROAM_IN_CONCURRENCY_DEFAULT  (1)

/*
 * Enable heart beat monitoring offload to FW
 */
#define CFG_ENABLE_HEART_BEAT_OFFLOAD          "gEnableIbssHeartBeatOffload"
#define CFG_ENABLE_HEART_BEAT_OFFLOAD_MIN      (0)
#define CFG_ENABLE_HEART_BEAT_OFFLOAD_MAX      (1)
#define CFG_ENABLE_HEART_BEAT_OFFLOAD_DEFAULT  (1)

#define CFG_ANTENNA_DIVERSITY_PARAM_NAME          "gAntennaDiversity"
#define CFG_ANTENNA_DIVERSITY_PARAM_MIN           (0)
#define CFG_ANTENNA_DIVERSITY_PARAM_MAX           (3)
#define CFG_ANTENNA_DIVERSITY_PARAM_DEFAULT       (0)

#define CFG_ENABLE_SNR_MONITORING_NAME              "gEnableSNRMonitoring"
#define CFG_ENABLE_SNR_MONITORING_MIN               (0)
#define CFG_ENABLE_SNR_MONITORING_MAX               (1)
#define CFG_ENABLE_SNR_MONITORING_DEFAULT           (0)

#ifdef FEATURE_WLAN_SCAN_PNO
#define CFG_PNO_SCAN_SUPPORT                         "gPNOScanSupport"
#define CFG_PNO_SCAN_SUPPORT_ENABLE                  (1)
#define CFG_PNO_SCAN_SUPPORT_DISABLE                 (0)
#define CFG_PNO_SCAN_SUPPORT_DEFAULT                 (1)

#define CFG_PNO_SCAN_TIMER_REPEAT_VALUE              "gPNOScanTimerRepeatValue"
#define CFG_PNO_SCAN_TIMER_REPEAT_VALUE_DEFAULT      (30)
#define CFG_PNO_SCAN_TIMER_REPEAT_VALUE_MIN          (0)
#define CFG_PNO_SCAN_TIMER_REPEAT_VALUE_MAX          (0xffffffff)

#define CFG_PNO_SLOW_SCAN_MULTIPLIER                 "gPNOSlowScanMultiplier"
#define CFG_PNO_SLOW_SCAN_MULTIPLIER_DEFAULT         (6)
#define CFG_PNO_SLOW_SCAN_MULTIPLIER_MIN             (0)
#define CFG_PNO_SLOW_SCAN_MULTIPLIER_MAX             (30)
#endif

#define CFG_AMSDU_SUPPORT_IN_AMPDU_NAME                "gAmsduSupportInAMPDU"
#define CFG_AMSDU_SUPPORT_IN_AMPDU_MIN                 (0)
#define CFG_AMSDU_SUPPORT_IN_AMPDU_MAX                 (1)
#define CFG_AMSDU_SUPPORT_IN_AMPDU_DEFAULT             (0)      /* disabled */

/* Prefer connecting to 5G AP even if its RSSI is lower by
   gSelect5GHzMargin dBm than 2.4G AP.
   This feature requires the dependent cfg.ini "gRoamPrefer5GHz" set to 1 */
#define CFG_STRICT_5GHZ_PREF_BY_MARGIN                 "gSelect5GHzMargin"
#define CFG_STRICT_5GHZ_PREF_BY_MARGIN_MIN             (0)
#define CFG_STRICT_5GHZ_PREF_BY_MARGIN_MAX             (60)
#define CFG_STRICT_5GHZ_PREF_BY_MARGIN_DEFAULT         (0)      /* set 0 to disable */

/* It enables IP, TCP and UDP checksum offload in hardware
 * and also advertise same to network stack.
 */
#define CFG_ENABLE_IP_TCP_UDP_CHKSUM_OFFLOAD            "gEnableIpTcpUdpChecksumOffload"
#define CFG_ENABLE_IP_TCP_UDP_CHKSUM_OFFLOAD_DISABLE    (0)
#define CFG_ENABLE_IP_TCP_UDP_CHKSUM_OFFLOAD_ENABLE     (1)
#define CFG_ENABLE_IP_TCP_UDP_CHKSUM_OFFLOAD_DEFAULT    (CFG_ENABLE_IP_TCP_UDP_CHKSUM_OFFLOAD_ENABLE)

/*
 * Power Save Offload
 * Power Save Offload configuration:
 * Current values of gEnablePowerSaveOffload:
 * 0 -> Power save offload is disabled
 * 1 -> Legacy Power save enabled + Deep sleep Disabled
 * 2 -> QPower enabled + Deep sleep Disabled
 * 3 -> Legacy Power save enabled + Deep sleep Enabled
 * 4 -> QPower enabled + Deep sleep Enabled
 * 5 -> Duty cycling QPower enabled
 */
#define CFG_POWERSAVE_OFFLOAD_NAME                "gEnablePowerSaveOffload"
#define CFG_POWERSAVE_OFFLOAD_MIN                 (0)
#define CFG_POWERSAVE_OFFLOAD_MAX                 (PS_DUTY_CYCLING_QPOWER)
#define CFG_POWERSAVE_OFFLOAD_DEFAULT             (CFG_POWERSAVE_OFFLOAD_MIN)

#ifdef WLAN_FEATURE_FASTPATH
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
#define CFG_ENABLE_FW_LOG_ENABLE                 (1)
#ifdef QCA_WIFI_3_0_ADRASTEA
#define CFG_ENABLE_FW_LOG_DEFAULT                (CFG_ENABLE_FW_LOG_DISABLE)
#else
#define CFG_ENABLE_FW_LOG_DEFAULT                (CFG_ENABLE_FW_LOG_ENABLE)
#endif

/*
 * Enable/Disable SSR for USB
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

#define CFG_MAX_WOW_FILTERS_NAME                       "gMaxWoWFilters"
#define CFG_MAX_WOW_FILTERS_MIN                        (0)
#define CFG_MAX_WOW_FILTERS_MAX                        (WOW_MAX_BITMAP_FILTERS)
#define CFG_MAX_WOW_FILTERS_DEFAULT                    (WOW_MAX_BITMAP_FILTERS)

/*
 * WOW Enable/Disable.
 * 0 - Disable both magic pattern match and pattern byte match.
 * 1 - Enable magic pattern match on all interfaces.
 * 2 - Enable pattern byte match on all interfaces.
 * 3 - Enable both magic patter and pattern byte match on all interfaces.
 */
#define CFG_WOW_STATUS_NAME                           "gEnableWoW"
#define CFG_WOW_ENABLE_MIN                            (0)
#define CFG_WOW_ENABLE_MAX                            (3)
#define CFG_WOW_STATUS_DEFAULT                        (3)

#define CFG_COALESING_IN_IBSS_NAME                "gCoalesingInIBSS"
#define CFG_COALESING_IN_IBSS_MIN                 (0)
#define CFG_COALESING_IN_IBSS_MAX                 (1)
#define CFG_COALESING_IN_IBSS_DEFAULT             (0)   /* disabled */

#define CFG_IBSS_ATIM_WIN_SIZE_NAME                "gIbssATIMWinSize"
#define CFG_IBSS_ATIM_WIN_SIZE_MIN                 (0)
#define CFG_IBSS_ATIM_WIN_SIZE_MAX                 (50)
#define CFG_IBSS_ATIM_WIN_SIZE_DEFAULT             (0)

/*
 * Indicates if IBSS Power Save is
 * supported or not. When not allowed,
 * IBSS station has to stay awake all
 * the time and should never set PM=1
 * in its transmitted frames. This
 * parameter is meaningful/valid only
 * when gIbssATIMWinSize is non-zero
 */
#define CFG_IBSS_IS_POWER_SAVE_ALLOWED_NAME        "gIbssIsPowerSaveAllowed"
#define CFG_IBSS_IS_POWER_SAVE_ALLOWED_MIN         (0)
#define CFG_IBSS_IS_POWER_SAVE_ALLOWED_MAX         (1)
#define CFG_IBSS_IS_POWER_SAVE_ALLOWED_DEFAULT     (1)

/*
 * Indicates if IBSS Power Collapse
 * is allowed or not.
 */
#define CFG_IBSS_IS_POWER_COLLAPSE_ALLOWED_NAME    "gIbssIsPowerCollapseAllowed"
#define CFG_IBSS_IS_POWER_COLLAPSE_ALLOWED_MIN     (0)
#define CFG_IBSS_IS_POWER_COLLAPSE_ALLOWED_MAX     (1)
#define CFG_IBSS_IS_POWER_COLLAPSE_ALLOWED_DEFAULT (1)

/*
 * This parameter indicates whether IBSS station
 * can exit power save mode and enter power active
 * state whenever there is a TX/RX activity.
 */
#define CFG_IBSS_AWAKE_ON_TX_RX_NAME               "gIbssAwakeOnTxRx"
#define CFG_IBSS_AWAKE_ON_TX_RX_MIN                (0)
#define CFG_IBSS_AWAKE_ON_TX_RX_MAX                (1)
#define CFG_IBSS_AWAKE_ON_TX_RX_DEFAULT            (0)

/*
 * In IBSS mode if Awake on TX/RX activity is enabled
 * Ibss Inactivity parameter indicates the data
 * inactivity time in number of beacon intervals
 * after which IBSS station re-inters power save
 * by sending Null frame with PM=1
 */
#define CFG_IBSS_INACTIVITY_TIME_NAME              "gIbssInactivityTime"
#define CFG_IBSS_INACTIVITY_TIME_MIN               (1)
#define CFG_IBSS_INACTIVITY_TIME_MAX               (10)
#define CFG_IBSS_INACTIVITY_TIME_DEFAULT           (1)

/*
 * In IBSS mode Tx Service Period Inactivity
 * time in msecs indicates the time after
 * which TX Service Period is terminated by
 * sending a Qos Null frame with EOSP.
 * If value is 0, TX SP is terminated with the
 * last buffered packet itself instead of waiting
 * for the inactivity
 */
#define CFG_IBSS_TXSP_END_INACTIVITY_NAME          "gIbssTxSpEndInactivityTime"
#define CFG_IBSS_TXSP_END_INACTIVITY_MIN           (0)
#define CFG_IBSS_TXSP_END_INACTIVITY_MAX           (100)
#define CFG_IBSS_TXSP_END_INACTIVITY_DEFAULT       (0)

/*
 * When IBSS network is initialized, PS-supporting device
 * does not enter protocol sleep state during first
 * gIbssPsWarmupTime seconds.
 */
#define CFG_IBSS_PS_WARMUP_TIME_NAME               "gIbssPsWarmupTime"
#define CFG_IBSS_PS_WARMUP_TIME_MIN                (0)
/* Allow unsigned Int Max for now */
#define CFG_IBSS_PS_WARMUP_TIME_MAX                (65535)
#define CFG_IBSS_PS_WARMUP_TIME_DEFAULT            (0)

/*
 * IBSS Power Save Enable/Disable 1 RX
 * chain usage during the ATIM window
 */
#define CFG_IBSS_PS_1RX_CHAIN_IN_ATIM_WINDOW_NAME    "gIbssPs1RxChainInAtim"
#define CFG_IBSS_PS_1RX_CHAIN_IN_ATIM_WINDOW_MIN     (0)
#define CFG_IBSS_PS_1RX_CHAIN_IN_ATIM_WINDOW_MAX     (1)
#define CFG_IBSS_PS_1RX_CHAIN_IN_ATIM_WINDOW_DEFAULT (0)

#define CFG_SAP_MAX_NO_PEERS                       "gSoftApMaxPeers"
#define CFG_SAP_MAX_NO_PEERS_MIN                   (1)
#define CFG_SAP_MAX_NO_PEERS_MAX                   (32)
#define CFG_SAP_MAX_NO_PEERS_DEFAULT               (32)

/*
 * Connection related log Enable/Disable.
 * 0x1 - Enable mgmt pkt logs (excpet probe req/rsp, beacons).
 * 0x2 - Enable EAPOL pkt logs.
 * 0x4 - Enable DHCP pkt logs.
 * 0x8 - Enable mgmt action frames logs.
 * 0x0 - Disable all the above connection related logs.
 *
 * The default value of 0x0F will enable all the above logs.
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
#define CFG_LL_TX_FLOW_LWM                         "TxFlowLowWaterMark"
#define CFG_LL_TX_FLOW_LWM_MIN                     (0)
#define CFG_LL_TX_FLOW_LWM_MAX                     (1000)

#define CFG_LL_TX_FLOW_LWM_DEFAULT                 (300)

#define CFG_LL_TX_FLOW_HWM_OFFSET                  "TxFlowHighWaterMarkOffset"
#define CFG_LL_TX_FLOW_HWM_OFFSET_MIN              (0)
#define CFG_LL_TX_FLOW_HWM_OFFSET_MAX              (300)

#define CFG_LL_TX_FLOW_HWM_OFFSET_DEFAULT          (94)

#define CFG_LL_TX_FLOW_MAX_Q_DEPTH                 "TxFlowMaxQueueDepth"
#define CFG_LL_TX_FLOW_MAX_Q_DEPTH_MIN             (400)
#define CFG_LL_TX_FLOW_MAX_Q_DEPTH_MAX             (3500)
#define CFG_LL_TX_FLOW_MAX_Q_DEPTH_DEFAULT         (1500)

#define CFG_LL_TX_LBW_FLOW_LWM                     "TxLbwFlowLowWaterMark"
#define CFG_LL_TX_LBW_FLOW_LWM_MIN                 (0)
#define CFG_LL_TX_LBW_FLOW_LWM_MAX                 (1000)

#define CFG_LL_TX_LBW_FLOW_LWM_DEFAULT             (450)

#define CFG_LL_TX_LBW_FLOW_HWM_OFFSET              "TxLbwFlowHighWaterMarkOffset"
#define CFG_LL_TX_LBW_FLOW_HWM_OFFSET_MIN          (0)
#define CFG_LL_TX_LBW_FLOW_HWM_OFFSET_MAX          (300)

#define CFG_LL_TX_LBW_FLOW_HWM_OFFSET_DEFAULT      (50)

#define CFG_LL_TX_LBW_FLOW_MAX_Q_DEPTH             "TxLbwFlowMaxQueueDepth"
#define CFG_LL_TX_LBW_FLOW_MAX_Q_DEPTH_MIN         (400)
#define CFG_LL_TX_LBW_FLOW_MAX_Q_DEPTH_MAX         (3500)
#define CFG_LL_TX_LBW_FLOW_MAX_Q_DEPTH_DEFAULT     (750)

#define CFG_LL_TX_HBW_FLOW_LWM                     "TxHbwFlowLowWaterMark"
#define CFG_LL_TX_HBW_FLOW_LWM_MIN                 (0)
#define CFG_LL_TX_HBW_FLOW_LWM_MAX                 (1000)

#define CFG_LL_TX_HBW_FLOW_LWM_DEFAULT             (406)

#define CFG_LL_TX_HBW_FLOW_HWM_OFFSET              "TxHbwFlowHighWaterMarkOffset"
#define CFG_LL_TX_HBW_FLOW_HWM_OFFSET_MIN          (0)
#define CFG_LL_TX_HBW_FLOW_HWM_OFFSET_MAX          (300)

#define CFG_LL_TX_HBW_FLOW_HWM_OFFSET_DEFAULT      (94)

#define CFG_LL_TX_HBW_FLOW_MAX_Q_DEPTH             "TxHbwFlowMaxQueueDepth"
#define CFG_LL_TX_HBW_FLOW_MAX_Q_DEPTH_MIN         (400)
#define CFG_LL_TX_HBW_FLOW_MAX_Q_DEPTH_MAX         (3500)
#define CFG_LL_TX_HBW_FLOW_MAX_Q_DEPTH_DEFAULT     (1500)
#endif /* QCA_LL_LEGACY_TX_FLOW_CONTROL */

#ifdef QCA_LL_TX_FLOW_CONTROL_V2

#define CFG_LL_TX_FLOW_STOP_QUEUE_TH		   "TxFlowStopQueueThreshold"
#define CFG_LL_TX_FLOW_STOP_QUEUE_TH_DEFAULT	   (15)
#define CFG_LL_TX_FLOW_STOP_QUEUE_TH_MIN	   (0)
#define CFG_LL_TX_FLOW_STOP_QUEUE_TH_MAX	   (50)

#define CFG_LL_TX_FLOW_START_QUEUE_OFFSET	   "TxFlowStartQueueOffset"
#define CFG_LL_TX_FLOW_START_QUEUE_OFFSET_DEFAULT  (10)
#define CFG_LL_TX_FLOW_START_QUEUE_OFFSET_MIN	   (0)
#define CFG_LL_TX_FLOW_START_QUEUE_OFFSET_MAX	   (30)

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
#define CFG_RA_FILTER_ENABLE_DEFAULT               (0)

#define CFG_RA_RATE_LIMIT_INTERVAL_NAME            "gRArateLimitInterval"
#define CFG_RA_RATE_LIMIT_INTERVAL_MIN             (60)
#define CFG_RA_RATE_LIMIT_INTERVAL_MAX             (3600)
#define CFG_RA_RATE_LIMIT_INTERVAL_DEFAULT         (60) /*60 SEC */
#endif

#define CFG_IGNORE_PEER_ERP_INFO_NAME      "gIgnorePeerErpInfo"
#define CFG_IGNORE_PEER_ERP_INFO_MIN       (0)
#define CFG_IGNORE_PEER_ERP_INFO_MAX       (1)
#define CFG_IGNORE_PEER_ERP_INFO_DEFAULT   (0)

/* Enable Memory Debug */
#ifdef MEMORY_DEBUG
#define CFG_ENABLE_MEMORY_DEBUG_NAME             "gEnableMemoryDebug"
#define CFG_ENABLE_MEMORY_DEBUG_MIN              (0)
#define CFG_ENABLE_MEMORY_DEBUG_MAX              (1)
#define CFG_ENABLE_MEMORY_DEBUG_DEFAULT          (1)
#endif

#define CFG_INITIAL_DWELL_TIME_NAME            "gInitialDwellTime"
#define CFG_INITIAL_DWELL_TIME_DEFAULT         (0)
#define CFG_INITIAL_DWELL_TIME_MIN             (0)
#define CFG_INITIAL_DWELL_TIME_MAX             (100)

#define CFG_INITIAL_SCAN_NO_DFS_CHNL_NAME         "gInitialScanNoDFSChnl"
#define CFG_INITIAL_SCAN_NO_DFS_CHNL_DEFAULT      (0)
#define CFG_INITIAL_SCAN_NO_DFS_CHNL_MIN          (0)
#define CFG_INITIAL_SCAN_NO_DFS_CHNL_MAX          (1)

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
 * Dynamic configuration of tcp delack is enabled by default.
 * User can set gTcpDelAckEnable flag to 0 in the INI file to disable dynamic
 * reconfiguration of tcp delack. This does not disable the update of receive
 * histogram
 */
#define CFG_ENABLE_TCP_DELACK                      "gTcpDelAckEnable"
#define CFG_ENABLE_TCP_DELACK_DEFAULT              (1)
#define CFG_ENABLE_TCP_DELACK_MIN                  (0)
#define CFG_ENABLE_TCP_DELACK_MAX                  (1)

#define CFG_TCP_DELACK_THRESHOLD_HIGH              "gTcpDelAckThresholdHigh"
#define CFG_TCP_DELACK_THRESHOLD_HIGH_DEFAULT      (500)
#define CFG_TCP_DELACK_THRESHOLD_HIGH_MIN          (0)
#define CFG_TCP_DELACK_THRESHOLD_HIGH_MAX          (16000)

#define CFG_TCP_DELACK_THRESHOLD_LOW               "gTcpDelAckThresholdLow"
#define CFG_TCP_DELACK_THRESHOLD_LOW_DEFAULT       (1000)
#define CFG_TCP_DELACK_THRESHOLD_LOW_MIN           (0)
#define CFG_TCP_DELACK_THRESHOLD_LOW_MAX           (10000)

#define CFG_TCP_DELACK_TIMER_COUNT                 "gTcpDelAckTimerCount"
#define CFG_TCP_DELACK_TIMER_COUNT_DEFAULT         (30)
#define CFG_TCP_DELACK_TIMER_COUNT_MIN             (1)
#define CFG_TCP_DELACK_TIMER_COUNT_MAX             (1000)


/* TCP_TX_HIGH_TPUT_THRESHOLD specifies the threshold of packets transmitted
 * over a period of 100 ms beyond which TCP can be considered to have a high
 * TX throughput requirement. The driver uses this condition to tweak TCP TX
 * specific parameters (via cnss-daemon).
 * default  - 500
 */
#define CFG_TCP_TX_HIGH_TPUT_THRESHOLD_NAME         "gTcpTxHighTputThreshold"
#define CFG_TCP_TX_HIGH_TPUT_THRESHOLD_DEFAULT      (500)
#define CFG_TCP_TX_HIGH_TPUT_THRESHOLD_MIN          (0)
#define CFG_TCP_TX_HIGH_TPUT_THRESHOLD_MAX          (16000)

#endif /* MSM_PLATFORM */

#ifdef WLAN_FEATURE_11W
#define CFG_PMF_SA_QUERY_MAX_RETRIES_NAME          "pmfSaQueryMaxRetries"
#define CFG_PMF_SA_QUERY_MAX_RETRIES_DEFAULT       (5)
#define CFG_PMF_SA_QUERY_MAX_RETRIES_MIN           (0)
#define CFG_PMF_SA_QUERY_MAX_RETRIES_MAX           (20)

#define CFG_PMF_SA_QUERY_RETRY_INTERVAL_NAME       "pmfSaQueryRetryInterval"
#define CFG_PMF_SA_QUERY_RETRY_INTERVAL_DEFAULT    (200)
#define CFG_PMF_SA_QUERY_RETRY_INTERVAL_MIN        (0)
#define CFG_PMF_SA_QUERY_RETRY_INTERVAL_MAX        (2000)
#endif

#define CFG_MAX_CONCURRENT_CONNECTIONS_NAME        "gMaxConcurrentActiveSessions"
#define CFG_MAX_CONCURRENT_CONNECTIONS_DEFAULT     (3)
#define CFG_MAX_CONCURRENT_CONNECTIONS_MIN         (1)
#define CFG_MAX_CONCURRENT_CONNECTIONS_MAX         (4)

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
#define CFG_REORDER_OFFLOAD_SUPPORT_NAME    "gReorderOffloadSupported"
#define CFG_REORDER_OFFLOAD_SUPPORT_MIN     (0)
#define CFG_REORDER_OFFLOAD_SUPPORT_MAX     (1)
#define CFG_REORDER_OFFLOAD_SUPPORT_DEFAULT (0)

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
#define CFG_ROAMING_OFFLOAD_NAME                "gRoamOffloadEnabled"
#define CFG_ROAMING_OFFLOAD_MIN                 (0)
#define CFG_ROAMING_OFFLOAD_MAX                 (1)
#define CFG_ROAMING_OFFLOAD_DEFAULT             (1)
#endif

#define CFG_IPA_UC_TX_BUF_COUNT_NAME               "IpaUcTxBufCount"
#define CFG_IPA_UC_TX_BUF_COUNT_MIN                (0)
#define CFG_IPA_UC_TX_BUF_COUNT_MAX                (2048)
#define CFG_IPA_UC_TX_BUF_COUNT_DEFAULT            (512)

#define CFG_IPA_UC_TX_BUF_SIZE_NAME                "IpaUcTxBufSize"
#define CFG_IPA_UC_TX_BUF_SIZE_MIN                (0)
#define CFG_IPA_UC_TX_BUF_SIZE_MAX                (4096)
#define CFG_IPA_UC_TX_BUF_SIZE_DEFAULT            (2048)

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

/* Enable FATAL and ERROR logs for kmsg console */
#define CFG_WLAN_LOGGING_FE_CONSOLE_SUPPORT_NAME    "wlanLoggingFEToConsole"
#define CFG_WLAN_LOGGING_FE_CONSOLE_SUPPORT_ENABLE  (1)
#define CFG_WLAN_LOGGING_FE_CONSOLE_SUPPORT_DISABLE (0)
#define CFG_WLAN_LOGGING_FE_CONSOLE_SUPPORT_DEFAULT (1)

/* Number of buffers to be used for WLAN logging */
#define CFG_WLAN_LOGGING_NUM_BUF_NAME               "wlanLoggingNumBuf"
#define CFG_WLAN_LOGGING_NUM_BUF_MIN                (4)
#define CFG_WLAN_LOGGING_NUM_BUF_MAX                (512)
#define CFG_WLAN_LOGGING_NUM_BUF_DEFAULT            (256)
#endif /* WLAN_LOGGING_SOCK_SVC_ENABLE */

#define CFG_ENABLE_SIFS_BURST                      "gEnableSifsBurst"
#define CFG_ENABLE_SIFS_BURST_MIN                  (0)
#define CFG_ENABLE_SIFS_BURST_MAX                  (1)
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

#ifdef WLAN_FEATURE_EXTWOW_SUPPORT
#define CFG_EXTWOW_GO_TO_SUSPEND                   "gExtWoWgotoSuspend"
#define CFG_EXTWOW_GO_TO_SUSPEND_MIN               (0)
#define CFG_EXTWOW_GO_TO_SUSPEND_MAX               (1)
#define CFG_EXTWOW_GO_TO_SUSPEND_DEFAULT           (1)

#define CFG_EXTWOW_APP1_WAKE_PIN_NUMBER            "gExtWowApp1WakeupPinNumber"
#define CFG_EXTWOW_APP1_WAKE_PIN_NUMBER_MIN        (0)
#define CFG_EXTWOW_APP1_WAKE_PIN_NUMBER_MAX        (255)
#define CFG_EXTWOW_APP1_WAKE_PIN_NUMBER_DEFAULT    (12)

#define CFG_EXTWOW_APP2_WAKE_PIN_NUMBER            "gExtWowApp2WakeupPinNumber"
#define CFG_EXTWOW_APP2_WAKE_PIN_NUMBER_MIN        (0)
#define CFG_EXTWOW_APP2_WAKE_PIN_NUMBER_MAX        (255)
#define CFG_EXTWOW_APP2_WAKE_PIN_NUMBER_DEFAULT    (16)

#define CFG_EXTWOW_KA_INIT_PING_INTERVAL           "gExtWoWApp2KAInitPingInterval"
#define CFG_EXTWOW_KA_INIT_PING_INTERVAL_MIN       (0)
#define CFG_EXTWOW_KA_INIT_PING_INTERVAL_MAX       (0xffffffff)
#define CFG_EXTWOW_KA_INIT_PING_INTERVAL_DEFAULT   (240)

#define CFG_EXTWOW_KA_MIN_PING_INTERVAL            "gExtWoWApp2KAMinPingInterval"
#define CFG_EXTWOW_KA_MIN_PING_INTERVAL_MIN        (0)
#define CFG_EXTWOW_KA_MIN_PING_INTERVAL_MAX        (0xffffffff)
#define CFG_EXTWOW_KA_MIN_PING_INTERVAL_DEFAULT    (240)

#define CFG_EXTWOW_KA_MAX_PING_INTERVAL            "gExtWoWApp2KAMaxPingInterval"
#define CFG_EXTWOW_KA_MAX_PING_INTERVAL_MIN        (0)
#define CFG_EXTWOW_KA_MAX_PING_INTERVAL_MAX        (0xffffffff)
#define CFG_EXTWOW_KA_MAX_PING_INTERVAL_DEFAULT    (1280)

#define CFG_EXTWOW_KA_INC_PING_INTERVAL            "gExtWoWApp2KAIncPingInterval"
#define CFG_EXTWOW_KA_INC_PING_INTERVAL_MIN        (0)
#define CFG_EXTWOW_KA_INC_PING_INTERVAL_MAX        (0xffffffff)
#define CFG_EXTWOW_KA_INC_PING_INTERVAL_DEFAULT    (4)

#define CFG_EXTWOW_TCP_SRC_PORT                    "gExtWoWApp2TcpSrcPort"
#define CFG_EXTWOW_TCP_SRC_PORT_MIN                (0)
#define CFG_EXTWOW_TCP_SRC_PORT_MAX                (65535)
#define CFG_EXTWOW_TCP_SRC_PORT_DEFAULT            (5000)

#define CFG_EXTWOW_TCP_DST_PORT                    "gExtWoWApp2TcpDstPort"
#define CFG_EXTWOW_TCP_DST_PORT_MIN                (0)
#define CFG_EXTWOW_TCP_DST_PORT_MAX                (65535)
#define CFG_EXTWOW_TCP_DST_PORT_DEFAULT            (5001)

#define CFG_EXTWOW_TCP_TX_TIMEOUT                  "gExtWoWApp2TcpTxTimeout"
#define CFG_EXTWOW_TCP_TX_TIMEOUT_MIN              (0)
#define CFG_EXTWOW_TCP_TX_TIMEOUT_MAX              (0xffffffff)
#define CFG_EXTWOW_TCP_TX_TIMEOUT_DEFAULT          (200)

#define CFG_EXTWOW_TCP_RX_TIMEOUT                  "gExtWoWApp2TcpRxTimeout"
#define CFG_EXTWOW_TCP_RX_TIMEOUT_MIN              (0)
#define CFG_EXTWOW_TCP_RX_TIMEOUT_MAX              (0xffffffff)
#define CFG_EXTWOW_TCP_RX_TIMEOUT_DEFAULT          (200)
#endif

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
 * If last disconnection was due to HB failure and we reconnect
 * to same AP next time, send Deauth before starting connection
 */
#define CFG_ENABLE_DEAUTH_BEFORE_CONNECTION                  "gSendDeauthBeforeCon"
#define CFG_ENABLE_DEAUTH_BEFORE_CONNECTION_MIN              (0)
#define CFG_ENABLE_DEAUTH_BEFORE_CONNECTION_MAX              (1)
#define CFG_ENABLE_DEAUTH_BEFORE_CONNECTION_DEFAULT          (0)

/*
 * Custom concurrency rule1:
 * If SAP comes up first and STA comes up later then SAP
 * needs to follow STA's channel.
 */
#define CFG_ENABLE_CUSTOM_CONC_RULE1_NAME         "gEnableCustomConcRule1"
#define CFG_ENABLE_CUSTOM_CONC_RULE1_NAME_MIN     (0)
#define CFG_ENABLE_CUSTOM_CONC_RULE1_NAME_MAX     (1)
#define CFG_ENABLE_CUSTOM_CONC_RULE1_NAME_DEFAULT (0)

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

#define CFG_P2P_LISTEN_DEFER_INTERVAL_NAME        "gP2PListenDeferInterval"
#define CFG_P2P_LISTEN_DEFER_INTERVAL_MIN         (100)
#define CFG_P2P_LISTEN_DEFER_INTERVAL_MAX         (200)
#define CFG_P2P_LISTEN_DEFER_INTERVAL_DEFAULT     (100)

#define CFG_STA_MIRACAST_MCC_REST_TIME_VAL          "gStaMiracastMccRestTimeVal"
#define CFG_STA_MIRACAST_MCC_REST_TIME_VAL_MIN     (100)
#define CFG_STA_MIRACAST_MCC_REST_TIME_VAL_MAX     (500)
#define CFG_STA_MIRACAST_MCC_REST_TIME_VAL_DEFAULT (400)

#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
#define CFG_SAP_MCC_CHANNEL_AVOIDANCE_NAME         "gSapChannelAvoidance"
#define CFG_SAP_MCC_CHANNEL_AVOIDANCE_MIN          (0)
#define CFG_SAP_MCC_CHANNEL_AVOIDANCE_MAX          (1)
#define CFG_SAP_MCC_CHANNEL_AVOIDANCE_DEFAULT      (0)
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */

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
#define CFG_MULTICAST_HOST_FW_MSGS_DEFAULT  (1)

/*
 * wlan system preference option to help policy manager decide
 * on Preferred Channel List for a new connection. For possible
 * values refer to enum hdd_conc_priority_mode
 */
#define CFG_CONC_SYSTEM_PREF               "gSystemPref"
#define CFG_CONC_SYSTEM_PREF_MIN           (0)
#define CFG_CONC_SYSTEM_PREF_MAX           (2)
#define CFG_CONC_SYSTEM_PREF_DEFAULT       (0)

#define CFG_TSO_ENABLED_NAME           "TSOEnable"
#define CFG_TSO_ENABLED_MIN            (0)
#define CFG_TSO_ENABLED_MAX            (1)
#define CFG_TSO_ENABLED_DEFAULT        (0)

/*
 * Configuration option to enable or disable LRO (Large Receive Offload)
 *  in the WLAN driver
 *  Set 1 - enable, 0 - disable
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
 * Active mode offload feature is bydefault enabled for adrastea and disabled
 * for non adrastea targets like ROME
 */

#define CFG_ACTIVE_MODE_OFFLOAD            "gActiveModeOffload"
#define CFG_ACTIVE_MODE_OFFLOAD_MIN        (0)
#define CFG_ACTIVE_MODE_OFFLOAD_MAX        (1)
#ifdef QCA_WIFI_3_0_ADRASTEA
#define CFG_ACTIVE_MODE_OFFLOAD_DEFAULT    (1)
#else
#define CFG_ACTIVE_MODE_OFFLOAD_DEFAULT    (0)
#endif

/*
 * 0: Disable BPF packet filter
 * 1: Enable BPF packet filter
 */
#define CFG_BPF_PACKET_FILTER_OFFLOAD           "gBpfFilterEnable"
#define CFG_BPF_PACKET_FILTER_OFFLOAD_MIN       (0)
#define CFG_BPF_PACKET_FILTER_OFFLOAD_MAX       (1)
#define CFG_BPF_PACKET_FILTER_OFFLOAD_DEFAULT   (1)

/*
 * 0: disable the cck tx chain mask (default)
 * 1: enable the cck tx chain mask
 */
#define CFG_TX_CHAIN_MASK_CCK          "gCckChainMaskEnable"
#define CFG_TX_CHAIN_MASK_CCK_MIN      (0)
#define CFG_TX_CHAIN_MASK_CCK_MAX      (1)
#define CFG_TX_CHAIN_MASK_CCK_DEFAULT  (0)

#define CFG_TX_CHAIN_MASK_1SS       "gTxChainMask1ss"
#define CFG_TX_CHAIN_MASK_1SS_MIN      (0)
#define CFG_TX_CHAIN_MASK_1SS_MAX      (3)
#define CFG_TX_CHAIN_MASK_1SS_DEFAULT  (1)

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
 *|   9-31   |  8  |  7  |   5  |   4  |   3   |   2   |  1  |  0  |
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
 * Maximum number of scans that host can queue at firmware is controlled
 * through the configuration item  'max_scan_count'.
 *
 * Rome - Rome firmware support 8 scan queue size and 4 are reserved
 * for internal scan requests like roaming. So host can send 4
 * scan requests.
 *
 * ihelium - There is no constraint in number of scan queue size at
 * firmware but the current uses cases needs support of maximum
 * of 4 scan request from host.
 */
#define CFG_MAX_SCAN_COUNT_NAME           "max_scan_count"
#define CFG_MAX_SCAN_COUNT_MIN            (1)
#define CFG_MAX_SCAN_COUNT_MAX            (8)
#define CFG_MAX_SCAN_COUNT_DEFAULT        (4)

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

#ifdef FEATURE_WLAN_EXTSCAN

/*
 * This ini is added to control the enabling of extscan feature outside of code.
 * By default extscan feature will be enabled, to disable extscan add
 * gExtScanEnable=0 in ini file.
 */
#define CFG_EXTSCAN_ALLOWED_NAME                   "gExtScanEnable"
#define CFG_EXTSCAN_ALLOWED_MIN                    (0)
#define CFG_EXTSCAN_ALLOWED_MAX                    (1)
#define CFG_EXTSCAN_ALLOWED_DEF                    (1)

#define CFG_EXTSCAN_PASSIVE_MAX_CHANNEL_TIME_NAME      "gExtScanPassiveMaxChannelTime"
#define CFG_EXTSCAN_PASSIVE_MAX_CHANNEL_TIME_MIN       (0)
#define CFG_EXTSCAN_PASSIVE_MAX_CHANNEL_TIME_MAX       (500)
#define CFG_EXTSCAN_PASSIVE_MAX_CHANNEL_TIME_DEFAULT   (110)

#define CFG_EXTSCAN_PASSIVE_MIN_CHANNEL_TIME_NAME      "gExtScanPassiveMinChannelTime"
#define CFG_EXTSCAN_PASSIVE_MIN_CHANNEL_TIME_MIN       (0)
#define CFG_EXTSCAN_PASSIVE_MIN_CHANNEL_TIME_MAX       (500)
#define CFG_EXTSCAN_PASSIVE_MIN_CHANNEL_TIME_DEFAULT   (60)

#define CFG_EXTSCAN_ACTIVE_MAX_CHANNEL_TIME_NAME       "gExtScanActiveMaxChannelTime"
#define CFG_EXTSCAN_ACTIVE_MAX_CHANNEL_TIME_MIN        (0)
#define CFG_EXTSCAN_ACTIVE_MAX_CHANNEL_TIME_MAX        (110)
#define CFG_EXTSCAN_ACTIVE_MAX_CHANNEL_TIME_DEFAULT    (40)

#define CFG_EXTSCAN_ACTIVE_MIN_CHANNEL_TIME_NAME       "gExtScanActiveMinChannelTime"
#define CFG_EXTSCAN_ACTIVE_MIN_CHANNEL_TIME_MIN        (0)
#define CFG_EXTSCAN_ACTIVE_MIN_CHANNEL_TIME_MAX        (110)
#define CFG_EXTSCAN_ACTIVE_MIN_CHANNEL_TIME_DEFAULT    (20)
#endif

#define CFG_CE_CLASSIFY_ENABLE_NAME	"gCEClassifyEnable"
#define CFG_CE_CLASSIFY_ENABLE_MIN	(0)
#define CFG_CE_CLASSIFY_ENABLE_MAX	(1)
#define CFG_CE_CLASSIFY_ENABLE_DEFAULT	(1)

#define CFG_DUAL_MAC_FEATURE_DISABLE               "gDualMacFeatureDisable"
#define CFG_DUAL_MAC_FEATURE_DISABLE_MIN          (0)
#define CFG_DUAL_MAC_FEATURE_DISABLE_MAX          (1)
#define CFG_DUAL_MAC_FEATURE_DISABLE_DEFAULT      (0)

/* Parameters for roaming scans performed at high RSSI */

/* Maximum number of scans after RSSI change */
#define CFG_ROAM_SCAN_HI_RSSI_MAXCOUNT_NAME         "gRoamScanHiRssiMaxCount"
#define CFG_ROAM_SCAN_HI_RSSI_MAXCOUNT_MIN          (0)
#define CFG_ROAM_SCAN_HI_RSSI_MAXCOUNT_MAX          (10)
#define CFG_ROAM_SCAN_HI_RSSI_MAXCOUNT_DEFAULT      (3)

/* Change in RSSI at which scan is triggered */
#define CFG_ROAM_SCAN_HI_RSSI_DELTA_NAME           "gRoamScanHiRssiDelta"
#define CFG_ROAM_SCAN_HI_RSSI_DELTA_MIN            (0)
#define CFG_ROAM_SCAN_HI_RSSI_DELTA_MAX            (16)
#define CFG_ROAM_SCAN_HI_RSSI_DELTA_DEFAULT        (10)

/* Delay between consecutive scans in milliseconds */
#define CFG_ROAM_SCAN_HI_RSSI_DELAY_NAME            "gRoamScanHiRssiDelay"
#define CFG_ROAM_SCAN_HI_RSSI_DELAY_MIN             (5000)
#define CFG_ROAM_SCAN_HI_RSSI_DELAY_MAX             (0x7fffffff)
#define CFG_ROAM_SCAN_HI_RSSI_DELAY_DEFAULT         (15000)

/* Upper bound after which scan will not be performed */
#define CFG_ROAM_SCAN_HI_RSSI_UB_NAME              "gRoamScanHiRssiUpperBound"
#define CFG_ROAM_SCAN_HI_RSSI_UB_MIN               (-66)
#define CFG_ROAM_SCAN_HI_RSSI_UB_MAX               (0)
#define CFG_ROAM_SCAN_HI_RSSI_UB_DEFAULT           (-30)
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
/*
 * The following parameter is the periodic timer upon which
 * a full scan needs to be triggered when PNO channel prediction
 * feature is enabled. This parameter is intended to tweak the
 * internal algortihm for experiments. This should not be changed
 * externally.
 */
#define CFG_CHANNEL_PREDICTION_FULL_SCAN_MS_NAME      "gChPredictionFullScanMs"
#define CFG_CHANNEL_PREDICTION_FULL_SCAN_MS_MIN       (30000)
#define CFG_CHANNEL_PREDICTION_FULL_SCAN_MS_MAX       (0x7fffffff)
#define CFG_CHANNEL_PREDICTION_FULL_SCAN_MS_DEFAULT   (60000)

/*
 * Early stop scan is a feature for roaming to stop the scans at
 * an early stage as soon as we find a better AP to roam. This
 * would make the roaming happen quickly.
 */
#define CFG_EARLY_STOP_SCAN_ENABLE           "gEnableEarlyStopScan"
#define CFG_EARLY_STOP_SCAN_ENABLE_MIN       (0)
#define CFG_EARLY_STOP_SCAN_ENABLE_MAX       (1)
#define CFG_EARLY_STOP_SCAN_ENABLE_DEFAULT   (1)
/*
 * Early stop scan minimum threshold is the minimum threshold
 * to be considered for stopping the scan. The algorithm starts
 * with a scan on the greedy channel list with the maximum threshold
 * and steps down the threshold by 20% for each further channel.
 * It can step down on each channel but cannot go lower than the minimum
 * threshold.
 */
#define CFG_EARLY_STOP_SCAN_MIN_THRESHOLD           "gEarlyStopScanMinThreshold"
#define CFG_EARLY_STOP_SCAN_MIN_THRESHOLD_MIN       (-80)
#define CFG_EARLY_STOP_SCAN_MIN_THRESHOLD_MAX       (-70)
#define CFG_EARLY_STOP_SCAN_MIN_THRESHOLD_DEFAULT   (-73)
/*
 * Early stop scan maximum threshold is the maximum threshold
 * at which the candidate AP should be to be qualified as a
 * potential roam candidate and good enough to stop the roaming scan
 */
#define CFG_EARLY_STOP_SCAN_MAX_THRESHOLD           "gEarlyStopScanMaxThreshold"
#define CFG_EARLY_STOP_SCAN_MAX_THRESHOLD_MIN       (-60)
#define CFG_EARLY_STOP_SCAN_MAX_THRESHOLD_MAX       (-40)
#define CFG_EARLY_STOP_SCAN_MAX_THRESHOLD_DEFAULT   (-43)

/*
 * This parameter will configure the first scan bucket
 * threshold to the mentioned value and all the AP's which
 * have RSSI under this threshold will fall under this
 * bucket.
 * This is a configuration item used to tweak and test the input
 * for internal algorithm. It should not be modified externally.
 */
#define CFG_FIRST_SCAN_BUCKET_THRESHOLD_NAME      "gFirstScanBucketThreshold"
#define CFG_FIRST_SCAN_BUCKET_THRESHOLD_MIN       (-50)
#define CFG_FIRST_SCAN_BUCKET_THRESHOLD_MAX       (-30)
#define CFG_FIRST_SCAN_BUCKET_THRESHOLD_DEFAULT   (-30)

#ifdef FEATURE_LFR_SUBNET_DETECTION
/*
 * Enable IP subnet detection during legacy fast roming version 3.
 * Legacy fast roaming could roam across IP subnets without host
 * processors' knowledge. This feature enables firmware to wake up
 * the host processor if it successfully determines change in the IP subnet.
 * Change in IP subnet could potentially cause disruption in IP connnectivity
 * if IP address is not refreshed.
 */
#define CFG_ENABLE_LFR_SUBNET_DETECTION    "gLFRSubnetDetectionEnable"
#define CFG_ENABLE_LFR_SUBNET_MIN          (0)
#define CFG_ENABLE_LFR_SUBNET_MAX          (1)
#define CFG_ENABLE_LFR_SUBNET_DEFAULT      (1)
#endif /* FEATURE_LFR_SUBNET_DETECTION */
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

/*
 * OBSS scan parameters
 * obss_active_dwelltime - minimum per channel scan duration in active scan
 * obss_passive_dwelltime - minimum per channel scan duration in passive scan
 * obss_width_trigger_interval - During an OBSS scan operation, each channel
 *  in the set is scanned at least once per configured trigger interval time.
 */
#define CFG_OBSS_HT40_SCAN_ACTIVE_DWELL_TIME_NAME    "obss_active_dwelltime"
#define CFG_OBSS_HT40_SCAN_ACTIVE_DWELL_TIME_MIN     (5)
#define CFG_OBSS_HT40_SCAN_ACTIVE_DWELL_TIME_MAX     (1000)
#define CFG_OBSS_HT40_SCAN_ACTIVE_DWELL_TIME_DEFAULT (10)

#define CFG_OBSS_HT40_SCAN_PASSIVE_DWELL_TIME_NAME   "obss_passive_dwelltime"
#define CFG_OBSS_HT40_SCAN_PASSIVE_DWELL_TIME_MIN    (10)
#define CFG_OBSS_HT40_SCAN_PASSIVE_DWELL_TIME_MAX    (1000)
#define CFG_OBSS_HT40_SCAN_PASSIVE_DWELL_TIME_DEFAULT (20)

#define CFG_OBSS_HT40_SCAN_WIDTH_TRIGGER_INTERVAL_NAME "obss_width_trigger_interval"
#define CFG_OBSS_HT40_SCAN_WIDTH_TRIGGER_INTERVAL_MIN  (10)
#define CFG_OBSS_HT40_SCAN_WIDTH_TRIGGER_INTERVAL_MAX  (900)
#define CFG_OBSS_HT40_SCAN_WIDTH_TRIGGER_INTERVAL_DEFAULT (200)

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
 * Dense Roam RSSI Threshold diff
 * offset value from normal RSSI threshold to dense RSSI threshold
 * Fw will optimize roaming based on new RSSI threshold once it detects
 * dense enviournment.
 */
#define CFG_ROAM_DENSE_RSSI_THRE_OFFSET         "groam_dense_rssi_thresh_offset"
#define CFG_ROAM_DENSE_RSSI_THRE_OFFSET_MIN     (0)
#define CFG_ROAM_DENSE_RSSI_THRE_OFFSET_MAX     (20)
#define CFG_ROAM_DENSE_RSSI_THRE_OFFSET_DEFAULT (10)

/*
 * Create bug report in case of nil scan results
 */
#define CFG_CREATE_BUG_REPORT_FOR_SCAN       "gbug_report_for_scan_results"
#define CFG_CREATE_BUG_REPORT_FOR_SCAN_DISABLE    (0)
#define CFG_CREATE_BUG_REPORT_FOR_SCAN_ENABLE     (1)
#define CFG_CREATE_BUG_REPORT_FOR_SCAN_DEFAULT    (0)

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
 * Dense Roam Min APs
 * minimum number of AP required for dense roam
 * FW will consider environment as dense once it detects #APs
 * operating is more than CFG_ROAM_DENSE_MIN_APS.
 */
#define CFG_ROAM_DENSE_MIN_APS         "groam_dense_min_aps"
#define CFG_ROAM_DENSE_MIN_APS_MIN     (1)
#define CFG_ROAM_DENSE_MIN_APS_MAX     (5)
#define CFG_ROAM_DENSE_MIN_APS_DEFAULT (3)

/*
 * Enable/Disable to initiate BUG report in case of fatal event
 * Default: Enable
 */
#define CFG_ENABLE_FATAL_EVENT_TRIGGER                 "gEnableFatalEvent"
#define CFG_ENABLE_FATAL_EVENT_TRIGGER_MIN             (0)
#define CFG_ENABLE_FATAL_EVENT_TRIGGER_MAX             (1)
#define CFG_ENABLE_FATAL_EVENT_TRIGGER_DEFAULT         (1)

/*
 * If gEnableEdcaParams is set to 1, params gEdcaVoCwmin,
 * gEdcaViCwmin, gEdcaBkCwmin, gEdcaBeCwmin, gEdcaVoCwmax,
 * gEdcaViCwmax, gEdcaBkCwmax, gEdcaBeCwmax, gEdcaVoAifs,
 * gEdcaViAifs, gEdcaBkAifs and gEdcaBeAifs values are used
 * to overwrite the values received from AP
 */
#define CFG_ENABLE_EDCA_INI_NAME       "gEnableEdcaParams"
#define CFG_ENABLE_EDCA_INI_MIN        (0)
#define CFG_ENABLE_EDCA_INI_MAX        (1)
#define CFG_ENABLE_EDCA_INI_DEFAULT    (0)

/* Cwmin value for EDCA_AC_VO. CWVomin = 2^gEdcaVoCwmin -1 */
#define CFG_EDCA_VO_CWMIN_VALUE_NAME      "gEdcaVoCwmin"
#define CFG_EDCA_VO_CWMIN_VALUE_MIN       (0x0)
#define CFG_EDCA_VO_CWMIN_VALUE_MAX       (15)
#define CFG_EDCA_VO_CWMIN_VALUE_DEFAULT   (2)

/* Cwmin value for EDCA_AC_VI. CWVimin = 2^gEdcaViCwmin -1 */
#define CFG_EDCA_VI_CWMIN_VALUE_NAME      "gEdcaViCwmin"
#define CFG_EDCA_VI_CWMIN_VALUE_MIN       (0x0)
#define CFG_EDCA_VI_CWMIN_VALUE_MAX       (15)
#define CFG_EDCA_VI_CWMIN_VALUE_DEFAULT   (3)

/* Cwmin value for EDCA_AC_BK. CWBkmin = 2^gEdcaBkCwmin -1 */
#define CFG_EDCA_BK_CWMIN_VALUE_NAME      "gEdcaBkCwmin"
#define CFG_EDCA_BK_CWMIN_VALUE_MIN       (0x0)
#define CFG_EDCA_BK_CWMIN_VALUE_MAX       (15)
#define CFG_EDCA_BK_CWMIN_VALUE_DEFAULT   (4)

/* Cwmin value for EDCA_AC_BE. CWBemin = 2^gEdcaBeCwmin -1 */
#define CFG_EDCA_BE_CWMIN_VALUE_NAME      "gEdcaBeCwmin"
#define CFG_EDCA_BE_CWMIN_VALUE_MIN       (0x0)
#define CFG_EDCA_BE_CWMIN_VALUE_MAX       (15)
#define CFG_EDCA_BE_CWMIN_VALUE_DEFAULT   (4)

/* Cwmax value for EDCA_AC_VO. CWVomax = 2^gEdcaVoCwmax -1 */
#define CFG_EDCA_VO_CWMAX_VALUE_NAME      "gEdcaVoCwmax"
#define CFG_EDCA_VO_CWMAX_VALUE_MIN       (0)
#define CFG_EDCA_VO_CWMAX_VALUE_MAX       (15)
#define CFG_EDCA_VO_CWMAX_VALUE_DEFAULT   (3)

/* Cwmax value for EDCA_AC_VI. CWVimax = 2^gEdcaViCwmax -1 */
#define CFG_EDCA_VI_CWMAX_VALUE_NAME      "gEdcaViCwmax"
#define CFG_EDCA_VI_CWMAX_VALUE_MIN       (0)
#define CFG_EDCA_VI_CWMAX_VALUE_MAX       (15)
#define CFG_EDCA_VI_CWMAX_VALUE_DEFAULT   (4)

/* Cwmax value for EDCA_AC_BK. CWBkmax = 2^gEdcaBkCwmax -1 */
#define CFG_EDCA_BK_CWMAX_VALUE_NAME      "gEdcaBkCwmax"
#define CFG_EDCA_BK_CWMAX_VALUE_MIN       (0)
#define CFG_EDCA_BK_CWMAX_VALUE_MAX       (15)
#define CFG_EDCA_BK_CWMAX_VALUE_DEFAULT   (10)

/* Cwmax value for EDCA_AC_BE. CWBemax = 2^gEdcaBeCwmax -1 */
#define CFG_EDCA_BE_CWMAX_VALUE_NAME      "gEdcaBeCwmax"
#define CFG_EDCA_BE_CWMAX_VALUE_MIN       (0)
#define CFG_EDCA_BE_CWMAX_VALUE_MAX       (15)
#define CFG_EDCA_BE_CWMAX_VALUE_DEFAULT   (10)

/* Aifs value for EDCA_AC_VO.*/
#define CFG_EDCA_VO_AIFS_VALUE_NAME       "gEdcaVoAifs"
#define CFG_EDCA_VO_AIFS_VALUE_MIN        (0)
#define CFG_EDCA_VO_AIFS_VALUE_MAX        (15)
#define CFG_EDCA_VO_AIFS_VALUE_DEFAULT    (2)

/* Aifs value for EDCA_AC_VI.*/
#define CFG_EDCA_VI_AIFS_VALUE_NAME       "gEdcaViAifs"
#define CFG_EDCA_VI_AIFS_VALUE_MIN        (0)
#define CFG_EDCA_VI_AIFS_VALUE_MAX        (15)
#define CFG_EDCA_VI_AIFS_VALUE_DEFAULT    (2)

/* Aifs value for EDCA_AC_BK.*/
#define CFG_EDCA_BK_AIFS_VALUE_NAME       "gEdcaBkAifs"
#define CFG_EDCA_BK_AIFS_VALUE_MIN        (0)
#define CFG_EDCA_BK_AIFS_VALUE_MAX        (15)
#define CFG_EDCA_BK_AIFS_VALUE_DEFAULT    (7)

/* Aifs value for EDCA_AC_BE.*/
#define CFG_EDCA_BE_AIFS_VALUE_NAME       "gEdcaBeAifs"
#define CFG_EDCA_BE_AIFS_VALUE_MIN        (0)
#define CFG_EDCA_BE_AIFS_VALUE_MAX        (15)
#define CFG_EDCA_BE_AIFS_VALUE_DEFAULT    (3)

/*
 * Enable/disable DPTRACE
 * Enabling this might have performace impact.
 */
#define CFG_ENABLE_DP_TRACE		"enable_dp_trace"
#define CFG_ENABLE_DP_TRACE_MIN		(0)
#define CFG_ENABLE_DP_TRACE_MAX		(1)
#define CFG_ENABLE_DP_TRACE_DEFAULT	(1)

/*
 * This parameter will globally disable/enable the adaptive dwell config.
 * Scan dwell time optimization
 * Following parameters will set different values of attributes for dwell
 * time optimization thus reducing total scan time.
 */

/*
 * This parameter will globally disable/enable the adaptive dwell config.
 * Acceptable values for this:
 * 0: Config is disabled
 * 1: Config is enabled
 */
#define CFG_ADAPTIVE_DWELL_MODE_ENABLED_NAME      "adaptive_dwell_mode_enabled"
#define CFG_ADAPTIVE_DWELL_MODE_ENABLED_MIN       (0)
#define CFG_ADAPTIVE_DWELL_MODE_ENABLED_MAX       (1)
#define CFG_ADAPTIVE_DWELL_MODE_ENABLED_DEFAULT   (0)

/*
 * This parameter will set default adaptive mode, will be used if any of the
 * scan dwell mode is set to default.
 * For uses : see enum wmi_dwelltime_adaptive_mode
 */
#define CFG_GLOBAL_ADAPTIVE_DWELL_MODE_NAME       "global_adapt_dwelltime_mode"
#define CFG_GLOBAL_ADAPTIVE_DWELL_MODE_MIN        (0)
#define CFG_GLOBAL_ADAPTIVE_DWELL_MODE_MAX        (4)
#define CFG_GLOBAL_ADAPTIVE_DWELL_MODE_DEFAULT    (0)

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
#define CFG_ADAPT_DWELL_PASMON_INTVAL_MAX      (10)
#define CFG_ADAPT_DWELL_PASMON_INTVAL_DEFAULT  (25)

/*
 * This parameter will set % of wifi activity used in passive scan 0-100.
 * Acceptable values for this: 0-100 (in %)
 */
#define CFG_ADAPT_DWELL_WIFI_THRESH_NAME       "adapt_dwell_wifi_act_threshold"
#define CFG_ADAPT_DWELL_WIFI_THRESH_MIN        (0)
#define CFG_ADAPT_DWELL_WIFI_THRESH_MAX        (100)
#define CFG_ADAPT_DWELL_WIFI_THRESH_DEFAULT    (10)

/*
 * This parameter will set the algo used in dwell time optimization during
 * host scan. see enum wmi_dwelltime_adaptive_mode.
 * Acceptable values for this:
 * 0: Default (Use firmware default mode)
 * 1: Conservative optimization
 * 2: Moderate optimization
 * 3: Aggressive optimization
 * 4: Static
 */
#define CFG_ADAPTIVE_SCAN_DWELL_MODE_NAME        "hostscan_adaptive_dwell_mode"
#define CFG_ADAPTIVE_SCAN_DWELL_MODE_MIN         (0)
#define CFG_ADAPTIVE_SCAN_DWELL_MODE_MAX         (4)
#define CFG_ADAPTIVE_SCAN_DWELL_MODE_DEFAULT     (0)

/*
 * This parameter will set the algo used in dwell time optimization during
 * roam scan. see enum wmi_dwelltime_adaptive_mode.
 * Acceptable values for this:
 * 0: Default (Use firmware default mode)
 * 1: Conservative optimization
 * 2: Moderate optimization
 * 3: Aggressive optimization
 * 4: Static
 */
#define CFG_ADAPTIVE_ROAMSCAN_DWELL_MODE_NAME    "roamscan_adaptive_dwell_mode"
#define CFG_ADAPTIVE_ROAMSCAN_DWELL_MODE_MIN     (0)
#define CFG_ADAPTIVE_ROAMSCAN_DWELL_MODE_MAX     (4)
#define CFG_ADAPTIVE_ROAMSCAN_DWELL_MODE_DEFAULT (0)

/*
 * This parameter will set the algo used in dwell time optimization during
 * ext scan. see enum wmi_dwelltime_adaptive_mode.
 * Acceptable values for this:
 * 0: Default (Use firmware default mode)
 * 1: Conservative optimization
 * 2: Moderate optimization
 * 3: Aggressive optimization
 * 4: Static
 */
#define CFG_ADAPTIVE_EXTSCAN_DWELL_MODE_NAME     "extscan_adaptive_dwell_mode"
#define CFG_ADAPTIVE_EXTSCAN_DWELL_MODE_MIN      (0)
#define CFG_ADAPTIVE_EXTSCAN_DWELL_MODE_MAX      (4)
#define CFG_ADAPTIVE_EXTSCAN_DWELL_MODE_DEFAULT  (0)

/*
 * This parameter will set the algo used in dwell time optimization during
 * pno scan. see enum wmi_dwelltime_adaptive_mode.
 * Acceptable values for this:
 * 0: Default (Use firmware default mode)
 * 1: Conservative optimization
 * 2: Moderate optimization
 * 3: Aggressive optimization
 * 4: Static
 */
#define CFG_ADAPTIVE_PNOSCAN_DWELL_MODE_NAME     "pnoscan_adaptive_dwell_mode"
#define CFG_ADAPTIVE_PNOSCAN_DWELL_MODE_MIN      (0)
#define CFG_ADAPTIVE_PNOSCAN_DWELL_MODE_MAX      (4)
#define CFG_ADAPTIVE_PNOSCAN_DWELL_MODE_DEFAULT  (0)

/*
 * gSub20ChannelWidth=0: indicates do not use Sub 20 MHz bandwidth
 * gSub20ChannelWidth=1: Bring up SAP/STA in 5 MHz bandwidth
 * gSub20ChannelWidth=2: Bring up SAP/STA in 10 MHz bandwidth
 */
#define CFG_SUB_20_CHANNEL_WIDTH_NAME              "gSub20ChannelWidth"
#define CFG_SUB_20_CHANNEL_WIDTH_MIN               (WLAN_SUB_20_CH_WIDTH_NONE)
#define CFG_SUB_20_CHANNEL_WIDTH_MAX               (WLAN_SUB_20_CH_WIDTH_10)
#define CFG_SUB_20_CHANNEL_WIDTH_DEFAULT           (WLAN_SUB_20_CH_WIDTH_NONE)

#define CFG_TGT_GTX_USR_CFG_NAME    "tgt_gtx_usr_cfg"
#define CFG_TGT_GTX_USR_CFG_MIN     (0)
#define CFG_TGT_GTX_USR_CFG_MAX     (32)
#define CFG_TGT_GTX_USR_CFG_DEFAULT (32)

/*
 * This parameter determines that which defered method will be use in rx path
 * If no bits are set then rx path processing will happen in tasklet context.
 * Bit 0: rx_thread enable
 * Bit 1: RPS enable
 * Bit 2: NAPI enable
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


/*---------------------------------------------------------------------------
   Type declarations
   -------------------------------------------------------------------------*/

struct hdd_config {
	/* Bitmap to track what is explicitly configured */
	DECLARE_BITMAP(bExplicitCfg, MAX_CFG_INI_ITEMS);

	/* Config parameters */
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
	uint32_t nBmpsModListenInterval;
	uint32_t nBmpsMaxListenInterval;
	uint32_t nBmpsMinListenInterval;
	eHddDot11Mode dot11Mode;
	uint32_t nChannelBondingMode24GHz;
	uint32_t nChannelBondingMode5GHz;
	uint32_t MaxRxAmpduFactor;
	uint16_t TxRate;
	uint32_t ShortGI20MhzEnable;
	uint32_t ScanResultAgeCount;
	uint32_t nScanAgeTimeNCNPS;
	uint32_t nScanAgeTimeNCPS;
	uint32_t nScanAgeTimeCNPS;
	uint32_t nScanAgeTimeCPS;
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
	uint32_t apLinkMonitorPeriod;
	uint32_t goLinkMonitorPeriod;
	uint32_t nBeaconInterval;
	uint8_t nTxPowerCap;    /* In dBm */
	bool fIsLowGainOverride;
	uint8_t disablePacketFilter;
	bool fRrmEnable;
	uint16_t nRrmRandnIntvl;
	/* length includes separator */
	char rm_capability[3 * DOT11F_IE_RRMENABLEDCAP_MAX_LEN];

	/* Vowifi 11r params */
	bool fFTResourceReqSupported;

	uint16_t nNeighborScanPeriod;
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
	uint32_t nRoamingTime;
	uint16_t nVccRssiTrigger;
	uint32_t nVccUlMacLossThreshold;

	uint32_t nPassiveMinChnTime;    /* in units of milliseconds */
	uint32_t nPassiveMaxChnTime;    /* in units of milliseconds */
	uint32_t nActiveMinChnTime;     /* in units of milliseconds */
	uint32_t nActiveMaxChnTime;     /* in units of milliseconds */

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
	uint8_t nNumStaChanCombinedConc;        /* number of channels combined for */
	/* STA in each split scan operation */
	uint8_t nNumP2PChanCombinedConc;        /* number of channels combined for */
	/* P2P in each split scan operation */
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

	/* WMM QoS Configuration */
	hdd_wmm_user_mode_t WmmMode;
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
#ifdef FEATURE_WLAN_ESE
	uint32_t InfraInactivityInterval;
	bool isEseIniFeatureEnabled;
#endif
	bool isFastTransitionEnabled;
	uint8_t RoamRssiDiff;
	bool isWESModeEnabled;
	bool isOkcIniFeatureEnabled;
	bool isRoamOffloadScanEnabled;
	bool bImplicitQosEnabled;

	/* default TSPEC parameters for AC_VO */
	sme_QosWmmDirType InfraDirAcVo;
	uint16_t InfraNomMsduSizeAcVo;
	uint32_t InfraMeanDataRateAcVo;
	uint32_t InfraMinPhyRateAcVo;
	uint16_t InfraSbaAcVo;

	/* default TSPEC parameters for AC_VI */
	sme_QosWmmDirType InfraDirAcVi;
	uint16_t InfraNomMsduSizeAcVi;
	uint32_t InfraMeanDataRateAcVi;
	uint32_t InfraMinPhyRateAcVi;
	uint16_t InfraSbaAcVi;

	/* default TSPEC parameters for AC_BE */
	sme_QosWmmDirType InfraDirAcBe;
	uint16_t InfraNomMsduSizeAcBe;
	uint32_t InfraMeanDataRateAcBe;
	uint32_t InfraMinPhyRateAcBe;
	uint16_t InfraSbaAcBe;

	/* default TSPEC parameters for AC_BK */
	sme_QosWmmDirType InfraDirAcBk;
	uint16_t InfraNomMsduSizeAcBk;
	uint32_t InfraMeanDataRateAcBk;
	uint32_t InfraMinPhyRateAcBk;
	uint16_t InfraSbaAcBk;

	uint32_t DelayedTriggerFrmInt;

	/* Wowl pattern */
	char wowlPattern[1024];

	/* Control for Replay counetr. value 1 means
	   single replay counter for all TID */
	bool bSingleTidRc;
	uint8_t mcastBcastFilterSetting;
	bool fhostArpOffload;
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
	uint16_t cfd_trace_enable_txrx;
	uint16_t qdf_trace_enable_htc;
	uint16_t qdf_trace_enable_hif;
	uint16_t qdf_trace_enable_hdd_sap_data;
	uint16_t qdf_trace_enable_hdd_data;
	uint16_t qdf_trace_enable_epping;
	uint16_t qdf_trace_enable_qdf_devices;

	uint16_t nTeleBcnTransListenInterval;
	uint16_t nTeleBcnMaxListenInterval;
	uint16_t nTeleBcnTransLiNumIdleBeacons;
	uint16_t nTeleBcnMaxLiNumIdleBeacons;
	uint8_t enableBypass11d;
	uint8_t enableDFSChnlScan;
	uint8_t enable_dfs_pno_chnl_scan;
	uint8_t enableDynamicDTIM;
	uint8_t ShortGI40MhzEnable;
	eHddLinkSpeedReportType reportMaxLinkSpeed;
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
	bool enable_txbf_sap_mode;
	uint8_t txBFCsnValue;
	bool enable_su_tx_bformer;
	uint8_t vhtRxMCS2x2;
	uint8_t vhtTxMCS2x2;
	bool enable2x2;
	uint32_t vdev_type_nss_2g;
	uint32_t vdev_type_nss_5g;
	bool txchainmask1x1;
	bool rxchainmask1x1;
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
	bool enableRxLDPC;
	bool enable5gEBT;
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
	bool enableTxLdpc;
	uint8_t disableLDPCWithTxbfAP;
	uint8_t enableMCCAdaptiveScheduler;
	bool sapAllowAllChannel;
	uint8_t retryLimitZero;
	uint8_t retryLimitOne;
	uint8_t retryLimitTwo;
	bool enableSSR;
	uint32_t cfgMaxMediumTime;
	bool enableVhtFor24GHzBand;
	/* Flag indicating whether legacy fast roam during concurrency is enabled in cfg.ini or not */
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
	uint8_t isAmsduSupportInAMPDU;
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
	bool enablePowersaveOffload;
	bool enablefwprint;
	bool enable_fw_log;
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
	uint32_t tcpDelackThresholdHigh;
	uint32_t tcpDelackThresholdLow;
	uint32_t tcp_tx_high_tput_thres;
	uint32_t tcp_delack_timer_count;
#endif /* MSM_PLATFORM */

	/* FW debug log parameters */
	uint32_t enableFwLogType;
	uint32_t enableFwLogLevel;
	uint8_t enableFwModuleLogLevel[FW_MODULE_LOG_LEVEL_STRING_LENGTH];

#ifdef WLAN_FEATURE_11W
	uint32_t pmfSaQueryMaxRetries;
	uint32_t pmfSaQueryRetryInterval;
#endif

	uint8_t gMaxConcurrentActiveSessions;

	uint8_t ignoreCAC;
	bool IsSapDfsChSifsBurstEnabled;

#ifdef FEATURE_GREEN_AP
	bool enableGreenAP;
	bool enable_egap;
	uint32_t egap_feature_flag;
	uint32_t egap_inact_time;
	uint32_t egap_wait_time;
#endif
	uint8_t force_sap_acs;
	uint8_t force_sap_acs_st_ch;
	uint8_t force_sap_acs_end_ch;

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
	uint32_t wlanLoggingEnable;
	uint32_t wlanLoggingFEToConsole;
	uint32_t wlanLoggingNumBuf;
#endif /* WLAN_LOGGING_SOCK_SVC_ENABLE */

	bool enableSifsBurst;

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

	bool enableSapSuspend;

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
	bool sendDeauthBeforeCon;
	bool tso_enable;
	bool lro_enable;
	bool flow_steering_enable;
	bool active_mode_offload;
	bool bpf_packet_filter_enable;
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
	bool     tx_chain_mask_cck;
	uint8_t  tx_chain_mask_1ss;
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
#endif
	uint32_t roam_dense_traffic_thresh;
	uint32_t roam_dense_rssi_thresh_offset;
	bool ignore_peer_ht_opmode;
	uint32_t roam_dense_min_aps;
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
	bool enable_fatal_event;
	bool bpf_enabled;
	bool enable_dp_trace;
	bool adaptive_dwell_mode_enabled;
	enum wmi_dwelltime_adaptive_mode scan_adaptive_dwell_mode;
	enum wmi_dwelltime_adaptive_mode roamscan_adaptive_dwell_mode;
	enum wmi_dwelltime_adaptive_mode extscan_adaptive_dwell_mode;
	enum wmi_dwelltime_adaptive_mode pnoscan_adaptive_dwell_mode;
	enum wmi_dwelltime_adaptive_mode global_adapt_dwelltime_mode;
	uint8_t adapt_dwell_lpf_weight;
	uint8_t adapt_dwell_passive_mon_intval;
	uint8_t adapt_dwell_wifi_act_threshold;
	bool bug_report_for_no_scan_results;
#ifdef WLAN_FEATURE_NAN_DATAPATH
	bool enable_nan_datapath;
	uint8_t nan_datapath_ndi_channel;
#endif
	uint32_t iface_change_wait_time;
	/* parameter to control GTX */
	uint32_t tgt_gtx_usr_cfg;
	enum cfg_sub_20_channel_width enable_sub_20_channel_width;
	bool indoor_channel_support;
	bool multicast_replay_filter;
	/* parameter for indicating sifs burst duration to fw */
	uint8_t sifs_burst_duration;
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

typedef enum {
	WLAN_PARAM_Integer,
	WLAN_PARAM_SignedInteger,
	WLAN_PARAM_HexInteger,
	WLAN_PARAM_String,
	WLAN_PARAM_MacAddr,
} WLAN_PARAMETER_TYPE;

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

typedef struct tREG_TABLE_ENTRY {

	char *RegName;          /* variable name in the qcom_cfg.ini file */
	WLAN_PARAMETER_TYPE RegType;    /* variable type in hdd_config struct */
	unsigned long Flags;    /* Specify optional parms and if RangeCheck is performed */
	unsigned short VarOffset;       /* offset to field from the base address of the structure */
	unsigned short VarSize; /* size (in bytes) of the field */
	unsigned long VarDefault;       /* default value to use */
	unsigned long VarMin;   /* minimum value, for range checking */
	unsigned long VarMax;   /* maximum value, for range checking */
	/* Dynamic modification notifier */
	void (*pfnDynamicnotify)(hdd_context_t *pHddCtx,
				 unsigned long notifyId);
	unsigned long notifyId; /* Dynamic modification identifier */
} REG_TABLE_ENTRY;

static __inline unsigned long util_min(unsigned long a, unsigned long b)
{
	unsigned long r;

	r = ((a < b) ? a : b);
	return r;
}

/* Function declarations and documenation */
QDF_STATUS hdd_parse_config_ini(hdd_context_t *pHddCtx);
QDF_STATUS hdd_update_mac_config(hdd_context_t *pHddCtx);
QDF_STATUS hdd_set_sme_config(hdd_context_t *pHddCtx);
QDF_STATUS hdd_set_sme_chan_list(hdd_context_t *hdd_ctx);
bool hdd_update_config_cfg(hdd_context_t *pHddCtx);
QDF_STATUS hdd_cfg_get_global_config(hdd_context_t *pHddCtx, char *pBuf,
				     int buflen);

eCsrPhyMode hdd_cfg_xlate_to_csr_phy_mode(eHddDot11Mode dot11Mode);
QDF_STATUS hdd_execute_global_config_command(hdd_context_t *pHddCtx,
					     char *command);

bool hdd_is_okc_mode_enabled(hdd_context_t *pHddCtx);
QDF_STATUS hdd_set_idle_ps_config(hdd_context_t *pHddCtx, uint32_t val);

void hdd_update_tgt_cfg(void *context, void *param);
bool hdd_dfs_indicate_radar(void *context, void *param);

QDF_STATUS hdd_string_to_u8_array(char *str, uint8_t *intArray, uint8_t *len,
				  uint8_t intArrayMaxLen);
QDF_STATUS hdd_hex_string_to_u16_array(char *str, uint16_t *int_array,
				uint8_t *len, uint8_t int_array_max_len);

void hdd_cfg_print(hdd_context_t *pHddCtx);

QDF_STATUS hdd_update_nss(hdd_context_t *hdd_ctx, uint8_t nss);
#ifdef FEATURE_WLAN_SCAN_PNO
void hdd_set_pno_channel_prediction_config(
	tpSmeConfigParams sme_config, hdd_context_t *hdd_ctx);
#else
static inline void hdd_set_pno_channel_prediction_config(
	tpSmeConfigParams sme_config, hdd_context_t *hdd_ctx)
{}
#endif

#endif
