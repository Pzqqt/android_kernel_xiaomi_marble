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

#ifndef WLAN_PMO_CFG_H__
#define WLAN_PMO_CFG_H__

#include "wlan_pmo_common_public_struct.h"

/*
 * <ini>
 * hostArpOffload - Enable/disable host ARP offload
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable/disable host ARP offload.
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_PMO_ENABLE_HOST_ARPOFFLOAD CFG_INI_BOOL( \
	"hostArpOffload", \
	1, \
	"enable/disable host arp offload")

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
#define CFG_PMO_HW_FILTER_MODE CFG_INI_UINT( \
	"gHwFilterMode", \
	0, \
	3, \
	1, \
	CFG_VALUE_OR_DEFAULT, \
	"hardware filter for DTIM mode")

/*
 * <ini>
 * ssdp - Enable/disable ssdp
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable/disable ssdp.
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_PMO_ENABLE_HOST_SSDP CFG_INI_BOOL( \
	"ssdp", \
	1, \
	"Enable/disable ssdp")

#ifdef FEATURE_RUNTIME_PM
/*
 * <ini>
 * gRuntimePMDelay - Set runtime pm's inactivity timer
 * @Min: 100
 * @Max: 10000
 * @Default: 500
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
#define CFG_PMO_RUNTIME_PM_DELAY CFG_INI_UINT( \
	"gRuntimePMDelay", \
	100, \
	10000, \
	500, \
	CFG_VALUE_OR_DEFAULT, \
	"Set runtime pm's inactivity timer")
#endif /* FEATURE_RUNTIME_PM */

