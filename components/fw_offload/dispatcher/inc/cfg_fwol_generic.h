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

#ifndef __CFG_FWOL_GENERIC_H
#define __CFG_FWOL_GENERIC_H


/*
 *
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
#define CFG_ENABLE_ANI CFG_INI_BOOL( \
		"gEnableANI", \
		1, \
		"Enable/Disable Adaptive Noise Immunity")

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
#define CFG_SET_RTS_FOR_SIFS_BURSTING CFG_INI_BOOL( \
		"gSetRTSForSIFSBursting", \
		0, \
		"Set rts for sifs bursting")

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
#define CFG_MAX_MPDUS_IN_AMPDU CFG_INI_INT( \
		"gMaxMPDUsInAMPDU", \
		0, \
		64, \
		0, \
		CFG_VALUE_OR_DEFAULT, \
		"This ini configure max mpdus in ampdu")

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
#define CFG_ARP_AC_CATEGORY CFG_INI_INT( \
		"arp_ac_category", \
		0, \
		3, \
		3, \
		CFG_VALUE_OR_DEFAULT, \
		"Override the default ARP AC configuration")

/*
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
#define CFG_ENABLE_PHY_REG CFG_INI_UINT( \
		"gEnableFastPwrTransition", \
		0x0, \
		0x2, \
		0x0, \
		CFG_VALUE_OR_DEFAULT, \
		"Configuration for fast power transition")

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
#define CFG_UPPER_BRSSI_THRESH CFG_INI_UINT( \
		"gUpperBrssiThresh", \
		36, \
		66, \
		46, \
		CFG_VALUE_OR_DEFAULT, \
		"Sets Upper threshold for beacon RSSI")

/*
 * <ini>
 * gLowerBrssiThresh - Sets Lower threshold for beacon RSSI
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
#define CFG_LOWER_BRSSI_THRESH CFG_INI_UINT( \
		"gLowerBrssiThresh", \
		6, \
		36, \
		26, \
		CFG_VALUE_OR_DEFAULT, \
		"Sets Lower threshold for beacon RSSI")

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
#define CFG_DTIM_1CHRX_ENABLE CFG_INI_BOOL( \
		"gDtim1ChRxEnable", \
		1, \
		"Enable/Disable DTIM 1Chrx feature")

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
#define CFG_ENABLE_COEX_ALT_CHAINMASK CFG_INI_BOOL( \
		"gEnableAlternativeChainmask", \
		0, \
		"Enable Co-Ex Alternative Chainmask")

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
#define CFG_ENABLE_SMART_CHAINMASK CFG_INI_BOOL( \
		"gEnableSmartChainmask", \
		0, \
		"Enable/disable the Smart Chainmask feature")

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
#define CFG_ENABLE_FW_RTS_PROFILE CFG_INI_INT( \
		"gEnableRTSProfiles", \
		0, \
		66, \
		33, \
		CFG_VALUE_OR_DEFAULT, \
		"It is used to configure different RTS profiles")

/* <ini>
 * gFwDebugLogLevel - Firmware debug log level
 * @Min: 0
 * @Max: 255
 * @Default: 3
 *
 * This option controls the level of firmware debug log. Default value is
 * DBGLOG_WARN, which is to enable error and warning logs.
 *
 * Related: None
 *
 * Supported Features: Debugging
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_ENABLE_FW_DEBUG_LOG_LEVEL CFG_INI_INT( \
		"gFwDebugLogLevel", \
		0, \
		255, \
		3, \
		CFG_VALUE_OR_DEFAULT, \
		"enable error and warning logs by default")

/* <ini>
 * gFwDebugLogType - Firmware debug log type
 * @Min: 0
 * @Max: 255
 * @Default: 3
 *
 * This option controls how driver is to give the firmware logs to net link
 * when cnss_diag service is started.
 *
 * Related: None
 *
 * Supported Features: Debugging
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_ENABLE_FW_LOG_TYPE CFG_INI_INT( \
		"gFwDebugLogType", \
		0, \
		255, \
		3, \
		CFG_VALUE_OR_DEFAULT, \
		"Default value to be given to the net link cnss_diag service")

#ifdef FEATURE_WLAN_RA_FILTERING
/* <ini>
 * gRAFilterEnable
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * Related: None
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_RA_FILTER_ENABLE CFG_INI_BOOL( \
		"gRAFilterEnable", \
		1, \
		"Enable RA Filter")
#else
#define CFG_RA_FILTER_ENABLE
#endif

/* <ini>
 * gtsf_gpio_pin
 * @Min: 0
 * @Max: 254
 * @Default: 255
 *
 * GPIO pin to toggle when capture tsf
 *
 * Related: None
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_SET_TSF_GPIO_PIN CFG_INI_INT( \
		"gtsf_gpio_pin", \
		0, \
		254, \
		255, \
		CFG_VALUE_OR_DEFAULT, \
		"GPIO pin to toggle when capture tsf")

#ifdef DHCP_SERVER_OFFLOAD
/* <ini>
 * gDHCPServerOffloadEnable
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * DHCP Server offload support
 *
 * Related: None
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_DHCP_SERVER_OFFLOAD_SUPPORT CFG_INI_BOOL( \
		"gDHCPServerOffloadEnable", \
		0, \
		"DHCP Server offload support")

/* <ini>
 * gDHCPMaxNumClients
 * @Min: 1
 * @Max: 8
 * @Default: 8
 *
 * Number of DHCP server offload clients
 *
 * Related: None
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_DHCP_SERVER_OFFLOAD_NUM_CLIENT CFG_INI_INT( \
		"gDHCPMaxNumClients", \
		1, \
		8, \
		8, \
		CFG_VALUE_OR_DEFAULT, \
		"Number of DHCP server offload clients")

#define CFG_FWOL_DHCP \
		CFG(CFG_DHCP_SERVER_OFFLOAD_SUPPORT) \
		CFG(CFG_DHCP_SERVER_OFFLOAD_NUM_CLIENT)

#else
#define CFG_FWOL_DHCP
#endif

#define CFG_FWOL_GENERIC_ALL \
	CFG_FWOL_DHCP \
	CFG(CFG_ENABLE_ANI) \
	CFG(CFG_SET_RTS_FOR_SIFS_BURSTING) \
	CFG(CFG_MAX_MPDUS_IN_AMPDU) \
	CFG(CFG_ARP_AC_CATEGORY) \
	CFG(CFG_ENABLE_PHY_REG) \
	CFG(CFG_UPPER_BRSSI_THRESH) \
	CFG(CFG_LOWER_BRSSI_THRESH) \
	CFG(CFG_DTIM_1CHRX_ENABLE) \
	CFG(CFG_ENABLE_COEX_ALT_CHAINMASK) \
	CFG(CFG_ENABLE_SMART_CHAINMASK) \
	CFG(CFG_ENABLE_FW_RTS_PROFILE) \
	CFG(CFG_ENABLE_FW_DEBUG_LOG_LEVEL) \
	CFG(CFG_ENABLE_FW_LOG_TYPE) \
	CFG(CFG_RA_FILTER_ENABLE) \
	CFG(CFG_SET_TSF_GPIO_PIN)

#endif
