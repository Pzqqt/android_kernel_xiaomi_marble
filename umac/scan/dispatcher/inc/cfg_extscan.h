/*
 * Copyright (c) 2012-2021 The Linux Foundation. All rights reserved.
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
 * DOC: This file contains centralized definitions of EXTSCAN component
 */
#ifndef _CONFIG_EXTSCAN_H_
#define _CONFIG_EXTSCAN_H_

#include "cfg_define.h"

#ifdef FEATURE_WLAN_EXTSCAN

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
#define CFG_EXTSCAN_PASSIVE_MAX_CHANNEL_TIME CFG_INI_UINT(\
		"gExtScanPassiveMaxChannelTime",\
		0, 500, 110, CFG_VALUE_OR_DEFAULT,\
		"ext scan passive max channel time")

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
#define CFG_EXTSCAN_PASSIVE_MIN_CHANNEL_TIME CFG_INI_UINT(\
		"gExtScanPassiveMinChannelTime",\
		0, 500, 60, CFG_VALUE_OR_DEFAULT,\
		"ext scan passive min channel time")

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

#define CFG_EXTSCAN_ACTIVE_MAX_CHANNEL_TIME CFG_INI_UINT(\
		"gExtScanActiveMaxChannelTime",\
		0, 110, 40, CFG_VALUE_OR_DEFAULT,\
		"ext scan active max channel time")

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
#define CFG_EXTSCAN_ACTIVE_MIN_CHANNEL_TIME CFG_INI_UINT(\
		"gExtScanActiveMinChannelTime",\
		0, 110, 20, CFG_VALUE_OR_DEFAULT,\
		"ext scan active min channel time")

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
#define CFG_EXTSCAN_ALLOWED CFG_INI_BOOL(\
		"gExtScanEnable",\
		1,\
		"ext scan enable")

#define CFG_EXTSCAN_ALL \
	CFG(CFG_EXTSCAN_PASSIVE_MAX_CHANNEL_TIME) \
	CFG(CFG_EXTSCAN_PASSIVE_MIN_CHANNEL_TIME) \
	CFG(CFG_EXTSCAN_ACTIVE_MAX_CHANNEL_TIME) \
	CFG(CFG_EXTSCAN_ACTIVE_MIN_CHANNEL_TIME) \
	CFG(CFG_EXTSCAN_ALLOWED)

#else
#define CFG_EXTSCAN_ALL
#endif
#endif
