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

/**
 * DOC: This file contains centralized definitions of converged configuration.
 */

#ifndef __CFG_GREEN_AP_PARAMS_H
#define __CFG_GREEN_AP_PARAMS_H

/*
 * <ini>
 * gEnableGreenAp - Enable green ap feature
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable green ap feature
 *
 * Related: None
 *
 * Supported Feature: SAP
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_ENABLE_GREEN_AP_FEATURE CFG_INI_BOOL( \
		"gEnableGreenAp", \
		MCL_OR_WIN_VALUE(1, 0), \
		"enable green ap")

/*
 * <ini>
 * gEnableEGAP - Enable the enhanced green ap feature
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable the enhanced green ap feature
 *
 * Related: None
 *
 * Supported Feature: SAP
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_ENABLE_EGAP_FEATURE CFG_INI_BOOL( \
		"gEnableEGAP", \
		MCL_OR_WIN_VALUE(1,0), \
		"enable e-gap")
/*
 * <ini>
 * gEGAPInactTime - configure the inactive time for EGAP
 * @Min: 0
 * @Max: 300000
 * @Default: 2000
 *
 * This ini is used to configure the inactive time for EGAP
 *
 * Related: None
 *
 * Supported Feature: SAP
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_EGAP_INACT_TIME_FEATURE CFG_INI_UINT( \
		"gEGAPInactTime", \
		0, \
		300000, \
		2000, \
		CFG_VALUE_OR_DEFAULT, \
		"egap inactivity time")
/*
 * <ini>
 * gEGAPWaitTime - configure the wait time for EGAP
 * @Min: 0
 * @Max: 300000
 * @Default: 150
 *
 * This ini is used to configure the wait time for EGAP
 *
 * Related: None
 *
 * Supported Feature: SAP
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_EGAP_WAIT_TIME_FEATURE CFG_INI_UINT( \
		"gEGAPWaitTime", \
		0, \
		300000, \
		150, \
		CFG_VALUE_OR_DEFAULT, \
		"egap wait time")
/*
 * <ini>
 * gEGAPFeatures - Configure the EGAP flags
 * @Min: 0
 * @Max: 15
 * @Default: 3
 *
 * This ini is used to configure the EGAP flags
 *
 * Related: None
 *
 * Supported Feature: SAP
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_EGAP_FLAGS_FEATURE CFG_INI_UINT( \
		"gEGAPFeatures", \
		0, \
		15, \
		3, \
		CFG_VALUE_OR_DEFAULT, \
		"egap flag")

#ifdef WLAN_SUPPORT_GREEN_AP
#define CFG_GREEN_AP_ALL \
	CFG(CFG_ENABLE_GREEN_AP_FEATURE) \
	CFG(CFG_ENABLE_EGAP_FEATURE) \
	CFG(CFG_EGAP_INACT_TIME_FEATURE) \
	CFG(CFG_EGAP_WAIT_TIME_FEATURE) \
	CFG(CFG_EGAP_FLAGS_FEATURE)
#else
#define CFG_GREEN_AP_ALL
#endif

#endif /* __CFG_GREEN_AP_PARAMS_H */

