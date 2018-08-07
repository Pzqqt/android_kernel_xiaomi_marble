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

#ifndef __CFG_MLME_RATES_H
#define __CFG_MLME_RATES_H

/*
 * <ini>
 * gMaxHTMCSForTxData - max HT mcs for TX
 * @Min: 0
 * @Max: 383
 * @Default: 0
 *
 * This ini is used to configure the max HT mcs
 * for tx data.
 *
 * Usage: External
 *
 * bits 0-15:  max HT mcs
 * bits 16-31: zero to disable, otherwise enable.
 *
 * </ini>
 */
#define CFG_INI_MAX_HT_MCS_FOR_TX_DATA CFG_INI_UINT( \
		"gMaxHTMCSForTxData", \
		0, \
		0x17f, \
		0, \
		CFG_VALUE_OR_DEFAULT, \
		"Max HT Mcs for Tx Data")

/*
 * <ini>
 * gDisableABGRateForTxData - disable abg rate for tx data
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to disable abg rate for tx data.
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_INI_DISABLE_ABG_RATE_FOR_TX_DATA CFG_INI_BOOL( \
		"gDisableABGRateForTxData", \
		0, \
		"Disable ABG RATE for TX Data")

/*
 * gSapMaxMCSForTxData - sap 11n max mcs
 * @Min: 0
 * @Max: 383
 * @Default: 0
 *
 * This ini configure SAP 11n max mcs
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_INI_SAP_MAX_MCS_FOR_TX_DATA CFG_INI_UINT( \
		"gSapMaxMCSForTxData", \
		0, \
		383, \
		0, \
		CFG_VALUE_OR_DEFAULT, \
		"SAP Max MCS for TX Data")

/*
 * <ini>
 * disable_high_ht_mcs_2x2 - disable high mcs index for 2nd stream in 2.4G
 * @Min: 0
 * @Max: 8
 * @Default: 0
 *
 * This ini is used to disable high HT MCS index for 2.4G STA connection.
 * It has been introduced to resolve IOT issue with one of the vendor.
 *
 * Note: This INI is not useful with 1x1 setting. If some platform supports
 * only 1x1 then this INI is not useful.
 *
 * 0 - It won't disable any HT MCS index (just like normal HT MCS)
 * 1 - It will disable 15th bit from HT RX MCS set (from 8-15 bits slot)
 * 2 - It will disable 14th & 15th bits from HT RX MCS set
 * 3 - It will disable 13th, 14th, & 15th bits from HT RX MCS set
 * and so on.
 *
 * Related: STA
 *
 * Supported Feature: 11n
 *
 * Usage: External
 */
#define CFG_INI_DISABLE_HIGH_HT_RX_MCS_2x2 CFG_INI_UINT( \
		"disable_high_ht_mcs_2x2", \
		0, \
		8, \
		0, \
		CFG_VALUE_OR_DEFAULT, \
		"Disable high MCS index for 2x2")

#define CFG_CFP_PERIOD CFG_UINT( \
		"cfpPeriod", \
		0, \
		255, \
		1, \
		CFG_VALUE_OR_DEFAULT, \
		"CFP Period")

#define CFG_CFP_MAX_DURATION CFG_UINT( \
		"cfpMaxDuration", \
		0, \
		65535, \
		30000, \
		CFG_VALUE_OR_DEFAULT, \
		"CFP Max Duration")

#define CFG_RATES_ALL \
	CFG(CFG_INI_MAX_HT_MCS_FOR_TX_DATA) \
	CFG(CFG_INI_DISABLE_ABG_RATE_FOR_TX_DATA) \
	CFG(CFG_INI_SAP_MAX_MCS_FOR_TX_DATA) \
	CFG(CFG_INI_DISABLE_HIGH_HT_RX_MCS_2x2) \
	CFG(CFG_CFP_PERIOD) \
	CFG(CFG_CFP_MAX_DURATION)

#endif /* __CFG_MLME_RATES_H */