/*
 * <ini>
 * hostNSOffload - Enable/disable NS offload
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable/disable NS offload.
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_PMO_ENABLE_HOST_NSOFFLOAD CFG_INI_BOOL( \
	"hostNSOffload", \
	1, \
	"Enable/disable NS offload")

/*
 * <ini>
 * gEnableDynamicDTIM - Enable Dynamic DTIM
 * @Min: 0
 * @Max: 9
 * @Default: 0
 *
 * This ini is used to enable/disable ssdp.
 *
 * 0 -Disable DynamicDTIM
 * 1 to 5 - SLM will switch to DTIM specified here when host suspends and
 *          switch DTIM1 when host resumes
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_PMO_ENABLE_DYNAMIC_DTIM CFG_INI_UINT( \
	"gEnableDynamicDTIM", \
	0, \
	9, \
	0, \
	CFG_VALUE_OR_DEFAULT, \
	"Enable Dynamic DTIM")

/*
 * <ini>
 * gEnableDynamicDTIM - Enable/Disable modulated DTIM feature
 * @Min: 0
 * @Max: 5
 * @Default: 0
 *
 * This ini is used to enable/disable modulated DTIM feature.
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_PMO_ENABLE_MODULATED_DTIM CFG_INI_UINT( \
	"gEnableModulatedDTIM", \
	0, \
	5, \
	0, \
	CFG_VALUE_OR_DEFAULT, \
	"Enable/disable modulated DTIM feature")

/*
 * <ini>
 * gMCAddrListEnable - Enable/disable multicast MAC address list feature
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable/disable multicast MAC address list feature.
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
#define CFG_PMO_MC_ADDR_LIST_ENABLE CFG_INI_BOOL( \
	"gMCAddrListEnable", \
	1, \
	"Enable/disable multicast MAC address list feature")

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
#define CFG_PMO_POWERSAVE_OFFLOAD CFG_INI_UINT( \
	"gEnablePowerSaveOffload", \
	0, \
	5, \
	0, \
	CFG_VALUE_OR_DEFAULT, \
	"Enable Power Save Offload")

/*
 * <ini>
 * gMaxPsPoll - Max powersave poll
 * @Min: 0
 * @Max: 255
 * @Default: 0
 *
 * This ini is used to set max powersave poll.
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_PMO_MAX_PS_POLL CFG_INI_UINT( \
		"gMaxPsPoll", \
		0, \
		255, \
		0, \
		CFG_VALUE_OR_DEFAULT, \
		"Max powersave poll")

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
#define CFG_PMO_WOW_ENABLE CFG_INI_UINT("gEnableWoW", \
					0, 3, 3, \
					CFG_VALUE_OR_DEFAULT, \
					"Enable WoW Support")
/*
 * <ini>
 * wowlan_deauth_enable - Enable/Disable wowlan deauth enable
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable/disable wowlan deauth enable.
 *
 * Related: None
 *
 * Supported Feature: Power Save
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_PMO_WOWLAN_DEAUTH_ENABLE CFG_INI_BOOL("wowlan_deauth_enable", \
						  1, \
						  "Enable WoWLan deauth")
/*
 * <ini>
 * wowlan_disassoc_enable - Enable/Disable wowlan disassoc enable
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable/disable wowlan disassoc enable.
 *
 * Related: None
 *
 * Supported Feature: Power Save
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_PMO_WOWLAN_DISASSOC_ENABLE CFG_INI_BOOL("wowlan_disassoc_enable", \
						    1, \
						    "Enable WoW Support")

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
#define CFG_EXTWOW_GOTO_SUSPEND CFG_INI_BOOL("gExtWoWgotoSuspend", \
					     1, \
					     "Enable Ext WoW goto support")
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
#define CFG_EXTWOW_APP1_WAKE_PIN_NUMBER \
		CFG_INI_UINT("gExtWowApp1WakeupPinNumber", \
			     0, 255, 12, \
			     CFG_VALUE_OR_DEFAULT, \
			     "Set wakeup1 PIN number")
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
#define CFG_EXTWOW_APP2_WAKE_PIN_NUMBER \
		CFG_INI_UINT("gExtWowApp2WakeupPinNumber", \
			     0, 255, 16, \
			     CFG_VALUE_OR_DEFAULT, \
			     "Set wakeup2 PIN number")
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
#define CFG_EXTWOW_KA_INIT_PING_INTERVAL \
		CFG_INI_UINT("gExtWoWApp2KAInitPingInterval", \
			     0, 0xffffffff, 240, \
			     CFG_VALUE_OR_DEFAULT, \
			     "Set Keep Alive Init Ping Interval")
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
#define CFG_EXTWOW_KA_MIN_PING_INTERVAL \
		CFG_INI_UINT("gExtWoWApp2KAMinPingInterval", \
			     0, 0xffffffff, 240, \
			     CFG_VALUE_OR_DEFAULT, \
			     "Set Keep Alive Minimum Ping Interval")
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
#define CFG_EXTWOW_KA_MAX_PING_INTERVAL \
		CFG_INI_UINT("gExtWoWApp2KAMaxPingInterval", \
			     0, 0xffffffff, 1280, \
			     CFG_VALUE_OR_DEFAULT, \
			     "Set Keep Alive Maximum Ping Interval")
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
#define CFG_EXTWOW_KA_INC_PING_INTERVAL \
		CFG_INI_UINT("gExtWoWApp2KAIncPingInterval", \
			     0, 0xffffffff, 4, \
			     CFG_VALUE_OR_DEFAULT, \
			     "Set Keep Alive increment of Ping Interval")
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
#define CFG_EXTWOW_TCP_SRC_PORT \
		CFG_INI_UINT("gExtWoWApp2KAIncPingInterval", \
			     0, 65535, 5000, \
			     CFG_VALUE_OR_DEFAULT, \
			     "Set TCP source port")
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
#define CFG_EXTWOW_TCP_DST_PORT \
		CFG_INI_UINT("gExtWoWApp2TcpDstPort", \
			     0, 65535, 5001, \
			     CFG_VALUE_OR_DEFAULT, \
			     "Set TCP Destination port")
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
#define CFG_EXTWOW_TCP_TX_TIMEOUT \
		CFG_INI_UINT("gExtWoWApp2TcpTxTimeout", \
			     0, 0xffffffff, 200, \
			     CFG_VALUE_OR_DEFAULT, \
			     "Set TCP tx timeout")

/*
 * <ini>
 * gExtWoWApp2TcpRxTimeout - Set TCP rx timeout
 * @Min: 0
 * @Max: 0xffffffff
 * @Default: 200
 *
 * This ini is used to set TCP Rx timeout when EXT WOW is enabled
 *
 * Supported Feature: Power Save
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_EXTWOW_TCP_RX_TIMEOUT \
		CFG_INI_UINT("gExtWoWApp2TcpRxTimeout", \
			     0, 0xffffffff, 200, \
			     CFG_VALUE_OR_DEFAULT, \
			     "ExtWow App2 tcp rx timeout")
#endif

/*
 * <ini>
 * gBpfFilterEnable - APF feature support configuration
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * When set to 1 APF feature will be enabled.
 *
 * Supported Feature: Android packet filter
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_PMO_APF_ENABLE CFG_INI_BOOL("gBpfFilterEnable", \
					1, \
					"Enable APF Support")

/*
 * <ini>
 * gActiveModeOffload - Active offload mode configuration
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * When set to 1 active mode offload will be enabled.
 *
 * If active mode offload is enabled then all applicable data offload/filtering
 * is enabled immediately in FW once config is available in WLAN driver and FW
 * caches this configuration across suspend/resume;
 * If active mode offload is disabled then all applicable data offload/filtering
 * is enabled during cfg80211 suspend and disabled during cfg80211 resume.
 *
 * Supported Feature: Active mode offload
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_PMO_ACTIVE_MODE CFG_INI_BOOL("gActiveModeOffload", \
					 1, \
					 "Enable active mode offload")

/*
 * <ini>
 * gwow_pulse_support - WOW pulse feature configuration
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * When set to 1 WOW pulse feature will be enabled.
 *
 * Related: gwow_pulse_pin, gwow_pulse_interval_low, gwow_pulse_interval_high
 *
 * Supported Feature: WOW pulse
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_PMO_WOW_PULSE_ENABLE CFG_INI_BOOL("gwow_pulse_support", \
					      0, \
					      "Enable wow pulse")

/*
 * <ini>
 * gwow_pulse_pin - GPIO pin for WOW pulse
 * @Min: 0
 * @Max: 254
 * @Default: 35
 *
 * Which PIN to send the Pulse
 *
 * Supported Feature: WOW pulse
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_PMO_WOW_PULSE_PIN CFG_INI_UINT("gwow_pulse_pin", \
					   0, 254, 35, \
					   CFG_VALUE_OR_DEFAULT, \
					   "Pin for wow pulse")

/*
 * <ini>
 * gwow_pulse_interval_low - Pulse interval low
 * @Min: 160
 * @Max: 480
 * @Default: 180
 *
 * The interval of low level in the pulse
 *
 * Supported Feature: WOW pulse
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_PMO_WOW_PULSE_LOW CFG_INI_UINT("gwow_pulse_interval_low", \
					   160, 480, 180, \
					   CFG_VALUE_OR_DEFAULT, \
					   "Interval of low pulse")

/*
 * <ini>
 * gwow_pulse_interval_high - Pulse interval high
 * @Min: 20
 * @Max: 40
 * @Default: 20
 *
 * The interval of high level in the pulse
 *
 * Supported Feature: WOW pulse
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_PMO_WOW_PULSE_HIGH CFG_INI_UINT("gwow_pulse_interval_high", \
					    20, 40, 20, \
					    CFG_VALUE_OR_DEFAULT, \
					    "Interval of high pulse")

/*
 * <ini>
 * g_auto_detect_power_failure_mode - Auto detect power save failure mode
 * @Min: PMO_FW_TO_CRASH_ON_PWR_FAILURE
 * @Max: PMO_AUTO_PWR_FAILURE_DETECT_DISABLE
 * @Default: PMO_FW_TO_CRASH_ON_PWR_FAILURE
 *
 * Specifies the behavior of FW in case of CHIP_POWER_SAVE_FAIL_DETECTED event
 *
 * Supported Feature: Auto detect power save failure
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_PMO_PWR_FAILURE CFG_INI_UINT("g_auto_detect_power_failure_mode", \
					 PMO_FW_TO_CRASH_ON_PWR_FAILURE, \
					 PMO_AUTO_PWR_FAILURE_DETECT_DISABLE, \
					 PMO_FW_TO_CRASH_ON_PWR_FAILURE, \
					 CFG_VALUE_OR_DEFAULT, \
					 "Auto detect power save failure mode")

/*
 * <ini>
 * g_enable_packet_filter_bitmap - Packet filters configuration
 * @Min: 0
 * @Max: 63
 * @Default: 0
 *
 * To enable packet filters when target goes to suspend, clear when resume:
 * bit-0 : IPv6 multicast
 * bit-1 : IPv4 multicast
 * bit-2 : IPv4 broadcast
 * bit-3 : XID - Exchange station Identification packet, solicits the
 *         identification of the receiving station
 * bit-4 : STP - Spanning Tree Protocol, builds logical loop free topology
 * bit-5 : DTP/LLC/CDP
 *         DTP - Dynamic Trunking Protocol is used by Cisco switches to
 *               negotiate whether an interconnection between two switches
 *               should be put into access or trunk mode
 *         LLC - Logical link control, used for multiplexing, flow & error
 *               control
 *         CDP - Cisco Discovery Protocol packet contains information about the
 *               cisco devices in the network
 *
 * Supported Feature: Packet filtering
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_PMO_PKT_FILTER CFG_INI_UINT("g_enable_packet_filter_bitmap", \
					0, 63, 0, \
					CFG_VALUE_OR_DEFAULT, \
					"Packet filter bitmap configure")

#ifdef WLAN_FEATURE_EXTWOW_SUPPORT
	#define CFG_EXTWOW_ALL \
		CFG(CFG_EXTWOW_GOTO_SUSPEND) \
		CFG(CFG_EXTWOW_APP1_WAKE_PIN_NUMBER) \
		CFG(CFG_EXTWOW_APP2_WAKE_PIN_NUMBER) \
		CFG(CFG_EXTWOW_KA_INIT_PING_INTERVAL) \
		CFG(CFG_EXTWOW_KA_MIN_PING_INTERVAL) \
		CFG(CFG_EXTWOW_KA_MAX_PING_INTERVAL) \
		CFG(CFG_EXTWOW_KA_INC_PING_INTERVAL) \
		CFG(CFG_EXTWOW_TCP_SRC_PORT) \
		CFG(CFG_EXTWOW_TCP_DST_PORT) \
		CFG(CFG_EXTWOW_TCP_TX_TIMEOUT) \
		CFG(CFG_EXTWOW_TCP_RX_TIMEOUT)
#else
	#define CFG_EXTWOW_ALL
#endif

#ifdef FEATURE_WLAN_APF
	#define CFG_PMO_APF_ALL \
	CFG(CFG_PMO_APF_ENABLE)
#else
	#define CFG_PMO_APF_ALL
#endif

#ifdef WLAN_FEATURE_WOW_PULSE
	#define CFG_WOW_ALL \
	CFG(CFG_PMO_WOW_PULSE_ENABLE) \
	CFG(CFG_PMO_WOW_PULSE_PIN) \
	CFG(CFG_PMO_WOW_PULSE_LOW) \
	CFG(CFG_PMO_WOW_PULSE_HIGH)
#else
	#define CFG_WOW_ALL
#endif

#ifdef WLAN_FEATURE_PACKET_FILTERING
	#define CFG_PACKET_FILTERING_ALL \
	CFG(CFG_PMO_PKT_FILTER)
#else
	#define CFG_PACKET_FILTERING_ALL
#endif

#ifdef FEATURE_RUNTIME_PM
	#define CFG_RUNTIME_PM_ALL \
	CFG(CFG_PMO_RUNTIME_PM_DELAY)
#else
	#define CFG_RUNTIME_PM_ALL
#endif

#define CFG_PMO_BASIC_ALL \
	CFG(CFG_PMO_ENABLE_HOST_ARPOFFLOAD) \
	CFG(CFG_PMO_HW_FILTER_MODE) \
	CFG(CFG_PMO_ENABLE_HOST_SSDP) \
	CFG(CFG_PMO_ENABLE_HOST_NSOFFLOAD) \
	CFG(CFG_PMO_ENABLE_DYNAMIC_DTIM) \
	CFG(CFG_PMO_ENABLE_MODULATED_DTIM) \
	CFG(CFG_PMO_MC_ADDR_LIST_ENABLE) \
	CFG(CFG_PMO_POWERSAVE_OFFLOAD) \
	CFG(CFG_PMO_MAX_PS_POLL) \
	CFG(CFG_PMO_WOWLAN_DEAUTH_ENABLE) \
	CFG(CFG_PMO_WOWLAN_DISASSOC_ENABLE) \
	CFG(CFG_PMO_WOW_ENABLE) \
	CFG(CFG_PMO_ACTIVE_MODE) \
	CFG(CFG_PMO_PWR_FAILURE)

#define CFG_PMO_ALL \
	CFG_EXTWOW_ALL \
	CFG_PACKET_FILTERING_ALL \
	CFG_PMO_APF_ALL \
	CFG_PMO_BASIC_ALL \
	CFG_RUNTIME_PM_ALL \
	CFG_WOW_ALL
#endif /* WLAN_PMO_CFG_H__ */
