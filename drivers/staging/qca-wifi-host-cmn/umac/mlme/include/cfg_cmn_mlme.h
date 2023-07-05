/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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
 * DOC: This file contains umac mlme related CFG/INI Items.
 */

#ifndef __CFG_CMN_MLME_H
#define __CFG_CMN_MLME_H

/*
 * <ini>
 * max_chan_switch_ie_enable - Flag to enable max chan switch IE support
 * @Min: false
 * @Max: true
 * @Default: false
 *
 * For non_ap platform, this flag will be enabled at later point and for ap
 * platform this flag will be disabled
 *
 * Related: None
 *
 * Supported Feature: Max channel switch IE
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_MLME_MAX_CHAN_SWITCH_IE_ENABLE \
	CFG_INI_BOOL("max_chan_switch_ie_enable", \
	PLATFORM_VALUE(false, false), \
	"To enable max channel switch IE")

#define CFG_CMN_MLME_ALL \
	CFG(CFG_MLME_MAX_CHAN_SWITCH_IE_ENABLE)

#endif /* __CFG_CMN_MLME_H */
