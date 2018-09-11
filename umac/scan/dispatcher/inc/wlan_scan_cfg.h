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
 * DOC: This file contains centralized definitions of SCAN component
 */
#ifndef __CONFIG_SCAN_H
#define __CONFIG_SCAN_H

#include "cfg_define.h"

/*
 * <ini>
 * drop_bcn_on_chan_mismatch - drop the beacon for chan mismatch
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to decide drop the beacon or not if channel received
 * in metadata doesn't match the one in beacon.
 *
 * Related: None
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_DROP_BCN_ON_CHANNEL_MISMATCH CFG_INI_BOOL(\
		"drop_bcn_on_chan_mismatch",\
		true,\
		"drop bcn on channel mismatch")

/*
 * <ini>
 * gActiveMaxChannelTime - Set max channel time for active scan
 * @Min: 0
 * @Max: 10000
 * @Default: 40
 *
 * This ini is used to set maximum channel time in msecs spent in
 * active scan
 *
 * Related: None
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ACTIVE_MAX_CHANNEL_TIME CFG_INI_UINT(\
		"gActiveMaxChannelTime",\
		0, 10000, MCL_OR_WIN_VALUE(40, 105),\
		CFG_VALUE_OR_DEFAULT, "active dwell time")

/*
 * <ini>
 * active_max_channel_time_2g - Set max time for active 2G channel scan
 * @Min: 0
 * @Max: 10000
 * @Default: 80
 *
 * This ini is used to set maximum time in msecs spent in active 2G channel scan
 * if it's not zero, in case of zero, CFG_ACTIVE_MAX_CHANNEL_TIME is used for 2G
 * channels also.
 *
 * Related: None
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ACTIVE_MAX_2G_CHANNEL_TIME CFG_INI_UINT(\
		"active_max_channel_time_2g",\
		0, 10000, MCL_OR_WIN_VALUE(80, 0),\
		CFG_VALUE_OR_DEFAULT, "active dwell time for 2G channels")

/*
 * <ini>
 * gPassiveMaxChannelTime - Set max channel time for passive scan
 * @Min: 0
 * @Max: 10000
 * @Default: 110
 *
 * This ini is used to set maximum channel time in msecs spent in
 * passive scan
 *
 * Related: None
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_PASSIVE_MAX_CHANNEL_TIME CFG_INI_UINT(\
		"gPassiveMaxChannelTime",\
		0, 10000, MCL_OR_WIN_VALUE(100, 300),\
		CFG_VALUE_OR_DEFAULT, "passive dwell time")

/*
 * <ini>
 * gScanNumProbes - Set the number of probes on each channel for active scan
 * @Min: 0
 * @Max: 20
 * @Default: 0
 *
 * This ini is used to set number of probes on each channel for
 * active scan
 */
#define CFG_SCAN_NUM_PROBES CFG_INI_UINT(\
			"gScanNumProbes",\
			0, 20, MCL_OR_WIN_VALUE(0, 5),\
			CFG_VALUE_OR_DEFAULT,\
			"number of probes on each channel")

/*
 * <ini>
 * gScanProbeRepeatTime - Set the probe repeat time on each channel
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
#define CFG_SCAN_PROBE_REPEAT_TIME CFG_INI_UINT(\
			"gScanProbeRepeatTime",\
			0, 30, MCL_OR_WIN_VALUE(20, 50),\
			CFG_VALUE_OR_DEFAULT,\
			"probe repeat time on each channel")

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
#define CFG_ADAPTIVE_SCAN_DWELL_MODE CFG_INI_UINT(\
			"hostscan_adaptive_dwell_mode",\
			0, 4, 2,\
			CFG_VALUE_OR_DEFAULT,\
			"Enable adaptive dwell mode")

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
#define CFG_IS_BSSID_HINT_PRIORITY CFG_INI_UINT(\
			"is_bssid_hint_priority",\
			0, 1, 0,\
			CFG_VALUE_OR_DEFAULT, \
			"Set priority for connection with bssid_hint")

#define CFG_SCAN_ALL \
	CFG(CFG_DROP_BCN_ON_CHANNEL_MISMATCH) \
	CFG(CFG_ACTIVE_MAX_CHANNEL_TIME) \
	CFG(CFG_ACTIVE_MAX_2G_CHANNEL_TIME) \
	CFG(CFG_PASSIVE_MAX_CHANNEL_TIME) \
	CFG(CFG_SCAN_NUM_PROBES) \
	CFG(CFG_SCAN_PROBE_REPEAT_TIME) \
	CFG(CFG_ADAPTIVE_SCAN_DWELL_MODE) \
	CFG(CFG_IS_BSSID_HINT_PRIORITY)

#endif
