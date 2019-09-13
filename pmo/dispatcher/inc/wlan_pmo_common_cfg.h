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

#ifndef WLAN_PMO_COMMON_CFG_H__
#define WLAN_PMO_COMMON_CFG_H__

#include "wlan_pmo_common_public_struct.h"

/*
 * <ini>
 * hostArpOffload - Enable/disable host ARP offload
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable/disable firmware's capability of sending ARP
 * response to clients.
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_PMO_ENABLE_HOST_ARPOFFLOAD CFG_INI_BOOL( \
	"hostArpOffload", \
	1, \
	"enable/disable host ARP offload")

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
 * ssdp - Enable/disable SSDP
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable/disable Simple Service Discovery Protocol(SSDP).
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_PMO_ENABLE_HOST_SSDP CFG_INI_BOOL( \
	"ssdp", \
	1, \
	"Enable/disable ssdp")

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
 * This ini is used to enable/disable dynamic DTIM.
 *
 * 0 - Disable Dynamic DTIM
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
 * gEnableModulatedDTIM - Enable/Disable modulated DTIM feature
 * @Min: 0
 * @Max: 5
 * @Default: 0
 *
 * This ini is used to enable/disable modulated DTIM feature.
 *
 * 0 - Disable modulated DTIM.
 * 1 to 5 - The maximum No. of modulated DTIM period used for calculating the
 * target listen interval.
 *
 * The target listen interval will be updated to firmware when host driver is
 * setting the suspend DTIM parameters.
 *
 * This configuration will be ignored when dynamic DTIM is enabled(by
 * gEnableDynamicDTIM).
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
 * @Default: 2
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
	2, \
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
 * gEnableSapSuspend - Enable/disable SAP Suspend
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 *
 * Related: None
 *
 * Supported Feature: SAP
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ENABLE_SAP_SUSPEND CFG_INI_BOOL( \
			"gEnableSapSuspend", \
			1, \
			"Enable/disable SAP Suspend")

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
#define CFG_PMO_WOW_DATA_INACTIVITY_TIMEOUT CFG_INI_UINT( \
		"g_wow_data_inactivity_timeout", \
		1, \
		255, \
		50, \
		CFG_VALUE_OR_DEFAULT, \
		"Data activity timeout in wow mode")
/*
 * <ini>
 * gRArateLimitInterval - RA rate limit interval
 * @Min: 60
 * @Max: 3600
 * @Default: 60
 * This ini is used to set RA rate limit interval.
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_RA_RATE_LIMIT_INTERVAL CFG_INI_UINT( \
	"gRArateLimitInterval", \
	60, \
	3600, \
	60, \
	CFG_VALUE_OR_DEFAULT, \
	"RA rate limit interval")

#define CFG_PMO_COMMON_ALL \
	CFG(CFG_ENABLE_SAP_SUSPEND) \
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
	CFG(CFG_PMO_PWR_FAILURE) \
	CFG(CFG_PMO_WOW_DATA_INACTIVITY_TIMEOUT) \
	CFG(CFG_RA_RATE_LIMIT_INTERVAL)

#endif /* WLAN_PMO_COMMON_CFG_H__ */
