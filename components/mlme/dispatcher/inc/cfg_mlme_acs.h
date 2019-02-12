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

#ifndef __CFG_MLME_ACS_H
#define __CFG_MLME_ACS_H

/*
 * <ini>
 * acs_with_more_param- Enable acs calculation with more param.
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable acs calculation with more param.
 *
 * Related: NA
 *
 * Supported Feature: ACS
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_ACS_WITH_MORE_PARAM CFG_INI_BOOL( \
		"acs_with_more_param", \
		0, \
		"Enable ACS with more param")

/*
 * <ini>
 * AutoChannelSelectWeight - ACS channel weight
 * @Min: 0
 * @Max: 0xFFFFFFFF
 * @Default: 0x000000FF
 *
 * This ini is used to adjust weight of factors in
 * acs algorithm.
 *
 * Supported Feature: ACS
 *
 * Usage: Internal/External
 *
 * bits 0-3:   rssi weight
 * bits 4-7:   bss count weight
 * bits 8-11:  noise floor weight
 * bits 12-15: channel free weight
 * bits 16-19: tx power range weight
 * bits 20-23: tx power throughput weight
 * bits 24-31: reserved
 *
 * </ini>
 */

#define CFG_AUTO_CHANNEL_SELECT_WEIGHT CFG_INI_UINT( \
		"AutoChannelSelectWeight", \
		0, \
		0xFFFFFFFF, \
		0x000000FF, \
		CFG_VALUE_OR_DEFAULT, \
		"Adjust weight factor in ACS")

/*
 * <ini>
 * gvendor_acs_support - vendor based channel selection manager
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * Enabling this parameter will force driver to use user application based
 * channel selection algo instead of driver based auto channel selection
 * logic.
 *
 * Supported Feature: ACS
 *
 * Usage: External/Internal
 *
 * </ini>
 */

#define CFG_USER_AUTO_CHANNEL_SELECTION CFG_INI_BOOL( \
		"gvendor_acs_support", \
		0, \
		"Vendor channel selection manager")

/*
 * <ini>
 * gacs_support_for_dfs_lte_coex - acs support for lte coex and dfs event
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * Enabling this parameter will force driver to use user application based
 * channel selection algo for channel selection in case of dfs and lte
 * coex event.
 *
 * Supported Feature: ACS
 *
 * Usage: Internal
 *
 * </ini>
 */

#define CFG_USER_ACS_DFS_LTE CFG_INI_BOOL( \
		"gacs_support_for_dfs_lte_coex", \
		0, \
		"Acs support for lte coex and dfs")

/*
 * <ini>
 * acs_policy - External ACS policy control
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * Values are per enum hdd_external_acs_policy.
 *
 * This ini is used to control the external ACS policy.
 *
 * 0 -Preferable for ACS to select a
 *    channel with non-zero pcl weight.
 * 1 -Mandatory for ACS to select a
 *    channel with non-zero pcl weight.
 *
 * Related: None
 *
 * Supported Feature: ACS
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_EXTERNAL_ACS_POLICY CFG_INI_BOOL( \
		"acs_policy", \
		1, \
		"External ACS Policy Control")

#define CFG_ACS_ALL \
	CFG(CFG_ACS_WITH_MORE_PARAM) \
	CFG(CFG_AUTO_CHANNEL_SELECT_WEIGHT) \
	CFG(CFG_USER_AUTO_CHANNEL_SELECTION) \
	CFG(CFG_USER_ACS_DFS_LTE) \
	CFG(CFG_EXTERNAL_ACS_POLICY)

#endif /* __CFG_MLME_ACS_H */
