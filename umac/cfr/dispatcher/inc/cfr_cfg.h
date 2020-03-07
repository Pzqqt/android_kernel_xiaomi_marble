/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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
 * DOC: This file contains centralized cfg definitions of CFR component
 */
#ifndef __CFR_CONFIG_H
#define __CFR_CONFIG_H

/*
 * <ini>
 * cfr_disable - disable channel frequence response(CFR) feature
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to disable cfr feature.
 *
 * Related: None
 *
 * Supported Feature: cfr
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_CFR_DISABLE \
	CFG_INI_BOOL("cfr_disable", false, \
			"CFR disable")

#define CFG_CFR_ALL \
	CFG(CFG_CFR_DISABLE)

#endif /* __CFR_CONFIG_H */
