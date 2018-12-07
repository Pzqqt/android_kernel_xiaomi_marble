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
 * DOC: This file contains centralized definitions of converged configuration.
 */

#ifndef __HDD_CONFIG_H
#define __HDD_CONFIG_H

#if defined(CONFIG_HL_SUPPORT)
#include "wlan_tgt_def_config_hl.h"
#else
#include "wlan_tgt_def_config.h"
#endif

#define CFG_QDF_TRACE_ENABLE_DEFAULT (0xffff)
#include <wlan_action_oui_public_struct.h>

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
#define CFG_PRIVATE_WEXT_CONTROL CFG_INI_UINT( \
			"private_wext_control", \
			hdd_wext_disabled, \
			hdd_wext_enabled, \
			hdd_wext_deprecated, \
			CFG_VALUE_OR_DEFAULT, \
			"Private WEXT Control")

/*
 * <ini>
 * gInterfaceChangeWait - Interface change wait
 * @Min: 10,
 * @Max: 500000
 * @Default: 10000
 *
 * Timer waiting for interface up from the upper layer. If
 * this timer expires all the cds modules shall be closed.
 * Time Units: ms
 *
 * Related: None
 *
 * Supported Feature: All
 *
 * </ini>
 */
#define CFG_INTERFACE_CHANGE_WAIT CFG_INI_UINT( \
			"gInterfaceChangeWait", \
			10, \
			500000, \
			10000, \
			CFG_VALUE_OR_DEFAULT, \
			"Interface change wait")

#ifdef QCA_WIFI_NAPIER_EMULATION
#define CFG_TIMER_MULTIPLIER_DEFAULT	100
#else
#define CFG_TIMER_MULTIPLIER_DEFAULT	1
#endif

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
 * Related: None
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_TIMER_MULTIPLIER CFG_INI_UINT( \
			"gTimerMultiplier", \
			1, \
			0xFFFFFFFF, \
			CFG_TIMER_MULTIPLIER_DEFAULT, \
			CFG_VALUE_OR_DEFAULT, \
			"Timer Multiplier")

#define CFG_BUG_ON_REINIT_FAILURE_DEFAULT 1
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
#define CFG_BUG_ON_REINIT_FAILURE CFG_INI_BOOL( \
		"g_bug_on_reinit_failure", \
		CFG_BUG_ON_REINIT_FAILURE_DEFAULT, \
		"BUG on reinit failure")

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
#define CFG_ENABLE_RAMDUMP_COLLECTION CFG_INI_BOOL( \
			"gEnableDumpCollect", \
			1, \
			"Enable dump collect")

#if defined(MDM_PLATFORM) && !defined(FEATURE_MULTICAST_HOST_FW_MSGS)
#define CFG_MULTICAST_HOST_FW_MSGS_DEFAULT	0
#else
#define CFG_MULTICAST_HOST_FW_MSGS_DEFAULT	1
#endif

/*
 * <ini>
 * gMulticastHostFwMsgs - Multicast host FW messages
 * @Min: 0
 * @Max: 1
 * @Default: 0 for MDM platform and 1 for other
 *
 * </ini>
 */
#define CFG_MULTICAST_HOST_FW_MSGS CFG_INI_UINT( \
			"gMulticastHostFwMsgs", \
			0, \
			1, \
			CFG_MULTICAST_HOST_FW_MSGS_DEFAULT, \
			CFG_VALUE_OR_DEFAULT, \
			"Multicast host FW msgs")

#ifdef WLAN_LOGGING_SOCK_SVC_ENABLE
/*
 * <ini>
 * wlanLoggingEnable - Wlan logging enable
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * </ini>
 */
#define CFG_WLAN_LOGGING_SUPPORT CFG_INI_BOOL( \
				"wlanLoggingEnable", \
				1, \
				"Wlan logging enable")

/*
 * <ini>
 * wlanLoggingToConsole - Wlan logging to console
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * </ini>
 */
#define CFG_WLAN_LOGGING_CONSOLE_SUPPORT CFG_INI_BOOL( \
				"wlanLoggingToConsole", \
				1, \
				"Wlan logging to console")

#define CFG_WLAN_LOGGING_SUPPORT_ALL \
	CFG(CFG_WLAN_LOGGING_SUPPORT) \
	CFG(CFG_WLAN_LOGGING_CONSOLE_SUPPORT)
#else
#define CFG_WLAN_LOGGING_SUPPORT_ALL
#endif

