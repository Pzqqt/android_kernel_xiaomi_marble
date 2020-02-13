/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: This file contains centralized definitions of DCS component
 */
#ifndef __CONFIG_DCS_H
#define __CONFIG_DCS_H

#include "cfg_define.h"

/*
 * <ini>
 * gEnableDcs - Enable/Disable DCS
 * @Min: 0
 * @Max: 3
 * @Default: 0
 *
 * This ini is used to enable/disable DCS. Configurations are as follows:
 * 0 - Disable DCS.
 * 1 - Enable DCS for CW interference mitigation(CW_IM).
 * 2 - Enable DCS for WLAN interference mitigation(WLAN_IM).
 * 3 - Enable both DCS for CW_IM and DCS for WLAN_IM.
 *
 * Related: None
 *
 * Supported Feature: DCS
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_DCS_ENABLE CFG_INI_UINT(\
		"gEnableDcs",\
		0, 3, 0,\
		CFG_VALUE_OR_DEFAULT, "Enable DCS")

/*
 * <ini>
 * dcs_debug - Configure dcs debug trace level for debug purpose
 * @Min: 0
 * @Max: 2
 * @Default: 0
 *
 * This ini is used to configure dcs debug trace level for debug purpose
 *
 *
 * Related: None
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_DCS_DEBUG CFG_INI_UINT(\
		"dcs_debug",\
		0, 2, 0,\
		CFG_VALUE_OR_DEFAULT,\
		"dcs debug trace level")

/*
 * <ini>
 * dcs_coch_intfr_threshold - Configure co-channel interference threshold
 * @Min: 0
 * @Max: 0xFFFFFFFF
 * @Default: 30
 *
 * This ini is used to configure co-channel interference threshold
 *
 *
 * Related: None
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_DCS_COCH_INTFR_THRESHOLD CFG_INI_UINT(\
		"dcs_coch_intfr_threshold",\
		0, 0xFFFFFFFF, 30,\
		CFG_VALUE_OR_DEFAULT,\
		"dcs co-channel interference threshold level")

/*
 * <ini>
 * dcs_tx_err_threshold - Configure transmission failure rate threshold
 * @Min: 0
 * @Max: 0xFFFFFFFF
 * @Default: 30
 *
 * This ini is used to configure transmission failure rate threshold
 *
 *
 * Related: None
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_DCS_TX_ERR_THRESHOLD CFG_INI_UINT(\
		"dcs_tx_err_threshold",\
		0, 0xFFFFFFFF, 30,\
		CFG_VALUE_OR_DEFAULT,\
		"dcs transmission failure rate threshold")

/*
 * <ini>
 * dcs_phy_err_penalty - Configure channel time wasted due to each Phy
 * error(phy error penalty)
 * @Min: 0
 * @Max: 0xFFFFFFFF
 * @Default: 500
 *
 * This ini is used to configure phy error penalty
 *
 *
 * Related: None
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_DCS_PHY_ERR_PENALTY CFG_INI_UINT(\
		"dcs_phy_err_penalty",\
		0, 0xFFFFFFFF, 500,\
		CFG_VALUE_OR_DEFAULT,\
		"dcs phy error penalty")

/*
 * <ini>
 * dcs_phy_err_threshold - Configure phy err threshold
 * @Min: 0
 * @Max: 0xFFFFFFFF
 * @Default: 300
 *
 * This ini is used to configure phy error threshold
 *
 *
 * Related: None
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_DCS_PHY_ERR_THRESHOLD CFG_INI_UINT(\
		"dcs_phy_err_threshold",\
		0, 0xFFFFFFFF, 300,\
		CFG_VALUE_OR_DEFAULT,\
		"dcs phy error threshold")

/*
 * <ini>
 * dcs_user_max_cu - Configure tx channel utilization due to AP's tx and rx
 * @Min: 0
 * @Max: 0xFFFFFFFF
 * @Default: 50
 *
 * This ini is used to configure tx channel utilization due to AP's tx and rx
 *
 *
 * Related: None
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_DCS_USER_MAX_CU CFG_INI_UINT(\
		"user_max_cu",\
		0, 0xFFFFFFFF, 50,\
		CFG_VALUE_OR_DEFAULT,\
		"dcs tx channel utilization")

/*
 * <ini>
 * dcs_radar_err_threshold - Configure radar error threshold
 * @Min: 0
 * @Max: 0xFFFFFFFF
 * @Default: 1000
 *
 * This ini is used to configure radar error threshold
 *
 *
 * Related: None
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_DCS_RADAR_ERR_THRESHOLD CFG_INI_UINT(\
		"dcs_radar_err_threshold",\
		0, 0xFFFFFFFF, 1000,\
		CFG_VALUE_OR_DEFAULT,\
		"dcs radar error threshold")

/*
 * <ini>
 * dcs_intfr_detection_threshold - Configure interference detection threshold
 * @Min: 0
 * @Max: 0xFFFFFFFF
 * @Default: 6
 *
 * This ini is used to configure interference detection threshold
 *
 *
 * Related: None
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_DCS_INTFR_DETECTION_THRESHOLD CFG_INI_UINT(\
		"dcs_intfr_detection_threshold",\
		0, 0xFFFFFFFF, 6,\
		CFG_VALUE_OR_DEFAULT,\
		"dcs interference detection threshold")

/*
 * <ini>
 * dcs_intfr_detection_window - Configure interference sampling window
 * @Min: 0
 * @Max: 0xFFFFFFFF
 * @Default: 10
 *
 * This ini is used to configure interference sampling window
 *
 *
 * Related: None
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_DCS_INTFR_DETECTION_WINDOW CFG_INI_UINT(\
		"dcs_intfr_detection_window",\
		0, 0xFFFFFFFF, 10,\
		CFG_VALUE_OR_DEFAULT,\
		"dcs interference sampling window")

/*
 * <ini>
 * dcs_disable_threshold_per_5mins - In five minutes, if dcs happen
 * more than threshold, then disable dcs for some time
 * @Min: 0
 * @Max: 10
 * @Default: 3
 *
 * This ini is used to dcs happen times threshold in five minutes
 *
 *
 * Related: None
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_DCS_DISABLE_THRESHOLD_PER_5MINS CFG_INI_UINT(\
		"dcs_disable_thresh_per_5mins",\
		0, 10, 3,\
		CFG_VALUE_OR_DEFAULT,\
		"dcs happen times threshold in five minutes")

/*
 * <ini>
 * dcs_restart_delay - When dcs happen more than threshold in five minutes,
 * then start to disable dcs for some minutes, then enable dcs again.
 * @Min: 0
 * @Max: 0xFFFFFFFF
 * @Default: 30
 *
 * This ini is used to configure dcs disable time length in minute unit
 *
 *
 * Related: None
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_DCS_RESTART_DELAY CFG_INI_UINT(\
		"dcs_restart_delay",\
		0, 0xFFFFFFFF, 30,\
		CFG_VALUE_OR_DEFAULT, "dcs restart delay")

#define CFG_DCS_ALL \
	CFG(CFG_DCS_ENABLE) \
	CFG(CFG_DCS_DEBUG) \
	CFG(CFG_DCS_COCH_INTFR_THRESHOLD) \
	CFG(CFG_DCS_TX_ERR_THRESHOLD) \
	CFG(CFG_DCS_PHY_ERR_PENALTY) \
	CFG(CFG_DCS_PHY_ERR_THRESHOLD) \
	CFG(CFG_DCS_USER_MAX_CU) \
	CFG(CFG_DCS_RADAR_ERR_THRESHOLD) \
	CFG(CFG_DCS_INTFR_DETECTION_THRESHOLD) \
	CFG(CFG_DCS_INTFR_DETECTION_WINDOW) \
	CFG(CFG_DCS_DISABLE_THRESHOLD_PER_5MINS) \
	CFG(CFG_DCS_RESTART_DELAY)

#endif /* __CONFIG_DCS_H */
