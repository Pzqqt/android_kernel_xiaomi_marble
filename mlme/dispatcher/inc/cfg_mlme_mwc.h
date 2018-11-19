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
 * DOC: This file contains configuration definitions for MLME WMC.
 */
#ifndef CFG_MLME_MWC_H_
#define CFG_MLME_MWC_H_

#ifdef MWS_COEX
/*
 * <ini>
 * gMwsCoex4gQuickTdm - Bitmap to control MWS-COEX 4G quick FTDM policy
 * @Min: 0x00000000
 * @Max: 0xFFFFFFFF
 * @Default: 0x00000000
 *
 * It is a 32 bit value such that the various bits represent as below:
 * Bit-0 : 0 - Don't allow quick FTDM policy (Default)
 *        1 - Allow quick FTDM policy
 * Bit 1-31 : reserved for future use
 *
 * It is used to enable or disable MWS-COEX 4G (LTE) Quick FTDM
 *
 * Usage: Internal
 *
 * </ini>
 */

#define CFG_MWS_COEX_4G_QUICK_FTDM CFG_INI_UINT( \
	"gMwsCoex4gQuickTdm", \
	0x00000000, \
	0xFFFFFFFF, \
	0x00000000, \
	CFG_VALUE_OR_DEFAULT, \
	"set mws-coex 4g quick ftdm policy")

/*
 * <ini>
 * gMwsCoex5gnrPwrLimit - Bitmap to set MWS-COEX 5G-NR power limit
 * @Min: 0x00000000
 * @Max: 0xFFFFFFFF
 * @Default: 0x00000000
 *
 * It is a 32 bit value such that the various bits represent as below:
 * Bit-0 : Don't apply user specific power limit,
 *        use internal power limit (Default)
 * Bit 1-2 : Invalid value (Ignored)
 * Bit 3-21 : Apply the specified value as the external power limit, in dBm
 * Bit 22-31 : Invalid value (Ignored)
 *
 * It is used to set MWS-COEX 5G-NR power limit
 *
 * Usage: Internal
 *
 * </ini>
 */

#define CFG_MWS_COEX_5G_NR_PWR_LIMIT CFG_INI_UINT( \
	"gMwsCoex5gnrPwrLimit", \
	0x00000000, \
	0xFFFFFFFF, \
	0x00000000, \
	CFG_VALUE_OR_DEFAULT, \
	"set mws-coex 5g-nr power limit")

#define CFG_MWC_ALL \
	CFG(CFG_MWS_COEX_4G_QUICK_FTDM) \
	CFG(CFG_MWS_COEX_5G_NR_PWR_LIMIT)

#else
#define CFG_MWC_ALL
#endif /* MWS_COEX */

#endif /* CFG_MLME_MWC_H_ */