#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
/*
 * <ini>
 * gWlanAutoShutdown - Wlan auto shutdown timer value
 * @Min: 0
 * @Max: 86400
 * @Default: 0
 *
 * This ini specifies the seconds of WLAN inactivity firmware has to wait
 * before indicating WLAN idle event to driver. Upon receiving firmware's
 * WLAN idle indication, driver may indicate similar event to upper layer
 * daemons(SCM, or any other components working to achieve the same purpose),
 * who may choose what to do next, e.g. whether to unload driver module or not.
 * 0 indicates no auto shutdown will take place.
 *
 * </ini>
 */
#define CFG_WLAN_AUTO_SHUTDOWN CFG_INI_UINT( \
			"gWlanAutoShutdown", \
			0, \
			86400, \
			0, \
			CFG_VALUE_OR_DEFAULT, \
			"Wlan auto shutdown")
#define CFG_WLAN_AUTO_SHUTDOWN_ALL \
	CFG(CFG_WLAN_AUTO_SHUTDOWN)
#else
#define CFG_WLAN_AUTO_SHUTDOWN_ALL
#endif

/*
 * <ini>
 * gEnablefwprint - Enable FW uart print
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * </ini>
 */
#define CFG_ENABLE_FW_UART_PRINT CFG_INI_BOOL( \
			"gEnablefwprint", \
			0, \
			"Enable FW uart print")

/*
 * <ini>
 * gEnablefwlog - Enable FW log
 * @Min: 0
 * @Max: 2
 * @Default: 1
 *
 * </ini>
 */
#define CFG_ENABLE_FW_LOG CFG_INI_UINT( \
			"gEnablefwlog", \
			0, \
			2, \
			1, \
			CFG_VALUE_OR_DEFAULT, \
			"Enable FW log")

#ifndef REMOVE_PKT_LOG

#ifdef FEATURE_PKTLOG
#define CFG_ENABLE_PACKET_LOG_DEFAULT	1
#else
#define CFG_ENABLE_PACKET_LOG_DEFAULT	0
#endif

/*
 * <ini>
 * gEnablePacketLog - Enale packet log
 * @Min: 0
 * @Max: 1
 * @Default: 1 if packet log code is enabled, 0 otherwise
 *
 * This option enables/disables packet log collecting.
 *
 * </ini>
 */
#define CFG_ENABLE_PACKET_LOG CFG_INI_BOOL( \
			"gEnablePacketLog", \
			CFG_ENABLE_PACKET_LOG_DEFAULT, \
			"Enable packet log")

#define CFG_ENABLE_PACKET_LOG_ALL \
	CFG(CFG_ENABLE_PACKET_LOG)
#else
#define CFG_ENABLE_PACKET_LOG_ALL
#endif

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
#define CFG_ENABLE_RUNTIME_PM CFG_INI_BOOL( \
		"gRuntimePM", \
		0, \
		"This ini is used to enable runtime_suspend")
#define CFG_ENABLE_RUNTIME_PM_ALL \
	CFG(CFG_ENABLE_RUNTIME_PM)
#else
#define CFG_ENABLE_RUNTIME_PM_ALL
#endif

/*
 * <ini>
 * gInformBssRssiRaw - Report rssi in cfg80211_inform_bss_frame
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * Option to report rssi in cfg80211_inform_bss_frame()
 *
 * Related: None
 *
 * Supported Feature: N/A
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_INFORM_BSS_RSSI_RAW CFG_INI_BOOL( \
		"gInformBssRssiRaw", \
		1, \
		"Option to report rssi in cfg80211_inform_bss_frame")

#ifdef FEATURE_WLAN_DYNAMIC_CVM
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
#define CFG_VC_MODE_BITMAP CFG_INI_INT( \
	"gConfigVCmode", \
	0x00000000, \
	0x0fffffff, \
	0x00000005, \
	CFG_VALUE_OR_DEFAULT, \
	"Bitmap for operating voltage corner mode")

#define CFG_VC_MODE_BITMAP_ALL CFG(CFG_VC_MODE_BITMAP)
#else
#define CFG_VC_MODE_BITMAP_ALL
#endif

/*
 * <ini>
 * gOperatingChannel- Default STA operating channel
 * @Min: 0
 * @Max: 14
 * @Default: 1
 *
 * This ini is used to specify the default operating channel of a STA during
 * initialization.
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: External
 *
 * <ini>
 */
