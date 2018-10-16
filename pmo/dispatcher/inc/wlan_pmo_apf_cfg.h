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

#ifndef WLAN_PMO_APF_CFG_H__
#define WLAN_PMO_APF_CFG_H__

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

#define CFG_PMO_APF_ALL \
	CFG(CFG_PMO_APF_ENABLE)

#endif /* WLAN_PMO_APF_CFG_H__ */
