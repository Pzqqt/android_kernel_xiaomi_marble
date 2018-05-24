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

#ifndef __CFG_MLME_HT_CAPS_H
#define __CFG_MLME_HT_CAPS_H

/*
 * <ini>
 * gTxLdpcEnable - Config Param to enable Tx LDPC capability
 * @Min: 0
 * @Max: 3
 * @Default: 3
 *
 * This ini is used to enable/disable Tx LDPC capability
 * 0 - disable
 * 1 - HT LDPC enable
 * 2 - VHT LDPC enable
 * 3 - HT & VHT LDPC enable
 *
 * Related: STA/SAP/P2P/IBSS/NAN.
 *
 * Supported Feature: Concurrency/Standalone
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_TX_LDPC_ENABLE CFG_INI_UINT( \
		"gTxLdpcEnable", \
		0, \
		3, \
		3, \
		CFG_VALUE_OR_DEFAULT, \
		"Tx LDPC capability")

/*
 * <ini>
 * gEnableRXLDPC - Config Param to enable Rx LDPC capability
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable/disable Rx LDPC capability
 * 0 - disable Rx LDPC
 * 1 - enable Rx LDPC
 *
 * Related: STA/SAP/P2P/IBSS/NAN.
 *
 * Supported Feature: Concurrency/Standalone
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_RX_LDPC_ENABLE CFG_INI_BOOL( \
		"gEnableRXLDPC", \
		0, \
		"Rx LDPC capability")

/*
 * <ini>
 * gEnableTXSTBC - Enables/disables Tx STBC capability in STA mode
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to set default Tx STBC capability
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_TX_STBC_ENABLE CFG_INI_BOOL( \
		"gEnableTXSTBC", \
		0, \
		"Tx STBC capability")

/*
 * <ini>
 * gEnableRXSTBC - Enables/disables Rx STBC capability in STA mode
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to set default Rx STBC capability
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_RX_STBC_ENABLE CFG_INI_BOOL( \
		"gEnableRXSTBC", \
		1, \
		"Rx STBC capability")

/*
 * <ini>
 * gShortGI20Mhz - Short Guard Interval for HT20
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to set default short interval for HT20
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_SHORT_GI_20MHZ CFG_INI_BOOL( \
		"gShortGI20Mhz", \
		1, \
		"Short Guard Interval for HT20")

/*
 * <ini>
 * gShortGI40Mhz - It will check gShortGI20Mhz and
 * gShortGI40Mhz from session entry
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to set default gShortGI40Mhz
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_SHORT_GI_40MHZ CFG_INI_BOOL( \
		"gShortGI40Mhz", \
		1, \
		"Short Guard Interval for HT40")

#define CFG_HT_CAP_INFO CFG_UINT( \
		"ht_cap_info", \
		0, \
		65535, \
		364, \
		CFG_VALUE_OR_DEFAULT, \
		"HT cap info")

#define CFG_HT_CAPS_ALL \
	CFG(CFG_HT_CAP_INFO) \
	CFG(CFG_TX_LDPC_ENABLE) \
	CFG(CFG_RX_LDPC_ENABLE) \
	CFG(CFG_TX_STBC_ENABLE) \
	CFG(CFG_RX_STBC_ENABLE) \
	CFG(CFG_SHORT_GI_20MHZ) \
	CFG(CFG_SHORT_GI_40MHZ)

#endif /* __CFG_MLME_HT_CAPS_H */