#define CFG_OPERATING_CHANNEL CFG_INI_UINT( \
			"gOperatingChannel", \
			0, \
			14, \
			1, \
			CFG_VALUE_OR_DEFAULT, \
			"Default Operating Channel")

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
#define CFG_NUM_VDEV_ENABLE CFG_INI_UINT( \
		"gNumVdevs", \
		1, \
		4, \
		CFG_TGT_NUM_VDEV, \
		CFG_VALUE_OR_DEFAULT, \
		"Number of VDEVs")

#define CFG_CONCURRENT_IFACE_MAX_LEN 16
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
 * Usage: Internal
 *
 * </ini>
 */

#define CFG_ENABLE_CONCURRENT_STA CFG_INI_STRING( \
		"gEnableConcurrentSTA", \
		0, \
		CFG_CONCURRENT_IFACE_MAX_LEN, \
		"", \
		"Enable Concurrent STA")

#define CFG_DBS_SCAN_PARAM_LENGTH 42
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
#define CFG_DBS_SCAN_SELECTION CFG_INI_STRING( \
			"gdbs_scan_selection", \
			0, \
			CFG_DBS_SCAN_PARAM_LENGTH, \
			"", \
			"DBS Scan Selection")


/*
 * <ini>
 * Intf0MacAddress - Interface address
 * @Default: 000AF58989FF
 *
 * Related: None
 *
 * Supported Feature: N/A
 *
 * Usage: External
 *
 * </ini>
 */
#define __CFG_INTF0_MAC_ADDR_DEFAULT \
	{.bytes = {0x00, 0x0a, 0xf5, 0x89, 0x89, 0xff} }
#define CFG_INTF0_MAC_ADDR CFG_INI_MAC( \
	"Intf0MacAddress", \
	__CFG_INTF0_MAC_ADDR_DEFAULT, \
	"Interface 0 MAC Address")

/*
 * <ini>
 * Intf1MacAddress - Interface address
 * @Default: 000AF58989FE
 *
 * Related: None
 *
 * Supported Feature: N/A
 *
 * Usage: External
 *
 * </ini>
 */
#define __CFG_INTF1_MAC_ADDR_DEFAULT \
	{.bytes = {0x00, 0x0a, 0xf5, 0x89, 0x89, 0xfe} }
#define CFG_INTF1_MAC_ADDR CFG_INI_MAC( \
	"Intf1MacAddress", \
	__CFG_INTF1_MAC_ADDR_DEFAULT, \
	"Interface 1 MAC Address")

/*
 * <ini>
 * Intf2MacAddress - Interface address
 * @Default: 000AF58989FD
 *
 * Related: None
 *
 * Supported Feature: N/A
 *
 * Usage: External
 *
 * </ini>
 */
#define __CFG_INTF2_MAC_ADDR_DEFAULT \
	{.bytes = {0x00, 0x0a, 0xf5, 0x89, 0x89, 0xfd} }
#define CFG_INTF2_MAC_ADDR CFG_INI_MAC( \
	"Intf2MacAddress", \
	__CFG_INTF2_MAC_ADDR_DEFAULT, \
	"Interface 2 MAC Address")

/*
 * <ini>
 * Intf3MacAddress - Interface address
 * @Default: 000AF58989FC
 *
 * Related: None
 *
 * Supported Feature: N/A
 *
 * Usage: External
 *
 * </ini>
 */
#define __CFG_INTF3_MAC_ADDR_DEFAULT \
	{.bytes = {0x00, 0x0a, 0xf5, 0x89, 0x89, 0xfc} }
#define CFG_INTF3_MAC_ADDR CFG_INI_MAC( \
	"Intf3MacAddress", \
	__CFG_INTF3_MAC_ADDR_DEFAULT, \
	"Interface 3 MAC Address")

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
 *		      ((Data_Length % 8) ? 1 : 0)
 * and <Data_Mask> has to be constructed from left to right.
 *
 * Presence of <Mac_Address> and <Capability> is
 * controlled by <Info_Presence_Bit> which is mandatory
 * <Info_Presence_Bit> will give the information for
 *   OUI – bit 0 Should be set to 1
 *		 Setting to 0 will ignore OUI and data check
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

