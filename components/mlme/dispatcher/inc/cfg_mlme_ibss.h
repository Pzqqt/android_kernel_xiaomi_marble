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

#ifndef __CFG_MLME_IBSS_H
#define __CFG_MLME_IBSS_H

/*
 * <ini>
 * g_IBSS_AUTO_BSSID - Control IBSS Auto BSSID setup
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * Control IBSS Auto BSSID enable / disable
 * Usage: External
 *
 * </ini>
 */
#define CFG_IBSS_AUTO_BSSID CFG_BOOL( \
			"gIbssAutoBssid", \
			1, \
			"Enable Auto BSSID for IBSS")

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
#define CFG_IBSS_ADHOC_CHANNEL_5GHZ CFG_INI_UINT( \
		"gAdHocChannel5G", \
		36, \
		165, \
		44, \
		CFG_VALUE_OR_DEFAULT, \
		"Default 5Ghz IBSS channel if not provided by supplicant")

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
#define CFG_IBSS_ADHOC_CHANNEL_24GHZ CFG_INI_UINT( \
		"gAdHocChannel24G", \
		1, \
		14, \
		6, \
		CFG_VALUE_OR_DEFAULT, \
		"Default 2.4Ghz IBSS channel if not provided by supplicant")

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
#define CFG_IBSS_COALESING CFG_INI_BOOL( \
		"gCoalesingInIBSS", \
		0, \
		"IBSS coalesing control param")

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
#define CFG_IBSS_ATIM_WIN_SIZE CFG_INI_UINT( \
		"gIbssATIMWinSize", \
		0, \
		50, \
		0, \
		CFG_VALUE_OR_DEFAULT, \
		"Set IBSS ATIM window size")

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
#define CFG_IBSS_IS_POWER_SAVE_ALLOWED CFG_INI_BOOL( \
		"gIbssIsPowerSaveAllowed", \
		1, \
		"IBSS Power Save control")

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
#define CFG_IBSS_IS_POWER_COLLAPSE_ALLOWED CFG_INI_BOOL( \
		"gIbssIsPowerCollapseAllowed", \
		1, \
		"Indicates if IBSS Power Collapse is allowed")

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
#define CFG_IBSS_AWAKE_ON_TX_RX CFG_INI_BOOL( \
		"gIbssAwakeOnTxRx", \
		0, \
		"IBSS sta power save mode on TX/RX activity")

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
#define CFG_IBSS_INACTIVITY_TIME CFG_INI_UINT( \
		"gIbssInactivityTime", \
		1, \
		10, \
		1, \
		CFG_VALUE_OR_DEFAULT, \
		"No of Beacons intervals of data inactivity for power save")

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
#define CFG_IBSS_TXSP_END_INACTIVITY CFG_INI_UINT( \
		"gIbssTxSpEndInactivityTime", \
		0, \
		100, \
		0, \
		CFG_VALUE_OR_DEFAULT, \
		"TX service period inactivity timeout")

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
#define CFG_IBSS_PS_WARMUP_TIME CFG_INI_UINT( \
		"gIbssPsWarmupTime", \
		0, \
		65535, \
		0, \
		CFG_VALUE_OR_DEFAULT, \
		"IBSS Power save skip time")

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
#define CFG_IBSS_PS_1RX_CHAIN_IN_ATIM_WINDOW CFG_INI_BOOL( \
		"gIbssPs1RxChainInAtim", \
		0, \
		"Control IBSS Power save in 1RX chain during ATIM")

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
#define IBSS_BSSID_DEFAULT { .bytes = { 0x00, 0x0a, 0xf5, 0x04, 0x05, 0x06 } }
#define CFG_IBSS_BSSID CFG_INI_MAC("gIbssBssid", \
		IBSS_BSSID_DEFAULT, \
		"IBSS BSSID if not provided by supplicant")

#define CFG_IBSS_ALL \
	CFG(CFG_IBSS_ADHOC_CHANNEL_5GHZ) \
	CFG(CFG_IBSS_ADHOC_CHANNEL_24GHZ) \
	CFG(CFG_IBSS_ATIM_WIN_SIZE) \
	CFG(CFG_IBSS_AUTO_BSSID) \
	CFG(CFG_IBSS_AWAKE_ON_TX_RX) \
	CFG(CFG_IBSS_BSSID) \
	CFG(CFG_IBSS_COALESING) \
	CFG(CFG_IBSS_INACTIVITY_TIME) \
	CFG(CFG_IBSS_IS_POWER_COLLAPSE_ALLOWED) \
	CFG(CFG_IBSS_IS_POWER_SAVE_ALLOWED) \
	CFG(CFG_IBSS_PS_1RX_CHAIN_IN_ATIM_WINDOW) \
	CFG(CFG_IBSS_PS_WARMUP_TIME) \
	CFG(CFG_IBSS_TXSP_END_INACTIVITY)
#endif
