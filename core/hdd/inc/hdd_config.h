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
 * gWlanAutoShutdown - Wlan auto shutdown
 * @Min: 0
 * @Max: 86400
 * @Default: 0
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
 * @Default: 1 if feature packet log define, 0 elsewhere
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

#define CFG_HDD_ALL \
	CFG_ENABLE_PACKET_LOG_ALL \
	CFG_WLAN_AUTO_SHUTDOWN_ALL \
	CFG_WLAN_LOGGING_SUPPORT_ALL \
	CFG(CFG_BUG_ON_REINIT_FAILURE) \
	CFG(CFG_ENABLE_FW_LOG) \
	CFG(CFG_ENABLE_FW_UART_PRINT) \
	CFG(CFG_ENABLE_RAMDUMP_COLLECTION) \
	CFG(CFG_INTERFACE_CHANGE_WAIT) \
	CFG(CFG_MULTICAST_HOST_FW_MSGS) \
	CFG(CFG_PRIVATE_WEXT_CONTROL) \
	CFG(CFG_TIMER_MULTIPLIER)
#endif