#define CFG_ACTION_OUI_MAX_LEN 256

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
#define CFG_ENABLE_ACTION_OUI CFG_INI_BOOL( \
	"gEnableActionOUI", \
	1, \
	"Enable/Disable action oui feature")

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
#define CFG_ACTION_OUI_CONNECT_1X1 CFG_INI_STRING( \
	"gActionOUIConnect1x1", \
	0, \
	ACTION_OUI_MAX_STR_LEN, \
	"000C43 00 25 42 001018 06 02FFF02C0000 BC 25 42 001018 06 02FF040C0000 BC 25 42 00037F 00 35 6C", \
	"Used to specify action OUIs for 1x1 connection")

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
#define CFG_ACTION_OUI_ITO_EXTENSION CFG_INI_STRING( \
	"gActionOUIITOExtension", \
	0, \
	ACTION_OUI_MAX_STR_LEN, \
	"00037F 06 01010000FF7F FC 01 000AEB 02 0100 C0 01 000B86 03 010408 E0 01", \
	"Used to extend in-activity time for specified APs")

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
#define CFG_ACTION_OUI_CCKM_1X1 CFG_INI_STRING( \
	"gActionOUICCKM1X1", \
	0, \
	ACTION_OUI_MAX_STR_LEN, \
	"", \
	"Used to specify action OUIs to control station's TX rates")

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
#define CFG_ACTION_OUI_ITO_ALTERNATE CFG_INI_STRING( \
	"gActionOUIITOAlternate", \
	0, \
	ACTION_OUI_MAX_STR_LEN, \
	"001018 06 0202001c0000 FC 01", \
	"Used to specify action OUIs to have alternate ITO")

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
#define CFG_ACTION_OUI_SWITCH_TO_11N_MODE CFG_INI_STRING( \
	"gActionOUISwitchTo11nMode", \
	0, \
	ACTION_OUI_MAX_STR_LEN, \
	"00904C 03 0418BF E0 21 40", \
	"Used to specify action OUIs for switching to 11n")

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
#define CFG_ACTION_OUI_CONNECT_1X1_WITH_1_CHAIN CFG_INI_STRING( \
	 "gActionOUIConnect1x1with1TxRxChain", \
	 0, \
	 ACTION_OUI_MAX_STR_LEN, \
	 "001018 06 02FFF0040000 BC 21 40 001018 06 02FFF0050000 BC 21 40 001018 06 02FFF4050000 BC 21 40", \
	 "Used to specify action OUIs for 1x1 connection with one Tx/Rx Chain")

 /* End of action oui inis */

#define CFG_HDD_ALL \
	CFG_ENABLE_PACKET_LOG_ALL \
	CFG_ENABLE_RUNTIME_PM_ALL \
	CFG_VC_MODE_BITMAP_ALL \
	CFG_WLAN_AUTO_SHUTDOWN_ALL \
	CFG_WLAN_LOGGING_SUPPORT_ALL \
	CFG(CFG_ACTION_OUI_CCKM_1X1) \
	CFG(CFG_ACTION_OUI_CONNECT_1X1) \
	CFG(CFG_ACTION_OUI_CONNECT_1X1_WITH_1_CHAIN) \
	CFG(CFG_ACTION_OUI_ITO_ALTERNATE) \
	CFG(CFG_ACTION_OUI_ITO_EXTENSION) \
	CFG(CFG_ACTION_OUI_SWITCH_TO_11N_MODE) \
	CFG(CFG_BUG_ON_REINIT_FAILURE) \
	CFG(CFG_DBS_SCAN_SELECTION) \
	CFG(CFG_ENABLE_CONCURRENT_STA) \
	CFG(CFG_ENABLE_ACTION_OUI) \
	CFG(CFG_ENABLE_FW_LOG) \
	CFG(CFG_ENABLE_FW_UART_PRINT) \
	CFG(CFG_ENABLE_RAMDUMP_COLLECTION) \
	CFG(CFG_INTERFACE_CHANGE_WAIT) \
	CFG(CFG_INFORM_BSS_RSSI_RAW) \
	CFG(CFG_INTF0_MAC_ADDR) \
	CFG(CFG_INTF1_MAC_ADDR) \
	CFG(CFG_INTF2_MAC_ADDR) \
	CFG(CFG_INTF3_MAC_ADDR) \
	CFG(CFG_MULTICAST_HOST_FW_MSGS) \
	CFG(CFG_NUM_VDEV_ENABLE) \
	CFG(CFG_OPERATING_CHANNEL) \
	CFG(CFG_PRIVATE_WEXT_CONTROL) \
	CFG(CFG_TIMER_MULTIPLIER)
#endif
