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

#ifndef __CFG_MLME_GENERIC_H
#define __CFG_MLME_GENERIC_H

#ifdef WLAN_FEATURE_11W
#define CFG_PMF_SA_QUERY_MAX_RETRIES_TYPE	CFG_INI_UINT
#define CFG_PMF_SA_QUERY_RETRY_INTERVAL_TYPE	CFG_INI_UINT
#else
#define CFG_PMF_SA_QUERY_MAX_RETRIES_TYPE	CFG_UINT
#define CFG_PMF_SA_QUERY_RETRY_INTERVAL_TYPE	CFG_UINT
#endif /*WLAN_FEATURE_11W*/

/*
 * pmfSaQueryMaxRetries - Control PMF SA query retries for SAP
 * @Min: 0
 * @Max: 20
 * @Default: 5
 *
 * This ini to set the number of PMF SA query retries for SAP
 *
 * Related: None.
 *
 * Supported Feature: PMF(11W)
 *
 */
#define CFG_PMF_SA_QUERY_MAX_RETRIES CFG_PMF_SA_QUERY_MAX_RETRIES_TYPE( \
		"pmfSaQueryMaxRetries", \
		0, \
		20, \
		5, \
		CFG_VALUE_OR_DEFAULT, \
		"PMF SA query retries for SAP")
/*
 * pmfSaQueryRetryInterval - Control PMF SA query retry interval
 * for SAP in ms
 * @Min: 10
 * @Max: 2000
 * @Default: 200
 *
 * This ini to set the PMF SA query retry interval for SAP in ms
 *
 * Related: None.
 *
 * Supported Feature: PMF(11W)
 *
 */
#define CFG_PMF_SA_QUERY_RETRY_INTERVAL CFG_PMF_SA_QUERY_RETRY_INTERVAL_TYPE( \
		"pmfSaQueryRetryInterval", \
		10, \
		2000, \
		200, \
		CFG_VALUE_OR_DEFAULT, \
		"PMF SA query retry interval for SAP")

#define CFG_RTT3_ENABLE CFG_BOOL( \
		"rtt3_enabled", \
		1, \
		"RTT3 enable/disable info")

#define CFG_GENERIC_ALL \
	CFG(CFG_PMF_SA_QUERY_MAX_RETRIES) \
	CFG(CFG_PMF_SA_QUERY_RETRY_INTERVAL) \
	CFG(CFG_RTT3_ENABLE)
#endif /* __CFG_MLME_GENERIC_H */
